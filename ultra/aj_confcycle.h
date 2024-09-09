/* Config List prototypes, defines etc */
typedef struct conflist_node
{
	char *filename;
	struct conflist_node *next;
} conflist_node;

void setup_config(void);
void read_configlist(void);

extern conflist_node *config_list, *this_config;

extern	cvar_t	*config_file;
extern	cvar_t	*use_configlist;
