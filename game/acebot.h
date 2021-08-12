/*
===========================================================================
Copyright (C) 1998 Steve Yeager

This file is part of ACE Bot source code.

ACE Bot source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

ACE Bot source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ACE Bot source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

///////////////////////////////////////////////////////////////////////
//
//  ACE - Quake II Bot Base Code
//
//  Version 1.0
//
//  This file is Copyright(c), Steve Yeager 1998, All Rights Reserved
//
//
//	All other files are Copyright(c) Id Software, Inc.
//
//	Please see liscense.txt in the source directory for the copyright
//	information regarding those files belonging to Id Software, Inc.
//	
//	Should you decide to release a modified version of ACE, you MUST
//	include the following text (minus the BEGIN and END lines) in the 
//	documentation for your modification.
//
//	--- BEGIN ---
//
//	The ACE Bot is a product of Steve Yeager, and is available from
//	the ACE Bot homepage, at http://www.axionfx.com/ace.
//
//	This program is a modification of the ACE Bot, and is therefore
//	in NO WAY supported by Steve Yeager.
//
//	--- END ---
//
//	I, Steve Yeager, hold no responsibility for any harm caused by the
//	use of this source code, especially to small children and animals.
//  It is provided as-is with no implied warranty or support.
//
//  I also wish to thank and acknowledge the great work of others
//  that has helped me to develop this code.
//
//  John Cricket    - For ideas and swapping code.
//  Ryan Feltrin    - For ideas and swapping code.
//  SABIN           - For showing how to do true client based movement.
//  BotEpidemic     - For keeping us up to date.
//  Telefragged.com - For giving ACE a home.
//  Microsoft       - For giving us such a wonderful crash free OS.
//  id              - Need I say more.
//  
//  And to all the other testers, pathers, and players and people
//  who I can't remember who the heck they were, but helped out.
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////	
//
//  acebot.h - Main header file for ACEBOT
// 
// 
///////////////////////////////////////////////////////////////////////

#ifndef _ACEBOT_H
#define _ACEBOT_H

// Only 100 allowed for now (probably never be enough edicts for 'em)
#define MAX_BOTS 100


/*// Platform states
#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3*/

// Maximum nodes
#define MAX_NODES 2048 //was 1000

// Link types
#define INVALID -1

// Node types
#define NODE_MOVE 0
#define NODE_LADDER 1
#define NODE_PLATFORM 2
#define NODE_TELEPORTER 3
#define NODE_ITEM 4
#define NODE_WATER 5
#define NODE_GRAPPLE 6
#define NODE_JUMP 7
#define NODE_ALL 99 // For selecting all nodes

// Density setting for nodes
#define NODE_DENSITY 128

// Bot state types
#define STATE_STAND 0
#define STATE_MOVE 1
#define STATE_ATTACK 2
#define STATE_WANDER 3
#define STATE_FLEE 4

#define MOVE_LEFT 0
#define MOVE_RIGHT 1
#define MOVE_FORWARD 2
#define MOVE_BACK 3

// Item defines (got this list from somewhere??....so thanks to whoever created it)
typedef enum
{
	ITEMLIST_NULLINDEX = 0,
	ITEMLIST_BODYARMOR,
	ITEMLIST_COMBATARMOR,
	ITEMLIST_JACKETARMOR,
	ITEMLIST_ARMORSHARD,
	ITEMLIST_ARMORSHARD_FLAT,
	ITEMLIST_POWERSCREEN,
	ITEMLIST_POWERSHIELD,

	ITEMLIST_GRAPPLE,
	ITEMLIST_BLASTER,
	ITEMLIST_SHOTGUN,
	ITEMLIST_SUPERSHOTGUN,
	ITEMLIST_MACHINEGUN,
	ITEMLIST_CHAINGUN,
	ITEMLIST_GRENADES,
	ITEMLIST_GRENADELAUNCHER,
	ITEMLIST_ROCKETLAUNCHER,
	ITEMLIST_HYPERBLASTER,
	ITEMLIST_RAILGUN,
	ITEMLIST_BFG10K,
	ITEMLIST_HML,
	ITEMLIST_NULLWEAP,

	ITEMLIST_SHELLS,
	ITEMLIST_BULLETS,
	ITEMLIST_CELLS,
	ITEMLIST_ROCKETS,
	ITEMLIST_HOMINGROCKETS,
	ITEMLIST_SLUGS,
	ITEMLIST_FUEL,

	ITEMLIST_QUADDAMAGE,
	ITEMLIST_INVULNERABILITY,
	ITEMLIST_SILENCER,
	ITEMLIST_REBREATHER,
	ITEMLIST_ENVIRONMENTSUIT,
	ITEMLIST_ANCIENTHEAD,
	ITEMLIST_ADRENALINE,
	ITEMLIST_BANDOLIER,
	ITEMLIST_AMMOPACK,
	ITEMLIST_FLASHLIGHT,
	ITEMLIST_JETPACK,
	ITEMLIST_STASIS,


	ITEMLIST_DATACD,
	ITEMLIST_POWERCUBE,
	ITEMLIST_PYRAMIDKEY,
	ITEMLIST_DATASPINNER,
	ITEMLIST_SECURITYPASS,
	ITEMLIST_BLUEKEY,
	ITEMLIST_REDKEY,
	ITEMLIST_COMMANDERSHEAD,
	ITEMLIST_AIRSTRIKEMARKER,
	//ITEMLIST_HEALTH,
	//ITEMLIST_BOT,
	//ITEMLIST_PLAYER,

	ITEMLIST_HEALTH_SMALL,
	ITEMLIST_HEALTH_MEDIUM,
	ITEMLIST_HEALTH_LARGE,
	ITEMLIST_HEALTH_MEGA,

	ITEMLIST_FLAG1,
	ITEMLIST_FLAG2,
	ITEMLIST_FLAG3,
	ITEMLIST_RESISTANCETECH,
	ITEMLIST_STRENGTHTECH,
	ITEMLIST_HASTETECH,
	ITEMLIST_REGENERATIONTECH,
	ITEMLIST_VAMPIRETECH,
	ITEMLIST_AMMOGENTECH,

	ITEMLIST_AMMOGENPACK,
// Knightmare- Lazarus stuff
} itemlist_t;

/*#define ITEMLIST_NULLINDEX			0
#define ITEMLIST_BODYARMOR			1
#define ITEMLIST_COMBATARMOR		2
#define ITEMLIST_JACKETARMOR		3
#define ITEMLIST_ARMORSHARD			4
#define ITEMLIST_POWERSCREEN		5
#define ITEMLIST_POWERSHIELD		6

#define ITEMLIST_GRAPPLE            7

#define ITEMLIST_BLASTER			8
#define ITEMLIST_SHOTGUN			9
#define ITEMLIST_SUPERSHOTGUN		10
#define ITEMLIST_MACHINEGUN			11
#define ITEMLIST_CHAINGUN			12
#define ITEMLIST_GRENADES			13
#define ITEMLIST_GRENADELAUNCHER	14
#define ITEMLIST_ROCKETLAUNCHER		15
#define ITEMLIST_HYPERBLASTER		16
#define ITEMLIST_RAILGUN			17
#define ITEMLIST_BFG10K				18

#define ITEMLIST_SHELLS				19
#define ITEMLIST_BULLETS			20
#define ITEMLIST_CELLS				21
#define ITEMLIST_ROCKETS			22
#define ITEMLIST_SLUGS				23
#define ITEMLIST_QUADDAMAGE			24
#define ITEMLIST_INVULNERABILITY	25
#define ITEMLIST_SILENCER			26
#define ITEMLIST_REBREATHER			27
#define ITEMLIST_ENVIRONMENTSUIT	28
#define ITEMLIST_ANCIENTHEAD		29
#define ITEMLIST_ADRENALINE			30
#define ITEMLIST_BANDOLIER			31
#define ITEMLIST_AMMOPACK			32
#define ITEMLIST_DATACD				33
#define ITEMLIST_POWERCUBE			34
#define ITEMLIST_PYRAMIDKEY			35
#define ITEMLIST_DATASPINNER		36
#define ITEMLIST_SECURITYPASS		37
#define ITEMLIST_BLUEKEY			38
#define ITEMLIST_REDKEY				39
#define ITEMLIST_COMMANDERSHEAD		40
#define ITEMLIST_AIRSTRIKEMARKER	41
#define ITEMLIST_HEALTH				42

// new for ctf
#define ITEMLIST_FLAG1              43
#define ITEMLIST_FLAG2              44
#define ITEMLIST_RESISTANCETECH     45
#define ITEMLIST_STRENGTHTECH       46
#define ITEMLIST_HASTETECH          47
#define ITEMLIST_REGENERATIONTECH   48

// my additions
#define ITEMLIST_HEALTH_SMALL		49
#define ITEMLIST_HEALTH_MEDIUM		50
#define ITEMLIST_HEALTH_LARGE		51
#define ITEMLIST_BOT				52
#define ITEMLIST_PLAYER				53
#define ITEMLIST_HEALTH_MEGA        54*/

// Node structure
typedef struct node_s
{
	vec3_t origin; // Using Id's representation
	int type;   // type of node
} node_t;

typedef struct item_table_s
{
	int item;
	float weight;
	edict_t *ent;
	int node;
} item_table_t;

extern int num_players;
extern edict_t *players[MAX_CLIENTS];		// pointers to all players in the game

// extern decs
extern node_t nodes[MAX_NODES]; 
extern item_table_t item_table[MAX_NODES]; // was MAX_EDICTS
extern qboolean debug_mode;
extern int numnodes;
extern int num_items;

// id Function Protos I need
void     LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker);
void     ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);
void     TossClientWeapon (edict_t *self);
void     ClientThink (edict_t *ent, usercmd_t *ucmd);
void	 SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles, int *style, int *health);
void     ClientUserinfoChanged (edict_t *ent, char *userinfo);
void     CopyToBodyQue (edict_t *ent);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void     Use_Plat (edict_t *ent, edict_t *other, edict_t *activator);

// acebot_ai.c protos
void     ACEAI_Think (edict_t *self);
void     ACEAI_PickLongRangeGoal(edict_t *self);
void     ACEAI_PickShortRangeGoal(edict_t *self);
qboolean ACEAI_FindEnemy(edict_t *self);
void     ACEAI_ChooseWeapon(edict_t *self);

// acebot_cmds.c protos
qboolean ACECM_Commands(edict_t *ent);
void     ACECM_Store();

// acebot_items.c protos
void     ACEIT_PlayerAdded(edict_t *ent);
void     ACEIT_PlayerRemoved(edict_t *ent);
qboolean ACEIT_IsVisible(edict_t *self, vec3_t goal);
qboolean ACEIT_IsReachable(edict_t *self,vec3_t goal);
qboolean ACEIT_ChangeWeapon (edict_t *ent, gitem_t *item);
qboolean ACEIT_CanUseArmor (gitem_t *item, edict_t *other);
float	 ACEIT_ItemNeed(edict_t *self, int item);
int		 ACEIT_ClassnameToIndex(char *classname);
void     ACEIT_BuildItemNodeTable (qboolean rebuild);

// acebot_movement.c protos
qboolean ACEMV_SpecialMove(edict_t *self,usercmd_t *ucmd);
void     ACEMV_Move(edict_t *self, usercmd_t *ucmd);
void     ACEMV_Attack (edict_t *self, usercmd_t *ucmd);
void     ACEMV_Wander (edict_t *self, usercmd_t *ucmd);

// acebot_nodes.c protos
int      ACEND_FindCost(int from, int to);
int      ACEND_FindCloseReachableNode(edict_t *self, int dist, int type);
int      ACEND_FindClosestReachableNode(edict_t *self, int range, int type);
void     ACEND_SetGoal(edict_t *self, int goal_node);
qboolean ACEND_FollowPath(edict_t *self);
void     ACEND_GrapFired(edict_t *self);
qboolean ACEND_CheckForLadder(edict_t *self);
void     ACEND_PathMap(edict_t *self);
void     ACEND_InitNodes(void);
void     ACEND_ShowNode(int node);
void     ACEND_DrawPath();
void     ACEND_ShowPath(edict_t *self, int goal_node);
int      ACEND_AddNode(edict_t *self, int type);
void     ACEND_UpdateNodeEdge(int from, int to);
void     ACEND_RemoveNodeEdge(edict_t *self, int from, int to);
void     ACEND_ResolveAllPaths();
void     ACEND_SaveNodes();
void     ACEND_LoadNodes();

// acebot_spawn.c protos
//void	 ACESP_SaveBots(); // Knightmare- removed this
//void	 ACESP_LoadBots(); // Knightmare- removed this
void	 ACESP_LoadBotInfo(); // Knightmare added
void     ACESP_HoldSpawn(edict_t *self);
void     ACESP_PutClientInServer (edict_t *bot, qboolean respawn, int team);
void     ACESP_Respawn (edict_t *self);
edict_t *ACESP_FindFreeClient (void);
void     ACESP_SetName(edict_t *bot, char *name, char *skin, char *team);
void     ACESP_SpawnBot (char *team, char *name, char *skin, char *userinfo);
void     ACESP_ReAddBots();
void     ACESP_RemoveBot(char *name);
void	 safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...);
void     safe_centerprintf (edict_t *ent, char *fmt, ...);
void     safe_bprintf (int printlevel, char *fmt, ...);
void     debug_printf (char *fmt, ...);

#endif