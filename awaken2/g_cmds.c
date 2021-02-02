// g_cmds.c

#include "g_local.h"
#include "m_player.h"


void SelectNextItem(edict_t *ent, int itflags)
{
	gclient_t	*cl;
	gitem_t		*it;
	int			index;
	int			i;

	cl = ent->client;

//ZOID++
	if (cl->menu)
	{
		PMenu_Next(ent);
		return;
	}
	else if (cl->chase_target)
	{
		ChaseNext(ent);
		return;
	}
//ZOID--

	// scan  for the next valid one
	for (i = 1; i <= MAX_ITEMS; ++i)
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

void SelectPrevItem(edict_t *ent, int itflags)
{
	gclient_t	*cl;
	gitem_t		*it;
	int			index;
	int			i;

	cl = ent->client;

//ZOID++
	if (cl->menu)
	{
		PMenu_Prev(ent);
		return;
	}
	else if (cl->chase_target)
	{
		ChasePrev(ent);
		return;
	}
//ZOID--

	// scan  for the next valid one
	for (i = 1; i <= MAX_ITEMS; ++i)
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

void ValidateSelectedItem(edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem(ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f(edict_t *ent)
{
	gitem_t		*it;
	edict_t		*it_ent;
	qboolean	give_all;
	char		*name;
	int			index;
	int			i;

	if (!(int)sv_cheats->value)																		//CW
	{
		gi_cprintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

//CW++
	if (ent->client->spectator)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are an observer.\n");
		return;
	}
//CW--

	name = gi.args();

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || (Q_stricmp(gi.argv(1), "health") == 0))
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;

		if (!give_all)
			return;
	}

	if (give_all || (Q_stricmp(name, "weapons") == 0))
	{
		for (i = 0; i < game.num_items; i++)
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

	if (give_all || (Q_stricmp(name, "ammo") == 0))
	{
		for (i = 0; i < game.num_items; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;

			Add_Ammo(ent, it, 1000);
		}
		if (!give_all)
			return;
	}

	if (give_all || (Q_stricmp(name, "armor") == 0))
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

	if (give_all || (Q_stricmp(name, "Power Shield") == 0))
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem(it_ent, it);
		Touch_Item(it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i = 0; i < game.num_items; i++)
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

	it = FindItem(name);

	if (!it)
	{
		name = gi.argv(1);
		it = FindItem(name);
		if (!it)
		{
			gi_cprintf(ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi_cprintf(ent, PRINT_HIGH, "non-pickup item\n");
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

//CW++
//		Allow for null item classnames due to Awakening power-up substitutes.

		if (!Q_stricmp(name, "quad damage"))
			it->classname = "item_quad";
		
		if (!Q_stricmp(name, "invulnerability"))
			it->classname = "item_invulnerability";
		
		if (!Q_stricmp(name, "silencer"))
			it->classname = "item_silencer";
//CW--

		it_ent->classname = it->classname;
		SpawnItem(it_ent, it);
		Touch_Item(it_ent, ent, NULL, NULL);
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
void Cmd_God_f(edict_t *ent)
{
	char	*msg;

	if (!(int)sv_cheats->value)																		//CW
	{
		gi_cprintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE))
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gi_cprintf(ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f(edict_t *ent)
{
	char	*msg;

	if (!(int)sv_cheats->value)																		//CW
	{
		gi_cprintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET))
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi_cprintf(ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f(edict_t *ent)
{
	char	*msg;

	if (!(int)sv_cheats->value)																		//CW
	{
		gi_cprintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

//CW++
	if (ent->client->spectator)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are an observer.\n");
		return;
	}
//CW--

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

	gi_cprintf(ent, PRINT_HIGH, msg);
}

//CW++
/*
==================
Cmd_Detonate_C4_f

Detonates all C4 bundles belonging to a player
==================
*/
void Cmd_Detonate_C4_f(edict_t *self)
{
	edict_t		*index;
	edict_t		*check;
	qboolean	found = false;
	qboolean	finished = false;

//	No sneaky detonating C4 from beyond the grave, or in Observer mode!

	if (self->health <= 0)
		return;

	if (self->client->spectator)
		return;

//	Search through the player's linked list of Trap and C4 entities, and explode the C4s.

	if (self->next_node)
	{
		index = self->next_node;
		while (index && !finished)
		{
			check = index;
			if (index->next_node)
				index = index->next_node;
			else
				finished = true;

			if (check->die == C4_DieFromDamage)
			{
				C4_Explode(check);
				found = true;
			}
		}

		if (found)
			gi_cprintf(self, PRINT_HIGH, "C4 detonated!\n");
	}
}

/*
==================
Cmd_Toggle_GP_f

Toggles Gauss Pistol between 'Blaster' and 'Particle Beam' modes.
==================
*/
void Cmd_Toggle_GP_f(edict_t *self)
{
	if (self->client->gauss_particle)
	{
		self->client->gauss_particle ^= 1;
		self->client->show_gausstarget = 0;
	}
	else
	{
		if (self->client->pers.inventory[ITEM_INDEX(FindItem ("slugs"))] < 5)
			gi_centerprintf(self, "Not enough Slugs for Particle Beam\n");
		else if (self->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))] < 25)
			gi_centerprintf(self, "Not enough Cells for Particle Beam\n");
		else
		{
			self->client->gauss_particle ^= 1;
			self->client->show_gausscharge = false;
			self->client->show_gausstarget = 1;
			self->client->showinventory	= 0;
			self->client->showscores = 0;
		}
	}

	if (self->client->gauss_particle)
	{
		if (self->client->pers.weap_note & WN_TEXT)
			gi_centerprintf(self, "Particle Beam\n");
		if (self->client->pers.weap_note & WN_VSEC)
			unicastSound(self, gi.soundindex("voice/u_partcl.wav"), 1.0);							//r1,CW
	}
	else
	{
		if (self->client->pers.weap_note & WN_TEXT)
			gi_centerprintf(self, "Blaster\n");
		if (self->client->pers.weap_note & WN_VSEC)
			unicastSound(self, gi.soundindex("voice/u_blast.wav"), 1.0);							//r1,CW
	}
}

/*
==================
Cmd_Toggle_ESG_f

Toggles ESG between single-shot and multi-spike modes.
==================
*/
void Cmd_Toggle_ESG_f(edict_t *self)
{
	if (self->client->buttons & BUTTON_ATTACK)
		return;

	if ((self->client->ps.gunframe > 7) && (self->client->ps.gunframe < 29))
		return;

	self->client->multi_spike ^= 1;
	self->client->machinegun_shots = 0;

	if (self->client->multi_spike)
	{
		if (self->client->pers.weap_note & WN_TEXT)
			gi_centerprintf(self, "Multi-spikes\n");
		if (self->client->pers.weap_note & WN_VSEC)
			unicastSound(self, gi.soundindex("voice/u_multi.wav"), 1.0);							//r1,CW
	}
	else
	{
		if (self->client->pers.weap_note & WN_TEXT)
			gi_centerprintf(self, "Single spikes\n");
		if (self->client->pers.weap_note & WN_VSEC)
			unicastSound(self, gi.soundindex("voice/u_spike.wav"), 1.0);							//r1,CW
	}
}

/*
==================
Cmd_Toggle_FT_f

Toggles Flamethrower ejecta between normal flames and firebombs.
==================
*/
void Cmd_Toggle_FT_f(edict_t *self)
{
	self->client->ft_firebomb ^= 1;

	if (self->client->ft_firebomb)
	{
		if (self->client->pers.weap_note & WN_TEXT)
			gi_centerprintf(self, "Firebombs\n");
		if (self->client->pers.weap_note & WN_VSEC)
			unicastSound(self, gi.soundindex("voice/u_firebm.wav"), 1.0);							//r1,CW
	}
	else
	{
		if (self->client->pers.weap_note & WN_TEXT)
			gi_centerprintf(self, "Standard Flamethrower\n");
		if (self->client->pers.weap_note & WN_VSEC)
			unicastSound(self, gi.soundindex("voice/u_flames.wav"), 1.0);							//r1,CW
	}
}

/*
==================
Cmd_Toggle_RL_f

Toggles rockets between normal and guided.
==================
*/
void Cmd_Toggle_RL_f(edict_t *self)
{
	self->client->normal_rockets ^= 1;

	if (self->client->normal_rockets)
	{
		if (self->client->pers.weap_note & WN_TEXT)
			gi_centerprintf(self, "Normal Rockets\n");
		if (self->client->pers.weap_note & WN_VSEC)
			unicastSound(self, gi.soundindex("voice/u_nrockt.wav"), 1.0);							//r1,CW
	}
	else
	{
		if (self->client->pers.weap_note & WN_TEXT)
			gi_centerprintf(self, "Guided Rockets\n");
		if (self->client->pers.weap_note & WN_VSEC)
			unicastSound(self, gi.soundindex("voice/u_grockt.wav"), 1.0);							//r1,CW
	}
}

/*
==================
Cmd_Toggle_SR_f

Toggles Shock Rifle between 'Disintegrator' and 'Homing Plasma' modes.
==================
*/
void Cmd_Toggle_SR_f(edict_t *self)
{
	self->client->homing_plasma ^= 1;

	if (self->client->homing_plasma)
	{
		if (self->client->pers.weap_note & WN_TEXT)
			gi_centerprintf(self, "Homing Plasma\n");
		if (self->client->pers.weap_note & WN_VSEC)
			unicastSound(self, gi.soundindex("voice/u_homin.wav"), 1.0);							//r1,CW
	}
	else
	{
		if (self->client->pers.weap_note & WN_TEXT)
			gi_centerprintf(self, "Disintegrator\n");
		if (self->client->pers.weap_note & WN_VSEC)
			unicastSound(self, gi.soundindex("voice/u_shock.wav"), 1.0);							//r1,CW
	}
}

/*
==================
Cmd_Toggle_AGM_f

Toggles AGM between manipulation and cell-disruption modes.
==================
*/
void Cmd_Toggle_AGM_f(edict_t *self)
{
	self->client->agm_disrupt ^= 1;

	if (self->client->agm_disrupt)
	{
		if (self->client->pers.weap_note & WN_TEXT)
			gi_centerprintf(self, "Cellular Disruption\n");
		if (self->client->pers.weap_note & WN_VSEC)
			unicastSound(self, gi.soundindex("voice/u_cd.wav"), 1.0);							//r1,CW
	}
	else
	{
		if (self->client->pers.weap_note & WN_TEXT)
			gi_centerprintf(self, "AG Manipulation\n");
		if (self->client->pers.weap_note & WN_VSEC)
			unicastSound(self, gi.soundindex("voice/u_agm.wav"), 1.0);							//r1,CW
	}
}

/*
==================
Cmd_Hook_f

Handles off-hand grappling.
==================
*/
void Cmd_Hook_f(edict_t *ent)
{
	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->client->hook_on = false;
		CTFPlayerResetGrapple(ent);
		return;
	}

	if (!(int)sv_allow_hook->value)
	{
		gi_centerprintf(ent, "The Grapple is not enabled.\n");
		return;
	}

	if (!(int)sv_hook_offhand->value)
	{
		gi_centerprintf(ent, "The Off-hand Grapple is not enabled.\n");
		ent->client->hook_on = false;
		return;
	}

	if (level.nohook)
	{
		gi_centerprintf(ent, "The Grapple is disabled for this level.\n");
		return;
	}

	ent->client->hook_on = true;
}

void Cmd_Unhook_f(edict_t *ent)
{
	if (!(int)sv_allow_hook->value)
		return;

	if (!(int)sv_hook_offhand->value)
		return;

	if (level.nohook)
		return;

	ent->client->hook_on = false;
	CTFPlayerResetGrapple(ent);
}

/*
==================
Cmd_Menu_f

Displays in-game menu
==================
*/
void Cmd_Menu_f(edict_t *ent)
{
	if (ent->client->spycam)
	{
		gi_centerprintf(ent, "Cannot view menu whilst\nusing a turret monitor.\n");
		return;
	}

//	Spectators use a different menu to those players already in the game.

	if (!ent->client->spectator)
		OpenGameMenu(ent);
	else
		CTFOpenJoinMenu(ent);
}

/*
==================
Cmd_MOTD_f

Displays the Message Of The Day
==================
*/
void Cmd_MOTD_f(edict_t *ent)
{
	PrintMOTD(ent);
}

/*
==================
Cmd_Vote_Map

Handles "vote map <bspname>" command
==================
*/
void Cmd_Vote_Map(edict_t *ent, char *bspname)
{
	FILE		*mapstream;
	char		*maplist[MAX_MAPS];
	char		filename[MAX_QPATH];
	char		text[128];
	qboolean	result;
	qboolean	finished = false;
	qboolean	found = false;
	int			nmaps = 0;
	int			fnum;
	int			i;

	if (strlen(bspname) > MAX_QPATH - 1)
	{
		gi_cprintf(ent, PRINT_HIGH, "Requested map name is too long.\n");
		return;
	}

//	The requested map must be in the map list file, unless the client is an Operator
//	and	the server is flagged to allow Ops to ignore the list.

	if (!(ent->client->pers.op_status && (int)sv_op_ignore_maplist->value))
	{
//		Read in list of bsp names from map list file.

		if ((mapstream = OpenMaplistFile(false)) != NULL)
		{
			while (!finished)
			{
				fnum = fscanf(mapstream, "%s", filename);
				if (fnum != EOF)
				{
					if ((filename[0] == '/') && (filename[1] == '/'))
						continue;

					maplist[nmaps] = strdup(filename);
					if (++nmaps == MAX_MAPS)
						finished = true;
				}
				else
					finished = true;
			}
		}
		else
		{
			gi.dprintf("** Failed to open list file for map vote (cmd)\n");
			return;
		}
		fclose(mapstream);

//		Traverse maplist, and see if the proposed map name is in it.

		for (i = 0; i < nmaps; ++i)
		{
			if (!Q_stricmp(maplist[i], bspname))
			{
				found = true;
				break;
			}
		}

		for (i = 0; i < nmaps; ++i)
			free(maplist[i]);

		if (!found)
		{
			gi_cprintf(ent, PRINT_HIGH, "Requested map is not in server's map list.\n");
			gi_cprintf(ent, PRINT_HIGH, "(For more info, use the 'maplist' command).\n");
			return;
		}
	}

	Com_sprintf(text, sizeof(text), "Proposal: change to level \"%s\"", bspname);

	if (ent->client->op_override)
	{
		strncpy(g_vote.vmsg, text, sizeof(g_vote.vmsg) - 1);
		strncpy(g_vote.vstring, bspname, sizeof(g_vote.vstring) - 1);
		g_vote.vote = VOTE_MAP;
		ent->client->op_override = false;
		gi_bprintf(PRINT_CHAT, "Action forced by Operator: %s\n", ent->client->pers.netname);
		G_VoteWin(true);
	}
	else
	{
		result = G_VoteBegin(ent, VOTE_MAP, text);
		if (result)
		{
			strncpy(g_vote.vstring, bspname, sizeof(g_vote.vstring) - 1);
			if (g_vote.nvotes >= g_vote.needvotes)
				G_VoteWin(false);
		}
	}
}

/*
==================
Cmd_Vote_AddBots

Handles "vote addbots <number>" command
==================
*/
void Cmd_Vote_AddBots(edict_t *ent, int numbots)
{
	char		text[VOTE_MSGLEN];
	qboolean	result;

	Com_sprintf(text, sizeof(text), "Proposal: add %d AwakenBots", numbots);

	if (ent->client->op_override)
	{
		strncpy(g_vote.vmsg, text, sizeof(g_vote.vmsg) - 1);
		g_vote.vnum = numbots;
		g_vote.vote = VOTE_ADDBOTS;
		ent->client->op_override = false;
		gi_bprintf(PRINT_CHAT, "Action forced by Operator: %s\n", ent->client->pers.netname);
		G_VoteWin(true);
	}
	else
	{
		result = G_VoteBegin(ent, VOTE_ADDBOTS, text);
		if (result)
		{
			g_vote.vnum = numbots;
			if (g_vote.nvotes >= g_vote.needvotes)
				G_VoteWin(false);
		}
	}
}

/*
==================
Cmd_Vote_RemoveBots

Handles "vote clearbots <number>" command
==================
*/
void Cmd_Vote_RemoveBots(edict_t *ent, int numbots)
{
	char		text[VOTE_MSGLEN];
	qboolean	result;

	Com_sprintf(text, sizeof(text), "Proposal: remove %d AwakenBots", numbots);

	if (ent->client->op_override)
	{
		strncpy(g_vote.vmsg, text, sizeof(g_vote.vmsg) - 1);
		g_vote.vnum = numbots;
		g_vote.vote = VOTE_REMOVEBOTS;
		ent->client->op_override = false;
		gi_bprintf(PRINT_CHAT, "Action forced by Operator: %s\n", ent->client->pers.netname);
		G_VoteWin(true);
	}
	else
	{
		result = G_VoteBegin(ent, VOTE_REMOVEBOTS, text);
		if (result)
		{
			g_vote.vnum = numbots;
			if (g_vote.nvotes >= g_vote.needvotes)
				G_VoteWin(false);
		}
	}
}

/*
==================
Cmd_Vote_RemoveAllBots

Handles "vote clearbots" command
==================
*/
void Cmd_Vote_RemoveAllBots(edict_t *ent)
{
	char		text[VOTE_MSGLEN];
	qboolean	result;

	Com_sprintf(text, sizeof(text), "Proposal: remove ALL AwakenBots");

	if (ent->client->op_override)
	{
		strncpy(g_vote.vmsg, text, sizeof(g_vote.vmsg) - 1);
		g_vote.vote = VOTE_REMOVEALLBOTS;
		ent->client->op_override = false;
		gi_bprintf(PRINT_CHAT, "Action forced by Operator: %s\n", ent->client->pers.netname);
		G_VoteWin(true);
	}
	else
	{
		result = G_VoteBegin(ent, VOTE_REMOVEALLBOTS, text);
		if (result)
		{
			if (g_vote.nvotes >= g_vote.needvotes)
				G_VoteWin(false);
		}
	}
}

/*
==================
Cmd_Vote_f

Handles vote <type> <arg> commands
==================
*/
void Cmd_Vote_f(edict_t *ent, qboolean auto_win)
{
	int	numbots = 0;

//	Validity checks.

	if (auto_win)
	{
		if (!ent->client->pers.op_status)
		{
			gi_cprintf(ent, PRINT_HIGH, "You are not an Operator.\n");
			return;
		}

		ent->client->op_override = true;
	}
	else
	{
		ent->client->op_override = false;
		if (((int)sv_gametype->value > G_FFA) && (teamgame.match >= MATCH_PREGAME))
		{
			gi_cprintf(ent, PRINT_HIGH, "Can't start a vote during a match.\n");
			return;
		}
	}

	if (gi.argc() < 2)
	{
		GMenu_VoteSettings(ent, NULL);
		return;
	}

//	Map voting.

	if (!Q_stricmp(gi.argv(1), "map"))
	{
		if (strlen(gi.argv(2)) == 0)
		{
			gi_cprintf(ent, PRINT_HIGH, "Usage:  vote map <bspname>\n");
			return;
		}

		if (!Q_stricmp(gi.argv(2), level.mapname))
		{
			gi_cprintf(ent, PRINT_HIGH, "Requested map is the current one!\n");
			return;
		}

		Cmd_Vote_Map(ent, gi.argv(2));
	}

//	AwakenBot voting.

	else if (!Q_stricmp(gi.argv(1), "addbots"))
	{
		if (!(int)sv_allow_bots->value)
		{
			gi_cprintf(ent, PRINT_HIGH, "AwakenBots are not enabled on this server.\n");
			return;
		}

		if (((int)sv_gametype->value == G_CTF) || ((int)sv_gametype->value == G_ASLT))
		{
			gi_cprintf(ent, PRINT_HIGH, "AwakenBots are not available for this gametype.\n");
			return;
		}

		if ((int)chedit->value)
		{
			gi_cprintf(ent, PRINT_HIGH, "Can't add bots during routing process.");
			return;
		}

		if (strlen(gi.argv(2)) == 0)
		{
			gi_cprintf(ent, PRINT_HIGH, "Usage:  vote addbots <number>\n");
			return;
		}

		if (NumBotsInGame >= MAXBOTS)
		{
			gi_cprintf(ent, PRINT_HIGH, "There are no bots left to add!\n");
			return;
		}

		if (g_reserve_used + g_public_used >= game.maxclients)
		{
			gi_cprintf(ent, PRINT_HIGH, "There are no free client spaces left!\n");
			return;
		}

		numbots = atoi(gi.argv(2));
		if (numbots > 0)
			Cmd_Vote_AddBots(ent, numbots);
		else
			gi_cprintf(ent, PRINT_HIGH, "Invalid number of bots!\n");
	}
	else if (!Q_stricmp(gi.argv(1), "clearbots"))
	{
		if (dedicated->value)
		{
			gi_cprintf(ent, PRINT_HIGH, "This command cannot be used on a dedicated server.\n");
			return;
		}

		if (!(int)sv_allow_bots->value)
			return;

		if ((int)chedit->value)
			return;

		if (((int)sv_gametype->value == G_CTF) || ((int)sv_gametype->value == G_ASLT))
			return;

		if (NumBotsInGame == 0)
		{
			gi_cprintf(ent, PRINT_HIGH, "There are no bots to remove!\n");
			return;
		}

		if (strlen(gi.argv(2)) == 0)
			Cmd_Vote_RemoveAllBots(ent);
		else
		{
			numbots = atoi(gi.argv(2));
			if (numbots > 0)
				Cmd_Vote_RemoveBots(ent, numbots);
			else
				gi_cprintf(ent, PRINT_HIGH, "Invalid number of bots!\n");
		}
	}
}

/*
==================
Cmd_TrapID_f

Toggles the player's Trap/C4 ID state.
==================
*/
void Cmd_TrapID_f(edict_t *ent)
{
	if (ent->client->resp.id_trap)
	{
		gi_cprintf(ent, PRINT_HIGH, "Disabling Trap/C4 identification.\n");
		ent->client->resp.id_trap = false;
	}
	else
	{
		gi_cprintf(ent, PRINT_HIGH, "Activating Trap/C4 identification.\n");
		ent->client->resp.id_trap = true;
	}
}

/*
==================
Cmd_WeaponNote_f

Sets the player's weapon selection notification preference.
0: no voice sounds or text
1: text only (for weapons with secondary modes)
2: text, plus voice for weapons with secondary modes
3: no text; voice for weapons with secondary modes
4: no text; voice for all weapons
==================
*/
void Cmd_WeaponNote_f(edict_t *ent)
{
	int	mode;
	int ret_mode = 0;

	if (gi.argc() < 2)
	{
		switch (ent->client->pers.weap_note)
		{
			case 0:
				ret_mode = 0;
				break;

			case 1:
				ret_mode = 1;
				break;

			case 2:
				ret_mode = 3;
				break;

			case 3:
				ret_mode = 2;
				break;

			case 6:
				ret_mode = 4;
				break;

			default:
				ret_mode = ent->client->pers.weap_note;
				break;
		}

		gi_cprintf(ent, PRINT_HIGH, "\"weap_note\" is \"%d\"\n", ret_mode);
		return;
	}

	mode = atoi(gi.argv(1));
	switch (mode)
	{
		case 0:
			ent->client->pers.weap_note = 0;
			break;

		case 1:
			ent->client->pers.weap_note = WN_TEXT;
			break;

		case 2:
			ent->client->pers.weap_note = WN_TEXT | WN_VSEC;
			break;

		case 3:
			ent->client->pers.weap_note = WN_VSEC;
			break;

		case 4:
			ent->client->pers.weap_note = WN_VALL | WN_VSEC;
			break;

		case 5:
			ent->client->pers.weap_note = WN_TEXT | WN_VALL | WN_VSEC;
			break;

		default:
			ent->client->pers.weap_note = 0;
			break;
	}
}

/*
==================
Cmd_VoteYes_f
==================
*/
void Cmd_VoteYes_f(edict_t *ent, qboolean op_forced)
{

//	Check to see if command is valid.

	if (op_forced && !ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are not an Operator.\n");
		return;
	}

	if (g_vote.vote == VOTE_NONE)
	{
		gi_cprintf(ent, PRINT_HIGH, "No vote is in progress.\n");
		return;
	}

	if (ent->client->resp.voted)
	{
		gi_cprintf(ent, PRINT_HIGH, "You already voted.\n");
		return;
	}

//	Add player's vote to total, and see if this is enough to win the vote.
//	If not, inform players of the vote status. NB: Operators can force a vote through if they wish.

	if (op_forced)
	{
		gi_bprintf(PRINT_CHAT, "Vote forced YES by Operator: %s\n", ent->client->pers.netname);
		G_VoteWin(true);
		return;
	}
	else
	{
		ent->client->resp.voted = true;
		gi_bprintf(PRINT_HIGH, "%s voted YES\n", ent->client->pers.netname);
		if (++g_vote.nvotes >= g_vote.needvotes)
		{
			G_VoteWin(false);
			return;
		}

		gi_bprintf(PRINT_CHAT, "Votes needed: %d  Time left: %ds\n", g_vote.needvotes - g_vote.nvotes, (int)(g_vote.votetime - level.time));
	}
}

/*
==================
Cmd_VoteNo_f
==================
*/
void Cmd_VoteNo_f(edict_t *ent, qboolean op_forced)
{

//	Check to see if command is valid.

	if (op_forced && !ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are not an Operator.\n");
		return;
	}

	if (g_vote.vote == VOTE_NONE)
	{
		gi_cprintf(ent, PRINT_HIGH, "No vote is in progress.\n");
		return;
	}

	if (ent->client->resp.voted)
	{
		gi_cprintf(ent, PRINT_HIGH, "You already voted.\n");
		return;
	}

//	Inform players of the vote status. NB: Operators can block a vote if they wish.

	if (op_forced)
	{
		gi_bprintf(PRINT_CHAT, "Vote forced NO by Operator: %s\n", ent->client->pers.netname);
		g_vote.vote = VOTE_NONE;
		g_vote.votetime = 0.0;
	}
	else
	{
		ent->client->resp.voted = true;
		gi_bprintf(PRINT_HIGH, "%s voted NO\n", ent->client->pers.netname);
		gi_bprintf(PRINT_CHAT, "Votes needed: %d  Time left: %ds\n", g_vote.needvotes - g_vote.nvotes, (int)(g_vote.votetime - level.time));
	}
}

/*
==================
Cmd_Mission_f

Displays mission objectives for an Assault map.
==================
*/
void Cmd_Mission_f(edict_t *self)
{
	if (sv_gametype->value != G_ASLT)
		return;

	ASLTShowMission(self);
}

/*
==================
Cmd_PlayVoice_f

Plays specified sound file on client's voice channel.
==================
*/
void Cmd_PlayVoice_f(edict_t *ent)
{
	char soundfile[MAX_QPATH];

	if (ent->client->spectator)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are an observer.\n");
		return;
	}

	if (ent->client->pers.muted)
	{
		gi_cprintf(ent, PRINT_HIGH, "You have been muted.\n");
		return;
	}

	if (gi.argc() < 2)
	{
		gi_cprintf(ent, PRINT_HIGH, "Usage:  play_voice <wav file>\n");
		return;
	}

	if (CheckFlood(ent))
		return;

//	Safety check for string length.																	//r1,CW

	if (strlen(gi.argv(1)) > MAX_QPATH - 5)
	{
		gi_cprintf(ent, PRINT_HIGH, "Filename is too long.\n");
		return;
	}

	Com_sprintf(soundfile, sizeof(soundfile), "%s.wav", gi.argv(1));
	gi.sound(ent, CHAN_VOICE, gi.soundindex(soundfile), 1, ATTN_NORM, 0);
}

/*
==================
Cmd_PlayTeam_f

Plays specified sound file to a client's team-mates.
==================
*/
void Cmd_PlayTeam_f(edict_t *ent)
{
	edict_t	*e;
	char	soundfile[MAX_QPATH];
	int		i;

	if (sv_gametype->value == G_FFA)
	{
		gi_cprintf(ent, PRINT_HIGH, "The play_team command is not available in DM.\n");
		return;
	}

	if (ent->client->resp.ctf_team < CTF_TEAM1)
	{
		gi_cprintf(ent, PRINT_HIGH, "play_team is invalid if you aren't on a team.\n");
		return;
	}

	if (ent->client->pers.muted)
	{
		gi_cprintf(ent, PRINT_HIGH, "You have been muted.\n");
		return;
	}

	if (gi.argc() < 2)
	{
		gi_cprintf(ent, PRINT_HIGH, "Usage:  play_team <wav file>\n");
		return;
	}

	if (CheckFlood(ent))
		return;

//	Safety check for string length.																	//r1,CW

	if (strlen(gi.argv(1)) > MAX_QPATH - 5)
	{
		gi_cprintf(ent, PRINT_HIGH, "Filename is too long.\n");
		return;
	}

	Com_sprintf(soundfile, sizeof(soundfile), "%s.wav", gi.argv(1));
	for (i = 1; i <= (int)maxclients->value; ++i)
	{
		e = g_edicts + i;
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;

		if (ent->client->resp.ctf_team == e->client->resp.ctf_team)
			unicastSound(e, gi.soundindex(soundfile), 1.0);											//r1,CW
	}
}

/*
==================
Cmd_AGMPull_f

Shortens the AG Manipulator beam.
==================
*/
void Cmd_AGMPull_f(edict_t *ent)
{
	if (ent->client->agm_target == NULL)
	{
		gi_centerprintf(ent, "No AG Manipulator target to pull\n");
		return;
	}

	ent->client->agm_pull = true;
	ent->client->agm_push = false;
}

/*
==================
Cmd_AGMUnpull_f

Stops shortening the AG Manipulator beam.
==================
*/
void Cmd_AGMUnpull_f(edict_t *ent)
{
	ent->client->agm_pull = false;
}

/*
==================
Cmd_AGMPush_f

Lengthens the AG Manipulator beam.
==================
*/
void Cmd_AGMPush_f(edict_t *ent)
{
	if (ent->client->agm_target == NULL)
	{
		gi_centerprintf(ent, "No AG Manipulator target to push\n");
		return;
	}

	ent->client->agm_pull = false;
	ent->client->agm_push = true;
}

/*
==================
Cmd_AGMUnpush_f

Stops lengthening the AG Manipulator beam.
==================
*/
void Cmd_AGMUnpush_f(edict_t *ent)
{
	ent->client->agm_push = false;
}

/*
==================
Cmd_AGMFling_f

Flings the player held by the AG Manipulator beam.
==================
*/
void Cmd_AGMFling_f(edict_t *ent)
{
	static gitem_t *tech = NULL;
	edict_t	*targ;
	vec3_t	forward;

//	Sanity check.

	if (ent->client->agm_target == NULL)
	{
		gi_centerprintf(ent, "No AG Manipulator target to fling\n");
		return;
	}

	targ = ent->client->agm_target;

//	Scale the target's velocity vector along our viewing angle. The old velocity needs to be
//	set to the same value, otherwise flinging the victim close to a wall won't necessarily
//	register a sufficient delta-v.

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, sv_agm_fling_power->value, targ->velocity);
	VectorCopy(targ->velocity, targ->client->oldvelocity);
	gi.linkentity(targ);

	targ->client->flung_by_agm = true;
	targ->client->held_by_agm = false;
	targ->client->thrown_by_agm = false;

//	Reset our AGM stuff.

	ent->client->agm_target = NULL;
	ent->client->agm_on = false;
	ent->client->agm_push = false;
	ent->client->agm_pull = false;
	ent->client->agm_charge = 0;
	ent->client->agm_tripped = true;

//	Play Quad or Tech sounds, if appropriate.

	if (!tech)
		tech = FindItemByClassname("item_tech2");

	if (tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)])
	{
		if (ent->client->ctf_techsndtime < level.time)
		{
			ent->client->ctf_techsndtime = level.time + 1.0;
			if (ent->client->quad_framenum > level.framenum)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2x.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2.wav"), 1, ATTN_NORM, 0);
		}
	}
	else if (ent->client->quad_framenum > level.framenum)
		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
}

/*
==================
Cmd_MapList_f

Prints out the bsp names from the maplist file.
==================
*/
void Cmd_MapList_f(edict_t *ent)
{
	FILE	*mapstream;
	char	bspname[MAX_QPATH];
	char	linebuf[80];
	char	text[1024];
	size_t	textLen;

	if ((mapstream = OpenMaplistFile(false)) != NULL)
	{
		*text = 0;
		while (fgets(bspname, MAX_QPATH, mapstream) != NULL)
		{
			if ((bspname[0] == '/') && (bspname[1] == '/'))
				continue;

			Com_sprintf(linebuf, sizeof(linebuf), "%s", bspname);

			if (strlen(text) + strlen(linebuf) > sizeof(text) - 50)
			{
			//	sprintf(text+strlen(text), "And more...\n");
				textLen = strlen(text);
				Com_sprintf(text+textLen, sizeof(text)-textLen, "And more...\n");
				gi_cprintf(ent, PRINT_HIGH, "%s", text);
				return;
			}
			Com_strcat(text, sizeof(text), linebuf);
		}
		gi_cprintf(ent, PRINT_HIGH, "%s", text);
	}
	else
	{
		gi_cprintf(ent, PRINT_HIGH, "Couldn't open map list \"%s\".\n", sv_map_file->string);
		return;
	}

	fclose(mapstream);
}

/*
==================
Cmd_DevEnts_f

Toggles display of entity information for a client.
==================
*/
void Cmd_DevEnts_f(edict_t *ent)
{
	int i = 0;

	if (gi.argc() < 2)
	{
		gi_cprintf(ent, PRINT_HIGH, "dev_ents %d\n", (ent->show_hostile)?1:0);
		return;
	}

	i = atoi(gi.argv(1));
	if (i > 0)
		ent->show_hostile = true;				// abused this flag to save increasing size of edict_t
	else
		ent->show_hostile = false;
}

/*
==================
Cmd_ShowBotRoute_f

Draws a line of dots along the bot route for the map.
==================
*/

#define PLOTROUTE_SUBSIZE 16		// too big => overflows

void Plot_Route(edict_t *self)
{
	vec3_t	start;
	vec3_t	end;
	int		i;

	self->nextthink = level.time + FRAMETIME;

	for (i = self->count; i < self->count + PLOTROUTE_SUBSIZE; i++)
	{
//		Remove the plotter if all the nodes have been plotted.

		if (i == TotalRouteNodes)
		{
			self->think = G_FreeEdict;
			return;
		}

//		Plot a dotted line between two adjacent nodes.

		VectorCopy(Route[i].Pt, end);
		if (i == 0)
			VectorCopy(Route[TotalRouteNodes-1].Pt, start);
		else
			VectorCopy(Route[i-1].Pt, start);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_DEBUGTRAIL);
		gi.WritePosition(start);
		gi.WritePosition(end);
		gi.multicast(Route[i].Pt, MULTICAST_ALL);
	}

	self->count += PLOTROUTE_SUBSIZE;
}

void Cmd_ShowBotRoute_f(edict_t *ent)
{
	edict_t	*plotter = NULL;
	
//	Sanity checks.

	if (dedicated->value)
	{
		gi_cprintf(ent, PRINT_HIGH, "This command cannot be used on a dedicated server.\n");
		return;
	}

	if (TotalRouteNodes == 0)
	{
		gi_cprintf(ent, PRINT_HIGH, "No bot route file loaded.\n");
		return;
	}

//	Create the plotter if it doesn't currently exist (its counter is reset
//	if it does).

	if ((plotter = G_Find(NULL, FOFS(classname), "route_plotter")) == NULL)
	{
		plotter = G_Spawn();
		plotter->classname = "route_plotter";
		plotter->svflags |= SVF_NOCLIENT;
		plotter->think = Plot_Route;
		plotter->nextthink = level.time + FRAMETIME;
	}
	
	plotter->count = 0;
}

/*
==================
Cmd_SayPrivs_f

(Un)mutes the specified player.
==================
*/
void Cmd_SayPrivs_f(edict_t *ent, qboolean mute)
{
	char *strbuf;

	if (!ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are not an Operator.\n");
		return;
	}

	if (gi.argc() < 2)
	{
		gi_cprintf(ent, PRINT_HIGH, "Usage:  %s <player_num>\n", (mute)?"op_mute":"op_unmute");
		return;
	}

	if (gi.argv(2) && *gi.argv(2))
		strbuf = gi.argv(2);
	else
		strbuf = "";

	Op_SayPrivs(ent, mute, atoi(gi.argv(1)), strbuf);
}

void Op_SayPrivs(edict_t *ent, qboolean mute, int pnum, char *reason)
{
	edict_t	*targ;

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

	if (targ == ent)
	{
		gi_cprintf(ent, PRINT_HIGH, "You cannot %s yourself!\n", (mute)?"mute":"unmute");
		return;
	}

	if (mute && targ->client->pers.muted)
	{
		gi_cprintf(ent, PRINT_HIGH, "That player is already muted.\n");
		return;
	}

	if (!mute && !targ->client->pers.muted)
	{
		gi_cprintf(ent, PRINT_HIGH, "That player is already unmuted.\n");
		return;
	}

	gi_bprintf(PRINT_CHAT, "%s was %s by %s\n", targ->client->pers.netname, (mute)?"muted":"unmuted", ent->client->pers.netname);
	if (strlen(reason) > 0)
		gi_bprintf(PRINT_CHAT, "REASON: %s\n", reason);

	targ->client->pers.muted = (mute)?true:false;
}

/*
==================
Cmd_SwapTeam_f

Moves the specified player to the other team.
==================
*/
void Op_SwapTeam(edict_t *ent, int pnum)
{
	edict_t		*targ;
	qboolean	swapped = false;

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

	if ((targ->client->resp.ctf_team != CTF_TEAM1) && (targ->client->resp.ctf_team != CTF_TEAM2))
	{
		gi_cprintf(ent, PRINT_HIGH, "That player is not on a team.\n");
		return;
	}

	if (targ->client->resp.ctf_team == CTF_TEAM1)
		swapped = JoinTeam(targ, CTF_TEAM2, true);
	else
		swapped = JoinTeam(targ, CTF_TEAM1, true);

	if (swapped)
		gi_bprintf(PRINT_CHAT, "%s was swapped to the %s team by %s\n", targ->client->pers.netname, CTFTeamName(targ->client->resp.ctf_team), ent->client->pers.netname);
}

void Cmd_SwapTeam_f(edict_t *ent)
{
	if (!ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are not an Operator.\n");
		return;
	}

	if ((int)sv_gametype->value < G_CTF)
	{
		gi_cprintf(ent, PRINT_HIGH, "There are no teams to swap between.\n");
		return;
	}

	if (gi.argc() < 2)
	{
		gi_cprintf(ent, PRINT_HIGH, "Usage:  op_swapteam <player_num>\n");
		return;
	}

	Op_SwapTeam(ent, atoi(gi.argv(1)));
}


/*
==================
Cmd_OpIPList_f

Displays a list of player names and IP addresses.
==================
*/
void Cmd_OpIPList_f(edict_t *ent)
{
	edict_t	*player;
	char	linebuf[80];
	char	text[1024];
	int		i;
	size_t	textLen;

	if (!ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are not an Operator.\n");
		return;
	}

//	Display number, name and IP address.

	*text = 0;
	for (i = 1; i <= (int)maxclients->value; i++)
	{
		player = g_edicts + i;
		if (!player->inuse)
			continue;

		Com_sprintf(linebuf, sizeof(linebuf), "%3d %-16.16s %-21.21s %s\n", i, player->client->pers.netname, player->client->pers.ip, (player->client->pers.op_status)?"(OP)":"");

		if (strlen(text) + strlen(linebuf) > sizeof(text) - 50)
		{
		//	sprintf(text+strlen(text), "And more...\n");
			textLen = strlen(text);
			Com_sprintf(text+textLen, sizeof(text)-textLen, "And more...\n");
			gi_cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		Com_strcat(text, sizeof(text), linebuf);
	}
	gi_cprintf(ent, PRINT_HIGH, "%s", text);
}

/*
==================
Cmd_OpAddBots_f

Adds the specified number of bots to the server.
==================
*/
void Cmd_OpAddBots_f(edict_t *ent)
{
	edict_t *remover = NULL;
	int		i = 0;

	if (gi.argc() < 2)
	{
		gi_cprintf(ent, PRINT_HIGH, "Usage:  op_addbots <number>\n");
		return;
	}

	if (!ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are not an Operator.\n");
		return;
	}

	if (!(int)sv_allow_bots->value)
	{
		gi_cprintf(ent, PRINT_HIGH, "AwakenBots are not enabled on this server.\n");
		return;
	}

	if (((int)sv_gametype->value == G_CTF) || ((int)sv_gametype->value == G_ASLT))
	{
		gi_cprintf(ent, PRINT_HIGH, "AwakenBots are not available for this gametype.\n");
		return;
	}

	if ((int)chedit->value)
	{
		gi_cprintf(ent, PRINT_HIGH, "Can't add bots during route creation.");
		return;
	}

	if (teamgame.match > MATCH_SETUP)
	{
		gi_cprintf(ent, PRINT_HIGH, "Cannot add bots during a match.\n");
		return;
	}

	if ((remover = G_Find(NULL, FOFS(classname), "bot_remover")) != NULL)
	{
		gi_cprintf(ent, PRINT_HIGH, "Cannot add bots - some are still being removed.\n");
		return;
	}

	i = atoi(gi.argv(1));
	if (i < 1)
	{
		gi_cprintf(ent, PRINT_HIGH, "Invalid number of bots.\n");
		return;
	}

	SpawnNumBots_Safe(i);
	gi.dprintf("%d bots added by %s\n", i, ent->client->pers.netname);
}

/*
==================
Cmd_AddBots_f

Adds the specified number of bots to a listen server.
==================
*/
void Cmd_AddBots_f(edict_t *ent)
{
	edict_t *remover = NULL;
	int		i = 0;

	if (gi.argc() < 2)
	{
		gi_cprintf(ent, PRINT_HIGH, "Usage:  addbots <number>\n");
		return;
	}

	if (dedicated->value)
	{
		gi_cprintf(ent, PRINT_HIGH, "This command cannot be used on a dedicated server.\n");
		return;
	}

	if (!(int)sv_allow_bots->value)
	{
		gi_cprintf(ent, PRINT_HIGH, "AwakenBots are not enabled on this server.\n");
		return;
	}

	if (((int)sv_gametype->value == G_CTF) || ((int)sv_gametype->value == G_ASLT))
	{
		gi_cprintf(ent, PRINT_HIGH, "AwakenBots are not available for this gametype.\n");
		return;
	}

	if ((int)chedit->value)
	{
		gi_cprintf(ent, PRINT_HIGH, "Can't add bots during route creation.");
		return;
	}

	if (teamgame.match > MATCH_SETUP)
	{
		gi_cprintf(ent, PRINT_HIGH, "Cannot add bots during a match.\n");
		return;
	}

	if ((remover = G_Find(NULL, FOFS(classname), "bot_remover")) != NULL)
	{
		gi_cprintf(ent, PRINT_HIGH, "Cannot add bots - some are still being removed.\n");
		return;
	}

	i = atoi(gi.argv(1));
	if (i < 1)
	{
		gi_cprintf(ent, PRINT_HIGH, "Invalid number of bots.\n");
		return;
	}

	SpawnNumBots_Safe(i);
}

/*
==================
Cmd_OpClearBots_f

Removes all bots from the server.
==================
*/
void Cmd_OpClearBots_f(edict_t *ent)
{
	edict_t	*spawner = NULL;

	if (!ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are not an Operator.\n");
		return;
	}

	if (!(int)sv_allow_bots->value)
		return;

	if ((int)chedit->value)
		return;

	if (((int)sv_gametype->value == G_CTF) || ((int)sv_gametype->value == G_ASLT))
		return;

	if (teamgame.match > MATCH_SETUP)
	{
		gi_cprintf(ent, PRINT_HIGH, "Cannot remove bots during a match.\n");
		return;
	}

	if ((spawner = G_Find(NULL, FOFS(classname), "bot_spawner")) != NULL)
	{
		gi_cprintf(ent, PRINT_HIGH, "Cannot clear bots - some are still being added.\n");
		return;
	}

	if (gi.argc() < 2)
		RemoveNumBots_Safe(MAXBOTS);
	else
		RemoveNumBots_Safe(atoi(gi.argv(1)));
}

/*
==================
Cmd_ClearBots_f

Removes all bots from a listen server.
==================
*/
void Cmd_ClearBots_f(edict_t *ent)
{
	edict_t	*spawner = NULL;

	if (dedicated->value)
	{
		gi_cprintf(ent, PRINT_HIGH, "This command cannot be used on a dedicated server.\n");
		return;
	}

	if (!(int)sv_allow_bots->value)
		return;

	if ((int)chedit->value)
		return;

	if (((int)sv_gametype->value == G_CTF) || ((int)sv_gametype->value == G_ASLT))
		return;

	if (teamgame.match > MATCH_SETUP)
	{
		gi_cprintf(ent, PRINT_HIGH, "Cannot remove bots during a match.\n");
		return;
	}

	if ((spawner = G_Find(NULL, FOFS(classname), "bot_spawner")) != NULL)
	{
		gi_cprintf(ent, PRINT_HIGH, "Cannot clear bots - some are still being added.\n");
		return;
	}

	if (gi.argc() < 2)
		RemoveNumBots_Safe(MAXBOTS);
	else
		RemoveNumBots_Safe(atoi(gi.argv(1)));
}

/*
==================
Cmd_DropAGM_f

Writes bspname and coords to the AGM drop file.
==================
*/
void Cmd_DropAGM_f(edict_t *ent)
{
	FILE	*entstream;
	float	x = 0.0;
	float	y = 0.0;
	float	z = 0.0;
	int		nbytes;

	if (!ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You do not have Operator status!\n");
		return;
	}

	if ((entstream = OpenAGMDropFile(false, false)) != NULL)
	{
		if (gi.argc() < 2)
		{
			x = ent->s.origin[0];
			y = ent->s.origin[1];
			z = ent->s.origin[2];
			gi_cprintf(ent, PRINT_HIGH, "Setting AGM drop point at %s.\n", vtos(ent->s.origin));
		}
		else
		{
			if (gi.argv(1))
				x = atoi(gi.argv(1));
			if (gi.argv(2))
				y = atoi(gi.argv(2));
			if (gi.argv(3))
				z = atoi(gi.argv(3));

			gi_cprintf(ent, PRINT_HIGH, "Setting AGM drop point at (%d %d %d).\n", (int)x, (int)y, (int)z);
		}

		if ((nbytes = fprintf(entstream, "\n%s %d %d %d", level.mapname, (int)x, (int)y, (int)z)) == 0)
			gi_cprintf(ent, PRINT_HIGH, "Error writing to AGM drop file.\n");

		fclose(entstream);
	}
	else
		gi_cprintf(ent, PRINT_HIGH, "Could not open the AGM drop file.\n");
}


/*
==================
Cmd_DropDL_f

Writes bspname and coords to the Disc Launcher drop file.
==================
*/
void Cmd_DropDL_f(edict_t *ent)
{
	FILE	*entstream;
	float	x = 0.0;
	float	y = 0.0;
	float	z = 0.0;
	int		nbytes;

	if (!ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You do not have Operator status!\n");
		return;
	}

	if ((entstream = OpenDiscLauncherDropFile(false, false)) != NULL)
	{
		if (gi.argc() < 2)
		{
			x = ent->s.origin[0];
			y = ent->s.origin[1];
			z = ent->s.origin[2];
			gi_cprintf(ent, PRINT_HIGH, "Setting Disc Launcher drop point at %s.\n", vtos(ent->s.origin));
		}
		else
		{
			if (gi.argv(1))
				x = atoi(gi.argv(1));
			if (gi.argv(2))
				y = atoi(gi.argv(2));
			if (gi.argv(3))
				z = atoi(gi.argv(3));

			gi_cprintf(ent, PRINT_HIGH, "Setting Disc Launcher drop point at (%d %d %d).\n", (int)x, (int)y, (int)z);
		}

		if ((nbytes = fprintf(entstream, "\n%s %d %d %d", level.mapname, (int)x, (int)y, (int)z)) == 0)
			gi_cprintf(ent, PRINT_HIGH, "Error writing to Disc Launcher drop file.\n");

		fclose(entstream);
	}
	else
		gi_cprintf(ent, PRINT_HIGH, "Could not open the Disc Launcher drop file.\n");
}


/*
=================
Cmd_GiveOp_f

Allows a player to give themselves Operator status,
if they know the password.
==================
*/
void Cmd_GiveOp_f(edict_t *ent)
{
	gi.dprintf("INFO: %s requested Op status...\n", ent->client->pers.netname);

	if (ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You already have OP status.\n");
		gi.dprintf("INFO: ...but already has OP status.\n");
		return;
	}

	if (strlen(sv_op_password->string) == 0)
	{
		gi_cprintf(ent, PRINT_HIGH, "Self-granting of Operator status is disabled.\n");
		gi.dprintf("INFO: ...but auto-opping was disabled.\n");
		return;
	}

	if (gi.argc() < 2)
	{
		gi_cprintf(ent, PRINT_HIGH, "You must specify a password.\n");
		gi.dprintf("INFO: ...but a password wasn't specified.\n");
		return;
	}
	
	if (strcmp(gi.argv(1), sv_op_password->string))
	{
		gi_cprintf(ent, PRINT_HIGH, "Operator status denied.\n");
		gi.dprintf("INFO: ...but the password was wrong.\n");
		return;
	}

	ent->client->pers.op_status = true;
	gi_bprintf(PRINT_CHAT, "OP status given to %s.\n", ent->client->pers.netname);
}

/*
==================
Cmd_OpMenu_f

Displays menu for Operator actions.
==================
*/
void Cmd_OpMenu_f(edict_t *ent)
{
	if (!ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You do not have Operator status!\n");
		return;
	}

	OpenOpMenu(ent);
}

/*
==================
Cmd_MatchStart_f

Forces a match to start (if one is in progress).
==================
*/
void Cmd_MatchStart_f(edict_t *ent)
{
	edict_t	*cl_ent;
	int		i;

	if (!ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You do not have Operator status!\n");
		return;
	}

//	Check for validity of command.

	if (teamgame.match != MATCH_SETUP)
	{
		gi_cprintf(ent, PRINT_HIGH, "A match is not being setup.\n");
		return;
	}

//	Force clients who haven't selected a team into Observer mode. Everyone else is considered
//	to be ready.

	gi_bprintf(PRINT_CHAT, "%s forced the match to start.\n", ent->client->pers.netname);
	for (i = 1; i <= (int)maxclients->value; i++)
	{
		cl_ent = &g_edicts[i];
		if (!cl_ent->inuse)
			continue;
		if (!cl_ent->client)
			continue;
		if (cl_ent->client->resp.ready)
			continue;

		if (cl_ent->client->resp.ctf_team == CTF_NOTEAM)
			CTFObserver(cl_ent);
	}

//	Start the match.

	teamgame.match = MATCH_PREGAME;
	teamgame.matchtime = level.time + matchstarttime->value;
	teamgame.countdown = false;
	gi.positioned_sound(world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/talk1.wav"), 1, ATTN_NONE, 0);
}

/*
==================
Cmd_ListSlots_f

Lists which public and reserved slots
are taken, and by whom.
==================
*/
void Cmd_ListSlots_f(edict_t *ent)
{
	char	linebuf[80];
	char	text[1024];
	int		i;
	size_t	textLen;

	if (!ent->client->pers.op_status)
	{
		gi_cprintf(ent, PRINT_HIGH, "You do not have Operator status!\n");
		return;
	}

//	Display number, name and player slot status (reserved/public).

	*text = 0;
	for (i = 0; i < game.maxclients; ++i)
	{
		if (!g_slots[i])
			Com_sprintf(linebuf, sizeof(linebuf), "%2d (%s) = (unused)\n", i+1, ((i < (int)sv_reserved->value) && *sv_rsv_password->string)?"Rsv":"Pub");
		else
			Com_sprintf(linebuf, sizeof(linebuf), "%2d (%s) = %-16.16s\n", i+1, ((i < (int)sv_reserved->value) && *sv_rsv_password->string)?"Rsv":"Pub", g_slots[i]->client->pers.netname);

		if (strlen(text) + strlen(linebuf) > sizeof(text) - 50)
		{
		//	sprintf(text+strlen(text), "And more...\n");
			textLen = strlen(text);
			Com_sprintf(text+textLen, sizeof(text)-textLen, "And more...\n");
			gi_cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		Com_strcat(text, sizeof(text), linebuf);
	}

	gi_cprintf(ent, PRINT_HIGH, "%s", text);
}
//CW--

/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f(edict_t *ent)
{
	gitem_t	*it;
	char	*s;
	char	itName[128];	// Knightmare added
	int		index;

//CW++
//	Dead people that use certain items cause server crashes. This is bad, mmmkay?

	if (ent->health <= 0)
		return;
//CW--

	s = gi.args();

//CW++
//	Substitute weapon names so as not to break old-style config files.
	Com_strcpy(itName, sizeof(itName), s);

//	if (!Q_stricmp(s, "blaster"))
//		sprintf(s, "Desert Eagle");
	if (!Q_stricmp(itName, "blaster"))
		Com_sprintf(itName, sizeof(itName), "Desert Eagle");

//	if (!Q_stricmp(s, "shotgun"))
//		sprintf(s, "Gauss Pistol");
	if (!Q_stricmp(itName, "shotgun"))
		Com_sprintf(itName, sizeof(itName), "Gauss Pistol");

//	if (!Q_stricmp(s, "machinegun"))
//		sprintf(s, "Mac-10");
	if (!Q_stricmp(itName, "machinegun"))
		Com_sprintf(itName, sizeof(itName), "Mac-10");

//	if (!Q_stricmp(s, "super shotgun"))
//		sprintf(s, "Jackhammer");
	if (!Q_stricmp(itName, "super shotgun"))
		Com_sprintf(itName, sizeof(itName), "Jackhammer");

//	if (!Q_stricmp(s, "grenades"))
//		sprintf(s, "C4");
	if (!Q_stricmp(itName, "grenades"))
		Com_sprintf(itName, sizeof(itName), "C4");

//	if (!Q_stricmp(s, "grenade launcher"))
//		sprintf(s, "Traps");
	if (!Q_stricmp(itName, "grenade launcher"))
		Com_sprintf(itName, sizeof(itName), "Traps");

//	if (!Q_stricmp(s, "chaingun"))
//		sprintf(s, "E.S.G.");
	if (!Q_stricmp(itName, "chaingun"))
		Com_sprintf(itName, sizeof(itName), "E.S.G.");

//	if (!Q_stricmp(s, "hyperblaster"))
//		sprintf(s, "Flamethrower");
	if (!Q_stricmp(itName, "hyperblaster"))
		Com_sprintf(itName, sizeof(itName), "Flamethrower");

//	if (!Q_stricmp(s, "bfg10k"))
//		sprintf(s, "Shock Rifle");
	if (!Q_stricmp(itName, "bfg10k"))
		Com_sprintf(itName, sizeof(itName), "Shock Rifle");
//CW--

//	it = FindItem(s);
	it = FindItem(itName);
	if (!it)
	{
	//	gi_cprintf(ent, PRINT_HIGH, "unknown item: %s\n", s);
		gi_cprintf(ent, PRINT_HIGH, "unknown item: %s\n", itName);
		return;
	}
	if (!it->use)
	{
		gi_cprintf(ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);

//CW++
//	Detonate any C4 bundles the player has laid if that weapon is currently active for them.

	if ((ent->client->pers.weapon->weapmodel == WEAP_C4) && (it->weapmodel == WEAP_C4) && (ent->client->resp.nodes_active > 0))
		Cmd_Detonate_C4_f(ent);

//	Primary/secondary weapon mode toggles.

	if ((ent->client->pers.weapon->weapmodel == WEAP_GAUSSPISTOL) && (it->weapmodel == WEAP_GAUSSPISTOL))
		Cmd_Toggle_GP_f(ent);

	if ((ent->client->pers.weapon->weapmodel == WEAP_ESG) && (it->weapmodel == WEAP_ESG))
		Cmd_Toggle_ESG_f(ent);

	if ((ent->client->pers.weapon->weapmodel == WEAP_FLAMETHROWER) && (it->weapmodel == WEAP_FLAMETHROWER))
		Cmd_Toggle_FT_f(ent);

	if ((ent->client->pers.weapon->weapmodel == WEAP_ROCKETLAUNCHER) && (it->weapmodel == WEAP_ROCKETLAUNCHER))
		Cmd_Toggle_RL_f(ent);

	if ((ent->client->pers.weapon->weapmodel == WEAP_SHOCKRIFLE) && (it->weapmodel == WEAP_SHOCKRIFLE))
		Cmd_Toggle_SR_f(ent);

	if ((ent->client->pers.weapon->weapmodel == WEAP_AGM) && (it->weapmodel == WEAP_AGM))
		Cmd_Toggle_AGM_f(ent);
//CW--

	if (!ent->client->pers.inventory[index])
	{

//CW++
//	Detonate any C4 bundles the player has laid.

		if ((ent->client->resp.nodes_active > 0) && (it->weapmodel == WEAP_C4))
			Cmd_Detonate_C4_f(ent);
		else
//CW--
			gi_cprintf(ent, PRINT_HIGH, "Out of item: %s\n", s);

		return;
	}

	it->use(ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f(edict_t *ent)
{
	gitem_t		*it;
	char		*s;
	int			index;

//ZOID++
	// special case for tech powerups
	if ((Q_stricmp(gi.args(), "tech") == 0) && (it = CTFWhat_Tech(ent)) != NULL)
	{
		it->drop(ent, it);
		return;
	}
//ZOID--

	s = gi.args();
	it = FindItem(s);
	if (!it)
	{
		gi_cprintf(ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}

	if (!it->drop)
	{
		gi_cprintf(ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}

	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi_cprintf(ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop(ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f(edict_t *ent)
{
	gclient_t	*cl;
	int			i;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;

//ZOID++
	if (ent->client->menu)
	{
		PMenu_Close(ent);
		ent->client->update_chase = true;
		return;
	}
//ZOID--

	if (cl->showinventory)
	{
		cl->showinventory = false;

//CW++
		if (cl->show_gausstarget & 2)
			cl->show_gausstarget = 1;
//CW--

		return;
	}

//ZOID++
	if (cl->resp.ctf_team == CTF_NOTEAM)															//CW
	{
		CTFOpenJoinMenu(ent);
		return;
	}
//ZOID--

	cl->showinventory = true;

	gi.WriteByte(svc_inventory);
	for (i = 0; i < MAX_ITEMS; i++)
		gi.WriteShort(cl->pers.inventory[i]);

	gi.unicast(ent, true);

//CW++
	if (ent->client->show_gausstarget & 1)
		ent->client->show_gausstarget = 2;
//CW--
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f(edict_t *ent)
{
	gitem_t	*it;

//ZOID++
	if (ent->client->menu)
	{
		PMenu_Select(ent);
		return;
	}
//ZOID--

	ValidateSelectedItem(ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi_cprintf(ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		gi_cprintf(ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use(ent, it);
}

//ZOID++
/*
=================
Cmd_LastWeap_f
=================
*/
void Cmd_LastWeap_f(edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	cl->pers.lastweapon->use(ent, cl->pers.lastweapon);
}
//ZOID--

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f(edict_t *ent)
{
	gclient_t	*cl;
	gitem_t		*it;
	int			selected_weapon;
	int			index;
	int			i;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i = 1; i <= MAX_ITEMS; i++)
	{
		index = (selected_weapon + i) % MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;

		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & IT_WEAPON))
			continue;

		it->use(ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f(edict_t *ent)
{
	gclient_t	*cl;
	gitem_t		*it;
	int			selected_weapon;
	int			index;
	int			i;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan for the next valid one
	for (i = 1; i <= MAX_ITEMS; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i) % MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;

		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & IT_WEAPON))
			continue;

		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f(edict_t *ent)
{
	gclient_t	*cl;
	gitem_t		*it;
	int			index;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;

	it = &itemlist[index];
	if (!it->use)
		return;
	if (!(it->flags & IT_WEAPON))
		return;

	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f(edict_t *ent)
{
	gitem_t	*it;

	ValidateSelectedItem(ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi_cprintf(ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		gi_cprintf(ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop(ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f(edict_t *ent)
{
//CW++
	if (ent->client->spectator)
	{
		gi_cprintf(ent, PRINT_HIGH, "You are an observer.\n");
		return;
	}
//CW--

	if ((level.time - ent->client->respawn_time) < REKILL_DELAY)
		return;

	ent->flags &= ~FL_GODMODE;
	ent->health = 0;

//CW++
	ent->s.effects = 0;
	ent->s.renderfx = 0;
//CW--

	meansOfDeath = MOD_SUICIDE;
	player_die(ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f(edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;

//ZOID++
	if (ent->client->menu)
		PMenu_Close(ent);

	ent->client->update_chase = true;
//ZOID--
}


int PlayerSort(void const *a, void const *b)
{
	int	anum;
	int	bnum;

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
void Cmd_Players_f(edict_t *ent)
{	
	char	small[64];
	char	large[1280];
	int		index[256];
	int		count = 0;
	int		i;

	for (i = 0 ; i < (int)maxclients->value ; i++)
	{
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}
	}

	// sort by frags
	qsort(index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf(small, sizeof(small), "%3i %s\n", game.clients[index[i]].ps.stats[STAT_FRAGS], game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100)
		{	// can't print all of them in one packet
			Com_strcat(large, sizeof(large), "...\n");
			break;
		}
		Com_strcat(large, sizeof(large), small);
	}

	gi_cprintf(ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f(edict_t *ent)
{
	int	i;

//CW++
	if (ent->client->spectator)
		return;
//CW--

	i = atoi(gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
		case 0:
			gi_cprintf(ent, PRINT_HIGH, "flipoff\n");
			ent->s.frame = FRAME_flip01-1;
			ent->client->anim_end = FRAME_flip12;
			break;

		case 1:
			gi_cprintf(ent, PRINT_HIGH, "salute\n");
			ent->s.frame = FRAME_salute01-1;
			ent->client->anim_end = FRAME_salute11;
			break;

		case 2:
			gi_cprintf(ent, PRINT_HIGH, "taunt\n");
			ent->s.frame = FRAME_taunt01-1;
			ent->client->anim_end = FRAME_taunt17;
			break;

		case 3:
			gi_cprintf(ent, PRINT_HIGH, "wave\n");
			ent->s.frame = FRAME_wave01-1;
			ent->client->anim_end = FRAME_wave11;
			break;

		case 4:
		default:
			gi_cprintf(ent, PRINT_HIGH, "point\n");
			ent->s.frame = FRAME_point01-1;
			ent->client->anim_end = FRAME_point12;
			break;
	}
}

qboolean CheckFlood(edict_t *ent)
{
	gclient_t *cl;
	int		i;

	if (flood_msgs->value)
	{
		cl = ent->client;

        if (level.time < cl->flood_locktill)
		{
			gi_cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n", (int)(cl->flood_locktill - level.time));
            return true;
        }

        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when) / sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && (level.time - cl->flood_when[i] < flood_persecond->value))
		{
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi_cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n", (int)flood_waitdelay->value);
            return true;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %	(sizeof(cl->flood_when) / sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}
	return false;
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f(edict_t *ent, qboolean arg0)															//CW
{
	edict_t	*other;
	char	*p;
	char	text[2048];
	int		j;

	if ((gi.argc () < 2) && !arg0)
		return;

//CW++
	if (ent->client->pers.muted)
	{
		gi_cprintf(ent, PRINT_HIGH, "You have been muted.\n");
		return;
	}
//CW--

	if (CheckFlood(ent))																			//CW
		return;

	Com_sprintf(text, sizeof(text), "%s: ", ent->client->pers.netname);
	if (arg0)
	{
//CW++
		if (strlen(gi.argv(0)) > 132)
		{
			gi_cprintf(ent, PRINT_HIGH, "Message is too long.\n");
			return;
		}
//CW--
		Com_strcat(text, sizeof(text), gi.argv(0));
		Com_strcat(text, sizeof(text), " ");
//CW++
		if (strlen(gi.args()) > 132)
		{
			gi_cprintf(ent, PRINT_HIGH, "Message is too long.\n");
			return;
		}
//CW--
		Com_strcat(text, sizeof(text), gi.args());
	}
	else
	{
//CW++
		if (strlen(gi.args()) > 132)
		{
			gi_cprintf(ent, PRINT_HIGH, "Message is too long.\n");
			return;
		}
//CW--
		p = gi.args();
		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		Com_strcat(text, sizeof(text), p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	Com_strcat(text, sizeof(text), "\n");

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
																									//CW
		gi_cprintf(other, PRINT_CHAT, "%s", text);
	}
}

//Pon++
/*
==================
Cmd_UndoChain_f
==================
*/
void Cmd_UndoChain_f(edict_t *ent, int step)
{
	if (!(int)chedit->value)
	{
		gi_cprintf(ent, PRINT_HIGH, "No route chaining in progress!\n");
		return;
	}

	if (!ent->deadflag && (ent == &g_edicts[1]))
	{
		trace_t	rs_trace;
		int		count;
		int		i;

		if (step < 2)
			count = 2;
		else
			count = step;

		for (i = CurrentIndex - 1; i > 0; i--)
		{
			if (Route[i].state == GRS_NORMAL)
			{
				rs_trace = gi.trace(Route[i].Pt, ent->mins, ent->maxs, Route[i].Pt, ent, MASK_BOTSOLID);
				if ((--count <= 0) && !rs_trace.allsolid && !rs_trace.startsolid)
					break;
			}  
		}

		gi_cprintf(ent, PRINT_HIGH, "Backed %i %i steps.\n", CurrentIndex - i, step);
		
		CurrentIndex = i;
		VectorCopy(Route[CurrentIndex].Pt, ent->s.origin);
		VectorCopy(Route[CurrentIndex].Pt, ent->s.old_origin);
		memset(&Route[CurrentIndex], 0, sizeof(route_t));
		if (CurrentIndex > 0)
			Route[CurrentIndex].index = Route[CurrentIndex - 1].index + 1; 
	}
}
//Pon--

/*
=================
ClientCommand
=================
*/
void ClientCommand(edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;			// not fully in game yet

	cmd = gi.argv(0);

	if (Q_stricmp(cmd, "players") == 0)
	{
		Cmd_Players_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "say") == 0)
	{
		Cmd_Say_f(ent, false);																		//CW
		return;
	}

	if ((Q_stricmp(cmd, "say_team") == 0) || (Q_stricmp(cmd, "steam") == 0))
	{
		CTFSay_Team(ent, gi.args());
		return;
	}

	if (Q_stricmp(cmd, "score") == 0)
	{
		Cmd_Score_f(ent);
		return;
	}

	if (Q_stricmp(cmd, "help") == 0)
	{
		Cmd_Score_f(ent);																			//CW
		return;
	}

	if (level.intermissiontime)
		return;

	if (Q_stricmp(cmd, "use") == 0)
		Cmd_Use_f(ent);
	else if (Q_stricmp(cmd, "drop") == 0)
		Cmd_Drop_f(ent);
	else if (Q_stricmp(cmd, "give") == 0)
		Cmd_Give_f(ent);
	else if (Q_stricmp(cmd, "god") == 0)
		Cmd_God_f(ent);
	else if (Q_stricmp(cmd, "notarget") == 0)
		Cmd_Notarget_f(ent);
	else if (Q_stricmp(cmd, "noclip") == 0)
		Cmd_Noclip_f(ent);
	else if (Q_stricmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
	else if (Q_stricmp(cmd, "invnext") == 0)
		SelectNextItem(ent, -1);
	else if (Q_stricmp(cmd, "invprev") == 0)
		SelectPrevItem(ent, -1);
	else if (Q_stricmp(cmd, "invnextw") == 0)
		SelectNextItem(ent, IT_WEAPON);
	else if (Q_stricmp(cmd, "invprevw") == 0)
		SelectPrevItem(ent, IT_WEAPON);
	else if (Q_stricmp(cmd, "invnextp") == 0)
		SelectNextItem(ent, IT_POWERUP);
	else if (Q_stricmp(cmd, "invprevp") == 0)
		SelectPrevItem(ent, IT_POWERUP);
	else if (Q_stricmp(cmd, "invuse") == 0)
		Cmd_InvUse_f(ent);
	else if (Q_stricmp(cmd, "invdrop") == 0)
		Cmd_InvDrop_f(ent);
	else if (Q_stricmp(cmd, "weapprev") == 0)
		Cmd_WeapPrev_f(ent);
	else if (Q_stricmp(cmd, "weapnext") == 0)
		Cmd_WeapNext_f(ent);
	else if (Q_stricmp(cmd, "weaplast") == 0)
		Cmd_WeapLast_f(ent);
	else if (Q_stricmp(cmd, "kill") == 0)
		Cmd_Kill_f(ent);
	else if (Q_stricmp(cmd, "putaway") == 0)
		Cmd_PutAway_f(ent);
	else if (Q_stricmp(cmd, "wave") == 0)
		Cmd_Wave_f(ent);

//ZOID++
	else if (Q_stricmp(cmd, "team") == 0)
		CTFTeam_f(ent);
	else if (Q_stricmp(cmd, "id") == 0)
		CTFID_f (ent);																				//CW
	else if (Q_stricmp(cmd, "ready") == 0)
		CTFReady(ent);
	else if (Q_stricmp(cmd, "notready") == 0)
		CTFNotReady(ent);
	else if (Q_stricmp(cmd, "ghost") == 0)
		CTFGhost(ent);
	else if (Q_stricmp(cmd, "stats") == 0)
		CTFStats(ent);
	else if (Q_stricmp(cmd, "playerlist") == 0)
		CTFPlayerList(ent);
	else if (Q_stricmp(cmd, "observer") == 0)
		CTFObserver(ent);
//ZOID--

//CW++
	else if (!Q_stricmp(cmd, "hook"))
		Cmd_Hook_f(ent);
	else if (!Q_stricmp(cmd, "unhook"))
		Cmd_Unhook_f(ent);
	else if (!Q_stricmp(cmd, "menu"))
		Cmd_Menu_f(ent);
	else if (!Q_stricmp(cmd, "motd"))
		Cmd_MOTD_f(ent);
	else if (!Q_stricmp(cmd, "vote"))
		Cmd_Vote_f(ent, false);
	else if (Q_stricmp(cmd, "yes") == 0)
		Cmd_VoteYes_f(ent, false);
	else if (Q_stricmp(cmd, "no") == 0)
		Cmd_VoteNo_f(ent, false);	
	else if (!Q_stricmp(cmd, "mission"))
		Cmd_Mission_f(ent);
	else if (!Q_stricmp(cmd, "play_voice"))
		Cmd_PlayVoice_f(ent);
	else if (!Q_stricmp(cmd, "play_team"))
		Cmd_PlayTeam_f(ent);
	else if (!Q_stricmp(cmd, "pull"))
		Cmd_AGMPull_f(ent);
	else if (!Q_stricmp(cmd, "unpull"))
		Cmd_AGMUnpull_f(ent);
	else if (!Q_stricmp(cmd, "push"))
		Cmd_AGMPush_f(ent);
	else if (!Q_stricmp(cmd, "unpush"))
		Cmd_AGMUnpush_f(ent);
	else if (!Q_stricmp(cmd, "fling"))
		Cmd_AGMFling_f(ent);
	else if (!Q_stricmp(cmd, "maplist"))
		Cmd_MapList_f(ent);
	else if (!Q_stricmp(cmd, "idtrap"))
		Cmd_TrapID_f(ent);
	else if (!Q_stricmp(cmd, "weap_note"))
		Cmd_WeaponNote_f(ent);
	else if (!Q_stricmp(cmd, "addbots"))
		Cmd_AddBots_f(ent);
	else if (!Q_stricmp(cmd, "clearbots"))
		Cmd_ClearBots_f(ent);

//	Developer commands.

	else if (!Q_stricmp(cmd, "dev_ents"))
		Cmd_DevEnts_f(ent);
	else if (!Q_stricmp(cmd, "show_botroute"))
		Cmd_ShowBotRoute_f(ent);

//	Operator commands.

	else if (!Q_stricmp(cmd, "op"))
		Cmd_GiveOp_f(ent);
	else if (!Q_stricmp(cmd, "op_menu"))
		Cmd_OpMenu_f(ent);
	else if (!Q_stricmp(cmd, "op_kick"))
		CTFBoot(ent, false);
	else if (!Q_stricmp(cmd, "op_ban"))
		CTFBoot(ent, true);
	else if (!Q_stricmp(cmd, "op_mute"))
		Cmd_SayPrivs_f(ent, true);
	else if (!Q_stricmp(cmd, "op_unmute"))
		Cmd_SayPrivs_f(ent, false);
	else if (!Q_stricmp(cmd, "op_swapteam"))
		Cmd_SwapTeam_f(ent);
	else if (!Q_stricmp(cmd, "op_iplist"))
		Cmd_OpIPList_f(ent);
	else if (!Q_stricmp(cmd, "op_vote"))
		Cmd_Vote_f(ent, true);
	else if (!Q_stricmp(cmd, "op_yes"))
		Cmd_VoteYes_f(ent, true);
	else if (!Q_stricmp(cmd, "op_no"))
		Cmd_VoteNo_f(ent, true);
	else if (!Q_stricmp(cmd, "op_dropagm"))
		Cmd_DropAGM_f(ent);
	else if (!Q_stricmp(cmd, "op_dropdisc"))
		Cmd_DropDL_f(ent);
	else if (!Q_stricmp(cmd, "op_start"))
		Cmd_MatchStart_f(ent);
	else if (!Q_stricmp(cmd, "op_slots"))
		Cmd_ListSlots_f(ent);
	else if (!Q_stricmp(cmd, "op_addbots"))
		Cmd_OpAddBots_f(ent);
	else if (!Q_stricmp(cmd, "op_clearbots"))
		Cmd_OpClearBots_f(ent);
//CW--

//Pon++
	else if (!Q_stricmp(cmd, "undo"))
	{
		if (gi.argc() <= 1)
			Cmd_UndoChain_f(ent, 1);
		else
			Cmd_UndoChain_f(ent, atoi(gi.argv(1)));
	}
//Pon--

	else						// anything that doesn't match a command will be a chat
		Cmd_Say_f(ent, true);																		//CW
}
