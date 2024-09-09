/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/jpn/jpn_guninfo.c,v $
 *   $Revision: 1.13 $
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

// guninfo.c
// D-Day: Normandy Team JPN GunInfo (handled in p_weapon.c)

GunInfo_t jpnguninfo[MAX_TEAM_GUNS]=
{
	{//NAMBU
		{4},{71}, 6,73, 58,61,

			"jpn/nambu/unload.wav",
				{49},
		
			"jpn/nambu/reload.wav",
				{63},

		"jpn/nambu/fire.wav",
		NULL,

		MOD_PISTOL,
		DAMAGE_NAMBU, 0, 
		0
	},
	{//arisaka
		{4},{86}, 16,98, 78,78,

			"jpn/arisaka/unload.wav",
		{33,48},//{70},
		
			"jpn/arisaka/reload.wav",
		{44},//{62,76},
		
		"jpn/arisaka/fire.wav",
		"jpn/arisaka/fire.wav",//lastround.wav",

		MOD_RIFLE,
		DAMAGE_ARISAKA, 0,
		13,//17,
		"jpn/arisaka/bolt.wav", 67
	},
	{//TYPE_100 
		{4,5},{79,80}, 5,80, 71,71,

			"jpn/type_100/unload.wav",
				{41,57},

			"jpn/type_100/reload.wav",
		{63},//{64},
		
		"jpn/type_100/fire.wav",
		NULL,

		MOD_SUBMG,
		DAMAGE_TYPE_100, 0, 
		0
	},
	{//Type 99
		{4,5},{79,80}, 5,80, 71,71,

			"jpn/type_99/unload.wav",
		{28},//{48},
		
			"jpn/type_99/reload.wav",
		{45},//{66},
		
		"jpn/type_99/fire.wav",
		NULL,

		MOD_LMG,
		DAMAGE_TYPE_99, 0, 
		0
	},

/*	{//M1 Carbine
		{4},{76}, 6,78, 64,67,
			
			"usa/m1carb/unload.wav",
				{56},
		
			"usa/m1carb/reload.wav",
				{58},
		
		"usa/m1carb/fire.wav",
		NULL,

		MOD_RIFLE,
		DAMAGE_M1CARB, 0,
		0
	},*/
/*	{//PanzerShrek 
		{4},{73}, 5,75, 65,65,

			"grm/panzer/unload.wav",
				{0},
		
			"grm/panzer/reload.wav",
				{59},
		
		"grm/panzer/fire.wav",
		NULL,

		MOD_ROCKET,
		100, 100,
		0
	}, */
	{//Sniper (arisakas) 
		{4},{52,60,75,80}, 26,80, 43,43,

			"jpn/arisakas/unload.wav",
				{23,38},//{31,42},
		
			"jpn/arisakas/reload.wav",
				{35},//			{37},
		
		"jpn/arisakas/fire.wav",
		"jpn/arisakas/lastround.wav",

		MOD_SNIPER,
		DAMAGE_ARISAKAS, 0,
		0,
		"jpn/arisaka/bolt.wav", 62 //66
	},
	{//MG42 
		{21,22},{86,87}, 22,87, 79,79,

			"grm/mg42/unload.wav",
				{4, 65},
		
			"grm/mg42/reload.wav",
		{13,72},//{13,74},
		
		"grm/mg42/fire.wav",
		NULL,

		MOD_HMG,
		DAMAGE_MG42, 0, 
		0
	}

};


//// NOT COMPILED ////
//////////////////////
#if 0
//////////////////////

typedef struct
{
	int		FO[MAX_FIRE_FRAMES],		//Starting fire frames
			AFO[MAX_FIRE_FRAMES],		//Starting fire frames (truesight)

			LastFire,					//Last Fire frame
			LastAFire,					//Last AFire frame (truesight)

			LastReload,					//Last Reload frame
			LastRound;					//Last LastRound frame

	char	*ReloadSound1;				//Reload Sound #1
	int		RSoundFrames1[RSOUNDS];		//List of frames of when to play ReloadSound1

	char	*ReloadSound2;				//Reload Sound #2
	int		RSoundFrames2[RSOUNDS];		//List of frames of when to play ReloadSound2

	char	*FireSound,					//Weapon Firing Sound
			*LastRoundSound;			//Last Round Sound

	int		MeansOfDeath;				//The gun's means of death

	int		damage_direct;				//Direct Damage
	float	damage_radius;				//Radius damage

	int		frame_delay;				//Frametime between each shot

	// SNIPER WEAPON ONLY
	char	*sniper_bolt_wav;			
	int		sniper_bolt_frame;	

	// OTHER
	int	  rnd_count;					//Hack to get the right # of rounds in the clip currently loaded (for dropping/picking up the weapon)

} GunInfo_t;

#endif
