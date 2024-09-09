/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/usa/usa_guninfo.c,v $
 *   $Revision: 1.12 $
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

#include "usa_main.h"

// gunframes.c
// D-Day: Normandy Team GunFrames (handled in p_weapon.c)


GunInfo_t usaguninfo[MAX_TEAM_GUNS]=
{
	{//Colt .45
		{4},{75}, 6,77, 62,62,

			"usa/colt45/unload.wav",
				{49},
		
			"usa/colt45/reload.wav",
				{56},
		
		"usa/colt45/fire.wav",
		NULL,

		MOD_PISTOL,
		DAMAGE_COLT45, 0,
		0
	},
	{//M1 Garand
		{4},{76}, 6,78, 64,67,
			
			"usa/m1/unload.wav",
				{55},//{56},
		
			"usa/m1/reload.wav",
				{57},//	{58},
		
		"usa/m1/fire.wav",
		"usa/m1/lastround.wav",

		MOD_RIFLE,
		DAMAGE_M1, 0,
		0
	},
	{//Thompson
		{4,5},{79,80}, 5,80, 71,71,

			"usa/thompson/unload.wav",
				{47},//{47},
		
			"usa/thompson/reload.wav",
				{63},//{63},
		
		"usa/thompson/fire.wav",
		NULL,

		MOD_SUBMG,
		DAMAGE_THOMPSON, 0,
		0
	},
	{//Browning Automatic Rifle (BAR)
		{4,5},{77,78}, 5,78, 69,69,
			
			"usa/bar/unload.wav",
		{46},//{47},
		
			"usa/bar/reload.wav",
		{61},//	{62},
		
		"usa/bar/fire.wav",
		NULL,

		MOD_LMG,
		DAMAGE_BAR, 0,
		0
	},
	{//BHMG 
		{20,21},{90,91}, 21,91, 79,82,

			"usa/bhmg/unload.wav",
		{64,71},//72},
		
			"usa/bhmg/reload.wav",
		{75},//76},
		
		"usa/bhmg/fire.wav",
		NULL,


		MOD_HMG,
		DAMAGE_BHMG, 0,
		0
	},
	{//Bazooka 
		{4},{73}, 5,75, 65,65,

			"usa/bazooka/unload.wav",
				{0},
		
			"usa/bazooka/reload.wav",
				{59},
		
		"usa/bazooka/fire.wav",
		NULL,

		MOD_ROCKET,
		DAMAGE_BAZOOKA_HIT, DAMAGE_BAZOOKA_RAD,
		0
	},
	{//Sniper (M1903) 
		{4},{52,60,75,80}, 26,80, 43,43,

			"usa/m1903/unload.wav",
		{29, 40},//{28},
		
			"usa/m1903/reload.wav",
		{36},//	{38},
		
		"usa/m1903/fire.wav",
		"usa/m1903/lastround.wav",

		MOD_SNIPER,
		DAMAGE_M1903, 0,
		0,
		"usa/m1903/bolt.wav", 67//66
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
