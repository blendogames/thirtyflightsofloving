#include "g_local.h"
#include "m_player.h"

int	nostatus = 0;

char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	Com_strcpy(value, sizeof(value), Info_ValueForKey (ent->client->pers.userinfo, "skin"));
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

	if (cl->menu)
	{
		PMenu_Next(ent);
		return;
	} 
	if (cl->textdisplay)
	{
		Text_Next (ent);
		return;
	} 
	else if (cl->chase_target)
	{
		ChaseNext(ent);
		return;
	}

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

	if (cl->menu)
	{
		PMenu_Prev(ent);
		return;
	}
	if (cl->textdisplay)
	{
		Text_Prev (ent);
		return;
	}
	else if (cl->chase_target)
	{
		ChasePrev(ent);
		return;
	}

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

static void Cmd_Give_Usage (edict_t *ent, char *parm1)
{
	gi.cprintf (ent, PRINT_HIGH, "usage: give %s {xatrix | rogue | neither | both}\n", parm1);
//	gi.cprintf (ent, PRINT_HIGH, "usage: give %s {xatrix | rogue | zaero | neither | both}\n", parm1);
}

qboolean Cmd_Give_ParseFlags (edict_t *ent, char *parm1, int *flags)
{
	if ( !ent || !parm1 || !flags )
		return false;

	if ( !gi.argv(2) ) {
		Cmd_Give_Usage (ent, parm1);
		return false;
	}
	if (Q_stricmp(gi.argv(2), "xatrix") == 0)
		*flags |= 2;
	else if (Q_stricmp(gi.argv(2), "rogue") == 0)
		*flags |= 4;
	else if (Q_stricmp(gi.argv(2), "both") == 0)
		*flags |= 6;
//	else if (Q_stricmp(gi.argv(2), "zaero") == 0)
//		*flags |= 8;
	else if (Q_stricmp(gi.argv(2), "neither")) {
		Cmd_Give_Usage (ent, parm1);
		return false;
	}

	return true;
}

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
	int			i, give_flags = 1;
	qboolean	give_all = false;
	edict_t		*it_ent;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}
	
	// Knightmare- override ammo pickup values with cvars
	SetAmmoPickupValues ();

	name = gi.args();

	if (!Q_stricmp(name,"jetpack"))
	{
		gitem_t *fuel;
		fuel = FindItem("fuel");
		Add_Ammo(ent,fuel,500);
	}

/*	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;*/

	if (Q_stricmp(gi.argv(1), "all") == 0)
	{
		if ( !Cmd_Give_ParseFlags(ent, gi.argv(1), &give_flags) )
			return;
		give_all = true;
	}

	if ( (Q_stricmp(gi.argv(1), "weapons") == 0) || ((Q_stricmp(gi.argv(1), "ammo") == 0) && Q_stricmp(name, "ammo pack")) )
	{
		if ( !Cmd_Give_ParseFlags(ent, gi.argv(1), &give_flags) )
			return;
	}

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (!give_all && gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

//	if (give_all || Q_stricmp(name, "weapons") == 0)
	if (give_all || Q_stricmp(gi.argv(1), "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
		//	if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			if ( (it->flags & IT_XATRIX) && !(give_flags & 2) )
				continue;
			if ( (it->flags & IT_ROGUE) && !(give_flags & 4) )
				continue;
			if ( (it->flags & IT_ZAERO) && !(give_flags & 8) )
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

//	if (give_all || Q_stricmp(name, "ammo") == 0)
	if ( give_all || ((Q_stricmp(gi.argv(1), "ammo") == 0) && Q_stricmp(name, "ammo pack")) )
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			if ( (it->flags & IT_XATRIX) && !(give_flags & 2) )
				continue;
			if ( (it->flags & IT_ROGUE) && !(give_flags & 4) )
				continue;
			if ( (it->flags & IT_ZAERO) && !(give_flags & 8) )
				continue;
			if (it->classname && !Q_stricmp(it->classname,"ammo_fuel") && !developer->value)
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
			if (!it->classname)									// ignore health placeholder
				continue;
			if (it->flags & IT_NOT_GIVEABLE)					// ROGUE
				continue;										// ROGUE
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			if ( (it->flags & IT_XATRIX) && !(give_flags & 2) )
				continue;
			if ( (it->flags & IT_ROGUE) && !(give_flags & 4) )
				continue;
			if ( (it->flags & IT_ZAERO) && !(give_flags & 8) )
				continue;
			if (it->classname && !Q_stricmp(it->classname,"item_jetpack") && !developer->value)
				continue;
			if (it->classname && !Q_stricmp(it->classname,"item_flashlight") && !developer->value)
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
			gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

//ROGUE
	if (it->flags & IT_NOT_GIVEABLE)		
	{
		gi.dprintf ("item cannot be given\n");
		return;							
	}
//ROGUE

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		// Lazarus: Bleah - special case for "homing rockets"
		if (it->tag == AMMO_HOMING_ROCKETS)
		{
			if (gi.argc() == 4)
				ent->client->pers.inventory[index] = atoi(gi.argv(3));
			else
				ent->client->pers.inventory[index] += it->quantity;
		}
		else
		{
			if (gi.argc() == 3)
				ent->client->pers.inventory[index] = atoi(gi.argv(2));
			else
				ent->client->pers.inventory[index] += it->quantity;
		}
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		// PMM - since some items don't actually spawn when you say to ..
		if (!it_ent->inuse)
			return;
		// pmm
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
		ent->solid = SOLID_BBOX;
		msg = "noclip OFF\n";
	}
	else
	{	//Knightmare- prevent players from squishing monsters, insanes, etc on moving platforms while in noclip mode
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
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
#ifdef JETPACK_MOD
	if(!Q_stricmp(s,"jetpack"))
	{
		// Special case - turns on/off
		if(!ent->client->jetpack)
		{
			if(ent->waterlevel > 0)
				return;
			if(!ent->client->pers.inventory[index])
			{
				gi.cprintf(ent, PRINT_HIGH, "Out of item: %s\n", s);
				return;
			}
			else if(ent->client->pers.inventory[fuel_index] <= 0)
			{
				gi.cprintf(ent, PRINT_HIGH, "No fuel for: %s\n", s);
				return;
			}
		}
		it->use(ent,it);
		return;
	}
#endif
	// added stasis generator support
	if (!Q_stricmp(s,"stasis generator"))
	{
		// Special case - turn freeze off if already on
		if(level.freeze)
		{
			level.freeze = false;
			level.freezeframes = 0;
			return;
		}
	}
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
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
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
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

	cl->showscores = false;
	cl->showhelp = false;

	if (cl->textdisplay)
	{
		Text_Close(ent);
		return;
	}
	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		// Don't show "No Weapon" or "Homing Rocket Launcher" in inventory
		if ((i == noweapon_index) || (i == hml_index))
			gi.WriteShort (0);
		else if((i == fuel_index) && (ent->client->jetpack_infinite))
			gi.WriteShort (0);
		else
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
#ifdef JETPACK_MOD
	if(!Q_stricmp(it->classname,"item_jetpack"))
	{
		if(!ent->client->jetpack)
		{
			if(ent->waterlevel > 0)
				return;
			if(ent->client->pers.inventory[fuel_index] <= 0)
			{
				gi.cprintf(ent, PRINT_HIGH, "No fuel for jetpack\n" );
				return;
			}
		}
	}
#endif

	it->use (ent, it);
}

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
		// PMM - prevent scrolling through ALL weapons
//		index = (selected_weapon + i)%MAX_ITEMS;
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		// PMM - prevent scrolling through ALL weapons
//		if (cl->pers.weapon == it)
//			return;	// successful
		if (cl->newweapon == it)
			return;
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
		// PMM - prevent scrolling through ALL weapons
//		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		// PMM - prevent scrolling through ALL weapons
//		if (cl->pers.weapon == it)
//			return;	// successful
		if (cl->newweapon == it)
			return;
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
	if (ent->client->menu)
		PMenu_Close(ent);
	if (ent->client->textdisplay)
		Text_Close(ent);
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
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

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
		Com_strcat(text, sizeof(text), p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	Com_strcat(text, sizeof(text), "\n");

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

//======
//ROGUE
void Cmd_Ent_Count_f (edict_t *ent)
{
	int		x;
	edict_t	*e;

	x=0;

	for (e=g_edicts;e < &g_edicts[globals.num_edicts] ; e++)
	{
		if(e->inuse)
			x++;
	}

	gi.dprintf("%d entites active\n", x);
}
//ROGUE
//======

void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;

		Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "");
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			Com_sprintf(text+strlen(text), sizeof(text)-1-strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		Com_strcat(text, sizeof(text), st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}

void DrawBBox(edict_t *ent)
{
	vec3_t	p1, p2;
	vec3_t	origin;

	VectorCopy(ent->s.origin,origin);
	VectorSet(p1,origin[0]+ent->mins[0],origin[1]+ent->mins[1],origin[2]+ent->mins[2]);
	VectorSet(p2,origin[0]+ent->mins[0],origin[1]+ent->mins[1],origin[2]+ent->maxs[2]);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (p1);
	gi.WritePosition (p2);
	gi.multicast (p1, MULTICAST_ALL);
	VectorSet(p2,origin[0]+ent->mins[0],origin[1]+ent->maxs[1],origin[2]+ent->mins[2]);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (p1);
	gi.WritePosition (p2);
	gi.multicast (p1, MULTICAST_ALL);
	VectorSet(p2,origin[0]+ent->maxs[0],origin[1]+ent->mins[1],origin[2]+ent->mins[2]);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (p1);
	gi.WritePosition (p2);
	gi.multicast (p1, MULTICAST_ALL);

	VectorSet(p1,origin[0]+ent->maxs[0],origin[1]+ent->maxs[1],origin[2]+ent->mins[2]);
	VectorSet(p2,origin[0]+ent->maxs[0],origin[1]+ent->maxs[1],origin[2]+ent->maxs[2]);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (p1);
	gi.WritePosition (p2);
	gi.multicast (p1, MULTICAST_ALL);
	VectorSet(p2,origin[0]+ent->maxs[0],origin[1]+ent->mins[1],origin[2]+ent->mins[2]);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (p1);
	gi.WritePosition (p2);
	gi.multicast (p1, MULTICAST_ALL);
	VectorSet(p2,origin[0]+ent->mins[0],origin[1]+ent->maxs[1],origin[2]+ent->mins[2]);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (p1);
	gi.WritePosition (p2);
	gi.multicast (p1, MULTICAST_ALL);

	VectorSet(p1,origin[0]+ent->maxs[0],origin[1]+ent->mins[1],origin[2]+ent->maxs[2]);
	VectorSet(p2,origin[0]+ent->maxs[0],origin[1]+ent->mins[1],origin[2]+ent->mins[2]);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (p1);
	gi.WritePosition (p2);
	gi.multicast (p1, MULTICAST_ALL);
	VectorSet(p2,origin[0]+ent->maxs[0],origin[1]+ent->maxs[1],origin[2]+ent->maxs[2]);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (p1);
	gi.WritePosition (p2);
	gi.multicast (p1, MULTICAST_ALL);
	VectorSet(p2,origin[0]+ent->mins[0],origin[1]+ent->mins[1],origin[2]+ent->maxs[2]);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (p1);
	gi.WritePosition (p2);
	gi.multicast (p1, MULTICAST_ALL);

	VectorSet(p1,origin[0]+ent->mins[0],origin[1]+ent->maxs[1],origin[2]+ent->maxs[2]);
	VectorSet(p2,origin[0]+ent->mins[0],origin[1]+ent->maxs[1],origin[2]+ent->mins[2]);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (p1);
	gi.WritePosition (p2);
	gi.multicast (p1, MULTICAST_ALL);
	VectorSet(p2,origin[0]+ent->mins[0],origin[1]+ent->mins[1],origin[2]+ent->maxs[2]);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (p1);
	gi.WritePosition (p2);
	gi.multicast (p1, MULTICAST_ALL);
	VectorSet(p2,origin[0]+ent->maxs[0],origin[1]+ent->maxs[1],origin[2]+ent->maxs[2]);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (p1);
	gi.WritePosition (p2);
	gi.multicast (p1, MULTICAST_ALL);
}
void Cmd_Bbox_f (edict_t *ent)
{
	edict_t	*viewing;

	viewing = LookingAt(ent, 0, NULL, NULL);
	if(!viewing) return;
	DrawBBox(viewing);
}

void SetLazarusCrosshair (edict_t *ent)
{
	if (deathmatch->value || coop->value) return;
	if (!ent->inuse) return;
	if (!ent->client) return;
	if (ent->client->zoomed || ent->client->zooming)
		return;

	gi.cvar_forceset("lazarus_crosshair",      va("%d",(int)(crosshair->value)));
	gi.cvar_forceset("lazarus_cl_gun",         va("%d",(int)(cl_gun->value)));
}

void SetSensitivities (edict_t *ent,qboolean reset)
{
	char	string[512];

	if (deathmatch->value || coop->value) return;
	if (!ent->inuse) return;
	if (!ent->client) return;
	if (reset)
	{
#ifndef KMQUAKE2_ENGINE_MOD // engine has zoom autosensitivity
		gi.cvar_set ("m_pitch", va("%f", lazarus_pitch->value));
		gi.cvar_set ("m_yaw", va("%f", lazarus_yaw->value));
		gi.cvar_set ("joy_pitchsensitivity", va("%f", lazarus_joyp->value));
		gi.cvar_set ("joy_yawsensitivity", va("%f", lazarus_joyy->value));
	//	m_pitch->value              = lazarus_pitch->value;
	//	m_yaw->value                = lazarus_yaw->value;
	//	joy_pitchsensitivity->value = lazarus_joyp->value;
	//	joy_yawsensitivity->value   = lazarus_joyy->value;
#endif
		if (crosshair->value != lazarus_crosshair->value)
		{
			Com_sprintf(string, sizeof(string), "set crosshair %0d\n",(int)(lazarus_crosshair->value));
			stuffcmd(ent,string);
		}
		if (cl_gun->value != lazarus_cl_gun->value)
		{
			Com_sprintf(string, sizeof(string), "cl_gun %i\n",atoi(lazarus_cl_gun->string));
			stuffcmd(ent,string);
		}
		ent->client->pers.hand = hand->value;
	}
	else
	{
		float	ratio;

		//save in lazarus_crosshair
		Com_sprintf(string, sizeof(string), "lazarus_crosshair %i\n",atoi(crosshair->string));
		stuffcmd(ent,string);
		Com_sprintf(string, sizeof(string), "crosshair 0");
		stuffcmd(ent,string);

		Com_sprintf(string, sizeof(string), "lazarus_cl_gun %i\n",atoi(cl_gun->string));
		stuffcmd(ent,string);
		Com_sprintf(string, sizeof(string), "cl_gun 0");
		stuffcmd(ent,string);

		if (!ent->client->sensitivities_init)
		{
#ifndef KMQUAKE2_ENGINE_MOD // engine has zoom autosensitivity
			ent->client->m_pitch = m_pitch->value;
			ent->client->m_yaw   = m_yaw->value;
			ent->client->joy_pitchsensitivity = joy_pitchsensitivity->value;
			ent->client->joy_yawsensitivity   = joy_yawsensitivity->value;
#endif
			ent->client->sensitivities_init = true;
		}
		if (ent->client->ps.fov >= ent->client->original_fov)
			ratio = 1.;
		else
			ratio = ent->client->ps.fov / ent->client->original_fov;

#ifndef KMQUAKE2_ENGINE_MOD // engine has zoom autosensitivity
		gi.cvar_set ("m_pitch", va("%f", ent->client->m_pitch * ratio));
		gi.cvar_set ("m_yaw", va("%f", ent->client->m_yaw * ratio));
		gi.cvar_set ("joy_pitchsensitivity", va("%f", ent->client->joy_pitchsensitivity * ratio));
		gi.cvar_set ("joy_yawsensitivity", va("%f", ent->client->joy_yawsensitivity * ratio));
	//	m_pitch->value = ent->client->m_pitch * ratio;
	//	m_yaw->value   = ent->client->m_yaw * ratio;
	//	joy_pitchsensitivity->value = ent->client->joy_pitchsensitivity * ratio;
	//	joy_yawsensitivity->value   = ent->client->joy_yawsensitivity * ratio;
#endif
	}
#ifndef KMQUAKE2_ENGINE_MOD // engine has zoom autosensitivity
	Com_sprintf(string, sizeof(string), "m_pitch %g;m_yaw %g;joy_pitchsensitivity %g;joy_yawsensitivity %g\n",
		m_pitch->value,m_yaw->value,joy_pitchsensitivity->value,joy_yawsensitivity->value);
	stuffcmd(ent,string);
#endif
}

/*
=====================
Cmd_attack2_f
Alternate firing mode
=====================
*/
/*void Cmd_attack2_f(edict_t *ent, qboolean bOn)
{
	if(!ent->client) return;
	if(ent->health <= 0) return;

	if(bOn)
	{
		ent->client->pers.fire_mode=1;
		//ent->client->nNewLatch |= BUTTON_ATTACK2;
	}
	else
	{
		ent->client->pers.fire_mode=0;
		//ent->client->nNewLatch &= ~BUTTON_ATTACK2;
	}
}*/


void decoy_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	BecomeExplosion1(self);
}

void decoy_think(edict_t *self)
{
	if(self->s.frame < 0 || self->s.frame > 39)
	{
		self->s.frame = 0;
	}
	else
	{
		self->s.frame++;
		if(self->s.frame > 39)
			self->s.frame = 0;
	}

	// Every 2 seconds, make visible monsters mad at me
	if(level.framenum % 20 == 0)
	{
		edict_t	*e;
		int		i;

		for(i=game.maxclients+1; i<globals.num_edicts; i++)
		{
			e = &g_edicts[i];
			if(!e->inuse)
				continue;
			if(!(e->svflags & SVF_MONSTER))
				continue;
			if(e->monsterinfo.aiflags & AI_GOOD_GUY)
				continue;
			if(!visible(e,self))
				continue;
			if(e->enemy == self)
				continue;
			e->enemy = e->goalentity = self;
			e->monsterinfo.aiflags |= AI_TARGET_ANGER;
			FoundTarget (e);
		}
	}

	self->nextthink = level.time + FRAMETIME;
	gi.linkentity(self);
}

void forcewall_think(edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_FORCEWALL);
	gi.WritePosition (self->pos1);
	gi.WritePosition (self->pos2);
	gi.WriteByte  (self->style);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	self->nextthink = level.time + FRAMETIME;
}

void SpawnForcewall(edict_t	*player)
{
	edict_t  *wall;
	vec3_t	forward, point, start;
	trace_t  tr;
	
	wall = G_Spawn();
	VectorCopy(player->s.origin,start);
	start[2] += player->viewheight;
	AngleVectors(player->client->v_angle,forward,NULL,NULL);
	VectorMA(start, WORLD_SIZE, forward, point);	// was 8192
	tr = gi.trace(start,NULL,NULL,point,player,MASK_SOLID);
	VectorCopy(tr.endpos,wall->s.origin);
	
	if(fabs(forward[0]) > fabs(forward[1]))
	{
		wall->pos1[0] = wall->pos2[0] = wall->s.origin[0];
		wall->mins[0] =  -1;
		wall->maxs[0] =   1;
		
		VectorCopy(wall->s.origin,point);
		point[1] -= WORLD_SIZE;	// was 8192
		tr = gi.trace(wall->s.origin,NULL,NULL,point,NULL,MASK_SOLID);
		wall->pos1[1] = tr.endpos[1];
		wall->mins[1] = wall->pos1[1] - wall->s.origin[1];
		
		point[1] = wall->s.origin[1] + WORLD_SIZE;	// was 8192
		tr = gi.trace(wall->s.origin,NULL,NULL,point,NULL,MASK_SOLID);
		wall->pos2[1] = tr.endpos[1];
		wall->maxs[1] = wall->pos2[1] - wall->s.origin[1];
	}
	else
	{
		VectorCopy(wall->s.origin,point);
		point[0] -= WORLD_SIZE;	// was 8192
		tr = gi.trace(wall->s.origin,NULL,NULL,point,NULL,MASK_SOLID);
		wall->pos1[0] = tr.endpos[0];
		wall->mins[0] = wall->pos1[0] - wall->s.origin[0];
		
		point[0] = wall->s.origin[0] + WORLD_SIZE;	// was 8192
		tr = gi.trace(wall->s.origin,NULL,NULL,point,NULL,MASK_SOLID);
		wall->pos2[0] = tr.endpos[0];
		wall->maxs[0] = wall->pos2[0] - wall->s.origin[0];
		
		wall->pos1[1] = wall->pos2[1] = wall->s.origin[1];
		wall->mins[1] = -1;
		wall->maxs[1] =  1;
	}
	wall->mins[2] = 0;
	
	VectorCopy(wall->s.origin,point);
	point[2] = wall->s.origin[2] + WORLD_SIZE;	// was 8192
	tr = gi.trace(wall->s.origin,NULL,NULL,point,NULL,MASK_SOLID);
	wall->maxs[2] = tr.endpos[2] - wall->s.origin[2];
	wall->pos1[2] = wall->pos2[2] = tr.endpos[2];
	
	wall->style = 208;	// Color from Q2 palette
	wall->movetype = MOVETYPE_NONE;
	wall->solid = SOLID_BBOX;
	wall->clipmask = MASK_PLAYERSOLID | MASK_MONSTERSOLID;
	wall->think = forcewall_think;
	wall->nextthink = level.time + FRAMETIME;
	wall->svflags = SVF_NOCLIENT;
	wall->classname = "forcewall";
	wall->activator = player;
	wall->owner = player;
	gi.linkentity(wall);
}

void ForcewallOff(edict_t *player)
{
	vec3_t	forward, point, start;
	trace_t  tr;
	
	VectorCopy(player->s.origin,start);
	start[2] += player->viewheight;
	AngleVectors(player->client->v_angle,forward,NULL,NULL);
	VectorMA(start, WORLD_SIZE, forward, point);	// was 8192
	tr = gi.trace(start,NULL,NULL,point,player,MASK_SHOT);
	if(Q_stricmp(tr.ent->classname,"forcewall"))
	{
		gi.cprintf(player,PRINT_HIGH,"Not a forcewall!\n");
		return;
	}
	if(tr.ent->activator != player)
	{
		gi.cprintf(player,PRINT_HIGH,"You don't own this forcewall, bub!\n");
		return;
	}
	G_FreeEdict(tr.ent);
}

/*
=================
ClientCommand
=================
*/
void Restart_FMOD(edict_t *self)
{
	FMOD_Init();
	G_FreeEdict(self);
}
void ClientCommand (edict_t *ent)
{
	char	*cmd;
	char	*parm;

	if (!ent->client)
		return;		// not fully in game yet

	cmd = gi.argv(0);
	if(gi.argc() < 2)
		parm = NULL;
	else
		parm = gi.argv(1);

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
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
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

	if (Q_stricmp (cmd, "use") == 0)
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
	else if (Q_stricmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);
	// ==================== fog stuff =========================
//	else if (developer->value && !Q_stricmp(cmd,"fog"))
	else if (Q_stricmp(cmd,"fog") == 0)
		Cmd_Fog_f(ent);
	else if (developer->value && !Q_strncasecmp(cmd, "fog_", 4))
		Cmd_Fog_f(ent);
	// ================ end fog stuff =========================

//CHASECAM
	else if (Q_stricmp (cmd, "thirdperson") == 0)
            Cmd_Chasecam_Toggle (ent);
	else if (Q_stricmp(cmd, "killtrap") == 0)
		Cmd_KillTrap_f(ent);
	else if (Q_stricmp (cmd, "entcount") == 0)		// PGM
		Cmd_Ent_Count_f (ent);						// PGM
	else if (Q_stricmp (cmd, "disguise") == 0)		// PGM
	{
		ent->flags |= FL_DISGUISED;
	}

	// alternate attack mode
	/*else if (!Q_stricmp(cmd,"attack2_off"))
		Cmd_attack2_f(ent,false);
	else if (!Q_stricmp(cmd,"attack2_on"))
		Cmd_attack2_f(ent,true);*/
	// zoom
	else if (!Q_stricmp(cmd, "zoomin"))
	{
		if(!deathmatch->value && !coop->value && !ent->client->chasetoggle)
		{
			if(ent->client->ps.fov > 5)
			{
				if(cl_gun->value)
					stuffcmd(ent,"cl_gun 0\n");
				ent->client->frame_zoomrate = zoomrate->value * ent->client->secs_per_frame;
				ent->client->zooming = 1;
				ent->client->zoomed = true;
			}
		}
	}
	else if (!Q_stricmp(cmd, "zoomout"))
	{
		if(!deathmatch->value && !coop->value && !ent->client->chasetoggle)
		{
			if(ent->client->ps.fov < ent->client->original_fov)
			{
				if(cl_gun->value)
					stuffcmd(ent,"cl_gun 0\n");
				ent->client->frame_zoomrate = zoomrate->value * ent->client->secs_per_frame;
				ent->client->zooming = -1;
				ent->client->zoomed = true;
			}
		}
	}
	else if (!Q_stricmp(cmd, "zoom"))
	{
		if(!deathmatch->value && !coop->value && !ent->client->chasetoggle)
		{
			if(!parm)
			{
				gi.dprintf("syntax: zoom [0/1]  (0=off, 1=on)\n");
			}
			else if (!atoi(parm))
			{
				ent->client->ps.fov = ent->client->original_fov;
				ent->client->zooming = 0;
				ent->client->zoomed = false;
				SetSensitivities(ent,true);
			}
			else if (!ent->client->zoomed && !ent->client->zooming)
			{
				ent->client->ps.fov = zoomsnap->value;
				ent->client->pers.hand = 2;
				if(cl_gun->value)
					stuffcmd(ent,"cl_gun 0\n");
				ent->client->zooming = 0;
				ent->client->zoomed = true;
				SetSensitivities(ent,false);
			}
		}
	}
	else if (!Q_stricmp(cmd, "zoomoff"))
	{
		if(!deathmatch->value && !coop->value && !ent->client->chasetoggle)
		{
			if(ent->client->zoomed && !ent->client->zooming)
			{
				ent->client->ps.fov = ent->client->original_fov;
				ent->client->zooming = 0;
				ent->client->zoomed = false;
				SetSensitivities(ent,true);
			}
		}
	}
	else if (!Q_stricmp(cmd, "zoomon"))
	{
		if(!deathmatch->value && !coop->value && !ent->client->chasetoggle)
		{
			if(!ent->client->zoomed && !ent->client->zooming)
			{
				ent->client->ps.fov = zoomsnap->value;
				ent->client->pers.hand = 2;
				if(cl_gun->value)
					stuffcmd(ent,"cl_gun 0\n");
				ent->client->zooming = 0;
				ent->client->zoomed = true;
				SetSensitivities(ent,false);
			}
		}
	}
	else if (!Q_stricmp(cmd, "zoominstop"))
	{
		if(!deathmatch->value && !coop->value && !ent->client->chasetoggle)
		{
			if(ent->client->zooming > 0)
			{
				ent->client->zooming = 0;
				if(ent->client->ps.fov == ent->client->original_fov)
				{
					ent->client->zoomed = false;
					SetSensitivities(ent,true);
				}
				else
				{
					gi.cvar_forceset("zoomsnap",va("%f",ent->client->ps.fov));
					SetSensitivities(ent,false);
				}
			}
		}
	}
	else if (!Q_stricmp(cmd, "zoomoutstop"))
	{
		if(!deathmatch->value && !coop->value && !ent->client->chasetoggle)
		{
			if(ent->client->zooming < 0)
			{
				ent->client->zooming = 0;
				if(ent->client->ps.fov == ent->client->original_fov)
				{
					ent->client->zoomed = false;
					SetSensitivities(ent,true);
				}
				else
				{
					gi.cvar_forceset("zoomsnap",va("%f",ent->client->ps.fov));
					SetSensitivities(ent,false);
				}
			}
		}
	}
#ifndef DISABLE_FMOD
	else if(!Q_stricmp(cmd, "playsound"))
	{
		vec3_t	pos = {0, 0, 0};
		vec3_t	vel = {0, 0, 0};
		if(s_primary->value)
		{
			gi.dprintf("target_playback requires s_primary be set to 0.\n"
				       "At the console type:\n"
					   "s_primary 0;sound_restart\n");
			return;
		}
		if(parm)
		{
			edict_t *temp;

			strlwr(parm);
			temp = G_Spawn();
			temp->message = parm;
			temp->volume = 255;

			if( strstr(parm,".mod") ||
				strstr(parm,".s3m") ||
				strstr(parm,".xm")  ||
				strstr(parm,".mid")   )
				temp->spawnflags |= 8;

			FMOD_PlaySound(temp);
			G_FreeEdict(temp);
		}
		else
			gi.dprintf("syntax: playsound <soundfile>, path relative to gamedir\n");
	}
	else if(!Q_stricmp(cmd,"sound_restart"))
	{
		// replacement for snd_restart to get around DirectSound/FMOD problem
		edict_t	*temp;
		FMOD_Shutdown();
		stuffcmd(ent,"snd_restart\n");
		temp = G_Spawn();
		temp->think = Restart_FMOD;
		temp->nextthink = level.time + 2;
	}
#endif // DISABLE_FMOD
	else if(!Q_stricmp(cmd,"hud"))
	{
		if(parm)
		{
			int	state = atoi(parm);

			if(state)
				Hud_On();
			else
				Hud_Off();
		}
		else
			Cmd_ToggleHud();
	}
	else if(!Q_stricmp(cmd,"whatsit"))
	{
		if(parm)
		{
			int state = atoi(parm);
			if(state)
				world->effects |= FX_WORLDSPAWN_WHATSIT;
			else
				world->effects &= ~FX_WORLDSPAWN_WHATSIT;
		}
		else
			world->effects ^= FX_WORLDSPAWN_WHATSIT;
	}
	else if (!Q_stricmp(cmd,"bbox"))
		Cmd_Bbox_f (ent);
	else if(!Q_stricmp(cmd,"forcewall"))
		SpawnForcewall(ent);
	else if(!Q_stricmp(cmd,"forcewall_off"))
		ForcewallOff(ent);
	else if (!Q_stricmp(cmd,"freeze"))
	{
		if(level.freeze)
			level.freeze = false;
		else
		{
			if(ent->client->jetpack)
				gi.dprintf("Cannot use freeze while using jetpack\n");
			else
				level.freeze = true;
		}
	}
	else if(developer->value)
	{
	//	if (!Q_stricmp(cmd,"lightswitch"))
	//		ToggleLights();
	}
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}
