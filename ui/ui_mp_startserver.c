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

// ui_mp_startserver.c -- the start server menu 

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

#if 0
/*
===============
UI_BuildMapList
===============
*/
void UI_BuildMapList (maptype_t maptype)
{
	int		i;

	if (ui_svr_mapnames)	free (ui_svr_mapnames);
	ui_svr_nummaps = ui_svr_listfile_nummaps + ui_svr_arena_nummaps[maptype];
	ui_svr_mapnames = malloc( sizeof( char * ) * ( ui_svr_nummaps + 1 ) );
	memset( ui_svr_mapnames, 0, sizeof( char * ) * ( ui_svr_nummaps + 1 ) );

	for (i = 0; i < ui_svr_nummaps; i++)
	{
		if (i < ui_svr_listfile_nummaps)
			ui_svr_mapnames[i] = ui_svr_listfile_mapnames[i];
		else
			ui_svr_mapnames[i] = ui_svr_arena_mapnames[maptype][i-ui_svr_listfile_nummaps];
	}
	ui_svr_mapnames[ui_svr_nummaps] = 0;
	ui_svr_maptype = maptype;

	if (s_startmap_list.curvalue >= ui_svr_nummaps) // paranoia
		s_startmap_list.curvalue = 0;
}	


/*
===============
UI_RefreshMapList
===============
*/
void UI_RefreshMapList (maptype_t maptype)
{
	int		i;

	if (maptype == ui_svr_maptype) // no change
		return;

	// reset startmap if it's in the part of the list that changed
	if (s_startmap_list.curvalue >= ui_svr_listfile_nummaps)
		s_startmap_list.curvalue = 0;

	UI_BuildMapList (maptype);
	s_startmap_list.itemnames = ui_svr_mapnames;
	for (i=0; s_startmap_list.itemnames[i]; i++);
	s_startmap_list.numitemnames = i;

	// levelshot found table
	if (ui_svr_mapshotvalid)	free(ui_svr_mapshotvalid);
	ui_svr_mapshotvalid = malloc( sizeof( byte ) * ( ui_svr_nummaps + 1 ) );
	memset( ui_svr_mapshotvalid, 0, sizeof( byte ) * ( ui_svr_nummaps + 1 ) );
	// register null levelshot
	if (ui_svr_mapshotvalid[ui_svr_nummaps] == M_UNSET) {	
		if (R_DrawFindPic("/gfx/ui/noscreen.pcx"))
			ui_svr_mapshotvalid[ui_svr_nummaps] = M_FOUND;
		else
			ui_svr_mapshotvalid[ui_svr_nummaps] = M_MISSING;
	}
}
#endif

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
	if (s_startmap_list.curvalue >= ui_svr_listfile_nummaps)
		s_startmap_list.curvalue = 0;

	s_startmap_list.itemnames = ui_svr_mapnames;
	for (i=0; s_startmap_list.itemnames[i]; i++);
	s_startmap_list.numitemnames = i;
}


//=============================================================================

void DMOptionsFunc (void *self)
{
	if (s_rules_box.curvalue == 1)
		return;
	M_Menu_DMOptions_f();
}

void RulesChangeFunc (void *self)
{
	maptype_t	maptype = MAP_DM;

	UI_SetCoopMenuMode (false);
	UI_SetCTFMenuMode (false);
	if (s_rules_box.curvalue == 0) 	// DM
	{
		s_maxclients_field.generic.statusbar = NULL;
		if (atoi(s_maxclients_field.buffer) <= 8) // set default of 8
		//	strncpy( s_maxclients_field.buffer, "8" );
			Q_strncpyz (s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), "8");
		s_startserver_dmoptions_action.generic.statusbar = NULL;
		maptype = MAP_DM;
	}
	else if (s_rules_box.curvalue == 1)		// coop				// PGM
	{
		s_maxclients_field.generic.statusbar = "4 maximum for cooperative";
		if (atoi(s_maxclients_field.buffer) > 4)
		//	strncpy( s_maxclients_field.buffer, "4" );
			Q_strncpyz (s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), "4");
		s_startserver_dmoptions_action.generic.statusbar = "N/A for cooperative";
		maptype = MAP_COOP;
		UI_SetCoopMenuMode (true);
	}
	else if (s_rules_box.curvalue == 2) // CTF
	{
		s_maxclients_field.generic.statusbar = NULL;
		if (atoi(s_maxclients_field.buffer) <= 12) // set default of 12
		//	strncpy( s_maxclients_field.buffer, "12" );
			Q_strncpyz (s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), "12");
		s_startserver_dmoptions_action.generic.statusbar = NULL;
		maptype = MAP_CTF;
		UI_SetCTFMenuMode (true);
	}
	else if (s_rules_box.curvalue == 3) // 3Team CTF
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
	else if (FS_RoguePath() && s_rules_box.curvalue == 4) // tag	
	{
		s_maxclients_field.generic.statusbar = NULL;
		if (atoi(s_maxclients_field.buffer) <= 8) // set default of 8
		//	strncpy( s_maxclients_field.buffer, "8" );
			Q_strncpyz (s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), "8");
		s_startserver_dmoptions_action.generic.statusbar = NULL;
		maptype = MAP_DM;
	}

//	UI_RefreshMapList (maptype);
	M_RefreshMapList (maptype);
}

void M_StartServerActionFunc (void *self)
{
	char	startmap[1024];
	int		timelimit;
	int		fraglimit;
	int		maxclients;
//	char	*spot;

//	strncpy (startmap, strchr( ui_svr_mapnames[s_startmap_list.curvalue], '\n' ) + 1);
	Q_strncpyz (startmap, sizeof(startmap), strchr( ui_svr_mapnames[s_startmap_list.curvalue], '\n' ) + 1);

	maxclients  = atoi( s_maxclients_field.buffer );
	timelimit	= atoi( s_timelimit_field.buffer );
	fraglimit	= atoi( s_fraglimit_field.buffer );

	Cvar_SetValue( "maxclients", ClampCvar( 0, maxclients, maxclients ) );
	Cvar_SetValue ("timelimit", ClampCvar( 0, timelimit, timelimit ) );
	Cvar_SetValue ("fraglimit", ClampCvar( 0, fraglimit, fraglimit ) );
	Cvar_Set("hostname", s_hostname_field.buffer );

	Cvar_SetValue ("deathmatch", s_rules_box.curvalue != 1);
	Cvar_SetValue ("coop", s_rules_box.curvalue == 1);
	Cvar_SetValue ("ctf", s_rules_box.curvalue == 2);
	Cvar_SetValue ("ttctf", s_rules_box.curvalue == 3);
	Cvar_SetValue ("gamerules", FS_RoguePath() ? ((s_rules_box.curvalue == 4) ? 2 : 0) : 0);

#if 1
	UI_StartServer (startmap, (s_dedicated_box.curvalue != 0));
#else
	spot = NULL;
	if (s_rules_box.curvalue == 1)		// PGM
	{
 		if(Q_stricmp(startmap, "bunk1") == 0)
  			spot = "start";
 		else if(Q_stricmp(startmap, "mintro") == 0)
  			spot = "start";
 		else if(Q_stricmp(startmap, "fact1") == 0)
  			spot = "start";
 		else if(Q_stricmp(startmap, "power1") == 0)
  			spot = "pstart";
 		else if(Q_stricmp(startmap, "biggun") == 0)
  			spot = "bstart";
 		else if(Q_stricmp(startmap, "hangar1") == 0)
  			spot = "unitstart";
 		else if(Q_stricmp(startmap, "city1") == 0)
  			spot = "unitstart";
 		else if(Q_stricmp(startmap, "boss1") == 0)
			spot = "bosstart";
	}

	if (spot)
	{
		if (Com_ServerState())
			Cbuf_AddText ("disconnect\n");
		Cbuf_AddText (va("gamemap \"*%s$%s\"\n", startmap, spot));
	}
	else
	{
		Cbuf_AddText (va("map %s\n", startmap));
	}

	UI_ForceMenuOff ();
#endif
}

void StartServer_MenuInit (void)
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
	
#if 0
	UI_BuildMapList (ui_svr_maptype); // was MAP_DM

	// levelshot found table
	if (ui_svr_mapshotvalid)	free(ui_svr_mapshotvalid);
	ui_svr_mapshotvalid = malloc( sizeof( byte ) * ( ui_svr_nummaps + 1 ) );
	memset( ui_svr_mapshotvalid, 0, sizeof( byte ) * ( ui_svr_nummaps + 1 ) );
	// register null levelshot
	if (ui_svr_mapshotvalid[ui_svr_nummaps] == M_UNSET) {	
		if (R_DrawFindPic("/gfx/ui/noscreen.pcx"))
			ui_svr_mapshotvalid[ui_svr_nummaps] = M_FOUND;
		else
			ui_svr_mapshotvalid[ui_svr_nummaps] = M_MISSING;
	}
#endif

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
	s_startmap_list.itemnames			= ui_svr_mapnames;

	s_rules_box.generic.type		= MTYPE_SPINCONTROL;
	s_rules_box.generic.textSize	= MENU_FONT_SIZE;
	s_rules_box.generic.x			= 0;
	s_rules_box.generic.y			= y += 2*MENU_LINE_SIZE;
	s_rules_box.generic.name		= "rules";
//PGM - rogue games only available with rogue DLL.
	if (FS_RoguePath())
		s_rules_box.itemnames		= dm_coop_names_rogue;
	else
		s_rules_box.itemnames		= dm_coop_names;
//PGM
	if (Cvar_VariableValue("ttctf"))
		s_rules_box.curvalue = 3;
	else if (Cvar_VariableValue("ctf"))
		s_rules_box.curvalue = 2;
	else if (FS_RoguePath() && Cvar_VariableValue("gamerules") == 2)
		s_rules_box.curvalue = 4;
	else if (Cvar_VariableValue("coop"))
		s_rules_box.curvalue = 1;
	else
		s_rules_box.curvalue = 0;
	s_rules_box.generic.callback	= RulesChangeFunc;

	s_timelimit_field.generic.type		= MTYPE_FIELD;
	s_timelimit_field.generic.textSize	= MENU_FONT_SIZE;
	s_timelimit_field.generic.name		= "time limit";
	s_timelimit_field.generic.flags		= QMF_NUMBERSONLY;
	s_timelimit_field.generic.x			= 0;
	s_timelimit_field.generic.y			= y += 2*MENU_FONT_SIZE;
	s_timelimit_field.generic.statusbar	= "0 = no limit";
	s_timelimit_field.length			= 4;
	s_timelimit_field.visible_length	= 4;
//	strncpy(s_timelimit_field.buffer, Cvar_VariableString("timelimit"));
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
//	strncpy( s_fraglimit_field.buffer, Cvar_VariableString("fraglimit") );
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
	//	strncpy(s_maxclients_field.buffer, "8");
		Q_strncpyz (s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), "8");
	else 
	//	strncpy(s_maxclients_field.buffer, Cvar_VariableString("maxclients"));
		Q_strncpyz (s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), Cvar_VariableString("maxclients"));
	s_maxclients_field.cursor				= (int)strlen( s_maxclients_field.buffer );

	s_hostname_field.generic.type			= MTYPE_FIELD;
	s_hostname_field.generic.textSize		= MENU_FONT_SIZE;
	s_hostname_field.generic.name			= "hostname";
	s_hostname_field.generic.flags			= 0;
	s_hostname_field.generic.x				= 0;
	s_hostname_field.generic.y				= y += 2.25*MENU_FONT_SIZE;
	s_hostname_field.generic.statusbar		= NULL;
	s_hostname_field.length					= 12;
	s_hostname_field.visible_length			= 12;
//	strncpy( s_hostname_field.buffer, Cvar_VariableString("hostname") );
	Q_strncpyz (s_hostname_field.buffer, sizeof(s_hostname_field.buffer), Cvar_VariableString("hostname"));
	s_hostname_field.cursor					= (int)strlen( s_hostname_field.buffer );

	s_dedicated_box.generic.type			= MTYPE_SPINCONTROL;
	s_dedicated_box.generic.textSize		= MENU_FONT_SIZE;
	s_dedicated_box.generic.name			= "dedicated server";;
	s_dedicated_box.generic.x				= 0;
	s_dedicated_box.generic.y				= y += 2*MENU_FONT_SIZE;
	s_dedicated_box.curvalue				= 0;	// always start off
	s_dedicated_box.generic.statusbar		= "makes the server faster, but you can't play on this computer";
	s_dedicated_box.itemnames				= yesno_names;

	s_startserver_dmoptions_action.generic.type			= MTYPE_ACTION;
	s_startserver_dmoptions_action.generic.textSize		= MENU_FONT_SIZE;
	s_startserver_dmoptions_action.generic.name			= " deathmatch flags";
	s_startserver_dmoptions_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_startserver_dmoptions_action.generic.x			= 24;
	s_startserver_dmoptions_action.generic.y			= y += 2*MENU_FONT_SIZE;
	s_startserver_dmoptions_action.generic.statusbar	= NULL;
	s_startserver_dmoptions_action.generic.callback		= DMOptionsFunc;

	s_startserver_start_action.generic.type		= MTYPE_ACTION;
	s_startserver_start_action.generic.textSize	= MENU_FONT_SIZE;
	s_startserver_start_action.generic.name		= " begin";
	s_startserver_start_action.generic.flags	= QMF_LEFT_JUSTIFY;
	s_startserver_start_action.generic.x		= 24;
	s_startserver_start_action.generic.y		= y += 2*MENU_LINE_SIZE;
	s_startserver_start_action.generic.callback	= M_StartServerActionFunc;

	s_startserver_back_action.generic.type		= MTYPE_ACTION;
	s_startserver_back_action.generic.textSize	= MENU_FONT_SIZE;
	s_startserver_back_action.generic.name		= " back to multiplayer";
	s_startserver_back_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_startserver_back_action.generic.x			= 24;
	s_startserver_back_action.generic.y			= y += 3*MENU_LINE_SIZE;
	s_startserver_back_action.generic.callback	= UI_BackMenu;

	Menu_AddItem( &s_startserver_menu, &s_startmap_list );
	Menu_AddItem( &s_startserver_menu, &s_rules_box );
	Menu_AddItem( &s_startserver_menu, &s_timelimit_field );
	Menu_AddItem( &s_startserver_menu, &s_fraglimit_field );
	Menu_AddItem( &s_startserver_menu, &s_maxclients_field );
	Menu_AddItem( &s_startserver_menu, &s_hostname_field );
	Menu_AddItem( &s_startserver_menu, &s_dedicated_box );
	Menu_AddItem( &s_startserver_menu, &s_startserver_dmoptions_action );
	Menu_AddItem( &s_startserver_menu, &s_startserver_start_action );
	Menu_AddItem( &s_startserver_menu, &s_startserver_back_action );

	Menu_Center( &s_startserver_menu );

	// call this now to set proper inital state
	RulesChangeFunc ( NULL );
}

#if 1
void DrawStartSeverLevelshot (void)
{
	char *mapshotname = UI_UpdateStartSeverLevelshot (s_startmap_list.curvalue);

	SCR_DrawFill (SCREEN_WIDTH/2+44, SCREEN_HEIGHT/2-70, 244, 184, ALIGN_CENTER, 60,60,60,255);

	if (mapshotname)
		SCR_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, mapshotname, 1.0);
	else
		SCR_DrawFill (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, 0,0,0,255);
}
#else
void DrawStartSeverLevelshot (void)
{
	char startmap[MAX_QPATH];
	char mapshotname [MAX_QPATH];
	int i = s_startmap_list.curvalue;
	Q_strncpyz (startmap, sizeof(startmap), strchr( ui_svr_mapnames[i], '\n' ) + 1);

	SCR_DrawFill (SCREEN_WIDTH/2+44, SCREEN_HEIGHT/2-70, 244, 184, ALIGN_CENTER, 60,60,60,255);

	if ( ui_svr_mapshotvalid[i] == M_UNSET) { // init levelshot
		Com_sprintf(mapshotname, sizeof(mapshotname), "/levelshots/%s.pcx", startmap);
		if (R_DrawFindPic(mapshotname))
			ui_svr_mapshotvalid[i] = M_FOUND;
		else
			ui_svr_mapshotvalid[i] = M_MISSING;
	}

	if ( ui_svr_mapshotvalid[i] == M_FOUND) {
		Com_sprintf(mapshotname, sizeof(mapshotname), "/levelshots/%s.pcx", startmap);

		SCR_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, mapshotname, 1.0);
	}
	else if (ui_svr_mapshotvalid[ui_svr_nummaps] == M_FOUND)
		SCR_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, "/gfx/ui/noscreen.pcx", 1.0);
	else
		SCR_DrawFill (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, 0,0,0,255);
}
#endif

void StartServer_MenuDraw (void)
{
	Menu_DrawBanner( "m_banner_start_server" ); // Knightmare added
	Menu_Draw( &s_startserver_menu );
	DrawStartSeverLevelshot (); // added levelshots
}

const char *StartServer_MenuKey (int key)
{
	return Default_MenuKey( &s_startserver_menu, key );
}

void M_Menu_StartServer_f (void)
{
	StartServer_MenuInit();
	UI_PushMenu( StartServer_MenuDraw, StartServer_MenuKey );
}
