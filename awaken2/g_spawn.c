// g_spawn.c

#include "g_local.h"

typedef struct
{
	char	*name;
	void	(*spawn)(edict_t *ent);
} spawn_t;


void SP_item_health(edict_t *self);
void SP_item_health_small(edict_t *self);
void SP_item_health_large(edict_t *self);
void SP_item_health_mega(edict_t *self);

void SP_info_player_start(edict_t *self);
void SP_info_player_deathmatch(edict_t *self);
void SP_info_player_coop(edict_t *self);
void SP_info_player_intermission(edict_t *self);

void SP_func_plat(edict_t *ent);
void SP_func_rotating(edict_t *ent);
void SP_func_button(edict_t *ent);
void SP_func_door(edict_t *ent);
void SP_func_door_secret(edict_t *ent);
void SP_func_door_rotating(edict_t *ent);
void SP_func_water(edict_t *ent);
void SP_func_train(edict_t *ent);
void SP_func_conveyor(edict_t *self);
void SP_func_wall(edict_t *self);
void SP_func_object(edict_t *self);
void SP_func_explosive(edict_t *self);
void SP_func_timer(edict_t *self);
void SP_func_areaportal(edict_t *ent);
void SP_func_clock(edict_t *ent);
void SP_func_killbox(edict_t *ent);

void SP_trigger_always(edict_t *ent);
void SP_trigger_once(edict_t *ent);
void SP_trigger_multiple(edict_t *ent);
void SP_trigger_relay(edict_t *ent);
void SP_trigger_push(edict_t *ent);
void SP_trigger_hurt(edict_t *ent);
void SP_trigger_key(edict_t *ent);
void SP_trigger_counter(edict_t *ent);
void SP_trigger_elevator(edict_t *ent);
void SP_trigger_gravity(edict_t *ent);

void SP_target_temp_entity(edict_t *ent);
void SP_target_speaker(edict_t *ent);
void SP_target_explosion(edict_t *ent);
void SP_target_changelevel(edict_t *ent);
void SP_target_splash(edict_t *ent);
void SP_target_spawner(edict_t *ent);
void SP_target_blaster(edict_t *ent);
void SP_target_laser(edict_t *self);
void SP_target_earthquake(edict_t *ent);
void SP_target_character(edict_t *ent);
void SP_target_string(edict_t *ent);

void SP_worldspawn(edict_t *ent);

void SP_light(edict_t *self);
void SP_light_mine1(edict_t *ent);
void SP_light_mine2(edict_t *ent);
void SP_info_null(edict_t *self);
void SP_info_notnull(edict_t *self);
void SP_path_corner(edict_t *self);

void SP_misc_explobox(edict_t *self);
void SP_misc_banner(edict_t *self);
void SP_misc_satellite_dish(edict_t *self);
void SP_misc_gib_arm(edict_t *self);
void SP_misc_gib_leg(edict_t *self);
void SP_misc_gib_head(edict_t *self);
void SP_misc_viper(edict_t *self);
void SP_misc_viper_bomb(edict_t *self);
void SP_misc_bigviper(edict_t *self);
void SP_misc_strogg_ship(edict_t *self);
void SP_misc_teleporter(edict_t *self);
void SP_misc_teleporter_dest(edict_t *self);
void SP_misc_blackhole(edict_t *self);
void SP_misc_eastertank(edict_t *self);
void SP_misc_easterchick(edict_t *self);
void SP_misc_easterchick2(edict_t *self);

void SP_turret_breach(edict_t *self);
void SP_turret_base(edict_t *self);
//void SP_turret_driver(edict_t *self);	// Knightmare- not used

// Knightmare- entities that use origin-based train pathing
void SP_func_train_origin (edict_t *self);
void SP_model_train_origin (edict_t *self);
void SP_misc_viper_origin (edict_t *ent);
void SP_misc_strogg_ship_origin (edict_t *ent);
// Knightmare- these are from Rogue
void SP_func_plat2 (edict_t *ent);
void SP_func_door_secret2(edict_t *ent);
void SP_func_force_wall(edict_t *ent);

//CW++
void SP_weapon_shotgun(edict_t *self);
void SP_weapon_supershotgun(edict_t *self);
void SP_weapon_machinegun(edict_t *self);
void SP_weapon_chaingun(edict_t *self);
void SP_weapon_grenadelauncher(edict_t *self);
void SP_weapon_hyperblaster(edict_t *self);
void SP_weapon_bfg(edict_t *self);
void SP_ammo_grenades(edict_t *self);

void SP_info_player_attack(edict_t *self);
void SP_info_player_defend(edict_t *self);
void SP_info_timelimit(edict_t *self);
void SP_info_mission(edict_t *self);
void SP_info_nohook(edict_t *self);

void SP_trigger_waypoint(edict_t *self);
void SP_trigger_kill(edict_t *self);
void SP_target_victory(edict_t *self);
//CW--

//DH++
void SP_model_spawn (edict_t *self);
void SP_model_train (edict_t *self);
void SP_model_turret (edict_t *self);
void SP_target_effect (edict_t *self);
void SP_func_monitor (edict_t *self);
//DH--

spawn_t	spawns[] = {
	{"item_health", SP_item_health},
	{"item_health_small", SP_item_health_small},
	{"item_health_large", SP_item_health_large},
	{"item_health_mega", SP_item_health_mega},

	{"info_player_start", SP_info_player_start},
	{"info_player_deathmatch", SP_info_player_deathmatch},
	{"info_player_coop", SP_info_player_coop},
	{"info_player_intermission", SP_info_player_intermission},
//ZOID++
	{"info_player_team1", SP_info_player_team1},
	{"info_player_team2", SP_info_player_team2},
//ZOID--

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

	{"target_temp_entity", SP_target_temp_entity},
	{"target_speaker", SP_target_speaker},
	{"target_explosion", SP_target_explosion},
	{"target_changelevel", SP_target_changelevel},
	{"target_splash", SP_target_splash},
	{"target_spawner", SP_target_spawner},
	{"target_blaster", SP_target_blaster},
	{"target_laser", SP_target_laser},
	{"target_earthquake", SP_target_earthquake},
	{"target_character", SP_target_character},
	{"target_string", SP_target_string},

	{"worldspawn", SP_worldspawn},

	{"light", SP_light},
	{"light_mine1", SP_light_mine1},
	{"light_mine2", SP_light_mine2},
	{"info_null", SP_info_null},
	{"func_group", SP_info_null},
	{"info_notnull", SP_info_notnull},
	{"path_corner", SP_path_corner},

	{"misc_explobox", SP_misc_explobox},
	{"misc_banner", SP_misc_banner},
//ZOID++
	{"misc_ctf_banner", SP_misc_ctf_banner},
	{"misc_ctf_small_banner", SP_misc_ctf_small_banner},
//ZOID--
	{"misc_satellite_dish", SP_misc_satellite_dish},
	{"misc_gib_arm", SP_misc_gib_arm},
	{"misc_gib_leg", SP_misc_gib_leg},
	{"misc_gib_head", SP_misc_gib_head},
	{"misc_viper", SP_misc_viper},
	{"misc_viper_bomb", SP_misc_viper_bomb},
	{"misc_bigviper", SP_misc_bigviper},
	{"misc_strogg_ship", SP_misc_strogg_ship},
	{"misc_teleporter", SP_misc_teleporter},
	{"misc_teleporter_dest", SP_misc_teleporter_dest},
//ZOID++
	{"trigger_teleport", SP_trigger_teleport},
	{"info_teleport_destination", SP_info_teleport_destination},
//ZOID--
	{"misc_blackhole", SP_misc_blackhole},
	{"misc_eastertank", SP_misc_eastertank},
	{"misc_easterchick", SP_misc_easterchick},
	{"misc_easterchick2", SP_misc_easterchick2},

	// Knightmare- entities that use origin-based train pathing
	{"func_train_origin", SP_func_train_origin},
	{"model_train_origin", SP_model_train_origin},
	{"misc_viper_origin", SP_misc_viper_origin},
	{"misc_strogg_ship_origin", SP_misc_strogg_ship_origin},
	// Knightmare- these are from rogue
	{"func_plat2", SP_func_plat2},
	{"func_door_secret2", SP_func_door_secret2},
	{"func_force_wall", SP_func_force_wall},

//CW++
	{"weapon_shotgun", SP_weapon_shotgun},
	{"weapon_supershotgun", SP_weapon_supershotgun},
	{"weapon_machinegun", SP_weapon_machinegun},
	{"weapon_grenadelauncher", SP_weapon_grenadelauncher},
	{"weapon_chaingun", SP_weapon_chaingun},
	{"weapon_hyperblaster", SP_weapon_hyperblaster},
	{"weapon_bfg", SP_weapon_bfg},
	{"ammo_grenades", SP_ammo_grenades},

	{"info_player_attack", SP_info_player_attack},
	{"info_player_defend", SP_info_player_defend},
	{"info_timelimit", SP_info_timelimit},
	{"info_mission", SP_info_mission},
	{"info_nohook", SP_info_nohook},

	{"trigger_waypoint", SP_trigger_waypoint},
	{"target_victory", SP_target_victory},
//CW--

//DH++
	{"turret_breach", SP_turret_breach},
	{"turret_base", SP_turret_base},
	{"model_spawn",  SP_model_spawn},
	{"model_train",  SP_model_train},
	{"model_turret", SP_model_turret},
	{"target_effect", SP_target_effect},
	{"func_monitor", SP_func_monitor},
//DH--
	{NULL, NULL}
};

//CW++ List of powerups to be spawned randomly at DM points.
static char *pupnames[] = { "item_mystery", "item_adrenaline", "item_pack", NULL };
//CW--

//CW++
/*
===============
IsStandardPowerup

Returns true if the specified entity is one of the standard Quake2 powerups
that is substituted for an Awakening one.
===============
*/
qboolean IsStandardPowerup(edict_t *ent)
{
	if (!ent || !ent->classname)
		return false;

	if (!Q_stricmp(ent->classname, "item_quad"))
		return true;

	if (!Q_stricmp(ent->classname, "item_invulnerability"))
		return true;
	
	if (!Q_stricmp(ent->classname, "item_silencer"))
		return true;

	return false;
}

/*
==============
SubstitutePowerup

Substitutes the vanilla Q2 powerups for the Awakening ones.
==============
*/
qboolean SubstitutePowerup(edict_t *ent)
{
	if (!ent || !ent->classname)
		return false;

	if (!Q_stricmp(ent->classname, "item_quad"))
	{
		SpawnItem(ent, FindItem("Awakening"));
		ent->classname = "item_siphon";
		return true;
	}
	
	if (!Q_stricmp(ent->classname, "item_invulnerability"))
	{
		SpawnItem(ent, FindItem("D89"));
		ent->classname = "item_needle";
		return true;
	}
	
	if (!Q_stricmp(ent->classname, "item_silencer"))
	{
		SpawnItem(ent, FindItem("Haste"));
		ent->classname = "item_haste";
		return true;
	}

	return false;
}
//CW--

/*
===============
ED_CallSpawn

Finds the spawn function for the entity and calls it.
===============
*/
void ED_CallSpawn(edict_t *ent)
{
	spawn_t	*s;
	gitem_t	*item;
	int		i;

	if (!ent->classname)
	{
		gi.dprintf("ED_CallSpawn: NULL classname\n");
		return;
	}

//	Check item spawn functions.

	for (i = 0, item = itemlist; i < game.num_items; i++, item++)
	{
		if (!item->classname)
			continue;
//CW++
		if (!((int)dmflags->value & DF_NO_REPLACEMENTS) && IsStandardPowerup(ent))
			continue;
//CW--
		if (!Q_stricmp(item->classname, ent->classname))											//CW
		{
			SpawnItem(ent, item);
			return;
		}
	}

//	Check normal spawn functions.

	for (s = spawns; s->name; s++)
	{
		if (!Q_stricmp(s->name, ent->classname))													//CW
		{
			s->spawn(ent);
			return;
		}
	}

//CW++
//	Do powerup substitution if the appropriate dmflag is set.

	if (!((int)dmflags->value & DF_NO_REPLACEMENTS))
	{
		if (SubstitutePowerup(ent))
			return;
	}
//CW--

	gi.dprintf("%s doesn't have a spawn function\n", ent->classname);
}

/*
=============
ED_NewString
=============
*/
char *ED_NewString(char *string)
{
	char	*newb;
	char	*new_p;
	int		i;
	int		l;
	
	l = (int)strlen(string) + 1;
	newb = gi.TagMalloc(l, TAG_LEVEL);
	new_p = newb;

	for (i = 0; i < l; i++)
	{
		if ((string[i] == '\\') && (i < l-1))
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
void ED_ParseField(char *key, char *value, edict_t *ent)
{
	field_t	*f;
	byte	*b;
	vec3_t	vec;
	float	v;

	for (f = fields; f->name; ++f)
	{
		if (!Q_stricmp(f->name, key))
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
//CW++
				case F_GSTRING:
				case F_EDICT:
				case F_ITEM:
				case F_CLIENT:
					break;

				default:	// sanity check
					gi.error("ED_ParseField(): unknown field type");
//CW--
			}
			return;
		}
	}
	gi.dprintf("%s is not a field\n", key);
}

/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
====================
*/
char *ED_ParseEdict(char *data, edict_t *ent)
{
	qboolean	init;
	char		keyname[256];
	char		*com_token;

	init = false;
	memset(&st, 0, sizeof(st));

//	go through all the dictionary pairs
	while (1)
	{	
		// parse key
		com_token = COM_Parse (&data);
		if (com_token[0] == '}')
			break;
		if (!data)
			gi.error("ED_ParseEntity: EOF without closing brace");

		strncpy(keyname, com_token, sizeof(keyname)-1);
		
		// parse value	
		com_token = COM_Parse(&data);
		if (!data)
			gi.error ("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			gi.error("ED_ParseEntity: closing brace without data");

		init = true;	

		// keynames with a leading underscore are used for utility comments, and are immediately discarded by quake
		if (keyname[0] == '_')
			continue;

		ED_ParseField(keyname, com_token, ent);
	}

	if (!init)
		memset(ent, 0, sizeof(*ent));

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
void G_FindTeams(void)
{
	edict_t	*e;
	edict_t	*e2;
	edict_t	*chain;
	int		i;
	int		j;
	int		c;
	int		c2;

	c = 0;
	c2 = 0;
	for (i = 1, e = g_edicts+i; i < globals.num_edicts; i++, e++)
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
		for (j = i+1, e2 = e+1; j < globals.num_edicts; j++, e2++)
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

	gi.dprintf("%i teams with %i entities\n", c, c2);
}


/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
void SpawnEntities(char *mapname, char *entities, char *spawnpoint)
{
	edict_t		*ent;
	char		*com_token;
	int			inhibit;
	int			i;
//CW++
	char		text[32];

	for (i = 0; i < (int)maxclients->value; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;

		// clean up any bots if a "gamemap" command has been issued
		if (ent->isabot)
		{
			ClearSlot(ent);
			Bot[ent->client->pers.botindex].ingame = 0;
			ent->isabot = false;
			--NumBotsInGame;
		}
	}
//CW--

	SaveClientData();

	gi.FreeTags(TAG_LEVEL);
	memset(&level, 0, sizeof(level));
	memset(g_edicts, 0, game.maxentities * sizeof(g_edicts[0]));

	strncpy(level.mapname, mapname, sizeof(level.mapname)-1);
	strncpy(game.spawnpoint, spawnpoint, sizeof(game.spawnpoint)-1);

//CW++
	switch ((int)sv_gametype->value)
	{
		case G_FFA:
			Com_sprintf(text, sizeof(text), "Gametype: FFA");
			break;

		case G_CTF:
			Com_sprintf(text, sizeof(text), "Gametype: CTF");
			break;

		case G_TDM:
			Com_sprintf(text, sizeof(text), "Gametype: Team-DM");
			break;

		case G_ASLT:
			Com_sprintf(text, sizeof(text), "Gametype: Assault");
			break;

		default:
			Com_sprintf(text, sizeof(text), "Gametype: UNDEFINED");
			break;
	}

	gi.dprintf("\n[%s]\n", text);
	gi.dprintf("Map: %s\n\n", level.mapname);
//CW--
	
//	Set client fields on player ents.

	for (i = 0; i < game.maxclients; ++i)
		g_edicts[i+1].client = game.clients + i;

	ent = NULL;
	inhibit = 0;

//	Parse ents.

	while (1)
	{
		// parse the opening brace	
		com_token = COM_Parse(&entities);
		if (!entities)
			break;

		if (com_token[0] != '{')
			gi.error("ED_LoadFromFile: found %s when expecting {", com_token);

		if (!ent)
			ent = g_edicts;
		else
			ent = G_Spawn();

		entities = ED_ParseEdict(entities, ent);

		// remove things (except the world) from deathmatch if flagged (ie. SP maps)
		if (ent != g_edicts)
		{
			if (ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH)											//CW
			{
				G_FreeEdict(ent);	
				inhibit++;
				continue;
			}

			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY|SPAWNFLAG_NOT_MEDIUM|SPAWNFLAG_NOT_HARD|SPAWNFLAG_NOT_COOP|SPAWNFLAG_NOT_DEATHMATCH);
		}
		ED_CallSpawn(ent);
	}	

	gi.dprintf("%i entities inhibited\n", inhibit);

	G_FindTeams();

//ZOID++
	CTFSpawn();
//ZOID--

//CW++
	if (sv_gametype->value == G_ASLT)
	{
		ASLTSpawn();
		if (!timelimit->value)
		{
			gi.dprintf("** No info_timelimit found in map.\n   => Forcing timelimit to 10 mins.\n");
			gi.cvar_forceset("timelimit", va("10.0"));
		}
	}

	memset(&g_vote, 0, sizeof(g_vote));

	RemoveDisabledPowerups();
	SetupRndPowerupSpawn();

//	Weapon drop files.

	if ((int)sv_agm_drop->value && (int)sv_allow_agm->value)
		SetupAGMSpawn();

	if ((int)sv_disc_drop->value && (int)sv_allow_disclauncher->value)
		SetupDiscLauncherSpawn();
//CW--

//Maj++
	if (((int)sv_gametype->value == G_FFA) || ((int)sv_gametype->value == G_TDM))						//CW
	{
		G_FindTrainTeam();
		ReadRouteFile();
	}
//Maj--
}


//===================================================================

/*	Description of statusbar variables.

Cursor positioning:
	xl <value>
	xr <value>
	yb <value>
	yt <value>
	xv <value>
	yv <value>

Drawing:
	statpic <name>
	pic <stat>
	num <fieldwidth> <stat>
	string <stat>

Control:
	if <stat>
	ifeq <stat> <value>
	ifbit <stat> <value>
	endif
*/

char *dm_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
"	xv	246 "
"	num	3	10 "																					//CW
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	150 "																						//CW
"	pic	11 "
"endif "

//CW++
// Traps/C4 ID view state
"if 29 "
  "xv 120 "
  "yb -46 "
  "pic 29 "
"endif "

// time remaining
"if 28 "
  "xl 0 "
  "yb -78 "
  "stat_string 28 "
"endif "
//CW--

//  frags
"xr	-50 "
"yt 2 "
"num 3 14"
;


/*QUAKED worldspawn (0 0 0) ?

Only used for the world.
"sky"		environment map name
"skyaxis"	vector axis for rotating sky
"skyrotate"	speed of rotation in degrees/second
"sounds"	music cd track number
"gravity"	800 is default gravity
"message"	text to print at user logon
*/
void SP_worldspawn(edict_t *ent)
{
//CW++
	int		i;
//CW--

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->inuse = true;			// since the world doesn't use G_Spawn()
	ent->s.modelindex = 1;		// world model is always index 1

	//---------------

//CW++
	for (i = MAX_OLDMAPS-1; i > 0; --i)
		strncpy(g_oldmaps[i].mapname, g_oldmaps[i-1].mapname, sizeof(g_oldmaps[i].mapname));
	strncpy(g_oldmaps[0].mapname, level.mapname, sizeof(g_oldmaps[0].mapname));
//CW--

	// reserve some spots for dead player bodies
	InitBodyQue();

	// set configstrings for items
	SetItemNames();

	if (st.nextmap)
		Com_strcpy(level.nextmap, sizeof(level.nextmap), st.nextmap);

	// make some data visible to the server
	if (ent->message && ent->message[0])
	{
		gi.configstring(CS_NAME, ent->message);
		strncpy(level.level_name, ent->message, sizeof(level.level_name));
	}
	else
		strncpy(level.level_name, level.mapname, sizeof(level.level_name));

	if (st.sky && st.sky[0])
		gi.configstring(CS_SKY, st.sky);
	else
		gi.configstring(CS_SKY, "unit1_");

	gi.configstring(CS_SKYROTATE, va("%f", st.skyrotate));
	gi.configstring(CS_SKYAXIS, va("%f %f %f", st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]));

	// Knightmare- if a named soundtrack is specified, play it instead of from CD
	if (ent->musictrack && strlen(ent->musictrack))
		gi.configstring (CS_CDTRACK, ent->musictrack);
	else
		gi.configstring(CS_CDTRACK, va("%i", ent->sounds));
	// end Knightmare

	gi.configstring(CS_MAXCLIENTS, va("%i", (int)(maxclients->value)));

	// status bar program
//ZOID++
	if (sv_gametype->value == G_CTF)																//CW
	{
		gi.configstring(CS_STATUSBAR, ctf_statusbar);
		CTFPrecache();
	}
//ZOID--

//CW++
	else if (sv_gametype->value == G_TDM)
	{
		gi.configstring(CS_STATUSBAR, tdm_statusbar);
		TDMPrecache();
	}
	else if (sv_gametype->value == G_ASLT)
	{
		gi.configstring(CS_STATUSBAR, aslt_statusbar);
		ASLTPrecache();
	}
	else
//CW--
		gi.configstring(CS_STATUSBAR, dm_statusbar);

	//---------------


	// help icon for statusbar
	gi.imageindex("i_help");
	level.pic_health = gi.imageindex("i_health");
	gi.imageindex("help");
	gi.imageindex("field_3");

	if (!st.gravity)
		gi.cvar_set("sv_gravity", "800");
	else
		gi.cvar_set("sv_gravity", st.gravity);

	snd_fry = gi.soundindex("player/fry.wav");

//CW++
	gi.imageindex("i_no");
	gi.modelindex("sprites/point.sp2");

	hook_index = gi.modelindex("models/objects/hook/tris.md2");
	spike_index = gi.modelindex("models/objects/spike/tris.md2");
	r_explode_index = gi.modelindex("models/objects/r_explode/tris.md2");
	tracer_index = gi.modelindex("models/objects/tracer/tris.md2");

//	Precache weapons, ammo and armour if players start with them in their possession.

	if ((int)sv_allow_hook->value)
		PrecacheItem(FindItem("Grapple"));

	PrecacheItem(FindItem("Chainsaw"));

	if (sv_have_deserteagle->value)
		PrecacheItem(FindItem("Desert Eagle"));

	if (sv_have_gausspistol->value)
		PrecacheItem(FindItem("Gauss Pistol"));

	if (sv_have_mac10->value)
		PrecacheItem(FindItem("Mac-10"));

	if (sv_have_jackhammer->value)
		PrecacheItem(FindItem("Jackhammer"));

	if (sv_have_c4->value)
		PrecacheItem(FindItem("C4"));

	if (sv_have_traps->value)
		PrecacheItem(FindItem("Traps"));

	if (sv_have_spikegun->value)
		PrecacheItem(FindItem("E.S.G."));

	if (sv_have_rocketlauncher->value)
		PrecacheItem(FindItem("Rocket Launcher"));

	if (sv_have_flamethrower->value)
		PrecacheItem(FindItem("Flamethrower"));

	if (sv_have_railgun->value)
		PrecacheItem(FindItem("Railgun"));

	if (sv_have_shockrifle->value)
		PrecacheItem(FindItem("Shock Rifle"));

	if (sv_have_agm->value)
		PrecacheItem(FindItem("AG Manipulator"));

	if (sv_have_disclauncher->value)
		PrecacheItem(FindItem("Disc Launcher"));

	if (sv_initial_bullets->value)
		PrecacheItem(FindItem("bullets"));

	if (sv_initial_shells->value)
		PrecacheItem(FindItem("shells"));

	if (sv_initial_rockets->value)
		PrecacheItem(FindItem("rockets"));

	if (sv_initial_cells->value)
		PrecacheItem(FindItem("cells"));

	if (sv_initial_slugs->value)
		PrecacheItem(FindItem("slugs"));

	if ((int)sv_initial_armor->value > 0)
	{
		if ((int)sv_initial_armortype->value == 0)
			PrecacheItem(FindItem("Jacket Armor"));
		else if ((int)sv_initial_armortype->value == 1)
			PrecacheItem(FindItem("Combat Armor"));
		else
			PrecacheItem(FindItem("Body Armor"));
	}

//	Precache Mystery Box, Pack and Adrenaline powerups if extra items are flagged.
	
	if ((int)dmflags->value & DF_EXTRA_ITEMS)
	{
		PrecacheItem(FindItem("Mystery"));
		PrecacheItem(FindItem("Ammo Pack"));
		PrecacheItem(FindItem("Adrenaline"));
	}
//CW--

	gi.soundindex("player/lava1.wav");
	gi.soundindex("player/lava2.wav");
	gi.soundindex("misc/pc_up.wav");
	gi.soundindex("misc/talk1.wav");
	gi.soundindex("misc/udeath.wav");
	gi.soundindex("items/respawn1.wav");

	// sexed sounds
	gi.soundindex("*death1.wav");
	gi.soundindex("*death2.wav");
	gi.soundindex("*death3.wav");
	gi.soundindex("*death4.wav");
	gi.soundindex("*fall1.wav");
	gi.soundindex("*fall2.wav");	
	gi.soundindex("*gurp1.wav");
	gi.soundindex("*gurp2.wav");	
	gi.soundindex("*jump1.wav");
	gi.soundindex("*pain25_1.wav");
	gi.soundindex("*pain25_2.wav");
	gi.soundindex("*pain50_1.wav");
	gi.soundindex("*pain50_2.wav");
	gi.soundindex("*pain75_1.wav");
	gi.soundindex("*pain75_2.wav");
	gi.soundindex("*pain100_1.wav");
	gi.soundindex("*pain100_2.wav");

	// sexed models (max 15)
	// NB: this order must match the #defines in g_local.h
//CW++
	gi.modelindex("#w_chainsaw.md2");
	gi.modelindex("#w_de.md2");
	gi.modelindex("#w_gauss.md2");
	gi.modelindex("#w_jhammer.md2");
	gi.modelindex("#w_macten.md2");
	gi.modelindex("#w_esg.md2");
	gi.modelindex("#a_grenades.md2");
	gi.modelindex("#w_trap.md2");
	gi.modelindex("#w_rlauncher.md2");
	gi.modelindex("#w_fthrower.md2");
	gi.modelindex("#w_railgun.md2");
	gi.modelindex("#w_shock.md2");
	gi.modelindex("#w_dlauncher.md2");
	gi.modelindex("#w_agm.md2");
	gi.modelindex("#w_grapple.md2");
//CW--

	//-------------------

	gi.soundindex("player/gasp1.wav");		// gasping for air
	gi.soundindex("player/gasp2.wav");		// head breaking surface, not gasping
	gi.soundindex("player/watr_in.wav");	// feet hitting water
	gi.soundindex("player/watr_out.wav");	// feet leaving water
	gi.soundindex("player/watr_un.wav");	// head going underwater
	gi.soundindex("player/u_breath1.wav");
	gi.soundindex("player/u_breath2.wav");
	gi.soundindex("items/pkup.wav");		// bonus item pickup
	gi.soundindex("world/land.wav");		// landing thud
	gi.soundindex("misc/h2ohit1.wav");		// landing splash
	gi.soundindex("items/damage.wav");
	gi.soundindex("items/protect.wav");
	gi.soundindex("items/protect4.wav");
	gi.soundindex("weapons/noammo.wav");

	sm_meat_index = gi.modelindex("models/objects/gibs/sm_meat/tris.md2");
	gi.modelindex("models/objects/gibs/arm/tris.md2");
	gi.modelindex("models/objects/gibs/bone/tris.md2");
	gi.modelindex("models/objects/gibs/bone2/tris.md2");
	gi.modelindex("models/objects/gibs/chest/tris.md2");
	gi.modelindex("models/objects/gibs/skull/tris.md2");
	gi.modelindex("models/objects/gibs/head2/tris.md2");

	// cvar overrides for effects flags:
	if (footstep_sounds->value)
		world->effects |= FX_WORLDSPAWN_STEPSOUNDS;

//	Setup light animation tables. 'a' is total darkness, 'z' is doublebright.

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
}

//CW++
// Random powerups are spawned at the DM pads if the appropriate dmflag is set.

void SpawnPowerup(gitem_t *item, edict_t *spot);

void PowerupThink(edict_t *powerup)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL)
	{
		SpawnPowerup(powerup->item, spot);
		G_FreeEdict(powerup);
	}
	else
	{
		powerup->nextthink = level.time + sv_rnd_powerup_timeout->value;
		powerup->think = PowerupThink;
	}
}

void SpawnPowerup(gitem_t *item, edict_t *spot)
{
	edict_t	*ent;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	angles;

	ent = G_Spawn();

	ent->classname = item->classname;
	ent->item = item;
	ent->spawnflags = DROPPED_ITEM;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	VectorSet(ent->mins, -15.0, -15.0, -15.0);
	VectorSet(ent->maxs, 15.0, 15.0, 15.0);
	gi.setmodel(ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;
	ent->owner = ent;
	ent->rnd_spawn = true;

	angles[0] = 0;
	angles[1] = rand() % 360;
	angles[2] = 0;

	AngleVectors(angles, forward, right, NULL);
	VectorCopy(spot->s.origin, ent->s.origin);
	ent->s.origin[2] += 16.0;
	VectorScale(forward, 300.0, ent->velocity);
	ent->velocity[2] = 300.0;

	gi.linkentity(ent);
}

void SpawnRndPowerups(edict_t *spawner)
{
	gitem_t		*powerup;
	edict_t		*spot;
	edict_t		*ent;
	qboolean	mys_valid = false;
	int			i;
	int			dm_num = 0;

	qboolean	found = false;

	if (!spawner)
		return;

//	Remove any currently existing randomly-spawned powerups.

	for (i = 0; i < globals.num_edicts; ++i)
	{
		ent = &g_edicts[i];
		if (ent->rnd_spawn)
		{
			found = true;
			G_FreeEdict(ent);
		}
	}

	if (found)
	{
		spawner->nextthink = level.time + 1.0;
		return;
	}
	else
		spawner->nextthink = level.time + sv_rnd_powerup_timeout->value;

//	Spawn new ones at random DM points.

	i = 0;
	mys_valid = (int)(sv_mystery_invuln->value)|(int)(sv_mystery_quad->value)|(int)(sv_mystery_d89->value)|(int)(sv_mystery_haste->value)|
				(int)(sv_mystery_siphon->value)|(int)(sv_mystery_antibeam->value)|(int)(sv_mystery_enviro->value)|(int)(sv_mystery_tele->value);
	while (pupnames[i])
	{
		if (((powerup = FindItemByClassname(pupnames[i++])) != NULL) && ((spot = FindTechSpawn()) != NULL))
		{
			if (!Q_stricmp(powerup->classname, "item_mystery") && !mys_valid)
				continue;

			SpawnPowerup(powerup, spot);
		}
	}

//	Spawn extra Mystery Boxes proportional to the number of DM spawn points.

	if (!mys_valid)
		return;

	if ((powerup = FindItemByClassname("item_mystery")) == NULL)
		return;

	for (i = (int)maxclients->value+1; i < globals.num_edicts; ++i)
	{
		spot = &g_edicts[i];
		if (spot->classname && (!Q_stricmp(spot->classname, "info_player_deathmatch")))
			++dm_num;
	}

	if (dm_num >= 2 * (int)sv_rnd_mystery_ratio->value)
	{
		dm_num = (int)(dm_num / sv_rnd_mystery_ratio->value);
		for (i = 1; i < dm_num; ++i)
		{
			if ((spot = FindTechSpawn()) != NULL)
				SpawnPowerup(powerup, spot);
		}
	}
}

void SetupRndPowerupSpawn(void)
{
	edict_t *ent;

	if (!((int)dmflags->value & DF_EXTRA_ITEMS))
		return;

	ent = G_Spawn();
	ent->classname = "powerup_spawner";
	ent->svflags |= SVF_NOCLIENT;
	ent->nextthink = level.time + 10.0;
	ent->think = SpawnRndPowerups;
}
//CW--

//CW++
void DropAGM(float x, float y, float z)
{
	edict_t	*agm;

//	Spawn an AGM at the specified position.

	agm = G_Spawn();
	agm->classname = "weapon_agm";
	agm->s.origin[0] = x;
	agm->s.origin[1] = y;
	agm->s.origin[2] = z;
	SpawnItem(agm, FindItem("AG Manipulator"));
	gi.linkentity(agm);

	gi.dprintf("** AGM dropped at %s\n", vtos(agm->s.origin));
}

void SetupAGMSpawn(void)
{
	FILE	*entstream;
	char	bspname[MAX_QPATH];
	int		fnum;
	int		n_agm = 0;
	float	x = 0.0;
	float	y = 0.0;
	float	z = 0.0;

	if ((entstream = OpenAGMDropFile(true, true)) != NULL)
	{
		while ((fnum = fscanf(entstream, "%s %f %f %f", bspname, &x, &y, &z)) != EOF)
		{
			if (!Q_stricmp(level.mapname, bspname))
			{
				DropAGM(x, y, z);
				++n_agm;
			}
		}
		fclose(entstream);

		if (n_agm == 0)
			gi.dprintf("** Current bsp is not listed in AGM drop file\n");
	}
}

void DropDL(float x, float y, float z)
{
	edict_t	*dl;

//	Spawn an AGM at the specified position.

	dl = G_Spawn();
	dl->classname = "weapon_disclauncher";
	dl->s.origin[0] = x;
	dl->s.origin[1] = y;
	dl->s.origin[2] = z;
	SpawnItem(dl, FindItem("Disc Launcher"));
	gi.linkentity(dl);

	gi.dprintf("** Disc Launcher dropped at %s\n", vtos(dl->s.origin));
}

void SetupDiscLauncherSpawn(void)
{
	FILE	*entstream;
	char	bspname[MAX_QPATH];
	int		fnum;
	int		n_dl = 0;
	float	x = 0.0;
	float	y = 0.0;
	float	z = 0.0;

	if ((entstream = OpenDiscLauncherDropFile(true, true)) != NULL)
	{
		while ((fnum = fscanf(entstream, "%s %f %f %f", bspname, &x, &y, &z)) != EOF)
		{
			if (!Q_stricmp(level.mapname, bspname))
			{
				DropDL(x, y, z);
				++n_dl;
			}
		}
		fclose(entstream);

		if (n_dl == 0)
			gi.dprintf("** Current bsp is not listed in Disc Launcher drop file\n");
	}
}

void RemoveDisabledPowerups(void)
{
//	Removes powerups which have been flagged as disabled by sv_allow_* cvars.

	edict_t	*e;
	int		i;

	for (i = 0; i < globals.num_edicts; ++i)
	{
		e = &g_edicts[i];
		if (!e->inuse)
			continue;
		if (!e->item)
			continue;
		if (!(e->item->flags & IT_POWERUP))
			continue;

		if (!(int)sv_allow_invuln->value && (e->item->tag == POWERUP_INVULN))
			G_FreeEdict(e);
		else if (!(int)sv_allow_quad->value && (e->item->tag == POWERUP_QUAD))
			G_FreeEdict(e);
		else if (!(int)sv_allow_siphon->value && (e->item->tag == POWERUP_SIPHON))
			G_FreeEdict(e);
		else if (!(int)sv_allow_d89->value && (e->item->tag == POWERUP_D89))
			G_FreeEdict(e);
		else if (!(int)sv_allow_haste->value && (e->item->tag == POWERUP_HASTE))
			G_FreeEdict(e);
		else if (!(int)sv_allow_tele->value && (e->item->tag == POWERUP_TELE))
			G_FreeEdict(e);
		else if (!(int)sv_allow_antibeam->value && (e->item->tag == POWERUP_ANTIBEAM))
			G_FreeEdict(e);
		else if (!(int)sv_allow_enviro->value && (e->item->tag == POWERUP_ENVIRO))
			G_FreeEdict(e);
		else if (!(int)sv_allow_silencer->value && (e->item->tag == POWERUP_SILENCER))
			G_FreeEdict(e);
		else if (!(int)sv_allow_breather->value && (e->item->tag == POWERUP_BREATHER))
			G_FreeEdict(e);
	}
}
//CW--
