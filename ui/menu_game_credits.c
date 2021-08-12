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

// menu_game_credits.c -- the credits scroll

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

/*
=============================================================================

CREDITS MENU

=============================================================================
*/

static int credits_start_time;
// Knigthtmare added- allow credits to scroll past top of screen
static int credits_start_line;
static const char **credits;
static char *creditsIndex[256];
//static char *creditsBuffer;

static const char *ui_idcredits[] =
{
#include "menu_credits_id.h"
};

static const char *ui_xatrixcredits[] =
{
#include "menu_credits_xatrix.h"
};

static const char *ui_roguecredits[] =
{
#include "menu_credits_rogue.h"
};


int stringLengthExtra (const char *string);
void Menu_Credits_Draw (void)
{
	float		alpha, time = (cls.realtime - credits_start_time) * 0.05;
	int			i, y, x, len, stringoffset;
	qboolean	bold;

	if ((SCREEN_HEIGHT - ((cls.realtime - credits_start_time)/40.0F)
		+ credits_start_line * MENU_LINE_SIZE) < 0)
	{
		credits_start_line++;
		if (!credits[credits_start_line])
		{
			credits_start_line = 0;
			credits_start_time = cls.realtime;
		}
	}

	//
	// draw the credits
	//
	for (i=credits_start_line, y=SCREEN_HEIGHT - ((cls.realtime - credits_start_time)/40.0F) + credits_start_line * MENU_LINE_SIZE;
		credits[i] && y < SCREEN_HEIGHT; y += MENU_LINE_SIZE, i++)
	{
		stringoffset = 0;
		bold = false;

		if (y <= -MENU_FONT_SIZE)
			continue;
		if (y > SCREEN_HEIGHT)
			continue;

		if (credits[i][0] == '+')
		{
			bold = true;
			stringoffset = 1;
		}
		else
		{
			bold = false;
			stringoffset = 0;
		}

		if (y > SCREEN_HEIGHT*(7.0/8.0))
		{
			float y_test, h_test;
			y_test = y - SCREEN_HEIGHT*(7.0/8.0);
			h_test = SCREEN_HEIGHT/8;

			alpha = 1 - (y_test/h_test);

			alpha = max(min(alpha, 1), 0);
		}
		else if (y < SCREEN_HEIGHT/8)
		{
			float y_test, h_test;
			y_test = y;
			h_test = SCREEN_HEIGHT/8;

			alpha = y_test/h_test;

			alpha = max(min(alpha, 1), 0);
		}
		else
			alpha = 1;

		len = (int)strlen(credits[i]) - stringLengthExtra(credits[i]);

		x = ( SCREEN_WIDTH - len * MENU_FONT_SIZE - stringoffset * MENU_FONT_SIZE ) / 2
			+ stringoffset * MENU_FONT_SIZE;
		UI_DrawString (x, y, MENU_FONT_SIZE, credits[i], alpha*255);
	}
}


const char *Menu_Credits_Key (int key)
{
	char *sound = NULL;

	switch (key)
	{
	case K_ESCAPE:
//	case K_MOUSE2:	
		if (creditsBuffer)
			FS_FreeFile (creditsBuffer);
		UI_PopMenu ();
		sound = menu_out_sound;
		break;
	default:
		break;
	}

	return sound;
}


void Menu_Credits_f (void)
{
	int		n;
	int		count;
	char	*p;

	creditsBuffer = NULL;
	count = FS_LoadFile ("credits", &creditsBuffer);
	if (count != -1)
	{
		p = creditsBuffer;
		for (n = 0; n < 255; n++)
		{
			creditsIndex[n] = p;
			while (*p != '\r' && *p != '\n')
			{
				p++;
				if (--count == 0)
					break;
			}
			if (*p == '\r')
			{
				*p++ = 0;
				if (--count == 0)
					break;
			}
			*p++ = 0;
			if (--count == 0)
				break;
		}
		creditsIndex[++n] = 0;
		credits = creditsIndex;
	}
	else
	{	
		if (FS_ModType("xatrix"))			// Xatrix
			credits = ui_xatrixcredits;
		else if (FS_ModType("rogue"))		// Rogue
			credits = ui_roguecredits;
		else
			credits = ui_idcredits;	
	}

	credits_start_time = cls.realtime;
	credits_start_line = 0; // allow credits to scroll past top of screen
	UI_PushMenu (Menu_Credits_Draw, Menu_Credits_Key);
}
