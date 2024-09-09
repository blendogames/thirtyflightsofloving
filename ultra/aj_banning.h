/* IP Banning prototypes etc */
typedef struct banlist_node
{
	char *ip;
	struct banlist_node *next;
} banlist_node;

extern banlist_node *ban_list;
extern cvar_t	*use_iplogging;
int check_ip (char *player_ip);
void read_banlist (void);

extern	cvar_t	*banlist;            // sets plain text file to read for IP banning
