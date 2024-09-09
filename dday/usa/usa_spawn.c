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

#include "usa_main.h"
// g_spawn.c
// D-Day: Normandy Team Spawns
gitem_t	*FindItemByClassname (char *classname);
void SpawnItem (edict_t *ent, gitem_t *item);
gitem_t	*FindItem (char *pickup_name);
void SP_item_weapon_colt45(edict_t *self)
{
    SpawnItem(self,FindItem("Colt .45"));
}

void SP_item_weapon_m1(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_m1"));
}

void SP_item_weapon_thompson(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_thompson"));
}

void SP_item_weapon_BAR(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_BAR"));
}

void SP_item_weapon_30cal(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_30cal"));
}

void SP_item_weapon_bazooka(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_bazooka"));
}

void SP_item_weapon_sniper(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_sniper"));
}



//ammo
void SP_item_ammo_grenades_usa(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_grenades_usa"));
}
void SP_item_ammo_colt45(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_colt45"));
}

void SP_item_ammo_thompson(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_thompson"));
}

void SP_item_ammo_BAR(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_BAR"));
}

void SP_item_ammo_rockets(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_rockets"));
}

void SP_item_ammo_m1(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_M1"));
}

void SP_item_ammo_m1903(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_m1903"));
}

void SP_item_ammo_HMG(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_HMG"));
}



void SP_misc_banner_usa (edict_t *ent)
{
	SP_misc_banner_generic(ent, "models/objects/banner/usa/tris.md2");
}

void SP_misc_banner_brit (edict_t *ent)
{
	SP_misc_banner_generic(ent, "models/objects/banner/usa/tris.md2");
}

void SP_misc_flag_usa (edict_t *ent)
{
	SP_misc_banner_generic(ent, "models/objects/banner/usa/tris.md2");
}

void SP_misc_flag_brit (edict_t *ent)
{
	SP_misc_banner_generic(ent, "models/objects/banner/usa/tris.md2");
}


/////////////////////////////////////////////////////////////

spawn_t sp_usa[MAX_TEAM_ITEMS] =

{
    "weapon_colt45", SP_item_weapon_colt45,
	"weapon_m1",SP_item_weapon_m1,
	"weapon_thompson", SP_item_weapon_thompson,
	"weapon_BAR", SP_item_weapon_BAR,
	"weapon_30cal", SP_item_weapon_30cal,
	"weapon_bazooka",SP_item_weapon_bazooka,
	"weapon_sniper",SP_item_weapon_sniper,
	"ammo_grenades_usa",SP_item_ammo_grenades_usa,
	"ammo_colt45",SP_item_ammo_colt45,
	"ammo_m1",SP_item_ammo_m1,
	"ammo_thompson",SP_item_ammo_thompson,
	"ammo_BAR",SP_item_ammo_BAR,
	"ammo_HMG",SP_item_ammo_HMG,
	"ammo_rockets",SP_item_ammo_rockets,
	"ammo_m1903",SP_item_ammo_m1903,
	"misc_banner_usa",SP_misc_banner_usa,
	"misc_banner_brit",SP_misc_banner_brit,
	"misc_flag_usa",SP_misc_flag_usa,
	"misc_flag_brit",SP_misc_flag_brit
};
