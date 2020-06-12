/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


#include "client.h"
#include "snd_loc.h"

#ifdef OGG_SUPPORT

#define BUFFER_SIZE		16384

static bgTrack_t	s_bgTrack;

static channel_t	*s_streamingChannel;


/*
=======================================================================

OGG VORBIS STREAMING

=======================================================================
*/


static size_t ovc_read (void *ptr, size_t size, size_t nmemb, void *datasource)
{
	bgTrack_t	*track = (bgTrack_t *)datasource;

	if (!size || !nmemb)
		return 0;
#ifdef OGG_DIRECT_FILE
	return fread(ptr, 1, size * nmemb, track->file) / size;
#else
	return FS_Read(ptr, size * nmemb, track->file) / size;
#endif
}


static int ovc_seek (void *datasource, ogg_int64_t offset, int whence)
{
	bgTrack_t	*track = (bgTrack_t *)datasource;

	switch (whence)
	{
	case SEEK_SET:
#ifdef OGG_DIRECT_FILE
		fseek(track->file, (int)offset, SEEK_SET);
		break;
	case SEEK_CUR:
		fseek(track->file, (int)offset, SEEK_CUR);
		break;
	case SEEK_END:
		fseek(track->file, (int)offset, SEEK_END);
#else
		FS_Seek(track->file, (int)offset, FS_SEEK_SET);
		break;
	case SEEK_CUR:
		FS_Seek(track->file, (int)offset, FS_SEEK_CUR);
		break;
	case SEEK_END:
		FS_Seek(track->file, (int)offset, FS_SEEK_END);
#endif
		break;
	default:
		return -1;
	}

	return 0;
}


static int ovc_close (void *datasource)
{
	return 0;
}


static long ovc_tell (void *datasource)
{
	bgTrack_t	*track = (bgTrack_t *)datasource;

#ifdef OGG_DIRECT_FILE
	return ftell(track->file);
#else
	return FS_Tell(track->file);
#endif
}


/*
=================
S_OpenBackgroundTrack
=================
*/
static qboolean S_OpenBackgroundTrack (const char *name, bgTrack_t *track)
{
	OggVorbis_File	*vorbisFile;
	vorbis_info		*vorbisInfo;
	ov_callbacks	vorbisCallbacks = {ovc_read, ovc_seek, ovc_close, ovc_tell};
#ifdef OGG_DIRECT_FILE
	char	filename[1024];
	char	*path = NULL;
#endif

	//Com_Printf("Opening background track: %s\n", name);

#ifdef OGG_DIRECT_FILE
	do {
		path = FS_NextPath( path );
		Com_sprintf( filename, sizeof(filename), "%s/%s", path, name );
		if ( (track->file = fopen(filename, "rb")) != 0)
			break;
	} while ( path );
#else
	FS_FOpenFile(name, &track->file, FS_READ);
#endif
	if (!track->file)
	{
		Com_Printf(S_COLOR_YELLOW"S_OpenBackgroundTrack: couldn't find %s\n", name);
		return false;
	}

	track->vorbisFile = vorbisFile = Z_Malloc(sizeof(OggVorbis_File));

	//Com_Printf("Opening callbacks for background track\n");

	// bombs out here- ovc_read, FS_Read 0 bytes error
	if (ov_open_callbacks(track, vorbisFile, NULL, 0, vorbisCallbacks) < 0)
	{
		Com_Printf(S_COLOR_YELLOW"S_OpenBackgroundTrack: couldn't open OGG stream (%s)\n", name);
		return false;
	}

	//Com_Printf("Getting info for background track\n");

	vorbisInfo = ov_info(vorbisFile, -1);
	if (vorbisInfo->channels != 1 && vorbisInfo->channels != 2)
	{
		Com_Printf(S_COLOR_YELLOW"S_OpenBackgroundTrack: only mono and stereo OGG files supported (%s)\n", name);
		return false;
	}

	track->start = ov_raw_tell(vorbisFile);
	track->rate = vorbisInfo->rate;
	track->width = 2;
	track->channels = vorbisInfo->channels; // Knightmare added
	track->format = (vorbisInfo->channels == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;

	//Com_Printf("Vorbis info: frequency: %i channels: %i bitrate: %i\n",
	//	vorbisInfo->rate, vorbisInfo->channels, vorbisInfo->bitrate_nominal);

	return true;
}


/*
=================
S_CloseBackgroundTrack
=================
*/
static void S_CloseBackgroundTrack (bgTrack_t *track)
{

	if (track->vorbisFile)
	{
		ov_clear(track->vorbisFile);

		Z_Free(track->vorbisFile);
		track->vorbisFile = NULL;
	}

	if (track->file)
	{
	#ifdef OGG_DIRECT_FILE
		fclose(track->file);
	#else
		FS_FCloseFile(track->file);
	#endif
		track->file = 0;
	}
}

#if 0
/*
=================
S_StreamBackgroundTrack
=================
*/
void S_StreamBackgroundTrack (void)
{
	byte		data[BUFFER_SIZE];
	int			queued = 0; //, processed, state;
	int			size, read, dummy;
	//unsigned	buffer;
	int			samples; // Knightmare added

	if (!s_bgTrack.file || !s_musicvolume->value)
		return;

	if (!s_streamingChannel)
		return;

	// Unqueue and delete any processed buffers
	/*qalGetSourcei(s_streamingChannel->sourceNum, AL_BUFFERS_PROCESSED, &processed);
	if (processed > 0){
		while (processed--){
			qalSourceUnqueueBuffers(s_streamingChannel->sourceNum, 1, &buffer);
			qalDeleteBuffers(1, &buffer);
		}
	}*/

	//Com_Printf("Streaming background track\n");

	// Make sure we always have at least 4 buffers in the queue
	//qalGetSourcei(s_streamingChannel->sourceNum, AL_BUFFERS_QUEUED, &queued);
	while (queued < 4)
	{
		size = 0;
		// Stream from disk
		while (size < BUFFER_SIZE)
		{
			read = ov_read(s_bgTrack.vorbisFile, data + size, BUFFER_SIZE - size, 0, 2, 1, &dummy);
			if (read == 0)
			{	// End of file
				if (!s_bgTrack.looping)
				{	// Close the intro track
					S_CloseBackgroundTrack(&s_bgTrack);

					// Open the loop track
					if (!S_OpenBackgroundTrack(s_bgTrack.loopName, &s_bgTrack))
					{
						S_StopBackgroundTrack();
						return;
					}

					s_bgTrack.looping = true;
				}

				// Restart the track, skipping over the header
				ov_raw_seek(s_bgTrack.vorbisFile, (ogg_int64_t)s_bgTrack.start);

				// Try streaming again
				read = ov_read(s_bgTrack.vorbisFile, data + size, BUFFER_SIZE - size, 0, 2, 1, &dummy);
			}

			if (read <= 0)
			{	// An error occurred
				S_StopBackgroundTrack();
				return;
			}

			size += read;
		}

		// Knightmare added
		samples = size / (s_bgTrack.width * s_bgTrack.channels);
		S_RawSamples(samples, s_bgTrack.rate,s_bgTrack. width, s_bgTrack.channels, data, true);

		// Upload and queue the new buffer
		/*qalGenBuffers(1, &buffer);
		qalBufferData(buffer, s_bgTrack.format, data, size, s_bgTrack.rate);
		qalSourceQueueBuffers(s_streamingChannel->sourceNum, 1, &buffer);*/

		queued++;
	}


	// Update volume
	//qalSourcef(s_streamingChannel->sourceNum, AL_GAIN, s_musicVolume->value);

	// If not playing, then do so
	/*qalGetSourcei(s_streamingChannel->sourceNum, AL_SOURCE_STATE, &state);
	if (state != AL_PLAYING)
		qalSourcePlay(s_streamingChannel->sourceNum);*/
}
#endif

/*
============
S_UpdateBackgroundTrack
============
*/
void S_UpdateBackgroundTrack (void)
{
	int		samples, maxSamples;
	int		read, maxRead, total, dummy;
	float	scale;
	byte	data[MAX_RAW_SAMPLES*4];

	if (!s_bgTrack.file || !s_musicvolume->value)
		return;

	if (!s_streamingChannel)
		return;

	if (s_rawend < paintedtime)
		s_rawend = paintedtime;

	scale = (float)s_bgTrack.rate / dma.speed;
	maxSamples = sizeof(data) / s_bgTrack.channels / s_bgTrack.width;

	while (1)
	{
		samples = (paintedtime + MAX_RAW_SAMPLES - s_rawend) * scale;
		if (samples <= 0)
			return;
		if (samples > maxSamples)
			samples = maxSamples;
		maxRead = samples * s_bgTrack.channels * s_bgTrack.width;

		total = 0;
		while (total < maxRead)
		{
			read = ov_read(s_bgTrack.vorbisFile, data + total, maxRead - total, 0, 2, 1, &dummy);
			if (!read)
			{	// End of file
				if (!s_bgTrack.looping)
				{	// Close the intro track
					S_CloseBackgroundTrack(&s_bgTrack);

					// Open the loop track
					if (!S_OpenBackgroundTrack(s_bgTrack.loopName, &s_bgTrack))
					{
						S_StopBackgroundTrack();
						return;
					}
					s_bgTrack.looping = true;
				}

				// Restart the track, skipping over the header
				ov_raw_seek(s_bgTrack.vorbisFile, (ogg_int64_t)s_bgTrack.start);
			}

			total += read;
		}
		S_RawSamples (samples, s_bgTrack.rate, s_bgTrack.width, s_bgTrack.channels, data, true );
	}
}


// =====================================================================

void Q_strncpyz (char *dst, const char *src, int dstSize);

/*
=================
S_StartBackgroundTrack
=================
*/
void S_StartBackgroundTrack (const char *introTrack, const char *loopTrack)
{
	if (!sound_started)
		return;

	// Stop any playing tracks
	S_StopBackgroundTrack();

	// Start it up
	Q_strncpyz(s_bgTrack.introName, introTrack, sizeof(s_bgTrack.introName));
	Q_strncpyz(s_bgTrack.loopName, loopTrack, sizeof(s_bgTrack.loopName));

	S_StartStreaming();

	//BC deleteme
	//Com_Printf("intro name  %s\n", s_bgTrack.introName);
	//Com_Printf("channel  %s\n", s_bgTrack.channels);
	//Com_Printf("loop  %d\n", s_bgTrack.looping);
	//Com_Printf("loopname  %s\n", s_bgTrack.loopName);
	

	// Open the intro track
	if (!S_OpenBackgroundTrack(s_bgTrack.introName, &s_bgTrack))
	{
		S_StopBackgroundTrack();
		return;
	}

	S_UpdateBackgroundTrack();
	//S_StreamBackgroundTrack();
}

/*
=================
S_StopBackgroundTrack
=================
*/
void S_StopBackgroundTrack (void)
{
	if (!sound_started)
		return;

	S_StopStreaming();

	S_CloseBackgroundTrack(&s_bgTrack);

	memset(&s_bgTrack, 0, sizeof(bgTrack_t));
}

/*
=================
S_StartStreaming
=================
*/
void S_StartStreaming (void)
{
	if (!sound_started)
		return;

	if (s_streamingChannel)
		return;		// Already started

	s_streamingChannel = S_PickChannel(0, 0);
	if (!s_streamingChannel)
		return;

	s_streamingChannel->streaming = true;

	// FIXME: OpenAL bug?
	/*qalDeleteSources(1, &s_streamingChannel->sourceNum);
	qalGenSources(1, &s_streamingChannel->sourceNum);*/

	// Set up the source
	/*qalSourcei(s_streamingChannel->sourceNum, AL_BUFFER, 0);
	qalSourcei(s_streamingChannel->sourceNum, AL_LOOPING, AL_FALSE);
	qalSourcei(s_streamingChannel->sourceNum, AL_SOURCE_RELATIVE, AL_TRUE);
	qalSourcefv(s_streamingChannel->sourceNum, AL_POSITION, vec3_origin);
	qalSourcefv(s_streamingChannel->sourceNum, AL_VELOCITY, vec3_origin);
	qalSourcef(s_streamingChannel->sourceNum, AL_REFERENCE_DISTANCE, 1.0);
	qalSourcef(s_streamingChannel->sourceNum, AL_MAX_DISTANCE, 1.0);
	qalSourcef(s_streamingChannel->sourceNum, AL_ROLLOFF_FACTOR, 0.0);*/
}

/*
=================
S_StopStreaming
=================
*/
void S_StopStreaming (void)
{
	//int			processed;
	//unsigned	buffer;

	if (!sound_started)
		return;

	if (!s_streamingChannel)
		return;		// Already stopped

	s_streamingChannel->streaming = false;

	// Clean up the source
	/*qalSourceStop(s_streamingChannel->sourceNum);

	qalGetSourcei(s_streamingChannel->sourceNum, AL_BUFFERS_PROCESSED, &processed);
	if (processed > 0){
		while (processed--){
			qalSourceUnqueueBuffers(s_streamingChannel->sourceNum, 1, &buffer);
			qalDeleteBuffers(1, &buffer);
		}
	}

	qalSourcei(s_streamingChannel->sourceNum, AL_BUFFER, 0);

	// FIXME: OpenAL bug?
	qalDeleteSources(1, &s_streamingChannel->sourceNum);
	qalGenSources(1, &s_streamingChannel->sourceNum);*/

	s_streamingChannel = NULL;
}

#if 0
/*
=================
S_StreamRawSamples

Cinematic streaming
=================
*/
void S_StreamRawSamples (const byte *data, int samples, int rate, int width, int channels)
{
	int			processed, state, size;
	unsigned	format, buffer;

	if (!s_initialized)
		return;

	if (!s_streamingChannel)
		return;

	// Unqueue and delete any processed buffers
	qalGetSourcei(s_streamingChannel->sourceNum, AL_BUFFERS_PROCESSED, &processed);
	if (processed > 0){
		while (processed--){
			qalSourceUnqueueBuffers(s_streamingChannel->sourceNum, 1, &buffer);
			qalDeleteBuffers(1, &buffer);
		}
	}

	// Calculate buffer size
	size = samples * width * channels;

	// Set buffer format
	if (width == 2)
	{
		if (channels == 2)
			format = AL_FORMAT_STEREO16;
		else
			format = AL_FORMAT_MONO16;
	}
	else
	{
		if (channels == 2)
			format = AL_FORMAT_STEREO8;
		else
			format = AL_FORMAT_MONO8;
	}

	// Upload and queue the new buffer
	qalGenBuffers(1, &buffer);
	qalBufferData(buffer, format, (byte *)data, size, rate);
	qalSourceQueueBuffers(s_streamingChannel->sourceNum, 1, &buffer);

	// Update volume
	qalSourcef(s_streamingChannel->sourceNum, AL_GAIN, s_sfxVolume->value);

	// If not playing, then do so
	qalGetSourcei(s_streamingChannel->sourceNum, AL_SOURCE_STATE, &state);
	if (state != AL_PLAYING)
		qalSourcePlay(s_streamingChannel->sourceNum);
}
#endif

#endif // OGG_SUPPORT
