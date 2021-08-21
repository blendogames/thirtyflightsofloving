// m_move.c -- monster movement

#include "g_local.h"

#define	STEPSIZE	18

// this is used for communications out of sv_movestep to say what entity
// is blocking us
edict_t		*new_bad;			//pmm

/*
=============
M_CheckBottom

Returns false if any part of the bottom of the entity is off an edge that
is not a staircase.

=============
*/
int c_yes, c_no;

qboolean M_CheckBottom (edict_t *ent)
{
	vec3_t	mins, maxs, start, stop;
	trace_t	trace;
	int		x, y;
	float	mid, bottom;

	VectorAdd (ent->s.origin, ent->mins, mins);
	VectorAdd (ent->s.origin, ent->maxs, maxs);

// if all of the points under the corners are solid world, don't bother
// with the tougher checks
// the corners must be within 16 of the midpoint

//PGM
#ifdef ROGUE_GRAVITY
	// FIXME - this will only handle 0,0,1 and 0,0,-1 gravity vectors
	start[2] = mins[2] - 1;
	if (ent->gravityVector[2] > 0)
		start[2] = maxs[2] + 1;
#else
	start[2] = mins[2] - 1;
#endif
//PGM

	for	(x=0 ; x<=1 ; x++)
		for	(y=0 ; y<=1 ; y++)
		{
			start[0] = x ? maxs[0] : mins[0];
			start[1] = y ? maxs[1] : mins[1];
			if (gi.pointcontents (start) != CONTENTS_SOLID)
				goto realcheck;
		}

	c_yes++;
	return true;		// we got out easy

realcheck:
	c_no++;
//
// check it for real...
//
	start[2] = mins[2];

// the midpoint must be within 16 of the bottom
	start[0] = stop[0] = (mins[0] + maxs[0])*0.5;
	start[1] = stop[1] = (mins[1] + maxs[1])*0.5;

//PGM
#ifdef ROGUE_GRAVITY
	if (ent->gravityVector[2] < 0)
	{
		start[2] = mins[2];
		stop[2] = start[2] - STEPSIZE - STEPSIZE;
	}
	else
	{
		start[2] = maxs[2];
		stop[2] = start[2] + STEPSIZE + STEPSIZE;
	}
#else
	stop[2] = start[2] - 2*STEPSIZE;
#endif
//PGM

	trace = gi.trace (start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID);

	if (trace.fraction == 1.0)
		return false;
	mid = bottom = trace.endpos[2];

// the corners must be within 16 of the midpoint
	for	(x=0 ; x<=1 ; x++)
		for	(y=0 ; y<=1 ; y++)
		{
			start[0] = stop[0] = x ? maxs[0] : mins[0];
			start[1] = stop[1] = y ? maxs[1] : mins[1];

			trace = gi.trace (start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID);

//PGM
#ifdef ROGUE_GRAVITY
			// FIXME - this will only handle 0,0,1 and 0,0,-1 gravity vectors
			if (ent->gravityVector[2] > 0)
			{
				if (trace.fraction != 1.0 && trace.endpos[2] < bottom)
					bottom = trace.endpos[2];
				if (trace.fraction == 1.0 || trace.endpos[2] - mid > STEPSIZE)
					return false;
			}
			else
			{
				if (trace.fraction != 1.0 && trace.endpos[2] > bottom)
					bottom = trace.endpos[2];
				if (trace.fraction == 1.0 || mid - trace.endpos[2] > STEPSIZE)
					return false;
			}
#else
			if (trace.fraction != 1.0 && trace.endpos[2] > bottom)
				bottom = trace.endpos[2];
			if (trace.fraction == 1.0 || mid - trace.endpos[2] > STEPSIZE)
				return false;
#endif
//PGM
		}

	c_yes++;
	return true;
}

//============
// ROGUE
qboolean IsBadAhead (edict_t *self, edict_t *bad, vec3_t move)
{
	vec3_t	dir;
	vec3_t	forward;
	float	dp_bad, dp_move;
	vec3_t	move_copy;

	VectorCopy (move, move_copy);

	VectorSubtract (bad->s.origin, self->s.origin, dir);
	VectorNormalize (dir);
	AngleVectors (self->s.angles, forward, NULL, NULL);
	dp_bad = DotProduct (forward, dir);

	VectorNormalize (move_copy);
	AngleVectors (self->s.angles, forward, NULL, NULL);
	dp_move = DotProduct (forward, move_copy);

	if ((dp_bad < 0) && (dp_move < 0))
		return true;
	if ((dp_bad > 0) && (dp_move > 0))
		return true;

	return false;
/*
	if (DotProduct(forward, dir) > 0)
	{
//		gi.dprintf ("bad ahead...\n");
		return true;
	}

//	gi.dprintf ("bad behind...\n");
	return false;
	*/
}
// ROGUE
//============

/*
=============
SV_movestep

Called by monster program code.
The move will be adjusted for slopes and stairs, but if the move isn't
possible, no move is done, false is returned, and
pr_global_struct->trace_normal is set to the normal of the blocking wall
=============
*/
// FIXME since we need to test end position contents here, can we avoid doing
// it again later in catagorize position?
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink)
{
	float		dz;
	vec3_t		oldorg, neworg, end;
	trace_t		trace;
	int			i;
	float		stepsize;
	vec3_t		test;
	int			contents;
	edict_t		*current_bad;		// PGM
	float		minheight;			// pmm

	qboolean	canjump;
	float		d1, d2;
	float		jumpheight;
	int			jump;		// 1=jump up, -1=jump down
	vec3_t		forward, up;
	vec3_t		dir;
	vec_t		dist;
	vec_t		g1, g2;
	edict_t		*grenade;
	edict_t		*target;

//======
//PGM
	// PMM - who cares about bad areas if you're dead?
	if (ent->health > 0)
	{
		current_bad = CheckForBadArea(ent);
		if (current_bad)
		{
			ent->bad_area = current_bad;

		//	if ( ent->enemy && !strcmp(ent->enemy->classname, "tesla") )
			if ( ent->enemy && (!strcmp(ent->enemy->classname, "tesla") || !strcmp(ent->enemy->classname, "prox")) )
			{
				// if the tesla or prox is in front of us, back up...
				if (IsBadAhead (ent, current_bad, move))
					VectorScale (move, -1, move);
			}
		}
		else if (ent->bad_area)
		{
			// if we're no longer in a bad area, get back to business.
			ent->bad_area = NULL;
			if (ent->oldenemy)	// && ent->bad_area->owner == ent->enemy)
			{
			//	gi.dprintf("resuming being pissed at %s\n", ent->oldenemy->classname);
				ent->enemy = ent->oldenemy;
				ent->goalentity = ent->oldenemy;
				FoundTarget (ent);

				// Knightmare- added missing relink
				if (relink)
				{
					gi.linkentity (ent);
					G_TouchTriggers (ent);
				}
				// end Knightmare

				// FIXME - remove this when ready!!!
			//	if (ent->lastMoveTime == level.time)
			//		if ((g_showlogic) && (g_showlogic->value))
			//			gi.dprintf ("Duplicate move detected for %s, please tell programmers!\n", ent->classname);
			//	ent->lastMoveTime = level.time;
				// FIXME

				return true;
			}
		}
	}
//PGM
//======

	// try the move
	VectorCopy (ent->s.origin, oldorg);
	VectorAdd (ent->s.origin, move, neworg);

	// Lazarus
	AngleVectors (ent->s.angles, forward, NULL, up);
	if (ent->enemy)
		target = ent->enemy;
	else if (ent->movetarget)
		target = ent->movetarget;
	else
		target = NULL;
	// end Lazarus

	// flying monsters don't step up
	if ( ent->flags & (FL_SWIM | FL_FLY) )
	{
		// try one move with vertical motion, then one without
		for (i = 0; i < 2; i++)
		{
			VectorAdd (ent->s.origin, move, neworg);
			if (i == 0 && ent->enemy)
			{
				if (!ent->goalentity)
					ent->goalentity = ent->enemy;
				dz = ent->s.origin[2] - ent->goalentity->s.origin[2];
				if (ent->goalentity->client)
				{
					// we want the carrier to stay a certain distance off the ground, to help prevent him
					// from shooting his fliers, who spawn in below him
					//
					if (!strcmp(ent->classname, "monster_carrier"))
						minheight = 104;
					else
						minheight = 40;
				//	if (dz > 40)
					if (dz > minheight)	//	pmm
						neworg[2] -= 8;
					if (!((ent->flags & FL_SWIM) && (ent->waterlevel < 2)))
						if (dz < (minheight - 10))
							neworg[2] += 8;
				}
				else
				{
					if (dz > 8)
						neworg[2] -= 8;
					else if (dz > 0)
						neworg[2] -= dz;
					else if (dz < -8)
						neworg[2] += 8;
					else
						neworg[2] += dz;
				}
			}

			trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, neworg, ent, MASK_MONSTERSOLID);

			// fly monsters don't enter water voluntarily
			if (ent->flags & FL_FLY)
			{
				if (!ent->waterlevel)
				{
					test[0] = trace.endpos[0];
					test[1] = trace.endpos[1];
					test[2] = trace.endpos[2] + ent->mins[2] + 1;
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
					test[2] = trace.endpos[2] + ent->mins[2] + 1;
					contents = gi.pointcontents(test);
					if (!(contents & MASK_WATER))
						return false;
				}
			}

		//	if (trace.fraction == 1)

			// PMM - changed above to this
			if ((trace.fraction == 1) && (!trace.allsolid) && (!trace.startsolid))
			{
				VectorCopy (trace.endpos, ent->s.origin);
//=====
//PGM
				if (!current_bad && CheckForBadArea(ent))
				{
				//	gi.dprintf("Oooh! Bad Area!\n");
					VectorCopy (oldorg, ent->s.origin);
				}
				else
				{
					if (relink)
					{
						gi.linkentity (ent);
						G_TouchTriggers (ent);
					}
					// FIXME - remove this when ready!!!
				//	if (ent->lastMoveTime == level.time)
				//		if ((g_showlogic) && (g_showlogic->value))
				//			gi.dprintf ("Duplicate move detected for %s, please tell programmers!\n", ent->classname);
				//	ent->lastMoveTime = level.time;
					// FIXME

					return true;
				}
//PGM
//=====
			}

			if (!ent->enemy)
				break;
		}

		return false;
	}

	// push down from a step height above the wished position
	if (!(ent->monsterinfo.aiflags & AI_NOSTEP))
		stepsize = STEPSIZE;
	else
		stepsize = 1;

//PGM
#ifdef ROGUE_GRAVITY
	// trace from 1 stepsize gravityUp to 2 stepsize gravityDown.
	VectorMA(neworg, -1 * stepsize, ent->gravityVector, neworg);
	VectorMA(neworg, 2 * stepsize, ent->gravityVector, end);
#else
	neworg[2] += stepsize;
	VectorCopy (neworg, end);
	end[2] -= stepsize*2;
#endif
//PGM

	trace = gi.trace (neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

	// Lazarus- Determine whether monster is capable of and/or should jump
	jump = 0;
	if ((ent->monsterinfo.jump) && !(ent->monsterinfo.aiflags & AI_DUCKED))
	{
		// Don't jump if path is blocked by monster or player. Otherwise,
		// monster might attempt to jump OVER the monster/player, which 
		// ends up looking a bit goofy. Also don't jump if the monster's
		// movement isn't deliberate (target=NULL)
		if (trace.ent && (trace.ent->client || (trace.ent->svflags & SVF_MONSTER)))
			canjump = false;
		else if (target)
		{
			// Never jump unless it places monster closer to his goal
			vec3_t	dir;
			VectorSubtract(target->s.origin, oldorg, dir);
			d1 = VectorLength(dir);
			VectorSubtract(target->s.origin, trace.endpos, dir);
			d2 = VectorLength(dir);
			if (d2 < d1)
				canjump = true;
			else
				canjump = false;
		}
		else
			canjump = false;
	}
	else
		canjump = false;

	if (trace.allsolid)
	//	return false;
	{
		if (canjump && (ent->monsterinfo.jumpup > 0))
		{
			neworg[2] += ent->monsterinfo.jumpup - stepsize;
			trace = gi.trace (neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);
			if (!trace.allsolid && !trace.startsolid && trace.fraction > 0 && (trace.plane.normal[2] > 0.9))
			{
				if (!trace.ent || (!trace.ent->client && !(trace.ent->svflags & SVF_MONSTER) && !(trace.ent->svflags & SVF_DEADMONSTER)))
				{
					// Good plane to jump on. Make sure monster is more or less facing
					// the obstacle to avoid cutting-corners jumps
					trace_t	tr;
					vec3_t	p2;

					VectorMA(ent->s.origin,1024,forward,p2);
					tr = gi.trace(ent->s.origin,ent->mins,ent->maxs,p2,ent,MASK_MONSTERSOLID);
					if (DotProduct(tr.plane.normal,forward) < -0.95)
					{
						jump = 1;
						jumpheight = trace.endpos[2] - ent->s.origin[2];
					}
					else
						return false;
				}
			}
			else
				return false;
		}
		else
			return false;
	}
	// end Lazarus

	if (trace.startsolid)
	{
		neworg[2] -= stepsize;
		trace = gi.trace (neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);
		if (trace.allsolid || trace.startsolid)
			return false;
	}


	// don't go in to water
	// Lazarus: misc_actors don't go swimming, but wading is fine
	if (ent->monsterinfo.aiflags & AI_ACTOR)
	{
		// First check for lava/slime under feet - but only if we're not already in
		// a liquid
		test[0] = trace.endpos[0];
		test[1] = trace.endpos[1];
		if (ent->waterlevel == 0)
		{
			test[2] = trace.endpos[2] + ent->mins[2] + 1;	
			contents = gi.pointcontents(test);
			if (contents & (CONTENTS_LAVA | CONTENTS_SLIME))
				return false;
		}
		test[2] = trace.endpos[2] + ent->viewheight - 1;
		contents = gi.pointcontents(test);
		if (contents & MASK_WATER)
			return false;
	}
	else if (ent->waterlevel == 0)
	{
#ifdef ROGUE_GRAVITY //PGM
		test[0] = trace.endpos[0];
		test[1] = trace.endpos[1];
		if (ent->gravityVector[2] > 0)
			test[2] = trace.endpos[2] + ent->maxs[2] - 1;
		else
			test[2] = trace.endpos[2] + ent->mins[2] + 1;
#else
		test[0] = trace.endpos[0];
		test[1] = trace.endpos[1];
		test[2] = trace.endpos[2] + ent->mins[2] + 1;
#endif //PGM
		contents = gi.pointcontents(test);

	//	if (contents & MASK_WATER)
		if ( (contents & MASK_WATER) && (ent->movetype != MOVETYPE_FALLFLOAT) )	// Zaero
			return false;
	}

	// Lazarus: Don't intentionally move closer to a grenade,
	//          but don't perform this check if we're already evading some
	//          other problem (maybe even this grenade)
	if (!(ent->monsterinfo.aiflags & AI_CHASE_THING))
	{
		grenade = NULL;
		while ( (grenade = findradius(grenade,neworg,128)) != NULL)
		{
			if (!grenade->inuse)
				continue;
			if (!grenade->classname)
				continue;
			if (!Q_stricmp(grenade->classname,"grenade") || !Q_stricmp(grenade->classname,"hgrenade"))
			{
				VectorSubtract(grenade->s.origin,oldorg,dir);
				g1 = VectorLength(dir);
				VectorSubtract(grenade->s.origin,neworg,dir);
				g2 = VectorLength(dir);
				if (g2 < g1)
					return false;
			}
		}
	}

	// Lazarus: Don't intentionally walk into lasers.
	dist = VectorLength(move);
	if (dist > 0.)
	{
		edict_t		*e;
		trace_t		laser_trace;
		vec_t		delta;
		vec3_t		laser_mins, laser_maxs;
		vec3_t		laser_start, laser_end;
		vec3_t		monster_mins, monster_maxs;

		for (i = game.maxclients+1; i < globals.num_edicts; i++)
		{
			e = &g_edicts[i];
			if (!e->inuse)
				continue;
			if (!e->classname)
				continue;
			if (Q_stricmp(e->classname,"target_laser"))
				continue;
			if (e->svflags & SVF_NOCLIENT)
				continue;
			if ( (e->style == 2) || (e->style == 3))
				continue;
			if (!gi.inPVS(ent->s.origin,e->s.origin))
				continue;
			// Check to see if monster is ALREADY in the path of this laser.
			// If so, allow the move so he can get out.
			VectorMA(e->s.origin,2048,e->movedir,laser_end);
			laser_trace = gi.trace(e->s.origin,NULL,NULL,laser_end,NULL,CONTENTS_SOLID|CONTENTS_MONSTER);
			if (laser_trace.ent == ent)
				continue;
			VectorCopy(laser_trace.endpos,laser_end);
			laser_mins[0] = min(e->s.origin[0],laser_end[0]);
			laser_mins[1] = min(e->s.origin[1],laser_end[1]);
			laser_mins[2] = min(e->s.origin[2],laser_end[2]);
			laser_maxs[0] = max(e->s.origin[0],laser_end[0]);
			laser_maxs[1] = max(e->s.origin[1],laser_end[1]);
			laser_maxs[2] = max(e->s.origin[2],laser_end[2]);
			monster_mins[0] = min(oldorg[0],trace.endpos[0]) + ent->mins[0];
			monster_mins[1] = min(oldorg[1],trace.endpos[1]) + ent->mins[1];
			monster_mins[2] = min(oldorg[2],trace.endpos[2]) + ent->mins[2];
			monster_maxs[0] = max(oldorg[0],trace.endpos[0]) + ent->maxs[0];
			monster_maxs[1] = max(oldorg[1],trace.endpos[1]) + ent->maxs[1];
			monster_maxs[2] = max(oldorg[2],trace.endpos[2]) + ent->maxs[2];
			if ( monster_maxs[0] < laser_mins[0] ) continue;
			if ( monster_maxs[1] < laser_mins[1] ) continue;
			if ( monster_maxs[2] < laser_mins[2] ) continue;
			if ( monster_mins[0] > laser_maxs[0] ) continue;
			if ( monster_mins[1] > laser_maxs[1] ) continue;
			if ( monster_mins[2] > laser_maxs[2] ) continue;
			// If we arrive here, some part of the bounding box surrounding
			// monster's total movement intersects laser bounding box.
			// If laser is parallel to x, y, or z, we definitely
			// know this move will put monster in path of laser
			if ( (e->movedir[0] == 1.) || (e->movedir[1] == 1.) || (e->movedir[2] == 1.))
				return false;
			// Shift psuedo laser towards monster's current position up to
			// the total distance he's proposing moving.
			delta = min(16,dist);
			VectorNormalize2(move,dir);
			while (delta < dist+15.875)
			{
				if (delta > dist) delta = dist;
				VectorMA(e->s.origin,    -delta,dir,laser_start);
				VectorMA(e->s.old_origin,-delta,dir,laser_end);
				laser_trace = gi.trace(laser_start,NULL,NULL,laser_end,world,CONTENTS_SOLID|CONTENTS_MONSTER);
				if (laser_trace.ent == ent)
					return false;
				delta += 16;
			}
		}
	}

	// Lazarus
	if ((trace.fraction == 1) && !jump && canjump && (ent->monsterinfo.jumpdn > 0))
	{
		end[2] = oldorg[2] + move[2] - ent->monsterinfo.jumpdn;
		trace = gi.trace (neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID | MASK_WATER);
		if (trace.fraction < 1 && (trace.plane.normal[2] > 0.9) && (trace.contents & MASK_SOLID) && (neworg[2] - 16 > trace.endpos[2]))
		{
			if (!trace.ent || (!trace.ent->client && !(trace.ent->svflags & SVF_MONSTER) && !(trace.ent->svflags & SVF_DEADMONSTER)))
				jump = -1;
		}
	}

	// if (trace.fraction == 1)
	if ( (trace.fraction == 1) && !jump )
	//  end Lazarus
	{
	// if monster had the ground pulled out, go ahead and fall
		if ( ent->flags & FL_PARTIALGROUND )
		{
			VectorAdd (ent->s.origin, move, ent->s.origin);
			if (relink)
			{
				gi.linkentity (ent);
				G_TouchTriggers (ent);
			}
			ent->groundentity = NULL;
			// FIXME - remove this when ready!!!
		//	if (ent->lastMoveTime == level.time)
		//		if ((g_showlogic) && (g_showlogic->value))
		//			gi.dprintf ("Duplicate move detected for %s, please tell programmers!\n", ent->classname);
		//	ent->lastMoveTime = level.time;
			// FIXME
			return true;
		}
		// Zaero
		else if (ent->movetype == MOVETYPE_FALLFLOAT)
		{
			// can fall over the edge
			VectorAdd (ent->s.origin, move, ent->s.origin);
			if (relink)
			{
				gi.linkentity (ent);
				G_TouchTriggers (ent);
			}
			ent->groundentity = NULL;
			return true;
		}
		// end Zaero

		return false;		// walked off an edge
	}

// check point traces down for dangling corners
	VectorCopy (trace.endpos, ent->s.origin);

//PGM
	// PMM - don't bother with bad areas if we're dead
	if (ent->health > 0)
	{
		// use AI_BLOCKED to tell the calling layer that we're now mad at a tesla
		new_bad = CheckForBadArea(ent);
		if (!current_bad && new_bad)
		{
			if (new_bad->owner)
			{
			//	if ((g_showlogic) && (g_showlogic->value))
			//		gi.dprintf("Blocked -");
			//	if (!strcmp(new_bad->owner->classname, "tesla"))
				if ( !strcmp(new_bad->owner->classname, "tesla") || !strcmp(new_bad->owner->classname, "prox") )
				{
				//	if ((g_showlogic) && (g_showlogic->value))
				//		gi.dprintf ("it's a tesla/prox -");
					if ((!(ent->enemy)) || (!(ent->enemy->inuse)))
					{
					//	if ((g_showlogic) && (g_showlogic->value))
					//		gi.dprintf ("I don't have a valid enemy, attacking tesla/prox!\n");
						TargetTesla (ent, new_bad->owner);
						ent->monsterinfo.aiflags |= AI_BLOCKED;
					}
				//	else if (!strcmp(ent->enemy->classname, "telsa"))
					else if ( !strcmp(ent->enemy->classname, "telsa") || !strcmp(ent->enemy->classname, "prox") )
					{
					//	if ((g_showlogic) && (g_showlogic->value))
					//		gi.dprintf ("but we're already mad at a tesla/prox -");
					}
					else if ((ent->enemy) && (ent->enemy->client))
					{
					//	if ((g_showlogic) && (g_showlogic->value))
					//		gi.dprintf ("we have a player enemy -");
						if (visible(ent, ent->enemy))
						{
						//	if ((g_showlogic) && (g_showlogic->value))
						//		gi.dprintf ("we can see him -");
						}
						else
						{
						//	if ((g_showlogic) && (g_showlogic->value))
						//		gi.dprintf ("can't see him, kill the tesla/prox! -");
							TargetTesla (ent, new_bad->owner);
							ent->monsterinfo.aiflags |= AI_BLOCKED;
						}
					}
					else
					{
					//	if ((g_showlogic) && (g_showlogic->value))
					//		gi.dprintf ("the enemy isn't a player, killing tesla/prox -");
						TargetTesla (ent, new_bad->owner);
						ent->monsterinfo.aiflags |= AI_BLOCKED;
					}
				}
			/*	else if ((g_showlogic) && (g_showlogic->value)) {
					gi.dprintf(" by non-tesla, non-prox bad area!");
				} */
			}
		//	if ((g_showlogic) && (g_showlogic->value))
		//		gi.dprintf ("\n");

			VectorCopy (oldorg, ent->s.origin);
			return false;
		}
	}
//PGM

	if (!M_CheckBottom (ent))
	{
		if ( ent->flags & FL_PARTIALGROUND )
		{	// entity had floor mostly pulled out from underneath it
			// and is trying to correct
			if (relink)
			{
				gi.linkentity (ent);
				G_TouchTriggers (ent);
			}
			// FIXME - remove this when ready!!!
		//	if (ent->lastMoveTime == level.time)
		//		if ((g_showlogic) && (g_showlogic->value))
		//			gi.dprintf ("Duplicate move detected for %s, please tell programmers!\n", ent->classname);
		//	ent->lastMoveTime = level.time;
			// FIXME
			return true;
		}
		// Zaero
		else if (ent->movetype == MOVETYPE_FALLFLOAT)
		{
			// can fall over the edge
			if (relink)
			{
				gi.linkentity (ent);
				G_TouchTriggers (ent);
			}
			return true;
		}
		// end Zaero
		VectorCopy (oldorg, ent->s.origin);
		return false;
	}

	if ( ent->flags & FL_PARTIALGROUND )
	{
		ent->flags &= ~FL_PARTIALGROUND;
	}
	ent->groundentity = trace.ent;
	ent->groundentity_linkcount = trace.ent->linkcount;

// the move is ok
	if (relink)
	{
		gi.linkentity (ent);
		G_TouchTriggers (ent);
	}
	// FIXME - remove this when ready!!!
//	if (ent->lastMoveTime == level.time)
//		if ((g_showlogic) && (g_showlogic->value))
//			gi.dprintf ("Duplicate move detected for %s, please tell programmers!\n", ent->classname);
//	ent->lastMoveTime = level.time;
	// FIXME
	return true;
}


//============================================================================

/*
===============
M_ChangeYaw

===============
*/
void M_ChangeYaw (edict_t *ent)
{
	float	ideal;
	float	current;
	float	move;
	float	speed;

	current = anglemod(ent->s.angles[YAW]);
	ideal = ent->ideal_yaw;

	if (current == ideal)
		return;

	move = ideal - current;
	speed = ent->yaw_speed;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}

	ent->s.angles[YAW] = anglemod (current + move);
}


/*
======================
SV_StepDirection

Turns to the movement direction, and walks the current distance if
facing it.
======================
*/
qboolean SV_StepDirection (edict_t *ent, float yaw, float dist)
{
	vec3_t		move, oldorigin;
	float		delta;

	if (!ent->inuse)	return true;		// PGM g_touchtrigger free problem

	ent->ideal_yaw = yaw;
	M_ChangeYaw (ent);

	yaw = yaw*M_PI*2 / 360;
	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

	VectorCopy (ent->s.origin, oldorigin);
	if ( SV_movestep (ent, move, false) )
	{
		ent->monsterinfo.aiflags &= ~AI_BLOCKED;
		if ( !ent->inuse )	return true;		// PGM g_touchtrigger free problem

		delta = ent->s.angles[YAW] - ent->ideal_yaw;
		if ( strncmp(ent->classname, "monster_widow", 13) )
		{
			if (delta > 45 && delta < 315)
			{		// not turned far enough, so don't take the step
				VectorCopy (oldorigin, ent->s.origin);
			}
		}
		gi.linkentity (ent);
		G_TouchTriggers (ent);
		return true;
	}
	gi.linkentity (ent);
	G_TouchTriggers (ent);
	return false;
}

/*
======================
SV_FixCheckBottom

======================
*/
void SV_FixCheckBottom (edict_t *ent)
{
	ent->flags |= FL_PARTIALGROUND;
}



/*
================
SV_NewChaseDir

================
*/
#define	DI_NODIR	-1
void SV_NewChaseDir (edict_t *actor, edict_t *enemy, float dist)
{
	float	deltax,deltay;
	float	d[3];
	float	tdir, olddir, turnaround;

	// FIXME: how did we get here with no enemy
	if (!enemy)
		return;

	olddir = anglemod( (int)(actor->ideal_yaw/45)*45 );
	turnaround = anglemod(olddir - 180);

	deltax = enemy->s.origin[0] - actor->s.origin[0];
	deltay = enemy->s.origin[1] - actor->s.origin[1];
	if (deltax>10)
		d[1] = 0;
	else if (deltax<-10)
		d[1] = 180;
	else
		d[1] = DI_NODIR;
	if (deltay<-10)
		d[2] = 270;
	else if (deltay>10)
		d[2] = 90;
	else
		d[2] = DI_NODIR;

// try direct route
	if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	{
		if (d[1] == 0)
			tdir = d[2] == 90 ? 45 : 315;
		else
			tdir = d[2] == 90 ? 135 : 215;

		if (tdir != turnaround && SV_StepDirection(actor, tdir, dist))
			return;
	}

	// try other directions
	if ( ((rand()&3) & 1) || (abs(deltay) > abs(deltax)) )
	{
		tdir = d[1];
		d[1] = d[2];
		d[2] = tdir;
	}

	if (d[1] != DI_NODIR && d[1] != turnaround
	&& SV_StepDirection(actor, d[1], dist))
			return;

	if (d[2] != DI_NODIR && d[2] != turnaround
	&& SV_StepDirection(actor, d[2], dist))
			return;

//ROGUE
	if (actor->monsterinfo.blocked)
	{
		if ((actor->inuse) && (actor->health > 0))
		{
			if ((actor->monsterinfo.blocked)(actor, dist))
				return;
		}
	}
//ROGUE

/* there is no direct path to the player, so pick another direction */

	if (olddir != DI_NODIR && SV_StepDirection(actor, olddir, dist))
			return;

	if (rand()&1) 	/*randomly determine direction of search*/
	{
		for (tdir = 0; tdir <= 315; tdir += 45)
			if (tdir!=turnaround && SV_StepDirection(actor, tdir, dist) )
					return;
	}
	else
	{
		for (tdir = 315; tdir >= 0; tdir -= 45)
			if (tdir != turnaround && SV_StepDirection(actor, tdir, dist) )
					return;
	}

	if (turnaround != DI_NODIR && SV_StepDirection(actor, turnaround, dist) )
			return;

	actor->ideal_yaw = olddir;		// can't move

// if a bridge was pulled out from underneath a monster, it may not have
// a valid standing position at all

	if (!M_CheckBottom (actor))
		SV_FixCheckBottom (actor);
}

/*
======================
SV_CloseEnough

======================
*/
qboolean SV_CloseEnough (edict_t *ent, edict_t *goal, float dist)
{
	int		i;

	for (i=0 ; i<3 ; i++)
	{
		if (goal->absmin[i] > ent->absmax[i] + dist)
			return false;
		if (goal->absmax[i] < ent->absmin[i] - dist)
			return false;
	}
	return true;
}

/*
======================
M_MoveToGoal
======================
*/
void M_MoveToGoal (edict_t *ent, float dist)
{
	edict_t		*goal;

	goal = ent->goalentity;

	if (!ent->groundentity && !(ent->flags & (FL_FLY|FL_SWIM)))
		return;

	if ( (ent->monsterinfo.aiflags & AI_FOLLOW_LEADER) &&
		(ent->movetarget) &&
		(ent->movetarget->inuse) &&
		(ent->movetarget->health > 0) )
	{
		if (ent->enemy)
			ent->monsterinfo.currentmove = &actor_move_run;
		else
		{
			float	R;

			R = realrange(ent,ent->movetarget);
			if (R > ACTOR_FOLLOW_RUN_RANGE)
				ent->monsterinfo.currentmove = &actor_move_run;
			else if (R < ACTOR_FOLLOW_STAND_RANGE && ent->movetarget->client)
			{
				ent->monsterinfo.pausetime = level.time + 0.5;
				ent->monsterinfo.currentmove = &actor_move_stand;
				return;
			}
			else
				ent->monsterinfo.currentmove = &actor_move_walk;
		}
	}

// if the next step hits the enemy, return immediately
	if (ent->enemy &&  SV_CloseEnough (ent, ent->enemy, dist) )
		return;

// bump around...
//	if ( (rand()&3)==1 || !SV_StepDirection (ent, ent->ideal_yaw, dist))
// PMM - charging monsters (AI_CHARGING) don't deflect unless they have to
	if ( (((rand()&3)==1) && !(ent->monsterinfo.aiflags & AI_CHARGING)) || !SV_StepDirection (ent, ent->ideal_yaw, dist))
	{
		if (ent->monsterinfo.aiflags & AI_BLOCKED)
		{
		//	if ((g_showlogic) && (g_showlogic->value))
		//		gi.dprintf ("tesla attack detected, not changing direction!\n");
			ent->monsterinfo.aiflags &= ~AI_BLOCKED;
			return;
		}
		if (ent->inuse)
			SV_NewChaseDir (ent, goal, dist);
	}
}


/*
===============
M_walkmove
===============
*/
qboolean M_walkmove (edict_t *ent, float yaw, float dist)
{
	vec3_t	move;
	// PMM
	qboolean	retval;

	if (!ent->groundentity && !(ent->flags & (FL_FLY|FL_SWIM)))
		return false;

	yaw = yaw*M_PI*2 / 360;

	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

	// PMM
	retval = SV_movestep(ent, move, true);
	ent->monsterinfo.aiflags &= ~AI_BLOCKED;
	return retval;
	// PMM
//	return SV_movestep(ent, move, true);
}


// Zaero add
qboolean ai_checkattack (edict_t *self, float dist);
/*
====================
M_MoveAwayFromFlare
====================
*/
qboolean M_MoveAwayFromFlare (edict_t *self, float dist)
{
	edict_t *e = NULL;
	edict_t *goal = NULL;
	vec3_t delta;
	vec3_t forward;

	// find the closest flare
	while (1)
	{
		e = findradius(e, self->s.origin, 256);
		if (e == NULL)
			break;

		if (Q_stricmp(e->classname, "flare") == 0)
			break;
	}
	
	goal = G_Spawn();
	self->goalentity = goal;
	if (e == NULL)
	{
		// just move forward
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorMA(self->s.origin, 128, forward, goal->s.origin);
	}
	else /* e != NULL */
	{
		VectorSubtract(self->s.origin, e->s.origin, delta);
		VectorNormalize(delta);
		VectorMA(self->s.origin, 128, delta, goal->s.origin);
	}

	if ( (rand() & 7) == 1 )
	{
		// set the ideal_yaw
		VectorSubtract(goal->s.origin, self->s.origin, delta);
		self->ideal_yaw = vectoyaw(delta);
	}

	if ( (rand()&3)==1 || !SV_StepDirection (self, self->ideal_yaw, dist))
	{
		SV_NewChaseDir (self, goal, dist);
	}

	self->goalentity = NULL;
	G_FreeEdict(goal);

	return true;
}
// end Zaero
