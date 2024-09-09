/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/ita/ita_guninfo.c,v $
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

#include "ita_main.h"

// guninfo.c
// D-Day: Normandy Team ITA GunInfo (handled in p_weapon.c)

GunInfo_t itaguninfo[MAX_TEAM_GUNS]=
{
	{//b34
		{4},{71}, 6,73, 58,61,

			"ita/b34/unload.wav",
				{49},
		
			"ita/b34/reload.wav",
				{54},

		"ita/b34/fire.wav",
		NULL,

		MOD_PISTOL,
		DAMAGE_B34, 0, 
		0
	},
	{//carcano
		{4},{86}, 16,98, 78,78,

			"ita/carcano/unload.wav",
		{59},//{70},
		
			"ita/carcano/reload.wav",
		{73},//{62,76},
		
		"ita/carcano/lastround.wav",//fire.wav",
		"ita/carcano/lastround.wav",

		MOD_RIFLE,
		DAMAGE_CARCANO, 0,
		13,//17,
		"ita/carcano/bolt.wav", 67
	},
	{//B38 
		{4,5},{79,80}, 5,80, 71,71,

			"ita/b38/unload.wav",
				{47},

			"ita/b38/reload.wav",
		{63},//{64},
		
		"ita/b38/fire.wav",
		NULL,

		MOD_SUBMG,
		DAMAGE_B38, 0, 
		0
	},
	{//K43 
		{4,5},{79,80}, 5,80, 71,71,

			"ita/k43/unload.wav",
		{49},//{48},
		
			"ita/k43/reload.wav",
		{63},//{66},
		
		"ita/k43/fire.wav",
		NULL,

		MOD_RIFLE,
		DAMAGE_K43, 0, 
		0
	},
	{//b3842 
		{4,5},{79,80}, 5,80, 71,71,

			"ita/b3842/unload.wav",
				{47},

			"ita/b3842/reload.wav",
		{67},//{64},
		
		"ita/b3842/fire.wav",
		NULL,

		MOD_SUBMG,
		DAMAGE_B3842, 0, 
		0
	},
	{//Panzerfaust 
		{4},{73}, 5,75, 65,65,

			"ita/panzer/unload.wav",
				{0},
		
			"ita/panzer/reload.wav",
				{59},
		
		"ita/panzer/fire.wav",
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
	},
	{//MG42 
		{21,22},{86,87}, 22,87, 79,79,

			"ita/breda/unload.wav",
				{4, 65},
		
			"ita/breda/reload.wav",
		{13,72},//{13,74},
		
		"ita/breda/fire.wav",
		NULL,

		MOD_HMG,
		DAMAGE_BREDA, 0, 
		0
	}
}
;


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
