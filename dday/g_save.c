/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_save.c,v $
 *   $Revision: 1.17 $
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

	{"ammo_type", FOFS(killtarget), F_LSTRING},//turret
	{"fire_sound", FOFS(obj_name), F_LSTRING},//turret

	{"wait", FOFS(wait), F_FLOAT},
	{"delay", FOFS(delay), F_FLOAT},
	{"random", FOFS(random), F_FLOAT},

	{"rate", FOFS(pain_debounce_time), F_FLOAT},//turret

	{"move_origin", FOFS(move_origin), F_VECTOR},
	{"move_angles", FOFS(move_angles), F_VECTOR},
	{"style", FOFS(style), F_INT},
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"sounds", FOFS(sounds), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(dmg), F_INT},

	{"ammo_speed", FOFS(numfired), F_INT},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},
	{"mass", FOFS(mass), F_INT},
	{"volume", FOFS(volume), F_FLOAT},
	{"attenuation", FOFS(attenuation), F_FLOAT},
	{"map", FOFS(map), F_LSTRING},

	{"kills", FOFS(dmg), F_INT},
	{"points", FOFS(health), F_INT},

	{"botfiles", FOFS(deathtarget), F_LSTRING},//turret

//these are for SP_info_Max_MOS
/*faf	{"officer",FOFS(spawnflags),F_INT},
	{"l_gunner",FOFS(style),F_INT},
	{"h_gunner",FOFS(count),F_INT},
	{"sniper",FOFS(health),F_INT},
	{"airborne",FOFS(sounds),F_INT},
	{"engineer",FOFS(dmg),F_INT},
	{"medic",FOFS(takedamage),F_INT},
	{"flamer",FOFS(mass),F_INT}, */

	{"frames",FOFS(mass),F_INT},//faf:  for animated misc_md2

//these are for SP_info_Skin
//	{"skindir",FOFS(pathtarget),F_LSTRING},

	{"md2",FOFS(pathtarget),F_LSTRING},//faf: turret md2s

	{"stance", FOFS(sounds), F_INT}, //turret: forces stance stanceflags
	{"tank",FOFS(obj_gain),F_INT},//turret:  tank 1 will make player disappear


	{"skin",FOFS(deathtarget),F_LSTRING},//faf: for custom skins, mapper sets this in info_team_start

	{"min", FOFS(move_origin), F_VECTOR},
	{"max", FOFS(move_angles), F_VECTOR},//faf:  for spawn_protect

//these are for SP_event_Arty_Battery
	{"guns",FOFS(health),F_INT},
	{"volly",FOFS(sounds),F_INT},
	{"round_type",FOFS(mass),F_INT},
//these are for SP_info_Arty_strike
	{"firing_bat",FOFS(count),F_INT},
	{"time_to_fire",FOFS(health),F_INT},
	{"accuracy",FOFS(dmg),F_INT},
	{"strike_type",FOFS(mass),F_INT},

	{"turret_sound",FOFS(map), F_LSTRING},


//these are for SP_info_Mission_Results
	{"nextmap",FOFS(map), F_LSTRING},
#ifdef KMQUAKE2_ENGINE_MOD
	{"salpha", FOFS(s.alpha), F_FLOAT}, // Knightmare- hack for setting alpha
#endif
	{"musictrack", FOFS(musictrack), F_LSTRING},	// Knightmare- for specifying OGG or CD track
	{"map1",FOFS(map), F_LSTRING},
	{"team1",FOFS(dmg), F_INT},
	{"map2",FOFS(classname), F_LSTRING},
	{"team2",FOFS(mass),F_INT},
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
	{"fade_start_dist", STOFS(fade_start_dist), F_INT, FFL_SPAWNTEMP},
	{"fade_end_dist", STOFS(fade_end_dist), F_INT, FFL_SPAWNTEMP},
	{"image", STOFS(image), F_LSTRING, FFL_SPAWNTEMP},
	{"rgba", STOFS(rgba), F_LSTRING, FFL_SPAWNTEMP},

//added by kmm
	{"dll",FOFS(pathtarget),F_LSTRING},
	{"obj_name",FOFS(obj_name),F_LSTRING},
	{"obj_area",FOFS(obj_area),F_FLOAT},
	{"obj_time",FOFS(obj_time),F_FLOAT},
	{"obj_owner",FOFS(obj_owner),F_INT},//pointer to who currently owns target
	{"obj_gain",FOFS(obj_gain),F_INT},
	{"obj_loss",FOFS(obj_loss),F_INT},
	{"obj_count",FOFS(obj_count),F_INT},
	{"obj_perm_owner",FOFS(obj_perm_owner),F_INT},

	{"chute",FOFS(obj_count),F_INT},

	{"weight", STOFS(weight), F_INT, FFL_SPAWNTEMP},//JABot

	{"fullbright",FOFS(groundentity_linkcount),F_INT},
	{"fog",FOFS(teleport_time),F_FLOAT},
	{"obj_origin", FOFS(obj_origin), F_VECTOR}




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
only happens when a new game is started or a save game
is loaded.
============
*/

int			GlobalAliciaModeVariable	= 0;

void InitGame (void)
{
	gi.dprintf ("==== InitGame (D-Day) ====\n");

#ifdef DEBUG
	gi.dprintf ("-- DEBUG  BUILD --\n");
#endif // DEBUG


	user_shout_count = 0;

	gun_x = gi.cvar ("gun_x", "0", 0);
	gun_y = gi.cvar ("gun_y", "0", 0);
	gun_z = gi.cvar ("gun_z", "0", 0);

	GlobalAliciaModeVariable=0;

	//FIXME: sv_ prefix is wrong for these
	sv_rollspeed		=	gi.cvar ("sv_rollspeed",		"200",  0);
	sv_rollangle		=	gi.cvar ("sv_rollangle",		"2",    0);
	sv_maxvelocity		=	gi.cvar ("sv_maxvelocity",		"2000", 0);
	sv_gravity			=	gi.cvar ("sv_gravity",			"800",  0);

	//Flyingnun Password
	flyingnun_password  =   gi.cvar("observer_password",	"",   0);

	//KMM: New CVARS
//	scope_setting		=	gi.cvar("scope_setting",		"25", CVAR_SERVERINFO);
//	auto_reload			=	gi.cvar("auto_reload",			"0",  CVAR_LATCH);
//	auto_weapon_change	=	gi.cvar("auto_weapon_change",	"0",  CVAR_LATCH);
//	observer_on_death	=	gi.cvar("observer_on_death",	"1",  CVAR_LATCH);
	//heal_wounds		=	gi.cvar("heal_wounds",			"1",  0);
//	flame_shots			=	gi.cvar("flame_shots",			"1",  0);
	RI					=	gi.cvar("RI",					"10", CVAR_SERVERINFO);
	team_kill			=	gi.cvar("team_kill",			"0",  CVAR_SERVERINFO);
	class_limits		=	gi.cvar("class_limits",			"1",  CVAR_SERVERINFO | CVAR_LATCH);
	spawn_camp_check	=	gi.cvar("spawn_camp_check",		"1",  0);//CVAR_SERVERINFO);
	spawn_camp_time		=	gi.cvar("spawn_camp_time",		"2",  0);//CVAR_SERVERINFO);
//	teamgren			=	gi.cvar("teamgren",				"1",  CVAR_SERVERINFO | CVAR_LATCH);
	death_msg			=	gi.cvar("death_msg",			"1",  CVAR_SERVERINFO);
	level_wait			=	gi.cvar("level_wait",			"20",  CVAR_SERVERINFO);
	invuln_spawn		=	gi.cvar("invuln_spawn",			"3", 0);//CVAR_SERVERINFO);

	// artillary
	arty_delay			=	gi.cvar("arty_delay",			"10", 0);
	arty_time			=	gi.cvar("arty_time",			"60", 0);
	arty_max			=	gi.cvar("arty_max",				"1",  0);

//bcass start - easter_egg cvar, AGAIN
	easter_egg			=	gi.cvar("easter_egg",			"1",  0);
	//bcass end

	// noset vars
	dedicated			=	gi.cvar ("dedicated",			"0",  CVAR_NOSET);

	// flood protection
	flood_msgs			=	gi.cvar ("flood_msgs",			"4",  0);
	flood_persecond		=	gi.cvar ("flood_persecond",		"4",  0);
	flood_waitdelay		=	gi.cvar ("flood_waitdelay",		"10", 0);

	// latched vars
	sv_cheats = gi.cvar ("cheats",	"0",			CVAR_SERVERINFO | CVAR_LATCH);
				gi.cvar ("gamename", GAMEVERSION ,	CVAR_SERVERINFO | CVAR_LATCH);
				gi.cvar ("gamedate", __DATE__ ,		CVAR_SERVERINFO | CVAR_LATCH);



	gi.cvar ("dll_version", "Dday 5.063",  CVAR_SERVERINFO | CVAR_LATCH); //faf

	gi.cvar ("website", "http://www.DdayDev.com",  CVAR_SERVERINFO | CVAR_LATCH); //faf

	sv_maplist = gi.cvar ("sv_maplist", "dday inland invade soviet desert poldday itadday", 0); //faf: from q2 code


	maxclients  = gi.cvar ("maxclients",	"8",	CVAR_SERVERINFO | CVAR_LATCH);
	deathmatch  = gi.cvar ("deathmatch",	"1",	CVAR_LATCH);
	coop	    = gi.cvar ("coop",			"0",	CVAR_LATCH);
	skill	    = gi.cvar ("skill",			"1",	CVAR_LATCH);
	// Knightmare- increase maxentities
//	maxentities = gi.cvar ("maxentities",	"1024", CVAR_LATCH);
	maxentities = gi.cvar ("maxentities", va("%i",MAX_EDICTS), CVAR_LATCH);

	// change anytime vars
	dmflags		= gi.cvar ("dmflags",		"0", CVAR_SERVERINFO);
	fraglimit	= gi.cvar ("fraglimit",		"0", CVAR_SERVERINFO);
	timelimit	= gi.cvar ("timelimit",		"0", CVAR_SERVERINFO);
	password	= gi.cvar ("password",		"",	 CVAR_USERINFO);

	g_select_empty = gi.cvar ("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch	= gi.cvar ("run_pitch", "0.002", 0);
	run_roll	= gi.cvar ("run_roll",	"0.005", 0);
	bob_up		= gi.cvar ("bob_up",	"0.005", 0);
	bob_pitch	= gi.cvar ("bob_pitch", "0.002", 0);
	bob_roll	= gi.cvar ("bob_roll",	"0.002", 0);

	player_scores   = gi.cvar ("player_scores",  "1", 0);//faf

	max_gibs		= gi.cvar  ("max_gibs",   "20", 0);//faf
	extra_anims		= gi.cvar  ("extra_anims", "1", 0);//faf
	force_auto_select	= gi.cvar  ("force_auto_select", "0", 0);//faf

	allied_password = gi.cvar ("allied_password", "", 0);
	axis_password = gi.cvar ("axis_password", "", 0);

	objective_protect = gi.cvar ("objective_protect", "1", 0);
	ent_files = gi.cvar ("ent_files", "1", 0);

	mauser_only = gi.cvar ("mauser_only", "0", 0);
	swords = gi.cvar ("swords", "0", 0);
	sniper_only = gi.cvar ("sniper_only", "0", 0);
	no_nades = gi.cvar ("no_nades", "0", 0);
	airstrikes = gi.cvar ("airstrikes", "1", 0);
	mapvoting = gi.cvar ("mapvoting", "0", 0);
	constant_play = gi.cvar ("constant_play", "0", 0);

	ctc = gi.cvar ("ctc", "0", 0);

	mashup = gi.cvar ("mashup", "0", 0);

	fast_knife = gi.cvar ("fast_knife", "0", 0);

	chile = gi.cvar ("chile", "0", CVAR_LATCH);

	knifefest = gi.cvar ("knifefest", "0", 0);

	fullbright = gi.cvar ("fullbright", "0", 0);

	stats = gi.cvar ("stats", "0", CVAR_LATCH);

	// pbowens: reduce lag by manipulating userinfo on server basis
//	crosshair	= gi.cvar ("crosshair", "0", CVAR_USERINFO);

	bots = gi.cvar ("bots", "0", CVAR_SERVERINFO);// | CVAR_LATCH);
	botchat = gi.cvar ("botchat", "1", 0);
	botchatfreq = gi.cvar ("botchatfreq", "10", 0);

//	alliedbots = gi.cvar ("alliedbots", "0", CVAR_LATCH);
//	axisbots = gi.cvar ("axisbots", "0", CVAR_LATCH);
	alliedlevel = gi.cvar ("alliedlevel", "5", 0);//CVAR_LATCH);
	axislevel = gi.cvar ("axislevel", "5", 0);//CVAR_LATCH);
	//playerminforbots = gi.cvar ("playerminforbots", "100", 0);
	playermaxforbots = gi.cvar ("playermaxforbots", "6", 0);
	campaign = gi.cvar ("campaign", "", CVAR_SERVERINFO | CVAR_LATCH);
	nohud = gi.cvar ("nohud", "0", 0);
	serverimg = gi.cvar ("serverimg", "", CVAR_SERVERINFO | CVAR_LATCH);

	sandbaglimit = gi.cvar ("sandbaglimit", "5", CVAR_LATCH);//CVAR_LATCH);

	afk_time = gi.cvar("afk_time", "120", 0);


	// items
	InitItems ();

	/*
	Com_sprintf (game.helpmessage1, sizeof(game.helpmessage1), "");
	Com_sprintf (game.helpmessage2, sizeof(game.helpmessage2), "");
	*/

	// initialize all entities for this game
	game.maxentities = maxentities->value;
	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// initialize all clients for this game
	game.maxclients = maxclients->value;
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients+1;


	campaign_winner = -1;
	alliedplatoons = -1;
	axisplatoons = -1;

	AI_Init();//JABot

    mapvotes[0]=mapvotes[1]=mapvotes[2]=mapvotes[3]=mapvotes[4]=0;

}

//=========================================================

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
	Q_strncpyz (str, sizeof(str), __DATE__);
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

	qbots = false;


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

