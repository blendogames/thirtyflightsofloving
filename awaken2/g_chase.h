// g_chase.h

//CW: This code was kindly sent in by Doug "Raven" Buckley;
//    it was used as part of The Match Mod for Q2.


#define CHASE_FREECAM	0
#define CHASE_CHASECAM	1
#define CHASE_FLOATCAM	2
#define CHASE_EYECAM	3

//CW++
#define CHASE_BUTTCAM	4
//CW--

#define CHASE_FIRSTMODE	CHASE_FREECAM
#define CHASE_LASTMODE	CHASE_BUTTCAM																//CW

void ToggleChaseCam(edict_t *ent, pmenu_t *p);
void SwitchModeChaseCam(edict_t *ent);
void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);
void ChaseRemoveTarget(edict_t *target);
void ChaseHelp(edict_t *ent);
