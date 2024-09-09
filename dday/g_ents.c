/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_ents.c,v $
 *   $Revision: 1.8 $
 *   $Date: 2002/07/23 21:11:37 $
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
#include "g_cmds.h"

// g_ents.c
// D-Day: Normandy Old / Unsorted Entities



#define NO_MISSION 99

void multi_trigger (edict_t *ent);
void InitTrigger (edict_t *self);
int Last_Team_Winner = 99;
//char map_tree[MAX_TEAMS][MAX_QPATH];

/*********************************************
** This is for all of the custom entity and
** trigger functions for DDay
*********************************************/


/////////////////////////////////////////////////////
//the following functions keep count of how many troops have triggered it


//count is total number required...
//heath is the mission number (0 defence, 1 offence, 2 patrol)
//dmg is the ammount of pointes added to the team
//delay is the time to win before the game is won


/////////////////////
void trigger_enough_troops_use (edict_t *self, edict_t *other, edict_t *activator)
{
	int i;

	gi.dprintf("trigger_enough_troops_use called\n");

	if ( (self->obj_owner < MAX_TEAMS) && (activator->client->resp.team_on->index != self->obj_owner) )
	{
		gi.dprintf("TRIGGER return code 1\n");
		return;
	}

	if (self->count == 0) {
		gi.dprintf("TRIGGER return code 2 \n");
		return;
	}


	if ( (activator->client->resp.team_on->mission != self->health) && (self->health != NO_MISSION) )
	{
		team_list[self->obj_owner]->time_to_win=0;
		self->obj_owner=activator->client->resp.team_on->index;
		gi.dprintf("TRIGGER return code 3\n");
		return;				// if not for this mission get out of here...
	}

	for(i=0; i < MAX_TEAM_MATES; i++)
	{
		if (self->who_touched[i])
		{	//if they are already registered...
			if (self->who_touched[i]==activator)
				i=MAX_TEAM_MATES;
			continue;
		}
		//otherwise go on through and add the to the list
		self->count--;
		self->who_touched[i]=activator;
		self->obj_owner=activator->client->resp.team_on->index;

		if (self->count)
		{
			if (! (self->spawnflags & 1))
			{
				safe_centerprintf(activator, "%i more to go...", self->count);
			}
			gi.dprintf("TRIGGER return code 4\n");
			return;
		}

		else
		{

			if (! (self->spawnflags & 1))
			{
				safe_centerprintf(activator, "Ok, we got 'em all here!");
			}
			activator->client->resp.team_on->score=self->dmg;
			team_list[self->obj_owner]->time_to_win+=self->delay;
		}


		gi.dprintf("TRIGGER return code 5\n");
		self->activator = activator;
		multi_trigger (self);
		i = MAX_TEAM_MATES;
	}
	gi.dprintf("TRIGGER return code 6\n");
}


void SP_trigger_enough_troops (edict_t *self)
{
	self->wait = -1;
	if (!self->count)
		self->count = 2;

	self->who_touched = gi.TagMalloc((sizeof(edict_t)*MAX_TEAM_MATES),TAG_LEVEL);
	self->use = trigger_enough_troops_use;
}

void SP_info_Mission_Results (edict_t *ent)
{
// put next map info here.
/*	strncpy (map_tree[ent->dmg], ent->map);

	if (!ent->classname)
		strncpy (map_tree[ent->mass], level.mapname);
	else
		strncpy (map_tree[ent->mass], ent->classname);

	G_FreeEdict(ent); */
}

/////////////////////////////////////////////////////////////////////////////////
//The following functions keep track of who owns the entity
/////////////////////////////////////////////////////////////////////////////////
//dmg is points for owning objtive
void target_objective_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->obj_owner==activator->client->resp.team_on->index)
		return; // if team already in possesion, continue

	if (team_list[self->obj_owner]!=NULL)
		team_list[self->obj_owner]->score-=self->dmg;

	safe_bprintf(PRINT_HIGH, "%s has captured an objective point for team %s!\n",
		activator->client->pers.netname,
		team_list[self->obj_owner]->teamname);

	self->obj_owner=activator->client->resp.team_on->index;
	team_list[self->obj_owner]->score+=self->dmg;
}

void SP_target_objective(edict_t *ent)
{
	ent->wait=-1;
	ent->use=target_objective_use;

	if (ent->delay && ent->obj_owner != 99)
		team_list[ent->obj_owner]->time_to_win = (level.time + ent->delay);
}




////////////////////////////////////////////////////////////////////////////////
//The following is for creating teams on spawn
////////////////////////////////////////////////////////////////////////////////
//int InitializeUserDLLs(userdll_list_node_t *unode,int teamindex);
//userdll_list_node_t *LoadUserDLLs(edict_t *ent, int team);

/*
void reinforcement_think1(edict_t *ent)
{
	if (!strcmp(ent->classname, "info_infantry_start"))
	{
		team_list[ent->obj_owner]->mos[INFANTRY]->max_available = ent->count;
	}
	else if (!strcmp(ent->classname, "info_officer_start"))
	{
		team_list[ent->obj_owner]->mos[OFFICER]->max_available = ent->count;
	}
	else if (!strcmp(ent->classname, "info_lgunner_start"))
	{
		team_list[ent->obj_owner]->mos[L_GUNNER]->max_available = ent->count;
	}
	else if (!strcmp(ent->classname, "info_hgunner_start"))
	{
		team_list[ent->obj_owner]->mos[H_GUNNER]->max_available = ent->count;
	}
	else if (!strcmp(ent->classname, "info_sniper_start"))
	{
		team_list[ent->obj_owner]->mos[SNIPER]->max_available = ent->count;
	}
	else if (!strcmp(ent->classname, "info_special_start"))
	{
		team_list[ent->obj_owner]->mos[SPECIAL]->max_available = ent->count;
	}
	else if (!strcmp(ent->classname, "info_engineer_start"))
	{
		team_list[ent->obj_owner]->mos[ENGINEER]->max_available = ent->count;
	}
	else if (!strcmp(ent->classname, "info_medic_start"))
	{
		team_list[ent->obj_owner]->mos[MEDIC]->max_available = ent->count;
	}
	else if (!strcmp(ent->classname, "info_flamer_start"))
	{
		team_list[ent->obj_owner]->mos[FLAMER]->max_available = ent->count;
	}
}*/

void bot_spawn(edict_t *ent)
{}

void SP_info_reinforcement_start(edict_t *ent)
{
//	ent->think= reinforcement_think;
//	ent->nextthink = level.time+((ent->delay)?ent->delay : RI->value);
//	ent->nextspawn = ent->nextthink;
	ent->spawnpoint = true;
}

void SP_info_reinforcement_startx(edict_t *ent)
{
//	ent->think= reinforcement_think;
//	ent->nextthink = level.time+((ent->delay)?ent->delay : RI->value);
//	ent->nextspawn = ent->nextthink;
	ent->spawnpoint = true;


/*needs work
	if (ent->count  && ent->obj_owner)
	{
		safe_bprintf (PRINT_HIGH, "%i %i.\n", ent->count, ent->obj_owner);
//		ent->think = reinforcement_think1;
//		ent->nextthink = level.time + 1;
	}
*/

}


gitem_t *InsertItem (gitem_t *it, spawn_t *spawnInfo);

// Knightmare- made these vars extern to fix compile on GCC
extern gitem_t usaitems[MAX_TEAM_ITEMS];
extern spawn_t sp_usa[MAX_TEAM_ITEMS];
extern SMos_t USA_MOS_List[NUM_CLASSES];
void USA_UserPrecache (void);

extern gitem_t grmitems[MAX_TEAM_ITEMS];
extern spawn_t sp_grm[MAX_TEAM_ITEMS];
extern SMos_t GRM_MOS_List[NUM_CLASSES];
void GRM_UserPrecache (void);

extern gitem_t rusitems[MAX_TEAM_ITEMS];
extern spawn_t sp_rus[MAX_TEAM_ITEMS];
extern SMos_t RUS_MOS_List[NUM_CLASSES];
void RUS_UserPrecache (void);

extern gitem_t gbritems[MAX_TEAM_ITEMS];
extern spawn_t sp_gbr[MAX_TEAM_ITEMS];
extern SMos_t GBR_MOS_List[NUM_CLASSES];
void GBR_UserPrecache (void);

extern gitem_t politems[MAX_TEAM_ITEMS];
extern spawn_t sp_pol[MAX_TEAM_ITEMS];
extern SMos_t POL_MOS_List[NUM_CLASSES];
void POL_UserPrecache (void);

extern gitem_t itaitems[MAX_TEAM_ITEMS];
extern spawn_t sp_ita[MAX_TEAM_ITEMS];
extern SMos_t ITA_MOS_List[NUM_CLASSES];
void ITA_UserPrecache (void);

extern gitem_t jpnitems[MAX_TEAM_ITEMS];
extern spawn_t sp_jpn[MAX_TEAM_ITEMS];
extern SMos_t JPN_MOS_List[NUM_CLASSES];
void JPN_UserPrecache (void);

extern gitem_t usmitems[MAX_TEAM_ITEMS];
extern spawn_t sp_usm[MAX_TEAM_ITEMS];
extern SMos_t USM_MOS_List[NUM_CLASSES];
void USM_UserPrecache (void);

void InitTeam (char *team, int i)
{
	int j;

	if (!strcmp(team, "usa"))
	{	for(j=0;usaitems[j].classname;j++)
		{	usaitems[j].mag_index=i;
			InsertItem(&usaitems[j],&sp_usa[j]);
		}
		InitMOS_List(team_list[i],USA_MOS_List);
		USA_UserPrecache();
		usa_index = i;
	}
	else if (!strcmp(team, "grm"))
	{	for(j=0;grmitems[j].classname;j++)
		{	grmitems[j].mag_index=i;
			InsertItem(&grmitems[j],&sp_grm[j]);
		}
		InitMOS_List(team_list[i],GRM_MOS_List);
		GRM_UserPrecache();
		grm_index = i;

	}
	else if (!strcmp(team, "rus"))
	{	for(j=0;rusitems[j].classname;j++)
		{	rusitems[j].mag_index=i;
			InsertItem(&rusitems[j],&sp_rus[j]);
		}
		InitMOS_List(team_list[i],RUS_MOS_List);
		RUS_UserPrecache();
		rus_index = i;

	}
	else if (!strcmp(team, "gbr"))
	{	for(j=0;gbritems[j].classname;j++)
		{	gbritems[j].mag_index=i;
			InsertItem(&gbritems[j],&sp_gbr[j]);
		}
		InitMOS_List(team_list[i],GBR_MOS_List);
		GBR_UserPrecache();
		gbr_index = i;

	}
	else if (!strcmp(team, "pol"))
	{	for(j=0;politems[j].classname;j++)
		{	politems[j].mag_index=i;
			InsertItem(&politems[j],&sp_pol[j]);
		}
		InitMOS_List(team_list[i],POL_MOS_List);
		POL_UserPrecache();
		pol_index = i;

	}
	else if (!strcmp(team, "ita"))
	{	for(j=0;itaitems[j].classname;j++)
		{	itaitems[j].mag_index=i;
			InsertItem(&itaitems[j],&sp_ita[j]);
		}
		InitMOS_List(team_list[i],ITA_MOS_List);
		ITA_UserPrecache();
		ita_index = i;

	}
	else if (!strcmp(team, "jpn"))
	{	for(j=0;jpnitems[j].classname;j++)
		{	jpnitems[j].mag_index=i;
			InsertItem(&jpnitems[j],&sp_jpn[j]);
		}
		InitMOS_List(team_list[i],JPN_MOS_List);
		JPN_UserPrecache();
		jpn_index = i;

	}
	else if (!strcmp(team, "usm"))
	{	for(j=0;usmitems[j].classname;j++)
		{	usmitems[j].mag_index=i;
			InsertItem(&usmitems[j],&sp_usm[j]);
		}
		InitMOS_List(team_list[i],USM_MOS_List);
		USM_UserPrecache();
		usm_index = i;

	}
	else if (i==0)
	{	for(j=0;usaitems[j].classname;j++)
		{	usaitems[j].mag_index=i;
			InsertItem(&usaitems[j],&sp_usa[j]);
		}
		InitMOS_List(team_list[i],USA_MOS_List);
		USA_UserPrecache();
		usa_index = i;
	}
	else if (i==1)
	{	for(j=0;grmitems[j].classname;j++)
		{	grmitems[j].mag_index=i;
			InsertItem(&grmitems[j],&sp_grm[j]);
		}
		InitMOS_List(team_list[i],GRM_MOS_List);
		GRM_UserPrecache();
		grm_index = i;

	}

	if (team_list[0] && team_list[1])
		SetItemNames ();


}

void LoadBotChat (int teamnum, char *teamid)
{
	char	*tempchat;
	char	*s, *f = NULL;
	int		c;
	char	filename[MAX_QPATH] = "";

	c = 0;

/*	Com_sprintf (filename, sizeof(filename), "dday/botchat/%s-sorry.txt", teamid);
	tempchat = ReadEntFile(filename);
	if (!tempchat)
		tempchat = ReadEntFile("dday/botchat/usa-sorry.txt"); */
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/botchat/%s-sorry.txt", GameDir(), teamid);
	tempchat = ReadEntFile(filename);
	if (!tempchat) {
		Com_sprintf (filename, sizeof(filename), "%s/botchat/usa-sorry.txt", GameDir());
		tempchat = ReadEntFile(filename);
	}

	if (tempchat)
	{
	//	f = strdup (tempchat);
		f = G_CopyString (tempchat);	// Knightmare- use G_CopyString instead
		s = strtok(f, "\n");
		Com_sprintf (botchat_sorry[teamnum][c], sizeof(botchat_sorry[teamnum][c]), "%s", s);
		while (s!= NULL) {
			s = strtok (NULL, "\n");
			c++;
			Com_sprintf (botchat_sorry[teamnum][c], sizeof(botchat_sorry[teamnum][c]), "%s", s);
		}
		botchat_sorry_count[teamnum] = c;
	}

	// Knightmare- free temp buffer
	if (f != NULL) {
		gi.TagFree (f);
		f = NULL;
	}

	c = 0;

/*	Com_sprintf (filename, sizeof(filename), "dday/botchat/%s-killed.txt", teamid);
	tempchat = ReadEntFile(filename);
	if (!tempchat)
		tempchat = ReadEntFile("dday/botchat/usa-killed.txt"); */
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/botchat/%s-killed.txt", GameDir(), teamid);
	tempchat = ReadEntFile(filename);
	if (!tempchat) {
		Com_sprintf (filename, sizeof(filename), "%s/botchat/usa-killed.txt", GameDir());
		tempchat = ReadEntFile(filename);
	}

	if (tempchat)
	{
	//	f = strdup (tempchat);
		f = G_CopyString (tempchat);	// Knightmare- use G_CopyString instead
		s = strtok(f, "\n");
		Com_sprintf (botchat_killed[teamnum][c], sizeof(botchat_killed[teamnum][c]), "%s", s);
		while (s!= NULL) {
			s = strtok (NULL, "\n");
			c++;
			Com_sprintf (botchat_killed[teamnum][c], sizeof(botchat_killed[teamnum][c]), "%s", s);
		}
		botchat_killed_count[teamnum] = c;
	}

	// Knightmare- free temp buffer
	if (f != NULL) {
		gi.TagFree (f);
		f = NULL;
	}

	c = 0;

/*	Com_sprintf (filename, sizeof(filename), "dday/botchat/%s-forgive.txt", teamid);
	tempchat = ReadEntFile(filename);
	if (!tempchat)
		tempchat = ReadEntFile("dday/botchat/usa-forgive.txt"); */
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/botchat/%s-forgive.txt", GameDir(), teamid);
	tempchat = ReadEntFile(filename);
	if (!tempchat) {
		Com_sprintf (filename, sizeof(filename), "%s/botchat/usa-forgive.txt", GameDir());
		tempchat = ReadEntFile(filename);
	}

	if (tempchat)
	{
	//	f = strdup (tempchat);
		f = G_CopyString (tempchat);	// Knightmare- use G_CopyString instead
		s = strtok(f, "\n");
		Com_sprintf (botchat_forgive[teamnum][c], sizeof(botchat_forgive[teamnum][c]), "%s", s);
		while (s!= NULL) {
			s = strtok (NULL, "\n");
				c++;
				Com_sprintf (botchat_forgive[teamnum][c], sizeof(botchat_forgive[teamnum][c]), "%s", s);
		}
		botchat_forgive_count[teamnum] = c;
	}

	// Knightmare- free temp buffer
	if (f != NULL) {
		gi.TagFree (f);
		f = NULL;
	}

	c = 0;

/*	Com_sprintf (filename, sizeof(filename), "dday/botchat/%s-self.txt", teamid);
	tempchat = ReadEntFile(filename);
	if (!tempchat)
		tempchat = ReadEntFile("dday/botchat/usa-self.txt"); */
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/botchat/%s-self.txt", GameDir(), teamid);
	tempchat = ReadEntFile(filename);
	if (!tempchat) {
		Com_sprintf (filename, sizeof(filename), "%s/botchat/usa-self.txt", GameDir());
		tempchat = ReadEntFile(filename);
	}

	if (tempchat)
	{
	//	f = strdup (tempchat);
		f = G_CopyString (tempchat);	// Knightmare- use G_CopyString instead
		s = strtok (f, "\n");
		Com_sprintf (botchat_self[teamnum][c], sizeof(botchat_self[teamnum][c]), "%s", s);
		while (s!= NULL) {
			s = strtok (NULL, "\n");
				c++;
				Com_sprintf (botchat_self[teamnum][c], sizeof(botchat_self[teamnum][c]), "%s", s);
		}
		botchat_self_count[teamnum] = c;
	}

	// Knightmare- free temp buffer
	if (f != NULL) {
		gi.TagFree (f);
		f = NULL;
	}
/*
	c = 0;

//	Com_sprintf (filename, sizeof(filename), "dday/botchat/%s-random.txt", teamid);
//	tempchat = ReadEntFile(filename);
//	if (!tempchat)
//		tempchat = ReadEntFile("dday/botchat/usa-random.txt");
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/botchat/%s-random.txt", GameDir(), teamid);
	tempchat = ReadEntFile(filename);
	if (!tempchat) {
		Com_sprintf (filename, sizeof(filename), "%s/botchat/usa-random.txt", GameDir());
		tempchat = ReadEntFile(filename);
	}

	if (tempchat)
	{
	//	f = strdup (tempchat);
		f = G_CopyString (tempchat);	// Knightmare- use G_CopyString instead
		s = strtok(f, "\n");
		Com_sprintf (botchat_random[teamnum][c], sizeof(botchat_random[teamnum][c]), "%s", s);
		while (s!= NULL) {
			s = strtok (NULL, "\n");
			c++;
			Com_sprintf (botchat_random[teamnum][c], sizeof(botchat_random[teamnum][c]), "%s", s);
		}
		botchat_random_count[teamnum] = c;
	}

	// Knightmare- free temp buffer
	if (f != NULL) {
		gi.TagFree (f);
		f = NULL;
	}
*/
}


void SP_info_team_start(edict_t *ent)
{
	int i, k;

	i = ent->obj_owner;

	// mapper set fullbright to 1 in info_team_start
	if (ent->groundentity_linkcount == true)
		level.fullbright = true;

	if (ent->teleport_time)
		level.fog = ent->teleport_time;

	// fix for mappers who use capital letters for teamid
	for (k = 0; ent->pathtarget[k]; k++)
		ent->pathtarget[k] = tolower(ent->pathtarget[k]);

	team_list[i] = gi.TagMalloc(sizeof(TeamS_t), TAG_LEVEL);
	team_list[i]->teamname = gi.TagMalloc(sizeof(ent->message + 2), TAG_LEVEL);
/*
	// Knightmare- use G_CopyString instead to avoid overflowing original buffer
//	strncpy (team_list[i]->teamname, ent->message);
	team_list[i]->teamname = G_CopyString (ent->message);
*/
	if (mashup->value)
	{
	/*	buggy pol rifle and probably other stuff
		char *team;
		int		r;
		r = (int)(random() *8);
		team = NULL;
		while (!team)
		{
			r = (int)(random() *8);
			switch (r)
			{
			case 1: team = "grm"; break;
			case 2: team = "rus"; break;
			case 3: team = "gbr"; break;
			case 4: team = "pol"; break;
			case 5: team = "ita"; break;
			case 6: team = "jpn"; break;
			case 7: team = "usm"; break;
			default: team = "usa"; break;
			}
			if (team_list[(i+1)%2] && !strcmp(team_list[(i+1)%2]->teamid, team))
				team = NULL;
		}
		// Knightmare- use G_CopyString instead to avoid overflowing original buffer
	//	strncpy (ent->pathtarget, team);
	//	strncpy (ent->message, team);
		ent->pathtarget = G_CopyString (team);
		ent->message = G_CopyString (team);
		*/
		int r;
		char *team;
		r = (int)(random() *5);
		if (i == 0)
		{
			switch (r)
			{
				r = (int)(random() *4);
				case 0: team = "rus"; break;
				case 1: team = "gbr"; break;
				case 2: team = "pol"; break;
				case 3: team = "usm"; break;
				default: team = "usa"; break;
			}
		}
		else
		{
			switch (r)
			{
				r = (int)(random() *2);
				case 0: team = "grm"; break;
				case 1: team = "ita"; break;
				default: team = "jpn"; break;
			}
		}
		// Knightmare- use G_CopyString instead to avoid overflowing original buffer
	//	strncpy (ent->pathtarget, team);
	//	strncpy (ent->message, team);
		ent->pathtarget = G_CopyString (team);
		ent->message = G_CopyString (team);
	}

	team_list[i]->teamname = ent->message;
	// long team names were causing crashes
	if (strlen(ent->message) > 12)
		ent->message[12] = 0;

//	if (strlen(ent->message) > 12)
//		team_list[i]->teamname = team_list[i]->teamid;
//	team_list[i]->playermodel = gi.TagMalloc( 64, TAG_LEVEL );
//	strncpy (team_list[i]->teamid, ent->pathtarget);
//	strncpy (team_list[i]->playermodel, ent->pathtarget);
	Q_strncpyz (team_list[i]->teamid, sizeof(team_list[i]->teamid), ent->pathtarget);
	Q_strncpyz (team_list[i]->playermodel, sizeof(team_list[i]->playermodel), ent->pathtarget);

	if (ent->style == 2)
		team_list[i]->kills_and_points = true;

	if (!Q_stricmp(team_list[i]->teamid, "usm"))
	{
	//	strncpy (team_list[i]->teamid, "usa");
	//	strncpy (team_list[i]->playermodel, "usa");
		Q_strncpyz (team_list[i]->teamid, sizeof(team_list[i]->teamid), "usa");
		Q_strncpyz (team_list[i]->playermodel, sizeof(team_list[i]->playermodel), "usa");
	}

	team_list[i]->nextmap = gi.TagMalloc( 64, TAG_LEVEL );

	team_list[i]->kills=0;
	team_list[i]->losses=0;
	team_list[i]->index=i;
	team_list[i]->score=0;

	if (ent->map)
		team_list[i]->nextmap = ent->map;
	else
		team_list[i]->nextmap = level.mapname;


	//make it so if allies win dday5 it goes to dday1
	if (!Q_stricmp(level.mapname, "dday5"))
	{
		team_list[0]->nextmap = "dday1";
	}

	// faf:  servers can turn off campaign mode
/*	if (campaign_mode->value != 1)
	{
		// sets it so the rotation always continues like the allied team won
		if (team_list[0] &&
			team_list[0]->nextmap &&
			team_list[1] &&
			team_list[1]->nextmap)
		{
			// faf: ubercrash here, doing all this code another way
			// Knightmare- use G_CopyString instead to avoid overflowing original buffer
		//	strncpy (team_list[1]->nextmap, team_list[0]->nextmap);
			team_list[1]->nextmap = G_CopyString (team_list[0]->nextmap);
		}

		// so we dont get stuck
	} */

	//faf: custom skins for map
	if (ent->deathtarget)
		team_list[i]->skin = ent->deathtarget;





	if (ent->count!=99 || Last_Team_Winner==99 )
	{
		if (ent->count==TEAM_OFFENCE) team_list[i]->mission=TEAM_OFFENCE;
		else if (ent->count==TEAM_DEFENCE) team_list[i]->mission=TEAM_DEFENCE;
		else team_list[i]->mission=TEAM_PATROL;
	}
	else
	{
		if (Last_Team_Winner==i) team_list[i]->mission=TEAM_OFFENCE;
		else team_list[i]->mission=TEAM_DEFENCE;
	}

//	if (ent->delay)
		team_list[i]->time_to_win = 0; //ent->delay;

	if (ent->health)
		team_list[i]->need_points = ent->health;
	else
		team_list[i]->need_points = 0;

	if (ent->dmg)
		team_list[i]->need_kills = ent->dmg;

	//set chute 1 to give all players on team parachute
	if (ent->obj_count)
		team_list[i]->chute = true;
	if (ent->delay)
		team_list[i]->delay = ent->delay;
	else
		team_list[i]->delay = 0;

	// now is the time to hook up the mos .dll files...

//    InitializeUserDLLs(LoadUserDLLs(ent, i),i);


	InitTeam(ent->pathtarget,i);

	if (campaign->string
		&& ent->obj_owner == 1)//so it doesn't get run twice
		SetupCampaign(false);

//	InitMOS_List(team_list[i]);


	LoadBotChat(i, team_list[i]->teamid);


	G_FreeEdict (ent);	//clean up entity now that it's not needed.

}



void SP_info_Max_MOS(edict_t *ent)
{
}
