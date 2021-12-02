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
extern	cvar_t	*allow_player_use_abandoned_turret; //whether to allow player to use turrets in exisiting maps
extern	cvar_t	*adjust_train_corners; //whether to subtract (1,1,1) from train path corners to fix misalignments

extern	cvar_t	*add_velocity_throw;	//whether to add player's velocity to thrown objects

extern	cvar_t	*falling_armor_damage; //whether player's armor absorbs damage from falling
extern	cvar_t	*player_jump_sounds; //whether to play that STUPID grunting sound when the player jumps

// Server-side speed control stuff
extern	cvar_t	*player_max_speed;
extern	cvar_t	*player_crouch_speed;
extern	cvar_t	*player_accel;
extern	cvar_t	*player_stopspeed;

extern	cvar_t	*use_vwep;

// weapon balancing
extern	cvar_t	*blaster_damage;
extern	cvar_t	*blaster_damage_dm;
extern	cvar_t	*blaster_speed;
extern	cvar_t	*blaster_color;

extern	cvar_t	*shotgun_damage;
extern	cvar_t	*shotgun_count;
extern	cvar_t	*shotgun_hspread;
extern	cvar_t	*shotgun_vspread;

extern	cvar_t	*sshotgun_damage;
extern	cvar_t	*sshotgun_count;
extern	cvar_t	*sshotgun_hspread;
extern	cvar_t	*sshotgun_vspread;

extern	cvar_t	*machinegun_damage;
extern	cvar_t	*machinegun_hspread;
extern	cvar_t	*machinegun_vspread;

extern	cvar_t	*chaingun_damage;
extern	cvar_t	*chaingun_damage_dm;
extern	cvar_t	*chaingun_hspread;
extern	cvar_t	*chaingun_vspread;

extern	cvar_t	*grenade_damage;
extern	cvar_t	*grenade_radius;
extern	cvar_t	*grenade_speed;

extern	cvar_t	*hand_grenade_damage;
extern	cvar_t	*hand_grenade_radius;

extern	cvar_t	*rocket_damage;
extern	cvar_t	*rocket_damage2;
extern	cvar_t	*rocket_rdamage;
extern	cvar_t	*rocket_radius;
extern	cvar_t	*rocket_speed;

extern	cvar_t	*hyperblaster_damage;
extern	cvar_t	*hyperblaster_damage_dm;
extern	cvar_t	*hyperblaster_speed;
extern	cvar_t	*hyperblaster_color;

extern	cvar_t	*railgun_damage;
extern	cvar_t	*railgun_damage_dm;
extern	cvar_t	*rail_color;

extern	cvar_t	*bfg_damage;
extern	cvar_t	*bfg_damage_dm;
extern	cvar_t	*bfg_damage2;
extern	cvar_t	*bfg_rdamage;
extern	cvar_t	*bfg_radius;
extern	cvar_t	*bfg_speed;

extern	cvar_t	*jump_kick_damage;

// DM start values
extern	cvar_t	*dm_start_shells;
extern	cvar_t	*dm_start_bullets;
extern	cvar_t	*dm_start_rockets;
extern	cvar_t	*dm_start_homing;
extern	cvar_t	*dm_start_grenades;
extern	cvar_t	*dm_start_cells;
extern	cvar_t	*dm_start_slugs;

extern	cvar_t	*dm_start_shotgun;
extern	cvar_t	*dm_start_sshotgun;
extern	cvar_t	*dm_start_machinegun;
extern	cvar_t	*dm_start_chaingun;
extern	cvar_t	*dm_start_grenadelauncher;
extern	cvar_t	*dm_start_rocketlauncher;
extern	cvar_t	*dm_start_hyperblaster;
extern	cvar_t	*dm_start_railgun;
extern	cvar_t	*dm_start_bfg;

// maximum values
extern	cvar_t	*max_health;
extern	cvar_t	*max_health_dm;
extern	cvar_t	*max_armor;
extern	cvar_t	*max_bullets;
extern	cvar_t	*max_shells;
extern	cvar_t	*max_rockets;
extern	cvar_t	*max_grenades;
extern	cvar_t	*max_cells;
extern	cvar_t	*max_slugs;
extern	cvar_t	*max_fuel;

// maximum settings if a player gets a bandolier
extern	cvar_t	*bando_bullets;
extern	cvar_t	*bando_shells;
extern	cvar_t	*bando_cells;
extern	cvar_t	*bando_slugs;
extern	cvar_t	*bando_fuel;

// maximum settings if a player gets a pack
extern	cvar_t	*pack_bullets;
extern	cvar_t	*pack_shells;
extern	cvar_t	*pack_rockets;
extern	cvar_t	*pack_grenades;
extern	cvar_t	*pack_cells;
extern	cvar_t	*pack_slugs;
extern	cvar_t	*pack_fuel;

extern	cvar_t	*box_shells; //value of shells
extern	cvar_t	*box_bullets; //value of bullets
extern	cvar_t	*box_grenades; //value of grenade pack
extern	cvar_t	*box_rockets; //value of rocket pack
extern	cvar_t	*box_cells; //value of cell pack
extern	cvar_t	*box_slugs; //value of slug box
extern	cvar_t	*box_fuel; //value of fuel

extern cvar_t	*armor_bonus_value; //value of armor shards
extern cvar_t	*health_bonus_value; //value of stimpacks
extern cvar_t	*powerup_max;
extern cvar_t	*quad_time;
extern cvar_t	*inv_time;
extern cvar_t	*breather_time;
extern cvar_t	*enviro_time;
extern cvar_t	*silencer_shots;
extern cvar_t	*stasis_time;

// CTF stuff
extern	cvar_t	*use_techs;          // enables techs
extern	cvar_t	*use_coloredtechs;   // enable colored techs, otherwise plain CTF Techs
extern	cvar_t	*use_lithiumtechs;   // enable lithium style colored runes, otherwise plain CTF Techs

extern	cvar_t	*ctf_blastercolors;   // enable different blaster colors for each team

extern	cvar_t	*allow_flagdrop;
extern	cvar_t	*allow_flagpickup;
extern	cvar_t	*allow_techdrop;
extern	cvar_t	*allow_techpickup;

extern	cvar_t	*tech_flags;         // determines which techs will show in the game, add these:
//                                   1 = resist, 2 = strength, 4 = haste, 8 = regen, 16 = vampire, 32 = ammogen
extern	cvar_t	*tech_spawn;         // chance a rune will spawn from another item respawning
extern	cvar_t	*tech_perplayer;     // sets techs per player that will appear in map
extern	cvar_t	*tech_life;          // seconds a rune will stay around before disappearing
extern	cvar_t	*tech_min;           // sets minimum number of techs to be in the game
extern	cvar_t	*tech_max;           // sets maximum number of techs to be in the game

extern	cvar_t	*tech_haste;		// what should I use this for?
extern	cvar_t	*tech_resist;        // sets how much damage is divided by with resist rune
extern	cvar_t	*tech_strength;      // sets how much damage is multiplied by with strength rune
extern	cvar_t	*tech_regen;         // sets how fast health is gained back
extern	cvar_t	*tech_regen_armor;
extern	cvar_t	*tech_regen_health_max;      // sets maximum health that can be gained from regen rune
extern	cvar_t	*tech_regen_armor_max;      // sets maximum armor that can be gained from regen rune
extern	cvar_t	*tech_regen_armor_always;      // sets whether armor should be regened regardless of if currently held
extern	cvar_t	*tech_vampire;       // sets percentage of health gained from damage inflicted
extern	cvar_t	*tech_vampiremax;    // sets maximum health that can be gained from vampire rune
// end CTF stuff
