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

// menu_options_interface.c -- the interface options menu

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

/*
=======================================================================

INTERFACE MENU

=======================================================================
*/

static menuframework_s	s_options_interface_menu;
static menuseparator_s	s_options_interface_header;
static menuslider_s		s_options_interface_conalpha_slider;
//static menuslider_s		s_options_interface_conheight_slider;
static menuslider_s		s_options_interface_menumouse_slider;
static menuslider_s		s_options_interface_menualpha_slider;
static menulist_s		s_options_interface_confont_box;
static menuslider_s		s_options_interface_fontsize_slider;
static menulist_s		s_options_interface_uifont_box;
static menulist_s		s_options_interface_scrfont_box;
static menulist_s		s_options_interface_alt_text_color_box;
static menulist_s		s_options_interface_simple_loadscreen_box;
static menulist_s		s_options_interface_newconback_box;
static menulist_s		s_options_interface_noalttab_box;
static menuaction_s		s_options_interface_defaults_action;
static menuaction_s		s_options_interface_back_action;

cvar_t *con_font;
cvar_t *ui_font;
cvar_t *scr_font;

static void MouseMenuFunc (void *unused)
{
//	Cvar_SetValue( "ui_sensitivity", s_options_interface_menumouse_slider.curvalue / 4.0f );
	Cvar_SetValue( "ui_sensitivity", UI_MenuSlider_GetValue(&s_options_interface_menumouse_slider) );
}

// menu alpha option
static void MenuAlphaFunc (void *unused)
{
//	Cvar_SetValue( "ui_background_alpha", s_options_interface_menualpha_slider.curvalue / 20.0f);
	Cvar_SetValue( "ui_background_alpha", UI_MenuSlider_GetValue(&s_options_interface_menualpha_slider) );
}

static void AltTextColorFunc (void *unused)
{
	Cvar_SetValue( "alt_text_color", s_options_interface_alt_text_color_box.curvalue);
}

// Psychospaz's transparent console
static void ConAlphaFunc (void *unused)
{
//	Cvar_SetValue( "scr_conalpha", s_options_interface_conalpha_slider.curvalue * 0.05 );
	Cvar_SetValue( "scr_conalpha", UI_MenuSlider_GetValue(&s_options_interface_conalpha_slider) );
}

// variable console height
/*static void ConHeightFunc (void *unused)
{
//	Cvar_SetValue( "scr_conheight", 0.25 + (s_options_interface_conheight_slider.curvalue * 0.05) );
	Cvar_SetValue( "scr_conheight", UI_MenuSlider_GetValue(&s_options_interface_conheight_slider) );
}*/

static void SimpleLoadscreenFunc (void *unused)
{
	Cvar_SetValue( "scr_simple_loadscreen", s_options_interface_simple_loadscreen_box.curvalue );
}

static void NewConbackFunc (void *unused)
{
	Cvar_SetValue( "scr_newconback", s_options_interface_newconback_box.curvalue );
}

static void NoAltTabFunc (void *unused)
{
	Cvar_SetValue( "win_noalttab", s_options_interface_noalttab_box.curvalue );
}

static void FontSizeFunc (void *unused)
{
//	Cvar_SetValue( "con_font_size", s_options_interface_fontsize_slider.curvalue * 2 );
	Cvar_SetValue( "con_font_size", UI_MenuSlider_GetValue(&s_options_interface_fontsize_slider) );
}

static void ConFontFunc (void *unused)
{
	Cvar_Set( "con_font", ui_font_names[s_options_interface_confont_box.curvalue] );
}

static void UIFontFunc (void *unused)
{
	Cvar_Set( "ui_font", ui_font_names[s_options_interface_uifont_box.curvalue] );
}

static void ScrFontFunc (void *unused)
{
	Cvar_Set( "scr_font", ui_font_names[s_options_interface_scrfont_box.curvalue] );
}

void SetFontCursor (void)
{
	int i;

	s_options_interface_confont_box.curvalue = 0;
	s_options_interface_uifont_box.curvalue = 0;
	s_options_interface_scrfont_box.curvalue = 0;

	if (!con_font)
		con_font = Cvar_Get ("con_font", "default", CVAR_ARCHIVE);
	if (!ui_font)
		ui_font = Cvar_Get ("ui_font", "default", CVAR_ARCHIVE);
	if (!scr_font)
		scr_font = Cvar_Get ("scr_font", "default", CVAR_ARCHIVE);

	if (ui_numfonts > 1)
	{
		for (i=0; ui_font_names[i]; i++)
		{
			if (!Q_strcasecmp(con_font->string, ui_font_names[i]))
			{
				s_options_interface_confont_box.curvalue = i;
				break;
			}
		}
		for (i=0; ui_font_names[i]; i++)
		{
			if (!Q_strcasecmp(ui_font->string, ui_font_names[i]))
			{
				s_options_interface_uifont_box.curvalue = i;
				break;
			}
		}
		for (i=0; ui_font_names[i]; i++)
		{
			if (!Q_strcasecmp(scr_font->string, ui_font_names[i]))
			{
				s_options_interface_scrfont_box.curvalue = i;
				break;
			}
		}
	}
}

//=======================================================================

static void InterfaceSetMenuItemValues (void)
{
	SetFontCursor ();

//	s_options_interface_menumouse_slider.curvalue		= ( Cvar_VariableValue("ui_sensitivity") ) * 4;
//	s_options_interface_menualpha_slider.curvalue		= ( Cvar_VariableValue("ui_background_alpha") ) * 20;
//	s_options_interface_fontsize_slider.curvalue		= ( Cvar_VariableValue("con_font_size") ) * 0.5;
	UI_MenuSlider_SetValue (&s_options_interface_menumouse_slider, Cvar_VariableValue("ui_sensitivity"));
	UI_MenuSlider_SetValue (&s_options_interface_menualpha_slider, Cvar_VariableValue("ui_background_alpha"));
	UI_MenuSlider_SetValue (&s_options_interface_fontsize_slider, Cvar_VariableValue("con_font_size"));

	Cvar_SetValue( "alt_text_color", ClampCvar( 0, 9, Cvar_VariableValue("alt_text_color") ) );
	s_options_interface_alt_text_color_box.curvalue	= Cvar_VariableValue("alt_text_color");

	Cvar_SetValue( "scr_conalpha", ClampCvar( 0, 1, Cvar_VariableValue("scr_conalpha") ) );
//	s_options_interface_conalpha_slider.curvalue		= ( Cvar_VariableValue("scr_conalpha") ) * 20;
	UI_MenuSlider_SetValue (&s_options_interface_conalpha_slider, Cvar_VariableValue("scr_conalpha"));

//	Cvar_SetValue( "scr_conheight", ClampCvar( 0.25, 0.75, Cvar_VariableValue("scr_conheight") ) );
//	s_options_interface_conheight_slider.curvalue		= 20 * (Cvar_VariableValue("scr_conheight") - 0.25);
//	UI_MenuSlider_SetValue (&s_options_interface_conheight_slider, Cvar_VariableValue("scr_conheight"));

	Cvar_SetValue( "scr_simple_loadscreen", ClampCvar( 0, 1, Cvar_VariableValue("scr_simple_loadscreen") ) );
	s_options_interface_simple_loadscreen_box.curvalue = Cvar_VariableValue("scr_simple_loadscreen");

	Cvar_SetValue( "scr_newconback", ClampCvar( 0, 1, Cvar_VariableValue("scr_newconback") ) );
	s_options_interface_newconback_box.curvalue = Cvar_VariableValue("scr_newconback");

	s_options_interface_noalttab_box.curvalue			= Cvar_VariableValue("win_noalttab");
}

static void InterfaceResetDefaultsFunc (void *unused)
{
	Cvar_SetToDefault ("ui_sensitivity");	
	Cvar_SetToDefault ("ui_background_alpha");	
	Cvar_SetToDefault ("con_font");	
	Cvar_SetToDefault ("con_font_size");	
	Cvar_SetToDefault ("ui_font");	
	Cvar_SetToDefault ("scr_font");	
	Cvar_SetToDefault ("alt_text_color");	
	Cvar_SetToDefault ("scr_alpha");	
//	Cvar_SetToDefault ("scr_height");	
	Cvar_SetToDefault ("scr_simple_loadscreen");
	Cvar_SetToDefault ("scr_newconback");
	Cvar_SetToDefault ("win_noalttab");	

	InterfaceSetMenuItemValues ();
}

void Options_Interface_MenuInit (void)
{
	static const char *yesno_names[] =
	{
		"no",
		"yes",
		0
	};

	static const char *textcolor_names[] =
	{
		"gray",
		"red",
		"green",
		"yellow",
		"blue",
		"cyan",
		"magenta",
		"white",
		"black",
		"orange",
		0
	};

	int y = 3*MENU_LINE_SIZE;

	s_options_interface_menu.x = SCREEN_WIDTH*0.5;
	s_options_interface_menu.y = SCREEN_HEIGHT*0.5 - 58;
	s_options_interface_menu.nitems = 0;

	s_options_interface_header.generic.type		= MTYPE_SEPARATOR;
	s_options_interface_header.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_options_interface_header.generic.name		= "Interface";
	s_options_interface_header.generic.x		= MENU_HEADER_FONT_SIZE/2 * (int)strlen(s_options_interface_header.generic.name);
	s_options_interface_header.generic.y		= -2*MENU_LINE_SIZE;	// 0

	// Knightmare- Psychospaz's menu mouse support
	s_options_interface_menumouse_slider.generic.type		= MTYPE_SLIDER;
	s_options_interface_menumouse_slider.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_menumouse_slider.generic.x			= 0;
	s_options_interface_menumouse_slider.generic.y			= y;
	s_options_interface_menumouse_slider.generic.name		= "mouse speed";
	s_options_interface_menumouse_slider.generic.callback	= MouseMenuFunc;
//	s_options_interface_menumouse_slider.minvalue			= 1;
//	s_options_interface_menumouse_slider.maxvalue			= 8;
	s_options_interface_menumouse_slider.maxPos				= 7;
	s_options_interface_menumouse_slider.baseValue			= 0.25f;
	s_options_interface_menumouse_slider.increment			= 0.25f;
	s_options_interface_menumouse_slider.displayAsPercent	= false;
	s_options_interface_menumouse_slider.generic.statusbar	= "changes sensitivity of mouse in menus";

	s_options_interface_menualpha_slider.generic.type		= MTYPE_SLIDER;
	s_options_interface_menualpha_slider.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_menualpha_slider.generic.x			= 0;
	s_options_interface_menualpha_slider.generic.y			= y+=MENU_LINE_SIZE;
	s_options_interface_menualpha_slider.generic.name		= "ingame menu transparency";
	s_options_interface_menualpha_slider.generic.callback	= MenuAlphaFunc;
//	s_options_interface_menualpha_slider.minvalue			= 0;
//	s_options_interface_menualpha_slider.maxvalue			= 20;
	s_options_interface_menualpha_slider.maxPos				= 20;
	s_options_interface_menualpha_slider.baseValue			= 0.0f;
	s_options_interface_menualpha_slider.increment			= 0.05f;
	s_options_interface_menualpha_slider.displayAsPercent	= true;
	s_options_interface_menualpha_slider.generic.statusbar	= "changes opacity of menu background";

#if 0
	// free any loaded fonts to prevent memory leak
	if (ui_numfonts > 0) {
		FS_FreeFileList (ui_font_names, ui_numfonts);
	}
	ui_numfonts = 0;
	ui_font_names = UI_SetFontNames ();
#endif

	s_options_interface_confont_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_interface_confont_box.generic.textSize		= MENU_FONT_SIZE;
	s_options_interface_confont_box.generic.x				= 0;
	s_options_interface_confont_box.generic.y				= y+=2*MENU_LINE_SIZE;
	s_options_interface_confont_box.generic.name			= "console font";
	s_options_interface_confont_box.generic.callback		= ConFontFunc;
	s_options_interface_confont_box.itemnames				= ui_font_names;
	s_options_interface_confont_box.generic.statusbar		= "changes font of console text";

	s_options_interface_fontsize_slider.generic.type		= MTYPE_SLIDER;
	s_options_interface_fontsize_slider.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_fontsize_slider.generic.x			= 0;
	s_options_interface_fontsize_slider.generic.y			= y+=MENU_LINE_SIZE;
	s_options_interface_fontsize_slider.generic.name		= "console font size";
	s_options_interface_fontsize_slider.generic.callback	= FontSizeFunc;
//	s_options_interface_fontsize_slider.minvalue			= 3;
//	s_options_interface_fontsize_slider.maxvalue			= 8;
	s_options_interface_fontsize_slider.maxPos				= 5;
	s_options_interface_fontsize_slider.baseValue			= 6.0f;
	s_options_interface_fontsize_slider.increment			= 2.0f;
	s_options_interface_fontsize_slider.displayAsPercent	= false;
	s_options_interface_fontsize_slider.generic.statusbar	= "changes size of console text";

	s_options_interface_uifont_box.generic.type				= MTYPE_SPINCONTROL;
	s_options_interface_uifont_box.generic.textSize			= MENU_FONT_SIZE;
	s_options_interface_uifont_box.generic.x				= 0;
	s_options_interface_uifont_box.generic.y				= y+=MENU_LINE_SIZE;
	s_options_interface_uifont_box.generic.name				= "menu font";
	s_options_interface_uifont_box.generic.callback			= UIFontFunc;
	s_options_interface_uifont_box.itemnames				= ui_font_names;
	s_options_interface_uifont_box.generic.statusbar		= "changes font of menu text";

	s_options_interface_scrfont_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_interface_scrfont_box.generic.textSize		= MENU_FONT_SIZE;
	s_options_interface_scrfont_box.generic.x				= 0;
	s_options_interface_scrfont_box.generic.y				= y+=MENU_LINE_SIZE;
	s_options_interface_scrfont_box.generic.name			= "HUD font";
	s_options_interface_scrfont_box.generic.callback		= ScrFontFunc;
	s_options_interface_scrfont_box.itemnames				= ui_font_names;
	s_options_interface_scrfont_box.generic.statusbar		= "changes font of HUD text";

	s_options_interface_alt_text_color_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_interface_alt_text_color_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_alt_text_color_box.generic.x		= 0;
	s_options_interface_alt_text_color_box.generic.y		= y+=MENU_LINE_SIZE;
	s_options_interface_alt_text_color_box.generic.name		= "alt text color";
	s_options_interface_alt_text_color_box.generic.callback	= AltTextColorFunc;
	s_options_interface_alt_text_color_box.itemnames		= textcolor_names;
	s_options_interface_alt_text_color_box.generic.statusbar	= "changes color of highlighted text";

	s_options_interface_conalpha_slider.generic.type		= MTYPE_SLIDER;
	s_options_interface_conalpha_slider.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_conalpha_slider.generic.x			= 0;
	s_options_interface_conalpha_slider.generic.y			= y+=2*MENU_LINE_SIZE;
	s_options_interface_conalpha_slider.generic.name		= "console transparency";
	s_options_interface_conalpha_slider.generic.callback	= ConAlphaFunc;
//	s_options_interface_conalpha_slider.minvalue			= 0;
//	s_options_interface_conalpha_slider.maxvalue			= 20;
	s_options_interface_conalpha_slider.maxPos				= 20;
	s_options_interface_conalpha_slider.baseValue			= 0.0f;
	s_options_interface_conalpha_slider.increment			= 0.05f;
	s_options_interface_conalpha_slider.displayAsPercent	= true;
	s_options_interface_conalpha_slider.generic.statusbar	= "changes opacity of console";

	/*
	s_options_interface_conheight_slider.generic.type		= MTYPE_SLIDER;
	s_options_interface_conheight_slider.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_conheight_slider.generic.x			= 0;
	s_options_interface_conheight_slider.generic.y			= y+=MENU_LINE_SIZE;
	s_options_interface_conheight_slider.generic.name		= "console height";
	s_options_interface_conheight_slider.generic.callback	= ConHeightFunc;
//	s_options_interface_conheight_slider.minvalue			= 0;
//	s_options_interface_conheight_slider.maxvalue			= 10;
	s_options_interface_conheight_slider.maxPos				= 10;
	s_options_interface_conheight_slider.baseValue			= 0.25f;
	s_options_interface_conheight_slider.increment			= 0.05f;
	s_options_interface_conheight_slider.displayAsPercent	= false;
	s_options_interface_conheight_slider.generic.statusbar	= "changes drop height of console";
	*/

	s_options_interface_simple_loadscreen_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_interface_simple_loadscreen_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_simple_loadscreen_box.generic.x			= 0;
	s_options_interface_simple_loadscreen_box.generic.y			= y+=2*MENU_LINE_SIZE;
	s_options_interface_simple_loadscreen_box.generic.name		= "simple load screens";
	s_options_interface_simple_loadscreen_box.generic.callback	= SimpleLoadscreenFunc;
	s_options_interface_simple_loadscreen_box.itemnames			= yesno_names;
	s_options_interface_simple_loadscreen_box.generic.statusbar	= "toggles simple map load screen";

	s_options_interface_newconback_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_interface_newconback_box.generic.textSize		= MENU_FONT_SIZE;
	s_options_interface_newconback_box.generic.x			= 0;
	s_options_interface_newconback_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_interface_newconback_box.generic.name			= "new console background";
	s_options_interface_newconback_box.generic.callback		= NewConbackFunc;
	s_options_interface_newconback_box.itemnames			= yesno_names;
	s_options_interface_newconback_box.generic.statusbar	= "enables Q3-style console background";

	s_options_interface_noalttab_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_interface_noalttab_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_noalttab_box.generic.x			= 0;
	s_options_interface_noalttab_box.generic.y			= y+=2*MENU_LINE_SIZE;
	s_options_interface_noalttab_box.generic.name		= "disable alt-tab";
	s_options_interface_noalttab_box.generic.callback	= NoAltTabFunc;
	s_options_interface_noalttab_box.itemnames			= yesno_names;
	s_options_interface_noalttab_box.generic.statusbar	= "disables alt-tabbing to desktop";

	s_options_interface_defaults_action.generic.type		= MTYPE_ACTION;
	s_options_interface_defaults_action.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_defaults_action.generic.x			= MENU_FONT_SIZE;
	s_options_interface_defaults_action.generic.y			= 18*MENU_LINE_SIZE;
	s_options_interface_defaults_action.generic.name		= "reset defaults";
	s_options_interface_defaults_action.generic.callback	= InterfaceResetDefaultsFunc;
	s_options_interface_defaults_action.generic.statusbar	= "resets all interface settings to internal defaults";

	s_options_interface_back_action.generic.type		= MTYPE_ACTION;
	s_options_interface_back_action.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_back_action.generic.x			= MENU_FONT_SIZE;
	s_options_interface_back_action.generic.y			= 20*MENU_LINE_SIZE;
	s_options_interface_back_action.generic.name		= "back to options";
	s_options_interface_back_action.generic.callback	= UI_BackMenu;

	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_header);
	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_menumouse_slider);
	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_menualpha_slider);
	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_confont_box);
	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_fontsize_slider);
	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_uifont_box);
	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_scrfont_box);
	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_alt_text_color_box);
	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_conalpha_slider);
//	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_conheight_slider);
	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_simple_loadscreen_box);
	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_newconback_box);
	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_noalttab_box);
	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_defaults_action);
	UI_AddMenuItem (&s_options_interface_menu, (void *) &s_options_interface_back_action);

	InterfaceSetMenuItemValues ();
}

void Menu_Options_Interface_Draw (void)
{
	UI_DrawBanner ("m_banner_options");

	UI_AdjustMenuCursor  (&s_options_interface_menu, 1);
	UI_DrawMenu (&s_options_interface_menu);
}

const char *Menu_Options_Interface_Key (int key)
{
	return UI_DefaultMenuKey (&s_options_interface_menu, key);
}

void Menu_Options_Interface_f (void)
{
	Options_Interface_MenuInit ();
	UI_PushMenu (Menu_Options_Interface_Draw, Menu_Options_Interface_Key);
}
