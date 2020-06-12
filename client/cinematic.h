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


#ifndef __CINEMATIC_H__
#define __CINEMATIC_H__


#define MAX_CINEMATICS	8

#define CIN_SYSTEM		1	// A cinematic handled by the client system
#define CIN_LOOPED		2	// Looped playback
#define CIN_SILENT		4	// Don't play audio
#define CIN_SHADER		8	// Used in a shader by the renderer

typedef int cinHandle_t;

// Will run a frame of the cinematic but will not draw it. Will return
// false if the end of the cinematic has been reached.
qboolean	CIN_RunCinematic (cinHandle_t handle);

// Allows you to resize the animation dynamically
void		CIN_SetExtents (cinHandle_t handle, int x, int y, int width, int height);

// Draws the current frame
void		CIN_DrawCinematic (cinHandle_t handle);

// Returns a handle to the cinematic
cinHandle_t	CIN_PlayCinematic (const char *name, int x, int y, int width, int height, int flags);

// Stops playing the cinematic
void		CIN_StopCinematic (cinHandle_t handle);


#endif	// __CINEMATIC_H__
