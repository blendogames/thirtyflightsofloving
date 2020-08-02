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

/*
	r_cin.c

	CIN File Decoder for cinematic textures

	Adapted from executable source by Robert 'Heffo' Heffernan
	1/1/2002, 1:34pm AEDST
*/

#include "r_local.h"
#include "r_cin.h"

typedef struct
{
	byte	*data;
	int		count;
} cblock_t;

void R_CIN_StopCinematic (void);
int	SmallestNode1 (int numhnodes);
void Huff1TableInit (void);
cblock_t Huff1Decompress (cblock_t in);
byte *R_CIN_ReadNextFrame (void);
void R_CIN_RunCinematic (void);
qboolean R_CIN_DrawCinematic (void);
void R_CIN_PlayCinematic (char *arg);
void R_CIN_StartCinematic (void);

cinematics_t	*cin;

//=============================================================

int GetInteger (byte *data)
{
	int b0, b1, b2, b3;

	b0 = (data[0] & 0xFF);
	b1 = (data[1] & 0xFF) << 8;
	b2 = (data[2] & 0xFF) << 16;
	b3 = (data[3] & 0xFF) << 24;

	return b0 + b1 + b2 + b3;
}

/*
==================
R_CIN_StopCinematic
==================
*/
void R_CIN_StopCinematic (void)
{
	cin->time = 0;	// done
	if (cin->pic)
	{
		free(cin->pic);
		cin->pic = NULL;
	}
	if (cin->pic_pending)
	{
		free(cin->pic_pending);
		cin->pic_pending = NULL;
	}
	if (cin->cinematic_file)
	{
		FS_FreeFile(cin->cinematic_file);
		cin->cinematic_file = NULL;
		cin->offset = NULL;
	}
	if (cin->hnodes1)
	{
		free(cin->hnodes1);
		cin->hnodes1 = NULL;
	}
}

//==========================================================================

/*
==================
SmallestNode1
==================
*/
int	SmallestNode1 (int numhnodes)
{
	int		i;
	int		best, bestnode;

	best = 99999999;
	bestnode = -1;
	for (i=0 ; i<numhnodes ; i++)
	{
		if (cin->h_used[i])
			continue;
		if (!cin->h_count[i])
			continue;
		if (cin->h_count[i] < best)
		{
			best = cin->h_count[i];
			bestnode = i;
		}
	}

	if (bestnode == -1)
		return -1;

	cin->h_used[bestnode] = true;
	return bestnode;
}


/*
==================
Huff1TableInit

Reads the 64k counts table and initializes the node trees
==================
*/
void Huff1TableInit (void)
{
	int		prev;
	int		j;
	int		*node, *nodebase;
	byte	counts[256];
	int		numhnodes;

	cin->hnodes1 = malloc(256*256*2*4);
	memset (cin->hnodes1, 0, 256*256*2*4);

	for (prev=0 ; prev<256 ; prev++)
	{
		memset (cin->h_count,0,sizeof(cin->h_count));
		memset (cin->h_used,0,sizeof(cin->h_used));

		// read a row of counts
		memcpy(counts, cin->offset, sizeof(counts)); cin->offset += sizeof(counts);
		for (j=0 ; j<256 ; j++)
			cin->h_count[j] = counts[j];

		// build the nodes
		numhnodes = 256;
		nodebase = cin->hnodes1 + prev*256*2;

		while (numhnodes != 511)
		{
			node = nodebase + (numhnodes-256)*2;

			// pick two lowest counts
			node[0] = SmallestNode1 (numhnodes);
			if (node[0] == -1)
				break;	// no more

			node[1] = SmallestNode1 (numhnodes);
			if (node[1] == -1)
				break;

			cin->h_count[numhnodes] = cin->h_count[node[0]] + cin->h_count[node[1]];
			numhnodes++;
		}

		cin->numhnodes1[prev] = numhnodes-1;
	}

	cin->framestart = cin->offset;
}

/*
==================
Huff1Decompress
==================
*/
cblock_t Huff1Decompress (cblock_t in)
{
	byte		*input;
	byte		*out_p;
	int			nodenum;
	int			count;
	cblock_t	out;
	int			inbyte;
	int			*hnodes, *hnodesbase;
//int		i;

	// get decompressed count
	count = GetInteger(in.data);
	input = in.data + 4;
	out_p = out.data = malloc(count);

	// read bits

	hnodesbase = cin->hnodes1 - 256*2;	// nodes 0-255 aren't stored

	hnodes = hnodesbase;
	nodenum = cin->numhnodes1[0];
	while (count)
	{
		inbyte = *input++;
		//-----------
		if (nodenum < 256)
		{
			hnodes = hnodesbase + (nodenum<<9);
			*out_p++ = nodenum;
			if (!--count)
				break;
			nodenum = cin->numhnodes1[nodenum];
		}
		nodenum = hnodes[nodenum*2 + (inbyte&1)];
		inbyte >>=1;
		//-----------
		if (nodenum < 256)
		{
			hnodes = hnodesbase + (nodenum<<9);
			*out_p++ = nodenum;
			if (!--count)
				break;
			nodenum = cin->numhnodes1[nodenum];
		}
		nodenum = hnodes[nodenum*2 + (inbyte&1)];
		inbyte >>=1;
		//-----------
		if (nodenum < 256)
		{
			hnodes = hnodesbase + (nodenum<<9);
			*out_p++ = nodenum;
			if (!--count)
				break;
			nodenum = cin->numhnodes1[nodenum];
		}
		nodenum = hnodes[nodenum*2 + (inbyte&1)];
		inbyte >>=1;
		//-----------
		if (nodenum < 256)
		{
			hnodes = hnodesbase + (nodenum<<9);
			*out_p++ = nodenum;
			if (!--count)
				break;
			nodenum = cin->numhnodes1[nodenum];
		}
		nodenum = hnodes[nodenum*2 + (inbyte&1)];
		inbyte >>=1;
		//-----------
		if (nodenum < 256)
		{
			hnodes = hnodesbase + (nodenum<<9);
			*out_p++ = nodenum;
			if (!--count)
				break;
			nodenum = cin->numhnodes1[nodenum];
		}
		nodenum = hnodes[nodenum*2 + (inbyte&1)];
		inbyte >>=1;
		//-----------
		if (nodenum < 256)
		{
			hnodes = hnodesbase + (nodenum<<9);
			*out_p++ = nodenum;
			if (!--count)
				break;
			nodenum = cin->numhnodes1[nodenum];
		}
		nodenum = hnodes[nodenum*2 + (inbyte&1)];
		inbyte >>=1;
		//-----------
		if (nodenum < 256)
		{
			hnodes = hnodesbase + (nodenum<<9);
			*out_p++ = nodenum;
			if (!--count)
				break;
			nodenum = cin->numhnodes1[nodenum];
		}
		nodenum = hnodes[nodenum*2 + (inbyte&1)];
		inbyte >>=1;
		//-----------
		if (nodenum < 256)
		{
			hnodes = hnodesbase + (nodenum<<9);
			*out_p++ = nodenum;
			if (!--count)
				break;
			nodenum = cin->numhnodes1[nodenum];
		}
		nodenum = hnodes[nodenum*2 + (inbyte&1)];
		inbyte >>=1;
	}

	/*if (input - in.data != in.count && input - in.data != in.count+1)
	{
		Com_Printf ("Decompression overread by %i", (input - in.data) - in.count);
	}*/
	out.count = out_p - out.data;

	return out;
}

/*
==================
R_CIN_ReadNextFrame
==================
*/
byte *R_CIN_ReadNextFrame (void)
{
	int		command;
	byte	compressed[0x20000];
	int		size;
	byte	*pic;
	cblock_t	in, huf1;
	int		start, end, count, i;
	byte	*rp = ( byte * ) cin->palette;

	// read the next frame
	command = GetInteger(cin->offset); cin->offset += 4;
	command = LittleLong(command);

	if (command == 2)
		return NULL;	// last frame marker

	if (command == 1)
	{	// read palette
		memcpy(cin->rawpalette, cin->offset, sizeof(cin->rawpalette));
		cin->offset += sizeof(cin->rawpalette);

		for ( i = 0; i < 256; i++ )
		{
			rp[i*4+0] = cin->rawpalette[i*3+0];
			rp[i*4+1] = cin->rawpalette[i*3+1];
			rp[i*4+2] = cin->rawpalette[i*3+2];
			rp[i*4+3] = 0xff;
		}
	}

	// decompress the next frame
	size = GetInteger(cin->offset); cin->offset += 4;
	if (size > sizeof(compressed) || size < 1)
		VID_Error (ERR_DROP, "Bad compressed frame size");
	memcpy(compressed, cin->offset, size);
	cin->offset += size;

	// read sound
	start = cin->frame*cin->s_rate/14;
	end = (cin->frame+1)*cin->s_rate/14;
	count = end - start;

	cin->offset += (count*cin->s_width*cin->s_channels);

	in.data = compressed;
	in.count = size;

	huf1 = Huff1Decompress (in);

	pic = huf1.data;

	cin->frame++;

	return pic;
}


/*
==================
R_CIN_RunCinematic
==================
*/
void R_CIN_RunCinematic (void)
{
	int		frame;

	if(!cin && !cin->cinematic_file)
		return;

	frame = (Sys_Milliseconds() - cin->time)*14.0/1000;
	if (frame <= cin->frame)
		return;

	if (frame > cin->frame+1)
	{
		//Com_Printf ("Dropped frame: %i > %i\n", frame, cin->frame+1);
		cin->time = Sys_Milliseconds() - cin->frame*1000/14;
	}
	if (cin->pic)
		free(cin->pic);
	cin->pic = cin->pic_pending;
	cin->pic_pending = NULL;
	cin->pic_pending = R_CIN_ReadNextFrame ();
	if (!cin->pic_pending)
	{
		R_CIN_StartCinematic ();
		cin->pic_pending = R_CIN_ReadNextFrame ();
	}

	R_CIN_DrawCinematic();
}

/*
==================
R_CIN_DrawCinematic

Returns true if a cinematic is active, meaning the view rendering
should be skipped
==================
*/
qboolean R_CIN_DrawCinematic (void)
{
	int				i, j;
	byte			*inrow;
	unsigned		frac, fracstep;
	static unsigned image[MAX_SCALE_SIZE*MAX_SCALE_SIZE];
	unsigned		*out;

	if (cin->time <= 0)
	{
		return false;
	}

	if (!cin->pic)
	{
		return true;
	}

	GL_Bind (cin->texnum);

	out = image;
	fracstep = cin->width*0x10000/cin->p2_width;
	for (i=0 ; i<cin->p2_height ; i++, out += cin->p2_width)
	{
		inrow = cin->pic + cin->width*(i*cin->height/cin->p2_height);
		frac = fracstep >> 1;
		for (j=0 ; j<cin->p2_width ; j+=4)
		{
			out[j] = cin->palette[inrow[frac>>16]];
			frac += fracstep;
			out[j+1] = cin->palette[inrow[frac>>16]];
			frac += fracstep;
			out[j+2] = cin->palette[inrow[frac>>16]];
			frac += fracstep;
			out[j+3] = cin->palette[inrow[frac>>16]];
			frac += fracstep;
		}
	}

	qglTexImage2D (GL_TEXTURE_2D, 0, gl_tex_solid_format, cin->p2_width, cin->p2_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

/*
==================
R_CIN_PlayCinematic
==================
*/
void R_CIN_PlayCinematic (char *arg)
{
 int  size;
 
 cin->frame = 0;
 
 size = FS_LoadFile(arg, (void **)&cin->cinematic_file);
 if(size == -1)
 {
  VID_Error (ERR_DROP, "Cinematic %s not found.\n", arg);
  cin->time = 0; // done
  return;
 }
 
 cin->offset = cin->cinematic_file;
 
 cin->width = GetInteger(cin->offset); cin->offset += 4;
 cin->height = GetInteger(cin->offset); cin->offset += 4;
 cin->s_rate = GetInteger(cin->offset); cin->offset += 4;
 cin->s_width = GetInteger(cin->offset); cin->offset += 4;
 cin->s_channels = GetInteger(cin->offset); cin->offset += 4;
 
 for( cin->p2_width = 2 ; cin->p2_width <= cin->width ; cin->p2_width <<= 1 );
 cin->p2_width >>= 1;
 
 if(cin->p2_width >= MAX_SCALE_SIZE)
 {
  cin->p2_width = MAX_SCALE_SIZE;
 }
 
 for( cin->p2_height = 2 ; cin->p2_height <= cin->height ; cin->p2_height <<= 1 );
 cin->p2_height >>= 1;
 
 if(cin->p2_height >= MAX_SCALE_SIZE)
 {
  cin->p2_height = MAX_SCALE_SIZE;
 }
 
 Huff1TableInit ();
 
 cin->frame = 0;
 cin->pic = R_CIN_ReadNextFrame ();
 cin->time = Sys_Milliseconds();
}

/*
==================
R_CIN_StartCinematic
==================
*/
void R_CIN_StartCinematic (void)
{
	cin->offset = cin->framestart;

	cin->frame = 0;
	cin->time = Sys_Milliseconds();

	cin->pic = R_CIN_ReadNextFrame ();
}


/*
==================
CIN_OpenCin
==================
*/
cinematics_t cinpool[8];
cinematics_t *CIN_OpenCin (char *name)
{
	int i;

	for(i=0; i<8; i++)
	{
		if(cinpool[i].texnum)
			continue;

		cin = &cinpool[i];
		R_CIN_PlayCinematic(name);

		return &cinpool[i];
	}

	return NULL;
}

void CIN_ProcessCins (void)
{
	int i;

	for(i=0; i<8; i++)
	{
		if(cinpool[i].texnum)
		{
			cin = &cinpool[i];

			R_CIN_RunCinematic();
		}
	}
}

void CIN_FreeCin (int texnum)
{
	int i;

	for(i=0; i<8; i++)
	{
		if(cinpool[i].texnum == texnum)
		{
			cin = &cinpool[i];
			R_CIN_StopCinematic();
			cinpool[i].texnum = 0;

			return;
		}
	}
}
