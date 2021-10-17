/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2000-2002 Knightmare

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

#include "g_local.h"

/******* Knightmare's cvar code file ***********/

// enable/disable options
cvar_t	*mega_gibs;			 // whether to spawn extra gibs, default to 0
cvar_t	*player_gib_health;  // what health level to gib players at
cvar_t	*adjust_train_corners; //whether to subtract (1,1,1) from train path corners to fix misalignments

cvar_t	*falling_armor_damage; //whether player's armor absorbs damage from falling
cvar_t	*player_jump_sounds; //whether to play that STUPID grunting sound when the player jumps

cvar_t	*use_vwep;

// weapon balancing
cvar_t	*sk_blaster_damage;
cvar_t	*sk_blaster_damage_dm;
cvar_t	*sk_blaster_speed;
cvar_t	*sk_blaster_color;  //blaster color- 1=yellow, 2=green, 3=blue, 4=red

cvar_t	*sk_shotgun_damage;
cvar_t	*sk_shotgun_count;
cvar_t	*sk_shotgun_hspread;
cvar_t	*sk_shotgun_vspread;

cvar_t	*sk_sshotgun_damage;
cvar_t	*sk_sshotgun_count;
cvar_t	*sk_sshotgun_hspread;
cvar_t	*sk_sshotgun_vspread;

cvar_t	*sk_machinegun_damage;
cvar_t	*sk_machinegun_hspread;
cvar_t	*sk_machinegun_vspread;

cvar_t	*sk_chaingun_damage;
cvar_t	*sk_chaingun_damage_dm;
cvar_t	*sk_chaingun_hspread;
cvar_t	*sk_chaingun_vspread;

cvar_t	*sk_grenade_damage;
cvar_t	*sk_grenade_radius;
cvar_t	*sk_grenade_speed;

cvar_t	*sk_hand_grenade_damage;
cvar_t	*sk_hand_grenade_radius;

cvar_t	*sk_rocket_damage;
cvar_t	*sk_rocket_damage2;
cvar_t	*sk_rocket_rdamage;
cvar_t	*sk_rocket_radius;
cvar_t	*sk_rocket_speed;

cvar_t	*sk_hyperblaster_damage;
cvar_t	*sk_hyperblaster_damage_dm;
cvar_t	*sk_hyperblaster_speed;
cvar_t	*sk_hyperblaster_color;  //hyperblaster color- 1=yellow, 2=green, 3=blue, 4=red

cvar_t	*sk_railgun_damage;
cvar_t	*sk_railgun_damage_dm;
cvar_t	*sk_railgun_skin;
cvar_t	*sk_rail_color;
cvar_t	*sk_rail_color_red;
cvar_t	*sk_rail_color_green;
cvar_t	*sk_rail_color_blue;

cvar_t	*sk_bfg_damage;
cvar_t	*sk_bfg_damage_dm;
cvar_t	*sk_bfg_damage2;
cvar_t	*sk_bfg_damage2_dm;
cvar_t	*sk_bfg_rdamage;
cvar_t	*sk_bfg_radius;
cvar_t	*sk_bfg_speed;

cvar_t	*sk_flaregun_damage;
cvar_t	*sk_flaregun_rdamage;
cvar_t	*sk_flaregun_radius;
cvar_t	*sk_flaregun_speed;
cvar_t	*sk_flare_life;

cvar_t	*sk_tbomb_damage;
cvar_t	*sk_tbomb_radius;
cvar_t	*sk_tbomb_shrapnel;
cvar_t	*sk_tbomb_shrapnel_damage;

cvar_t	*sk_sniperrifle_damage;
cvar_t	*sk_sniperrifle_damage_dm;
cvar_t	*sk_sniperrifle_kick;
cvar_t	*sk_sniperrifle_kick_dm;

cvar_t	*sk_soniccannon_damage;
cvar_t	*sk_soniccannon_damage2;
cvar_t	*sk_soniccannon_radius;
cvar_t	*sk_soniccannon_maxfiretime;
cvar_t	*sk_soniccannon_maxcells;

cvar_t	*sk_empnuke_radius;

cvar_t	*sk_a2k_damage;
cvar_t	*sk_a2k_radius;

cvar_t	*sk_plasmashield_health;
cvar_t	*sk_plasmashield_life;

// DM start values
cvar_t	*sk_dm_start_shells;
cvar_t	*sk_dm_start_bullets;
cvar_t	*sk_dm_start_rockets;
cvar_t	*sk_dm_start_grenades;
cvar_t	*sk_dm_start_cells;
cvar_t	*sk_dm_start_slugs;
cvar_t	*sk_dm_start_flares;
cvar_t	*sk_dm_start_tbombs;
cvar_t	*sk_dm_start_empnuke;
cvar_t	*sk_dm_start_plasmashield;

cvar_t	*sk_dm_start_shotgun;
cvar_t	*sk_dm_start_sshotgun;
cvar_t	*sk_dm_start_machinegun;
cvar_t	*sk_dm_start_chaingun;
cvar_t	*sk_dm_start_grenadelauncher;
cvar_t	*sk_dm_start_rocketlauncher;
cvar_t	*sk_dm_start_hyperblaster;
cvar_t	*sk_dm_start_railgun;
cvar_t	*sk_dm_start_bfg;
cvar_t	*sk_dm_start_flaregun;
cvar_t	*sk_dm_start_sniperrifle;
cvar_t	*sk_dm_start_soniccannon;

// maximum values
cvar_t	*sk_max_health;
cvar_t	*sk_max_health_dm;
cvar_t	*sk_max_armor_jacket;
cvar_t	*sk_max_armor_combat;
cvar_t	*sk_max_armor_body;
cvar_t	*sk_max_bullets;	
cvar_t	*sk_max_shells;	
cvar_t	*sk_max_rockets;	
cvar_t	*sk_max_grenades;	
cvar_t	*sk_max_cells;		
cvar_t	*sk_max_slugs;
cvar_t	*sk_max_flares;
cvar_t	*sk_max_tbombs;
cvar_t	*sk_max_a2k;
cvar_t	*sk_max_empnuke;
cvar_t	*sk_max_plasmashield;

// maximum settings if a player gets a bandolier
cvar_t	*sk_bando_bullets; // 300
cvar_t	*sk_bando_shells;  // 200
cvar_t	*sk_bando_cells;   // 300
cvar_t	*sk_bando_slugs;   // 100
cvar_t	*sk_bando_flares;	// 45

// maximum settings if a player gets a pack
cvar_t	*sk_pack_health;
cvar_t	*sk_pack_armor;
cvar_t	*sk_pack_bullets;  // 300
cvar_t	*sk_pack_shells;   // 200
cvar_t	*sk_pack_rockets;  // 100
cvar_t	*sk_pack_grenades; // 100
cvar_t	*sk_pack_cells;    // 300
cvar_t	*sk_pack_slugs;    // 100
cvar_t	*sk_pack_flares;	// 60
cvar_t	*sk_pack_tbombs;	// 100
cvar_t	*sk_pack_a2k;		// 1
cvar_t	*sk_pack_empnuke;	// 100
cvar_t	*sk_pack_plasmashield; // 40

cvar_t	*sk_box_shells;		// value of shells
cvar_t	*sk_box_bullets;	// value of bullets
cvar_t	*sk_box_grenades;	// value of grenade pack
cvar_t	*sk_box_rockets;	// value of rocket pack
cvar_t	*sk_box_cells;		// value of cell pack
cvar_t	*sk_box_slugs;		// value of slug box
cvar_t	*sk_box_flares;	// value of flares box
cvar_t	*sk_box_tbombs;	// value of IRED box
cvar_t	*sk_box_empnuke;	// value of EMP nuke
cvar_t	*sk_box_plasmashield;	// value of plasma shield

cvar_t	*sk_armor_bonus_value;		// value of armor shards
cvar_t	*sk_health_bonus_value;	// value of stimpacks
cvar_t	*sk_powerup_max;
cvar_t	*sk_quad_time;
cvar_t	*sk_inv_time;
cvar_t	*sk_breather_time;
cvar_t	*sk_enviro_time;
cvar_t	*sk_silencer_shots;
cvar_t	*sk_visor_time;
cvar_t	*sk_sniper_charge_time;
cvar_t	*sk_a2k_detonate_time;


void InitLithiumVars (void)
{
	mega_gibs = gi.cvar("mega_gibs", "0", 0);
	player_gib_health = gi.cvar("player_gib_health", "-40", 0);

	adjust_train_corners = gi.cvar("adjust_train_corners", "0", CVAR_ARCHIVE);

	falling_armor_damage = gi.cvar("falling_armor_damage", "0", CVAR_ARCHIVE);
	player_jump_sounds = gi.cvar("player_jump_sounds", "1", CVAR_ARCHIVE);

	use_vwep = gi.cvar("use_vwep", "1", CVAR_ARCHIVE);

	// weapon balancing
	sk_blaster_damage = gi.cvar("sk_blaster_damage", "10", 0);
	sk_blaster_damage_dm = gi.cvar("sk_blaster_damage_dm", "15", 0);
	sk_blaster_speed = gi.cvar("sk_blaster_speed", "1000", 0);
	sk_blaster_color = gi.cvar("sk_blaster_color", "1", 0);

	sk_shotgun_damage = gi.cvar("sk_shotgun_damage", "4", 0);
	sk_shotgun_count = gi.cvar("sk_shotgun_count", "12", 0);
	sk_shotgun_hspread = gi.cvar("sk_shotgun_hspread", "500", 0);
	sk_shotgun_vspread = gi.cvar("sk_shotgun_vspread", "500", 0);

	sk_sshotgun_damage = gi.cvar("sk_sshotgun_damage", "6", 0);
	sk_sshotgun_count = gi.cvar("sk_sshotgun_count", "20", 0);
	sk_sshotgun_hspread = gi.cvar("sk_sshotgun_hspread", "1000", 0);
	sk_sshotgun_vspread = gi.cvar("sk_sshotgun_vspread", "500", 0);

	sk_machinegun_damage = gi.cvar("sk_machinegun_damage", "8", 0);
	sk_machinegun_hspread = gi.cvar("sk_machinegun_hspread", "300", 0);
	sk_machinegun_vspread = gi.cvar("sk_machinegun_vspread", "500", 0);

	sk_chaingun_damage = gi.cvar("sk_chaingun_damage", "8", 0);
	sk_chaingun_damage_dm = gi.cvar("sk_chaingun_damage_dm", "6", 0);
	sk_chaingun_hspread = gi.cvar("sk_chaingun_hspread", "300", 0);
	sk_chaingun_vspread = gi.cvar("sk_chaingun_vspread", "500", 0);

	sk_grenade_damage = gi.cvar("sk_grenade_damage", "120", 0);
	sk_grenade_radius = gi.cvar("sk_grenade_radius", "160", 0);
	sk_grenade_speed = gi.cvar("sk_grenade_speed", "600", 0);

	sk_hand_grenade_damage = gi.cvar("sk_hand_grenade_damage", "125", 0);
	sk_hand_grenade_radius = gi.cvar("sk_hand_grenade_radius", "165", 0);

	sk_rocket_damage = gi.cvar("sk_rocket_damage", "100", 0);
	sk_rocket_damage2 = gi.cvar("sk_rocket_damage2", "20", 0);
	sk_rocket_rdamage = gi.cvar("sk_rocket_rdamage", "120", 0);
	sk_rocket_radius = gi.cvar("sk_rocket_radius", "140", 0);
	sk_rocket_speed = gi.cvar("sk_rocket_speed", "650", 0);

	sk_hyperblaster_damage = gi.cvar("sk_hyperblaster_damage", "10", 0);
	sk_hyperblaster_damage_dm = gi.cvar("sk_hyperblaster_damage_dm", "15", 0);
	sk_hyperblaster_speed = gi.cvar("sk_hyperblaster_speed", "1000", 0);
	sk_hyperblaster_color = gi.cvar("sk_hyperblaster_color", "1", 0);

	sk_railgun_damage = gi.cvar("sk_railgun_damage", "150", 0);
	sk_railgun_damage_dm = gi.cvar("sk_railgun_damage_dm", "100", 0);
	sk_railgun_skin = gi.cvar("sk_railgun_skin", "0", 0);
	sk_rail_color = gi.cvar("sk_rail_color", "1", 0);
	sk_rail_color_red = gi.cvar("sk_rail_color_red", "255", 0);
	sk_rail_color_green = gi.cvar("sk_rail_color_green", "20", 0);
	sk_rail_color_blue = gi.cvar("sk_rail_color_blue", "20", 0);

	sk_bfg_damage = gi.cvar("sk_bfg_damage", "500", 0);
	sk_bfg_damage_dm = gi.cvar("sk_bfg_damage_dm", "200", 0);
	sk_bfg_damage2 = gi.cvar("sk_bfg_damage2", "10", 0);
	sk_bfg_damage2_dm = gi.cvar("sk_bfg_damage2_dm", "5", 0);
	sk_bfg_rdamage = gi.cvar("sk_bfg_rdamage", "200", 0);
	sk_bfg_radius = gi.cvar("sk_bfg_radius", "1000", 0);
	sk_bfg_speed = gi.cvar("sk_bfg_speed", "400", 0);

	sk_flaregun_damage = gi.cvar("sk_flaregun_damage", "1", 0);
	sk_flaregun_rdamage = gi.cvar("sk_flaregun_rdamage", "1", 0);
	sk_flaregun_radius = gi.cvar("sk_flaregun_radius", "1", 0);
	sk_flaregun_speed = gi.cvar("sk_flaregun_speed", "600", 0);
	sk_flare_life = gi.cvar("sk_flare_life", "12", 0);

	sk_tbomb_damage = gi.cvar("sk_tbomb_damage", "150", 0);
	sk_tbomb_radius = gi.cvar("sk_tbomb_radius", "384", 0);
	sk_tbomb_shrapnel = gi.cvar("sk_tbomb_shrapnel", "5", 0);
	sk_tbomb_shrapnel_damage = gi.cvar("sk_tbomb_shrapnel_damage", "15", 0);

	sk_sniperrifle_damage = gi.cvar("sk_sniperrifle_damage", "250", 0);
	sk_sniperrifle_damage_dm = gi.cvar("sk_sniperrifle_damage_dm", "150", 0);
	sk_sniperrifle_kick = gi.cvar("sk_sniperrifle_kick", "400", 0);
	sk_sniperrifle_kick_dm = gi.cvar("sk_sniperrifle_kick_dm", "300", 0);

	sk_soniccannon_damage = gi.cvar("sk_soniccannon_damage", "10", 0);
	sk_soniccannon_damage2 = gi.cvar("sk_soniccannon_damage2", "990", 0);
	sk_soniccannon_radius = gi.cvar("sk_soniccannon_radius", "500", 0);
	sk_soniccannon_maxfiretime = gi.cvar("sk_soniccannon_maxfiretime", "5", 0);
	sk_soniccannon_maxcells = gi.cvar("sk_soniccannon_maxcells", "100", 0);

	sk_empnuke_radius = gi.cvar("sk_empnuke_radius", "1024", 0);

	sk_a2k_damage = gi.cvar("sk_a2k_damage", "2500", 0);
	sk_a2k_radius = gi.cvar("sk_a2k_radius", "512", 0);

	sk_plasmashield_health = gi.cvar("sk_plasmashield_health", "4000", 0);
	sk_plasmashield_life = gi.cvar("sk_plasmashield_life", "10", 0);

	// DM start values
	sk_dm_start_shells = gi.cvar("sk_dm_start_shells", "0", 0);
	sk_dm_start_bullets = gi.cvar("sk_dm_start_bullets", "0", 0);
	sk_dm_start_rockets = gi.cvar("sk_dm_start_rockets", "0", 0);
	sk_dm_start_grenades = gi.cvar("sk_dm_start_grenades", "0", 0);
	sk_dm_start_cells = gi.cvar("sk_dm_start_cells", "0", 0);
	sk_dm_start_slugs = gi.cvar("sk_dm_start_slugs", "0", 0);
	sk_dm_start_flares = gi.cvar("sk_dm_start_flares", "0", 0);
	sk_dm_start_tbombs = gi.cvar("sk_dm_start_tbombs", "0", 0);
	sk_dm_start_empnuke = gi.cvar("sk_dm_start_empnuke", "0", 0);
	sk_dm_start_plasmashield = gi.cvar("sk_dm_start_plasmashield", "0", 0);

	sk_dm_start_shotgun = gi.cvar("sk_dm_start_shotgun", "0", 0);
	sk_dm_start_sshotgun = gi.cvar("sk_dm_start_sshotgun", "0", 0);
	sk_dm_start_machinegun = gi.cvar("sk_dm_start_machinegun", "0", 0);
	sk_dm_start_chaingun = gi.cvar("sk_dm_start_chaingun", "0", 0);
	sk_dm_start_grenadelauncher = gi.cvar("sk_dm_start_grenadelauncher", "0", 0);
	sk_dm_start_rocketlauncher = gi.cvar("sk_dm_start_rocketlauncher", "0", 0);
	sk_dm_start_hyperblaster = gi.cvar("sk_dm_start_hyperblaster", "0", 0);
	sk_dm_start_railgun = gi.cvar("sk_dm_start_railgun", "0", 0);
	sk_dm_start_bfg = gi.cvar("sk_dm_start_bfg", "0", 0);
	sk_dm_start_flaregun = gi.cvar("sk_dm_start_flaregun", "0", 0);
	sk_dm_start_sniperrifle = gi.cvar("sk_dm_start_sniperrifle", "0", 0);
	sk_dm_start_soniccannon = gi.cvar("sk_dm_start_soniccannon", "0", 0);

	// maximum values
	sk_max_health = gi.cvar("sk_max_health", "100", 0);
	sk_max_health_dm = gi.cvar("sk_max_health_dm", "120", 0);
	sk_max_armor_jacket = gi.cvar("sk_max_armor_jacket", "50", 0);
	sk_max_armor_combat = gi.cvar("sk_max_armor_combat", "100", 0);
	sk_max_armor_body = gi.cvar("sk_max_armor_body", "200", 0);
	sk_max_bullets = gi.cvar("sk_max_bullets", "200", 0);
	sk_max_shells = gi.cvar("sk_max_shells", "100", 0);
	sk_max_rockets = gi.cvar("sk_max_rockets", "50", 0);
	sk_max_grenades = gi.cvar("sk_max_grenades", "50", 0);
	sk_max_cells = gi.cvar("sk_max_cells", "200", 0);
	sk_max_slugs = gi.cvar("sk_max_slugs", "50", 0);
	sk_max_flares = gi.cvar("sk_max_flares", "30", 0);
	sk_max_tbombs = gi.cvar("sk_max_tbombs", "30", 0);
	sk_max_a2k = gi.cvar("sk_max_a2k", "1", 0);
	sk_max_empnuke = gi.cvar("sk_max_empnuke", "50", 0);
	sk_max_plasmashield = gi.cvar("sk_max_plasmashield", "20", 0);

	// maximum settings if a player gets a bandolier
	sk_bando_bullets = gi.cvar("sk_bando_bullets", "250", 0);
	sk_bando_shells = gi.cvar("sk_bando_shells", "150", 0);
	sk_bando_cells = gi.cvar("sk_bando_cells", "250", 0);
	sk_bando_slugs = gi.cvar("sk_bando_slugs", "75", 0);
	sk_bando_flares = gi.cvar("sk_bando_flares", "45", 0);

	// maximum settings if a player gets a pack
	sk_pack_bullets = gi.cvar("sk_pack_bullets", "300", 0);
	sk_pack_shells = gi.cvar("sk_pack_shells", "200", 0);
	sk_pack_rockets = gi.cvar("sk_pack_rockets", "100", 0);
	sk_pack_grenades = gi.cvar("sk_pack_grenades", "100", 0);
	sk_pack_cells = gi.cvar("sk_pack_cells", "300", 0);
	sk_pack_slugs = gi.cvar("sk_pack_slugs", "100", 0);
	sk_pack_flares = gi.cvar("sk_pack_flares", "60", 0);
	sk_pack_tbombs = gi.cvar("sk_pack_tbombs", "100", 0);
	sk_pack_a2k = gi.cvar("sk_pack_a2k", "1", 0);
	sk_pack_empnuke = gi.cvar("sk_pack_empnuke", "100", 0);
	sk_pack_plasmashield = gi.cvar("sk_pack_plasmashield", "40", 0);

	// pickup values
	sk_box_shells = gi.cvar("sk_box_shells", "10", 0);
	sk_box_bullets = gi.cvar("sk_box_bullets", "50", 0);
	sk_box_grenades = gi.cvar("sk_box_grenades", "5", 0);
	sk_box_rockets = gi.cvar("sk_box_rockets", "5", 0);
	sk_box_cells = gi.cvar("sk_box_cells", "50", 0);
	sk_box_slugs = gi.cvar("sk_box_slugs", "10", 0);
	sk_box_flares = gi.cvar("sk_box_flares", "3", 0);
	sk_box_tbombs = gi.cvar("sk_box_tbombs", "3", 0);
	sk_box_empnuke = gi.cvar("sk_box_empnuke", "1", 0);
	sk_box_plasmashield = gi.cvar("sk_box_plasmashield", "5", 0);

	// items/powerups
	sk_armor_bonus_value = gi.cvar("sk_armor_bonus_value", "2", 0);
	sk_health_bonus_value = gi.cvar("sk_health_bonus_value", "2", 0);
	sk_powerup_max = gi.cvar("sk_powerup_max", "2", 0);
	sk_quad_time = gi.cvar("sk_quad_time", "30", 0);
	sk_inv_time = gi.cvar("sk_inv_time", "30", 0);
	sk_breather_time = gi.cvar("sk_breather_time", "30", 0);
	sk_enviro_time = gi.cvar("sk_enviro_time", "30", 0);
	sk_silencer_shots = gi.cvar("sk_silencer_shots", "30", 0);
	sk_visor_time = gi.cvar("sk_visor_time", "30", 0);
	sk_sniper_charge_time = gi.cvar("sk_sniper_charge_time", "3", 0);
	sk_a2k_detonate_time = gi.cvar("sk_a2k_detonate_time", "5", 0);
}
