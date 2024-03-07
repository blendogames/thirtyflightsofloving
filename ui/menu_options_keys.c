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

// menu_options_keys.c -- the key binding menu

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

#ifdef NOTTHIRTYFLIGHTS
#define USE_KEYBIND_CONTROL
#endif

/*
=======================================================================

KEYS MENU

=======================================================================
*/

char *bindnames[][2] =
{
#ifdef NOTTHIRTYFLIGHTS
{"+attack", 		"attack"},
{"+attack2", 		"alternate attack"},
{"+use", 			"activate"},
{"weapprev", 		"prev weapon"},
{"weapnext", 		"next weapon"},
{"+forward", 		"walk forward"},
{"+back", 			"backpedal"},
{"+left", 			"turn left"},
{"+right", 			"turn right"},
{"+speed", 			"run"},
{"+moveleft", 		"step left"},
{"+moveright", 		"step right"},
{"+strafe", 		"sidestep"},
{"+lookup", 		"look up"},
{"+lookdown", 		"look down"},
{"centerview", 		"center view"},
{"+mlook", 			"mouse look"},
{"+klook", 			"keyboard look"},
{"+moveup",			"up / jump"},
{"+movedown",		"down / crouch"},
{"inven",			"inventory"},
{"invuse",			"use item"},
{"invdrop",			"drop item"},
{"invprev",			"prev item"},
{"invnext",			"next item"},
{"cmd help", 		"help computer" }, 
#else
{"+forward", 		"Move Forward"},
{"+back", 			"Move Backward"},
{"+moveleft", 		"Move Left"},
{"+moveright", 		"Move Right"},
{"+moveup",			"Jump"},
{"+movedown",		"Crouch"},
{"+attack", 		"Use Item"},
{"weapprev", 		"Select Previous Item"},
{"weapnext", 		"Select Next Item"},
{"+use", 			"Action"},
{"centerview", 		"Center View"},
{"save quick",		"Quick Save"},
{"load quick",		"Quick Load"},
#endif
{ 0, 0 }
};

#ifndef USE_KEYBIND_CONTROL
int				keys_cursor;
static int		bind_grab;
#endif

static menuframework_s	s_keys_menu;
#ifdef USE_KEYBIND_CONTROL
static menukeybind_s	s_keys_binds[64];
#else
static menuaction_s		s_keys_binds[64];
#endif
static menuaction_s		s_keys_back_action;

//=======================================================================

#ifndef USE_KEYBIND_CONTROL
static void M_UnbindCommand (char *command)
{
	int		j;
	int		l;
	char	*b;

	l = (int)strlen(command);

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		// Knightmare- fix bug with key bound to +attack2 being confused with +attack
		if (!strncmp (b, "+attack2", 8) && strncmp (command, "+attack2", 8))
			continue;
		if (!strncmp (b, command, l) )
			Key_SetBinding (j, "");
	}
}

static void M_FindKeysForCommand (char *command, int *twokeys)
{
	int		count, j, l;
	char	*b;

	twokeys[0] = twokeys[1] = -1;
	l = (int)strlen(command);
	count = 0;

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		// Knightmare- fix bug with key bound to +attack2 being confused with +attack
		if (!strncmp (b, "+attack2", 8) && strncmp (command, "+attack2", 8))
			continue;
		if (!strncmp (b, command, l))
		{
			twokeys[count] = j;
			count++;
			if (count == 2)
				break;
		}
	}
}

static void M_KeysBackCursorDrawFunc (menuaction_s *self) // back action
{
#ifdef NOTTHIRTYFLIGHTS
	char	*cursor;

	cursor = ((int)(Sys_Milliseconds()/250)&1) ? UI_ITEMCURSOR_DEFAULT_PIC : UI_ITEMCURSOR_BLINK_PIC;

	UI_DrawPic (SCREEN_WIDTH*0.5 - 24, s_keys_menu.y + self->generic.y, MENU_FONT_SIZE, MENU_FONT_SIZE, ALIGN_CENTER, false, cursor, 255);
/*	UI_DrawChar (SCREEN_WIDTH*0.5 - 24, s_keys_menu.y + self->generic.y, MENU_FONT_SIZE, ALIGN_CENTER,
					12+((int)(Sys_Milliseconds()/250)&1), 255, 255, 255, 255, false, true);
*/
#else
	SCR_DrawChar (SCREEN_WIDTH*0.5 - 24+ (5*sin(anglemod(cl.time*0.01))),
		s_keys_menu.y + self->generic.y,
		MENU_FONT_SIZE,
		ALIGN_CENTER,
		13,
		FONT_UI,
		255,255,255,255, false, true);
#endif
}

static void M_KeyCursorDrawFunc (menuframework_s *menu)
{
#ifdef NOTTHIRTYFLIGHTS
	char	*cursor;

	if (bind_grab)
		cursor = UI_ITEMCURSOR_KEYBIND_PIC;
	else
		cursor = ((int)(Sys_Milliseconds()/250)&1) ? UI_ITEMCURSOR_DEFAULT_PIC : UI_ITEMCURSOR_BLINK_PIC;

	UI_DrawPic (menu->x, menu->y + menu->cursor * MENU_LINE_SIZE, MENU_FONT_SIZE, MENU_FONT_SIZE, ALIGN_CENTER, false, cursor, 255);
/*	if (bind_grab)
		UI_DrawChar (menu->x, menu->y + menu->cursor * MENU_LINE_SIZE, MENU_FONT_SIZE, ALIGN_CENTER,
						'=', 255, 255, 255, 255, false, true);
	else
		UI_DrawChar (menu->x, menu->y + menu->cursor * MENU_LINE_SIZE, MENU_FONT_SIZE, ALIGN_CENTER,
						12+((int)(Sys_Milliseconds()/250)&1), 255, 255, 255, 255, false, true);
*/
#else
		SCR_DrawChar (menu->x+ (5*sin(anglemod(cl.time*0.01))),
			menu->y + menu->cursor * MENU_LINE_SIZE,
			MENU_FONT_SIZE,
			ALIGN_CENTER,
			13,
			FONT_UI,
			255,255,255,
			255, false, true);
#endif
}

static void M_DrawKeyBindingFunc (void *self)
{
	int keys[2];
	menuaction_s *a = (menuaction_s *) self;

	M_FindKeysForCommand( bindnames[a->generic.localdata[0]][0], keys);
		
	if (keys[0] == -1)
	{
#ifdef NOTTHIRTYFLIGHTS
		UI_DrawString (a->generic.x + a->generic.parent->x + 16,
						a->generic.y + a->generic.parent->y, a->generic.textSize, ALIGN_CENTER, "???", FONT_UI, 255);
#else
		UI_DrawString (a->generic.x + a->generic.parent->x + 16,
						a->generic.y + a->generic.parent->y, a->generic.textSize, ALIGN_CENTER, "^1<NONE>", FONT_UI, 255);
#endif
	}
	else
	{
		int x;
		const char *name;
		int alpha;
#ifdef NOTTHIRTYFLIGHTS
		alpha=255;
#else
		if (ui_mousecursor.menuitem == a)
			alpha=255;
		else
			alpha=160;
#endif

		name = Key_KeynumToString (keys[0]);

		UI_DrawString (a->generic.x + a->generic.parent->x + 16,
						a->generic.y + a->generic.parent->y, a->generic.textSize, ALIGN_CENTER, name, FONT_UI, alpha);

		x = (int)strlen(name) * MENU_FONT_SIZE;

		if (keys[1] != -1)
		{
			UI_DrawString (a->generic.x + a->generic.parent->x + MENU_FONT_SIZE*3 + x,
							a->generic.y + a->generic.parent->y, a->generic.textSize, ALIGN_CENTER, "or", FONT_UI, alpha);
			UI_DrawString (a->generic.x + a->generic.parent->x + MENU_FONT_SIZE*6 + x,
							a->generic.y + a->generic.parent->y, a->generic.textSize, ALIGN_CENTER, Key_KeynumToString(keys[1]), FONT_UI, alpha);
		}
	}
}

static void M_KeyBindingFunc (void *self)
{
	menuaction_s *a = ( menuaction_s * ) self;
	int keys[2];

	M_FindKeysForCommand (bindnames[a->generic.localdata[0]][0], keys);

	if (keys[1] != -1)
		M_UnbindCommand (bindnames[a->generic.localdata[0]][0]);

	bind_grab = true;

#ifdef NOTTHIRTYFLIGHTS
	UI_SetMenuStatusBar (&s_keys_menu, "press a key or button for this action");
#else
	UI_SetMenuStatusBar (&s_keys_menu, "Press a key or button.");
#endif
}

void M_AddBindOption (int i, char *list[][2])
{
	s_keys_binds[i].generic.type	= MTYPE_ACTION;
	s_keys_binds[i].generic.textSize = MENU_FONT_SIZE;
	s_keys_binds[i].generic.flags  = QMF_GRAYED;
	s_keys_binds[i].generic.x		= 0;
	s_keys_binds[i].generic.y		= i*MENU_LINE_SIZE;
	s_keys_binds[i].generic.ownerdraw = M_DrawKeyBindingFunc;
	s_keys_binds[i].generic.localdata[0] = i;
	s_keys_binds[i].generic.name	= list[s_keys_binds[i].generic.localdata[0]][1];
	s_keys_binds[i].generic.callback = M_KeyBindingFunc;

	if (strstr ("MENUSPACE", list[i][0]))
		s_keys_binds[i].generic.type	= MTYPE_SEPARATOR;
}
#endif	// USE_KEYBIND_CONTROL

//=======================================================================

static void Menu_Keys_Init (void)
{
	int		BINDS_MAX;
	int		i = 0, x = 0, y = 0;

	s_keys_menu.x = SCREEN_WIDTH*0.5;
	s_keys_menu.y = SCREEN_HEIGHT*0.5 - 72;
	s_keys_menu.nitems = 0;
#ifndef USE_KEYBIND_CONTROL
	s_keys_menu.cursordraw = M_KeyCursorDrawFunc;
#endif

	BINDS_MAX = listSize(bindnames);
	for (i=0;i<BINDS_MAX;i++)
#ifdef USE_KEYBIND_CONTROL
	for (i=0; i<BINDS_MAX; i++)
	{
		s_keys_binds[i].generic.type			= MTYPE_KEYBIND;
		s_keys_binds[i].generic.flags			= QMF_ALTCOLOR;
		s_keys_binds[i].generic.x				= x;
		s_keys_binds[i].generic.y				= y + i*MENU_LINE_SIZE;
		s_keys_binds[i].generic.name			= bindnames[i][1];
		s_keys_binds[i].generic.statusbar		= "enter or mouse1 to change, backspace or del to clear";
		s_keys_binds[i].commandName				= bindnames[i][0];
		s_keys_binds[i].enter_statusbar			= "press a key or button for this action";
	}
#else
		M_AddBindOption (i, bindnames);
#endif

	s_keys_back_action.generic.type		= MTYPE_ACTION;
	s_keys_back_action.generic.textSize	= MENU_FONT_SIZE;
	s_keys_back_action.generic.flags	= QMF_LEFT_JUSTIFY;
	s_keys_back_action.generic.x		= x;
	s_keys_back_action.generic.y		= y + (BINDS_MAX+2)*MENU_LINE_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_keys_back_action.generic.name		= "Back";
#else
	s_keys_back_action.generic.name		= "Done";
#endif
	s_keys_back_action.generic.callback	= UI_BackMenu;
#ifndef USE_KEYBIND_CONTROL
	s_keys_back_action.generic.cursordraw = M_KeysBackCursorDrawFunc;
#endif

	for (i=0;i<BINDS_MAX;i++)
		UI_AddMenuItem (&s_keys_menu, (void *) &s_keys_binds[i]);
	UI_AddMenuItem (&s_keys_menu, (void *) &s_keys_back_action);

#ifndef USE_KEYBIND_CONTROL
#ifdef NOTTHIRTYFLIGHTS
	UI_SetMenuStatusBar (&s_keys_menu, "enter or mouse1 to change, backspace to clear");
#else
	UI_SetMenuStatusBar (&s_keys_menu, "Press ENTER or LEFT CLICK to change the key. Press BACKSPACE to clear.");
#endif
#endif
	// Don't center it- it's too large
//	UI_CenterMenu (&s_keys_menu);
}

static void Menu_Keys_Draw (void)
{
#ifdef NOTTHIRTYFLIGHTS
	UI_DrawBanner ("m_banner_customize"); // Knightmare added
#else
	UI_DrawBanner ("m_banner_options");
#endif

	UI_AdjustMenuCursor (&s_keys_menu, 1);
	UI_DrawMenu (&s_keys_menu);
}

static const char *Menu_Keys_Key (int key)
{
#ifdef USE_KEYBIND_CONTROL
	return UI_DefaultMenuKey (&s_keys_menu, key);
#else
	menuaction_s *item = (menuaction_s *) UI_ItemAtMenuCursor( &s_keys_menu );

	// pressing mouse1 to pick a new bind wont force bind/unbind itself - spaz
	if ( bind_grab && !(ui_mousecursor.buttonused[MOUSEBUTTON1]&&key==K_MOUSE1))
	{	
		if ( key != K_ESCAPE && key != '`' )
		{
			char cmd[1024];

			Com_sprintf (cmd, sizeof(cmd), "bind \"%s\" \"%s\"\n", Key_KeynumToString(key), bindnames[item->generic.localdata[0]][0]);
			Cbuf_InsertText (cmd);
		}
		
		//  Knightmare- added Psychospaz's mouse support
		//dont let selecting with mouse buttons screw everything up
		UI_RefreshCursorButtons();
		if (key == K_MOUSE1)
			ui_mousecursor.buttonclicks[MOUSEBUTTON1] = -1;

#ifdef NOTTHIRTYFLIGHTS
		UI_SetMenuStatusBar (&s_keys_menu, "enter to change, backspace to clear");
#else
		UI_SetMenuStatusBar (&s_keys_menu, "Press ENTER or LEFT CLICK to change the key. Press BACKSPACE to clear.");
#endif
		bind_grab = false;
		return ui_menu_out_sound;
	}

	switch (key)
	{
	case K_KP_ENTER:
	case K_ENTER:
		if (item == &s_keys_back_action) { // back action hack
			UI_BackMenu(item); return NULL; }
		M_KeyBindingFunc (item);
		return ui_menu_in_sound;
	case K_BACKSPACE:		// delete bindings
	case K_DEL:				// delete bindings
	case K_KP_DEL:
		M_UnbindCommand (bindnames[item->generic.localdata[0]][0]);
		return ui_menu_out_sound;
	default:
		return UI_DefaultMenuKey (&s_keys_menu, key);
	}
#endif
}

void Menu_Keys_f (void)
{
	Menu_Keys_Init ();
	UI_PushMenu (Menu_Keys_Draw, Menu_Keys_Key);
}
