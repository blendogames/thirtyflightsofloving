/*****************************************************************

	Eraser Bot source code - by Ryan Feltrin

	..............................................................

	This file is Copyright(c) 1998, Ryan Feltrin, All Rights Reserved.

	..............................................................

	All other files are Copyright(c) Id Software, Inc.

	Please see liscense.txt in the source directory for the copyright
	information regarding those files belonging to Id Software, Inc.

	..............................................................

	Should you decide to release a modified version of Eraser, you MUST
	include the following text (minus the BEGIN and END lines) in the
	documentation for your modification.

	--- BEGIN ---

	The Eraser Bot is a product of Ryan Feltrin, and is available from
	the Eraser Bot homepage, at http://impact.frag.com.

	This program is a modification of the Eraser Bot, and is therefore
	in NO WAY supported by Ryan Feltrin.

	This program MUST NOT be sold in ANY form. If you have paid for
	this product, you should contact Ryan Feltrin immediately, via
	the Eraser Bot homepage.

	--- END ---

	..............................................................

	You will find p_trail.c has not been included with the Eraser
	source code release. This is NOT an error. I am unable to
	distribute this file because it contains code that is bound by
	legal documents, and signed by myself, never to be released
	to the public. Sorry guys, but law is law.

	I have therefore include the compiled version of these files
	in .obj form in the src\Release and src\Debug directories.
	So while you cannot edit and debug code within these files,
	you can still compile this source as-is. Although these will only
	work in MSVC v5.0, linux versions can be made available upon
	request.

	NOTE: When compiling this source, you will get a warning
	message from the compiler, regarding the missing p_trail.c
	file. Just ignore it, it will still compile fine.

	..............................................................

	I, Ryan Feltrin, hold no responsibility for any harm caused by the
	use of this source code. I also am NOT willing to provide any form
	of help or support for this source code. It is provided as-is,
	as a service by me, with no documentation, other then the comments
	contained within the code. If you have any queries, I suggest you
	visit the "official" Eraser source web-board, at
	http://www.telefragged.com/epidemic/. I will stop by there from
	time to time, to answer questions and help with any problems that
	may arise.

	Otherwise, have fun, and I look forward to seeing what can be done
	with this.

	-Ryan Feltrin

 *****************************************************************/

#include "g_local.h"
#include "m_player.h"
#include "bot_procs.h"
// Q2 Camera Begin
#include "camclient.h"
// Q2 Camera End
#include "aj_startmax.h"	// AJ
#include "aj_statusbars.h"

qboolean ClientConnect (edict_t *ent, char *userinfo, qboolean loadgame);

// BEGIN: SABIN code
edict_t *bot_GetLastFreeClient (void)
{
	edict_t	*bot = NULL;
	int		i;

	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if (!bot->inuse)
			break;
	}

	if (bot->inuse)
		bot = NULL;

	return bot;
}
// END: SABIN code

void respawn_bot (edict_t *self)
{
// AJ - small optimisation
	gclient_t	*client = self->client;
// end AJ
	if (level.intermissiontime)
		return;

	self->s.event = EV_PLAYER_TELEPORT;

	PutClientInServer(self);

	self->last_goal = NULL;
	self->enemy = self->goalentity = self->movetarget = NULL;
	self->viewheight = 22;

	// reset weapon to blaster
	self->last_fire = level.time + 0.2;
//	self->fire_interval = FIRE_INTERVAL_BLASTER;
//	self->bot_fire = botBlaster;

// AJ force to best weapon

	if ( client->pers.inventory[ITEM_INDEX(item_bfg10k)]
		&& client->pers.inventory[ITEM_INDEX(item_cells)]
		&& ((sk_start_weapon->value == 0) || (sk_start_weapon->value == 10)) )
	{
		client->newweapon = item_bfg10k;
		self->bot_fire = botBFG;
		self->fire_interval = FIRE_INTERVAL_BFG;
	}
	else if ( client->pers.inventory[ITEM_INDEX(item_railgun)]
		&& client->pers.inventory[ITEM_INDEX(item_slugs)]
		&& ((sk_start_weapon->value == 0) || (sk_start_weapon->value == 9)) )
	{
		client->newweapon = item_railgun;
		self->bot_fire = botRailgun;
		self->fire_interval = FIRE_INTERVAL_RAILGUN;
	}
	else if ( client->pers.inventory[ITEM_INDEX(item_hyperblaster)]
		&&  client->pers.inventory[ITEM_INDEX(item_cells)]
		&& ((sk_start_weapon->value == 0) || (sk_start_weapon->value == 8)) )
	{
		client->newweapon = item_hyperblaster;
		self->bot_fire = botHyperblaster;
		self->fire_interval = FIRE_INTERVAL_HYPERBLASTER;
	}
	else if ( client->pers.inventory[ITEM_INDEX(item_rocketlauncher)]
		&& client->pers.inventory[ITEM_INDEX(item_rockets)]
		&& ((sk_start_weapon->value == 0) || (sk_start_weapon->value == 7)) )
	{
		client->newweapon = item_rocketlauncher;
		self->bot_fire = botRocketLauncher;
		self->fire_interval = FIRE_INTERVAL_ROCKETLAUNCHER;
	}
	else if ( client->pers.inventory[ITEM_INDEX(item_grenadelauncher)]
		&& client->pers.inventory[ITEM_INDEX(item_grenades)]
		&& ((sk_start_weapon->value == 0) || (sk_start_weapon->value == 6)) )
	{
		client->newweapon = item_grenadelauncher;
		self->bot_fire = botGrenadeLauncher;
		self->fire_interval = FIRE_INTERVAL_GRENADELAUNCHER;
	}
	else if ( client->pers.inventory[ITEM_INDEX(item_chaingun)]
		&& client->pers.inventory[ITEM_INDEX(item_bullets)]
		&& ((sk_start_weapon->value == 0) || (sk_start_weapon->value == 5)) )
	{
		client->newweapon = item_chaingun;
		self->bot_fire = botChaingun;
		self->fire_interval = FIRE_INTERVAL_CHAINGUN;
	}
	else if ( client->pers.inventory[ITEM_INDEX(item_machinegun)]
		&&  client->pers.inventory[ITEM_INDEX(item_bullets)]
		&& ((sk_start_weapon->value == 0) || (sk_start_weapon->value == 4)) )
	{
		client->newweapon = item_machinegun;
		self->bot_fire = botMachineGun;
		self->fire_interval = FIRE_INTERVAL_MACHINEGUN;
	}
	else if ( client->pers.inventory[ITEM_INDEX(item_supershotgun)]
		&&  client->pers.inventory[ITEM_INDEX(item_shells)]
		&& ((sk_start_weapon->value == 0) || (sk_start_weapon->value == 3)) )
	{
		client->newweapon = item_supershotgun;
		self->bot_fire = botSuperShotgun;
		self->fire_interval = FIRE_INTERVAL_SSHOTGUN;
	}
	else if ( client->pers.inventory[ITEM_INDEX(item_shotgun)]
		&&  client->pers.inventory[ITEM_INDEX(item_shells)]
		&& ((sk_start_weapon->value == 0) || (sk_start_weapon->value == 2)) )
	{
		client->newweapon = item_shotgun;
		self->bot_fire = botShotgun;
		self->fire_interval = FIRE_INTERVAL_SHOTGUN;
	}
	else if ( client->pers.inventory[ITEM_INDEX(item_blaster)]
		&& ((sk_start_weapon->value == 0) || (sk_start_weapon->value == 1)) )
	{
		client->newweapon = item_blaster;
		self->bot_fire = botBlaster;
		self->fire_interval = FIRE_INTERVAL_BLASTER;
	}


	client->pers.weapon = client->newweapon;
	client->pers.lastweapon = client->newweapon;
//	botPickBestWeapon(self);
// end AJ

	self->bored_suicide_time = -1;
	self->checkstuck_time = level.time;
	self->last_reached_trail = level.time + 1;

	self->client->killer_yaw = 0;	// chaingun wind-up
	self->avoid_ent = NULL;
	self->flagpath_goal = NULL;
	self->last_move_nocloser = level.time;

// AJ - switch on safety mode
	if (use_safety->value)
	{
		self->takedamage = DAMAGE_NO;
		self->client->safety_mode = TRUE;
		self->client->safety_time = level.time + safety_time->value;
		self->client->ps.stats[STAT_LITHIUM_MODE] = CS_SAFETY;
	}
// end AJ

	// go for it
	walkmonster_start(self);
}

// Find an available edict, and initialize some default values
edict_t *G_SpawnBot ()
{
	edict_t		*bot;

	if (!deathmatch->value)
	{
		return NULL;
	}

	if ((!bot_calc_nodes->value) && !loaded_trail_flag)
	{
		my_bprintf(PRINT_HIGH, "Route-table not found!\n");
		return NULL;
	}

	last_bot_spawn = level.time;

//	bot = G_Spawn();
	bot = bot_GetLastFreeClient();

	if (!bot)
	{
		gi.dprintf("No client spots available!\n");
		return NULL;
	}

//	bot->bot_client = gi.TagMalloc (sizeof(struct gclient_s), TAG_GAME);
	bot->bot_stats =  gi.TagMalloc (sizeof(bot_stats_t), TAG_GAME);

	bot->classname = "player";

	bot->movetype = MOVETYPE_WALK;
	bot->solid = SOLID_BBOX;

	VectorSet (bot->mins, -16, -16, -24);
	VectorSet (bot->maxs, 16, 16, 32);

	bot->health = bot->max_health = 100;
	bot->mass = 200;
	bot->gravity = 1;

	bot->last_goal = NULL;

	bot->pain = bot_pain;
	bot->die = bot_die;

	bot->monsterinfo.stand = bot_run; //bot_stand;
	bot->monsterinfo.walk = bot_run; //bot_walk;
	bot->monsterinfo.run = bot_run;
	bot->monsterinfo.attack = bot_run;
	bot->monsterinfo.melee = NULL;
	bot->monsterinfo.sight = NULL;

	bot->monsterinfo.scale = MODEL_SCALE;

	bot->enemy = bot->goalentity = bot->movetarget = NULL;

	players[num_players++] = bot;

	bot->last_fire = level.time + 0.2;
	bot->fire_interval = FIRE_INTERVAL_BLASTER;
	bot->bot_fire = botBlaster;

	bot->bored_suicide_time = -1;
	bot->checkstuck_time = level.time;

	bot->viewheight = 22;
	bot->yaw_speed = 50;	// turn at yaw_speed degrees per FRAME

	bot->last_reached_trail = level.time + 1;

	bot->avoid_ent = NULL;
	bot->last_move_nocloser = level.time;

	return bot;
}

// Perform the ClientBegin() and ClientBeginDeathmatch() functions in one routine, that is bot-specific
edict_t *spawn_bot (char *botname)
{
	edict_t		*bot, *chat;
	bot_info_t	*botdata=NULL;
	char		skin[256];
	char		userinfo[MAX_INFO_STRING];
	vec3_t		spawn_origin, spawn_angles;

	if (!(botdata = GetBotData(botname)))
	{
		gi.dprintf("Unable to find bot, or no bots left\n");
		return NULL;
	}

	bot = G_SpawnBot();

	if (!bot)
	{
		gi.dprintf("Unable to spawn bot: cannot create entity\n");
		return NULL;
	}

	bot->bot_client = true;
	bot->client = &game.clients[bot-g_edicts-1];

	memset(bot->client, 0, sizeof(*(bot->client)));

	// copy the stats across from the bot config
	botdata->ingame_count++;
	bot->botdata = botdata;

//	strcpy(skin, botdata->skin);
	Com_strcpy (skin, sizeof(skin), botdata->skin);

// BEGIN: SABIN code
	// initialise userinfo
	memset (userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey (userinfo, "name", botdata->name);
	Info_SetValueForKey (userinfo, "skin", skin);
	Info_SetValueForKey (userinfo, "hand", "2");	// bot is center handed for now
	Info_SetValueForKey (userinfo, "color1", botdata->color1);	// Knightmare- added custom client color
// END: SABIN code

    ///Q2 Camera Begin
    EntityListAdd(bot);
    ///Q2 Camera End

	ClientConnect (bot, userinfo, false);

	if (ctf->value)
	{
		my_bprintf(PRINT_HIGH, "%s joined the %s team.\n",
			bot->client->pers.netname, CTFTeamName(bot->client->resp.ctf_team));
	}

	SelectSpawnPoint (bot, spawn_origin, spawn_angles);

	VectorCopy(spawn_origin, bot->s.origin);
//	bot->s.origin[2] += 1;	// make sure off ground

	VectorCopy(spawn_angles, bot->s.angles);

	bot->client->killer_yaw = 0;	// chaingun wind-up

	// clear playerstate values
	memset (&bot->client->ps, 0, sizeof(bot->client->ps));

	bot->client->ps.pmove.origin[0] = bot->s.origin[0]*8;
	bot->client->ps.pmove.origin[1] = bot->s.origin[1]*8;
	bot->client->ps.pmove.origin[2] = bot->s.origin[2]*8;

	bot->client->ps.fov = 90;

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (bot-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (bot->s.origin, MULTICAST_PVS);

	// copy the bot stats
	memcpy(bot->bot_stats, &(botdata->bot_stats), sizeof(bot_stats_t));

	// set starting skill level
	bot->skill_level = skill->value;
	AdjustRatingsToSkill(bot);

//	bot->s.modelindex = gi.modelindex(model);
	bot->s.modelindex = MAX_MODELS-1; //was 255
	bot->s.modelindex2 = MAX_MODELS-1; //was 255
	bot->s.skinnum = bot-g_edicts - 1;

/*
	bot->client->buttons = bot->s.modelindex;
	bot->client->oldbuttons = bot->s.skinnum;
*/
	bot->map = G_CopyString(botdata->name);
//	strcpy(bot->client->pers.netname, botdata->name);
	Com_strcpy(bot->client->pers.netname, sizeof(bot->client->pers.netname), botdata->name);
	my_bprintf(PRINT_HIGH, "%s entered the game", bot->client->pers.netname);

// AJ change weapon if started with other than blaster..
// AJ force to best weapon
	//start with BFG?  I sure as hell hope not!!!!
	if ( bot->client->pers.inventory[ITEM_INDEX(item_bfg10k)]
		&& bot->client->pers.inventory[ITEM_INDEX(item_cells)]
		&& (sk_start_weapon->value==0 || sk_start_weapon->value==10))
	{
		bot->client->newweapon = item_bfg10k;
		bot->bot_fire = botBFG;
		bot->fire_interval = FIRE_INTERVAL_BFG;
	}
	else if (bot->client->pers.inventory[ITEM_INDEX(item_railgun)]
		&& bot->client->pers.inventory[ITEM_INDEX(item_slugs)]
		&& (sk_start_weapon->value==0 || sk_start_weapon->value==9))
	{
		bot->client->newweapon = item_railgun;
		bot->bot_fire = botRailgun;
		bot->fire_interval = FIRE_INTERVAL_RAILGUN;
	}
	else if ( bot->client->pers.inventory[ITEM_INDEX(item_hyperblaster)]
		&&  bot->client->pers.inventory[ITEM_INDEX(item_cells)]
		&& (sk_start_weapon->value==0 || sk_start_weapon->value==8))
	{
		bot->client->newweapon = item_hyperblaster;
		bot->bot_fire = botHyperblaster;
		bot->fire_interval = FIRE_INTERVAL_HYPERBLASTER;
	}
	else if ( bot->client->pers.inventory[ITEM_INDEX(item_rocketlauncher)]
		&& bot->client->pers.inventory[ITEM_INDEX(item_rockets)]
		&& (sk_start_weapon->value==0 || sk_start_weapon->value==7))
	{
		bot->client->newweapon = item_rocketlauncher;
		bot->bot_fire = botRocketLauncher;
		bot->fire_interval = FIRE_INTERVAL_ROCKETLAUNCHER;
	}
	else if ( bot->client->pers.inventory[ITEM_INDEX(item_grenadelauncher)]
		&& bot->client->pers.inventory[ITEM_INDEX(item_grenades)]
		&& (sk_start_weapon->value==0 || sk_start_weapon->value==6))
	{
		bot->client->newweapon = item_grenadelauncher;
		bot->bot_fire = botGrenadeLauncher;
		bot->fire_interval = FIRE_INTERVAL_GRENADELAUNCHER;
	}
	else if (bot->client->pers.inventory[ITEM_INDEX(item_chaingun)]
		&& bot->client->pers.inventory[ITEM_INDEX(item_bullets)]
		&& (sk_start_weapon->value==0 || sk_start_weapon->value==5))
	{
		bot->client->newweapon = item_chaingun;
		bot->bot_fire = botChaingun;
		bot->fire_interval = FIRE_INTERVAL_CHAINGUN;
	}
	else if ( bot->client->pers.inventory[ITEM_INDEX(item_machinegun)]
		&&  bot->client->pers.inventory[ITEM_INDEX(item_bullets)]
		&& (sk_start_weapon->value==0 || sk_start_weapon->value==4))
	{
		bot->client->newweapon = item_machinegun;
		bot->bot_fire = botMachineGun;
		bot->fire_interval = FIRE_INTERVAL_MACHINEGUN;
	}
	else if ( bot->client->pers.inventory[ITEM_INDEX(item_supershotgun)]
		&&  bot->client->pers.inventory[ITEM_INDEX(item_shells)]
		&& (sk_start_weapon->value==0 || sk_start_weapon->value==3))
	{
		bot->client->newweapon = item_supershotgun;
		bot->bot_fire = botSuperShotgun;
		bot->fire_interval = FIRE_INTERVAL_SSHOTGUN;
	}
	else if ( bot->client->pers.inventory[ITEM_INDEX(item_shotgun)]
		&&  bot->client->pers.inventory[ITEM_INDEX(item_shells)]
		&& (sk_start_weapon->value==0 || sk_start_weapon->value==2))
	{
		bot->client->newweapon = item_shotgun;
		bot->bot_fire = botShotgun;
		bot->fire_interval = FIRE_INTERVAL_SHOTGUN;
	}
	else if ( bot->client->pers.inventory[ITEM_INDEX(item_blaster)]
		&& (sk_start_weapon->value==0 || sk_start_weapon->value==1))
	{
		bot->client->newweapon = item_blaster;
		bot->bot_fire = botBlaster;
		bot->fire_interval = FIRE_INTERVAL_BLASTER;
	}


	bot->client->pers.weapon = bot->client->newweapon;
	bot->client->pers.lastweapon = bot->client->newweapon;
//	botPickBestWeapon(self);
// end AJ

	ShowGun(bot);
/*
	if (view_weapons->value && (bot->s.modelindex2 == (MAX_MODELS-1)) && bot_show_connect_info->value) //was 255
	{
		my_bprintf(PRINT_HIGH, " (no view weapon)");
	}
*/
	my_bprintf(PRINT_HIGH, "\n");

	bot_count++;

	// generic bot stuff
	if (!KillBox (bot))
	{	// could't spawn in?
	}

	gi.linkentity (bot);

	bot->viewheight = 22;

	bot->inuse = true;

// AJ - switch on safety mode
	if (use_safety->value)
	{
		bot->takedamage = DAMAGE_NO;
		bot->client->safety_mode = TRUE;
		bot->client->safety_time = level.time + safety_time->value;
		bot->client->ps.stats[STAT_LITHIUM_MODE] = CS_SAFETY;
	}
// end AJ

	// go for it
	walkmonster_start(bot);

	if (random() < 0.3)
	{
		// spawn the greetings thinker
		chat = G_Spawn();
		chat->owner = bot;
		chat->think = BotGreeting;
		chat->nextthink = level.time + 1.5 + random();
	}

	return bot;
}

extern int num_clients;

void botRemovePlayer(edict_t *self)
{
	int i;

	self->health = 0;	// so other bots stop looking for us

	// remove the client from the players array
	for (i=0; i<num_players; i++)
		if (players[i] == self)
			break;

	if (i == num_players)	// didn't find them
	{
		gi.dprintf("WARNING: Unable to remove player from player[] array, problems will arise.\n");
		return;
	}

	i++;
	for (; i<num_players; i++)
		players[i-1] = players[i];

	players[i] = NULL;


	// remove from team list
	if (self->client->team)
	{
		self->client->team->num_players--;
		self->client->team->num_bots--;
		self->client->team = NULL;
	}

	self->client->resp.ctf_team = CTF_NOTEAM;

	num_players--;

	if (self->bot_client)
	{
		bot_count--;
		self->botdata->ingame_count--;
	}
	else
	{
		num_clients--;
	}

	// tell all other bots not to look for this bot anymore
	for (i=0; i<num_players; i++)
		if (players[i]->enemy == self)
			players[i]->enemy = players[i]->goalentity = NULL;
}

void	botDisconnect(edict_t	*self)
{	// disconnects a bot from the game

	ClientDisconnect(self);

	self->bot_client = false;
	self->bot_stats = NULL;
}
