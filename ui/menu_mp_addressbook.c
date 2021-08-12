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

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"


/*
=============================================================================

ADDRESS BOOK MENU

=============================================================================
*/
#define NUM_ADDRESSBOOK_ENTRIES 12 // was 9

static menuframework_s	s_addressbook_menu;
static menufield_s		s_addressbook_fields[NUM_ADDRESSBOOK_ENTRIES];
static menuaction_s		s_addressbook_back_action;


void AddressBook_SaveEntries (void)
{
	int index;
	char buffer[20];

	for (index = 0; index < NUM_ADDRESSBOOK_ENTRIES; index++)
	{
		Com_sprintf(buffer, sizeof(buffer), "adr%d", index );
		Cvar_Set (buffer, s_addressbook_fields[index].buffer);
	}
}

static void AddressBookBack (void *unused)
{
	AddressBook_SaveEntries ();
	UI_BackMenu (unused);
}

void Menu_AddressBook_Init (void)
{
	int i;

	s_addressbook_menu.x = SCREEN_WIDTH*0.5 - 142;
	s_addressbook_menu.y = SCREEN_HEIGHT*0.5 - 76; // was 58
	s_addressbook_menu.nitems = 0;

	for (i = 0; i < NUM_ADDRESSBOOK_ENTRIES; i++)
	{
		cvar_t *adr;
		char buffer[20];

		Com_sprintf (buffer, sizeof(buffer), "adr%d", i);

		adr = Cvar_Get( buffer, "", CVAR_ARCHIVE );

		s_addressbook_fields[i].generic.type = MTYPE_FIELD;
		s_addressbook_fields[i].generic.textSize = MENU_FONT_SIZE;
		s_addressbook_fields[i].generic.name = 0;
		s_addressbook_fields[i].generic.callback = 0;
		s_addressbook_fields[i].generic.x		= 0;
		s_addressbook_fields[i].generic.y		= i * 2.25*MENU_LINE_SIZE;
		s_addressbook_fields[i].generic.localdata[0] = i;
		s_addressbook_fields[i].length			= 60;
		s_addressbook_fields[i].visible_length	= 30;
	//	strncpy(s_addressbook_fields[i].buffer, adr->string);
		Q_strncpyz (s_addressbook_fields[i].buffer, sizeof(s_addressbook_fields[i].buffer), adr->string);
		s_addressbook_fields[i].cursor = (int)strlen(adr->string);

		UI_AddMenuItem (&s_addressbook_menu, &s_addressbook_fields[i]);
	}

	s_addressbook_back_action.generic.type = MTYPE_ACTION;
	s_addressbook_back_action.generic.textSize = MENU_FONT_SIZE;
	s_addressbook_back_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_addressbook_back_action.generic.x	= 24;
	s_addressbook_back_action.generic.y	= (NUM_ADDRESSBOOK_ENTRIES*2.25+0.5)*MENU_LINE_SIZE;
	s_addressbook_back_action.generic.name	= " back";
	s_addressbook_back_action.generic.callback = AddressBookBack; // UI_BackMenu;

	UI_AddMenuItem (&s_addressbook_menu, &s_addressbook_back_action);
}


const char *Menu_AddressBook_Key (int key)
{
	if (key == K_ESCAPE)
		AddressBook_SaveEntries ();

	return UI_DefaultMenuKey (&s_addressbook_menu, key);
}

void Menu_AddressBook_Draw (void)
{
	UI_DrawBanner ("m_banner_addressbook");
	UI_DrawMenu (&s_addressbook_menu);
}

void Menu_AddressBook_f(void)
{
	Menu_AddressBook_Init ();
	UI_PushMenu (Menu_AddressBook_Draw, Menu_AddressBook_Key);
}
