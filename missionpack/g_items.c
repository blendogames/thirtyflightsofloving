#include "g_local.h"

qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);

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
void Weapon_HomingMissileLauncher (edict_t *ent);
void Weapon_Null(edict_t *ent);

// *** Zaero prototypes ***
void Weapon_FlareGun (edict_t *ent);
//void Weapon_SniperRifle(edict_t *ent);
void Weapon_LaserTripBomb(edict_t *ent);
//void Weapon_SonicCannon (edict_t *ent);
void Weapon_EMPNuke (edict_t *ent);
//void Weapon_A2k (edict_t *ent);
void Use_Visor (edict_t *ent, gitem_t *item);
//void Action_Push(edict_t *ent);
void Use_PlasmaShield (edict_t *ent, gitem_t *item);


gitem_armor_t jacketarmor_info	= { 25,  100, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 200, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {100, 10000, .80, .60, ARMOR_BODY};

int	noweapon_index;
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
int fuel_index;
int	homing_index;
int	blaster_index;
int rl_index;
int	hml_index;
int	pl_index;
int	pr_index;		// SKWiD MOD
int	magslug_index;
int	flechettes_index;
int	prox_index;
int	disruptors_index;
int	tesla_index;
int	trap_index;
int	shocksphere_index;
int	flares_index;
int	tbombs_index;
int	empnuke_index;
int	plasmashield_index;

// added for convenience with triger_key sound hack
int	key_q1_med_silver_index;
int	key_q1_med_gold_index;
int	key_q1_rune_silver_index;
int	key_q1_rune_gold_index;
int	key_q1_base_silver_index;
int	key_q1_base_gold_index;


#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2
#define	HEALTH_FOODCUBE		4
#define	HEALTH_SMALL		8
#define	HEALTH_LARGE		16


void Use_Quad (edict_t *ent, gitem_t *item);
void Use_Double (edict_t *ent, gitem_t *item);
// RAFAEL
void Use_QuadFire (edict_t *ent, gitem_t *item);

// added stasis generator support
void Use_Stasis (edict_t *ent, gitem_t *item);

static int	quad_drop_timeout_hack;
static int	double_drop_timeout_hack;
// RAFAEL
static int	quad_fire_drop_timeout_hack;

//======================================================================

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
	else if (item_index == fuel_index)
		value = client->pers.max_fuel;
	else if (item_index == homing_index)
		value = client->pers.max_homing_rockets;
	else if (item_index == magslug_index)
		value = client->pers.max_magslug;
	else if (item_index == flechettes_index)
		value = client->pers.max_flechettes;
	else if (item_index == prox_index)
		value = client->pers.max_prox;
	else if (item_index == disruptors_index)
		value = client->pers.max_disruptors;
	else if (item_index == tesla_index)
		value = client->pers.max_tesla;
	else if (item_index == trap_index)
		value = client->pers.max_trap;
	else if (item_index == shocksphere_index)
		value = client->pers.max_shockspheres;
	else if (item_index == flares_index)
		value = client->pers.max_flares;
	else if (item_index == tbombs_index)
		value = client->pers.max_tbombs;
	else if (item_index == empnuke_index)
		value = client->pers.max_empnuke;
	else if (item_index == plasmashield_index)
		value = client->pers.max_plasmashield;
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
		value = sk_max_armor_jacket->value;
	else if (item_index == combat_armor_index)
		value = sk_max_armor_combat->value;
	else if (item_index == body_armor_index)
		value = sk_max_armor_body->value;
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
gitem_t	*FindItem (char *pickup_name)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if (!it->pickup_name)
			continue;
		if (!Q_stricmp(it->pickup_name, pickup_name))
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

		for (count = 0, ent = master; ent; ent = ent->chain, count++)
			;

	//	choice = rand() % count;
		choice = count ? rand() % count : 0;

		for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
			;
	}

//=====
//ROGUE
	if (randomrespawn && randomrespawn->value)
	{
		edict_t *newEnt;

		newEnt = DoRandomRespawn (ent);
		
		// if we've changed entities, then do some sleight of hand.
		// otherwise, the old entity will respawn
		if (newEnt)
		{
			G_FreeEdict (ent);
			ent = newEnt;
		}
	}
//ROGUE
//=====

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
	if ((skill->value == 1 && quantity >= sk_powerup_max->value) || (skill->value >= 2 && quantity >= (sk_powerup_max->value - 1)))
		return false;

	if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;

	// Can only pickup one flashlight
	if (!strcmp(ent->classname, "item_flashlight") && quantity >= 1)
		return false;

#ifdef JETPACK_MOD
	if ( !Q_stricmp(ent->classname,"item_jetpack") )
	{
		gitem_t *fuel;

		if ( quantity >= 1 )
			return false;

		fuel = FindItem("Fuel");
		if (ent->count < 0)
		{
			other->client->jetpack_infinite = true;
			Add_Ammo(other,fuel,100000);
		}
		else
		{
			other->client->jetpack_infinite = false;
			Add_Ammo(other,fuel,ent->count);
		}
	}
#endif

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	// Knightmare- set cell usage for flashlight
	if (!strcmp(ent->classname, "item_flashlight"))
	{
		if (ent->count)
			other->client->flashlight_cell_usage = ent->count;
		else
			other->client->flashlight_cell_usage = 0;
	}
	
	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);

#ifdef JETPACK_MOD
		// DON'T Instant-use Jetpack
		if (ent->item->use == Use_Jet) return true;
#endif

		if ( ((int)dmflags->value & DF_INSTANT_ITEMS)
			|| ((ent->item->use == Use_Quad || ent->item->use == Use_Double || ent->item->use == Use_QuadFire)
			&& (ent->spawnflags & DROPPED_PLAYER_ITEM)) )
		{
			if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			if ((ent->item->use == Use_Double) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				double_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			if ((ent->item->use == Use_QuadFire) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_fire_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			ent->item->use (other, ent->item);
		}
//PGM
	/*	if (ent->item->use)
			ent->item->use (other, ent->item);
		else
			gi.dprintf("Powerup has no use function!\n");*/
//PGM
	
	}

	return true;
}

void Drop_General (edict_t *ent, gitem_t *item)
{
	Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
}

#ifdef JETPACK_MOD
void Drop_Jetpack (edict_t *ent, gitem_t *item)
{
	if (ent->client->jetpack)
		gi.cprintf(ent,PRINT_HIGH,"Cannot drop jetpack in use\n");
	else
	{
		edict_t	*dropped;

		dropped = Drop_Item (ent, item);
		if (ent->client->jetpack_infinite)
		{
			dropped->count = -1;
			ent->client->pers.inventory[fuel_index] = 0;
			ent->client->jetpack_infinite = false;
		}
		else
		{
			dropped->count = ent->client->pers.inventory[fuel_index];
			if (dropped->count > 500)
				dropped->count = 500;
			ent->client->pers.inventory[fuel_index] -= dropped->count;
		}
		ent->client->pers.inventory[ITEM_INDEX(item)]--;
		ValidateSelectedItem (ent);
	}
}
#endif

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

	// Knightmare- override ammo pickup values with cvars
	SetAmmoPickupValues ();

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

	// PMM
	if (other->client->pers.max_flechettes < sk_bando_flechettes->value)
		other->client->pers.max_flechettes = sk_bando_flechettes->value;
	if (other->client->pers.max_disruptors < sk_bando_rounds->value)
		other->client->pers.max_disruptors = sk_bando_rounds->value;
	// pmm

	if (other->client->pers.max_fuel  < sk_bando_fuel->value)
		other->client->pers.max_fuel  = sk_bando_fuel->value;

	// Zaero
	if (other->client->pers.max_flares < sk_bando_flares->value)
		other->client->pers.max_flares = sk_bando_flares->value;
	// end Zaero

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

	// Knightmare- override ammo pickup values with cvars
	SetAmmoPickupValues ();

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
	if (other->client->pers.max_magslug < sk_pack_magslugs->value)
		other->client->pers.max_magslug = sk_pack_magslugs->value;
	if (other->client->pers.max_trap < sk_pack_traps->value)
		other->client->pers.max_trap = sk_pack_traps->value;

	// PMM
	if (other->client->pers.max_flechettes < sk_pack_flechettes->value)
		other->client->pers.max_flechettes = sk_pack_flechettes->value;
	if (other->client->pers.max_prox < sk_pack_prox->value)
		other->client->pers.max_prox = sk_pack_prox->value;
	if (other->client->pers.max_tesla < sk_pack_tesla->value)
		other->client->pers.max_tesla = sk_pack_tesla->value;
	if (other->client->pers.max_disruptors < sk_pack_rounds->value)
		other->client->pers.max_disruptors = sk_pack_rounds->value;
	if (other->client->pers.max_shockspheres < sk_pack_shocksphere->value)
		other->client->pers.max_shockspheres = sk_pack_shocksphere->value;
	if (other->client->pers.max_homing_rockets < sk_pack_rockets->value)
		other->client->pers.max_homing_rockets = sk_pack_rockets->value;
	// pmm

	if (other->client->pers.max_fuel  < sk_pack_fuel->value)
		other->client->pers.max_fuel  = sk_pack_fuel->value;

	// Zaero
	if (other->client->pers.max_flares < sk_pack_flares->value)
		other->client->pers.max_flares = sk_pack_flares->value;
	if (other->client->pers.max_tbombs < sk_pack_tbombs->value)
		other->client->pers.max_tbombs = sk_pack_tbombs->value;
/*	if (other->client->pers.max_a2k < sk_pack_a2k->value)
		other->client->pers.max_a2k = sk_pack_a2k->value;*/
	if (other->client->pers.max_empnuke < sk_pack_empnuke->value)
		other->client->pers.max_empnuke = sk_pack_empnuke->value;
	if (other->client->pers.max_plasmashield < sk_pack_plasmashield->value)
		other->client->pers.max_plasmashield = sk_pack_plasmashield->value;
	// end Zaero


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

	item = FindItem("Magslug");
	if (item && sk_pack_give_xatrix_ammo->value)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_magslug)
			other->client->pers.inventory[index] = other->client->pers.max_magslug;
	}

// PMM
	item = FindItem("Flechettes");
	if (item && sk_pack_give_rogue_ammo->value)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_flechettes)
			other->client->pers.inventory[index] = other->client->pers.max_flechettes;
	}
	item = FindItem("Disruptors");
	if (item && sk_pack_give_rogue_ammo->value)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_disruptors)
			other->client->pers.inventory[index] = other->client->pers.max_disruptors;
	}
// pmm

// Zaero
	item = FindItem("Flares");
	if (item && sk_pack_give_zaero_ammo->value)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_empnuke)
			other->client->pers.inventory[index] = other->client->pers.max_empnuke;
	}

	item = FindItem("IRED");
	if (item && sk_pack_give_zaero_ammo->value)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_tbombs)
			other->client->pers.inventory[index] = other->client->pers.max_tbombs;
	}
/*
	item = FindItem("A2k");
	if (item && sk_pack_give_zaero_ammo->value)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_a2k)
			other->client->pers.inventory[index] = other->client->pers.max_a2k;
	}
*/
	item = FindItem("EMPNuke");
	if (item && sk_pack_give_zaero_ammo->value)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_empnuke)
			other->client->pers.inventory[index] = other->client->pers.max_empnuke;
	}

	item = FindItem("Plasma Shield");
	if (item && sk_pack_give_zaero_ammo->value)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_plasmashield)
			other->client->pers.inventory[index] = other->client->pers.max_plasmashield;
	}
// end Zaero

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}
// ================
// PMM
qboolean Pickup_Nuke (edict_t *ent, edict_t *other)
{
	int		quantity;

//	if (!deathmatch->value)
//		return;
	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (quantity >= sk_nuke_max->value)
		return false;

	/*if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;*/

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, 300);
	}

	return true;
}

qboolean Pickup_Nbomb (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (quantity >= sk_nbomb_max->value)
		return false;

	/*if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;*/

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, 300);
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
	{
		timeout = (sk_double_time->value * 10);
	}

	if (ent->client->double_framenum > level.framenum)
		ent->client->double_framenum += (sk_double_time->value * 10);
	else
		ent->client->double_framenum = level.framenum + (sk_double_time->value * 10);

	gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/ddamage1.wav"), 1, ATTN_NORM, 0);
}

/*
void Use_Torch (edict_t *ent, gitem_t *item)
{
	ent->client->torch_framenum = level.framenum + 6000;
}
*/

// Knightmare
void Use_Flashlight (edict_t *ent, gitem_t *item)
{
	if (ent->client->flashlight_active)
	{
		ent->client->flashlight_active = false;
		ent->client->flashlight_framenum = 0;
	}
	else
	{	//can't use flashlight if we don't have the cells
		if ((ent->client->flashlight_cell_usage > 0)
			&& (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < ent->client->flashlight_cell_usage))
			return;
		ent->client->flashlight_active = true;
		//Think every 60 seconds
		ent->client->flashlight_framenum = level.time + 60;
	}
}

void Use_Compass (edict_t *ent, gitem_t *item)
{
	int ang;

	ang = (int)(ent->client->v_angle[1]);
	if (ang<0)
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

void Use_Nbomb (edict_t *ent, gitem_t *item)
{
	vec3_t	forward, right, start;
	float	speed;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorCopy (ent->s.origin, start);
	speed = 100;
	fire_nbomb (ent, start, forward, speed);
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

	if (!FindSpawnPoint(createPt, ent->mins, ent->maxs, spawnPt, 32))
		return;

	if (!CheckGroundSpawnPoint(spawnPt, ent->mins, ent->maxs, 64, -1))
		return;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	SpawnGrow_Spawn (spawnPt, 0);
	fire_doppleganger (ent, spawnPt, forward);
}

qboolean Pickup_Doppleganger (edict_t *ent, edict_t *other)
{
	int		quantity;

	//if (!(deathmatch->value))		// item is DM only
		//return false;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	if (quantity >= sk_doppleganger_max->value)		// FIXME - apply max to dopplegangers
		return false;

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (!(ent->spawnflags & DROPPED_ITEM) )
		SetRespawn (ent, ent->item->quantity);

	return true;
}


qboolean Pickup_Sphere (edict_t *ent, edict_t *other)
{
	int		quantity;

	if (other->client && other->client->owned_sphere)
	{
//		gi.cprintf(other, PRINT_HIGH, "Only one sphere to a customer!\n");
	//	return false;
	}

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	if ((skill->value == 1 && quantity >= sk_powerup_max->value) || (skill->value >= 2 && quantity >= (sk_powerup_max->value - 1)))
		return false;

	if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		if (((int)dmflags->value & DF_INSTANT_ITEMS))
		{
//PGM
			if (ent->item->use)
				ent->item->use (other, ent->item);
			else
				gi.dprintf("Powerup has no use function!\n");
//PGM
		}
	}

	return true;
}

void Use_Defender (edict_t *ent, gitem_t *item)
{
	if (ent->client && ent->client->owned_sphere)
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
	if (ent->client && ent->client->owned_sphere)
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
	if (ent->client && ent->client->owned_sphere)
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
	{
		timeout = (sk_quad_time->value * 10);
	}

	if (ent->client->quad_framenum > level.framenum)
		ent->client->quad_framenum += timeout;
	else
		ent->client->quad_framenum = level.framenum + timeout;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

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
	{
		timeout = (sk_quad_fire_time->value * 10);
	}

	if (ent->client->quadfire_framenum > level.framenum)
		ent->client->quadfire_framenum += timeout;
	else
		ent->client->quadfire_framenum = level.framenum + timeout;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/quadfire1.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Breather (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->breather_framenum > level.framenum)
		ent->client->breather_framenum += (sk_breather_time->value * 10);
	else
		ent->client->breather_framenum = level.framenum + (sk_breather_time->value * 10);

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Envirosuit (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->enviro_framenum > level.framenum)
		ent->client->enviro_framenum += (sk_enviro_time->value * 10);
	else
		ent->client->enviro_framenum = level.framenum + (sk_enviro_time->value * 10);

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Invulnerability (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum += (sk_inv_time->value * 10);
	else
		ent->client->invincible_framenum = level.framenum + (sk_inv_time->value * 10);

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void	Use_Silencer (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	ent->client->silencer_shots += sk_silencer_shots->value;

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

	if (!ent->client)
		return false;

	if (item->tag == AMMO_BULLETS)
		max = ent->client->pers.max_bullets;
	else if (item->tag == AMMO_SHELLS)
		max = ent->client->pers.max_shells;
	else if (item->tag == AMMO_ROCKETS)
		max = ent->client->pers.max_rockets;
	else if (item->tag == AMMO_GRENADES)
		max = ent->client->pers.max_grenades;
	else if (item->tag == AMMO_CELLS)
		max = ent->client->pers.max_cells;
	else if (item->tag == AMMO_SLUGS)
		max = ent->client->pers.max_slugs;
	else if (item->tag == AMMO_MAGSLUG)
		max = ent->client->pers.max_magslug;
	else if (item->tag == AMMO_TRAP)
		max = ent->client->pers.max_trap;
	else if (item->tag == AMMO_FLECHETTES)
		max = ent->client->pers.max_flechettes;
	else if (item->tag == AMMO_PROX)
		max = ent->client->pers.max_prox;
	else if (item->tag == AMMO_TESLA)
		max = ent->client->pers.max_tesla;
	else if (item->tag == AMMO_DISRUPTOR)
		max = ent->client->pers.max_disruptors;
	else if (item->tag == AMMO_SHOCKSPHERE)
		max = ent->client->pers.max_shockspheres;
	else if (item->tag == AMMO_FUEL)
		max = ent->client->pers.max_fuel;
	else if (item->tag == AMMO_HOMING_ROCKETS)
		max = ent->client->pers.max_homing_rockets;
	else if (item->tag == AMMO_LASERTRIPBOMB)
		max = ent->client->pers.max_tbombs;
	else if (item->tag == AMMO_FLARES)
		max = ent->client->pers.max_flares;
	else if (item->tag == AMMO_EMPNUKE)
		max = ent->client->pers.max_empnuke;
	else if (item->tag == AMMO_PLASMASHIELD)
		max = ent->client->pers.max_plasmashield;

// ROGUE
	else
	{
		gi.dprintf("undefined ammo type\n");
		return false;
	}

	index = ITEM_INDEX(item);

	if (ent->client->pers.inventory[index] >= max)
		return false;

	ent->client->pers.inventory[index] += count;

	if (ent->client->pers.inventory[index] > max)
		ent->client->pers.inventory[index] = max;

	return true;
}

// Knightmare- this function overrides ammo pickup values with cvars
void SetAmmoPickupValues (void)
{
	gitem_t		*item;

	item = FindItem("Shells");
	if (item)
		item->quantity = sk_box_shells->value;

	item = FindItem("Bullets");
	if (item)
		item->quantity = sk_box_bullets->value;

	item = FindItem("Grenades");
	if (item)
		item->quantity = sk_box_grenades->value;

	item = FindItem("Rockets");
	if (item)
		item->quantity = sk_box_rockets->value;

	item = FindItem("Homing Rockets");
	if (item)
		item->quantity = sk_box_rockets->value;

	item = FindItem("Cells");
	if (item)
		item->quantity = sk_box_cells->value;

	item = FindItem("Slugs");
	if (item)
		item->quantity = sk_box_slugs->value;

	item = FindItem("Magslug");
	if (item)
		item->quantity = sk_box_magslugs->value;

	item = FindItem("Trap");
	if (item)
		item->quantity = sk_box_trap->value;

	item = FindItem("Flechettes");
	if (item)
		item->quantity = sk_box_flechettes->value;

	item = FindItem("Prox");
	if (item)
		item->quantity = sk_box_prox->value;

	item = FindItem("Tesla");
	if (item)
		item->quantity = sk_box_tesla->value;

	item = FindItem("Disruptors");
	if (item)
		item->quantity = sk_box_disruptors->value;

	item = FindItem("Shocksphere");
	if (item)
		item->quantity = sk_box_shocksphere->value;

	item = FindItem("Fuel");
	if (item)
		item->quantity = sk_box_fuel->value;

	item = FindItem("Flares");
	if (item)
		item->quantity = sk_box_flares->value;

	item = FindItem("IRED");
	if (item)
		item->quantity = sk_box_tbombs->value;

	item = FindItem("EMPNuke");
	if (item)
		item->quantity = sk_box_empnuke->value;

	item = FindItem("Plasma Shield");
	if (item)
		item->quantity = sk_box_plasmashield->value;
}

qboolean Pickup_Ammo (edict_t *ent, edict_t *other)
{
	int			oldcount;
	int			count;
	qboolean	weapon;
		
	// Knightmare- override ammo pickup values with cvars
	SetAmmoPickupValues ();

	weapon = (ent->item->flags & IT_WEAPON);
	if ( (weapon) && ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		count = 1000;
	else if (ent->count)
		count = ent->count;
	else
		count = ent->item->quantity;

	oldcount = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (!Add_Ammo (other, ent->item, count))
		return false;

	if (weapon && !oldcount)
	{
		// don't switch to tesla
		if (other->client->pers.weapon != ent->item
				&& ( !deathmatch->value || other->client->pers.weapon == FindItem("blaster"))
				&& (strcmp(ent->classname, "ammo_tesla")) )

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

	index = ITEM_INDEX(item);
	dropped = Drop_Item (ent, item);
	if (ent->client->pers.inventory[index] >= item->quantity)
		dropped->count = item->quantity;
	else
		dropped->count = ent->client->pers.inventory[index];

	if (ent->client->pers.weapon && 
		ent->client->pers.weapon->tag == AMMO_GRENADES &&
		item->tag == AMMO_GRENADES &&
		ent->client->pers.inventory[index] - dropped->count <= 0) {
		gi.cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		G_FreeEdict(dropped);
		return;
	}

	ent->client->pers.inventory[index] -= dropped->count;
	ValidateSelectedItem (ent);
}


//======================================================================

void MegaHealth_think (edict_t *self)
{
	if (self->owner->health > self->owner->max_health) //&& self->owner->health > self->owner->base_health)
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
			return false;

	// Knightmare- cap foodcube
	if ((ent->style & HEALTH_FOODCUBE) && other->health >= other->client->pers.max_fc_health)
		return false;

	// backup current health upon getting megahealth
/*	if (ent->style & HEALTH_TIMED)
	{
		if (other->base_health && other->health > other->base_health)
		{
			gi.dprintf("Player already has megahealth active, not saving current health\n");
		}
		else if (other->health >= other->max_health)
		{
			other->base_health = other->health;
			gi.dprintf("Current health saved as base_health\n");
		}
		else
		{
			other->base_health = other->max_health;
			gi.dprintf("Current max_health saved as base_health\n");
		}
	}*/

	other->health += ent->count;

	// stimpacks shouldn't rot away
/*	if (ent->style & HEALTH_IGNORE_MAX
		&& other->base_health >= other->max_health && other->health >= other->base_health) 
		other->base_health += ent->count;
*/
	// PMM - health sound fix
	/*
	if (ent->count == 2)
		ent->item->pickup_sound = "items/s_health.wav";
	else if (ent->count == 10)
		ent->item->pickup_sound = "items/n_health.wav";
	else if (ent->count == 25)
		ent->item->pickup_sound = "items/l_health.wav";
	else // (ent->count == 100)
		ent->item->pickup_sound = "items/m_health.wav";
	*/

	// Knightmare- cap foodcube
	if (ent->style & HEALTH_FOODCUBE)
	{
		if (other->health > other->client->pers.max_fc_health)
			other->health = other->client->pers.max_fc_health;
	}
	else if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health > other->max_health)
			other->health = other->max_health;
	}

	if (ent->style & HEALTH_TIMED)
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

qboolean Pickup_Armor (edict_t *ent, edict_t *other)
{
	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int				newcount;
	float			salvage;
	int				salvagecount;
	int				armor_maximum;

	// set armor cap
	if (ent->item->tag == ARMOR_JACKET)
		armor_maximum = sk_max_armor_jacket->value;
	else if (ent->item->tag == ARMOR_COMBAT)
		armor_maximum = sk_max_armor_combat->value;
	else	// ARMOR_BODY
		armor_maximum = sk_max_armor_body->value;

	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;

	old_armor_index = ArmorIndex (other);

	// handle armor shards specially
	if (ent->item->tag == ARMOR_SHARD)
	{
		if (!old_armor_index)
			other->client->pers.inventory[jacket_armor_index] = sk_armor_bonus_value->value;
		else
			other->client->pers.inventory[old_armor_index] += sk_armor_bonus_value->value;
	}

	// if player has no armor, just use it
	else if (!old_armor_index)
	{
		other->client->pers.inventory[ITEM_INDEX(ent->item)] = newinfo->base_count;
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
			salvagecount = salvage * other->client->pers.inventory[old_armor_index];
			newcount = newinfo->base_count + salvagecount;
			if (newcount > armor_maximum)
				newcount = armor_maximum;

			// zero count of old armor so it goes away
			other->client->pers.inventory[old_armor_index] = 0;

			// change armor to new item with computed value
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = newcount;
		}
		else
		{
			// calc new armor values
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = salvage * newinfo->base_count;
			newcount = other->client->pers.inventory[old_armor_index] + salvagecount;
			if (newcount > armor_maximum)
				newcount = armor_maximum;

			// if we're already maxed out then we don't need the new armor
			if (other->client->pers.inventory[old_armor_index] >= newcount)
				return false;

			// update current armor value
			other->client->pers.inventory[old_armor_index] = newcount;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 20);

	return true;
}

//======================================================================

// Knightmare- rewrote this so it's handled properly
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

// Knightmare- rewrote this to differentiate between power shield and power screen
void Use_PowerArmor (edict_t *ent, gitem_t *item)
{
	int		index;

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
		//if they have an active power shield, deactivate it
		else if (ent->flags & FL_POWER_SHIELD)
		{
			ent->flags &= ~FL_POWER_SHIELD;
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
		}
		else //activate power shield
		{
			index = ITEM_INDEX(FindItem("cells"));
			if (!ent->client->pers.inventory[index])
			{
				gi.cprintf (ent, PRINT_HIGH, "No cells for power shield.\n");
				return;
			}
			ent->flags |= FL_POWER_SHIELD;
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
		}
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
		if ( !quantity
			//Knightmare- don't switch to power screen if we already have a power shield active
			&& !(ent->item == FindItemByClassname("item_power_screen") && (other->flags & FL_POWER_SHIELD)))
			ent->item->use (other, ent->item);
	}
	return true;
}

// Knightmare- rewrote this so it's handled properly
void Drop_PowerArmor (edict_t *ent, gitem_t *item)
{
	if (item == FindItemByClassname("item_power_shield"))
	{
		if ((ent->flags & FL_POWER_SHIELD) && (ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
			Use_PowerArmor (ent, item);
	}
	else
		if ((ent->flags & FL_POWER_SCREEN) && (ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
			Use_PowerArmor (ent, item);
	Drop_General (ent, item);
}

//======================================================================

// Zaero
qboolean Pickup_PlasmaShield(edict_t *ent, edict_t *other)
{
	if (other->client->pers.inventory[ITEM_INDEX(ent->item)] > ent->client->pers.max_plasmashield)
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;// = 1;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
	}

	return true;
}


qboolean Pickup_Visor (edict_t *ent, edict_t *other)
{
	int frames_per_visor = (max(sk_visor_time->value, 2) * 10); // was 300

	// do we already have a visor?
	if (other->client->pers.inventory[ITEM_INDEX(ent->item)] >= sk_powerup_max->value)
	//	&& other->client->pers.visorFrames == frames_per_visor)
	{
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (ent->spawnflags & DROPPED_ITEM)
		other->client->pers.visorFrames = (int)(other->client->pers.visorFrames + ent->visorFrames)
																				% frames_per_visor;
	else
		other->client->pers.visorFrames = (int)(other->client->pers.visorFrames + frames_per_visor)
																				% frames_per_visor;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, 30);
	}

	return true;
}


void Drop_Visor (edict_t *ent, gitem_t *item)
{
	int frames_per_visor = (max(sk_visor_time->value, 2) * 10);

	edict_t *visor = Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	visor->visorFrames = (ent->client->pers.visorFrames > 0) ? ent->client->pers.visorFrames : frames_per_visor;
	ent->client->pers.visorFrames = 0;
//	if (ent->client->pers.inventory[ITEM_INDEX(item)] == 0)
//		ent->client->pers.visorFrames = 0;
}
// end Zaero

//======================================================================

/*
===============
Touch_Item
===============
*/
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean	taken;

	if (!other->client)
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

		// PMM - health sound fix
		if (ent->item->pickup == Pickup_Health)
		{
			if (ent->style & HEALTH_FOODCUBE)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 1, ATTN_NORM, 0);
			else if (ent->style == (HEALTH_IGNORE_MAX|HEALTH_TIMED))	// if (ent->count == 100)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 1, ATTN_NORM, 0);
			else if	(ent->style & HEALTH_SMALL)		// if (ent->count < 10) // Knightmare
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 1, ATTN_NORM, 0);
			else  if (ent->style & HEALTH_LARGE)	// if (ent->count == 25)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);
			else									// if (ent->count == 10)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/n_health.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->item->pickup_sound) // PGM - paranoia
		{
			gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
		}
	}

	if (!(ent->spawnflags & ITEM_TARGETS_USED))
	{
		G_UseTargets (ent, other);
		ent->spawnflags |= ITEM_TARGETS_USED;
	}

	if (!taken)
		return;

	// Lazarus reflections
	DeleteReflection (ent, -1);

	if (!((coop->value) &&  (ent->item->flags & IT_STAY_COOP)) || (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
	{
		if (ent->flags & FL_RESPAWN)
			ent->flags &= ~FL_RESPAWN;
		else
			G_FreeEdict (ent);
	}
}

// Knightmare added
/*
===============
Touch_Q1Backpack
===============
*/
void Touch_Q1Backpack (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean	taken;

//	if (other == ent->owner)	// can't be touched by the entity that dropped us
//		return;

	if (!other->client)
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

	//	gi.sound(other, CHAN_ITEM, gi.soundindex("q1weapons/lock4.wav"), 1, ATTN_NORM, 0);
	/*	if (ent->noise_index != 0) {
			gi.sound(other, CHAN_ITEM, ent->noise_index, 1, ATTN_NORM, 0);
		}*/
	}

	if (!taken)
		return;

	// Lazarus reflections
	DeleteReflection (ent, -1);

	// just delete it, no respawning
	G_FreeEdict (ent);
}
// end Knightmare

//======================================================================

/*static*/ void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	Touch_Item (ent, other, plane, surf);
}

/*static*/ void drop_make_touchable (edict_t *ent)
{
	ent->touch = Touch_Item;
	if (deathmatch->value)
	{
		ent->nextthink = level.time + 29;
		ent->think = G_FreeEdict;
	}
}

void Item_Die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	BecomeExplosion1 (self);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectExplosion (TE_EXPLOSION1, self->s.origin);

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (self, 30);
	else
		G_FreeEdict (self);
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
	dropped->s.skinnum = item->world_model_skinnum; // Knightmare- skinnum specified in item table
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW | RF_IR_VISIBLE;		// PGM
	dropped->s.angles[1] = ent->s.angles[1];	// Knightmare- preserve yaw from dropping entity
	// mxd- Actually randomize it a bit (and not in 0 .. 32767 range like rand() does)
	/*if (rand() > 0.5)							// randomize it a bit
		dropped->s.angles[1] += rand()*45;
	else
		dropped->s.angles[1] -= rand()*45;*/
	dropped->s.angles[1] += crandom() * 45;

	VectorSet (dropped->mins, -16, -16, -16);
	VectorSet (dropped->maxs, 16, 16, 16);
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

/*	if (ent->solid == SOLID_BSP) //Knightmare- hack for items dropped by shootable boxes
	{
		VectorCopy (ent->velocity, dropped->velocity); //if it's moving on a conveyor
	}
	else*/
	if (ent->solid != SOLID_BSP) // Knightmare- hack for items dropped by shootable boxes
		VectorScale (forward, 100, dropped->velocity);

	if (!strcmp(ent->classname, "func_pushable")) // Make items dropped by movable crates destroyable
	{
		dropped->spawnflags |= ITEM_SHOOTABLE;
		dropped->solid = SOLID_BBOX;
		dropped->health = 20;
		dropped->takedamage = DAMAGE_YES;
	//	dropped->die = BecomeExplosion1;
		// Knightmare- this compiles cleaner
		dropped->die = Item_Die;

	}

	dropped->velocity[2] += 300;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	return dropped;
}

// Knightmare added
/*
============
Drop_Q1Backpack

Spawns backpack dropped by Q1 monsters.
Similar to Drop_Item, but the backpack is not an item in the item table.
It contains an item instead.
============
*/
edict_t *Drop_Q1Backpack (edict_t *ent, gitem_t *item, int count)
{
	edict_t	*backpack;
	vec3_t	forward, right;
	vec3_t	offset;

	if (!ent)
		return NULL;
	if (!item) {
		gi.dprintf ("Drop_Q1Backpack: invalid item\n");
		return NULL;
	}

	backpack = G_Spawn();

	backpack->classname = "misc_q1_backpack";
	backpack->item = item;
	backpack->count = count;
	backpack->touch = Touch_Q1Backpack;
	backpack->spawnflags = DROPPED_ITEM;
	backpack->s.skinnum = 0;
	backpack->s.effects = EF_ROTATE;
	backpack->s.renderfx = RF_GLOW | RF_IR_VISIBLE;
	backpack->s.angles[1] = ent->s.angles[1];	// Knightmare- preserve yaw from dropping entity
	backpack->s.angles[1] += crandom() * 45;
//	backpack->noise_index = gi.soundindex("q1weapons/lock4.wav");

	VectorSet (backpack->mins, -16, -16, -16);
	VectorSet (backpack->maxs, 16, 16, 16);
	gi.setmodel (backpack, "models/items/q1backpack/tris.md2");
	backpack->solid = SOLID_TRIGGER;
	backpack->movetype = MOVETYPE_TOSS;  
	backpack->touch = drop_temp_touch;
	backpack->owner = ent;

	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, backpack->s.origin);
		trace = gi.trace (ent->s.origin, backpack->mins, backpack->maxs,
			backpack->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, backpack->s.origin);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, backpack->s.origin);
	}

	VectorScale (forward, 100, backpack->velocity);

	backpack->velocity[2] += 300;

	backpack->think = G_FreeEdict;
	backpack->nextthink = level.time + 1800;

	gi.linkentity (backpack);

	return backpack;
}
// end Knightmare

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
	else if (ent->item->world_model)	// PGM we shouldn't need this check, but paranoia...
		gi.setmodel (ent, ent->item->world_model);

	ent->solid = SOLID_TRIGGER;
	if (ent->spawnflags & ITEM_NO_DROPTOFLOOR)
		ent->movetype = MOVETYPE_NONE;
	else
		ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	if (!(ent->spawnflags & ITEM_NO_DROPTOFLOOR))	// Knightmare- allow marked items to spawn in solids
	{
		v = tv(0,0,-128);
		VectorAdd (ent->s.origin, v, dest);

		tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
		if (tr.startsolid)
		{
			gi.dprintf ("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
			G_FreeEdict (ent);
			return;
		}

		VectorCopy (tr.endpos, ent->s.origin);
	}

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

	if (ent->spawnflags & ITEM_NO_ROTATE)
		ent->s.effects &= ~EF_ROTATE;

	if (ent->spawnflags & ITEM_SHOOTABLE)
	{
		ent->solid = SOLID_BBOX;
		if (!ent->health)
			ent->health = 20;
		ent->takedamage = DAMAGE_YES;
		//ent->die = BecomeExplosion1;
		//Knightmare- this compiles cleaner
		ent->die = Item_Die;

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


//=================
// Item_TriggeredSpawn - create the item marked for spawn creation
//=================
void Item_TriggeredSpawn (edict_t *self, edict_t *other, edict_t *activator)
{
//	self->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
//	self->think = droptofloor;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	if (strcmp(self->classname, "key_power_cube"))	// leave them be on key_power_cube..
		self->spawnflags = 0;
	droptofloor (self);
}

//=================
// SetTriggeredSpawn - set up an item to spawn in later.
//=================
void SetTriggeredSpawn (edict_t *ent)
{
	// don't do anything on key_power_cubes.
	if (!strcmp(ent->classname, "key_power_cube"))
		return;

	ent->think = NULL;
	ent->nextthink = 0;
	ent->use = Item_TriggeredSpawn;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
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
// PGM - since the item may be freed by the following rules, go ahead
//		 and move the precache until AFTER the following rules have been checked.
//		 keep an eye on this.
//	PrecacheItem (item);

/*	if (ent->spawnflags > 1)		// PGM
	{
		if (strcmp(ent->classname, "key_power_cube") != 0)
		{
			ent->spawnflags = 0;
			gi.dprintf("%s at %s has invalid spawnflags set\n", ent->classname, vtos(ent->s.origin));
		}
	}*/

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
		if ( (int)dmflags->value & DF_NO_MINES )
		{
			if ( !strcmp(ent->classname, "ammo_prox") || 
				 !strcmp(ent->classname, "ammo_tesla") )
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_NO_NUKES )
		{
			if ( !strcmp(ent->classname, "ammo_nuke") )
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_NO_SPHERES )
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
//==========
//ROGUE
// DM only items
	if (!deathmatch->value)
	{
		if (item->pickup == Pickup_Doppleganger || item->pickup == Pickup_Nuke)
		{
			G_FreeEdict (ent);
			return;
		}
		if ((item->use == Use_Vengeance) || (item->use == Use_Hunter))
		{
			G_FreeEdict (ent);
			return;
		}
	}
//ROGUE
//==========

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

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	if (ent->skinnum)								// Knightmare- use mapper-specified skinnum
		ent->s.skinnum = ent->skinnum;
	else
		ent->s.skinnum = item->world_model_skinnum;	// Knightmare- skinnum specified in item table
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	if (ent->model)
		gi.modelindex (ent->model);

	// Knightmare added- all items are IR visible
	ent->s.renderfx |= RF_IR_VISIBLE;

	if (ent->spawnflags & 1)
		SetTriggeredSpawn (ent);
}

//======================================================================

gitem_t	itemlist[] = 
{
	{NULL},	// leave index 0 alone

//
// ARMOR
//

// 1
/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_armor_body", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/body/tris.md2", 0, EF_ROTATE,
		NULL,
		"i_bodyarmor", // icon
		"Body Armor", // pickup
		3, // width
		0,
		NULL,
		IT_ARMOR,
		0,
		&bodyarmor_info,
		ARMOR_BODY,
		"" // precache
	},

// 2
/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_armor_combat", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/combat/tris.md2", 0, EF_ROTATE,
		NULL,
		"i_combatarmor", // icon
		"Combat Armor", // pickup
		3, // width
		0,
		NULL,
		IT_ARMOR,
		0,
		&combatarmor_info,
		ARMOR_COMBAT,
		"" // precache
	},

// 3
/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_armor_jacket", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/jacket/tris.md2", 0, EF_ROTATE,
		NULL,
		"i_jacketarmor", // icon
		"Jacket Armor", // pickup
		3, // width
		0,
		NULL,
		IT_ARMOR,
		0,
		&jacketarmor_info,
		ARMOR_JACKET,
		"" // precache
	},

// 4
/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_armor_shard", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar2_pkup.wav",
		"models/items/armor/shard/tris.md2", 0, EF_ROTATE,
		NULL,
#ifdef KMQUAKE2_ENGINE_MOD
		"i_shard", // icon
#else
		"i_jacketarmor", // icon
#endif
		"Armor Shard", // pickup
		3, // width
		0,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		ARMOR_SHARD,
		"" // precache
	},

//Knightmare- armor shard that lies flat on the ground
// 5
/*QUAKED item_armor_shard_flat (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_armor_shard_flat", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar2_pkup.wav",
		"models/items/armor/shard/flat/tris.md2", 0, 0,
		NULL,
#ifdef KMQUAKE2_ENGINE_MOD
		"i_shard", // icon
#else
		"i_jacketarmor", // icon
#endif
		"Armor Shard", // pickup
		3, // width
		0,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		ARMOR_SHARD,
		"" // precache
	},

// 6
/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_power_screen", 
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/screen/tris.md2", 0, EF_ROTATE,
		NULL,
		"i_powerscreen", // icon
		"Power Screen", // pickup
		0, // width
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
		"misc/power2.wav misc/power1.wav" // precache
	},

// 7
/*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_power_shield",
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/shield/tris.md2", 0, EF_ROTATE,
		NULL,
		"i_powershield", // icon
		"Power Shield", // pickup
		0, // width
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
		"misc/power2.wav misc/power1.wav" // precache
	},

	//
	// WEAPONS 
	//
// 8
/*QUAKED weapon_flaregun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_flaregun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_FlareGun,
		"misc/w_pkup.wav",
		"models/weapons/g_flare/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_flare/tris.md2",
		"w_flare", // icon
		"Flare Gun", // pickup
		0,
		1,
		"Flares",
		IT_WEAPON|IT_STAY_COOP|IT_ZAERO,
		WEAP_BLASTER,
		NULL,
		0,
		"models/objects/flare/tris.md2 weapons/flare/shoot.wav weapons/flare/flarehis.wav" // precache
	},

// 9
/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_blaster", 
		Pickup_Weapon, //was NULL
		Use_Weapon,
		Drop_Weapon, //was NULL
		Weapon_Blaster,
		"misc/w_pkup.wav",
		"models/weapons/g_blast/tris.md2", 0, EF_ROTATE, //was NULL, 0
		"models/weapons/v_blast/tris.md2",
		"w_blaster", // icon
		"Blaster", // pickup
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
		"weapons/blastf1a.wav misc/lasfly.wav" // precache
	},

// 10
/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_shotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Shotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
		"w_shotgun",	// icon
		"Shotgun",	// pickup
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SHOTGUN,
		NULL,
		0,
		"weapons/shotgf1b.wav weapons/shotgr1b.wav" // precache
	},

// 11
/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_supershotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_SuperShotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
		"w_sshotgun", // icon
		"Super Shotgun", // pickup
		0,
		2,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
		"weapons/sshotf1b.wav" // precache
	},

// 12
/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_machinegun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Machinegun,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_machn/tris.md2",
		"w_machinegun", // icon
		"Machinegun", // pickup
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
		"weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav" // precache
	},

// 13
/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_chaingun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Chaingun,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
		"w_chaingun", // icon
		"Chaingun", // pickup
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CHAINGUN,
		NULL,
		0,
		"weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav weapons/chngnd1a.wav" // precache
	},

// 14
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
		"models/weapons/g_etf_rifle/tris.md2", 0, EF_ROTATE,// world model, skinnum, world model flags
		"models/weapons/v_etf_rifle/tris.md2",				// view model
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
	// rogue

// 15
/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_grenades",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0, 0,
		"models/weapons/v_handgr/tris.md2",
		"a_grenades", // icon
		"Grenades", // pickup
		3, // width
		5,
		"grenades",
		IT_AMMO|IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		AMMO_GRENADES,
		"models/objects/grenade2/tris.md2 weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav" // precache
	},

// 16
/*QUAKED ammo_ired (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_ired",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_LaserTripBomb,
		"misc/am_pkup.wav",
		"models/items/ammo/ireds/tris.md2", 0, 0,
		"models/weapons/v_ired/tris.md2",
		"w_ired", // icon
		"IRED", // pickup
		3, // width
		3,
		"IRED",
		IT_AMMO|IT_WEAPON|IT_ZAERO,
		WEAP_GRENADES,
		NULL,
		AMMO_LASERTRIPBOMB,
		"models/weapons/v_ired/hand.md2 models/objects/ired/tris.md2 modes/objects models/objects/shrapnel/tris.md2 weapons/ired/las_set.wav weapons/ired/las_arm.wav " // precache
	/*"weapons/ired/las_tink.wav "weapons/ired/las_trig.wav "*/
	/*"weapons/ired/las_glow.wav"*/,
	},

// 17
/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_GrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
		"w_glauncher", // icon
		"Grenade Launcher", // pickup
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
		"models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav" // precache
	},

// 18
// ROGUE
/*QUAKED weapon_proxlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_proxlauncher",								// classname
		Pickup_Weapon,										// pickup
		Use_Weapon,											// use
		Drop_Weapon,										// drop
		Weapon_ProxLauncher,								// weapon think
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_plaunch/tris.md2", 0, EF_ROTATE,	// world model, skinnum, world model flags
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
	// rogue

// 19
/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
		"w_rlauncher", // icon
		"Rocket Launcher", // pickup
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
		"models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2" // precache
	},

// 20
/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_hyperblaster", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_HyperBlaster,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
		"w_hyperblaster", // icon
		"HyperBlaster", // pickup
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
		"weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav" // removed weapons/hyprbu1a.wav
	},

// 21
// ROGUE
/*QUAKED weapon_plasmabeam (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/ 
	{
		"weapon_plasmabeam",								// classname
		Pickup_Weapon,										// pickup function
		Use_Weapon,											// use function
		Drop_Weapon,										// drop function
		Weapon_Heatbeam,									// weapon think function
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_beamer/tris.md2", 0, EF_ROTATE,	// world model, skinnum, world model flags
		"models/weapons/v_beamer/tris.md2",					// view model
		"w_heatbeam",											// icon
		"Plasma Beam",											// name printed when picked up 
		0,													// number of digits for statusbar
		2,													// amount used / contained- if this changes, change it in NoAmmoWeaponChange as well
		"Cells",											// ammo type used 
		IT_WEAPON|IT_STAY_COOP|IT_ROGUE,					// inventory flags
		WEAP_PLASMA,										// visible weapon
		NULL,												// info (void *)
		0,													// tag
		"models/weapons/v_beamer2/tris.md2 weapons/bfg__l1a.wav", // precache
	},
// ROGUE

// 22
/*QUAKED weapon_boomer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/

	{
		"weapon_boomer",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Ionripper,
		"misc/w_pkup.wav",
		"models/weapons/g_boom/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_boomer/tris.md2",
		"w_ripper", // icon
		"ION Ripper", /// pickup
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

// 23
/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_railgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Railgun,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
		"w_railgun", // icon
		"Railgun", // pickup
		0,
		1,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
		"weapons/rg_hum.wav weapons/railgf1a.wav" // precache
	},

// 24
/*QUAKED weapon_phalanx (.3 .3 1) (-16 -16 -16) (16 16 16)
*/

	{
		"weapon_phalanx",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Phalanx,
		"misc/w_pkup.wav",
		"models/weapons/g_shotx/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_shotx/tris.md2",
		"w_phallanx", // icon
		"Phalanx", // pickup
		0,
		1,
		"Magslug",
		IT_WEAPON|IT_STAY_COOP|IT_XATRIX,
		WEAP_PHALANX,
		NULL,
		0,
#ifdef KMQUAKE2_ENGINE_MOD
		"sprites/s_photon.sp2 eapons/plasshot.wav weapons/phaloop.wav" // precache
#else
		"sprites/s_photon.sp2 weapons/plasshot.wav" // precache
#endif
	},

// 25
/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BFG,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
		"w_bfg", // icon
		"BFG10K", // pickup
		0,
		50,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
#ifdef KMQUAKE2_ENGINE_MOD
		"sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav" // precache
#else
		"sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav" // precache
#endif
	},

// 26
/*QUAKED weapon_disintegrator (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_disintegrator",							// classname
		Pickup_Weapon,									// pickup function
		Use_Weapon,										// use function
		Drop_Weapon,									// drop function
		Weapon_Disintegrator,							// weapon think function
		"misc/w_pkup.wav",								// pick up sound
		"models/weapons/g_dist/tris.md2", 0, EF_ROTATE,	// world model, skinnum, world model flags
		"models/weapons/v_dist/tris.md2",				// view model
		"w_disintegrator",								// icon
		"Disintegrator",								// name printed when picked up 
		0,												// number of digits for statusbar
		1,												// amount used / contained
		"Disruptors",									// ammo type used 
		IT_WEAPON|IT_STAY_COOP|IT_ROGUE,				// inventory flags
		WEAP_DISRUPTOR,									// visible weapon
		NULL,											// info (void *)
		1,												// tag
		"models/items/spawngro/tris.md2 models/proj/disintegrator/tris.md2 weapons/disrupt.wav weapons/disint2.wav weapons/disrupthit.wav",	// precaches
	},

// 27
/*QUAKED weapon_chainfist (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_chainfist",									// classname
		Pickup_Weapon,										// pickup function
		Use_Weapon,											// use function
		Drop_Weapon,										// drop function
		Weapon_ChainFist,									// weapon think function
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_chainf/tris.md2", 0, EF_ROTATE,	// world model, world model flags
		"models/weapons/v_chainf/tris.md2",					// view model
		"w_chainfist",										// icon
		"Chainfist",										// name printed when picked up 
		0,													// number of digits for statusbar
		0,													// amount used / contained
		NULL,												// ammo type used 
		IT_WEAPON|IT_STAY_COOP|IT_MELEE|IT_ROGUE,			// inventory flags
		WEAP_CHAINFIST,										// visible weapon
		NULL,												// info (void *)
		1,													// tag
		"weapons/sawidle.wav weapons/sawhit.wav",			// precaches
	},

// 28
/*QUAKED weapon_shockwave (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_shockwave", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Shockwave,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
		"w_chaingun", // icon 
		"Shockwave", // pickup
		0,
		1,
		"Shocksphere",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SHOCKWAVE,				// visible weapon
		NULL,
		0,
		"weapons/shockactive.wav weapons/shock_hum.wav weapons/shockfire.wav weapons/shockaway.wav weapons/shockhit.wav weapons/shockexp.wav models/objects/shocksphere/tris.md2 models/objects/shockfield/tris.md2 sprites/s_trap.sp2"
	},

// 29	SKWiD MOD
/*QUAKED weapon_plasma (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_plasma",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Plasma_Rifle,
		"misc/w_pkup.wav",
		PLASMA_MODEL_WORLD, 0, EF_ROTATE,
		PLASMA_MODEL_VIEW,
		"w_plasma",		// icon
		PLASMA_PICKUP_NAME,	// pickup
		0,
		PLASMA_CELLS_PER_SHOT,	// bat - was 1
		"Cells",
		IT_WEAPON|IT_STAY_COOP|IT_LM,
		WEAP_PLASMARIFLE,
		NULL,
		0,
		"PLASMA_PRECACHE"
	},

// 30
/*QUAKED weapon_hml (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hml",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_HomingMissileLauncher,
		NULL,
		NULL, 0, EF_ROTATE,
		"models/weapons/v_homing/tris.md2",
		NULL, // icon
		"Homing Rocket Launcher", // pickup
		0,
		1,
		"Homing Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
		"models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

// 31
// Lazarus: No weapon - we HAVE to have a weapon
	{
		"weapon_null",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Null,
		"misc/w_pkup.wav",
		NULL, 0, 0,
		NULL,
		NULL,
		"No Weapon",
	  	0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_NONE,
		NULL,
		0,
		""
	},
	//
	// AMMO ITEMS
	//

// 32
/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_shells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/shells/medium/tris.md2", 0, 0,
		NULL,
		"a_shells", // icon
		"Shells", // pickup
		3, // width
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SHELLS,
		"" // precache
	},

// 33
/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_bullets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/bullets/medium/tris.md2", 0, 0,
		NULL,
		"a_bullets", // icon
		"Bullets", // pickup
		3, // width
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BULLETS,
		"" // precache
	},

// 34
/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_cells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/cells/medium/tris.md2", 0, 0,
		NULL,
		"a_cells", // icon
		"Cells", // pickup
		3, // width
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_CELLS,
		"" // precache
	},

// 35
/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_rockets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rockets/medium/tris.md2", 0, 0,
		NULL,
		"a_rockets", // icon
		"Rockets", // pickup
		3, // width
		5,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_ROCKETS,
		"" // precache
	},

// 36
/*QUAKED ammo_homing_missiles (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_homing_missiles",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/homing/medium/tris.md2", 0, 0,
		NULL,
		"a_homing", // icon
		"Homing Rockets", // pickup
		3, // width
		5,
		NULL,
		IT_AMMO|IT_LAZARUS,
		0,
		NULL,
		AMMO_HOMING_ROCKETS,
		"" // precache
	},

// 37
/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_slugs",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/slugs/medium/tris.md2", 0, 0,
		NULL,
		"a_slugs", // icon
		"Slugs", // pickup
		3, // width
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SLUGS,
		"" // precache
	},

// 38
/*QUAKED ammo_magslug (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_magslug",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/objects/ammo/tris.md2", 0, 0,
		NULL,
		"a_mslugs", // icon
		"Magslug", // pickup
		3, // width
		10,
		NULL,
		IT_AMMO|IT_XATRIX,
		0,
		NULL,
		AMMO_MAGSLUG,
/* precache */ "" // precache
	},

// 39
/*QUAKED ammo_flechettes (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_flechettes",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/ammo/am_flechette/tris.md2", 0, 0,
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
	},

// 40
/*QUAKED ammo_disruptor (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_disruptor",						// Classname
		Pickup_Ammo,							// pickup function
		NULL,									// use function
		Drop_Ammo,								// drop function
		NULL,									// weapon think function
		"misc/am_pkup.wav",						// pickup sound
		"models/ammo/am_disr/tris.md2", 0, 0,	// world model, world model flags
		NULL,									// view model
		"a_disruptor",							// icon
		"Disruptors",							// pickup 
		3,  									// number of digits for status bar
		15, 									// amount contained
		NULL,									// ammo type used
		IT_AMMO|IT_ROGUE,						// inventory flags
		0,										// vwep index
		NULL,									// info (void *)
		AMMO_DISRUPTOR,							// tag
	},

// 41
/*QUAKED ammo_prox (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_prox",							// Classname
		Pickup_Ammo,							// pickup function
		NULL,									// use function
		Drop_Ammo,								// drop function
		NULL,									// weapon think function
		"misc/am_pkup.wav",						// pickup sound
		"models/ammo/am_prox/tris.md2", 0, 0,	// world model, world model flags
		NULL,									// view model
		"a_prox",								// icon
		"Prox",									// Name printed when picked up
		3,										// number of digits for status bar
		5,										// amount contained
		NULL,									// ammo type used
		IT_AMMO|IT_ROGUE,						// inventory flags
		0,										// vwep index
		NULL,									// info (void *)
		AMMO_PROX,								// tag
		"models/weapons/g_prox/tris.md2 weapons/proxwarn.wav"	// precaches
	},

// 42
/*QUAKED ammo_shocksphere (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_shocksphere",						// Classname
		Pickup_Ammo,							// pickup function
		NULL,									// use function
		Drop_Ammo,								// drop function
		NULL,									// weapon think function
		"misc/am_pkup.wav",						// pickup sound
		"models/items/tagtoken/tris.md2", 0, 0,	// world model, world model flags
		NULL,									// view model
		"i_tagtoken",							// icon
		"Shocksphere",							// Name printed when picked up
		3,										// number of digits for status bar
		1,										// amount contained
		NULL,									// ammo type used
		IT_AMMO,								// inventory flags
		0,										// vwep index
		NULL,									// info (void *)
		AMMO_SHOCKSPHERE,						// tag
		""	// precache
	},

// 43
/*QUAKED ammo_flares (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_flares",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/flares/tris.md2", 0, 0,
		NULL,
		"a_flares", // icon 
		"Flares", // pickup 
		3, // width
		3,
		NULL,
		IT_AMMO|IT_ZAERO,
		0,
		NULL,
		AMMO_FLARES,
		"" //  precache
	},

// 44
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
		"models/ammo/am_tesl/tris.md2", 0, 0,
		"models/weapons/v_tesla/tris.md2",
		"a_tesla",
		"Tesla",
		3,
		5,
		"Tesla",						// PGM
		IT_AMMO|IT_WEAPON|IT_ROGUE,		// inventory flags
		WEAP_GRENADES,
		NULL,							// info (void *)
		AMMO_TESLA,						// tag
		"models/weapons/v_tesla2/tris.md2 weapons/teslaopen.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav models/weapons/g_tesla/tris.md2"	// precache
	},

// 45
/*QUAKED ammo_trap (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_trap",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Trap,
		"misc/am_pkup.wav",
		"models/weapons/g_trap/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_trap/tris.md2",
		"a_trap", // icon
		"Trap", // pickup
		3, // width
		1,
		"trap",
		IT_AMMO|IT_WEAPON|IT_XATRIX,
		WEAP_GRENADES, //WEAP_TRAP
		NULL,
		AMMO_TRAP,
		"weapons/trapcock.wav weapons/traploop.wav weapons/trapsuck.wav weapons/trapdown.wav" // precache
	},

// 46
/*QUAKED ammo_nuke (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_nuke",
		Pickup_Nuke,
		Use_Nuke,						// PMM
		Drop_Ammo,
		NULL,							// PMM
		"misc/am_pkup.wav",
		"models/weapons/g_nuke/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_nuke",  // icon 
		"A-M Bomb",  // pickup 
		3,  // width 
		1, // quantity 
		"A-M Bomb",
		IT_POWERUP|IT_ROGUE,	
		0,
		NULL,
		0,
		"weapons/nukewarn2.wav world/rumble.wav"
	},

// 47
/*QUAKED ammo_nbomb (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"ammo_nbomb",
		Pickup_Nbomb,
		Use_Nbomb,						// PMM
		Drop_Ammo,
		NULL,							// PMM
		"misc/am_pkup.wav",
		"models/items/ammo/nbomb/tris.md2", 0, 0,
		NULL,
		"w_nbomb", // icon 
#ifdef CITADELMOD_FEATURES
		"CBU-71", // pickup 
#else
		"BLU-86", // pickup 
#endif
		3, // width 
		1, // quantity
		"BLU-86",
		IT_POWERUP,	
		0,
		NULL,
		0,
		"weapons/nukewarn2.wav world/rumble.wav" //  precache
	},

// 48
/*QUAKED ammo_empnuke (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_empnuke",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_EMPNuke,
		"misc/am_pkup.wav",
		"models/weapons/g_enuke/tris.md2", 0, EF_ROTATE,
		"models/weapons/v_enuke/tris.md2",
		"w_enuke", // icon 
		"EMPNuke", // pickup 
		3, // width
		1,
		"EMPNuke",
		IT_AMMO|IT_ZAERO,
		0,
		NULL,
		AMMO_EMPNUKE,
		"items/empnuke/emp_trg.wav"  //items/empnuke/emp_act.wav items/empnuke/emp_spin.wav items/empnuke/emp_idle.wav  //  precache
	},

// 49
/*QUAKED ammo_fuel (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/items/ammo/fuel/medium/"
*/
	{
		"ammo_fuel",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/fuel/medium/tris.md2", 0, 0,
		NULL,
		"a_fuel", // icon 
		"Fuel", // pickup 
		4, // width
		500, // quantity
		NULL,
		IT_AMMO|IT_LAZARUS,
		0,
		NULL,
		AMMO_FUEL,
		"" //  precache
	},

	//
	// POWERUP ITEMS
	//

// 50
/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_quad", 
		Pickup_Powerup,
		Use_Quad,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/quaddama/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_quad", // icon
		"Quad Damage", // pickup
		2, // width
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
		"items/damage.wav items/damage2.wav items/damage3.wav" // precache
	},

// 51
/*QUAKED item_double (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_double", 
		Pickup_Powerup,
		Use_Double,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/ddamage/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_double", // icon
		"Double Damage", // pickup
		2, // width
		60,
		NULL,
		IT_POWERUP|IT_ROGUE,
		0,
		NULL,
		0,
		"misc/ddamage1.wav misc/ddamage2.wav misc/ddamage3.wav" // precache
	},

// 52
/*QUAKED item_quadfire (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_quadfire", 
		Pickup_Powerup,
		Use_QuadFire,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/quadfire/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_quadfire", // icon
		"DualFire Damage", // pickup
		2, // width
		60,
		NULL,
		IT_POWERUP|IT_XATRIX,
		0,
		NULL,
		0,
		"items/quadfire1.wav items/quadfire2.wav items/quadfire3.wav" // precache
	},

// 53
/*QUAKED item_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_invulnerability",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/invulner/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_invulnerability", // icon
		"Invulnerability", // pickup
		2, // width
		300,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav" // precache
	},

// 54
/*QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_silencer",
		Pickup_Powerup,
		Use_Silencer,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/silencer/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_silencer", // icon
		"Silencer", // pickup
		2, // width
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
		"" // precache
	},

// 55
/*QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_breather",
		Pickup_Powerup,
		Use_Breather,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/breather/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_rebreather", // icon
		"Rebreather", // pickup
		2, // width
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		0,
		NULL,
		0,
		"player/u_breath1.wav player/u_breath2.wav items/airout.wav" // precache
	},

// 56
/*QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_enviro",
		Pickup_Powerup,
		Use_Envirosuit,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/enviro/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_envirosuit", // icon
		"Environment Suit", // pickup
		2, // width
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		0,
		NULL,
		0,
		"player/u_breath1.wav player/u_breath2.wav items/airout.wav" // precache
	},

// 57
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
		"models/items/goggles/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_ir", // icon
		"IR Goggles", // pickup
		2, // width
		60,
		NULL,
		IT_POWERUP|IT_ROGUE,
		0,
		NULL,
		0,
		"misc/ir_start.wav" // precache
	},

// 58
/*QUAKED item_visor (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_visor",
		Pickup_Visor,
		Use_Visor,
		Drop_Visor,
		NULL,
		"items/pkup.wav",
		"models/items/visor/tris.md2", 0, EF_ROTATE,
		NULL,
		"i_visor", // icon
		"Visor", // pickup
		1, // width
		30,
		"Cells",
		IT_STAY_COOP|IT_POWERUP|IT_ZAERO,
		0,
		NULL,
		0,
		"items/visor/act.wav items/visor/deact.wav" // items/visor/next.wav" // precache
	},

// 59
/*QUAKED ammo_plasmashield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_plasmashield",
		Pickup_Ammo,
		Use_PlasmaShield,
		Drop_Ammo,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/plasma/tris.md2", 0, EF_ROTATE,
		NULL,
		"i_plasma", // icon
		"Plasma Shield", // pickup
		1, // width
		5,
		"",
		IT_AMMO|IT_ZAERO,
		0,
		NULL,
		AMMO_PLASMASHIELD,
		"items/plasmashield/psactive.wav sprites/plasmashield.sp2" // precache
	//	"items/plasmashield/psfire.wav items/plasmashield/psactive.wav items/plasmashield/psdie.wav sprites/plasmashield.sp2"
	},

// 60
/*QUAKED item_ancient_head (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
Special item that gives +2 to maximum health
*/
	{
		"item_ancient_head",
		Pickup_AncientHead,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/c_head/tris.md2", 0, EF_ROTATE,
		NULL,
		"i_fixme", // icon
		"Ancient Head", // pickup
		2, // width
		60,
		NULL,
		0,
		0,
		NULL,
		0,
		"" // precache
	},

// New item (sorta) for Citadel pack by Andrea Rosa
#ifdef CITADELMOD_FEATURES
// 61
/*QUAKED item_steroid_pack (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
Special item that gives +2 to maximum health
*/
	{
		"item_steroid_pack",
		Pickup_AncientHead,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/steroid/tris.md2", 0, EF_ROTATE,
		NULL,
		"i_stero", // icon
		"Steroids", // pickup
		2, // width
		60,
		NULL,
		0,
		0,
		NULL,
		0,
		"" // precache
	},
#endif

// 61
/*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
gives +1 to maximum health
*/
	{
		"item_adrenaline",
		Pickup_Adrenaline,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/adrenal/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_adrenaline", // icon
		"Adrenaline", // pickup
		2, // width
		60,
		NULL,
		0,
		0,
		NULL,
		0,
		"" // precache
	},

// 62
/*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_bandolier",
		Pickup_Bandolier,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/band/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_bandolier", // icon
		"Bandolier", // pickup
		2, // width
		60,
		NULL,
		0,
		0,
		NULL,
		0,
		"" // precache
	},

// 63
/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_pack",
		Pickup_Pack,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/pack/tris.md2", 0, EF_ROTATE,
		NULL,
		"i_pack", // icon
		"Ammo Pack", // pickup
		2, // width
		180,
		NULL,
		0,
		0,
		NULL,
		0,
		"" // precache
	},

#ifdef JETPACK_MOD
// 64
/*QUAKED item_jetpack (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
model="models/items/jet/"
*/

	{
		"item_jetpack",
		Pickup_Powerup,
		Use_Jet,
	    Drop_Jetpack,
		NULL,
		"items/pkup.wav",
		"models/items/jet/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_jet",
		"Jetpack",
		2,
		600,
		"Fuel",
		IT_POWERUP|IT_LAZARUS,
		0,
		NULL,
		0,
		"jetpack/activate.wav jetpack/rev1.wav jetpack/revrun.wav jetpack/running.wav jetpack/shutdown.wav jetpack/stutter.wav"
  },
#endif

// ======================================
// PGM

/*QUAKED item_torch (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
/*
	{
		"item_torch", 
		Pickup_Powerup,
		Use_Torch,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/objects/fire/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_torch",
		"torch",
		2,
		60,
		NULL,
		IT_POWERUP|IT_ROGUE,
		0,
		NULL,
		0,
		"" // precache
	},*/

// 65
/*QUAKED item_flashlight (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_flashlight", 
		Pickup_Powerup,
		Use_Flashlight,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/f_light/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_flash",
		"Flashlight",
		2,
		60,
		NULL,
		IT_POWERUP|IT_LAZARUS,
		0,
		NULL,
		0,
		"" // precache
	},

// 66
/*QUAKED item_compass (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_compass", 
		Pickup_Powerup,
		Use_Compass,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/objects/fire/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_compass", // icon
		"Compass", // pickup
		2, // width
		60,
		NULL,
		IT_POWERUP|IT_ROGUE,
		0,
		NULL,
		0,
		"" // precache
	},

// 67
/*QUAKED item_sphere_vengeance (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_sphere_vengeance", 
		Pickup_Sphere,
		Use_Vengeance,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/vengnce/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_vengeance", // icon
		"Vengeance Sphere", // pickup
		2, // width
		60,
		NULL,
		IT_POWERUP|IT_ROGUE,
		0,
		NULL,
		0,
		"spheres/v_idle.wav" // precache
	},

// 68
/*QUAKED item_sphere_hunter (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_sphere_hunter", 
		Pickup_Sphere,
		Use_Hunter,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/hunter/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_hunter", // icon
		"Hunter Sphere", // pickup
		2, // width
		120,
		NULL,
		IT_POWERUP|IT_ROGUE,
		0,
		NULL,
		0,
		"spheres/h_idle.wav spheres/h_active.wav spheres/h_lurk.wav" // precache
	},

// 69
/*QUAKED item_sphere_defender (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_sphere_defender", 
		Pickup_Sphere,
		Use_Defender,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/defender/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_defender", // icon
		"Defender Sphere", // pickup
		2, // width
		60,													// respawn time
		NULL,												// ammo type used
		IT_POWERUP|IT_ROGUE,								// inventory flags
		0,
		NULL,												// info (void *)
		0,													// tag
		"models/proj/laser2/tris.md2 models/items/shell/tris.md2 spheres/d_idle.wav" // precache
	},

// 70
/*QUAKED item_doppleganger (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_doppleganger",								// classname
		Pickup_Doppleganger,								// pickup function
		Use_Doppleganger,									// use function
		Drop_General,										// drop function
		NULL,												// weapon think function
		"items/pkup.wav",									// pick up sound
		"models/items/dopple/tris.md2",	0, EF_ROTATE,		// world model, skinnum, world model flags
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

// 71
/*QUAKED item_freeze (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"item_freeze",
		Pickup_Powerup,
		Use_Stasis,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/stasis/tris.md2", 0, EF_ROTATE,
		NULL,
		"p_freeze",
		"Stasis Generator",
		2,
		30,
		NULL,
		IT_POWERUP|IT_LAZARUS,
		0,
		NULL,
		0,
		"items/stasis_start.wav items/stasis.wav items/stasis_stop.wav"
	},

// 72
	{
		"dm_tag_token",										// classname
	//	NULL,												// classname
		Tag_PickupToken,									// pickup function
		NULL,												// use function
		NULL,												// drop function
		NULL,												// weapon think function
		"items/pkup.wav",									// pick up sound
		"models/items/tagtoken/tris.md2", 0, EF_ROTATE | EF_TAGTRAIL,	// world model, skinnum, world model flags
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


	//
	// KEYS
	//
// 73
/*QUAKED key_data_cd (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
key for computer centers
*/
	{
		"key_data_cd",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/data_cd/tris.md2", 0, EF_ROTATE,
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
		"" // precache
	},

// 74
/*QUAKED key_dstarchart (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
starchart in Makron's tomb
*/
	{
		"key_dstarchart",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/data_cd/tris.md2", 1, EF_ROTATE,
		NULL,
		"k_dstarchart",
		"Digital Starchart",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
		"" // precache
	},

// 75
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
		"models/items/keys/power/tris.md2", 0, EF_ROTATE,
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
		"" // precache
	},

// 76
/*QUAKED key_pyramid (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
key for the entrance of jail3
*/
	{
		"key_pyramid",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", 0, EF_ROTATE,
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
		"" // precache
	},

// 77
/*QUAKED key_data_spinner (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
key for the city computer
*/
	{
		"key_data_spinner",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/spinner/tris.md2", 0, EF_ROTATE,
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
		"" // precache
	},

// 78
/*QUAKED key_pass (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
security pass for the security level
*/
	{
		"key_pass",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", 0, EF_ROTATE,
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
		"" // precache
	},

// 79
/*QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
normal door key - blue
*/
	{
		"key_blue_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/key/tris.md2", 0, EF_ROTATE,
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
		"" // precache
	},

// 80
/*QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
normal door key - red
*/
	{
		"key_red_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/red_key/tris.md2", 0, EF_ROTATE,
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
		"" // precache
	},

// 81
// RAFAEL
/*QUAKED key_green_key (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
normal door key - green
*/
	{
		"key_green_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/green_key/tris.md2", 0, EF_ROTATE,
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
		"" // precache
	},

// 82
/*QUAKED key_commander_head (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
tank commander's head
*/
	{
		"key_commander_head",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/monsters/commandr/head/tris.md2", 0, EF_GIB,
		NULL,
		"k_comhead", // icon
		"Commander's Head", // pickup
		2, // width
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
		"" // precache
	},

// 83
/*QUAKED key_airstrike_target (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
marker for airstrike
*/
	{
		"key_airstrike_target",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/target/tris.md2", 0, EF_ROTATE,
		NULL,
		"i_airstrike",		// icon
		"Airstrike Marker", // pickup
		2,					// width
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
		"" // precache
	},

// ======================================
// PGM
// 84
/*QUAKED key_nuke_container (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"key_nuke_container",								// classname
		Pickup_Key,											// pickup function
		NULL,												// use function
		Drop_General,										// drop function
		NULL,												// weapon think function
		"items/pkup.wav",									// pick up sound
		"models/weapons/g_nuke/tris.md2", 0, EF_ROTATE,		// world model, skinnum, world model flags
		NULL,												// view model
		"i_contain",										// icon
		"Antimatter Pod",									// name printed when picked up 
		2,													// number of digits for statusbar
		0,													// respawn time
		NULL,												// ammo type used 
		IT_STAY_COOP|IT_KEY|IT_ROGUE,						// inventory flags
		0,
		NULL,												// info (void *)
		0,													// tag
		""													// precache
	},

// 85
/*QUAKED key_nuke (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"key_nuke",											// classname
		Pickup_Key,											// pickup function
		NULL,												// use function
		Drop_General,										// drop function
		NULL,												// weapon think function
		"items/pkup.wav",									// pick up sound
		"models/weapons/g_nuke/tris.md2", 0, EF_ROTATE,		// world model, skinnum, world model flags
		NULL,												// view model
		"i_nuke",											// icon
		"Antimatter Bomb",									// name printed when picked up 
		2,													// number of digits for statusbar
		0,													// respawn time
		NULL,												// ammo type used 
		IT_STAY_COOP|IT_KEY|IT_ROGUE,						// inventory flags
		0,
		NULL,												// info (void *)
		0,													// tag
		""													// precache
	},
// PGM
// ======================================

// 86
// New item (sorta) for Citadel pack by Andrea Rosa
/*QUAKED key_mystery (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
key for Citadel Pack 3.0
*/
	{
		"key_mystery",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/mystery/tris.md2", 0, EF_ROTATE,
		NULL,
		"k_mystery",
		"Mystery Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
		"" // precache
	},

// Zaero keys
// 87
/*QUAKED key_landing_arena (0 .5 .8) (-16 -16 -16) (16 16 16)
landing arena key - blue
*/
	{
		"key_landing_area",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/key/tris.md2", 0, EF_ROTATE,
		NULL,
		"k_bluekey",
		"Airfield Pass",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

// 88
/*QUAKED key_lab (0 .5 .8) (-16 -16 -16) (16 16 16)
security pass for the laboratory
*/
	{
		"key_lab",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", 0, EF_ROTATE,
		NULL,
		"k_security",
		"Laboratory Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

// 89
/*QUAKED key_clearancepass (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_clearancepass",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", 0, EF_ROTATE,
		NULL,
		"k_security",
		"Clearance Pass",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

// 90
/*QUAKED key_energy (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_energy",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/energy/tris.md2", 0, EF_ROTATE,
		NULL,
		"k_energy",
		"Energy Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

// 91
/*QUAKED key_lava (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_lava",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/lava/tris.md2", 0, EF_ROTATE,
		NULL,
		"k_lava",
		"Lava Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

// 92
/*QUAKED key_slime (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_slime",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/slime/tris.md2", 0, EF_ROTATE,
		NULL,
		"k_slime",
		"Slime Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

// 93
/*QUAKED key_q1_gold (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
medevial door key - gold
model="models/items/q1keys/gold/tris.md2"
*/
	{
		"key_q1_gold",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"q1misc/medkey.wav",
		"models/items/q1keys/gold/tris.md2", 0, EF_ROTATE,
		NULL,
		"k_med_gold",						
		"Gold Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_Q1,
		0,
		NULL,
		0,
/* precache */ ""
	},

// 94
/*QUAKED key_q1_silver (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
medevial door key - silver
model="models/items/q1keys/silver/tris.md2"
*/
	{
		"key_q1_silver",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"q1misc/medkey.wav",
		"models/items/q1keys/silver/tris.md2", 0, EF_ROTATE,
		NULL,
		"k_med_silver",								
		"Silver Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_Q1,
		0,
		NULL,
		0,
/* precache */ ""
	},

// 95
/*QUAKED runekey_q1_gold (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
runic door key - gold
model="models/items/q1keys/gold/rune/tris.md2"
*/
	{
		"runekey_q1_gold",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"q1misc/runekey.wav",
		"models/items/q1keys/gold/rune/tris.md2", 0, EF_ROTATE,
		NULL,
		"k_rune_gold",						
		"Gold Rune Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_Q1,
		0,
		NULL,
		0,
/* precache */ ""
	},

// 96
/*QUAKED runekey_q1_silver (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
runic door key - silver
model="models/items/q1keys/silver/rune/tris.md2"
*/
	{
		"runekey_q1_silver",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"q1misc/runekey.wav",
		"models/items/q1keys/silver/rune/tris.md2", 0, EF_ROTATE,
		NULL,
		"k_rune_silver",						
		"Silver Rune Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_Q1,
		0,
		NULL,
		0,
/* precache */ ""
	},

// 97
/*QUAKED basekey_q1_gold (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
base door key - gold
model="models/items/q1keys/gold/base/tris.md2"
*/
	{
		"basekey_q1_gold",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"q1misc/basekey.wav",
		"models/items/q1keys/gold/base/tris.md2", 0, EF_ROTATE,
		NULL,
		"k_base_gold",						
		"Gold Keycard",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_Q1,
		0,
		NULL,
		0,
/* precache */ ""
	},

// 98
/*QUAKED basekey_q1_silver (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
base door key - silver
model="models/items/q1keys/silver/base/tris.md2"
*/
	{
		"basekey_q1_silver",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"q1misc/basekey.wav",
		"models/items/q1keys/silver/base/tris.md2", 0, EF_ROTATE,
		NULL,
		"k_base_silver",						
		"Silver Keycard",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_Q1,
		0,
		NULL,
		0,
/* precache */ ""
	},

	/*
	Insert new key items here

	Key item template fields:
	classname
	pickup sound
	model
	skinnum
	effects?
	icon name
	full (pickup) name
	*/

// 99
	{
		NULL,
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		NULL, 0, 0,
		NULL,
		"i_health",	// icon
		"Health",	// pickup
		3,			// width
		0,
		NULL,
		0,
		0,
		NULL,
		0,
		"items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"   // precache PMM - health sound fix
	},

	// end of list marker
	{NULL}
};


/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
void SP_item_health (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	self->class_id = ENTITY_ITEM_HEALTH;

	if (!self->model)							// Knightmare- allow mapper-spcified model
		self->model = "models/items/healing/medium/tris.md2";
	if (!self->count)							// Knightmare- allow mapper-spcified count
		self->count = 10;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/n_health.wav");
}

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
void SP_item_health_small (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	self->class_id = ENTITY_ITEM_HEALTH_SMALL;

	if (!self->model)							// Knightmare- allow mapper-spcified model
		self->model = "models/items/healing/stimpack/tris.md2";
	if (!self->count)							// Knightmare- allow mapper-spcified count
		self->count = sk_health_bonus_value->value; // Knightmare- made this cvar
	SpawnItem (self, FindItem ("Health"));
	self->style = HEALTH_IGNORE_MAX|HEALTH_SMALL;
	gi.soundindex ("items/s_health.wav");
}

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
void SP_item_health_large (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	self->class_id = ENTITY_ITEM_HEALTH_LARGE;

	if (!self->model)							// Knightmare- allow mapper-spcified model
		self->model = "models/items/healing/large/tris.md2";
	if (!self->count)							// Knightmare- allow mapper-spcified count
		self->count = 25;
	SpawnItem (self, FindItem ("Health"));
	self->style = HEALTH_LARGE;
	gi.soundindex ("items/l_health.wav");
}

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
void SP_item_health_mega (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	self->class_id = ENTITY_ITEM_HEALTH_MEGA;

	if (!self->model)							// Knightmare- allow mapper-spcified model
		self->model = "models/items/mega_h/tris.md2";
	if (!self->count)							// Knightmare- allow mapper-spcified count
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
	self->class_id = ENTITY_ITEM_FOODCUBE;

	self->model = "models/objects/trapfx/tris.md2";
	SpawnItem (self, FindItem ("Health"));
	self->spawnflags |= DROPPED_ITEM;
	self->style = HEALTH_IGNORE_MAX|HEALTH_FOODCUBE;
	//gi.soundindex ("items/s_health.wav");
	gi.soundindex ("items/m_health.wav");
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

	noweapon_index		= ITEM_INDEX(FindItem("No Weapon"));
	jacket_armor_index	= ITEM_INDEX(FindItem("Jacket Armor"));
	combat_armor_index	= ITEM_INDEX(FindItem("Combat Armor"));
	body_armor_index	= ITEM_INDEX(FindItem("Body Armor"));
	power_screen_index	= ITEM_INDEX(FindItem("Power Screen"));
	power_shield_index	= ITEM_INDEX(FindItem("Power Shield"));
	shells_index		= ITEM_INDEX(FindItem("Shells"));
	bullets_index		= ITEM_INDEX(FindItem("Bullets"));
	grenades_index		= ITEM_INDEX(FindItem("Grenades"));
	rockets_index		= ITEM_INDEX(FindItem("Rockets"));
	cells_index			= ITEM_INDEX(FindItem("Cells"));
	slugs_index			= ITEM_INDEX(FindItem("Slugs"));
	fuel_index			= ITEM_INDEX(FindItem("Fuel"));
	homing_index		= ITEM_INDEX(FindItem("Homing Rockets"));
	blaster_index       = ITEM_INDEX(FindItem("Blaster"));
	rl_index			= ITEM_INDEX(FindItem("Rocket Launcher"));
	pr_index			= ITEM_INDEX(FindItem(PLASMA_PICKUP_NAME));		// SKWiD MOD
	hml_index			= ITEM_INDEX(FindItem("Homing Rocket Launcher"));
	pl_index			= ITEM_INDEX(FindItem("prox launcher"));
	magslug_index		= ITEM_INDEX(FindItem("Magslug"));
	flechettes_index	= ITEM_INDEX(FindItem("Flechettes"));
	prox_index			= ITEM_INDEX(FindItem("Prox"));
	disruptors_index	= ITEM_INDEX(FindItem("Disruptors"));
	tesla_index			= ITEM_INDEX(FindItem("Tesla"));
	trap_index			= ITEM_INDEX(FindItem("Trap"));
	shocksphere_index	= ITEM_INDEX(FindItem("Shocksphere"));
	flares_index		= ITEM_INDEX(FindItem("Flares"));
	tbombs_index		= ITEM_INDEX(FindItem("IRED"));
	empnuke_index		= ITEM_INDEX(FindItem("EMPNuke"));
	plasmashield_index	= ITEM_INDEX(FindItem("Plasma Shield"));

	// added for convenience with triger_key sound hack
	key_q1_med_silver_index		= ITEM_INDEX(FindItem("Silver Key"));
	key_q1_med_gold_index		= ITEM_INDEX(FindItem("Gold Key"));
	key_q1_rune_silver_index	= ITEM_INDEX(FindItem("Silver Rune Key"));
	key_q1_rune_gold_index		= ITEM_INDEX(FindItem("Gold Rune Key"));
	key_q1_base_silver_index	= ITEM_INDEX(FindItem("Silver Keycard"));
	key_q1_base_gold_index		= ITEM_INDEX(FindItem("Gold Keycard"));
}

#ifdef JETPACK_MOD
//==============================================================================
void Use_Jet ( edict_t *ent, gitem_t *item )
{
	if (ent->client->jetpack)
	{
		// Currently on... turn it off and store remaining time
		ent->client->jetpack = false;
		ent->client->jetpack_framenum  = 0;
		// Force frame. While using the jetpack ClientThink forces the frame to
		// stand20 when it really SHOULD be jump2. This is fine, but if we leave
		// it at that then the player cycles through the wrong frames to complete
		// his "jump" when the jetpack is turned off. The same thing is done in 
		// ClientThink when jetpack timer expires.
		ent->s.frame = 67;
		gi.sound(ent,CHAN_GIZMO,gi.soundindex("jetpack/shutdown.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		// Knightmare- don't allow activating during stasis- or player can't descend
		if (level.freeze)
		{
			gi.dprintf("Cannot use jetpack while using stasis generator\n");
			return;
		}

		// Currently off. Turn it on, and add time, if any, remaining
		// from last jetpack.
		if ( ent->client->pers.inventory[ITEM_INDEX(item)] )
		{
			ent->client->jetpack = true;
			// Lazarus: Never remove jetpack from inventory (unless dropped)
			// ent->client->pers.inventory[ITEM_INDEX(item)]--;
			ValidateSelectedItem (ent);
			ent->client->jetpack_framenum = level.framenum;
			ent->client->jetpack_activation = level.framenum;
		}
		else if (ent->client->pers.inventory[fuel_index] > 0)
		{
			ent->client->jetpack = true;
			ent->client->jetpack_framenum = level.framenum;
			ent->client->jetpack_activation = level.framenum;
		}
		else
			return;  // Shouldn't have been able to get here, but I'm a pessimist
		gi.sound( ent, CHAN_GIZMO, gi.soundindex("jetpack/activate.wav"), 1, ATTN_NORM, 0);
	}
}
#endif

// added stasis generator support
// Lazarus: Stasis field generator
void Use_Stasis ( edict_t *ent, gitem_t *item )
{
	if (ent->client->jetpack)
	{
		gi.dprintf("Cannot use stasis generator while using jetpack\n");
		return;
	}
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	level.freeze = true;
	level.freezeframes = 0;
	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/stasis_start.wav"), 1, ATTN_NORM, 0);
}


//===============
//ROGUE
void SP_xatrix_item (edict_t *self)
{
	gitem_t	*item;
	int		i;
	char	*spawnClass;

	if (!self->classname)
		return;

	if (!strcmp(self->classname, "ammo_magslug"))
		spawnClass = "ammo_flechettes";
	else if (!strcmp(self->classname, "ammo_trap"))
		spawnClass = "weapon_proxlauncher";
	else if (!strcmp(self->classname, "item_quadfire"))
	{
		float	chance;

		chance = random();
		if (chance < 0.2)
			spawnClass = "item_sphere_hunter";
		else if (chance < 0.6)
			spawnClass = "item_sphere_vengeance";
		else
			spawnClass = "item_sphere_defender";
	}
	else if (!strcmp(self->classname, "weapon_boomer"))
		spawnClass = "weapon_etf_rifle";
	else if (!strcmp(self->classname, "weapon_phalanx"))
		spawnClass = "weapon_plasmabeam";


	// check item spawn functions
	for (i=0,item=itemlist ; i<game.num_items ; i++,item++)
	{
		if (!item->classname)
			continue;
		if (!strcmp(item->classname, spawnClass))
		{	// found it
			SpawnItem (self, item);
			return;
		}
	}
}
//ROGUE
//===============
