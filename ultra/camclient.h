#define MAX_VISIBLE_RANGE   1000
#define CAMERA_SWITCH_TIME  20
#define CAMERA_DEAD_SWITCH_TIME 2.5
#define TRUE 1
#define FALSE 0

void CameraThink (edict_t *ent, usercmd_t *ucmd);
void PlayerDied (edict_t *ent);

typedef struct sPlayerList_s
{
    edict_t     *pEntity;
    struct sPlayerList_s *pNext;
} sPlayerList;

int CameraCmd (edict_t *ent);
void EntityListRemove( edict_t *pEntity);
void EntityListAdd (edict_t *pEntity);
unsigned long EntityListNumber ();
void PrintEntityList (void);
void EnitityListClean (void);

// Knightmare- made these vars extern to fix GCC compile
extern  sPlayerList *EntityListHead();
extern  sPlayerList *EntityListNext(sPlayerList *pCurrent);
extern  sPlayerList *pTempFind;
