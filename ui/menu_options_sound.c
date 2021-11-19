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

// menu_options_sound.c -- the sound options menu

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

//=======================================================================

static void VolumeFunc (void *unused)
{
	UI_MenuSlider_SaveValue (&s_options_sound_sfxvolume_slider, "s_volume");
}

static void MusicVolumeFunc (void *unused)
{
	UI_MenuSlider_SaveValue (&s_options_sound_musicvolume_slider, "s_musicvolume");
}

static void OggMusicFunc (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_options_sound_oggmusic_box, "cl_ogg_music");
}

static void CDVolumeFunc (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_options_sound_cdvolume_box, "cd_nocd");
}

//=======================================================================

void M_Sound_DrawUpdateMessage (void)
{
	UI_DrawPopupMessage ("Restarting the sound system.\nThis could take up to a minute,\nso please be patient.");
}

static void M_UpdateSoundQualityFunc (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_options_sound_quality_list, "s_khz");
	Cvar_SetInteger ("s_loadas8bit", (s_options_sound_quality_list.curValue == 0));

	UI_MenuSpinControl_SaveValue (&s_options_sound_compatibility_list, "s_primary");

	// show update wait message
	M_Sound_DrawUpdateMessage ();	
	CL_Snd_Restart_f ();
}

static void M_SoundSetMenuItemValues (void)
{
	UI_MenuSlider_SetValue (&s_options_sound_sfxvolume_slider, "s_volume", 0.0f, 1.0f, true);
	UI_MenuSlider_SetValue (&s_options_sound_musicvolume_slider, "s_musicvolume", 0.0f, 1.0f, true);
	UI_MenuSpinControl_SetValue (&s_options_sound_oggmusic_box, "cl_ogg_music", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_options_sound_cdvolume_box, "cd_nocd", 0, 0, false);
	UI_MenuSpinControl_SetValue (&s_options_sound_quality_list, "s_khz", 0, 0, false);
	UI_MenuSpinControl_SetValue (&s_options_sound_compatibility_list, "s_primary", 0, 1, true);
}

static void M_SoundResetDefaultsFunc (void *unused)
{
	Cvar_SetToDefault ("s_volume");
	Cvar_SetToDefault ("cd_nocd");
	Cvar_SetToDefault ("cd_loopcount");
	Cvar_SetToDefault ("s_khz");
	Cvar_SetToDefault ("s_loadas8bit");
	Cvar_SetToDefault ("s_primary");

	// show update wait message
	M_Sound_DrawUpdateMessage ();
	CL_Snd_Restart_f ();
	M_SoundSetMenuItemValues ();
}

//=======================================================================

void Menu_Options_Sound_Init (void)
{
	static const char *cd_music_items[] =
	{
		"disabled",
		"enabled",
		0
	};
	static const char *cd_music_values[] =
	{
		"1",
		"0",
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
	static const char *quality_values[] =
	{
		"11",
		"22",	
		"44",
		"48",
		0
	};
	static const char *compatibility_items[] =
	{
		"max compatibility",
		"max performance",
		0
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
	s_options_sound_sfxvolume_slider.generic.callback	= VolumeFunc;
	s_options_sound_sfxvolume_slider.maxPos				= 20;
	s_options_sound_sfxvolume_slider.baseValue			= 0.0f;
	s_options_sound_sfxvolume_slider.increment			= 0.05f;
	s_options_sound_sfxvolume_slider.displayAsPercent	= true;
	s_options_sound_sfxvolume_slider.generic.statusbar	= "volume of sound effects";
	UI_MenuSlider_SetValue (&s_options_sound_sfxvolume_slider, "s_volume", 0.0f, 1.0f, true);

	s_options_sound_musicvolume_slider.generic.type			= MTYPE_SLIDER;
	s_options_sound_musicvolume_slider.generic.textSize		= MENU_FONT_SIZE;
	s_options_sound_musicvolume_slider.generic.x			= 0;
	s_options_sound_musicvolume_slider.generic.y			= y+=MENU_LINE_SIZE;
	s_options_sound_musicvolume_slider.generic.name			= "music volume";
	s_options_sound_musicvolume_slider.generic.callback		= MusicVolumeFunc;
	s_options_sound_musicvolume_slider.maxPos				= 20;
	s_options_sound_musicvolume_slider.baseValue			= 0.0f;
	s_options_sound_musicvolume_slider.increment			= 0.05f;
	s_options_sound_musicvolume_slider.displayAsPercent		= true;
	s_options_sound_musicvolume_slider.generic.statusbar	= "volume of ogg vorbis music";
	UI_MenuSlider_SetValue (&s_options_sound_musicvolume_slider, "s_musicvolume", 0.0f, 1.0f, true);

	s_options_sound_oggmusic_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_sound_oggmusic_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_sound_oggmusic_box.generic.x			= 0;
	s_options_sound_oggmusic_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_sound_oggmusic_box.generic.name		= "ogg vorbis music";
	s_options_sound_oggmusic_box.generic.callback	= OggMusicFunc;
	s_options_sound_oggmusic_box.itemNames			= cd_music_items;
	s_options_sound_oggmusic_box.generic.statusbar	= "override of CD music with ogg vorbis tracks";
	UI_MenuSpinControl_SetValue (&s_options_sound_oggmusic_box,"cl_ogg_music", 0, 1, true);

	s_options_sound_cdvolume_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_sound_cdvolume_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_sound_cdvolume_box.generic.x			= 0;
	s_options_sound_cdvolume_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_sound_cdvolume_box.generic.name		= "CD music";
	s_options_sound_cdvolume_box.generic.callback	= CDVolumeFunc;
	s_options_sound_cdvolume_box.itemNames			= cd_music_items;
	s_options_sound_cdvolume_box.itemValues			= cd_music_values;
	s_options_sound_cdvolume_box.generic.statusbar	= "enables or disables CD music";
	UI_MenuSpinControl_SetValue (&s_options_sound_cdvolume_box, "cd_nocd", 0, 0, false);

	s_options_sound_quality_list.generic.type		= MTYPE_SPINCONTROL;
	s_options_sound_quality_list.generic.textSize	= MENU_FONT_SIZE;
	s_options_sound_quality_list.generic.x			= 0;
	s_options_sound_quality_list.generic.y			= y+=MENU_LINE_SIZE;
	s_options_sound_quality_list.generic.name		= "sound quality";
	s_options_sound_quality_list.generic.callback	= M_UpdateSoundQualityFunc;
	s_options_sound_quality_list.itemNames			= quality_items;
	s_options_sound_quality_list.itemValues			= quality_values;
	s_options_sound_quality_list.generic.statusbar	= "changes quality of sound";
	UI_MenuSpinControl_SetValue (&s_options_sound_quality_list, "s_khz", 0, 0, false);

	s_options_sound_compatibility_list.generic.type			= MTYPE_SPINCONTROL;
	s_options_sound_compatibility_list.generic.textSize		= MENU_FONT_SIZE;
	s_options_sound_compatibility_list.generic.x			= 0;
	s_options_sound_compatibility_list.generic.y			= y+=MENU_LINE_SIZE;
	s_options_sound_compatibility_list.generic.name			= "sound compatibility";
	s_options_sound_compatibility_list.generic.callback		= M_UpdateSoundQualityFunc;
	s_options_sound_compatibility_list.itemNames			= compatibility_items;
	s_options_sound_compatibility_list.generic.statusbar	= "changes buffering mode of sound system";
	UI_MenuSpinControl_SetValue (&s_options_sound_compatibility_list, "s_primary", 0, 1, true);

	s_options_sound_defaults_action.generic.type		= MTYPE_ACTION;
	s_options_sound_defaults_action.generic.textSize	= MENU_FONT_SIZE;
	s_options_sound_defaults_action.generic.x			= MENU_FONT_SIZE;
	s_options_sound_defaults_action.generic.y			= 18*MENU_LINE_SIZE;
	s_options_sound_defaults_action.generic.name		= "Reset to Defaults";
	s_options_sound_defaults_action.generic.callback	= M_SoundResetDefaultsFunc;
	s_options_sound_defaults_action.generic.statusbar	= "resets all sound settings to internal defaults";

	s_options_sound_back_action.generic.type			= MTYPE_ACTION;
	s_options_sound_back_action.generic.textSize		= MENU_FONT_SIZE;
	s_options_sound_back_action.generic.x				= MENU_FONT_SIZE;
	s_options_sound_back_action.generic.y				= 20*MENU_LINE_SIZE;
	s_options_sound_back_action.generic.name			= "Back to Options";
	s_options_sound_back_action.generic.callback		= UI_BackMenu;

	UI_AddMenuItem (&s_options_sound_menu, (void *) &s_options_sound_header);
	UI_AddMenuItem (&s_options_sound_menu, (void *) &s_options_sound_sfxvolume_slider);
	UI_AddMenuItem (&s_options_sound_menu, (void *) &s_options_sound_musicvolume_slider);
	UI_AddMenuItem (&s_options_sound_menu, (void *) &s_options_sound_oggmusic_box);
	UI_AddMenuItem (&s_options_sound_menu, (void *) &s_options_sound_cdvolume_box);
	UI_AddMenuItem (&s_options_sound_menu, (void *) &s_options_sound_quality_list);
	UI_AddMenuItem (&s_options_sound_menu, (void *) &s_options_sound_compatibility_list);
	UI_AddMenuItem (&s_options_sound_menu, (void *) &s_options_sound_defaults_action);
	UI_AddMenuItem (&s_options_sound_menu, (void *) &s_options_sound_back_action);

	M_SoundSetMenuItemValues ();
}

void Menu_Options_Sound_Draw (void)
{
	UI_DrawBanner ("m_banner_options");

	UI_AdjustMenuCursor (&s_options_sound_menu, 1);
	UI_DrawMenu (&s_options_sound_menu);
}

const char *Menu_Options_Sound_Key (int key)
{
	return UI_DefaultMenuKey (&s_options_sound_menu, key);
}

void Menu_Options_Sound_f (void)
{
	Menu_Options_Sound_Init ();
	UI_PushMenu (Menu_Options_Sound_Draw, Menu_Options_Sound_Key);
}
