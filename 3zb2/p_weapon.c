// g_weapon.c

#include "g_local.h"
#include "m_player.h"
#include "bot.h"

qboolean	is_quad;
// RAFAEL
static qboolean is_quadfire;
static byte		is_silenced;
static int		damage_multiplier;	// Knightmare added

void weapon_grenade_fire (edict_t *ent, qboolean held);
// RAFAEL
void weapon_trap_fire (edict_t *ent, qboolean held);

void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
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

void ShowGun(edict_t *ent);
qboolean Pickup_Weapon (edict_t *ent, edict_t *other)
{
	int			index,i;
	gitem_t		*ammo,*item;

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
		ammo = FindItem (ent->item->ammo);
	
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
			Add_Ammo (other, ammo, 1000);
		else
			Add_Ammo (other, ammo, ammo->quantity);

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

	if ((other->svflags & SVF_MONSTER) && ctf->value && other->client->zc.route_trace)
	{
		if (!other->client->zc.first_target)
		{
			for (i = 0;i < (5 * 2);i++)
			{
				if ((other->client->zc.routeindex + i) >= CurrentIndex) break;
				if (Route[other->client->zc.routeindex + i].state == GRS_GRAPSHOT)
				{
					item = Fdi_GRAPPLE;//FindItem("Grapple");
					if (	other->client->pers.inventory[ITEM_INDEX(item)]) item->use(other,item);
					return true;
				}
			}
		}
	}

	if (other->client->pers.weapon != ent->item && 
		(other->client->pers.inventory[index] == 1) &&
		( !deathmatch->value || other->client->pers.weapon == Fdi_BLASTER/*FindItem("blaster")*/ ) )
	{
		if (other->svflags & SVF_MONSTER) ent->item->use(other,ent->item);
		else other->client->newweapon = ent->item;
	}

	if (other->svflags & SVF_MONSTER 
		&& (other->client->pers.weapon == Fdi_BLASTER/*FindItem("blaster") */
		|| other->client->pers.weapon == Fdi_GRENADES/*FindItem("Grenades")*/))
	{
		ent->item->use(other,ent->item);//other->client->pers.weapon = ent->item;
		ShowGun(other);
	}
	return true;
}


/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
// ### Hentai ### BEGIN
void ShowGun (edict_t *ent)
{
	int i,j;

	if (!ent->client->pers.weapon)
	{
		ent->s.modelindex2 = 0;
		return;
	}
	if (!vwep->value)
	{
		ent->s.modelindex2 = MAX_MODELS-1;
		return;
	}

	j = Get_KindWeapon(ent->client->pers.weapon);
	if (j == WEAP_GRAPPLE) j = WEAP_BLASTER;

	ent->s.modelindex2 = MAX_MODELS-1;
	if (ent->client->pers.weapon)
		i = ((j & 0xff) << 8);
	else
		i = 0;

	ent->s.skinnum = (ent - g_edicts - 1) | i;

}
// ### Hentai ### END

void ChangeWeapon (edict_t *ent)
{
	char *mdl;
#ifdef KMQUAKE2_ENGINE_MOD
	int		color;
#endif	// KMQUAKE2_ENGINE_MOD

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

	// Knightmare- set blaster skin based on bolt color
#ifdef KMQUAKE2_ENGINE_MOD
	if (ITEM_INDEX(ent->client->pers.weapon) == blaster_index)
	{
		// select color
		color = (int)sk_blaster_color->value;
		// blaster_color could be any other value, so clamp it
		if ( ((int)sk_blaster_color->value < 1) || ((int)sk_blaster_color->value > 4) )
			color = BLASTER_ORANGE; 
		// CTF color override
		if ( (int)ctf->value && (int)ctf_blastercolors->value && ent->client )
			color = (5 - ent->client->resp.ctf_team);
		ent->client->ps.gunskin = max((color - 1), 0);
	}
	else if (ITEM_INDEX(ent->client->pers.weapon) == rg_index)
	{
		// select color
		color = (int)sk_railgun_skin->value;
		// CTF color override
		if ( (int)ctf->value && (int)ctf_railcolors->value && ent->client )
			color = (ent->client->resp.ctf_team - 1);
		ent->client->ps.gunskin = min(max(color, 0), 3);
	}
	else
		ent->client->ps.gunskin = 0;
#endif	// KMQUAKE2_ENGINE_MOD

// lm ctf
	mdl = ent->client->pers.weapon->view_model;
	if (ctf->value == 2)
	{
		if (Q_stricmp (ent->client->pers.weapon->classname, "weapon_grapple") == 0)
		{
			mdl = "models/weapons/v_hook/tris.md2";
		}

	}
	ent->client->ps.gunindex = gi.modelindex(mdl/*ent->client->pers.weapon->view_model*/);
//lm ctf

	// ### Hentai ### BEGIN
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
	
	ShowGun(ent);	

	// ### Hentai ### END
}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{
	gitem_t*	item = NULL;

	if ( ent->client->pers.inventory[ITEM_INDEX(Fdi_SLUGS/*FindItem("slugs")*/)]
		&&  ent->client->pers.inventory[ITEM_INDEX(Fdi_RAILGUN/*FindItem("railgun")*/)] )
	{
		item = Fdi_RAILGUN;//FindItem ("railgun");
//		return;
	}
	// RAFAEL
	else if ( ent->client->pers.inventory[ITEM_INDEX (Fdi_MAGSLUGS/*FindItem ("mag slug")*/)]
		&& ent->client->pers.inventory[ITEM_INDEX (Fdi_PHALANX/*FindItem ("phalanx")*/)])
	{
		item = Fdi_PHALANX;//FindItem ("phalanx");	
	}
	// RAFAEL
	else if ( ent->client->pers.inventory[ITEM_INDEX (Fdi_CELLS/*FindItem ("cells")*/)]
		&& ent->client->pers.inventory[ITEM_INDEX (Fdi_BOOMER/*FindItem ("ionripper")*/)])
	{
		item = Fdi_BOOMER;//FindItem ("ionrippergun");	
	}
	
	else if ( ent->client->pers.inventory[ITEM_INDEX(Fdi_CELLS/*FindItem("cells")*/)]
		&&  ent->client->pers.inventory[ITEM_INDEX(Fdi_HYPERBLASTER/*FindItem("hyperblaster")*/)] )
	{
		item = Fdi_HYPERBLASTER;//FindItem ("hyperblaster");
//		return;
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(Fdi_BULLETS/*FindItem("bullets")*/)]
		&&  ent->client->pers.inventory[ITEM_INDEX(Fdi_CHAINGUN/*FindItem("chaingun")*/)] )
	{
		item = Fdi_CHAINGUN;//FindItem ("chaingun");
//		return;
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(Fdi_BULLETS/*FindItem("bullets")*/)]
		&&  ent->client->pers.inventory[ITEM_INDEX(Fdi_MACHINEGUN/*FindItem("machinegun")*/)] )
	{
		item = Fdi_MACHINEGUN;//FindItem ("machinegun");
//		return;
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(Fdi_SHELLS/*FindItem("shells")*/)] > 1
		&&  ent->client->pers.inventory[ITEM_INDEX(Fdi_SUPERSHOTGUN/*FindItem("super shotgun")*/)] )
	{
		item = Fdi_SUPERSHOTGUN;//FindItem ("super shotgun");
//		return;
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(Fdi_SHELLS/*FindItem("shells")*/)]
		&&  ent->client->pers.inventory[ITEM_INDEX(Fdi_SHOTGUN/*FindItem("shotgun")*/)] )
	{
		item = Fdi_SHOTGUN;//FindItem ("shotgun");
//		return;
	}
	if (item == NULL) item = Fdi_BLASTER;//FindItem ("blaster");

	if (ent->svflags & SVF_MONSTER) item->use(ent,item);
	else ent->client->newweapon = item;

}

 
/*
=================
GetDamageModifier

// Knightmare- addded double damage
=================
*/
void GetDamageModifier (edict_t *ent)
{
	is_quad = false;
	damage_multiplier = 1;

	if (ent->client->quad_framenum > level.framenum) {
		is_quad = true;
		damage_multiplier *= 4;
	}
	if (ent->client->double_framenum > level.framenum)
	{
		if ( (damage_multiplier == 1) ||
			(deathmatch->value && !((int)(dmflags->value) & DF_NO_STACK_DOUBLE)) )
		{
			is_quad = true;
			damage_multiplier *= 2;
		}
	}
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

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
		GetDamageModifier (ent);	// Knightmare- addded double damage
	//	is_quad = (ent->client->quad_framenum > level.framenum);
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
void Use_Weapon (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

	if (ent->svflags & SVF_MONSTER) 
	{
		if (ent->client->newweapon != NULL) return;
		if (!Q_stricmp (item->pickup_name, "Blaster"))
		{
			ent->client->newweapon = item;
			return;
		}
	}

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
			if (!(ent->svflags & SVF_MONSTER)) gi.cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
			if (!(ent->svflags & SVF_MONSTER)) gi.cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
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

	if (ent->svflags & SVF_MONSTER) 
	{
		Use_Weapon(ent,item);
		return;	
	}

	if (strcmp (item->pickup_name, "HyperBlaster") == 0)
	{
		if (item == ent->client->pers.weapon)
		{
			item = Fdi_BOOMER;//FindItem ("Ionripper");
			index = ITEM_INDEX (item);
			if (!ent->client->pers.inventory[index])
			{
				item = Fdi_HYPERBLASTER;//FindItem ("HyperBlaster");
			}
		}
	}
	
	else if (strcmp (item->pickup_name, "Railgun") == 0)
  	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (!ent->client->pers.inventory[ammo_index])
		{
			nextitem = Fdi_PHALANX;//FindItem ("Phalanx");
			ammo_item = FindItem(nextitem->ammo);
			ammo_index = ITEM_INDEX(ammo_item);
			if (ent->client->pers.inventory[ammo_index])
			{
				item = Fdi_PHALANX;//FindItem ("Phalanx");
				index = ITEM_INDEX (item);
				if (!ent->client->pers.inventory[index])
				{
					item = Fdi_RAILGUN;//FindItem ("Railgun");
				}
			}
		}
		else if (item == ent->client->pers.weapon)
		{
			item = Fdi_PHALANX;//FindItem ("Phalanx");
			index = ITEM_INDEX (item);
			if (!ent->client->pers.inventory[index])
			{
				item = Fdi_RAILGUN;//FindItem ("Railgun");
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
		if (!(ent->svflags & SVF_MONSTER)) gi.cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	Drop_Item (ent, item);
	ent->client->pers.inventory[index]--;
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

void Weapon_Generic2 (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent, qboolean altfire))
{
	int		n;
	int		current_weapon_index = ITEM_INDEX(ent->client->pers.weapon);

	// Knightmare- activate and putaway sounds for ION Ripper
	if (!strcmp (ent->client->pers.weapon->pickup_name, "ION Ripper") && sk_ionripper_extra_sounds->value)
	{
		if (ent->client->ps.gunframe == 0)
			gi.sound (ent, CHAN_AUTO, gi.soundindex("weapons/ionactive.wav"), 1.0, ATTN_NORM, 0);
#ifdef KMQUAKE2_ENGINE_MOD
		else if (ent->client->ps.gunframe == 37)
			gi.sound (ent, CHAN_AUTO, gi.soundindex("weapons/ionaway.wav"), 1.0, ATTN_NORM, 0);
#endif
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}// ### Hentai ### BEGIN
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
		// ### Hentai ### END

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
		// ### Hentai ### BEGIN
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
		// ### Hentai ### END

		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{

		// Knightmare- catch alt fire commands
		if ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK2)
		{
			// Add weapon alt attack handlers here
			// Examples:
		/*	if (current_weapon_index == rl_index) // homing rocket switch
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
			}*/
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
/*if (ent->client->pers.weapon) gi.bprintf(PRINT_HIGH,"weapon %s %i\n"
	,ent->client->pers.weapon->pickup_name
	,ent->client->ammo_index);*/
//	,ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo))]
//	);
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
//ZOID
				if (!CTFApplyStrengthSound(ent))
//ZOID
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
				// Knightmare- addded double damage
				else if (ent->client->double_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/ddamage3.wav"), 1, ATTN_NORM, 0);
//ZOID
				CTFApplyHasteSound(ent);
//ZOID
				fire (ent, ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK2));
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	}
}

//ZOID
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent, qboolean altfire))
{
	int oldstate = ent->client->weaponstate;

	Weapon_Generic2 (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, 
		FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, 
		fire_frames, fire);

	// run the weapon frame again if hasted
	if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		ent->client->weaponstate == WEAPON_FIRING)
		return;

	if ((CTFApplyHaste(ent) ||
		(Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		ent->client->weaponstate != WEAPON_FIRING))
		&& oldstate == ent->client->weaponstate) {
		Weapon_Generic2 (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, 
			FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, 
			fire_frames, fire);
	}
}
//ZOID

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
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = sk_hand_grenade_damage->value; // was 125
	float	timer;
	int		speed;
	float	radius;

	radius = sk_hand_grenade_radius->value;	// was damage + 40

	if (is_quad)
//		damage *= 4;
		damage *= damage_multiplier;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

	// ### Hentai ### BEGIN

/*	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
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
	}*/
	// ### Hentai ### END

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	if (ent->deadflag || ent->s.modelindex != MAX_MODELS-1) // VWep animations screw up corpses
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
	int		damage = sk_grenade_damage->value;	// was 120
	float	radius;

	radius = sk_grenade_radius->value;	// was damage + 40
	if (is_quad)
	//	damage *= 4;
		damage += damage_multiplier;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

//	fire_grenade (ent, start, forward, damage, 600, 2.5, radius, altfire);
	fire_grenade (ent, start, forward, damage, (int)sk_grenade_speed->value, 2.5, radius, altfire);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

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

	// RAFAEL
	if (is_quadfire)
		Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}

/*
======================================================================

ROCKET

======================================================================
*/
void fire_lockon_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);


void Weapon_RocketLauncher_Fire (edict_t *ent, qboolean altfire)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

//	damage = 100 + (int)(random() * 20.0);
	damage = (int)sk_rocket_damage->value + (int)(random() * (int)sk_rocket_damage2->value);
	radius_damage = (int)sk_rocket_rdamage->value;	// was 120
	damage_radius = sk_rocket_radius->value;	// was 120
	if (is_quad)
	{
	//	damage *= 4;
	//	radius_damage *= 4;
		damage *= damage_multiplier;
		radius_damage *= damage_multiplier;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	if (ent->client->zc.aiming != 4) {
		fire_rocket (ent, start, forward, damage, (int)sk_rocket_speed->value, damage_radius, radius_damage);	// was 650
	}
	else 
	{
		damage -= 20;//ロックオンは20ダメージ減り
		fire_lockon_rocket (ent, start, forward, damage, (int)sk_rocket_lockon_speed->value, damage_radius, radius_damage);	// was 20
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

//	ent->client->ps.fov = 90;					//ズーム解除
	ent->client->zc.aiming = 0;					//ズーム不可
}

//ロックオンロケットランチャー
void Weapon_LockonRocketLauncher_Fire (edict_t *ent, qboolean altfire)
{
	vec3_t	tmp,out,aim,min,max;
	vec_t		f;
	trace_t		rs_trace;

	if (ent->client->buttons & BUTTONS_ATTACK)
	{
		ent->client->zc.lockon = false;		//スナイパーにロックオン機能なし
		if (ent->client->zc.aiming == 0)
		{
			gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NONE, 0);
			ent->client->zc.aiming = 3;
			if (ent->client->zc.distance <10 || ent->client->zc.distance > 90) ent->client->zc.distance = 90;
			ent->client->ps.fov = ent->client->zc.distance;//ズーム開始
		}

		VectorSet (max, 8, 8, 8);
		VectorSet (min, -8, -8, -8);	
		AngleVectors (ent->client->v_angle, aim, NULL , NULL);
		VectorNormalize (aim);
		VectorScale (aim, 8193, out);
		VectorCopy (ent->s.origin, tmp);
		if (ent->maxs[2] >= 32) tmp[2] += 22;
		else tmp[2] -= 2;
		VectorAdd(tmp,out,aim);
		rs_trace = gi.trace (tmp, min, max, aim,ent,  MASK_PLAYERSOLID);	
		if (rs_trace.ent != NULL)
		{
			if (Q_stricmp (rs_trace.ent->classname, "player") == 0)
			{
				if (ctf->value)
				{
					if (ent->client->resp.ctf_team != rs_trace.ent->client->resp.ctf_team)				
					{
						ent->client->zc.lockon = true;
					if (ent->client->zc.first_target != rs_trace.ent)
						gi.sound (ent, CHAN_AUTO, gi.soundindex("3zb/locrloc.wav"), 1, ATTN_NORM, 0);
						ent->client->zc.first_target = rs_trace.ent;
					}
					else ent->client->zc.first_target = NULL;
				}
				else
				{
					ent->client->zc.lockon = true;
					if (ent->client->zc.first_target != rs_trace.ent)
						gi.sound (ent, CHAN_AUTO, gi.soundindex("3zb/locrloc.wav"), 1, ATTN_NORM, 0);
					ent->client->zc.first_target = rs_trace.ent;
				}
				return;	//オートズーム反応せず
			}
			else ent->client->zc.first_target = NULL;
		}
		else ent->client->zc.first_target = NULL;

		if (ent->client->zc.autozoom )
		{
			VectorSubtract(ent->s.origin,rs_trace.endpos,tmp);
			f = VectorLength(tmp);
	
			if (f < 200)	ent->client->zc.distance = 90;
		//	else if (f < 300) ent->client->zc.distance = 75;
		//	else if (f < 500) ent->client->zc.distance = 60;
		//	else if (f < 800) ent->client->zc.distance = 45;
			else if (f < 1300)
			{
				ent->client->zc.distance = 90 - (f - 200) / 14.6;//30;
			}
			else ent->client->zc.distance = 14;
	
			if (ent->client->ps.fov != ent->client->zc.distance)
			{
				f = ent->client->ps.fov - ent->client->zc.distance;
				if (f > 15 || f < -15 )
					gi.sound (ent, CHAN_AUTO, gi.soundindex("3zb/zoom.wav"), 1, ATTN_NORM, 0);
				
				ent->client->ps.fov = ent->client->zc.distance;
			}
		}
		return;
	}
	ent->client->zc.aiming = 4;
	Weapon_RocketLauncher_Fire (ent, altfire);
}


void Weapon_RocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	if (!(ent->client->buttons & BUTTONS_ATTACK)) ent->client->zc.aiming = 0;	//アクティベート0

	if (0/*1*/)
	{
		Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_LockonRocketLauncher_Fire);
	}
	else
	{
		Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
	// RAFAEL
	if (is_quadfire)
		Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
	}

}


/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect, int color)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	int		muzzleflash;

	if (is_quad)
//		damage *= 4;
		damage *= damage_multiplier;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);

	if (!(ent->svflags & SVF_MONSTER))
	{
		VectorAdd (offset, g_offset, offset);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;
	}
	else
	{
		VectorSet(offset, 0, 0, ent->viewheight-8);
		VectorAdd (offset, ent->s.origin, start);
	}

	if (!hyper)
		fire_blaster (ent, start, forward, damage, (int)sk_blaster_speed->value, effect, hyper, color);			// was 1000
	else
		fire_blaster (ent, start, forward, damage, (int)sk_hyperblaster_speed->value, effect, hyper, color);	// was 1000

	// Knightmare- changeable color muzzle flash
	if (hyper)
	{
		if (color == BLASTER_GREEN)
	#ifdef KMQUAKE2_ENGINE_MOD
			muzzleflash = MZ_GREENHYPERBLASTER;
	#else
			muzzleflash = MZ_HYPERBLASTER;
	#endif
		else if (color == BLASTER_BLUE)
			muzzleflash = MZ_BLUEHYPERBLASTER;
	#ifdef KMQUAKE2_ENGINE_MOD
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
	#ifdef KMQUAKE2_ENGINE_MOD
			muzzleflash = MZ_BLUEBLASTER;
	#else
			muzzleflash = MZ_BLASTER;
	#endif
	#ifdef KMQUAKE2_ENGINE_MOD
		else if (color == BLASTER_RED)
			muzzleflash = MZ_REDBLASTER;
	#endif
		else //standard orange
			muzzleflash = MZ_BLASTER;
	}
	// end Knightmare

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
/*	if (hyper)
		gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
	else
		gi.WriteByte (MZ_BLASTER | is_silenced); */
	gi.WriteByte (muzzleflash | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void Weapon_Blaster_Fire (edict_t *ent, qboolean altfire)
{
	int		damage;
	int		effect, color;	// Knightmare added

	if (deathmatch->value)
		damage = (int)sk_blaster_damage_dm->value;	// was 15
	else
		damage = (int)sk_blaster_damage->value;		// was 10	

	// Knightmare- selectable color
	color = (int)sk_blaster_color->value;
	// blaster_color could be any other value, so clamp it
	if ( ((int)sk_blaster_color->value < 1) || ((int)sk_blaster_color->value > 4) )
		color = BLASTER_ORANGE; 
	// CTF color override
	if ( (int)ctf->value && (int)ctf_blastercolors->value && ent->client )
		color = (5 - ent->client->resp.ctf_team);
#ifndef KMQUAKE2_ENGINE_MOD
	if (color == BLASTER_RED) color = BLASTER_ORANGE;
#endif

	if (color == BLASTER_GREEN)
		effect = (EF_BLASTER|EF_TRACKER);
	else if (color == BLASTER_BLUE)
#ifdef KMQUAKE2_ENGINE_MOD
		effect = EF_BLASTER|EF_BLUEHYPERBLASTER;
#else
		effect = EF_BLUEHYPERBLASTER;
#endif
	else if (color == BLASTER_RED)
		effect = EF_BLASTER|EF_IONRIPPER;
	else // standard orange
		effect = EF_BLASTER;
	// end Knightmare

//	Blaster_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	Blaster_Fire (ent, vec3_origin, damage, false, effect, color);
	ent->client->ps.gunframe++;
}

void Weapon_Blaster (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
	// RAFAEL
	if (is_quadfire)
		Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}


void Weapon_HyperBlaster_Fire (edict_t *ent, qboolean altfire)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;
	int		color;	// Knightmare added

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

			// Knightmare- selectable color
			color = (int)sk_hyperblaster_color->value;
			// hyperblaster_color could be any other value, so clamp it
			if ( ((int)sk_hyperblaster_color->value < 1) || ((int)sk_hyperblaster_color->value > 4) )
				color = BLASTER_ORANGE;
			// CTF color override
			if ( (int)ctf->value && (int)ctf_blastercolors->value && ent->client )
				color = 5-ent->client->resp.ctf_team;
		#ifndef KMQUAKE2_ENGINE_MOD
			if (color == BLASTER_RED) color = BLASTER_ORANGE;
		#endif
			if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9))
			{
				if (color == BLASTER_GREEN)
					effect = (EF_HYPERBLASTER|EF_TRACKER);
				else if (color == BLASTER_BLUE)
					effect = EF_BLUEHYPERBLASTER;
				else if (color == BLASTER_RED)
					effect = EF_HYPERBLASTER|EF_IONRIPPER;
				else // standard orange
					effect = EF_HYPERBLASTER;
			}
			else
				effect = 0;
			// end Knightmare

		/*	if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9))
				effect = EF_HYPERBLASTER;
			else
				effect = 0; */

			if (deathmatch->value)
				damage = (int)sk_hyperblaster_damage_dm->value;	// was 15
			else
				damage = (int)sk_hyperblaster_damage->value;	// was 20
			Blaster_Fire (ent, offset, damage, true, effect, color);
			// ### Hentai ### BEGIN

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
	
			// ### Hentai ### END

			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.inventory[ent->client->ammo_index]--;
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

	// RAFAEL
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
	int			damage = (int)sk_machinegun_damage->value;	// was 8
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

	if (is_quad)
	{
	//	damage *= 4;
	//	kick *= 4;
		damage *= damage_multiplier;
		kick *= damage_multiplier;
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
//	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
	fire_bullet (ent, start, forward, damage, kick, (int)sk_machinegun_hspread->value, (int)sk_machinegun_vspread->value, MOD_MACHINEGUN);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

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
ent->client->weaponstate = WEAPON_READY;
}

void Weapon_Machinegun (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);

	// RAFAEL
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
		damage = (int)sk_chaingun_damage_dm->value;	// was 6
	else
		damage = (int)sk_chaingun_damage->value;	//was 8

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

	if (is_quad)
	{
	//	damage *= 4;
	//	kick *= 4;
		damage *= damage_multiplier;
		kick *= damage_multiplier;
	}

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	//	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
		fire_bullet (ent, start, forward, damage, kick, (int)sk_chaingun_hspread->value, (int)sk_chaingun_vspread->value, MOD_CHAINGUN);
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	// ### Hentai ### BEGIN

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - 1 + (ent->client->ps.gunframe % 3);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - 1 + (ent->client->ps.gunframe % 3);
		ent->client->anim_end = FRAME_attack8;
	}

	// ### Hentai ### END

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}

#if 0
void Gatringgun_Fire (edict_t *ent, qboolean altfire)
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
		damage = 8;
	else
		damage = 8;

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

	if (ent->client->ps.gunframe <= 9)
		shots = 10;//1;
	else if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTONS_ATTACK)
			shots = 10;//2;
		else
			shots = 10;//1;
	}
	else
		shots = 3;//3;

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];

	if (shots == 10 ) return;

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

	if (is_quad)
	{
	//	damage *= 4;
	//	kick *= 4;
		damage *= damage_multiplier;
		kick *= damage_multiplier;
	}

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
	}

	if (is_silenced) u = 0.5;
	else u = 1.0;

	gi.sound (ent, CHAN_AUTO, gi.soundindex("3zb/gatgf.wav"), u, ATTN_NORM, 0);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_BLASTER) | MZ_SILENCED);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	// ### Hentai ### BEGIN

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - 1 + (ent->client->ps.gunframe % 3);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - 1 + (ent->client->ps.gunframe % 3);
		ent->client->anim_end = FRAME_attack8;
	}

	// ### Hentai ### END

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 1;//shots;
}
#endif

void Weapon_Chaingun (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

#if 0
	if (0)
		Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Gatringgun_Fire);
	else
#endif
	{
		Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);
		// RAFAEL
		if (is_quadfire)
			Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);

	}
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
	int			damage = (int)sk_shotgun_damage->value;	// was 4
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

	if (is_quad)
	{
	//	damage *= 4;
	//	kick *= 4;
		damage *= damage_multiplier;
		kick *= damage_multiplier;
	}

	if (deathmatch->value)
	//	fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
		fire_shotgun (ent, start, forward, damage, kick, (int)sk_shotgun_hspread->value, (int)sk_shotgun_vspread->value, (int)sk_shotgun_count->value, MOD_SHOTGUN);
	else
	//	fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
		fire_shotgun (ent, start, forward, damage, kick, (int)sk_shotgun_hspread->value, (int)sk_shotgun_vspread->value, (int)sk_shotgun_count->value, MOD_SHOTGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

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
	// RAFAEL
	if (is_quadfire)
		Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}


void weapon_supershotgun_fire (edict_t *ent, qboolean altfire)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = (int)sk_sshotgun_damage->value;	// was 6
	int			kick = 12;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
	//	damage *= 4;
	//	kick *= 4;
		damage *= damage_multiplier;
		kick *= damage_multiplier;
	}

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 5;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
//	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
	fire_shotgun (ent, start, forward, damage, kick, (int)sk_sshotgun_hspread->value, (int)sk_sshotgun_vspread->value, (int)sk_sshotgun_count->value/2, MOD_SSHOTGUN);
	v[YAW]   = ent->client->v_angle[YAW] + 5;
	AngleVectors (v, forward, NULL, NULL);
//	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
	fire_shotgun (ent, start, forward, damage, kick, (int)sk_sshotgun_hspread->value, (int)sk_sshotgun_vspread->value, (int)sk_sshotgun_count->value/2, MOD_SSHOTGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SSHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

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
	// RAFAEL
	if (is_quadfire)
		Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}



/*
======================================================================

RAILGUN

======================================================================
*/
void fire_sniperail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);

void RSight_think (edict_t *ent)
{
	trace_t		rs_trace;	
	vec3_t		aim,out,tmp;
	vec3_t		max,min;

	vec_t		f;

	if (ent->owner->client->ps.gunframe != 4 || ent->owner->deadflag)
	{
		G_FreeEdict(ent);
		return;
	} 	
	VectorSet (max, 4 ,4, 4);
	VectorSet (min, -4, -4, -4);	
	AngleVectors (ent->owner->client->v_angle, aim, NULL , NULL);
	VectorNormalize(aim);
	VectorScale (aim, 8193, out);
	VectorCopy(ent->owner->s.origin,tmp);
	if (ent->owner->maxs[2] >= 32) tmp[2] += 22;
	else tmp[2] -= 2;
	VectorAdd(tmp,out,aim);
	rs_trace = gi.trace (tmp, min, max, aim,ent->owner,  MASK_PLAYERSOLID);	
	VectorCopy(rs_trace.endpos,ent->s.origin);
	ent->nextthink = level.time + FRAMETIME;

	if (rs_trace.ent != NULL)
	{
		if (Q_stricmp (rs_trace.ent->classname, "player") == 0) return;//オートズーム反応せず
	}

	if (ent->owner->client->zc.autozoom )
	{
		VectorSubtract(ent->s.origin,ent->owner->s.origin,tmp);
		f = VectorLength(tmp);

		if (f < 100)	ent->owner->client->zc.distance = 90;
//		else if (f < 300) ent->owner->client->zc.distance = 75;
//		else if (f < 500) ent->owner->client->zc.distance = 60;
//		else if (f < 800) ent->owner->client->zc.distance = 45;
		else if (f < 1000)
		{
			ent->owner->client->zc.distance = 90 - (f - 100) / 12;//30;
		}
		else ent->owner->client->zc.distance = 15;

		if (ent->owner->client->ps.fov != ent->owner->client->zc.distance)
		{
			f = ent->owner->client->ps.fov - ent->owner->client->zc.distance;
			if ( f > 15 || f < -15)
				gi.sound (ent->owner, CHAN_AUTO, gi.soundindex("3zb/zoom.wav"), 1, ATTN_NORM, 0);

			ent->owner->client->ps.fov = ent->owner->client->zc.distance;
		}
	}
}


void weapon_railgun_fire (edict_t *ent, qboolean altfire)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;
	int			red=20, green=48, blue=176;
	qboolean	useColor=false;

	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		damage = (int)sk_railgun_damage_dm->value;	// was 100
		kick = 200;
	}
	else
	{
		damage = (int)sk_railgun_damage->value;	// was 150
		kick = 250;
	}

	if (is_quad)
	{
	//	damage *= 4;
	//	kick *= 4;
		damage *= damage_multiplier;
		kick *= damage_multiplier;
	}

	// Knightmare- CTF color override
	if ( (int)ctf->value && (int)ctf_railcolors->value && ent->client )
	{
		useColor = true;
		if (ent->client->resp.ctf_team == CTF_TEAM1) {
			red = 176;	green = 20;	blue = 20;
		}
		else if (ent->client->resp.ctf_team == CTF_TEAM2) {
			red = 20;	green = 20;	blue = 176;
		}
	/*	else if (ent->client->resp.ctf_team == CTF_TEAM3) {
			red = 20;	green = 176;	blue = 20;
		} */
	}
	// Knightmare- custom client color
	else if ( ent->client && (ent->client->pers.color1[3] != 0) )
	{
		useColor = true;
		red = ent->client->pers.color1[0];
		green = ent->client->pers.color1[1];
		blue = ent->client->pers.color1[2];
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (ent->client->zc.aiming == 0) {
		fire_rail (ent, start, forward, damage, kick, useColor, red, green, blue);
	}
	else
	{
		damage += 20;
		fire_sniperail (ent, start, forward, damage, kick);
	}

//	gi.bprintf(PRINT_HIGH,"jj %i\n",ent->moveinfo.sound_start);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

//	ent->client->ps.fov = 90;					//ズーム解除
	ent->client->zc.aiming = 0;					//ズーム不可
}
//スナイパー用railガン
void Weapon_SnipeRailgun (edict_t *ent, qboolean altfire)
{
	edict_t	*sight;

	if (ent->client->buttons & BUTTONS_ATTACK)
	{
		ent->client->zc.lockon = false;		//スナイパーにロックオン機能なし
		if ( ent->client->zc.aiming == 0)
		{
			//サイトの作成
			sight = G_Spawn();
			VectorClear (sight->mins);
			VectorClear (sight->maxs);
			sight->movetype = MOVETYPE_FLYMISSILE;
			sight->solid = SOLID_NOT;
			sight->owner = ent;
			sight->s.modelindex = gi.modelindex ("sprites/zsight.sp2");
			sight->s.effects = 0;
			sight->s.renderfx = RF_SHELL_RED;
			sight->think = RSight_think;
			sight->nextthink = level.time + FRAMETIME;
			sight->classname = "rail sight";		
			if ( ent->client->resp.ctf_team == CTF_TEAM2 && ctf->value)
			{ 
				sight->s.frame = 1;
			}
			else sight->s.frame = 0;

			gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NONE, 0);
			ent->client->zc.aiming = 1;
			if (ent->client->zc.distance <10 || ent->client->zc.distance > 90) ent->client->zc.distance = 90;
			ent->client->ps.fov = ent->client->zc.distance;//ズーム開始
		}
		return;
	}
	
//	if (ent->client->buttons & BUTTONS_ATTACK) return;
	ent->client->zc.aiming = 2;
	weapon_railgun_fire (ent, altfire);
}

void Weapon_Railgun (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	if (!(ent->client->buttons & BUTTONS_ATTACK)) ent->client->zc.aiming = 0;	//アクティベート0

	if (0)
	{
		Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, Weapon_SnipeRailgun/*weapon_railgun_fire*/);
	}
	else
	{
		Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
		// RAFAEL
		if (is_quadfire)
			Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
	}
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
	float	damage_radius = sk_bfg_radius->value;	// was 1000

	if (deathmatch->value)
		damage = (int)sk_bfg_damage_dm->value;	// was 200
	else
		damage = (int)sk_bfg_damage->value;		// was 500

	if (ent->client->ps.gunframe == 9)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BFG | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

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

	if (is_quad)
	//	damage *= 4;
		damage *= damage_multiplier;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bfg (ent, start, forward, damage, (int)sk_bfg_speed->value, damage_radius);	// was 400

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
	// RAFAEL
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
		damage = (int)sk_ionripper_damage_dm->value;	// was 30
		kick = (int)sk_ionripper_kick_dm->value;		// was 40
	}
	else
	{
		damage = (int)sk_ionripper_damage->value;	// was 50
		kick = (int)sk_ionripper_kick->value;		// was 60
	}
	
	if (is_quad)
	{
	//	damage *= 4;
	//	kick *= 4;
		damage *= damage_multiplier;
		kick *= damage_multiplier;
	}

	VectorCopy (ent->client->v_angle, tempang);
	tempang[YAW] += crandom();

	AngleVectors (tempang, forward, right, NULL);
	
	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	// VectorSet (offset, 0, 7, ent->viewheight - 8);
	VectorSet (offset, 16, 7, ent->viewheight - 8);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_ionripper (ent, start, forward, damage, (int)sk_ionripper_speed->value, EF_IONRIPPER);	// was 500

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent - g_edicts);
	gi.WriteByte (MZ_IONRIPPER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

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

//	damage = 70 + (int)(random() * 10.0);
	damage = (int)sk_phalanx_damage->value  + (int)(random() * (int)sk_phalanx_damage2->value);
	radius_damage = (int)sk_phalanx_radius_damage->value;	// was 120
	damage_radius = sk_phalanx_radius->value;				// was 120
	
	if (is_quad)
	{
	//	damage *= 4;
	//	radius_damage *= damage_multiplier;
		damage *= 4;
		radius_damage *= damage_multiplier;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (ent->client->ps.gunframe == 8)
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW]   = ent->client->v_angle[YAW] - 1.5;
		v[ROLL]  = ent->client->v_angle[ROLL];
		AngleVectors (v, forward, right, up);
		
		radius_damage = 30;
		damage_radius = 120;
	
		fire_plasma (ent, start, forward, damage, (int)sk_phalanx_speed->value, damage_radius, radius_damage);	// was 725

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index]--;
	}
	else
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW]   = ent->client->v_angle[YAW] + 1.5;
		v[ROLL]  = ent->client->v_angle[ROLL];
		AngleVectors (v, forward, right, up);
		fire_plasma (ent, start, forward, damage, (int)sk_phalanx_speed->value, damage_radius, radius_damage);	// was 725

		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_PHALANX | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		
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
	if (is_quad)
	//	damage *= 4;
		damage *= damage_multiplier;

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
