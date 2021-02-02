// g_team.c

#include "g_local.h"
#include "m_player.h"

cvar_t *ctf_forcejoin;
cvar_t *matchlock;
cvar_t *matchtime;
cvar_t *matchsetuptime;
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
int imageindex_i_aslta;																				//CW++

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
  "xv 246 "
  "num 3 10 "																						//CW
  "xv 296 "
  "pic 9 "
"endif "

// help / weapon icon 
"if 11 "
  "xv 150 "																							//CW
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
;

static char *tnames[] = {
	"item_tech1", "item_tech2", "item_tech3", "item_tech4",
	NULL
};

void StuffCmd(edict_t *ent, char *s) 	
{
//Maj++
	// don't unicast() to bots!
	if (ent->isabot)
		return;
//Maj--

   	gi.WriteByte(11);	        
	gi.WriteString(s);
    gi.unicast(ent, true);	
}

//CW++
void StuffCmd_Ent(edict_t *self)
{
	if (!self->target_ent)
	{
		gi.dprintf("BUG: StuffCmd_Ent() called for a null target_ent.\nPlease contact musashi@planetquake.com\n");
		G_FreeEdict(self);
		return;
	}

	if (!self->target_ent->client)
	{
		gi.dprintf("BUG: StuffCmd_Ent() called for a null client.\nPlease contact musashi@planetquake.com\n");
		G_FreeEdict(self);
		return;
	}

	if (!self->target_ent->inuse)
	{
		G_FreeEdict(self);
		return;
	}

	if (!self->message)
	{
		G_FreeEdict(self);
		return;
	}

	StuffCmd(self->target_ent, self->message);
	G_FreeEdict(self);
}
//CW--

/*--------------------------------------------------------------------------*/

/*
=================
findradius

Returns entities whose bounding boxes are within 
a spherical volume centred on the specified point.
=================
*/
static edict_t *loc_findradius(edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		i;

	if (!from)
		from = g_edicts;
	else
		from++;

	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;

		for (i = 0; i < 3; ++i)
			eorg[i] = org[i] - (from->s.origin[i] + (from->mins[i] + from->maxs[i])*0.5);

		if (VectorLength(eorg) > rad)
			continue;

		return from;
	}

	return NULL;
}

static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
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

static qboolean loc_CanSee(edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	vec3_t	viewpoint;
	int		i;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
		return false; // bmodels not supported

	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;

	for (i = 0; i < 8; i++)
	{
		trace = gi.trace(viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
	}

	return false;
}

/*--------------------------------------------------------------------------*/

static gitem_t *flag1_item;
static gitem_t *flag2_item;

void CTFSpawn(void)
{
	if (!flag1_item)
		flag1_item = FindItemByClassname("item_flag_team1");
	if (!flag2_item)
		flag2_item = FindItemByClassname("item_flag_team2");

	memset(&teamgame, 0, sizeof(teamgame));

	CTFSetupTechSpawn();

	if ((int)competition->value > 1)
	{
		teamgame.match = MATCH_SETUP;
		teamgame.matchtime = level.time + (matchsetuptime->value * 60.0);
	}
}

void CTFInit(void)
{
	ctf_forcejoin = gi.cvar("ctf_forcejoin", "", 0);
	competition = gi.cvar("competition", "0", 0);													//CW
	matchlock = gi.cvar("matchlock", "1", 0);														//CW
	matchtime = gi.cvar("matchtime", "20", 0);														//CW
	matchsetuptime = gi.cvar("matchsetuptime", "10", 0);
	matchstarttime = gi.cvar("matchstarttime", "20", 0);
	warn_unbalanced = gi.cvar("warn_unbalanced", "1", 0);
}

/*
 * Precache CTF items
 */
void CTFPrecache(void)
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

/*--------------------------------------------------------------------------*/

char *CTFTeamName(int team)
{
	switch (team)
	{
		case CTF_TEAM1:
			return sv_team1_name->string;															//CW
		
		case CTF_TEAM2:
			return sv_team2_name->string;															//CW
	}

	return "UNKNOWN";
}

char *CTFOtherTeamName(int team)
{
	switch (team)
	{
		case CTF_TEAM1:
			return sv_team2_name->string;															//CW
		
		case CTF_TEAM2:
			return sv_team1_name->string;															//CW
	}

	return "UNKNOWN";
}

int CTFOtherTeam(int team)
{
	switch (team)
	{
		case CTF_TEAM1:
			return CTF_TEAM2;
		
		case CTF_TEAM2:
			return CTF_TEAM1;
	}

	return -1; // invalid value
}

/*--------------------------------------------------------------------------*/

edict_t *SelectRandomDeathmatchSpawnPoint(void);
edict_t *SelectFarthestDeathmatchSpawnPoint(void);
float PlayersRangeFromSpot(edict_t *spot);
float TeamPlayersRangeFromSpot(edict_t *spot, ctfteam_t team);										//CW++


void CTFAssignSkin(edict_t *ent, char *s)
{	//r1,CW modified heavily throughout to prevent potential configstring overflows

	int		playernum = ent - g_edicts - 1;
	char	t[MAX_SKINLEN];

//	Assign team models if the appropriate cvars are set, otherwise use "male" as the default.

	if (strlen(sv_team1_model->string) && (ent->client->resp.ctf_team == CTF_TEAM1))
	{
		strncpy(t, sv_team1_model->string, sizeof(t)-2);
		Com_strcat(t, sizeof(t), "/");
	}
	else if (strlen(sv_team2_model->string) && (ent->client->resp.ctf_team == CTF_TEAM2))
	{
		strncpy(t, sv_team2_model->string, sizeof(t)-2);
		Com_strcat(t, sizeof(t), "/");
	}
	else
		Com_strcpy(t, sizeof(t), "male/");

//	Assign team skins if the appropriate cvars are set, otherwise use "ctf_r" and "ctf_b" as the defaults.

	switch (ent->client->resp.ctf_team)
	{
		case CTF_TEAM1:
			if (strlen(sv_team1_skin->string))
			{
				if (strlen(ent->client->pers.netname) + strlen(t) + strlen(sv_team1_skin->string) + 4 > MAX_SKINLEN - 1)
				{
					gi.dprintf("Team 1 model/skin name is too long.\n");
					gi_cprintf(ent, PRINT_HIGH, "Team 1 model/skin name is too long.\n");
					gi.configstring(CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, "male/grunt"));
				}
				else
					gi.configstring(CS_PLAYERSKINS+playernum, va("%s\\%s%s", ent->client->pers.netname, t, sv_team1_skin->string));
			}
			else
			{
				if (strlen(ent->client->pers.netname) + strlen(t) + strlen(CTF_TEAM1_SKIN) + 4 > MAX_SKINLEN - 1)
				{
					gi.dprintf("Team 1 model/skin name is too long.\n");
					gi_cprintf(ent, PRINT_HIGH, "Team 1 model/skin name is too long.\n");
					gi.configstring(CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, "male/ctf_r"));
				}
				else
					gi.configstring(CS_PLAYERSKINS+playernum, va("%s\\%s%s", ent->client->pers.netname, t, CTF_TEAM1_SKIN));
			}
			break;

		case CTF_TEAM2:
			if (strlen(sv_team2_skin->string))
			{
				if (strlen(ent->client->pers.netname) + strlen(t) + strlen(sv_team2_skin->string) + 4 > MAX_SKINLEN - 1)
				{
					gi.dprintf("Team 2 model/skin name is too long.\n");
					gi_cprintf(ent, PRINT_HIGH, "Team 2 model/skin name is too long.\n");
					gi.configstring(CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, "male/ctf_b"));
				}
				else
					gi.configstring(CS_PLAYERSKINS+playernum, va("%s\\%s%s", ent->client->pers.netname, t, sv_team2_skin->string));
			}
			else
			{
				if (strlen(ent->client->pers.netname) + strlen(t) + strlen(CTF_TEAM2_SKIN) + 4 > MAX_SKINLEN - 1)
				{
					gi.dprintf("Team 2 model/skin name is too long.\n");
					gi_cprintf(ent, PRINT_HIGH, "Team 2 model/skin name is too long.\n");
					gi.configstring(CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, "male/ctf_b"));
				}
				else
					gi.configstring(CS_PLAYERSKINS+playernum, va("%s\\%s%s", ent->client->pers.netname, t, CTF_TEAM2_SKIN));
			}
			break;

		default:
			if (strlen(ent->client->pers.netname) + strlen(s) + 4 > MAX_SKINLEN - 1)
			{
				gi_cprintf(ent, PRINT_HIGH, "Skin name is too long.\n");
				s = "male/grunt";
			}

			gi.configstring(CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s));
			break;
	}
//CW--
}

void CTFAssignTeam(gclient_t *who)
{
	edict_t	*player;
	int		team1count = 0;
	int		team2count = 0;
	int		i;

//CW++
	edict_t	 *ent;
	qboolean isabot = false;

	for (i = 1; i <= (int)maxclients->value; i++)
	{
		ent = &g_edicts[i];
		if (ent->client == who)
		{
			if (ent->isabot)
				isabot = true;

			break;
		}
	}
//CW--

	who->resp.ctf_state = 0;

	if (!((int)dmflags->value & DF_FORCEJOIN) && !isabot)
	{
		who->resp.ctf_team = CTF_NOTEAM;
		return;
	}

//CW++
	if (sv_gametype->value == G_FFA)
	{
		who->resp.ctf_team = CTF_TEAM_FFA;
		return;
	}
//CW--

	for (i = 1; i <= (int)maxclients->value; i++)
	{
		player = &g_edicts[i];

		if (!player->inuse || (player->client == who))
			continue;

		switch (player->client->resp.ctf_team)
		{
			case CTF_TEAM1:
				team1count++;
				break;

			case CTF_TEAM2:
				team2count++;
		}
	}

	if (team1count < team2count)
		who->resp.ctf_team = CTF_TEAM1;
	else if (team2count < team1count)
		who->resp.ctf_team = CTF_TEAM2;
	else if (rand() & 1)
		who->resp.ctf_team = CTF_TEAM1;
	else
		who->resp.ctf_team = CTF_TEAM2;
}

/*
================
SelectCTFSpawnPoint

go to a ctf point, but NOT the two points closest
to other players
================
*/
edict_t *SelectCTFSpawnPoint(edict_t *ent, qboolean ctf_only)										//CW
{
	edict_t	*spot;
	edict_t	*spot1;
	edict_t	*spot2;
	char	*cname;
	float	range;
	float	range1;
	float	range2;
	int		count = 0;
	int		selection;

	if (ent->client->resp.ctf_state && !((int)(dmflags->value) & DF_CTF_SPAWNS_ONLY) && !ctf_only)	//CW
	{
		if ((int)(dmflags->value) & DF_SPAWN_FARTHEST)
			return SelectFarthestDeathmatchSpawnPoint();
		else
			return SelectRandomDeathmatchSpawnPoint();
	}

//CW++
	if (!ent->client->resp.ctf_state)		// due to addition of DF_CTF_SPAWNS_ONLY check above
//CW--
		ent->client->resp.ctf_state++;

	switch (ent->client->resp.ctf_team)
	{
		case CTF_TEAM1:
			cname = "info_player_team1";
			break;

		case CTF_TEAM2:
			cname = "info_player_team2";
			break;

		default:
			return SelectRandomDeathmatchSpawnPoint();
	}

	spot = NULL;
	range1 = range2 = 99999.0;
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
		spot = G_Find(spot, FOFS(classname), cname);
		if ((spot == spot1) || (spot == spot2))
			selection++;
	} while (selection--);

	return spot;
}

/*------------------------------------------------------------------------*/
/*
CTFFragBonuses

Calculate the bonuses for flag defense, flag carrier defense, etc.
Note that bonuses are not cumulative.  You get one, they are in importance
order.
*/
void CTFFragBonuses(edict_t *targ, edict_t *inflictor, edict_t *attacker)
{
	edict_t	*ent;
	edict_t	*flag;
	edict_t	*carrier = NULL;																		//CW
	gitem_t	*flag_item;
	gitem_t	*enemy_flag_item;
	vec3_t	v1;
	vec3_t	v2;
	char	*c;
	int		i;
	int		otherteam;

	if (targ->client && attacker->client)
	{
		if (attacker->client->resp.ghost)
		{
			if (attacker != targ)
				attacker->client->resp.ghost->kills++;
		}

		if (targ->client->resp.ghost)
			targ->client->resp.ghost->deaths++;
	}

	// no bonus for fragging yourself
	if (!targ->client || !attacker->client || (targ == attacker))
		return;

	otherteam = CTFOtherTeam(targ->client->resp.ctf_team);
	if (otherteam < 0)
		return; // whoever died isn't on a team

	// same team, if the flag at base, check to he has the enemy flag
	if (targ->client->resp.ctf_team == CTF_TEAM1)
	{
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	}
	else
	{
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}

	// did the attacker frag the flag carrier?
	if (targ->client->pers.inventory[ITEM_INDEX(enemy_flag_item)])
	{
		attacker->client->resp.ctf_lastfraggedcarrier = level.time;
		attacker->client->resp.score += CTF_FRAG_CARRIER_BONUS;
		gi_cprintf(attacker, PRINT_MEDIUM, "BONUS: %d points for fragging enemy flag carrier.\n", CTF_FRAG_CARRIER_BONUS);

		// the target had the flag, clear the hurt carrier
		// field on the other team
		for (i = 1; i <= (int)maxclients->value; i++)
		{
			ent = g_edicts + i;
			if (ent->inuse && ent->client->resp.ctf_team == otherteam)
				ent->client->resp.ctf_lasthurtcarrier = 0;
		}
		return;
	}

	if (targ->client->resp.ctf_lasthurtcarrier &&
		(level.time - targ->client->resp.ctf_lasthurtcarrier < CTF_CARRIER_DANGER_PROTECT_TIMEOUT) &&
		!attacker->client->pers.inventory[ITEM_INDEX(flag_item)])
	{
		// attacker is on the same team as the flag carrier and
		// fragged a guy who hurt our flag carrier
		attacker->client->resp.score += CTF_CARRIER_DANGER_PROTECT_BONUS;
		gi_bprintf(PRINT_MEDIUM, "%s defends %s's flag carrier against an agressive enemy\n",
			attacker->client->pers.netname, 
			CTFTeamName(attacker->client->resp.ctf_team));
		if (attacker->client->resp.ghost)
			attacker->client->resp.ghost->carrierdef++;
		return;
	}

	// flag and flag carrier area defense bonuses
	// we have to find the flag and carrier entities

	// find the flag
	switch (attacker->client->resp.ctf_team)
	{
		case CTF_TEAM1:
			c = "item_flag_team1";
			break;

		case CTF_TEAM2:
			c = "item_flag_team2";
			break;

		default:
			return;
	}

	flag = NULL;
	while ((flag = G_Find(flag, FOFS(classname), c)) != NULL)
	{
		if (!(flag->spawnflags & DROPPED_ITEM))
			break;
	}

	if (!flag)
		return; // can't find attacker's flag

	// find attacker's team's flag carrier
	for (i = 1; i <= (int)maxclients->value; i++)
	{
		carrier = g_edicts + i;
		if (carrier->inuse && carrier->client->pers.inventory[ITEM_INDEX(flag_item)])
			break;

		carrier = NULL;
	}

	// ok we have the attackers flag and a pointer to the carrier

	// check to see if we are defending the base's flag
	VectorSubtract(targ->s.origin, flag->s.origin, v1);
	VectorSubtract(attacker->s.origin, flag->s.origin, v2);

	if (((VectorLength(v1) < CTF_TARGET_PROTECT_RADIUS) || (VectorLength(v2) < CTF_TARGET_PROTECT_RADIUS) ||
		loc_CanSee(flag, targ) || loc_CanSee(flag, attacker)) && (attacker->client->resp.ctf_team != targ->client->resp.ctf_team))
	{
		// we defended the base flag
		attacker->client->resp.score += CTF_FLAG_DEFENSE_BONUS;
		if (flag->solid == SOLID_NOT)
			gi_bprintf(PRINT_MEDIUM, "%s defends the %s base.\n", attacker->client->pers.netname, CTFTeamName(attacker->client->resp.ctf_team));
		else
			gi_bprintf(PRINT_MEDIUM, "%s defends the %s flag.\n", attacker->client->pers.netname, CTFTeamName(attacker->client->resp.ctf_team));
		if (attacker->client->resp.ghost)
			attacker->client->resp.ghost->basedef++;
		return;
	}

	if (carrier && (carrier != attacker))
	{
		VectorSubtract(targ->s.origin, carrier->s.origin, v1);
		VectorSubtract(attacker->s.origin, carrier->s.origin, v1);

		if ((VectorLength(v1) < CTF_ATTACKER_PROTECT_RADIUS) || (VectorLength(v2) < CTF_ATTACKER_PROTECT_RADIUS) ||
			loc_CanSee(carrier, targ) || loc_CanSee(carrier, attacker))
		{
			attacker->client->resp.score += CTF_CARRIER_PROTECT_BONUS;
			gi_bprintf(PRINT_MEDIUM, "%s defends the %s's flag carrier.\n", attacker->client->pers.netname, CTFTeamName(attacker->client->resp.ctf_team));
			if (attacker->client->resp.ghost)
				attacker->client->resp.ghost->carrierdef++;
			return;
		}
	}
}

void CTFCheckHurtCarrier(edict_t *targ, edict_t *attacker)
{
	gitem_t *flag_item;

	if (!targ->client || !attacker->client)
		return;

	if (targ->client->resp.ctf_team == CTF_TEAM1)
		flag_item = flag2_item;
	else
		flag_item = flag1_item;

	if (targ->client->pers.inventory[ITEM_INDEX(flag_item)] && (targ->client->resp.ctf_team != attacker->client->resp.ctf_team))
		attacker->client->resp.ctf_lasthurtcarrier = level.time;
}


/*------------------------------------------------------------------------*/

void CTFResetFlag(int ctf_team)
{
	char *c;
	edict_t *ent;

	switch (ctf_team)
	{
		case CTF_TEAM1:
			c = "item_flag_team1";
			break;

		case CTF_TEAM2:
			c = "item_flag_team2";
			break;

		default:
			return;
	}

	ent = NULL;
	while ((ent = G_Find (ent, FOFS(classname), c)) != NULL)
	{
		if (ent->spawnflags & DROPPED_ITEM)
			G_FreeEdict(ent);
		else
		{
			ent->svflags &= ~SVF_NOCLIENT;
			ent->solid = SOLID_TRIGGER;
			gi.linkentity(ent);
			ent->s.event = EV_ITEM_RESPAWN;
		}
	}
}

void CTFResetFlags(void)
{
	CTFResetFlag(CTF_TEAM1);
	CTFResetFlag(CTF_TEAM2);
}

qboolean CTFPickup_Flag(edict_t *ent, edict_t *other)
{
	edict_t	*player;
	gitem_t	*flag_item;
	gitem_t	*enemy_flag_item;
	int		ctf_team;
	int		i;

//	Figure out what team this flag is.

	if (strcmp(ent->classname, "item_flag_team1") == 0)
		ctf_team = CTF_TEAM1;
	else if (strcmp(ent->classname, "item_flag_team2") == 0)
		ctf_team = CTF_TEAM2;
	else
	{
		gi_cprintf(ent, PRINT_HIGH, "Don't know what team the flag is on.\n");
		return false;
	}

//	Same team; if the flag at base, check to see if he has the enemy flag.

	if (ctf_team == CTF_TEAM1)
	{
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	}
	else
	{
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}

	if (ctf_team == other->client->resp.ctf_team)
	{

		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			
//			The flag is at home base. If the player has the enemy flag, he's just won!
		
			if (other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)])
			{
				gi_bprintf(PRINT_HIGH, "%s captured the %s flag!\n", other->client->pers.netname, CTFOtherTeamName(ctf_team));
				other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)] = 0;

				teamgame.last_flag_capture = level.time;
				teamgame.last_capture_team = ctf_team;
				if (ctf_team == CTF_TEAM1)
					teamgame.team1++;
				else
					teamgame.team2++;

				gi.sound(ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagcap.wav"), 1, ATTN_NONE, 0);

//				Other gets another 10 frag bonus.

				other->client->resp.score += CTF_CAPTURE_BONUS;
				if (other->client->resp.ghost)
					other->client->resp.ghost->caps++;

//				Ok, let's do the player loop, hand out the bonuses.

				for (i = 1; i <= (int)maxclients->value; i++)
				{
					player = &g_edicts[i];
					if (!player->inuse)
						continue;

					if (player->client->resp.ctf_team != other->client->resp.ctf_team)
						player->client->resp.ctf_lasthurtcarrier = -5;
					else if (player->client->resp.ctf_team == other->client->resp.ctf_team)
					{
						if (player != other)
							player->client->resp.score += CTF_TEAM_BONUS;

//						Award extra points for capture assists.

						if (player->client->resp.ctf_lastreturnedflag + CTF_RETURN_FLAG_ASSIST_TIMEOUT > level.time)
						{
							gi_bprintf(PRINT_HIGH, "%s gets an assist for returning the flag!\n", player->client->pers.netname);
							player->client->resp.score += CTF_RETURN_FLAG_ASSIST_BONUS;
						}
						if (player->client->resp.ctf_lastfraggedcarrier + CTF_FRAG_CARRIER_ASSIST_TIMEOUT > level.time)
						{
							gi_bprintf(PRINT_HIGH, "%s gets an assist for fragging the flag carrier!\n", player->client->pers.netname);
							player->client->resp.score += CTF_FRAG_CARRIER_ASSIST_BONUS;
						}
					}
				}

				CTFResetFlags();
				return false;
			}
			return false; // its at home base already
		}	

//		Hey, it's not home. Return it by teleporting it back.

		gi_bprintf(PRINT_HIGH, "%s returned the %s flag!\n", other->client->pers.netname, CTFTeamName(ctf_team));
		other->client->resp.score += CTF_RECOVERY_BONUS;
		other->client->resp.ctf_lastreturnedflag = level.time;
		gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagret.wav"), 1, ATTN_NONE, 0);
		
		//CTFResetFlag will remove this entity!  We must return false.
		CTFResetFlag(ctf_team);
		return false;
	}

//	Hey, its not our flag, so pick it up.

	gi_bprintf(PRINT_HIGH, "%s got the %s flag!\n", other->client->pers.netname, CTFTeamName(ctf_team));
	other->client->resp.score += CTF_FLAG_BONUS;
	other->client->pers.inventory[ITEM_INDEX(flag_item)] = 1;
	other->client->resp.ctf_flagsince = level.time;

//CW++
//	Player loses respawn invulnerability upon touching the enemy flag.

	if (level.time - other->client->respawn_time < sv_respawn_invuln_time->value)
	{
		if (other->client->invincible_framenum - level.framenum < 10.0 * sv_respawn_invuln_time->value)
			other->client->invincible_framenum = 0;
	}
//CW--

//	If it's not a dropped flag, we just make is disappear.
//	If it's dropped, it will be removed by the pickup caller.

	if (!(ent->spawnflags & DROPPED_ITEM))
	{
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	return true;
}

static void CTFDropFlagTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{

//	Owner (who dropped us) can't touch us for two secs.

	if ((other == ent->owner) && (ent->nextthink - level.time > CTF_AUTO_FLAG_RETURN_TIMEOUT - 2))
		return;

	Touch_Item(ent, other, plane, surf);
}

static void CTFDropFlagThink(edict_t *ent)
{

//	Auto return the flag. Reset flag will remove ourselves.

	if (strcmp(ent->classname, "item_flag_team1") == 0)
	{
		CTFResetFlag(CTF_TEAM1);
		gi_bprintf(PRINT_HIGH, "The %s flag has returned!\n", CTFTeamName(CTF_TEAM1));
	}
	else if (strcmp(ent->classname, "item_flag_team2") == 0)
	{
		CTFResetFlag(CTF_TEAM2);
		gi_bprintf(PRINT_HIGH, "The %s flag has returned!\n",CTFTeamName(CTF_TEAM2));
	}
}

// Called from PlayerDie, to drop the flag from a dying player
void CTFDeadDropFlag(edict_t *self)
{
	edict_t *dropped = NULL;

	if (self->client->pers.inventory[ITEM_INDEX(flag1_item)])
	{
		dropped = Drop_Item(self, flag1_item);
		self->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
		gi_bprintf(PRINT_HIGH, "%s lost the %s flag!\n", self->client->pers.netname, CTFTeamName(CTF_TEAM1));
	}
	else if (self->client->pers.inventory[ITEM_INDEX(flag2_item)])
	{
		dropped = Drop_Item(self, flag2_item);
		self->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
		gi_bprintf(PRINT_HIGH, "%s lost the %s flag!\n", self->client->pers.netname, CTFTeamName(CTF_TEAM2));
	}

	if (dropped)
	{
		dropped->think = CTFDropFlagThink;
		dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
		dropped->touch = CTFDropFlagTouch;
	}
}

void CTFDrop_Flag(edict_t *ent, gitem_t *item)														//CW
{
	if (rand() & 1) 
		gi_cprintf(ent, PRINT_HIGH, "Only lusers drop flags.\n");
	else
		gi_cprintf(ent, PRINT_HIGH, "Winners don't drop flags.\n");
	return;																							//CW
}

static void CTFFlagThink(edict_t *ent)
{
	if (ent->solid != SOLID_NOT)
		ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);

	ent->nextthink = level.time + FRAMETIME;
}


void CTFFlagSetup(edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15.0, -15.0, -15.0);
	VectorCopy(v, ent->mins);
	v = tv(15.0, 15.0, 15.0);
	VectorCopy(v, ent->maxs);

	if (ent->model)
		gi.setmodel(ent, ent->model);
	else
		gi.setmodel(ent, ent->item->world_model);

	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	v = tv(0.0, 0.0, -128.0);
	VectorAdd(ent->s.origin, v, dest);

	tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf("CTFFlagSetup: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict(ent);
		return;
	}

	VectorCopy(tr.endpos, ent->s.origin);
	gi.linkentity(ent);

	ent->think = CTFFlagThink;
	ent->nextthink = level.time + FRAMETIME;
}

void CTFEffects(edict_t *player)
{
	player->s.effects &= ~(EF_FLAG1 | EF_FLAG2);
	if (player->health > 0)
	{
		if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])
			player->s.effects |= EF_FLAG1;
		if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])
			player->s.effects |= EF_FLAG2;
	}

	if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])
		player->s.modelindex3 = gi.modelindex("players/male/flag1.md2");
	else if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])
		player->s.modelindex3 = gi.modelindex("players/male/flag2.md2");
	else
		player->s.modelindex3 = 0;
}

// called when we enter the intermission
void CTFCalcScores(void)
{
	int i;

	teamgame.total1 = teamgame.total2 = 0;
	for (i = 0; i < (int)maxclients->value; i++)
	{
		if (!g_edicts[i+1].inuse)
			continue;

		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			teamgame.total1 += game.clients[i].resp.score;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			teamgame.total2 += game.clients[i].resp.score;
	}
}

void CTFID_f(edict_t *ent)
{
	if (ent->client->resp.id_state)
	{
		gi_cprintf(ent, PRINT_HIGH, "Disabling player identification display.\n");
		ent->client->resp.id_state = false;
	}
	else
	{
		gi_cprintf(ent, PRINT_HIGH, "Activating player identification display.\n");
		ent->client->resp.id_state = true;
	}
}

static void CTFSetIDView(edict_t *ent)
{
	edict_t	*who;
	edict_t	*best;
	vec3_t	forward;
	vec3_t	dir;
	trace_t	tr;
	float	bd = 0;
	float	d;
	int		i;

//	Only check every few frames.

	if (level.time - ent->client->resp.lastidtime < CLIENT_ID_CHECKTIME)							//CW
		return;

	ent->client->resp.lastidtime = level.time;

	ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;
	ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = 0;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 1024.0, forward);
	VectorAdd(ent->s.origin, forward, forward);
	tr = gi.trace(ent->s.origin, NULL, NULL, forward, ent, MASK_SOLID);
	if ((tr.fraction < 1.0) && tr.ent && tr.ent->client)
	{
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = CS_GENERAL + (tr.ent - g_edicts - 1);
		if (tr.ent->client->resp.ctf_team == CTF_TEAM1)
			ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = imageindex_sbfctf1;
		else if (tr.ent->client->resp.ctf_team == CTF_TEAM2)
			ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = imageindex_sbfctf2;
		return;
	}

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	best = NULL;
	for (i = 1; i <= (int)maxclients->value; i++)
	{
		who = g_edicts + i;
		if (!who->inuse || (who->solid == SOLID_NOT))
			continue;

		VectorSubtract(who->s.origin, ent->s.origin, dir);
		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if ((d > bd) && loc_CanSee(ent, who))
		{
			bd = d;
			best = who;
		}
	}
	if (bd > 0.90)
	{
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = CS_GENERAL + (best - g_edicts - 1);
		if (best->client->resp.ctf_team == CTF_TEAM1)
			ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = imageindex_sbfctf1;
		else if (best->client->resp.ctf_team == CTF_TEAM2)
			ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = imageindex_sbfctf2;
	}
}

void SetCTFStats(edict_t *ent)
{
	gitem_t *tech;
	edict_t *e;
	int		i;
	int		p1;
	int		p2;

	ent->client->ps.stats[STAT_CTF_MATCH] = CONFIG_CTF_MATCH;

	if (teamgame.warnactive)
		ent->client->ps.stats[STAT_CTF_TEAMINFO] = CONFIG_CTF_TEAMINFO;
	else
		ent->client->ps.stats[STAT_CTF_TEAMINFO] = 0;

	// ghosting
	if (ent->client->resp.ghost)
	{
		ent->client->resp.ghost->score = ent->client->resp.score;
	//	strcpy(ent->client->resp.ghost->netname, ent->client->pers.netname);
		Com_strcpy(ent->client->resp.ghost->netname, sizeof(ent->client->resp.ghost->netname), ent->client->pers.netname);
		ent->client->resp.ghost->number = ent->s.number;
	}

	// logo headers for the frag display
	ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = imageindex_ctfsb1;
	ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = imageindex_ctfsb2;

	// if during intermission, we must blink the team header of the winning team
	if (level.intermissiontime && (level.framenum & 8))	// blink 1/8th second
	{
//CW++
//		Victory for Assault games based is solely on whether or not the attacking team achieved their objective(s).

		if (sv_gametype->value == G_ASLT)
		{
			if (asltgame.victory)		// attackers were successful
			{
				if (asltgame.t_attack == CTF_TEAM1)
					ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
				else
					ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			}
			else						// timelimit reached before attackers completed their mission
			{
				if (asltgame.t_attack == CTF_TEAM1)
					ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
				else
					ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			}
		}
		else
		{
//CW--
			// note that teamgame.total[12] is set when we go to intermission
			if (teamgame.team1 > teamgame.team2)
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			else if (teamgame.team2 > teamgame.team1)
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			else if (teamgame.total1 > teamgame.total2)				// frag tie breaker
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			else if (teamgame.total2 > teamgame.total1) 
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			else													// tie game!
			{
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			}
		}
	}

	// tech icon
	i = 0;
	ent->client->ps.stats[STAT_CTF_TECH] = 0;
	while (tnames[i])
	{
		if ((tech = FindItemByClassname(tnames[i])) != NULL && ent->client->pers.inventory[ITEM_INDEX(tech)])
		{
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
	if (e != NULL)
	{
		if (e->solid == SOLID_NOT)
		{
			int i;

			// not at base; check if on player
			p1 = imageindex_i_ctf1d;						// default to dropped
			for (i = 1; i <= (int)maxclients->value; i++)
				if (g_edicts[i].inuse && g_edicts[i].client->pers.inventory[ITEM_INDEX(flag1_item)])
				{
					// enemy has it
					p1 = imageindex_i_ctf1t;
					break;
				}
		}
		else if (e->spawnflags & DROPPED_ITEM)
			p1 = imageindex_i_ctf1d;						// must be dropped
	}
	p2 = imageindex_i_ctf2;
	e = G_Find(NULL, FOFS(classname), "item_flag_team2");
	if (e != NULL)
	{
		if (e->solid == SOLID_NOT)
		{
			int i;

			// not at base; check if on player
			p2 = imageindex_i_ctf2d;						// default to dropped
			for (i = 1; i <= (int)maxclients->value; i++)
			{
				if (g_edicts[i].inuse && g_edicts[i].client->pers.inventory[ITEM_INDEX(flag2_item)])
				{
					// enemy has it
					p2 = imageindex_i_ctf2t;
					break;
				}
			}
		}
		else if (e->spawnflags & DROPPED_ITEM)
			p2 = imageindex_i_ctf2d;						// must be dropped
	}

	ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
	ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;

	if (teamgame.last_flag_capture && (level.time - teamgame.last_flag_capture < 5.0))
	{
		if (teamgame.last_capture_team == CTF_TEAM1)
		{
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
			else
				ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = 0;
		}
		else
		{
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;
			else
				ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = 0;
		}
	}

//CW++
	if (sv_gametype->value == G_CTF)
	{
//CW---
		ent->client->ps.stats[STAT_CTF_TEAM1_CAPS] = teamgame.team1;
		ent->client->ps.stats[STAT_CTF_TEAM2_CAPS] = teamgame.team2;
//CW++
	}
	else if ((sv_gametype->value == G_TDM) || (sv_gametype->value == G_ASLT))	//display frags instead of caps
	{
		ent->client->ps.stats[STAT_CTF_TEAM1_CAPS] = teamgame.frags1;
		ent->client->ps.stats[STAT_CTF_TEAM2_CAPS] = teamgame.frags2;
	}
//CW--

	ent->client->ps.stats[STAT_CTF_FLAG_PIC] = 0;

//CW++
	if (sv_gametype->value == G_CTF)
	{
//CW--
		if ((ent->client->resp.ctf_team == CTF_TEAM1) && ent->client->pers.inventory[ITEM_INDEX(flag2_item)] && (level.framenum & 8))
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = imageindex_i_ctf2;
		else if ((ent->client->resp.ctf_team == CTF_TEAM2) && ent->client->pers.inventory[ITEM_INDEX(flag1_item)] && (level.framenum & 8))
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = imageindex_i_ctf1;
	}

//CW++
//	Indicate the attacking team.

	ent->client->ps.stats[STAT_ASLT_ATTACK2] = 0;
	if (sv_gametype->value == G_ASLT)
	{
		if (asltgame.t_attack == CTF_TEAM1)
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = imageindex_i_aslta;
		else
			ent->client->ps.stats[STAT_ASLT_ATTACK2] = imageindex_i_aslta;
	}
//CW--

	ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = 0;
	ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = 0;
	if (ent->client->resp.ctf_team == CTF_TEAM1)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = imageindex_i_ctfj;
	else if (ent->client->resp.ctf_team == CTF_TEAM2)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = imageindex_i_ctfj;

//CW++
	if ((int)sv_gametype->value > G_FFA)
	{
//CW--
		if (ent->client->resp.id_state)
			CTFSetIDView(ent);
		else
		{
			ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;
			ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = 0;
		}
	}
}

/*------------------------------------------------------------------------*/

/*QUAKED info_player_team1 (1 0 0) (-16 -16 -24) (16 16 32)
potential team1 spawning position for ctf games
*/
void SP_info_player_team1(edict_t *self)
{
//CW++
	if (sv_gametype->value != G_CTF)
	{
		G_FreeEdict(self);
		return;
	}

	self->svflags |= SVF_NOCLIENT;
//CW--
}

/*QUAKED info_player_team2 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team2(edict_t *self)
{
//CW++
	if (sv_gametype->value != G_CTF)
	{
		G_FreeEdict(self);
		return;
	}

	self->svflags |= SVF_NOCLIENT;
//CW--
}


/*
------------------------------------------------------------------------
 GRAPPLE
------------------------------------------------------------------------
*/

// ent is player
void CTFPlayerResetGrapple(edict_t *ent)
{
	if (ent->client && ent->client->ctf_grapple)
		CTFResetGrapple(ent->client->ctf_grapple);
}

// self is grapple, not player
void CTFResetGrapple(edict_t *self)
{
//CW++
//	Sanity checks.

	if (!self->owner)
		return;

	if (!self->owner->client)
		return;
//CW--

	if (self->owner->client->ctf_grapple)
	{
		gclient_t	*cl;
		float		volume = 1.0;

		if (self->owner->client->silencer_shots)
			volume = 0.2;

		gi.sound(self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grreset.wav"), volume, ATTN_NORM, 0);
		cl = self->owner->client;
		cl->ctf_grapple = NULL;
		cl->ctf_grapplereleasetime = level.time;
		cl->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY;	// we're firing, not on hook
		cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

//CW++
		if ((int)sv_hook_offhand->value)
			cl->hookstate = WEAPON_READY;
//CW--

		G_FreeEdict(self);
	}
}

void CTFGrappleTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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

//CW++
	if (other->wep_proj)
		return;

	PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);
//CW--

	VectorCopy(vec3_origin, self->velocity);
	if (other->takedamage)
	{
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_GRAPPLE);
		CTFResetGrapple(self);
		return;
	}

	self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_PULL; // we're on hook
	self->enemy = other;

	self->solid = SOLID_NOT;

	if (self->owner->client->silencer_shots)
		volume = 0.2;

	gi.sound(self->owner, CHAN_WEAPON, gi.soundindex("weapons/grapple/grpull.wav"), volume, ATTN_NORM, 0);	//CW
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhit.wav"), volume, ATTN_NORM, 0);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SPARKS);
	gi.WritePosition(self->s.origin);
	if (!plane)
		gi.WriteDir(vec3_origin);
	else
		gi.WriteDir(plane->normal);
	gi.multicast(self->s.origin, MULTICAST_PVS);

//CW++
	if (plane)
		VectorCopy(plane->normal, self->move_angles);	// used in CTFGrapplePull()

	gi.linkentity(self);
//CW--
}

// draw beam between grapple and self
void CTFGrappleDrawCable(edict_t *self)
{
	vec3_t	offset;
	vec3_t	start;
	vec3_t	end;
	vec3_t	f;
	vec3_t	r;
	vec3_t	dir;
	float	distance;

	AngleVectors(self->owner->client->v_angle, f, r, NULL);

//CW++
	if ((int)sv_hook_offhand->value)
		VectorSet(offset, 0.0, 0.0, self->owner->viewheight-8.0);
	else
//CW--
		VectorSet(offset, 16.0, 16.0, self->owner->viewheight-8.0);

	P_ProjectSource(self->owner->client, self->owner->s.origin, offset, f, r, start);
	VectorSubtract(start, self->owner->s.origin, offset);
	VectorSubtract(start, self->s.origin, dir);

	// don't draw cable if too close
	distance = VectorLength(dir);
	if (distance < 64.0)
		return;

	VectorCopy(self->s.origin, end);																//CW

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_GRAPPLE_CABLE);
	gi.WriteShort(self->owner - g_edicts);
	gi.WritePosition(self->owner->s.origin);
	gi.WritePosition(end);
	gi.WritePosition(offset);
	gi.multicast(self->s.origin, MULTICAST_PVS);
}

void SV_AddGravity(edict_t *ent);

// pull the player toward the grapple
void CTFGrapplePull(edict_t *self)
{
	vec3_t	hookdir;
	vec3_t	v;
	float	vlen;

//CW++
	float	speed;
//CW--

	if ((self->owner->client->pers.weapon->weapmodel == WEAP_GRAPPLE) && !self->owner->client->newweapon &&
		(self->owner->client->weaponstate != WEAPON_FIRING) && (self->owner->client->weaponstate != WEAPON_ACTIVATING))	//CW
	{
		CTFResetGrapple(self);
		return;
	}

//CW++
	if ((int)sv_hook_offhand->value && !self->owner->client->hook_on && (self->owner->client->hookstate != WEAPON_FIRING))
	{
		CTFResetGrapple(self);
		return;
	}
//CW--

	if (self->enemy)
	{
		if (self->enemy->solid == SOLID_NOT)
		{
			CTFResetGrapple(self);
			return;
		}

		if (self->enemy->solid == SOLID_BBOX)
		{
			VectorScale(self->enemy->size, 0.5, v);
			VectorAdd(v, self->enemy->s.origin, v);
			VectorAdd(v, self->enemy->mins, self->s.origin);
			gi.linkentity(self);
		}
		else
			VectorCopy(self->enemy->velocity, self->velocity);

		if (self->enemy->takedamage && !CheckTeamDamage(self->enemy, self->owner))
		{
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			T_Damage(self->enemy, self, self->owner, self->velocity, self->s.origin, vec3_origin, 1, 1, 0, MOD_GRAPPLE);
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhurt.wav"), volume, ATTN_NORM, 0);
		}

		if (self->enemy->deadflag)	 // he died
		{
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
		// that velocity in the direction of the point
		vec3_t forward;
		vec3_t up;

		AngleVectors(self->owner->client->v_angle, forward, NULL, up);
		VectorCopy(self->owner->s.origin, v);
		v[2] += self->owner->viewheight;
		VectorSubtract(self->s.origin, v, hookdir);

		vlen = VectorLength(hookdir);

		if ((self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL) && (vlen < 64.0))
		{
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			self->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			gi.sound(self->owner, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhang.wav"), volume, ATTN_NORM, 0);	//CW
			self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_HANG;
		}

//CW++
		speed = sv_hook_pull_speed->value;
		if (self->owner->client->held_by_agm)
			speed *= sv_hook_pull_agm->value;
//CW--

//CW++
//		When hanging on the grapple next to a surface, make the pulling direction be the 
//		negative of the normal vector of the plane. This prevents unwanted oscillations
//		when the grapple has hit the surface at a shallow angle.

		if (vlen < 16.0)
			VectorScale(self->move_angles, -1.0, hookdir);
		else
//CW--
			VectorNormalize(hookdir);

		VectorScale(hookdir, speed, hookdir);														//CW
		VectorCopy(hookdir, self->owner->velocity);
		SV_AddGravity(self->owner);
	}
}

void CTFFireGrapple(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*grapple;
	trace_t	tr;

	VectorNormalize(dir);

	grapple = G_Spawn();
	grapple->solid = SOLID_BBOX;
	grapple->movetype = MOVETYPE_FLYMISSILE;
	grapple->clipmask = MASK_SHOT;
	grapple->s.effects |= effect;
	grapple->owner = self;
	grapple->touch = CTFGrappleTouch;
	grapple->dmg = damage;
	self->client->ctf_grapple = grapple;
	self->client->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY;		// we're firing, not on hook

	grapple->s.modelindex = gi.modelindex("models/weapons/grapple/hook/tris.md2");
	VectorCopy(start, grapple->s.origin);
	VectorCopy(start, grapple->s.old_origin);
	vectoangles(dir, grapple->s.angles);
	VectorScale(dir, speed, grapple->velocity);

//CW++
	grapple->svflags = SVF_PROJECTILE;
//CW--

	gi.linkentity(grapple);

	tr = gi.trace (self->s.origin, NULL, NULL, grapple->s.origin, grapple, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA(grapple->s.origin, -10.0, dir, grapple->s.origin);
		grapple->touch(grapple, tr.ent, NULL, NULL);
	}
}	

void CTFGrappleFire(edict_t *ent, vec3_t g_offset, int damage, int effect)
{
	vec3_t	forward;
	vec3_t	right;
	vec3_t	start;
	vec3_t	offset;
	float	volume = 1.0;

	if (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
		return; // it's already out

	AngleVectors(ent->client->v_angle, forward, right, NULL);

//CW++
	if ((int)sv_hook_offhand->value)
		VectorSet(offset, 0.0, 0.0, ent->viewheight-6.0);
	else
//CW--
		VectorSet(offset, 24.0, 8.0, ent->viewheight-6.0);											//CW

	VectorAdd(offset, g_offset, offset);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale(forward, -2.0, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1.0;

	if (ent->client->silencer_shots)
		volume = 0.2;

//CW++
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/grapple/grfire.wav"), volume, ATTN_NORM, 0);
	CTFFireGrapple(ent, start, forward, damage, (int)sv_hook_speed->value, effect);
	PlayerNoise(ent, start, PNOISE_WEAPON);
//CW--
}

void CTFWeapon_Grapple_Fire(edict_t *ent)
{
	int	damage;

//CW++
	if (ent->deadflag || ent->s.modelindex != (MAX_MODELS-1)) // was 255
		return;

	damage = (int)sv_hook_damage->value;
	if (ent->client->quad_framenum > level.framenum)
		damage *= 4;
//CW--

	CTFGrappleFire(ent, vec3_origin, damage, 0);													//CW
	ent->client->ps.gunframe++;
}

//CW++
void CTFWeapon_Grapple_OffHand(edict_t *self)
{
	int damage;

//CW++
	if (self->deadflag || (self->s.modelindex != (MAX_MODELS-1))) // was 255
		return;
//CW--

	if (self->health < 1)																			//CW
		return;

	if (self->client->hook_on)
	{
		if (self->client->frozen_framenum > level.framenum)
		{
			if (self->client->ctf_grapple)
				CTFResetGrapple(self->client->ctf_grapple);
			
			if (self->client->hookstate == WEAPON_FIRING)
				self->client->hookstate = WEAPON_READY;

			self->client->hook_on = false;
			return;
		}

		if (self->client->hookstate != WEAPON_FIRING)
		{
			damage = (int)sv_hook_damage->value;
			if (self->client->quad_framenum > level.framenum)
				damage *= 4;

			CTFGrappleFire(self, vec3_origin, damage, 0);
		}

		if (self->client->ctf_grapple)
			self->client->hookstate = WEAPON_FIRING;

		return;
	}

	if (self->client->ctf_grapple)		// hook_on will be false at this point
	{
		CTFResetGrapple(self->client->ctf_grapple);
		if (self->client->hookstate == WEAPON_FIRING)
			self->client->hookstate = WEAPON_READY;
	}
}
//CW--

void CTFWeapon_Grapple(edict_t *ent)
{
	static int	pause_frames[]	= {10, 18, 27, 0};
	static int	fire_frames[]	= {6, 0};
	int			prevstate;

	// if the the attack button is still down, stay in the firing frame
	if ((ent->client->buttons & BUTTON_ATTACK) && (ent->client->weaponstate == WEAPON_FIRING) && ent->client->ctf_grapple)
		ent->client->ps.gunframe = 9;

	if (!(ent->client->buttons & BUTTON_ATTACK) && ent->client->ctf_grapple)
	{
		CTFResetGrapple(ent->client->ctf_grapple);
		if (ent->client->weaponstate == WEAPON_FIRING)
			ent->client->weaponstate = WEAPON_READY;
	}


	if (ent->client->newweapon && (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) && (ent->client->weaponstate == WEAPON_FIRING))
	{
		// he wants to change weapons while grappled
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = 32;
	}

	prevstate = ent->client->weaponstate;
	Weapon_Generic(ent, 5, 9, 31, 36, pause_frames, fire_frames, CTFWeapon_Grapple_Fire);

	// if we just switched back to grapple, immediately go to fire frame
	if ((prevstate == WEAPON_ACTIVATING) && (ent->client->weaponstate == WEAPON_READY) && (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY))
	{
		if (!(ent->client->buttons & BUTTON_ATTACK))
			ent->client->ps.gunframe = 9;
		else
			ent->client->ps.gunframe = 5;
		ent->client->weaponstate = WEAPON_FIRING;
	}
}

void CTFTeam_f(edict_t *ent)
{
	char		*t;
	char		*s;
	int			desired_team;

//CW++
	edict_t		*index;
	edict_t		*check;
	qboolean	finished = false;

	if (sv_gametype->value == G_FFA)
	{
		gi_cprintf(ent, PRINT_HIGH, "You don't have a team in FFA games!\n");
		return;
	}
//CW--

	t = gi.args();
	if (!*t)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are on the %s team.\n", CTFTeamName(ent->client->resp.ctf_team));
		return;
	}

	if (teamgame.match > MATCH_SETUP)
	{
		gi_cprintf(ent, PRINT_HIGH, "Can't change teams during a match.\n");
		return;
	}

	if (Q_stricmp(t, "red") == 0)
		desired_team = CTF_TEAM1;
	else if (Q_stricmp(t, "blue") == 0)
		desired_team = CTF_TEAM2;

//CW++
	else if (!Q_stricmp(t, sv_team1_name->string))
		desired_team = CTF_TEAM1;
	else if (!Q_stricmp(t, sv_team2_name->string))
		desired_team = CTF_TEAM2;
//CW--

	else
	{
		gi_cprintf(ent, PRINT_HIGH, "Unknown team.\n", t);
		return;
	}

	if (ent->client->resp.ctf_team == desired_team)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are already on the %s team.\n", CTFTeamName(ent->client->resp.ctf_team));
		return;
	}

//CW++
	ent->s.effects = 0;
	ent->s.renderfx = 0;

//	If the player is using a spycam, bring them out of it first.

	if (ent->client->spycam)
		camera_off(ent);

//	Search through the player's linked list of Trap and C4 entities (if any), and pop them.

	if (ent->next_node)
	{
		index = ent->next_node;
		while (index && !finished)
		{
			check = index;
			if (index->next_node)
				index = index->next_node;
			else
				finished = true;

			if (check->die == C4_DieFromDamage)
				C4_Die(check);
			else if (check->die == Trap_DieFromDamage)
				Trap_Die(check);
			else
				gi.dprintf("BUG: Invalid next_node pointer in CTFTeam_f().\nPlease contact musashi@planetquake.com\n");
		}
	}
//CW--

	ent->svflags = 0;
	ent->flags &= ~FL_GODMODE;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = 0;
	s = Info_ValueForKey(ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

	if (ent->solid == SOLID_NOT)	// observer
	{
//CW++
		ent->client->spectator = false;
//CW--
		PutClientInServer(ent);

//		Add a teleportation effect, and hold the player in place briefly.

		ent->s.event = EV_PLAYER_TELEPORT;
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		gi_bprintf(PRINT_HIGH, "%s joined the %s team.\n", ent->client->pers.netname, CTFTeamName(desired_team));

		return;
	}

	ent->health = 0;

//CW++
	if ((sv_gametype->value == G_TDM) || (sv_gametype->value == G_ASLT))
		ent->client->mod_changeteam = true;
//CW--

	player_die(ent, ent, world, 100000, vec3_origin);												//CW

//	Don't even bother waiting for death frames.

	ent->deadflag = DEAD_DEAD;
	Respawn(ent);
	ent->client->resp.score = 0;
	gi_bprintf(PRINT_HIGH, "%s changed to the %s team.\n", ent->client->pers.netname, CTFTeamName(desired_team));
}

/*
==================
CTFScoreboardMessage
==================
*/
void CTFScoreboardMessage(edict_t *ent, edict_t *killer)
{
	gclient_t	*cl;
	edict_t		*cl_ent;
	char		entry[1024];
	char		string[1400];
	int			len;
	int			sorted[2][MAX_CLIENTS];
	int			sortedscores[2][MAX_CLIENTS];
	int			score;
	int			total[2];
	int			totalscore[2];
	int			last[2];
	int			team;
	int			maxsize = 1000;
	int			i, j, k, n;
	size_t		entryLen, stringLen;

	// sort the clients by team and score
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			team = 0;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			team = 1;
		else
			continue; // unknown team?

		score = game.clients[i].resp.score;
		for (j = 0; j < total[team]; j++)
		{
			if (score > sortedscores[team][j])
				break;
		}
		for (k = total[team]; k > j; k--)
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
	Com_sprintf(string, sizeof(string),
					"if 24 xv 8 yv 8 pic 24 endif "
					"xv 40 yv 28 string \"%4d/%-3d\" "
					"xv 98 yv 12 num 2 18 "
					"if 25 xv 168 yv 8 pic 25 endif "
					"xv 200 yv 28 string \"%4d/%-3d\" "
					"xv 256 yv 12 num 2 20 ",
					totalscore[0], total[0],
					totalscore[1], total[1]);
	len = (int)strlen(string);

	for (i = 0; i < 16 ; i++)
	{
		if ((i >= total[0]) && (i >= total[1]))
			break; // we're done
	
		*entry = 0;

		// left side
		if (i < total[0])
		{
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];

			entryLen = strlen(entry);
		//	sprintf(entry + strlen(entry),
			Com_sprintf(entry + entryLen,
					sizeof(entry) - entryLen,
					"ctf 0 %d %d %d %d ",
					42 + (i * 8),
					sorted[0][i],
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
			//	sprintf(entry + strlen(entry), "xv 56 yv %d picn sbfctf2 ",	42 + (i * 8));
				entryLen = strlen(entry);
				Com_sprintf(entry + entryLen, sizeof(entry) - entryLen, "xv 56 yv %d picn sbfctf2 ",	42 + (i * 8));
			}

			if (maxsize - len > strlen(entry))
			{
			//	strcat(string, entry);
				Com_strcat(string, sizeof(string), entry);
				len = (int)strlen(string);
				last[0] = i;
			}
		}

		*entry = 0;																					//CW++

		// right side
		if (i < total[1])
		{
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];

			entryLen = strlen(entry);
		//	sprintf(entry + strlen(entry),
			Com_sprintf(entry + entryLen,
					sizeof(entry) - entryLen,
					"ctf 160 %d %d %d %d ",
					42 + (i * 8),
					sorted[1][i],
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
			//	sprintf(entry + strlen(entry), "xv 216 yv %d picn sbfctf1 ", 42 + i * 8);
				entryLen = strlen(entry);
				Com_sprintf(entry + entryLen, sizeof(entry) - entryLen, "xv 216 yv %d picn sbfctf1 ", 42 + i * 8);
			}

			if (maxsize - len > strlen(entry))
			{
			//	strcat(string, entry);
				Com_strcat(string, sizeof(string), entry);
				len = (int)strlen(string);
				last[1] = i;
			}
		}
	}

	// put in spectators if we have enough room
	if (last[0] > last[1])
		j = last[0];
	else
		j = last[1];

	j = ((j + 2) * 8) + 42;

	k = n = 0;
	if (maxsize - len > 50)
	{
		for (i = 0; i < (int)maxclients->value; i++)
		{
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			if (!cl_ent->inuse || (cl_ent->solid != SOLID_NOT) || (cl_ent->client->resp.ctf_team != CTF_NOTEAM))
				continue;

			if (!k)
			{
				k = 1;
				Com_sprintf(entry, sizeof(entry), "xv 0 yv %d string2 \"Spectators\" ", j);
				Com_strcat(string, sizeof(string), entry);
				len = (int)strlen(string);
				j += 8;
			}

			entryLen = strlen(entry);
		//	sprintf(entry + strlen(entry),
			Com_sprintf(entry + entryLen,
					sizeof(entry) - entryLen,
					"ctf %d %d %d %d %d ",
					(n & 1) ? 160 : 0,	// x
					j,					// y
					i,					// playernum
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);
			if (maxsize - len > strlen(entry))
			{
				Com_strcat(string, sizeof(string), entry);
				len = (int)strlen(string);
			}
			
			if (n & 1)
				j += 8;

			n++;
		}
	}

	if (total[0] - last[0] > 1) { // couldn't fit everyone
	//	sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ", 42 + (last[0]+1)*8, total[0] - last[0] - 1);
		stringLen = strlen(string);
		Com_sprintf(string + stringLen, sizeof(string) - stringLen, "xv 8 yv %d string \"..and %d more\" ", 42 + (last[0]+1)*8, total[0] - last[0] - 1);
	}

	if (total[1] - last[1] > 1) { // couldn't fit everyone
	//	sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ", 42 + (last[1]+1)*8, total[1] - last[1] - 1);
		stringLen = strlen(string);
		Com_sprintf(string + stringLen, sizeof(string) - stringLen, "xv 168 yv %d string \"..and %d more\" ", 42 + (last[1]+1)*8, total[1] - last[1] - 1);
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}

/*------------------------------------------------------------------------*/
/* TECH																	  */
/*------------------------------------------------------------------------*/

void CTFHasTech(edict_t *who)
{
	if (level.time - who->client->ctf_lasttechmsg > 2.0)
	{
		gi_centerprintf(who, "You already have a TECH powerup.");
		who->client->ctf_lasttechmsg = level.time;
	}
}

gitem_t *CTFWhat_Tech(edict_t *ent)
{
	gitem_t	*tech;
	int		i;

	i = 0;
	while (tnames[i])
	{
		if (((tech = FindItemByClassname(tnames[i])) != NULL) && ent->client->pers.inventory[ITEM_INDEX(tech)])
			return tech;

		i++;
	}
	return NULL;
}

qboolean CTFPickup_Tech(edict_t *ent, edict_t *other)
{
	gitem_t	*tech;
	int		i;

	i = 0;
	while (tnames[i])
	{
		if (((tech = FindItemByClassname(tnames[i])) != NULL) && other->client->pers.inventory[ITEM_INDEX(tech)])
		{
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

static void SpawnTech(gitem_t *item, edict_t *spot);

edict_t *FindTechSpawn(void)																		//CW
{
	edict_t	*spot = NULL;
	int		i = rand() % 16;

	while (i--)
		spot = G_Find(spot, FOFS(classname), "info_player_deathmatch");

	if (!spot)
		spot = G_Find(spot, FOFS(classname), "info_player_deathmatch");

	return spot;
}

static void TechThink(edict_t *tech)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL)
	{
		SpawnTech(tech->item, spot);
		G_FreeEdict(tech);
	}
	else
	{
		tech->nextthink = level.time + CTF_TECH_TIMEOUT;
		tech->think = TechThink;
	}
}

void CTFDrop_Tech(edict_t *ent, gitem_t *item)
{
	edict_t *tech;

	tech = Drop_Item(ent, item);
	tech->nextthink = level.time + CTF_TECH_TIMEOUT;
	tech->think = TechThink;
	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
}

void CTFDeadDropTech(edict_t *ent)
{
	gitem_t	*tech;
	edict_t	*dropped;
	int		i;

	i = 0;
	while (tnames[i])
	{
		if (((tech = FindItemByClassname(tnames[i])) != NULL) && ent->client->pers.inventory[ITEM_INDEX(tech)])
		{
			dropped = Drop_Item(ent, tech);
			dropped->velocity[0] = (float)(rand() % 600) - 300.0;
			dropped->velocity[1] = (float)(rand() % 600) - 300.0;
			dropped->nextthink = level.time + CTF_TECH_TIMEOUT;
			dropped->think = TechThink;
			dropped->owner = NULL;
			ent->client->pers.inventory[ITEM_INDEX(tech)] = 0;
		}
		i++;
	}
}

static void SpawnTech(gitem_t *item, edict_t *spot)
{
	edict_t	*ent;
	vec3_t	forward;
	vec3_t	right;
	vec3_t  angles;

	ent = G_Spawn();

	ent->classname = item->classname;
	ent->item = item;
	ent->spawnflags = DROPPED_ITEM;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	VectorSet(ent->mins, -15.0, -15.0, -15.0);
	VectorSet(ent->maxs, 15.0, 15.0, 15.0);
	gi.setmodel(ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;
	ent->owner = ent;

	angles[0] = 0.0;
	angles[1] = (float)(rand() % 360);
	angles[2] = 0.0;

	AngleVectors(angles, forward, right, NULL);
	VectorCopy(spot->s.origin, ent->s.origin);
	ent->s.origin[2] += 16.0;
	VectorScale(forward, 100.0, ent->velocity);
	ent->velocity[2] = 300.0;

	ent->nextthink = level.time + CTF_TECH_TIMEOUT;
	ent->think = TechThink;

	gi.linkentity(ent);
}

static void SpawnTechs(edict_t *ent)
{
	gitem_t	*tech;
	edict_t	*spot;
	int		i;

	i = 0;
	while (tnames[i])
	{
		if (((tech = FindItemByClassname(tnames[i])) != NULL) && ((spot = FindTechSpawn()) != NULL))
			SpawnTech(tech, spot);
		i++;
	}

	if (ent)
		G_FreeEdict(ent);
}

// frees the passed edict!
void CTFRespawnTech(edict_t *ent)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL)
		SpawnTech(ent->item, spot);

	G_FreeEdict(ent);
}

void CTFSetupTechSpawn(void)
{
	edict_t *ent;

	if (((int)dmflags->value & DF_CTF_NO_TECH))
		return;

//CW++
	if (sv_gametype->value != G_CTF)
		return;
//CW--

	ent = G_Spawn();

//CW++
	ent->classname = "tech_spawner";
	ent->svflags |= SVF_NOCLIENT;
//CW--

	ent->nextthink = level.time + 5.0;																//CW
	ent->think = SpawnTechs;
}

void CTFResetTech(void)
{
	edict_t	*ent;
	int		i;

	for (ent = g_edicts + 1, i = 1; i < globals.num_edicts; i++, ent++)
	{
		if (ent->inuse)
		{
			if (ent->item && (ent->item->flags & IT_TECH))
				G_FreeEdict(ent);
		}
	}
	SpawnTechs(NULL);
}

int CTFApplyResistance(edict_t *ent, int dmg)
{
	static gitem_t	*tech = NULL;
	float			volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech1");

	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)])
	{
	   	gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech1.wav"), volume, ATTN_NORM, 0);
		return ((int)(dmg * 0.5));
	}

	return dmg;
}

int CTFApplyStrength(edict_t *ent, int dmg)
{
	static gitem_t	*tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech2");

	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)])
		return dmg * 2;

	return dmg;
}

qboolean CTFApplyStrengthSound(edict_t *ent)
{
	static gitem_t	*tech = NULL;
	float			volume = 1.0;

//CW++						
	if (ent->client->pers.weapon->weapmodel == WEAP_AGM)
		return false;
//CW--

	if (!tech)
		tech = FindItemByClassname("item_tech2");

	if (tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)])
	{
		if (ent->client->ctf_techsndtime < level.time)
		{
			ent->client->ctf_techsndtime = level.time + 1.0;
			if (ent->client->silencer_shots)
				volume = 0.2;

			if (ent->client->quad_framenum > level.framenum)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2x.wav"), volume, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2.wav"), volume, ATTN_NORM, 0);
		}
		return true;
	}

	return false;
}


qboolean CTFApplyHaste(edict_t *ent)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech3");

	if (tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)])
		return true;

	return false;
}

void CTFApplyHasteSound(edict_t *ent)
{
	static gitem_t	*tech = NULL;
	float			volume = 1.0;

//CW++
	if (!Q_stricmp(ent->client->pers.weapon->pickup_name, "AG Manipulator") && !ent->client->agm_disrupt)
		return;
//CW--

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech3");

	if (tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)] &&	(ent->client->ctf_techsndtime < level.time))
	{
		ent->client->ctf_techsndtime = level.time + 1.0;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech3.wav"), volume, ATTN_NORM, 0);
	}

//CW++
	else if (ent->client && (ent->client->haste_framenum > level.framenum))
		gi.sound(ent, CHAN_VOICE, gi.soundindex("items/haste3.wav"), volume, ATTN_NORM, 0);
//CW--
}

void CTFApplyRegeneration(edict_t *ent)
{
	static gitem_t	*tech = NULL;
	gclient_t	*client;
	float		volume = 1.0;
	qboolean	noise = false;
	int			index;

	client = ent->client;
	if (!client)
		return;

	if (ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech4");

	if (tech && client->pers.inventory[ITEM_INDEX(tech)])
	{
		if (client->ctf_regentime < level.time)
		{
			client->ctf_regentime = level.time;
			if (ent->health < 150)
			{
				ent->health += 5;
				if (ent->health > 150)
					ent->health = 150;

				client->ctf_regentime += 0.5;
				noise = true;
			}
			index = ArmorIndex(ent);
			if (index && (client->pers.inventory[index] < 150))
			{
				client->pers.inventory[index] += 5;
				if (client->pers.inventory[index] > 150)
					client->pers.inventory[index] = 150;

				client->ctf_regentime += 0.5;
				noise = true;
			}
		}

		if (noise && (ent->client->ctf_techsndtime < level.time))
		{
			ent->client->ctf_techsndtime = level.time + 1.0;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech4.wav"), volume, ATTN_NORM, 0);
		}
	}
}

qboolean CTFHasRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech4");

	if (tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)])
		return true;

	return false;
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
	{	"item_quad",				2 }, 
	{	"item_invulnerability",		2 },
//CW++
	{	"item_siphon",				2 },
	{	"item_needle",				2 },
	{	"item_haste",				2 },

	{	"weapon_shockrifle",		3 },
	{	"weapon_gausspistol",		3 },
	{	"weapon_railgun",			3 },
	{	"weapon_rocketlauncher",	4 },
	{	"weapon_agm",				4 },
	{	"weapon_esg",				4 },
	{	"weapon_flamethrower",		4 },
	{	"weapon_mac10",				4 },
	{	"weapon_jackhammer",		4 },
	{	"ammo_c4",					4 },
	{	"ammo_traps",				4 },
//CW--
	{	"item_power_screen",		5 },
	{	"item_power_shield",		5 },
	{	"item_armor_body",			6 },
	{	"item_armor_combat",		6 },
	{	"item_armor_jacket",		6 },
//CW++
	{	"item_teleporter",			7 },
//CW--
	{	"item_silencer",			7 },
	{	"item_breather",			7 },
	{	"item_enviro",				7 },
	{	"item_adrenaline",			7 },
	{	"item_bandolier",			8 },
	{	"item_pack",				8 },
	{ NULL, 0 }
};


static void CTFSay_Team_Location (edict_t *who, char *buf, size_t bufSize)
{
	edict_t		*what = NULL;
	edict_t		*hot = NULL;
	edict_t		*flag1;
	edict_t		*flag2;
	gitem_t		*item;
	vec3_t		v;
	float		hotdist = 999999.0;
	float		newdist;
	int			hotindex = 999;
	int			i;
	int			nearteam = -1;
	qboolean	hotsee = false;
	qboolean	cansee;
	char		teamname[32];																		//CW++

	while ((what = loc_findradius(what, who->s.origin, 1024.0)) != NULL)
	{
//		Find 'what' in 'loc_classnames'.

		for (i = 0; loc_names[i].classname; i++)
		{
			if (!Q_stricmp(what->classname, loc_names[i].classname))								//CW
				break;
		}

		if (!loc_names[i].classname)
			continue;
//CW++
//		Ignore randomly spawned items.

		if (what->rnd_spawn)
			continue;
//CW--

//		Something we can see gets priority over something we can't.

		cansee = loc_CanSee(what, who);
		if (cansee && !hotsee)
		{
			hotsee = true;
			hotindex = loc_names[i].priority;
			hot = what;
			VectorSubtract(what->s.origin, who->s.origin, v);
			hotdist = VectorLength(v);
			continue;
		}

//		If we can't see this, but we have something we can see, skip it.

		if (hotsee && !cansee)
			continue;
		if (hotsee && (hotindex < loc_names[i].priority))
			continue;

		VectorSubtract(what->s.origin, who->s.origin, v);
		newdist = VectorLength(v);
		if ((newdist < hotdist) || (cansee && loc_names[i].priority < hotindex))
		{
			hot = what;
			hotdist = newdist;
			hotindex = i;
			hotsee = loc_CanSee(hot, who);
		}
	}

	if (!hot)
	{
		Com_strcpy(buf, bufSize, "nowhere");
		return;
	}

//	We now have the closest item. See if there's more than one in the map; if so,
//	we need to determine what team is closest.

	what = NULL;
	while ((what = G_Find(what, FOFS(classname), hot->classname)) != NULL)
	{
		if (what == hot)
			continue;

//		If we are here, there is more than one; find out if 'hot' is closer 
//		to the red flag or blue flag.

		if (((flag1 = G_Find(NULL, FOFS(classname), "item_flag_team1")) != NULL) &&	((flag2 = G_Find(NULL, FOFS(classname), "item_flag_team2")) != NULL))
		{
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


	if ((item = FindItemByClassname(hot->classname)) == NULL)
	{
		Com_strcpy(buf, bufSize, "nowhere");
		return;
	}

//	Check to see if it's in water.

	if (who->waterlevel)
		Com_strcpy(buf, bufSize, "in the water ");
	else
		*buf = 0;

//	Determine if it's near or above/below.

	VectorSubtract(who->s.origin, hot->s.origin, v);
	if ((fabs(v[2]) > fabs(v[0])) && (fabs(v[2]) > fabs(v[1])))
	{
		if (v[2] > 0)
			Com_strcat(buf, bufSize, "above ");
		else
			Com_strcat(buf, bufSize, "below ");
	}
	else
		Com_strcat(buf, bufSize, "near ");

	if (nearteam == CTF_TEAM1)
		Com_sprintf(teamname, sizeof(teamname), "the %s ", sv_team1_name->string);										//CW
	else if (nearteam == CTF_TEAM2)
		Com_sprintf(teamname, sizeof(teamname), "the %s ", sv_team2_name->string);										//CW
	else
		Com_sprintf(teamname, sizeof(teamname), "the ");																	//CW

	Com_strcat(buf, bufSize, teamname);																			//CW++
	Com_strcat(buf, bufSize, item->pickup_name);
}

static void CTFSay_Team_Armor (edict_t *who, char *buf, size_t bufSize)
{
	gitem_t	*item;
	int		index;
	int		cells;
	int		power_armor_type;
	size_t	bufLen;

	*buf = 0;

	power_armor_type = PowerArmorType(who);
	if (power_armor_type)
	{
		cells = who->client->pers.inventory[ITEM_INDEX(FindItem("cells"))];
		if (cells) {
		//	sprintf(buf+strlen(buf), "%s with %i cells ", (power_armor_type == POWER_ARMOR_SCREEN)?"Power Screen":"Power Shield", cells);
			bufLen = strlen(buf);
			Com_sprintf(buf+bufLen, bufSize-bufLen, "%s with %i cells ", (power_armor_type == POWER_ARMOR_SCREEN)?"Power Screen":"Power Shield", cells);
		}
	}

	index = ArmorIndex(who);
	if (index)
	{
		item = GetItemByIndex(index);
		if (item)
		{
			if (*buf)
				Com_strcat(buf, bufSize, "and ");
		//	sprintf(buf+strlen(buf), "%i units of %s", who->client->pers.inventory[index], item->pickup_name);
			bufLen = strlen(buf);
			Com_sprintf(buf+bufLen, bufSize-bufLen, "%i units of %s", who->client->pers.inventory[index], item->pickup_name);
		}
	}

	if (!*buf)
		Com_strcpy(buf, bufSize, "no armor");
}

static void CTFSay_Team_Health (edict_t *who, char *buf, size_t bufSize)
{
	if (who->health <= 0)
		Com_strcpy(buf, bufSize, "dead");
	else
		Com_sprintf(buf, bufSize, "%i health", who->health);
}

static void CTFSay_Team_Tech (edict_t *who, char *buf, size_t bufSize)
{
	gitem_t	*tech;
	int		i;

	// see if the player has a tech powerup
	i = 0;
	while (tnames[i])
	{
		if (((tech = FindItemByClassname(tnames[i])) != NULL) && who->client->pers.inventory[ITEM_INDEX(tech)])
		{
			Com_sprintf(buf, bufSize, "the %s", tech->pickup_name);
			return;
		}
		i++;
	}
	Com_strcpy(buf, bufSize, "no powerup");
}

static void CTFSay_Team_Weapon (edict_t *who, char *buf, size_t bufSize)
{
	if (who->client->pers.weapon)
		Com_strcpy(buf, bufSize, who->client->pers.weapon->pickup_name);
	else
		Com_strcpy(buf, bufSize, "none");
}

static void CTFSay_Team_Sight (edict_t *who, char *buf, size_t bufSize)
{
	edict_t	*targ;
	char	s[1024];
	char	s2[1024];
	int		i;
	int		n = 0;

	*s = *s2 = 0;
	for (i = 1; i <= (int)maxclients->value; i++)
	{
		targ = g_edicts + i;
		if (!targ->inuse || (targ == who) || !loc_CanSee(targ, who))
			continue;

		if (*s2)
		{
			if (strlen(s) + strlen(s2) + 3 < sizeof(s))
			{
				if (n)
					Com_strcat(s, sizeof(s), ", ");
				Com_strcat(s, sizeof(s), s2);
				*s2 = 0;
			}
			n++;
		}
		Com_strcpy(s2, sizeof(s2), targ->client->pers.netname);
	}

	if (*s2)
	{
		if (strlen(s) + strlen(s2) + 6 < sizeof(s))
		{
			if (n)
				Com_strcat(s, sizeof(s), " and ");
			Com_strcat(s, sizeof(s), s2);
		}
		Com_strcpy(buf, bufSize, s);
	}
	else
		Com_strcpy(buf, bufSize, "no one");
}

void CTFSay_Team (edict_t *who, char *msg)
{
	edict_t	*cl_ent;
	char	outmsg[256];
	char	buf[256];
	char	*p;
	int		i;

	if (CheckFlood(who))
		return;

	outmsg[0] = 0;

	if (*msg == '\"')
	{
		msg[strlen(msg) - 1] = 0;
		msg++;
	}

	for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 1; msg++)	// was -2
	{
		if (*msg == '%')
		{
			switch (*++msg)
			{
				case 'l' :
				case 'L' :
					CTFSay_Team_Location(who, buf, sizeof(buf));
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 1)	// was -2
					{
						Com_strcpy(p, sizeof(outmsg) - (p - outmsg) - 1, buf);
					//	strcpy(p, buf);
						p += strlen(buf);
					}
					break;

				case 'a' :
				case 'A' :
					CTFSay_Team_Armor(who, buf, sizeof(buf));
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 1)	// was -2
					{
						Com_strcpy(p, sizeof(outmsg) - (p - outmsg) - 1, buf);
					//	strcpy(p, buf);
						p += strlen(buf);
					}
					break;

				case 'h' :
				case 'H' :
					CTFSay_Team_Health(who, buf, sizeof(buf));
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 1)	// was -2
					{
						Com_strcpy(p, sizeof(outmsg) - (p - outmsg) - 1, buf);
					//	strcpy(p, buf);
						p += strlen(buf);
					}
					break;

				case 't' :
				case 'T' :
					CTFSay_Team_Tech(who, buf, sizeof(buf));
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 1)	// was -2
					{
						Com_strcpy(p, sizeof(outmsg) - (p - outmsg) - 1, buf);
					//	strcpy(p, buf);
						p += strlen(buf);
					}
					break;

				case 'w' :
				case 'W' :
					CTFSay_Team_Weapon(who, buf, sizeof(buf));
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 1)	// was -2
					{
						Com_strcpy(p, sizeof(outmsg) - (p - outmsg) - 1, buf);
					//	strcpy(p, buf);
						p += strlen(buf);
					}
					break;

				case 'n' :
				case 'N' :
					CTFSay_Team_Sight(who, buf, sizeof(buf));
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 1)	// was -2
					{
						Com_strcpy(p, sizeof(outmsg) - (p - outmsg) - 1, buf);
					//	strcpy(p, buf);
						p += strlen(buf);
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

	for (i = 0; i < (int)maxclients->value; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

		if (cl_ent->client->resp.ctf_team == who->client->resp.ctf_team)
			gi_cprintf(cl_ent, PRINT_CHAT, "(%s): %s\n", who->client->pers.netname, outmsg);
	}

//CW++
	gi.dprintf("(%s -> team): %s\n", who->client->pers.netname, outmsg);
//CW--
}

/*-----------------------------------------------------------------------*/
/*QUAKED misc_ctf_banner (1 .5 0) (-4 -64 0) (4 64 248) TEAM2
The origin is the bottom of the banner.
The banner is 248 tall.
*/
static void misc_ctf_banner_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_ctf_banner(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex("models/ctf/banner/tris.md2");
	if (ent->spawnflags & 1)	// team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity(ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_ctf_small_banner (1 .5 0) (-4 -32 0) (4 32 124) TEAM2
The origin is the bottom of the banner.
The banner is 124 tall.
*/
void SP_misc_ctf_small_banner(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex("models/ctf/banner/small.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity(ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*-----------------------------------------------------------------------*/

void SetLevelName(pmenu_t *p)
{
	static char levelname[33];

	levelname[0] = '*';
	if (g_edicts[0].message)
		strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
	else
		strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
	levelname[sizeof(levelname) - 1] = 0;
	p->text = levelname;
}


/*-----------------------------------------------------------------------*/

void DoRespawn(edict_t *ent);

void CTFResetAllPlayers(void)
{
	edict_t *ent;
	int		i;

	for (i = 1; i <= (int)maxclients->value; i++)
	{
		ent = g_edicts + i;
		if (!ent->inuse)
			continue;

		if (ent->client->menu)
			PMenu_Close(ent);

		CTFPlayerResetGrapple(ent);

//CW++
		if ((sv_gametype->value != G_FFA))
		{
//CW--
			CTFDeadDropFlag(ent);
			CTFDeadDropTech(ent);
			ent->client->resp.ctf_team = CTF_NOTEAM;
		}
//CW++
		else
			ent->client->resp.ctf_team = CTF_TEAM_FFA;

		if (ent->isabot)
		{
			CTFAssignTeam(ent->client);
			ent->client->resp.ready = true;
			ent->client->routetrace = false;
			ent->client->respawn_time = level.time;
			ent->nextthink = level.time + FRAMETIME;
			ent->s.modelindex2 = 0;
			ent->s.modelindex3 = 0;
			ent->s.modelindex4 = 0;
		}
		else
//CW--
			ent->client->resp.ready = false;

		ent->svflags = 0;
		ent->flags &= ~FL_GODMODE;
		PutClientInServer(ent);
	}

//CW++
	if (sv_gametype->value == G_CTF)
	{
//CW--
		CTFResetTech();
		CTFResetFlags();
	}

	for (ent = g_edicts + 1, i = 1; i < globals.num_edicts; i++, ent++)
	{
		if (ent->inuse && !ent->client)
		{
			if ((ent->solid == SOLID_NOT) && (ent->think == DoRespawn) && (ent->nextthink >= level.time))
			{
				ent->nextthink = 0;
				DoRespawn(ent);
			}
		}
	}

	if (teamgame.match == MATCH_SETUP)
		teamgame.matchtime = level.time + (matchsetuptime->value * 60.0);
}

void CTFAssignGhost(edict_t *ent)
{
	int ghost;
	int	i;

	for (ghost = 0; ghost < MAX_CLIENTS; ghost++)
	{
		if (!teamgame.ghosts[ghost].code)
			break;
	}
	if (ghost == MAX_CLIENTS)
		return;

	teamgame.ghosts[ghost].team = ent->client->resp.ctf_team;
	teamgame.ghosts[ghost].score = 0;
	for (;;)
	{
		teamgame.ghosts[ghost].code = 10000 + (rand() % 90000);
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if ((i != ghost) && (teamgame.ghosts[i].code == teamgame.ghosts[ghost].code))
				break;
		}
		if (i == MAX_CLIENTS)
			break;
	}

	teamgame.ghosts[ghost].ent = ent;
//	strcpy(teamgame.ghosts[ghost].netname, ent->client->pers.netname);
	Com_strcpy(teamgame.ghosts[ghost].netname, sizeof(teamgame.ghosts[ghost].netname), ent->client->pers.netname);
	ent->client->resp.ghost = teamgame.ghosts + ghost;
	gi_cprintf(ent, PRINT_CHAT, "Your ghost code = %d\n", teamgame.ghosts[ghost].code);
	gi_cprintf(ent, PRINT_HIGH, "Following loss of connection, rejoin with your score intact by typing \"ghost %d\"\n", teamgame.ghosts[ghost].code);
}

// start a match
void CTFStartMatch(void)
{
	edict_t	*ent;
	int		i;

	teamgame.match = MATCH_GAME;
	teamgame.matchtime = level.time + (matchtime->value * 60.0);
	teamgame.countdown = false;

	teamgame.team1 = teamgame.team2 = 0;
//CW++
	teamgame.frags1 = teamgame.frags2 = 0;
//CW--

	memset(teamgame.ghosts, 0, sizeof(teamgame.ghosts));

	for (i = 1; i <= (int)maxclients->value; i++)
	{
		ent = g_edicts + i;
		if (!ent->inuse)
			continue;

		ent->client->resp.score = 0;
		ent->client->resp.ctf_state = 0;
		ent->client->resp.ghost = NULL;

		gi_centerprintf(ent, "******************\n\nMATCH HAS STARTED!\n\n******************");

		if (ent->client->resp.ctf_team != CTF_NOTEAM)
		{
//CW++
			if (ent->isabot)
			{
				ent->client->respawn_time = level.time + 1.0 + ((float)(rand()%30)/10.0);
				ent->client->ps.pmove.pm_type = PM_DEAD;
				ent->deadflag = DEAD_DEAD;
				ent->svflags = SVF_NOCLIENT;
				ent->health = 0;
			}
			else
			{
//CW--
			// make up a ghost code
				CTFAssignGhost(ent);
				CTFPlayerResetGrapple(ent);
				ent->svflags = SVF_NOCLIENT;
				ent->flags &= ~FL_GODMODE;

				ent->client->respawn_time = level.time + 1.0 + ((float)(rand()%30)/10.0);
				ent->client->ps.pmove.pm_type = PM_DEAD;
				ent->client->anim_priority = ANIM_DEATH;
				ent->s.frame = FRAME_death308-1;
				ent->client->anim_end = FRAME_death308;
				ent->deadflag = DEAD_DEAD;
				ent->movetype = MOVETYPE_NOCLIP;
				ent->client->ps.gunindex = 0;
				gi.linkentity(ent);
			}
		}
	}
}

void CTFEndMatch(void)
{
	teamgame.match = MATCH_POST;
	gi_bprintf(PRINT_CHAT, "MATCH COMPLETED!\n");

	CTFCalcScores();

//CW++
	if (sv_gametype->value == G_TDM)
	{
		gi_bprintf(PRINT_HIGH, "%s TEAM: %d points\n", sv_team1_name->string, teamgame.total1);		//CW
		gi_bprintf(PRINT_HIGH, "%s TEAM: %d points\n", sv_team2_name->string, teamgame.total2);		//CW
	}
	else
	{
//CW--
		gi_bprintf(PRINT_HIGH, "%s TEAM: %d captures, %d points\n",
			sv_team1_name->string, teamgame.team1, teamgame.total1);								//CW
		gi_bprintf(PRINT_HIGH, "%s TEAM: %d captures, %d points\n",
			sv_team2_name->string, teamgame.team2, teamgame.total2);								//CW
	}

	if (teamgame.team1 > teamgame.team2)
		gi_bprintf(PRINT_CHAT, "%s team won over the %s team by %d CAPTURES!\n",
			sv_team1_name->string, sv_team2_name->string, teamgame.team1 - teamgame.team2);			//CW
	else if (teamgame.team2 > teamgame.team1)
		gi_bprintf(PRINT_CHAT, "%s team won over the %s team by %d CAPTURES!\n",
			sv_team2_name->string, sv_team1_name->string, teamgame.team2 - teamgame.team1);			//CW
	else if (teamgame.total1 > teamgame.total2)		// frag tie breaker
		gi_bprintf(PRINT_CHAT, "%s team won over the %s team by %d POINTS!\n",
			sv_team1_name->string, sv_team2_name->string, teamgame.total1 - teamgame.total2);		//CW
	else if (teamgame.total2 > teamgame.total1) 
		gi_bprintf(PRINT_CHAT, "%s team won over the %s team by %d POINTS!\n",
			sv_team2_name->string, sv_team1_name->string, teamgame.total2 - teamgame.total1);		//CW
	else
		gi_bprintf(PRINT_CHAT, "TIE GAME!\n");

	EndDMLevel();
}

qboolean CTFNextMap(void)
{
	if (teamgame.match == MATCH_POST)
	{
		teamgame.match = MATCH_SETUP;
		CTFResetAllPlayers();
		return true;
	}

	return false;
}


void CTFReady(edict_t *ent)
{
	edict_t	*e;
	int		i;
	int		j;
	int		t1;
	int		t2;

	if (ent->client->resp.ctf_team == CTF_NOTEAM)
	{
		gi_cprintf(ent, PRINT_HIGH, "Pick a team first (hit <TAB> for menu)\n");
		return;
	}

	if (teamgame.match != MATCH_SETUP)
	{
		gi_cprintf(ent, PRINT_HIGH, "A match is not being setup.\n");
		return;
	}

	if (ent->client->resp.ready)
	{
		gi_cprintf(ent, PRINT_HIGH, "You have already committed.\n");
		return;
	}

	ent->client->resp.ready = true;
	gi_bprintf(PRINT_HIGH, "%s is ready.\n", ent->client->pers.netname);

	t1 = t2 = 0;
	for (j = 0, i = 1; i <= (int)maxclients->value; i++)
	{
		e = g_edicts + i;
		if (!e->inuse)
			continue;

		if ((e->client->resp.ctf_team != CTF_NOTEAM) && !e->client->resp.ready)
			j++;
		if (e->client->resp.ctf_team == CTF_TEAM1)
			t1++;
		else if (e->client->resp.ctf_team == CTF_TEAM2)
			t2++;
	}

	if (!j && t1 && t2)
	{
		// everyone has committed
		gi_bprintf(PRINT_CHAT, "All players have committed. Match starting.\n");
		teamgame.match = MATCH_PREGAME;
		teamgame.matchtime = level.time + matchstarttime->value;
		teamgame.countdown = false;
		gi.positioned_sound(world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/talk1.wav"), 1, ATTN_NONE, 0);
	}
}

void CTFNotReady(edict_t *ent)
{
	if (ent->client->resp.ctf_team == CTF_NOTEAM)
	{
		gi_cprintf(ent, PRINT_HIGH, "Pick a team first (hit <TAB> for menu)\n");
		return;
	}

	if (teamgame.match != MATCH_SETUP && teamgame.match != MATCH_PREGAME)
	{
		gi_cprintf(ent, PRINT_HIGH, "A match is not being setup.\n");
		return;
	}

	if (!ent->client->resp.ready)
	{
		gi_cprintf(ent, PRINT_HIGH, "You haven't committed.\n");
		return;
	}

	ent->client->resp.ready = false;
	gi_bprintf(PRINT_HIGH, "%s is no longer ready.\n", ent->client->pers.netname);

	if (teamgame.match == MATCH_PREGAME)
	{
		gi_bprintf(PRINT_CHAT, "Match halted.\n");
		teamgame.match = MATCH_SETUP;
		teamgame.matchtime = level.time + (matchsetuptime->value * 60.0);
	}
}

void CTFGhost(edict_t *ent)
{
	int i;
	int n;

	if (gi.argc() < 2)
	{
		gi_cprintf(ent, PRINT_HIGH, "Usage:  ghost <code>\n");
		return;
	}

	if (ent->client->resp.ctf_team != CTF_NOTEAM)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are already in the game.\n");
		return;
	}
	if (teamgame.match != MATCH_GAME)
	{
		gi_cprintf(ent, PRINT_HIGH, "No match is in progress.\n");
		return;
	}

	n = atoi(gi.argv(1));

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (teamgame.ghosts[i].code && (teamgame.ghosts[i].code == n))
		{
			gi_cprintf(ent, PRINT_HIGH, "Ghost code accepted, your position has been reinstated.\n");
			teamgame.ghosts[i].ent->client->resp.ghost = NULL;
			ent->client->resp.ctf_team = teamgame.ghosts[i].team;
			ent->client->resp.ghost = teamgame.ghosts + i;
			ent->client->resp.score = teamgame.ghosts[i].score;
			ent->client->resp.ctf_state = 0;
			teamgame.ghosts[i].ent = ent;
			ent->svflags = 0;
			ent->flags &= ~FL_GODMODE;
//CW++
			ent->client->spectator = false;
//CW--
			PutClientInServer(ent);
			gi_bprintf(PRINT_HIGH, "%s has been reinstated to %s team.\n", ent->client->pers.netname, CTFTeamName(ent->client->resp.ctf_team));

			return;
		}
	}
	gi_cprintf(ent, PRINT_HIGH, "Invalid ghost code.\n");
}

qboolean CTFMatchSetup(void)
{
	if ((teamgame.match == MATCH_SETUP) || (teamgame.match == MATCH_PREGAME))
		return true;

	return false;
}

qboolean CTFMatchOn(void)
{
	if (teamgame.match == MATCH_GAME)
		return true;

	return false;
}


/*-----------------------------------------------------------------------*/

void CTFJoinTeam1(edict_t *ent, pmenuhnd_t *p);
void CTFJoinTeam2(edict_t *ent, pmenuhnd_t *p);
void CTFCredits(edict_t *ent, pmenuhnd_t *p);
void CTFChaseCam(edict_t *ent, pmenuhnd_t *p);

//CW++
void AWKCredits1(edict_t *ent, pmenuhnd_t *p);
void AWKCredits2(edict_t *ent, pmenuhnd_t *p);
void AWKCredits3(edict_t *ent, pmenuhnd_t *p);
void AWKCredits4(edict_t *ent, pmenuhnd_t *p);
void AWKCredits5(edict_t *ent, pmenuhnd_t *p);
void AWKJoinGame(edict_t *ent, pmenuhnd_t *p);
//CW--

pmenu_t creditsmenu[] = {
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Credits (ThreeWave CTF)",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "*Programming",				PMENU_ALIGN_CENTER, NULL }, 
	{ "Dave 'Zoid' Kirsch",			PMENU_ALIGN_CENTER, NULL },
	{ "*Level Design", 				PMENU_ALIGN_CENTER, NULL },
	{ "Christian Antkow",			PMENU_ALIGN_CENTER, NULL },
	{ "Tim Willits",				PMENU_ALIGN_CENTER, NULL },
	{ "Dave 'Zoid' Kirsch",			PMENU_ALIGN_CENTER, NULL },
	{ "*Art",						PMENU_ALIGN_CENTER, NULL },
	{ "Adrian Carmack Paul Steed",	PMENU_ALIGN_CENTER, NULL },
	{ "Kevin Cloud",				PMENU_ALIGN_CENTER, NULL },
	{ "*Sound",						PMENU_ALIGN_CENTER, NULL },
	{ "Tom 'Bjorn' Klok",			PMENU_ALIGN_CENTER, NULL },
	{ "*Original CTF Art Design",	PMENU_ALIGN_CENTER, NULL },
	{ "Brian 'Whaleboy' Cozzens",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "Return to Main Menu",		PMENU_ALIGN_LEFT, CTFReturnToMain }
};

//CW++
pmenu_t creditsmenu_awk_1[] = {
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Credits",		            PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "*Original Concept and Design", PMENU_ALIGN_CENTER, NULL },
	{ "Eric 'Redchurch' v Rothkirch", PMENU_ALIGN_CENTER, NULL },
	{ "Patrick Martin",				PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "*Programming",				PMENU_ALIGN_CENTER, NULL }, 
	{ "Chris 'Musashi' Walker",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "*Additional Programming",	PMENU_ALIGN_CENTER, NULL },
	{ "Philip 'Maj.Bitch' Blair",	PMENU_ALIGN_CENTER, NULL },
	{ "Doug 'Raven' Buckley",		PMENU_ALIGN_CENTER, NULL },
	{ "David Hyde",					PMENU_ALIGN_CENTER, NULL },
	{ "David 'Zoid' Kirsch",		PMENU_ALIGN_CENTER, NULL },
	{ "'Ponpoko'",					PMENU_ALIGN_CENTER, NULL },
	{ "Richard 'r1ch' Stanway",		PMENU_ALIGN_CENTER, NULL },
	{ "More...",					PMENU_ALIGN_LEFT, AWKCredits2 }
};

pmenu_t creditsmenu_awk_2[] = {
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Credits",					PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "*3D Models & Animation",		PMENU_ALIGN_CENTER, NULL },
	{ "Ben 'Flecko' Foote",			PMENU_ALIGN_CENTER, NULL },
	{ "Jeff 'Stecki' Garstecki",	PMENU_ALIGN_CENTER, NULL },
	{ "Rikki 'Phukymoto' Knight",	PMENU_ALIGN_CENTER, NULL },
	{ "John 'Malekyth' Sheffield",	PMENU_ALIGN_CENTER, NULL },
	{ "Tony 'Crash-S9' Westerlund",	PMENU_ALIGN_CENTER, NULL },
	{ "Alex 'Strykerwolf' Wright",	PMENU_ALIGN_CENTER, NULL },
	{ "Lee David Ash",				PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "*Sound & Voice Effects",		PMENU_ALIGN_CENTER, NULL },
	{ "Eric 'Redchurch' v Rothkirch", PMENU_ALIGN_CENTER, NULL },
	{ "Mike 'Awesund' Kaminski",	PMENU_ALIGN_CENTER, NULL },
	{ "Jake 'Snake' Schytt",		PMENU_ALIGN_CENTER, NULL },
	{ NULL, 						PMENU_ALIGN_CENTER, NULL },
	{ "More...",					PMENU_ALIGN_LEFT, AWKCredits3 }
};

pmenu_t creditsmenu_awk_3[] = {
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Credits",					PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "*2D Skin Art",				PMENU_ALIGN_CENTER, NULL },
	{ "Francois-Xavier Delmotte",	PMENU_ALIGN_CENTER, NULL },
	{ "Jeff 'Stecki' Garstecki",	PMENU_ALIGN_CENTER, NULL },
	{ "Christopher Greenhaw",		PMENU_ALIGN_CENTER, NULL },
	{ "Chris 'Shatter' Holden",		PMENU_ALIGN_CENTER, NULL },
	{ "Rikki 'Phukymoto' Knight",	PMENU_ALIGN_CENTER, NULL },
	{ "Tony 'Crash-S9' Westerlund",	PMENU_ALIGN_CENTER, NULL },
	{ "Alex 'Strykerwolf' Wright",	PMENU_ALIGN_CENTER, NULL },
	{ "Lee David Ash",				PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "More...",					PMENU_ALIGN_LEFT, AWKCredits4 }
};

pmenu_t creditsmenu_awk_4[] = {
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Credits",					PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "*Level Design",				PMENU_ALIGN_CENTER, NULL },
	{ "Shon 'Maric' Shaffer",		PMENU_ALIGN_CENTER, NULL },
	{ "Rob 'Panzer' Berwick",		PMENU_ALIGN_CENTER, NULL },
	{ "Eric 'Sherminator' Sherman",	PMENU_ALIGN_CENTER, NULL },
	{ "Steve 'Jester' Veihl",		PMENU_ALIGN_CENTER, NULL },
	{ "Chris 'Musashi' Walker",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "*Website Design",			PMENU_ALIGN_CENTER, NULL },
	{ "Alex 'sonNeh' Bunskoek",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "More...",					PMENU_ALIGN_LEFT, AWKCredits5 }
};

pmenu_t creditsmenu_awk_5[] = {
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Credits",					PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "*Testing & Gameplay Concepts", PMENU_ALIGN_CENTER, NULL },
	{ "Pete 'Wizkid' Duhamel",		PMENU_ALIGN_CENTER, NULL },
	{ "Jake 'Snake' Schytt",		PMENU_ALIGN_CENTER, NULL },
	{ "Shon 'Maric' Shaffer",		PMENU_ALIGN_CENTER, NULL },
	{ "Eric 'Sherminator' Sherman",	PMENU_ALIGN_CENTER, NULL },
	{ "Mike 'Quiet' Berger",		PMENU_ALIGN_CENTER, NULL },
	{ "Paul 'Bass' Boblett",		PMENU_ALIGN_CENTER, NULL },
	{ "Carl 'CyberGhost' Ewing",	PMENU_ALIGN_CENTER, NULL },
	{ "Gene 'Wild' Kaufman",		PMENU_ALIGN_CENTER, NULL },
	{ "Chris 'Death' Sandy",		PMENU_ALIGN_CENTER, NULL },
	{ "Cassie 'Mistery' Scott",		PMENU_ALIGN_CENTER, NULL },
	{ "Michael 'Ledhead' Smock",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "More...",					PMENU_ALIGN_LEFT, CTFCredits }
};
//CW--

//CW++
static const int jmenu_game = 1;
//CW--
static const int jmenu_level = 2;
static const int jmenu_match = 3;
static const int jmenu_red = 5;
static const int jmenu_blue = 7;
static const int jmenu_chase = 9;

pmenu_t joinmenu[] = {
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Gametype : ThreeWave CTF",	PMENU_ALIGN_CENTER, NULL },										//CW
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "Join Red Team",				PMENU_ALIGN_LEFT, CTFJoinTeam1 },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "Join Blue Team",				PMENU_ALIGN_LEFT, CTFJoinTeam2 },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "Chase Camera",				PMENU_ALIGN_LEFT, CTFChaseCam },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "Credits",					PMENU_ALIGN_LEFT, AWKCredits1 },								//CW
	{ "Message Of The Day",			PMENU_ALIGN_LEFT, ShowMOTD },									//CW
	{ "Help",						PMENU_ALIGN_LEFT, GMenu_Help },									//CW
	{ NULL,							PMENU_ALIGN_LEFT, NULL },										//CW
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL },
	{ "ENTER to select",			PMENU_ALIGN_LEFT, NULL },
	{ "v" AWK_STRING_VERSION,		PMENU_ALIGN_RIGHT, NULL },
};

//CW++
static const int jmenu_ffa_level = 2;

pmenu_t joinmenu_ffa[] = {
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Gametype : FFA",			PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "Join Game",					PMENU_ALIGN_LEFT, AWKJoinGame },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "Chase Camera",				PMENU_ALIGN_LEFT, CTFChaseCam },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "Credits",					PMENU_ALIGN_LEFT, AWKCredits1 },
	{ "Message Of The Day",			PMENU_ALIGN_LEFT, ShowMOTD },
	{ "Help",						PMENU_ALIGN_LEFT, GMenu_Help },
	{ NULL,							PMENU_ALIGN_LEFT, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL },
	{ "ENTER to select",			PMENU_ALIGN_LEFT, NULL },
	{ "v" AWK_STRING_VERSION,		PMENU_ALIGN_RIGHT, NULL }
};

static const int ncmenu_game = 1;
//CW--

pmenu_t nochasemenu[] = {
	{ "*- AWAKENING II -",			PMENU_ALIGN_CENTER, NULL },
	{ "*Gametype : ThreeWave CTF",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "No one to chase",			PMENU_ALIGN_LEFT, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ "Return to main menu",		PMENU_ALIGN_LEFT, CTFReturnToMain }
};

void CTFJoinTeam(edict_t *ent, int desired_team)
{
	char *s;

	PMenu_Close(ent);

	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = 0;

	s = Info_ValueForKey(ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

//	Assign a ghost if we are in match mode.

	if (teamgame.match == MATCH_GAME)
	{
		if (ent->client->resp.ghost)
			ent->client->resp.ghost->code = 0;

		ent->client->resp.ghost = NULL;
		CTFAssignGhost(ent);
	}

//CW++
	ent->client->spectator = false;
	ent->client->chase_target = NULL;
//CW--

	PutClientInServer(ent);

//	Add a teleportation effect, and hold player in place briefly.

	ent->s.event = EV_PLAYER_TELEPORT;
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
	gi_bprintf(PRINT_HIGH, "%s joined the %s team.\n", ent->client->pers.netname, CTFTeamName(desired_team));

	if (teamgame.match == MATCH_SETUP)
	{
		gi_centerprintf(ent,	"***********************\n"
								"Type \"ready\" in console\n"
								"to ready up.\n"
								"***********************");
	}
}

void CTFJoinTeam1(edict_t *ent, pmenuhnd_t *p)
{
	CTFJoinTeam(ent, CTF_TEAM1);
}

void CTFJoinTeam2(edict_t *ent, pmenuhnd_t *p)
{
	CTFJoinTeam(ent, CTF_TEAM2);
}

void CTFChaseCam(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	ChaseHelp(ent);
}

void CTFReturnToMain(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	CTFOpenJoinMenu(ent);
}

void DeathmatchScoreboard(edict_t *ent);

void CTFShowScores(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	ent->client->showscores = true;
	ent->client->showinventory = false;
	DeathmatchScoreboard(ent);
}

int CTFUpdateJoinMenu(edict_t *ent)
{
	static char team1players[32];
	static char team2players[32];
	char	msgbuf[32];																				//CW++
	int		num1;
	int		num2;
	int		i;

	if ((teamgame.match >= MATCH_PREGAME) && matchlock->value)
	{
		joinmenu[jmenu_red].text = "MATCH IS LOCKED";
		joinmenu[jmenu_red].SelectFunc = NULL;
		joinmenu[jmenu_blue].text = "  (entry is not permitted)";
		joinmenu[jmenu_blue].SelectFunc = NULL;
	}
	else
	{
		if (teamgame.match >= MATCH_PREGAME)
		{
//CW++
			Com_sprintf(msgbuf, sizeof(msgbuf), "Join %s MATCH Team", sv_team1_name->string);
			joinmenu[jmenu_red].text = strdup(msgbuf);
			Com_sprintf(msgbuf, sizeof(msgbuf), "Join %s MATCH Team", sv_team2_name->string);
			joinmenu[jmenu_blue].text = strdup(msgbuf);
//CW--
		}
		else
		{
//CW++
			Com_sprintf(msgbuf, sizeof(msgbuf), "Join %s Team", sv_team1_name->string);
			joinmenu[jmenu_red].text = strdup(msgbuf);
			Com_sprintf(msgbuf, sizeof(msgbuf), "Join %s Team", sv_team2_name->string);
			joinmenu[jmenu_blue].text = strdup(msgbuf);
//CW--
		}

		joinmenu[jmenu_red].SelectFunc = CTFJoinTeam1;
		joinmenu[jmenu_blue].SelectFunc = CTFJoinTeam2;
	}

	if (ctf_forcejoin->string && *ctf_forcejoin->string)
	{
		if (Q_stricmp(ctf_forcejoin->string, "red") == 0)
		{
			joinmenu[jmenu_blue].text = NULL;
			joinmenu[jmenu_blue].SelectFunc = NULL;
		}
		else if (Q_stricmp(ctf_forcejoin->string, "blue") == 0)
		{
			joinmenu[jmenu_red].text = NULL;
			joinmenu[jmenu_red].SelectFunc = NULL;
		}
//CW++
		else if (Q_stricmp(ctf_forcejoin->string, sv_team1_name->string))
		{
			joinmenu[jmenu_blue].text = NULL;
			joinmenu[jmenu_blue].SelectFunc = NULL;
		}
		else if (Q_stricmp(ctf_forcejoin->string, sv_team2_name->string))
		{
			joinmenu[jmenu_red].text = NULL;
			joinmenu[jmenu_red].SelectFunc = NULL;
		}
//CW--
	}

	joinmenu[jmenu_chase].text = "Chase Camera (Help)";
	SetLevelName(joinmenu + jmenu_level);

	num1 = num2 = 0;
	for (i = 0; i < (int)maxclients->value; i++)
	{
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			num1++;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			num2++;
	}

	Com_sprintf(team1players, sizeof(team1players), "  (%d players)", num1);
	Com_sprintf(team2players, sizeof(team2players), "  (%d players)", num2);

	switch (teamgame.match)
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
//CW++
		case MATCH_POST:
			joinmenu[jmenu_match].text = "*MATCH COMPLETED";
			break;
//CW--
	}

	if (joinmenu[jmenu_red].text)
		joinmenu[jmenu_red+1].text = team1players;
	else
		joinmenu[jmenu_red+1].text = NULL;

	if (joinmenu[jmenu_blue].text)
		joinmenu[jmenu_blue+1].text = team2players;
	else
		joinmenu[jmenu_blue+1].text = NULL;

	if (num1 > num2)
		return CTF_TEAM1;
	else if (num2 > num1)
		return CTF_TEAM2;

	return (rand() & 1) ? CTF_TEAM1 : CTF_TEAM2;
}

void CTFOpenJoinMenu(edict_t *ent)
{
	int team;

//CW++
	if (ent->client->menu)
		PMenu_Close(ent);

	if (sv_gametype->value == G_FFA)
	{
		SetLevelName(joinmenu_ffa + jmenu_ffa_level);
		PMenu_Open(ent, joinmenu_ffa, -1, sizeof(joinmenu_ffa) / sizeof(pmenu_t), NULL);
		return;
	}
	else if (sv_gametype->value == G_TDM)
		joinmenu[jmenu_game].text = "*Gametype : Team-DM";
	else if (sv_gametype->value == G_ASLT)
		joinmenu[jmenu_game].text = "*Gametype : Assault";
//CW--

	team = CTFUpdateJoinMenu(ent);
	if (ent->client->chase_target)
		team = 8;
	else if (team == CTF_TEAM1)
		team = 4;
	else
		team = 6;

	PMenu_Open(ent, joinmenu, team, sizeof(joinmenu) / sizeof(pmenu_t), NULL);
}

void CTFCredits(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu, -1, sizeof(creditsmenu) / sizeof(pmenu_t), NULL);
}

//CW++
void AWKCredits1(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu_awk_1, -1, sizeof(creditsmenu_awk_1) / sizeof(pmenu_t), NULL);
}

void AWKCredits2(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu_awk_2, -1, sizeof(creditsmenu_awk_2) / sizeof(pmenu_t), NULL);
}

void AWKCredits3(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu_awk_3, -1, sizeof(creditsmenu_awk_3) / sizeof(pmenu_t), NULL);
}

void AWKCredits4(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu_awk_4, -1, sizeof(creditsmenu_awk_4) / sizeof(pmenu_t), NULL);
}

void AWKCredits5(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu_awk_5, -1, sizeof(creditsmenu_awk_5) / sizeof(pmenu_t), NULL);
}

void AWKJoinGame(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);

	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.ctf_team = CTF_TEAM_FFA;
	ent->client->resp.ctf_state = 0;
	ent->client->chase_target = NULL;
	ent->client->spectator = false;
	PutClientInServer(ent);

//	Add a teleportation effect, and briefly hold the player in place.

	ent->s.event = EV_PLAYER_TELEPORT;
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;

	gi_bprintf(PRINT_HIGH, "%s has joined the fray\n", ent->client->pers.netname);
}

void e_menu_on(edict_t *self)
{
	if (!self->owner->client->menu)
		CTFOpenJoinMenu(self->owner);

	G_FreeEdict(self);
}
//CW--

qboolean CTFStartClient(edict_t *ent)
{
//CW++
	edict_t	*e_menu;

	if (ent->isabot)
		return false;
//CW--

	if (ent->client->resp.ctf_team != CTF_NOTEAM)
		return false;

	if (!((int)dmflags->value & DF_FORCEJOIN) || (teamgame.match >= MATCH_SETUP))
	{
//		Start as an observer.

		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->ps.gunindex = 0;
//CW++
		ent->client->chase_target = NULL;
		ent->client->spectator = true;
//CW--
		gi.linkentity(ent);

//CW++
		e_menu = G_Spawn();
		e_menu->classname = "menu_on";
		e_menu->svflags |= SVF_NOCLIENT;
		e_menu->owner = ent;
		e_menu->think = e_menu_on;
		e_menu->nextthink = level.time + 1.0 + random();
//CW--
		return true;
	}
	return false;
}

void CTFObserver(edict_t *ent)
{
	char		userinfo[MAX_INFO_STRING];

//CW++
	edict_t		*index;
	edict_t		*check;
	qboolean	finished = false;
	int			i;

	if (ent->client->spectator)
		return;

//	If the player is using a spycam, bring them out of it first.

	if (ent->client->spycam)
		camera_off(ent);
//CW--

	CTFPlayerResetGrapple(ent);
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);

//CW++
	gi_bprintf(PRINT_HIGH, "%s is watching from the sidelines.\n", ent->client->pers.netname);

//	Clear all our weapon entities, and droptofloor any spikes stuck in us.

	for (i = 0; i < globals.num_edicts; ++i)
	{
		check = &g_edicts[i];
		if (!check->inuse)
			continue;

		if (((check->owner == ent) || (check->oldenemy == ent)) && check->wep_proj)
		{
			G_FreeEdict(check);
			continue;
		}

		if ((check->think == Spike_MoveWithEnt) && (check->enemy == ent))
		{
			check->enemy = NULL;
			G_FreeEdict(check);
			continue;
		}

		if ((check->die == Trap_DieFromDamage) && (check->enemy == ent))
		{
			Trap_Die(check);
			continue;
		}
	}

//	Snuff out flames if player is on fire.

	if (ent->burning)
	{
		ent->burning = false;
		if (ent->flame)				// sanity check
		{
			ent->flame->touch = NULL;
			Flame_Expire(ent->flame);
		}
	}

//	Search through the player's linked list of Trap and C4 entities (if any), and pop them.

	if (ent->next_node)
	{
		index = ent->next_node;
		while (index && !finished)
		{
			check = index;
			if (index->next_node)
				index = index->next_node;
			else
				finished = true;

			if (check->die == C4_DieFromDamage)
				C4_Die(check);
			else if (check->die == Trap_DieFromDamage)
				Trap_Die(check);
			else
				gi.dprintf("BUG: Invalid next_node pointer in CTFObserver().\nPlease contact musashi@planetquake.com\n");
		}
	}

//	Reset remaining client data.

	ent->client->hook_on = false;
	ent->tractored = false;
	ent->disintegrated = false;
	ent->client->mod_changeteam = false;
	ent->client->spectator = true;

	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->antibeam_framenum = 0;
	ent->client->frozen_framenum = 0;
	ent->client->siphon_framenum = 0;
	ent->client->needle_framenum = 0;
	ent->client->haste_framenum = 0;

	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	ent->client->show_gausscharge = false;
	ent->client->show_gausstarget = 0;
	ent->client->gauss_dmg = 0;
	ent->client->gauss_framenum = 0;

	ent->client->agm_charge = 0;
	ent->client->agm_showcharge = false;
	ent->client->agm_tripped = false;
	ent->client->agm_on = false;
	ent->client->agm_push = false;
	ent->client->agm_pull = false;
	ent->client->held_by_agm = false;
	ent->client->flung_by_agm = false;
	ent->client->thrown_by_agm = false;
	ent->client->agm_enemy = NULL;
	if (ent->client->agm_target != NULL)
	{
		ent->client->agm_target->client->held_by_agm = false;
		ent->client->agm_target->client->flung_by_agm = false;
		ent->client->agm_target = NULL;
	}

	ent->client->resp.ready = false;
//CW--

	ent->deadflag = DEAD_NO;
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->resp.ctf_team = CTF_NOTEAM;
	ent->client->ps.gunindex = 0;
	ent->client->resp.score = 0;

	memcpy(userinfo, ent->client->pers.userinfo, sizeof(userinfo));
	InitClientPersistant(ent->client);
	ClientUserinfoChanged(ent, userinfo);
	gi.linkentity(ent);

	CTFOpenJoinMenu(ent);
}

qboolean CTFInMatch(void)
{
	if (teamgame.match > MATCH_NONE)
		return true;

	return false;
}

qboolean CTFCheckRules(void)
{
	edict_t	*ent;
	char	text[64];
	char	msg[64];																				//CW++
	int		t;
	int		i;
	int		j;

	if (teamgame.match != MATCH_NONE)
	{
		t = (int)(teamgame.matchtime - level.time);

		// no team warnings in match mode
		teamgame.warnactive = 0;

		if (t <= 0)		 // time ended on something
		{
			switch (teamgame.match)
			{

//CW++
				case MATCH_NONE:
					return false;
//CW--
				case MATCH_SETUP :
					// go back to normal mode
					if (competition->value < 3)
					{
						teamgame.match = MATCH_NONE;
						gi.cvar_set("competition", "0");											//CW
						CTFResetAllPlayers();
//CW++
						EndDMLevel();
//CW--
					}
					else	// reset the time
						teamgame.matchtime = level.time + matchsetuptime->value * 60.0;
					return false;

				case MATCH_PREGAME :
					// match started! 
					CTFStartMatch();
					gi.positioned_sound(world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NONE, 0);
					return false;

				case MATCH_GAME :
					// match ended!
					CTFEndMatch();
					gi.positioned_sound(world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/bigtele.wav"), 1, ATTN_NONE, 0);
					return false;
//CW++
				case MATCH_POST:
					return false;
//CW--
			}
		}

		if (t == teamgame.lasttime)
			return false;

		teamgame.lasttime = t;

		switch (teamgame.match)
		{
//CW++
			case MATCH_NONE:
				break;
//CW--
			case MATCH_SETUP :
				for (j = 0, i = 1; i <= (int)maxclients->value; i++)
				{
					ent = g_edicts + i;
					if (!ent->inuse)
						continue;

					if ((ent->client->resp.ctf_team != CTF_NOTEAM) && !ent->client->resp.ready)
						j++;
				}

				if (competition->value < 3)
					Com_sprintf(text, sizeof(text), "%02d:%02d SETUP: %d not ready", (int)(t / 60), (int)(t % 60), j);
				else
					Com_sprintf(text, sizeof(text), "SETUP: %d not ready", j);

				gi.configstring(CONFIG_CTF_MATCH, text);
				break;


			case MATCH_PREGAME :
				Com_sprintf(text, sizeof(text), "%02d:%02d UNTIL START", (int)(t / 60), (int)(t % 60));
				gi.configstring(CONFIG_CTF_MATCH, text);

				if ((t <= 10) && !teamgame.countdown)
				{
					teamgame.countdown = true;
					gi.positioned_sound(world->s.origin, world, CHAN_VOICE | CHAN_RELIABLE, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);
				}
				break;

			case MATCH_GAME:
				Com_sprintf(text, sizeof(text), "%02d:%02d MATCH", (int)(t / 60), (int)(t % 60));
				gi.configstring(CONFIG_CTF_MATCH, text);
				if ((t <= 10) && !teamgame.countdown)
				{
					teamgame.countdown = true;
					gi.positioned_sound(world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);
				}
				break;
//CW++
			case MATCH_POST:
				break;
//CW--
		}
		return false;

	}
	else
	{
		int team1 = 0;
		int	team2 = 0;

		if (level.time == teamgame.lasttime)
			return false;

		teamgame.lasttime = level.time;
		// this is only done in non-match (public) mode

		if (warn_unbalanced->value)
		{
			// count up the team totals
			for (i = 1; i <= (int)maxclients->value; i++)
			{
				ent = g_edicts + i;
				if (!ent->inuse)
					continue;

				if (ent->client->resp.ctf_team == CTF_TEAM1)
					team1++;
				else if (ent->client->resp.ctf_team == CTF_TEAM2)
					team2++;
			}

			if ((team1 - team2 >= 2) && (team2 >= 2))
			{
				if (teamgame.warnactive != CTF_TEAM1)
				{
					Com_sprintf(msg, sizeof(msg), "WARNING: %s has too many players", sv_team1_name->string);		//CW++
					teamgame.warnactive = CTF_TEAM1;
					gi.configstring(CONFIG_CTF_TEAMINFO, msg);										//CW
				}
			}
			else if ((team2 - team1 >= 2) && (team1 >= 2))
			{
				if (teamgame.warnactive != CTF_TEAM2)
				{
					Com_sprintf(msg, sizeof(msg), "WARNING: %s has too many players", sv_team2_name->string);		//CW++
					teamgame.warnactive = CTF_TEAM2;
					gi.configstring(CONFIG_CTF_TEAMINFO, msg);										//CW
				}
			}
			else
				teamgame.warnactive = 0;
		}
		else
			teamgame.warnactive = 0;

	}

	if ((int)capturelimit->value && ((teamgame.team1 >= capturelimit->value) || (teamgame.team2 >= capturelimit->value)))
	{
		gi_bprintf(PRINT_HIGH, "Capturelimit hit.\n");
		return true;
	}

	return false;
}

/*--------------------------------------------------------------------------
 * just here to help old map conversions
 *--------------------------------------------------------------------------*/

static void old_teleporter_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*dest;
	vec3_t	forward;
	int		i;

	if (!other->client)
		return;

	dest = G_Find(NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi_cprintf(other, PRINT_HIGH, "Couldn't find destination\n");								//CW
		return;
	}

//ZOID++
	CTFPlayerResetGrapple(other);
//ZOID--

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity(other);

	VectorCopy(dest->s.origin, other->s.origin);
	VectorCopy(dest->s.origin, other->s.old_origin);

	// clear the velocity and hold them in place briefly
	VectorClear(other->velocity);
	other->client->ps.pmove.pm_time = 160>>3;
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->enemy->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i = 0; i < 3 ; i++)
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

	other->s.angles[PITCH] = 0.0;
	other->s.angles[YAW] = dest->s.angles[YAW];
	other->s.angles[ROLL] = 0.0;
	VectorCopy(dest->s.angles, other->client->ps.viewangles);
	VectorCopy(dest->s.angles, other->client->v_angle);

	// give a little forward velocity
	AngleVectors(other->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 200.0, other->velocity);

	// kill anything at the destination
	if (!KillBox(other))
	{
	}

	gi.linkentity(other);
}

/*QUAKED trigger_teleport (0.5 0.5 0.5) ?
Players touching this will be teleported
*/
void SP_trigger_teleport(edict_t *ent)
{
	edict_t	*s;
	int		i;

	if (!ent->target)
	{
		gi.dprintf("trigger_teleporter without a target at %s.\n", vtos(ent->s.origin));			//CW
		G_FreeEdict(ent);
		return;
	}

	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->touch = old_teleporter_touch;
	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);

	// noise maker and splash effect dude
	s = G_Spawn();
	ent->enemy = s;
	for (i = 0; i < 3; i++)
		s->s.origin[i] = ent->mins[i] + (0.5 * (ent->maxs[i] - ent->mins[i]));

	s->s.sound = gi.soundindex("world/hum1.wav");

//CW++
	s->classname = "tele_noise";
//CW--

	gi.linkentity(s);
	
}

/*QUAKED info_teleport_destination (0.5 0.5 0.5) (-16 -16 -24) (16 16 32)
Point trigger_teleports at these.
*/
void SP_info_teleport_destination(edict_t *ent)
{
	ent->s.origin[2] += 16.0;
}


/*----------------------------------------------------------------*/

void CTFStats(edict_t *ent)
{
	edict_t	*e2;
	ghost_t	*g;
	char	st[80];
	char	text[1024];
	int		i;
	int		e;
	size_t	textLen;

	*text = 0;
	if (teamgame.match == MATCH_SETUP)
	{
		for (i = 1; i <= (int)maxclients->value; i++)
		{
			e2 = g_edicts + i;
			if (!e2->inuse)
				continue;

			if (!e2->client->resp.ready && (e2->client->resp.ctf_team != CTF_NOTEAM))
			{
				Com_sprintf(st, sizeof(st), "%s is not ready.\n", e2->client->pers.netname);
				if (strlen(text) + strlen(st) < sizeof(text) - 50)
					Com_strcat(text, sizeof(text), st);
			}
		}
	}

	for (i = 0, g = teamgame.ghosts; i < MAX_CLIENTS; i++, g++)
	{
		if (g->ent)
			break;
	}

	if (i == MAX_CLIENTS)
	{
		if (*text)
			gi_cprintf(ent, PRINT_HIGH, "%s", text);

		gi_cprintf(ent, PRINT_HIGH, "No statistics available.\n");
		return;
	}

	Com_strcat(text, sizeof(text), "  #|Name            |Score|Kills|Death|BasDf|CarDf|Effcy|\n");

	for (i = 0, g = teamgame.ghosts; i < MAX_CLIENTS; i++, g++)
	{
		if (!*g->netname)
			continue;

		if (g->deaths + g->kills == 0)
			e = 50;
		else
			e = (int)(g->kills * 100 / (g->kills + g->deaths));

	//	sprintf(st, "%3d|%-16.16s|%5d|%5d|%5d|%5d|%5d|%4d%%|\n",
		Com_sprintf(st, sizeof(st), "%3d|%-16.16s|%5d|%5d|%5d|%5d|%5d|%4d%%|\n",
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
			textLen = strlen(text);
			Com_sprintf(text+textLen, sizeof(text)-textLen, "And more...\n");
			gi_cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		Com_strcat(text, sizeof(text), st);
	}
	gi_cprintf(ent, PRINT_HIGH, "%s", text);
}

void CTFPlayerList(edict_t *ent)
{
	edict_t	*e2;
	char	st[80];
	char	text[1024];																				//CW: was 1400
	int		i;
	size_t	textLen;

//	Display number, name, connect time, ping, score, and operator status.

	*text = 0;
	for (i = 1; i <= (int)maxclients->value; i++)
	{
		e2 = g_edicts + i;
		if (!e2->inuse)
			continue;

		Com_sprintf(st, sizeof(st), "%3d %-16.16s %02d:%02d %4d %3d%s%s\n",
			i,
			e2->client->pers.netname,
			(int)((level.framenum - e2->client->resp.enterframe) / 600),
			(int)(((level.framenum - e2->client->resp.enterframe) % 600) * 0.1),
			e2->client->ping,
			e2->client->resp.score,
			((teamgame.match == MATCH_SETUP) || (teamgame.match == MATCH_PREGAME)) ?
			((e2->client->resp.ready) ? " (ready)" : " (notready)") : "",
//CW++
			(e2->client->pers.op_status) ? " (OP)" : ((e2->isabot) ? "(Bot)" : ""));
//CW--

		if (strlen(text) + strlen(st) > sizeof(text) - 50)
		{
		//	sprintf(text+strlen(text), "And more...\n");
			textLen = strlen(text);
			Com_sprintf(text+textLen, sizeof(text)-textLen, "And more...\n");
			gi_cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		Com_strcat(text, sizeof(text), st);
	}

	gi_cprintf(ent, PRINT_HIGH, "%s", text);
}


void CTFBoot(edict_t *ent, qboolean ban)															//CW
{
	char *strbuf;																					//CW

//CW++
	if (!ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are not an Operator.\n");
		return;
	}
//CW--

	if (gi.argc() < 2)
	{
//CW++
		CTFPlayerList(ent);
		gi_cprintf(ent, PRINT_HIGH, "Usage:  %s <player_num>\n", (ban)?"op_ban":"op_kick");
//CW--
		return;
	}

//CW++
	if (gi.argv(2) && *gi.argv(2))
		strbuf = gi.argv(2);
	else
		strbuf = "";

	Op_Boot(ent, ban, atoi(gi.argv(1)), strbuf);
//CW--
}

//CW++
void Op_Boot(edict_t *ent, qboolean ban, int pnum, char *reason)
{
	edict_t	*targ;
	char	text[80];

	if ((pnum < 1) || (pnum > (int)maxclients->value))
	{
		gi_cprintf(ent, PRINT_HIGH, "Invalid player number.\n");
		return;
	}

	targ = g_edicts + pnum;
	if (!targ->inuse)
	{
		gi_cprintf(ent, PRINT_HIGH, "That player number is not connected.\n");
		return;
	}

//CW++
	if (targ == ent)
	{
		gi_cprintf(ent, PRINT_HIGH, "You cannot %s yourself!\n", (ban)?"ban":"kick");
		return;
	}

	if (targ->isabot)
		ban = false;

	gi_bprintf(PRINT_CHAT, "%s was %s by %s\n", targ->client->pers.netname, (ban)?"banned":"kicked", ent->client->pers.netname);
	if (strlen(reason))
		gi_bprintf(PRINT_CHAT, "REASON: %s\n", reason);

	if (ban)
	{
		gi_cprintf(ent, PRINT_HIGH, "Banned IP = %s\n", targ->client->pers.ip);
		gi.dprintf("-> Banned IP = %s\n", targ->client->pers.ip);
		
		Com_sprintf(text, sizeof(text), "sv addip %s\n", targ->client->pers.ip);
		gi.AddCommandString(text);
		Com_sprintf(text, sizeof(text), "sv writeip\n");
		gi.AddCommandString(text);
	}
//CW--

	if (targ->isabot)
		RemoveBot(targ);
	else
	{
		Com_sprintf(text, sizeof(text), "kick %d\n", pnum - 1);
		gi.AddCommandString(text);
	}
}


void CTFSetPowerUpEffect(edict_t *ent, int def)
{
	if (ent->client->resp.ctf_team == CTF_TEAM1)
		ent->s.effects |= EF_PENT; // red
	else if (ent->client->resp.ctf_team == CTF_TEAM2)
		ent->s.effects |= EF_QUAD; // blue
	else
		ent->s.effects |= def;
}


//CW++
//======================================================================
// Team-DM stuff
//======================================================================
char *tdm_statusbar =
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
  "xv 246 "
  "num 3 10 "																						//CW
  "xv 296 "
  "pic 9 "
"endif "

//  help / weapon icon 
"if 11 "
  "xv 150 "																							//CW
  "pic 11 "
"endif "

//  frags
"xr	-50 "
"yt 2 "
"num 3 14 "

//tech
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
"xr -80 "
"num 3 18 "
//joined overlay
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
"xr -80 "
"num 3 20 "
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
;

// =====================
// Precache TDM items
// =====================
void TDMPrecache(void)
{
	imageindex_i_ctf1 =   gi.imageindex("i_ctf1"); 
	imageindex_i_ctf2 =   gi.imageindex("i_ctf2"); 
	imageindex_i_ctfj =   gi.imageindex("i_ctfj"); 
	imageindex_sbfctf1 =  gi.imageindex("sbfctf1");
	imageindex_sbfctf2 =  gi.imageindex("sbfctf2");
	imageindex_ctfsb1 =   gi.imageindex("ctfsb1");
	imageindex_ctfsb2 =   gi.imageindex("ctfsb2");
}

// =====================
// TDMScoreboardMessage
// =====================
void TDMScoreboardMessage(edict_t *ent, edict_t *killer)
{
	gclient_t	*cl;
	edict_t		*cl_ent;
	char		entry[1024];
	char		string[1400];
	int			len;
	int			i, j, k, n;
	int			sorted[2][MAX_CLIENTS];
	int			sortedscores[2][MAX_CLIENTS];
	int			score;
	int			total[2];
	int			totalscore[2];
	int			last[2];
	int			team;
	int			maxsize = 1000;
	size_t		entryLen, stringLen;

//	Sort the clients by team and score.

	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	for (i = 0; i < game.maxclients; ++i)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			team = 0;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			team = 1;
		else
			continue; // unknown team?

		score = game.clients[i].resp.score;
		for (j = 0; j < total[team]; ++j)
		{
			if (score > sortedscores[team][j])
				break;
		}
		for (k = total[team]; k > j; --k)
		{
			sorted[team][k] = sorted[team][k-1];
			sortedscores[team][k] = sortedscores[team][k-1];
		}
		sorted[team][j] = i;
		sortedscores[team][j] = score;
		totalscore[team] += score;
		++total[team];
	}

//	Print level name, and add the clients in the sorted order.

	*string = 0;
	len = 0;

//	sprintf(string, "if 24 xv 8 yv 8 pic 24 endif "
	Com_sprintf(string, sizeof(string),
					"if 24 xv 8 yv 8 pic 24 endif "
					"xv 40 yv 28 string \"%4d/%-3d\" "
					"if 25 xv 168 yv 8 pic 25 endif "
					"xv 200 yv 28 string \"%4d/%-3d\" ",
					totalscore[0], total[0],
					totalscore[1], total[1]);
	len = (int)strlen(string);

	for (i = 0; i < 16; ++i)
	{
		if ((i >= total[0]) && (i >= total[1]))
			break;

		*entry = 0;

//		Left side.

		if (i < total[0])
		{
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];
		//	sprintf(entry+strlen(entry),
			entryLen = strlen(entry);
			Com_sprintf(entry+entryLen, sizeof(entry)-entryLen,
					"ctf 0 %d %d %d %d ",
					42 + i * 8,
					sorted[0][i],
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);

			if (maxsize - len > strlen(entry))
			{
				Com_strcat(string, sizeof(string), entry);
				len = (int)strlen(string);
				last[0] = i;
			}
		}

//		Right side.

		if (i < total[1])
		{
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];
		//	sprintf(entry+strlen(entry),
			entryLen = strlen(entry);
			Com_sprintf(entry+entryLen, sizeof(entry)-entryLen,
				"ctf 160 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

			if (maxsize - len > strlen(entry))
			{
				Com_strcat(string, sizeof(string), entry);
				len = (int)strlen(string);
				last[1] = i;
			}
		}
	}

//	Put in spectators if we have enough room.

	if (last[0] > last[1])
		j = last[0];
	else
		j = last[1];
	j = (j + 2) * 8 + 42;

	k = n = 0;
	if (maxsize - len > 50)
	{
		for (i = 0; i < (int)maxclients->value; ++i)
		{
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			if (!cl_ent->inuse || (cl_ent->solid != SOLID_NOT) || (cl_ent->client->resp.ctf_team != CTF_NOTEAM))
				continue;

			if (!k)
			{
				k = 1;
				Com_sprintf(entry, sizeof(entry), "xv 0 yv %d string2 \"Spectators\" ", j);
				Com_strcat(string, sizeof(string), entry);
				len = (int)strlen(string);
				j += 8;
			}

		//	sprintf(entry+strlen(entry),
			entryLen = strlen(entry);
			Com_sprintf(entry+entryLen, sizeof(entry)-entryLen,
					"ctf %d %d %d %d %d ",
					(n & 1) ? 160 : 0,		// x
					j,						// y
					i,						// playernum
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);
			if (maxsize - len > strlen(entry))
			{
				Com_strcat(string, sizeof(string), entry);
				len = (int)strlen(string);
			}
			
			if (n & 1)
				j += 8;
			++n;
		}
	}

//	Indicate if we couldn't fit everyone in.

	if (total[0] - last[0] > 1) {
	//	sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ", 42 + (last[0]+1)*8, total[0] - last[0] - 1);
		stringLen = strlen(string);
		Com_sprintf(string + stringLen, sizeof(stringLen) - stringLen, "xv 8 yv %d string \"..and %d more\" ", 42 + (last[0]+1)*8, total[0] - last[0] - 1);
	}
	if (total[1] - last[1] > 1) {
	//	sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ", 42 + (last[1]+1)*8, total[1] - last[1] - 1);
		stringLen = strlen(string);
		Com_sprintf(string + stringLen, sizeof(stringLen) - stringLen, "xv 168 yv %d string \"..and %d more\" ", 42 + (last[1]+1)*8, total[1] - last[1] - 1);
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}
//CW--


//CW++
//======================================================================
// Assault stuff
//======================================================================
char *aslt_statusbar =
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
  "xv 246 "
  "num 3 10 "
  "xv 296 "
  "pic 9 "
"endif "

// help / weapon icon 
"if 11 "
  "xv 150 "																							//CW
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
"xr -80 "
"num 3 18 "

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

"xr -80 "
"num 3 20 "

"if 23 "
  "yb -77 "
  "xr -28 "
  "pic 23 "
"endif "

// attacking team indicator
"if 21 "
  "yb -104 "
  "xr -28 "
  "pic 21 "
"endif "

"if 31 "
  "yb -77 "
  "xr -28 "
  "pic 31 "
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
;

// =====================
// Precache Assault items
// =====================
void ASLTPrecache(void)
{
	imageindex_i_ctf1 =   gi.imageindex("i_ctf1"); 
	imageindex_i_ctf2 =   gi.imageindex("i_ctf2"); 
	imageindex_i_ctfj =   gi.imageindex("i_ctfj");
	imageindex_i_aslta =  gi.imageindex("i_aslta");
	imageindex_sbfctf1 =  gi.imageindex("sbfctf1");
	imageindex_sbfctf2 =  gi.imageindex("sbfctf2");
	imageindex_ctfsb1 =   gi.imageindex("ctfsb1");
	imageindex_ctfsb2 =   gi.imageindex("ctfsb2");
}

void ASLTSpawn(void)
{
	memset(&asltgame, 0, sizeof(asltgame));

	asltgame.victory = false;
	asltgame.spawn = 0;

	if (competition->value > 1)			// teamgame will have already been setup by this point
	{
		teamgame.match = MATCH_SETUP;
		teamgame.matchtime = level.time + (matchsetuptime->value * 60.0);
	}

//	Swap attacking and defending sides after each game.

	if (g_round % 2)
		asltgame.t_attack = CTF_TEAM2;
	else
		asltgame.t_attack = CTF_TEAM1;

//	Set end-of-level messages, if they have been defined in the worldspawn entity.

	if (!world->inuse)
		return;

	if (world->message2)
		asltgame.msg_attack = strdup(world->message2);

	if (world->message3)
		asltgame.msg_defend = strdup(world->message3);
}

qboolean ASLTCheckRules(void)
{
	edict_t	*ent;
	char	text[64];
	char	msg[64];
	int		team1 = 0;
	int		team2 = 0;
	int		t;
	int		i;
	int		j;

	if (teamgame.match != MATCH_NONE)
	{
		t = (int)(teamgame.matchtime - level.time);
		teamgame.warnactive = 0;					// no team warnings in match mode

//		Timer ended on something.

		if (t <= 0)
		{
			switch (teamgame.match)
			{
//CW++
				case MATCH_NONE:
					return false;
//CW--
				case MATCH_SETUP :
					if (competition->value < 3)
					{
						teamgame.match = MATCH_NONE;
						gi.cvar_set("competition", "0");
						CTFResetAllPlayers();
						EndDMLevel();
					}
					else
						teamgame.matchtime = level.time + (matchsetuptime->value * 60.0);
					return false;

				case MATCH_PREGAME :
					CTFStartMatch();
					gi.positioned_sound(world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NONE, 0);
					return false;

				case MATCH_GAME :
					CTFEndMatch();
					gi.positioned_sound(world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/bigtele.wav"), 1, ATTN_NONE, 0);
					return false;
//CW++
				case MATCH_POST:
					return false;
//CW--
			}
		}

		if (t == teamgame.lasttime)
			return false;

		teamgame.lasttime = t;

		switch (teamgame.match)
		{
//CW++
			case MATCH_NONE:
				break;
//CW--
			case MATCH_SETUP :
				for (j = 0, i = 1; i <= (int)maxclients->value; ++i)
				{
					ent = g_edicts + i;
					if (!ent->inuse)
						continue;

					if ((ent->client->resp.ctf_team != CTF_NOTEAM) && !ent->client->resp.ready)
						++j;
				}

				if (competition->value < 3)
					Com_sprintf(text, sizeof(text), "%02d:%02d SETUP: %d not ready", (int)(t / 60), (int)(t % 60), j);
				else
					Com_sprintf(text, sizeof(text), "SETUP: %d not ready", j);

				gi.configstring(CONFIG_CTF_MATCH, text);
				break;


			case MATCH_PREGAME :
				Com_sprintf(text, sizeof(text), "%02d:%02d UNTIL START", (int)(t / 60), (int)(t % 60));
				gi.configstring(CONFIG_CTF_MATCH, text);

				if ((t <= 10) && !teamgame.countdown)
				{
					teamgame.countdown = true;
					gi.positioned_sound(world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);
				}
				break;

			case MATCH_GAME:
				Com_sprintf(text, sizeof(text), "%02d:%02d MATCH", (int)(t / 60), (int)(t % 60));
				gi.configstring(CONFIG_CTF_MATCH, text);
				if ((t <= 10) && !teamgame.countdown)
				{
					teamgame.countdown = true;
					gi.positioned_sound(world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);
				}
				break;
//CW++
			case MATCH_POST:
				break;
//CW--
		}
		return false;

	}
	else	// this is only done in non-match (public) mode
	{
		if (level.time == teamgame.lasttime)
			return false;

		teamgame.lasttime = level.time;

		if (warn_unbalanced->value)
		{
			for (i = 1; i <= (int)maxclients->value; ++i)
			{
				ent = g_edicts + i;
				if (!ent->inuse)
					continue;

				if (ent->client->resp.ctf_team == CTF_TEAM1)
					++team1;

				else if (ent->client->resp.ctf_team == CTF_TEAM2)
					++team2;
			}

			if ((team1 - team2 >= 2) && (team2 >= 2))
			{
				if (teamgame.warnactive != CTF_TEAM1)
				{
					Com_sprintf(msg, sizeof(msg), "WARNING: %s has too many players", sv_team1_name->string);
					teamgame.warnactive = CTF_TEAM1;
					gi.configstring(CONFIG_CTF_TEAMINFO, msg);
				}
			}
			else if ((team2 - team1 >= 2) && (team1 >= 2))
			{
				if (teamgame.warnactive != CTF_TEAM2)
				{
					Com_sprintf(msg, sizeof(msg), "WARNING: %s has too many players", sv_team2_name->string);
					teamgame.warnactive = CTF_TEAM2;
					gi.configstring(CONFIG_CTF_TEAMINFO, msg);
				}
			}
			else
				teamgame.warnactive = 0;
		}
		else
			teamgame.warnactive = 0;
	}

	if (asltgame.victory)		// attackers have won
		return true;

	return false;
}

/*
================
SelectASLTSpawnPoint

Go to an Assault spawn point.
================
*/
edict_t *SelectASLTSpawnPoint(edict_t *ent)
{
	edict_t		*point_ok[MAX_SPAWNS];
	edict_t		*point_tf[MAX_SPAWNS];
	edict_t		*spot = NULL;
	char		*cname;
	qboolean	attacker = false;
	int			num_ok = 0;
	int			num_tf = 0;

	if (!ent->client->resp.ctf_state)
		ent->client->resp.ctf_state++;

//	Determine whether we're looking for an attacker's or a defender's spawn point.

	switch (ent->client->resp.ctf_team)
	{
		case CTF_TEAM1:
			if (asltgame.t_attack == CTF_TEAM1)
			{
				cname = "info_player_attack";
				attacker = true;
			}
			else
				cname = "info_player_defend";
			break;

		case CTF_TEAM2:
			if (asltgame.t_attack == CTF_TEAM2)
			{
				cname = "info_player_attack";
				attacker = true;
			}
			else
				cname = "info_player_defend";
			break;

		default:
			return SelectRandomDeathmatchSpawnPoint();
	}

//	Attacker: only respawn at any [info_player_attack] whose 'count' value is the same as the
//	game 'spawn' value.
//	Defender: as attacker, but can also respawn at any [info_player_defend] whose 'count' value
//	is greater than the game 'spawn' value if its SPAWNFLAG_GT_COUNT spawnflag is set.

	while ((spot = G_Find(spot, FOFS(classname), cname)) != NULL)
	{
		if (attacker)
		{
			if (spot->count != asltgame.spawn)
				continue;
		}
		else
		{
			if (spot->count != asltgame.spawn)
			{
				if (!((spot->count > asltgame.spawn) && (spot->spawnflags & SPAWNFLAG_GT_COUNT)))
					continue;
			}
		}

		if (TeamPlayersRangeFromSpot(spot, ent->client->resp.ctf_team) < TELEFRAG_DIST)
		{
			point_tf[num_tf] = spot;
			if (num_tf < MAX_SPAWNS - 1)
				++num_tf;
		}
		else
		{
			point_ok[num_ok] = spot;
			if (++num_ok == MAX_SPAWNS)
				break;
		}
	}

	if ((num_ok == 0) && (num_tf == 0))
	{
		gi.dprintf("INFO: No Assault spawn points for %s team.\n", (attacker)?"attacking":"defending");
		return SelectRandomDeathmatchSpawnPoint();
	}

	if (num_ok)
		return point_ok[rand() % num_ok];

	return point_tf[rand() % num_tf];
}

/*
==================
ASLTShowMission
==================
*/
const int msnmenu_obj = 1;
const int msnmenu_first = 3;
const int msnmenu_last = 15;
const int msnmenu_cmd = 17;

typedef struct mission_info_s
{
	char*	 next;
	int		 obj;
	qboolean first;
} mission_info_t;

void ASLTMissionNextObj(edict_t *ent, pmenuhnd_t *p);
void ASLTMissionClose(edict_t *ent, pmenuhnd_t *p);
void ASLTMissionUpdate(edict_t *ent, pmenuhnd_t *display);

pmenu_t mission_menu[] = {
	{ "*Assault - Mission Briefing",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,						        PMENU_ALIGN_CENTER, NULL },				// objective #
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },				// start of objective text
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, ASLTMissionNextObj }	// next page/close
};

void ASLTMissionNextObj(edict_t *ent, pmenuhnd_t *p)
{
	mission_info_t *mission = p->arg;

	++mission->obj;
	ASLTMissionUpdate(ent, p);
}

void ASLTMissionClose(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
}

void ASLTMissionUpdate(edict_t *ent, pmenuhnd_t *display)
{
	mission_info_t	*mission = display->arg;
	edict_t			*e = NULL;
	char			title[32];
	char			text[LINESIZE + 1];
	char			*buffer = "";
	char			*c = "";
	int				itext = 0;
	int				line;
	int				n_msg;
	qboolean		finished = false;

//	Clear current display.

	for (line = msnmenu_first; line <= msnmenu_last; ++line)
		PMenu_UpdateEntry(display->entries + line, " ", PMENU_ALIGN_LEFT, NULL);

//	Search through map entities until the appropriate [info_mission] is found.

	while ((e = G_Find(e, FOFS(classname), "info_mission")) != NULL)
	{
		if (mission->first)
		{
			if (e->spawnflags & SPAWNFLAG_FIRST)
			{
				mission->first = false;
				break;
			}
			else
				continue;
		}
		else
		{
			if (e->targetname && !Q_stricmp(mission->next, e->targetname))
				break;
			else
				continue;
		}
	}

	if (e == NULL)
	{
		gi_centerprintf(ent, "Cannot find objective #%d for this map.\n", mission->obj);
		PMenu_Close(ent);
		return;
	}

	//	Display the objective number as a subtitle.

	Com_sprintf(title, sizeof(title), "*Objective #%d", mission->obj);
	PMenu_UpdateEntry(display->entries + msnmenu_obj, title, PMENU_ALIGN_CENTER, NULL);

//	Display the message of the current [info_mission].

	n_msg = 1;
	line = msnmenu_first;
	while (!finished)
	{
		if (n_msg == 1)
			buffer = strdup(e->message);
		else if (n_msg == 2)
			buffer = strdup(e->message2);
		else if (n_msg == 3)
			buffer = strdup(e->message3);

		c = buffer;
		while (*c != 0)
		{

//			Dump the line buffer to the menu if:
//			(a) a line's worth of characters have been read into the buffer, or
//			(b) a line feed is reached.

			if ((itext > LINESIZE) || (*c == 10))
			{

//				Word-wrapping: if we're in the middle of a word, backtrack through the buffer until the
//				first space is reached,	and only dump the buffer to that point.

				if ((*c != 32) && (*c != 10))
				{
					while ((*c != 32) && (itext > 0))
					{
						--c;
						--itext;
					}

//					If the word is longer that one line, break it across to the next one.

					if (*c != 32)
					{
						c += LINESIZE;
						itext += LINESIZE + 1;
					}
				}

				*c = 0;
				*(text + itext) = 0;
				PMenu_UpdateEntry(display->entries + line, text, PMENU_ALIGN_LEFT, NULL);
				itext = 0;
				++line;
				++c;

//				Insert consecutive line feeds if they're present in the message string.

				while ((*c == 10) && (line < msnmenu_last))		
				{
					++line;
					++c;
				}

//				If there is a space at the start of a new line, ignore it.

				if (*c == 32)
					++c;
			}

			if (line > msnmenu_last)
				break;

			strncpy(text + itext++, c++, 1);
		}

		++n_msg;
		if (((n_msg == 2) && !e->message2) || ((n_msg == 3) && !e->message3) || (n_msg == 4))
			finished = true;
	}

//	If the end of the message string has been reached, dump any characters in the buffer to the display.

	if (itext && !(line > msnmenu_last))
	{
		*c = 0;
		*(text + itext) = 0;
		PMenu_UpdateEntry(display->entries + line, text, PMENU_ALIGN_LEFT, NULL);
	}

	free(buffer);

//	Set the "next objective" menu command if there is a target [info_mission] to follow.

	if (e->target)
	{
		mission->next = e->target;
		Com_sprintf(text, sizeof(text), "View next objective");
		PMenu_UpdateEntry(display->entries + msnmenu_cmd, text, PMENU_ALIGN_LEFT, ASLTMissionNextObj);
	}
	else
	{
		mission->next = NULL;
		Com_sprintf(text, sizeof(text), "End briefing");
		PMenu_UpdateEntry(display->entries + msnmenu_cmd, text, PMENU_ALIGN_LEFT, ASLTMissionClose);
	}

	PMenu_Update(ent);
}

void ASLTShowMission(edict_t *ent)
{
	mission_info_t	*mission;
	pmenuhnd_t		*menu;

	if (ent->client->menu)
		PMenu_Close(ent);

	mission = gi.TagMalloc(sizeof(*mission), TAG_LEVEL);
	mission->first = true;
	mission->obj = 1;

	menu = PMenu_Open(ent, mission_menu, -1, sizeof(mission_menu) / sizeof(pmenu_t), mission);
	ASLTMissionUpdate(ent, menu);
}
//CW--
