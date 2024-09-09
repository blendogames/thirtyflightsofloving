/* Replace List prototypes etc */

typedef struct replace_details
{
	char *from;
	char *to;
	struct replace_details *next;
} replace_details;

extern replace_details *replace_list;

void Svcmd_Replace(void);
void read_replacelist(void);
gitem_t *lithium_replace_item(gitem_t *item);
void aj_onegun(int weapon);
void Svcmd_OneGun(void);
void Clcmd_Replace(void);

extern	cvar_t	*use_replacelist;
extern	cvar_t	*replace_file;
