/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/p_hud.c,v $
 *   $Revision: 1.27 $
 *   $Date: 2002/07/23 23:00:48 $
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


/*
======================================================================

INTERMISSION

======================================================================
*/
void SV_CalcViewOffset (edict_t *ent);

void MoveClientToIntermission (edict_t *ent)
{
	if (deathmatch->value || coop->value)
	{ 	// pbowens: turned on later
		ent->client->layout_type = SHOW_NONE;
	}

	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
#ifdef KMQUAKE2_ENGINE_MOD
	ent->client->ps.gunindex2 = 0;
#endif
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;
/*-----/ PM /-----/ NEW:  Kill any flames /-----*/
	ent->burnout = 0;
/*----------------------------------------------*/

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	//ent->client->grenade_blew_up = false;
	//ent->client->grenade_time = 0;
	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex4 = 0;
#ifdef KMQUAKE2_ENGINE_MOD
	ent->s.modelindex5 = 0;
	ent->s.modelindex6 = 0;
#endif
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	
	ent->viewheight = 20;//faf
	ent->stance_view = 20;//faf
	ent->stanceflags = STANCE_STAND;//faf
	
	if (!ent->ai)
		SV_CalcViewOffset (ent);//faf

	// pbowens: make sure limbo_mode is set
	ent->client->limbo_mode = true;

	// add the layout

	if (deathmatch->value || coop->value)
	{
//		DeathmatchScoreboardMessage (ent, NULL);




		// pbowens: victory song
		if (!level.victory_song_played)//faf:  so it's only played once, keeps it from being blasted
		{
			//faf:  shut off all target_speakers on the map so victory song can play alone
			edict_t *t;

			t = NULL;
			
			while ((t = G_Find (t, FOFS(classname), "target_speaker")))
			{
				t->s.sound = 0;	
			}

			if (level.campaign && campaign_winner == 0)
			{
				gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex(va("campaigns/%s-allies.wav", level.campaign)), 1, ATTN_NONE, 0);
				gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex(va("campaigns/%s-allies.wav", level.campaign)), 1, ATTN_NONE, 0);
				gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex(va("campaigns/%s-allies.wav", level.campaign)), 1, ATTN_NONE, 0);
			}
			else if (level.campaign && campaign_winner == 1)
			{
				gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex(va("campaigns/%s-axis.wav", level.campaign)), 1, ATTN_NONE, 0);
				gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex(va("campaigns/%s-axis.wav", level.campaign)), 1, ATTN_NONE, 0);
				gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex(va("campaigns/%s-axis.wav", level.campaign)), 1, ATTN_NONE, 0);
			}

			//faf:  tie game music
			else if (Last_Team_Winner == -1)
			{
				if (random() < 0.33)
                    gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex("faf/maria.wav"), 1, ATTN_NONE, 0);
				else if (random() < 0.33)
                    gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex("inland/ulla.wav"), 1, ATTN_NONE, 0);
				else 
                    gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex("julhelm/dmusic2.wav"), 1, ATTN_NONE, 0);
			}
			else if (Last_Team_Winner != 99)
			{
				//faf:  playing this 2 times so it's loud enough.  It would be better to edit the wav file of course
				gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex(va("%s/victory.wav", team_list[Last_Team_Winner]->teamid)), 1, ATTN_NONE, 0);
				gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex(va("%s/victory.wav", team_list[Last_Team_Winner]->teamid)), 1, ATTN_NONE, 0);
				//gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex(va("%s/victory.wav", team_list[Last_Team_Winner]->teamid)), 1, ATTN_NONE, 0);
			}

			level.victory_song_played = true;
		}
	
	}

}

void BeginIntermission (edict_t *targ)
{
	int		i, n;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

	game.autosaved = false;

	// respawn any dead clients
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		if (client->health <= 0)
			respawn(client);
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;





	if (stats->value)
	{
		for (i=0 ; i<maxclients->value ; i++)
		{
			ent = g_edicts + 1 + i;

			if (ent->client && ent->inuse & !ent->ai)
			{
				Write_Player_Stats(ent);
			}
		}
	}








	if (!level.changemap)
		 level.changemap = level.mapname;

	if (strstr(level.changemap, "*"))
	{
		if (coop->value)
		{
			for (i=0 ; i<maxclients->value ; i++)
			{
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				// strip players of all keys between units
				for (n = 0; n < MAX_ITEMS; n++)
				{
					if (itemlist[n].flags & IT_KEY)
						client->client->pers.inventory[n] = 0;
				}
			}
		}
	}
	else
	{
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}

	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (ent && ent->obj_owner)
	{
		if (ent->obj_owner != Last_Team_Winner)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (ent && ent->obj_owner && ent->obj_owner != Last_Team_Winner)
				ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
		}

	}
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	/*else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}*/

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);


	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}
}

/*
===================================================================================\
Death Match Scoreboard players
===================================================================================
*/
/*
void DeathmatchPlayerScore (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;

	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;

	// sort the clients by score
	total = 0;
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
	
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// print level name and exit rules
	string[0] = 0;

	stringlength = strlen(string);

	// add the clients in sorted order
	if (total > 12)
		total = 12;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		picnum = gi.imageindex ("i_fixme");
		x = (i>=6) ? 160 : 0;
		y = 32 + 32 * (i%6);

		// add a dogtag
		if (cl_ent == ent)
			tag = "tag1";
		else if (cl_ent == killer)
			tag = "tag2";
		else
			tag = NULL;
		if (tag)
		{
			Com_sprintf (entry, sizeof(entry),
				"xv %i yv %i picn %s ",x+32, y, tag);
			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
		//	strncpy (string + stringlength, entry);
			Q_strncpyz (string + stringlength, sizeof(string) - stringlength, entry);
			stringlength += j;
		}

		// send the layout
		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i ",
			//"picn 24by24 %i %i %i %i %i %i ",
			x, y, sorted[i], cl->ping, (level.framenum - cl->resp.enterframe)/600);
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
	//	strncpy (string + stringlength, entry);
		Q_strncpyz (string + stringlength, sizeof(string) - stringlength, entry);
		stringlength += j;
	 }
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}
*/
/*
==================
DeathmatchScoreboardMessage

==================
*/
/*
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	victory_type[10];
	char	victory_adj[15];
	int		points, enemy_total, team_total;
	char	killstr[15],pointstr[15],lossestr[15];
	
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j;
	int		x, y;

	float	kill_p[MAX_TEAMS],	point_p[MAX_TEAMS];  // The kill and point ratios of each team
	float	kill_h,				point_h;			 // The highest of the two 
	int		kill_win,			point_win;			 // The owner of the highest ratio

	// print level name and exit rules
	string[0] = 0;
	stringlength = strlen(string);

	for(i=0;i<MAX_TEAMS;i++)
	{
		if (!team_list[i])
			continue;

		enemy_total = 1;

		// find out the enemy's stats
		for(j=0;j<MAX_TEAMS;j++)
		{
			if(j==i) 
				continue;
			if(!team_list[j]) 
				continue;
			enemy_total += team_list[j]->total;

			// Get the enemy's KILL and POINT ratios
			if (team_list[j]->need_kills)
				kill_p[j] = team_list[j]->kills / team_list[j]->need_kills;
			if (team_list[j]->need_points)
				point_p[j] = team_list[j]->score / team_list[j]->need_points;
		}
	
		team_total= (team_list[i]->total > 1) ? team_list[i]->total : 1;
		points = team_list[i]->score;

		// Get my team's KILL and POINT ratios
		if (team_list[i]->need_kills)
			kill_p[i] = team_list[i]->kills / team_list[i]->need_kills;
		if (team_list[i]->need_points)
			point_p[i] = team_list[i]->score / team_list[i]->need_points;

		// Determine which ratio is the highest of KILLs
		if (kill_p[i] > kill_p[j]) {
			kill_win	= i;
			kill_h		= kill_p[i];
		} else {
			kill_win	= j;
			kill_h		= kill_p[j];
		}

		// and POINTs
		if (point_p[i] > point_p[j]) {
			point_win	= i;
			point_h		= point_p[i];
		} else {
			point_win	= j;
			point_h		= point_p[j];
		}

		// FINALLY determine who is winning
		if (kill_win == i && point_win == i)
		{
			Q_strncpyz (victory_adj, sizeof(victory_adj), "Decisive");
			Q_strncpyz (victory_type, sizeof(victory_type), "Victory");
		}
		else if (point_h > kill_h)
		{
			if (point_win == i)
			{
				// Victory Conditions
			}
			else
			{
				// Loss Conditions
			}

		}
		else if (kill_h > point_h)
		{
			if (kill_win == i)
			{
				// Victory Conditions
			}
			else
			{
				// Loss Conditions
			}
		}
		else
		{
			// Stalemate
		}


		//points=team_list[i]->score + (team_list[i]->kills/enemy_total) / ( (team_list[i]->losses) ? team_list[i]->losses:1) / ( (team_list[i]->total) ? team_list[i]->total:1);

		// it limits the losses and totals to a bottom of 1, it means this:
		//points=team_list[i]->score + (team_list[i]->kills/enemy_total) / ( (if team_list[i]->losses is true, use team_list[i]->losses, otherwise 1) / (if team_list[i]->total is true, use team_list[i]->total, otherwise 1)

		//add stuff for victory conditions here...

		if (Last_Team_Winner == 99 ||
			level.intermissiontime != level.time)
		{
			Q_strncpyz (victory_adj, sizeof(victory_adj), "Currently");
			Q_strncpyz (victory_type, "in Battle");

		}
		else if (i == Last_Team_Winner)		
		{
			if ((team_list[i]->need_kills > 0) && team_list[i]->kills >= team_list[i]->need_kills) 
				Q_strncpyz (victory_adj, sizeof(victory_adj), "Forceful");
			else if ((team_list[i]->need_points > 0) && team_list[i]->score >= team_list[i]->need_points) 
				Q_strncpyz (victory_adj, sizeof(victory_adj), "Strategical");
			else
				Q_strncpyz (victory_adj, sizeof(victory_adj), "Decisive");

			Q_strncpyz (victory_type, sizeof(victory_type), "Victory");
		}
		else
		{
			Q_strncpyz (victory_adj, sizeof(victory_adj), "Definate");
			Q_strncpyz (victory_type, sizeof(victory_type), "Loss");
		}

		x = ( ((i+1)%2) ) ? 0 : 110;
		y = ( i>2 ) ?  64:32;

		Com_sprintf (killstr, sizeof(killstr),  "Kills:  %i",team_list[i]->kills);
		Com_sprintf (lossestr, sizeof(lossestr),"Losses: %i",team_list[i]->losses);
		Com_sprintf (pointstr, sizeof(pointstr),"Points: %i",points);
	
		
	//										x     y     teamname    x    y        kills       x     y        losses      x     y        points      x      y          vict      x    y      vict-type     x     y    
		Com_sprintf (entry,sizeof(entry),"xv %i yv %i string \"%s\" xv %i yv %i string2 \"%s\" xv %i yv %i string2 \"%s\" xv %i yv %i string2 \"%s\" xv %i yv %i string \"%s\" xv %i yv %i string \"%s\" ",
										 x     ,y ,team_list[i]->teamname,
																  x+15,  y+10,  killstr,   x+15,  y+20,    lossestr,  x+15,  y+30,   pointstr,     x+5, y+45, victory_adj,x+5,y+55,victory_type);	
		j = strlen(entry);
		if (stringlength + j > 1024) break;
	//	strncpy (string + stringlength, entry);
		Q_strncpyz (string + stringlength, sizeof(string) - stringlength, entry);
		stringlength += j;
	}

		////////////////////////////!!
	return; // DISABLED //      !!
//////////////////////////      !!


	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}
*/

/*
=============
TeamStats
=============
*/
void TeamStats (edict_t *ent)
{
	int i;

	for (i=0 ; i<MAX_TEAMS ; i++)
	{
		int STAT_ICON, STAT_KILLS, STAT_POINTS;

		if (i == 0)
		{
			STAT_ICON	= STAT_TEAM0_ICON;
			STAT_KILLS	= STAT_TEAM0_KILLS;
			STAT_POINTS	= STAT_TEAM0_POINTS;
		}
		else if (i == 1)
		{
			STAT_ICON	= STAT_TEAM1_ICON;
			STAT_KILLS	= STAT_TEAM1_KILLS;
			STAT_POINTS	= STAT_TEAM1_POINTS;
		}
		else
			return; // don't crash the server

		if (team_list[i])	{
			ent->client->ps.stats[STAT_ICON] = gi.imageindex(va("teams/%s", team_list[i]->teamid));

			ent->client->ps.stats[STAT_KILLS]	= team_list[i]->kills;
			ent->client->ps.stats[STAT_POINTS]	= team_list[i]->score;
			
			//winning team icon blinks
			if (level.intermissiontime && (Last_Team_Winner == i || Last_Team_Winner == -1) &&
				level.framenum%10 > 4)
				ent->client->ps.stats[STAT_ICON] = 0;

			//blink after choosing team
			else if (ent->client->resp.changeteam == true &&
				ent->client->resp.team_on &&
				ent->client->resp.team_on->index == i &&
				level.framenum%10 > 4)
				ent->client->ps.stats[STAT_ICON] = 0;

		}
		else
		{
			ent->client->ps.stats[STAT_ICON]	= 0;
			ent->client->ps.stats[STAT_KILLS]	= 0;
			ent->client->ps.stats[STAT_POINTS]	= 0;
		}
	}
}

//faf:  scoreboard from aq2
// Maximum number of lines of scores to put under each team's header.
#define MAX_SCORES_PER_TEAM 9
#define TEAM_TOP        (MAX_TEAMS+1)
#define TEAM1    0
#define TEAM2    1
        
void A_ScoreboardMessage (edict_t *ent)//, edict_t *killer)
{
	char scoreleftpic[256];   //faf:  for team dll support
    char scorerightpic[256];  //      loads background pic according to team
	char scoretopleftpic[256];
    char scoretoprightpic[256];  //faf: end

        char        string2[1400], string[1400];//, damage[50];
        gclient_t   *cl;
        edict_t     *cl_ent;
        int         maxsize = 1000, i, j, k;

                int team, len;//, deadview;
                int sorted[TEAM_TOP][MAX_CLIENTS];
                int sortedscores[TEAM_TOP][MAX_CLIENTS];
                int score, total[TEAM_TOP], totalscore[TEAM_TOP];
                int totalalive[TEAM_TOP], totalaliveprinted[TEAM_TOP];
                int stoppedat[TEAM_TOP];
	//	char	pingstring[3];
		char	pingstring[4];

	//JABot[start]
	if (ent->ai || !ent->inuse)
		return;
	//[end]



                total[TEAM1] = total[TEAM2] = totalalive[TEAM1] = totalalive[TEAM2] = 
                        totalscore[TEAM1] = totalscore[TEAM2] = 0;

                for (i=0 ; i<game.maxclients ; i++)
                {
                        cl_ent = g_edicts + 1 + i;
                        if (!cl_ent->inuse)
                                continue;
        
                        if (!game.clients[i].resp.team_on)
                            continue;
                        else
                            team = game.clients[i].resp.team_on->index;

       
                        score = game.clients[i].resp.score;
                        if (player_scores->value == 0)
                        {
                               j = total[team];
                        }
                        else 
                        {
                            for (j = 0; j < total[team]; j++)
                            {
                                 if (score > sortedscores[team][j])
                                        break;
                            }
                            for (k=total[team] ; k>j ; k--)
                            {
                                  sorted[team][k] = sorted[team][k-1];
                                  sortedscores[team][k] = sortedscores[team][k-1];
                            }
                        }
                        sorted[team][j] = i;
                        sortedscores[team][j] = score;
                        totalscore[team] += score;
                        total[team]++;
                        if (cl_ent->solid != SOLID_NOT &&
                                cl_ent->deadflag != DEAD_DEAD)
                                totalalive[team]++;
                } 
        
        if (!strcmp(team_list[0]->teamid, "usa") && !strcmp(team_list[1]->teamid, "grm"))
        {
			Com_sprintf (string, sizeof(string),
						"xv 0  yv   0 picn scorehead  " // background header
						"yv 80 xv   0 picn scoreleft  " // background left list
						"      xv 160 picn scoreright " // background right list
						);
        }
		else
			// if teams are anything else or grm|usa, display the split graphics:
		{
			Q_strncpyz (scoreleftpic, sizeof(scoreleftpic), "yv 80 xv   0 picn ");
			Q_strncatz (scoreleftpic, sizeof(scoreleftpic), team_list[0]->teamid);
			Q_strncatz (scoreleftpic, sizeof(scoreleftpic), "_score  ");

			Q_strncpyz (scorerightpic, sizeof(scorerightpic), "      xv 160 picn ");
			Q_strncatz (scorerightpic, sizeof(scorerightpic), team_list[1]->teamid);
			Q_strncatz (scorerightpic, sizeof(scorerightpic), "_score  ");

			Q_strncpyz (scoretopleftpic, sizeof(scoretopleftpic), "xv 0  yv   0 picn ");
			Q_strncatz (scoretopleftpic, sizeof(scoretopleftpic), team_list[0]->teamid);
			Q_strncatz (scoretopleftpic, sizeof(scoretopleftpic), "_score_top  ");

			Q_strncpyz (scoretoprightpic, sizeof(scoretoprightpic), "xv 0  xv 160 picn ");
			Q_strncatz (scoretoprightpic, sizeof(scoretoprightpic), team_list[1]->teamid);
			Q_strncatz (scoretoprightpic, sizeof(scoretoprightpic), "_score_top  ");

			Com_sprintf (string, sizeof(string), scoretopleftpic); //team 0
			Q_strncatz (string, sizeof(string), scoretoprightpic); // team 1
			Q_strncatz (string, sizeof(string), scoreleftpic); //background left list pic
			Q_strncatz (string, sizeof(string), scorerightpic); // background right list pic
		}
		if (team_list[0]->kills_and_points)
			Q_strncatz (string, sizeof(string), "xv 90 yv 20 string \"&\" ");

		if (team_list[1]->kills_and_points)
			Q_strncatz (string, sizeof(string), "xv 245 yv 20 string \"&\" ");


            Com_sprintf (string2, sizeof(string2), 
                    // TEAM1
                    "xv 37 yv 38 string \"%4d/%-3d\" "
                    "xv 97 yv 38 string \"%4d/%-3d\" "
                    // TEAM2
                    "xv 191 yv 38 string \"%4d/%-3d\" "
                    "xv 250 yv 38 string \"%4d/%-3d\" ",
				//	50,60,200,200,50,60,200,200);//test
					team_list[TEAM1]->kills, team_list[TEAM1]->need_kills, team_list[TEAM1]->score, team_list[TEAM1]->need_points,// name_pos[TEAM1], team_list[TEAM1]->teamname,
					team_list[TEAM2]->kills, team_list[TEAM2]->need_kills, team_list[TEAM2]->score, team_list[TEAM2]->need_points);//, name_pos[TEAM2] + 160, team_list[TEAM2]->teamname);
  
			Q_strncatz (string, sizeof(string), string2);


			if (player_scores->value)
			{
				Q_strncatz (string, sizeof(string),		"xv 4   yv 67 string  \"Ping  Player  Score\" ");//faf
				Q_strncatz (string, sizeof(string),		"xv 164   yv 67 string  \"Ping  Player  Score\" ");//faf

			}
			else
			{
				Q_strncatz (string, sizeof(string),		"xv 4   yv 67 string  \"Ping  Player\" ");//faf
				Q_strncatz (string, sizeof(string),		"xv 164   yv 67 string  \"Ping  Player\" ");//faf
			}

		//	Q_strncatz (string, sizeof(string),		"xv 0   yv 67 string  \" Ping Player         Ping Player\" "

			// pbowens: team victory pix
			if (level.intermissiontime && Last_Team_Winner != 99 &&
				Last_Team_Winner != -1) 
			{
				if (campaign_winner == 0){
					Q_strncatz (string, sizeof(string), va("xv 0 yv -80 picn allies_campaign "));
				}
				else if (campaign_winner == 1){
					Q_strncatz (string, sizeof(string), va("xv 0 yv -80 picn axis_campaign "));
				}
				else{
					Q_strncatz (string, sizeof(string), va("xv 0 yv -80 picn victory_%s ", 
						team_list[Last_Team_Winner]->teamid ));
				}
			}


                len = (int)strlen(string);

                totalaliveprinted[TEAM1] = totalaliveprinted[TEAM2] = 0;
                stoppedat[TEAM1] = stoppedat[TEAM2] = -1;
        
                for (i=0 ; i < (MAX_SCORES_PER_TEAM + 1) ; i++)
                {
                        if (i >= total[TEAM1] && i >= total[TEAM2])
                                break; 

        
                        // ok, if we're approaching the "maxsize", then let's stop printing members of each
                        // teams (if there's more than one member left to print in that team...)
                        if (len > (maxsize - 100))
                        {
                                if (i < (total[TEAM1] - 1))
                                        stoppedat[TEAM1] = i;
                                if (i < (total[TEAM2] - 1))
                                        stoppedat[TEAM2] = i;
                        }
                        if (i == MAX_SCORES_PER_TEAM-1) 
                        {
                                if (total[TEAM1] > MAX_SCORES_PER_TEAM)
                                        stoppedat[TEAM1] = i;
                                if (total[TEAM2] > MAX_SCORES_PER_TEAM)
                                        stoppedat[TEAM2] = i;
                        }
        
                        if (i < total[TEAM1] && stoppedat[TEAM1] == -1)  // print next team 1 member...
                        {
                                cl = &game.clients[sorted[TEAM1][i]];
                                cl_ent = g_edicts + 1 + sorted[TEAM1][i];
                                if (cl_ent->solid != SOLID_NOT &&
                                        cl_ent->deadflag != DEAD_DEAD)
                                        totalaliveprinted[TEAM1]++;
        
								if (cl_ent->ai)
									Com_sprintf (pingstring, sizeof(pingstring), "BOT");
								else if (game.clients[sorted[TEAM1][i]].ping<10)
									Com_sprintf (pingstring, sizeof(pingstring), "  %i", game.clients[sorted[TEAM1][i]].ping);
								else if (game.clients[sorted[TEAM1][i]].ping<100)
									Com_sprintf (pingstring, sizeof(pingstring), " %i", game.clients[sorted[TEAM1][i]].ping);
								else if (game.clients[sorted[TEAM1][i]].ping<1000)
									Com_sprintf (pingstring, sizeof(pingstring), "%i", game.clients[sorted[TEAM1][i]].ping);
								else
									Com_sprintf (pingstring, sizeof(pingstring), "999");


								if (player_scores->value)
								{
								//	sprintf (string + strlen(string), 
									Com_sprintf (string + strlen(string), sizeof(string) - strlen(string), 
											"xv 3 yv %d string \"%s%-12.12s%3d\"",
											87 + i * 14,//8, 
										//	game.clients[sorted[TEAM1][i]].ping,
											pingstring,
											va("%s%s",(game.clients[sorted[TEAM1][i]].resp.mos == MEDIC) ? "+" : " ", game.clients[sorted[TEAM1][i]].pers.netname),
										//	game.clients[sorted[TEAM1][i]].pers.netname,
                                        game.clients[sorted[TEAM1][i]].resp.score);//, damage);   
								}
								else
								{
								//	sprintf (string + strlen(string), 
									Com_sprintf (string + strlen(string), sizeof(string) - strlen(string), 
											"xv 3 yv %d string \"%3d%-12.12s\"",
											87 + i * 14,//8, 
										//	game.clients[sorted[TEAM1][i]].ping,
											(game.clients[sorted[TEAM1][i]].ping > 999) ? 999 : game.clients[sorted[TEAM1][i]].ping,
											va("%s%s",(game.clients[sorted[TEAM1][i]].resp.mos == MEDIC) ? "+" : " ", game.clients[sorted[TEAM1][i]].pers.netname)
										//	game.clients[sorted[TEAM1][i]].pers.netname
										);	//, damage);   
								}

                        }
        
                        if (i < total[TEAM2] && stoppedat[TEAM2] == -1)  // print next team 2 member...
                        {
                                cl = &game.clients[sorted[TEAM2][i]];
                                cl_ent = g_edicts + 1 + sorted[TEAM2][i];
                                if (cl_ent->solid != SOLID_NOT &&
                                        cl_ent->deadflag != DEAD_DEAD)
                                        totalaliveprinted[TEAM2]++;
    	
								if (cl_ent->ai)
									Com_sprintf (pingstring, sizeof(pingstring), "BOT");
								else if (game.clients[sorted[TEAM2][i]].ping<10)
									Com_sprintf (pingstring, sizeof(pingstring), "  %i", game.clients[sorted[TEAM2][i]].ping);
								else if (game.clients[sorted[TEAM2][i]].ping<100)
									Com_sprintf (pingstring, sizeof(pingstring), " %i", game.clients[sorted[TEAM2][i]].ping);
								else if (game.clients[sorted[TEAM2][i]].ping<1000)
									Com_sprintf (pingstring, sizeof(pingstring), "%i", game.clients[sorted[TEAM2][i]].ping);
								else
									Com_sprintf (pingstring, sizeof(pingstring), "999");

    
                                // AQ truncates names at 12, not sure why, except maybe to conserve scoreboard 
                                // string space?  skipping that "feature".  -FB
        
							//	Com_sprintf (string+strlen(string), sizeof(string) - strlen(string), 
							//			"xv 175 yv %d string%s \"%s\" ",  
							//			87 + i * 8,//faf 12,
							//			"",//deadview ? (cl_ent->solid == SOLID_NOT ? "" : "2") : "",
							//			game.clients[sorted[TEAM2][i]].pers.netname);
							if (player_scores->value)
							{
							//	sprintf (string + strlen(string), 
								Com_sprintf (string + strlen(string), sizeof(string) - strlen(string), 
									"xv 165 yv %d string \"%s%-12.12s%3d\"",
									87 + i * 14,//8, 
								//	game.clients[sorted[TEAM2][i]].ping,
									pingstring,	
									va("%s%s",(game.clients[sorted[TEAM2][i]].resp.mos == MEDIC) ? "+" : " ", game.clients[sorted[TEAM2][i]].pers.netname),

     //                                   game.clients[sorted[TEAM2][i]].pers.netname,
                                        game.clients[sorted[TEAM2][i]].resp.score);//, damage);   
							}
							else
							{
							//	sprintf (string + strlen(string), 
								Com_sprintf (string + strlen(string), sizeof(string) - strlen(string), 
									"xv 165 yv %d string \"%3d%-12.12s\"",
									87 + i * 14,//8, 
									(game.clients[sorted[TEAM2][i]].ping > 999) ? 999 : game.clients[sorted[TEAM2][i]].ping,
								//	game.clients[sorted[TEAM2][i]].ping,
									va("%s%s",(game.clients[sorted[TEAM2][i]].resp.mos == MEDIC) ? "+" : " ", game.clients[sorted[TEAM2][i]].pers.netname)

								//	game.clients[sorted[TEAM2][i]].pers.netname
									);	//, damage);   
							}

                        }
        
                        len = (int)strlen(string);
                }
        
				// Print remaining players if we ran out of room...
				/*	if (!deadview) // live player viewing scoreboard...
				{
					if (stoppedat[TEAM1] > -1)
					{
					//	sprintf (string + strlen(string),
						Com_sprintf (string + strlen(string), sizeof(string) - strlen(string),
								"xv 0 yv %d string \"..and %d more\" ",
								42 + (stoppedat[TEAM1] * 8), total[TEAM1] - stoppedat[TEAM1]);
					}
					if (stoppedat[TEAM2] > -1)
					{
					//	sprintf (string + strlen(string),
						Com_sprintf (string + strlen(string), sizeof(string) - strlen(string),
								"xv 160 yv %d string \"..and %d more\" ",
								42 + (stoppedat[TEAM2] * 8), total[TEAM2] - stoppedat[TEAM2]);
					}
				}
				else // dead player viewing scoreboard...
				{*/
					if (stoppedat[TEAM1] > -1)
					{
					//	sprintf (string + strlen(string),
						Com_sprintf (string + strlen(string), sizeof(string) - strlen(string),
								"xv 0 yv 200 /*160*/ string \" and %d more\" ",
								total[TEAM1] - stoppedat[TEAM1]);
					}
					if (stoppedat[TEAM2] > -1)
					{
					//	sprintf (string + strlen(string),
						Com_sprintf (string + strlen(string), sizeof(string) - strlen(string),
								"xv 160 yv 200 /*160*/ string \" and %d more\" ",
								total[TEAM2] - stoppedat[TEAM2]);
					}
			//	}
        


        if (strlen(string) > 1300)  // for debugging...
                gi.dprintf("Warning: scoreboard string neared or exceeded max length\nDump:\n%s\n---\n", 
                                string);

        gi.WriteByte (svc_layout);
        gi.WriteString (string);
}

//faf: new secondary scoreboard to show more statistics
void A_ScoreboardMessage2 (edict_t *ent)//, edict_t *killer)
{
	float accuracy;
	char scoreleftpic[256];   //faf:  for team dll support
    char scorerightpic[256];  //      loads background pic according to team
	char scoretopleftpic[256];
    char scoretoprightpic[256];  //faf: end

        char        string2[1400], string[1400];//, damage[50];
        gclient_t   *cl;
        edict_t     *cl_ent;
        int         maxsize = 1000, i, j, k;

                int team, len;//, deadview;
                int sorted[TEAM_TOP][MAX_CLIENTS];
                int sortedscores[TEAM_TOP][MAX_CLIENTS];
                int score, total[TEAM_TOP], totalscore[TEAM_TOP];
                int totalalive[TEAM_TOP], totalaliveprinted[TEAM_TOP];
                int stoppedat[TEAM_TOP];

				int total_deaths;


	//JABot[start]
	if (ent->ai || !ent->inuse)
		return;
	//[end]

                total[TEAM1] = total[TEAM2] = totalalive[TEAM1] = totalalive[TEAM2] = 
                        totalscore[TEAM1] = totalscore[TEAM2] = 0;

                for (i=0 ; i<game.maxclients ; i++)
                {
                        cl_ent = g_edicts + 1 + i;
                        if (!cl_ent->inuse)
                                continue;
        
                        if (!game.clients[i].resp.team_on)
                            continue;
                        else
                            team = game.clients[i].resp.team_on->index;

       
                        score = game.clients[i].resp.score;
   
                            for (j = 0; j < total[team]; j++)
                            {
                                 if (score > sortedscores[team][j])
                                        break;
                            }
                            for (k=total[team] ; k>j ; k--)
                            {
                                  sorted[team][k] = sorted[team][k-1];
                                  sortedscores[team][k] = sortedscores[team][k-1];
                            }
                       
                        sorted[team][j] = i;
                        sortedscores[team][j] = score;
                        totalscore[team] += score;
                        total[team]++;
                        if (cl_ent->solid != SOLID_NOT &&
                                cl_ent->deadflag != DEAD_DEAD)
                                totalalive[team]++;
                } 
        

				if (!strcmp(team_list[0]->teamid, "usa") && !strcmp(team_list[1]->teamid, "grm"))
				{
					Com_sprintf (string, sizeof(string),
								"xv 0  yv   0 picn scorehead  " // background header
								"yv 80 xv   0 picn scoreleft  " // background left list
								"      xv 160 picn scoreright " // background right list
								);
				}
				else
				// if teams are anything else or grm|usa, display the split graphics:
				{
					Q_strncpyz (scoreleftpic, sizeof(scoreleftpic), "yv 80 xv   0 picn ");
					Q_strncatz (scoreleftpic, sizeof(scoreleftpic), team_list[0]->teamid);
					Q_strncatz (scoreleftpic, sizeof(scoreleftpic), "_score  ");

					Q_strncpyz (scorerightpic, sizeof(scorerightpic), "      xv 160 picn ");
					Q_strncatz (scorerightpic, sizeof(scorerightpic), team_list[1]->teamid);
					Q_strncatz (scorerightpic, sizeof(scorerightpic), "_score  ");

					Q_strncpyz (scoretopleftpic, sizeof(scoretopleftpic), "xv 0  yv   0 picn ");
					Q_strncatz (scoretopleftpic, sizeof(scoretopleftpic), team_list[0]->teamid);
					Q_strncatz (scoretopleftpic, sizeof(scoretopleftpic), "_score_top  ");

					Q_strncpyz (scoretoprightpic, sizeof(scoretoprightpic), "xv 0  xv 160 picn ");
					Q_strncatz (scoretoprightpic, sizeof(scoretoprightpic), team_list[1]->teamid);
					Q_strncatz (scoretoprightpic, sizeof(scoretoprightpic), "_score_top  ");

					Com_sprintf (string, sizeof(string), scoretopleftpic); //team 0
					Q_strncatz (string, sizeof(string), scoretoprightpic); // team 1
					Q_strncatz (string, sizeof(string), scoreleftpic); //background left list pic
					Q_strncatz (string, sizeof(string), scorerightpic); // background right list pic
				}

                Com_sprintf (string2, sizeof(string2), 
						// TEAM1
						"xv 37 yv 38 string \"%4d/%-3d\" "
						"xv 97 yv 38 string \"%4d/%-3d\" "
						 // TEAM2
						 "xv 191 yv 38 string \"%4d/%-3d\" "
						"xv 250 yv 38 string \"%4d/%-3d\" ",
						team_list[TEAM1]->kills, team_list[TEAM1]->need_kills, team_list[TEAM1]->score, team_list[TEAM1]->need_points,// name_pos[TEAM1], team_list[TEAM1]->teamname,
						team_list[TEAM2]->kills, team_list[TEAM2]->need_kills, team_list[TEAM2]->score, team_list[TEAM2]->need_points);//, name_pos[TEAM2] + 160, team_list[TEAM2]->teamname);

				Q_strncatz (string, sizeof(string), string2);

				if (stats->value)
				{
					Q_strncatz (string, sizeof(string),		"xv 4   yv 67 string  \"Player   Rating +/-\" ");//faf
					Q_strncatz (string, sizeof(string),		"xv 164   yv 67 string  \"Player   Rating +/-\" ");//faf
				}
				else
				{
					Q_strncatz (string, sizeof(string),		"xv 4   yv 67 string  \"Player   Acrcy% +/-\" ");//faf
					Q_strncatz (string, sizeof(string),		"xv 164   yv 67 string  \"Player   Acrcy% +/-\" ");//faf
				}

			// pbowens: team victory pix
			if (level.intermissiontime && Last_Team_Winner != 99 &&
				Last_Team_Winner != -1) 
			{
				if (campaign_winner == 0){
					Q_strncatz (string, sizeof(string), va("xv 0 yv -80 picn allies_campaign "));
				}
				else if (campaign_winner == 1){
					Q_strncatz (string, sizeof(string), va("xv 0 yv -80 picn axis_campaign "));
				}
				else{
					Q_strncatz (string, sizeof(string), va("xv 0 yv -80 picn victory_%s ", 
						team_list[Last_Team_Winner]->teamid ));
				}
			}



                len = (int)strlen(string);

                totalaliveprinted[TEAM1] = totalaliveprinted[TEAM2] = 0;
                stoppedat[TEAM1] = stoppedat[TEAM2] = -1;
        
                for (i=0 ; i < (MAX_SCORES_PER_TEAM + 1) ; i++)
                {
                        if (i >= total[TEAM1] && i >= total[TEAM2])
                                break; 

        
                        // ok, if we're approaching the "maxsize", then let's stop printing members of each
                        // teams (if there's more than one member left to print in that team...)
                        if (len > (maxsize - 100))
                        {
                                if (i < (total[TEAM1] - 1))
                                        stoppedat[TEAM1] = i;
                                if (i < (total[TEAM2] - 1))
                                        stoppedat[TEAM2] = i;
                        }
                        if (i == MAX_SCORES_PER_TEAM-1) 
                        {
                                if (total[TEAM1] > MAX_SCORES_PER_TEAM)
                                        stoppedat[TEAM1] = i;
                                if (total[TEAM2] > MAX_SCORES_PER_TEAM)
                                        stoppedat[TEAM2] = i;
                        }
        
                        if (i < total[TEAM1] && stoppedat[TEAM1] == -1)  // print next team 1 member...
                        {
                                cl = &game.clients[sorted[TEAM1][i]];
                                cl_ent = g_edicts + 1 + sorted[TEAM1][i];
                                if (cl_ent->solid != SOLID_NOT &&
                                        cl_ent->deadflag != DEAD_DEAD)
                                        totalaliveprinted[TEAM1]++;
        
								if (game.clients[sorted[TEAM1][i]].resp.accuracy_hits + game.clients[sorted[TEAM1][i]].resp.accuracy_misses == 0)
									accuracy = 0;
								else
									accuracy = (100 * game.clients[sorted[TEAM1][i]].resp.accuracy_hits /(game.clients[sorted[TEAM1][i]].resp.accuracy_hits + game.clients[sorted[TEAM1][i]].resp.accuracy_misses));

								//if using stats, replace accuracy with average stats
								if (stats->value)
								{
										total_deaths = game.clients[sorted[TEAM1][i]].pers.stat_human_kills +
														game.clients[sorted[TEAM1][i]].pers.stat_bot_kills +
														game.clients[sorted[TEAM1][i]].resp.stat_human_plus	+
														game.clients[sorted[TEAM1][i]].resp.stat_bot_plus	+
														game.clients[sorted[TEAM1][i]].pers.stat_human_deaths +
														game.clients[sorted[TEAM1][i]].pers.stat_bot_deaths +
														game.clients[sorted[TEAM1][i]].resp.stat_human_minus +
														game.clients[sorted[TEAM1][i]].resp.stat_bot_minus;

										cl_ent = g_edicts + 1 + sorted[TEAM1][i];

										if (total_deaths != 0 && !cl_ent->ai)
										{	
											accuracy =100 * (game.clients[sorted[TEAM1][i]].pers.stat_human_kills + game.clients[sorted[TEAM1][i]].pers.stat_bot_kills + game.clients[sorted[TEAM1][i]].resp.stat_human_plus + game.clients[sorted[TEAM1][i]].resp.stat_bot_plus)/total_deaths;

										}
										else
											accuracy = 0;
								}


							//	sprintf (string + strlen(string), 
								Com_sprintf (string + strlen(string), sizeof(string) - strlen(string), 
										"xv 3 yv %d string \"%-12.12s%3d%3d\"",
										87 + i * 14,//8,
										va("%s%s",(game.clients[sorted[TEAM1][i]].resp.mos == MEDIC) ? "+" : " ", game.clients[sorted[TEAM1][i]].pers.netname),
									//	game.clients[sorted[TEAM1][i]].pers.netname,
										(int)accuracy,
									//	game.clients[sorted[TEAM1][i]].resp.plus_minus);   
										game.clients[sorted[TEAM1][i]].resp.stat_human_plus + game.clients[sorted[TEAM1][i]].resp.stat_bot_plus 
										- game.clients[sorted[TEAM1][i]].resp.stat_human_minus - game.clients[sorted[TEAM1][i]].resp.stat_bot_minus);   	
                        }
        
                        if (i < total[TEAM2] && stoppedat[TEAM2] == -1)  // print next team 2 member...
                        {
                                cl = &game.clients[sorted[TEAM2][i]];
                                cl_ent = g_edicts + 1 + sorted[TEAM2][i];
                                if (cl_ent->solid != SOLID_NOT &&
                                        cl_ent->deadflag != DEAD_DEAD)
                                        totalaliveprinted[TEAM2]++;
        
								if (game.clients[sorted[TEAM2][i]].resp.accuracy_hits + game.clients[sorted[TEAM2][i]].resp.accuracy_misses == 0)
									accuracy = 0;
								else
									accuracy = (100 * game.clients[sorted[TEAM2][i]].resp.accuracy_hits /(game.clients[sorted[TEAM2][i]].resp.accuracy_hits + game.clients[sorted[TEAM2][i]].resp.accuracy_misses));



								//if using stats, replace accuracy with average stats
								if (stats->value)
								{
										total_deaths = game.clients[sorted[TEAM2][i]].pers.stat_human_kills +
														game.clients[sorted[TEAM2][i]].pers.stat_bot_kills +
														game.clients[sorted[TEAM2][i]].resp.stat_human_plus	+
														game.clients[sorted[TEAM2][i]].resp.stat_bot_plus	+
														game.clients[sorted[TEAM2][i]].pers.stat_human_deaths +
														game.clients[sorted[TEAM2][i]].pers.stat_bot_deaths +
														game.clients[sorted[TEAM2][i]].resp.stat_human_minus +
														game.clients[sorted[TEAM2][i]].resp.stat_bot_minus;

										cl_ent = g_edicts + 1 + sorted[TEAM2][i];

										if (total_deaths != 0 && !cl_ent->ai)
										{	
											accuracy =100 * (game.clients[sorted[TEAM2][i]].pers.stat_human_kills + game.clients[sorted[TEAM2][i]].pers.stat_bot_kills + game.clients[sorted[TEAM2][i]].resp.stat_human_plus + game.clients[sorted[TEAM2][i]].resp.stat_bot_plus)/total_deaths;

										}
										else
											accuracy = 0;
								}
							
							//	sprintf (string + strlen(string), 
								Com_sprintf (string + strlen(string), sizeof(string) - strlen(string), 
										"xv 165 yv %d string \"%-12.12s%3d%3d\"",
										87 + i * 14,//8, 
										va("%s%s",(game.clients[sorted[TEAM2][i]].resp.mos == MEDIC) ? "+" : " ", game.clients[sorted[TEAM2][i]].pers.netname),
										(int)accuracy,
									//	game.clients[sorted[TEAM2][i]].resp.plus_minus);
										game.clients[sorted[TEAM2][i]].resp.stat_human_plus + game.clients[sorted[TEAM2][i]].resp.stat_bot_plus 
										- game.clients[sorted[TEAM2][i]].resp.stat_human_minus - game.clients[sorted[TEAM2][i]].resp.stat_bot_minus);   
                        }
        
                        len = (int)strlen(string);
                }


				if (stoppedat[TEAM1] > -1)
				{
				//	sprintf (string + strlen(string),
					Com_sprintf (string + strlen(string), sizeof(string) - strlen(string),
							"xv 0 yv 200 string \" and %d more\" ",
							total[TEAM1] - stoppedat[TEAM1]);
				}
				if (stoppedat[TEAM2] > -1)
				{
				//	sprintf (string + strlen(string),
					Com_sprintf (string + strlen(string), sizeof(string) - strlen(string),
							"xv 160 yv 200 string \" and %d more\" ",
							total[TEAM2] - stoppedat[TEAM2]);
				}
             //   }
        


        if (strlen(string) > 1300)  // for debugging...
                gi.dprintf("Warning: scoreboard string neared or exceeded max length\nDump:\n%s\n---\n", 
                                string);

        gi.WriteByte (svc_layout);
        gi.WriteString (string);
}




/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard (edict_t *ent)
{

/*
	if(!ent->client->showpscores) 
		DeathmatchScoreboardMessage (ent, ent->enemy);
	else 
		DeathmatchPlayerScore (ent, ent->enemy);
*/

	// pbowens: just do the scoreboard



	//JABot[start]
	if (ent->ai || !ent->inuse)
		return;
	//[end]


	if (ent->client->layout_type == SHOW_SCORES)
		A_ScoreboardMessage(ent);
	else if (ent->client->layout_type == SHOW_PSCORES)
		A_ScoreboardMessage2(ent);

	gi.unicast (ent, true);
}
/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void ShowCampaign (edict_t *ent);
void Cmd_Score_f (edict_t *ent)
{
	//JABot[start]
	if (ent->ai)
		return;
	//[end]

	if (level.intermissiontime && mapvoting->value && !ent->client->voted)
			return;


	if (ent->client->menu)
		PMenu_Close(ent);

	if (!deathmatch->value && !coop->value)
		return; 

	if (level.intermissiontime)
	{
		if (mapvoting->value && !ent->client->voted)
			return;

		if (ent->client->layout_type == SHOW_SCORES)
			ent->client->layout_type = SHOW_PSCORES;
		else if (level.campaign && ent->client->layout_type == SHOW_PSCORES)
		{	ent->client->layout_type = SHOW_CAMPAIGN;
			ShowCampaign(ent);
		}
		else
			ent->client->layout_type = SHOW_SCORES;
	}
	else
	{
		//if scoreboard is already on...
		if (ent->client->layout_type == SHOW_SCORES)
		{
			if (ent->client->last_menu_time && ent->client->last_menu_time > level.time - .5)
			{
				ent->client->layout_type = SHOW_PSCORES;
				ent->client->last_menu_time = level.time;
				DeathmatchScoreboard (ent);
				return;
			}
			else
			{
				ent->client->layout_type = SHOW_NONE;

				if (ent->client->resp.team_on)
				{
					ent->client->layout_type = SHOW_OBJECTIVES_TEMP;
					ent->client->show_obj_temp_time = level.time;
					Cmd_Objectives(ent);
				}
				return;
			}
		
		}
		else if (ent->client->layout_type == SHOW_PSCORES)
		{
			if (ent->client->last_menu_time && ent->client->last_menu_time > level.time - .5)
			{
				if (level.objectivepic)
					ent->client->display_info = true;
				
				ent->client->layout_type = SHOW_NONE;

				if (level.campaign)
				{
					ent->client->layout_type = SHOW_CAMPAIGN;
					ShowCampaign (ent);
					return;
				}
			}
			else
			{
				ent->client->layout_type = SHOW_NONE;

				if (ent->client->resp.team_on)
				{
					ent->client->layout_type = SHOW_OBJECTIVES_TEMP;
					ent->client->show_obj_temp_time = level.time;
					Cmd_Objectives(ent);
				}
				return;
			}



		}
		else if (ent->client->layout_type == SHOW_CAMPAIGN || ent->client->display_info == true)
		{
			ent->client->layout_type = SHOW_NONE;	
			ent->client->display_info = 0;
		}
		else// if (ent->client->layout_type == SHOW_NONE)
		{
			ent->client->layout_type = SHOW_SCORES;			
			ent->client->last_menu_time = level.time;
		}
	}

//	ent->client->showscores = true;
	DeathmatchScoreboard (ent);
}


/*
==================
HelpComputer

Draw help computer.
==================
*/
void ShowCampaign (edict_t *ent)
{
	char	string[1024];
	int		i, curx = 0, cury = 0;

	//JABot[start]
	if (ent->ai || !ent->inuse)
		return;
	//[end]

//	gi.dprintf("%s\n", campaign->string);
	if (!level.campaign)
		return;

	
	for (i = 0; campaign_spots[i].bspname; i++)
	{
		if (!strcmp(level.mapname,campaign_spots[i].bspname))
		{
			curx = campaign_spots[i].xpos - 1;
			cury = campaign_spots[i].ypos - 1;
		}
	}

	// send the layout
//	Com_sprintf (string, sizeof(string), "");
//	sprintf (string, "%sxv -16 yv 10 picn %s ", string, campaign->string);
	Com_sprintf (string, sizeof(string), "%sxv 7 yv 7 picn %s ", string, level.campaign);

	if (curx && cury)
	//	sprintf (string,
		Com_sprintf (string + strlen(string), sizeof(string) - strlen(string),
				"%sxv %i yv %i picn o ", string, curx, cury);

	for (i = 0; campaign_spots[i].bspname; i++)
	{
	//	sprintf (string,
		Com_sprintf (string + strlen(string), sizeof(string) - strlen(string),
					"%sxv %i yv %i picn ", string, campaign_spots[i].xpos, campaign_spots[i].ypos);

		if (campaign_spots[i].owner == 0)
		//	sprintf (string,
			Com_sprintf (string + strlen(string), sizeof(string) - strlen(string),
					"%su ", string);
		else if (campaign_spots[i].owner == 1)
		//	sprintf (string,
			Com_sprintf (string + strlen(string), sizeof(string) - strlen(string),
					"%sg ", string);
		else
			Q_strncatz (string, sizeof(string), "q ");
	}

	Q_strncatz (string, sizeof(string), "xv 22 yv 36 picn u ");
	Q_strncatz (string, sizeof(string), "xv 37 yv 38 string \"");
//	sprintf (string,
	Com_sprintf (string + strlen(string), sizeof(string) - strlen(string),
				"%s%i", string, alliedplatoons);
	Q_strncatz (string, sizeof(string), "\" ");
	Q_strncatz (string, sizeof(string), "xv 22 yv 56 picn g ");
	Q_strncatz (string, sizeof(string), "xv 37 yv 58 string \"");
//	sprintf (string,
	Com_sprintf (string + strlen(string), sizeof(string) - strlen(string),
				"%s%i", string, axisplatoons);
	Q_strncatz (string, sizeof(string), "\" ");


	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);

//	gi.dprintf ("%s\n", string);
}

void ShowServerImg (edict_t *ent)
{
	char	string[1024];

	//JABot[start]
	if (ent->ai || !ent->inuse)
		return;
	//[end]

	if (!serverimg->string)
		return;

	// send the layout
//	Com_sprintf (string, sizeof(string), "");
//	sprintf (string, "%sxv -16 yv 10 picn %s ", string, campaign->string);
	Com_sprintf (string, sizeof(string), "%sxv 7 yv 7 picn %s ", string, serverimg->string);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);

}

/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent)
{
	char	string[1024];
	char	*sk;

	//JABot[start]
	if (ent->ai || !ent->inuse)
		return;
	//[end]

	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else
		sk = "hard+";

	// send the layout
	Com_sprintf (string, sizeof(string),
				"xv 32 yv 8 picn help "			// background
				"xv 202 yv 12 string2 \"%s\" "		// skill
				"xv 0 yv 24 cstring2 \"%s\" "		// level name
				"xv 0 yv 54 cstring2 \"%s\" "		// help 1
				"xv 0 yv 110 cstring2 \"%s\" "		// help 2
				"xv 50 yv 164 string2 \" kills     goals    secrets\" "
				"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ", 
				sk,
				level.level_name,
				game.helpmessage1,
				game.helpmessage2,
				level.killed_monsters, level.total_monsters, 
				level.found_goals, level.total_goals,
				level.found_secrets, level.total_secrets);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{


	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	/*
	ent->client->showinventory = false;
	ent->client->showscores = false;
	ent->client->showobjectives = 0;

	if (ent->client->showhelp && (ent->client->resp.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->resp.helpchanged = 0; */

}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent)
{
	gitem_t		*item;
	int			index,delay;

	if (ent->ai)
		return;
	//
	// HEALTH 
	//
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	if (ent->health > 0 &&
		ent->client->enter_spawn_time &&
		ent->client->enter_spawn_time > level.time - 4)
		ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex ("i_respcount");






	//
	// SELECTED WEAPON
	//
	if (ent->client->pers.weapon && ent->client->pers.weapon->icon) 
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ( ent->client->pers.weapon->icon );
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;

	//
	// MAGAZINES / CLIPS / ROCKETS / ROUNDS
	//
	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
	{
		item	= FindItem(ent->client->pers.weapon->ammo);
		
		if (item) 
		{
			index	= ITEM_INDEX(item);
			ent->client->ps.stats[STAT_MAGS] =	ent->client->pers.inventory[index]; //anum

			// handle rounds here
			if (ent->client->p_rnd)
			{
				ent->client->ps.stats[STAT_MAGS_ICON] = (item->icon) ? gi.imageindex(item->icon) : 0;
				ent->client->ps.stats[STAT_ROUNDS] = *ent->client->p_rnd;
			}
			else // if there are no rounds, do not display icon/count
			{
				ent->client->ps.stats[STAT_MAGS_ICON] = 0;
				ent->client->ps.stats[STAT_ROUNDS] = 0;
			}

		} 
		else // if there is no ammo, then do not display anything
		{
			ent->client->ps.stats[STAT_MAGS] = 0;
			ent->client->ps.stats[STAT_MAGS_ICON] = 0;
			ent->client->ps.stats[STAT_ROUNDS] = 0;
		}
	}
	else // if there is not a weapon, then do not display anything
	{
		ent->client->ps.stats[STAT_MAGS] = 0;		
		ent->client->ps.stats[STAT_MAGS_ICON] = 0;
		ent->client->ps.stats[STAT_ROUNDS] = 0;
	}


	//
	// PICKUP MESSAGES
	//
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	//
	// STAMINA
	//
	// pbowens: new stamina bar
	ent->client->ps.stats[STAT_STAMINA_CUR] = gi.imageindex( (ent->client->jump_stamina < 10.0) ? "s_00" : va("s_%i", ((int)(ent->client->jump_stamina / 10.0) * 10)) );

	//
	// TIMERS
	//
	// level_wait timer (i_dday)
	delay = 0;
	if (ent->client->resp.team_on)
		delay = ent->client->resp.team_on->delay;
	if (level.framenum < ((int)(delay + level_wait->value) * 10) )
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("i_dday");
		ent->client->ps.stats[STAT_TIMER] = ((int)(delay + level_wait->value) - (level.framenum / 10));
	} 	
	// forced respawn tuner (i_respcount)
	else if (level.framenum <= ent->client->forcespawn)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("i_respcount");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->forcespawn - level.framenum) / 10;
	}
	else if ((level.time < ent->leave_limbo_time) && (ent->client->limbo_mode))  //faf:  respawn timer
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("i_dday");
		ent->client->ps.stats[STAT_TIMER] = ((int)(ent->leave_limbo_time - level.time)) + 1;
	}
	else if (level.map_vote_time > 0)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("i_dday");
		ent->client->ps.stats[STAT_TIMER] = ((int)(15 + level.map_vote_time - level.time));

	}



	// nothing at all
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON]	= 0;
		ent->client->ps.stats[STAT_TIMER]		= 0;
	}
	

	if (level.obj_time)  //faf:  respawn timer
	{
//		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("i_dday");
		ent->client->ps.stats[STAT_TIMER2] = level.obj_time;
		ent->client->ps.stats[STAT_TIMER2_ICON] = gi.imageindex(va("teams/%s", team_list[level.obj_team]->teamid));
	}
	else
	{
		ent->client->ps.stats[STAT_TIMER2] =0;
	}

	if (ent->client->resp.autopickup == true)
		ent->client->ps.stats[STAT_AUTOPICKUP] = gi.imageindex ("w_fists");//1;
	else
		ent->client->ps.stats[STAT_AUTOPICKUP] = 0;



	// 
	//  TIMER # 2   //faf: ctb code
	//
	if (level.ctb_time)
	{
		if ((level.ctb_time - level.time) > 0)
		{
			ent->client->ps.stats[STAT_TIMER2] = ((level.ctb_time) - (level.time));
			if  (((level.ctb_time) - level.time) < 100)
			{
				ent->client->ps.stats[STAT_TIMER2] = ((level.ctb_time) - level.time);
			}
		}
		else
			ent->client->ps.stats[STAT_TIMER2] = 0;
	}



	//
	// OBJECTIVES
	//
	if (level.objectivepic && ent->client->display_info) 
	{
		char	pic[26];
		Q_strncpyz (pic, sizeof(pic), "objectives\\");
		Q_strncatz (pic, sizeof(pic), level.mapname);

		ent->client->ps.stats[STAT_OBJECTIVE] = gi.imageindex (pic);
	}
	else 
		ent->client->ps.stats[STAT_OBJECTIVE] = 0;

	//
	// REGULAR CROSSHAIR
	//
	if (ent->client->resp.mos == MEDIC && ent->client->pers.weapon &&
		ent->client->pers.weapon->classnameb == WEAPON_MORPHINE)
		ent->client->ps.stats[STAT_CROSSHAIR] = gi.imageindex ("crosshair");
	else
		ent->client->ps.stats[STAT_CROSSHAIR] = 0;

	if (level.intermissiontime)
		ent->client->ps.stats[STAT_CROSSHAIR] = 0;

	//
	// SNIPER CROSSHAIR
	//
	if (ent->client->crosshair && ent->client->pers.weapon && ent->client->pers.weapon->position == LOC_SNIPER)
		ent->client->ps.stats[STAT_CROSSHAIR] = gi.imageindex (va("scope_%s", ent->client->pers.weapon->dllname));
	else if (ent->client->turret)
		ent->client->ps.stats[STAT_CROSSHAIR] = gi.imageindex ("scope_usa");
	else if (ent->client->resp.mos == OFFICER && ent->client->pers.weapon && ent->client->aim && ent->client->pers.weapon->classnameb == WEAPON_BINOCULARS &&
		ent->client->weaponstate == WEAPON_READY)
		ent->client->ps.stats[STAT_CROSSHAIR] = gi.imageindex ("scope_usa");
	else
		ent->client->ps.stats[STAT_CROSSHAIR] = 0;
	
	if (ent->client->chasetarget && ent->client->chasetarget->client->aim && ent->client->aim)
		ent->client->ps.stats[STAT_CROSSHAIR] = ent->client->chasetarget->client->ps.stats[STAT_CROSSHAIR];

	//
	// SELECTED ITEM
	//
	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//
	// LAYOUTS
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->layout_type == SHOW_OBJECTIVES ||
			ent->client->layout_type == SHOW_OBJECTIVES_TEMP ||
			ent->client->layout_type == SHOW_CAMPAIGN ||
			ent->client->pers.health <= 0 || 
			ent->client->layout_type == SHOW_SCORES || 
			ent->client->layout_type == SHOW_PSCORES ||
			ent->client->layout_type == SHOW_MEDIC_SCREEN ||
			ent->client->layout_type == SHOW_SERVERIMG ||
			(level.intermissiontime && (level.time >= level.intermissiontime + INTERMISSION_DELAY)) )
				ent->client->ps.stats[STAT_LAYOUTS] |= 1;

		if (ent->client->layout_type == SHOW_INVENTORY && ent->client->pers.health >0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->layout_type == SHOW_SCORES || ent->client->layout_type == SHOW_PSCORES)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->layout_type == SHOW_INVENTORY && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}

	//
	// FRAGS
	//
	//faf:  so scores appear on gamespy
	if (player_scores->value)
		ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	//
	// help icon / current weapon if not shown
	//
	//ent->client->resp.helpchanged = 0;
	if (ent->client->resp.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
//	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > STANDARD_FOV + 1)
//		&& ent->client->pers.weapon)
//		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;
 
	TeamStats(ent);

	//faf:  ctb code:  always put briefcase pic here if holding briefcase
	if (ent->client->has_briefcase)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("briefcase");


	//ent->client->ps.stats[STAT_TEAM0_INFO] = 0;
	//ent->client->ps.stats[STAT_TEAM1_INFO] = 0;
}

