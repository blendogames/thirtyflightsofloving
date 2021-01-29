/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (?RTCW SP Source Code?).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following the
terms and conditions of the GNU General Public License which accompanied the RTCW SP
Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms,
you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif
#include "l_memory.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_log.h"

typedef enum {false, true}  qboolean;

//#define PATHSEPERATOR_STR		"\\"

typedef struct replacefunc_s
{
	char *name;
	char *newname;
	char *filename;
	char dec[MAX_TOKEN];            //function declaration
	struct replacefunc_s *next;
} replacefunc_t;

typedef struct tokenList_s
{
	token_t token;
	struct tokenList_s *next;
} tokenList_t;

extern int Q_stricmp (const char *s1, const char *s2);
replacefunc_t *FindFunctionName (char *funcname);
void Error (char *error, ...);
void Com_sprintf (char *dest, size_t size, const char *fmt, ...);
void Q_strncpyz (char *dest, size_t destSize, const char *src);
void Q_strncatz (char *dest, size_t destSize, const char *src);

extern int verbose;
