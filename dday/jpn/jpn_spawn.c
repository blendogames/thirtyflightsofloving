/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/jpn/jpn_spawn.c,v $
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

#include "jpn_main.h"

// g_spawn.c
// D-Day: Normandy Team Spawns
gitem_t	*FindItemByClassname (char *classname);
void SpawnItem (edict_t *ent, gitem_t *item);
gitem_t	*FindItem (char *pickup_name);

//no fancy spawning stuff here, just nice and easy

void SP_item_weapon_nambu(edict_t *self)

{
    SpawnItem(self,FindItem("weapon_nambu"));
}

void SP_item_weapon_arisaka(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_arisaka"));
}

void SP_item_weapon_type_100(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_type_100"));
}

void SP_item_weapon_type_99(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_type_99"));
}

//void SP_item_weapon_m1carb(edict_t *self)
//{
 //   SpawnItem(self,FindItemByClassname("weapon_m1carb"));
//}


void SP_item_weapon_arisakas(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_arisakas"));
}
void SP_item_weapon_katana(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_katana"));
}

//ammo
void SP_item_molotov(edict_t *self);

void SP_item_ammo_grenades_jpn(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_grenades_jpn"));
}
void SP_item_ammo_nambu(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_nambu"));
}

void SP_item_ammo_arisaka(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_arisaka"));
}
void SP_item_ammo_arisakas(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_arisakas"));
}

void SP_item_ammo_type_100(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_type_100"));
}

void SP_item_ammo_type_99(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_type_99"));
}
void SP_item_weapon_mg42(edict_t *self);

void SP_item_ammo_mg42(edict_t *self);

//void SP_item_ammo_m1carb(edict_t *self)
//{
//    SpawnItem(self,FindItemByClassname("ammo_m1carb"));
//}



spawn_t sp_jpn[MAX_TEAM_ITEMS] =

{
    "weapon_nambu", SP_item_weapon_nambu,
	"weapon_arisaka",SP_item_weapon_arisaka,
	"weapon_type_100", SP_item_weapon_type_100,
	"weapon_type_99", SP_item_weapon_type_99,
//	"weapon_m1carb", SP_item_weapon_m1carb,
	"weapon_arisakas",SP_item_weapon_arisakas,
    "weapon_katana", SP_item_weapon_katana,
	"weapon_molotov", SP_item_molotov,
	"weapon_mg42", SP_item_weapon_mg42,
	"ammo_nambu",SP_item_ammo_nambu,
	"ammo_arisaka",SP_item_ammo_arisaka,
	"ammo_arisakas",SP_item_ammo_arisakas,
	"ammo_type_100",SP_item_ammo_type_100,
	"ammo_type_99",SP_item_ammo_type_99,
//	"ammo_m1carb",SP_item_ammo_m1carb,
	"ammo_grenades_jpn", SP_item_ammo_grenades_jpn,
	"ammo_mg42",SP_item_ammo_mg42

};


