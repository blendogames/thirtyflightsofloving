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

// menu_apply_changes.c -- apply changes confirm popup menu

#include "../client/client.h"
#include "ui_local.h"

/*
=======================================================================

APPLY CHANGES POPUP

=======================================================================
*/

static menuFramework_s	s_applychanges_popup;
static menuRectangle_s	s_applychanges_background;
static menuLabel_s		s_applychanges_header[3];
static menuAction_s		s_applychanges_yes_action;
static menuAction_s		s_applychanges_no_action;

//=======================================================================

static void M_AcceptChangesFunc (void *self)
{
	UI_PopMenu ();
	UI_ApplyMenuChanges ();
}

//=======================================================================

void Menu_ApplyChanges_Init (void)
{
	s_applychanges_popup.x				= SCREEN_WIDTH*0.5 - 30;
	s_applychanges_popup.y				= SCREEN_HEIGHT*0.5 - 60;
	s_applychanges_popup.nitems			= 0;
	s_applychanges_popup.isPopup		= true;
	s_applychanges_popup.background		= NULL;
	s_applychanges_popup.drawFunc		= UI_DefaultMenuDraw;
	s_applychanges_popup.keyFunc		= UI_DefaultMenuKey;
	s_applychanges_popup.canOpenFunc	= NULL;

	s_applychanges_background.generic.type		= MTYPE_RECTANGLE;
	s_applychanges_background.generic.x			= -150;
	s_applychanges_background.generic.y			= 0;
	s_applychanges_background.width				= 360;
	s_applychanges_background.height			= 120;
	s_applychanges_background.color[0]			= 0;
	s_applychanges_background.color[1]			= 0;
	s_applychanges_background.color[2]			= 0;
	s_applychanges_background.color[3]			= 224; // 192
	s_applychanges_background.border			= 1;
	s_applychanges_background.borderColor[0]	= 128;
	s_applychanges_background.borderColor[1]	= 128;
	s_applychanges_background.borderColor[2]	= 128;
	s_applychanges_background.borderColor[3]	= 255;
	s_applychanges_background.generic.isHidden	= false;

	s_applychanges_header[0].generic.type		= MTYPE_LABEL;
	s_applychanges_header[0].generic.textSize	= MENU_FONT_SIZE;
	s_applychanges_header[0].generic.name		= UI_GetApplyChangesMessage(0);
	s_applychanges_header[0].generic.x			= 30 + MENU_FONT_SIZE * 0.5 * strlen(s_applychanges_header[0].generic.name);
	s_applychanges_header[0].generic.y			= 20;

	s_applychanges_header[1].generic.type		= MTYPE_LABEL;
	s_applychanges_header[1].generic.textSize	= MENU_FONT_SIZE;
	s_applychanges_header[1].generic.name		= UI_GetApplyChangesMessage(1);
	s_applychanges_header[1].generic.x			= 30 + MENU_FONT_SIZE * 0.5 * strlen(s_applychanges_header[1].generic.name);
	s_applychanges_header[1].generic.y			= 32;

	s_applychanges_header[2].generic.type		= MTYPE_LABEL;
	s_applychanges_header[2].generic.textSize	= MENU_FONT_SIZE;
	s_applychanges_header[2].generic.name		= UI_GetApplyChangesMessage(2);
	s_applychanges_header[2].generic.x			= 30 + MENU_FONT_SIZE * 0.5 * strlen(s_applychanges_header[2].generic.name);
	s_applychanges_header[2].generic.y			= 56;

	s_applychanges_yes_action.generic.type			= MTYPE_ACTION;
	s_applychanges_yes_action.generic.textSize		= MENU_HEADER_FONT_SIZE;
	s_applychanges_yes_action.generic.flags			= QMF_LEFT_JUSTIFY;
	s_applychanges_yes_action.generic.x				= -MENU_FONT_SIZE*3;	// MENU_FONT_SIZE*5
	s_applychanges_yes_action.generic.y				= 90;
	s_applychanges_yes_action.generic.name			= "OK";
	s_applychanges_yes_action.generic.callback		= M_AcceptChangesFunc;

	s_applychanges_no_action.generic.type			= MTYPE_ACTION;
	s_applychanges_no_action.generic.textSize		= MENU_HEADER_FONT_SIZE;
	s_applychanges_no_action.generic.flags			= QMF_LEFT_JUSTIFY;
	s_applychanges_no_action.generic.x				= MENU_FONT_SIZE*10;	// MENU_FONT_SIZE*5
	s_applychanges_no_action.generic.y				= 90;
	s_applychanges_no_action.generic.name			= "Cancel";
	s_applychanges_no_action.generic.callback		= UI_BackMenu;

	UI_AddMenuItem (&s_applychanges_popup, (void *) &s_applychanges_background);
	UI_AddMenuItem (&s_applychanges_popup, (void *) &s_applychanges_header[0]);
	UI_AddMenuItem (&s_applychanges_popup, (void *) &s_applychanges_header[1]);
	UI_AddMenuItem (&s_applychanges_popup, (void *) &s_applychanges_header[2]);
	UI_AddMenuItem (&s_applychanges_popup, (void *) &s_applychanges_yes_action);
	UI_AddMenuItem (&s_applychanges_popup, (void *) &s_applychanges_no_action);
}


void Menu_ApplyChanges_f (void)
{
	Menu_ApplyChanges_Init ();
	UI_PushMenu (&s_applychanges_popup);
}
