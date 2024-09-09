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

#include "../client/client.h"
#include "ui_local.h"

#ifdef NOTTHIRTYFLIGHTS
#define USE_LISTBOX	// enable to use new listBox control

/*
=============================================================================

START SERVER MENU

=============================================================================
*/
static menuFramework_s s_startserver_menu;
static menuImage_s		s_startserver_banner;
#ifdef USE_LISTBOX
static menuListBox_s	s_startmap_list;
static menuComboBox_s	s_rules_box;
#else	// USE_LISTBOX
static menuPicker_s		s_startmap_list;
static menuPicker_s		s_rules_box;
#endif	// USE_LISTBOX
static menuImage_s		s_startserver_mapshot;
static menuComboBox_s	s_rules_box;
static menuField_s		s_timelimit_field;
static menuField_s		s_fraglimit_field;
static menuField_s		s_maxclients_field;
static menuField_s		s_hostname_field;
static menuPicker_s		s_dedicated_box;
static menuAction_s		s_startserver_dmoptions_action;
static menuAction_s		s_startserver_start_action;
static menuAction_s		s_startserver_back_action;

//=============================================================================

/*
===============
M_RefreshMapList
===============
*/
void M_RefreshMapList (maptype_t maptype)
{
	if (maptype == ui_svr_maptype) // no change
		return;

	// load map list
	UI_UpdateMapList (maptype);

	// reset startmap if it's in the part of the list that changed
	if (s_startmap_list.curValue >= ui_svr_listfile_nummaps)
		s_startmap_list.curValue = 0;
	s_startmap_list.itemNames = ui_svr_mapnames;
	UI_InitMenuItem (&s_startmap_list);
}


//=============================================================================

void M_StartmapChangeFunc (void *unused)
{
	s_startserver_mapshot.imageName = UI_UpdateStartSeverLevelshot (s_startmap_list.curValue);
}

void M_RulesChangeFunc (void *self)
{
	int			maxclients_default[2] = {-1, -1};
	maptype_t	maptype = MAP_DM;

	s_maxclients_field.generic.statusbar = NULL;
	s_startserver_dmoptions_action.generic.statusbar = NULL;

	UI_SetCoopMenuMode (false);
	UI_SetCTFMenuMode (false);

	if ( (s_rules_box.curValue == 0) || (s_rules_box.curValue == 4) ) 	// DM / tag
	{
		Vector2Set (maxclients_default, 8, -1);		// set default of 8
		maptype = MAP_DM;
	}
	else if (s_rules_box.curValue == 1)		// coop				// PGM
	{
		Vector2Set (maxclients_default, 2, 4);		// set max of 4
		maptype = MAP_COOP;
		UI_SetCoopMenuMode (true);
		s_maxclients_field.generic.statusbar = "4 maximum for cooperative";
	}
	else if (s_rules_box.curValue == 2)		// CTF
	{
		Vector2Set (maxclients_default, 12, -1);	// set default of 12
		maptype = MAP_CTF;
		UI_SetCTFMenuMode (true);
	}
	else if (s_rules_box.curValue == 3)		// 3Team CTF
	{
		Vector2Set (maxclients_default, 18, -1);	// set default of 18
		maptype = MAP_3TCTF;
		UI_SetCTFMenuMode (true);
	}

	if (maxclients_default[0] >= 0) {
		if (atoi(s_maxclients_field.buffer) < maxclients_default[0])
			Q_strncpyz(s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), va("%i", maxclients_default[0]));
	}
	if (maxclients_default[1] >= 0) {
		if (atoi(s_maxclients_field.buffer) > maxclients_default[1])
			Q_strncpyz(s_maxclients_field.buffer, sizeof(s_maxclients_field.buffer), va("%i", maxclients_default[1]));
	}

	M_RefreshMapList (maptype);
	s_startserver_mapshot.imageName = UI_UpdateStartSeverLevelshot (s_startmap_list.curValue);
}

void M_DMOptionsFunc (void *self)
{
	Menu_DMOptions_f ();
}

void M_StartServerActionFunc (void *self)
{
	char	startmap[1024];

	Q_strncpyz (startmap, sizeof(startmap), strchr( ui_svr_mapnames[s_startmap_list.curValue], '\n' ) + 1);

	UI_SaveMenuItemValue (&s_maxclients_field);
	UI_SaveMenuItemValue (&s_timelimit_field);
	UI_SaveMenuItemValue (&s_fraglimit_field);
	UI_SaveMenuItemValue (&s_hostname_field);

	Cvar_SetValue ("deathmatch", s_rules_box.curValue != 1);
	Cvar_SetValue ("coop", s_rules_box.curValue == 1);
	Cvar_SetValue ("ctf", s_rules_box.curValue == 2);
	Cvar_SetValue ("ttctf", s_rules_box.curValue == 3);
	Cvar_SetValue ("gamerules", (s_rules_box.curValue == 4) ? 2 : 0);

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
	int		x, y;
	
	// menu.x = 168, menu.y = 150
	x = SCREEN_WIDTH*0.5 - 152;	// was -140
	y = SCREEN_HEIGHT*0.5 - 9*MENU_LINE_SIZE;

	//
	// initialize the menu stuff
	//
	s_startserver_menu.x			= 0;
	s_startserver_menu.y			= 0;
	s_startserver_menu.nitems		= 0;
	s_startserver_menu.isPopup		= false;
	s_startserver_menu.background	= NULL;
	s_startserver_menu.drawFunc		= UI_DefaultMenuDraw;
	s_startserver_menu.keyFunc		= UI_DefaultMenuKey;
	s_startserver_menu.canOpenFunc	= NULL;
	s_startserver_menu.onOpenFunc	= M_RulesChangeFunc;

	s_startserver_banner.generic.type		= MTYPE_IMAGE;
	s_startserver_banner.generic.x			= 0;
	s_startserver_banner.generic.y			= 9*MENU_LINE_SIZE;
	s_startserver_banner.width				= 275;
	s_startserver_banner.height				= 32;
	s_startserver_banner.imageName			= "/pics/m_banner_start_server.pcx";
	s_startserver_banner.alpha				= 255;
	s_startserver_banner.border				= 0;
	s_startserver_banner.hCentered			= true;
	s_startserver_banner.vCentered			= false;
	s_startserver_banner.useAspectRatio		= false;
	s_startserver_banner.generic.isHidden	= false;

#ifdef USE_LISTBOX
	s_startmap_list.generic.type		= MTYPE_LISTBOX;
	s_startmap_list.generic.x			= x;
	s_startmap_list.generic.y			= y;
	s_startmap_list.generic.name		= "initial map";
	s_startmap_list.itemNames			= ui_svr_mapnames;
	s_startmap_list.generic.callback	= M_StartmapChangeFunc;
	s_startmap_list.itemWidth			= 40;
	s_startmap_list.itemHeight			= 2;
	s_startmap_list.items_y				= 5;
	s_startmap_list.itemSpacing			= 0;
	s_startmap_list.itemTextSize		= 8;
	s_startmap_list.border				= 2;
	s_startmap_list.borderColor[0]		= 60;
	s_startmap_list.borderColor[1]		= 60;
	s_startmap_list.borderColor[2]		= 60;
	s_startmap_list.borderColor[3]		= 255;
	s_startmap_list.backColor[0]		= 0;
	s_startmap_list.backColor[1]		= 0;
	s_startmap_list.backColor[2]		= 0;
	s_startmap_list.backColor[3]		= 192;
	s_startmap_list.altBackColor[0]		= 10;
	s_startmap_list.altBackColor[1]		= 10;
	s_startmap_list.altBackColor[2]		= 10;
	s_startmap_list.altBackColor[3]		= 192;
#else	// USE_LISTBOX
	s_startmap_list.generic.type		= MTYPE_PICKER;
	s_startmap_list.generic.textSize	= MENU_FONT_SIZE;
	s_startmap_list.generic.x			= x;
	s_startmap_list.generic.y			= y;
	s_startmap_list.generic.name		= "initial map";
	s_startmap_list.itemNames			= ui_svr_mapnames;
	s_startmap_list.generic.callback	= M_StartmapChangeFunc;
#endif	// USE_LISTBOX

//	x = SCREEN_WIDTH/2+46, y = SCREEN_HEIGHT/2-68, w = 240, h = 180
	s_startserver_mapshot.generic.type		= MTYPE_IMAGE;
	s_startserver_mapshot.generic.x			= x + 23*MENU_FONT_SIZE+2;	// +186
#ifdef USE_LISTBOX
	s_startserver_mapshot.generic.y			= y + 10.6*MENU_LINE_SIZE;	// was 88
	s_startserver_mapshot.width				= 220;
	s_startserver_mapshot.height			= 165;
#else	// USE_LISTBOX
	s_startserver_mapshot.generic.y			= y + 2.2*MENU_LINE_SIZE;	// +22
	s_startserver_mapshot.width				= 240;	// 200
	s_startserver_mapshot.height			= 180;	// 150
#endif	// USE_LISTBOX
	s_startserver_mapshot.imageName			= UI_NOSCREEN_NAME;
	s_startserver_mapshot.alpha				= 255;
	s_startserver_mapshot.border			= 2;
	s_startserver_mapshot.borderColor[0]	= 60;
	s_startserver_mapshot.borderColor[1]	= 60;
	s_startserver_mapshot.borderColor[2]	= 60;
	s_startserver_mapshot.borderColor[3]	= 255;
	s_startserver_mapshot.hCentered			= false;
	s_startserver_mapshot.vCentered			= false;
	s_startserver_mapshot.useAspectRatio	= false;
	s_startserver_mapshot.generic.isHidden	= false;

#ifdef USE_LISTBOX
	s_rules_box.generic.type		= MTYPE_COMBOBOX;
	s_rules_box.generic.x			= x;
	s_rules_box.generic.y			= y += 10.5*MENU_LINE_SIZE; // was 8.5
	s_rules_box.generic.name		= "rules";
	s_rules_box.generic.callback	= M_RulesChangeFunc;
	s_rules_box.items_y				= 5;
	s_rules_box.itemWidth			= 12;
	s_rules_box.itemSpacing			= 1;
	s_rules_box.itemTextSize		= 8;
	s_rules_box.border				= 1;
	s_rules_box.borderColor[0]		= 60;
	s_rules_box.borderColor[1]		= 60;
	s_rules_box.borderColor[2]		= 60;
	s_rules_box.borderColor[3]		= 255;
	s_rules_box.backColor[0]		= 0;
	s_rules_box.backColor[1]		= 0;
	s_rules_box.backColor[2]		= 0;
	s_rules_box.backColor[3]		= 192;
#else	// USE_LISTBOX
	s_rules_box.generic.type		= MTYPE_PICKER;
	s_rules_box.generic.textSize	= MENU_FONT_SIZE;
	s_rules_box.generic.x			= x;
	s_rules_box.generic.y			= y += 2*MENU_LINE_SIZE;
	s_rules_box.generic.name		= "rules";
	s_rules_box.generic.callback	= M_RulesChangeFunc;
#endif	// USE_LISTBOX
// PGM - rogue games only available with rogue DLL
	if ( FS_RoguePath() )
		s_rules_box.itemNames		= dm_coop_names_rogue;
	else
		s_rules_box.itemNames		= dm_coop_names;
// PGM
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

	s_timelimit_field.generic.type			= MTYPE_FIELD;
	s_timelimit_field.generic.textSize		= MENU_FONT_SIZE;
	s_timelimit_field.generic.name			= "time limit";
	s_timelimit_field.generic.flags			= QMF_NUMBERSONLY;
	s_timelimit_field.generic.x				= x;
	s_timelimit_field.generic.y				= y += 2.5*MENU_FONT_SIZE;	// was 2*MENU_FONT_SIZE
	s_timelimit_field.generic.statusbar		= "0 = no limit";
	s_timelimit_field.length				= 4;
	s_timelimit_field.visible_length		= 4;
	s_timelimit_field.generic.cvar			= "timelimit";
	s_timelimit_field.generic.cvarNoSave	= true;
	s_timelimit_field.length				= 4;
	s_timelimit_field.visible_length		= 4;

	s_fraglimit_field.generic.type			= MTYPE_FIELD;
	s_fraglimit_field.generic.textSize		= MENU_FONT_SIZE;
	s_fraglimit_field.generic.name			= "frag limit";
	s_fraglimit_field.generic.flags			= QMF_NUMBERSONLY;
	s_fraglimit_field.generic.x				= x;
	s_fraglimit_field.generic.y				= y += 2.25*MENU_FONT_SIZE;
	s_fraglimit_field.generic.statusbar		= "0 = no limit";
	s_fraglimit_field.length				= 4;
	s_fraglimit_field.visible_length		= 4;
	s_fraglimit_field.generic.cvar			= "fraglimit";
	s_fraglimit_field.generic.cvarNoSave	= true;
	s_fraglimit_field.length				= 4;
	s_fraglimit_field.visible_length		= 4;

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
	s_maxclients_field.generic.x			= x;
	s_maxclients_field.generic.y			= y += 2.25*MENU_FONT_SIZE;
	s_maxclients_field.generic.statusbar	= NULL;
	s_maxclients_field.generic.cvar			= "maxclients";
	s_maxclients_field.generic.cvarNoSave	= true;
	s_maxclients_field.length				= 3;
	s_maxclients_field.visible_length		= 3;

	s_hostname_field.generic.type			= MTYPE_FIELD;
	s_hostname_field.generic.textSize		= MENU_FONT_SIZE;
	s_hostname_field.generic.name			= "hostname";
	s_hostname_field.generic.flags			= 0;
	s_hostname_field.generic.x				= x;
	s_hostname_field.generic.y				= y += 2.25*MENU_FONT_SIZE;
	s_hostname_field.generic.statusbar		= NULL;
	s_hostname_field.generic.cvar			= "hostname";
	s_hostname_field.generic.cvarNoSave		= true;
	s_hostname_field.length					= 16;
	s_hostname_field.visible_length			= 16;

	s_dedicated_box.generic.type			= MTYPE_PICKER;
	s_dedicated_box.generic.textSize		= MENU_FONT_SIZE;
	s_dedicated_box.generic.name			= "dedicated server";;
	s_dedicated_box.generic.x				= x;
	s_dedicated_box.generic.y				= y += 2*MENU_FONT_SIZE;
	s_dedicated_box.curValue				= 0;	// always start off
	s_dedicated_box.itemNames				= yesno_names;
	s_dedicated_box.generic.statusbar		= "makes the server faster, but you can't play on this computer";

	s_startserver_dmoptions_action.generic.type			= MTYPE_ACTION;
	s_startserver_dmoptions_action.generic.textSize		= MENU_FONT_SIZE;
	s_startserver_dmoptions_action.generic.name			= "Deathmatch Flags";
	s_startserver_dmoptions_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_startserver_dmoptions_action.generic.x			= x + 4*MENU_FONT_SIZE;
	s_startserver_dmoptions_action.generic.y			= y += 2*MENU_FONT_SIZE;
	s_startserver_dmoptions_action.generic.statusbar	= NULL;
	s_startserver_dmoptions_action.generic.callback		= M_DMOptionsFunc;

	s_startserver_start_action.generic.type		= MTYPE_ACTION;
	s_startserver_start_action.generic.textSize	= MENU_FONT_SIZE;
	s_startserver_start_action.generic.name		= "Begin";
	s_startserver_start_action.generic.flags	= QMF_LEFT_JUSTIFY;
	s_startserver_start_action.generic.x		= x + 4*MENU_FONT_SIZE;
	s_startserver_start_action.generic.y		= y += 2*MENU_LINE_SIZE;
	s_startserver_start_action.generic.callback	= M_StartServerActionFunc;

	s_startserver_back_action.generic.type		= MTYPE_ACTION;
	s_startserver_back_action.generic.textSize	= MENU_FONT_SIZE;
	s_startserver_back_action.generic.name		= "Back to Multiplayer";
	s_startserver_back_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_startserver_back_action.generic.x			= x + 4*MENU_FONT_SIZE;
	s_startserver_back_action.generic.y			= y += 3*MENU_LINE_SIZE;
	s_startserver_back_action.generic.callback	= UI_BackMenu;

	UI_AddMenuItem (&s_startserver_menu, &s_startserver_banner);
	UI_AddMenuItem (&s_startserver_menu, &s_startmap_list);
	UI_AddMenuItem (&s_startserver_menu, &s_startserver_mapshot);
	UI_AddMenuItem (&s_startserver_menu, &s_rules_box);
	UI_AddMenuItem (&s_startserver_menu, &s_timelimit_field);
	UI_AddMenuItem (&s_startserver_menu, &s_fraglimit_field);
	UI_AddMenuItem (&s_startserver_menu, &s_maxclients_field);
	UI_AddMenuItem (&s_startserver_menu, &s_hostname_field);
	UI_AddMenuItem (&s_startserver_menu, &s_dedicated_box);
	UI_AddMenuItem (&s_startserver_menu, &s_startserver_dmoptions_action);
	UI_AddMenuItem (&s_startserver_menu, &s_startserver_start_action);
	UI_AddMenuItem (&s_startserver_menu, &s_startserver_back_action);
}


void Menu_StartServer_f (void)
{
	Menu_StartServer_Init ();
	UI_PushMenu (&s_startserver_menu);
}
#endif
