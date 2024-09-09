/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_local.h,v $
 *   $Revision: 1.34 $
 *   $Date: 2002/07/25 08:28:44 $
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

#ifdef _WIN32
#include <malloc.h>
#include <direct.h>
#endif

// g_local.h -- local definitions for game module
#include "g_defines.h"
#include "q_shared.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE 1

#include "game.h"

#include "ai/ai.h"//JABot

//
// p_menu.c
//
#include "p_menu.h"

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"dday"
//#define DEVVERSION	"4.1"
#define DEVVERSION "5.0"//faf
//#define	DEBUG		1

// protocol bytes that can be directly added to messages
#define	svc_muzzleflash		1
#define	svc_muzzleflash2	2
#define	svc_temp_entity		3
#define	svc_layout			4
#define	svc_inventory		5

//==================================================================

// view pitching times

#define DAMAGE_TIME		0.5
#define	FALL_TIME		0.3


// edict->spawnflags

// these are set with checkboxes on each entity in the map editor
#define	SPAWNFLAG_NOT_EASY			0x00000100
#define	SPAWNFLAG_NOT_MEDIUM		0x00000200
#define	SPAWNFLAG_NOT_HARD			0x00000400
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00000800
#define	SPAWNFLAG_NOT_COOP			0x00001000


// edict->flags
#define	FL_FLY					0x00000001
#define	FL_SWIM					0x00000002	// implied immunity to drowining
#define FL_IMMUNE_LASER			0x00000004
#define	FL_INWATER				0x00000008
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define FL_IMMUNE_SLIME			0x00000040
#define FL_IMMUNE_LAVA			0x00000080
#define	FL_PARTIALGROUND		0x00000100	// not all corners are valid
#define	FL_WATERJUMP			0x00000200	// player jumping out of water
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_POWER_ARMOR			0x00001000	// power armor (if any) is active
#define FL_RESPAWN				0x80000000	// used for item respawning
#define FL_BOOTS                0x00002000  //Anti-Gravity boots flag


#define	FRAMETIME		0.1

// memory tags to allow dynamic memory to be cleaned up

#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level
#define TAG_TEMP	767


#define MELEE_DISTANCE	80
#define BODY_QUEUE_SIZE		8

//faf
#define MOVE_LEFT  1
#define MOVE_RIGHT 2
#define MOVE_FORWARD 3
#define MOVE_BACKWARD 4



typedef enum
{
	DAMAGE_NO,
	DAMAGE_YES,			// will take damage if hit
	DAMAGE_AIM			// auto targeting recognizes this
} damage_t;


typedef enum
{
	WEAPON_READY,
	WEAPON_ACTIVATING,
	WEAPON_DROPPING,
	WEAPON_FIRING,
	WEAPON_END_MAG,
    WEAPON_RELOADING,
	WEAPON_RAISE,
	WEAPON_LOWER
} weaponstate_t;

typedef enum
{
	AMMO_TYPE_SUBMG,
	AMMO_TYPE_PISTOL,
	AMMO_TYPE_ROCKETS,
	AMMO_TYPE_GRENADES,
//bcass start - TNT
	AMMO_TYPE_TNT,
//bcass end
	AMMO_TYPE_LMG,
	AMMO_TYPE_HMG,
	AMMO_TYPE_RIFLE,
	AMMO_TYPE_SNIPER,
	AMMO_TYPE_FLAME,
	AMMO_TYPE_SHOTGUN
} ammo_t;

typedef enum
{
	NONE,
	INFANTRY,
	OFFICER,
	L_GUNNER,
	H_GUNNER,
	SNIPER,
	SPECIAL,
	ENGINEER,
	MEDIC,
	FLAMER,
} mos_t;


//deadflag
#define DEAD_NO					0
#define DEAD_DYING				1
#define DEAD_DEAD				2
#define DEAD_RESPAWNABLE		3

//range
#define RANGE_MELEE				0
#define RANGE_NEAR				1
#define RANGE_MID				2
#define RANGE_FAR				3

//gib types
#define GIB_ORGANIC				0
#define GIB_METALLIC			1

//monster ai flags
#define AI_STAND_GROUND			0x00000001
#define AI_TEMP_STAND_GROUND	0x00000002
#define AI_SOUND_TARGET			0x00000004
#define AI_LOST_SIGHT			0x00000008
#define AI_PURSUIT_LAST_SEEN	0x00000010
#define AI_PURSUE_NEXT			0x00000020
#define AI_PURSUE_TEMP			0x00000040
#define AI_HOLD_FRAME			0x00000080
#define AI_GOOD_GUY				0x00000100
#define AI_BRUTAL				0x00000200
#define AI_NOSTEP				0x00000400
#define AI_DUCKED				0x00000800
#define AI_COMBAT_POINT			0x00001000
#define AI_MEDIC				0x00002000
#define AI_RESURRECTING			0x00004000

//monster attack state
#define AS_STRAIGHT				1
#define AS_SLIDING				2
#define	AS_MELEE				3
#define	AS_MISSILE				4

// armor types
#define ARMOR_NONE				0
#define ARMOR_JACKET			1
#define ARMOR_COMBAT			2
#define ARMOR_BODY				3
#define ARMOR_SHARD				4

// power armor types
#define POWER_ARMOR_NONE		0
#define POWER_ARMOR_SCREEN		1
#define POWER_ARMOR_SHIELD		2

// handedness values
#define RIGHT_HANDED			0
#define LEFT_HANDED				1
#define CENTER_HANDED			2

// game.serverflags values
#define SFL_CROSS_TRIGGER_1		0x00000001
#define SFL_CROSS_TRIGGER_2		0x00000002
#define SFL_CROSS_TRIGGER_3		0x00000004
#define SFL_CROSS_TRIGGER_4		0x00000008
#define SFL_CROSS_TRIGGER_5		0x00000010
#define SFL_CROSS_TRIGGER_6		0x00000020
#define SFL_CROSS_TRIGGER_7		0x00000040
#define SFL_CROSS_TRIGGER_8		0x00000080
#define SFL_CROSS_TRIGGER_MASK	0x000000ff

// noise types for PlayerNoise
#define PNOISE_SELF				0
#define PNOISE_WEAPON			1
#define PNOISE_IMPACT			2

// edict->movetype values
typedef enum
{
MOVETYPE_NONE,			// never moves
MOVETYPE_NOCLIP,		// origin and angles change with no interaction
MOVETYPE_PUSH,			// no clip to world, push on box contact
MOVETYPE_STOP,			// no clip to world, stops on box contact

MOVETYPE_WALK,			// gravity
MOVETYPE_STEP,			// gravity, special edge handling
MOVETYPE_FLY,
MOVETYPE_TOSS,			// gravity
MOVETYPE_FLYMISSILE,	// extra size to monsters
MOVETYPE_BOUNCE
} movetype_t;


typedef struct
{
	int		base_count;
	int		max_count;
	float	normal_protection;
	float	energy_protection;
	int		armor;

} gitem_armor_t;



// gitem_t->flags
#define	IT_WEAPON		1		// use makes active weapon
#define	IT_AMMO			2
#define IT_ARMOR		4
#define IT_STAY_COOP	8
#define IT_KEY			16
#define IT_POWERUP		32

#define RSOUNDS	3

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


typedef struct gitem_s
{
	char		*classname;	// spawning name

	int			classnameb;

	qboolean	(*pickup)(struct edict_s *ent, struct edict_s *other);

	void		(*use)(struct edict_s *ent, struct gitem_s *item);
	void		(*drop)(struct edict_s *ent, struct gitem_s *item);
	void		(*weaponthink)(struct edict_s *ent);
	char		*pickup_sound;
	char		*world_model;
	int			world_model_flags;
	char		*view_model;

	// client side info
	char		*icon;
	char		*pickup_name;	// for printing on pickup
	int			count_width;		// number of digits to display by icon

	int			quantity;		// for ammo how much, for weapons how much is used per shot
	char		*ammo;			// for weapons
	int			flags;			// IT_* flags


	void		*info;
	int			tag;
	int			position;		//position on keyboard weap belongs in
	int			topoff;			//weapon may be topped off
	float		weight;			//weight of the weapon
	int			max_range;		//max effective range for bullets
	int			max_y;			//max elevation of bullet in flight

	char		*precaches;		// string of all models, sounds, and images this item will use
	char		*dllname;		// faf: so weapons/ammo can be referred to by team name.
	GunInfo_t	*guninfo;		//will be used in optimized code
	int			mag_index;


} gitem_t;


//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
typedef struct
{
	char		helpmessage1[512];
	char		helpmessage2[512];
	int			helpchanged;	// flash F1 icon if non 0, play sound
								// and increment only if 1, 2, or 3

	gclient_t	*clients;		// [maxclients]

	// can't store spawnpoint in level, because
	// it would get overwritten by the savegame restore
	char		spawnpoint[512];	// needed for coop respawns

	// store latched cvars here that we want to get at often
	int			maxclients;
	int			maxentities;

	// cross level triggers
	int			serverflags;

	// items
	int			num_items;
	qboolean	autosaved;



} game_locals_t;


//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
typedef struct
{
	int			framenum;
	float		time;

	char		level_name[MAX_QPATH];	// the descriptive name (Outer Base, etc)
	char		mapname[MAX_QPATH];		// the server name (base1, etc)
	char		nextmap[MAX_QPATH];		// go here when fraglimit is hit

	//char		objective[1024];		// for objective printing

	char		*objectivepic;		// for objectives pictures


	// intermission state
	float		intermissiontime;		// time the intermission was started
	char		*changemap;
	int			exitintermission;
	vec3_t		intermission_origin;

	vec3_t		intermission_angle;


	edict_t		*sight_client;	// changed once each frame for coop games

	edict_t		*sight_entity;
	int			sight_entity_framenum;
	edict_t		*sound_entity;
	int			sound_entity_framenum;
	edict_t		*sound2_entity;
	int			sound2_entity_framenum;


	int			pic_health;

	int			total_secrets;
	int			found_secrets;

	int			total_goals;
	int			found_goals;

	int			total_monsters;
	int			killed_monsters;

	edict_t		*current_entity;	// entity running from G_RunFrame
	int			body_que;			// dead bodies


	int			power_cubes;		// ugly necessity for coop

	int			gib_count;//faf:  to limit number of gibs and debris;

	qboolean	victory_song_played;//faf

	int			obj_time;
	int			obj_team;

	int			last_bot_shout_time;

	qboolean	fullbright;//faf
	float		fog;

	char		*campaign;
	float		map_vote_time;
	float		last_vote_time;

	qboolean	allied_cmps;
	qboolean	axis_cmps;

	char        *botfiles;  //whether override file is used or not

	float		ctb_time;

} level_locals_t;


// spawn_temp_t is only used to hold entity field values that

// can be set from the editor, but aren't actualy present
// in edict_t during gameplay

typedef struct
{
	// world vars
	char		*sky;
	float		skyrotate;
	vec3_t		skyaxis;
	char		*nextmap;


	int			lip;
	int			distance;
	int			height;
	char		*noise;
	float		pausetime;
	char		*item;
	char		*gravity;


	float		minyaw;
	float		maxyaw;
	float		minpitch;
	float		maxpitch;
	int			obj_owner;

	int			weight;	// JABot
	float		phase;

	float		shift;

	// Knightmare- added for sky distance
	float		skydistance;
	// Knightmare- added for DK-style clouds
	char		*cloudname;
	float		lightningfreq;
	float		cloudxdir;
	float		cloudydir;
	float		cloud1tile;
	float		cloud1speed;
	float		cloud1alpha;
	float		cloud2tile;
	float		cloud2speed;
	float		cloud2alpha;
	float		cloud3tile;
	float		cloud3speed;
	float		cloud3alpha;

	// Knightmare- added for misc_flare
	int			fade_start_dist;
	int			fade_end_dist;
	char		*image;
	char		*rgba;
} spawn_temp_t;

typedef struct
{
	// fixed data
	vec3_t		start_origin;
	vec3_t		start_angles;
	vec3_t		end_origin;
	vec3_t		end_angles;

	int			sound_start;
	int			sound_middle;
	int			sound_end;

	float		accel;
	float		speed;
	float		decel;
	float		distance;

	float		wait;

	// state data
	int			state;
	vec3_t		dir;
	float		current_speed;
	float		move_speed;
	float		next_speed;
	float		remaining_distance;
	float		decel_distance;
	void		(*endfunc)(edict_t *);
} moveinfo_t;


typedef struct
{
	void	(*aifunc)(edict_t *self, float dist);
	float	dist;
	void	(*thinkfunc)(edict_t *self);
} mframe_t;

typedef struct
{
	int			firstframe;
	int			lastframe;
	mframe_t	*frame;
	void		(*endfunc)(edict_t *self);
} mmove_t;

typedef struct
{
	mmove_t		*currentmove;
	int			aiflags;
	int			nextframe;
	float		scale;

	void		(*stand)(edict_t *self);
	void		(*idle)(edict_t *self);
	void		(*search)(edict_t *self);
	void		(*walk)(edict_t *self);
	void		(*run)(edict_t *self);
	void		(*dodge)(edict_t *self, edict_t *other, float eta);
	void		(*attack)(edict_t *self);
	void		(*melee)(edict_t *self);
	void		(*sight)(edict_t *self, edict_t *other);
	qboolean	(*checkattack)(edict_t *self);

	float		pausetime;
	float		attack_finished;

	vec3_t		saved_goal;
	float		search_time;
	float		trail_time;
	vec3_t		last_sighting;
	int			attack_state;
	int			lefty;
	float		idle_time;
	int			linkcount;

	int			power_armor_type;
	int			power_armor_power;
} monsterinfo_t;


extern	game_locals_t	game;
extern	level_locals_t	level;
extern	game_import_t	gi;
extern	game_export_t	globals;
extern	spawn_temp_t	st;

extern	int	sm_meat_index;
extern	int	snd_fry;

extern	int	jacket_armor_index;
extern	int	combat_armor_index;
extern	int	body_armor_index;


// means of death

#define MOD_UNKNOWN			0
#define MOD_PISTOL			1
#define MOD_SHOTGUN			2
#define MOD_RIFLE			3
#define MOD_LMG				4
#define MOD_HMG				5
#define MOD_GRENADE			6
#define MOD_G_SPLASH		7
#define MOD_ROCKET			8
#define MOD_R_SPLASH		9
#define MOD_SUBMG			10
#define MOD_SNIPER			11
#define MOD_HANDGRENADE		15
#define MOD_HG_SPLASH		16
#define MOD_WATER			17
#define MOD_SLIME			18
#define MOD_LAVA			19
#define MOD_CRUSH			20
#define MOD_TELEFRAG		21
#define MOD_FALLING			22
#define MOD_SUICIDE			23
#define MOD_HELD_GRENADE	24
#define MOD_EXPLOSIVE		25
#define MOD_BARREL			26
#define MOD_BOMB			27
#define MOD_EXIT			28
#define MOD_SPLASH			29
#define MOD_TARGET_LASER	30
#define MOD_TRIGGER_HURT	31
#define MOD_HIT				32
#define MOD_TARGET_BLASTER	33
#define MOD_BACKBLAST		34
#define	MOD_WOUND			35
#define MOD_KNIFE			36
#define MOD_FISTS			37
#define MOD_CHANGETEAM		38
#define MOD_PENALTY			39
#define MOD_NOMOD			40
//bcass start - TNT
#define MOD_TNT				41
#define MOD_TNT1			42
#define MOD_HELD_TNT		43
#define MOD_TNT_SPLASH		44
#define MOD_TNT1_SPLASH		45
//bcass end
#define MOD_HELMET          46//faf
#define MOD_AIRSTRIKE		47
#define MOD_AIRSTRIKE_SPLASH  48//
#define MOD_BAYONET			49//faf
#define MOD_PLONK           50//faf
#define MOD_SPAWNCAMP		51
#define MOD_CHANGETEAM_WOUNDED  52
#define MOD_FRIENDLY_FIRE	0x8000000
#define	MOD_BOTTLE			53
#define	MOD_TANKHIT			54
#define	MOD_SHOTGUN2		55

extern	int	meansOfDeath;


extern	edict_t			*g_edicts;

#define	FOFS(x) (size_t)&(((edict_t *)0)->x)	// Knightmare- was int
#define	STOFS(x) (size_t)&(((spawn_temp_t *)0)->x)	// Knightmare- was int
#define	LLOFS(x) (size_t)&(((level_locals_t *)0)->x)	// Knightmare- was int
#define	CLOFS(x) (size_t)&(((gclient_t *)0)->x)	// Knightmare- was int

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

extern	cvar_t	*maxentities;
extern	cvar_t	*deathmatch;

extern	cvar_t	*coop;

extern	cvar_t	*dmflags;

extern	cvar_t	*skill;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
extern	cvar_t	*password;

extern	cvar_t	*g_select_empty;

extern	cvar_t	*dedicated;

extern	cvar_t	*sv_gravity;
extern	cvar_t	*sv_maxvelocity;

extern	cvar_t	*gun_x, *gun_y, *gun_z;
extern	cvar_t	*sv_rollspeed;
extern	cvar_t	*sv_rollangle;

extern	cvar_t	*run_pitch;
extern	cvar_t	*run_roll;
extern	cvar_t	*bob_up;
extern	cvar_t	*bob_pitch;
extern	cvar_t	*bob_roll;
extern	cvar_t	*sv_cheats;
extern	cvar_t	*maxclients;

//Flyingnun Password

extern  cvar_t  *flyingnun_password;

//KMM: new console vars

//extern	cvar_t	*scope_setting;
//extern	cvar_t	*auto_reload;
//extern  cvar_t	*auto_weapon_change;
//extern	cvar_t	*observer_on_death;
//extern	cvar_t	*heal_wounds;
//extern	cvar_t	*flame_shots;
extern	cvar_t	*RI;
extern	cvar_t	*team_kill;
extern	cvar_t	*class_limits;
extern	cvar_t	*spawn_camp_check;
extern	cvar_t	*spawn_camp_time;

//extern	cvar_t	*teamgren;
extern	cvar_t	*death_msg;
extern	cvar_t	*level_wait;
extern	cvar_t	*invuln_spawn;

extern	cvar_t	*arty_delay;
extern	cvar_t	*arty_time;
extern	cvar_t  *arty_max;

//bcass start - easter_egg cvar, AGAIN
extern	cvar_t	*easter_egg;
//bcass end

extern	cvar_t	*flood_msgs;
extern	cvar_t	*flood_persecond;
extern	cvar_t	*flood_waitdelay;

//faf
extern  cvar_t  *player_scores;
extern  cvar_t  *max_gibs;
extern  cvar_t  *extra_anims;
extern  cvar_t  *force_auto_select;

extern cvar_t  *allied_password;
extern cvar_t  *axis_password;

extern cvar_t  *objective_protect;


extern cvar_t  *ent_files;

extern cvar_t  *sniper_only;
extern cvar_t  *mauser_only;
extern cvar_t  *swords;
extern cvar_t  *no_nades;
extern cvar_t  *airstrikes;
extern cvar_t  *nohud;
extern cvar_t  *mapvoting;
extern cvar_t  *constant_play; //for testing
extern cvar_t  *serverimg;

extern cvar_t  *ctc;

extern cvar_t  *stats;

extern cvar_t *mashup;
extern cvar_t *fast_knife;
extern cvar_t *sv_maplist; //faf
extern cvar_t *chile;

extern cvar_t *bots;
extern cvar_t *botchat;
extern cvar_t *botchatfreq;
//extern cvar_t *alliedbots;
//extern cvar_t *axisbots;
extern cvar_t *alliedlevel;
extern cvar_t *axislevel;
//extern cvar_t *playerminforbots;
extern cvar_t *playermaxforbots;

extern cvar_t  *knifefest;
extern cvar_t  *fullbright;

extern cvar_t  *campaign;

extern cvar_t *sandbaglimit;

extern cvar_t *afk_time;

//extern	cvar_t	*crosshair;

#define world	(&g_edicts[0])

// item spawnflags

#define ITEM_TRIGGER_SPAWN		0x00000001
#define ITEM_NO_TOUCH			0x00000002

// 6 bits reserved for editor flags
// 8 bits used as power cube id bits for coop games

#define DROPPED_ITEM			0x00010000
#define	DROPPED_PLAYER_ITEM		0x00020000
#define ITEM_TARGETS_USED		0x00040000


//
// fields are needed for spawning from the entity string
// and saving / loading games
//
#define FFL_SPAWNTEMP		1
#define FFL_NOSPAWN			2
#define FFL_DEFAULT_NEG		4	// Knightmare- spawntemp that defaults to -1

typedef enum {
	F_INT,
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_FUNCTION,
	F_MMOVE,
	F_IGNORE
} fieldtype_t;



typedef struct
{
	char	*name;
	size_t		ofs;	// Knightmare- was int
	fieldtype_t	type;
	int		flags;
} field_t;

extern	field_t fields[];
extern	gitem_t	itemlist[];

#ifndef USER_EXCLUDE_FUNCTIONS

// Knightmare- made this var extern to fix compile on GCC
extern byte		is_silenced;

void Feed_Ammo(edict_t *ent);
void Fire_Arty(edict_t *battary,edict_t *ent);
void Cmd_Artillery_f(edict_t *ent);
void Cmd_Airstrike_f(edict_t *ent);
//
// g_cmds.c
//

void Cmd_Reload_f(edict_t *ent);
void Cmd_Help_f (edict_t *ent);

void Cmd_Score_f (edict_t *ent);


//
// g_items.c
//
void PrecacheItem (gitem_t *it);

void InitItems (void);
void SetItemNames (void);
gitem_t	*FindItem (char *pickup_name);
gitem_t	*FindItemByClassname (char *classname);
#define	ITEM_INDEX(x) ((x)-itemlist)
edict_t *Drop_Item (edict_t *ent, gitem_t *item);
void SetRespawn (edict_t *ent, float delay);
void ChangeWeapon (edict_t *ent);
void SpawnItem (edict_t *ent, gitem_t *item);
void Think_Weapon (edict_t *ent);


// CCH: new prototype for function called when airstrike arrives

void Think_Airstrike (edict_t *ent);

// KMM: prototype for artillery

void Think_Arty (edict_t *ent);

//qboolean FindOnTeam(edict_t *ent);

qboolean OnSameTeam (edict_t *self, edict_t *target);



int ArmorIndex (edict_t *ent);
int PowerArmorType (edict_t *ent);
gitem_t	*GetItemByIndex (int index);
qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count);

qboolean Add_Class_Ammo (gclient_t *client, gitem_t *item, int count);

void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);


//
// g_utils.c
//
qboolean	KillBox (edict_t *ent);


void	G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
edict_t *G_Find (edict_t *from, size_t fieldofs, char *match);	// Knightmare- changed fieldofs from int
edict_t *G_Find_Team (edict_t *from, size_t fieldofs, char *match, int team);	// Knightmare- changed fieldofs from int
edict_t *findradius (edict_t *from, vec3_t org, float rad);
edict_t *G_PickTarget (char *targetname);
void	G_UseTargets (edict_t *ent, edict_t *activator);
void	G_SetMovedir (vec3_t angles, vec3_t movedir);

void	G_InitEdict (edict_t *e);
edict_t	*G_Spawn (void);
void	G_FreeEdict (edict_t *e);

void	G_TouchTriggers (edict_t *ent);
void	G_TouchSolids (edict_t *ent);

char	*G_CopyString (char *in);

float	*tv (float x, float y, float z);
char	*vtos (vec3_t v);

float vectoyaw (vec3_t vec);
void vectoangles (vec3_t vec, vec3_t angles);

// Knightmare added
char *GameDir (void);
char *SavegameDir (void);
void GameDirRelativePath (const char *filename, char *output, size_t outputSize);
void SavegameDirRelativePath (const char *filename, char *output, size_t outputSize);
void CreatePath (const char *path);
qboolean LocalFileExists (const char *path);
qboolean G_CopyFile (const char *src_filename, const char *dst_filename);
// end Knightmare

qboolean WeighPlayer(edict_t *ent);
void reinforcement_think(edict_t *ent);

void centerprintall (char *mesg, ...);

qboolean IsValidPlayer(edict_t *ent);

//
// g_combat.c
//
//qboolean OnSameTeam (edict_t *ent1, edict_t *ent2);

qboolean CanDamage (edict_t *targ, edict_t *inflictor);

void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod);
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);

//Wheaty: Stuff for blood-sprays
void BloodSprayThink (edict_t *self);
void blood_spray_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void SprayBlood(edict_t *self, vec3_t point, vec3_t angle, int damage, int mod);

//pbowens: for explosion effects
void SetExplosionEffect (edict_t *inflictor, float damage, float radius);
#endif



void Load_Weapon (edict_t *ent, gitem_t	*item);
char *ReadEntFile(char *filename);



// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage
#define DAMAGE_ENERGY			0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET			0x00000010  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION	0x00000020  // armor, shields, invulnerability, and godmode have no effect

#define DEFAULT_BULLET_HSPREAD	0
#define DEFAULT_BULLET_VSPREAD	500
#define DEFAULT_SHOTGUN_HSPREAD	1000
#define DEFAULT_SHOTGUN_VSPREAD	500
#define DEFAULT_DEATHMATCH_SHOTGUN_COUNT	12
#define DEFAULT_SHOTGUN_COUNT	12
#define DEFAULT_SSHOTGUN_COUNT	20

#ifndef USER_EXCLUDE_FUNCTIONS

//
// g_monster.c
//
void monster_fire_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int kick, int hspread, int vspread, int flashtype);
//void monster_fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int flashtype);
//void monster_fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect);
//void monster_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int flashtype);
//void monster_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype);
//void monster_fire_railgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int flashtype);

//void monster_fire_bfg (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int kick, float damage_radius, int flashtype);

void M_droptofloor (edict_t *ent);

void monster_think (edict_t *self);
void walkmonster_start (edict_t *self);
void swimmonster_start (edict_t *self);
void flymonster_start (edict_t *self);
void AttackFinished (edict_t *self, float time);
void monster_death_use (edict_t *self);
void M_CatagorizePosition (edict_t *ent);
qboolean M_CheckAttack (edict_t *self);

void M_FlyCheck (edict_t *self);

void M_CheckGround (edict_t *ent);


//
// g_misc.c
//
void ThrowHead (edict_t *self, char *gibname, int damage, int type);

void ThrowClientHead (edict_t *self, int damage);

void ThrowGib (edict_t *self, char *gibname, int damage, int type);
void BecomeExplosion1(edict_t *self);

//Wheaty: HeadShotGib
void HeadShotGib (edict_t *ent, vec3_t head_pos, vec3_t forward, vec3_t right, vec3_t player_vel);

//
// g_ai.c
//
void AI_SetSightClient (void);

void ai_stand (edict_t *self, float dist);
void ai_move (edict_t *self, float dist);
void ai_walk (edict_t *self, float dist);
void ai_turn (edict_t *self, float dist);
void ai_run (edict_t *self, float dist);
void ai_charge (edict_t *self, float dist);
int range (edict_t *self, edict_t *other);

void FoundTarget (edict_t *self);
qboolean infront (edict_t *self, edict_t *other);
qboolean visible (edict_t *self, edict_t *other);
qboolean FacingIdeal(edict_t *self);


//
// g_weapon.c
//
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin);
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick);
void fire_fragment (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);

void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean tracers_on);

//void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);

void fire_blaster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect, qboolean hyper);

void fire_tracer (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int mod);
//void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float time, float damage_radius, int team);
//bcass start - TNT
void fire_tnt (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float time, float damage_radius, int team);
//bcass end
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);

void fire_shell (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_rifle (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean calcv);
//void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);

//
// g_ptrail.c
//
void PlayerTrail_Init (void);
void PlayerTrail_Add (vec3_t spot);
void PlayerTrail_New (vec3_t spot);
edict_t *PlayerTrail_PickFirst (edict_t *self);
edict_t *PlayerTrail_PickNext (edict_t *self);
edict_t	*PlayerTrail_LastSpot (void);


//
// g_client.c
//

void respawn (edict_t *ent);
void BeginIntermission (edict_t *targ);
void PutClientInServer (edict_t *ent);
void InitClientPersistant (gclient_t *client);

void InitClientResp (gclient_t *client);

void InitBodyQue (void);
void ClientBeginServerFrame (edict_t *ent);

//
// g_player.c
//
void player_pain (edict_t *self, edict_t *other, float kick, int damage);
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

//
// g_svcmds.c
//
void	ServerCommand (void);

//
// p_view.c
//
void ClientEndServerFrame (edict_t *ent);

//
// p_hud.c
//
void MoveClientToIntermission (edict_t *client);

void G_SetStats (edict_t *ent);
void ValidateSelectedItem (edict_t *ent);
//void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer);
//void DeathmatchPlayerScore (edict_t *ent, edict_t *killer);

//
// g_pweapon.c
//
void PlayerNoise(edict_t *who, vec3_t where, int type);

//
// m_move.c
//
qboolean M_CheckBottom (edict_t *ent);
qboolean M_walkmove (edict_t *ent, float yaw, float dist);
void M_MoveToGoal (edict_t *ent, float dist);
void M_ChangeYaw (edict_t *ent);

//
// g_phys.c
//
void G_RunEntity (edict_t *ent);

//
// g_main.c
//
void SaveClientData (void);
void FetchClientEntData (edict_t *ent);
void EndDMLevel (void);

//
// g_maps.c
//
#include "g_maps.h"

//
// p_classes.c
//

void Give_Class_Weapon(edict_t *ent);
void Give_Class_Ammo(edict_t *ent);

//
// p_observer.c
//

#include "p_observer.h"

//
// g_cmds.c
//



#endif




//============================================================================

// client_t->anim_priority
#define	ANIM_BASIC		0		// stand / run
#define	ANIM_WAVE		1
#define	ANIM_JUMP		2
#define	ANIM_PAIN		3
#define	ANIM_ATTACK		4
#define	ANIM_DEATH		5
#define ANIM_CHANGESTANCE   6
//pbowens: v_wep
#define ANIM_REVERSE    -1

//the following is a generic container for mos's on a team. it is used to initialize the
//player on startup, as well as keep track how many slots are open for each mos.

typedef struct
{
	char *name;				//mos name
	char *skinname;			//mos skin (general model defined within user dll)
	char *weapon1;			//primary weapon
	int		ammo1;			//number of mags for w1
	char *weapon2;			//secondary weapon
	int		ammo2;			//number of mags for w2
	char *grenades;			//grenade type
	int grenadenum;			//number of grenades
	char *special;			//special weapon, if any
	int	specnum;			//number of special items given
	mos_t	mos;			//MOS type that struct represents...
	int		available;		//number allowed on team
	float	normal_weight;	//normal ammount ent may carry without speed effect
	float	max_weight;		//amount of weight before player cannot stand up (-normal_weight)
	float	speed_mod;		//class speed
	float	marksmanship;	// %chance of greater elevation
	char *MOS_Spaw_Point;	//name of spawn location

}SMos_t;

typedef struct
{
	char *teamname;	// name for each team
	int mission;
	edict_t *units[MAX_TEAM_MATES];
	int losses;		//losses for a level
	int kills;		//enemy kills for a level
	int total;//faf:  not used		//total on team. add stuff to count upon joining
	int index;		//index on team_list
	int score;		//current score of team
	float time_to_win;	//time before winning game
//	char *skindir;		//this is the subdir of skins that the skins are located

	SMos_t	**mos;

//	mos_t	officer_mos;	//this is the mos for the officer's old backup var
	char	teamid[64];		//letter that denotes which seperate wave file to use
	char	playermodel[64];	// the defined player model to use
	char	*nextmap;
	int		need_kills;		// Needed Kills to win level
	int		need_points;	// Needed Points to win level


	float	arty_fire_time;	// last time artillary was fired
	int		arty_fire_count; // how many times the battary has been fired

	char    *skin;//faf:  for mapper to set custom skin
	qboolean	kills_and_points; // if team need both minimum kills and minimum points to win
	qboolean chute;
	float		delay;//delay at start of map

}TeamS_t;



// client data that stays across multiple level loads

typedef struct
{
	char		userinfo[MAX_INFO_STRING];
	char		netname[16];
	int			hand;

	qboolean	connected;			// a loadgame will leave valid entities that
									// just don't have a connection yet

	// values saved and restored from edicts when changing levels
	int			health;
	int			max_health;
	qboolean	powerArmorActive;

	int			selected_item;
	int			inventory[MAX_ITEMS];

	// ammo capacities

	int			max_bullets;
	int			max_shells;
	int			max_rockets;
	int			max_grenades;
//bcass start - TNT
	int			max_tnt;
//bcass end
	int			max_30cal;
	int			max_slugs;
	int			max_flame;

	gitem_t		*weapon;
	gitem_t		*lastweapon;

//	int			power_cubes;	// used for tracking the cubes in coop games
//	int			score;			// for calculating total unit score in coop games


	int		stat_human_kills;
	int		stat_human_deaths;
	int		stat_bot_kills;
	int		stat_bot_deaths;

	char		stat_chat[150];


	char		*ip;


	int		afk_check_time;
	vec3_t	last_angles;

} client_persistant_t;


// client data that stays across deathmatch respawns

typedef struct
{
	client_persistant_t	coop_respawn;	// what to set client->pers to on a respawn
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
	vec3_t		cmd_angles;			// angles sent over in the last command
	int			game_helpchanged;
	int			helpchanged;
	TeamS_t		*team_on;			//pointer to team
	int			unit_index;			//RM-Index of player in team_list[x]->units[unit_index]
	float		deathblend;			//RM-For fading to black when dead
	int			scopewobble;		// rezmoth - scope wobble frame

	mos_t		mos;				//player's MOS
	mos_t		newmos;				//player's next mos

	//mos_t		smos;				//player's secondary MOS
	//mos_t		bkupmos;			//storage space for when player is inf

	qboolean	AlreadySpawned;		//keeps track of spawned or not
	qboolean	autopickup;			//whether or not to pickup weapons (weight)
	qboolean	show_id;			//whether or not to show player stats
	qboolean	changeteam;			// recently switched teams

	float		killtime;			// when player was killed
	float		oldkilltime;		// for comparisons

	//int			plus_minus;
	int			accuracy_hits;
	int			accuracy_misses;

	edict_t		*laser;

	char		chatsave[150];
	qboolean	chatsavetype;
	int			chatsavetime;

	int			stat_human_plus;
	int			stat_bot_plus;
	int			stat_human_minus;
	int			stat_bot_minus;
	int			stat_castrations;
	int			stat_helmets;
	int			stat_fists;
} client_respawn_t;


typedef struct

{
	int	pistol_rnd,
		pistol_fract,

		submg_rnd,
		submg_fract,

		rifle_rnd,
		rifle_fract,

		sniper_rnd,
		sniper_fract,

		lmg_rnd,
		lmg_fract,

		hmg_rnd,

		antitank_rnd,

		shotgun_rnd,
		shotgun_fract,

		submg2_rnd,
		submg2_fract
;

} mags_t;


// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'

struct gclient_s
{
	// known to server
	player_state_t	ps;				// communicated by server to clients
	int				ping;


	// inside the game
	client_persistant_t	pers;
	client_respawn_t	resp;
//	client_respawn_t	old_resp;

	pmove_state_t		old_pmove;	// for detecting out-of-pmove changes


	int			layout_type;
	/* get rid of this crap
	qboolean	showscores;			// set layout stat
	qboolean	showinventory;		// set layout stat
	int			showpscores;		// playerscores
	qboolean	showhelp;
	qboolean	showhelpicon;
	*/

	qboolean	inmenu;				// in menu

	pmenuhnd_t	*menu;				// current menu
	pmenu_t		 menu_cur[MENU_MAX];

	int			ammo_index;

	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	qboolean	weapon_thunk;

	gitem_t		*newweapon;
	gitem_t		*oldweapon;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_parmor;		// damage absorbed by power armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation
	float		damage_blendtime;	// how long to display damage

	float		killer_yaw;			// when dead, look at killer

	weaponstate_t	weaponstate;
	weaponstate_t	weaponstate_last;

	vec3_t		kick_angles;	// weapon kicks
	vec3_t		kick_origin;
	float		v_dmg_roll, v_dmg_pitch, v_dmg_time;	// damage kicks
	float		fall_time, fall_value;		// for view drop on fall
	float		damage_alpha;
	float		bonus_alpha;
	//float		deathfade;
	vec3_t		damage_blend;
	vec3_t		v_angle;			// aiming direction
	float		bobtime;			// so off-ground doesn't change it
	vec3_t		oldviewangles;
	vec3_t		oldvelocity;

	float		next_drown_time;
	int			old_waterlevel;
	int			old_watertype;
	int			breather_sound;

	int			machinegun_shots;	// for weapon raising

	// animation vars
	int			anim_end;
	int			anim_priority;
	qboolean	anim_duck;
	qboolean	anim_run;

	// powerup timers
	float		quad_framenum;
	float		invincible_framenum;
	float		breather_framenum;
	float		enviro_framenum;

	edict_t		*grenade;
	int			grenade_index;

//bcass start - TNT
	edict_t		*tnt;
//bcass end
	int			silencer_shots;
	int			weapon_sound;

	float		pickup_msg_time;
	float		respawn_time;			// can respawn when time > this
	int			next_fire_frame;

//bcass start - gibola
	int         gibmachine;
//bcass end
	//KMM: other

	int			aim;			// hawkins truesight

//	float		OBTime;					//delay before becoming solid
//	qboolean	feedammo;
	qboolean	limbo_mode;				//when player is dead in the neather regions

	mags_t		mags[MAX_TEAMS];		//structure that contains magazines
	int			*p_rnd,					//pointers for rnds
				*p_fract;				//and fractional mags
	int			flame_rnd;				//rounds for flamethrower
	float		speedmod;				//speed modifier based on weight
	int			speedmax[3];			//maximum speeds for the player

	//pbowens

	qboolean	display_info;			// If player is looking at objectives
	qboolean	crosshair;				// hack to get rid of crosshair
	qboolean	syncspeed;				// for cmd issues
	qboolean	ident;					// do they see a player id?
	qboolean	movement;				// is client moving?

	float		spawntime;
	float		cmdtime;				//
	float		medic_call;				// when "MEDIC!!!" timer runs out
	int			forcespawn;				// respawn force timer

	float		flood_locktill;			// locked from talking
	float		flood_when[10];			// when messages were said
	int			flood_whenhead;			// head pointer for when said

	int			penalty;				// penalty type
//	int			lastcmd;

//faf	qboolean	arty_called;
//faf	qboolean	arty_fired;
//faf	qboolean	arty_sound;
//faf	int			arty_num;
//faf	int			arty_location;
	vec3_t		arty_entry;
	vec3_t		arty_target;
//faf	float		arty_time_position;
//faf	float		arty_time_fire;
	float		arty_time_restrict;
	edict_t     *airstrike;//faf

	float		jump_stamina;
	qboolean	jump_push;
	qboolean	jump_stuck;
	qboolean	jump_last;

//bcass start - semi auto guns
	float		heldfire;  //needed for semi-autos
	float		damage_div;  //needed for hit slowing
//bcass end

	//pbowens
	int			dmgef_startframe;
	int			dmgef_sway_value;
	qboolean	dmgef_sway_switch;
	float		dmgef_intensity;
	qboolean	dmgef_flash;
	float		dmgef_ablend;

	qboolean	landed;
	qboolean	drop;		// player weapon

	// sniper bolt animation hack
	qboolean	sniper_loaded[MAX_TEAMS];


	//faf
	vec3_t		explosion_angles;

	float		last_jump_time;
	float		jump_pause_time;

	edict_t     *last_wound_inflictor;

	int			sidestep_anim;
	int			last_sidestep_anim;

	float		last_id_time;  //faf:  for delay on player id
	qboolean	anim_aim;
	qboolean	anim_crawl;
	float		last_shout_time;
	float       footstep_framenum;
	qboolean	bleedwarn;
	float		scopetime;//faf
	float		unscopetime;//faf

	float       last_fire_time;//faf

	qboolean    tank_hit;//faf

	int			anim_end2;//faf:  for alternate models

	float       enter_spawn_time;//faf
	int			spawn_kill_time;//faf

	float		spawn_safe_time;



	edict_t     *chasetarget;//faf

	edict_t     *turret;//faf


	float crosshair_offset_x;
	float crosshair_offset_y;


	float		last_menu_time;

	float		last_obj_dmg_time;
	int			last_obj_health;
	char		*last_obj_name;
	qboolean	scopetry;
	qboolean	reloadtry;
	int			reload_pause_frame;
	int			activate_framenum;
//	int			showobjectives;
//	float		obj_menu_time;
	float		gunwarntime;//stop warning spam

	vec3_t		screen_shake;

	float		show_obj_temp_time;

	float		smoke_effect_goal;
	float		smoke_effect_actual;

	qboolean	in_air;

	float		mg42_temperature;

	edict_t		*sandbag_preview;
	vec3_t		sandbag_pos;


	qboolean	vote_started;
	qboolean	voted;

	vec3_t		last_pos1, last_pos2, last_pos3;

	qboolean	has_chute;

	qboolean	has_briefcase;

	float		throw_grenade_time;
};


struct edict_s
{

	entity_state_t	s;
	struct gclient_s	*client;	// NULL if not a player
									// the server expects the first part
									// of gclient_s to be a player_state_t
									// but the rest of it is opaque
	qboolean	inuse;
	int			linkcount;

// FIXME: move these fields to a server private sv_entity_t
	link_t		area;				// linked to a division node or leaf

	int			num_clusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			headnode;			// unused if num_clusters != -1
	int			areanum, areanum2;

	//================================

	int			svflags;
	vec3_t		mins, maxs;
	vec3_t		absmin, absmax, size;
	solid_t		solid;
	int			clipmask;
	edict_t		*owner;


	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

	//================================
	int			index;
	int			movetype;
	int			flags;

	char		*model;
	float		freetime;			// sv.time when the object was freed

	//
	// only used locally in game, not by server
	//
	char		*message;
	char		*classname;
	int			classnameb;
	int			spawnflags;

	float		timestamp;

	float		angle;			// set in qe3, -1 = up, -2 = down
	char		*target;
	char		*targetname;
	char		*killtarget;
	char		*team;
	char		*pathtarget;
	char		*deathtarget;
	char		*combattarget;
	edict_t		*target_ent;


	float		speed, accel, decel;
	vec3_t		movedir;
	vec3_t		pos1, pos2;

	vec3_t		velocity;
	vec3_t		avelocity;
	int			mass;
	float		air_finished;
	float		gravity;		// per entity gravity multiplier (1.0 is normal)
								// use for lowgrav artifact, flares

	edict_t		*goalentity;
	edict_t		*movetarget;
	float		yaw_speed;
	float		ideal_yaw;

	float		nextthink;
	void		(*prethink) (edict_t *ent);
	void		(*think)(edict_t *self);
	void		(*blocked)(edict_t *self, edict_t *other);	//move to moveinfo?
	void		(*touch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
	void		(*use)(edict_t *self, edict_t *other, edict_t *activator);
	void		(*pain)(edict_t *self, edict_t *other, float kick, int damage);
	void		(*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

	float		touch_debounce_time;		// are all these legit?  do we need more/less of them?
	float		pain_debounce_time;
	float		damage_debounce_time;
	float		fly_sound_debounce_time;	//move to clientinfo
	float		last_move_time;

	int			health;
	int			max_health;
	int			gib_health;
	int			deadflag;
	qboolean	show_hostile;

	float		powerarmor_time;

	char		*map;			// target_changelevel


	int			viewheight;		// height above origin where eyesight is determined
	int			takedamage;
	int			dmg;
	int			radius_dmg;
	float		dmg_radius;
	int			sounds;			//make this a spawntemp var?
	int			count;
	int			mod;			//means of death

	edict_t		*chain;
	edict_t		*enemy;
	edict_t		*oldenemy;
	edict_t		*activator;
	edict_t		*groundentity;
	int			groundentity_linkcount;
	edict_t		*teamchain;
	edict_t		*teammaster;

	edict_t		*mynoise;		// can go in client only
	edict_t		*mynoise2;

	int			noise_index;
	int			noise_index2;
	float		volume;
	float		attenuation;

	// timing variables
	float		wait;
	float		delay;			// before firing targets
	float		random;

	float		teleport_time;

	int			watertype;
	int			waterlevel;
	int			oldwatertype;
	int			oldwaterlevel;

	vec3_t		move_origin;
	vec3_t		move_angles;

	// move this to clientinfo?
	int			light_level;

	int			style;			// also used as areaportal number

	gitem_t		*item;			// for bonus items

	//new variables for tracer
	int				numfired;
	// new var for stance
	int				stanceflags;
	int				stance_max,stance_min, stance_view;

	//new var for hit location
	int				wound_location;
//faf	int				die_time;
	float die_time;

	// common data blocks
	moveinfo_t		moveinfo;
	monsterinfo_t	monsterinfo;

	float movedown;

	char *obj_name;
	float obj_area;
	float obj_time;
	int	obj_owner;	//entity that owns this item
	int	obj_gain;
	int	obj_loss;
	int obj_count;
	int obj_perm_owner;

	edict_t **who_touched;	//used for entities that need to keep track of who touched
	qboolean spawnpoint;


//bcass start - TNT
	qboolean		armed_tnt;
//bcass end
/*-----/ PM /-----/ NEW:  Extra fields used in Napalm2 mod. /-----*/
    edict_t         *master;

	int             fireflags;
    float           burnout;
    edict_t         *burner;
/*----------------------------------------------------------------*/
	qboolean		flyingnun;  //For flying nun mode (ref/observer mode)
	qboolean		sexpistols; //these are for the sexpistols easteregg
	int				anarchy;
	float			nextspawn;	//what order to spawn
	int				distance;
//	qboolean		ident;

	float			leave_limbo_time;//faf
	int				oldstance;//faf

	vec3_t			last_turret_driver_spot;
	float			lava_debounce_time;


	ai_handle_t		*ai;		//jabot092(2)
	qboolean		is_swim;	//AI_CategorizePosition
	qboolean		is_step;
	qboolean		is_ladder;
	qboolean		was_swim;
	qboolean		was_step;

	qboolean		playedsound;  //for dropping guns/ammo

	vec3_t			obj_origin; //for bots to aim at

	char			*musictrack;	// Knightmare- for specifying OGG or CD track
};


typedef struct
{
	char	*name;
	void	(*spawn)(edict_t *ent);
} spawn_t;


// Knightmare- made this var extern to fix compile on GCC
extern TeamS_t *team_list[MAX_TEAMS];

void InitMOS_List(TeamS_t *team,SMos_t *mos_list);

extern int Last_Team_Winner;

extern qboolean	frame_output;

//extern char map_tree[MAX_TEAMS][MAX_QPATH];

#define DIZZYTIME 6 //seconds
#define	flame_normal 0
#define flame_gib 1

typedef struct
{
	int  limit;
} mapclasslimits_t;

// Knightmare- made this var extern to fix compile on GCC
extern mapclasslimits_t mapclasslimits[MAX_TEAMS][10];

// Knightmare- made these vars extern to fix compile on GCC
extern int	usa_index;
extern int	grm_index;
extern int	rus_index;
extern int	gbr_index;
extern int	pol_index;
extern int	ita_index;
extern int	jpn_index;
extern int	usm_index;

#define SMG_SPREAD 50//faf: raise a hair 40
#define PISTOL_SPREAD 50
#define LMG_SPREAD 80
#define HMG_SPREAD 100


//JABot[start]
#define WEAP_NONE			    0
#define WEAP_BLASTER			1
#define WEAP_SHOTGUN			2
#define WEAP_MACHINEGUN			3
#define WEAP_CHAINGUN			4
#define WEAP_GRENADES			5
#define WEAP_ROCKETLAUNCHER		6
#define WEAP_RIFLE				7
#define WEAP_FLAMER				8
#define WEAP_MELEE				9
#define WEAP_LAST               9

#define WEAP_TOTAL			    (WEAP_LAST + 1) // Knightmare- this was the wrong size (9)!
//JABot[end]


typedef enum
{
	CAMP_DISABLED,
	CAMP_NORMAL,
	CAMP_OBJECTIVE
} camp_t;

typedef struct
{
	int			team;
	vec3_t		origin;
	int			angle;
	int			stance;
	qboolean	type;
	edict_t		*owner;
} camp_spots_t;

// Knightmare- made these vars extern to fix compile on GCC
extern camp_spots_t	camp_spots[128];
extern int			total_camp_spots;
extern int			num_clients;
extern qboolean		qbots;

typedef struct
{
	char 		*bspname;
	char		*exita;
	char		*exitb;
	char		*exitc;
	int			owner;
	int			xpos;
	int			ypos;
	qboolean	alliedstart;
	qboolean	axisstart;
} campaign_spots_t;

// Knightmare- made these vars extern to fix compile on GCC
extern campaign_spots_t campaign_spots[50];

extern int alliedplatoons;
extern int	axisplatoons;
extern int	alliedneedspots;
extern int	axisneedspots;

extern int campaign_winner;


typedef enum
{
	SHOW_NONE,
	SHOW_SCORES,
	SHOW_PSCORES,
	SHOW_INVENTORY,
	SHOW_HELP,
	SHOW_HELPICON,
	SHOW_CAMPAIGN,
	SHOW_OBJECTIVES,
	SHOW_OBJECTIVES_TEMP,
	SHOW_MEDIC_SCREEN,
	SHOW_SERVERIMG
} display_t;

// Knightmare- made this var extern to fix compile on GCC
extern char	*last_maps_played[20];


typedef enum
{
	ITEM_BRIEFCASE,
	ITEM_HELMET,
	WEAPON_KNIFE,
	WEAPON_FISTS,
	WEAPON_BINOCULARS,
	WEAPON_MORPHINE,
	WEAPON_FLAMETHROWER,
	WEAPON_COLT45,
	WEAPON_M1,
	WEAPON_THOMPSON,
	WEAPON_BAR,
	WEAPON_30CAL,
	WEAPON_BAZOOKA,
	WEAPON_SNIPER,
	WEAPON_P38,
	WEAPON_MAUSER98K,
	WEAPON_MP40,
	WEAPON_MP43,
	WEAPON_MG42,
	WEAPON_PANZER,
	WEAPON_TT33,
	WEAPON_M9130,
	WEAPON_PPSH41,
	WEAPON_PPS43,
	WEAPON_DPM,
	WEAPON_RPG1,
	WEAPON_M9130S,
	WEAPON_WEBLEY,
	WEAPON_ENFIELD,
	WEAPON_STEN,
	WEAPON_BREN,
	WEAPON_VICKERS,
	WEAPON_PIAT,
	WEAPON_ENFIELDS,
	WEAPON_VIS,
	WEAPON_SVT,
	WEAPON_RKM,
	WEAPON_MG34,
	WEAPON_MORS,
	WEAPON_POL_M98KS,
	WEAPON_SABRE,
	WEAPON_B34,
	WEAPON_CARCANO,
	WEAPON_B38,
	WEAPON_K43,
	WEAPON_3842,
	WEAPON_PANZERFAUST,
	WEAPON_M98KS,
	WEAPON_BREDA,
	WEAPON_NAMBU,
	WEAPON_ARISAKA,
	WEAPON_TYPE_100,
	WEAPON_TYPE_99,
	WEAPON_ARISAKAS,
	WEAPON_KATANA,
	WEAPON_MOLOTOV,
	WEAPON_SHOTGUN,
	WEAPON_SANDBAGS,
	SANDBAGS,
	AMMO_TNT,
	AMMO_NAPALM,
	AMMO_COLT45,
	AMMO_M1,
	AMMO_THOMPSON,
	AMMO_BAR,
	AMMO_HMG,
	AMMO_ROCKETS,
	AMMO_M1903,
	AMMO_GRENADES_GRM,
	AMMO_P38,
	AMMO_MAUSER98K,
	AMMO_MP40,
	AMMO_MP43,
	AMMO_ROCKETSG,
	AMMO_GRENADES_RUS,
	AMMO_TT33,
	AMMO_M9130,
	AMMO_PPSH41,
	AMMO_PPS43,
	AMMO_DPM,
	AMMO_ROCKETSR,
	AMMO_GRENADES_GBR,
	AMMO_WEBLEY,
	AMMO_ENFIELD,
	AMMO_STEN,
	AMMO_BREN,
	AMMO_VICKERS,
	AMMO_PIAT,
	AMMO_ENFIELDS,
	AMMO_GRENADES_POL,
	AMMO_VIS,
	AMMO_POL_M98K,
	AMMO_SVT,
	AMMO_RKM,
	AMMO_MG34,
	AMMO_MORS,
	AMMO_GRENADES_ITA,
	AMMO_B34,
	AMMO_CARCANO,
	AMMO_B38,
	AMMO_K43,
	AMMO_B3842,
	AMMO_ROCKETSI,
	AMMO_MAUSER98KS,
	AMMO_BREDA,
	AMMO_GRENADES_JPN,
	AMMO_NAMBU,
	AMMO_ARISAKA,
	AMMO_ARISAKAS,
	AMMO_TYPE_100,
	AMMO_TYPE_99,
	AMMO_MG42,
	AMMO_GRENADES_USA,
	AMMO_SHOTGUN,
	FUNC_EXPLOSIVE,
	FUNC_EXPLOSIVE_OBJECTIVE,
	TIMED_OBJECTIVE_TOUCH,
	OBJECTIVE_TOUCH,
	FUNC_PLAT,
	FUNC_DOOR,
	FUNC_DOOR_ROTATING,
	ITEM_BOTROAM,
	TRIGGER_PUSH,
	TRIGGER_TELEPORT,
	FIRE,
	TNT,
	HGRENADE,
	ROCKET,
	FUNC_TRAIN,
	BOTWARN,
	OBJECTIVE_VIP
} classnameb_t;

typedef enum
{
	SOUND_WOOD,
	SOUND_METAL,
	SOUND_GLASS,
	SOUND_SAND,
	SOUND_CONCRETE
} soundtype_t;


// Knightmare- made these vars extern to fix compile on GCC
extern qboolean no_objectives_left;

extern qboolean dropnodes;

extern int		allied_sandbags;
extern int		axis_sandbags;

extern char		*votemaps[5];
extern int		mapvotes[5];


#define MAX_TEAM_ITEMS 19
#define NUM_CLASSES 10
#define MAX_TEAM_GUNS 8


qboolean objective_hittable (edict_t *self, edict_t *objective, vec3_t orig);
void change_stance(edict_t *self, int stance);
qboolean Cmd_Scope_f(edict_t *ent);
void AI_MoveToDodgeSpot(edict_t *self, usercmd_t *ucmd);
qboolean toright (edict_t *self, vec3_t point);
qboolean Surface(char *name, int type);
void AI_PathMap( qboolean force );
void Cmd_Wave_f (edict_t *ent, int wave);
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0, qboolean saved);
void Medic_Screen (edict_t *ent);
void Cmd_Objectives (edict_t *ent);
void ShowCampaign (edict_t *ent);
qboolean MapExists (char *map);
void MapVote(edict_t *ent);
void Play_Footstep_Sound (edict_t *ent);
void Update_Campaign_Info (void);
void WriteCampaignTxt(void);
void SetupCampaign (qboolean restart);
int PlayerCountForTeam (int team_number, qboolean countbots);
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_LFIRE_LAST, int FRAME_LIDLE_LAST, int FRAME_RELOAD_LAST, int FRAME_LASTRD_LAST,
					 int FRAME_DEACTIVATE_LAST, int FRAME_RAISE_LAST,int FRAME_AFIRE_LAST, int FRAME_AIDLE_LAST,
					 int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
gitem_t	*FindNextPickup (edict_t *ent, int location);
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
void ShowServerImg (edict_t *ent);
void M_ChooseMOS(edict_t *ent);
void AI_PickLongRangeGoal(edict_t *self);
void AI_ResetWeights(edict_t *ent);
void AI_ResetNavigation(edict_t *ent);
void Write_Player_Stats (edict_t *ent);
void SetPlayerRating(edict_t *ent);
qboolean hmgvisible (edict_t *self, edict_t *other);
void Spawn_Chute_Special(edict_t *ent);
void SP_objective_VIP (edict_t *self);
void VIP_think (edict_t *self);
void Set_VIP_Defense_Bot (edict_t *ent);
int Surface2(char *name);
void Find_Mission_Start_Point(edict_t *ent, vec3_t origin, vec3_t angles);
void SP_objective_flag (edict_t *self);
void briefcase_warn (edict_t *ent);//faf;  ctb code
qboolean Pickup_Briefcase (edict_t *ent, edict_t *other);
void Drop_Briefcase (edict_t *ent, gitem_t *item);
void Remove_Nearby_Sandbags(edict_t *ent);
void ParseBotChat (char *text, edict_t *attacker);
char *SeekBufEnd(char *buf);
void explosion_effect (vec3_t start, int type);
void Cmd_Arty_f (edict_t *ent);
gitem_t	*FindItem (char *pickup_name);
void Helmet_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void func_explosive_objective_explode (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void train_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
int Play_Bullet_Hit(edict_t *ent, char *surface, vec3_t endpos, edict_t *impact_ent);


// Knightmare- made these vars extern to fix compile on GCC
extern char botchat_taunt[MAX_TEAMS][200][150];
extern int  botchat_taunt_count[MAX_TEAMS];
extern char botchat_sorry[MAX_TEAMS][200][150];
extern int  botchat_sorry_count[MAX_TEAMS];
extern char botchat_brag[MAX_TEAMS][200][150];
extern int  botchat_brag_count[MAX_TEAMS];
extern char botchat_tked[MAX_TEAMS][200][150];
extern int  botchat_tked_count[MAX_TEAMS];
extern char botchat_insult[MAX_TEAMS][200][150];
extern int  botchat_insult_count[MAX_TEAMS];
extern char botchat_forgive[MAX_TEAMS][200][150];
extern int  botchat_forgive_count[MAX_TEAMS];
extern char botchat_random[MAX_TEAMS][200][150];
extern int  botchat_random_count[MAX_TEAMS];
extern char botchat_killed[MAX_TEAMS][200][150];
extern int  botchat_killed_count[MAX_TEAMS];
extern char botchat_self[MAX_TEAMS][200][150];
extern int  botchat_self_count[MAX_TEAMS];

extern char user_shouts[20][100];
extern int user_shout_count;
