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

//=======================================================================

static void MouseMenuFunc (void *unused)
{
	UI_MenuSlider_SaveValue (&s_options_interface_menumouse_slider, "ui_sensitivity");
}

// menu alpha option
static void MenuAlphaFunc (void *unused)
{
	UI_MenuSlider_SaveValue (&s_options_interface_menualpha_slider, "ui_background_alpha");
}

static void ConFontFunc (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_options_interface_confont_box, "con_font");
}

static void FontSizeFunc (void *unused)
{
	UI_MenuSlider_SaveValue (&s_options_interface_fontsize_slider, "con_font_size");
}

static void UIFontFunc (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_options_interface_uifont_box, "ui_font");
}

static void ScrFontFunc (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_options_interface_scrfont_box, "scr_font");
}

static void AltTextColorFunc (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_options_interface_alt_text_color_box, "alt_text_color");
}

// Psychospaz's transparent console
static void ConAlphaFunc (void *unused)
{
	UI_MenuSlider_SaveValue (&s_options_interface_conalpha_slider, "scr_conalpha");
}

#if 0
// variable console height
static void ConHeightFunc (void *unused)
{
	UI_MenuSlider_SaveValue (&s_options_interface_conheight_slider, "scr_conheight");
}
#endif

static void SimpleLoadscreenFunc (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_options_interface_simple_loadscreen_box, "scr_simple_loadscreen");
}

static void NewConbackFunc (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_options_interface_newconback_box, "scr_newconback");
}

static void NoAltTabFunc (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_options_interface_noalttab_box, "win_noalttab");
}

//=======================================================================

static void M_InterfaceSetMenuItemValues (void)
{
	UI_MenuSlider_SetValue (&s_options_interface_menumouse_slider, "ui_sensitivity", 0.25f, 2.0f, true);
	UI_MenuSlider_SetValue (&s_options_interface_menualpha_slider, "ui_background_alpha", 0.0f, 1.0f, true );

	UI_MenuSpinControl_SetValue (&s_options_interface_confont_box, "con_font", 0, 0, false);
	UI_MenuSlider_SetValue (&s_options_interface_fontsize_slider, "con_font_size", 6.0f, 16.0f, true);
	UI_MenuSpinControl_SetValue (&s_options_interface_uifont_box, "ui_font", 0, 0, false);
	UI_MenuSpinControl_SetValue (&s_options_interface_scrfont_box, "scr_font", 0, 0, false);
	UI_MenuSpinControl_SetValue (&s_options_interface_alt_text_color_box, "alt_text_color", 0, 9, true);

	UI_MenuSlider_SetValue (&s_options_interface_conalpha_slider, "scr_conalpha", 0.0f, 1.0f, true);
//	UI_MenuSlider_SetValue (&s_options_interface_conheight_slider, "scr_conheight", 0.10f, 1.0f, true);
	UI_MenuSpinControl_SetValue (&s_options_interface_simple_loadscreen_box, "scr_simple_loadscreen", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_options_interface_newconback_box, "scr_newconback", 0, 1, true);

	UI_MenuSpinControl_SetValue (&s_options_interface_noalttab_box, "win_noalttab", 0, 1, true);
}

static void M_InterfaceResetDefaultsFunc (void *unused)
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

	M_InterfaceSetMenuItemValues ();
}

//=======================================================================

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
	s_options_interface_menualpha_slider.maxPos				= 20;
	s_options_interface_menualpha_slider.baseValue			= 0.0f;
	s_options_interface_menualpha_slider.increment			= 0.05f;
	s_options_interface_menualpha_slider.displayAsPercent	= true;
	s_options_interface_menualpha_slider.generic.statusbar	= "changes opacity of menu background";

	s_options_interface_confont_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_interface_confont_box.generic.textSize		= MENU_FONT_SIZE;
	s_options_interface_confont_box.generic.x				= 0;
	s_options_interface_confont_box.generic.y				= y+=2*MENU_LINE_SIZE;
	s_options_interface_confont_box.generic.name			= "console font";
	s_options_interface_confont_box.generic.callback		= ConFontFunc;
	s_options_interface_confont_box.itemNames				= ui_font_names;
	s_options_interface_confont_box.itemValues				= ui_font_names;
	s_options_interface_confont_box.generic.statusbar		= "changes font of console text";

	s_options_interface_fontsize_slider.generic.type		= MTYPE_SLIDER;
	s_options_interface_fontsize_slider.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_fontsize_slider.generic.x			= 0;
	s_options_interface_fontsize_slider.generic.y			= y+=MENU_LINE_SIZE;
	s_options_interface_fontsize_slider.generic.name		= "console font size";
	s_options_interface_fontsize_slider.generic.callback	= FontSizeFunc;
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
	s_options_interface_uifont_box.itemNames				= ui_font_names;
	s_options_interface_uifont_box.itemValues				= ui_font_names;
	s_options_interface_uifont_box.generic.statusbar		= "changes font of menu text";

	s_options_interface_scrfont_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_interface_scrfont_box.generic.textSize		= MENU_FONT_SIZE;
	s_options_interface_scrfont_box.generic.x				= 0;
	s_options_interface_scrfont_box.generic.y				= y+=MENU_LINE_SIZE;
	s_options_interface_scrfont_box.generic.name			= "HUD font";
	s_options_interface_scrfont_box.generic.callback		= ScrFontFunc;
	s_options_interface_scrfont_box.itemNames				= ui_font_names;
	s_options_interface_scrfont_box.itemValues				= ui_font_names;
	s_options_interface_scrfont_box.generic.statusbar		= "changes font of HUD text";

	s_options_interface_alt_text_color_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_interface_alt_text_color_box.generic.textSize		= MENU_FONT_SIZE;
	s_options_interface_alt_text_color_box.generic.x			= 0;
	s_options_interface_alt_text_color_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_interface_alt_text_color_box.generic.name			= "alt text color";
	s_options_interface_alt_text_color_box.generic.callback		= AltTextColorFunc;
	s_options_interface_alt_text_color_box.itemNames			= textcolor_names;
	s_options_interface_alt_text_color_box.generic.statusbar	= "changes color of highlighted text";

	s_options_interface_conalpha_slider.generic.type		= MTYPE_SLIDER;
	s_options_interface_conalpha_slider.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_conalpha_slider.generic.x			= 0;
	s_options_interface_conalpha_slider.generic.y			= y+=2*MENU_LINE_SIZE;
	s_options_interface_conalpha_slider.generic.name		= "console transparency";
	s_options_interface_conalpha_slider.generic.callback	= ConAlphaFunc;
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
	s_options_interface_simple_loadscreen_box.itemNames			= yesno_names;
	s_options_interface_simple_loadscreen_box.generic.statusbar	= "toggles simple map load screen";

	s_options_interface_newconback_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_interface_newconback_box.generic.textSize		= MENU_FONT_SIZE;
	s_options_interface_newconback_box.generic.x			= 0;
	s_options_interface_newconback_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_interface_newconback_box.generic.name			= "new console background";
	s_options_interface_newconback_box.generic.callback		= NewConbackFunc;
	s_options_interface_newconback_box.itemNames			= yesno_names;
	s_options_interface_newconback_box.generic.statusbar	= "enables Q3-style console background";

	s_options_interface_noalttab_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_interface_noalttab_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_noalttab_box.generic.x			= 0;
	s_options_interface_noalttab_box.generic.y			= y+=2*MENU_LINE_SIZE;
	s_options_interface_noalttab_box.generic.name		= "disable alt-tab";
	s_options_interface_noalttab_box.generic.callback	= NoAltTabFunc;
	s_options_interface_noalttab_box.itemNames			= yesno_names;
	s_options_interface_noalttab_box.generic.statusbar	= "disables alt-tabbing to desktop";

	s_options_interface_defaults_action.generic.type		= MTYPE_ACTION;
	s_options_interface_defaults_action.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_defaults_action.generic.x			= MENU_FONT_SIZE;
	s_options_interface_defaults_action.generic.y			= y += 2*MENU_LINE_SIZE;
	s_options_interface_defaults_action.generic.name		= "Reset to Defaults";
	s_options_interface_defaults_action.generic.callback	= M_InterfaceResetDefaultsFunc;
	s_options_interface_defaults_action.generic.statusbar	= "resets all interface settings to internal defaults";

	s_options_interface_back_action.generic.type		= MTYPE_ACTION;
	s_options_interface_back_action.generic.textSize	= MENU_FONT_SIZE;
	s_options_interface_back_action.generic.x			= MENU_FONT_SIZE;
	s_options_interface_back_action.generic.y			= y += 2*MENU_LINE_SIZE;
	s_options_interface_back_action.generic.name		= "Back to Options";
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

	M_InterfaceSetMenuItemValues ();
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
