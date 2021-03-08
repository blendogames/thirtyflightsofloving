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

// ui_options_sound.c -- the sound options menu

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

/*
=======================================================================

SOUND MENU

=======================================================================
*/

static menuframework_s	s_options_sound_menu;
static menuseparator_s	s_options_sound_header;
static menuslider_s		s_options_sound_sfxvolume_slider;
static menuslider_s		s_options_sound_musicvolume_slider;
static menulist_s		s_options_sound_oggmusic_box;
static menulist_s		s_options_sound_cdvolume_box;
static menulist_s		s_options_sound_quality_list;
static menulist_s		s_options_sound_compatibility_list;
static menuaction_s		s_options_sound_defaults_action;
static menuaction_s		s_options_sound_back_action;


static void UpdateVolumeFunc ( void *unused )
{
//	Cvar_SetValue( "s_volume", s_options_sound_sfxvolume_slider.curvalue / 10 );
	Cvar_SetValue( "s_volume", MenuSlider_GetValue(&s_options_sound_sfxvolume_slider) );
}

static void UpdateMusicVolumeFunc ( void *unused )
{
//	Cvar_SetValue( "s_musicvolume", s_options_sound_musicvolume_slider.curvalue / 10 );
	Cvar_SetValue( "s_musicvolume", MenuSlider_GetValue(&s_options_sound_musicvolume_slider) );
}

static void UpdateOggMusicFunc ( void *unused )
{
	Cvar_SetValue( "cl_ogg_music", s_options_sound_oggmusic_box.curvalue );
}

static void UpdateCDVolumeFunc ( void *unused )
{
	Cvar_SetValue( "cd_nocd", !s_options_sound_cdvolume_box.curvalue );
}

static void UpdateSoundQualityFunc ( void *unused )
{
	//Knightmare- added DMP's 44/48 KHz sound support
	//** DMP check the newly added sound quality menu options
	switch (s_options_sound_quality_list.curvalue)
	{
	case 1:
		Cvar_SetValue( "s_khz", 22 );
		Cvar_SetValue( "s_loadas8bit", false );
		break;
	case 2:
		Cvar_SetValue( "s_khz", 44 );
		Cvar_SetValue( "s_loadas8bit", false );
		break;
	case 3:
		Cvar_SetValue( "s_khz", 48 );
		Cvar_SetValue( "s_loadas8bit", false );
		break;
	default:
		Cvar_SetValue( "s_khz", 11 );
		Cvar_SetValue( "s_loadas8bit", true );
		break;
	}
	//** DMP end sound menu changes

	Cvar_SetValue ("s_primary", s_options_sound_compatibility_list.curvalue);

	Menu_DrawTextBox (168, 192, 36, 3);
	SCR_DrawString (188, 192+MENU_FONT_SIZE, MENU_FONT_SIZE, ALIGN_CENTER, S_COLOR_ALT"Restarting the sound system. This", FONT_UI, 255);
	SCR_DrawString (188, 192+MENU_FONT_SIZE*2, MENU_FONT_SIZE, ALIGN_CENTER, S_COLOR_ALT"could take up to a minute, so", FONT_UI, 255);
	SCR_DrawString (188, 192+MENU_FONT_SIZE*3, MENU_FONT_SIZE, ALIGN_CENTER, S_COLOR_ALT"please be patient.", FONT_UI, 255);

	// the text box won't show up unless we do a buffer swap
	GLimp_EndFrame();

	CL_Snd_Restart_f();
}

static void SoundSetMenuItemValues( void )
{
//	s_options_sound_sfxvolume_slider.curvalue	= Cvar_VariableValue("s_volume") * 10;
//	s_options_sound_musicvolume_slider.curvalue	= Cvar_VariableValue("s_musicvolume") * 10;
	MenuSlider_SetValue (&s_options_sound_sfxvolume_slider, Cvar_VariableValue("s_volume"));
	MenuSlider_SetValue (&s_options_sound_musicvolume_slider, Cvar_VariableValue("s_musicvolume"));

	s_options_sound_oggmusic_box.curvalue		= (Cvar_VariableValue("cl_ogg_music") > 0);
	s_options_sound_cdvolume_box.curvalue 		= !Cvar_VariableValue("cd_nocd");
	//s_options_sound_quality_list.curvalue			= !Cvar_VariableValue( "s_loadas8bit" );
	//**  DMP convert setting into index for option display text
	switch((int)Cvar_VariableValue("s_khz"))
	{
	case 48:  s_options_sound_quality_list.curvalue = 3;  break;
	case 44:  s_options_sound_quality_list.curvalue = 2;  break;
	case 22:  s_options_sound_quality_list.curvalue = 1;  break;
	default:  s_options_sound_quality_list.curvalue = 0;  break;
	}
	//** DMP end sound menu changes
	s_options_sound_compatibility_list.curvalue = Cvar_VariableValue( "s_primary");
}

static void SoundResetDefaultsFunc ( void *unused )
{
	Cvar_SetToDefault ("s_volume");
	Cvar_SetToDefault ("cd_nocd");
	Cvar_SetToDefault ("cd_loopcount");
	Cvar_SetToDefault ("s_khz");
	Cvar_SetToDefault ("s_loadas8bit");
	Cvar_SetToDefault ("s_primary");

	Menu_DrawTextBox (168, 192, 36, 3);
	SCR_DrawString (188, 192+MENU_FONT_SIZE, MENU_FONT_SIZE, ALIGN_CENTER, S_COLOR_ALT"Restarting the sound system. This", FONT_UI, 255);
	SCR_DrawString (188, 192+MENU_FONT_SIZE*2, MENU_FONT_SIZE, ALIGN_CENTER, S_COLOR_ALT"could take up to a minute, so", FONT_UI, 255);
	SCR_DrawString (188, 192+MENU_FONT_SIZE*3, MENU_FONT_SIZE, ALIGN_CENTER, S_COLOR_ALT"please be patient.", FONT_UI, 255);

	// the text box won't show up unless we do a buffer swap
	GLimp_EndFrame();

	CL_Snd_Restart_f();

	SoundSetMenuItemValues();
}

void Options_Sound_MenuInit ( void )
{
	static const char *cd_music_items[] =
	{
		"disabled",
		"enabled",
		0
	};

	static const char *quality_items[] =
	{
		"low (11KHz/8-bit)",			//** DMP - changed text
		"normal (22KHz/16-bit)",		//** DMP - changed text
		"high (44KHz/16-bit)",			//** DMP - added 44 Khz menu item
		"highest (48KHz/16-bit)",		//** DMP - added 48 Khz menu item
		0
	};

	static const char *compatibility_items[] =
	{
		"max compatibility", "max performance", 0
	};

	int y = 3*MENU_LINE_SIZE;

	s_options_sound_menu.x = SCREEN_WIDTH*0.5;
	s_options_sound_menu.y = SCREEN_HEIGHT*0.5 - 58;
	s_options_sound_menu.nitems = 0;

	s_options_sound_header.generic.type		= MTYPE_SEPARATOR;
	s_options_sound_header.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_options_sound_header.generic.name		= "Sound";
	s_options_sound_header.generic.x		= MENU_HEADER_FONT_SIZE/2 * (int)strlen(s_options_sound_header.generic.name);
	s_options_sound_header.generic.y		= -2*MENU_LINE_SIZE;	// 0

	s_options_sound_sfxvolume_slider.generic.type		= MTYPE_SLIDER;
	s_options_sound_sfxvolume_slider.generic.textSize	= MENU_FONT_SIZE;
	s_options_sound_sfxvolume_slider.generic.x			= 0;
	s_options_sound_sfxvolume_slider.generic.y			= y;
	s_options_sound_sfxvolume_slider.generic.name		= "effects volume";
	s_options_sound_sfxvolume_slider.generic.callback	= UpdateVolumeFunc;
//	s_options_sound_sfxvolume_slider.minvalue			= 0;
//	s_options_sound_sfxvolume_slider.maxvalue			= 10;
//	s_options_sound_sfxvolume_slider.curvalue			= Cvar_VariableValue( "s_volume" ) * 10;
	s_options_sound_sfxvolume_slider.maxPos				= 20;
	s_options_sound_sfxvolume_slider.baseValue			= 0.0f;
	s_options_sound_sfxvolume_slider.increment			= 0.05f;
	s_options_sound_sfxvolume_slider.displayAsPercent	= true;
	MenuSlider_SetValue (&s_options_sound_sfxvolume_slider, Cvar_VariableValue("s_volume"));
	s_options_sound_sfxvolume_slider.generic.statusbar	= "volume of sound effects";

	s_options_sound_musicvolume_slider.generic.type			= MTYPE_SLIDER;
	s_options_sound_musicvolume_slider.generic.textSize		= MENU_FONT_SIZE;
	s_options_sound_musicvolume_slider.generic.x			= 0;
	s_options_sound_musicvolume_slider.generic.y			= y+=MENU_LINE_SIZE;
	s_options_sound_musicvolume_slider.generic.name			= "music volume";
	s_options_sound_musicvolume_slider.generic.callback		= UpdateMusicVolumeFunc;
//	s_options_sound_musicvolume_slider.minvalue				= 0;
//	s_options_sound_musicvolume_slider.maxvalue				= 10;
//	s_options_sound_musicvolume_slider.curvalue				= Cvar_VariableValue( "s_musicvolume" ) * 10;
	s_options_sound_musicvolume_slider.maxPos				= 20;
	s_options_sound_musicvolume_slider.baseValue			= 0.0f;
	s_options_sound_musicvolume_slider.increment			= 0.05f;
	s_options_sound_musicvolume_slider.displayAsPercent		= true;
	MenuSlider_SetValue (&s_options_sound_musicvolume_slider, Cvar_VariableValue("s_musicvolume"));
	s_options_sound_musicvolume_slider.generic.statusbar	= "volume of ogg vorbis music";

	s_options_sound_oggmusic_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_sound_oggmusic_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_sound_oggmusic_box.generic.x			= 0;
	s_options_sound_oggmusic_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_sound_oggmusic_box.generic.name		= "ogg vorbis music";
	s_options_sound_oggmusic_box.generic.callback	= UpdateOggMusicFunc;
	s_options_sound_oggmusic_box.itemnames			= cd_music_items;
	s_options_sound_oggmusic_box.curvalue 			= (Cvar_VariableValue("cl_ogg_music") > 0);
	s_options_sound_oggmusic_box.generic.statusbar	= "override of CD music with ogg vorbis tracks";

	s_options_sound_cdvolume_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_sound_cdvolume_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_sound_cdvolume_box.generic.x			= 0;
	s_options_sound_cdvolume_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_sound_cdvolume_box.generic.name		= "CD music";
	s_options_sound_cdvolume_box.generic.callback	= UpdateCDVolumeFunc;
	s_options_sound_cdvolume_box.itemnames			= cd_music_items;
	s_options_sound_cdvolume_box.curvalue 			= !Cvar_VariableValue("cd_nocd");
	s_options_sound_cdvolume_box.generic.statusbar	= "enables or disables CD music";

	s_options_sound_quality_list.generic.type		= MTYPE_SPINCONTROL;
	s_options_sound_quality_list.generic.textSize	= MENU_FONT_SIZE;
	s_options_sound_quality_list.generic.x			= 0;
	s_options_sound_quality_list.generic.y			= y+=MENU_LINE_SIZE;
	s_options_sound_quality_list.generic.name		= "sound quality";
	s_options_sound_quality_list.generic.callback	= UpdateSoundQualityFunc;
	s_options_sound_quality_list.itemnames			= quality_items;
	s_options_sound_quality_list.curvalue			= !Cvar_VariableValue( "s_loadas8bit" );
	s_options_sound_quality_list.generic.statusbar	= "changes quality of sound";

	s_options_sound_compatibility_list.generic.type			= MTYPE_SPINCONTROL;
	s_options_sound_compatibility_list.generic.textSize		= MENU_FONT_SIZE;
	s_options_sound_compatibility_list.generic.x			= 0;
	s_options_sound_compatibility_list.generic.y			= y+=MENU_LINE_SIZE;
	s_options_sound_compatibility_list.generic.name			= "sound compatibility";
	s_options_sound_compatibility_list.generic.callback		= UpdateSoundQualityFunc;
	s_options_sound_compatibility_list.itemnames			= compatibility_items;
	s_options_sound_compatibility_list.curvalue				= Cvar_VariableValue( "s_primary" );
	s_options_sound_compatibility_list.generic.statusbar	= "changes buffering mode of sound system";

	s_options_sound_defaults_action.generic.type		= MTYPE_ACTION;
	s_options_sound_defaults_action.generic.textSize	= MENU_FONT_SIZE;
	s_options_sound_defaults_action.generic.x			= MENU_FONT_SIZE;
	s_options_sound_defaults_action.generic.y			= 18*MENU_LINE_SIZE;
	s_options_sound_defaults_action.generic.name		= "reset defaults";
	s_options_sound_defaults_action.generic.callback	= SoundResetDefaultsFunc;
	s_options_sound_defaults_action.generic.statusbar	= "resets all sound settings to internal defaults";

	s_options_sound_back_action.generic.type			= MTYPE_ACTION;
	s_options_sound_back_action.generic.textSize		= MENU_FONT_SIZE;
	s_options_sound_back_action.generic.x				= MENU_FONT_SIZE;
	s_options_sound_back_action.generic.y				= 20*MENU_LINE_SIZE;
	s_options_sound_back_action.generic.name			= "back to options";
	s_options_sound_back_action.generic.callback		= UI_BackMenu;

	Menu_AddItem( &s_options_sound_menu, ( void * ) &s_options_sound_header );
	Menu_AddItem( &s_options_sound_menu, ( void * ) &s_options_sound_sfxvolume_slider );
	Menu_AddItem( &s_options_sound_menu, ( void * ) &s_options_sound_musicvolume_slider );
	Menu_AddItem( &s_options_sound_menu, ( void * ) &s_options_sound_oggmusic_box );
	Menu_AddItem( &s_options_sound_menu, ( void * ) &s_options_sound_cdvolume_box );
	Menu_AddItem( &s_options_sound_menu, ( void * ) &s_options_sound_quality_list );
	Menu_AddItem( &s_options_sound_menu, ( void * ) &s_options_sound_compatibility_list );
	Menu_AddItem( &s_options_sound_menu, ( void * ) &s_options_sound_defaults_action );
	Menu_AddItem( &s_options_sound_menu, ( void * ) &s_options_sound_back_action );

	SoundSetMenuItemValues();
}

void Options_Sound_MenuDraw (void)
{
	Menu_DrawBanner( "m_banner_options" );

	Menu_AdjustCursor( &s_options_sound_menu, 1 );
	Menu_Draw( &s_options_sound_menu );
}

const char *Options_Sound_MenuKey( int key )
{
	return Default_MenuKey( &s_options_sound_menu, key );
}

void M_Menu_Options_Sound_f (void)
{
	Options_Sound_MenuInit();
	UI_PushMenu ( Options_Sound_MenuDraw, Options_Sound_MenuKey );
}
