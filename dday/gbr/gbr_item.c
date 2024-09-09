/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/gbr/gbr_item.c,v $
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

#include "gbr_main.h"

// g_items.c
// D-Day: Normandy Team Items

qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);
qboolean Pickup_Ammo (edict_t *ent, edict_t *other);
void Drop_Ammo(edict_t * ent , gitem_t * item );
void Weapon_Grenade (edict_t *ent);

// Knightmare- made this var extern to fix compile on GCC
extern GunInfo_t gbrguninfo[MAX_TEAM_GUNS];



//////////////////////////////////////////////////////////////////////////////////////
//                              GBR.DLL ITEMS                                       //
//////////////////////////////////////////////////////////////////////////////////////

gitem_t gbritems[]=
{
/* Webley becomes the standard issue weapon */
	{
		"weapon_webley",
		WEAPON_WEBLEY,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Webley,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_webley/tris.md2", 0,
		"models/weapons/gbr/v_webley/tris.md2",
/* icon */		"w_webley",
/* pickup */	"Webley",
		0,
		1,
		"webley_mag",
		IT_WEAPON,
		NULL,
		0,
		LOC_PISTOL,
		2,
		1,
		5000,
		100,
/* precache */ "gbr/webley/fire.wav gbr/webley/reload.wav gbr/webley/unload.wav",
		"gbr",
		&gbrguninfo[Webley_FRAME],
		0
	},
/* QUAKED weapon_Enfield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_enfield",
		WEAPON_ENFIELD,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Enfield,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_303/tris.md2", 0, //EF_ROTATE
		"models/weapons/gbr/v_303/tris2.md2",
/* icon */		"w_303",
/* pickup */	"Lee Enfield",
		0,
		1,
		"303_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_RIFLE,
		2,
		9,
		5000,
		100,
/* precache */ "gbr/303/fire.wav gbr/303/lastround.wav gbr/303/reload.wav gbr/303/unload.wav gbr/bayonet/hit.wav gbr/bayonet/stab.wav gbr/bayonet/swipe.wav",
		"gbr",
		&gbrguninfo[Enfield_FRAME],
		0
	},

/*QUAKED weapon_sten (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_sten",
		WEAPON_STEN,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Sten,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_sten/tris.md2", 0,
		"models/weapons/gbr/v_sten/tris.md2",
/* icon */		"w_sten",
/* pickup */	"Sten",
		0,
		1,
		"sten_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SUBMACHINEGUN,
		2,
		10,
		5000,
		100,
/* precache */ "gbr/sten/fire.wav gbr/sten/reload.wav gbr/sten/unload.wav gbr/sten/jam.wav",
		"gbr",
		&gbrguninfo[Sten_FRAME],
		0
	},



/*QUAKED weapon_Bren (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_Bren",
		WEAPON_BREN,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Bren,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_bren/tris.md2", 0,
		"models/weapons/gbr/v_bren/tris.md2",
/* icon */		"w_bren",
/* pickup */	"Bren",
		0,
		1,
		"Bren_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_L_MACHINEGUN,
		2,
		20,
		5000,
		100,
/* precache */ "gbr/bren/fire.wav gbr/bren/reload.wav gbr/bren/unload.wav",
		"gbr",
		&gbrguninfo[Bren_FRAME],
		20
	},

	/*QUAKED weapon_vickers (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_vickers",
		WEAPON_VICKERS,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Vickers,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_vickers/tris.md2", 0,
		"models/weapons/gbr/v_vickers/tris.md2",
/* icon */		"w_vickers",
/* pickup */	"Vickers",
		0,
		1,
		"vickers_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_H_MACHINEGUN,
		2,
		50,
		5000,
		100,
/* precache */ "gbr/vickers/fire.wav gbr/vickers/reload.wav gbr/vickers/unload.wav gbr/vickers/bolt.wav",
		"gbr",
		&gbrguninfo[Vickers_FRAME],
		0
	},


	/*QUAKED weapon_PIAT (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_piat",
		WEAPON_PIAT,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_PIAT,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_piat/tris.md2", 0,
		"models/weapons/gbr/v_piat/tris.md2",
/* icon */		"w_piat",
/* pickup */	"PIAT",
		0,
		1,
		"gbr_rockets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_ROCKET,
		3,
		10,
		0,
		0,
/* precache */ "gbr/piat/fire.wav gbr/piat/reload.wav gbr/piat/rockfly.wav models/objects/rocket/tris.md2 models/objects/debris2/tris.md2",
		"gbr",
		&gbrguninfo[PIAT_FRAME],
		0
	},



/*QUAKED weapon_enfields (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_enfields",
		WEAPON_ENFIELDS,
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Enfields,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_303s/tris.md2", 0,
		"models/weapons/gbr/v_303s/tris.md2",
/* icon */		"w_303s",
/* pickup */	"Enfield Sniper",
		0,
		1,
		"303s_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SNIPER,
		2,
		8,
		5000,
		100,
/* precache */ "gbr/303s/fire.wav gbr/303s/lastround.wav gbr/303s/reload.wav gbr/303s/unload.wav",
		"gbr",
		&gbrguninfo[Enfields_FRAME],
		0
	},

//////////////////////////////////////////////////////////////////////////////////////
//                                   AMMO                                           //
//////////////////////////////////////////////////////////////////////////////////////

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades_gbr",
		AMMO_GRENADES_GBR,
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/weapons/gbr/g_grenade/tris.md2", 0,
		"models/weapons/gbr/v_grenade/tris.md2",
/* icon */		"a_mills_bomb",
/* pickup */	"Mills Bomb",
/* width */		3,
		5,
		"Mills Bomb",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_TYPE_GRENADES,
		LOC_GRENADES,
		0,
		0.25,
		0,
		0,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav weapons/throw.wav",
		"gbr",
		0,
		0
	},

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_webley",
		AMMO_WEBLEY,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/webbullets/tris.md2", 0,
		NULL,
/* icon */		"a_webley",
/* pickup */	"webley_mag",
/* width */		3,
		WebleyMAG,
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
		"gbr",
0,
0
	},


/*QUAKED ammo_enfield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_enfield",
		AMMO_ENFIELD,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/303mag/tris.md2", 0,
		NULL,
/* icon */		"a_303",
/* pickup */	"303_mag",
/* width */		3,
		Enfield_MAG,
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
		"gbr",
		0,
		0
	},


/*QUAKED ammo_sten (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_sten",
		AMMO_STEN,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/stenmag/tris.md2", 0,
		NULL,
/* icon */		"a_sten",
/* pickup */	"sten_mag",
/* width */		3,
		StenMAG,
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
		"gbr",
		0,
		0
	},


/*QUAKED ammo_bren (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bren",
		AMMO_BREN,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/brenmag/tris.md2", 0,
		NULL,
/* icon */		"a_bren",
/* pickup */	"bren_mag",
/* width */		3,
		Bren_MAG,
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
		"gbr",
		0,
		0
	},

/*QUAKED ammo_vickers (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_vickers",
		AMMO_VICKERS,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/vickdrum/tris.md2", 0,
		NULL,
/* icon */		"a_vickers",
/* pickup */	"vickers_mag",
/* width */		3,
		Vickers_MAG,
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
		"gbr",
		0,
		0
	},



/*QUAKED ammo_piat (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_piat",
		AMMO_PIAT,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/piatrocks/tris.md2", 0,
		NULL,
/* icon */		"a_piat",
/* pickup */	"gbr_rockets",
/* width */		3,
		PIAT_MAG,
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
		"gbr",
		0,
		0
	},


/*QUAKED ammo_enfields (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_enfields",
		AMMO_ENFIELDS,
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/303smag/tris.md2", 0,
		NULL,
/* icon */		"a_303s",
/* pickup */	"303s_mag",
/* width */		3,
		Enfields_MAG,
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
		"gbr",
		0,
		0
	},






{
	NULL
},
};







