/* Statusbar (HUD) prototypes+defines */

extern char *lithium_statusbar;

#define STAT_LITHIUM_FPH		28
#define STAT_LITHIUM_PLAYERS	29
#define	STAT_LITHIUM_PLACE		30
#define STAT_LITHIUM_RUNE		31
#define	STAT_LITHIUM_MODE		16

/*
#define STAT_CTF_TEAM3_PIC		28
#define STAT_CTF_TEAM3_HEADER	29
#define	STAT_CTF_TEAM3_CAPS		30
#define STAT_CTF_JOINED_TEAM3_PIC 31
*/

#define CS_TECH1	CS_GENERAL
#define CS_TECH2	CS_TECH1+1
#define	CS_TECH3	CS_TECH1+2
#define	CS_TECH4	CS_TECH1+3
#define	CS_TECH5	CS_TECH1+4
#define	CS_TECH6	CS_TECH1+5
#define	CS_TECHNONE	CS_TECH1+6
#define	CS_SAFETY	CS_TECHNONE+1
#define	CS_OBSERVER	CS_SAFETY+1

void lithium_updatestats(edict_t *client);
void aj_choosebar(void);
int getammocount (edict_t *ent, int index);
int getammoicon (int index);
void sortammo (edict_t *ent, int table2 [6][2]);


extern	cvar_t	*def_hud;				// sets default client HUD
										// 0 = none, 1 = normal, 2 = lithium, 3 = ammo
extern	cvar_t	*intermission_time;		// minimum time the intermission will last
extern	cvar_t	*intermission_sound;	// sound to play during intermission
