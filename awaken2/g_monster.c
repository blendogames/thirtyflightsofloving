// g_monster.c

#include "g_local.h"

#define	STEPSIZE	18																				//CW

/*
===============
M_CheckGround
===============
*/
void M_CheckGround(edict_t *ent)
{
	vec3_t	point;
	trace_t	trace;

	if (ent->flags & (FL_SWIM|FL_FLY))
		return;

//Maj++
	ResetGroundSlope(ent);
//Maj--

	if (ent->velocity[2] > 100.0)
	{
		ent->groundentity = NULL;
		return;
	}

	// if the hull point one-quarter unit down is solid the entity is on ground
	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
	point[2] = ent->s.origin[2] - 0.25;

	trace = gi.trace(ent->s.origin, ent->mins, ent->maxs, point, ent, MASK_MONSTERSOLID);

	// check steepness
	if ((trace.plane.normal[2] < 0.7) && !trace.startsolid)
	{
		ent->groundentity = NULL;
		return;
	}

/*	//Maj - code superseded
	if (!trace.startsolid && !trace.allsolid)
	{
		VectorCopy(trace.endpos, ent->s.origin);
		ent->groundentity = trace.ent;
		ent->groundentity_linkcount = trace.ent->linkcount;
		ent->velocity[2] = 0.0;
	}
*/

//Maj++
	TraceAllSolid(ent, point, trace);
//Maj--
}


/*
===============
M_CatagorizePosition
===============
*/
void M_CatagorizePosition(edict_t *ent)
{
	vec3_t	point;
	int		cont;

//	Get the current waterlevel.

	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
	point[2] = ent->s.origin[2] + ent->mins[2] + 1.0;	
	cont = gi.pointcontents(point);

	if (!(cont & MASK_WATER))
	{
		ent->waterlevel = 0;
		ent->watertype = 0;
		return;
	}

	ent->watertype = cont;
	ent->waterlevel = 1;
	point[2] += 26.0;
	cont = gi.pointcontents(point);
	if (!(cont & MASK_WATER))
		return;

	ent->waterlevel = 2;
	point[2] += 22.0;
	cont = gi.pointcontents(point);
	if (cont & MASK_WATER)
		ent->waterlevel = 3;
}


/*
===============
M_DropToFloor
===============
*/
void M_DropToFloor(edict_t *ent)
{
	vec3_t	end;
	trace_t	trace;

	ent->s.origin[2] += 1.0;
	VectorCopy(ent->s.origin, end);
	end[2] -= 256.0;
	
	trace = gi.trace(ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

	if ((trace.fraction == 1.0) || trace.allsolid)
		return;

	VectorCopy(trace.endpos, ent->s.origin);
	gi.linkentity(ent);

	M_CheckGround(ent);
	M_CatagorizePosition(ent);
}

/*
=============
M_CheckBottom

Returns false if any part of the bottom of the entity is off an edge that
is not a staircase.
=============
*/
qboolean M_CheckBottom(edict_t *ent)																//CW
{
	vec3_t	mins;
	vec3_t	maxs;
	vec3_t	start;
	vec3_t	stop;
	trace_t	trace;
	float	mid;
	float	bottom;
	int		x;
	int		y;
	
	VectorAdd(ent->s.origin, ent->mins, mins);
	VectorAdd(ent->s.origin, ent->maxs, maxs);

//	If all of the points under the corners are solid world, don't bother
//	with the tougher checks; the corners must be within 16 units of the midpoint.

	start[2] = mins[2] - 1;
	for	(x = 0; x <= 1; x++)
	{
		for	(y = 0; y <= 1; y++)
		{
			start[0] = (x) ? maxs[0] : mins[0];
			start[1] = (y) ? maxs[1] : mins[1];
			if (gi.pointcontents(start) != CONTENTS_SOLID)
				goto realcheck;
		}
	}

	return true;		// we got out easy

realcheck:
//	Check it for real...

	start[2] = mins[2];
	
//	The midpoint must be within 16 units of the bottom.

	start[0] = stop[0] = (mins[0] + maxs[0]) * 0.5;
	start[1] = stop[1] = (mins[1] + maxs[1]) * 0.5;
	stop[2] = start[2] - 2 * STEPSIZE;
	trace = gi.trace(start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID);

	if (trace.fraction == 1.0)
		return false;

	mid = bottom = trace.endpos[2];
	
//	The corners must be within 16 units of the midpoint.

	for	(x = 0; x <= 1; x++)
	{
		for	(y = 0; y <= 1; y++)
		{
			start[0] = stop[0] = x ? maxs[0] : mins[0];
			start[1] = stop[1] = y ? maxs[1] : mins[1];
			trace = gi.trace(start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID);
			if ((trace.fraction != 1.0) && (trace.endpos[2] > bottom))
				bottom = trace.endpos[2];

			if ((trace.fraction == 1.0) || (mid - trace.endpos[2] > STEPSIZE))
				return false;
		}
	}

	return true;
}


/*
=============
SV_MoveStep

Called by monster program code.
The move will be adjusted for slopes and stairs, but if the move isn't
possible, no move is done, false is returned, and
pr_global_struct->trace_normal is set to the normal of the blocking wall
=============
*/
//FIXME since we need to test end position contents here, can we avoid doing
//it again later in catagorize position?

qboolean SV_MoveStep(edict_t *ent, vec3_t move, qboolean relink)									//CW
{
	trace_t	trace;
	vec3_t	oldorg;
	vec3_t	neworg;
	vec3_t	end;
	vec3_t	test;
	float	dz;
	float	stepsize;
	int		contents;
	int		i;

//	Try the move.

	VectorCopy(ent->s.origin, oldorg);
	VectorAdd(ent->s.origin, move, neworg);

//	Flying monsters don't step up.

	if (ent->flags & (FL_SWIM | FL_FLY))
	{
	
		// try one move with vertical motion, then one without
		for (i = 0; i < 2; i++)
		{
			VectorAdd(ent->s.origin, move, neworg);
			if ((i == 0) && ent->enemy)
			{
				if (!ent->goalentity)
					ent->goalentity = ent->enemy;
				dz = ent->s.origin[2] - ent->goalentity->s.origin[2];
				if (ent->goalentity->client)
				{
					if (dz > 40.0)
						neworg[2] -= 8.0;

					if (!((ent->flags & FL_SWIM) && (ent->waterlevel < 2)))
						if (dz < 30.0)
							neworg[2] += 8.0;
				}
				else
				{
					if (dz > 8.0)
						neworg[2] -= 8.0;
					else if (dz > 0.0)
						neworg[2] -= dz;
					else if (dz < -8.0)
						neworg[2] += 8.0;
					else
						neworg[2] += dz;
				}
			}
			trace = gi.trace(ent->s.origin, ent->mins, ent->maxs, neworg, ent, MASK_MONSTERSOLID);
	
			// fly monsters don't enter water voluntarily
			if (ent->flags & FL_FLY)
			{
				if (!ent->waterlevel)
				{
					test[0] = trace.endpos[0];
					test[1] = trace.endpos[1];
					test[2] = trace.endpos[2] + ent->mins[2] + 1.0;
					contents = gi.pointcontents(test);
					if (contents & MASK_WATER)
						return false;
				}
			}

			// swim monsters don't exit water voluntarily
			if (ent->flags & FL_SWIM)
			{
				if (ent->waterlevel < 2)
				{
					test[0] = trace.endpos[0];
					test[1] = trace.endpos[1];
					test[2] = trace.endpos[2] + ent->mins[2] + 1.0;
					contents = gi.pointcontents(test);
					if (!(contents & MASK_WATER))
						return false;
				}
			}

			if (trace.fraction == 1.0)
			{
				VectorCopy(trace.endpos, ent->s.origin);
				if (relink)
				{
					gi.linkentity(ent);
					G_TouchTriggers(ent);
				}
				return true;
			}
			
			if (!ent->enemy)
				break;
		}
		
		return false;
	}

//	Push down from a step height above the wished position.

	if (!(ent->monsterinfo.aiflags & AI_NOSTEP))
		stepsize = STEPSIZE;
	else
		stepsize = 1;

	neworg[2] += stepsize;
	VectorCopy(neworg, end);
	end[2] -= stepsize * 2.0;

	trace = gi.trace(neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

	if (trace.allsolid)
		return false;

	if (trace.startsolid)
	{
		neworg[2] -= stepsize;
		trace = gi.trace(neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);
		if (trace.allsolid || trace.startsolid)
			return false;
	}


//	Don't go into water.

	if (ent->waterlevel == 0)
	{
		test[0] = trace.endpos[0];
		test[1] = trace.endpos[1];
		test[2] = trace.endpos[2] + ent->mins[2] + 1.0;	
		contents = gi.pointcontents(test);

		if (contents & MASK_WATER)
			return false;
	}

	if (trace.fraction == 1.0)
	{
		// if monster had the ground pulled out, go ahead and fall
		if (ent->flags & FL_PARTIALGROUND)
		{
			VectorAdd(ent->s.origin, move, ent->s.origin);
			if (relink)
			{
				gi.linkentity(ent);
				G_TouchTriggers(ent);
			}
			ent->groundentity = NULL;
			return true;
		}
	
		return false;		// walked off an edge
	}

//	Check point traces down for dangling corners.

	VectorCopy(trace.endpos, ent->s.origin);
	
	if (!M_CheckBottom(ent))
	{
		if (ent->flags & FL_PARTIALGROUND)
		{	// entity had floor mostly pulled out from underneath it and is trying to correct
			if (relink)
			{
				gi.linkentity(ent);
				G_TouchTriggers(ent);
			}
			return true;
		}
		VectorCopy(oldorg, ent->s.origin);
		return false;
	}

	if (ent->flags & FL_PARTIALGROUND)
		ent->flags &= ~FL_PARTIALGROUND;

	ent->groundentity = trace.ent;
	ent->groundentity_linkcount = trace.ent->linkcount;

//	The move is OK.

	if (relink)
	{
		gi.linkentity(ent);
		G_TouchTriggers(ent);
	}

	return true;
}

/*
===============
M_WalkMove
===============
*/
qboolean M_WalkMove(edict_t *ent, float yaw, float dist)											//CW
{
	vec3_t	move;
	
	if (!ent->groundentity && !(ent->flags & (FL_FLY|FL_SWIM)))
		return false;

	yaw = DEG2RAD(yaw);																				//CW
	move[0] = cos(yaw) * dist;
	move[1] = sin(yaw) * dist;
	move[2] = 0.0;

	return SV_MoveStep(ent, move, true);
}
