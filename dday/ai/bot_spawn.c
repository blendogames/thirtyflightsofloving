/*
Copyright (C) 1997-2001 Id Software, Inc.

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
--------------------------------------------------------------
The ACE Bot is a product of Steve Yeager, and is available from
the ACE Bot homepage, at http://www.axionfx.com/ace.

This program is a modification of the ACE Bot, and is therefore
in NO WAY supported by Steve Yeager.
*/

#include "../g_local.h"
#include "ai_local.h"

void change_stance(edict_t *self, int stance);
void Find_Mission_Start_Point(edict_t *ent, vec3_t origin, vec3_t angles);
void Spawn_Chute(edict_t *ent);


void Assign_Bot_Class (edict_t *self)
{
	int randnum;
	int taken = 0, available;
	int j;
	edict_t *cl_ent;


	randnum = rand()%8;
	 if (randnum == 0)
		self->client->resp.mos = SPECIAL;
	else if (randnum == 1 && strcmp (level.mapname,"soviet5"))
		self->client->resp.mos = L_GUNNER;
	else if (randnum == 2)
	{
		self->client->resp.mos = H_GUNNER;
	}
	else if (randnum == 3)
	{
		self->client->resp.mos = SNIPER;
	}
	else if (randnum == 4)
		self->client->resp.mos = OFFICER;
	else if (randnum == 5 && rand()%2 == 1 &&
	strcmp(self->client->resp.team_on->teamid, "jpn") && strcmp(self->client->resp.team_on->teamid, "rus"))
		self->client->resp.mos = FLAMER;
	else if (randnum == 6 && strcmp(self->client->resp.team_on->teamid, "gbr") && strcmp(self->client->resp.team_on->teamid, "pol"))
		self->client->resp.mos = ENGINEER;
	else if (randnum == 7 && rand()%2 == 1)
		self->client->resp.mos = MEDIC;
	else 
		self->client->resp.mos = INFANTRY;
		

	for (j=0 ; j < game.maxclients ; j++)
	{
		cl_ent = g_edicts + 1 + j;

		if (!cl_ent->inuse)
			continue;
		if (!cl_ent->client || 
			!cl_ent->client->resp.team_on ||
			!cl_ent->client->resp.team_on->mos ||
			cl_ent->client->resp.team_on->index != self->client->resp.team_on->index)
			continue; 
		
//			if (cl_ent == ent && ent->client->resp.mos == INFANTRY)
		if (cl_ent == self)// && (!self->client->resp.AlreadySpawned || self->client->resp.changeteam))
		continue;

		if (!self->client->resp.mos || 
			cl_ent->client->resp.mos != self->client->resp.mos)
			continue;

			taken++;
	}

	// Now set the available for this class
	switch (self->client->resp.mos) //crash
	{
		case INFANTRY:
			available = MAX_INFANTRY - taken;
			break;
		case OFFICER:
			available = MAX_OFFICERS - taken;
			break;
		case L_GUNNER:
			available = MAX_L_GUNNER - taken;
			break;
		case H_GUNNER:
			available = MAX_H_GUNNER - taken;
			break;
		case SNIPER:
			available = MAX_SNIPER - taken;
			break;
		case SPECIAL:
			available = MAX_SPECIAL - taken;
			break;
		case ENGINEER:
			available = MAX_ENGINEER - taken;
			break;
		case MEDIC:
			available = 1 - taken;
			break;
		case FLAMER:
			available = MAX_FLAMER - taken;
			break;
		default:
			available = 0;
			break;
	}

	if (mapclasslimits[self->client->resp.team_on->index][self->client->resp.mos].limit)
	{
		available = mapclasslimits[self->client->resp.team_on->index][self->client->resp.mos].limit;
	}
    
	//gi.dprintf ("%i  %i\n", self->client->resp.mos, available);
	if (available <= 0 )
		self->client->resp.mos = INFANTRY;



}
//===============================================================
//
//				BOT SPAWN
//
//===============================================================



///////////////////////////////////////////////////////////////////////
// Respawn the bot
///////////////////////////////////////////////////////////////////////
void BOT_Respawn (edict_t *self)
{
	vec3_t	spawn_origin, spawn_angles;


	CopyToBodyQue (self);

	PutClientInServer (self);

	// add a teleportation effect
	self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
	self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	self->client->ps.pmove.pm_time = 14;

	self->client->respawn_time = level.time;

	AI_ResetWeights(self);
	AI_ResetNavigation(self);

	change_stance(self, STANCE_STAND);



	Assign_Bot_Class (self);




	//join game
	self->movetype = MOVETYPE_WALK;
//	self->solid = SOLID_BBOX;
	self->svflags &= ~SVF_NOCLIENT;
	self->client->ps.gunindex = 0;
	self->client->forcespawn = level.time + RI->value;//.5;//faf: fixes standing corpse bug
	self->client->limbo_mode=false;
	self->stance_max=self->stance_min=0; //these 2 lines are for stances that have to be set
	self->stance_view=20;//faf 22;				//in each clientthink.
	self->die_time=0;
	self->client->jump_stamina = JUMP_MAX;
	self->flyingnun = false;
	self->client->syncspeed = false;//faf:  so we're frozen when in death view
	self->oldstance = STANCE_STAND;//faf
	self->client->last_fire_time = 0;
	SyncUserInfo(self, true); 
	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_WALK;
	self->viewheight = 20;//faf 22;
	self->inuse = true;
	//self->classname = "private";
	self->mass = 200;
	self->solid = SOLID_TRIGGER; //don't set this until seconds after respawn
	//self->client->OBTime=level.time+OBDELAY;
	self->deadflag = DEAD_NO;
	self->air_finished = level.time + 12;
	self->clipmask = MASK_PLAYERSOLID;
    self->svflags &= ~SVF_NOCLIENT; 
	self->wound_location=0;
	Give_Class_Weapon(self);
	Give_Class_Ammo(self);
	self->client->limbo_mode = false;
	self->client->resp.changeteam = false;
	self->client->respawn_time = level.time;
	self->client->spawntime = level.time;
	Find_Mission_Start_Point(self, spawn_origin, spawn_angles);
	gi.unlinkentity (self);
	self->client->ps.pmove.origin[0] = spawn_origin[0]*8;
	self->client->ps.pmove.origin[1] = spawn_origin[1]*8;
	self->client->ps.pmove.origin[2] = spawn_origin[2]*8;
	VectorCopy (spawn_origin, self->s.origin);
	self->s.origin[2] += 1;	// make sure off ground
	VectorCopy (self->s.origin, self->s.old_origin);
	// clear the velocity and hold them in place briefly
	VectorClear (self->velocity);
	self->client->ps.pmove.pm_time = 160>>3;		// hold time
	self->client->ps.pmove.pm_flags |= PMF_TIME_LAND; // pbowens: changed from PMF_TIME_TELEPORT, no particles 
	self->client->limbo_mode=false;
	gi.linkentity (self);
	self->client->resp.AlreadySpawned=true;
	WeighPlayer(self);
	self->client->landed = true;
/*	self->client->speedmax[0]=120;
	self->client->speedmax[1]=120;
	self->client->speedmax[2]=120;*/
//	self->client->speedmod = 2;
	self->ai->camp_targ = -1;

	if (self->client->resp.mos == SPECIAL)
	{
		Spawn_Chute(self);
	}



	self->solid = SOLID_TRIGGER;

	Set_VIP_Defense_Bot(self);


}

///////////////////////////////////////////////////////////////////////
// Find a free client spot - //jabot092(2)
///////////////////////////////////////////////////////////////////////
edict_t *BOT_FindFreeClient (void)
{
	edict_t *bot;
	edict_t	*ent;
	int	i;
	int max_count=0;

	bot = NULL;
//faf: hack to stop crash	

//	for( i = 0, ent = g_edicts + 1; i < game.maxclients; i++, ent++ ) 
	for( i = game.maxclients, ent = g_edicts + game.maxclients; i >= 0; i--, ent-- ) 
//	for( i = 16, ent = g_edicts + 17; i < game.maxclients; i++, ent++ ) 
	{
		if( !ent->inuse && bot == NULL )
			bot = ent;

		//count bots for bot names
		if( ent->count > max_count )
			max_count = ent->count;
	}

	if (bot == NULL || (max_count + 2) >= game.maxclients ) //always leave room for 1 player
		return NULL;

	bot->count = max_count + 1; // Will become bot name...

	return bot;
}

///////////////////////////////////////////////////////////////////////
// Set the name of the bot and update the userinfo
///////////////////////////////////////////////////////////////////////
void BOT_SetName(edict_t *bot, char *name, char *skin, int team)
{
//	float rnd;
	int  randname = 0;
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];
	int i,j;
	edict_t *e;
	qboolean gotname = false;


	char * usanames [14] = {"SaBOTeur", "BOTched", "loBOTomy", "Dipsymama", "saBOTage", "G.I. Joe", "Audie", "Winters", "Speirs", "Blithe", "Jim", "Bob", "Patton", "Eisenhower"};
	char * grmnames [14] = {"VerBOTen", "PolkaBOT", "BOTskrieg", "BOTwurst", "VolksBOT", "Adolf", "Lars", "Franz", "Frederick", "Niels", "Erik", "Goebbels", "Goering", "Rommel"};
	char * rusnames [14] = {"Stalin", "Zhukov", "Molotov", "Igor", "Vladimir", "Boris", "Viktor", "Dmitri", "KalashniBOT", "ShishkaBOT", "CosmoBOT", "Rostislav", "KGBot", "BOTshevik"}; 
	char * gbrnames [14] = {"Churchill", "Montgomery", "Prats", "Strokes", "Sgt. Leper", "Ewan", "Oliver", "Jack", "Edward", "Charles", "Ethan", "Jeeves", "Godfrey", "Thomas"};
	char * polnames [14] = {"Jakub", "Krzysztof", "Szymon", "Maciej", "Filip", "Pawel", "Kamil", "Kacper", "Bartek", "Nilew", "Dominik", "Fryderyk", "Gustaw", "Jozef"};
	char * itanames [14] = {"Mario", "Luigi", "Fat Tony", "Don Bot", "Adolfo", "Alonzo", "Angelo", "Aldo", "Mussolini", "Carlo", "Emilio", "Vincenzo", "Giovanni", "Paolo"};
	char * jpnnames [14] = {"Daisuke", "Kazuki", "Takumi", "Tatsuya", "Naoto", "Daichi", "Izumi", "Katsutohi", "Shinakio", "Naoko", "Tojo", "Hirohito", "Yamamoto", "Miyagi"};
	char * ddaynames [14] = {"Dummy", "Muff", "Dimferno", "Dipsymama", "Botcom", "Vap'd", "Hack Bot 1", "Hack Bot 2", "Icky", "Moho", "Dork", "MentalSod", "GriNerd", "tomatovego"};

	for (j=0; j<50 && gotname == false; j++)
	{

		randname = (int)rand()%14;

	if (!strcmp (team_list[team]->teamid, "grm"))
		Com_sprintf (bot_name, sizeof(bot_name), "%s", grmnames[randname]);
	else if (!strcmp (team_list[team]->teamid, "rus"))
		Com_sprintf (bot_name, sizeof(bot_name), "%s", rusnames[randname]);
	else if (!strcmp (team_list[team]->teamid, "gbr"))
		Com_sprintf (bot_name, sizeof(bot_name), "%s", gbrnames[randname]);
	else if (!strcmp (team_list[team]->teamid, "pol"))
		Com_sprintf (bot_name, sizeof(bot_name), "%s", polnames[randname]);
	else if (!strcmp (team_list[team]->teamid, "ita"))
		Com_sprintf (bot_name, sizeof(bot_name), "%s", itanames[randname]);
	else if (!strcmp (team_list[team]->teamid, "jpn"))
		Com_sprintf (bot_name, sizeof(bot_name), "%s", jpnnames[randname]);
	else 
		Com_sprintf (bot_name, sizeof(bot_name), "%s", usanames[randname]);

		gotname = true;

		for (i=0 ; i<maxclients->value ; i++)
		{
			e = g_edicts + 1 + i;
			if (!e->inuse)
				continue;

			if (!strcmp (bot_name, e->client->pers.netname))
				gotname = false;

		}
	}	


	if ((int)rand()%35 == 1)
		Com_sprintf (bot_name, sizeof(bot_name), "%s", ddaynames[randname]);




	// Set the name for the bot.
	// name
/*	if(strlen(name) == 0)
	{
		Com_sprintf (bot_name, sizeof(bot_name), "Bot%d", bot->count);
	}
	else
		Q_strncpyz (bot_name, sizeof(bot_name), name);
*/
/*	
	// skin
	if(strlen(skin) == 0)
	{
		// randomly choose skin 
		rnd = random();
	if(rnd  < 0.05)
			Com_sprintf (bot_skin, sizeof(bot_skin), "female/athena");
		else if(rnd < 0.1)
			Com_sprintf (bot_skin, sizeof(bot_skin), "female/brianna");
		else if(rnd < 0.15)
			Com_sprintf (bot_skin, sizeof(bot_skin), "female/cobalt");
		else if(rnd < 0.2)
			Com_sprintf (bot_skin, sizeof(bot_skin), "female/ensign");
		else if(rnd < 0.25)
			Com_sprintf (bot_skin, sizeof(bot_skin), "female/jezebel");
		else if(rnd < 0.3)
			Com_sprintf (bot_skin, sizeof(bot_skin), "female/jungle");
		else if(rnd < 0.35)
			Com_sprintf (bot_skin, sizeof(bot_skin), "female/lotus");
		else if(rnd < 0.4)
			Com_sprintf (bot_skin, sizeof(bot_skin), "female/stiletto");
		else if(rnd < 0.45)
			Com_sprintf (bot_skin, sizeof(bot_skin), "female/venus");
		else if(rnd < 0.5)
			Com_sprintf (bot_skin, sizeof(bot_skin), "female/voodoo");
		else if(rnd < 0.55)
			Com_sprintf (bot_skin, sizeof(bot_skin), "male/cipher");
		else if(rnd < 0.6)
			Com_sprintf (bot_skin, sizeof(bot_skin), "male/flak");
		else if(rnd < 0.65)
			Com_sprintf (bot_skin, sizeof(bot_skin), "male/grunt");
		else if(rnd < 0.7)
			Com_sprintf (bot_skin, sizeof(bot_skin), "male/howitzer");
		else if(rnd < 0.75)
			Com_sprintf (bot_skin, sizeof(bot_skin), "male/major");
		else if(rnd < 0.8)
			Com_sprintf (bot_skin, sizeof(bot_skin), "male/nightops");
		else if(rnd < 0.85)
			Com_sprintf (bot_skin, sizeof(bot_skin), "male/pointman");
		else if(rnd < 0.9)
			Com_sprintf (bot_skin, sizeof(bot_skin), "male/psycho");
		else if(rnd < 0.95)
			Com_sprintf (bot_skin, sizeof(bot_skin), "male/razor");
		else 
			Com_sprintf (bot_skin, sizeof(bot_skin), "male/sniper");
	}
	else
		Q_strncpyz (bot_skin, sizeof(bot_skin), skin);
*/

			Com_sprintf (bot_skin, sizeof(bot_skin), "grm/class_infantry");

	// initialise userinfo
	memset (userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey (userinfo, "name", bot_name);
	Info_SetValueForKey (userinfo, "skin", bot_skin);
	Info_SetValueForKey (userinfo, "hand", "2"); // bot is center handed for now!


	ClientConnect (bot, userinfo);

//	ACESP_SaveBots(); // make sure to save the bots
}


//==========================================
// BOT_NextCTFTeam
// Get the emptier CTF team
//==========================================
int	BOT_NextCTFTeam()
{
		return 0;
}

//==========================================
// BOT_JoinCTFTeam
// Assign a team for the bot
//==========================================
qboolean BOT_JoinCTFTeam (edict_t *ent, char *team_name)
{
return false;
/*	char	*s;
	int		team;
//	edict_t	*event;


	if (ent->client->resp.ctf_team != CTF_NOTEAM)
		return false;
	
	// find what ctf team
	if ((team_name !=NULL) && (strcmp(team_name, "blue") == 0))
		team = CTF_TEAM2;
	else if ((team_name !=NULL) && (strcmp(team_name, "red") == 0))
		team = CTF_TEAM1;
	else
		team = BOT_NextCTFTeam();

	if (team == CTF_NOTEAM)
		return false;
	
	//join ctf team
	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.ctf_state = 1;//0?
	ent->client->resp.ctf_team = team;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

	PutClientInServer(ent);

	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;

	Com_Printf ( "%s joined the %s team.\n",
		ent->client->pers.netname, CTFTeamName(ent->client->resp.ctf_team));

	return true;*/
}

//if we're doing VIP mode and there is a VIP to defend and attack, assign a certain number of bots to defend.
void Set_VIP_Defense_Bot (edict_t *ent)
{
	qboolean  allied_vip, axis_vip;
	
	edict_t *e, *bot;
	int i;
	int teamcount;
	int	numdefenders;

	allied_vip = false;
	axis_vip = false;
	numdefenders = 0;




	ent->ai->defend_bot = false;

    for (i=0 ; i<globals.num_edicts ; i++)
	{
		e = &g_edicts[i];

		if (!e->inuse)
			continue;

		if (!e->classnameb)
			continue;

		if (e->classnameb != OBJECTIVE_VIP)
			continue;

		if (e->obj_owner == 0)
			allied_vip = true;

		if (e->obj_owner == 1)
			axis_vip = true;
	}


	if (allied_vip == true && axis_vip == true)
	{
		for(i=0;i<maxclients->value;i++)
		{
			bot = g_edicts + i + 1;
			if( !bot->inuse || !bot->ai )
				continue;
			if (!bot->client && !bot->client->resp.team_on)
				continue;
			if (bot->client->resp.team_on->index !=
				ent->client->resp.team_on->index)
				continue;
			if (bot->ai->defend_bot == true)
				numdefenders++;
		
		}

		teamcount = PlayerCountForTeam(ent->client->resp.team_on->index, true);

		if (teamcount == 2)
		{
			if (numdefenders == 0)	
				ent->ai->defend_bot = true;
		}
		else if (teamcount == 3)
		{
			if (numdefenders ==0)	
				ent->ai->defend_bot = true;
		}
		else if (teamcount > 2 && teamcount < 6)
		{
			if (numdefenders < 2)
				ent->ai->defend_bot = true;
		}
		else if (teamcount > 5)
		{
			if (numdefenders < 3)	
				ent->ai->defend_bot = true;
		}
		//gi.dprintf("myteam:%i tcount:%i numdef %i defendbot? %i\n", ent->client->resp.team_on->index, teamcount, numdefenders, ent->ai->defend_bot);
	}

}
//==========================================
// BOT_DMClass_JoinGame
// put the bot into the game.
//==========================================
void BOT_DMClass_JoinGame (edict_t *ent, char *team_name)
{

	vec3_t	spawn_origin, spawn_angles;
	int randnum = rand()%5;

	if ( !BOT_JoinCTFTeam(ent, team_name) )
		Com_Printf ( "%s joined the game.\n",
		ent->client->pers.netname);

	ent->think = AI_Think;
	ent->nextthink = level.time + FRAMETIME;

//	ent->client->resp.team_on=team_list[1];


	Assign_Bot_Class (ent);


	//join game
	ent->movetype = MOVETYPE_WALK;
	//ent->solid = SOLID_BBOX;
	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->ps.gunindex = 0;

	ent->client->forcespawn = level.time + RI->value;//.5;//faf: fixes standing corpse bug

	ent->client->limbo_mode=false;
	ent->stance_max=ent->stance_min=0; //these 2 lines are for stances that have to be set
	ent->stance_view=20;//faf 22;				//in each clientthink.
	ent->die_time=0;
	ent->client->jump_stamina = JUMP_MAX;
	ent->flyingnun = false;
	ent->client->syncspeed = false;//faf:  so we're frozen when in death view
	ent->oldstance = STANCE_STAND;//faf
	ent->client->last_fire_time = 0;
	SyncUserInfo(ent, true); 
	ent->takedamage = DAMAGE_YES;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 20;//faf 22;
	ent->inuse = true;
	//ent->classname = "private";
	ent->mass = 200;
	ent->solid = SOLID_TRIGGER; //don't set this until seconds after respawn
	//ent->client->OBTime=level.time+OBDELAY;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
    ent->svflags &= ~SVF_NOCLIENT; 
	ent->wound_location=0;
	Give_Class_Weapon(ent);
	Give_Class_Ammo(ent);
	ent->client->limbo_mode = false;
	ent->client->resp.changeteam = false;
	ent->client->respawn_time = level.time;
	ent->client->spawntime = level.time;
	Find_Mission_Start_Point(ent, spawn_origin, spawn_angles);
	gi.unlinkentity (ent);
	ent->client->ps.pmove.origin[0] = spawn_origin[0]*8;
	ent->client->ps.pmove.origin[1] = spawn_origin[1]*8;
	ent->client->ps.pmove.origin[2] = spawn_origin[2]*8;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);
	// clear the velocity and hold them in place briefly
	VectorClear (ent->velocity);
	ent->client->ps.pmove.pm_time = 160>>3;		// hold time
	ent->client->ps.pmove.pm_flags |= PMF_TIME_LAND; // pbowens: changed from PMF_TIME_TELEPORT, no particles 
	ent->client->limbo_mode=false;
	gi.linkentity (ent);
	ent->client->resp.AlreadySpawned=true;
	WeighPlayer(ent);
	ent->client->landed = true;
/*	ent->client->speedmax[0]=120;
	ent->client->speedmax[1]=120;
	ent->client->speedmax[2]=120;*/
//	ent->client->speedmod = 2;
	ent->ai->camp_targ = -1;

	if (ent->client->resp.mos == SPECIAL)
	{
		Spawn_Chute(ent);
	}

//	if (!KillBox (ent))
//	{	// could't spawn in?
//	}
	gi.linkentity (ent);

	change_stance(ent, STANCE_STAND);

	
	Set_VIP_Defense_Bot(ent);


}

//==========================================
// BOT_StartAsSpectator
//==========================================
void BOT_StartAsSpectator (edict_t *ent)
{
	// start as 'observer'
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
//	ent->client->resp.ctf_team = CTF_NOTEAM;
	ent->client->ps.gunindex = 0;
	gi.linkentity (ent);
}


//==========================================
// BOT_JoinGame
// 3 for teams and such
//==========================================
void BOT_JoinBlue (edict_t *ent)
{
	BOT_DMClass_JoinGame( ent, "blue" );
}
void BOT_JoinRed (edict_t *ent)
{
	BOT_DMClass_JoinGame( ent, "red" );
}
void BOT_JoinGame (edict_t *ent)
{
	BOT_DMClass_JoinGame( ent, NULL );
}

qboolean nobotwarn = false;
///////////////////////////////////////////////////////////////////////
// Spawn the bot
///////////////////////////////////////////////////////////////////////
void BOT_SpawnBot (int team, char *name, char *skin, char *userinfo)
{
	edict_t	*bot;



	//if (!nobotwarn && !nav.loaded)
	if (!nobotwarn && (nav.loaded == -1))
	{
		nobotwarn = true;
		Com_Printf("No .nav file for this map, can't load bots!\n");
		return;
	}
	if (!nobotwarn && total_camp_spots == 0)
	{
		nobotwarn = true;
		Com_Printf("No .cmp file for this map, can't load bots!\n");
		return;
	}
	
	//	if( !nav.loaded ) {
	if (nav.loaded == -1 || total_camp_spots == 0){
		//Com_Printf("Can't spawn bots without a valid navigation file\n");
		return;
	}
	


	if (level.intermissiontime)
		return;
		

	bot = BOT_FindFreeClient ();
	
	if (!bot)
	{
//		safe_bprintf (PRINT_MEDIUM, "Server is full, increase Maxclients.\n");
		return;
	}

	
	qbots = true;



	//init the bot
	bot->inuse = true;
	bot->yaw_speed = 100;



	// To allow bots to respawn
	if(userinfo == NULL)
		BOT_SetName(bot, name, skin, team);
	else
	{
		ClientConnect (bot, userinfo);
	}



	G_InitEdict (bot);
	G_SpawnAI(bot); //jabot092(2)
	bot->ai->is_bot = true;

	InitClientResp (bot->client);

	PutClientInServer(bot);
	BOT_StartAsSpectator (bot);

	//skill
	bot->ai->pers.skillLevel = (int)(random()*MAX_BOT_SKILL);
	if (bot->ai->pers.skillLevel > MAX_BOT_SKILL)	//fix if off-limits
		bot->ai->pers.skillLevel =  MAX_BOT_SKILL;
	else if (bot->ai->pers.skillLevel < 0)
		bot->ai->pers.skillLevel =  0;

	BOT_DMclass_InitPersistant(bot);
	AI_ResetWeights(bot);
	AI_ResetNavigation(bot);


	bot->client->resp.team_on = team_list[team];

	bot->think = BOT_JoinGame;


	if (level.time >= level_wait->value + bot->client->resp.team_on->delay)
		bot->nextthink = level.time + (random()*2.0);
	else
		bot->nextthink = level_wait->value + bot->client->resp.team_on->delay + (random()*2.0);


/*	if( ctf->value && team != NULL )
	{
		if( !Q_stricmp( team, "blue" ) )
			bot->think = BOT_JoinBlue;
		else if( !Q_stricmp( team, "red" ) )
			bot->think = BOT_JoinRed;
	}
*/	
	AI_EnemyAdded (bot); // let the ai know we added another
	bot->solid = SOLID_TRIGGER;
}


///////////////////////////////////////////////////////////////////////
// Remove a bot by name or all bots
///////////////////////////////////////////////////////////////////////
void BOT_RemoveBot(char *name, edict_t *botremove)
{
	int i;
	qboolean freed=false;
	edict_t *bot;

	for(i=0;i<maxclients->value;i++)
	{
		bot = g_edicts + i + 1;
		if( !bot->inuse || !bot->ai )  //jabot092(2)
			continue;
		
		if( bot->ai->is_bot && 
			//(!strcmp(bot->client->pers.netname,name) || !strcmp(name,"all")) ||
			botremove == bot)
		{
			bot->health = 0;
			meansOfDeath = MOD_SUICIDE;
			player_die (bot, bot, bot, 100000, vec3_origin);

			bot->solid = SOLID_NOT; 
			bot->svflags |= SVF_NOCLIENT; 
			bot->takedamage = DAMAGE_NO;

			// don't even bother waiting for death frames
			bot->deadflag = DEAD_DEAD;
			//bot->s.frame=197;
			bot->s.modelindex = 0;
			bot->inuse = false;
			freed = true;
			AI_EnemyRemoved (bot);
			G_FreeAI( bot ); //jabot092(2)
			//safe_bprintf (PRINT_MEDIUM, "%s removed\n", bot->client->pers.netname);
		}
	}

//	if(!freed && !Q_stricmp( name, "all") )
//		safe_bprintf (PRINT_MEDIUM, "%s not found\n", name);
}
