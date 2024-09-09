#include "g_local.h"
#include "e_hook.h"
#include "aj_statusbars.h" // AJ
#include "aj_scoreboard.h" // AJ


/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	gclient_t	*client;

	client = ent->client;

	if (deathmatch->value || coop->value)
		ent->client->showscores = true;
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

	// clean up powerup info
	client->quad_framenum = 0;
	client->invincible_framenum = 0;
	client->breather_framenum = 0;
	client->enviro_framenum = 0;
	client->grenade_blew_up = false;
	client->grenade_time = 0;

	// RAFAEL
	ent->client->quadfire_framenum = 0;
	ent->client->ps.rdflags &= ~RDF_IRGOGGLES;		// PGM
	ent->client->ir_framenum = 0;					// PGM
	ent->client->nuke_framenum = 0;					// PMM
	ent->client->double_framenum = 0;				// PMM
	
	// RAFAEL
	ent->client->trap_blew_up = false;
	ent->client->trap_time = 0;
	
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

	gi.linkentity(ent);

	// add the layout

	if (!ent->bot_client && (deathmatch->value || coop->value))
	{
// AJ added intermission sound
		gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex(intermission_sound->string), 1, ATTN_NONE, 0);
// end AJ
		DeathmatchScoreboardMessage (ent, NULL);
		gi.unicast (ent, true);
	}

// AJ abandon hope hook
	if (sk_hook_style->value == 1)
		abandon_hook_reset(ent->client->hook);
}

void BeginIntermission (edict_t *targ)
{
	int		i, n;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

//ZOID
	if (deathmatch->value && ctf->value)
		CTFCalcScores();
//ZOID

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
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

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


void TeamplayScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k, t;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];

	bot_team_t	*sortedteams[MAX_TEAMS];
	int			doneteam[MAX_TEAMS];
	int			numteams, best, bestscore;

	int		score, total;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;

	// clear the doneteam flags
	memset(doneteam, 0, sizeof(int) * MAX_TEAMS);

	numteams = 0;	// incremented each time we add a team to the list

	// sort the teams
	for (i=0; i<MAX_TEAMS; i++)
	{
		if (!bot_teams[i])
			break;

		if (!bot_teams[i]->ingame)
			continue;

		bestscore = -999999;
		best = -1;

		// find the highest scoring team
		for (j=0; j<MAX_TEAMS; j++)
		{
			if (!bot_teams[j])
				break;

			if (doneteam[j])
				continue;
			if (!bot_teams[j]->ingame)
				continue;

			if (bot_teams[j]->score > bestscore)
			{
				best = j;
				bestscore = bot_teams[j]->score;
			}
		}

		if (best > -1)
		{
			doneteam[best] = true;
			sortedteams[numteams] = bot_teams[best];
			numteams++;
		}
		else	// must be done
		{
			break;
		}
	}

	string[0] = 0;
	stringlength = (int)strlen(string);

	for (t=0; t<numteams; t++)
	{	// print each team/player entry, maximum of 4 players per team
		if (t > 3)	// only print the top 4 teams
			break;

		// sort the clients by score
		total = 0;
		for (i=0 ; i<game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;

			if (cl_ent->client->team != sortedteams[t])
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

		if (total > 4)
			total = 4;

		x = 160;
		y = (t * 48);

		if (ent->client->team == sortedteams[t])
		{
			Com_sprintf (entry, sizeof(entry),
				"xv %i yv %i picn tag1 ", 32, y);
			j = (int)strlen(entry);
			if (stringlength + j > 1024)
				break;
		//	strcpy (string + stringlength, entry);
			Com_strcpy (string + stringlength, sizeof(string) - stringlength, entry);
			stringlength += j;
		}

		// Send team info
		Com_sprintf (entry, sizeof(entry),
			"xv %i yv %i string \"%s\" ", 70, y+6, sortedteams[t]->teamname);
		j = (int)strlen(entry);
		if (stringlength + j > 1024)
			break;
	//	strcpy (string + stringlength, entry);
		Com_strcpy (string + stringlength, sizeof(string) - stringlength, entry);
		stringlength += j;

		// Send team score info
		Com_sprintf (entry, sizeof(entry),
			"xv %i yv %i string \"%i\" ", 80, y + 20, sortedteams[t]->score);
		j = (int)strlen(entry);
		if (stringlength + j > 1024)
			break;
	//	strcpy (string + stringlength, entry);
		Com_strcpy (string + stringlength, sizeof(string) - stringlength, entry);
		stringlength += j;

		for (i=0 ; i<total ; i++)
		{
			cl = &game.clients[sorted[i]];
			cl_ent = g_edicts + 1 + sorted[i];

			y = (t * 48) + (i * 10);

			if (ent == cl_ent)
			{
				Com_sprintf (entry, sizeof(entry),
					"xv %i yv %i string \"%3i %s\" ",
					x, y, cl->resp.score, cl->pers.netname);
			}
			else
			{
				Com_sprintf (entry, sizeof(entry),
					"xv %i yv %i string2 \"%3i %s\" ",
					x, y, cl->resp.score, cl->pers.netname);
			}

			j = (int)strlen(entry);
			if (stringlength + j > 1024)
				break;

		//	strcpy (string + stringlength, entry);
			Com_strcpy (string + stringlength, sizeof(string) - stringlength, entry);
			stringlength += j;
		}
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
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

// AJ added 3tctf suport
/*	if (ctf->value && ttctf->value) 
	{
		ThreeTeamCTFScoreboardMessage (ent, killer);	
		return;
	}*/
// end AJ
//ZOID
	if (ctf->value)
	{
		CTFScoreboardMessage (ent, killer);
		return;
	}
//ZOID

	if (teamplay->value && ent->client->team)
	{
		TeamplayScoreboardMessage(ent, killer);
		return;
	}

// AJ
	if (def_scores->value==1)
	{
		Lithium_FragsScoreboardMessage(ent, killer);
		return;
	}
	else if (def_scores->value==2)
	{
		Lithium_FPHScoreboardMessage(ent, killer);
		return;
	}
// end AJ

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

	stringlength = (int)strlen(string);

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

//===============
//ROGUE
		// allow new DM games to override the tag picture
		if (gamerules && gamerules->value)
		{
			if(DMGame.DogTag)
				DMGame.DogTag(cl_ent, killer, &tag);
		}
//ROGUE
//===============

		if (tag)
		{
			Com_sprintf (entry, sizeof(entry),
				"xv %i yv %i picn %s ",x+32, y, tag);
			j = (int)strlen(entry);
			if (stringlength + j > 1024)
				break;
		//	strcpy (string + stringlength, entry);
			Com_strcpy (string + stringlength, sizeof(string) - stringlength, entry);
			stringlength += j;
		}

		if (cl_ent->bot_client)
		{
			cl->ping = (int) cl_ent->bot_stats->avg_ping + ((random() * 2) - 1) * 80;
			if (cl->ping < 0)
				cl->ping = 0;
		}

		// send the layout
		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i %i ",
			x, y, sorted[i], cl->resp.score, cl->ping + (int)(cl->latency + (random() * 25)), (level.framenum - cl->resp.enterframe)/600);
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


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard (edict_t *ent)
{
	DeathmatchScoreboardMessage (ent, ent->enemy);

	gi.unicast (ent, true);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;
//ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
//ZOID

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		return;
	}

	ent->client->showscores = true;
	DeathmatchScoreboard (ent);
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
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = false;

	if (ent->client->showhelp && (ent->client->resp.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->resp.helpchanged = 0;
	HelpComputer (ent);
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
	int			index, cells = 0;
	int			power_armor_type;

	//
	// health
	//
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
//	ent->client->ps.stats[STAT_MAXHEALTH] = min(max(ent->client->pers.max_health, 0), SHRT_MAX);
	ent->client->ps.stats[STAT_MAXHEALTH] = min(max(ent->max_health, 0), SHRT_MAX);
#endif

	//
	// ammo
	//
	if (!ent->client->ammo_index /* || !ent->client->pers.inventory[ent->client->ammo_index] */)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_MAXAMMO] = 0;
#endif
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_MAXAMMO] = min(max(GetMaxAmmoByIndex(ent->client, ent->client->ammo_index), 0), SHRT_MAX);
#endif
	}
	
	//
	// armor
	//
	power_armor_type = PowerArmorType (ent);
	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
		if (cells == 0)
		{	// ran out of cells for power armor
			ent->flags &= ~FL_POWER_ARMOR;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;;
		}
	}
	index = ArmorIndex (ent);
	if (power_armor_type && (!index || (level.framenum & 8) ) )
	{	// flash between power armor and other armor icon
		if (power_armor_type == POWER_ARMOR_SHIELD)
			ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powershield");
		else	// POWER_ARMOR_SCREEN
			ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powerscreen");
		ent->client->ps.stats[STAT_ARMOR] = cells;
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_MAXARMOR] = min(max(ent->client->pers.max_cells, 0), SHRT_MAX);
#endif
	}
	else if (index)
	{
		item = GetItemByIndex (index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_MAXARMOR] = min(max(ent->client->pers.max_armor, 0), SHRT_MAX);
#endif
	}
	else
	{
		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_MAXARMOR] = 0;
#endif
	}

	//
	// pickup message
	//
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	//
	// timers
	//
	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum)/10;
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max((int)sk_quad_time->value, 0), SHRT_MAX);
#endif
	}
	// RAFAEL
	else if (ent->client->quadfire_framenum > level.framenum)
	{
		// note to self
		// need to change imageindex
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quadfire");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quadfire_framenum - level.framenum)/10;
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max((int)sk_quad_fire_time->value, 0), SHRT_MAX);
#endif
	}
// PMM
	else if (ent->client->double_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_double");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->double_framenum - level.framenum)/10;
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max((int)sk_double_time->value, 0), SHRT_MAX);
#endif
	}
// PMM
	else if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum)/10;
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max((int)sk_inv_time->value, 0), SHRT_MAX);
#endif
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum)/10;
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max((int)sk_enviro_time->value, 0), SHRT_MAX);
#endif
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum)/10;
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max((int)sk_breather_time->value, 0), SHRT_MAX);
#endif
	}
// PGM
	else if (ent->client->owned_sphere)
	{
		int sphere_time;
		if(ent->client->owned_sphere->spawnflags == 1) {			// defender
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_defender");
			sphere_time = sk_defender_time->value;
		}
		else if(ent->client->owned_sphere->spawnflags == 2) {		// hunter
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_hunter");
			sphere_time = sk_hunter_time->value;
		}
		else if(ent->client->owned_sphere->spawnflags == 4) {		// vengeance
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_vengeance");
			sphere_time = sk_vengeance_time->value;
		}
		else {													// error case
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("i_fixme");
			sphere_time = 0;
		}
		ent->client->ps.stats[STAT_TIMER] = (int)(ent->client->owned_sphere->wait - level.time);
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max(sphere_time, 0), SHRT_MAX);
#endif
	}
	else if (ent->client->ir_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_ir");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->ir_framenum - level.framenum)/10;
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max((int)sk_ir_time->value, 0), SHRT_MAX);
#endif
	}
// PGM
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
#ifdef KMQUAKE2_ENGINE_MOD
		ent->client->ps.stats[STAT_TIMER_RANGE] = 0;
#endif
	}

	//
	// selected item
	//
	if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);

	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}

	//
	// frags
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	//
	// help icon / current weapon if not shown
	//
	if (ent->client->resp.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
	if (ent->client->pers.weapon)
		ent->client->ps.stats[STAT_WEAPON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_WEAPON] = 0;
#endif

//ZOID
	if (ctf->value)
		SetCTFStats (ent);
//ZOID

	// Knightmare- show tech icon if in DM
	if ( deathmatch->value && !(int)ctf->value && !(int)ttctf->value && ((int)def_hud->value == 1) )
	{
		int i = 0;
		gitem_t *tech;

	//	gi.dprintf ("Clearing STAT_CTF_TECH\n");
		ent->client->ps.stats[STAT_CTF_TECH] = 0;
		while (ctf_tnames[i]) {
			if ((tech = FindItemByClassname(ctf_tnames[i])) != NULL &&
				ent->client->pers.inventory[ITEM_INDEX(tech)])
			{
				ent->client->ps.stats[STAT_CTF_TECH] = gi.imageindex(tech->icon);
				break;
			}
			i++;
		}
	}
	// end Knightmare

// AJ
//	if (pci->value)
//		SetPCIdisplay(ent);
	if (!ent->bot_client && def_id->value /* ent->client->resp.id_state */ )
		CTFSetIDView (ent);

	if ( (!(level.framenum & 0x3)) || (level.framenum < 3) )
		lithium_updatestats (ent);	
// end AJ

}

