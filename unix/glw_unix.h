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

#if !defined(__linux__) && !defined(__FreeBSD__) && !defined(__APPLE__)
#error You shouldn't be including this file on non-Linux platforms
#endif
#endif

#ifndef __GLW_UNIX_H__
#define __GLW_UNIX_H__

typedef struct
{
	SDL_Window *glWindow;
	SDL_GLContext glContext;

	FILE *log_fp;
} glwstate_t;

extern glwstate_t glw_state;

#endif	// __GLW_UNIX_H__
