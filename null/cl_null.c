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

// cl_null.c -- this file can stub out the entire client system
// for pure dedicated servers

#include "../qcommon/qcommon.h"

void Key_Bind_Null_f(void)
{
}

void CL_Init (void)
{
}

void CL_Drop (void)
{
}

void CL_Shutdown (void)
{
}

void CL_Frame (int msec)
{
}

void Con_Print (char *text)
{
}

void Cmd_ForwardToServer (void)
{
	char *cmd;

	cmd = Cmd_Argv(0);
	Com_Printf ("Unknown command \"%s\"\n", cmd);
}

void SCR_DebugGraph (float value, int color)
{
}

void SCR_BeginLoadingPlaque (void)
{
}

void SCR_EndLoadingPlaque (void)
{
}

void Key_Init (void)
{
	Cmd_AddCommand ("bind", Key_Bind_Null_f);
}

