/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/rus/rus_item.c,v $
 *   $Revision: 1.7 $
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

// g_items.c
// D-Day: Normandy Team Items
qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);
qboolean Pickup_Ammo (edict_t *ent, edict_t *other);
void Drop_Ammo(edict_t * ent , gitem_t * item );
void Weapon_Grenade (edict_t *ent);

// Knightmare- made this var extern to fix compile on GCC
extern GunInfo_t rusguninfo[MAX_TEAM_GUNS];


//////////////////////////////////////////////////////////////////////////////////////
//                              rus.DLL ITEMS                                       //
//////////////////////////////////////////////////////////////////////////////////////


gitem_t rusitems[MAX_TEAM_ITEMS]=
{
/* Tokarev TT33 becomes the standard issue weapon
*/
	{
		"weapon_tt33",
		WEAPON_TT33,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_tt33,
		"misc/w_pkup.wav",
		"models/weapons/rus/g_tt33/tris.md2", 0,
		"models/weapons/rus/v_tt33/tris.md2",
		"w_tt33",
		"Tokarev TT33",
		0,
		1,
		"tt33_mag",
		IT_WEAPON,
		NULL,
		0,
		LOC_PISTOL,
		2,
		1,
		5000,
		100,
/* precache */ "rus/tt33/fire.wav rus/tt33/reload.wav rus/tt33/unload.wav",
		"rus",
		&rusguninfo[tt33_FRAME],
		0
},
/*QUAKED weapon_m9130 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_m9130",
		WEAPON_M9130,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_m9130,
		"misc/w_pkup.wav",
		"models/weapons/rus/g_m9130/tris.md2", 0, //EF_ROTATE
		"models/weapons/rus/v_m9130/tris.md2",
/* icon */		"w_m9130",
/* pickup */	"M91/30",
		0,
		1,
		"m9130_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_RIFLE,
		2,
		9,
		5000,
		100,
/* precache */ "rus/m9130/fire.wav rus/m9130/lastround.wav rus/m9130/reload.wav rus/m9130/unload.wav",
		"rus",
		&rusguninfo[m9130_FRAME],
		0
	},

/*QUAKED weapon_ppsh41 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_ppsh41",
		WEAPON_PPSH41,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ppsh41,
		"misc/w_pkup.wav",
		"models/weapons/rus/g_ppsh41/tris.md2", 0,
		"models/weapons/rus/v_ppsh41/tris.md2",
/* icon */		"w_ppsh41",
/* pickup */	"PPsh41",
		0,
		1,
		"ppsh41_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SUBMACHINEGUN,
		2,
		10,
		5000,
		100,
/* precache */ "rus/ppsh41/fire.wav rus/ppsh41/reload.wav rus/ppsh41/unload.wav",
		"rus",
	&rusguninfo[ppsh41_FRAME],
		0
	},

/*QUAKED weapon_pps43 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_pps43",
		WEAPON_PPS43,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_pps43,
		"misc/w_pkup.wav",
//		"models/weapons/g_bar/tris.md2", EF_ROTATE,
		"models/weapons/rus/g_pps43/tris.md2", 0,
		"models/weapons/rus/v_pps43/tris.md2",
/* icon */		"w_pps43",
/* pickup */	"PPS43",
		0,
		1,
		"pps43_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_L_MACHINEGUN,
		2,
		20,
		5000,
		100,
/* precache */ "rus/pps43/fire.wav rus/pps43/reload.wav rus/pps43/unload.wav",
		"rus",
	&rusguninfo[pps43_FRAME],
		0
	},

	/*QUAKED weapon_dpm (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_dpm",
		WEAPON_DPM,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_dpm,
		"misc/w_pkup.wav",
		"models/weapons/rus/g_dpm/tris.md2", 0,
		"models/weapons/rus/v_dpm/tris.md2",
/* icon */		"w_dpm",
/* pickup */	"DPM",
		0,
		1,
		"dpm_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_H_MACHINEGUN,
		2,
		50,
		5000,
		100,
/* precache */ "rus/dpm/fire.wav rus/dpm/reload.wav rus/dpm/unload.wav",
		"rus",
	&rusguninfo[dpm_FRAME],
		0
	},


/*QUAKED weapon_rpg1 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rpg1",
		WEAPON_RPG1,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RPG1,
		"misc/w_pkup.wav",
//		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/rus/g_panzer/tris.md2", 0,
		"models/weapons/rus/v_panzer/tris.md2",
/* icon */		"w_panzer",
/* pickup */	"RPG-1",
		0,
		1,
		"rus_rockets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_ROCKET,
		3,
		10,
		0,
		0,
/* precache */ "rus/panzer/fire.wav rus/panzer/reload.wav rus/panzer/rockfly.wav models/objects/rocket/tris.md2 models/objects/debris2/tris.md2",
		"rus",
	&rusguninfo[PSK_FRAME],
		0
	},


/*QUAKED weapon_m9130s (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_m9130s",
		WEAPON_M9130S,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_m9130s,
		"misc/w_pkup.wav",
		"models/weapons/rus/g_m9130s/tris.md2", 0,
		"models/weapons/rus/v_m9130s/tris.md2",
/* icon */		"w_m9130s",
/* pickup */	"M91/30 Sniper",
		0,
		1,
		"m9130_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SNIPER,
		2,
		8,
		5000,
		100,
/* precache */ "rus/m9130s/fire.wav rus/m9130s/lastround.wav rus/m9130s/reload.wav rus/m9130s/unload.wav",
		"rus",
	&rusguninfo[m9130S_FRAME],
		0
	},

//////////////////////////////////////////////////////////////////////////////////////
//                                   AMMO                                           //
//////////////////////////////////////////////////////////////////////////////////////

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades_rus",
		AMMO_GRENADES_RUS,
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/weapons/rus/g_grenade/tris.md2", 0,
		"models/weapons/rus/v_grenade/tris.md2",
/* icon */		"a_f1grenade",
/* pickup */	"F1 Grenade",
/* width */		3,
		5,
		"F1 Grenade",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_TYPE_GRENADES,
		LOC_GRENADES,
		0,
		0.25,
		0,
		0,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav weapons/throw.wav",
		"rus",
		0,
		0
	},

/*QUAKED ammo_tt33 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_tt33",
		AMMO_TT33,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/pistols/tris.md2", 0,
		NULL,
/* icon */		"a_tt33",
/* pickup */	"tt33_mag",
/* width */		3,
		tt33_MAG,
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
		"rus",
		0,
		0
	},


/*QUAKED ammo_m9130 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_m9130",
		AMMO_M9130,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rifle/tris.md2", 0,
		NULL,
/* icon */		"a_m9130",
/* pickup */	"m9130_mag",
/* width */		3,
		MC_MAG,
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
		"rus",
		0,
		0
	},

/*QUAKED ammo_ppsh41 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_ppsh41",
		AMMO_PPSH41,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rus_drum2/tris.md2", 0,
		NULL,
/* icon */		"a_ppsh41",
/* pickup */	"ppsh41_mag",
/* width */		3,
		ppsh41_MAG,
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
		"rus",
		0,
		0
	},


/*QUAKED ammo_pps43 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_pps43",
		AMMO_PPS43,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rus_pph43/tris.md2", 0,
		NULL,
/* icon */		"a_pps43",
/* pickup */	"pps43_mag",
/* width */		3,
		pps43_MAG,
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
		"rus",
		0,
		0
	},

/*QUAKED ammo_dpm (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_dpm",
		AMMO_DPM,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rus_drum1/tris.md2", 0,
		NULL,
/* icon */		"a_dpm",
/* pickup */	"dpm_mag",
/* width */		3,
		dpm_MAG,
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
		"rus",
		0,
		0
	},


/*QUAKED ammo_rocketsG (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_rocketsR",
		AMMO_ROCKETSR,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rockets/tris.md2", 0,
		NULL,
/* icon */		"a_panzer",
/* pickup */	"rus_rockets",
/* width */		3,
		PSK_MAG,
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
		"rus",
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
		"rus",
		0,
		0


		},
{
	NULL
},
};






