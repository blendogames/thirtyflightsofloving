// g_utils.c -- misc utility functions for game module

#include "g_local.h"

void G_ProjectSource(vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1];
	result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1];
	result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] + distance[2];
}


/*
=============
G_Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL.
NULL will be returned if the end of the list is reached.

=============
*/
edict_t *G_Find(edict_t *from, size_t fieldofs, char *match)	// Knightmare- changed fieldofs from int
{
	char *s;

	if (!from)
		from = g_edicts;
	else
		from++;

	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;

		s = *(char **) ((byte *)from + fieldofs);
		if (!s)
			continue;

		if (!Q_stricmp(s, match))
			return from;
	}

	return NULL;
}

//CW++
/*
=================
FindClientRadius

Returns active client entities whose bounding boxes are within a 
spherical volume centred around the specified point.
=================
*/
edict_t *FindClientRadius(edict_t *from, vec3_t origin, float radius)
{
	vec3_t	ent_vec;
	int		i;

	if (!from)
		from = g_edicts;
	else
		++from;
	for ( ; from < &g_edicts[game.maxclients]; ++from)
	{
		if (!from->client)
			continue;
		if (!from->inuse)
			continue;
		if (from->solid == SOLID_NOT)
			continue;

		for (i = 0; i < 3; ++i)
			ent_vec[i] = origin[i] - (from->s.origin[i] + (from->mins[i] + from->maxs[i])*0.5);

		if (VectorLength(ent_vec) > radius)
			continue;

		return from;
	}

	return NULL;
}

/*
=================
FindPointRadius

Returns solid and non-solid entities whose origins are within a 
spherical volume centred around the specified point.
=================
*/
edict_t *FindPointRadius(edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		i;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; ++from)
	{
		if (!from->inuse)
			continue;

		for (i = 0; i < 3; ++i)
			eorg[i] = org[i] - from->s.origin[i];
		if (VectorLength(eorg) > rad)
			continue;

		return from;
	}

	return NULL;
}
//CW--

/*
=================
FindRadius

Returns solid entities whose bounding boxes are within a 
spherical volume centred around the specified point.
=================
*/
edict_t *FindRadius(edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		i;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
		if (from->solid == SOLID_NOT)
			continue;

		for (i = 0; i < 3; ++i)
			eorg[i] = org[i] - (from->s.origin[i] + (from->mins[i] + from->maxs[i])*0.5);
		if (VectorLength(eorg) > rad)
			continue;

		return from;
	}

	return NULL;
}


/*
=============
G_PickTarget

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.
=============
*/
#define MAXCHOICES	8

edict_t *G_PickTarget(char *targetname)
{
	edict_t	*ent = NULL;
	edict_t	*choice[MAXCHOICES];
	int		num_choices = 0;

	if (!targetname)
	{
		gi.dprintf("G_PickTarget called with NULL targetname\n");
		return NULL;
	}

	while (1)
	{
		ent = G_Find(ent, FOFS(targetname), targetname);
		if (!ent)
			break;

		choice[num_choices++] = ent;
		if (num_choices == MAXCHOICES)
			break;
	}

	if (!num_choices)
	{
		gi.dprintf("G_PickTarget: target %s not found\n", targetname);
		return NULL;
	}

	return choice[rand() % num_choices];
}


void Think_Delay(edict_t *ent)
{
	G_UseTargets(ent, ent->activator);
	G_FreeEdict(ent);
}

/*
==============================
G_UseTargets

the global "activator" should be set to the entity that initiated the firing.

If self.delay is set, a DelayedUse entity will be created that will actually
do the SUB_UseTargets after that many seconds have passed.

Centerprints any self.message to the activator.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function

==============================
*/
void G_UseTargets(edict_t *ent, edict_t *activator)
{
	edict_t	*t;

//CW++
	edict_t	*cl_ent;
	int		i;
//CW--


//	Check for a delay.

	if (ent->delay)
	{
		// create a temp object to fire at a later time
		t = G_Spawn();
		t->classname = "DelayedUse";
		t->nextthink = level.time + ent->delay;
		t->think = Think_Delay;
		t->activator = activator;
		if (!activator)
			gi.dprintf("Think_Delay with no activator\n");

		t->message = ent->message;
		t->target = ent->target;
		t->killtarget = ent->killtarget;
//CW++
		t->svflags |= SVF_NOCLIENT;
//CW--
		return;
	}
	
//	Print the message.

	if ((ent->message) && !(activator->svflags & SVF_MONSTER))
	{

//CW++
//		For a trigger_waypoint, the message is displayed to all players.

		if (ent->classname && !Q_stricmp(ent->classname, "trigger_waypoint"))
		{
			for (i = 1; i <= game.maxclients; ++i)
			{
				cl_ent = &g_edicts[i];

				if (!cl_ent->client)
					continue;
				if (!cl_ent->inuse)
					continue;

				gi_centerprintf(cl_ent, "%s", ent->message);
			}
			
			if (ent->noise_index)
				gi.positioned_sound(world->s.origin, world, CHAN_AUTO, ent->noise_index, 1, ATTN_NONE, 0);
			else
				gi.positioned_sound(world->s.origin, world, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NONE, 0);
		}
		else
		{
//CW--
			gi_centerprintf(activator, "%s", ent->message);
			if (ent->noise_index)
				gi.sound(activator, CHAN_AUTO, ent->noise_index, 1, ATTN_NORM, 0);
			else
				gi.sound(activator, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NORM, 0);
		}
	}

//	Kill killtargets.

	if (ent->killtarget)
	{
		t = NULL;
		while ((t = G_Find(t, FOFS(targetname), ent->killtarget)))
		{
			G_FreeEdict(t);
			if (!ent->inuse)
			{
				gi.dprintf("entity was removed while using killtargets\n");
				return;
			}
		}
	}

//	Fire targets.

	if (ent->target)
	{
		t = NULL;
		while ((t = G_Find(t, FOFS(targetname), ent->target)))
		{
			// doors fire area portals in a specific way
			if (!Q_stricmp(t->classname, "func_areaportal") && (!Q_stricmp(ent->classname, "func_door") || !Q_stricmp(ent->classname, "func_door_rotating")))
				continue;

			if (t == ent)
				gi.dprintf("WARNING: Entity used itself.\n");
			else
			{
				if (t->use)
					t->use(t, ent, activator);
			}
			if (!ent->inuse)
			{
				gi.dprintf("entity was removed while using targets\n");
				return;
			}
		}
	}
}


/*
=============
TempVector

This is just a convenience function
for making temporary vectors for function calls
=============
*/
float *tv(float x, float y, float z)
{
	static	int		index;
	static	vec3_t	vecs[8];
	float	*v;

	// use an array so that multiple tempvectors won't collide for a while
	v = vecs[index];
	index = (index + 1)&7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}


/*
=============
VectorToString

This is just a convenience function for printing vectors.
=============
*/
char *vtos(vec3_t v)
{
	static	int		index;
	static	char	str[8][32];
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf(s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);
	return s;
}

//CW++
/*
=============
VectorToString - floating point

Prints out vector components to 3dp.
=============
*/
char *vtosf(vec3_t v)
{
	static	int		index;
	static	char	str[8][64];
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf(s, 64, "(%.3f %.3f %.3f)", v[0], v[1], v[2]);
	return s;
}
//CW--


vec3_t VEC_UP		= {0.0F, -1.0F, 0.0F};
vec3_t MOVEDIR_UP	= {0.0F, 0.0F, 1.0F};
vec3_t VEC_DOWN		= {0.0F, -2.0F, 0.0F};
vec3_t MOVEDIR_DOWN	= {0.0F, 0.0F, -1.0F};

void G_SetMovedir(vec3_t angles, vec3_t movedir)
{
	if (VectorCompare(angles, VEC_UP))
		VectorCopy(MOVEDIR_UP, movedir);
	else if(VectorCompare (angles, VEC_DOWN))
		VectorCopy(MOVEDIR_DOWN, movedir);
	else
		AngleVectors(angles, movedir, NULL, NULL);

	VectorClear(angles);
}


float vectoyaw(vec3_t vec)
{
	float yaw;
	
	if (/* vec[YAW] == 0 && */ vec[PITCH] == 0.0) 
	{
		yaw = 0.0;
		if (vec[YAW] > 0.0)
			yaw = 90.0;
		else if (vec[YAW] < 0.0)
			yaw = -90.0;
	}
	else
	{
		yaw = (int)(RAD2DEG(atan2(vec[YAW], vec[PITCH])));											//CW
		if (yaw < 0.0)
			yaw += 360.0;
	}

	return yaw;
}

void vectoangles(vec3_t value1, vec3_t angles)
{
	float	forward;
	float	yaw;
	float	pitch;
	
	if ((value1[1] == 0.0) && (value1[0] == 0.0))
	{
		yaw = 0.0F;
		if (value1[2] > 0.0)
			pitch = 90.0;
		else
			pitch = 270.0;
	}
	else
	{
		if (value1[0])
			yaw = (int)(RAD2DEG(atan2(value1[1], value1[0])));										//CW
		else if (value1[1] > 0.0)
			yaw = 90.0;
		else
			yaw = -90.0;
		if (yaw < 0)
			yaw += 360.0;

		forward = sqrt(value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (int)(RAD2DEG(atan2(value1[2], forward)));											//CW	
		if (pitch < 0.0)
			pitch += 360.0;
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0.0;
}

// Knightmare added
void vectoangles2 (vec3_t value1, vec3_t angles)
{
	float	forward;
	float	yaw, pitch;
	
	if (value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		if (value1[0])
			yaw = (atan2(value1[1], value1[0]) * 180 / M_PI);
		else if (value1[1] > 0)
			yaw = 90;
		else
			yaw = 270;

		if (yaw < 0)
			yaw += 360;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}
// end Knightmare

char *G_CopyString(char *in)
{
	size_t	outSize;
	char *out;
	
//	out = gi.TagMalloc((int)strlen(in)+1, TAG_LEVEL);
	outSize = strlen(in) + 1;
	out = gi.TagMalloc(outSize, TAG_LEVEL);
//	strcpy(out, in);
	Com_strcpy(out, outSize, in);
	return out;
}

void G_InitEdict(edict_t *e)
{
	e->inuse = true;
	e->classname = "noclass";
	e->gravity = 1.0;
	e->s.number = e - g_edicts;
}

/*
=================
G_Spawn

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
edict_t *G_Spawn(void)
{
	edict_t	*e;
	int		i;

	e = &g_edicts[(int)maxclients->value + 1];
	for (i = (int)maxclients->value + 1; i < globals.num_edicts; i++, e++)
	{
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (!e->inuse && ((level.time - e->freetime > 0.5) || (e->freetime < 2.0)))
		{
			G_InitEdict(e);
			return e;
		}
	}
	
	if (i == game.maxentities)
		gi.error("ED_Alloc: no free edicts");
		
	globals.num_edicts++;
	G_InitEdict(e);

	return e;
}

/*
=================
G_FreeEdict

Marks the edict as free
=================
*/
void G_FreeEdict(edict_t *ed)
{
	gi.unlinkentity(ed);		// unlink from world

	if ((ed - g_edicts) <= ((int)maxclients->value + BODY_QUEUE_SIZE))
	{
		gi.dprintf("BUG: tried to free special edict (%d)\n\n", (ed - g_edicts));
//CW++
		if (ed->classname)
			gi.dprintf("ed->classname = %s\n", ed->classname);
		gi.dprintf("ed->movetype = %d\n", ed->movetype);
		gi.dprintf("ed->solid = %d\n", ed->solid);
		if (ed->model)
			gi.dprintf("ed->model = %s\n", ed->model);
		if (ed->client)
			gi.dprintf("ed->client->name = %s\n", ed->client->pers.netname);
		gi.dprintf("ed->burning = %s\n", (ed->burning)?"TRUE":"FALSE");
		gi.dprintf("ed->disintegrated = %s\n", (ed->disintegrated)?"TRUE":"FALSE");

		if (ed->owner && ed->owner->client)
			gi.dprintf("owner->name = %s\n", ed->owner->client->pers.netname);
		if (ed->owner && ed->owner->classname)
			gi.dprintf("owner->classname = %s\n", ed->owner->classname);

		if (ed->enemy)
		{
			if (ed->enemy->client)
				gi.dprintf("enemy->name = %s\n", ed->enemy->client->pers.netname);
			if (ed->enemy->classname)
				gi.dprintf("enemy->classname = %s\n", ed->enemy->classname);
			gi.dprintf("enemy->burning = %s\n", (ed->enemy->burning)?"TRUE":"FALSE");
			gi.dprintf("enemy->disintegrated = %s\n", (ed->enemy->disintegrated)?"TRUE":"FALSE");
		}

		if (ed->oldenemy)
		{
			if (ed->oldenemy->client)
				gi.dprintf("oldenemy->name = %s\n", ed->oldenemy->client->pers.netname);
			if (ed->oldenemy->classname)
				gi.dprintf("oldenemy->classname = %s\n", ed->oldenemy->classname);
			gi.dprintf("oldenemy->burning = %s\n", (ed->oldenemy->burning)?"TRUE":"FALSE");
			gi.dprintf("oldenemy->disintegrated = %s\n", (ed->oldenemy->disintegrated)?"TRUE":"FALSE");
		}

		gi.dprintf("----------------------------\n\n");
//CW--
		return;
	}

	memset(ed, 0, sizeof(*ed));
	ed->classname = "freed";
	ed->freetime = level.time;
	ed->inuse = false;
}

//CW++
void trigger_push_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
//CW--

/*
============
G_TouchTriggers

============
*/
void G_TouchTriggers(edict_t *ent)
{
	static edict_t	*touch[MAX_EDICTS];	// Knightmare- made static due to stack size
	edict_t			*hit;
	int				num;
	int				i;

	// dead things don't activate triggers!
	if ((ent->client || (ent->svflags & SVF_MONSTER)) && (ent->health <= 0))
		return;

	num = gi.BoxEdicts(ent->absmin, ent->absmax, touch, MAX_EDICTS, AREA_TRIGGERS);

	// be careful - it is possible to have an entity in this list removed before we get to it (killtriggered)
	for (i = 0; i < num; ++i)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (!hit->touch)
			continue;
//DH++
		if (ent->client && ent->client->spycam)
			continue;
//DH--

//CW++
		if (ent->isabot && (hit->touch == trigger_push_touch))
			ent->client->movestate |= STS_TRIGPUSH;
//CW--
		hit->touch(hit, ent, NULL, NULL);
	}
}

/*
============
G_TouchSolids

Call after linking a new trigger in during gameplay
to force all entities it covers to immediately touch it
============
*/
void G_TouchSolids(edict_t *ent)
{
	static edict_t	*touch[MAX_EDICTS];	// Knightmare- made static due to stack size
	edict_t			*hit;
	int				num;
	int				i;

	num = gi.BoxEdicts(ent->absmin, ent->absmax, touch, MAX_EDICTS, AREA_SOLID);

	// be careful, it is possible to have an entity in this list removed before we get to it (killtriggered)
	for (i = 0; i < num; ++i)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;

		if (ent->touch)
			ent->touch(hit, ent, NULL, NULL);

		if (!ent->inuse)
			break;
	}
}


/*
=================
KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
qboolean KillBox(edict_t *ent)
{
	trace_t tr;

//CW++
	trace_t	tr_check;
	edict_t	*kill_ent;
//CW--

	while (1)
	{
		tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, ent->s.origin, NULL, MASK_PLAYERSOLID);

//CW++
//		If we're inside a solid, do a point check on our origin to see if 
//		we're also inside another player (this fixes the "buried teleporters" bug).

		if (tr.startsolid)
		{
			tr_check = gi.trace(ent->s.origin, vec3_origin, vec3_origin, ent->s.origin, NULL, MASK_PLAYERSOLID);
			if (!tr_check.ent)
				break;
			else
				kill_ent = tr_check.ent;
		}
		else
		{
			if (!tr.ent)
				break;
			else
				kill_ent = tr.ent;
		}
//CW--
		// nail it
		T_Damage(kill_ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);	//CW

		// if we didn't kill it, fail
		if (kill_ent->solid)																		//CW
			return false;
	}

	return true;		// all clear
}

// Knightmare added
void GameDirRelativePath (const char *filename, char *output, size_t outputSize)
{
#ifdef KMQUAKE2_ENGINE_MOD
	Com_sprintf(output, outputSize, "%s/%s", gi.GameDir(), filename);
#else	// KMQUAKE2_ENGINE_MOD
	cvar_t	*basedir, *gamedir;

	basedir = gi.cvar("basedir", "", 0);
	gamedir = gi.cvar("gamedir", "", 0);
	if (strlen(gamedir->string))
		Com_sprintf(output, outputSize, "%s/%s/%s", basedir->string, gamedir->string, filename);
	else
		Com_sprintf(output, outputSize, "%s/baseq2/%s", basedir->string, filename);
#endif	// KMQUAKE2_ENGINE_MOD
}

void SavegameDirRelativePath (const char *filename, char *output, size_t outputSize)
{
#ifdef KMQUAKE2_ENGINE_MOD
	Com_sprintf(output, outputSize, "%s/%s", gi.SaveGameDir(), filename);
#else	// KMQUAKE2_ENGINE_MOD
	cvar_t	*basedir, *gamedir;

	basedir = gi.cvar("basedir", "", 0);
	gamedir = gi.cvar("gamedir", "", 0);
	if (strlen(gamedir->string))
		Com_sprintf(output, outputSize, "%s/%s/%s", basedir->string, gamedir->string, filename);
	else
		Com_sprintf(output, outputSize, "%s/baseq2/%s", basedir->string, filename);
#endif	// KMQUAKE2_ENGINE_MOD
}

void CreatePath (const char *path)
{
#ifdef KMQUAKE2_ENGINE_MOD
	gi.CreatePath (path);
#else	// KMQUAKE2_ENGINE_MOD
	char	tmpBuf[MAX_OSPATH];
	char	*ofs;

	if (strstr(path, "..") || strstr(path, "::") || strstr(path, "\\\\") || strstr(path, "//"))
	{
		gi.dprintf("WARNING: refusing to create relative path '%s'\n", path);
		return;
	}
	Com_strcpy (tmpBuf, sizeof(tmpBuf), path);

	for (ofs = tmpBuf+1 ; *ofs ; ofs++)
	{
		if (*ofs == '/' || *ofs == '\\')
		{	// create the directory
			*ofs = 0;
			_mkdir (tmpBuf);
			*ofs = '/';
		}
	}
#endif	// KMQUAKE2_ENGINE_MOD
}
// end Knightmare

/*
=============
visible

returns 1 if the entity is visible to self, even if not infront()
=============
*/
qboolean visible(edict_t *self, edict_t *other)														//CW
{
	vec3_t	spot1;
	vec3_t	spot2;
	trace_t	trace;

	VectorCopy(self->s.origin, spot1);
	spot1[2] += self->viewheight;
	VectorCopy(other->s.origin, spot2);
	spot2[2] += other->viewheight;
	trace = gi.trace(spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE);
	
	if (trace.fraction == 1.0)
		return true;

	return false;
}


/*
=============
infront

returns 1 if the entity is in front (in sight) of self
=============
*/
qboolean infront(edict_t *self, edict_t *other)														//CW
{
	vec3_t	vec;
	vec3_t	forward;
	float	dot;
	
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorSubtract(other->s.origin, self->s.origin, vec);
	VectorNormalize(vec);
	dot = DotProduct(vec, forward);
	
	if (dot > 0.3)
		return true;

	return false;
}


//CW++
/*
============
VecRange

Returns the magnitude of the vector between two points.
============
*/
float VecRange(vec3_t start, vec3_t end)
{
	vec3_t	vec;

	VectorSubtract(end, start, vec);
	return VectorLength(vec);
}


/*
============
TList_DelNode

Deletes a node from a player's linked list of C4/Trap entities
============
*/
void TList_DelNode(edict_t *ent)
{
	edict_t	*index;
	edict_t	*prev;

//	Sanity checks.

	if (!ent->owner)
	{
		gi.dprintf("BUG: Entity with no owner passed to TList_DelNode().\nPlease contact musashi@planetquake.com\n");
		return;
	}

	if (!ent->owner->next_node)
	{
		gi.dprintf("BUG: TList_DelNode() called for an empty list.\nPlease contact musashi@planetquake.com\n");
		return;
	}

	if ((ent->owner->next_node->die != C4_DieFromDamage) && (ent->owner->next_node->die != Trap_DieFromDamage))
	{
		gi.dprintf("BUG: Invalid pointer passed to TList_DelNode().\nPlease contact musashi@planetquake.com\n");
		return;
	}

//	Find the node.

	index = ent->owner->next_node;
	prev = ent->owner->next_node;
	while (ent != index)
	{
		if (!index)
			return;

		prev = index;
		index = index->next_node;
	}

//	Remove the node from the list, and reroute the linking.

	if (index == ent->owner->next_node)	//first in chain
		ent->owner->next_node = ent->owner->next_node->next_node;
	else if (!index->next_node)			//last in chain
		prev->next_node = NULL;
	else
	{
		prev->next_node = index->next_node;
		index->next_node->prev_node = prev;
	}
	
	--ent->owner->client->resp.nodes_active;
}

/*
============
TList_AddNode

Adds a node to a player's linked list of C4/Trap entities
============
*/
void TList_AddNode(edict_t *ent)
{
	edict_t	*index;

//	Sanity checks.

	if (!ent->owner)
	{
		gi.dprintf("BUG: Entity with no owner passed to TList_AddNode().\nPlease contact musashi@planetquake.com\n");
		return;
	}

	if (ent->owner->next_node && (ent->owner->next_node->die != C4_DieFromDamage) && (ent->owner->next_node->die != Trap_DieFromDamage))
	{
		gi.dprintf("BUG: Invalid pointer passed to TList_AddNode().\nPlease contact musashi@planetquake.com\n");
		return;
	}

//	If the player currently has their maximum number of C4/Traps active, pop the oldest (first) one.

	if (ent->owner->client->resp.nodes_active >= (int)sv_traps_max_active->value)
	{
		if (ent->owner->next_node->die == C4_DieFromDamage)
			C4_Die(ent->owner->next_node);
		else
			Trap_Die(ent->owner->next_node);			
	}

//	Add the new node to the end of the list.
	
	if (ent->owner->next_node)
	{
		index = ent->owner->next_node;
		while (index->next_node)
			index = index->next_node;

		index->next_node = ent;
	}
	else
		ent->owner->next_node = ent;

	++ent->owner->client->resp.nodes_active;
}

/*
=================
PrintFFAScores

Sort the player scores for DM/FFA games, 
and print them to the server console.
=================
*/
void PrintFFAScores(void)
{
	edict_t		*cl_ent;
	gclient_t	*cl;
	int			sorted[MAX_CLIENTS];
	int			sortedscores[MAX_CLIENTS];
	int			score;
	int			total;
	int			i;
	int			j;
	int			k;

	gi.dprintf("-------------------------\n");
	gi.dprintf("Player Scores:\n\n");

//	Sort the clients according to score.

	total = 0;
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->spectator)
			continue;

		score = game.clients[i].resp.score;
		for (j = 0; j < total; j++)
		{
			if (score > sortedscores[j])
				break;
		}

		for (k = total; k > j; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}

		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

//	Write the sorted list to the server console.

	for (i = 0; i < total; i++)
	{
		cl = &game.clients[sorted[i]];
		gi.dprintf(" %-16.16s  : %3d\n", cl->pers.netname, cl->resp.score);
	}

//	Write the spectator list to the server console.

	gi.dprintf("-------------------------\n");
	gi.dprintf("Spectators:\n\n");
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

		if (cl_ent->client->spectator)
			gi.dprintf(" %-16.16s\n", cl_ent->client->pers.netname);
	}

	gi.dprintf("-------------------------\n");
}

/*
=================
PrintTeamScores

Sort the player scores by team, 
and print them to the server console.
=================
*/
void PrintTeamScores(void)
{
	edict_t		*cl_ent;
	gclient_t	*cl;
	int			sorted[2][MAX_CLIENTS];
	int			sortedscores[2][MAX_CLIENTS];
	int			score;
	int			total[2];
	int			totalscore[2];
	int			team;
	int			i;
	int			j;
	int			k;

	gi.dprintf("-------------------------\n");

//	Sort the clients according to team and score.

	total[0] = total[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->spectator)
			continue;

		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			team = 0;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			team = 1;
		else
			continue;

		score = game.clients[i].resp.score;
		for (j = 0; j < total[team]; j++)
		{
			if (score > sortedscores[team][j])
				break;
		}

		for (k = total[team]; k > j; k--)
		{
			sorted[team][k] = sorted[team][k-1];
			sortedscores[team][k] = sortedscores[team][k-1];
		}

		sorted[team][j] = i;
		sortedscores[team][j] = score;
		totalscore[team] += score;
		total[team]++;
	}

//	Write the sorted lists to the server console...
//	Team 1's scores.

	gi.dprintf("Red Team Scores   : %4d\n\n", totalscore[0]);
	for (i = 0; i < total[0]; i++)
	{
		cl = &game.clients[sorted[0][i]];
		gi.dprintf(" %-16.16s : %4d\n", cl->pers.netname, cl->resp.score);
	}

//	Team 2's scores.

	gi.dprintf("-------------------------\n");
	gi.dprintf("Blue Team Scores  : %4d\n\n", totalscore[1]);
	for (i = 0; i < total[1]; i++)
	{
		cl = &game.clients[sorted[1][i]];
		gi.dprintf(" %-16.16s : %4d\n", cl->pers.netname, cl->resp.score);
	}

//	Write the spectator list to the server console.

	gi.dprintf("-------------------------\n");
	gi.dprintf("Spectators:\n\n");
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

		if (cl_ent->client->spectator)
			gi.dprintf(" %-16.16s\n", cl_ent->client->pers.netname);
	}

	gi.dprintf("-------------------------\n");
}

/*
============
dm

"Debug Message" - displays a single line of text in the server console
(used for development/debugging purposes).
============
*/
void dm(char *msg)
{
	gi.dprintf("%s", msg);
}

/*
===============================
File opening functions
===============================
*/
FILE* OpenMaplistFile(qboolean report)
{
	FILE	*iostream;
	cvar_t	*game;
	char	filename[MAX_OSPATH];

	if (strlen(sv_map_file->string) == 0)
		return NULL;

	game = gi.cvar("game", "", 0);
	if (!*game->string)
		Com_sprintf(filename, sizeof(filename), "%s/%s", GAMEVERSION, sv_map_file->string);
	else
		Com_sprintf(filename, sizeof(filename), "%s/%s", game->string, sv_map_file->string);

	iostream = fopen(filename, "r");
	if (report)
	{
		if (iostream != NULL)
			gi.dprintf("\n>> Map list \"%s\" opened\n", filename);
		else
			gi.dprintf("\n** Failed to open \"%s\"\n", filename);
	}

	return iostream;
}

FILE* OpenConfiglistFile(qboolean report)
{
	FILE	*iostream;
	cvar_t	*game;
	char	filename[MAX_OSPATH];

	if (strlen(sv_config_file->string) == 0)
		return NULL;

	game = gi.cvar("game", "", 0);
	if (!*game->string)
		Com_sprintf(filename, sizeof(filename), "%s/%s", GAMEVERSION, sv_config_file->string);
	else
		Com_sprintf(filename, sizeof(filename), "%s/%s", game->string, sv_config_file->string);

	iostream = fopen(filename, "r");
	if (report)
	{
		if (iostream != NULL)
			gi.dprintf("\n>> Config list \"%s\" opened\n", filename);
		else
			gi.dprintf("\n** Failed to open \"%s\"\n", filename);
	}

	return iostream;
}

FILE* OpenAGMDropFile(qboolean report, qboolean readonly)
{
	FILE	*iostream;
	cvar_t	*game;
	char	filename[MAX_OSPATH];

	if (strlen(sv_agm_drop_file->string) == 0)
		return NULL;

	game = gi.cvar("game", "", 0);
	if (!*game->string)
		Com_sprintf(filename, sizeof(filename), "%s/%s", GAMEVERSION, sv_agm_drop_file->string);
	else
		Com_sprintf(filename, sizeof(filename), "%s/%s", game->string, sv_agm_drop_file->string);

	if (readonly)
		iostream = fopen(filename, "r");
	else
		iostream = fopen(filename, "a+");

	if (report)
	{
		if (iostream != NULL)
			gi.dprintf("\n>> AGM drop file \"%s\" opened\n", filename);
		else
			gi.dprintf("\n** Failed to open \"%s\"\n", filename);
	}

	return iostream;
}

FILE* OpenDiscLauncherDropFile(qboolean report, qboolean readonly)
{
	FILE	*iostream;
	cvar_t	*game;
	char	filename[MAX_OSPATH];

	if (strlen(sv_disc_drop_file->string) == 0)
		return NULL;

	game = gi.cvar("game", "", 0);
	if (!*game->string)
		Com_sprintf(filename, sizeof(filename), "%s/%s", GAMEVERSION, sv_disc_drop_file->string);
	else
		Com_sprintf(filename, sizeof(filename), "%s/%s", game->string, sv_disc_drop_file->string);

	if (readonly)
		iostream = fopen(filename, "r");
	else
		iostream = fopen(filename, "a+");

	if (report)
	{
		if (iostream != NULL)
			gi.dprintf("\n>> Disc Launcher drop file \"%s\" opened\n", filename);
		else
			gi.dprintf("\n** Failed to open \"%s\"\n", filename);
	}

	return iostream;
}

FILE* OpenBotConfigFile(qboolean report, qboolean readonly)
{
	FILE	*iostream;
	cvar_t	*game;
	char	filename[MAX_OSPATH];

	if (strlen(sv_bots_config_file->string) == 0)
		return NULL;

	game = gi.cvar("game", "", 0);
	if (!*game->string)
		Com_sprintf(filename, sizeof(filename), "%s/%s", GAMEVERSION, sv_bots_config_file->string);
	else
		Com_sprintf(filename, sizeof(filename), "%s/%s", game->string, sv_bots_config_file->string);

	if (readonly)
		iostream = fopen(filename, "r");
	else
		iostream = fopen(filename, "a+");

	if (report)
	{
		if (iostream != NULL)
			gi.dprintf("\n>> Bot config file \"%s\" opened\n", filename);
		else
			gi.dprintf("\n** Failed to open \"%s\"\n", filename);
	}

	return iostream;
}
//CW--

//DH++
//CW: These functions were written by David Hyde for the Lazarus mod.

edict_t	*LookingAt(edict_t *ent, int filter, vec3_t endpos, float *range)
{
	edict_t			*who;
	static edict_t	*trigger[MAX_EDICTS];	// Knightmare- made static due to stack size
	edict_t			*ignore;
	trace_t			tr;
	vec_t			r;
	vec3_t			end;
	vec3_t			forward;
	vec3_t			start;
	vec3_t			dir;
	vec3_t			entp;
	vec3_t			mins;
	vec3_t			maxs;
	int				i;
	int				num;

	if (!ent->client)
	{
		if (endpos)
			VectorClear(endpos);

		if (range)
			*range = 0;

		return NULL;
	}

	VectorClear(end);
	if (ent->client->spycam)
	{
		AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
		VectorCopy(ent->s.origin, start);
		ignore = ent->client->spycam;
	}
	else
	{
		AngleVectors(ent->client->v_angle, forward, NULL, NULL);
		VectorCopy(ent->s.origin, start);
		start[2] += ent->viewheight;
		ignore = ent;
	}

	VectorMA(start, WORLD_SIZE, forward, end);	// was 8192.0
	
	// First check for looking directly at a pickup item
	VectorSet(mins, -4096.0, -4096.0, -4096.0);
	VectorSet(maxs,  4096.0,  4096.0,  4096.0);
	num = gi.BoxEdicts(mins, maxs, trigger, MAX_EDICTS, AREA_TRIGGERS);
	for (i = 0; i < num; ++i)
	{
		who = trigger[i];
		if (!who->inuse)
			continue;
		if (!who->item)
			continue;
		if (!visible(ent,who))
			continue;
		if (!infront(ent,who))
			continue;

		VectorSubtract(who->s.origin,start,dir);
		r = VectorLength(dir);
		VectorMA(start, r, forward, entp);

		if (entp[0] < who->s.origin[0] - 17.0)
			continue;
		if (entp[1] < who->s.origin[1] - 17.0)
			continue;
		if (entp[2] < who->s.origin[2] - 17.0)
			continue;
		if (entp[0] > who->s.origin[0] + 17.0)
			continue;
		if (entp[1] > who->s.origin[1] + 17.0)
			continue;
		if (entp[2] > who->s.origin[2] + 17.0)
			continue;

		if (endpos)
			VectorCopy(who->s.origin,endpos);

		if (range)
			*range = r;

		return who;
	}

	tr = gi.trace(start, NULL, NULL, end, ignore, MASK_SHOT);
	if (tr.fraction == 1.0)
	{
		// too far away
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NORM, 0);
		return NULL;
	}

	if (!tr.ent)
	{
		// no hit
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NORM, 0);
		return NULL;
	}

	if (!tr.ent->classname)
	{
		// should never happen
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NORM, 0);
		return NULL;
	}

	if ((strstr(tr.ent->classname, "func_") != NULL) && (filter & LOOKAT_NOBRUSHMODELS))
	{
		// don't hit on brush models
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NORM, 0);
		return NULL;
	}

	if ((Q_stricmp(tr.ent->classname, "worldspawn") == 0) && (filter & LOOKAT_NOWORLD))
	{
		// world brush
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NORM, 0);
		return NULL;
	}

	if (endpos)
	{
		endpos[0] = tr.endpos[0];
		endpos[1] = tr.endpos[1];
		endpos[2] = tr.endpos[2];
	}

	if (range)
	{
		VectorSubtract(tr.endpos,start,start);
		*range = VectorLength(start);
	}

	return tr.ent;
}

void AnglesNormalize(vec3_t vec)
{
	while(vec[0] > 180.0)
		vec[0] -= 360.0;

	while(vec[0] < -180.0)
		vec[0] += 360.0;

	while(vec[1] > 360.0)
		vec[1] -= 360.0;

	while(vec[1] < 0.0)
		vec[1] += 360.0;
}

float SnapToEights(float x)
{
	x *= 8.0;
	if (x > 0.0)
		x += 0.5;
	else
		x -= 0.5;

	return 0.125 * (int)x;
}

/*
===============================
Checks that the specified file is inside a PAK.
===============================
*/
qboolean InPak(char *basedir, char *gamedir, char *filename)
{
	char			pakfile[MAX_OSPATH];
	FILE			*f;
	pak_header_t	pakheader;
	pak_item_t		pakitem;
	qboolean		found = false;
	int				k;
	int				kk;
	int				num;
	int				numitems;

//	Search paks in the game folder.

	for (k = 9; (k >= 0) && !found; k--)
	{
	//	strcpy(pakfile, basedir);
		Com_strcpy(pakfile, sizeof(pakfile), basedir);
		if (strlen(gamedir))
		{
			Com_strcat(pakfile, sizeof(pakfile), "/");
			Com_strcat(pakfile, sizeof(pakfile), gamedir);
		}
		Com_strcat(pakfile, sizeof(pakfile), va("/pak%d.pak", k));

		if (NULL != (f = fopen(pakfile, "rb")))
		{
			num = (int)fread(&pakheader, 1, sizeof(pak_header_t), f);
			if (num >= sizeof(pak_header_t))
			{
				if ((pakheader.id[0] == 'P') && (pakheader.id[1] == 'A') && (pakheader.id[2] == 'C') && (pakheader.id[3] == 'K'))
				{
					numitems = pakheader.dsize / sizeof(pak_item_t);
					fseek(f, pakheader.dstart, SEEK_SET);
					for (kk = 0; (kk < numitems) && !found; kk++)
					{
						fread(&pakitem, 1, sizeof(pak_item_t), f);
						if (!Q_stricmp(pakitem.name, filename))
							found = true;
					}
				}
			}
			fclose(f);
		}
	}

	return found;
}
//DH--

//CW++
/*
===============================
Checks that the specified file exists.
NB: This function assumes that the file specified as 'checkname' has no extension.

This is based on code written by David Hyde for the Lazarus mod.
===============================
*/
qboolean FileExists(char *checkname, filetype_t ftype)
{
	FILE	*fstream;
	cvar_t	*basedir;
	cvar_t	*gamedir;
	char	filename[MAX_OSPATH];
	char	path[MAX_OSPATH];

	basedir = gi.cvar("basedir", "", 0);
	gamedir = gi.cvar("gamedir", "", 0);

	switch (ftype)
	{
		case FILE_MAP:
			Com_sprintf(path, sizeof(path), "maps/%s.bsp", checkname);
			break;

		case FILE_MODEL:
			Com_sprintf(path, sizeof(path), "models/%s.md2", checkname);
			break;

		case FILE_SOUND:
			Com_sprintf(path, sizeof(path), "sound/%s.wav", checkname);
			break;

		case FILE_TEXTURE:
			Com_sprintf(path, sizeof(path), "textures/%s.wal", checkname);
			break;

		default:
			Com_sprintf(path, sizeof(path), "%s", checkname);
			break;
	}

	if (strlen(gamedir->string))
	{

//		Search in the game directory for external file.

		Com_sprintf(filename, sizeof(filename), "%s/%s/%s", basedir->string, gamedir->string, path);
		if ((fstream = fopen(filename, "r")) != NULL)
		{
			fclose(fstream);
			return true;
		}
		
//		Search paks in the game directory.

		if (InPak(basedir->string, gamedir->string, path))
			return true;
	}
	
//	Search in the 'baseq2' directory for external file.

	Com_sprintf(filename, sizeof(filename), "%s/baseq2/%s", basedir->string, path);
	if ((fstream = fopen(filename, "r")) != NULL)
	{
		fclose(fstream);
		return true;
	}
	
//	Search paks in the 'baseq2' directory.

	if (InPak(basedir->string, "baseq2", path))
		return true;

	return false;
}
//CW--

//Maj++
void gi_cprintf(edict_t *ent, int printlevel, char *fmt, ...)
{
	va_list	argptr;
	char	bigbuffer[0x10000];
	int		len;

	if (!ent || !ent->inuse || !ent->client)
		return;

	// bots don't get messages
	if (ent->isabot)
		return;

	va_start(argptr, fmt);
//	len = vsprintf(bigbuffer, fmt, argptr);
	len = Q_vsnprintf(bigbuffer, sizeof(bigbuffer), fmt, argptr);

//CW++	//r1
//	Check for overflow.

	if (len > sizeof(bigbuffer))
	{
		gi.dprintf("String too long for gi_bprintf().\n");
		return;
	}
//CW--

	va_end(argptr);

	gi.cprintf(ent, printlevel, "%s", bigbuffer);													//r1: format string exploit fix
}


void gi_centerprintf(edict_t *ent, char *fmt, ...)
{
	va_list	argptr;
	char	bigbuffer[0x10000];
	int		len;

	if (!ent || !ent->inuse || !ent->client)
		return;

	// bots don't get messages
	if (ent->isabot)
		return;

	va_start(argptr, fmt);
//	len = vsprintf(bigbuffer, fmt, argptr);
	len = Q_vsnprintf(bigbuffer, sizeof(bigbuffer), fmt, argptr);

//CW++	//r1
//	Check for overflow.

	if (len > sizeof(bigbuffer))
	{
		gi.dprintf("String too long for gi_bprintf().\n");
		return;
	}
//CW--

	va_end(argptr);

	gi.centerprintf(ent, "%s", bigbuffer);															//r1: format string exploit fix
}


void gi_bprintf(int printlevel, char *fmt, ...)
{
	edict_t	*ent;
	va_list	argptr;
	char	bigbuffer[0x10000];
	int		len;
	int		i;

	va_start(argptr, fmt);
//	len = vsprintf(bigbuffer, fmt, argptr);
	len = Q_vsnprintf(bigbuffer, sizeof(bigbuffer), fmt, argptr);

//CW++	//r1
//	Check for overflow.

	if (len > sizeof(bigbuffer))
	{
		gi.dprintf("String too long for gi_bprintf().\n");
		return;
	}
//CW--

	va_end(argptr);

	if (dedicated->value)		
		gi.cprintf(NULL, printlevel, "%s", bigbuffer);												//r1: format string exploit fix

	for (i = 0; i < game.maxclients; i++)
	{
		ent = g_edicts + 1 + i;
		gi_cprintf(ent, printlevel, "%s", bigbuffer);												//r1: format string exploit fix
	}
}
//Maj--


//r1++
#define MASK_VOLUME			1
#define MASK_ATTENUATION	2
#define MASK_POSITION		4
#define MASK_ENTITY_CHANNEL	8
#define MASK_TIMEOFS		16

void unicastSound (edict_t *player, int soundIndex, float volume)
{
	int mask = MASK_ENTITY_CHANNEL;

	// Knightmare- don't send this to bots- causes fatal server error
	if (!player || player->isabot)
		return;

	if (volume != 1.0)
		mask |= MASK_VOLUME;

	gi.WriteByte(svc_sound);
	gi.WriteByte((byte)mask);
	gi.WriteByte((byte)soundIndex);

	if (mask & MASK_VOLUME)
		gi.WriteByte((byte)(volume * 255));

	gi.WriteShort(((player - g_edicts - 1) << 3) + CHAN_NO_PHS_ADD);
	gi.unicast(player, true);
}
//r1--
