// g_local.h -- local definitions for game module

#ifdef _WIN32
#include <malloc.h>
#include <direct.h>
#endif

#include "./q_shared.h"
#include "laz_misc.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "game.h"
#include "p_menu.h"
#include "p_text.h"
#include "km_cvar.h"
#include "plasma.h"
#define JETPACK_MOD

// Zaero
// some custom defines
//#define USE_ZAERO_ITEMS_WEAPONS	// enable for Zaero player weapons
#ifdef Z_MAX
#undef Z_MAX
#endif // Z_MAX
#define Z_MAX(a,b)	((a) > (b) ? (a) : (b))

#ifdef Z_MIN
#undef Z_MIN
#endif // Z_MIN
#define Z_MIN(a,b)	((a) < (b) ? (a) : (b))

#define Z_MALLOC(size)	gi.TagMalloc(size, TAG_GAME)
#define Z_FREE(block)	gi.TagFree(block)
// end Zaero

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"Q2MP4"

// protocol bytes that can be directly added to messages
#define	svc_muzzleflash		1
#define	svc_muzzleflash2	2
#define	svc_temp_entity		3
#define	svc_layout			4
#define	svc_inventory		5
#define	svc_stufftext		11
#define	svc_fog				21

//==================================================================

#ifndef _WIN32
//#define min(a,b) ((a) < (b) ? (a) : (b))
//#define max(a,b) ((a) > (b) ? (a) : (b))
#ifdef __sun__
#include <nan.h>
#define _isnan(a) (NaN((a)))
#else
#define _isnan(a) ((a)==NAN)
#endif
#endif

//==================================================================

// Lazarus: When visibility is reduced below this level, aiming accuracy suffers:
#define FOG_CANSEEGOOD 0.12

// view pitching times
#define DAMAGE_TIME		0.5
#define	FALL_TIME		0.3

// ROGUE- id killed this weapon
//Knightmare brought it back
//#define	KILL_DISRUPTOR	1
// rogue

// edict->spawnflags
// these are set with checkboxes on each entity in the map editor
#define	SPAWNFLAG_NOT_EASY			0x00000100	// 256
#define	SPAWNFLAG_NOT_MEDIUM		0x00000200	// 512
#define	SPAWNFLAG_NOT_HARD			0x00000400	// 1024
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00000800	// 2048
#define	SPAWNFLAG_NOT_COOP			0x00001000	// 4096

// Zaero start
// edict->spawnflags2
// these are set with checkboxes on each entity in the map editor
#define	SPAWNFLAG2_MIRRORLEVEL  0x0001
#define	SPAWNFLAG2_NOT_COOP		0x0002
#define SPAWNFLAG2_NOT_SINGLE   0x0004
// end Zaero

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
#define FL_POWER_SHIELD			0x00001000	// power armor (if any) is active
#define FL_POWER_SCREEN			0x00002000
// ROGUE
#define FL_MECHANICAL			0x00004000	// entity is mechanical, use sparks not blood
#define FL_SAM_RAIMI			0x00008000	// entity is in sam raimi cam mode
#define FL_DISGUISED			0x00010000	// entity is in disguise, monsters will not recognize.
#define	FL_NUKED				0x00020000	// player has been vaporized by a nuke, drop no gibs
// ROGUE
// Mappack - this flag says the player is in a turret and controlling it
#define	FL_TURRET_OWNER			0x00040000
// Knightmare- this says that an entity is set to move with another
#define	FL_MOVESLAVE			0x00080000

// Knightmare- flags for trains
#define FL_TRACKTRAIN			0x00100000
#define	FL_ROTTRAIN				0x00200000
#define	FL_REVERSIBLE			0x00400000
#define	FL_REVOLVING			0x00800000
#define	FL_BOB					0x01000000
#define FL_ROBOT				0x02000000	// Player-controlled robot or monster. Relax yaw constraints
#define FL_REFLECT              0x04000000	// Reflection entity

#define FL_Q1_MONSTER			0x08000000	

#define FL_RESPAWN				0x80000000	// used for item respawning

// edict->moreflags
#define	FL2_TURRET_DOUBLE		0x00000001	// this is a double-barreled turret
#define	FL2_TURRET_DOUBLE_ALT	0x00000002	// this turret alternates firing its barrels (style is set)
#define	FL2_TURRET_DOUBLE_ALT_FIRING	0x00000004	// secondary barrel in use for alternate firing
#define FL2_CRUCIFIED			0x00000008	// insane is crucified 
#define FL2_COMMANDER			0x00000008	// Medic Commander / Daedalus internal flag
#define FL2_WEAPON_ALT			0x00000010	// plasma guard has spread mode
#define FL2_DO_NOT_REFLECT		0x00000020	// do not reflect this entity


#define	FRAMETIME		0.1

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level


#define MELEE_DISTANCE	80

#define BODY_QUEUE_SIZE		8

typedef enum
{
	DAMAGE_NO,
	DAMAGE_YES,			// will take damage if hit
	DAMAGE_AIM,			// auto targeting recognizes this
	// Zaero
	DAMAGE_IMMORTAL		// similar to DAMAGE_YES, but health is not deducted
} damage_t;

typedef enum 
{
	WEAPON_READY, 
	WEAPON_ACTIVATING,
	WEAPON_DROPPING,
	WEAPON_FIRING
} weaponstate_t;

typedef enum
{
	AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_GRENADES,
	AMMO_CELLS,
	AMMO_SLUGS,
	// Xatrix
	AMMO_MAGSLUG,
	AMMO_TRAP,
	// ROGUE
	AMMO_FLECHETTES,
	AMMO_TESLA,
	AMMO_PROX,
	AMMO_DISRUPTOR,
	// Knightmare added
	AMMO_SHOCKSPHERE,
	AMMO_FUEL,
	AMMO_HOMING_ROCKETS,
	// Zaero (unused for now)
	AMMO_FLARES,
	AMMO_LASERTRIPBOMB,
	AMMO_EMPNUKE,
	AMMO_A2K,
	AMMO_PLASMASHIELD
} ammo_t;


//deadflag
#define DEAD_NO					0
#define DEAD_DYING				1
#define DEAD_DEAD				2
#define DEAD_RESPAWNABLE		3
#define DEAD_FROZEN             4  // Lazarus: Don't shift angles, just freeze him

//range
#define RANGE_MELEE				0
#define RANGE_NEAR				1
#define RANGE_MID				2
#define RANGE_FAR				3

//gib types
#define GIB_ORGANIC				0
#define GIB_METALLIC			1
#define GIB_FEATHER				2	// Knightmare- added for vulture

// monster ai flags
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
#define AI_MANUAL_STEERING		0x00008000
#define AI_TARGET_ANGER			0x00010000	// Lazarus keep
#define AI_DODGING				0x00020000
#define AI_CHARGING				0x00040000
#define AI_HINT_PATH			0x00080000	// Lazarus keep
#define	AI_IGNORE_SHOTS			0x00100000
#define	AI_BLOCKED				0x00200000	// used by blocked_checkattack: set to say I'm attacking while blocked 
											// (prevents run-attacks)
// Lazarus:
#define AI_ACTOR                0x00400000  // Is this a misc_actor?
#define AI_FOLLOW_LEADER        0x00800000  // misc_actor only
#define AI_TWO_GUNS             0x01000000  // misc_actor only - nothing to do with AI really,
                                            // but we're out of spawnflags
#define AI_RESPAWN_FINDPLAYER   0x02000000  // used for monsters that change maps with
                                            // a trigger_transition... tells 'em to find SP
                                            // player right away
#define AI_CHASE_THING          0x04000000
#define AI_SEEK_COVER           0x08000000
#define AI_CHICKEN              0x10000000
#define AI_MEDIC_PATROL         0x20000000
#define AI_CROUCH               0x40000000
#define AI_EVADE_GRENADE		0x80000000

// Knightmare- thes are for aiflags2
#define AI2_FREEFORALL           0x00000001	// Set by target_monsterbattle, lets dmgteam monsters
                                            // attack monsters on opposion dmgteam
#define AI2_RANGE_PAUSE          0x00000002
#define AI2_HINT_TEST            0x00000004
// Zaero
#define AI2_SCHOOLING			 0x00000008
#define AI2_REDUCEDDAMAGE		 0x00000010
#define AI2_DODGETIMEOUT		 0x00000020
#define AI2_MONREDUCEDDAMAGE	 0x00000040
#define AI2_ONESHOTTARGET		 0x00000080
// end Zaero
#define AI2_LEAD_TARGET          0x00000100


// Knightmare- monster flags
#define MFL_WALK_WALLS			1
#define	MFL_DO_NOT_COUNT		2	// set for healed monsters
#define	MFL_SPAWNED_CARRIER		4	// both do_not_count and spawned are set for spawned monsters
#define	MFL_SPAWNED_MEDIC_C		8	// both do_not_count and spawned are set for spawned monsters
#define	MFL_SPAWNED_WIDOW		16	// both do_not_count and spawned are set for spawned monsters
#define	MFL_SPAWNED_MASK		28	// mask to catch all three flavors of spawned
#define	MFL_KNOWS_PROX_MINES	32	// this monster know prox mines

// monster attack state
#define AS_STRAIGHT				1
#define AS_SLIDING				2
#define	AS_MELEE				3
#define	AS_MISSILE				4
#define	AS_BLIND				5	// PMM - used by boss code to do nasty things even if it can't see you

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
#define SFL_CROSS_TRIGGER_MASK	0x000000ff	// 0xffffe0ff would allow 27 trigger bits


// noise types for PlayerNoise
#define PNOISE_SELF				0
#define PNOISE_WEAPON			1
#define PNOISE_IMPACT			2


// actor follow parms
#define ACTOR_FOLLOW_RUN_RANGE   256    // AI_FOLLOW_LEADER actors run if farther away than this
#define ACTOR_FOLLOW_STAND_RANGE 128    //       ..          ..    stand if closer than this

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
MOVETYPE_BOUNCE,
MOVETYPE_NEWTOSS,		// PGM - for deathball
// RAFAEL
MOVETYPE_WALLBOUNCE,
MOVETYPE_VEHICLE,
MOVETYPE_PUSHABLE,		// Pushable object
MOVETYPE_DEBRIS,		// falling rocks
MOVETYPE_RAIN,			// identical to MOVETYPE_FLYMISSILE, but doesn't cause splash noises
						//   when touching water.
MOVETYPE_PENDULUM,		// same as MOVETYPE_PUSH, but used only for pendulums to grab special-case

MOVETYPE_CONVEYOR,		// conveyor
MOVETYPE_SHOCKBOUNCE,	// Knightmare- added for shockwave
MOVETYPE_FEATHER,		// Knightmare- added for vulture, specifies slow falling
// Zaero
MOVETYPE_FREEZE,		// player freeze, used for Zaero Camera
MOVETYPE_FALLFLOAT,		// falls down slopes and floats in water
MOVETYPE_RIDE			// basically won't move unless it rides on a MOVETYPE_PUSH entity
// end Zaero
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
#define	IT_WEAPON			0x00000001		// use makes active weapon
#define	IT_AMMO				0x00000002
#define IT_ARMOR			0x00000004
#define IT_STAY_COOP		0x00000008
#define IT_KEY				0x00000010
#define IT_POWERUP			0x00000020

// ROGUE
#define IT_MELEE			0x00000040
#define IT_NOT_GIVEABLE		0x00000080	// item can not be given
// ROGUE
// Knightmare added
#define IT_XATRIX			0x00000100	// Xatrix item
#define IT_ROGUE			0x00000200	// Rogue item
#define IT_LAZARUS			0x00000400	// Lazarus item
#define IT_LM				0x00000800	// Loki's Missions item
#define IT_ZAERO			0x00001000	// Zaero item
#define IT_Q1				0x00002000	// added for Q1 keys

// gitem_t->weapmodel for weapons indicates model index
#define WEAP_BLASTER			1 
#define WEAP_SHOTGUN			2 
#define WEAP_SUPERSHOTGUN		3 
#define WEAP_MACHINEGUN			4 
#define WEAP_CHAINGUN			5 
#define WEAP_GRENADES			6 
#define WEAP_GRENADELAUNCHER	7 
#define WEAP_ROCKETLAUNCHER		8 
#define WEAP_HYPERBLASTER		9 
#define WEAP_RAILGUN			10
#define WEAP_BFG				11

#define WEAP_DISRUPTOR			12		// PGM
#define WEAP_ETFRIFLE			13		// PGM
#define WEAP_PLASMA				14		// PGM
#define WEAP_PROXLAUNCH			15		// PGM
#define WEAP_CHAINFIST			16		// PGM
#define WEAP_PHALANX			17
#define WEAP_BOOMER				18
#define WEAP_SHOCKWAVE			19
#define WEAP_PLASMARIFLE		20
#define WEAP_TRAP				21
#define WEAP_TESLA				22
#define WEAP_GRAPPLE			23
#define WEAP_NONE               24


typedef struct gitem_s
{
	char		*classname;	// spawning name
	qboolean	(*pickup)(struct edict_s *ent, struct edict_s *other);
	void		(*use)(struct edict_s *ent, struct gitem_s *item);
	void		(*drop)(struct edict_s *ent, struct gitem_s *item);
	void		(*weaponthink)(struct edict_s *ent);
	char		*pickup_sound;
	char		*world_model;
	int			world_model_skinnum; //Knightmare- added skinnum here since some items share models
	int			world_model_flags;
	char		*view_model;

	// client side info
	char		*icon;
	char		*pickup_name;	// for printing on pickup
	int			count_width;		// number of digits to display by icon

	int			quantity;		// for ammo how much, for weapons how much is used per shot
	char		*ammo;			// for weapons
	int			flags;			// IT_* flags

	int			weapmodel;		// weapon model index (for weapons)

	void		*info;
	int			tag;

	char		*precaches;		// string of all models, sounds, and images this item will use
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

	// Lazarus: target_lock combination
	char		lock_code[9];
	int			lock_revealed;
	qboolean	lock_hud;
	// Lazarus: number of entities moved between maps (not counting players)
	int			transition_ents;

	// items
	int			num_items;

	qboolean	autosaved;
} game_locals_t;

struct fog_s
{
	qboolean	Trigger;
	int			Model;
	float		Near;
	float		Far;
	float		Density;
	float		Density1;
	float		Density2;
	vec3_t		Dir;
	int			GL_Model;
	vec3_t		Color;
	edict_t		*ent;
};
typedef struct fog_s fog_t;

// Knightmare- map vendor for pack-specific code changes
typedef enum
{
	MAPTYPE_ID,
	MAPTYPE_XATRIX,
	MAPTYPE_ROGUE,
	MAPTYPE_ZAERO,
	MAPTYPE_CUSTOM
} maptype_t;

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

	// ROGUE
	edict_t		*disguise_violator;
	int			disguise_violation_framenum;
	// end ROGUE

	// Lazarus
	int			num_fogs;
	int			num_trigger_fogs;
	int			active_target_fog;
	int			active_fog;
	int			last_active_fog;
	fog_t		current_fog;
	int			flashlight_cost;	// cost/10 seconds for flashlight
	int			mud_puddles;
	int			num_3D_sounds;
	qboolean	restart_for_actor_models;
	qboolean	freeze;
	int			freezeframes;
	int			next_skill;
	int			num_reflectors;
	qboolean	intermission_letterbox;		// Knightmare- letterboxing
	// end Lazarus

	// Zaero
	int			fadeFrames;

	// Knightmare- for map-specific logic switching
	maptype_t	maptype;				// map vendor for pack-specific code changes
	qboolean	isZaeroRailgunHackMap;	// railgun thru window hack for zdef4
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
	float		phase;

	float		shift;
} spawn_temp_t;


typedef struct
{
	// fixed data
	vec3_t		start_origin;
	vec3_t		start_angles;
	vec3_t		end_origin;
	vec3_t		end_angles;
	// Knightmare- these are for the bezier curves
//	vec3_t		last_pathpoint_origin;
//	vec3_t		last_pathpoint_angles;
//	vec3_t		bezier_begin_point;
///	vec3_t		bezier_start_origin;
//	vec3_t		bezier_end_origin;
//	vec3_t		bezier_start_angles;
//	vec3_t		bezier_end_angles;
//	float		bezier_speed;
//	float		bezier_distance;
	float		bezier_ratio; //Decimal position in beizier curve

	int			sound_start;
	int			sound_middle;
	int			sound_end;
	int			is_blocked;

	float		accel;
	float		speed;
	float		decel;
	float		distance;
	float		wait;

	// state data
	int			state;
	int			prevstate;
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
	mmove_t		*savemove;
	unsigned int	aiflags;		// PGM - unsigned, since we're close to the max
	unsigned int	aiflags2;		// Knightmare- more AI flags, needed for Lazarus stuff
	unsigned int	monsterflags;	// Knightmare- moved some of the Rogue AI flags here
	int			nextframe;
	float		scale;

	void		(*stand)(edict_t *self);
	void		(*idle)(edict_t *self);
	void		(*search)(edict_t *self);
	void		(*walk)(edict_t *self);
	void		(*run)(edict_t *self);
	void		(*dodge)(edict_t *self, edict_t *other, float eta, trace_t *tr);
	void		(*attack)(edict_t *self);
	void		(*melee)(edict_t *self);
	void		(*sight)(edict_t *self, edict_t *other);
	qboolean	(*checkattack)(edict_t *self);
	void		(*jump)(edict_t *self);

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

	// Mappack - array for ai.
	//vec3_t	radial_chk[8];

	// Mappack - for the pathing rountine
	qboolean	following_nodes;
	edict_t		*target_node;

	// Mappack - for buoy system
	edict_t		*buoy;

	// Mappack - damit I need one
	vec3_t		tempvec;

// ROGUE
	qboolean	(*blocked)(edict_t *self, float dist);
//	edict_t		*last_hint;			// last hint_path the monster touched
	float		last_hint_time;		// last time the monster checked for hintpaths.
	edict_t		*goal_hint;			// which hint_path we're trying to get to
	int			medicTries;
	edict_t		*badMedic1, *badMedic2;	// these medics have declared this monster "unhealable"
	edict_t		*healer;	// this is who is healing this monster
	void		(*duck)(edict_t *self, float eta);
	void		(*unduck)(edict_t *self);
	void		(*sidestep)(edict_t *self);
	//  while abort_duck would be nice, only monsters which duck but don't sidestep would use it .. only the brain
	//  not really worth it.  sidestep is an implied abort_duck
//	void		(*abort_duck)(edict_t *self);
	float		base_height;
	float		next_duck_time;
	float		duck_wait_time;
	edict_t		*last_player_enemy;
	// blindfire stuff .. the boolean says whether the monster will do it, and blind_fire_time is the timing
	// (set in the monster) of the next shot
	qboolean	blindfire;		// will the monster blindfire?
	float		blind_fire_delay;
	vec3_t		blind_fire_target;
	// used by the spawners to not spawn too much and keep track of #s of monsters spawned
	int			monster_slots;
	int			monster_used;
	edict_t		*commander;
	// powerup timers, used by widow, our friend
	float		quad_framenum;
	float		double_framenum;
	float		invincible_framenum;
// ROGUE
	edict_t		*leader;
	edict_t		*old_leader;
// ROGUE
// Lazarus
	float		min_range;		// Monsters stop chasing enemy at this distance
	float		max_range;		// Monsters won't notice or attack targets farther than this
	float		ideal_range[2];	// Ideal low and high range from target, weapon-specific
	float		flies;			// Probability of dead monster generating flies
	float		jumpup;
	float		jumpdn;
	float		rangetime;
	int			chicken_framenum;
	int			pathdir;		// Up/down a hint_path chain flag for medic
	float		visibility;		// Ratio of visibility (it's a fog thang)
// end Lazarus
// Zaero
	int			flashTime;
	int			flashBase;

	// strafing
	float		flyStrafePitch;
	float		flyStraanimfeTimeout;

	// schooling info
	float		zSchoolSightRadius;
	float		zSchoolMaxSpeed, zSchoolMinSpeed;
	float		zSpeedStandMax, zSpeedWalkMax;
	float		zSchoolDecayRate, zSchoolMinimumDistance;
	int			zSchoolFlags;

	float		reducedDamageAmount;
	float		dodgetimeout;

	vec3_t		shottarget;
// end Zaero
} monsterinfo_t;

// ROGUE
// this determines how long to wait after a duck to duck again.  this needs to be longer than
// the time after the monster_duck_up in all of the animation sequences
#define	DUCK_INTERVAL	0.5
// ROGUE

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

extern	int	noweapon_index;
extern	int	jacket_armor_index;
extern	int	combat_armor_index;
extern	int	body_armor_index;
extern	int	shells_index;
extern	int	bullets_index;
extern	int	grenades_index;
extern	int	rockets_index;
extern	int	cells_index;
extern	int	slugs_index;
extern	int fuel_index;
extern	int	homing_index;
extern	int	blaster_index;
extern	int	rl_index;
extern	int	hml_index;
extern	int	pl_index;
extern	int	rg_index;
extern	int	pr_index;		// SKWiD MOD
extern	int magslug_index;
extern	int flechettes_index;
extern	int	prox_index;
extern	int	disruptors_index;
extern	int tesla_index;
extern	int trap_index;
extern	int shocksphere_index;

// added for convenience with triger_key sound hack
extern	int	key_q1_med_silver_index;
extern	int	key_q1_med_gold_index;
extern	int	key_q1_rune_silver_index;
extern	int	key_q1_rune_gold_index;
extern	int	key_q1_base_silver_index;
extern	int	key_q1_base_gold_index;

// mxd added
extern int gibsthisframe;
extern int lastgibframe;

// means of death
#define MOD_UNKNOWN			0
#define MOD_BLASTER			1
#define MOD_SHOTGUN			2
#define MOD_SSHOTGUN		3
#define MOD_MACHINEGUN		4
#define MOD_CHAINGUN		5
#define MOD_GRENADE			6
#define MOD_G_SPLASH		7
#define MOD_ROCKET			8
#define MOD_R_SPLASH		9
#define MOD_HYPERBLASTER	10
#define MOD_RAILGUN			11
#define MOD_BFG_LASER		12
#define MOD_BFG_BLAST		13
#define MOD_BFG_EFFECT		14
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

// Xatrix
#define MOD_RIPPER				34
#define MOD_PHALANX				35
#define MOD_BRAINTENTACLE		36
#define MOD_BLASTOFF			37
#define MOD_GEKK				38
#define MOD_TRAP				39
// Xatrix

// ROGUE
#define MOD_CHAINFIST			40
#define MOD_DISINTEGRATOR		41
#define MOD_ETF_RIFLE			42
#define MOD_BLASTER2			43
#define MOD_HEATBEAM			44
#define MOD_TESLA				45
#define MOD_PROX				46
#define MOD_NUKE				47
#define MOD_NBOMB               48
#define MOD_VENGEANCE_SPHERE	49
#define MOD_HUNTER_SPHERE		50
#define MOD_DEFENDER_SPHERE		51
#define MOD_TRACKER				52
#define MOD_DBALL_CRUSH			54
#define MOD_DOPPLE_EXPLODE		55
#define MOD_DOPPLE_VENGEANCE	56
#define MOD_DOPPLE_HUNTER		57
// end ROGUE

// Knightmare
#define MOD_SHOCK_SPHERE		58
#define MOD_SHOCK_SPLASH		59
#define MOD_PROX_SPLASH			60
#define MOD_PHALANX_SPLASH		61
#define MOD_ETF_SPLASH			62
#define	MOD_FALLING_ROCKS		63
#define MOD_VEHICLE				65
#define MOD_KICK				66
#define MOD_MISSILE				67
#define MOD_MISSILE_SPLASH		68
#define	MOD_PLASMA				69	// SKWiD MOD

// Zaero
#define MOD_SNIPERRIFLE			70
#define MOD_TRIPBOMB		    71
#define MOD_FLARE				72
#define MOD_A2K					73
#define MOD_SONICCANNON			74
#define MOD_AUTOCANNON			75
#define MOD_GL_POLYBLEND		76
// end Zaero

//===============================
// Extra MODs
// Quake1     -Skid
//===============================
#define MOD_Q1_AXE				76
#define MOD_Q1_SG				77
#define MOD_Q1_SSG				78
#define MOD_Q1_NG				79
#define MOD_Q1_SNG				80
#define MOD_Q1_GL				81
#define MOD_Q1_RL				82
#define MOD_Q1_LG				83
#define MOD_Q1_GL_SPLASH		84
#define MOD_Q1_RL_SPLASH		85
#define MOD_Q1_LG_SPLASH		86
#define MOD_Q1_LASER			87
#define MOD_Q1_FLAMEBOLT		88
#define MOD_Q1_FIREPOD			89

#define MOD_FRIENDLY_FIRE	0x8000000

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
extern	cvar_t	*zdmflags;	// Zaero added
extern	cvar_t	*skill;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
extern	cvar_t	*password;
extern	cvar_t	*spectator_password;
extern	cvar_t	*g_select_empty;
extern	cvar_t	*dedicated;

extern	cvar_t	*filterban;

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
extern	cvar_t	*maxspectators;

extern	cvar_t	*flood_msgs;
extern	cvar_t	*flood_persecond;
extern	cvar_t	*flood_waitdelay;

extern	cvar_t	*sv_maplist;

extern	cvar_t	*sv_stopspeed;		// PGM - this was a define in g_phys.c
extern	cvar_t	*sv_step_fraction;	// Knightmare- this was a define in p_view.c

//ROGUE
extern	cvar_t	*g_showlogic;
extern	cvar_t	*gamerules;
extern	cvar_t	*huntercam;
extern	cvar_t	*strong_mines;
extern	cvar_t	*randomrespawn;

extern	cvar_t	*actorchicken;
extern	cvar_t	*actorjump;
extern	cvar_t	*actorscram;
extern	cvar_t	*alert_sounds;
extern	cvar_t	*allow_download;
extern  cvar_t	*allow_fog;       // Set to 0 for no fog

// set to 0 to bypass target_changelevel clear inventory flag
// because some user maps have this erroneously set
extern  cvar_t	*allow_clear_inventory; 

extern	cvar_t	*bounce_bounce;
extern	cvar_t	*bounce_minv;
extern	cvar_t	*cd_loopcount;
extern	cvar_t	*cl_gun;
extern	cvar_t	*cl_thirdperson; // Knightmare added
extern	cvar_t	*corpse_fade;
extern	cvar_t	*corpse_fadetime;
extern	cvar_t	*crosshair;
//extern	cvar_t	*crossh;
extern	cvar_t	*developer;
extern	cvar_t	*fmod_nomusic;
extern	cvar_t	*footstep_sounds;
extern	cvar_t	*fov;
extern	cvar_t	*gl_clear;
extern  cvar_t  *gl_driver;
extern	cvar_t	*gl_driver_fog;   // Name of dll to load for Default OpenGL mode
extern	cvar_t	*hand;
extern	cvar_t	*jetpack_weenie;
extern	cvar_t	*joy_pitchsensitivity;
extern	cvar_t	*joy_yawsensitivity;
extern	cvar_t	*jump_kick;
extern	cvar_t	*lazarus_cd_loop;
extern	cvar_t	*lazarus_cl_gun;
extern	cvar_t	*lazarus_crosshair;
extern	cvar_t	*lazarus_gl_clear;
extern	cvar_t	*lazarus_joyp;
extern	cvar_t	*lazarus_joyy;
extern	cvar_t	*lazarus_pitch;
extern	cvar_t	*lazarus_yaw;
extern	cvar_t	*lights;
extern	cvar_t	*lightsmin;
extern	cvar_t	*m_pitch;
extern	cvar_t	*m_yaw;
extern	cvar_t	*monsterjump;
extern	cvar_t	*packet_fmod_playback;
extern	cvar_t	*readout;
extern	cvar_t	*rocket_strafe;
extern	cvar_t	*rotate_distance;
extern	cvar_t	*s_primary;
extern	cvar_t	*shift_distance;
extern	cvar_t	*sv_maxgibs;
extern  cvar_t  *tpp;			  // third person perspective
extern	cvar_t	*tpp_auto;
extern	cvar_t	*turn_rider;
extern	cvar_t	*vid_ref;
extern	cvar_t	*zoomrate;
extern	cvar_t	*zoomsnap;

extern	int		max_modelindex;
extern	int		max_soundindex;

// this is for the count of monsters
#define ENT_SLOTS_LEFT		(ent->monsterinfo.monster_slots - ent->monsterinfo.monster_used)
#define SELF_SLOTS_LEFT		(self->monsterinfo.monster_slots - self->monsterinfo.monster_used)
//ROGUE

#define world	(&g_edicts[0])

// item spawnflags
#define ITEM_TRIGGER_SPAWN		0x00000001
#define ITEM_NO_TOUCH			0x00000002
#define ITEM_NO_ROTATE			0x00000004
#define ITEM_NO_DROPTOFLOOR		0x00000008
#define ITEM_SHOOTABLE			0x00000010
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
	char		*name;
	size_t		ofs;	// Knightmare- was int
	fieldtype_t	type;
	int			flags;
} field_t;

typedef struct
{
	char	*name;
	void	(*spawn)(edict_t *ent);
} spawn_t;

// Lazarus: worldspawn effects
#define FX_WORLDSPAWN_NOHELP       1
#define FX_WORLDSPAWN_STEPSOUNDS   2
#define FX_WORLDSPAWN_WHATSIT      4
#define FX_WORLDSPAWN_ALERTSOUNDS  8
#define FX_WORLDSPAWN_CORPSEFADE  16
#define FX_WORLDSPAWN_JUMPKICK    32


extern	field_t fields[];
extern	gitem_t	itemlist[];
extern	spawn_t	spawns[];


//
// g_cmds.c
//
void Cmd_Help_f (edict_t *ent);
void Cmd_Score_f (edict_t *ent);
void SetLazarusCrosshair (edict_t *ent);
void SetSensitivities(edict_t *ent,qboolean reset);

//
// g_crane.c
//
edict_t *CrateOnTop (edict_t *from, edict_t *ent);
void G_FindCraneParts();
void crane_control_action(edict_t *crane, edict_t *activator, vec3_t point);
//void Moving_Speaker_Think(edict_t *ent);

//
// g_items.c
//
void PrecacheItem (gitem_t *it);
void InitItems (void);
void SetItemNames (void);
void SetAmmoPickupValues(void);
gitem_t	*FindItem (char *pickup_name);
gitem_t	*FindItemByClassname (char *classname);
#define	ITEM_INDEX(x) ((x)-itemlist)
edict_t *Drop_Item (edict_t *ent, gitem_t *item);
edict_t *Drop_Q1Backpack (edict_t *ent, gitem_t *item, int count);	// Knightmare added
void SetRespawn (edict_t *ent, float delay);
void ChangeWeapon (edict_t *ent);
void SpawnItem (edict_t *ent, gitem_t *item);
void Think_Weapon (edict_t *ent);
int ArmorIndex (edict_t *ent);
int PowerArmorType (edict_t *ent);
gitem_t	*GetItemByIndex (int index);
int GetMaxAmmoByIndex (gclient_t *client, int item_index); // Knightmare added
int GetMaxArmorByIndex (int item_index); // Knightmare added
qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count);
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

//
// g_utils.c
//
qboolean	KillBox (edict_t *ent);
qboolean MonsterPlayerKillBox (edict_t *ent);	// Zaero added
void	G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
edict_t *G_Find (edict_t *from, size_t fieldofs, char *match);	// Knightmare- changed fieldofs from int
edict_t *findradius (edict_t *from, vec3_t org, float rad);
edict_t *G_PickTarget (char *targetname);
void	G_UseTargets (edict_t *ent, edict_t *activator);
void	G_SetMovedir (vec3_t angles, vec3_t movedir);
void	G_SetMovedir2 (vec3_t angles, vec3_t movedir);

void	G_InitEdict (edict_t *e);
edict_t	*G_Spawn (void);
void	G_FreeEdict (edict_t *e);

void	G_TouchTriggers (edict_t *ent);
void	G_TouchSolids (edict_t *ent);

char	*G_CopyString (char *in);

float	*tv (float x, float y, float z);
char	*vtos (vec3_t v);

float vectoyaw (vec3_t vec);
float vectopitch (vec3_t vec); // Knightmare added
void vectoangles (vec3_t vec, vec3_t angles);
void stuffcmd(edict_t *ent, char *s);

qboolean point_infront (edict_t *self, vec3_t point);
void AnglesNormalize(vec3_t vec);
float SnapToEights(float x);
// Lazarus
float AtLeast (float x, float dx);
edict_t	*LookingAt (edict_t *ent, int filter, vec3_t endpos, float *range);
void GameDirRelativePath (const char *filename, char *output, size_t outputSize);
void SavegameDirRelativePath (const char *filename, char *output, size_t outputSize);
void CreatePath (const char *path);
void G_UseTarget (edict_t *ent, edict_t *activator, edict_t *target);
qboolean IsIdMap (void); // Knightmare added
qboolean IsXatrixMap (void); // Knightmare added
qboolean IsRogueMap (void); // Knightmare added
qboolean IsZaeroMap (void); // Knightmare added
qboolean IsZaeroRailgunHackMap (void); // Knightmare added
qboolean CheckCoop_MapHacks (edict_t *ent); // FS: Coop: Check if we have to modify some stuff for coop so we don't have to rely on distributing ent files
qboolean UseSpecialGoodGuyFlag (edict_t *monster); // Knightmare added
qboolean UseRegularGoodGuyFlag (edict_t *monster); // Knightmare added

//ROGUE
void	G_ProjectSource2 (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t up, vec3_t result);
float	vectoyaw2 (vec3_t vec);
void	vectoangles2 (vec3_t vec, vec3_t angles);
edict_t *findradius2 (edict_t *from, vec3_t org, float rad);
//ROGUE

//
// g_utils_q1.c
//
float PointDist (vec3_t x, vec3_t y);
void Q1TeleportSounds (edict_t *ent);

//
// g_combat.c
//
qboolean OnSameTeam (edict_t *ent1, edict_t *ent2);
qboolean CanDamage (edict_t *targ, edict_t *inflictor);
void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod);
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);

//ROGUE
void T_RadiusNukeDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);
//float max (float a, float b);
void T_RadiusClassDamage (edict_t *inflictor, edict_t *attacker, float damage, char *ignoreClass, float radius, int mod);
void cleanupHealTarget (edict_t *ent);
//ROGUE

//
// g_fog.c
//
#define MAX_FOGS 64	// was 16
extern fog_t gfogs[MAX_FOGS];
void Cmd_Fog_f (edict_t *ent);
void Fog_Init (void);
void Fog (edict_t *ent);
void Fog_Off (edict_t *ent);
void Fog_Off_Global (void);
void Fog_SetFogParms (void);

//
// km_cvar.c
//
void InitDMStartVars (void);	// deathmatch start cvars
void InitLithiumVars (void);	// init lithium cvars

//
// p_gencam.c
//
extern void CheckChasecam_Viewent(edict_t *ent);
void Cmd_Chasecam_Toggle (edict_t *ent);
void ChasecamStart (edict_t *ent);

//
// g_func.c
//
#define TRAIN_START_ON		   1
#define TRAIN_TOGGLE		   2
#define TRAIN_BLOCK_STOPS	   4
#define TRAIN_ROTATE           8
#define TRAIN_ROTATE_CONSTANT 16
#define TRAIN_ROTATE_MASK     (TRAIN_ROTATE | TRAIN_ROTATE_CONSTANT)
#define TRAIN_ANIMATE         32
#define TRAIN_ANIMATE_FAST    64
#define TRAIN_SMOOTH         128
#define TRAIN_SPLINE        8192
#define TRAIN_ORIGIN        16384

void door_use_areaportals (edict_t *self, qboolean open);
qboolean box_walkmove (edict_t *ent, float yaw, float dist);
void button_use (edict_t *self, edict_t *other, edict_t *activator);
void trainbutton_use (edict_t *self, edict_t *other, edict_t *activator);
void Move_Calc (edict_t *ent, vec3_t dest, void(*func)(edict_t*));
void movewith_init (edict_t *ent);
void train_move_children (edict_t *self);
void train_kill_children (edict_t *self);
void train_remove_children (edict_t *self);
void fade_children (edict_t *self);
void fade_children2 (edict_t *self);
void train_next (edict_t *self);
void func_train_find (edict_t *self);
void train_use (edict_t *self, edict_t *other, edict_t *activator);
void train_blocked (edict_t *self, edict_t *other);

// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage
#define DAMAGE_ENERGY			0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET			0x00000010  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION	0x00000020  // armor, shields, invulnerability, and godmode have no effect
//ROGUE
#define DAMAGE_DESTROY_ARMOR	0x00000040	// damage is done to armor and health.
#define DAMAGE_NO_REG_ARMOR		0x00000080	// damage skips regular armor
#define DAMAGE_NO_POWER_ARMOR	0x00000100	// damage skips power armor
//ROGUE


#define DEFAULT_BULLET_HSPREAD	300
#define DEFAULT_BULLET_VSPREAD	500
#define DEFAULT_SHOTGUN_HSPREAD	1000
#define DEFAULT_SHOTGUN_VSPREAD	500
#define DEFAULT_DEATHMATCH_SHOTGUN_COUNT	12
#define DEFAULT_SHOTGUN_COUNT	12
#define DEFAULT_SSHOTGUN_COUNT	20

//
// g_monster.c
//
#define SF_MONSTER_AMBUSH          1
#define SF_MONSTER_TRIGGER_SPAWN   2
#define SF_MONSTER_SIGHT           4
#define SF_MONSTER_GOODGUY         8
#define SF_MONSTER_NOGIB          16
#define SF_MONSTER_SPECIAL        32
#define SF_ACTOR_BAD_GUY          64
#define SF_MONSTER_FLIES         128	// only used for monster_commander_body
#define SF_MONSTER_IGNORESHOTS   128
#define SF_MONSTER_KNOWS_MIRRORS 0x00010000

void monster_fire_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int kick, int hspread, int vspread, int flashtype);
void monster_fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int flashtype);
void monster_fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect, int color);
void monster_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int flashtype, qboolean contact);
void monster_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, edict_t *homing_target);
void monster_fire_missile (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, edict_t *homing_target);	// Knightmare added
void monster_fire_railgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int flashtype);
void monster_fire_bfg (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int kick, float damage_radius, int flashtype);

// RAFAEL
void monster_fire_blueblaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect);
void monster_fire_ionripper (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect);
void monster_fire_rocket_heat (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype);
void monster_fire_dabeam (edict_t *self);
// Knightmare added
void monster_fire_phalanx (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, int flashtype);

// ROGUE
void monster_fire_blaster2 (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect);
void monster_fire_tracker (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, edict_t *enemy, int flashtype);
void monster_fire_heat (edict_t *self, vec3_t start, vec3_t dir, vec3_t offset, int damage, int kick, int flashtype);
// ROGUE

// Knightmare added
void monster_fire_flechette (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, int flashtype);
void monster_fire_prox (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int damage_multiplier, int speed, int health, float timer, float damage_radius, int flashtype);
// SKWiD MOD
void monster_fire_plasma_rifle (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, qboolean spread);
void HintTestNext (edict_t *self, edict_t *hint);
int  HintTestStart (edict_t *self);

void M_droptofloor (edict_t *ent);
void monster_think (edict_t *self);
void deadmonster_think (edict_t *self);
void walkmonster_start (edict_t *self);
void swimmonster_start (edict_t *self);
void flymonster_start (edict_t *self);
void AttackFinished (edict_t *self, float time);
void monster_death_use (edict_t *self);
void M_CatagorizePosition (edict_t *ent);
qboolean M_CheckAttack (edict_t *self);
void M_FlyCheck (edict_t *self);
void M_CheckGround (edict_t *ent);

// ROGUE
void stationarymonster_start (edict_t *self);	
void monster_done_dodge (edict_t *self);
// ROGUE

void InitiallyDead (edict_t *self);
qboolean M_SetDeath(edict_t *self, mmove_t **deathmoves);
int PatchMonsterModel (char *modelname);


//
// g_misc.c
//
void gib_fade (edict_t *self);
void ThrowHead (edict_t *self, char *gibname, int frame, int skinnum, int damage, int type);
void ThrowClientHead (edict_t *self, int damage);
void ThrowGib (edict_t *self, char *gibname, int frame, int skinnum, int damage, int type);
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin, int frame, int skin, int effects);
void BecomeExplosion1(edict_t *self);
void BecomeExplosion2(edict_t *self);
void BecomeExplosion3(edict_t *self);

//void ThrowHeadACID (edict_t *self, char *gibname, int damage, int type);
//void ThrowGibACID (edict_t *self, char *gibname, int damage, int type);
void func_explosive_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void PrecacheDebris (int style);

//
// g_mappack.c
//
void model_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

//
// g_monster.c
//
#define SF_MONSTER_AMBUSH          1
#define SF_MONSTER_TRIGGER_SPAWN   2
#define SF_MONSTER_SIGHT           4
#define SF_MONSTER_GOODGUY         8
#define SF_MONSTER_NOGIB          16
#define SF_MONSTER_SPECIAL        32
#define SF_ACTOR_BAD_GUY          64
#define SF_MONSTER_FLIES         128	// only used for monster_commander_body
#define SF_MONSTER_IGNORESHOTS   128

void FadeSink (edict_t *ent);
void FadeDieSink (edict_t *ent);
void BossExplode (edict_t *self);

//
// g_ai.c
//
void AI_SetSightClient (void);
//float min (float a, float b);
void ai_stand (edict_t *self, float dist);
void ai_move (edict_t *self, float dist);
void ai_walk (edict_t *self, float dist);
void ai_turn (edict_t *self, float dist);
void ai_run (edict_t *self, float dist);
void ai_charge (edict_t *self, float dist);
int range (edict_t *self, edict_t *other);
qboolean canReach (edict_t *self, edict_t *other);

void FoundTarget (edict_t *self);
void HuntTarget (edict_t *self);
qboolean infront (edict_t *self, edict_t *other);
qboolean visible (edict_t *self, edict_t *other);
qboolean FacingIdeal(edict_t *self);
qboolean ai_chicken (edict_t *ent, edict_t *badguy);

//
// g_weapon.c
//
#define BLASTER_ORANGE	1
#define BLASTER_GREEN	2
#define BLASTER_BLUE	3
#define BLASTER_RED		4

void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed);
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick);
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void fire_blaster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect, qboolean hyper, int color);
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean contact);
void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, qboolean useColor, int red, int green, int blue);
void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);
void Grenade_Explode (edict_t *ent);
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, edict_t *home_target);
// Knightmare added
void fire_missile (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, edict_t *home_target);
// Lazarus
qboolean AimGrenade (edict_t *launcher, vec3_t start, vec3_t target, vec_t speed, vec3_t aim, qboolean isProx);
void Grenade_Evade (edict_t *monster);
void Grenade_Add_To_Chain (edict_t *grenade);
void Grenade_Remove_From_Chain (edict_t *grenade);

//
// g_weapon_xatrix.c
//
void fire_ionripper (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect);
void fire_rocket_heat (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_blueblaster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect);
void fire_phalanx_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);	// Knightmare- renamed this, was fire_plasma
void fire_trap (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void Trap_Die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void Cmd_KillTrap_f (edict_t *ent);
void Trap_Explode (edict_t *ent);

//
// g_weapon_lm.c
//
void fire_plasma_rifle (edict_t *ent, vec3_t start, vec3_t dir, int damage, int speed, qboolean spread);	// SKWiD MOD

//
// g_weapon_km.c
//
void fire_shock_sphere (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_nbomb (edict_t *self, vec3_t start, vec3_t aimdir, int speed);		

//
// g_weapon_q1.c
//
void q1_fire_nail (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, qboolean sng);
void q1_nail_precache (void);
void q1_fire_laser (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
void q1_laser_precache (void);
void q1_fire_flame (edict_t *self, vec3_t start, vec3_t dir, float leftrightoff);
void q1_flame_precache (void);
void q1_explode (edict_t *self);
void q1_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void q1_grenade_precache (void);
void q1_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void q1_rocket_precahce (void);
void q1_fire_lightning (edict_t *self, vec3_t start, vec3_t dir, int damage);
void q1_fire_firepod (edict_t *self, vec3_t dir);
void q1_firepod_precache (void);
void q1_fire_lavaball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void q1_fire_lavaball_precache (void);
void q1_fire_acidspit (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
void q1_acidspit_precache (void);
void q1_fire_gib (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed);
void q1_gib_precache (void);

//
// m_actor.c
//
void actor_attack (edict_t *actor);
void actor_files ();
void actor_fire (edict_t *actor);
void actor_jump (edict_t *actor);
void actor_moveit (edict_t *player, edict_t *actor);
void actor_run (edict_t *actor);
void actor_run_back (edict_t *actor);
void actor_salute (edict_t *actor);
void actor_stand (edict_t *actor);
void actor_walk (edict_t *actor);
void actor_walk_back (edict_t *actor);
mmove_t actor_move_crouch;
mmove_t actor_move_crouchwalk;
mmove_t actor_move_crouchwalk_back;
mmove_t	actor_move_run;
mmove_t	actor_move_run_back;
mmove_t	actor_move_run_bad;
mmove_t actor_move_stand;
mmove_t actor_move_walk;
mmove_t	actor_move_walk_back;

//
// m_medic.c
//
#define	MEDIC_MIN_DISTANCE	32
#define MEDIC_MAX_HEAL_DISTANCE	400
#define	MEDIC_TRY_TIME		10.0

void abortHeal (edict_t *self, qboolean change_frame, qboolean gib, qboolean mark);

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
void InitClientPersistant (gclient_t *client, int style);
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
qboolean SV_FilterPacket (char *from);

//
// p_view.c
//
void ClientEndServerFrame (edict_t *ent);
void WhatsIt(edict_t *ent);

//
// p_hud.c
//
void MoveClientToIntermission (edict_t *client);
void G_SetStats (edict_t *ent);
void G_SetSpectatorStats (edict_t *ent);
void G_CheckChaseStats (edict_t *ent);
void ValidateSelectedItem (edict_t *ent);
void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer);

//
// p_text.c
//
void Do_Text_Display(edict_t *activator, int flags, char *message);

//
// p_weapon.c
//
void PlayerNoise(edict_t *who, vec3_t where, int type);

//
// m_actor.c
//
void actor_attack (edict_t *actor);
void actor_files ();
void actor_fire (edict_t *actor);
void actor_jump (edict_t *actor);
void actor_moveit (edict_t *player, edict_t *actor);
void actor_run (edict_t *actor);
void actor_run_back (edict_t *actor);
void actor_salute (edict_t *actor);
void actor_stand (edict_t *actor);
void actor_walk (edict_t *actor);
void actor_walk_back (edict_t *actor);
mmove_t actor_move_crouch;
mmove_t actor_move_crouchwalk;
mmove_t actor_move_crouchwalk_back;
mmove_t	actor_move_run;
mmove_t	actor_move_run_back;
mmove_t	actor_move_run_bad;
mmove_t actor_move_stand;
mmove_t actor_move_walk;
mmove_t	actor_move_walk_back;

//
// m_medic.c
//
#define	MEDIC_MIN_DISTANCE	32
#define MEDIC_MAX_HEAL_DISTANCE	400
#define	MEDIC_TRY_TIME		10.0

void abortHeal (edict_t *self, qboolean change_frame, qboolean gib, qboolean mark);
void medic_NextPatrolPoint(edict_t *ent,edict_t *hintpath);
edict_t *medic_FindDeadMonster (edict_t *ent);
void medic_StopPatrolling(edict_t *ent);
//
// m_move.c
//
qboolean M_CheckBottom (edict_t *ent);
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink);
qboolean M_walkmove (edict_t *ent, float yaw, float dist);
void M_MoveToGoal (edict_t *ent, float dist);
void M_ChangeYaw (edict_t *ent);
qboolean M_MoveAwayFromFlare(edict_t *self, float dist);	// Zaero

//
// g_phys.c
//
void G_RunEntity (edict_t *ent);

#ifdef JETPACK_MOD
void Use_Jet (edict_t *ent, gitem_t *item);

//
// g_jetpack.c
//
void Jet_ApplyJet( edict_t *ent, usercmd_t *ucmd );
qboolean Jet_AvoidGround( edict_t *ent );
void Jet_BecomeExplosion( edict_t *ent, int damage );
#endif
//
// g_lights.c
//
void Lights();
void ToggleLights();
//
// g_lock.c
//
void lock_digit_increment (edict_t *digit, edict_t *activator);
//
// g_main.c
//
void SaveClientData (void);
void FetchClientEntData (edict_t *ent);
void EndDMLevel (void);

//
// g_camera.c
//
void use_camera(edict_t *ent, edict_t *other, edict_t *activator);
void camera_on(edict_t *ent);
void camera_off(edict_t *ent);
void faker_animate(edict_t *self);
edict_t *G_FindNextCamera (edict_t *camera, edict_t *monitor);
edict_t *G_FindPrevCamera (edict_t *camera, edict_t *monitor);

//
// g_chase.c
//
void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);
void GetChaseTarget(edict_t *ent);

//
// p_gencam.c
//
void ChasecamStart (edict_t *ent);
void ChasecamRemove (edict_t *ent);
void Cmd_Chasecam_Toggle (edict_t *ent);

//
// p_weapon.c
//
void PlayerNoise(edict_t *who, vec3_t where, int type);
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void kick_attack (edict_t *ent);

//====================
// ROGUE PROTOTYPES
//
// g_newweap.c
//
#define PROX_TEST_SIZE 7.0f
//extern float nuke_framenum;
void fire_flechette (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
edict_t *CheckForProxField (edict_t *ent);
//void fire_prox (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed);	
void fire_prox (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int damage_multiplier, int speed, int health, float timer, float damage_radius);
void Cmd_DetProx_f (edict_t *ent);
void fire_nuke (edict_t *self, vec3_t start, vec3_t aimdir, int speed);		
void fire_flame (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed);
void fire_burst (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed);
void fire_maintain (edict_t *, edict_t *, vec3_t start, vec3_t aimdir, int damage, int speed);
void fire_incendiary_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_player_melee (edict_t *self, vec3_t start, vec3_t aim, int reach, int damage, int kick, int quiet, int mod);
void fire_tesla (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed);
void fire_blaster2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect, qboolean hyper);
void fire_heat (edict_t *self, vec3_t start, vec3_t aimdir, vec3_t offset, int damage, int kick, qboolean monster);
void fire_tracker (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, edict_t *enemy);

// ROGUE
//
// g_newai.c
//
#define	MAX_HINT_CHAINS		100
extern int	hint_paths_present;
extern edict_t *hint_path_start[MAX_HINT_CHAINS];
extern int	num_hint_paths;

qboolean blocked_checkshot (edict_t *self, float shotChance);
qboolean blocked_checkplat (edict_t *self, float dist);
qboolean blocked_checkjump (edict_t *self, float dist, float maxDown, float maxUp);
qboolean blocked_checknewenemy (edict_t *self);
qboolean monsterlost_checkhint (edict_t *self);
qboolean inback (edict_t *self, edict_t *other);
float realrange (edict_t *self, edict_t *other);
edict_t *SpawnBadArea (vec3_t mins, vec3_t maxs, float lifespan, edict_t *owner);
edict_t *CheckForBadArea (edict_t *ent);
qboolean MarkTeslaArea (edict_t *self, edict_t *tesla);
//qboolean MarkProxArea (edict_t *prox);	// Knightmare added
void InitHintPaths (void);
void PredictAim (edict_t *target, vec3_t start, float bolt_speed, qboolean eye_height, float offset, vec3_t aimdir, vec3_t aimpoint);
qboolean below (edict_t *self, edict_t *other);
void drawbbox (edict_t *self);
void M_MonsterDodge (edict_t *self, edict_t *attacker, float eta, trace_t *tr);
void monster_duck_down (edict_t *self);
void monster_duck_hold (edict_t *self);
void monster_duck_up (edict_t *self);
qboolean has_valid_enemy (edict_t *self);
void TargetTesla (edict_t *self, edict_t *tesla);
void hintpath_stop (edict_t *self);
edict_t * PickCoopTarget (edict_t *self);
int CountPlayers (void);
void monster_jump_start (edict_t *self);
qboolean monster_jump_finished (edict_t *self);


//
// g_sphere.c
//
void Defender_Launch (edict_t *self);
void Vengeance_Launch (edict_t *self);
void Hunter_Launch (edict_t *self);

//
// g_newdm.c
//
void InitGameRules(void);
edict_t *DoRandomRespawn (edict_t *ent);
void PrecacheForRandomRespawn (void);
qboolean Tag_PickupToken (edict_t *ent, edict_t *other);
void Tag_DropToken (edict_t *ent, gitem_t *item);
void Tag_PlayerDeath(edict_t *targ, edict_t *inflictor, edict_t *attacker);
void fire_doppleganger (edict_t *ent, vec3_t start, vec3_t aimdir);

//
// g_spawn.c
//
void ReInitialize_Entity (edict_t *ent);
edict_t *CreateMonster(vec3_t origin, vec3_t angles, char *classname);
edict_t *CreateFlyMonster (vec3_t origin, vec3_t angles, vec3_t mins, vec3_t maxs, char *classname);
edict_t *CreateGroundMonster (vec3_t origin, vec3_t angles, vec3_t mins, vec3_t maxs, char *classname, int height);
qboolean FindSpawnPoint (vec3_t startpoint, vec3_t mins, vec3_t maxs, vec3_t spawnpoint, float maxMoveUp);
qboolean CheckSpawnPoint (vec3_t origin, vec3_t mins, vec3_t maxs);
qboolean CheckGroundSpawnPoint (vec3_t origin, vec3_t entMins, vec3_t entMaxs, float height, float gravity);
void DetermineBBox (char *classname, vec3_t mins, vec3_t maxs);
void SpawnGrow_Spawn (vec3_t startpos, int size);
void Widowlegs_Spawn (vec3_t startpos, vec3_t angles);

//
// p_client.c
//
void RemoveAttackingPainDaemons (edict_t *self);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles, int *style, int *health);
float PlayersRangeFromSpot (edict_t *spot);

// ROGUE PROTOTYPES
//====================

//
// g_patchplayermodels.c
//
int PatchPlayerModels (char *modelname);

//
// g_phys.c
//
void SV_AddGravity (edict_t *ent);
void G_RunEntity (edict_t *ent);

//
// g_reflect.c
//
void AddReflection (edict_t *ent);
void DeleteReflection (edict_t *ent, int index);
void ReflectExplosion (int type, vec3_t origin);
void ReflectSparks (int type, vec3_t origin, vec3_t movedir);
void ReflectWeldingSparks (byte count, byte color, vec3_t origin, vec3_t movedir);
void ReflectSteam (vec3_t origin,vec3_t movedir,int count,int sounds,int speed, int wait, int nextid);
void ReflectTrail (int type, vec3_t start, vec3_t end, int red, int green, int blue);
void ReflectLightning (edict_t *srcEnt, edict_t *dstEnt, vec3_t start, vec3_t end);
void ReflectHeatBeam (int te_beam, edict_t *srcEnt, vec3_t start, vec3_t end);

//
// g_sound.c (interface to FMOD)
//
qboolean FMOD_IsPlaying(edict_t *ent);
void FMOD_Shutdown();
void FMOD_Stop();
void FMOD_StopSound(edict_t *ent, qboolean free);
int FMOD_PlaySound(edict_t *ent);
void FMOD_UpdateListenerPos();
void FMOD_UpdateSpeakerPos(edict_t *speaker);
qboolean FMOD_Init();
//Knightmare- this is now handled client-side
#ifdef FMOD_FOOTSTEPS
void FootStep(edict_t *ent);
void PlayFootstep(edict_t *ent, footstep_t index);
extern qboolean qFMOD_Footsteps;
#endif
void target_playback_delayed_restart (edict_t *ent);
void target_playback_delayed_start (edict_t *ent);

//
// g_spawn.c
//
void ED_CallSpawn (edict_t *ent);
void G_FindTeams();
void Cmd_ToggleHud ();
void Hud_On();
void Hud_Off();
//
// g_svcmds.c
//
void	ServerCommand (void);
qboolean SV_FilterPacket (char *from);
//
// g_thing.c
//
edict_t *SpawnThing();
//
// g_tracktrain.c
//
void tracktrain_disengage (edict_t *train);
//
// g_turret.c
//
void turret_breach_fire(edict_t *ent);
void turret_disengage (edict_t *ent);
//
// g_trigger.c
//
typedef struct
{
	char	*name;
} entlist_t;

qboolean HasSpawnFunction(edict_t *ent);
int trigger_transition_ents (edict_t *changelevel, edict_t *self);

// Zaero
//
// z_item.c
//
qboolean EMPNukeCheck(edict_t	*ent, vec3_t pos);
void updateVisorHud (edict_t *ent);
void startVisorStatic (edict_t *ent);
void stopCamera (edict_t *self);

//
// z_weapon.c
//
void fire_bb (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_flare (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);

//
// z_ai.c
//
void ai_schoolStand (edict_t *self, float dist);
void ai_schoolRun (edict_t *self, float dist);
void ai_schoolWalk (edict_t *self, float dist);
void ai_schoolCharge (edict_t *self, float dist);
void ai_schoolBackWalk (edict_t *self, float dist);
void ai_schoolSideStepRight (edict_t *self, float dist);
void ai_schoolSideStepLeft (edict_t *self, float dist);
// end Zaero

//============================================================================

// client_t->anim_priority
#define	ANIM_BASIC		0		// stand / run
#define	ANIM_WAVE		1
#define	ANIM_JUMP		2
#define	ANIM_PAIN		3
#define	ANIM_ATTACK		4
#define	ANIM_DEATH		5
#define	ANIM_REVERSE	6


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
	int			max_fc_health;
	int			savedFlags;

	int			selected_item;
	int			inventory[MAX_ITEMS];

	// ammo capacities
	int			max_bullets;
	int			max_shells;
	int			max_rockets;
	int			max_grenades;
	int			max_cells;
	int			max_slugs;
	// RAFAEL
	int			max_magslug;
	int			max_trap;

	gitem_t		*weapon;
	gitem_t		*lastweapon;

	qboolean    fire_mode;		// Lazarus - alternate firing mode
	qboolean	plasma_mode;	// SKWiD MOD- plasma rifle mode

	int			power_cubes;	// used for tracking the cubes in coop games
	int			score;			// for calculating total unit score in coop games

	int			game_helpchanged;
	int			helpchanged;

	qboolean	spectator;			// client is a spectator

// ROGUE
	int			max_tesla;
	int			max_prox;
	int			max_mines;
	int			max_flechettes;
	int			max_disruptors;
// end ROGUE

	// Knightmare added
	int			max_shockspheres;
	int			max_fuel;
	int			max_homing_rockets;

//	int			max_armor; // KM

	qboolean	spawn_landmark;
	qboolean	spawn_levelchange;
	vec3_t		spawn_offset;
	vec3_t		spawn_velocity;
	vec3_t		spawn_angles;
	vec3_t		spawn_viewangles;
	int			spawn_pm_flags;
	int			spawn_gunframe;
	int			spawn_modelframe;
	int			spawn_anim_end;
	gitem_t		*newweapon;

	// Zaero
	int			max_flares;
	int			max_tbombs;
	int			max_a2k;
	int			max_empnuke;
	int			max_plasmashield;

	float		visorFrames;
	// end Zaero
} client_persistant_t;

// client data that stays across deathmatch respawns
typedef struct
{
	client_persistant_t	coop_respawn;	// what to set client->pers to on a respawn
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
	vec3_t		cmd_angles;			// angles sent over in the last command

	qboolean	spectator;			// client is a spectator
	int         player_class;
} client_respawn_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
struct gclient_s
{
	// known to server
	player_state_t	ps;				// communicated by server to clients
	int				ping;

	// private to game
	client_persistant_t	pers;
	client_respawn_t	resp;
	pmove_state_t		old_pmove;	// for detecting out-of-pmove changes

	qboolean	showscores;			// set layout stat
	qboolean	showinventory;		// set layout stat
	qboolean	showhelp;
	qboolean	showhelpicon;

	int			ammo_index;

	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	//int			nNewLatch;

	qboolean	weapon_thunk;

	gitem_t		*newweapon;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_parmor;		// damage absorbed by power armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation

	float		killer_yaw;			// when dead, look at killer

	weaponstate_t	weaponstate;
	vec3_t		kick_angles;	// weapon kicks
	vec3_t		kick_origin;
	float		v_dmg_roll, v_dmg_pitch, v_dmg_time;	// damage kicks
	float		fall_time, fall_value;		// for view drop on fall
	float		damage_alpha;
	float		bonus_alpha;
	vec3_t		damage_blend;
	vec3_t		v_angle;			// aiming direction
	float		bobtime;			// so off-ground doesn't change it
	vec3_t		oldviewangles;
	vec3_t		oldvelocity;

	float		next_drown_time;
	int			old_waterlevel;
	int			breather_sound;

	int			machinegun_shots;	// for weapon raising

	qboolean    backpedaling;  //<- CDawg added this
	
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

	float		flashlight_framenum;
	qboolean	flashlight_active;
	int			flashlight_cell_usage;

	// menu stuff ala CTF
	qboolean	inmenu;				// in menu
	int         menutimer;
	pmenuhnd_t	*menu;				// current menu
	texthnd_t	*textdisplay;		// currently displayed text
	char		*whatsit;

	qboolean	grenade_blew_up;
	float		grenade_time;
		// RAFAEL
	float		quadfire_framenum;
	qboolean	trap_blew_up;
	float		trap_time;
	int			silencer_shots;
	int			weapon_sound;

	float		pickup_msg_time;

	float		flood_locktill;		// locked from talking
	float		flood_when[10];		// when messages were said
	int			flood_whenhead;		// head pointer for when said

	float		respawn_time;		// can respawn when time > this

	edict_t		*chase_target;		// player we are chasing
	qboolean	update_chase;		// need to update chase info?

	usercmd_t	ucmd;				// Lazarus: Copied for convenience in ClientThink
	int			use;				// indicates whether +use key is pressed

	// Mappack - set when the client is a camera. Change this to a flag
	qboolean		incamera;
	//=====================================================
	// Chasecam	
	//=====================================================
	int             chasetoggle; //whether chasecam is toggled on
	int				chaseactive; //whether chasecam is active
	edict_t         *chasecam;	//from Generations source	
	edict_t         *oldplayer;

	//=====================================================
	// Misc Timing Vars
	//=====================================================	

	float			oldweapon;				//Lightning gun, Chainsaw etc
	float			maxvelocity;

	edict_t         *push;		//object player is pushing
	edict_t			*homing_rocket;		// used to limit firing frequency

#ifdef JETPACK_MOD
	qboolean	jetpack;
	float		jetpack_framenum;
	float		jetpack_nextthink;
	qboolean	jetpack_thrusting;
	qboolean	jetpack_infinite;
	float		jetpack_start_thrust;
	float		jetpack_last_thrust;
	float		jetpack_activation;
	float		jetpack_roll;
#endif

	// security camera
	edict_t     *spycam;
	edict_t		*monitor;
	edict_t		*camplayer;
	vec3_t		org_viewangles;
	short		old_owner_angles[2];

	int			vehicle_framenum;	// last time player engaged or disengaged vehicle
	int			zooming;
	float		joy_pitchsensitivity;
	float		joy_yawsensitivity;
	float		m_pitch;
	float		m_yaw;
	qboolean	sensitivities_init;
	qboolean	zoomed;
	float		original_fov;
	float		fps_time_start;
	int			fps_frames;
	float		secs_per_frame;
	float		frame_zoomrate;

	int			shift_dir;			// direction code for debugging/moving an item
	int			startframe;			// time at which ClientBegin is called

	float		fadestart;
	float		fadein;				// for fading screen to black at mission failure
	float		fadehold;
	float		fadeout;
	vec3_t		fadecolor;
	float		fadealpha;

	int			leftfoot;			// 0 or 1, used for footstep sounds
	int			jumping;			// 0 or 1, used for jumpkick

// ROGUE
	float		double_framenum;
	float		ir_framenum;
//	float		torch_framenum;
	float		nuke_framenum;
	float		tracker_pain_framenum;

	edict_t		*owned_sphere;		// this points to the player's sphere
// end ROGUE

// Zaero
	float		a2kFramenum;

	// used for blinding
	int			flashTime;
	int			flashBase;

	edict_t		*zCameraTrack;    // the entity to see through
	vec3_t		zCameraOffset;   // offset from camera origin
	edict_t		*zCameraLocalEntity;
	float		zCameraStaticFramenum;

	qboolean	showOrigin;

	// for sniper rifle
	int			sniperFramenum;

	// for sonic cannon
	float		startFireTime;
// end Zaero

	qboolean	bfg_missfire;	// Knightmare- added for Zaero EMP Nuke
};

/*
Mappack - 4/5/99
new structure for the backup client data while its in the camera.
*/

typedef struct
{
	vec3_t		oldorigin;		// clients world origin
	vec3_t		moveorigin;		// for client prediction, don't think this is needed as its only an sp modification
	vec3_t		viewangles;		// which way the client is facing

	int			pm_type;		// player movetype I believe. (noclip, fly, etc)
	int			gunindex;		// gun model

	vec3_t		blend;			// screen blend value (for bleeding, etc)
	int			rdflags;		// render flags for the clent

	// how long left before something happens/runs out
	int			quad_framenum;
	int			invincible_framenum;
	int			breather_framenum;
	int			enviro_framenum;
	int			grenade_blew_up;
	int			grenade_time;

	int 		viewheight;		// viewheight from the origin

	// various model files the player can be displayed as.
	int			modelindex;
	int			modelindex2;
	int			modelindex3;
	unsigned int	effects;	// what effect are we (needs to be unsigned because theres a few
	int			sound;
	solid_t		solid;			// current solid state, 90% certain its SOLID_PLAYER ;)

} backup_t;

//Mappack - thats all.

/*
Mappack - 15/5/99
New structure for nodeinfo
*/

#define		NODE_CENTRAL	0x00000001	//No change in z
#define		NODE_UP		0x00000002	//path goes up
#define		NODE_DOWN		0x00000004	//pat goes down
#define		NODE_HEAD		0x00000008	//starting point
#define		NODE_END		0x00000010	//end point
#define		NODE_LOST_SIGHT	0x00000020	//we can't see self->enemy

typedef struct
{
	vec3_t	last_sighting;	//last point that we saw the player
	int		last_sighttime;	//last time that we saw the player

	int		chainnum;		//node position in the chain
	int		nodeflags;		//node flags i.e. NODE_UP, NODE_DOWN, etc
	edict_t	*target_node;
} nodeinfo_t;

//Mappack - thats all

#define NUM_ACTOR_SOUNDS   13

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
	entity_id	class_id;			// Lazarus: Added in lieu of doing string comparisons
									// on classnames.

	int			movetype;
	int			flags;

	char		*model;
	float		freetime;			// sv.time when the object was freed
	
	//
	// only used locally in game, not by server
	//
	char		*message;
	char        *key_message;   // Lazarus: used from tremor_trigger_key
	char		*classname;
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
	edict_t		*movewith_ent;  // Knightmare- movewith entity
	edict_t		*movewith_next;
	edict_t		*from;
	edict_t		*to;
	edict_t		*viewer;
	edict_t		*speaker; //pointer to train's noise speaker
	edict_t		*inflictor;  //for delayed func_explosive
	edict_t		*attacker;  //for delayed func_explosive

	edict_t		*next_grenade;
	edict_t		*prev_grenade;

	// FMOD
	int			*stream;	// Actually a FSOUND_STREAM * or FMUSIC_MODULE *
	int			channel;

	// tracktrain
	char		*target2;
	edict_t		*prevpath;

	float		speed, accel, decel;

	// Knightmare added
	int			oldmovetype;		// backup of movetype
	vec3_t		relative_velocity;	// relative velocity of movewith children
	vec3_t		relative_avelocity;	// relative angular velocity of movewith children
	vec3_t		movewith_offset;
	vec3_t		old_offset;
	int			movewith_set;
	int			smooth_movement;
	int			bounce_me;
	int			in_mud;
	float		width;
	float		length;
	float		side;
	vec3_t		origin_offset;  // These are from Lazarus for the rider code
	vec3_t		org_angles;
	vec3_t		org_mins;
	vec3_t		org_maxs;
	vec3_t		org_size;
	vec3_t		parent_attach_angles;
	vec3_t		child_attach_angles;
	vec3_t		deploy_angles;
	vec3_t		aim_point;
	int			axis;

	// crane
	qboolean    busy;
	qboolean	attracted;
	int         crane_increment;
	int         crane_dir;
	edict_t     *crane_control;
	edict_t     *crane_onboard_control;
	edict_t     *crane_beam;
	edict_t     *crane_hoist;
	edict_t     *crane_hook;
	edict_t     *crane_cargo;
	edict_t		*crane_cable;
	edict_t     *crane_light;
	vec_t       crane_bonk;

	vec3_t		movedir;
	vec3_t		pos1, pos2;
	vec3_t		pos0;	// Knightmare- initial position for secret doors

	vec3_t		velocity;
	vec3_t		oldvelocity; // Knightmare added
	vec3_t		avelocity;
	int			mass;
	float		density;
	float		air_finished;
	float		gravity;		// per entity gravity multiplier (1.0 is normal)
								// use for lowgrav artifact, flares
	int			actor_sound_index[NUM_ACTOR_SOUNDS];
	int			actor_gunframe;
	int			actor_current_weapon;		// Index into weapon[]
	int			actor_weapon[2];
	int			actor_model_index[2];
	float		actor_crouch_time;
	qboolean	actor_id_model;
	vec3_t		muzzle;						// Offset from origin to gun muzzle
	vec3_t		muzzle2;					// Offset to left weapon (must have SF | 128)

	edict_t		*goalentity;
	edict_t		*movetarget;

	// Knightmare- rotating train stuff
	float		pitch_speed;
	float		yaw_speed;
	float		roll_speed;
	float		ideal_yaw;
	float		ideal_pitch;
	float		ideal_roll; // roll value for a path_corner
	float		roll;		// roll value for a path_corner
	int			turn_rider;	// whether to turn rider

	char		*common_name;

	char		*newtargetname; // used with target_change
	char		*followtarget;	// used with func_door_swinging
	char		*destroytarget;
	char		*viewmessage;
	char		*source;		// used with target_bmodel_spawner
	char		*newteam;		// used with target_bmodel_spawner

	char		*musictrack;	// Knightmare- for specifying OGG or CD track

	// selected brush models will move their origin to 
	// the origin of this entity:
	char		*move_to;

	float		distance;	// target_playback stuff
	float		duration;
	float		bob;
	int			lip;
	int			height;
	int			bobframe;
	float		shift;
	float		base_radius;

	vec3_t		color;
	float		alpha;
	float		holdtime;	// target_fade stuff
	float		fadein;
	float		fadeout;

	int			powerarmor;
	int			powerarmortype;
	
	int			owner_id;					// These are used ONLY for ents that
	int			id;							// change maps via trigger_transition
	int			last_attacked_framenum;		// Used to turn off chicken mode

	float		nextthink;
	void		(*prethink) (edict_t *ent);
	void		(*think)(edict_t *self);
	void		(*postthink) (edict_t *ent); // Knightmare added
	void		(*blocked)(edict_t *self, edict_t *other);	// move to moveinfo?
	void		(*touch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
	void		(*use)(edict_t *self, edict_t *other, edict_t *activator);
	void		(*pain)(edict_t *self, edict_t *other, float kick, int damage);
	void		(*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

	void		(*play)(edict_t *self, edict_t *activator);

	float		touch_debounce_time;		// are all these legit?  do we need more/less of them?
	float		pain_debounce_time;
	float		damage_debounce_time;
	float		gravity_debounce_time;		// used by item_ movement commands to prevent
											// monsters from dropping to floor
	float		fly_sound_debounce_time;	// move to clientinfo
	float		last_move_time;
	float		last_fire_time; // Knightmare added

	int			health;
	int			max_health;
	int			base_health;
	int			gib_health;
	int			deadflag;
	qboolean	show_hostile;

	// Lazarus: health2 and mass2 are passed from jorg to makron health and mass
	int			health2;
	int			mass2;

	float		powerarmor_time;

	char		*map;			// target_changelevel

	int			viewheight;		// height above origin where eyesight is determined
	int			takedamage;
	int			dmg;
	int			radius_dmg;
	float		dmg_radius;
	int			sounds;			//make this a spawntemp var?
	int			count;

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
	// Lazarus: laser timing
	float		starttime;
	float		endtime;

	float		teleport_time;

	int			watertype;
	int			waterlevel;
	int			old_watertype;

	vec3_t		move_origin;
	vec3_t		move_angles;

	// move this to clientinfo?
	int			light_level;

	int			style;			// also used as areaportal number
	int			gib_type;
	int			blood_type;

	int			moreflags;

	// actor muzzle flash
	edict_t		*flash;

	// Psychospaz reflections
	edict_t		*reflection[6];

	gitem_t		*item;			// for bonus items

	// common data blocks
	moveinfo_t		moveinfo;
	monsterinfo_t	monsterinfo;

	float		goal_frame;

//=====================================================
//	Chasecam variables - from Generations source
//=====================================================
	int         chasedist1;   
	int         chasedist2;
    edict_t		*crosshair;
	// end tpp

	// item identification
	char		*datafile;

	// Mappack- for the camera to backup the players position, ammo, powerups, etc
	backup_t	backup;	// this holds the client data

	vec3_t		fog_color;
	int			fog_model;
	float		fog_near;
	float		fog_far;
	float		fog_density;
	int			fog_index;
	int			fogclip;		// only used by worldspawn to indicate whether gl_clear
								// should be forced to a good value for fog obscuration
								// of HOM

	// Mappack - for ridahs controllable turret (but beels weapon thing)
	edict_t		*turret;	//ugly ?
	edict_t		*child;			// "real" infantry guy, child of remote turret_driver

	edict_t		*vehicle;		//pointer for vehicle player is driving
	char		*idle_noise;
	float		radius;

	// Mappack - for the sprite/model spawner
	char		*usermodel;
	int			startframe;
	int			framenumbers;
	int			solidstate;
	int			renderfx;
	int			effects;
	int			skinnum;
	vec3_t		bleft;
	vec3_t		tright;

	// Knightmare- to support Lazarus func_train movewith
	char			*movewith;
	char			*dmgteam;		// for target_monsterbattle
	int				do_not_rotate;	// whether to movewith rotate a func_door

// ROGUE
	int			plat2flags;
	vec3_t		offset;
	vec3_t		gravityVector;
	edict_t		*bad_area;
	edict_t		*hint_chain;
	edict_t		*monster_hint_chain;
	edict_t		*target_hint_chain;
	int			hint_chain_id;
	// FIXME - debug help!
	float		lastMoveTime;
// end ROGUE

// Zaero
	char		*model2;
	char		*model3;
	char		*model4;
	
	float		aspeed;

	// can use this for misc. timeouts
	float		timeout;

//	int			blood_type; // specifies blood effect

	// for func_door, also used by monster_autocannon, and misc_securitycamera
	int			active;
	int			seq;
	
	// between level saves/loads
	int			spawnflags2;
	int			oldentnum;
	
	// titan laser
	edict_t		*laser;

	float		weaponsound_time;

	// schooling info
	edict_t		*zRaduisList, *zSchoolChain;
	float		zDistance;

	// this is for MOVETYPE_RIDE
	edict_t		*rideWith[2];
	vec3_t		rideWithOffset[2];

	// camera number
	vec3_t		mangle;
	
	// time left for the visor (stored if a visor is dropped)
	int			visorFrames;

	// monster team
	char		*mteam;

	// for random func_timer targets
//	char		targets[16][MAX_QPATH];
//	int			numTargets;

	// used by floor-mounted autocannon
	int			onFloor;

	float		bossFireTimeout;
	int			bossFireCount;
// end Zaero
};

//=============
//ROGUE
#define ROGUE_GRAVITY	1

#define SPHERE_DEFENDER			0x0001
#define SPHERE_HUNTER			0x0002
#define SPHERE_VENGEANCE		0x0004
#define SPHERE_DOPPLEGANGER		0x0100

#define SPHERE_TYPE			0x00FF
#define SPHERE_FLAGS			0xFF00

//
// deathmatch games
//
#define		RDM_TAG			2
#define		RDM_DEATHBALL	3

typedef struct dm_game_rs
{
	void		(*GameInit)(void);
	void		(*PostInitSetup)(void);
	void		(*ClientBegin) (edict_t *ent);
	void		(*SelectSpawnPoint) (edict_t *ent, vec3_t origin, vec3_t angles, int *style, int *health);
	void		(*PlayerDeath)(edict_t *targ, edict_t *inflictor, edict_t *attacker);
	void		(*Score)(edict_t *attacker, edict_t *victim, int scoreChange);
	void		(*PlayerEffects)(edict_t *ent);
	void		(*DogTag)(edict_t *ent, edict_t *killer, char **pic);
	void		(*PlayerDisconnect)(edict_t *ent);
	int			(*ChangeDamage)(edict_t *targ, edict_t *attacker, int damage, int mod);
	int			(*ChangeKnockback)(edict_t *targ, edict_t *attacker, int knockback, int mod);
	int			(*CheckDMRules)(void);
} dm_game_rt;

extern dm_game_rt	DMGame;

void Tag_GameInit (void);
void Tag_PostInitSetup (void);
void Tag_PlayerDeath (edict_t *targ, edict_t *inflictor, edict_t *attacker);
void Tag_Score (edict_t *attacker, edict_t *victim, int scoreChange);
void Tag_PlayerEffects (edict_t *ent);
void Tag_DogTag (edict_t *ent, edict_t *killer, char **pic);
void Tag_PlayerDisconnect (edict_t *ent);
int  Tag_ChangeDamage (edict_t *targ, edict_t *attacker, int damage, int mod);

void DBall_GameInit (void);
void DBall_ClientBegin (edict_t *ent);
void DBall_SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles, int *style, int *health);
int  DBall_ChangeKnockback (edict_t *targ, edict_t *attacker, int knockback, int mod);
int  DBall_ChangeDamage (edict_t *targ, edict_t *attacker, int damage, int mod);
void DBall_PostInitSetup (void);
int  DBall_CheckDMRules (void);
//void Tag_PlayerDeath (edict_t *targ, edict_t *inflictor, edict_t *attacker);
//void Tag_Score (edict_t *attacker, edict_t *victim, int scoreChange);
//void Tag_PlayerEffects (edict_t *ent);
//void Tag_DogTag (edict_t *ent, edict_t *killer, char **pic);
//void Tag_PlayerDisconnect (edict_t *ent);
//int  Tag_ChangeDamage (edict_t *targ, edict_t *attacker, int damage);

//ROGUE
//============

#define	LOOKAT_NOBRUSHMODELS  1
#define LOOKAT_NOWORLD        2
#define LOOKAT_MD2			  (LOOKAT_NOBRUSHMODELS | LOOKAT_NOWORLD)

#define BeepBeep(ent) (gi.sound (ent, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0))


#define POWERUP_REPLACE_ENT 0
#define POWERUP_NEW_ENT     1
#define POWERUP_USE_ITEM    2

#define FLASHLIGHT_MOD
#define FLASHLIGHT_USE POWERUP_NEW_ENT
#define FLASHLIGHT_DRAIN     60
#define FLASHLIGHT_ITEM      "Cells"

// Zaero dmflags
#define ZDM_NO_GL_POLYBLEND_DAMAGE	1
#define ZDM_ZAERO_ITEMS				2
