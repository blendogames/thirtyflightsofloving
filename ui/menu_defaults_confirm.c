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

// menu_defaults_confirm.c -- reset defaults confirm popup menu

#include "../client/client.h"
#include "ui_local.h"

/*
=======================================================================

DEFAULTS CONFIRM MENU

=======================================================================
*/

static menuFramework_s	s_defaults_popup;
static menuRectangle_s	s_defaults_background;
static menuImage_s		s_defaults_banner;
static menuLabel_s		s_defaults_header;
static menuAction_s		s_defaults_yes_action;
static menuAction_s		s_defaults_no_action;

//=======================================================================

static void M_DefaultsApplyFunc (void *self)
{
	UI_PopMenu ();
	UI_SetMenuDefaults ();
}

//=======================================================================

void Menu_DefaultsConfirm_Init (void)
{
	s_defaults_popup.x				= SCREEN_WIDTH*0.5 - 30;
	s_defaults_popup.y				= SCREEN_HEIGHT*0.5 - 60;
	s_defaults_popup.nitems			= 0;
	s_defaults_popup.isPopup		= true;
	s_defaults_popup.background		= NULL;
	s_defaults_popup.drawFunc		= UI_DefaultMenuDraw;
	s_defaults_popup.keyFunc		= UI_DefaultMenuKey;
	s_defaults_popup.canOpenFunc	= NULL;

	s_defaults_background.generic.type		= MTYPE_RECTANGLE;
	s_defaults_background.generic.x			= -150;
	s_defaults_background.generic.y			= 0;
	s_defaults_background.width				= 360;
	s_defaults_background.height			= 120;
	s_defaults_background.color[0]			= 0;
	s_defaults_background.color[1]			= 0;
	s_defaults_background.color[2]			= 0;
	s_defaults_background.color[3]			= 224; // 192
	s_defaults_background.border			= 1;
	s_defaults_background.borderColor[0]	= 128;
	s_defaults_background.borderColor[1]	= 128;
	s_defaults_background.borderColor[2]	= 128;
	s_defaults_background.borderColor[3]	= 255;
	s_defaults_background.generic.isHidden	= false;

	s_defaults_banner.generic.type		= MTYPE_IMAGE;
	s_defaults_banner.generic.x			= 0;
	s_defaults_banner.generic.y			= 20;
	s_defaults_banner.width				= 212;
	s_defaults_banner.height			= 25;
	s_defaults_banner.imageName			= "/pics/areyousure.pcx";
	s_defaults_banner.alpha				= 255;
	s_defaults_banner.border			= 0;
	s_defaults_banner.hCentered			= true;
	s_defaults_banner.vCentered			= false;
	s_defaults_banner.useAspectRatio	= false;
	s_defaults_banner.generic.isHidden	= false;

	s_defaults_header.generic.type		= MTYPE_LABEL;
	s_defaults_header.generic.textSize	= MENU_FONT_SIZE;
	s_defaults_header.generic.name		= UI_GetDefaultsMessage();
	s_defaults_header.generic.x			= 30 + MENU_FONT_SIZE * 0.5 * strlen(s_defaults_header.generic.name);
	s_defaults_header.generic.y			= 60;

	s_defaults_yes_action.generic.type		= MTYPE_ACTION;
	s_defaults_yes_action.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_defaults_yes_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_defaults_yes_action.generic.x			= -MENU_FONT_SIZE*3;	// MENU_FONT_SIZE*5
	s_defaults_yes_action.generic.y			= 90;
	s_defaults_yes_action.generic.name		= "Yes";
	s_defaults_yes_action.generic.callback	= M_DefaultsApplyFunc;

	s_defaults_no_action.generic.type		= MTYPE_ACTION;
	s_defaults_no_action.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_defaults_no_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_defaults_no_action.generic.x			= MENU_FONT_SIZE*12;		// MENU_FONT_SIZE*5
	s_defaults_no_action.generic.y			= 90;
	s_defaults_no_action.generic.name		= "No";
	s_defaults_no_action.generic.callback	= UI_BackMenu;

	UI_AddMenuItem (&s_defaults_popup, (void *) &s_defaults_background);
	UI_AddMenuItem (&s_defaults_popup, (void *) &s_defaults_banner);
	UI_AddMenuItem (&s_defaults_popup, (void *) &s_defaults_header);
	UI_AddMenuItem (&s_defaults_popup, (void *) &s_defaults_yes_action);
	UI_AddMenuItem (&s_defaults_popup, (void *) &s_defaults_no_action);
}


void Menu_DefaultsConfirm_f (void)
{
	Menu_DefaultsConfirm_Init ();
	UI_PushMenu (&s_defaults_popup);
}
