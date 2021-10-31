// p_weapon.c

#include "g_local.h"
#include "m_player.h"

//CW++
#ifdef LINUX
	#define min(a,b) (((a)<(b))?(a):(b))
#endif
//CW--

qboolean	is_quad;																				//CW
static byte	is_silenced;


void P_ProjectSource(gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy(distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1.0F;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0.0F;

	G_ProjectSource(point, _distance, forward, right, result);
}


/*
===============
PlayerNoise

Each player can have two noise objects associated with it:
a personal noise (jumping, pain, weapon firing), and a weapon
target noise (bullet wall impacts)

Bots that don't directly see opponents can move
to a noise in hopes of seeing their enemy from there.
===============
*/
void PlayerNoise(edict_t *who, vec3_t where, int type)
{
	edict_t	*noise;

//CW++
	if (!who)
		return;
//CW--

	if ((type == PNOISE_WEAPON) && who->client)														//CW
	{
		if (who->client->silencer_shots)
		{
			who->client->silencer_shots--;
			return;
		}
	}

//CW++
	if (NumBotsInGame == 0)
		return;
//CW--

	if (who->flags & FL_NOTARGET)
		return;

	if (!who->mynoise)
	{
		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet(noise->mins, -8, -8, -8);
		VectorSet(noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;

		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet(noise->mins, -8, -8, -8);
		VectorSet(noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
	}

	if ((type == PNOISE_SELF) || (type == PNOISE_WEAPON))
		noise = who->mynoise;
	else	//PNOISE_IMPACT
		noise = who->mynoise2;

	VectorCopy(where, noise->s.origin);
	VectorSubtract(where, noise->maxs, noise->absmin);
	VectorAdd(where, noise->maxs, noise->absmax);
	noise->teleport_time = level.time;

	gi.linkentity(noise);
}


qboolean Pickup_Weapon(edict_t *ent, edict_t *other)
{
	gitem_t	*ammo;
	int		index;

	index = ITEM_INDEX(ent->item);

//Maj++
	CheckCampSite(ent, other);
//Maj--

	if (((int)dmflags->value & DF_WEAPONS_STAY) && other->client->pers.inventory[index])			//CW
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
			return false;	// leave the weapon for others to pickup
	}

	other->client->pers.inventory[index]++;

	if (!(ent->spawnflags & DROPPED_ITEM))
	{
		// give them some ammo with it
		ammo = FindItem(ent->item->ammo);
		if ((int)dmflags->value & DF_INFINITE_AMMO)
			Add_Ammo(other, ammo, 1000);
		else
			Add_Ammo(other, ammo, ammo->quantity);

		if (!(ent->spawnflags & DROPPED_PLAYER_ITEM))
		{
			if ((int)(dmflags->value) & DF_WEAPONS_STAY)											//CW
				ent->flags |= FL_RESPAWN;
			else
			{
//CW++
				if (ent->delay)
					SetRespawn(ent, ent->delay);
				else
//CW--
					SetRespawn(ent, 30);
			}
		}
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
void ChangeWeapon(edict_t *ent)
{
	int i;

//CW++
	int	mode;

	if (!ent->client)
	{
		gi.dprintf("BUG: ChangeWeapon() called with non-client edict.\n");
		if (ent->classname)
			gi.dprintf("     classname = %s\n", ent->classname);
		if (ent->owner && ent->owner->client)
			gi.dprintf("     owner->name = %s\n", ent->owner->client->pers.netname);
		return;
	}
//CW--

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	// set visible model
	if (ent->s.modelindex == (MAX_MODELS-1))
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
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

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

//CW++
	ent->client->show_gausscharge = false;
	ent->client->show_gausstarget = 0;

	ent->client->agm_charge = 0;
	ent->client->agm_showcharge = false;
	ent->client->agm_tripped = false;
	ent->client->agm_on = false;
	ent->client->agm_push = false;
	ent->client->agm_pull = false;
	ent->client->agm_target = NULL;

	if (ent->isabot)
		return;

	mode = ent->client->pers.weap_note;

	if (ent->client->pers.weapon->weapmodel == WEAP_DESERTEAGLE)
	{
		if ((mode & WN_VALL) && (ent->client->pers.lastweapon->weapmodel != WEAP_DESERTEAGLE))
			unicastSound(ent, gi.soundindex("voice/u_pistol.wav"), 1.0);							//r1,CW
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_GAUSSPISTOL)
	{
		if (ent->client->gauss_particle)
		{
			if (ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] < GP_SLUGS_PER_SHOT)
			{
				gi_centerprintf(ent, "Not enough Slugs ...\nSwitching to Blaster mode\n");
				ent->client->gauss_particle = false;
				ent->client->show_gausstarget = 0;
				ent->client->show_gausscharge = true;
			}
			else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] < GP_CELLS_PER_SHOT)
			{
				gi_centerprintf(ent, "Not enough Cells ...\nSwitching to Blaster mode\n");
				ent->client->gauss_particle = false;
				ent->client->show_gausstarget = 0;
				ent->client->show_gausscharge = true;
			}
			else
			{
				ent->client->show_gausstarget = 1;
				ent->client->show_gausscharge = false;
				ent->client->showinventory = 0;
				ent->client->showscores = 0;
			}
		}

		if (ent->client->gauss_particle)
		{
			if (mode & WN_TEXT)
				gi_centerprintf(ent, "Particle Beam\n");
			if (mode & WN_VSEC)
				unicastSound(ent, gi.soundindex("voice/u_partcl.wav"), 1.0);						//r1,CW
		}
		else
		{
			if (mode & WN_TEXT)
				gi_centerprintf(ent, "Blaster\n");
			if (mode & WN_VSEC)
				unicastSound(ent, gi.soundindex("voice/u_blast.wav"), 1.0);							//r1,CW
		}

		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/gauss/use.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_JACKHAMMER)
	{
		if ((mode & WN_VALL) && (ent->client->pers.lastweapon->weapmodel != WEAP_JACKHAMMER))
			unicastSound(ent, gi.soundindex("voice/u_jack.wav"), 1.0);								//r1,CW

		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_MAC10)
	{
		if ((mode & WN_VALL) && (ent->client->pers.lastweapon->weapmodel != WEAP_MAC10))
			unicastSound(ent, gi.soundindex("voice/u_mac10.wav"), 1.0);								//r1,CW
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_C4)
	{
		if (mode & WN_VALL)
			unicastSound(ent, gi.soundindex("voice/u_c4.wav"), 1.0);								//r1,CW
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_TRAP)
	{
		if (mode & WN_VALL)
			unicastSound(ent, gi.soundindex("voice/u_traps.wav"), 1.0);								//r1,CW
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_ESG)
	{
		if (ent->client->multi_spike)
		{
			if (mode & WN_TEXT)
				gi_centerprintf(ent, "Multi-spikes\n");
			if (mode & WN_VSEC)
				unicastSound(ent, gi.soundindex("voice/u_multi.wav"), 1.0);							//r1,CW
		}
		else
		{
			if (mode & WN_TEXT)
				gi_centerprintf(ent, "Single spikes\n");
			if (mode & WN_VSEC)
				unicastSound(ent, gi.soundindex("voice/u_spike.wav"), 1.0);							//r1,CW
		}

		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/esg/use.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_FLAMETHROWER)
	{
		if (ent->client->ft_firebomb)
		{
			if (ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] < FT_CELLS_PER_SHOT)
			{
				gi_centerprintf(ent, "Not enough Cells ...\nSwitching to Standard mode\n");
				ent->client->ft_firebomb = false;
			}
		}

		if (ent->client->ft_firebomb)
		{
			if (mode & WN_TEXT)
				gi_centerprintf(ent, "Firebombs\n");
			if (mode & WN_VSEC)
				unicastSound(ent, gi.soundindex("voice/u_firebm.wav"), 1.0);						//r1,CW
		}
		else
		{
			if (mode & WN_TEXT)	
				gi_centerprintf(ent, "Standard Flamethrower\n");
			if (mode & WN_VSEC)
				unicastSound(ent, gi.soundindex("voice/u_flames.wav"), 1.0);						//r1,CW
		}

		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/flamer/use.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_ROCKETLAUNCHER)
	{
		if (ent->client->normal_rockets)
		{
			if (mode & WN_TEXT)
				gi_centerprintf(ent, "Normal Rockets\n");
			if (mode & WN_VSEC)
				unicastSound(ent, gi.soundindex("voice/u_nrockt.wav"), 1.0);						//r1,CW
		}
		else
		{
			if (mode & WN_TEXT)
				gi_centerprintf(ent, "Guided Rockets\n");
			if (mode & WN_VSEC)
				unicastSound(ent, gi.soundindex("voice/u_grockt.wav"), 1.0);						//r1,CW
		}
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_RAILGUN)
	{
		if (mode & WN_VALL)						
			unicastSound(ent, gi.soundindex("voice/u_rail.wav"), 1.0);								//r1,CW
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_SHOCKRIFLE)
	{
		if (ent->client->homing_plasma)
		{
			if (mode & WN_TEXT)
				gi_centerprintf(ent, "Homing Plasma\n");
			if (mode & WN_VSEC)
				unicastSound(ent, gi.soundindex("voice/u_homin.wav"), 1.0);							//r1,CW
		}
		else
		{
			if (mode & WN_TEXT)
				gi_centerprintf(ent, "Disintegrator\n");
			if (mode & WN_VSEC)
				unicastSound(ent, gi.soundindex("voice/u_shock.wav"), 1.0);							//r1,CW
		}
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_AGM)
	{
		ent->client->agm_charge = 0;
		ent->client->agm_showcharge = false;
		ent->client->agm_tripped = false;

		if (ent->client->agm_disrupt)
		{
			if (mode & WN_TEXT)
				gi_centerprintf(ent, "Cellular Disruption\n");
			if (mode & WN_VSEC)
				unicastSound(ent, gi.soundindex("voice/u_cd.wav"), 1.0);							//r1,CW
		}
		else
		{
			if (mode & WN_TEXT)
				gi_centerprintf(ent, "AG Manipulation\n");
			if (mode & WN_VSEC)
				unicastSound(ent, gi.soundindex("voice/u_agm.wav"), 1.0);							//r1,CW
		}
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_DISCLAUNCHER)
	{
		if (mode & WN_VALL)						
			unicastSound(ent, gi.soundindex("voice/u_disc.wav"), 1.0);								//r1,CW
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_CHAINSAW)
	{
		if (mode & WN_VALL)						
			unicastSound(ent, gi.soundindex("voice/u_chnsaw.wav"), 1.0);							//r1,CW

		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/chainsaw/use.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->client->pers.weapon->weapmodel == WEAP_GRAPPLE)
	{
		if (mode & WN_VALL)						
			unicastSound(ent, gi.soundindex("voice/u_grappl.wav"), 1.0);							//r1,CW
	}
//CW--
}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange(edict_t *ent)
{
//CW++
	if (!ent->client)
	{
		gi.dprintf("BUG: NoAmmoWeaponChange() called for non-client edict.\nPlease contact musashi@planetquake.com\n");
		return;
	}

	if (ent->health < 1)
	{
		gi.dprintf("BUG: NoAmmoWeaponChange() called for a dead player.\nPlease contact musashi@planetquake.com\n");
		return;
	}

	if ((ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] >= SR_CELLS_PER_SHOT)
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Shock Rifle"))])
	{
		ent->client->newweapon = FindItem("Shock Rifle");
		return;
	}
//CW--

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Railgun"))])
	{
		ent->client->newweapon = FindItem("Railgun");
		return;
	}

//CW++
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Gauss Pistol"))])
	{
		ent->client->newweapon = FindItem("Gauss Pistol");
		return;
	}
	
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))])
	{
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Rocket Launcher"))])
		{
			ent->client->newweapon = FindItem("Rocket Launcher");
			return;
		}
		else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("E.S.G."))])
		{
			ent->client->newweapon = FindItem("E.S.G.");
			return;
		}
		else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Disc Launcher"))])
		{
			ent->client->newweapon = FindItem("Disc Launcher");
			return;
		}
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Flamethrower"))])
	{
		ent->client->newweapon = FindItem("Flamethrower");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Mac-10"))])
	{
		ent->client->newweapon = FindItem("Mac-10");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Jackhammer"))])
	{
		ent->client->newweapon = FindItem("Jackhammer");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Desert Eagle"))])
	{
		ent->client->newweapon = FindItem("Desert Eagle");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("AG Manipulator"))])
	{
		ent->client->newweapon = FindItem("AG Manipulator");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("C4"))])
	{
		ent->client->newweapon = FindItem("C4");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Traps"))])
	{
		ent->client->newweapon = FindItem("Traps");
		return;
	}

	ent->client->newweapon = FindItem("Chainsaw");
//CW--
}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon(edict_t *ent)
{
	// if just died, put the weapon away
	if (ent->health < 1)
	{
		ent->client->newweapon = NULL;
		ChangeWeapon(ent);
	}

//DH++
	if (ent->flags & FL_TURRET_OWNER)
	{
		if (((ent->client->latched_buttons | ent->client->buttons) & BUTTON_ATTACK))
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			turret_breach_fire(ent->turret);
		}
		return;
	}
//DH--

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
		is_quad = (ent->client->quad_framenum > level.framenum);
		if (ent->client->silencer_shots)
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;

		ent->client->pers.weapon->weaponthink(ent);
	}
}


/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Use_Weapon(edict_t *ent, gitem_t *item)
{
	gitem_t	*ammo_item;
	int		ammo_index;

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

//CW++
	if (item->weapmodel == WEAP_GRAPPLE)
	{
		if (!(int)sv_allow_hook->value)
			return;

		if ((int)sv_hook_offhand->value)
		{
			gi_centerprintf(ent, "Cannot switch to off-hand grapple!\n");
			return;
		}

		if (level.nohook)
		{
			gi_centerprintf(ent, "The Grapple is disabled for this level.\n");
			return;
		}
	}
//CW--

	if (item->ammo && !((int)g_select_empty->value) && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
			gi_cprintf(ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
			gi_cprintf(ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent->client->newweapon = item;
}


/*
================
Drop_Weapon
================
*/
void Drop_Weapon(edict_t *ent, gitem_t *item)
{
	int	index;

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
		return;

	index = ITEM_INDEX(item);
	// see if we're already using it
	if (((item == ent->client->pers.weapon) || (item == ent->client->newweapon)) && (ent->client->pers.inventory[index] == 1))
	{
		gi_cprintf(ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	Drop_Item(ent, item);
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

static void Weapon_Generic2(edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int n;

	if (ent->deadflag || (ent->s.modelindex != (MAX_MODELS-1))) // VWep animations screw up corpses
		return;

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon(ent);
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
//CW++
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST || ((int)dmflags->value & DF_FAST_SWITCH))	
//CW--
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->newweapon && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
//CW++
		if ((int)dmflags->value & DF_FAST_SWITCH)
//CW--
		{
			if (ent->client->pers.weapon->weapmodel == WEAP_CHAINSAW)
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/chainsaw/done.wav"), 1, ATTN_NORM, 0);

			ChangeWeapon(ent);
			return;
		}
		else
			ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

		if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
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
		if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK))
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if ((!ent->client->ammo_index) || (ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
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
					ent->pain_debounce_time = level.time + 1.0;
				}
				NoAmmoWeaponChange(ent);
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
//ZOID++
				if (!CTFApplyStrengthSound(ent))
				{
//ZOID--
					if (ent->client->quad_framenum > level.framenum)
					{
//CW++
						if ((ent->client->pers.weapon->weapmodel == WEAP_AGM) && !ent->client->agm_disrupt)
							;
						else
//CW--
							gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
					}
				}
//ZOID++
				CTFApplyHasteSound(ent);
//ZOID--
				fire(ent);
				break;
			}
		}

//CW++
//		Reload the Desert Eagle if required (a clip holds 9 bullets).

		if (ent->client->pers.weapon->weapmodel == WEAP_DESERTEAGLE)
		{
			if ((ent->client->machinegun_shots > 8) && (ent->client->ps.gunframe == 8))
			{
				if (level.time >= ent->pain_debounce_time)
					ent->pain_debounce_time = level.time + 1.0;

				ent->client->machinegun_shots = 0;
				if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
					NoAmmoWeaponChange(ent);
				else
					ent->client->newweapon = FindItem("Desert Eagle");
			}
		}

//		Reload the Jackhammer if required (a clip holds 10 shells).

		else if (ent->client->pers.weapon->weapmodel == WEAP_JACKHAMMER)
		{
			if ((ent->client->machinegun_shots > 9) && (ent->client->ps.gunframe == 5))
			{
				if (level.time >= ent->pain_debounce_time)
					ent->pain_debounce_time = level.time + 1.0;

				ent->client->machinegun_shots = 0;
				if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
					NoAmmoWeaponChange(ent);
				else
					ent->client->newweapon = FindItem("Jackhammer");
			}
		}

//		Reload the Mac-10 if required (a clip holds 32 bullets).

		else if (ent->client->pers.weapon->weapmodel == WEAP_MAC10)
		{
			if ((ent->client->machinegun_shots > 15) && (ent->client->ps.gunframe == 3))
			{
				ent->client->ps.gunframe = 5;
				if (level.time >= ent->pain_debounce_time)
					ent->pain_debounce_time = level.time + 1.0;

				ent->client->machinegun_shots = 0;
				if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
					NoAmmoWeaponChange(ent);
				else
					ent->client->newweapon = FindItem("Mac-10");
			}
		}
//CW--

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	}
}

//ZOID++
void Weapon_Generic(edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int oldstate = ent->client->weaponstate;

	if (ent->client->frozen_framenum > level.framenum)
		return;

	Weapon_Generic2(ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, fire_frames, fire);

//	Run the weapon frame again if hasted, unless the weapon is the Grapple or AGM.

	if ((ent->client->pers.weapon->weapmodel == WEAP_GRAPPLE) && (ent->client->weaponstate == WEAPON_FIRING))
		return;

//CW++
	if ((ent->client->pers.weapon->weapmodel == WEAP_AGM) && (ent->client->weaponstate == WEAPON_FIRING))
		return;
//CW--

//	If the player has it/them, apply the Haste Tech effect...

	if ((CTFApplyHaste(ent) ||
		((ent->client->pers.weapon->weapmodel == WEAP_GRAPPLE) && (ent->client->weaponstate != WEAPON_FIRING)))
		&& (oldstate == ent->client->weaponstate))
	{
		Weapon_Generic2(ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, fire_frames, fire);
	}

//CW++
//	... or the Haste powerup effect (not both together).

	else if ((ent->client->haste_framenum > level.framenum) && (oldstate == ent->client->weaponstate))
		Weapon_Generic2(ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, fire_frames, fire);
//CW--
}
//ZOID--


/*
======================================================================
ROCKET LAUNCHER
======================================================================
*/
void Weapon_RocketLauncher_Fire(edict_t *self)
{
	vec3_t		offset;
	vec3_t		start;
	vec3_t		forward;
	vec3_t		right;
	int			damage;
	int			radius_damage;

//CW++
	qboolean	guided;
//CW--

//	Set damage values.

	damage = (int)(sv_rocket_damage->value + (random() * 20.0));									//CW
	radius_damage = (int)sv_rocket_radius_damage->value;											//CW
	if (is_quad)
	{
		damage *= (int)sv_quad_factor->value;														//CW
		radius_damage *= (int)sv_quad_factor->value;												//CW
	}

//	Set projectile start position and weapon kick info.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorScale(forward, -2.0, self->client->kick_origin);
	self->client->kick_angles[0] = -1.0;

	VectorSet(offset, 8.0, 8.0, self->viewheight-8.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);

//	Fire!

//CW++
	guided = (self->client->normal_rockets)?false:true;
//CW--

	Fire_Rocket(self, start, forward, damage, sv_rocket_speed->value, sv_rocket_radius->value, radius_damage, guided);	//CW
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self-g_edicts);
	gi.WriteByte(MZ_ROCKET | is_silenced);
	gi.multicast(self->s.origin, MULTICAST_PVS);
	self->client->ps.gunframe++;
	PlayerNoise(self, start, PNOISE_WEAPON);														//CW

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		self->client->pers.inventory[self->client->ammo_index]--;
}

void Weapon_RocketLauncher(edict_t *self)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic(self, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}


/*
======================================================================
RAILGUN
======================================================================
*/
void Weapon_Railgun_Fire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	offset;
	int		damage;
	int		kick = 200;
	int			red=20, green=48, blue=176;
	qboolean	useColor=false;

//	Set damage and kick values.

	damage = (int)sv_railgun_damage->value;															//CW
	if (is_quad)
	{
		damage *= (int)sv_quad_factor->value;
		kick *= (int)sv_quad_factor->value;
	}

	// Knightmare- custom client color
	if ( self->client && (self->client->pers.color1[3] != 0) )
	{
		useColor = true;
		red = self->client->pers.color1[0];
		green = self->client->pers.color1[1];
		blue = self->client->pers.color1[2];
	}

//	Set projectile start position and weapon kick info.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorScale(forward, -3.0, self->client->kick_origin);
	self->client->kick_angles[0] = -3.0;

	VectorSet(offset, 0.0, 7.0, self->viewheight-8.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);

//	Fire!

	Fire_Rail (self, start, forward, damage, kick, useColor, red, green, blue);
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self-g_edicts);
	gi.WriteByte(MZ_RAILGUN | is_silenced);
	gi.multicast(self->s.origin, MULTICAST_PVS);
	self->client->ps.gunframe++;
	PlayerNoise(self, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		self->client->pers.inventory[self->client->ammo_index]--;
}

void Weapon_Railgun(edict_t *self)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic(self, 3, 18, 56, 61, pause_frames, fire_frames, Weapon_Railgun_Fire);
}


//CW++
// Awakening weapons.
/*
======================================================================
DESERT EAGLE
======================================================================
*/
void Weapon_DesertEagle_Fire(edict_t *self)
{
	trace_t	tr;
	vec3_t	start;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	offset;
	vec3_t	t_start;
	int		damage;
	int		kick = 2;

//	Set damage and kick values. There is a 20% chance of doing extra damage.

	damage = (int)sv_deserteagle_damage->value + ((random()<0.20)?15:0);
	if (is_quad)
	{
		damage *= (int)sv_quad_factor->value;
		kick *= (int)sv_quad_factor->value;
	}

//	Set projectile start position and weapon kick info.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0.0, 0.0, self->viewheight);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);
	
	VectorScale(forward, -2.0, self->client->kick_origin);
	self->client->kick_angles[0] = -1.0;
	
//	Fire!

	Fire_Bullet(self, start, forward, damage, kick, (int)sv_deserteagle_hspread->value, (int)sv_deserteagle_vspread->value, MOD_DESERTEAGLE);
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self-g_edicts);
	gi.WriteByte(MZ_BLASTER | is_silenced);
	gi.multicast(self->s.origin, MULTICAST_PVS);
	self->client->ps.gunframe++;
	self->client->machinegun_shots++;
	PlayerNoise(self, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		self->client->pers.inventory[self->client->ammo_index]--;

//	Spawn a tracer sometimes (50% chance, if the path is clear).

	VectorMA(start, 200.0, forward, t_start);
	tr = gi.trace(start, NULL, NULL, t_start, self, MASK_SHOT);
	if ((tr.fraction == 1.0) && (random() < 0.5))
		Fire_Tracer(self, t_start, forward, 2000.0, 0.2);
}

void Weapon_DesertEagle(edict_t *self)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic(self, 5, 8, 52, 55, pause_frames, fire_frames, Weapon_DesertEagle_Fire);
}


/*
======================================================================
JACKHAMMER
======================================================================
*/
void Weapon_Jackhammer_Fire(edict_t *self)
{
	vec3_t	start;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	offset;
	int		damage;
	int		kick = 8;

//	Set damage and kick values.

	damage = (int)sv_jackhammer_damage->value;
	if (is_quad)
	{
		damage *= (int)sv_quad_factor->value;
		kick *= (int)sv_quad_factor->value;
	}

//	Set projectile start position and weapon kick info.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0.0, 8.0,  self->viewheight-8.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);

	VectorScale(forward, -2.0, self->client->kick_origin);
	self->client->kick_angles[0] = -2.0;

//	Fire!

	Fire_Shotgun(self, start, forward, damage, kick, (int)sv_jackhammer_hspread->value, (int)sv_jackhammer_vspread->value, DEFAULT_JACKHAMMER_COUNT, MOD_JACKHAMMER);
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self-g_edicts);
	gi.WriteByte(MZ_SSHOTGUN | is_silenced);
	gi.multicast(self->s.origin, MULTICAST_PVS);
	self->client->ps.gunframe++;
	self->client->machinegun_shots++;
	PlayerNoise(self, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		self->client->pers.inventory[self->client->ammo_index]--;
}

void Weapon_Jackhammer(edict_t *self)
{
	static int	pause_frames[]	= {45, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic(self, 3, 5, 45, 49, pause_frames, fire_frames, Weapon_Jackhammer_Fire);
}


/*
======================================================================
MAC-10
======================================================================
*/
void Weapon_Mac10_Fire(edict_t *self)
{
	vec3_t	start;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	offset;
	vec3_t	t_start;
	trace_t	tr;
	int		damage;
	int		kick = 2;
	int		i;
	
//	Set damage and kick values.

	damage = (int)sv_mac10_damage->value;
	if (is_quad)
	{
		damage *= (int)sv_quad_factor->value;
		kick *= (int)sv_quad_factor->value;
	}
		
//	Set projectile start position and weapon kick info.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0.0, 0.0, self->viewheight);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);
	
	VectorScale(forward, -2.0, self->client->kick_origin);
	self->client->kick_angles[0] = -1.0;
	for (i = 1; i < 3; ++i)
		self->client->kick_angles[i] = (random() < 0.5)?-1.0:1.0;

//	Fire! (two bullets per trigger press).

	for (i = 0; i < 2; ++i)
	{
		Fire_Bullet(self, start, forward, damage, kick, (int)sv_mac10_hspread->value, (int)sv_mac10_vspread->value, MOD_MAC10);

		if (!((int)dmflags->value & DF_INFINITE_AMMO))
			self->client->pers.inventory[self->client->ammo_index]--;

//		Check if there's enough ammo; swap weapons if we're dry.

		if (self->client->pers.inventory[self->client->ammo_index] < 1)
		{
			self->client->ps.gunframe = 5;
			if (level.time >= self->pain_debounce_time)
			{
				gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				self->pain_debounce_time = level.time + 1.0;
			}
			self->client->machinegun_shots = 0;
			NoAmmoWeaponChange(self);
			return;
		}
	}
	
	self->client->ps.gunframe++;
	self->client->machinegun_shots++;

//	Spawn a tracer every 4 shots.

	if ((self->client->machinegun_shots % 4) == 0)
	{
		VectorSet(offset, 24.0, 7.0, self->viewheight-6.0);
		P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);
		VectorMA(start, 300.0, forward, t_start);
		tr = gi.trace(start, NULL, NULL, t_start, self, MASK_SHOT);
		if (tr.fraction == 1.0)
			Fire_Tracer(self, t_start, forward, 2000.0, 0.3);
	}

//	Loop the animation sequence if the firing button is still being pressed.

	if ((self->client->ps.gunframe == 5) && (self->client->buttons & BUTTON_ATTACK))
		self->client->ps.gunframe = 3;

//	Do the muzzleflash and sound.

	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self-g_edicts);
	gi.WriteByte(MZ_MACHINEGUN | is_silenced);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	PlayerNoise(self, start, PNOISE_WEAPON);
}

void Weapon_Mac10(edict_t *self)
{
	static int	pause_frames[]	= {29, 45, 0};
	static int	fire_frames[]	= {3, 4, 0};

	Weapon_Generic(self, 2, 4, 45, 49, pause_frames, fire_frames, Weapon_Mac10_Fire);
}


/*
======================================================================
C4
======================================================================
*/
void Weapon_C4_Fire(edict_t *self, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	start;
	float	timer;
	float	radius;
	int		damage;
	int		speed;

//	Set damage values.

	damage = (int)sv_c4_damage->value;
	radius = sv_c4_radius->value;
	if (is_quad)
		damage *= (int)sv_quad_factor->value;

//	Set projectile start position.

	VectorSet(offset, 8.0, 8.0, self->viewheight);
	AngleVectors(self->client->v_angle, forward, right, NULL);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);

//	Determine throwing speed.

	timer = level.time - self->client->grenade_time;
	if (CTFApplyHaste(self) || (self->client->haste_framenum > level.framenum))
	{
		timer *= 2.0;
		CTFApplyHasteSound(self);
	}
	speed = (int)(min(sv_c4_min_speed->value + (timer * sv_c4_hold_accel->value), sv_c4_max_speed->value));

//	Fire!

	Fire_C4(self, start, forward, damage, speed, radius, held);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
	
	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		self->client->pers.inventory[self->client->ammo_index]--;

//	Handle player model throwing animation.

	if (self->deadflag || (self->s.modelindex != (MAX_MODELS-1)))		// VWep animations screw up corpses
		return;

	if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		self->client->anim_priority = ANIM_ATTACK;
		self->s.frame = FRAME_crattak1-1;
		self->client->anim_end = FRAME_crattak3;
	}
	else
	{
		self->client->anim_priority = ANIM_REVERSE;
		self->s.frame = FRAME_wave08;
		self->client->anim_end = FRAME_wave01;
	}
}

void Weapon_C4(edict_t *self)
{
	int	remaining;

	if ((self->client->newweapon) && (self->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon(self);
		return;
	}

	if (self->client->weaponstate == WEAPON_ACTIVATING)
	{
		self->client->weaponstate = WEAPON_READY;
		self->client->ps.gunframe = 16;
		return;
	}

	if (self->client->weaponstate == WEAPON_READY)
	{
		if (((self->client->latched_buttons | self->client->buttons) & BUTTON_ATTACK))
		{
			self->client->latched_buttons &= ~BUTTON_ATTACK;
			if (self->client->pers.inventory[self->client->ammo_index])
			{
				self->client->ps.gunframe = 1;
				self->client->weaponstate = WEAPON_FIRING;
				self->client->grenade_time = 0.0;
				self->client->c4_boom_time = 0.0;
			}
			else
			{
				if (level.time >= self->pain_debounce_time)
				{
					gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					self->pain_debounce_time = level.time + 1.0;
				}
				NoAmmoWeaponChange(self);
			}
			return;
		}

		if ((self->client->ps.gunframe == 29) || (self->client->ps.gunframe == 34) || (self->client->ps.gunframe == 39) || (self->client->ps.gunframe == 48))
		{
			if (rand() & 15)
				return;
		}

		if (++self->client->ps.gunframe > 48)
			self->client->ps.gunframe = 16;
		return;
	}

	if (self->client->weaponstate == WEAPON_FIRING)
	{
		if (self->client->ps.gunframe == 5)
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/c4/arm.wav"), 1, ATTN_NORM, 0);

		if (self->client->ps.gunframe == 11)
		{
			if (!self->client->grenade_time)
				self->client->grenade_time = level.time;

			if (sv_c4_timelimit->value > 0.0)		// auto-detonation 2-second warning sound
			{
				if (!self->client->c4_boom_time)
					self->client->c4_boom_time = level.time + sv_c4_timelimit->value;

				remaining = (int)(self->client->c4_boom_time - level.time);
				if ((remaining == 1) || (remaining == 2))
					self->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

//			If player has waited too long, detonate it in their hand.

			if (!self->client->grenade_blew_up && (sv_c4_timelimit->value > 0.0) && (level.time >= self->client->c4_boom_time))
			{
				self->client->weapon_sound = 0;
				Weapon_C4_Fire(self, true);
				self->client->grenade_blew_up = true;
			}

			if (self->client->buttons & BUTTON_ATTACK)
				return;

			if (self->client->grenade_blew_up)
			{
				if (level.time >= self->client->c4_boom_time)
				{
					self->client->ps.gunframe = 15;
					self->client->grenade_blew_up = false;
				}
				else
					return;
			}
		}

		if (self->client->ps.gunframe == 12)
		{
			self->client->weapon_sound = 0;
			Weapon_C4_Fire(self, false);
		}

		self->client->ps.gunframe++;

		if (self->client->ps.gunframe == 16)
		{
			self->client->grenade_time = 0.0;
			self->client->weaponstate = WEAPON_READY;
		}
	}
}


/*
======================================================================
TRAPS
======================================================================
*/
void Weapon_Trap_Fire(edict_t *self, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	start;
	float	timer;
	int		damage_hook;
	int		damage_beam;
	int		speed;

//	Set damage values.

	damage_hook = (int)sv_trap_hook_damage->value;
	damage_beam = (int)sv_trap_beam_damage->value;
	if (is_quad)
	{
		damage_hook *= (int)sv_quad_factor->value;
		damage_beam *= (int)sv_quad_factor->value;
	}

//	Set projectile start position.

	VectorSet(offset, 8.0, 8.0, self->viewheight);
	AngleVectors(self->client->v_angle, forward, right, NULL);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);

//	Determine throwing speed.

	timer = level.time - self->client->grenade_time;
	if (CTFApplyHaste(self) || (self->client->haste_framenum > level.framenum))
	{
		timer *= 2.0;
		CTFApplyHasteSound(self);
	}
	speed = (int)(min(sv_trap_min_speed->value + (timer * sv_trap_hold_accel->value), sv_trap_max_speed->value));

//	Fire!

	Fire_Trap(self, start, forward, damage_hook, speed, damage_beam, (int)sv_trap_beam_power->value, held);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
	
	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		self->client->pers.inventory[self->client->ammo_index]--;

//	Handle player model throwing animation.

	if (self->deadflag || (self->s.modelindex != (MAX_MODELS-1)))		// VWep animations screw up corpses
		return;

	if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		self->client->anim_priority = ANIM_ATTACK;
		self->s.frame = FRAME_crattak1-1;
		self->client->anim_end = FRAME_crattak3;
	}
	else
	{
		self->client->anim_priority = ANIM_REVERSE;
		self->s.frame = FRAME_wave08;
		self->client->anim_end = FRAME_wave01;
	}
}

void Weapon_Trap(edict_t *self)
{
	int remaining;

	if ((self->client->newweapon) && (self->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon(self);
		return;
	}

	if (self->client->weaponstate == WEAPON_ACTIVATING)
	{
		self->client->weaponstate = WEAPON_READY;
		self->client->ps.gunframe = 16;
		return;
	}

	if (self->client->weaponstate == WEAPON_READY)
	{
		if (((self->client->latched_buttons | self->client->buttons) & BUTTON_ATTACK))
		{
			self->client->latched_buttons &= ~BUTTON_ATTACK;
			if (self->client->pers.inventory[self->client->ammo_index])
			{
				self->client->ps.gunframe = 1;
				self->client->weaponstate = WEAPON_FIRING;
				self->client->grenade_time = 0.0;
				self->client->c4_boom_time = 0.0;
			}
			else
			{
				if (level.time >= self->pain_debounce_time)
				{
					gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					self->pain_debounce_time = level.time + 1.0;
				}
				NoAmmoWeaponChange(self);
			}
			return;
		}

		if ((self->client->ps.gunframe == 29) || (self->client->ps.gunframe == 34) || (self->client->ps.gunframe == 39) || (self->client->ps.gunframe == 48))
		{
			if (rand() & 15)
				return;
		}

		if (++self->client->ps.gunframe > 48)
			self->client->ps.gunframe = 16;
		return;
	}

	if (self->client->weaponstate == WEAPON_FIRING)
	{
		if (self->client->ps.gunframe == 5)
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/trap/arm.wav"), 1, ATTN_NORM, 0);

		if (self->client->ps.gunframe == 11)
		{
			if (!self->client->grenade_time)
				self->client->grenade_time = level.time;

			if (sv_traps_timelimit->value > 0.0)		// auto-trapping 2-second warning sound
			{
				if (!self->client->c4_boom_time)
					self->client->c4_boom_time = level.time + sv_traps_timelimit->value;

				remaining = (int)(self->client->c4_boom_time - level.time);
				if ((remaining == 1) || (remaining == 2))
					self->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

//			If player has waited too long, set the Trap off in their hand.

			if (!self->client->grenade_blew_up && (sv_traps_timelimit->value > 0.0) && (level.time >= self->client->c4_boom_time))
			{
				self->client->weapon_sound = 0;
				Weapon_Trap_Fire(self, true);
				self->client->grenade_blew_up = true;
			}

			if (self->client->buttons & BUTTON_ATTACK)
				return;

			if (self->client->grenade_blew_up)
			{
				if (level.time >= self->client->c4_boom_time)
				{
					self->client->ps.gunframe = 15;
					self->client->grenade_blew_up = false;
				}
				else
					return;
			}
		}

		if (self->client->ps.gunframe == 12)
		{
			self->client->weapon_sound = 0;
			Weapon_Trap_Fire(self, false);
		}

		self->client->ps.gunframe++;

		if (self->client->ps.gunframe == 16)
		{
			self->client->grenade_time = 0.0;
			self->client->weaponstate = WEAPON_READY;
		}
	}
}


/*
======================================================================
EXPLOSIVE SPIKE GUN
======================================================================
*/
void Weapon_ESG_Fire(edict_t *self)
{
	vec3_t	start;
	vec3_t	start_p;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	offset;
	vec3_t	dir;
	vec3_t	end;
	trace_t tr;
	int		damage;
	int		radius_damage;
	int		kick = 2;

//	Fire up to 3 spikes quickly if the firing button is pressed, then reload (via viewmodel animation).

	if (self->client->ps.gunframe == 14)
	{
		if ((self->client->buttons & BUTTON_ATTACK) && (self->client->machinegun_shots < 3))
			self->client->ps.gunframe = 11;
		else
		{
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/esg/reload.wav"), 1, ATTN_NORM, 0);
			self->client->machinegun_shots = 0;
			self->client->ps.gunframe++;
			return;
		}
	}

	if (self->client->ps.gunframe == 24)
	{
		if (self->client->buttons & BUTTON_ATTACK)
			self->client->ps.gunframe = 11;
		else
		{
			self->client->ps.gunframe++;
			return;
		}
	}

//	Set damage and kick values.

	damage = (int)sv_spike_damage->value;
	radius_damage = (int)sv_spike_bang_damage->value;
	if (is_quad)
	{
		damage *= (int)sv_quad_factor->value;
		radius_damage *= (int)sv_quad_factor->value;
		kick *= (int)sv_quad_factor->value;
	}

//	Set projectile start position and weapon kick info.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8.0, 8.0, self->viewheight-8.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);

	VectorScale(forward, -2.0, self->client->kick_origin);
	self->client->kick_angles[0] = -2.0;

//	Set projectile end position and flight direction.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorMA(self->s.origin, WORLD_SIZE, forward, end);	// was 8192.0
	VectorSet(offset, 0.0, 0.0, self->viewheight-2.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start_p);
	tr = gi.trace(start_p, NULL, NULL, end, self, MASK_SHOT);
	VectorSubtract(tr.endpos, start, dir);
	VectorNormalize(dir);

//	Fire!

	Fire_Spike(self, start, dir, damage, sv_spike_speed->value, kick, sv_spike_bang_radius->value, radius_damage);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/esg/fire.wav"), (is_silenced)?0.2:1, ATTN_NORM, 0);
	PlayerNoise(self, start, PNOISE_WEAPON);
	self->client->ps.gunframe++;
	self->client->machinegun_shots++;

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		self->client->pers.inventory[self->client->ammo_index]--;

//	Check if there's enough ammo; swap weapons if we're dry.

	if (self->client->pers.inventory[self->client->ammo_index] < 1)
	{
		self->client->ps.gunframe = 29;
		if (level.time >= self->pain_debounce_time)
		{
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			self->pain_debounce_time = level.time + 1.0;
		}
		self->client->machinegun_shots = 0;
		NoAmmoWeaponChange(self);
	}
}

void Fire_MultiSpikes(edict_t *self, vec3_t start, vec3_t aimdir, int damage, float speed, int kick, float damage_radius, int radius_damage, int count)
{
	vec3_t	dir;
	vec3_t	newdir;
	vec3_t	end;
	vec3_t	forward;
	vec3_t	right;
	int		i;

	switch (count)
	{
		case 0:
		case 1:
			Fire_Spike(self, start, aimdir, damage, speed, kick, damage_radius, radius_damage);
			break;

		case 2:
			vectoangles(aimdir, dir);
			AngleVectors(dir, forward, right, NULL);
			for (i = -1; i < 2; i += 2)
			{
				VectorMA(start, WORLD_SIZE, forward, end);	// was 8192.0
				VectorMA(end, i * SPIKE_SPREAD_HALF, right, end);
				VectorSubtract(end, start, newdir);
				VectorNormalize(newdir);
				Fire_Spike(self, start, newdir, damage, speed, kick, damage_radius, radius_damage);
			}
			break;

		case 3:
			vectoangles(aimdir, dir);
			AngleVectors(dir, forward, right, NULL);
			for (i = -1; i < 2; i++)
			{
				VectorMA(start, WORLD_SIZE, forward, end);	// was 8192.0
				VectorMA(end, i * SPIKE_SPREAD, right, end);
				VectorSubtract(end, start, newdir);
				VectorNormalize(newdir);
				Fire_Spike(self, start, newdir, damage, speed, kick, damage_radius, radius_damage);
			}
			break;

		case 4:
			vectoangles(aimdir, dir);
			AngleVectors(dir, forward, right, NULL);
			for (i = -3; i < 4; i += 2)
			{
				VectorMA(start, WORLD_SIZE, forward, end);	// was 8192.0
				VectorMA(end, i * SPIKE_SPREAD_HALF, right, end);
				VectorSubtract(end, start, newdir);
				VectorNormalize(newdir);
				Fire_Spike(self, start, newdir, damage, speed, kick, damage_radius, radius_damage);
			}
			break;

		case 5:
			vectoangles(aimdir, dir);
			AngleVectors(dir, forward, right, NULL);
			for (i = -2; i < 3; i++)
			{
				VectorMA(start, WORLD_SIZE, forward, end);	// was 8192.0
				VectorMA(end, i * SPIKE_SPREAD, right, end);
				VectorSubtract(end, start, newdir);
				VectorNormalize(newdir);
				Fire_Spike(self, start, newdir, damage, speed, kick, damage_radius, radius_damage);
			}
			break;

		case 6:
			vectoangles(aimdir, dir);
			AngleVectors(dir, forward, right, NULL);
			for (i = -5; i < 6; i += 2)
			{
				VectorMA(start, WORLD_SIZE, forward, end);	// was 8192.0
				VectorMA(end, i * SPIKE_SPREAD_HALF, right, end);
				VectorSubtract(end, start, newdir);
				VectorNormalize(newdir);
				Fire_Spike(self, start, newdir, damage, speed, kick, damage_radius, radius_damage);
			}
			break;

		default:
			gi.dprintf("BUG: Fire_MultiSpikes() called for > 6 spikes.\nPlease contact musashi@planetquake.com\n");
			return;
	}
}

void Weapon_ESG_MultiFire(edict_t *self)
{
	vec3_t	start;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	offset;
	int		damage;
	int		radius_damage;
	int		kick = 2;

//	If we ran out of ammo during the previous function call, fire all the loaded spikes.

	if (self->client->pers.inventory[self->client->ammo_index] == 0)
		goto fire_now;

//	As long as the firing button is held down, keep building up the store of spikes. When there are
//	six spikes in the chamber, or the firing button has been released, or we're about to run out 
//	of ammo, fire the spikes together.

	if (self->client->ps.gunframe == 11)		// start of weapon kick animation
	{
		if (self->client->buttons & BUTTON_ATTACK)
		{
			if (!((int)dmflags->value & DF_INFINITE_AMMO))
				self->client->pers.inventory[self->client->ammo_index]--;

			if (self->client->machinegun_shots == -1)		// reset the "recently fired" flag
				self->client->machinegun_shots = 0;

			self->client->machinegun_shots++;
			if (self->client->machinegun_shots < 6)
			{
				self->client->ps.gunframe = 15;
					gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/esg/reload.wav"), 1, ATTN_NORM, 0);

				if (self->client->pers.inventory[self->client->ammo_index] >= 0)
					return;
			}
		}
		else
		{
			if (self->client->machinegun_shots == -1)		// have fired recently, so don't fire another spike yet
				return;
		}
	}

	if (self->client->ps.gunframe == 14)		// end of weapon kick animation
	{
		if (self->client->buttons & BUTTON_ATTACK)
			self->client->ps.gunframe = 11;
		else
			self->client->ps.gunframe = 24;

		return;
	}

	if (self->client->ps.gunframe == 22)		// near end of reload animation
	{
		if ((self->client->buttons & BUTTON_ATTACK) && (self->client->machinegun_shots < 6))
		{
			self->client->ps.gunframe = 11;
			return;
		}
	}
	
//	Set damage and kick values.

fire_now:
	damage = (int)sv_spike_damage->value;
	radius_damage = (int)sv_spike_bang_damage->value;
	if (is_quad)
	{
		damage *= (int)sv_quad_factor->value;
		radius_damage *= (int)sv_quad_factor->value;
		kick *= (int)sv_quad_factor->value;
	}

//	Set projectile start position and weapon kick info.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8.0, 8.0, self->viewheight-8.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);

	VectorScale(forward, -2.0, self->client->kick_origin);
	self->client->kick_angles[0] = -2.0;

//	Fire!

	Fire_MultiSpikes(self, start, forward, damage, sv_spike_speed->value, kick, sv_spike_bang_radius->value, radius_damage, self->client->machinegun_shots);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/esg/fire.wav"), (is_silenced)?0.2:1, ATTN_NORM, 0);
	PlayerNoise(self, start, PNOISE_WEAPON);
	self->client->ps.gunframe = 12;

	if (self->client->machinegun_shots == 0)	// was a quick fire-button press, so ammo not depleted yet (see above)
		self->client->pers.inventory[self->client->ammo_index]--;

	self->client->machinegun_shots = -1;		// use as a "recently fired" flag

//	Check if there's enough ammo; swap weapons if we're dry.

	if (self->client->pers.inventory[self->client->ammo_index] < 1)
	{
		self->client->ps.gunframe = 29;
		if (level.time >= self->pain_debounce_time)
		{
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			self->pain_debounce_time = level.time + 1.0;
		}
		self->client->machinegun_shots = 0;
		NoAmmoWeaponChange(self);
	}
}

void Weapon_ESG(edict_t *self)
{
	static int	pause_frames[]	= {30, 40, 0};
	static int	fire_frames[]	= {11, 14, 24, 0};
	static int	fire_frames_multi[]	= {11, 14, 22, 0};

	if (self->client->multi_spike)
	{
		if ((self->client->ps.gunframe == 28) && (self->client->machinegun_shots == -1))
			self->client->machinegun_shots = 0;		// reset the "recently fired" flag

		Weapon_Generic(self, 7, 28, 40, 44, pause_frames, fire_frames_multi, Weapon_ESG_MultiFire);
		if (!(self->client->buttons & BUTTON_ATTACK) && (self->client->machinegun_shots > 0))
			Weapon_ESG_MultiFire(self);
	}
	else
		Weapon_Generic(self, 7, 28, 40, 44, pause_frames, fire_frames, Weapon_ESG_Fire);
}


/*
======================================================================
FLAMETHROWER
======================================================================
*/
void Weapon_Flamethrower_Fire(edict_t *self)
{
	vec3_t	forward;
	vec3_t	right;
	vec3_t	start;
	vec3_t	offset;
	int		damage;
	int		damage_minor;

//	Check if there's enough ammo; swap weapons if we're dry.

	if (self->client->pers.inventory[self->client->ammo_index] < 1)
	{
		self->client->ps.gunframe = 9;
		if (level.time >= self->pain_debounce_time)
		{
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			self->pain_debounce_time = level.time + 1.0;
		}
		NoAmmoWeaponChange(self);
		return;
	}

//	Set projectile/puff start position.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 20.0, 8.0, self->viewheight-8.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);

//	Flamethrower doesn't work underwater.

	if (self->waterlevel < 2)
	{

//		Set damage value.

		damage = (int)sv_flame_damage->value;
		damage_minor = (int)sv_flame_small_damage->value;
		if (is_quad)
		{
			damage *= (int)sv_quad_factor->value;
			damage_minor *= (int)sv_quad_factor->value;
		}

//		Fire!

		if (self->client->ps.gunframe == 5)
			Fire_Fireball(self, start, forward, damage, damage_minor, sv_flame_speed->value, true);
		else
			Fire_Fireball(self, start, forward, damage, damage_minor, sv_flame_speed->value, false);

		if (self->client->ps.gunframe % 2)
		{
			gi.WriteByte(svc_muzzleflash);
			gi.WriteShort(self-g_edicts);
			gi.WriteByte(MZ_HYPERBLASTER | is_silenced);
			gi.multicast(self->s.origin, MULTICAST_PVS);

			PlayerNoise(self, start, PNOISE_WEAPON);
		}
	}
	else
	{
		VectorSet(forward, 0.0, 0.0, 1.0);
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_HEATBEAM_SPARKS);
		gi.WritePosition(start);
		gi.WriteDir(forward);
		gi.multicast(start, MULTICAST_PVS);

		if (self->client->ps.gunframe == 5)
			gi.sound(self, CHAN_WEAPON, gi.soundindex("misc/lasfly.wav"), 1, ATTN_NORM, 0);
	}

//	Advance animation frame, and remove 0.1 second delay between 4-flame bursts when holding down the fire button.

	self->client->ps.gunframe++;
	if ((self->client->ps.gunframe == 9) && (self->client->buttons & BUTTON_ATTACK))
		self->client->ps.gunframe = 5;

//	Spend ammo.

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		self->client->pers.inventory[self->client->ammo_index]--;
}

void Weapon_Flamethrower_FB_Fire(edict_t *self)
{
	vec3_t	forward;
	vec3_t	right;
	vec3_t	start;
	vec3_t	offset;
	int		damage;
	int		damage_minor;
	int		kick = 2;

//	We want the rate-of-fire to be 1 shot per second.

	if (self->last_move_time > level.time)
	{
		self->client->ps.gunframe = 5;
		return;
	}

//	Check if there's enough ammo; swap weapons if we're dry.

	if (self->client->pers.inventory[self->client->ammo_index] < FT_CELLS_PER_SHOT)
	{
		self->client->ps.gunframe = 9;
		if (level.time >= self->pain_debounce_time)
		{
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			self->pain_debounce_time = level.time + 1.0;
		}
		NoAmmoWeaponChange(self);
		return;
	}

//	Set projectile/puff start position.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 20.0, 8.0, self->viewheight-8.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);

//	Flamethrower doesn't work underwater.

	if (self->waterlevel < 2)
	{

//		Set damage value.

		damage = (int)sv_firebomb_damage->value;
		damage_minor = (int)sv_flame_damage->value;
		if (is_quad)
		{
			damage *= (int)sv_quad_factor->value;
			damage_minor *= (int)sv_quad_factor->value;
			kick *= (int)sv_quad_factor->value;
		}

//		Fire!

		Fire_Firebomb(self, start, forward, damage, damage_minor, kick, sv_firebomb_radius->value, sv_firebomb_speed->value);
		self->last_move_time = level.time + 1.0;

		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(self-g_edicts);
		gi.WriteByte(MZ_HYPERBLASTER | is_silenced);
		gi.multicast(self->s.origin, MULTICAST_PVS);

		PlayerNoise(self, start, PNOISE_WEAPON);
	}
	else
	{
		VectorSet(forward, 0.0, 0.0, 1.0);
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_HEATBEAM_SPARKS);
		gi.WritePosition(start);
		gi.WriteDir(forward);
		gi.multicast(start, MULTICAST_PVS);

		if (self->client->ps.gunframe == 5)
			gi.sound(self, CHAN_WEAPON, gi.soundindex("misc/lasfly.wav"), 1, ATTN_NORM, 0);
	}

	self->client->ps.gunframe++;

//	Spend ammo.

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		self->client->pers.inventory[self->client->ammo_index] -= FT_CELLS_PER_SHOT;
}

void Weapon_Flamethrower(edict_t *self)
{
	static int	pause_frames[]	= {49, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 0};
	static int	fire_frames_fb[] = {5, 0};

	if (self->client->ft_firebomb)
		Weapon_Generic(self, 4, 8, 49, 53, pause_frames, fire_frames_fb, Weapon_Flamethrower_FB_Fire);
	else
		Weapon_Generic(self, 4, 8, 49, 53, pause_frames, fire_frames, Weapon_Flamethrower_Fire);
}


/*
======================================================================
SHOCK RIFLE
======================================================================
*/
void Weapon_ShockRifle_Fire(edict_t *self)
{
	vec3_t	start;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	offset;
	float	speed;
	int		damage_plasma;
	int		damage_shockbolt;
	int		kick = 2;

//	Check if there's enough ammo; swap weapons if we're dry.

	if (self->client->pers.inventory[self->client->ammo_index] < SR_CELLS_PER_SHOT)
	{
		self->client->ps.gunframe = 16;
		if (level.time >= self->pain_debounce_time)
		{
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			self->pain_debounce_time = level.time + 1.0;
		}
		NoAmmoWeaponChange(self);
		return;
	}

//	Set damage, kick and speed values.

	damage_plasma = (int)sv_shock_homing_damage->value;
	damage_shockbolt = (int)sv_shock_radius_damage->value;
	if (is_quad)
	{
		damage_plasma *= (int)sv_quad_factor->value;
		damage_shockbolt *= (int)sv_quad_factor->value;
		kick *= (int)sv_quad_factor->value;
	}

	speed = (self->client->homing_plasma)? sv_shock_homing_speed->value : sv_shock_speed->value;

//	Set projectile start position and weapon kick info.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8.0, 8.0, self->viewheight-8.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);
	
	VectorScale(forward, -2.0, self->client->kick_origin);
	self->client->kick_angles[0] = -1.0;
	
//	Fire!

	Fire_Shock(self, start, forward, damage_plasma, speed, kick, damage_shockbolt, sv_shock_radius->value, self->client->homing_plasma);
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self-g_edicts);
	gi.WriteByte(MZ_BFG | is_silenced);
	gi.multicast(self->s.origin, MULTICAST_PVS);
	self->client->ps.gunframe++;
	PlayerNoise(self, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		self->client->pers.inventory[self->client->ammo_index] -= SR_CELLS_PER_SHOT;
}

void Weapon_ShockRifle(edict_t *self)
{
	static int	pause_frames[]	= {18, 29, 39, 58, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic(self, 3, 15, 58, 62, pause_frames, fire_frames, Weapon_ShockRifle_Fire);
}


/*
======================================================================
GAUSS PISTOL
======================================================================
*/
void Weapon_GaussPistol_Fire(edict_t *self)
{
	vec3_t		start;
	vec3_t		forward;
	vec3_t		right;
	vec3_t		offset;
	int			damage;
	int			kick = 2;
	qboolean	insufficient_ammo = false;

//	Check if there's enough ammo; swap weapons if we're dry.

	if (self->client->gauss_particle)
	{
		if ((self->client->pers.inventory[ITEM_INDEX(FindItem ("slugs"))] < GP_SLUGS_PER_SHOT)
			|| (self->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))] < GP_CELLS_PER_SHOT))
			insufficient_ammo = true;
	}
	else
	{
		if (self->client->pers.inventory[self->client->ammo_index] < 1)
			insufficient_ammo = true;
	}
	if (insufficient_ammo)
	{
		self->client->ps.gunframe = 10;
		if (level.time >= self->pain_debounce_time)
		{
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			self->pain_debounce_time = level.time + 1.0;
		}
		NoAmmoWeaponChange(self);
		return;
	}

//	Set damage and kick values.

	damage = (self->client->gauss_particle)? (int)sv_gauss_damage_particle->value : self->client->gauss_dmg;
	if (is_quad)
	{
		damage *= (int)sv_quad_factor->value;
		kick *= (int)sv_quad_factor->value;
	}

//	Set projectile start position and weapon kick info.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	if (self->client->gauss_particle)
		VectorSet(offset, 24.0, 8.0, self->viewheight-5.0);		// NB: should be same as offset in ShowGaussTarget()
	else
		VectorSet(offset, 0.0, 0.0, self->viewheight-2.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);
	
	VectorScale(forward, -2.0, self->client->kick_origin);
	self->client->kick_angles[0] = -1.0;
	
//	Fire!

	if (self->client->gauss_particle)
		Fire_Particle(self, start, forward, damage, kick);
	else
	{
		Fire_Instabolt(self, start, forward, damage, kick);
		self->client->gauss_framenum = level.framenum;
		self->client->gauss_dmg = (int)sv_gauss_damage_base->value;
	}

	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self-g_edicts);
	gi.WriteByte(MZ_SHOTGUN | is_silenced);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	self->client->ps.gunframe++;
	PlayerNoise(self, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		if (self->client->gauss_particle)
		{
			self->client->pers.inventory[self->client->ammo_index] -= GP_CELLS_PER_SHOT;
			self->client->pers.inventory[ITEM_INDEX(FindItem ("slugs"))] -= GP_SLUGS_PER_SHOT;
		}
		else
			self->client->pers.inventory[self->client->ammo_index]--;
	}
}

void Weapon_GaussPistol(edict_t *self)
{
	static int	pause_frames[]	= {18, 50, 0};
	static int	fire_frames[]	= {4, 0};

	if (!self->client->show_gausscharge)
	{
		if (!self->client->gauss_particle && ((self->client->weaponstate == WEAPON_FIRING) || (self->client->weaponstate == WEAPON_READY)))
		{
			self->client->show_gausscharge = true;
			self->client->gauss_framenum = level.framenum;
			self->client->gauss_dmg = (int)sv_gauss_damage_base->value;
		}
		else
			self->client->show_gausscharge = false;
	}

	Weapon_Generic(self, 3, 9, 50, 53, pause_frames, fire_frames, Weapon_GaussPistol_Fire);
}


/*
======================================================================
ANTI-GRAV MANIPULATOR
======================================================================
*/
void AGM_Reset(edict_t *self)
{
	self->client->agm_on = false;
	self->client->agm_push = false;
	self->client->agm_pull = false;

	if (self->client->agm_target != NULL)
	{
		self->client->agm_target->client->held_by_agm = false;
		self->client->agm_target->client->flung_by_agm = false;
		self->client->agm_target->client->thrown_by_agm = true;
		self->client->agm_target = NULL;
	}
}

void Weapon_AGM_Fire(edict_t *self)
{
	vec3_t		start;
	vec3_t		forward;
	vec3_t		right;
	vec3_t		offset;

	if (!(self->client->buttons & BUTTON_ATTACK))
	{
		self->client->weaponstate = WEAPON_READY;
		return;
	}

//	Once the AGM has tripped off, it can't be fired again until it has recharged to 100.

	if ((int)sv_agm_mode->value == 0)
	{
		if (self->client->agm_tripped && (self->client->agm_target == NULL))
		{
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/agm/agm_trip.wav"), 1, ATTN_STATIC, 0);
			return;
		}
	}

//	Wrap round animation to the start of the firing set if the fire button is still being pressed.

	self->client->ps.gunframe++;
	if ((self->client->ps.gunframe == 21) && self->client->pers.inventory[self->client->ammo_index])
	{
		if (self->client->buttons & BUTTON_ATTACK)
			self->client->ps.gunframe = 6;
		else
			return;
	}

//	Check if there's enough ammo; swap weapons if we're dry.

	if (self->client->pers.inventory[self->client->ammo_index] < (int)sv_agm_beam_cells->value)
	{
		self->client->ps.gunframe = 21;
		if (level.time >= self->pain_debounce_time)
		{
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			self->pain_debounce_time = level.time + 1.0;
		}
		AGM_Reset(self);
		NoAmmoWeaponChange(self);
		return;
	}

//	Set beam start position.
	
	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24.0, 7.0, self->viewheight-6.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);

//	Fire!

	if (self->client->agm_target != NULL)
		Move_AGM(self, start, forward);
	else
	{
		if (!self->client->agm_on)
		{
			self->client->agm_on = true;
			if (((int)sv_agm_mode->value == 0) && (!((int)dmflags->value & DF_INFINITE_AMMO)))
				self->client->pers.inventory[self->client->ammo_index] -= (int)sv_agm_shot_cells->value;
		}
		Fire_AGM(self, start, forward, self->client->agm_disrupt);
		gi.sound(self, CHAN_WEAPON, gi.soundindex("medic/medatck1.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(self, start, PNOISE_WEAPON);
	}

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		self->client->pers.inventory[self->client->ammo_index] -= (int)sv_agm_beam_cells->value;

//		The Haste powerup/Tech effect needs to be applied separately due to the way that
//		the secondary mode is implemented.

		if (CTFApplyHaste(self) || (self->client->haste_framenum > level.framenum))
		{
			self->client->pers.inventory[self->client->ammo_index] -= (int)sv_agm_beam_cells->value;
			CTFApplyHasteSound(self);
		}

		if (self->client->pers.inventory[self->client->ammo_index] < 0)
			self->client->pers.inventory[self->client->ammo_index] = 0;
	}

	if (self->client->pers.inventory[self->client->ammo_index] == 0)
		return;

//	Spend extra ammo if we're manipulating a player who has the Invulnerability.

	if ((self->client->agm_target != NULL) && (self->client->agm_target->client->invincible_framenum > level.framenum))
	{
		if ((int)sv_agm_invuln_cells->value > 0)
		{
			self->client->pers.inventory[self->client->ammo_index] -= (int)sv_agm_invuln_cells->value;
			if (self->client->pers.inventory[self->client->ammo_index] < 0)
				self->client->pers.inventory[self->client->ammo_index] = 0;
		}
	}

//	Decrease AGM charge if there's no target.

	if ((self->client->agm_target == NULL) && ((int)sv_agm_mode->value == 0))
	{
		self->client->agm_charge -= ((is_quad && self->client->agm_disrupt)?2:1) * (int)sv_agm_fire_rate->value;
		if (self->client->agm_charge <= 0)
		{
			self->client->agm_charge = 0;
			if (!self->client->agm_tripped)
			{
				self->client->agm_tripped = true;
				AGM_Reset(self);
			}
		}
	}
}

void Weapon_AGM(edict_t *self)
{
	static int	pause_frames[]	= {21, 43, 49, 0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0};
	static gitem_t *tech = NULL;

	if (!self->client->agm_showcharge && ((int)sv_agm_mode->value == 0))
		self->client->agm_showcharge = true;

//	Implement the effects of the '+push' and '+pull' commands.

	if (self->client->agm_pull)
	{
		self->client->agm_range -= AGM_RANGE_DELTA;
		if (self->client->agm_range < AGM_RANGE_MIN)
			self->client->agm_range = AGM_RANGE_MIN;
	}
	else if (self->client->agm_push)
	{
		self->client->agm_range += AGM_RANGE_DELTA;
		if (self->client->agm_range > AGM_RANGE_MAX)
			self->client->agm_range = AGM_RANGE_MAX;
	}

//	If the player has stopped firing and they had an AGM target, flag that target as being thrown.

	if (self->client->agm_on && !(self->client->buttons & BUTTON_ATTACK))
	{
		AGM_Reset(self);
		self->client->ps.gunframe = 21;

//		Play Quad or Tech sounds, if appropriate.

		if (!tech)
			tech = FindItemByClassname("item_tech2");

		if (tech && self->client && self->client->pers.inventory[ITEM_INDEX(tech)])
		{
			if (self->client->ctf_techsndtime < level.time)
			{
				self->client->ctf_techsndtime = level.time + 1.0;
				if (self->client->quad_framenum > level.framenum)
					gi.sound(self, CHAN_VOICE, gi.soundindex("ctf/tech2x.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound(self, CHAN_VOICE, gi.soundindex("ctf/tech2.wav"), 1, ATTN_NORM, 0);
			}
		}
		else if (self->client->quad_framenum > level.framenum)
			gi.sound(self, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
	}

//	Sanity check: if we have an AGM target and they've just died, but for some reason our agm_target
//	pointer hasn't been reset, then do it manually here.

	if ((self->client->agm_target != NULL) && (self->client->agm_target->health < 1))
	{
		self->client->agm_on = false;
		self->client->agm_target->client->flung_by_agm = false;
		self->client->agm_target->client->thrown_by_agm = false;
		self->client->agm_target = NULL;
	}

//	Charge up the AGM if it's not firing.

	if ((int)sv_agm_mode->value == 0)
	{
		if (!self->client->agm_on && (self->client->agm_charge < 100))
		{
			self->client->agm_charge += (int)sv_agm_charge_rate->value;
			if (self->client->agm_charge >= 100)
			{
				self->client->agm_charge = 100;
				if (self->client->agm_tripped)
				{
					self->client->agm_tripped = false;
					gi.sound(self, CHAN_VOICE, gi.soundindex("world/fusein.wav"), 1, ATTN_STATIC, 0);
				}
			}
		}
	}

//	Weapon actions.

	Weapon_Generic(self, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_AGM_Fire);
}


/*
======================================================================
DISC LAUNCHER
======================================================================
*/
void Weapon_DiscLauncher_Fire(edict_t *self)
{
	vec3_t	offset;
	vec3_t	start;
	vec3_t	forward;
	vec3_t	right;
	int		damage;
	int		kick = 20;

//	Set damage and kick values.

	damage = (int)sv_disc_damage->value;
	if (is_quad)
	{
		damage *= (int)sv_quad_factor->value;
		kick *= (int)sv_quad_factor->value;
	}

//	Set projectile start position and weapon kick info.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorScale(forward, -2.0, self->client->kick_origin);
	self->client->kick_angles[0] = -2.0;

	VectorSet(offset, 0.0, 8.0, self->viewheight-8.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);

//	Fire!

	Fire_Disc(self, start, forward, damage, sv_disc_speed->value, kick);
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self-g_edicts);
    gi.WriteByte(MZ_IONRIPPER | is_silenced);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	self->client->ps.gunframe++;
	PlayerNoise(self, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		self->client->pers.inventory[self->client->ammo_index]--;
}

void Weapon_DiscLauncher(edict_t *self)
{
	static int	pause_frames[]	= {25, 54, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic(self, 4, 14, 54, 58, pause_frames, fire_frames, Weapon_DiscLauncher_Fire);		
}


/*
======================================================================
CHAINSAW
======================================================================
*/
void Weapon_Chainsaw_Fire(edict_t *self)
{
	vec3_t	offset;
	vec3_t	start;
	vec3_t	forward;
	vec3_t	right;
	int		damage;
	int		kick = 25;

//	Set damage and kick values.

	damage = (int)sv_chainsaw_damage->value;
	if (is_quad)
	{
		damage *= (int)sv_quad_factor->value;
		kick *= (int)sv_quad_factor->value;
	}

	if ((self->client->haste_framenum > level.framenum) || CTFApplyHaste(self))
	{
		damage *= 2;
		kick *= 2;
	}

//	Set damage point start position and weapon kick info.

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorScale(forward, -2.0, self->client->kick_origin);
	self->client->kick_angles[0] = -2.0;

	VectorSet(offset, 8.0, 8.0, self->viewheight-8.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);

//	Fire!

	Fire_Chainsaw(self, start, forward, damage, kick);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/chainsaw/fire.wav"), 1, ATTN_NORM, 0);
	PlayerNoise(self, start, PNOISE_WEAPON);

//	We just want to repeat a couple of animation frames (19 and 20). Although the chainsaw model 
//	has two nifty attack sequences, we want to make it more like the Doom II chainsaw.

	self->client->ps.gunframe++;
	self->client->machinegun_shots = 1;
	if ((self->client->buttons & BUTTON_ATTACK) && (self->client->ps.gunframe == 21))
	{
		self->client->ps.gunframe = 19;
		return;
	}
	
	if (self->client->ps.gunframe == 8)
		self->client->ps.gunframe = 19;
}

void Weapon_Chainsaw(edict_t *self)
{
	static int	pause_frames[]	= {69, 0};
	static int	fire_frames[]	= {7, 19, 20, 21, 0};

//	Handle customised view model animation. Note that the machinegun_shots hack is needed 
//	to prevent screwing up the weaponstate value if instant switching is not enabled.

	if ((self->client->ps.gunframe < 22) && self->client->machinegun_shots && !(self->client->buttons & BUTTON_ATTACK))
	{
		self->client->ps.gunframe = 28;
		self->client->machinegun_shots ^= 1;
	}

	Weapon_Generic(self, 6, 29, 69, 73, pause_frames, fire_frames, Weapon_Chainsaw_Fire);

//	Play the "revving down" sound at the start of the putaway sequence. (Note that this frame isn't 
//	played if DF_FAST_SWITCH is set, so for that case we have to do it in Weapon_Generic2() manually).

	if (self->client->ps.gunframe == 70)
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/chainsaw/done.wav"), 1, ATTN_NORM, 0);
}
//CW--
