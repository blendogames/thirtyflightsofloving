// g_save.c

#include "g_local.h"

field_t fields[] = {
	{"classname", FOFS(classname), F_LSTRING},
	{"origin", FOFS(s.origin), F_VECTOR},
	{"model", FOFS(model), F_LSTRING},
	{"spawnflags", FOFS(spawnflags), F_INT},
	{"speed", FOFS(speed), F_FLOAT},
	{"accel", FOFS(accel), F_FLOAT},
	{"decel", FOFS(decel), F_FLOAT},
	{"target", FOFS(target), F_LSTRING},
	{"targetname", FOFS(targetname), F_LSTRING},
	{"pathtarget", FOFS(pathtarget), F_LSTRING},
	{"deathtarget", FOFS(deathtarget), F_LSTRING},
	{"killtarget", FOFS(killtarget), F_LSTRING},
	{"combattarget", FOFS(combattarget), F_LSTRING},
//DH++
	{"followtarget", FOFS(followtarget), F_LSTRING},
	{"destroytarget", FOFS(destroytarget), F_LSTRING},
	{"base_radius", FOFS(base_radius), F_FLOAT},
	{"viewer", FOFS(viewer), F_EDICT},
	{"usermodel", FOFS(usermodel), F_LSTRING},
	{"solidstate", FOFS(solidstate), F_INT},
	{"startframe", FOFS(startframe), F_INT},
	{"framenumbers", FOFS(framenumbers), F_INT},
	{"bleft", FOFS(bleft), F_VECTOR},
	{"tright", FOFS(tright), F_VECTOR},
	{"effects", FOFS(effects), F_INT},
	{"skinnum", FOFS(skinnum), F_INT},
	{"renderfx", FOFS(renderfx), F_INT},
	{"gib_health", FOFS(gib_health), F_INT},
	{"viewmessage", FOFS(viewmessage), F_LSTRING},
//DH--
	{"message", FOFS(message), F_LSTRING},
//CW++
	{"message2", FOFS(message2), F_LSTRING},
	{"message3", FOFS(message3), F_LSTRING},
//CW--
	{"team", FOFS(team), F_LSTRING},
	{"wait", FOFS(wait), F_FLOAT},
	{"delay", FOFS(delay), F_FLOAT},
	{"random", FOFS(random), F_FLOAT},
	{"move_origin", FOFS(move_origin), F_VECTOR},
	{"move_angles", FOFS(move_angles), F_VECTOR},
	{"style", FOFS(style), F_INT},
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"sounds", FOFS(sounds), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(dmg), F_INT},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},
	{"mass", FOFS(mass), F_INT},
	{"volume", FOFS(volume), F_FLOAT},
	{"attenuation", FOFS(attenuation), F_FLOAT},
	{"map", FOFS(map), F_LSTRING},
#ifdef KMQUAKE2_ENGINE_MOD
	{"salpha", FOFS(s.alpha), F_FLOAT},
#endif
	{"musictrack", FOFS(musictrack), F_LSTRING},	// Knightmare- for specifying OGG or CD track
	// model_spawn stuff
	{"usermodel", FOFS(usermodel), F_LSTRING},
	{"startframe", FOFS(startframe), F_INT},
	{"framenumbers", FOFS(framenumbers), F_INT},
	{"solidstate", FOFS(solidstate), F_INT},
	{"renderfx", FOFS(renderfx), F_INT},
	{"effects", FOFS(effects), F_INT},
	{"bleft", FOFS(bleft), F_VECTOR},
	{"tright", FOFS(tright), F_VECTOR},
	// rotating train stuff
	{"pitch_speed", FOFS(pitch_speed), F_FLOAT},
	{"yaw_speed", FOFS(yaw_speed), F_FLOAT},
	{"roll_speed", FOFS(roll_speed), F_FLOAT},
	{"roll", FOFS(roll), F_FLOAT},
	{"from", FOFS(from), F_EDICT},
	{"to", FOFS(to), F_EDICT},
	{"smooth_movement", FOFS(smooth_movement), F_INT},
	{"turn_rider", FOFS(turn_rider), F_INT},
	{"origin_offset", FOFS(origin_offset), F_VECTOR},
	// Knightmare- these are needed to update func_door_secret's positions
	{"width", FOFS(width), F_FLOAT},	
	{"length", FOFS(length), F_FLOAT},
	{"side", FOFS(side), F_FLOAT},
	// end Knightmare

	// temp spawn vars -- only valid when the spawn function is called
	{"lip", STOFS(lip), F_INT, FFL_SPAWNTEMP},
	{"distance", STOFS(distance), F_INT, FFL_SPAWNTEMP},
	{"height", STOFS(height), F_INT, FFL_SPAWNTEMP},
	{"noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP},
	{"pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP},
	{"item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP},
	{"gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP},
	{"sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP},
	{"skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP},
	{"skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP},
	{"minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP},

	// ROGUE
	{"bad_area", FOFS(bad_area), F_EDICT},
	// while the hint_path stuff could be reassembled on the fly, no reason to be different
	{"hint_chain", FOFS(hint_chain), F_EDICT},
	{"monster_hint_chain", FOFS(monster_hint_chain), F_EDICT},
	{"target_hint_chain", FOFS(target_hint_chain), F_EDICT},

//CW++
	{NULL, 0, F_INT, 0}
//CW--
};

// -------- just for savegames ----------
// all pointer fields should be listed here, or savegames
// won't work properly (they will crash and burn).
// this wasn't just tacked on to the fields array, because
// these don't need names, we wouldn't want map fields using
// some of these, and if one were accidentally present twice
// it would double swizzle (fuck) the pointer.

field_t		savefields[] =
{
	{"", FOFS(classname), F_LSTRING},
	{"", FOFS(target), F_LSTRING},
	{"", FOFS(targetname), F_LSTRING},
	{"", FOFS(killtarget), F_LSTRING},
	{"", FOFS(team), F_LSTRING},
	{"", FOFS(pathtarget), F_LSTRING},
	{"", FOFS(deathtarget), F_LSTRING},
	{"", FOFS(combattarget), F_LSTRING},
	{"", FOFS(model), F_LSTRING},
	{"", FOFS(map), F_LSTRING},
	{"", FOFS(message), F_LSTRING},

	{"", FOFS(client), F_CLIENT},
	{"", FOFS(item), F_ITEM},

	{"", FOFS(goalentity), F_EDICT},
	{"", FOFS(movetarget), F_EDICT},
	{"", FOFS(enemy), F_EDICT},
	{"", FOFS(oldenemy), F_EDICT},
	{"", FOFS(activator), F_EDICT},
	{"", FOFS(groundentity), F_EDICT},
	{"", FOFS(teamchain), F_EDICT},
	{"", FOFS(teammaster), F_EDICT},
	{"", FOFS(owner), F_EDICT},
	{"", FOFS(mynoise), F_EDICT},
	{"", FOFS(mynoise2), F_EDICT},
	{"", FOFS(target_ent), F_EDICT},
	{"", FOFS(chain), F_EDICT},

	{NULL, 0, F_INT}
};

field_t		levelfields[] =
{
	{"", LLOFS(changemap), F_LSTRING},																//CW

	{NULL, 0, F_INT}
};

field_t		clientfields[] =
{
	{"", CLOFS(pers.weapon), F_ITEM},
	{"", CLOFS(pers.lastweapon), F_ITEM},
	{"", CLOFS(newweapon), F_ITEM},

	{NULL, 0, F_INT}
};

/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/
void InitGame(void)
{
//CW++
	char	*namebuf = "          ";
	int		newval;
	int		i;
//CW--

	gi.dprintf("====== Init Game ======\n\n");

//CW++
	gi.dprintf("***********************\n");
	gi.dprintf("     Awakening II\n");
	gi.dprintf("        v%s\n", AWK_STRING_VERSION);
	gi.dprintf("***********************\n");
	gi.dprintf("E-mail: musashi@planetquake.com\n\n");

	srand((unsigned)time(NULL));
//CW--

	gun_x = gi.cvar("gun_x", "0", 0);
	gun_y = gi.cvar("gun_y", "0", 0);
	gun_z = gi.cvar("gun_z", "0", 0);

	sv_rollspeed = gi.cvar("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar("sv_maxvelocity", "2000", 0);
	sv_gravity = gi.cvar("sv_gravity", "800", 0);

//	Noset cvars.

	dedicated = gi.cvar("dedicated", "0", CVAR_NOSET);

//	Latched cvars.

	sv_cheats = gi.cvar("cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar("gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar("gamedate", __DATE__ , CVAR_SERVERINFO | CVAR_LATCH);									//CW

	deathmatch = gi.cvar ("deathmatch", "1", CVAR_LATCH);											//CW
	maxclients = gi.cvar("maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH);							//CW

	// Knightmare- increase maxentities
//	maxentities = gi.cvar("maxentities", "1024", CVAR_LATCH);
	maxentities = gi.cvar ("maxentities", va("%i",MAX_EDICTS), CVAR_LATCH);

//CW++
//	Game customisation cvars.

	sv_gametype = gi.cvar("gametype", "0", CVAR_SERVERINFO | CVAR_LATCH);

	sv_respawn_invuln_time = gi.cvar("respawn_invuln_time", "3.0", CVAR_LATCH);
	sv_rnd_powerup_timeout = gi.cvar("rnd_powerup_timeout", "60.0", CVAR_LATCH);
	sv_rnd_mystery_ratio = gi.cvar("rnd_mystery_ratio", "4", CVAR_LATCH);
	sv_show_leader = gi.cvar("show_leader", "0", 0);
	sv_quad_factor = gi.cvar("quad_factor", "4", CVAR_LATCH);
	sv_reject_blank_ip = gi.cvar("reject_blank_ip", "1", CVAR_LATCH);
	sv_map_random = gi.cvar("map_random", "0", 0);

	sv_op_password = gi.cvar("op_password", "", 0);
	sv_op_ignore_maplist = gi.cvar("op_ignore_maplist", "0", 0);

	sv_reserved = gi.cvar("reserved", "0", CVAR_SERVERINFO | CVAR_LATCH);
	sv_rsv_password = gi.cvar("rsv_password", "", 0);

	sv_map_file = gi.cvar("map_file", "mapcfg/maps.txt", 0);
	sv_config_file = gi.cvar("config_file", "configs.txt", 0);
	sv_motd_file = gi.cvar("motd_file", "motd/motd.txt", 0);

	sv_agm_drop = gi.cvar("agm_drop", "0", CVAR_LATCH);
	sv_agm_drop_file = gi.cvar("agm_drop_file", "mapcfg/agm.txt", 0);

	sv_disc_drop = gi.cvar("disc_drop", "0", CVAR_LATCH);
	sv_disc_drop_file = gi.cvar("disc_drop_file", "mapcfg/disc.txt", 0);

	sv_team1_name = gi.cvar("team1_name", "RED", CVAR_LATCH);
	sv_team2_name = gi.cvar("team2_name", "BLUE", CVAR_LATCH);
	sv_team1_model = gi.cvar("team1_model", "male", CVAR_LATCH);
	sv_team2_model = gi.cvar("team2_model", "male", CVAR_LATCH);
	sv_team1_skin = gi.cvar("team1_skin", "ctf_r", CVAR_LATCH);
	sv_team2_skin = gi.cvar("team2_skin", "ctf_b", CVAR_LATCH);

	sv_allow_bots = gi.cvar("allow_bots", "1", CVAR_LATCH);
	sv_bots_camp = gi.cvar("bots_camp", "1", 0);
	sv_bots_insult = gi.cvar("bots_insult", "1", 0);
	sv_bots_taunt = gi.cvar("bots_taunt", "1", 0);
	sv_bots_chat = gi.cvar("bots_chat", "1", 0);
	sv_bots_random = gi.cvar("bots_random", "0", CVAR_LATCH);
	sv_bots_use_file = gi.cvar("bots_use_file", "1", CVAR_LATCH);
	sv_bots_config_file = gi.cvar("bots_config_file", "bots.cfg", 0);

	chedit = gi.cvar ("chedit", "0", CVAR_LATCH);

	sv_allow_gausspistol = gi.cvar("allow_gausspistol", "1", CVAR_LATCH);
	sv_allow_mac10 = gi.cvar("allow_mac10", "1", CVAR_LATCH);
	sv_allow_jackhammer = gi.cvar("allow_jackhammer", "1", CVAR_LATCH);
	sv_allow_traps = gi.cvar("allow_traps", "1", CVAR_LATCH);
	sv_allow_c4 = gi.cvar("allow_c4", "1", CVAR_LATCH);
	sv_allow_spikegun = gi.cvar("allow_spikegun", "1", CVAR_LATCH);
	sv_allow_rocketlauncher = gi.cvar("allow_rocketlauncher", "1", CVAR_LATCH);
	sv_allow_flamethrower = gi.cvar("allow_flamethrower", "1", CVAR_LATCH);
	sv_allow_railgun = gi.cvar("allow_railgun", "1", CVAR_LATCH);
	sv_allow_shockrifle = gi.cvar("allow_shockrifle", "1", CVAR_LATCH);
	sv_allow_agm = gi.cvar("allow_agm", "1", CVAR_LATCH);
	sv_allow_disclauncher = gi.cvar("allow_disclauncher", "1", CVAR_LATCH);
	sv_allow_hook = gi.cvar("allow_hook", "0", CVAR_LATCH);

	sv_allow_invuln = gi.cvar("allow_invuln", "1", CVAR_LATCH);
	sv_allow_quad = gi.cvar("allow_quad", "1", CVAR_LATCH);
	sv_allow_siphon = gi.cvar("allow_siphon", "1", CVAR_LATCH);
	sv_allow_d89 = gi.cvar("allow_d89", "1", CVAR_LATCH);
	sv_allow_haste = gi.cvar("allow_haste", "1", CVAR_LATCH);
	sv_allow_tele = gi.cvar("allow_tele", "1", CVAR_LATCH);
	sv_allow_antibeam = gi.cvar("allow_antibeam", "1", CVAR_LATCH);
	sv_allow_enviro = gi.cvar("allow_enviro", "1", CVAR_LATCH);
	sv_allow_silencer = gi.cvar("allow_silencer", "1", CVAR_LATCH);
	sv_allow_breather = gi.cvar("allow_breather", "1", CVAR_LATCH);

	sv_allow_bullets = gi.cvar("allow_bullets", "1", CVAR_LATCH);
	sv_allow_shells = gi.cvar("allow_shells", "1", CVAR_LATCH);
	sv_allow_rockets = gi.cvar("allow_rockets", "1", CVAR_LATCH);
	sv_allow_cells = gi.cvar("allow_cells", "1", CVAR_LATCH);
	sv_allow_slugs = gi.cvar("allow_slugs", "1", CVAR_LATCH);

	sv_chainsaw_damage = gi.cvar("chainsaw_damage", "50", CVAR_LATCH);
	
	sv_deserteagle_damage = gi.cvar("deserteagle_damage", "35", CVAR_LATCH);
	sv_deserteagle_hspread = gi.cvar("deserteagle_hspread", "300.0", CVAR_LATCH);
	sv_deserteagle_vspread = gi.cvar("deserteagle_vspread", "300.0", CVAR_LATCH);

	sv_jackhammer_damage = gi.cvar("jackhammer_damage", "10", CVAR_LATCH);
	sv_jackhammer_hspread = gi.cvar("jackhammer_hspread", "500.0", CVAR_LATCH);
	sv_jackhammer_vspread = gi.cvar("jackhammer_vspread", "500.0", CVAR_LATCH);

	sv_mac10_damage = gi.cvar("mac10_damage", "20", CVAR_LATCH);
	sv_mac10_hspread = gi.cvar("mac10_hspread", "1200.0", CVAR_LATCH);
	sv_mac10_vspread = gi.cvar("mac10_vspread", "800.0", CVAR_LATCH);
	
	sv_gauss_damage_base = gi.cvar("gauss_damage_base", "35", CVAR_LATCH);
	sv_gauss_damage_rate = gi.cvar("gauss_damage_rate", "0.1", CVAR_LATCH);
	sv_gauss_damage_step = gi.cvar("gauss_damage_step", "5", CVAR_LATCH);
	sv_gauss_damage_max = gi.cvar("gauss_damage_max", "100", CVAR_LATCH);
	sv_gauss_damage_particle = gi.cvar("gauss_damage_particle", "100", CVAR_LATCH);
	sv_gauss_scan_range = gi.cvar("gauss_scan_range", "4000.0", CVAR_LATCH);

	sv_c4_damage = gi.cvar("c4_damage", "100", CVAR_LATCH);
	sv_c4_radius = gi.cvar("c4_radius", "150", CVAR_LATCH);
	sv_c4_touchbang = gi.cvar("c4_touchbang", "0", CVAR_LATCH);
	sv_c4_proximity = gi.cvar("c4_proximity", "0", CVAR_LATCH);
	sv_c4_proximity_range = gi.cvar("c4_proximity_range", "75", CVAR_LATCH);
	sv_c4_proximity_delay = gi.cvar("c4_proximity_delay", "0.5", CVAR_LATCH);
	sv_c4_proximity_life = gi.cvar("c4_proximity_life", "15.0", CVAR_LATCH);
	sv_c4_min_speed = gi.cvar("c4_min_speed", "400", CVAR_LATCH);
	sv_c4_max_speed = gi.cvar("c4_max_speed", "1600", CVAR_LATCH);
	sv_c4_hold_accel = gi.cvar("c4_hold_accel", "120", CVAR_LATCH);
	sv_c4_timelimit = gi.cvar("c4_timelimit", "0", CVAR_LATCH);
	sv_c4_spawn_range = gi.cvar("c4_spawn_range", "128.0", CVAR_LATCH);

	sv_trap_hook_damage = gi.cvar("trap_hook_damage", "20", CVAR_LATCH);
	sv_trap_hook_speed = gi.cvar("trap_hook_speed", "1000", CVAR_LATCH);
	sv_trap_beam_damage = gi.cvar("trap_beam_damage", "1", CVAR_LATCH);
	sv_trap_beam_power = gi.cvar("trap_beam_power", "175", CVAR_LATCH);
	sv_trap_proximity_range = gi.cvar("trap_proximity_range", "500", CVAR_LATCH);
	sv_trap_activate_delay = gi.cvar("trap_activate_delay", "2.0", CVAR_LATCH);
	sv_trap_min_speed = gi.cvar("trap_min_speed", "400", CVAR_LATCH);
	sv_trap_max_speed = gi.cvar("trap_max_speed", "800", CVAR_LATCH);
	sv_trap_hold_accel = gi.cvar("trap_hold_accel", "40", CVAR_LATCH);
	sv_trap_thru_tele = gi.cvar("trap_thru_tele", "0", CVAR_LATCH);
	sv_traps_max_active = gi.cvar("traps_max_active", "5", CVAR_LATCH);
	sv_traps_timelimit = gi.cvar("traps_timelimit", "0", CVAR_LATCH);

	sv_spike_damage = gi.cvar("spike_damage", "25", CVAR_LATCH);
	sv_spike_speed = gi.cvar("spike_speed", "1750", CVAR_LATCH);
	sv_spike_bang_damage = gi.cvar("spike_bang_damage", "100", CVAR_LATCH);
	sv_spike_bang_radius = gi.cvar("spike_bang_radius", "150", CVAR_LATCH);
	sv_spike_bang_delay = gi.cvar("spike_bang_delay", "2.0", CVAR_LATCH);

	sv_rocket_damage = gi.cvar("rocket_damage", "100", CVAR_LATCH);
	sv_rocket_radius = gi.cvar("rocket_radius", "120", CVAR_LATCH);
	sv_rocket_radius_damage = gi.cvar("rocket_radius_damage", "120", CVAR_LATCH);
	sv_rocket_speed = gi.cvar("rocket_speed", "650", CVAR_LATCH);
	sv_rocket_kick_scale = gi.cvar("rocket_kick_scale", "1500.0", CVAR_LATCH);

	sv_flame_damage = gi.cvar("flame_damage", "5", CVAR_LATCH);
	sv_flame_small_damage = gi.cvar("flame_small_damage", "2", CVAR_LATCH);
	sv_flame_speed = gi.cvar("flame_speed", "550", CVAR_LATCH);
	sv_firebomb_damage = gi.cvar("firebomb_damage", "50", CVAR_LATCH);
	sv_firebomb_radius = gi.cvar("firebomb_radius", "150", CVAR_LATCH);
	sv_firebomb_speed = gi.cvar("firebomb_speed", "650", CVAR_LATCH);
	sv_flame_thru_tele = gi.cvar("flame_thru_tele", "0", CVAR_LATCH);

	sv_railgun_damage = gi.cvar("railgun_damage", "100", CVAR_LATCH);
	
	sv_shock_homing_damage = gi.cvar("shock_homing_damage", "40", CVAR_LATCH);
	sv_shock_homing_range = gi.cvar("shock_homing_range", "250", CVAR_LATCH);
	sv_shock_speed = gi.cvar("shock_speed", "1000", CVAR_LATCH);
	sv_shock_homing_speed = gi.cvar("shock_homing_speed", "700", CVAR_LATCH);
	sv_shock_radius_damage = gi.cvar("shock_radius_damage", "75", CVAR_LATCH);
	sv_shock_radius = gi.cvar("shock_radius", "128", CVAR_LATCH);
	sv_shock_freeze_time = gi.cvar("shock_freeze_time", "1.2", CVAR_LATCH);
	sv_shock_live_time = gi.cvar("shock_live_time", "3.0", CVAR_LATCH);

	sv_agm_mode = gi.cvar("agm_mode", "0", CVAR_LATCH);
	sv_agm_charge_rate = gi.cvar("agm_charge_rate", "5", CVAR_LATCH);
	sv_agm_fire_rate = gi.cvar("agm_fire_rate", "10", CVAR_LATCH);
	sv_agm_shot_cells = gi.cvar("agm_shot_cells", "10", CVAR_LATCH);
	sv_agm_beam_cells = gi.cvar("agm_beam_cells", "1", CVAR_LATCH);
	sv_agm_fling_power = gi.cvar("agm_fling_power", "2000.0", CVAR_LATCH);
	sv_agm_cross_damage = gi.cvar("agm_cross_damage", "20.0", CVAR_LATCH);
	sv_agm_cross_timeout = gi.cvar("agm_cross_timeout", "2.0", CVAR_LATCH);
	sv_agm_reflect_damage = gi.cvar("agm_reflect_damage", "20.0", CVAR_LATCH);
	sv_agm_reflect_timeout = gi.cvar("agm_reflect_timeout", "2.0", CVAR_LATCH);
	sv_agm_invuln_cells = gi.cvar("agm_invuln_cells", "1", CVAR_LATCH);
	sv_agm_disrupt_damage = gi.cvar("agm_disrupt_damage", "20.0", CVAR_LATCH);
	sv_agm_disrupt_siphon = gi.cvar("agm_disrupt_siphon", "0.2", CVAR_LATCH);

	sv_disc_damage = gi.cvar("disc_damage", "75", CVAR_LATCH);
	sv_disc_speed = gi.cvar("disc_speed", "1500.0", CVAR_LATCH);
	sv_disc_live_time = gi.cvar("disc_live_time", "2.0", CVAR_LATCH);

	sv_plasma_damage = gi.cvar("plasma_damage", "25.0", CVAR_LATCH);
	sv_plasma_speed = gi.cvar("plasma_speed", "800.0", CVAR_LATCH);

	sv_hook_damage = gi.cvar("hook_damage", "10", CVAR_LATCH);
	sv_hook_speed = gi.cvar("hook_speed", "750", CVAR_LATCH);
	sv_hook_pull_speed = gi.cvar("hook_pull_speed", "650", CVAR_LATCH);
	sv_hook_pull_agm = gi.cvar("hook_pull_agm", "0.5", CVAR_LATCH);
	sv_hook_offhand = gi.cvar("hook_offhand", "1", CVAR_LATCH);

	sv_health_initial = gi.cvar("health_initial", "100", CVAR_LATCH);
	sv_health_max = gi.cvar("health_max", "100", CVAR_LATCH);
	sv_health_max_siphon = gi.cvar("health_max_siphon", "150", CVAR_LATCH);
	sv_health_max_bonus = gi.cvar("health_max_bonus", "250", CVAR_LATCH);

	sv_have_deserteagle = gi.cvar("have_deserteagle", "1", CVAR_LATCH);
	sv_have_gausspistol = gi.cvar("have_gausspistol", "0", CVAR_LATCH);
	sv_have_mac10 = gi.cvar("have_mac10", "0", CVAR_LATCH);
	sv_have_jackhammer = gi.cvar("have_jackhammer", "0", CVAR_LATCH);
	sv_have_c4 = gi.cvar("have_c4", "0", CVAR_LATCH);
	sv_have_traps = gi.cvar("have_traps", "0", CVAR_LATCH);
	sv_have_spikegun = gi.cvar("have_spikegun", "0", CVAR_LATCH);
	sv_have_rocketlauncher = gi.cvar("have_rocketlauncher", "0", CVAR_LATCH);
	sv_have_flamethrower = gi.cvar("have_flamethrower", "0", CVAR_LATCH);
	sv_have_railgun = gi.cvar("have_railgun", "0", CVAR_LATCH);
	sv_have_shockrifle = gi.cvar("have_shockrifle", "0", CVAR_LATCH);
	sv_have_agm = gi.cvar("have_agm", "0", CVAR_LATCH);
	sv_have_disclauncher = gi.cvar("have_disclauncher", "0", CVAR_LATCH);

	sv_initial_weapon = gi.cvar("initial_weapon", "0", CVAR_LATCH);

	sv_initial_bullets = gi.cvar("initial_bullets", "50", CVAR_LATCH);
	sv_initial_shells = gi.cvar("initial_shells", "0", CVAR_LATCH);
	sv_initial_c4 = gi.cvar("initial_c4", "0", CVAR_LATCH);
	sv_initial_traps = gi.cvar("initial_traps", "0", CVAR_LATCH);
	sv_initial_rockets = gi.cvar("initial_rockets", "0", CVAR_LATCH);
	sv_initial_cells = gi.cvar("initial_cells", "0", CVAR_LATCH);
	sv_initial_slugs = gi.cvar("initial_slugs", "0", CVAR_LATCH);

	sv_initial_armor = gi.cvar("initial_armor", "0", CVAR_LATCH);
	sv_initial_armortype = gi.cvar("initial_armortype", "0", CVAR_LATCH);

	sv_max_bullets = gi.cvar("max_bullets", "200", CVAR_LATCH);
	sv_max_shells = gi.cvar("max_shells", "80", CVAR_LATCH);
	sv_max_c4 = gi.cvar("max_c4", "20", CVAR_LATCH);
	sv_max_traps = gi.cvar("max_traps", "20", CVAR_LATCH);
	sv_max_rockets = gi.cvar("max_rockets", "20", CVAR_LATCH);
	sv_max_cells = gi.cvar("max_cells", "200", CVAR_LATCH);
	sv_max_slugs = gi.cvar("max_slugs", "40", CVAR_LATCH);

	sv_max_band_bullets = gi.cvar("max_band_bullets", "250", CVAR_LATCH);
	sv_max_band_shells = gi.cvar("max_band_shells", "100", CVAR_LATCH);
	sv_max_band_cells = gi.cvar("max_band_cells", "250", CVAR_LATCH);
	sv_max_band_slugs = gi.cvar("max_band_slugs", "50", CVAR_LATCH);

	sv_max_pack_bullets = gi.cvar("max_pack_bullets", "300", CVAR_LATCH);
	sv_max_pack_shells = gi.cvar("max_pack_shells", "120", CVAR_LATCH);
	sv_max_pack_c4 = gi.cvar("max_pack_c4", "30", CVAR_LATCH);
	sv_max_pack_traps = gi.cvar("max_pack_traps", "30", CVAR_LATCH);
	sv_max_pack_rockets = gi.cvar("max_pack_rockets", "30", CVAR_LATCH);
	sv_max_pack_cells = gi.cvar("max_pack_cells", "300", CVAR_LATCH);
	sv_max_pack_slugs = gi.cvar("max_pack_slugs", "60", CVAR_LATCH);

	sv_mystery_invuln = gi.cvar("mystery_invuln", "1", CVAR_LATCH);
	sv_mystery_quad = gi.cvar("mystery_quad", "1", CVAR_LATCH);
	sv_mystery_d89 = gi.cvar("mystery_d89", "1", CVAR_LATCH);
	sv_mystery_haste = gi.cvar("mystery_haste", "1", CVAR_LATCH);
	sv_mystery_siphon = gi.cvar("mystery_siphon", "1", CVAR_LATCH);
	sv_mystery_antibeam = gi.cvar("mystery_antibeam", "1", CVAR_LATCH);
	sv_mystery_enviro = gi.cvar("mystery_enviro", "1", CVAR_LATCH);
	sv_mystery_tele = gi.cvar("mystery_tele", "1", CVAR_LATCH);

	sv_vote_percentage = gi.cvar("vote_percentage", "51", 0);
	sv_vote_time = gi.cvar("vote_time", "20.0", 0);
	sv_vote_minclients = gi.cvar("vote_minclients", "2", 0);

	sv_vote_allow_map = gi.cvar("vote_allow_map", "1", 0);
	sv_vote_allow_config = gi.cvar("vote_allow_config", "1", 0);
	sv_vote_allow_match = gi.cvar("vote_allow_match", "1", 0);
	sv_vote_allow_timelimit = gi.cvar("vote_allow_timelimit", "1", 0);
	sv_vote_allow_fraglimit = gi.cvar("vote_allow_fraglimit", "1", 0);
	sv_vote_allow_capturelimit = gi.cvar("vote_allow_capturelimit", "1", 0);
	sv_vote_allow_hook = gi.cvar("vote_allow_hook", "1", 0);
	sv_vote_allow_hook_offhand = gi.cvar("vote_allow_hook_offhand", "1", 0);

	sv_vote_allow_bots = gi.cvar("vote_allow_bots", "1", 0);

	sv_vote_allow_gausspistol = gi.cvar("vote_allow_gausspistol", "1", 0);
	sv_vote_allow_mac10 = gi.cvar("vote_allow_mac10", "1", 0);
	sv_vote_allow_jackhammer = gi.cvar("vote_allow_jackhammer", "1", 0);
	sv_vote_allow_c4 = gi.cvar("vote_allow_c4", "1", 0);
	sv_vote_allow_traps = gi.cvar("vote_allow_traps", "1", 0);
	sv_vote_allow_esg = gi.cvar("vote_allow_esg", "1", 0);
	sv_vote_allow_rocketlauncher = gi.cvar("vote_allow_rocketlauncher", "1", 0);
	sv_vote_allow_flamethrower = gi.cvar("vote_allow_flamethrower", "1", 0);
	sv_vote_allow_railgun = gi.cvar("vote_allow_railgun", "1", 0);
	sv_vote_allow_shockrifle = gi.cvar("vote_allow_shockrifle", "1", 0);
	sv_vote_allow_agm = gi.cvar("vote_allow_agm", "1", 0);
	sv_vote_allow_disclauncher = gi.cvar("vote_allow_disclauncher", "1", 0);

	sv_vote_allow_invuln = gi.cvar("vote_allow_invuln", "1", 0);
	sv_vote_allow_quad = gi.cvar("vote_allow_quad", "1", 0);
	sv_vote_allow_siphon = gi.cvar("vote_allow_siphon", "1", 0);
	sv_vote_allow_d89 = gi.cvar("vote_allow_d89", "1", 0);
	sv_vote_allow_haste = gi.cvar("vote_allow_haste", "1", 0);
	sv_vote_allow_tele = gi.cvar("vote_allow_tele", "1", 0);
	sv_vote_allow_antibeam = gi.cvar("vote_allow_antibeam", "1", 0);
	sv_vote_allow_enviro = gi.cvar("vote_allow_enviro", "1", 0);
	sv_vote_allow_silencer = gi.cvar("vote_allow_silencer", "1", 0);
	sv_vote_allow_breather = gi.cvar("vote_allow_breather", "1", 0);

	sv_vote_allow_nohealth = gi.cvar("vote_allow_nohealth", "1", 0);
	sv_vote_allow_noitems = gi.cvar("vote_allow_noitems", "1", 0);
	sv_vote_allow_weaponsstay = gi.cvar("vote_allow_weaponsstay", "1", 0);
	sv_vote_allow_nofalling = gi.cvar("vote_allow_nofalling", "1", 0);
	sv_vote_allow_noarmor = gi.cvar("vote_allow_noarmor", "1", 0);
	sv_vote_allow_infiniteammo = gi.cvar("vote_allow_infiniteammo", "1", 0);
	sv_vote_allow_quaddrop = gi.cvar("vote_allow_quaddrop", "1", 0);
	sv_vote_allow_fastswitch = gi.cvar("vote_allow_fastswitch", "1", 0);
	sv_vote_allow_extraitems = gi.cvar("vote_allow_extraitems", "1", 0);
	sv_vote_allow_noreplacements = gi.cvar("vote_allow_noreplacements", "1", 0);
	sv_vote_allow_notech = gi.cvar("vote_allow_notech", "1", 0);
	sv_vote_allow_ctfspawns = gi.cvar("vote_allow_ctfspawns", "1", 0);

	sv_vote_timelimit_max = gi.cvar("vote_timelimit_max", "60", 0);
	sv_vote_timelimit_step = gi.cvar("vote_timelimit_step", "5", 0);
	sv_vote_fraglimit_max = gi.cvar("vote_fraglimit_max", "100", 0);
	sv_vote_fraglimit_step = gi.cvar("vote_fraglimit_step", "5", 0);
	sv_vote_capturelimit_max = gi.cvar("vote_capturelimit_max", "10", 0);
	sv_vote_capturelimit_step = gi.cvar("vote_capturelimit_step", "1", 0);

	footstep_sounds = gi.cvar("footstep_sounds", "0", CVAR_SERVERINFO|CVAR_LATCH);	// Knightmare added
	sv_step_fraction = gi.cvar ("sv_step_fraction", "0.90", 0);	// Knightmare- this was a define in p_view.c

//	Enforce limits on certain cvars...

//	...general game settings.

	if (deathmatch->value != 1)
		gi.cvar_forceset("deathmatch", "1");

	gi.cvar_forceset("gametype", va("%d", (int)sv_gametype->value));
	if ((sv_gametype->value < 0) || (sv_gametype->value > G_ASLT))
		gi.cvar_forceset("gametype", va("%d", G_FFA));

	gi.cvar_forceset("sv_rnd_mystery_ratio", va("%d", (int)sv_rnd_mystery_ratio->value));

	if (sv_quad_factor->value < 0.0)
		gi.cvar_forceset("quad_factor", "4.0");

	if (sv_reserved->value < 0.0)
		gi.cvar_forceset("reserved", "0");

	if (sv_reserved->value > (int)maxclients->value)
		gi.cvar_forceset("reserved", va("%d", (int)maxclients->value));

	if (strlen(sv_rsv_password->string) == 0)
		gi.cvar_forceset("reserved", "0");

//	...weapons.

	gi.cvar_set("traps_max_active", va("%d", (int)sv_traps_max_active->value));
	if (sv_traps_max_active->value < 1.0)
		gi.cvar_forceset("traps_max_active", "1");

	if (sv_agm_fling_power->value < 0.0)
		gi.cvar_forceset("agm_fling_power", "0.0");

	if ((sv_c4_timelimit->value > 0.0) && (sv_c4_timelimit->value < 1.0))
		gi.cvar_forceset("c4_timelimit", "1.0");

	if (sv_c4_proximity_life->value <= 0.0)
		gi.cvar_forceset("c4_proximity_life", "15.0");

	if (sv_c4_proximity_life->value > 30.0)
		gi.cvar_forceset("c4_proximity_life", "30.0");

	if ((sv_traps_timelimit->value > 0.0) && (sv_traps_timelimit->value < 1.0))
		gi.cvar_forceset("traps_timelimit", "1.0");

	if (sv_rocket_kick_scale->value < 0.0)
		gi.cvar_forceset("rocket_kick_scale", "0.0");

	if (sv_shock_live_time->value < 1.0)
		gi.cvar_forceset("shock_live_time", "1.0");

	if (sv_agm_charge_rate->value < 0.0)
		gi.cvar_forceset("agm_charge_rate", "0.0");

	if (sv_agm_fire_rate->value < 0.0)
		gi.cvar_forceset("agm_fire_rate", "0.0");

	gi.cvar_forceset("agm_beam_cells", va("%d", (int)sv_agm_beam_cells->value));
	if (sv_agm_beam_cells->value < 1.0)
		gi.cvar_forceset("agm_beam_cells", "1.0");

	gi.cvar_forceset("agm_shot_cells", va("%d", (int)sv_agm_shot_cells->value));
	if (sv_agm_shot_cells->value < 1.0)
		gi.cvar_forceset("agm_shot_cells", "1.0");

	if (sv_agm_cross_damage->value < 0.0)
		gi.cvar_forceset("agm_cross_damage", "0.0");

	if (sv_agm_reflect_damage->value < 0.0)
		gi.cvar_forceset("agm_reflect_damage", "0.0");

	if (sv_disc_live_time->value < 1.0)
		gi.cvar_forceset("disc_live_time", "1.0");

//	...grappling hook.

	if (sv_hook_pull_agm->value < 0.0)
		gi.cvar_forceset("hook_pull_agm", "0.0");

	if (sv_hook_pull_agm->value > 1.0)
		gi.cvar_forceset("hook_pull_agm", "1.0");

//	...team names.

	if (strlen(sv_team1_name->string) > 10)
	{
		for (i = 0; i < 10; ++i)
			*(namebuf + i) = *(sv_team1_name->string + i);
		gi.cvar_set("team1_name", namebuf);
	}

	if (strlen(sv_team2_name->string) > 10)
	{
		for (i = 0; i < 10; ++i)
			*(namebuf + i) = *(sv_team2_name->string + i);
		gi.cvar_set("team2_name", namebuf);
	}

//	...initial ammo counts.

	if (sv_initial_bullets->value < 0.0)
		gi.cvar_forceset("initial_bullets", "0.0");

	if (sv_initial_shells->value < 0.0)
		gi.cvar_forceset("initial_shells", "0.0");

	if (sv_initial_c4->value < 0.0)
		gi.cvar_forceset("initial_c4", "0.0");

	if (sv_initial_traps->value < 0.0)
		gi.cvar_forceset("initial_traps", "0.0");

	if (sv_initial_rockets->value < 0.0)
		gi.cvar_forceset("initial_rockets", "0.0");

	if (sv_initial_cells->value < 0.0)
		gi.cvar_forceset("initial_cells", "0.0");

	if (sv_initial_slugs->value < 0.0)
		gi.cvar_forceset("initial_slugs", "0.0");

	if ((int)sv_initial_c4->value)
		gi.cvar_forceset("have_c4", "1");

	if ((int)sv_initial_traps->value)
		gi.cvar_forceset("have_traps", "1");

//	...voting.

	if (sv_vote_percentage->value > 100.0)
		gi.cvar_forceset("vote_percentage", "100.0");

	if (sv_vote_time->value < 5.0)
		gi.cvar_forceset("vote_time", "5.0");
//CW--

//	Change anytime cvars.

	dmflags = gi.cvar("dmflags", "0", CVAR_SERVERINFO);
	fraglimit = gi.cvar("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar("timelimit", "0", CVAR_SERVERINFO);
	capturelimit = gi.cvar("capturelimit", "0", CVAR_SERVERINFO);									//ZOID++

//CW++
	if (fraglimit->value < 0.0)
		gi.cvar_forceset("fraglimit", "0.0");

	if (timelimit->value < 0.0)
		gi.cvar_forceset("timelimit", "0.0");

	if (capturelimit->value < 0.0)
		gi.cvar_forceset("capturelimit", "0.0");

//	Note that DF_MODELTEAMS and DF_SKINTEAMS are obsolete given the new team code.

	if ((int)dmflags->value & DF_MODELTEAMS)
	{
		newval = (int)dmflags->value & ~DF_MODELTEAMS;
		gi.dprintf("Server: removing dmflag DF_MODELTEAMS.\n");
		gi.cvar_forceset("dmflags", va("%d", newval));
	}

	if ((int)dmflags->value & DF_SKINTEAMS)
	{
		newval = (int)dmflags->value & ~DF_SKINTEAMS;
		gi.dprintf("Server: removing dmflag DF_SKINTEAMS.\n");
		gi.cvar_forceset("dmflags", va("%d", newval));
	}
//CW--

	password = gi.cvar("password", "", CVAR_USERINFO);
	filterban = gi.cvar("filterban", "1", 0);

	g_select_empty = gi.cvar("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar("run_pitch", "0.002", 0);
	run_roll = gi.cvar("run_roll", "0.005", 0);
	bob_up  = gi.cvar("bob_up", "0.005", 0);
	bob_pitch = gi.cvar("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar("bob_roll", "0.002", 0);

	turn_rider = gi.cvar("turn_rider", "1", CVAR_SERVERINFO);	// Knightmare added

//	Flood control.

	flood_msgs = gi.cvar("flood_msgs", "4", 0);
	flood_persecond = gi.cvar("flood_persecond", "4", 0);
	flood_waitdelay = gi.cvar("flood_waitdelay", "10", 0);

//	Initialize items.

	InitItems();

	Com_sprintf(game.helpmessage1, sizeof(game.helpmessage1), "");
	Com_sprintf(game.helpmessage2, sizeof(game.helpmessage2), "");

//	Initialize all entities for this game.

	game.maxentities = (int)maxentities->value;
	g_edicts = gi.TagMalloc(game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

//	Initialize all clients for this game.

	game.maxclients = (int)maxclients->value;
	game.clients = gi.TagMalloc(game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients + 1;

//CW++
	memset(g_oldmaps, 0, sizeof(g_oldmaps));
	memset(g_slots, 0, sizeof(g_slots));
//CW--

	CTFInit();																						//ZOID++
	LoadBots();																						//CW++
}

//=========================================================

void WriteField1(FILE *f, field_t *field, byte *base)
{
	void	*p;
	int		len;
	int		index;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
		case F_INT:
		case F_FLOAT:
		case F_ANGLEHACK:
		case F_VECTOR:
		case F_IGNORE:
			break;

		case F_LSTRING:
		case F_GSTRING:
			if (*(char **)p)
				len = (int)strlen(*(char **)p) + 1;
			else
				len = 0;
			*(int *)p = len;
			break;

		case F_EDICT:
			if (*(edict_t **)p == NULL)
				index = -1;
			else
				index = *(edict_t **)p - g_edicts;
			*(int *)p = index;
			break;

		case F_CLIENT:
			if (*(gclient_t **)p == NULL)
				index = -1;
			else
				index = *(gclient_t **)p - game.clients;
			*(int *)p = index;
			break;

		case F_ITEM:
			if (*(edict_t **)p == NULL)
				index = -1;
			else
				index = *(gitem_t **)p - itemlist;
			*(int *)p = index;
			break;

		default:
			gi.error("WriteField1(): unknown field type");											//CW
	}
}

void WriteField2(FILE *f, field_t *field, byte *base)
{
	int			len;
	void		*p;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
		case F_LSTRING:
		case F_GSTRING:
			if (*(char **)p)
			{
				len = (int)strlen(*(char **)p) + 1;
				fwrite (*(char **)p, len, 1, f);
			}
			break;

//CW++
		case F_INT:
		case F_FLOAT:
		case F_VECTOR:
		case F_ANGLEHACK:
		case F_EDICT:
		case F_ITEM:
		case F_CLIENT:
		case F_IGNORE:
			break;

		default:	// sanity check
			gi.error("WriteField2(): unknown field type");
//CW--
	}
}

void ReadField(FILE *f, field_t *field, byte *base)
{
	void	*p;
	int		len;
	int		index;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
		case F_INT:
		case F_FLOAT:
		case F_ANGLEHACK:
		case F_VECTOR:
		case F_IGNORE:
			break;

		case F_LSTRING:
			len = *(int *)p;
			if (!len)
				*(char **)p = NULL;
			else
			{
				*(char **)p = gi.TagMalloc (len, TAG_LEVEL);
				fread (*(char **)p, len, 1, f);
			}
			break;

		case F_GSTRING:
			len = *(int *)p;
			if (!len)
				*(char **)p = NULL;
			else
			{
				*(char **)p = gi.TagMalloc (len, TAG_GAME);
				fread (*(char **)p, len, 1, f);
			}
			break;

		case F_EDICT:
			index = *(int *)p;
			if ( index == -1 )
				*(edict_t **)p = NULL;
			else
				*(edict_t **)p = &g_edicts[index];
			break;

		case F_CLIENT:
			index = *(int *)p;
			if ( index == -1 )
				*(gclient_t **)p = NULL;
			else
				*(gclient_t **)p = &game.clients[index];
			break;

		case F_ITEM:
			index = *(int *)p;
			if ( index == -1 )
				*(gitem_t **)p = NULL;
			else
				*(gitem_t **)p = &itemlist[index];
			break;

		default:
			gi.error("ReadEdict: unknown field type");
	}
}

//=========================================================

/*
==============
WriteClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteClient(FILE *f, gclient_t *client)
{
	field_t		*field;
	gclient_t	temp;
	
	// all of the ints, floats, and vectors stay as they are
	temp = *client;

	// change the pointers to lengths or indexes
	for (field = clientfields; field->name; ++field)
		WriteField1(f, field, (byte *)&temp);

	// write the block
	fwrite(&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field = clientfields; field->name; ++field)
		WriteField2(f, field, (byte *)client);
}

/*
==============
ReadClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadClient(FILE *f, gclient_t *client)
{
	field_t	*field;

	fread(client, sizeof(*client), 1, f);

	for (field = clientfields; field->name; ++field)
		ReadField(f, field, (byte *)client);
}

/*
============
WriteGame

This will be called whenever the game goes to a new level,
and when the user explicitly saves the game.

Game information include cross level data, like multi level
triggers, help computer info, and all client states.

A single player death will automatically restore from the
last save position.
============
*/
void WriteGame(char *filename, qboolean autosave)
{
	FILE	*f;
	int		i;
	char	str[16];

	if (!autosave)
		SaveClientData();

	f = fopen(filename, "wb");
	if (!f)
		gi.error("Couldn't open %s", filename);

	memset(str, 0, sizeof(str));
	Com_strcpy(str, sizeof(str), __DATE__);
	fwrite(str, sizeof(str), 1, f);

	game.autosaved = autosave;
	fwrite(&game, sizeof(game), 1, f);
	game.autosaved = false;

	for (i = 0; i < game.maxclients; i++)
		WriteClient(f, &game.clients[i]);

	fclose(f);
}

void ReadGame(char *filename)
{
	FILE	*f;
	int		i;
	char	str[16];

	gi.FreeTags(TAG_GAME);

	f = fopen(filename, "rb");
	if (!f)
		gi.error("Couldn't open %s", filename);

	fread(str, sizeof(str), 1, f);
	if (strcmp(str, __DATE__))
	{
		fclose(f);
		gi.error("Savegame from an older version.\n");
	}

	g_edicts =  gi.TagMalloc(game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;

	fread(&game, sizeof(game), 1, f);
	game.clients = gi.TagMalloc(game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	for (i = 0; i < game.maxclients; i++)
		ReadClient(f, &game.clients[i]);

	fclose(f);
}

//==========================================================


/*
==============
WriteEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteEdict(FILE *f, edict_t *ent)
{
	field_t	*field;
	edict_t	temp;

	// all of the ints, floats, and vectors stay as they are
	temp = *ent;

	// change the pointers to lengths or indexes
	for (field = savefields; field->name; field++)
		WriteField1(f, field, (byte *)&temp);

	// write the block
	fwrite(&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field = savefields; field->name; field++)
		WriteField2(f, field, (byte *)ent);
}

/*
==============
WriteLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteLevelLocals(FILE *f)
{
	field_t			*field;
	level_locals_t	temp;

	// all of the ints, floats, and vectors stay as they are
	temp = level;

	// change the pointers to lengths or indexes
	for (field = levelfields; field->name; field++)
		WriteField1(f, field, (byte *)&temp);

	// write the block
	fwrite(&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field = levelfields; field->name; field++)
		WriteField2(f, field, (byte *)&level);
}


/*
==============
ReadEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadEdict(FILE *f, edict_t *ent)
{
	field_t *field;

	fread(ent, sizeof(*ent), 1, f);

	for (field = savefields; field->name; field++)
		ReadField(f, field, (byte *)ent);
}

/*
==============
ReadLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadLevelLocals(FILE *f)
{
	field_t *field;

	fread(&level, sizeof(level), 1, f);

	for (field = levelfields; field->name; field++)
		ReadField(f, field, (byte *)&level);
}

/*
=================
WriteLevel
=================
*/
void WriteLevel(char *filename)
{
	edict_t	*ent;
	void	*base;
	FILE	*f;
	int		i;

	f = fopen(filename, "wb");
	if (!f)
		gi.error("Couldn't open %s", filename);

	// write out edict size for checking
	i = sizeof(edict_t);
	fwrite(&i, sizeof(i), 1, f);

	// write out a function pointer for checking
	base = (void *)InitGame;
	fwrite(&base, sizeof(base), 1, f);

	// write out level_locals_t
	WriteLevelLocals(f);

	// write out all the entities
	for (i = 0; i < globals.num_edicts; i++)
	{
		ent = &g_edicts[i];
		if (!ent->inuse)
			continue;

		fwrite(&i, sizeof(i), 1, f);
		WriteEdict(f, ent);
	}
	i = -1;
	fwrite(&i, sizeof(i), 1, f);

	fclose(f);
}


/*
=================
ReadLevel

SpawnEntities will allready have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines
set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
=================
*/
void ReadLevel(char *filename)
{
	edict_t	*ent;
	void	*base;
	FILE	*f;
	int		entnum;
	int		i;

	f = fopen(filename, "rb");
	if (!f)
		gi.error("Couldn't open %s", filename);

	// free any dynamic memory allocated by loading the level
	// base state
	gi.FreeTags(TAG_LEVEL);

	// wipe all the entities
	memset(g_edicts, 0, game.maxentities * sizeof(g_edicts[0]));
	globals.num_edicts = (int)maxclients->value + 1;

	// check edict size
	fread(&i, sizeof(i), 1, f);
	if (i != sizeof(edict_t))
	{
		fclose(f);
		gi.error("ReadLevel: mismatched edict size");
	}

	// check function pointer base address
	fread(&base, sizeof(base), 1, f);
	if (base != (void *)InitGame)
	{
		fclose(f);
		gi.error("ReadLevel: function pointers have moved");
	}

	// load the level locals
	ReadLevelLocals(f);

	// load all the entities
	while (1)
	{
		if (fread(&entnum, sizeof(entnum), 1, f) != 1)
		{
			fclose(f);
			gi.error("ReadLevel: failed to read entnum");
		}
		if (entnum == -1)
			break;

		if (entnum >= globals.num_edicts)
			globals.num_edicts = entnum + 1;

		ent = &g_edicts[entnum];
		ReadEdict(f, ent);

		// let the server rebuild world links for this ent
		memset(&ent->area, 0, sizeof(ent->area));
		gi.linkentity(ent);
	}
	fclose(f);

	// mark all clients as unconnected
	for (i = 0; i < (int)maxclients->value; i++)
	{
		ent = &g_edicts[i+1];
		ent->client = game.clients + i;
		ent->client->pers.connected = false;
	}

	// do any load time things at this point
	for (i = 0; i < globals.num_edicts; i++)
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
			continue;

		// fire any cross-level triggers
		if (ent->classname)
		{
			if (strcmp(ent->classname, "target_crosslevel_target") == 0)
				ent->nextthink = level.time + ent->delay;
		}
	}
}
