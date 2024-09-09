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

#include "../client/client.h"
#include "ui_local.h"

/*
=============================================================================

DOWNLOAD OPTIONS BOOK MENU

=============================================================================
*/
static menuFramework_s	s_downloadoptions_menu;
static menuImage_s		s_downloadoptions_banner;
static menuLabel_s		s_downloadoptions_title;
static menuPicker_s		s_allow_download_box;

#ifdef USE_CURL	// HTTP downloading from R1Q2
static menuPicker_s		s_allow_http_download_box;
static menuPicker_s		s_http_pathtype_box;
static menuPicker_s		s_http_download_lowercase_box;
#endif	// USE_CURL

static menuPicker_s		s_allow_download_maps_box;
static menuPicker_s		s_allow_download_textures_24bit_box;	// option to allow downloading 24-bit textures
static menuPicker_s		s_allow_download_models_box;
static menuPicker_s		s_allow_download_players_box;
static menuPicker_s		s_allow_download_sounds_box;

static menuAction_s		s_download_back_action;

//=======================================================================

void Menu_DownloadOptions_Init (void)
{
	static const char *yes_no_names[] =
	{
		"no",
		"yes",
		0
	};
	static const char *http_pathtype_items[] = {
		"R1Q2",
		"Q2Pro",
		"both",
		0
	};
	int		x, y;

	// menu.x = 320, menu.y = 162
	x = SCREEN_WIDTH*0.5;
	y = SCREEN_HEIGHT*0.5 - 78;

	s_downloadoptions_menu.x			= 0;	// SCREEN_WIDTH*0.5;
	s_downloadoptions_menu.y			= 0;	// SCREEN_HEIGHT*0.5 - 58;
	s_downloadoptions_menu.nitems		= 0;
	s_downloadoptions_menu.isPopup		= false;
	s_downloadoptions_menu.background	= NULL;
	s_downloadoptions_menu.drawFunc		= UI_DefaultMenuDraw;
	s_downloadoptions_menu.keyFunc		= UI_DefaultMenuKey;
	s_downloadoptions_menu.canOpenFunc	= NULL;

	s_downloadoptions_banner.generic.type		= MTYPE_IMAGE;
	s_downloadoptions_banner.generic.x			= 0;
	s_downloadoptions_banner.generic.y			= 9*MENU_LINE_SIZE;
	s_downloadoptions_banner.width				= 275;
	s_downloadoptions_banner.height				= 32;
	s_downloadoptions_banner.imageName			= "/pics/m_banner_multiplayer.pcx";
	s_downloadoptions_banner.alpha				= 255;
	s_downloadoptions_banner.border				= 0;
	s_downloadoptions_banner.hCentered			= true;
	s_downloadoptions_banner.vCentered			= false;
	s_downloadoptions_banner.useAspectRatio		= false;
	s_downloadoptions_banner.generic.isHidden	= false;

	s_downloadoptions_title.generic.type		= MTYPE_LABEL;
	s_downloadoptions_title.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_downloadoptions_title.generic.name		= "Download Options";
	s_downloadoptions_title.generic.x			= x + MENU_HEADER_FONT_SIZE/2 * (int)strlen(s_downloadoptions_title.generic.name); // was 48
	s_downloadoptions_title.generic.y			= y;	// - 2*MENU_LINE_SIZE;

	s_allow_download_box.generic.type			= MTYPE_PICKER;
	s_allow_download_box.generic.textSize		= MENU_FONT_SIZE;
	s_allow_download_box.generic.x				= x;
	s_allow_download_box.generic.y				= y += 4*MENU_LINE_SIZE;
	s_allow_download_box.generic.name			= "allow downloading";
	s_allow_download_box.itemNames				= yes_no_names;
	s_allow_download_box.generic.cvar			= "allow_download";
	s_allow_download_box.generic.statusbar		= "enable or disable all downloading";

#ifdef USE_CURL	// HTTP downloading from R1Q2
	s_allow_http_download_box.generic.type			= MTYPE_PICKER;
	s_allow_http_download_box.generic.textSize		= MENU_FONT_SIZE;
	s_allow_http_download_box.generic.x				= x;
	s_allow_http_download_box.generic.y				= y += MENU_LINE_SIZE;
	s_allow_http_download_box.generic.name			= "HTTP downloading";
	s_allow_http_download_box.itemNames				= yes_no_names;
	s_allow_http_download_box.generic.cvar			= "cl_http_downloads";
	s_allow_http_download_box.generic.statusbar		= "use HTTP downloading on supported servers";

	s_http_pathtype_box.generic.type			= MTYPE_PICKER;
	s_http_pathtype_box.generic.textSize		= MENU_FONT_SIZE;
	s_http_pathtype_box.generic.x				= x;
	s_http_pathtype_box.generic.y				= y += MENU_LINE_SIZE;
	s_http_pathtype_box.generic.name			= "HTTP path type";
	s_http_pathtype_box.itemNames				= http_pathtype_items;
	s_http_pathtype_box.generic.cvar			= "cl_http_pathtype";
	s_http_pathtype_box.generic.cvarClamp		= true;
	s_http_pathtype_box.generic.cvarMin			= 0;
	s_http_pathtype_box.generic.cvarMax			= 2;
	s_http_pathtype_box.generic.statusbar		= "sets server path type for HTTP downloading";

	s_http_download_lowercase_box.generic.type		= MTYPE_PICKER;
	s_http_download_lowercase_box.generic.textSize	= MENU_FONT_SIZE;
	s_http_download_lowercase_box.generic.x			= x;
	s_http_download_lowercase_box.generic.y			= y += MENU_LINE_SIZE;
	s_http_download_lowercase_box.generic.name		= "HTTP convert lowercase";
	s_http_download_lowercase_box.itemNames			= yes_no_names;
	s_http_download_lowercase_box.generic.cvar		= "cl_http_download_lowercase";
	s_http_download_lowercase_box.generic.statusbar	= "converts filenames to lowercase for HTTP downloading";
#endif	// USE_CURL

	s_allow_download_maps_box.generic.type			= MTYPE_PICKER;
	s_allow_download_maps_box.generic.textSize		= MENU_FONT_SIZE;
	s_allow_download_maps_box.generic.x				= x;
	s_allow_download_maps_box.generic.y				= y += 2*MENU_LINE_SIZE;
	s_allow_download_maps_box.generic.name			= "maps/textures";
	s_allow_download_maps_box.itemNames				= yes_no_names;
	s_allow_download_maps_box.generic.cvar			= "allow_download_maps";
	s_allow_download_maps_box.generic.statusbar		= "enable to allow downloading of maps and textures";

	// option to allow downloading 24-bit textures
	s_allow_download_textures_24bit_box.generic.type		= MTYPE_PICKER;
	s_allow_download_textures_24bit_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_textures_24bit_box.generic.x			= x;
	s_allow_download_textures_24bit_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_download_textures_24bit_box.generic.name		= "24-bit textures";
	s_allow_download_textures_24bit_box.generic.statusbar	= "enable to allow downloading of JPG and TGA textures";
	s_allow_download_textures_24bit_box.itemNames			= yes_no_names;
	s_allow_download_textures_24bit_box.generic.cvar		= "allow_download_textures_24bit";
	s_allow_download_textures_24bit_box.generic.statusbar	= "enable to allow downloading of TGA, PNG, and JPG textures";

	s_allow_download_players_box.generic.type		= MTYPE_PICKER;
	s_allow_download_players_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_players_box.generic.x			= x;
	s_allow_download_players_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_download_players_box.generic.name		= "player models/skins";
	s_allow_download_players_box.itemNames			= yes_no_names;
	s_allow_download_players_box.generic.cvar		= "allow_download_players";
	s_allow_download_players_box.generic.statusbar	= "enable to allow downloading of player models";

	s_allow_download_models_box.generic.type		= MTYPE_PICKER;
	s_allow_download_models_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_models_box.generic.x			= x;
	s_allow_download_models_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_download_models_box.generic.name		= "models";
	s_allow_download_models_box.itemNames			= yes_no_names;
	s_allow_download_models_box.generic.cvar		= "allow_download_models";
	s_allow_download_models_box.generic.statusbar	= "enable to allow downloading of models";

	s_allow_download_sounds_box.generic.type		= MTYPE_PICKER;
	s_allow_download_sounds_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_sounds_box.generic.x			= x;
	s_allow_download_sounds_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_download_sounds_box.generic.name		= "sounds";
	s_allow_download_sounds_box.itemNames			= yes_no_names;
	s_allow_download_sounds_box.generic.cvar		= "allow_download_sounds";
	s_allow_download_sounds_box.generic.statusbar	= "enable to allow downloading of sounds";

	s_download_back_action.generic.type			= MTYPE_ACTION;
	s_download_back_action.generic.textSize		= MENU_FONT_SIZE;
	s_download_back_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_download_back_action.generic.x			= x;	// + MENU_FONT_SIZE;
	s_download_back_action.generic.y			= 41*MENU_LINE_SIZE;	// y += 3*MENU_LINE_SIZE
	s_download_back_action.generic.name			= "Back to Multiplayer";
	s_download_back_action.generic.callback		= UI_BackMenu;

	UI_AddMenuItem (&s_downloadoptions_menu, &s_downloadoptions_banner);
	UI_AddMenuItem (&s_downloadoptions_menu, &s_downloadoptions_title);
	UI_AddMenuItem (&s_downloadoptions_menu, &s_allow_download_box);

#ifdef USE_CURL	// HTTP downloading from R1Q2
	UI_AddMenuItem (&s_downloadoptions_menu, &s_allow_http_download_box);
	UI_AddMenuItem (&s_downloadoptions_menu, &s_http_pathtype_box);
	UI_AddMenuItem (&s_downloadoptions_menu, &s_http_download_lowercase_box);
#endif	// USE_CURL

	UI_AddMenuItem (&s_downloadoptions_menu, &s_allow_download_maps_box);
	UI_AddMenuItem (&s_downloadoptions_menu, &s_allow_download_textures_24bit_box);	// option to allow downloading 24-bit textures
	UI_AddMenuItem (&s_downloadoptions_menu, &s_allow_download_players_box);
	UI_AddMenuItem (&s_downloadoptions_menu, &s_allow_download_models_box);
	UI_AddMenuItem (&s_downloadoptions_menu, &s_allow_download_sounds_box);
	UI_AddMenuItem (&s_downloadoptions_menu, &s_download_back_action );
}


void Menu_DownloadOptions_f (void)
{
	Menu_DownloadOptions_Init ();
	UI_PushMenu (&s_downloadoptions_menu);
}
