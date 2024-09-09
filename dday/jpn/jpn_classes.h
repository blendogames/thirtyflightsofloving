/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/jpn/jpn_classes.h,v $
 *   $Revision: 1.14 $
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

/*
typedef struct 
{
	char *name;				//mos name
	char *skinname;			//mos skin (general model defined within user dll)
	char *weapon1;			//primary weapon
	int		ammo1;			//number of mags for w1
	char *weapon2;			//secondary weapon
	int		ammo2;			//number of mags for w2
	char *grenades;			//grenade type
	int grenadenum;			//number of grenades
	char *special;			//special weapon, if any
	int	specnum;			//number of special items given
	mos_t	mos;			//MOS type that struct represents...
	int		available;		//number allowed on team
	float	normal_weight;	//normal ammount ent may carry without speed effect
	float	max_weight;		//amount of weight before player cannot stand up (-normal_weight)
	float	speed_mod;		//class speed
	float	marksmanship;	// %chance of greater elevation (less == better)
	char *MOS_Spaw_Point;	//name of spawn location
} SMos_t;
*/


SMos_t JPN_MOS_List[NUM_CLASSES]=
{
	{NULL}, // for NO MOS selectin
	{
		"Infantry",
		"class_infantry",
		"Arisaka Type99",
		6,
		NULL,
		0,
		"Type97 Grenade",
		STARTING_GRENADES,
		NULL,
		0,
		INFANTRY,
		MAX_INFANTRY,
		STANDARD_NORMAL,
		STANDARD_MAX,
		(STANDARD_SPEED_MOD),
		STANDARD_MARKSMANSHIP,
		"info_infantry_start"
	},
	
	{
		"Officer",
		"class_officer",
		"Type100 SMG",
		5,
		"Katana",
		0,
		"Type97 Grenade",
		STARTING_GRENADES,
		"Binoculars",
		1,
		OFFICER,
		MAX_OFFICERS,
		STANDARD_NORMAL,
		STANDARD_MAX,
		(STANDARD_SPEED_MOD * 0.90),
		STANDARD_MARKSMANSHIP,
		"info_officer_start"
	},	

	{
		"Light Gunner",
		"class_lmg",
		"Type99 LMG",
		5,
		NULL,
		0,
		"Type97 Grenade",
		STARTING_GRENADES,
		NULL,
		0,
		L_GUNNER,
		MAX_L_GUNNER,
		STANDARD_NORMAL,
		STANDARD_MAX,
		(STANDARD_SPEED_MOD * 0.85),
		STANDARD_MARKSMANSHIP,
		"info_lgunner_start"
	},

	{
		"Heavy Gunner",
		"class_hmg",
		"Type99 LMG",
		5,
		NULL,
		0,
		"Type97 Grenade",
		STARTING_GRENADES,
		NULL,
		0,
		H_GUNNER,
		MAX_H_GUNNER,
		STANDARD_NORMAL+15,
		STANDARD_MAX+5,
		(STANDARD_SPEED_MOD * 0.80),
		0.55,
		"info_hgunner_start"
	},

	{
		"Sniper",
		"class_sniper",
		"Arisaka Scoped",
		5,
		"Nambu Pistol",
		3,
		"Type97 Grenade",
		STARTING_GRENADES,
		NULL,
		0,
		SNIPER,
		MAX_SNIPER,
		STANDARD_NORMAL-10,
		STANDARD_MAX-10,
		(STANDARD_SPEED_MOD * 1.10),
		0,
		"info_sniper_start"
	},

	{
		"Banzai!!!",
		"class_special",
		"Katana",
		1,
		"Nambu Pistol",
		4,
		"Type97 Grenade",
		STARTING_GRENADES,
		NULL,
		0,
		SPECIAL,
		MAX_SPECIAL,
		STANDARD_NORMAL+10,
		STANDARD_MAX+5,
		(STANDARD_SPEED_MOD * 1.5),
		0.33,
		"info_special_start"
	},
	
	{
		"Engineer",
		"class_engineer",
		"Arisaka Type99",
		5,
		"Nambu Pistol",
		3,
		"Type97 Grenade",
		STARTING_GRENADES,
		"TNT",
		1,
		ENGINEER,
		MAX_ENGINEER,
		STANDARD_NORMAL,
		STANDARD_MAX,
		(STANDARD_SPEED_MOD * 0.85),
		0.60,
		"info_engineer_start"
	},	
	
	{
		"Medic",
		"class_medic",
		"Morphine",
		1,
		"Nambu Pistol",
		2,
		NULL,
		0,
		NULL,
		0,
		MEDIC,
		MAX_MEDIC,
		STANDARD_NORMAL-15,
		STANDARD_MAX,
		(STANDARD_SPEED_MOD * 1.10),
		0.80,
		"info_medic_start"
	},	
	{
		"Flamethrower",
		"class_flamethrower",
		"Molotov",
		5,
		"Nambu Pistol",
		3,
		"Type97 Grenade",
		STARTING_GRENADES,
		NULL,
		0,
		FLAMER,
		MAX_FLAMER,
		STANDARD_NORMAL+5,
		STANDARD_MAX,
		(STANDARD_SPEED_MOD * 0.85),
		0.70,
		"info_flamethrower_start"
	}
};


	
	
