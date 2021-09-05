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

// menu_mp_startserver.c -- the start server menu 

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"


/*
=============================================================================

START SERVER MENU

=============================================================================
*/
static menuframework_s s_startserver_menu;

static menuaction_s	s_startserver_start_action;
static menuaction_s	s_startserver_dmoptions_action;
static menufield_s	s_timelimit_field;
static menufield_s	s_fraglimit_field;
static menufield_s	s_maxclients_field;
static menufield_s	s_hostname_field;
static menulist_s	s_startmap_list;
menulist_s			s_rules_box;
static menulist_s	s_dedicated_box;

static menuaction_s	s_startserver_back_action;

//=============================================================================

/*
===============
M_RefreshMapList
===============
*/
void M_RefreshMapList (maptype_t maptype)
{
	int		i;

	if (maptype == ui_svr_maptype) // no change
		return;

	// load map list
	UI_UpdateMapList (maptype);

	// reset startmap if it's in the part of the list that changed
	if (s_startmap_list.curValue >= ui_svr_listfile_nummaps)
		s_startmap_list.curValue = 0;

	s_startmap_list.itemNames = ui_svr_mapnames;
	for (i=0; s_startmap_list.itemNames[i]; i++);
	s_startmap_list.numItems = i;
}


//=============================================================================

void M_DMOptionsFunc (void *self)
{
	if (s_rules_box.curValue == 1)
		return;
	Menu_DMOptions_f ();
}

void M_RulesChangeFunc (void *self)
{
	maptype_t	maptype = MAP_DM;

	UI_SetCoopMenuMode (false);
	UI_SetCTFMenuMode (false);
	if (s_rules_box.curValue == 0) 	// DM
	{
		s_maxclients_field.generic.statusbar = NULL;
		if (atoi(s_maxclients_field.buffer) <= 8) // set default of 8
		//	strncpy( s_maxclients_field.buffer, "8" );
			Q_strncpyz (s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), "8");
		s_startserver_dmoptions_action.generic.statusbar = NULL;
		maptype = MAP_DM;
	}
	else if (s_rules_box.curValue == 1)		// coop				// PGM
	{
		s_maxclients_field.generic.statusbar = "4 maximum for cooperative";
		if (atoi(s_maxclients_field.buffer) > 4)
		//	strncpy( s_maxclients_field.buffer, "4" );
			Q_strncpyz (s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), "4");
		s_startserver_dmoptions_action.generic.statusbar = "N/A for cooperative";
		maptype = MAP_COOP;
		UI_SetCoopMenuMode (true);
	}
	else if (s_rules_box.curValue == 2) // CTF
	{
		s_maxclients_field.generic.statusbar = NULL;
		if (atoi(s_maxclients_field.buffer) <= 12) // set default of 12
		//	strncpy( s_maxclients_field.buffer, "12" );
			Q_strncpyz (s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), "12");
		s_startserver_dmoptions_action.generic.statusbar = NULL;
		maptype = MAP_CTF;
		UI_SetCTFMenuMode (true);
	}
	else if (s_rules_box.curValue == 3) // 3Team CTF
	{
		s_maxclients_field.generic.statusbar = NULL;
		if (atoi(s_maxclients_field.buffer) <= 18) // set default of 18
		//	strncpy( s_maxclients_field.buffer, "18" );
			Q_strncpyz (s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), "18");
		s_startserver_dmoptions_action.generic.statusbar = NULL;
		maptype = MAP_3TCTF;
		UI_SetCTFMenuMode (true);
	}
	// ROGUE GAMES
	else if (FS_RoguePath() && s_rules_box.curValue == 4) // tag	
	{
		s_maxclients_field.generic.statusbar = NULL;
		if (atoi(s_maxclients_field.buffer) <= 8) // set default of 8
		//	strncpy( s_maxclients_field.buffer, "8" );
			Q_strncpyz (s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), "8");
		s_startserver_dmoptions_action.generic.statusbar = NULL;
		maptype = MAP_DM;
	}

	M_RefreshMapList (maptype);
}

void Menu_StartServerActionFunc (void *self)
{
	char	startmap[1024];
	int		timelimit;
	int		fraglimit;
	int		maxclients;

	Q_strncpyz (startmap, sizeof(startmap), strchr( ui_svr_mapnames[s_startmap_list.curValue], '\n' ) + 1);

	maxclients  = atoi( s_maxclients_field.buffer );
	timelimit	= atoi( s_timelimit_field.buffer );
	fraglimit	= atoi( s_fraglimit_field.buffer );

	Cvar_SetValue( "maxclients", ClampCvar( 0, maxclients, maxclients ) );
	Cvar_SetValue ("timelimit", ClampCvar( 0, timelimit, timelimit ) );
	Cvar_SetValue ("fraglimit", ClampCvar( 0, fraglimit, fraglimit ) );
	Cvar_Set("hostname", s_hostname_field.buffer );

	Cvar_SetValue ("deathmatch", s_rules_box.curValue != 1);
	Cvar_SetValue ("coop", s_rules_box.curValue == 1);
	Cvar_SetValue ("ctf", s_rules_box.curValue == 2);
	Cvar_SetValue ("ttctf", s_rules_box.curValue == 3);
	Cvar_SetValue ("gamerules", FS_RoguePath() ? ((s_rules_box.curValue == 4) ? 2 : 0) : 0);

	UI_StartServer (startmap, (s_dedicated_box.curValue != 0));
}

//=======================================================================

void Menu_StartServer_Init (void)
{
	static const char *yesno_names[] =
	{
		"no",
		"yes",
		0
	};
	static const char *dm_coop_names[] =
	{
		"deathmatch",
		"cooperative",
		"CTF",
		"3Team CTF",
		0
	};
	static const char *dm_coop_names_rogue[] =
	{
		"deathmatch",
		"cooperative",
		"CTF",
		"3Team CTF",
		"tag",
		0
	};

	int y = 0;
	
	//
	// initialize the menu stuff
	//
	s_startserver_menu.x = SCREEN_WIDTH*0.5 - 140;
	//s_startserver_menu.y = 0;
	s_startserver_menu.nitems = 0;

	s_startmap_list.generic.type		= MTYPE_SPINCONTROL;
	s_startmap_list.generic.textSize	= MENU_FONT_SIZE;
	s_startmap_list.generic.x			= 0;
	s_startmap_list.generic.y			= y;
	s_startmap_list.generic.name		= "initial map";
	s_startmap_list.itemNames			= ui_svr_mapnames;

	s_rules_box.generic.type		= MTYPE_SPINCONTROL;
	s_rules_box.generic.textSize	= MENU_FONT_SIZE;
	s_rules_box.generic.x			= 0;
	s_rules_box.generic.y			= y += 2*MENU_LINE_SIZE;
	s_rules_box.generic.name		= "rules";
//PGM - rogue games only available with rogue DLL.
	if ( FS_RoguePath() )
		s_rules_box.itemNames		= dm_coop_names_rogue;
	else
		s_rules_box.itemNames		= dm_coop_names;
//PGM
	if (Cvar_VariableValue("ttctf"))
		s_rules_box.curValue = 3;
	else if (Cvar_VariableValue("ctf"))
		s_rules_box.curValue = 2;
	else if (FS_RoguePath() && Cvar_VariableValue("gamerules") == 2)
		s_rules_box.curValue = 4;
	else if (Cvar_VariableValue("coop"))
		s_rules_box.curValue = 1;
	else
		s_rules_box.curValue = 0;
	s_rules_box.generic.callback	= M_RulesChangeFunc;

	s_timelimit_field.generic.type		= MTYPE_FIELD;
	s_timelimit_field.generic.textSize	= MENU_FONT_SIZE;
	s_timelimit_field.generic.name		= "time limit";
	s_timelimit_field.generic.flags		= QMF_NUMBERSONLY;
	s_timelimit_field.generic.x			= 0;
	s_timelimit_field.generic.y			= y += 2*MENU_FONT_SIZE;
	s_timelimit_field.generic.statusbar	= "0 = no limit";
	s_timelimit_field.length			= 4;
	s_timelimit_field.visible_length	= 4;
	Q_strncpyz (s_timelimit_field.buffer, sizeof(s_timelimit_field.buffer), Cvar_VariableString("timelimit"));
	s_timelimit_field.cursor			= (int)strlen( s_timelimit_field.buffer );

	s_fraglimit_field.generic.type		= MTYPE_FIELD;
	s_fraglimit_field.generic.textSize	= MENU_FONT_SIZE;
	s_fraglimit_field.generic.name		= "frag limit";
	s_fraglimit_field.generic.flags		= QMF_NUMBERSONLY;
	s_fraglimit_field.generic.x			= 0;
	s_fraglimit_field.generic.y			= y += 2.25*MENU_FONT_SIZE;
	s_fraglimit_field.generic.statusbar	= "0 = no limit";
	s_fraglimit_field.length			= 4;
	s_fraglimit_field.visible_length	= 4;
	Q_strncpyz (s_fraglimit_field.buffer, sizeof(s_fraglimit_field.buffer), Cvar_VariableString("fraglimit"));
	s_fraglimit_field.cursor			= (int)strlen( s_fraglimit_field.buffer );

	/*
	** maxclients determines the maximum number of players that can join
	** the game.  If maxclients is only "1" then we should default the menu
	** option to 8 players, otherwise use whatever its current value is. 
	** Clamping will be done when the server is actually started.
	*/
	s_maxclients_field.generic.type			= MTYPE_FIELD;
	s_maxclients_field.generic.textSize		= MENU_FONT_SIZE;
	s_maxclients_field.generic.name			= "max players";
	s_maxclients_field.generic.flags		= QMF_NUMBERSONLY;
	s_maxclients_field.generic.x			= 0;
	s_maxclients_field.generic.y			= y += 2.25*MENU_FONT_SIZE;
	s_maxclients_field.generic.statusbar	= NULL;
	s_maxclients_field.length				= 3;
	s_maxclients_field.visible_length		= 3;
	if ( Cvar_VariableValue( "maxclients" ) == 1 )
		Q_strncpyz (s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), "8");
	else 
		Q_strncpyz (s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), Cvar_VariableString("maxclients"));
	s_maxclients_field.cursor				= (int)strlen( s_maxclients_field.buffer );

	s_hostname_field.generic.type			= MTYPE_FIELD;
	s_hostname_field.generic.textSize		= MENU_FONT_SIZE;
	s_hostname_field.generic.name			= "hostname";
	s_hostname_field.generic.flags			= 0;
	s_hostname_field.generic.x				= 0;
	s_hostname_field.generic.y				= y += 2.25*MENU_FONT_SIZE;
	s_hostname_field.generic.statusbar		= NULL;
	s_hostname_field.length					= 16;
	s_hostname_field.visible_length			= 16;
	Q_strncpyz (s_hostname_field.buffer, sizeof(s_hostname_field.buffer), Cvar_VariableString("hostname"));
	s_hostname_field.cursor					= (int)strlen( s_hostname_field.buffer );

	s_dedicated_box.generic.type			= MTYPE_SPINCONTROL;
	s_dedicated_box.generic.textSize		= MENU_FONT_SIZE;
	s_dedicated_box.generic.name			= "dedicated server";;
	s_dedicated_box.generic.x				= 0;
	s_dedicated_box.generic.y				= y += 2*MENU_FONT_SIZE;
	s_dedicated_box.curValue				= 0;	// always start off
	s_dedicated_box.generic.statusbar		= "makes the server faster, but you can't play on this computer";
	s_dedicated_box.itemNames				= yesno_names;

	s_startserver_dmoptions_action.generic.type			= MTYPE_ACTION;
	s_startserver_dmoptions_action.generic.textSize		= MENU_FONT_SIZE;
	s_startserver_dmoptions_action.generic.name			= "Deathmatch Flags";
	s_startserver_dmoptions_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_startserver_dmoptions_action.generic.x			= 4*MENU_FONT_SIZE;
	s_startserver_dmoptions_action.generic.y			= y += 2*MENU_FONT_SIZE;
	s_startserver_dmoptions_action.generic.statusbar	= NULL;
	s_startserver_dmoptions_action.generic.callback		= M_DMOptionsFunc;

	s_startserver_start_action.generic.type		= MTYPE_ACTION;
	s_startserver_start_action.generic.textSize	= MENU_FONT_SIZE;
	s_startserver_start_action.generic.name		= "Begin";
	s_startserver_start_action.generic.flags	= QMF_LEFT_JUSTIFY;
	s_startserver_start_action.generic.x		= 4*MENU_FONT_SIZE;
	s_startserver_start_action.generic.y		= y += 2*MENU_LINE_SIZE;
	s_startserver_start_action.generic.callback	= Menu_StartServerActionFunc;

	s_startserver_back_action.generic.type		= MTYPE_ACTION;
	s_startserver_back_action.generic.textSize	= MENU_FONT_SIZE;
	s_startserver_back_action.generic.name		= "Back to Multiplayer";
	s_startserver_back_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_startserver_back_action.generic.x			= 4*MENU_FONT_SIZE;
	s_startserver_back_action.generic.y			= y += 3*MENU_LINE_SIZE;
	s_startserver_back_action.generic.callback	= UI_BackMenu;

	UI_AddMenuItem (&s_startserver_menu, &s_startmap_list);
	UI_AddMenuItem (&s_startserver_menu, &s_rules_box);
	UI_AddMenuItem (&s_startserver_menu, &s_timelimit_field);
	UI_AddMenuItem (&s_startserver_menu, &s_fraglimit_field);
	UI_AddMenuItem (&s_startserver_menu, &s_maxclients_field);
	UI_AddMenuItem (&s_startserver_menu, &s_hostname_field);
	UI_AddMenuItem (&s_startserver_menu, &s_dedicated_box);
	UI_AddMenuItem (&s_startserver_menu, &s_startserver_dmoptions_action);
	UI_AddMenuItem (&s_startserver_menu, &s_startserver_start_action);
	UI_AddMenuItem (&s_startserver_menu, &s_startserver_back_action);

	UI_CenterMenu (&s_startserver_menu);

	// call this now to set proper inital state
	M_RulesChangeFunc (NULL);
}

void Menu_DrawStartSeverLevelshot (void)
{
	char *mapshotname = UI_UpdateStartSeverLevelshot (s_startmap_list.curValue);

	UI_DrawFill (SCREEN_WIDTH/2+44, SCREEN_HEIGHT/2-70, 244, 184, ALIGN_CENTER, false, 60,60,60,255);

	if (mapshotname)
		UI_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, false, mapshotname, 1.0);
	else
		UI_DrawFill (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, false, 0,0,0,255);
}

void Menu_StartServer_Draw (void)
{
	UI_DrawBanner ("m_banner_start_server"); // Knightmare added
	UI_DrawMenu (&s_startserver_menu);
	Menu_DrawStartSeverLevelshot (); // added levelshots
}

const char *Menu_StartServer_Key (int key)
{
	return UI_DefaultMenuKey (&s_startserver_menu, key);
}

void Menu_StartServer_f (void)
{
	Menu_StartServer_Init ();
	UI_PushMenu (Menu_StartServer_Draw, Menu_StartServer_Key);
}
