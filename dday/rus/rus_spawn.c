/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/rus/rus_spawn.c,v $
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

#include "rus_main.h"

// g_spawn.c
// D-Day: Normandy Team Spawns

//no fancy spawning stuff here, just nice and easy
gitem_t	*FindItemByClassname (char *classname);
void SpawnItem (edict_t *ent, gitem_t *item);
gitem_t	*FindItem (char *pickup_name);

void SP_item_weapon_tt33(edict_t *self)

{
    SpawnItem(self,FindItem("weapon_tt33"));
}

void SP_item_weapon_m9130(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_m9130"));
}

void SP_item_weapon_ppsh41(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_ppsh41"));
}

void SP_item_weapon_pps43(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_pps43"));
}

void SP_item_weapon_dpm(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_dpm"));
}

void SP_item_weapon_rpg1(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_rpg1"));
}

void SP_item_weapon_m9130s(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_m9130s"));
}

//ammo

void SP_item_ammo_grenades_rus(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_grenades_rus"));
}
void SP_item_ammo_tt33(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_tt33"));
}

void SP_item_ammo_m9130(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_m9130"));
}

void SP_item_ammo_ppsh41(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_ppsh41"));
}

void SP_item_ammo_pps43(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_pps43"));
}

void SP_item_ammo_dpm(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_dpm"));
}

void SP_item_ammo_rocketsR(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_rocketsR"));
}

void SP_item_molotov(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_molotov"));
}


spawn_t sp_rus[MAX_TEAM_ITEMS] =

{
    "weapon_tt33", SP_item_weapon_tt33,
	"weapon_m9130",SP_item_weapon_m9130,
	"weapon_ppsh41", SP_item_weapon_ppsh41,
	"weapon_pps43", SP_item_weapon_pps43,
	"weapon_dpm", SP_item_weapon_dpm,
	"weapon_rpg1",SP_item_weapon_rpg1,
	"weapon_m9130s",SP_item_weapon_m9130s,
	"weapon_molotov", SP_item_molotov,
	"ammo_tt33",SP_item_ammo_tt33,
	"ammo_m9130",SP_item_ammo_m9130,
	"ammo_ppsh41",SP_item_ammo_ppsh41,
	"ammo_pps43",SP_item_ammo_pps43,
	"ammo_dpm",SP_item_ammo_dpm,
	"ammo_rocketsR",SP_item_ammo_rocketsR,
	"ammo_grenades_rus", SP_item_ammo_grenades_rus
};


