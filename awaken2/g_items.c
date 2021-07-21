// g_items.c

#include "g_local.h"

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

qboolean Pickup_Weapon(edict_t *ent, edict_t *other);
void Use_Weapon(edict_t *ent, gitem_t *inv);
void Drop_Weapon(edict_t *ent, gitem_t *inv);
void Use_Quad(edict_t *ent, gitem_t *item);

void Weapon_RocketLauncher(edict_t *ent);
void Weapon_Railgun(edict_t *ent);

//CW++
void Weapon_Chainsaw(edict_t *self);
void Weapon_DesertEagle(edict_t *self);
void Weapon_Jackhammer(edict_t *self);
void Weapon_Mac10(edict_t *self);
void Weapon_GaussPistol(edict_t *self);
void Weapon_Trap(edict_t *self);
void Weapon_C4(edict_t *self);
void Weapon_ESG(edict_t *self);
void Weapon_Flamethrower(edict_t *self);
void Weapon_ShockRifle(edict_t *self);
void Weapon_AGM(edict_t *self);
void Weapon_DiscLauncher(edict_t *self);
//CW--

gitem_armor_t jacketarmor_info	= { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {100, 200, .80, .60, ARMOR_BODY};

int	jacket_armor_index;
int	combat_armor_index;
int	body_armor_index;
int	power_screen_index;
int	power_shield_index;
// Knightmare added
int	shells_index;
int	bullets_index;
//int	grenades_index;
int	c4_index;
int	rockets_index;
int	cells_index;
int	slugs_index;
int	traps_index;
// end Knightmare

static int	quad_drop_timeout_hack;


//======================================================================

/*
===============
GetItemByIndex
===============
*/
gitem_t	*GetItemByIndex(int index)
{
	if ((index == 0) || (index >= game.num_items))
		return NULL;

	return &itemlist[index];
}

// Knightmare added
/*
===============
GetMaxAmmoByIndex
===============
*/
int GetMaxAmmoByIndex (gclient_t *client, int item_index)
{
	int value;

	if (!client)
		return 0;

	if (item_index == shells_index)
		value = client->pers.max_shells;
	else if (item_index == bullets_index)
		value = client->pers.max_bullets;
//	else if (item_index == grenades_index)
//		value = client->pers.max_grenades;
	else if (item_index == c4_index)
		value = client->pers.max_c4;
	else if (item_index == rockets_index)
		value = client->pers.max_rockets;
	else if (item_index == cells_index)
		value = client->pers.max_cells;
	else if (item_index == slugs_index)
		value = client->pers.max_slugs;
	else if (item_index == traps_index)
		value = client->pers.max_traps;
	else
		value = 0;

	return value;
}

/*
===============
GetMaxArmorByIndex
===============
*/
int GetMaxArmorByIndex (int item_index)
{
	int value;

	if (item_index == jacket_armor_index)
		value = jacketarmor_info.max_count;
	else if (item_index == combat_armor_index)
		value = combatarmor_info.max_count;
	else if (item_index == body_armor_index)
		value = bodyarmor_info.max_count;
	else
		value = 0;

	return value;
}
// end Knightmare

/*
===============
FindItemByClassname

===============
*/
gitem_t	*FindItemByClassname(char *classname)
{
	gitem_t	*it;
	int		i;

	it = itemlist;
	for (i = 0; i < game.num_items; i++, it++)
	{
		if (!it->classname)
			continue;

		if (!Q_stricmp(it->classname, classname))
			return it;
	}

	return NULL;
}

/*
===============
FindItem

===============
*/
gitem_t	*FindItem(char *pickup_name)
{
	gitem_t	*it;
	int		i;

	it = itemlist;
	for (i = 0; i < game.num_items; ++i, ++it)
	{
		if (!it->pickup_name)
			continue;

		if (!Q_stricmp(it->pickup_name, pickup_name))
			return it;
	}

	return NULL;
}

//======================================================================

void DoRespawn(edict_t *ent)
{
	if (ent->team)
	{
		edict_t	*master;
		int	count;
		int choice;

		master = ent->teammaster;

//ZOID++
//		In CTF, when weapons-stay is on, only the master of a team of weapons is spawned.

		if (((int)sv_gametype->value > G_FFA) && ((int)dmflags->value & DF_WEAPONS_STAY) && master->item && (master->item->flags & IT_WEAPON)) //CW
			ent = master;
		else
		{
//ZOID--
			for (count = 0, ent = master; ent; ent = ent->chain, count++)
				;

			choice = rand() % count;
			for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
				;
		}
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	gi.linkentity(ent);

//Maj++
	if (ent->classname[0] == 'R')
		return;
//Maj--

	// send an effect
	ent->s.event = EV_ITEM_RESPAWN;
}

void SetRespawn(edict_t *ent, float delay)
{
//CW++
//	Items (usually powerups) that have been spawned randomly are not to be respawned.

	if (ent->rnd_spawn)
		return;
//CW--

	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->nextthink = level.time + delay;
	ent->think = DoRespawn;
	gi.linkentity(ent);
}


//======================================================================

qboolean Pickup_Powerup(edict_t *ent, edict_t *other)
{
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;											//CW

	if (!(ent->spawnflags & DROPPED_ITEM))
	{
//CW++
		if (ent->delay)
			SetRespawn(ent, ent->delay);
		else
//CW--
			SetRespawn(ent, ent->item->quantity);
	}

	if (((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
	{
		if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
			quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
//CW++
		if (ent->item->use != Use_Teleporter)
//CW--
			ent->item->use(other, ent->item);
	}

	return true;
}

void Drop_General(edict_t *ent, gitem_t *item)
{
	Drop_Item(ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);
}


//======================================================================

qboolean Pickup_Adrenaline(edict_t *ent, edict_t *other)
{
	if (other->health < other->max_health)
		other->health = other->max_health;

	if (!(ent->spawnflags & DROPPED_ITEM))															//CW
	{
//CW++
		if (ent->delay)
			SetRespawn(ent, ent->delay);
		else
//CW--
			SetRespawn(ent, ent->item->quantity);
	}

//CW++
//	Extinguish flame if on fire.

	if (other->burning)
	{
		other->burning = false;
		if (other->flame)				// sanity check
		{
			other->flame->touch = NULL;
			other->flame->think = Flame_Expire;
			other->flame->nextthink = level.time + FRAMETIME;
		}
	}
//CW--

	return true;
}

qboolean Pickup_AncientHead(edict_t *ent, edict_t *other)
{
	other->max_health += 2;

	if (!(ent->spawnflags & DROPPED_ITEM))															//CW
	{
//CW++
		if (ent->delay)
			SetRespawn(ent, ent->delay);
		else
//CW--
			SetRespawn(ent, ent->item->quantity);
	}

	return true;
}

qboolean Pickup_Bandolier(edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	if (other->client->pers.max_bullets < (int)sv_max_band_bullets->value)							//CW...
		other->client->pers.max_bullets = (int)sv_max_band_bullets->value;

	if (other->client->pers.max_shells < (int)sv_max_band_shells->value)
		other->client->pers.max_shells = (int)sv_max_band_shells->value;

	if (other->client->pers.max_cells < (int)sv_max_band_cells->value)
		other->client->pers.max_cells = (int)sv_max_band_cells->value;

	if (other->client->pers.max_slugs < (int)sv_max_band_slugs->value)
		other->client->pers.max_slugs = (int)sv_max_band_slugs->value;

	if ((int)sv_allow_bullets->value)
	{
//CW--
		item = FindItem("bullets");
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
				other->client->pers.inventory[index] = other->client->pers.max_bullets;
		}
	}

//CW++
	if ((int)sv_allow_shells->value)
	{
//CW--
		item = FindItem("shells");
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_shells)
				other->client->pers.inventory[index] = other->client->pers.max_shells;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM))
	{
//CW++
		if (ent->delay)
			SetRespawn(ent, ent->delay);
		else
//CW--
			SetRespawn(ent, ent->item->quantity);
	}

	return true;
}

qboolean Pickup_Pack(edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	if (other->client->pers.max_bullets < (int)sv_max_pack_bullets->value)							//CW...
		other->client->pers.max_bullets = (int)sv_max_pack_bullets->value;

	if (other->client->pers.max_shells < (int)sv_max_pack_shells->value)
		other->client->pers.max_shells = (int)sv_max_pack_shells->value;

	if (other->client->pers.max_rockets < (int)sv_max_pack_rockets->value)
		other->client->pers.max_rockets = (int)sv_max_pack_rockets->value;

	if (other->client->pers.max_cells < (int)sv_max_pack_cells->value)
		other->client->pers.max_cells = (int)sv_max_pack_cells->value;

	if (other->client->pers.max_slugs < (int)sv_max_pack_slugs->value)
		other->client->pers.max_slugs = (int)sv_max_pack_slugs->value;
//CW++
	if (other->client->pers.max_c4 < (int)sv_max_pack_c4->value)
		other->client->pers.max_c4 = (int)sv_max_pack_c4->value;

	if (other->client->pers.max_traps < (int)sv_max_pack_traps->value)
		other->client->pers.max_traps = (int)sv_max_pack_traps->value;

	if ((int)sv_allow_bullets->value)
	{
//CW--
		item = FindItem("bullets");
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
				other->client->pers.inventory[index] = other->client->pers.max_bullets;
		}
	}

//CW++
	if ((int)sv_allow_shells->value)
	{
//CW--
		item = FindItem("shells");
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_shells)
				other->client->pers.inventory[index] = other->client->pers.max_shells;
		}
	}

//CW++
	if ((int)sv_allow_cells->value)
	{
//CW--
		item = FindItem("cells");
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_cells)
				other->client->pers.inventory[index] = other->client->pers.max_cells;
		}
	}

//CW++
	if ((int)sv_allow_rockets->value)
	{
//CW--
		item = FindItem("rockets");
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
				other->client->pers.inventory[index] = other->client->pers.max_rockets;
		}
	}

//CW++
	if ((int)sv_allow_slugs->value)
	{
//CW--
		item = FindItem("slugs");
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_slugs)
				other->client->pers.inventory[index] = other->client->pers.max_slugs;
		}
	}

//CW++

	if ((int)sv_allow_traps->value)
	{
		item = FindItem("traps");
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_traps)
				other->client->pers.inventory[index] = other->client->pers.max_traps;
		}
	}

	if ((int)sv_allow_c4->value)
	{
		item = FindItem("c4");
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_c4)
				other->client->pers.inventory[index] = other->client->pers.max_c4;
		}
	}
//CW--

	if (!(ent->spawnflags & DROPPED_ITEM))															//CW
	{
//CW++
		if (ent->delay)
			SetRespawn(ent, ent->delay);
		else
//CW--
			SetRespawn(ent, ent->item->quantity);
	}

	return true;
}

//======================================================================

void Use_Quad(edict_t *ent, gitem_t *item)
{
	int timeout;

//CW++
//	Sanity check.

	if (!ent->client)
	{
		gi.dprintf("BUG: Use_Quad() called for non-client entity.\nPlease contact musashi@planetquake.com\n");
		return;
	}
//CW--

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (quad_drop_timeout_hack)
	{
		timeout = quad_drop_timeout_hack;
		quad_drop_timeout_hack = 0;
	}
	else
		timeout = QUAD_TIMEOUT_FRAMES;	// was 300

	if (ent->client->quad_framenum > level.framenum)
		ent->client->quad_framenum += timeout;
	else
		ent->client->quad_framenum = level.framenum + timeout;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Breather(edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->breather_framenum > level.framenum)
		ent->client->breather_framenum += BREATHER_TIMEOUT_FRAMES;	// was 300
	else
		ent->client->breather_framenum = level.framenum + BREATHER_TIMEOUT_FRAMES;	// was 300
}

//======================================================================

void Use_Envirosuit(edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->enviro_framenum > level.framenum)
		ent->client->enviro_framenum += ENVIROSUIT_TIMEOUT_FRAMES;	// was 300
	else
		ent->client->enviro_framenum = level.framenum + ENVIROSUIT_TIMEOUT_FRAMES;	// was 300

//CW++
//	Extinguish flame if on fire.

	if (ent->burning)
	{
		ent->burning = false;
		if (ent->flame)				// sanity check
		{
			ent->flame->touch = NULL;
			ent->flame->think = Flame_Expire;
			ent->flame->nextthink = level.time + FRAMETIME;
		}
	}
//CW--
}

//======================================================================

void Use_Invulnerability(edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum += INVINCIBLE_TIMEOUT_FRAMES;	// was 300
	else
		ent->client->invincible_framenum = level.framenum + INVINCIBLE_TIMEOUT_FRAMES;	// was 300

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);

//CW++
//	Extinguish flame if on fire.

	if (ent->burning)
	{
		ent->burning = false;
		if (ent->flame)				// sanity check
		{
			ent->flame->touch = NULL;
			ent->flame->think = Flame_Expire;
			ent->flame->nextthink = level.time + FRAMETIME;
		}
	}
//CW--
}

//======================================================================

void Use_Silencer(edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);
	ent->client->silencer_shots += SILENCER_SHOTS;	// was 30
}

//======================================================================

//CW++
void Use_Siphon(edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (ent->client->siphon_framenum > level.framenum)
		ent->client->siphon_framenum += SIPHON_TIMEOUT_FRAMES;	// was 300
	else
		ent->client->siphon_framenum = level.framenum + SIPHON_TIMEOUT_FRAMES;	// was 300

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/siphon.wav"), 1, ATTN_NORM, 0);
}

void Use_Needle(edict_t *ent, gitem_t *item)
{
	int remaining;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	remaining = ent->client->needle_framenum - level.framenum;
	if (remaining <= 0)
		ent->client->needle_framenum = level.framenum + NEEDLE_TIMEOUT_FRAMES;	// was 600
	else if (remaining <= 600)
		ent->client->needle_framenum = level.framenum + NEEDLE_TIMEOUT_FRAMES + (int)(remaining * 0.5);	// was 600
	else
		T_Damage(ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, (int)(0.1 * remaining), 0, DAMAGE_NO_ARMOR, MOD_D89);

	gi.sound(ent, CHAN_ITEM, gi.soundindex("parasite/paratck4.wav"), 1, ATTN_NORM, 0);
}

void Use_Haste(edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (ent->client->haste_framenum > level.framenum)
		ent->client->haste_framenum += HASTE_TIMEOUT_FRAMES;	// was 300
	else
		ent->client->haste_framenum = level.framenum + HASTE_TIMEOUT_FRAMES;	// was 300

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/haste.wav"), 1, ATTN_NORM, 0);
}

void Use_AntiBeam(edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

	if (ent->client->antibeam_framenum > level.framenum)
		ent->client->antibeam_framenum += ANTIBEAM_TIMEOUT_FRAMES;	// was 300
	else
		ent->client->antibeam_framenum = level.framenum + ANTIBEAM_TIMEOUT_FRAMES;	// was 300

	gi.sound(ent, CHAN_ITEM, gi.soundindex("ctf/tech1.wav"), 1, ATTN_NORM, 0);
}

void Use_Teleporter(edict_t *ent, gitem_t *item)
{
	edict_t	*spot = NULL;
	edict_t *e_check;
	edict_t	*t_splash;
	int		i;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem(ent);

//	Find a suitable spawn point.

	if (sv_gametype->value == G_CTF)
		spot = SelectCTFSpawnPoint(ent, true);
	else if (sv_gametype->value == G_ASLT)
		spot = SelectASLTSpawnPoint(ent);
	else
		spot = SelectDeathmatchSpawnPoint();

	if (spot == NULL)
	{
		spot = G_Find(spot, FOFS(classname), "info_player_start");
		if (spot == NULL)
		{
			gi.dprintf("BUG: Couldn't find P/T spawn point for %s\n", ent->client->pers.netname);
			T_Damage(ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_UNKNOWN);
			return;
		}
	}

//	Teleporting players in CTF games can't take the flag with them.

	if (sv_gametype->value == G_CTF)
		CTFDeadDropFlag(ent);

	CTFPlayerResetGrapple(ent);

//	Break Trap tractor beam(s) if the player is held (and flagged to do so).

	if (ent->tractored && !((int)sv_trap_thru_tele->value))
	{
		ent->tractored = false;
		for (i = 0; i < globals.num_edicts; ++i)
		{
			e_check = &g_edicts[i];
			if (!e_check->inuse)
				continue;
			if (e_check->client)
				continue;
			if (e_check->wep_proj)
				continue;

			if ((e_check->die == Trap_DieFromDamage) && (e_check->enemy == ent))
			{
				e_check->think = Trap_Die;
				e_check->nextthink = level.time + FRAMETIME;
			}
		}
	}

//	Break AGM stream if the player is being manipulated.

	if (ent->client->agm_enemy != NULL)
	{
		ent->client->thrown_by_agm = false;
		ent->client->flung_by_agm = false;
		ent->client->held_by_agm = false;
		
		ent->client->agm_enemy->client->agm_on = false;
		ent->client->agm_enemy->client->agm_push = false;
		ent->client->agm_enemy->client->agm_pull = false;
		ent->client->agm_enemy->client->agm_charge = 0;
		ent->client->agm_enemy->client->agm_target = NULL;
		ent->client->agm_enemy = NULL;
	}

//	Snuff out the flame if the player is on fire (and flagged to do so).

	if (ent->burning && !((int)sv_flame_thru_tele->value))
	{
		ent->burning = false;
		if (ent->flame)
		{
			ent->flame->touch = NULL;
			Flame_Expire(ent->flame);
		}
	}

//	Spawn a temporary entitity to draw a teleport splash at the player's current position.

	t_splash = G_Spawn();
	VectorCopy(ent->s.origin, t_splash->s.origin);	
	t_splash->svflags &= ~SVF_NOCLIENT;
	t_splash->think = G_FreeEdict;
	t_splash->nextthink = level.time + 1.0;
	gi.linkentity(t_splash);

	t_splash->s.event = EV_PLAYER_TELEPORT;
	gi.sound(t_splash, CHAN_ITEM, gi.soundindex("misc/tele_up.wav"), 1, ATTN_IDLE, 0);

//	Unlink to make sure they can't possibly interfere with the KillBox, then move them
//	to their new location.

	gi.unlinkentity(ent);

	VectorCopy(spot->s.origin, ent->s.origin);
	VectorCopy(spot->s.origin, ent->s.old_origin);
	ent->s.origin[2] += 10.0;

//	Clear their velocity and hold them in place briefly. Draw a teleport splash on the 
//	player at their new location.

	VectorClear(ent->velocity);
	VectorClear(ent->client->oldvelocity);
	ent->client->ps.pmove.pm_time = 160>>3;
	ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
	ent->s.event = EV_PLAYER_TELEPORT;

//	Set angles.

	for (i = 0; i < 3; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spot->s.angles[i] - ent->client->resp.cmd_angles[i]);

	VectorClear(ent->s.angles);
	VectorClear(ent->client->ps.viewangles);
	VectorClear(ent->client->v_angle);

//	Kill anything at the destination.

	KillBox(ent);

	gi.linkentity(ent);
}

#define	M_INVULN	0
#define	M_QUAD		1
#define	M_D89		2
#define	M_HASTE		3
#define	M_SIPHON	4
#define	M_BEAM		5
#define	M_ENVIRO	6
#define	M_TELE		7

#define M_NUMITEMS	8

qboolean Pickup_Mystery(edict_t *ent, edict_t *other)
{
	gitem_t	*surprise = NULL;
	float	r;
	float	p[M_NUMITEMS];
	float	p_r[M_NUMITEMS];
	float	p_total = 0;
	int		i;

	if (!(ent->spawnflags & DROPPED_ITEM))
	{
		if (ent->delay)
			SetRespawn(ent, ent->delay);
		else
			SetRespawn(ent, ent->item->quantity);
	}

//	Setup custom probabilities.

	p[M_INVULN] = sv_mystery_invuln->value;
	p[M_QUAD] = sv_mystery_quad->value;
	p[M_D89] = sv_mystery_d89->value;
	p[M_HASTE] = sv_mystery_haste->value;
	p[M_SIPHON] = sv_mystery_siphon->value;
	p[M_BEAM] = sv_mystery_antibeam->value;
	p[M_ENVIRO] = sv_mystery_enviro->value;
	p[M_TELE] = sv_mystery_tele->value;

	for (i = 0; i < M_NUMITEMS; ++i)
		p_total += p[i];

	if (p_total == 0)
		p_total = 1;

	for (i = 0; i < M_NUMITEMS; i++)
		p_r[i] = (p[i] / p_total) + ((i > 0)?p_r[i-1]:0.0);

//	Randomly determine what the Mystery Box item is.

	r = random();

	if (r < p_r[M_INVULN])
		surprise = FindItem("Invulnerability");
	else if (r < p_r[M_QUAD])
		surprise = FindItem("Quad Damage");
	else if (r < p_r[M_D89])
		surprise = FindItem("Awakening");
	else if (r < p_r[M_HASTE])
		surprise = FindItem("D89");
	else if (r < p_r[M_SIPHON])
		surprise = FindItem("Haste");
	else if (r < p_r[M_BEAM])
		surprise = FindItem("Beam Reflector");
	else if (r < p_r[M_ENVIRO])
		surprise = FindItem("Environment Suit");
	else if (r < p_r[M_TELE])
		surprise = FindItem("Teleporter");

	if (surprise != NULL)
	{
		if (surprise->tag == POWERUP_INVULN)
			surprise->classname = "item_invulnerability";
		if (surprise->tag == POWERUP_QUAD)
			surprise->classname = "item_quad";

		gi_centerprintf(other, "Mystery Item:  %s", surprise->pickup_name);
		other->client->pers.inventory[ITEM_INDEX(surprise)]++;
		
		if (surprise->use != Use_Teleporter)
			surprise->use(other, surprise);
		return true;
	}
	else
		return false;
}
//======================================================================
//CW--

qboolean Pickup_Key(edict_t *ent, edict_t *other)
{
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;											//CW
	return true;
}

//======================================================================

qboolean Add_Ammo(edict_t *ent, gitem_t *item, int count)
{
	int	index;
	int	max;

	if (!ent->client)
		return false;

	if (item->tag == AMMO_BULLETS)
		max = ent->client->pers.max_bullets;
	else if (item->tag == AMMO_SHELLS)
		max = ent->client->pers.max_shells;
	else if (item->tag == AMMO_ROCKETS)
		max = ent->client->pers.max_rockets;
	else if (item->tag == AMMO_C4)
		max = ent->client->pers.max_c4;
	else if (item->tag == AMMO_CELLS)
		max = ent->client->pers.max_cells;
	else if (item->tag == AMMO_SLUGS)
		max = ent->client->pers.max_slugs;
//CW++
	else if (item->tag == AMMO_TRAPS)
		max = ent->client->pers.max_traps;
//CW--
	else
		return false;

	index = ITEM_INDEX(item);

	if (ent->client->pers.inventory[index] == max)
		return false;

	ent->client->pers.inventory[index] += count;

//Pon++
	if ((int)chedit->value && (ent == &g_edicts[1]))
		ent->client->pers.inventory[index] = 0;
//Pon--

	if (ent->client->pers.inventory[index] > max)
		ent->client->pers.inventory[index] = max;

	return true;
}

qboolean Pickup_Ammo(edict_t *ent, edict_t *other)
{
	int			count;
	qboolean	weapon;

	weapon = (ent->item->flags & IT_WEAPON);
	if (weapon && ((int)dmflags->value & DF_INFINITE_AMMO))
		count = 1000;
	else if (ent->count)
		count = ent->count;
	else
		count = ent->item->quantity;

	if (!Add_Ammo(other, ent->item, count))
		return false;

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))									//CW
	{
//CW++
		if (ent->delay)
			SetRespawn(ent, ent->delay);
		else
//CW--
			SetRespawn(ent, 30);
	}

	return true;
}

void Drop_Ammo(edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	int		index;

	index = ITEM_INDEX(item);
	dropped = Drop_Item(ent, item);
	if (ent->client->pers.inventory[index] >= item->quantity)
		dropped->count = item->quantity;
	else
		dropped->count = ent->client->pers.inventory[index];

	ent->client->pers.inventory[index] -= dropped->count;
	ValidateSelectedItem(ent);
}


//======================================================================

void MegaHealth_think(edict_t *self)
{
	if ((self->owner->health > self->owner->max_health) && !CTFHasRegeneration(self->owner))		//ZOID
	{
		self->nextthink = level.time + 1.0;
		self->owner->health -= 1;
		return;
	}

	if (!(self->spawnflags & DROPPED_ITEM))															//CW
	{
//CW++
		if (self->delay)
			SetRespawn(self, self->delay);
		else
//CW--
			SetRespawn(self, 20);
	}
	else
		G_FreeEdict(self);
}

qboolean Pickup_Health(edict_t *ent, edict_t *other)
{
	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health >= other->max_health)
			return false;
	}

//ZOID++
	if ((other->health >= (int)sv_health_max_bonus->value) && (ent->count > 25))					//CW
		return false;
//ZOID--

	other->health += ent->count;

//CW++
//	Extinguish flame if on fire, except if it's just a piddly little stimpack.

	if (other->burning && (ent->count > 9))
	{
		other->burning = false;
		if (other->flame)
		{
			other->flame->touch = NULL;
			other->flame->think = Flame_Expire;
			other->flame->nextthink = level.time + FRAMETIME;
		}
	}
//CW--

//ZOID++
	if ((other->health > (int)sv_health_max_bonus->value) && (ent->count > 25))						//CW
		other->health = (int)sv_health_max_bonus->value;											//CW
//ZOID--

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health > other->max_health)
			other->health = other->max_health;
	}

//ZOID++
	if ((ent->style & HEALTH_TIMED) && !CTFHasRegeneration(other))
//ZOID--
	{
//Maj++
		CheckCampSite(ent, other);
//Maj--

		ent->think = MegaHealth_think;
		ent->nextthink = level.time + 5.0;
		ent->owner = other;
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	else
	{
		if (!(ent->spawnflags & DROPPED_ITEM))														//CW
		{
//CW++
			if (ent->delay)
				SetRespawn(ent, ent->delay);
			else
//CW--
				SetRespawn(ent, 30);
		}
	}

	return true;
}

//======================================================================

int ArmorIndex(edict_t *ent)
{
	if (!ent->client)
		return 0;

	if (ent->client->pers.inventory[jacket_armor_index] > 0)
		return jacket_armor_index;

	if (ent->client->pers.inventory[combat_armor_index] > 0)
		return combat_armor_index;

	if (ent->client->pers.inventory[body_armor_index] > 0)
		return body_armor_index;

	return 0;
}

qboolean Pickup_Armor(edict_t *ent, edict_t *other)
{
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	float			salvage;
	int				salvagecount;
	int				old_armor_index;
	int				newcount;

	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;
	old_armor_index = ArmorIndex(other);

//CW++
	CheckCampSite(ent, other);
//CW--

	// handle armor shards specially
	if (ent->item->tag == ARMOR_SHARD)
	{
		if (!old_armor_index)
			other->client->pers.inventory[jacket_armor_index] = 2;
		else
			other->client->pers.inventory[old_armor_index] += 2;
	}

	// if player has no armor, just use it
	else if (!old_armor_index)
		other->client->pers.inventory[ITEM_INDEX(ent->item)] = newinfo->base_count;

	// use the better armor
	else
	{
		// get info on old armor
		if (old_armor_index == jacket_armor_index)
			oldinfo = &jacketarmor_info;
		else if (old_armor_index == combat_armor_index)
			oldinfo = &combatarmor_info;
		else // (old_armor_index == body_armor_index)
			oldinfo = &bodyarmor_info;

		if (newinfo->normal_protection > oldinfo->normal_protection)
		{
			// calc new armor values
			salvage = oldinfo->normal_protection / newinfo->normal_protection;
			salvagecount = salvage * other->client->pers.inventory[old_armor_index];
			newcount = newinfo->base_count + salvagecount;
			if (newcount > newinfo->max_count)
				newcount = newinfo->max_count;

			// zero count of old armor so it goes away
			other->client->pers.inventory[old_armor_index] = 0;

			// change armor to new item with computed value
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = newcount;
//Pon++
			if ((int)chedit->value && (other == &g_edicts[1]))
				other->client->pers.inventory[ITEM_INDEX(ent->item)] = 0;
//Pon--
		}
		else
		{
			// calc new armor values
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = salvage * newinfo->base_count;
			newcount = other->client->pers.inventory[old_armor_index] + salvagecount;
			if (newcount > oldinfo->max_count)
				newcount = oldinfo->max_count;

			// if we're already maxed out then we don't need the new armor
			if (other->client->pers.inventory[old_armor_index] >= newcount)
				return false;

			// update current armor value
			other->client->pers.inventory[old_armor_index] = newcount;
//Pon++
			if ((int)chedit->value && (other == &g_edicts[1]))
				other->client->pers.inventory[old_armor_index] = 0;
//Pon--
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM))															//CW
	{
//CW++
		if (ent->delay)
			SetRespawn(ent, ent->delay);
		else
//CW--
			SetRespawn(ent, 20);
	}

	return true;
}

//======================================================================

int PowerArmorType (edict_t *ent)
{
	if (!ent->client)
		return POWER_ARMOR_NONE;

//	if (!(ent->flags & FL_POWER_SHIELD) && !(ent->flags & FL_POWER_SCREEN))
//		return POWER_ARMOR_NONE;

//	if (ent->client->pers.inventory[power_shield_index] > 0)
	if (ent->flags & FL_POWER_SHIELD)
		return POWER_ARMOR_SHIELD;

//	if (ent->client->pers.inventory[power_screen_index] > 0)
	if (ent->flags & FL_POWER_SCREEN)
		return POWER_ARMOR_SCREEN;

	return POWER_ARMOR_NONE;
}

void Use_PowerArmor (edict_t *ent, gitem_t *item)
{
	int index;

	if (item == FindItemByClassname("item_power_screen"))
	{	//if player has an active power shield, deacivate that and activate power screen
		if (ent->flags & FL_POWER_SHIELD)
		{
			index = ITEM_INDEX(FindItem("cells"));
			if (!ent->client->pers.inventory[index])
			{
				gi.cprintf (ent, PRINT_HIGH, "No cells for power screen.\n");
				return;
			}
			ent->flags &= ~FL_POWER_SHIELD;
			ent->flags |= FL_POWER_SCREEN;
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
		}
		//if they have an active power screen, deactivate that
		else if (ent->flags & FL_POWER_SCREEN)
		{
			ent->flags &= ~FL_POWER_SCREEN;
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
		}
		else //activate power screen
		{
			index = ITEM_INDEX(FindItem("cells"));
			if (!ent->client->pers.inventory[index])
			{
				gi.cprintf (ent, PRINT_HIGH, "No cells for power screen.\n");
				return;
			}
			ent->flags |= FL_POWER_SCREEN;
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);

		}
	}
	else if (item == FindItemByClassname("item_power_shield"))
	{	//if player has an active power screen, deacivate that and activate power shield
		if (ent->flags & FL_POWER_SCREEN)
		{
			index = ITEM_INDEX(FindItem("cells"));
			if (!ent->client->pers.inventory[index])
			{
				gi.cprintf (ent, PRINT_HIGH, "No cells for power shield.\n");
				return;
			}
			ent->flags &= ~FL_POWER_SCREEN;
			ent->flags |= FL_POWER_SHIELD;
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->flags & FL_POWER_SHIELD)
		{
			ent->flags &= ~FL_POWER_SHIELD;
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			index = ITEM_INDEX(FindItem("cells"));
			if (!ent->client->pers.inventory[index])
			{
				gi_cprintf(ent, PRINT_HIGH, "No cells for power shield.\n");
				return;
			}
			ent->flags |= FL_POWER_SHIELD;
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
		}
	}
}

qboolean Pickup_PowerArmor(edict_t *ent, edict_t *other)
{
	int quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

//Pon++
	if ((int)chedit->value && (other == &g_edicts[1]))
		other->client->pers.inventory[ITEM_INDEX(ent->item)] = 0;
//Pon--

	if (!(ent->spawnflags & DROPPED_ITEM))
	{
//CW++
		if (ent->delay)
			SetRespawn(ent, ent->delay);
		else
//CW--
			SetRespawn(ent, ent->item->quantity);
	}

	// auto-use for DM only if we didn't already have one
	if (!quantity)
		ent->item->use(other, ent->item);

	return true;
}

void Drop_PowerArmor(edict_t *ent, gitem_t *item)
{
	if (item == FindItemByClassname("item_power_shield"))
	{
		if ((ent->flags & FL_POWER_SHIELD) && (ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
			Use_PowerArmor(ent, item);
	}
	else
		if ((ent->flags & FL_POWER_SCREEN) && (ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
			Use_PowerArmor (ent, item);

	Drop_General(ent, item);
}

//======================================================================

/*
===============
Touch_Item
===============
*/
void Touch_Item(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean taken;

	if (!other->client)
		return;

	if (other->health < 1)
		return;		// dead people can't pickup

	if (!ent->item->pickup)
		return;		// not a grabbable item?

	if (CTFMatchSetup())
		return;		// can't pick stuff up right now

	taken = ent->item->pickup(ent, other);
	if (taken)
	{
		// flash the screen
		other->client->bonus_alpha = 0.25;	

		// show icon and name on status bar
		other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
		other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);
		other->client->pickup_msg_time = level.time + 3.0;

		// change selected item
		if (ent->item->use)
			other->client->pers.selected_item = other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);

//Maj++
		CheckPrimaryWeapon(ent,	other); 
		if (ent->classname[0] != 'R')
		{
//Maj--
			if (ent->item->pickup == Pickup_Health)
			{
				if (ent->count == 2)
					gi.sound(other, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 1, ATTN_NORM, 0);
				else if (ent->count == 10)
					gi.sound(other, CHAN_ITEM, gi.soundindex("items/n_health.wav"), 1, ATTN_NORM, 0);
				else if (ent->count == 25)
					gi.sound(other, CHAN_ITEM, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound(other, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 1, ATTN_NORM, 0);

				PlayerNoise(other, other->s.origin, PNOISE_SELF);									//CW++
			}
			else if (ent->item->pickup_sound)
			{
				gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
				PlayerNoise(other, other->s.origin, PNOISE_SELF);									//CW++
			}
		}
	}
	else																							//CW
		return;

	if (!(ent->spawnflags & ITEM_TARGETS_USED))
	{
		G_UseTargets(ent, other);
		ent->spawnflags |= ITEM_TARGETS_USED;
	}

//Pon++
	if ((int)chedit->value)
	{
		qboolean k = false;

		if (ent->groundentity)
		{
			if (ent->groundentity->union_ent)
				k = true;		// flag set
		}

//		Route update.

		if (!k && (CurrentIndex < MAXNODES) && (other == &g_edicts[1]))
		{
			if (((ent->classname[0] == 'w') ||
				((ent->classname[0] == 'i') && ((ent->classname[5] == 'q') ||
												(ent->classname[5] == 't') ||
												(ent->classname[5] == 'f') ||
												(ent->classname[5] == 'i') ||
												(ent->classname[5] == 'p') ||
												(ent->classname[5] == 's') ||
												(ent->classname[5] == 'b') ||
												(ent->classname[5] == 'e') ||
												(ent->classname[5] == 'n') ||
												(ent->classname[5] == 'b') ||
												(ent->classname[5] == 'a'))) ||
				 ((ent->classname[0] == 'i') && (ent->classname[5] == 'h') && ((ent->classname[6] == 'a') || (ent->classname[12] == 'm'))) ||
				 ((ent->classname[0] == 'a'))) &&
				 !(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))							//CW
			{
				gi.bprintf(PRINT_HIGH, "Picked up: %s\n", ent->classname);
				VectorCopy(ent->monsterinfo.last_sighting, Route[CurrentIndex].Pt);
				Route[CurrentIndex].ent = ent;
				Route[CurrentIndex].state = GRS_ITEMS;

				if (++CurrentIndex < MAXNODES)
				{
					gi.bprintf(PRINT_HIGH, "Last %i pod(s).\n", MAXNODES - CurrentIndex);
					memset(&Route[CurrentIndex], 0, sizeof(route_t));
					Route[CurrentIndex].index = Route[CurrentIndex - 1].index + 1;
				}
			}
		}
	}
//Pon--

	if (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))										//CW
	{
		if (ent->flags & FL_RESPAWN)
			ent->flags &= ~FL_RESPAWN;
		else
			G_FreeEdict(ent);
	}
}

//======================================================================

static void drop_temp_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	Touch_Item(ent, other, plane, surf);
}

static void drop_make_touchable(edict_t *ent)
{
	ent->touch = Touch_Item;
	ent->nextthink = level.time + 29.0;
	ent->think = G_FreeEdict;
}

edict_t *Drop_Item(edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	offset;

//CW++
//	Sanity checks.

	if (item == NULL)
	{
		gi.dprintf("BUG: Drop_Item() called with null item.\nPlease contact musashi@planetquake.com\n");
		return NULL;
	}

	if (ent == NULL)
	{
		gi.dprintf("BUG: Drop_Item() called with null ent.\nPlease contact musashi@planetquake.com\n");
		return NULL;
	}
//CW--

	dropped = G_Spawn();
	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	VectorSet(dropped->mins, -15.0, -15.0, -15.0);
	VectorSet(dropped->maxs, 15.0, 15.0, 15.0);
	gi.setmodel(dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;  
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24.0, 0.0, -16.0);
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
		trace = gi.trace (ent->s.origin, dropped->mins, dropped->maxs, dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}

	VectorScale(forward, 100.0, dropped->velocity);
	dropped->velocity[2] = 300.0;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1.0;

	gi.linkentity(dropped);

	return dropped;
}

void Use_Item(edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->svflags &= ~SVF_NOCLIENT;
	ent->use = NULL;

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->touch = Touch_Item;
	}

	gi.linkentity(ent);
}

//======================================================================

/*
================
droptofloor
================
*/
void droptofloor(edict_t *ent)
{
	trace_t	tr;
	vec3_t	dest;
	float	*v;

	v = tv(-15.0, -15.0, -15.0);
	VectorCopy(v, ent->mins);
	v = tv(15.0, 15.0, 15.0);
	VectorCopy(v, ent->maxs);

	if (ent->model)
		gi.setmodel(ent, ent->model);
	else
		gi.setmodel(ent, ent->item->world_model);

	ent->solid = SOLID_TRIGGER;

//CW++
	if (ent->spawnflags & ITEM_FLOATING)
		ent->movetype = MOVETYPE_NONE;
	else
//CW--
		ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	v = tv(0.0, 0.0, -128.0);
	VectorAdd(ent->s.origin, v, dest);

	tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid && !(int)chedit->value)														//Pon
	{
		gi.dprintf("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict(ent);
		return;
	}

//CW++
	if (!(ent->spawnflags & ITEM_FLOATING))
//CW--
		VectorCopy(tr.endpos, ent->s.origin);

	if (ent->team)
	{
		ent->flags &= ~FL_TEAMSLAVE;
		ent->chain = ent->teamchain;
		ent->teamchain = NULL;

		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		if (ent == ent->teammaster)
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = DoRespawn;
		}
	}

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
		ent->s.effects &= ~EF_ROTATE;
		ent->s.renderfx &= ~RF_GLOW;
	}

	if (ent->spawnflags & ITEM_TRIGGER_SPAWN)
	{
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		ent->use = Use_Item;
	}	

	gi.linkentity(ent);
}


/*
===============
PrecacheItem

Precaches all data needed for a given item.
This will be called for each item spawned in a level,
and for each item in each client's inventory.
===============
*/
void PrecacheItem(gitem_t *it)
{
	gitem_t	*ammo;
	char	*s;
	char	*start;
	char	data[MAX_QPATH];
	int		len;

//CW++
	char	pcxdata[MAX_QPATH];
//CW--

	if (!it)
		return;

	if (it->pickup_sound)
		gi.soundindex(it->pickup_sound);
	if (it->world_model)
		gi.modelindex(it->world_model);
	if (it->view_model)
		gi.modelindex(it->view_model);
	if (it->icon)
		gi.imageindex(it->icon);

	// parse everything for its ammo
	if (it->ammo && it->ammo[0])
	{
		ammo = FindItem(it->ammo);
		if (ammo != it)
			PrecacheItem(ammo);
	}

	// parse the space seperated precache string for other items
	s = it->precaches;
	if (!s || !s[0])
		return;

	while (*s)
	{
		start = s;
		while (*s && (*s != ' '))
			s++;

		len = s - start;
		if ((len >= MAX_QPATH) || (len < 5))
			gi.error("PrecacheItem: %s has bad precache string", it->classname);

		memcpy(data, start, len);
		data[len] = 0;
		if (*s)
			s++;

		// determine type based on extension
		if (!strcmp(data+len-3, "md2"))
			gi.modelindex(data);
		else if (!strcmp(data+len-3, "sp2"))
			gi.modelindex(data);
		else if (!strcmp(data+len-3, "wav"))
			gi.soundindex(data);
		if (!strcmp(data+len-3, "pcx"))
//CW++
		{	//imageindex doesn't use the '.pcx' extension
			strncpy(pcxdata, data, len-4);
			memcpy(pcxdata+len-4, "\0", sizeof("\0"));
			gi.imageindex(pcxdata);
		}
//CW--
	}
}

/*
============
SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void SpawnItem(edict_t *ent, gitem_t *item)
{
//CW++
//	Sanity checks.

	if (item == NULL)
	{
		gi.dprintf("BUG: SpawnItem() called with null item.\nPlease contact musashi@planetquake.com\n");
		return;
	}

	if (ent == NULL)
	{
		gi.dprintf("BUG: SpawnItem() called with null ent.\nPlease contact musashi@planetquake.com\n");
		return;
	}

//	Remove disabled weapons.

	if (item->weapmodel)
	{
		if (!(int)sv_allow_gausspistol->value && (item->weapmodel == WEAP_GAUSSPISTOL))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_jackhammer->value && (item->weapmodel == WEAP_JACKHAMMER))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_mac10->value && (item->weapmodel == WEAP_MAC10))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_c4->value && (item->weapmodel == WEAP_C4))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_traps->value && (item->weapmodel == WEAP_TRAP))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_spikegun->value && (item->weapmodel == WEAP_ESG))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_rocketlauncher->value && (item->weapmodel == WEAP_ROCKETLAUNCHER))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_flamethrower->value && (item->weapmodel == WEAP_FLAMETHROWER))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_railgun->value && (item->weapmodel == WEAP_RAILGUN))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_shockrifle->value && (item->weapmodel == WEAP_SHOCKRIFLE))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_agm->value && (item->weapmodel == WEAP_AGM))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_disclauncher->value && (item->weapmodel == WEAP_DISCLAUNCHER))
		{
			G_FreeEdict(ent);
			return;
		}
	}

//	Removed disabled ammo entities.

	if (item->tag && (item->flags & IT_AMMO))
	{
		if (!(int)sv_allow_bullets->value && (item->tag == AMMO_BULLETS))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_shells->value && (item->tag == AMMO_SHELLS))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_rockets->value && (item->tag == AMMO_ROCKETS))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_cells->value && (item->tag == AMMO_CELLS))
		{
			G_FreeEdict(ent);
			return;
		}

		if (!(int)sv_allow_slugs->value && (item->tag == AMMO_SLUGS))
		{
			G_FreeEdict(ent);
			return;
		}
	}
//CW--

	if (ent->spawnflags)
	{
		if (strcmp(ent->classname, "key_power_cube") != 0)
		{
//CW++
			if (!(ent->spawnflags & ITEM_FLOATING) && !(ent->spawnflags & ITEM_FULLBRIGHT))
			{
//CW--
				ent->spawnflags = 0;
				gi.dprintf("%s at %s has invalid spawnflags set\n", ent->classname, vtos(ent->s.origin));
			}
		}
	}

//	Remove disabled items.

	if ((int)dmflags->value & DF_NO_ARMOR)
	{
		if ((item->pickup == Pickup_Armor) || (item->pickup == Pickup_PowerArmor))
		{
			G_FreeEdict(ent);
			return;
		}
	}

	if ((int)dmflags->value & DF_NO_ITEMS)
	{
		if (item->pickup == Pickup_Powerup)
		{
			G_FreeEdict(ent);
			return;
		}
	}

	if ((int)dmflags->value & DF_NO_HEALTH)
	{
		if ((item->pickup == Pickup_Health) || (item->pickup == Pickup_Adrenaline) || (item->pickup == Pickup_AncientHead))
		{
			G_FreeEdict(ent);
			return;
		}
	}

//ZOID++
//	Don't spawn the flags unless enabled.

	if ((sv_gametype->value != G_CTF) && (!strcmp(ent->classname, "item_flag_team1") || !strcmp(ent->classname, "item_flag_team2"))) //CW
	{
		G_FreeEdict(ent);
		return;
	}
//ZOID--

	PrecacheItem(item);																				//CW

	ent->item = item;
	ent->nextthink = level.time + (2.0 * FRAMETIME);		// items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;

//CW++
	if (ent->spawnflags & ITEM_FULLBRIGHT)
		ent->s.renderfx |= RF_FULLBRIGHT;
//CW--

	ent->s.renderfx |= RF_GLOW;																		//CW

	if (ent->model)
		gi.modelindex(ent->model);

//Pon++
	VectorCopy(ent->s.origin, ent->monsterinfo.last_sighting);
//Pon--

//ZOID++
//	Flags are server animated and have special handling.

	if (ent->item && (ent->item->flags & IT_FLAG))													//CW
		ent->think = CTFFlagSetup;
//ZOID--
}

//======================================================================

gitem_t	itemlist[] = 
{
	{
		NULL
	},	// leave index 0 alone

//Maj++ ----------------------------------------
  //
  // NAVIGATION NODES
  //
	{
		"R_navi1",
		Pickup_Navi,
		NULL,
		NULL,
		NULL,
		NULL,
		"models/items/keys/spinner/tris.md2", 0,
		NULL,
		"i_fixme",																					//CW
		"Roam Navi1",
		2,
		10,
		NULL,
		IT_NODE,
		0,
		NULL,
		0,
		""
	},

	{
		"R_navi2",
		Pickup_Navi,
		NULL,
		NULL,
		NULL,
		NULL,
		"models/items/keys/spinner/tris.md2", 0,
		NULL,
		"i_fixme",																					//CW
		"Roam Navi2",
		2,
		30,
		NULL,
		IT_NODE,
		0,
		NULL,
		0,
		""
		},

		{
		"R_navi3",
		Pickup_Navi,
		NULL,
		NULL,
		NULL,
		"i_fixme",																					//CW
		"models/items/keys/spinner/tris.md2", 0,
		NULL,
		NULL,
		"Roam Navi3",
		2,
		20,
		NULL,
		IT_NODE,
		0,
		NULL,
		0,
		""
	},
//Maj--

	//
	// ARMOR
	//

/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_body", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/body/tris.md2", EF_ROTATE,
		NULL,
		"i_bodyarmor",
		"Body Armor",
		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&bodyarmor_info,
		ARMOR_BODY,
		""
	},

/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_combat", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/combat/tris.md2", EF_ROTATE,
		NULL,
		"i_combatarmor",
		"Combat Armor",
		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&combatarmor_info,
		ARMOR_COMBAT,
		""
	},

/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_jacket", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/jacket/tris.md2", EF_ROTATE,
		NULL,
		"i_jacketarmor",
		"Jacket Armor",
		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&jacketarmor_info,
		ARMOR_JACKET,
		""
	},

/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_shard", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar2_pkup.wav",
		"models/items/armor/shard/tris.md2", EF_ROTATE,
		NULL,
#ifdef KMQUAKE2_ENGINE_MOD
		"i_shard",
#else
		"i_jacketarmor",
#endif
		"Armor Shard",
		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		ARMOR_SHARD,
		""
	},


/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_power_screen", 
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/screen/tris.md2", EF_ROTATE,
		NULL,
		"i_powerscreen",
		"Power Screen",
		0,
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
		""
	},

/*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_power_shield",
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/shield/tris.md2", EF_ROTATE,
		NULL,
		"i_powershield",
		"Power Shield",
		0,
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
		"misc/power2.wav misc/power1.wav"
	},


	//
	// WEAPONS 
	//

//ZOID++
/* weapon_grapple (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_grapple", 
		NULL,
		Use_Weapon,
		NULL,
		CTFWeapon_Grapple,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/grapple/tris.md2",
		"w_grapple",
		"Grapple",
		0,
		0,
		NULL,
		IT_WEAPON,
		WEAP_GRAPPLE,
		NULL,
		0,
		"models/weapons/grapple/hook/tris.md2 weapons/grapple/grfire.wav weapons/grapple/grpull.wav weapons/grapple/grhang.wav weapons/grapple/grreset.wav weapons/grapple/grhit.wav"	//CW
	},
//ZOID--

//CW++
/* weapon_chainsaw
NB: always owned, never in the world
*/
	{
		"weapon_chainsaw", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Chainsaw,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_chainsaw/tris.md2",
		"w_saw",
		"Chainsaw",
		0,
		0,
		NULL,
		IT_WEAPON,
		WEAP_CHAINSAW,
		NULL,
		0,
		"weapons/chainsaw/done.wav weapons/chainsaw/fire.wav weapons/chainsaw/idle.wav weapons/chainsaw/use.wav"
	},

/* weapon_deserteagle
NB: always owned, never in the world
*/
	{
		"weapon_deserteagle", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_DesertEagle,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_de/tris.md2",
		"w_de",
		"Desert Eagle",
		0,
		1,
		"Bullets",
		IT_WEAPON,
		WEAP_DESERTEAGLE,
		NULL,
		0,
		"weapons/blastf1a.wav"
	},

	
/*QUAKED weapon_gausspistol (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_gausspistol", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_GaussPistol,
		"misc/w_pkup.wav",
		"models/weapons/g_gauss/tris.md2", EF_ROTATE,
		"models/weapons/v_gauss/tris.md2",
		"w_gauss",
		"Gauss Pistol",
		0,
		1,
		"Cells",
		IT_WEAPON,
		WEAP_GAUSSPISTOL,
		NULL,
		0,
		"weapons/shotgf1b.wav weapons/gauss/use.wav weapons/gauss/warn.wav a_blaster.pcx g_scan.pcx g_targ.pcx"
	},

/*QUAKED weapon_jackhammer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_jackhammer", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Jackhammer,
		"misc/w_pkup.wav",
		"models/weapons/g_pancor/tris.md2", EF_ROTATE,
		"models/weapons/v_pancor/tris.md2",
		"w_pancor",
		"Jackhammer",
		0,
		1,
		"Shells",
		IT_WEAPON,
		WEAP_JACKHAMMER,
		NULL,
		0,
		"weapons/sshotf1b.wav weapons/sshotr1b.wav"
	},

/*QUAKED weapon_mac10 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_mac10", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Mac10,
		"misc/w_pkup.wav",
		"models/weapons/g_macten/tris.md2", EF_ROTATE,
		"models/weapons/v_macten/tris.md2",
		"w_macten",
		"Mac-10",
		0,
		1,
		"Bullets",
		IT_WEAPON,
		WEAP_MAC10,
		NULL,
		0,
		"weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav"
	},

/*QUAKED weapon_esg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_esg", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ESG,
		"misc/w_pkup.wav",
		"models/weapons/g_esg/tris.md2", EF_ROTATE,
		"models/weapons/v_esg/tris.md2",
		"w_esg",
		"E.S.G.",
		0,
		1,
		"Rockets",
		IT_WEAPON,
		WEAP_ESG,
		NULL,
		0,
		"models/objects/spike/tris.md2 weapons/esg/fire.wav weapons/esg/hit_body.wav weapons/esg/hit_wall.wav weapons/esg/reload.wav weapons/esg/use.wav"
	},
//CW--

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
		"w_rlauncher",
		"Rocket Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
		"models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

//CW++
/*QUAKED weapon_flamethrower (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_flamethrower", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Flamethrower,
		"misc/w_pkup.wav",
		"models/weapons/g_flame/tris.md2", EF_ROTATE,
		"models/weapons/v_flame/tris.md2",
		"w_flamer",
		"Flamethrower",
		0,
		1,
		"Cells",
		IT_WEAPON,
		WEAP_FLAMETHROWER,
		NULL,
		0,
		"models/fire/tris.md2 models/objects/r_explode/tris.md2 models/objects/firebomb/tris.md2 weapons/hyprbf1a.wav weapons/flamer/use.wav weapons/flamer/fire1.wav weapons/rocklx1a.wav"
	},
//CW--

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Railgun,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
		"w_railgun",
		"Railgun",
		0,
		1,
		"Slugs",
		IT_WEAPON,
		WEAP_RAILGUN,
		NULL,
		0,
		"weapons/rg_hum.wav"
	},

//CW++
/*QUAKED weapon_shockrifle (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shockrifle",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ShockRifle,
		"misc/w_pkup.wav",
		"models/weapons/g_shock/tris.md2", EF_ROTATE,
		"models/weapons/v_shock/tris.md2",
		"w_shock",
		"Shock Rifle",
		0,
		10,
		"Cells",
		IT_WEAPON,
		WEAP_SHOCKRIFLE,
		NULL,
		0,
		"models/objects/shock/tris.md2 weapons/bfg__f1y.wav weapons/shock/shockfly.wav weapons/shock/shockhit.wav"
	},

/*QUAKED weapon_agm (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_agm", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_AGM,
		"misc/w_pkup.wav",
		"models/weapons/g_agm/tris.md2", EF_ROTATE,
		"models/weapons/v_agm/tris.md2",
		"w_agm",
		"AG Manipulator",
		0,
		10,
		"Cells",
		IT_WEAPON,
		WEAP_AGM,
		NULL,
		0,
		"world/amb15.wav world/fusein.wav parasite/paratck3.wav medic/medatck1.wav weapons/agm/agm_cross.wav weapons/agm/agm_trip.wav a_agm.pcx a_refuse.pcx"
	},

/*QUAKED weapon_disclauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_disclauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_DiscLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_disc/tris.md2", EF_ROTATE,
		"models/weapons/v_disc/tris.md2",
		"w_disc",
		"Disc Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON,
		WEAP_DISCLAUNCHER,
		NULL,
		0,
		"models/objects/disc/tris.md2 weapons/rippfire.wav weapons/disc/hit.wav weapons/disc/idle.wav weapons/disc/ric1.wav weapons/disc/ric2.wav weapons/disc/ric3.wav"
	},
//CW--

	//
	// AMMO ITEMS
	//

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_shells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/shells/medium/tris.md2", 0,
		NULL,
		"a_shells",
		"Shells",
		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SHELLS,
		""
	},

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bullets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/bullets/medium/tris.md2", 0,
		NULL,
		"a_bullets",
		"Bullets",
		3,
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BULLETS,
		""
	},

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_cells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/cells/medium/tris.md2", 0,
		NULL,
		"a_cells",
		"Cells",
		3,
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_CELLS,
		""
	},

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_rockets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rockets/medium/tris.md2", 0,
		NULL,
		"a_rockets",
		"Rockets",
		3,
		5,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_ROCKETS,
		""
	},

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_slugs",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/slugs/medium/tris.md2", 0,
		NULL,
		"a_slugs",
		"Slugs",
		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SLUGS,
		""
	},

//CW++
/*QUAKED ammo_c4 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_c4",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_C4,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
		"a_grenades",
		"C4",
		3,
		5,
		"C4",
		IT_WEAPON|IT_AMMO,
		WEAP_C4,
		NULL,
		AMMO_C4,
		"models/objects/grenade/tris.md2 models/objects/debris2/tris.md2 weapons/grenlx1a.wav weapons/hgrenc1b.wav weapons/c4/arm.wav weapons/c4/bounce1.wav weapons/c4/bounce2.wav weapons/c4/timer.wav"
	},

/*QUAKED ammo_traps (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_traps",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Trap,
		"misc/w_pkup.wav",
		"models/weapons/g_tract/tris.md2", 0,
		"models/weapons/v_tract/tris.md2",
		"a_traps",
		"Traps",
		3,
		5,
		"Traps",
		IT_WEAPON|IT_AMMO,
		WEAP_TRAP,
		NULL,
		AMMO_TRAPS,
		"models/objects/trap/tris.md2 models/objects/hook/tris.md2 models/objects/debris2/tris.md2 weapons/grenlx1a.wav weapons/hgrenc1b.wav weapons/trap/arm.wav weapons/trap/bounce1.wav weapons/trap/bounce2.wav weapons/trap/timer.wav weapons/hook/hit_body.wav weapons/hook/hit_wall.wav weapons/hook/rip.wav weapons/esg/fire.wav makron/blaster.wav"
	},
//CW--

	//
	// POWERUP ITEMS
	//
/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_quad",																				//CW
		Pickup_Powerup,
		Use_Quad,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/quaddama/tris.md2", EF_ROTATE,
		NULL,
		"p_quad",
		"Quad Damage",
		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		POWERUP_QUAD,																				//CW
		"items/damage.wav items/damage2.wav items/damage3.wav"
	},

/*QUAKED item_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_invulnerability",																		//CW
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/invulner/tris.md2", EF_ROTATE,
		NULL,
		"p_invulnerability",
		"Invulnerability",
		2,
		300,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		POWERUP_INVULN,																				//CW
		"items/protect.wav items/protect2.wav items/protect4.wav"
	},

/*QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_silencer",																			//CW
		Pickup_Powerup,
		Use_Silencer,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/silencer/tris.md2", EF_ROTATE,
		NULL,
		"p_silencer",
		"Silencer",
		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		POWERUP_SILENCER,																			//CW
		""
	},

/*QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_breather",
		Pickup_Powerup,
		Use_Breather,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/breather/tris.md2", EF_ROTATE,
		NULL,
		"p_rebreather",
		"Rebreather",
		2,
		60,
		NULL,
		IT_POWERUP,																					//CW
		0,
		NULL,
		POWERUP_BREATHER,																			//CW
		"items/airout.wav"
	},

/*QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_enviro",
		Pickup_Powerup,
		Use_Envirosuit,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/enviro/tris.md2", EF_ROTATE,
		NULL,
		"p_envirosuit",
		"Environment Suit",
		2,
		60,
		NULL,
		IT_POWERUP,																					//CW
		0,
		NULL,
		POWERUP_ENVIRO,																				//CW
		"items/airout.wav"
	},

/*QUAKED item_ancient_head (.3 .3 1) (-16 -16 -16) (16 16 16)
Special item that gives +2 to maximum health
*/
	{
		"item_ancient_head",
		Pickup_AncientHead,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/c_head/tris.md2", EF_ROTATE,
		NULL,
		"i_fixme",
		"Ancient Head",
		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
		""
	},

/*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16)
gives +1 to maximum health
*/
	{
		"item_adrenaline",
		Pickup_Adrenaline,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/adrenal/tris.md2", EF_ROTATE,
		NULL,
		"p_adrenaline",
		"Adrenaline",
		2,
		60,
		NULL,
		IT_HEALTH,																					//Maj
		0,
		NULL,
		0,
		""
	},

/*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_bandolier",
		Pickup_Bandolier,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/band/tris.md2", EF_ROTATE,
		NULL,
		"p_bandolier",
		"Bandolier",
		2,
		60,
		NULL,
		IT_PACK,																					//Maj
		0,
		NULL,
		0,
		""
	},

/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_pack",
		Pickup_Pack,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/pack/tris.md2", EF_ROTATE,
		NULL,
		"i_pack",
		"Ammo Pack",
		2,
		180,
		NULL,
		IT_PACK,																					//Maj
		0,
		NULL,
		0,
		""
	},

//CW++
/*QUAKED item_siphon (.3 .3 1) (-16 -16 -16) (16 16 16)
Awakening (Siphon) - substitute for item_quad
*/
	{
		"item_siphon",
		Pickup_Powerup,
		Use_Siphon,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/siphon/tris.md2", EF_ROTATE,
		NULL,
		"p_siphon",
		"Awakening",
		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		POWERUP_SIPHON,																				//CW
		"items/siphon.wav items/siphon2.wav items/siphon3.wav"
	},

/*QUAKED item_needle (.3 .3 1) (-16 -16 -16) (16 16 16)
D89 (Needle) - substitute for item_invulnerability
*/
	{
		"item_needle",
		Pickup_Powerup,
		Use_Needle,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/needle/tris.md2", EF_ROTATE,
		NULL,
		"p_needle",
		"D89",
		2,
		120,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		POWERUP_D89,																				//CW
		"parasite/paratck4.wav"
	},

/*QUAKED item_haste (.3 .3 1) (-16 -16 -16) (16 16 16)
Haste - substitute for item_silencer
*/
	{
		"item_haste",
		Pickup_Powerup,
		Use_Haste,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/haste/tris.md2", EF_ROTATE,
		NULL,
		"p_haste",
		"Haste",
		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		POWERUP_HASTE,																				//CW
		"items/haste.wav"
	},

/*QUAKED item_antibeam (.3 .3 1) (-16 -16 -16) (16 16 16)
Gauss and AGM Beam Reflector
*/
	{
		"item_antibeam",
		Pickup_Powerup,
		Use_AntiBeam,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/data_cd/tris.md2", EF_ROTATE,
		NULL,
		"k_datacd",
		"Beam Reflector",
		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		POWERUP_ANTIBEAM,																			//CW
		"ctf/tech1.wav"
	},

/*QUAKED item_teleporter (.3 .3 1) (-16 -16 -16) (16 16 16)
Personal Teleporter
*/
	{
		"item_teleporter",
		Pickup_Powerup,
		Use_Teleporter,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/power/tris.md2", EF_ROTATE,
		NULL,
		"k_powercube",
		"Teleporter",
		2,
		30,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		POWERUP_TELE,																				//CW
		"misc/tele_up.wav"
	},

//	Mystery Box
	{
		"item_mystery",
		Pickup_Mystery,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/mystery/tris.md2", EF_ROTATE,
		NULL,
		"i_fixme",
		"Mystery",
		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		POWERUP_MYSTERY,																			//CW
		"p_quad.pcx models/items/quaddama/tris.md2 items/damage.wav items/damage2.wav items/damage3.wav p_invulnerability.pcx items/protect.wav items/protect2.wav items/protect4.wav items/airout.wav p_envirosuit.pcx p_siphon.pcx items/siphon.wav items/siphon2.wav items/siphon3.wav p_needle.pcx parasite/paratck4.wav p_haste.pcx items/haste.wav k_datacd.pcx ctf/tech1.wav k_powercube.pcx misc/tele_up.wav"
	},
//CW--

	//
	// KEYS
	//
/*QUAKED key_data_cd (0 .5 .8) (-16 -16 -16) (16 16 16)
key for computer centers
*/
	{
		"key_data_cd",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/data_cd/tris.md2", EF_ROTATE,
		NULL,
		"k_datacd",
		"Data CD",
		2,
		0,
		NULL,
		IT_KEY,																						//CW
		0,
		NULL,
		0,
		""
	},

/*QUAKED key_power_cube (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN NO_TOUCH
warehouse circuits
*/
	{
		"key_power_cube",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/power/tris.md2", EF_ROTATE,
		NULL,
		"k_powercube",
		"Power Cube",
		2,
		0,
		NULL,
		IT_KEY,																						//CW
		0,
		NULL,
		0,
		""
	},

/*QUAKED key_pyramid (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the entrance of jail3
*/
	{
		"key_pyramid",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
		"k_pyramid",
		"Pyramid Key",
		2,
		0,
		NULL,
		IT_KEY,																						//CW
		0,
		NULL,
		0,
		""
	},

/*QUAKED key_data_spinner (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the city computer
*/
	{
		"key_data_spinner",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/spinner/tris.md2", EF_ROTATE,
		NULL,
		"k_dataspin",
		"Data Spinner",
		2,
		0,
		NULL,
		IT_KEY,																						//CW
		0,
		NULL,
		0,
		""
	},

/*QUAKED key_pass (0 .5 .8) (-16 -16 -16) (16 16 16)
security pass for the security level
*/
	{
		"key_pass",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		"k_security",
		"Security Pass",
		2,
		0,
		NULL,
		IT_KEY,																						//CW
		0,
		NULL,
		0,
		""
	},

/*QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - blue
*/
	{
		"key_blue_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/key/tris.md2", EF_ROTATE,
		NULL,
		"k_bluekey",
		"Blue Key",
		2,
		0,
		NULL,
		IT_KEY,																						//CW
		0,
		NULL,
		0,
		""
	},

/*QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - red
*/
	{
		"key_red_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/red_key/tris.md2", EF_ROTATE,
		NULL,
		"k_redkey",
		"Red Key",
		2,
		0,
		NULL,
		IT_KEY,																						//CW
		0,
		NULL,
		0,
		""
	},

/*QUAKED key_commander_head (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
	{
		"key_commander_head",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/monsters/commandr/head/tris.md2", EF_GIB,
		NULL,
		"k_comhead",
		"Commander's Head",
		2,
		0,
		NULL,
		IT_KEY,																						//CW
		0,
		NULL,
		0,
		""
	},

/*QUAKED key_airstrike_target (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
	{
		"key_airstrike_target",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/target/tris.md2", EF_ROTATE,
		NULL,
		"i_airstrike",
		"Airstrike Marker",
		2,
		0,
		NULL,
		IT_KEY,																						//CW
		0,
		NULL,
		0,
		""
	},

	{
		NULL,
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		NULL, 0,
		NULL,
		"i_health",
		"Health",
		3,
		0,
		NULL,
		IT_HEALTH,																					//Maj
		0,
		NULL,
		0,
		"items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},


//ZOID++
/*QUAKED item_flag_team1 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
		"item_flag_team1",
		CTFPickup_Flag,
		NULL,
		CTFDrop_Flag,
		NULL,
		"ctf/flagtk.wav",
		"players/male/flag1.md2", EF_FLAG1,
		NULL,
		"i_ctf1",
		"Red Flag",
		2,
		0,
		NULL,
		IT_FLAG,																					//CW
		0,
		NULL,
		0,
		"ctf/flagcap.wav"
	},

/*QUAKED item_flag_team2 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
		"item_flag_team2",
		CTFPickup_Flag,
		NULL,
		CTFDrop_Flag,
		NULL,
		"ctf/flagtk.wav",
		"players/male/flag2.md2", EF_FLAG2,
		NULL,
		"i_ctf2",
		"Blue Flag",
		2,
		0,
		NULL,
		IT_FLAG,																					//CW
		0,
		NULL,
		0,
		"ctf/flagcap.wav"
	},

/* Resistance Tech */
	{
		"item_tech1",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech,
		NULL,
		"items/pkup.wav",
		"models/ctf/resistance/tris.md2", EF_ROTATE,
		NULL,
		"tech1",
		"Disruptor Shield",
		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
		"ctf/tech1.wav"
	},

/* Strength Tech */
	{
		"item_tech2",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech,
		NULL,
		"items/pkup.wav",
		"models/ctf/strength/tris.md2", EF_ROTATE,
		NULL,
		"tech2",
		"Power Amplifier",
		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
		"ctf/tech2.wav ctf/tech2x.wav"
	},

/* Haste Tech */
	{
		"item_tech3",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech,
		NULL,
		"items/pkup.wav",
		"models/ctf/haste/tris.md2", EF_ROTATE,
		NULL,
		"tech3",
		"Time Accel",
		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
		"ctf/tech3.wav"
	},

/* Regeneration Tech */
	{
		"item_tech4",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech,
		NULL,
		"items/pkup.wav",
		"models/ctf/regeneration/tris.md2", EF_ROTATE,
		NULL,
		"tech4",
		"AutoDoc",
		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
		"ctf/tech4.wav"
	},
//ZOID--

	// end of list marker
	{NULL}
};


/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health(edict_t *self)
{
	if ((int)dmflags->value & DF_NO_HEALTH)															//CW
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
	SpawnItem(self, FindItem("Health"));
	gi.soundindex("items/n_health.wav");
}

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_small(edict_t *self)
{
	if ((int)dmflags->value & DF_NO_HEALTH)															//CW
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/items/healing/stimpack/tris.md2";
	self->count = 2;
	SpawnItem(self, FindItem("Health"));
	gi.soundindex("items/s_health.wav");
	self->style = HEALTH_IGNORE_MAX;
}

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_large(edict_t *self)
{
	if ((int)dmflags->value & DF_NO_HEALTH)															//CW
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/items/healing/large/tris.md2";
	self->count = 25;
	SpawnItem(self, FindItem("Health"));
	gi.soundindex("items/l_health.wav");
}

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_mega(edict_t *self)
{
	if ((int)dmflags->value & DF_NO_HEALTH)															//CW
	{
		G_FreeEdict(self);
		return;
	}

	self->model = "models/items/mega_h/tris.md2";
	self->count = 100;
	SpawnItem(self, FindItem("Health"));
	gi.soundindex("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX | HEALTH_TIMED;
}

//CW++
//	Standard Q2 weapon substitutions.

void SP_weapon_shotgun(edict_t *self)
{
	self->classname = "weapon_gausspistol";
	SpawnItem(self, FindItem("Gauss Pistol"));
}

void SP_weapon_supershotgun(edict_t *self)
{
	self->classname = "weapon_jackhammer";
	SpawnItem(self, FindItem("Jackhammer"));
}

void SP_weapon_machinegun(edict_t *self)
{
	self->classname = "weapon_mac10";
	SpawnItem(self, FindItem("Mac-10"));
}

void SP_weapon_chaingun(edict_t *self)
{
	self->classname = "weapon_esg";
	SpawnItem(self, FindItem("E.S.G."));
}

void SP_weapon_grenadelauncher(edict_t *self)
{
	self->classname = "ammo_traps";
	SpawnItem(self, FindItem("Traps"));
}

void SP_weapon_hyperblaster(edict_t *self)
{
	self->classname = "weapon_flamethrower";
	SpawnItem(self, FindItem("Flamethrower"));
}

void SP_weapon_bfg(edict_t *self)
{
	self->classname = "weapon_shockrifle";
	SpawnItem(self, FindItem("Shock Rifle"));
}

void SP_ammo_grenades(edict_t *self)
{
	self->classname = "ammo_c4";
	SpawnItem(self, FindItem("C4"));
}
//CW--

void InitItems (void)
{
	game.num_items = sizeof(itemlist) / sizeof(itemlist[0]) - 1;
}

/*
===============
SetItemNames

Called by worldspawn
===============
*/
void SetItemNames(void)
{
	gitem_t	*it;
	int		i;

	for (i = 1; i < game.num_items; ++i)															//CW
	{
		it = &itemlist[i];
		gi.configstring(CS_ITEMS+i, it->pickup_name);
	}

//Maj++
	 InitAllItems();
//Maj--

	jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
	combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
	body_armor_index   = ITEM_INDEX(FindItem("Body Armor"));
	power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
	power_shield_index = ITEM_INDEX(FindItem("Power Shield"));

	// Knightmare added
	shells_index       = ITEM_INDEX(FindItem("Shells"));
	bullets_index      = ITEM_INDEX(FindItem("Bullets"));
//	grenades_index     = ITEM_INDEX(FindItem("Grenades"));
	c4_index		   = ITEM_INDEX(FindItem("C4"));
	rockets_index      = ITEM_INDEX(FindItem("Rockets"));
	cells_index        = ITEM_INDEX(FindItem("Cells"));
	slugs_index        = ITEM_INDEX(FindItem("Slugs"));
	traps_index        = ITEM_INDEX(FindItem("Traps"));
	// end Knightmare
}
