/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_utils.c,v $
 *   $Revision: 1.14 $
 *   $Date: 2002/07/16 01:44:04 $
 *  
 ***********************************

Copyright (C) 2002 Vipersoft

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// g_utils.c -- misc utility functions for game module

#include "g_local.h"

void change_stance(edict_t *self, int stance);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);

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

	if (!from)
		from = g_edicts;
	else
		from++;

	for ( ; from < &g_edicts[globals.num_edicts] ; from++)
	{
		if (!from->inuse)
			continue;
		s = *(char **) ((byte *)from + fieldofs);
		if (!s)
			continue;
		if (!Q_stricmp (s, match))
			return from;
	}

	return NULL;
}

edict_t *G_Find_Team (edict_t *from, size_t fieldofs, char *match, int team)	// Knightmare- changed fieldofs from int
{
	char	*s;
	edict_t *e;
	qboolean gotspot;

	gotspot = false;

	//find if team already has inf_re_start
    for (e = g_edicts; e < &g_edicts[globals.num_edicts]; e++)
    {
		if (!e->inuse)
			continue;

		if (strcmp(e->classname, "info_reinforcements_start"))
			continue;
		if (e->obj_owner == team)
			gotspot = true;
	}



	if (!from)
		from = g_edicts;
	else
		from++;

	for ( ; from < &g_edicts[globals.num_edicts] ; from++)
	{
		if (!from->inuse)
			continue;
		s = *(char **) ((byte *)from + fieldofs);
		if (!s)
			continue;
		if (from->obj_owner != team && (gotspot || from->obj_owner != -1))
			continue;
		if (!Q_stricmp (s, match))
		{
			return from;
		}
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

//
// check for a delay
//
	if (ent->delay && ent->classnameb != OBJECTIVE_TOUCH)
	{
	// create a temp object to fire at a later time
		t = G_Spawn();
		t->classname = "DelayedUse";
		t->nextthink = level.time + ent->delay;
		t->think = Think_Delay;
		t->activator = activator;
		if (!activator)
			gi.dprintf ("Think_Delay with no activator\n");
		t->message = ent->message;
		t->target = ent->target;
		t->killtarget = ent->killtarget;
		return;
	}

	
//
// print the message
//
	if ((ent->message) && !(activator->svflags & SVF_MONSTER)  &&
		strncmp(ent->classname, "objective", 9))
	{
		safe_centerprintf (activator, "%s", ent->message);
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
			if (!Q_stricmp(t->classname, "func_areaportal") &&
				(ent->classnameb == FUNC_DOOR || ent->classnameb == FUNC_DOOR_ROTATING))
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
	
	outSize = strlen(in)+1;
	out = gi.TagMalloc (outSize, TAG_LEVEL);
	Q_strncpyz (out, outSize, in);
	return out;
}


void G_InitEdict (edict_t *e)
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

	G_FreeAI (ed); //jabot092(2)
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
	static edict_t	*touch[MAX_EDICTS];
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
		if (!hit)
			continue;
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
	static	edict_t	*touch[MAX_EDICTS];
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

	while (1)
	{
		tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, ent->s.origin, NULL, MASK_PLAYERSOLID);
		if (!tr.ent)
			break;

		// nail it
		T_Damage (tr.ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_SPAWNCAMP);

		// if we didn't kill it, fail
		if (tr.ent->solid)
			return false;
	}

	return true;		// all clear
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
		Q_strncpyz (buffer, sizeof(buffer), savegamepath->string);
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
	Q_strncpyz (tmpBuf, sizeof(tmpBuf), path);

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

qboolean G_CopyFile (const char *src_filename, const char *dst_filename)
{
	int			partSize;
	FILE		*fp_src, *fp_dst;
	byte		buf[8192];

	fp_src = fopen (src_filename, "rb");
	if ( !fp_src ) {
		return false;
	}

	fp_dst = fopen (dst_filename, "wb");
	if ( !fp_dst ) {
		fclose (fp_src);
		return false;
	}

	do {
		partSize = (int)fread (&buf, 1, sizeof(buf), fp_dst);
		if (partSize > 0)
			fwrite (&buf, 1, partSize, fp_dst);
	} while (partSize > 0);

	fclose (fp_src);
	fclose (fp_dst);

	return true;
}
// end Knightmare

/************************************************************************************
**  These commands deal with Teams
**
************************************************************************************/

void ClientSetMaxSpeed (edict_t *ent, qboolean sync);
void ClientCvarSync (edict_t *ent, qboolean dump);


qboolean OnSameTeam(edict_t *self,edict_t *target)
{
	if (!self ||
		!target ||
		!self->client ||
		!target->client ||
		!self->client->resp.team_on ||
		!target->client->resp.team_on ||
		!self->client->resp.mos ||
		!target->client->resp.mos)
		return false;

	if (self->client->resp.team_on->index == target->client->resp.team_on->index)
		return true;
	else
		return false;
}
			

/* The funny thing about WeighPlayer:

    The normal weight for a player with a knife and fists only is -29.0.
	The weight before speed affected is 35.0.

	That gives us 64 units (hereafter pounds) to work with. With large things (like flamethrowers)
	that are 70+ pounds, this actually works out great.  Obviously, 70 > 64, therefore speed is
	reduced. 

	However, most "experts" say that you should carry only 20% of your body weight. Without a doubt
	this rule is almost always broken in war.  *BUT* If you did the calculations, 64 is 20% of a
	hefty 320 pounds.

	So in order for soldiers in D-Day: Normandy to "adhere" to the carry standard, they'd have to
	all be 320 pounds.  Furthermore, when I look at our models we use, these soldiers look perfectly
	healthy.  I'd say they weight in at a good 175 _at most_.  
	
	Unfortunately, 20% of 175 pounts  is just 35 pounds. That's no fun, especially with a large
	weapon like the flamethrower, so I opt for you guys in later versions to make a soldier that 
	actually respects his body and doesn't carry over 20%.. Do me a favor and call him "Billy Bob" =)
*/

qboolean WeighPlayer(edict_t *ent)
{
	int i;
	float weight	 = 0;
	float normweight = 0;
	//this just saves some typing	
	

//	if( ent->ai)
//	{ent->client->speedmod = 1;
//	return false;}

	if (!ent || !ent->client ||
		ent->client->ping >= 999 ||
		!ent->client->resp.team_on ||
		!ent->client->resp.mos ||
		ent->client->limbo_mode ||
		!ent->client->resp.team_on->mos[ent->client->resp.mos]) 
		return false;






	ent->client->speedmod = ent->client->resp.team_on->mos[ent->client->resp.mos]->speed_mod;
	normweight = ent->client->resp.team_on->mos[ent->client->resp.mos]->normal_weight;

	for(i=0;i<MAX_ITEMS;i++)
		if(ent->client->pers.inventory[i])
			weight+=((itemlist[i].weight) * (ent->client->pers.inventory[i]));

	weight -= normweight; 	//subtract normal carring weight for mos from weight.
	
	// pbowens: this was a funny fix. originally used = instead of *=
	ent->client->speedmod *= normweight/((weight > normweight)?weight:normweight);
	

	if(ent->stanceflags==STANCE_DUCK)
		ent->client->speedmod *= 0.60;
	if(ent->stanceflags==STANCE_CRAWL)
	{
	//	if (ent->waterlevel ==3)//swimming
		//	ent->client->speedmod *= 1.1;		
		//else
			ent->client->speedmod *= 0.45;		
	}	

	if( (ent->wound_location & LEG_WOUND) /*&& (!ent->arty_time)*/ )
		ent->client->speedmod *= 0.75;
	
	if(ent->waterlevel)
	{
		if (ent->waterlevel==1)//feet are in the water
			ent->client->speedmod*=0.95;
		else if (ent->waterlevel==2)//waist is in the water
			ent->client->speedmod*=0.80;
		else if(ent->waterlevel==3)//whole body is in the water
		{	
			ent->client->speedmod*=0.55;

			if (ent->stanceflags == STANCE_CRAWL)
			{	ent->client->speedmod*=3;
				//gi.dprintf("sdfjlksdfjkl\n");
			}
		}

	}  

	if(weight>ent->client->resp.team_on->mos[ent->client->resp.mos]->max_weight)
	{
		ent->client->speedmod *= 0.5;
		return false;

		//safe_centerprintf(ent, "You weigh too much! Drop some items out of your inventory.\n");
		//change_stance(ent, STANCE_CRAWL);
	}

	ClientSetMaxSpeed (ent, true);

	return true;
}


// based off of Com_sprintf
void centerprintall (char *mesg, ...)
{
	int		i, len, size;
	edict_t *ent;
	va_list	argptr;
	char	buffer[0x10000];
	char	print[0x10000];

	size = sizeof(print);

	va_start (argptr, mesg);
//	len = vsprintf (buffer, mesg, argptr);
	len = Q_vsnprintf (buffer, sizeof(buffer), mesg, argptr);
	va_end (argptr);

	// erm this should never happen at all but it's here incase
	if (len >= size)
		Com_Printf ("centerprintall: overflow of %i in %i\n", len, size);

	strncpy (print, buffer, size-1);
	
	for (i = 1; i <= game.maxclients; i++)
	{
			ent = &g_edicts[i];
			if (!ent->inuse)
				continue;
			if (!ent->client)
				continue;

			safe_centerprintf(ent,print);
	}
}

qboolean IsValidPlayer(edict_t *ent) {

	if (ent && 
		ent->client && 
		ent->client->resp.team_on && 
		ent->client->resp.mos &&
		!ent->flyingnun)
		return true;
	else
		return false;
}

/*
char *skinDir (char *s, char *extra)
{
	static	char string[512];	// use two buffers so compares
	static	char *val;

	if (extra)
		Com_sprintf (string, sizeof(string), "players/%s", s);
	else
		Com_sprintf (string, sizeof(string), "%s", s);

	val = &string[0];

	while (*val != '/')
	{
		val++;
	}

	if (extra)
	{
		val++;
		while (*val != '/')
		{
			val++;
		}
		val++;
	}
	*val = 0;

	if (extra)
		Com_sprintf (string, sizeof(string), "%s%s", string, extra);
	else
		Com_sprintf (string, sizeof(string), "%s", string);


	return &string[0];
}
*/
