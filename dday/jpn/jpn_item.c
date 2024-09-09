/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/jpn/jpn_item.c,v $
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

#include "jpn_main.h"

// g_items.c
// D-Day: Normandy Team Items

qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);
qboolean Pickup_Ammo (edict_t *ent, edict_t *other);
void Drop_Ammo(edict_t * ent , gitem_t * item );
void Weapon_Grenade (edict_t *ent);

// Knightmare- made this var extern to fix compile on GCC
extern GunInfo_t jpnguninfo[MAX_TEAM_GUNS];


//////////////////////////////////////////////////////////////////////////////////////
//                              JPN.DLL ITEMS                                       //
//////////////////////////////////////////////////////////////////////////////////////


gitem_t jpnitems[MAX_TEAM_ITEMS]=
{
/* Nambu Pistol becomes the standard issue weapon
*/
	{
		"weapon_nambu",
		WEAPON_NAMBU,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Nambu,
		"misc/w_pkup.wav",
		"models/weapons/jpn/g_nambu/tris.md2", 0,
		"models/weapons/jpn/v_nambu/tris.md2",
		"w_nambu",
		"Nambu Pistol",
		0,
		1,
		"nambu_mag",
		IT_WEAPON,
		NULL,
		0,
		LOC_PISTOL,
		2,
		1,
		5000,
		100,
/* precache */ "jpn/nambu/fire.wav jpn/nambu/reload.wav jpn/nambu/unload.wav",
		"jpn",
		&jpnguninfo[NAMBU_FRAME],
		0
},
/*QUAKED weapon_arisaka (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_arisaka",
		WEAPON_ARISAKA,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Arisaka,
		"misc/w_pkup.wav",
		"models/weapons/jpn/g_arisaka/tris.md2", 0, //EF_ROTATE
		"models/weapons/jpn/v_arisaka/trisb.md2",
/* icon */		"w_arisaka",
/* pickup */	"Arisaka Type99",
		0,
		1,
		"arisaka_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_RIFLE,
		2,
		9,
		5000,
		100,
/* precache */ "jpn/arisaka/fire.wav jpn/arisaka/lastround.wav jpn/arisaka/reload.wav jpn/arisaka/unload.wav",
		"jpn",
		&jpnguninfo[ARISAKA_FRAME],
		0
	},

/*QUAKED weapon_type_100 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_type_100",
		WEAPON_TYPE_100,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Type_100,
		"misc/w_pkup.wav",
		"models/weapons/jpn/g_type_100/tris.md2", 0,
		"models/weapons/jpn/v_type_100/tris.md2",
/* icon */		"w_type_100",
/* pickup */	"Type100 SMG",
		0,
		1,
		"type_100_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SUBMACHINEGUN,
		2,
		10,
		5000,
		100,
/* precache */ "jpn/type_100/fire.wav jpn/type_100/reload.wav jpn/type_100/unload.wav",
		"jpn",
	&jpnguninfo[TYPE_100_FRAME],
		0
	},

/*QUAKED weapon_type_99 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_type_99",
		WEAPON_TYPE_99,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Type_99,
		"misc/w_pkup.wav",
//		"models/weapons/g_bar/tris.md2", EF_ROTATE,
		"models/weapons/jpn/g_type_99/tris.md2", 0,
		"models/weapons/jpn/v_type_99/tris.md2",
/* icon */		"w_type_99",
/* pickup */	"Type99 LMG",
		0,
		1,
		"type_99_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_L_MACHINEGUN,
		2,
		20,
		5000,
		100,
/* precache */ "jpn/type_99/fire.wav jpn/type_99/reload.wav jpn/type_99/unload.wav",
		"jpn",
	&jpnguninfo[TYPE_99_FRAME],
		0
	},

/* QUAKED weapon_m1carb (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
/*	{
		"weapon_m1carb",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_M1Carbine,
		"misc/w_pkup.wav",
		"models/weapons/usa/g_m1carb/tris.md2", 0,
		"models/weapons/usa/v_m1carb/tris.md2",
/* icon */	//	"w_m1carb",
/* pickup *///	"M1 Carbine",
/*		0,
		1,
		"m1carb_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_H_MACHINEGUN,
		2,
		9,
		5000,
		100,
		"jpn",
/* precache */ /*"usa/m1carb/fire.wav usa/m1carb/reload.wav usa/m1carb/unload.wav",
		&jpnguninfo[M1CARB_FRAME]
	},
*/


/*QUAKED weapon_m98ks (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_arisakas",
		WEAPON_ARISAKAS,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Arisakas,
		"misc/w_pkup.wav",
		"models/weapons/jpn/g_arisakas/tris.md2", 0,
		"models/weapons/jpn/v_arisakas/tris.md2",
/* icon */		"w_arisakas",
/* pickup */	"Arisaka Scoped",
		0,
		1,
		"arisakas_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SNIPER,
		2,
		8,
		5000,
		100,
/* precache */ "jpn/arisakas/fire.wav jpn/arisakas/lastround.wav jpn/arisakas/reload.wav jpn/arisakas/unload.wav",
		"jpn",
	&jpnguninfo[ARISAKAS_FRAME],
		0
	},




	/*QUAKED weapon_mg42 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_mg42",
		WEAPON_MG42,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		JPN_Weapon_MG42,
		"misc/w_pkup.wav",
		"models/weapons/grm/g_mg42/tris.md2", 0,
		"models/weapons/grm/v_mg42/tris.md2",
/* icon */		"w_mg42",
/* pickup */	"MG42",
		0,
		1,
		"mg42_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_H_MACHINEGUN,
		2,
		50,
		5000,
		100,
/* precache */ "grm/mg42/fire.wav grm/mg42/reload.wav grm/mg42/unload.wav",
		"jpn",
	&jpnguninfo[MG42_FRAME],
		0
	},







/* Weapon_Katana (sword)

*/
{
       "weapon_katana",
	   WEAPON_KATANA,
       Pickup_Weapon,
       Use_Weapon,                             //How to use
       Drop_Weapon,
       Weapon_Katana,                           //What the function is
       "misc/w_pkup.wav",
       "models/weapons/jpn/g_katana/tris.md2", 0,
       "models/weapons/jpn/v_katana/tris.md2",      //The models stuff
       "w_katana",                                    //Icon to be used
       "Katana",                                        //Pickup name
       0,
       1,
       "Katana",
       IT_WEAPON,
       NULL,
       0,
	   LOC_KNIFE,
	   0,
	   0,
	   0,
		0,
       "misc/fhit3.wav knife/fire.wav knife/hit.wav knife/pullout.wav",
		"jpn",
		0,
		0
		},


{
       "weapon_molotov",
	   WEAPON_MOLOTOV,
       Pickup_Ammo,
       Use_Weapon,                             //How to use
       Drop_Weapon,
       Weapon_Molotov,                           //What the function is
       "misc/w_pkup.wav",
       "models/weapons/jpn/g_molotov/tris.md2", 0,
       "models/weapons/jpn/v_molotov/tris.md2",      //The models stuff
       "w_molotov",                                    //Icon to be used
       "Molotov",                                        //Pickup name
       1,
       5,
       "Molotov",
       IT_AMMO|IT_WEAPON,
       NULL,
       AMMO_TYPE_ROCKETS,
	   LOC_ROCKET,
	   0,
	   .25,
	   0,
		0,
       "",
		"jpn",
		0,
		0


		},





//////////////////////////////////////////////////////////////////////////////////////
//                                   AMMO                                           //
//////////////////////////////////////////////////////////////////////////////////////

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades_jpn",
		AMMO_GRENADES_JPN,
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/weapons/jpn/g_nade/tris.md2", 0,
		"models/weapons/jpn/v_nade/tris.md2",
/* icon */		"a_jpn_grenade",
/* pickup */	"Type97 Grenade",
/* width */		3,
		5,
		"Type97 Grenade",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_TYPE_GRENADES,
		LOC_GRENADES,
		0,
		0.25,
		0,
		0,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav weapons/throw.wav",
		"jpn",
		0,
		0
	},

/*QUAKED ammo_nambu (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_nambu",
		AMMO_NAMBU,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/jpn/pistolmag/tris.md2", 0,
		NULL,
/* icon */		"a_nambu",
/* pickup */	"nambu_mag",
/* width */		3,
		NAMBU_MAG,
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
		"jpn",
		0,
		0
	},


/*QUAKED ammo_mauser98k (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_arisaka",
		AMMO_ARISAKA,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/jpn/riflemag/tris.md2", 0,
		NULL,
/* icon */		"a_arisaka",
/* pickup */	"arisaka_mag",
/* width */		3,
		ARISAKA_MAG,
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
		"jpn",
		0,
		0
	},

/*QUAKED ammo_mauser98k (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_arisakas",
		AMMO_ARISAKAS,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/jpn/riflemag/tris.md2", 0,
		NULL,
/* icon */		"a_arisakas",
/* pickup */	"arisakas_mag",
/* width */		3,
		ARISAKAS_MAG,
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
		"jpn",
		0,
		0
	},

/*QUAKED ammo_type_100 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_type_100",
		AMMO_TYPE_100,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/jpn/smgmag/tris.md2", 0,
		NULL,
/* icon */		"a_type_100",
/* pickup */	"type_100_mag",
/* width */		3,
		TYPE_100_MAG,
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
		"jpn",
		0,
		0
	},


/*QUAKED ammo_type_99 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_type_99",
		AMMO_TYPE_99,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/jpn/lmgmag/tris.md2", 0,
		NULL,
/* icon */		"a_type_99",
/* pickup */	"type_99_mag",
/* width */		3,
		TYPE_99_MAG,
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
		"jpn",
		0,
		0
	},



/*QUAKED ammo_m1carb (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
/*	{
		"ammo_m1carb",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/m1/tris.md2", 0,
		NULL,
/* icon */	//	"a_m1carb",
/* pickup *///	"m1carb_mag",
/* width */	//	3,
/*		M1CARB_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_TYPE_HMG,//AMMO_CARBINE,
		0,
		0,
		1,
		0,
		0,
		"jpn",
/* precache */ //""
//	},

	{
		"ammo_mg42",
		AMMO_MG42,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/hmg/tris.md2", 0,
		NULL,
/* icon */		"a_mg42",
/* pickup */	"mg42_mag",
/* width */		3,
		MG42_MAG,
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
		"jpn",
		0,
		0
	},

{
	NULL
},
};







