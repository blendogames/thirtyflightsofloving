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

/* Knightmare's cvar header file */

extern	cvar_t	*mega_gibs;			 // whether to spawn extra gibs, default to 0
extern	cvar_t	*player_gib_health;  // what health level to gib players at
extern	cvar_t	*adjust_train_corners; //whether to subtract (1,1,1) from train path corners to fix misalignments

extern	cvar_t	*falling_armor_damage; //whether player's armor absorbs damage from falling
extern	cvar_t	*player_jump_sounds; //whether to play that STUPID grunting sound when the player jumps

extern	cvar_t	*use_vwep;

// weapon balancing
extern	cvar_t	*sk_blaster_damage;
extern	cvar_t	*sk_blaster_damage_dm;
extern	cvar_t	*sk_blaster_speed;
extern	cvar_t	*sk_blaster_color;

extern	cvar_t	*sk_shotgun_damage;
extern	cvar_t	*sk_shotgun_count;
extern	cvar_t	*sk_shotgun_hspread;
extern	cvar_t	*sk_shotgun_vspread;

extern	cvar_t	*sk_sshotgun_damage;
extern	cvar_t	*sk_sshotgun_count;
extern	cvar_t	*sk_sshotgun_hspread;
extern	cvar_t	*sk_sshotgun_vspread;

extern	cvar_t	*sk_machinegun_damage;
extern	cvar_t	*sk_machinegun_hspread;
extern	cvar_t	*sk_machinegun_vspread;

extern	cvar_t	*sk_chaingun_damage;
extern	cvar_t	*sk_chaingun_damage_dm;
extern	cvar_t	*sk_chaingun_hspread;
extern	cvar_t	*sk_chaingun_vspread;

extern	cvar_t	*sk_grenade_damage;
extern	cvar_t	*sk_grenade_radius;
extern	cvar_t	*sk_grenade_speed;

extern	cvar_t	*sk_hand_grenade_damage;
extern	cvar_t	*sk_hand_grenade_radius;

extern	cvar_t	*sk_rocket_damage;
extern	cvar_t	*sk_rocket_damage2;
extern	cvar_t	*sk_rocket_rdamage;
extern	cvar_t	*sk_rocket_radius;
extern	cvar_t	*sk_rocket_speed;

extern	cvar_t	*sk_hyperblaster_damage;
extern	cvar_t	*sk_hyperblaster_damage_dm;
extern	cvar_t	*sk_hyperblaster_speed;
extern	cvar_t	*sk_hyperblaster_color;

extern	cvar_t	*sk_railgun_damage;
extern	cvar_t	*sk_railgun_damage_dm;
extern	cvar_t	*sk_railgun_skin;

extern	cvar_t	*sk_bfg_damage;
extern	cvar_t	*sk_bfg_damage_dm;
extern	cvar_t	*sk_bfg_damage2;
extern	cvar_t	*sk_bfg_damage2_dm;
extern	cvar_t	*sk_bfg_rdamage;
extern	cvar_t	*sk_bfg_radius;
extern	cvar_t	*sk_bfg_speed;

extern	cvar_t	*sk_flaregun_damage;
extern	cvar_t	*sk_flaregun_rdamage;
extern	cvar_t	*sk_flaregun_radius;
extern	cvar_t	*sk_flaregun_speed;
extern	cvar_t	*sk_flare_life;

extern	cvar_t	*sk_tbomb_damage;
extern	cvar_t	*sk_tbomb_radius;
extern	cvar_t	*sk_tbomb_shrapnel;
extern	cvar_t	*sk_tbomb_shrapnel_damage;

extern	cvar_t	*sk_sniperrifle_damage;
extern	cvar_t	*sk_sniperrifle_damage_dm;
extern	cvar_t	*sk_sniperrifle_kick;
extern	cvar_t	*sk_sniperrifle_kick_dm;

extern	cvar_t	*sk_soniccannon_damage;
extern	cvar_t	*sk_soniccannon_damage2;
extern	cvar_t	*sk_soniccannon_radius;
extern	cvar_t	*sk_soniccannon_maxfiretime;
extern	cvar_t	*sk_soniccannon_maxcells;

extern	cvar_t	*sk_empnuke_radius;

extern	cvar_t	*sk_a2k_damage;
extern	cvar_t	*sk_a2k_radius;

extern	cvar_t	*sk_plasmashield_health;
extern	cvar_t	*sk_plasmashield_life;

// DM start values
extern	cvar_t	*sk_dm_start_shells;
extern	cvar_t	*sk_dm_start_bullets;
extern	cvar_t	*sk_dm_start_rockets;
extern	cvar_t	*sk_dm_start_grenades;
extern	cvar_t	*sk_dm_start_cells;
extern	cvar_t	*sk_dm_start_slugs;
extern	cvar_t	*sk_dm_start_flares;
extern	cvar_t	*sk_dm_start_tbombs;
extern	cvar_t	*sk_dm_start_empnuke;
extern	cvar_t	*sk_dm_start_plasmashield;

extern	cvar_t	*sk_dm_start_shotgun;
extern	cvar_t	*sk_dm_start_sshotgun;
extern	cvar_t	*sk_dm_start_machinegun;
extern	cvar_t	*sk_dm_start_chaingun;
extern	cvar_t	*sk_dm_start_grenadelauncher;
extern	cvar_t	*sk_dm_start_rocketlauncher;
extern	cvar_t	*sk_dm_start_hyperblaster;
extern	cvar_t	*sk_dm_start_railgun;
extern	cvar_t	*sk_dm_start_bfg;
extern	cvar_t	*sk_dm_start_flaregun;
extern	cvar_t	*sk_dm_start_sniperrifle;
extern	cvar_t	*sk_dm_start_soniccannon;

// maximum values
extern	cvar_t	*sk_max_health;
extern	cvar_t	*sk_max_health_dm;
extern	cvar_t	*sk_max_armor;
extern	cvar_t	*sk_max_armor_jacket;
extern	cvar_t	*sk_max_armor_combat;
extern	cvar_t	*sk_max_armor_body;
extern	cvar_t	*sk_max_bullets;
extern	cvar_t	*sk_max_shells;
extern	cvar_t	*sk_max_rockets;
extern	cvar_t	*sk_max_grenades;
extern	cvar_t	*sk_max_cells;
extern	cvar_t	*sk_max_slugs;
extern	cvar_t	*sk_max_flares;
extern	cvar_t	*sk_max_tbombs;
extern	cvar_t	*sk_max_a2k;
extern	cvar_t	*sk_max_empnuke;
extern	cvar_t	*sk_max_plasmashield;

// maximum settings if a player gets a bandolier
extern	cvar_t	*sk_bando_bullets;
extern	cvar_t	*sk_bando_shells;
extern	cvar_t	*sk_bando_cells;
extern	cvar_t	*sk_bando_slugs;
extern	cvar_t	*sk_bando_flares;

// maximum settings if a player gets a pack
extern	cvar_t	*sk_pack_bullets;
extern	cvar_t	*sk_pack_shells;
extern	cvar_t	*sk_pack_rockets;
extern	cvar_t	*sk_pack_grenades;
extern	cvar_t	*sk_pack_cells;
extern	cvar_t	*sk_pack_slugs;
extern	cvar_t	*sk_pack_flares;
extern	cvar_t	*sk_pack_tbombs;
extern	cvar_t	*sk_pack_a2k;
extern	cvar_t	*sk_pack_empnuke;
extern	cvar_t	*sk_pack_plasmashield;

// pickup values
extern	cvar_t	*sk_box_shells; //value of shells
extern	cvar_t	*sk_box_bullets; //value of bullets
extern	cvar_t	*sk_box_grenades; //value of grenade pack
extern	cvar_t	*sk_box_rockets; //value of rocket pack
extern	cvar_t	*sk_box_cells; //value of cell pack
extern	cvar_t	*sk_box_slugs; //value of slug box
extern	cvar_t	*sk_box_flares;
extern	cvar_t	*sk_box_tbombs;
extern	cvar_t	*sk_box_empnuke;
extern	cvar_t	*sk_box_plasmashield;

// items/powerups
extern	cvar_t	*sk_armor_bonus_value; //value of armor shards
extern	cvar_t	*sk_health_bonus_value; //value of stimpacks
extern	cvar_t	*sk_powerup_max;
extern	cvar_t	*sk_quad_time;
extern	cvar_t	*sk_inv_time;
extern	cvar_t	*sk_breather_time;
extern	cvar_t	*sk_enviro_time;
extern	cvar_t	*sk_silencer_shots;
extern	cvar_t	*sk_visor_time;
extern	cvar_t	*sk_sniper_charge_time;
extern	cvar_t	*sk_a2k_detonate_time;

