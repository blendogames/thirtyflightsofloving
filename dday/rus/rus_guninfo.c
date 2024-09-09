/*       D-Day: Normandy by Vipersoft
 **********************************
 *   $Source: /usr/local/cvsroot/dday/src/rus/rus_guninfo.c,v $
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

#include "rus_main.h"

// guninfo.c
// D-Day: Normandy Team rus GunInfo (handled in p_weapon.c)

GunInfo_t rusguninfo[MAX_TEAM_GUNS]=
{
	{//tt33
		{4},{75}, 6,77, 62,62,

//		{4},{71}, 6,73, 58,61,

			"rus/tt33/unload.wav",
				{49},
		
			"rus/tt33/reload.wav",
				{56},

		"rus/tt33/fire.wav",
		NULL,

		MOD_PISTOL,
		DAMAGE_tt33, 0, 
		0
	},
	{//m9130
		{4},{86}, 16,98, 78,78,

			"rus/m9130/unload.wav",
				{70},
		
			"rus/m9130/reload.wav",
				{62,76},
		
		"rus/m9130/fire.wav",
		"rus/m9130/lastround.wav",

		MOD_RIFLE,
		DAMAGE_m9130, 0,
		13,//17
	},
	{//ppsh41 
		{4,5},{89,90}, 5,90, 81,81,

			"rus/ppsh41/unload.wav",
				{47},

			"rus/ppsh41/reload.wav",
				{76},
		
		"rus/ppsh41/fire.wav",
		NULL,

		MOD_SUBMG,
		DAMAGE_ppsh41, 0, 
		0
	},
	{//pps43 
		{4,5},{79,80}, 5,80, 71,71,

			"rus/pps43/unload.wav",
				{48},
		
			"rus/pps43/reload.wav",
				{66},
		
		"rus/pps43/fire.wav",
		NULL,

		MOD_LMG,
		DAMAGE_pps43, 0, 
		0
	},
	{//dpm 
		{21,22},{86,87}, 22,87, 79,79,

			"rus/dpm/unload.wav",
				{4, 65},
		
			"rus/dpm/reload.wav",
				{13,74},
		
		"rus/dpm/fire.wav",
		NULL,

		MOD_HMG,
		DAMAGE_dpm, 0, 
		0
	},
	{//RPG-1 // was PanzerShrek 
		{4},{73}, 5,75, 65,65,

			"rus/panzer/unload.wav",
				{0},
		
			"rus/panzer/reload.wav",
				{59},
		
		"rus/panzer/fire.wav",
		NULL,

		MOD_ROCKET,
		DAMAGE_RPG1_HIT, DAMAGE_RPG1_RAD,
		0
	},
	{//Sniper (m9130s) 
		{4},{52,60,75,80}, 26,80, 43,43,

			"rus/m9130s/unload.wav",
				{31,42},
		
			"rus/m9130s/reload.wav",
				{37},
		
		"rus/m9130s/fire.wav",
		"rus/m9130s/lastround.wav",

		MOD_SNIPER,
		DAMAGE_m9130S, 0,
		0,
		"rus/m9130s/bolt.wav", 64
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