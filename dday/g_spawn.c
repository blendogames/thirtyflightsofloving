/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_spawn.c,v $
 *   $Revision: 1.14 $
 *   $Date: 2002/06/04 19:49:47 $
 *
 ***********************************

Copyright (C) 2002 Vipersoft

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

//Ok, since we are modifying this file, we might as well declare the
//item spawning functions here. These are the functions that actually
//cause the item to be spawned. By convention, the actual spawning
//function is defined wherever the entity (or item in our case) is
//defined.
//We are actually having to do two things. We need to manage the itemlist[]
//and we need to manage the spawns[] lists. When a level is brought up, the
//first thing that is does is run through the list of things in it and spawns
//them.


void SP_item_armor_body(edict_t *self);
void SP_item_armor_combat(edict_t *self);
void SP_item_armor_jacket(edict_t *self);
void SP_item_armor_shard(edict_t *self);

//weapons
void SP_item_ammo_grenades(edict_t *self);
void SP_item_weapon_flamethrower(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_flamethrower"));
}

//ammo
void SP_item_ammo_napalm(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_napalm"));
}

void SP_item_powerup_silencer(edict_t *self);

void SP_item_powerup_breather(edict_t *self);

void SP_item_powerup_enviro(edict_t *self);
void SP_item_powerup_adrenaline(edict_t *self);
void SP_item_powerup_pack(edict_t *self);


//end of added item spawn functions

void SP_item_health (edict_t *self);
void SP_item_health_small (edict_t *self);
void SP_item_health_large (edict_t *self);
void SP_item_health_mega (edict_t *self);

void SP_info_player_start (edict_t *ent);
void SP_info_player_deathmatch (edict_t *ent);
void SP_info_player_coop (edict_t *ent);
void SP_info_player_intermission (edict_t *ent);

void SP_func_plat (edict_t *ent);
void SP_func_rotating (edict_t *ent);
void SP_func_button (edict_t *ent);
void SP_func_door (edict_t *ent);
void SP_func_door_secret (edict_t *ent);
void SP_func_door_rotating (edict_t *ent);
void SP_func_water (edict_t *ent);
void SP_func_train (edict_t *ent);
void SP_func_conveyor (edict_t *self);
void SP_func_wall (edict_t *self);
void SP_func_object (edict_t *self);
void SP_func_explosive (edict_t *self);
void SP_func_timer (edict_t *self);
void SP_func_areaportal (edict_t *ent);
void SP_func_clock (edict_t *ent);
void SP_func_killbox (edict_t *ent);

void SP_trigger_always (edict_t *ent);
void SP_trigger_once (edict_t *ent);
void SP_trigger_multiple (edict_t *ent);
void SP_trigger_relay (edict_t *ent);
void SP_trigger_push (edict_t *ent);
void SP_trigger_hurt (edict_t *ent);
void SP_trigger_key (edict_t *ent);
void SP_trigger_counter (edict_t *ent);
void SP_trigger_elevator (edict_t *ent);
void SP_trigger_gravity (edict_t *ent);
void SP_trigger_monsterjump (edict_t *ent);

//added by kmm
void SP_info_team_start(edict_t *ent);
void SP_info_reinforcement_start(edict_t *ent);
void SP_info_reinforcements_nearest(edict_t *ent);
void SP_trigger_enough_troops(edict_t *ent);
void SP_target_objective(edict_t *ent);
//void SP_info_Max_MOS(edict_t *ent);
//void SP_info_Skin(edict_t *ent);
//void SP_info_Air_Battery(edict_t *ent);
//void SP_info_Arty_Battery(edict_t *ent);
//void SP_event_Arty_Strike(edict_t *ent);
void bot_spawn (edict_t *ent);

void SP_info_Infantry_Start(edict_t *ent);
void SP_info_L_Gunner_Start(edict_t *ent);
void SP_info_H_Gunner_Start(edict_t *ent);
void SP_info_Sniper_Start(edict_t *ent);
void SP_info_Engineer_Start(edict_t *ent);
void SP_info_Medic_Start(edict_t *ent);
void SP_info_Flamethrower_Start(edict_t *ent);
void SP_info_Special_Start(edict_t *ent);
void SP_info_Officer_Start(edict_t *ent);

void SP_target_temp_entity (edict_t *ent);
void SP_target_speaker (edict_t *ent);
void SP_target_explosion (edict_t *ent);
void SP_target_changelevel (edict_t *ent);
void SP_target_secret (edict_t *ent);
void SP_target_goal (edict_t *ent);
void SP_target_splash (edict_t *ent);
void SP_target_spawner (edict_t *ent);
void SP_target_blaster (edict_t *ent);
void SP_target_crosslevel_trigger (edict_t *ent);
void SP_target_crosslevel_target (edict_t *ent);
void SP_target_laser (edict_t *self);
void SP_target_help (edict_t *ent);
void SP_target_actor (edict_t *ent);
void SP_target_lightramp (edict_t *self);
void SP_target_earthquake (edict_t *ent);
void SP_target_character (edict_t *ent);
void SP_target_string (edict_t *ent);

void SP_worldspawn (edict_t *ent);
void SP_viewthing (edict_t *ent);

void SP_light (edict_t *self);
void SP_light_mine1 (edict_t *ent);
void SP_light_mine2 (edict_t *ent);
void SP_info_null (edict_t *self);
void SP_info_notnull (edict_t *self);
void SP_path_corner (edict_t *self);
void SP_point_combat (edict_t *self);

void SP_misc_explobox (edict_t *self);
void SP_misc_banner (edict_t *self);
//bcass start - banner thing
void SP_misc_banner_x (edict_t *self);
void SP_misc_banner_1 (edict_t *self);
void SP_misc_banner_2 (edict_t *self);
void SP_misc_banner_3 (edict_t *self);
void SP_misc_banner_4 (edict_t *self);
//bcass end
//bcass start - skeleton model thing
void SP_misc_skeleton (edict_t *self);
//bcass end
void SP_misc_md2 (edict_t *self);
void SP_misc_crate (edict_t *self);
void SP_misc_satellite_dish (edict_t *self);
void SP_misc_actor (edict_t *self);
void SP_misc_gib_arm (edict_t *self);
void SP_misc_gib_leg (edict_t *self);
void SP_misc_gib_head (edict_t *self);
void SP_misc_insane (edict_t *self);
void SP_misc_civilian (edict_t *self);
void SP_misc_deadsoldier (edict_t *self);
void SP_misc_viper (edict_t *self);
void SP_misc_viper_bomb (edict_t *self);
void SP_misc_bigviper (edict_t *self);
void SP_misc_strogg_ship (edict_t *self);
void SP_misc_teleporter (edict_t *self);
void SP_misc_teleporter_dest (edict_t *self);
void SP_misc_blackhole (edict_t *self);
void SP_misc_eastertank (edict_t *self);
void SP_misc_easterchick (edict_t *self);
void SP_misc_easterchick2 (edict_t *self);

void SP_turret_breach (edict_t *self);
void SP_turret_base (edict_t *self);

void SP_turret_range (edict_t *self);

void SP_info_Mission_Results (edict_t *ent);
void SP_turret_driver (edict_t *self);
//void SP_tank_Tank1 (edict_t *self); // for DDAY tanks (pbowens)

// Objectives courtesy of BinaryCowboy
void SP_objective_area(edict_t *self);
void SP_objective_touch(edict_t *self);
void SP_timed_objective_touch(edict_t *self);
void SP_func_explosive_objective (edict_t *self);

void SP_map_location (edict_t *self);//faf
void SP_spawn_protect (edict_t *self);
void SP_airstrike (edict_t *ent);
void SP_Spawn_Toggle (edict_t *self);

void SP_item_botroam (edict_t *self);	//JABot
void SP_briefcase(edict_t *self);//faf  ctb code



spawn_t spawns[MAX_EDICTS] = {
	{"item_health", SP_item_health},
	{"item_health_small", SP_item_health_small},
	{"item_health_large", SP_item_health_large},
	{"item_health_mega", SP_item_health_mega},

	{"info_player_start", SP_info_player_start},
	{"info_player_deathmatch", SP_info_player_deathmatch},
	{"info_player_coop", SP_info_player_coop},
	{"info_player_intermission", SP_info_player_intermission},

	{"func_plat", SP_func_plat},
	{"func_button", SP_func_button},
	{"func_door", SP_func_door},
	{"func_door_secret", SP_func_door_secret},
	{"func_door_rotating", SP_func_door_rotating},
	{"func_rotating", SP_func_rotating},
	{"func_train", SP_func_train},
	{"func_water", SP_func_water},
	{"func_conveyor", SP_func_conveyor},
	{"func_areaportal", SP_func_areaportal},
	{"func_clock", SP_func_clock},
	{"func_wall", SP_func_wall},
	{"func_object", SP_func_object},
	{"func_timer", SP_func_timer},
	{"func_explosive", SP_func_explosive},
	{"func_killbox", SP_func_killbox},

	{"trigger_always", SP_trigger_always},
	{"trigger_once", SP_trigger_once},
	{"trigger_multiple", SP_trigger_multiple},
	{"trigger_relay", SP_trigger_relay},
	{"trigger_push", SP_trigger_push},
	{"trigger_hurt", SP_trigger_hurt},
	{"trigger_key", SP_trigger_key},
	{"trigger_counter", SP_trigger_counter},
	{"trigger_elevator", SP_trigger_elevator},
	{"trigger_gravity", SP_trigger_gravity},
	{"trigger_monsterjump", SP_trigger_monsterjump},

// added by kmm
	{"info_reinforcements_start",SP_info_reinforcement_start},
	{"info_team_start",SP_info_team_start},
	{"info_reinforcements_nearest",SP_info_reinforcement_start},
	{"bot_spawn",bot_spawn},

//	{"info_mos_max",SP_info_Max_MOS},
//	{"info_skin_type",SP_info_Skin},
//	{"info_arty_bat",SP_info_Arty_Battery},
//	{"info_airstrike_bat",SP_info_Air_Battery},
//	{"info_arty_strike",SP_event_Arty_Strike},
//	{"event_arty_strike",SP_event_Arty_Strike},

//	{"trigger_enough_troops", SP_trigger_enough_troops},

//	{"target_objective",SP_target_objective},
	{"info_infantry_start", SP_info_Infantry_Start},
	{"info_officer_start",SP_info_Officer_Start},
	{"info_lgunner_start",SP_info_L_Gunner_Start},
	{"info_hgunner_start",SP_info_H_Gunner_Start},
	{"info_sniper_start",SP_info_Sniper_Start},
	{"info_engineer_start",SP_info_Engineer_Start},
	{"info_medic_start",SP_info_Medic_Start},
	{"info_special_start",SP_info_Special_Start},
	{"info_flamethrower_start",SP_info_Flamethrower_Start},


/*	{"info_infantry_start", SP_info_reinforcement_start},
	{"info_officer_start",SP_info_reinforcement_start},
	{"info_lgunner_start",SP_info_reinforcement_start},
	{"info_hgunner_start",SP_info_reinforcement_start},
	{"info_sniper_start",SP_info_reinforcement_start},
	{"info_engineer_start",SP_info_reinforcement_start},
	{"info_medic_start",SP_info_reinforcement_start},
	{"info_special_start",SP_info_reinforcement_start},
	{"info_flamethrower_start",SP_info_reinforcement_start},

*/



	{"info_mission_results",SP_info_Mission_Results},
	{"info_Mission_Results",SP_info_Mission_Results},
	{"SP_info_Mission_Results",SP_info_Mission_Results},


//end add
	{"target_temp_entity", SP_target_temp_entity},
	{"target_speaker", SP_target_speaker},
	{"target_explosion", SP_target_explosion},
	{"target_changelevel", SP_target_changelevel},
	{"target_secret", SP_target_secret},
	{"target_goal", SP_target_goal},
	{"target_splash", SP_target_splash},
	{"target_spawner", SP_target_spawner},
	{"target_blaster", SP_target_blaster},
	{"target_crosslevel_trigger", SP_target_crosslevel_trigger},
	{"target_crosslevel_target", SP_target_crosslevel_target},
	{"target_laser", SP_target_laser},
	{"target_help", SP_target_help},
	{"target_actor", SP_target_actor},
	{"target_lightramp", SP_target_lightramp},
	{"target_earthquake", SP_target_earthquake},
	{"target_character", SP_target_character},
	{"target_string", SP_target_string},

	{"worldspawn", SP_worldspawn},
	{"viewthing", SP_viewthing},

	{"light", SP_light},
	{"light_mine1", SP_light_mine1},
	{"light_mine2", SP_light_mine2},
	{"info_null", SP_info_null},
	{"func_group", SP_info_null},
	{"info_notnull", SP_info_notnull},
	{"path_corner", SP_path_corner},
	{"point_combat", SP_point_combat},

	{"misc_explobox", SP_misc_explobox},
	{"misc_banner", SP_misc_banner},
//bcass start - banner thing
	{"misc_banner_x", SP_misc_banner_x},
	{"misc_banner_1", SP_misc_banner_1},
	{"misc_banner_2", SP_misc_banner_2},
	{"misc_banner_3", SP_misc_banner_3},
	{"misc_banner_4", SP_misc_banner_4},
//bcass end - banner thing
//bcass start - skeleton model thing
	{"misc_skeleton", SP_misc_skeleton},
//bcass end
	{"misc_md2", SP_misc_md2},
	{"misc_crate", SP_misc_crate},

	{"misc_satellite_dish", SP_misc_satellite_dish},
	{"misc_actor", SP_misc_actor},
	{"misc_civilian", SP_misc_civilian},
	{"misc_gib_arm", SP_misc_gib_arm},
	{"misc_gib_leg", SP_misc_gib_leg},
	{"misc_gib_head", SP_misc_gib_head},
	{"misc_insane", SP_misc_insane},
	{"misc_deadsoldier", SP_misc_deadsoldier},
	{"misc_viper", SP_misc_viper},
	{"misc_viper_bomb", SP_misc_viper_bomb},
	{"misc_bigviper", SP_misc_bigviper},
	{"misc_strogg_ship", SP_misc_strogg_ship},
	{"misc_teleporter", SP_misc_teleporter},
	{"misc_teleporter_dest", SP_misc_teleporter_dest},
	{"misc_blackhole", SP_misc_blackhole},
	{"misc_eastertank", SP_misc_eastertank},
	{"misc_easterchick", SP_misc_easterchick},
	{"misc_easterchick2", SP_misc_easterchick2},
/*
	{"monster_berserk", SP_monster_berserk},
	{"monster_gladiator", SP_monster_gladiator},
	{"monster_gunner", SP_monster_gunner},
	{"monster_infantry", SP_monster_infantry},
	{"monster_soldier_light", SP_monster_soldier_light},
	{"monster_soldier", SP_monster_soldier},
	{"monster_soldier_ss", SP_monster_soldier_ss},


	{"monster_medic", SP_monster_medic},
*/
	{"turret_breach", SP_turret_breach},
	{"turret_base", SP_turret_base},
	{"turret_range", SP_turret_range},
	{"turret_driver", SP_turret_driver},




    {"ammo_grenades", SP_item_ammo_grenades},
	{"weapon_flamethrower",SP_item_weapon_flamethrower},

	{"ammo_napalm",SP_item_ammo_napalm},



//	{"dday_tank1", SP_tank_Tank1},
	{"objective_flag", SP_objective_flag},

	{"objective_VIP", SP_objective_VIP},

	{"objective_area",SP_objective_area},
	{"objective_touch",SP_objective_touch},
	{"objective_touch_ent",SP_objective_touch},
	{"timed_objective_touch",SP_timed_objective_touch},
	{"func_explosive_objective", SP_func_explosive_objective}, //test
	{"map_location", SP_map_location},//faf
	{"spawn_protect", SP_spawn_protect},//faf
	{"misc_airstrike", SP_airstrike},
	{"spawn_toggle", SP_Spawn_Toggle},
     //end of item modifications.

	{"item_botroam", SP_item_botroam},	//JABot

	{"briefcase", SP_briefcase},//faf:ctb code


	{NULL, NULL}
};
/*
===============
ED_CallSpawn

Finds the spawn function for the entity and calls it
===============
*/
void ED_CallSpawn (edict_t *ent)
{
	spawn_t	*s;
	gitem_t	*item;
	int		i;

	if (!ent->classname)
	{
		gi.dprintf ("ED_CallSpawn: NULL classname\n");
		return;
	}

	// check item spawn functions
	for (i=0,item=itemlist ; i<game.num_items ; i++,item++)
	{
		if (!item->classname)
			continue;
		if (!strcmp(item->classname, ent->classname))
		{	// found it
			SpawnItem (ent, item);
			return;
		}
	}

	// check normal spawn functions
	for (s=spawns ; s->name ; s++)
	{
		if (!strcmp(s->name, ent->classname))
		{	// found it
			s->spawn (ent);
			return;
		}
	}
	gi.dprintf ("%s doesn't have a spawn function\n", ent->classname);
}

/*
=============
ED_NewString
=============
*/
char *ED_NewString (char *string)
{
	char	*newb, *new_p;
	int		i, l;

	l = (int)strlen(string) + 1;

	newb = gi.TagMalloc (l, TAG_LEVEL);

	new_p = newb;

	for (i=0 ; i< l ; i++)
	{
		if (string[i] == '\\' && i < l-1)
		{
			i++;
			if (string[i] == 'n')
				*new_p++ = '\n';
			else
				*new_p++ = '\\';
		}
		else
			*new_p++ = string[i];
	}

	return newb;
}




/*
===============
ED_ParseField

Takes a key/value pair and sets the binary values
in an edict
===============
*/
void ED_ParseField (char *key, char *value, edict_t *ent)
{
	field_t	*f;
	byte	*b;
	float	v;
	vec3_t	vec;

	for (f=fields ; f->name ; f++)
	{
		if (!(f->flags & FFL_NOSPAWN) && !Q_stricmp(f->name, key))
		{	// found it
			if (f->flags & FFL_SPAWNTEMP)
				b = (byte *)&st;
			else
				b = (byte *)ent;

			switch (f->type)
			{
			case F_LSTRING:
				*(char **)(b+f->ofs) = ED_NewString (value);
				break;
			case F_VECTOR:
				sscanf (value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
				((float *)(b+f->ofs))[0] = vec[0];
				((float *)(b+f->ofs))[1] = vec[1];
				((float *)(b+f->ofs))[2] = vec[2];
				break;
			case F_INT:
				*(int *)(b+f->ofs) = atoi(value);
				break;
			case F_FLOAT:
				*(float *)(b+f->ofs) = atof(value);
				break;
			case F_ANGLEHACK:
				v = atof(value);
				((float *)(b+f->ofs))[0] = 0;
				((float *)(b+f->ofs))[1] = v;
				((float *)(b+f->ofs))[2] = 0;
				break;
			case F_IGNORE:
				break;
			}
			return;
		}
	}
	gi.dprintf ("%s is not a field\n", key);
}


// Knightmare added
/*
===============
ED_SetDefaultFields

Sets the default binary values in an edict
===============
*/
void ED_SetDefaultFields (edict_t *ent)
{
	field_t	*f;
	byte	*b;

	for (f=fields ; f->name ; f++)
	{
		if (f->flags & FFL_DEFAULT_NEG)
		{
			if (f->flags & FFL_SPAWNTEMP)
				b = (byte *)&st;
			else
				b = (byte *)ent;

			if (f->type == F_LSTRING)
				*(char **)(b+f->ofs) = ED_NewString ("-1");
			else if ( (f->type == F_VECTOR) || (f->type == F_ANGLEHACK) ) {
				((float *)(b+f->ofs))[0] = -1.0f;
				((float *)(b+f->ofs))[1] = -1.0f;
				((float *)(b+f->ofs))[2] = -1.0f;
			}
			else if (f->type == F_INT)
				*(int *)(b+f->ofs) = -1;
			else if (f->type == F_FLOAT)
				*(float *)(b+f->ofs) = -1.0f;
		}
	}
}
// end Knightmare


/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
====================
*/
char *ED_ParseEdict (char *data, edict_t *ent)
{
	qboolean	init;
	char		keyname[256];
	char		*com_token;

	init = false;
	memset (&st, 0, sizeof(st));

	// Knightmare- set field defaults
	ED_SetDefaultFields (ent);

// go through all the dictionary pairs
	while (1)
	{
	// parse key
		com_token = COM_Parse (&data);
		if (com_token[0] == '}')
			break;
		if (!data)
			gi.error ("ED_ParseEntity: EOF without closing brace");

		strncpy (keyname, com_token, sizeof(keyname)-1);

	// parse value
		com_token = COM_Parse (&data);
		if (!data)
			gi.error ("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			gi.error ("ED_ParseEntity: closing brace without data");

		init = true;

	// keynames with a leading underscore are used for utility comments,
	// and are immediately discarded by quake
		if (keyname[0] == '_')
			continue;

		ED_ParseField (keyname, com_token, ent);
	}

	if (!init)
		memset (ent, 0, sizeof(*ent));

	return data;
}


/*
================
G_FindTeams

Chain together all entities with a matching team field.

All but the first will have the FL_TEAMSLAVE flag set.
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams (void)
{
	edict_t	*e, *e2, *chain;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for (i=1, e=g_edicts+i ; i < globals.num_edicts ; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		if (e->flags & FL_TEAMSLAVE)
			continue;
		chain = e;
		e->teammaster = e;
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < globals.num_edicts ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				chain->teamchain = e2;
				e2->teammaster = e;
				chain = e2;
				e2->flags |= FL_TEAMSLAVE;
			}
		}
	}

	gi.dprintf ("%i teams with %i entities\n", c, c2);
}

/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
void SpawnEntities (char *mapname, char *entities, char *spawnpoint)
{
	edict_t		*ent;
	int			inhibit;
	char		*com_token;
	int			i;
	float		skill_level;

	skill_level = floor (skill->value);
	if (skill_level < 0)
		skill_level = 0;
	if (skill_level > 3)
		skill_level = 3;
//	if (skill->value != skill_level)
//		gi.cvar_forceset("skill", va("%f", skill_level));


	SaveClientData ();

	gi.FreeTags (TAG_LEVEL);

	memset (&level, 0, sizeof(level));
	memset (g_edicts, 0, game.maxentities * sizeof (g_edicts[0]));

	strncpy (level.mapname, mapname, sizeof(level.mapname)-1);
	strncpy (game.spawnpoint, spawnpoint, sizeof(game.spawnpoint)-1);

	// set client fields on player ents
	for (i=0 ; i<game.maxclients ; i++)
		g_edicts[i+1].client = game.clients + i;

	ent = NULL;
	inhibit = 0;

// parse ents
	while (1)
	{
		// parse the opening brace
		com_token = COM_Parse (&entities);
		if (!entities)
			break;
		if (com_token[0] != '{')
			gi.error ("ED_LoadFromFile: found %s when expecting {",com_token);

		if (!ent)
			ent = g_edicts;
		else
			ent = G_Spawn ();
		entities = ED_ParseEdict (entities, ent);

		// yet another map hack
		if (!Q_stricmp(level.mapname, "command") && !Q_stricmp(ent->classname, "trigger_once") && !Q_stricmp(ent->model, "*27"))
			ent->spawnflags &= ~SPAWNFLAG_NOT_HARD;

		// remove things (except the world) from different skill levels or deathmatch
		if (ent != g_edicts)
		{
			if (deathmatch->value)
			{
				if ( ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH )
				{
					G_FreeEdict (ent);
					inhibit++;
					continue;
				}
			}
			else
			{
				if ( /* ((coop->value) && (ent->spawnflags & SPAWNFLAG_NOT_COOP)) || */
					((skill->value == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
					((skill->value == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
					(((skill->value == 2) || (skill->value == 3)) && (ent->spawnflags & SPAWNFLAG_NOT_HARD))
					)
					{
						G_FreeEdict (ent);
						inhibit++;
						continue;
					}
			}

			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY|SPAWNFLAG_NOT_MEDIUM|SPAWNFLAG_NOT_HARD|SPAWNFLAG_NOT_COOP|SPAWNFLAG_NOT_DEATHMATCH);
		}

		ED_CallSpawn (ent);
	}

	gi.dprintf ("%i entities inhibited\n", inhibit);

#if 0
	i = 1;
	ent = EDICT_NUM(i);
	while (i < globals.num_edicts) {
		if (ent->inuse != 0 || ent->inuse != 1)
			Com_DPrintf("Invalid entity %d\n", i);
		i++, ent++;
	}
#endif

	G_FindTeams ();

	PlayerTrail_Init ();

	// pbowens: this is actually very handy
	gi.dprintf ("server map: %s\n", mapname);

	AI_NewMap ();	// JABot
}


char *ReadEntFile (char *filename)
{

	FILE		*fp;
	char		*filestring = NULL;
	long int	i = 0;
	int			ch;

	while (true)
	{
		fp = fopen(filename, "r");
		if (!fp) break;

		for (i=0; (ch = fgetc(fp)) != EOF; i++)
		;

		filestring = gi.TagMalloc(i+1, TAG_LEVEL);
		if (!filestring)
			break;

		fseek (fp, 0, SEEK_SET);
		for (i=0; (ch = fgetc(fp)) != EOF; i++)
		filestring[i] = ch;
		filestring[i] = '\0';

		break;
	}

	if (fp) fclose(fp);

	return(filestring);
}

char *LoadEntFile (char *mapname, char *entities)
{
	char	cmpname[MAX_QPATH];				// Knightmare added
	char	entfilename[MAX_OSPATH] = "";
	char	*newentities;
	int		i;

	if (ent_files->value == 0)
		return (entities);

	// hack so civ override files load
	if ( strstr(entities, "misc_civilian") )
		return (entities);

	// can put the word "override" in the override ents somewhere so the ent file doesnt load
	if ( strstr(entities, "override") ) {
		return (entities);
	}

	// Knightmare- use separate buffer for mapname
	// convert string to all lowercase (for Linux)
	Q_strncpyz (cmpname, sizeof(cmpname), mapname);
	for (i = 0; cmpname[i]; i++)
		cmpname[i] = tolower(cmpname[i]);

//	Com_sprintf (entfilename, sizeof(entfilename), "dday/ents/%s.ent", cmpname);
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (entfilename, sizeof(entfilename), "%s/ents/%s.ent", GameDir(), cmpname);

	newentities = ReadEntFile(entfilename);

	if (newentities)
	{   // leave these dprints active they show up in the server init console section
		gi.dprintf ("%s.ent Loaded\n", mapname);
		return (newentities);	// reassign the ents
	}
	else
	{
		gi.dprintf ("No .ent File for %s.bsp\n", mapname);
		return (entities);
	}
}


char *LoadCTCFile (char *mapname, char *entities)
{
	char	cmpname[MAX_QPATH];				// Knightmare added
	char	entfilename[MAX_OSPATH] = "";
	char	*newentities;
	int		i;

	// Knightmare- use separate buffer for mapname
	// convert string to all lowercase (for Linux)
	Q_strncpyz (cmpname, sizeof(cmpname), mapname);
	for (i = 0; cmpname[i]; i++)
		cmpname[i] = tolower(cmpname[i]);

//	Com_sprintf (entfilename, sizeof(entfilename), "dday/ents/%s.ctc", cmpname);
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (entfilename, sizeof(entfilename), "%s/ents/%s.ctc", GameDir(), cmpname);

	newentities = ReadEntFile(entfilename);

	if (newentities)
	{   //leave these dprints active they show up in the server init console section
		gi.dprintf("%s.ctc Loaded\n", mapname);
		return(newentities);	// reassign the ents
	}
	else
	{
		gi.dprintf("No .ctc File for %s.bsp\n", mapname);
		return(entities);
	}
}


void LoadCampFile (void)
{
	char	cmpname[MAX_QPATH];				// Knightmare added
	char	cmpfilename[MAX_OSPATH] = "";	// Knightmare- was MAX_QPATH
	char	*camplocs;
	int		i, c;
	char	*s, *f = NULL;
	vec3_t	loc;
	int		x = 0, y = 0, z = 0;
	int		angle = 0;
	int		team = 0;
	int		stance;
    // Knightmare added
	FILE	*check;
#ifndef _WIN32
	size_t  len;
#endif // _WIN32
	// end Knightmare

	// Knightmare- use separate buffer for mapname
	if (level.botfiles) {
		Q_strncpyz (cmpname, sizeof(cmpname), level.botfiles);
	}
	else {
		Q_strncpyz (cmpname, sizeof(cmpname), level.mapname);
	}

	// convert string to all lowercase (for Linux)
	for (i = 0; cmpname[i]; i++)
		cmpname[i] = tolower(cmpname[i]);

//	Com_sprintf (cmpfilename, sizeof(cmpfilename), "dday/navigation/%s.cmp", cmpname);
	// Knightmare- use  SavegameDir() / GameDir() instead for compatibility on all platforms
	Com_sprintf (cmpfilename, sizeof(cmpfilename), "%s/navigation/%s.cmp", SavegameDir(), cmpname);

	// fall back to GameDir() if not found in SavegameDir()
	check = fopen(cmpfilename, "r");
	if ( !check )
		Com_sprintf (cmpfilename, sizeof(cmpfilename), "%s/navigation/%s.cmp", GameDir(), cmpname);
	else
		fclose (check);
	// end Knightmare

//	gi.dprintf("sdfl %s\n", cmpfilename);
	gi.dprintf ("Reading camp loc file %s\n", cmpfilename);

	camplocs = ReadEntFile(cmpfilename);

	if (camplocs)
	{   //leave these dprints active they show up in the server init console section
		gi.dprintf("%s.cmp Loaded\n", level.mapname);

		c = 0;
	//	f = strdup (camplocs);
		f = G_CopyString (camplocs);	// Knightmare- use G_CopyString instead
		// Knightmare- replace carriage returns on Linux
#ifndef _WIN32
		len = strlen(f);
		for (i = 0; i < len; i++) {
			if (f[i] == '\r')
				f[i] = ' ';
		}
#endif // _WIN32
		s = strtok(f, "\n");
		while (s !=NULL)
		{
			if (s != NULL) {
				team = atoi (s);
				//gi.dprintf ("s: %s\n",s);
				s = strtok (NULL, "\n");
				if (team == 0)
					level.allied_cmps = true;
				else if (team == 1)
					level.axis_cmps = true;


			}
			if (s != NULL) {
				x = atoi (s);
				//gi.dprintf ("s: %i\n",x);
				s = strtok (NULL, "\n");
			}
			if (s != NULL) {
				y = atoi (s);
				//gi.dprintf ("s: %i\n",y);
				s = strtok (NULL, "\n");
			}
			if (s != NULL) {
				z = atoi (s);
				//gi.dprintf ("s: %i\n",z);
				s = strtok (NULL, "\n");
			}
			if (s != NULL) {
				angle = atoi (s);
				//gi.dprintf ("s: %i\n",z);
				s = strtok (NULL, "\n");
			}
			if (s != NULL) {
				stance = atoi (s);
				//gi.dprintf ("s: %i\n",stance);
				s = strtok (NULL, "\n");
/*			}
			if (s != NULL) {
				//= atoi (s);
				//gi.dprintf ("s: %i\n",);
				s = strtok (NULL, "\n");

			}
			if (s != NULL) {
				//= atoi (s);
				//gi.dprintf ("s: %i\n",);
				s = strtok (NULL, "\n");

			}
			if (s != NULL) {
				//= atoi (s);
				//gi.dprintf ("s: %i\n",);
				s = strtok (NULL, "\n");
*/

				VectorSet (loc, x, y, z);
				camp_spots[c].angle = angle;
				camp_spots[c].team = team;
				VectorCopy (loc, camp_spots[c].origin);
				camp_spots[c].stance = stance;
				camp_spots[c].type = CAMP_NORMAL;
				c++;
				total_camp_spots = c;


			}
			//

		}

	}
	else
	{
		gi.dprintf ("Couldn't load camp locs from %s\n", cmpfilename);
		return;
	}

	// Knightmare- free temp buffer
	if (f != NULL) {
		gi.TagFree (f);
		f = NULL;
	}
}


void SpawnEntities2 (char *mapname, char *entities, char *spawnpoint)
{
	edict_t		*ent;
	int			inhibit;
	char		*com_token;
	int			i;
	float		skill_level;

	skill_level = floor (skill->value);
	if (skill_level < 0)
		skill_level = 0;
	if (skill_level > 3)
		skill_level = 3;
//	if (skill->value != skill_level)
//		gi.cvar_forceset("skill", va("%f", skill_level));

	SaveClientData ();

	gi.FreeTags (TAG_LEVEL);



	memset (&level, 0, sizeof(level));
	memset (g_edicts, 0, game.maxentities * sizeof (g_edicts[0]));

	strncpy (level.mapname, mapname, sizeof(level.mapname)-1);
	strncpy (game.spawnpoint, spawnpoint, sizeof(game.spawnpoint)-1);

	// set client fields on player ents
	for (i=0 ; i<game.maxclients ; i++)
		g_edicts[i+1].client = game.clients + i;

	ent = NULL;
	inhibit = 0;


	InitItems ();


	if (ctc->value)
		entities = LoadCTCFile(mapname,entities);
	else
		entities = LoadEntFile(mapname, entities);//faf
// parse ents
	while (1)
	{
		// parse the opening brace
		com_token = COM_Parse (&entities);
		if (!entities)
			break;
		if (com_token[0] != '{')
			gi.error ("ED_LoadFromFile: found %s when expecting {",com_token);

		if (!ent)
			ent = g_edicts;
		else
			ent = G_Spawn ();
		entities = ED_ParseEdict (entities, ent);

		// yet another map hack
		if (!Q_stricmp(level.mapname, "command") && !Q_stricmp(ent->classname, "trigger_once") && !Q_stricmp(ent->model, "*27"))
			ent->spawnflags &= ~SPAWNFLAG_NOT_HARD;

		// remove things (except the world) from different skill levels or deathmatch
		if (ent != g_edicts)
		{
			if (deathmatch->value)
			{
				if ( ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH )
				{
					G_FreeEdict (ent);
					inhibit++;
					continue;
				}
			}
			else
			{
				if ( /* ((coop->value) && (ent->spawnflags & SPAWNFLAG_NOT_COOP)) || */
					((skill->value == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
					((skill->value == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
					(((skill->value == 2) || (skill->value == 3)) && (ent->spawnflags & SPAWNFLAG_NOT_HARD))
					)
					{
						G_FreeEdict (ent);
						inhibit++;
						continue;
					}
			}

			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY|SPAWNFLAG_NOT_MEDIUM|SPAWNFLAG_NOT_HARD|SPAWNFLAG_NOT_COOP|SPAWNFLAG_NOT_DEATHMATCH);
		}

		ED_CallSpawn (ent);
	}

	gi.dprintf ("%i entities inhibited\n", inhibit);

#if 0
	i = 1;
	ent = EDICT_NUM(i);
	while (i < globals.num_edicts) {
		if (ent->inuse != 0 || ent->inuse != 1)
			Com_DPrintf("Invalid entity %d\n", i);
		i++, ent++;
	}
#endif

	G_FindTeams ();

	PlayerTrail_Init ();

	//pbowens: this is actually very handy
	gi.dprintf ("server map: %s\n", mapname);

	LoadCampFile ();

	AI_NewMap ();    // JABot
}


//===================================================================

#if 0
	// cursor positioning
	xl <value>
	xr <value>
	yb <value>
	yt <value>
	xv <value>
	yv <value>

	// drawing
	statpic <name>
	pic <stat>
	num <fieldwidth> <stat>
	string <stat>

	// control
	if <stat>
	ifeq <stat> <value>
	ifbit <stat> <value>
	endif

#endif


char *dday_statusbar =
"yb	-24 "

// health
"if 6 "
"   xv	0 "
"   hnum "
"   xv	50 "
"   pic 0 "
"endif "

// ammo/rounds
" if 4 "
"	xv	100 "
"	anum "		// round count (show only if ammo icon)
"	xv	200 "
"	pic 4 "		// ammo icons
//"	xv	150 "
//"	pic 2 "
" endif "

// armor/clips
"if 6 "
"	xv	150 " // 200
"	rnum "		// mag count -show only if selected item icon
"endif "

// selected item
"if 6 "
"	xv	250 " // 296
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
//"	xv	26 "
"	yb	-68 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 10 "
"	xv	246 "
"	num	2	10 "
"	xv	296 "
"	pic	9 "
"endif "

/*
//new timer
"if 10 "
"   xv  140"
"   yv  46"
"	pic	9 "
"   xv  134"
"   yv  74 "
"	num	2 10 "
"endif "
*/

//  help / weapon icon
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "

// stamina
/*
"if 6 "
"   xl 10 "
"   num 3 28 "
"   xl 15 "
"   yb -60 "
"   string2 \"STAMINA\" "
"endif "
*/

"if 6 "
"   xl 16 "
"   yb -60 "
"   pic 28 "
"endif "

/*
//autopickup
"if 29 "
"    xl 70 "
"    yb -30 "
"    string2 \"PICKUP\" "
"    xl 65 "
"    yb -20 "
"    string2 \"ENABLED\" "
"endif "
*/
//autopickup
"if 29 "
//"    xl 70 "
//"    yb -30 "
"   yb	-24 "
"   xv	-60 "


"pic 29 "
"endif "


// objectives
"if 16 "
"   xl 0 "
"   yt 0 "
"   pic 16 "
"endif "

// ident
"if 17 "
"   xv 0 "
"   yb -200 "
"   stat_string 18 "
"   if 19 "
"      yb -190 "
"      stat_string 19 "
"   endif "
"   if 20 "
"      xv -50  "
"      yb -200 "
"      pic 20  "
"   endif "
"endif "

// sniper scope
" if 21 "
"   xr 1 0 "
"   yb  0 "
"   xv  0 "
"   yv  0 "
"   pic 21 "
"endif "

/*
// crosshair
" if 29 "
"   xr 1 0 "
"   yb  0 "
"   xv  156 "
"   yv  119 "
"   pic 29 "
"endif " */

// stats
"yt 17 "
"xr -75 "
" string2 \"KILLS\" "
"xr -140 "
" string2 \"POINTS\" "

// team 0
"yt 32 "
"if 22 "
  "xr -33 "  //faf was -26, so team icon fits on screen for software mode
  "pic 22 "
"endif "
"yt 34 "
"xr -98 "
"num 3 23 "
"xr -160 "
"num 3 24 "

// team 1
"yt 64 "
"if 25 "
  "xr -33 "  //faf was -26, so team icon fits on screen for software mode
  "pic 25 "
"endif "
"yt 68 "
"xr -98 "
"num 3 26 "
"xr -160 "
"num 3 27 "


// timer  //faf

"if 30 "
	"yt 115 "
	"xr -70 "
	"string2 \"TIME\" "
	"yt 128 "
	"xr -85 "
	"num 3 30 "


	"yt 158 "
	"xr -70 "
	"pic 31 "
"end if "


// respawn timer
//"if 30 "
//"  xv 10 "
//"  yv 10 "
//"  num 3 30 "
//"endif "

/*
// team stats
"xl 10 "
"yb	-45 "
"stat_string 22 "
"yb -55 "
"stat_string 23 "
 */

 /*
 DISABLED FOR PUBLIC OPINION

  //  frags
"xr	-50 "
"yt 2 "
"num 3 14 "
*/
;


void GetMapObjective (void);

/*QUAKED worldspawn (0 0 0) ?

Only used for the world.
"sky"	environment map name
"skyaxis"	vector axis for rotating sky
"skyrotate"	speed of rotation in degrees/second
"sounds"	music cd track number
"gravity"	800 is default gravity
"message"	text to print at user logon
*/
void SP_worldspawn (edict_t *ent)
{
	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->inuse = true;			// since the world doesn't use G_Spawn()
	ent->s.modelindex = 1;		// world model is always index 1

	//---------------

	// reserve some spots for dead player bodies for coop / deathmatch
	InitBodyQue ();

	// set configstrings for items
	//SetItemNames ();

	if (ent->deathtarget)
		level.botfiles = ent->deathtarget;

	if (st.nextmap)
		Q_strncpyz (level.nextmap, sizeof(level.nextmap), st.nextmap);


//	map_tree[0][0]=0;
	// make some data visible to the server

	if (ent->message && ent->message[0])
	{
		gi.configstring (CS_NAME, ent->message);
	//	strncpy (level.level_name, ent->message, sizeof(level.level_name));
		Q_strncpyz (level.level_name, sizeof(level.level_name), ent->message);
	}
	else
	//	strncpy (level.level_name, level.mapname, sizeof(level.level_name));
		Q_strncpyz (level.level_name, sizeof(level.level_name), level.mapname);

	if (st.sky && st.sky[0])
		gi.configstring (CS_SKY, st.sky);
	else
		gi.configstring (CS_SKY, "unit1_");

	gi.configstring (CS_SKYROTATE, va("%f", st.skyrotate) );

	gi.configstring (CS_SKYAXIS, va("%f %f %f",
		st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]) );

	// Knightmare- configstrings added for DK-style clouds support
#ifdef KMQUAKE2_ENGINE_MOD
	gi.configstring (CS_SKYDISTANCE, va("%f", st.skydistance) );

	if (st.cloudname && st.cloudname[0])
		gi.configstring (CS_CLOUDNAME, st.cloudname);
	else
		gi.configstring (CS_CLOUDNAME, "");

	gi.configstring (CS_CLOUDLIGHTFREQ, va("%f", st.lightningfreq) );

	gi.configstring (CS_CLOUDDIR, va("%f %f", st.cloudxdir, st.cloudydir) );

	gi.configstring (CS_CLOUDTILE, va("%f %f %f", st.cloud1tile, st.cloud2tile, st.cloud3tile) );

	gi.configstring (CS_CLOUDSPEED, va("%f %f %f", st.cloud1speed, st.cloud2speed, st.cloud3speed) );

	gi.configstring (CS_CLOUDALPHA, va("%f %f %f", st.cloud1alpha, st.cloud2alpha, st.cloud3alpha) );
#endif	// KMQUAKE2_ENGINE_MOD
	// end DK-style clouds support

	// Knightmare- if a named soundtrack is specified, play it instead of from CD
	if (ent->musictrack && strlen(ent->musictrack))
		gi.configstring (CS_CDTRACK, ent->musictrack);
	else
		gi.configstring (CS_CDTRACK, va("%i", ent->sounds) );

	gi.configstring (CS_MAXCLIENTS, va("%i", (int)(maxclients->value) ) );

	// status bar program
	gi.configstring (CS_STATUSBAR, dday_statusbar);

	//---------------


	// help icon for statusbar
	gi.imageindex ("i_help");
	level.pic_health = gi.imageindex ("i_health");
	gi.imageindex ("i_medic");

	gi.imageindex ("help");
	gi.imageindex ("field_3");

	//DDAY
	gi.imageindex ("i_respcount");
	gi.imageindex ("i_dday");
	gi.imageindex ("crosshair");

	// pbowens: scoreboard
	gi.imageindex ("scorehead");
	gi.imageindex ("scoreleft");
	gi.imageindex ("scoreright");

	gi.imageindex ("inventory");

	// pbowens: stamina bar
	gi.imageindex ("s_00");
	gi.imageindex ("s_10");
	gi.imageindex ("s_20");
	gi.imageindex ("s_30");
	gi.imageindex ("s_40");
	gi.imageindex ("s_50");
	gi.imageindex ("s_60");
	gi.imageindex ("s_70");
	gi.imageindex ("s_80");
	gi.imageindex ("s_90");
	gi.imageindex ("s_100");

	if (!st.gravity)
		gi.cvar_set("sv_gravity", "800");
	else
		gi.cvar_set("sv_gravity", st.gravity);

	gi.cvar_set("knifefest", "0");
	gi.cvar_set("swords", "0");

	if (skill->value < 0)
		gi.cvar_forceset ("skill", "0");
	else if (skill->value > 3)
		gi.cvar_forceset ("skill", "3");


	//snd_fry = gi.soundindex ("player/fry.wav");	// standing in lava / slime
	snd_fry = gi.soundindex ("players/tear.wav");	// standing in lava / slime
	gi.soundindex ("players/cloth.wav");

	if (*campaign->string)
	{
		gi.imageindex ("o");
		gi.imageindex ("u");
		gi.imageindex ("g");
		gi.imageindex ("q");
		gi.imageindex ("allies_campaign");
		gi.imageindex ("axis_campaign");
		gi.soundindex ("campaigns/norm-allies.wav");
		gi.soundindex ("campaigns/norm-axis.wav");
		gi.imageindex(va("%s", campaign->string));
	}

	if (*serverimg->string)
	{
		gi.imageindex(va("%s", serverimg->string));
	}


	gi.soundindex("bullet/wood1.wav");
	gi.soundindex("bullet/wood2.wav");
	gi.soundindex("bullet/wood3.wav");
	gi.soundindex("bullet/metal1.wav");
	gi.soundindex("bullet/metal2.wav");
	gi.soundindex("bullet/metal3.wav");
	gi.soundindex("bullet/glass1.wav");
	gi.soundindex("bullet/glass2.wav");
	gi.soundindex("bullet/sand1.wav");
	gi.soundindex("bullet/sand2.wav");
	gi.soundindex("bullet/sand3.wav");
	gi.soundindex("bullet/concrete1.wav");
	gi.soundindex("bullet/concrete2.wav");
	gi.soundindex("bullet/concrete3.wav");
	gi.soundindex("bullet/water1.wav");
	gi.soundindex("bullet/water2.wav");




//	PrecacheItem (FindItem ("Colt .45"));

	gi.soundindex ("player/lava1.wav");
	gi.soundindex ("player/lava2.wav");

	gi.soundindex ("misc/pc_up.wav");
	gi.soundindex ("misc/talk1.wav");

	gi.soundindex ("misc/udeath.wav");

	// gibs
	gi.soundindex ("items/respawn1.wav");

	// sexed sounds
	gi.soundindex ("*death1.wav");
	gi.soundindex ("*death2.wav");
	gi.soundindex ("*death3.wav");
	gi.soundindex ("*death4.wav");
//	gi.soundindex ("*fall1.wav");
//	gi.soundindex ("*fall2.wav");
	gi.soundindex ("*gurp1.wav");		// drowning damage
	gi.soundindex ("*gurp2.wav");
	gi.soundindex ("*jump1.wav");		// player jump
	gi.soundindex ("*pain25_1.wav");
	gi.soundindex ("*pain25_2.wav");
	gi.soundindex ("*pain50_1.wav");
	gi.soundindex ("*pain50_2.wav");
	gi.soundindex ("*pain75_1.wav");
	gi.soundindex ("*pain75_2.wav");
	gi.soundindex ("*pain100_1.wav");
	gi.soundindex ("*pain100_2.wav");

	gi.soundindex("foot/grass1.wav");
	gi.soundindex("foot/grass2.wav");
	gi.soundindex("foot/wood1.wav");
	gi.soundindex("foot/wood2.wav");
	gi.soundindex("foot/metal1.wav");
	gi.soundindex("foot/metal2.wav");
	gi.soundindex("foot/sand1.wav");
	gi.soundindex("foot/sand2.wav");
	gi.soundindex("player/step1.wav");
	gi.soundindex("player/step2.wav");
	gi.soundindex("player/step3.wav");
	gi.soundindex("player/step4.wav");
	gi.soundindex("misc/rumble.wav");
	gi.soundindex("misc/ring.wav");

	// for dday generic vwep
	// this defeats the purpose of team dlls, but owell


	//faf: going to try to remove this.  this should stop
	//     gbr map crashes.

	/*
	// GRM
	gi.modelindex("#w_p38.md2");
	gi.modelindex("#w_m98k.md2");
	gi.modelindex("#w_mp40.md2");
	gi.modelindex("#w_mp43.md2");
	gi.modelindex("#w_mg42.md2");
	gi.modelindex("#w_panzer.md2");
	gi.modelindex("#w_m98ks.md2");
	gi.modelindex("#a_masher.md2");

	// USA
	gi.modelindex("#w_colt45.md2");
	gi.modelindex("#w_m1.md2");
	gi.modelindex("#w_thompson.md2");
	gi.modelindex("#w_bar.md2");
	gi.modelindex("#w_bhmg.md2");
	gi.modelindex("#w_bazooka.md2");
	gi.modelindex("#w_m1903.md2");
	gi.modelindex("#a_grenade.md2");

	// GENERIC
	gi.modelindex("#w_flame.md2");
	gi.modelindex("#w_morphine.md2");
	gi.modelindex("#w_knife.md2");
	gi.modelindex("#w_binoc.md2");
	*/

/*
	gi.modelindex( "#w_pistol.md2");
	gi.modelindex( "#w_rifle.md2");
	gi.modelindex( "#w_submg.md2");
	gi.modelindex( "#w_lmg.md2");
	gi.modelindex( "#w_hmg.md2");
	gi.modelindex( "#w_rocket.md2");
	gi.modelindex( "#w_sniper.md2");
	gi.modelindex( "#a_grenades.md2");
	gi.modelindex( "#w_masher.md2");
*/
//bcass start - TNT - GET RIGHT MODEL
//faf	gi.modelindex( "#w_tnt.md2");
//bcass end

	//-------------------

	gi.soundindex ("player/gasp1.wav");		// gasping for air
	gi.soundindex ("player/gasp2.wav");		// head breaking surface, not gasping

	gi.soundindex ("player/watr_in.wav");	// feet hitting water
	gi.soundindex ("player/watr_out.wav");	// feet leaving water

	gi.soundindex ("player/watr_un.wav");	// head going underwater

//	gi.soundindex ("player/u_breath1.wav");
//	gi.soundindex ("player/u_breath2.wav");

	gi.soundindex ("items/pkup.wav");		// bonus item pickup
	gi.soundindex ("world/land.wav");		// landing thud
	gi.soundindex ("misc/h2ohit1.wav");		// landing splash

//	gi.soundindex ("items/damage.wav");
//	gi.soundindex ("items/protect.wav");
//	gi.soundindex ("items/protect4.wav");
	gi.soundindex ("weapons/noammo.wav");

//	gi.soundindex ("infantry/inflies1.wav");


	gi.soundindex ("player/bodyfall.wav");
	gi.soundindex ("weapons/gundrop.wav");
	gi.soundindex ("weapons/ammodrop.wav");


	gi.soundindex ("player/breathe.wav"); //faf






	sm_meat_index = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");
	gi.modelindex ("models/objects/gibs/arm/tris.md2");
	gi.modelindex ("models/objects/gibs/bone/tris.md2");
	gi.modelindex ("models/objects/gibs/bone2/tris.md2");
	gi.modelindex ("models/objects/gibs/chest/tris.md2");
	gi.modelindex ("models/objects/gibs/skull/tris.md2");
	gi.modelindex ("models/objects/gibs/head2/tris.md2");

	// DDAY
	gi.soundindex ("misc/hitleg.wav");
	gi.soundindex ("misc/hittorso.wav");
	gi.soundindex ("misc/hithelm.wav");
	gi.soundindex ("misc/hithead.wav");


	gi.soundindex ("misc/gibs1.wav");
	gi.soundindex ("misc/gibs2.wav");


	// pbowens: precache these here b/c players use them
	PrecacheItem ( FindItem("Morphine")		);
	PrecacheItem ( FindItem("Helmet")		);
	PrecacheItem ( FindItem("Fists")		);
	PrecacheItem ( FindItem("Binoculars")	);
	PrecacheItem ( FindItem("Knife")		);
	PrecacheItem ( FindItem("Flamethrower")	);
	PrecacheItem ( FindItem("flame_mag")	);

//
// Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
//

	// 0 normal
	gi.configstring(CS_LIGHTS+0, "m");

	// 1 FLICKER (first variety)
	gi.configstring(CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo");

	// 2 SLOW STRONG PULSE
	gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

	// 3 CANDLE (first variety)
	gi.configstring(CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

	// 4 FAST STROBE
	gi.configstring(CS_LIGHTS+4, "mamamamamama");

	// 5 GENTLE PULSE 1
	gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");

	// 6 FLICKER (second variety)
	gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");

	// 7 CANDLE (second variety)
	gi.configstring(CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm");

	// 8 CANDLE (third variety)
	gi.configstring(CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

	// 9 SLOW STROBE (fourth variety)
	gi.configstring(CS_LIGHTS+9, "aaaaaaaazzzzzzzz");

	// 10 FLUORESCENT FLICKER
	gi.configstring(CS_LIGHTS+10, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	gi.configstring(CS_LIGHTS+63, "a");

	GetMapObjective();

}

