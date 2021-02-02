// g_local.h -- local definitions for game module

#include "q_shared.h"

// Define GAME_INCLUDE so that game.h does not define the short, server-visible 
// gclient_t and edict_t structures, because we define the full size ones in this file.
#define	GAME_INCLUDE
#include "game.h"
#include "p_menu.h"																					//ZOID++
#include "g_chase.h"																				//CW++


// The "gameversion" client command will print this plus the compile date.
#define	GAMEVERSION	"awaken2"																		//CW

//CW++
#define AWK_VERSION			2.05
#define AWK_VSTRING2(x)		#x
#define AWK_VSTRING(x)		AWK_VSTRING2(x)
#define AWK_STRING_VERSION	AWK_VSTRING(AWK_VERSION)

/*
Author tags:
 CW   = Chris Walker (Musashi)
 ZOID = David Kirsch (Zoid)
 DH   = David Hyde
 SNX  = Sn4xx0r
 Maj  = Philip Blair (Maj.Bitch)
 Pon  = Ponpoko
 r1   = r1ch


Version release dates:
 v2.00		20-Mar-2004
 v2.01		17-Apr-2004
 v2.02		21-Jun-2004
 v2.03		10-Mar-2005
 v2.04		25-Feb-2007
 v2.05		28-Apr-2007
*/
//CW--


// protocol bytes that can be directly added to messages
#define	svc_muzzleflash		1
#define	svc_muzzleflash2	2
#define	svc_temp_entity		3
#define	svc_layout			4
#define	svc_inventory		5
#define svc_sound			9																		//CW++

//==================================================================

// view pitching times
#define DAMAGE_TIME		0.5
#define	FALL_TIME		0.3


//CW++
// gametypes
#define G_FFA	0		// Free-For-All
#define	G_CTF	1		// Capture The Flag
#define	G_TDM	2		// Team Deathmatch
#define G_ASLT	3		// Assault
//CW--


// edict->spawnflags
// these are set with checkboxes on each entity in the map editor
#define	SPAWNFLAG_NOT_EASY			0x00000100
#define	SPAWNFLAG_NOT_MEDIUM		0x00000200
#define	SPAWNFLAG_NOT_HARD			0x00000400
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00000800
#define	SPAWNFLAG_NOT_COOP			0x00001000

// edict->flags
#define	FL_FLY					0x00000001
#define	FL_SWIM					0x00000002		// implied immunity to drowining
#define FL_IMMUNE_LASER			0x00000004
#define	FL_INWATER				0x00000008
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define FL_IMMUNE_SLIME			0x00000040
#define FL_IMMUNE_LAVA			0x00000080
#define	FL_PARTIALGROUND		0x00000100		// not all corners are valid
#define	FL_WATERJUMP			0x00000200		// player jumping out of water
#define	FL_TEAMSLAVE			0x00000400		// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_POWER_SHIELD			0x00001000		// power armor (if any) is active
#define FL_POWER_SCREEN			0x00002000		// Knightmare added
#define	FL_TURRET_OWNER			0x00004000		// player on turret and controlling it				//DH++

#define FL_RESPAWN				0x80000000		// used for item respawning

//CW++
// weapon notification text/voice flags
#define WN_TEXT					0x00000001		// display text
#define WN_VSEC					0x00000002		// play voice for weapons with secondary modes only
#define WN_VALL					0x00000004		// play voice for all weapons

#define REKILL_DELAY			5.0
//CW--

#define	FRAMETIME				0.1

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME				765				// clear when unloading the dll
#define	TAG_LEVEL				766				// clear when loading a new level

#define MELEE_DISTANCE			80

#define BODY_QUEUE_SIZE			8

typedef enum
{
	DAMAGE_NO,
	DAMAGE_YES,					// will take damage if hit
	DAMAGE_AIM					// auto targeting recognizes this
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
//CW++
	AMMO_NULL,
	AMMO_C4,
	AMMO_TRAPS,
//CW--
	AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_GRENADES,
	AMMO_CELLS,
	AMMO_SLUGS
} ammo_t;


//CW++
typedef enum
{
	POWERUP_NULL,
	POWERUP_INVULN,
	POWERUP_QUAD,
	POWERUP_ENVIRO,
	POWERUP_SILENCER,
	POWERUP_BREATHER,
	POWERUP_SIPHON,
	POWERUP_D89,
	POWERUP_HASTE,
	POWERUP_ANTIBEAM,
	POWERUP_TELE,
	POWERUP_MYSTERY
} powerup_t;
//CW--

//deadflag
#define DEAD_NO					0
#define DEAD_DYING				1
#define DEAD_DEAD				2
#define DEAD_RESPAWNABLE		3

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
MOVETYPE_BOUNCE,
MOVETYPE_FLYBOUNCE		// used for DL discs														//CW++
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
#define IT_PACK			64																			//Maj++
#define IT_HEALTH		128																			//Maj++
#define IT_NODE			256																			//Maj++
#define IT_TECH			512																			//ZOID++	//CW
#define IT_FLAG			1024																		//CW++

//CW++
// gitem_t->weapmodel for weapons indicates model index
#define WEAP_CHAINSAW			1
#define WEAP_DESERTEAGLE		2
#define WEAP_GAUSSPISTOL		3
#define WEAP_JACKHAMMER			4
#define WEAP_MAC10				5
#define WEAP_ESG				6
#define WEAP_C4					7
#define WEAP_TRAP				8
#define WEAP_ROCKETLAUNCHER		9
#define WEAP_FLAMETHROWER		10
#define WEAP_RAILGUN			11
#define WEAP_SHOCKRIFLE			12
#define WEAP_DISCLAUNCHER		13
#define WEAP_AGM				14
#define WEAP_GRAPPLE			15

// Entity identification values (arbitrary; large)
#define ENT_ID_PLAYER_SPAWN		666000
#define	ENT_ID_FAKER			666001
#define ENT_ID_DOOR_ROTATING	666002
#define ENT_ID_INTERMISSION		666003
#define ENT_ID_TELE_DEST		666004
//CW--

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
	int			count_width;	// number of digits to display by icon

	int			quantity;		// for ammo how much, for weapons how much is used per shot
	char		*ammo;			// for weapons
	int			flags;			// IT_* flags

	int			weapmodel;		// weapon model index (for weapons)

	void		*info;
	int			tag;

	char		*precaches;		// string of all models, sounds, and images this item will use
} gitem_t;

//Maj++
// Item Ammo (IT_AMMO)
gitem_t *item_shells,
        *item_cells,
        *item_rockets,
        *item_slugs,
        *item_bullets,

// Item Weapons (IT_WEAPON)
//CW++
		*item_chainsaw,
        *item_deserteagle,
        *item_gausspistol,
        *item_mac10,
        *item_jackhammer,
        *item_esg,
        *item_c4,
        *item_trap,
        *item_rocketlauncher,
        *item_flamethrower,
        *item_railgun,
        *item_shockrifle,
		*item_disclauncher,
		*item_agm,
		*item_grapple,
//CW--

// Item Armor (IT_ARMOR)
        *item_jacketarmor,
        *item_combatarmor,
        *item_bodyarmor,
        *item_armorshard,
        *item_powerscreen,
        *item_powershield,

// Item Health (IT_HEALTH)
        *item_adrenaline,
        *item_health,
        *item_stimpak,
        *item_health_large,
        *item_health_mega,

// Item Powerup (IT_POWERUP)
        *item_quad,
        *item_invulnerability,
        *item_silencer,
        *item_breather,
        *item_enviro,
		*item_teleporter,																			//CW++

// Item Pak (IT_PACK)
        *item_pack,
        *item_bandolier,

// Item Nodes (IT_NODE)
        *item_navi1,
        *item_navi2,
        *item_navi3;
//Maj--


// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
typedef struct
{
	char		helpmessage1[512];
	char		helpmessage2[512];
	int			helpchanged;			// flash F1 icon if non 0; play sound and increment only if 1, 2, or 3

	gclient_t	*clients;				// [maxclients]

	// can't store spawnpoint in level, because it would get overwritten by the savegame restore
	char		spawnpoint[512];

	// store latched cvars here that we want to get at often
	int			maxclients;
	int			maxentities;

	// cross level triggers
	int			serverflags;

	// items
	int			num_items;

	qboolean	autosaved;
} game_locals_t;


// This structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames.
typedef struct
{
	int			framenum;
	float		time;

	char		level_name[MAX_QPATH];		// the descriptive name (Outer Base, etc)
	char		mapname[MAX_QPATH];			// the server name (base1, etc)
	char		nextmap[MAX_QPATH];			// go here when fraglimit is hit
	char		forcemap[MAX_QPATH];		// go here

	// intermission state
	float		intermissiontime;			// time the intermission was started
	char		*changemap;
	int			exitintermission;
	vec3_t		intermission_origin;
	vec3_t		intermission_angle;

	int			pic_health;

	edict_t		*current_entity;			// entity running from G_RunFrame
	int			body_que;					// dead bodies

//CW++
	float		starttime;

	qboolean	nohook;

	edict_t		*leader;
	int			lead_score;
//CW--
} level_locals_t;

//CW++
// Voting info
typedef enum
{
	VOTE_NONE,
	VOTE_MAP,
	VOTE_CONFIG,
	VOTE_MATCH,

	VOTE_TIMELIMIT,
	VOTE_FRAGLIMIT,
	VOTE_CAPLIMIT,
	VOTE_GRAPPLE,
	VOTE_GRAPPLE_OFFHAND,

	VOTE_GAUSSPISTOL,
	VOTE_MAC10,
	VOTE_JACKHAMMER,
	VOTE_C4,
	VOTE_TRAPS,
	VOTE_ESG,
	VOTE_ROCKETLAUNCHER,
	VOTE_FLAMETHROWER,
	VOTE_RAILGUN,
	VOTE_SHOCKRIFLE,
	VOTE_AGM,
	VOTE_DISCLAUNCHER,

	VOTE_INVULN,
	VOTE_QUAD,
	VOTE_SIPHON,
	VOTE_D89,
	VOTE_HASTE,
	VOTE_TELE,
	VOTE_ANTIBEAM,
	VOTE_ENVIRO,
	VOTE_SILENCER,
	VOTE_BREATHER,

	VOTE_NOHEALTH,
	VOTE_NOITEMS,
	VOTE_WEAPONSSTAY,
	VOTE_NOFALLING,
	VOTE_NOARMOR,
	VOTE_INFINITEAMMO,
	VOTE_QUADDROP,
	VOTE_FASTSWITCH,
	VOTE_EXTRAITEMS,
	VOTE_NOREPLACEMENTS,
	VOTE_NOTECHS,
	VOTE_CTFSPAWNS,

	VOTE_ADDBOTS,
	VOTE_REMOVEBOTS,
	VOTE_REMOVEALLBOTS
} vote_t;

typedef struct voteinfo_s
{
	vote_t	vote;							// vote type
	char	vmsg[256];						// vote name
	char	vstring[MAX_QPATH];				// for map vote = target level; for config vote = .cfg file
	float	votetime;						// remaining time until vote times out
	int		nvotes;							// votes so far
	int		needvotes;						// votes needed
	int		vnum;							// timelimit, fraglimit, gametype, etc
} voteinfo_t;

typedef struct vote_settings_s
{
	int		 map;
	int		 config;
	int		 timelimit;
	int		 fraglimit;
	int		 caplimit;
	qboolean hook;
	qboolean hook_offhand;
	qboolean match;

	qboolean gausspistol;
	qboolean mac10;
	qboolean jackhammer;
	qboolean c4;
	qboolean traps;
	qboolean esg;	
	qboolean rocketlauncher;
	qboolean flamethrower;
	qboolean railgun;
	qboolean shockrifle;
	qboolean agm;
	qboolean disclauncher;

	qboolean invuln;
	qboolean quad;
	qboolean siphon;
	qboolean d89;
	qboolean haste;
	qboolean tele;
	qboolean antibeam;
	qboolean enviro;
	qboolean silencer;
	qboolean breather;

	qboolean no_health;
	qboolean no_items;
	qboolean weapons_stay;
	qboolean no_falling;
	qboolean no_armor;
	qboolean infinite_ammo;
	qboolean quad_drop;
	qboolean fast_switch;
	qboolean extra_items;
	qboolean no_replacements;
	qboolean no_techs;
	qboolean ctf_spawns;

	int		 bots_added;
	int		 bots_removed;

	vote_t	v_type;

	int		g_nmaps;
	int		g_nconfigs;
	char	*g_maplist[MAX_MAPS];					// "MAX_MAPS maps ought to be enough for anybody"
	char	*g_configlist[MAX_CONFIGS];
} vote_settings_t;

// Filetype info.
typedef enum
{
	FILE_MAP,
	FILE_MODEL,
	FILE_SOUND,
	FILE_TEXTURE
} filetype_t;
//CW--

//DH++
// Pakfile info.

typedef struct
{
	char	id[4];			// should be 'PACK'
	int		dstart;			// offest in the file to the directory
	int		dsize;			// size in bytes of the directory, same as num_items*64
} pak_header_t;

typedef struct
{
	char	name[56];		// the name of the item, normal C string
	int		start;			// offset in .pak file to start of item
	int		size;			// size of item in bytes
} pak_item_t;
//DH--

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
	// Knightmare added
	float		ratio;
	qboolean	is_blocked;
	// end Knightmare
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

//CW++
extern	voteinfo_t		g_vote;

extern	level_locals_t	g_oldmaps[MAX_OLDMAPS];
extern	edict_t			*g_slots[MAX_CLIENTS];
extern	int				g_reserve_used;
extern	int				g_public_used;
extern	int				g_round;

extern int				hook_index;
extern int				spike_index;
extern int				r_explode_index;
extern int				tracer_index;
//CW--

extern	int	sm_meat_index;
extern	int	snd_fry;

extern	int	jacket_armor_index;
extern	int	combat_armor_index;
extern	int	body_armor_index;


// means of death
#define MOD_UNKNOWN			0
#define MOD_EXIT			1
#define MOD_WATER			2
#define MOD_SLIME			3
#define MOD_LAVA			4
#define MOD_CRUSH			5
#define MOD_TELEFRAG		6
#define MOD_FALLING			7
#define MOD_SUICIDE			8
#define MOD_EXPLOSIVE		9
#define MOD_BARREL			10
#define MOD_BOMB			11
#define MOD_SPLASH			12
#define MOD_TARGET_LASER	13
#define MOD_TARGET_BLASTER	14
#define MOD_TRIGGER_HURT	15
#define MOD_FORBIDDEN		16																		//CW

#define MOD_MACHINEGUN		17
#define MOD_ROCKET			18
#define MOD_R_SPLASH		19
#define MOD_RAILGUN			20
#define MOD_GRAPPLE			21

//CW++
#define MOD_DESERTEAGLE		22
#define MOD_JACKHAMMER		23
#define MOD_MAC10			24
#define MOD_GAUSS_BLASTER	25
#define MOD_GAUSS_BEAM		26
#define MOD_GAUSS_BEAM_REF	27
#define MOD_TRAP			28
#define MOD_TRAP_HELD		29
#define MOD_C4				30
#define MOD_C4_HELD			31
#define MOD_C4_PROXIMITY	32
#define MOD_C4_LIFETIME		33
#define MOD_SPIKE			34
#define MOD_SPIKE_SPLASH	35
#define MOD_FLAMETHROWER	36
#define MOD_FLAME			37
#define MOD_FIREBOMB		38
#define MOD_FIREBOMB_SPLASH	39
#define MOD_SR_HOMING		40
#define MOD_SR_DISINT		41
#define MOD_SR_DISINT_WAVE	42
#define MOD_D89				43
#define	MOD_AGM_FLING		44
#define MOD_AGM_SMASH		45
#define MOD_AGM_DROP		46
#define MOD_AGM_THROW		47
#define MOD_AGM_HIT			48
#define MOD_AGM_LAVA_HELD	49
#define MOD_AGM_LAVA_DROP	50
#define MOD_AGM_SLIME_HELD	51
#define MOD_AGM_SLIME_DROP	52
#define MOD_AGM_WATER_HELD	53
#define MOD_AGM_TRIG_HURT	54
#define	MOD_AGM_TARG_LASER	55
#define MOD_AGM_FEEDBACK	56
#define MOD_AGM_BEAM_REF	57
#define MOD_AGM_DISRUPT		58
#define MOD_PLASMA			59
#define MOD_DISC			60
#define MOD_CHAINSAW		61
//CW--
#define MOD_FRIENDLY_FIRE	0x8000000

extern	int	meansOfDeath;

#define	FOFS(x) (size_t)&(((edict_t *)0)->x)
#define	STOFS(x) (size_t)&(((spawn_temp_t *)0)->x)
#define	LLOFS(x) (size_t)&(((level_locals_t *)0)->x)
#define	CLOFS(x) (size_t)&(((gclient_t *)0)->x)

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

extern	cvar_t	*deathmatch;																		//CW
extern	cvar_t	*maxentities;
extern	cvar_t	*dmflags;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
extern	cvar_t	*capturelimit;																		//ZOID++

extern	cvar_t	*password;
extern	cvar_t	*g_select_empty;
extern	cvar_t	*dedicated;

extern	cvar_t	*filterban;

extern	cvar_t	*sv_gravity;
extern	cvar_t	*sv_maxvelocity;

extern	cvar_t	*gun_x;
extern	cvar_t	*gun_y;
extern	cvar_t	*gun_z;
extern	cvar_t	*sv_rollspeed;
extern	cvar_t	*sv_rollangle;

extern	cvar_t	*run_pitch;
extern	cvar_t	*run_roll;
extern	cvar_t	*bob_up;
extern	cvar_t	*bob_pitch;
extern	cvar_t	*bob_roll;

extern	cvar_t	*sv_cheats;
extern	cvar_t	*maxclients;

extern	cvar_t	*flood_msgs;
extern	cvar_t	*flood_persecond;
extern	cvar_t	*flood_waitdelay;

//CW++
extern	cvar_t	*sv_gametype;
extern	cvar_t	*sv_respawn_invuln_time;
extern	cvar_t	*sv_rnd_powerup_timeout;
extern	cvar_t	*sv_rnd_mystery_ratio;
extern	cvar_t	*sv_show_leader;
extern	cvar_t	*sv_quad_factor;
extern	cvar_t	*sv_reject_blank_ip;
extern	cvar_t	*sv_map_random;

extern	cvar_t	*sv_op_password;
extern	cvar_t	*sv_op_ignore_maplist;

extern	cvar_t	*sv_reserved;
extern	cvar_t	*sv_rsv_password;

extern	cvar_t	*sv_map_file;
extern	cvar_t	*sv_config_file;
extern	cvar_t	*sv_motd_file;

extern	cvar_t	*sv_agm_drop;
extern	cvar_t	*sv_agm_drop_file;

extern	cvar_t	*sv_disc_drop;
extern	cvar_t	*sv_disc_drop_file;

extern	cvar_t	*competition;
extern	cvar_t	*matchstarttime;

extern	cvar_t	*sv_team1_name;
extern	cvar_t	*sv_team2_name;
extern	cvar_t	*sv_team1_model;
extern	cvar_t	*sv_team2_model;
extern	cvar_t	*sv_team1_skin;
extern	cvar_t	*sv_team2_skin;

extern	cvar_t	*sv_allow_bots;
extern	cvar_t	*sv_bots_camp;
extern	cvar_t	*sv_bots_insult;
extern	cvar_t	*sv_bots_taunt;
extern	cvar_t	*sv_bots_chat;
extern	cvar_t	*sv_bots_random;
extern	cvar_t	*sv_bots_use_file;
extern	cvar_t	*sv_bots_config_file;

extern	cvar_t	*chedit;

extern	cvar_t	*sv_allow_gausspistol;
extern	cvar_t	*sv_allow_mac10;
extern	cvar_t	*sv_allow_jackhammer;
extern	cvar_t	*sv_allow_traps;
extern	cvar_t	*sv_allow_c4;
extern	cvar_t	*sv_allow_spikegun;
extern	cvar_t	*sv_allow_rocketlauncher;
extern	cvar_t	*sv_allow_flamethrower;
extern	cvar_t	*sv_allow_railgun;
extern	cvar_t	*sv_allow_shockrifle;
extern	cvar_t	*sv_allow_agm;
extern	cvar_t	*sv_allow_disclauncher;
extern	cvar_t	*sv_allow_hook;

extern	cvar_t	*sv_allow_invuln;
extern	cvar_t	*sv_allow_quad;
extern	cvar_t	*sv_allow_siphon;
extern	cvar_t	*sv_allow_d89;
extern	cvar_t	*sv_allow_haste;
extern	cvar_t	*sv_allow_tele;
extern	cvar_t	*sv_allow_antibeam;
extern	cvar_t	*sv_allow_enviro;
extern	cvar_t	*sv_allow_silencer;
extern	cvar_t	*sv_allow_breather;

extern	cvar_t	*sv_allow_bullets;
extern	cvar_t	*sv_allow_shells;
extern	cvar_t	*sv_allow_rockets;
extern	cvar_t	*sv_allow_cells;
extern	cvar_t	*sv_allow_slugs;

extern	cvar_t	*sv_chainsaw_damage;

extern	cvar_t	*sv_deserteagle_damage;
extern	cvar_t	*sv_deserteagle_hspread;
extern	cvar_t	*sv_deserteagle_vspread;

extern	cvar_t	*sv_jackhammer_damage;
extern	cvar_t	*sv_jackhammer_hspread;
extern	cvar_t	*sv_jackhammer_vspread;

extern	cvar_t	*sv_mac10_damage;
extern	cvar_t	*sv_mac10_hspread;
extern	cvar_t	*sv_mac10_vspread;

extern	cvar_t	*sv_gauss_damage_base;
extern	cvar_t	*sv_gauss_damage_rate;
extern	cvar_t	*sv_gauss_damage_step;
extern	cvar_t	*sv_gauss_damage_max;
extern	cvar_t	*sv_gauss_damage_particle;
extern	cvar_t	*sv_gauss_scan_range;

extern	cvar_t	*sv_c4_damage;
extern	cvar_t	*sv_c4_radius;
extern	cvar_t	*sv_c4_touchbang;
extern	cvar_t	*sv_c4_proximity;
extern	cvar_t	*sv_c4_proximity_range;
extern	cvar_t	*sv_c4_proximity_delay;
extern	cvar_t	*sv_c4_proximity_life;
extern	cvar_t	*sv_c4_min_speed;
extern	cvar_t	*sv_c4_max_speed;
extern	cvar_t	*sv_c4_hold_accel;
extern	cvar_t	*sv_c4_timelimit;
extern	cvar_t	*sv_c4_spawn_range;

extern	cvar_t	*sv_trap_hook_damage;
extern	cvar_t	*sv_trap_hook_speed;
extern	cvar_t	*sv_trap_beam_damage;
extern	cvar_t	*sv_trap_beam_power;
extern	cvar_t	*sv_trap_proximity_range;
extern	cvar_t	*sv_trap_activate_delay;
extern	cvar_t	*sv_trap_min_speed;
extern	cvar_t	*sv_trap_max_speed;
extern	cvar_t	*sv_trap_hold_accel;
extern	cvar_t	*sv_trap_thru_tele;
extern	cvar_t	*sv_traps_max_active;
extern	cvar_t	*sv_traps_timelimit;

extern	cvar_t	*sv_spike_damage;
extern	cvar_t	*sv_spike_bang_damage;
extern	cvar_t	*sv_spike_bang_radius;
extern	cvar_t	*sv_spike_bang_delay;
extern	cvar_t	*sv_spike_speed;

extern	cvar_t	*sv_rocket_damage;
extern	cvar_t	*sv_rocket_radius;
extern	cvar_t	*sv_rocket_radius_damage;
extern	cvar_t	*sv_rocket_speed;
extern	cvar_t	*sv_rocket_kick_scale;

extern	cvar_t	*sv_flame_damage;
extern	cvar_t	*sv_flame_small_damage;
extern	cvar_t	*sv_flame_speed;
extern	cvar_t	*sv_firebomb_damage;
extern	cvar_t	*sv_firebomb_radius;
extern	cvar_t	*sv_firebomb_speed;
extern	cvar_t	*sv_flame_thru_tele;

extern	cvar_t	*sv_railgun_damage;

extern	cvar_t	*sv_shock_homing_damage;
extern	cvar_t	*sv_shock_homing_range;
extern	cvar_t	*sv_shock_speed;
extern	cvar_t	*sv_shock_homing_speed;
extern	cvar_t	*sv_shock_radius_damage;
extern	cvar_t	*sv_shock_radius;
extern	cvar_t	*sv_shock_freeze_time;
extern	cvar_t	*sv_shock_live_time;

extern	cvar_t	*sv_agm_mode;
extern	cvar_t	*sv_agm_charge_rate;
extern	cvar_t	*sv_agm_fire_rate;
extern	cvar_t	*sv_agm_shot_cells;
extern	cvar_t	*sv_agm_beam_cells;
extern	cvar_t	*sv_agm_fling_power;
extern	cvar_t	*sv_agm_cross_damage;
extern	cvar_t	*sv_agm_cross_timeout;
extern	cvar_t	*sv_agm_reflect_damage;
extern	cvar_t	*sv_agm_reflect_timeout;
extern	cvar_t	*sv_agm_invuln_cells;
extern	cvar_t	*sv_agm_disrupt_damage;
extern	cvar_t	*sv_agm_disrupt_siphon;

extern	cvar_t	*sv_disc_damage;
extern	cvar_t	*sv_disc_speed;
extern	cvar_t	*sv_disc_live_time;

extern	cvar_t	*sv_plasma_damage;
extern	cvar_t	*sv_plasma_speed;

extern	cvar_t	*sv_hook_damage;
extern	cvar_t	*sv_hook_speed;
extern	cvar_t	*sv_hook_pull_speed;
extern	cvar_t	*sv_hook_pull_agm;
extern	cvar_t	*sv_hook_offhand;

extern	cvar_t	*sv_health_initial;
extern	cvar_t	*sv_health_max;
extern	cvar_t	*sv_health_max_siphon;
extern	cvar_t	*sv_health_max_bonus;

extern	cvar_t	*sv_have_deserteagle;
extern	cvar_t	*sv_have_gausspistol;
extern	cvar_t	*sv_have_mac10;
extern	cvar_t	*sv_have_jackhammer;
extern	cvar_t	*sv_have_c4;
extern	cvar_t	*sv_have_traps;
extern	cvar_t	*sv_have_spikegun;
extern	cvar_t	*sv_have_rocketlauncher;
extern	cvar_t	*sv_have_flamethrower;
extern	cvar_t	*sv_have_railgun;
extern	cvar_t	*sv_have_shockrifle;
extern	cvar_t	*sv_have_agm;
extern	cvar_t	*sv_have_disclauncher;

extern	cvar_t	*sv_initial_weapon;

extern	cvar_t	*sv_initial_bullets;
extern	cvar_t	*sv_initial_shells;
extern	cvar_t	*sv_initial_c4;
extern	cvar_t	*sv_initial_traps;
extern	cvar_t	*sv_initial_rockets;
extern	cvar_t	*sv_initial_cells;
extern	cvar_t	*sv_initial_slugs;

extern	cvar_t	*sv_initial_armor;
extern	cvar_t	*sv_initial_armortype;

extern	cvar_t	*sv_max_bullets;
extern	cvar_t	*sv_max_shells;
extern	cvar_t	*sv_max_c4;
extern	cvar_t	*sv_max_traps;
extern	cvar_t	*sv_max_rockets;
extern	cvar_t	*sv_max_cells;
extern	cvar_t	*sv_max_slugs;

extern	cvar_t	*sv_max_band_bullets;
extern	cvar_t	*sv_max_band_shells;
extern	cvar_t	*sv_max_band_cells;
extern	cvar_t	*sv_max_band_slugs;

extern	cvar_t	*sv_max_pack_bullets;
extern	cvar_t	*sv_max_pack_shells;
extern	cvar_t	*sv_max_pack_c4;
extern	cvar_t	*sv_max_pack_traps;
extern	cvar_t	*sv_max_pack_rockets;
extern	cvar_t	*sv_max_pack_cells;
extern	cvar_t	*sv_max_pack_slugs;

extern	cvar_t	*sv_mystery_invuln;
extern	cvar_t	*sv_mystery_quad;
extern	cvar_t	*sv_mystery_d89;
extern	cvar_t	*sv_mystery_haste;
extern	cvar_t	*sv_mystery_siphon;
extern	cvar_t	*sv_mystery_antibeam;
extern	cvar_t	*sv_mystery_enviro;
extern	cvar_t	*sv_mystery_tele;

extern	cvar_t	*sv_vote_percentage;
extern	cvar_t	*sv_vote_time;
extern	cvar_t	*sv_vote_minclients;

extern	cvar_t	*sv_vote_allow_map;
extern	cvar_t	*sv_vote_allow_config;
extern	cvar_t	*sv_vote_allow_match;
extern	cvar_t	*sv_vote_allow_timelimit;
extern	cvar_t	*sv_vote_allow_fraglimit;
extern	cvar_t	*sv_vote_allow_capturelimit;
extern	cvar_t	*sv_vote_allow_hook;
extern	cvar_t	*sv_vote_allow_hook_offhand;

extern	cvar_t	*sv_vote_allow_bots;

extern	cvar_t	*sv_vote_allow_gausspistol;
extern	cvar_t	*sv_vote_allow_mac10;
extern	cvar_t	*sv_vote_allow_jackhammer;
extern	cvar_t	*sv_vote_allow_c4;
extern	cvar_t	*sv_vote_allow_traps;
extern	cvar_t	*sv_vote_allow_esg;
extern	cvar_t	*sv_vote_allow_rocketlauncher;
extern	cvar_t	*sv_vote_allow_flamethrower;
extern	cvar_t	*sv_vote_allow_railgun;
extern	cvar_t	*sv_vote_allow_shockrifle;
extern	cvar_t	*sv_vote_allow_agm;
extern	cvar_t	*sv_vote_allow_disclauncher;

extern	cvar_t	*sv_vote_allow_invuln;
extern	cvar_t	*sv_vote_allow_quad;
extern	cvar_t	*sv_vote_allow_siphon;
extern	cvar_t	*sv_vote_allow_d89;
extern	cvar_t	*sv_vote_allow_haste;
extern	cvar_t	*sv_vote_allow_tele;
extern	cvar_t	*sv_vote_allow_antibeam;
extern	cvar_t	*sv_vote_allow_enviro;
extern	cvar_t	*sv_vote_allow_silencer;
extern	cvar_t	*sv_vote_allow_breather;

extern	cvar_t	*sv_vote_allow_nohealth;
extern	cvar_t	*sv_vote_allow_noitems;
extern	cvar_t	*sv_vote_allow_weaponsstay;
extern	cvar_t	*sv_vote_allow_nofalling;
extern	cvar_t	*sv_vote_allow_noarmor;
extern	cvar_t	*sv_vote_allow_infiniteammo;
extern	cvar_t	*sv_vote_allow_quaddrop;
extern	cvar_t	*sv_vote_allow_fastswitch;
extern	cvar_t	*sv_vote_allow_extraitems;
extern	cvar_t	*sv_vote_allow_noreplacements;
extern	cvar_t	*sv_vote_allow_notech;
extern	cvar_t	*sv_vote_allow_ctfspawns;

extern	cvar_t	*sv_vote_timelimit_max;
extern	cvar_t	*sv_vote_timelimit_step;
extern	cvar_t	*sv_vote_fraglimit_max;
extern	cvar_t	*sv_vote_fraglimit_step;
extern	cvar_t	*sv_vote_capturelimit_max;
extern	cvar_t	*sv_vote_capturelimit_step;

extern	cvar_t	*footstep_sounds;	// Knightmare added
extern	cvar_t	*sv_step_fraction;	// Knightmare- this was a define in p_view.c

// Weapon flags for inventory.

#define WINV_DESERTEAGLE		0
#define WINV_GAUSSPISTOL		1
#define WINV_MAC10				2
#define WINV_JACKHAMMER			3
#define WINV_C4					4
#define WINV_TRAPS				5
#define WINV_ESG				6
#define WINV_ROCKETLAUNCHER		7
#define WINV_FLAMETHROWER		8
#define WINV_RAILGUN			9
#define WINV_SHOCKRIFLE			10
#define WINV_AGM				11
#define WINV_DISCLAUNCHER		12
#define WINV_CHAINSAW			13
//CW--

extern	edict_t	*g_edicts;

extern	cvar_t	*turn_rider;	// Knightmare added

#define world					(&g_edicts[0])

// item spawnflags
#define ITEM_TRIGGER_SPAWN		0x00000001
#define ITEM_NO_TOUCH			0x00000002

//CW++
#define ITEM_FLOATING			0x00000004		// all items
#define ITEM_FULLBRIGHT			0x00000008		// all items

#define SPAWNFLAG_NOMODEL		0x00000001		// teleporters
#define SPAWNFLAG_GT_COUNT		0x00000002		// Assault: defender spawn points

#define SPAWNFLAG_NOFOUNTAIN	0x00000002		// teleporters

#define SPAWNFLAG_FIRST			0x00000001		// info_mission
//CW--

// 6 bits reserved for editor flags
#define DROPPED_ITEM			0x00010000
#define	DROPPED_PLAYER_ITEM		0x00020000
#define ITEM_TARGETS_USED		0x00040000


// fields are needed for spawning from the entity string
// and saving / loading games
//
#define FFL_SPAWNTEMP		1

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
	F_IGNORE
} fieldtype_t;

typedef struct
{
	char		*name;
	size_t		ofs;	// Knightmare- was int
	fieldtype_t	type;
	int			flags;
} field_t;

// Lazarus: worldspawn effects
#define FX_WORLDSPAWN_NOHELP       1
#define FX_WORLDSPAWN_STEPSOUNDS   2
#define FX_WORLDSPAWN_WHATSIT      4
#define FX_WORLDSPAWN_ALERTSOUNDS  8
#define FX_WORLDSPAWN_CORPSEFADE  16
#define FX_WORLDSPAWN_JUMPKICK    32

extern	field_t fields[];
extern	gitem_t	itemlist[];

//CW++
// g_team.c
edict_t *FindTechSpawn(void);
void CTFReturnToMain(edict_t *ent, pmenuhnd_t *p);
void SetLevelName(pmenu_t *p);
void StuffCmd(edict_t *ent, char *s);
void StuffCmd_Ent(edict_t *self);
void ShowMOTD(edict_t *ent, pmenuhnd_t *p);
void Op_Boot(edict_t *ent, qboolean ban, int pnum, char *reason);
//CW--


//DH++
// g_camera.c
void use_camera(edict_t *ent, edict_t *other, edict_t *activator);
void camera_on(edict_t *ent);
void camera_off(edict_t *ent);
void faker_animate(edict_t *self);
edict_t *G_FindNextCamera (edict_t *camera, edict_t *monitor);
edict_t *G_FindPrevCamera (edict_t *camera, edict_t *monitor);
//DH--


// g_cmds.c
qboolean CheckFlood(edict_t *ent);
void Cmd_Score_f (edict_t *ent);

//CW++
void Cmd_Kill_f(edict_t *ent);
void Cmd_Vote_f(edict_t *ent, qboolean auto_win);
void Cmd_MatchStart_f(edict_t *ent);
void Cmd_Detonate_C4_f(edict_t *self);
void Op_SwapTeam(edict_t *ent, int pnum);
void Op_SayPrivs(edict_t *ent, qboolean mute, int pnum, char *reason);
//CW--


// g_items.c
// Knightmare added
#define QUAD_TIMEOUT_FRAMES			300
#define INVINCIBLE_TIMEOUT_FRAMES	300
#define BREATHER_TIMEOUT_FRAMES		300
#define ENVIROSUIT_TIMEOUT_FRAMES	300
#define SIPHON_TIMEOUT_FRAMES	300
#define NEEDLE_TIMEOUT_FRAMES	600
#define HASTE_TIMEOUT_FRAMES	300
#define ANTIBEAM_TIMEOUT_FRAMES	300
#define SILENCER_SHOTS			30
// end Knightmare

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
int GetMaxAmmoByIndex (gclient_t *client, int item_index); // Knightmare added
int GetMaxArmorByIndex (int item_index); // Knightmare added
qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count);
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void Use_Teleporter(edict_t *ent, gitem_t *item);													//CW++


// g_utils.c
qboolean KillBox (edict_t *ent);
void	G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
edict_t *G_Find (edict_t *from, size_t fieldofs, char *match);	// Knightmare- changed fieldofs from int
edict_t *FindRadius (edict_t *from, vec3_t org, float rad);
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
float	vectoyaw (vec3_t vec);
void	vectoangles (vec3_t vec, vec3_t angles);
qboolean infront(edict_t *self, edict_t *other);													//CW
qboolean visible(edict_t *self, edict_t *other);													//CW
// Knightmare added
void	vectoangles2 (vec3_t vec, vec3_t angles);
void GameDirRelativePath (const char *filename, char *output, size_t outputSize);
void SavegameDirRelativePath (const char *filename, char *output, size_t outputSize);
void CreatePath (const char *path);
// end Knightmare

//CW++
edict_t *FindClientRadius(edict_t *from, vec3_t org, float rad);
edict_t *FindPointRadius(edict_t *from, vec3_t org, float rad);
float VecRange(vec3_t start, vec3_t end);
void	TList_AddNode(edict_t *ent);
void	TList_DelNode(edict_t *ent);
void	PrintFFAScores(void);
void	PrintTeamScores(void);
char	*vtosf(vec3_t v);
void	dm(char *msg);
FILE*	OpenMaplistFile(qboolean report);
FILE*	OpenConfiglistFile(qboolean report);
FILE*	OpenAGMDropFile(qboolean report, qboolean readonly);
FILE*	OpenDiscLauncherDropFile(qboolean report, qboolean readonly);
FILE*	OpenBotConfigFile(qboolean report, qboolean readonly);
qboolean FileExists(char *filename, filetype_t ftype);
void	unicastSound(edict_t *player, int soundIndex, float volume);								//r1
//CW--

//DH++
edict_t	*LookingAt(edict_t *ent, int filter, vec3_t endpos, float *range);
void AnglesNormalize(vec3_t vec);
float SnapToEights(float x);
//DH--


// g_combat.c
qboolean CanDamage (edict_t *targ, edict_t *inflictor);
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker);
void T_Damage (edict_t *in_targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod);
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);

// damage flags
#define DAMAGE_RADIUS				0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR				0x00000002	// armour does not protect from this damage
#define DAMAGE_ENERGY				0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK			0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET				0x00000010  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION		0x00000020  // armor, shields, invulnerability, and godmode have no effect


// g_monster.c
void M_DropToFloor(edict_t *ent);
void M_CheckGround(edict_t *ent);

//CW++
qboolean M_CheckBottom(edict_t *ent);
qboolean M_WalkMove(edict_t *ent, float yaw, float dist);
void M_CatagorizePosition(edict_t *ent);
//CW--


// g_misc.c
void ThrowHead (edict_t *self, char *gibname, int damage, int type);
void ThrowClientHead (edict_t *self, int damage);
void ThrowGib (edict_t *self, char *gibname, int damage, int type, float livetime);
void BecomeExplosion1(edict_t *self);


//CW++
// g_menus.c
void OpenGameMenu(edict_t *self);
void PrintMOTD(edict_t *self);
void Cmd_VoteYes_f(edict_t *ent, qboolean op_forced);
void Cmd_VoteYes_f(edict_t *ent, qboolean op_forced);
void Cmd_VoteNo_f(edict_t *ent, qboolean op_forced);
void GMenu_VoteSettings(edict_t *ent, pmenuhnd_t *p);
void G_VoteWin(qboolean op_forced);
qboolean G_VoteBegin(edict_t *ent, vote_t type, char *msg);
qboolean JoinTeam(edict_t *ent, int desired_team, qboolean op_forced);
void OpenOpMenu(edict_t *ent);
void GMenu_Help(edict_t *ent, pmenuhnd_t *p);
//CW--


// g_weapon.c
void ThrowDebris(edict_t *self, char *modelname, float speed, vec3_t origin);
void Fire_Bullet(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void Fire_Shotgun(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void Fire_Blaster(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect);	//CW
void Fire_Rocket(edict_t *self, vec3_t start, vec3_t dir, int damage, float speed, float damage_radius, int radius_damage, qboolean guided);
void Fire_Rail(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);

//CW++
void Rocket_Touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void Spike_Explode(edict_t *self);
void Spike_MoveWithEnt(edict_t *self);
void C4_Explode(edict_t *self);
void C4_Die(edict_t *self);
void C4_DieFromDamage(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void Trap_Die(edict_t *self);
void Trap_DieFromDamage(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void Spawn_Flame(edict_t *self, edict_t *other, vec3_t start, int damage);
void Flame_Expire(edict_t *self);
void Fire_C4(edict_t *self, vec3_t start, vec3_t aimdir, int damage, float speed, float damage_radius, qboolean held);
void Fire_Trap(edict_t *self, vec3_t start, vec3_t aimdir, int hook_damage, float speed, int beam_damage, int beam_power, qboolean held);
void Fire_Spike(edict_t *self, vec3_t start, vec3_t aimdir, int damage, float speed, int kick, float damage_radius, int radius_damage);
void Fire_MultiSpikes(edict_t *self, vec3_t start, vec3_t aimdir, int damage, float speed, int kick, float damage_radius, int radius_damage, int count);
void Fire_Fireball(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int damage_smallflame, float speed, qboolean glow);
void Fire_Firebomb(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int damage_minor, int kick, float damage_radius, float speed);
void Fire_Shock(edict_t *self, vec3_t start, vec3_t aimdir, int damage_plasma, float speed, int kick, int damage_shockbolt, float damage_radius, qboolean homing);
void Fire_Particle(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
void Fire_Instabolt(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
qboolean Particle_Can_Hit(vec3_t start, vec3_t aimdir, edict_t *targ);
void Fire_Tracer(edict_t *self, vec3_t start, vec3_t aimdir, float speed, float lifetime);
void Fire_AGM(edict_t *self, vec3_t start, vec3_t aimdir, qboolean disrupt);
void Move_AGM(edict_t *self, vec3_t start, vec3_t aimdir);
void Fire_Disc(edict_t *self, vec3_t start, vec3_t aimdir, int damage, float speed, int kick);
void Fire_Chainsaw(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
void Fire_Plasma(edict_t *self, vec3_t start, vec3_t dir, int damage, float speed);
//CW--

//CW++
// p_weapon.c
void NoAmmoWeaponChange(edict_t *ent);
void Use_Weapon(edict_t *ent, gitem_t *item);
void PlayerNoise(edict_t *who, vec3_t where, int type);
//CW--


// g_client.c
void Respawn (edict_t *ent);
void BeginIntermission (edict_t *targ);
void PutClientInServer (edict_t *ent);
void InitClientPersistant (gclient_t *client);
void InitClientResp (gclient_t *client);
void InitBodyQue (void);
void ClientBeginServerFrame (edict_t *ent);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void body_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);		//CW++


// g_player.c
void player_pain (edict_t *self, edict_t *other, float kick, int damage);
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);


//CW++
// g_func.c
// Knightmare added
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

void plat_blocked(edict_t *self, edict_t *other);
void door_blocked(edict_t *self, edict_t *other);
void train_blocked(edict_t *self, edict_t *other);
void rotating_blocked(edict_t *self, edict_t *other);


// g_spawn.c
void SetupAGMSpawn(void);
void SetupDiscLauncherSpawn(void);
void SetupRndPowerupSpawn(void);
void RemoveDisabledWeapons(void);
void RemoveDisabledPowerups(void);
//CW--


// g_svcmds.c
void ServerCommand(void);


// p_view.c
void ClientEndServerFrame (edict_t *ent);


// p_hud.c
void MoveClientToIntermission (edict_t *client);
void G_SetStats (edict_t *ent);
void ValidateSelectedItem (edict_t *ent);
void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer);
void ShowGaussTarget(edict_t *ent, char *string, size_t stringSize);											//CW++


//CW++
// p_client.c
void SelectSpawnPoint(edict_t *ent, vec3_t origin, vec3_t angles);
edict_t *SelectDeathmatchSpawnPoint(void);
void ClearSlot(edict_t *ent);
void CopyToBodyQue(edict_t *ent);
//CW--


// g_pweapon.c
void P_ProjectSource(gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void Weapon_Generic(edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));


// g_phys.c
void G_RunEntity(edict_t *ent);


// g_main.c
void SaveClientData(void);
void FetchClientEntData(edict_t *ent);
void EndDMLevel(void);

// g_svcmds.c
qboolean SV_FilterPacket(char *from);


//DH++
// g_turret.c
void turret_breach_fire(edict_t *ent);
void turret_disengage(edict_t *ent);
//DH--

//============================================================================

// client_t->anim_priority
#define	ANIM_BASIC		0			// stand / run
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
	char		netname[MAX_NAMELEN];																//CW
	int			hand;

	qboolean	connected;			// a loadgame will leave valid entities that just don't have a connection yet

	// values saved and restored from edicts when changing levels
	int			health;
	int			max_health;
	int			savedFlags;

	int			selected_item;
	int			inventory[MAX_ITEMS];

	// ammo capacities
	int			max_bullets;
	int			max_shells;
	int			max_rockets;
	int			max_c4;
	int			max_cells;
	int			max_slugs;

//CW++
	int			max_traps;

	int			weap_note;

	char		old_name[MAX_NAMELEN];																//CW
	char		ip[22];
	
	qboolean	op_status;
	qboolean	muted;
//CW--

//Maj++
	int			botindex;
	int			routeindex;
//Maj--

	gitem_t		*weapon;
	gitem_t		*lastweapon;																		//CW
} client_persistant_t;

// client data that stays across deathmatch respawns
typedef struct
{																									//CW
	int			enterframe;				// level.framenum the client entered the game
	int			score;

//ZOID++
	int			ctf_team;
	int			ctf_state;
	float		ctf_lasthurtcarrier;
	float		ctf_lastreturnedflag;
	float		ctf_flagsince;
	float		ctf_lastfraggedcarrier;
	qboolean	id_state;
	float		lastidtime;
	qboolean	voted;
	qboolean	ready;

	struct ghost_s *ghost;				// for ghost codes
//ZOID--

	vec3_t		cmd_angles;				// angles sent over in the last command
	int			game_helpchanged;
	int			helpchanged;

//CW++
	int			nodes_active;			// number of C4/Trap nodes that a player has active
	qboolean	aliases_set;
	qboolean	id_trap;				// ID for Traps & C4s
	float		id_trap_time;
//CW--
} client_respawn_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
struct gclient_s
{
	// known to server
	player_state_t	ps;					// communicated by server to clients
	int				ping;

	// private to game
	client_persistant_t	pers;
	client_respawn_t	resp;
	pmove_state_t		old_pmove;		// for detecting out-of-pmove changes

	qboolean	showscores;				// set layout stat

//ZOID++
	qboolean	inmenu;					// in menu
	pmenuhnd_t	*menu;					// current menu
//ZOID--

	qboolean	showinventory;			// set layout stat
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
	int			damage_armor;			// damage absorbed by armor
	int			damage_parmor;			// damage absorbed by power armor
	int			damage_blood;			// damage taken out of health
	int			damage_knockback;		// impact damage
	vec3_t		damage_from;			// origin for vector calculation

	float		killer_yaw;				// when dead, look at killer

	weaponstate_t	weaponstate;
	vec3_t		kick_angles;			// weapon kicks
	vec3_t		kick_origin;
	float		v_dmg_roll;				// damage kicks
	float		v_dmg_pitch;			//
	float		v_dmg_time;				//

	float		fall_time;				// for view drop on fall
	float		fall_value;				//

	float		damage_alpha;
	float		bonus_alpha;
	vec3_t		damage_blend;

	vec3_t		v_angle;				// aiming direction
	float		bobtime;				// so off-ground doesn't change it
	vec3_t		oldviewangles;
	vec3_t		oldvelocity;

	float		next_drown_time;
	int			old_waterlevel;
	int			breather_sound;

	int			machinegun_shots;		// for weapon raising

	qboolean    backpedaling;  //<- CDawg added this

	// animation vars
	int			anim_end;
	int			anim_priority;
	qboolean	anim_duck;
	qboolean	anim_run;

	// powerup timers
	int			quad_framenum;																		//CW
	int			invincible_framenum;																//CW
	int			breather_framenum;																	//CW
	int			enviro_framenum;																	//CW

	qboolean	grenade_blew_up;
	float		grenade_time;
	int			silencer_shots;
	int			weapon_sound;

	float		pickup_msg_time;

	float		flood_locktill;			// locked from talking
	float		flood_when[10];			// when messages were said
	int			flood_whenhead;			// head pointer for when said

	float		respawn_time;			// can respawn when time > this

//ZOID++
	void		*ctf_grapple;			// entity of grapple
	int			ctf_grapplestate;		// true if pulling
	float		ctf_grapplereleasetime;	// time of grapple release
	float		ctf_regentime;			// regen tech
	float		ctf_techsndtime;
	float		ctf_lasttechmsg;
	edict_t		*chase_target;
	qboolean	update_chase;
	float		menutime;				// time to update menu
	qboolean	menudirty;
//ZOID--

//CW++
	int			chase_mode;
	qboolean	spectator;

	qboolean	op_override;

	qboolean	hook_on;
	weaponstate_t hookstate;

	qboolean	mod_changeteam;
	qboolean	gauss_particle;			// Gauss Pistol mode
	qboolean	multi_spike;			// Multiple-spike-spread mode
	qboolean	ft_firebomb;			// Flamethrower mode
	qboolean	normal_rockets;			// Rocket Launcher mode
	qboolean	homing_plasma;			// Shock Rifle mode
	qboolean	agm_disrupt;			// AGM cellular-disruption mode

	qboolean	show_gausscharge;
	int			show_gausstarget;		// bit 0 => on/off; bit 1 => temporarily deactivated
	int			gauss_framenum;
	int			gauss_dmg;

	int			antibeam_framenum;
	int			frozen_framenum;
	int			siphon_framenum;
	int			needle_framenum;
	int			haste_framenum;

	edict_t		*agm_target;
	float		agm_range;
	int			agm_charge;
	qboolean	agm_on;
	qboolean	agm_push;
	qboolean	agm_pull;
	qboolean	agm_showcharge;
	qboolean	agm_tripped;

	edict_t		*agm_enemy;
	qboolean	held_by_agm;
	qboolean	flung_by_agm;			// player used "fling" command
	qboolean	thrown_by_agm;			// victim given high velocity by an AGM, but not via "fling" cmd

	float		c4_boom_time;			// time until C4 detonates in player's hand
//CW--

//DH++
	int			use;					// indicates whether +use key is pressed

	edict_t     *spycam;				// security camera stuff
	edict_t		*monitor;				//
	edict_t		*camplayer;				//
	vec3_t		org_viewangles;			//
	short		old_owner_angles[2];	//

	usercmd_t	ucmd;					// copied for convenience in ClientThink()
//DH--

//Maj++
	vec3_t		my_old_origin;			//bot's old origin

	float		chattime;				//time of next chat
	float		insulttime;				//time of next insult
	float		taunttime;				//time of next taunting;
	float		duckedtime;				//next ducked time

	int			movestate;				//movement state
	int			combatstate;			//combat state
	int			waterstate;				//current water state (0..2)

	qboolean	havetarget;				//got a target? (yes/no)
	edict_t		*current_enemy;			//current enemy target
	edict_t		*prev_enemy;			//old enemy
	int			enemysearchcnt;			//enemy search count

	int			enemy_routeindex;		//index of target
	float		targetlock;				//enemy locking time
	float		nextcheck;				//checking time
	vec3_t		targ_old_origin;		//target's old origin

	edict_t		*waiting_obj;			//ent bot is waiting on (elevators, etc)
	vec3_t		movtarget_pt;			//moving target waiting point
	qboolean	objshot;				//button has been activated

	int			ground_contents;		//contents under bot
	float		ground_slope;			//slope under bot

	qboolean	routetrace;				//
	int			routeindex;				//route[] index
	float		routelocktime;			//route[] locking time
	float		routereleasetime;		//route[] release time

	int			battlemode;				//current battle mode - see FIRE_*
	int			battlecount;			//temp battle count
	int			battlesubcnt;			//subcount
	int			battleduckcnt;			//ducktime during battle

	float		moveyaw;				//temp moving yaw
	vec3_t		vtemp;					//temp storage vec

	float		camptime;				//time spent camping
	gitem_t		*campitem;				//item camping near
	vec3_t		lastorigin;				//origin of camping spot
	qboolean	camping;				//TRUE => currently camping									//CW++
//Maj--
};


struct edict_s
{
	entity_state_t		s;
	struct gclient_s	*client;		// NULL if not a player.
										// The server expects the first part of gclient_s to be a player_state_t
										// but the rest of it is opaque.

	qboolean	inuse;
	int			linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t		area;					// linked to a division node or leaf
	
	int			num_clusters;			// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			headnode;				// unused if num_clusters != -1
	int			areanum, areanum2;

	//================================

	int			svflags;
	vec3_t		mins;
	vec3_t		maxs;
	vec3_t		absmin;
	vec3_t		absmax;
	vec3_t		size;
	solid_t		solid;
	int			clipmask;
	edict_t		*owner;


	// DO NOT MODIFY ANYTHING ABOVE THIS -- THE SERVER EXPECTS THE FIELDS IN THAT ORDER!

	//================================
	int			movetype;
	int			flags;

	char		*model;
	float		freetime;				// sv.time when the object was freed
	
	//
	// only used locally in game, not by server
	//
	char		*message;
	char		*classname;
	int			spawnflags;

	float		timestamp;

	float		angle;					// set in qe3, -1 = up, -2 = down
	char		*target;
	char		*targetname;
	char		*killtarget;
	char		*team;
	char		*pathtarget;
	char		*deathtarget;
	char		*combattarget;
	edict_t		*target_ent;

	float		speed;
	float		accel;
	float		decel;
	vec3_t		movedir;
	vec3_t		pos1;
	vec3_t		pos2;

	vec3_t		velocity;
	vec3_t		avelocity;
	int			mass;
	float		air_finished;
	float		gravity;			// per entity gravity multiplier (1.0 is normal; use for lowgrav artifact, flares)

	edict_t		*goalentity;
	edict_t		*movetarget;
	float		yaw_speed;
	float		ideal_yaw;

	float		nextthink;
	void		(*prethink)(edict_t *ent);
	void		(*think)(edict_t *self);
	void		(*blocked)(edict_t *self, edict_t *other);
	void		(*touch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
	void		(*use)(edict_t *self, edict_t *other, edict_t *activator);
	void		(*pain)(edict_t *self, edict_t *other, float kick, int damage);
	void		(*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

	float		touch_debounce_time;
	float		pain_debounce_time;
	float		damage_debounce_time;
	float		fly_sound_debounce_time;	//FIXME move to clientinfo
	float		last_move_time;

	int			health;
	int			max_health;
	int			gib_health;
	int			deadflag;
	qboolean	show_hostile;

	float		powerarmor_time;

	char		*map;					// target_changelevel

	int			viewheight;				// height above origin where eyesight is determined
	int			takedamage;
	int			dmg;					// direct damage
	int			radius_dmg;				// splash damage
	float		dmg_radius;				// radius of splash damage
	int			sounds;					// FIXME make this a spawntemp var?
	int			count;

	edict_t		*chain;
	edict_t		*enemy;
	edict_t		*oldenemy;
	edict_t		*activator;
	edict_t		*groundentity;
	int			groundentity_linkcount;
	edict_t		*teamchain;
	edict_t		*teammaster;

	edict_t		*mynoise;				//FIXME can go in client only
	edict_t		*mynoise2;

	int			noise_index;
	int			noise_index2;
	float		volume;
	float		attenuation;

	// timing variables
	float		wait;
	float		delay;					// before firing targets
	float		random;

	float		teleport_time;

	int			watertype;
	int			waterlevel;

	vec3_t		move_origin;
	vec3_t		move_angles;

	// move this to clientinfo?
	int			light_level;

	int			style;					// also used as areaportal number

	gitem_t		*item;					// for bonus items

	// common data blocks
	moveinfo_t		moveinfo;
	monsterinfo_t	monsterinfo;

//CW++
	edict_t		*next_node;				// next C4/Trap node in linked list
	edict_t		*prev_node;				// previous C4/Trap node in linked list
	
	qboolean	tractored;				// we've been killed by a tractor-beam trap
	qboolean	disintegrated;			// we've been disintegrated by a shockbolt
	qboolean	burning;				// we're on fire
	edict_t		*flame;					// flame entity that's attached to us

	qboolean	wep_proj;				// we're a weapon projectile
	qboolean	rnd_spawn;				// we've been spawned randomly (so don't respawn ... usually a powerup)

	char		*message2;				// extra string
	char		*message3;				// extra string
//CW--

//DH++
	edict_t		*turret;				// player-controlled turret
	char		*followtarget;			// tracking turret target
	char		*destroytarget;
	char		*viewmessage;
	vec_t		base_radius;			// used to project "viewpoint" of TRACK turret out past base

	edict_t		*viewer;				// spycam

	char		*usermodel;				// for model spawner
	int			startframe;
	int			framenumbers;
	int			solidstate;
	vec3_t		bleft;					// bounding box data
	vec3_t		tright;					// bounding box data
	int			renderfx;
	int         effects;

	// Lazarus: for rotating brush models:
	float		pitch_speed;
	float		roll_speed;
	float		ideal_pitch;
	float		ideal_roll;
	float		roll;

	// spline train
	edict_t		*from;
	edict_t		*to;
	// if true, brush models will move directly to Move_Done
	// at Move_Final rather than slowing down.
	qboolean	smooth_movement;
	int			turn_rider;
	vec3_t      origin_offset;  // used to help locate brush models w/o origin brush
//DH--

//Maj++
	qboolean	isabot;
	edict_t		*union_ent;
	edict_t		*trainteam;
//Maj--

	char		*musictrack;	// Knightmare- for specifying OGG or CD track

//=========
//ROGUE
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
//ROGUE
//=========
};

//DH++
#define	LOOKAT_NOBRUSHMODELS  1
#define LOOKAT_NOWORLD        2
#define LOOKAT_MD2			  (LOOKAT_NOBRUSHMODELS | LOOKAT_NOWORLD)
//DH--


#include "g_bot.h"																					// Maj++

//CW++
extern int NumBotsInGame;
extern botinfo_t Bot[MAXBOTS+1];

// Things which affect weapon behaviour, but which we don't want to make into cvars.

#define TELEFRAG_DIST				20

#define RANGE_CHAINSAW				48

#define DEFAULT_JACKHAMMER_COUNT	6

#define DEFAULT_MGTURRET_HSPREAD	500
#define DEFAULT_MGTURRET_VSPREAD	500

#define GAUSS_AIMCAL				100

#define TRAP_ID_RANGE				500
#define TRAP_ID_DOTPRODUCT			0.995
#define TRAP_ID_CHECKTIME			0.3

#define	SPIKE_SPREAD				800
#define	SPIKE_SPREAD_HALF			400

#define AGM_RANGE_DELTA				50
#define AGM_RANGE_MIN				50
#define AGM_RANGE_MAX				8000
#define AGM_MOVE_SCALE				10.0

#define TRAP_DMG_RATE				1.0

#define FT_IGNITE_RADIUS			40.0
#define	FT_SMALLFLAME_PROB			0.1
#define	FT_IGNITE_PROB				0.5
#define	FT_SMALL_IGNITE_PROB		0.2
#define FLAME_DMG_RATE				1.0
#define FLAME_EXPIRE_PROB			0.05

#define FLAME_LIVETIME_0			2.0
#define FLAME_LIVETIME_R			5.0
#define SPIKE_LIVETIME				3.0
#define PLASMA_LIVETIME				5.0	
#define	BLASTER_LIVETIME			2.0

#define FT_CELLS_PER_SHOT			10
#define SR_CELLS_PER_SHOT			10
#define GP_SLUGS_PER_SHOT			5
#define GP_CELLS_PER_SHOT			25

#include "g_team.h"
//CW--
