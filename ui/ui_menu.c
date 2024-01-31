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

// ui_menu.c -- menu support/handling functions

#include "../client/client.h"
#include "ui_local.h"

menulayer_t	ui_layers[MAX_MENU_DEPTH];
int			ui_menudepth;

void	(*m_drawfunc) (void);
const char *(*m_keyfunc) (int key);

#ifndef NOTTHIRTYFLIGHTS
extern char *menu_click;
#endif

/*
=======================================================================

MENU HANDLING

=======================================================================
*/

/*
=================
UI_AddButton
From Q2max
=================
*/
void UI_AddButton (buttonmenuobject_t *thisObj, int index, float x, float y, float w, float h)
{
	float x1, y1, w1, h1;

	x1 = x;	y1 = y;	w1 = w;	h1 = h;
	SCR_ScaleCoords (&x1, &y1, &w1, &h1, ALIGN_CENTER);
	thisObj->min[0] = x1;	thisObj->max[0] = x1 + w1;
	thisObj->min[1] = y1;	thisObj->max[1] = y1 + h1;
	thisObj->index = index;
}


/*
=============
UI_AddMainButton
From Q2max
=============
*/
void UI_AddMainButton (mainmenuobject_t *thisObj, int index, int x, int y, char *name)
{
	int		w, h;
	float	x1, y1, w1, h1;

	R_DrawGetPicSize (&w, &h, name);
	
	x1 = x; y1 = y; w1 = w; h1 = h;
	SCR_ScaleCoords (&x1, &y1, &w1, &h1, ALIGN_CENTER);
	thisObj->min[0] = x1;	thisObj->max[0] = x1 + w1;
	thisObj->min[1] = y1;	thisObj->max[1] = y1 + h1;

	switch (index)
	{
	case 0:
		thisObj->OpenMenu = Menu_Game_f;
		break;
	case 1:
		thisObj->OpenMenu = Menu_Multiplayer_f;
		break;
	case 2:
		thisObj->OpenMenu = Menu_Options_f;
		break;
	case 3:
		thisObj->OpenMenu = Menu_Video_f;
		break;
	case 4:
		thisObj->OpenMenu = Menu_Quit_f;
		break;
	}
}


/*
=================
UI_PushMenu
=================
*/
void UI_PushMenu ( void (*draw) (void), const char *(*key) (int k) )
{
	int		i;

	if (Cvar_VariableValue ("maxclients") == 1 && Com_ServerState () && !cls.consoleActive) // Knightmare added
		Cvar_Set ("paused", "1");

	// Knightmare- if just opened menu, and ingame and not DM, grab screen first
	if (cls.key_dest != key_menu && !Cvar_VariableValue("deathmatch")
		&& Com_ServerState() == 2) //ss_game
		//&& !cl.cinematictime && Com_ServerState())
		R_GrabScreen();

	// if this menu is already present, drop back to that level
	// to avoid stacking menus by hotkeys
	for (i=0 ; i<ui_menudepth ; i++)
		if (ui_layers[i].draw == draw &&
			ui_layers[i].key == key)
		{
			ui_menudepth = i;
		}

	if (i == ui_menudepth)
	{
		if (ui_menudepth >= MAX_MENU_DEPTH)
			Com_Error (ERR_FATAL, "UI_PushMenu: MAX_MENU_DEPTH");
		ui_layers[ui_menudepth].draw = m_drawfunc;
		ui_layers[ui_menudepth].key = m_keyfunc;
		ui_menudepth++;
	}

	m_drawfunc = draw;
	m_keyfunc = key;

	ui_entersound = true;

	// Knightmare- added Psychospaz's mouse support
	UI_RefreshCursorLink ();
	UI_RefreshCursorButtons ();

	cls.key_dest = key_menu;
}


/*
=================
UI_ForceMenuOff
=================
*/
void UI_ForceMenuOff (void)
{
	// Knightmare- added Psychospaz's mouse support
	UI_RefreshCursorLink ();
	m_drawfunc = 0;
	m_keyfunc = 0;
	cls.key_dest = key_game;
	ui_menudepth = 0;
	Key_ClearStates ();
	if (!cls.consoleActive && Cvar_VariableValue ("maxclients") == 1 && Com_ServerState()) // Knightmare added
		Cvar_Set ("paused", "0");
}


/*
=================
UI_PopMenu
=================
*/
void UI_PopMenu (void)
{
	S_StartLocalSound (ui_menu_out_sound);
	if (ui_menudepth < 1)
		Com_Error (ERR_FATAL, "UI_PopMenu: depth < 1");
	ui_menudepth--;

	m_drawfunc = ui_layers[ui_menudepth].draw;
	m_keyfunc = ui_layers[ui_menudepth].key;

	// Knightmare- added Psychospaz's mouse support
	UI_RefreshCursorLink ();
	UI_RefreshCursorButtons ();

	if (!ui_menudepth)
		UI_ForceMenuOff ();
}


/*
=================
UI_BackMenu
=================
*/
void UI_BackMenu (void *unused)
{
	// We need to manually save changes for playerconfig menu here
	if (m_drawfunc == Menu_PlayerConfig_Draw)
		Menu_PConfigSaveChanges ();

	UI_PopMenu ();
}


/*
==========================
UI_AddMenuItem
==========================
*/
void UI_AddMenuItem (menuframework_s *menu, void *item)
{
	int			i, j;
	menulist_s	*list;
	menucommon_s *baseItem;

	if (menu->nitems == 0)
		menu->nslots = 0;

	if (menu->nitems < MAXMENUITEMS)
	{
		menu->items[menu->nitems] = item;
		( (menucommon_s *)menu->items[menu->nitems] )->parent = menu;
		menu->nitems++;
	}

	menu->nslots = UI_TallyMenuSlots(menu);

	list = (menulist_s *)item;

	switch (list->generic.type) {
	case MTYPE_SPINCONTROL:
		for (i=0; list->itemNames[i]; i++);
		list->numItems = i;
		if (list->itemValues)	// Check if itemvalues count matches itemnames
		{
			for (j=0; list->itemValues[j]; j++);
			if (j != i) {
				Com_Printf (S_COLOR_YELLOW"UI_AddMenuItem: itemvalues size mismatch for %s!\n",
							(list->generic.name && (list->generic.name[0] != 0)) ? list->generic.name : "<noname>");
			}
		}
		break;
	}

	// Knightmare- init text size
	baseItem = (menucommon_s *)item;
	if (!baseItem->textSize)
		baseItem->textSize = MENU_FONT_SIZE;
	baseItem->textSize = min(max(baseItem->textSize, 4), 32);
	// end Knightmare

	UI_ClearGrabBindItem (menu); // make sure this starts out unset
}


/*
=================
UI_SetGrabBindItem
=================
*/
void UI_SetGrabBindItem (menuframework_s *m, menucommon_s *item)
{
	int				i;
	menucommon_s	*it;
	qboolean		found = false;

	for (i = 0; i < m->nitems; i++)
	{
		it = (menucommon_s *)m->items[i];
		if (it->type == MTYPE_KEYBIND) 
		{
			if (it == item) {
				((menukeybind_s *)it)->grabBind = true;
				m->grabBindCursor = i;
				found = true;
			}
			else // clear grab flag if it's not the one
				((menukeybind_s *)it)->grabBind = false;
		}
	}

	if (!found)
		m->grabBindCursor = -1;
}


/*
=================
UI_ClearGrabBindItem
=================
*/
void UI_ClearGrabBindItem (menuframework_s *m)
{
	int				i;
	menucommon_s	*it;

	m->grabBindCursor = -1;

	// clear grab flag for all keybind items
	for (i = 0; i < m->nitems; i++)
	{
		it = (menucommon_s *)m->items[i];
		if (it->type == MTYPE_KEYBIND)
			((menukeybind_s *)it)->grabBind = false;
	}
}


/*
=================
UI_HasValidGrabBindItem
=================
*/
qboolean UI_HasValidGrabBindItem (menuframework_s *m)
{
	if (!m)	return false;

	if ( (m->grabBindCursor != -1)
		&& (m->grabBindCursor >= 0)
		&& (m->grabBindCursor < m->nitems)
		&& (((menucommon_s *)m->items[m->grabBindCursor])->type == MTYPE_KEYBIND)
		&& (((menukeybind_s *)m->items[m->grabBindCursor])->grabBind) )
		return true;

	return false;
}


/*
==========================
UI_MenuItemIsValidCursorPosition
Checks if an item can be used
as a cursor position.
==========================
*/
qboolean UI_MenuItemIsValidCursorPosition (void *item)
{
	if (!item)	return false;
	
//	if ( (((menuCommon_s *)item)->flags & QMF_NOINTERACTION) || (((menuCommon_s *)item)->flags & QMF_MOUSEONLY) )
//		return false;

	// hidden items are invalid
	if ( ((menucommon_s *)item)->flags & QMF_HIDDEN )
		return false;

	switch ( ((menucommon_s *)item)->type )
	{
	case MTYPE_SEPARATOR:
		return false;
	default:
		return true;
	}
	return true;
}


/*
==========================
UI_AdjustMenuCursor

This function takes the given menu, the direction, and attempts
to adjust the menu's cursor so that it's at the next available
slot.
==========================
*/
void UI_AdjustMenuCursor (menuframework_s *m, int dir)
{
	menucommon_s *citem;

	//
	// see if it's in a valid spot
	//
	if (m->cursor >= 0 && m->cursor < m->nitems)
	{
		if ( (citem = UI_ItemAtMenuCursor(m)) != 0 )
		{
		//	if (citem->type != MTYPE_SEPARATOR)
			if ( UI_MenuItemIsValidCursorPosition(citem) )
				return;
		}
	}

	//
	// it's not in a valid spot, so crawl in the direction indicated until we
	// find a valid spot
	//
	if (dir == 1)
	{
		while (1)
		{
			if ( (citem = UI_ItemAtMenuCursor(m)) != 0 )
			//	if ( citem->type != MTYPE_SEPARATOR )
				if ( UI_MenuItemIsValidCursorPosition(citem) )
					break;
			m->cursor += dir;
			if ( m->cursor >= m->nitems )
				m->cursor = 0;
		}
	}
	else
	{
		while (1)
		{
			if ( (citem = UI_ItemAtMenuCursor(m)) != 0 )
			//	if (citem->type != MTYPE_SEPARATOR)
				if ( UI_MenuItemIsValidCursorPosition(citem) )
					break;
			m->cursor += dir;
			if (m->cursor < 0)
				m->cursor = m->nitems - 1;
		}
	}
}


/*
==========================
UI_CenterMenu
==========================
*/
void UI_CenterMenu (menuframework_s *menu)
{
	int height = ((menucommon_s *) menu->items[menu->nitems-1])->y + 10;
	menu->y = (SCREEN_HEIGHT - height)*0.5;
}


/*
==========================
UI_DrawMenu
==========================
*/
void UI_DrawMenu (menuframework_s *menu)
{
	int i;
	menucommon_s *item;

	//
	// draw contents
	//
	for (i = 0; i < menu->nitems; i++)
	{
		UI_DrawMenuItem (menu->items[i]);
	}

	// Psychspaz's mouse support
	// check for mouseovers
	UI_Mouseover_Check (menu);

	item = UI_ItemAtMenuCursor(menu);

	if (item && item->cursordraw)
	{
		item->cursordraw(item);
	}
	else if (menu->cursordraw)
	{
		menu->cursordraw(menu);
	}
	else if (item && item->type != MTYPE_FIELD)
	{
		char	*cursor;
		int		cursorX;

#ifdef NOTTHIRTYFLIGHTS
		if (item->type == MTYPE_KEYBIND && ((menukeybind_s *)item)->grabBind)
			cursor = UI_ITEMCURSOR_KEYBIND_PIC;
		else
			cursor = ((int)(Sys_Milliseconds()/250)&1) ? UI_ITEMCURSOR_DEFAULT_PIC : UI_ITEMCURSOR_BLINK_PIC;
#else
		if (item->flags & QMF_LEFT_JUSTIFY)
			cursorX = menu->x + item->x + item->cursor_offset - 24;
		else
			cursorX = menu->x + item->cursor_offset;
#endif
#ifdef NOTTHIRTYFLIGHTS
		cursorX = menu->x + item->x + item->cursor_offset;
		if ( (item->flags & QMF_LEFT_JUSTIFY) && (item->type == MTYPE_ACTION) )
			cursorX -= 4*MENU_FONT_SIZE;

		UI_DrawPic (cursorX, menu->y+item->y, item->textSize, item->textSize, ALIGN_CENTER, false, cursor, 255);
#else
		// TFOL added the sin/anglemod portion and removed the blink
		if (item->flags & QMF_LEFT_JUSTIFY)
		{
			SCR_DrawChar (menu->x+item->x+item->cursor_offset-28+ (3*sin(anglemod(cl.time*0.01))), menu->y+item->y,
						item->textSize, ALIGN_CENTER, 13 /* 12+((int)(Sys_Milliseconds()/250)&1) */,
						FONT_UI, 255,255,255,255, false, true);
		}
		else
		{
			SCR_DrawChar (menu->x+item->cursor_offset+ (5*sin(anglemod(cl.time*0.01))), menu->y+item->y,
						item->textSize, ALIGN_CENTER, 13 /* 12+((int)(Sys_Milliseconds()/250)&1) */,
						FONT_UI, 255,255,255,255, false, true);
		}
#endif
	}

	if (item)
	{
	//	if (item->statusbarfunc)
	//		item->statusbarfunc ( (void *)item );
		if (item->type == MTYPE_KEYBIND && ((menukeybind_s *)item)->grabBind && ((menukeybind_s *)item)->enter_statusbar)
			UI_DrawMenuStatusBar (((menukeybind_s *)item)->enter_statusbar);
		else if (item->statusbar)
			UI_DrawMenuStatusBar (item->statusbar);
		else
			UI_DrawMenuStatusBar (menu->statusbar);
	}
	else
		UI_DrawMenuStatusBar (menu->statusbar);
}


/*
=================
UI_DefaultMenuKey
=================
*/
const char *UI_DefaultMenuKey (menuframework_s *m, int key)
{
	const char *sound = NULL;
	menucommon_s *item;

	if ( m )
	{
		if ( ( item = UI_ItemAtMenuCursor( m ) ) != 0 )
		{
			if ( item->type == MTYPE_FIELD )
			{
				if ( UI_MenuField_Key((menufield_s *) item, key) )
					return NULL;
			}
			else if (item->type == MTYPE_KEYBIND)
			{
				if ( (sound = UI_MenuKeyBind_Key((menukeybind_s *)item, key)) != ui_menu_null_sound )
					return sound;
				else
					sound = NULL;
			}
		}
	}

	switch ( key )
	{
	case K_JOY2: //BC 1/30/2024 gamepad B button
	case K_ESCAPE:
		UI_PopMenu ();
		return ui_menu_out_sound;
	case K_AUX29: //BC 1/30/2024 dpad up.
	case K_JOY_UP: //BC 1/30/2024 joystick up.
	case K_KP_UPARROW:
	case K_UPARROW:
		if ( m )
		{
			m->cursor--;
			// Knightmare- added Psychospaz's mouse support
			UI_RefreshCursorLink ();

			UI_AdjustMenuCursor (m, -1);
			sound = ui_menu_move_sound;
		}
		break;
	case K_AUX31: //BC 1/30/2024 dpad down.
	case K_JOY_DOWN: //BC 1/30/2024 joystick down.
	case K_TAB:
	case K_KP_DOWNARROW:
	case K_DOWNARROW:
		if ( m )
		{
			m->cursor++;
			// Knightmare- added Psychospaz's mouse support

			UI_RefreshCursorLink ();
			UI_AdjustMenuCursor (m, 1);
			sound = ui_menu_move_sound;
		}
		break;
	case K_AUX32: //BC 1/30/2024 dpad left.
	case K_KP_LEFTARROW:
	case K_LEFTARROW:
		if ( m )
		{
			UI_SlideMenuItem (m, -1);
			sound = ui_menu_move_sound;
		}
		break;
	case K_AUX30: //BC 1/30/2024 dpad right.
	case K_KP_RIGHTARROW:
	case K_RIGHTARROW:
		if ( m )
		{
			UI_SlideMenuItem (m, 1);
			sound = ui_menu_move_sound;
		}
		break;

	/*case K_MOUSE1:
	case K_MOUSE2:
	case K_MOUSE3:
	//Knightmare 12/22/2001
	case K_MOUSE4:
	case K_MOUSE5:*/
	//end Knightmare
	case K_JOY1:
	//case K_JOY2: //BC B button for menu selection
	case K_JOY3:
	case K_JOY4:
	case K_AUX1:
	case K_AUX2:
	case K_AUX3:
	case K_AUX4:
	case K_AUX5:
	case K_AUX6:
	case K_AUX7:
	case K_AUX8:
	case K_AUX9:
	case K_AUX10:
	case K_AUX11:
	case K_AUX12:
	case K_AUX13:
	case K_AUX14:
	case K_AUX15:
	case K_AUX16:
	case K_AUX17:
	case K_AUX18:
	case K_AUX19:
	case K_AUX20:
	case K_AUX21:
	case K_AUX22:
	case K_AUX23:
	case K_AUX24:
	case K_AUX25:
	case K_AUX26:
	case K_AUX27:
	case K_AUX28:
	//case K_AUX29: //BC dpad up for menu selection
	//case K_AUX30: //BC dpad right for menu selection
	//case K_AUX31: //BC dpad down for menu selection
	//case K_AUX32: //BC dpad left for menu selection
		
#ifndef NOTTHIRTYFLIGHTS
	case K_SPACE:
#endif
	case K_KP_ENTER:
	case K_ENTER:
#ifdef NOTTHIRTYFLIGHTS
		if ( m )
			UI_SelectMenuItem (m);
		sound = ui_menu_move_sound;
#else
		if ( m )
		{
			if ( item->type == MTYPE_CHECKBOX )
			{
				
				UI_SlideMenuItem( m, 1 );
				sound = menu_click;
			}
			else 
			{
				
				UI_SelectMenuItem( m );

				if ( item->type == MTYPE_ACTION )
					sound = ui_menu_move_sound;
				
			}
		}
#endif
		break;
	}

	return sound;
}
