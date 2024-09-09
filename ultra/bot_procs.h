#define	ERASER_VERSION		1.01

#define	MAX_BOTS	25

#define	STEPSIZE	24
#define	BOT_RUN_SPEED		300
#define BOT_STRAFE_SPEED	200
#define	BOT_IDEAL_DIST_FROM_ENEMY	160

#define	WANT_KINDA		1
#define	WANT_YEH_OK		2
#define	WANT_SHITYEAH	3

#define	BOT_GUARDING_RANGE 600.0

// bot_ai.c
// these define how long the bot will search for it's enemy before giving up
#define	BOT_SEARCH_LONG		4
#define	BOT_SEARCH_MEDIUM	2
#define	BOT_SEARCH_SHORT	1

#define	SIGHT_FIRE_DELAY	0.8		// so bot's don't fire straight away after sighting an enemy

// Knightmare- made extern to fix GCC compile
extern	int	 spawn_bots;
extern	int	 roam_calls_this_frame;
extern	int	 bestdirection_callsthisframe;

// ---- BOT CHAT DATA ----

#define	CHAT_GREETINGS			0
#define	CHAT_INSULTS_GENERAL	1
#define	CHAT_INSULTS_KICKASS	2
#define	CHAT_INSULTS_LOSING		3
#define	CHAT_COMEBACKS			4
#define	CHAT_TEAMPLAY_HELP		5
#define	CHAT_TEAMPLAY_DROPITEM	6
#define	CHAT_TEAMPLAY_GROUP		7

#define	NUM_CHAT_SECTIONS		8
#define	MAX_CHAT_PER_SECTION	64

// Knightmare- made extern to fix GCC compile
extern	char	*bot_chat_text[NUM_CHAT_SECTIONS][MAX_CHAT_PER_SECTION];
extern	int		bot_chat_count[NUM_CHAT_SECTIONS];
extern	float	last_bot_chat[NUM_CHAT_SECTIONS];

extern	int		num_view_weapons;
extern	char	view_weapon_models[64][64];

// -----------------------

int	RoamFindBestItem (edict_t	*self, edict_t	*list_head, int	check_paths);
void	bot_ChangeYaw (edict_t *self);
void	bot_MoveAI (edict_t *self, int dist);
float	bot_ReachedTrail (edict_t *self);
void	botMachineGun (edict_t *self);
int		bot_move (edict_t *self, float dist);
int		bot_oldmove (edict_t *self, float dist);
void	respawn_bot (edict_t *self);
void	bot_SuicideIfStuck (edict_t *self);

void bot_pain (edict_t *self, edict_t *other, float kick, int damage);
void bot_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void bot_run (edict_t *self);

edict_t	*bot_FindAnyTrail (edict_t *bot);
int	CanJump (edict_t *ent);

void bot_AnimateFrames (edict_t *self);
void bot_roam (edict_t *self, int force_enemy);
int	botCheckStuck (edict_t	*self);
int CanStand (edict_t	*self);
int	CanSee (edict_t *self, edict_t *targ);
int	CanReach (edict_t *self, edict_t *targ);

// Knightmare- made extern to fix GCC compile
extern	int	botdebug;
void botDebugPrint (char *msg, ...);

// bot_wpns.c

#define	FIRE_INTERVAL_BLASTER			0.6
#define	FIRE_INTERVAL_ROCKETLAUNCHER	0.8
#define	FIRE_INTERVAL_GRENADELAUNCHER	0.9
#define	FIRE_INTERVAL_RAILGUN			1.5
#define	FIRE_INTERVAL_HYPERBLASTER		0
#define	FIRE_INTERVAL_CHAINGUN			0
#define FIRE_INTERVAL_MACHINEGUN		0
#define FIRE_INTERVAL_SHOTGUN			1
#define FIRE_INTERVAL_SSHOTGUN			1
#define FIRE_INTERVAL_BFG				2.8

#define	BOT_CHANGEWEAPON_DELAY	0.9

void	bot_FireWeapon (edict_t	*self);
void	bot_Attack (edict_t *self);

void botBlaster (edict_t *self);
void botMachineGun (edict_t *self);
void botShotgun (edict_t *self);
void botSuperShotgun (edict_t *self);
void botChaingun (edict_t *self);
void botRailgun (edict_t *self);
void botRocketLauncher (edict_t *self);
void botGrenadeLauncher (edict_t *self);
void botHyperblaster (edict_t *self);
void botBFG (edict_t *self);

void	botPickBestWeapon (edict_t *self);
int		botHasWeaponForAmmo (gclient_t *client, gitem_t *item);
int		ClientHasAnyWeapon(gclient_t	*client);
int		botCanPickupAmmo (gclient_t *client, gitem_t *item);
int		botCanPickupArmor (edict_t *self, edict_t *ent);

void	botPickBestFarWeapon (edict_t *self);
void	botPickBestCloseWeapon (edict_t *self);

void	GetBotFireForWeapon (gitem_t	*weapon,	void (**bot_fire)(edict_t	*self));

// bot_spawn.c
edict_t	*spawn_bot (char *botname);
void	botDisconnect (edict_t	*self);

// bot_misc.c
void	ReadBotConfig (void);
bot_info_t	*GetBotData (char *botname);
void	NodeDebug (char *fmt, ...);
void	FindVisibleItemsFromNode(edict_t	*node);
void	AdjustRatingsToSkill(edict_t *self);
edict_t *DrawLine (edict_t *owner, vec3_t spos, vec3_t epos);
void	TeamGroup (edict_t *ent);
void	TeamDisperse (edict_t *self);
void	BotGreeting (edict_t *chat);
void	BotInsultStart (edict_t *self);
void	BotInsult (edict_t *self, edict_t *enemy, int chat_type);
qboolean SameTeam (edict_t *plyr1, edict_t *plyr2);
float	HomeFlagDist (edict_t *self);
int		CarryingFlag (edict_t *ent);

// bot_nav.c
void	botRoamFindBestDirection (edict_t	*self);
void	botRandomJump (edict_t	*self);
void	BotMoveThink (edict_t *ent, usercmd_t *ucmd);
int		botJumpAvoidEnt (edict_t *self, edict_t *e_avoid);


// FIXME: this should go in g_local.h
qboolean monster_start (edict_t *self);
qboolean monster_start_go (edict_t *self);
void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
void	Use_Quad (edict_t *ent, gitem_t *item);
void	Use_Double (edict_t *ent, gitem_t *item);
void	Use_QuadFire (edict_t *ent, gitem_t *item);
void	Use_Invulnerability (edict_t *ent, gitem_t *item);
void	Use_Doppleganger (edict_t *ent, gitem_t *item);
void	Use_Defender (edict_t *ent, gitem_t *item);
void	Use_Hunter (edict_t *ent, gitem_t *item);
void	Use_Vengeance (edict_t *ent, gitem_t *item);
void	ClientDisconnect (edict_t *ent);
void	Use_Plat (edict_t *ent, edict_t *other, edict_t *activator);
void	ShowGun (edict_t *ent);
void	FlagPathTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

extern char	respawn_bots[64][256];

// CTF stuff
extern gitem_t *flag1_item;
extern gitem_t *flag2_item;
extern gitem_t *flag3_item; // AJ

extern edict_t *flag1_ent;
extern edict_t *flag2_ent;
extern edict_t *flag3_ent; // AJ

extern gitem_t	*item_tech1, *item_tech2, *item_tech3, *item_tech4, *item_tech5, *item_tech6;
