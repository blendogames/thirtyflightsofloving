#include "g_local.h"
#include "bot.h"

void SpawnItem3 (edict_t *ent, gitem_t *item);

// Knightmare added
void train_children_think (edict_t *self);
void spline_calc (edict_t *train, vec3_t p1, vec3_t p2, vec3_t a1, vec3_t a2, float m, vec3_t p, vec3_t a)
{
	/* p1, p2  =  origins of path_* ents
	   a1, a2  =  angles from path_* ents
	   m       =  decimal position along curve */

	vec3_t v1, v2; // direction vectors
	vec3_t c1, c2; // control-point coords
	vec3_t d, v;   // temps
	float  s;      // vector scale
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
	vectoangles2(v, a);
	if (train->roll_speed > 0)	// Knightmare added
		a[ROLL] = a1[ROLL] + m*(a2[ROLL] - a1[ROLL]);
}

void train_wait (edict_t *self);
void train_spline (edict_t *self)
{
	edict_t	*train;
	vec3_t	p;
	vec3_t	a;

	train = self->enemy;
	if (!train || !train->inuse)
		return;
	//Knightmare- check for killtargeted path_corners
	if ((!train->from) || (!train->from->inuse) || (!train->to) || (!train->to->inuse))
		return;
	//Knightmare- check for removed spline flag- get da hell outta here
	if (!train->spawnflags & TRAIN_SPLINE)
	{
		self->think = train_children_think;
		return;
	}

	if ( (train->from != train->to) && !train->moveinfo.is_blocked && (train->spawnflags & TRAIN_START_ON))
	{
		if (train->moveinfo.ratio >= 1.0) //Knightmare- don't keep moving at end of curve
		{
			VectorClear(self->avelocity);
			VectorClear(self->velocity);
			self->nextthink = level.time + FRAMETIME;
			return;
		}

		spline_calc (train, train->from->s.origin, train->to->s.origin, 
		                    train->from->s.angles, train->to->s.angles,
		 				    train->moveinfo.ratio, p, a);
		if ( !(train->spawnflags & TRAIN_ORIGIN) ) // Knightmare- func_train_origin support
			VectorSubtract(p, train->mins, p);
		VectorSubtract(p, train->s.origin, train->velocity);
		VectorScale(train->velocity, 10 ,train->velocity);
		VectorSubtract(a, train->s.angles, train->avelocity);
		VectorScale(train->avelocity, 10, train->avelocity);
		if (train->pitch_speed < 0)
			train->avelocity[PITCH] = 0;
		if (train->yaw_speed < 0)
			train->avelocity[YAW] = 0;
		gi.linkentity(train);
		train->moveinfo.ratio += train->moveinfo.speed * FRAMETIME / train->moveinfo.distance;
	//	if (train->movewith_next && (train->movewith_next->movewith_ent == train))
	//		set_child_movement(train);
		if (train->moveinfo.ratio >= 1.0)
		{
			train->moveinfo.endfunc = NULL;
			train->think = train_wait;
			train->nextthink = level.time + FRAMETIME;
		}
	}
	self->nextthink = level.time + FRAMETIME;
}
// end Knightmare

/*
=========================================================

  PLATS

  movement options:

  linear
  smooth start, hard stop
  smooth start, smooth stop

  start
  end
  acceleration
  speed
  deceleration
  begin sound
  end sound
  target fired when reaching end
  wait at end

  object characteristics that use move segments
  ---------------------------------------------
  movetype_push, or movetype_stop
  action when touched
  action when blocked
  action when used
	disabled?
  auto trigger spawning


=========================================================
*/

#define PLAT_LOW_TRIGGER	1

//====
//PGM
#define PLAT2_TOGGLE			2
#define PLAT2_TOP				4
#define PLAT2_TRIGGER_TOP		8
#define PLAT2_TRIGGER_BOTTOM	16
#define PLAT2_BOX_LIFT			32

#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3
#define STATE_LOWEST		4

#define DOOR_START_OPEN		1
#define DOOR_REVERSE		2
#define DOOR_CRUSHER		4
#define DOOR_NOMONSTER		8
#define DOOR_TOGGLE			32
#define DOOR_X_AXIS			64
#define DOOR_Y_AXIS			128


//
// Support routines for movement (changes in origin using velocity)
//

void Move_Done (edict_t *ent)
{
	VectorClear (ent->velocity);
	ent->moveinfo.endfunc (ent);
}

void Move_Final (edict_t *ent)
{
	if (ent->moveinfo.remaining_distance == 0)
	{
		Move_Done (ent);
		return;
	}

	VectorScale (ent->moveinfo.dir, ent->moveinfo.remaining_distance / FRAMETIME, ent->velocity);

	ent->think = Move_Done;
	ent->nextthink = level.time + FRAMETIME;
}

void Move_Begin (edict_t *ent)
{
	float	frames;

	if ((ent->moveinfo.speed * FRAMETIME) >= ent->moveinfo.remaining_distance)
	{
		Move_Final (ent);
		return;
	}

	VectorScale (ent->moveinfo.dir, ent->moveinfo.speed, ent->velocity);
	frames = floor((ent->moveinfo.remaining_distance / ent->moveinfo.speed) / FRAMETIME);
	ent->moveinfo.remaining_distance -= frames * ent->moveinfo.speed * FRAMETIME;
	ent->nextthink = level.time + (frames * FRAMETIME);
	ent->think = Move_Final;
}

void Think_AccelMove (edict_t *ent);

void Move_Calc (edict_t *ent, vec3_t dest, void(*func)(edict_t*))
{
	VectorClear (ent->velocity);
	VectorSubtract (dest, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize (ent->moveinfo.dir);
	ent->moveinfo.endfunc = func;

	if (ent->moveinfo.speed == ent->moveinfo.accel && ent->moveinfo.speed == ent->moveinfo.decel)
	{
		if (level.current_entity == ((ent->flags & FL_TEAMSLAVE) ? ent->teammaster : ent))
		{
			Move_Begin (ent);
		}
		else
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = Move_Begin;
		}
	}
	else
	{
		// accelerative
		ent->moveinfo.current_speed = 0;
		ent->think = Think_AccelMove;
		ent->nextthink = level.time + FRAMETIME;
	}
}


//
// Support routines for angular movement (changes in angle using avelocity)
//

void AngleMove_Done (edict_t *ent)
{
	VectorClear (ent->avelocity);
	ent->moveinfo.endfunc (ent);
}

void AngleMove_Final (edict_t *ent)
{
	vec3_t	move;

	if (ent->moveinfo.state == STATE_UP)
		VectorSubtract (ent->moveinfo.end_angles, ent->s.angles, move);
	else
		VectorSubtract (ent->moveinfo.start_angles, ent->s.angles, move);

	if (VectorCompare (move, vec3_origin))
	{
		AngleMove_Done (ent);
		return;
	}

	VectorScale (move, 1.0/FRAMETIME, ent->avelocity);

	ent->think = AngleMove_Done;
	ent->nextthink = level.time + FRAMETIME;
}

void AngleMove_Begin (edict_t *ent)
{
	vec3_t	destdelta;
	float	len;
	float	traveltime;
	float	frames;

	// set destdelta to the vector needed to move
	if (ent->moveinfo.state == STATE_UP)
		VectorSubtract (ent->moveinfo.end_angles, ent->s.angles, destdelta);
	else
		VectorSubtract (ent->moveinfo.start_angles, ent->s.angles, destdelta);
	
	// calculate length of vector
	len = VectorLength (destdelta);
	
	// divide by speed to get time to reach dest
	traveltime = len / ent->moveinfo.speed;

	if (traveltime < FRAMETIME)
	{
		AngleMove_Final (ent);
		return;
	}

	frames = floor(traveltime / FRAMETIME);

	// scale the destdelta vector by the time spent traveling to get velocity
	VectorScale (destdelta, 1.0 / traveltime, ent->avelocity);

	// set nextthink to trigger a think when dest is reached
	ent->nextthink = level.time + frames * FRAMETIME;
	ent->think = AngleMove_Final;
}

void AngleMove_Calc (edict_t *ent, void(*func)(edict_t*))
{
	VectorClear (ent->avelocity);
	ent->moveinfo.endfunc = func;
	if (level.current_entity == ((ent->flags & FL_TEAMSLAVE) ? ent->teammaster : ent))
	{
		AngleMove_Begin (ent);
	}
	else
	{
		ent->nextthink = level.time + FRAMETIME;
		ent->think = AngleMove_Begin;
	}
}


/*
==============
Think_AccelMove

The team has completed a frame of movement, so
change the speed for the next frame
==============
*/
#define AccelerationDistance(target, rate)	(target * ((target / rate) + 1) / 2)

void plat_CalcAcceleratedMove(moveinfo_t *moveinfo)
{
	float	accel_dist;
	float	decel_dist;

	moveinfo->move_speed = moveinfo->speed;

	if (moveinfo->remaining_distance < moveinfo->accel)
	{
		moveinfo->current_speed = moveinfo->remaining_distance;
		return;
	}

	accel_dist = AccelerationDistance (moveinfo->speed, moveinfo->accel);
	decel_dist = AccelerationDistance (moveinfo->speed, moveinfo->decel);

	if ((moveinfo->remaining_distance - accel_dist - decel_dist) < 0)
	{
		float	f;

		f = (moveinfo->accel + moveinfo->decel) / (moveinfo->accel * moveinfo->decel);
		moveinfo->move_speed = (-2 + sqrt(4 - 4 * f * (-2 * moveinfo->remaining_distance))) / (2 * f);
		decel_dist = AccelerationDistance (moveinfo->move_speed, moveinfo->decel);
	}

	moveinfo->decel_distance = decel_dist;
};

void plat_Accelerate (moveinfo_t *moveinfo)
{
	// are we decelerating?
	if (moveinfo->remaining_distance <= moveinfo->decel_distance)
	{
		if (moveinfo->remaining_distance < moveinfo->decel_distance)
		{
			if (moveinfo->next_speed)
			{
				moveinfo->current_speed = moveinfo->next_speed;
				moveinfo->next_speed = 0;
				return;
			}
			if (moveinfo->current_speed > moveinfo->decel)
				moveinfo->current_speed -= moveinfo->decel;
		}
		return;
	}

	// are we at full speed and need to start decelerating during this move?
	if (moveinfo->current_speed == moveinfo->move_speed)
		if ((moveinfo->remaining_distance - moveinfo->current_speed) < moveinfo->decel_distance)
		{
			float	p1_distance;
			float	p2_distance;
			float	distance;

			p1_distance = moveinfo->remaining_distance - moveinfo->decel_distance;
			p2_distance = moveinfo->move_speed * (1.0 - (p1_distance / moveinfo->move_speed));
			distance = p1_distance + p2_distance;
			moveinfo->current_speed = moveinfo->move_speed;
			moveinfo->next_speed = moveinfo->move_speed - moveinfo->decel * (p2_distance / distance);
			return;
		}

	// are we accelerating?
	if (moveinfo->current_speed < moveinfo->speed)
	{
		float	old_speed;
		float	p1_distance;
		float	p1_speed;
		float	p2_distance;
		float	distance;

		old_speed = moveinfo->current_speed;

		// figure simple acceleration up to move_speed
		moveinfo->current_speed += moveinfo->accel;
		if (moveinfo->current_speed > moveinfo->speed)
			moveinfo->current_speed = moveinfo->speed;

		// are we accelerating throughout this entire move?
		if ((moveinfo->remaining_distance - moveinfo->current_speed) >= moveinfo->decel_distance)
			return;

		// during this move we will accelrate from current_speed to move_speed
		// and cross over the decel_distance; figure the average speed for the
		// entire move
		p1_distance = moveinfo->remaining_distance - moveinfo->decel_distance;
		p1_speed = (old_speed + moveinfo->move_speed) / 2.0;
		p2_distance = moveinfo->move_speed * (1.0 - (p1_distance / p1_speed));
		distance = p1_distance + p2_distance;
		moveinfo->current_speed = (p1_speed * (p1_distance / distance)) + (moveinfo->move_speed * (p2_distance / distance));
		moveinfo->next_speed = moveinfo->move_speed - moveinfo->decel * (p2_distance / distance);
		return;
	}

	// we are at constant velocity (move_speed)
	return;
};

void Think_AccelMove (edict_t *ent)
{
	ent->moveinfo.remaining_distance -= ent->moveinfo.current_speed;

	// Knightmare- fix plats with high accel and decel getting stuck just before top and bottom
//	if (ent->moveinfo.current_speed == 0)		// starting or blocked
		plat_CalcAcceleratedMove(&ent->moveinfo);

	plat_Accelerate (&ent->moveinfo);

	// will the entire move complete on next frame?
	if (ent->moveinfo.remaining_distance <= ent->moveinfo.current_speed)
	{
		Move_Final (ent);
		return;
	}

	VectorScale (ent->moveinfo.dir, ent->moveinfo.current_speed*10, ent->velocity);
	ent->nextthink = level.time + FRAMETIME;
	ent->think = Think_AccelMove;
}


void plat_go_down (edict_t *ent);

void plat_hit_top (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ent->attenuation, 0); // Knightmare- was ATTN_STATIC
	//	ent->s.sound = 0;
	}
	ent->s.sound = 0;	// Knightmare- make sure this is always set to 0, lead mover or not!
#ifdef LOOP_SOUND_ATTENUATION	// Knightmare added
	ent->s.attenuation = ent->attenuation;
#endif

	ent->moveinfo.state = STATE_TOP;

	ent->think = plat_go_down;
	ent->nextthink = level.time + 3;
}

void plat_hit_bottom (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ent->attenuation, 0); // Knightmare- was ATTN_STATIC
	//	ent->s.sound = 0;
	}
	ent->s.sound = 0;	// Knightmare- make sure this is always set to 0, lead mover or not!
#ifdef LOOP_SOUND_ATTENUATION	// Knightmare added
	ent->s.attenuation = ent->attenuation;
#endif

	ent->moveinfo.state = STATE_BOTTOM;
}

void plat_go_down (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_start)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ent->attenuation, 0); // Knightmare- was ATTN_STATIC
		ent->s.sound = ent->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION	// Knightmare added
		ent->s.attenuation = ent->attenuation;
#endif
	}
	ent->moveinfo.state = STATE_DOWN;
	Move_Calc (ent, ent->moveinfo.end_origin, plat_hit_bottom);
}

void plat_go_up (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_start)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ent->attenuation, 0); // Knightmare- was ATTN_STATIC
		ent->s.sound = ent->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION	// Knightmare added
		ent->s.attenuation = ent->attenuation;
#endif
	}
	ent->moveinfo.state = STATE_UP;
	Move_Calc (ent, ent->moveinfo.start_origin, plat_hit_top);
}

void plat_blocked (edict_t *self, edict_t *other)
{
	if (!(other->svflags & SVF_MONSTER) && (!other->client))
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other)
			BecomeExplosion1 (other);
		return;
	}

	if (other->deadflag) T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
	else T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

	if (self->moveinfo.state == STATE_UP)
		plat_go_down (self);
	else if (self->moveinfo.state == STATE_DOWN)
		plat_go_up (self);
}


void Use_Plat (edict_t *ent, edict_t *other, edict_t *activator)
{ 
	if (ent->think)
		return;		// already down
	plat_go_down (ent);
}


void Touch_Plat_Center (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;
		
	if (other->health <= 0)
		return;

	ent = ent->enemy;	// now point at the plat, not the trigger
	if (ent->moveinfo.state == STATE_BOTTOM)
		plat_go_up (ent);
	else if (ent->moveinfo.state == STATE_TOP)
		ent->nextthink = level.time + 1;	// the player is still on the plat, so delay going down
}

// PGM - plat2's change the trigger field
//void plat_spawn_inside_trigger (edict_t *ent)
edict_t *plat_spawn_inside_trigger (edict_t *ent)
{
	edict_t	*trigger;
	vec3_t	tmin, tmax;

//
// middle trigger
//	
	trigger = G_Spawn();
	trigger->touch = Touch_Plat_Center;
	trigger->movetype = MOVETYPE_NONE;
	trigger->solid = SOLID_TRIGGER;
	trigger->enemy = ent;
	
	tmin[0] = ent->mins[0] + 25;
	tmin[1] = ent->mins[1] + 25;
	tmin[2] = ent->mins[2];

	tmax[0] = ent->maxs[0] - 25;
	tmax[1] = ent->maxs[1] - 25;
	tmax[2] = ent->maxs[2] + 8;

	// Knightmare- for PLAT_LOW_TRIGGER, we need to add st.lip, not subtract it!
//	tmin[2] = tmax[2] - (ent->pos1[2] - ent->pos2[2] + st.lip);

	if (ent->spawnflags & PLAT_LOW_TRIGGER) {
		tmin[2] = tmax[2] - (ent->pos1[2] - ent->pos2[2]) + st.lip;
		tmax[2] = tmin[2] + 8;
	}
	else
		tmin[2] = tmax[2] - (ent->pos1[2] - ent->pos2[2] + st.lip);
	
	if (tmax[0] - tmin[0] <= 0)
	{
		tmin[0] = (ent->mins[0] + ent->maxs[0]) *0.5;
		tmax[0] = tmin[0] + 1;
	}
	if (tmax[1] - tmin[1] <= 0)
	{
		tmin[1] = (ent->mins[1] + ent->maxs[1]) *0.5;
		tmax[1] = tmin[1] + 1;
	}
	
	VectorCopy (tmin, trigger->mins);
	VectorCopy (tmax, trigger->maxs);

	gi.linkentity (trigger);

	return trigger;			// PGM 11/17/97
}


/*QUAKED func_plat (0 .5 .8) ? PLAT_LOW_TRIGGER
speed	default 150

Plats are always drawn in the extended position, so they will light correctly.

If the plat is the target of another trigger or button, it will start out disabled in the extended position until it is trigger, when it will lower and become a normal plat.

"speed"	overrides default 200.
"accel" overrides default 500
"lip"	overrides default 8 pixel lip

If the "height" key is set, that will determine the amount the plat moves, instead of being implicitly determoveinfoned by the model's height.

Set "sounds" to one of the following:
1) base fast
2) chain slow
*/
void SP_func_plat (edict_t *ent)
{
	gitem_t		*it;		//j
	edict_t		*it_ent;	//j

	VectorClear (ent->s.angles);
	ent->solid = SOLID_BSP;
	ent->movetype = MOVETYPE_PUSH;

	gi.setmodel (ent, ent->model);

	ent->blocked = plat_blocked;

	if (!ent->speed)
		ent->speed = 20;
	else
		ent->speed *= 0.1;

	if (!ent->accel)
		ent->accel = 5;
	else
		ent->accel *= 0.1;

	if (!ent->decel)
		ent->decel = 5;
	else
		ent->decel *= 0.1;

	if (!ent->dmg)
		ent->dmg = 2;

	if (!st.lip)
		st.lip = 8;

	// pos1 is the top position, pos2 is the bottom
	VectorCopy (ent->s.origin, ent->pos1);
	VectorCopy (ent->s.origin, ent->pos2);
	if (st.height)
		ent->pos2[2] -= st.height;
	else
		ent->pos2[2] -= (ent->maxs[2] - ent->mins[2]) - st.lip;

	ent->use = Use_Plat;

	plat_spawn_inside_trigger (ent);	// the "start moving" trigger	

	if (ent->targetname)
	{
		ent->moveinfo.state = STATE_UP;
	}
	else
	{
		VectorCopy (ent->pos2, ent->s.origin);
		gi.linkentity (ent);
		ent->moveinfo.state = STATE_BOTTOM;
	}

	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	VectorCopy (ent->pos1, ent->moveinfo.start_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.start_angles);
	VectorCopy (ent->pos2, ent->moveinfo.end_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.end_angles);

	ent->moveinfo.sound_start = gi.soundindex ("plats/pt1_strt.wav");
	ent->moveinfo.sound_middle = gi.soundindex ("plats/pt1_mid.wav");
	ent->moveinfo.sound_end = gi.soundindex ("plats/pt1_end.wav");

	if (ent->attenuation <= 0)	// Knightmare added
		ent->attenuation = ATTN_STATIC;

////////
	VectorAdd(ent->s.origin,ent->mins,ent->monsterinfo.last_sighting);
//	VectorCopy(ent->s.origin,ent->monsterinfo.last_sighting);


	it = FindItem("Roam Navi");
	it_ent = G_Spawn();
	it_ent->classname = it->classname;
/*	VectorCopy (ent->s.origin,it_ent->s.origin);
	it_ent->s.origin[0] = (ent->s.origin[0] + ent->mins[0] + ent->s.origin[0] + ent->maxs[0])/2;
	it_ent->s.origin[1] = (ent->s.origin[1] + ent->mins[1] + ent->s.origin[1] + ent->maxs[1])/2;
	it_ent->s.origin[2] = 0;*/

	it_ent->union_ent = ent;
	ent->union_ent = it_ent;

	SpawnItem3 (it_ent, it);
}

// ==========================================
// PLAT 2
// ==========================================
#define PLAT2_CALLED		1
#define PLAT2_MOVING		2
#define PLAT2_WAITING		4

void plat2_go_down (edict_t *ent);
void plat2_go_up (edict_t *ent);

void plat2_spawn_danger_area (edict_t *ent)
{
	vec3_t	mins, maxs;

	VectorCopy(ent->mins, mins);
	VectorCopy(ent->maxs, maxs);
	maxs[2] = ent->mins[2] + 64;

//	SpawnBadArea(mins, maxs, 0, ent);
}

void plat2_kill_danger_area (edict_t *ent)
{
	edict_t *t;

	t = NULL;
	while ((t = G_Find (t, FOFS(classname), "bad_area")))
	{
		if (t->owner == ent)
			G_FreeEdict(t);
	}
}

void plat2_hit_top (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ent->attenuation, 0);	// Knightmare- was ATTN_STATIC
	//	ent->s.sound = 0;
	}
	ent->s.sound = 0;	// Knightmare- make sure this is always set to 0, lead mover or not!
	ent->moveinfo.state = STATE_TOP;

	if (ent->plat2flags & PLAT2_CALLED)
	{
		ent->plat2flags = PLAT2_WAITING;
		if (!(ent->spawnflags & PLAT2_TOGGLE))
		{
			ent->think = plat2_go_down;
			ent->nextthink = level.time + 5.0;
		}
		if (deathmatch->value)
			ent->last_move_time = level.time - 1.0;
		else
			ent->last_move_time = level.time - 2.0;
	}
	else if (!(ent->spawnflags & PLAT2_TOP) && !(ent->spawnflags & PLAT2_TOGGLE))
	{
		ent->plat2flags = 0;
		ent->think = plat2_go_down;
		ent->nextthink = level.time + 2.0;
		ent->last_move_time = level.time;
	}
	else
	{
		ent->plat2flags = 0;
		ent->last_move_time = level.time;
	}

	G_UseTargets (ent, ent);
}

void plat2_hit_bottom (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ent->attenuation, 0);	// Knightmare- was ATTN_STATIC
	//	ent->s.sound = 0;
	}
	ent->s.sound = 0;	// Knightmare- make sure this is always set to 0, lead mover or not!
	ent->moveinfo.state = STATE_BOTTOM;
	
	if (ent->plat2flags & PLAT2_CALLED)
	{
		ent->plat2flags = PLAT2_WAITING;
		if (!(ent->spawnflags & PLAT2_TOGGLE))
		{
			ent->think = plat2_go_up;
			ent->nextthink = level.time + 5.0;
		}
		if (deathmatch->value)
			ent->last_move_time = level.time - 1.0;
		else
			ent->last_move_time = level.time - 2.0;
	}
	else if ((ent->spawnflags & PLAT2_TOP) && !(ent->spawnflags & PLAT2_TOGGLE))
	{
		ent->plat2flags = 0;
		ent->think = plat2_go_up;
		ent->nextthink = level.time + 2.0;
		ent->last_move_time = level.time;
	}
	else
	{
		ent->plat2flags = 0;
		ent->last_move_time = level.time;
	}

	plat2_kill_danger_area (ent);
	G_UseTargets (ent, ent);
}

void plat2_go_down (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_start)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ent->attenuation, 0);	// Knightmare- was ATTN_STATIC
		ent->s.sound = ent->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION	// Knightmare added
		ent->s.attenuation = ent->attenuation;
#endif
	}
	ent->moveinfo.state = STATE_DOWN;
	ent->plat2flags |= PLAT2_MOVING;

	Move_Calc (ent, ent->moveinfo.end_origin, plat2_hit_bottom);
}

void plat2_go_up (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_start)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ent->attenuation, 0);	// Knightmare- was ATTN_STATIC
		ent->s.sound = ent->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION	// Knightmare added
		ent->s.attenuation = ent->attenuation;
#endif
	}
	ent->moveinfo.state = STATE_UP;
	ent->plat2flags |= PLAT2_MOVING;

	plat2_spawn_danger_area(ent);

	Move_Calc (ent, ent->moveinfo.start_origin, plat2_hit_top);
}

void plat2_operate (edict_t *ent, edict_t *other)
{
	int		otherState;
	float	pauseTime;
	float	platCenter;
	edict_t *trigger;

	trigger = ent;
	ent = ent->enemy;	// now point at the plat, not the trigger

	if (ent->plat2flags & PLAT2_MOVING)
		return;

	if ((ent->last_move_time + 2) > level.time)
		return;

	platCenter = (trigger->absmin[2] + trigger->absmax[2]) / 2;

	if (ent->moveinfo.state == STATE_TOP)
	{
		otherState = STATE_TOP;
		if (ent->spawnflags & PLAT2_BOX_LIFT)
		{
			if (platCenter > other->s.origin[2])
				otherState = STATE_BOTTOM;
		}
		else
		{
			if (trigger->absmax[2] > other->s.origin[2])
				otherState = STATE_BOTTOM;
		}
	}
	else
	{
		otherState = STATE_BOTTOM;
		if (other->s.origin[2] > platCenter)
			otherState = STATE_TOP;
	}

	ent->plat2flags = PLAT2_MOVING;

	if (deathmatch->value)
		pauseTime = 0.3;
	else
		pauseTime = 0.5;

	if (ent->moveinfo.state != otherState)
	{
		ent->plat2flags |= PLAT2_CALLED;
		pauseTime = 0.1;
	}

	ent->last_move_time = level.time;
	
	if (ent->moveinfo.state == STATE_BOTTOM)
	{
		ent->think = plat2_go_up;
		ent->nextthink = level.time + pauseTime;
	}
	else
	{
		ent->think = plat2_go_down;
		ent->nextthink = level.time + pauseTime;
	}
}

void Touch_Plat_Center2 (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// this requires monsters to actively trigger plats, not just step on them.

	//FIXME - commented out for E3
	//if (!other->client)
	//	return;
		
	if (other->health <= 0)
		return;

	// PMM - don't let non-monsters activate plat2s
	if ((!(other->svflags & SVF_MONSTER)) && (!other->client))
		return;
	
	plat2_operate (ent, other);
}

void plat2_blocked (edict_t *self, edict_t *other)
{
	if (!(other->svflags & SVF_MONSTER) && (!other->client))
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other && other->inuse)
			BecomeExplosion1 (other);
		return;
	}

	// gib dead things
	if (other->health < 1)
	{
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 100, 1, 0, MOD_CRUSH);
	}

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

	if (self->moveinfo.state == STATE_UP)
		plat2_go_down (self);
	else if (self->moveinfo.state == STATE_DOWN)
		plat2_go_up (self);
}

void Use_Plat2 (edict_t *ent, edict_t *other, edict_t *activator)
{ 
	edict_t		*trigger;
	int			i;

	if (ent->moveinfo.state > STATE_BOTTOM)
		return;
	if ((ent->last_move_time + 2) > level.time)
		return;

	for (i = 1, trigger = g_edicts + 1; i < globals.num_edicts; i++, trigger++)
	{
		if (!trigger->inuse)
			continue;
		if (trigger->touch == Touch_Plat_Center2)
		{
			if (trigger->enemy == ent)
			{
//				Touch_Plat_Center2 (trigger, activator, NULL, NULL);
				plat2_operate (trigger, activator);
				return;
			}
		} 
	}
}

void plat2_activate (edict_t *ent, edict_t *other, edict_t *activator)
{
	edict_t *trigger;

//	if (ent->targetname)
//		ent->targetname[0] = 0;

	ent->use = Use_Plat2;

	trigger = plat_spawn_inside_trigger (ent);	// the "start moving" trigger	

	trigger->maxs[0]+=10;
	trigger->maxs[1]+=10;
	trigger->mins[0]-=10;
	trigger->mins[1]-=10;

	gi.linkentity (trigger);
	
	trigger->touch = Touch_Plat_Center2;		// Override trigger touch function

	plat2_go_down(ent);
}

/*QUAKED func_plat2 (0 .5 .8) ? PLAT_LOW_TRIGGER PLAT2_TOGGLE PLAT2_TOP PLAT2_TRIGGER_TOP PLAT2_TRIGGER_BOTTOM BOX_LIFT
speed	default 150

PLAT_LOW_TRIGGER - creates a short trigger field at the bottom
PLAT2_TOGGLE - plat will not return to default position.
PLAT2_TOP - plat's default position will the the top.
PLAT2_TRIGGER_TOP - plat will trigger it's targets each time it hits top
PLAT2_TRIGGER_BOTTOM - plat will trigger it's targets each time it hits bottom
BOX_LIFT - this indicates that the lift is a box, rather than just a platform

Plats are always drawn in the extended position, so they will light correctly.

If the plat is the target of another trigger or button, it will start out disabled in the extended position until it is trigger, when it will lower and become a normal plat.

"speed"	overrides default 200.
"accel" overrides default 500
"lip"	no default

If the "height" key is set, that will determine the amount the plat moves, instead of being implicitly determoveinfoned by the model's height.

*/
void SP_func_plat2 (edict_t *ent)
{
	edict_t *trigger;

	VectorClear (ent->s.angles);
	ent->solid = SOLID_BSP;
	ent->movetype = MOVETYPE_PUSH;

	gi.setmodel (ent, ent->model);

	ent->blocked = plat2_blocked;

	if (!ent->speed)
		ent->speed = 20;
	else
		ent->speed *= 0.1;

	if (!ent->accel)
		ent->accel = 5;
	else
		ent->accel *= 0.1;

	if (!ent->decel)
		ent->decel = 5;
	else
		ent->decel *= 0.1;

	if (deathmatch->value)
	{
		ent->speed *= 2;
		ent->accel *= 2;
		ent->decel *= 2;
	}


	//PMM Added to kill things it's being blocked by 
	if (!ent->dmg)
		ent->dmg = 2;

//	if (!st.lip)
//		st.lip = 8;

	// pos1 is the top position, pos2 is the bottom
	VectorCopy (ent->s.origin, ent->pos1);
	VectorCopy (ent->s.origin, ent->pos2);

	if (st.height)
		ent->pos2[2] -= (st.height - st.lip);
	else
		ent->pos2[2] -= (ent->maxs[2] - ent->mins[2]) - st.lip;

	ent->moveinfo.state = STATE_TOP;

	if (ent->targetname)
	{
		ent->use = plat2_activate;
	}
	else
	{
		ent->use = Use_Plat2;

		trigger = plat_spawn_inside_trigger (ent);	// the "start moving" trigger	

		// PGM - debugging??
		trigger->maxs[0]+=10;
		trigger->maxs[1]+=10;
		trigger->mins[0]-=10;
		trigger->mins[1]-=10;

		gi.linkentity (trigger);

		trigger->touch = Touch_Plat_Center2;		// Override trigger touch function

		if (!(ent->spawnflags & PLAT2_TOP))
		{
			VectorCopy (ent->pos2, ent->s.origin);
			ent->moveinfo.state = STATE_BOTTOM;
		}	
	}

	gi.linkentity (ent);

	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	VectorCopy (ent->pos1, ent->moveinfo.start_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.start_angles);
	VectorCopy (ent->pos2, ent->moveinfo.end_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.end_angles);

	ent->moveinfo.sound_start = gi.soundindex ("plats/pt1_strt.wav");
	ent->moveinfo.sound_middle = gi.soundindex ("plats/pt1_mid.wav");
	ent->moveinfo.sound_end = gi.soundindex ("plats/pt1_end.wav");

	if (ent->attenuation <= 0)	// Knightmare added
		ent->attenuation = ATTN_STATIC;

//Maj++
//	bFuncPlat(ent);
//Maj--

}

//====================================================================

/*QUAKED func_rotating (0 .5 .8) ? START_ON REVERSE X_AXIS Y_AXIS TOUCH_PAIN STOP ANIMATED ANIMATED_FAST
You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"speed" determines how fast it moves; default value is 100.
"dmg"	damage to inflict when blocked (2 default)

REVERSE will cause the it to rotate in the opposite direction.
STOP mean it will stop moving instead of pushing entities
*/

void rotating_blocked (edict_t *self, edict_t *other)
{
	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void rotating_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//ponko
	if (other->svflags & SVF_MONSTER) return;

	if (self->avelocity[0] || self->avelocity[1] || self->avelocity[2])
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void rotating_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!VectorCompare (self->avelocity, vec3_origin))
	{
		self->s.sound = 0;
		VectorClear (self->avelocity);
		self->touch = NULL;
	}
	else
	{
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION	// Knightmare added
		self->s.attenuation = self->attenuation;
#endif
		VectorScale (self->movedir, self->speed, self->avelocity);
		if (self->spawnflags & 16)
			self->touch = rotating_touch;
	}
}

void SP_func_rotating (edict_t *ent)
{
	ent->solid = SOLID_BSP;
	if (ent->spawnflags & 32)
		ent->movetype = MOVETYPE_STOP;
	else
		ent->movetype = MOVETYPE_PUSH;

	// set the axis of rotation
	VectorClear(ent->movedir);
	if (ent->spawnflags & 4)
		ent->movedir[2] = 1.0;
	else if (ent->spawnflags & 8)
		ent->movedir[0] = 1.0;
	else // Z_AXIS
		ent->movedir[1] = 1.0;

	// check for reverse rotation
	if (ent->spawnflags & 2)
		VectorNegate (ent->movedir, ent->movedir);

	if (!ent->speed)
		ent->speed = 100;
	if (!ent->dmg)
		ent->dmg = 2;

//	ent->moveinfo.sound_middle = "doors/hydro1.wav";

	if (ent->attenuation <= 0)	// Knightmare added
		ent->attenuation = ATTN_STATIC;

	ent->use = rotating_use;
	if (ent->dmg)
		ent->blocked = rotating_blocked;

	if (ent->spawnflags & 1)
		ent->use (ent, NULL, NULL);

	if (ent->spawnflags & 64)
		ent->s.effects |= EF_ANIM_ALL;
	if (ent->spawnflags & 128)
		ent->s.effects |= EF_ANIM_ALLFAST;

	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);
}

/*
======================================================================

BUTTONS

======================================================================
*/

/*QUAKED func_button (0 .5 .8) ?
When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets, waits some time, then returns to it's original position where it can be triggered again.

"angle"		determines the opening direction
"target"	all entities with a matching targetname will be used
"speed"		override the default 40 speed
"wait"		override the default 1 second wait (-1 = never return)
"lip"		override the default 4 pixel lip remaining at end of move
"health"	if set, the button must be killed instead of touched
"sounds"
1) silent
2) steam metal
3) wooden clunk
4) metallic click
5) in-out
*/

void button_done (edict_t *self)
{
	self->moveinfo.state = STATE_BOTTOM;
	self->s.effects &= ~EF_ANIM23;
	self->s.effects |= EF_ANIM01;
}

void button_return (edict_t *self)
{
	self->moveinfo.state = STATE_DOWN;

	Move_Calc (self, self->moveinfo.start_origin, button_done);

	self->s.frame = 0;

	if (self->health)
		self->takedamage = DAMAGE_YES;
}

void button_wait (edict_t *self)
{
	self->moveinfo.state = STATE_TOP;
	self->s.effects &= ~EF_ANIM01;
	self->s.effects |= EF_ANIM23;

	G_UseTargets (self, self->activator);
	self->s.frame = 1;
	if (self->moveinfo.wait >= 0)
	{
		self->nextthink = level.time + self->moveinfo.wait;
		self->think = button_return;
	}
}

void button_fire (edict_t *self)
{
	if (self->moveinfo.state == STATE_UP || self->moveinfo.state == STATE_TOP)
		return;


	if (self->activator)
	{
		if (chedit->value && CurrentIndex < MAXNODES && !self->activator->deadflag && self->activator == &g_edicts[1])
		{
			VectorCopy(self->monsterinfo.last_sighting,Route[CurrentIndex].Pt);
			Route[CurrentIndex].ent = self;
			Route[CurrentIndex].state = GRS_PUSHBUTTON;
			if (++CurrentIndex < MAXNODES)
			{
				gi.bprintf(PRINT_HIGH,"Last %i pod(s).\n",MAXNODES - CurrentIndex);
				memset(&Route[CurrentIndex],0,sizeof(route_t));
				Route[CurrentIndex].index = Route[CurrentIndex - 1].index +1;
			}			
		
		}
	}


	self->moveinfo.state = STATE_UP;
	if (self->moveinfo.sound_start && !(self->flags & FL_TEAMSLAVE))
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // Knightmare- was ATTN_STATIC
	Move_Calc (self, self->moveinfo.end_origin, button_wait);
}

void button_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;
	button_fire (self);
}

void button_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;

	if (other->health <= 0)
		return;

	self->activator = other;
	button_fire (self);
}

void button_killed (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	if (self->takedamage) self->monsterinfo.attack_finished = level.time + FRAMETIME * 40; 
		
	self->activator = attacker;
	self->health = self->max_health;
	self->takedamage = DAMAGE_NO;
	button_fire (self);
}

void SP_func_button (edict_t *ent)
{
	vec3_t	abs_movedir,tdir,tdir2;
	float	dist;
	gitem_t		*it;		//j
	edict_t		*it_ent;	//j
	int		i;

	G_SetMovedir (ent->s.angles, ent->movedir);
	ent->movetype = MOVETYPE_STOP;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);

	if (ent->sounds != 1)
		ent->moveinfo.sound_start = gi.soundindex ("switches/butn2.wav");

	if (ent->attenuation <= 0)	// Knightmare added
		ent->attenuation = ATTN_STATIC;

	if (!ent->speed)
		ent->speed = 40;
	if (!ent->accel)
		ent->accel = ent->speed;
	if (!ent->decel)
		ent->decel = ent->speed;

	if (!ent->wait)
		ent->wait = 3;
	if (!st.lip)
		st.lip = 4;

	VectorCopy (ent->s.origin, ent->pos1);
	abs_movedir[0] = fabs(ent->movedir[0]);
	abs_movedir[1] = fabs(ent->movedir[1]);
	abs_movedir[2] = fabs(ent->movedir[2]);
	dist = abs_movedir[0] * ent->size[0] + abs_movedir[1] * ent->size[1] + abs_movedir[2] * ent->size[2] - st.lip;
	VectorMA (ent->pos1, dist, ent->movedir, ent->pos2);

	ent->use = button_use;
	ent->s.effects |= EF_ANIM01;

	if (ent->health)
	{
		ent->max_health = ent->health;
		ent->die = button_killed;
		ent->takedamage = DAMAGE_YES;
	}
	else if (! ent->targetname)
		ent->touch = button_touch;

	ent->moveinfo.state = STATE_BOTTOM;

	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	VectorCopy (ent->pos1, ent->moveinfo.start_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.start_angles);
	VectorCopy (ent->pos2, ent->moveinfo.end_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.end_angles);

	gi.linkentity (ent);

	VectorAdd(ent->s.origin,ent->mins,ent->monsterinfo.last_sighting);
//	VectorCopy(ent->s.origin,ent->monsterinfo.last_sighting);

	if (1/*!ent->health*/)
	{
		//sp roam navi
		it = FindItem("Roam Navi2");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		VectorCopy (ent->s.origin,it_ent->s.origin);
		it_ent->s.origin[0] = (ent->absmin[0] + ent->absmax[0])/2;
		it_ent->s.origin[1] = (ent->absmin[1] + ent->absmax[1])/2;
		it_ent->s.origin[2] = (ent->absmin[2] + ent->absmax[2])/2;

		it_ent->union_ent = ent;
		ent->union_ent = it_ent;

		VectorSubtract (ent->moveinfo.start_origin, ent->moveinfo.end_origin, abs_movedir);
		VectorNormalize (abs_movedir);
		dist = 1;
		while(dist < 500)
		{
			VectorScale (abs_movedir, dist, tdir);
			VectorAdd(it_ent->s.origin,tdir,tdir2);
			i = gi.pointcontents(tdir2);
			if (!(i & CONTENTS_SOLID) ) break;
			dist++;
		}
		VectorScale (abs_movedir, (dist + 20), tdir);
		VectorAdd(it_ent->s.origin,tdir,tdir2);
		VectorCopy(tdir2,it_ent->s.origin);

	it_ent->item = it;
	it_ent->s.effects = 0;
	it_ent->s.renderfx = 0;
	it_ent->s.modelindex  = 0;
//it_ent->s.modelindex =gi.modelindex ("models/items/armor/body/tris.md2");

	it_ent->solid = SOLID_TRIGGER;
	it_ent->movetype = MOVETYPE_NONE;  
	it_ent->touch = Touch_Item;

	gi.linkentity (it_ent);



//		SpawnItem3 (it_ent, it);
	}
}

/*
======================================================================

DOORS

  spawn a trigger surrounding the entire team unless it is
  already targeted by another

======================================================================
*/

/*QUAKED func_door (0 .5 .8) ? START_OPEN x CRUSHER NOMONSTER ANIMATED TOGGLE ANIMATED_FAST
TOGGLE		wait in both the start and end states for a trigger event.
START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER	monsters will not trigger this door

"message"	is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"	if set, door must be shot open
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"lip"		lip remaining at end of move (8 default)
"dmg"		damage to inflict when blocked (2 default)
"sounds"
1)	silent
2)	light
3)	medium
4)	heavy
*/

void door_use_areaportals (edict_t *self, qboolean open)
{
	edict_t	*t = NULL;

	if (!self->target)
		return;

	while ((t = G_Find (t, FOFS(targetname), self->target)))
	{
		if (Q_stricmp(t->classname, "func_areaportal") == 0)
		{
			gi.SetAreaPortalState (t->style, open);
		}
	}
}

void door_go_down (edict_t *self);

void door_hit_top (edict_t *self)
{
	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_end)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // Knightmare- was ATTN_STATIC
	//	self->s.sound = 0;
	}
	self->s.sound = 0;	// Knightmare- make sure this is always set to 0, lead mover or not!

	self->moveinfo.state = STATE_TOP;
	if (self->spawnflags & DOOR_TOGGLE)
	{
		if (self->union_ent)
		{
			self->union_ent->solid = SOLID_NOT;
		}
		return;
	}
	if (self->moveinfo.wait >= 0)
	{
		self->think = door_go_down;
		self->nextthink = level.time + self->moveinfo.wait;
	}
}

void door_hit_bottom (edict_t *self)
{
	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_end)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // Knightmare- was ATTN_STATIC
	//	self->s.sound = 0;
	}
	self->s.sound = 0;	// Knightmare- make sure this is always set to 0, lead mover or not!

	if (self->union_ent)
	{
		self->union_ent->solid = SOLID_NOT;
	}

	self->moveinfo.state = STATE_BOTTOM;
	door_use_areaportals (self, false);
}

void door_go_down (edict_t *self)
{
	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_start)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // Knightmare- was ATTN_STATIC
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION	// Knightmare added
		self->s.attenuation = self->attenuation;
#endif
	}
	if (self->max_health)
	{
		self->takedamage = DAMAGE_YES;
		self->health = self->max_health;
	}
	
	self->moveinfo.state = STATE_DOWN;
	if (strcmp(self->classname, "func_door") == 0)
		Move_Calc (self, self->moveinfo.start_origin, door_hit_bottom);
	else if (strcmp(self->classname, "func_door_rotating") == 0)
		AngleMove_Calc (self, door_hit_bottom);
}

void door_go_up (edict_t *self, edict_t *activator)
{
	if (self->moveinfo.state == STATE_UP)
		return;		// already going up

	if (self->moveinfo.state == STATE_TOP)
	{	// reset top wait time
		if (self->moveinfo.wait >= 0)
			self->nextthink = level.time + self->moveinfo.wait;
		return;
	}
	
	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_start)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // Knightmare- was ATTN_STATIC
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION	// Knightmare added
		self->s.attenuation = self->attenuation;
#endif
	}
	self->moveinfo.state = STATE_UP;
	if (strcmp(self->classname, "func_door") == 0)
		Move_Calc (self, self->moveinfo.end_origin, door_hit_top);
	else if (strcmp(self->classname, "func_door_rotating") == 0)
		AngleMove_Calc (self, door_hit_top);

	G_UseTargets (self, activator);
	door_use_areaportals (self, true);
}

void door_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*ent;

	if (self->flags & FL_TEAMSLAVE)
		return;

	if (self->spawnflags & DOOR_TOGGLE)
	{
		if (self->moveinfo.state == STATE_UP || self->moveinfo.state == STATE_TOP)
		{
			// trigger all paired doors
			for (ent = self ; ent ; ent = ent->teamchain)
			{
				ent->message = NULL;
				ent->touch = NULL;
				door_go_down (ent);
			}
			return;
		}
	}
	
	// trigger all paired doors
	for (ent = self ; ent ; ent = ent->teamchain)
	{
		ent->message = NULL;
		ent->touch = NULL;
		door_go_up (ent, activator);
	}
};

void Touch_DoorTrigger (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->health <= 0)
		return;

	if (!(other->svflags & SVF_MONSTER) && (!other->client))
		return;

//	if ((self->owner->spawnflags & DOOR_NOMONSTER) && (other->svflags & SVF_MONSTER))
//		return;

	if (level.time < self->touch_debounce_time)
		return;
	self->touch_debounce_time = level.time + 1.0;

	door_use (self->owner, other, other);
}

void Think_CalcMoveSpeed (edict_t *self)
{
	edict_t	*ent;
	float	min;
	float	time;
	float	newspeed;
	float	ratio;
	float	dist;

	if (self->flags & FL_TEAMSLAVE)
		return;		// only the team master does this

	// find the smallest distance any member of the team will be moving
	min = fabs(self->moveinfo.distance);
	for (ent = self->teamchain; ent; ent = ent->teamchain)
	{
		dist = fabs(ent->moveinfo.distance);
		if (dist < min)
			min = dist;
	}

	time = min / self->moveinfo.speed;

	// adjust speeds so they will all complete at the same time
	for (ent = self; ent; ent = ent->teamchain)
	{
		newspeed = fabs(ent->moveinfo.distance) / time;
		ratio = newspeed / ent->moveinfo.speed;
		if (ent->moveinfo.accel == ent->moveinfo.speed)
			ent->moveinfo.accel = newspeed;
		else
			ent->moveinfo.accel *= ratio;
		if (ent->moveinfo.decel == ent->moveinfo.speed)
			ent->moveinfo.decel = newspeed;
		else
			ent->moveinfo.decel *= ratio;
		ent->moveinfo.speed = newspeed;
	}
}

void Think_SpawnDoorTrigger (edict_t *ent)
{
	edict_t		*other;
	vec3_t		mins, maxs;

	if (ent->flags & FL_TEAMSLAVE)
		return;		// only the team leader spawns a trigger

	VectorCopy (ent->absmin, mins);
	VectorCopy (ent->absmax, maxs);

	for (other = ent->teamchain ; other ; other=other->teamchain)
	{
		AddPointToBounds (other->absmin, mins, maxs);
		AddPointToBounds (other->absmax, mins, maxs);
	}

	// expand 
	mins[0] -= 60;
	mins[1] -= 60;
	maxs[0] += 60;
	maxs[1] += 60;

	other = G_Spawn ();
	VectorCopy (mins, other->mins);
	VectorCopy (maxs, other->maxs);
	other->owner = ent;
	other->solid = SOLID_TRIGGER;
	other->movetype = MOVETYPE_NONE;
	other->touch = Touch_DoorTrigger;
	gi.linkentity (other);

	if (ent->spawnflags & DOOR_START_OPEN)
		door_use_areaportals (ent, true);

	Think_CalcMoveSpeed (ent);
}

void door_blocked  (edict_t *self, edict_t *other)
{
	edict_t	*ent;
	int	i;

	if (!(other->svflags & SVF_MONSTER) && (!other->client) )
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other)
			BecomeExplosion1 (other);
		return;
	}
	if (other->deadflag) T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
	else T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

	//bot's state change
	for ( i = 1 ; i <= maxclients->value; i++)
	{
		ent = &g_edicts[i];

		if (ent->inuse && (ent->svflags & SVF_MONSTER) && ent->client)
		{
			if (ent->client->zc.waitin_obj == self && ent->client->zc.zcstate )
			{
				ent->client->zc.zcstate |= STS_W_DONT;
			}
		}
	}
	//

//	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

	if (self->spawnflags & DOOR_CRUSHER)
		return;


// if a door has a negative wait, it would never come back if blocked,
// so let it just squash the object to death real fast
	if (self->moveinfo.wait >= 0)
	{
		if (self->moveinfo.state == STATE_DOWN)
		{
			for (ent = self->teammaster ; ent ; ent = ent->teamchain)
				door_go_up (ent, ent->activator);
		}
		else
		{
			for (ent = self->teammaster ; ent ; ent = ent->teamchain)
				door_go_down (ent);
		}
	}
}

void door_killed (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t	*ent;

	for (ent = self->teammaster ; ent ; ent = ent->teamchain)
	{
		ent->health = ent->max_health;
		ent->takedamage = DAMAGE_NO;
	}
	door_use (self->teammaster, attacker, attacker);
}

void door_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;

	if (level.time < self->touch_debounce_time)
		return;
	self->touch_debounce_time = level.time + 5.0;

	if (!(other->svflags & SVF_MONSTER))
	{
		gi.centerprintf (other, "%s", self->message);
		gi.sound (other, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}
}

void SP_func_door (edict_t *ent)
{
	gitem_t		*it;		//j
	edict_t		*it_ent;	//j

	vec3_t	abs_movedir;

	if (ent->sounds != 1)
	{
		ent->moveinfo.sound_start = gi.soundindex  ("doors/dr1_strt.wav");
		ent->moveinfo.sound_middle = gi.soundindex  ("doors/dr1_mid.wav");
		ent->moveinfo.sound_end = gi.soundindex  ("doors/dr1_end.wav");
	}

	if (ent->attenuation <= 0)	// Knightmare added
		ent->attenuation = ATTN_STATIC;

	G_SetMovedir (ent->s.angles, ent->movedir);
	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);

	ent->blocked = door_blocked;
	ent->use = door_use;
	
	if (!ent->speed)
		ent->speed = 100;
	if (deathmatch->value)
		ent->speed *= 2;

	if (!ent->accel)
		ent->accel = ent->speed;
	if (!ent->decel)
		ent->decel = ent->speed;

	if (!ent->wait)
		ent->wait = 3;
	if (!st.lip)
		st.lip = 8;
	if (!ent->dmg)
		ent->dmg = 2;

	// calculate second position
	VectorCopy (ent->s.origin, ent->pos1);
	abs_movedir[0] = fabs(ent->movedir[0]);
	abs_movedir[1] = fabs(ent->movedir[1]);
	abs_movedir[2] = fabs(ent->movedir[2]);
	ent->moveinfo.distance = abs_movedir[0] * ent->size[0] + abs_movedir[1] * ent->size[1] + abs_movedir[2] * ent->size[2] - st.lip;
	VectorMA (ent->pos1, ent->moveinfo.distance, ent->movedir, ent->pos2);

	// if it starts open, switch the positions
	if (ent->spawnflags & DOOR_START_OPEN)
	{
		VectorCopy (ent->pos2, ent->s.origin);
		VectorCopy (ent->pos1, ent->pos2);
		VectorCopy (ent->s.origin, ent->pos1);
	}

	ent->moveinfo.state = STATE_BOTTOM;

	if (ent->health)
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = door_killed;
		ent->max_health = ent->health;
	}
	else if (ent->targetname && ent->message)
	{
		gi.soundindex ("misc/talk.wav");
		ent->touch = door_touch;
	}
	
	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	VectorCopy (ent->pos1, ent->moveinfo.start_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.start_angles);
	VectorCopy (ent->pos2, ent->moveinfo.end_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.end_angles);

	if (ent->spawnflags & 16)
		ent->s.effects |= EF_ANIM_ALL;
	if (ent->spawnflags & 64)
		ent->s.effects |= EF_ANIM_ALLFAST;

	// to simplify logic elsewhere, make non-teamed doors into a team of one
	if (!ent->team)
		ent->teammaster = ent;

	gi.linkentity (ent);

	ent->nextthink = level.time + FRAMETIME;
	if (ent->health || ent->targetname)
		ent->think = Think_CalcMoveSpeed;
	else
		ent->think = Think_SpawnDoorTrigger;

	VectorAdd(ent->s.origin,ent->mins,ent->monsterinfo.last_sighting);
//	VectorCopy(ent->s.origin,ent->monsterinfo.last_sighting);

/////////
	if (fabs(ent->moveinfo.start_origin[2] - ent->moveinfo.end_origin[2]) >20)
	{
		it = FindItem("Roam Navi3");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;

		it_ent->union_ent = ent;
		ent->union_ent = it_ent;

		SpawnItem3 (it_ent, it);
	}
}


/*QUAKED func_door_rotating (0 .5 .8) ? START_OPEN REVERSE CRUSHER NOMONSTER ANIMATED TOGGLE X_AXIS Y_AXIS
TOGGLE causes the door to wait in both the start and end states for a trigger event.

START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER	monsters will not trigger this door

You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"distance" is how many degrees the door will be rotated.
"speed" determines how fast the door moves; default value is 100.

REVERSE will cause the door to rotate in the opposite direction.

"message"	is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"	if set, door must be shot open
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"dmg"		damage to inflict when blocked (2 default)
"sounds"
1)	silent
2)	light
3)	medium
4)	heavy
*/

void SP_func_door_rotating (edict_t *ent)
{
	VectorClear (ent->s.angles);

	// set the axis of rotation
	VectorClear(ent->movedir);
	if (ent->spawnflags & DOOR_X_AXIS)
		ent->movedir[2] = 1.0;
	else if (ent->spawnflags & DOOR_Y_AXIS)
		ent->movedir[0] = 1.0;
	else // Z_AXIS
		ent->movedir[1] = 1.0;

	// check for reverse rotation
	if (ent->spawnflags & DOOR_REVERSE)
		VectorNegate (ent->movedir, ent->movedir);

	if (!st.distance)
	{
		gi.dprintf("%s at %s with no distance set\n", ent->classname, vtos(ent->s.origin));
		st.distance = 90;
	}

	VectorCopy (ent->s.angles, ent->pos1);
	VectorMA (ent->s.angles, st.distance, ent->movedir, ent->pos2);
	ent->moveinfo.distance = st.distance;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);

	ent->blocked = door_blocked;
	ent->use = door_use;

	if (!ent->speed)
		ent->speed = 100;
	if (!ent->accel)
		ent->accel = ent->speed;
	if (!ent->decel)
		ent->decel = ent->speed;

	if (!ent->wait)
		ent->wait = 3;
	if (!ent->dmg)
		ent->dmg = 2;

	if (ent->sounds != 1)
	{
		ent->moveinfo.sound_start = gi.soundindex  ("doors/dr1_strt.wav");
		ent->moveinfo.sound_middle = gi.soundindex  ("doors/dr1_mid.wav");
		ent->moveinfo.sound_end = gi.soundindex  ("doors/dr1_end.wav");
	}

	if (ent->attenuation <= 0)	// Knightmare added
		ent->attenuation = ATTN_STATIC;

	// if it starts open, switch the positions
	if (ent->spawnflags & DOOR_START_OPEN)
	{
		VectorCopy (ent->pos2, ent->s.angles);
		VectorCopy (ent->pos1, ent->pos2);
		VectorCopy (ent->s.angles, ent->pos1);
		VectorNegate (ent->movedir, ent->movedir);
	}

	if (ent->health)
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = door_killed;
		ent->max_health = ent->health;
	}
	
	if (ent->targetname && ent->message)
	{
		gi.soundindex ("misc/talk.wav");
		ent->touch = door_touch;
	}

	ent->moveinfo.state = STATE_BOTTOM;
	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	VectorCopy (ent->s.origin, ent->moveinfo.start_origin);
	VectorCopy (ent->pos1, ent->moveinfo.start_angles);
	VectorCopy (ent->s.origin, ent->moveinfo.end_origin);
	VectorCopy (ent->pos2, ent->moveinfo.end_angles);

	if (ent->spawnflags & 16)
		ent->s.effects |= EF_ANIM_ALL;

	// to simplify logic elsewhere, make non-teamed doors into a team of one
	if (!ent->team)
		ent->teammaster = ent;

	gi.linkentity (ent);

	ent->nextthink = level.time + FRAMETIME;
	if (ent->health || ent->targetname)
		ent->think = Think_CalcMoveSpeed;
	else
		ent->think = Think_SpawnDoorTrigger;
}


/*QUAKED func_water (0 .5 .8) ? START_OPEN
func_water is a moveable water brush.  It must be targeted to operate.  Use a non-water texture at your own risk.

START_OPEN causes the water to move to its destination when spawned and operate in reverse.

"angle"		determines the opening direction (up or down only)
"speed"		movement speed (25 default)
"wait"		wait before returning (-1 default, -1 = TOGGLE)
"lip"		lip remaining at end of move (0 default)
"sounds"	(yes, these need to be changed)
0)	no sound
1)	water
2)	lava
*/

void SP_func_water (edict_t *self)
{
	vec3_t	abs_movedir;

	G_SetMovedir (self->s.angles, self->movedir);
	self->movetype = MOVETYPE_PUSH;
	self->solid = SOLID_BSP;
	gi.setmodel (self, self->model);

	switch (self->sounds)
	{
		default:
			break;

		case 1: // water
			self->moveinfo.sound_start = gi.soundindex  ("world/mov_watr.wav");
			self->moveinfo.sound_end = gi.soundindex  ("world/stp_watr.wav");
			break;

		case 2: // lava
			self->moveinfo.sound_start = gi.soundindex  ("world/mov_watr.wav");
			self->moveinfo.sound_end = gi.soundindex  ("world/stp_watr.wav");
			break;
	}

	// calculate second position
	VectorCopy (self->s.origin, self->pos1);
	abs_movedir[0] = fabs(self->movedir[0]);
	abs_movedir[1] = fabs(self->movedir[1]);
	abs_movedir[2] = fabs(self->movedir[2]);
	self->moveinfo.distance = abs_movedir[0] * self->size[0] + abs_movedir[1] * self->size[1] + abs_movedir[2] * self->size[2] - st.lip;
	VectorMA (self->pos1, self->moveinfo.distance, self->movedir, self->pos2);

	// if it starts open, switch the positions
	if (self->spawnflags & DOOR_START_OPEN)
	{
		VectorCopy (self->pos2, self->s.origin);
		VectorCopy (self->pos1, self->pos2);
		VectorCopy (self->s.origin, self->pos1);
	}

	VectorCopy (self->pos1, self->moveinfo.start_origin);
	VectorCopy (self->s.angles, self->moveinfo.start_angles);
	VectorCopy (self->pos2, self->moveinfo.end_origin);
	VectorCopy (self->s.angles, self->moveinfo.end_angles);

	self->moveinfo.state = STATE_BOTTOM;

	if (!self->speed)
		self->speed = 25;
	self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed = self->speed;

	if (!self->wait)
		self->wait = -1;
	self->moveinfo.wait = self->wait;

	self->use = door_use;

	if (self->wait == -1)
		self->spawnflags |= DOOR_TOGGLE;

	self->classname = "func_door";

	VectorAdd(self->s.origin,self->mins,self->monsterinfo.last_sighting);
//	VectorCopy(self->s.origin,self->monsterinfo.last_sighting);

	gi.linkentity (self);
}


#define TRAIN_START_ON		1
#define TRAIN_TOGGLE		2
#define TRAIN_BLOCK_STOPS	4

/*QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
speed	default 100
dmg		default	2
noise	looping sound to play when the train is in motion

*/

// Lazarus: Added health key to func_train
void train_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	edict_t	*e, *next;

	if (self->deathtarget)
	{
		self->target = self->deathtarget;
		G_UseTargets (self, attacker);
	}

/*	e = self->movewith_next;
	while(e) {
		next = e->movewith_next;
		e->nextthink = 0;
		if (e->takedamage)
			T_Damage (e, self, self, vec3_origin, e->s.origin, vec3_origin, 100000, 1, DAMAGE_NO_PROTECTION, MOD_CRUSH);
		else if (e->die)
			e->die(e,self,self,100000,e->s.origin);
		else if (e->solid == SOLID_NOT)
			G_FreeEdict(e);
		else
			BecomeExplosion1 (e);
		e = next;
	}
	*/
	BecomeExplosion1 (self);
}

void train_next (edict_t *self);

void train_blocked (edict_t *self, edict_t *other)
{
	if (!(other->svflags & SVF_MONSTER) && (!other->client) )
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other)
			BecomeExplosion1 (other);
		return;
	}

	if (level.time < self->touch_debounce_time)
		return;

	if (!self->dmg)
		return;
	self->touch_debounce_time = level.time + 0.5;
	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void train_wait (edict_t *self)
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
	if (self->target_ent)
	{
		if (self->target_ent->speed)
		{
			self->speed = self->target_ent->speed;
			self->moveinfo.speed = self->speed;
			self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;
		}
		if (self->spawnflags & TRAIN_ROTATE)
		{
			if (self->target_ent->pitch_speed)
				self->pitch_speed = self->target_ent->pitch_speed;
			if (self->target_ent->yaw_speed)
				self->yaw_speed   = self->target_ent->yaw_speed;
			if (self->target_ent->roll_speed)
				self->roll_speed  = self->target_ent->roll_speed;
		}
		else if (self->spawnflags & TRAIN_ROTATE_CONSTANT)
		{
			if (self->target_ent->pitch_speed)
				self->pitch_speed += self->target_ent->pitch_speed;
			if (self->target_ent->yaw_speed)
				self->yaw_speed   += self->target_ent->yaw_speed;
			if (self->target_ent->roll_speed)
				self->roll_speed  += self->target_ent->roll_speed;
		}
	}

	if (self->moveinfo.wait)
	{
		// Knightmare added
		// Spline trains stop rotating when waiting
		if (self->spawnflags & TRAIN_SPLINE)
		{
			VectorClear(self->avelocity);
			VectorClear(self->velocity);
		//	if (self->movewith_next && (self->movewith_next->movewith_ent == self))
		//		set_child_movement(self);
		}

		if (self->moveinfo.wait > 0)
		{
			// Lazarus: turn off animation for stationary trains
			if (!strcmp(self->classname, "func_train"))
				self->s.effects &= ~(EF_ANIM_ALL | EF_ANIM_ALLFAST);
			self->nextthink = level.time + self->moveinfo.wait;
			self->think = train_next;
		}
		else if (self->spawnflags & TRAIN_TOGGLE)  // && wait < 0
		{
			train_next (self);
			self->spawnflags &= ~TRAIN_START_ON;
			VectorClear (self->velocity);
			// Knightmare added
			if (!self->spawnflags & TRAIN_ROTATE_CONSTANT)
				VectorClear (self->avelocity);
			// Lazarus: turn off animation for stationary trains
			if (!strcmp(self->classname, "func_train"))
				self->s.effects &= ~(EF_ANIM_ALL | EF_ANIM_ALLFAST);
			// end Knightmare
			self->nextthink = 0;
		}

		if (!(self->flags & FL_TEAMSLAVE))
		{
			if (self->moveinfo.sound_end)
				gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0);	// Knightmare- was ATTN_STATIC
			self->s.sound = 0;
		}
	}
	else
	{
		train_next (self);
	}
	
}

// Knightmare added
// Rroff's rotating train stuff, with quite a few changes

void train_yaw (edict_t *self);
void train_spline (edict_t *self);
void train_children_think (edict_t *self)
{
	if (!self || self->enemy) return;

	if (self->enemy->spawnflags & TRAIN_ROTATE)
	{
		// The the train was changed from TRAIN_ROTATE_CONSTANT to TRAIN_ROTATE
		// by a target_change... get da hell outta here.
		self->think = train_yaw;
		self->think(self);
		return;
	}

/*	if (self->enemy->movewith_next && (self->enemy->movewith_next->movewith_ent == self->enemy))
	{
		set_child_movement(self->enemy);
		self->nextthink = level.time + FRAMETIME;
	}
	else*/ if (level.time < 2)
		self->nextthink = level.time + FRAMETIME;
}


void train_yaw (edict_t *self)
{
	float	cur_yaw, idl_yaw, cur_pitch, idl_pitch, cur_roll, idl_roll;
	float	yaw_vel, pitch_vel, roll_vel;
	float	Dist_1, Dist_2, Distance;

	if (!self || !self->enemy || !self->enemy->inuse)
		return;

	if (self->enemy->spawnflags & TRAIN_ROTATE_CONSTANT)
	{
		// The the train was changed from TRAIN_ROTATE to TRAIN_ROTATE_CONSTANT
		// by a target_change... get da hell outta here.
		self->think = train_children_think;
		self->think (self);
		return;
	}

	cur_yaw   = self->enemy->s.angles[YAW];
	idl_yaw   = self->enemy->ideal_yaw;
	cur_pitch = self->enemy->s.angles[PITCH];
	idl_pitch = self->enemy->ideal_pitch;
	cur_roll  = self->enemy->s.angles[ROLL];
	idl_roll  = self->enemy->ideal_roll;

//	gi.dprintf("current angles=%g %g %g, ideal angles=%g %g %g\n",
//		cur_pitch,cur_yaw,cur_roll,idl_pitch,idl_yaw,idl_roll);

	yaw_vel   = self->enemy->yaw_speed;
	pitch_vel = self->enemy->pitch_speed;
	roll_vel  = self->enemy->roll_speed;

	if (cur_yaw == idl_yaw)
		self->enemy->avelocity[YAW] = 0;
	if (cur_pitch == idl_pitch)
		self->enemy->avelocity[PITCH] = 0;
	if (cur_roll == idl_roll)
		self->enemy->avelocity[ROLL] = 0;
	if ((cur_yaw == idl_yaw) && (cur_pitch == idl_pitch) && (cur_roll == idl_roll) )
	{
		self->nextthink = level.time + FRAMETIME;
//		if (self->enemy->movewith_next && (self->enemy->movewith_next->movewith_ent == self->enemy))
//			set_child_movement(self->enemy);
		return;
	} 

	if (cur_yaw != idl_yaw) {
		if (cur_yaw < idl_yaw)
		{
			Dist_1 = (idl_yaw - cur_yaw)*10;
			Dist_2 = ((360 - idl_yaw) + cur_yaw)*10;
			
			if (Dist_1 < Dist_2)
			{
				Distance = Dist_1;
				
				if (Distance < yaw_vel)
					yaw_vel = Distance;
				
				self->enemy->avelocity[YAW] = yaw_vel;
			}
			else
			{
				Distance = Dist_2;
				
				if (Distance < yaw_vel)
					yaw_vel = Distance;
				
				self->enemy->avelocity[YAW] = -yaw_vel;
			}
		}
		else
		{
			Dist_1 = (cur_yaw - idl_yaw)*10;
			Dist_2 = ((360 - cur_yaw) + idl_yaw)*10;
			
			if (Dist_1 < Dist_2)
			{
				Distance = Dist_1;
				
				if (Distance < yaw_vel)
					yaw_vel = Distance;
				
				self->enemy->avelocity[YAW] = -yaw_vel;
			}
			else
			{
				Distance = Dist_2;
				
				if (Distance < yaw_vel)
					yaw_vel = Distance;
				
				self->enemy->avelocity[YAW] = yaw_vel;
			}
		}
		
		//	gi.dprintf ("train cy: %g iy: %g ys: %g\n", cur_yaw, idl_yaw, self->enemy->avelocity[1]);
		
		if (self->enemy->s.angles[YAW] < 0)
			self->enemy->s.angles[YAW] += 360;
		
		if (self->enemy->s.angles[YAW] >= 360)
			self->enemy->s.angles[YAW] -= 360;
	}

	if (cur_pitch != idl_pitch) {

		if (cur_pitch < idl_pitch)
		{
			Dist_1 = (idl_pitch - cur_pitch)*10;
			Dist_2 = ((360 - idl_pitch) + cur_pitch)*10;
			
			if (Dist_1 < Dist_2)
			{
				Distance = Dist_1;
				
				if (Distance < pitch_vel)
					pitch_vel = Distance;
				
				self->enemy->avelocity[PITCH] = pitch_vel;
			}
			else
			{
				Distance = Dist_2;
				
				if (Distance < pitch_vel)
					pitch_vel = Distance;
				
				self->enemy->avelocity[PITCH] = -pitch_vel;
			}
		}
		else
		{
			Dist_1 = (cur_pitch - idl_pitch)*10;
			Dist_2 = ((360 - cur_pitch) + idl_pitch)*10;
			
			if (Dist_1 < Dist_2)
			{
				Distance = Dist_1;
				
				if (Distance < pitch_vel)
					pitch_vel = Distance;
				
				self->enemy->avelocity[PITCH] = -pitch_vel;
			}
			else
			{
				Distance = Dist_2;
				
				if (Distance < pitch_vel)
					pitch_vel = Distance;
				
				self->enemy->avelocity[PITCH] = pitch_vel;
			}
		}
		
		if (self->enemy->s.angles[PITCH] <  0)
			self->enemy->s.angles[PITCH] += 360;
		
		if (self->enemy->s.angles[PITCH] >= 360)
			self->enemy->s.angles[PITCH] -= 360;
	}

	if (cur_roll != idl_roll) {
		if (cur_roll < idl_roll)
		{
			Dist_1 = (idl_roll - cur_roll)*10;
			Dist_2 = ((360 - idl_roll) + cur_roll)*10;
			
			if (Dist_1 < Dist_2)
			{
				Distance = Dist_1;
				
				if (Distance < roll_vel)
					roll_vel = Distance;
				
				self->enemy->avelocity[ROLL] = roll_vel;
			}
			else
			{
				Distance = Dist_2;
				
				if (Distance < roll_vel)
					roll_vel = Distance;
				
				self->enemy->avelocity[ROLL] = -roll_vel;
			}
		}
		else
		{
			Dist_1 = (cur_roll - idl_roll)*10;
			Dist_2 = ((360 - cur_roll) + idl_roll)*10;
			
			if (Dist_1 < Dist_2)
			{
				Distance = Dist_1;
				
				if (Distance < roll_vel)
					roll_vel = Distance;
				
				self->enemy->avelocity[ROLL] = -roll_vel;
			}
			else
			{
				Distance = Dist_2;
				
				if (Distance < roll_vel)
					roll_vel = Distance;
				
				self->enemy->avelocity[ROLL] = roll_vel;
			}
		}
		
		if (self->enemy->s.angles[ROLL] < 0)
			self->enemy->s.angles[ROLL] += 360;
		
		if (self->enemy->s.angles[ROLL] >= 360)
			self->enemy->s.angles[ROLL] -= 360;
	}

//	if (self->enemy->movewith_next && (self->enemy->movewith_next->movewith_ent == self->enemy))
//		set_child_movement(self->enemy);
	self->nextthink = level.time + FRAMETIME;
}
// end Knightmare

void train_next (edict_t *self)
{
	edict_t		*ent;
	vec3_t		dest;
	qboolean	first;
	vec3_t		angles, v;	// Knightmare added

	// Knightmare- func_train_origin support
//	if (self->spawnflags & TRAIN_ORIGIN)
//		gi.dprintf ("train_next: pathing by train origin\n");

	first = true;
again:
	if (!self->target)
	{
	//	gi.dprintf ("train_next: no next target\n");
		self->s.sound = 0;	// Knightmare added
		return;
	}

	ent = G_PickTarget (self->target);
	if (!ent)
	{
		gi.dprintf ("train_next: bad target %s\n", self->target);
		return;
	}

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
		if (self->spawnflags & TRAIN_ORIGIN)	// Knightmare- func_train_origin support
			VectorCopy (ent->s.origin, self->s.origin);
		else
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
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}

	if (self->spawnflags & TRAIN_ORIGIN)	// Knightmare- func_train_origin support
		VectorCopy (ent->s.origin, dest);
	else
		VectorSubtract (ent->s.origin, self->mins, dest);
	self->moveinfo.state = STATE_TOP;
	VectorCopy (self->s.origin, self->moveinfo.start_origin);
	VectorCopy (dest, self->moveinfo.end_origin);

	// Knightmare added
	if (self->spawnflags & TRAIN_SPLINE)
	{
		float	speed;
		int		frames;

		self->from = self->to;
		self->to   = ent;
		self->moveinfo.ratio = 0.0;

		VectorSubtract(dest,self->s.origin,v);
		self->moveinfo.distance = VectorLength(v);
		frames = (int)(10 * self->moveinfo.distance/self->speed);
		if (frames < 1) frames = 1;
		speed = (10*self->moveinfo.distance)/(float)frames;
		self->moveinfo.speed = speed;
		self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;
	}

	// Rroff rotating
	if ( (self->spawnflags & TRAIN_ROTATE) && !(ent->spawnflags & 2))
	{
		// Lazarus: No no no :-). This is measuring from the center
		//          of the func_train to the path_corner. Should
		//          be path_corner to path_corner.
		//VectorSubtract (ent->s.origin, self->s.origin, v);
		if (self->spawnflags & TRAIN_ORIGIN) {	// Knightmare- func_train_origin support
			VectorSubtract(ent->s.origin, self->s.origin, v);
		}
		else {
			VectorAdd(self->s.origin,self->mins,v);
			VectorSubtract(ent->s.origin, v, v); // use adjusted_pathpoint
		}
		vectoangles2(v,angles);
		self->ideal_yaw = angles[YAW];
		self->ideal_pitch = angles[PITCH];
		if (self->ideal_pitch < 0) self->ideal_pitch += 360;
		self->ideal_roll = ent->roll;

		VectorClear(self->movedir);
		self->movedir[1] = 1.0;
	}
	/* Lazarus: We don't want to do this... this would give an
	//          instantaneous change in pitch and roll and look
	//          pretty goofy. Instead we'll set the new ideal_pitch
	//          to the path_corner's angles[PITCH], and move to that
	//          angle at pitch_speed. Roll changes? Can't do it (yet)
	if (ent->count)
		if (ent->count >= 0 && ent->count <= 360)
			self->s.angles[PITCH] = ent->count;

	if (ent->sounds)
		if (ent->sounds >= 0 && ent->sounds <= 360)
			self->s.angles[ROLL] = ent->sounds;
	*/
	// end Rroff

	// Lazarus:
	if (self->spawnflags & TRAIN_ROTATE_CONSTANT)
	{
		self->avelocity[PITCH] = self->pitch_speed;
		self->avelocity[YAW]   = self->yaw_speed;
		self->avelocity[ROLL]  = self->roll_speed;
	}
	// end Knightmare

	Move_Calc (self, dest, train_wait);
	self->spawnflags |= TRAIN_START_ON;
}

void train_resume (edict_t *self)
{
	edict_t	*ent;
	vec3_t	dest;

	ent = self->target_ent;

	if (self->spawnflags & TRAIN_ORIGIN)	// Knightmare- func_train_origin support
		VectorCopy (ent->s.origin, dest);
	else
		VectorSubtract (ent->s.origin, self->mins, dest);
	self->moveinfo.state = STATE_TOP;
	VectorCopy (self->s.origin, self->moveinfo.start_origin);
	VectorCopy (dest, self->moveinfo.end_origin);

	Move_Calc (self, dest, train_wait);
	self->spawnflags |= TRAIN_START_ON;

	// Knightmare- added from Lazarus
	if (self->spawnflags & TRAIN_ROTATE_CONSTANT)
	{
		self->avelocity[PITCH] = self->pitch_speed;
		self->avelocity[YAW]   = self->yaw_speed;
		self->avelocity[ROLL]  = self->roll_speed;
	}
	// end Knightmare
}

void func_train_find (edict_t *self)
{
	edict_t *ent;

	if (!self->target)
	{
		gi.dprintf ("train_find: no target\n");
		return;
	}
	ent = G_PickTarget (self->target);
	if (!ent)
	{
		gi.dprintf ("train_find: target %s not found\n", self->target);
		return;
	}

	// Knightmare added
	// Lazarus: trains can change speed at path_corners
	if (ent->speed) {
		self->speed = ent->speed;
		self->moveinfo.speed = self->speed;
		self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;
	}
	if (ent->pitch_speed)
		self->pitch_speed = ent->pitch_speed;
	if (ent->yaw_speed)
		self->yaw_speed   = ent->yaw_speed;
	if (ent->roll_speed)
		self->roll_speed  = ent->roll_speed;

	// Lazarus: spline stuff
	self->from = self->to = ent;
	// end spline stuff
	// end Knightmare

	self->target = ent->target;

	// Knightmare added
	if ( self->spawnflags & TRAIN_ROTATE )
		ent->think = train_yaw;
	else if (self->spawnflags & TRAIN_SPLINE )
		ent->think = train_spline;
	else
		ent->think = train_children_think;
	// end Knightmare

	ent->enemy = self;
	ent->nextthink = level.time + FRAMETIME;

	if (self->spawnflags & TRAIN_ORIGIN)	// Knightmare- func_train_origin support
		VectorCopy (ent->s.origin, self->s.origin);
	else
		VectorSubtract (ent->s.origin, self->mins, self->s.origin);

	VectorCopy (self->s.origin, self->s.old_origin);
	gi.linkentity (self);

	// if not triggered, start immediately
	if (!self->targetname)
		self->spawnflags |= TRAIN_START_ON;

	if (self->spawnflags & TRAIN_START_ON)
	{
		// Knightmare added
		// Lazarus: animated trains
		if (!strcmp(self->classname, "func_train"))
		{
			if (self->spawnflags & TRAIN_ANIMATE)
				self->s.effects |= EF_ANIM_ALL;
			else if (self->spawnflags & TRAIN_ANIMATE_FAST)
				self->s.effects |= EF_ANIM_ALLFAST;
		}
		// end Knightmare
		self->nextthink = level.time + FRAMETIME;
		self->think = train_next;
		self->activator = self;
	}
}

void train_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;

	if (self->spawnflags & TRAIN_START_ON)
	{
		if (!(self->spawnflags & TRAIN_TOGGLE))
			return;
		self->spawnflags &= ~TRAIN_START_ON;
		VectorClear (self->velocity);
		// Knightmare added
		VectorClear (self->avelocity);
		self->s.sound = 0;
		if (!strcmp(self->classname, "func_train"))
			self->s.effects &= ~(EF_ANIM_ALL | EF_ANIM_ALLFAST);
		// end Knightmare
		self->nextthink = 0;
	}
	else
	{
		// Knightmare added
		if (!strcmp(self->classname, "func_train"))
		{
			if (self->spawnflags & TRAIN_ANIMATE)
				self->s.effects |= EF_ANIM_ALL;
			else if (self->spawnflags & TRAIN_ANIMATE_FAST)
				self->s.effects |= EF_ANIM_ALLFAST;
		}
		if (self->spawnflags & TRAIN_SPLINE)
		{
			// Back up a step
			self->moveinfo.ratio -= self->moveinfo.speed * FRAMETIME / self->moveinfo.distance;
			if (self->moveinfo.ratio < 0.0f)
				self->moveinfo.ratio = 0.0f;
		}
		// end Knightmare

		if (self->target_ent)
			train_resume(self);
		else
			train_next(self);
	}
}

void SP_func_train (edict_t *self)
{
	gitem_t		*it;		//j
	edict_t		*it_ent;	//j

	self->movetype = MOVETYPE_PUSH;

	// Knightmare added
	if ( (self->spawnflags & (TRAIN_ROTATE | TRAIN_ROTATE_CONSTANT)) == (TRAIN_ROTATE | TRAIN_ROTATE_CONSTANT))
	{
		self->spawnflags &= ~(TRAIN_ROTATE | TRAIN_ROTATE_CONSTANT);
		self->spawnflags |= TRAIN_SPLINE;
	}
	// end Knightmare

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

	// Knightmare added
	if (self->attenuation <= 0)
		self->attenuation = ATTN_IDLE;
	// end Knightmare

	if (!self->speed)
		self->speed = 100;

	// Lazarus: Do NOT set default values for rotational speeds - if they're 0, then they're 0.

	self->moveinfo.speed = self->speed;
	self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;

	self->use = train_use;

	// Knightmare added
	// Lazarus: damageable
	if (self->health) {
		self->die = train_die;
		self->takedamage = DAMAGE_YES;
	}
	else {
		self->die = NULL;
		self->takedamage = DAMAGE_NO;
	}
	// end Knightmare

	gi.linkentity (self);

	if (self->target)
	{
		// start trains on the second frame, to make sure their targets have had
		// a chance to spawn
		self->nextthink = level.time + FRAMETIME;
		self->think = func_train_find;
	}
	else
	{
		gi.dprintf ("func_train without a target at %s\n", vtos(self->absmin));
	}

	// Lazarus: TRAIN_SMOOTH forces trains to go directly to Move_Done from
	//       Move_Final rather than slowing down (if necessary) for one
	//       frame.
	if (self->spawnflags & TRAIN_SMOOTH)
		self->smooth_movement = true;
	else
		self->smooth_movement = false;




///////////
	VectorAdd(self->s.origin, self->mins, self->monsterinfo.last_sighting);
//	VectorCopy(self->s.origin, self->monsterinfo.last_sighting);


	it = FindItem("Roam Navi");
	it_ent = G_Spawn();
	it_ent->classname = it->classname;
/*	VectorCopy (self->s.origin,it_ent->s.origin);
	it_ent->s.origin[0] = (self->moveinfo.start_origin[0] + self->mins[0] + self->moveinfo.start_origin[0] + self->maxs[0])/2;
	it_ent->s.origin[1] = (self->moveinfo.start_origin[1] + self->mins[1] + self->moveinfo.start_origin[1] + self->maxs[1])/2;
	it_ent->s.origin[2] = 0;
*/
	it_ent->union_ent = self;
	self->union_ent = it_ent;

	SpawnItem3 (it_ent, it);
}

void SP_func_train_origin (edict_t *self)
{
	self->spawnflags |= TRAIN_ORIGIN;
	self->classname = "func_train";
	SP_func_train (self);
}


/*QUAKED trigger_elevator (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
*/
void trigger_elevator_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *target;

	if (self->movetarget->nextthink)
	{
//		gi.dprintf("elevator busy\n");
		return;
	}

	if (!other->pathtarget)
	{
		gi.dprintf("elevator used with no pathtarget\n");
		return;
	}

	target = G_PickTarget (other->pathtarget);
	if (!target)
	{
		gi.dprintf("elevator used with bad pathtarget: %s\n", other->pathtarget);
		return;
	}

	self->movetarget->target_ent = target;
	train_resume (self->movetarget);
}

void trigger_elevator_init (edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("trigger_elevator has no target\n");
		return;
	}
	self->movetarget = G_PickTarget (self->target);
	if (!self->movetarget)
	{
		gi.dprintf("trigger_elevator unable to find target %s\n", self->target);
		return;
	}
	if (strcmp(self->movetarget->classname, "func_train") != 0)
	{
		gi.dprintf("trigger_elevator target %s is not a train\n", self->target);
		return;
	}

	self->use = trigger_elevator_use;
	self->svflags = SVF_NOCLIENT;

}

void SP_trigger_elevator (edict_t *self)
{
	self->think = trigger_elevator_init;
	self->nextthink = level.time + FRAMETIME;
}


/*QUAKED func_timer (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
"wait"			base time between triggering all targets, default is 1
"random"		wait variance, default is 0

so, the basic time between firing is a random time between
(wait - random) and (wait + random)

"delay"			delay before first firing when turned on, default is 0

"pausetime"		additional delay used only the very first time
				and only if spawned with START_ON

These can used but not touched.
*/
void func_timer_think (edict_t *self)
{
	G_UseTargets (self, self->activator);
	self->nextthink = level.time + self->wait + crandom() * self->random;
}

void func_timer_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;

	// if on, turn it off
	if (self->nextthink)
	{
		self->nextthink = 0;
		return;
	}

	// turn it on
	if (self->delay)
		self->nextthink = level.time + self->delay;
	else
		func_timer_think (self);
}

void SP_func_timer (edict_t *self)
{
	if (!self->wait)
		self->wait = 1.0;

	self->use = func_timer_use;
	self->think = func_timer_think;

	if (self->random >= self->wait)
	{
		self->random = self->wait - FRAMETIME;
		gi.dprintf("func_timer at %s has random >= wait\n", vtos(self->s.origin));
	}

	if (self->spawnflags & 1)
	{
		self->nextthink = level.time + 1.0 + st.pausetime + self->delay + self->wait + crandom() * self->random;
		self->activator = self;
	}

	self->svflags = SVF_NOCLIENT;
}


/*QUAKED func_conveyor (0 .5 .8) ? START_ON TOGGLE
Conveyors are stationary brushes that move what's on them.
The brush should be have a surface with at least one current content enabled.
speed	default 100
*/

void func_conveyor_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & 1)
	{
		self->speed = 0;
		self->spawnflags &= ~1;
	}
	else
	{
		self->speed = self->count;
		self->spawnflags |= 1;
	}

	if (!(self->spawnflags & 2))
		self->count = 0;
}

void SP_func_conveyor (edict_t *self)
{
	if (!self->speed)
		self->speed = 100;

	if (!(self->spawnflags & 1))
	{
		self->count = self->speed;
		self->speed = 0;
	}

	self->use = func_conveyor_use;

	gi.setmodel (self, self->model);
	self->solid = SOLID_BSP;
	gi.linkentity (self);
}


/*QUAKED func_door_secret (0 .5 .8) ? always_shoot 1st_left 1st_down
A secret door.  Slide back and then to the side.

open_once		doors never closes
1st_left		1st move is left of arrow
1st_down		1st move is down from arrow
always_shoot	door is shootebale even if targeted

"angle"		determines the direction
"dmg"		damage to inflic when blocked (default 2)
"wait"		how long to hold in the open position (default 5, -1 means hold)
*/

#define SECRET_ALWAYS_SHOOT	1
#define SECRET_1ST_LEFT		2
#define SECRET_1ST_DOWN		4

void door_secret_move1 (edict_t *self);
void door_secret_move2 (edict_t *self);
void door_secret_move3 (edict_t *self);
void door_secret_move4 (edict_t *self);
void door_secret_move5 (edict_t *self);
void door_secret_move6 (edict_t *self);
void door_secret_done (edict_t *self);

void door_secret_use (edict_t *self, edict_t *other, edict_t *activator)
{
	// make sure we're not already moving
//	if (!VectorCompare(self->s.origin, vec3_origin))
	if ((self->moveinfo.state != STATE_LOWEST) && (self->moveinfo.state != STATE_TOP))
		return;

	// Knightmare- added sound
	if (self->moveinfo.sound_start)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
	if (self->moveinfo.sound_middle)
	{
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}

//	Move_Calc (self, self->pos1, door_secret_move1);
//	door_use_areaportals (self, true);
	if (self->moveinfo.state == STATE_LOWEST)
	{
		self->moveinfo.state = STATE_DOWN;
		Move_Calc (self, self->pos1, door_secret_move1);
		door_use_areaportals (self, true);
	}
	else	// Knightmare added
	{
		self->moveinfo.state = STATE_UP;
		Move_Calc (self, self->pos1, door_secret_move5);
	}
}

void door_secret_move1 (edict_t *self)
{
	self->nextthink = level.time + 1.0;
	self->think = door_secret_move2;
	self->moveinfo.state = STATE_BOTTOM;

	// Knightmare- added sound
	self->s.sound = 0;
	if (self->moveinfo.sound_end)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC
}

void door_secret_move2 (edict_t *self)
{
	// Knightmare- added sound
	if (self->moveinfo.sound_start)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
	if (self->moveinfo.sound_middle) {
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}

	self->moveinfo.state = STATE_UP;
	Move_Calc (self, self->pos2, door_secret_move3);
}

void door_secret_move3 (edict_t *self)
{
	self->moveinfo.state = STATE_TOP;

	// Knightmare- added sound
	self->s.sound = 0;
	if (self->moveinfo.sound_end)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC

	if (self->wait == -1)
		return;

	self->nextthink = level.time + self->wait;
	self->think = door_secret_move4;
}

void door_secret_move4 (edict_t *self)
{
	// Knightmare- added sound
	if (self->moveinfo.sound_start)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
	if (self->moveinfo.sound_middle) {
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}
	self->moveinfo.state = STATE_UP;
	Move_Calc (self, self->pos1, door_secret_move5);
}

void door_secret_move5 (edict_t *self)
{
	self->nextthink = level.time + 1.0;
	self->think = door_secret_move6;
	self->moveinfo.state = STATE_BOTTOM;

	// Knightmare- added sound
	self->s.sound = 0;
	if (self->moveinfo.sound_end)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC
}

void door_secret_move6 (edict_t *self)
{
	// Knightmare- added sound
	if (self->moveinfo.sound_start)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
	if (self->moveinfo.sound_middle) {
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}

	self->moveinfo.state = STATE_DOWN;
	Move_Calc (self, self->pos0, door_secret_done);	// was vec3_origin
}

void door_secret_done (edict_t *self)
{
	if (!(self->targetname) || (self->spawnflags & SECRET_ALWAYS_SHOOT))
	{
		// Knightmare- restore user-set health here
		// now that the correct die function is set
	//	self->health = 0;
		self->health = self->max_health;
		self->takedamage = DAMAGE_YES;
	}
	self->moveinfo.state = STATE_LOWEST;	// Knightmare added

    // Knightmare- added sound
	self->s.sound = 0;
	if (self->moveinfo.sound_end)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC

	door_use_areaportals (self, false);
}

void door_secret_blocked  (edict_t *self, edict_t *other)
{
	if (!(other->svflags & SVF_MONSTER) && (!other->client) || !Q_stricmp(other->classname,"bodyque"))
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other)
			BecomeExplosion1 (other);
		return;
	}

	if (level.time < self->touch_debounce_time)
		return;
	self->touch_debounce_time = level.time + 0.5;

	if (other->deadflag)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
	else
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

//	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void door_secret_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	door_secret_use (self, attacker, attacker);
}

void SP_func_door_secret (edict_t *ent)
{
	vec3_t	forward, right, up;
	float	side;
	float	width;
	float	length;

	if (ent->sounds != 1)
	{
		ent->moveinfo.sound_start = gi.soundindex("doors/dr1_strt.wav");
		ent->moveinfo.sound_middle = gi.soundindex("doors/dr1_mid.wav");
		ent->moveinfo.sound_end = gi.soundindex("doors/dr1_end.wav");
	}
	else
	{
		ent->moveinfo.sound_start = 0;
		ent->moveinfo.sound_middle = 0;
		ent->moveinfo.sound_end = 0;
	}

	if (ent->attenuation <= 0)	// Knightmare added
		ent->attenuation = ATTN_STATIC;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);

	ent->blocked = door_secret_blocked;
	ent->use = door_secret_use;

	if (!(ent->targetname) || (ent->spawnflags & SECRET_ALWAYS_SHOOT))
	{
		// Knightmare- we can allow user-set health here
		// now that the correct die function is set
	//	ent->health = 0;
	//	if (!ent->health)
			ent->health = 1;
		ent->max_health = ent->health;
		ent->takedamage = DAMAGE_YES;
		ent->die = door_secret_die;
	}

	if (!ent->dmg)
		ent->dmg = 2;

	if (!ent->wait)
		ent->wait = 5;

	ent->moveinfo.accel =
	ent->moveinfo.decel =
	ent->moveinfo.speed = 50;
	ent->moveinfo.state = STATE_LOWEST;	// Knightmare added

	// calculate positions
	VectorCopy (ent->s.origin, ent->pos0);
	AngleVectors (ent->s.angles, forward, right, up);
	VectorClear (ent->s.angles);
	side = 1.0 - (ent->spawnflags & SECRET_1ST_LEFT);
	if (ent->spawnflags & SECRET_1ST_DOWN)
		width = fabs(DotProduct(up, ent->size));
	else
		width = fabs(DotProduct(right, ent->size));
	length = fabs(DotProduct(forward, ent->size));
	if (ent->spawnflags & SECRET_1ST_DOWN)
		VectorMA (ent->s.origin, -1 * width, up, ent->pos1);
	else
		VectorMA (ent->s.origin, side * width, right, ent->pos1);
	VectorMA (ent->pos1, length, forward, ent->pos2);

	if (ent->health)
	{
		ent->takedamage = DAMAGE_YES;
	//	ent->die = door_killed;
		ent->die = door_secret_die;	// Knightmare- this had the wrong die function set!
		ent->max_health = ent->health;
	}
	else if (ent->targetname && ent->message)
	{
		gi.soundindex ("misc/talk.wav");
		ent->touch = door_touch;
	}
	
	ent->classname = "func_door";
	VectorAdd(ent->s.origin, ent->mins, ent->monsterinfo.last_sighting);

	gi.linkentity (ent);
}


/*QUAKED func_killbox (1 0 0) ?
Kills everything inside when fired, irrespective of protection.
*/
void use_killbox (edict_t *self, edict_t *other, edict_t *activator)
{
	KillBox (self);
}

void SP_func_killbox (edict_t *ent)
{
	gi.setmodel (ent, ent->model);
	ent->use = use_killbox;
	ent->svflags = SVF_NOCLIENT;
}

/*QUAKED rotating_light (0 .5 .8) (-8 -8 -8) (8 8 8) START_OFF ALARM
"health"	if set, the light may be killed.
*/

// RAFAEL 
// note to self
// the lights will take damage from explosions
// this could leave a player in total darkness very bad
 
#define START_OFF	1

void rotating_light_alarm (edict_t *self)
{
	if (self->spawnflags & START_OFF)
	{
		self->think = NULL;
		self->nextthink = 0;	
	}
	else
	{
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0);
		self->nextthink = level.time + 1;
	}
}

void rotating_light_killed (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WELDING_SPARKS);
	gi.WriteByte (30);
	gi.WritePosition (self->s.origin);
	gi.WriteDir (vec3_origin);
	gi.WriteByte (0xe0 + (rand()&7));
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->s.effects &= ~EF_SPINNINGLIGHTS;
	self->use = NULL;

	self->think = G_FreeEdict;	
	self->nextthink = level.time + 0.1;
	
}

static void rotating_light_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & START_OFF)
	{
		self->spawnflags &= ~START_OFF;
		self->s.effects |= EF_SPINNINGLIGHTS;

		if (self->spawnflags & 2)
		{
			self->think = rotating_light_alarm;
			self->nextthink = level.time + 0.1;
		}
	}
	else
	{
		self->spawnflags |= START_OFF;
		self->s.effects &= ~EF_SPINNINGLIGHTS;
	}
}
	

void SP_rotating_light (edict_t *self)
{

	self->movetype = MOVETYPE_STOP;
	self->solid = SOLID_BBOX;
	
	self->s.modelindex = gi.modelindex ("models/objects/light/tris.md2");
	
	self->s.frame = 0;
		
	self->use = rotating_light_use;
	
	if (self->spawnflags & START_OFF)
		self->s.effects &= ~EF_SPINNINGLIGHTS;
	else
	{
		self->s.effects |= EF_SPINNINGLIGHTS;
	}

	if (!self->speed)
		self->speed = 32;
	// this is a real cheap way
	// to set the radius of the light
	// self->s.frame = self->speed;

	if (!self->health)
	{
		self->health = 10;
		self->max_health = self->health;
		self->die = rotating_light_killed;
		self->takedamage = DAMAGE_YES;
	}
	else
	{
		self->max_health = self->health;
		self->die = rotating_light_killed;
		self->takedamage = DAMAGE_YES;
	}
	
	if (self->spawnflags & 2)
	{
		self->moveinfo.sound_start = gi.soundindex ("misc/alarm.wav");	
	}
	
	gi.linkentity (self);

}


/*QUAKED func_object_repair (1 .5 0) (-8 -8 -8) (8 8 8) 
object to be repaired.
The default delay is 1 second
"delay" the delay in seconds for spark to occur
*/

void object_repair_fx (edict_t *ent)
{
 
 
	ent->nextthink = level.time + ent->delay;

	if (ent->health <= 100)
		ent->health++;
 	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_WELDING_SPARKS);
		gi.WriteByte (10);
		gi.WritePosition (ent->s.origin);
		gi.WriteDir (vec3_origin);
		gi.WriteByte (0xe0 + (rand()&7));
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	
}


void object_repair_dead (edict_t *ent)
{
	G_UseTargets (ent, ent);
	ent->nextthink = level.time + 0.1;
	ent->think = object_repair_fx;
}

void object_repair_sparks (edict_t *ent)
{
 
	if (ent->health < 0)
	{
		ent->nextthink = level.time + 0.1;
		ent->think = object_repair_dead;
		return;
	}

	ent->nextthink = level.time + ent->delay;
	
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WELDING_SPARKS);
	gi.WriteByte (10);
	gi.WritePosition (ent->s.origin);
	gi.WriteDir (vec3_origin);
	gi.WriteByte (0xe0 + (rand()&7));
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
}

void SP_object_repair (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->classname = "object_repair";
	VectorSet (ent->mins, -8, -8, 8);
	VectorSet (ent->maxs, 8, 8, 8);
	ent->think = object_repair_sparks;
	ent->nextthink = level.time + 1.0;
	ent->health = 100;
	if (!ent->delay)
		ent->delay = 1.0;
	
}


