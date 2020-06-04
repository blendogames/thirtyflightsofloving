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

#define MAX_SCALE_SIZE 256

typedef struct
{
	int		s_rate;
	int		s_width;
	int		s_channels;

	int		width;
	int		height;
	int		p2_width;
	int		p2_height;
	byte	*pic;
	byte	*pic_pending;

	// order 1 huffman stuff
	int		*hnodes1;	// [256][256][2];
	int		numhnodes1[256];

	int		h_used[512];
	int		h_count[512];

	byte	*cinematic_file;
	byte	*offset;
	byte	*framestart;

	int		time;
	int		lastupload;
	int		frame;
	byte	rawpalette[768];
	unsigned palette[256];

	int		texnum;

} cinematics_t;

cinematics_t *CIN_OpenCin (char *name);
void CIN_ProcessCins(void);
void CIN_FreeCin (int texnum);

extern cinematics_t	*cin;