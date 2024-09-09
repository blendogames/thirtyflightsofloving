// g_local.h -- local definitions for game module

#include "q_shared.h"
#include <sys/timeb.h>

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "game.h"

//ZOID
#include "p_menu.h"
//ZOID

#include "plasma.h"	// SKWiD MOD

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"ultra"

// protocol bytes that can be directly added to messages
#define	svc_muzzleflash		1
#define	svc_muzzleflash2	2
#define	svc_temp_entity		3
#define	svc_layout			4
#define	svc_inventory		5
#define svc_stufftext		11

//==================================================================

//#define	USE_AM_ROCKETS	// uncomment to enable anti-matter rockets

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

#define FL_SHOWPATH				0x00002000	// used to show current path (debugging)
#define	FL_NO_VWEAPON			0x00004000	//	STUPID hacks so we don't keep checking for
#define	FL_SUPPORTS_VWEAPON		0x00008000	//	VWeapon support, and eating up CPU time
#define FL_SHOW_FLAGPATHS		0x00010000

//ROGUE
#define FL_MECHANICAL			0x00002000	// entity is mechanical, use sparks not blood
#define FL_SAM_RAIMI			0x00004000	// entity is in sam raimi cam mode
#define FL_DISGUISED			0x00008000	// entity is in disguise, monsters will not recognize.
#define	FL_NOGIB				0x00010000	// player has been vaporized by a nuke, drop no gibs
//ROGUE

#define FL_TURRET_OWNER			0x00020000	// player is in a turret and controlling it

#define FL_RESPAWN				0x80000000	// used for item respawning


#define	FRAMETIME		0.1

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level


// plat constants
#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3
#define STATE_LOWEST		4	// Knightmare added


#define MELEE_DISTANCE	80

#define BODY_QUEUE_SIZE		8

static vec3_t VEC_ORIGIN = {0,0,0};

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
	// RAFAEL
	AMMO_MAGSLUG,
	AMMO_TRAP,
	//ROGUE
	AMMO_FLECHETTES,
	AMMO_TESLA,
#ifdef KILL_DISRUPTOR
	AMMO_PROX
#else
	AMMO_PROX,
	AMMO_DISRUPTOR
#endif

} ammo_t;


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

//ROGUE
#define AI_WALK_WALLS			0x00008000
#define AI_MANUAL_STEERING		0x00010000
#define AI_TARGET_ANGER			0x00020000
#define AI_DODGING				0x00040000
#define AI_CHARGING				0x00080000
#define AI_HINT_PATH			0x00100000
#define	AI_IGNORE_SHOTS			0x00200000
// PMM - FIXME - last second added for E3 .. there's probably a better way to do this, but
// this works
#define	AI_DO_NOT_COUNT			0x00400000	// set for healed monsters
#define	AI_SPAWNED_CARRIER		0x00800000	// both do_not_count and spawned are set for spawned monsters
#define	AI_SPAWNED_MEDIC_C		0x01000000	// both do_not_count and spawned are set for spawned monsters
#define	AI_SPAWNED_WIDOW		0x02000000	// both do_not_count and spawned are set for spawned monsters
#define AI_SPAWNED_MASK			0x03800000	// mask to catch all three flavors of spawned
#define	AI_BLOCKED				0x04000000	// used by blocked_checkattack: set to say I'm attacking while blocked
											// (prevents run-attacks)
//ROGUE

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
MOVETYPE_BOUNCE,		// added this (the comma at the end of line)
// RAFAEL
MOVETYPE_WALLBOUNCE,
MOVETYPE_NEWTOSS,		// PGM - for deathball
MOVETYPE_SHOCKBOUNCE	// Knightmare- added for shockwave
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
//ZOID
#define IT_TECH			64
//ZOID
// ROGUE
#define IT_MELEE			0x00000080 //128
#define IT_NOT_GIVEABLE		0x00000100	// item can not be given- 256
#define IT_ALTWEAPON	512

#define IT_XATRIX			0x00000100	// Xatrix item
#define IT_ROGUE			0x00000200	// Rogue item
#define IT_LAZARUS			0x00000400	// Lazarus item
#define IT_LMCTF			0x00000800	// Loki's Minions CTF item

// ROGUE

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
#define WEAP_GRAPPLE			12

#define WEAP_PHALANX			13
#define WEAP_BOOMER				14
#define WEAP_DISRUPTOR			15		// PGM
#define WEAP_ETFRIFLE			16		// PGM
#define WEAP_PLASMA				17		// PGM
#define WEAP_PROXLAUNCH			18		// PGM
#define WEAP_CHAINFIST			19		// PGM

#define WEAP_PLASMARIFLE		20
#define WEAP_SHOCKWAVE			21

typedef struct gitem_s
{
	char		*classname;	// spawning name
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

	// items
	int			num_items;

	qboolean	autosaved;
} game_locals_t;


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

	// Knightmare- for map-specific logic switching
	maptype_t	maptype;				// map vendor for pack-specific code changes
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
	float		radius;
	float		alpha;
	int			fade_start_dist;
	int			fade_end_dist;
	char		*image;
	char		*rgba;

	// Knightmare- added for angled sprites
	int			spritetype;
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
	// we can't currently add these to moveinfo_t because of precompiled p_trail.c!
	float		ratio;
	qboolean	is_blocked;
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
extern	int	blaster_index;
extern	int	pl_index;
extern	int	rg_index;
extern	int	pr_index;		// SKWiD MOD


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
#define MOD_GRAPPLE			34
// RAFAEL 14-APR-98
#define MOD_RIPPER				35 // AJ - increased these all by one... why use CTF grapple index??
#define MOD_PHALANX				36
#define MOD_BRAINTENTACLE		37
#define MOD_BLASTOFF			38
#define MOD_GEKK				39
#define MOD_TRAP				40
// END 14-APR-98

// AJ - funhook
#define MOD_HOOK_DAMAGE1        41
#define MOD_HOOK_VAMPIRE1       42
#define MOD_HOOK_VAMPIRE2       43

//========
// ROGUE
#define MOD_CHAINFIST			44
#define MOD_DISINTEGRATOR		45
#define MOD_ETF_RIFLE			46
#define MOD_BLASTER2			47
#define MOD_HEATBEAM			48
#define MOD_TESLA				49
#define MOD_PROX				50
#define MOD_NUKE				51
#define MOD_VENGEANCE_SPHERE	52
#define MOD_HUNTER_SPHERE		53
#define MOD_DEFENDER_SPHERE		54
#define MOD_TRACKER				55
#define MOD_DBALL_CRUSH			56
#define MOD_DOPPLE_EXPLODE		57
#define MOD_DOPPLE_VENGEANCE	58
#define MOD_DOPPLE_HUNTER		59
#define MOD_ETF_SPLASH			60
// end ROGUE
//========
#define	MOD_PLASMA				61	// SKWiD MOD
#define MOD_SHOCK_SPHERE		62
#define MOD_SHOCK_SPLASH		63
#define MOD_AM_ROCKET			64
// end AJ



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
extern	cvar_t	*skill;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
//ZOID
extern	cvar_t	*capturelimit;
//ZOID
extern	cvar_t	*passwd;
extern	cvar_t	*spectator_password;
extern	cvar_t	*g_select_empty;
extern	cvar_t	*dedicated;

// AJ - funhook
extern  cvar_t  *sk_hook_is_homing;
extern  cvar_t  *sk_hook_homing_radius;
extern  cvar_t  *sk_hook_homing_factor;
extern  cvar_t  *sk_hook_players;
// extern	cvar_t  *sk_hook_sky;
extern  cvar_t  *sk_hook_min_length;
extern  cvar_t  *sk_hook_max_length;
extern  cvar_t  *sk_hook_pull_speed;
extern  cvar_t  *sk_hook_fire_speed;
extern  cvar_t  *sk_hook_messages;
extern  cvar_t  *sk_hook_vampirism;
extern  cvar_t  *sk_hook_vampire_ratio;
// end AJ

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

//ZOID
extern	qboolean	is_quad;
//ZOID

extern	cvar_t	*bot_num;
extern	cvar_t	*bot_name;
extern	cvar_t	*bot_allow_client_commands;
extern	cvar_t	*bot_free_clients;
extern	cvar_t	*bot_debug;
extern	cvar_t	*bot_show_connect_info;
extern	cvar_t	*bot_calc_nodes;
extern	cvar_t	*bot_debug_nodes;
extern	cvar_t	*bot_auto_skill;
extern	cvar_t	*bot_drop;
extern	cvar_t	*bot_chat;
extern	cvar_t	*bot_optimize;
extern	cvar_t	*bot_tarzan;

extern	cvar_t	*players_per_team;
extern	cvar_t	*addteam;
extern	cvar_t	*teamplay;
extern	cvar_t	*ctf_auto_teams;
extern	cvar_t	*grapple;
extern	cvar_t	*ctf_special_teams; // ~~JLH
extern	cvar_t	*ctf_humanonly_teams; // ~~JLH

extern	cvar_t	*view_weapons;

//ROGUE
extern	cvar_t	*g_showlogic;
extern	cvar_t	*gamerules;
extern	cvar_t	*huntercam;
extern	cvar_t	*strong_mines;
extern	cvar_t	*randomrespawn;

extern	cvar_t	*turn_rider;	// Knightmare added

extern	cvar_t	*g_aimfix;				// Knightmare- from Yamagi Q2
extern	cvar_t	*g_aimfix_min_dist;		// Knightmare- minimum range for aimfix
extern	cvar_t	*g_aimfix_taper_dist;	// Knightmare- transition range for aimfix

#define world	(&g_edicts[0])

// item spawnflags
#define ITEM_TRIGGER_SPAWN		0x00000001
#define ITEM_NO_TOUCH			0x00000002
#define ITEM_NO_ROTATE			0x00000004	// Knightmare added
#define ITEM_NO_DROPTOFLOOR		0x00000008	// Knightmare added
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
	char		*name;
	size_t		ofs;	// Knightmare- was int
	fieldtype_t	type;
	int			flags;
} field_t;


extern	field_t fields[];
extern	gitem_t	itemlist[];


//
// g_cmds.c
//
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
int ArmorIndex (edict_t *ent);
int PowerArmorType (edict_t *ent);
gitem_t	*GetItemByIndex (int index);
int GetMaxAmmoByIndex (gclient_t *client, int item_index);
qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count);
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

//
// g_utils.c
//
qboolean	KillBox (edict_t *ent);
void	G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
edict_t *G_Find (edict_t *from, size_t fieldofs, char *match);	// Knightmare- changed fieldofs from int
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

char *GameDir (void);
char *SavegameDir (void);
void GameDirRelativePath (const char *filename, char *output, size_t outputSize);
void SavegameDirRelativePath (const char *filename, char *output, size_t outputSize);
void CreatePath (const char *path);
qboolean LocalFileExists (const char *path);

qboolean IsIdMap (void); // Knightmare added
qboolean IsXatrixMap (void); // Knightmare added
qboolean IsRogueMap (void); // Knightmare added
qboolean IsZaeroMap (void); // Knightmare added
qboolean CheckCoop_MapHacks (edict_t *ent); // FS: Coop: Check if we have to modify some stuff for coop so we don't have to rely on distributing ent files

// Ridah
void stuffcmd(edict_t *ent, char *text);
float	entdist(edict_t *ent1, edict_t *ent2);
void AddModelSkin (char *modelfile, char *skinname);

void my_bprintf (int printlevel, char *fmt, ...);
// end
// ACE compatibility routines
void safe_bprintf (int printlevel, char *fmt, ...);
void safe_centerprintf (edict_t *ent, char *fmt, ...);
void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...);

//
// g_combat.c
//
qboolean OnSameTeam (edict_t *ent1, edict_t *ent2);
qboolean CanDamage (edict_t *targ, edict_t *inflictor);
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker);
void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod);
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);
void T_RadiusNukeDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);

// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect 1
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage 2
#define DAMAGE_ENERGY			0x00000004	// damage is from an energy based weapon 4
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles 8
#define DAMAGE_BULLET			0x00000010  // damage is from a bullet (used for ricochets) 16
#define DAMAGE_NO_PROTECTION	0x00000020  // armor, shields, invulnerability, and godmode have no effect 32

//ROGUE
#define DAMAGE_DESTROY_ARMOR	0x00000040	// damage is done to armor and health. 64
#define DAMAGE_NO_REG_ARMOR		0x00000080	// damage skips regular armor 128
#define DAMAGE_NO_POWER_ARMOR	0x00000100	// damage skips power armor 256
//ROGUE

#define DEFAULT_BULLET_HSPREAD	300
#define DEFAULT_BULLET_VSPREAD	500
#define DEFAULT_SHOTGUN_HSPREAD	1000
#define DEFAULT_SHOTGUN_VSPREAD	500
#define DEFAULT_DEATHMATCH_SHOTGUN_COUNT	12
#define DEFAULT_SHOTGUN_COUNT	12
#define DEFAULT_SSHOTGUN_COUNT	20

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
// end Knightmare

void door_use_areaportals (edict_t *self, qboolean open);


//
// g_monster.c
//
void monster_fire_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int kick, int hspread, int vspread, int flashtype, int mod);
void monster_fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int flashtype, int mod);
void monster_fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect, int color);
void monster_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int flashtype);
void monster_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype);
void monster_fire_railgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int flashtype);
void monster_fire_bfg (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int kick, float damage_radius, int flashtype);
// RAFAEL
void monster_fire_ionripper (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect);
void monster_fire_heat (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype);
void monster_dabeam (edict_t *self);
void monster_fire_blueblaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect);

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
//void ThrowHeadACID (edict_t *self, char *gibname, int damage, int type);
//void ThrowGibACID (edict_t *self, char *gibname, int damage, int type);
void Cmd_PurgeGibs_f (edict_t *ent); //ScarFace

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

qboolean visible_box (edict_t *self, edict_t *other);
qboolean visible_fullbox (edict_t *self, edict_t *other);

//
// g_weapon.c
//
#define BLASTER_ORANGE	1
#define BLASTER_GREEN	2
#define BLASTER_BLUE	3
#define BLASTER_RED		4

void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed);
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin);
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick);
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void fire_blaster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect, qboolean hyper, int color);
void Grenade_Explode (edict_t *ent);
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean contact);
void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, qboolean useColor, int red, int green, int blue);
void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);
void fire_shock_sphere (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
#ifdef USE_AM_ROCKETS
void fire_am_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
#endif	// USE_AM_ROCKETS

//
// g_weapon_xatrix.c
//
void fire_ionripper (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect);
void fire_rocket_heat (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_blueblaster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect);
void fire_phalanx_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_trap (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void Cmd_KillTrap_f (edict_t *ent);
void Trap_Explode (edict_t *self);

//
// g_weapon_lmctf.c
//
void fire_plasma_rifle (edict_t *ent, vec3_t start, vec3_t dir, int damage, int speed, qboolean spread);	// SKWiD MOD

//====================
// ROGUE PROTOTYPES
//
// g_newweap.c
//
//extern float nuke_framenum;

void fire_flechette (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
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
void fire_heat_rogue (edict_t *self, vec3_t start, vec3_t aimdir, vec3_t offset, int damage, int kick, qboolean monster);
void fire_tracker (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, edict_t *enemy);


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
qboolean FindSpawnPoint (vec3_t startpoint, vec3_t mins, vec3_t maxs, vec3_t spawnpoint, float maxMoveUp);
qboolean CheckSpawnPoint (vec3_t origin, vec3_t mins, vec3_t maxs);
qboolean CheckGroundSpawnPoint (vec3_t origin, vec3_t entMins, vec3_t entMaxs, float height, float gravity);
void DetermineBBox (char *classname, vec3_t mins, vec3_t maxs);
void SpawnGrow_Spawn (vec3_t startpos, int size);


//
// p_client.c
//
void RemoveAttackingPainDaemons (edict_t *self);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
float PlayersRangeFromSpot (edict_t *spot);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);


//
// p_trail.c
//
void PlayerTrail_Init (void);
void PlayerTrail_Add (edict_t *self, vec3_t spot, edict_t *goalent, int nocheck, int calc_routes, int node_type);
void PlayerTrail_New (vec3_t spot);
edict_t *PlayerTrail_PickFirst (edict_t *self);
edict_t *PlayerTrail_PickNext (edict_t *self);
edict_t	*PlayerTrail_LastSpot (void);

void PlayerTrail_FindPaths(int marker);
edict_t *PlayerTrail_VisibleTrailInRange(edict_t *ent1, edict_t *ent2, float range);
edict_t *bot_PickBestTrail (edict_t *self, edict_t *target, int recurse_depth);
edict_t *bot_TouchedTrail(edict_t *ent, edict_t *targ);

// Knightmare- made these vars extern to fix GCC compile
extern	qboolean nodes_done;		// used to determine whether or not to enable node calculation
extern	edict_t	*check_nodes_done;	// after 20 mins of play, this ent checks if we should turn off node calc. at set intervals

//extern	edict_t *last_trail_dropped;
extern	qboolean	loaded_trail_flag;

#define		TRAIL_LENGTH			750
extern	edict_t		*trail[TRAIL_LENGTH];			// the actual trail!

#define		TRAIL_TELEPORT_SOURCE	1


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

// Knightmare- made this extern to fix GCC compile
extern	float	last_trail_time;

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
// p_weapon.c
//
void PlayerNoise(edict_t *who, vec3_t where, int type);
void P_ProjectSource (edict_t *client_ent, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);	// Knightmare- changed parms for aimfix
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent, qboolean altfire));

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


//#define	BOT_FRAMETIME	0.1
// Knightmare- made this extern to fix GCC compile
extern	double	bot_frametime;

//
// g_chase.c
//
void UpdateChaseCam (edict_t *ent);
void ChaseNext (edict_t *ent);
void ChasePrev (edict_t *ent);
void GetChaseTarget (edict_t *ent);

// Knightmare- made these extern to fix GCC compile
extern	int 	max_bots;
extern	float	last_bot_spawn;
extern	int		bot_male_names_used;
extern	int 	bot_female_names_used;
extern	int 	bot_count;

#define	MAX_BOT_FEMALE_NAMES		10
#define	MAX_BOT_MALE_NAMES			15

//
// g_compress.c
//
int G_OpenFile (const char *name, fileHandle_t *f, fsMode_t mode);
int G_OpenCompressedFile (const char *zipName, const char *fileName, fileHandle_t *f, fsMode_t mode);
void G_CloseFile (fileHandle_t f);
int G_FRead (void *buffer, int size, fileHandle_t f);
int G_FWrite (const void *buffer, int size, fileHandle_t f);
int G_CompressFile (const char *fileName, const char *zipName, const char *internalName, qboolean appendToZip);
int G_DecompressFile (const char *fileName, const char *zipName, const char *internalName);

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
	qboolean	powerArmorActive;

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

	qboolean	spectator;		// client is a spectator

//=========
//ROGUE
	int			max_tesla;
	int			max_prox;
	int			max_mines;
	int			max_flechettes;
#ifndef KILL_DISRUPTOR
	int			max_rounds;
#endif
//ROGUE
//=========

	int			max_armor;		// ADJ

	// Knightmare- custom client colors
	color_t		color1;
	color_t		color2;
} client_persistant_t;

// client data that staus across deathmatch respawns
typedef struct
{
	client_persistant_t	coop_respawn;	// what to set client->pers to on a respawn
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
//ZOID
	int			ctf_team;			// CTF team
	int			ctf_state;
	float		ctf_lasthurtcarrier;
	float		ctf_lastreturnedflag;
	float		ctf_flagsince;
	float		ctf_lastfraggedcarrier;
	qboolean	id_state;
//ZOID
	vec3_t		cmd_angles;			// angles sent over in the last command
	int			game_helpchanged;
	int			helpchanged;

// AJ - antizbot code
	int			bot_retry;
	int			bot_start;
	int			bot_end;
// end AJ

	qboolean	spectator;			// client is a spectator
} client_respawn_t;

//========================================================================================

#define BOTDATA_STRINGSIZE	128	// Knightmare added

typedef struct
{
	float		accuracy;		// (1-5)	aiming accuracy
	float		aggr;			// (1-5)	higher aggression will attack more players, rather than collect items
	float		combat;			// (1-5)	effects strafing, jumping and crouching while firing
	gitem_t		*fav_weapon;	//			will seek this weapon from further distances, and attack more frequently when gained
	int			quad_freak;		// (0/1)	won't seek quad from far away if no
	int			camper;			// (0/1)	more likely to find a dark corner with favourite weapon
	int			avg_ping;		//			average ping time shown on scoreboard (just for looks)
} bot_stats_t;

// this stores a linked list of bots, which are read in from bots.cfg when the game starts
struct bot_info_s
{
	struct bot_info_s	*next;

	char	*name;
	char	*skin;

	// Knightmare- custom railgun colors
	char	*color1;

	int		ingame_count;	// incremented each time this bot enters the game

	// stats
	bot_stats_t		bot_stats;

};

typedef struct bot_info_s bot_info_t;

// TEAMPLAY structures
#define MAX_PLAYERS_PER_TEAM	32
#define MAX_TEAMS				64

struct bot_team_s
{
	// static data
	char *teamname;
	char *abbrev;
	char *default_skin;

	bot_info_t	*bots[MAX_PLAYERS_PER_TEAM];

	// in-game stuff
	int		ingame;			// set when this team has been spawned
	int		score;
	int		num_players;	// incremented each time a player/bot joins this team
	int		num_bots;

	float	last_grouping;
};

typedef struct bot_team_s bot_team_t;

// Knightmare- made these extern to fix GCC compile
extern	bot_team_t	*bot_teams[MAX_TEAMS];
extern	int			total_teams;
// END Teamplay structure
//========================================================================================

typedef vec3_t	lag_trail_t[10];

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
//ZOID
	qboolean	inmenu;				// in menu
	pmenuhnd_t	*menu;				// current menu
//ZOID
	qboolean	showinventory;		// set layout stat
	qboolean	showhelp;
	qboolean	showhelpicon;

	int			ammo_index;

	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

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

	///Q2 Camera Begin
    qboolean    bIsCamera;
    int         iMode;
    edict_t     *pTarget;
    qboolean    bWatchingTheDead;
    vec3_t      vDeadOrigin;
    double       fXYLag;
    double       fZLag;
    double       fAngleLag;
	///Q2 Camera End

	// Eraser additions
	bot_team_t	*team;
	float		firing_delay;
	float		latency;
	lag_trail_t	*lag_trail;			// stores the last 1 second of movements for lag simulation
	lag_trail_t *lag_angles;
	qboolean		ctf_has_tech;
	float		slide_time;
	float		reached_goal_time;

//ZOID
	edict_t		*ctf_grapple;		// entity of grapple
	int			ctf_grapplestate;		// true if pulling
	float		ctf_grapplereleasetime;	// time of grapple release
	float		ctf_grapplestart;
	float		ctf_regentime;		// regen tech
	float		ctf_techsndtime;
	float		ctf_lasttechmsg;
	edict_t		*chase_target;
	qboolean	update_chase;
//ZOID

// AJ added
	// Expert: Grapple fields
	edict_t		*hook;				// Pointer to grappling hook entity
	edict_t		*hook_touch;		// Edict that hook has touched
	qboolean	on_hook;			// Shows if player is being moved by hook
	qboolean	oh_hook_button;		// Shows if player is being moved by hook

	// The hook needs to be sure not to fire right after teleporting,
	// since there are some timing issues involved that mess up the
	// firing angle. client->ps.pmove.teleport_time is set by events other
	// than teleported, so hook_frame is needed as a second timer.
	int			hook_frame;			// Time to fire hook

	// Expert Pogo
	int		 	pogo_time;	// Time when player can pogo again

	// funhook
	int hookstate;
    int	hook_vampire_time;

	qboolean	observer_mode;
	qboolean	safety_mode;
	float		safety_time;

//=======
//ROGUE
	float		double_framenum;
	float		ir_framenum;
//	float		torch_framenum;
	float		nuke_framenum;
	float		tracker_pain_framenum;

	edict_t		*owned_sphere;		// this points to the player's sphere
//ROGUE
//=======

	qboolean	hook_out;
	qboolean	hook_on;

// end AJ

};

// this stores the route-table data
typedef struct
{
	short int	route_path[TRAIL_LENGTH];	// visible node that will take us to each node
	unsigned short int	route_dist[TRAIL_LENGTH];	// dist to each node
} routes_t;

#define MAX_PATHS	24			// each node cannot see more than MAX_PATHS nodes
// Knighmare- made this extern to fix GCC compile
extern	edict_t	*the_client;			// points to the first client to enter the game (used for debugging)

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
	int			movetype;
	int			flags;

	char		*model;
	float		freetime;			// sv.time when the object was freed

	//
	// only used locally in game, not by server
	//
	char		*message;
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

	vec3_t		move_origin;
	vec3_t		move_angles;

	// move this to clientinfo?
	int			light_level;

	int			style;			// also used as areaportal number

	gitem_t		*item;			// for bonus items

	// common data blocks
	moveinfo_t		moveinfo;
	monsterinfo_t	monsterinfo;

	// bot data (FIXME: these should really be in a separate structure!)
	edict_t		*last_goal;
	float		last_goal_time;
	float		movetogoal_time;
	float		strafe_dir, strafe_changedir_time;
	float		crouch_attack_time;
	float		last_enemy_sight, search_time;
	float		last_besttrail_dist;		// updated when a trail is found
	float		last_path_enemy_timestamp;
	float		bored_suicide_time;			// for suiciding when getting stuck
	edict_t		*node_target;
	vec3_t		jump_velocity;
	vec3_t		checkstuck_origin;
	float		checkstuck_time;
	qboolean	bot_client;
//	struct gclient_s	*bot_client;		// stores client relevant data for bots, like weapon ammo, health, etc
	float		fire_interval, last_fire;
	void		(*bot_fire) (edict_t *self);
	float		last_ladder_touch;
	float		bot_plat_pausetime;
	int			closest_trail;
	float		closest_trail_time;
	float		last_think_time;
	float		last_reached_trail;

#ifdef _WIN32
	struct _timeb	lastattack_time;
#else
	struct timeb	lastattack_time;
#endif

	vec3_t		animate_org;
	float		last_inair;
	int			movetarget_want;
	edict_t		*avoid_ent;					// avoid grenade, rocket, etc
	vec3_t		avoid_dir;					// direction to run to avoid danger
	float		avoid_dir_time;				// amount of time to spend avoiding in this direction, before looking for a new direction
	float		last_roam_time;
	float		sight_enemy_time;
	float		last_best_direction;

	bot_stats_t	*bot_stats;
	bot_info_t	*botdata;
	int			skill_level;

	int			last_closest_node;
	float		last_closest_time;
	float		last_move_nocloser;

	edict_t		*giveup_lastgoal;			// checked each frame to see if we've been going for the same goal for too long
	float		last_seek_enemy;

	float		last_insult, last_help, last_drop, last_group;
	float		group_pausetime;
	float		last_pain, last_jump;
	float		last_nopaths_roam;
	edict_t		*save_movetarget, *save_goalentity;
	edict_t		*last_movegoal;

	vec3_t		last_forward, last_start;		// used to prevent calculating twice per FRAMETIME
	edict_t		*flagpath_goal;
	float		activator_time;

	// trail data
	int			paths[MAX_PATHS];
	int			ignore;
	int			trail_index;	// for backwards referencing
	edict_t		*trigger_ent;	// points to the trigger entity that is associated with this node
	char		node_type;		// uses NOTE_* values

	routes_t		*routes;		// routes to all other nodes

	edict_t		*last_trail_dropped;	// this player's last trail node dropped

	// misc
	float		ignore_time;	// ignore hunting this ent while > level.time
	int			frags;			// used by plats/teleporters so we know they've been routed

	// used by player, for node calculation
	float last_max_z;
	edict_t *last_groundentity;
	edict_t	*jump_ent;		// set everytime the player jumps
	edict_t	*duck_ent;		// set everytime the player begins to duck

	// used to speed up View Weapons code, so we don't need to find the model each frame, for each player
	char *modelname;

	// special ACE vars
	qboolean	isbot;
	int			ace_last_node;

// ===========================================================================
// 	DO NOT MODIFY ANYTHING ABOVE THIS !!
//	ERASER NEEDS THE FIELDS IN THIS ORDER (since P_TRAIL.C is precompiled)
//
//	You may safely add fields below this point

	char		*musictrack;	// Knightmare- for specifying OGG or CD track

	// Knightmare- added for func_door_secret
	vec3_t		pos0;	// Knightmare- initial position for secret doors
	float		width;
	float		length;
	float		side;

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

	// Knightmare- rotating train stuff
	float		pitch_speed;
//	float		yaw_speed;
	float		roll_speed;
//	float		ideal_yaw;
	float		ideal_pitch;
	float		ideal_roll; // roll value for a path_corner
	float		roll;		// roll value for a path_corner
	int			turn_rider;	// whether to turn rider
	// added these here because we can't add to moveinfo_t because of precompiled p_trail.c!
//	float		moveinfo_ratio;
//	qboolean	moveinfo_is_blocked;

	// spline train
	edict_t		*from;
	edict_t		*to;
	// if true, brush models will move directly to Move_Done
	// at Move_Final rather than slowing down.
	qboolean	smooth_movement;
	vec3_t      origin_offset;  // used to help locate brush models w/o origin brush
	// gib fields
	int			gib_type;
	int			blood_type;
	// end Knightmare

// rogue (aj)
	int			plat2flags;
	vec3_t		offset;
	vec3_t		gravityVector;
	edict_t		*bad_area;
	edict_t		*hint_chain;
	edict_t		*monster_hint_chain;
	edict_t		*target_hint_chain;
	int			hint_chain_id;
	edict_t		*laser;

	void		(*play)(edict_t *self, edict_t *activator);	// Knightmare added
	void		(*postthink) (edict_t *ent);		// Knightmare added
};

// Knightmare- made these extern to fix GCC compile
extern	int	num_players;
extern	edict_t *players[MAX_CLIENTS];		// pointers to all players in the game
extern	edict_t	*weapons_head;				// pointers to all weapons in the game (use node_target and last_goal to traverse forward/back)
extern	edict_t	*health_head;
extern	edict_t	*bonus_head;				// armour, Quad, etc
extern	edict_t	*ammo_head;

// the following are just faster ways of accessing FindItem("item_name"), set in Worldspawn
extern	gitem_t	*item_shells, *item_cells, *item_rockets, *item_grenades, *item_slugs, *item_bullets;
extern	gitem_t	*item_shotgun, *item_hyperblaster, *item_supershotgun, *item_grenadelauncher, *item_chaingun, *item_railgun, *item_machinegun, *item_bfg10k, *item_rocketlauncher, *item_blaster;
extern	gitem_t *item_proxlauncher, *item_disruptor, *item_etfrifle, *item_chainfist, *item_plasmabeam, *item_rounds, *item_flechettes, *item_tesla, *item_prox, *item_plasmarifle, *item_shockwave;

extern	bot_info_t	*botinfo_list;
extern	int			total_bots;		// number of bots read in from bots.cfg

extern	bot_info_t	*teambot_list;	// bots that were generated solely for teamplay

extern	qboolean paused;			// fake a paused game during deathmatch

//ZOID
#include "g_ctf.h"
//ZOID

// AJ
#include "aj_main.h"

#define USE_ID_VWEP			1
#define USE_ITEMTABLE_VWEP	1
// end AJ


// ROGUE

#define SPHERE_DEFENDER			0x0001
#define SPHERE_HUNTER			0x0002
#define SPHERE_VENGEANCE		0x0004
#define SPHERE_DOPPLEGANGER		0x0100

#define SPHERE_TYPE				0x00FF
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
	void		(*SelectSpawnPoint) (edict_t *ent, vec3_t origin, vec3_t angles);
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
void DBall_SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
int  DBall_ChangeKnockback (edict_t *targ, edict_t *attacker, int knockback, int mod);
int  DBall_ChangeDamage (edict_t *targ, edict_t *attacker, int damage, int mod);
void DBall_PostInitSetup (void);
int  DBall_CheckDMRules (void);


float realrange (edict_t *self, edict_t *other);

void G_ProjectSource2 (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t up, vec3_t result);
float vectoyaw2 (vec3_t vec);
void vectoangles2 (vec3_t value1, vec3_t angles);
