// g_bot.h

//Maj/Pon++
#define MAXNODES	10000
#define MAXLINKPOD	6			// don't modify this
#define MAXBOTS		51			// number of name/skin entries

//skill[]
#define AIMACCURACY		0		// 0..9
#define AGGRESSION		1		// 0..9
#define COMBATSKILL		2		// 0..9
#define VRANGEVIEW		3		// 60..120 deg	- Vertical view range
#define HRANGEVIEW		4		// 60..120 deg	- Horizontal view range
#define PRIMARYWEAP		5		// 2..14		- 2 = Desert Eagle, 14 = AGM

typedef struct
{
	vec3_t	Pt;
	union
	{
		vec3_t		Tcorner;					// target corner (train and grapple-shot only)
		unsigned	short linkpod[MAXLINKPOD];	// (GRS_NORMAL, GRS_ITEMS only 0 = do not select pod)
	} podunion;																						//CW
	edict_t	*ent;								// target ent
	short	index;								// index num
	short	state;								// targetstate
} route_t;

//CW++
// Avoid nameless unions for Linux compiler.

#define Tcorner	podunion.Tcorner
#define linkpod	podunion.linkpod
//CW--

extern route_t	Route[MAXNODES];		// list of bot route nodes (from chn file)
extern int		TotalRouteNodes;		// number of used nodes in the 'Route[]' array
extern int		CurrentIndex;			// current node number that bot is at
extern float	JumpMax;				// max delta-height that bot can jump to 					//Pon


typedef  struct
{
	char		netname[MAX_NAMELEN];	// netname													//CW was 21
	char		skin[MAX_SKINLEN];		// skin														//CW
	int			ingame;					// spawned
	int			skill[6];				// parameters
	qboolean	camper;					// TRUE => bot will camp certain items						//CW++
} botinfo_t;

extern botinfo_t Bot[MAXBOTS+1];


void gi_cprintf(edict_t *ent, int printlevel, char *fmt, ...);
void gi_centerprintf(edict_t *ent, char *fmt, ...);
void gi_bprintf(int printlevel, char *fmt, ...);

void Use_Plat(edict_t *ent, edict_t *other, edict_t *activator);
void train_use(edict_t *self, edict_t *other, edict_t *activator);
void button_use(edict_t *self, edict_t *other, edict_t *activator);
void door_use(edict_t *self, edict_t *other, edict_t *activator);
void rotating_use(edict_t *self, edict_t *other, edict_t *activator);
void trigger_relay_use(edict_t *self, edict_t *other, edict_t *activator);
void plat_go_up(edict_t *ent);
void SV_Physics_Step(edict_t *ent);
void path_corner_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void DoRespawn(edict_t *ent);
void Cmd_Kill_f(edict_t *ent);
void ClientUserinfoChanged(edict_t *ent, char *userinfo);
void ClientDisconnect(edict_t *ent);
void Use_Item(edict_t *ent, edict_t *other, edict_t *activator);

// weapon functions
int WeapIndex(int weap);
void Weapon_DesertEagle(edict_t *self);																//CW
void Weapon_GaussPistol(edict_t *self);
void Weapon_Jackhammer(edict_t *self);
void Weapon_Mac10(edict_t *self);
void Weapon_ESG(edict_t *self);
void Weapon_Flamethrower(edict_t *self);
void Weapon_RocketLauncher (edict_t *self);
void Weapon_C4(edict_t *self);
void Weapon_Trap(edict_t *self);
void Weapon_Railgun (edict_t *self);
void Weapon_ShockRifle(edict_t *self);
void Weapon_AGM(edict_t *self);
void Weapon_DiscLauncher(edict_t *self);
void Weapon_Chainsaw(edict_t *self);


void LoadBots(void);																				//CW
int GetKindWeapon(gitem_t *it);
void ReadRouteFile(void);
void droptofloor2(edict_t *ent);
void RespawnAllBots(void);
void TauntVictim(edict_t *ent, edict_t *victim);
void InsultVictim(edict_t *ent, edict_t *victim);
qboolean Pickup_Navi(edict_t *ent, edict_t *other);
void Bot_Think(edict_t *ent);
void CheckCampSite(edict_t *ent,edict_t *other);
void bFuncPlat(edict_t *ent);
void bFuncButton(edict_t *ent);
void bDoorBlocked(edict_t *ent);
void bFuncDoor(edict_t *ent);
void bFuncTrain(edict_t *ent);
void ResetGroundSlope(edict_t *ent);
void TraceAllSolid(edict_t *ent, vec3_t point, trace_t tr);
void CheckPrimaryWeapon(edict_t *ent, edict_t *other);
void G_FindTrainTeam(void);
void ForceRouteReset(edict_t *other);
void InitAllItems(void);
float SetBotXYSpeed(edict_t *ent, float *xyspeed);
void SetBotThink(edict_t *ent);
void CheckBotCrushed(edict_t *targ,edict_t *inflictor,int mod);
void Bot_CheckEnemy(gclient_t *client, edict_t *attacker, edict_t *targ, int mod);
void BotCheckGrapple(edict_t *ent);

//CW++
qboolean TraceX(edict_t *ent, vec3_t p2);
float Get_yaw(vec3_t vec);
float Get_vec_yaw(vec3_t vec, float yaw);
qboolean RemoveBot(edict_t *ent);
void SpawnNumBots_Safe(int numbots);
void RemoveNumBots_Safe(int numbots);
void Move_LastRouteIndex(void);
void Get_WaterState(edict_t *ent);
void Get_RouteOrigin(int index, vec3_t pos);


// behaviour variables/factors
#define BOT_FALLCHK_LOOPMAX		30
#define BOT_MAX_TAUNT_DIST		250
#define BOT_CAMP_PROB			0.5

#define BOT_TELE_MINHEALTH		20		// bot teleports if health falls below this during a fight...
#define BOT_TELE_ENEMYRANGE		480		// ...and enemy is within this range
#define BOT_TELE_FALLSPEED		-400	// bot teleports if vertical velocity exceeds this towards an enviro hazard

#define	BOT_CHICKENAIM_FACTOR	0.7		// aiming modification for chicken-shooting

#define BOT_DODGE_ENEMYRANGE	300		// range within which bot will try to dodge enemy shots
#define BOT_RUSH_ENEMYRANGE		400		// range within which bot will try to rush an enemy

#define BOT_PNOISE_SELF_DIST	700		// range within which bot will hear a personal noise (eg. jumping, pain, weapon firing)
#define BOT_PNOISE_IMPACT_DIST	300		// range within which bot will hear a weapon target noise (eg. bullet wall impacts)
#define BOT_PNOISE_RADIUS		300		// enemy's sound projected forward this much when testing for hidden sounds
//CW--

// moving speed ----------------------------------------------------
#define MOVE_SPD_DUCK		16
#define MOVE_SPD_WATER		16
#define	MOVE_SPD_WALK		20
#define MOVE_SPD_RUN		30
#define MOVE_SPD_JUMP		30

#define VEL_BOT_JUMP		340			// jump velocity
#define VEL_BOT_WJUMP		340			// waterjump velocity
#define VEL_BOT_JUMP_NUDGE	20			// nudge-jump velocity
#define VEL_BOT_LADDERUP	200			// ladder-up velocity
#define VEL_BOT_WLADDERUP	200			// water ladder-up gain
#define VEL_BOT_ROCJUMP		500			// rocketjump velocity

// function's state P ----------------------------------------------
#define	PSTATE_TOP			0
#define	PSTATE_BOTTOM		1
#define PSTATE_UP			2
#define PSTATE_DOWN			3

#define PDOOR_TOGGLE		32

// height ----------------------------------------------------------
#define TOP_LIMIT			52
#define TOP_LIMIT_WATER		100
#define BOTTOM_LIMIT		-52
#define BOTTOM_LIMIT_WATER	-8190
#define BOTTOM_LIMITM		-300

// waterstate ------------------------------------------------------
#define	WAS_NONE			0
#define	WAS_FLOAT			1	
#define	WAS_IN				2

// route chaining pod state ----------------------------------------
#define GRS_NORMAL			0
#define GRS_ONROTATE		1
#define GRS_TELEPORT		2
#define GRS_ITEMS			3
#define GRS_ONPLAT			4
#define	GRS_ONTRAIN			5
#define GRS_ONDOOR			6
#define GRS_PUSHBUTTON		7

#define GRS_GRAPSHOT		20
#define GRS_GRAPHOOK		21
#define GRS_GRAPRELEASE		22

#define GRS_REDFLAG			-10
#define GRS_BLUEFLAG		-11

#define POD_LOCKFRAME		15
#define POD_RELEFRAME		20

#define MAX_SEARCH			12			// max search count / FRAMETIME
#define MAX_DOORSEARCH		10

// trace params ----------------------------------------------------
#define	TRP_NOT				0			// don't trace
#define TRP_NORMAL			1			// trace normal
#define	TRP_ANGLEKEEP		2			// trace and keep angle
#define TRP_MOVEKEEP		3			// angle and move vec keep but move
#define TRP_ALLKEEP			4			// don't move

// combat: aiming --------------------------------------------------
#define AIM_SFPOS			5.0			// aiming position offset scale-factor
#define AIM_SFANG_HITSCAN	0.75		// aimimg angle scale-factor: hitscan weapons
#define AIM_SFANG_PROJ		0.35		// aimimg angle scale-factor: projectile weapons

// movestate (general status list) ---------------------------------
#define STS_IDLE			0x00000000	// normal running
#define STS_LADDERUP		0x00000001	// climb the ladder
#define STS_TURBOJ			0x00000002	// turbo jump
#define STS_WATERJ			0x00000004	// water jump
#define STS_ROCJ			0x00000008	// rocket jumping
#define STS_TRIGPUSH		0x00001000	// hit a trigger_push										//CW++
#define STS_AGMMOVE			0x00002000	// movement due to enemy AGM								//CW++

#define STS_SJMASK			(STS_ROCJ | STS_TURBOJ | STS_TRIGPUSH | STS_AGMMOVE | STS_WATERJ)		//CW
#define STS_SJMASKEXW		(STS_ROCJ | STS_TURBOJ | STS_TRIGPUSH | STS_AGMMOVE)					//CW

// wait
#define STS_W_DONT			0x00000010	// don't wait for door/plat/train
#define STS_W_DOOROPEN		0x00000020	// wait for door open or down to bottom
#define STS_W_ONPLAT		0x00000040	// wait for plat or door reach to the top
#define STS_W_ONDOORUP		0x00000080	// wait for door reach to the top
#define STS_W_ONDOORDWN		0x00000100	// wait for door reach to the bottom
#define STS_W_ONTRAIN		0x00000200	// wait for door/plat/train reach to the top
#define STS_W_COMEPLAT		0x00000400	// wait for plat to come
#define STS_W_COMETRAIN		0x00000800	// wait for train to come

#define STS_WAITS			(STS_W_DONT | STS_W_DOOROPEN | STS_W_ONPLAT | STS_W_ONDOORUP | STS_W_ONDOORDWN | STS_W_ONTRAIN | STS_W_COMEPLAT | STS_W_COMETRAIN)	//CW
#define STS_WAITSMASK		(STS_W_DOOROPEN | STS_W_ONPLAT | STS_W_ONDOORUP | STS_W_ONDOORDWN | STS_W_ONTRAIN | STS_W_COMEPLAT)
#define STS_WAITSMASK2		(STS_W_ONDOORUP | STS_W_ONDOORDWN | STS_W_ONPLAT | STS_W_ONTRAIN)
#define STS_WAITSMASKCOM	(STS_W_DOOROPEN | STS_W_ONPLAT | STS_W_ONDOORUP | STS_W_ONDOORDWN | STS_W_ONTRAIN)

// battlemode (& bot skills)----------------------------------------
#define FIRE_NULL			0x00000000	// mode:		none										//CW++
#define FIRE_SLIDEMODE		0x00000001	// mode:		slide with route
#define	FIRE_PRESTAYFIRE	0x00000002	// mode/skill:	stand still and fires (pre)
#define FIRE_STAYFIRE		0x00000004	// mode/skill:	stand still and fires
#define FIRE_CHICKEN		0x00000008	// mode:		chicken-shooting
#define FIRE_RUSH			0x00000010	// mode:		rush attack
#define	FIRE_ESTIMATE		0x00000020	// mode:		fire in estimated enemy direction
#define FIRE_C4				0x00000040	// mode:		using C4 									//CW++

#define FIRE_JUMPROC		0x00000100	// skill:		jump whilst firing rockets
#define FIRE_REFUGE			0x00000200	// mode/skill:	retreat (back along route) whilst firing
#define FIRE_QUADUSE		0x00000400	// skill:		select good weapons when Quadded
#define FIRE_C4USE			0x00000800	// skill:		use C4 more competently						//CW++
#define FIRE_AVOIDEXPLO		0x00001000	// mode/skill:	try to avoid own RL & SR explosions
#define FIRE_AVOIDINVULN	0x00002000	// skill:		avoid Invulnerable players
#define FIRE_DODGE			0x00004000	// mode/skill:	dodge when being aimed at					//CW++
#define FIRE_IGNORE			0x00008000	// mode/skill:	ignore distant enemies

#define FIRE_SHIFT_R		0x10000000	// mode:		strafe right
#define FIRE_SHIFT_L		0x20000000	// mode:		strafe left
#define FIRE_SHIFT			(FIRE_SHIFT_R | FIRE_SHIFT_L)

// combatstate ----------------------------------------------------
#define CTS_ENEM_NSEE		0x00000001	// have enemy but can't see
#define CTS_AGRBATTLE		0x00000002	// aggressive battle
#define	CTS_ESCBATTLE		0x00000004	// escaping battle (item want)
#define CTS_HIPBATTLE		0x00000008	// high position battle (camp)

// combatstate: shooting
#define CTS_PREAIMING		0x00000010	// prepare for snipe or lockon
#define CTS_AIMING			0x00000020	// aimning for snipe or lockon
#define CTS_GRENADE			0x00000040  // hand grenade mode
#define CTS_JUMPSHOT		0x00000080	// jump shot

#define CTS_COMBS			(CTS_AGRBATTLE | CTS_ESCBATTLE | CTS_HIPBATTLE | CTS_ENEM_NSEE)
//Maj/Pon--
