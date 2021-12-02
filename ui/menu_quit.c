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

// menu_quit.c -- the quit menu

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

//#define QUITMENU_NOKEY

/*
=======================================================================

QUIT MENU

=======================================================================
*/

#ifdef QUITMENU_NOKEY

static menuframework_s	s_quit_menu;
static menuseparator_s	s_quit_header;
static menuaction_s		s_quit_yes_action;
static menuaction_s		s_quit_no_action;

static void QuitYesFunc (void *unused)
{
	cls.key_dest = key_console;
	CL_Quit_f ();
}

void Menu_Quit_Init (void)
{
	s_quit_menu.x = SCREEN_WIDTH*0.5 - 24;
	s_quit_menu.y = SCREEN_HEIGHT*0.5 - 58;
	s_quit_menu.nitems = 0;

	s_quit_header.generic.type		= MTYPE_SEPARATOR;
	s_quit_header.generic.textSize	= MENU_FONT_SIZE;
	s_quit_header.generic.name		= "Quit game?";
	s_quit_header.generic.x			= MENU_FONT_SIZE*0.7 * (int)strlen(s_quit_header.generic.name);
	s_quit_header.generic.y			= 20;

	s_quit_yes_action.generic.type			= MTYPE_ACTION;
	s_quit_yes_action.generic.textSize		= MENU_FONT_SIZE;
	s_quit_yes_action.generic.flags			= QMF_LEFT_JUSTIFY;
	s_quit_yes_action.generic.x				= MENU_FONT_SIZE*3;
	s_quit_yes_action.generic.y				= 60;
	s_quit_yes_action.generic.name			= "yes";
	s_quit_yes_action.generic.callback		= QuitYesFunc;
	s_quit_yes_action.generic.cursor_offset	= -MENU_FONT_SIZE;

	s_quit_no_action.generic.type			= MTYPE_ACTION;
	s_quit_no_action.generic.textSize		= MENU_FONT_SIZE;
	s_quit_no_action.generic.flags			= QMF_LEFT_JUSTIFY;
	s_quit_no_action.generic.x				= MENU_FONT_SIZE*3;
	s_quit_no_action.generic.y				= 80;
	s_quit_no_action.generic.name			= "no";
	s_quit_no_action.generic.callback		= UI_BackMenu;
	s_quit_no_action.generic.cursor_offset	= -MENU_FONT_SIZE;

	UI_AddMenuItem (&s_quit_menu, (void *) &s_quit_header);
	UI_AddMenuItem (&s_quit_menu, (void *) &s_quit_yes_action);
	UI_AddMenuItem (&s_quit_menu, (void *) &s_quit_no_action);
}

#endif // QUITMENU_NOKEY


const char *Menu_Quit_Key (int key)
{
#ifdef QUITMENU_NOKEY
	return UI_DefaultMenuKey (&s_quit_menu, key);
#else // QUITMENU_NOKEY
	switch (key)
	{
#ifndef NOTTHIRTYFLIGHTS
	case K_BACKSPACE:
#endif
	case K_ESCAPE:
	case 'n':
	case 'N':
#ifndef NOTTHIRTYFLIGHTS
		S_StartLocalSound ("world/cheer.wav");
#endif
		UI_PopMenu ();
		break;

	case 'Y':
	case 'y':
		cls.key_dest = key_console;
		CL_Quit_f ();
		break;

	default:
		break;
	}
	return NULL;
#endif // QUITMENU_NOKEY
}


void Menu_Quit_Draw (void)
{
#ifdef QUITMENU_NOKEY
	UI_AdjustMenuCursor (&s_quit_menu, 1);
	UI_DrawMenu (&s_quit_menu);
#else // QUITMENU_NOKEY
	int		w, h;

	R_DrawGetPicSize (&w, &h, "quit");
#ifdef NOTTHIRTYFLIGHTS
	UI_DrawPic (SCREEN_WIDTH/2-w/2, SCREEN_HEIGHT/2-h/2, w, h, ALIGN_CENTER, false, "quit", 1.0);
#else
	w *= 0.6;
	h *= 0.6;

	SCR_DrawPic (
		SCREEN_WIDTH/2-w/2  + (5*sin(anglemod(cl.time*0.0025))),
		SCREEN_HEIGHT/2-h/2 - 20  +  (3*sin(anglemod(cl.time*0.005))),
		w, h, ALIGN_CENTER, false, "quit", 1.0);



	SCR_DrawPic (
		SCREEN_WIDTH/2-w/2   + (9*sin(anglemod(cl.time*0.0025))),
		SCREEN_HEIGHT/2-h/2 + 20  +  (6*sin(anglemod(cl.time*0.005))),
		w, h, ALIGN_CENTER, false, "quit2", 1.0);
#endif
#endif // QUITMENU_NOKEY
}


void Menu_Quit_f (void)
{
#ifdef QUITMENU_NOKEY
	Menu_Quit_Init();
#endif
	UI_PushMenu (Menu_Quit_Draw, Menu_Quit_Key);
}
