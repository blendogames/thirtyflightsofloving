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

// ui_options_controls.c -- the controls options menu

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

// controls
static menuframework_s	s_options_controls_menu;
static menuseparator_s	s_options_controls_header;
static menuslider_s		s_options_controls_sensitivity_slider;
static menulist_s		s_options_controls_alwaysrun_box;
static menulist_s		s_options_controls_thirdperson_box;
static menuslider_s		s_options_controls_thirdperson_distance_slider;
static menuslider_s		s_options_controls_thirdperson_angle_slider;
static menulist_s		s_options_controls_invertmouse_box;
static menulist_s		s_options_controls_lookspring_box;
static menulist_s		s_options_controls_lookstrafe_box;
static menulist_s		s_options_controls_freelook_box;
static menulist_s		s_options_controls_joystick_box;
static menulist_s		s_options_controls_console_box;
static menuaction_s		s_options_controls_customize_keys_action;
static menuaction_s		s_options_controls_defaults_action;
static menuaction_s		s_options_controls_back_action;


static void MouseSpeedFunc( void *unused )
{
	Cvar_SetValue( "sensitivity", s_options_controls_sensitivity_slider.curvalue / 2.0F );
}

static void AlwaysRunFunc( void *unused )
{
	Cvar_SetValue( "cl_run", s_options_controls_alwaysrun_box.curvalue );
}

// Psychospaz's chaseam
static void ThirdPersonFunc( void *unused )
{
	Cvar_SetValue( "cl_3dcam", s_options_controls_thirdperson_box.curvalue );
}

static void ThirdPersonDistFunc( void *unused )
{
	Cvar_SetValue( "cl_3dcam_dist", (int)(s_options_controls_thirdperson_distance_slider.curvalue * 25) );
}

static void ThirdPersonAngleFunc( void *unused )
{
	Cvar_SetValue( "cl_3dcam_angle", (int)(s_options_controls_thirdperson_angle_slider.curvalue * 10) );
}

static void FreeLookFunc( void *unused )
{
	Cvar_SetValue( "freelook", s_options_controls_freelook_box.curvalue );
}

static void InvertMouseFunc( void *unused )
{
	Cvar_SetValue( "m_pitch", -m_pitch->value );
}

static void LookspringFunc( void *unused )
{
	Cvar_SetValue( "lookspring", !lookspring->value );
}

static void LookstrafeFunc( void *unused )
{
	Cvar_SetValue( "lookstrafe", !lookstrafe->value );
}

static void JoystickFunc( void *unused )
{
	Cvar_SetValue( "in_joystick", s_options_controls_joystick_box.curvalue );
}

static void ConsoleFunc( void *unused )
{
	Cvar_SetValue( "cl_enableconsole", s_options_controls_console_box.curvalue );
}

static void CustomizeControlsFunc( void *unused )
{
	M_Menu_Keys_f();
}

static void ControlsSetMenuItemValues( void )
{
	s_options_controls_sensitivity_slider.curvalue	= ( Cvar_VariableValue("sensitivity") ) * 2;
	s_options_controls_invertmouse_box.curvalue		= Cvar_VariableValue("m_pitch") < 0;

	Cvar_SetValue( "cl_3dcam", ClampCvar( 0, 1, Cvar_VariableValue("cl_3dcam") ) );
	s_options_controls_thirdperson_box.curvalue		= Cvar_VariableValue("cl_3dcam");
	s_options_controls_thirdperson_distance_slider.curvalue	= Cvar_VariableValue("cl_3dcam_dist") / 25;
	s_options_controls_thirdperson_angle_slider.curvalue	= Cvar_VariableValue("cl_3dcam_angle") / 10;

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

static void ControlsResetDefaultsFunc ( void *unused )
{
	//Cvar_SetToDefault ("sensitivity");
	//Cvar_SetToDefault ("m_pitch");
	Cvar_SetToDefault ("cl_3dcam");
	Cvar_SetToDefault ("cl_3dcam_dist");
	Cvar_SetToDefault ("scl_3dcam_angle");
	//Cvar_SetToDefault ("cl_run");
	//Cvar_SetToDefault ("lookspring");
	//Cvar_SetToDefault ("lookstrafe");
	//Cvar_SetToDefault ("freelook");
	Cvar_SetToDefault ("in_joystick");

	Cbuf_AddText ("exec defaultbinds.cfg\n"); // reset default binds
	Cbuf_Execute();

	ControlsSetMenuItemValues();
}

void Options_Controls_MenuInit ( void )
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
//	s_options_controls_menu.x = viddef.width * 0.50;
//	s_options_controls_menu.y = viddef.height * 0.50 - SCR_ScaledVideo(7.25*MENU_FONT_SIZE); //58
	s_options_controls_menu.nitems = 0;

	s_options_controls_header.generic.type	= MTYPE_SEPARATOR;
	s_options_controls_header.generic.name	= "CONTROLS";
	s_options_controls_header.generic.x	= MENU_FONT_SIZE/2 * strlen(s_options_controls_header.generic.name);
	s_options_controls_header.generic.y	= 0;

	s_options_controls_sensitivity_slider.generic.type	= MTYPE_SLIDER;
	s_options_controls_sensitivity_slider.generic.x		= 0;
	s_options_controls_sensitivity_slider.generic.y		= y;
	s_options_controls_sensitivity_slider.generic.name	= "Mouse Sensitivity";
	s_options_controls_sensitivity_slider.generic.callback = MouseSpeedFunc;
	s_options_controls_sensitivity_slider.minvalue		= 2;
	s_options_controls_sensitivity_slider.maxvalue		= 22;

	s_options_controls_invertmouse_box.generic.type = MTYPE_CHECKBOX;
	s_options_controls_invertmouse_box.generic.x	= 0;
	s_options_controls_invertmouse_box.generic.y	= y+=MENU_LINE_SIZE;
	s_options_controls_invertmouse_box.generic.name	= "Invert Mouse";
	s_options_controls_invertmouse_box.generic.callback = InvertMouseFunc;
	s_options_controls_invertmouse_box.itemnames = yesno_names;

	/*

	s_options_controls_thirdperson_box.generic.type = MTYPE_SPINCONTROL;
	s_options_controls_thirdperson_box.generic.x	= 0;
	s_options_controls_thirdperson_box.generic.y	= y+=2*MENU_LINE_SIZE;
	s_options_controls_thirdperson_box.generic.name	= "third person";
	s_options_controls_thirdperson_box.generic.callback = ThirdPersonFunc;
	s_options_controls_thirdperson_box.itemnames = yesno_names;

	s_options_controls_thirdperson_distance_slider.generic.type	= MTYPE_SLIDER;
	s_options_controls_thirdperson_distance_slider.generic.x		= 0;
	s_options_controls_thirdperson_distance_slider.generic.y		= y+=MENU_LINE_SIZE;
	s_options_controls_thirdperson_distance_slider.generic.name	= "camera distance";
	s_options_controls_thirdperson_distance_slider.generic.callback = ThirdPersonDistFunc;
	s_options_controls_thirdperson_distance_slider.minvalue		= 1;
	s_options_controls_thirdperson_distance_slider.maxvalue		= 5;

	s_options_controls_thirdperson_angle_slider.generic.type	= MTYPE_SLIDER;
	s_options_controls_thirdperson_angle_slider.generic.x		= 0;
	s_options_controls_thirdperson_angle_slider.generic.y		= y+=MENU_LINE_SIZE;
	s_options_controls_thirdperson_angle_slider.generic.name	= "camera angle";
	s_options_controls_thirdperson_angle_slider.generic.callback = ThirdPersonAngleFunc;
	s_options_controls_thirdperson_angle_slider.minvalue		= 0;
	s_options_controls_thirdperson_angle_slider.maxvalue		= 4;

	s_options_controls_alwaysrun_box.generic.type = MTYPE_SPINCONTROL;
	s_options_controls_alwaysrun_box.generic.x	= 0;
	s_options_controls_alwaysrun_box.generic.y	= y+=2*MENU_LINE_SIZE;
	s_options_controls_alwaysrun_box.generic.name	= "always run";
	s_options_controls_alwaysrun_box.generic.callback = AlwaysRunFunc;
	s_options_controls_alwaysrun_box.itemnames = yesno_names;

	s_options_controls_lookspring_box.generic.type = MTYPE_SPINCONTROL;
	s_options_controls_lookspring_box.generic.x	= 0;
	s_options_controls_lookspring_box.generic.y	= y+=MENU_LINE_SIZE;
	s_options_controls_lookspring_box.generic.name	= "lookspring";
	s_options_controls_lookspring_box.generic.callback = LookspringFunc;
	s_options_controls_lookspring_box.itemnames = yesno_names;

	s_options_controls_lookstrafe_box.generic.type = MTYPE_SPINCONTROL;
	s_options_controls_lookstrafe_box.generic.x	= 0;
	s_options_controls_lookstrafe_box.generic.y	= y+=MENU_LINE_SIZE;
	s_options_controls_lookstrafe_box.generic.name	= "lookstrafe";
	s_options_controls_lookstrafe_box.generic.callback = LookstrafeFunc;
	s_options_controls_lookstrafe_box.itemnames = yesno_names;

	s_options_controls_freelook_box.generic.type = MTYPE_SPINCONTROL;
	s_options_controls_freelook_box.generic.x	= 0;
	s_options_controls_freelook_box.generic.y	= y+=MENU_LINE_SIZE;
	s_options_controls_freelook_box.generic.name	= "free look";
	s_options_controls_freelook_box.generic.callback = FreeLookFunc;
	s_options_controls_freelook_box.itemnames = yesno_names;

	*/

	s_options_controls_joystick_box.generic.type = MTYPE_CHECKBOX;
	s_options_controls_joystick_box.generic.x	= 0;
	s_options_controls_joystick_box.generic.y	= y+=MENU_LINE_SIZE*2;
	s_options_controls_joystick_box.generic.name	= "Use Gamepad";
	s_options_controls_joystick_box.generic.callback = JoystickFunc;
	s_options_controls_joystick_box.itemnames = yesno_names;


	s_options_controls_console_box.generic.type = MTYPE_CHECKBOX;
	s_options_controls_console_box.generic.x	= 0;
	s_options_controls_console_box.generic.y	= y+=MENU_LINE_SIZE*2;
	s_options_controls_console_box.generic.name	= "Enable Console";
	s_options_controls_console_box.generic.callback = ConsoleFunc;
	s_options_controls_console_box.itemnames = yesno_names;



	s_options_controls_customize_keys_action.generic.type	= MTYPE_ACTION;
	s_options_controls_customize_keys_action.generic.x		= MENU_FONT_SIZE;
	s_options_controls_customize_keys_action.generic.y		= y+=2*MENU_LINE_SIZE;
	s_options_controls_customize_keys_action.generic.name	= "Customize Keys";
	s_options_controls_customize_keys_action.generic.callback = CustomizeControlsFunc;

	s_options_controls_defaults_action.generic.type	= MTYPE_ACTION;
	s_options_controls_defaults_action.generic.x		= MENU_FONT_SIZE;
	s_options_controls_defaults_action.generic.y		= 18*MENU_LINE_SIZE;
	s_options_controls_defaults_action.generic.name	= "Reset Defaults";
	s_options_controls_defaults_action.generic.callback = ControlsResetDefaultsFunc;

	s_options_controls_back_action.generic.type	= MTYPE_ACTION;
	s_options_controls_back_action.generic.x		= MENU_FONT_SIZE;
	s_options_controls_back_action.generic.y		= 20*MENU_LINE_SIZE;
	s_options_controls_back_action.generic.name	= "Done";
	s_options_controls_back_action.generic.callback = UI_BackMenu;

	Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_header );
	Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_sensitivity_slider );
	Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_invertmouse_box );
	//Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_thirdperson_box );
	//Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_thirdperson_distance_slider );
	//Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_thirdperson_angle_slider );
	//Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_alwaysrun_box );
	//Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_lookspring_box );
	//Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_lookstrafe_box );
	//Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_freelook_box );
	Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_joystick_box );

	Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_console_box );

	Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_customize_keys_action );
	Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_defaults_action );
	Menu_AddItem( &s_options_controls_menu, ( void * ) &s_options_controls_back_action );

	ControlsSetMenuItemValues();
}

void Options_Controls_MenuDraw (void)
{
	Menu_Banner( "m_banner_options" );

	Menu_AdjustCursor( &s_options_controls_menu, 1 );
	Menu_Draw( &s_options_controls_menu );
}

const char *Options_Controls_MenuKey( int key )
{
	return Default_MenuKey( &s_options_controls_menu, key );
}

void M_Menu_Options_Controls_f (void)
{
	Options_Controls_MenuInit();
	UI_PushMenu ( Options_Controls_MenuDraw, Options_Controls_MenuKey );
}
