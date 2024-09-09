/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/usa/usa_spawn.c,v $
 *   $Revision: 1.5 $
 *   $Date: 2002/06/04 19:49:51 $
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

#include "usm_main.h"


// g_spawn.c
// D-Day: Normandy Team Spawns

gitem_t	*FindItemByClassname (char *classname);
void SpawnItem (edict_t *ent, gitem_t *item);
gitem_t	*FindItem (char *pickup_name);

//no fancy spawning stuff here, just nice and easy

void SP_item_weapon_colt45(edict_t *self);

void SP_item_weapon_m1(edict_t *self);

void SP_item_weapon_shotgun(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_shotgun"));
}

void SP_item_weapon_BAR(edict_t *self);

void SP_item_weapon_30cal(edict_t *self);

void SP_item_weapon_bazooka(edict_t *self);

void SP_item_weapon_sniper(edict_t *self);

void SP_item_weapon_thompson(edict_t *self);



//ammo
void SP_item_ammo_grenades_usa(edict_t *self);
void SP_item_ammo_colt45(edict_t *self);

void SP_item_ammo_shotgun(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_shotgun"));
}
void SP_item_ammo_BAR(edict_t *self);

void SP_item_ammo_rockets(edict_t *self);

void SP_item_ammo_m1(edict_t *self);

void SP_item_ammo_m1903(edict_t *self);

void SP_item_ammo_HMG(edict_t *self);
void SP_item_ammo_thompson(edict_t *self);

// Misc stuff


void SP_misc_banner_usa (edict_t *ent);




/////////////////////////////////////////////////////////////

spawn_t sp_usm[MAX_TEAM_ITEMS] =

{
    "weapon_colt45", SP_item_weapon_colt45,
	"weapon_m1",SP_item_weapon_m1,
	"weapon_shotgun", SP_item_weapon_shotgun,
	"weapon_BAR", SP_item_weapon_BAR,
	"weapon_30cal", SP_item_weapon_30cal,
	"weapon_bazooka",SP_item_weapon_bazooka,
	"weapon_sniper",SP_item_weapon_sniper,
	"weapon_thompson", SP_item_weapon_thompson,
	"ammo_grenades_usa",SP_item_ammo_grenades_usa,
	"ammo_colt45",SP_item_ammo_colt45,
	"ammo_m1",SP_item_ammo_m1,
	"ammo_shotgun",SP_item_ammo_shotgun,
	"ammo_BAR",SP_item_ammo_BAR,
	"ammo_HMG",SP_item_ammo_HMG,
	"ammo_rockets",SP_item_ammo_rockets,
	"ammo_m1903",SP_item_ammo_m1903,
	"ammo_thompson",SP_item_ammo_thompson,
	"misc_banner_usa",SP_misc_banner_usa
};
