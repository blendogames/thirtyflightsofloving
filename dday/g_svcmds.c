/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_svcmds.c,v $
 *   $Revision: 1.6 $
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






void Svcmd_Teamswitch_f (void)
{
	int cl_num, team;
	edict_t *ent;

	if (!strcmp (gi.argv(2), ""))
	{
		gi.dprintf("Usage: sv switch client#\n");
		return;
	}

	cl_num = atoi(gi.argv(2));

	ent = g_edicts + 1 + cl_num;
	if (!ent->inuse)
		return;

	if (!ent->client)
		return;

	if (ent->client->resp.team_on)
	{
		team = (ent->client->resp.team_on->index+1)%2;
	}
	else
		return;




	if (ent->client->resp.AlreadySpawned) // used choose_team cmd
	{
		if (ent->health ==100)
			T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, ent->health + 999, 0, DAMAGE_NO_PROTECTION, MOD_CHANGETEAM);
		else
			T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, ent->health + 999, 0, DAMAGE_NO_PROTECTION, MOD_CHANGETEAM_WOUNDED);
	}
	

	ent->client->resp.team_on=team_list[team];
	safe_bprintf(PRINT_HIGH, "%s has been switched to team %s.\n", ent->client->pers.netname, ent->client->resp.team_on->teamname);


	ent->client->resp.mos = NONE; // reset MOS

	ent->client->resp.mos = INFANTRY;
	ent->client->resp.changeteam = true;
	ent->client->forcespawn = level.time + .5;//faf: fixes standing corpse bug


	M_ChooseMOS(ent);

	return;



}









void Svcmd_Teams_f (void)
{
	int i;
	edict_t *e;

	if (!team_list[0] || !team_list[1])
		return;

	gi.dprintf("%s (%i)\n",		team_list[0]->teamid, PlayerCountForTeam(0, true));//faf team_list[0]->total);
	for (i=0 ; i< maxclients->value ; i++)
	{
		e = g_edicts + 1 + i;
		if (!e->client)
			continue;
		if (!e->inuse)
			continue;
		if (!e->client->resp.team_on)
			continue;
		if (e->client->resp.team_on->index != 0)
			continue;
		//gi.dprintf ("%i  %i  %s\n", e->client->resp.score, e->client->resp.plus_minus, e->client->pers.netname);
		gi.dprintf ("%i  %i  %s\n", e->client->resp.score, e->client->resp.stat_human_plus + e->client->resp.stat_bot_plus - e->client->resp.stat_human_minus - e->client->resp.stat_bot_minus, e->client->pers.netname);
	}

	gi.dprintf("%s (%i)\n", team_list[1]->teamid, PlayerCountForTeam(1, true));//faf team_list[1]->total);
	for (i=0 ; i< maxclients->value ; i++)
	{
		e = g_edicts + 1 + i;
		if (!e->client)
			continue;
		if (!e->inuse)
			continue;
		if (!e->client->resp.team_on)
			continue;
		if (e->client->resp.team_on->index != 1)
			continue;
		//gi.dprintf ("%i  %i  %s\n", e->client->resp.score, e->client->resp.plus_minus, e->client->pers.netname);
		gi.dprintf ("%i  %i  %s\n", e->client->resp.score, e->client->resp.stat_human_plus + e->client->resp.stat_bot_plus - e->client->resp.stat_human_minus - e->client->resp.stat_bot_minus, e->client->pers.netname);
	}
	gi.dprintf("Not on team:\n");
	for (i=0 ; i< maxclients->value ; i++)
	{
		e = g_edicts + 1 + i;
		if (!e->client)
			continue;
		if (!e->inuse)
			continue;
		if (e->client->resp.team_on)
			continue;

		gi.dprintf ("%s\n", e->client->pers.netname);
	}
}









void	Svcmd_Test_f (void)
{
	safe_cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

void Svcmd_Mapinfo_f (void)
{
	gi.dprintf("%s (%i) ---\n",		team_list[0]->teamname, PlayerCountForTeam(0, true));//faf team_list[0]->total);
	gi.dprintf("  Score: %i/%i\n",	team_list[0]->score, team_list[0]->need_points);
	gi.dprintf("  Kills: %i/%i\n",	team_list[0]->kills, team_list[0]->need_kills);
	gi.dprintf("%s (%i) -----\n", team_list[1]->teamname, PlayerCountForTeam(1, true));//faf team_list[1]->total);
	gi.dprintf("  Score: %i/%i\n",	team_list[1]->score, team_list[1]->need_points);
	gi.dprintf("  Kills: %i/%i\n",	team_list[1]->kills, team_list[1]->need_kills);
}


int	Get_Campaign_Spot_Number (char *mapname)
{
	int i;
	for (i = 0; campaign_spots[i].bspname; i++)
	{
		if (campaign_spots[i].bspname!= NULL && !strcmp (mapname, campaign_spots[i].bspname))
			return i;
	}
	return -1;
}

char *Get_Next_Campaign_Map ()
{
	int possible_maps[50];
	int i, j,k;
	int checkexit;
	int	checknum;
	qboolean add;
	int mapcount;
	int t,x,y;
	int randnum;
	int removed;

	for (j = 0; j<50; j++)
		possible_maps[j]=-1;

  	for (i = 0; campaign_spots[i].bspname; i++)
	{
		//go through all campaign_spots, find entrance points for this team not owned by them
		if ((Last_Team_Winner == 0 && campaign_spots[i].alliedstart == true && campaign_spots[i].owner == 1)||
			(Last_Team_Winner == 1 && campaign_spots[i].axisstart == true && campaign_spots[i].owner == 0))
		{
			add = true;
			for (j = 0; j<50; j++){
				if (possible_maps[j] == i)
					add = false;				
			}
			if (add){
				for (k = 0; k<50; k++){
					if (possible_maps[k]==-1){
						possible_maps[k] = i;
						break;
					}
				}
			}
		}
		//go through all campaign_spots, find ones owned by winning team
		if (campaign_spots[i].owner == Last_Team_Winner)
		{
			//gi.dprintf("%i\n",i);
			//go through their exits, find ones owned by other team, and add them to possible_maps
			checkexit = Get_Campaign_Spot_Number(campaign_spots[i].exita);
			if (checkexit > -1 && campaign_spots[checkexit].owner != Last_Team_Winner)
			{
				add = true;
				for (j = 0; j<50; j++){
					if (possible_maps[j] > -1 && possible_maps[j] == checkexit)
						add = false;				
				}
				if (add){
					for (k = 0; k<50; k++){
						if (checkexit > -1 && possible_maps[k]==-1){
							possible_maps[k] = checkexit;
							break;
						}
					}
				}
			}
			checkexit = Get_Campaign_Spot_Number(campaign_spots[i].exitb);
			if (checkexit > -1 && campaign_spots[checkexit].owner != Last_Team_Winner)
			{
				add = true;
				for (j = 0; j<50; j++)			{
					if (possible_maps[j] > -1 && possible_maps[j] == checkexit)
						add = false;				
				}
				if (add){
					for (k = 0; k<50; k++){
						if (checkexit > -1 && possible_maps[k]==-1){
							possible_maps[k] = checkexit;
							break;
						}
					}
				}
			}
			checkexit = Get_Campaign_Spot_Number(campaign_spots[i].exitc);
			if (checkexit > -1 && campaign_spots[checkexit].owner != Last_Team_Winner)
			{
				add = true;
				for (j = 0; j<50; j++)			{
					if (possible_maps[j] > -1 && possible_maps[j] == checkexit)
						add = false;				
				}
				if (add){
					for (k = 0; k<50; k++){
						if (checkexit > -1 && possible_maps[k]==-1){
							possible_maps[k] = checkexit;
							break;
						}
					}
				}
			}
		}
	}
	mapcount = 0;
	for (i = 0; i<50; i++)	{
		if (possible_maps[i] > -1)	{
			mapcount++;
			//gi.dprintf ("%s\n",campaign_spots[possible_maps[i]].bspname);
		}
	}

	removed = 0;
	//remove most recently played maps
	for (i = 0; i<20 && removed != mapcount-1; i++) //for last_maps_played
	{
		for (j=0; j < 50; j++)
		{
			if (possible_maps[j] < 0)
				continue;

			checknum = Get_Campaign_Spot_Number(last_maps_played[i]);
			//gi.dprintf ("%i  %s  %s   %s\n", i, last_maps_played[i], campaign_spots[checknum].bspname,campaign_spots[possible_maps[j]].bspname);			
			if (checknum == possible_maps[j])
			{	
				possible_maps[j]=-1;
				//gi.dprintf("EEEEEEEE Removing %s\n",campaign_spots[checknum].bspname);
				removed++;
				break;
			}
		}
	} 


	mapcount=0;
	//sort possible_maps
	for (x=0; x < 50-1; x++){
		for (y=0; y < 50-x-1; y++){
			if (possible_maps[y] < possible_maps[y+1])
			{
				t=possible_maps[y];
				possible_maps[y]=possible_maps[y+1];
				possible_maps[y+1]=t;
				if (t > 0)
					mapcount++;
			}
		}
	}
/*	for (j = 0; j<50; j++)	{
		if (possible_maps[j] > -1)	{
			gi.dprintf ("%s\n",campaign_spots[possible_maps[j]].bspname);
		}
	}	*/
	randnum = (int)(random()*mapcount);
	
	if (campaign_spots[possible_maps[randnum]].bspname != NULL)
		return campaign_spots[possible_maps[randnum]].bspname;
	
	return NULL;
}

//1 team just won map
void Update_Campaign_Info (void)
{
	int i;
	for (i = 0; campaign_spots[i].bspname; i++)
	{
		if (!strcmp (level.mapname,campaign_spots[i].bspname))
		{
			level.mapname,campaign_spots[i].owner=Last_Team_Winner;
			if (Last_Team_Winner == 0)
				axisplatoons--;
			else if (Last_Team_Winner == 1)
				alliedplatoons--;
			break;
		}
	}
}


void WriteCampaignTxt (void)
{
	FILE	*fp;
	char	campaignfilename[MAX_QPATH] = "";
	int		i;

	if (!campaign->string || !level.campaign)
		return;

//	Com_sprintf (campaignfilename, sizeof(campaignfilename), "dday/campaigns/%s.campaign", campaign->string);
	// Knightmare- use SavegameDir() instead for compatibility on all platforms
	Com_sprintf (campaignfilename, sizeof(campaignfilename), "%s/campaigns/%s.campaign", SavegameDir(), campaign->string);

	fp = fopen (campaignfilename, "wb");
	if (!fp)
		gi.error ("Couldn't open %s", campaignfilename);

	fprintf (fp, "%i\n", alliedplatoons);
	fprintf (fp, "%i\n", axisplatoons);
	fprintf (fp, "%i\n", alliedneedspots);
	fprintf (fp, "%i\n", axisneedspots);

	for (i = 0; campaign_spots[i].bspname; i++)
	{
		fprintf (fp, "%s\n", campaign_spots[i].bspname);
		fprintf (fp, "%s\n", campaign_spots[i].exita);
		fprintf (fp, "%s\n", campaign_spots[i].exitb);
		fprintf (fp, "%s\n", campaign_spots[i].exitc);
		fprintf (fp, "%i\n", campaign_spots[i].owner);
		fprintf (fp, "%i\n", campaign_spots[i].xpos);
		fprintf (fp, "%i\n", campaign_spots[i].ypos);
		fprintf (fp, "%i\n", campaign_spots[i].alliedstart);
		fprintf (fp, "%i\n", campaign_spots[i].axisstart);
	}

	fclose (fp);
}



//called when first creating campaign, or at map start.  Creates new [campaign].campaign file if needed or loads existing one
void SetupCampaign (qboolean restart)
{
	char		campaignfilename[MAX_QPATH] = "";
	char		*campinfo;
	int			i, c;
	char		*s, *f = NULL;
	char		*bspname = NULL;
	char		*exita = NULL, *exitb = NULL, *exitc = NULL;
	int			owner = 0;
	int			xpos = 0, ypos = 0;
	qboolean	alliedstart = false, axisstart = false;
	FILE		*check;

	if ( !*campaign->string )
		return;

//	Com_sprintf (campaignfilename, sizeof(campaignfilename), "dday/campaigns/%s.campaign", campaign->string);

	// Knightmare- use SavegameDir() / GameDir() instead for compatibility on all platforms
	Com_sprintf (campaignfilename, sizeof(campaignfilename), "%s/campaigns/%s.campaign", SavegameDir(), campaign->string);
	
	// convert string to all lowercase (for Linux)
	for (i = 0; campaignfilename[i]; i++)
		campaignfilename[i] = tolower(campaignfilename[i]);

	// fall back to GameDir() if not found in SavegameDir()
	check = fopen(campaignfilename, "r");
	if ( !check )
	{
		Com_sprintf (campaignfilename, sizeof(campaignfilename), "%s/campaigns/%s.campaign", GameDir(), campaign->string);

		// convert string to all lowercase (for Linux)
		for (i = 0; campaignfilename[i]; i++)
			campaignfilename[i] = tolower(campaignfilename[i]);
	}
	else {
		fclose (check);
	}
	// end Knightmare

	if ( (restart == true) || !(check = fopen(campaignfilename, "r")) )	// no current campaign
	{
	//	Com_sprintf (campaignfilename, sizeof(campaignfilename), "dday/campaigns/%s.cpgntemplate", campaign->string);
		// Knightmare- use GameDir() instead for compatibility on all platforms
		Com_sprintf (campaignfilename, sizeof(campaignfilename), "%s/campaigns/%s.cpgntemplate", GameDir(), campaign->string);
	}

	campinfo = ReadEntFile(campaignfilename);

	if (campinfo)
	{   //leave these dprints active they show up in the server init console section
		gi.dprintf("%s.cpgntemplate Loaded\n", campaign->string);

		level.campaign = campaign->string;

		c = 0;
	//	f = strdup (campinfo);
		f = G_CopyString (campinfo);	// Knightmare- use G_CopyString insteads
		s = strtok(f, "\n");

		if (s != NULL) {
			alliedplatoons = atoi (s);
			//gi.dprintf ("alliedplatoons: %s\n",s);
			s = strtok (NULL, "\n");
		}

		if (s != NULL) {
			axisplatoons = atoi (s);
			//gi.dprintf ("axisplatoons: %s\n",s);
			s = strtok (NULL, "\n");
		}
		if (s != NULL) {
			alliedneedspots = atoi (s);
			//gi.dprintf ("alliedneedspots: %s\n",s);
			s = strtok (NULL, "\n");
		}

		if (s != NULL) {
			axisneedspots = atoi (s);
			//gi.dprintf ("axisneedspots: %s\n",s);
			s = strtok (NULL, "\n");
		}
		while (s !=NULL)
		{
			if (s != NULL) {
				bspname = s;
				//gi.dprintf ("bsp: %s\n",s);
				s = strtok (NULL, "\n");
			}
			if (s != NULL) {
				exita = s;
				//gi.dprintf ("exita: %s\n",exita);
				s = strtok (NULL, "\n");
			}
			if (s != NULL) {
				exitb = s;
				//gi.dprintf ("exitb: %s\n",exitb);
				s = strtok (NULL, "\n");
			}
			if (s != NULL) {
				exitc = s;
				//gi.dprintf ("exitc: %s\n",exitc);
				s = strtok (NULL, "\n");
			}
			if (s != NULL) {
				owner = atoi (s);
				//gi.dprintf ("owner: %i\n",owner);
				s = strtok (NULL, "\n");
			}
			if (s != NULL) {
				xpos = atoi (s);
				//gi.dprintf ("xpos: %i\n",xpos);
				s = strtok (NULL, "\n");
			}
			if (s != NULL) {
				ypos = atoi (s);
				//gi.dprintf ("ypos: %i\n",ypos);
				s = strtok (NULL, "\n");
			}
			if (s != NULL) {
				alliedstart = atoi (s);
				//gi.dprintf ("alliedstart: %i\n",alliedstart);
				s = strtok (NULL, "\n");
			}
			if (s != NULL) {
				axisstart = atoi (s);
				//gi.dprintf ("axisstart: %i\n",axisstart);
				s = strtok (NULL, "\n");



				campaign_spots[c].bspname = bspname;
				campaign_spots[c].exita = exita;
				campaign_spots[c].exitb = exitb;
				campaign_spots[c].exitc = exitc;
				campaign_spots[c].owner = owner;
				campaign_spots[c].xpos = xpos;
				campaign_spots[c].ypos = ypos;
				campaign_spots[c].alliedstart = alliedstart;
				campaign_spots[c].axisstart = axisstart;
				c++;
			}

		}

	}
	else
	{
		gi.dprintf ("%s not found.\n",campaignfilename);
		gi.cvar_set("campaign", "");
		return;
	}
	WriteCampaignTxt ();

	// Knightmare- free temp buffer
	if (f != NULL) {
		gi.TagFree (f);
		f = NULL;
	}
}














/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void	ServerCommand (void)
{
	char	*cmd;

	// JABot[start]
	if (BOT_ServerCommand ())
		return;
	// [end]

	cmd = gi.argv(1);

	if (Q_stricmp (cmd, "test") == 0)
		Svcmd_Test_f ();
	else if (Q_stricmp (cmd, "maplist") == 0) 
		Svcmd_Maplist_f (); 
	else if (Q_stricmp (cmd, "mapinfo") == 0)
		Svcmd_Mapinfo_f ();
	else if (Q_stricmp (cmd, "teams") ==0)
		Svcmd_Teams_f();
	else if (Q_stricmp (cmd, "switch") ==0)
		Svcmd_Teamswitch_f();
	else
		safe_cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

