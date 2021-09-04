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

// menu_mp_download.c -- the autodownload options menu 

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"


/*
=============================================================================

DOWNLOAD OPTIONS BOOK MENU

=============================================================================
*/
static menuframework_s s_downloadoptions_menu;

static menuseparator_s	s_download_title;
static menulist_s	s_allow_download_box;

#ifdef USE_CURL	// HTTP downloading from R1Q2
static menulist_s	s_http_download_box;
static menulist_s	s_http_fallback_box;
#endif	// USE_CURL

static menulist_s	s_allow_download_maps_box;
static menulist_s	s_allow_download_textures_24bit_box;	// option to allow downloading 24-bit textures
static menulist_s	s_allow_download_models_box;
static menulist_s	s_allow_download_players_box;
static menulist_s	s_allow_download_sounds_box;

static menuaction_s	s_download_back_action;

//=======================================================================

static void AllowDownloadCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_allow_download_box, "allow_download");
}

#ifdef USE_CURL	// HTTP downloading from R1Q2
static void HTTPDownloadCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_http_download_box, "cl_http_downloads");
}

static void HTTPFallbackCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_http_fallback_box, "cl_http_fallback");
}
#endif	// USE_CURL

static void DownloadMapsCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_allow_download_maps_box, "allow_download_maps");
}

static void DownloadTextures24BitCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_allow_download_textures_24bit_box, "allow_download_textures_24bit");
}

static void DownloadPlayersCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_allow_download_players_box, "allow_download_players");
}

static void DownloadModelsCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_allow_download_models_box, "allow_download_models");
}

static void DownloadSoundsCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_allow_download_sounds_box, "allow_download_sounds");
}

//=======================================================================

static void M_Download_SetMenuItemValues (void)
{
	UI_MenuSpinControl_SetValue (&s_allow_download_box, "allow_download", 0, 1, true);
#ifdef USE_CURL	// HTTP downloading from R1Q2
	UI_MenuSpinControl_SetValue (&s_http_download_box, "cl_http_downloads", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_http_fallback_box, "cl_http_fallback", 0, 1, true);
#endif	// USE_CURL
	UI_MenuSpinControl_SetValue (&s_allow_download_maps_box, "allow_download_maps", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_allow_download_textures_24bit_box, "allow_download_textures_24bit", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_allow_download_players_box, "allow_download_players", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_allow_download_models_box, "allow_download_models", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_allow_download_sounds_box, "allow_download_sounds", 0, 1, true);
}

//=======================================================================

void Menu_DownloadOptions_Init (void)
{
	static const char *yes_no_names[] =
	{
		"no",
		"yes",
		0
	};
	int y = 3*MENU_LINE_SIZE;	// 0

	s_downloadoptions_menu.x = SCREEN_WIDTH*0.5;
	s_downloadoptions_menu.y = SCREEN_HEIGHT*0.5 - 58;
	s_downloadoptions_menu.nitems = 0;

	s_download_title.generic.type		= MTYPE_SEPARATOR;
	s_download_title.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_download_title.generic.name		= "Download Options";
	s_download_title.generic.x			= MENU_HEADER_FONT_SIZE/2 * (int)strlen(s_download_title.generic.name); // was 48
	s_download_title.generic.y			= y - 2.75*MENU_LINE_SIZE;	// y

	s_allow_download_box.generic.type		= MTYPE_SPINCONTROL;
	s_allow_download_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_box.generic.x			= 0;
	s_allow_download_box.generic.y			= y += 2*MENU_LINE_SIZE;
	s_allow_download_box.generic.name		= "allow downloading";
	s_allow_download_box.generic.callback	= AllowDownloadCallback;	// DownloadCallback
	s_allow_download_box.itemNames			= yes_no_names;
	s_allow_download_box.generic.statusbar	= "enable or disable all downloading";

#ifdef USE_CURL	// HTTP downloading from R1Q2
	s_http_download_box.generic.type		= MTYPE_SPINCONTROL;
	s_http_download_box.generic.textSize	= MENU_FONT_SIZE;
	s_http_download_box.generic.x			= 0;
	s_http_download_box.generic.y			= y += MENU_LINE_SIZE;
	s_http_download_box.generic.name		= "HTTP downloading";
	s_http_download_box.generic.callback	= HTTPDownloadCallback;	// DownloadCallback
	s_http_download_box.itemNames			= yes_no_names;
	s_http_download_box.generic.statusbar	= "use HTTP downloading on supported servers";

	s_http_fallback_box.generic.type		= MTYPE_SPINCONTROL;
	s_http_fallback_box.generic.textSize	= MENU_FONT_SIZE;
	s_http_fallback_box.generic.x			= 0;
	s_http_fallback_box.generic.y			= y += MENU_LINE_SIZE;
	s_http_fallback_box.generic.name		= "HTTP fallback";
	s_http_fallback_box.generic.callback	= HTTPFallbackCallback;	// DownloadCallback
	s_http_fallback_box.itemNames			= yes_no_names;
	s_http_fallback_box.generic.statusbar	= "enable to allow HTTP downloads to fall back to Q2Pro path and UDP";
#endif	// USE_CURL

	s_allow_download_maps_box.generic.type		= MTYPE_SPINCONTROL;
	s_allow_download_maps_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_maps_box.generic.x			= 0;
	s_allow_download_maps_box.generic.y			= y += 2*MENU_LINE_SIZE;
	s_allow_download_maps_box.generic.name		= "maps/textures";
	s_allow_download_maps_box.generic.callback	= DownloadMapsCallback;	// DownloadCallback
	s_allow_download_maps_box.itemNames			= yes_no_names;
	s_allow_download_maps_box.generic.statusbar	= "enable to allow downloading of maps and textures";

	// Knightmare- option to allow downloading 24-bit textures
	s_allow_download_textures_24bit_box.generic.type		= MTYPE_SPINCONTROL;
	s_allow_download_textures_24bit_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_textures_24bit_box.generic.x			= 0;
	s_allow_download_textures_24bit_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_download_textures_24bit_box.generic.name		= "24-bit textures";
	s_allow_download_textures_24bit_box.generic.callback	= DownloadTextures24BitCallback;	// DownloadCallback
	s_allow_download_textures_24bit_box.generic.statusbar	= "enable to allow downloading of JPG and TGA textures";
	s_allow_download_textures_24bit_box.itemNames			= yes_no_names;
	s_allow_download_textures_24bit_box.generic.statusbar	= "enable to allow downloading of JPG and TGA textures";

	s_allow_download_players_box.generic.type		= MTYPE_SPINCONTROL;
	s_allow_download_players_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_players_box.generic.x			= 0;
	s_allow_download_players_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_download_players_box.generic.name		= "player models/skins";
	s_allow_download_players_box.generic.callback	= DownloadPlayersCallback;	// DownloadCallback
	s_allow_download_players_box.itemNames			= yes_no_names;
	s_allow_download_players_box.generic.statusbar	= "enable to allow downloading of player models";

	s_allow_download_models_box.generic.type		= MTYPE_SPINCONTROL;
	s_allow_download_models_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_models_box.generic.x			= 0;
	s_allow_download_models_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_download_models_box.generic.name		= "models";
	s_allow_download_models_box.generic.callback	= DownloadModelsCallback;	// DownloadCallback
	s_allow_download_models_box.itemNames			= yes_no_names;
	s_allow_download_models_box.generic.statusbar	= "enable to allow downloading of models";

	s_allow_download_sounds_box.generic.type		= MTYPE_SPINCONTROL;
	s_allow_download_sounds_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_sounds_box.generic.x			= 0;
	s_allow_download_sounds_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_download_sounds_box.generic.name		= "sounds";
	s_allow_download_sounds_box.generic.callback	= DownloadSoundsCallback;	// DownloadCallback
	s_allow_download_sounds_box.itemNames			= yes_no_names;
	s_allow_download_sounds_box.generic.statusbar	= "enable to allow downloading of sounds";

	s_download_back_action.generic.type			= MTYPE_ACTION;
	s_download_back_action.generic.textSize		= MENU_FONT_SIZE;
	s_download_back_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_download_back_action.generic.x			= 0;
	s_download_back_action.generic.y			= y += 3*MENU_LINE_SIZE;
	s_download_back_action.generic.name			= " back";
	s_download_back_action.generic.callback		= UI_BackMenu;

	M_Download_SetMenuItemValues ();

	UI_AddMenuItem (&s_downloadoptions_menu, &s_download_title);
	UI_AddMenuItem (&s_downloadoptions_menu, &s_allow_download_box);

#ifdef USE_CURL	// HTTP downloading from R1Q2
	UI_AddMenuItem (&s_downloadoptions_menu, &s_http_download_box);
	UI_AddMenuItem (&s_downloadoptions_menu, &s_http_fallback_box);
#endif	// USE_CURL

	UI_AddMenuItem (&s_downloadoptions_menu, &s_allow_download_maps_box);
	UI_AddMenuItem (&s_downloadoptions_menu, &s_allow_download_textures_24bit_box);	// option to allow downloading 24-bit textures
	UI_AddMenuItem (&s_downloadoptions_menu, &s_allow_download_players_box);
	UI_AddMenuItem (&s_downloadoptions_menu, &s_allow_download_models_box);
	UI_AddMenuItem (&s_downloadoptions_menu, &s_allow_download_sounds_box);

	UI_AddMenuItem (&s_downloadoptions_menu, &s_download_back_action );

	UI_CenterMenu (&s_downloadoptions_menu);

	// skip over title
	if (s_downloadoptions_menu.cursor == 0)
		s_downloadoptions_menu.cursor = 1;
}

void Menu_DownloadOptions_Draw(void)
{
	UI_DrawBanner ("m_banner_multiplayer");
	UI_DrawMenu (&s_downloadoptions_menu);
}

const char *Menu_DownloadOptions_Key (int key)
{
	return UI_DefaultMenuKey (&s_downloadoptions_menu, key);
}

void Menu_DownloadOptions_f (void)
{
	Menu_DownloadOptions_Init ();
	UI_PushMenu (Menu_DownloadOptions_Draw, Menu_DownloadOptions_Key);
}
