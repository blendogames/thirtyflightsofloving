#include "g_local.h"
#include "m_player.h"
#include "bot_procs.h"
///Q2 Camera Begin
#include "camclient.h"
///Q2 Camera End
#include "p_trail.h"

#include "aj_replacelist.h"
#include "aj_weaponbalancing.h"
#include "aj_menu.h"
#include "aj_lmctf.h"

void botRemovePlayer (edict_t *self);

// AJ funhook
void Cmd_FunHook_f (edict_t *ent);

char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	Com_strcpy (value, sizeof(value), Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char	ent1Team [512];
	char	ent2Team [512];

	//ScarFace- CTF-specific check: fixes bug with players on opposite teams
	//with same selected non-CTF skin not damaging each other
	if (ctf->value)
		if (ent1->client && ent2->client)
			return (ent1->client->resp.ctf_team == ent2->client->resp.ctf_team);

	// ERASER teams
	if (ent1->client && ent2->client)
		if (ent1->client->team && ent2->client->team && (ent1->client->team == ent2->client->team))
			return true;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	Com_strcpy (ent1Team, sizeof(ent1Team), ClientTeam (ent1));
	Com_strcpy (ent2Team, sizeof(ent2Team), ClientTeam (ent2));

	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}

void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//ZOID
	if (cl->menu) {
		PMenu_Next(ent);
		return;
	} else if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}
//ZOID

	cl = ent->client;

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//ZOID
	if (cl->menu) {
		PMenu_Prev(ent);
		return;
	} else if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}
//ZOID

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}



void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi.dprintf ("unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.dprintf ("non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
/*
// ERASER, only enable grapple if calc_nodes = 0
	if (bot_calc_nodes->value && !strcmp(s, "grapple"))
	{
		gi.cprintf (ent, PRINT_HIGH, "Grapple not available while bot_calc_nodes = 1\n");
		return;
	}
*/
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		// RAFAEL
		if (strcmp (it->pickup_name, "HyperBlaster") == 0)
		{
			it = FindItem ("Ionripper");
			index = ITEM_INDEX (it);
			if (!ent->client->pers.inventory[index])
			{
				gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
				return;
			}
		}
		// RAFAEL
		else if (strcmp (it->pickup_name, "Railgun") == 0)
		{
			it = FindItem ("Phalanx");
			index = ITEM_INDEX (it);
			if (!ent->client->pers.inventory[index])
			{
				gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
				return;
			}
		}
		else
		{
			gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
			return;
		}
	}

	it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

// AJ - dont allow dropping when in observer mode
	if (ent->client->observer_mode)
		return;
// end AJ

//ZOID--special case for tech powerups
// AJ added condition so that "drop rune" is also valid
	if ((Q_stricmp(gi.args(), "tech") == 0 || Q_stricmp(gi.args(), "rune") == 0)
	    && (it = CTFWhat_Tech(ent)) != NULL) {
		it->drop (ent, it);
		return;
	}
//ZOID

// AJ allow "drop flag"
	if ((Q_stricmp(gi.args(), "flag") == 0)
	    && (  (ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
		|| (ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
		|| (ent->client->pers.inventory[ITEM_INDEX(flag3_item)])  ))
	{
//		flag1_item->drop (ent, flag1_item); // doesn't matter which flag it works it out
		CTFDrop_Flag(ent, flag1_item);
		return;
	}
// end AJ

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		// RAFAEL
		if (strcmp (it->pickup_name, "HyperBlaster") == 0)
		{
			it = FindItem ("Ionripper");
			index = ITEM_INDEX (it);
			if (!ent->client->pers.inventory[index])
			{
				gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
				return;
			}
		}
		// RAFAEL
		else if (strcmp (it->pickup_name, "Railgun") == 0)
		{
			it = FindItem ("Phalanx");
			index = ITEM_INDEX (it);
			if (!ent->client->pers.inventory[index])
			{
				gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
				return;
			}
		}
		else
		{
			gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
			return;
		}
	}

	it->drop (ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;

	cl = ent->client;

//ZOID
	if (ent->client->menu) {
		PMenu_Close(ent);
		ent->client->update_chase = true;
		return;
	}
//ZOID

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

//ZOID
	if (ctf->value && cl->resp.ctf_team == CTF_NOTEAM) {
// AJ add support for 3TCTF
		if (ttctf->value)
			TTCTFOpenJoinMenu(ent);
		else CTFOpenJoinMenu(ent);
// end AJ
		return;
	}
//ZOID

	cl->showinventory = true;
	cl->showscores = false;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

//ZOID
	if (ent->client->menu) {
		PMenu_Select(ent);
		return;
	}
//ZOID

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

//ZOID
/*
=================
Cmd_LastWeap_f
=================
*/
void Cmd_LastWeap_f (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	cl->pers.lastweapon->use (ent, cl->pers.lastweapon);
}
//ZOID

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
//ZOID
	if (ent->solid == SOLID_NOT)
		return;
//ZOID

	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
//ROGUE
	// make sure no trackers are still hurting us.
	if(ent->client->tracker_pain_framenum)
		RemoveAttackingPainDaemons (ent);

	if (ent->client->owned_sphere)
	{
		G_FreeEdict(ent->client->owned_sphere);
		ent->client->owned_sphere = NULL;
	}
//ROGUE
	player_die (ent, ent, ent, 100000, vec3_origin);
	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;
	respawn (ent);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
//ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
	ent->client->update_chase = true;
//ZOID
}

int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			Com_strcat (large, sizeof(large), "...\n");
			break;
		}
		Com_strcat (large, sizeof(large), small);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}


/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		gi.cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		gi.cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		gi.cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		gi.cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

/*
=================
Cmd_Join_f
=================
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void Cmd_Join_f(edict_t *ent, char *teamname)
{
	int i;
	char	userinfo[MAX_INFO_STRING];

	if (ctf->value)
	{
		gi.cprintf(ent, PRINT_HIGH, "\nTeams not available in CTF mode.\nUse \"sv bluebots <name1> <name2> ..\" and \"sv bluebots <name1> <name2> ..\" to spawn groups of bots in CTF.\n");
		return;
	}

	if (ent->client->team)
	{
		gi.cprintf(ent, PRINT_HIGH, "\nYou are already a member of a team.\nYou must disconnect to change teams.\n\n");
		return;
	}

	for (i=0; i<MAX_TEAMS; i++)
	{
		if (!bot_teams[i])
		{
			gi.cprintf(ent, PRINT_HIGH, "Team \"%s\" does not exist.\n", teamname);
			return;
		}

		if (!bot_teams[i]->ingame && dedicated->value)
			continue;		// only allow joining a team that has been created on the server

		if ( !Q_stricmp(bot_teams[i]->teamname, teamname) || !Q_stricmp(bot_teams[i]->abbrev, teamname) )
		{	// match found

			// check team isn't already full
			if ((bot_teams[i]->num_players >= players_per_team->value) && (bot_teams[i]->num_bots == 0))
			{
				gi.cprintf(ent, PRINT_HIGH, "Team \"%s\" is full.\n", bot_teams[i]->teamname);
				return;
			}

			bot_teams[i]->num_players++;

			if (strlen(bot_teams[i]->default_skin) > 0)	// set the team's skin
			{
				// copy userinfo
				Com_strcpy (userinfo, sizeof(userinfo), ent->client->pers.userinfo);

				// set skin
				Info_SetValueForKey (userinfo, "skin", bot_teams[i]->default_skin);

				// record change
				ClientUserinfoChanged(ent, userinfo);
			}

			// must set this after skin!
			ent->client->team = bot_teams[i];
			bot_teams[i]->ingame = true;		// make sure we enable the team

			my_bprintf(PRINT_HIGH, "%s has joined team %s\n", ent->client->pers.netname, bot_teams[i]->teamname);
			return;
		}
	}
}

void Cmd_Lag_f (edict_t *ent, char *val)
{
	int i;

	i = atoi(val);

	if (i > 0)
	{
		if (i < 1000)
		{
			ent->client->latency = i;
			gi.cprintf(ent, PRINT_HIGH, "Latency set to %i\n", i);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "lag must be lower than 1000\n");
		}
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "lag must be higher than 0\n");
	}
}

void Cmd_Teams_f (edict_t *ent)
{
	char str[256];
	int i, j;

	if (ctf->value)
	{
		gi.cprintf(ent, PRINT_HIGH, "\nTeams not available in CTF mode.\nUse \"sv bluebots <name1> <name2> ..\" and \"sv redbots <name1> <name2> ..\" to spawn groups of bots in CTF.\n\n");
		return;
	}

	gi.cprintf(ent, PRINT_HIGH, "\n=====================================\nAvailable teams:\n\n");

	// list all available teams
	for (i=0; i<MAX_TEAMS; i++)
	{
		if (!bot_teams[i])
			break;

		if (!bot_teams[i]->ingame && dedicated->value)
			continue;	// don't show teams that haven't been created, when in dedicated server mode

		// print the team name
		gi.cprintf(ent, PRINT_HIGH, "%s ", bot_teams[i]->teamname);

		for (j=0; j<(15-strlen(bot_teams[i]->teamname)); j++)
			str[j] = ' ';
		str[j] = 0;

		gi.cprintf(ent, PRINT_HIGH, "%s(%s)", str, bot_teams[i]->abbrev);

		for (j=0; j<(4-strlen(bot_teams[i]->abbrev)); j++)
			str[j] = ' ';
		str[j] = 0;

		gi.cprintf(ent, PRINT_HIGH, str);

		if (bot_teams[i]->ingame)
		{
			gi.cprintf(ent, PRINT_HIGH, "%i plyrs", bot_teams[i]->num_players);
			if (bot_teams[i]->num_bots)
				gi.cprintf(ent, PRINT_HIGH, " (%i bots)\n", bot_teams[i]->num_bots);

			gi.cprintf(ent, PRINT_HIGH, "\n");
		}
		else
			gi.cprintf(ent, PRINT_HIGH, "[none]\n");

	}

	gi.cprintf(ent, PRINT_HIGH, "\n=====================================\n");
}

void Cmd_BotCommands_f	(edict_t	*ent)
{	// show bot info
	gi.dprintf("\n=================================\nSERVER ONLY COMMANDS:\n\n \"bot_num <n>\" - sets the maximum number of bots at once to <n>\n\n \"bot_name <name>\" - spawns a specific bot\n\n \"bot_free_clients <n>\" - makes sure there are always <n> free client spots\n\n \"bot_calc_nodes 0/1\" - Enable/Disable dynamic node-table calculation\n\n \"bot_allow_client_commands <n>\" - set to 1 to allow clients to spawn bots via \"cmd bots <n>\"\n=================================\n\n");
}

void Cmd_Tips_f (edict_t *ent)
{
	gi.cprintf(ent, PRINT_HIGH, "\nERASER TIPS:\n\n * Set \"skill 0-3\" to vary the difficulty of your opponents (1 is default)\n\n * You can create your own bots by editing the file BOTS.CFG in the Eraser directory\n\n * Set \"maxclients 32\" to allow play against more bots\n\n");
}

void Cmd_Botpath_f (edict_t *ent)
{
	trace_t tr;
	vec3_t	dest;

	AngleVectors(ent->client->v_angle, dest, NULL, NULL);
	VectorScale(dest, 600, dest);
	VectorAdd(ent->s.origin, dest, dest);

	tr = gi.trace(ent->s.origin, VEC_ORIGIN, VEC_ORIGIN, dest, ent, MASK_PLAYERSOLID);

	if (tr.ent && tr.ent->bot_client)
	{
		tr.ent->flags |= FL_SHOWPATH;

		gi.dprintf("Showing path for %s\n", tr.ent->client->pers.netname);
	}
}

// toggles the debug path for this client
void Cmd_Showpath_f (edict_t *ent)
{
	if (ent->flags & FL_SHOWPATH)
		ent->flags -= FL_SHOWPATH;
	else
		ent->flags |= FL_SHOWPATH;
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		j;
	edict_t	*other;
	char	*p;
	char	text[2048];

	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	if (arg0)
	{
		Com_strcat (text, sizeof(text), gi.argv(0));
		Com_strcat (text, sizeof(text), " ");
		Com_strcat (text, sizeof(text), gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		Com_strcat (text, sizeof(text), p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	Com_strcat (text, sizeof(text), "\n");

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client || other->bot_client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

//==========================================================
// CTF flagpath hack, provide alternate routes for bots to return
// the flag to base
void FlagPathTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->bot_client)
		return;

	if (ent->last_goal)	// this is a source flagpath
	{
		if (other->client->resp.ctf_team != ent->skill_level)
			return;
		if (other->flagpath_goal)	// already heading for a destination
			return;
	}
	else	// this is a destination
	{
		if (other->flagpath_goal == ent)		// reached destination, so clear it
			other->flagpath_goal = NULL;

		return;
	}

	if (!CarryingFlag(other))
		return;

	// carring flag, if this is a src path, send them on an alternate (safest) route
	if (ent->last_goal)
	{
		float count1, count2;
		int i;

		if (!other->flagpath_goal)
		{
			count1 = count2 = 0;

			for (i=0; i<num_players; i++)
			{
				if (players[i]->client->resp.ctf_team && (players[i]->client->resp.ctf_team != other->client->resp.ctf_team))
				{
					count1 += entdist(ent->last_goal, players[i]);
					count2 += entdist(ent->target_ent, players[i]);
				}
			}

			if (count1 > count2)
				other->flagpath_goal = ent->last_goal;
			else
				other->flagpath_goal = ent->target_ent;
		}

	}
	else	// this is a destination path
	{
		other->flagpath_goal = NULL;
	}
}

extern int dropped_trail;

edict_t *flagpaths[3] = {NULL, NULL, NULL};
void FlagPath (edict_t *ent, int ctf_team)
{
	int			i;
	static int	flagpath_type = 0;

	if (flagpath_type == 0)
	{
		// new source
		if (flagpaths[flagpath_type])
		{
			if (ent->client)
			{
				gi.cprintf(ent, PRINT_HIGH, "Incomplete FlagPath, starting a new path.\n");

				for (i=0; i<3; i++)
				{
					G_FreeEdict(flagpaths[i]);
					flagpaths[i] = NULL;
				}
			}
		}

		flagpaths[flagpath_type] = G_Spawn();
		flagpaths[flagpath_type]->classname = "flag_path_src";
		VectorCopy(ent->s.origin, flagpaths[flagpath_type]->s.origin);
		flagpaths[flagpath_type]->last_goal = NULL;
		flagpaths[flagpath_type]->target_ent = NULL;
		flagpaths[flagpath_type]->skill_level = ctf_team;

		if (ent->client)
			gi.cprintf(ent, PRINT_HIGH, "Flagpath SOURCE dropped.\n");
	}
	else if (flagpath_type == 1)
	{

		flagpaths[flagpath_type] = G_Spawn();
		flagpaths[flagpath_type]->classname = "flag_path_dest";
		VectorCopy(ent->s.origin, flagpaths[flagpath_type]->s.origin);
		flagpaths[flagpath_type]->last_goal = NULL;
		flagpaths[flagpath_type]->target_ent = NULL;
		flagpaths[flagpath_type]->paths[0] = -1;

		flagpaths[0]->last_goal = flagpaths[flagpath_type];

		if (ent->client)
			gi.cprintf(ent, PRINT_HIGH, "Flagpath DEST 1 dropped.\n");
	}
	else if (flagpath_type == 2)
	{
		flagpaths[flagpath_type] = G_Spawn();
		flagpaths[flagpath_type]->classname = "flag_path_dest";
		VectorCopy(ent->s.origin, flagpaths[flagpath_type]->s.origin);
		flagpaths[flagpath_type]->last_goal = NULL;
		flagpaths[flagpath_type]->target_ent = NULL;
		flagpaths[flagpath_type]->paths[0] = -1;

		flagpaths[0]->target_ent = flagpaths[flagpath_type];

		// completed paths, so make them triggers, and clear
		for (i=0; i<3; i++)
		{
			flagpaths[i]->solid = SOLID_TRIGGER;
			VectorSet(flagpaths[i]->mins, -16, -16, -16);
			VectorSet(flagpaths[i]->maxs,  16,  16,  4);
			flagpaths[i]->touch = FlagPathTouch;
			gi.linkentity(flagpaths[i]);

			flagpaths[i] = NULL;
		}

		flagpath_type = -1;

		if (ent->client)
		{
			gi.cprintf(ent, PRINT_HIGH, "Flagpath DEST 2 dropped.\nSequence complete.\n\n");
			dropped_trail = true;
		}
	}

	flagpath_type++;
}

// From Yamagi Q2
static int get_ammo_usage(gitem_t *weap)
{
	if (!weap) {
		return 0;
	}

	// handles grenades and tesla which only use 1 ammo per shot
	// have to check this because they don't store their ammo usage in weap->quantity
	if (weap->flags & IT_AMMO) {
		return 1;
	}

	// weapons store their ammo usage in the quantity field
	return weap->quantity;
}

static gitem_t *cycle_weapon (edict_t *ent)
{
	gclient_t	*cl = NULL;
	gitem_t		*noammo_fallback = NULL;
	gitem_t		*noweap_fallback = NULL;
	gitem_t		*weap = NULL;
	gitem_t		*ammo = NULL;
	int			i;
	int			start;
	int			num_weaps;
	const char	*weapname = NULL;

	if (!ent) {
		return NULL;
	}

	cl = ent->client;

	if (!cl) {
		return NULL;
	}

	num_weaps = gi.argc();

	// find where we want to start the search for the next eligible weapon
	if (cl->newweapon) {
		weapname = cl->newweapon->classname;
	}
	else if (cl->pers.weapon) {
		weapname = cl->pers.weapon->classname;
	}

	if (weapname)
	{
		for (i = 1; i < num_weaps; i++) {
			if (Q_stricmp((char *)weapname, gi.argv(i)) == 0) {
				break;
			}
		}
		i++;

		if (i >= num_weaps) {
			i = 1;
		}
	}
	else {
		i = 1;
	}

	start = i;
	noammo_fallback = NULL;
	noweap_fallback = NULL;

	// find the first eligible weapon in the list we can switch to
	do
	{
		weap = FindItemByClassname(gi.argv(i));

		if (weap && weap != cl->pers.weapon && (weap->flags & IT_WEAPON) && weap->use)
		{
			if (cl->pers.inventory[ITEM_INDEX(weap)] > 0)
			{
				if (weap->ammo)
				{
					ammo = FindItem(weap->ammo);
					if (ammo)
					{
						if (cl->pers.inventory[ITEM_INDEX(ammo)] >= get_ammo_usage(weap)) {
							return weap;
						}
						if (!noammo_fallback) {
							noammo_fallback = weap;
						}
					}
				}
				else {
					return weap;
				}
			}
			else if (!noweap_fallback) {
				noweap_fallback = weap;
			}
		}

		i++;

		if (i >= num_weaps) {
			i = 1;
		}
	}
	while (i != start);

	// if no weapon was found, the fallbacks will be used for
	// printing the appropriate error message to the console
	if (noammo_fallback) {
		return noammo_fallback;
	}

	return noweap_fallback;
}

void Cmd_CycleWeap_f (edict_t *ent)
{
	gitem_t		*weap = NULL;

	if (!ent) {
		return;
	}

	if (gi.argc() <= 1) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: cycleweap classname1 classname2 .. classnameN\n");
		return;
	}

	weap = cycle_weapon(ent);
	if (weap)
	{
		if (ent->client->pers.inventory[ITEM_INDEX(weap)] <= 0) {
			gi.cprintf(ent, PRINT_HIGH, "Out of item: %s\n", weap->pickup_name);
		}
		else {
			weap->use(ent, weap);
		}
	}
}

static gitem_t *preferred_weapon (edict_t *ent)
{
	gclient_t	*cl = NULL;
	gitem_t		*noammo_fallback = NULL;
	gitem_t		*noweap_fallback = NULL;
	gitem_t		*weap = NULL;
	gitem_t		*ammo = NULL;
	int			i;
	int			num_weaps;

	if (!ent) {
		return NULL;
	}

	cl = ent->client;

	if (!cl) {
		return NULL;
	}

	num_weaps = gi.argc();

	// find the first eligible weapon in the list we can switch to
	for (i = 1; i < num_weaps; i++)
	{
		weap = FindItemByClassname(gi.argv(i));

		if (weap && (weap->flags & IT_WEAPON) && weap->use)
		{
			if (cl->pers.inventory[ITEM_INDEX(weap)] > 0)
			{
				if (weap->ammo)
				{
					ammo = FindItem(weap->ammo);
					if (ammo)
					{
						if (cl->pers.inventory[ITEM_INDEX(ammo)] >= get_ammo_usage(weap)) {
							return weap;
						}

						if (!noammo_fallback) {
							noammo_fallback = weap;
						}
					}
				}
				else {
					return weap;
				}
			}
			else if (!noweap_fallback) {
				noweap_fallback = weap;
			}
		}
	}

	// If no weapon was found, the fallbacks will be used for
	// printing the appropriate error message to the console
	if (noammo_fallback) {
		return noammo_fallback;
	}

	return noweap_fallback;
}


void Cmd_PrefWeap_f (edict_t *ent)
{
	gitem_t *weap;

	if (!ent) {
		return;
	}

	if (gi.argc() <= 1) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: prefweap classname1 classname2 .. classnameN\n");
		return;
	}

	weap = preferred_weapon (ent);
	if (weap)
	{
		if (ent->client->pers.inventory[ITEM_INDEX(weap)] <= 0) {
			gi.cprintf(ent, PRINT_HIGH, "Out of item: %s\n", weap->pickup_name);
		}
		else {
			weap->use(ent, weap);
		}
	}
}
// end from Yamagi Q2

/*
=================
ClientCommand
=================
*/
extern float	team1_rushbase_time, team2_rushbase_time, team3_rushbase_time;	// used by RUSHBASE command
extern float	team1_defendbase_time, team2_defendbase_time, team3_defendbase_time;
#define	RUSHBASE_OVERRIDE_TIME	180.0
void Cmd_RuneCount_f (edict_t *ent);

void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client /*|| ent->bot_client*/)
		return;		// not fully in game yet

	cmd = gi.argv(0);

	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0 || Q_stricmp (cmd, "steam") == 0)
	{
// AJ - use the CTF sayteam all the time...
//		if (ctf->value)
			CTFSay_Team(ent, gi.args());
//		else
//			Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}

	if (level.intermissiontime)
		return;

// AJ - funhook
	if (Q_stricmp (cmd, "funhook") == 0)
		Cmd_FunHook_f (ent);
    else if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_stricmp (cmd, "drop") == 0)
		Cmd_Drop_f (ent);
	else if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
	else if (Q_stricmp (cmd, "invnext") == 0)
		SelectNextItem (ent, -1);
	else if (Q_stricmp (cmd, "invprev") == 0)
		SelectPrevItem (ent, -1);
	else if (Q_stricmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
	else if (Q_stricmp (cmd, "invdrop") == 0)
		Cmd_InvDrop_f (ent);
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_stricmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_stricmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	else if (Q_stricmp(cmd, "killtrap") == 0)
		Cmd_KillTrap_f (ent);
	else if (Q_stricmp(cmd, "purgegibs") == 0)
		Cmd_PurgeGibs_f (ent);
	else if (Q_stricmp(cmd, "runecount") == 0)
		Cmd_RuneCount_f (ent);

// CHASECAM
//	else if (Q_stricmp (cmd, "chasecam") == 0)
//		Cmd_Chasecam_Toggle (ent);
	else if (Q_stricmp (cmd, "botname") == 0) {
		if (!bot_allow_client_commands->value)
		{
			gi.dprintf("Server has disabled bot commands\n");
			return;
		}

		spawn_bot (gi.argv(1));
	}
	else if (Q_stricmp (cmd, "bots") == 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "\nThis command is not used anymore.\nUse bot_num # to spawn some bots.\n\n");
	}
	else if (Q_stricmp (cmd, "servcmd") == 0)
	{
		Cmd_BotCommands_f(ent);
	}
	else if (Q_stricmp (cmd, "tips") == 0)
	{
		Cmd_Tips_f(ent);
	}
	else if (Q_stricmp (cmd, "addmd2skin") == 0)
	{
		AddModelSkin(gi.argv(1), gi.argv(2));
	}
	else if (Q_stricmp (cmd, "join") == 0)
	{
		Cmd_Join_f(ent, gi.argv(1));
	}
	else if (Q_stricmp (cmd, "lag") == 0)
	{
		Cmd_Lag_f(ent, gi.argv(1));
	}
	else if (Q_stricmp (cmd, "teams") == 0)
	{
		Cmd_Teams_f(ent);
	}
	else if (Q_stricmp (cmd, "botpath") == 0)
	{
		Cmd_Botpath_f(ent);
	}
	else if (Q_stricmp (cmd, "showpath") == 0)
	{
		Cmd_Showpath_f(ent);
	}
	else if (Q_stricmp (cmd, "group") == 0)
	{
		TeamGroup(ent);
	}
	else if (Q_stricmp (cmd, "disperse") == 0)
	{
		TeamDisperse(ent);
	}
	else if (Q_stricmp (cmd, "rushbase") == 0)
	{
		edict_t	*plyr;
		int	i;
		edict_t *flag = NULL, *enemy_flag = NULL, *enemy_flag2 = NULL, *target_flag = NULL;

		if (!ctf->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "Command only available during CTF play\n");
			return;
		}

		if (ent->client->resp.ctf_team == CTF_TEAM1)
		{
			flag = flag1_ent;
			enemy_flag = flag2_ent;
			enemy_flag2 = flag3_ent;
			team1_rushbase_time = RUSHBASE_OVERRIDE_TIME;
			team1_defendbase_time = 0;
		}
		else if (ent->client->resp.ctf_team == CTF_TEAM2)
		{
			flag = flag2_ent;
			enemy_flag = flag1_ent;
			enemy_flag2 = flag3_ent;
			team2_rushbase_time = RUSHBASE_OVERRIDE_TIME;
			team2_defendbase_time = 0;
		}
		else if (ttctf->value && ent->client->resp.ctf_team == CTF_TEAM3)
		{
			flag = flag3_ent;
			enemy_flag = flag1_ent;
			enemy_flag2 = flag2_ent;
			team3_rushbase_time = RUSHBASE_OVERRIDE_TIME;
			team3_defendbase_time = 0;
		}

		gi.centerprintf(ent, "All available units RUSH BASE!\n\n(Type \"freestyle\" to return to normal)\n", ent->client->pers.netname);

		if (ttctf->value)
		{
			if (random() > 0.5) //ScarFace- decide which base to attack randomly
				target_flag = enemy_flag;
			else
				target_flag = enemy_flag2;
		}
		else
			target_flag = enemy_flag;
		for (i=0; i<num_players; i++)
		{
			plyr = players[i];
			if (plyr->client->resp.ctf_team != ent->client->resp.ctf_team)
				continue;
//			if (plyr->target_ent)
//				continue;
			if (plyr->bot_client)
			{
				//plyr->movetarget = enemy_flag;
				plyr->movetarget = target_flag;
				plyr->movetarget_want = 99;
			}
			else if (plyr != ent)
			{
				gi.cprintf(plyr, PRINT_CHAT, "<%s> Rushing base!\n", ent->client->pers.netname);
			}
		}
	}
	else if (Q_stricmp (cmd, "rushredbase") == 0)
	{
		edict_t	*plyr;
		int	i;
		edict_t *flag = NULL, *enemy_flag = NULL;

		if (!ctf->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "Command only available during CTF play\n");
			return;
		}
		if (ent->client->resp.ctf_team == CTF_TEAM1)
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't tell your bots to rush your own base!\n");
			return;
		}
		if (ent->client->resp.ctf_team == CTF_TEAM2)
		{
			flag = flag2_ent;
			enemy_flag = flag1_ent;
			team2_rushbase_time = RUSHBASE_OVERRIDE_TIME;
			team2_defendbase_time = 0;
		}
		else if (ttctf->value && ent->client->resp.ctf_team == CTF_TEAM3)
		{
			flag = flag3_ent;
			enemy_flag = flag1_ent;
			team3_rushbase_time = RUSHBASE_OVERRIDE_TIME;
			team3_defendbase_time = 0;
		}
		gi.centerprintf(ent, "All available units RUSH RED BASE!\n\n(Type \"freestyle\" to return to normal)\n", ent->client->pers.netname);

		for (i=0; i<num_players; i++)
		{
			plyr = players[i];
			if (plyr->client->resp.ctf_team != ent->client->resp.ctf_team)
				continue;
//			if (plyr->target_ent)
//				continue;
			if (plyr->bot_client)
			{
				plyr->movetarget = enemy_flag;
				plyr->movetarget_want = 99;
			}
			else if (plyr != ent)
			{
				gi.cprintf(plyr, PRINT_CHAT, "<%s> Rushing Red Base!\n", ent->client->pers.netname);
			}
		}
	}
	else if (Q_stricmp (cmd, "rushbluebase") == 0)
	{
		edict_t	*plyr;
		int	i;
		edict_t *flag = NULL, *enemy_flag = NULL;

		if (!ctf->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "Command only available during CTF play\n");
			return;
		}
		if (ent->client->resp.ctf_team == CTF_TEAM1)
		{
			flag = flag1_ent;
			enemy_flag = flag2_ent;
			team1_rushbase_time = RUSHBASE_OVERRIDE_TIME;
			team1_defendbase_time = 0;
		}
		if (ent->client->resp.ctf_team == CTF_TEAM2)
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't tell your bots to rush your own base!\n");
			return;
		}
		else if (ttctf->value && ent->client->resp.ctf_team == CTF_TEAM3)
		{
			flag = flag3_ent;
			enemy_flag = flag2_ent;
			team3_rushbase_time = RUSHBASE_OVERRIDE_TIME;
			team3_defendbase_time = 0;
		}
		gi.centerprintf(ent, "All available units RUSH BLUE BASE!\n\n(Type \"freestyle\" to return to normal)\n", ent->client->pers.netname);

		for (i=0; i<num_players; i++)
		{
			plyr = players[i];
			if (plyr->client->resp.ctf_team != ent->client->resp.ctf_team)
				continue;
//			if (plyr->target_ent)
//				continue;
			if (plyr->bot_client)
			{
				plyr->movetarget = enemy_flag;
				plyr->movetarget_want = 99;
			}
			else if (plyr != ent)
			{
				gi.cprintf(plyr, PRINT_CHAT, "<%s> Rushing Blue Base!\n", ent->client->pers.netname);
			}
		}
	}
	else if (Q_stricmp (cmd, "rushgreenbase") == 0)
	{
		edict_t	*plyr;
		int	i;
		edict_t *flag = NULL, *enemy_flag = NULL;

		if (!ttctf->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "Command only available during 3Team CTF play\n");
			return;
		}
		if (ent->client->resp.ctf_team == CTF_TEAM1)
		{
			flag = flag1_ent;
			enemy_flag = flag3_ent;
			team1_rushbase_time = RUSHBASE_OVERRIDE_TIME;
			team1_defendbase_time = 0;
		}
		if (ent->client->resp.ctf_team == CTF_TEAM2)
		{
			flag = flag2_ent;
			enemy_flag = flag3_ent;
			team2_rushbase_time = RUSHBASE_OVERRIDE_TIME;
			team2_defendbase_time = 0;
		}
		else if (ent->client->resp.ctf_team == CTF_TEAM3)
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't tell your bots to rush your own base!\n");
			return;
		}
		gi.centerprintf(ent, "All available units RUSH GREEN BASE!\n\n(Type \"freestyle\" to return to normal)\n", ent->client->pers.netname);

		for (i=0; i<num_players; i++)
		{
			plyr = players[i];
			if (plyr->client->resp.ctf_team != ent->client->resp.ctf_team)
				continue;
//			if (plyr->target_ent)
//				continue;
			if (plyr->bot_client)
			{
				plyr->movetarget = enemy_flag;
				plyr->movetarget_want = 99;
			}
			else if (plyr != ent)
			{
				gi.cprintf(plyr, PRINT_CHAT, "<%s> Rushing Green Base!\n", ent->client->pers.netname);
			}
		}
	}
	else if (Q_stricmp (cmd, "defendbase") == 0)
	{
		edict_t	*plyr;
		int	i;
		edict_t *flag = NULL, *enemy_flag = NULL, *enemy_flag2 = NULL;

		if (!ctf->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "Command only available during CTF play\n");
			return;
		}
		if (ent->client->resp.ctf_team == CTF_TEAM1)
		{
			flag = flag1_ent;
			enemy_flag = flag2_ent;
			enemy_flag2 = flag3_ent;
			team1_rushbase_time = 0;
			team1_defendbase_time = RUSHBASE_OVERRIDE_TIME;
		}
		else if (ent->client->resp.ctf_team == CTF_TEAM2)
		{
			flag = flag2_ent;
			enemy_flag = flag1_ent;
			enemy_flag2 = flag3_ent;
			team2_rushbase_time = 0;
			team2_defendbase_time = RUSHBASE_OVERRIDE_TIME;
		}
		else if (ttctf->value && ent->client->resp.ctf_team == CTF_TEAM3)
		{
			flag = flag3_ent;
			enemy_flag = flag1_ent;
			enemy_flag2 = flag2_ent;
			team3_rushbase_time = 0;
			team3_defendbase_time = RUSHBASE_OVERRIDE_TIME;
		}
		gi.centerprintf(ent, "All available units DEFEND BASE!\n\n(Type \"freestyle\" to return to normal)\n", ent->client->pers.netname);

		for (i=0; i<num_players; i++)
		{
			plyr = players[i];
			if (plyr->client->resp.ctf_team != ent->client->resp.ctf_team)
				continue;
		//	if (plyr->target_ent)
		//		continue;
			if (plyr->bot_client)
			{
				plyr->movetarget = flag;
				plyr->target_ent = flag;
				plyr->movetarget_want = WANT_SHITYEAH;
			}
			else if (plyr != ent)
			{
				gi.cprintf(plyr, PRINT_CHAT, "<%s> Defending base!\n", ent->client->pers.netname);
			}
		}
	}

	else if (Q_stricmp (cmd, "freestyle") == 0)
	{
		if (!ctf->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "Command only available during CTF play\n");
			return;
		}
		if (ent->client->resp.ctf_team == CTF_TEAM1)
		{
			team1_rushbase_time = 0;
			team1_defendbase_time = 0;
		}
		else if (ent->client->resp.ctf_team == CTF_TEAM2)
		{
			team2_rushbase_time = 0;
			team2_defendbase_time = 0;
		}

		else if (ttctf->value && ent->client->resp.ctf_team == CTF_TEAM3)
		{
			team3_rushbase_time = 0;
			team3_defendbase_time = 0;
		}

		gi.cprintf(ent, PRINT_HIGH, "Returning bots to Freestyle mode.\n");
	}

	else if (Q_stricmp (cmd, "flagpath") == 0)
	{
		FlagPath (ent, ent->client->resp.ctf_team);
	}
	else if (Q_stricmp (cmd, "clear_flagpaths") == 0)
	{
		edict_t	*trav = NULL, *last = NULL;
		int		count = 0;

		trav = G_Find(last, FOFS(classname), "flag_path_src");
	//	while (trav = G_Find(last, FOFS(classname), "flag_path_src"))
		while (trav)
		{
			last = trav;
			G_FreeEdict(trav);
			count++;
			trav = G_Find(last, FOFS(classname), "flag_path_src");
		}

		last = NULL;
		trav = G_Find(last, FOFS(classname), "flag_path_dest");
	//	while (trav = G_Find(last, FOFS(classname), "flag_path_dest"))
		while (trav)
		{
			last = trav;
			G_FreeEdict(trav);
			count++;
			trav = G_Find(last, FOFS(classname), "flag_path_dest");
		}

		if (count)
			gi.cprintf(ent, PRINT_HIGH, "\nSuccessfully cleared all flagpaths\n\n");
	}
	else if (Q_stricmp (cmd, "botpause") == 0)
	{
		paused = !paused;

		if (!paused)
		{	// just resumed play
			int i;

			for (i=0; i<num_players; i++)
			{
				if (!players[i]->bot_client && !ent->client->ctf_grapple)
				{
					players[i]->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				}
			}

			safe_bprintf(PRINT_HIGH, "%s unpaused the game\n", ent->client->pers.netname);
		}
	} // AJ added greenflag to line below
	else if ( (Q_stricmp (cmd, "redflag") == 0) || (Q_stricmp (cmd, "blueflag") == 0) || (Q_stricmp (cmd, "greenflag") == 0) )
	{	// spawn a red flag, which gets saved as node data to make DM levels into CTF levels
		edict_t *flag;
		size_t	cmdLen = strlen(cmd) + 1;

		flag = G_Spawn();
	//	flag->classname = gi.TagMalloc(16, TAG_LEVEL);
	//	strcpy (flag->classname, cmd);
		flag->classname = gi.TagMalloc(cmdLen, TAG_LEVEL);
		Com_strcpy (flag->classname, cmdLen, cmd);
		VectorCopy(ent->s.origin, flag->s.origin);
		VectorCopy(ent->s.angles, flag->s.angles);

		gi.cprintf(ent, PRINT_HIGH, "Dropped \"%s\" node\n", cmd);
		dropped_trail = true;
	}
	else if (Q_stricmp (cmd, "clearflags") == 0)
	{
		edict_t *flag = NULL;

		flag = G_Find( flag, FOFS(classname), "blueflag");
	//	while (flag = G_Find( flag, FOFS(classname), "blueflag"))
		while (flag)
		{
			G_FreeEdict (flag);
			flag = G_Find( flag, FOFS(classname), "blueflag");
		}

	//	flag = NULL;
		flag = G_Find( flag, FOFS(classname), "redflag");
	//	while (flag = G_Find( flag, FOFS(classname), "redflag"))
		while (flag)
		{
			G_FreeEdict (flag);
			flag = G_Find( flag, FOFS(classname), "redflag");
		}
// AJ
		flag = G_Find( flag, FOFS(classname), "greenflag");
	//	while (flag = G_Find( flag, FOFS(classname), "greenflag"))
		while (flag)
		{
			G_FreeEdict (flag);
			flag = G_Find( flag, FOFS(classname), "greenflag");
		}
// end AJ
		gi.cprintf (ent, PRINT_HIGH, "Cleared user created CTF flags.\n");
	}
	else if (Q_stricmp (cmd, "ctf_item") == 0)
	{
		ctf_item_t *old_ctf_item_head;

		cmd = gi.argv(1);

		if (strlen(cmd) == 0)
		{
			gi.cprintf(ent, PRINT_HIGH, "No classname specified, ignored.\n");
			return;
		}

		old_ctf_item_head = ctf_item_head;

		ctf_item_head = gi.TagMalloc(sizeof(ctf_item_t), TAG_LEVEL);
		memset(ctf_item_head, 0, sizeof(ctf_item_t));

	//	strcpy(ctf_item_head->classname, cmd);
		Com_strcpy (ctf_item_head->classname, sizeof(ctf_item_head->classname), cmd);
		VectorCopy(ent->s.origin, ctf_item_head->origin);
		VectorCopy(ent->s.angles, ctf_item_head->angles);
		ctf_item_head->next = old_ctf_item_head;

		gi.cprintf(ent, PRINT_HIGH, "Successfully placed %s at (%i %i %i)\nThis item will appear upon reloading the current map\n", cmd, (int)ent->s.origin[0], (int)ent->s.origin[1], (int)ent->s.origin[2]);
		dropped_trail = true;
	}
	else if (Q_stricmp (cmd, "clear_items") == 0)
	{
		ctf_item_head = NULL;
		gi.cprintf(ent, PRINT_HIGH, "Cleared CTF_ITEM data\n");
		dropped_trail = true;
	}

	else if (Q_stricmp (cmd, "toggle_flagpaths") == 0)
	{
		if (ent->flags & FL_SHOW_FLAGPATHS)
		{
			ent->flags &= ~FL_SHOW_FLAGPATHS;

			// turn off the models
			{
				edict_t *trav = NULL;

				// show lines between alternate routes
			//	while (trav = G_Find(trav, FOFS(classname), "flag_path_src"))
				trav = G_Find(trav, FOFS(classname), "flag_path_src");
				while (trav)
				{
					trav->s.modelindex = 0;
					trav = G_Find(trav, FOFS(classname), "flag_path_src");
				}
			}
		}
		else
			ent->flags |= FL_SHOW_FLAGPATHS;
	}

// AJ LMCTF radio
	else if (Q_stricmp (cmd, "play_team") == 0)
	{
		lmctf_playradio(ent, gi.argv(1));
	}
	else if (Q_stricmp (cmd, "radio") == 0)
	{
		lmctf_radio(ent, gi.argv(1));
	}
	else if (Q_stricmp (cmd, "radiomenu") == 0)
	{
		lmctf_radiomenu(ent);
	}
// end of AJ

//ZOID
	else if (Q_stricmp (cmd, "team") == 0)
	{
		CTFTeam_f (ent);
	} else if (Q_stricmp(cmd, "id") == 0) {
		CTFID_f (ent);
	}
//ZOID

    ///Q2 Camera Begin
	else if (Q_stricmp (cmd, "cam") == 0)
    {
        if (CameraCmd(ent))
			botRemovePlayer(ent);	// Ridah, must remove from list first (make sure to add to list, if ability to change back to player is added)
    }
    ///Q2 Camera End

// AJ
	else if (Q_stricmp (cmd, "menu") == 0)
    {
        ultra_openmenu(ent, NULL);
    }
	else if (Q_stricmp (cmd, "observer") == 0)
    {
        lithium_observer(ent, NULL);
    }
	else if (Q_stricmp (cmd, "killme") == 0)
	{
		ClientDisconnect(ent);
	}
	else if (Q_stricmp (cmd, "replace") == 0)
		Clcmd_Replace();
	else if (Q_stricmp (cmd, "replace_list") == 0)
	{
		replace_details *current=replace_list;

		while (current)
		{
			gi.cprintf(ent, PRINT_HIGH, "Replacing \"%s\" with \"%s\" \n", current->from, current->to);
			current=current->next;
		}
	}
	else if (Q_stricmp (cmd, "bot_drop_all") == 0)
	{	// kill all bots
		edict_t	*trav;
		int	i;

		for (i=0; i < game.maxclients; i++)
		{
			trav = &g_edicts[i];
			if (!trav->bot_client || !trav->inuse)
				continue;
			botDisconnect(trav);
		}
	}
	else if (Q_stricmp(cmd, "!zbot") == 0)
	{
        ent->client->resp.bot_end = 0; // not a bot.
		gi.dprintf ("[ZKick]: client#%d (%s) @ %s is ok\n",
					(ent-g_edicts)-1, ent->client->pers.netname, Info_ValueForKey (ent->client->pers.userinfo,"ip"));
	}
// end AJ
	// from Yamagi Q2
	else if ( !Q_stricmp(cmd, "cycleweap") )
	{
		Cmd_CycleWeap_f (ent);
	}
	else if ( !Q_stricmp(cmd, "prefweap") )
	{
		Cmd_PrefWeap_f (ent);
	}
	// end from Yamagi Q2

	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}

