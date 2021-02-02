// p_hud.c

#include "g_local.h"

/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission(edict_t *ent)
{
	ent->client->showscores = true;																	//CW

	VectorCopy(level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0] * 8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1] * 8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2] * 8;
	VectorCopy(level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
#ifdef KMQUAKE2_ENGINE_MOD
	ent->client->ps.gunindex2 = 0;
#endif
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

//	Clean up powerup info.

	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;
//CW++
	ent->client->show_gausscharge = false;
	ent->client->show_gausstarget = 0;
	ent->client->gauss_dmg = 0;
	ent->client->gauss_framenum = 0;

	ent->client->antibeam_framenum = 0;
	ent->client->frozen_framenum = 0;
	ent->client->siphon_framenum = 0;
	ent->client->needle_framenum = 0;
	ent->client->haste_framenum = 0;

	ent->client->mod_changeteam = false;
	ent->burning = false;
	ent->disintegrated = false;
	ent->tractored = false;

	ent->client->agm_charge = 0;
	ent->client->agm_showcharge = false;
	ent->client->agm_tripped = false;
	ent->client->agm_on = false;
	ent->client->agm_push = false;
	ent->client->agm_pull = false;
	ent->client->held_by_agm = false;
	ent->client->flung_by_agm = false;
	ent->client->thrown_by_agm = false;
	ent->client->agm_target = NULL;
	ent->client->agm_enemy = NULL;

	if (ent->client->ctf_grapple)
		CTFResetGrapple(ent->client->ctf_grapple);
//CW--

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex4 = 0;																			//CW
#ifdef KMQUAKE2_ENGINE_MOD
	ent->s.modelindex5 = 0;
	ent->s.modelindex6 = 0;
#endif
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

//	Add the layout.

//Maj++
	// don't unicast() to bots!
	if (ent->isabot)
		return;
//Maj--

	DeathmatchScoreboardMessage(ent, NULL);															//CW
	gi.unicast(ent, true);
}

void BeginIntermission(edict_t *targ)
{
	edict_t	*ent;
	edict_t	*client;
	edict_t	*wep_ent;																				//CW++
	int		i;

	if (level.intermissiontime)
		return;		// already activated

//ZOID++
	if ((int)sv_gametype->value > G_FFA)															//CW
		CTFCalcScores();
//ZOID--

	game.autosaved = false;

//	Respawn any dead clients.

	for (i = 0; i < (int)maxclients->value; ++i)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;

		if (client->health < 1)
			Respawn(client);
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;
	level.exitintermission = 0;																		//CW

//	Find an intermission spot.

	ent = G_Find(NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find(NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find(ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find(ent, FOFS(classname), "info_player_intermission");
		}
	}

	VectorCopy(ent->s.origin, level.intermission_origin);
	VectorCopy(ent->s.angles, level.intermission_angle);

//CW++
//	Remove weapon entities to avoid server crashes due to delayed player deaths.

	for (i = 0; i < globals.num_edicts; ++i)
	{
		wep_ent = &g_edicts[i];

		if (!wep_ent->inuse)
			continue;

		if ((wep_ent->die == Trap_DieFromDamage) || (wep_ent->die == C4_DieFromDamage))
		{
			TList_DelNode(wep_ent);
			G_FreeEdict(wep_ent);
		}
		
		if (wep_ent->wep_proj)
			G_FreeEdict(wep_ent);
	}
//CW--

//	Move all clients to the intermission point.

	for (i = 0; i < (int)maxclients->value; ++i)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
//CW++
		if (client->isabot)
			continue;
//CW--

		MoveClientToIntermission(client);
	}
}


/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage(edict_t *ent, edict_t *killer)
{
	gclient_t	*cl;
	edict_t		*cl_ent;
	char		entry[1024];
	char		string[1400];
	char		*tag;
	char		entry2[64];																			//CW++
	int			stringlength;
	int			i, j, k;
	int			sorted[MAX_CLIENTS];
	int			sortedscores[MAX_CLIENTS];
	int			score;
	int			total;
	int			picnum;
	int			x, y;

//CW++
	if (ent->client->showscores || ent->client->showinventory)
	{
		if (ent->client->show_gausstarget)
			ent->client->show_gausstarget = 2;
	}

	if (ent->client->showscores)
	{
//CW--

//ZOID++
		if (sv_gametype->value == G_CTF)															//CW
		{
			CTFScoreboardMessage(ent, killer);
			return;
		}
//ZOID--

//CW++
		else if ((sv_gametype->value == G_TDM) || (sv_gametype->value == G_ASLT))
		{
			TDMScoreboardMessage(ent, killer);
			return;
		}
//CW--

		// sort the clients by score
		total = 0;
		for (i = 0; i < game.maxclients; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;
//CW++
			if (cl_ent->client->spectator)
				continue;
//CW--
			score = game.clients[i].resp.score;
			for (j = 0; j < total; j++)
			{
				if (score > sortedscores[j])
					break;
			}
			for (k = total; k > j; k--)
			{
				sorted[k] = sorted[k-1];
				sortedscores[k] = sortedscores[k-1];
			}
			sorted[j] = i;
			sortedscores[j] = score;
			total++;
		}

//CW++
//		Add spectators to the end of the list.

		for (i = 0; i < game.maxclients; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;
			if (!cl_ent->client->spectator)
				continue;

			sorted[total] = i;
			sortedscores[total] = 0;
			total++;
		}
//CW--

		// print level name and exit rules
		string[0] = 0;
		stringlength = (int)strlen(string);

		// add the clients in sorted order
		if (total > 12)
			total = 12;

		for (i = 0; i < total; i++)
		{
			cl = &game.clients[sorted[i]];
			cl_ent = g_edicts + 1 + sorted[i];

			picnum = gi.imageindex("i_fixme");
			x = (i >= 6) ? 160 : 0;
			y = 32 + 32 * (i % 6);

			// add a dogtag
			if (cl_ent == ent)
				tag = "tag1";
			else if (cl_ent == killer)
				tag = "tag2";
			else
				tag = NULL;
			if (tag)
			{
				Com_sprintf(entry, sizeof(entry), "xv %i yv %i picn %s ", x+32, y, tag);
				j = (int)strlen(entry);
				if (stringlength + j > 1024)
					break;

			//	strcpy(string + stringlength, entry);
				Com_strcpy(string + stringlength, sizeof(string)-stringlength, entry);
				stringlength += j;
			}

			// send the layout
//CW++
			if (cl->spectator)
			{
				Com_sprintf(entry, sizeof(entry), "xv %d yv %d string2 \"%s\" ", x+32, y, cl->pers.netname);
				Com_sprintf(entry2, sizeof(entry2), "xv %d yv %d string \"(Spectator)\" ", x+32, y+8);
				Com_strcat(entry, sizeof(entry), entry2);
				if (cl->chase_target)
				{
					Com_sprintf(entry2, sizeof(entry2), "xv %d yv %d string \"Watching:\" ", x+32, y+16);
					Com_strcat(entry, sizeof(entry), entry2);
					Com_sprintf(entry2, sizeof(entry2), "xv %d yv %d string \"%s\" ", x+32, y+24, cl->chase_target->client->pers.netname);
					Com_strcat(entry, sizeof(entry), entry2);
				}
			}
			else
//CW--
				Com_sprintf(entry, sizeof(entry), "client %i %i %i %i %i %i ", x, y, sorted[i], cl->resp.score, cl->ping, (int)((level.framenum - cl->resp.enterframe)/600));
			j = (int)strlen(entry);
			if (stringlength + j > 1024)
				break;

		//	strcpy(string + stringlength, entry);
			Com_strcpy(string + stringlength, sizeof(string)-stringlength,entry);
			stringlength += j;
		}

//CW++
	}
	else
		*string = 0;

	if (ent->client->show_gausstarget & 1)
		ShowGaussTarget(ent, string, sizeof(string));
//CW--

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard(edict_t *ent)
{
//Maj++
	// don't unicast() to bots!
	if (ent->isabot)
		return;
//Maj--

	DeathmatchScoreboardMessage(ent, ent->enemy);
	gi.unicast(ent, true);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f(edict_t *ent)
{
//CW++
	if (ent->isabot)
		return;
//CW--

	ent->client->showinventory = false;
	ent->client->showhelp = false;

//ZOID++
	if (ent->client->menu)
		PMenu_Close(ent);
//ZOID--

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		ent->client->update_chase = true;

//CW++
		if (ent->client->show_gausstarget & 2)
			ent->client->show_gausstarget = 1;
//CW--
		return;
	}

	ent->client->showscores = true;
	DeathmatchScoreboard(ent);
}


//=======================================================================

//CW++
/*
===============
SetWepIDView

This is a modified version of the CTFSetIDView() function written by Zoid.
===============
*/
void SetWepIDView(edict_t *ent)
{
	edict_t		*index;
	edict_t		*check;
	vec3_t		forward;
	vec3_t		start;
	vec3_t		dir;
	float		dp_best = 0.0;
	float		dp;
	float		range;
	qboolean	finished = false;

//	If the player has no C4 or Traps, don't bother.

	if (!ent->next_node)
	{
		ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = 0;
		return;
	}

//	Only check every few frames.

	if (level.time - ent->client->resp.id_trap_time < TRAP_ID_CHECKTIME)
		return;

//	Cycle through the player's linked list of C4 and Trap entities to determine which
//	visible entity (if any) is closest to the player's facing direction.

	ent->client->resp.id_trap_time = level.time;
	ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = 0;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	start[0] = ent->s.origin[0];
	start[1] = ent->s.origin[1];
	start[2] = ent->s.origin[2] + ent->viewheight;

	index = ent->next_node;
	while (index && !finished)
	{
		check = index;
		if (index->next_node)
			index = index->next_node;
		else
			finished = true;

		VectorSubtract(check->s.origin, start, dir);
		range = VectorLength(dir);
		VectorNormalize(dir);
		dp = DotProduct(forward, dir);
		if ((dp > dp_best) && (range < TRAP_ID_RANGE) && visible(ent, check))
			dp_best = dp;
	}

	if (dp_best > TRAP_ID_DOTPRODUCT)
		ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = gi.imageindex("i_no");
}
//CW--

/*
===============
G_SetStats
===============
*/
void G_SetStats(edict_t *ent)
{
	gitem_t	*item;
	int		index;
	int		cells = 0;																				//CW
	int		power_armor_type;

	// health
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
//	ent->client->ps.stats[STAT_MAXHEALTH] = min(max(ent->client->pers.max_health, 0), 10000);
	ent->client->ps.stats[STAT_MAXHEALTH] = min(max(ent->max_health, 0), 10000);
#endif

	// ammo
	if (!ent->client->ammo_index)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_MAXAMMO] = 0;
#endif
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex(item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_MAXAMMO] = min(max(GetMaxAmmoByIndex(ent->client, ent->client->ammo_index), 0), 10000);
#endif
	}
	
	// armor
	power_armor_type = PowerArmorType(ent);
	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))];
		if (cells == 0)
		{	// ran out of cells for power armor
			ent->flags &= ~(FL_POWER_SHIELD|FL_POWER_SCREEN);
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;
		}
	}

	index = ArmorIndex (ent);
	if (power_armor_type && (!index || (level.framenum & 8)))
	{	// flash between power armor and other armor icon
		if (power_armor_type == POWER_ARMOR_SHIELD)
			ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex("i_powershield");
		else	// POWER_ARMOR_SCREEN
			ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powerscreen");
		ent->client->ps.stats[STAT_ARMOR] = cells;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_MAXARMOR] = min(max(ent->client->pers.max_cells, 0), 10000);
#endif
	}
	else if (index)
	{
		item = GetItemByIndex(index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex(item->icon);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_MAXARMOR] = min(max(GetMaxArmorByIndex(index), 0), 10000);
#endif
	}
	else
	{
		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_MAXARMOR] = 0;
#endif
	}

	// pickup message
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	// timers
//CW++
//	Show weapon charges before powerups.

	if (ent->client->show_gausscharge)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("a_blaster");
		if (ent->client->pers.inventory[ent->client->ammo_index] > 0)
		{
			ent->client->gauss_dmg = (int)((level.framenum - ent->client->gauss_framenum) / (10.0 * sv_gauss_damage_rate->value));
			ent->client->gauss_dmg *= (int)sv_gauss_damage_step->value;
			ent->client->gauss_dmg += (int)sv_gauss_damage_base->value;
		}
		else
			ent->client->gauss_dmg = (int)sv_gauss_damage_base->value;

		if (ent->client->gauss_dmg > sv_gauss_damage_max->value)
			ent->client->gauss_dmg = (int)sv_gauss_damage_max->value;

		ent->client->ps.stats[STAT_TIMER] = ent->client->gauss_dmg;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max((int)sv_gauss_damage_max->value, 0), 10000);
#endif
	}
	else if (ent->client->agm_showcharge)
	{
		if (ent->client->agm_tripped)
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("a_refuse");
		else
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("a_agm");
		ent->client->ps.stats[STAT_TIMER] = ent->client->agm_charge;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_TIMER_RANGE] = 100;
#endif
	}
//CW--
	else if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum) / 10;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max(QUAD_TIMEOUT_FRAMES / 10, 0), 10000);
#endif
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum) / 10;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max(INVINCIBLE_TIMEOUT_FRAMES / 10, 0), 10000);
#endif
	}
//CW++
	else if (ent->client->siphon_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_siphon");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->siphon_framenum - level.framenum) / 10;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max(SIPHON_TIMEOUT_FRAMES / 10, 0), 10000);
#endif
	}
	else if (ent->client->needle_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_needle");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->needle_framenum - level.framenum) / 10;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max(NEEDLE_TIMEOUT_FRAMES / 10, 0), 10000);
#endif
	}
	else if (ent->client->haste_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_haste");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->haste_framenum - level.framenum) / 10;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max(HASTE_TIMEOUT_FRAMES / 10, 0), 10000);
#endif
	}
//CW--
	else if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_envirosuit");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum) / 10;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max(ENVIROSUIT_TIMEOUT_FRAMES / 10, 0), 10000);
#endif
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_rebreather");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum) / 10;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max(BREATHER_TIMEOUT_FRAMES / 10, 0), 10000);
#endif
	}
//CW++
	else if (ent->client->antibeam_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("k_datacd");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->antibeam_framenum - level.framenum) / 10;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_TIMER_RANGE] = min(max(ANTIBEAM_TIMEOUT_FRAMES / 10, 0), 10000);
#endif
	}
//CW--
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
		ent->client->ps.stats[STAT_TIMER_RANGE] = 0;
#endif
	}

	// selected item
	if ((ent->client->pers.selected_item == -1) || !ent->client->pers.selected_item)				//CW
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex(itemlist[ent->client->pers.selected_item].icon);

	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	// layouts
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if ((ent->client->pers.health <= 0) || level.intermissiontime || ent->client->showscores || ent->client->show_gausstarget) //CW
		ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	if (ent->client->showinventory && (ent->client->pers.health > 0))
		ent->client->ps.stats[STAT_LAYOUTS] |= 2;


	// frags
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	// help icon / current weapon if not shown
	if (ent->client->resp.helpchanged && (level.framenum & 8))
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex("i_help");
	else if (((ent->client->pers.hand == CENTER_HANDED) || (ent->client->ps.fov > 91.0)) && ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex(ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

#ifdef KMQUAKE2_ENGINE_MOD	// for enhanced HUD
	if (ent->client->pers.weapon) {
		ent->client->ps.stats[STAT_WEAPON] = gi.imageindex (ent->client->pers.weapon->icon);
	}
	else
		ent->client->ps.stats[STAT_WEAPON] = 0;
#endif

//ZOID++
	SetCTFStats(ent);
//ZOID--

//CW++
	if (((int)sv_gametype->value == G_FFA) && ent->client->resp.id_trap)
		SetWepIDView(ent);
//CW--
}

//CW++
/*
==================
ShowGaussTarget

Helpful hints for the implementation of this code
were given by Damien "Yaya" Slee.
==================
*/
void ShowGaussTarget (edict_t *ent, char *string, size_t stringSize)
{
	edict_t		*cl_ent;
	edict_t		*targ = NULL;
	vec3_t		targ_vec;
	vec3_t		forward;
	vec3_t		right;
	vec3_t		offset;
	vec3_t		start;
	vec3_t		vec;
	vec3_t		t_ang;
	char		str_temp[64];
	float		dist;
	float		min_dist = WORLD_SIZE;	// was 8192.0
	int			x = 999;
	int			y = 999;
	int			i;

//	Display the targeting area overlay.

	Com_sprintf(str_temp, sizeof(str_temp), "xv 32 yv -4 picn g_scan ");
	if ((strlen(string) + strlen(str_temp)) < 1024)
	//	strcat(string, str_temp);
		Com_strcat(string, stringSize, str_temp);

//	Search through the list of entities to determine if any live players are in front of us.
//	For multiple targets, select the nearest one.

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24.0, 8.0, ent->viewheight-5.0);		// NB: should be same as offset in Weapon_GaussPistol_Fire()
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	for (i = 0; i < globals.num_edicts; ++i)
	{
		cl_ent = &g_edicts[i];

		if (!cl_ent->client)
			continue;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->movetype == MOVETYPE_NOCLIP)
			continue;
		if (cl_ent == ent)
			continue;
		if (cl_ent->health < 1)
			continue;
		if (((int)sv_gametype->value > G_FFA) && (ent->client->resp.ctf_team == cl_ent->client->resp.ctf_team))
			continue;

		VectorSubtract(cl_ent->s.origin, start, vec);
		if ((dist = VectorLength(vec)) > sv_gauss_scan_range->value)
			continue;

		VectorNormalize(vec);
		if (DotProduct(vec, forward) > 0.75)
		{
			if (dist < min_dist)
			{
				targ = cl_ent;
				VectorCopy(vec, targ_vec);
				min_dist = dist;
			}
		}
	}

//	If a valid target has been found, check that it is within the scanner's field-of-view,
//	and if so, position the tracking reticle over them.

	if (targ != NULL)
	{
		vectoangles(targ_vec, t_ang);
		x = (int)(GAUSS_AIMCAL * sin(DEG2RAD((ent->client->v_angle[1] - t_ang[1]))));
		y = (int)(GAUSS_AIMCAL * sin(DEG2RAD((t_ang[0] - ent->client->v_angle[0]))));
	}
	else
		return;

	if ((abs(x) < 70) && (abs(y) < 70))
	{
		Com_sprintf(str_temp, sizeof(str_temp), "xv %i yv %i picn g_targ ", 136+x, 100+y);
		if ((strlen(string) + strlen(str_temp)) < 1024)
		//	strcat(string, str_temp);
			Com_strcat(string, stringSize, str_temp);

		if (level.time > targ->delay)
		{
			targ->delay = level.time + 2.0;
			if (targ->target_ent && targ->target_ent->client && targ->target_ent->client->spycam)
			{
				unicastSound(targ->target_ent, gi.soundindex("weapons/gauss/warn.wav"), 1.0);		//r1,CW
				targ->target_ent->delay = level.time + 2.0;
			}
			else
				unicastSound(targ, gi.soundindex("weapons/gauss/warn.wav"), 1.0);					//r1,CW
		}
	}
}
//CW--
