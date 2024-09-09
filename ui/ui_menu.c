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
menulayer_t	ui_menuState;

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
void UI_AddMainButton (mainmenuobject_t *thisObj, int index, float x, float y, char *name)
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
#ifdef NOTTHIRTYFLIGHTS
		thisObj->OpenMenu = Menu_Multiplayer_f;
#endif
		break;
	case 2:
		thisObj->OpenMenu = Menu_Options_f;
		break;
	case 3:
#ifdef NOTTHIRTYFLIGHTS
		thisObj->OpenMenu = Menu_Video_f;
#endif
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
void UI_PushMenu (menuFramework_s *menu)
{
	int			i;
	qboolean	canOpen;

	if (!menu)	return;

	// if new menu has a conditional opening func, use it
	if (menu->canOpenFunc)
	{
		if ( (canOpen = menu->canOpenFunc(menu)) != true ) {

			if ( (ui_menudepth > 0) && ui_menuState.menu ) {
				if (menu->cantOpenMessage)
					UI_SetMenuCurrentItemStatusBar (ui_menuState.menu, menu->cantOpenMessage);
				else
					UI_SetMenuCurrentItemStatusBar (ui_menuState.menu, NULL);
			}
			return;
		}
		if (ui_menudepth > 0 && ui_menuState.menu)
			UI_SetMenuCurrentItemStatusBar (ui_menuState.menu, NULL);
	}

	// set item values for menu
	UI_LoadMenuBitFlags (menu);
	UI_SetMenuItemValues (menu);

	if (Cvar_VariableValue ("maxclients") == 1 && Com_ServerState () && !cls.consoleActive) // Knightmare added
		Cvar_Set ("paused", "1");

	// if just opened menu, and ingame and not DM, grab screen first
	if ( (cls.key_dest != key_menu) && !Cvar_VariableValue("deathmatch")
		&& (Com_ServerState() == 2) ) // ss_game
		//&& !cl.cinematictime && Com_ServerState())
		R_GrabScreen ();

	// if this menu is already present, drop back to that level
	// to avoid stacking menus by hotkeys
	for (i=0; i<ui_menudepth; i++)
		if (ui_layers[i].menu == menu)
		{
			ui_menudepth = i;
		}

	if (i == ui_menudepth)
	{
		if (ui_menudepth >= MAX_MENU_DEPTH)
			Com_Error (ERR_FATAL, "UI_PushMenu: MAX_MENU_DEPTH");
		ui_layers[ui_menudepth].menu = ui_menuState.menu;
		ui_menudepth++;
	}

	ui_menuState.menu = menu;

	ui_entersound = true;

	// added Psychospaz's mouse support
	UI_RefreshCursorLink ();
	UI_RefreshCursorButtons ();

	cls.key_dest = key_menu;

	// if menu has an on open function, call it now
	if (menu->onOpenFunc)
		menu->onOpenFunc (NULL);
}


/*
=================
UI_ForceMenuOff
=================
*/
void UI_ForceMenuOff (void)
{
	// added Psychospaz's mouse support
	UI_RefreshCursorLink ();
	memset (&ui_menuState, 0, sizeof (ui_menuState));
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

	UI_SetMenuStatusBar (ui_menuState.menu, NULL);	// Clear status bar
	ui_menudepth--;

	ui_menuState.menu = ui_layers[ui_menudepth].menu;

	// added Psychospaz's mouse support
	UI_RefreshCursorLink ();
	UI_RefreshCursorButtons ();

	if (!ui_menudepth)
	{	// start demo loop here if disconnected
		if (cls.state == ca_disconnected)
			Cbuf_AddText ("d1\n");
		UI_ForceMenuOff ();
	}
	else	// Refresh items in restored menu
		UI_RefreshMenuItems ();
}


/*
=================
UI_BackMenu
=================
*/
void UI_BackMenu (void *item)
{
	menuFramework_s	*menu;

	if (item) {
		menu = ((menuCommon_s *)item)->parent;
		if (menu && menu->onExitFunc)
			menu->onExitFunc (menu);
	}

	UI_PopMenu ();
}


/*
=================
UI_CheckAndPopMenu
=================
*/
void UI_CheckAndPopMenu (menuFramework_s *menu)
{
	if (menu && menu->onExitFunc)
		menu->onExitFunc (menu);

	UI_PopMenu ();
}


/*
=================
UI_LoadMenuBitFlags
=================
*/
void UI_LoadMenuBitFlags (menuFramework_s *menu)
{
	if ( !menu || !menu->flagCvar || !strlen(menu->flagCvar) )
		return;

	menu->bitFlags = Cvar_VariableValue(menu->flagCvar);
	Com_sprintf (menu->bitFlags_statusbar, sizeof(menu->bitFlags_statusbar), "%s = %d", menu->flagCvar, menu->bitFlags);
}


/*
=================
UI_SetMenuBitFlags
=================
*/
void UI_SetMenuBitFlags (menuFramework_s *menu, int bit, qboolean set)
{
	if ( !menu || !menu->flagCvar || !strlen(menu->flagCvar) )
		return;

	if (bit > 0) {
		if (set)
			menu->bitFlags |= bit;
		else
			menu->bitFlags &= ~bit;
	}
	Cvar_SetValue (menu->flagCvar, menu->bitFlags);
	Com_sprintf (menu->bitFlags_statusbar, sizeof(menu->bitFlags_statusbar), "%s = %d", menu->flagCvar, menu->bitFlags);
}


/*
==========================
UI_AddMenuItem
==========================
*/
void UI_AddMenuItem (menuFramework_s *menu, void *item)
{
	qboolean	canOpen;

	// if menu can't open, don't add items to it
	if (menu->canOpenFunc) {
		if ( (canOpen = menu->canOpenFunc(menu)) != true )
			return;
	}

	if (menu->nitems == 0)
		menu->nslots = 0;

	if (menu->nitems < MAX_MENUITEMS)
	{
		menu->items[menu->nitems] = item;
		( (menuCommon_s *)menu->items[menu->nitems] )->parent = menu;
		menu->nitems++;
	}

	menu->nslots = UI_TallyMenuSlots(menu);

	// setup each item's fields
	UI_InitMenuItem (item);

	UI_ClearGrabBindItem (menu); // make sure this starts out unset
}


/*
=================
UI_SetGrabBindItem
=================
*/
void UI_SetGrabBindItem (menuFramework_s *menu, menuCommon_s *item)
{
	int				i;
	menuCommon_s	*it;
	qboolean		found = false;

	for (i = 0; i < menu->nitems; i++)
	{
		it = (menuCommon_s *)menu->items[i];
		if (it->type == MTYPE_KEYBIND) 
		{
			if (it == item) {
				((menuKeyBind_s *)it)->grabBind = true;
				menu->grabBindCursor = i;
				found = true;
			}
			else // clear grab flag if it's not the one
				((menuKeyBind_s *)it)->grabBind = false;
		}
		else if (it->type == MTYPE_KEYBINDLIST) 
		{
			if (it == item) {
				((menuKeyBindList_s *)it)->grabBind = true;
				menu->grabBindCursor = i;
				found = true;
			}
			else // clear grab flag if it's not the one
				((menuKeyBindList_s *)it)->grabBind = false;
		}
	}

	if (!found)
		menu->grabBindCursor = -1;
}


/*
=================
UI_ClearGrabBindItem
=================
*/
void UI_ClearGrabBindItem (menuFramework_s *menu)
{
	int				i;
	menuCommon_s	*it;

	menu->grabBindCursor = -1;

	// clear grab flag for all keybind items
	for (i = 0; i < menu->nitems; i++)
	{
		it = (menuCommon_s *)menu->items[i];
		if (it->type == MTYPE_KEYBIND)
			((menuKeyBind_s *)it)->grabBind = false;
		else if (it->type == MTYPE_KEYBINDLIST)
			((menuKeyBindList_s *)it)->grabBind = false;
	}
}


/*
=================
UI_HasValidGrabBindItem
=================
*/
qboolean UI_HasValidGrabBindItem (menuFramework_s *menu)
{
	if (!menu)	return false;

	if ( (menu->grabBindCursor != -1)
		&& (menu->grabBindCursor >= 0)
		&& (menu->grabBindCursor < menu->nitems)
		&& (((menuCommon_s *)menu->items[menu->grabBindCursor])->type == MTYPE_KEYBIND)
		&& (((menuKeyBind_s *)menu->items[menu->grabBindCursor])->grabBind) )
		return true;

	if ( (menu->grabBindCursor != -1)
		&& (menu->grabBindCursor >= 0)
		&& (menu->grabBindCursor < menu->nitems)
		&& (((menuCommon_s *)menu->items[menu->grabBindCursor])->type == MTYPE_KEYBINDLIST)
		&& (((menuKeyBindList_s *)menu->items[menu->grabBindCursor])->grabBind) )
		return true;

	return false;
}


/*
=================
UI_RefreshItemsForMenu

Refreshes models for the given menu.
=================
*/
void UI_RefreshItemsForMenu (menuFramework_s *menu)
{
	int				i;
	menuCommon_s	*item;

	if (!menu)	return;

	// Check if this menu is a popup.
	// If it is, refresh menu beneath.
	if ( menu->isPopup && (ui_menudepth > 1) && ui_layers[ui_menudepth-1].menu )
		UI_RefreshItemsForMenu (ui_layers[ui_menudepth-1].menu);

	for (i=0; i<menu->nitems; i++)
	{
		item = menu->items[i];
		if (!item)	continue;
		UI_ReregisterMenuItem (item);
	}
}


/*
=================
UI_RefreshMenuItems

Refreshes models in all menus on stack.
Called after a vid restart.
=================
*/
void UI_RefreshMenuItems (void)
{
	menuFramework_s	*menu;

	if (cls.key_dest != key_menu)	return;

	menu = ui_menuState.menu;
	if (!menu)	return;

	UI_RefreshItemsForMenu (menu);
}


/*
=================
UI_SetMenuItemValues

Loads values for all menu items
from linked cvars.
=================
*/
void UI_SetMenuItemValues (menuFramework_s *menu)
{
	int				i;
	menuCommon_s	*item;

	if (!menu)	return;

	for (i=0; i<menu->nitems; i++)
	{
		item = menu->items[i];
		if (!item)	continue;
		UI_SetMenuItemValue (item);
	}
}


/*
=================
UI_SetMenuDefaults
=================
*/
void UI_SetMenuDefaults (void)
{
	menuFramework_s *curMenu;

	if (ui_menudepth < 1)
		return;

	curMenu = ui_menuState.menu;
	if (curMenu && curMenu->defaultsFunc) {
		curMenu->defaultsFunc ();
		UI_SetMenuItemValues (curMenu);	// refresh menu items
	}
}


/*
=================
UI_GetDefaultsMessage
=================
*/
const char *UI_GetDefaultsMessage (void)
{
	menuFramework_s *curMenu;

	if (ui_menudepth < 1)
		return UI_DEFAULTS_MESSAGE;

	curMenu = ui_menuState.menu;
	if (curMenu && curMenu->defaultsMessage
		&& (strlen(curMenu->defaultsMessage) > 0))
		return curMenu->defaultsMessage;
	else
		return UI_DEFAULTS_MESSAGE;
}

/*
=================
UI_Defaults_Popup
=================
*/
void UI_Defaults_Popup (void *unused)
{
	Menu_DefaultsConfirm_f ();
}


/*
=================
UI_ApplyMenuChanges
=================
*/
void UI_ApplyMenuChanges (void)
{
	menuFramework_s *curMenu;

	if (ui_menudepth < 1)
		return;

	curMenu = ui_menuState.menu;
	if (curMenu && curMenu->applyChangesFunc) {
		curMenu->applyChangesFunc ();
		UI_SetMenuItemValues (curMenu);	// refresh menu items
	}
}

/*
=================
UI_GetApplyChangesMessage
=================
*/
const char *UI_GetApplyChangesMessage (int line)
{
	menuFramework_s *curMenu;

	if (ui_menudepth < 1)
		return UI_APPLYCHANGES_MESSAGE;

	curMenu = ui_menuState.menu;
	if (line == 1) {
		if (curMenu && curMenu->applyChangesMessage[1]
			&& (strlen(curMenu->applyChangesMessage[1]) > 0))
			return curMenu->applyChangesMessage[1];
		else
			return NULL;
	}
	else if (line == 2) {
		if (curMenu && curMenu->applyChangesMessage[2]
			&& (strlen(curMenu->applyChangesMessage[2]) > 0))
			return curMenu->applyChangesMessage[2];
		else
			return UI_APPLYCHANGES_MESSAGE2;
	}
	else {
		if (curMenu && curMenu->applyChangesMessage[0]
			&& (strlen(curMenu->applyChangesMessage[0]) > 0))
			return curMenu->applyChangesMessage[0];
		else
			return UI_APPLYCHANGES_MESSAGE;
	}
}


/*
=================
UI_ApplyChanges_Popup
=================
*/
void UI_ApplyChanges_Popup (void *unused)
{
	Menu_ApplyChanges_f ();
}


/*
==========================
UI_AdjustMenuCursor

This function takes the given menu, the direction, and attempts
to adjust the menu's cursor so that it's at the next available
slot.
==========================
*/
void UI_AdjustMenuCursor (menuFramework_s *menu, int dir)
{
	int				loopCount;
	menuCommon_s	*citem;

	// check if menu only has one item
	if (menu->nitems < 2)
		return;

	//
	// see if it's in a valid spot
	//
	if (menu->cursor >= 0 && menu->cursor < menu->nitems)
	{
		if ( (citem = UI_ItemAtMenuCursor(menu)) != 0 )
		{
			if ( UI_ItemIsValidCursorPosition(citem) )
				return;
		}
	}

	//
	// it's not in a valid spot, so crawl in the direction indicated until we
	// find a valid spot
	//
	loopCount = 0;
	while (1)
	{
		if ( (citem = UI_ItemAtMenuCursor(menu)) != 0 )
			if ( UI_ItemIsValidCursorPosition(citem) )
				break;
		menu->cursor += dir;
		if (dir == 1)
		{
			if ( menu->cursor >= menu->nitems ) {
				menu->cursor = 0;
				loopCount++;
			}
		}
		else
		{
			if (menu->cursor < 0) {
				menu->cursor = menu->nitems - 1;
				loopCount++;
			}
		}
		if (loopCount > 1)
			break;
	}
}


/*
==========================
UI_DrawMenu
==========================
*/
void UI_DrawMenu (menuFramework_s *menu)
{
	int				i;
	menuCommon_s	*item, *cursorItem;
	qboolean		hasCursorItem = false;

	// Check if this menu is a popup.
	// If it is, draw menu beneath.
	if ( menu->isPopup && (ui_menudepth > 1) && ui_layers[ui_menudepth-1].menu )
		UI_DrawMenu (ui_layers[ui_menudepth-1].menu);

	// update cursor item coords
	if ( ((cursorItem = menu->cursorItem) != NULL)
		&& ((item = UI_ItemAtMenuCursor (menu)) != NULL) )
	{
		if (cursorItem->isCursorItem) {
			cursorItem->x = item->x + cursorItem->cursorItemOffset[0];
			cursorItem->y = item->y + cursorItem->cursorItemOffset[1];
			UI_UpdateMenuItemCoords (cursorItem);
			hasCursorItem = true;
		}
	}

	//
	// draw contents
	//
	for (i = 0; i < menu->nitems; i++)
	{
		// cursor items can only be drawn if specified by the menu
		if ( ((menuCommon_s *)menu->items[i])->isCursorItem
			&& (menu->cursorItem != menu->items[i]) )
			continue;

		if (ui_debug_itembounds->integer)
		{
			float	w, h;

			item = menu->items[i];
			w = item->botRight[0] - item->topLeft[0];
			h = item->botRight[1] - item->topLeft[1];

			// add length and height of current item
			UI_SetMenuItemDynamicSize (menu->items[i]);
			w += item->dynamicWidth;
			h += item->dynamicHeight;

			if ( UI_ItemHasMouseBounds(item) )
				UI_DrawFill (item->topLeft[0], item->topLeft[1], w, h, item->scrAlign, false, 128,128,128,128);
			item = NULL;
		}

		UI_DrawMenuItem (menu->items[i]);
	}

	// Check if this menu is drawn behind a popup.
	// If it is, get outta here.
	if (menu != ui_menuState.menu)
		return;

	//
	// draw item extensions
	//
	for (i = 0; i < menu->nitems; i++)
	{
		UI_DrawMenuItemExtension (menu->items[i]);
	}

	// Psychspaz's mouse support
	// check for mouseovers
	UI_Mouseover_Check (menu);

	item = UI_ItemAtMenuCursor(menu);

	if (!hasCursorItem)
	{
		if (item && item->cursordraw)
		{
			item->cursordraw (item);
		}
		else if (menu->cursordraw)
		{
			menu->cursordraw (menu);
		}
		else if ( item && (item->type != MTYPE_FIELD) && !(item->flags & QMF_NOINTERACTION) )
		{
			char		*cursor;
			int			cursorX, cursorOscillate = 0;
			qboolean	oscillate = false;

			if ( (menu->cursorOscillate_amplitude != 0.0f) && (menu->cursorOscillate_timeScale != 0.0f) ) {
				oscillate = true;
				cursorOscillate = (int)(menu->cursorOscillate_amplitude * sin(anglemod(cl.time * menu->cursorOscillate_timeScale)));
			}

			if ( (item->type == MTYPE_KEYBIND) && ((menuKeyBind_s *)item)->grabBind )
				cursor = UI_ITEMCURSOR_KEYBIND_PIC;
			else if ( oscillate )
				cursor = UI_ITEMCURSOR_DEFAULT_PIC;	// oscillating cursor doesn't blink
			else
				cursor = ((int)(Sys_Milliseconds()/250)&1) ? UI_ITEMCURSOR_DEFAULT_PIC : UI_ITEMCURSOR_BLINK_PIC;

			cursorX = menu->x + item->x + item->cursor_offset;
			if ( (item->flags & QMF_LEFT_JUSTIFY) && (item->type == MTYPE_ACTION) )
				cursorX -= 4*MENU_FONT_SIZE;

			UI_DrawPic (cursorX + cursorOscillate, menu->y + item->y, item->textSize, item->textSize, item->scrAlign, false, cursor, 255);

		/*	if (item->flags & QMF_LEFT_JUSTIFY)
			{
				UI_DrawChar (menu->x+item->x+item->cursor_offset-24, menu->y+item->y,
							item->textSize, ALIGN_CENTER, 12+((int)(Sys_Milliseconds()/250)&1),
							FONT_UI, 255,255,255,255, false, true);
			}
			else
			{
				UI_DrawChar (menu->x+item->cursor_offset, menu->y+item->y,
							item->textSize, ALIGN_CENTER, 12+((int)(Sys_Milliseconds()/250)&1),
							FONT_UI, 255,255,255,255, false, true);
			} */
		}
	}

	if (!menu->hide_statusbar)
	{
		if (item)
		{
		//	if (item->statusbarfunc)
		//		item->statusbarfunc ( (void *)item );
			if ( (item->type == MTYPE_KEYBIND) && ((menuKeyBind_s *)item)->grabBind && ((menuKeyBind_s *)item)->enter_statusbar)
				UI_DrawMenuStatusBar (((menuKeyBind_s *)item)->enter_statusbar);
			else if ( (item->type == MTYPE_KEYBINDLIST) && ((menuKeyBindList_s *)item)->grabBind && ((menuKeyBindList_s *)item)->enter_statusbar)
				UI_DrawMenuStatusBar (((menuKeyBindList_s *)item)->enter_statusbar);
			else if (item->statusbar)
				UI_DrawMenuStatusBar (item->statusbar);
			else if ( menu->flagCvar && (strlen(menu->flagCvar) > 0) )
				UI_DrawMenuStatusBar (menu->bitFlags_statusbar);
			else
				UI_DrawMenuStatusBar (menu->statusbar);
		}
		else
			UI_DrawMenuStatusBar (menu->statusbar);
	}
}


/*
=================
UI_DefaultMenuDraw
=================
*/
void UI_DefaultMenuDraw (menuFramework_s *menu)
{
	if (!menu)	return;

	UI_AdjustMenuCursor (menu, 1);
	UI_DrawMenu (menu);
}


/*
=================
UI_DefaultMenuKey
=================
*/
const char *UI_DefaultMenuKey (menuFramework_s *menu, int key)
{
	const char		*sound = NULL;
	menuCommon_s	*item;

	if ( menu )
	{
		if ( ( item = UI_ItemAtMenuCursor( menu ) ) != 0 )
		{
			if ( item->type == MTYPE_FIELD )
			{
				if ( UI_MenuField_Key((menuField_s *) item, key) )
					return NULL;
			}
			else if (item->type == MTYPE_KEYBIND)
			{
				if ( (sound = UI_MenuKeyBind_Key((menuKeyBind_s *)item, key)) != ui_menu_null_sound )
					return sound;
				else
					sound = NULL;
			}
			else if (item->type == MTYPE_KEYBINDLIST)
			{
				if ( (sound = UI_MenuKeyBindList_Key((menuKeyBindList_s *)item, key)) != ui_menu_null_sound )
					return sound;
				else
					sound = NULL;
			}
		}
	}

	switch ( key )
	{
	case K_ESCAPE:
		UI_CheckAndPopMenu (menu);
		return ui_menu_out_sound;
	case K_KP_UPARROW:
	case K_UPARROW:
		if ( menu )
		{
			menu->cursor--;
			// added Psychospaz's mouse support
			UI_RefreshCursorLink ();
			UI_AdjustMenuCursor (menu, -1);
			sound = ui_menu_move_sound;
		}
		break;
	case K_TAB:
	case K_KP_DOWNARROW:
	case K_DOWNARROW:
		if ( menu )
		{
			menu->cursor++;
			// added Psychospaz's mouse support
			UI_RefreshCursorLink ();
			UI_AdjustMenuCursor (menu, 1);
			sound = ui_menu_move_sound;
		}
		break;
	case K_KP_LEFTARROW:
	case K_LEFTARROW:
		if ( menu )
		{
			sound = UI_SlideMenuItem (menu, -1);
		}
		break;
	case K_KP_RIGHTARROW:
	case K_RIGHTARROW:
		if ( menu )
		{
			sound = UI_SlideMenuItem (menu, 1);
		}
		break;
	case K_MWHEELUP:
		if (menu)
		{
			UI_ScrollMenuItem (menu, -1);
		}
		break;
	case K_MWHEELDOWN:
		if (menu)
		{
			UI_ScrollMenuItem (menu, 1);
		}
		break;

	case K_JOY1:
	case K_JOY2:
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
	case K_AUX29:
	case K_AUX30:
	case K_AUX31:
	case K_AUX32:
		
	case K_KP_ENTER:
	case K_ENTER:
		if ( menu )
			UI_SelectMenuItem (menu);
		sound = ui_menu_move_sound;
		break;
	}

	return sound;
}


/*
=================
UI_QuitMenuKey

Just used by the quit menu
=================
*/
const char *UI_QuitMenuKey (menuFramework_s *menu, int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case 'n':
	case 'N':
		UI_PopMenu ();
		break;

	case 'Y':
	case 'y':
	//	UI_QuitYesFunc ();
		cls.key_dest = key_console;
		CL_Quit_f ();
		break;

	default:
		break;
	}
	return NULL;
}
