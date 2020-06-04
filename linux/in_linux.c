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

// in_linux.c -- linux mouse and joystick code

#include "../client/client.h"

cvar_t	*in_mouse;
cvar_t	*in_joystick;

void IN_Init (void)
{
    in_mouse = Cvar_Get ("in_mouse", "1", CVAR_ARCHIVE);
    in_joystick = Cvar_Get ("in_joystick", "0", CVAR_ARCHIVE);
}

void IN_Shutdown (void)
{
}

void IN_Commands (void)
{
}

void IN_Move (usercmd_t *cmd)
{
}

void IN_Activate (qboolean active)
{
}

