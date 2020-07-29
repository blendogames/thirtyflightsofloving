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

// ui_download.c -- the autodownload options menu 

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
static menulist_s	s_allow_http_download_box;
static menulist_s	s_allow_http_fallback_box;
#endif	// USE_CURL

static menulist_s	s_allow_download_maps_box;
static menulist_s	s_allow_download_textures_24bit_box;	// option to allow downloading 24-bit textures
static menulist_s	s_allow_download_models_box;
static menulist_s	s_allow_download_players_box;
static menulist_s	s_allow_download_sounds_box;

static menuaction_s	s_download_back_action;

static void DownloadCallback( void *self )
{
	menulist_s *f = ( menulist_s * ) self;

	if (f == &s_allow_download_box)
	{
		Cvar_SetValue("allow_download", f->curvalue);
	}

#ifdef USE_CURL	// HTTP downloading from R1Q2
	if (f == &s_allow_http_download_box)
	{
		Cvar_SetValue("cl_http_downloads", f->curvalue);
	}
	else if (f == &s_allow_http_fallback_box)
	{
		Cvar_SetValue("cl_http_fallback", f->curvalue);
	}
#endif	// USE_CURL

	else if (f == &s_allow_download_maps_box)
	{
		Cvar_SetValue("allow_download_maps", f->curvalue);
	}

	// Knightmare- option to allow downloading 24-bit textures
	else if (f == &s_allow_download_textures_24bit_box)
	{
		Cvar_SetValue("allow_download_textures_24bit", f->curvalue);
	}

	else if (f == &s_allow_download_models_box)
	{
		Cvar_SetValue("allow_download_models", f->curvalue);
	}

	else if (f == &s_allow_download_players_box)
	{
		Cvar_SetValue("allow_download_players", f->curvalue);
	}

	else if (f == &s_allow_download_sounds_box)
	{
		Cvar_SetValue("allow_download_sounds", f->curvalue);
	}
}

void DownloadOptions_MenuInit( void )
{
	static const char *yes_no_names[] =
	{
		"no",
		"yes",
		0
	};

//	int y = 0;
	int y = 3*MENU_LINE_SIZE;

	s_downloadoptions_menu.x = SCREEN_WIDTH*0.5;
	s_downloadoptions_menu.y = SCREEN_HEIGHT*0.5 - 58;
//	s_downloadoptions_menu.x = viddef.width * 0.50;
//	s_downloadoptions_menu.y = 0;
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
	s_allow_download_box.generic.callback	= DownloadCallback;
	s_allow_download_box.itemnames			= yes_no_names;
	s_allow_download_box.curvalue			= (Cvar_VariableValue("allow_download") != 0);
	s_allow_download_box.generic.statusbar	= "enable or disable all downloading";

#ifdef USE_CURL	// HTTP downloading from R1Q2
	s_allow_http_download_box.generic.type		= MTYPE_SPINCONTROL;
	s_allow_http_download_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_http_download_box.generic.x			= 0;
	s_allow_http_download_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_http_download_box.generic.name		= "HTTP downloading";
	s_allow_http_download_box.generic.callback	= DownloadCallback;
	s_allow_http_download_box.itemnames			= yes_no_names;
	s_allow_http_download_box.curvalue			= (Cvar_VariableValue("cl_http_downloads") != 0);
	s_allow_http_download_box.generic.statusbar	= "use HTTP downloading on supported servers";

	s_allow_http_fallback_box.generic.type		= MTYPE_SPINCONTROL;
	s_allow_http_fallback_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_http_fallback_box.generic.x			= 0;
	s_allow_http_fallback_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_http_fallback_box.generic.name		= "HTTP fallback";
	s_allow_http_fallback_box.generic.callback	= DownloadCallback;
	s_allow_http_fallback_box.itemnames			= yes_no_names;
	s_allow_http_fallback_box.curvalue			= (Cvar_VariableValue("cl_http_fallback") != 0);
	s_allow_http_fallback_box.generic.statusbar	= "enable to allow HTTP downloads to fall back to Q2Pro path and UDP";
#endif	// USE_CURL

	s_allow_download_maps_box.generic.type		= MTYPE_SPINCONTROL;
	s_allow_download_maps_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_maps_box.generic.x			= 0;
	s_allow_download_maps_box.generic.y			= y += 2*MENU_LINE_SIZE;
	s_allow_download_maps_box.generic.name		= "maps/textures";
	s_allow_download_maps_box.generic.callback	= DownloadCallback;
	s_allow_download_maps_box.itemnames			= yes_no_names;
	s_allow_download_maps_box.curvalue			= (Cvar_VariableValue("allow_download_maps") != 0);
	s_allow_download_maps_box.generic.statusbar	= "enable to allow downloading of maps and textures";

	// Knightmare- option to allow downloading 24-bit textures
	s_allow_download_textures_24bit_box.generic.type		= MTYPE_SPINCONTROL;
	s_allow_download_textures_24bit_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_textures_24bit_box.generic.x			= 0;
	s_allow_download_textures_24bit_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_download_textures_24bit_box.generic.name		= "24-bit textures";
	s_allow_download_textures_24bit_box.generic.callback	= DownloadCallback;
	s_allow_download_textures_24bit_box.generic.statusbar	= "enable to allow downloading of JPG and TGA textures";
	s_allow_download_textures_24bit_box.itemnames			= yes_no_names;
	s_allow_download_textures_24bit_box.curvalue			= (Cvar_VariableValue("allow_download_textures_24bit") != 0);
	s_allow_download_textures_24bit_box.generic.statusbar	= "enable to allow downloading of JPG and TGA textures";

	s_allow_download_players_box.generic.type		= MTYPE_SPINCONTROL;
	s_allow_download_players_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_players_box.generic.x			= 0;
	s_allow_download_players_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_download_players_box.generic.name		= "player models/skins";
	s_allow_download_players_box.generic.callback	= DownloadCallback;
	s_allow_download_players_box.itemnames			= yes_no_names;
	s_allow_download_players_box.curvalue			= (Cvar_VariableValue("allow_download_players") != 0);
	s_allow_download_players_box.generic.statusbar	= "enable to allow downloading of player models";

	s_allow_download_models_box.generic.type		= MTYPE_SPINCONTROL;
	s_allow_download_models_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_models_box.generic.x			= 0;
	s_allow_download_models_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_download_models_box.generic.name		= "models";
	s_allow_download_models_box.generic.callback	= DownloadCallback;
	s_allow_download_models_box.itemnames			= yes_no_names;
	s_allow_download_models_box.curvalue			= (Cvar_VariableValue("allow_download_models") != 0);
	s_allow_download_models_box.generic.statusbar	= "enable to allow downloading of models";

	s_allow_download_sounds_box.generic.type		= MTYPE_SPINCONTROL;
	s_allow_download_sounds_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_download_sounds_box.generic.x			= 0;
	s_allow_download_sounds_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_download_sounds_box.generic.name		= "sounds";
	s_allow_download_sounds_box.generic.callback	= DownloadCallback;
	s_allow_download_sounds_box.itemnames			= yes_no_names;
	s_allow_download_sounds_box.curvalue			= (Cvar_VariableValue("allow_download_sounds") != 0);
	s_allow_download_sounds_box.generic.statusbar	= "enable to allow downloading of sounds";

	s_download_back_action.generic.type			= MTYPE_ACTION;
	s_download_back_action.generic.textSize		= MENU_FONT_SIZE;
	s_download_back_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_download_back_action.generic.x			= 0;
	s_download_back_action.generic.y			= y += 3*MENU_LINE_SIZE;
	s_download_back_action.generic.name			= " back";
	s_download_back_action.generic.callback		= UI_BackMenu;

	Menu_AddItem( &s_downloadoptions_menu, &s_download_title );
	Menu_AddItem( &s_downloadoptions_menu, &s_allow_download_box );

#ifdef USE_CURL	// HTTP downloading from R1Q2
	Menu_AddItem( &s_downloadoptions_menu, &s_allow_http_download_box );
	Menu_AddItem( &s_downloadoptions_menu, &s_allow_http_fallback_box );
#endif	// USE_CURL

	Menu_AddItem( &s_downloadoptions_menu, &s_allow_download_maps_box );
	Menu_AddItem( &s_downloadoptions_menu, &s_allow_download_textures_24bit_box );	// option to allow downloading 24-bit textures
	Menu_AddItem( &s_downloadoptions_menu, &s_allow_download_players_box );
	Menu_AddItem( &s_downloadoptions_menu, &s_allow_download_models_box );
	Menu_AddItem( &s_downloadoptions_menu, &s_allow_download_sounds_box );

	Menu_AddItem( &s_downloadoptions_menu, &s_download_back_action );

	Menu_Center( &s_downloadoptions_menu );

	// skip over title
	if (s_downloadoptions_menu.cursor == 0)
		s_downloadoptions_menu.cursor = 1;
}

void DownloadOptions_MenuDraw(void)
{
	Menu_DrawBanner( "m_banner_multiplayer" );
	Menu_Draw( &s_downloadoptions_menu );
}

const char *DownloadOptions_MenuKey( int key )
{
	return Default_MenuKey( &s_downloadoptions_menu, key );
}

void M_Menu_DownloadOptions_f (void)
{
	DownloadOptions_MenuInit();
	UI_PushMenu( DownloadOptions_MenuDraw, DownloadOptions_MenuKey );
}
