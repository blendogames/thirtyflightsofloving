#include "g_local.h"
#include "p_trail.h"
#include "bot_procs.h"
#include "g_items.h"

//-------------------------------------------------------------------------

#define TRAIL_DIRECT_ZIP_READ	// whether to use direct reads to zipped route files
#define TRAIL_DIRECT_ZIP_WRITE	// whether to use direct writes to zipped route files

#ifdef TRAIL_DIRECT_ZIP_READ
#define TR_FRead(d, s, c, f)	G_FRead(d, s, f)
#else	// TRAIL_DIRECT_ZIP_READ
#define TR_FRead(d, s, c, f)	fread(d, s, c, f)
#endif	// TRAIL_DIRECT_ZIP_READ

#ifdef TRAIL_DIRECT_ZIP_WRITE
#define	TR_FWrite(d, s, c, f)	G_FWrite(d, s, f)
#else	// TRAIL_DIRECT_ZIP_WRITE
#define	TR_FWrite(d, s, c, f)	fwrite(d, s, c, f)
#endif	// TRAIL_DIRECT_ZIP_WRITE

//-------------------------------------------------------------------------
// Function declarations
void botButtonThink (edict_t *ent);

void WriteTrail (void);
void ReadTrail (void);
void FlagPath (edict_t *ent, int ctf_team);
void ED_CallSpawn (edict_t *ent);

//-------------------------------------------------------------------------
// Data declarations
const int TRAIL_VERSION = 13;

char *flag_path_src_classname = "flag_path_src";
char *misc_teleporter_classname = "misc_teleporter";
char *func_plat_classname = "func_plat";
char *func_button_classname = "func_button";
char *redflag_classname = "redflag";
char *blueflag_classname = "blueflag";
char *item_flag_team1_classname = "item_flag_team1";
char *item_flag_team2_classname = "item_flag_team2";
char *path_beam_classname = "path_beam";
char *player_duck_classname = "player_duck";
char *player_jump_classname = "player_jump";

#ifdef _WIN32
#include <direct.h>
#define PATH_SEP "/"
#define mkdir(n, m) _mkdir(n)
#define chdir _chdir
#else
#include <unistd.h>
#include <sys/stat.h>
#define PATH_SEP "/"
#endif

const char *up_dir = "..";
const char *relative_dir = "." PATH_SEP;
const char *dir_sep = PATH_SEP;
const char *routes_dir = PATH_SEP "routes";
const char *routes_dir_no_sep = "routes";
/*const*/ char *relative_routes_dir_sep = "." PATH_SEP "routes" PATH_SEP;
/*const*/ char *relative_routes_dir = "." PATH_SEP "routes";
const char *routes_dir_sep = PATH_SEP "routes" PATH_SEP;

const char *route_ext = ".rt3";
const char *route_ext_zipped = ".rtz";
const char *route_ext_zipped_old = "_rt.zipped";
const char *zip_ext = ".zip";

//char *note_grapple_enabled = "............................\nMap contains grapple nodes..\nGrapple-Hook has been ENABLED\n............................\n";
char *debug_no_path_fmt = "Debug_ShowPathToGoal: no path to %s\n";
char *debug_path_length = "PathToEnt: %f units\n";

qboolean	trail_active;
int			dst;
int			src;
float		last_looped_warning;
float		last_fix_break;
qboolean	reading_trail;
float		last_laser_route;
qboolean	notify_force_grapple;
int			trail_version;
int			recursion_count;
int			trail_portals[TRAIL_PORTAL_SUBDIVISION+1][TRAIL_PORTAL_SUBDIVISION+1][MAX_TRAILS_PER_PORTAL];
int			num_trail_portals[TRAIL_PORTAL_SUBDIVISION+1][TRAIL_PORTAL_SUBDIVISION+1];

// Knightmare- moved these vars here to fix GCC compile
edict_t	*the_client;			// points to the first client to enter the game (used for debugging)
float	last_trail_time;

qboolean nodes_done;		// used to determine whether or not to enable node calculation
edict_t	*check_nodes_done;	// after 20 mins of play, this ent checks if we should turn off node calc. at set intervals

//edict_t *last_trail_dropped;
qboolean	loaded_trail_flag;

edict_t		*trail[TRAIL_LENGTH];			// the actual trail!

edict_t	*PathToEnt_Node;		// self's goal node, to get to target, from last PathToEnt() call
edict_t	*PathToEnt_TargetNode;	// target's goal node, to get FROM self, from last PathToEnt() call

int		trail_head, last_head;

int		dropped_trail;
float	last_optimize;
int		optimize_marker;

int		trail_portals[TRAIL_PORTAL_SUBDIVISION+1][TRAIL_PORTAL_SUBDIVISION+1][MAX_TRAILS_PER_PORTAL];

int		num_trail_portals[TRAIL_PORTAL_SUBDIVISION+1][TRAIL_PORTAL_SUBDIVISION+1];

ctf_item_t	*ctf_item_head;

// end Knightmare


// These are the vars assumed to be available globally.
extern float last_optimize;
extern edict_t *weapons_head;
extern qboolean nodes_done;
extern qboolean loaded_trail_flag;
//extern edict_t *the_client;
extern edict_t *health_head;
extern edict_t *PathToEnt_Node;
//extern float last_trail_time;
extern ctf_item_t *ctf_item_head;
extern int last_head;
extern int num_players;
extern edict_t *ammo_head;
extern int trail_head;
extern edict_t *trail[TRAIL_LENGTH];
extern edict_t *PathToEnt_TargetNode;
extern edict_t *bonus_head;
extern edict_t *players[MAX_CLIENTS];
extern int dropped_trail;
extern level_locals_t level;
extern game_import_t gi;
extern edict_t *g_edicts;
extern cvar_t *bot_calc_nodes;
extern cvar_t *bot_debug_nodes;
extern cvar_t *bot_optimize;
extern cvar_t *ctf;

//-------------------------------------------------------------------------

void PlayerTrail_Init (void)
{
	int		i, k;
	edict_t *e = NULL;

	for (i = 0; i < TRAIL_LENGTH; ++i)
	{
		e = trail[i] = gi.TagMalloc(sizeof(edict_t), TAG_LEVEL);
		e->classname = "player_trail";
		VectorSet (e->mins, -16.f, -16.0f, -24.0);
		VectorSet (e->maxs, 16.f, 16.f, 32.f);
		e->trail_index = i;
		e->routes = gi.TagMalloc(sizeof(routes_t), TAG_LEVEL);

		memset (e->paths, -1, sizeof(e->paths));

		for (k = 0; k < TRAIL_LENGTH; ++k)
			e->routes->route_path[k] = -1;
	}
	memset (num_trail_portals, 0, sizeof(num_trail_portals));

	trail_head = 0;
	trail_active = 1;
	dropped_trail = 0;
	last_optimize = level.time;
	ctf_item_head = NULL;

	ReadTrail ();
}

edict_t *matching_trail (vec3_t a1)
{
	int		i;
	int		v4 = GetGridPortal(a1[0]);
	int		v2 = GetGridPortal(a1[1]);

	for (i = 0; i < num_trail_portals[v4][v2]; ++i)
		if ( VectorCompare(a1, trail[trail_portals[v4][v2][i]]->s.origin) )
			return trail[trail_portals[v4][v2][i]];

	return NULL;
}

void NodeDebug (char *fmt, ...)
{
	static char buffer[1024];
	va_list args;

	if (!bot_debug_nodes->value || reading_trail)
		return;

	va_start(args, fmt);
//	vsprintf(buffer, fmt, args);
	Q_vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	gi.dprintf ("%s", buffer);
}

void PlayerTrail_Add (edict_t *self, vec3_t spot, edict_t *goalent, int nocheck, int calc_routes, int node_type)
{
	trace_t	tr;
	vec3_t	end;
	edict_t	*e = NULL;

	if (!trail_active)
		return;

	if (trail_head >= TRAIL_LENGTH - 1)
	{
		gi.dprintf ("Reach trail limit, unable to create more nodes.\n");
		gi.cvar_set("bot_calc_nodes", "0");
		return;
	}

	if (nocheck && matching_trail(spot))
	{
		NodeDebug ("Tried to drop duplicate node, aborted\n");
		return;
	}

	/*edict_t **/e = trail[trail_head];

	NodeDebug ("Dropping trail - %i\n", trail_head);

	VectorCopy (spot, e->s.origin);
	e->timestamp = level.time + 0.1;
	e->s.angles[1] = self->s.angles[1];

	if (self != g_edicts)
	{
		VectorCopy (self->mins, e->mins);
		VectorCopy (self->maxs, e->maxs);
	}

	e->viewheight = ((e->maxs[2] - e->mins[2]) * 2.0 / 3.0) - 17.0;
	e->waterlevel = self->waterlevel;
	VectorCopy (self->velocity, e->velocity);
	e->goalentity = goalent;
	e->node_type = node_type;

	VectorCopy (e->s.origin, end);
	end[2] -= 8.f;

	tr = gi.trace(e->s.origin, e->mins, e->maxs, end, e, MASK_PLAYERSOLID);
	e->groundentity = tr.ent;

	PlayerTrail_FindPaths (trail_head);

	if (calc_routes)
		CalcRoutes (trail_head);

	FindVisibleItemsFromNode (e);

	AddTrailToPortals (e);

	self->last_trail_dropped = e;
	dropped_trail = 1;
	last_head = trail_head++;
}

void PlayerTrail_New (vec3_t spot)
{
	if (trail_active)
	{
		PlayerTrail_Init ();
		PlayerTrail_Add (g_edicts, spot, NULL, 0, 1, NODE_NORMAL);
	}
}

edict_t *PlayerTrail_PickFirst (edict_t *self)
{
	int		i, t;

	if (!trail_active)
		return NULL;

	/*int*/ t = trail_head;

	for (/*int*/ i = TRAIL_LENGTH; i && trail[t]->timestamp <= self->monsterinfo.trail_time; --i)
		t = (t + 1) & (TRAIL_LENGTH - 1);

	if ( !visible(self, trail[t]) )
		if ( visible(self, trail[(t - 1) & (TRAIL_LENGTH - 1)]) )
			return trail[(t - 1) & (TRAIL_LENGTH - 1)];

	return trail[t];
}

edict_t *PlayerTrail_PickNext (edict_t *self)
{
	int		i, t;

	if (!trail_active)
		return NULL;

	/*int*/ t = trail_head;

	for (/*int*/ i = TRAIL_LENGTH; i && trail[t]->timestamp <= self->monsterinfo.trail_time; --i)
		t = (t + 1) & (TRAIL_LENGTH - 1);

	return trail[t];
}

edict_t *PlayerTrail_LastSpot (void)
{
	return trail[last_head];
}

int GetGridPortal (float pos)
{
	if (pos > MAX_MAP_AXIS)
		pos = MAX_MAP_AXIS - 1;
	else if (pos < -MAX_MAP_AXIS)
		pos = -(MAX_MAP_AXIS - 1);

	return (int)floor(pos / MAX_MAP_AXIS * 12.0 + 12.0 + 0.5);
}

void AddTrailToPortals (edict_t *trail)
{
	int x = GetGridPortal(trail->s.origin[0]);
	int y = GetGridPortal(trail->s.origin[1]);
	int num, v1, v2;

	trail_portals[x][y][num_trail_portals[x][y]++] = trail->trail_index;

	if (x >= trail->s.origin[0] / MAX_MAP_AXIS * 12.0 + 12.0)
	{
		v1 = -1;
		if (x - 1 < 0)
			v1 = 0;
		else
			trail_portals[x - 1][y][num_trail_portals[x - 1][y]++] = trail->trail_index;
	}
	else
	{
		v1 = 1;
		if (x + 1 >= TRAIL_PORTAL_SUBDIVISION)
			v1 = 0;
		else
			trail_portals[x + 1][y][num_trail_portals[x + 1][y]++] = trail->trail_index;
	}

	if (y >= trail->s.origin[1] / MAX_MAP_AXIS * 12.0 + 12.0)
	{
		v2 = -1;
		if (y - 1 < 0)
			v2 = 0;
		else
			trail_portals[x - 1][y + TRAIL_PORTAL_SUBDIVISION][num_trail_portals[x - 1][y + TRAIL_PORTAL_SUBDIVISION]++] = trail->trail_index;
	}
	else
	{
		v2 = 1;
		if (y + 1 >= TRAIL_PORTAL_SUBDIVISION)
			v2 = 0;
		else
			trail_portals[x][y + 1][num_trail_portals[x][y + 1]++] = trail->trail_index;
	}

	if (!v1 || !v2)
		return;

	/*int*/ num = num_trail_portals[v1][(TRAIL_PORTAL_SUBDIVISION + 1) * x + v2 + y]++;
	trail_portals[x][(TRAIL_PORTAL_SUBDIVISION + 1) * v1][MAX_TRAILS_PER_PORTAL * v2 + MAX_TRAILS_PER_PORTAL * y + num] = trail->trail_index;
}

void PlayerTrail_FindPaths (int marker)
{
	int		j, k, x, y, num_trails, trail_index;
	int		path_index = 0;
	edict_t *ent1 = trail[marker];
	edict_t *ent2 = NULL;

	memset (ent1->paths, -1, sizeof(ent1->paths));

	if (ent1->goalentity)
	{
		ent1->paths[0] = ent1->goalentity->trail_index;
		path_index = 1;
	}

	/*int*/ x = GetGridPortal(ent1->s.origin[0]);
	/*int*/ y = GetGridPortal(ent1->s.origin[1]);

	/*int*/ num_trails = num_trail_portals[x][y];

	NodeDebug ("num_trails - %i\n", num_trails);

	for (/*int*/ j = 0; j < num_trails; ++j)
	{
		/*int*/ trail_index = trail_portals[x][y][j];
		/*edict_t **/ent2 = trail[trail_index];

		if ( trail_index != marker
			&& (!ent1->goalentity || ent1->goalentity->trail_index != trail_index)
			&& (!ent2->goalentity || ent2->goalentity->trail_index != marker)
			&& (!ent1->goalentity && !ent2->goalentity && ent2->node_type != NODE_LANDING
				|| fabs(ent1->s.origin[2] - ent2->s.origin[2]) <= 48.0)
			&& (entdist(ent1, ent2) <= 512.0)
			&& visible_box(ent2, ent1)
			&& CanReach(ent2, ent1) )
		{
			if (ent2->node_type != NODE_LANDING && ent2->timestamp != ent1->timestamp)
				ent1->paths[path_index++] = trail_index;

			if (ent1->node_type != NODE_LANDING)
			{
			//	int k;

				for (k = 0; k < MAX_PATHS && ent2->paths[k] >= 0; ++k)
					;

				if (k < MAX_PATHS)
					ent2->paths[k] = marker;
			}

			if (path_index >= MAX_PATHS)
				break;
		}
	}
	NodeDebug ("FindPaths: %i paths found\n", path_index);
}

int ClosestNodeToEnt (edict_t *self, int check_fullbox, int check_all_nodes)
{
	int		i, x, y, num_portals, portal_index;
	int		best_node = -1;
	float	best_dist = 999999.0;

	if (self->item || self->touch == FlagPathTouch)
	{
		if (self->target_ent)
			return self->target_ent->trail_index;

		for (/*int*/ i = 0; i < MAX_PATHS && self->paths[i] != -1; ++i)
		{
			float dist = entdist(self, trail[self->paths[i]]);

			if (dist < best_dist)
			{
				best_dist = dist;
				best_node = self->paths[i];
			}
		}

		if (best_node > -1)
		{
			self->target_ent = trail[best_node];
			return best_node;
		}
	}

	if ((!bot_calc_nodes->value || self->bot_client) && level.time - 0.2 < self->last_closest_time)
		return self->last_closest_node;

	self->last_closest_node = -1;

	if (!(bot_calc_nodes->value && self->client && !self->bot_client || check_all_nodes))
	{
		if (level.time - 0.2 < self->closest_trail_time)
		{
			self->last_closest_node = self->closest_trail;
			self->last_closest_time = level.time;
			return self->closest_trail;
		}
		else if ( self->closest_trail > -1
			&& trail[self->closest_trail]->ignore_time <= level.time
			&& trail[self->closest_trail]->waterlevel == self->waterlevel
			&& (entdist(self, trail[self->closest_trail]) <= 128.0)
			&& (gi.trace(
				self->s.origin,
				self->mins,
				self->maxs,
				trail[self->closest_trail]->s.origin,
				self,
				MASK_PLAYERSOLID).fraction >= 1.0) )
		{
			self->last_closest_node = self->closest_trail;
			self->last_closest_time = level.time;
			self->closest_trail_time = level.time;
			return self->closest_trail;
		}
	}

	self->closest_trail = -1;
	self->closest_trail_time = 0.0;

	/*int*/ x = GetGridPortal(self->s.origin[0]);
	/*int*/ y = GetGridPortal(self->s.origin[1]);
	/*int*/ num_portals = num_trail_portals[x][y];
	/*int*/ portal_index = 0;

	if (self->bot_client && num_portals > 30)
	{
		portal_index = random() * (num_portals - 30);
		num_portals = portal_index + 30;
	}

	while (portal_index < num_portals)
	{
		int node_index = trail_portals[x][y][portal_index];
		edict_t *ent1 = trail[node_index];

		if (ent1 != self && ent1->ignore_time <= level.time)
		{
			float dist = entdist(self, ent1);

			if (dist <= best_dist && (check_fullbox || visible_box(ent1, self)) && (!check_fullbox || visible_fullbox(ent1, self)))
			{
				if (CanReach(self, ent1))
				{
					best_node = node_index;
					best_dist = dist;
					if (dist < 64.0)
						break;
				}
			}
		}

		++portal_index;
	}

	if (best_node > -1 && self->item)
		self->target_ent = trail[best_node];

	if (self->client)
	{
		self->closest_trail = best_node;
		self->closest_trail_time = level.time;
		self->last_closest_node = best_node;
		self->last_closest_time = level.time;
	}

	return best_node;
}

float PathToEnt (edict_t *self, edict_t *target, int check_fullbox, int check_all_nodes)
{
	qboolean	self_has_routes;
	qboolean	target_has_routes;
	int			trail_index, closest_node;
	unsigned short	dist, v8;

	if (!target)
		return -1.0;

	/*qboolean*/ self_has_routes = self->routes != NULL;
	/*qboolean*/ target_has_routes = target->routes != NULL;

	if (target->routes)
		target_has_routes = true;

	else if (entdist(self, target) < 400.0
		&& (check_fullbox && visible_fullbox(self, target) || !check_fullbox && visible_box(self, target))
		&& CanReach(self, target)
		&& (!bot_calc_nodes->value || !target->client || target->bot_client || entdist(self, target) < 64.0))
	{
		PathToEnt_TargetNode = target;
		PathToEnt_Node = target;
		return entdist(self, target);
	}

	/*int*/ trail_index = -1;

	if (target_has_routes)
		trail_index = target->trail_index;

	if (trail_index < 0)
	{
		trail_index = ClosestNodeToEnt(target, check_fullbox, 0);
		if (trail_index == -1)
			return -1.0;
	}

	if (self_has_routes && self->routes->route_path[trail_index] > -1)
	{
		PathToEnt_Node = trail[self->routes->route_path[trail_index]];
		PathToEnt_TargetNode = trail[trail_index];
		/*unsigned short*/ dist = self->routes->route_dist[trail_index];
		return entdist(self, trail[trail_index]) + dist;
	}

	/*int*/ closest_node = ClosestNodeToEnt(self, check_fullbox, 1);

	if (closest_node == -1 ||
		trail[closest_node]->routes->route_path[trail_index] == -1)
		return -1.0;

	/*unsigned short*/ v8 = trail[closest_node]->routes->route_dist[trail_index];
	PathToEnt_Node = trail[closest_node];
	PathToEnt_TargetNode = trail[trail_index];
	return v8;
}

void CalcRoutes (int node_index)
{
	int		i, j;
	short	dist;
	char	Str[1024] = { 0 };

	for (/*int*/ i = 0; i < TRAIL_LENGTH && trail[i]->timestamp != 0.0; ++i)
	{
		edict_t *trail_ni = trail[node_index];
		edict_t *trail_i = trail[i];

		if (i != node_index)
		{
			edict_t		*ent1 = NULL;
			unsigned short best_dist = 99999;
			routes_t	*ni_routes = NULL;
			routes_t	*i_routes = NULL;

			for (/*int*/ j = 0; j < MAX_PATHS && trail_ni->paths[j] != -1; ++j)
			{
				vec3_t vec;

				if (trail_ni->paths[j] == i)
				{
					VectorSubtract (trail_i->s.origin, trail_ni->s.origin, vec);
					ent1 = trail_i;
					best_dist = VectorLength(vec);
					break;
				}

				if (trail[trail_ni->paths[j]]->routes->route_path[i] != -1)
				{
					VectorSubtract (trail[trail_ni->paths[j]]->s.origin, trail_ni->s.origin, vec);
					/*short*/ dist = VectorLength(vec) + trail[trail_ni->paths[j]]->routes->route_dist[i];

					if (ent1)
					{
						if (dist < best_dist)
						{
							ent1 = trail[trail_ni->paths[j]];
							best_dist = dist;
						}
					}
					else
					{
						ent1 = trail[trail_ni->paths[j]];
						best_dist = dist;
					}
				}
			}

			/*routes_t **/ni_routes = trail_ni->routes;
			/*routes_t **/i_routes = trail_i->routes;

			if (ent1)
			{
				qboolean found = false;

				ni_routes->route_path[i] = ent1->trail_index;
				ni_routes->route_dist[i] = best_dist;

				if (i == ent1->trail_index && ent1->paths[0] > -1)
				{
					for (/*int*/ j = 0; j < MAX_PATHS && ent1->paths[j] != -1; ++j)
					{
						if (trail[ent1->paths[j]]->trail_index == node_index)
						{
							found = true;
							break;
						}
					}
				}

				if (found)
				{
					i_routes->route_path[node_index] = node_index;
					i_routes->route_dist[node_index] = best_dist;
				}
				else if (i_routes->route_path[ent1->trail_index] <= -1)
				{
					i_routes->route_path[node_index] = -1;
					i_routes->route_dist[node_index] = -2536;
				}
				else
				{
					i_routes->route_path[node_index] = i_routes->route_path[ent1->trail_index];
					i_routes->route_dist[node_index] = entdist(ent1, trail_i) + i_routes->route_dist[ent1->trail_index];
				}
			}
			else
			{
				ni_routes->route_path[i] = -1;
				ni_routes->route_dist[i] = -2536;

				if (bot_debug_nodes->value)
				{
					if (strlen(Str) >= 0x100)
					{
						if (Str[strlen(Str)] != '.')
							Com_strcat (Str, sizeof(Str), "..");
					}
					else
						Com_strcat (Str, sizeof(Str), va("%i ", i));
				}
			}
		}
	}

	if (strlen(Str))
	{
		NodeDebug ("Node %i has no route to nodes..\n(%s)\n", node_index, Str);
		memset (Str, 0, sizeof(Str));
	}
}

void OptimizeRouteCache (void)
{
	int		i, max_it, total_it;

	int optimize_num = bot_optimize->value;

	if (num_players <= 1)
		max_it = 8 * optimize_num;
	else
		max_it = optimize_num / num_players;

	if (!bot_calc_nodes->value)
		max_it /= 3;

	if (max_it < 100)
		max_it = 100;

	/*int*/ total_it = 0;

	while (src < TRAIL_LENGTH)
	{
		if (trail[src]->timestamp == 0.0)
		{
			src = 0;
			dst = 0;
			break;
		}

		if (total_it++ > max_it)
			return;

		while (dst < TRAIL_LENGTH)
		{
			if (trail[dst]->timestamp == 0.0)
			{
				dst = 0;
				break;
			}

			if (trail[src]->routes->route_path[dst] != dst)
			{
				if (trail[src]->routes->route_path[dst] > -1)
				{
					short path_index = trail[trail[src]->routes->route_path[dst]]->routes->route_path[dst];

					if (path_index == src)
					{
						if (trail[trail[src]->routes->route_path[dst]]->goalentity != trail[src])
							trail[trail[src]->routes->route_path[dst]]->routes->route_path[dst] = -1;

						if (trail[src]->goalentity != trail[trail[src]->routes->route_path[dst]])
							trail[src]->routes->route_path[dst] = -1;

						if (level.time - 0.1 > last_looped_warning)
							last_looped_warning = level.time;
					}
					else if (path_index == -1)
					{
						trail[src]->routes->route_path[dst] = -1;
						last_fix_break = level.time;

						if (bot_debug_nodes->value && the_client && num_players <= 1 && level.time - 0.1 > last_optimize)
						{
							gi.WriteByte (svc_layout);
							gi.WriteString ("Fixing routes..");
							gi.unicast (the_client, true);
							last_optimize = level.time;
						}
					}
				}

				if (total_it++ > max_it)
					return;

				if (dst != src && level.time - 2.0 >= last_fix_break && level.time >= 7.0)
				{
					for (/*int*/ i = 0; i < MAX_PATHS && trail[src]->paths[i] != -1; ++i)
					{
						if (trail[src]->paths[i] == dst
							|| trail[trail[src]->paths[i]]->routes->route_path[dst] != -1
							&& trail[trail[src]->paths[i]]->routes->route_path[dst] != src)
						{
							float dist;

							++total_it;

							/*float*/ dist = entdist(trail[src], trail[trail[src]->paths[i]]);

							if (trail[src]->paths[i] != dst)
								dist = trail[trail[src]->paths[i]]->routes->route_dist[dst] + dist;

							if (trail[src]->routes->route_path[dst] == -1
								|| trail[src]->routes->route_dist[dst] > dist + 32.0)
							{
								if (bot_debug_nodes->value && the_client && num_players <= 1 && level.time - 0.1 > last_optimize)
								{
									char layout[1024];

									Com_sprintf (layout, sizeof(layout),
										"xv 10 yv 180 string \"Optimizing node route %i -> %i\" ",
										src, dst);

									gi.WriteByte (svc_layout);
									gi.WriteString (layout);
									gi.unicast (the_client, true);
									last_optimize = level.time;
								}

								trail[src]->routes->route_path[dst] = trail[src]->paths[i];
								trail[src]->routes->route_dist[dst] = dist;
								dropped_trail = 1;
							}
						}
					}
				}
			}

			++dst;
		}

		if (dst == TRAIL_LENGTH)
			dst = 0;

		++src;
	}

	if (src == TRAIL_LENGTH)
		src = 0;
}

#ifdef TRAIL_DIRECT_ZIP_WRITE
edict_t *WriteTrailNode (fileHandle_t trFile, edict_t *node)
#else	// TRAIL_DIRECT_ZIP_WRITE
edict_t *WriteTrailNode (FILE *trFile, edict_t *node)
#endif	// TRAIL_DIRECT_ZIP_WRITE
{
	int		/*path_id,*/ node_id;
	short	b;
	char	path_id;

	if (node->ignore)
		return node;

	TR_FWrite (&node->trail_index, sizeof(node->trail_index), 1u, trFile);

	if (node->goalentity)
		TR_FWrite (&node->goalentity->trail_index, sizeof(node->goalentity->trail_index), 1u, trFile);
	else
	{
		int goal = -1;
		TR_FWrite (&goal, sizeof(goal), 1u, trFile);
	}

	TR_FWrite (node->s.origin, sizeof(node->s.origin), 1u, trFile);
	TR_FWrite (&node->timestamp, sizeof(node->timestamp), 1u, trFile);
	TR_FWrite (node->s.angles, sizeof(node->s.angles), 1u, trFile);
	TR_FWrite (node->mins, sizeof(node->mins), 1u, trFile);
	TR_FWrite (node->maxs, sizeof(node->maxs), 1u, trFile);
	TR_FWrite (node->velocity, sizeof(node->velocity), 1u, trFile);
	TR_FWrite (&node->waterlevel, sizeof(node->waterlevel), 1u, trFile);
	TR_FWrite (&node->node_type, sizeof(node->node_type), 1u, trFile);

	b = node->paths[0] & 0xFFFF;

	for (path_id = 0; path_id < MAX_PATHS && b > -1; ++path_id)
	{
		b = node->paths[path_id] & 0xFFFF;
		TR_FWrite (&b, sizeof(b), 1u, trFile);
	}

	b = -2;
	TR_FWrite (&b, sizeof(b), 1u, trFile);

//	char path_id;

	for (node_id = 0; node_id < TRAIL_LENGTH && trail[node_id]->timestamp != 0.0; ++node_id)
	{
		if (node->routes->route_path[node_id] >= 0)
		{
			for (path_id = 0; path_id < MAX_PATHS && node->paths[path_id] != node->routes->route_path[node_id]; ++path_id)
			{
				if (node->paths[path_id] < 0 || path_id == MAX_PATHS)
				{
					path_id = -1;
					gi.dprintf ("ERROR: Unable to locate goal node in ->paths[]\n");
					break;
				}
			}
		}
		else
			path_id = -1;

		TR_FWrite (&path_id, sizeof(path_id), 1u, trFile);
	}

	path_id = -99;
	TR_FWrite (&path_id, sizeof(path_id), 1u, trFile);

	node->ignore = 1;
	return node;
}

void WriteTrail (void)
{
	int			i, end_marker;
	char		fileName[MAX_QPATH] = { 0 };
	char		filePath[MAX_OSPATH] = { 0 };
	char		zipName[MAX_OSPATH] = { 0 };
	char		intName[MAX_QPATH] = { 0 };
	char		dst[MAX_OSPATH] = { 0 };
	size_t		v2;
	edict_t		*from = NULL;
	ctf_item_t	*item = NULL;
	cvar_t		*game = gi.cvar("game", "", 0);
	cvar_t		*basedir = gi.cvar("basedir", "", 0);
#ifdef TRAIL_DIRECT_ZIP_WRITE
	fileHandle_t	trFile;
	int				fileSize;
#else	// TRAIL_DIRECT_ZIP_WRITE
	FILE		*trFile = NULL;
#endif	// TRAIL_DIRECT_ZIP_WRITE

	if ( !(((bot_calc_nodes && bot_calc_nodes->value) || dropped_trail) && (trail_head > 30)) )
		return;

	Com_sprintf (fileName, sizeof(fileName), "%s/%s%s", routes_dir_no_sep, level.mapname, route_ext);
	Com_sprintf (filePath, sizeof(filePath), "%s%s/%s%s", SavegameDir(), routes_dir, level.mapname, route_ext);
	Com_sprintf (zipName, sizeof(zipName), "%s/%s%s", routes_dir_no_sep, level.mapname, route_ext_zipped);
	Com_sprintf (intName, sizeof(intName), "%s%s", level.mapname, route_ext);

	CreatePath (filePath);

#ifdef TRAIL_DIRECT_ZIP_WRITE
	fileSize = G_OpenCompressedFile (zipName, intName, &trFile, FS_WRITE);
	if (fileSize == -1) {
		gi.dprintf ("Couldn't open compressed route file %s->%s for writing, trying uncompressed file %s\n", zipName, intName, fileName);
		fileSize = G_OpenFile (fileName, &trFile, FS_WRITE);
	}
	if (fileSize == -1) {
		gi.error ("Couldn't open route file %s for writing", fileName);
	}
#else	// TRAIL_DIRECT_ZIP_WRITE
	trFile = fopen(filePath, "wb");

	if (!trFile)
		gi.error ("Couldn't open %s", filePath);
#endif	// TRAIL_DIRECT_ZIP_WRITE

	TR_FWrite (&TRAIL_VERSION, sizeof(int), 1u, trFile);

	from = G_Find(NULL, FOFS(classname), flag_path_src_classname);

	while (from)
	{
		if (!from->last_goal || !from->target_ent)
			continue;

		TR_FWrite (&from->skill_level, sizeof(from->skill_level), 1u, trFile);
		TR_FWrite (from->s.origin, sizeof(from->s.origin), 1u, trFile);
		TR_FWrite (from->last_goal->s.origin, sizeof(from->last_goal->s.origin), 1u, trFile);
		TR_FWrite (from->target_ent->s.origin, sizeof(from->target_ent->s.origin), 1u, trFile);
		from = G_Find(from, FOFS(classname), flag_path_src_classname);
	}

	end_marker = -1;
	TR_FWrite (&end_marker, sizeof(end_marker), 1u, trFile);

	for (from = G_Find(NULL, FOFS(classname), misc_teleporter_classname); from; from = G_Find(from, FOFS(classname), misc_teleporter_classname))
		TR_FWrite (&from->ignore_time, sizeof(from->ignore_time), 1u, trFile);

	end_marker = -99;
	TR_FWrite (&end_marker, sizeof(end_marker), 1u, trFile);

	for (from = G_Find(NULL, FOFS(classname), func_plat_classname); from; from = G_Find(from, FOFS(classname), func_plat_classname))
		TR_FWrite (&from->ignore_time, sizeof(from->ignore_time), 1u, trFile);

	end_marker = -99;
	TR_FWrite (&end_marker, sizeof(end_marker), 1u, trFile);

	for (from = G_Find(NULL, FOFS(classname), func_button_classname); from; from = G_Find(from, FOFS(classname), func_button_classname))
		TR_FWrite (&from->ignore_time, sizeof(from->ignore_time), 1u, trFile);

	end_marker = -99;
	TR_FWrite (&end_marker, sizeof(end_marker), 1u, trFile);

	from = G_Find(NULL, FOFS(classname), redflag_classname);
	if (from)
	{
		int ctf_team = 1;
		TR_FWrite (&ctf_team, sizeof(ctf_team), 1u, trFile);
		TR_FWrite (from->s.origin, sizeof(from->s.origin), 1u, trFile);
		TR_FWrite (from->s.angles, sizeof(from->s.angles), 1u, trFile);
	}

	from = G_Find(NULL, FOFS(classname), blueflag_classname);
	if (from)
	{
		int ctf_team = 1;
		TR_FWrite (&ctf_team, sizeof(ctf_team), 1u, trFile);
		TR_FWrite (from->s.origin, sizeof(from->s.origin), 1u, trFile);
		TR_FWrite (from->s.angles, sizeof(from->s.angles), 1u, trFile);
	}

	end_marker = -1;
	TR_FWrite (&end_marker, sizeof(end_marker), 1u, trFile);

	for (item = ctf_item_head; item; item = item->next)
	{
		int has_ctf_item = 1;
		TR_FWrite (&has_ctf_item, 4u, 1u, trFile);
		v2 = strlen(item->classname);
		TR_FWrite (item->classname, (int)v2 + 1, 1u, trFile);
		TR_FWrite (item->origin, sizeof(item->origin), 1u, trFile);
		TR_FWrite (item->angles, sizeof(item->angles), 1u, trFile);
	}

	end_marker = -1;
	TR_FWrite (&end_marker, sizeof(end_marker), 1u, trFile);

	for (i = 0; i < TRAIL_LENGTH; i++)
		trail[i]->ignore = 0;

	for (i = 0; i < TRAIL_LENGTH && trail[i]->timestamp != 0.0; ++i)
	{
		if (trail[i]->goalentity)
		{
			from = trail[i]->goalentity;
			WriteTrailNode (trFile, from);
		}
	}

	for (i = 0; i < TRAIL_LENGTH; ++i)
	{
		if (trail[i]->timestamp != 0.0)
		{
			from = trail[i];
			WriteTrailNode (trFile, from);
		}
	}

#ifdef TRAIL_DIRECT_ZIP_WRITE
	G_CloseFile (trFile);
#else	// TRAIL_DIRECT_ZIP_WRITE
	fclose (trFile);

	if ( G_CompressFile(filePath, zipName, intName, false) != -1 )
	{
		remove (filePath);
		Com_strcpy (dst, sizeof(dst), zipName);
		dst[strlen(dst) - 4] = 0;
		Com_strcat (dst, sizeof(dst), route_ext_zipped_old);
		remove (dst);
		dst[strlen(dst) - 3] = 0;
		remove (dst);
	}
	else
		gi.dprintf ("\nError creating ZIP file.\nUnable to compress node-table.\n\n");
#endif	// TRAIL_DIRECT_ZIP_WRITE

	gi.dprintf ("Route-table saved.\n");
}

int CalculateRouteDistance (edict_t *src, edict_t *dst)
{
	if (src->routes->route_dist[dst->trail_index])
		return src->routes->route_dist[dst->trail_index];

	if (src->routes->route_path[dst->trail_index] >= 0)
	{
		int recursion = recursion_count++;

		if (recursion <= trail_head)
		{
			int dist = CalculateRouteDistance(trail[src->routes->route_path[dst->trail_index]], dst);

			if (dist <= 0)
			{
				src->routes->route_path[dst->trail_index] = -1;
				return -1;
			}

			if (dist > 60000)
				dist = 60000;

			src->routes->route_dist[dst->trail_index] = dist + src->routes->route_dist[src->routes->route_path[dst->trail_index]];
			return src->routes->route_dist[dst->trail_index];
		}

		if (bot_debug_nodes->value)
		//	gi.dprintf ("%s: Recursive link (src=%i, dst=%i)\n", __func__, src->trail_index, dst->trail_index);
			gi.dprintf ("CalculateRouteDistance: Recursive link (src=%i, dst=%i)\n", src->trail_index, dst->trail_index);

		src->routes->route_path[dst->trail_index] = -1;
		return -1;
	}

	if (bot_debug_nodes->value)
	//	gi.dprintf ("%s: Broken chain (src=%i, dst=%i)\n", __func__, src->trail_index, dst->trail_index);
		gi.dprintf ("CalculateRouteDistance: Broken chain (src=%i, dst=%i)\n", src->trail_index, dst->trail_index);

	return -1;
}

void CalculateDistances (void)
{
	int		i, j;

	if (!trail_head)
		return;

	for (i = 0; i < trail_head; ++i)
	{
		edict_t *node = trail[i];

		for (j = 0; j < MAX_PATHS && node->paths[j] >= 0; ++j)
		{
			if (node->node_type == NODE_TELEPORT)
				node->routes->route_dist[node->paths[j]] = 1;
			else
				node->routes->route_dist[node->paths[j]] = entdist(node, trail[node->paths[j]]);

			if (!node->routes->route_dist[node->paths[j]])
				node->routes->route_dist[node->paths[j]] = 1;
		}
	}

	for (i = 0; i < trail_head; ++i)
	{
		edict_t *node = trail[i];

		for (j = 0; j < trail_head; ++j)
		{
			if (!node->routes->route_dist[j] && node->routes->route_path[j] >= 0)
			{
				recursion_count = 0;
				CalculateRouteDistance (node, trail[j]);
			}
		}
	}
}

#ifdef TRAIL_DIRECT_ZIP_READ
int ReadTrailNode (fileHandle_t trFile, edict_t *node)
#else	// TRAIL_DIRECT_ZIP_READ
FILE *ReadTrailNode (FILE *trFile, edict_t *node)
#endif	// TRAIL_DIRECT_ZIP_READ
{
	int		i, goal, readSize;
	short	buf;
	char	path_id = 0;
	vec3_t	end;
	trace_t	tr;

	TR_FRead (&goal, sizeof(goal), 1u, trFile);

	if (goal > -1)
		node->goalentity = trail[goal];

	TR_FRead (node->s.origin, sizeof(node->s.origin), 1u, trFile);
	TR_FRead (&node->timestamp, sizeof(node->timestamp), 1u, trFile);
	TR_FRead (node->s.angles, sizeof(node->s.angles), 1u, trFile);
	TR_FRead (node->mins, sizeof(node->mins), 1u, trFile);
	TR_FRead (node->maxs, sizeof(node->maxs), 1u, trFile);
	TR_FRead (node->velocity, sizeof(node->velocity), 1u, trFile);

	if (trail_version > 1)
	{
		TR_FRead (&node->waterlevel, sizeof(node->waterlevel), 1u, trFile);
		TR_FRead (&node->node_type, sizeof(node->node_type), 1u, trFile);
	}

	buf = 0;

	for (i = 0; i < MAX_PATHS; ++i)
	{
		if (buf != -2)
			TR_FRead (&buf, sizeof(buf), 1u, trFile);

		if (buf == -2)
			node->paths[i] = -1;
		else
			node->paths[i] = buf;
	}

	while (buf != -2)
	{
		readSize = TR_FRead (&buf, sizeof(buf), 1u, trFile);

#ifdef TRAIL_DIRECT_ZIP_READ
		if (readSize != sizeof(buf))
#else	// TRAIL_DIRECT_ZIP_READ
		if ( feof(trFile) )
#endif	// TRAIL_DIRECT_ZIP_READ
		{
			gi.error ("Unexpected end of route-table\n");
#ifdef TRAIL_DIRECT_ZIP_READ
			return -1;
#else	// TRAIL_DIRECT_ZIP_READ
			return NULL;
#endif	// TRAIL_DIRECT_ZIP_READ
		}
	}

	for (i = 0; i < TRAIL_LENGTH; ++i)
	{
		if (trail_version >= 10)
		{
			TR_FRead (&path_id, sizeof(path_id), 1u, trFile);

			if (path_id == -99)
			{
				node->routes->route_path[i] = -1;
				node->routes->route_dist[i] = 0;
				break;
			}

			if (path_id <= -1)
				node->routes->route_path[i] = -1;
			else
			{
				node->routes->route_path[i] = node->paths[path_id];
				if (node->paths[path_id] == -1 && bot_debug_nodes->value)
					gi.dprintf ("Node reference turned out to be empty\n");
			}
		}
		else
		{
			TR_FRead (&buf, sizeof(buf), 1u, trFile);

			if (buf == -99)
			{
				node->routes->route_path[i] = -1;
				node->routes->route_dist[i] = 0;
				break;
			}

			for (path_id = 0; path_id < MAX_PATHS && node->paths[path_id] != buf; ++path_id)
				;

			if (path_id == MAX_PATHS)
				node->routes->route_path[i] = -1;
			else
				node->routes->route_path[i] = buf;

			// Unused byte from old file format?
			TR_FRead (&buf, 1u, 1u, trFile);
		}
		node->routes->route_dist[i] = 0;
	}

	if (trail_version >= 10)
	{
		while (path_id != -99)
		{
			readSize = TR_FRead (&path_id, sizeof(path_id), 1u, trFile);
#ifdef TRAIL_DIRECT_ZIP_READ
			if (readSize != sizeof(path_id))
				return -1;
#else	// TRAIL_DIRECT_ZIP_READ
			if ( feof(trFile) )
				return NULL;
#endif	// TRAIL_DIRECT_ZIP_READ
		}
	}
	else
	{
		while (buf != -99)
		{
			readSize = TR_FRead (&buf, sizeof(buf), 1u, trFile);
#ifdef TRAIL_DIRECT_ZIP_READ
			if (readSize != sizeof(buf))
				return -1;
#else	// TRAIL_DIRECT_ZIP_READ
			if ( feof(trFile) )
				return NULL;
#endif	// TRAIL_DIRECT_ZIP_READ
		}
	}

	while (i < TRAIL_LENGTH)
	{
		node->routes->route_path[i] = -1;
		node->routes->route_dist[i++] = 0;
	}

	AddTrailToPortals (node);

	VectorCopy (node->s.origin, end);
	end[2] -= 8.0;

	tr = gi.trace(node->s.origin, node->mins, node->maxs, end, node, MASK_PLAYERSOLID);

	if (tr.fraction >= 1.0)
		node->groundentity = NULL;
	else
		node->groundentity = tr.ent;

	if (node->node_type == NODE_BUTTON)
	{
		edict_t *buttonActivator = G_Spawn();
		buttonActivator->owner = node;
		buttonActivator->think = botButtonThink;
		buttonActivator->nextthink = level.time + 2.0;
		VectorMA (node->s.origin, -24.0, node->movedir, buttonActivator->s.origin);
	}
	else if (node->node_type == NODE_GRAPPLE)
	{
		gi.cvar_forceset("grapple", "1");

		if (!ctf->value && notify_force_grapple && !G_Find(NULL, FOFS(classname), item_flag_team1_classname))
		{
			gi.dprintf ("............................\nMap contains grapple nodes..\nGrapple-Hook has been ENABLED\n............................\n");
			notify_force_grapple = false;
		}
	}

#ifdef TRAIL_DIRECT_ZIP_READ
	return 0;
#else	// TRAIL_DIRECT_ZIP_READ
	return trFile;
#endif	// TRAIL_DIRECT_ZIP_READ
}

void ReadTrail (void)
{
	int				nodes_loaded, ignore_time;
	char		fileName[MAX_QPATH] = { 0 };
	char		filePath[MAX_OSPATH] = { 0 };
	char		zipName[MAX_OSPATH] = { 0 };
	char		zipNameOld[MAX_OSPATH] = { 0 };
	char		deletePath[MAX_OSPATH] = { 0 };
	char		intName[MAX_QPATH] = { 0 };
	qboolean	using_old_zip_path = false;
	edict_t		*from = NULL;
	qboolean	isZipped = false;
	qboolean	unknown_flag = true;
#ifdef TRAIL_DIRECT_ZIP_READ
	fileHandle_t	trFile;
	int			fileSize = 0, nodeRet = 0;
#else
	FILE		*trFile = NULL;
#endif	// TRAIL_DIRECT_ZIP_READ

	cvar_t *game = gi.cvar("game", "", 0);
	cvar_t *basedir = gi.cvar("basedir", "", 0);

	Com_sprintf (fileName, sizeof(fileName), "%s/%s%s", routes_dir_no_sep, level.mapname, route_ext);
	Com_sprintf (filePath, sizeof(filePath), "%s%s/%s%s", SavegameDir(), routes_dir, level.mapname, route_ext);
	Com_sprintf (zipName, sizeof(zipName), "%s/%s%s", routes_dir_no_sep, level.mapname, route_ext_zipped);
	Com_sprintf (zipNameOld, sizeof(zipNameOld), "%s/%s%s", routes_dir_no_sep, level.mapname, route_ext_zipped_old);
	Com_sprintf (intName, sizeof(intName), "%s%s", level.mapname, route_ext);

#ifdef TRAIL_DIRECT_ZIP_READ
	fileSize = G_OpenCompressedFile (zipName, intName, &trFile, FS_READ);
	if (fileSize != -1) {
		isZipped = true;
	}
	else
	{
		fileSize = G_OpenCompressedFile (zipNameOld, intName, &trFile, FS_READ);
		if (fileSize != -1) {
			isZipped = true;
			using_old_zip_path = true;
		}
		else {
			fileSize = G_OpenFile (fileName, &trFile, FS_READ);
		}
	}
#else	// TRAIL_DIRECT_ZIP_READ
	// We may be decompressing from gamedir into savegamedir, so always try to create path
	CreatePath (filePath);

	if ( G_DecompressFile(filePath, zipName, intName) != -1 ) {
		isZipped = true;
	}
	else if ( G_DecompressFile(filePath, zipNameOld, intName) != -1 ) {
		isZipped = true;
		using_old_zip_path = true;
	}

	trFile = fopen(filePath, "rb");
#endif	// TRAIL_DIRECT_ZIP_READ

#ifdef TRAIL_DIRECT_ZIP_READ
	if (fileSize == -1)
#else	// TRAIL_DIRECT_ZIP_READ
	if (!trFile)
#endif	// TRAIL_DIRECT_ZIP_READ
	{
		if (!bot_calc_nodes->value)
		{
			gi.cvar_set("bot_calc_nodes", "1");
			gi.dprintf ("Unable to load route-table file.\nDynamic node-table generation ENABLED.\n");
		}

		return;
	}

	reading_trail = 1;
	TR_FRead (&trail_version, sizeof(trail_version), 1u, trFile);

	if (trail_version < 6)
	{
		gi.dprintf ("Route-table incompatible (v%i), ignoring\n", trail_version);
#ifdef TRAIL_DIRECT_ZIP_READ
		G_CloseFile (trFile);
#else	// TRAIL_DIRECT_ZIP_READ
		fclose (trFile);
#endif	// TRAIL_DIRECT_ZIP_READ

#ifndef TRAIL_DIRECT_ZIP_READ
		if (isZipped)
			remove (filePath);
#endif	// TRAIL_DIRECT_ZIP_READ

		return;
	}

	if (trail_version > 6)
	{
		int ctf_team;
		TR_FRead (&ctf_team, sizeof(ctf_team), 1u, trFile);

		while (ctf_team > -1)
		{
			static edict_t temp;
			temp.client = NULL;
			TR_FRead (temp.s.origin, sizeof(temp.s.origin), 1u, trFile);
			FlagPath (&temp, ctf_team);
			TR_FRead (temp.s.origin, sizeof(temp.s.origin), 1u, trFile);
			FlagPath (&temp, ctf_team);
			TR_FRead (temp.s.origin, sizeof(temp.s.origin), 1u, trFile);
			FlagPath (&temp, ctf_team);
			TR_FRead (&ctf_team, sizeof(ctf_team), 1u, trFile);
		}
	}

	if (trail_version >= 4)
	{
		from = G_Find(NULL, FOFS(classname), misc_teleporter_classname);

		TR_FRead (&ignore_time, sizeof(ignore_time), 1u, trFile);

		while (ignore_time >= 0)
		{
			if (from)
				from->ignore_time = ignore_time;
			else
				gi.dprintf ("Couldn't find teleporter\n");

			from = G_Find(from, FOFS(classname), misc_teleporter_classname);
			TR_FRead (&ignore_time, sizeof(ignore_time), 1u, trFile);
		}

		from = G_Find(NULL, FOFS(classname), func_plat_classname);

		TR_FRead (&ignore_time, sizeof(ignore_time), 1u, trFile);

		while (ignore_time >= 0)
		{
			if (from)
				from->ignore_time = ignore_time;
			else
				gi.dprintf ("Couldn't find func_plat\n");

			from = G_Find(from, FOFS(classname), func_plat_classname);
			TR_FRead (&ignore_time, sizeof(ignore_time), 1u, trFile);
		}
	}

	if (trail_version > 12)
	{
		from = G_Find(NULL, FOFS(classname), func_button_classname);

		TR_FRead (&ignore_time, sizeof(ignore_time), 1u, trFile);

		while (ignore_time >= 0)
		{
			if (from)
				from->ignore_time = ignore_time;
			else
				gi.dprintf ("Couldn't find func_button\n");

			from = G_Find(from, FOFS(classname), func_button_classname);
			TR_FRead (&ignore_time, sizeof(ignore_time), 1u, trFile);
		}
	}

	if (trail_version > 10)
	{
		while (1)
		{
			int		ctf_team;
			vec3_t	origin, angles;
			edict_t *flag = NULL;

			TR_FRead (&ctf_team, sizeof(ctf_team), 1u, trFile);

			if (ctf_team <= -1)
				break;

			TR_FRead (origin, sizeof(origin), 1u, trFile);
			TR_FRead (angles, sizeof(angles), 1u, trFile);

			if (!ctf->value)
				continue;

			flag = G_Spawn();

			if (ctf_team == 1)
				flag->classname = item_flag_team1_classname;
			else
				flag->classname = item_flag_team2_classname;

			VectorCopy (origin, flag->s.origin);
			VectorCopy (angles, flag->s.angles);

			ED_CallSpawn (flag);
		}
	}

	if (trail_version > 11)
	{
		int ctf_team;

		ctf_item_head = NULL;

		TR_FRead (&ctf_team, sizeof(ctf_team), 1u, trFile);

		while (ctf_team > -1)
		{
			int			classname_len = -1;
			ctf_item_t	*item = gi.TagMalloc(sizeof(ctf_item_t), TAG_LEVEL);
			edict_t		*ctf_item = NULL;

			memset (item, 0, sizeof(ctf_item_t));

			do {
				TR_FRead (&item->classname[++classname_len], sizeof(char), 1u, trFile);
			} while (item->classname[classname_len]);

			TR_FRead (item->origin, sizeof(item->origin), 1u, trFile);
			TR_FRead (item->angles, sizeof(item->angles), 1u, trFile);

			if ( !ctf->value )
			{
				ctf_item_head = NULL;
				gi.TagFree(item);
			}
			else
			{
				item->next = ctf_item_head;
				ctf_item_head = item;

				ctf_item = G_Spawn();
				ctf_item->classname = ctf_item_head->classname;
				VectorCopy (ctf_item_head->origin, ctf_item->s.origin);
				VectorCopy (ctf_item_head->angles, ctf_item->s.angles);
				ED_CallSpawn(ctf_item);
			}

			TR_FRead (&ctf_team, sizeof(ctf_team), 1u, trFile);
		}
	}

	nodes_loaded = 0;

	notify_force_grapple = 1;

#ifdef TRAIL_DIRECT_ZIP_READ
	while (nodeRet != -1)
#else	// TRAIL_DIRECT_ZIP_READ
	while ( !feof(trFile) )
#endif	// TRAIL_DIRECT_ZIP_READ
	{
		int		node_id;
		float	dist;
		vec3_t	vec;
	//	edict_t	*from = NULL;
		edict_t	*plat = NULL;
#ifdef TRAIL_DIRECT_ZIP_READ
		int		readSize;

		readSize = TR_FRead (&node_id, sizeof(node_id), 1u, trFile);
		if ( readSize != sizeof(node_id) )
#else	// TRAIL_DIRECT_ZIP_READ
		if ( !TR_FRead (&node_id, sizeof(node_id), 1u, trFile) )
#endif	// TRAIL_DIRECT_ZIP_READ
			break;

		from = trail[node_id];

#ifdef TRAIL_DIRECT_ZIP_READ
		nodeRet = ReadTrailNode (trFile, from);

		if (nodeRet == -1)
#else	// TRAIL_DIRECT_ZIP_READ
		trFile = ReadTrailNode (trFile, from);

		if ( !trFile )
#endif	// TRAIL_DIRECT_ZIP_READ
		{
#ifndef TRAIL_DIRECT_ZIP_READ
			remove (filePath);
#endif	// TRAIL_DIRECT_ZIP_READ
			if (isZipped)
			{
				if ( using_old_zip_path ) {
					SavegameDirRelativePath (zipNameOld, deletePath, sizeof(deletePath));
					remove (deletePath);
					GameDirRelativePath (zipNameOld, deletePath, sizeof(deletePath));
					remove (deletePath);
				}
				else {
					SavegameDirRelativePath (zipName, deletePath, sizeof(deletePath));
					remove (deletePath);
					GameDirRelativePath (zipName, deletePath, sizeof(deletePath));
					remove (deletePath);
				}
			}

			gi.error ("End of paths flag not found.\nRoute-table is corrupt, deleting.\n");
			return;
		}

		if (from->node_type == NODE_PLAT)
		{
			float best_dist = 99999.0;
			from->target_ent = NULL;

			plat = NULL;

			while (1)
			{
				plat = G_Find(plat, FOFS(classname), func_plat_classname);

				if (!plat)
					break;

				VectorSubtract (from->s.origin, plat->mins, vec);
				dist = VectorLength(vec);

				if (dist < best_dist)
				{
					from->target_ent = plat;
					best_dist = dist;
				}
			}
		}
		else if (unknown_flag)
		{
			if (from->trail_index)
				from->node_type = NODE_LANDING;
			else
				unknown_flag = false;
		}

		++nodes_loaded;

		if (from->paths[0] == -1)
			PlayerTrail_FindPaths (node_id);

		if (nodes_loaded >= TRAIL_LENGTH)
			break;
	}

	gi.dprintf ("Route-table loaded, %i nodes updated\n", nodes_loaded);
	nodes_done = nodes_loaded > 40;

#ifdef TRAIL_DIRECT_ZIP_READ
	G_CloseFile (trFile);
#else	// TRAIL_DIRECT_ZIP_READ
	fclose (trFile);
#endif	// TRAIL_DIRECT_ZIP_READ

#ifndef TRAIL_DIRECT_ZIP_READ
	if (isZipped)
		remove (filePath);
#endif	// TRAIL_DIRECT_ZIP_READ

	last_head = nodes_loaded - 1;
	trail_head = nodes_loaded;

	for (from = G_Find(NULL, FOFS(classname), misc_teleporter_classname); from; from = G_Find(from, FOFS(classname), misc_teleporter_classname))
	{
		int		node_id;
		edict_t	*best_node = NULL;
		float	best_dist = 99999.0;

		if ( !from->ignore_time )
			continue;

		for (node_id = 0; node_id < nodes_loaded; ++node_id)
		{
			float dist = entdist(from, trail[node_id]);

			if (dist >= 64.0 || dist >= best_dist)
				continue;

			best_node = trail[node_id];
			best_dist = dist;
		}

		if (best_node)
			best_node->node_type = NODE_TELEPORT;
	}

	CalculateDistances ();

	loaded_trail_flag = true;
	reading_trail = false;
}

void Debug_ShowPathToGoal (edict_t *self, edict_t *goalent)
{
	float	dist, total_dist;
	int		node_count;
	edict_t	*from = NULL;
	edict_t	*node = NULL;
	edict_t	*prev = NULL;

	if (!(level.time - 0.5 >= last_laser_route))
		return;

	last_laser_route = level.time;

	from = G_Find(NULL, FOFS(classname), path_beam_classname);

	while (from)
	{
		edict_t *beam = from;
		from = G_Find(from, FOFS(classname), path_beam_classname);
		G_FreeEdict (beam);
	}

	if (self->bot_client)
	{
		if (self->goalentity)
		{
			dist = PathToEnt(self->goalentity, goalent, 0, 1);
			PathToEnt_Node = self->goalentity;
		}
		else
		{
			dist = PathToEnt(self, goalent, 0, 1);

			if (dist == -1.0)
			{
				gi.dprintf (debug_no_path_fmt, goalent->classname);
				return;
			}
		}

		gi.dprintf (debug_path_length, dist);
	}
	else
	{
		dist = PathToEnt(self, goalent, 0, 1);

		if (dist == -1.0)
		{
			gi.dprintf (debug_no_path_fmt, goalent->classname);
			return;
		}

		gi.dprintf (debug_path_length, dist);
	}

	node = PathToEnt_Node;
	prev = self;
	total_dist = 0.0;
	node_count = 0;

	while (node != goalent && total_dist < 1000.0 && node_count < 20)
	{
		edict_t *laser = DrawLine (self, node->s.origin, prev->s.origin);

		if (node->node_type == 5)
			gi.dprintf (" (GRAPPLE) ");

		gi.dprintf ("%i, ", node->trail_index);
		gi.linkentity(laser);

		if (PathToEnt(node, goalent, 0, 0) == -1.0)
			return;

		prev = node;
		node = PathToEnt_Node;

		total_dist = entdist(prev, PathToEnt_Node) + total_dist;
		++node_count;
	}

	DrawLine (self, prev->s.origin, node->s.origin);
	gi.dprintf ("%i.\n\n", node->trail_index);
}

void CheckMoveForNodes (edict_t *ent)
{
	int		closest, node_index;
	vec3_t	vec, vel;
	edict_t *target = NULL;
	edict_t *goalent = NULL;

	if (!(trail_head < TRAIL_LENGTH - 1 && ent->movetype != MOVETYPE_NOCLIP && ent->solid))
		return;

	goalent = PlayerTrail_LastSpot();

	if (trail[0]->timestamp == 0.0)
	{
		PlayerTrail_Add (ent, ent->s.origin, NULL, 0, 1, NODE_NORMAL);
		return;
	}

	if (!(goalent->trail_index < TRAIL_LENGTH - 1))
		return;

	goalent = ent->last_trail_dropped;

	if (ent->last_trail_dropped)
		VectorSubtract (goalent->s.origin, ent->s.origin, vec);
	else
	{
		VectorClear (vec);

		closest = ClosestNodeToEnt(ent, 0, 1);

		if (closest <= -1)
		{
			PlayerTrail_Add (ent, ent->s.origin, NULL, 1, 1, NODE_NORMAL);
			goto nocheck;
		}

		ent->last_trail_dropped = trail[closest];
		goalent = ent->last_trail_dropped;
	}

	if (ent->flags & FL_NO_KNOCKBACK)
	{
nocheck:
		if (ent->maxs[2] == 32.0 && ent->duck_ent)
		{
			G_FreeEdict (ent->duck_ent);
			ent->duck_ent = NULL;
		}

		ent->last_max_z = ent->maxs[2];
		ent->last_groundentity = ent->groundentity;

		if (ent->last_max_z > 4.0 && ent->last_trail_dropped->maxs[2] == 4.0)
			ent->last_trail_dropped->flags |= 4u;

		return;
	}

	if (ent->maxs[2] == 4.0 && ent->last_max_z != ent->maxs[2])
	{
		ent->duck_ent = G_Spawn();
		ent->duck_ent->classname = player_duck_classname;
		VectorCopy (ent->mins, ent->duck_ent->mins);
		VectorCopy (ent->maxs, ent->duck_ent->maxs);
		ent->duck_ent->closest_trail_time = 0.0;
		ent->duck_ent->closest_trail = 0;
		VectorClear (ent->duck_ent->velocity);
		VectorCopy (ent->s.origin, ent->duck_ent->s.origin);
		goto LABEL_62;
	}

	if (ent->last_max_z != ent->maxs[2]
		&& ent->maxs[2] == 32.0
		&& ent->last_trail_dropped->maxs[2] < 32.0
		&& !(ent->last_trail_dropped->flags & 4))
	{
		ent->last_trail_dropped->flags = (ent->last_trail_dropped->flags | 4) & 0xffff;
		ent->maxs[2] = 4.0;

		PlayerTrail_Add (ent, ent->s.old_origin, NULL, 1, 1, NODE_NORMAL);

		last_trail_time = level.time;
		ent->maxs[2] = 32.0;
		goto nocheck;
	}

	if (ent->groundentity || !ent->last_groundentity || ent->waterlevel >= 2)
	{
		if (ent->groundentity || !ent->waterlevel || ent->waterlevel >= 3 || ent->velocity[2] <= 80.0)
		{
			if (!ent->last_groundentity && (ent->waterlevel > 0 || ent->groundentity))
			{
				if (ent->jump_ent)
				{
					qboolean check_fullbox = abs(ent->jump_ent->s.origin[2] - ent->s.origin[2]) > 18;
					float dist = PathToEnt(ent->jump_ent, ent, check_fullbox, 1);

					if (dist == -1.0 || entdist(ent->jump_ent, ent) * 4.0 < dist || entdist(ent->jump_ent, ent) + 512.0 < dist)
					{
						if (visible_box(ent, ent->jump_ent) && CanReach(ent, ent->jump_ent))
						{
							if (ent->maxs[2] == 4.0 && ent->duck_ent)
							{
								PlayerTrail_Add (ent, ent->duck_ent->s.origin, NULL, 0, 1, NODE_NORMAL);
								G_FreeEdict (ent->duck_ent);
								ent->duck_ent = NULL;
							}

							PlayerTrail_Add (ent, ent->jump_ent->s.origin, NULL, 0, 1, NODE_NORMAL);
							PlayerTrail_Add (ent, ent->s.origin, NULL, 0, 1, NODE_NORMAL);
							G_FreeEdict (ent->jump_ent);
							ent->jump_ent = NULL;
							last_trail_time = level.time;
						}
						else
						{
							NodeDebug ("Dropping jump nodes\n");

							PlayerTrail_Add (ent, ent->s.origin, NULL, 1, 1, NODE_LANDING);

							goalent = ent->last_trail_dropped;

							PlayerTrail_Add (ent->jump_ent, ent->jump_ent->s.origin, goalent, 1, 1, ent->jump_ent->flags);

							VectorCopy (ent->jump_ent->velocity, ent->last_trail_dropped->velocity);

							node_index = goalent->trail_index;

							if (!(ent->jump_ent->flags & FL_NO_KNOCKBACK) && ent->s.origin[2] + 48.0 > ent->jump_ent->s.origin[2])
							{
								NodeDebug ("Adding reverse jump nodes\n");

								level.time = level.time - 0.1;

								PlayerTrail_Add (ent->jump_ent, ent->jump_ent->s.origin, NULL, 1, 1, NODE_LANDING);

								goalent = ent->jump_ent->last_trail_dropped;

								PlayerTrail_Add (ent, ent->s.origin, goalent, 1, 1, NODE_NORMAL);

								VectorScale (ent->velocity, -2.0, vel);

								if (vel[2] <= 200.0)
								{
									if (vel[2] < 40.0)
										vel[2] = 40.0;
								}
								else
									vel[2] = 310.0;

								VectorCopy (vel, ent->last_trail_dropped->velocity);
								level.time = level.time + 0.1;
							}

							CalcRoutes (node_index);

							G_FreeEdict (ent->jump_ent);

							ent->jump_ent = NULL;
						}

						goto nocheck;
					}
				}
			}

			if (ent->jump_ent && (ent->groundentity || ent->waterlevel))
			{
				G_FreeEdict (ent->jump_ent);
				ent->jump_ent = NULL;
			}

			goto LABEL_62;
		}

		if (!ent->jump_ent)
		{
			ent->jump_ent = G_Spawn();
			ent->jump_ent->classname = player_jump_classname;
			VectorCopy (ent->mins, ent->jump_ent->mins);
			VectorCopy (ent->maxs, ent->jump_ent->maxs);
			ent->jump_ent->closest_trail_time = 0.0;
			ent->jump_ent->closest_trail = 0;
		}

		VectorCopy (ent->velocity, ent->jump_ent->velocity);
		VectorCopy (ent->s.old_origin, ent->jump_ent->s.origin);
		ent->jump_ent->waterlevel = ent->waterlevel;

		if (ent->client->ctf_grapple)
			goto LABEL_27;

		ent->jump_ent->flags = NODE_NORMAL;
	}
	else
	{
		if (!ent->jump_ent)
		{
			ent->jump_ent = G_Spawn();
			ent->jump_ent->classname = player_jump_classname;
			VectorSet (ent->jump_ent->mins, -16.0, -16.0, -24.0);
			VectorSet (ent->jump_ent->maxs, 16.0, 16.0, 32.0);
			ent->jump_ent->closest_trail_time = 0.0;
			ent->jump_ent->closest_trail = 0;
		}

		VectorCopy (ent->velocity, ent->jump_ent->velocity);
		VectorCopy (ent->s.old_origin, ent->jump_ent->s.origin);
		ent->jump_ent->waterlevel = ent->waterlevel;

		if (ent->client->ctf_grapple)
		{
LABEL_27:
			ent->jump_ent->flags = NODE_GRAPPLE;
			VectorCopy (ent->animate_org, ent->jump_ent->s.origin);
			VectorSubtract (ent->client->ctf_grapple->s.origin, ent->animate_org, ent->jump_ent->velocity);
			VectorNormalize (ent->jump_ent->velocity);
			goto LABEL_62;
		}
		ent->jump_ent->flags = NODE_NORMAL;
	}

LABEL_62:
	if ((ent->groundentity || ent->waterlevel > 0) && (VectorLength(vec) > 128.0 || !visible_fullbox(ent, goalent)))
	{
		float dist = PathToEnt(goalent, ent, 0, 1);

		if (dist == -1.0 || dist > 2000.0)
			goto LABEL_89;

		if ( (PathToEnt_TargetNode == ent) || !(target = PathToEnt_TargetNode) )
			target = goalent;

		if (target && entdist(target, ent) > 128.0)
		{
LABEL_89:
			if (ent->maxs[2] == 4.0 && ent->duck_ent)
			{
				PlayerTrail_Add (ent, ent->duck_ent->s.origin, NULL, 0, 1, 0);
				G_FreeEdict (ent->duck_ent);
				ent->duck_ent = 0;
			}

			PlayerTrail_Add (ent, ent->s.old_origin, NULL, 0, 1, 0);
			last_trail_time = level.time;
		}
		else if (target && target->routes)
		{
			vec3_t ent_target_vec, ent_goal_vec;

			VectorSubtract (ent->s.origin, target->s.origin, ent_target_vec);
			VectorSubtract (ent->s.origin, goalent->s.origin, ent_goal_vec);

			if ( VectorLength(ent_goal_vec) > VectorLength(ent_target_vec) )
				ent->last_trail_dropped = target;
		}
	}

	goto nocheck;
}

void CalcItemPaths (edict_t *ent)
{
	static vec3_t mins = { -16.f, -16.f, 0.f };

	if (level.time >= 3.0)
	{
		int		i, j, contents, x, y, num, node_index;
		int		best_node = -1;
		float	best_dist = 99999.0;
		float	dist;
		char	path_index = 0;
		vec3_t	point, vec;
		trace_t tr;

		VectorSubtract (ent->s.origin, tv(0.f, 0.f, 10.f), point);
		contents = gi.pointcontents(point);

		if (contents & (CONTENTS_SLIME | CONTENTS_LAVA))
			return;

		x = GetGridPortal(ent->s.origin[0]);
		y = GetGridPortal(ent->s.origin[1]);
		num = num_trail_portals[x][y];

		for (i = 0; i < num; ++i)
		{
			node_index = trail_portals[x][y][i];

			tr = gi.trace(
				trail[node_index]->s.origin,
				mins,
				trail[node_index]->maxs,
				ent->s.origin,
				NULL,
				MASK_SOLID);

			VectorSubtract (ent->s.origin, tr.endpos, vec);

			if (VectorLength(vec) >= 30.0)
				continue;

			if (!CanReach(trail[node_index], ent))
				continue;

			ent->paths[path_index++] = node_index;
			dist = entdist(trail[node_index], ent);

			if (dist < best_dist)
			{
				best_dist = dist;
				best_node = node_index;
			}

			if (path_index >= MAX_PATHS)
				break;
		}

		while (path_index < MAX_PATHS)
			ent->paths[path_index++] = -1;

		if (best_node <= -1)
			ent->movetarget = NULL;
		else
			ent->movetarget = trail[best_node];

		if (ent->item->pickup == Pickup_Weapon)
			weapons_head = AddToItemList(ent, weapons_head);
		else if (ent->item->pickup == Pickup_Health)
			health_head = AddToItemList(ent, health_head);
		else if (ent->item->pickup == Pickup_Ammo)
			ammo_head = AddToItemList(ent, ammo_head);
		else
			bonus_head = AddToItemList(ent, bonus_head);

		for (j = 0; j < num_players; ++j)
		{
			if (!players[j]->bot_client)
				continue;

			dist = entdist(ent, players[j]);

			if (dist > 384.0)
				continue;

			if (players[j]->movetarget && entdist(players[j]->movetarget, players[j]) <= dist)
				continue;

			if (players[j]->enemy
				&& players[j]->bot_fire != botBlaster
				&& dist >= 128.0
				&& (entdist(players[j]->enemy, players[j]) <= dist))
				continue;

			if (PathToEnt(players[j], ent, 0, 0) == -1.0)
				continue;

			players[j]->movetarget = ent;
			players[j]->goalentity = PathToEnt_Node;
		}
	}
	else
	{
		int		i, node_index;
		char	path_index = 0;
		int		best_node = -1;
		float	best_dist = 99999.0;
		float	dist;
		int		x = GetGridPortal(ent->s.origin[0]);
		int		y = GetGridPortal(ent->s.origin[1]);
		int		num = num_trail_portals[x][y];
		vec3_t	vec;
		trace_t tr;

		for (i = 0; i < num; ++i)
		{
			node_index = trail_portals[x][y][i];

			tr = gi.trace(
				trail[node_index]->s.origin,
				mins,
				trail[node_index]->maxs,
				ent->s.origin,
				NULL,
				MASK_SOLID);

			VectorSubtract (ent->s.origin, tr.endpos, vec);

			if (VectorLength(vec) >= 30.0)
				continue;

			ent->paths[path_index++] = node_index;

			dist = entdist(trail[node_index], ent);

			if (dist < best_dist)
			{
				best_dist = dist;
				best_node = node_index;
			}

			if (path_index >= MAX_PATHS)
				break;
		}

		while (path_index < MAX_PATHS)
			ent->paths[path_index++] = -1;

		if (best_node <= -1)
		{
			if (ent->count != 2
				&& ent->item->tag != ARMOR_SHARD
				&& (ent->item->pickup != Pickup_Health || (ent->style & 1)))
			{
				if (bot_debug_nodes && bot_debug_nodes->value)
				{
					gi.dprintf ("%s not reachable\n", ent->classname);
					ent->s.effects |= EF_FLAG1;
					ent->s.renderfx |= RF_WEAPONMODEL;
				}

				ent->movetarget = NULL;
				nodes_done = false;
			}
		}
		else
			ent->movetarget = trail[best_node];

		if (ent->item->pickup == Pickup_Weapon)
			weapons_head = AddToItemList(ent, weapons_head);
		else if (ent->item->pickup == Pickup_Health)
			health_head = AddToItemList(ent, health_head);
		else if (ent->item->pickup == Pickup_Ammo)
			ammo_head = AddToItemList(ent, ammo_head);
		else
			bonus_head = AddToItemList(ent, bonus_head);
	}
}
