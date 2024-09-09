/*****************************************************************

	Eraser Bot source code - by Ryan Feltrin

	..............................................................

	This file is Copyright(c) 1998, Ryan Feltrin, All Rights Reserved.

	..............................................................

	All other files are Copyright(c) Id Software, Inc.

	Please see liscense.txt in the source directory for the copyright
	information regarding those files belonging to Id Software, Inc.

	..............................................................
	
	Should you decide to release a modified version of Eraser, you MUST
	include the following text (minus the BEGIN and END lines) in the 
	documentation for your modification.

	--- BEGIN ---

	The Eraser Bot is a product of Ryan Feltrin, and is available from
	the Eraser Bot homepage, at http://impact.frag.com.

	This program is a modification of the Eraser Bot, and is therefore
	in NO WAY supported by Ryan Feltrin.

	This program MUST NOT be sold in ANY form. If you have paid for 
	this product, you should contact Ryan Feltrin immediately, via
	the Eraser Bot homepage.

	--- END ---

	..............................................................

	You will find p_trail.c has not been included with the Eraser
	source code release. This is NOT an error. I am unable to 
	distribute this file because it contains code that is bound by
	legal documents, and signed by myself, never to be released
	to the public. Sorry guys, but law is law.

	I have therefore include the compiled version of these files
	in .obj form in the src\Release and src\Debug directories.
	So while you cannot edit and debug code within these files,
	you can still compile this source as-is. Although these will only
	work in MSVC v5.0, linux versions can be made available upon
	request.

	NOTE: When compiling this source, you will get a warning
	message from the compiler, regarding the missing p_trail.c
	file. Just ignore it, it will still compile fine.

	..............................................................

	I, Ryan Feltrin, hold no responsibility for any harm caused by the
	use of this source code. I also am NOT willing to provide any form
	of help or support for this source code. It is provided as-is,
	as a service by me, with no documentation, other then the comments
	contained within the code. If you have any queries, I suggest you
	visit the "official" Eraser source web-board, at
	http://www.telefragged.com/epidemic/. I will stop by there from
	time to time, to answer questions and help with any problems that
	may arise.

	Otherwise, have fun, and I look forward to seeing what can be done
	with this.

	-Ryan Feltrin

 *****************************************************************/

//
// bot_nav.c
//
// This file conatins mostly (simple) environment sampling, which
// in no way is related to the the navigation code, contained within
// p_trail.c
//

#include "g_local.h"
#include "m_player.h"
#include "bot_procs.h"
#include "p_trail.h"

/*
================
botRoamFindBestDirection

Finds the best direction to walk for the next few seconds
Set ideal_yaw accordingly
================
*/
#define		TRACE_DIST		256
void	botRoamFindBestDirection(edict_t	*self)
{
	float		best_dist = 0, this_dist = 0, best_yaw;
	int		i;
	vec3_t	dir, dest, angle, this_angle;
	vec3_t	mins;
	trace_t	trace;

	if (self->last_best_direction > (level.time - 1))
		return;
	self->last_best_direction = level.time;

	bestdirection_callsthisframe++;
	if (bestdirection_callsthisframe > 2)
		return;

	best_yaw = self->ideal_yaw;
	VectorAdd(self->mins, tv(0,0,STEPSIZE), mins);

	// check eight compass directions

	VectorClear(angle);
	VectorClear(this_angle);
	angle[1] = self->ideal_yaw;

	// start at center, then fan out in 45 degree intervals, swapping between + and -
	for (i=1; i<8; i++)
	{
//		if (i<7 && random() < 0.4)	// skip random intervals
//			i++;

		if (i==4)
			i=6;

		this_angle[1] = anglemod(angle[1] + ((((i % 2)*2 - 1) * (int) (floor(i/2))) * 45));
		AngleVectors(this_angle, dir, NULL, NULL);
		
		VectorMA(self->s.origin, TRACE_DIST, dir, dest);

		trace = gi.trace(self->s.origin, mins, self->maxs, dest, self, MASK_SOLID);

		if (trace.fraction > 0)
		{	// check that destination is onground, or not above lava/slime
			dest[0] = trace.endpos[0];
			dest[1] = trace.endpos[1];
			dest[2] = trace.endpos[2] - 32;

			if (gi.pointcontents(dest) & MASK_PLAYERSOLID)
				goto nocheckground;

			this_dist = trace.fraction * TRACE_DIST;

			VectorCopy(trace.endpos, dest);
			dest[2] -= 256;
			trace = gi.trace(trace.endpos, VEC_ORIGIN, VEC_ORIGIN, dest, self, MASK_SOLID | MASK_WATER);

			if ( ! ((trace.fraction == 1) || (trace.contents & MASK_WATER)))	// avoid ALL forms of liquid for now
			{
//gi.dprintf("Dist %i: %i\n", (int) this_angle[1], (int) this_dist);

				if (trace.fraction > 0.4)		// if there is a drop in this direction, try to avoid it if possible
					this_dist *= 0.5;

nocheckground:
				if (this_dist > best_dist)
				{
					best_dist = this_dist;
					best_yaw = this_angle[1];

					if (this_dist == TRACE_DIST)
						break;
				}
			}
		}

	}

//gi.dprintf("Best yaw: %i\n", (int) best_yaw);

	self->ideal_yaw = best_yaw;
}

void	botRandomJump(edict_t	*self)
{
	vec3_t	dir, right, angles;

	if (self->groundentity)
	{
		if (self->last_jump > (level.time - 0.5))
			return;
	}
	else
	{
//		if (self->last_jump > (level.time - 2))
			return;
	}

	if (!CanJump(self))
		return;

	botRoamFindBestDirection(self);

	VectorClear(angles);
	angles[1] = self->ideal_yaw;

	AngleVectors(angles, dir, right, NULL);
//	VectorScale(dir, (crandom() + 0.5) / 1.5, dir);
//	VectorScale(right, crandom() * 0.5, right);
//	VectorAdd(dir, right, dir);
	VectorNormalize2(dir, dir);

	VectorScale(dir, 300 * ((random() * 0.6) + 0.4), dir);
	VectorCopy(dir, self->velocity);

	self->velocity[2] = 300;
	self->groundentity = NULL;
//	self->s.origin[2] += 1;

	gi.linkentity(self);

	VectorCopy(self->velocity, self->jump_velocity);

	if (self->groundentity)
		gi.sound(self, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, 2, 0);

	self->last_jump = level.time;
}

extern edict_t	*pm_passent;
extern trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end);

qboolean	touched_player;

void BotMoveThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;

	level.current_entity = ent;
	client = ent->client;

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

	pm_passent = ent;

	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->s.modelindex != (MAX_MODELS-1)) //was 255
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag)
		client->ps.pmove.pm_type = PM_DEAD;
	else
		client->ps.pmove.pm_type = PM_NORMAL;

	client->ps.pmove.gravity = sv_gravity->value;
	if (ent->maxs[2] == 4)
	{
		client->ps.pmove.pm_flags |= PMF_DUCKED;
		ucmd->upmove = -400;
	}
	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}
/*
//	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
//	{
		pm.snapinitial = true;
//		gi.dprintf ("pmove changed!\n");
//	}
*/
//	ucmd->buttons = 128;

	pm.cmd = *ucmd;

	pm.trace = PM_trace;	// adds default parms
	pm.pointcontents = gi.pointcontents;

	// perform a pmove
	gi.Pmove (&pm);

	// save results of pmove
	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;

	for (i=0 ; i<3 ; i++)
	{
		ent->s.origin[i] = pm.s.origin[i]*0.125;
		ent->velocity[i] = pm.s.velocity[i]*0.125;
	}

	VectorCopy (pm.mins, ent->mins);
	VectorCopy (pm.maxs, ent->maxs);

	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	ent->viewheight = pm.viewheight;
	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;
	if (pm.groundentity)
		ent->groundentity_linkcount = pm.groundentity->linkcount;

	if (ent->deadflag)
	{
		client->ps.viewangles[ROLL] = 40;
		client->ps.viewangles[PITCH] = -15;
		client->ps.viewangles[YAW] = client->killer_yaw;
	}
	else
	{
		VectorCopy (pm.viewangles, client->v_angle);
		VectorCopy (pm.viewangles, client->ps.viewangles);
	}


	gi.linkentity (ent);

	if (ent->movetype != MOVETYPE_NOCLIP)
		G_TouchTriggers (ent);

	touched_player = false;

	// touch other objects
	for (i=0 ; i<pm.numtouch ; i++)
	{
		other = pm.touchents[i];
		for (j=0 ; j<i ; j++)
			if (pm.touchents[j] == other)
				break;
		if (j != i)
			continue;	// duplicated

		if (other->client)
			touched_player = true;

		if (!other->touch)
			continue;
		other->touch (other, ent, NULL, NULL);
	}

}

int botJumpAvoidEnt(edict_t *self, edict_t *e_avoid)
{
	vec3_t	dir, trail_vec, vec, tr_end;
	float	avoid_dist;
	trace_t	tr;

	if (!CanJump(self))
		return false;

	if (e_avoid->owner && (((int) dmflags->value) & DF_NO_FRIENDLY_FIRE) && SameTeam(self, e_avoid->owner))
		return 2; 

	if ((avoid_dist = entdist(self, e_avoid)) > 300)
	{
		self->avoid_ent = NULL;
		return 2;		// just keep going for our current goal
	}

	if (self->movetarget && (entdist(self, self->movetarget) < 256))
	{
		return 2;
	}

	if (!visible(self, e_avoid))
		return 2;

	// make sure we attack this person, if not currently attacking anything
	if (!self->enemy && e_avoid->owner && e_avoid->owner->client)
		self->enemy = e_avoid->owner;

	VectorSubtract(self->s.origin, e_avoid->s.origin, dir);
	VectorNormalize2(dir, dir);

	trail_vec[0] = dir[1];
	trail_vec[1] = dir[0];
	trail_vec[2] = 0;

	// determine which side we're on, so we know which way to try and dodge
	VectorMA(e_avoid->s.origin, 4, trail_vec, vec);
	VectorSubtract(self->s.origin, vec, vec);

	if (avoid_dist < 200)
	{
		if (VectorLength(vec) > avoid_dist)
		{
			VectorScale(trail_vec, -1, trail_vec);
		}
	}
	else if (random() < 0.5)	// pick a random direction
	{
		VectorScale(trail_vec, -1, trail_vec);
	}

	// add some forwards/backwards movement
	VectorMA(trail_vec, crandom(), dir, trail_vec);
	VectorNormalize2(trail_vec, trail_vec);

	if (self->groundentity)
	{
		VectorMA(e_avoid->s.origin, 200, trail_vec, vec);
		tr = gi.trace(self->s.origin, vec3_origin, vec3_origin, vec, self, MASK_PLAYERSOLID);

		VectorCopy(tr.endpos, tr_end);
		tr_end[2] -= 512;
		tr = gi.trace(tr.endpos, vec3_origin, vec3_origin, tr_end, self, MASK_PLAYERSOLID | MASK_WATER);

		if (!(tr.contents & (CONTENTS_LAVA | CONTENTS_SLIME)))/* && ((e_avoid->s.effects & EF_GRENADE) || (avoid_dist < 160)))*/
		{ // go ahead and jump!
			VectorScale(trail_vec, BOT_RUN_SPEED, dir);
			dir[2] = 300;

			VectorCopy(dir, self->velocity);
			VectorCopy(dir, self->jump_velocity);

			self->groundentity = NULL;
//			self->s.origin[2] += 1;
			gi.linkentity(self);

			gi.sound(self, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, 2, 0);
		}
		else	// strafe away
		{
			VectorCopy(trail_vec, self->avoid_dir);
			self->avoid_dir_time = level.time + 0.3;
		}
	}
	else if (self->waterlevel)
	{
		VectorCopy(trail_vec, self->avoid_dir);
		self->avoid_dir_time = level.time + 1;
	}

	return 1;
}
