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

// ui_dmoptions.c -- DM options menu 

#include "../client/client.h"
#include "ui_local.h"

/*
=============================================================================

DMOPTIONS BOOK MENU

=============================================================================
*/

static menuFramework_s	s_dmoptions_menu;
static menuImage_s		s_dmoptions_banner;

static menuPicker_s		s_friendlyfire_box;
static menuPicker_s		s_falls_box;
static menuPicker_s		s_weapons_stay_box;
static menuPicker_s		s_instant_powerups_box;
static menuPicker_s		s_powerups_box;
static menuPicker_s		s_health_box;
static menuPicker_s		s_spawn_farthest_box;
static menuPicker_s		s_teamplay_box;
static menuPicker_s		s_samelevel_box;
static menuPicker_s		s_force_respawn_box;
static menuPicker_s		s_armor_box;
static menuPicker_s		s_allow_exit_box;
static menuPicker_s		s_infinite_ammo_box;
static menuPicker_s		s_fixed_fov_box;
static menuPicker_s		s_quad_drop_box;

// Xatrix
static menuPicker_s		s_quadfire_drop_box;

// ROGUE
static menuPicker_s		s_no_mines_box;
static menuPicker_s		s_no_nukes_box;
static menuPicker_s		s_stack_double_box;
static menuPicker_s		s_no_spheres_box;

// CTF
static menuPicker_s		s_ctf_forceteam_box;
static menuPicker_s		s_ctf_armor_protect_box;
static menuPicker_s		s_ctf_notechs_box;

static menuAction_s		s_dmoptions_back_action;

#define DF_CTF_FORCEJOIN	131072	
#define DF_ARMOR_PROTECT	262144
#define DF_CTF_NO_TECH      524288

//=======================================================================

void Menu_DMOptions_Init (void)
{
	static const char *yes_no_names[] =
	{
		"no", "yes", 0
	};
	static const char *teamplay_names[] = 
	{
		"disabled", "by skin", "by model", 0
	};
	static const int teamplay_bitflags[] =
	{
		0, DF_SKINTEAMS, DF_MODELTEAMS
	};
	int		x, y;

	// menu.x = 320, menu.y = 160
	x = SCREEN_WIDTH*0.5;
	y = SCREEN_HEIGHT*0.5 - 8*MENU_LINE_SIZE;

	s_dmoptions_menu.x					= 0;	// SCREEN_WIDTH*0.5;
	s_dmoptions_menu.y					= 0;	// SCREEN_HEIGHT*0.5 - 80;
	s_dmoptions_menu.nitems				= 0;
	s_dmoptions_menu.isPopup			= false;
	s_dmoptions_menu.background			= NULL;
	s_dmoptions_menu.drawFunc			= UI_DefaultMenuDraw;
	s_dmoptions_menu.keyFunc			= UI_DefaultMenuKey;
	s_dmoptions_menu.canOpenFunc		= UI_CanOpenDMFlagsMenu;
	s_dmoptions_menu.cantOpenMessage	= "N/A for cooperative";
	s_dmoptions_menu.flagCvar			= "dmflags";

	s_dmoptions_banner.generic.type		= MTYPE_IMAGE;
	s_dmoptions_banner.generic.x		= 0;
	s_dmoptions_banner.generic.y		= 9*MENU_LINE_SIZE;
	s_dmoptions_banner.width			= 275;
	s_dmoptions_banner.height			= 32;
	s_dmoptions_banner.imageName		= "/pics/m_banner_start_server.pcx";
	s_dmoptions_banner.alpha			= 255;
	s_dmoptions_banner.border			= 0;
	s_dmoptions_banner.hCentered		= true;
	s_dmoptions_banner.vCentered		= false;
	s_dmoptions_banner.useAspectRatio	= false;
	s_dmoptions_banner.useAspectRatio	= false;
	s_dmoptions_banner.generic.isHidden	= false;

	s_falls_box.generic.type		= MTYPE_PICKER;
	s_falls_box.generic.textSize	= MENU_FONT_SIZE;
	s_falls_box.generic.x			= x;
	s_falls_box.generic.y			= y; // 0
	s_falls_box.generic.name		= "falling damage";
	s_falls_box.itemNames			= yes_no_names;
	s_falls_box.bitFlag				= DF_NO_FALLING;
	s_falls_box.invertValue			= true;

	s_weapons_stay_box.generic.type		= MTYPE_PICKER;
	s_weapons_stay_box.generic.textSize	= MENU_FONT_SIZE;
	s_weapons_stay_box.generic.x		= x;
	s_weapons_stay_box.generic.y		= y += MENU_LINE_SIZE;
	s_weapons_stay_box.generic.name		= "weapons stay";
	s_weapons_stay_box.itemNames		= yes_no_names;
	s_weapons_stay_box.bitFlag			= DF_WEAPONS_STAY;
	s_weapons_stay_box.invertValue		= false;

	s_instant_powerups_box.generic.type		= MTYPE_PICKER;
	s_instant_powerups_box.generic.textSize	= MENU_FONT_SIZE;
	s_instant_powerups_box.generic.x		= x;
	s_instant_powerups_box.generic.y		= y += MENU_LINE_SIZE;
	s_instant_powerups_box.generic.name		= "instant powerups";
	s_instant_powerups_box.itemNames		= yes_no_names;
	s_instant_powerups_box.bitFlag			= DF_INSTANT_ITEMS;
	s_instant_powerups_box.invertValue		= false;

	s_powerups_box.generic.type		= MTYPE_PICKER;
	s_powerups_box.generic.textSize	= MENU_FONT_SIZE;
	s_powerups_box.generic.x		= x;
	s_powerups_box.generic.y		= y += MENU_LINE_SIZE;
	s_powerups_box.generic.name		= "allow powerups";
	s_powerups_box.itemNames		= yes_no_names;
	s_powerups_box.bitFlag			= DF_NO_ITEMS;
	s_powerups_box.invertValue		= true;

	s_health_box.generic.type		= MTYPE_PICKER;
	s_health_box.generic.textSize	= MENU_FONT_SIZE;
	s_health_box.generic.x			= x;
	s_health_box.generic.y			= y += MENU_LINE_SIZE;
	s_health_box.generic.name		= "allow health";
	s_health_box.itemNames			= yes_no_names;
	s_health_box.bitFlag			= DF_NO_HEALTH;
	s_health_box.invertValue		= true;

	s_armor_box.generic.type		= MTYPE_PICKER;
	s_armor_box.generic.textSize	= MENU_FONT_SIZE;
	s_armor_box.generic.x			= x;
	s_armor_box.generic.y			= y += MENU_LINE_SIZE;
	s_armor_box.generic.name		= "allow armor";
	s_armor_box.itemNames			= yes_no_names;
	s_armor_box.bitFlag				= DF_NO_ARMOR;
	s_armor_box.invertValue			= true;

	s_spawn_farthest_box.generic.type		= MTYPE_PICKER;
	s_spawn_farthest_box.generic.textSize	= MENU_FONT_SIZE;
	s_spawn_farthest_box.generic.x			= x;
	s_spawn_farthest_box.generic.y			= y += MENU_LINE_SIZE;
	s_spawn_farthest_box.generic.name		= "spawn farthest";
	s_spawn_farthest_box.itemNames			= yes_no_names;
	s_spawn_farthest_box.bitFlag			= DF_SPAWN_FARTHEST;
	s_spawn_farthest_box.invertValue		= false;

	s_samelevel_box.generic.type		= MTYPE_PICKER;
	s_samelevel_box.generic.textSize	= MENU_FONT_SIZE;
	s_samelevel_box.generic.x			= x;
	s_samelevel_box.generic.y			= y += MENU_LINE_SIZE;
	s_samelevel_box.generic.name		= "same map";
	s_samelevel_box.itemNames			= yes_no_names;
	s_samelevel_box.bitFlag				= DF_SAME_LEVEL;
	s_samelevel_box.invertValue			= false;

	s_force_respawn_box.generic.type		= MTYPE_PICKER;
	s_force_respawn_box.generic.textSize	= MENU_FONT_SIZE;
	s_force_respawn_box.generic.x			= x;
	s_force_respawn_box.generic.y			= y += MENU_LINE_SIZE;
	s_force_respawn_box.generic.name		= "force respawn";
	s_force_respawn_box.itemNames			= yes_no_names;
	s_force_respawn_box.bitFlag				= DF_FORCE_RESPAWN;
	s_force_respawn_box.invertValue			= false;

	s_teamplay_box.generic.type			= MTYPE_PICKER;
	s_teamplay_box.generic.textSize		= MENU_FONT_SIZE;
	s_teamplay_box.generic.x			= x;
	s_teamplay_box.generic.y			= y += MENU_LINE_SIZE;
	s_teamplay_box.generic.name			= "teamplay";
	s_teamplay_box.itemNames			= teamplay_names;
	s_teamplay_box.bitFlags				= teamplay_bitflags;

	s_allow_exit_box.generic.type		= MTYPE_PICKER;
	s_allow_exit_box.generic.textSize	= MENU_FONT_SIZE;
	s_allow_exit_box.generic.x			= x;
	s_allow_exit_box.generic.y			= y += MENU_LINE_SIZE;
	s_allow_exit_box.generic.name		= "allow exit";
	s_allow_exit_box.itemNames			= yes_no_names;
	s_allow_exit_box.bitFlag			= DF_ALLOW_EXIT;
	s_allow_exit_box.invertValue		= false;

	s_infinite_ammo_box.generic.type		= MTYPE_PICKER;
	s_infinite_ammo_box.generic.textSize	= MENU_FONT_SIZE;
	s_infinite_ammo_box.generic.x			= x;
	s_infinite_ammo_box.generic.y			= y += MENU_LINE_SIZE;
	s_infinite_ammo_box.generic.name		= "infinite ammo";
	s_infinite_ammo_box.itemNames			= yes_no_names;
	s_infinite_ammo_box.bitFlag				= DF_INFINITE_AMMO;
	s_infinite_ammo_box.invertValue			= false;

	s_fixed_fov_box.generic.type		= MTYPE_PICKER;
	s_fixed_fov_box.generic.textSize	= MENU_FONT_SIZE;
	s_fixed_fov_box.generic.x			= x;
	s_fixed_fov_box.generic.y			= y += MENU_LINE_SIZE;
	s_fixed_fov_box.generic.name		= "fixed FOV";
	s_fixed_fov_box.itemNames			= yes_no_names;
	s_fixed_fov_box.bitFlag				= DF_FIXED_FOV;
	s_fixed_fov_box.invertValue			= false;

	s_quad_drop_box.generic.type		= MTYPE_PICKER;
	s_quad_drop_box.generic.textSize	= MENU_FONT_SIZE;
	s_quad_drop_box.generic.x			= x;
	s_quad_drop_box.generic.y			= y += MENU_LINE_SIZE;
	s_quad_drop_box.generic.name		= "quad drop";
	s_quad_drop_box.itemNames			= yes_no_names;
	s_quad_drop_box.bitFlag				= DF_QUAD_DROP;
	s_quad_drop_box.invertValue			= false;

	s_friendlyfire_box.generic.type		= MTYPE_PICKER;
	s_friendlyfire_box.generic.textSize	= MENU_FONT_SIZE;
	s_friendlyfire_box.generic.x		= x;
	s_friendlyfire_box.generic.y		= y += MENU_LINE_SIZE;
	s_friendlyfire_box.generic.name		= "friendly fire";
	s_friendlyfire_box.itemNames		= yes_no_names;
	s_friendlyfire_box.bitFlag			= DF_NO_FRIENDLY_FIRE;
	s_friendlyfire_box.invertValue		= true;

	// Xatrix
	if ( FS_XatrixPath() )
	{
		s_quadfire_drop_box.generic.type		= MTYPE_PICKER;
		s_quadfire_drop_box.generic.textSize	= MENU_FONT_SIZE;
		s_quadfire_drop_box.generic.x			= x;
		s_quadfire_drop_box.generic.y			= y += MENU_LINE_SIZE;
		s_quadfire_drop_box.generic.name		= "dualfire drop";
		s_quadfire_drop_box.itemNames			= yes_no_names;
		s_quadfire_drop_box.bitFlag				= DF_QUADFIRE_DROP;
		s_quadfire_drop_box.invertValue			= false;
	}
	// Rogue
	else if ( FS_RoguePath() )
	{
		s_no_mines_box.generic.type			= MTYPE_PICKER;
		s_no_mines_box.generic.textSize		= MENU_FONT_SIZE;
		s_no_mines_box.generic.x			= x;
		s_no_mines_box.generic.y			= y += MENU_LINE_SIZE;
		s_no_mines_box.generic.name			= "remove mines";
		s_no_mines_box.itemNames			= yes_no_names;
		s_no_mines_box.bitFlag				= DF_NO_MINES;
		s_no_mines_box.invertValue			= false;

		s_no_nukes_box.generic.type			= MTYPE_PICKER;
		s_no_nukes_box.generic.textSize		= MENU_FONT_SIZE;
		s_no_nukes_box.generic.x			= x;
		s_no_nukes_box.generic.y			= y += MENU_LINE_SIZE;
		s_no_nukes_box.generic.name			= "remove nukes";
		s_no_nukes_box.itemNames			= yes_no_names;
		s_no_nukes_box.bitFlag				= DF_NO_NUKES;
		s_no_nukes_box.invertValue			= false;

		s_stack_double_box.generic.type		= MTYPE_PICKER;
		s_stack_double_box.generic.textSize	= MENU_FONT_SIZE;
		s_stack_double_box.generic.x		= x;
		s_stack_double_box.generic.y		= y += MENU_LINE_SIZE;
		s_stack_double_box.generic.name		= "2x/4x stacking off";
		s_stack_double_box.itemNames		= yes_no_names;
		s_stack_double_box.bitFlag			= DF_NO_STACK_DOUBLE;
		s_stack_double_box.invertValue		= false;

		s_no_spheres_box.generic.type		= MTYPE_PICKER;
		s_no_spheres_box.generic.textSize	= MENU_FONT_SIZE;
		s_no_spheres_box.generic.x			= x;
		s_no_spheres_box.generic.y			= y += MENU_LINE_SIZE;
		s_no_spheres_box.generic.name		= "remove spheres";
		s_no_spheres_box.itemNames			= yes_no_names;
		s_no_spheres_box.bitFlag			= DF_NO_SPHERES;
		s_no_spheres_box.invertValue		= false;
	}
	// CTF
	else if ( UI_CTF_MenuMode() )
	{
		s_ctf_forceteam_box.generic.type			= MTYPE_PICKER;
		s_ctf_forceteam_box.generic.textSize		= MENU_FONT_SIZE;
		s_ctf_forceteam_box.generic.x				= x;
		s_ctf_forceteam_box.generic.y				= y += MENU_LINE_SIZE;
		s_ctf_forceteam_box.generic.name			= "force team join";
		s_ctf_forceteam_box.itemNames				= yes_no_names;
		s_ctf_forceteam_box.bitFlag					= DF_CTF_FORCEJOIN;
		s_ctf_forceteam_box.invertValue				= false;

		s_ctf_armor_protect_box.generic.type		= MTYPE_PICKER;
		s_ctf_armor_protect_box.generic.textSize	= MENU_FONT_SIZE;
		s_ctf_armor_protect_box.generic.x			= x;
		s_ctf_armor_protect_box.generic.y			= y += MENU_LINE_SIZE;
		s_ctf_armor_protect_box.generic.name		= "team armor protect";
		s_ctf_armor_protect_box.itemNames			= yes_no_names;
		s_ctf_armor_protect_box.bitFlag				= DF_ARMOR_PROTECT;
		s_ctf_armor_protect_box.invertValue			= false;

		s_ctf_notechs_box.generic.type				= MTYPE_PICKER;
		s_ctf_notechs_box.generic.textSize			= MENU_FONT_SIZE;
		s_ctf_notechs_box.generic.x					= x;
		s_ctf_notechs_box.generic.y					= y += MENU_LINE_SIZE;
		s_ctf_notechs_box.generic.name				= "disable techs";
		s_ctf_notechs_box.itemNames					= yes_no_names;
		s_ctf_notechs_box.bitFlag					= DF_CTF_NO_TECH;
		s_ctf_notechs_box.invertValue				= false;
	}

	s_dmoptions_back_action.generic.type		= MTYPE_ACTION;
	s_dmoptions_back_action.generic.textSize	= MENU_FONT_SIZE;
	s_dmoptions_back_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_dmoptions_back_action.generic.x			= x;
	s_dmoptions_back_action.generic.y			= y += 3*MENU_LINE_SIZE;
	s_dmoptions_back_action.generic.name		= "Back";
	s_dmoptions_back_action.generic.callback	= UI_BackMenu;

	UI_AddMenuItem (&s_dmoptions_menu, &s_dmoptions_banner);
	UI_AddMenuItem (&s_dmoptions_menu, &s_falls_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_weapons_stay_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_instant_powerups_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_powerups_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_health_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_armor_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_spawn_farthest_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_samelevel_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_force_respawn_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_teamplay_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_allow_exit_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_infinite_ammo_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_fixed_fov_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_quad_drop_box);
	UI_AddMenuItem (&s_dmoptions_menu, &s_friendlyfire_box);

	// Xatrix
	if ( FS_XatrixPath() )
	{
		UI_AddMenuItem (&s_dmoptions_menu, &s_quadfire_drop_box);
	}

	// Rogue
	else if ( FS_RoguePath() )
	{
		UI_AddMenuItem (&s_dmoptions_menu, &s_no_mines_box);
		UI_AddMenuItem (&s_dmoptions_menu, &s_no_nukes_box);
		UI_AddMenuItem (&s_dmoptions_menu, &s_stack_double_box);
		UI_AddMenuItem (&s_dmoptions_menu, &s_no_spheres_box);
	}

	// CTF
	else if ( UI_CTF_MenuMode() )
	{
		UI_AddMenuItem (&s_dmoptions_menu, &s_ctf_forceteam_box);
		UI_AddMenuItem (&s_dmoptions_menu, &s_ctf_armor_protect_box);
		UI_AddMenuItem (&s_dmoptions_menu, &s_ctf_notechs_box);
	}
	UI_AddMenuItem (&s_dmoptions_menu, &s_dmoptions_back_action);
}


void Menu_DMOptions_f (void)
{
	Menu_DMOptions_Init ();
	UI_PushMenu (&s_dmoptions_menu);
}
