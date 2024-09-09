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

#include "../client/client.h"
#include "ui_local.h"

#ifdef NOTTHIRTYFLIGHTS
#define MAPLIST_ARENAS

/*
=======================================================================

MULTIPLAYER MENU

=======================================================================
*/

menuFramework_s			s_multiplayer_menu;
static menuImage_s		s_multiplayer_banner;
static menuAction_s		s_join_network_server_action;
static menuAction_s		s_start_network_server_action;
static menuAction_s		s_player_setup_action;
static menuAction_s		s_download_options_action;
static menuAction_s		s_backmain_action;

//=======================================================================

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
	int		x, y;

	// menu.x = 264, menu.y = 200
	x = SCREEN_WIDTH*0.5 - 9*MENU_FONT_SIZE;
	y = SCREEN_HEIGHT*0.5 - 5*MENU_LINE_SIZE;

	s_multiplayer_menu.x			= 0;	// SCREEN_WIDTH*0.5 - 9*MENU_FONT_SIZE;		// -64
	s_multiplayer_menu.y			= 0;	// SCREEN_HEIGHT*0.5 - 5*MENU_LINE_SIZE;	// 0
	s_multiplayer_menu.nitems		= 0;
	s_multiplayer_menu.isPopup		= false;
	s_multiplayer_menu.background	= NULL;
	s_multiplayer_menu.drawFunc		= UI_DefaultMenuDraw;
	s_multiplayer_menu.keyFunc		= UI_DefaultMenuKey;
	s_multiplayer_menu.canOpenFunc	= NULL;

	s_multiplayer_banner.generic.type		= MTYPE_IMAGE;
	s_multiplayer_banner.generic.x			= 0;
	s_multiplayer_banner.generic.y			= 9*MENU_LINE_SIZE;
	s_multiplayer_banner.width				= 275;
	s_multiplayer_banner.height				= 32;
	s_multiplayer_banner.imageName			= "/pics/m_banner_multiplayer.pcx";
	s_multiplayer_banner.alpha				= 255;
	s_multiplayer_banner.border				= 0;
	s_multiplayer_banner.hCentered			= true;
	s_multiplayer_banner.vCentered			= false;
	s_multiplayer_banner.useAspectRatio		= false;
	s_multiplayer_banner.generic.isHidden	= false;

	s_join_network_server_action.generic.type		= MTYPE_ACTION;
	s_join_network_server_action.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_join_network_server_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_join_network_server_action.generic.x			= x;
	s_join_network_server_action.generic.y			= y;
	s_join_network_server_action.generic.name		= "Join Network Server";
	s_join_network_server_action.generic.callback	= JoinNetworkServerFunc;

	s_start_network_server_action.generic.type		= MTYPE_ACTION;
	s_start_network_server_action.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_start_network_server_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_start_network_server_action.generic.x			= x;
	s_start_network_server_action.generic.y			= y += 2*MENU_LINE_SIZE;	// 2*MENU_FONT_SIZE
	s_start_network_server_action.generic.name		= "Start Network Server";
	s_start_network_server_action.generic.callback	= StartNetworkServerFunc;

	s_player_setup_action.generic.type		= MTYPE_ACTION;
	s_player_setup_action.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_player_setup_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_player_setup_action.generic.x			= x;
	s_player_setup_action.generic.y			= y += 2*MENU_LINE_SIZE;	// 4*MENU_FONT_SIZE
	s_player_setup_action.generic.name		= "Player Setup";
	s_player_setup_action.generic.callback	= PlayerSetupFunc;

	s_download_options_action.generic.type		= MTYPE_ACTION;
	s_download_options_action.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_download_options_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_download_options_action.generic.x			= x;
	s_download_options_action.generic.y			= y += 2*MENU_LINE_SIZE;	// 6*MENU_FONT_SIZE
	s_download_options_action.generic.name		= "Download Options";
	s_download_options_action.generic.callback	= DownloadOptionsFunc;

	s_backmain_action.generic.type		= MTYPE_ACTION;
	s_backmain_action.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_backmain_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_backmain_action.generic.x			= x;
	s_backmain_action.generic.y			= y += 3*MENU_HEADER_LINE_SIZE;	// 9*MENU_FONT_SIZE
	s_backmain_action.generic.name		= "Back to Main";
	s_backmain_action.generic.callback	= UI_BackMenu;

	UI_AddMenuItem (&s_multiplayer_menu, (void *) &s_multiplayer_banner);
	UI_AddMenuItem (&s_multiplayer_menu, (void *) &s_join_network_server_action);
	UI_AddMenuItem (&s_multiplayer_menu, (void *) &s_start_network_server_action);
	UI_AddMenuItem (&s_multiplayer_menu, (void *) &s_player_setup_action);
	UI_AddMenuItem (&s_multiplayer_menu, (void *) &s_download_options_action);
	UI_AddMenuItem (&s_multiplayer_menu, (void *) &s_backmain_action);

	UI_SetMenuStatusBar (&s_multiplayer_menu, NULL);
}


void Menu_Multiplayer_f (void)
{
	Menu_Multiplayer_Init ();
	UI_PushMenu (&s_multiplayer_menu);
}
#endif
