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

//
// console
//


#define	NUM_CON_TIMES 8	// was 4

#define		MAXCMDLINE	1024	// Max length of console command line
								// increased from 256, fixes buffer overflow if vert res > 2048
								// allows max vert res of 8192 for fullscreen console 

#define		CON_TEXTSIZE	131072	// Knightmare- increased from 32768
typedef struct
{
	qboolean	initialized;

	char	text[CON_TEXTSIZE];
	int		current;		// line where next message will be printed
	int		x;				// offset in current line for next print
	int		display;		// bottom of console displays this line

	int		ormask;			// high bit mask for colored characters

	int 	linewidth;		// characters across screen
	int		totallines;		// total lines in console scrollback
	int		backedit;

	float	cursorspeed;

	int		vislines;

	float	times[NUM_CON_TIMES];	// cls.realtime time the line was generated
								// for transparent notify lines
} console_t;

extern	console_t	con;

extern	qboolean	halfconback;	// whether to draw Q3-style console

void Con_DrawCharacter (int cx, int line, int num);

void Con_CheckResize (void);
void Con_Init (void);
//Knightmare changed
//void Con_DrawConsole (float frac);
void Con_DrawConsole (float frac, qboolean ingame);
void Con_Print (char *txt);
void Con_CenteredPrint (char *text);
void Con_Clear_f (void);
void Con_DrawNotify (void);
void Con_ClearNotify (void);
void Con_ToggleConsole_f (void);
