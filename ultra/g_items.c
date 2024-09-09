#include "g_local.h"
#include "g_items.h"

#include	"bot_procs.h"
#include	"p_trail.h"

#include "aj_startmax.h" // AJ
#include "aj_replacelist.h" // AJ

int		CanReach(edict_t *self, edict_t *targ);

void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Use_Weapon2 (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);

gitem_armor_t jacketarmor_info	= { 25,  65000, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 65000, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {100, 65000, .80, .60, ARMOR_BODY};

void Weapon_Blaster (edict_t *ent);
void Weapon_Shotgun (edict_t *ent);
void Weapon_SuperShotgun (edict_t *ent);
void Weapon_Machinegun (edict_t *ent);
void Weapon_Chaingun (edict_t *ent);
void Weapon_HyperBlaster (edict_t *ent);
void Weapon_RocketLauncher (edict_t *ent);
void Weapon_Grenade (edict_t *ent);
void Weapon_GrenadeLauncher (edict_t *ent);
void Weapon_Railgun (edict_t *ent);
void Weapon_BFG (edict_t *ent);
// RAFAEL
void Weapon_Ionripper (edict_t *ent);
void Weapon_Phalanx (edict_t *ent);
void Weapon_Trap (edict_t *ent);

void Use_Quad (edict_t *ent, gitem_t *item);
// RAFAEL
void Use_QuadFire (edict_t *ent, gitem_t *item);

//=========
//Rogue Weapons
void Weapon_ChainFist (edict_t *ent);
void Weapon_Disintegrator (edict_t *ent);
void Weapon_ETF_Rifle (edict_t *ent);
void Weapon_Heatbeam (edict_t *ent);
void Weapon_Prox (edict_t *ent);
void Weapon_Tesla (edict_t *ent);
void Weapon_ProxLauncher (edict_t *ent);
//void Weapon_Nuke (edict_t *ent);
//Rogue Weapons
//=========

void Weapon_Shockwave (edict_t *ent);
void Weapon_Plasma_Rifle (edict_t *ent);	// SKWiD MOD

static int	quad_drop_timeout_hack;
static int	double_drop_timeout_hack;

// RAFAEL
static int	quad_fire_drop_timeout_hack;

int	jacket_armor_index;
int	combat_armor_index;
int	body_armor_index;
int	power_screen_index;
int	power_shield_index;
int	shells_index;
int	bullets_index;
int	grenades_index;
int	rockets_index;
int	cells_index;
int	slugs_index;
int	magslug_index;
int	flechettes_index;
int	prox_index;
int	disruptors_index;
int	tesla_index;
int	trap_index;
int	blaster_index;
int	pl_index;
int	rg_index;
int	pr_index;		// SKWiD MOD

//======================================================================

/*
===============
AddToItemList

  Adds an item to the respective linked list, returns the head of that list
===============
*/
edict_t	*AddToItemList(edict_t *ent, edict_t	*head)
{
	edict_t *trav;

	// make sure this item isn't already in the list
	trav = head;
	while (trav)
	{
		if (trav == ent)		// already in there!
			return head;

		trav = trav->node_target;
	}

	if (head)
		head->last_goal = ent;

	ent->node_target = head;
	ent->last_goal = NULL;

	return ent;
};

void	RemoveFromItemList(edict_t *ent)
{
	if (ent->node_target)
		ent->node_target->last_goal = ent->last_goal;

	if (ent->last_goal)
	{
		ent->last_goal->node_target = ent->node_target;
//		ent->last_goal->last_goal = NULL;
	}
	else if (ent->node_target)	// ent must be head
	{
		if (ent->item->pickup == Pickup_Weapon)
			weapons_head = ent->node_target;
		else if (ent->item->pickup == Pickup_Health)
			health_head = ent->node_target;
		else if (ent->item->pickup == Pickup_Ammo)
			ammo_head = ent->node_target;
		else
			bonus_head = ent->node_target;
	}

	ent->node_target = NULL;
	ent->last_goal = NULL;
};

void	RemoveDroppedItem(edict_t	*ent)
{
	RemoveFromItemList(ent);
	G_FreeEdict(ent);
};

/*
===============
GetItemByIndex
===============
*/
gitem_t	*GetItemByIndex (int index)
{
	if (index == 0 || index >= game.num_items)
		return NULL;

	return &itemlist[index];
}

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
	else if (item_index == grenades_index)
		value = client->pers.max_grenades;
	else if (item_index == rockets_index)
		value = client->pers.max_rockets;
	else if (item_index == cells_index)
		value = client->pers.max_cells;
	else if (item_index == slugs_index)
		value = client->pers.max_slugs;
	else if (item_index == magslug_index)
		value = client->pers.max_magslug;
	else if (item_index == flechettes_index)
		value = client->pers.max_flechettes;
	else if (item_index == prox_index)
		value = client->pers.max_prox;
	else if (item_index == disruptors_index)
		value = client->pers.max_rounds;
	else if (item_index == tesla_index)
		value = client->pers.max_tesla;
	else if (item_index == trap_index)
		value = client->pers.max_trap;
	else
		value = 0;

	return value;
}

/*
===============
FindItemByClassname

===============
*/
gitem_t	*FindItemByClassname (char *classname)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if ( !it->classname )
			continue;
		if ( !Q_stricmp(it->classname, classname) )
			return it;
	}

	return NULL;
}

/*
===============
FindItem

===============
*/
gitem_t	*FindItem (char *pickup_name)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if ( !it->pickup_name )
			continue;
		if ( !Q_stricmp(it->pickup_name, pickup_name) )
			return it;
	}

	return NULL;
}

//======================================================================

void DoRespawn (edict_t *ent)
{
	if (ent->team)
	{
		edict_t	*master;
		int	count;
		int choice;

		master = ent->teammaster;

//ZOID
//in ctf, when we are weapons stay, only the master of a team of weapons
//is spawned
		if (ctf->value &&
			((int)dmflags->value & DF_WEAPONS_STAY) &&
			master->item && (master->item->flags & IT_WEAPON))
			ent = master;
		else {
//ZOID
			for (count = 0, ent = master; ent; ent = ent->chain, count++)
				;

			choice = rand() % count;

			for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
				;
		}
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	gi.linkentity (ent);

	// send an effect
	ent->s.event = EV_ITEM_RESPAWN;
}

void SetRespawn (edict_t *ent, float delay)
{
	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->nextthink = level.time + delay;
	ent->think = DoRespawn;
	gi.linkentity (ent);
}


//======================================================================

qboolean Pickup_Powerup (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	if ( ((skill->value == 1) && (quantity >= sk_powerup_max->value)) || ((skill->value >= 2) && (quantity >= sk_powerup_max->value)) )
		return false;

	if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		if ((other->bot_client) || ((int)dmflags->value & DF_INSTANT_ITEMS)
			|| (((ent->item->use == Use_Quad) || (ent->item->use == Use_Double) || (ent->item->use == Use_QuadFire)) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			if ((ent->item->use == Use_Double) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				double_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			if ((ent->item->use == Use_QuadFire) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_fire_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			ent->item->use (other, ent->item);
		}
		// RAFAEL
/*		else if (((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_QuadFire) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if ((ent->item->use == Use_QuadFire) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_fire_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			ent->item->use (other, ent->item);
		}*/
	}

	return true;
}

void Drop_General (edict_t *ent, gitem_t *item)
{
	Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
}


//======================================================================

qboolean Pickup_Adrenaline (edict_t *ent, edict_t *other)
{
	if (!deathmatch->value) {
		other->max_health += 1;
		// Knightmare- copy max health to client_persistant_t
		// Fixes health reverting to prev max_health value on
		// map change when game is not saved first
		if (other->client)
			other->client->pers.max_health = other->max_health;
	}

	if (other->health < other->max_health)
		other->health = other->max_health;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_AncientHead (edict_t *ent, edict_t *other)
{
	other->max_health += 2;
	// Knightmare- copy max health to client_persistant_t
	// Fixes health reverting to prev max_health value on
	// map change when game is not saved first
	if (other->client)
		other->client->pers.max_health = other->max_health;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Bandolier (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	if (other->client->pers.max_bullets < sk_bando_bullets->value)
		other->client->pers.max_bullets = sk_bando_bullets->value;
	if (other->client->pers.max_shells < sk_bando_shells->value)
		other->client->pers.max_shells = sk_bando_shells->value;
	if (other->client->pers.max_cells < sk_bando_cells->value)
		other->client->pers.max_cells = sk_bando_cells->value;
	if (other->client->pers.max_slugs < sk_bando_slugs->value)
		other->client->pers.max_slugs = sk_bando_slugs->value;
	// RAFAEL
	if (other->client->pers.max_magslug < sk_bando_magslugs->value)
		other->client->pers.max_magslug = sk_bando_magslugs->value;
	// Rogue
	if (other->client->pers.max_flechettes < sk_bando_flechettes->value)
		other->client->pers.max_flechettes = sk_bando_flechettes->value;
	if (other->client->pers.max_rounds < sk_bando_rounds->value)
		other->client->pers.max_rounds = sk_bando_rounds->value;

	item = FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Pack (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

// AJ - added change to max health
//      also changed absolute values to cvar references
// FIXME: need sk_pack_armor->value reference... but add to what type of armor?
	if (other->client->pers.max_health < sk_pack_health->value)
		other->client->pers.max_health = sk_pack_health->value;
	other->max_health = other->client->pers.max_health;
	if (other->client->pers.max_bullets < sk_pack_bullets->value)
		other->client->pers.max_bullets = sk_pack_bullets->value;
	if (other->client->pers.max_shells < sk_pack_shells->value)
		other->client->pers.max_shells = sk_pack_shells->value;
	if (other->client->pers.max_rockets < sk_pack_rockets->value)
		other->client->pers.max_rockets = sk_pack_rockets->value;
	if (other->client->pers.max_grenades < sk_pack_grenades->value)
		other->client->pers.max_grenades = sk_pack_grenades->value;
	if (other->client->pers.max_cells < sk_pack_cells->value)
		other->client->pers.max_cells = sk_pack_cells->value;
	if (other->client->pers.max_slugs < sk_pack_slugs->value)
		other->client->pers.max_slugs = sk_pack_slugs->value;
	// RAFAEL
	if (other->client->pers.max_magslug < sk_pack_magslugs->value)
		other->client->pers.max_magslug = sk_pack_magslugs->value;
	// Rogue
	if (other->client->pers.max_flechettes < sk_pack_flechettes->value)
		other->client->pers.max_flechettes = sk_pack_flechettes->value;
	if (other->client->pers.max_rounds < sk_pack_rounds->value)
		other->client->pers.max_rounds = sk_pack_rounds->value;
	if (other->client->pers.max_prox < sk_pack_prox->value)
		other->client->pers.max_prox = sk_pack_prox->value;
	if (other->client->pers.max_tesla < sk_pack_tesla->value)
		other->client->pers.max_tesla = sk_pack_tesla->value;
	if (other->client->pers.max_trap < sk_pack_traps->value)
		other->client->pers.max_trap  = sk_pack_traps->value;
	if (other->client->pers.max_armor < sk_pack_armor->value)
		other->client->pers.max_armor = sk_pack_armor->value;
// end AJ

	item = FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	item = FindItem("Cells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_cells)
			other->client->pers.inventory[index] = other->client->pers.max_cells;
	}

	item = FindItem("Grenades");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_grenades)
			other->client->pers.inventory[index] = other->client->pers.max_grenades;
	}

	item = FindItem("Rockets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
			other->client->pers.inventory[index] = other->client->pers.max_rockets;
	}

	item = FindItem("Slugs");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_slugs)
			other->client->pers.inventory[index] = other->client->pers.max_slugs;
	}

	// RAFAEL
	item = FindItem ("Mag Slug");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_magslug)
			other->client->pers.inventory[index] = other->client->pers.max_magslug;
	}

	item = FindItem ("Flechettes");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_flechettes)
			other->client->pers.inventory[index] = other->client->pers.max_flechettes;
	}

	item = FindItem ("Prox");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_flechettes)
			other->client->pers.inventory[index] = other->client->pers.max_flechettes;
	}

	item = FindItem ("Rounds");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_rounds)
			other->client->pers.inventory[index] = other->client->pers.max_rounds;
	}
	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

// Knightmare added- Ammogen tech-spawned backpack
qboolean Pickup_AmmogenPack (edict_t *ent, edict_t *other)
{
	gitem_t	*item;

	if (!ent || !other)
		return false;

	// Knightmare- override ammo pickup values with cvars
//	SetAmmoPickupValues ();

	item = FindItem("Bullets");
	if (item && other->client->pers.inventory[ITEM_INDEX(item)])
		Add_Ammo (other, item, item->quantity);

	item = FindItem("Shells");
	if (item && other->client->pers.inventory[ITEM_INDEX(item)])
		Add_Ammo (other, item, item->quantity);

	item = FindItem("Cells");
	if (item && other->client->pers.inventory[ITEM_INDEX(item)])
		Add_Ammo (other, item, item->quantity);

	item = FindItem("Grenades");
	if (item && other->client->pers.inventory[ITEM_INDEX(item)])
		Add_Ammo (other, item, item->quantity);

	item = FindItem("Rockets");
	if (item && other->client->pers.inventory[ITEM_INDEX(item)])
		Add_Ammo (other, item, item->quantity);

	item = FindItem("Slugs");
	if (item && other->client->pers.inventory[ITEM_INDEX(item)])
		Add_Ammo (other, item, item->quantity);

	item = FindItem ("Mag Slug");
	if (item && other->client->pers.inventory[ITEM_INDEX(item)])
		Add_Ammo (other, item, item->quantity);

	item = FindItem ("Flechettes");
	if (item && other->client->pers.inventory[ITEM_INDEX(item)])
		Add_Ammo (other, item, item->quantity);

	item = FindItem ("Prox");
	if (item && other->client->pers.inventory[ITEM_INDEX(item)])
		Add_Ammo (other, item, item->quantity);

	item = FindItem ("Tesla");
	if (item && other->client->pers.inventory[ITEM_INDEX(item)])
		Add_Ammo (other, item, item->quantity);

	item = FindItem ("Rounds");
	if (item && other->client->pers.inventory[ITEM_INDEX(item)])
		Add_Ammo (other, item, item->quantity);

	return true;
}

//============ ROGUE (aj) ==============================================

// ================
// PMM
qboolean Pickup_Nuke (edict_t *ent, edict_t *other)
{
	int		quantity;

//	if (!deathmatch->value)
//		return;
	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
//	if ((skill->value == 1 && quantity >= 2) || (skill->value >= 2 && quantity >= 1))
//		return false;

	if (quantity >= sk_nuke_max->value)
		return false;

	if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
	}

	return true;
}

// ================
// PGM
void Use_IR (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->ir_framenum > level.framenum)
		ent->client->ir_framenum += (sk_ir_time->value * 10);
	else
		ent->client->ir_framenum = level.framenum + (sk_ir_time->value * 10);

	gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/ir_start.wav"), 1, ATTN_NORM, 0);
}

void Use_Double (edict_t *ent, gitem_t *item)
{
	int		timeout;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (double_drop_timeout_hack)
	{
		timeout = double_drop_timeout_hack;
		double_drop_timeout_hack = 0;
	}
	else
		timeout = (sk_double_time->value * 10);

	if (ent->client->double_framenum > level.framenum)
		ent->client->double_framenum += timeout;
	else
		ent->client->double_framenum = level.framenum + timeout;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/ddamage1.wav"), 1, ATTN_NORM, 0);

	if (ent->bot_client) // select best weapon for Double
		botPickBestWeapon(ent);
}

/*
void Use_Torch (edict_t *ent, gitem_t *item)
{
	ent->client->torch_framenum = level.framenum + 600;
}
*/

void Use_Compass (edict_t *ent, gitem_t *item)
{
	int ang;

	ang = (int)(ent->client->v_angle[1]);
	if(ang<0)
		ang += 360;

	gi.cprintf(ent, PRINT_HIGH, "Origin: %0.0f,%0.0f,%0.0f    Dir: %d\n", ent->s.origin[0], ent->s.origin[1],
				ent->s.origin[2], ang);
}

void Use_Nuke (edict_t *ent, gitem_t *item)
{
	vec3_t	forward, right, start;
	float	speed;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorCopy (ent->s.origin, start);
	speed = 100;
	fire_nuke (ent, start, forward, speed);
}

void Use_Doppleganger (edict_t *ent, gitem_t *item)
{
	vec3_t		forward, right;
	vec3_t		createPt, spawnPt;
	vec3_t		ang;

	VectorClear(ang);
	ang[YAW] = ent->client->v_angle[YAW];
	AngleVectors (ang, forward, right, NULL);

	VectorMA(ent->s.origin, 48, forward, createPt);

	if(!FindSpawnPoint(createPt, ent->mins, ent->maxs, spawnPt, 32))
		return;

	if(!CheckGroundSpawnPoint(spawnPt, ent->mins, ent->maxs, 64, -1))
		return;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	SpawnGrow_Spawn (spawnPt, 0);
	fire_doppleganger (ent, spawnPt, forward);
}

qboolean Pickup_Doppleganger (edict_t *ent, edict_t *other)
{
	int		quantity;

	if (!(deathmatch->value))		// item is DM only
		return false;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	if (quantity >= sk_doppleganger_max->value)		// FIXME - apply max to dopplegangers
		return false;

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (!(ent->spawnflags & DROPPED_ITEM) )
		SetRespawn (ent, ent->item->quantity);

	if (other->bot_client) //ScarFace- bots always activate on pickup
		ent->item->use (other, ent->item);

	return true;
}


qboolean Pickup_Sphere (edict_t *ent, edict_t *other)
{
	int		quantity;

/*	if(other->client && other->client->owned_sphere)
	{
//		gi.cprintf(other, PRINT_HIGH, "Only one sphere to a customer!\n");
		return false;
	}
*/
	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	if ( ((skill->value == 1) && (quantity >= sk_powerup_max->value)) || ((skill->value >= 2) && (quantity >= sk_powerup_max->value)) )
		return false;

	if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;
	//ScarFace- since bots can't use items stuffed in inventory,
	// don't let them pick it up if they have an active sphere
	if (other->bot_client && other->client->owned_sphere && !ctf->value)
		return false;

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		if (((int)dmflags->value & DF_INSTANT_ITEMS))
		{
//PGM
			if(ent->item->use)
				ent->item->use (other, ent->item);
			else
				gi.dprintf("Powerup has no use function!\n");
//PGM
		}
		//ScarFace- bots always activate on pickup, except for CTF
		if (other->bot_client && (!ctf->value || CarryingFlag(other)) )
			ent->item->use (other, ent->item);
	}

	return true;
}

void Use_Defender (edict_t *ent, gitem_t *item)
{
	if(ent->client && ent->client->owned_sphere)
	{
		gi.cprintf(ent, PRINT_HIGH, "Only one sphere at a time!\n");
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	Defender_Launch (ent);
}

void Use_Hunter (edict_t *ent, gitem_t *item)
{
	if(ent->client && ent->client->owned_sphere)
	{
		gi.cprintf(ent, PRINT_HIGH, "Only one sphere at a time!\n");
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	Hunter_Launch (ent);
}

void Use_Vengeance (edict_t *ent, gitem_t *item)
{
	if(ent->client && ent->client->owned_sphere)
	{
		gi.cprintf(ent, PRINT_HIGH, "Only one sphere at a time!\n");
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	Vengeance_Launch (ent);
}

// PGM
// ================


//======================================================================

void Use_Quad (edict_t *ent, gitem_t *item)
{
	int		timeout;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (quad_drop_timeout_hack)
	{
		timeout = quad_drop_timeout_hack;
		quad_drop_timeout_hack = 0;
	}
	else
		timeout = (sk_quad_time->value * 10);

	if (ent->client->quad_framenum > level.framenum)
		ent->client->quad_framenum += timeout;
	else
		ent->client->quad_framenum = level.framenum + timeout;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);

	if (ent->bot_client) // select best weapon for Quad
		botPickBestWeapon(ent);
}

// =====================================================================

// RAFAEL
void Use_QuadFire (edict_t *ent, gitem_t *item)
{
	int		timeout;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (quad_fire_drop_timeout_hack)
	{
		timeout = quad_fire_drop_timeout_hack;
		quad_fire_drop_timeout_hack = 0;
	}
	else
		timeout = (sk_quad_fire_time->value * 10);

	if (ent->client->quadfire_framenum > level.framenum)
		ent->client->quadfire_framenum += timeout;
	else
		ent->client->quadfire_framenum = level.framenum + timeout;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/quadfire1.wav"), 1, ATTN_NORM, 0);

	if (ent->bot_client) // select best weapon for QuadFire
		botPickBestWeapon(ent);
}

//======================================================================

void Use_Breather (edict_t *ent, gitem_t *item)
{
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return;

	client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (client->breather_framenum > level.framenum)
		client->breather_framenum += (sk_breather_time->value * 10);
	else
		client->breather_framenum = level.framenum + (sk_breather_time->value * 10);

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Envirosuit (edict_t *ent, gitem_t *item)
{
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return;

	client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (client->enviro_framenum > level.framenum)
		client->enviro_framenum += (sk_enviro_time->value * 10);
	else
		client->enviro_framenum = level.framenum + (sk_enviro_time->value * 10);

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Invulnerability (edict_t *ent, gitem_t *item)
{
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return;

	client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (client->invincible_framenum > level.framenum)
		client->invincible_framenum += (sk_inv_time->value * 10);
	else
		client->invincible_framenum = level.framenum + (sk_inv_time->value * 10);

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void	Use_Silencer (edict_t *ent, gitem_t *item)
{
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return;

	client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	client->silencer_shots += sk_silencer_shots->value;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

qboolean Pickup_Key (edict_t *ent, edict_t *other)
{
	if (coop->value)
	{
		if (strcmp(ent->classname, "key_power_cube") == 0)
		{
			if (other->client->pers.power_cubes & ((ent->spawnflags & 0x0000ff00)>> 8))
				return false;
			other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
			other->client->pers.power_cubes |= ((ent->spawnflags & 0x0000ff00) >> 8);
		}
		else
		{
			if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
				return false;
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
		}
		return true;
	}
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	return true;
}

//======================================================================

qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count)
{
	int			index;
	int			max;
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return false;

	if (item->tag == AMMO_BULLETS)
		max = client->pers.max_bullets;
	else if (item->tag == AMMO_SHELLS)
		max = client->pers.max_shells;
	else if (item->tag == AMMO_ROCKETS)
		max = client->pers.max_rockets;
	else if (item->tag == AMMO_GRENADES)
		max = client->pers.max_grenades;
	else if (item->tag == AMMO_CELLS)
		max = client->pers.max_cells;
	else if (item->tag == AMMO_SLUGS)
		max = client->pers.max_slugs;
	// RAFAEL
	else if (item->tag == AMMO_MAGSLUG)
		max = ent->client->pers.max_magslug;
	// RAFAEL
	else if (item->tag == AMMO_TRAP)
		max = ent->client->pers.max_trap;
// ROGUE
//	else if (item->tag == AMMO_MINES)
//		max = ent->client->pers.max_mines;
	else if (item->tag == AMMO_FLECHETTES)
		max = ent->client->pers.max_flechettes;
	else if (item->tag == AMMO_PROX)
		max = ent->client->pers.max_prox;
	else if (item->tag == AMMO_TESLA)
		max = ent->client->pers.max_tesla;
#ifndef KILL_DISRUPTOR
	else if (item->tag == AMMO_DISRUPTOR)
		max = ent->client->pers.max_rounds;
#endif
// ROGUE
	else
		return false;

	index = ITEM_INDEX(item);

	if (client->pers.inventory[index] == max)
		return false;

	client->pers.inventory[index] += count;

	if (client->pers.inventory[index] > max)
		client->pers.inventory[index] = max;

	return true;
}

qboolean Pickup_Ammo (edict_t *ent, edict_t *other)
{
	int			oldcount;
	int			count;
	qboolean	weapon;

	weapon = (ent->item->flags & IT_WEAPON);
	if ( (weapon) && ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		count = 1000;
	else if (ent->count)
		count = ent->count;
	else
		count = ent->item->quantity;

	oldcount = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (!Add_Ammo (other, ent->item, count))
	{
		if (other->bot_client && (other->movetarget == ent))
		{	// ignore this item for a while
			ent->ignore_time = level.time + 3;
			other->movetarget = NULL;
			bot_roam(other, false);
		}

		return false;
	}

	if (weapon && !oldcount)
	{
		if (other->client->pers.weapon != ent->item && ( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
			other->client->newweapon = ent->item;
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, 30);
	return true;
}

void Drop_Ammo (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	int		index;
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return;

	index = ITEM_INDEX(item);
	dropped = Drop_Item (ent, item);
	if (client->pers.inventory[index] >= item->quantity)
		dropped->count = item->quantity;
	else
		dropped->count = client->pers.inventory[index];
	client->pers.inventory[index] -= dropped->count;
	ValidateSelectedItem (ent);
}


//======================================================================

void MegaHealth_think (edict_t *self)
{
	if (self->owner->health > self->owner->max_health
//ZOID
		&& !CTFHasRegeneration(self->owner)
//ZOID
		)
	{
		self->nextthink = level.time + 1;
		self->owner->health -= 1;
		return;
	}

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (self, 20);
	else
		G_FreeEdict (self);
}

qboolean Pickup_Health (edict_t *ent, edict_t *other)
{
	if (!(ent->style & HEALTH_IGNORE_MAX))
		if (other->health >= other->max_health)
		{
			if (other->bot_client && (other->movetarget == ent))
			{	// ignore this item for a while
				ent->ignore_time = level.time + 1;
				other->movetarget = NULL;
				bot_roam(other, false);
			}

			return false;
		}

//ZOID
	if (other->health >= 250 && ent->count > 25)
		return false;
//ZOID

	other->health += ent->count;

//ZOID
	if (other->health > 250 && ent->count > 25)
		other->health = 250;
//ZOID

	if (ent->count == sk_health_bonus_value->value)
		ent->item->pickup_sound = "items/s_health.wav";
	else if (ent->count == 10)
		ent->item->pickup_sound = "items/n_health.wav";
	else if (ent->count == 25)
		ent->item->pickup_sound = "items/l_health.wav";
	else // (ent->count == 100)
		ent->item->pickup_sound = "items/m_health.wav";

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health > other->max_health)
			other->health = other->max_health;
	}

//ZOID
	if ((ent->style & HEALTH_TIMED)
		&& !CTFHasRegeneration(other))
//ZOID
	{
		ent->think = MegaHealth_think;
		ent->nextthink = level.time + 5;
		ent->owner = other;
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	else
	{
		if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
			SetRespawn (ent, 30);
	}

	return true;
}

//======================================================================

int ArmorIndex (edict_t *ent)
{
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return false;

	if (client->pers.inventory[jacket_armor_index] > 0)
		return jacket_armor_index;

	if (client->pers.inventory[combat_armor_index] > 0)
		return combat_armor_index;

	if (client->pers.inventory[body_armor_index] > 0)
		return body_armor_index;

	return 0;
}

qboolean Pickup_Armor (edict_t *ent, edict_t *other)
{
	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int			newcount;
	float			salvage;
	int			salvagecount;
	int			armor_maximum;
	gclient_t	*client;

	//set armor cap
	armor_maximum = other->client->pers.max_armor;

	if (other->client)
		client = other->client;
	else
		return false;

	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;

	old_armor_index = ArmorIndex (other);

	// handle armor shards specially
	if (ent->item->tag == ARMOR_SHARD)
	{
		if (!old_armor_index)
			client->pers.inventory[jacket_armor_index] = sk_armor_bonus_value->value;
		else
			client->pers.inventory[old_armor_index] += sk_armor_bonus_value->value;
	}

	// if player has no armor, just use it
	else if (!old_armor_index)
	{
		client->pers.inventory[ITEM_INDEX(ent->item)] = newinfo->base_count;
	}

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
		//if stroner than current armor (always pick up)
		if (newinfo->normal_protection > oldinfo->normal_protection)
		{
			// calc new armor values
			salvage = oldinfo->normal_protection / newinfo->normal_protection;
			salvagecount = salvage * client->pers.inventory[old_armor_index];
			newcount = newinfo->base_count + salvagecount;
			if (newcount > armor_maximum)
				newcount = armor_maximum;

			// zero count of old armor so it goes away
			client->pers.inventory[old_armor_index] = 0;

			// change armor to new item with computed value
			client->pers.inventory[ITEM_INDEX(ent->item)] = newcount;

		}
		else  //if picking up same or weaker armor
		{
			// calc new armor values
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = salvage * newinfo->base_count;
			newcount = client->pers.inventory[old_armor_index] + salvagecount;
			if (newcount > armor_maximum)
				newcount = armor_maximum;

			// if we're already maxed out then we don't need the new armor
			if (client->pers.inventory[old_armor_index] >= armor_maximum)
			{
				if (other->bot_client && (other->movetarget == ent))
				{
					other->movetarget = NULL;
				}
				ent->ignore_time = level.time + 2;
				return false;
			}
			client->pers.inventory[old_armor_index] = newcount;

		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 20);

	return true;
}

//======================================================================

int PowerArmorType (edict_t *ent)
{
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return POWER_ARMOR_NONE;

	if (!(ent->flags & FL_POWER_ARMOR))
		return POWER_ARMOR_NONE;

	if (client->pers.inventory[power_shield_index] > 0)
		return POWER_ARMOR_SHIELD;

	if (client->pers.inventory[power_screen_index] > 0)
		return POWER_ARMOR_SCREEN;

	return POWER_ARMOR_NONE;
}

void Use_PowerArmor (edict_t *ent, gitem_t *item)
{
	int		index;

	if (ent->flags & FL_POWER_ARMOR)
	{
		ent->flags &= ~FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		index = ITEM_INDEX(FindItem("cells"));
		if (!ent->client->pers.inventory[index])
		{
			if (!ent->bot_client)
				gi.cprintf (ent, PRINT_HIGH, "No cells for power armor.\n");
			return;
		}
		ent->flags |= FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
	}
}

qboolean Pickup_PowerArmor (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		// auto-use for DM only if we didn't already have one
		if (!quantity)
			ent->item->use (other, ent->item);
	}

	return true;
}

void Drop_PowerArmor (edict_t *ent, gitem_t *item)
{
	if ((ent->flags & FL_POWER_ARMOR) && (ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
		Use_PowerArmor (ent, item);
	Drop_General (ent, item);
}

//======================================================================

/*
===============
Touch_Item
===============
*/
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean	taken;
	gclient_t	*client;

	if (other->bot_client)
	{
		if	(!(	 (ent->item->pickup == Pickup_Weapon)
			  || (ent->item->pickup == Pickup_Health)
			  || (ent->item->pickup == Pickup_Ammo)
// added AJ
			  || (ent->item->pickup == Pickup_Adrenaline)
			  || (ent->item->pickup == Pickup_AncientHead)
			  || (ent->item->pickup == Pickup_PowerArmor)
// end AJ
			  || (ent->item->pickup == Pickup_Powerup)
			  || (ent->item->pickup == Pickup_Armor)
			  || (ent->item->pickup == Pickup_Pack)
			  || (ent->item->pickup == Pickup_Bandolier)
			  || (ent->item->pickup == CTFPickup_Flag)
			  || (ent->item->pickup == CTFPickup_Tech)
			  || (ent->item->pickup == Pickup_Doppleganger)
			  || (ent->item->pickup == Pickup_Sphere)
			  || (ent->item->pickup == Tag_PickupToken))) //ScarFace- DM tag support
		{
			return;
		}
		else if (other->movetarget == ent)	// if this was their movetarget, then clear it
		{
			other->movetarget = other->goalentity = NULL;

			// force search for new item next think
			other->last_nopaths_roam = 0;
			other->last_roam_time   = 0;
		}

	}

	if (other->client)
		client = other->client;
	else
		return;

	if (other->health < 1)
		return;		// dead people can't pickup
	if (!ent->item->pickup)
		return;		// not a grabbable item?

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

		gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
	}

	if (!(ent->spawnflags & ITEM_TARGETS_USED))
	{
		G_UseTargets (ent, other);
		ent->spawnflags |= ITEM_TARGETS_USED;
	}

	if (!taken)
		return;

	if (!((coop->value) &&  (ent->item->flags & IT_STAY_COOP)) || (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
	{
		if (ent->flags & FL_RESPAWN)
		{
			ent->flags &= ~FL_RESPAWN;
		}
		else
		{
			if ((ent->item->pickup == Pickup_Weapon) ||
				(ent->item->pickup == Pickup_Health) ||
				(ent->item->pickup == Pickup_Ammo) ||
				(ent->item->pickup == Pickup_Powerup) ||
				(ent->item->pickup == Pickup_Armor) ||
				(ent->item->pickup == CTFPickup_Flag) ||
				(ent->item->pickup == CTFPickup_Tech))
			{
				RemoveFromItemList(ent);
			}

			G_FreeEdict (ent);
		}
	}

}

//======================================================================

static void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	Touch_Item (ent, other, plane, surf);
}

static void drop_make_touchable (edict_t *ent)
{
	ent->touch = Touch_Item;
	if (deathmatch->value)
	{
		// Set visible nodes
		if ((ent->item->pickup == Pickup_Weapon) ||
			(ent->item->pickup == Pickup_Health) ||
			(ent->item->pickup == Pickup_Ammo)   ||
			(ent->item->pickup == Pickup_Armor)  ||
			(ent->item->pickup == Pickup_Powerup) ||
			(ent->item->pickup == CTFPickup_Flag) ||
			(ent->item->pickup == CTFPickup_Tech))
		{
			CalcItemPaths(ent);

			ent->nextthink = level.time + 29;
			ent->think = RemoveDroppedItem;
		}
		else
		{
			ent->nextthink = level.time + 29;
			ent->think = G_FreeEdict;
		}
	}
}

edict_t *Drop_Item (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	vec3_t	forward, right;
	vec3_t	offset;

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW | RF_IR_VISIBLE;
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	gi.setmodel (dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
		trace = gi.trace (ent->s.origin, dropped->mins, dropped->maxs,
			dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}

	VectorScale (forward, 100, dropped->velocity);
	dropped->velocity[2] = 300;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	return dropped;
}

void Use_Item (edict_t *ent, edict_t *other, edict_t *activator)
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

	gi.linkentity (ent);
}

//======================================================================
void Tag_Respawn (edict_t *ent);
void Tag_MakeTouchable (edict_t *ent);

/*
================
droptofloor
================
*/
void droptofloor (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	if (ent->spawnflags & ITEM_NO_DROPTOFLOOR)	// Knightmare- added no_droptofloor
		ent->movetype = MOVETYPE_NONE;
	else
		ent->movetype = MOVETYPE_TOSS;
	ent->touch = Touch_Item;

	if (!(ent->spawnflags & ITEM_NO_DROPTOFLOOR))	// Knightmare- allow marked items to spawn in solids
	{
		v = tv(0, 0, -128);
		VectorAdd (ent->s.origin, v, dest);

		tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
		if (tr.startsolid)
		{
			// RAFAEL
			if (strcmp (ent->classname, "foodcube") == 0)
			{
				VectorCopy (ent->s.origin, tr.endpos);
				ent->velocity[2] = 0;
			}
			else
			{
				gi.dprintf ("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
				G_FreeEdict (ent);
				return;
			}
		}

		VectorCopy (tr.endpos, ent->s.origin);
	}

	// ScarFace- set tag token to respawn
	if (!strcmp(ent->classname, "dm_tag_token"))
	{
	//	ent->think = Tag_Respawn;
	//	ent->nextthink = level.time + 30;
		ent->think = Tag_MakeTouchable;
		ent->nextthink = level.time + 1;
	}
	else if (ent->team)
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

	if (ent->spawnflags & ITEM_NO_ROTATE)	// Knightmare added
		ent->s.effects &= ~EF_ROTATE;

	if (((int)dmflags->value & DF_INFINITE_AMMO) && (ent->item->pickup == Pickup_Ammo))
	{	// no ammo, since infinite ammo is set
		G_FreeEdict(ent);
		return;
	}

	// Set visible nodes
	if ((ent->item->pickup == Pickup_Weapon) ||
		(ent->item->pickup == Pickup_Health) ||
		(ent->item->pickup == Pickup_Ammo)   ||
		(ent->item->pickup == Pickup_Armor) ||
		(ent->item->pickup == Pickup_Powerup) ||
		(ent->item->pickup == Pickup_Pack) ||
		(ent->item->pickup == Pickup_Bandolier) ||
		(ent->item->pickup == CTFPickup_Flag) ||
		(ent->item->pickup == CTFPickup_Tech))
	{
		CalcItemPaths(ent);
	}

	gi.linkentity (ent);
}


/*
===============
PrecacheItem

Precaches all data needed for a given item.
This will be called for each item spawned in a level,
and for each item in each client's inventory.
===============
*/
void PrecacheItem (gitem_t *it)
{
	char	*s, *start;
	char	data[MAX_QPATH];
	int		len;
	gitem_t	*ammo;

	if (!it)
		return;

	if (it->pickup_sound)
		gi.soundindex (it->pickup_sound);
	if (it->world_model)
		gi.modelindex (it->world_model);
	if (it->view_model)
		gi.modelindex (it->view_model);
	if (it->icon)
		gi.imageindex (it->icon);

	// parse everything for its ammo
	if (it->ammo && it->ammo[0])
	{
		ammo = FindItem (it->ammo);
		if (ammo != it)
			PrecacheItem (ammo);
	}

	// parse the space seperated precache string for other items
	s = it->precaches;
	if (!s || !s[0])
		return;

	while (*s)
	{
		start = s;
		while (*s && *s != ' ')
			s++;

		len = s-start;
		if (len >= MAX_QPATH || len < 5)
			gi.error ("PrecacheItem: %s has bad precache string", it->classname);
		memcpy (data, start, len);
		data[len] = 0;
		if (*s)
			s++;

		// determine type based on extension
		if (!strcmp(data+len-3, "md2"))
			gi.modelindex (data);
		else if (!strcmp(data+len-3, "sp2"))
			gi.modelindex (data);
		else if (!strcmp(data+len-3, "wav"))
			gi.soundindex (data);
		if (!strcmp(data+len-3, "pcx"))
			gi.imageindex (data);
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
void SpawnItem (edict_t *ent, gitem_t *item)
{
// AJ - check for replaced items
	item = lithium_replace_item(item);
// end AJ
	PrecacheItem (item);

	if (ent->spawnflags)
	{
		if (strcmp(ent->classname, "key_power_cube") != 0)
		{
			ent->spawnflags = 0;
			gi.dprintf("%s at %s has invalid spawnflags set\n", ent->classname, vtos(ent->s.origin));
		}
	}

	// some items will be prevented in deathmatch
	if (deathmatch->value)
	{
		if ( (int)dmflags->value & DF_NO_ARMOR )
		{
			if (item->pickup == Pickup_Armor || item->pickup == Pickup_PowerArmor)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_NO_ITEMS )
		{
			if (item->pickup == Pickup_Powerup)
			{
				G_FreeEdict (ent);
				return;
			}
			//=====
			//ROGUE
			if (item->pickup  == Pickup_Sphere)
			{
				G_FreeEdict (ent);
				return;
			}
			if (item->pickup == Pickup_Doppleganger)
			{
				G_FreeEdict (ent);
				return;
			}
			//ROGUE
			//=====
		}
		if ( (int)dmflags->value & DF_NO_HEALTH )
		{
			if (item->pickup == Pickup_Health || item->pickup == Pickup_Adrenaline || item->pickup == Pickup_AncientHead)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		{
			if ( (item->flags == IT_AMMO) || (strcmp(ent->classname, "weapon_bfg") == 0) )
			{
				G_FreeEdict (ent);
				return;
			}
		}

//==========
//ROGUE
		if ( (int)dmflags->value & DF_NO_MINES || sk_no_mines->value)
		{
			if ( !strcmp(ent->classname, "ammo_prox") ||
				 !strcmp(ent->classname, "ammo_tesla") ||
				 !strcmp(ent->classname, "weapon_proxlauncher") )
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_NO_NUKES || sk_no_nukes->value)
		{
			if ( !strcmp(ent->classname, "ammo_nuke") )
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_NO_SPHERES || sk_no_spheres->value )
		{
			if (item->pickup  == Pickup_Sphere)
			{
				G_FreeEdict (ent);
				return;
			}
		}
//ROGUE
//==========

	}

// AJ no_* item banning
	if (!lithium_weaponbanning(ent))
		return;
// end AJ

//PGM
	PrecacheItem (item);
//PGM

	if (coop->value && (strcmp(ent->classname, "key_power_cube") == 0))
	{
		ent->spawnflags |= (1 << (8 + level.power_cubes));
		level.power_cubes++;
	}

	// don't let them drop items that stay in a coop game
	if ((coop->value) && (item->flags & IT_STAY_COOP))
	{
		item->drop = NULL;
	}
/*
//ZOID
//Don't spawn the flags unless enabled
	if (!ctf->value &&
		(strcmp(ent->classname, "item_flag_team1") == 0 ||
		strcmp(ent->classname, "item_flag_team2") == 0)) {
		G_FreeEdict(ent);
		return;
	}
//ZOID
*/
	ent->item = item;
	ent->movetarget = NULL;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	if (ent->model)
		gi.modelindex (ent->model);
	//ScarFace- add spheres and dopples to tiem list
	if (item->pickup  == Pickup_Sphere || item->pickup  == Pickup_Doppleganger)
		bonus_head = AddToItemList(ent, bonus_head);

//ZOID
//flags are server animated and have special handling
	if (strcmp(ent->classname, "item_flag_team1") == 0 ||
		strcmp(ent->classname, "item_flag_team2") == 0 ||
		strcmp(ent->classname, "item_flag_team3") == 0) { // AJ added 3rd flag
		ent->think = CTFFlagSetup;
	}
//ZOID
}

//======================================================================

gitem_t	itemlist[] =
{
	{
		NULL
	},	// leave index 0 alone

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
/* icon */		"i_bodyarmor",
/* pickup */	"Body Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&bodyarmor_info,
		ARMOR_BODY,
/* precache */ ""
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
/* icon */		"i_combatarmor",
/* pickup */	"Combat Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&combatarmor_info,
		ARMOR_COMBAT,
/* precache */ ""
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
/* icon */		"i_jacketarmor",
/* pickup */	"Jacket Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&jacketarmor_info,
		ARMOR_JACKET,
/* precache */ ""
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
/* icon */		"i_shard",
#else
/* icon */		"i_jacketarmor",
#endif
/* pickup */	"Armor Shard",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		ARMOR_SHARD,
/* precache */ ""
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
/* icon */		"i_powerscreen",
/* pickup */	"Power Screen",
/* width */		0,
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
/* precache */ "misc/power2.wav misc/power1.wav"
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
/* icon */		"i_powershield",
/* pickup */	"Power Shield",
/* width */		0,
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
/* precache */ "misc/power2.wav misc/power1.wav"
	},


	//
	// WEAPONS
	//

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
/* icon */		"w_grapple",
/* pickup */	"Grapple",
		0,
		0,
		NULL,
		IT_WEAPON,
		WEAP_GRAPPLE,
		NULL,
		0,
/* precache */ "weapons/grapple/grfire.wav weapons/grapple/grpull.wav weapons/grapple/grhang.wav weapons/grapple/grreset.wav weapons/grapple/grhit.wav"
	},

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blaster",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Blaster,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Blaster",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shotgun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Shotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
/* icon */		"w_shotgun",
/* pickup */	"Shotgun",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav"
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_supershotgun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_SuperShotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
/* icon */		"w_sshotgun",
/* pickup */	"Super Shotgun",
		0,
		2,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "weapons/sshotf1b.wav"
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_machinegun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Machinegun,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_machn/tris.md2",
/* icon */		"w_machinegun",
/* pickup */	"Machinegun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav"
	},

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_chaingun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Chaingun,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
/* icon */		"w_chaingun",
/* pickup */	"Chaingun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav weapons/chngnd1a.wav"
	},

		// ROGUE
/*QUAKED weapon_etf_rifle (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_etf_rifle",									// classname
		Pickup_Weapon,										// pickup function
		Use_Weapon,											// use function
		Drop_Weapon,										// drop function
		Weapon_ETF_Rifle,									// weapon think function
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_etf_rifle/tris.md2", EF_ROTATE,		// world model, world model flags
		"models/weapons/v_etf_rifle/tris.md2",					// view model
		"w_etf_rifle",										// icon
		"ETF Rifle",										// name printed when picked up
		0,													// number of digits for statusbar
		1,													// amount used / contained
		"Flechettes",										// ammo type used
		IT_WEAPON|IT_STAY_COOP|IT_ROGUE,					// inventory flags
		WEAP_ETFRIFLE,										// visible weapon
		NULL,												// info (void *)
		0,													// tag
		"weapons/nail1.wav models/proj/flechette/tris.md2",	// precaches
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Grenades",
/* width */		3,
		5,
		"grenades",
		IT_AMMO|IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		AMMO_GRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_GrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

/*QUAKED weapon_proxlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_proxlauncher",								// classname
		Pickup_Weapon,										// pickup
		Use_Weapon,											// use
		Drop_Weapon,										// drop
		Weapon_ProxLauncher,								// weapon think
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_plaunch/tris.md2", EF_ROTATE,		// world model, world model flags
		"models/weapons/v_plaunch/tris.md2",				// view model
		"w_proxlaunch",										// icon
		"Prox Launcher",									// name printed when picked up
		0,													// number of digits for statusbar
		1,													// amount used
		"Prox",												// ammo type used
		IT_WEAPON|IT_STAY_COOP|IT_ROGUE,					// inventory flags
		WEAP_PROXLAUNCH,									// visible weapon
		NULL,												// info (void *)
		AMMO_PROX,											// tag
		"weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav weapons/proxwarn.wav weapons/proxopen.wav",
	},

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
/* icon */		"w_rlauncher",
/* pickup */	"Rocket Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hyperblaster",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_HyperBlaster,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
/* icon */		"w_hyperblaster",
/* pickup */	"HyperBlaster",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav"	// removed weapons/hyprbu1a.wav
	},

/*QUAKED weapon_plasmabeam (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_plasmabeam",								// classname
		Pickup_Weapon,										// pickup function
		Use_Weapon,											// use function
		Drop_Weapon,										// drop function
		Weapon_Heatbeam,									// weapon think function
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_beamer/tris.md2", EF_ROTATE,		// world model, world model flags
		"models/weapons/v_beamer/tris.md2",					// view model
		"w_heatbeam",											// icon
		"Plasma Beam",											// name printed when picked up
		0,													// number of digits for statusbar
		// FIXME - if this changes, change it in NoAmmoWeaponChange as well
		2,													// amount used / contained
		"Cells",											// ammo type used
		IT_WEAPON|IT_STAY_COOP|IT_ROGUE,					// inventory flags
		WEAP_PLASMA,										// visible weapon
		NULL,												// info (void *)
		0,													// tag
		"models/weapons/v_beamer2/tris.md2 weapons/bfg__l1a.wav",		// precaches
	},

/*QUAKED weapon_boomer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_boomer",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Ionripper,
		"misc/w_pkup.wav",
		"models/weapons/g_boom/tris.md2", EF_ROTATE,
		"models/weapons/v_boomer/tris.md2",
/* icon */	"w_ripper",
/* pickup */ "Ionripper",
		0,
		2,
		"Cells",
		IT_WEAPON|IT_STAY_COOP|IT_XATRIX,
		WEAP_BOOMER,
		NULL,
		0,
#ifdef KMQUAKE2_ENGINE_MOD
		"weapons/ionactive.wav weapons/ion_hum.wav weapons/ionaway.wav weapons/rippfire.wav misc/lasfly.wav weapons/ionhit1.wav weapons/ionhit2.wav weapons/ionhit3.wav weapons/ionexp.wav models/objects/boomrang/tris.md2" // precache
#else
		"weapons/rippfire.wav misc/lasfly.wav models/objects/boomrang/tris.md2" // precache
#endif
	},
// END 14-APR-98

// SKWiD MOD
/*QUAKED weapon_plasma (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_plasma",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Plasma_Rifle,
		"misc/w_pkup.wav",
		PLASMA_MODEL_WORLD, EF_ROTATE,
		PLASMA_MODEL_VIEW,
		"w_plasma",		// icon
		PLASMA_PICKUP_NAME,	// pickup
		0,
		PLASMA_CELLS_PER_SHOT,	// bat - was 1
		"Cells",
		IT_WEAPON|IT_STAY_COOP|IT_LMCTF,
		WEAP_PLASMARIFLE,
		NULL,
		0,
		"PLASMA_PRECACHE"
	},

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
/* icon */		"w_railgun",
/* pickup */	"Railgun",
		0,
		1,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
		"weapons/rg_hum.wav weapons/railgf1a.wav" // precache
	},

/*QUAKED weapon_phalanx (.3 .3 1) (-16 -16 -16) (16 16 16)
*/

	{
		"weapon_phalanx",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Phalanx,
		"misc/w_pkup.wav",
		"models/weapons/g_shotx/tris.md2", EF_ROTATE,
		"models/weapons/v_shotx/tris.md2",
/* icon */	"w_phallanx",
/* pickup */ "Phalanx",
		0,
		1,
		"Mag Slug",
		IT_WEAPON|IT_STAY_COOP|IT_XATRIX,
		WEAP_PHALANX,
		NULL,
		0,
		"sprites/s_photon.sp2 weapons/plasshot.wav weapons/phaloop.wav" // precache
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BFG,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */	"BFG10K",
		0,
		50,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav"
	},

// =========================
// ROGUE WEAPONS

/*QUAKED weapon_disintegrator (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_disintegrator",								// classname
		Pickup_Weapon,										// pickup function
		Use_Weapon,											// use function
		Drop_Weapon,										// drop function
		Weapon_Disintegrator,								// weapon think function
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_dist/tris.md2", EF_ROTATE,		// world model, world model flags
		"models/weapons/v_dist/tris.md2",					// view model
		"w_disintegrator",									// icon
		"Disruptor",										// name printed when picked up
		0,													// number of digits for statusbar
		1,													// amount used / contained
		"Rounds",											// ammo type used
#ifdef KILL_DISRUPTOR
		IT_NOT_GIVEABLE,
#else
		IT_WEAPON|IT_STAY_COOP|IT_ROGUE,					// inventory flags
#endif
		WEAP_DISRUPTOR,										// visible weapon
		NULL,												// info (void *)
		1,													// tag
		"models/items/spawngro/tris.md2 models/proj/disintegrator/tris.md2 weapons/disrupt.wav weapons/disint2.wav weapons/disrupthit.wav",	// precaches
	},

/*QUAKED weapon_chainfist (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_chainfist",									// classname
		Pickup_Weapon,										// pickup function
		Use_Weapon,											// use function
		Drop_Weapon,										// drop function
		Weapon_ChainFist,									// weapon think function
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_chainf/tris.md2", EF_ROTATE,		// world model, world model flags
		"models/weapons/v_chainf/tris.md2",					// view model
		"w_chainfist",										// icon
		"Chainfist",										// name printed when picked up
		0,													// number of digits for statusbar
		0,													// amount used / contained
		NULL,												// ammo type used
		IT_WEAPON|IT_MELEE|IT_STAY_COOP|IT_ROGUE,			// inventory flags
		WEAP_CHAINFIST,										// visible weapon
		NULL,												// info (void *)
		1,													// tag
		"weapons/sawidle.wav weapons/sawhit.wav",			// precaches
	},

/*QUAKED weapon_shockwave (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shockwave",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Shockwave,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
		"w_chaingun", // icon
		"Shockwave", // pickup
		0,
		5,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SHOCKWAVE,
		NULL,
		0,
#ifdef KMQUAKE2_ENGINE_MOD
		"weapons/shockactive.wav weapons/shock_hum.wav weapons/shockfire.wav weapons/shockaway.wav weapons/shockhit.wav weapons/shockexp.wav models/objects/shocksphere/tris.md2 models/objects/shockfield/tris.md2 sprites/s_trap.sp2"
#else
		"weapons/shockfire.wav weapons/shockhit.wav weapons/shockexp.wav models/objects/shocksphere/tris.md2 models/objects/shockfield/tris.md2 sprites/s_trap.sp2"
#endif
	},

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
/* icon */		"a_shells",
/* pickup */	"Shells",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
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
/* icon */		"a_bullets",
/* pickup */	"Bullets",
/* width */		3,
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
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
/* icon */		"a_cells",
/* pickup */	"Cells",
/* width */		3,
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_CELLS,
/* precache */ ""
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
/* icon */		"a_rockets",
/* pickup */	"Rockets",
/* width */		3,
		5,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
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
/* icon */		"a_slugs",
/* pickup */	"Slugs",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SLUGS,
/* precache */ ""
	},

/*QUAKED ammo_magslug (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_magslug",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/objects/ammo/tris.md2", 0,
		NULL,
/* icon */		"a_mslugs",
/* pickup */	"Mag Slug",
/* width */		3,
		10,
		NULL,
		IT_AMMO|IT_XATRIX,
		0,
		NULL,
		AMMO_MAGSLUG,
/* precache */ ""
	},

/*QUAKED ammo_flechettes (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_flechettes",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/ammo/am_flechette/tris.md2", 0,
		NULL,
		"a_flechettes",
		"Flechettes",
		3,
		50,
		NULL,
		IT_AMMO|IT_ROGUE,
		0,
		NULL,
		AMMO_FLECHETTES,
/* precache */ ""
	},

/*QUAKED ammo_prox (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_prox",										// Classname
		Pickup_Ammo,										// pickup function
		NULL,												// use function
		Drop_Ammo,											// drop function
		NULL,												// weapon think function
		"misc/am_pkup.wav",									// pickup sound
		"models/ammo/am_prox/tris.md2", 0,					// world model, world model flags
		NULL,												// view model
		"a_prox",											// icon
		"Prox",												// Name printed when picked up
		3,													// number of digits for status bar
		5,													// amount contained
		NULL,												// ammo type used
		IT_AMMO|IT_ROGUE,									// inventory flags
		0,													// vwep index
		NULL,												// info (void *)
		AMMO_PROX,											// tag
		"models/weapons/g_prox/tris.md2 weapons/proxwarn.wav"	// precaches
	},

/*QUAKED ammo_tesla (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_tesla",
		Pickup_Ammo,
		Use_Weapon,						// PGM
		Drop_Ammo,
		Weapon_Tesla,					// PGM
		"misc/am_pkup.wav",
//		"models/weapons/g_tesla/tris.md2", 0,
		"models/ammo/am_tesl/tris.md2", 0,
		"models/weapons/v_tesla/tris.md2",
		"a_tesla",
		"Tesla",
		3,
		5,
		"Tesla",									// PGM
		IT_AMMO|IT_WEAPON|IT_ROGUE,					// inventory flags
		0,
		NULL,										// info (void *)
		AMMO_TESLA,									// tag
		"models/weapons/v_tesla2/tris.md2 weapons/teslaopen.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav models/weapons/g_tesla/tris.md2"			// precache
	},

/*QUAKED ammo_disruptor (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_disruptor",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/ammo/am_disr/tris.md2", 0,
		NULL,
		"a_disruptor",	//icon
		"Rounds",		//pickup
		3,				//width
		15,
		NULL,
#ifdef KILL_DISRUPTOR
		IT_NOT_GIVEABLE,
#else
		IT_AMMO|IT_ROGUE,						// inventory flags
#endif
		0,
		NULL,
#ifdef KILL_DISRUPTOR
		0,
#else
		AMMO_DISRUPTOR,
#endif
/* precache */ ""
	},

/*QUAKED ammo_trap (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_trap",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Trap,
		"misc/am_pkup.wav",
		"models/weapons/g_trap/tris.md2", EF_ROTATE,
		"models/weapons/v_trap/tris.md2",
/* icon */		"a_trap",
/* pickup */	"Trap",
/* width */		3,
		2,
		"trap",
		IT_AMMO|IT_WEAPON|IT_XATRIX,
		0,
		NULL,
		AMMO_TRAP,
/* precache */ "weapons/trapcock.wav weapons/traploop.wav weapons/trapsuck.wav weapons/trapdown.wav"
// "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED ammo_nuke (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_nuke",
		Pickup_Nuke,
		Use_Nuke,						// PMM
		Drop_Ammo,
		NULL,							// PMM
		"misc/am_pkup.wav",
		"models/weapons/g_nuke/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_nuke",
/* pickup */	"A-M Bomb",
/* width */		3,
		300, /* quantity (used for respawn time) */
		"A-M Bomb",
		IT_POWERUP|IT_ROGUE,
		0,
		NULL,
		0,
		"weapons/nukewarn2.wav world/rumble.wav"
	},

	//
	// POWERUP ITEMS
	//
/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_quad",
		Pickup_Powerup,
		Use_Quad,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/quaddama/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_quad",
/* pickup */	"Quad Damage",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/damage.wav items/damage2.wav items/damage3.wav"
	},

/*QUAKED item_double (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_double",
		Pickup_Powerup,
		Use_Double,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/ddamage/tris.md2", EF_ROTATE,
		NULL,
		"p_double",			//icon
		"Double Damage",	//pickup
		2,					//width
		60,
		NULL,
		IT_POWERUP|IT_ROGUE,
		0,
		NULL,
		0,
		"misc/ddamage1.wav misc/ddamage2.wav misc/ddamage3.wav"
	},

/*QUAKED item_quadfire (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_quadfire",
		Pickup_Powerup,
		Use_QuadFire,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/quadfire/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_quadfire",

/* pickup */	"DualFire Damage",
/* width */		2,
		60,
		NULL,
		IT_POWERUP|IT_XATRIX,
		0,
		NULL,
		0,
/* precache */ "items/quadfire1.wav items/quadfire2.wav items/quadfire3.wav"
	},

/*QUAKED item_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_invulnerability",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/invulner/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_invulnerability",
/* pickup */	"Invulnerability",
/* width */		2,
		300,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
	},

/*QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_silencer",
		Pickup_Powerup,
		Use_Silencer,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/silencer/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_silencer",
/* pickup */	"Silencer",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ ""
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
/* icon */		"p_rebreather",
/* pickup */	"Rebreather",
/* width */		2,
		60,
		NULL,
		IT_POWERUP|IT_STAY_COOP,
		0,
		NULL,
		0,
/* precache */ "items/airout.wav"
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
/* icon */		"p_envirosuit",
/* pickup */	"Environment Suit",
/* width */		2,
		60,
		NULL,
		IT_POWERUP|IT_STAY_COOP,
		0,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

/*QUAKED item_ir_goggles (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
gives +1 to maximum health
*/
	{
		"item_ir_goggles",
		Pickup_Powerup,
		Use_IR,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/goggles/tris.md2", EF_ROTATE,
		NULL,
		"p_ir",			//icon
		"IR Goggles",	//pickup
		2,				//width
		60,
		NULL,
		IT_POWERUP|IT_ROGUE,
		0,
		NULL,
		0,
		"misc/ir_start.wav"
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
/* icon */		"i_fixme",
/* pickup */	"Ancient Head",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
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
/* icon */		"p_adrenaline",
/* pickup */	"Adrenaline",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
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
/* icon */		"p_bandolier",
/* pickup */	"Bandolier",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
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
/* icon */		"i_pack",
/* pickup */	"Ammo Pack",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},



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
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
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
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
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
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
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
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
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
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
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
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
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
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

// RAFAEL
/*QUAKED key_green_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - green
*/
	{
		"key_green_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/green_key/tris.md2", EF_ROTATE,
		NULL,
		"k_green",
		"Green Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_XATRIX,
		0,
		NULL,
		0,
/* precache */ ""
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
/* icon */		"k_comhead",
/* pickup */	"Commander's Head",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
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
/* icon */		"i_airstrike",
/* pickup */	"Airstrike Marker",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
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
/* icon */		"i_health",
/* pickup */	"Health",
/* width */		3,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

//ZOID
/*QUAKED item_flag_team1 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
		"item_flag_team1",
		CTFPickup_Flag,
		NULL,
		CTFDrop_Flag, //Should this be null if we don't want players to drop it manually?
		NULL,
		"ctf/flagtk.wav",
		"players/male/flag1.md2", EF_FLAG1,
		NULL,
/* icon */		"i_ctf1",
/* pickup */	"Red Flag",
/* width */		2,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},

/*QUAKED item_flag_team2 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
		"item_flag_team2",
		CTFPickup_Flag,
		NULL,
		CTFDrop_Flag, //Should this be null if we don't want players to drop it manually?
		NULL,
		"ctf/flagtk.wav",
		"players/male/flag2.md2", EF_FLAG2,
		NULL,
/* icon */		"i_ctf2",
/* pickup */	"Blue Flag",
/* width */		2,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},

// AJ added third flag
/*QUAKED item_flag_team3 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
		"item_flag_team3",
		CTFPickup_Flag,
		NULL,
		CTFDrop_Flag, //Should this be null if we don't want players to drop it manually?
		NULL,
		"ctf/flagtk.wav",
#ifndef KMQUAKE2_ENGINE_MOD
		"models/ctf/flags/flag3.md2", EF_FLAG2,
#else
		"models/ctf/flags/flag3.md2", EF_FLAG1|EF_FLAG2,
#endif
		NULL,
/* icon */		"3tctfg",
/* pickup */	"Green Flag",
/* width */		2,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},
// end AJ

/* Resistance Tech */
	{
		"item_tech1",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, // Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/resistance/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech1",
/* pickup */	"Disruptor Shield",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech1.wav"
	},

/* Strength Tech */
	{
		"item_tech2",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, // Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/strength/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech2",
/* pickup */	"Power Amplifier",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech2.wav ctf/tech2x.wav"
	},

/* Haste Tech */
	{
		"item_tech3",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, // Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/haste/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech3",
/* pickup */	"Time Accel",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech3.wav"
	},

/* Regeneration Tech */
	{
		"item_tech4",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, // Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/regeneration/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech4",
/* pickup */	"AutoDoc",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech4.wav"
	},

//ZOID

// AJ
/* Vampire Rune */
	{
		"item_tech5",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, // Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech4",
/* pickup */	"Vampire",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "makron/pain2.wav"
	},
// end AJ

/* Ammogen Rune */
	{
		"item_tech6",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, // Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech4",
/* pickup */	"AmmoGen",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "misc/spawn1.wav"
	},

/* Ammogen-spawned Backpack */
	{
		"item_ammogen_pack",
		Pickup_AmmogenPack,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/pack/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_pack",
/* pickup */	"Backpack",
/* width */		2,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

// ======================================
// PGM

/*QUAKED item_sphere_vengeance (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_sphere_vengeance",
		Pickup_Sphere,
		Use_Vengeance,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/vengnce/tris.md2", EF_ROTATE,
		NULL,
		"p_vengeance",		//icon
		"Vengeance Sphere",	//pickup
		2,					//width
		60,
		NULL,
		IT_POWERUP|IT_ROGUE,
		0,
		NULL,
		0,
		"spheres/v_idle.wav"		// precache
	},

/*QUAKED item_sphere_hunter (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_sphere_hunter",
		Pickup_Sphere,
		Use_Hunter,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/hunter/tris.md2", EF_ROTATE,
		NULL,
		"p_hunter",			//icon
		"Hunter Sphere",	//pickup
		2,					//width
		120,
		NULL,
		IT_POWERUP|IT_ROGUE,
		0,
		NULL,
		0,
		"spheres/h_idle.wav spheres/h_active.wav spheres/h_lurk.wav"		// precache
	},

/*QUAKED item_sphere_defender (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_sphere_defender",
		Pickup_Sphere,
		Use_Defender,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/defender/tris.md2", EF_ROTATE,
		NULL,
		"p_defender",		//icon
		"Defender Sphere",	//pickup
		2,					//width
		60,													// respawn time
		NULL,												// ammo type used
		IT_POWERUP|IT_ROGUE,								// inventory flags
		0,
		NULL,												// info (void *)
		0,													// tag
		"models/proj/laser2/tris.md2 models/items/shell/tris.md2 spheres/d_idle.wav"		// precache
	},

/*QUAKED item_doppleganger (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_doppleganger",								// classname
		Pickup_Doppleganger,								// pickup function
		Use_Doppleganger,									// use function
		Drop_General,										// drop function
		NULL,												// weapon think function
		"items/pkup.wav",									// pick up sound
		"models/items/dopple/tris.md2",						// world model
		EF_ROTATE,											// world model flags
		NULL,												// view model
		"p_doppleganger",									// icon
		"Doppleganger",										// name printed when picked up
		0,													// number of digits for statusbar
		90,													// respawn time
		NULL,												// ammo type used
		IT_POWERUP|IT_ROGUE,								// inventory flags
		0,
		NULL,												// info (void *)
		0,													// tag
		"models/objects/dopplebase/tris.md2 models/items/spawngro2/tris.md2 models/items/hunter/tris.md2 models/items/vengnce/tris.md2",		// precaches
	},

	{
		"dm_tag_token",										// classname
//		NULL,												// classname
		Tag_PickupToken,									// pickup function
		NULL,												// use function
		NULL,												// drop function
		NULL,												// weapon think function
		"items/pkup.wav",									// pick up sound
		"models/items/tagtoken/tris.md2",					// world model
		EF_ROTATE | EF_TAGTRAIL,							// world model flags
		NULL,												// view model
		"i_tagtoken",										// icon
		"Tag Token",										// name printed when picked up
		0,													// number of digits for statusbar
		0,													// amount used / contained
		NULL,												// ammo type used
		IT_POWERUP|IT_NOT_GIVEABLE|IT_ROGUE,				// inventory flags
		0,
		NULL,												// info (void *)
		1,													// tag
		NULL,												// precaches
	},
// PGM
// ======================================

	// end of list marker
	{NULL}
};




/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/n_health.wav");
}

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_small (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/stimpack/tris.md2";
	self->count = sk_health_bonus_value->value;
	SpawnItem (self, FindItem ("Health"));
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex ("items/s_health.wav");
}

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_large (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/large/tris.md2";
	self->count = 25;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/l_health.wav");
}

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_mega (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/mega_h/tris.md2";
	self->count = 100;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;
}

// RAFAEL
void SP_item_foodcube (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/objects/trapfx/tris.md2";
	SpawnItem (self, FindItem ("Health"));
	self->spawnflags |= DROPPED_ITEM;
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex ("items/s_health.wav");
	self->classname = "foodcube";
}


void InitItems (void)
{
	game.num_items = sizeof(itemlist)/sizeof(itemlist[0]) - 1;
}



/*
===============
SetItemNames

Called by worldspawn
===============
*/
void SetItemNames (void)
{
	int		i;
	gitem_t	*it;

	for (i=0 ; i<game.num_items ; i++)
	{
		it = &itemlist[i];
		gi.configstring (CS_ITEMS+i, it->pickup_name);
	}

	jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
	combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
	body_armor_index   = ITEM_INDEX(FindItem("Body Armor"));
	power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
	power_shield_index = ITEM_INDEX(FindItem("Power Shield"));
	shells_index       = ITEM_INDEX(FindItem("Shells"));
	bullets_index      = ITEM_INDEX(FindItem("Bullets"));
	grenades_index     = ITEM_INDEX(FindItem("Grenades"));
	rockets_index      = ITEM_INDEX(FindItem("Rockets"));
	cells_index        = ITEM_INDEX(FindItem("Cells"));
	slugs_index        = ITEM_INDEX(FindItem("Slugs"));
	magslug_index	   = ITEM_INDEX(FindItem("Mag Slug"));
	flechettes_index   = ITEM_INDEX(FindItem("Flechettes"));
	prox_index         = ITEM_INDEX(FindItem("Prox"));
	disruptors_index   = ITEM_INDEX(FindItem("Rounds"));
	tesla_index        = ITEM_INDEX(FindItem("Tesla"));
	trap_index         = ITEM_INDEX(FindItem("Trap"));
	blaster_index      = ITEM_INDEX(FindItem("Blaster"));
	pl_index           = ITEM_INDEX(FindItem("Prox Launcher"));
	rg_index           = ITEM_INDEX(FindItem("Railgun"));
	pr_index		   = ITEM_INDEX(FindItem(PLASMA_PICKUP_NAME));		// SKWiD MOD
}
