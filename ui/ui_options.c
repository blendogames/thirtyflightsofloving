/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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
static menuaction_s		s_options_ingame_section;
static menuaction_s		s_options_interface_section;
static menuaction_s		s_options_back_action;
static menuaction_s		s_options_video_section;


static menuframework_s	s_dialogue_menu;
static menuaction_s	s_dialogue_text_section;
static menuaction_s		s_dialogue_ok_section;



static void MenuSoundFunc ( void *unused )
{
	M_Menu_Options_Sound_f();
}

static void MenuControlsFunc ( void *unused )
{
	M_Menu_Options_Controls_f();
}

static void MenuIngameFunc ( void *unused )
{
	M_Menu_Options_Ingame_f();
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
//	s_options_menu.x = viddef.width * 0.50 - SCR_ScaledVideo(3*MENU_FONT_SIZE);
//	s_options_menu.y = viddef.height * 0.50 - SCR_ScaledVideo(7.25*MENU_FONT_SIZE); //58
	s_options_menu.nitems = 0;

	s_options_sound_section.generic.type	= MTYPE_ACTION;
	s_options_sound_section.generic.flags  = QMF_LEFT_JUSTIFY;
	s_options_sound_section.generic.name	= "Sound";
	s_options_sound_section.generic.x		= 0; //MENU_FONT_SIZE*0.5*strlen(s_options_sound_section.generic.name);
	s_options_sound_section.generic.y		= MENU_FONT_SIZE * 2;
	s_options_sound_section.generic.callback = MenuSoundFunc;
	//s_options_sound_section.generic.cursor_offset = -(MENU_FONT_SIZE*10);
	
	s_options_controls_section.generic.type	= MTYPE_ACTION;
	s_options_controls_section.generic.flags  = QMF_LEFT_JUSTIFY;
	s_options_controls_section.generic.name	= "Controls";
	s_options_controls_section.generic.x		= 0; //MENU_FONT_SIZE*0.5*strlen(s_options_controls_section.generic.name);
	s_options_controls_section.generic.y		= MENU_FONT_SIZE * 4;
	s_options_controls_section.generic.callback = MenuControlsFunc;
	//s_options_controls_section.generic.cursor_offset = -(MENU_FONT_SIZE*10);
	
	s_options_ingame_section.generic.type	= MTYPE_ACTION;
	s_options_ingame_section.generic.flags  = QMF_LEFT_JUSTIFY;
	s_options_ingame_section.generic.name	= " ingame";
	s_options_ingame_section.generic.x		= 0; //MENU_FONT_SIZE*0.5*strlen(s_options_ingame_section.generic.name);
	s_options_ingame_section.generic.y		= MENU_FONT_SIZE * 6;
	s_options_ingame_section.generic.callback = MenuIngameFunc;
	//s_options_ingame_section.generic.cursor_offset = -(MENU_FONT_SIZE*10);

	s_options_interface_section.generic.type	= MTYPE_ACTION;
	s_options_interface_section.generic.flags  = QMF_LEFT_JUSTIFY;
	s_options_interface_section.generic.name	= " interface";
	s_options_interface_section.generic.x		= 0; //MENU_FONT_SIZE*0.5*strlen(s_options_interface_section.generic.name);
	s_options_interface_section.generic.y		= MENU_FONT_SIZE * 8;
	s_options_interface_section.generic.callback = MenuInterfaceFunc;
	//s_options_interface_section.generic.cursor_offset = -(MENU_FONT_SIZE*10);

	s_options_back_action.generic.type	= MTYPE_ACTION;
	s_options_back_action.generic.flags  = QMF_LEFT_JUSTIFY;
	s_options_back_action.generic.name	= "Cancel";
	s_options_back_action.generic.x		= 0; //MENU_FONT_SIZE*0.5*strlen(s_options_back_action.generic.name);
	s_options_back_action.generic.y		= MENU_FONT_SIZE * 11;
	s_options_back_action.generic.callback = UI_BackMenu;
	//s_options_back_action.generic.cursor_offset = -(MENU_FONT_SIZE*10);

		s_options_video_section.generic.type	= MTYPE_ACTION;
		s_options_video_section.generic.flags  = QMF_LEFT_JUSTIFY;
		s_options_video_section.generic.name	= "Display";
		s_options_video_section.generic.x		= 0; //MENU_FONT_SIZE*0.5*strlen(s_options_sound_section.generic.name);
		s_options_video_section.generic.y		= MENU_FONT_SIZE * 6;
		s_options_video_section.generic.callback = M_Menu_Video_f;

	Menu_AddItem( &s_options_menu,	( void * ) &s_options_sound_section );
	Menu_AddItem( &s_options_menu,	( void * ) &s_options_controls_section );
	//Menu_AddItem( &s_options_menu,	( void * ) &s_options_ingame_section );
	//Menu_AddItem( &s_options_menu,	( void * ) &s_options_interface_section );
	Menu_AddItem( &s_options_menu,	( void * ) &s_options_video_section );
	Menu_AddItem( &s_options_menu,	( void * ) &s_options_back_action );
	

}



void Options_MenuDraw (void)
{
	menucommon_s *citem;

	Menu_Banner( "m_banner_options" );
	Menu_AdjustCursor( &s_options_menu, 1 );
	Menu_Draw( &s_options_menu );

	citem = Menu_ItemAtCursor( &s_options_menu );
	if ( citem )
	{
		char	name[16];
		Com_sprintf (name, sizeof(name), "c_%s", citem->name);
		icondraw( name );
	}
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






