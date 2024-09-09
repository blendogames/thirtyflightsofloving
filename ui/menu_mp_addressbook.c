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

// menu_mp_addressbook.c -- the address book menu 

#include "../client/client.h"
#include "ui_local.h"

/*
=============================================================================

ADDRESS BOOK MENU

=============================================================================
*/
#define NUM_ADDRESSBOOK_ENTRIES 12 // was 9

static menuFramework_s	s_addressbook_menu;
static menuImage_s		s_addressbook_banner;
static menuField_s		s_addressbook_fields[NUM_ADDRESSBOOK_ENTRIES];
static menuAction_s		s_addressbook_back_action;

//=========================================================

void Menu_AddressBook_Init (void)
{
	int			i, x, y;
	static char	adrCvarNames[NUM_ADDRESSBOOK_ENTRIES][6];

	// menu.x = 178, menu.y = 164
	x = SCREEN_WIDTH*0.5 - 142;
	y = SCREEN_HEIGHT*0.5 - 76; // was 58

	s_addressbook_menu.x			= 0;	// SCREEN_WIDTH*0.5 - 142;
	s_addressbook_menu.y			= 0;	// SCREEN_HEIGHT*0.5 - 76;
	s_addressbook_menu.nitems		= 0;
	s_addressbook_menu.isPopup		= false;
	s_addressbook_menu.background	= NULL;
	s_addressbook_menu.drawFunc		= UI_DefaultMenuDraw;
	s_addressbook_menu.keyFunc		= UI_DefaultMenuKey;
	s_addressbook_menu.canOpenFunc	= NULL;

	s_addressbook_banner.generic.type		= MTYPE_IMAGE;
	s_addressbook_banner.generic.x			= 0;
	s_addressbook_banner.generic.y			= 9*MENU_LINE_SIZE;
	s_addressbook_banner.width				= 275;
	s_addressbook_banner.height				= 32;
	s_addressbook_banner.imageName			= "/pics/m_banner_addressbook.pcx";
	s_addressbook_banner.alpha				= 255;
	s_addressbook_banner.border				= 0;
	s_addressbook_banner.hCentered			= true;
	s_addressbook_banner.vCentered			= false;
	s_addressbook_banner.useAspectRatio		= false;
	s_addressbook_banner.generic.isHidden	= false;

	for (i = 0; i < NUM_ADDRESSBOOK_ENTRIES; i++)
	{
		Com_sprintf (adrCvarNames[i], sizeof(adrCvarNames[i]), "adr%d", i);

		s_addressbook_fields[i].generic.type			= MTYPE_FIELD;
		s_addressbook_fields[i].generic.textSize		= MENU_FONT_SIZE;
		s_addressbook_fields[i].generic.name			= 0;
		s_addressbook_fields[i].generic.callback		= 0;
		s_addressbook_fields[i].generic.x				= x;
		s_addressbook_fields[i].generic.y				= y + i * 2.25*MENU_LINE_SIZE;
		s_addressbook_fields[i].length					= 60;
		s_addressbook_fields[i].visible_length			= 30;
		s_addressbook_fields[i].generic.cvar			= adrCvarNames[i];
	}

	s_addressbook_back_action.generic.type = MTYPE_ACTION;
	s_addressbook_back_action.generic.textSize = MENU_FONT_SIZE;
	s_addressbook_back_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_addressbook_back_action.generic.x	= x + 24;
	s_addressbook_back_action.generic.y	= y + (NUM_ADDRESSBOOK_ENTRIES*2.25+0.5)*MENU_LINE_SIZE;
	s_addressbook_back_action.generic.name	= "Back";
	s_addressbook_back_action.generic.callback = UI_BackMenu;

	UI_AddMenuItem (&s_addressbook_menu, &s_addressbook_banner);
	for (i = 0; i < NUM_ADDRESSBOOK_ENTRIES; i++)
		UI_AddMenuItem (&s_addressbook_menu, &s_addressbook_fields[i]);
	UI_AddMenuItem (&s_addressbook_menu, &s_addressbook_back_action);
}


void Menu_AddressBook_f(void)
{
	Menu_AddressBook_Init ();
	UI_PushMenu (&s_addressbook_menu);
}
