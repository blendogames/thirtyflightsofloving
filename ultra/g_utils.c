// g_utils.c -- misc utility functions for game module

#include "g_local.h"

#ifdef _WIN32
#include <direct.h>	// Knightmare added
#endif

void G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
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

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
edict_t *G_Find (edict_t *from, size_t fieldofs, char *match)	// Knightmare- changed fieldofs from int
{
	char	*s;

	if ( !from )
		from = g_edicts;
	else
		from++;

	for ( ; from < &g_edicts[globals.num_edicts] ; from++)
	{
		if (!from->inuse)
			continue;
		s = *(char **) ((byte *)from + fieldofs);
		if ( !s )
			continue;
		if ( !Q_stricmp (s, match) )
			return from;
	}

	return NULL;
}


/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
edict_t *findradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

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
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
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

edict_t *G_PickTarget (char *targetname)
{
	edict_t	*ent = NULL;
	int		num_choices = 0;
	edict_t	*choice[MAXCHOICES];

	if (!targetname)
	{
		gi.dprintf("G_PickTarget called with NULL targetname\n");
		return NULL;
	}

	while(1)
	{
		ent = G_Find (ent, FOFS(targetname), targetname);
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



void Think_Delay (edict_t *ent)
{
	G_UseTargets (ent, ent->activator);
	G_FreeEdict (ent);
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
void G_UseTargets (edict_t *ent, edict_t *activator)
{
	edict_t		*t;
	edict_t		*master;
	qboolean	done = false;

//
// check for a delay
//
	if (ent->delay)
	{
	// create a temp object to fire at a later time
		t = G_Spawn();
		t->classname = "DelayedUse";
		t->nextthink = level.time + ent->delay;
		t->think = Think_Delay;
		t->activator = activator;
		if (!activator)
			gi.dprintf ("MAP DESIGN ERROR: Think_Delay with no activator\n");
		t->message = ent->message;
		t->target = ent->target;
		t->killtarget = ent->killtarget;
		return;
	}


//
// print the message
//
	if ((ent->message) && !(activator->svflags & SVF_MONSTER))
	{
		if (!activator->bot_client)
			gi.centerprintf (activator, "%s", ent->message);
		if (ent->noise_index)
			gi.sound (activator, CHAN_AUTO, ent->noise_index, 1, ATTN_NORM, 0);
		else
			gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}

//
// kill killtargets
//
	if (ent->killtarget)
	{
		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), ent->killtarget)))
		{
			// PMM - if this entity is part of a train, cleanly remove it
			if ( (Q_stricmp(level.mapname, "biggun") == 0) || (Q_stricmp(level.mapname, "city2") == 0) )
			{}
			else if (t->flags & FL_TEAMSLAVE)
			{
//				if ((g_showlogic) && (g_showlogic->value))
//					gi.dprintf ("Removing %s from train!\n", t->classname);

				if (t->teammaster)
				{
					master = t->teammaster;
					while (!done)
					{
						if (master->teamchain == t)
						{
							master->teamchain = t->teamchain;
							done = true;
						}
						master = master->teamchain;
						if (!master)
						{
//							if ((g_showlogic) && (g_showlogic->value))
//								gi.dprintf ("Couldn't find myself in master's chain, ignoring!\n");
						}
					}
				}
				else
				{
//					if ((g_showlogic) && (g_showlogic->value))
//						gi.dprintf ("No master to free myself from, ignoring!\n");
				}
			}
			// PMM
			G_FreeEdict (t);
			if (!ent->inuse)
			{
				gi.dprintf("entity was removed while using killtargets\n");
				return;
			}
		}
	}

//
// fire targets
//
	if (ent->target)
	{
		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), ent->target)))
		{
			// doors fire area portals in a specific way
			if ( !Q_stricmp(t->classname, "func_areaportal") &&
				( !Q_stricmp(ent->classname, "func_door") || !Q_stricmp(ent->classname, "func_door_rotating") ) )
				continue;

			if (t == ent)
			{
				gi.dprintf ("WARNING: Entity used itself.\n");
			}
			else
			{
				if (t->use)
					t->use (t, ent, activator);
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
float	*tv (float x, float y, float z)
{
	static	int		index;
	static	vec3_t	vecs[8];
	float	*v;

	// use an array so that multiple tempvectors won't collide
	// for a while
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

This is just a convenience function
for printing vectors
=============
*/
char	*vtos (vec3_t v)
{
	static	int		index;
	static	char	str[8][32];
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf (s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

	return s;
}


vec3_t VEC_UP		= {0, -1, 0};
vec3_t MOVEDIR_UP	= {0, 0, 1};
vec3_t VEC_DOWN		= {0, -2, 0};
vec3_t MOVEDIR_DOWN	= {0, 0, -1};

void G_SetMovedir (vec3_t angles, vec3_t movedir)
{
	if (VectorCompare (angles, VEC_UP))
	{
		VectorCopy (MOVEDIR_UP, movedir);
	}
	else if (VectorCompare (angles, VEC_DOWN))
	{
		VectorCopy (MOVEDIR_DOWN, movedir);
	}
	else
	{
		AngleVectors (angles, movedir, NULL, NULL);
	}

	VectorClear (angles);
}


float vectoyaw (vec3_t vec)
{
	float	yaw;

	if (vec[YAW] == 0 && vec[PITCH] == 0)
		yaw = 0;
	else
	{
		yaw = (int) (atan2(vec[YAW], vec[PITCH]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;
	}

	return yaw;
}


void vectoangles (vec3_t value1, vec3_t angles)
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
		yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (int) (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}

char *G_CopyString (char *in)
{
	char	*out;
	size_t	outSize;

//	out = gi.TagMalloc (strlen(in)+1, TAG_LEVEL);
	outSize = strlen(in)+1;
	out = gi.TagMalloc (outSize, TAG_LEVEL);
	Com_strcpy (out, outSize, in);
	return out;
}


void G_InitEdict (edict_t *e)
{
	e->inuse = true;
	e->classname = "noclass";
	e->gravity = 1.0;
	e->s.number = e - g_edicts;

	e->trail_index = 0;
	e->closest_trail = 0;
	e->ignore_time = 0;
	e->last_reached_trail = 0;
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
edict_t *G_Spawn (void)
{
	int			i;
	edict_t		*e;

	e = &g_edicts[(int)maxclients->value+1];
	for ( i=maxclients->value+1 ; i<globals.num_edicts ; i++, e++)
	{
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (!e->inuse && ( e->freetime < 2 || level.time - e->freetime > 0.5 ) )
		{
			G_InitEdict (e);
			return e;
		}
	}

	if (i == game.maxentities)
		gi.error ("ED_Alloc: no free edicts");

	globals.num_edicts++;
	G_InitEdict (e);
	return e;
}

/*
=================
G_FreeEdict

Marks the edict as free
=================
*/
void G_FreeEdict (edict_t *ed)
{
	gi.unlinkentity (ed);		// unlink from world

	if ((ed - g_edicts) <= (maxclients->value + BODY_QUEUE_SIZE))
	{
//		gi.dprintf("tried to free special edict\n");
		return;
	}

	memset (ed, 0, sizeof(*ed));
	ed->classname = "freed";
	ed->freetime = level.time;
	ed->inuse = false;
}


/*
============
G_TouchTriggers

============
*/
void	G_TouchTriggers (edict_t *ent)
{
	int				i, num;
	static edict_t	*touch[MAX_EDICTS];	// Knightmare- made static due to stack size
	edict_t			*hit;

	// dead things don't activate triggers!
	if ((ent->client || (ent->svflags & SVF_MONSTER)) && (ent->health <= 0))
		return;

	num = gi.BoxEdicts (ent->absmin, ent->absmax, touch, MAX_EDICTS, AREA_TRIGGERS);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (!hit->touch)
			continue;
		hit->touch (hit, ent, NULL, NULL);
	}
}

/*
============
G_TouchSolids

Call after linking a new trigger in during gameplay
to force all entities it covers to immediately touch it
============
*/
void	G_TouchSolids (edict_t *ent)
{
	int				i, num;
	static edict_t	*touch[MAX_EDICTS];	// Knightmare- made static due to stack size
	edict_t			*hit;

	num = gi.BoxEdicts (ent->absmin, ent->absmax, touch, MAX_EDICTS, AREA_SOLID);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (ent->touch)
			ent->touch (hit, ent, NULL, NULL);
		if (!ent->inuse)
			break;
	}
}




/*
==============================================================================

Kill box

==============================================================================
*/

/*
=================
KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
qboolean KillBox (edict_t *ent)
{
	trace_t		tr;
	int			i;
	edict_t		*trav;

	gi.unlinkentity(ent);

	while (1)
	{
		tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, ent->s.origin, NULL, MASK_PLAYERSOLID);
		if (!tr.ent)
			break;

		// nail it
		T_Damage (tr.ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TRIGGER_HURT);

		// if we didn't kill it, fail
		if (tr.ent->solid)
		{
//			if (tr.ent == world)
				break;

//			gi.dprintf("Couldn't KillBox(), something is wrong.\n");
//			return false;
		}
	}

	// fixes wierd teleporter/spawning inside others
	i = -1;
	while (++i < num_players)
	{
		if ((trav = players[i]) == ent)
			continue;

		if (entdist(trav, ent) > 42)
			continue;

		// nail it
		T_Damage (trav, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TRIGGER_HURT);
	}

	return true;		// all clear
}


// Ridah
/*
==================
stuffcmd

  QC equivalent, sends a command to the client's consol
==================
*/
void stuffcmd(edict_t *ent, char *text)
{
	gi.WriteByte(11);				// 11 = svc_stufftext
	gi.WriteString(text);
	gi.unicast(ent, 1);
}

float	entdist(edict_t *ent1, edict_t *ent2)
{
	vec3_t	vec;

/*
	if (!ent2)
	{
		gi.dprintf("entdist called with NULL source-ent\n");
		return 0;
	}

	if (!ent2)
	{
		gi.dprintf("entdist called with NULL dest-ent\n");
		return 0;
	}
*/

	VectorSubtract(ent1->s.origin, ent2->s.origin, vec);
	return VectorLength(vec);
}

/*
=================
AddModelSkin

Adds a skin reference to an .md2 file, saving as filename.md2new
=================
*/
void AddModelSkin (char *modelfile, char *skinname)
{
	FILE	*f, *out;
	int		/*count = 0,*/ buffer_int, i;
	char	filename[256], infilename[256];
	char	buffer;

//	i = sprintf(infilename, modelfile);
	Com_strcpy (infilename, sizeof(infilename), modelfile);

	f = fopen (infilename, "rb");
	if (!f)
	{
		gi.dprintf("Cannot open file %s\n", infilename);
		return;
	}
/*
	i =  sprintf(filename, ".\\");
	i += sprintf(filename + i, GAMEVERSION);
	i += sprintf(filename + i, "\\");
	i += sprintf(filename + i, modelfile);
	i += sprintf(filename + i, "new");
*/
	Com_strcpy (filename, sizeof(filename), ".\\");
	Com_strcat (filename, sizeof(filename), GAMEVERSION);
	Com_strcat (filename, sizeof(filename), "\\");
	Com_strcat (filename, sizeof(filename), modelfile);
	Com_strcat (filename, sizeof(filename), "new");

	out = fopen (filename, "wb");
	if (!out)
		return;

	// mirror header stuff before skinnum
	for (i=0; i<5; i++)
	{
		fread(&buffer_int, sizeof(buffer_int), 1, f);
		fwrite(&buffer_int, sizeof(buffer_int), 1, out);
	}

	// increment skinnum
	fread(&buffer_int, sizeof(buffer_int), 1, f);
	++buffer_int;
	fwrite(&buffer_int, sizeof(buffer_int), 1, out);

	// mirror header stuff before skin_ofs
	for (i=0; i<5; i++)
	{
		fread(&buffer_int, sizeof(buffer_int), 1, f);
		fwrite(&buffer_int, sizeof(buffer_int), 1, out);
	}

	// copy the skins offset value, since it doesn't change
	fread(&buffer_int, sizeof(buffer_int), 1, f);
	fwrite(&buffer_int, sizeof(buffer_int), 1, out);

	// increment all offsets by 64 to make way for new skin
	for (i=0; i<5; i++)
	{
		fread(&buffer_int, sizeof(buffer_int), 1, f);
		buffer_int += 64;
		fwrite(&buffer_int, sizeof(buffer_int), 1, out);
	}

	// write the new skin
	for (i=0; i<strlen(skinname); i++)
	{
		fwrite(&(skinname[i]), 1, 1, out);
	}

	buffer = '\0';
	fwrite(&buffer, 1, 1, out);
	buffer = 'x';
	fwrite(&buffer, 1, 1, out);

	buffer = '\0';
	for (i = ((int)strlen(skinname)+2); i<64; i++)
	{
		fwrite(&buffer, 1, 1, out);
	}

	// copy the rest of the file
	fread(&buffer, sizeof(buffer), 1, f);
	while (!feof(f))
	{
		fwrite(&buffer, sizeof(buffer), 1, out);
		fread(&buffer, sizeof(buffer), 1, f);
	}

	fclose (f);
	fclose (out);

	// copy the new file over the old file
	remove(infilename);
	rename(filename, infilename);

	gi.dprintf("Model skin added.\n", filename);
}

void my_bprintf (int printlevel, char *fmt, ...)
{
	int i;
	char	bigbuffer[0x10000];
	int		len;
	va_list		argptr;
	edict_t	*cl_ent;

	va_start (argptr,fmt);
//	len = vsprintf (bigbuffer,fmt,argptr);
	len = Q_vsnprintf (bigbuffer, sizeof(bigbuffer), fmt, argptr);
	va_end (argptr);

	if (dedicated->value)
	//	gi.cprintf(NULL, printlevel, bigbuffer);
		gi.cprintf(NULL, printlevel, "%s", bigbuffer);

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || cl_ent->bot_client)
			continue;

	//	gi.cprintf(cl_ent, printlevel, bigbuffer);
		gi.cprintf(cl_ent, printlevel, "%s", bigbuffer);
	}
/*
	for (i=0; i<num_players; i++)
	{
		if (!players[i]->bot_client)
			gi.cprintf(players[i], printlevel, "%s", bigbuffer);
	}
*/
}

//======================================================================
// New ACE-compatible message routines

// botsafe cprintf
void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...)
{
	char bigbuffer[0x10000];
	va_list  argptr;
	int len;

	if (!ent->inuse || ent->isbot || ent->bot_client)
	  return;

	va_start (argptr,fmt);
//	len = vsprintf (bigbuffer,fmt,argptr);
	len = Q_vsnprintf (bigbuffer, sizeof(bigbuffer), fmt, argptr);
	va_end (argptr);

//	gi.cprintf(ent, printlevel, bigbuffer);
	gi.cprintf(ent, printlevel, "%s", bigbuffer);
}

// botsafe centerprintf
void safe_centerprintf (edict_t *ent, char *fmt, ...)
{
	char bigbuffer[0x10000];
	va_list  argptr;
	int len;

	if (!ent->inuse || ent->isbot || ent->bot_client)
	  return;

	va_start (argptr,fmt);
//	len = vsprintf (bigbuffer,fmt,argptr);
	len = Q_vsnprintf (bigbuffer, sizeof(bigbuffer), fmt, argptr);
	va_end (argptr);

//	gi.centerprintf(ent, bigbuffer);
	gi.centerprintf(ent, "%s", bigbuffer);
}

// botsafe bprintf
void safe_bprintf (int printlevel, char *fmt, ...)
{
	int i;
	char bigbuffer[0x10000];
	int  len;
	va_list  argptr;
	edict_t *cl_ent;

	va_start (argptr,fmt);
//	len = vsprintf (bigbuffer,fmt,argptr);
	len = Q_vsnprintf (bigbuffer, sizeof(bigbuffer), fmt, argptr);
	va_end (argptr);


	if (dedicated->value)
	//	gi.cprintf(NULL, printlevel, bigbuffer);
		gi.cprintf(NULL, printlevel, "%s", bigbuffer);

	// This is to be compatible with Eraser (ACE)
	//for (i=0; i<num_players; i++)
	//{
	// Ridah, changed this so CAM works
	for (i=0 ; i<maxclients->value ; i++)
	{
		cl_ent = g_edicts + 1 + i;

		if (cl_ent->inuse && !cl_ent->bot_client && !cl_ent->isbot)
		//	gi.cprintf(cl_ent, printlevel, bigbuffer);
			gi.cprintf(cl_ent, printlevel, "%s", bigbuffer);
	}
}


// rogue - WHY DIDNT THEY COMMENT THIS

void G_ProjectSource2 (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t up, vec3_t result)
{
	result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1] + up[0] * distance[2];
	result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1] + up[1] * distance[2];
	result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] + up[2] * distance[2];
}

float vectoyaw2 (vec3_t vec)
{
	float	yaw;

	// PMM - fixed to correct for pitch of 0
	if (/*vec[YAW] == 0 &&*/ vec[PITCH] == 0)
		if (vec[YAW] == 0)
			yaw = 0;
		else if (vec[YAW] > 0)
			yaw = 90;
		else
			yaw = 270;
	else
	{
		yaw = (atan2(vec[YAW], vec[PITCH]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;
	}

	return yaw;
}

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
	// PMM - fixed to correct for pitch of 0
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

// originally from newai

float realrange (edict_t *self, edict_t *other)
{
	vec3_t dir;

	VectorSubtract (self->s.origin, other->s.origin, dir);

	return VectorLength(dir);
}

// Knightmare added
char *GameDir (void)
{
#ifdef KMQUAKE2_ENGINE_MOD
	return gi.GameDir();
#else	// KMQUAKE2_ENGINE_MOD
	static char	buffer[MAX_OSPATH];
	cvar_t		*basedir, *gamedir;

	basedir = gi.cvar("basedir", "", 0);
	gamedir = gi.cvar("gamedir", "", 0);
	if ( strlen(gamedir->string) > 0 )
		Com_sprintf (buffer, sizeof(buffer), "%s/%s", basedir->string, gamedir->string);
	else
		Com_sprintf (buffer, sizeof(buffer), "%s/baseq2", basedir->string);

	return buffer;
#endif	// KMQUAKE2_ENGINE_MOD
}

char *SavegameDir (void)
{
#ifdef KMQUAKE2_ENGINE_MOD
	return gi.SaveGameDir();
#else	// KMQUAKE2_ENGINE_MOD
	static char	buffer[MAX_OSPATH];
	cvar_t	*basedir, *gamedir, *savegamepath;

	basedir = gi.cvar("basedir", "", 0);
	gamedir = gi.cvar("gamedir", "", 0);
	savegamepath = gi.cvar("savegamepath", "", 0);
	// use Unofficial Q2 Patch's savegamepath cvar if set
	if ( strlen(savegamepath->string) > 0 ) {
		Com_strcpy (buffer, sizeof(buffer), savegamepath->string);
	}
	else {
		if ( strlen(gamedir->string) > 0 )
			Com_sprintf (buffer, sizeof(buffer), "%s/%s", basedir->string, gamedir->string);
		else
			Com_sprintf (buffer, sizeof(buffer), "%s/baseq2", basedir->string);
	}

	return buffer;
#endif	// KMQUAKE2_ENGINE_MOD
}

void GameDirRelativePath (const char *filename, char *output, size_t outputSize)
{
#ifdef KMQUAKE2_ENGINE_MOD
	Com_sprintf(output, outputSize, "%s/%s", gi.GameDir(), filename);
#else	// KMQUAKE2_ENGINE_MOD
	cvar_t	*basedir, *gamedir;

	basedir = gi.cvar("basedir", "", 0);
	gamedir = gi.cvar("gamedir", "", 0);
	if ( strlen(gamedir->string) > 0 )
		Com_sprintf (output, outputSize, "%s/%s/%s", basedir->string, gamedir->string, filename);
	else
		Com_sprintf (output, outputSize, "%s/baseq2/%s", basedir->string, filename);
#endif	// KMQUAKE2_ENGINE_MOD
}

void SavegameDirRelativePath (const char *filename, char *output, size_t outputSize)
{
#ifdef KMQUAKE2_ENGINE_MOD
	Com_sprintf(output, outputSize, "%s/%s", gi.SaveGameDir(), filename);
#else	// KMQUAKE2_ENGINE_MOD
	cvar_t	*basedir, *gamedir, *savegamepath;

	basedir = gi.cvar("basedir", "", 0);
	gamedir = gi.cvar("gamedir", "", 0);
	savegamepath = gi.cvar("savegamepath", "", 0);
	// use Unofficial Q2 Patch's savegamepath cvar if set
	if ( strlen(savegamepath->string) > 0 ) {
		Com_sprintf (output, outputSize, "%s/%s", savegamepath->string, filename);
	}
	else {
		if ( strlen(gamedir->string) > 0 )
			Com_sprintf (output, outputSize, "%s/%s/%s", basedir->string, gamedir->string, filename);
		else
			Com_sprintf (output, outputSize, "%s/baseq2/%s", basedir->string, filename);
	}
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

	for (ofs = tmpBuf+1; *ofs; ofs++)
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

qboolean LocalFileExists (const char *path)
{
	char	realPath[MAX_OSPATH];
	FILE	*f;

	SavegameDirRelativePath (path, realPath, sizeof(realPath));
	f = fopen (realPath, "rb");
	if (f) {
		fclose (f);
		return true;
	}
	return false;
}


// Knightmare added
/*
====================
IsIdMap

Checks if the current map is a stock id map,
this is used for certain hacks.
====================
*/
qboolean IsIdMap (void)
{
	if (Q_stricmp(level.mapname, "base1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "base2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "base3") == 0)
		return true;
	if (Q_stricmp(level.mapname, "biggun") == 0)
		return true;
	if (Q_stricmp(level.mapname, "boss1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "boss2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "bunk1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "city1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "city2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "city3") == 0)
		return true;
	if (Q_stricmp(level.mapname, "command") == 0)
		return true;
	if (Q_stricmp(level.mapname, "cool1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "fact1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "fact2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "fact3") == 0)
		return true;
	if (Q_stricmp(level.mapname, "hangar1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "hangar2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "jail1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "jail2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "jail3") == 0)
		return true;
	if (Q_stricmp(level.mapname, "jail4") == 0)
		return true;
	if (Q_stricmp(level.mapname, "jail5") == 0)
		return true;
	if (Q_stricmp(level.mapname, "lab") == 0)
		return true;
	if (Q_stricmp(level.mapname, "mine1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "mine2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "mine3") == 0)
		return true;
	if (Q_stricmp(level.mapname, "mine4") == 0)
		return true;
	if (Q_stricmp(level.mapname, "mintro") == 0)
		return true;
	if (Q_stricmp(level.mapname, "power1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "power2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "security") == 0)
		return true;
	if (Q_stricmp(level.mapname, "space") == 0)
		return true;
	if (Q_stricmp(level.mapname, "strike") == 0)
		return true;
	if (Q_stricmp(level.mapname, "train") == 0)
		return true;
	if (Q_stricmp(level.mapname, "ware1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "ware2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "waste1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "waste2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "waste3") == 0)
		return true;
	if (Q_stricmp(level.mapname, "q2dm1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "q2dm2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "q2dm3") == 0)
		return true;
	if (Q_stricmp(level.mapname, "q2dm4") == 0)
		return true;
	if (Q_stricmp(level.mapname, "q2dm5") == 0)
		return true;
	if (Q_stricmp(level.mapname, "q2dm6") == 0)
		return true;
	if (Q_stricmp(level.mapname, "q2dm7") == 0)
		return true;
	if (Q_stricmp(level.mapname, "q2dm8") == 0)
		return true;
	if (Q_stricmp(level.mapname, "base64") == 0)
		return true;
	if (Q_stricmp(level.mapname, "city64") == 0)
		return true;
	if (Q_stricmp(level.mapname, "sewer64") == 0)
		return true;

	return false;
}


// Knightmare added
/*
====================
IsXatrixMap

Checks if the current map is from the Xatrix mission pack.
This is used for certain hacks.
====================
*/
qboolean IsXatrixMap (void)
{
	if (Q_stricmp(level.mapname, "badlands") == 0)
		return true;
	if (Q_stricmp(level.mapname, "industry") == 0)
		return true;
	if (Q_stricmp(level.mapname, "outbase") == 0)
		return true;
	if (Q_stricmp(level.mapname, "refinery") == 0)
		return true;
	if (Q_stricmp(level.mapname, "w_treat") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xcompnd1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xcompnd2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xhangar1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xhangar2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xintell") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xmoon1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xmoon2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xreactor") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xsewer1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xsewer2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xship") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xswamp") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xware") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xdm1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xdm2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xdm3") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xdm4") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xdm5") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xdm6") == 0)
		return true;
	if (Q_stricmp(level.mapname, "xdm7") == 0)
		return true;

	return false;
}


// Knightmare added
/*
====================
IsRogueMap

Checks if the current map is from the Rogue mission pack.
This is used for certain hacks.
====================
*/
qboolean IsRogueMap (void)
{
	if (Q_stricmp(level.mapname, "rammo1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rammo2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rbase1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rbase2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rboss") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rhangar1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rhangar2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rlava1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rlava2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rmine1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rmine2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rsewer1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rsewer2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rware1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rware2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm3") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm4") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm5") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm6") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm7") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm8") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm9") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm10") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm11") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm12") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm13") == 0)
		return true;
	if (Q_stricmp(level.mapname, "rdm14") == 0)
		return true;

	return false;
}


// Knightmare added
/*
====================
IsZaeroMap

Checks if the current map is from the Zaero mission pack.
This is used for certain hacks.
====================
*/
qboolean IsZaeroMap (void)
{
	if (Q_stricmp(level.mapname, "zbase1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "zbase2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "zboss") == 0)
		return true;
	if (Q_stricmp(level.mapname, "zdef1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "zdef2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "zdef3") == 0)
		return true;
	if (Q_stricmp(level.mapname, "zdef4") == 0)
		return true;
	if (Q_stricmp(level.mapname, "ztomb1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "ztomb2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "ztomb3") == 0)
		return true;
	if (Q_stricmp(level.mapname, "ztomb4") == 0)
		return true;
	if (Q_stricmp(level.mapname, "zwaste1") == 0)
		return true;
	if (Q_stricmp(level.mapname, "zwaste2") == 0)
		return true;
	if (Q_stricmp(level.mapname, "zwaste3") == 0)
		return true;
	return false;
}


/*
====================
CheckCoop_MapHacks

Checks if the entity needs to be modified for coop.
Returns true if entity is to be inhibited.
====================
*/
qboolean CheckCoop_MapHacks (edict_t *ent)
{
	if ( !coop->value || !ent )
		return false;

	if (level.maptype == MAPTYPE_XATRIX)
	{
		if ( !Q_stricmp(level.mapname, "xsewer1") )	// FS: Coop: Progress breaker hack in xsewer1.bsp
		{
			if ( ent->classname && !Q_stricmp(ent->classname, "trigger_relay") && ent->target && !Q_stricmp(ent->target, "t3") && ent->targetname && !Q_stricmp(ent->targetname, "t2") )
			{
				return true;
			}
			if ( ent->classname && !Q_stricmp(ent->classname, "func_button") && ent->target && !Q_stricmp(ent->target, "t16") && ent->model && !Q_stricmp(ent->model, "*71") )
			{
				ent->message = "Overflow valve maintenance\nhatch A opened.";
				return false;
			}
			if ( ent->classname && !Q_stricmp(ent->classname, "trigger_once") && ent->model && !Q_stricmp(ent->model, "*3") )
			{
				ent->message = "Overflow valve maintenance\nhatch B opened.";
				return false;
			}
		}
	}

	return false;
}
