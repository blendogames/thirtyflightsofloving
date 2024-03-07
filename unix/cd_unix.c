/*
/*
===========================================================================
Copyright (C) 1997-2001 Id Software, Inc.

This file is part of Quake 2 source code.

Quake 2 source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake 2 source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake 2 source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// Quake is a trademark of Id Software, Inc., (c) 1996 Id Software, Inc. All
// rights reserved.

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#if defined(__FreeBSD__)
  #include <sys/cdio.h>
#else
  #include <linux/cdrom.h>
#endif

#include "../client/client.h"

static qboolean cdValid = false;
static qboolean	playing = false;
static qboolean	wasPlaying = false;
static qboolean	initialized = false;
static qboolean	enabled = true;
static qboolean playLooping = false;
static float	cdvolume;
static byte 	remap[100];
static byte		playTrack;
static byte		maxTrack;

static int cdfile = -1;

//static char cd_dev[64] = "/dev/cdrom";

cvar_t	*cd_volume;
cvar_t	*cd_nocd;
cvar_t	*cd_loopcount;
cvar_t	*cd_looptrack;
cvar_t	*cd_dev;

int		loopcounter;

void CDAudio_Pause(void);

static void CDAudio_Eject(void)
{
	if (cdfile == -1 || !enabled)
		return; // no cd init'd

#if defined(__FreeBSD__)
	if ( ioctl(cdfile, CDIOCEJECT) == -1 ) 
		Com_DPrintf("ioctl cdioeject failed\n");
#else
	if ( ioctl(cdfile, CDROMEJECT) == -1 ) 
		Com_DPrintf("ioctl cdromeject failed\n");
#endif
}


static void CDAudio_CloseDoor(void)
{
	if (cdfile == -1 || !enabled)
		return; // no cd init'd

#if defined(__FreeBSD__)
	if ( ioctl(cdfile, CDIOCCLOSE) == -1 ) 
		Com_DPrintf("ioctl cdiocclose failed\n");
#else
	if ( ioctl(cdfile, CDROMCLOSETRAY) == -1 ) 
		Com_DPrintf("ioctl cdromclosetray failed\n");
#endif
}

static int CDAudio_GetAudioDiskInfo(void)
{
#if defined(__FreeBSD__)
	struct ioc_toc_header tochdr;
#endif
#ifdef __linux__
	struct cdrom_tochdr tochdr;
#endif

	cdValid = false;

#if defined(__FreeBSD__)
	if ( ioctl(cdfile, CDIOREADTOCHEADER, &tochdr) == -1 ) 
    {
      Com_DPrintf("ioctl cdioreadtocheader failed\n");
	  return -1;
    }
#endif
#ifdef __linux__
	if ( ioctl(cdfile, CDROMREADTOCHDR, &tochdr) == -1 ) 
    {
      Com_DPrintf("ioctl cdromreadtochdr failed\n");
	  return -1;
    }
#endif

#if defined(__FreeBSD__)
	if (tochdr.starting_track < 1)
#endif
#ifdef __linux__
	if (tochdr.cdth_trk0 < 1)
#endif
	{
		Com_DPrintf("CDAudio: no music tracks\n");
		return -1;
	}

	cdValid = true;
#if defined(__FreeBSD__)
	maxTrack = tochdr.ending_track;
#endif
#ifdef __linux__
	maxTrack = tochdr.cdth_trk1;
#endif

	return 0;
}


void CDAudio_Play2(int track, qboolean looping)
{
#if defined(__FreeBSD__)
	struct ioc_read_toc_entry entry;
	struct cd_toc_entry toc_buffer;
	struct ioc_play_track ti;
#endif
#if defined(__linux__)
	struct cdrom_tocentry entry;
	struct cdrom_ti ti;
#endif

	if (cdfile == -1 || !enabled)
		return;
	
	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo();
		if (!cdValid)
			return;
	}

	track = remap[track];

	if (track < 1 || track > maxTrack)
	{
		Com_DPrintf("CDAudio: Bad track number %u.\n", track);
		return;
	}

#if defined(__FreeBSD__)
	#define CDROM_DATA_TRACK 4
	bzero((char *)&toc_buffer, sizeof(toc_buffer));
	entry.data_len = sizeof(toc_buffer);
	entry.data = &toc_buffer;
	// don't try to play a non-audio track
	entry.starting_track = track;
	entry.address_format = CD_MSF_FORMAT;
    if ( ioctl(cdfile, CDIOREADTOCENTRYS, &entry) == -1 )
	{
		Com_DPrintf("ioctl cdromreadtocentry failed\n");
		return;
	}
	if (toc_buffer.control == CDROM_DATA_TRACK)
#endif
#if defined(__linux__)
	// don't try to play a non-audio track
	entry.cdte_track = track;
	entry.cdte_format = CDROM_LBA;
    if ( ioctl(cdfile, CDROMREADTOCENTRY, &entry) == -1 )
	{
		Com_DPrintf("ioctl cdromreadtocentry failed\n");
		return;
	}
	if (entry.cdte_ctrl == CDROM_DATA_TRACK)
#endif
	{
		Com_Printf("CDAudio: track %i is not audio\n", track);
		return;
	}


	if (playing)
	{
		if (playTrack == track)
			return;
		CDAudio_Stop();
	}

#if defined(__FreeBSD__)
	ti.start_track = track;
	ti.end_track = track;
	ti.start_index = 1;
	ti.end_index = 99;
#endif
#if defined(__linux__)
	ti.cdti_trk0 = track;
	ti.cdti_trk1 = track;
	ti.cdti_ind0 = 0;
	ti.cdti_ind1 = 0;
#endif

#if defined(__FreeBSD__)
	if ( ioctl(cdfile, CDIOCPLAYTRACKS, &ti) == -1 )
#endif
#if defined(__linux__)	
	if ( ioctl(cdfile, CDROMPLAYTRKIND, &ti) == -1 )
#endif
	{
		Com_DPrintf("ioctl cdromplaytrkind failed\n");
		return;
	}

#if defined(__FreeBSD__)
	if ( ioctl(cdfile, CDIOCRESUME) == -1 )
#endif
#if defined(__linux__)
	if ( ioctl(cdfile, CDROMRESUME) == -1 )
#endif	
		Com_DPrintf("ioctl cdromresume failed\n");

	playLooping = looping;
	playTrack = track;
	playing = true;

	if (cd_volume->value == 0.0)
		CDAudio_Pause ();
}

void CDAudio_Play(int track, qboolean looping)
{
	// set a loop counter so that this track will change to the
	// looptrack later
	loopcounter = 0;
	CDAudio_Play2(track, looping);
}


void CDAudio_Stop(void)
{
	if (cdfile == -1 || !enabled)
		return;
	
	if (!playing)
		return;

#if defined(__FreeBSD__)
	if ( ioctl(cdfile, CDIOCSTOP) == -1 )
		Com_DPrintf("ioctl cdiocstop failed (%d)\n", errno);
#endif
#if defined(__linux__)
	if ( ioctl(cdfile, CDROMSTOP) == -1 )
		Com_DPrintf("ioctl cdromstop failed (%d)\n", errno);
#endif

	wasPlaying = false;
	playing = false;
}

void CDAudio_Pause(void)
{
	if (cdfile == -1 || !enabled)
		return;

	if (!playing)
		return;

#if defined(__FreeBSD__)
	if ( ioctl(cdfile, CDIOCPAUSE) == -1 ) 
		Com_DPrintf("ioctl cdiocpause failed\n");
#endif
#if defined(__linux__)
	if ( ioctl(cdfile, CDROMPAUSE) == -1 ) 
		Com_DPrintf("ioctl cdrompause failed\n");
#endif

	wasPlaying = playing;
	playing = false;
}


void CDAudio_Resume(void)
{
	if (cdfile == -1 || !enabled)
		return;
	
	if (!cdValid)
		return;

	if (!wasPlaying)
		return;
	
#if defined(__FreeBSD__)
	if ( ioctl(cdfile, CDIOCRESUME) == -1 ) 
		Com_DPrintf("ioctl cdiocresume failed\n");
#endif
#if defined(__linux__)
	if ( ioctl(cdfile, CDROMRESUME) == -1 ) 
		Com_DPrintf("ioctl cdromresume failed\n");
#endif
	playing = true;
}

static void CD_f (void)
{
	char	*command;
	int		ret;
	int		n;

	if (Cmd_Argc() < 2)
		return;

	command = Cmd_Argv (1);

	if (Q_strcasecmp(command, "on") == 0)
	{
		enabled = true;
		return;
	}

	if (Q_strcasecmp(command, "off") == 0)
	{
		if (playing)
			CDAudio_Stop();
		enabled = false;
		return;
	}

	if (Q_strcasecmp(command, "reset") == 0)
	{
		enabled = true;
		if (playing)
			CDAudio_Stop();
		for (n = 0; n < 100; n++)
			remap[n] = n;
		CDAudio_GetAudioDiskInfo();
		return;
	}

	if (Q_strcasecmp(command, "remap") == 0)
	{
		ret = Cmd_Argc() - 2;
		if (ret <= 0)
		{
			for (n = 1; n < 100; n++)
				if (remap[n] != n)
					Com_Printf("  %u -> %u\n", n, remap[n]);
			return;
		}
		for (n = 1; n <= ret; n++)
			remap[n] = atoi(Cmd_Argv (n+1));
		return;
	}

	if (Q_strcasecmp(command, "close") == 0)
	{
		CDAudio_CloseDoor();
		return;
	}

	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo();
		if (!cdValid)
		{
			Com_Printf("No CD in player.\n");
			return;
		}
	}

	if (Q_strcasecmp(command, "play") == 0)
	{
		CDAudio_Play2((byte)atoi(Cmd_Argv (2)), false);
		return;
	}

	if (Q_strcasecmp(command, "loop") == 0)
	{
		CDAudio_Play((byte)atoi(Cmd_Argv (2)), true);
		return;
	}

	if (Q_strcasecmp(command, "stop") == 0)
	{
		CDAudio_Stop();
		return;
	}

	if (Q_strcasecmp(command, "pause") == 0)
	{
		CDAudio_Pause();
		return;
	}

	if (Q_strcasecmp(command, "resume") == 0)
	{
		CDAudio_Resume();
		return;
	}

	if (Q_strcasecmp(command, "eject") == 0)
	{
		if (playing)
			CDAudio_Stop();
		CDAudio_Eject();
		cdValid = false;
		return;
	}

	if (Q_strcasecmp(command, "info") == 0)
	{
		Com_Printf("%u tracks\n", maxTrack);
		if (playing)
			Com_Printf("Currently %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		else if (wasPlaying)
			Com_Printf("Paused %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		Com_Printf("Volume is %f\n", cdvolume);
		return;
	}
}

void CDAudio_Update(void)
{
#if defined(__FreeBSD__)
	struct ioc_read_subchannel subchnl;
	struct cd_sub_channel_info data;
#endif
#if defined(__linux__)
	struct cdrom_subchnl subchnl;
#endif
	static time_t lastchk;

	if (cdfile == -1 || !enabled)
		return;

	if (cd_volume && cd_volume->value != cdvolume)
	{
		if (cdvolume)
		{
			Cvar_SetValue ("cd_volume", 0.0);
			cdvolume = cd_volume->value;
			CDAudio_Pause ();
		}
		else
		{
			Cvar_SetValue ("cd_volume", 1.0);
			cdvolume = cd_volume->value;
			CDAudio_Resume ();
		}
	}


	if (playing && lastchk < time(NULL)) {
		lastchk = time(NULL) + 2; //two seconds between chks
#if defined(__FreeBSD__)
		subchnl.address_format = CD_MSF_FORMAT;
		subchnl.data_format = CD_CURRENT_POSITION;
		subchnl.data_len = sizeof(data);
		subchnl.track = playTrack;
		subchnl.data = &data;
		if (ioctl(cdfile, CDIOCREADSUBCHANNEL, &subchnl) == -1 ) {
			Com_DPrintf("ioctl cdiocreadsubchannel failed\n");
			playing = false;
			return;
		}
		if (subchnl.data->header.audio_status != CD_AS_PLAY_IN_PROGRESS &&
			subchnl.data->header.audio_status != CD_AS_PLAY_PAUSED) {
			playing = false;
			if (playLooping)
			{
				// if the track has played the given number of times,
				// go to the ambient track
				if (++loopcounter >= cd_loopcount->value)
					CDAudio_Play2(cd_looptrack->value, true);
				else
					CDAudio_Play2(playTrack, true);
			//	CDAudio_Play(playTrack, true);
			}
		}
#endif
#if defined(__linux__)
		subchnl.cdsc_format = CDROM_MSF;
		if (ioctl(cdfile, CDROMSUBCHNL, &subchnl) == -1 ) {
			Com_DPrintf("ioctl cdromsubchnl failed\n");
			playing = false;
			return;
		}
		if (subchnl.cdsc_audiostatus != CDROM_AUDIO_PLAY &&
			subchnl.cdsc_audiostatus != CDROM_AUDIO_PAUSED) {
			playing = false;
			if (playLooping)
				CDAudio_Play(playTrack, true);
		}
#endif
	}
}

int CDAudio_Init(void)
{
	int i;
	cvar_t			*cv;
	extern uid_t saved_euid;

#ifdef NOTTHIRTYFLIGHTS
	cv = Cvar_Get ("nocdaudio", "0", CVAR_NOSET);
#else
	cv = Cvar_Get ("nocdaudio", "1", CVAR_NOSET);
#endif
	if (cv->value)
		return -1;

	cd_nocd = Cvar_Get ("cd_nocd", "0", CVAR_ARCHIVE );
	Cvar_SetDescription ("cd_nocd", "CD music disable option.  Disables CD music when set to 1.");
	cd_loopcount = Cvar_Get ("cd_loopcount", "10", CVAR_ARCHIVE);	// Knightmare increased, was 4, added archive flag
	Cvar_SetDescription ("cd_loopcount", "Sets number of CD track loops until the ambient CD track is played.");
	cd_looptrack = Cvar_Get ("cd_looptrack", "11", 0);
	Cvar_SetDescription ("cd_looptrack", "Sets the number of the ambient CD track.");
	if ( cd_nocd->value)
		return -1;

	cd_volume = Cvar_Get ("cd_volume", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("cd_volume", "Sets the CD music volume (normalized).  0 = mute, 1.0 = max.");

	cd_dev = Cvar_Get("cd_dev", "/dev/cdrom", CVAR_ARCHIVE);
	Cvar_SetDescription ("cd_dev", "Sets device name for CD playback.");

	seteuid(saved_euid);

	cdfile = open(cd_dev->string, O_RDONLY);

	seteuid(getuid());

	if (cdfile == -1) {
		Com_Printf("CDAudio_Init: open of \"%s\" failed (%i)\n", cd_dev->string, errno);
		cdfile = -1;
		return -1;
	}

	for (i = 0; i < 100; i++)
		remap[i] = i;
	initialized = true;
	enabled = true;

	if (CDAudio_GetAudioDiskInfo())
	{
		Com_Printf("CDAudio_Init: No CD in player.\n");
		cdValid = false;
	}

	Cmd_AddCommand ("cd", CD_f);

	Com_Printf("CD Audio Initialized\n");

	return 0;
}

void CDAudio_Activate (qboolean active)
{
	if (active)
		CDAudio_Resume ();
	else
		CDAudio_Pause ();
}

void CDAudio_Shutdown(void)
{
	if (!initialized)
		return;
	CDAudio_Stop();
	close(cdfile);
	cdfile = -1;
}

/*
===========
CDAudio_Active

===========
*/
qboolean CDAudio_Active (void)
{
	return playing;
}

