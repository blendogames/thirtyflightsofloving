/* Scoreboards */
#include "g_local.h"
#include "aj_scoreboard.h"

void Lithium_FragsScoreboardMessage (edict_t *ent, edict_t *killer)
{ 
	char entry[1024]; 
	char string[1400]; 
	char ping[6];
	int stringlength=0; 
	int i, j, k; 
	int sorted[MAX_CLIENTS]; 
	int sortedscores[MAX_CLIENTS]; 
	int score, total; 
	int picnum; 
	int x, y; 
	gclient_t *cl;
	edict_t *cl_ent; 
	float	frags_per_hour;

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

	// make a header for the data 
	Com_sprintf (entry, sizeof(entry), 
				"xv 160 yv 16 string2 \"Frags\" " 
				"xv 216 yv 16 string \"FPH\" "
				"xv 248 yv 16 string \"Time\" " 
				"xv 288 yv 16 string \"Ping\" ");
	j = (int)strlen(entry); 
	if (stringlength + j < 1024) 
	{ 
	//	strcpy (string + stringlength, entry); 
		Com_strcpy (string + stringlength, sizeof(string) - stringlength, entry); 
		stringlength += j; 
	} 

     // add the clients in sorted order 

	if (total > 25) 
		total = 25; 

	for (i=0 ; i<total; i++)
	{ 
		cl = &game.clients[sorted[i]]; 
		cl_ent = g_edicts + 1 + sorted[i]; 

		picnum = gi.imageindex ("i_fixme"); 

		x = 32; 
		y = 24 + 8 * i; 

		if (cl_ent->bot_client)
		{
			if (showbotping->value)
			{
				cl->ping = (int) cl_ent->bot_stats->avg_ping + ((random() * 2) - 1) * 80;
				if (cl->ping < 0)
					cl->ping = 0;
				Com_sprintf (ping, sizeof(ping), "%4i", cl->ping);
			}
			else
				Com_strcpy (ping, sizeof(ping), " bot");
		}
		else
			Com_sprintf (ping, sizeof(ping), "%4i", cl->ping);

		frags_per_hour = (float)(cl->resp.score) * (60.0/((level.framenum - cl->resp.enterframe)/600.0));

		if (cl_ent->client->observer_mode || cl_ent->client->bIsCamera)
		{
			if (cl_ent == ent)
			{
				Com_sprintf (entry, sizeof(entry), 
							"xv 8 yv %i string2 \"%2i\" "
							"xv 32 yv %i string2 \"%s\" " 
							"xv 168 yv %i string2 \"(Observer)     %s\" ", 
							y, i+1,
							y, cl->pers.netname, 
							y, ping); 
			}
			else
			{
				Com_sprintf (entry, sizeof(entry), 
							"xv 8 yv %i string \"%2i\" "
							"xv 32 yv %i string \"%s\" " 
							"xv 168 yv %i string \"(Observer)     %s\" ", 
							y, i+1,
							y, cl->pers.netname, 
							y, ping); 
			}
		}
		else
		{
			if (cl_ent==ent) // highlight this player in green
			{
				Com_sprintf (entry, sizeof(entry), 
							"xv 8 yv %i string2 \"%2i\" "
							"xv 32 yv %i string2 \"%s\" " 
							"xv 168 yv %i string2 \"%4i %4i %4i %s\" ", 
							y, i+1,
							y, cl->pers.netname, 
							y, cl->resp.score, 
							(int)(frags_per_hour),
							(level.framenum - cl->resp.enterframe)/600,
							ping); 
			}
			else
			{
				Com_sprintf (entry, sizeof(entry), 
							"xv 8 yv %i string \"%2i\" "
							"xv 32 yv %i string \"%s\" " 
							"xv 168 yv %i string \"%4i %4i %4i %s\" ", 
							y, i+1,
							y, cl->pers.netname, 
							y, cl->resp.score, 
							(int)(frags_per_hour),
							(level.framenum - cl->resp.enterframe)/600,
							ping); 
			}
		}

		j = (int)strlen(entry); 
		if (stringlength + j > 1024) 
			break; 
	//	strcpy (string + stringlength, entry); 
		Com_strcpy (string + stringlength, sizeof(string) - stringlength, entry); 
		stringlength += j; 
	} 

	gi.WriteByte (svc_layout); 
	gi.WriteString (string); 
} 



void Lithium_FPHScoreboardMessage (edict_t *ent, edict_t *killer)
{ 
	char entry[1024]; 
	char string[1400]; 
	char ping[6];
	int stringlength=0; 
	int i, j, k; 
	int sorted[MAX_CLIENTS]; 
	int sortedscores[MAX_CLIENTS]; 
	int score, total; 
	int picnum; 
	int x, y; 
	gclient_t *cl;
	edict_t *cl_ent; 
	float	frags_per_hour;

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

	// make a header for the data 
	Com_sprintf (entry, sizeof(entry), 
				"xv 160 yv 16 string \"Frags\" " 
				"xv 216 yv 16 string2 \"FPH\" "
				"xv 248 yv 16 string \"Time\" " 
				"xv 288 yv 16 string \"Ping\" ");
	j = (int)strlen(entry); 
	if (stringlength + j < 1024) 
	{ 
	//	strcpy (string + stringlength, entry); 
		Com_strcpy (string + stringlength, sizeof(string) - stringlength, entry); 
		stringlength += j; 
	} 

     // add the clients in sorted order 

	if (total > 25) 
		total = 25; 

	for (i=0 ; i<total ; i++)
	{ 
		cl = &game.clients[sorted[i]]; 
		cl_ent = g_edicts + 1 + sorted[i]; 

		picnum = gi.imageindex ("i_fixme"); 

		x = 32; 
		y = 24 + 8 * i; 

		if (cl_ent->bot_client)
		{
			if (showbotping->value)
			{
				cl->ping = (int) cl_ent->bot_stats->avg_ping + ((random() * 2) - 1) * 80;
				if (cl->ping < 0)
					cl->ping = 0;
				Com_sprintf (ping, sizeof(ping), "%4i", cl->ping);
			}
			else
				Com_strcpy (ping, sizeof(ping), " bot");
		}
		else
			Com_sprintf (ping, sizeof(ping), "%4i", cl->ping);

		frags_per_hour = (float)(cl->resp.score) * (60.0/((level.framenum - cl->resp.enterframe)/600.0));

		if (cl_ent->client->observer_mode || cl_ent->client->bIsCamera)
		{
			if (cl_ent == ent) // highlight this player in green
			{
				Com_sprintf (entry, sizeof(entry), 
							"xv 8 yv %i string2 \"%2i\" "
							"xv 32 yv %i string2 \"%s\" " 
							"xv 168 yv %i string2 \"(Observer)     %s\" ", 
							y, i+1,
							y, cl->pers.netname, 
							y, ping); 
			}
			else
			{
				Com_sprintf (entry, sizeof(entry), 
							"xv 8 yv %i string \"%2i\" "
							"xv 32 yv %i string \"%s\" " 
							"xv 168 yv %i string \"(Observer)     %s\" ", 
							y, i+1,
							y, cl->pers.netname, 
							y, ping); 
			}
		}
		else
		{
			if (cl_ent == ent) // highlight this player in green
			{
				Com_sprintf (entry, sizeof(entry), 
							"xv 8 yv %i string2 \"%2i\" "
							"xv 32 yv %i string2 \"%s\" " 
							"xv 168 yv %i string2 \"%4i %4i %4i %s\" ", 
							y, i+1,
							y, cl->pers.netname, 
							y, cl->resp.score, 
							(int)(frags_per_hour),
							(level.framenum - cl->resp.enterframe)/600,
							ping); 
			}
			else
			{
				Com_sprintf (entry, sizeof(entry), 
							"xv 8 yv %i string \"%2i\" "
							"xv 32 yv %i string \"%s\" " 
							"xv 168 yv %i string \"%4i %4i %4i %s\" ", 
							y, i+1,
							y, cl->pers.netname, 
							y, cl->resp.score, 
							(int)(frags_per_hour),
							(level.framenum - cl->resp.enterframe)/600,
							ping); 
			}
		}

		j = (int)strlen(entry); 
		if (stringlength + j > 1024) 
			break; 
	//	strcpy (string + stringlength, entry); 
		Com_strcpy (string + stringlength, sizeof(string) - stringlength, entry); 
		stringlength += j; 
	} 

	gi.WriteByte (svc_layout); 
	gi.WriteString (string); 
} 


#if 0
void ThreeTeamCTFScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	size_t	entryLen, stringLen;
	int		len;
	int		i, j, k, n;
	int		sorted[3][MAX_CLIENTS];
	int		sortedscores[3][MAX_CLIENTS];
	int		score, total[3], totalscore[3];
	int		last[3];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;

	// sort the clients by team and score
	total[0] = total[1] = total[2] = 0;
	last[0] = last[1] = last[2] = 0;
	totalscore[0] = totalscore[1] = totalscore[2] = 0;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			team = 0;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			team = 1;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM3)
			team = 2;
		else
			continue; // unknown team?

		score = game.clients[i].resp.score;
		for (j=0 ; j<total[team] ; j++)
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
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	// team one
	Com_sprintf (string, sizeof(string), "if 24 xv 0 yv 8 pic 24 endif "
		"xv 32 yv 28 string \"%4d/%-3d\" "
		"xv 90 yv 12 num 2 18 "
		"if 25 xv 152 yv 8 pic 25 endif "
		"xv 186 yv 28 string \"%4d/%-3d\" " //was 154
		"xv 240 yv 12 num 2 20 "
		"if 29 xv 304 yv 8 pic 29 endif "
		"xv 336 yv 28 string \"%4d/%-3d\" "
		"xv 392 yv 12 num 2 30 ",
		totalscore[0], total[0],
		totalscore[1], total[1],
		totalscore[2], total[2]);
	len = strlen(string);

	for (i=0 ; i<24; i++)
	{
		if (i >= total[0] && i >= total[1] && i>=total[2])
			break; // we're done

		*entry = 0;

		// red team
		if (i < total[0]) 
		{
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];

		//	sprintf (entry + strlen(entry),
			entryLen = strlen(entry);
			Com_sprintf (entry + entryLen, sizeof(entry) - entryLen,
				"ctf 0 %d %d %d %d ",
				42 + i * 8,
				sorted[0][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

		/*	if (!lmctf->value && cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
				entryLen = strlen(entry);
				Com_sprintf (entry + entryLen, sizeof(entry) - entryLen, "xv 56 yv %d picn sbfctf2 ",
					42 + i * 8);
			}*/

			if (maxsize - len > strlen(entry))
			{
				Com_strcat (string, sizeof(string), entry);
				len = strlen(string);
				last[0] = i;
			}
		}

		// blue team
		if (i < total[1]) 
		{
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];

		//	sprintf (entry + strlen(entry),
			entryLen = strlen(entry);
			Com_sprintf (entry + entryLen, sizeof(entry) - entryLen,
				"ctf 152 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

		/*	if (!lmctf->value && cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
				entryLen = strlen(entry);
				Com_sprintf (entry + entryLen, sizeof(entry) - entryLen, "xv 216 yv %d picn sbfctf1 ",
					42 + i * 8);
			}*/

			if (maxsize - len > strlen(entry)) 
			{
				Com_strcat (string, sizeof(string), entry);
				len = strlen(string);
				last[1] = i;
			}
		}

		// greenteam
		if (i < total[2]) 
		{
			cl = &game.clients[sorted[2][i]];
			cl_ent = g_edicts + 1 + sorted[2][i];

		//	sprintf(entry+strlen(entry),
			entryLen = strlen(entry);
			Com_sprintf (entry + entryLen, sizeof(entry) - entryLen,
				"ctf 304 %d %d %d %d ",
				42 + i * 8,
				sorted[2][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

		/*	if (!lmctf->value && cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
				entryLen = strlen(entry);
				Com_sprintf (entry + entryLen, sizeof(entry) - entryLen, "xv 216 yv %d picn sbfctf1 ",
					42 + i * 8);
			}*/

			if (maxsize - len > strlen(entry)) 
			{
				Com_strcat (string, sizeof(string), entry);
				len = strlen(string);
				last[2] = i;
			}
		}

	}

	// put in spectators if we have enough room
//	if (last[0] > last[1])
	if (last[0] > last[1] && last[0] > last[2])
		j = last[0];
//	else
	else if (last[1] > last[0] && last[1] > last[2])
		j = last[1];
	else
		j = last[2];

	j = (j + 2) * 8 + 42;

	k = n = 0;
	if (maxsize - len > 50)
	{
		for (i = 0; i < maxclients->value; i++)
		{
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			if (!cl_ent->inuse ||
				cl_ent->solid != SOLID_NOT ||
				cl_ent->client->resp.ctf_team != CTF_NOTEAM)
				continue;

			if (!k) {
				k = 1;
				Com_sprintf (entry, sizeof(entry), "xv 0 yv %d string2 \"Spectators\" ", j);
				Com_strcat (string, sizeof(string), entry);
				len = strlen(string);
				j += 8;
			}

		//	sprintf(entry+strlen(entry),
			entryLen = strlen(entry);
			Com_sprintf (entry + entryLen, sizeof(entry) - entryLen,
				"ctf %d %d %d %d %d ",
				(n & 1) ? 160 : 0, // x
				j, // y
				i, // playernum
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);
			if (maxsize - len > strlen(entry)) {
				Com_strcat (string, sizeof(string), entry);
				len = strlen(string);
			}
			
			if (n & 1)
				j += 8;
			n++;
		}
	}

	if (total[0] - last[0] > 1) { // couldn't fit everyone
		stringLen = strlen(string);
		Com_sprintf (string + stringLen, sizeof(string) - stringLen, "xv 0 yv %d string \"..and %d more\" ",
			42 + (last[0]+1)*8, total[0] - last[0] - 1);
	}
	if (total[1] - last[1] > 1) { // couldn't fit everyone
		stringLen = strlen(string);
		Com_sprintf (string + stringLen, sizeof(string) - stringLen, "xv 160 yv %d string \"..and %d more\" ",
			42 + (last[1]+1)*8, total[1] - last[1] - 1);
	}
	if (total[2] - last[2] > 1) { // couldn't fit everyone
		stringLen = strlen(string);
		Com_sprintf (string + stringLen, sizeof(string) - stringLen, "xv 168 yv %d string \"..and %d more\" ",
			42 + (last[2]+1)*8, total[2] - last[2] - 1);
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}
#endif
