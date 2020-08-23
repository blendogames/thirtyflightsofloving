// p_weapon.c

#include "g_local.h"
#include "m_player.h"

qboolean	is_quad;
qboolean	is_double;
/*static*/ qboolean is_quadfire;
/*static*/ byte		is_silenced;

//PGM
static byte		damage_multiplier;
//PGM
// RAFAEL
void weapon_trap_fire (edict_t *ent, qboolean held);

void weapon_grenade_fire (edict_t *ent, qboolean held);

//========
//ROGUE
byte P_DamageModifier (edict_t *ent)
{
	is_quad = 0;
	is_double = 0;
	damage_multiplier = 1;

	if (ent->client->quad_framenum > level.framenum)
	{
		damage_multiplier *= 4;
		is_quad = 1;

		// if we're quad and DF_NO_STACK_DOUBLE is on, return now.
		if (((int)(dmflags->value) & DF_NO_STACK_DOUBLE))
			return damage_multiplier;
	}
	if (ent->client->double_framenum > level.framenum)
	{
		if ((deathmatch->value) || (damage_multiplier == 1))
		{
			damage_multiplier *= 2;
			is_double = 1;
		}
	}
	
	return damage_multiplier;
}
//ROGUE
//========

/*static*/ void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}

/*static*/ void P_ProjectSource2 (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, 
							  vec3_t right, vec3_t up, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource2 (point, _distance, forward, right, up, result);
}

/*
===============
PlayerNoise

Each player can have two noise objects associated with it:
a personal noise (jumping, pain, weapon firing), and a weapon
target noise (bullet wall impacts)

Monsters that don't directly see the player can move
to a noise in hopes of seeing the player from there.
===============
*/
void PlayerNoise(edict_t *who, vec3_t where, int type)
{
	edict_t		*noise;

	if (type == PNOISE_WEAPON)
	{
		if (who->client->silencer_shots)
		{
			who->client->silencer_shots--;
			return;
		}
	}

	if (deathmatch->value)
		return;

	if (who->flags & FL_NOTARGET)
		return;

	if (who->flags & FL_DISGUISED)
	{
		if (type == PNOISE_WEAPON)
		{
			level.disguise_violator = who;
			level.disguise_violation_framenum = level.framenum + 5;
		}
		else
			return;
	}

	if (!who->mynoise)
	{
		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;

		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
	}

	if (type == PNOISE_SELF || type == PNOISE_WEAPON)
	{
		noise = who->mynoise;
		level.sound_entity = noise;
		level.sound_entity_framenum = level.framenum;
	}
	else // type == PNOISE_IMPACT
	{
		noise = who->mynoise2;
		level.sound2_entity = noise;
		level.sound2_entity_framenum = level.framenum;
	}

	VectorCopy (where, noise->s.origin);
	VectorSubtract (where, noise->maxs, noise->absmin);
	VectorAdd (where, noise->maxs, noise->absmax);
	noise->teleport_time = level.time;
	gi.linkentity (noise);
}


qboolean Pickup_Weapon (edict_t *ent, edict_t *other)
{
	int			index;
	gitem_t		*ammo;
	
	//Knightmare- override ammo pickup values with cvars
	SetAmmoPickupValues ();

	index = ITEM_INDEX(ent->item);

	if ( ( ((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value) 
		&& other->client->pers.inventory[index])
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
			return false;	// leave the weapon for others to pickup
	}

	other->client->pers.inventory[index]++;

	if (!(ent->spawnflags & DROPPED_ITEM) )
	{
		// give them some ammo with it
		// PGM -- IF APPROPRIATE!
		if (ent->item->ammo)			//PGM
		{
			ammo = FindItem (ent->item->ammo);
			if ( (int)dmflags->value & DF_INFINITE_AMMO )
				Add_Ammo (other, ammo, 1000);
			else
				Add_Ammo (other, ammo, ammo->quantity);
		}

		if (! (ent->spawnflags & DROPPED_PLAYER_ITEM) )
		{
			if (deathmatch->value)
			{
				if ((int)(dmflags->value) & DF_WEAPONS_STAY)
					ent->flags |= FL_RESPAWN;
				else
					SetRespawn (ent, 30);
			}
			if (coop->value)
				ent->flags |= FL_RESPAWN;
		}
	}

	if (other->client->pers.weapon != ent->item && 
		(other->client->pers.inventory[index] == 1) &&
		( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ||
		other->client->pers.weapon == FindItem("No weapon") ) )
		other->client->newweapon = ent->item;

	// If rocket launcher, give the HML (but no ammo).
	if (index == rl_index)
		other->client->pers.inventory[hml_index] = other->client->pers.inventory[index];

	return true;
}

/*
===============
ShowGUN
VWEP
===============
*/

void ShowGun(edict_t *ent)
{
	if (ent->s.modelindex == (MAX_MODELS-1)) //was 255 
	{
		int i=0;

		if (ent->client->pers.weapon)
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		else
			i = 0;
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}
}

/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void ChangeWeapon (edict_t *ent)
{
	int i;

	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire (ent, false);
		ent->client->grenade_time = 0;
	}

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	// set visible model
	if (ent->s.modelindex == (MAX_MODELS-1)) //was 255
	{
		if (ent->client->pers.weapon)
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		else
			i = 0;
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
		ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
	else
		ent->client->ammo_index = 0;

	if (!ent->client->pers.weapon)
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
//Knightmare- Gen cam code
//Skid - CHASECAM
//	if (!ent->client->chasetoggle)
	if (!ent->client->chaseactive)
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
//end

	// DWH: change weapon model index if necessary
	if (ITEM_INDEX(ent->client->pers.weapon) == noweapon_index)
		ent->s.modelindex2 = 0;
	else
		ent->s.modelindex2 = MAX_MODELS-1; //was 255

	ent->client->anim_priority = ANIM_PAIN;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crpain1;
		ent->client->anim_end = FRAME_crpain4;
	}
	else
	{
		ent->s.frame = FRAME_pain301;
		ent->client->anim_end = FRAME_pain304;	
	}
}

/*
=================
NoAmmoWeaponChange
=================
*/

// PMM - added rogue weapons to the list

void NoAmmoWeaponChange (edict_t *ent)
{
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))] )
	{
		ent->client->newweapon = FindItem ("railgun");
		return;
	}
	// RAFAEL
	if ( ent->client->pers.inventory[ITEM_INDEX (FindItem ("Magslug"))]
		&& ent->client->pers.inventory[ITEM_INDEX (FindItem ("phalanx"))])
	{
		ent->client->newweapon = FindItem ("phalanx");	
		return;
	}
	// RAFAEL
	if ( (ent->client->pers.inventory[ITEM_INDEX (FindItem ("cells"))] >= 2)
		&& ent->client->pers.inventory[ITEM_INDEX (FindItem ("ION Ripper"))])
	{
		ent->client->newweapon = FindItem ("ION Ripper");	
		return;
	}
	// ROGUE
	if ( (ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] >= 2)
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Plasma Beam"))] )
	{
		ent->client->newweapon = FindItem ("Plasma Beam");
		return;
	}
	// -ROGUE
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))] )
	{
		ent->client->newweapon = FindItem ("hyperblaster");
		return;
	}
	// ROGUE
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("flechettes"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("ETF Rifle"))] )
	{
		ent->client->newweapon = FindItem ("ETF Rifle");
		return;
	}
	// -ROGUE
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))] )
	{
		ent->client->newweapon = FindItem ("chaingun");
		return;
	}
	if ( ent->client->pers.inventory[disruptors_index]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Disintegrator"))] )
	{
		ent->client->pers.weapon = FindItem ("Disintegrator");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))] )
	{
		ent->client->newweapon = FindItem ("machinegun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] > 1
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("super shotgun"))] )
	{
		ent->client->newweapon = FindItem ("super shotgun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("shotgun"))] )
	{
		ent->client->newweapon = FindItem ("shotgun");
		return;
	}
	// DWH: Dude may not HAVE a blaster
	//ent->client->newweapon = FindItem ("blaster");
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("blaster"))] )
		ent->client->newweapon = FindItem ("blaster");
	else
		ent->client->newweapon = FindItem ("No Weapon");
}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon (edict_t *ent)
{
	// if just died, put the weapon away
	if (ent->health < 1)
	{
		ent->client->newweapon = NULL;
		ChangeWeapon (ent);
	}

	// added stasis generator support
	// Lazarus: Don't fire if game is frozen
	if (level.freeze)
		return;

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
//PGM
		P_DamageModifier (ent);	
//		is_quad = (ent->client->quad_framenum > level.framenum);
//PGM
		// RAFAEL
		is_quadfire = (ent->client->quadfire_framenum > level.framenum);
		if (ent->client->silencer_shots)
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;
		ent->client->pers.weapon->weaponthink (ent);
	}
}


/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Cmd_DetProx_f (edict_t *ent);

void Use_Weapon (edict_t *ent, gitem_t *in_item)
{
	int			ammo_index;
	gitem_t		*ammo_item;
	int			index;
	gitem_t		*item;
	int			current_weapon_index;

	item  = in_item;
	index = ITEM_INDEX(item);
	current_weapon_index = ITEM_INDEX(ent->client->pers.weapon);

	// see if we're already using it
	//if (item == ent->client->pers.weapon)
	//	return;

	// see if we're already using it
	if ( (index == current_weapon_index) ||
		 ( (index == rl_index)  && (current_weapon_index == hml_index) ) ||
		 ( (index == hml_index) && (current_weapon_index == rl_index)  ) ||
		 ( (index == pl_index) && (current_weapon_index == pl_index)  ) ||
		 ( (index == pr_index) && (current_weapon_index == pr_index)  ) )
	{
		if (current_weapon_index == rl_index)
		{
			if (ent->client->pers.inventory[homing_index] > 0)
			{
				item = FindItem("homing rocket launcher");
				index = hml_index;
			}
			else
				return;
		}
		else if (current_weapon_index == hml_index)
		{
			if (ent->client->pers.inventory[rockets_index] > 0)
			{
				item = FindItem("rocket launcher");
				index = rl_index;
			}
			else
				return;
		}
		// Knightmare- detprox command
		else if (current_weapon_index == pl_index)
		{
			Cmd_DetProx_f (ent);
			return;
		}
		// Knightmare- we already have alterate weapon selection implemented here.
		// So let's use it for the LM Plasma rifle, too!
		else if (current_weapon_index == pr_index)	
		{
			ent->client->pers.plasma_mode = (!ent->client->pers.plasma_mode);
			if (ent->client->pers.plasma_mode)
				gi.cprintf (ent, PRINT_HIGH, "spread plasma\n");
			else
				gi.cprintf (ent, PRINT_HIGH, "bounce plasma\n");
			return;
		}
		else
			return;
	}

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
			// Lazarus: If player is attempting to switch to RL and doesn't have rockets,
			//          but DOES have homing rockets, switch to HRL
			if (index == rl_index)
			{
				if ( (ent->client->pers.inventory[homing_index] > 0) &&
					(ent->client->pers.inventory[hml_index] > 0) )
				{
					ent->client->newweapon = FindItem("homing rocket launcher");
					return;
				}
			}
			gi.cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
			gi.cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent->client->newweapon = item;
}


// RAFAEL 14-APR-98
void Use_Weapon2 (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;
	gitem_t		*nextitem;
	int			index;

	if (strcmp (item->pickup_name, "HyperBlaster") == 0)
	{
		if (item == ent->client->pers.weapon)
		{
			item = FindItem ("ION Ripper");
			index = ITEM_INDEX (item);
			if (!ent->client->pers.inventory[index])
			{
				item = FindItem ("HyperBlaster");
			}
		}
	}
	
	else if (strcmp (item->pickup_name, "Railgun") == 0)
  	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (!ent->client->pers.inventory[ammo_index])
		{
			nextitem = FindItem ("Phalanx");
			ammo_item = FindItem(nextitem->ammo);
			ammo_index = ITEM_INDEX(ammo_item);
			if (ent->client->pers.inventory[ammo_index])
			{
				item = FindItem ("Phalanx");
				index = ITEM_INDEX (item);
				if (!ent->client->pers.inventory[index])
				{
					item = FindItem ("Railgun");
				}
			}
		}
		else if (item == ent->client->pers.weapon)
		{
			item = FindItem ("Phalanx");
			index = ITEM_INDEX (item);
			if (!ent->client->pers.inventory[index])
			{
				item = FindItem ("Railgun");
			}
		}
				
	}

	
	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;
	
	if (item->ammo)
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (!ent->client->pers.inventory[ammo_index] && !g_select_empty->value)
		{
			gi.cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent->client->newweapon = item;
	
}
// END 14-APR-98


/*
================
Drop_Weapon
================
*/
void Drop_Weapon (edict_t *ent, gitem_t *item)
{
	int		index;

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
		return;

	index = ITEM_INDEX(item);
	// see if we're already using it
	if ( ((item == ent->client->pers.weapon) || (item == ent->client->newweapon))&& (ent->client->pers.inventory[index] == 1) )
	{
		gi.cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	// Lazarus: Don't drop rocket launcher if current weapon is homing rocket launcher
	if (index == rl_index)
	{
		int	current_weapon_index;
		current_weapon_index = ITEM_INDEX(ent->client->pers.weapon);
		if (current_weapon_index == hml_index)
		{
			gi.cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
			return;
		}
	}

	Drop_Item (ent, item);
	ent->client->pers.inventory[index]--;

	// Lazarus: if dropped weapon is RL, decrement HML inventory also
	if (item->weapmodel == WEAP_ROCKETLAUNCHER)
		ent->client->pers.inventory[hml_index] = ent->client->pers.inventory[index];
}


/*
================
Weapon_Generic

A generic function to handle the basics of weapon thinking
================
*/
#define FRAME_FIRE_FIRST		(FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST		(FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST	(FRAME_IDLE_LAST + 1)

void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent, qboolean altfire))
{
	int		n;
	int		current_weapon_index = ITEM_INDEX(ent->client->pers.weapon);

	// Knightmare- no weapon activity while controlling turret
	if (ent->flags & FL_TURRET_OWNER)
	{
		ent->client->ps.gunframe = 0;
		ent->client->weaponstate = WEAPON_ACTIVATING;
		return;
	}

	if (ent->deadflag || ent->s.modelindex != (MAX_MODELS-1)) //was 255,  VWep animations screw up corpses
	{
		return;
	}

	// Knightmare- activate and putaway sounds for ION Ripper and Shockwave
	if (ion_ripper_extra_sounds->value && !strcmp (ent->client->pers.weapon->pickup_name, "ION Ripper"))
	{
		if (ent->client->ps.gunframe == 0)
			gi.sound (ent, CHAN_AUTO, gi.soundindex("weapons/ionactive.wav"), 1.0, ATTN_NORM, 0);
#ifdef KMQUAKE2_ENGINE_MOD
		else if (ent->client->ps.gunframe == 37)
			gi.sound (ent, CHAN_AUTO, gi.soundindex("weapons/ionaway.wav"), 1.0, ATTN_NORM, 0);
#endif
	}
	if (!strcmp (ent->client->pers.weapon->pickup_name, "Shockwave"))
	{
		if (ent->client->ps.gunframe == 0)
			gi.sound (ent, CHAN_AUTO, gi.soundindex("weapons/shockactive.wav"), 1.0, ATTN_NORM, 0);
		else if (ent->client->ps.gunframe == 62)
			gi.sound (ent, CHAN_AUTO, gi.soundindex("weapons/shockaway.wav"), 1.0, ATTN_NORM, 0);
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}
		else if ((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;
				
			}
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			if (current_weapon_index == pr_index) // -bat plasma rifle bounce/spread switch
			{
				if (ent->client->pers.plasma_mode)
					gi.cprintf(ent, PRINT_HIGH, "spread plasma\n");
				else
					gi.cprintf(ent, PRINT_HIGH, "bounce plasma\n");
			}
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

		if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;
				
			}
		}
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		// Lazarus: Head off firing 2nd homer NOW, so firing animations aren't played
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTONS_ATTACK) )
		{
			if (ent->client->ammo_index == homing_index)
			{
				if (ent->client->homing_rocket && ent->client->homing_rocket->inuse)
				{
					ent->client->latched_buttons &= ~BUTTONS_ATTACK;
					ent->client->buttons &= ~BUTTONS_ATTACK;
				}
			}
		}

		// Knightmare- catch alt fire commands
		if ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK2)
		{
		//	int	current_weapon_index = ITEM_INDEX(ent->client->pers.weapon);
			
			if (current_weapon_index == pl_index) // prox launcher detonate
			{
				Cmd_DetProx_f (ent);
				ent->client->latched_buttons &= ~BUTTONS_ATTACK;
				ent->client->buttons &= ~BUTTONS_ATTACK;
			}
			if (current_weapon_index == pr_index) // plasma rifle bounce/spread switch
			{
				ent->client->pers.plasma_mode = (!ent->client->pers.plasma_mode);
				if (ent->client->pers.plasma_mode)
					gi.cprintf (ent, PRINT_HIGH, "spread plasma\n");
				else
					gi.cprintf (ent, PRINT_HIGH, "bounce plasma\n");

				ent->client->latched_buttons &= ~BUTTONS_ATTACK;
				ent->client->buttons &= ~BUTTONS_ATTACK;
			}
			else if (current_weapon_index == rl_index) // homing rocket switch
			{
				if (ent->client->pers.inventory[homing_index] > 0)
					Use_Weapon (ent, FindItem("homing rocket launcher"));
				ent->client->latched_buttons &= ~BUTTONS_ATTACK;
				ent->client->buttons &= ~BUTTONS_ATTACK;
				return;
			}
			else if (current_weapon_index == hml_index)
			{
				if (ent->client->pers.inventory[rockets_index] > 0)
					Use_Weapon (ent, FindItem("rocket launcher"));
				ent->client->latched_buttons &= ~BUTTONS_ATTACK;
				ent->client->buttons &= ~BUTTONS_ATTACK;
				return;
			}
		}

		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTONS_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTONS_ATTACK;
			if ((!ent->client->ammo_index) || 
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent->s.frame = FRAME_crattak1-1;
					ent->client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent->s.frame = FRAME_attack1-1;
					ent->client->anim_end = FRAME_attack8;
				}
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			{
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (pause_frames)
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
						if (rand()&15)
							return;
					}
				}
			}

			// SKWiD MOD
			if (current_weapon_index == pr_index)
			{
				if ( ent->client->ps.gunframe == 35 )
					gi.sound(ent, CHAN_WEAPON, gi.soundindex(PLASMA_SOUND_VENT), 1, ATTN_NORM,0 );
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				// FIXME - double should use different sound
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
				else if (ent->client->double_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/ddamage3.wav"), 1, ATTN_NORM, 0);
				// Knightmare- the missing quadfire sounds
				if (ent->client->quadfire_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/quadfire3.wav"), 1, ATTN_NORM, 0);
				fire (ent, ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK2) );
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	}
}


/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

void weapon_grenade_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right, up;
	vec3_t	start;
	int		damage = sk_hand_grenade_damage->value; //was 125
	float	timer;
	int		speed;
	float	radius;

	radius = sk_hand_grenade_radius->value; //was damage + 40
	if (is_quad) {
		damage *= 4;
//		damage *= damage_multiplier;		// PGM
	}
	if (is_double) {
		damage *= 2;
	}

	AngleVectors (ent->client->v_angle, forward, right, up);
	if (ent->client->pers.weapon->tag == AMMO_TESLA)
	{
//		VectorSet(offset, 0, -12, ent->viewheight-26);
		VectorSet(offset, 0, -4, ent->viewheight-22);
	}
	else
	{
//		VectorSet(offset, 8, 8, ent->viewheight-8);
		VectorSet(offset, 2, 6, ent->viewheight-14);
	}
	P_ProjectSource2 (ent->client, ent->s.origin, offset, forward, right, up, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	if (speed > GRENADE_MAXSPEED)
		speed = GRENADE_MAXSPEED;

//	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

// ============
// PGM
	switch(ent->client->pers.weapon->tag)
	{
		case AMMO_GRENADES:
			fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);
			break;
		case AMMO_TESLA:
			fire_tesla (ent, start, forward, damage_multiplier, speed);
			break;
		default:
			fire_prox (ent, start, forward, damage_multiplier, speed);
			break;
	}
// PGM
// ============

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	if (ent->deadflag || ent->s.modelindex != (MAX_MODELS-1)) //was 255,  VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}
/*
void Weapon_Grenade (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_grenade_fire (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_grenade_fire (ent, false);
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}
*/

#define FRAME_IDLE_FIRST		(FRAME_FIRE_LAST + 1)

//void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
//									15                      48						5						11					12				29,34,39,48
void Throw_Generic (edict_t *ent, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_THROW_SOUND,
					int FRAME_THROW_HOLD, int FRAME_THROW_FIRE, int *pause_frames, int EXPLODE,
					void (*fire)(edict_t *ent, qboolean held))
{
	int n;

	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	// Knightmare- no throwing things while controlling turret
	if (ent->flags & FL_TURRET_OWNER)
	{
		ent->client->ps.gunframe = 0;
		ent->client->weaponstate = WEAPON_ACTIVATING;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = FRAME_IDLE_FIRST;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTONS_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTONS_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
		{
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		if (pause_frames)
		{
			for (n = 0; pause_frames[n]; n++)
			{
				if (ent->client->ps.gunframe == pause_frames[n])
				{
					if (rand()&15)
						return;
				}
			}
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == FRAME_THROW_SOUND)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == FRAME_THROW_HOLD)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				switch(ent->client->pers.weapon->tag)
				{
					case AMMO_GRENADES:
						ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
						break;
				}
			}

			// they waited too long, detonate it in their hand
			if (EXPLODE && !ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				fire (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTONS_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = FRAME_FIRE_LAST;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == FRAME_THROW_FIRE)
		{
			ent->client->weapon_sound = 0;
			fire (ent, true);
		}

		if ((ent->client->ps.gunframe == FRAME_FIRE_LAST) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

//void Throw_Generic (edict_t *ent, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_THROW_SOUND,
//						int FRAME_THROW_HOLD, int FRAME_THROW_FIRE, int *pause_frames, 
//						int EXPLOSION_TIME, void (*fire)(edict_t *ent))

void Weapon_Grenade (edict_t *ent)
{
	static int	pause_frames[]	= {29,34,39,48,0};

	Throw_Generic (ent, 15, 48, 5, 11, 12, pause_frames, GRENADE_TIMER, weapon_grenade_fire);
}

void Weapon_Prox (edict_t *ent)
{
	static int	pause_frames[]	= {22, 29, 0};

	Throw_Generic (ent, 7, 27, 99, 2, 4, pause_frames, 0, weapon_grenade_fire);
}

void Weapon_Tesla (edict_t *ent)
{
	static int	pause_frames[]	= {21, 0};

	if ((ent->client->ps.gunframe > 1) && (ent->client->ps.gunframe < 9))
	{
//		if (ent->client && !ent->client->chasetoggle) //Knightmare- fix for third person mode
		if (ent->client && !ent->client->chaseactive) //Knightmare- fix for third person mode
			ent->client->ps.gunindex = gi.modelindex  ("models/weapons/v_tesla2/tris.md2");
	}
	else
	{
//		if (ent->client && !ent->client->chasetoggle) //Knightmare- fix for third person mode
		if (ent->client && !ent->client->chaseactive) //Knightmare- fix for third person mode
			ent->client->ps.gunindex = gi.modelindex  ("models/weapons/v_tesla/tris.md2");
	}

	Throw_Generic (ent, 8, 32, 99, 1, 2, pause_frames, 0, weapon_grenade_fire);
}



/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire (edict_t *ent, qboolean altfire)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
//	int		damage = 120;
	int		damage;			// PGM
	float	radius;
	int		multiplier = 1;

// =====
// PGM
	switch(ent->client->pers.weapon->tag)
	{
		case AMMO_PROX:
			damage = sk_prox_damage->value;
			break;
		default:
			damage = sk_grenade_damage->value;
			break;
	}
// PGM
// =====

	radius = sk_grenade_radius->value;	// damage + 40;
	if (is_quad) {
		damage *= 4;
	//	damage *= damage_multiplier;		//pgm
		multiplier *= 4;
	}
	if (is_double) {
		damage *= 2;
		multiplier *= 2;
	}

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

//	fire_grenade (ent, start, forward, damage, 600, 2.5, radius);
// =====
// PGM
	switch(ent->client->pers.weapon->tag)
	{
		case AMMO_PROX:
			fire_prox (ent, start, forward, multiplier, sk_prox_speed->value); //was damage_multiplier
			break;
		default:
			fire_grenade (ent, start, forward, damage, sk_grenade_speed->value, 2.5, radius, altfire);
			break;
	}
// PGM
// =====
	//Knightmare- Gen cam code
//	if (ent->client && ent->client->chasetoggle)
	if (ent->client && ent->client->chaseactive)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_GRENADE | is_silenced);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_GRENADE | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_GrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
	if (is_quadfire)
		Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}

//==========
//PGM
void Weapon_ProxLauncher (edict_t *ent)
{
	static int      pause_frames[]  = {34, 51, 59, 0};
	static int      fire_frames[]   = {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
	if (is_quadfire)
		Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);

}
//PGM
//==========

/*
======================================================================

ROCKET

======================================================================
*/

edict_t	*rocket_target(edict_t *self, vec3_t start, vec3_t forward)
{
	float       bd, d;
	int			i;
	edict_t	    *who, *best;
	trace_t     tr;
	vec3_t      dir, end;

	VectorMA(start, WORLD_SIZE, forward, end);	// was 8192

	/* Check for aiming directly at a damageable entity */
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent->takedamage != DAMAGE_NO) && (tr.ent->solid != SOLID_NOT))
		return tr.ent;

	/* Check for damageable entity within a tolerance of view angle */
	bd = 0;
	best = NULL;
	for (i=1, who=g_edicts+1; i<globals.num_edicts; i++, who++)	{
		if (!who->inuse)
			continue;
		if (who == self)
			continue;
		if (who->takedamage == DAMAGE_NO)
			continue;
		if (who->solid == SOLID_NOT)
			continue;
		VectorMA(who->absmin,0.5,who->size,end);
		tr = gi.trace (start, vec3_origin, vec3_origin, end, self, MASK_OPAQUE);
		if (tr.fraction < 1.0)
			continue;
		VectorSubtract(end, self->s.origin, dir);
		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if (d > bd) {
			bd = d;
			best = who;
		}
	}
	if (bd > 0.90)
		return best;

	return NULL;
}

void Weapon_RocketLauncher_Fire (edict_t *ent, qboolean altfire)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = sk_rocket_damage->value + (int)(random() * sk_rocket_damage2->value);
	radius_damage = sk_rocket_rdamage->value;
	damage_radius = sk_rocket_radius->value;
	if (is_quad) {
//PGM
		damage *= 4;
	//	damage *= damage_multiplier;
		radius_damage *= 4;
	//	radius_damage *= damage_multiplier;
//PGM
	}
	if (is_double) {
		damage *= 2;
		radius_damage *= 2;
	}
	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// Zaero add
	if (EMPNukeCheck(ent, start))
	{
		ent->client->ps.gunframe++;
		gi.sound (ent, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	// KM changed constant 650 for cvar sk_rocket_speed->value
	if (ent->client->pers.fire_mode)
	{
		edict_t	*target;

		if (ent->client->homing_rocket && ent->client->homing_rocket->inuse)
		{
			ent->client->ps.gunframe++;
			return;
		}
		
		target = rocket_target(ent, start, forward);
		fire_rocket (ent, start, forward, damage, sk_rocket_speed->value, damage_radius, radius_damage, target);
	}
	else
		fire_rocket (ent, start, forward, damage, sk_rocket_speed->value, damage_radius, radius_damage, NULL);

	// send muzzle flash
	// Knightmare- Gen cam code
//	if (ent->client && ent->client->chasetoggle)
	if (ent->client && ent->client->chaseactive)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_ROCKET | is_silenced);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_ROCKET | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_RocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
	if (is_quadfire)
		Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}

void Weapon_HomingMissileLauncher_Fire (edict_t *ent, qboolean altfire)
{
	ent->client->pers.fire_mode = 1;
	Weapon_RocketLauncher_Fire (ent, false);
	ent->client->pers.fire_mode = 0;
}

void Weapon_HomingMissileLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_HomingMissileLauncher_Fire);
}
/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

//void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect, int color)
int Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect, int color)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	int		muzzleflash;

	if (is_quad) {
		damage *= 4;		
	}
	if (is_double) {
		damage *= 2;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	// Zaero add
	if (EMPNukeCheck(ent, start))
	{
		gi.sound (ent, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return 0;
	}
	// end Zaero

	if (!hyper)
		fire_blaster (ent, start, forward, damage, sk_blaster_speed->value, effect, hyper, color);
	else
		fire_blaster (ent, start, forward, damage, sk_hyperblaster_speed->value, effect, hyper, color);

	// Knightmare- select muzzle flash
	if (hyper)
	{
		if (color == BLASTER_GREEN)
//	#ifdef KMQUAKE2_ENGINE_MOD
	#if defined (KMQUAKE2_ENGINE_MOD) || defined (Q2E_ENGINE_MOD)
			muzzleflash = MZ_GREENHYPERBLASTER;
	#else
			muzzleflash = MZ_HYPERBLASTER;
	#endif
		else if (color == BLASTER_BLUE)
			muzzleflash = MZ_BLUEHYPERBLASTER;
//	#ifdef KMQUAKE2_ENGINE_MOD
	#if defined (KMQUAKE2_ENGINE_MOD) || defined (Q2E_ENGINE_MOD)
		else if (color == BLASTER_RED)
			muzzleflash = MZ_REDHYPERBLASTER;
	#endif
		else //standard orange
			muzzleflash = MZ_HYPERBLASTER;
	}
	else
	{
		if (color == BLASTER_GREEN)
			muzzleflash = MZ_BLASTER2;
		else if (color == BLASTER_BLUE)
//	#ifdef KMQUAKE2_ENGINE_MOD
	#if defined (KMQUAKE2_ENGINE_MOD) || defined (Q2E_ENGINE_MOD)
			muzzleflash = MZ_BLUEBLASTER;
	#else
			muzzleflash = MZ_BLASTER;
	#endif
//	#ifdef KMQUAKE2_ENGINE_MOD
	#if defined (KMQUAKE2_ENGINE_MOD) || defined (Q2E_ENGINE_MOD)
		else if (color == BLASTER_RED)
			muzzleflash = MZ_REDBLASTER;
	#endif
		else //standard orange
			muzzleflash = MZ_BLASTER;
	}

	// send muzzle flash	
	// Knightmare- Gen cam code
	if (ent->client && ent->client->chaseactive)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (muzzleflash | is_silenced);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (muzzleflash | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}	
	PlayerNoise(ent, start, PNOISE_WEAPON);
	return 1;
}


void Weapon_Blaster_Fire (edict_t *ent, qboolean altfire)
{
	int		damage;
	int		effect;
	int		color;

	if (deathmatch->value)
		damage = sk_blaster_damage_dm->value;
	else
		damage = sk_blaster_damage->value;

	// Knightmare- select color
	color = sk_blaster_color->value;
	// blaster_color could be any other value, so clamp it
	if (sk_blaster_color->value < 2 || sk_blaster_color->value > 4)
		color = BLASTER_ORANGE;
//#ifndef KMQUAKE2_ENGINE_MOD
#if !defined (KMQUAKE2_ENGINE_MOD) && !defined (Q2E_ENGINE_MOD)
	if (color == BLASTER_RED) color = BLASTER_ORANGE;
#endif

	if (color == BLASTER_GREEN)
		effect = EF_BLASTER|EF_TRACKER;
	else if (color == BLASTER_BLUE)
//#ifdef KMQUAKE2_ENGINE_MOD
#if defined (KMQUAKE2_ENGINE_MOD) || defined (Q2E_ENGINE_MOD)
		effect = EF_BLASTER|EF_BLUEHYPERBLASTER;
#else
		effect = EF_BLUEHYPERBLASTER;
#endif
	else if (color == BLASTER_RED)
		effect = EF_BLASTER|EF_IONRIPPER;
	else //standard orange
		effect = EF_BLASTER;

	Blaster_Fire (ent, vec3_origin, damage, false, effect, color);
	ent->client->ps.gunframe++;
}

void Weapon_Blaster (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
	if (is_quadfire)
		Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}

void Weapon_HyperBlaster_Fire (edict_t *ent, qboolean altfire)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;
	int		color;

	ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

	if (!(ent->client->buttons & BUTTONS_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (! ent->client->pers.inventory[ent->client->ammo_index] )
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		}
		else
		{
			rotation = (ent->client->ps.gunframe - 5) * 2*M_PI/6;
			offset[0] = -4 * sin(rotation);
			offset[1] = 0;
			offset[2] = 4 * cos(rotation);

			// Knightmare- select color
			color = sk_hyperblaster_color->value;
			if (sk_hyperblaster_color->value < 2 || sk_hyperblaster_color->value > 4)
				color = BLASTER_ORANGE;
	//	#ifndef KMQUAKE2_ENGINE_MOD
		#if !defined (KMQUAKE2_ENGINE_MOD) && !defined (Q2E_ENGINE_MOD)
			if (color == BLASTER_RED) color = BLASTER_ORANGE;
		#endif

			if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9))
			{
				if (color == BLASTER_GREEN)
					effect = EF_HYPERBLASTER|EF_TRACKER;
				else if (color == BLASTER_BLUE)
					effect = EF_BLUEHYPERBLASTER;
				else if (color == BLASTER_RED)
					effect = EF_HYPERBLASTER|EF_IONRIPPER;
				else //standard orange
					effect = EF_HYPERBLASTER;
			}
			else
				effect = 0;

			if (deathmatch->value)
				damage = sk_hyperblaster_damage_dm->value;
			else
				damage = sk_hyperblaster_damage->value;
		//	Blaster_Fire (ent, offset, damage, true, effect, color);
			if ( Blaster_Fire (ent, offset, damage, true, effect, color) ) {
				if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
					ent->client->pers.inventory[ent->client->ammo_index]--;
			}

			ent->client->anim_priority = ANIM_ATTACK;
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crattak1 - 1;
				ent->client->anim_end = FRAME_crattak9;
			}
			else
			{
				ent->s.frame = FRAME_attack1 - 1;
				ent->client->anim_end = FRAME_attack8;
			}
		}

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 12 && ent->client->pers.inventory[ent->client->ammo_index])
			ent->client->ps.gunframe = 6;
	}

	if (ent->client->ps.gunframe == 12)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}
}

void Weapon_HyperBlaster (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

	Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
	if (is_quadfire)
		Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
}

/*
======================================================================

MACHINEGUN / CHAINGUN

======================================================================
*/

void Machinegun_Fire (edict_t *ent, qboolean altfire)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = sk_machinegun_damage->value;
	int			kick = 2;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTONS_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad) {
//PGM
	//	damage *= 4;
		damage *= damage_multiplier;
		kick *= 4;
	//	kick *= damage_multiplier;
//PGM
	}
	if (is_double) {
		damage *= 2;
		kick *= 2;
	}

	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	// raise the gun as it is firing
	if (!deathmatch->value)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, damage, kick, sk_machinegun_hspread->value, sk_machinegun_vspread->value, MOD_MACHINEGUN);

	//Knightmare- Gen cam code
//	if (ent->client && ent->client->chasetoggle)
	if (ent->client && ent->client->chaseactive)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | is_silenced);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}
}

void Weapon_Machinegun (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
	if (is_quadfire)
		Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
}

void Chaingun_Fire (edict_t *ent, qboolean altfire)
{
	int			i;
	int			shots;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;
	int			kick = 2;

	if (deathmatch->value)
		damage = sk_chaingun_damage_dm->value;
	else
		damage = sk_chaingun_damage->value;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTONS_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTONS_ATTACK)
		&& ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_attack8;
	}

	if (ent->client->ps.gunframe <= 9)
		shots = 1;
	else if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTONS_ATTACK)
			shots = 2;
		else
			shots = 1;
	}
	else
		shots = 3;

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];

	if (!shots)
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad) {
//PGM
	//	damage *= 4;
		damage *= damage_multiplier;
		kick *= 4;
	//	kick *= damage_multiplier;
//PGM
	}
	if (is_double) {
		damage *= 2;
		kick *= 2;
	}

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	// Zaero add
	if (EMPNukeCheck(ent, ent->s.origin))
	{
		gi.sound (ent, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_bullet (ent, start, forward, damage, kick, sk_chaingun_hspread->value, sk_chaingun_vspread->value, MOD_CHAINGUN);
	}

	// send muzzle flash
	// Knightmare- Gen cam code
//	if (ent->client && ent->client->chasetoggle)
	if (ent->client && ent->client->chaseactive)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}


void Weapon_Chaingun (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);
	if (is_quadfire)
		Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);	
}


/*
======================================================================

SHOTGUN / SUPERSHOTGUN

======================================================================
*/

void weapon_shotgun_fire (edict_t *ent, qboolean altfire)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = sk_shotgun_damage->value;
	int			kick = 8;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad) {
//PGM
		damage *= 4;
	//	damage *= damage_multiplier;
		kick *= 4;
	//	kick *= damage_multiplier;
//PGM
	}
	if (is_double) {
		damage *= 2;
		kick *= 2;
	}

	if (deathmatch->value)
		fire_shotgun (ent, start, forward, damage, kick, sk_shotgun_hspread->value, sk_shotgun_vspread->value, sk_shotgun_count->value, MOD_SHOTGUN);
	else
		fire_shotgun (ent, start, forward, damage, kick, sk_shotgun_hspread->value, sk_shotgun_vspread->value, sk_shotgun_count->value, MOD_SHOTGUN);

	// send muzzle flash
	//Knightmare- Gen cam code
//	if (ent->client && ent->client->chasetoggle)
	if (ent->client && ent->client->chaseactive)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_SHOTGUN | is_silenced);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_SHOTGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Shotgun (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {8, 9, 0};

	Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
	if (is_quadfire)
		Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}


void weapon_supershotgun_fire (edict_t *ent, qboolean altfire)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = sk_sshotgun_damage->value;
	int			kick = 12;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad) {
//PGM
	//	damage *= 4;
		damage *= damage_multiplier;
		kick *= 4;
	//	kick *= damage_multiplier;
//PGM
	}
	if (is_double) {
		damage *= 2;
		kick *= 2;
	}

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 5;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, sk_sshotgun_hspread->value, sk_sshotgun_vspread->value, sk_sshotgun_count->value/2, MOD_SSHOTGUN);
	v[YAW]   = ent->client->v_angle[YAW] + 5;
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, sk_sshotgun_hspread->value, sk_sshotgun_vspread->value, sk_sshotgun_count->value/2, MOD_SSHOTGUN);

	// send muzzle flash
	// Knightmare- Gen cam code
//	if (ent->client && ent->client->chasetoggle)
	if (ent->client && ent->client->chaseactive)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_SSHOTGUN | is_silenced);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_SSHOTGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_SuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
	if (is_quadfire)
		Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}



/*
======================================================================

RAILGUN

======================================================================
*/

void weapon_railgun_fire (edict_t *ent, qboolean altfire)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		damage = sk_railgun_damage_dm->value;
		kick = 200;
	}
	else
	{
		damage = sk_railgun_damage->value;
		kick = 250;
	}

	if (is_quad) {
//PGM
		damage *= 4;
	//	damage *= damage_multiplier;
		kick *= 4;
	//	kick *= damage_multiplier;
//PGM
	}
	if (is_double) {
		damage *= 2;
		kick *= 2;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// Zaero add
	if (EMPNukeCheck(ent, start))
	{
		ent->client->ps.gunframe++;
		gi.sound (ent, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_rail (ent, start, forward, damage, kick);

	// send muzzle flash
	// Knightmare- Gen cam code
//	if (ent->client && ent->client->chasetoggle)
	if (ent->client && ent->client->chaseactive)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_RAILGUN | is_silenced);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_RAILGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Railgun (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
	if (is_quadfire)
		Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
}


/*
======================================================================

BFG10K

======================================================================
*/

void weapon_bfg_fire (edict_t *ent, qboolean altfire)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius = sk_bfg_radius->value;

	// Zaero- moved AngleVectors/VectorSet/P_ProjectSource here
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet (offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// Zaero add
	if (ent->client->ps.gunframe == 9)
	{
	//	ent->flags &= ~FL_BFGMISSFIRE;
		ent->client->bfg_missfire = false;
	}

//	if ( !(ent->flags & FL_BFGMISSFIRE) && EMPNukeCheck(ent, start))
	if ( !ent->client->bfg_missfire && EMPNukeCheck(ent, start) )
	{
	//	ent->flags |= FL_BFGMISSFIRE;
		ent->client->bfg_missfire = true;
		gi.sound (ent, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
	}

//	if (ent->flags & FL_BFGMISSFIRE)
	if (ent->client->bfg_missfire)
	{
		ent->client->ps.gunframe++;
		return;
	}
	// end Zaero

	if (deathmatch->value)
		damage = sk_bfg_damage_dm->value;
	else
		damage = sk_bfg_damage->value;

	if (ent->client->ps.gunframe == 9)
	{
		// send muzzle flash
		// Knightmare- Gen cam code
//		if (ent->client && ent->client->chasetoggle)
		if (ent->client && ent->client->chaseactive)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent->client->oldplayer-g_edicts);
			gi.WriteByte (MZ_BFG | is_silenced);
			gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
		}
		else
		{	
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_BFG | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}

		ent->client->ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
	if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (is_quad) {
//PGM
		damage *= 4;
	//	damage *= damage_multiplier;
//PGM
	}
	if (is_double) {
		damage *= 2;
	}

//	Zaero- moved above
//	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

/*	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
*/

	fire_bfg (ent, start, forward, damage, sk_bfg_speed->value, damage_radius); //was 400

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
}

void Weapon_BFG (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
	if (is_quadfire)
		Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
}

//======================================================================


// RAFAEL
/*
	RipperGun
*/

void weapon_ionripper_fire (edict_t *ent, qboolean altfire)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	offset;
	vec3_t	tempang;
	int		damage;
	int		kick;

	if (deathmatch->value)
	{
		// tone down for deathmatch
		damage = sk_ionripper_damage_dm->value;
		kick = sk_ionripper_kick_dm->value;	// 40
	}
	else
	{
		damage = sk_ionripper_damage->value;
		kick = sk_ionripper_kick->value;	// 60
	}
	
	if (is_quad) {
		damage *= 4;
		kick *= 4;
	}
	if (is_double) {
		damage *= 2;
		kick *= 2;
	}

	VectorCopy (ent->client->v_angle, tempang);
	tempang[YAW] += crandom();

	AngleVectors (tempang, forward, right, NULL);
	
	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	// VectorSet (offset, 0, 7, ent->viewheight - 8);
	VectorSet (offset, 16, 7, ent->viewheight - 8);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// Zaero add
	if (EMPNukeCheck(ent, start))
	{
		ent->client->ps.gunframe++;
		gi.sound (ent, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_ionripper (ent, start, forward, damage, sk_ionripper_speed->value, EF_IONRIPPER);

	// send muzzle flash
	//Knightmare- Gen cam code
//	if (ent->client && ent->client->chasetoggle)
	if (ent->client && ent->client->chaseactive)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_IONRIPPER | is_silenced);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent - g_edicts);
		gi.WriteByte (MZ_IONRIPPER | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;
	PlayerNoise (ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
	
	if (ent->client->pers.inventory[ent->client->ammo_index] < 0)
		ent->client->pers.inventory[ent->client->ammo_index] = 0;
}


void Weapon_Ionripper (edict_t *ent)
{
	static int pause_frames[] = {36, 0};
	static int fire_frames[] = {5, 0};

	Weapon_Generic (ent, 4, 6, 36, 39, pause_frames, fire_frames, weapon_ionripper_fire);

	if (is_quadfire)
		Weapon_Generic (ent, 4, 6, 36, 39, pause_frames, fire_frames, weapon_ionripper_fire);
}


// 
//	Phalanx
//

void weapon_phalanx_fire (edict_t *ent, qboolean altfire)
{
	vec3_t		start;
	vec3_t		forward, right, up;
	vec3_t		offset;
	vec3_t		v;
	int			kick = 12;
	int			damage;
	float		damage_radius;
	int			radius_damage;

	damage = sk_phalanx_damage->value  + (int)(random() * sk_phalanx_damage2->value);
	radius_damage = sk_phalanx_radius_damage->value;
	damage_radius = sk_phalanx_radius->value;
	
	if (is_quad) {
		damage *= 4;
		radius_damage *= 4;
	}
	if (is_double) {
		damage *= 2;
		radius_damage *= 2;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// Zaero add
	if (EMPNukeCheck(ent, start))
	{
		ent->client->ps.gunframe++;
		gi.sound (ent, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	if (ent->client->ps.gunframe == 8)
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW]   = ent->client->v_angle[YAW] - 1.5;
		v[ROLL]  = ent->client->v_angle[ROLL];
		AngleVectors (v, forward, right, up);
			
		fire_plasma (ent, start, forward, damage, sk_phalanx_speed->value, damage_radius, radius_damage);

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index]--;
	}
	else
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW]   = ent->client->v_angle[YAW] + 1.5;
		v[ROLL]  = ent->client->v_angle[ROLL];
		AngleVectors (v, forward, right, up);
		fire_plasma (ent, start, forward, damage, sk_phalanx_speed->value, damage_radius, radius_damage);

		// send muzzle flash
		// Knightmare- Gen cam code
//		if (ent->client && ent->client->chasetoggle)
		if (ent->client && ent->client->chaseactive)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent->client->oldplayer-g_edicts);
			gi.WriteByte (MZ_PHALANX | is_silenced);
			gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
		}
		else
		{	
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_PHALANX | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}

		PlayerNoise(ent, start, PNOISE_WEAPON);
	}
	
	ent->client->ps.gunframe++;
	
}

void Weapon_Phalanx (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 55, 0};
	static int	fire_frames[]	= {7, 8, 0};

	Weapon_Generic (ent, 5, 20, 58, 63, pause_frames, fire_frames, weapon_phalanx_fire);

	if (is_quadfire)
		Weapon_Generic (ent, 5, 20, 58, 63, pause_frames, fire_frames, weapon_phalanx_fire);
}

/*
======================================================================

TRAP

======================================================================
*/

#define TRAP_TIMER			5.0
#define TRAP_MINSPEED		300
#define TRAP_MAXSPEED		700

void weapon_trap_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 125;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad) {
		damage *= 4;
	}
	if (is_double) {
		damage *= 2;
	}

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	// fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);
	fire_trap (ent, start, forward, damage, speed, timer, radius, held);
	
// you don't get infinite traps!  ZOID
//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )

	ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;
}

void Weapon_Trap (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	// Knightmare- no throwing traps while controlling turret
	if (ent->flags & FL_TURRET_OWNER)
	{
		ent->client->ps.gunframe = 0;
		ent->client->weaponstate = WEAPON_ACTIVATING;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTONS_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTONS_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			// RAFAEL 16-APR-98
			// gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/trapcock.wav"), 1, ATTN_NORM, 0);
			// END 16-APR-98

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				// RAFAEL 16-APR-98
				ent->client->weapon_sound = gi.soundindex("weapons/traploop.wav");
				// END 16-APR-98
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_trap_fire (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTONS_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}
		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_trap_fire (ent, false);
			if (ent->client->pers.inventory[ent->client->ammo_index] == 0)
				NoAmmoWeaponChange (ent);
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

//======================================================================
// ROGUE MODS BELOW
//======================================================================

/*
======================================================================

CHAINFIST

======================================================================
*/
#define CHAINFIST_REACH 64

void weapon_chainfist_fire (edict_t *ent, qboolean altfire)
{
	vec3_t	offset;
	vec3_t	forward, right, up;
	vec3_t	start;
	int		damage;

	if (deathmatch->value)
		damage = sk_chainfist_damage_dm->value;
	else
		damage = sk_chainfist_damage->value;

	if (is_quad) {
	//	damage *= damage_multiplier;
		damage *= 4;
	}
	if (is_double) {
		damage *= 2;
	}

	AngleVectors (ent->client->v_angle, forward, right, up);

	// kick back
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	// set start point
	VectorSet(offset, 0, 8, ent->viewheight-4);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee (ent, start, forward, CHAINFIST_REACH, damage, 100, 1, MOD_CHAINFIST);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->client->ps.gunframe++;
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
}

// this spits out some smoke from the motor. it's a two-stroke, you know.
void chainfist_smoke (edict_t *ent)
{
	vec3_t	tempVec, forward, right, up;
	vec3_t	offset;

	AngleVectors(ent->client->v_angle, forward, right, up);
	VectorSet(offset, 8, 8, ent->viewheight -4);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, tempVec);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_CHAINFIST_SMOKE);
	gi.WritePosition (tempVec);
	gi.unicast (ent, 0);
//	gi.multicast (tempVec, MULTICAST_PVS);
}

#define HOLD_FRAMES			0

void Weapon_ChainFist (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {8, 9, 16, 17, 18, 30, 31, 0};

	// these are caches for the sound index. there's probably a better way to do this.
//	static int	idle_index;
//	static int	attack_index;
	float		chance;
	int			last_sequence;
	
	last_sequence = 0;

	// load chainsaw sounds and store the indexes for later use.
//	if (!idle_index && !attack_index)
//	{
//		idle_index = gi.soundindex("weapons/sawidle.wav");
//		attack_index = gi.soundindex("weapons/sawhit.wav");
//	}

	if (ent->client->ps.gunframe == 13 ||
		ent->client->ps.gunframe == 23)			// end of attack, go idle
		ent->client->ps.gunframe = 32;

#if HOLD_FRAMES
	else if (ent->client->ps.gunframe == 9 && ((ent->client->buttons) & BUTTONS_ATTACK))
		ent->client->ps.gunframe = 7;
	else if (ent->client->ps.gunframe == 18 && ((ent->client->buttons) & BUTTONS_ATTACK))
		ent->client->ps.gunframe = 16;
#endif

	// holds for idle sequence
	else if (ent->client->ps.gunframe == 42 && (rand()&7))
	{
		if ((ent->client->pers.hand != CENTER_HANDED) && random() < 0.4)
			chainfist_smoke(ent);
	//	ent->client->ps.gunframe = 40;
	}
	else if (ent->client->ps.gunframe == 51 && (rand()&7))
	{
		if ((ent->client->pers.hand != CENTER_HANDED) && random() < 0.4)
			chainfist_smoke(ent);
	//	ent->client->ps.gunframe = 49;
	}	

	// set the appropriate weapon sound.
	if (ent->client->weaponstate == WEAPON_FIRING)
	//	ent->client->weapon_sound = attack_index;
		ent->client->weapon_sound = gi.soundindex("weapons/sawhit.wav");
	else if (ent->client->weaponstate == WEAPON_DROPPING)
		ent->client->weapon_sound = 0;
	else
	//	ent->client->weapon_sound = idle_index;
		ent->client->weapon_sound = gi.soundindex("weapons/sawidle.wav");

	Weapon_Generic (ent, 4, 32, 57, 60, pause_frames, fire_frames, weapon_chainfist_fire);

//	gi.dprintf("chainfist %d\n", ent->client->ps.gunframe);
	if ((ent->client->buttons) & BUTTONS_ATTACK)
	{
		if (ent->client->ps.gunframe == 13 ||
			ent->client->ps.gunframe == 23 ||
			ent->client->ps.gunframe == 32)
		{
			last_sequence = ent->client->ps.gunframe;
			ent->client->ps.gunframe = 6;
		}
	}

	if (ent->client->ps.gunframe == 6)
	{
		chance = random();
		if (last_sequence == 13)			// if we just did sequence 1, do 2 or 3.
			chance -= 0.34;
		else if (last_sequence == 23)	// if we just did sequence 2, do 1 or 3
			chance += 0.33;
		else if (last_sequence == 32)	// if we just did sequence 3, do 1 or 2
		{
			if (chance >= 0.33)
				chance += 0.34;
		}

		if (chance < 0.33)
			ent->client->ps.gunframe = 14;
		else if (chance < 0.66)
			ent->client->ps.gunframe = 24;
	}

}

/*
======================================================================

DISINTEGRATOR

======================================================================
*/
void weapon_tracker_fire (edict_t *self, qboolean altfire)
{
	vec3_t		forward, right;
	vec3_t		start;
	vec3_t		end;
	vec3_t		offset;
	edict_t		*enemy;
	trace_t		tr;
	int			damage;
	vec3_t		mins, maxs;

	// PMM - felt a little high at 25
	if (deathmatch->value)
		damage = sk_disruptor_damage_dm->value;
	else
		damage = sk_disruptor_damage->value;

	if (is_quad) {
//		damage *= damage_multiplier;		//PGM
		damage *= 4;
	}
	if (is_double) {
		damage *= 2;
	}

	VectorSet(mins, -16, -16, -16);
	VectorSet(maxs, 16, 16, 16);
	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, self->viewheight-8);
	P_ProjectSource (self->client, self->s.origin, offset, forward, right, start);

	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		self->client->ps.gunframe++;
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	// FIXME - can we shorten this? do we need to?
	VectorMA (start, WORLD_SIZE, forward, end);	// was 8192
	enemy = NULL;
	//PMM - doing two traces .. one point and one box.  
	tr = gi.trace (start, vec3_origin, vec3_origin, end, self, MASK_SHOT);
	if (tr.ent != world)
	{	//Knightmare- track all destroyable objects
		if (tr.ent->svflags & SVF_MONSTER || tr.ent->client || tr.ent->svflags & SVF_DAMAGEABLE || tr.ent->takedamage == DAMAGE_YES)
		{
			if (tr.ent->health > 0)
				enemy = tr.ent;
		}
	}
	else
	{
		tr = gi.trace (start, mins, maxs, end, self, MASK_SHOT);
		if (tr.ent != world)
		{
			if (tr.ent->svflags & SVF_MONSTER || tr.ent->client || tr.ent->svflags & SVF_DAMAGEABLE)
			{
				if (tr.ent->health > 0)
					enemy = tr.ent;
			}
		}
	}

	VectorScale (forward, -2, self->client->kick_origin);
	self->client->kick_angles[0] = -1;

	fire_tracker (self, start, forward, damage, sk_disruptor_speed->value, enemy);

	// send muzzle flash
	// Knightmare- Gen cam code
//	if (self->client && self->client->chasetoggle)
	if (self->client && self->client->chaseactive)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (self->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_TRACKER | is_silenced);
		gi.multicast (self->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (self-g_edicts);
		gi.WriteByte (MZ_TRACKER | is_silenced);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	PlayerNoise(self, start, PNOISE_WEAPON);

	self->client->ps.gunframe++;
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		self->client->pers.inventory[self->client->ammo_index] -= self->client->pers.weapon->quantity;
}

void Weapon_Disintegrator (edict_t *ent)
{
	static int	pause_frames[]	= {14, 19, 23, 0};
//	static int	fire_frames[]	= {7, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 9, 29, 34, pause_frames, fire_frames, weapon_tracker_fire);
	if (is_quadfire)
		Weapon_Generic (ent, 4, 9, 29, 34, pause_frames, fire_frames, weapon_tracker_fire);
}

/*
======================================================================

ETF RIFLE

======================================================================
*/
void weapon_etf_rifle_fire (edict_t *ent, qboolean altfire)
{
	vec3_t	forward, right, up;
	vec3_t	start, tempPt;
	int		damage;
	float	damage_radius; //was = 3
	int		radius_damage;
	int		i;
	vec3_t	angles;
	vec3_t	offset;

	damage_radius = sk_etf_rifle_radius->value;
	radius_damage = sk_etf_rifle_radius_damage->value;

	if (deathmatch->value)
		damage = sk_etf_rifle_damage->value;
	else
		damage = sk_etf_rifle_damage->value;

	// PGM - adjusted to use the quantity entry in the weapon structure.
	if (ent->client->pers.inventory[ent->client->ammo_index] < ent->client->pers.weapon->quantity)
	{
		VectorClear (ent->client->kick_origin);
		VectorClear (ent->client->kick_angles);
		ent->client->ps.gunframe = 8;

		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad) {
		damage *= 4;
		damage_radius *= 4;
	}
	if (is_double) {
		damage *= 2;
		damage_radius *= 2;
	}

	for (i=0;i<3;i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.85;
		ent->client->kick_angles[i] = crandom() * 0.85;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
//	AngleVectors (angles, forward, right, NULL);
//	gi.dprintf("v_angle: %s\n", vtos(ent->client->v_angle));
	AngleVectors (ent->client->v_angle, forward, right, up);

	// FIXME - set correct frames for different offsets.

	if (ent->client->ps.gunframe == 6)					// right barrel
	{
	//	gi.dprintf("right\n");
		VectorSet(offset, 15, 8, -8);
	}
	else										// left barrel
	{
	//	gi.dprintf("left\n");
		VectorSet(offset, 15, 6, -8);
	}
	
	VectorCopy (ent->s.origin, tempPt);
	tempPt[2] += ent->viewheight;
	P_ProjectSource2 (ent->client, tempPt, offset, forward, right, up, start);
//	gi.dprintf("start: %s\n", vtos(start));
	fire_flechette (ent, start, forward, damage, sk_etf_rifle_speed->value, damage_radius, radius_damage);

	// send muzzle flash
	//Knightmare- Gen cam code
	if (ent->client && ent->client->chaseactive)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_ETF_RIFLE | is_silenced);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_ETF_RIFLE | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->client->ps.gunframe++;
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - 1;
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - 1;
		ent->client->anim_end = FRAME_attack8;
	}

}

void Weapon_ETF_Rifle (edict_t *ent)
{
	static int	pause_frames[]	= {18, 28, 0};
	static int	fire_frames[]	= {6, 7, 0};
//	static int	idle_seq;

	// note - if you change the fire frame number, fix the offset in weapon_etf_rifle_fire.

//	if (!(ent->client->buttons & BUTTON_ATTACK))
//		ent->client->machinegun_shots = 0;

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->pers.inventory[ent->client->ammo_index] <= 0)
			ent->client->ps.gunframe = 8;
	}

	Weapon_Generic (ent, 4, 7, 37, 41, pause_frames, fire_frames, weapon_etf_rifle_fire);
	if (ent->client->ps.gunframe == 8 && (ent->client->buttons & BUTTONS_ATTACK))
		ent->client->ps.gunframe = 6;
	if (is_quadfire) {
		Weapon_Generic (ent, 4, 7, 37, 41, pause_frames, fire_frames, weapon_etf_rifle_fire);
		if (ent->client->ps.gunframe == 8 && (ent->client->buttons & BUTTONS_ATTACK))
			ent->client->ps.gunframe = 6;
	}
	//gi.dprintf("etf rifle %d\n", ent->client->ps.gunframe);
}

// pgm - this now uses ent->client->pers.weapon->quantity like all the other weapons
//#define HEATBEAM_AMMO_USE		2		
#define	HEATBEAM_DM_DMG			30
#define HEATBEAM_SP_DMG			30

void Heatbeam_Fire (edict_t *ent, qboolean altfire)
{
	vec3_t		start;
	vec3_t		forward, right, up;
	vec3_t		offset;
	int			damage;
	int			kick;

	// for comparison, the hyperblaster is 15/20
	// jim requested more damage, so try 15/15 --- PGM 07/23/98
	if (deathmatch->value)
		damage = sk_plasmabeam_damage->value;
	else
		damage = sk_plasmabeam_damage->value;

	if (deathmatch->value)  // really knock 'em around in deathmatch
		kick = 75;
	else
		kick = 30;

//	if (ent->client->pers.inventory[ent->client->ammo_index] < HEATBEAM_AMMO_USE)
//	{
//		NoAmmoWeaponChange (ent);
//		return;
//	}

	ent->client->ps.gunframe++;

	// Zaero- moved below
	if (ent->client && !ent->client->chaseactive) // Knightmare- fix for third person mode
		ent->client->ps.gunindex = gi.modelindex ("models/weapons/v_beamer2/tris.md2");

	if (is_quad) {
	//	damage *= damage_multiplier;
		damage *= 4;
	//	kick *= damage_multiplier;
		kick *= 4;
	}
	if (is_double) {
		damage *= 2;
		kick *= 2;
	}

	VectorClear (ent->client->kick_origin);
	VectorClear (ent->client->kick_angles);

	// get start / end positions
	AngleVectors (ent->client->v_angle, forward, right, up);

	// This offset is the "view" offset for the beam start (used by trace)
	VectorSet(offset, 7, 2, ent->viewheight-3);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// This offset is the entity offset
	VectorSet(offset, 2, 7, -3);

	// Zaero add
	if (EMPNukeCheck(ent, start))
	{
		gi.sound (ent, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_heat (ent, start, forward, offset, damage, kick, false);

	// send muzzle flash
	// Knightmare- Gen cam code
//	if (ent->client && ent->client->chasetoggle)
	if (ent->client && ent->client->chaseactive)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_HEATBEAM | is_silenced);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_HEATBEAM | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - 1;
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - 1;
		ent->client->anim_end = FRAME_attack8;
	}

}

void Weapon_Heatbeam (edict_t *ent)
{
//	static int	pause_frames[]	= {38, 43, 51, 61, 0};
//	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};
	static int	pause_frames[]	= {35, 0};
//	static int	fire_frames[]	= {9, 0};
	static int	fire_frames[]	= {9, 10, 11, 12, 0};
//	static int	attack_index;
//	static int  off_model, on_model;

//	if ((g_showlogic) && (g_showlogic->value)) {
//		gi.dprintf ("Frame %d, skin %d\n", ent->client->ps.gunframe, ent->client->ps.gunskin);
//	}
	
//	if (!attack_index)
//	{
//		attack_index = gi.soundindex ("weapons/bfg__l1a.wav");
//		off_model = gi.modelindex ("models/weapons/v_beamer/tris.md2");
//		on_model = gi.modelindex ("models/weapons/v_beamer2/tris.md2");
		//ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
//	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
//		ent->client->weapon_sound = attack_index;
		ent->client->weapon_sound = gi.soundindex ("weapons/bfg__l1a.wav");
		if ((ent->client->pers.inventory[ent->client->ammo_index] >= 2) && ((ent->client->buttons) & BUTTONS_ATTACK))
		{
//			if (ent->client->ps.gunframe >= 9 && ((ent->client->buttons) & BUTTON_ATTACK))
//			if (ent->client->ps.gunframe >= 12 && ((ent->client->buttons) & BUTTON_ATTACK))
			if (ent->client->ps.gunframe >= 13)
			{
				ent->client->ps.gunframe = 9;
//				ent->client->ps.gunframe = 8;
//				ent->client->ps.gunskin = 0;
//				ent->client->ps.gunindex = on_model;
				if (ent->client && !ent->client->chaseactive) //Knightmare- fix for third person mode
					ent->client->ps.gunindex = gi.modelindex ("models/weapons/v_beamer2/tris.md2");
			}
			else
			{
//				ent->client->ps.gunskin = 1;
//				ent->client->ps.gunindex = on_model;
				if (ent->client && !ent->client->chaseactive) //Knightmare- fix for third person mode
					ent->client->ps.gunindex = gi.modelindex ("models/weapons/v_beamer2/tris.md2");
			}
		}
		else
		{
//			ent->client->ps.gunframe = 10;
			ent->client->ps.gunframe = 13;
//			ent->client->ps.gunskin = 1;
//			ent->client->ps.gunindex = off_model;
			if (ent->client && !ent->client->chaseactive) //Knightmare- fix for third person mode
				ent->client->ps.gunindex = gi.modelindex ("models/weapons/v_beamer/tris.md2");
		}
	}
	else
	{
//		ent->client->ps.gunskin = 1;
//		ent->client->ps.gunindex = off_model;
		if (ent->client && !ent->client->chaseactive) //Knightmare- fix for third person mode
			ent->client->ps.gunindex = gi.modelindex ("models/weapons/v_beamer/tris.md2");
		ent->client->weapon_sound = 0;
	}

//	Weapon_Generic (ent, 8, 9, 39, 44, pause_frames, fire_frames, Heatbeam_Fire);
	Weapon_Generic (ent, 8, 12, 39, 44, pause_frames, fire_frames, Heatbeam_Fire);
	if (is_quadfire)
		Weapon_Generic (ent, 8, 12, 39, 44, pause_frames, fire_frames, Heatbeam_Fire);
}


/*
======================================================================

SHOCKWAVE

======================================================================
*/

void Shockwave_Fire (edict_t *ent, qboolean altfire)
{
	vec3_t		offset, start;
	vec3_t		forward, right;
	int			damage;
	float		damage_radius;
	int			radius_damage;
	qboolean	emp_missfire = false;	// added for Zaero

	damage = sk_shockwave_damage->value + (int)(random() * sk_shockwave_damage2->value);
	radius_damage = sk_shockwave_rdamage->value;
	damage_radius = sk_shockwave_radius->value;

	if (is_quad) {
		damage *= 4;
		radius_damage *= 4;
	}
	if (is_double) {
		damage *= 2;
		radius_damage *= 2;
	}

	// Zaero- moved here
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;
	VectorSet(offset, 0, 7, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// Zaero add
	if (EMPNukeCheck(ent, start))
	{
		emp_missfire = true;
	//	ent->client->ps.gunframe++;
	//	gi.sound (ent, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
	//	return;
	}
	// end Zaero

	if (ent->client->ps.gunframe == 5) // spin up and fire sound
	{
		if (emp_missfire)
			gi.sound (ent, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (ent, CHAN_AUTO, gi.soundindex("weapons/shockfire.wav"), 1.0, ATTN_NORM, 0);
		ent->client->ps.gunframe++;
		return;
	}
	else if (ent->client->ps.gunframe > 20)
	{
		ent->client->ps.gunframe++;
		return;
	}
	else if (ent->client->ps.gunframe < 20)
	{
		ent->client->ps.gunframe++;
		return;
	}

	// Zaero add
	if (emp_missfire)
	{
		ent->client->ps.gunframe++;
		return;
	}
	// end Zaero

/*	Zaero- moved above
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;
	VectorSet(offset, 0, 7, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
*/

	fire_shock_sphere (ent, start, forward, damage, sk_shockwave_speed->value, damage_radius, radius_damage);

	// send muzzle flash and sound
	// Knightmare- Gen cam code
	if (ent->client && ent->client->chaseactive)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ2_MAKRON_BFG);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ2_MAKRON_BFG);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 1;
}

void Weapon_Shockwave (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Shockwave_Fire);

	// RAFAEL
	if (is_quadfire)
		Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Shockwave_Fire);
}

// SKWiD MOD
/*
======================================================================

Plasma Rifle

======================================================================
*/

void weapon_plasma_rifle_fire (edict_t *ent, qboolean altfire)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;

	if ( ent->client->pers.plasma_mode ) {
		if (deathmatch->value)	// tone down for deathmatch
			damage = (int)sk_plasma_rifle_damage_spread_dm->value;
		else
			damage = (int)sk_plasma_rifle_damage_spread->value;
	}
	else {
		if (deathmatch->value)	// tone down for deathmatch
			damage = (int)sk_plasma_rifle_damage_bounce_dm->value;
		else
			damage = (int)sk_plasma_rifle_damage_bounce->value;
	}

	if (is_quad) {
		damage *= 4;
	}
	if (is_double) {
		damage *= 2;
	}

	// if outa ammo, don't fire
	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe++;

		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex(PLASMA_SOUND_EMPTY), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		
		NoAmmoWeaponChange (ent);
		return;
	}

	if (ent->client->ps.gunframe == 4)
	{
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorScale (forward, -2, ent->client->kick_origin);
		
		// fire weapon
		VectorSet(offset, 8, 8, ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		// Zaero add
		if (EMPNukeCheck(ent, start))
		{
			ent->client->ps.gunframe++;
			gi.sound (ent, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
			return;
		}
		// end Zaero

		if ( ent->client->pers.plasma_mode ) {
			gi.sound( ent, CHAN_WEAPON, gi.soundindex(PLASMA_SOUND_FIRE2), 1, ATTN_NORM,0 );
			fire_plasma_rifle (ent, start, forward, damage, (int)sk_plasma_rifle_speed_spread->value, true);
		}
		else {
			gi.sound( ent, CHAN_WEAPON, gi.soundindex(PLASMA_SOUND_FIRE1), 1, ATTN_NORM,0 );
			fire_plasma_rifle (ent, start, forward, damage, (int)sk_plasma_rifle_speed_bounce->value, false);
		}

		if ( !( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index] -= PLASMA_CELLS_PER_SHOT;	// was -= 1

		// make a big pitch kick with an inverse fall
		ent->client->v_dmg_pitch = -2;
		ent->client->v_dmg_roll = crandom()*2;
		ent->client->v_dmg_time = level.time + DAMAGE_TIME;
	}

	//-bat Silence??
	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_ROCKET | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_Plasma_Rifle (edict_t *ent)
{
	static int	pause_frames[]	= {16, 46, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 11, 46, 51, pause_frames, fire_frames, weapon_plasma_rifle_fire);

	// RAFAEL
	if (is_quadfire)
		Weapon_Generic (ent, 3, 11, 46, 51, pause_frames, fire_frames, weapon_plasma_rifle_fire);
}
// end SKWiD MOD

//======================================================================

void Weapon_Null (edict_t *ent)
{
	if (ent->client->newweapon)
		ChangeWeapon(ent);
}
//======================================================================

qboolean Pickup_Health (edict_t *ent, edict_t *other);
void kick_attack (edict_t *ent )
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = sk_jump_kick_damage->value;
	int			kick = 300;
	trace_t		tr;
	vec3_t		end;

	if (ent->client->quad_framenum > level.framenum) {
		damage *= 4;
		kick *= 4;
	}
	if (ent->client->double_framenum > level.framenum) {
		damage *= 2;
		kick *= 2;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	
	VectorScale (forward, 0, ent->client->kick_origin);
	
	VectorSet(offset, 0, 0,  ent->viewheight-20);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	
	VectorMA( start, 25, forward, end );

	tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);
	
	// don't need to check for water
    if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))    
    {
        if (tr.fraction < 1.0)        
        {            
            if (tr.ent->takedamage)
            {
				if ( tr.ent->health <= 0 )
					return;
				// Knightmare- don't jump kick exploboxes or pushable crates, or insanes, or ambient models
				if (!strcmp(tr.ent->classname, "misc_explobox") || !strcmp(tr.ent->classname, "func_pushable")
					|| !strcmp(tr.ent->classname, "model_spawn") || !strcmp(tr.ent->classname, "model_train")
					|| !strcmp(tr.ent->classname, "misc_insane"))
					return;
				// also don't jumpkick actors, unless they're bad guys
				if (!strcmp(tr.ent->classname, "misc_actor") && (tr.ent->monsterinfo.aiflags & AI_GOOD_GUY))
					return;
				// nor goodguy monsters
				if (strstr(tr.ent->classname, "monster_") && tr.ent->monsterinfo.aiflags & AI_GOOD_GUY)
					return;
				// nor shootable items
				if (tr.ent->item && (strstr(tr.ent->classname, "ammo_") || strstr(tr.ent->classname, "weapon_")
					|| strstr(tr.ent->classname, "item_") || strstr(tr.ent->classname, "key_") || tr.ent->item->pickup == Pickup_Health) )
					return;
				if (((tr.ent != ent) && ((deathmatch->value && ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))) || coop->value) && OnSameTeam (tr.ent, ent)))
					return;
				// zucc stop powerful upwards kicking
			//	forward[2] = 0;
				
				// glass fx
				T_Damage (tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_KICK );
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/kick.wav"), 1, ATTN_NORM, 0);
				PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
				ent->client->jumping = 0; // only 1 jumpkick per jump
            }        
		}
	}
} 
