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

// ui_options_effects.c -- the effects options menu

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

/*
=======================================================================

EFFECTS MENU

=======================================================================
*/

static menuframework_s	s_options_effects_menu;
static menuseparator_s	s_options_effects_header;
static menulist_s		s_options_effects_blood_box;
static menulist_s		s_options_effects_oldexplosions_box;
static menulist_s		s_options_effects_plasmaexplosound_box;
static menulist_s		s_options_effects_itembob_box;
static menuslider_s		s_options_effects_decal_slider;
static menuslider_s		s_options_effects_particle_comp_slider;
static menulist_s		s_options_effects_railtrail_box;
static menuslider_s		s_options_effects_railcolor_slider[3];
static menulist_s		s_options_effects_footstep_box;
static menuaction_s		s_options_effects_defaults_action;
static menuaction_s		s_options_effects_back_action;


static void BloodFunc( void *unused )
{
	Cvar_SetValue( "cl_blood", s_options_effects_blood_box.curvalue );
}

static void OldExplosionFunc( void *unused )
{
	Cvar_SetValue( "cl_old_explosions", s_options_effects_oldexplosions_box.curvalue );
}

static void PlasmaExploSoundFunc( void *unused )
{
	Cvar_SetValue( "cl_plasma_explo_sound", s_options_effects_plasmaexplosound_box.curvalue );
}

static void ItemBobFunc( void *unused )
{
	Cvar_SetValue( "cl_item_bobbing", s_options_effects_itembob_box.curvalue );
}

static void ParticleCompFunc( void *unused )
{
//	Cvar_SetValue( "cl_particle_scale", (s_options_effects_particle_comp_slider.curvalue-3)*-1+3);
	Cvar_SetValue( "cl_particle_scale", MenuSlider_GetValue(&s_options_effects_particle_comp_slider) );
}

static void DecalCallback( void *unused )
{
//	Cvar_SetValue( "r_decals", s_options_effects_decal_slider.curvalue * 50);
	Cvar_SetValue( "r_decals", MenuSlider_GetValue(&s_options_effects_decal_slider) );
}

// Psychospaz's changeable rail trail
static void RailTrailFunc( void *unused )
{
	Cvar_SetValue( "cl_railtype", s_options_effects_railtrail_box.curvalue );
}

static void RailColorRedFunc( void *unused )
{
//	Cvar_SetValue( "cl_railred", s_options_effects_railcolor_slider[0].curvalue*16 );
	Cvar_SetValue( "cl_railred", MenuSlider_GetValue(&s_options_effects_railcolor_slider[0]) );
}

static void RailColorGreenFunc( void *unused )
{
//	Cvar_SetValue( "cl_railgreen", s_options_effects_railcolor_slider[1].curvalue*16 );
	Cvar_SetValue( "cl_railgreen", MenuSlider_GetValue(&s_options_effects_railcolor_slider[1]) );
}

static void RailColorBlueFunc( void *unused )
{
//	Cvar_SetValue( "cl_railblue", s_options_effects_railcolor_slider[2].curvalue*16 );
	Cvar_SetValue( "cl_railblue", MenuSlider_GetValue(&s_options_effects_railcolor_slider[2]) );
}

// foostep override option
static void FootStepFunc( void *unused )
{
	Cvar_SetValue( "cl_footstep_override", s_options_effects_footstep_box.curvalue );
}


//=======================================================================

static void EffectsSetMenuItemValues( void )
{
	Cvar_SetValue( "cl_blood", ClampCvar( 0, 4, Cvar_VariableValue("cl_blood") ) );
	s_options_effects_blood_box.curvalue			= Cvar_VariableValue("cl_blood");

	Cvar_SetValue( "cl_old_explosions", ClampCvar( 0, 1, Cvar_VariableValue("cl_old_explosions") ) );
	s_options_effects_oldexplosions_box.curvalue = Cvar_VariableValue("cl_old_explosions");

	Cvar_SetValue( "cl_plasma_explo_sound", ClampCvar( 0, 1, Cvar_VariableValue("cl_plasma_explo_sound") ) );
	s_options_effects_plasmaexplosound_box.curvalue = Cvar_VariableValue("cl_plasma_explo_sound");

	Cvar_SetValue( "cl_item_bobbing", ClampCvar( 0, 1, Cvar_VariableValue("cl_item_bobbing") ) );
	s_options_effects_itembob_box.curvalue = Cvar_VariableValue("cl_item_bobbing");

	Cvar_SetValue( "r_decals", ClampCvar (0, 1000, Cvar_VariableValue("r_decals")) );
//	s_options_effects_decal_slider.curvalue = Cvar_VariableValue("r_decals") / 50;
	MenuSlider_SetValue (&s_options_effects_decal_slider, Cvar_VariableValue("r_decals"));

	Cvar_SetValue( "cl_particle_scale", ClampCvar( 0, 5, Cvar_VariableValue("cl_particle_scale") ) );
//	s_options_effects_particle_comp_slider.curvalue	= (Cvar_VariableValue("cl_particle_scale") -3)*-1+3;
	MenuSlider_SetValue (&s_options_effects_particle_comp_slider, Cvar_VariableValue("cl_particle_scale"));

	Cvar_SetValue( "cl_railtype", ClampCvar( 0, 2, Cvar_VariableValue("cl_railtype") ) );
	s_options_effects_railtrail_box.curvalue		= Cvar_VariableValue("cl_railtype");
//	s_options_effects_railcolor_slider[0].curvalue		= Cvar_VariableValue("cl_railred")/16;
//	s_options_effects_railcolor_slider[1].curvalue		= Cvar_VariableValue("cl_railgreen")/16;
//	s_options_effects_railcolor_slider[2].curvalue		= Cvar_VariableValue("cl_railblue")/16;
	MenuSlider_SetValue (&s_options_effects_railcolor_slider[0], Cvar_VariableValue("cl_railred"));
	MenuSlider_SetValue (&s_options_effects_railcolor_slider[1], Cvar_VariableValue("cl_railgreen"));
	MenuSlider_SetValue (&s_options_effects_railcolor_slider[2], Cvar_VariableValue("cl_railblue"));

	Cvar_SetValue( "cl_footstep_override", ClampCvar( 0, 1, Cvar_VariableValue("cl_footstep_override") ) );
	s_options_effects_footstep_box.curvalue			= Cvar_VariableValue("cl_footstep_override");
}

static void EffectsResetDefaultsFunc ( void *unused )
{
	Cvar_SetToDefault ("cl_blood");
	Cvar_SetToDefault ("cl_old_explosions");
	Cvar_SetToDefault ("cl_plasma_explo_sound");
	Cvar_SetToDefault ("cl_item_bobbing");
	Cvar_SetToDefault ("r_decals");
	Cvar_SetToDefault ("cl_particle_scale");
	Cvar_SetToDefault ("cl_railtype");
	Cvar_SetToDefault ("cl_railred");
	Cvar_SetToDefault ("cl_railgreen");
	Cvar_SetToDefault ("cl_railblue");	
	Cvar_SetToDefault ("cl_footstep_override");

	EffectsSetMenuItemValues();
}

void Options_Effects_MenuInit ( void )
{
	static const char *yesno_names[] =
	{
		"no",
		"yes",
		0
	};

	static const char *blood_names[] =
	{
		"none",
		"puff",
		"splat",
		"bleed",
		"gore",
		0
	};

	static const char *railtrail_names[] =
	{
		"colored spiral",
		"colored beam", // laser
		"colored devrail",
		0
	};


	int y = 3*MENU_LINE_SIZE;

	s_options_effects_menu.x = SCREEN_WIDTH*0.5;
	s_options_effects_menu.y = SCREEN_HEIGHT*0.5 - 58;
	s_options_effects_menu.nitems = 0;

	s_options_effects_header.generic.type			= MTYPE_SEPARATOR;
	s_options_effects_header.generic.textSize		= MENU_HEADER_FONT_SIZE;
	s_options_effects_header.generic.name			= "Effects";
	s_options_effects_header.generic.x				= MENU_HEADER_FONT_SIZE/2 * (int)strlen(s_options_effects_header.generic.name);
	s_options_effects_header.generic.y				= -2*MENU_LINE_SIZE;	//	0
	
	s_options_effects_blood_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_effects_blood_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_effects_blood_box.generic.x			= 0;
	s_options_effects_blood_box.generic.y			= y;
	s_options_effects_blood_box.generic.name		= "blood type";
	s_options_effects_blood_box.generic.callback	= BloodFunc;
	s_options_effects_blood_box.itemnames			= blood_names;
	s_options_effects_blood_box.generic.statusbar	= "changes blood effect type";

	s_options_effects_oldexplosions_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_effects_oldexplosions_box.generic.textSize	= MENU_FONT_SIZE;
	s_options_effects_oldexplosions_box.generic.x			= 0;
	s_options_effects_oldexplosions_box.generic.y			= y += 2*MENU_LINE_SIZE;
	s_options_effects_oldexplosions_box.generic.name		= "old style explosions";
	s_options_effects_oldexplosions_box.generic.callback	= OldExplosionFunc;
	s_options_effects_oldexplosions_box.itemnames			= yesno_names;
	s_options_effects_oldexplosions_box.generic.statusbar	= "brings back those cheesy model explosions";

	s_options_effects_plasmaexplosound_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_effects_plasmaexplosound_box.generic.textSize		= MENU_FONT_SIZE;
	s_options_effects_plasmaexplosound_box.generic.x			= 0;
	s_options_effects_plasmaexplosound_box.generic.y			= y += MENU_LINE_SIZE;
	s_options_effects_plasmaexplosound_box.generic.name			= "unique plasma explode sound";
	s_options_effects_plasmaexplosound_box.generic.callback		= PlasmaExploSoundFunc;
	s_options_effects_plasmaexplosound_box.itemnames			= yesno_names;
	s_options_effects_plasmaexplosound_box.generic.statusbar	= "gives Phalanx Cannon plasma explosions a unique sound";

	s_options_effects_itembob_box.generic.type					= MTYPE_SPINCONTROL;
	s_options_effects_itembob_box.generic.textSize				= MENU_FONT_SIZE;
	s_options_effects_itembob_box.generic.x						= 0;
	s_options_effects_itembob_box.generic.y						= y += MENU_LINE_SIZE;
	s_options_effects_itembob_box.generic.name					= "item bobbing";
	s_options_effects_itembob_box.generic.callback				= ItemBobFunc;
	s_options_effects_itembob_box.itemnames						= yesno_names;
	s_options_effects_itembob_box.generic.statusbar				= "adds bobbing effect to rotating items";

	s_options_effects_decal_slider.generic.type					= MTYPE_SLIDER;
	s_options_effects_decal_slider.generic.textSize				= MENU_FONT_SIZE;
	s_options_effects_decal_slider.generic.x					= 0;
	s_options_effects_decal_slider.generic.y					= y += 2*MENU_LINE_SIZE;
	s_options_effects_decal_slider.generic.name					= "decal quantity";
	s_options_effects_decal_slider.generic.callback				= DecalCallback;
//	s_options_effects_decal_slider.minvalue						= 0;
//	s_options_effects_decal_slider.maxvalue						= 20;
	s_options_effects_decal_slider.maxPos						= 20;
	s_options_effects_decal_slider.baseValue					= 0.0f;
	s_options_effects_decal_slider.increment					= 50.0f;
	s_options_effects_decal_slider.displayAsPercent				= false;
	s_options_effects_decal_slider.generic.statusbar			= "how many decals to display at once (max = 1000)";

	s_options_effects_particle_comp_slider.generic.type			= MTYPE_SLIDER;
	s_options_effects_particle_comp_slider.generic.textSize		= MENU_FONT_SIZE;
	s_options_effects_particle_comp_slider.generic.x			= 0;
	s_options_effects_particle_comp_slider.generic.y			= y += MENU_LINE_SIZE;
	s_options_effects_particle_comp_slider.generic.name			= "particle effect complexity";
	s_options_effects_particle_comp_slider.generic.callback		= ParticleCompFunc;
//	s_options_effects_particle_comp_slider.minvalue				= 1;
//	s_options_effects_particle_comp_slider.maxvalue				= 5;
	s_options_effects_particle_comp_slider.maxPos				= 4;
	s_options_effects_particle_comp_slider.baseValue			= 5.0f;
	s_options_effects_particle_comp_slider.increment			= -1.0f;
	s_options_effects_particle_comp_slider.displayAsPercent		= false;
	s_options_effects_particle_comp_slider.generic.statusbar	= "lower = faster performance";

	// Psychospaz's changeable rail trail
	s_options_effects_railtrail_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_effects_railtrail_box.generic.textSize		= MENU_FONT_SIZE;
	s_options_effects_railtrail_box.generic.x				= 0;
	s_options_effects_railtrail_box.generic.y				= y += 2*MENU_LINE_SIZE;
	s_options_effects_railtrail_box.generic.name			= "railtrail type";
	s_options_effects_railtrail_box.generic.callback		= RailTrailFunc;
	s_options_effects_railtrail_box.itemnames				= railtrail_names;
	s_options_effects_railtrail_box.generic.statusbar		= "changes railgun particle effect";

	s_options_effects_railcolor_slider[0].generic.type		= MTYPE_SLIDER;
	s_options_effects_railcolor_slider[0].generic.textSize	= MENU_FONT_SIZE;
	s_options_effects_railcolor_slider[0].generic.x			= 0;
	s_options_effects_railcolor_slider[0].generic.y			= y += MENU_LINE_SIZE;
	s_options_effects_railcolor_slider[0].generic.name		= "railtrail - red";
	s_options_effects_railcolor_slider[0].generic.callback	= RailColorRedFunc;
//	s_options_effects_railcolor_slider[0].minvalue			= 0;
//	s_options_effects_railcolor_slider[0].maxvalue			= 16;
	s_options_effects_railcolor_slider[0].maxPos			= 64;
	s_options_effects_railcolor_slider[0].baseValue			= 0.0f;
	s_options_effects_railcolor_slider[0].increment			= 4.0f;
	s_options_effects_railcolor_slider[0].displayAsPercent	= false;
	s_options_effects_railcolor_slider[0].generic.statusbar	= "changes railtrail red component";

	s_options_effects_railcolor_slider[1].generic.type		= MTYPE_SLIDER;
	s_options_effects_railcolor_slider[1].generic.textSize	= MENU_FONT_SIZE;
	s_options_effects_railcolor_slider[1].generic.x			= 0;
	s_options_effects_railcolor_slider[1].generic.y			= y += MENU_LINE_SIZE;
	s_options_effects_railcolor_slider[1].generic.name		= "railtrail - green";
	s_options_effects_railcolor_slider[1].generic.callback	= RailColorGreenFunc;
//	s_options_effects_railcolor_slider[1].minvalue			= 0;
//	s_options_effects_railcolor_slider[1].maxvalue			= 16;
	s_options_effects_railcolor_slider[1].maxPos			= 64;
	s_options_effects_railcolor_slider[1].baseValue			= 0.0f;
	s_options_effects_railcolor_slider[1].increment			= 4.0f;
	s_options_effects_railcolor_slider[1].displayAsPercent	= false;
	s_options_effects_railcolor_slider[1].generic.statusbar	= "changes railtrail green component";

	s_options_effects_railcolor_slider[2].generic.type		= MTYPE_SLIDER;
	s_options_effects_railcolor_slider[2].generic.textSize	= MENU_FONT_SIZE;
	s_options_effects_railcolor_slider[2].generic.x			= 0;
	s_options_effects_railcolor_slider[2].generic.y			= y += MENU_LINE_SIZE;
	s_options_effects_railcolor_slider[2].generic.name		= "railtrail - blue";
	s_options_effects_railcolor_slider[2].generic.callback	= RailColorBlueFunc;
//	s_options_effects_railcolor_slider[2].minvalue			= 0;
//	s_options_effects_railcolor_slider[2].maxvalue			= 16;
	s_options_effects_railcolor_slider[2].maxPos			= 64;
	s_options_effects_railcolor_slider[2].baseValue			= 0.0f;
	s_options_effects_railcolor_slider[2].increment			= 4.0f;
	s_options_effects_railcolor_slider[2].displayAsPercent	= false;
	s_options_effects_railcolor_slider[2].generic.statusbar	= "changes railtrail blue component";

	// foostep override option
	s_options_effects_footstep_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_effects_footstep_box.generic.textSize		= MENU_FONT_SIZE;
	s_options_effects_footstep_box.generic.x			= 0;
	s_options_effects_footstep_box.generic.y			= y += 2*MENU_LINE_SIZE;
	s_options_effects_footstep_box.generic.name			= "override footstep sounds";
	s_options_effects_footstep_box.generic.callback		= FootStepFunc;
	s_options_effects_footstep_box.itemnames			= yesno_names;
	s_options_effects_footstep_box.generic.statusbar	= "sets footstep sounds with definitions in texsurfs.txt";

	s_options_effects_defaults_action.generic.type		= MTYPE_ACTION;
	s_options_effects_defaults_action.generic.textSize	= MENU_FONT_SIZE;
	s_options_effects_defaults_action.generic.x			= MENU_FONT_SIZE;
	s_options_effects_defaults_action.generic.y			= y += 2*MENU_LINE_SIZE;
	s_options_effects_defaults_action.generic.name		= "reset defaults";
	s_options_effects_defaults_action.generic.callback	= EffectsResetDefaultsFunc;
	s_options_effects_defaults_action.generic.statusbar	= "resets all effects settings to internal defaults";

	s_options_effects_back_action.generic.type			= MTYPE_ACTION;
	s_options_effects_back_action.generic.textSize		= MENU_FONT_SIZE;
	s_options_effects_back_action.generic.x				= MENU_FONT_SIZE;
	s_options_effects_back_action.generic.y				= y += 2*MENU_LINE_SIZE;
	s_options_effects_back_action.generic.name			= "back to options";
	s_options_effects_back_action.generic.callback		= UI_BackMenu;

	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_header );
	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_blood_box );
	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_oldexplosions_box );
	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_plasmaexplosound_box );
	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_itembob_box );
	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_decal_slider );
	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_particle_comp_slider );
	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_railtrail_box );
	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_railcolor_slider[0] );
	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_railcolor_slider[1] );
	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_railcolor_slider[2] );
	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_footstep_box );
	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_defaults_action );
	Menu_AddItem( &s_options_effects_menu, ( void * ) &s_options_effects_back_action );

	EffectsSetMenuItemValues();
}

void Options_Effects_MenuDraw (void)
{
	Menu_DrawBanner( "m_banner_options" );

	Menu_AdjustCursor( &s_options_effects_menu, 1 );
	Menu_Draw( &s_options_effects_menu );
}

const char *Options_Effects_MenuKey( int key )
{
	return Default_MenuKey( &s_options_effects_menu, key );
}

void M_Menu_Options_Effects_f (void)
{
	Options_Effects_MenuInit();
	UI_PushMenu ( Options_Effects_MenuDraw, Options_Effects_MenuKey );
}
