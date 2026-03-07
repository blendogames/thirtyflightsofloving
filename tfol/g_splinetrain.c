/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2000-2002 Mr. Hyde and Mad Dog

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

#include "g_local.h"

#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3

void Move_Calc (edict_t *ent, vec3_t dest, void(*func)(edict_t*));
void train_blocked (edict_t *self, edict_t *other);
void train_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

void train_spline_calc (edict_t *train, vec3_t p1, vec3_t p2, vec3_t a1, vec3_t a2, float m)
{
	/* p1, p2  =  origins of path_* ents
	   a1, a2  =  angles from path_* ents
	   m       =  decimal position along curve */

	vec3_t v1, v2; // direction vectors
	vec3_t c1, c2; // control-point coords
	vec3_t d, v;   // temps
	float  s;      // vector scale
	vec3_t p, a;   // final computed position & angles for mover
	// these greatly simplify/speed up equations
	// (make sure m is already assigned a value)
	float n     = 1.0 - m;
	float m2    = m * m;
	float m3    = m2 * m;
	float n2    = n * n;
	float n3    = n2 * n;
	float mn2_3 = m * n2 * 3;
	float m2n_3 = m2 * n * 3;
	float mn_2  = m * n * 2;

	// Beziers need two control-points to define the shape of the curve.
	// These can be created from the available data.  They are offset a
	// specific distance from the endpoints (path_*s), in the direction
	// of the endpoints' angle vectors (the 2nd control-point is offset in
	// the opposite direction).  The distance used is a fraction of the total
	// distance between the endpoints, ensuring it's scaled proportionally.
	// The factor of 0.4 is simply based on experimentation, as a value that
	// yields nice even curves.

	AngleVectors(a1, v1, NULL, NULL);
	AngleVectors(a2, v2, NULL, NULL);

	VectorSubtract(p2, p1, d);
	s = sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]) * 0.4;

	VectorMA(p1,  s, v1, c1);
	VectorMA(p2, -s, v2, c2);

	// cubic interpolation of the four points
	// gives the position along the curve
	p[0] = n3 * p1[0] + mn2_3 * c1[0] + m2n_3 * c2[0] + m3 * p2[0];
	p[1] = n3 * p1[1] + mn2_3 * c1[1] + m2n_3 * c2[1] + m3 * p2[1];
	p[2] = n3 * p1[2] + mn2_3 * c1[2] + m2n_3 * c2[2] + m3 * p2[2];

	// should be optional:
	// first derivative of bezier formula provides direction vector
	// along the curve (equation simplified in terms of m & n)
	v[0] = (n2 * p1[0] - (n2 - mn_2) * c1[0] - (mn_2 - m2) * c2[0] - m2 * p2[0]) / -s;
	v[1] = (n2 * p1[1] - (n2 - mn_2) * c1[1] - (mn_2 - m2) * c2[1] - m2 * p2[1]) / -s;
	v[2] = (n2 * p1[2] - (n2 - mn_2) * c1[2] - (mn_2 - m2) * c2[2] - m2 * p2[2]) / -s;
	vectoangles(v, a);

	VectorSubtract(p,train->mins,train->s.origin);
	VectorCopy(a,train->s.angles);

	gi.linkentity(train);
}
void train_spline_next (edict_t *self);
void train_spline_wait (edict_t *self)
{
	if (self->target_ent->pathtarget)
	{
		char	*savetarget;
		edict_t	*ent;

		ent = self->target_ent;
		savetarget = ent->target;
		ent->target = ent->pathtarget;
		G_UseTargets (ent, self->activator);
		ent->target = savetarget;

		// make sure we didn't get killed by a killtarget
		if (!self->inuse)
			return;
	}

	// Lazarus: rotating trains
	if(self->target_ent) {
		if(self->target_ent->speed) {
			self->speed = self->target_ent->speed;
			self->moveinfo.speed = self->speed;
			self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;
		}
		if(self->spawnflags & TRAIN_ROTATE) {
			if(self->target_ent->pitch_speed)
				self->pitch_speed = self->target_ent->pitch_speed;
			if(self->target_ent->yaw_speed)
				self->yaw_speed   = self->target_ent->yaw_speed;
			if(self->target_ent->roll_speed)
				self->roll_speed  = self->target_ent->roll_speed;
		}
	}

	if (self->moveinfo.wait)
	{
		if (self->moveinfo.wait > 0)
		{
			// Lazarus: turn off animation for stationary trains
			self->s.effects &= ~(EF_ANIM_ALL | EF_ANIM_ALLFAST);
			self->nextthink = level.time + self->moveinfo.wait;
			self->think = train_spline_next;
		}
		else if (self->spawnflags & TRAIN_TOGGLE)  // && wait < 0
		{
			train_spline_next (self);
			self->spawnflags &= ~TRAIN_START_ON;
			VectorClear (self->velocity);
			// Lazarus: turn off animation for stationary trains
			self->s.effects &= ~(EF_ANIM_ALL | EF_ANIM_ALLFAST);
			self->nextthink = 0;
		}

		if (!(self->flags & FL_TEAMSLAVE))
		{
			if (self->s.sound && self->moveinfo.sound_end)
				gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_STATIC, 0);
			self->s.sound = 0;
		}
	}
	else
	{
		train_spline_next (self);
	}
	
}

void train_spline_move (edict_t *self)
{
	edict_t	*train;

	train = self->enemy;
	if(!train || !train->inuse)
		return;
	if(train->from != train->to)
	{
		train_spline_calc (train, train->from->s.origin, train->to->s.origin, 
		                          train->from->s.angles, train->to->s.angles,
								  train->moveinfo.ratio);
		train->moveinfo.ratio += train->moveinfo.speed * FRAMETIME / train->moveinfo.distance;
	}
	self->nextthink = level.time + FRAMETIME;
}

void train_spline_next (edict_t *self)
{
	edict_t		*ent;
	vec3_t		dest;
	qboolean	first;
	vec3_t		angles,v;

	first = true;
again:
	if (!self->target)
	{
		self->s.sound = 0;
		return;
	}

	ent = G_PickTarget (self->target);
	if (!ent)
	{
		gi.dprintf ("train_spline_next: bad target %s\n", self->target);
		return;
	}

	// spline stuff:
	self->from = self->to;
	self->to   = ent;
	VectorSubtract(self->from->s.origin,self->to->s.origin,v);
	self->moveinfo.distance = VectorLength(v);
	self->moveinfo.ratio = 0.0;
	// end spline stuff
	self->target = ent->target;

	// check for a teleport path_corner
	if (ent->spawnflags & 1)
	{
		if (!first)
		{
			gi.dprintf ("connected teleport path_corners, see %s at %s\n", ent->classname, vtos(ent->s.origin));
			return;
		}
		first = false;
		VectorSubtract (ent->s.origin, self->mins, self->s.origin);
		VectorCopy (self->s.origin, self->s.old_origin);
		self->s.event = EV_OTHER_TELEPORT;
		gi.linkentity (self);
		goto again;
	}

	self->moveinfo.wait = ent->wait;
	self->target_ent = ent;

	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_start)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0);
		self->s.sound = self->moveinfo.sound_middle;
	}

	VectorSubtract (ent->s.origin, self->mins, dest);
	self->moveinfo.state = STATE_TOP;
	VectorCopy (self->s.origin, self->moveinfo.start_origin);
	VectorCopy (dest, self->moveinfo.end_origin);

	// Rroff rotating
	if (self->spawnflags & TRAIN_ROTATE && !(ent->spawnflags & 2))
	{
		// Lazarus: No no no :-). This is measuring from the center
		//          of the func_train to the path_corner. Should
		//          be path_corner to path_corner.
		//VectorSubtract (ent->s.origin, self->s.origin, v);
		VectorAdd(self->s.origin,self->mins,v);
		VectorSubtract(ent->s.origin,v,v);
		vectoangles(v,angles);
		self->ideal_yaw = angles[YAW];
		self->ideal_pitch = angles[PITCH];
		if(self->ideal_pitch < 0) self->ideal_pitch += 360;
		self->ideal_roll = ent->roll;

		VectorClear(self->movedir);
		self->movedir[1] = 1.0;
	}
	Move_Calc (self, dest, train_spline_wait);
	self->spawnflags |= TRAIN_START_ON;
}

void train_spline_resume (edict_t *self)
{
	edict_t	*ent;
	vec3_t	dest;

	ent = self->target_ent;

	VectorSubtract (ent->s.origin, self->mins, dest);
	self->moveinfo.state = STATE_TOP;
	VectorCopy (self->s.origin, self->moveinfo.start_origin);
	VectorCopy (dest, self->moveinfo.end_origin);
	Move_Calc (self, dest, train_spline_wait);
	self->spawnflags |= TRAIN_START_ON;
}

void func_train_spline_find (edict_t *self)
{
	edict_t *ent;

	if (!self->target)
	{
		gi.dprintf ("train_spline_find: no target\n");
		return;
	}
	ent = G_PickTarget (self->target);
	if (!ent)
	{
		gi.dprintf ("train_spline_find: target %s not found\n", self->target);
		return;
	}

	// Lazarus: trains can change speed at path_corners
	if(ent->speed) {
		self->speed = ent->speed;
		self->moveinfo.speed = self->speed;
		self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;
	}
	if(ent->pitch_speed)
		self->pitch_speed = ent->pitch_speed;
	if(ent->yaw_speed)
		self->yaw_speed   = ent->yaw_speed;
	if(ent->roll_speed)
		self->roll_speed  = ent->roll_speed;

	// Lazarus: spline stuff
	self->from = self->to = ent;

	self->target = ent->target;

	ent->think = train_spline_move;
	ent->enemy = self;
	ent->nextthink = level.time + FRAMETIME;

	VectorSubtract (ent->s.origin, self->mins, self->s.origin);
	gi.linkentity (self);

	// if not triggered, start immediately
	if (!self->targetname)
		self->spawnflags |= TRAIN_START_ON;

	if (self->spawnflags & TRAIN_START_ON)
	{
		// Lazarus: animated trains
		if (self->spawnflags & TRAIN_ANIMATE)
			self->s.effects |= EF_ANIM_ALL;
		else if (self->spawnflags & TRAIN_ANIMATE_FAST)
			self->s.effects |= EF_ANIM_ALLFAST;

		self->nextthink = level.time + FRAMETIME;
		self->think = train_spline_next;
		self->activator = self;
	}
}

void train_spline_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;

	if (self->spawnflags & TRAIN_START_ON)
	{
		if (!(self->spawnflags & TRAIN_TOGGLE))
			return;
		self->spawnflags &= ~TRAIN_START_ON;
		VectorClear (self->velocity);
		VectorClear (self->avelocity);
		self->s.effects &= ~(EF_ANIM_ALL | EF_ANIM_ALLFAST);
		self->nextthink = 0;
	}
	else
	{
		if (self->spawnflags & TRAIN_ANIMATE)
			self->s.effects |= EF_ANIM_ALL;
		else if (self->spawnflags & TRAIN_ANIMATE_FAST)
			self->s.effects |= EF_ANIM_ALLFAST;

		if (self->target_ent)
			train_spline_resume(self);
		else
			train_spline_next(self);
	}
}

void SP_func_train_spline (edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;

	VectorClear (self->s.angles);
	self->blocked = train_blocked;
	if (self->spawnflags & TRAIN_BLOCK_STOPS)
		self->dmg = 0;
	else
	{
		if (!self->dmg)
			self->dmg = 100;
	}
	self->solid = SOLID_BSP;
	gi.setmodel (self, self->model);

	if (st.noise)
		self->moveinfo.sound_middle = gi.soundindex  (st.noise);

	if (!self->speed)
		self->speed = 100;

	// Lazarus: Do NOT set default values for rotational speeds - if they're 0, then they're 0.

	self->moveinfo.speed = self->speed;
	self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;
	self->use = train_spline_use;

	// Lazarus: damageable
	if (self->health) {
		self->die = train_die;
		self->takedamage = DAMAGE_YES;
	} else {
		self->die = NULL;
		self->takedamage = DAMAGE_NO;
	}

	gi.linkentity (self);
	if (self->target)
	{
		// start trains on the second frame, to make sure their targets have had
		// a chance to spawn
		self->nextthink = level.time + FRAMETIME;
		self->think = func_train_spline_find;
	}
	else
	{
		gi.dprintf ("func_train_spline without a target at %s\n", vtos(self->absmin));
	}

	// Lazarus: TRAIN_SMOOTH forces trains to go directly to Move_Done from
	//       Move_Final rather than slowing down (if necessary) for one
	//       frame.
	if (self->spawnflags & TRAIN_SMOOTH)
		self->smooth_movement = true;
	else
		self->smooth_movement = false;

	// Lazarus: make noise field work w/o origin brush
	// ver. 1.3 change - do this for ALL trains
//	if(st.noise && !VectorLength(self->s.origin) )
	if(st.noise)
	{
		edict_t *speaker;

		self->noise_index    = self->moveinfo.sound_middle;
		self->moveinfo.sound_middle = 0;
		speaker = G_Spawn();
		speaker->classname   = "moving_speaker";
		speaker->s.sound     = 0;
		speaker->volume      = 1;
		speaker->attenuation = 1;
		speaker->owner       = self;
		speaker->think       = Moving_Speaker_Think;
		speaker->nextthink   = level.time + 2*FRAMETIME;
		speaker->spawnflags  = 7;       // owner must be moving to play
		self->speaker        = speaker;
		if(VectorLength(self->s.origin))
			VectorCopy(self->s.origin,speaker->s.origin);
		else {
			VectorAdd(self->absmin,self->absmax,speaker->s.origin);
			VectorScale(speaker->s.origin,0.5,speaker->s.origin);
		}
		VectorSubtract(speaker->s.origin,self->s.origin,speaker->offset);
	}

}
