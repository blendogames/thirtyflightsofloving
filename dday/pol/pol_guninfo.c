/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/pol/pol_guninfo.c,v $
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

#include "pol_main.h"

// guninfo.c
// D-Day: Normandy Team POL GunInfo (handled in p_weapon.c)

GunInfo_t polguninfo[MAX_TEAM_GUNS]=
{
	{//vis
		{4},{71}, 6,73, 58,61,

			"pol/vis/unload.wav",
				{49},
		
			"pol/vis/reload.wav",
				{54},

		"pol/vis/fire.wav",
		NULL,

		MOD_PISTOL,
		DAMAGE_VIS, 0, 
		0
	},
	{//svt
		{4},{76}, 6,78, 64,67,
			
			"pol/svt/unload.wav",
				{50},
		
			"pol/svt/reload.wav",
				{60},
		
		"pol/svt/fire.wav",
		"pol/svt/lastround.wav",

		MOD_RIFLE,
		DAMAGE_SVT, 0,
		0
	},
	{//Mors 
		{4,5},{79,80}, 5,80, 71,71,

			"pol/mors/unload.wav",
				{47},

			"pol/mors/reload.wav",
		{57},
		
		"pol/mors/fire.wav",
		NULL,

		MOD_SUBMG,
		DAMAGE_MORS, 0, 
		0
	},
/*	{//Sten
		{4,5},{79,80}, 5,80, 71,71,

			"gbr/sten/unload.wav",
				{47},
		
			"gbr/sten/reload.wav",
				{60},
		
		"gbr/sten/fire.wav",
		NULL,

		MOD_SUBMG,
		DAMAGE_STEN, 0,
		0
	},*/
	{//RKM
		{4,5},{77,78}, 5,78, 69,69,
			
			"pol/rkm/unload.wav",
		{46},//{47},
		
			"pol/rkm/reload.wav",
		{61},//	{62},
		
		"pol/rkm/fire.wav",
		NULL,

		MOD_LMG,
		DAMAGE_RKM, 0,
		0
	},
	{//MG34 
		{21,22},{86,87}, 22,87, 79,79,

			"pol/mg34/unload.wav",
				{4, 65},
		
			"pol/mg34/reload.wav",
		{13,72},//{13,74},
		
		"pol/mg34/fire.wav",
		NULL,

		MOD_HMG,
		DAMAGE_MG34, 0, 
		0
	},
	{//PIAT 
		{7},{83}, 5,85, 75,75,

			"pol/piat/unload.wav",
				{0},
		
			"pol/piat/reload.wav",
				{66},   //Nick 22/11/2002 - Changed from frame 59 -> 56 
						//reload.wav loaded too late.
		
		"pol/piat/fire.wav",
		NULL,

		MOD_ROCKET,
		DAMAGE_PIAT_HIT, DAMAGE_PIAT_RAD,
		0
	},
	{//Sniper (M98ks) 
		{4},{52,60,75,80}, 26,80, 43,43,

			"grm/m98ks/unload.wav",
				{29,40},//{31,42},
		
			"grm/m98ks/reload.wav",
				{36},//			{37},
		
		"grm/m98ks/fire.wav",
		"grm/m98ks/lastround.wav",

		MOD_SNIPER,
		DAMAGE_M98KS, 0,
		0,
		"grm/m98ks/bolt.wav", 67 //66
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
