/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/gbr/gbr_guninfo.c,v $
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

#include "gbr_main.h"

// gunframes.c
// D-Day: Normandy Team GunFrames (handled in p_weapon.c)


GunInfo_t gbrguninfo[MAX_TEAM_GUNS]=
{
	{//Webley (pistol)
		{4},{82}, 6,84, 69,69,

			"gbr/webley/unload.wav",
				{49},  // 42
		
			"gbr/webley/reload.wav",
				{56},  // 56
		
		"gbr/webley/fire.wav",
		NULL,

		MOD_PISTOL,
		DAMAGE_Webley, 0,
		0
	},
	{//Lee Enfield
		{4},{86}, 16,101, 81,81,

			"gbr/303/unload.wav",
				{58},
		
			"gbr/303/reload.wav",
				{67},
		
		"gbr/303/fire.wav",
		"gbr/303/lastround.wav",

		MOD_RIFLE,
		DAMAGE_Enfield, 0,
		13//17
	},
	{//Sten
		{4,5},{79,80}, 5,80, 71,71,

			"gbr/sten/unload.wav",
				{47},
		
			"gbr/sten/reload.wav",
				{60},
		
		"gbr/sten/fire.wav",
		NULL,

		MOD_SUBMG,
		DAMAGE_Sten, 0,
		0
	},
	{//Bren  (LMG)
		{4,5},{77,78}, 5,78, 69,69,
			
			"gbr/bren/unload.wav",
				{46},
		
			"gbr/bren/reload.wav",
				{61},
		
		"gbr/bren/fire.wav",
		NULL,

		MOD_LMG,
		DAMAGE_Bren, 0,
		0
	},
	{//Vickers 
		{20,21},{104,105}, 21,105, 86,89,

			"gbr/vickers/unload.wav",
				{64},
		
			"gbr/vickers/reload.wav",
				{76}, //Nick 22/11/2002 - Removed a frame - reload.wav was played twice.
		
		"gbr/vickers/fire.wav",
		NULL,


		MOD_HMG,
		DAMAGE_Vickers, 0,
		0
	},
	{//PIAT 
		{7},{83}, 5,85, 75,75,

			"gbr/piat/unload.wav",
				{0},
		
			"gbr/piat/reload.wav",
				{66},   //Nick 22/11/2002 - Changed from frame 59 -> 56 
						//reload.wav loaded too late.
		
		"gbr/piat/fire.wav",
		NULL,

		MOD_ROCKET,
		DAMAGE_PIAT_HIT, DAMAGE_PIAT_RAD,
		0
	},
	{//Lee Enfield Sniper 
		{4},{62,71,86,91}, 26,91, 53,53,

			"gbr/303s/unload.wav",
				{28},
		
			"gbr/303s/reload.wav",
				{37},
		
		"gbr/303s/fire.wav",
		"gbr/303s/lastround.wav",

		MOD_SNIPER,
		DAMAGE_Enfields, 0,
		0,
		"gbr/303s/bolt.wav", 77
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
