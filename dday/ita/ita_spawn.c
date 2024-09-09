/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/ita/ita_spawn.c,v $
 *   $Revision: 1.5 $
 *   $Date: 2002/06/04 19:49:50 $
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

#include "ita_main.h"

// g_spawn.c
// D-Day: Normandy Team Spawns

//no fancy spawning stuff here, just nice and easy
gitem_t	*FindItemByClassname (char *classname);
void SpawnItem (edict_t *ent, gitem_t *item);
gitem_t	*FindItem (char *pickup_name);

void SP_item_weapon_b34(edict_t *self)

{
    SpawnItem(self,FindItem("weapon_b34"));
}

void SP_item_weapon_carcano(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_carcano"));
}

void SP_item_weapon_b38(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_b38"));
}

void SP_item_weapon_k43(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_k43"));
}

void SP_item_weapon_b3842(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_b3842"));
}

void SP_item_weapon_panzer(edict_t *self);

void SP_item_weapon_m98ks(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_m98ks"));
}
//ammo

void SP_item_ammo_grenades_ita(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_grenades_ita"));
}
void SP_item_ammo_b34(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_b34"));
}

void SP_item_ammo_carcano(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_carcano"));
}

void SP_item_ammo_b38(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_b38"));
}

void SP_item_ammo_k43(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_k43"));
}

void SP_item_ammo_b3842(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_b3842"));
}

void SP_item_ammo_rocketsI(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_rocketsI"));
}

void SP_item_ammo_mauser98k(edict_t *self);

void SP_item_weapon_breda(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_breda"));
}
void SP_item_ammo_breda(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_breda"));
}




spawn_t sp_ita[MAX_TEAM_ITEMS] =

{
    "weapon_b34", SP_item_weapon_b34,
	"weapon_carcano",SP_item_weapon_carcano,
	"weapon_b38", SP_item_weapon_b38,
	"weapon_k43", SP_item_weapon_k43,
	"weapon_b3842", SP_item_weapon_b3842,
	"weapon_panzerfaust",SP_item_weapon_panzer,
	"weapon_m98ks",SP_item_weapon_m98ks,
	"weapon_breda", SP_item_weapon_breda,
	"ammo_b34",SP_item_ammo_b34,
	"ammo_carcano",SP_item_ammo_carcano,
	"ammo_b38",SP_item_ammo_b38,
	"ammo_k43",SP_item_ammo_k43,
	"ammo_b3842",SP_item_ammo_b3842,
	"ammo_rocketsI",SP_item_ammo_rocketsI,
	"ammo_mauser98k",SP_item_ammo_mauser98k,
	"ammo_grenades_ita", SP_item_ammo_grenades_ita,
	"ammo_breda",SP_item_ammo_breda
};
