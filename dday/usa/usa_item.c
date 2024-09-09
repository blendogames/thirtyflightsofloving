/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/usa/usa_item.c,v $
 *   $Revision: 1.8 $
 *   $Date: 2002/07/23 22:48:29 $
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


// g_items.c
// D-Day: Normandy Team Items


qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);
qboolean Pickup_Ammo (edict_t *ent, edict_t *other);
void Drop_Ammo(edict_t * ent , gitem_t * item );
void Weapon_Grenade (edict_t *ent);

// Knightmare- made this var extern to fix compile on GCC
extern GunInfo_t usaguninfo[MAX_TEAM_GUNS];


//////////////////////////////////////////////////////////////////////////////////////
//                              USA.DLL ITEMS                                       //
//////////////////////////////////////////////////////////////////////////////////////

gitem_t usaitems[MAX_TEAM_ITEMS]=
{
/* Colt .45 becomes the standard issue weapon */
	{
		"weapon_colt45",
		WEAPON_COLT45,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Colt45,
		"misc/w_pkup.wav",
		"models/weapons/usa/g_colt45/tris.md2", 0,
		"models/weapons/usa/v_colt45/tris.md2",
/* icon */		"w_colt45",
/* pickup */	"Colt .45",
		0,
		1,
		"colt45_mag",
		IT_WEAPON,
		NULL,
		0,
		LOC_PISTOL,
		2,
		1,
		5000,
		100,
/* precache */ "usa/colt45/fire.wav usa/colt45/reload.wav usa/colt45/unload.wav",
		"usa",
		&usaguninfo[COLT45_FRAME],
		0
	},
/* QUAKED weapon_m1 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_m1",
		WEAPON_M1,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_m1,
		"misc/w_pkup.wav",
		"models/weapons/usa/g_m1/tris.md2", 0,
		"models/weapons/usa/v_m1/tris.md2",
/* icon */		"w_m1",
/* pickup */	"M1 Garand",
		0,
		1,
		"m1_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_RIFLE,
		3,
		9,
		5000,
		100,
/* precache */ "usa/m1/fire.wav usa/m1/lastround.wav usa/m1/reload.wav usa/m1/unload.wav",
		"usa",
		&usaguninfo[M1_FRAME],
		0
	},

/*QUAKED weapon_Thompson (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_thompson",
		WEAPON_THOMPSON,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Thompson,
		"misc/w_pkup.wav",
		"models/weapons/usa/g_thompson/tris.md2", 0,
		"models/weapons/usa/v_thompson/tris.md2",
/* icon */		"w_thompson",
/* pickup */	"Thompson",
		0,
		1,
		"thompson_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SUBMACHINEGUN,
		2,
		10,
		5000,
		100,
/* precache */ "usa/thompson/fire.wav usa/thompson/reload.wav usa/thompson/unload.wav",
		"usa",
		&usaguninfo[THOMPSON_FRAME],
		0
	},

/*QUAKED weapon_BAR (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_BAR",
		WEAPON_BAR,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BAR,
		"misc/w_pkup.wav",
		"models/weapons/usa/g_bar/tris.md2", 0,
		"models/weapons/usa/v_bar/tris.md2",
/* icon */		"w_bar",
/* pickup */	"BAR",
		0,
		1,
		"bar_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_L_MACHINEGUN,
		2,
		20,
		5000,
		100,
/* precache */ "usa/bar/fire.wav usa/bar/reload.wav usa/bar/unload.wav",
		"usa",
		&usaguninfo[BAR_FRAME],
		20
	},

	/*QUAKED weapon_30cal (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_30cal",
		WEAPON_30CAL,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_30cal,
		"misc/w_pkup.wav",
		"models/weapons/usa/g_bhmg/tris.md2", 0,
		"models/weapons/usa/v_bhmg/tris.md2",
/* icon */		"w_bhmg",
/* pickup */	"M1919 Browning",
		0,
		1,
		"hmg_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_H_MACHINEGUN,
		2,
		50,
		5000,
		100,
/* precache */ "usa/bhmg/fire.wav usa/bhmg/reload.wav usa/bhmg/unload.wav",
		"usa",
		&usaguninfo[BHMG_FRAME],
		0
	},

	/*QUAKED weapon_Bazooka (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bazooka",
		WEAPON_BAZOOKA,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Bazooka,
		"misc/w_pkup.wav",
		"models/weapons/usa/g_bazooka/tris.md2", 0,
		"models/weapons/usa/v_bazooka/tris.md2",
/* icon */		"w_bazooka",
/* pickup */	"Bazooka",
		0,
		1,
		"usa_rockets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_ROCKET,
		3,
		10,
		0,
		0,
/* precache */ "usa/bazooka/fire.wav usa/bazooka/reload.wav usa/bazooka/rockfly.wav models/objects/rocket/tris.md2 models/objects/debris2/tris.md2",
		"usa",
		&usaguninfo[BAZOOKA_FRAME],
		0
	},


/*QUAKED weapon_Sniper (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_sniper",
		WEAPON_SNIPER,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Sniper,
		"misc/w_pkup.wav",
		"models/weapons/usa/g_m1903/tris.md2", 0,
		"models/weapons/usa/v_m1903/tris.md2",
/* icon */		"w_m1903",
/* pickup */	"M1903",
		0,
		1,
		"m1903_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SNIPER,
		2,
		8,
		5000,
		100,
/* precache */ "usa/m1903/fire.wav usa/m1903/lastround.wav usa/m1903/reload.wav usa/m1903/unload.wav",
		"usa",
		&usaguninfo[SNIPER_FRAME],
		0
	},

//////////////////////////////////////////////////////////////////////////////////////
//                                   AMMO                                           //
//////////////////////////////////////////////////////////////////////////////////////

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades_usa",
		AMMO_GRENADES_USA,
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/weapons/usa/g_grenade/tris.md2", 0,
		"models/weapons/usa/v_grenade/tris.md2",
/* icon */		"a_grenade",
/* pickup */	"Mk 2 Grenade",
/* width */		3,
		5,
		"Mk 2 Grenade",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_TYPE_GRENADES,
		LOC_GRENADES,
		0,
		0.25,
		0,
		0,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav weapons/throw.wav",
		"usa",
		NULL,
		0
	},

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_colt45",
		AMMO_COLT45,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/pistols/tris.md2", 0,
		NULL,
/* icon */		"a_colt45",
/* pickup */	"colt45_mag",
/* width */		3,
		COLT45MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_TYPE_PISTOL,
		0,
		0,
		0.25,
		0,
		0,
/* precache */ "",
		"usa",
		NULL,
		0
	},


/*QUAKED ammo_m1 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_M1",
		AMMO_M1,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rifle/tris.md2", 0,
		NULL,
/* icon */		"a_m1",
/* pickup */	"m1_mag",
/* width */		3,
		M1_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_TYPE_RIFLE,
		0,
		0,
		0.25,
		0,
		0,
/* precache */ "",
		"usa",
		NULL,
		0
	},

/*QUAKED ammo_thompson (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_thompson",
		AMMO_THOMPSON,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/smg/tris.md2", 0,
		NULL,
/* icon */		"a_thompson",
/* pickup */	"thompson_mag",
/* width */		3,
		THOMPSONMAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_TYPE_SUBMG,
		0,
		0,
		0.25,
		0,
		0,
/* precache */ "",
		"usa",
		NULL,
		0
	},


/*QUAKED ammo_BAR (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_BAR",
		AMMO_BAR,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/lmg2/tris.md2", 0,
		NULL,
/* icon */		"a_bar",
/* pickup */	"bar_mag",
/* width */		3,
		BAR_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_TYPE_LMG,
		0,
		0,
		0.5,
		0,
		0,
/* precache */ "",
		"usa",
		NULL,
		0
	},

/*QUAKED ammo_HMG (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_HMG",
		AMMO_HMG,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/hmg/tris.md2", 0,
		NULL,
/* icon */		"a_bhmg",
/* pickup */	"hmg_mag",
/* width */		3,
		HMG_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_TYPE_HMG,
		0,
		0,
		1,
		0,
		0,
/* precache */ "",
		"usa",
		NULL,
		0
	},


/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_rockets",
		AMMO_ROCKETS,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rockets/tris.md2", 0,
		NULL,
/* icon */		"a_bazooka",
/* pickup */	"usa_rockets",
/* width */		3,
		BAZOOKA_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_TYPE_ROCKETS,
		0,
		0,
		3,
		0,
		0,
/* precache */ "",
		"usa",
		NULL,
		0
	},


/*QUAKED ammo_m1903 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_m1903",
		AMMO_M1903,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rifle/tris.md2", 0,
		NULL,
/* icon */		"a_m1903",
/* pickup */	"m1903_mag",
/* width */		3,
		M1903_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_TYPE_SNIPER,
		0,
		0,
		0.25,
		0,
		0,
/* precache */ "",
		"usa",
		NULL,
		0
	},




{
	NULL
},
};






