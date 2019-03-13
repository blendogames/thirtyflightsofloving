/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// ui_addressbook.c -- the address book menu 

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

void AddressBook_MenuInit( void )
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
		s_addressbook_fields[i].generic.name = 0;
		s_addressbook_fields[i].generic.callback = 0;
		s_addressbook_fields[i].generic.x		= 0;
		s_addressbook_fields[i].generic.y		= i * 2.25*MENU_LINE_SIZE;
		s_addressbook_fields[i].generic.localdata[0] = i;
		s_addressbook_fields[i].length			= 60;
		s_addressbook_fields[i].visible_length	= 30;
	//	strncpy(s_addressbook_fields[i].buffer, adr->string);
		Q_strncpyz(s_addressbook_fields[i].buffer, adr->string, sizeof(s_addressbook_fields[i].buffer));
		s_addressbook_fields[i].cursor = strlen(adr->string);

		Menu_AddItem(&s_addressbook_menu, &s_addressbook_fields[i]);
	}

	s_addressbook_back_action.generic.type = MTYPE_ACTION;
	s_addressbook_back_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_addressbook_back_action.generic.x	= 24;
	s_addressbook_back_action.generic.y	= (NUM_ADDRESSBOOK_ENTRIES*2.25+0.5)*MENU_LINE_SIZE;
	s_addressbook_back_action.generic.name	= " back";
	s_addressbook_back_action.generic.callback = AddressBookBack; // UI_BackMenu;

	Menu_AddItem(&s_addressbook_menu, &s_addressbook_back_action);
}


const char *AddressBook_MenuKey (int key)
{
	if (key == K_ESCAPE)
		AddressBook_SaveEntries ();

	return Default_MenuKey (&s_addressbook_menu, key);
}

void AddressBook_MenuDraw (void)
{
	Menu_DrawBanner ("m_banner_addressbook");
	Menu_Draw (&s_addressbook_menu);
}

void M_Menu_AddressBook_f(void)
{
	AddressBook_MenuInit();
	UI_PushMenu( AddressBook_MenuDraw, AddressBook_MenuKey );
}
