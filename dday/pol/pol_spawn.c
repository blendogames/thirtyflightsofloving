/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/pol/pol_spawn.c,v $
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

#include "pol_main.h"

// g_spawn.c
// D-Day: Normandy Team Spawns
gitem_t	*FindItemByClassname (char *classname);
void SpawnItem (edict_t *ent, gitem_t *item);
gitem_t	*FindItem (char *pickup_name);

//no fancy spawning stuff here, just nice and easy

void SP_item_weapon_vis(edict_t *self)

{
    SpawnItem(self,FindItem("weapon_vis"));
}

void SP_item_weapon_svt(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_svt"));
}

/*void SP_item_weapon_sten(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_sten"));
}*/
void SP_item_weapon_mors(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_mors"));
}

void SP_item_weapon_RKM(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_RKM"));
}

void SP_item_weapon_mg34(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_mg34"));
}

void SP_item_weapon_piat(edict_t *self);

void SP_item_weapon_POL_mauser98ks(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_pol_m98ks"));
}
void SP_item_weapon_sabre(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("weapon_sabre"));
}

//ammo

void SP_item_ammo_grenades_pol(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_grenades_pol"));
}
void SP_item_ammo_vis(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_vis"));
}

void SP_item_ammo_svt(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_svt"));
}
/*
void SP_item_ammo_sten(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_sten"));
}
*/
void SP_item_ammo_mors(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_mors"));
}
void SP_item_ammo_RKM(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_RKM"));
}
void SP_item_ammo_pol_mauser98k(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_pol_m98k"));
}
void SP_item_ammo_mg34(edict_t *self)
{
    SpawnItem(self,FindItemByClassname("ammo_mg34"));
}


void SP_item_ammo_piat(edict_t *self);



spawn_t sp_pol[MAX_TEAM_ITEMS] =

{
    "weapon_vis", SP_item_weapon_vis,
	"weapon_svt",SP_item_weapon_svt,
//	"weapon_sten", SP_item_weapon_sten,
	"weapon_mors", SP_item_weapon_mors,
	"weapon_RKM", SP_item_weapon_RKM,
	"weapon_mg34", SP_item_weapon_mg34,
	"weapon_piat",SP_item_weapon_piat,
	"weapon_pol_m98ks",SP_item_weapon_POL_mauser98ks,
    "weapon_sabre", SP_item_weapon_sabre,
	"ammo_vis",SP_item_ammo_vis,
	"ammo_svt",SP_item_ammo_svt,
//	"ammo_sten",SP_item_ammo_sten,
	"ammo_mors",SP_item_ammo_mors,
	"ammo_pol_m98k",SP_item_ammo_pol_mauser98k,
	"ammo_RKM",SP_item_ammo_RKM,
	"ammo_mg34",SP_item_ammo_mg34,
	"ammo_PIAT",SP_item_ammo_piat,
	"ammo_grenades_pol", SP_item_ammo_grenades_pol
};



