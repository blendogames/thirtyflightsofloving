
#include "g_local.h"
#include "g_map_mod.h"
#include "aj_confcycle.h" // AJ

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
	{"message", FOFS(message), F_LSTRING},
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
	// Knightmare- hack for setting alpha, allows mappers to specify
	// an entity's alpha value with the key "salpha"
#ifdef NEW_ENTITY_STATE_MEMBERS
	{"salpha", FOFS(s.alpha), F_FLOAT},
#endif

	{"mass", FOFS(mass), F_INT},
	{"volume", FOFS(volume), F_FLOAT},
	{"attenuation", FOFS(attenuation), F_FLOAT},
	{"map", FOFS(map), F_LSTRING},

	// Knightmare added
	{"musictrack", FOFS(musictrack), F_LSTRING},	// Knightmare- for specifying OGG or CD track

	// model_spawn stuff
	{"usermodel", FOFS(usermodel), F_LSTRING},
	{"startframe", FOFS(startframe), F_INT},
	{"framenumbers", FOFS(framenumbers), F_INT},
	{"solidstate", FOFS(solidstate), F_INT},
	{"renderfx", FOFS(renderfx), F_INT},
	{"effects", FOFS(effects), F_INT},
	{"skinnum", FOFS(skinnum), F_INT},
	{"bleft", FOFS(bleft), F_VECTOR},
	{"tright", FOFS(tright), F_VECTOR},
	// rotating train stuff
	{"pitch_speed", FOFS(pitch_speed), F_FLOAT},
	{"yaw_speed", FOFS(yaw_speed), F_FLOAT},
	{"roll_speed", FOFS(roll_speed), F_FLOAT},
	{"ideal_pitch", FOFS(ideal_pitch), F_FLOAT},
	{"ideal_yaw", FOFS(ideal_yaw), F_FLOAT},
	{"ideal_roll", FOFS(ideal_roll), F_FLOAT},
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
	// gib fields
	{"blood_type", FOFS(blood_type), F_INT},
	{"gib_health", FOFS(gib_health), F_INT},
	{"gib_type", FOFS(gib_type), F_INT},
	// Knightmare- new function pointers
	{"play", FOFS(play), F_FUNCTION, FFL_NOSPAWN},
	{"postthink", FOFS(postthink), F_FUNCTION, FFL_NOSPAWN},
	// end Knightmare

	// Knightmare added
	{"phase", STOFS(phase), F_FLOAT, FFL_SPAWNTEMP},
	{"shift", STOFS(shift), F_FLOAT, FFL_SPAWNTEMP},
	{"skydistance", STOFS(skydistance), F_FLOAT, FFL_SPAWNTEMP},
	{"cloudname", STOFS(cloudname), F_LSTRING, FFL_SPAWNTEMP},
	{"lightningfreq", STOFS(lightningfreq), F_FLOAT, FFL_SPAWNTEMP|FFL_DEFAULT_NEG},
	{"cloudxdir", STOFS(cloudxdir), F_FLOAT, FFL_SPAWNTEMP|FFL_DEFAULT_NEG},
	{"cloudydir", STOFS(cloudydir), F_FLOAT, FFL_SPAWNTEMP|FFL_DEFAULT_NEG},
	{"cloud1tile", STOFS(cloud1tile), F_FLOAT, FFL_SPAWNTEMP|FFL_DEFAULT_NEG},
	{"cloud1speed", STOFS(cloud1speed), F_FLOAT, FFL_SPAWNTEMP|FFL_DEFAULT_NEG},
	{"cloud1alpha", STOFS(cloud1alpha), F_FLOAT, FFL_SPAWNTEMP|FFL_DEFAULT_NEG},
	{"cloud2tile", STOFS(cloud2tile), F_FLOAT, FFL_SPAWNTEMP|FFL_DEFAULT_NEG},
	{"cloud2speed", STOFS(cloud2speed), F_FLOAT, FFL_SPAWNTEMP|FFL_DEFAULT_NEG},
	{"cloud2alpha", STOFS(cloud2alpha), F_FLOAT, FFL_SPAWNTEMP|FFL_DEFAULT_NEG},
	{"cloud3tile", STOFS(cloud3tile), F_FLOAT, FFL_SPAWNTEMP|FFL_DEFAULT_NEG},
	{"cloud3speed", STOFS(cloud3speed), F_FLOAT, FFL_SPAWNTEMP|FFL_DEFAULT_NEG},
	{"cloud3alpha", STOFS(cloud3alpha), F_FLOAT, FFL_SPAWNTEMP|FFL_DEFAULT_NEG},
	{"radius", STOFS(radius), F_FLOAT, FFL_SPAWNTEMP},
	{"alpha", STOFS(alpha), F_FLOAT, FFL_SPAWNTEMP},
	{"fade_start_dist", STOFS(fade_start_dist), F_INT, FFL_SPAWNTEMP},
	{"fade_end_dist", STOFS(fade_end_dist), F_INT, FFL_SPAWNTEMP},
	{"image", STOFS(image), F_LSTRING, FFL_SPAWNTEMP},
	{"rgba", STOFS(rgba), F_LSTRING, FFL_SPAWNTEMP},
	{"spritetype", STOFS(spritetype), F_INT, FFL_SPAWNTEMP},

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
	{"nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP}
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
	{"", LLOFS(changemap), F_LSTRING},

	{"", LLOFS(sight_client), F_EDICT},
	{"", LLOFS(sight_entity), F_EDICT},
	{"", LLOFS(sound_entity), F_EDICT},
	{"", LLOFS(sound2_entity), F_EDICT},

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
only happens when a new game is begun
============
*/
qboolean default_exec = false;

void InitGame (void)
{

	gi.dprintf ("==== InitGame (Eraser ULTRA) ====\n");

// AJ
	lithium_defaults ();
	read_configlist ();
// end AJ

	gun_x = gi.cvar ("gun_x", "0", 0);
	gun_y = gi.cvar ("gun_y", "0", 0);
	gun_z = gi.cvar ("gun_z", "0", 0);

	//FIXME: sv_ prefix is wrong for these
	sv_rollspeed = gi.cvar ("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar ("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar ("sv_maxvelocity", "2000", 0);
	sv_gravity = gi.cvar ("sv_gravity", "800", 0);

	sv_stopspeed = gi.cvar ("sv_stopspeed", "100", 0);		// PGM - was #define in g_phys.c

//ROGUE
	g_showlogic = gi.cvar ("g_showlogic", "0", 0);
	huntercam = gi.cvar ("huntercam", "1", CVAR_SERVERINFO|CVAR_LATCH);
	strong_mines = gi.cvar ("strong_mines", "0", 0);
	randomrespawn = gi.cvar ("randomrespawn", "0", 0);
//ROGUE

	// noset vars
	dedicated = gi.cvar ("dedicated", "0", CVAR_NOSET);

	// latched vars
	sv_cheats = gi.cvar ("cheats", "0", CVAR_SERVERINFO|CVAR_LATCH);
	gi.cvar ("gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar ("gamedate", __DATE__ , CVAR_SERVERINFO | CVAR_LATCH);

	maxclients = gi.cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	deathmatch = gi.cvar ("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar ("coop", "0", CVAR_LATCH);
	skill = gi.cvar ("skill", "3", CVAR_LATCH);
	maxentities = gi.cvar ("maxentities", va("%i", MAX_EDICTS), CVAR_LATCH); //was 1024
	gamerules = gi.cvar ("gamerules", "0", CVAR_LATCH);			//PGM

//ZOID
//This game.dll only supports deathmatch
	if (!deathmatch->value) {
		gi.dprintf("Forcing deathmatch.");
		gi.cvar_set("deathmatch", "1");
	}
	//force coop off
	if (coop->value)
		gi.cvar_set("coop", "0");
//ZOID

	// change anytime vars
	dmflags = gi.cvar ("dmflags", "0", CVAR_SERVERINFO); // AJ removed | CVAR_ARCHIVE
	fraglimit = gi.cvar ("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar ("timelimit", "0", CVAR_SERVERINFO);
//ZOID
	capturelimit = gi.cvar ("capturelimit", "0", CVAR_SERVERINFO);
//ZOID
	passwd = gi.cvar ("password", "", CVAR_USERINFO);

	g_select_empty = gi.cvar ("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar ("run_pitch", "0.002", 0);
	run_roll = gi.cvar ("run_roll", "0.005", 0);
	bob_up  = gi.cvar ("bob_up", "0.005", 0);
	bob_pitch = gi.cvar ("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar ("bob_roll", "0.002", 0);

	// AJ - the "fun" grappling hook stuff
	sk_hook_is_homing     = gi.cvar ("sk_hook_is_homing", "1", CVAR_SERVERINFO);
	sk_hook_homing_radius = gi.cvar ("sk_hook_homing_radius", "200", CVAR_SERVERINFO);
	sk_hook_homing_factor = gi.cvar ("sk_hook_homing_factor", "5", CVAR_SERVERINFO);
	sk_hook_players       = gi.cvar ("sk_hook_players", "1", CVAR_SERVERINFO);
//	sk_hook_sky           = gi.cvar ("sk_hook_sky", "1", CVAR_SERVERINFO);
	sk_hook_min_length    = gi.cvar ("sk_hook_min_length", "20", CVAR_SERVERINFO);
	sk_hook_max_length    = gi.cvar ("sk_hook_max_length", "2000", CVAR_SERVERINFO);
	sk_hook_pull_speed    = gi.cvar ("sk_hook_pull_speed", "80", CVAR_SERVERINFO);
	sk_hook_fire_speed    = gi.cvar ("sk_hook_fire_speed", "2000", CVAR_SERVERINFO);
	sk_hook_messages      = gi.cvar ("sk_hook_messages", "1", CVAR_SERVERINFO);
	sk_hook_vampirism     = gi.cvar ("sk_hook_vampirism", "0", CVAR_SERVERINFO);
	sk_hook_vampire_ratio = gi.cvar ("sk_hook_vampire_ratio", "0.5", CVAR_SERVERINFO);
	// end AJ

	// bot commands
	bot_num = gi.cvar ("bot_num", "0", 0);
	bot_name = gi.cvar ("bot_name", "", 0);
	bot_allow_client_commands = gi.cvar ("bot_allow_client_commands", "0", CVAR_ARCHIVE);
	bot_free_clients = gi.cvar ("bot_free_clients", "0", CVAR_ARCHIVE);
	bot_debug = gi.cvar ("bot_debug", "0", 0);
	bot_show_connect_info = gi.cvar ("bot_show_connect_info", "1", CVAR_ARCHIVE);
	bot_calc_nodes = gi.cvar ("bot_calc_nodes", "1", 0);
	bot_debug_nodes = gi.cvar ("bot_debug_nodes", "0", 0);
	bot_auto_skill = gi.cvar ("bot_auto_skill", "0", CVAR_ARCHIVE);
	bot_drop = gi.cvar ("bot_drop", "", 0);
	bot_chat = gi.cvar ("bot_chat", "1", CVAR_ARCHIVE);
	bot_optimize = gi.cvar ("bot_optimize", "1200", 0);
	bot_tarzan = gi.cvar ("bot_tarzan", "0", CVAR_ARCHIVE);

	players_per_team = gi.cvar ("players_per_team", "4", CVAR_LATCH|CVAR_ARCHIVE);
	addteam = gi.cvar ("addteam", "", 0);
	teamplay = gi.cvar ("teamplay", "0", CVAR_LATCH|CVAR_ARCHIVE);
	ctf_auto_teams = gi.cvar ("ctf_auto_teams", "0", 0);
	ctf_special_teams = gi.cvar ("ctf_special_teams", "0", 0); //~~JLH
	ctf_humanonly_teams = gi.cvar ("ctf_humanonly_teams", "0", 0); //~~JLH
    // Diable ctf_auto_teams if ctf_special_teams is enabled //~~JLH
    if ( ctf_auto_teams->value > 0 && ctf_special_teams->value > 0 )  // ~~JLH
		gi.cvar_set ("ctf_auto_teams", "0");
	grapple = gi.cvar ("grapple", "0", CVAR_LATCH);

	view_weapons = gi.cvar ("view_weapons", "0", CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);

	turn_rider = gi.cvar("turn_rider", "1", CVAR_SERVERINFO);	// Knightmare added

	g_aimfix = gi.cvar ("g_aimfix", "0", CVAR_ARCHIVE);								// Knightmare- from Yamagi Q2
	g_aimfix_min_dist = gi.cvar ("g_aimfix_min_dist", "128", CVAR_ARCHIVE);			// Knightmare- minimum range for aimfix
	g_aimfix_taper_dist = gi.cvar ("g_aimfix_taper_dist", "128", CVAR_ARCHIVE);	// Knightmare- transition range for aimfix

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

    // MAP MOD
	map_mod_set_up();
	// MAP MOD

//ZOID
	CTFInit();
//ZOID
//======
//ROGUE
	if (gamerules)
	{
		InitGameRules ();	// if there are game rules to set up, do so now.
	}
//ROGUE
//======

/*	{
		size_t	size1, size2, size3, size4, totalSize;
		struct _timeb	testTime;
		size1 = sizeof(testTime.time);
		size2 = sizeof(testTime.millitm);
		size3 = sizeof(testTime.timezone);
		size4 = sizeof(testTime.dstflag);
		totalSize = sizeof(testTime);
		gi.dprintf ("Size of _timeb struct members: %d %d %d %d\n", size1, size2, size3, size4);
		gi.dprintf ("Size of _timeb: %d\n", totalSize);
	}*/

/*	{
		size_t	ofs1, ofs2, ofs3;
		ofs1 = (size_t)&(((edict_t *)0)->waterlevel) - (size_t)&(((edict_t *)0)->client);
		ofs2 = (size_t)&(((edict_t *)0)->node_target) - (size_t)&(((edict_t *)0)->client);
		ofs3 = (size_t)&(((edict_t *)0)->avoid_ent) - (size_t)&(((edict_t *)0)->client);
		gi.dprintf ("Offset of waterlevel from client ptr in edict_t: %d\n", ofs1);
		gi.dprintf ("Offset of node_target from client ptr in edict_t: %d\n", ofs2);
		gi.dprintf ("Offset of avoid_ent from client ptr in edict_t: %d\n", ofs3);
	}*/
}

//=========================================================

#ifdef _MSC_VER
#pragma warning(disable : 4054)	// type cast for function pointers
#endif	// _MSC_VER

void WriteField1 (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

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
			len = (int)strlen(*(char **)p) + 1;
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

	default:
		gi.error ("WriteEdict: unknown field type");
	}
}

void WriteField2 (FILE *f, field_t *field, byte *base)
{
	int			len;
	void		*p;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_LSTRING:
	case F_GSTRING:
		if ( *(char **)p )
		{
			len = (int)strlen(*(char **)p) + 1;
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

	if (!autosave)
		SaveClientData ();

	f = fopen (filename, "wb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	memset (str, 0, sizeof(str));
	Com_strcpy (str, sizeof(str), __DATE__);
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
	for (field=savefields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=savefields ; field->name ; field++)
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

	fread (ent, sizeof(*ent), 1, f);

	for (field=savefields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)ent);
	}
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

SpawnEntities will already have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines
set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
=================


*/
void ReadLevel (char *filename)
{
	int		entnum;
	FILE	*f;
	int		i;
	void	*base;
	edict_t	*ent;

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
	if (base != (void *)InitGame)
	{
		fclose (f);
		gi.error ("ReadLevel: function pointers have moved");
	}

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
}

