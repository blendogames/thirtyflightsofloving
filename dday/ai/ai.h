/*
Copyright (C) 1997-2001 Id Software, Inc.

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
--------------------------------------------------------------
The ACE Bot is a product of Steve Yeager, and is available from
the ACE Bot homepage, at http://www.axionfx.com/ace.

This program is a modification of the ACE Bot, and is therefore
in NO WAY supported by Steve Yeager.
*/
#include "AStar.h"

//	declaration of botedict for the game
//----------------------------------------------------------
#define MAX_BOT_ROAMS		128

typedef struct
{
	qboolean	jumpadReached;
	qboolean	TeleportReached;

	float		inventoryWeights[MAX_ITEMS];
	float		playersWeights[MAX_EDICTS];
	float		broam_timeouts[MAX_BOT_ROAMS];	//revisit bot roams
	
} ai_status_t;

typedef struct
{
	char		*netname;
	int			skillLevel;			// Affects AIM and fire rate
	int			moveTypesMask;		// bot can perform these moves, to check against required moves for a given path

	float		inventoryWeights[MAX_ITEMS];

	//class based functions
	void		(*UpdateStatus)(edict_t *ent);
	void		(*RunFrame)(edict_t *ent);
	void		(*bloquedTimeout)(edict_t *ent);
	void		(*deadFrame)(edict_t *ent);

} ai_pers_t;



typedef struct
{
	ai_pers_t		pers;			//persistant definition (class?)
	ai_status_t		status;			//player (bot, NPC) status for AI frame

	qboolean		is_bot;			//used for fakeclient classname determination

	//NPC state
	int				state;			// Bot State (WANDER, MOVE, etc)
	float			state_combat_timeout;


	// movement
	vec3_t			move_vector;
	float			next_move_time;
	float			wander_timeout;
	float			bloqued_timeout;
	float			changeweapon_timeout;

	// nodes
	int				current_node;
	int				goal_node;
	int				next_node;
	int				node_timeout;

	int				tries;

	struct astarpath_s	path;

	int				nearest_node_tries;	//for increasing radius of search with each try

	int			    camp_targ;
	float			aimtime;
	float			start_camp_time;
	float			actual_camp_start;
	float			tktime; //for hold your fire shout
	float			door_block_time;
	float			last_reload_try;
	float			med_heal_time;

	edict_t			*objective;

	int				dodge_node;
	edict_t			*nadedodge;
	float			fireattempt;
	float			reached_obj_time;

	edict_t			*teammatedodge;
	float			teammatedodgetime;

	float			ducktime;
	float			sniperspot;

	float			last_jump_try;
	edict_t			*last_enemy;
	int				unduck_stance;

	float			last_enemy_time;
	vec3_t			last_enemy_origin;
	int				last_enemy_stance;

	int		checknode;
	float	last_checknode_distance;

	float	door_touch_time;

	qboolean	defend_bot;

	char    *chat;
	int		chatdelay;

} ai_handle_t;


// bot_cmds.c
qboolean	BOT_Commands(edict_t *ent);
qboolean	BOT_ServerCommand (void);

// ai_main.c
void		AI_Init(void);
void		AI_NewMap(void);
void		G_FreeAI( edict_t *ent );
void		G_SpawnAI( edict_t *ent );
qboolean 	pointinfront (edict_t *self, vec3_t point);

// ai_items.c
void		AI_EnemyAdded(edict_t *ent);
void		AI_EnemyRemoved(edict_t *ent);

// bot_spawn.c
void		BOT_SpawnBot (int team, char *name, char *skin, char *userinfo);
void		BOT_RemoveBot(char *name, edict_t *botremove);
void		BOT_Respawn (edict_t *self);

//bot_misc.c
//void		AI_BotObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);

// ai_tools.c
void		AIDebug_ToogleBotDebug(void);

void		AITools_Frame(void);
void		AITools_DropNodes(edict_t *ent);

// safe **cough** prints
void		debug_printf(char *fmt, ...);
void		safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...);
void		safe_centerprintf (edict_t *ent, char *fmt, ...);
void		safe_bprintf (int printlevel, char *fmt, ...);
