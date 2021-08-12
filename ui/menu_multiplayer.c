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

// menu_multiplayer.c -- the multiplayer menu 

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

#define MAPLIST_ARENAS


/*
=======================================================================

MULTIPLAYER MENU

=======================================================================
*/

menuframework_s			s_multiplayer_menu;
static menuaction_s		s_join_network_server_action;
static menuaction_s		s_start_network_server_action;
static menuaction_s		s_player_setup_action;
static menuaction_s		s_download_options_action;
static menuaction_s		s_backmain_action;

static void PlayerSetupFunc (void *unused)
{
	Menu_PlayerConfig_f ();
}

static void JoinNetworkServerFunc (void *unused)
{
	Menu_JoinServer_f ();
}

static void StartNetworkServerFunc (void *unused)
{
	Menu_StartServer_f ();
}

void DownloadOptionsFunc (void *unused)
{
	Menu_DownloadOptions_f ();
}

//=======================================================================

void Menu_Multiplayer_Init (void)
{
	s_multiplayer_menu.x = SCREEN_WIDTH*0.5 - 64;
//	s_multiplayer_menu.y = 0;
	s_multiplayer_menu.nitems = 0;

	s_join_network_server_action.generic.type		= MTYPE_ACTION;
	s_join_network_server_action.generic.textSize	= MENU_FONT_SIZE;
	s_join_network_server_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_join_network_server_action.generic.x			= 0;
	s_join_network_server_action.generic.y			= 0;
	s_join_network_server_action.generic.name		= " join network server";
	s_join_network_server_action.generic.callback	= JoinNetworkServerFunc;

	s_start_network_server_action.generic.type		= MTYPE_ACTION;
	s_start_network_server_action.generic.textSize	= MENU_FONT_SIZE;
	s_start_network_server_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_start_network_server_action.generic.x			= 0;
	s_start_network_server_action.generic.y			= 2*MENU_FONT_SIZE;
	s_start_network_server_action.generic.name		= " start network server";
	s_start_network_server_action.generic.callback	= StartNetworkServerFunc;

	s_player_setup_action.generic.type		= MTYPE_ACTION;
	s_player_setup_action.generic.textSize	= MENU_FONT_SIZE;
	s_player_setup_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_player_setup_action.generic.x			= 0;
	s_player_setup_action.generic.y			= 4*MENU_FONT_SIZE;
	s_player_setup_action.generic.name		= " player setup";
	s_player_setup_action.generic.callback	= PlayerSetupFunc;

	s_download_options_action.generic.type		= MTYPE_ACTION;
	s_download_options_action.generic.textSize	= MENU_FONT_SIZE;
	s_download_options_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_download_options_action.generic.x			= 0;
	s_download_options_action.generic.y			= 6*MENU_FONT_SIZE;
	s_download_options_action.generic.name		= " download options";
	s_download_options_action.generic.callback	= DownloadOptionsFunc;

	s_backmain_action.generic.type		= MTYPE_ACTION;
	s_backmain_action.generic.textSize	= MENU_FONT_SIZE;
	s_backmain_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_backmain_action.generic.x			= 0;
	s_backmain_action.generic.y			= 9*MENU_FONT_SIZE;
	s_backmain_action.generic.name		= " back to main";
	s_backmain_action.generic.callback	= UI_BackMenu;

	UI_AddMenuItem (&s_multiplayer_menu, (void *) &s_join_network_server_action);
	UI_AddMenuItem (&s_multiplayer_menu, (void *) &s_start_network_server_action);
	UI_AddMenuItem (&s_multiplayer_menu, (void *) &s_player_setup_action);
	UI_AddMenuItem (&s_multiplayer_menu, (void *) &s_download_options_action);
	UI_AddMenuItem (&s_multiplayer_menu, (void *) &s_backmain_action);

	UI_SetMenuStatusBar (&s_multiplayer_menu, NULL);

	UI_CenterMenu (&s_multiplayer_menu);
}

static void Menu_Multiplayer_Draw (void)
{
	UI_DrawBanner ("m_banner_multiplayer");

	UI_AdjustMenuCursor (&s_multiplayer_menu, 1);
	UI_DrawMenu (&s_multiplayer_menu);
}

const char *Menu_Multiplayer_Key (int key)
{
	return UI_DefaultMenuKey (&s_multiplayer_menu, key);
}

void Menu_Multiplayer_f (void)
{
	Menu_Multiplayer_Init ();
	UI_PushMenu (Menu_Multiplayer_Draw, Menu_Multiplayer_Key);
}
