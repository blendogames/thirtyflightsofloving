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

// ui_options.c -- the options menu

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

/*
=======================================================================

OPTIONS MENU

=======================================================================
*/

static menuframework_s	s_options_menu;
static menuaction_s		s_options_sound_section;
static menuaction_s		s_options_controls_section;
static menuaction_s		s_options_screen_section;
static menuaction_s		s_options_effects_section;
static menuaction_s		s_options_interface_section;
static menuaction_s		s_options_back_action;

static void MenuSoundFunc ( void *unused )
{
	M_Menu_Options_Sound_f();
}

static void MenuControlsFunc ( void *unused )
{
	M_Menu_Options_Controls_f();
}

static void MenuScreenFunc ( void *unused )
{
	M_Menu_Options_Screen_f();
}

static void MenuEffectsFunc ( void *unused )
{
	M_Menu_Options_Effects_f();
}

static void MenuInterfaceFunc ( void *unused )
{
	M_Menu_Options_Interface_f();
}

//=======================================================================

void Options_MenuInit ( void )
{
	s_options_menu.x = SCREEN_WIDTH*0.5 - 24;
	s_options_menu.y = SCREEN_HEIGHT*0.5 - 58;
	s_options_menu.nitems = 0;

	s_options_sound_section.generic.type		= MTYPE_ACTION;
	s_options_sound_section.generic.textSize	= MENU_FONT_SIZE;
	s_options_sound_section.generic.flags		= QMF_LEFT_JUSTIFY;
	s_options_sound_section.generic.name		= " sound";
	s_options_sound_section.generic.x			= 0;
	s_options_sound_section.generic.y			= MENU_FONT_SIZE * 2;
	s_options_sound_section.generic.callback	= MenuSoundFunc;
	s_options_sound_section.generic.statusbar		= "change sound settings";
//	s_options_sound_section.generic.cursor_offset = -(MENU_FONT_SIZE*10);
	
	s_options_controls_section.generic.type		= MTYPE_ACTION;
	s_options_controls_section.generic.textSize	= MENU_FONT_SIZE;
	s_options_controls_section.generic.flags	= QMF_LEFT_JUSTIFY;
	s_options_controls_section.generic.name		= " controls";
	s_options_controls_section.generic.x		= 0;
	s_options_controls_section.generic.y		= MENU_FONT_SIZE * 4;
	s_options_controls_section.generic.callback = MenuControlsFunc;
	s_options_controls_section.generic.statusbar	= "change control settings and bind keys";
//	s_options_controls_section.generic.cursor_offset = -(MENU_FONT_SIZE*10);
	
	s_options_screen_section.generic.type		= MTYPE_ACTION;
	s_options_screen_section.generic.textSize	= MENU_FONT_SIZE;
	s_options_screen_section.generic.flags		= QMF_LEFT_JUSTIFY;
	s_options_screen_section.generic.name		= " screen";
	s_options_screen_section.generic.x			= 0;
	s_options_screen_section.generic.y			= MENU_FONT_SIZE * 6;
	s_options_screen_section.generic.callback	= MenuScreenFunc;
	s_options_screen_section.generic.statusbar		= "change HUD/crosshair settings";
//	s_options_screen_section.generic.cursor_offset = -(MENU_FONT_SIZE*10);

	s_options_effects_section.generic.type		= MTYPE_ACTION;
	s_options_effects_section.generic.textSize	= MENU_FONT_SIZE;
	s_options_effects_section.generic.flags		= QMF_LEFT_JUSTIFY;
	s_options_effects_section.generic.name		= " effects";
	s_options_effects_section.generic.x			= 0;
	s_options_effects_section.generic.y			= MENU_FONT_SIZE * 8;
	s_options_effects_section.generic.callback	= MenuEffectsFunc;
	s_options_effects_section.generic.statusbar		= "change ingame effects settings";
//	s_options_effects_section.generic.cursor_offset = -(MENU_FONT_SIZE*10);

	s_options_interface_section.generic.type		= MTYPE_ACTION;
	s_options_interface_section.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_section.generic.flags		= QMF_LEFT_JUSTIFY;
	s_options_interface_section.generic.name		= " interface";
	s_options_interface_section.generic.x			= 0;
	s_options_interface_section.generic.y			= MENU_FONT_SIZE * 10;
	s_options_interface_section.generic.callback	= MenuInterfaceFunc;
	s_options_interface_section.generic.statusbar	= "change menu/console settings";
//	s_options_interface_section.generic.cursor_offset = -(MENU_FONT_SIZE*10);

	s_options_back_action.generic.type		= MTYPE_ACTION;
	s_options_back_action.generic.textSize	= MENU_FONT_SIZE;
	s_options_back_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_options_back_action.generic.name		= " back to main";
	s_options_back_action.generic.x			= 0;
	s_options_back_action.generic.y			= MENU_FONT_SIZE * 13;
	s_options_back_action.generic.callback	= UI_BackMenu;
//	s_options_back_action.generic.cursor_offset = -(MENU_FONT_SIZE*10);

	Menu_AddItem( &s_options_menu,	( void * ) &s_options_sound_section );
	Menu_AddItem( &s_options_menu,	( void * ) &s_options_controls_section );
	Menu_AddItem( &s_options_menu,	( void * ) &s_options_screen_section );
	Menu_AddItem( &s_options_menu,	( void * ) &s_options_effects_section );
	Menu_AddItem( &s_options_menu,	( void * ) &s_options_interface_section );
	Menu_AddItem( &s_options_menu,	( void * ) &s_options_back_action );
}

void Options_MenuDraw (void)
{
	Menu_DrawBanner( "m_banner_options" );

	Menu_AdjustCursor( &s_options_menu, 1 );
	Menu_Draw( &s_options_menu );
}

const char *Options_MenuKey( int key )
{
	return Default_MenuKey( &s_options_menu, key );
}

void M_Menu_Options_f (void)
{
	Options_MenuInit();
	UI_PushMenu ( Options_MenuDraw, Options_MenuKey );
}
