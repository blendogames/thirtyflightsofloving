/* Scoreboard prototypes */

void Lithium_FragsScoreboardMessage (edict_t *ent, edict_t *killer);
void Lithium_FPHScoreboardMessage (edict_t *ent, edict_t *killer);
void ThreeTeamCTFScoreboardMessage (edict_t *ent, edict_t *killer);

extern	cvar_t	*showbotping;		// show the fake-ping for bots (else display "bot")
extern	cvar_t	*def_scores;         // sets default client scoreboard layout
//                                   0 = old, 1 = by frags, 2 = by FPH