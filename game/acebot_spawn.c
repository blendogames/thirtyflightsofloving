/*
===========================================================================
Copyright (C) 1998 Steve Yeager

This file is part of ACE Bot source code.

ACE Bot source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

ACE Bot source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ACE Bot source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

///////////////////////////////////////////////////////////////////////
//
//  ACE - Quake II Bot Base Code
//
//  Version 1.0
//
//  This file is Copyright(c), Steve Yeager 1998, All Rights Reserved
//
//
//	All other files are Copyright(c) Id Software, Inc.
//
//	Please see liscense.txt in the source directory for the copyright
//	information regarding those files belonging to Id Software, Inc.
//	
//	Should you decide to release a modified version of ACE, you MUST
//	include the following text (minus the BEGIN and END lines) in the 
//	documentation for your modification.
//
//	--- BEGIN ---
//
//	The ACE Bot is a product of Steve Yeager, and is available from
//	the ACE Bot homepage, at http://www.axionfx.com/ace.
//
//	This program is a modification of the ACE Bot, and is therefore
//	in NO WAY supported by Steve Yeager.
//
//	--- END ---
//
//	I, Steve Yeager, hold no responsibility for any harm caused by the
//	use of this source code, especially to small children and animals.
//  It is provided as-is with no implied warranty or support.
//
//  I also wish to thank and acknowledge the great work of others
//  that has helped me to develop this code.
//
//  John Cricket    - For ideas and swapping code.
//  Ryan Feltrin    - For ideas and swapping code.
//  SABIN           - For showing how to do true client based movement.
//  BotEpidemic     - For keeping us up to date.
//  Telefragged.com - For giving ACE a home.
//  Microsoft       - For giving us such a wonderful crash free OS.
//  id              - Need I say more.
//  
//  And to all the other testers, pathers, and players and people
//  who I can't remember who the heck they were, but helped out.
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//
//  acebot_spawn.c - This file contains all of the 
//                   spawing support routines for the ACE bot.
//
///////////////////////////////////////////////////////////////////////

#include "g_local.h"
#include "m_player.h"
#include "acebot.h"

///////////////////////////////////////////////////////////////////////
// Had to add this function in this version for some reason.
// any globals are wiped out between level changes....so
// save the bots out to a file. 
//
// NOTE: There is still a bug when the bots are saved for
//       a dm game and then reloaded into a CTF game.
///////////////////////////////////////////////////////////////////////
/*void ACESP_SaveBots()
{
    edict_t *bot;
    FILE *pOut;
	int i,count = 0;
	    			
	if((pOut = fopen("ace\\bots.tmp", "wb" )) == NULL)
		return; // bail
	
	// Get number of bots
	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if (bot->inuse && bot->is_bot)
			count++;
	}
	
	fwrite(&count,sizeof (int),1,pOut); // Write number of bots

	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if (bot->inuse && bot->is_bot)
			fwrite(bot->client->pers.userinfo,sizeof (char) * MAX_INFO_STRING,1,pOut); 
	}
		
    fclose(pOut);
}*/

///////////////////////////////////////////////////////////////////////
// Had to add this function in this version for some reason.
// any globals are wiped out between level changes....so
// load the bots from a file.
//
// Side effect/benefit is that the bots persist between games.
///////////////////////////////////////////////////////////////////////
/*void ACESP_LoadBots()
{
    FILE *pIn;
	char userinfo[MAX_INFO_STRING];
	int i, count;

	if((pIn = fopen("ace\\bots.tmp", "rb" )) == NULL)
		return; // bail
	
	fread(&count,sizeof (int),1,pIn); 

	for(i=0;i<count;i++)
	{
		fread(userinfo,sizeof(char) * MAX_INFO_STRING,1,pIn); 
		ACESP_SpawnBot (NULL, NULL, NULL, userinfo);
	}
		
    fclose(pIn);
}*/

///////////////////////////////////////////////////////////////////////
// Called by PutClient in Server to actually release the bot into the game
// Keep from killin' each other when all spawned at once
///////////////////////////////////////////////////////////////////////
void ACESP_HoldSpawn(edict_t *self)
{
	if (!KillBox (self))
	{	// could't spawn in?
	}

	gi.linkentity (self);

	self->think = ACEAI_Think;
	self->nextthink = level.time + FRAMETIME;

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if(ctf->value)
	safe_bprintf(PRINT_MEDIUM, "%s joined the %s team.\n",
		self->client->pers.netname, CTFTeamName(self->client->resp.ctf_team));
	else
		safe_bprintf (PRINT_MEDIUM, "%s entered the game\n", self->client->pers.netname);

}

///////////////////////////////////////////////////////////////////////
// Modified version of id's code
///////////////////////////////////////////////////////////////////////
void ACESP_PutClientInServer (edict_t *bot, qboolean respawn, int team)
{
	vec3_t		mins = {-16, -16, -24};
	vec3_t		maxs = {16, 16, 32};
	int			index;
	vec3_t		spawn_origin, spawn_angles;
	gclient_t	*client;
	int			i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	char		*s;
	int			spawn_style;
	int			spawn_health;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (bot, spawn_origin, spawn_angles, &spawn_style, &spawn_health);
	
	index = bot-g_edicts-1;
	client = bot->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char userinfo[MAX_INFO_STRING];

		resp = bot->client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client, spawn_style);
		ClientUserinfoChanged (bot, userinfo);
	}
	else
		memset (&resp, 0, sizeof(resp));
	
	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	client->resp = resp;
	
	// copy some data from the client to the entity
	FetchClientEntData (bot);
	
	// clear entity values
	bot->groundentity = NULL;
	bot->client = &game.clients[index];
	bot->takedamage = DAMAGE_AIM;
	bot->movetype = MOVETYPE_WALK;
	bot->viewheight = 24;
	bot->classname = "bot";
	bot->mass = 200;
	bot->solid = SOLID_BBOX;
	bot->deadflag = DEAD_NO;
	bot->air_finished = level.time + 12;
	bot->clipmask = MASK_PLAYERSOLID;
	bot->model = "players/male/tris.md2";
	bot->pain = player_pain;
	bot->die = player_die;
	bot->waterlevel = 0;
	bot->watertype = 0;
	bot->flags &= ~FL_NO_KNOCKBACK;
	bot->svflags &= ~SVF_DEADMONSTER;
	bot->is_jumping = false;
	
	if (ctf->value)
	{
		client->resp.ctf_team = team;
		client->resp.ctf_state = CTF_STATE_START;
		s = Info_ValueForKey (client->pers.userinfo, "skin");
		CTFAssignSkin(bot, s);
	}

	VectorCopy (mins, bot->mins);
	VectorCopy (maxs, bot->maxs);
	VectorClear (bot->velocity);

	// clear playerstate values
	memset (&bot->client->ps, 0, sizeof(client->ps));
	
	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID

	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

	// Knightmare- fix for null model
	if (client->pers.weapon && client->pers.weapon->view_model)
		client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	bot->s.effects = 0;
	bot->s.skinnum = bot - g_edicts - 1;
	bot->s.modelindex = MAX_MODELS-1;		// will use the skin specified model
	bot->s.modelindex2 = MAX_MODELS-1;		// custom gun model
	bot->s.frame = 0;
	VectorCopy (spawn_origin, bot->s.origin);
	bot->s.origin[2] += 1;	// make sure off ground
	VectorCopy (bot->s.origin, bot->s.old_origin);	// Knightmare- was missing oldorigin!

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);
	}

	bot->s.angles[PITCH] = 0;
	bot->s.angles[YAW] = spawn_angles[YAW];
	bot->s.angles[ROLL] = 0;
	VectorCopy (bot->s.angles, client->ps.viewangles);
	VectorCopy (bot->s.angles, client->v_angle);
	
	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (bot);

	bot->enemy = NULL;
	bot->movetarget = NULL; 
	bot->state = STATE_MOVE;

	// Set the current node
	bot->current_node = ACEND_FindClosestReachableNode(bot,NODE_DENSITY, NODE_ALL);
	bot->goal_node = bot->current_node;
	bot->next_node = bot->current_node;
	bot->next_move_time = level.time;		
	bot->suicide_timeout = level.time + 15.0;

	// If we are not respawning hold off for up to three seconds before releasing into game
    if (!respawn)
	{
		bot->think = ACESP_HoldSpawn;
		bot->nextthink = level.time + 0.1;
		bot->nextthink = level.time + random()*3.0; // up to three seconds
	}
	else
	{
		if (!KillBox (bot))
		{	// could't spawn in?
		}

		gi.linkentity (bot);

		bot->think = ACEAI_Think;
		bot->nextthink = level.time + FRAMETIME;

		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (bot-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (bot->s.origin, MULTICAST_PVS);
	}
	
}

///////////////////////////////////////////////////////////////////////
// Respawn the bot
///////////////////////////////////////////////////////////////////////
void ACESP_Respawn (edict_t *self)
{
	CopyToBodyQue (self);

	if (ctf->value)
		ACESP_PutClientInServer (self ,true, self->client->resp.ctf_team);
	else
		ACESP_PutClientInServer (self, true, 0);

	// add a teleportation effect
	self->s.event = EV_PLAYER_TELEPORT;

	// hold in place briefly
	self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	self->client->ps.pmove.pm_time = 14;

	self->client->respawn_time = level.time;
}

///////////////////////////////////////////////////////////////////////
// Find a free client spot
///////////////////////////////////////////////////////////////////////
edict_t *ACESP_FindFreeClient (void)
{
	edict_t *bot;
	int	i;
	int max_count=0;
	
	// This is for the naming of the bots
	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;
		
		if(bot->count > max_count)
			max_count = bot->count;
	}

	// Check for free spot
	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if (!bot->inuse)
			break;
	}

	bot->count = max_count + 1; // Will become bot name...

	if (bot->inuse)
		bot = NULL;
	
	return bot;
}

// Knightmare- added bot name/skin table
///////////////////////////////////////////////////////////////////////
// Bot name/skin table
///////////////////////////////////////////////////////////////////////
struct botinfo_s
{
	char	name[128];
	char	skin[128];
	int		ingame_count;
};
typedef struct botinfo_s botinfo_t;
botinfo_t bot_info[MAX_BOTS];
int		num_botinfo = 0;

///////////////////////////////////////////////////////////////////////
// Load a table of bots from bots.cfg.
///////////////////////////////////////////////////////////////////////
void ACESP_LoadBotInfo()
{
    FILE	*pIn;
	char	filename[MAX_OSPATH] = "";
	char	line[256], *parseline;
	char	*token;
	char	botname[128];

	if (num_botinfo > 0) // already loaded
		return;
	// Knightmare- rewote this
	GameDirRelativePath ("bots.cfg", filename, sizeof(filename));
	if((pIn = fopen(filename, "rb" )) == NULL)
	{
		safe_bprintf(PRINT_MEDIUM,"ACE: No bots.cfg file found, using default bots.\n");
		return; // bail
	}
	safe_bprintf(PRINT_MEDIUM,"ACE: Loading bot data...");

	while (fgets(line, sizeof(line), pIn) && num_botinfo < MAX_BOTS)
	{
		parseline = line;
		token = COM_Parse (&parseline);
		if (!token || !strlen(token)) // catch bad line
			continue;
	//	strncpy (botname, token, sizeof(botname)-1);
		Q_strncpyz (botname, token, sizeof(botname));
		token = COM_Parse (&parseline);
		if (!token || !strlen(token)) // catch bad line
			continue;
	//	strncpy (bot_info[num_botinfo].name, botname, sizeof(bot_info[num_botinfo].name)-1);
	//	strncpy (bot_info[num_botinfo].skin, token, sizeof(bot_info[num_botinfo].skin)-1);
		Q_strncpyz (bot_info[num_botinfo].name, botname, sizeof(bot_info[num_botinfo].name));
		Q_strncpyz (bot_info[num_botinfo].skin, token, sizeof(bot_info[num_botinfo].skin));
		//gi.dprintf("%s %s\n", bot_info[num_botinfo].name, bot_info[num_botinfo].skin);
		bot_info[num_botinfo].ingame_count = 0;
		num_botinfo++;
	}
	//gi.dprintf("Number of bots loaded: %d\n\n", num_botinfo);
	safe_bprintf(PRINT_MEDIUM, "done.\n");
	fclose(pIn);
}

///////////////////////////////////////////////////////////////////////
// Hardcoded bot skin list
///////////////////////////////////////////////////////////////////////
char *skinnames[] = {
	"female/athena",
	"female/brianna",
	"female/cobalt",
	"female/ensign",
	"female/jezebel",
	"female/jungle",
	"female/lotus",
	"female/stiletto",
	"female/venus",
	"female/voodoo",
	"male/cipher",
	"male/claymore",
	"male/flak",
	"male/grunt",
	"male/howitzer",
	"male/major",
	"male/nightops",
	"male/pointman",
	"male/psycho",
	"male/rampage",
	"male/razor",
	"male/recon",
	"male/scout",
	"male/sniper",
	"male/viper",
	"cyborg/oni911",
	"cyborg/ps9000",
	"cyborg/tyr574",
	0
};

int listSize (char* list[])
{
	int i=0;
	while (list[i]) i++;
	return i;	
}
static int NUM_BOT_SKINS = 0;


///////////////////////////////////////////////////////////////////////
// Set the name of the bot and update the userinfo
///////////////////////////////////////////////////////////////////////
void ACESP_SetName(edict_t *bot, char *name, char *skin, char *team)
{
	float rnd;
	char userinfo[MAX_INFO_STRING] = "";
	char bot_skin[MAX_INFO_STRING] = "";
	char bot_name[MAX_INFO_STRING] = "";
	int i, r;

	if (NUM_BOT_SKINS == 0)
		NUM_BOT_SKINS = listSize(skinnames);

	// Set the name for the bot.
	// name
	if(strlen(name) == 0)
	{
		// Randomly select from the name/skin table
		if (num_botinfo > 0)
		{
			//int numtries = 0;
			rnd = random();
			for (i = 0; i < num_botinfo; i++)
				if (rnd < ((float)(i+1)/(float)num_botinfo) )
				{	r=i; break; }
			// Search for an unused name starting at selected pos
			for (i = r; i < num_botinfo; i++)
				if (!bot_info[i].ingame_count)
				{
					Com_sprintf(bot_name, sizeof(bot_name), bot_info[i].name);
					bot_info[i].ingame_count++;
					break;
				}
			// If none found, loop back
			if (strlen(bot_name) == 0)
				for (i = 0; i < r; i++)
					if (!bot_info[i].ingame_count)
					{
						Com_sprintf(bot_name, sizeof(bot_name), bot_info[i].name);
						bot_info[i].ingame_count++;
						break;
					}
			// If no more free bots in table, use a numbered name
			if (strlen(bot_name) == 0)
				Com_sprintf(bot_name, sizeof(bot_name), "ACEBot_%d",bot->count);
		}
		else
			Com_sprintf(bot_name, sizeof(bot_name), "ACEBot_%d",bot->count);
	}
	else
	//	strncpy(bot_name, name);
		Q_strncpyz(bot_name, name, sizeof(bot_name));

	// skin
	if (strlen(skin) == 0)
	{	// check if this bot is in the table
		for (i = 0; i < num_botinfo; i++)
			if (!Q_stricmp(bot_name, bot_info[i].name))
			{
				Com_sprintf(bot_name, sizeof(bot_name), bot_info[i].name); // fix capitalization
				Com_sprintf(bot_skin, sizeof(bot_skin), bot_info[i].skin);
				bot_info[i].ingame_count++;
				break;
			}
			if (strlen(bot_skin) == 0)
			{	// randomly choose skin 
				rnd = random();
				for (i = 0; i < NUM_BOT_SKINS; i++)
					if (rnd < ((float)(i+1)/(float)NUM_BOT_SKINS) )
					{	r=i; break; }
				Com_sprintf(bot_skin, sizeof(bot_skin), skinnames[r]);
			}
	}
	else
	//	strncpy(bot_skin, skin);
		Q_strncpyz(bot_skin, skin, sizeof(bot_skin));
	
	// initialise userinfo
	memset (userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey (userinfo, "name", bot_name);
	Info_SetValueForKey (userinfo, "skin", bot_skin);
	Info_SetValueForKey (userinfo, "hand", "2"); // bot is center handed for now!

	ClientConnect (bot, userinfo);
	// Knightmare- removed this
	//ACESP_SaveBots(); // make sure to save the bots
}

///////////////////////////////////////////////////////////////////////
// Spawn the bot
///////////////////////////////////////////////////////////////////////
void ACESP_SpawnBot (char *team, char *name, char *skin, char *userinfo)
{
	edict_t	*bot;
	
	bot = ACESP_FindFreeClient ();
	
	if (!bot)
	{
		safe_bprintf (PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	bot->yaw_speed = 100; // yaw speed
	bot->inuse = true;
	bot->is_bot = true;

	// To allow bots to respawn
	if(userinfo == NULL)
		ACESP_SetName(bot, name, skin, team);
	else
		ClientConnect (bot, userinfo);
	
	G_InitEdict (bot);

	InitClientResp (bot->client);
	
	// locate ent at a spawn point
    if (ctf->value)
	{	// Knightmare- rewrote this
		edict_t		*player;
		int i;
		int team1count = 0, team2count = 0, team3count = 0;
		int jointeam;
		float r = random();

		for (i = 1; i <= maxclients->value; i++)
		{
			player = &g_edicts[i];
			if (!player->inuse || !player->client || player == bot)
				continue;
			switch (player->client->resp.ctf_team)
			{
			case CTF_TEAM1:
				team1count++;
				break;
			case CTF_TEAM2:
				team2count++;
				break;
			case CTF_TEAM3: 
				team3count++;
				break;
			}
		}

		if (ttctf->value)
		{
			if (team != NULL && strcmp(team,"red")==0)
				jointeam = CTF_TEAM1;
			else if (team != NULL && strcmp(team,"blue")==0)
				jointeam = CTF_TEAM2;
			else if (team != NULL && strcmp(team,"green")==0)
				jointeam = CTF_TEAM3;
			// join either of the outnumbered teams
			else if (team1count == team2count && team1count < team3count)
				jointeam = (r < 0.5) ? CTF_TEAM1 : CTF_TEAM2;
			else if (team1count == team3count && team1count < team2count)
				jointeam = (r < 0.5) ? CTF_TEAM1 : CTF_TEAM3;
			else if (team2count == team3count && team2count < team1count)
				jointeam = (r < 0.5) ? CTF_TEAM2 : CTF_TEAM3;
			// join outnumbered team
			else if (team1count < team2count && team1count < team3count)
				jointeam = CTF_TEAM1;
			else if (team2count < team1count &&  team2count < team3count) 
				jointeam = CTF_TEAM2;
			else if (team3count < team1count &&  team3count < team2count) 
				jointeam = CTF_TEAM3;
			// pick random team
			else if (r < 0.33)
				jointeam = CTF_TEAM1;
			else if (r < 0.66)
				jointeam = CTF_TEAM2;
			else
				jointeam = CTF_TEAM3;
		}
		else
		{
			if (team != NULL && strcmp(team,"red")==0)
				jointeam = CTF_TEAM1;
			else if (team != NULL && strcmp(team,"blue")==0)
				jointeam = CTF_TEAM2;
			// join outnumbered team
			else if (team1count < team2count)
				jointeam = CTF_TEAM1;
			else if (team2count < team1count)
				jointeam = CTF_TEAM2;
			// pick random team
			else if (r < 0.5)
				jointeam = CTF_TEAM1;
			else
				jointeam = CTF_TEAM2;
		}
		ACESP_PutClientInServer (bot,false, jointeam);
	}
	else
 		ACESP_PutClientInServer (bot,false,0);

	// make sure all view stuff is valid
	ClientEndServerFrame (bot);
	
	ACEIT_PlayerAdded (bot); // let the world know we added another

	ACEAI_PickLongRangeGoal(bot); // pick a new goal

}

///////////////////////////////////////////////////////////////////////
// Remove a bot by name or all bots
///////////////////////////////////////////////////////////////////////
void ACESP_RemoveBot(char *name)
{
	int i, j;
	qboolean freed=false;
	edict_t *bot;

	for(i=0;i<maxclients->value;i++)
	{
		bot = g_edicts + i + 1;
		if(bot->inuse)
		{
			if(bot->is_bot && (Q_stricmp(bot->client->pers.netname,name)==0 || Q_stricmp(name,"all")==0))
			{
				bot->health = 0;
				player_die (bot, bot, bot, 100000, vec3_origin);
				// don't even bother waiting for death frames
				bot->deadflag = DEAD_DEAD;
				bot->inuse = false;
				freed = true;
				ACEIT_PlayerRemoved (bot);

				safe_bprintf (PRINT_MEDIUM, "%s removed\n", bot->client->pers.netname);
				// Knightmare- decrement this bot name's counter and exit loop
				if (Q_stricmp(name,"all"))
				{
					for (j = 0; j < num_botinfo; j++)
						if (!Q_stricmp(name, bot_info[j].name))
						{
							bot_info[j].ingame_count--;
							bot_info[j].ingame_count = max(bot_info[j].ingame_count, 0);
							break;
						}
					break;
				}
			}
		}
	}

	if(!freed)	
		safe_bprintf (PRINT_MEDIUM, "%s not found\n", name);
	// Knightmare- removed this
	//ACESP_SaveBots(); // Save them again
}

