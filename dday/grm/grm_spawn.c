/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/grm/grm_spawn.c,v $
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

#include "grm_main.h"

// g_spawn.c
// D-Day: Normandy Team Spawns

//no fancy spawning stuff here, just nice and easy

void SP_item_weapon_p38(edict_t *self)

{
    SpawnItem(self,FindItem("weapon_p38"));
}

void SP_item_weapon_mauser98k(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_mauser98k"));
}

void SP_item_weapon_mp40(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_mp40"));
}

void SP_item_weapon_mp43(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_mp43"));
}

void SP_item_weapon_mg42(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_mg42"));
}

void SP_item_weapon_panzer(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_panzer"));
}

void SP_item_weapon_mauser98ks(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_mauser98ks"));
}

//ammo

void SP_item_ammo_grenades_grm(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_grenades_grm"));
}
void SP_item_ammo_p38(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_p38"));
}

void SP_item_ammo_mauser98k(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_mauser98k"));
}

void SP_item_ammo_mp40(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_mp40"));
}

void SP_item_ammo_mp43(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_mp43"));
}

void SP_item_ammo_mg42(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_mg42"));
}

void SP_item_ammo_rocketsG(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_rocketsG"));
}



spawn_t sp_grm[MAX_TEAM_ITEMS] =

{
    "weapon_p38", SP_item_weapon_p38,
	"weapon_mauser98k",SP_item_weapon_mauser98k,
	"weapon_mp40", SP_item_weapon_mp40,
	"weapon_mp43", SP_item_weapon_mp43,
	"weapon_mg42", SP_item_weapon_mg42,
	"weapon_panzer",SP_item_weapon_panzer,
	"weapon_mauser98ks",SP_item_weapon_mauser98ks,
	"ammo_p38",SP_item_ammo_p38,
	"ammo_mauser98k",SP_item_ammo_mauser98k,
	"ammo_mp40",SP_item_ammo_mp40,
	"ammo_mp43",SP_item_ammo_mp43,
	"ammo_mg42",SP_item_ammo_mg42,
	"ammo_rocketsG",SP_item_ammo_rocketsG,
	"ammo_grenades_grm", SP_item_ammo_grenades_grm
};


