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

// vid_null.c -- null video loader to aid porting efforts
// this assumes that one of the refs is statically linked to the executable

#include "../client/client.h"

viddef_t	viddef;				// global video state

refexport_t	re;

refexport_t GetRefAPI (refimport_t rimp);

/*
==========================================================================

DIRECT LINK GLUE

==========================================================================
*/

#define	MAXPRINTMSG	8192 // was 4096
void VID_Printf (int print_level, char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr, fmt);
//	vsprintf (msg, fmt, argptr);
	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	if (print_level == PRINT_ALL)
		Com_Printf ("%s", msg);
	else
		Com_DPrintf ("%s", msg);
}

void VID_Error (int err_level, char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr, fmt);
//	vsprintf (msg, fmt, argptr);
	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	Com_Error (err_level, "%s", msg);
}

void VID_NewWindow (int width, int height)
{
	viddef.width = width;
	viddef.height = height;
}

/*
** VID_GetModeInfo
*/
typedef struct vidmode_s
{
    const char *description;
    int         width, height;
    int         mode;
} vidmode_t;

vidmode_t vid_modes[] =
{
#include "../qcommon/vid_modes.h"
};

#define VID_NUM_MODES ( sizeof( vid_modes ) / sizeof( vid_modes[0] ) )

qboolean VID_GetModeInfo( int *width, int *height, int mode )
{
	if ( mode < 0 || mode >= VID_NUM_MODES )
		return false;

	*width  = vid_modes[mode].width;
	*height = vid_modes[mode].height;

	return true;
}


void	VID_Init (void)
{
	refimport_t	ri;

	viddef.width = 320;
	viddef.height = 240;

	ri.Cmd_AddCommand = Cmd_AddCommand;
	ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
	ri.Cmd_Argc = Cmd_Argc;
	ri.Cmd_Argv = Cmd_Argv;
	ri.Cmd_ExecuteText = Cbuf_ExecuteText;
	ri.Con_Printf = VID_Printf;
	ri.Sys_Error = VID_Error;
	ri.FS_LoadFile = FS_LoadFile;
	ri.FS_FreeFile = FS_FreeFile;
	ri.FS_Gamedir = FS_Gamedir;
	ri.Vid_NewWindow = VID_NewWindow;
	ri.Cvar_Get = Cvar_Get;
	ri.Cvar_Set = Cvar_Set;
	ri.Cvar_SetValue = Cvar_SetValue;
	ri.Vid_GetModeInfo = VID_GetModeInfo;
	ri.SetParticlePics = CL_RegisterParticleImages; // Knightmare added

	re = GetRefAPI(ri);

	if (re.api_version != API_VERSION)
		Com_Error (ERR_FATAL, "Re has incompatible api_version");

	// call the init function
	if (re.Init (NULL, NULL) == -1)
		Com_Error (ERR_FATAL, "Couldn't start refresh");
}

void	VID_Shutdown (void)
{
	if (re.Shutdown)
		re.Shutdown ();
}

void	VID_CheckChanges (void)
{
}

void	VID_MenuInit (void)
{
}

void	VID_MenuDraw (void)
{
}

const char *VID_MenuKey( int k)
{
	return NULL;
}
