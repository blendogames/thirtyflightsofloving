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

// menu_options_controls.c -- the controls options menu

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

/*
=======================================================================

CONTROLS MENU

=======================================================================
*/

static menuframework_s	s_options_controls_menu;
static menuseparator_s	s_options_controls_header;
static menuslider_s		s_options_controls_sensitivity_slider;
static menulist_s		s_options_controls_alwaysrun_box;
static menulist_s		s_options_controls_thirdperson_box;
static menuslider_s		s_options_controls_thirdperson_distance_slider;
static menuslider_s		s_options_controls_thirdperson_offset_slider;
static menuslider_s		s_options_controls_thirdperson_angle_slider;
static menulist_s		s_options_controls_invertmouse_box;
static menulist_s		s_options_controls_autosensitivity_box;
static menulist_s		s_options_controls_lookspring_box;
static menulist_s		s_options_controls_lookstrafe_box;
static menulist_s		s_options_controls_freelook_box;
static menulist_s		s_options_controls_joystick_box;
static menuaction_s		s_options_controls_customize_keys_action;
static menuaction_s		s_options_controls_defaults_action;
static menuaction_s		s_options_controls_back_action;


static void MouseSpeedFunc (void *unused)
{
	Cvar_SetValue( "sensitivity", UI_MenuSlider_GetValue(&s_options_controls_sensitivity_slider) );
}

static void AlwaysRunFunc (void *unused)
{
	Cvar_SetValue( "cl_run", s_options_controls_alwaysrun_box.curvalue );
}

// Psychospaz's chaseam
static void ThirdPersonFunc (void *unused)
{
	Cvar_SetValue( "cg_thirdperson", s_options_controls_thirdperson_box.curvalue );
}

static void ThirdPersonDistFunc (void *unused)
{
	Cvar_SetValue( "cg_thirdperson_dist", UI_MenuSlider_GetValue(&s_options_controls_thirdperson_distance_slider) );
}

static void ThirdPersonOffsetFunc (void *unused)
{
	Cvar_SetValue( "cg_thirdperson_offset", UI_MenuSlider_GetValue(&s_options_controls_thirdperson_offset_slider) );
}

static void ThirdPersonAngleFunc (void *unused)
{
	Cvar_SetValue( "cg_thirdperson_angle", UI_MenuSlider_GetValue(&s_options_controls_thirdperson_angle_slider) );
}

static void FreeLookFunc (void *unused)
{
	Cvar_SetValue( "freelook", s_options_controls_freelook_box.curvalue );
}

static void InvertMouseFunc (void *unused)
{
	Cvar_SetValue( "m_pitch", -m_pitch->value );
}

static void AutosensitivityFunc (void *unused)
{
	Cvar_SetValue( "in_autosensitivity", s_options_controls_autosensitivity_box.curvalue );
}

static void LookspringFunc (void *unused)
{
	Cvar_SetValue( "lookspring", !lookspring->value );
}

static void LookstrafeFunc (void *unused)
{
	Cvar_SetValue( "lookstrafe", !lookstrafe->value );
}

static void JoystickFunc (void *unused)
{
	Cvar_SetValue( "in_joystick", s_options_controls_joystick_box.curvalue );
}

static void CustomizeControlsFunc(void *unused)
{
	Menu_Keys_f ();
}

static void ControlsSetMenuItemValues (void)
{
	UI_MenuSlider_SetValue (&s_options_controls_sensitivity_slider, Cvar_VariableValue("sensitivity"));

	s_options_controls_invertmouse_box.curvalue		= Cvar_VariableValue("m_pitch") < 0;

	Cvar_SetValue( "in_autosensitivity", ClampCvar( 0, 1, Cvar_VariableValue("in_autosensitivity") ) );
	s_options_controls_autosensitivity_box.curvalue = Cvar_VariableValue("in_autosensitivity");

	Cvar_SetValue( "cg_thirdperson", ClampCvar( 0, 1, Cvar_VariableValue("cg_thirdperson") ) );
	s_options_controls_thirdperson_box.curvalue		= Cvar_VariableValue("cg_thirdperson");
	UI_MenuSlider_SetValue (&s_options_controls_thirdperson_distance_slider, Cvar_VariableValue("cg_thirdperson_dist"));
	UI_MenuSlider_SetValue (&s_options_controls_thirdperson_offset_slider, Cvar_VariableValue("cg_thirdperson_offset"));
	UI_MenuSlider_SetValue (&s_options_controls_thirdperson_angle_slider, Cvar_VariableValue("cg_thirdperson_angle"));

	Cvar_SetValue( "cl_run", ClampCvar( 0, 1, Cvar_VariableValue("cl_run") ) );
	s_options_controls_alwaysrun_box.curvalue		= Cvar_VariableValue("cl_run");

	Cvar_SetValue( "lookspring", ClampCvar( 0, 1, Cvar_VariableValue("lookspring") ) );
	s_options_controls_lookspring_box.curvalue		= Cvar_VariableValue("lookspring");

	Cvar_SetValue( "lookstrafe", ClampCvar( 0, 1, Cvar_VariableValue("lookstrafe") ) );
	s_options_controls_lookstrafe_box.curvalue		= Cvar_VariableValue("lookstrafe");

	Cvar_SetValue( "freelook", ClampCvar( 0, 1, Cvar_VariableValue("freelook") ) );
	s_options_controls_freelook_box.curvalue			= Cvar_VariableValue("freelook");

	Cvar_SetValue( "in_joystick", ClampCvar( 0, 1, Cvar_VariableValue("in_joystick") ) );
	s_options_controls_joystick_box.curvalue		= Cvar_VariableValue("in_joystick");
}

static void ControlsResetDefaultsFunc (void *unused)
{
//	Cvar_SetToDefault ("sensitivity");
//	Cvar_SetToDefault ("m_pitch");
	Cvar_SetToDefault ("in_autosensitivity");

	Cvar_SetToDefault ("cg_thirdperson");
	Cvar_SetToDefault ("cg_thirdperson_dist");
	Cvar_SetToDefault ("cg_thirdperson_offset");
	Cvar_SetToDefault ("cg_thirdperson_angle");
//	Cvar_SetToDefault ("cl_run");
//	Cvar_SetToDefault ("lookspring");
//	Cvar_SetToDefault ("lookstrafe");
//	Cvar_SetToDefault ("freelook");
	Cvar_SetToDefault ("in_joystick");

	Cbuf_AddText ("exec defaultbinds.cfg\n"); // reset default binds
	Cbuf_Execute();

	ControlsSetMenuItemValues ();
}

void Menu_Options_Controls_Init (void)
{
	static const char *yesno_names[] =
	{
		"no",
		"yes",
		0
	};

	int y = 3*MENU_LINE_SIZE;

	s_options_controls_menu.x = SCREEN_WIDTH*0.5;
	s_options_controls_menu.y = SCREEN_HEIGHT*0.5 - 58;
	s_options_controls_menu.nitems = 0;

	s_options_controls_header.generic.type		= MTYPE_SEPARATOR;
	s_options_controls_header.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_options_controls_header.generic.name		= "Controls";
	s_options_controls_header.generic.x			= MENU_HEADER_FONT_SIZE/2 * (int)strlen(s_options_controls_header.generic.name);
	s_options_controls_header.generic.y			= -2*MENU_LINE_SIZE;	// 0

	s_options_controls_sensitivity_slider.generic.type		= MTYPE_SLIDER;
	s_options_controls_sensitivity_slider.generic.textSize	= MENU_FONT_SIZE;
	s_options_controls_sensitivity_slider.generic.x			= 0;
	s_options_controls_sensitivity_slider.generic.y			= y;
	s_options_controls_sensitivity_slider.generic.name		= "mouse speed";
	s_options_controls_sensitivity_slider.generic.callback	= MouseSpeedFunc;
	s_options_controls_sensitivity_slider.maxPos			= 20;
	s_options_controls_sensitivity_slider.baseValue			= 1.0f;
	s_options_controls_sensitivity_slider.increment			= 0.5f;
	s_options_controls_sensitivity_slider.displayAsPercent	= false;
	s_options_controls_sensitivity_slider.generic.statusbar	= "changes sensitivity of mouse for head movement";

	s_options_controls_invertmouse_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_controls_invertmouse_box.generic.textSize		= MENU_FONT_SIZE;
	s_options_controls_invertmouse_box.generic.x			= 0;
	s_options_controls_invertmouse_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_controls_invertmouse_box.generic.name			= "invert mouse";
	s_options_controls_invertmouse_box.generic.callback		= InvertMouseFunc;
	s_options_controls_invertmouse_box.itemnames			= yesno_names;
	s_options_controls_invertmouse_box.generic.statusbar	= "inverts mouse y-axis movement";

	s_options_controls_autosensitivity_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_controls_autosensitivity_box.generic.textSize		= MENU_FONT_SIZE;
	s_options_controls_autosensitivity_box.generic.x			= 0;
	s_options_controls_autosensitivity_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_controls_autosensitivity_box.generic.name			= "scale mouse to FOV";
	s_options_controls_autosensitivity_box.generic.callback		= AutosensitivityFunc;
	s_options_controls_autosensitivity_box.itemnames			= yesno_names;
	s_options_controls_autosensitivity_box.generic.statusbar	= "adjusts mouse sensitivity to zoomed FOV";

	s_options_controls_thirdperson_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_controls_thirdperson_box.generic.textSize		= MENU_FONT_SIZE;
	s_options_controls_thirdperson_box.generic.x			= 0;
	s_options_controls_thirdperson_box.generic.y			= y+=2*MENU_LINE_SIZE;
	s_options_controls_thirdperson_box.generic.name			= "third person";
	s_options_controls_thirdperson_box.generic.callback		= ThirdPersonFunc;
	s_options_controls_thirdperson_box.itemnames			= yesno_names;
	s_options_controls_thirdperson_box.generic.statusbar	= "enables third-person mode";

	s_options_controls_thirdperson_distance_slider.generic.type			= MTYPE_SLIDER;
	s_options_controls_thirdperson_distance_slider.generic.textSize		= MENU_FONT_SIZE;
	s_options_controls_thirdperson_distance_slider.generic.x			= 0;
	s_options_controls_thirdperson_distance_slider.generic.y			= y+=MENU_LINE_SIZE;
	s_options_controls_thirdperson_distance_slider.generic.name			= "camera distance";
	s_options_controls_thirdperson_distance_slider.generic.callback		= ThirdPersonDistFunc;
	s_options_controls_thirdperson_distance_slider.maxPos				= 10;
	s_options_controls_thirdperson_distance_slider.baseValue			= 25.0f;
	s_options_controls_thirdperson_distance_slider.increment			= 5.0f;
	s_options_controls_thirdperson_distance_slider.displayAsPercent		= false;
	s_options_controls_thirdperson_distance_slider.generic.statusbar	= "changes camera distance in third-person mode";

	s_options_controls_thirdperson_offset_slider.generic.type		= MTYPE_SLIDER;
	s_options_controls_thirdperson_offset_slider.generic.textSize	= MENU_FONT_SIZE;
	s_options_controls_thirdperson_offset_slider.generic.x			= 0;
	s_options_controls_thirdperson_offset_slider.generic.y			= y+=MENU_LINE_SIZE;
	s_options_controls_thirdperson_offset_slider.generic.name		= "camera offset";
	s_options_controls_thirdperson_offset_slider.generic.callback	= ThirdPersonOffsetFunc;
	s_options_controls_thirdperson_offset_slider.maxPos				= 8;
	s_options_controls_thirdperson_offset_slider.baseValue			= 0.0f;
	s_options_controls_thirdperson_offset_slider.increment			= 4.0f;
	s_options_controls_thirdperson_offset_slider.displayAsPercent	= false;
	s_options_controls_thirdperson_offset_slider.generic.statusbar	= "changes horizontal camera offset in third-person mode";

	s_options_controls_thirdperson_angle_slider.generic.type		= MTYPE_SLIDER;
	s_options_controls_thirdperson_angle_slider.generic.textSize	= MENU_FONT_SIZE;
	s_options_controls_thirdperson_angle_slider.generic.x			= 0;
	s_options_controls_thirdperson_angle_slider.generic.y			= y+=MENU_LINE_SIZE;
	s_options_controls_thirdperson_angle_slider.generic.name		= "camera angle";
	s_options_controls_thirdperson_angle_slider.generic.callback	= ThirdPersonAngleFunc;
	s_options_controls_thirdperson_angle_slider.maxPos				= 6;
	s_options_controls_thirdperson_angle_slider.baseValue			= 0.0f;
	s_options_controls_thirdperson_angle_slider.increment			= 5.0f;
	s_options_controls_thirdperson_angle_slider.displayAsPercent	= false;
	s_options_controls_thirdperson_angle_slider.generic.statusbar	= "changes camera angle in third-person mode";

	s_options_controls_alwaysrun_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_controls_alwaysrun_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_controls_alwaysrun_box.generic.x			= 0;
	s_options_controls_alwaysrun_box.generic.y			= y+=2*MENU_LINE_SIZE;
	s_options_controls_alwaysrun_box.generic.name		= "always run";
	s_options_controls_alwaysrun_box.generic.callback	= AlwaysRunFunc;
	s_options_controls_alwaysrun_box.itemnames			= yesno_names;
	s_options_controls_alwaysrun_box.generic.statusbar	= "enables running as default movement";

	s_options_controls_lookspring_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_controls_lookspring_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_controls_lookspring_box.generic.x			= 0;
	s_options_controls_lookspring_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_controls_lookspring_box.generic.name		= "lookspring";
	s_options_controls_lookspring_box.generic.callback	= LookspringFunc;
	s_options_controls_lookspring_box.itemnames			= yesno_names;

	s_options_controls_lookstrafe_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_controls_lookstrafe_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_controls_lookstrafe_box.generic.x			= 0;
	s_options_controls_lookstrafe_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_controls_lookstrafe_box.generic.name		= "lookstrafe";
	s_options_controls_lookstrafe_box.generic.callback	= LookstrafeFunc;
	s_options_controls_lookstrafe_box.itemnames			= yesno_names;

	s_options_controls_freelook_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_controls_freelook_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_controls_freelook_box.generic.x			= 0;
	s_options_controls_freelook_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_controls_freelook_box.generic.name		= "free look";
	s_options_controls_freelook_box.generic.callback	= FreeLookFunc;
	s_options_controls_freelook_box.itemnames			= yesno_names;
	s_options_controls_freelook_box.generic.statusbar	= "enables free head movement with mouse";

	s_options_controls_joystick_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_controls_joystick_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_controls_joystick_box.generic.x			= 0;
	s_options_controls_joystick_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_controls_joystick_box.generic.name		= "use joystick";
	s_options_controls_joystick_box.generic.callback	= JoystickFunc;
	s_options_controls_joystick_box.itemnames			= yesno_names;
	s_options_controls_joystick_box.generic.statusbar	= "enables use of joystick";

	s_options_controls_customize_keys_action.generic.type		= MTYPE_ACTION;
	s_options_controls_customize_keys_action.generic.textSize	= MENU_FONT_SIZE;
	s_options_controls_customize_keys_action.generic.x			= MENU_FONT_SIZE;
	s_options_controls_customize_keys_action.generic.y			= y+=2*MENU_LINE_SIZE;
	s_options_controls_customize_keys_action.generic.name		= "customize controls";
	s_options_controls_customize_keys_action.generic.callback	= CustomizeControlsFunc;

	s_options_controls_defaults_action.generic.type			= MTYPE_ACTION;
	s_options_controls_defaults_action.generic.textSize		= MENU_FONT_SIZE;
	s_options_controls_defaults_action.generic.x			= MENU_FONT_SIZE;
	s_options_controls_defaults_action.generic.y			= 20*MENU_LINE_SIZE;
	s_options_controls_defaults_action.generic.name			= "reset defaults";
	s_options_controls_defaults_action.generic.callback		= ControlsResetDefaultsFunc;
	s_options_controls_defaults_action.generic.statusbar	= "resets all control settings to internal defaults";

	s_options_controls_back_action.generic.type			= MTYPE_ACTION;
	s_options_controls_back_action.generic.textSize		= MENU_FONT_SIZE;
	s_options_controls_back_action.generic.x			= MENU_FONT_SIZE;
	s_options_controls_back_action.generic.y			= 22*MENU_LINE_SIZE;
	s_options_controls_back_action.generic.name			= "back to options";
	s_options_controls_back_action.generic.callback		= UI_BackMenu;

	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_header);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_sensitivity_slider);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_invertmouse_box);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_autosensitivity_box);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_thirdperson_box);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_thirdperson_distance_slider);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_thirdperson_offset_slider);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_thirdperson_angle_slider);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_alwaysrun_box);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_lookspring_box);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_lookstrafe_box);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_freelook_box);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_joystick_box);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_customize_keys_action);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_defaults_action);
	UI_AddMenuItem (&s_options_controls_menu, (void *) &s_options_controls_back_action);

	ControlsSetMenuItemValues ();
}

void Menu_Options_Controls_Draw (void)
{
	UI_DrawBanner ("m_banner_options");

	UI_AdjustMenuCursor (&s_options_controls_menu, 1);
	UI_DrawMenu (&s_options_controls_menu);
}

const char *Menu_Options_Controls_Key (int key)
{
	return UI_DefaultMenuKey (&s_options_controls_menu, key);
}

void Menu_Options_Controls_f (void)
{
	Menu_Options_Controls_Init ();
	UI_PushMenu (Menu_Options_Controls_Draw, Menu_Options_Controls_Key);
}
