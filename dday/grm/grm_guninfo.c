/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/grm/grm_guninfo.c,v $
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

#include "grm_main.h"

// guninfo.c
// D-Day: Normandy Team GRM GunInfo (handled in p_weapon.c)

GunInfo_t grmguninfo[MAX_TEAM_GUNS]=
{
	{//p38
		{4},{71}, 6,73, 58,61,

			"grm/p38/unload.wav",
				{49},
		
			"grm/p38/reload.wav",
				{54},

		"grm/p38/fire.wav",
		NULL,

		MOD_PISTOL,
		DAMAGE_P38, 0, 
		0
	},
	{//m98k
		{4},{86}, 16,98, 78,78,

			"grm/m98k/unload.wav",
		{64},//{70},
		
			"grm/m98k/reload.wav",
		{58,73},//{62,76},
		
		"grm/m98k/lastround.wav",//fire.wav",
		"grm/m98k/lastround.wav",

		MOD_RIFLE,
		DAMAGE_M98K, 0,
		13,//17,
		"grm/m98ks/bolt.wav", 67
	},
	{//MP40 
		{4,5},{79,80}, 5,80, 71,71,

			"grm/mp40/unload.wav",
				{47},

			"grm/mp40/reload.wav",
		{63},//{64},
		
		"grm/mp40/fire.wav",
		NULL,

		MOD_SUBMG,
		DAMAGE_MP40, 0, 
		0
	},
	{//MP43 
		{4,5},{79,80}, 5,80, 71,71,

			"grm/mp43/unload.wav",
		{49},//{48},
		
			"grm/mp43/reload.wav",
		{65},//{66},
		
		"grm/mp43/fire.wav",
		NULL,

		MOD_LMG,
		DAMAGE_MP43, 0, 
		0
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
	},
	{//PanzerShrek 
		{4},{73}, 5,75, 65,65,

			"grm/panzer/unload.wav",
				{0},
		
			"grm/panzer/reload.wav",
				{59},
		
		"grm/panzer/fire.wav",
		NULL,

		MOD_ROCKET,
		DAMAGE_PANZER_HIT, DAMAGE_PANZER_RAD,
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
