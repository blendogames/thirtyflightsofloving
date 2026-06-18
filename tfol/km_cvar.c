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
cvar_t	*allow_player_use_abandoned_turret; //whether to allow player to use turrets in exisiting maps
cvar_t	*adjust_train_corners; //whether to subtract (1,1,1) from train path corners to fix misalignments

cvar_t	*add_velocity_throw;	//whether to add player's velocity to thrown objects

cvar_t	*falling_armor_damage; //whether player's armor absorbs damage from falling
cvar_t	*player_jump_sounds; //whether to play that STUPID grunting sound when the player jumps

// Server-side speed control stuff
cvar_t	*player_max_speed;
cvar_t	*player_crouch_speed;
cvar_t	*player_accel;
cvar_t	*player_stopspeed;

cvar_t	*use_vwep;

// weapon balancing
cvar_t	*blaster_damage;
cvar_t	*blaster_damage_dm;
cvar_t	*blaster_speed;
extern cvar_t	*blaster_color;  //blaster color- 1=yellow, 2=green, 3=blue, 4=red

cvar_t	*shotgun_damage;
cvar_t	*shotgun_count;
cvar_t	*shotgun_hspread;
cvar_t	*shotgun_vspread;

cvar_t	*sshotgun_damage;
cvar_t	*sshotgun_count;
cvar_t	*sshotgun_hspread;
cvar_t	*sshotgun_vspread;

cvar_t	*machinegun_damage;
cvar_t	*machinegun_hspread;
cvar_t	*machinegun_vspread;

cvar_t	*chaingun_damage;
cvar_t	*chaingun_damage_dm;
cvar_t	*chaingun_hspread;
cvar_t	*chaingun_vspread;

cvar_t	*grenade_damage;
cvar_t	*grenade_radius;
cvar_t	*grenade_speed;

cvar_t	*hand_grenade_damage;
cvar_t	*hand_grenade_radius;

cvar_t	*rocket_damage;
cvar_t	*rocket_damage2;
cvar_t	*rocket_rdamage;
cvar_t	*rocket_radius;
cvar_t	*rocket_speed;

cvar_t	*hyperblaster_damage;
cvar_t	*hyperblaster_damage_dm;
cvar_t	*hyperblaster_speed;
cvar_t	*hyperblaster_color;  //hyperblaster color- 1=yellow, 2=green, 3=blue, 4=red

cvar_t	*railgun_damage;
cvar_t	*railgun_damage_dm;
cvar_t	*rail_color;

cvar_t	*bfg_damage;
cvar_t	*bfg_damage_dm;
cvar_t	*bfg_damage2;
cvar_t	*bfg_rdamage;
cvar_t	*bfg_radius;
cvar_t	*bfg_speed;

cvar_t	*jump_kick_damage;

// DM start values
cvar_t	*dm_start_shells;
cvar_t	*dm_start_bullets;
cvar_t	*dm_start_rockets;
cvar_t	*dm_start_homing;
cvar_t	*dm_start_grenades;
cvar_t	*dm_start_cells;
cvar_t	*dm_start_slugs;

cvar_t	*dm_start_shotgun;
cvar_t	*dm_start_sshotgun;
cvar_t	*dm_start_machinegun;
cvar_t	*dm_start_chaingun;
cvar_t	*dm_start_grenadelauncher;
cvar_t	*dm_start_rocketlauncher;
cvar_t	*dm_start_hyperblaster;
cvar_t	*dm_start_railgun;
cvar_t	*dm_start_bfg;

// maximum values
cvar_t	*max_health;
cvar_t	*max_health_dm;
cvar_t	*max_armor;	
cvar_t	*max_bullets;	
cvar_t	*max_shells;	
cvar_t	*max_rockets;	
cvar_t	*max_grenades;	
cvar_t	*max_cells;		
cvar_t	*max_slugs;
cvar_t	*max_fuel;

// maximum settings if a player gets a bandolier
cvar_t	*bando_bullets;  // 300
cvar_t	*bando_shells;   // 200
cvar_t	*bando_cells;    // 300
cvar_t	*bando_slugs;    // 100
cvar_t	*bando_fuel;

// maximum settings if a player gets a pack
cvar_t	*pack_health;
cvar_t	*pack_armor;
cvar_t	*pack_bullets;  // 300
cvar_t	*pack_shells;   // 200
cvar_t	*pack_rockets;  // 100
cvar_t	*pack_grenades; // 100
cvar_t	*pack_cells;    // 300
cvar_t	*pack_slugs;    // 100
cvar_t	*pack_fuel;

cvar_t	*box_shells; //value of shells
cvar_t	*box_bullets; //value of bullets
cvar_t	*box_grenades; //value of grenade pack
cvar_t	*box_rockets; //value of rocket pack
cvar_t	*box_cells; //value of cell pack
cvar_t	*box_slugs; //value of slug box
cvar_t	*box_fuel; //value of fuel

cvar_t	*armor_bonus_value; //value of armor shards
cvar_t	*health_bonus_value; //value of stimpacks
cvar_t	*powerup_max;
cvar_t	*nuke_max;
cvar_t	*nbomb_max;
cvar_t	*quad_time;
cvar_t	*inv_time;
cvar_t	*breather_time;
cvar_t	*enviro_time;
cvar_t	*silencer_shots;
cvar_t	*stasis_time;

// CTF stuff
cvar_t	*use_techs;          // enables techs
cvar_t	*use_coloredtechs;   // enable colored techs, otherwise plain CTF Techs
cvar_t	*use_lithiumtechs;   // enable lithium style colored runes, otherwise plain CTF Techs

cvar_t	*ctf_blastercolors;   // enable different blaster colors for each team

cvar_t	*allow_flagdrop;
cvar_t	*allow_flagpickup;
cvar_t	*allow_techdrop;
cvar_t	*allow_techpickup;

cvar_t	*tech_flags;         // determines which techs will show in the game, add these:
//                                   1 = resist, 2 = strength, 4 = haste, 8 = regen, 16 = vampire, 32 = ammogen
cvar_t	*tech_spawn;         // chance a rune will spawn from another item respawning
cvar_t	*tech_perplayer;     // sets techs per player that will appear in map
cvar_t	*tech_life;          // seconds a rune will stay around before disappearing
cvar_t	*tech_min;           // sets minimum number of techs to be in the game
cvar_t	*tech_max;           // sets maximum number of techs to be in the game

cvar_t	*tech_haste; // what should I use this for?
cvar_t	*tech_resist;        // sets how much damage is divided by with resist rune
cvar_t	*tech_strength;      // sets how much damage is multiplied by with strength rune
cvar_t	*tech_regen;         // sets how fast health is gained back
cvar_t	*tech_regen_armor;
cvar_t	*tech_regen_health_max;      // sets maximum health that can be gained from regen rune
cvar_t	*tech_regen_armor_max;      // sets maximum armor that can be gained from regen rune
cvar_t	*tech_regen_armor_always;      // sets whether armor should be regened regardless of if currently held
cvar_t	*tech_vampire;       // sets percentage of health gained from damage inflicted
cvar_t	*tech_vampiremax;    // sets maximum health that can be gained from vampire rune
// end CTF stuff


void lithium_defaults(void)
{
	mega_gibs = gi.cvar("mega_gibs", "0", 0);
	player_gib_health = gi.cvar("player_gib_health", "-40", 0);

	allow_player_use_abandoned_turret = gi.cvar("allow_player_use_abandoned_turret", "0", CVAR_ARCHIVE);
	adjust_train_corners = gi.cvar("adjust_train_corners", "0", CVAR_ARCHIVE);

	add_velocity_throw = gi.cvar("add_velocity_throw", "0", CVAR_ARCHIVE);

	falling_armor_damage = gi.cvar("falling_armor_damage", "1", CVAR_ARCHIVE);
	player_jump_sounds = gi.cvar("player_jump_sounds", "1", CVAR_ARCHIVE);

	player_max_speed = gi.cvar("player_max_speed", "300", 0);
	player_crouch_speed = gi.cvar("player_crouch_speed", "100", 0); 
	player_accel = gi.cvar("player_accel", "10", 0);
	player_stopspeed = gi.cvar("player_stopspeed", "100", 0);

	use_vwep = gi.cvar("use_vwep", "1", CVAR_ARCHIVE);

	max_health = gi.cvar("max_health", "100", 0);
	max_health_dm = gi.cvar("max_health_dm", "120", 0);
	max_armor = gi.cvar("max_armor", "200", 0);
	max_bullets = gi.cvar("max_bullets", "200", 0);
	max_shells = gi.cvar("max_shells", "100", 0);
	max_rockets = gi.cvar("max_rockets", "50", 0);
	max_grenades = gi.cvar("max_grenades", "50", 0);
	max_cells = gi.cvar("max_cells", "200", 0);
	max_slugs = gi.cvar("max_slugs", "50", 0);
	max_fuel = gi.cvar("max_fuel", "1000", 0);

	bando_bullets = gi.cvar("bando_bullets", "250", 0);
	bando_shells = gi.cvar("bando_shells", "150", 0);
	bando_cells = gi.cvar("bando_cells", "250", 0);
	bando_slugs = gi.cvar("bando_slugs", "75", 0);
	bando_fuel = gi.cvar("bando_fuel", "1500", 0);

	pack_bullets = gi.cvar("pack_bullets", "300", 0);
	pack_shells = gi.cvar("pack_shells", "200", 0);
	pack_rockets = gi.cvar("pack_rockets", "100", 0);
	pack_grenades = gi.cvar("pack_grenades", "100", 0);
	pack_cells = gi.cvar("pack_cells", "300", 0);
	pack_slugs = gi.cvar("pack_slugs", "100", 0);
	pack_fuel = gi.cvar("pack_fuel", "2000", 0);

	box_shells = gi.cvar("box_shells", "10", 0);
	box_bullets = gi.cvar("box_bullets", "50", 0);
	box_grenades = gi.cvar("box_grenades", "5", 0);
	box_rockets = gi.cvar("box_rockets", "5", 0);
	box_cells = gi.cvar("box_cells", "50", 0);
	box_slugs = gi.cvar("box_slugs", "10", 0);
	box_fuel = gi.cvar("box_fuel", "500", 0);

	armor_bonus_value = gi.cvar("armor_bonus_value", "2", 0);
	health_bonus_value = gi.cvar("health_bonus_value", "2", 0);
	powerup_max = gi.cvar("powerup_max", "2", 0);
	quad_time = gi.cvar("quad_time", "30", 0);
	inv_time = gi.cvar("inv_time", "30", 0);
	breather_time = gi.cvar("breather_time", "30", 0);
	enviro_time = gi.cvar("enviro_time", "30", 0);
	silencer_shots = gi.cvar("silencer_shots", "30", 0);
	stasis_time = gi.cvar("stasis_time", "30", 0);

	blaster_damage = gi.cvar("blaster_damage", "10", 0);
	blaster_damage_dm = gi.cvar("blaster_damage_dm", "15", 0);
	blaster_speed = gi.cvar("blaster_speed", "1000", 0);
	blaster_color = gi.cvar("blaster_color", "1", 0);

	shotgun_damage = gi.cvar("shotgun_damage", "6", 0);
	shotgun_count = gi.cvar("shotgun_count", "12", 0);
	shotgun_hspread = gi.cvar("shotgun_hspread", "500", 0);
	shotgun_vspread = gi.cvar("shotgun_vspread", "500", 0);

	sshotgun_damage = gi.cvar("sshotgun_damage", "6", 0);
	sshotgun_count = gi.cvar("sshotgun_count", "22", 0);
	sshotgun_hspread = gi.cvar("sshotgun_hspread", "1000", 0);
	sshotgun_vspread = gi.cvar("sshotgun_vspread", "500", 0);

	machinegun_damage = gi.cvar("machinegun_damage", "8", 0);
	machinegun_hspread = gi.cvar("machinegun_hspread", "300", 0);
	machinegun_vspread = gi.cvar("machinegun_vspread", "500", 0);

	chaingun_damage = gi.cvar("chaingun_damage", "8", 0);
	chaingun_damage_dm = gi.cvar("chaingun_damage_dm", "6", 0);
	chaingun_hspread = gi.cvar("chaingun_hspread", "300", 0);
	chaingun_vspread = gi.cvar("chaingun_vspread", "500", 0);

	grenade_damage = gi.cvar("grenade_damage", "120", 0);
	grenade_radius = gi.cvar("grenade_radius", "160", 0);
	grenade_speed = gi.cvar("grenade_speed", "600", 0);

	hand_grenade_damage = gi.cvar("hand_grenade_damage", "125", 0);
	hand_grenade_radius = gi.cvar("hand_grenade_radius", "165", 0);
	
	rocket_damage = gi.cvar("rocket_damage", "100", 0);
	rocket_damage2 = gi.cvar("rocket_damage2", "20", 0);
	rocket_rdamage = gi.cvar("rocket_rdamage", "120", 0);
	rocket_radius = gi.cvar("rocket_radius", "140", 0);
	rocket_speed = gi.cvar("rocket_speed", "650", 0);

	hyperblaster_damage = gi.cvar("hyperblaster_damage", "10", 0);
	hyperblaster_damage_dm = gi.cvar("hyperblaster_damage_dm", "15", 0);
	hyperblaster_speed = gi.cvar("hyperblaster_speed", "1000", 0);
	hyperblaster_color = gi.cvar("hyperblaster_color", "1", 0);

	railgun_damage = gi.cvar("railgun_damage", "150", 0);
	railgun_damage_dm = gi.cvar("railgun_damage_dm", "100", 0);
	rail_color = gi.cvar("rail_color", "1", 0);

	bfg_damage = gi.cvar("bfg_damage", "500", 0);
	bfg_damage_dm = gi.cvar("bfg_damage_dm", "200", 0);
	bfg_damage2 = gi.cvar("bfg_damage2", "10", 0);
	bfg_rdamage = gi.cvar("bfg_rdamage", "200", 0);
	bfg_radius = gi.cvar("bfg_radius", "1000", 0);
	bfg_speed = gi.cvar("bfg_speed", "400", 0);

	jump_kick_damage = gi.cvar("jump_kick_damage", "10", 0);

	dm_start_shells = gi.cvar("dm_start_shells", "0", 0);
	dm_start_bullets = gi.cvar("dm_start_bullets", "0", 0);
	dm_start_rockets = gi.cvar("dm_start_rockets", "0", 0);
	dm_start_homing = gi.cvar("dm_start_homing", "0", 0);
	dm_start_grenades = gi.cvar("dm_start_grenades", "0", 0);
	dm_start_cells = gi.cvar("dm_start_cells", "0", 0);
	dm_start_slugs = gi.cvar("dm_start_slugs", "0", 0);

	dm_start_shotgun = gi.cvar("dm_start_shotgun", "0", 0);
	dm_start_sshotgun = gi.cvar("dm_start_sshotgun", "0", 0);
	dm_start_machinegun = gi.cvar("dm_start_machinegun", "0", 0);
	dm_start_chaingun = gi.cvar("dm_start_chaingun", "0", 0);
	dm_start_grenadelauncher = gi.cvar("dm_start_grenadelauncher", "0", 0);
	dm_start_rocketlauncher = gi.cvar("dm_start_rocketlauncher", "0", 0);
	dm_start_hyperblaster = gi.cvar("dm_start_hyperblaster", "0", 0);
	dm_start_railgun = gi.cvar("dm_start_railgun", "0", 0);
	dm_start_bfg = gi.cvar("dm_start_bfg", "0", 0);

	// CTF stuff
	use_techs = gi.cvar("use_techs", "1", 0);
	use_coloredtechs = gi.cvar("use_coloredtechs", "0", 0);
	use_lithiumtechs = gi.cvar("use_lithiumtechs", "0", 0);

	ctf_blastercolors = gi.cvar("ctf_blastercolors", "1", 0);

	allow_flagdrop = gi.cvar("allow_flagdrop", "1", 0);
	allow_flagpickup = gi.cvar("allow_flagpickup", "1", 0);
	allow_techdrop = gi.cvar("allow_techdrop", "1", 0);
	allow_techpickup = gi.cvar("allow_techpickup", "1", 0);

	tech_flags = gi.cvar("tech_flags", "15", 0);
	tech_spawn = gi.cvar("tech_spawn", "0.10", 0);
	tech_perplayer = gi.cvar("tech_perplayer", "0.7", 0);
	tech_life = gi.cvar("tech_life", "20", 0);
	tech_min = gi.cvar("tech_min", "2", 0);
	tech_max = gi.cvar("tech_max", "10", 0);

	tech_haste = gi.cvar("tech_haste", "", 0);
	tech_resist = gi.cvar("tech_resist", "2.0", 0);
	tech_strength = gi.cvar("tech_strength", "2.0", 0);
	tech_regen = gi.cvar("tech_regen", "0.25", 0);
	tech_regen_armor = gi.cvar("tech_regen_armor", "1", 0);
	tech_regen_health_max = gi.cvar("tech_regen_health_max", "150", 0);
	tech_regen_armor_max = gi.cvar("tech_regen_armor_max", "150", 0);
	tech_regen_armor_always = gi.cvar("tech_regen_armor_always", "0", 0);
	tech_vampire = gi.cvar("tech_vampire", "0.5", 0);
	tech_vampiremax = gi.cvar("tech_vampiremax", "200", 0);
	// end CTF Tech stuff
}
