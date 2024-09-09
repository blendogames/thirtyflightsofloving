/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/p_weapon.c,v $
 *   $Revision: 1.49 $
 *   $Date: 2002/07/23 22:48:28 $
 *
 ***********************************

Copyright (C) 2002 Vipersoft

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// g_weapon.c

#include "g_local.h"
#include "m_player.h"

/*-----/ PM /-----/ NEW:  Include new header files. /-----*/
#include "x_fbomb.h"
#include "x_fire.h"
/*--------------------------------------------------------*/

//bcass start - medic sound thing
#define MEDIC1		0
#define MEDIC2		33
#define MEDIC3		66
#define MEDICH		100
//bcass end

void Cmd_WeapNext_f (edict_t *ent);
void weapon_grenade_fire (edict_t *ent);
//bcass start - TNT
void weapon_tnt_fire (edict_t *ent);
//bcass end
void ifchangewep(edict_t *ent);
edict_t *ApplyFirstAid (edict_t *ent);
void Play_WepSound(edict_t *ent, char *sound);
void Use_Weapon (edict_t *ent, gitem_t *item);
void ClientSetMaxSpeed(edict_t *ent, qboolean sync);
void Shrapnel_Explode (edict_t *ent);
//bcass start
void TNT_Think (edict_t *ent);
void TNT_Explode (edict_t *ent);
void TNT_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
//bcass end
void Shrapnel_Dud (edict_t *ent);
void Shrapnel_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void check_unscope (edict_t *ent);//faf

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

//	if (deathmatch->value)
//		return;

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


qboolean Pickup_Weapon (edict_t *ent, edict_t *other)
{
	int			index;
	gitem_t		*ammo;
	gitem_t		*item;
	index = ITEM_INDEX(ent->item);

	item = ent->item;

	if ( ( ((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value) 
		&& other->client->pers.inventory[index])
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
			return false;	// leave the weapon for others to pickup
	}

	other->client->pers.inventory[index]++;

	if (!WeighPlayer(other))
	{
		safe_cprintf(other, PRINT_HIGH, "You can not carry anymore weapons!\n");
		other->client->pers.inventory[index]--;
		return false;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) )
	{
		// give them some ammo with it
		if(ent->item->ammo)
		{
			ammo = FindItem (ent->item->ammo);
//			if ( (int)dmflags->value & DF_INFINITE_AMMO )
//				Add_Ammo (other, ammo, 1000);
//			else
//				Add_Ammo (other, ammo, ammo->quantity);
			if (ent->item->guninfo && ent->item->guninfo->rnd_count)
			ent->item->guninfo->rnd_count = ammo->quantity;


			//faf: put bullets in map spawned guns
			if (other->client)
				Load_Weapon (other, item);

/*
			if (ent->item->guninfo &&
				ent->item->guninfo->rnd_count)
			{
				if (!strcmp(item->ammo, "p38_mag")) {
					other->client->mags[1].pistol_rnd = item->guninfo->rnd_count;
				}
				else if (!strcmp(item->ammo, "mauser98k_mag")) { // Both Rifle and Sniper ammo
					other->client->mags[1].rifle_rnd = item->guninfo->rnd_count;
				}
				else if (!strcmp(item->ammo, "mp40_mag")) {
					other->client->mags[1].submg_rnd = item->guninfo->rnd_count;
				}
				else if (!strcmp(item->ammo, "mp43_mag")) {
					other->client->mags[1].lmg_rnd = item->guninfo->rnd_count;
				}
				else if (!strcmp(item->ammo, "mg42_mag")) {
					other->client->mags[1].hmg_rnd = item->guninfo->rnd_count;
				}
				else if (!strcmp(item->ammo, "grm_rockets")) {
					other->client->mags[1].antitank_rnd = item->guninfo->rnd_count;
				}
				else if (!strcmp(item->ammo, "colt45_mag")) {
					other->client->mags[0].pistol_rnd = item->guninfo->rnd_count;
				}
				else if (!strcmp(item->ammo, "m1_mag")) {
					other->client->mags[0].rifle_rnd = item->guninfo->rnd_count;
				}
				else if (!strcmp(item->ammo, "thompson_mag")) {
					other->client->mags[0].submg_rnd = item->guninfo->rnd_count;
				}
				else if (!strcmp(item->ammo, "bar_mag")) {
					other->client->mags[0].lmg_rnd = item->guninfo->rnd_count;
				}
				else if (!strcmp(item->ammo, "hmg_mag")) {
					other->client->mags[0].hmg_rnd = item->guninfo->rnd_count;
				}
				else if (!strcmp(item->ammo, "usa_rockets")) {
					other->client->mags[0].antitank_rnd = item->guninfo->rnd_count;
				}
				else if (!strcmp(item->ammo, "m1903_mag")) {
					other->client->mags[0].sniper_rnd = item->guninfo->rnd_count;
				}
				else if (!strcmp(item->ammo, "flame_mag")) {
					other->client->flame_rnd = item->guninfo->rnd_count;
				}
			}
*/
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

/*  BROKEN: Pick up flamethrower bug..
	Using guninfo in this function crashes Quake2...
	if (!strcmp(ent->item->classname, "weapon_flamethrower"))
		other->client->flame_rnd .....
*/

	if (other->client->pers.weapon != ent->item && 
		(other->client->pers.inventory[index] == 1) &&
		( !deathmatch->value ))//faf:  removing this fixes bug with picking up helmet|| other->client->pers.weapon == FindItem("Colt .45") ) )
		other->client->newweapon = ent->item;

	if (!Q_stricmp(other->client->pers.weapon->pickup_name,"Fists") )
	{
		if (Q_stricmp(ent->item->pickup_name, "Helmet") )
			other->client->newweapon = ent->item;
	}

//	gi.dprintf("%s\n",other->client->pers.weapon->pickup_name);


//	WeighPlayer (other);
	return true;
}


/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/

//char *skinDir (char *s, char *extra);
//pbowens: v_wep
void ShowGun( edict_t *ent)
{
    int nIndex;
    char *pszIcon;
	char *weap_team;//faf

    // No weapon?
    if ( !ent->client->pers.weapon)
    {
        ent->s.modelindex2 = 0;
        return;
    }

    // Determine the weapon's precache index.

    nIndex = 0;
    pszIcon = ent->client->pers.weapon->icon;
	weap_team = ent->client->pers.weapon->dllname;//faf

	/*******************************************
	      THIS IS HARD CODED INTO THE DLL
	*******************************************/

	// GRM 
	if		( strcmp( pszIcon, "w_p38") == 0)
		nIndex = 1;
	else if ( strcmp( pszIcon, "w_m98k") == 0)
        nIndex = 2;
	else if ( strcmp( pszIcon, "w_mp40") == 0)
		nIndex = 3;
	else if ( strcmp( pszIcon, "w_mp43") == 0)
		nIndex = 4;
	else if ( strcmp( pszIcon, "w_mg42") == 0)
		nIndex = 5;
	else if ( strcmp( pszIcon, "w_panzer") == 0)
		nIndex = 6;
	else if ( strcmp( pszIcon, "w_m98ks") == 0)
		nIndex = 7;
	else if ( strcmp( pszIcon, "a_masher") == 0)
		nIndex = 8;

	// USA
	else if ( strcmp( pszIcon, "w_colt45") == 0)
		nIndex = 9;
	else if ( strcmp( pszIcon, "w_m1") == 0)
		nIndex = 10;
	else if ( strcmp( pszIcon, "w_thompson") == 0)
		nIndex = 11;
	else if ( strcmp( pszIcon, "w_bar") == 0)
		nIndex = 12;
	else if ( strcmp( pszIcon, "w_bhmg") == 0)
		nIndex = 13;
	else if ( strcmp( pszIcon, "w_bazooka") == 0)
		nIndex = 14;
	else if ( strcmp( pszIcon, "w_m1903") == 0)
		nIndex = 15;
	else if ( strcmp( pszIcon, "a_grenade") == 0)
		nIndex = 16;

	// GENERIC
	else if ( strcmp( pszIcon, "w_flame") == 0)
		nIndex = 17;
	else if ( strcmp( pszIcon, "w_morphine") == 0)
		nIndex = 18;
	else if ( strcmp( pszIcon, "w_knife") == 0)
		nIndex = 19;
	else if ( strcmp( pszIcon, "w_binoc") == 0)
		nIndex = 20;
	else if ( strcmp( pszIcon, "w_tnt") == 0)
		nIndex = 21;


/*
    if		( strcmp( pszIcon, "w_pistol") == 0)
        nIndex = 1;
    else if ( strcmp( pszIcon, "w_rifle") == 0)
        nIndex = 2;
    else if ( strcmp( pszIcon, "w_submg") == 0)
        nIndex = 3;
    else if ( strcmp( pszIcon, "w_lmg") == 0)
        nIndex = 4;
    else if ( strcmp( pszIcon, "w_hmg") == 0)
        nIndex = 5;
    else if ( strcmp( pszIcon, "w_rocket") == 0)
        nIndex = 6;
    else if ( strcmp( pszIcon, "w_sniper") == 0)
        nIndex = 7;
    else if ( strcmp( pszIcon, "w_flame") == 0)
        nIndex = 8;
    else if ( strcmp( pszIcon, "w_morphine") == 0)
        nIndex = 9;
    else if ( strcmp( pszIcon, "w_knife") == 0)
        nIndex = 10;
    else if ( strcmp( pszIcon, "a_grenades") == 0)
        nIndex = 11;
    else if ( strcmp( pszIcon, "w_masher") == 0)
        nIndex = 12;
	else if (strcmp( pszIcon, "w_tnt") == 0)
		nIndex = 13;
*/
	// Clear previous weapon model.
    ent->s.skinnum &= 255;

    // Set new weapon model.
    ent->s.skinnum |= (nIndex << 8);
    //ent->s.modelindex2 = (MAX_MODELS-1);

	//faf: old way->	ent->s.modelindex2 = gi.modelindex (va("players/%s/%s.md2", ent->client->resp.team_on->playermodel, pszIcon));

	//faf:  this makes it so only 1 w_weapon is loaded per weapon, except russian
	//      weapons have different player animations, so they need to be separate.  This
	//      will have to be removed if Parts adds arm animations.
	
//	if (extra_anims->value ==0)
//		ent->s.modelindex2 = gi.modelindex (va("players/%s/%s.md2", ent->client->resp.team_on->playermodel, pszIcon));
//	else
//	{
		
//		if (!strcmp(team_list[0]->teamid, "rus") || !strcmp(team_list[1]->teamid, "rus"))
//			ent->s.modelindex2 = gi.modelindex (va("players/%s/%s.md2", ent->client->resp.team_on->playermodel, pszIcon));

//		else 
	if (ent->client->pers.weapon->classnameb == WEAPON_MOLOTOV)
	{
		ent->s.modelindex2 = gi.modelindex ("players/jpn/w_molotov.md2");
	}
	else if (ent->client->pers.weapon->classnameb == WEAPON_SANDBAGS)
	{
		ent->s.modelindex2 = gi.modelindex ("players/usa/w_fists.md2");
	}	
	else if (ent->client->pers.weapon->position == LOC_FLAME && !strcmp(ent->client->resp.team_on->teamid, "gbr"))
	{
		ent->s.modelindex2 = gi.modelindex (va("players/%s/%s.md2", ent->client->resp.team_on->playermodel, pszIcon));
	}
	else if (ent->client->pers.weapon->classnameb == WEAPON_KATANA)
	{
		ent->s.modelindex2 = gi.modelindex ("players/jpn/w_katana.md2");
	}

	else if (ent->client->pers.weapon->classnameb == WEAPON_SABRE)
	{
		ent->s.modelindex2 = gi.modelindex ("players/pol/w_sabre.md2");
	}
	//faf: non team specific weaps will just use the w_ md2s in players/usa folder
	else if (ent->client->pers.weapon->position == LOC_KNIFE ||
		ent->client->pers.weapon->position == LOC_HELMET ||
		ent->client->pers.weapon->position == LOC_FLAME ||
		
		ent->client->pers.weapon->position == LOC_TNT ||
		strcmp( pszIcon, "w_binoc") == 0 ||
		strcmp( pszIcon, "w_morphine") == 0
		)
		ent->s.modelindex2 = gi.modelindex (va("players/usa/%s.md2", pszIcon));
	else if (weap_team)
	{
		ent->s.modelindex2 = gi.modelindex (va("players/%s/%s.md2", weap_team, pszIcon));//faf
	}
	else  //this shouldnt happen...
		ent->s.modelindex2 = gi.modelindex (va("players/%s/%s.md2", ent->client->resp.team_on->playermodel, pszIcon));
//	}
//	safe_cprintf(ent, PRINT_HIGH, "%s.\n", weap_team);
}

void ChangeWeapon (edict_t *ent)
{
	//pbowens: This was causing it to explode once it was picked up.
	//			moved to ClientThink();
	/*
	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire (ent, false);
		ent->client->grenade_time = 0;
	}
*/
	// pbowens: dont switch weapons if live grenade
	if (ent->client->newweapon && 
		((ent->client->grenade	&& ent->client->newweapon->position != LOC_GRENADES && !ent->client->grenade_index) ||
		 (ent->client->tnt		&& ent->client->newweapon->position != LOC_TNT)		) )
		return;


	if (ent->client->pers.weapon && ent->client->pers.weapon->position  == LOC_GRENADES)
		ent->client->aim = false;


	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	check_unscope(ent);//faf

	ent->client->ps.fov = STANDARD_FOV;

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
	{	ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
	}
	else{
		ent->client->ammo_index = 0;
	}





	//gi.dprintf("ammo_index: %i\n", ent->client->ammo_index);

	if (!ent->client->pers.weapon || ent->s.modelindex != (MAX_MODELS-1)) //pbowens: v_wep
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}
	ent->client->p_rnd=NULL;
	ent->client->p_fract=NULL;
	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

	// pbowens: v_wep
	ent->client->anim_priority = ANIM_PAIN;

	// pbowens: added frames for prone/crawl
    if (ent->stanceflags == STANCE_STAND)
    {
            ent->s.frame = FRAME_pain301;
            ent->client->anim_end = FRAME_pain304;
    }
    else if (ent->stanceflags == STANCE_DUCK)
    {
            ent->s.frame = FRAME_crpain1;
            ent->client->anim_end = FRAME_crpain4;
    }
    else if (ent->stanceflags == STANCE_CRAWL)
    {
            ent->s.frame = FRAME_crawlpain01;
            ent->client->anim_end = FRAME_crawlpain04;
    }
    
    ShowGun (ent);    
	WeighPlayer (ent);
	
	ent->client->pickup_msg_time = level.time + 2.0;

	if (!Q_stricmp(ent->client->pers.weapon->pickup_name,"Fists") )
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}
	else
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->client->pers.weapon->icon);
		ent->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->client->pers.weapon);
	}
} 

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("m1903"))] )
	{
		ent->client->newweapon = FindItem ("M1903");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Thompson"))] )
	{
		ent->client->newweapon = FindItem ("Thompson");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("HMGAmmo"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("M1919 Browning"))] )
	{
		ent->client->newweapon = FindItem ("M1919 Browning");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("HMGAmmo"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("BAR"))] )
	{
		ent->client->newweapon = FindItem ("BAR");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] > 1
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("M1 Garand"))] )
	{
		ent->client->newweapon = FindItem ("M1 Garand");
		return;
	}
	ent->client->newweapon = FindItem ("Colt .45");
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
	if(ent->client->limbo_mode ) 
		return;

	if (ent->health < 1)
	{
		ent->client->newweapon = NULL;
		ChangeWeapon (ent);
	}


	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
//		is_quad = (ent->client->quad_framenum > level.framenum);
		if (ent->client->silencer_shots)
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;
		ent->client->pers.weapon->weaponthink (ent);
	}
}

void turret_off (edict_t *self);
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
	int			item_rounds;

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;


	turret_off(ent);


	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!strcmp(item->ammo, "p38_mag"))
			item_rounds = ent->client->mags[1].pistol_rnd;
		else if (!strcmp(item->ammo, "mauser98k_mag")) // Both Rifle and Sniper ammo
			item_rounds = ent->client->mags[1].rifle_rnd + ent->client->mags[1].sniper_rnd;
		else if (!strcmp(item->ammo, "mp40_mag"))
			item_rounds = ent->client->mags[1].submg_rnd;
		else if (!strcmp(item->ammo, "mp43_mag"))
			item_rounds = ent->client->mags[1].lmg_rnd;
		else if (!strcmp(item->ammo, "mg42_mag"))
			item_rounds = ent->client->mags[1].hmg_rnd;
		else if (!strcmp(item->ammo, "grm_rockets"))
			item_rounds = ent->client->mags[1].antitank_rnd;
		else if (!strcmp(item->ammo, "colt45_mag"))
			item_rounds = ent->client->mags[0].pistol_rnd;
		else if (!strcmp(item->ammo, "m1_mag"))
			item_rounds = ent->client->mags[0].rifle_rnd;
		else if (!strcmp(item->ammo, "thompson_mag"))
			item_rounds = ent->client->mags[0].submg_rnd;
		else if (!strcmp(item->ammo, "bar_mag"))
			item_rounds = ent->client->mags[0].lmg_rnd;
		else if (!strcmp(item->ammo, "hmg_mag"))
			item_rounds = ent->client->mags[0].hmg_rnd;
		else if (!strcmp(item->ammo, "usa_rockets"))
			item_rounds = ent->client->mags[0].antitank_rnd;
		else if (!strcmp(item->ammo, "m1903_mag"))
			item_rounds = ent->client->mags[0].sniper_rnd;
		else if (!strcmp(item->ammo, "flame_mag"))
			item_rounds = ent->client->flame_rnd;

		/* RM:Future site of the team-enhanced guninfo statement
		if ( !strcmp( item->icon, "w_pistol") )
			item_rounds = ent->client->mags[item->guninfo->teamindex].pistol_rnd;
		else if ( !strcmp( item->icon, "w_rifle") )  // Add rifle and sniper ammo if axis?
			item_rounds = ent->client->mags[item->guninfo->teamindex].rifle_rnd;
		else if ( !strcmp( item->icon, "w_submg") )
			item_rounds = ent->client->mags[item->guninfo->teamindex].submg_rnd;
		else if ( !strcmp( item->icon, "w_lmg") )
			item_rounds = ent->client->mags[item->guninfo->teamindex].lmg_rnd;
		else if ( !strcmp( item->icon, "w_hmg") )
			item_rounds = ent->client->mags[item->guninfo->teamindex].hmg_rnd;
		else if ( !strcmp( item->icon, "w_rocket") )
			item_rounds = ent->client->mags[item->guninfo->teamindex].antitank_rnd;
		else if ( !strcmp( item->icon, "w_sniper") )
			item_rounds = ent->client->mags[item->guninfo->teamindex].sniper_rnd;
		*/

		//pbowens: allows player to switch to empty gun now
/*		if (!ent->client->pers.inventory[ammo_index] && !item_rounds)
		{
			safe_cprintf (ent, PRINT_HIGH, "No magazines or clips for %s.\n", item->pickup_name);
			//return;
		}*/

	}

	// change to this weapon when down
	//ent->client->weaponstate=WEAPON_READY;
	ent->client->newweapon = item;
}


/*
===============
Drop_Weapon
================
*/

void Drop_Weapon (edict_t *ent, gitem_t *item)
{
	int		index;
	int		item_rounds;
	item_rounds = 0;

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
		return;

	index = ITEM_INDEX(item);
	
	// pbowens: fix in not drop weapon/while prone
	if ( !item || 
		((item == ent->client->newweapon)) && (ent->client->pers.inventory[index] == 1))// && (ent->stanceflags != STANCE_STAND))
	{
		//safe_cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	//pbowens: This is one disgusting, ugly hack to get the weapons to retain their current
	//			round counts when dropped/picked up.

	        //faf:  changed a bit for team dll support:

        //faf:  gonna leave this in for grm mauser/sniper using same ammo
        if (!strcmp(item->ammo, "mauser98k_mag"))  // Both Rifle and Sniper ammo
        {
                item_rounds = ent->client->mags[1].rifle_rnd + ent->client->mags[1].sniper_rnd;
                ent->client->mags[1].rifle_rnd = ent->client->mags[1].sniper_rnd = 0;
        }


        else if (!strcmp(item->dllname, team_list[1]->teamid))  //faf: if team 1 weap... usually axis
        {
                if (item->position == LOC_PISTOL)
                {
                        item_rounds = ent->client->mags[1].pistol_rnd;
                        ent->client->mags[1].pistol_rnd = 0;
                }
        //faf:  small bug below here:  Sniper ammo will work properly, rifle wont... (if they use the same ammo)
                else if (item->position == LOC_RIFLE)
                {
                        item_rounds = ent->client->mags[1].rifle_rnd + ent->client->mags[1].sniper_rnd;
                        ent->client->mags[1].rifle_rnd = 0;
                }
                else if (item->position == LOC_SNIPER)
                {
                        item_rounds = ent->client->mags[1].sniper_rnd + ent->client->mags[1].rifle_rnd;
                        ent->client->mags[1].sniper_rnd = 0;
                }
                else if (item->position == LOC_SUBMACHINEGUN)
                {
                        item_rounds = ent->client->mags[1].submg_rnd;
                        ent->client->mags[1].submg_rnd = 0;
                }
                else if (item->position == LOC_L_MACHINEGUN)
                {
                        item_rounds = ent->client->mags[1].lmg_rnd;
                        ent->client->mags[1].lmg_rnd = 0;
                }
                else if (item->position == LOC_H_MACHINEGUN)
                {
                        item_rounds = ent->client->mags[1].hmg_rnd;
                        ent->client->mags[1].hmg_rnd = 0;
                }
                else if (item->position == LOC_ROCKET)
                {
                        item_rounds = ent->client->mags[1].antitank_rnd;
                        ent->client->mags[1].antitank_rnd = 0;
                }
                 else if (item->position == LOC_SHOTGUN)
                { 
				   item_rounds = ent->client->mags[1].shotgun_rnd;
					 ent->client->mags[1].shotgun_rnd = 0;
                }

                else if (item->position == LOC_SUBMACHINEGUN2)
                {
                        item_rounds = ent->client->mags[1].submg_rnd;
                        ent->client->mags[1].submg2_rnd = 0;
                }
        }
        else if (!strcmp(item->dllname, team_list[0]->teamid))  //faf:  if team 0 weap...usually allied
        {
                if (item->position == LOC_PISTOL)
                {
                item_rounds = ent->client->mags[0].pistol_rnd;
                ent->client->mags[0].pistol_rnd = 0;
                }
                else if (item->position == LOC_SUBMACHINEGUN)
                {
                item_rounds = ent->client->mags[0].submg_rnd;
                ent->client->mags[0].submg_rnd = 0;
                }
                else if (item->position == LOC_L_MACHINEGUN)
                {
                item_rounds = ent->client->mags[0].lmg_rnd;
                ent->client->mags[0].lmg_rnd = 0;
                }
                else if (item->position == LOC_H_MACHINEGUN)
                {
                item_rounds = ent->client->mags[0].hmg_rnd;
                ent->client->mags[0].hmg_rnd = 0;
                }
                else if (item->position == LOC_ROCKET)
                {
                item_rounds = ent->client->mags[0].antitank_rnd;
                ent->client->mags[0].antitank_rnd = 0;
                }
        //faf:  small bug below here:  Sniper ammo will work properly, rifle wont... (if they use same ammo)
                else if (item->position == LOC_SNIPER)
                {
                        item_rounds = ent->client->mags[0].sniper_rnd + ent->client->mags[0].rifle_rnd;
                        ent->client->mags[0].sniper_rnd = 0;
                }
                else if (item->position == LOC_RIFLE)
                { 
                        item_rounds = ent->client->mags[0].rifle_rnd + ent->client->mags[0].sniper_rnd;
                        ent->client->mags[0].rifle_rnd = 0;
                }
                 else if (item->position == LOC_SHOTGUN)
                { 
					item_rounds = ent->client->mags[0].shotgun_rnd;
					ent->client->mags[0].shotgun_rnd = 0;
                }
                else if (item->position == LOC_SUBMACHINEGUN2)
                {
                item_rounds = ent->client->mags[0].submg2_rnd;
                ent->client->mags[0].submg2_rnd = 0;
                }
       }
        else if (!strcmp(item->ammo, "flame_mag"))
        {
                item_rounds = ent->client->flame_rnd;
                ent->client->flame_rnd = 0;
        }




	if (item->guninfo)
		item->guninfo->rnd_count = item_rounds;

	Drop_Item (ent, item);

	ent->client->pers.inventory[index] = 0;
}


//weapon generic wuz here!
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_LFIRE_LAST, int FRAME_LIDLE_LAST, int FRAME_RELOAD_LAST, int FRAME_LASTRD_LAST,
					 int FRAME_DEACTIVATE_LAST, int FRAME_RAISE_LAST,int FRAME_AFIRE_LAST, int FRAME_AIDLE_LAST,
					 int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));






/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER		4.5
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

void weapon_grenade_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 225;
	int		speed;
	float	radius;

	// unused
	float	time = 0;
	int		team = 0;


	//faf
	if (ent->client && ent->client->grenade) // disable the grenade from triggering self, but still leave timers on
	{
		VectorCopy(ent->s.origin, ent->client->grenade->s.origin);
		ent->client->grenade->s.modelindex = 0; // set model to null
		//client->grenade_caught->touch = NULL;
//		ent->client->grenade->s.origin[2]+= 100;//faf

	}


	radius = damage + 40;
	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (ent->client->ps.pmove.pm_type == PM_DEAD)
		speed = 5; // drop the grenade
	else
	{	speed = GRENADE_MINSPEED + (int)(-(ent->client->grenade->nextthink - level.time) + 2.75) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	//gi.dprintf("speed: %i\n", speed);

	speed = 500;
}
	if (ent->client->aim)
		speed *=1.4;

	ent->client->aim = false;

//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;

	fire_grenade2 (ent, start, forward, damage, speed, time, radius, team);

//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;

//	ent->client->grenade_time = level.time + 1.0;
}

void weapon_grenade_prime (edict_t *ent, int team)
{
//	vec3_t	offset;
//	vec3_t	forward, right;
//	vec3_t	start;
	int		damage;
//	int		speed;
//	int		team;
	float	radius;
	float	timer;
	float	dudchance;
	
	edict_t	*grenade;

	damage = 255;
	timer  =  level.time + GRENADE_TIMER + abs((int)random());
	radius = damage + 40;

	grenade = G_Spawn();

	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_TRIGGER;
	//		grenade->s.effects |= EF_GRENADE;
		
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	
	//grenade->s.modelindex = gi.modelindex (va("models/objects/%s/tris.md2", (ent->client->resp.team_on->index) ? "masher" : "grenade2"));
	
	grenade->owner = ent;
	grenade->nextthink = timer;
	grenade->dmg = damage;
	grenade->dmg_radius = radius;
	grenade->classname = "hgrenade";
	grenade->classnameb = HGRENADE;
	grenade->obj_owner = team;

//	if (IsValidPlayer(ent) && ent->client->resp.team_on->mos[ent->client->resp.mos]->grenades)
//		grenade->item = FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->grenades);
//	else
//		grenade->item = FindItem(va("%s", (team) ? "Stielhandgranate" : "Mk 2 Grenade" ));

	if (ent->client->pers.weapon && ent->client->pers.weapon->position == LOC_GRENADES)
		grenade->item = ent->client->pers.weapon;
	else
		grenade->item = FindItem(va("%s", (team) ? "Stielhandgranate" : "Mk 2 Grenade" ));
	
	dudchance = rand() % 100;
	//gi.dprintf("chance: %f", dudchance);

	if (dudchance > 1.0) // 1% chance its a dud
		grenade->think = Shrapnel_Explode;
	else
	{
		ent->client->grenade_index = ITEM_INDEX(grenade->item);
		grenade->think = Shrapnel_Dud;
	}

	//grenade->touch = Shrapnel_Touch;

	grenade->spawnflags = 1;

	gi.linkentity (grenade);

	ent->client->grenade = grenade;

//	ent->client->grenade_time = level.time + 1.0;
}


void Weapon_Grenade (edict_t *ent)
{
//	if(	(!ent->client->grenade_index && !ent->client->pers.inventory[ent->client->ammo_index]) || 
//buggy, changed back to old way, watch for crash?		(ent->client->grenade_index) )//jpn crash here
	if(	(!ent->client->grenade_index && !ent->client->pers.inventory[ent->client->ammo_index]) || 
		(ent->client->grenade_index && !ent->client->pers.inventory[ent->client->grenade_index] && ent->client->weaponstate != WEAPON_FIRING) )

	{
	//	gi.dprintf("next\n");
		
		if (ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon1 &&
			ent->client->pers.inventory[ITEM_INDEX(FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon1))])
		{
			ent->client->newweapon = FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon1);
			ChangeWeapon(ent);
		} else
			Cmd_WeapNext_f(ent);

		ent->client->grenade_index = 0;
	}

	if (ent->client->pers.weapon &&
		ent->client->pers.weapon->pickup_name &&
		frame_output)
		gi.dprintf("%i / %i - %s\n", ent->client->weaponstate, ent->client->ps.gunframe, ent->client->pers.weapon->pickup_name);


	ent->client->crosshair = false;

//	if (ent->client->aim)
//		ent->client->aim = false;

	if (!ent->client->grenade && (ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_RAISE || 
		ent->client->weaponstate == WEAPON_LOWER )	{
		ent->client->weaponstate =  WEAPON_READY;
		ent->client->ps.gunframe = (ent->client->grenade) ? 10 : 18;
	}


	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate =  WEAPON_READY;
		ent->client->ps.gunframe = (ent->client->grenade) ? 10 : 18;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			/*
				// dont work in water, jimmy
				if (ent->waterlevel == 3)
				{
					ent->client->weaponstate = WEAPON_READY;
					//ent->client->ps.gunframe = 1;
					return;
				}
			*/

			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (1)//ent->client->pers.inventory[ent->client->ammo_index])
			{
				// You don't pull a pin from a live grenade
				if (ent->client->pers.weapon->classnameb == AMMO_GRENADES_GBR)
					ent->client->ps.gunframe = (ent->client->grenade) ? 11 : 5;
				else
					ent->client->ps.gunframe = (ent->client->grenade) ? 11 : 1;
				ent->client->weaponstate = WEAPON_FIRING;
//				if (!ent->client->grenade) 
//					ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				/*if(auto_weapon_change->value)*/
					//NoAmmoWeaponChange (ent);
			}
			return;
		}

		if (ent->client->grenade) { // No "idle" frames if it's active
			ent->client->ps.gunframe = 10;
			return;
		}
		else
		{

			if ( (ent->client->ps.gunframe == 29) || 
				 (ent->client->ps.gunframe == 41) || 
				 (ent->client->ps.gunframe == 19) || 
				 (ent->client->ps.gunframe == 48) )
			{
				if (rand()&15)
					return;
			}

			ent->client->ps.gunframe++;

			if (ent->client->ps.gunframe >= 51)
				ent->client->ps.gunframe = 18;

			return;
		}
	}

	//animation for pulling pin
	if (ent->client->weaponstate == WEAPON_FIRING)
	{

		if (ent->client->ps.gunframe == 3)
		{
			//faf:  play part of wave animation when pulling pin
			if (ent->client && ent->stanceflags == STANCE_STAND) 
			{
				ent->client->anim_priority = ANIM_WAVE;
				ent->s.frame = 61;//(FRAME_wave05);
				ent->client->anim_end = 65;// (FRAME_wave10);
			} //end faf
			else if (ent->stanceflags == STANCE_DUCK) 
			{
				ent->client->anim_priority = ANIM_WAVE;
				ent->s.frame = 169;//(FRAME_crpain1);
				ent->client->anim_end = 172;// (FRAME_crpain4);
			} //end faf
			else if (ent->stanceflags == STANCE_CRAWL) 
			{
				ent->client->anim_priority = ANIM_WAVE;
				ent->s.frame = 222;//(FRAME_crawlattck02);
				ent->client->anim_end = 225;// (FRAME_crawlattck05);
			} //end faf
		}



		// Pull the pin, and prime the grenade
//faf		if (ent->client->ps.gunframe == 5 && !ent->client->grenade)
		//faf:  time pin pull sound better
		if (ent->client->pers.weapon->classnameb == AMMO_GRENADES_GBR)
		{
			if (ent->client->ps.gunframe == 5 && !ent->client->grenade)//faf
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);
				weapon_grenade_prime(ent, ent->client->resp.team_on->index);
				if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
					ent->client->pers.inventory[ent->client->ammo_index]--;

			}
		}
		else if (ent->client->pers.weapon->classnameb == AMMO_GRENADES_RUS)
		{
			if (ent->client->ps.gunframe == 2 && !ent->client->grenade)//faf
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);
				weapon_grenade_prime(ent, ent->client->resp.team_on->index);
				if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
					ent->client->pers.inventory[ent->client->ammo_index]--;

			}
		}
		else if (ent->client->pers.weapon->classnameb == AMMO_GRENADES_JPN)
		{
			if (ent->client->ps.gunframe == 1 && !ent->client->grenade)//faf
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);
			}

			if (ent->client->ps.gunframe == 6 && !ent->client->grenade)//faf
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("ita/b3842/unload.wav"), 1, ATTN_NORM, 0);
				weapon_grenade_prime(ent, ent->client->resp.team_on->index);
				if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
					ent->client->pers.inventory[ent->client->ammo_index]--;
			}
		}
		else if (ent->client->ps.gunframe == 1 && !ent->client->grenade)
		{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);
			weapon_grenade_prime(ent, ent->client->resp.team_on->index);
				if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
					ent->client->pers.inventory[ent->client->ammo_index]--;
		}


		// Let the player hold the grenade mid-firing
		if (ent->client->ps.gunframe == 13)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
				return;
		}

		// Throw it
		if (ent->client->ps.gunframe == 14)
		{
			weapon_grenade_fire (ent);
//			ent->client->grenade = NULL;
		}

		if (ent->client->ps.gunframe == 14) // throw the grenade
		{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/throw.wav"), 1, ATTN_NORM, 0);
			ent->client->throw_grenade_time = level.time;
		}
		
//		if ((ent->client->ps.gunframe == 17))
//			return;

		if (ent->client->ps.gunframe >= 17)
		{
//			ent->client->grenade_time = 0;
//			ent->client->ps.gunframe = 10;
			ent->client->weaponstate = WEAPON_READY;
			return;
		}

		ent->client->ps.gunframe++;

	}
}

/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
	float	radius;

	radius = damage+40;
	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

//	fire_grenade (ent, start, forward, damage, 600, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Mine (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64,0,0,0,0,0, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}

int Play_Bullet_Hit(edict_t *ent, char *surface, vec3_t endpos, edict_t *impact_ent);

/*
=============
fire_knife
=============
*/
qboolean Cmd_Scope_f(edict_t *ent);
void fire_Knife ( edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, char *wav, qboolean fists)
{    
    trace_t tr; //detect whats in front of you up to range "vec3_t end"

    vec3_t end;

    // Figure out what we hit, if anything:

    VectorMA (start, KNIFE_RANGE, aimdir, end);  //calculates the range vector                      
    tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
                        // figures out what in front of the player up till "end"
    
   // Figure out what to do about what we hit, if anything

    if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))    
    {
        if (tr.fraction < 1.0)        
        {            
            if (tr.ent->takedamage)            
            {
            //This tells us to damage the thing that in our path...hehe
				if (self->client && self->client->pers.weapon &&
					self->client->pers.weapon->classnameb == WEAPON_ENFIELD)
				{
					T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, 100, 50, 0,MOD_BAYONET);//faf
				}
				else if (self->client && self->client->pers.weapon &&
					self->client->pers.weapon->classnameb == WEAPON_SVT)
				{
					T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, 100, 50, 0,MOD_BAYONET);//faf
				}
				else if (self->client && self->client->pers.weapon &&
					self->client->pers.weapon->classnameb == WEAPON_CARCANO)
				{
					T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, 100, 50, 0,MOD_BAYONET);//faf
				}				else if (self->client && self->client->pers.weapon &&
					self->client->pers.weapon->classnameb == WEAPON_ARISAKA)
				{
					T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, 100, 50, 0,MOD_BAYONET);//faf
				}
				else //if (!OnSameTeam(tr.ent, self))
				{
					if (tr.ent && tr.ent->classnameb == SANDBAGS)
						damage*=25;

					T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage,  (fists)?200:0   , 0,(fists)?MOD_FISTS:MOD_KNIFE);//faf:  adding knockback for fists
					if (tr.ent->client &&
						tr.ent->client->aim)
					{
						//knock em out of truesight
						Cmd_Scope_f(tr.ent);
					}
				}
			//	else
			//		T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_NO_KNOCKBACK,(fists)?MOD_FISTS:MOD_KNIFE);
                //gi.sound (self, CHAN_AUTO, gi.soundindex((fists)?wav:"brain/melee3.wav") , 1, ATTN_NORM, 0); 

				if (self->client && !tr.ent->client && tr.ent->classnameb != OBJECTIVE_VIP &&  ( !(tr.ent->svflags & SVF_MONSTER) ))
				{
					Play_Bullet_Hit(self, tr.surface->name, tr.endpos, tr.ent);
					if ((!tr.ent->obj_owner || tr.ent->obj_owner != self->client->resp.team_on->index) && (rand() % 100) < 20)
					{
						if (tr.ent->classnameb != OBJECTIVE_VIP && !tr.ent->svflags & SVF_DEADMONSTER)
							ThrowDebris (self, "models/objects/debris2/tris.md2", 1, tr.endpos);
					}
				}
				else if (self->client && (tr.ent->client || tr.ent->classnameb== OBJECTIVE_VIP) && fists)
				{
					//gi.positioned_sound (tr.endpos, g_edicts, CHAN_AUTO, gi.soundindex("fists/punch.wav"), 1.0, ATTN_NORM, 0);
					gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/tnt/wall.wav"), 1, ATTN_NORM, 0);
				}


            }        
            else        
            {                
                gi.WriteByte (svc_temp_entity);    
                gi.WriteByte (TE_SPARKS);
                gi.WritePosition (tr.endpos);    
                gi.WriteDir (tr.plane.normal);
                gi.multicast (tr.endpos, MULTICAST_PVS);

                //gi.sound (self, CHAN_AUTO, gi.soundindex(wav) , 1, ATTN_NORM, 0);
				if (!tr.ent->client &&  ( !(tr.ent->svflags & SVF_MONSTER) ))
					Play_Bullet_Hit(self, tr.surface->name, tr.endpos, tr.ent);

            }    
        }
    }
    return;
} 




void Knife_Drop (edict_t *self)
{

	gitem_t *item = FindItem("Knife");
	edict_t         *dropped;



	if (self->velocity[0] != 0 ||
		self->velocity[1] != 0 ||
		self->velocity[2] != 0)
		{
			self->nextthink = level.time + .1;
			return;
		}
	


		dropped = G_Spawn();
		dropped->classname = item->classname;
		dropped->item = item;
        dropped->spawnflags = DROPPED_ITEM;
        dropped->s.effects = item->world_model_flags;
//faf        dropped->s.renderfx = RF_GLOW;
//        VectorSet (dropped->mins, -15, -15, -15);
  //      VectorSet (dropped->maxs, 15, 15, 15);
	VectorSet (dropped->mins, -5, -5, -1);
	VectorSet (dropped->maxs, 5, 5, 5);



        gi.setmodel (dropped, dropped->item->world_model);
        dropped->solid = SOLID_TRIGGER;
		dropped->movetype = MOVETYPE_STEP;//TOSS;  

        dropped->touch = Touch_Item;
        dropped->owner = self;
        dropped->gravity = 0;

//		safe_bprintf(PRINT_HIGH, "%i fsdfsd\n", other->health);

		
		VectorCopy (self->s.origin, dropped->s.origin);
		VectorCopy (self->s.angles, dropped->s.angles);

                        
        dropped->nextthink = level.time + 120;
        dropped->think = G_FreeEdict;

//		gi.sound (dropped, CHAN_WEAPON, gi.soundindex ("knife/hit.wav"), 1.0, ATTN_NORM, 0);

        gi.linkentity (dropped);

		G_FreeEdict(self);

}

int Play_Bullet_Hit(edict_t *ent, char *surface, vec3_t endpos, edict_t *impact_ent);


#define KNIFE_THROW_SPEED 950
#define HELMET_THROW_SPEED 600

qboolean Surface (char *name, int type);
void Blade_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean fistarmed = Q_stricmp(self->classname,"blade");
	gitem_t *item = FindItem("Knife");
	edict_t         *dropped;
    vec3_t          move_angles;//, origin;

	if (other == self->owner)
		return;



	if (surf && (surf->flags & SURF_SKY))
	{
//faf ??		Drop_Item (self, item);
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);


	if (self->dmg)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_KNIFE);
	}

	if (other->client && (other->health <= 0 ||
		other->health == 100))//helmet
	{
		Drop_Item (self, item);
		G_FreeEdict (self);
		return;
	}
	
	self->dmg = 0;//faf so it only damages on initial throw




	if (!other->client && other->inuse &&  ( !(other->svflags & SVF_MONSTER) )) //stick
	{

		if (surf && (Surface(surf->name, SURF_METAL) || Surface(surf->name,SURF_GLASS)))
		{
				Play_Bullet_Hit(self, surf->name, self->s.origin, other);
				self->think = Knife_Drop;
				self->nextthink = level.time + .1;
				//self->touch = Touch_Item;
				self->touch = NULL;
				VectorClear (self->avelocity);
				VectorClear (self->s.angles);
				self->movetype = MOVETYPE_STEP;


				//this shouldn't really happen for glass, but...
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_SPARKS);
				gi.WritePosition (self->s.origin);
				gi.WriteDir (vec3_origin);
				gi.multicast (self->s.origin, MULTICAST_PVS);



				return;

		}
	
		else
		{
			dropped = G_Spawn();
			dropped->classname = item->classname;
			dropped->item = item;
			dropped->spawnflags = DROPPED_ITEM;
			dropped->s.effects = item->world_model_flags;
	//faf        dropped->s.renderfx = RF_GLOW;
			VectorSet (dropped->mins, -15, -15, -15);
			VectorSet (dropped->maxs, 15, 15, 15);
			gi.setmodel (dropped, dropped->item->world_model);
			dropped->solid = SOLID_TRIGGER;
			dropped->movetype = MOVETYPE_TOSS;  
			dropped->touch = Touch_Item;
			dropped->owner = self;
			dropped->gravity = 0;


			vectoangles (self->velocity, move_angles);

			VectorCopy (self->s.origin, dropped->s.origin);
			VectorCopy (move_angles, dropped->s.angles);

			
			dropped->nextthink = level.time + 120;
			dropped->think = G_FreeEdict;
			gi.linkentity (dropped);	
			

			Play_Bullet_Hit(self, surf->name, dropped->s.origin, other);

		}

		

	}
	else //faf:  so knife doesnt float when it destroys an objective
	{
		self->think = Knife_Drop;
		self->nextthink = level.time + .1;
		return;
	}

/*    if ( !(self->waterlevel) )
	{

		gi.WriteByte (svc_temp_entity);    
		gi.WriteByte (TE_SPARKS);
		gi.WritePosition (self->s.origin);    
        gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
    }                                */

	G_FreeEdict (self);
}

void Helmet_Drop (edict_t *self)
{
	gitem_t *item = FindItem("Helmet");
	edict_t         *dropped;
//    vec3_t          move_angles, origin;

//	safe_bprintf (PRINT_HIGH, "%i xxx \n", self->count);



	if ((self->velocity[0] != 0 ||
		self->velocity[1] != 0 ||
		self->velocity[2] != 0) &&
		self->count <= 5)
	{
		self->nextthink = level.time + .1;
		return;
	}


		dropped = G_Spawn();
		dropped->classname = item->classname;
		dropped->item = item;
        dropped->spawnflags = DROPPED_ITEM;
        dropped->s.effects = item->world_model_flags;
//faf        dropped->s.renderfx = RF_GLOW;
        VectorSet (dropped->mins, -15, -15, -15);
        VectorSet (dropped->maxs, 15, 15, 15);
        gi.setmodel (dropped, dropped->item->world_model);
        dropped->solid = SOLID_TRIGGER;
		dropped->movetype = MOVETYPE_STEP;  
        dropped->touch = Touch_Item;
        dropped->owner = self;
	    dropped->gravity = 0;
		
		VectorCopy (self->s.origin, dropped->s.origin);
		VectorCopy (self->s.angles, dropped->s.angles);

		
        dropped->nextthink = level.time + 30;
        dropped->think = G_FreeEdict;
                   

		gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);

        gi.linkentity (dropped);

		G_FreeEdict (self);
}


void Helmet_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	gitem_t *item = FindItem("Helmet");

	self->count++;

	if (self->count > 5)
	{
		self->think = Helmet_Drop;
		self->nextthink = level.time + .1;
		return;
	}


//	if (other == self->owner)
//		return;

	if (surf && (surf->flags & SURF_SKY))
	{
//faf 		Drop_Item (self, item);
		G_FreeEdict (self);
		return;
	}


	self->movetype = MOVETYPE_STEP;


	VectorClear (self->avelocity);
	VectorClear (self->s.angles);


	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage && self->dmg)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_HELMET);
		gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/tnt/wall.wav"), 1, ATTN_NORM, 0);
		self->dmg = 0;
	}

	
	

	else if (self->dmg)// if ((!other->takedamage || !other->client) && (other != self))
	{
		if (random() > 0.5)
			gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
        else
		{
			gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		


	}

	self->think = Helmet_Drop;
	self->nextthink = level.time + .1;


/*	if(other->client) 
	{
		other->client->pers.inventory[ITEM_INDEX(item)]++;
		G_FreeEdict(self);
		return;
	}*/
	
	//Drop_Item (self, item);
//	G_FreeEdict (self);
}







void Knife_Throw (edict_t *self, vec3_t start, vec3_t dir, int damage)
{
	int effect=EF_ANIM01;
	edict_t	*blade;
	trace_t	tr;
	qboolean fistarmed = Q_stricmp(self->client->pers.weapon->pickup_name, "Knife");

	VectorNormalize (dir);

	blade = G_Spawn();
	VectorCopy (start, blade->s.origin);
	VectorCopy (start, blade->s.old_origin);
	vectoangles (dir, blade->s.angles);
	VectorScale (dir, (fistarmed)?HELMET_THROW_SPEED:KNIFE_THROW_SPEED, blade->velocity);
//faf	blade->movetype = MOVETYPE_STEP;
	blade->clipmask = MASK_SHOT;
	blade->solid = SOLID_BBOX;
//faf 	blade->s.effects |= effect;


	if (fistarmed)//faf: helmet
	{
		blade->movetype = MOVETYPE_TOSS;//STEP;
		VectorSet (blade->mins, -4, -4, 0);
		VectorSet (blade->maxs, 4, 4, 4);

		VectorSet (blade->avelocity, (500 - 1000 * random()), (500 - 1000 * random()), (500 - 1000 * random()));
	}
	else
	{
		VectorSet (blade->avelocity, -20000, 0, 0);
		blade->movetype = MOVETYPE_TOSS;
		VectorSet (blade->mins, -1, -1, -1);
		VectorSet (blade->maxs, 1, 1, 1);
//		VectorClear (blade->mins);
//		VectorClear (blade->maxs);
		//blade->s.sound = gi.soundindex("knife/spin.wav");
	}

	blade->s.modelindex = (fistarmed)?gi.modelindex ("models/weapons/g_helmet/tris.md2"):
									  gi.modelindex ("models/weapons/g_knife/tris.md2");
	blade->owner = self;
//faf: for separate helmet/knife death messages  	blade->touch = Blade_touch;
	blade->touch = (fistarmed)?Helmet_touch:Blade_touch;//faf

	blade->nextthink = level.time + 5;//faf 2;
	blade->think = G_FreeEdict;
	blade->dmg = damage;
	blade->s.frame = 0;
	
	if (fistarmed)
		blade->classname = "fists";
	else 
		blade->classname = "blade";

	blade->spawnflags = 1;
	gi.linkentity (blade);

//	if (self->client)
//		check_dodge (self, blade->s.origin, dir, KNIFE_THROW_SPEED);

	tr = gi.trace (self->s.origin, NULL, NULL, blade->s.origin, blade, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (blade->s.origin, -10, dir, blade->s.origin);
		blade->touch (blade, tr.ent, NULL, NULL);
	}
}


void Weapon_Knife_Fire (edict_t *ent)
{
	int knives;		//var to keep track of how many knives are left.
	int knife_index; //index to knife
	vec3_t  forward, right;
    vec3_t  start;
    vec3_t  offset;
	vec3_t g_offset;
	qboolean armedfists = Q_stricmp(ent->client->pers.weapon->pickup_name,"Knife");
//	gi.dprintf("armedfists: %s\n",(armedfists)?"True":"False");

	ent->client->ps.gunframe++;//faf



	knife_index = (armedfists)?ITEM_INDEX(FindItem("Helmet")):ITEM_INDEX(FindItem("Knife"));
	
	if(!armedfists && !ent->client->pers.inventory[knife_index]) 
	{
		// rezmoth - following line crashed the game [PBFIX]
		if (ent->client && ent->client->pers.weapon &&//faf
			ent->client->pers.weapon->guninfo)
			ent->client->ps.gunframe=ent->client->pers.weapon->guninfo->LastFire;
		ent->client->aim=false;
		return;
	}
	VectorCopy (vec3_origin,g_offset);

    AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorSet(offset, 24, 8, ent->viewheight-8);
    VectorAdd (offset, g_offset, offset);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    VectorScale (forward, -2, ent->client->kick_origin);
    ent->client->kick_angles[0] = -1;
 
	if(ent->client->aim)
	{
		if(ent->client->pers.inventory[knife_index])
		{
			start[2]+= 5;//faf
			Knife_Throw(ent, start,forward, (armedfists)?DAMAGE_HELMET:55);//DAMAGE_KNIFE);
			ent->client->pers.inventory[knife_index]--;
		}
		knives=ent->client->pers.inventory[knife_index];
		if(knives==0)
		{
			ent->client->aim=false;
			ent->client->weaponstate=WEAPON_LOWER;
			Use_Weapon (ent, FindItem("fists"));
		} 
				
	}
	else
	{ 
		//gi.dprintf("going to fire_knife\n");
		fire_Knife (ent, start, forward, (armedfists)?DAMAGE_FIST:DAMAGE_KNIFE, KNIFE_KICK, (armedfists)?"fists/hit.wav":"knife/hit.wav", armedfists);
		
		if (!(ent->s.frame >= 283 && ent->s.frame <= 294 ))
		{
			if (ent->client && ent->stanceflags == STANCE_STAND) 
			{
				ent->client->anim_priority = ANIM_ATTACK;
				ent->s.frame = 128;
				ent->client->anim_end = 133;// (FRAME_wave10);
			} //end faf
			else if (ent->stanceflags == STANCE_DUCK) 
			{
				ent->client->anim_priority = ANIM_ATTACK;
				ent->s.frame = 158;//(FRAME_crpain1);
				ent->client->anim_end = 160;// (FRAME_crpain4);
			} //end faf
			else if (ent->stanceflags == STANCE_CRAWL) 
			{
				ent->client->anim_priority = ANIM_ATTACK;
				ent->s.frame = 213;//(FRAME_crawlattck02);
				ent->client->anim_end = 215;// (FRAME_crawlattck05);
			} //end faf	
		}
	}
	
	ent->client->ps.gunframe++;
	Play_WepSound(ent, (armedfists)?"fists/fire.wav":"knife/fire.wav");
	//	gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
	PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
}


void Weapon_Turret_Fire(edict_t *ent);

void Weapon_Knife (edict_t *ent)
{
	static int      pause_frames[]  = {0};//{19, 32, 0};
    int				fire_frames[] = {7, 9};
	qboolean armedfists = Q_stricmp(ent->client->pers.weapon->pickup_name,"Knife");

	ent->client->crosshair = false;

	if (ent->client->turret)
	{
		if (ent->client->turret->count >= 0)
			ent->client->p_rnd= &ent->client->turret->count;
		Weapon_Turret_Fire(ent);
		return;
	}
	
	//ent->client->aim=false;
	//fire_frames[0]=(ent->client->aim)?54:4;
	ent->client->p_rnd=NULL;
	
	//faf

	if (!armedfists)
	{
		if (fast_knife->value)
		{
//		fire_frames[0]=(ent->client->aim)?57:5;//faf54:7;
		fire_frames[0]=(ent->client->aim)?54:5;
		}
		else
		fire_frames[0]=(ent->client->aim)?57:5;
	}
	else
		fire_frames[0]=(ent->client->aim)?54:5;

	if (armedfists)
		fire_frames[1] = 0;
	else
		fire_frames[1]= (ent->client->aim)?0:8;

	// Darwin's xtra knife sounds
	if(!armedfists && ent->client->ps.gunframe == 0)
		Play_WepSound(ent, "knife/pullout.wav");

	Weapon_Generic (ent, 3, 10, 45, 45,45,49,53,59,71, pause_frames, fire_frames, Weapon_Knife_Fire);
}




void Arty_Sound (edict_t *ent);
void Airstrike_Plane_Launch(edict_t *ent);

//was void Cmd_Arty_f (edict_t *ent)
void Binocular_Fire(edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward;
	vec3_t	end;
	vec3_t world_up;

	edict_t *airstrike;//faf



	trace_t	tr;
	//int randnum;		

	if (!IsValidPlayer(ent))
		return;


	if (ent->client->airstrike)
	{
		safe_cprintf(ent, PRINT_HIGH, "Airstrike cancelled sir!\n");

		G_FreeEdict(ent->client->airstrike);
		ent->client->airstrike = NULL;
		return;
	}


	

	if ( ent->client->arty_time_restrict > level.time)//faf && ent->client->arty_num >= (int)arty_max->value)
	{
		int delay;

		delay = ((int)(ent->client->arty_time_restrict - level.time) +1);

		if (delay == 0)
			safe_cprintf(ent, PRINT_HIGH, "Can not call airstrike for another 1 second, sir!\n");
		else
			safe_cprintf(ent, PRINT_HIGH, "Can not call airstrike for another %i seconds, sir!\n", delay);

		return;
	}

	// reset the fired counter if past restrict time
//	if ( ent->client->arty_time_restrict <= level.time)//faf && ent->client->arty_num >= (int)arty_max->value )
//		ent->client->arty_num = 0;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

	if ( tr.surface && !(tr.surface->flags & SURF_SKY) )
	{ // We hit something but it wasn't sky, so let's see if there is sky above it

		VectorCopy(tr.endpos,ent->client->arty_target); //assign target to Arty
		VectorSet(world_up, 0, 0, 1);
		VectorMA(start, 8192, world_up, end);

		tr = gi.trace(ent->client->arty_target, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

		if ( tr.surface && !(tr.surface->flags & SURF_SKY))  // No sky above it either
		{
			//safe_cprintf(ent, PRINT_HIGH, "Sir! Airstrikes must be fired in open areas at stationary grounds.\n");
				//return;
			
			//this spot is not under the sky... try moving around a bunch of spots to find a nearby spot that is
			{
				ent->client->arty_target[0]+=60;
				VectorMA(start, 8192, world_up, end);
				tr = gi.trace(ent->client->arty_target, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);
			
				if ( tr.surface && !(tr.surface->flags & SURF_SKY))
				{
					ent->client->arty_target[0]-=120;
					VectorMA(start, 8192, world_up, end);
					tr = gi.trace(ent->client->arty_target, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

  					if ( tr.surface && !(tr.surface->flags & SURF_SKY))
					{
						ent->client->arty_target[0]+=60;
						ent->client->arty_target[1]+=60;
						VectorMA(start, 8192, world_up, end);
						tr = gi.trace(ent->client->arty_target, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);
  						if ( tr.surface && !(tr.surface->flags & SURF_SKY))
						{
							ent->client->arty_target[1]-=120;
							VectorMA(start, 8192, world_up, end);
							tr = gi.trace(ent->client->arty_target, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);
							
							if ( tr.surface && !(tr.surface->flags & SURF_SKY))
							{
								safe_cprintf(ent, PRINT_HIGH, "Can't find a target for airstrike, Sir!\n");
								return;
							}
						}
					}
				}

			}
		}
	}
	else  //faf:  this stops horizontal airstrikes
	{
		safe_cprintf(ent, PRINT_HIGH, "Sir! Airstrikes must be fired in open areas at stationary grounds.\n");
			return;
	}


	// set up for the arty strike
	VectorCopy(tr.endpos, ent->client->arty_entry);
	
		
	//randnum = ((rand() % ARTILLARY_WAIT) + 5);  //generate random number for eta

	if (ent)
		safe_cprintf(ent, PRINT_HIGH, "Ok, give us %d seconds to reach the target!\n", (int)arty_delay->value);




/*				check_unscope(ent);//faf
				
				ent->client->ps.fov = STANDARD_FOV;

				ent->client->aim = false;

				ent->client->ps.gunframe = 8;
*/




//	ent->client->arty_num++;
//	ent->client->arty_time_fire = level.time + arty_delay->value;

//	ent->client->arty_location = 1;//(rand() % 4) + 1;
	//gi.sound(ent, CHAN_ITEM, gi.soundindex(va("%s/arty/target%i.wav", ent->client->resp.team_on->teamid, 1)), 1, ATTN_NORM, 0);

	gi.positioned_sound (ent->s.origin, g_edicts, CHAN_AUTO, gi.soundindex(va("%s/arty/target%i.wav", ent->client->resp.team_on->teamid, 1)), 1.0, ATTN_NORM, 0);


	//faf:  so we can get rid of the arty stuff in clientthink



		airstrike = G_Spawn();
		gi.linkentity (airstrike);

		VectorCopy(tr.endpos, airstrike->pos2);
		airstrike->classname ="airstrike_called";

		airstrike->owner = ent;

		if (airstrikes->value == 1)
		{
			airstrike->think = Airstrike_Plane_Launch;

			ent->client->airstrike = airstrike;

			if (arty_delay->value > 2.1)
			{
				airstrike->nextthink = level.time + arty_delay->value - 2;
			}
			else
				airstrike->nextthink = level.time +.1;
		}
		else//ddaylife
		{
			airstrike->think = Arty_Sound;
			
			ent->client->airstrike = airstrike;

			if (arty_delay->value > 3.1)
			{
				airstrike->nextthink = level.time + arty_delay->value - 3;
			}
			else
				airstrike->nextthink = level.time +.1;
		}

}







void Weapon_Binoculars_Look(edict_t *ent)
{
}

void Weapon_Binoculars(edict_t *ent)
{
//gi.dprintf ("%i\n",			ent->client->ps.gunframe);

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe< 3)
		{
			ent->client->ps.gunframe++;
			return;
		}
		else 
		{
			ent->client->ps.gunframe = 8;
			ent->client->weaponstate = WEAPON_READY;
			return;
		}
	}

	if (ent->client->weaponstate != WEAPON_DROPPING)
	{
		if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
		{
			ent->client->weaponstate = WEAPON_DROPPING;
			ent->client->ps.gunframe = 47;
		}
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->drop || ent->client->ps.gunframe == 51)
		{
			ent->client->drop = false;
			ent->client->aim  = false;
			ChangeWeapon (ent);
			return;
		}		
		ent->client->ps.gunframe++;
		return;
	}
	

	//idle
	if (ent->client->ps.gunframe >7 &&
		ent->client->ps.gunframe < 48)
		ent->client->ps.gunframe++;
	
	if (ent->client->ps.gunframe == 48)
	{
		ent->client->ps.gunframe = 8;
	}

	if (ent->client->aim == true)
	{
		ent->client->crosshair = true;
		ent->client->ps.fov = SCOPE_FOV;
		ent->client->ps.gunframe = 6;
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->last_fire_time < level.time - .5)
					Binocular_Fire(ent);
				ent->client->last_fire_time = level.time;

			}
	}
	else
	{
		ent->client->crosshair = false;
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) 
				&& 	(ent->client->last_fire_time < level.time -.5))
			{
				ent->client->last_fire_time = level.time;
					if (ent->client->airstrike)
					{
						safe_cprintf(ent, PRINT_HIGH, "Airstrike cancelled sir!\n");

						G_FreeEdict(ent->client->airstrike);
						ent->client->airstrike = NULL;
						check_unscope(ent);//faf
						ent->client->ps.fov = STANDARD_FOV;
						return;
					}
					else
					{
						safe_cprintf(ent, PRINT_HIGH, "Press the aim button to aim the binoculars!\n");
						ent->client->latched_buttons = 0;
					}
			}

			check_unscope(ent);//faf
				
			ent->client->ps.fov = STANDARD_FOV;
	}

	

}


void Weapon_Antidote_Use(edict_t *ent)
{
	//find the other entity, check for chemical exposer, fix it
	return;
}


void Weapon_Antidote(edict_t *ent)
{
	static int		pause_frames[] = {19,32,0};
	static int		fire_frames[] = {6};

	Weapon_Generic(ent,3,8,48,52,48,48,0,0,0,pause_frames,fire_frames,Weapon_Antidote_Use);
}

void Weapon_Morphine_Use(edict_t *ent)
{
//bcass start - medic sound thing
	int			randnum;
//bcass end
	edict_t		*target;
	qboolean	full_recovery;	// pbowens: healing rewards

	ent->client->ps.gunframe++;

	if (ent->client->aim)
		target = ent;
	else 
	{
		if (!(target=ApplyFirstAid(ent)))
			return;

		if (!target->client)
			return;
	}
	
	if (target->burnout > level.time) // they are on fire
		return;

	if (target->waterlevel == 3) // they are under water
		return;

	target->wound_location	= 0;
	target->die_time		= 0;

	if (target->health < HEALTH_MAX && target->health > 0) {

		// pbowens: do the new medic thang
		if (target == ent)
		{
			ent->health += (7 + crandom()); // just increment self 7+r health points	

			
			if (ent->ai && ent->health >= 100)
			{
				ent->ai->reached_obj_time = 0;
				if (ent->ai->camp_targ > -1)
					camp_spots[ent->ai->camp_targ].owner = NULL;
				ent->ai->camp_targ = -2;
				ent->ai->objective = NULL;
				ent->movetarget = NULL;
				ent->goalentity = NULL;
				ent->enemy = NULL;
				AI_PickLongRangeGoal(ent);
				AI_ResetWeights(ent);
				AI_ResetNavigation(ent);
			}



		}
		else 
		{
			full_recovery = ((HEALTH_MAX - target->health) >= HEALTH_INCREMENT) ? true : false;
			target->health += (full_recovery) ? (HEALTH_MAX - target->health) : HEALTH_INCREMENT;

			if (full_recovery && target != ent  && team_kill->value == 0 && target->wound_location ) {
				safe_cprintf(target, PRINT_HIGH, "You have been fully recovered by %s.\n", ent->client->pers.netname);
				safe_centerprintf(ent, "You have fully recovered %s.\n", target->client->pers.netname );

				// give medic 1 'frag' for scoreboard sorting and deduct 1 kill from other team
				ent->client->resp.score++;

				if (target->client->resp.team_on == ent->client->resp.team_on)
					team_list[(ent->client->resp.team_on->index) ? 0 : 1]->kills--;
			} 
			else
			{
				safe_cprintf(target, PRINT_HIGH, "You were patched up by %s.\n", ent->client->pers.netname);
				safe_cprintf(ent, PRINT_HIGH, "You patched up %s.\n", target->client->pers.netname);
				if (target->ai)
					target->ai->med_heal_time = level.time;

				if (ent->ai)
				{
					ent->ai->reached_obj_time = 0;

					if (ent->ai->camp_targ > -1)
						camp_spots[ent->ai->camp_targ].owner = NULL;
					ent->ai->camp_targ = -2;

					ent->ai->objective = NULL;
					ent->movetarget = NULL;
					ent->goalentity = NULL;
					ent->enemy = NULL;
					AI_ResetWeights(ent);
					AI_ResetNavigation(ent);
					AI_PickLongRangeGoal(ent);

				}

			}
		}

//bcass start - medic sound thing
		srand(rand());
		randnum=rand()%100;
		
		//let the fun begin defining sounds
		if(randnum > MEDIC1 && randnum < MEDIC2)
		{
			gi.sound (target, CHAN_WEAPON, gi.soundindex ("items/morphine1.wav"), 1.0, ATTN_NORM, 0);
		}
		else if (randnum > MEDIC2 && randnum < MEDIC3)
		{
			gi.sound (target, CHAN_WEAPON, gi.soundindex ("items/morphine2.wav"), 1.0, ATTN_NORM, 0);
		}
		else if (randnum > MEDIC3 && randnum < MEDICH)
		{
			gi.sound (target, CHAN_WEAPON, gi.soundindex ("items/morphine3.wav"), 1.0, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (target, CHAN_WEAPON, gi.soundindex ("items/l_health.wav"), 1.0, ATTN_NORM, 0);
		}
//bcass end		
//			gi.sound (ent, CHAN_WEAPON, gi.soundindex ("items/l_health.wav"), 1.0, ATTN_NORM, 0);

		if (target->health > HEALTH_MAX)
			target->health = HEALTH_MAX;

		//faf
		if (target->client &&
			target->health == HEALTH_MAX)
			target->client->last_wound_inflictor = NULL;
		

//		ClientSetMaxSpeed(target, true);
		WeighPlayer(ent);
	}
}


void Weapon_Morphine(edict_t *ent)
{
	static int		pause_frames[] = {0};//{19,32,0};
	static int		fire_frames[2];

	fire_frames[0] = (ent->client->aim)?53:4;
	fire_frames[1] = (ent->client->aim)?53:5;
	fire_frames[2] = (ent->client->aim)?53:6;

	ent->client->crosshair = true;

	Weapon_Generic(ent,
		3,  10, 45, 
		45, 45, 49, 
		52, 55, 66, 
		pause_frames,fire_frames,Weapon_Morphine_Use);
}

//6 minutes for bandage
#define BANDAGE_TIME 360
void Weapon_Bandage_Use(edict_t *ent)
{
	edict_t *target;
	ent->client->ps.gunframe++;
	if ( !(target = ApplyFirstAid(ent)) )
		return;
	if (target->wound_location & (CHEST_WOUND|STOMACH_WOUND))
	{
		if(target->die_time) target->die_time+=(BANDAGE_TIME);
		else target->die_time=BANDAGE_TIME+level.time;
		target->wound_location= target->wound_location & ~(CHEST_WOUND|STOMACH_WOUND);
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Bandage"))]--;
		WeighPlayer(target);
	}
	else 
	{

		ent->client->pers.inventory[ITEM_INDEX(FindItem("Bandage"))]--;
		target->client->pers.inventory[ITEM_INDEX(FindItem("Bandage"))]++;
	}
	//Find the other entity, check for leg wound, set arty_strike_time(?) to wear off,
	//remove wound modifier from speed multiplyer,
	//then check in client_think where die time is for arty_strike_time, then remultiply the 
	//speed modifier.
	return;

}


void Weapon_Bandage(edict_t *ent)
{
	static int		pause_frames[] = {19,32,0};
	static int		fire_frames[] = {6};

	Weapon_Generic(ent,3,8,48,52,48,48,0,0,0,pause_frames,fire_frames,Weapon_Bandage_Use);
}


#define	MZ_FLAMER			45

/*
======================================================================

Flamethrower

======================================================================
*/

void weapon_flame_fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	vec3_t	direct_damage = {1, 1, 1}; //6,9,50
	vec3_t	radius_damage = {1, 1, 1}; //6,4,25
    vec3_t  spread = {5, 5, 0}; //10,20,0
	int     speed;

	//gi.dprintf("flamer gunframe: %i\n", ent->client->ps.gunframe);

	if(!((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		//ent->client->ps.gunframe = 6;
		ent->client->ps.gunframe++;
		return;
	}
	
	if (!ent->client->flame_rnd)
	{
		 ent->client->ps.gunframe = 6; //last fire frame
		 if (level.time >= ent->pain_debounce_time)
		 {
             gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
             ent->pain_debounce_time = level.time + 1;
		 }
//Make the user change weapons MANUALLY!
	
		//ifchangewep(ent);
		return;
	}


	AngleVectors (ent->client->v_angle, forward, right, NULL);
//    VectorSet(offset, 48, 8, ent->viewheight-8);//faf
    VectorSet(offset, 0, 8, ent->viewheight-8);

//	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

                
/* Set flamethrower shot speed. */
	//pbowens: 1/2'd everything
	//250 / 500 / 500
	//375 / 900 / 900
/*	speed = 250 + (ent->client->machinegun_shots * 150);
	if (speed >= 200)
	{
		speed = 200;
        ent->client->machinegun_shots = 0;
	}
	else
		ent->client->machinegun_shots++;
*/

	speed = 300;
/*--------------------------------------------------*/
/*  Currently, the flamethrower shoots two shots.   */
/*  This is NOT speed-friendly, and can cause some  */
/*  lag.  It is better for DM to make the flamer    */
/*  shoot one shot with double radius damage.       */
/*--------------------------------------------------*/
//	ptrgi->dprintf("%i...",i);
//	if (ent->client->ps.gunframe == 4 || ent->client->ps.gunframe == 5) {





       gi.sound (ent, CHAN_AUTO, gi.soundindex("weapons/flamer/fire.wav") , 1, ATTN_NORM, 0); 
	   PBM_FireFlameThrower (ent, start, spread, forward, speed, direct_damage, radius_damage, 10, 0);
	
	   ent->client->flame_rnd--;

	  // send muzzle flash
	   gi.WriteByte (svc_muzzleflash);
	   gi.WriteShort (ent-g_edicts);
	   gi.WriteByte (MZ_FLAMER);
       gi.multicast (ent->s.origin, MULTICAST_PVS);

	   PlayerNoise(ent, start, PNOISE_WEAPON);
//	}

//	ent->client->ps.gunframe++;
}

void Weapon_Flamethrower (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {4, 5};
	
	ent->client->aim=false;
	ent->client->p_rnd= &ent->client->flame_rnd;

	ent->client->crosshair = false;

	Weapon_Generic (ent, 3, 5, 28, 37,37,46,0,0,0, pause_frames, fire_frames, weapon_flame_fire);
}

//bcass start - TNT
#define TNT_TIMER		15.0
#define TNT_MINSPEED	400
#define TNT_MAXSPEED	800

void weapon_tnt_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 225;
	int		speed;
	float	radius;

	// unused
	float	time = 0;
	int		team = 0;

	radius = damage + 40;
	VectorSet(offset, 16, -16, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (ent->client->ps.pmove.pm_type == PM_DEAD)
		speed = 5; // drop the grenade
	else
		speed = TNT_MINSPEED + (int)(-(ent->client->tnt->nextthink - level.time) + 2.75) * ((TNT_MAXSPEED - TNT_MINSPEED) / TNT_TIMER);
	//gi.dprintf("speed: %i\n", speed);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	fire_tnt (ent, start, forward, damage, speed, time, radius, team);

//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;

//	ent->client->grenade_time = level.time + 1.0;
}

void weapon_tnt_prime (edict_t *ent, int team)
{
//	vec3_t	offset;
//	vec3_t	forward, right;
//	vec3_t	start;
	int		damage;
//	int		speed;
//	int		team;
	float	radius;
	
	edict_t	*tnt;

	damage = 1500;
	radius = 550;

	tnt = G_Spawn();

	tnt->movetype = MOVETYPE_BOUNCE;
	tnt->clipmask = MASK_SHOT;
	tnt->solid = SOLID_TRIGGER;
	//		grenade->s.effects |= EF_GRENADE;
		
	//VectorClear (tnt->mins);
	//VectorClear (tnt->maxs);
	
	VectorSet (tnt->mins, -5, -5, -5);
	VectorSet (tnt->maxs, 5, 5, 5);
	//grenade->s.modelindex = gi.modelindex (va("models/objects/%s/tris.md2", (ent->client->resp.team_on->index) ? "masher" : "grenade2"));
	
	tnt->owner = ent;
	tnt->nextthink = level.time + .1;
	tnt->think = TNT_Think;
	tnt->delay = level.time + 15;
	tnt->dmg = damage;
	tnt->dmg_radius = radius;
	tnt->classname = "tnt";
	tnt->classnameb = TNT;
	tnt->obj_owner = team;
	tnt->s.sound = gi.soundindex ("weapons/tnt/fizz.wav");
	
	//	gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/tnt/fizz1.wav"), 1, ATTN_NORM, 0);
/*
	if (IsValidPlayer(ent) && ent->client->resp.team_on->mos[ent->client->resp.mos]->grenades)
		tnt->item = FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->grenades);
	else
		tnt->item = FindItem(va("%s", (team) ? "Stielhandgranate" : "Mk 2 Grenade" ));
*/	
	//grenade->touch = Shrapnel_Touch;

	tnt->spawnflags = 1;

	gi.linkentity (tnt);

	ent->client->tnt = tnt;

//	ent->client->grenade_time = level.time + 1.0;
}


void Weapon_TNT (edict_t *ent)
{
	if(!ent->client->pers.inventory[ent->client->ammo_index])
		Cmd_WeapNext_f (ent);

	ent->client->crosshair = false;

	if (ent->client->aim)
		ent->client->aim = false;

	if (ent->client->pers.weapon &&
		ent->client->pers.weapon->pickup_name &&
		frame_output)
		gi.dprintf("%i / %i - %s\n", ent->client->weaponstate, ent->client->ps.gunframe, ent->client->pers.weapon->pickup_name);


	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_RAISE || 
		ent->client->weaponstate == WEAPON_LOWER )	{
		ent->client->weaponstate =  WEAPON_READY;
		ent->client->ps.gunframe = (ent->client->tnt) ? 10 : 18;
	}


	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/tnt/pullout.wav"), 1, ATTN_NORM, 0);
		ent->client->ps.gunframe = 1;
		ent->client->weaponstate =  WEAPON_READY;
		ent->client->ps.gunframe = 52;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			/*
				// dont work in water, jimmy
				if (ent->waterlevel == 3)
				{
					ent->client->weaponstate = WEAPON_READY;
					//ent->client->ps.gunframe = 1;
					return;
				}
			*/

			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				// You don't pull a pin from a live grenade
				ent->client->ps.gunframe = (ent->client->tnt) ? 11 : 1;
				ent->client->weaponstate = WEAPON_FIRING;
//				if (!ent->client->grenade) 
//					ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				/*if(auto_weapon_change->value)*/
					//NoAmmoWeaponChange (ent);
			}
			return;
		}

		if (ent->client->tnt) { // No "idle" frames if it's active
			ent->client->ps.gunframe = 10;
		}
		else
		{

			if ( (ent->client->ps.gunframe == 29) || 
				 (ent->client->ps.gunframe == 41) || 
				 (ent->client->ps.gunframe == 19) || 
				 (ent->client->ps.gunframe == 48) )
			{
				if (rand()&15)
					return;
			}

			ent->client->ps.gunframe++;

			if (ent->client->ps.gunframe >= 51)
				ent->client->ps.gunframe = 18;

			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		// Pull the pin, and prime the grenade
		if (ent->client->ps.gunframe == 1)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/tnt/light.wav"), 1, ATTN_NORM, 0);
		
		if (ent->client->ps.gunframe == 10 && !ent->client->tnt)
		{
			weapon_tnt_prime(ent, ent->client->resp.team_on->index);
		}

		// Let the player hold the grenade mid-firing
		if (ent->client->ps.gunframe == 13)
			if (ent->client->buttons & BUTTON_ATTACK)
				return;

		// Throw it
		if (ent->client->ps.gunframe == 14)
		{
			weapon_tnt_fire (ent);
//			ent->client->grenade = NULL;
		}

		if (ent->client->ps.gunframe == 15) // throw the grenade
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/tnt/toss.wav"), 1, ATTN_NORM, 0);
		
//		if ((ent->client->ps.gunframe == 17))
//			return;

		if (ent->client->ps.gunframe >= 17)
		{
//			ent->client->grenade_time = 0;
//			ent->client->ps.gunframe = 10;
			ent->client->weaponstate = WEAPON_READY;
			return;
		}

		ent->client->ps.gunframe++;

	}
}
//bcass end

void nevergethere(edict_t *ent) { return; }
