/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2000-2002 Mr. Hyde and Mad Dog

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

#define Function(f) {#f, f}

mmove_t mmove_reloc;

field_t fields[] = {
	{"classname", FOFS(classname), F_LSTRING},
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
	{"message", FOFS(message), F_LSTRING},
	{"key_message", FOFS(key_message), F_LSTRING},
	{"team", FOFS(team), F_LSTRING},
	{"wait", FOFS(wait), F_FLOAT},
	{"delay", FOFS(delay), F_FLOAT},
	{"random", FOFS(random), F_FLOAT},
	{"move_origin", FOFS(move_origin), F_VECTOR},
	{"move_angles", FOFS(move_angles), F_VECTOR},
	{"style", FOFS(style), F_INT},
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"health2", FOFS(health2), F_INT},
	{"sounds", FOFS(sounds), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(dmg), F_INT},
	{"mass", FOFS(mass), F_INT},
	{"volume", FOFS(volume), F_FLOAT},
	{"attenuation", FOFS(attenuation), F_FLOAT},
	{"map", FOFS(map), F_LSTRING},
	{"origin", FOFS(s.origin), F_VECTOR},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},

	{"goalentity", FOFS(goalentity), F_EDICT, FFL_NOSPAWN},
	{"movetarget", FOFS(movetarget), F_EDICT, FFL_NOSPAWN},
	{"enemy", FOFS(enemy), F_EDICT, FFL_NOSPAWN},
	{"oldenemy", FOFS(oldenemy), F_EDICT, FFL_NOSPAWN},
	{"activator", FOFS(activator), F_EDICT, FFL_NOSPAWN},
	{"groundentity", FOFS(groundentity), F_EDICT, FFL_NOSPAWN},
	{"teamchain", FOFS(teamchain), F_EDICT, FFL_NOSPAWN},
	{"teammaster", FOFS(teammaster), F_EDICT, FFL_NOSPAWN},
	{"owner", FOFS(owner), F_EDICT, FFL_NOSPAWN},
	{"mynoise", FOFS(mynoise), F_EDICT, FFL_NOSPAWN},
	{"mynoise2", FOFS(mynoise2), F_EDICT, FFL_NOSPAWN},
	{"target_ent", FOFS(target_ent), F_EDICT, FFL_NOSPAWN},
	{"chain", FOFS(chain), F_EDICT, FFL_NOSPAWN},

	{"prethink", FOFS(prethink), F_FUNCTION, FFL_NOSPAWN},
	{"think", FOFS(think), F_FUNCTION, FFL_NOSPAWN},
	{"postthink", FOFS(postthink), F_FUNCTION, FFL_NOSPAWN}, // Knightmare added
	{"blocked", FOFS(blocked), F_FUNCTION, FFL_NOSPAWN},
	{"touch", FOFS(touch), F_FUNCTION, FFL_NOSPAWN},
	{"use", FOFS(use), F_FUNCTION, FFL_NOSPAWN},
	{"pain", FOFS(pain), F_FUNCTION, FFL_NOSPAWN},
	{"die", FOFS(die), F_FUNCTION, FFL_NOSPAWN},

	{"stand", FOFS(monsterinfo.stand), F_FUNCTION, FFL_NOSPAWN},
	{"idle", FOFS(monsterinfo.idle), F_FUNCTION, FFL_NOSPAWN},
	{"search", FOFS(monsterinfo.search), F_FUNCTION, FFL_NOSPAWN},
	{"walk", FOFS(monsterinfo.walk), F_FUNCTION, FFL_NOSPAWN},
	{"run", FOFS(monsterinfo.run), F_FUNCTION, FFL_NOSPAWN},
	{"dodge", FOFS(monsterinfo.dodge), F_FUNCTION, FFL_NOSPAWN},
	{"attack", FOFS(monsterinfo.attack), F_FUNCTION, FFL_NOSPAWN},
	{"melee", FOFS(monsterinfo.melee), F_FUNCTION, FFL_NOSPAWN},
	{"sight", FOFS(monsterinfo.sight), F_FUNCTION, FFL_NOSPAWN},
	{"jump", FOFS(monsterinfo.jump), F_FUNCTION, FFL_NOSPAWN},
	{"checkattack", FOFS(monsterinfo.checkattack), F_FUNCTION, FFL_NOSPAWN},
	{"currentmove", FOFS(monsterinfo.currentmove), F_MMOVE, FFL_NOSPAWN},

	{"endfunc", FOFS(moveinfo.endfunc), F_FUNCTION, FFL_NOSPAWN},

	// temp spawn vars -- only valid when the spawn function is called
	{"lip", STOFS(lip), F_INT, FFL_SPAWNTEMP},
	{"distance", STOFS(distance), F_INT, FFL_SPAWNTEMP},
	{"height", STOFS(height), F_INT, FFL_SPAWNTEMP},
	{"noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP},
	{"pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP},
	{"phase", STOFS(phase), F_FLOAT, FFL_SPAWNTEMP},
	{"item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP},
	{"shift", STOFS(shift), F_FLOAT, FFL_SPAWNTEMP},

//need for item field in edict struct, FFL_SPAWNTEMP item will be skipped on saves
	{"item", FOFS(item), F_ITEM},

	{"gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP},
	{"sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP},
	{"skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP},
	{"skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP},
	{"minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP},
#ifdef KMQUAKE2_ENGINE_MOD
	{"salpha", FOFS(s.alpha), F_FLOAT}, // Knightmare- hack for setting alpha
#endif
	{"musictrack", FOFS(musictrack), F_LSTRING},
	//Knightmare- movetype backup
	{"oldmovetype", FOFS(oldmovetype), F_INT},
	{"relative_velocity", FOFS(relative_velocity), F_VECTOR}, 	//relative velocity
	{"relative_avelocity", FOFS(relative_avelocity), F_VECTOR}, //relative angular velocity
	{"oldvelocity", FOFS(oldvelocity), F_VECTOR}, //relative angular velocity

	// Lazarus additions
	{"actor_current_weapon", FOFS(actor_current_weapon), F_INT},
	{"aiflags", FOFS(monsterinfo.aiflags), F_INT},
	{"alpha", FOFS(alpha), F_FLOAT},
	{"axis", FOFS(axis), F_INT},
	{"base_radius", FOFS(base_radius), F_FLOAT},
	{"bleft", FOFS(bleft), F_VECTOR},
	{"blood_type", FOFS(blood_type), F_INT},
	{"bob", FOFS(bob), F_FLOAT},
	{"bobframe", FOFS(bobframe), F_INT},
	{"busy", FOFS(busy), F_INT},
	{"child", FOFS(child), F_EDICT},
	{"class_id", FOFS(class_id), F_INT},
	{"color", FOFS(color), F_VECTOR},
	{"crane_beam", FOFS(crane_beam), F_EDICT, FFL_NOSPAWN},
	{"crane_bonk", FOFS(crane_bonk), F_VECTOR},
	{"crane_cable", FOFS(crane_cable), F_EDICT, FFL_NOSPAWN},
	{"crane_cargo", FOFS(crane_cargo), F_EDICT, FFL_NOSPAWN},
	{"crane_control", FOFS(crane_control), F_EDICT, FFL_NOSPAWN},
	{"crane_dir", FOFS(crane_dir), F_INT},
	{"crane_hoist", FOFS(crane_hoist), F_EDICT, FFL_NOSPAWN},
	{"crane_hook", FOFS(crane_hook), F_EDICT, FFL_NOSPAWN},
	{"crane_increment", FOFS(crane_increment), F_INT},
	{"crane_light", FOFS(crane_light), F_EDICT, FFL_NOSPAWN},
	{"crane_onboard_control", FOFS(crane_onboard_control), F_EDICT, FFL_NOSPAWN},
	{"datafile", FOFS(datafile), F_LSTRING},
	{"deadflag", FOFS(deadflag), F_INT},
	{"show_hostile", FOFS(show_hostile), F_INT}, // Knightmare added
	{"powerarmor_time", FOFS(powerarmor_time), F_FLOAT}, // Knightmare added
	{"density", FOFS(density), F_FLOAT},
	{"destroytarget", FOFS(destroytarget), F_LSTRING},
	{"dmgteam", FOFS(dmgteam), F_LSTRING},
	{"do_not_rotate", FOFS(do_not_rotate), F_INT},
	{"duration", FOFS(duration), F_FLOAT},
	{"effects", FOFS(effects), F_INT},
	{"fadein", FOFS(fadein), F_FLOAT},
	{"fadeout", FOFS(fadeout), F_FLOAT},
	{"flies", FOFS(monsterinfo.flies), F_FLOAT},
	{"fog_color", FOFS(fog_color), F_VECTOR},
	{"fog_density", FOFS(fog_density), F_FLOAT},
	{"fog_far", FOFS(fog_far), F_FLOAT},
	{"fog_model", FOFS(fog_model), F_INT},
	{"fog_near", FOFS(fog_near), F_FLOAT},
	{"fogclip", FOFS(fogclip), F_INT},
	{"followtarget", FOFS(followtarget), F_LSTRING},
	{"frame", FOFS(s.frame), F_INT},
	{"framenumbers", FOFS(framenumbers), F_INT},
	{"gib_health", FOFS(gib_health), F_INT},
	{"gib_type", FOFS(gib_type), F_INT},
	{"health2", FOFS(health2), F_INT},
	{"holdtime", FOFS(holdtime), F_FLOAT},
	{"id", FOFS(id), F_INT},
	{"idle_noise", FOFS(idle_noise), F_LSTRING},
	{"jumpdn", FOFS(monsterinfo.jumpdn), F_FLOAT},
	{"jumpup", FOFS(monsterinfo.jumpup), F_FLOAT},
	{"mass2", FOFS(mass2), F_INT},
	{"max_health", FOFS(max_health), F_INT},
	{"max_range", FOFS(monsterinfo.max_range), F_FLOAT},
	{"moreflags", FOFS(moreflags), F_INT},
	{"movewith", FOFS(movewith), F_LSTRING},
	{"movewith_ent", FOFS(movewith_ent), F_EDICT},
	{"movewith_next", FOFS(movewith_next), F_EDICT},
	{"movewith_offset", FOFS(movewith_offset), F_VECTOR},
	{"move_to", FOFS(move_to), F_LSTRING},
	{"muzzle", FOFS(muzzle), F_VECTOR},
	{"muzzle2", FOFS(muzzle2), F_VECTOR},
	{"newtargetname", FOFS(newtargetname), F_LSTRING},
	{"next_grenade", FOFS(next_grenade), F_EDICT, FFL_NOSPAWN},
	{"origin_offset", FOFS(origin_offset), F_VECTOR},
	{"offset", FOFS(offset), F_VECTOR},
	{"org_maxs", FOFS(org_maxs), F_VECTOR},
	{"org_mins", FOFS(org_mins), F_VECTOR},
	{"org_size", FOFS(org_size), F_VECTOR},
	{"owner_id", FOFS(owner_id), F_INT},
	{"parent_attach_angles", FOFS(parent_attach_angles), F_VECTOR},
	{"pitch_speed", FOFS(pitch_speed), F_FLOAT},
	{"powerarmor", FOFS(powerarmor), F_INT},
	{"prev_grenade", FOFS(prev_grenade), F_EDICT, FFL_NOSPAWN},
	{"prevpath", FOFS(prevpath), F_EDICT},
	{"radius", FOFS(radius), F_FLOAT},
	{"renderfx", FOFS(renderfx), F_INT},
	{"roll", FOFS(roll), F_FLOAT},
	{"roll_speed", FOFS(roll_speed), F_FLOAT},
	{"skinnum", FOFS(s.skinnum), F_INT},
	{"speaker", FOFS(speaker), F_EDICT, FFL_NOSPAWN},
	{"smooth_movement", FOFS(smooth_movement), F_INT},
	{"solidstate", FOFS(solidstate), F_INT},
	{"source", FOFS(source), F_LSTRING},
	{"startframe", FOFS(startframe), F_INT},
	{"target2", FOFS(target2), F_LSTRING},
	{"tright", FOFS(tright), F_VECTOR},
	{"turn_rider", FOFS(turn_rider), F_INT},
	{"turret", FOFS(turret), F_EDICT},
	{"usermodel", FOFS(usermodel), F_LSTRING},
	{"vehicle", FOFS(vehicle), F_EDICT, FFL_NOSPAWN},
	{"viewer", FOFS(viewer), F_EDICT},
	{"viewheight", FOFS(viewheight), F_INT},
	{"viewmessage", FOFS(viewmessage), F_LSTRING},
	{"yaw_speed", FOFS(yaw_speed), F_FLOAT},

	{"crosshair", FOFS(crosshair), F_EDICT},
	{"from", FOFS(from), F_EDICT},
	{"to", FOFS(to), F_EDICT},
	{"flash", FOFS(flash), F_EDICT},
	// FIXME: how to save 6-part reflection field?

	// fields added by Rogue mission pack
	{"bad_area", FOFS(bad_area), F_EDICT},
	{"hint_chain", FOFS(hint_chain), F_EDICT},
	{"monster_hint_chain", FOFS(monster_hint_chain), F_EDICT},
	{"target_hint_chain", FOFS(target_hint_chain), F_EDICT},
	{"goal_hint", FOFS(monsterinfo.goal_hint), F_EDICT},
	{"badMedic1", FOFS(monsterinfo.badMedic1), F_EDICT},
	{"badMedic2", FOFS(monsterinfo.badMedic2), F_EDICT},
	{"last_player_enemy", FOFS(monsterinfo.last_player_enemy), F_EDICT},
	{"commander", FOFS(monsterinfo.commander), F_EDICT},
	{"blocked", FOFS(monsterinfo.blocked), F_MMOVE, FFL_NOSPAWN},
	{"duck", FOFS(monsterinfo.duck), F_MMOVE, FFL_NOSPAWN},
	{"unduck", FOFS(monsterinfo.unduck), F_MMOVE, FFL_NOSPAWN},
	{"sidestep", FOFS(monsterinfo.sidestep), F_MMOVE, FFL_NOSPAWN},

// ACEBOT_ADD
	{"is_bot", FOFS(is_bot), F_INT},
	{"is_jumping", FOFS(is_jumping), F_INT},
	{"move_vector", FOFS(move_vector), F_VECTOR},
	{"next_move_time", FOFS(next_move_time), F_FLOAT},
	{"wander_timeout", FOFS(wander_timeout), F_FLOAT},
	{"suicide_timeout", FOFS(suicide_timeout), F_FLOAT},
	{"current_node", FOFS(current_node), F_INT},
	{"goal_node", FOFS(goal_node), F_INT},
	{"next_node", FOFS(next_node), F_INT},
	{"node_timeout", FOFS(node_timeout), F_INT},
	{"last_node", FOFS(last_node), F_INT},
	{"tries", FOFS(tries), F_INT},
	{"state", FOFS(state), F_INT},
// ACEBOT_END

	{0, 0, 0, 0}

};

field_t		levelfields[] =
{
	{"changemap", LLOFS(changemap), F_LSTRING},
	{"sight_client", LLOFS(sight_client), F_EDICT},
	{"sight_entity", LLOFS(sight_entity), F_EDICT},
	{"sound_entity", LLOFS(sound_entity), F_EDICT},
	{"sound2_entity", LLOFS(sound2_entity), F_EDICT},
	{"disguise_violator", LLOFS(disguise_violator), F_EDICT},

	{NULL, 0, F_INT}
};

field_t		clientfields[] =
{
	{"pers.weapon", CLOFS(pers.weapon), F_ITEM},
	{"pers.lastweapon", CLOFS(pers.lastweapon), F_ITEM},
	{"newweapon", CLOFS(newweapon), F_ITEM},
	{"chasecam", CLOFS(chasecam), F_EDICT},
	{"oldplayer", CLOFS(oldplayer), F_EDICT},

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
void InitGame (void)
{
	gi.dprintf ("==== InitGame (Lazarus) ====\n");
	gi.dprintf("by Mr. Hyde & Mad Dog\ne-mail: rascal@vicksburg.com\n\n");

	// Knightmare- init cvars
	lithium_defaults();

	gun_x = gi.cvar ("gun_x", "0", 0);
	gun_y = gi.cvar ("gun_y", "0", 0);
	gun_z = gi.cvar ("gun_z", "0", 0);

	//FIXME: sv_ prefix is wrong for these
	sv_rollspeed = gi.cvar ("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar ("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar ("sv_maxvelocity", "2000", 0);
	sv_gravity = gi.cvar ("sv_gravity", "800", 0);

	// noset vars
	dedicated = gi.cvar ("dedicated", "0", CVAR_NOSET);

	// latched vars
	sv_cheats = gi.cvar ("cheats", "0", CVAR_SERVERINFO|CVAR_LATCH);
	gi.cvar ("gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar ("gamedate", __DATE__ , CVAR_SERVERINFO | CVAR_LATCH);

	maxclients = gi.cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	maxspectators = gi.cvar ("maxspectators", "4", CVAR_SERVERINFO);
	deathmatch = gi.cvar ("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar ("coop", "0", CVAR_LATCH);
	skill = gi.cvar ("skill", "1", CVAR_LATCH);
	commentary = gi.cvar ("commentary", "0", CVAR_LATCH);

	// Knightmare- increase maxentities
	//maxentities = gi.cvar ("maxentities", "1024", CVAR_LATCH);
	maxentities = gi.cvar ("maxentities", va("%i",MAX_EDICTS), CVAR_LATCH);

	// change anytime vars
	dmflags = gi.cvar ("dmflags", "0", CVAR_SERVERINFO);
	fraglimit = gi.cvar ("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar ("timelimit", "0", CVAR_SERVERINFO);
//ZOID
	capturelimit = gi.cvar ("capturelimit", "0", CVAR_SERVERINFO);
	instantweap = gi.cvar ("instantweap", "0", CVAR_SERVERINFO);
//ZOID
	password = gi.cvar ("password", "", CVAR_USERINFO);
	spectator_password = gi.cvar ("spectator_password", "", CVAR_USERINFO);
	needpass = gi.cvar ("needpass", "0", CVAR_SERVERINFO);
	filterban = gi.cvar ("filterban", "1", 0);

	g_select_empty = gi.cvar ("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar ("run_pitch", "0.002", 0);
	run_roll = gi.cvar ("run_roll", "0.005", 0);
	bob_up  = gi.cvar ("bob_up", "0.005", 0);
	bob_pitch = gi.cvar ("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar ("bob_roll", "0.002", 0);

	// flood control
	flood_msgs = gi.cvar ("flood_msgs", "4", 0);
	flood_persecond = gi.cvar ("flood_persecond", "4", 0);
	flood_waitdelay = gi.cvar ("flood_waitdelay", "10", 0);

	// dm map list
	sv_maplist = gi.cvar ("sv_maplist", "", 0);

	// Lazarus
	actorchicken = gi.cvar("actorchicken", "1", CVAR_SERVERINFO|CVAR_LATCH);
	actorjump = gi.cvar("actorjump", "1", CVAR_SERVERINFO|CVAR_LATCH);
	actorscram = gi.cvar("actorscram", "1", CVAR_SERVERINFO|CVAR_LATCH);
	alert_sounds = gi.cvar("alert_sounds", "0", CVAR_SERVERINFO|CVAR_LATCH);
	allow_fog = gi.cvar ("allow_fog", "1", CVAR_ARCHIVE);

	// set to 0 to bypass target_changelevel clear inventory flag
	// because some user maps have this erroneously set
	allow_clear_inventory = gi.cvar ("allow_clear_inventory", "1", CVAR_ARCHIVE);

	cd_loopcount = gi.cvar("cd_loopcount","4",0);
	cl_gun = gi.cvar("cl_gun", "1", 0);
	cl_3dcam = gi.cvar("cl_3dcam", "0", 0); // Knightmare added
	corpse_fade = gi.cvar("corpse_fade", "0", CVAR_SERVERINFO|CVAR_LATCH);
	corpse_fadetime = gi.cvar("corpse_fadetime", "20", 0);
	crosshair = gi.cvar("crosshair", "1", 0);
	footstep_sounds = gi.cvar("footstep_sounds", "0", CVAR_SERVERINFO|CVAR_LATCH);
	fov = gi.cvar("fov", "90", 0);
	hand = gi.cvar("hand", "0", 0);
	jetpack_weenie = gi.cvar("jetpack_weenie", "0", CVAR_SERVERINFO);
	joy_pitchsensitivity = gi.cvar("joy_pitchsensitivity", "1", 0);
	joy_yawsensitivity = gi.cvar("joy_yawsensitivity", "-1", 0);
	jump_kick = gi.cvar("jump_kick", "0", CVAR_SERVERINFO|CVAR_LATCH);
	lights = gi.cvar("lights", "1", 0);
	lightsmin = gi.cvar("lightsmin", "a", CVAR_SERVERINFO);
	m_pitch = gi.cvar("m_pitch", "0.022", 0);
	m_yaw = gi.cvar("m_yaw", "0.022", 0);
	monsterjump = gi.cvar("monsterjump", "1", CVAR_SERVERINFO|CVAR_LATCH);
	rocket_strafe = gi.cvar("rocket_strafe", "0", 0);
	s_primary = gi.cvar("s_primary", "0", 0);
#ifdef KMQUAKE2_ENGINE_MOD
	sv_maxgibs = gi.cvar("sv_maxgibs", "160", CVAR_SERVERINFO);
#else
	sv_maxgibs = gi.cvar("sv_maxgibs", "20", CVAR_SERVERINFO);
#endif
	turn_rider = gi.cvar("turn_rider", "1", CVAR_SERVERINFO);
	zoomrate = gi.cvar("zoomrate", "80", CVAR_ARCHIVE);
	zoomsnap = gi.cvar("zoomsnap", "20", CVAR_ARCHIVE);

	// shift_ and rotate_distance only used for debugging stuff - this is the distance
	// an entity will be moved by "item_left", "item_right", etc.
	shift_distance = gi.cvar("shift_distance", "1", CVAR_SERVERINFO);
	rotate_distance = gi.cvar("rotate_distance", "1", CVAR_SERVERINFO);

	// GL stuff
	gl_clear = gi.cvar("gl_clear", "0", 0);

	// Lazarus saved cvars that we may or may not manipulate, but need to
	// restore to original values upon map exit.
	lazarus_cd_loop = gi.cvar("lazarus_cd_loop", "0", 0);
	lazarus_gl_clear= gi.cvar("lazarus_gl_clear","0", 0);
	lazarus_pitch   = gi.cvar("lazarus_pitch",   "0", 0);
	lazarus_yaw     = gi.cvar("lazarus_yaw",     "0", 0);
	lazarus_joyp    = gi.cvar("lazarus_joyp",    "0", 0);
	lazarus_joyy    = gi.cvar("lazarus_joyy",    "0", 0);
	lazarus_cl_gun  = gi.cvar("lazarus_cl_gun",  "0", 0);
	lazarus_crosshair = gi.cvar("lazarus_crosshair", "0", 0);

	/*if(lazarus_gl_clear->value)
		gi.cvar_forceset("gl_clear",         va("%d",lazarus_gl_clear->value));
	else
		gi.cvar_forceset("lazarus_gl_clear", va("%d",gl_clear->value));*/

	if(!deathmatch->value && !coop->value)
	{
		/*if(lazarus_pitch->value) {
			gi.cvar_forceset("cd_loopcount",         va("%d",(int)(lazarus_cd_loop->value)));
			gi.cvar_forceset("m_pitch",              va("%f",lazarus_pitch->value));
			gi.cvar_forceset("m_yaw",                va("%f",lazarus_yaw->value));
			gi.cvar_forceset("cl_gun",               va("%d",(int)(lazarus_cl_gun->value)));
			gi.cvar_forceset("crosshair",            va("%d",(int)(lazarus_crosshair->value)));
		} else {*/
			gi.cvar_forceset("lazarus_cd_loop",        va("%d",(int)(cd_loopcount->value)));
#ifndef KMQUAKE2_ENGINE_MOD // engine has zoom autosensitivity
			gi.cvar_forceset("lazarus_pitch",          va("%f",m_pitch->value));
			gi.cvar_forceset("lazarus_yaw",            va("%f",m_yaw->value));
			gi.cvar_forceset("lazarus_joyp",           va("%f",joy_pitchsensitivity->value));
			gi.cvar_forceset("lazarus_joyy",           va("%f",joy_yawsensitivity->value));
#endif
			gi.cvar_forceset("lazarus_cl_gun",         va("%d",(int)(cl_gun->value)));
			gi.cvar_forceset("lazarus_crosshair",      va("%d",(int)(crosshair->value)));
		//}
	}

	tpp = gi.cvar ("tpp", "0", CVAR_ARCHIVE);
	tpp_auto = gi.cvar ("tpp_auto", "1", 0);
	crossh = gi.cvar ("crossh", "1", 0);
	allow_download = gi.cvar("allow_download", "0", 0);

	blaster_color = gi.cvar("blaster_color", "1", 0); // Knightmare added

	// If this is an SP game and "readout" is not set, force allow_download off
	// so we don't get the annoying "Refusing to download path with .." messages
	// due to misc_actor sounds.
	if(allow_download->value && !readout->value && !deathmatch->value)
		gi.cvar_forceset("allow_download", "0");

	bounce_bounce = gi.cvar("bounce_bounce", "0.5", 0);
	bounce_minv   = gi.cvar("bounce_minv",   "60",  0);

	// items
	InitItems ();

	Com_sprintf (game.helpmessage1, sizeof(game.helpmessage1), "");

	Com_sprintf (game.helpmessage2, sizeof(game.helpmessage2), "");

	// initialize all entities for this game
	game.maxentities = maxentities->value;
	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// initialize all clients for this game
	game.maxclients = maxclients->value;
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients+1;

//ZOID
	CTFInit();
//ZOID
}

//=========================================================

void WriteField1 (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	if (field->flags & FFL_SPAWNTEMP)
		return;

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
		if ( *(char **)p )
			len = strlen(*(char **)p) + 1;
		else
			len = 0;
		*(int *)p = len;
		break;
	case F_EDICT:
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(edict_t **)p - g_edicts;
		*(int *)p = index;
		break;
	case F_CLIENT:
		if ( *(gclient_t **)p == NULL)
			index = -1;
		else
			index = *(gclient_t **)p - game.clients;
		*(int *)p = index;
		break;
	case F_ITEM:
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(gitem_t **)p - itemlist;
		*(int *)p = index;
		break;

	//relative to code segment
	case F_FUNCTION:
		if (*(byte **)p == NULL)
			index = 0;
		else
			index = *(byte **)p - ((byte *)InitGame);
		*(int *)p = index;
		break;

	//relative to data segment
	case F_MMOVE:
		if (*(byte **)p == NULL)
			index = 0;
		else
			index = *(byte **)p - (byte *)&mmove_reloc;
		*(int *)p = index;
		break;

	default:
		gi.error ("WriteEdict: unknown field type");
	}
}


void WriteField2 (FILE *f, field_t *field, byte *base)
{
	int			len;
	void		*p;

	if (field->flags & FFL_SPAWNTEMP)
		return;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_LSTRING:
		if ( *(char **)p )
		{
			len = strlen(*(char **)p) + 1;
			fwrite (*(char **)p, len, 1, f);
		}
		break;
	}
}

void ReadField (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	if (field->flags & FFL_SPAWNTEMP)
		return;

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

	//relative to code segment
	case F_FUNCTION:
		index = *(int *)p;
		if ( index == 0 )
			*(byte **)p = NULL;
		else
			*(byte **)p = ((byte *)InitGame) + index;
		break;

	//relative to data segment
	case F_MMOVE:
		index = *(int *)p;
		if (index == 0)
			*(byte **)p = NULL;
		else
			*(byte **)p = (byte *)&mmove_reloc + index;
		break;

	default:
		gi.error ("ReadEdict: unknown field type");
	}
}

//=========================================================

/*
==============
WriteClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteClient (FILE *f, gclient_t *client)
{
	field_t		*field;
	gclient_t	temp;
	
	// all of the ints, floats, and vectors stay as they are
	temp = *client;

	// change the pointers to lengths or indexes
	for (field=clientfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=clientfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)client);
	}
}

/*
==============
ReadClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadClient (FILE *f, gclient_t *client)
{
	field_t		*field;

	fread (client, sizeof(*client), 1, f);

	client->pers.spawn_landmark = false;
	client->pers.spawn_levelchange = false;
	for (field=clientfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)client);
	}
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
void WriteGame (char *filename, qboolean autosave)
{
	FILE	*f;
	int		i;
	char	str[16];

	if(developer->value)
		gi.dprintf ("==== WriteGame ====\n");

	if (!autosave)
		game.transition_ents = 0;

	if (!autosave)
		SaveClientData ();

	f = fopen (filename, "wb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	memset (str, 0, sizeof(str));
	strcpy (str, __DATE__);
	fwrite (str, sizeof(str), 1, f);

	game.autosaved = autosave;
	fwrite (&game, sizeof(game), 1, f);
	game.autosaved = false;

	for (i=0 ; i<game.maxclients ; i++)
		WriteClient (f, &game.clients[i]);

	fclose (f);
}

void ReadGame (char *filename)
{
	FILE	*f;
	int		i;
	char	str[16];

	if(developer->value)
		gi.dprintf ("==== ReadGame ====\n");

	gi.FreeTags (TAG_GAME);

	f = fopen (filename, "rb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	fread (str, sizeof(str), 1, f);
	if (strcmp (str, __DATE__))
	{
		fclose (f);
		gi.error ("Savegame from an older version.\n");
	}

	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;

	fread (&game, sizeof(game), 1, f);
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	for (i=0 ; i<game.maxclients ; i++)
		ReadClient (f, &game.clients[i]);
	fclose (f);
}

//==========================================================


/*
==============
WriteEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteEdict (FILE *f, edict_t *ent)
{
	field_t		*field;
	edict_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = *ent;

	// change the pointers to lengths or indexes
	for (field=fields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=fields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)ent);
	}

}

/*
==============
WriteLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteLevelLocals (FILE *f)
{
	field_t		*field;
	level_locals_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = level;

	// change the pointers to lengths or indexes
	for (field=levelfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=levelfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)&level);
	}
}


/*
==============
ReadEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadEdict (FILE *f, edict_t *ent)
{
	field_t		*field;
	int		i; // Knightmare added

	fread (ent, sizeof(*ent), 1, f);

	for (field=fields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)ent);
	}
	// Knightmare- nullify reflection pointers to prevent crash
	for (i=0; i<6; i++)
		ent->reflection[i] = NULL;
}

/*
==============
ReadLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadLevelLocals (FILE *f)
{
	field_t		*field;

	fread (&level, sizeof(level), 1, f);

	for (field=levelfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)&level);
	}
}

/*
=================
WriteLevel

=================
*/
void WriteLevel (char *filename)
{
	int		i;
	edict_t	*ent;
	FILE	*f;
	void	*base;

	if(developer->value)
		gi.dprintf ("==== WriteLevel ====\n");

	f = fopen (filename, "wb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	// write out edict size for checking
	i = sizeof(edict_t);
	fwrite (&i, sizeof(i), 1, f);

	// write out a function pointer for checking
	base = (void *)InitGame;
	fwrite (&base, sizeof(base), 1, f);

	// write out level_locals_t
	WriteLevelLocals (f);

	// write out all the entities
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];
		if (!ent->inuse)
			continue;
		// Knightmare- don't save reflections
		if (ent->flags & FL_REFLECT)
			continue;
		fwrite (&i, sizeof(i), 1, f);
		WriteEdict (f, ent);
	}
	i = -1;
	fwrite (&i, sizeof(i), 1, f);

	fclose (f);
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
void LoadTransitionEnts();
void ReadLevel (char *filename)
{
	int		entnum;
	FILE	*f;
	int		i;
	void	*base;
	edict_t	*ent;

	if(developer->value)
		gi.dprintf ("==== ReadLevel ====\n");

	f = fopen (filename, "rb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	// free any dynamic memory allocated by loading the level
	// base state
	gi.FreeTags (TAG_LEVEL);

	// wipe all the entities
	memset (g_edicts, 0, game.maxentities*sizeof(g_edicts[0]));
	globals.num_edicts = maxclients->value+1;

	// check edict size
	fread (&i, sizeof(i), 1, f);
	if (i != sizeof(edict_t))
	{
		fclose (f);
		gi.error ("ReadLevel: mismatched edict size");
	}

	// check function pointer base address
	fread (&base, sizeof(base), 1, f);

/*	Lazarus: The __DATE__ check in ReadGame is sufficient for a version
	         check. The following is reported to fail under some 
	         circumstances (though I've never seen it).

#ifdef _WIN32
	if (base != (void *)InitGame)
	{
		fclose (f);
		gi.error ("ReadLevel: function pointers have moved");
	}
#else
	gi.dprintf("Function offsets %d\n", ((byte *)base) - ((byte *)InitGame));
#endif

*/

	// load the level locals
	ReadLevelLocals (f);

	// load all the entities
	while (1)
	{
		if (fread (&entnum, sizeof(entnum), 1, f) != 1)
		{
			fclose (f);
			gi.error ("ReadLevel: failed to read entnum");
		}
		if (entnum == -1)
			break;
		if (entnum >= globals.num_edicts)
			globals.num_edicts = entnum+1;

		ent = &g_edicts[entnum];
		ReadEdict (f, ent);

		// let the server rebuild world links for this ent
		memset (&ent->area, 0, sizeof(ent->area));
		gi.linkentity (ent);
	}

	fclose (f);

	// mark all clients as unconnected
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = &g_edicts[i+1];
		ent->client = game.clients + i;
		ent->client->pers.connected = false;
	}

	// do any load time things at this point
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
			continue;

		// fire any cross-level triggers
		if (ent->classname)
			if (strcmp(ent->classname, "target_crosslevel_target") == 0)
				ent->nextthink = level.time + ent->delay;
	}

	// DWH: Load transition entities
	if(game.transition_ents)
	{
		LoadTransitionEnts();
		actor_files();
	}
}
