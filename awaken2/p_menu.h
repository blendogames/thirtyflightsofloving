// p_menu.h

enum {
	PMENU_ALIGN_LEFT,
	PMENU_ALIGN_CENTER,
	PMENU_ALIGN_RIGHT
};

typedef struct pmenuhnd_s
{
	struct pmenu_s	*entries;
	int				cur;
	int				num;
	void			*arg;
} pmenuhnd_t;

typedef void (*SelectFunc_t)(edict_t *ent, pmenuhnd_t *hnd);

typedef struct pmenu_s
{
	char			*text;
	int				align;
	SelectFunc_t	SelectFunc;
} pmenu_t;

pmenuhnd_t *PMenu_Open(edict_t *ent, pmenu_t *entries, int cur, int num, void *arg);
void PMenu_Close(edict_t *ent);
void PMenu_UpdateEntry(pmenu_t *entry, const char *text, int align, SelectFunc_t SelectFunc);
void PMenu_Do_Update(edict_t *ent);
void PMenu_Update(edict_t *ent);
void PMenu_Next(edict_t *ent);
void PMenu_Prev(edict_t *ent);
void PMenu_Select(edict_t *ent);
