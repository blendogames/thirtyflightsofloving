// g_team.h

#define STAT_CTF_TEAM1_PIC			17
#define STAT_CTF_TEAM1_CAPS			18
#define STAT_CTF_TEAM2_PIC			19
#define STAT_CTF_TEAM2_CAPS			20
#define STAT_CTF_FLAG_PIC			21		//CW: (ab)used as an attacking team indicator for Assault
#define STAT_CTF_JOINED_TEAM1_PIC	22
#define STAT_CTF_JOINED_TEAM2_PIC	23
#define STAT_CTF_TEAM1_HEADER		24
#define STAT_CTF_TEAM2_HEADER		25
#define STAT_CTF_TECH				26
#define STAT_CTF_ID_VIEW			27
#define STAT_CTF_MATCH				28
#define STAT_CTF_ID_VIEW_COLOR		29
#define STAT_CTF_TEAMINFO			30
//CW++
#define STAT_ASLT_ATTACK2			31

#define CLIENT_ID_CHECKTIME			0.5
//CW--

#define CONFIG_CTF_MATCH (CS_AIRACCEL-1)
#define CONFIG_CTF_TEAMINFO	(CS_AIRACCEL-2)

typedef enum
{
	CTF_NOTEAM,
	CTF_TEAM1,
	CTF_TEAM2,
//CW++
	CTF_TEAM_FFA
//CW--
} ctfteam_t;

typedef enum
{
	CTF_GRAPPLE_STATE_FLY,
	CTF_GRAPPLE_STATE_PULL,
	CTF_GRAPPLE_STATE_HANG
} ctfgrapplestate_t;

//CW++
typedef enum match_s
{
	MATCH_NONE,
	MATCH_SETUP,
	MATCH_PREGAME,
	MATCH_GAME,
	MATCH_POST
} match_t;
//CW--

typedef struct ghost_s
{
	char	netname[MAX_NAMELEN];																	//CW
	int		number;

	int		deaths;						// stats
	int		kills;						//
	int		caps;						//
	int		basedef;					//
	int		carrierdef;					//

	int		code;						// ghost code
	int		team;						// team
	int		score;						// frags at time of disconnect
	edict_t	*ent;
} ghost_t;

typedef struct teamgame_s																			//CW
{
	int		team1;
	int		team2;
	int		total1;						// only set when going into intermission!
	int		total2;						// only set when going into intermission!
	float	last_flag_capture;
	int		last_capture_team;

	match_t	match;						// match state
	float	matchtime;					// time for match start/end (depends on state)
	int		lasttime;					// last time update
	qboolean countdown;					// has audio countdown started?

	int		warnactive;					// true if stat string 30 is active

	ghost_t ghosts[MAX_CLIENTS]; 		// ghost codes

//CW++
	int		frags1;						// scores for TDM
	int		frags2;						//
//CW--
} teamgame_t;																						//CW

//CW++
// Assault game info (NB: teamgame_t is still used for most of the relevant team data).
typedef struct asltgame_s
{
	qboolean	victory;				// attacking team has achieved objective(s)
	int			t_attack;				// attacking team
	int			spawn;					// current spawn point selection
	char		*msg_attack;			// message displayed if the attackers win the level
	char		*msg_defend;			// message displayed if the defenders win the level
} asltgame_t;
//CW--

#define CTF_TEAM1_SKIN "ctf_r"
#define CTF_TEAM2_SKIN "ctf_b"

#define CTF_CAPTURE_BONUS					15	// what you get for capture
#define CTF_TEAM_BONUS						10	// what your team gets for capture
#define CTF_RECOVERY_BONUS					1	// what you get for recovery
#define CTF_FLAG_BONUS						0	// what you get for picking up enemy flag
#define CTF_FRAG_CARRIER_BONUS				2	// what you get for fragging enemy flag carrier
#define CTF_FLAG_RETURN_TIME				40	// seconds until auto return

#define CTF_CARRIER_DANGER_PROTECT_BONUS	2	// bonus for fragging someone who has recently hurt your flag carrier
#define CTF_CARRIER_PROTECT_BONUS			1	// bonus for fragging someone while either you or your target are near your flag carrier
#define CTF_FLAG_DEFENSE_BONUS				1	// bonus for fragging someone while either you or your target are near your flag
#define CTF_RETURN_FLAG_ASSIST_BONUS		1	// awarded for returning a flag that causes a capture to happen almost immediately
#define CTF_FRAG_CARRIER_ASSIST_BONUS		2	// award for fragging a flag carrier if a capture happens almost immediately

#define CTF_TARGET_PROTECT_RADIUS			400	// the radius around an object being defended where a target will be worth extra frags
#define CTF_ATTACKER_PROTECT_RADIUS			400	// the radius around an object being defended where an attacker will get extra frags when making kills

#define CTF_CARRIER_DANGER_PROTECT_TIMEOUT	8
#define CTF_FRAG_CARRIER_ASSIST_TIMEOUT		10
#define CTF_RETURN_FLAG_ASSIST_TIMEOUT		10

#define CTF_AUTO_FLAG_RETURN_TIMEOUT		30	// number of seconds before dropped flag auto-returns

#define CTF_TECH_TIMEOUT					60  // seconds before techs spawn again


void CTFInit(void);
void CTFSpawn(void);
void CTFPrecache(void);

void SP_info_player_team1(edict_t *self);
void SP_info_player_team2(edict_t *self);

char *CTFTeamName(int team);
char *CTFOtherTeamName(int team);
void CTFAssignSkin(edict_t *ent, char *s);
void CTFAssignTeam(gclient_t *who);
edict_t *SelectCTFSpawnPoint (edict_t *ent, qboolean ctf_only);										//CW
qboolean CTFPickup_Flag(edict_t *ent, edict_t *other);
void CTFDrop_Flag(edict_t *ent, gitem_t *item);														//CW
void CTFEffects(edict_t *player);
void CTFCalcScores(void);
void SetCTFStats(edict_t *ent);
void CTFDeadDropFlag(edict_t *self);
void CTFScoreboardMessage (edict_t *ent, edict_t *killer);
void CTFTeam_f (edict_t *ent);
void CTFID_f (edict_t *ent);
void CTFSay_Team(edict_t *who, char *msg);
void CTFFlagSetup (edict_t *ent);
void CTFResetFlag(int ctf_team);
void CTFFragBonuses(edict_t *targ, edict_t *inflictor, edict_t *attacker);
void CTFCheckHurtCarrier(edict_t *targ, edict_t *attacker);

// GRAPPLE
//CW++
void CTFWeapon_Grapple_OffHand(edict_t *self);
void CTFResetAllPlayers(void);
//CW--

void CTFWeapon_Grapple (edict_t *ent);
void CTFPlayerResetGrapple(edict_t *ent);
void CTFGrapplePull(edict_t *self);
void CTFResetGrapple(edict_t *self);

//TECH
gitem_t *CTFWhat_Tech(edict_t *ent);
qboolean CTFPickup_Tech (edict_t *ent, edict_t *other);
void CTFDrop_Tech(edict_t *ent, gitem_t *item);
void CTFDeadDropTech(edict_t *ent);
void CTFSetupTechSpawn(void);
int CTFApplyResistance(edict_t *ent, int dmg);
int CTFApplyStrength(edict_t *ent, int dmg);
qboolean CTFApplyStrengthSound(edict_t *ent);
qboolean CTFApplyHaste(edict_t *ent);
void CTFApplyHasteSound(edict_t *ent);
void CTFApplyRegeneration(edict_t *ent);
qboolean CTFHasRegeneration(edict_t *ent);
void CTFRespawnTech(edict_t *ent);
void CTFResetTech(void);

void CTFOpenJoinMenu(edict_t *ent);
qboolean CTFStartClient(edict_t *ent);
void CTFVoteYes(edict_t *ent);
void CTFVoteNo(edict_t *ent);
void CTFReady(edict_t *ent);
void CTFNotReady(edict_t *ent);
qboolean CTFNextMap(void);
qboolean CTFMatchSetup(void);
qboolean CTFMatchOn(void);
void CTFGhost(edict_t *ent);
qboolean CTFInMatch(void);
void CTFStats(edict_t *ent);
void CTFBoot(edict_t *ent, qboolean ban);															//CW
void CTFPlayerList(edict_t *ent);

qboolean CTFCheckRules(void);

void SP_misc_ctf_banner (edict_t *ent);
void SP_misc_ctf_small_banner (edict_t *ent);

extern char *ctf_statusbar;

void CTFObserver(edict_t *ent);

void SP_trigger_teleport (edict_t *ent);
void SP_info_teleport_destination (edict_t *ent);

void CTFSetPowerUpEffect(edict_t *ent, int def);

//CW++
extern	teamgame_t	teamgame;
extern	asltgame_t	asltgame;

//======================================================================
// Team-DM stuff
//======================================================================
extern char *tdm_statusbar;

void TDMPrecache(void);
void TDMScoreboardMessage(edict_t *ent, edict_t *killer);

//======================================================================
// Assault stuff
//======================================================================
#define LINESIZE			23
#define MAX_SPAWNS			32

extern char *aslt_statusbar;

void ASLTPrecache(void);
void ASLTSpawn(void);
void ASLTShowMission(edict_t *ent);
edict_t *SelectASLTSpawnPoint (edict_t *ent);
qboolean ASLTCheckRules(void);
//CW--
