void	OptimizeRouteCache (void);
void	CalcRoutes (int node_index);
int		ClosestNodeToEnt (edict_t *self, int check_fullbox, int check_all_nodes);
float	PathToEnt (edict_t *self, edict_t *target, int check_fullbox, int check_all_nodes);

// Knightmare- made these vars extern to fix GCC compile
extern	edict_t	*PathToEnt_Node;		// self's goal node, to get to target, from last PathToEnt() call
extern	edict_t	*PathToEnt_TargetNode;	// target's goal node, to get FROM self, from last PathToEnt() call

void	Debug_ShowPathToGoal (edict_t	*self, edict_t	*goalent);

edict_t *matching_trail (vec3_t spot);
void	AddTrailToPortals (edict_t	*trail);
int		GetGridPortal (float pos);

void	NodeDebug (char *fmt, ...);
void	CheckMoveForNodes (edict_t *ent);
void	CalcItemPaths (edict_t *ent);

// Knightmare- made these vars extern to fix GCC compile
extern	int		trail_head, last_head;

extern	int		dropped_trail;
extern	float	last_optimize;
extern	int		optimize_marker;

#define	NODE_NORMAL		0
#define	NODE_PLAT		1
#define	NODE_LANDING	2		// jump destination node (cannot be seen from any other nodes other than the jumping node)
#define NODE_BUTTON		3
#define NODE_TELEPORT	4
#define NODE_GRAPPLE	5		// set when this jump node is a grapple start

#define	TRAIL_PORTAL_SUBDIVISION	24
#define	MAX_TRAILS_PER_PORTAL		196
#define	MAX_MAP_AXIS				5000

// Knightmare- made this var extern to fix GCC compile
extern	int		trail_portals[TRAIL_PORTAL_SUBDIVISION+1][TRAIL_PORTAL_SUBDIVISION+1][MAX_TRAILS_PER_PORTAL];
// contains a list of trails grouped by grid blocks, in the X/Y plane, with
// size 512x512 (covers the entire map)

// each trail can be a member of up to 4 portals, so as to somewhat "blur" the
// line between nodes, so that selection of a ClosestNodeToEnt is less likely
// to be restricted by being close to a boundary

// Knightmare- made this var extern to fix GCC compile
extern	int		num_trail_portals[TRAIL_PORTAL_SUBDIVISION+1][TRAIL_PORTAL_SUBDIVISION+1];
// keep track of the total nodes in each trail_portal

typedef struct ctf_item_s
{
	char	classname[64];
	vec3_t	origin;
	vec3_t	angles;
	struct ctf_item_s	*next;
} ctf_item_t;

// Knightmare- made this var extern to fix GCC compile
extern	ctf_item_t	*ctf_item_head;
