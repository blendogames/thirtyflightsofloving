/*
===========================================================================
Copyright (C) 1997-2001 Id Software, Inc.

This file is part of Quake 2 source code.

Quake 2 source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake 2 source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake 2 source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "g_local.h"
#include "m_player.h"

/*
// moved to g_ctf.h
typedef enum match_s {
	MATCH_NONE,
	MATCH_SETUP,
	MATCH_PREGAME,
	MATCH_GAME,
	MATCH_POST
} match_t;

typedef enum {
	ELECT_NONE,
	ELECT_MATCH,
	ELECT_ADMIN,
	ELECT_MAP
} elect_t;
*/

typedef struct ctfgame_s
{
	int team1, team2, team3; // Knightmare added
	int total1, total2, total3; // these are only set when going into intermission!
	float last_flag_capture;
	int last_capture_team;

	int team1_doublecapture_time; //ScarFace- for double captures with two carriers
	int team2_doublecapture_time;
	int team3_doublecapture_time;
	edict_t *team1_last_flag_capturer;
	edict_t *team2_last_flag_capturer;
	edict_t *team3_last_flag_capturer;

	match_t match;		// match state
	float matchtime;	// time for match start/end (depends on state)
	int lasttime;		// last time update
	qboolean countdown;	// has audio countdown started?

	elect_t election;	// election type
	edict_t *etarget;	// for admin election, who's being elected
	char elevel[32];	// for map election, target level
	int evotes;			// votes so far
	int needvotes;		// votes needed
	float electtime;	// remaining time until election times out
	char emsg[256];		// election name
	int warnactive; // true if stat string 30 is active

	ghost_t ghosts[MAX_CLIENTS]; // ghost codes
} ctfgame_t;

ctfgame_t ctfgame;

cvar_t *ctf;
cvar_t *ttctf; // Knightmare added
cvar_t *ctf_forcejoin;

cvar_t *competition;
cvar_t *matchlock;
cvar_t *electpercentage;
cvar_t *matchtime;
cvar_t *matchsetuptime;
cvar_t *matchstarttime;
cvar_t *admin_password;
cvar_t *allow_admin;
cvar_t *warp_list;
cvar_t *warn_unbalanced;

// Index for various CTF pics, this saves us from calling gi.imageindex
// all the time and saves a few CPU cycles since we don't have to do
// a bunch of string compares all the time.
// These are set in CTFPrecache() called from worldspawn
int imageindex_i_ctf1;
int imageindex_i_ctf2;
int imageindex_i_ctf1d;
int imageindex_i_ctf2d;
int imageindex_i_ctf1t;
int imageindex_i_ctf2t;
int imageindex_i_ctfj;
int imageindex_sbfctf1;
int imageindex_sbfctf2;
int imageindex_ctfsb1;
int imageindex_ctfsb2;
int imageindex_i_ctf3; // Knightmare added
int imageindex_i_ctf3d; // Knightmare added
int imageindex_i_ctf3t; // Knightmare added
int imageindex_sbfctf3; // Knightmare added
int imageindex_ctfsb3; // Knightmare added

char *ctf_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
  "xv 230 "
  "num 4 10 "
  "xv 296 "
  "pic 9 "
"endif "

// help / weapon icon 
"if 11 "
  "xv 148 "
  "pic 11 "
"endif "

// frags
"xr	-50 "
"yt 2 "
"num 3 14 "

// tech
"yb -129 "
"if 26 "
  "xr -26 "
  "pic 26 "
"endif "

// red team
"yb -102 "
"if 17 "
  "xr -26 "
  "pic 17 "
"endif "
"xr -62 "
"num 2 18 "
// joined overlay
"if 22 "
  "yb -104 "
  "xr -28 "
  "pic 22 "
"endif "

// blue team
"yb -75 "
"if 19 "
  "xr -26 "
  "pic 19 "
"endif "
"xr -62 "
"num 2 20 "
// joined overlay
"if 23 "
  "yb -77 "
  "xr -28 "
  "pic 23 "
"endif "

// have flag graph
"if 21 "
  "yt 26 "
  "xr -24 "
  "pic 21 "
"endif "

// id view state
"if 27 "
  "xv 112 "
  "yb -58 "
  "stat_string 27 "
"endif "

"if 29 "
  "xv 96 "
  "yb -58 "
  "pic 29 "
"endif "

"if 28 "
  "xl 0 "
  "yb -78 "
  "stat_string 28 "
"endif "

"if 30 "
  "xl 0 "
  "yb -88 "
  "stat_string 30 "
"endif "

// vehicle speed
"if 31 "
"	yb -90 "
"	xv 128 "
"	pic 31 "
"endif "
;


char *ttctf_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
  "xv 230 "
  "num 4 10 "
  "xv 296 "
  "pic 9 "
"endif "

// help / weapon icon 
"if 11 "
  "xv 148 "
  "pic 11 "
"endif "

// frags
"xr	-50 "
"yt 2 "
"num 3 14 "

// tech
"yb -129 "
"if 26 "
  "xr -26 "
  "pic 26 "
"endif "

// red team
"yb -104 "
"if 17 "
  "xr -28 "
  "pic 17 "
"endif "
"xr -62 "
"num 2 18 "
// joined overlay
"if 22 "
  "yb -104 "
  "xr -28 "
  "pic 22 "
"endif "

// blue team
"yb -77 "
"if 19 "
  "xr -28 "
  "pic 19 "
"endif "
"xr -62 "
"num 2 20 "
// joined overlay
"if 23 "
  "yb -77 "
  "xr -28 "
  "pic 23 "
"endif "

// green team
"yb -50 "
"if 32 "
  "xr -28 "
  "pic 32 "
"endif "
"xr -62 "
"num 2 33 "
// joined overlay
"if 34 "
  "yb -50 "
  "xr -28 "
  "pic 34 "
"endif "

// have flag graph
"if 21 "
  "yt 26 "
  "xr -26 "
  "pic 21 "
"endif "

// have flag graph2
"if 35 "
  "yt 26 "
  "xr -51 "
  "pic 35 "
"endif "

// id view state
"if 27 "
  "xv 112 "
  "yb -58 "
  "stat_string 27 "
"endif "

"if 29 "
  "xv 96 "
  "yb -58 "
  "pic 29 "
"endif "

"if 28 "
  "xl 0 "
  "yb -78 "
  "stat_string 28 "
"endif "

"if 30 "
  "xl 0 "
  "yb -88 "
  "stat_string 30 "
"endif "

// vehicle speed
"if 31 "
"	yb -90 "
"	xv 128 "
"	pic 31 "
"endif "
;

#define TECHTYPES 6

static char *tnames[] = {
	"item_tech1", "item_tech2", "item_tech3", "item_tech4", "item_tech5", "item_tech6",
	NULL
};

/*void stuffcmd(edict_t *ent, char *s) 	
{
   	gi.WriteByte (11);	        
	gi.WriteString (s);
    gi.unicast (ent, true);	
}*/

/*--------------------------------------------------------------------------*/

/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
static edict_t *loc_findradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
#if 0
		if (from->solid == SOLID_NOT)
			continue;
#endif
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}

/*static*/ void loc_buildboxpoints (vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
{
	VectorAdd(org, mins, p[0]);
	VectorCopy(p[0], p[1]);
	p[1][0] -= mins[0];
	VectorCopy(p[0], p[2]);
	p[2][1] -= mins[1];
	VectorCopy(p[0], p[3]);
	p[3][0] -= mins[0];
	p[3][1] -= mins[1];
	VectorAdd(org, maxs, p[4]);
	VectorCopy(p[4], p[5]);
	p[5][0] -= maxs[0];
	VectorCopy(p[0], p[6]);
	p[6][1] -= maxs[1];
	VectorCopy(p[0], p[7]);
	p[7][0] -= maxs[0];
	p[7][1] -= maxs[1];
}

/*static*/ qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	int i;
	vec3_t viewpoint;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
		return false; // bmodels not supported

	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);
	
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;

	for (i = 0; i < 8; i++) {
		trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
	}

	return false;
}

/*--------------------------------------------------------------------------*/

static gitem_t *flag1_item;
static gitem_t *flag2_item;
static gitem_t *flag3_item; // Knightmare added

void CTFSpawn (void)
{
	if (!ctf->value)
		return;

	if (!flag1_item)
		flag1_item = FindItemByClassname("item_flag_team1");
	if (!flag2_item)
		flag2_item = FindItemByClassname("item_flag_team2");
	if (!flag3_item)
		flag3_item = FindItemByClassname("item_flag_team3");
	memset(&ctfgame, 0, sizeof(ctfgame));
	CTFSetupTechSpawn();

	if (competition->value > 1) {
		ctfgame.match = MATCH_SETUP;
		ctfgame.matchtime = level.time + matchsetuptime->value * 60;
	}
}

void CTFInit (void)
{
	ctf = gi.cvar("ctf", "0", CVAR_SERVERINFO|CVAR_LATCH);
	ttctf = gi.cvar("ttctf", "0", CVAR_SERVERINFO|CVAR_LATCH); // Knightmare added
	ctf_forcejoin = gi.cvar("ctf_forcejoin", "", 0);
	competition = gi.cvar("competition", "0", CVAR_SERVERINFO);
	matchlock = gi.cvar("matchlock", "1", CVAR_SERVERINFO);
	electpercentage = gi.cvar("electpercentage", "66", 0);
	matchtime = gi.cvar("matchtime", "20", CVAR_SERVERINFO);
	matchsetuptime = gi.cvar("matchsetuptime", "10", 0);
	matchstarttime = gi.cvar("matchstarttime", "20", 0);
	admin_password = gi.cvar("admin_password", "", 0);
	allow_admin = gi.cvar("allow_admin", "1", 0);
	warp_list = gi.cvar("warp_list", "q2ctf1 q2ctf2 q2ctf3 q2ctf4 q2ctf5", 0);
	warn_unbalanced = gi.cvar("warn_unbalanced", "1", 0);
}

/*
 * Precache CTF items
 */

void CTFPrecache (void)
{
	if (ttctf->value)
	{
		imageindex_i_ctf1 =   gi.imageindex("3tctfr"); 
		imageindex_i_ctf2 =   gi.imageindex("3tctfb"); 
		imageindex_i_ctf3 =   gi.imageindex("3tctfg"); 
		imageindex_i_ctf1d =  gi.imageindex("3tctfrd");
		imageindex_i_ctf2d =  gi.imageindex("3tctfbd");
		imageindex_i_ctf3d =  gi.imageindex("3tctfgd");
		imageindex_i_ctf1t =  gi.imageindex("3tctfrt");
		imageindex_i_ctf2t =  gi.imageindex("3tctfbt");
		imageindex_i_ctf3t =  gi.imageindex("3tctfgt");
		imageindex_i_ctfj =   gi.imageindex("i_ctfj"); 
		imageindex_sbfctf1 =  gi.imageindex("sbfctf1");
		imageindex_sbfctf2 =  gi.imageindex("sbfctf2");
		imageindex_sbfctf3 =  gi.imageindex("sbfctf3");
		imageindex_ctfsb1 =   gi.imageindex("3tctfsb1");
		imageindex_ctfsb2 =   gi.imageindex("3tctfsb2");
		imageindex_ctfsb3 =   gi.imageindex("3tctfsb3");
	}
	else
	{
		imageindex_i_ctf1 =   gi.imageindex("i_ctf1"); 
		imageindex_i_ctf2 =   gi.imageindex("i_ctf2"); 
		imageindex_i_ctf1d =  gi.imageindex("i_ctf1d");
		imageindex_i_ctf2d =  gi.imageindex("i_ctf2d");
		imageindex_i_ctf1t =  gi.imageindex("i_ctf1t");
		imageindex_i_ctf2t =  gi.imageindex("i_ctf2t");
		imageindex_i_ctfj =   gi.imageindex("i_ctfj"); 
		imageindex_sbfctf1 =  gi.imageindex("sbfctf1");
		imageindex_sbfctf2 =  gi.imageindex("sbfctf2");
		imageindex_ctfsb1 =   gi.imageindex("ctfsb1");
		imageindex_ctfsb2 =   gi.imageindex("ctfsb2");
	}
}

/*--------------------------------------------------------------------------*/

char *CTFTeamName (int team)
{
	switch (team) {
	case CTF_TEAM1:
		return "RED";
	case CTF_TEAM2:
		return "BLUE";
	case CTF_TEAM3:
		return "GREEN"; // Knightmare added
	}
	return "UNKNOWN"; // Hanzo pointed out this was spelled wrong as "UKNOWN"
}

//Knightmare added
int PlayersOnCTFTeam (int checkteam)
{
	int i, total = 0;
	for (i = 0; i < maxclients->value; i++)
	{
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == checkteam)
			total++;
	}
	return total;
}

// Knightmare added
int CTFFlagTeam (gitem_t *flag)
{
	if (flag == flag1_item)
		return CTF_TEAM1;
	else if (flag == flag2_item)
		return CTF_TEAM2;
	else if (flag == flag3_item)
		return CTF_TEAM3;
	return CTF_NOTEAM; // invalid flag
}

char *CTFOtherTeamName (int team)
{
	switch (team) {
	case CTF_TEAM1:
		return "BLUE";
	case CTF_TEAM2:
		return "RED";
	case CTF_TEAM3: // Knightmare added
		return "RED";
	}
	return "UNKNOWN"; // Hanzo pointed out this was spelled wrong as "UKNOWN"
}

// Knightmare added
char *CTFOtherTeamName2 (int team)
{
	switch (team) {
	case CTF_TEAM1:
		return "GREEN";
	case CTF_TEAM2:
		return "GREEN";
	case CTF_TEAM3: // Knightmare added
		return "BLUE";
	}
	return "UNKNOWN"; // Hanzo pointed out this was spelled wrong as "UKNOWN"
}

int CTFOtherTeam (int team)
{
	switch (team) {
	case CTF_TEAM1:
		return CTF_TEAM2;
	case CTF_TEAM2:
		return CTF_TEAM1;
	case CTF_TEAM3: // Knightmare added
		return CTF_TEAM1;
	}
	return -1; // invalid value
}

int CTFOtherTeam2 (int team)
{
	switch (team) {
	case CTF_TEAM1:
		return CTF_TEAM3;
	case CTF_TEAM2:
		return CTF_TEAM3;
	case CTF_TEAM3: // Knightmare added
		return CTF_TEAM2;
	}
	return -1; // invalid value
}

/*--------------------------------------------------------------------------*/

edict_t *SelectRandomDeathmatchSpawnPoint (void);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);
float	PlayersRangeFromSpot (edict_t *spot);

void CTFAssignSkin (edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char *p;
	char t[64];

	Com_sprintf (t, sizeof(t), "%s", s);

	if ((p = strchr(t, '/')) != NULL)
		p[1] = 0;
	else
	//	strncpy(t, "male/");
		Q_strncpyz(t, sizeof(t), "male/");

	switch (ent->client->resp.ctf_team) {
	case CTF_TEAM1:
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
			ent->client->pers.netname, t, CTF_TEAM1_SKIN) );
		break;
	case CTF_TEAM2:
		gi.configstring (CS_PLAYERSKINS+playernum,
			va("%s\\%s%s", ent->client->pers.netname, t, CTF_TEAM2_SKIN) );
		break;
	case CTF_TEAM3: // Knightmare added
		gi.configstring (CS_PLAYERSKINS+playernum,
			va("%s\\%s%s", ent->client->pers.netname, t, CTF_TEAM3_SKIN) );
		break;
	default:
		gi.configstring (CS_PLAYERSKINS+playernum, 
			va("%s\\%s", ent->client->pers.netname, s) );
		break;
	}
//	safe_cprintf(ent, PRINT_HIGH, "You have been assigned to %s team.\n", ent->client->pers.netname);
}

void CTFAssignTeam (gclient_t *who)
{
	edict_t		*player;
	int i;
	int team1count = 0, team2count = 0, team3count = 0;

	who->resp.ctf_state = 0;

	if (!((int)dmflags->value & DF_CTF_FORCEJOIN)) {
		who->resp.ctf_team = CTF_NOTEAM;
		return;
	}

	for (i = 1; i <= maxclients->value; i++) {
		player = &g_edicts[i];

		if (!player->inuse || player->client == who)
			continue;

		switch (player->client->resp.ctf_team) {
		case CTF_TEAM1:
			team1count++;
			break;
		case CTF_TEAM2:
			team2count++;
			break;
		case CTF_TEAM3: // Knightmare added
			team3count++;
			break;
		}
	}

	if (ttctf->value) // Knightmare added
	{
		float	r = random();
		// find team with the fewest players
		if ( (team1count < team2count) && (team1count < team3count) )
			who->resp.ctf_team = CTF_TEAM1;
		else if ( (team2count < team1count) && (team2count < team3count) ) 
			who->resp.ctf_team = CTF_TEAM2;
		else if ( (team3count < team1count) && (team3count < team2count) ) 
			who->resp.ctf_team = CTF_TEAM3;
		// else select a random team
		else if (r < 0.33)
			who->resp.ctf_team = CTF_TEAM1;
		else if (r < 0.67)
			who->resp.ctf_team = CTF_TEAM2;
		else
			who->resp.ctf_team = CTF_TEAM3;
	}
	else
	{
		float	r = random();
		if (team1count < team2count)
			who->resp.ctf_team = CTF_TEAM1;
		else if (team2count < team1count)
			who->resp.ctf_team = CTF_TEAM2;
		else if (r < 0.5)
			who->resp.ctf_team = CTF_TEAM1;
		else
			who->resp.ctf_team = CTF_TEAM2;
	}

}

/*
================
SelectCTFSpawnPoint

go to a ctf point, but NOT the two points closest
to other players
================
*/
edict_t *SelectCTFSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char	*cname;

	if (ent->client->resp.ctf_state)
		if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
			return SelectFarthestDeathmatchSpawnPoint ();
		else
			return SelectRandomDeathmatchSpawnPoint ();

	ent->client->resp.ctf_state++;

	switch (ent->client->resp.ctf_team) {
	case CTF_TEAM1:
		cname = "info_player_team1";
		break;
	case CTF_TEAM2:
		cname = "info_player_team2";
		break;
	case CTF_TEAM3: // Knightmare added
		cname = "info_player_team3";
		break;
	default:
		return SelectRandomDeathmatchSpawnPoint();
	}

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), cname)) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return SelectRandomDeathmatchSpawnPoint();

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), cname);
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*------------------------------------------------------------------------*/
/*
CTFFragBonuses

Calculate the bonuses for flag defense, flag carrier defense, etc.
Note that bonuses are not cumaltive.  You get one, they are in importance
order.
*/
void CTFFragBonuses (edict_t *targ, edict_t *inflictor, edict_t *attacker)
{
	int i;
	edict_t *ent;
	gitem_t *flag_item, *enemy_flag_item, *enemy_flag_item2; // Knightmare added
	int otherteam, otherteam2; // Knightmare added
	edict_t *flag, *carrier;
	char *c;
	vec3_t v1, v2;

	if (targ->client && attacker->client) {
		if (attacker->client->resp.ghost)
			if (attacker != targ)
				attacker->client->resp.ghost->kills++;
		if (targ->client->resp.ghost)
			targ->client->resp.ghost->deaths++;
	}

	// no bonus for fragging yourself
	if (!targ->client || !attacker->client || targ == attacker)
		return;

	otherteam = CTFOtherTeam(targ->client->resp.ctf_team);
	otherteam2 = CTFOtherTeam2(targ->client->resp.ctf_team);
	if (otherteam < 0)
		return; // whoever died isn't on a team

	// same team, if the flag at base, check to he has the enemy flag
	if (targ->client->resp.ctf_team == CTF_TEAM1) {
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
		enemy_flag_item2 = flag3_item;
	}
	// Knightmare added
	else if (targ->client->resp.ctf_team == CTF_TEAM2) {
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
		enemy_flag_item2 = flag3_item;
	}
	else {
		flag_item = flag3_item;
		enemy_flag_item = flag1_item;
		enemy_flag_item2 = flag2_item;
	}

	// did the attacker frag the flag carrier?
	if ( (targ->client->pers.inventory[ITEM_INDEX(enemy_flag_item)]
			&& attacker->client->resp.ctf_team == CTFFlagTeam(enemy_flag_item))
		|| (targ->client->pers.inventory[ITEM_INDEX(enemy_flag_item2)]
			&& attacker->client->resp.ctf_team == CTFFlagTeam(enemy_flag_item2)) )
	{
		attacker->client->resp.ctf_lastfraggedcarrier = level.time;
		attacker->client->resp.score += CTF_FRAG_CARRIER_BONUS;
		safe_cprintf(attacker, PRINT_MEDIUM, "BONUS: %d points for fragging enemy flag carrier.\n",
			CTF_FRAG_CARRIER_BONUS);

		// the target had the flag, clear the hurt carrier
		// field on the other team
		for (i = 1; i <= maxclients->value; i++)
		{
			ent = g_edicts + i;
			if (ent->inuse
				&& (ent->client->resp.ctf_team == otherteam || ent->client->resp.ctf_team == otherteam2) )
				ent->client->resp.ctf_lasthurtcarrier = 0;
		}
		return;
	}

	if (targ->client->resp.ctf_lasthurtcarrier &&
		level.time - targ->client->resp.ctf_lasthurtcarrier < CTF_CARRIER_DANGER_PROTECT_TIMEOUT
		&& !attacker->client->pers.inventory[ITEM_INDEX(flag_item)])
	{
		// attacker is on the same team as the flag carrier and
		// fragged a guy who hurt our flag carrier
		attacker->client->resp.score += CTF_CARRIER_DANGER_PROTECT_BONUS;
		safe_bprintf(PRINT_MEDIUM, "%s defends %s's flag carrier against an agressive enemy\n",
			attacker->client->pers.netname, 
			CTFTeamName(attacker->client->resp.ctf_team));
		if (attacker->client->resp.ghost)
			attacker->client->resp.ghost->carrierdef++;
		return;
	}

	// flag and flag carrier area defense bonuses

	// we have to find the flag and carrier entities

	// find the flag
	switch (attacker->client->resp.ctf_team) {
	case CTF_TEAM1:
		c = "item_flag_team1";
		break;
	case CTF_TEAM2:
		c = "item_flag_team2";
		break;
	case CTF_TEAM3: // Knightmare added
		c = "item_flag_team3";
		break;
	default:
		return;
	}

	flag = NULL;
	while ((flag = G_Find (flag, FOFS(classname), c)) != NULL) {
		if (!(flag->spawnflags & DROPPED_ITEM))
			break;
	}

	if (!flag)
		return; // can't find attacker's flag

	// find attacker's team's flag carrier
	for (i = 1; i <= maxclients->value; i++) {
		carrier = g_edicts + i;
		if (carrier->inuse && 
			carrier->client->pers.inventory[ITEM_INDEX(flag_item)])
			break;
		carrier = NULL;
	}

	// ok we have the attackers flag and a pointer to the carrier

	// check to see if we are defending the base's flag
	VectorSubtract(targ->s.origin, flag->s.origin, v1);
	VectorSubtract(attacker->s.origin, flag->s.origin, v2);

	if ((VectorLength(v1) < CTF_TARGET_PROTECT_RADIUS ||
		VectorLength(v2) < CTF_TARGET_PROTECT_RADIUS ||
		loc_CanSee(flag, targ) || loc_CanSee(flag, attacker)) &&
		attacker->client->resp.ctf_team != targ->client->resp.ctf_team) {
		// we defended the base flag
		attacker->client->resp.score += CTF_FLAG_DEFENSE_BONUS;
		if (flag->solid == SOLID_NOT)
			safe_bprintf(PRINT_MEDIUM, "%s defends the %s base.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.ctf_team));
		else
			safe_bprintf(PRINT_MEDIUM, "%s defends the %s flag.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.ctf_team));
		if (attacker->client->resp.ghost)
			attacker->client->resp.ghost->basedef++;
		return;
	}

	if (carrier && carrier != attacker) {
		VectorSubtract(targ->s.origin, carrier->s.origin, v1);
		VectorSubtract(attacker->s.origin, carrier->s.origin, v1);

		if (VectorLength(v1) < CTF_ATTACKER_PROTECT_RADIUS ||
			VectorLength(v2) < CTF_ATTACKER_PROTECT_RADIUS ||
			loc_CanSee(carrier, targ) || loc_CanSee(carrier, attacker)) {
			attacker->client->resp.score += CTF_CARRIER_PROTECT_BONUS;
			safe_bprintf(PRINT_MEDIUM, "%s defends the %s's flag carrier.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.ctf_team));
			if (attacker->client->resp.ghost)
				attacker->client->resp.ghost->carrierdef++;
			return;
		}
	}
}

void CTFCheckHurtCarrier (edict_t *targ, edict_t *attacker)
{
	gitem_t *flag_item1, *flag_item2;

	if (!ctf->value)
		return;

	if (!targ || !attacker)
		return;

	if (!targ->client || !attacker->client)
		return;

	if (targ->client->resp.ctf_team == CTF_TEAM1) {
		flag_item1 = flag2_item;
		flag_item2 = flag3_item;
	}
	// Knightmare added
	else if (targ->client->resp.ctf_team == CTF_TEAM2) {
		flag_item1 = flag1_item;
		flag_item2 = flag3_item;
	}
	else if (targ->client->resp.ctf_team == CTF_TEAM3) {
		flag_item1 = flag1_item;
		flag_item2 = flag2_item;
	}

	if ( ( targ->client->pers.inventory[ITEM_INDEX(flag_item1)]
		 || targ->client->pers.inventory[ITEM_INDEX(flag_item2)] ) // Knightmare added
		&& targ->client->resp.ctf_team != attacker->client->resp.ctf_team)
		attacker->client->resp.ctf_lasthurtcarrier = level.time;
}


/*------------------------------------------------------------------------*/

void CTFResetFlag (int ctf_team)
{
	char *c;
	edict_t *ent;

	switch (ctf_team) {
	case CTF_TEAM1:
		c = "item_flag_team1";
		break;
	case CTF_TEAM2:
		c = "item_flag_team2";
		break;
	case CTF_TEAM3: // Knightmare added
		c = "item_flag_team3";
		break;
	default:
		return;
	}

	ent = NULL;
	while ((ent = G_Find (ent, FOFS(classname), c)) != NULL) {
		if (ent->spawnflags & DROPPED_ITEM)
			G_FreeEdict(ent);
		else {
			ent->svflags &= ~SVF_NOCLIENT;
			ent->solid = SOLID_TRIGGER;
			gi.linkentity(ent);
			ent->s.event = EV_ITEM_RESPAWN;
		}
	}
}

void CTFResetFlags (void)
{
	CTFResetFlag(CTF_TEAM1);
	CTFResetFlag(CTF_TEAM2);
	CTFResetFlag(CTF_TEAM3); // Knightmare added
}

qboolean CTFPickup_Flag (edict_t *ent, edict_t *other)
{
	int ctf_team;
	int i, captures = 0;
	edict_t *player;
	gitem_t *flag_item, *enemy_flag_item1, *enemy_flag_item2, *captured_flag_item;

	// figure out what team this flag is
	if (strcmp(ent->classname, "item_flag_team1") == 0)
		ctf_team = CTF_TEAM1;
	else if (strcmp(ent->classname, "item_flag_team2") == 0)
		ctf_team = CTF_TEAM2;
	// Knightmare added
	else if (strcmp(ent->classname, "item_flag_team3") == 0)
		ctf_team = CTF_TEAM3;
	else {
		safe_cprintf(ent, PRINT_HIGH, "Don't know what team the flag is on.\n");
		return false;
	}

	// same team, if the flag at base, check to he has the enemy flag
	if (ctf_team == CTF_TEAM1) {
		flag_item = flag1_item;
		enemy_flag_item1 = flag2_item;
		enemy_flag_item2 = flag3_item;
	// Knightmare added
	} else if (ctf_team == CTF_TEAM2) {
		flag_item = flag2_item;
		enemy_flag_item1 = flag1_item;
		enemy_flag_item2 = flag3_item;
	} else {
		flag_item = flag3_item;
		enemy_flag_item1 = flag1_item;
		enemy_flag_item2 = flag2_item;
	}

	if (ctf_team == other->client->resp.ctf_team)
	{	// if returning to our flag
		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			// the flag is at home base.  if the player has the enemy
			// flag, he's just scored!
			if (other->client->pers.inventory[ITEM_INDEX(enemy_flag_item1)]
				|| other->client->pers.inventory[ITEM_INDEX(enemy_flag_item2)])
			{
				if (other->client->pers.inventory[ITEM_INDEX(enemy_flag_item1)])
				{
					if (!ttctf->value)
						safe_bprintf(PRINT_HIGH, "%s captured the %s flag!\n",
								other->client->pers.netname, CTFTeamName(CTFFlagTeam(enemy_flag_item1)) );
					other->client->pers.inventory[ITEM_INDEX(enemy_flag_item1)] = 0;

					captured_flag_item = enemy_flag_item1;
					ctfgame.last_flag_capture = level.time;
					ctfgame.last_capture_team = ctf_team;
					if (ctf_team == CTF_TEAM1)
						ctfgame.team1++;
					// Knightmare added
					else if (ctf_team == CTF_TEAM2)
						ctfgame.team2++;
					else if (ctf_team == CTF_TEAM3)
						ctfgame.team3++;
					captures++;
					if (other->client->resp.ghost)
						other->client->resp.ghost->caps++;
					CTFResetFlag(CTFFlagTeam(enemy_flag_item1));
				}
				if (ttctf->value && other->client->pers.inventory[ITEM_INDEX(enemy_flag_item2)])
				{
					//safe_bprintf(PRINT_HIGH, "%s captured the %s flag!\n",
					//		other->client->pers.netname, CTFTeamName(CTFFlagTeam(enemy_flag_item2)) );
					other->client->pers.inventory[ITEM_INDEX(enemy_flag_item2)] = 0;

					captured_flag_item = enemy_flag_item2;
					ctfgame.last_flag_capture = level.time;
					ctfgame.last_capture_team = ctf_team;
					if (ctf_team == CTF_TEAM1)
						ctfgame.team1++;
					// Knightmare added
					else if (ctf_team == CTF_TEAM2)
						ctfgame.team2++;
					else if (ctf_team == CTF_TEAM3)
						ctfgame.team3++;
					captures++;
					if (other->client->resp.ghost)
						other->client->resp.ghost->caps++;
					CTFResetFlag(CTFFlagTeam(enemy_flag_item2));
				}

				gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagcap.wav"), 1, ATTN_NONE, 0);

				//ScarFace- double capture detection
				if (captures == 2) { // other gets 40 frag bonus
					other->client->resp.score += CTF_DOUBLE_CAPTURE_BONUS;
					safe_bprintf(PRINT_HIGH, "%s captured the %s and %s flags for a double capture!\n",
							other->client->pers.netname, CTFTeamName(CTFFlagTeam(enemy_flag_item1)),
							CTFTeamName(CTFFlagTeam(enemy_flag_item2)) );
				}
				else // other gets 15 frag bonus
					other->client->resp.score += CTF_CAPTURE_BONUS;

				//ScarFace- support for 2-carrier double capture in 3Team CTF mode
				//2 captures must be made within 20 seconds of each other
				//Each carrier gets 30 points
				//Carrier 1: CTF_CAPTURE_BONUS 15pts + CTF_TEAM_BONUS 10pts + 5pts = 30pts total
				//Carrier 2: CTF_TEAM_BONUS 10pts + CTF_CAPTURE_BONUS 15pts + 5pts = 30pts total
				//FIXME- how to check if the same flag has been captured twice?
				if (ttctf->value && ctf_team == CTF_TEAM1 && captures == 1)
				{
					if (ctfgame.team1_doublecapture_time
						&& ctfgame.team1_doublecapture_time > level.time
						&& other != ctfgame.team1_last_flag_capturer)
					{
						safe_bprintf(PRINT_HIGH, "%s captured the %s flag for a double capture!\n",
							other->client->pers.netname, CTFTeamName(CTFFlagTeam(captured_flag_item)) );
						other->client->resp.score += 5;
						if (ctfgame.team1_last_flag_capturer && ctfgame.team1_last_flag_capturer->client)
							ctfgame.team1_last_flag_capturer->client->resp.score += 5;
						ctfgame.team1_doublecapture_time = 0;
						ctfgame.team1_last_flag_capturer = other;
					}
					else
					{
					//	my_bprintf(PRINT_HIGH, "%s started the double capture timer!\n", other->client->pers.netname);
						safe_bprintf(PRINT_HIGH, "%s captured the %s flag!\n",
							other->client->pers.netname, CTFTeamName(CTFFlagTeam(captured_flag_item)) );
						ctfgame.team1_doublecapture_time = level.time + CTF_DOUBLE_CAPTURE_TIMEOUT;
						ctfgame.team1_last_flag_capturer = other;
					}
				}
				else if (ttctf->value && ctf_team == CTF_TEAM2 && captures == 1)
				{
					if (ctfgame.team2_doublecapture_time
						&& ctfgame.team2_doublecapture_time > level.time
						&& other != ctfgame.team2_last_flag_capturer)
					{
						safe_bprintf(PRINT_HIGH, "%s captured the %s flag for a double capture!\n",
							other->client->pers.netname, CTFTeamName(CTFFlagTeam(captured_flag_item)) );
						other->client->resp.score += 5;
						if (ctfgame.team2_last_flag_capturer && ctfgame.team2_last_flag_capturer->client)
							ctfgame.team2_last_flag_capturer->client->resp.score += 5;
						ctfgame.team2_doublecapture_time = 0;
						ctfgame.team2_last_flag_capturer = other;
					}
					else
					{
					//	my_bprintf(PRINT_HIGH, "%s started the double capture timer!\n", other->client->pers.netname);
						safe_bprintf(PRINT_HIGH, "%s captured the %s flag!\n",
							other->client->pers.netname, CTFTeamName(CTFFlagTeam(captured_flag_item)) );
						ctfgame.team2_doublecapture_time = level.time + CTF_DOUBLE_CAPTURE_TIMEOUT;
						ctfgame.team2_last_flag_capturer = other;
					}
				}
				else if (ttctf->value && ctf_team == CTF_TEAM3 && captures == 1)
				{
					if (ctfgame.team3_doublecapture_time
						&& ctfgame.team3_doublecapture_time > level.time
						&& other != ctfgame.team3_last_flag_capturer)
					{
						safe_bprintf(PRINT_HIGH, "%s captured the %s flag for a double capture!\n",
							other->client->pers.netname, CTFTeamName(CTFFlagTeam(captured_flag_item)) );
						other->client->resp.score += 5;
						if (ctfgame.team3_last_flag_capturer && ctfgame.team3_last_flag_capturer->client)
							ctfgame.team3_last_flag_capturer->client->resp.score += 5;
						ctfgame.team3_doublecapture_time = 0;
						ctfgame.team3_last_flag_capturer = other;
					}
					else
					{
					//	my_bprintf(PRINT_HIGH, "%s started the double capture timer!\n", other->client->pers.netname);
						safe_bprintf(PRINT_HIGH, "%s captured the %s flag!\n",
							other->client->pers.netname, CTFTeamName(CTFFlagTeam(captured_flag_item)) );
						ctfgame.team3_doublecapture_time = level.time + CTF_DOUBLE_CAPTURE_TIMEOUT;
						ctfgame.team3_last_flag_capturer = other;
					}
				}
				// end ScarFace


				// Ok, let's do the player loop, hand out the bonuses
				for (i = 1; i <= maxclients->value; i++)
				{
					player = &g_edicts[i];
					if (!player->inuse)
						continue;

					if (player->client->resp.ctf_team != other->client->resp.ctf_team)
						player->client->resp.ctf_lasthurtcarrier = -5;
					else if (player->client->resp.ctf_team == other->client->resp.ctf_team)
					{
						if (player != other)
							player->client->resp.score += captures*CTF_TEAM_BONUS;
						// award extra points for capture assists
						if (player->client->resp.ctf_lastreturnedflag + CTF_RETURN_FLAG_ASSIST_TIMEOUT > level.time) {
							safe_bprintf(PRINT_HIGH, "%s gets an assist for returning the flag!\n", player->client->pers.netname);
							player->client->resp.score += CTF_RETURN_FLAG_ASSIST_BONUS;
						}
						if (player->client->resp.ctf_lastfraggedcarrier + CTF_FRAG_CARRIER_ASSIST_TIMEOUT > level.time) {
							safe_bprintf(PRINT_HIGH, "%s gets an assist for fragging the flag carrier!\n", player->client->pers.netname);
							player->client->resp.score += CTF_FRAG_CARRIER_ASSIST_BONUS;
						}
					}
				}
			}
			return false; // its at home base already
		}

		// hey, its not home.  return it by teleporting it back
		safe_bprintf(PRINT_HIGH, "%s returned the %s flag!\n", 
			other->client->pers.netname, CTFTeamName(ctf_team));
		other->client->resp.score += CTF_RECOVERY_BONUS;
		other->client->resp.ctf_lastreturnedflag = level.time;
		gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagret.wav"), 1, ATTN_NONE, 0);
		//CTFResetFlag will remove this entity!  We must return false
		CTFResetFlag(ctf_team);
		return false;
	}

	// hey, its not our flag, pick it up

	// Knightmare- added disable pickup option
	if (!allow_flagpickup->value && PlayersOnCTFTeam (ctf_team) == 0) {
		if (level.time - other->client->ctf_lasttechmsg > 2) {
			safe_centerprintf(other, "Not allowed to take empty teams' flags!");
			other->client->ctf_lasttechmsg = level.time;
		}
		return false;
	}

	safe_bprintf(PRINT_HIGH, "%s got the %s flag!\n",
		other->client->pers.netname, CTFTeamName(ctf_team));
	other->client->resp.score += CTF_FLAG_BONUS;

	other->client->pers.inventory[ITEM_INDEX(flag_item)] = 1;
	other->client->resp.ctf_flagsince = level.time;

	// pick up the flag
	// if it's not a dropped flag, we just make is disappear
	// if it's dropped, it will be removed by the pickup caller
	if (!(ent->spawnflags & DROPPED_ITEM)) {
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	return true;
}

gitem_t *CTFWhat_Flag (edict_t *ent)
{
	gitem_t *flag;

	if ((flag = FindItemByClassname("item_flag_team1")) != NULL &&
		ent->client->pers.inventory[ITEM_INDEX(flag)])
		return flag;

	if ((flag = FindItemByClassname("item_flag_team2")) != NULL &&
		ent->client->pers.inventory[ITEM_INDEX(flag)])
		return flag;
	// Knightmare added
	if ((flag = FindItemByClassname("item_flag_team3")) != NULL &&
		ent->client->pers.inventory[ITEM_INDEX(flag)])
		return flag;

	return NULL;
}

/*static*/ void CTFDropFlagTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//owner (who dropped us) can't touch for two secs
	if ( (other == ent->owner) && 
		( (ent->nextthink - level.time > CTF_AUTO_FLAG_RETURN_TIMEOUT-2)
		 || (level.time < ent->touch_debounce_time) ) )
		return;

	Touch_Item (ent, other, plane, surf);
}

/*static*/ void CTFDropFlagThink (edict_t *ent)
{
	// auto return the flag
	// reset flag will remove ourselves
	if (strcmp(ent->classname, "item_flag_team1") == 0)
	{
		CTFResetFlag(CTF_TEAM1);
		safe_bprintf(PRINT_HIGH, "The %s flag has returned!\n",
			CTFTeamName(CTF_TEAM1));
	}
	else if (strcmp(ent->classname, "item_flag_team2") == 0)
	{
		CTFResetFlag(CTF_TEAM2);
		safe_bprintf(PRINT_HIGH, "The %s flag has returned!\n",
			CTFTeamName(CTF_TEAM2));
	}
	// Knightmare added
	else if (strcmp(ent->classname, "item_flag_team3") == 0)
	{
		CTFResetFlag(CTF_TEAM3);
		safe_bprintf(PRINT_HIGH, "The %s flag has returned!\n",
			CTFTeamName(CTF_TEAM3));
	}
}

// Called from PlayerDie, to drop the flag from a dying player
void CTFDeadDropFlag (edict_t *self)
{
	edict_t *dropped = NULL;

	if (!ctf->value)
		return;

	if (self->client->pers.inventory[ITEM_INDEX(flag1_item)])
	{
		dropped = Drop_Item(self, flag1_item);
		self->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
		safe_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM1));
		if (dropped) {
			dropped->think = CTFDropFlagThink;
			dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
			dropped->touch = CTFDropFlagTouch;
		}
	}
	if (self->client->pers.inventory[ITEM_INDEX(flag2_item)])
	{
		dropped = Drop_Item(self, flag2_item);
		self->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
		safe_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM2));
		if (dropped) {
			dropped->think = CTFDropFlagThink;
			dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
			dropped->touch = CTFDropFlagTouch;
		}
	}
	// Knightmare added
	if (self->client->pers.inventory[ITEM_INDEX(flag3_item)])
	{
		dropped = Drop_Item(self, flag3_item);
		self->client->pers.inventory[ITEM_INDEX(flag3_item)] = 0;
		safe_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM3));
		if (dropped) {
			dropped->think = CTFDropFlagThink;
			dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
			dropped->touch = CTFDropFlagTouch;
		}
	}
}

qboolean CTFDrop_Flag (edict_t *ent, gitem_t *item)
{
	edict_t *dropped = NULL;

	// Knightmare changed this
	if (!allow_flagdrop->value)
	{
		if (rand() & 1) 
			safe_cprintf(ent, PRINT_HIGH, "Only llamas drop flags.\n");
		else
			safe_cprintf(ent, PRINT_HIGH, "Winners don't drop flags.\n");
		return false;
	}
	else
	{
		if (ent->client->pers.inventory[ITEM_INDEX(flag1_item)]) 
		{
			dropped = Drop_Item(ent, flag1_item);
			ent->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
			safe_bprintf(PRINT_HIGH, "%s dropped the RED flag!\n", ent->client->pers.netname);
			if (dropped) {
				// hack the velocity to make it bounce random
				dropped->velocity[0] = (rand() % 600) - 300;
				dropped->velocity[1] = (rand() % 600) - 300;
				dropped->think = CTFDropFlagThink;
				dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
				dropped->touch = CTFDropFlagTouch;
				dropped->timestamp = level.time;
				dropped->touch_debounce_time = level.time + 1.0;
				dropped->owner = ent; 
			}
		}
		if (ent->client->pers.inventory[ITEM_INDEX(flag2_item)]) 
		{
			dropped = Drop_Item(ent, flag2_item);
			ent->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
			safe_bprintf(PRINT_HIGH, "%s dropped the BLUE flag!\n", ent->client->pers.netname);
			if (dropped) {
				// hack the velocity to make it bounce random
				dropped->velocity[0] = (rand() % 600) - 300;
				dropped->velocity[1] = (rand() % 600) - 300;
				dropped->think = CTFDropFlagThink;
				dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
				dropped->touch = CTFDropFlagTouch;
				dropped->timestamp = level.time;
				dropped->touch_debounce_time = level.time + 1.0;
				dropped->owner = ent; 
			}
		}
		if (ent->client->pers.inventory[ITEM_INDEX(flag3_item)]) 
		{
			dropped = Drop_Item(ent, flag3_item);
			ent->client->pers.inventory[ITEM_INDEX(flag3_item)] = 0;
			safe_bprintf(PRINT_HIGH, "%s dropped the GREEN flag!\n", ent->client->pers.netname);
			if (dropped) {
				// hack the velocity to make it bounce random
				dropped->velocity[0] = (rand() % 600) - 300;
				dropped->velocity[1] = (rand() % 600) - 300;
				dropped->think = CTFDropFlagThink;
				dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
				dropped->touch = CTFDropFlagTouch;
				dropped->timestamp = level.time;
				dropped->touch_debounce_time = level.time + 1.0;
				dropped->owner = ent; 
			}
		}
		return true;
	}
}

/*static*/ void CTFFlagThink (edict_t *ent)
{
	if (ent->solid != SOLID_NOT)
		ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);
	ent->nextthink = level.time + FRAMETIME;
}


void CTFFlagSetup (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	// 3Team CTF uses different flag models
	if (ttctf->value) {
		if (strcmp(ent->classname, "item_flag_team1") == 0)
			ent->item->world_model = "models/ctf/flags/flag1.md2";
		if (strcmp(ent->classname, "item_flag_team2") == 0)
			ent->item->world_model = "models/ctf/flags/flag2.md2";
	} else {
		if (strcmp(ent->classname, "item_flag_team1") == 0)
			ent->item->world_model = "players/male/flag1.md2";
		if (strcmp(ent->classname, "item_flag_team2") == 0)
			ent->item->world_model = "players/male/flag2.md2";
	}

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf ("CTFFlagSetup: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);

	gi.linkentity (ent);

	ent->nextthink = level.time + FRAMETIME;
	ent->think = CTFFlagThink;
}

void CTFEffects (edict_t *player)
{
	player->s.effects &= ~(EF_FLAG1 | EF_FLAG2);

	if (!ctf->value)
	{
		player->s.modelindex3 = 0;
		return;
	}

	if (!player || !player->client)
		return;

	if (player->health > 0)
	{
		if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])
			player->s.effects |= EF_FLAG1;
		else if (player->client->pers.inventory[ITEM_INDEX(flag3_item)])
#ifdef KMQUAKE2_ENGINE_MOD
			player->s.effects |= EF_FLAG1|EF_FLAG2;
#else
			player->s.effects |= EF_FLAG2;
#endif
		else if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])
			player->s.effects |= EF_FLAG2;
	}

	if (ttctf->value) // Knightmare added
	{
		if (player->client->pers.inventory[ITEM_INDEX(flag1_item)]
			&& player->client->pers.inventory[ITEM_INDEX(flag2_item)])
			player->s.modelindex3 = gi.modelindex("models/ctf/flags/flag4.md2");
		else if (player->client->pers.inventory[ITEM_INDEX(flag1_item)]
			&& player->client->pers.inventory[ITEM_INDEX(flag3_item)])
			player->s.modelindex3 = gi.modelindex("models/ctf/flags/flag5.md2");
		else if (player->client->pers.inventory[ITEM_INDEX(flag2_item)]
			&& player->client->pers.inventory[ITEM_INDEX(flag3_item)])
			player->s.modelindex3 = gi.modelindex("models/ctf/flags/flag6.md2");
		else if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])
			player->s.modelindex3 = gi.modelindex("models/ctf/flags/flag1.md2");
		else if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])
			player->s.modelindex3 = gi.modelindex("models/ctf/flags/flag2.md2");
		else if (player->client->pers.inventory[ITEM_INDEX(flag3_item)])
			player->s.modelindex3 = gi.modelindex("models/ctf/flags/flag3.md2");
		else
			player->s.modelindex3 = 0;
	}
	else 
	{
		if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])
			player->s.modelindex3 = gi.modelindex("players/male/flag1.md2");
		else if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])
			player->s.modelindex3 = gi.modelindex("players/male/flag2.md2");
		else
			player->s.modelindex3 = 0;
	}
}

// called when we enter the intermission
void CTFCalcScores (void)
{
	int i;

	ctfgame.total1 = ctfgame.total2 = 0;
	for (i = 0; i < maxclients->value; i++)
	{
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			ctfgame.total1 += game.clients[i].resp.score;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			ctfgame.total2 += game.clients[i].resp.score;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM3)
			ctfgame.total3 += game.clients[i].resp.score;
	}
}

void CTFID_f (edict_t *ent)
{
	if (ent->client->resp.id_state) {
		safe_cprintf(ent, PRINT_HIGH, "Disabling player identication display.\n");
		ent->client->resp.id_state = false;
	} else {
		safe_cprintf(ent, PRINT_HIGH, "Activating player identication display.\n");
		ent->client->resp.id_state = true;
	}
}

/*static*/ void CTFSetIDView (edict_t *ent)
{
	vec3_t	forward, dir;
	trace_t	tr;
	edict_t	*who, *best;
	float	bd = 0, d;
	int i;

	// only check every few frames
	if (level.time - ent->client->resp.lastidtime < 0.25)
		return;
	ent->client->resp.lastidtime = level.time;

	ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;
	ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = 0;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 1024, forward);
	VectorAdd(ent->s.origin, forward, forward);
	tr = gi.trace(ent->s.origin, NULL, NULL, forward, ent, MASK_SOLID);
	if (tr.fraction < 1 && tr.ent && tr.ent->client) {
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
			CS_GENERAL + (tr.ent - g_edicts - 1);
		if (tr.ent->client->resp.ctf_team == CTF_TEAM1)
			ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = imageindex_sbfctf1;
		else if (tr.ent->client->resp.ctf_team == CTF_TEAM2)
			ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = imageindex_sbfctf2;
		else if (tr.ent->client->resp.ctf_team == CTF_TEAM3)
			ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = imageindex_sbfctf3;
		return;
	}

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	best = NULL;
	for (i = 1; i <= maxclients->value; i++) {
		who = g_edicts + i;
		if (!who->inuse || who->solid == SOLID_NOT)
			continue;
		VectorSubtract(who->s.origin, ent->s.origin, dir);
		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if (d > bd && loc_CanSee(ent, who)) {
			bd = d;
			best = who;
		}
	}
	if (bd > 0.90) {
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
			CS_GENERAL + (best - g_edicts - 1);
		if (best->client->resp.ctf_team == CTF_TEAM1)
			ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = imageindex_sbfctf1;
		else if (best->client->resp.ctf_team == CTF_TEAM2)
			ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = imageindex_sbfctf2;
		else if (best->client->resp.ctf_team == CTF_TEAM3)
			ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = imageindex_sbfctf3;
	}
}

void SetCTFStats (edict_t *ent)
{
	gitem_t *tech;
	int i;
	int p1, p2, p3;
	edict_t *e;

	if (!ctf->value)
		return;

	if (ctfgame.match > MATCH_NONE)
		ent->client->ps.stats[STAT_CTF_MATCH] = CONFIG_CTF_MATCH;
	else
		ent->client->ps.stats[STAT_CTF_MATCH] = 0;

	if (ctfgame.warnactive)
		ent->client->ps.stats[STAT_CTF_TEAMINFO] = CONFIG_CTF_TEAMINFO;
	else
		ent->client->ps.stats[STAT_CTF_TEAMINFO] = 0;

	//ghosting
	if (ent->client->resp.ghost) {
		ent->client->resp.ghost->score = ent->client->resp.score;
	//	strncpy(ent->client->resp.ghost->netname, ent->client->pers.netname);
		Q_strncpyz(ent->client->resp.ghost->netname, sizeof(ent->client->resp.ghost->netname), ent->client->pers.netname);
		ent->client->resp.ghost->number = ent->s.number;
	}

	// logo headers for the frag display
	ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = imageindex_ctfsb1;
	ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = imageindex_ctfsb2;
	if (ttctf->value)
		ent->client->ps.stats[STAT_CTF_TEAM3_HEADER] = imageindex_ctfsb3;

	// if during intermission, we must blink the team header of the winning team
	if (level.intermissiontime && (level.framenum & 8))
	{ // blink 1/8th second
		if (ttctf->value)
		{
			if (ctfgame.team1 > ctfgame.team2 && ctfgame.team1 > ctfgame.team3)
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			else if (ctfgame.team2 > ctfgame.team1 && ctfgame.team2 > ctfgame.team3)
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			else if (ctfgame.team3 > ctfgame.team1 && ctfgame.team3 > ctfgame.team2)
				ent->client->ps.stats[STAT_CTF_TEAM3_HEADER] = 0;
			// frag tie breaker
			else if (ctfgame.total1 > ctfgame.total2 && ctfgame.total1 > ctfgame.total3) 
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			else if (ctfgame.total2 > ctfgame.total1 && ctfgame.total2 > ctfgame.total3) 
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			else if (ctfgame.total3 > ctfgame.total1 && ctfgame.total3 > ctfgame.total2) 
				ent->client->ps.stats[STAT_CTF_TEAM3_HEADER] = 0;
			else { // tie game!
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
				ent->client->ps.stats[STAT_CTF_TEAM3_HEADER] = 0;
			}
		}
		else
		{
			// note that ctfgame.total[12] is set when we go to intermission
			if (ctfgame.team1 > ctfgame.team2)
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			else if (ctfgame.team2 > ctfgame.team1)
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			else if (ctfgame.total1 > ctfgame.total2) // frag tie breaker
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			else if (ctfgame.total2 > ctfgame.total1) 
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			else { // tie game!
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			}
		}
	}

	// tech icon
	i = 0;
	ent->client->ps.stats[STAT_CTF_TECH] = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) {
			ent->client->ps.stats[STAT_CTF_TECH] = gi.imageindex(tech->icon);
			break;
		}
		i++;
	}

	// figure out what icon to display for team logos
	// three states:
	//   flag at base
	//   flag taken
	//   flag dropped
	p1 = imageindex_i_ctf1;
	e = G_Find(NULL, FOFS(classname), "item_flag_team1");
	if (e != NULL) {
		if (e->solid == SOLID_NOT) {
			int i;

			// not at base
			// check if on player
			p1 = imageindex_i_ctf1d; // default to dropped
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag1_item)])
				{
					// enemy has it
					p1 = imageindex_i_ctf1t;
					break;
				}
		} else if (e->spawnflags & DROPPED_ITEM)
			p1 = imageindex_i_ctf1d; // must be dropped
	}
	p2 = imageindex_i_ctf2;
	e = G_Find(NULL, FOFS(classname), "item_flag_team2");
	if (e != NULL)
	{
		if (e->solid == SOLID_NOT)
		{
			int i;

			// not at base
			// check if on player
			p2 = imageindex_i_ctf2d; // default to dropped
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag2_item)])
				{
					// enemy has it
					p2 = imageindex_i_ctf2t;
					break;
				}
		} else if (e->spawnflags & DROPPED_ITEM)
			p2 = imageindex_i_ctf2d; // must be dropped
	}
	// Knightmare added
	p3 = imageindex_i_ctf3;
	e = G_Find(NULL, FOFS(classname), "item_flag_team3");
	if (e != NULL)
	{
		if (e->solid == SOLID_NOT)
		{
			int i;

			// not at base
			// check if on player
			p3 = imageindex_i_ctf3d; // default to dropped
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag3_item)])
				{
					// enemy has it
					p3 = imageindex_i_ctf3t;
					break;
				}
		} else if (e->spawnflags & DROPPED_ITEM)
			p3 = imageindex_i_ctf3d; // must be dropped
	}


	ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
	ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;
	if (ttctf->value) // Knightmare added
		ent->client->ps.stats[STAT_CTF_TEAM3_PIC] = p3;

	if (ctfgame.last_flag_capture && level.time - ctfgame.last_flag_capture < 5)
	{
		if (ctfgame.last_capture_team == CTF_TEAM1)
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
			else
				ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = 0;
		else if (ctfgame.last_capture_team == CTF_TEAM2)
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;
			else
				ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = 0;
		else  // Knightmare added
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_CTF_TEAM3_PIC] = p3;
			else
				ent->client->ps.stats[STAT_CTF_TEAM3_PIC] = 0;
	}

	ent->client->ps.stats[STAT_CTF_TEAM1_CAPS] = ctfgame.team1;
	ent->client->ps.stats[STAT_CTF_TEAM2_CAPS] = ctfgame.team2;
	if (ttctf->value) // Knightmare added
		ent->client->ps.stats[STAT_CTF_TEAM3_CAPS] = ctfgame.team3;

	ent->client->ps.stats[STAT_CTF_FLAG_PIC] = 0;
	ent->client->ps.stats[STAT_CTF_FLAG_PIC2] = 0;

	// Knightmare changed
	if (ent->client->resp.ctf_team == CTF_TEAM1)
	{
		if (ttctf->value && ent->client->pers.inventory[ITEM_INDEX(flag2_item)]
			&& ent->client->pers.inventory[ITEM_INDEX(flag3_item)]
			&& (level.framenum & 8))
		{
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = imageindex_i_ctf2;
			ent->client->ps.stats[STAT_CTF_FLAG_PIC2] = imageindex_i_ctf3;
		}
		else if (ttctf->value && ent->client->pers.inventory[ITEM_INDEX(flag3_item)]
			&& (level.framenum & 8))
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = imageindex_i_ctf3;
		else if (ent->client->pers.inventory[ITEM_INDEX(flag2_item)]
			&& (level.framenum & 8))
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = imageindex_i_ctf2;
	}
	else if (ent->client->resp.ctf_team == CTF_TEAM2)
	{
		if (ttctf->value && ent->client->pers.inventory[ITEM_INDEX(flag1_item)]
			&& ent->client->pers.inventory[ITEM_INDEX(flag3_item)]
			&& (level.framenum & 8))
		{
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = imageindex_i_ctf1;
			ent->client->ps.stats[STAT_CTF_FLAG_PIC2] = imageindex_i_ctf3;
		}
		else if (ttctf->value && ent->client->pers.inventory[ITEM_INDEX(flag3_item)]
			&& (level.framenum & 8))
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = imageindex_i_ctf3;
		else if (ent->client->pers.inventory[ITEM_INDEX(flag1_item)]
			&& (level.framenum & 8))
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = imageindex_i_ctf1;
	}
	else if (ttctf->value && ent->client->resp.ctf_team == CTF_TEAM3)
	{
		if (ent->client->pers.inventory[ITEM_INDEX(flag1_item)]
			&& ent->client->pers.inventory[ITEM_INDEX(flag2_item)]
			&& (level.framenum & 8))
		{
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = imageindex_i_ctf1;
			ent->client->ps.stats[STAT_CTF_FLAG_PIC2] = imageindex_i_ctf2;
		}
		else if (ent->client->pers.inventory[ITEM_INDEX(flag1_item)]
			&& (level.framenum & 8))
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = imageindex_i_ctf1;
		else if (ent->client->pers.inventory[ITEM_INDEX(flag2_item)]
			&& (level.framenum & 8))
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = imageindex_i_ctf2;
	}

	ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = 0;
	ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = 0;
	if (ttctf->value) // Knightmare added
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM3_PIC] = 0;

	if (ent->client->resp.ctf_team == CTF_TEAM1)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = imageindex_i_ctfj;
	else if (ent->client->resp.ctf_team == CTF_TEAM2)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = imageindex_i_ctfj;
	// Knightmare added
	else if (ent->client->resp.ctf_team == CTF_TEAM3)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM3_PIC] = imageindex_i_ctfj;

	if (ent->client->resp.id_state)
		CTFSetIDView(ent);
	else {
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;
		ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = 0;
	}
}

/*------------------------------------------------------------------------*/

/*QUAKED info_player_team1 (1 0 0) (-16 -16 -24) (16 16 32)
potential team1 spawning position for CTF games
*/
void SP_info_player_team1 (edict_t *self)
{
}

/*QUAKED info_player_team2 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for CTF games
*/
void SP_info_player_team2 (edict_t *self)
{
}

// Knightmare added
/*QUAKED info_player_team3 (0 0 1) (-16 -16 -24) (16 16 32)
potential team3 spawning position for 3Team-CTF games
*/
void SP_info_player_team3 (edict_t *self)
{
}


/*------------------------------------------------------------------------*/
/* GRAPPLE																  */
/*------------------------------------------------------------------------*/

// ent is player
void CTFPlayerResetGrapple (edict_t *ent)
{
	if (ent->client && ent->client->ctf_grapple)
		CTFResetGrapple(ent->client->ctf_grapple);
}

// self is grapple, not player
void CTFResetGrapple (edict_t *self)
{
	if (self->owner->client->ctf_grapple) {
		float volume = 1.0;
		gclient_t *cl;

		if (self->owner->client->silencer_shots)
			volume = 0.2;

		gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grreset.wav"), volume, ATTN_NORM, 0);
		cl = self->owner->client;
		cl->ctf_grapple = NULL;
		cl->ctf_grapplereleasetime = level.time;
		cl->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
		cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		G_FreeEdict(self);
	}
}

void CTFGrappleTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float volume = 1.0;

	if (other == self->owner)
		return;

	if (self->owner->client->ctf_grapplestate != CTF_GRAPPLE_STATE_FLY)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		CTFResetGrapple(self);
		return;
	}

	VectorCopy(vec3_origin, self->velocity);

	PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage) {
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_GRAPPLE);
		CTFResetGrapple(self);
		return;
	}

	self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_PULL; // we're on hook
	self->enemy = other;

	self->solid = SOLID_NOT;

	if (self->owner->client->silencer_shots)
		volume = 0.2;

	gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grpull.wav"), volume, ATTN_NORM, 0);
	gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhit.wav"), volume, ATTN_NORM, 0);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (self->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

// draw beam between grapple and self
void CTFGrappleDrawCable (edict_t *self)
{
	vec3_t	offset, start, end, f, r;
	vec3_t	dir;
	float	distance;

	AngleVectors (self->owner->client->v_angle, f, r, NULL);
	VectorSet(offset, 16, 16, self->owner->viewheight-8);
	P_ProjectSource (self->owner->client, self->owner->s.origin, offset, f, r, start);

	VectorSubtract(start, self->owner->s.origin, offset);

	VectorSubtract (start, self->s.origin, dir);
	distance = VectorLength(dir);
	// don't draw cable if close
	if (distance < 64)
		return;

// ACEBOT_ADD
	ACEND_GrapFired(self);
// ACEBOT_END

#if 0
	if (distance > 256)
		return;

	// check for min/max pitch
	vectoangles (dir, angles);
	if (angles[0] < -180)
		angles[0] += 360;
	if (fabs(angles[0]) > 45)
		return;

	trace_t	tr; //!!

	tr = gi.trace (start, NULL, NULL, self->s.origin, self, MASK_SHOT);
	if (tr.ent != self) {
		CTFResetGrapple(self);
		return;
	}
#endif

	// adjust start for beam origin being in middle of a segment
//	VectorMA (start, 8, f, start);

	VectorCopy (self->s.origin, end);
	// adjust end z for end spot since the monster is currently dead
//	end[2] = self->absmin[2] + self->size[2] / 2;

	gi.WriteByte (svc_temp_entity);
#if 1 //def USE_GRAPPLE_CABLE
	gi.WriteByte (TE_GRAPPLE_CABLE);
	gi.WriteShort (self->owner - g_edicts);
	gi.WritePosition (self->owner->s.origin);
	gi.WritePosition (end);
	gi.WritePosition (offset);
#else
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (end);
	gi.WritePosition (start);
#endif
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void SV_AddGravity (edict_t *ent);

// pull the player toward the grapple
void CTFGrapplePull (edict_t *self)
{
	vec3_t hookdir, v;
	float vlen;

	if (strcmp(self->owner->client->pers.weapon->classname, "weapon_grapple") == 0 &&
		!self->owner->client->newweapon &&
		self->owner->client->weaponstate != WEAPON_FIRING &&
		self->owner->client->weaponstate != WEAPON_ACTIVATING) {
		CTFResetGrapple(self);
		return;
	}

	if (self->enemy) {
		if (self->enemy->solid == SOLID_NOT) {
			CTFResetGrapple(self);
			return;
		}
		if (self->enemy->solid == SOLID_BBOX) {
			VectorScale(self->enemy->size, 0.5, v);
			VectorAdd(v, self->enemy->s.origin, v);
			VectorAdd(v, self->enemy->mins, self->s.origin);
			gi.linkentity (self);
		} else
			VectorCopy(self->enemy->velocity, self->velocity);
		if (self->enemy->takedamage &&
			!CheckTeamDamage (self->enemy, self->owner))
		{
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			T_Damage (self->enemy, self, self->owner, self->velocity, self->s.origin, vec3_origin, 1, 1, 0, MOD_GRAPPLE);
			gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhurt.wav"), volume, ATTN_NORM, 0);
		}
		if (self->enemy->deadflag) { // he died
			CTFResetGrapple(self);
			return;
		}
	}

	CTFGrappleDrawCable(self);

	if (self->owner->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
	{
		// pull player toward grapple
		// this causes icky stuff with prediction, we need to extend
		// the prediction layer to include two new fields in the player
		// move stuff: a point and a velocity.  The client should add
		// that velociy in the direction of the point
		vec3_t forward, up;

		AngleVectors (self->owner->client->v_angle, forward, NULL, up);
		VectorCopy(self->owner->s.origin, v);
		v[2] += self->owner->viewheight;
		VectorSubtract (self->s.origin, v, hookdir);

		vlen = VectorLength(hookdir);

		if (self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL &&
			vlen < 64)
		{
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			self->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grhang.wav"), volume, ATTN_NORM, 0);
			self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_HANG;
		}

		VectorNormalize (hookdir);
		VectorScale(hookdir, CTF_GRAPPLE_PULL_SPEED, hookdir);
		VectorCopy(hookdir, self->owner->velocity);
		SV_AddGravity(self->owner);
	}
}

void CTFFireGrapple (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*grapple;
	trace_t	tr;

	VectorNormalize (dir);

	grapple = G_Spawn();
	VectorCopy (start, grapple->s.origin);
	VectorCopy (start, grapple->s.old_origin);
	vectoangles (dir, grapple->s.angles);
	VectorScale (dir, speed, grapple->velocity);
	grapple->movetype = MOVETYPE_FLYMISSILE;
	grapple->clipmask = MASK_SHOT;
	grapple->solid = SOLID_BBOX;
	grapple->s.effects |= effect;
	VectorClear (grapple->mins);
	VectorClear (grapple->maxs);
	grapple->s.modelindex = gi.modelindex ("models/weapons/grapple/hook/tris.md2");
	grapple->classname = "grapple"; // Knightmare added
//	grapple->s.sound = gi.soundindex ("misc/lasfly.wav");
	grapple->owner = self;
	grapple->touch = CTFGrappleTouch;
//	grapple->nextthink = level.time + FRAMETIME;
//	grapple->think = CTFGrappleThink;
	grapple->dmg = damage;
	self->client->ctf_grapple = grapple;
	self->client->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
	gi.linkentity (grapple);

	tr = gi.trace (self->s.origin, NULL, NULL, grapple->s.origin, grapple, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (grapple->s.origin, -10, dir, grapple->s.origin);
		grapple->touch (grapple, tr.ent, NULL, NULL);
	}
}	

void CTFGrappleFire (edict_t *ent, vec3_t g_offset, int damage, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	float volume = 1.0;

	if (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
		return; // it's already out

	AngleVectors (ent->client->v_angle, forward, right, NULL);
//	VectorSet(offset, 24, 16, ent->viewheight-8+2);
	VectorSet(offset, 24, 8, ent->viewheight-8+2);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (ent->client->silencer_shots)
		volume = 0.2;

	gi.sound (ent, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grfire.wav"), volume, ATTN_NORM, 0);
	CTFFireGrapple (ent, start, forward, damage, CTF_GRAPPLE_SPEED, effect);

#if 0
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
#endif

	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void CTFWeapon_Grapple_Fire (edict_t *ent, qboolean altfire)
{
	int		damage;

	damage = 10;
	CTFGrappleFire (ent, vec3_origin, damage, 0);
	ent->client->ps.gunframe++;
}

void CTFWeapon_Grapple (edict_t *ent)
{
	static int	pause_frames[]	= {10, 18, 27, 0};
	static int	fire_frames[]	= {6, 0};
	int prevstate;

	// if the the attack button is still down, stay in the firing frame
	if ((ent->client->buttons & BUTTON_ATTACK) && 
		ent->client->weaponstate == WEAPON_FIRING &&
		ent->client->ctf_grapple)
		ent->client->ps.gunframe = 9;

	if (!(ent->client->buttons & BUTTON_ATTACK) && 
		ent->client->ctf_grapple) {
		CTFResetGrapple(ent->client->ctf_grapple);
		if (ent->client->weaponstate == WEAPON_FIRING)
			ent->client->weaponstate = WEAPON_READY;
	}


	if (ent->client->newweapon && 
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY &&
		ent->client->weaponstate == WEAPON_FIRING) {
		// he wants to change weapons while grappled
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = 32;
	}

	prevstate = ent->client->weaponstate;
	Weapon_Generic (ent, 5, 9, 31, 36, pause_frames, fire_frames, 
		CTFWeapon_Grapple_Fire);

	// if we just switched back to grapple, immediately go to fire frame
	if (prevstate == WEAPON_ACTIVATING &&
		ent->client->weaponstate == WEAPON_READY &&
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
		if (!(ent->client->buttons & BUTTON_ATTACK))
			ent->client->ps.gunframe = 9;
		else
			ent->client->ps.gunframe = 5;
		ent->client->weaponstate = WEAPON_FIRING;
	}
}

void CTFTeam_f (edict_t *ent)
{
	char *t, *s;
	int desired_team;

	t = gi.args();
	if (!*t) {
		safe_cprintf(ent, PRINT_HIGH, "You are on the %s team.\n",
			CTFTeamName(ent->client->resp.ctf_team));
		return;
	}

	if (ctfgame.match > MATCH_SETUP) {
		safe_cprintf(ent, PRINT_HIGH, "Can't change teams in a match.\n");
		return;
	}

	if (Q_stricmp(t, "red") == 0)
		desired_team = CTF_TEAM1;
	else if (Q_stricmp(t, "blue") == 0)
		desired_team = CTF_TEAM2;
	else if (ttctf->value && Q_stricmp(t, "green") == 0)
		desired_team = CTF_TEAM3;
	else {
		safe_cprintf(ent, PRINT_HIGH, "Unknown team %s.\n", t);
		return;
	}

	if (ent->client->resp.ctf_team == desired_team) {
		safe_cprintf(ent, PRINT_HIGH, "You are already on the %s team.\n",
			CTFTeamName(ent->client->resp.ctf_team));
		return;
	}

////
	ent->svflags = 0;
	ent->flags &= ~FL_GODMODE;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = 0;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

	if (ent->solid == SOLID_NOT) { // spectator
		PutClientInServer (ent);
		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		safe_bprintf(PRINT_HIGH, "%s joined the %s team.\n",
			ent->client->pers.netname, CTFTeamName(desired_team));
		return;
	}

	ent->health = 0;
	player_die (ent, ent, ent, 100000, vec3_origin);
	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;
	respawn (ent);

	ent->client->resp.score = 0;

	safe_bprintf(PRINT_HIGH, "%s changed to the %s team.\n",
		ent->client->pers.netname, CTFTeamName(desired_team));
}

/*
==================
CTFScoreboardMessage
==================
*/
void CTFScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	tmp[128];	// Knightmare added
	char	string[1400];
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
	total[0] = total[1] = total[2]= 0;
	last[0] = last[1] = last[2]= 0;
	totalscore[0] = totalscore[1] = totalscore[2] = 0;
	for (i=0; i<game.maxclients; i++)
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
		for (j=0; j<total[team]; j++)
		{
			if (score > sortedscores[team][j])
				break;
		}
		for (k=total[team]; k>j; k--)
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

	// team headers
	if (ttctf->value)
		Com_sprintf (string, sizeof(string), "if 24 xv -64 yv 8 pic 24 endif "
			"xv -32 yv 28 string \"%4d/%-3d\" "
			"xv 24 yv 12 num 2 18 "
			"if 25 xv 96 yv 8 pic 25 endif "
			"xv 128 yv 28 string \"%4d/%-3d\" "
			"xv 184 yv 12 num 2 20 "
			"if 36 xv 256 yv 8 pic 36 endif "
			"xv 288 yv 28 string \"%4d/%-3d\" "
			"xv 344 yv 12 num 2 33 ",
			totalscore[0], total[0],
			totalscore[1], total[1],
			totalscore[2], total[2]);
	else
		Com_sprintf (string, sizeof(string), "if 24 xv 8 yv 8 pic 24 endif "
			"xv 40 yv 28 string \"%4d/%-3d\" "
			"xv 98 yv 12 num 2 18 "
			"if 25 xv 168 yv 8 pic 25 endif "
			"xv 200 yv 28 string \"%4d/%-3d\" "
			"xv 256 yv 12 num 2 20 ",
			totalscore[0], total[0],
			totalscore[1], total[1]);

	len = (int)strlen(string);

	for (i=0; i<16; i++)
	{
		if (i >= total[0] && i >= total[1] && i > total[2])
			break; // we're done
		
		*entry = 0;
		
		// left side
		if (i < total[0])
		{
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];
			
			if (ttctf->value) {
			//	sprintf(entry+strlen(entry),
				Com_sprintf (tmp, sizeof(tmp),
			#ifdef KMQUAKE2_ENGINE_MOD
					"3tctf -72 %d %d %d %d ",
			#else
					"ctf -72 %d %d %d %d ",
			#endif
					42 + i * 8,
					sorted[0][i],
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);
				Q_strncatz (entry, sizeof(entry), tmp);
			}
			else {
			//	sprintf(entry+strlen(entry),
				Com_sprintf (tmp, sizeof(tmp),
					"ctf 0 %d %d %d %d ",
					42 + i * 8,
					sorted[0][i],
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);
				Q_strncatz (entry, sizeof(entry), tmp);
			}
			
			if (ttctf->value)
			{
				if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)]
					&& cl_ent->client->pers.inventory[ITEM_INDEX(flag3_item)])
				{
				//	sprintf(entry + strlen(entry), "xv -16 yv %d picn sbfctf2 "
					Com_sprintf (tmp, sizeof(tmp), "xv -16 yv %d picn sbfctf2 "
												   "xv -8 yv %d picn sbfctf3 ", 42 + i * 8, 42 + i * 8);
					Q_strncatz (entry, sizeof(entry), tmp);
					/*if (level.framenum & 1)
						Com_sprintf (tmp, sizeof(tmp), "xv -16 yv %d picn sbfctf2 ", 42 + i * 8);
					else
						Com_sprintf (tmp, sizeof(tmp), "xv -16 yv %d picn sbfctf3 ", 42 + i * 8);
					Q_strncatz (entry, sizeof(entry), tmp);
					*/
				}
				else if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
				//	sprintf(entry + strlen(entry), "xv -16 yv %d picn sbfctf2 ", 42 + i * 8);
					Com_sprintf (tmp, sizeof(tmp), "xv -16 yv %d picn sbfctf2 ", 42 + i * 8);
					Q_strncatz (entry, sizeof(entry), tmp);
				}
				else if (cl_ent->client->pers.inventory[ITEM_INDEX(flag3_item)]) {
				//	sprintf(entry + strlen(entry), "xv -16 yv %d picn sbfctf3 ", 42 + i * 8);
					Com_sprintf (tmp, sizeof(tmp), "xv -16 yv %d picn sbfctf3 ", 42 + i * 8);
					Q_strncatz (entry, sizeof(entry), tmp);
				}
			}
			else
				if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
				//	sprintf(entry + strlen(entry), "xv 56 yv %d picn sbfctf2 ", 42 + i * 8);
					Com_sprintf (tmp, sizeof(tmp), "xv 56 yv %d picn sbfctf2 ", 42 + i * 8);
					Q_strncatz (entry, sizeof(entry), tmp);
				}

			if (maxsize - len > strlen(entry)) {
			//	strncat(string, entry);
				Q_strncatz(string, sizeof(string), entry);
				len = (int)strlen(string);
				last[0] = i;
			}
		}

		// right side /center
		if (i < total[1])
		{
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];
			
			if (ttctf->value) {
			//	sprintf(entry+strlen(entry),
				Com_sprintf (tmp, sizeof(tmp),
			#ifdef KMQUAKE2_ENGINE_MOD
					"3tctf 88 %d %d %d %d ",
			#else
					"ctf 88 %d %d %d %d ",
			#endif
					42 + i * 8,
					sorted[1][i],
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);
				Q_strncatz (entry, sizeof(entry), tmp);
			}
			else {
			//	sprintf(entry+strlen(entry),
				Com_sprintf (tmp, sizeof(tmp),
					"ctf 160 %d %d %d %d ",
					42 + i * 8,
					sorted[1][i],
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);
				Q_strncatz (entry, sizeof(entry), tmp);
			}
				
			if (ttctf->value)
			{
				if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)]
					&& cl_ent->client->pers.inventory[ITEM_INDEX(flag3_item)])
				{
				//	sprintf(entry + strlen(entry), "xv 144 yv %d picn sbfctf1 "
					Com_sprintf (tmp, sizeof(tmp), "xv 144 yv %d picn sbfctf1 "
												   "xv 152 yv %d picn sbfctf3 ", 42 + i * 8, 42 + i * 8);
					Q_strncatz (entry, sizeof(entry), tmp);
					/*if (level.framenum & 1)
						Com_sprintf (tmp, sizeof(tmp), "xv 144 yv %d picn sbfctf1 ", 42 + i * 8);
					else
						Com_sprintf (tmp, sizeof(tmp), "xv 144 yv %d picn sbfctf3", 42 + i * 8);
					Q_strncatz (entry, tmp, sizeof(entry));
					*/
				}
				else if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
				//	sprintf(entry + strlen(entry), "xv 144 yv %d picn sbfctf1 ", 42 + i * 8);
					Com_sprintf (tmp, sizeof(tmp), "xv 144 yv %d picn sbfctf1 ", 42 + i * 8);
					Q_strncatz (entry, sizeof(entry), tmp);
				}
				else if (cl_ent->client->pers.inventory[ITEM_INDEX(flag3_item)]) {
				//	sprintf(entry + strlen(entry), "xv 144 yv %d picn sbfctf3 ", 42 + i * 8);
					Com_sprintf (tmp, sizeof(tmp), "xv 144 yv %d picn sbfctf3 ", 42 + i * 8);
					Q_strncatz (entry, sizeof(entry), tmp);
				}
			}
			else
				if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
				//	sprintf(entry + strlen(entry), "xv 216 yv %d picn sbfctf1 ", 42 + i * 8);
					Com_sprintf (tmp, sizeof(tmp), "xv 216 yv %d picn sbfctf1 ", 42 + i * 8);
					Q_strncatz (entry, sizeof(entry), tmp);
				}
			
			if (maxsize - len > strlen(entry)) {
			//	strncat(string, entry);
				Q_strncatz(string, sizeof(string), entry);
				len = (int)strlen(string);
				last[1] = i;
			}
		}

		// 3Team CTF right side
		if (ttctf->value && i < total[2])
		{
			cl = &game.clients[sorted[2][i]];
			cl_ent = g_edicts + 1 + sorted[2][i];
			
		//	sprintf(entry+strlen(entry),
			Com_sprintf (tmp, sizeof(tmp),
		#ifdef KMQUAKE2_ENGINE_MOD
				"3tctf 248 %d %d %d %d ",
		#else
				"ctf 248 %d %d %d %d ",
		#endif
				42 + i * 8,
				sorted[2][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);
			Q_strncatz (entry, sizeof(entry), tmp);
			
			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)]
				&& cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
			{
			//	sprintf(entry + strlen(entry), "xv 304 yv %d picn sbfctf1 "
				Com_sprintf (tmp, sizeof(tmp), "xv 304 yv %d picn sbfctf1 "
											   "xv 312 yv %d picn sbfctf2 ", 42 + i * 8, 42 + i * 8);
				Q_strncatz (entry, sizeof(entry), tmp);
				/*if (level.framenum & 1)
					Com_sprintf (tmp, sizeof(tmp), "xv 304 yv %d picn sbfctf1 ", 42 + i * 8);
				else
					Com_sprintf (tmp, sizeof(tmp), "xv 304 yv %d picn sbfctf2", 42 + i * 8);
				Q_strncatz (entry, sizeof(entry), tmp);
				*/
			}
			else if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
			//	sprintf(entry + strlen(entry), "xv 304 yv %d picn sbfctf1 ", 42 + i * 8);
				Com_sprintf (tmp, sizeof(tmp), "xv 304 yv %d picn sbfctf1 ", 42 + i * 8);
				Q_strncatz (entry, sizeof(entry), tmp);
			}
			else if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
			//	sprintf(entry + strlen(entry), "xv 304 yv %d picn sbfctf2 ", 42 + i * 8);			
				Com_sprintf (tmp, sizeof(tmp), "xv 304 yv %d picn sbfctf2 ", 42 + i * 8);			
				Q_strncatz (entry, sizeof(entry), tmp);
			}
			
			if (maxsize - len > strlen(entry)) {
			//	strncat(string, entry);
				Q_strncatz(string, sizeof(string), entry);
				len = (int)strlen(string);
				last[2] = i;
			}
		}
	}

	// put in spectators if we have enough room
	if (ttctf->value)
	{
		if (last[0] > last[1] && last[0] > last[2])
			j = last[0];
		else if (last[1] > last[0] && last[1] > last[2])
			j = last[1];
		else
			j = last[2];
	}
	else
	{
		if (last[0] > last[1])
			j = last[0];
		else
			j = last[1];
	}
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
			
			if (!k)
			{
				k = 1;
			//	sprintf(entry, "xv 0 yv %d string2 \"Spectators\" ", j);
				Com_sprintf (entry, sizeof(entry), "xv 0 yv %d string2 \"Spectators\" ", j);
			//	strncat(string, entry);
				Q_strncatz(string, sizeof(string), entry);
				len = (int)strlen(string);
				j += 8;
			}
			
		//	sprintf(entry+strlen(entry),
			Com_sprintf (tmp, sizeof(tmp),
				"ctf %d %d %d %d %d ",
				(n & 1) ? 160 : 0, // x
				j, // y
				i, // playernum
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);
			Q_strncatz (entry, sizeof(entry), tmp);
			if (maxsize - len > strlen(entry)) {
			//	strncat(string, entry);
				Q_strncatz(string, sizeof(string), entry);
				len = (int)strlen(string);
			}
			
			if (n & 1)
				j += 8;
			n++;
		}
	}
	
	if (ttctf->value)
	{
		if (total[0] - last[0] > 1) { // couldn't fit everyone
		//	sprintf(string + strlen(string), "xv -64 yv %d string \"..and %d more\" ",
			Com_sprintf (tmp, sizeof(tmp), "xv -64 yv %d string \"..and %d more\" ",
				42 + (last[0]+1)*8, total[0] - last[0] - 1);
			Q_strncatz (string, sizeof(string), tmp);
		}
		if (total[1] - last[1] > 1) { // couldn't fit everyone
		//	sprintf(string + strlen(string), "xv 96 yv %d string \"..and %d more\" ",
			Com_sprintf (tmp, sizeof(tmp), "xv 96 yv %d string \"..and %d more\" ",
				42 + (last[1]+1)*8, total[1] - last[1] - 1);
			Q_strncatz (string, sizeof(string), tmp);
		}
		if (total[2] - last[2] > 1) { // couldn't fit everyone
		//	sprintf(string + strlen(string), "xv 256 yv %d string \"..and %d more\" ",
			Com_sprintf (tmp, sizeof(tmp), "xv 256 yv %d string \"..and %d more\" ",
				42 + (last[2]+1)*8, total[2] - last[2] - 1);
			Q_strncatz (string, sizeof(string), tmp);
		}
	}
	else
	{
		if (total[0] - last[0] > 1) { // couldn't fit everyone
		//	sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
			Com_sprintf (tmp, sizeof(tmp), "xv 8 yv %d string \"..and %d more\" ",
				42 + (last[0]+1)*8, total[0] - last[0] - 1);
			Q_strncatz (string, sizeof(string), tmp);
		}
		if (total[1] - last[1] > 1) { // couldn't fit everyone
		//	sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ",
			Com_sprintf (tmp, sizeof(tmp), "xv 168 yv %d string \"..and %d more\" ",
				42 + (last[1]+1)*8, total[1] - last[1] - 1);
			Q_strncatz (string, sizeof(string), tmp);
		}
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/*------------------------------------------------------------------------*/
/* TECH																	  */
/*------------------------------------------------------------------------*/


void Apply_Tech_Shell (gitem_t *item, edict_t *ent)
{
	ent->s.renderfx = RF_GLOW;

	if (use_lithiumtechs->value)
	{
		ent->s.modelindex = gi.modelindex ("models/items/keys/pyramid/tris.md2");
		ent->item->world_model = "models/items/keys/pyramid/tris.md2";
		ent->item->icon = "k_pyramid";
	}
	else
	{
		if (!strcmp(item->classname, "item_tech1")) {
			ent->s.modelindex = gi.modelindex ("models/ctf/resistance/tris.md2");
			ent->item->world_model = "models/ctf/resistance/tris.md2";
			ent->item->icon = "tech1";
		}
		else if (!strcmp(item->classname, "item_tech2")) {
			ent->s.modelindex = gi.modelindex ("models/ctf/strength/tris.md2");
			ent->item->world_model = "models/ctf/strength/tris.md2";
			ent->item->icon = "tech2";
		}
		else if (!strcmp(item->classname, "item_tech3")) {
			ent->s.modelindex = gi.modelindex ("models/ctf/haste/tris.md2");
			ent->item->world_model = "models/ctf/haste/tris.md2";
			ent->item->icon = "tech3";
		}
		else if (!strcmp(item->classname, "item_tech4")) {
			ent->s.modelindex = gi.modelindex ("models/ctf/regeneration/tris.md2");
			ent->item->world_model = "models/ctf/regeneration/tris.md2";
			ent->item->icon = "tech4";
		}
		else if (!strcmp(item->classname, "item_tech5")) {
			ent->s.modelindex = gi.modelindex ("models/ctf/vampire/tris.md2");
			ent->item->world_model = "models/ctf/vampire/tris.md2";
			ent->item->icon = "tech5";
		}
		else if (!strcmp(item->classname, "item_tech6")) {
			ent->s.modelindex = gi.modelindex ("models/ctf/ammogen/tris.md2");
			ent->item->world_model = "models/ctf/ammogen/tris.md2";
			ent->item->icon = "tech6";
		}
	}

	if (!use_coloredtechs->value && !use_lithiumtechs->value)
		return;

	if (!strcmp(item->classname, "item_tech1")) // resist
		ent->s.effects |= EF_QUAD;
	else if (!strcmp(item->classname, "item_tech2")) // strength
		ent->s.effects |= EF_PENT;
	else if (!strcmp(item->classname, "item_tech3")) // haste
		ent->s.effects |= EF_DOUBLE;
	else if (!strcmp(item->classname, "item_tech4")) { // regen
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx = RF_SHELL_GREEN; 
	}
	else if (!strcmp(item->classname, "item_tech5")) // vampire
		ent->s.effects |= EF_QUAD | EF_PENT;
	else if (!strcmp(item->classname, "item_tech6")) // ammogen
		ent->s.effects |= EF_HALF_DAMAGE;
}

void CTFHasTech (edict_t *who)
{
	if (level.time - who->client->ctf_lasttechmsg > 2) {
		safe_centerprintf(who, "You already have a TECH powerup.");
		who->client->ctf_lasttechmsg = level.time;
	}
}

gitem_t *CTFWhat_Tech (edict_t *ent)
{
	gitem_t *tech;
	int i;

	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) {
			return tech;
		}
		i++;
	}
	return NULL;
}

qboolean CTFPickup_Tech (edict_t *ent, edict_t *other)
{
	gitem_t *tech;
	int i;

	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			other->client->pers.inventory[ITEM_INDEX(tech)]) {
			CTFHasTech(other);
			return false; // has this one
		}
		i++;
	}
	
	// client only gets one tech
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	other->client->ctf_regentime = level.time;
	return true;
}

/*static*/ void SpawnTech (gitem_t *item, edict_t *spot);

void CTFTechTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//owner (who dropped us) can't touch for two secs
	if ((other == ent->owner) && (level.time < ent->touch_debounce_time))
		return;

	Touch_Item (ent, other, plane, surf);
}

static edict_t *FindTechSpawn (void)
{
	edict_t *spot = NULL;
	int i = rand() % 16;

	while (i--)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	if (!spot)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	return spot;
}

/*static*/ void TechThink (edict_t *tech)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL) {
		SpawnTech(tech->item, spot);
		G_FreeEdict(tech);
	} else {
		tech->nextthink = level.time + CTF_TECH_TIMEOUT;
		tech->think = TechThink;
	}
}

void CTFDrop_Tech (edict_t *ent, gitem_t *item)
{
	edict_t *tech;

	if (!allow_techdrop->value)
		return;

	tech = Drop_Item(ent, item);
	tech->nextthink = level.time + tech_life->value; // was CTF_TECH_TIMEOUT
	tech->think = TechThink;

	if (allow_techpickup->value)
	{
		tech->touch = CTFTechTouch;
		tech->touch_debounce_time = level.time + 1.0;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;

	Apply_Tech_Shell(item, tech);
}

void CTFDeadDropTech (edict_t *ent)
{
	gitem_t *tech;
	edict_t *dropped;
	int i;

	i = 0;
	while (tnames[i])
	{
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)])
		{
			dropped = Drop_Item(ent, tech);
			// hack the velocity to make it bounce random
			dropped->velocity[0] = (rand() % 600) - 300;
			dropped->velocity[1] = (rand() % 600) - 300;
			dropped->nextthink = level.time + tech_life->value; // was CTF_TECH_TIMEOUT
			dropped->think = TechThink;
			dropped->owner = NULL;
			ent->client->pers.inventory[ITEM_INDEX(tech)] = 0;
			Apply_Tech_Shell (tech, dropped);
		}
		i++;
	}
}

//ScarFace- this function counts the number of runes in circulation
int TechCount (void)
{
	gitem_t	*tech;
	edict_t	*cl_ent;
	edict_t	*mapent = NULL;
	int i, j;
	int count = 0;

	mapent = g_edicts+1; // skip the worldspawn
	// cycle through all ents to find techs
	for (i = 1; i < globals.num_edicts; i++, mapent++)
	{
		if (!mapent->classname)
			continue;

		if (!strncmp(mapent->classname, "item_tech", 9))
			count++;
	}
	//cycle through all players to find techs
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (cl_ent->inuse)
		{
			j = 0;
			while (tnames[j])
			{
				if ((tech = FindItemByClassname(tnames[j])) != NULL &&
					cl_ent->client->pers.inventory[ITEM_INDEX(tech)])
					count++;
				j++;
			}
		}
	}
	return count;
}

int NumOfTech (int index)
{
	gitem_t	*tech;
	edict_t	*cl_ent;
	edict_t	*mapent = NULL;
	int i;
	int count = 0;
	char techname [80];

	mapent = g_edicts+1; // skip the worldspawn
	// cycle through all ents to find techs
	for (i = 1; i < globals.num_edicts; i++, mapent++)
	{
		if (!mapent->classname)
			continue;

		Com_sprintf(techname, sizeof(techname), "item_tech%d", index+1);
		if (!strcmp(mapent->classname, techname))
			count++;
	}
	//cycle through all players to find techs
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (cl_ent->inuse)
		{
			if ((tech = FindItemByClassname(tnames[index])) != NULL &&
				cl_ent->client->pers.inventory[ITEM_INDEX(tech)])
				count++;
		}
	}
	//gi.dprintf ("Number of tech%d in map: %d\n", index+1, count);
	return count;
}


// ScarFace- a diagnostic function to display the number of runes in circulation
void Cmd_TechCount_f (edict_t *ent)
{
	int count;
	count = TechCount();
	//gi.dprintf ("Number of techs in game: %d\n", count);
	safe_cprintf(ent, PRINT_HIGH, "Number of techs in game: %d\n", count);
}

// ScarFace- spawn the additional runes
void SpawnMoreTechs (int oldtechcount, int newtechcount, int numtechtypes)
{
	gitem_t	*tech;
	edict_t	*spot;
	int i, j;

	//gi.dprintf ("Number of techs in loop: %d\n", numtechtypes);
	i = oldtechcount % numtechtypes; //Spawn next tech in succession of the last one spawned
	j = oldtechcount; //Start with count at old tech 
	//gi.dprintf ("tech number to start on: %d\n", (i+1));
	while ( (j < numtechtypes) || ((j < tech_max->value) && (j < newtechcount)) )
	{
		while ( (tnames[i]) &&
				((j < numtechtypes) || ((j < tech_max->value) && (j < newtechcount))) )
		{
			if (((tech = FindItemByClassname(tnames[i])) != NULL &&
				 (spot = FindTechSpawn()) != NULL)
				&& ((int)(tech_flags->value) & (0x1 << i)))
			{
				//gi.dprintf ("Spawning tech%d\n", (i+1));
				SpawnTech(tech, spot);
				j++;
			}
			i++;
		}
		i = 0;
	}
	//gi.dprintf ("Current number of techs in game: %d\n", j);
}

// ScarFace- remove some runes
void RemoveTechs (int oldtechcount, int newtechcount, int numtechtypes)
{
	edict_t	*mapent = NULL;
	int i, j, k;
	int removed;

	//gi.dprintf ("Number of techs desired: %d\n", newtechcount);
	// find the last tech added or the tech to be removed
	if ((oldtechcount % numtechtypes) == 0)
		i = TECHTYPES-1;
	else {

		for (i=TECHTYPES-1; i>=0; i--)
			if ( NumOfTech(i) > (oldtechcount / numtechtypes)
				|| !((int)(tech_flags->value) & (0x1 << i)) )
				break;
	}

	j = oldtechcount;
	//gi.dprintf ("tech number to start removing on: %d\n", (i+1));
	while ((tnames[i]) && (j > newtechcount)) //leave at least 1 of each tech
	{
		removed = 0; //flag to remove only one tech per pass
		mapent = g_edicts+1; //skip the worldspawn
		for (k = 1; k < globals.num_edicts; k++, mapent++)
		{
			if (!mapent->classname)
				continue;
			if (!strcmp(mapent->classname, tnames[i]))
			{
				//gi.dprintf ("Removing tech%d\n", (i+1));
				G_FreeEdict(mapent);
				j--;
				removed = 1;
			}
			if (removed == 1) //don't keep removing techs of this type
				break;
		}
		//If we can't find this tech in map, wait until a player drops it instead of removing others
		if (removed == 0)
			return;
		i--;
	}
	//gi.dprintf ("Current number of techs in game: %d\n", j);
}

// ScarFace- this function checks to see if we need to spawn or remove runes
void CheckNumTechs (void)
{
	edict_t	*cl_ent;
	int i, numclients;
	int newtechcount, numtechs, techbits;
	int numtechtypes = 0;

	if (!deathmatch->value || level.time <= 0) // don't run while no in map
		return;

	if (!use_techs->value || (ctf->value && ((int)dmflags->value & DF_CTF_NO_TECH)) )
		return;

	// clamp tech_flags to valid range
	techbits = (int)(tech_flags->value);
	if (techbits < 1 || techbits > 63) {
		gi.dprintf ("Invalid tech_flags, setting default of 15.\n");
		gi.cvar_forceset("tech_flags", "15");
	}

	// count number of tech types enabled
	i = 0;
	while (tnames[i]) {
		if ((int)(tech_flags->value) & (0x1 << i))
			numtechtypes++;
		i++;
	}
	
	// count num of clients
	numclients = 0;
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (cl_ent->inuse)
			numclients++;
	}

	newtechcount = tech_perplayer->value * numclients;
	if (newtechcount > tech_max->value) // cap at tech_max
		newtechcount = tech_max->value;
	if (newtechcount < numtechtypes) // leave at least 1 of each enabled tech
		newtechcount = numtechtypes;
	numtechs = TechCount();
	if (newtechcount > numtechs)
	{
	//	gi.dprintf ("Number of techs to spawn: %d\n", newtechcount);
		SpawnMoreTechs (numtechs, newtechcount, numtechtypes);
	}
	if (newtechcount < numtechs)
	{
	//	gi.dprintf ("Number of techs to spawn: %d\n", newtechcount);
		RemoveTechs (numtechs, newtechcount, numtechtypes);
	}
}

/*static*/ void SpawnTech (gitem_t *item, edict_t *spot)
{
	edict_t	*ent;
	vec3_t	forward, right;
	vec3_t  angles;

	ent = G_Spawn();

	ent->classname = item->classname;
	ent->item = item;
	ent->spawnflags = DROPPED_ITEM;
	ent->s.effects = item->world_model_flags;

	Apply_Tech_Shell (item, ent);

	VectorSet (ent->mins, -15, -15, -15);
	VectorSet (ent->maxs, 15, 15, 15);
	gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;
	ent->owner = ent;

	angles[0] = 0;
	angles[1] = rand() % 360;
	angles[2] = 0;

	AngleVectors (angles, forward, right, NULL);
	VectorCopy (spot->s.origin, ent->s.origin);
	ent->s.origin[2] += 16;
	VectorScale (forward, 100, ent->velocity);
	ent->velocity[2] = 300;

//	ent->nextthink = level.time + CTF_TECH_TIMEOUT;
	ent->nextthink = level.time + tech_life->value;
	ent->think = TechThink;

	gi.linkentity (ent);
}

/*static*/ void SpawnTechs (edict_t *ent)
{
	gitem_t *tech;
	edict_t *spot;
	int i, techbits;

	// clamp tech_flags to valid range
	techbits = (int)(tech_flags->value);
	if (techbits < 1 || techbits > 63) {
		gi.dprintf ("Invalid tech_flags, setting default of 15.\n");
		gi.cvar_forceset("tech_flags", "15");
	}

	i = 0;
	while (tnames[i])
	{
		if ( (tech = FindItemByClassname(tnames[i])) != NULL &&
			(spot = FindTechSpawn()) != NULL
			&& ((int)tech_flags->value & (0x1 << i)) )
			SpawnTech(tech, spot);
		i++;
	}
	if (ent)
		G_FreeEdict(ent);
}

// frees the passed edict!
void CTFRespawnTech (edict_t *ent)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL)
		SpawnTech(ent->item, spot);
	G_FreeEdict(ent);
}

void CTFSetupTechSpawn (void)
{
	edict_t *ent;

	if (!use_techs->value || (ctf->value && ((int)dmflags->value & DF_CTF_NO_TECH)) )
		return;

	ent = G_Spawn();
	ent->nextthink = level.time + 2;
	ent->think = SpawnTechs;
}

void CTFResetTech (void)
{
	edict_t *ent;
	int i;

	for (ent = g_edicts + 1, i = 1; i < globals.num_edicts; i++, ent++) {
		if (ent->inuse)
			if (ent->item && (ent->item->flags & IT_TECH))
				G_FreeEdict(ent);
	}
	SpawnTechs(NULL);
}

int CTFApplyResistance (edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	//if (!deathmatch->value)
	//	return dmg;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech1");
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		// make noise
	   	gi.sound (ent, CHAN_VOICE, gi.soundindex("ctf/tech1.wav"), volume, ATTN_NORM, 0);
		return dmg / tech_resist->value;
	}
	return dmg;
}

int CTFApplyStrength (edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;

	//if (!deathmatch->value)
	//	return dmg;

	if (!tech)
		tech = FindItemByClassname("item_tech2");
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		return dmg * tech_strength->value;
	}
	return dmg;
}

qboolean CTFApplyStrengthSound (edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	//if (!deathmatch->value)
	//	return false;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech2");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		if (ent->client->ctf_techsndtime < level.time) {
			ent->client->ctf_techsndtime = level.time + 1;
			if (ent->client->quad_framenum > level.framenum)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2x.wav"), volume, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2.wav"), volume, ATTN_NORM, 0);
		}
		return true;
	}
	return false;
}


qboolean CTFApplyHaste (edict_t *ent)
{
	static gitem_t *tech = NULL;

	//if (!deathmatch->value)
	//	return false;

	if (!tech)
		tech = FindItemByClassname("item_tech3");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)])
		return true;
	return false;
}

void CTFApplyHasteSound (edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	//if (!deathmatch->value)
	//	return;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech3");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)] &&
		ent->client->ctf_techsndtime < level.time) {
		ent->client->ctf_techsndtime = level.time + 1;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech3.wav"), volume, ATTN_NORM, 0);
	}
}

void CTFApplyRegeneration (edict_t *ent)
{
	static gitem_t *tech = NULL;
	qboolean noise = false;
	gclient_t *client;
	int index;
	float volume = 1.0;

	//if (!deathmatch->value)
	//	return;

	client = ent->client;
	if (!client)
		return;

	if (ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech4");
	if (tech && client->pers.inventory[ITEM_INDEX(tech)])
	{
		if (client->ctf_regentime < level.time) {
			client->ctf_regentime = level.time;
			if (ent->health < tech_regen_health_max->value)
			{
				ent->health += 5;
				if (ent->health > tech_regen_health_max->value)
					ent->health = tech_regen_health_max->value;
				client->ctf_regentime += 0.5;
				noise = true;
			}
			if (tech_regen_armor->value)
			{
				index = ArmorIndex (ent);
				if (index)
				{
					index = ArmorIndex (ent);
					if (index && client->pers.inventory[index] < tech_regen_armor_max->value)
					{
						client->pers.inventory[index] += 5;
						if (client->pers.inventory[index] > tech_regen_armor_max->value)
							client->pers.inventory[index] = tech_regen_armor_max->value;
						client->ctf_regentime += 0.5;
						noise = true;
					}
				}
				else if (tech_regen_armor_always->value)
				{
					index = combat_armor_index;
					if (index && client->pers.inventory[index] < tech_regen_armor_max->value) 
					{
						client->pers.inventory[index] += 5;
						if (client->pers.inventory[index] > tech_regen_armor_max->value)
							client->pers.inventory[index] = tech_regen_armor_max->value;
						client->ctf_regentime += 0.5;
						noise = true;
					}

				}
			}
		}
		if (noise && ent->client->ctf_techsndtime < level.time) {
			ent->client->ctf_techsndtime = level.time + 1;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech4.wav"), volume, ATTN_NORM, 0);
		}
	}
}

qboolean CTFHasRegeneration (edict_t *ent)
{
	static gitem_t *tech = NULL;

	//if (!deathmatch->value)
	//	return false;

	if (!tech)
		tech = FindItemByClassname("item_tech4");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)])
		return true;
	return false;
}

// Knightmare added
void CTFApplyVampire (edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;

	//if (!deathmatch->value)
	//	return;

	if (!tech)
		tech = FindItemByClassname("item_tech5");
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)])
	{
		if (ent->health < tech_vampiremax->value)
		{
			ent->health += dmg * tech_vampire->value;
			ent->health = min(ent->health,tech_vampiremax->value);
		}
		CTFApplyVampireSound(ent);
	}
}

// Knightmare added
void CTFApplyVampireSound (edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech5");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)] &&
		ent->client->ctf_techsndtime < level.time) {
		ent->client->ctf_techsndtime = level.time + 1;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech5.wav"), volume, ATTN_NORM, 0);
	}
}

// Knightmare added
void CTFApplyAmmogen (edict_t *attacker, edict_t *targ)
{
	static gitem_t *tech = NULL, *pack = NULL;

	if (!deathmatch->value)
		return;

	if (!attacker || !targ)
		return;

	if (!tech)
		tech = FindItemByClassname("item_tech6");
	if (!pack)
		pack = FindItemByClassname("item_ammogen_pack");
	if (tech && pack && attacker->client && targ->client && attacker->client->pers.inventory[ITEM_INDEX(tech)])
	{
		Drop_Item(targ, pack);
		CTFApplyAmmogenSound(attacker);
	}
}

// Knightmare added
void CTFApplyAmmogenSound (edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech6");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)] &&
		ent->client->ctf_techsndtime < level.time)
	{
		ent->client->ctf_techsndtime = level.time + 1;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech6.wav"), volume, ATTN_NORM, 0);
	}
}

/*
======================================================================

SAY_TEAM

======================================================================
*/

// This array is in 'importance order', it indicates what items are
// more important when reporting their names.
struct {
	char *classname;
	int priority;
} loc_names[] = 
{
	{	"item_flag_team1",			1 },
	{	"item_flag_team2",			1 },
	{	"item_flag_team3",			1 }, // Knightmare added
	{	"item_quad",				2 }, 
	{	"item_invulnerability",		2 },
	{	"item_jetpack",				3 }, // Knightmare added
	{	"item_tech1",				3 },
	{	"item_tech2",				3 },
	{	"item_tech3",				3 },
	{	"item_tech4",				3 },
	{	"item_tech5",				3 },
	{	"item_tech6",				3 },
	{	"weapon_railgun",			4 },
	{	"weapon_rocketlauncher",	4 },
	{	"weapon_hyperblaster",		4 },
	{	"weapon_chaingun",			4 },
	{	"weapon_grenadelauncher",	4 },
	{	"weapon_machinegun",		4 },
	{	"weapon_supershotgun",		4 },
	{	"weapon_shotgun",			4 },
	{	"item_power_screen",		5 },
	{	"item_power_shield",		5 },
	{	"item_armor_body",			6 },
	{	"item_armor_combat",		6 },
	{	"item_armor_jacket",		6 },
	{	"item_silencer",			7 },
	{	"item_breather",			7 },
	{	"item_enviro",				7 },
	{	"item_adrenaline",			7 },
	{	"item_bandolier",			8 },
	{	"item_pack",				8 },
	{	"item_flashlight",			8 }, // Knightmare added
	{ NULL, 0 }
};


/*static*/ void CTFSay_Team_Location (edict_t *who, char *buf, size_t bufSize)
{
	edict_t *what = NULL;
	edict_t *hot = NULL;
	float hotdist = 999999, newdist;
	vec3_t v;
	int hotindex = 999;
	int i;
	gitem_t *item;
	int nearteam = -1;
	edict_t *flag1, *flag2;
	qboolean hotsee = false;
	qboolean cansee;

	while ((what = loc_findradius(what, who->s.origin, 1024)) != NULL) {
		// find what in loc_classnames
		for (i = 0; loc_names[i].classname; i++)
			if (strcmp(what->classname, loc_names[i].classname) == 0)
				break;
		if (!loc_names[i].classname)
			continue;
		// something we can see get priority over something we can't
		cansee = loc_CanSee(what, who);
		if (cansee && !hotsee) {
			hotsee = true;
			hotindex = loc_names[i].priority;
			hot = what;
			VectorSubtract(what->s.origin, who->s.origin, v);
			hotdist = VectorLength(v);
			continue;
		}
		// if we can't see this, but we have something we can see, skip it
		if (hotsee && !cansee)
			continue;
		if (hotsee && hotindex < loc_names[i].priority)
			continue;
		VectorSubtract(what->s.origin, who->s.origin, v);
		newdist = VectorLength(v);
		if (newdist < hotdist || 
			(cansee && loc_names[i].priority < hotindex)) {
			hot = what;
			hotdist = newdist;
			hotindex = i;
			hotsee = loc_CanSee(hot, who);
		}
	}

	if (!hot) {
	//	strncpy(buf, "nowhere");
		Q_strncpyz(buf, bufSize, "nowhere");
		return;
	}

	// we now have the closest item
	// see if there's more than one in the map, if so
	// we need to determine what team is closest
	what = NULL;
	while ((what = G_Find(what, FOFS(classname), hot->classname)) != NULL) {
		if (what == hot)
			continue;
		// if we are here, there is more than one, find out if hot
		// is closer to red flag or blue flag
		if ((flag1 = G_Find(NULL, FOFS(classname), "item_flag_team1")) != NULL &&
			(flag2 = G_Find(NULL, FOFS(classname), "item_flag_team2")) != NULL) {
			VectorSubtract(hot->s.origin, flag1->s.origin, v);
			hotdist = VectorLength(v);
			VectorSubtract(hot->s.origin, flag2->s.origin, v);
			newdist = VectorLength(v);
			if (hotdist < newdist)
				nearteam = CTF_TEAM1;
			else if (hotdist > newdist)
				nearteam = CTF_TEAM2;
		}
		break;
	}

	if ((item = FindItemByClassname(hot->classname)) == NULL) {
	//	strncpy(buf, "nowhere");
		Q_strncpyz(buf, bufSize, "nowhere");
		return;
	}

	// in water?
	if (who->waterlevel)
	//	strncpy(buf, "in the water ");
		Q_strncpyz(buf, bufSize, "in the water ");
	else
		*buf = 0;

	// near or above
	VectorSubtract(who->s.origin, hot->s.origin, v);
	if (fabs(v[2]) > fabs(v[0]) && fabs(v[2]) > fabs(v[1]))
		if (v[2] > 0)
		//	strncat(buf, "above ");
			Q_strncatz(buf, bufSize, "above ");
		else
		//	strncat(buf, "below ");
			Q_strncatz(buf, bufSize, "below ");
	else
	//	strncat(buf, "near ");
		Q_strncatz(buf, bufSize, "near ");

	if (nearteam == CTF_TEAM1)
	//	strncat(buf, "the red ");
		Q_strncatz(buf, bufSize, "the red ");
	else if (nearteam == CTF_TEAM2)
	//	strncat(buf, "the blue ");
		Q_strncatz(buf, bufSize, "the blue ");
	else
	//	strncat(buf, "the ");
		Q_strncatz(buf, bufSize, "the ");

//	strncat(buf, item->pickup_name);
	Q_strncatz(buf, bufSize, item->pickup_name);
}

/*static*/ void CTFSay_Team_Armor (edict_t *who, char *buf, size_t bufSize)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;
	char		tmp[128];	// Knightmare added

	*buf = 0;

	power_armor_type = PowerArmorType (who);
	if (power_armor_type)
	{
		cells = who->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
		if (cells) {
		//	sprintf(buf+strlen(buf), "%s with %i cells ",
			Com_sprintf (tmp, sizeof(tmp), "%s with %i cells ",
				(power_armor_type == POWER_ARMOR_SCREEN) ?
				"Power Screen" : "Power Shield", cells);
			Q_strncatz (buf, bufSize, tmp);
		}
	}

	index = ArmorIndex (who);
	if (index)
	{
		item = GetItemByIndex (index);
		if (item) {
			if (*buf)
			//	strncat(buf, "and ");
				Q_strncatz(buf, bufSize, "and ");
		//	sprintf(buf+strlen(buf), "%i units of %s",
			Com_sprintf (tmp, sizeof(tmp), "%i units of %s",
				who->client->pers.inventory[index], item->pickup_name);
			Q_strncatz (buf, bufSize, tmp);
		}
	}

	if (!*buf)
	//	strncpy(buf, "no armor", bufSize);
		Q_strncpyz(buf, bufSize, "no armor");
}

/*static*/ void CTFSay_Team_Health (edict_t *who, char *buf, size_t bufSize)
{
	if (who->health <= 0)
	//	strncpy(buf, "dead", bufSize);
		Q_strncpyz (buf, bufSize, "dead");
	else
		Com_sprintf (buf, bufSize, "%i health", who->health);
}

/*static*/ void CTFSay_Team_Tech (edict_t *who, char *buf, size_t bufSize)
{
	gitem_t *tech;
	int i;

	// see if the player has a tech powerup
	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			who->client->pers.inventory[ITEM_INDEX(tech)]) {
			Com_sprintf (buf, bufSize, "the %s", tech->pickup_name);
			return;
		}
		i++;
	}
//	strncpy(buf, "no powerup", bufSize);
	Q_strncpyz(buf, bufSize, "no powerup");
}

/*static*/ void CTFSay_Team_Weapon (edict_t *who, char *buf, size_t bufSize)
{
	if (who->client->pers.weapon)
	//	strncpy(buf, who->client->pers.weapon->pickup_name, bufSize);
		Q_strncpyz(buf, bufSize, who->client->pers.weapon->pickup_name);
	else
	//	strncpy(buf, "none", bufSize);
		Q_strncpyz(buf, bufSize, "none");
}

/*static*/ void CTFSay_Team_Sight (edict_t *who, char *buf, size_t bufSize)
{
	int i;
	edict_t *targ;
	int n = 0;
	char s[1024];
	char s2[1024];

	*s = *s2 = 0;
	for (i = 1; i <= maxclients->value; i++) {
		targ = g_edicts + i;
		if (!targ->inuse || 
			targ == who ||
			!loc_CanSee(targ, who))
			continue;
		if (*s2) {
			if (strlen(s) + strlen(s2) + 3 < sizeof(s)) {
				if (n)
				//	strncat(s, ", ");
					Q_strncatz(s, sizeof(s), ", ");
			//	strncat(s, s2);
				Q_strncatz(s, sizeof(s), s2);
				*s2 = 0;
			}
			n++;
		}
	//	strncpy(s2, targ->client->pers.netname);
		Q_strncpyz(s2, sizeof(s2), targ->client->pers.netname);
	}
	if (*s2) {
		if (strlen(s) + strlen(s2) + 6 < sizeof(s)) {
			if (n)
			//	strncat(s, " and ");
				Q_strncatz(s, sizeof(s), " and ");
		//	strncat(s, s2);
			Q_strncatz(s, sizeof(s), s2);
		}
	//	strncpy(buf, s, bufSize);
		Q_strncpyz(buf, bufSize, s);
	} else
	//	strncpy(buf, "no one", bufSize);
		Q_strncpyz(buf, bufSize, "no one");
}

void CTFSay_Team (edict_t *who, char *msg)
{
	char	outmsg[256];
	char	buf[256];
	int		i;
	char	*p;
	edict_t *cl_ent;
//	size_t	outmsgSize = sizeof(outmsg);

	if (CheckFlood(who))
		return;

	outmsg[0] = 0;

	if (*msg == '\"') {
		msg[strlen(msg) - 1] = 0;
		msg++;
	}

	for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 2; msg++)
	{
		if (*msg == '%')
		{
			switch (*++msg)
			{
				case 'l' :
				case 'L' :
					CTFSay_Team_Location(who, buf, sizeof(buf));
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
						strcpy(p, buf);
					//	Q_strncpyz(p, outmsgSize-2, buf);
						p += strlen(buf);
					//	outmsgSize -= strlen(buf);
					}
					break;
				case 'a' :
				case 'A' :
					CTFSay_Team_Armor(who, buf, sizeof(buf));
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
						strcpy(p, buf);
					//	Q_strncpyz(p, outmsgSize-2, buf);
						p += strlen(buf);
					//	outmsgSize -= strlen(buf);
					}
					break;
				case 'h' :
				case 'H' :
					CTFSay_Team_Health(who, buf, sizeof(buf));
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
						strcpy(p, buf);
					//	Q_strncpyz(p, outmsgSize-2, buf);
						p += strlen(buf);
					//	outmsgSize -= strlen(buf);
					}
					break;
				case 't' :
				case 'T' :
					CTFSay_Team_Tech(who, buf, sizeof(buf));
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
						strcpy(p, buf);
					//	Q_strncpyz(p, outmsgSize-2, buf);
						p += strlen(buf);
					//	outmsgSize -= strlen(buf);
					}
					break;
				case 'w' :
				case 'W' :
					CTFSay_Team_Weapon(who, buf, sizeof(buf));
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
						strcpy(p, buf);
					//	Q_strncpyz(p, outmsgSize-2, buf);
						p += strlen(buf);
					//	outmsgSize -= strlen(buf);
					}
					break;

				case 'n' :
				case 'N' :
					CTFSay_Team_Sight(who, buf, sizeof(buf));
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
						strcpy(p, buf);
					//	Q_strncpyz(p, outmsgSize-2, buf);
						p += strlen(buf);
					//	outmsgSize -= strlen(buf);
					}
					break;

				default :
					*p++ = *msg;
			}
		}
		else
			*p++ = *msg;
	}
	*p = 0;

	for (i = 0; i < maxclients->value; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->resp.ctf_team == who->client->resp.ctf_team)
			safe_cprintf(cl_ent, PRINT_CHAT, "(%s): %s\n", 
				who->client->pers.netname, outmsg);
	}
}

/*-----------------------------------------------------------------------*/
/*QUAKED misc_ctf_banner (1 .5 0) (-4 -64 0) (4 64 248) TEAM2 TEAM3
The origin is the bottom of the banner.
The banner is 248 tall.
*/
/*static*/ void misc_ctf_banner_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_ctf_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ctf/banner/tris.md2");
	ent->s.renderfx |= RF_NOSHADOW;
	// Knightmare- added team 3 skin
	if (ent->spawnflags & 2) // team3
		ent->s.skinnum = 2;
	else if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_ctf_small_banner (1 .5 0) (-4 -32 0) (4 32 124) TEAM2 TEAM3
The origin is the bottom of the banner.
The banner is 124 tall.
*/
void SP_misc_ctf_small_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ctf/banner/small.md2");
	ent->s.renderfx |= RF_NOSHADOW;
	// Knightmare- added team 3 skin
	if (ent->spawnflags  & 2) // team3
		ent->s.skinnum = 2;
	else if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*-----------------------------------------------------------------------*/

/*static*/ void SetLevelName (pmenu_t *p)
{
	static char levelname[33];

	levelname[0] = '*';
	if (g_edicts[0].message)
	//	strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
		Q_strncpyz(levelname+1, sizeof(levelname) - 1, g_edicts[0].message);
	else
	//	strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
		Q_strncpyz(levelname+1, sizeof(levelname) - 1, level.mapname);
//	levelname[sizeof(levelname) - 1] = 0;
	p->text = levelname;
}


/*-----------------------------------------------------------------------*/


/* ELECTIONS */

qboolean CTFBeginElection (edict_t *ent, elect_t type, char *msg)
{
	int i;
	int count;
	edict_t *e;

	if (electpercentage->value == 0) {
		safe_cprintf(ent, PRINT_HIGH, "Elections are disabled, only an admin can process this action.\n");
		return false;
	}


	if (ctfgame.election != ELECT_NONE) {
		safe_cprintf(ent, PRINT_HIGH, "Election already in progress.\n");
		return false;
	}

	// clear votes
	count = 0;
	for (i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		e->client->resp.voted = false;
		if (e->inuse)
			count++;
	}

	if (count < 2) {
		safe_cprintf(ent, PRINT_HIGH, "Not enough players for election.\n");
		return false;
	}

	ctfgame.etarget = ent;
	ctfgame.election = type;
	ctfgame.evotes = 0;
	ctfgame.needvotes = (count * electpercentage->value) / 100;
	ctfgame.electtime = level.time + 20; // twenty seconds for election
//	strncpy(ctfgame.emsg, msg, sizeof(ctfgame.emsg) - 1);
	Q_strncpyz(ctfgame.emsg, sizeof(ctfgame.emsg), msg);

	// tell everyone
	safe_bprintf(PRINT_CHAT, "%s\n", ctfgame.emsg);
	safe_bprintf(PRINT_HIGH, "Type YES or NO to vote on this request.\n");
	safe_bprintf(PRINT_HIGH, "Votes: %d  Needed: %d  Time left: %ds\n", ctfgame.evotes, ctfgame.needvotes,
		(int)(ctfgame.electtime - level.time));

	return true;
}

void DoRespawn (edict_t *ent);

void CTFResetAllPlayers (void)
{
	int i;
	edict_t *ent;

	for (i = 1; i <= maxclients->value; i++) {
		ent = g_edicts + i;
		if (!ent->inuse)
			continue;

		if (ent->client->menu)
			PMenu_Close(ent);

		CTFPlayerResetGrapple(ent);
		CTFDeadDropFlag(ent);
		CTFDeadDropTech(ent);

		ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->resp.ready = false;

		ent->svflags = 0;
		ent->flags &= ~FL_GODMODE;
		PutClientInServer(ent);
	}

	// reset the level
	CTFResetTech();
	CTFResetFlags();

	for (ent = g_edicts + 1, i = 1; i < globals.num_edicts; i++, ent++) {
		if (ent->inuse && !ent->client) {
			if (ent->solid == SOLID_NOT && ent->think == DoRespawn &&
				ent->nextthink >= level.time) {
				ent->nextthink = 0;
				DoRespawn(ent);
			}
		}
	}
	if (ctfgame.match == MATCH_SETUP)
		ctfgame.matchtime = level.time + matchsetuptime->value * 60;
}

void CTFAssignGhost (edict_t *ent)
{
	int ghost, i;

	for (ghost = 0; ghost < MAX_CLIENTS; ghost++)
		if (!ctfgame.ghosts[ghost].code)
			break;
	if (ghost == MAX_CLIENTS)
		return;
	ctfgame.ghosts[ghost].team = ent->client->resp.ctf_team;
	ctfgame.ghosts[ghost].score = 0;
	for (;;) {
		ctfgame.ghosts[ghost].code = 10000 + (rand() % 90000);
		for (i = 0; i < MAX_CLIENTS; i++)
			if (i != ghost && ctfgame.ghosts[i].code == ctfgame.ghosts[ghost].code)
				break;
		if (i == MAX_CLIENTS)
			break;
	}
	ctfgame.ghosts[ghost].ent = ent;
//	strncpy(ctfgame.ghosts[ghost].netname, ent->client->pers.netname);
	Q_strncpyz(ctfgame.ghosts[ghost].netname, sizeof(ctfgame.ghosts[ghost].netname), ent->client->pers.netname);
	ent->client->resp.ghost = ctfgame.ghosts + ghost;
	safe_cprintf(ent, PRINT_CHAT, "Your ghost code is **** %d ****\n", ctfgame.ghosts[ghost].code);
	safe_cprintf(ent, PRINT_HIGH, "If you lose connection, you can rejoin with your score "
		"intact by typing \"ghost %d\".\n", ctfgame.ghosts[ghost].code);
}

// start a match
void CTFStartMatch (void)
{
	int i;
	edict_t *ent;
	int ghost = 0;

	ctfgame.match = MATCH_GAME;
	ctfgame.matchtime = level.time + matchtime->value * 60;
	ctfgame.countdown = false;

	ctfgame.team1 = ctfgame.team2 = 0;

	memset(ctfgame.ghosts, 0, sizeof(ctfgame.ghosts));

	for (i = 1; i <= maxclients->value; i++) {
		ent = g_edicts + i;
		if (!ent->inuse)
			continue;

		ent->client->resp.score = 0;
		ent->client->resp.ctf_state = 0;
		ent->client->resp.ghost = NULL;

		safe_centerprintf(ent, "******************\n\nMATCH HAS STARTED!\n\n******************");

		if (ent->client->resp.ctf_team != CTF_NOTEAM)
		{
			// make up a ghost code
			CTFAssignGhost(ent);
			CTFPlayerResetGrapple(ent);
			ent->svflags = SVF_NOCLIENT;
			ent->flags &= ~FL_GODMODE;

			ent->client->respawn_time = level.time + 1.0 + ((rand()%30)/10.0);
			ent->client->ps.pmove.pm_type = PM_DEAD;
			ent->client->anim_priority = ANIM_DEATH;
			ent->s.frame = FRAME_death308-1;
			ent->client->anim_end = FRAME_death308;
			ent->deadflag = DEAD_DEAD;
			ent->movetype = MOVETYPE_NOCLIP;
			ent->client->ps.gunindex = 0;
			gi.linkentity (ent);
		}
	}
}

void CTFEndMatch (void)
{
	ctfgame.match = MATCH_POST;
	safe_bprintf(PRINT_CHAT, "MATCH COMPLETED!\n");

	CTFCalcScores();

	safe_bprintf(PRINT_HIGH, "RED TEAM:  %d captures, %d points\n",
		ctfgame.team1, ctfgame.total1);
	safe_bprintf(PRINT_HIGH, "BLUE TEAM:  %d captures, %d points\n",
		ctfgame.team2, ctfgame.total2);

	if (ttctf->value) // Knightmare added
	{
		safe_bprintf(PRINT_HIGH, "GREEN TEAM:  %d captures, %d points\n",
			ctfgame.team3, ctfgame.total3);

		if (ctfgame.team1 > ctfgame.team2 && ctfgame.team1 > ctfgame.team3)
			safe_bprintf(PRINT_CHAT, "RED team won over the BLUE and GREEN teams by %d CAPTURES!\n",
				ctfgame.team1 - (ctfgame.team2 > ctfgame.team3)? ctfgame.team2:ctfgame.team3);
		else if (ctfgame.team2 > ctfgame.team1 && ctfgame.team2 > ctfgame.team3)
			safe_bprintf(PRINT_CHAT, "BLUE team won over the RED and GREEN teams by %d CAPTURES!\n",
				ctfgame.team2 - (ctfgame.team1 > ctfgame.team3)? ctfgame.team1:ctfgame.team3);
		else if (ctfgame.team3 > ctfgame.team1 && ctfgame.team3 > ctfgame.team2)
			safe_bprintf(PRINT_CHAT, "GREEN team won over the RED and BLUE teams by %d CAPTURES!\n",
				ctfgame.team3 - (ctfgame.team1 > ctfgame.team2)? ctfgame.team1:ctfgame.team2);
		// frag tie breaker
		else if (ctfgame.total1 > ctfgame.total2 && ctfgame.total1 > ctfgame.total3) 
			safe_bprintf(PRINT_CHAT, "RED team won over the BLUE and GREEN teams by %d POINTS!\n",
				ctfgame.total1 - (ctfgame.total2 > ctfgame.total3)?ctfgame.total2:ctfgame.total3);
		else if (ctfgame.total2 > ctfgame.total1 && ctfgame.total2 > ctfgame.total3) 
			safe_bprintf(PRINT_CHAT, "BLUE team won over the RED and GREEN teams by %d POINTS!\n",
				ctfgame.total2 - (ctfgame.total1 > ctfgame.total3)?ctfgame.total1:ctfgame.total3);
		else if (ctfgame.total3 > ctfgame.total1 && ctfgame.total3 > ctfgame.total2) 
			safe_bprintf(PRINT_CHAT, "GREEN team won over the RED and BLUE teams by %d POINTS!\n",
				ctfgame.total3 - (ctfgame.total1 > ctfgame.total2)?ctfgame.total1:ctfgame.total2);
		else
			safe_bprintf(PRINT_CHAT, "TIE GAME!\n");
	}
	else
	{
		if (ctfgame.team1 > ctfgame.team2)
			safe_bprintf(PRINT_CHAT, "RED team won over the BLUE team by %d CAPTURES!\n",
				ctfgame.team1 - ctfgame.team2);
		else if (ctfgame.team2 > ctfgame.team1)
			safe_bprintf(PRINT_CHAT, "BLUE team won over the RED team by %d CAPTURES!\n",
				ctfgame.team2 - ctfgame.team1);
		else if (ctfgame.total1 > ctfgame.total2) // frag tie breaker
			safe_bprintf(PRINT_CHAT, "RED team won over the BLUE team by %d POINTS!\n",
				ctfgame.total1 - ctfgame.total2);
		else if (ctfgame.total2 > ctfgame.total1) 
			safe_bprintf(PRINT_CHAT, "BLUE team won over the RED team by %d POINTS!\n",
				ctfgame.total2 - ctfgame.total1);
		else
			safe_bprintf(PRINT_CHAT, "TIE GAME!\n");
	}
	EndDMLevel();
}

qboolean CTFNextMap (void)
{
	if (ctfgame.match == MATCH_POST) {
		ctfgame.match = MATCH_SETUP;
		CTFResetAllPlayers();
		return true;
	}
	return false;
}

void CTFWinElection (void)
{
	switch (ctfgame.election) {
	case ELECT_MATCH :
		// reset into match mode
		if (competition->value < 3)
			gi.cvar_set("competition", "2");
		ctfgame.match = MATCH_SETUP;
		CTFResetAllPlayers();
		break;

	case ELECT_ADMIN :
		ctfgame.etarget->client->resp.admin = true;
		safe_bprintf(PRINT_HIGH, "%s has become an admin.\n", ctfgame.etarget->client->pers.netname);
		safe_cprintf(ctfgame.etarget, PRINT_HIGH, "Type 'admin' to access the adminstration menu.\n");
		break;

	case ELECT_MAP :
		safe_bprintf(PRINT_HIGH, "%s is warping to level %s.\n", 
			ctfgame.etarget->client->pers.netname, ctfgame.elevel);
	//	strncpy(level.forcemap, ctfgame.elevel, sizeof(level.forcemap) - 1);
		Q_strncpyz(level.forcemap, sizeof(level.forcemap), ctfgame.elevel);
		EndDMLevel();
		break;
	}
	ctfgame.election = ELECT_NONE;
}

void CTFVoteYes (edict_t *ent)
{
	if (ctfgame.election == ELECT_NONE) {
		safe_cprintf(ent, PRINT_HIGH, "No election is in progress.\n");
		return;
	}
	if (ent->client->resp.voted) {
		safe_cprintf(ent, PRINT_HIGH, "You already voted.\n");
		return;
	}
	if (ctfgame.etarget == ent) {
		safe_cprintf(ent, PRINT_HIGH, "You can't vote for yourself.\n");
		return;
	}

	ent->client->resp.voted = true;

	ctfgame.evotes++;
	if (ctfgame.evotes == ctfgame.needvotes) {
		// the election has been won
		CTFWinElection();
		return;
	}
	safe_bprintf(PRINT_HIGH, "%s\n", ctfgame.emsg);
	safe_bprintf(PRINT_CHAT, "Votes: %d  Needed: %d  Time left: %ds\n", ctfgame.evotes, ctfgame.needvotes,
		(int)(ctfgame.electtime - level.time));
}

void CTFVoteNo (edict_t *ent)
{
	if (ctfgame.election == ELECT_NONE) {
		safe_cprintf(ent, PRINT_HIGH, "No election is in progress.\n");
		return;
	}
	if (ent->client->resp.voted) {
		safe_cprintf(ent, PRINT_HIGH, "You already voted.\n");
		return;
	}
	if (ctfgame.etarget == ent) {
		safe_cprintf(ent, PRINT_HIGH, "You can't vote for yourself.\n");
		return;
	}

	ent->client->resp.voted = true;

	safe_bprintf(PRINT_HIGH, "%s\n", ctfgame.emsg);
	safe_bprintf(PRINT_CHAT, "Votes: %d  Needed: %d  Time left: %ds\n", ctfgame.evotes, ctfgame.needvotes,
		(int)(ctfgame.electtime - level.time));
}

void CTFReady (edict_t *ent)
{
	int i, j;
	edict_t *e;
	int t1, t2;

	if (ent->client->resp.ctf_team == CTF_NOTEAM) {
		safe_cprintf(ent, PRINT_HIGH, "Pick a team first (hit <TAB> for menu)\n");
		return;
	}

	if (ctfgame.match != MATCH_SETUP) {
		safe_cprintf(ent, PRINT_HIGH, "A match is not being setup.\n");
		return;
	}

	if (ent->client->resp.ready) {
		safe_cprintf(ent, PRINT_HIGH, "You have already commited.\n");
		return;
	}

	ent->client->resp.ready = true;
	safe_bprintf(PRINT_HIGH, "%s is ready.\n", ent->client->pers.netname);

	t1 = t2 = 0;
	for (j = 0, i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		if (!e->inuse)
			continue;
		if (e->client->resp.ctf_team != CTF_NOTEAM && !e->client->resp.ready)
			j++;
		if (e->client->resp.ctf_team == CTF_TEAM1)
			t1++;
		else if (e->client->resp.ctf_team == CTF_TEAM2)
			t2++;
	}
	if (!j && t1 && t2) {
		// everyone has commited
		safe_bprintf(PRINT_CHAT, "All players have commited.  Match starting\n");
		ctfgame.match = MATCH_PREGAME;
		ctfgame.matchtime = level.time + matchstarttime->value;
		ctfgame.countdown = false;
		gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/talk1.wav"), 1, ATTN_NONE, 0);
	}
}

void CTFNotReady (edict_t *ent)
{
	if (ent->client->resp.ctf_team == CTF_NOTEAM) {
		safe_cprintf(ent, PRINT_HIGH, "Pick a team first (hit <TAB> for menu)\n");
		return;
	}

	if (ctfgame.match != MATCH_SETUP && ctfgame.match != MATCH_PREGAME) {
		safe_cprintf(ent, PRINT_HIGH, "A match is not being setup.\n");
		return;
	}

	if (!ent->client->resp.ready) {
		safe_cprintf(ent, PRINT_HIGH, "You haven't commited.\n");
		return;
	}

	ent->client->resp.ready = false;
	safe_bprintf(PRINT_HIGH, "%s is no longer ready.\n", ent->client->pers.netname);

	if (ctfgame.match == MATCH_PREGAME) {
		safe_bprintf(PRINT_CHAT, "Match halted.\n");
		ctfgame.match = MATCH_SETUP;
		ctfgame.matchtime = level.time + matchsetuptime->value * 60;
	}
}

void CTFGhost (edict_t *ent)
{
	int i;
	int n;

	if (gi.argc() < 2) {
		safe_cprintf(ent, PRINT_HIGH, "Usage:  ghost <code>\n");
		return;
	}

	if (ent->client->resp.ctf_team != CTF_NOTEAM) {
		safe_cprintf(ent, PRINT_HIGH, "You are already in the game.\n");
		return;
	}
	if (ctfgame.match != MATCH_GAME) {
		safe_cprintf(ent, PRINT_HIGH, "No match is in progress.\n");
		return;
	}

	n = atoi(gi.argv(1));

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (ctfgame.ghosts[i].code && ctfgame.ghosts[i].code == n) {
			safe_cprintf(ent, PRINT_HIGH, "Ghost code accepted, your position has been reinstated.\n");
			ctfgame.ghosts[i].ent->client->resp.ghost = NULL;
			ent->client->resp.ctf_team = ctfgame.ghosts[i].team;
			ent->client->resp.ghost = ctfgame.ghosts + i;
			ent->client->resp.score = ctfgame.ghosts[i].score;
			ent->client->resp.ctf_state = 0;
			ctfgame.ghosts[i].ent = ent;
			ent->svflags = 0;
			ent->flags &= ~FL_GODMODE;
			PutClientInServer(ent);
			safe_bprintf(PRINT_HIGH, "%s has been reinstated to %s team.\n",
				ent->client->pers.netname, CTFTeamName(ent->client->resp.ctf_team));
			return;
		}
	}
	safe_cprintf(ent, PRINT_HIGH, "Invalid ghost code.\n");
}

qboolean CTFMatchSetup (void)
{
	if (ctfgame.match == MATCH_SETUP || ctfgame.match == MATCH_PREGAME)
		return true;
	return false;
}

qboolean CTFMatchOn (void)
{
	if (ctfgame.match == MATCH_GAME)
		return true;
	return false;
}


/*-----------------------------------------------------------------------*/

void CTFJoinTeam1(edict_t *ent, pmenuhnd_t *p);
void CTFJoinTeam2(edict_t *ent, pmenuhnd_t *p);
void CTFJoinTeam3(edict_t *ent, pmenuhnd_t *p); // Knightmare added
void CTFCredits(edict_t *ent, pmenuhnd_t *p);
void CTFReturnToMain(edict_t *ent, pmenuhnd_t *p);
void CTFChaseCam(edict_t *ent, pmenuhnd_t *p);

pmenu_t creditsmenu[] = {
	{ "*Quake II",						PMENU_ALIGN_CENTER, NULL },
	{ "*ThreeWave Capture the Flag",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ "*Programming",					PMENU_ALIGN_CENTER, NULL }, 
	{ "Dave 'Zoid' Kirsch",				PMENU_ALIGN_CENTER, NULL },
	{ "*Level Design", 					PMENU_ALIGN_CENTER, NULL },
	{ "Christian Antkow",				PMENU_ALIGN_CENTER, NULL },
	{ "Tim Willits",					PMENU_ALIGN_CENTER, NULL },
	{ "Dave 'Zoid' Kirsch",				PMENU_ALIGN_CENTER, NULL },
	{ "*Art",							PMENU_ALIGN_CENTER, NULL },
	{ "Adrian Carmack Paul Steed",		PMENU_ALIGN_CENTER, NULL },
	{ "Kevin Cloud",					PMENU_ALIGN_CENTER, NULL },
	{ "*Sound",							PMENU_ALIGN_CENTER, NULL },
	{ "Tom 'Bjorn' Klok",				PMENU_ALIGN_CENTER, NULL },
	{ "*Original CTF Art Design",		PMENU_ALIGN_CENTER, NULL },
	{ "Brian 'Whaleboy' Cozzens",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_LEFT, CTFReturnToMain }
};

static const int jmenu_level = 2;
static const int jmenu_match = 3;
static const int jmenu_red = 5;
static const int jmenu_blue = 7;
static const int jmenu_chase = 9;
static const int jmenu_reqmatch = 11;


pmenu_t joinmenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL },
	{ "*ThreeWave Capture the Flag",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "Join Red Team",		PMENU_ALIGN_LEFT, CTFJoinTeam1 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Join Blue Team",		PMENU_ALIGN_LEFT, CTFJoinTeam2 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Chase Camera",		PMENU_ALIGN_LEFT, CTFChaseCam },
	{ "Credits",			PMENU_ALIGN_LEFT, CTFCredits },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL },
	{ "v" CTF_STRING_VERSION,	PMENU_ALIGN_RIGHT, NULL },
};

pmenu_t nochasemenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL },
	{ "*ThreeWave Capture the Flag",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "No one to chase",	PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "Return to Main Menu", PMENU_ALIGN_LEFT, CTFReturnToMain }
};

static const int ttctf_jmenu_level = 2;
static const int ttctf_jmenu_match = 3;
static const int ttctf_jmenu_red = 4;
static const int ttctf_jmenu_blue = 6;
static const int ttctf_jmenu_green = 8;
static const int ttctf_jmenu_chase = 10;
static const int ttctf_jmenu_reqmatch = 12;

pmenu_t ttctf_joinmenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL },
	{ "*3Team CTF",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "Join Red Team",		PMENU_ALIGN_LEFT, CTFJoinTeam1 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Join Blue Team",		PMENU_ALIGN_LEFT, CTFJoinTeam2 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Join Green Team",	PMENU_ALIGN_LEFT, CTFJoinTeam3 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Chase Camera",		PMENU_ALIGN_LEFT, CTFChaseCam },
	{ "Credits",			PMENU_ALIGN_LEFT, CTFCredits },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL },
	{ "v" CTF_STRING_VERSION,	PMENU_ALIGN_RIGHT, NULL },
};


pmenu_t ttctf_nochasemenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL },
	{ "*3Team CTF",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "No one to chase",	PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "Return to Main Menu", PMENU_ALIGN_LEFT, CTFReturnToMain }
};

void CTFJoinTeam (edict_t *ent, int desired_team)
{
	char *s;

	// Knightmare added
	if (ent->solid == SOLID_NOT && ctfgame.match > MATCH_SETUP) {
		safe_cprintf(ent, PRINT_HIGH, "Can't change teams in a match.\n");
		return;
	}

	PMenu_Close(ent);

	gi.dprintf ("joining team\n");

	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = 0;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

	// assign a ghost if we are in match mode
	if (ctfgame.match == MATCH_GAME) {
		if (ent->client->resp.ghost)
			ent->client->resp.ghost->code = 0;
		ent->client->resp.ghost = NULL;
		CTFAssignGhost(ent);
	}

	PutClientInServer (ent);
	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
	safe_bprintf(PRINT_HIGH, "%s joined the %s team.\n",
		ent->client->pers.netname, CTFTeamName(desired_team));

	if (ctfgame.match == MATCH_SETUP) {
		safe_centerprintf(ent,	"***********************\n"
								"Type \"ready\" in console\n"
								"to ready up.\n"
								"***********************");
	}
}

void CTFJoinTeam1 (edict_t *ent, pmenuhnd_t *p)
{
	CTFJoinTeam(ent, CTF_TEAM1);
}

void CTFJoinTeam2 (edict_t *ent, pmenuhnd_t *p)
{
	CTFJoinTeam(ent, CTF_TEAM2);
}

// Knightmare added
void CTFJoinTeam3 (edict_t *ent, pmenuhnd_t *p)
{
	CTFJoinTeam(ent, CTF_TEAM3);
}

pmenuhnd_t *PMenu_Open (edict_t *ent, pmenu_t *entries, int cur, int num, void *arg);
void CTFChaseCam (edict_t *ent, pmenuhnd_t *p)
{
	int i;
	edict_t *e;

	if (ent->client->chase_target)
	{
		ent->client->chase_target = NULL;
		ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		PMenu_Close(ent);
		return;
	}

	for (i = 1; i <= maxclients->value; i++)
	{
		e = g_edicts + i;
		if (e->inuse && e->solid != SOLID_NOT && e != ent)
		{
			ent->client->chase_target = e;
		#ifdef KMQUAKE2_ENGINE_MOD
			// Knightmare-  turn off client-side chasecam
			stuffcmd (ent, CLIENT_THIRDPERSON_CVAR" 0");
		#endif
			PMenu_Close(ent);
			ent->client->update_chase = true;
			return;
		}
	}

	// Knightmare added
	if (ttctf->value) {
		SetLevelName(ttctf_nochasemenu + jmenu_level);
		PMenu_Close(ent);
		PMenu_Open(ent, ttctf_nochasemenu, -1, sizeof(ttctf_nochasemenu) / sizeof(pmenu_t), NULL);
	}
	else {
		SetLevelName(nochasemenu + jmenu_level);
		PMenu_Close(ent);
		PMenu_Open(ent, nochasemenu, -1, sizeof(nochasemenu) / sizeof(pmenu_t), NULL);
	}
}

void CTFReturnToMain (edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	if (ttctf->value)
		TTCTFOpenJoinMenu(ent);
	else
		CTFOpenJoinMenu(ent);
}

void CTFRequestMatch (edict_t *ent, pmenuhnd_t *p)
{
	char text[1024];

	PMenu_Close(ent);

	Com_sprintf (text, sizeof(text), "%s has requested to switch to competition mode.",
		ent->client->pers.netname);
	CTFBeginElection(ent, ELECT_MATCH, text);
}

void DeathmatchScoreboard (edict_t *ent);

void CTFShowScores (edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	ent->client->showscores = true;
	ent->client->showinventory = false;
	DeathmatchScoreboard (ent);
}

int CTFUpdateJoinMenu (edict_t *ent)
{
	static char team1players[32];
	static char team2players[32];
	int num1, num2, num3, i;
	float r;

	if (ctfgame.match >= MATCH_PREGAME && matchlock->value)
	{
		joinmenu[jmenu_red].text = "MATCH IS LOCKED";
		joinmenu[jmenu_red].SelectFunc = NULL;
		joinmenu[jmenu_blue].text = "  (entry is not permitted)";
		joinmenu[jmenu_blue].SelectFunc = NULL;
	}
	else
	{
		if (ctfgame.match >= MATCH_PREGAME)
		{
			joinmenu[jmenu_red].text = "Join Red MATCH Team";
			joinmenu[jmenu_blue].text = "Join Blue MATCH Team";
		}
		else
		{
			joinmenu[jmenu_red].text = "Join Red Team";
			joinmenu[jmenu_blue].text = "Join Blue Team";
		}
		joinmenu[jmenu_red].SelectFunc = CTFJoinTeam1;
		joinmenu[jmenu_blue].SelectFunc = CTFJoinTeam2;
	}

	if (ctf_forcejoin->string && *ctf_forcejoin->string)
	{
		if (Q_stricmp(ctf_forcejoin->string, "red") == 0) {
			joinmenu[jmenu_blue].text = NULL;
			joinmenu[jmenu_blue].SelectFunc = NULL;
		} else if (Q_stricmp(ctf_forcejoin->string, "blue") == 0) {
			joinmenu[jmenu_red].text = NULL;
			joinmenu[jmenu_red].SelectFunc = NULL;
		} 
	}

	if (ent->client->chase_target)
		joinmenu[jmenu_chase].text = "Leave Chase Camera";
	else
		joinmenu[jmenu_chase].text = "Chase Camera";

	SetLevelName(joinmenu + jmenu_level);

	num1 = num2 = num3 = 0;
	for (i = 0; i < maxclients->value; i++)
	{
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			num1++;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			num2++;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM3)
			num3++;
	}

	Com_sprintf (team1players, sizeof(team1players), "  (%d players)", num1);
	Com_sprintf (team2players, sizeof(team2players), "  (%d players)", num2);

	switch (ctfgame.match)
	{
	case MATCH_NONE :
		joinmenu[jmenu_match].text = NULL;
		break;

	case MATCH_SETUP :
		joinmenu[jmenu_match].text = "*MATCH SETUP IN PROGRESS";
		break;

	case MATCH_PREGAME :
		joinmenu[jmenu_match].text = "*MATCH STARTING";
		break;

	case MATCH_GAME :
		joinmenu[jmenu_match].text = "*MATCH IN PROGRESS";
		break;
	}

	if (joinmenu[jmenu_red].text)
		joinmenu[jmenu_red+1].text = team1players;
	else
		joinmenu[jmenu_red+1].text = NULL;

	if (joinmenu[jmenu_blue].text)
		joinmenu[jmenu_blue+1].text = team2players;
	else
		joinmenu[jmenu_blue+1].text = NULL;

	joinmenu[jmenu_reqmatch].text = NULL;
	joinmenu[jmenu_reqmatch].SelectFunc = NULL;
	if (competition->value && ctfgame.match < MATCH_SETUP) {
		joinmenu[jmenu_reqmatch].text = "Request Match";
		joinmenu[jmenu_reqmatch].SelectFunc = CTFRequestMatch;
	}
	
	r = random();
	if (num1 > num2)
		return CTF_TEAM1;
	else if (num2 > num1)
		return CTF_TEAM2;
	else if (r < 0.5)
		return CTF_TEAM1;
	else
		return CTF_TEAM2;
}

// Knightmare added
int TTCTFUpdateJoinMenu (edict_t *ent)
{
	static char team1players[32];
	static char team2players[32];
	static char team3players[32];
	int num1, num2, num3, i;
	float r;

	if (ctfgame.match >= MATCH_PREGAME && matchlock->value)
	{
		ttctf_joinmenu[ttctf_jmenu_red].text = "MATCH IS LOCKED";
		ttctf_joinmenu[ttctf_jmenu_red].SelectFunc = NULL;
		ttctf_joinmenu[ttctf_jmenu_blue].text = "  (entry is not permitted)";
		ttctf_joinmenu[ttctf_jmenu_blue].SelectFunc = NULL;
		ttctf_joinmenu[ttctf_jmenu_green].text = NULL;
		ttctf_joinmenu[ttctf_jmenu_green].SelectFunc = NULL;
	}
	else
	{
		if (ctfgame.match >= MATCH_PREGAME)
		{
			ttctf_joinmenu[ttctf_jmenu_red].text = "Join Red MATCH Team";
			ttctf_joinmenu[ttctf_jmenu_blue].text = "Join Blue MATCH Team";
			ttctf_joinmenu[ttctf_jmenu_green].text = "Join Green MATCH Team";
		}
		else
		{
			ttctf_joinmenu[ttctf_jmenu_red].text = "Join Red Team";
			ttctf_joinmenu[ttctf_jmenu_blue].text = "Join Blue Team";
			ttctf_joinmenu[ttctf_jmenu_green].text = "Join Green Team";
		}
		ttctf_joinmenu[ttctf_jmenu_red].SelectFunc = CTFJoinTeam1;
		ttctf_joinmenu[ttctf_jmenu_blue].SelectFunc = CTFJoinTeam2;
		ttctf_joinmenu[ttctf_jmenu_green].SelectFunc = CTFJoinTeam3;
	}

	if (ctf_forcejoin->string && *ctf_forcejoin->string)
	{
		if (Q_stricmp(ctf_forcejoin->string, "red") == 0) {
			ttctf_joinmenu[ttctf_jmenu_blue].text = NULL;
			ttctf_joinmenu[ttctf_jmenu_blue].SelectFunc = NULL;
			ttctf_joinmenu[ttctf_jmenu_green].text = NULL;
			ttctf_joinmenu[ttctf_jmenu_green].SelectFunc = NULL;
		} else if (Q_stricmp(ctf_forcejoin->string, "blue") == 0) {
			ttctf_joinmenu[ttctf_jmenu_red].text = NULL;
			ttctf_joinmenu[ttctf_jmenu_red].SelectFunc = NULL;
			ttctf_joinmenu[ttctf_jmenu_green].text = NULL;
			ttctf_joinmenu[ttctf_jmenu_green].SelectFunc = NULL;
		} 
		else if (Q_stricmp(ctf_forcejoin->string, "green") == 0) {
			ttctf_joinmenu[ttctf_jmenu_red].text = NULL;
			ttctf_joinmenu[ttctf_jmenu_red].SelectFunc = NULL;
			ttctf_joinmenu[ttctf_jmenu_blue].text = NULL;
			ttctf_joinmenu[ttctf_jmenu_blue].SelectFunc = NULL;
		}
	}

	if (ent->client->chase_target)
		ttctf_joinmenu[ttctf_jmenu_chase].text = "Leave Chase Camera";
	else
		ttctf_joinmenu[ttctf_jmenu_chase].text = "Chase Camera";

	SetLevelName(ttctf_joinmenu + jmenu_level);

	num1 = num2 = num3 = 0;
	for (i = 0; i < maxclients->value; i++)
	{
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			num1++;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			num2++;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM3)
			num3++;
	}

	Com_sprintf (team1players, sizeof(team1players), "  (%d players)", num1);
	Com_sprintf (team2players, sizeof(team2players), "  (%d players)", num2);
	Com_sprintf (team3players, sizeof(team3players), "  (%d players)", num3);

	switch (ctfgame.match)
	{
	case MATCH_NONE :
		ttctf_joinmenu[ttctf_jmenu_match].text = NULL;
		break;

	case MATCH_SETUP :
		ttctf_joinmenu[ttctf_jmenu_match].text = "*MATCH SETUP IN PROGRESS";
		break;

	case MATCH_PREGAME :
		ttctf_joinmenu[ttctf_jmenu_match].text = "*MATCH STARTING";
		break;

	case MATCH_GAME :
		ttctf_joinmenu[ttctf_jmenu_match].text = "*MATCH IN PROGRESS";
		break;
	}

	if (ttctf_joinmenu[ttctf_jmenu_red].text)
		ttctf_joinmenu[ttctf_jmenu_red+1].text = team1players;
	else
		ttctf_joinmenu[ttctf_jmenu_red+1].text = NULL;

	if (ttctf_joinmenu[ttctf_jmenu_blue].text)
		ttctf_joinmenu[ttctf_jmenu_blue+1].text = team2players;
	else
		ttctf_joinmenu[ttctf_jmenu_blue+1].text = NULL;

	if (ttctf_joinmenu[ttctf_jmenu_green].text)
		ttctf_joinmenu[ttctf_jmenu_green+1].text = team3players;
	else
		ttctf_joinmenu[ttctf_jmenu_green+1].text = NULL;

	ttctf_joinmenu[ttctf_jmenu_reqmatch].text = NULL;
	ttctf_joinmenu[ttctf_jmenu_reqmatch].SelectFunc = NULL;
	if (competition->value && ctfgame.match < MATCH_SETUP) {
		ttctf_joinmenu[ttctf_jmenu_reqmatch].text = "Request Match";
		ttctf_joinmenu[ttctf_jmenu_reqmatch].SelectFunc = CTFRequestMatch;
	}
	
	r = random();
	if (num1 > num2 && num1 > num3)
		return CTF_TEAM1;
	else if (num2 > num1 && num2 > num3)
		return CTF_TEAM2;
	else if (num3 > num1 && num3 > num2)
		return CTF_TEAM3;
	// random
	else if (r < 0.33)
		return CTF_TEAM1;
	else if (r < 0.67)
		return CTF_TEAM2;
	else
		return CTF_TEAM3;
}

void CTFOpenJoinMenu (edict_t *ent)
{
	int team;

	if (ent->client->textdisplay)
		Text_Close(ent);

	if (ent->client->showinventory)
		ent->client->showinventory = false;

	team = CTFUpdateJoinMenu(ent);
	if (ent->client->chase_target)
		team = 9;
	else if (team == CTF_TEAM1)
		team = 5;
	else if (team == CTF_TEAM2)
		team = 7;
	PMenu_Open(ent, joinmenu, team, sizeof(joinmenu) / sizeof(pmenu_t), NULL);
}

// Knightmare added
void TTCTFOpenJoinMenu (edict_t *ent)
{
	int team;

	if (ent->client->textdisplay)
		Text_Close(ent);

	if (ent->client->showinventory)
		ent->client->showinventory = false;

	team = TTCTFUpdateJoinMenu(ent);
	if (ent->client->chase_target)
		team = 10;
	else if (team == CTF_TEAM1)
		team = 4;
	else if (team == CTF_TEAM2)
		team = 6;
	else if (team == CTF_TEAM3)
		team = 8;
	PMenu_Open(ent, ttctf_joinmenu, team, sizeof(ttctf_joinmenu) / sizeof(pmenu_t), NULL);
}

void CTFCredits (edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu, -1, sizeof(creditsmenu) / sizeof(pmenu_t), NULL);
}

qboolean CTFStartClient (edict_t *ent)
{
	if (ent->client->resp.ctf_team != CTF_NOTEAM)
		return false;

	if (!((int)dmflags->value & DF_CTF_FORCEJOIN) || ctfgame.match >= MATCH_SETUP) {
		// start as 'observer'
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);

		if (ttctf->value)
			TTCTFOpenJoinMenu(ent);
		else
			CTFOpenJoinMenu(ent);
		return true;
	}
	return false;
}

void CTFObserver (edict_t *ent)
{
	char		userinfo[MAX_INFO_STRING];

	// start as 'observer'
	if (ent->movetype == MOVETYPE_NOCLIP)

	CTFPlayerResetGrapple(ent);
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);

	ent->deadflag = DEAD_NO;
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->resp.ctf_team = CTF_NOTEAM;
	ent->client->ps.gunindex = 0;
	ent->client->resp.score = 0;
	memcpy (userinfo, ent->client->pers.userinfo, sizeof(userinfo));
	InitClientPersistant (ent->client, 0);
	ClientUserinfoChanged (ent, userinfo);
	gi.linkentity (ent);
	if (ttctf->value)
		TTCTFOpenJoinMenu(ent);
	else
		CTFOpenJoinMenu(ent);
}

qboolean CTFInMatch (void)
{
	if (ctfgame.match > MATCH_NONE)
		return true;
	return false;
}

qboolean CTFCheckRules (void)
{
	int t;
	int i, j;
	char text[64];
	edict_t *ent;

	if (ctfgame.election != ELECT_NONE && ctfgame.electtime <= level.time) {
		safe_bprintf(PRINT_CHAT, "Election timed out and has been cancelled.\n");
		ctfgame.election = ELECT_NONE;
	}

	if (ctfgame.match != MATCH_NONE) {
		t = ctfgame.matchtime - level.time;

		// no team warnings in match mode
		ctfgame.warnactive = 0;

		if (t <= 0) { // time ended on something
			switch (ctfgame.match) {
			case MATCH_SETUP :
				// go back to normal mode
				if (competition->value < 3) {
					ctfgame.match = MATCH_NONE;
					gi.cvar_set("competition", "1");
					CTFResetAllPlayers();
				} else {
					// reset the time
					ctfgame.matchtime = level.time + matchsetuptime->value * 60;
				}
				return false;

			case MATCH_PREGAME :
				// match started!
				CTFStartMatch();
				gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NONE, 0);
				return false;

			case MATCH_GAME :
				// match ended!
				CTFEndMatch();
				gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/bigtele.wav"), 1, ATTN_NONE, 0);
				return false;
			}
		}

		if (t == ctfgame.lasttime)
			return false;

		ctfgame.lasttime = t;

		switch (ctfgame.match) {
		case MATCH_SETUP :
			for (j = 0, i = 1; i <= maxclients->value; i++) {
				ent = g_edicts + i;
				if (!ent->inuse)
					continue;
				if (ent->client->resp.ctf_team != CTF_NOTEAM &&
					!ent->client->resp.ready)
					j++;
			}

			if (competition->value < 3)
				Com_sprintf (text, sizeof(text), "%02d:%02d SETUP: %d not ready",
					t / 60, t % 60, j);
			else
				Com_sprintf(text, sizeof(text), "SETUP: %d not ready", j);

			gi.configstring (CONFIG_CTF_MATCH, text);
			break;


		case MATCH_PREGAME :
			Com_sprintf (text, sizeof(text), "%02d:%02d UNTIL START",
				t / 60, t % 60);
			gi.configstring (CONFIG_CTF_MATCH, text);

			if (t <= 10 && !ctfgame.countdown) {
				ctfgame.countdown = true;
				gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);
			}
			break;

		case MATCH_GAME:
			Com_sprintf (text, sizeof(text), "%02d:%02d MATCH",
				t / 60, t % 60);
			gi.configstring (CONFIG_CTF_MATCH, text);
			if (t <= 10 && !ctfgame.countdown) {
				ctfgame.countdown = true;
				gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);
			}
			break;
		}
		return false;

	} else {
		int team1 = 0, team2 = 0;

		if (level.time == ctfgame.lasttime)
			return false;
		ctfgame.lasttime = level.time;
		// this is only done in non-match (public) mode

		if (warn_unbalanced->value) {
			// count up the team totals
			for (i = 1; i <= maxclients->value; i++) {
				ent = g_edicts + i;
				if (!ent->inuse)
					continue;
				if (ent->client->resp.ctf_team == CTF_TEAM1)
					team1++;
				else if (ent->client->resp.ctf_team == CTF_TEAM2)
					team2++;
			}

			if (team1 - team2 >= 2 && team2 >= 2) {
				if (ctfgame.warnactive != CTF_TEAM1) {
					ctfgame.warnactive = CTF_TEAM1;
					gi.configstring (CONFIG_CTF_TEAMINFO, "WARNING: Red has too many players");
				}
			} else if (team2 - team1 >= 2 && team1 >= 2) {
				if (ctfgame.warnactive != CTF_TEAM2) {
					ctfgame.warnactive = CTF_TEAM2;
					gi.configstring (CONFIG_CTF_TEAMINFO, "WARNING: Blue has too many players");
				}
			} else
				ctfgame.warnactive = 0;
		} else
			ctfgame.warnactive = 0;

	}



	if (capturelimit->value && 
		(ctfgame.team1 >= capturelimit->value ||
		ctfgame.team2 >= capturelimit->value)) {
		safe_bprintf (PRINT_HIGH, "Capturelimit hit.\n");
		return true;
	}
	return false;
}

/*--------------------------------------------------------------------------
 * just here to help old map conversions
 *--------------------------------------------------------------------------*/

/*static*/ void old_teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;
	int			i;
	vec3_t		forward;

	if (!other->client)
		return;
	dest = G_Find (NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

//ZOID
	CTFPlayerResetGrapple(other);
//ZOID

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	VectorCopy (dest->s.origin, other->s.old_origin);
//	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	other->client->ps.pmove.pm_time = 160>>3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->enemy->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i=0 ; i<3 ; i++)
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

	other->s.angles[PITCH] = 0;
	other->s.angles[YAW] = dest->s.angles[YAW];
	other->s.angles[ROLL] = 0;
	VectorCopy (dest->s.angles, other->client->ps.viewangles);
	VectorCopy (dest->s.angles, other->client->v_angle);

	// give a little forward velocity
	AngleVectors (other->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 200, other->velocity);

	// kill anything at the destination
	if (!KillBox (other))
	{
	}

	gi.linkentity (other);
}

/*QUAKED trigger_teleport (0.5 0.5 0.5) ?
Players touching this will be teleported
*/
void SP_trigger_teleport (edict_t *ent)
{
	edict_t *s;
	int i;

	if (!ent->target)
	{
		gi.dprintf ("teleporter without a target.\n");
		G_FreeEdict (ent);
		return;
	}

	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->touch = old_teleporter_touch;
	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);

	// noise maker and splash effect dude
	s = G_Spawn();
	ent->enemy = s;
	for (i = 0; i < 3; i++)
		s->s.origin[i] = ent->mins[i] + (ent->maxs[i] - ent->mins[i])/2;
	s->s.sound = gi.soundindex ("world/hum1.wav");
	gi.linkentity(s);
	
}

/*QUAKED info_teleport_destination (0.5 0.5 0.5) (-16 -16 -24) (16 16 32)
Point trigger_teleports at these.
*/
void SP_info_teleport_destination (edict_t *ent)
{
	ent->s.origin[2] += 16;
}

/*----------------------------------------------------------------------------------*/
/* ADMIN */

typedef struct admin_settings_s {
	int matchlen;
	int matchsetuplen;
	int matchstartlen;
	qboolean weaponsstay;
	qboolean instantitems;
	qboolean quaddrop;
	qboolean instantweap;
	qboolean matchlock;
} admin_settings_t;

#define SETMENU_SIZE (7 + 5)

void CTFAdmin_UpdateSettings(edict_t *ent, pmenuhnd_t *setmenu);
void CTFOpenAdminMenu(edict_t *ent);

void CTFAdmin_SettingsApply(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;
	char st[80];
	int i;

	if (settings->matchlen != matchtime->value) {
		safe_bprintf(PRINT_HIGH, "%s changed the match length to %d minutes.\n",
			ent->client->pers.netname, settings->matchlen);
		if (ctfgame.match == MATCH_GAME) {
			// in the middle of a match, change it on the fly
			ctfgame.matchtime = (ctfgame.matchtime - matchtime->value*60) + settings->matchlen*60;
		} 
		Com_sprintf (st, sizeof(st), "%d", settings->matchlen);
		gi.cvar_set("matchtime", st);
	}

	if (settings->matchsetuplen != matchsetuptime->value) {
		safe_bprintf(PRINT_HIGH, "%s changed the match setup time to %d minutes.\n",
			ent->client->pers.netname, settings->matchsetuplen);
		if (ctfgame.match == MATCH_SETUP) {
			// in the middle of a match, change it on the fly
			ctfgame.matchtime = (ctfgame.matchtime - matchsetuptime->value*60) + settings->matchsetuplen*60;
		} 
		Com_sprintf (st, sizeof(st), "%d", settings->matchsetuplen);
		gi.cvar_set("matchsetuptime", st);
	}

	if (settings->matchstartlen != matchstarttime->value) {
		safe_bprintf(PRINT_HIGH, "%s changed the match start time to %d seconds.\n",
			ent->client->pers.netname, settings->matchstartlen);
		if (ctfgame.match == MATCH_PREGAME) {
			// in the middle of a match, change it on the fly
			ctfgame.matchtime = (ctfgame.matchtime - matchstarttime->value) + settings->matchstartlen;
		} 
		Com_sprintf (st, sizeof(st), "%d", settings->matchstartlen);
		gi.cvar_set("matchstarttime", st);
	}

	if (settings->weaponsstay != !!((int)dmflags->value & DF_WEAPONS_STAY)) {
		safe_bprintf(PRINT_HIGH, "%s turned %s weapons stay.\n",
			ent->client->pers.netname, settings->weaponsstay ? "on" : "off");
		i = (int)dmflags->value;
		if (settings->weaponsstay)
			i |= DF_WEAPONS_STAY;
		else
			i &= ~DF_WEAPONS_STAY;
		Com_sprintf (st, sizeof(st), "%d", i);
		gi.cvar_set("dmflags", st);
	}

	if (settings->instantitems != !!((int)dmflags->value & DF_INSTANT_ITEMS)) {
		safe_bprintf(PRINT_HIGH, "%s turned %s instant items.\n",
			ent->client->pers.netname, settings->instantitems ? "on" : "off");
		i = (int)dmflags->value;
		if (settings->instantitems)
			i |= DF_INSTANT_ITEMS;
		else
			i &= ~DF_INSTANT_ITEMS;
		Com_sprintf (st, sizeof(st), "%d", i);
		gi.cvar_set("dmflags", st);
	}

	if (settings->quaddrop != !!((int)dmflags->value & DF_QUAD_DROP)) {
		safe_bprintf(PRINT_HIGH, "%s turned %s quad drop.\n",
			ent->client->pers.netname, settings->quaddrop ? "on" : "off");
		i = (int)dmflags->value;
		if (settings->quaddrop)
			i |= DF_QUAD_DROP;
		else
			i &= ~DF_QUAD_DROP;
		Com_sprintf (st, sizeof(st), "%d", i);
		gi.cvar_set("dmflags", st);
	}

	if (settings->instantweap != !!((int)instantweap->value)) {
		safe_bprintf(PRINT_HIGH, "%s turned %s instant weapons.\n",
			ent->client->pers.netname, settings->instantweap ? "on" : "off");
		Com_sprintf (st, sizeof(st), "%d", (int)settings->instantweap);
		gi.cvar_set("instantweap", st);
	}

	if (settings->matchlock != !!((int)matchlock->value)) {
		safe_bprintf(PRINT_HIGH, "%s turned %s match lock.\n",
			ent->client->pers.netname, settings->matchlock ? "on" : "off");
		Com_sprintf (st, sizeof(st), "%d", (int)settings->matchlock);
		gi.cvar_set("matchlock", st);
	}

	PMenu_Close(ent);
	CTFOpenAdminMenu(ent);
}

void CTFAdmin_SettingsCancel(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	PMenu_Close(ent);
	CTFOpenAdminMenu(ent);
}

void CTFAdmin_ChangeMatchLen(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->matchlen = (settings->matchlen % 60) + 5;
	if (settings->matchlen < 5)
		settings->matchlen = 5;

	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_ChangeMatchSetupLen(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->matchsetuplen = (settings->matchsetuplen % 60) + 5;
	if (settings->matchsetuplen < 5)
		settings->matchsetuplen = 5;

	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_ChangeMatchStartLen(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->matchstartlen = (settings->matchstartlen % 600) + 10;
	if (settings->matchstartlen < 20)
		settings->matchstartlen = 20;

	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_ChangeWeapStay(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->weaponsstay = !settings->weaponsstay;
	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_ChangeInstantItems(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->instantitems = !settings->instantitems;
	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_ChangeQuadDrop(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->quaddrop = !settings->quaddrop;
	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_ChangeInstantWeap(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->instantweap = !settings->instantweap;
	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_ChangeMatchLock(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->matchlock = !settings->matchlock;
	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_UpdateSettings(edict_t *ent, pmenuhnd_t *setmenu)
{
	int i = 2;
	char text[64];
	admin_settings_t *settings = setmenu->arg;

	Com_sprintf (text, sizeof(text), "Match Len:       %2d mins", settings->matchlen);
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeMatchLen);
	i++;

	Com_sprintf (text, sizeof(text), "Match Setup Len: %2d mins", settings->matchsetuplen);
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeMatchSetupLen);
	i++;

	Com_sprintf (text, sizeof(text), "Match Start Len: %2d secs", settings->matchstartlen);
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeMatchStartLen);
	i++;

	Com_sprintf (text, sizeof(text), "Weapons Stay:    %s", settings->weaponsstay ? "Yes" : "No");
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeWeapStay);
	i++;

	Com_sprintf (text, sizeof(text), "Instant Items:   %s", settings->instantitems ? "Yes" : "No");
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeInstantItems);
	i++;

	Com_sprintf (text, sizeof(text), "Quad Drop:       %s", settings->quaddrop ? "Yes" : "No");
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeQuadDrop);
	i++;

	Com_sprintf (text, sizeof(text), "Instant Weapons: %s", settings->instantweap ? "Yes" : "No");
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeInstantWeap);
	i++;

	Com_sprintf (text, sizeof(text), "Match Lock:      %s", settings->matchlock ? "Yes" : "No");
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeMatchLock);
	i++;

	PMenu_Update(ent);
}

pmenu_t def_setmenu[] = {
	{ "*Settings Menu", PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//int matchlen;         
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//int matchsetuplen;    
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//int matchstartlen;    
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//qboolean weaponsstay; 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//qboolean instantitems;
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//qboolean quaddrop;    
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//qboolean instantweap; 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//qboolean matchlock; 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Apply",			PMENU_ALIGN_LEFT, CTFAdmin_SettingsApply },
	{ "Cancel",			PMENU_ALIGN_LEFT, CTFAdmin_SettingsCancel }
};

void CTFAdmin_Settings(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings;
	pmenuhnd_t *menu;

	PMenu_Close(ent);

//	settings = malloc(sizeof(*settings));
	settings = gi.TagMalloc(sizeof(*settings), TAG_LEVEL);

	settings->matchlen = matchtime->value;
	settings->matchsetuplen = matchsetuptime->value;
	settings->matchstartlen = matchstarttime->value;
	settings->weaponsstay = !!((int)dmflags->value & DF_WEAPONS_STAY);
	settings->instantitems = !!((int)dmflags->value & DF_INSTANT_ITEMS);
	settings->quaddrop = !!((int)dmflags->value & DF_QUAD_DROP);
	settings->instantweap = instantweap->value != 0;
	settings->matchlock = matchlock->value != 0;

	menu = PMenu_Open(ent, def_setmenu, -1, sizeof(def_setmenu) / sizeof(pmenu_t), settings);
	CTFAdmin_UpdateSettings(ent, menu);
}

void CTFAdmin_MatchSet(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);

	if (ctfgame.match == MATCH_SETUP) {
		safe_bprintf(PRINT_CHAT, "Match has been forced to start.\n");
		ctfgame.match = MATCH_PREGAME;
		ctfgame.matchtime = level.time + matchstarttime->value;
		gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/talk1.wav"), 1, ATTN_NONE, 0);
		ctfgame.countdown = false;
	} else if (ctfgame.match == MATCH_GAME) {
		safe_bprintf(PRINT_CHAT, "Match has been forced to terminate.\n");
		ctfgame.match = MATCH_SETUP;
		ctfgame.matchtime = level.time + matchsetuptime->value * 60;
		CTFResetAllPlayers();
	}
}

void CTFAdmin_MatchMode(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);

	if (ctfgame.match != MATCH_SETUP) {
		if (competition->value < 3)
			gi.cvar_set("competition", "2");
		ctfgame.match = MATCH_SETUP;
		CTFResetAllPlayers();
	}
}

void CTFAdmin_Reset(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);

	// go back to normal mode
	safe_bprintf(PRINT_CHAT, "Match mode has been terminated, reseting to normal game.\n");
	ctfgame.match = MATCH_NONE;
	gi.cvar_set("competition", "1");
	CTFResetAllPlayers();
}

void CTFAdmin_Cancel(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
}


pmenu_t adminmenu[] = {
	{ "*Administration Menu",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,						PMENU_ALIGN_CENTER, NULL }, // blank
	{ "Settings",				PMENU_ALIGN_LEFT, CTFAdmin_Settings },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ "Cancel",					PMENU_ALIGN_LEFT, CTFAdmin_Cancel },
	{ NULL,						PMENU_ALIGN_CENTER, NULL },
};

void CTFOpenAdminMenu(edict_t *ent)
{
	adminmenu[3].text = NULL;
	adminmenu[3].SelectFunc = NULL;
	adminmenu[4].text = NULL;
	adminmenu[4].SelectFunc = NULL;
	if (ctfgame.match == MATCH_SETUP) {
		adminmenu[3].text = "Force start match";
		adminmenu[3].SelectFunc = CTFAdmin_MatchSet;
		adminmenu[4].text = "Reset to pickup mode";
		adminmenu[4].SelectFunc = CTFAdmin_Reset;
	} else if (ctfgame.match == MATCH_GAME || ctfgame.match == MATCH_PREGAME) {
		adminmenu[3].text = "Cancel match";
		adminmenu[3].SelectFunc = CTFAdmin_MatchSet;
	} else if (ctfgame.match == MATCH_NONE && competition->value) {
		adminmenu[3].text = "Switch to match mode";
		adminmenu[3].SelectFunc = CTFAdmin_MatchMode;
	}


//	if (ent->client->menu)
//		PMenu_Close(ent->client->menu);

	PMenu_Open(ent, adminmenu, -1, sizeof(adminmenu) / sizeof(pmenu_t), NULL);
}

void CTFAdmin(edict_t *ent)
{
	char text[1024];

	if (!allow_admin->value) {
		safe_cprintf(ent, PRINT_HIGH, "Administration is disabled\n");
		return;
	}

	if (gi.argc() > 1 && admin_password->string && *admin_password->string &&
		!ent->client->resp.admin && strcmp(admin_password->string, gi.argv(1)) == 0) {
		ent->client->resp.admin = true;
		safe_bprintf(PRINT_HIGH, "%s has become an admin.\n", ent->client->pers.netname);
		safe_cprintf(ent, PRINT_HIGH, "Type 'admin' to access the adminstration menu.\n");
	}

	if (!ent->client->resp.admin) {
		Com_sprintf (text, sizeof(text), "%s has requested admin rights.",
			ent->client->pers.netname);
		CTFBeginElection(ent, ELECT_ADMIN, text);
		return;
	}

	if (ent->client->menu)
		PMenu_Close(ent);

	CTFOpenAdminMenu(ent);
}

/*----------------------------------------------------------------*/

void CTFStats(edict_t *ent)
{
	int		i, e;
	ghost_t *g;
	char	st[80];
	char	text[1024];
	char	tmp[128];	// Knightmare added
	edict_t	*e2;

	*text = 0;
	if (ctfgame.match == MATCH_SETUP)
	{
		for (i = 1; i <= maxclients->value; i++)
		{
			e2 = g_edicts + i;
			if (!e2->inuse)
				continue;
			if (!e2->client->resp.ready && e2->client->resp.ctf_team != CTF_NOTEAM)
			{
				Com_sprintf (st, sizeof(st), "%s is not ready.\n", e2->client->pers.netname);
				if (strlen(text) + strlen(st) < sizeof(text) - 50)
				//	strncat(text, st);
					Q_strncatz(text, sizeof(text), st);
			}
		}
	}

	for (i = 0, g = ctfgame.ghosts; i < MAX_CLIENTS; i++, g++)
		if (g->ent)
			break;

	if (i == MAX_CLIENTS)
	{
		if (*text)
			safe_cprintf(ent, PRINT_HIGH, "%s", text);
		safe_cprintf(ent, PRINT_HIGH, "No statistics available.\n");
		return;
	}

//	strncat(text, "  #|Name            |Score|Kills|Death|BasDf|CarDf|Effcy|\n");
	Q_strncatz(text, sizeof(text), "  #|Name            |Score|Kills|Death|BasDf|CarDf|Effcy|\n");

	for (i = 0, g = ctfgame.ghosts; i < MAX_CLIENTS; i++, g++)
	{
		if (!*g->netname)
			continue;

		if (g->deaths + g->kills == 0)
			e = 50;
		else
			e = g->kills * 100 / (g->kills + g->deaths);
		Com_sprintf (st, sizeof(st), "%3d|%-16.16s|%5d|%5d|%5d|%5d|%5d|%4d%%|\n",
			g->number, 
			g->netname, 
			g->score, 
			g->kills, 
			g->deaths, 
			g->basedef,
			g->carrierdef, 
			e);
		if (strlen(text) + strlen(st) > sizeof(text) - 50)
		{
		//	sprintf(text+strlen(text), "And more...\n");
			Com_sprintf (tmp, sizeof(tmp), "And more...\n");
			Q_strncatz (text, sizeof(text), tmp);
			safe_cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
	//	strncat(text, st);
		Q_strncatz(text, sizeof(text), st);
	}
	safe_cprintf(ent, PRINT_HIGH, "%s", text);
}

void CTFPlayerList(edict_t *ent)
{
	int		i;
	char	st[80];
	char	text[1400];
	char	tmp[128];	// Knightmare added
	edict_t	*e2;

#if 0
	*text = 0;
	if (ctfgame.match == MATCH_SETUP)
	{
		for (i = 1; i <= maxclients->value; i++)
		{
			e2 = g_edicts + i;
			if (!e2->inuse)
				continue;
			if (!e2->client->resp.ready && e2->client->resp.ctf_team != CTF_NOTEAM) {
				Com_sprintf (st, sizeof(st), "%s is not ready.\n", e2->client->pers.netname);
				if (strlen(text) + strlen(st) < sizeof(text) - 50)
				//	strncat(text, st);
					Q_strncatz (text, sizeof(text), st);
			}
		}
	}
#endif

	// number, name, connect time, ping, score, admin

	*text = 0;
	for (i = 1; i <= maxclients->value; i++) {
		e2 = g_edicts + i;
		if (!e2->inuse)
			continue;

		Com_sprintf (st, sizeof(st), "%3d %-16.16s %02d:%02d %4d %3d%s%s\n",
			i,
			e2->client->pers.netname,
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			(ctfgame.match == MATCH_SETUP || ctfgame.match == MATCH_PREGAME) ?
			(e2->client->resp.ready ? " (ready)" : " (notready)") : "",
			e2->client->resp.admin ? " (admin)" : "");

		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
		//	sprintf(text+strlen(text), "And more...\n");
			Com_sprintf (tmp, sizeof(tmp), "And more...\n");
			Q_strncatz (text, sizeof(text), tmp);
			safe_cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
	//	strncat(text, st);
		Q_strncatz(text, sizeof(text), st);
	}
	safe_cprintf(ent, PRINT_HIGH, "%s", text);
}


void CTFWarp(edict_t *ent)
{
	char text[1024];
	char *mlist, *token;
	static const char *seps = " \t\n\r";

	if (gi.argc() < 2) {
		safe_cprintf(ent, PRINT_HIGH, "Where do you want to warp to?\n");
		safe_cprintf(ent, PRINT_HIGH, "Available levels are: %s\n", warp_list->string);
		return;
	}

	mlist = strdup(warp_list->string);

	token = strtok(mlist, seps);
	while (token != NULL) {
		if (Q_stricmp(token, gi.argv(1)) == 0)
			break;
		token = strtok(NULL, seps);
	}

	if (token == NULL) {
		safe_cprintf(ent, PRINT_HIGH, "Unknown CTF level.\n");
		safe_cprintf(ent, PRINT_HIGH, "Available levels are: %s\n", warp_list->string);
		free(mlist);
		return;
	}

	free(mlist);


	if (ent->client->resp.admin) {
		safe_bprintf(PRINT_HIGH, "%s is warping to level %s.\n", 
			ent->client->pers.netname, gi.argv(1));
	//	strncpy(level.forcemap, gi.argv(1), sizeof(level.forcemap) - 1);
		Q_strncpyz(level.forcemap, sizeof(level.forcemap), gi.argv(1));
		EndDMLevel();
		return;
	}

	Com_sprintf (text, sizeof(text), "%s has requested warping to level %s.", 
			ent->client->pers.netname, gi.argv(1));
	if (CTFBeginElection(ent, ELECT_MAP, text))
	//	strncpy(ctfgame.elevel, gi.argv(1), sizeof(ctfgame.elevel) - 1);
		Q_strncpyz(ctfgame.elevel, sizeof(ctfgame.elevel), gi.argv(1));
}

void CTFBoot(edict_t *ent)
{
	int i;
	edict_t *targ;
	char text[80];

	if (!ent->client->resp.admin) {
		safe_cprintf(ent, PRINT_HIGH, "You are not an admin.\n");
		return;
	}

	if (gi.argc() < 2) {
		safe_cprintf(ent, PRINT_HIGH, "Who do you want to kick?\n");
		return;
	}

	if (*gi.argv(1) < '0' && *gi.argv(1) > '9') {
		safe_cprintf(ent, PRINT_HIGH, "Specify the player number to kick.\n");
		return;
	}

	i = atoi(gi.argv(1));
	if (i < 1 || i > maxclients->value) {
		safe_cprintf(ent, PRINT_HIGH, "Invalid player number.\n");
		return;
	}

	targ = g_edicts + i;
	if (!targ->inuse) {
		safe_cprintf(ent, PRINT_HIGH, "That player number is not connected.\n");
		return;
	}

	Com_sprintf (text, sizeof(text), "kick %d\n", i - 1);
	gi.AddCommandString(text);
}


void CTFSetPowerUpEffect(edict_t *ent, int def)
{
	if (ent->client->resp.ctf_team == CTF_TEAM1)
		ent->s.effects |= EF_PENT; // red
	else if (ent->client->resp.ctf_team == CTF_TEAM2)
		ent->s.effects |= EF_QUAD; // red
	else
		ent->s.effects |= def;
}

