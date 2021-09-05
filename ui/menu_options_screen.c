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

// menu_options_screen.c -- the screen options menu

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

/*
=======================================================================

SCREEN MENU

=======================================================================
*/

static menuframework_s	s_options_screen_menu;
static menuseparator_s	s_options_screen_header;
static menulist_s		s_options_screen_crosshair_box;
static menuslider_s		s_options_screen_crosshairscale_slider;
static menuslider_s		s_options_screen_crosshairalpha_slider;
static menuslider_s		s_options_screen_crosshairpulse_slider;
static menuslider_s		s_options_screen_hudscale_slider;
static menuslider_s		s_options_screen_hudalpha_slider;
static menulist_s		s_options_screen_hudsqueezedigits_box;
static menulist_s		s_options_screen_fps_box;
static menuaction_s		s_options_screen_defaults_action;
static menuaction_s		s_options_screen_back_action;

//=======================================================================

static void CrosshairFunc (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_options_screen_crosshair_box, "crosshair");
}

// Psychospaz's changeable size crosshair
static void CrosshairSizeFunc (void *unused)
{
	UI_MenuSlider_SaveValue (&s_options_screen_crosshairscale_slider, "crosshair_scale");
}

static void CrosshairAlphaFunc (void *unused)
{
	UI_MenuSlider_SaveValue (&s_options_screen_crosshairalpha_slider, "crosshair_alpha");
}

static void CrosshairPulseFunc (void *unused)
{
	UI_MenuSlider_SaveValue (&s_options_screen_crosshairpulse_slider, "crosshair_pulse");
}

// hud scaling option
static void HudScaleFunc (void *unused)
{
	UI_MenuSlider_SaveValue (&s_options_screen_hudscale_slider, "scr_hudsize");
}

// hud trans option
static void HudAlphaFunc (void *unused)
{
	UI_MenuSlider_SaveValue (&s_options_screen_hudalpha_slider, "scr_hudalpha");
}

// hud squeeze digits option
static void HudSqueezeDigitsFunc (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_options_screen_hudsqueezedigits_box, "scr_hudsqueezedigits");
}

// FPS counter option
static void FPSFunc (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_options_screen_fps_box, "cl_drawfps");
}

//=======================================================================

static void M_ScreenSetMenuItemValues (void)
{
	UI_MenuSpinControl_SetValue (&s_options_screen_crosshair_box, "crosshair", 0, 100, true);
	UI_MenuSlider_SetValue (&s_options_screen_crosshairscale_slider, "crosshair_scale", 0.25f, 5.0f, true);
	UI_MenuSlider_SetValue (&s_options_screen_crosshairalpha_slider, "crosshair_alpha", 0.05f, 1.0f, true);
	UI_MenuSlider_SetValue (&s_options_screen_crosshairpulse_slider, "crosshair_pulse", 0.0f, 0.5f, true);
	UI_MenuSlider_SetValue (&s_options_screen_hudscale_slider, "scr_hudsize", 0, 8, true);
	UI_MenuSlider_SetValue (&s_options_screen_hudalpha_slider, "scr_hudalpha", 0.0f, 1.0f, true);
	UI_MenuSpinControl_SetValue (&s_options_screen_hudsqueezedigits_box, "scr_hudsqueezedigits", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_options_screen_fps_box, "cl_drawfps", 0, 1, true);
}

static void M_ScreenResetDefaultsFunc (void *unused)
{
	Cvar_SetToDefault ("crosshair");
	Cvar_SetToDefault ("crosshair_scale");
	Cvar_SetToDefault ("crosshair_alpha");
	Cvar_SetToDefault ("crosshair_pulse");
	Cvar_SetToDefault ("scr_hudsize");
	Cvar_SetToDefault ("scr_hudalpha");
	Cvar_SetToDefault ("scr_hudsqueezedigits");
	Cvar_SetToDefault ("cl_drawfps");

	M_ScreenSetMenuItemValues ();
}

//=======================================================================

void Menu_Options_Screen_Init (void)
{
	static const char *yesno_names[] =
	{
		"no",
		"yes",
		0
	};

	int y = 3*MENU_LINE_SIZE;

	s_options_screen_menu.x = SCREEN_WIDTH*0.5;
	s_options_screen_menu.y = SCREEN_HEIGHT*0.5 - 58;
	s_options_screen_menu.nitems = 0;

	s_options_screen_header.generic.type		= MTYPE_SEPARATOR;
	s_options_screen_header.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_options_screen_header.generic.name		= "Screen";
	s_options_screen_header.generic.x			= MENU_HEADER_FONT_SIZE/2 * (int)strlen(s_options_screen_header.generic.name);
	s_options_screen_header.generic.y			= -2*MENU_LINE_SIZE;	// 0

	s_options_screen_crosshair_box.generic.type				= MTYPE_SPINCONTROL;
	s_options_screen_crosshair_box.generic.textSize			= MENU_FONT_SIZE;
	s_options_screen_crosshair_box.generic.x				= 0;
	s_options_screen_crosshair_box.generic.y				= y;
	s_options_screen_crosshair_box.generic.name				= "crosshair";
	s_options_screen_crosshair_box.generic.callback			= CrosshairFunc;
	s_options_screen_crosshair_box.itemNames				= ui_crosshair_names;
	s_options_screen_crosshair_box.itemValues				= ui_crosshair_values;
	s_options_screen_crosshair_box.generic.statusbar		= "changes crosshair";

	// Psychospaz's changeable size crosshair
	s_options_screen_crosshairscale_slider.generic.type			= MTYPE_SLIDER;
	s_options_screen_crosshairscale_slider.generic.textSize		= MENU_FONT_SIZE;
	s_options_screen_crosshairscale_slider.generic.x			= 0;
	s_options_screen_crosshairscale_slider.generic.y			= y += 5*MENU_LINE_SIZE;
	s_options_screen_crosshairscale_slider.generic.name			= "crosshair scale";
	s_options_screen_crosshairscale_slider.generic.callback		= CrosshairSizeFunc;
	s_options_screen_crosshairscale_slider.maxPos				= 19;
	s_options_screen_crosshairscale_slider.baseValue			= 0.25f;
	s_options_screen_crosshairscale_slider.increment			= 0.25f;
	s_options_screen_crosshairscale_slider.displayAsPercent		= false;
	s_options_screen_crosshairscale_slider.generic.statusbar	= "changes size of crosshair";

	s_options_screen_crosshairalpha_slider.generic.type			= MTYPE_SLIDER;
	s_options_screen_crosshairalpha_slider.generic.textSize		= MENU_FONT_SIZE;
	s_options_screen_crosshairalpha_slider.generic.x			= 0;
	s_options_screen_crosshairalpha_slider.generic.y			= y += MENU_LINE_SIZE;
	s_options_screen_crosshairalpha_slider.generic.name			= "crosshair alpha";
	s_options_screen_crosshairalpha_slider.generic.callback		= CrosshairAlphaFunc;
	s_options_screen_crosshairalpha_slider.maxPos				= 19;
	s_options_screen_crosshairalpha_slider.baseValue			= 0.05f;
	s_options_screen_crosshairalpha_slider.increment			= 0.05f;
	s_options_screen_crosshairalpha_slider.displayAsPercent		= true;
	s_options_screen_crosshairalpha_slider.generic.statusbar	= "changes opacity of crosshair";

	s_options_screen_crosshairpulse_slider.generic.type			= MTYPE_SLIDER;
	s_options_screen_crosshairpulse_slider.generic.textSize		= MENU_FONT_SIZE;
	s_options_screen_crosshairpulse_slider.generic.x			= 0;
	s_options_screen_crosshairpulse_slider.generic.y			= y += MENU_LINE_SIZE;
	s_options_screen_crosshairpulse_slider.generic.name			= "crosshair pulse";
	s_options_screen_crosshairpulse_slider.generic.callback		= CrosshairPulseFunc;
	s_options_screen_crosshairpulse_slider.maxPos				= 10;
	s_options_screen_crosshairpulse_slider.baseValue			= 0.0f;
	s_options_screen_crosshairpulse_slider.increment			= 0.05f;
	s_options_screen_crosshairpulse_slider.displayAsPercent		= true;
	s_options_screen_crosshairpulse_slider.generic.statusbar	= "changes pulse amplitude of crosshair";

	// hud scaling option
	s_options_screen_hudscale_slider.generic.type			= MTYPE_SLIDER;
	s_options_screen_hudscale_slider.generic.textSize		= MENU_FONT_SIZE;
	s_options_screen_hudscale_slider.generic.x				= 0;
	s_options_screen_hudscale_slider.generic.y				= y += 2*MENU_LINE_SIZE;
	s_options_screen_hudscale_slider.generic.name			= "status bar scale";
	s_options_screen_hudscale_slider.generic.callback		= HudScaleFunc;
	s_options_screen_hudscale_slider.maxPos					= 8;
	s_options_screen_hudscale_slider.baseValue				= 0.0f;
	s_options_screen_hudscale_slider.increment				= 1.0f;
	s_options_screen_hudscale_slider.displayAsPercent		= false;
	s_options_screen_hudscale_slider.generic.statusbar		= "changes size of HUD elements";

	// hud trans option
	s_options_screen_hudalpha_slider.generic.type			= MTYPE_SLIDER;
	s_options_screen_hudalpha_slider.generic.textSize		= MENU_FONT_SIZE;
	s_options_screen_hudalpha_slider.generic.x				= 0;
	s_options_screen_hudalpha_slider.generic.y				= y += MENU_LINE_SIZE;
	s_options_screen_hudalpha_slider.generic.name			= "status bar transparency";
	s_options_screen_hudalpha_slider.generic.callback		= HudAlphaFunc;
	s_options_screen_hudalpha_slider.maxPos					= 20;
	s_options_screen_hudalpha_slider.baseValue				= 0.0f;
	s_options_screen_hudalpha_slider.increment				= 0.05f;
	s_options_screen_hudalpha_slider.displayAsPercent		= true;
	s_options_screen_hudalpha_slider.generic.statusbar		= "changes opacity of HUD elements";

	// hud squeeze digits option
	s_options_screen_hudsqueezedigits_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_screen_hudsqueezedigits_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_screen_hudsqueezedigits_box.generic.x			= 0;
	s_options_screen_hudsqueezedigits_box.generic.y			= y += MENU_LINE_SIZE;
	s_options_screen_hudsqueezedigits_box.generic.name		= "status bar digit squeezing";
	s_options_screen_hudsqueezedigits_box.generic.callback	= HudSqueezeDigitsFunc;
	s_options_screen_hudsqueezedigits_box.itemNames			= yesno_names;
	s_options_screen_hudsqueezedigits_box.generic.statusbar	= "enables showing of longer numbers on HUD";

	s_options_screen_fps_box.generic.type				= MTYPE_SPINCONTROL;
	s_options_screen_fps_box.generic.textSize			= MENU_FONT_SIZE;
	s_options_screen_fps_box.generic.x					= 0;
	s_options_screen_fps_box.generic.y					= y += 2*MENU_LINE_SIZE;
	s_options_screen_fps_box.generic.name				= "FPS counter";
	s_options_screen_fps_box.generic.callback			= FPSFunc;
	s_options_screen_fps_box.itemNames					= yesno_names;
	s_options_screen_fps_box.generic.statusbar			= "enables FPS counter";

	s_options_screen_defaults_action.generic.type		= MTYPE_ACTION;
	s_options_screen_defaults_action.generic.textSize	= MENU_FONT_SIZE;
	s_options_screen_defaults_action.generic.x			= MENU_FONT_SIZE;
	s_options_screen_defaults_action.generic.y			= y += 2*MENU_LINE_SIZE;
	s_options_screen_defaults_action.generic.name		= "Reset to Defaults";
	s_options_screen_defaults_action.generic.callback	= M_ScreenResetDefaultsFunc;
	s_options_screen_defaults_action.generic.statusbar	= "resets all screen settings to internal defaults";

	s_options_screen_back_action.generic.type			= MTYPE_ACTION;
	s_options_screen_back_action.generic.textSize		= MENU_FONT_SIZE;
	s_options_screen_back_action.generic.x				= MENU_FONT_SIZE;
	s_options_screen_back_action.generic.y				= y += 2*MENU_LINE_SIZE;
	s_options_screen_back_action.generic.name			= "Back to Options";
	s_options_screen_back_action.generic.callback		= UI_BackMenu;

	UI_AddMenuItem (&s_options_screen_menu, (void *) &s_options_screen_header);
	UI_AddMenuItem (&s_options_screen_menu, (void *) &s_options_screen_crosshair_box);
	UI_AddMenuItem (&s_options_screen_menu, (void *) &s_options_screen_crosshairscale_slider);
	UI_AddMenuItem (&s_options_screen_menu, (void *) &s_options_screen_crosshairalpha_slider);
	UI_AddMenuItem (&s_options_screen_menu, (void *) &s_options_screen_crosshairpulse_slider);
	UI_AddMenuItem (&s_options_screen_menu, (void *) &s_options_screen_hudscale_slider);
	UI_AddMenuItem (&s_options_screen_menu, (void *) &s_options_screen_hudalpha_slider);
	UI_AddMenuItem (&s_options_screen_menu, (void *) &s_options_screen_hudsqueezedigits_box);
	UI_AddMenuItem (&s_options_screen_menu, (void *) &s_options_screen_fps_box);
	UI_AddMenuItem (&s_options_screen_menu, (void *) &s_options_screen_defaults_action);
	UI_AddMenuItem (&s_options_screen_menu, (void *) &s_options_screen_back_action);

	M_ScreenSetMenuItemValues ();
}

void Menu_Options_Screen_Crosshair_MouseClick (void)
{
	char *sound = NULL;
	buttonmenuobject_t crosshairbutton;
	int button_x, button_y;
	int button_size;

	button_size = 36;

	button_x = SCREEN_WIDTH*0.5 - 14;
	button_y = s_options_screen_menu.y + 42;

	UI_AddButton (&crosshairbutton, 0, button_x, button_y, button_size, button_size);

	if ( (ui_mousecursor.x >= crosshairbutton.min[0]) && (ui_mousecursor.x <= crosshairbutton.max[0]) &&
		(ui_mousecursor.y >= crosshairbutton.min[1]) && (ui_mousecursor.y <= crosshairbutton.max[1]) )
	{
		if (!ui_mousecursor.buttonused[MOUSEBUTTON1] && (ui_mousecursor.buttonclicks[MOUSEBUTTON1] == 1) )
		{
			s_options_screen_crosshair_box.curValue++;
			if (s_options_screen_crosshair_box.curValue > ui_numcrosshairs-1)
				s_options_screen_crosshair_box.curValue = 0; // wrap around
			CrosshairFunc (NULL);

			ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
			ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 0;
			sound = ui_menu_move_sound;
			if ( sound )
				S_StartLocalSound( sound );
		}
		if (!ui_mousecursor.buttonused[MOUSEBUTTON2] && (ui_mousecursor.buttonclicks[MOUSEBUTTON2] == 1) )
		{
			s_options_screen_crosshair_box.curValue--;
			if (s_options_screen_crosshair_box.curValue < 0)
				s_options_screen_crosshair_box.curValue = ui_numcrosshairs-1; // wrap around
			CrosshairFunc (NULL);

			ui_mousecursor.buttonused[MOUSEBUTTON2] = true;
			ui_mousecursor.buttonclicks[MOUSEBUTTON2] = 0;
			sound = ui_menu_move_sound;
			if ( sound )
				S_StartLocalSound( sound );
		}
	}
}

void Menu_Options_Screen_DrawCrosshair (void)
{
	UI_DrawFill (SCREEN_WIDTH*0.5 - 18, s_options_screen_menu.y + 42,
					36, 36, ALIGN_CENTER, false, 60,60,60,255);
	UI_DrawFill (SCREEN_WIDTH*0.5 - 17, s_options_screen_menu.y + 43,
					34, 34, ALIGN_CENTER,  false, 0,0,0,255);

	UI_DrawPic (SCREEN_WIDTH*0.5-16, s_options_screen_menu.y + 44,
					32, 32, ALIGN_CENTER, false, ui_crosshair_display_names[s_options_screen_crosshair_box.curValue], 1.0);
}

void Menu_Options_Screen_Draw (void)
{
	UI_DrawBanner ("m_banner_options");

	UI_AdjustMenuCursor (&s_options_screen_menu, 1);
	UI_DrawMenu (&s_options_screen_menu);
	Menu_Options_Screen_DrawCrosshair ();
}

const char *Menu_Options_Screen_Key (int key)
{
	return UI_DefaultMenuKey (&s_options_screen_menu, key);
}

void Menu_Options_Screen_f (void)
{
	Menu_Options_Screen_Init ();
	UI_PushMenu (Menu_Options_Screen_Draw, Menu_Options_Screen_Key);
}
