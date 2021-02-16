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

// ui_utils.c -- misc support functions for the menus

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"
#include <sys/types.h>
#include <sys/stat.h>

/*
=======================================================================

	MISC UTILITY FUNCTIONS

=======================================================================
*/

#if 0
/*
=================
FreeFileList
=================
*/
void FreeFileList (char **list, int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		if (list && list[i])
		{
			free(list[i]);
			list[i] = 0;
		}
	}
	free(list);
}

/*
=================
ItemInList
=================
*/
qboolean ItemInList (char *check, int num, char **list)
{
	int i;
	for (i=0;i<num;i++)
		if (!Q_strcasecmp(check, list[i]))
			return true;
	return false;
}

/*
=================
InsertInList
=================
*/
void InsertInList (char **list, char *insert, int len, int start)
{
	int i;
	if (!list) return;

	for (i=start; i<len; i++)
	{
		if (!list[i])
		{
			list[i] = strdup(insert);
			return;
		}
	}
	list[len] = strdup(insert);
}
#endif

/*
==========================
IsValidImageFilename
==========================
*/
qboolean IsValidImageFilename (char *name)
{
	int		len = (int)strlen(name);

	if (	!strcmp(name+max(len-4,0), ".pcx")
		||	!strcmp(name+max(len-4,0), ".tga")
#ifdef PNG_SUPPORT
		||	!strcmp(name+max(len-4,0), ".png")
#endif	// PNG_SUPPORT
		||  !strcmp(name+max(len-4,0), ".jpg")
		 )
		return true;

	return false;
}
