/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_defines.h,v $
 *   $Revision: 1.19 $
 *   $Date: 2002/07/12 00:52:57 $
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

#define MOUSE_DELAY			0.3	//Used to determine how long to wait between mouse clicks.

//KMM: These are for the stance routine

#define STANCE_STAND 1
#define STANCE_DUCK  2
#define STANCE_CRAWL 4


#define LEG_WOUND			1
#define STOMACH_WOUND		2
#define CHEST_WOUND			4
#define HEAD_WOUND			8
#define L_ARM_WOUND			16
#define R_ARM_WOUND			32
#define NEAR_MISS			64 //Wheaty: Near Miss/Bullet Whiz
#define TANK_HIT			128 //faf


//Wheaty: Relative direction strike came from

/* Wheaty: This could be used to make flamers
explode if hit in the tank.  Unsure if Rez has
implemented it yet? */

#define LOCATION_LEFT		128
#define LOCATION_RIGHT		256
#define LOCATION_FRONT		512
#define LOCATION_BACK		1024

#define STANDARD_NORMAL			30
#define STANDARD_MAX			60
#define STANDARD_SPEED_MOD		0.85
#define STANDARD_MARKSMANSHIP	0.5

// 99 is basically unlimited

#define MAX_OFFICERS		1
#define MAX_INFANTRY		99
#define MAX_L_GUNNER		2
#define MAX_H_GUNNER		2
#define MAX_SNIPER			2
#define MAX_SPECIAL			1
#define MAX_ENGINEER		1
#define MAX_MEDIC			4
#define MAX_FLAMER			1	// To reduce flamer-rambos due to the new power of this weapon



/*

#define DAMAGE_PISTOL		25
#define DAMAGE_RIFLE		75
#define DAMAGE_SUBMG		25
#define DAMAGE_LMG			35
#define DAMAGE_HMG			50
#define DAMAGE_ROCKET_HIT	1000 // bridges default to 2000 with cast's maps =)
#define DAMAGE_ROCKET_RAD	500
#define DAMAGE_SNIPER		100

*/

#define DAMAGE_KNIFE		30
#define DAMAGE_FIST			20//faf: was  10
#define DAMAGE_HELMET		20//faf: raised from 5

#define FLAME_MAG			100

#define MAX_HMG_PITCH		-30			// makes for a 60 degree firing pitch for HMG
#define MIN_HMG_PITCH		30

#define TEAM_DEFENCE		0		//this is for missions that are supposed to defend
#define TEAM_OFFENCE		1
#define TEAM_PATROL			2

#define MAX_TEAMS			2 // 0 being NONE, 1 and 2 are real teams
#define MAX_TEAM_MATES		16  //max number of people on team

#define SHOUT_RADIUS		1500

#define KNIFE_KICK			500
#define KNIFE_RANGE			22

#define STARTING_GRENADES	2


#define TRACERSHOT			5


#define MAX_MOS				10
#define MAX_BATTARY			10
#define MAX_MOS_NAME_CHAR	16	// Knightmare- was 15


#define FLAME_DEFAULT		"1"


#define	LOC_NONE			0
#define LOC_KNIFE			1
#define LOC_HELMET			2 // unused
#define LOC_PISTOL			3
#define LOC_RIFLE			4
#define LOC_SUBMACHINEGUN	5
#define LOC_L_MACHINEGUN	6
#define LOC_H_MACHINEGUN	7
#define LOC_SPECIAL			8
#define LOC_ROCKET			9
#define LOC_SNIPER			10
#define LOC_FLAME			11
#define LOC_GRENADES		12
#define LOC_TNT				13
#define LOC_SHOTGUN			14
#define LOC_SUBMACHINEGUN2	15

#define OBDELAY				1.5

#define PARACHUTE_FACTOR	0.25

#define MAX_FIRE_FRAMES		4

#define	PENALTY_NONE		0
#define PENALTY_TEAM_KILL	1
#define PENALTY_SPAWN_CAMP	2
#define PENALTY_TEAM_CHANGE 3

#define MENU_MAX			36

#define	SNIPER_WAIT			2 // seconds

#define FORCE_RESPAWN		100 // frames

#define PSPEED_FORWARD		120
#define PSPEED_SIDE			75
#define	PSPEED_UP			50


#define TRUESIGHT_FACTOR	1.75		// / 
#define MAX_CMD_BUFFER		64	// bytes
#define CMD_DELAY			2	// seconds

// #define JUMP_TH				60  faf:  renaming this to jump_minimum for clarity
#define JUMP_MINIMUM		60
#define JUMP_TAKE			20
#define JUMP_MAX			100
#define JUMP_REGEN			0.09

// pbowens: P_ExplosionEffects
#define SWAY_START			5
#define SWAY_BREAK			5
#define SWAY_MULTI			4

// pbowens: surface types
#define SURF_SAND			1
#define SURF_GRASS			2
#define SURF_WOOD			3
#define SURF_METAL			4
#define SURF_GLASS			5


//pbowens: easier to modify
#define HEALTH_MAX			100 
#define HEALTH_INCREMENT	50
#define	HEALTH_RECOVERY		40

#define	SCOPE_FOV			25
#define STANDARD_FOV		85
#define	TS_FOV				78
#define	MAX_FOV				160

// pbowens: fix overflows
#define	INTERMISSION_DELAY	1.5	// seconds
