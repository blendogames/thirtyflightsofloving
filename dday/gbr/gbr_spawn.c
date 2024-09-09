/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/gbr/gbr_spawn.c,v $
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

#include "gbr_main.h"


// g_spawn.c
// D-Day: Normandy Team Spawns


//no fancy spawning stuff here, just nice and easy
gitem_t	*FindItemByClassname (char *classname);
void SpawnItem (edict_t *ent, gitem_t *item);
gitem_t	*FindItem (char *pickup_name);

void SP_item_weapon_webley(edict_t *self)

{
    SpawnItem(self,FindItem("webley"));
}

void SP_item_weapon_enfield(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_enfield"));
}

void SP_item_weapon_sten(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_sten"));
}

void SP_item_weapon_bren(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_bren"));
}
void SP_item_weapon_vickers(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_vickers"));
}


void SP_item_weapon_piat(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_piat"));
}

void SP_item_weapon_enfields(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_enfields"));
}



//ammo
void SP_item_ammo_grenades_gbr(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_grenades_gbr"));
}
void SP_item_ammo_webley(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_webley"));
}

void SP_item_ammo_sten(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_sten"));
}

void SP_item_ammo_bren(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_bren"));
}

void SP_item_ammo_piat(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_piat"));
}

void SP_item_ammo_enfield(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_enfield"));
}

void SP_item_ammo_enfields(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_enfields"));
}

void SP_item_ammo_vickers(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_vickers"));
}

// Misc stuff

void SP_misc_banner_gbr (edict_t *ent)
{
	SP_misc_banner_generic(ent, "models/objects/banner/gbr/tris.md2");
}

void SP_misc_banner_brit (edict_t *ent);


void SP_misc_flag_gbr (edict_t *ent)
{
	SP_misc_banner_generic(ent, "models/objects/banner/gbr/tris.md2");
}

void SP_misc_flag_brit (edict_t *ent);




/////////////////////////////////////////////////////////////

spawn_t sp_gbr[MAX_TEAM_ITEMS] =

{
    "weapon_webley", SP_item_weapon_webley,
	"weapon_enfield",SP_item_weapon_enfield,
	"weapon_sten", SP_item_weapon_sten,
	"weapon_bren", SP_item_weapon_bren,
	"weapon_vickers", SP_item_weapon_vickers,
	"weapon_piat",SP_item_weapon_piat,
	"weapon_enfields",SP_item_weapon_enfields,
	"ammo_grenades_gbr",SP_item_ammo_grenades_gbr,
	"ammo_webley",SP_item_ammo_webley,
	"ammo_wembley",SP_item_ammo_enfield,
	"ammo_sten",SP_item_ammo_sten,
	"ammo_bren",SP_item_ammo_bren,
	"ammo_vickers",SP_item_ammo_vickers,
	"ammo_PIAT",SP_item_ammo_piat,
	"ammo_enfields",SP_item_ammo_enfields,
	"misc_banner_gbr",SP_misc_banner_gbr,
	"misc_flag_gbr",SP_misc_flag_gbr,
	"misc_flag_brit",SP_misc_flag_brit
};
