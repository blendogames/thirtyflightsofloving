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

// menu_game_saveload.c -- the single save/load menus

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"
#include <sys/types.h>
#include <sys/stat.h>

static menuframework_s	s_loadgame_menu;
static menuaction_s		s_loadgame_actions[UI_MAX_SAVEGAMES];
static menuaction_s		s_loadgame_back_action;

static menuframework_s	s_savegame_menu;
static menuaction_s		s_savegame_actions[UI_MAX_SAVEGAMES];
static menuaction_s		s_savegame_back_action;

/*
=============================================================================

SAVESHOT HANDLING

=============================================================================
*/

#if 1
void UI_DrawMenuSaveshot (qboolean loadmenu)
{
	char	*shotname = NULL;
	int		i;

	if (loadmenu) {
		if ( (s_loadgame_menu.cursor < 0) || (s_loadgame_menu.cursor >= UI_MAX_SAVEGAMES))	// catch back action
			i = UI_MAX_SAVEGAMES;
		else
			i = s_loadgame_actions[s_loadgame_menu.cursor].generic.localdata[0];
	}
	else {	// save menu
		if ( (s_savegame_menu.cursor < 0) || (s_savegame_menu.cursor >= UI_MAX_SAVEGAMES-1))	// catch back action
			i = UI_MAX_SAVEGAMES;
		else
			i = s_savegame_actions[s_savegame_menu.cursor].generic.localdata[0];
	}
	shotname = UI_UpdateSaveshot (i);

	UI_DrawFill (SCREEN_WIDTH/2+44, SCREEN_HEIGHT/2-70, 244, 184, ALIGN_CENTER, false, 60,60,60,255);

	if (shotname)
		UI_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, false, shotname, 1.0);
	else
		UI_DrawFill (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, false, 0,0,0,255);
}
#else
void UI_DrawMenuSaveshot (qboolean loadmenu)
{
	char	shotname [MAX_QPATH];
	char	mapshotname [MAX_QPATH];
	int		i;
	
	if (loadmenu) {
		if ( (s_loadgame_menu.cursor < 0) || (s_loadgame_menu.cursor >= UI_MAX_SAVEGAMES))	// catch back action
			i = UI_MAX_SAVEGAMES;
		else
			i = s_loadgame_actions[s_loadgame_menu.cursor].generic.localdata[0];
	}
	else {	// save menu
		if ( (s_savegame_menu.cursor < 0) || (s_savegame_menu.cursor >= UI_MAX_SAVEGAMES-1))	// catch back action
			i = UI_MAX_SAVEGAMES;
		else
			i = s_savegame_actions[s_savegame_menu.cursor].generic.localdata[0];
	}

	UI_DrawFill (SCREEN_WIDTH/2+44, SCREEN_HEIGHT/2-70, 244, 184, ALIGN_CENTER, false, 60,60,60,255);

	if ( loadmenu && (i == 0) && ui_savevalid[i] && ui_saveshotvalid[i])	// m_mapshotvalid ) // autosave shows mapshot
	{
		Com_sprintf(mapshotname, sizeof(mapshotname), "/levelshots/%s.pcx", ui_mapname);

		UI_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, false, mapshotname, 1.0);
	}
	else if ( ui_savevalid[i] && ui_saveshotvalid[i] )
	{
	//	Com_sprintf(shotname, sizeof(shotname), "/save/kmq2save%03i/shot.jpg", i);
		Com_sprintf(shotname, sizeof(shotname), "/"SAVEDIRNAME"/kmq2save%03i/shot.jpg", i);

		UI_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, false, shotname, 1.0);
	}
	else if (ui_saveshotvalid[UI_MAX_SAVEGAMES])
		UI_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, false, "/gfx/ui/noscreen.pcx", 1.0);
	else
		UI_DrawFill (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, false, 0,0,0,255);
}
#endif

/*
=============================================================================

LOADGAME MENU

=============================================================================
*/

extern	char *load_saveshot;
char loadshotname[MAX_QPATH];

void LoadGameCallback (void *self)
{
	menuaction_s *a = (menuaction_s *) self;

	// set saveshot name here
	if ( ui_saveshotvalid[ a->generic.localdata[0] ] && (a->generic.localdata[0] != 0) )	// autosave has no saveshot, but uses levelshot instead
	{
	//	Com_sprintf(loadshotname, sizeof(loadshotname), "/save/kmq2save%03i/shot.jpg", a->generic.localdata[0]);
		Com_sprintf(loadshotname, sizeof(loadshotname), "/"SAVEDIRNAME"/kmq2save%03i/shot.jpg", a->generic.localdata[0]);
		load_saveshot = loadshotname; }
	else {
		load_saveshot = NULL;
	}

	if ( ui_savevalid[ a->generic.localdata[0] ] ) {
		Cbuf_AddText (va("load kmq2save%03i\n",  a->generic.localdata[0] ) );
		UI_ForceMenuOff ();
	}
}

//=======================================================================

void Menu_LoadGame_Init (void)
{
	int i;

	UI_UpdateSavegameData ();

	s_loadgame_menu.x = SCREEN_WIDTH*0.5 - 240;
	s_loadgame_menu.y = SCREEN_HEIGHT*0.5 - 68;
	s_loadgame_menu.nitems = 0;

//	Load_Savestrings ();

	for (i = 0; i < UI_MAX_SAVEGAMES; i++)
	{
		s_loadgame_actions[i].generic.name			= ui_savestrings[i];
		s_loadgame_actions[i].generic.flags			= QMF_LEFT_JUSTIFY;
		s_loadgame_actions[i].generic.localdata[0]	= i;
		s_loadgame_actions[i].generic.callback		= LoadGameCallback;

		s_loadgame_actions[i].generic.x = 0;
		s_loadgame_actions[i].generic.y = (i) * MENU_LINE_SIZE;
		if (i>0)	// separate from autosave
			s_loadgame_actions[i].generic.y += 10;

		s_loadgame_actions[i].generic.type = MTYPE_ACTION;
		s_loadgame_actions[i].generic.textSize = MENU_FONT_SIZE;

		UI_AddMenuItem (&s_loadgame_menu, &s_loadgame_actions[i]);
	}

	s_loadgame_back_action.generic.type		= MTYPE_ACTION;
	s_loadgame_back_action.generic.textSize	= MENU_FONT_SIZE;
	s_loadgame_back_action.generic.flags	= QMF_LEFT_JUSTIFY;
	s_loadgame_back_action.generic.x		= 0;
	s_loadgame_back_action.generic.y		= (UI_MAX_SAVEGAMES+3)*MENU_LINE_SIZE;
	s_loadgame_back_action.generic.name		= "Back";
	s_loadgame_back_action.generic.callback = UI_BackMenu;

	UI_AddMenuItem (&s_loadgame_menu, &s_loadgame_back_action);

//	ValidateSaveshots (true); // register saveshots
}

void Menu_LoadGame_Draw (void)
{
	UI_DrawBanner ("m_banner_load_game");
//	UI_AdjustMenuCursor (&s_loadgame_menu,  );
	UI_DrawMenu (&s_loadgame_menu);
	UI_DrawMenuSaveshot (true);
}

const char *Menu_LoadGame_Key (int key)
{
	if ( key == K_ESCAPE || key == K_ENTER )
	{
		s_savegame_menu.cursor = s_loadgame_menu.cursor - 1;
		if ( s_savegame_menu.cursor < 0 )
			s_savegame_menu.cursor = 0;
	}
	return UI_DefaultMenuKey (&s_loadgame_menu, key);
}

void Menu_LoadGame_f (void)
{
	Menu_LoadGame_Init ();
	UI_PushMenu (Menu_LoadGame_Draw, Menu_LoadGame_Key);
}


/*
=============================================================================

SAVEGAME MENU

=============================================================================
*/

void SaveGameCallback (void *self)
{
	menuaction_s *a = (menuaction_s *) self;

	Cbuf_AddText (va("save kmq2save%03i\n", a->generic.localdata[0] ));
	UI_ForceMenuOff ();
}

//=======================================================================

void Menu_SaveGame_Init (void)
{
	int i;

	UI_UpdateSavegameData ();

	s_savegame_menu.x = SCREEN_WIDTH*0.5 - 240;
	s_savegame_menu.y = SCREEN_HEIGHT*0.5 - 68;
	s_savegame_menu.nitems = 0;

//	Load_Savestrings ();

	// don't include the autosave slot
	for ( i = 0; i < UI_MAX_SAVEGAMES-1; i++ )
	{
		s_savegame_actions[i].generic.name = ui_savestrings[i+1];
		s_savegame_actions[i].generic.localdata[0] = i+1;
		s_savegame_actions[i].generic.flags = QMF_LEFT_JUSTIFY;
		s_savegame_actions[i].generic.callback = SaveGameCallback;

		s_savegame_actions[i].generic.x = 0;
		s_savegame_actions[i].generic.y = ( i ) * MENU_LINE_SIZE;

		s_savegame_actions[i].generic.type = MTYPE_ACTION;
		s_savegame_actions[i].generic.textSize = MENU_FONT_SIZE;

		UI_AddMenuItem (&s_savegame_menu, &s_savegame_actions[i]);
	}
	s_savegame_back_action.generic.type	= MTYPE_ACTION;
	s_savegame_back_action.generic.textSize	= MENU_FONT_SIZE;
	s_savegame_back_action.generic.flags  = QMF_LEFT_JUSTIFY;
	s_savegame_back_action.generic.x		= 0;
	s_savegame_back_action.generic.y		= (UI_MAX_SAVEGAMES+1)*MENU_LINE_SIZE;
	s_savegame_back_action.generic.name		= "Back";
	s_savegame_back_action.generic.callback = UI_BackMenu;

	UI_AddMenuItem (&s_savegame_menu, &s_savegame_back_action);

//	ValidateSaveshots (false);
}

void Menu_SaveGame_Draw (void)
{
	UI_DrawBanner ("m_banner_save_game");
	UI_AdjustMenuCursor (&s_savegame_menu, 1);
	UI_DrawMenu (&s_savegame_menu);
	UI_DrawMenuSaveshot (false);
}

const char *Menu_SaveGame_Key (int key)
{
	if ( key == K_ENTER || key == K_ESCAPE )
	{
		s_loadgame_menu.cursor = s_savegame_menu.cursor - 1;
		if ( s_loadgame_menu.cursor < 0 )
			s_loadgame_menu.cursor = 0;
	}
	return UI_DefaultMenuKey (&s_savegame_menu, key);
}

void Menu_SaveGame_f (void)
{
	if ( !Com_ServerState() )
		return;		// not playing a game

	Menu_SaveGame_Init ();
	UI_PushMenu (Menu_SaveGame_Draw, Menu_SaveGame_Key);
//	Load_Savestrings ();
}
