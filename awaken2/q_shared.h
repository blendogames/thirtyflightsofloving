// q_shared.h -- included first by ALL program modules

#ifdef _WIN32
	// unknown pragmas are SUPPOSED to be ignored, but....
	#pragma warning(disable : 4244)     		// MIPS
	#pragma warning(disable : 4136)     		// X86
	#pragma warning(disable : 4051)     		// ALPHA

	#pragma warning(disable : 4018)     		// signed/unsigned mismatch
	#pragma warning(disable : 4305)				// truncation from const double to float

//r1++ Enabled some more warnings.
	//useful warnings
	#pragma warning(2 : 4189 4210 4389)

	//"off by default" warnings
	#pragma warning(3 : 4056)
	#pragma warning(3 : 4191)
	#pragma warning(3 : 4254)
	#pragma warning(3 : 4287)
	#pragma warning(3 : 4296)
	#pragma warning(3 : 4514)
	#pragma warning(3 : 4545)
	#pragma warning(3 : 4546)
	#pragma warning(3 : 4547)
	#pragma warning(3 : 4548)
	#pragma warning(3 : 4549)
	#pragma warning(3 : 4555)
	#pragma warning(3 : 4905)
	#pragma warning(3 : 4906)
	#pragma warning(3 : 4245)
//r1--

#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#if (defined _M_IX86 || defined __i386__) && !defined C_ONLY && !defined __sun__
	#define id386	1
#else
	#define id386	0
#endif

#if defined _M_ALPHA && !defined C_ONLY
	#define idaxp	1
#else
	#define idaxp	0
#endif

typedef unsigned char 		byte;
typedef enum {false, true}	qboolean;


#ifndef NULL
	#define NULL ((void *)0)
#endif

#ifndef min
#define min(a,b)        (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)        (((a) > (b)) ? (a) : (b))
#endif

// from Quake3 source
#ifdef _MSC_VER	// _WIN32
//#define Q_vsnprintf _vsnprintf
__inline int Q_vsnprintf (char *Dest, size_t Count, const char *Format, va_list Args) {
	int ret = _vsnprintf(Dest, Count, Format, Args);
	Dest[Count-1] = 0;	// null terminate
	return ret;
}
#else
// TODO: do we need Mac define?
#define Q_vsnprintf vsnprintf
#endif

// Knightmare- whether to include new engine enhancements
#define	KMQUAKE2_ENGINE_MOD

#ifdef KMQUAKE2_ENGINE_MOD
// 32-bit pmove coords when using custom engine
#define LARGE_MAP_SIZE
// looping of attenuated sounds
#define LOOP_SOUND_ATTENUATION
#endif

// angle indexes
#define	PITCH				0				// up / down
#define	YAW					1				// left / right
#define	ROLL				2				// fall over

#define	MAX_STRING_CHARS	1024			// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	80				// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		128				// max length of an individual token

#define	MAX_QPATH			64				// max length of a quake game pathname
#define	MAX_OSPATH			128				// max length of a filesystem pathname

//CW++
#define MAX_MAPS			1000			// max number of maps in file
#define MAX_CONFIGS			100				// max number of configs in file
#define MAX_OLDMAPS			10				// max number of previously-played maps to store

#define MAX_NAMELEN			16				// max length of player names
#define MAX_SKINLEN			64				// max length of skin names

#define MENU_MSGLEN			128				// length of menu messages
#define VOTE_MSGLEN			128				// length of vote messages
//CW--

//
// per-level limits
//
#define	MAX_CLIENTS			256				// absolute limit
#ifdef KMQUAKE2_ENGINE_MOD		// Knightmare- increase MAX_EDICTS
#define	MAX_EDICTS			8192	// must change protocol to increase more
#else
#define	MAX_EDICTS			1024	// must change protocol to increase more
#endif
#define	MAX_LIGHTSTYLES		256

#ifdef KMQUAKE2_ENGINE_MOD		//Knightmare- Ding-Dong, Index: Overflow is dead!
#define	MAX_MODELS			8192	// these are sent over the net as shorts
#define	MAX_SOUNDS			8192	// so they cannot exceed 64K
#define	MAX_IMAGES			2048
#else
#define	MAX_MODELS			256	// these are sent over the net as bytes
#define	MAX_SOUNDS			256	// so they cannot be blindly increased
#define	MAX_IMAGES			256
#endif

// Knightmare- hacked offsets for old demos
#define	OLD_MAX_MODELS		256
#define	OLD_MAX_SOUNDS		256
#define	OLD_MAX_IMAGES		256
// end Knightmare

#define	MAX_ITEMS			256
#define MAX_GENERAL			(MAX_CLIENTS*2)	// general config strings

// Knightmare- world size
#ifdef KMQUAKE2_ENGINE_MOD
#define MAX_WORLD_COORD		(16384)
#define MIN_WORLD_COORD		(-16384)
#else
#define MAX_WORLD_COORD		(4096)
#define MIN_WORLD_COORD		(-4096)
#endif
#define WORLD_SIZE			(MAX_WORLD_COORD - MIN_WORLD_COORD)
// end Knightmare


// game print flags
#define	PRINT_LOW			0				// pickup messages
#define	PRINT_MEDIUM		1				// death messages
#define	PRINT_HIGH			2				// critical messages
#define	PRINT_CHAT			3				// chat messages

#define	ERR_FATAL			0				// exit the entire game with a popup window
#define	ERR_DROP			1				// print to console and disconnect from game
#define	ERR_DISCONNECT		2				// don't kill server

#define	PRINT_ALL			0
#define PRINT_DEVELOPER		1				// only print when "developer 1"
#define PRINT_ALERT			2		


// destination class for gi.multicast()
typedef enum
{
MULTICAST_ALL,
MULTICAST_PHS,
MULTICAST_PVS,
MULTICAST_ALL_R,
MULTICAST_PHS_R,
MULTICAST_PVS_R
} multicast_t;


/*
==============================================================

MATHLIB

==============================================================
*/

typedef float vec_t;
typedef vec_t vec3_t[3];
typedef vec_t vec5_t[5];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

#ifndef M_PI
#define M_PI		3.14159265358979323846				// matches value in gcc v2 math.h
#endif

//CW++
#define DEG2RAD( a ) ( a * 0.01745329251994329577 )
#define RAD2DEG( a ) ( a * 57.29577951308232087680 )
#define INVROOT2 0.70710678118654752440
//CW--

struct cplane_s;

extern vec3_t vec3_origin;
//CW++
extern vec3_t vec3_up;
extern vec3_t vec3_dn;
extern vec3_t vec3_border;
//CW--

#define	nanmask (255<<23)

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

// microsoft's fabs seems to be ungodly slow...
//float Q_fabs (float f);
//#define	fabs(f) Q_fabs(f)
#if !defined C_ONLY && !defined __linux__ && !defined __sgi
extern long Q_ftol( float f );
#else
#define Q_ftol( f ) ( long ) (f)
#endif

#define DotProduct(x,y)			(x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define VectorSubtract(a,b,c)	(c[0]=a[0]-b[0],c[1]=a[1]-b[1],c[2]=a[2]-b[2])
#define VectorAdd(a,b,c)		(c[0]=a[0]+b[0],c[1]=a[1]+b[1],c[2]=a[2]+b[2])
#define VectorCopy(a,b)			(b[0]=a[0],b[1]=a[1],b[2]=a[2])
#define VectorClear(a)			(a[0]=a[1]=a[2]=0)
#define VectorNegate(a,b)		(b[0]=-a[0],b[1]=-a[1],b[2]=-a[2])
#define VectorSet(v, x, y, z)	(v[0]=(x), v[1]=(y), v[2]=(z))

void VectorMA(vec3_t veca, float scale, vec3_t vecb, vec3_t vecc);

// just in case you do't want to use the macros
vec_t _DotProduct(vec3_t v1, vec3_t v2);
void _VectorSubtract(vec3_t veca, vec3_t vecb, vec3_t out);
void _VectorAdd(vec3_t veca, vec3_t vecb, vec3_t out);
void _VectorCopy(vec3_t in, vec3_t out);

void ClearBounds(vec3_t mins, vec3_t maxs);
void AddPointToBounds(vec3_t v, vec3_t mins, vec3_t maxs);
int VectorCompare(vec3_t v1, vec3_t v2);
vec_t VectorLength(vec3_t v);
void CrossProduct(vec3_t v1, vec3_t v2, vec3_t cross);
vec_t VectorNormalize(vec3_t v);
vec_t VectorNormalize2(vec3_t v, vec3_t out);
void VectorInverse(vec3_t v);
void VectorScale(vec3_t in, vec_t scale, vec3_t out);
int Q_log2(int val);

void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3]);
void R_ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4]);

void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *plane);
float	anglemod(float a);
float LerpAngle (float a1, float a2, float frac);

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))

void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal );
void PerpendicularVector( vec3_t dst, const vec3_t src );
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );


//=============================================

char *COM_SkipPath (char *pathname);
void COM_StripExtension (char *in, char *out, size_t outSize);
void COM_FileBase (char *in, char *out, size_t outSize);
void COM_FilePath (char *in, char *out, size_t outSize);
void COM_DefaultExtension (char *path, size_t pathSize, char *extension);

char *COM_Parse (char **data_p);
// data is an in/out parm, returns a parsed out token

void Com_sprintf (char *dest, size_t size, char *fmt, ...);
// Knightmare added
size_t Com_strcpy (char *dest, size_t destSize, const char *src);
size_t Com_strcat (char *dest, size_t destSize, const char *src);

void Com_PageInMemory (byte *buffer, int size);

//=============================================

// portable case insensitive compare
int Q_stricmp (char *s1, char *s2);
int Q_strcasecmp (char *s1, char *s2);
int Q_strncasecmp (char *s1, char *s2, int n);

//=============================================

short	BigShort(short l);
short	LittleShort(short l);
int		BigLong (int l);
int		LittleLong (int l);
float	BigFloat (float l);
float	LittleFloat (float l);

void	Swap_Init (void);
char	*va(char *format, ...);

//=============================================

//
// key / value info strings
//
#define	MAX_INFO_KEY		64
#define	MAX_INFO_VALUE		64
#define	MAX_INFO_STRING		512

char *Info_ValueForKey (char *s, char *key);
void Info_RemoveKey (char *s, char *key);
void Info_SetValueForKey (char *s, char *key, char *value);
qboolean Info_Validate (char *s);

/*
==============================================================

SYSTEM SPECIFIC

==============================================================
*/

extern	int	curtime;		// time returned by last Sys_Milliseconds

int		Sys_Milliseconds(void);
void	Sys_Mkdir(char *path);

// large block stack allocation routines
void	*Hunk_Begin (int maxsize);
void	*Hunk_Alloc (int size);
void	Hunk_Free (void *buf);
int		Hunk_End (void);

// directory searching
#define SFF_ARCH    0x01
#define SFF_HIDDEN  0x02
#define SFF_RDONLY  0x04
#define SFF_SUBDIR  0x08
#define SFF_SYSTEM  0x10


// pass in an attribute mask of things you wish to REJECT
char *Sys_FindFirst (char *path, unsigned musthave, unsigned canthave );
char *Sys_FindNext ( unsigned musthave, unsigned canthave );
void Sys_FindClose (void);


// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...);
void Com_Printf (char *msg, ...);


/*
==========================================================

CVARS (console variables)

==========================================================
*/

#ifndef CVAR
#define	CVAR

#define	CVAR_ARCHIVE	1			// set to cause it to be saved to vars.rc
#define	CVAR_USERINFO	2			// added to userinfo when changed
#define	CVAR_SERVERINFO	4			// added to serverinfo when changed
#define	CVAR_NOSET		8			// don't allow change from console at all, but can be set from the command line
#define	CVAR_LATCH		16			// save changes until server restart

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s
{
	char		*name;
	char		*string;
	char		*latched_string;			// for CVAR_LATCH vars
	int			flags;
	qboolean	modified;					// set each time the cvar is changed
	float		value;
	struct cvar_s *next;
	// Knightmare- added cvar defaults
#ifdef KMQUAKE2_ENGINE_MOD
	char		*default_string;
	int			integer;
#endif
} cvar_t;

#endif		// CVAR

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

// lower bits are stronger, and will eat weaker brushes completely
#define	CONTENTS_SOLID			1			// an eye is never valid in a solid
#define	CONTENTS_WINDOW			2			// translucent, but not watery
#define	CONTENTS_AUX			4
#define	CONTENTS_LAVA			8
#define	CONTENTS_SLIME			16
#define	CONTENTS_WATER			32
#define	CONTENTS_MIST			64
#define	LAST_VISIBLE_CONTENTS	64
#define CONTENTS_MUD            128    // not a "real" content property - used only for watertype

// remaining contents are non-visible, and don't eat brushes

#define	CONTENTS_AREAPORTAL		0x8000

#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000

// currents can be added to any other contents, and may be mixed
#define	CONTENTS_CURRENT_0		0x40000
#define	CONTENTS_CURRENT_90		0x80000
#define	CONTENTS_CURRENT_180	0x100000
#define	CONTENTS_CURRENT_270	0x200000
#define	CONTENTS_CURRENT_UP		0x400000
#define	CONTENTS_CURRENT_DOWN	0x800000

#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity

#define	CONTENTS_MONSTER		0x2000000	// should never be on a brush, only in game
#define	CONTENTS_DEADMONSTER	0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes to be added after vis leafs
#define	CONTENTS_TRANSLUCENT	0x10000000	// auto set if any surface has trans
#define	CONTENTS_LADDER			0x20000000



#define	SURF_LIGHT		0x1					// value will hold the light strength
#define	SURF_SLICK		0x2					// effects game physics
#define	SURF_SKY		0x4					// don't draw, but add to skybox
#define	SURF_WARP		0x8					// turbulent water warp
#define	SURF_TRANS33	0x10
#define	SURF_TRANS66	0x20
#define	SURF_FLOWING	0x40				// scroll towards angle
#define	SURF_NODRAW		0x80				// don't bother referencing the texture



// content masks
#define	MASK_ALL				(-1)
#define	MASK_SOLID				(CONTENTS_SOLID|CONTENTS_WINDOW)
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define	MASK_DEADSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW)
#define	MASK_MONSTERSOLID		(CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEADMONSTER)
#define MASK_CURRENT			(CONTENTS_CURRENT_0|CONTENTS_CURRENT_90|CONTENTS_CURRENT_180|CONTENTS_CURRENT_270|CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN)

//Maj++
#define MASK_BOTSOLID			(CONTENTS_SOLID|CONTENTS_LADDER|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define MASK_BOTSOLIDX			(CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTER)
//Maj--


// gi.BoxEdicts() can return a list of either solid or trigger entities
// FIXME: eliminate AREA_ distinction?
#define	AREA_SOLID		1
#define	AREA_TRIGGERS	2


// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
typedef struct cplane_s
{
	vec3_t	normal;
	float	dist;
	byte	type;							// for fast side tests
	byte	signbits;						// signx + (signy<<1) + (signz<<1)
	byte	pad[2];
} cplane_t;

// structure offset for asm code
#define CPLANE_NORMAL_X			0
#define CPLANE_NORMAL_Y			4
#define CPLANE_NORMAL_Z			8
#define CPLANE_DIST				12
#define CPLANE_TYPE				16
#define CPLANE_SIGNBITS			17
#define CPLANE_PAD0				18
#define CPLANE_PAD1				19

typedef struct cmodel_s
{
	vec3_t		mins;
	vec3_t		maxs;
	vec3_t		origin;						// for sounds or lights
	int			headnode;
} cmodel_t;

typedef struct csurface_s
{
	char		name[16];
	int			flags;
	int			value;
} csurface_t;

//ZOID++
typedef struct mapsurface_s					// used internally due to name len probs
{
	csurface_t	c;
	char		rname[32];
} mapsurface_t;
//ZOID--

// a trace is returned when a box is swept through the world
typedef struct
{
	qboolean		allsolid;				// if true, plane is not valid
	qboolean		startsolid;				// if true, the initial point was in a solid area
	float			fraction;				// time completed, 1.0 = didn't hit anything
	vec3_t			endpos;					// final position
	cplane_t		plane;					// surface normal at impact
	csurface_t		*surface;				// surface hit
	int				contents;				// contents on other side of surface hit
	struct edict_s	*ent;					// not set by CM_*() functions
} trace_t;



// pmove_state_t is the information necessary for client side movement
// prediction
typedef enum 
{
	// can accelerate and turn
	PM_NORMAL,
	PM_SPECTATOR,
	// no acceleration or turning
	PM_DEAD,
	PM_GIB,			// different bounding box
	PM_FREEZE
} pmtype_t;

// pmove->pm_flags
#define	PMF_DUCKED			1
#define	PMF_JUMP_HELD		2
#define	PMF_ON_GROUND		4
#define	PMF_TIME_WATERJUMP	8				// pm_time is waterjump
#define	PMF_TIME_LAND		16				// pm_time is time before rejump
#define	PMF_TIME_TELEPORT	32				// pm_time is non-moving time
#define PMF_NO_PREDICTION	64				// temporarily disables prediction (used for grappling hook)

// This structure needs to be communicated bit-accurate from the server to the client to guarantee that
// prediction stays in sync, so no floats are used. If any part of the game code modifies this struct, it
// will result in a prediction error of some degree.
typedef struct
{
	pmtype_t	pm_type;

#ifdef KMQUAKE2_ENGINE_MOD	// Knightmare- larger coordinate range
	int			origin[3];		// 20.3
#else
	short		origin[3];		// 12.3
#endif
	short		velocity[3];				// 12.3
	byte		pm_flags;					// ducked, jump_held, etc
	byte		pm_time;					// each unit = 8 ms
	short		gravity;
	short		delta_angles[3];			// add to command angles to get view direction
											// (changed by spawns, rotating objects, and teleporters)
} pmove_state_t;


// button bits
#define	BUTTON_ATTACK		1
#define	BUTTON_USE			2
#define BUTTON_ATTACK2      4
#define BUTTONS_ATTACK (BUTTON_ATTACK | BUTTON_ATTACK2)
#define	BUTTON_ANY			128				// any key whatsoever


// usercmd_t is sent to the server each client frame
typedef struct usercmd_s
{
	byte	msec;
	byte	buttons;
	short	angles[3];
	short	forwardmove;
	short	sidemove;
	short	upmove;
	byte	impulse;						// remove?
	byte	lightlevel;						// light level the player is standing on
} usercmd_t;


#define	MAXTOUCH	32
typedef struct
{
	// state (in / out)
	pmove_state_t	s;

	// command (in)
	usercmd_t		cmd;
	qboolean		snapinitial;			// if s has been changed outside pmove

	// results (out)
	int			numtouch;
	struct edict_s	*touchents[MAXTOUCH];

	vec3_t		viewangles;					// clamped
	float		viewheight;

	vec3_t		mins;						// bounding box size
	vec3_t		maxs;						//

	struct edict_s	*groundentity;
	int			watertype;
	int			waterlevel;

	// callbacks to test the world
	trace_t		(*trace) (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end);
	int			(*pointcontents) (vec3_t point);
} pmove_t;


// entity_state_t->effects
// Effects are things handled on the client side (lights, particles, frame animations)
// that happen constantly on the given entity.
// An entity that has effects will be sent to the client even if it has a zero index model.
#define	EF_ROTATE			0x00000001		// rotate (bonus items)
#define	EF_GIB				0x00000002		// leave a trail
#define	EF_BLASTER			0x00000008		// redlight + trail
#define	EF_ROCKET			0x00000010		// redlight + trail
#define	EF_GRENADE			0x00000020
#define	EF_HYPERBLASTER		0x00000040
#define	EF_BFG				0x00000080
#define EF_COLOR_SHELL		0x00000100
#define EF_POWERSCREEN		0x00000200
#define	EF_ANIM01			0x00000400		// automatically cycle between frames 0 and 1 at 2 hz
#define	EF_ANIM23			0x00000800		// automatically cycle between frames 2 and 3 at 2 hz
#define EF_ANIM_ALL			0x00001000		// automatically cycle through all frames at 2hz
#define EF_ANIM_ALLFAST		0x00002000		// automatically cycle through all frames at 10hz
#define	EF_FLIES			0x00004000
#define	EF_QUAD				0x00008000
#define	EF_PENT				0x00010000
#define	EF_TELEPORTER		0x00020000		// particle fountain
#define EF_FLAG1			0x00040000
#define EF_FLAG2			0x00080000
// RAFAEL
#define EF_IONRIPPER		0x00100000
#define EF_GREENGIB			0x00200000
#define	EF_BLUEHYPERBLASTER 0x00400000
#define EF_SPINNINGLIGHTS	0x00800000
#define EF_PLASMA			0x01000000
#define EF_TRAP				0x02000000

//ROGUE
#define EF_TRACKER			0x04000000
#define	EF_DOUBLE			0x08000000
#define	EF_SPHERETRANS		0x10000000
#define EF_TAGTRAIL			0x20000000
#define EF_HALF_DAMAGE		0x40000000
#define EF_TRACKERTRAIL		0x80000000
//ROGUE

// entity_state_t->renderfx flags
#define	RF_MINLIGHT			1				// allways have some light (viewmodel)
#define	RF_VIEWERMODEL		2				// don't draw through eyes, only mirrors
#define	RF_WEAPONMODEL		4				// only draw through eyes
#define	RF_FULLBRIGHT		8				// allways draw full intensity
#define	RF_DEPTHHACK		16				// for view weapon Z crunching
#define	RF_TRANSLUCENT		32
#define	RF_FRAMELERP		64
#define RF_BEAM				128
#define	RF_CUSTOMSKIN		256				// skin is an index in image_precache
#define	RF_GLOW				512				// pulse lighting for bonus items
#define RF_SHELL_RED		1024
#define	RF_SHELL_GREEN		2048
#define RF_SHELL_BLUE		4096

//ROGUE
#define RF_IR_VISIBLE		0x00008000		// 32768
#define	RF_SHELL_DOUBLE		0x00010000		// 65536
#define	RF_SHELL_HALF_DAM	0x00020000
#define RF_USE_DISGUISE		0x00040000
//ROGUE

// player_state_t->refdef flags
#define	RDF_UNDERWATER		1				// warp the screen as apropriate
#define RDF_NOWORLDMODEL	2				// used for player configuration screen

//ROGUE
#define	RDF_IRGOGGLES		4
#define RDF_UVGOGGLES		8
//ROGUE


// muzzle flashes / player effects
#define	MZ_BLASTER			0
#define MZ_MACHINEGUN		1
#define	MZ_SHOTGUN			2
#define	MZ_CHAINGUN1		3
#define	MZ_CHAINGUN2		4
#define	MZ_CHAINGUN3		5
#define	MZ_RAILGUN			6
#define	MZ_ROCKET			7
#define	MZ_GRENADE			8
#define	MZ_LOGIN			9
#define	MZ_LOGOUT			10
#define	MZ_RESPAWN			11
#define	MZ_BFG				12
#define	MZ_SSHOTGUN			13
#define	MZ_HYPERBLASTER		14
#define	MZ_ITEMRESPAWN		15
// RAFAEL
#define MZ_IONRIPPER		16
#define MZ_BLUEHYPERBLASTER 17
#define MZ_PHALANX			18
#define MZ_SILENCED			128				// bit flag ORed with one of the above numbers
//ROGUE
#define MZ_ETF_RIFLE		30
#define MZ_UNUSED			31
#define MZ_SHOTGUN2			32
#define MZ_HEATBEAM			33
#define MZ_BLASTER2			34
#define	MZ_TRACKER			35
#define	MZ_NUKE1			36
#define	MZ_NUKE2			37
#define	MZ_NUKE4			38
#define	MZ_NUKE8			39
//ROGUE


// temp entity events
//
// Temp entity events are for things that happen at a location seperate from any existing entity.
// Temporary entity messages are explicitly constructed and broadcast.
typedef enum
{
	TE_GUNSHOT,
	TE_BLOOD,
	TE_BLASTER,
	TE_RAILTRAIL,
	TE_SHOTGUN,
	TE_EXPLOSION1,
	TE_EXPLOSION2,
	TE_ROCKET_EXPLOSION,
	TE_GRENADE_EXPLOSION,
	TE_SPARKS,
	TE_SPLASH,
	TE_BUBBLETRAIL,
	TE_SCREEN_SPARKS,
	TE_SHIELD_SPARKS,
	TE_BULLET_SPARKS,
	TE_LASER_SPARKS,
	TE_PARASITE_ATTACK,
	TE_ROCKET_EXPLOSION_WATER,
	TE_GRENADE_EXPLOSION_WATER,
	TE_MEDIC_CABLE_ATTACK,
	TE_BFG_EXPLOSION,
	TE_BFG_BIGEXPLOSION,
	TE_BOSSTPORT,				// used as '22' in a map, so DON'T RENUMBER!
	TE_BFG_LASER,
	TE_GRAPPLE_CABLE,
	TE_WELDING_SPARKS,
	TE_GREENBLOOD,
	TE_BLUEHYPERBLASTER,
	TE_PLASMA_EXPLOSION,
	TE_TUNNEL_SPARKS,
// ROGUE
	TE_BLASTER2,
	TE_RAILTRAIL2,
	TE_FLAME,
	TE_LIGHTNING,
	TE_DEBUGTRAIL,
	TE_PLAIN_EXPLOSION,
	TE_FLASHLIGHT,
	TE_FORCEWALL,
	TE_HEATBEAM,
	TE_MONSTER_HEATBEAM,
	TE_STEAM,
	TE_BUBBLETRAIL2,
	TE_MOREBLOOD,
	TE_HEATBEAM_SPARKS,
	TE_HEATBEAM_STEAM,
	TE_CHAINFIST_SMOKE,
	TE_ELECTRIC_SPARKS,
	TE_TRACKER_EXPLOSION,
	TE_TELEPORT_EFFECT,
	TE_DBALL_GOAL,
	TE_WIDOWBEAMOUT,
	TE_NUKEBLAST,
	TE_WIDOWSPLASH,
	TE_EXPLOSION1_BIG,
	TE_EXPLOSION1_NP,
	TE_FLECHETTE,
// ROGUE
// Knightmare added
	TE_REDBLASTER,			//56
	TE_SHOCKSPLASH,			//57
	TE_BLASTER_COLORED,		//58
	TE_RAILTRAIL_COLORED,	//59
} temp_event_t;

#define SPLASH_UNKNOWN		0
#define SPLASH_SPARKS		1
#define SPLASH_BLUE_WATER	2
#define SPLASH_BROWN_WATER	3
#define SPLASH_SLIME		4
#define	SPLASH_LAVA			5
#define SPLASH_BLOOD		6


// sound channels
// channel 0 never willingly overrides
// other channels (1-7) allways override a playing sound on that channel
#define	CHAN_AUTO               0
#define	CHAN_WEAPON             1
#define	CHAN_VOICE              2
#define	CHAN_ITEM               3
#define	CHAN_BODY               4
// modifier flags
#define	CHAN_NO_PHS_ADD			8	// send to all clients, not just ones in PHS (ATTN 0 will also do this)
#define	CHAN_RELIABLE			16	// send by reliable message, not datagram


// sound attenuation values
#define	ATTN_NONE               0	// full volume the entire level
#define	ATTN_NORM               1
#define	ATTN_IDLE               2
#define	ATTN_STATIC             3	// diminish very rapidly with distance


// player_state->stats[] indexes
#define STAT_HEALTH_ICON		0
#define	STAT_HEALTH				1
#define	STAT_AMMO_ICON			2
#define	STAT_AMMO				3
#define	STAT_ARMOR_ICON			4
#define	STAT_ARMOR				5
#define	STAT_SELECTED_ICON		6
#define	STAT_PICKUP_ICON		7
#define	STAT_PICKUP_STRING		8
#define	STAT_TIMER_ICON			9
#define	STAT_TIMER				10
#define	STAT_HELPICON			11
#define	STAT_SELECTED_ITEM		12
#define	STAT_LAYOUTS			13
#define	STAT_FRAGS				14
#define	STAT_FLASHES			15		// cleared each frame, 1 = health, 2 = armor
// Knightmare- some extra stats for scripted HUDs
#define STAT_TIMER_RANGE		251
#define STAT_MAXAMMO			252
#define STAT_MAXARMOR			253
#define STAT_MAXHEALTH			254
#define STAT_WEAPON				255

#ifdef KMQUAKE2_ENGINE_MOD // Knightmare increased
#define	MAX_STATS				256
#else
#define	MAX_STATS				32
#endif
#define	OLD_MAX_STATS			32	// needed for backward compatibility


// dmflags->value flags
#define	DF_NO_HEALTH		0x00000001	// 1
#define	DF_NO_ITEMS			0x00000002	// 2
#define	DF_WEAPONS_STAY		0x00000004	// 4
#define	DF_NO_FALLING		0x00000008	// 8
#define	DF_INSTANT_ITEMS	0x00000010	// 16
#define	DF_SAME_LEVEL		0x00000020	// 32
#define DF_SKINTEAMS		0x00000040	// 64			//CW: Note that this flag is no longer used
#define DF_MODELTEAMS		0x00000080	// 128			//CW: Note that this flag is no longer used
#define DF_NO_FRIENDLY_FIRE	0x00000100	// 256			//CW: Note that this flag is no longer used
#define	DF_SPAWN_FARTHEST	0x00000200	// 512
#define DF_FORCE_RESPAWN	0x00000400	// 1024
#define DF_NO_ARMOR			0x00000800	// 2048
#define DF_ALLOW_EXIT		0x00001000	// 4096
#define DF_INFINITE_AMMO	0x00002000	// 8192
#define DF_QUAD_DROP		0x00004000	// 16384
#define DF_FIXED_FOV		0x00008000	// 32768
//CW++
#define DF_FAST_SWITCH      0x00010000	// 65536
#define DF_EXTRA_ITEMS      0x00020000	// 131072
#define DF_NO_REPLACEMENTS  0x00040000	// 262144
#define DF_FORCEJOIN		0x00080000	// 524288	
#define DF_ARMOR_PROTECT	0x00100000	// 1048576
#define DF_CTF_NO_TECH		0x00200000	// 2097152
#define DF_CTF_SPAWNS_ONLY	0x00400000	// 4194304
//CW--

/*
==========================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

==========================================================
*/

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))


// Config strings are a general means of communication from the server to all connected clients.
// Each config string can be at most MAX_QPATH characters.
#define	CS_NAME				0
#define	CS_CDTRACK			1
#define	CS_SKY				2
#define	CS_SKYAXIS			3		// %f %f %f format
#define	CS_SKYROTATE		4
#define	CS_STATUSBAR		5		// display program string

#define CS_AIRACCEL			29		// air acceleration control
#define	CS_MAXCLIENTS		30
#define	CS_MAPCHECKSUM		31		// for catching cheater maps

#define	CS_MODELS			32
#define	CS_SOUNDS			(CS_MODELS+MAX_MODELS)				//288
#define	CS_IMAGES			(CS_SOUNDS+MAX_SOUNDS)				//544
#define	CS_LIGHTS			(CS_IMAGES+MAX_IMAGES)				//800
#define	CS_ITEMS			(CS_LIGHTS+MAX_LIGHTSTYLES)			//1056
#define	CS_PLAYERSKINS		(CS_ITEMS+MAX_ITEMS)				//1312
#define CS_GENERAL			(CS_PLAYERSKINS+MAX_CLIENTS)		//1568
#define	CS_PAKFILE			(CS_GENERAL+MAX_GENERAL)
#define	MAX_CONFIGSTRINGS	(CS_PAKFILE+1)
//#define	MAX_CONFIGSTRINGS	(CS_GENERAL+MAX_GENERAL)			//2080

// Knightmare- hacked configstring offsets for backward compatiblity
#define OLD_CS_SOUNDS			(CS_MODELS+OLD_MAX_MODELS)
#define	OLD_CS_IMAGES			(OLD_CS_SOUNDS+OLD_MAX_SOUNDS)
#define	OLD_CS_LIGHTS			(OLD_CS_IMAGES+OLD_MAX_IMAGES)
#define	OLD_CS_ITEMS			(OLD_CS_LIGHTS+MAX_LIGHTSTYLES)
#define	OLD_CS_PLAYERSKINS		(OLD_CS_ITEMS+MAX_ITEMS)
#define OLD_CS_GENERAL			(OLD_CS_PLAYERSKINS+MAX_CLIENTS)
#define	OLD_MAX_CONFIGSTRINGS	(OLD_CS_GENERAL+MAX_GENERAL)
// end Knightmare

//==============================================

// entity_state_t->event values
// Entity events are for effects that take place relative to an existing entities origin.
// Very network efficient. All muzzle flashes really should be converted to events.
typedef enum
{
	EV_NONE,
	EV_ITEM_RESPAWN,
	EV_FOOTSTEP,
	EV_FALLSHORT,
	EV_FALL,
	EV_FALLFAR,
	EV_PLAYER_TELEPORT,
	EV_OTHER_TELEPORT,
	// Knightmare added
	EV_LOUDSTEP, // loud footstep from landing
	EV_SLOSH, // Sloshing in ankle-deep water
	EV_WADE, // wading or treading water
	EV_WADE_MUD, // wading in mud
	EV_CLIMB_LADDER, //climbing ladder
	EV_PLAYER_TELEPORT2	// Q1-style teleport, sounds in rogue pak0.pak
	// end Knightmare
} entity_event_t;


// entity_state_t is the information conveyed from the server in an update message 
// about entities that the client will need to render in some way
typedef struct entity_state_s
{
	int		number;			// edict index

	vec3_t	origin;
	vec3_t	angles;
	vec3_t	old_origin;		// for lerping

	int		modelindex;
	int		modelindex2;	// weapons
	int		modelindex3;	// CTF flags
	int		modelindex4;	// indicator sprites/models
#ifdef KMQUAKE2_ENGINE_MOD //Knightmare- Privater wanted this
	int		modelindex5, modelindex6;	//more attached models
#endif
	int		frame;
	int		skinnum;
#ifdef KMQUAKE2_ENGINE_MOD //Knightmare- allow the server to set this
	float	alpha;	//entity transparency
#endif
	unsigned int effects;	// PGM - we're filling it, so it needs to be unsigned
	int		renderfx;
	int		solid;			// for client side prediction, 8*(bits 0-4) is x/y radius
							// 8*(bits 5-9) is z down distance, 8(bits10-15) is z up (gi.linkentity sets this properly)
	int		sound;			// for looping sounds, to guarantee shutoff
#ifdef LOOP_SOUND_ATTENUATION // Knightmare- added sound attenuation
	float	attenuation;
#endif
	int		event;			// impulse events -- muzzle flashes, footsteps, etc
							// events only go out for a single frame; they are automatically cleared each frame
} entity_state_t;

//==============================================


// player_state_t is the information needed in addition to pmove_state_t to render a view.
// There will only be 10 player_state_t sent each second, but the number of pmove_state_t 
// changes will be reletive to client frame rates.
typedef struct
{
	pmove_state_t	pmove;				// for prediction

	// these fields do not need to be communicated bit-precise
	vec3_t		viewangles;				// for fixed views
	vec3_t		viewoffset;				// add to pmovestate->origin
	vec3_t		kick_angles;			// add to view direction to get render angles
										// set by weapon kicks, pain effects, etc
	vec3_t		gunangles;
	vec3_t		gunoffset;
	int			gunindex;
	int			gunframe;

#ifdef KMQUAKE2_ENGINE_MOD		// Knightmare added
	int			gunskin;		// for animated weapon skins
	int			gunindex2;		// for a second weapon model (boot)
	int			gunframe2;
	int			gunskin2;

	// server-side speed control!
	int			maxspeed;
	int			duckspeed;
	int			waterspeed;
	int			accel;
	int			stopspeed;
#endif					//end Knightmare

	float		blend[4];				// rgba full screen effect
	float		fov;					// horizontal field of view
	int			rdflags;				// refdef flags
	short		stats[MAX_STATS];		// fast status bar updates
} player_state_t;


// ==================
// PGM 
#define VIDREF_GL		1
#define VIDREF_SOFT		2
#define VIDREF_OTHER	3

extern int vidref_val;
// PGM
// ==================
