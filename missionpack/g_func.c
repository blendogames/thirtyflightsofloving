#include "g_local.h"

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

void plat2_spawn_danger_area (edict_t *ent);
void plat2_kill_danger_area (edict_t *ent);
void Move_Done (edict_t *ent);
void train_children_think (edict_t *self);
void train_blocked (edict_t *self, edict_t *other);
void moving_speaker_think (edict_t *self);

//PGM
//====

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
// !easy					256
// !med						512
// !hard					1024
// !dm						2048
// !coop					4096
#define DOOR_INACTIVE		8192

#define TRAIN_START_ON		1
#define TRAIN_TOGGLE		2
#define TRAIN_BLOCK_STOPS	4
//Knightmare
#define	TRAIN_ROTATE		8
#define	TRAIN_ROT_CONST		16
#define	TRAIN_ANIM			32
#define	TRAIN_ANIM_FAST		64
#define TRAIN_SMOOTH		128
#define TRAIN_SPLINE        8192

//
// Support routines for movement (changes in origin using velocity)
//

//
//Knightmare- smart functions for child movement
// These constantly check the updated destination point
//

void Move_pos0_Final (edict_t *ent)
{
	if (!ent->movewith_ent)
		return;

	VectorSubtract (ent->pos0, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize (ent->moveinfo.dir);
	if (ent->moveinfo.remaining_distance == 0)
	{
		Move_Done (ent);
		return;
	}
	VectorScale (ent->moveinfo.dir, ent->moveinfo.remaining_distance / FRAMETIME, ent->velocity);
	VectorAdd(ent->movewith_ent->velocity, ent->velocity, ent->velocity);
	ent->think = Move_Done;
	ent->nextthink = level.time + FRAMETIME;
}

void Move_pos0_Think (edict_t *ent)
{
	if (!ent->movewith_ent)
		return;

	VectorSubtract (ent->pos0, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize (ent->moveinfo.dir);
	if ((ent->moveinfo.speed * FRAMETIME) >= ent->moveinfo.remaining_distance)
	{
		Move_pos0_Final (ent);
		return;
	}
	VectorScale (ent->moveinfo.dir, ent->moveinfo.speed, ent->velocity);
	VectorAdd(ent->movewith_ent->velocity, ent->velocity, ent->velocity);
	ent->nextthink = level.time + FRAMETIME;
	ent->think = Move_pos0_Think;
}

void Move_pos1_Final (edict_t *ent)
{
	if (!ent->movewith_ent)
		return;

	VectorSubtract (ent->pos1, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize (ent->moveinfo.dir);
	if (ent->moveinfo.remaining_distance == 0)
	{
		Move_Done (ent);
		return;
	}
	VectorScale (ent->moveinfo.dir, ent->moveinfo.remaining_distance / FRAMETIME, ent->velocity);
	VectorAdd(ent->movewith_ent->velocity, ent->velocity, ent->velocity);
	ent->think = Move_Done;
	ent->nextthink = level.time + FRAMETIME;
}

void Move_pos1_Think (edict_t *ent)
{
	if (!ent->movewith_ent)
		return;

	VectorSubtract (ent->pos1, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize (ent->moveinfo.dir);
	if ((ent->moveinfo.speed * FRAMETIME) >= ent->moveinfo.remaining_distance)
	{
		Move_pos1_Final (ent);
		return;
	}
	VectorScale (ent->moveinfo.dir, ent->moveinfo.speed, ent->velocity);
	VectorAdd(ent->movewith_ent->velocity, ent->velocity, ent->velocity);
	ent->nextthink = level.time + FRAMETIME;
	ent->think = Move_pos1_Think;
}

void Move_pos2_Final (edict_t *ent)
{
	if (!ent->movewith_ent)
		return;

	VectorSubtract (ent->pos2, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize (ent->moveinfo.dir);
	if (ent->moveinfo.remaining_distance == 0)
	{
		Move_Done (ent);
		return;
	}
	VectorScale (ent->moveinfo.dir, ent->moveinfo.remaining_distance / FRAMETIME, ent->velocity);
	VectorAdd(ent->movewith_ent->velocity, ent->velocity, ent->velocity);
	ent->think = Move_Done;
	ent->nextthink = level.time + FRAMETIME;
}

void Move_pos2_Think (edict_t *ent)
{
	if (!ent->movewith_ent)
		return;

	VectorSubtract (ent->pos2, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize (ent->moveinfo.dir);
	if ((ent->moveinfo.speed * FRAMETIME) >= ent->moveinfo.remaining_distance)
	{
		Move_pos2_Final (ent);
		return;
	}
	VectorScale (ent->moveinfo.dir, ent->moveinfo.speed, ent->velocity);
	VectorAdd(ent->movewith_ent->velocity, ent->velocity, ent->velocity);
	ent->nextthink = level.time + FRAMETIME;
	ent->think = Move_pos2_Think;
}

void Move_Done (edict_t *ent)
{
	VectorClear (ent->velocity);
	if (ent->movewith && ent->movewith_ent)
	{
		VectorCopy(ent->movewith_ent->velocity, ent->velocity);
	}
	// call end function
	if(ent->moveinfo.endfunc)
		ent->moveinfo.endfunc (ent);
}

void Move_Final (edict_t *ent)
{
	//if we're there, don't bother, we're done
	if (ent->moveinfo.remaining_distance == 0 || ent->smooth_movement)
	{
		Move_Done (ent);
		return;
	}
	//velocity = direction * distance * 10
	//so we always reach the goal in 0.1 seconds
	VectorScale (ent->moveinfo.dir, ent->moveinfo.remaining_distance / FRAMETIME, ent->velocity);
	if (ent->movewith && ent->movewith_ent)
	{
		VectorAdd(ent->movewith_ent->velocity, ent->velocity, ent->velocity);
	}
	ent->think = Move_Done;
	ent->nextthink = level.time + FRAMETIME;
}

void Move_Begin (edict_t *ent)
{
	float	frames;

	// if we're within one timestamp of reaching the goal, go to Move_Final
	if ((ent->moveinfo.speed * FRAMETIME) >= ent->moveinfo.remaining_distance)
	{
		Move_Final (ent);
		return;
	}
	//set object's velocity to direction times speed
	VectorScale (ent->moveinfo.dir, ent->moveinfo.speed, ent->velocity);
	if (ent->movewith && ent->movewith_ent)
	{
		VectorAdd(ent->movewith_ent->velocity, ent->velocity, ent->velocity);
		ent->moveinfo.remaining_distance -= ent->moveinfo.speed * FRAMETIME;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = Move_Begin;
	}
	else
	{
		// if func_train is moving toward a moving path_corner
		if (!strcmp(ent->classname, "func_train") && ent->target_ent->movewith)
		{
			vec3_t		dest;
			if (ent->spawnflags & TRAIN_ORIGIN)	// Knightmare- func_train_origin support
				VectorCopy (ent->target_ent->s.origin, dest);
			else
				VectorSubtract (ent->target_ent->s.origin, ent->mins, dest);
			VectorCopy (ent->s.origin, ent->moveinfo.start_origin);
			VectorCopy (dest, ent->moveinfo.end_origin);
			if (ent->spawnflags & TRAIN_ROTATE && !(ent->target_ent->spawnflags & 2))
			{
				vec3_t	v, angles;
				if (ent->spawnflags & TRAIN_ORIGIN){	// Knightmare- func_train_origin support
					VectorSubtract(ent->target_ent->s.origin, ent->s.origin, v);
				}
				else {
					VectorAdd(ent->s.origin, ent->mins, v);
					VectorSubtract(ent->target_ent->s.origin, v, v);
				}
				vectoangles2(v,angles);
				ent->ideal_yaw = angles[YAW];
				ent->ideal_pitch = angles[PITCH];
				if(ent->ideal_pitch < 0) ent->ideal_pitch += 360;
				VectorClear(ent->movedir);
				ent->movedir[1] = 1.0;
			}
			VectorSubtract (dest, ent->s.origin, ent->moveinfo.dir);
			ent->moveinfo.remaining_distance = VectorNormalize (ent->moveinfo.dir);
			VectorScale (ent->moveinfo.dir, ent->moveinfo.speed, ent->velocity);
			ent->nextthink = level.time + FRAMETIME;
			ent->think = Move_Begin;
		}
		else
		{
			frames = floor((ent->moveinfo.remaining_distance / ent->moveinfo.speed) / FRAMETIME);
			ent->moveinfo.remaining_distance -= frames * ent->moveinfo.speed * FRAMETIME;
			ent->nextthink = level.time + (frames * FRAMETIME);
			ent->think = Move_Final;
		}
	}
}

void Think_AccelMove (edict_t *ent);

void Move_Calc (edict_t *ent, vec3_t dest, void(*func)(edict_t*))
{
	VectorClear (ent->velocity);
	if (ent->movewith && ent->movewith_ent)
	{
		VectorCopy(ent->movewith_ent->velocity, ent->velocity);
	}
	VectorSubtract (dest, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize (ent->moveinfo.dir);
	ent->moveinfo.endfunc = func;
	if (ent->movewith && ent->movewith_ent)
	{	// Knightmare- use smart functions for child movement
		if (VectorCompare(dest, ent->pos0))
			Move_pos0_Think(ent);
		else if (VectorCompare(dest, ent->pos1))
			Move_pos1_Think(ent);
		else if (VectorCompare(dest, ent->pos2))
			Move_pos2_Think(ent);
		else
			Move_Begin (ent);
	}
	else if (ent->moveinfo.speed == ent->moveinfo.accel && ent->moveinfo.speed == ent->moveinfo.decel)
	{	// what does this mean?
		if (level.current_entity == ((ent->flags & FL_TEAMSLAVE) ? ent->teammaster : ent))
			Move_Begin (ent);
		else if (ent->movewith)
			Move_Begin (ent);
		else // wait 0.1 second to start moving
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = Move_Begin;
		}
	}
	else // accelerative
	{	// clear speed
		ent->moveinfo.current_speed = 0;
		ent->think = Think_AccelMove;
		ent->nextthink = level.time + FRAMETIME;
	}
}

//
// Knightmare- Support routines for bezier curve movement
//

void Spline_Calc (edict_t *train, vec3_t p1, vec3_t p2, vec3_t a1, vec3_t a2, float m, vec3_t p, vec3_t a)
{

	/* p1, p2  =  origins of path_* ents
	  a1, a2  =  angles from path_* ents
	  m       =  decimal position along curve */

	// Argh! - Bezier curve interpolation
	
	// the rest are computed locally
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
	// specific distance from the endpoints (path_*s), in the direction of
	// the endpoints' angle vectors (the 2nd control-point is offset in the
	// opposite direction).  The distance used is a fraction of the total
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
	if (train->roll_speed > 0)
		a[ROLL] = a1[ROLL] + m*(a2[ROLL] - a1[ROLL]);
}

// David Hyde's Spline support function
void train_wait (edict_t *self);
void Train_Move_Spline (edict_t *self)
{
	edict_t	*train;
	vec3_t	p;
	vec3_t	a;

	train = self->enemy;
	if(!train || !train->inuse)
		return;
	//Knightmare- check for killtargeted path_corners
	if ((!train->from) || (!train->from->inuse) || (!train->to) || (!train->to->inuse))
		return;
	//Knightmare- check for removed spline flag- get da hell outta here
	if ( !(train->spawnflags & TRAIN_SPLINE) )
	{
		self->think = train_children_think;
		return;
	}

	if ( (train->from != train->to) && !train->moveinfo.is_blocked && (train->spawnflags & TRAIN_START_ON))
	{
		if (train->moveinfo.bezier_ratio >= 1.0) //Knightmare- don't keep moving at end of curve
		{
			VectorClear(self->avelocity);
			VectorClear(self->velocity);
			self->nextthink = level.time + FRAMETIME;
			return;
		}
		Spline_Calc (train, train->from->s.origin, train->to->s.origin, 
		                    train->from->s.angles, train->to->s.angles,
		 				    train->moveinfo.bezier_ratio, p, a);
		if ( !(train->spawnflags & TRAIN_ORIGIN) ) // Knightmare- func_train_origin support
			VectorSubtract(p, train->mins, p);
		VectorSubtract(p, train->s.origin, train->velocity);
		VectorScale(train->velocity, 10, train->velocity);
		VectorSubtract(a, train->s.angles, train->avelocity);
		VectorScale(train->avelocity, 10, train->avelocity);
		if (train->pitch_speed < 0)
			train->avelocity[PITCH] = 0;
		if (train->yaw_speed < 0)
			train->avelocity[YAW] = 0;
		if (train->roll_speed < 0)
			train->avelocity[ROLL] = 0;
		gi.linkentity(train);
		train->moveinfo.bezier_ratio += train->moveinfo.speed * FRAMETIME / train->moveinfo.distance;
		train_move_children(self);
		if (train->moveinfo.bezier_ratio >= 1.0)
		{
			train->moveinfo.endfunc = NULL;
			train->think = train_wait;
			train->nextthink = level.time + FRAMETIME;
		}
	}
	self->nextthink = level.time + FRAMETIME;
}

void check_reverse_rotation (edict_t *self, vec3_t point)
{
	vec3_t	vec;
	vec3_t	vnext;
	vec_t	rotation;
	vec_t	cross;

	if (!(self->flags & FL_REVERSIBLE))
		return;

	VectorSubtract(point,self->s.origin,vec);
	VectorCopy(self->move_origin,vnext);
	VectorNormalize(vec);
	VectorNormalize(vnext);

	if (self->spawnflags & DOOR_X_AXIS)
	{
		rotation = self->moveinfo.distance * self->movedir[ROLL];
		cross = vec[1]*vnext[2] - vec[2]*vnext[1];
	}
	else if (self->spawnflags & DOOR_Y_AXIS)
	{
		rotation = self->moveinfo.distance * self->movedir[PITCH];
		cross = vec[2]*vnext[0] - vec[0]*vnext[2];
	}
	else
	{
		rotation = self->moveinfo.distance * self->movedir[YAW];
		cross = vec[0]*vnext[1] - vec[1]*vnext[0];
	}

	if ((self->spawnflags & 1) && (DotProduct(vec,vnext) < 0))
		cross = -cross;

	if ( (cross < 0) && (rotation > 0) )
	{
		VectorNegate (self->movedir, self->movedir);
		VectorMA (self->pos1, self->moveinfo.distance, self->movedir, self->pos2);
		VectorCopy (self->pos2, self->moveinfo.end_angles);
	}
	else if ((cross > 0) && (rotation < 0))
	{
		VectorNegate (self->movedir, self->movedir);
		VectorMA (self->pos1, self->moveinfo.distance, self->movedir, self->pos2);
		VectorCopy (self->pos2, self->moveinfo.end_angles);
	}
}

//
// Support routines for angular movement (changes in angle using avelocity)
//

void AngleMove_Done (edict_t *ent)
{
	//clear angular velocity
	VectorClear (ent->avelocity);
	//call end function
	if(ent->moveinfo.endfunc)
		ent->moveinfo.endfunc (ent);
}

void AngleMove_Final (edict_t *ent)
{
	vec3_t	move;

	//find out the move we need to make
	if (ent->moveinfo.state == STATE_UP)
		VectorSubtract (ent->moveinfo.end_angles, ent->s.angles, move);
	else
		VectorSubtract (ent->moveinfo.start_angles, ent->s.angles, move);
	//if we're there, we're done
	if (VectorCompare (move, vec3_origin))
	{
		AngleMove_Done (ent);
		return;
	}
	//set speed to move there in one timestamp
	VectorScale (move, 1.0/FRAMETIME, ent->avelocity);
	//think again in 0.1 sec
	ent->think = AngleMove_Done;
	ent->nextthink = level.time + FRAMETIME;
}

void AngleMove_Begin (edict_t *ent)
{
	vec3_t	destdelta;
	float	len;
	float	traveltime;
	float	frames;

//PGM		accelerate as needed
	if(ent->moveinfo.speed < ent->speed)
	{
		ent->moveinfo.speed += ent->accel;
		if(ent->moveinfo.speed > ent->speed)
			ent->moveinfo.speed = ent->speed;
	}
//PGM

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
//PGM
	// if we're done accelerating, act as a normal rotation
	if(ent->moveinfo.speed >= ent->speed)
	{
		// set nextthink to trigger a think when dest is reached
		ent->nextthink = level.time + frames * FRAMETIME;
		ent->think = AngleMove_Final;
	}
	else
	{
		ent->nextthink = level.time + FRAMETIME;
		ent->think = AngleMove_Begin;
	}
//PGM
}

void AngleMove_Calc (edict_t *ent, void(*func)(edict_t*))
{	
	//clear velocity
	VectorClear (ent->avelocity);
	//assign end function
	ent->moveinfo.endfunc = func;

//PGM
	// if we're supposed to accelerate, this will tell anglemove_begin to do so
	if(ent->accel != ent->speed)
		ent->moveinfo.speed = 0;
//PGM

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

	// PGM 04/21/98  - this should fix sthoms' sinking drop pod. Hopefully it wont break stuff.
//	if (ent->moveinfo.current_speed == 0)		// starting or blocked
		plat_CalcAcceleratedMove(&ent->moveinfo);

	plat_Accelerate (&ent->moveinfo);

	// will the entire move complete on next frame?
	if (ent->moveinfo.remaining_distance <= ent->moveinfo.current_speed)
	{
		Move_Final (ent);
		return;
	}

	VectorScale (ent->moveinfo.dir, ent->moveinfo.current_speed * 10, ent->velocity);
	if (ent->movewith && ent->movewith_ent)
		VectorAdd(ent->movewith_ent->velocity, ent->velocity, ent->velocity);
	ent->nextthink = level.time + FRAMETIME;
	ent->think = Think_AccelMove;
}


void plat_go_down (edict_t *ent);

void plat_hit_top (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ent->attenuation, 0); // was ATTN_STATIC
	//	ent->s.sound = 0;
	}
	ent->s.sound = 0;	// Knightmare- make sure this is always set to 0, lead mover or not!

	ent->moveinfo.state = STATE_TOP;

	ent->think = plat_go_down;
	ent->nextthink = level.time + 3;
}

void plat_hit_bottom (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ent->attenuation, 0); // was ATTN_STATIC
	//	ent->s.sound = 0;
	}
	ent->s.sound = 0;	// Knightmare- make sure this is always set to 0, lead mover or not!

	ent->moveinfo.state = STATE_BOTTOM;
	
	plat2_kill_danger_area (ent);		// PGM
}

void plat_go_down (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_start)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ent->attenuation, 0); // was ATTN_STATIC
		ent->s.sound = ent->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
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
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ent->attenuation, 0); // was ATTN_STATIC
		ent->s.sound = ent->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		ent->s.attenuation = ent->attenuation;
#endif
	}
	ent->moveinfo.state = STATE_UP;

	Move_Calc (ent, ent->moveinfo.start_origin, plat_hit_top);
	
	plat2_spawn_danger_area(ent);	// PGM
}

void plat_blocked (edict_t *self, edict_t *other)
{
	if (!(other->svflags & SVF_MONSTER) && (!other->client) )
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other && other->inuse)		// PGM
			BecomeExplosion1 (other);
		return;
	}

//PGM
	// gib dead things
	if(other->health < 1)
	{
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 100, 1, 0, MOD_CRUSH);
	}
//PGM

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

	if (self->moveinfo.state == STATE_UP)
		plat_go_down (self);
	else if (self->moveinfo.state == STATE_DOWN)
		plat_go_up (self);
}


void Use_Plat (edict_t *ent, edict_t *other, edict_t *activator)
{ 
//======
//ROGUE
	// if a monster is using us, then allow the activity when stopped.
	if (other->svflags & SVF_MONSTER)
	{
		if (ent->moveinfo.state == STATE_TOP)
			plat_go_down (ent);
		else if (ent->moveinfo.state == STATE_BOTTOM)
			plat_go_up (ent);

		return;
	}
//ROGUE
//======

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
	if (ent->movewith)
		trigger->movewith = ent->movewith;
	VectorCopy (trigger->s.angles, trigger->org_angles);
	
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
Plats are always drawn in the extended position, so they will light correctly.

If the plat is the target of another trigger or button, it will start out disabled in the extended position until it is trigger, when it will lower and become a normal plat.

"speed"	overrides default 200
"accel" overrides default 50
"decel" overrides default 50
"lip"	overrides default 8 unit lip

If the "height" key is set, that will determine the amount the plat moves, instead of being implicitly determoveinfoned by the model's height.

"sounds"
0,1		default
2-99	custom sound- e.g. plats/pt02_strt.wav, plats/pt02_mid.wav, plats/pt02_end.wav
*/
void SP_func_plat (edict_t *ent)
{
	ent->class_id = ENTITY_FUNC_PLAT;

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

	if (ent->lip)
		st.lip = ent->lip;
	if (!st.lip)
		st.lip = ent->lip = 8;

	// pos1 is the top position, pos2 is the bottom
	VectorCopy (ent->s.origin, ent->pos1);
	VectorCopy (ent->s.origin, ent->pos2);
	if (ent->height)
		st.height = ent->height;
	if (st.height)
		ent->pos2[2] -= st.height;
	else
		ent->pos2[2] -= (ent->maxs[2] - ent->mins[2]) - st.lip;

	ent->postthink = train_move_children; //supports movewith

	ent->use = Use_Plat;

	plat_spawn_inside_trigger (ent);	// the "start moving" trigger	

	if (ent->targetname)
	{
		ent->moveinfo.state = STATE_TOP; //was STATE_UP
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
	ent->moveinfo.distance = ent->pos1[2] - ent->pos2[2]; //Knightmare- store distance

	if (ent->sounds > 1 && ent->sounds < 100) // custom sounds
	{
		ent->moveinfo.sound_start = gi.soundindex  (va("plats/pt%02i_strt.wav", ent->sounds));
		ent->moveinfo.sound_middle = gi.soundindex  (va("plats/pt%02i_mid.wav", ent->sounds));
		ent->moveinfo.sound_end = gi.soundindex  (va("plats/pt%02i_end.wav", ent->sounds));
	}
	else
	{
		ent->moveinfo.sound_start = gi.soundindex ("plats/pt1_strt.wav");
		ent->moveinfo.sound_middle = gi.soundindex ("plats/pt1_mid.wav");
		ent->moveinfo.sound_end = gi.soundindex ("plats/pt1_end.wav");
	}

	if (ent->attenuation <= 0)
		ent->attenuation = ATTN_STATIC;
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

	SpawnBadArea(mins, maxs, 0, ent);
}

void plat2_kill_danger_area (edict_t *ent)
{
	edict_t *t;

	t = NULL;
	while ((t = G_Find (t, FOFS(classname), "bad_area")))
	{
		if(t->owner == ent)
			G_FreeEdict(t);
	}
}

void plat2_hit_top (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ent->attenuation, 0); // was ATTN_STATIC
		ent->s.sound = 0;
	}
	ent->moveinfo.state = STATE_TOP;

	if(ent->plat2flags & PLAT2_CALLED)
	{
		ent->plat2flags = PLAT2_WAITING;
		if(!(ent->spawnflags & PLAT2_TOGGLE))
		{
			ent->think = plat2_go_down;
			ent->nextthink = level.time + 5.0;
		}
		if(deathmatch->value)
			ent->last_move_time = level.time - 1.0;
		else
			ent->last_move_time = level.time - 2.0;
	}
	else if(!(ent->spawnflags & PLAT2_TOP) && !(ent->spawnflags & PLAT2_TOGGLE))
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
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ent->attenuation, 0); // was ATTN_STATIC
		ent->s.sound = 0;
	}
	ent->moveinfo.state = STATE_BOTTOM;
	
	if(ent->plat2flags & PLAT2_CALLED)
	{
		ent->plat2flags = PLAT2_WAITING;
		if(!(ent->spawnflags & PLAT2_TOGGLE))
		{
			ent->think = plat2_go_up;
			ent->nextthink = level.time + 5.0;
		}
		if(deathmatch->value)
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
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ent->attenuation, 0); // was ATTN_STATIC
		ent->s.sound = ent->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
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
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ent->attenuation, 0); // was ATTN_STATIC
		ent->s.sound = ent->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
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

	if(ent->moveinfo.state == STATE_TOP)
	{
		otherState = STATE_TOP;
		if(ent->spawnflags & PLAT2_BOX_LIFT)
		{
			if(platCenter > other->s.origin[2])
				otherState = STATE_BOTTOM;
		}
		else
		{
			if(trigger->absmax[2] > other->s.origin[2])
				otherState = STATE_BOTTOM;
		}
	}
	else
	{
		otherState = STATE_BOTTOM;
		if(other->s.origin[2] > platCenter)
			otherState = STATE_TOP;
	}

	ent->plat2flags = PLAT2_MOVING;

	if(deathmatch->value)
		pauseTime = 0.3;
	else
		pauseTime = 0.5;

	if(ent->moveinfo.state != otherState)
	{
		ent->plat2flags |= PLAT2_CALLED;
		pauseTime = 0.1;
	}

	ent->last_move_time = level.time;
	
	if(ent->moveinfo.state == STATE_BOTTOM)
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
	
	plat2_operate(ent, other);
}

void plat2_blocked (edict_t *self, edict_t *other)
{
	if (!(other->svflags & SVF_MONSTER) && (!other->client))
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if(other && other->inuse)
			BecomeExplosion1 (other);
		return;
	}

	// gib dead things
	if(other->health < 1)
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

	if(ent->moveinfo.state > STATE_BOTTOM)
		return;
	if((ent->last_move_time + 2) > level.time)
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

//	if(ent->targetname)
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

"speed"	overrides default 200
"accel" overrides default 50
"decel" overrides default 50
"lip"	no default

If the "height" key is set, that will determine the amount the plat moves, instead of being implicitly determoveinfoned by the model's height.

"sounds"
0,1		default
2-99	custom sound- e.g. plats/pt02_strt.wav, plats/pt02_mid.wav, plats/pt02_end.wav
*/
void SP_func_plat2 (edict_t *ent)
{
	edict_t *trigger;

	ent->class_id = ENTITY_FUNC_PLAT2;

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

	if (ent->lip)
		st.lip = ent->lip;
//	if (!st.lip)
//		st.lip = 8;

	// pos1 is the top position, pos2 is the bottom
	VectorCopy (ent->s.origin, ent->pos1);
	VectorCopy (ent->s.origin, ent->pos2);

	if (ent->height)
		st.height = ent->height;
	if (st.height)
		ent->pos2[2] -= (st.height - st.lip);
	else
		ent->pos2[2] -= (ent->maxs[2] - ent->mins[2]) - st.lip;

	ent->moveinfo.state = STATE_TOP;

	if(ent->targetname)
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

		if(!(ent->spawnflags & PLAT2_TOP))
		{
			VectorCopy (ent->pos2, ent->s.origin);
			ent->moveinfo.state = STATE_BOTTOM;
		}	
	}

	ent->postthink = train_move_children; //supports movewith

	gi.linkentity (ent);

	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	VectorCopy (ent->pos1, ent->moveinfo.start_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.start_angles);
	VectorCopy (ent->pos2, ent->moveinfo.end_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.end_angles);
	ent->moveinfo.distance = ent->pos1[2] - ent->pos2[2]; // Knightmare- store distance

	if (ent->sounds > 1 && ent->sounds < 100) // custom sounds
	{
		ent->moveinfo.sound_start = gi.soundindex  (va("plats/pt%02i_strt.wav", ent->sounds));
		ent->moveinfo.sound_middle = gi.soundindex  (va("plats/pt%02i_mid.wav", ent->sounds));
		ent->moveinfo.sound_end = gi.soundindex  (va("plats/pt%02i_end.wav", ent->sounds));
	}
	else
	{
		ent->moveinfo.sound_start = gi.soundindex ("plats/pt1_strt.wav");
		ent->moveinfo.sound_middle = gi.soundindex ("plats/pt1_mid.wav");
		ent->moveinfo.sound_end = gi.soundindex ("plats/pt1_end.wav");
	}

	if (ent->attenuation <= 0)
		ent->attenuation = ATTN_STATIC;
}


//====================================================================

/*QUAKED func_rotating (0 .5 .8) ? START_ON REVERSE X_AXIS Y_AXIS TOUCH_PAIN STOP ANIMATED ANIMATED_FAST !EASY !MED !HARD !DM !COOP ACCEL
You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

func_rotating will use it's targets when it stops and starts.

"speed" determines how fast it moves; default value is 100.
"dmg"	damage to inflict when blocked (2 default)
"accel" if specified, is how much the rotation speed will increase per .1sec.

REVERSE will cause the it to rotate in the opposite direction.
STOP mean it will stop moving instead of pushing entities
ACCEL means it will accelerate to it's final speed and decelerate when shutting down.
*/

//============
//PGM
void rotating_accel (edict_t *self)
{
	float	current_speed;

	current_speed = VectorLength (self->avelocity);
	if(current_speed >= (self->speed - self->accel))		// done
	{
		VectorScale (self->movedir, self->speed, self->avelocity);
		G_UseTargets (self, self);
	}
	else
	{
		current_speed += self->accel;
		VectorScale (self->movedir, current_speed, self->avelocity);
		self->think = rotating_accel;
		self->nextthink = level.time + FRAMETIME;
	}
}

void rotating_decel (edict_t *self)
{
	float	current_speed;

	current_speed = VectorLength (self->avelocity);
	if(current_speed <= self->decel)		// done
	{
		VectorClear (self->avelocity);
		G_UseTargets (self, self);
		self->touch = NULL;
	}
	else
	{
		current_speed -= self->decel;
		VectorScale (self->movedir, current_speed, self->avelocity);
		self->think = rotating_decel;
		self->nextthink = level.time + FRAMETIME;
	}
}
//PGM
//============


void rotating_blocked (edict_t *self, edict_t *other)
{
	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void rotating_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (self->avelocity[0] || self->avelocity[1] || self->avelocity[2])
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void rotating_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!VectorCompare (self->avelocity, vec3_origin))
	{
		self->s.sound = 0;
//PGM
		if(self->spawnflags & 8192)	// Decelerate
			rotating_decel (self);
		else
		{
			VectorClear (self->avelocity);
			G_UseTargets (self, self);
			self->touch = NULL;
		}
//PGM
	}
	else
	{
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
//PGM
		if(self->spawnflags & 8192)	// accelerate
			rotating_accel (self);
		else
		{
			VectorScale (self->movedir, self->speed, self->avelocity);
			G_UseTargets (self, self);
		}
		if (self->spawnflags & 16)
			self->touch = rotating_touch;
//PGM
	}
}

void SP_func_rotating (edict_t *ent)
{
	ent->class_id = ENTITY_FUNC_ROTATING;

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

	// Lazarus: Why was this removed? Dunno, but we're gonna use
	//          our own sound anyway
	//ent->moveinfo.sound_middle = "doors/hydro1.wav";
	if (st.noise)
		ent->moveinfo.sound_middle = gi.soundindex  (st.noise);

	if (ent->attenuation <= 0)
		ent->attenuation = ATTN_STATIC;

	ent->postthink = train_move_children; // supports movewith

	ent->use = rotating_use;
	if (ent->dmg)
		ent->blocked = rotating_blocked;

	if (ent->spawnflags & 1)
		ent->use (ent, NULL, NULL);

	if (ent->spawnflags & 64)
		ent->s.effects |= EF_ANIM_ALL;
	if (ent->spawnflags & 128)
		ent->s.effects |= EF_ANIM_ALLFAST;

//PGM
	if(ent->spawnflags & 8192)	// Accelerate / Decelerate
	{
		if(!ent->accel)
			ent->accel = 1;
		else if (ent->accel > ent->speed)
			ent->accel = ent->speed;

		if(!ent->decel)
			ent->decel = 1;
		else if (ent->decel > ent->speed)
			ent->decel = ent->speed;
	}
//PGM

	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);
}

/* Lazarus: SP_func_rotating_dh is identical to func_rotating, but allows you
            to build the model in a separate location (as with func_train, to
            get the lighting the way you'd like) then move at runtime to the
            origin of the "pathtarget". */

void func_rotating_dh_init (edict_t *ent)
{
	edict_t		*new_origin;

	new_origin = G_Find (NULL, FOFS(targetname), ent->pathtarget);
	if(new_origin)
		VectorCopy(new_origin->s.origin,ent->s.origin);
	SP_func_rotating (ent);
}

void SP_func_rotating_dh (edict_t *ent)
{
	if( !ent->pathtarget )
	{
		SP_func_rotating(ent);
		return;
	}

	// Wait a few frames so that we're sure pathtarget has been parsed.
	ent->think = func_rotating_dh_init;
	ent->nextthink = level.time + 2*FRAMETIME;
	gi.linkentity(ent);
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
0)	default
1)	silent
2-99 custom sound- e.g. switches/butn02.wav
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

	self->moveinfo.state = STATE_UP;
	if (self->moveinfo.sound_start && !(self->flags & FL_TEAMSLAVE))
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
	
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
	self->activator = attacker;
	self->health = self->max_health;
	self->takedamage = DAMAGE_NO;
	button_fire (self);
}

void SP_func_button (edict_t *ent)
{
	vec3_t	abs_movedir;
	float	dist;

	ent->class_id = ENTITY_FUNC_BUTTON;

	if (!VectorLength(ent->movedir))
		G_SetMovedir (ent->s.angles, ent->movedir);
	ent->movetype = MOVETYPE_STOP;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);

	if (ent->sounds > 1 && ent->sounds < 100) // custom sounds
		ent->moveinfo.sound_start = gi.soundindex  (va("switches/butn%02i.wav", ent->sounds));
	else if (ent->sounds != 1)
		ent->moveinfo.sound_start = gi.soundindex ("switches/butn2.wav");

	if (ent->attenuation <= 0)
		ent->attenuation = ATTN_STATIC;

	if (!ent->speed)
		ent->speed = 40;
	if (!ent->accel)
		ent->accel = ent->speed;
	if (!ent->decel)
		ent->decel = ent->speed;

	if (!ent->wait)
		ent->wait = 3;
	if (ent->lip)
		st.lip = ent->lip;
	if (!st.lip)
		st.lip = ent->lip = 4;

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

	ent->postthink = train_move_children; //supports movewith

	ent->moveinfo.state = STATE_BOTTOM;

	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	ent->moveinfo.distance = dist;
	VectorCopy (ent->pos1, ent->moveinfo.start_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.start_angles);
	VectorCopy (ent->pos2, ent->moveinfo.end_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.end_angles);

	gi.linkentity (ent);
}
//====================================================================
//
// SP_func_trainbutton is a button linked to a func_train with the
// "movewith" key. The button itself does NOT move relative to the 
// train, but the effect can be simulated with animations and sounds
//
// Spawnflags 1 = can be activated by looking at it and pressing +attack
//            2 = can NOT be activated by touching - must use +attack
//
//====================================================================
void trainbutton_done (edict_t *self)
{
	self->moveinfo.state = STATE_BOTTOM;
	self->s.effects &= ~EF_ANIM23;
	self->s.effects |= EF_ANIM01;
}

void trainbutton_return (edict_t *self)
{
	self->moveinfo.state = STATE_DOWN;
	self->s.frame = 0;
	self->s.effects &= ~EF_ANIM23;
	self->s.effects |= EF_ANIM01;
	if (self->health)
		self->takedamage = DAMAGE_YES;
}

void trainbutton_wait (edict_t *self)
{
	self->moveinfo.state = STATE_TOP;
	self->s.effects &= ~EF_ANIM01;
	self->s.effects |= EF_ANIM23;

	G_UseTargets (self, self->activator);
	self->s.frame = 1;
	if (self->moveinfo.wait >= 0)
	{
		self->nextthink = level.time + self->moveinfo.wait;
		self->think = trainbutton_return;
	}
}
void trainbutton_fire (edict_t *self)
{
	if (self->moveinfo.state == STATE_UP || self->moveinfo.state == STATE_TOP)
		return;
	self->moveinfo.state = STATE_UP;
	if (self->moveinfo.sound_start && !(self->flags & FL_TEAMSLAVE))
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
	trainbutton_wait(self);
}

void trainbutton_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;
	trainbutton_fire (self);
}

void trainbutton_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;

	if (other->health <= 0)
		return;

	self->activator = other;
	trainbutton_fire (self);
}

void trainbutton_killed (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->activator = attacker;
	self->health = self->max_health;
	self->takedamage = DAMAGE_NO;
	trainbutton_fire (self);
}

/*void movewith_init (edict_t *ent)
{
	edict_t *e, *child;

	// Unnamed entities can't be movewith parents
	if(!ent->targetname) return;

	child = G_Find(NULL,FOFS(movewith),ent->targetname);
	e = ent;
	while(child)
	{
		child->movewith_ent = ent;
		// Copy parent's current angles to the child. They SHOULD be 0,0,0 at this point
		// for all currently supported parents, but ya never know.
		VectorCopy(ent->s.angles,child->parent_attach_angles);
		if(child->org_movetype < 0)
			child->org_movetype = child->movetype;
		if(child->movetype != MOVETYPE_NONE)
			child->movetype = MOVETYPE_PUSH;
		VectorCopy(child->mins,child->org_mins);
		VectorCopy(child->maxs,child->org_maxs);
		VectorSubtract(child->s.origin,ent->s.origin,child->movewith_offset);
		e->movewith_next = child;
		e = child;
		child = G_Find(child,FOFS(movewith),ent->targetname);
	}
}*/

void SP_func_trainbutton (edict_t *ent)
{
	if(!ent->movewith)
	{
		SP_func_button(ent);
		return;
	}
	ent->class_id = ENTITY_FUNC_TRAINBUTTON;

	G_SetMovedir (ent->s.angles, ent->movedir);
	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);

	if (ent->sounds > 1 && ent->sounds < 100) // custom sounds
		ent->moveinfo.sound_start = gi.soundindex  (va("switches/butn%02i.wav", ent->sounds));
	else if (ent->sounds != 1)
		ent->moveinfo.sound_start = gi.soundindex ("switches/butn2.wav");

	if (ent->attenuation <= 0)
		ent->attenuation = ATTN_STATIC;

	if (!ent->wait)
		ent->wait = 3;
	ent->use = trainbutton_use;
	ent->s.effects |= EF_ANIM01;
	ent->blocked = train_blocked;
	ent->moveinfo.state = STATE_BOTTOM;
	ent->moveinfo.wait = ent->wait;
	if (!ent->targetname && !(ent->spawnflags & 2))
		ent->touch = trainbutton_touch;
	gi.linkentity (ent);
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
0)		default
1)		silent
2-4		not used
5-99	custom sound- e.g. doors/dr05_strt.wav, doors/dr05_mid.wav, doors/dr05_end.wav
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

//Lazarus
void swinging_door_reset (edict_t *self)
{
	self->moveinfo.state = STATE_BOTTOM;
	self->health = self->max_health;
	if(self->die)
		self->takedamage = DAMAGE_YES;
	VectorCopy (self->pos2, self->pos1);
	VectorMA (self->s.angles, self->moveinfo.distance, self->movedir, self->pos2);
	VectorCopy (self->pos1, self->moveinfo.start_angles);
	VectorCopy (self->pos2, self->moveinfo.end_angles);
	vectoangles2(self->move_origin,self->move_origin);
	VectorMA(self->move_origin,self->moveinfo.distance,self->movedir,self->move_origin);
	AngleVectors(self->move_origin,self->move_origin,NULL,NULL);
}

void door_hit_top (edict_t *self)
{
	if ( (strcmp(self->classname, "func_door_rotating") == 0)
		|| (strcmp(self->classname, "func_door_rot_dh") == 0) )
		self->do_not_rotate = 0;

	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_end)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC
	//	self->s.sound = 0;
	}
	self->s.sound = 0;	// Knightmare- make sure this is always set to 0, lead mover or not!

	self->moveinfo.state = STATE_TOP;
	if(self->flags & FL_REVOLVING) //Lazarus
	{
		self->think = swinging_door_reset;
		if(self->moveinfo.wait > 0)
			self->nextthink = level.time + self->moveinfo.wait;
		else
			self->think(self);
		return;
	}
	if (self->spawnflags & DOOR_TOGGLE)
		return;
	if (self->moveinfo.wait >= 0)
	{
		self->think = door_go_down;
		self->nextthink = level.time + self->moveinfo.wait;
	}
}

void door_hit_bottom (edict_t *self)
{
	if ( (strcmp(self->classname, "func_door_rotating") == 0)
		|| (strcmp(self->classname, "func_door_rot_dh") == 0) )
		self->do_not_rotate = 0;

	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_end)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC
	//	self->s.sound = 0;
	}
	self->s.sound = 0;	// Knightmare- make sure this is always set to 0, lead mover or not!

	self->moveinfo.state = STATE_BOTTOM;
	door_use_areaportals (self, false);
}

void door_go_down (edict_t *self)
{
	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_start)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
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
	{
		self->do_not_rotate = 1;
		self->moveinfo.state = STATE_DOWN;
		AngleMove_Calc (self, door_hit_bottom);
	}
	else if (strcmp(self->classname, "func_door_rot_dh") == 0)
	{
		self->do_not_rotate = 1;
		self->moveinfo.state = STATE_DOWN;
		AngleMove_Calc (self, door_hit_bottom);
	}
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
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}
	self->moveinfo.state = STATE_UP;
	
	if (strcmp(self->classname, "func_door") == 0)
		Move_Calc (self, self->moveinfo.end_origin, door_hit_top);
	else if (strcmp(self->classname, "func_door_rotating") == 0)
	{
		self->do_not_rotate = 1;
		self->moveinfo.state = STATE_UP;
		AngleMove_Calc (self, door_hit_top);
	}
	else if (strcmp(self->classname, "func_door_rot_dh") == 0)
	{
		self->do_not_rotate = 1;
		self->moveinfo.state = STATE_UP;
		AngleMove_Calc (self, door_hit_top);
	}
	G_UseTargets (self, activator);
	door_use_areaportals (self, true);
}

//======
//PGM
void smart_water_go_up (edict_t *self)
{
	float		distance;
	edict_t		*lowestPlayer;
	edict_t		*ent;
	float		lowestPlayerPt;
	int			i;

	if (self->moveinfo.state == STATE_TOP)
	{	// reset top wait time
		if (self->moveinfo.wait >= 0)
			self->nextthink = level.time + self->moveinfo.wait;
		return;
	}

	if (self->health)
	{
		if(self->absmax[2] >= self->health)
		{
			VectorClear (self->velocity);
			self->nextthink = 0;
			self->moveinfo.state = STATE_TOP;
			return;
		}
	}

	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_start)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}

	// find the lowest player point.
	lowestPlayerPt = 999999;
	lowestPlayer = NULL;
	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];

		// don't count dead or unused player slots
		if((ent->inuse) && (ent->health > 0))
		{
			if (ent->absmin[2] < lowestPlayerPt)
			{
				lowestPlayerPt = ent->absmin[2];
				lowestPlayer = ent;
			}
		}
	}

	if(!lowestPlayer)
	{
		return;
	}

	distance = lowestPlayerPt - self->absmax[2];

	// for the calculations, make sure we intend to go up at least a little.
	if(distance < self->accel)
	{
		distance = 100;
		self->moveinfo.speed = 5;
	}
	else
		self->moveinfo.speed = distance / self->accel;

	if(self->moveinfo.speed < 5)
		self->moveinfo.speed = 5;
	else if(self->moveinfo.speed > self->speed)
		self->moveinfo.speed = self->speed;

	// FIXME - should this allow any movement other than straight up?
	VectorSet(self->moveinfo.dir, 0, 0, 1);	
	VectorScale (self->moveinfo.dir, self->moveinfo.speed, self->velocity);
	self->moveinfo.remaining_distance = distance;

	if(self->moveinfo.state != STATE_UP)
	{
		G_UseTargets (self, lowestPlayer);
		door_use_areaportals (self, true);
		self->moveinfo.state = STATE_UP;
	}

	self->think = smart_water_go_up;
	self->nextthink = level.time + FRAMETIME;
}
//PGM
//======

void door_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*ent;
	vec3_t	center;			//PGM

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

//PGM
	// smart water is different
	VectorAdd(self->mins, self->maxs, center);
	VectorScale(center, 0.5, center);
	if ((gi.pointcontents (center) & MASK_WATER) && self->spawnflags & 2)
	{
		self->message = NULL;
		self->touch = NULL;
		self->enemy = activator;
		smart_water_go_up (self);
		return;
	}
//PGM

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

	if ((self->owner->spawnflags & DOOR_NOMONSTER) && (other->svflags & SVF_MONSTER))
		return;

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
	if (ent->movewith)
		other->movewith = ent->movewith;
	VectorCopy (other->s.angles, other->org_angles);
	gi.linkentity (other);

	if (ent->spawnflags & DOOR_START_OPEN)
		door_use_areaportals (ent, true);

	Think_CalcMoveSpeed (ent);
}

void door_blocked  (edict_t *self, edict_t *other)
{
	edict_t	*ent;

	if (!(other->svflags & SVF_MONSTER) && (!other->client) )
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other && other->inuse)
			BecomeExplosion1 (other);
		return;
	}

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

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

void func_explosive_explode (edict_t *self);
void door_destroyed (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->moveinfo.state == STATE_DOWN || self->moveinfo.state == STATE_BOTTOM)
		door_use_areaportals(self,true);
	self->dmg = 0;
	func_explosive_explode(self);
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

	gi.centerprintf (other, "%s", self->message);
	gi.sound (other, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
}

void SP_func_door (edict_t *ent)
{
	vec3_t	abs_movedir;

	ent->class_id = ENTITY_FUNC_DOOR;

	if (ent->sounds > 4 && ent->sounds < 100) // custom sounds
	{
		ent->moveinfo.sound_start = gi.soundindex  (va("doors/dr%02i_strt.wav", ent->sounds));
		ent->moveinfo.sound_middle = gi.soundindex  (va("doors/dr%02i_mid.wav", ent->sounds));
		ent->moveinfo.sound_end = gi.soundindex  (va("doors/dr%02i_end.wav", ent->sounds));
	}
	else if (ent->sounds != 1)
	{
		ent->moveinfo.sound_start = gi.soundindex  ("doors/dr1_strt.wav");
		ent->moveinfo.sound_middle = gi.soundindex  ("doors/dr1_mid.wav");
		ent->moveinfo.sound_end = gi.soundindex  ("doors/dr1_end.wav");
	}
	else
	{
		ent->moveinfo.sound_start = 0;
		ent->moveinfo.sound_middle = 0;
		ent->moveinfo.sound_end = 0;
	}

	if (ent->attenuation <= 0)
		ent->attenuation = ATTN_STATIC;

	if (!VectorLength(ent->movedir))
		G_SetMovedir (ent->s.angles, ent->movedir);
	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);

	ent->blocked = door_blocked;
	ent->use = door_use;
	
	// Lazarus: make noise field work w/o origin brush 
	// create a speaker entity at center of bmodel
	// Removed because this breaks when the center point of
	// the door moves inside the wall (can't be heard).
	/*if (ent->moveinfo.sound_middle)// && ent->sounds > 4)
	{
		edict_t *speaker;

		ent->noise_index    = ent->moveinfo.sound_middle;
		ent->moveinfo.sound_middle = 0;
		speaker = G_Spawn();
		speaker->classname	= "moving_speaker";
		speaker->s.sound	= 0;
		speaker->volume		= 1;
		speaker->attenuation = 1;
		speaker->owner		= ent;
		speaker->think		= moving_speaker_think;
		speaker->nextthink	= level.time + 2*FRAMETIME;
		speaker->spawnflags	= 7; // owner must be moving to play
		ent->speaker		= speaker;
		if (VectorLength(ent->s.origin))
			VectorCopy(ent->s.origin,speaker->s.origin);
		else
		{
			VectorAdd(ent->absmin,ent->absmax,speaker->s.origin);
			VectorScale(speaker->s.origin,0.5,speaker->s.origin);
		}
		VectorSubtract (speaker->s.origin, ent->s.origin, speaker->offset);
	}*/
	// end Lazarus

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
	if (ent->lip)
		st.lip = ent->lip;
	if (!st.lip)
		st.lip = ent->lip = 8;
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

		// Lazarus: negative health means "damage kills me" rather than "damage opens me"
		if (ent->health < 0)
		{
			PrecacheDebris(ent->gib_type);
			ent->die = door_destroyed;
			ent->health = -ent->health;
		}
		else
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

	ent->postthink = train_move_children; //Knightmare- now supports movewith
	ent->nextthink = level.time + FRAMETIME;
	if (ent->health || ent->targetname)
		ent->think = Think_CalcMoveSpeed;
	else
		ent->think = Think_SpawnDoorTrigger;
}

//PGM
void Door_Activate (edict_t *self, edict_t *other, edict_t *activator)
{
	self->use = NULL;
	
	if (self->health)
	{
		self->takedamage = DAMAGE_YES;
		self->die = door_killed;
		self->max_health = self->health;
	}

	if (self->health)
		self->think = Think_CalcMoveSpeed;
	else
		self->think = Think_SpawnDoorTrigger;
	self->nextthink = level.time + FRAMETIME;

}
//PGM

/*QUAKED func_door_rotating (0 .5 .8) ? START_OPEN REVERSE CRUSHER NOMONSTER ANIMATED TOGGLE X_AXIS Y_AXIS EASY MED HARD DM COOP INACTIVE
TOGGLE causes the door to wait in both the start and end states for a trigger event.

START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER	monsters will not trigger this door

You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"distance" is how many degrees the door will be rotated.
"speed" determines how fast the door moves; default value is 100.
"accel" if specified,is how much the rotation speed will increase each .1 sec. (default: no accel)

REVERSE will cause the door to rotate in the opposite direction.
INACTIVE will cause the door to be inactive until triggered.

"message"	is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"	if set, door must be shot open
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"dmg"		damage to inflict when blocked (2 default)
"sounds"
0)		default
1)		silent
2-4		not used
5-99	custom sound- e.g. doors/dr05_strt.wav, doors/dr05_mid.wav, doors/dr05_end.wav
*/

void SP_func_door_rotating (edict_t *ent)
{
	ent->class_id = ENTITY_FUNC_DOOR_ROTATING;

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

	if (ent->distance)
		st.distance = ent->distance;
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

	if (ent->sounds > 4 && ent->sounds < 100) // custom sounds
	{
		ent->moveinfo.sound_start = gi.soundindex  (va("doors/dr%02i_strt.wav", ent->sounds));
		ent->moveinfo.sound_middle = gi.soundindex  (va("doors/dr%02i_mid.wav", ent->sounds));
		ent->moveinfo.sound_end = gi.soundindex  (va("doors/dr%02i_end.wav", ent->sounds));
	}
	else if (ent->sounds != 1)
	{
		ent->moveinfo.sound_start = gi.soundindex  ("doors/dr1_strt.wav");
		ent->moveinfo.sound_middle = gi.soundindex  ("doors/dr1_mid.wav");
		ent->moveinfo.sound_end = gi.soundindex  ("doors/dr1_end.wav");
	}
	else
	{
		ent->moveinfo.sound_start = 0;
		ent->moveinfo.sound_middle = 0;
		ent->moveinfo.sound_end = 0;
	}

	if (ent->attenuation <= 0)
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

	ent->do_not_rotate = 0;
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

	ent->postthink = train_move_children; //supports movewith

	gi.linkentity (ent);

	ent->nextthink = level.time + FRAMETIME;
	if (ent->health || ent->targetname)
		ent->think = Think_CalcMoveSpeed;
	else
		ent->think = Think_SpawnDoorTrigger;

//PGM
	if (ent->spawnflags & DOOR_INACTIVE)
	{
		ent->takedamage = DAMAGE_NO;
		ent->die = NULL;
		ent->think = NULL;
		ent->nextthink = 0;
		ent->use = Door_Activate;
	}
//PGM
}

/* Lazarus: SP_func_door_rot_dh is identical to func_door_rotating, but allows you
            to build the model in a separate location (as with func_train, to
            get the lighting the way you'd like) then move at runtime to the
            origin of the "pathtarget". */

void func_door_rot_dh_init (edict_t *ent)
{
	edict_t		*new_origin;

	new_origin = G_Find (NULL, FOFS(targetname), ent->pathtarget);
	if(new_origin) {
		VectorCopy(new_origin->s.origin,ent->s.origin);
		VectorCopy (ent->s.origin, ent->moveinfo.start_origin);
		VectorCopy (ent->s.origin, ent->moveinfo.end_origin);
		gi.linkentity(ent);
	}
	ent->nextthink = level.time + FRAMETIME;
	if (ent->health || ent->targetname)
		ent->think = Think_CalcMoveSpeed;
	else
		ent->think = Think_SpawnDoorTrigger;
}

void SP_func_door_rot_dh (edict_t *ent)
{
	SP_func_door_rotating(ent);
	if(!ent->pathtarget) return;

	// Wait a few frames so that we're sure pathtarget has been parsed.
	ent->think = func_door_rot_dh_init;
	ent->nextthink = level.time + 2*FRAMETIME;
	gi.linkentity(ent);
}

void smart_water_blocked (edict_t *self, edict_t *other)
{
	if (!(other->svflags & SVF_MONSTER) && (!other->client) )
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_LAVA);
		// if it's still there, nuke it
		if (other && other->inuse)		// PGM
			BecomeExplosion1 (other);
		return;
	}

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100, 1, 0, MOD_LAVA);
}

/*QUAKED func_water (0 .5 .8) ? START_OPEN SMART MUD
func_water is a moveable water brush.  It must be targeted to operate.  Use a non-water texture at your own risk.

START_OPEN causes the water to move to its destination when spawned and operate in reverse.
SMART causes the water to adjust its speed depending on distance to player. 
MUD        turns the water to mud - difficult to move through, swallows players

(speed = distance/accel, min 5, max self->speed)
"accel"		for smart water, the divisor to determine water speed. default 20 (smaller = faster)

"health"	maximum height of this water brush
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

	self->class_id = ENTITY_FUNC_WATER;

	if (!VectorLength(self->movedir))
  		G_SetMovedir (self->s.angles, self->movedir);
	self->movetype = MOVETYPE_PUSH;
	self->solid = SOLID_BSP;
	gi.setmodel (self, self->model);

	if(self->spawnflags & 4)
	{
		level.mud_puddles++;
		self->svflags |= SVF_MUD;
	}

	if (!self->lip)
		self->lip = 0;
	st.lip = self->lip;

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

	if ( self->spawnflags & 2)	// smart water
	{
		// this is actually the divisor of the lowest player's distance to determine speed.
		// self->speed then becomes the cap of the speed.
		if(!self->accel)
			self->accel = 20;
		self->blocked = smart_water_blocked;
	}

	if (!self->wait)
		self->wait = -1;
	self->moveinfo.wait = self->wait;

	self->use = door_use;

	if (self->wait == -1)
		self->spawnflags |= DOOR_TOGGLE;

	self->classname = "func_door";

	gi.linkentity (self);
}


/*QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS ROTATE ROT_CONST ANIM ANIM_FAST SMOOTH
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
	
speed		default 100
dmg			default	2, needed if you want it to explode when destroyed
health		If this is set, the train will be destroyable
noise		looping sound to play when the train is in motion
accel		[1-100] specify a percentage to speed up every frame, i.e. 50 means increase speed by 50% every frame. 
decel		Works exactly like accel. 
pitch_speed		(Nose up & Down) in degrees per second (defualt 20)
yaw_speed		(Side-to-side "wiggle") in degrees per second (default 20)
roll_speed		(Banking) in degrees per second toward the next path corner's set roll
turn_rider		rotates player. 0=don't turn; 1=player turns with model

The two below values are needed if the train is rotating and enclosed
bleft			Lower coordinate bounding a safe area in the train, default (-16, -16, -16) offset from train origin
tright			Upper coordinate bounding a safe area in the train, default (16, 16, 16) offset from train origin

If the path_corner's wait = -1, then the train will stop and not restart unless the train is retriggered.
To have other entities move with the train, set all the piece's team value to the same thing. They will move in unison.

Better yet, you can set the train pieces' movewith values to the same as the train's targetname and they will move in
unison, and also still be able to move relative to the train themselves, and can be attached and detached using target_movewith.
NOTE: All the pieces must be created after the train entity, otherwise they will move ahead of it.
*/

void prox_movewith_host (edict_t *self);

void train_move_prox (edict_t *self)
{
	edict_t	*ent = NULL;
	int i;
	if (!self->classname)
		return;
	ent = g_edicts+1; // skip the worldspawn
	for (i = 1; i < globals.num_edicts; i++, ent++)
	{
		if (!ent->classname)
			continue;
		if (!strcmp(ent->classname,"prox") && (ent->movewith_ent == self))
			prox_movewith_host (ent);
	}
}

//Knightmare- this function moves the movewith chilren
//Most of this code is by David Hyde, he got tired of just helping me piecemeal...
void turret_stand (edict_t *self);
void train_move_children (edict_t *self)
{
	edict_t	*ent = NULL;
	int		i;
	vec3_t	amove, angles;
	vec3_t	forward, right, up;
	vec3_t	parent_angle_change, offset;
	qboolean    is_monster;

	if (!self->classname)
		return;
	if (!self->targetname)
		return;

	ent = g_edicts+1; // skip the worldspawn
	for (i = 1; i < globals.num_edicts; i++, ent++)
	{
		if (!ent->classname)
			continue;
		if (!ent->movewith)
			continue;
		if(!ent->inuse)
			return;
		if (!strcmp(ent->movewith, self->targetname))
		{
			ent->movewith_ent = self;
			//backup and set movetype to push
			if (ent->movetype && (ent->movetype != MOVETYPE_PUSH)) //backup only non-push movetypes
				ent->oldmovetype = ent->movetype;
			ent->movetype = MOVETYPE_PUSH;

			//set rotational offset
			if (!ent->movewith_set) 
			{
				VectorCopy(ent->mins, ent->org_mins);
				VectorCopy(ent->maxs, ent->org_maxs);
				VectorSubtract (ent->s.origin, self->s.origin, ent->movewith_offset);
				//Remeber child's and parent's angles when child was attached
				VectorCopy(self->s.angles, ent->parent_attach_angles);
				VectorCopy(ent->s.angles, ent->child_attach_angles);

				// Knightmare- backup turret's old angle offset
				if (!strcmp(ent->classname,"monster_turret") && ent->offset)
					VectorCopy(ent->offset, ent->old_offset); 

				ent->movewith_set = 1;
			}
			//Get change in parent's angles from when child was attached, this tells us how far we need to rotate
			VectorSubtract(self->s.angles, ent->parent_attach_angles, parent_angle_change);
			AngleVectors(parent_angle_change, forward, right, up);
			VectorNegate(right, right);

			//if(!strncmp(ent->classname,"monster_",8))
			if(ent->svflags & SVF_MONSTER)
				is_monster = true;
			else
				is_monster = false;

			/*My old rotational movement code, the children always hanged off a bit...
			//get length of offset
			offset_length = VectorLength (ent->movewith_offset);
			//get angular vector
			vectoangles (ent->movewith_offset, dir);
			//get rotation for this server frame
			VectorScale (self->avelocity, FRAMETIME, amove);
			//add rotation
			VectorAdd (dir, amove, dir);
			//reduce to angular vector
			AngleVectors (dir, dir, NULL, NULL);
			//restore to original length
			VectorScale (dir, offset_length, ent->movewith_offset);
			//add to origin of parent
			VectorAdd (self->s.origin, ent->movewith_offset, ent->s.origin);*/

			// For all but buttons, doors, and plats, move origin and match velocities
			if( strcmp(ent->classname,"func_door") && strcmp(ent->classname,"func_button")
				&& strcmp(ent->classname,"func_door_secret") && strcmp(ent->classname,"func_door_secret2")
				&& strcmp(ent->classname,"func_plat") && strcmp(ent->classname,"func_plat2")
				&& strcmp(ent->classname,"func_water") && strcmp(ent->classname,"turret_wall")
				&& (strcmp(ent->classname,"monster_turret") || !(ent->spawnflags & 128)) )
			{
				VectorMA(self->s.origin, ent->movewith_offset[0], forward, ent->s.origin);
				VectorMA(ent->s.origin, ent->movewith_offset[1], right, ent->s.origin);
				VectorMA(ent->s.origin, ent->movewith_offset[2], up, ent->s.origin);
				VectorCopy(self->velocity, ent->velocity);
			}

			// If parent is spinning, add appropriate velocities
			VectorSubtract(ent->s.origin, self->s.origin, offset);
			if(self->avelocity[PITCH] != 0)
			{
				ent->velocity[2] -= offset[0] * self->avelocity[PITCH] * M_PI / 180;
				ent->velocity[0] += offset[2] * self->avelocity[PITCH] * M_PI / 180;
			}
			if(self->avelocity[YAW] != 0)
			{
				ent->velocity[0] -= offset[1] * self->avelocity[YAW] * M_PI / 180.;
				ent->velocity[1] += offset[0] * self->avelocity[YAW] * M_PI / 180.;
			}
			if(self->avelocity[ROLL] != 0)
			{
				ent->velocity[1] -= offset[2] * self->avelocity[ROLL] * M_PI / 180;
				ent->velocity[2] += offset[1] * self->avelocity[ROLL] * M_PI / 180;
			}
			VectorScale (self->avelocity, FRAMETIME, amove);
			//match angular velocities
			if (self->turn_rider)
			{
				if (!strcmp(ent->classname,"func_rotating"))
				{
				/*	if(ent->movedir[0] > 0)
						ent->s.angles[1] = self->s.angles[1];
					else if(ent->movedir[1] > 0)
					//	ent->s.angles[1] += amove[1];
						ent->s.angles[1] += (ent->child_attach_angles[1] + parent_angle_change[1]);
					else if(ent->movedir[2] > 0)
						ent->s.angles[1] = self->s.angles[1];*/
					float	cr, sr;
					float	cy, sy;

					cy = cos((ent->s.angles[1]-parent_angle_change[1])*M_PI/180);
					sy = sin((ent->s.angles[1]-parent_angle_change[1])*M_PI/180);
					cr = cos((ent->s.angles[2]-parent_angle_change[2])*M_PI/180);
					sr = sin((ent->s.angles[2]-parent_angle_change[2])*M_PI/180);
					if(ent->movedir[0] > 0)
					{
						ent->s.angles[1] = parent_angle_change[1];
					}
					else if(ent->movedir[1] > 0)
					{
						ent->s.angles[1] += amove[1];
						ent->s.angles[2] =  parent_angle_change[2]*cy;
						ent->s.angles[0] = -parent_angle_change[2]*sy;
					}
					else if(ent->movedir[2] > 0)
					{
						ent->s.angles[1] = parent_angle_change[0]*-sy;
					}

				}
				else if (!is_monster)
				{   // Not a monster/actor. We want monsters/actors to be able to turn on
					// their own.
					if(!ent->do_not_rotate)
					{
						if(!strcmp(ent->classname,"turret_breach") || !strcmp(ent->classname,"turret_base"))
							VectorCopy(self->avelocity, ent->avelocity);
						else if (!strcmp(ent->classname,"func_door_rotating"))
						{
							VectorCopy(self->avelocity, ent->avelocity);
						//	VectorCopy(parent_angle_change, ent->pos1);
							VectorAdd(ent->child_attach_angles, parent_angle_change, ent->pos1);
							VectorMA (ent->pos1, ent->moveinfo.distance, ent->movedir, ent->pos2);
							if (ent->moveinfo.state == STATE_TOP)
								VectorCopy (ent->pos2, ent->s.angles);
							else
								VectorCopy (ent->pos1, ent->s.angles);
							VectorCopy (ent->s.origin, ent->moveinfo.start_origin);
							VectorCopy (ent->pos1,     ent->moveinfo.start_angles);
							VectorCopy (ent->s.origin, ent->moveinfo.end_origin);
							VectorCopy (ent->pos2,     ent->moveinfo.end_angles);
						}
						//don't move child's angles if not moving self
						else if (ent->solid == SOLID_BSP) //&& (VectorLength(self->velocity) || VectorLength(self->avelocity)) )
						{	// brush models always start out with angles=0,0,0 (after G_SetMoveDir).
							// Use more accuracy here.
							VectorCopy(self->avelocity, ent->avelocity);
						//	VectorCopy(self->s.angles, ent->s.angles);
							VectorAdd(ent->child_attach_angles, parent_angle_change, ent->s.angles);
						}
						else if(ent->movetype == MOVETYPE_NONE)
						{
							VectorCopy(self->avelocity, ent->avelocity);
						//	VectorCopy(self->s.angles, ent->s.angles);
							VectorAdd(ent->child_attach_angles, parent_angle_change, ent->s.angles);
						}
						else
						{
							// For point entities, best we can do is apply a delta to
							// the angles. This may result in foulups if anything gets blocked
						//	VectorAdd(ent->s.angles, amove, ent->s.angles);
							VectorAdd(ent->child_attach_angles, parent_angle_change, ent->s.angles);
						}
					}
				}
				else if (is_monster)
				{
					// Knightmare- adjust monster_turret's aiming angles for yaw movement
					if (!strcmp(ent->classname,"monster_turret") && amove[YAW] != 0)
					{
						if (ent->offset && ent->old_offset && (ent->offset[1] != -1 && ent->offset[1] != -2))
						{
							ent->offset[1] = ent->old_offset[1] + parent_angle_change[1];
							// Don't let it hit -1 or -2, these are reserved for up and down
							while (ent->offset[1] < 0) 
								ent->offset[1] += 360;

							while (ent->offset[1] >= 360) // Don't let it go over 360
								ent->offset[1] -= 360;
						}
						// if it's inactive, rotate it like other point entities
						if (ent->s.frame < 2)
							VectorAdd(ent->child_attach_angles, parent_angle_change, ent->s.angles);
						// update aiming
						//else if (!strcmp(self->classname, "func_rotating") || !strcmp(self->classname, "func_rotating_dh"))
						//	TurretAim (ent);
					}
					// otherwise don't re-angle turrets
					//else if (strcmp(ent->classname,"monster_turret"))
					//	VectorAdd(ent->s.angles, amove, ent->s.angles);
				}
			}
			// Special cases:
			// Func_door/func_button and trigger fields
			if( !strcmp(ent->classname,"func_door") || !strcmp(ent->classname,"func_button")
				|| !strcmp(ent->classname,"func_door_secret") || !strcmp(ent->classname,"func_door_secret2")
				|| !strcmp(ent->classname,"func_plat") || !strcmp(ent->classname,"func_plat2")
				|| !strcmp(ent->classname,"func_water") || !strcmp(ent->classname,"turret_wall")
				|| (!strcmp(ent->classname,"monster_turret") && ent->spawnflags & 128) )
			{
				VectorAdd(ent->s.angles, ent->org_angles, angles);
				G_SetMovedir (angles, ent->movedir);
				//Knightmare- these entities need special calculations
				if (!strcmp(ent->classname,"monster_turret") || !strcmp(ent->classname,"turret_wall")) 
				{
					vec3_t		eforward, tangles;

					VectorAdd(ent->deploy_angles, parent_angle_change, tangles);
					AngleVectors (tangles, eforward, NULL, NULL);
					VectorMA(self->s.origin, ent->movewith_offset[0], forward, ent->pos1);
					VectorMA(ent->pos1, ent->movewith_offset[1], right, ent->pos1);
					VectorMA(ent->pos1,  ent->movewith_offset[2], up, ent->pos1);
					VectorMA(ent->pos1, 32, eforward, ent->pos2);
				}
				else if (!strcmp(ent->classname,"func_door_secret"))
				{
					vec3_t	eforward, eright, eup;

					AngleVectors (ent->s.angles, eforward, eright, eup);
					VectorMA(self->s.origin, ent->movewith_offset[0], forward, ent->pos0);
					VectorMA(ent->pos0, ent->movewith_offset[1], right, ent->pos0);
					VectorMA(ent->pos0,  ent->movewith_offset[2], up, ent->pos0);
					if (ent->spawnflags & 4) // SECRET_1ST_DOWN
						VectorMA (ent->pos0, -1 * ent->width, eup, ent->pos1);
					else
						VectorMA (ent->pos0, ent->side * ent->width, eright, ent->pos1);
					VectorMA (ent->pos1, ent->length, eforward, ent->pos2);
				}
				else if (!strcmp(ent->classname,"func_door_secret2"))
				{
					vec3_t	eforward, eright, eup;

					AngleVectors(ent->s.angles, eforward, eright, eup);
					VectorMA(self->s.origin, ent->movewith_offset[0], forward, ent->pos0);
					VectorMA(ent->pos0, ent->movewith_offset[1], right, ent->pos0);
					VectorMA(ent->pos0,  ent->movewith_offset[2], up, ent->pos0);
					if (ent->spawnflags & 64) // SEC_MOVE_FORWARD
						VectorScale(eforward, ent->length, eforward);
					else
						VectorScale(eforward, ent->length * -1 , eforward);
					if (ent->spawnflags & 32) //SEC_MOVE_RIGHT
						VectorScale(eright, ent->width, eright);
					else
						VectorScale(eright, ent->width * -1, eright);
					if (ent->spawnflags & 4) // SEC_1ST_DOWN
					{
						VectorAdd(ent->pos0, eforward, ent->pos1);
						VectorAdd(ent->pos1, eright, ent->pos2);
					}
					else
					{
						VectorAdd(ent->pos0, eright, ent->pos1);
						VectorAdd(ent->pos1, eforward, ent->pos2);
					}
				}
				else if (!strcmp(ent->classname,"func_plat") || !strcmp(ent->classname,"func_plat2"))
				{
					VectorMA(self->s.origin, ent->movewith_offset[0], forward, ent->pos1);
					VectorMA(ent->pos1, ent->movewith_offset[1], right, ent->pos1);
					VectorMA(ent->pos1,  ent->movewith_offset[2], up, ent->pos1);
					VectorCopy(ent->pos1, ent->pos2);
					ent->pos2[2] -= ent->moveinfo.distance;
					VectorCopy (ent->pos1, ent->moveinfo.start_origin);
					VectorCopy (ent->s.angles, ent->moveinfo.start_angles);
					VectorCopy (ent->pos2, ent->moveinfo.end_origin);
					VectorCopy (ent->s.angles, ent->moveinfo.end_angles);
				}
				else // func_button or func_door or func_water
				{
					VectorMA(self->s.origin, ent->movewith_offset[0], forward, ent->pos1);
					VectorMA(ent->pos1, ent->movewith_offset[1], right, ent->pos1);
					VectorMA(ent->pos1,  ent->movewith_offset[2], up, ent->pos1);
					VectorMA (ent->pos1, ent->moveinfo.distance, ent->movedir, ent->pos2);
					VectorCopy(ent->pos1, ent->moveinfo.start_origin);
					VectorCopy(ent->s.angles, ent->moveinfo.start_angles);
					VectorCopy(ent->pos2, ent->moveinfo.end_origin);
					VectorCopy(ent->s.angles, ent->moveinfo.end_angles);
				}
				if (ent->moveinfo.state == STATE_LOWEST || ent->moveinfo.state == STATE_BOTTOM
					|| ent->moveinfo.state == STATE_TOP)
				{
					// Velocities for door/button movement are handled in normal
					// movement routines
					VectorCopy(self->velocity, ent->velocity);
					// Sanity insurance:
					if (!strcmp(ent->classname,"func_plat") || !strcmp(ent->classname,"func_plat2"))
					{	// top and bottom states are reversed for plats
						if (ent->moveinfo.state == STATE_BOTTOM)
							VectorCopy(ent->pos2, ent->s.origin);
						else if (ent->moveinfo.state == STATE_TOP)
							VectorCopy(ent->pos1, ent->s.origin);
					}
					else
					{	// STATE_LOWEST is for func_door_secret in its starting postion
						if (ent->moveinfo.state == STATE_LOWEST)
							VectorCopy(ent->pos0, ent->s.origin);
						else if (ent->moveinfo.state == STATE_BOTTOM)
							VectorCopy(ent->pos1, ent->s.origin);
						else if (ent->moveinfo.state == STATE_TOP)
							VectorCopy(ent->pos2, ent->s.origin);
					}
				}
			}
			if (amove[YAW])
			{
				// Cross fingers here... move bounding boxes of doors and buttons
				if( !strcmp(ent->classname,"func_door") || !strcmp(ent->classname,"func_button")
				|| !strcmp(ent->classname,"func_door_secret") || !strcmp(ent->classname,"func_door_secret2")
				|| !strcmp(ent->classname,"func_plat") || !strcmp(ent->classname,"func_plat2")
				|| !strcmp(ent->classname,"func_water") || (ent->solid == SOLID_TRIGGER)
				|| !strcmp(ent->classname,"turret_wall")
				|| (!strcmp(ent->classname,"monster_turret") && ent->spawnflags & 128) )
				{
					float       ca, sa, yaw;
					vec3_t      p00, p01, p10, p11;

					// Adjust bounding box for yaw
					yaw = ent->s.angles[YAW] * M_PI / 180.;
					ca  = cos(yaw);
					sa  = sin(yaw);
					p00[0] = ent->org_mins[0]*ca - ent->org_mins[1]*sa;
					p00[1] = ent->org_mins[1]*ca + ent->org_mins[0]*sa;
					p01[0] = ent->org_mins[0]*ca - ent->org_maxs[1]*sa;
					p01[1] = ent->org_maxs[1]*ca + ent->org_mins[0]*sa;
					p10[0] = ent->org_maxs[0]*ca - ent->org_mins[1]*sa;
					p10[1] = ent->org_mins[1]*ca + ent->org_maxs[0]*sa;
					p11[0] = ent->org_maxs[0]*ca - ent->org_maxs[1]*sa;
					p11[1] = ent->org_maxs[1]*ca + ent->org_maxs[0]*sa;
					ent->mins[0] = p00[0];
					ent->mins[0] = min(ent->mins[0],p01[0]);
					ent->mins[0] = min(ent->mins[0],p10[0]);
					ent->mins[0] = min(ent->mins[0],p11[0]);
					ent->mins[1] = p00[1];
					ent->mins[1] = min(ent->mins[1],p01[1]);
					ent->mins[1] = min(ent->mins[1],p10[1]);
					ent->mins[1] = min(ent->mins[1],p11[1]);
					ent->maxs[0] = p00[0];
					ent->maxs[0] = max(ent->maxs[0],p01[0]);
					ent->maxs[0] = max(ent->maxs[0],p10[0]);
					ent->maxs[0] = max(ent->maxs[0],p11[0]);
					ent->maxs[1] = p00[1];
					ent->maxs[1] = max(ent->maxs[1],p01[1]);
					ent->maxs[1] = max(ent->maxs[1],p10[1]);
					ent->maxs[1] = max(ent->maxs[1],p11[1]);
				}
			}

			// FMOD 
			if(!Q_stricmp(ent->classname,"target_playback"))
				FMOD_UpdateSpeakerPos(ent);

			// Correct func_door_rotating start/end positions
	/*		if (!strcmp(ent->classname,"func_door_rotating") && (!ent->do_not_rotate))
			{
				VectorCopy (ent->s.angles, ent->pos1);
				VectorMA (ent->s.angles, ent->moveinfo.distance, ent->movedir, ent->pos2);
				VectorCopy (ent->s.origin, ent->moveinfo.start_origin);
				VectorCopy (ent->pos1, ent->moveinfo.start_angles);
				VectorCopy (ent->s.origin, ent->moveinfo.end_origin);
				VectorCopy (ent->pos2, ent->moveinfo.end_angles);
			}*/
			ent->s.event = self->s.event;
			gi.linkentity(ent);
		}
	}
	//Knightmare- now move prox mines
	if (self->spawnflags & TRAIN_SPLINE)
		train_move_prox (self);
}

/*
void Throw_child_debris (edict_t *self)
{
	vec3_t	origin;
	vec3_t	chunkorigin;
	vec3_t	size;
	int		count;
	int		mass;

	// bmodel origins are (0 0 0), we need to adjust that here
	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);
	VectorCopy (origin, self->s.origin);
	self->takedamage = DAMAGE_NO;

	// start chunks towards the center
	VectorScale (size, 0.5, size);
	mass = self->mass;
	if (!mass)
		mass = 75;
	// big chunks
	if (mass >= 100)
	{
		count = mass / 100;
		if (count > 8)
			count = 8;
		while(count--)
		{
			chunkorigin[0] = origin[0] + crandom() * size[0];
			chunkorigin[1] = origin[1] + crandom() * size[1];
			chunkorigin[2] = origin[2] + crandom() * size[2];
			ThrowDebris (self, "models/objects/debris1/tris.md2", 1, chunkorigin, 0, 0);
		}
	}
	// small chunks
	count = mass / 25;
	if (count > 16)
		count = 16;
	while(count--)
	{
		chunkorigin[0] = origin[0] + crandom() * size[0];
		chunkorigin[1] = origin[1] + crandom() * size[1];
		chunkorigin[2] = origin[2] + crandom() * size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", 2, chunkorigin, 0, 0);
	}
}*/

void train_kill_children (edict_t *self)
{
	edict_t	*ent = NULL;
	int i;

	if (!self->targetname)
		return;
	ent = g_edicts+1; // skip the worldspawn
	for (i = 1; i < globals.num_edicts; i++, ent++)
	{
		if (!ent->classname)
			continue;
		if (!ent->movewith)
			continue;
		if(!ent->inuse)
			return;
		if (!strcmp(ent->movewith, self->targetname))
		{	//recursively destroy each child's children
			train_kill_children (ent);
			//if (ent->solid == SOLID_BSP)
			//	Throw_child_debris(ent);
			//gib monsters
			if ((ent->svflags & SVF_MONSTER) || (ent->svflags & SVF_DEADMONSTER))
				T_Damage (ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, (ent->health + 999), self->dmg, DAMAGE_NO_PROTECTION, MOD_TRIGGER_HURT);
		//	else if (ent->solid == SOLID_BSP)
		//		func_explosive_die (ent, self, self, 100, ent->s.origin);
			else if (ent->mass >= 400)
				BecomeExplosion3 (ent);
			else
				BecomeExplosion1 (ent);
		}
	}
}

void train_remove_children (edict_t *self)
{
	edict_t	*ent = NULL;
	int i;

	if (!self->targetname)
		return;
	ent = g_edicts+1; // skip the worldspawn
	for (i = 1; i < globals.num_edicts; i++, ent++)
	{
		if (!ent->classname)
			continue;
		if (!ent->movewith)
			continue;
		if(!ent->inuse)
			return;
		if (!strcmp(ent->movewith, self->targetname))
		{	//recursively remove each child's children
			train_remove_children (ent);
			G_FreeEdict(ent);
		}
	}
}

//Knightmare- these functions fade away the movewith children of func_breakaway
#ifdef KMQUAKE2_ENGINE_MOD
void fade_child (edict_t *ent);

void fade_children (edict_t *self)
{
	edict_t	*ent = NULL;
	int i;

	if (!self->targetname)
		return;
	if (strcmp(self->classname, "func_breakaway")) //only for func_breakaway
		return;
	ent = g_edicts+1; // skip the worldspawn
	for (i = 1; i < globals.num_edicts; i++, ent++)
	{
		if (!ent->classname)
			continue;
		if (!ent->movewith)
			continue;
		if(!ent->inuse)
			return;
		if (!strcmp(ent->movewith, self->targetname) && (ent->s.modelindex > 0))
		{
			if (ent->s.renderfx & RF_TRANSLUCENT)
				ent->s.alpha = 0.70;
			else if (ent->s.effects & EF_SPHERETRANS)
				ent->s.alpha = 0.30;
			else if (!(self->s.alpha)  || self->s.alpha <= 0.0)
				ent->s.alpha = 1.00;
			fade_child (ent);
		}
	}
}

void fade_child (edict_t *ent)
{
	ent->s.alpha -= 0.05;
	if (ent->s.alpha <= 0.0)
	{
		G_FreeEdict(ent);
		return;
	}
	ent->nextthink = level.time + 0.2;
	ent->think = fade_child;
	gi.linkentity (ent);
}

// Gotta have this for extractfuncs...
void fade_children2 (edict_t *self)
{
}
#else
void fade_children (edict_t *self)
{
	edict_t	*ent = NULL;
	int i;

	if (!self->targetname)
		return;
	if (strcmp(self->classname, "func_breakaway")) //only for func_breakaway
		return;
	ent = g_edicts+1; // skip the worldspawn
	for (i = 1; i < globals.num_edicts; i++, ent++)
	{
		if (!ent->classname)
			continue;
		if (!ent->movewith)
			continue;
		if(!ent->inuse)
			return;
		if (!strcmp(ent->movewith, self->targetname)
			&& (ent->s.modelindex > 0) && !(ent->s.effects & EF_SPHERETRANS) )			 
			ent->s.renderfx |= RF_TRANSLUCENT;
	}
}

void fade_children2 (edict_t *self)
{
	edict_t	*ent = NULL;
	int i;

	if (!self->targetname)
		return;
	if (strcmp(self->classname, "func_breakaway")) //only for func_breakaway
		return;
	ent = g_edicts+1; // skip the worldspawn
	for (i = 1; i < globals.num_edicts; i++, ent++)
	{
		if (!ent->classname)
			continue;
		if (!ent->movewith)
			continue;
		if(!ent->inuse)
			return;
		if (!strcmp(ent->movewith, self->targetname))
		{
			if (ent->s.modelindex > 0)
			{
				ent->s.effects |= EF_SPHERETRANS;
				ent->s.renderfx &= ~RF_TRANSLUCENT;
			}
			ent->nextthink = level.time + 2;
			ent->think = G_FreeEdict;
		}
	}
}
#endif

void train_blocked (edict_t *self, edict_t *other)
{
	if (!(other->svflags & SVF_MONSTER) && (!other->client) )
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other && other->inuse)
		{
			// Lazarus: Some of our ents don't have origin near the model
			vec3_t save;
			VectorCopy(other->s.origin,save);
			VectorMA (other->absmin, 0.5, other->size, other->s.origin);
			BecomeExplosion1 (other);
		}
		return;
	}
	VectorClear (self->avelocity); //don't turn children when blocked

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
	if (self->target_ent && !(IsRogueMap()))
	{	//Knightmare- skip this for Rogue maps
		// train speed changes are handled differently
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
		else if (self->spawnflags & TRAIN_ROT_CONST)
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
		// Spline trains stop rotating when waiting
		if (self->spawnflags & TRAIN_SPLINE)
		{
			VectorClear(self->avelocity);
			VectorClear(self->velocity);
			train_move_children(self);
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
			// PMM - clear target_ent, let train_next get called when we get used
			if (IsRogueMap()) // Knightmare- only do this for Rogue maps
				self->target_ent = NULL;
			else
				train_next (self);
			// pmm
			self->spawnflags &= ~TRAIN_START_ON;
			VectorClear (self->velocity);
			if ( !(self->spawnflags & TRAIN_ROT_CONST) )
				VectorClear (self->avelocity); // Knightmare added
			// Lazarus: turn off animation for stationary trains
			if (!strcmp(self->classname, "func_train"))
				self->s.effects &= ~(EF_ANIM_ALL | EF_ANIM_ALLFAST);
			self->nextthink = 0;
		}

		if (!(self->flags & FL_TEAMSLAVE))
		{
			if (self->moveinfo.sound_end)
				gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC
			self->s.sound = 0;
		}
	}
	else
	{
		if (self->spawnflags & TRAIN_SPLINE)
			train_move_children(self);
		train_next (self);
	}
	
}

//PGM
void train_piece_wait (edict_t *self)
{
}
//PGM

//
// Rroff rotating train
//

void train_rotate (edict_t *self);

void train_children_think (edict_t *self)
{
	if (!self->enemy)
		return;
	if (self->enemy->spawnflags & TRAIN_ROTATE)
	{
		// The the train was changed from TRAIN_ROT_CONST to TRAIN_ROTATE
		// by a target_change... get da hell outta here.
		self->think = train_rotate;
		self->think(self);
		return;
	}

//	if(self->enemy->movewith_next && (self->enemy->movewith_next->movewith_ent == self->enemy))
//	{
//		set_child_movement(self->enemy);
//		self->nextthink = level.time + FRAMETIME;
//	}
	else 
//	if (level.time < 2)
		self->nextthink = level.time + FRAMETIME;
}

void train_rotate (edict_t *self)
{
	float	cur_yaw, idl_yaw, cur_pitch, idl_pitch, cur_roll, idl_roll;
	float	yaw_vel, pitch_vel, roll_vel;
	float	Dist_1, Dist_2, Distance;
	float	train_speed;

	if(!self->enemy || !self->enemy->inuse)
		return;

	if(self->enemy->spawnflags & TRAIN_ROT_CONST)
	{
		// The the train was changed from TRAIN_ROTATE to TRAIN_ROT_CONST
		// by a target_change... get da hell outta here.
		self->think = train_children_think;
		self->think(self);
		return;
	}

	cur_yaw = self->enemy->s.angles[YAW];
	idl_yaw = self->enemy->ideal_yaw;
	cur_pitch = self->enemy->s.angles[PITCH];
	idl_pitch = self->enemy->ideal_pitch;
	cur_roll = self->enemy->s.angles[ROLL];
	idl_roll = self->enemy->ideal_roll;

//	gi.dprintf("current angles=%g %g %g, ideal angles=%g %g %g\n",
//		cur_pitch,cur_yaw,cur_roll,idl_pitch,idl_yaw,idl_roll);

	yaw_vel   = self->enemy->yaw_speed;
	pitch_vel = self->enemy->pitch_speed;
	roll_vel  = self->enemy->roll_speed;

	if (cur_yaw == idl_yaw)
		self->enemy->avelocity[1] = 0;
	if (cur_pitch == idl_pitch)
		self->enemy->avelocity[0] = 0;
	if (cur_roll == idl_roll)
		self->enemy->avelocity[2] = 0;

	if ((cur_yaw == idl_yaw) && (cur_pitch == idl_pitch) && (cur_roll == idl_roll) )
	{
		self->nextthink = level.time + FRAMETIME;
		return;
	}

//YAW CHANGE CODE
	if (cur_yaw != idl_yaw)
	{
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
//Knightmare added
//PITCH CHANGE CODE
	if (cur_pitch != idl_pitch)
	{
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
//ROLL CHANGE CODE
	if (cur_roll != idl_roll)
	{
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
	//ys = self->enemy->avelocity[1];
	//gi.bprintf (PRINT_HIGH, "train cy: %i iy: %i ys: %i\n", cur_yaw, idl_yaw, ys);

	train_speed = VectorLength (self->enemy->velocity);
	if (train_speed == 0) //Don't rotate if we're stopped
		VectorClear(self->enemy->avelocity);

	self->nextthink = level.time + FRAMETIME;
}

void train_next (edict_t *self)
{
	edict_t		*ent;
	vec3_t		dest;
	qboolean	first;
	vec3_t		daoldorigin;
	vec3_t		v, angles; //Rroff
	// Knightmare added
	vec3_t		adjusted_pathpoint;
	vec3_t		corner_offset = {1,1,1};

	// Knightmare- func_train_origin support
//	if (self->spawnflags & TRAIN_ORIGIN)
//		gi.dprintf ("train_next: pathing by train origin\n");

	first = true;
again:
	if (!self->target)
	{
	//	gi.dprintf ("train_next: no next target\n");
		return;
	}
	if (self->movetype == MOVETYPE_TOSS)
	{
	//	gi.dprintf ("train_next: Strogg ship shot down\n");
		return;
	}

	ent = G_PickTarget (self->target);
	if (!ent)
	{
		gi.dprintf ("train_next: bad target %s\n", self->target);
		return;
	}
	// Knightmare- calc the real target for the train's mins,
	// since that is 1 unit below the corner of the bmodel in all 3 dimensions
	if ( adjust_train_corners->value )
		VectorSubtract(ent->s.origin, corner_offset, adjusted_pathpoint);
	else
		VectorCopy(ent->s.origin, adjusted_pathpoint);
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
		VectorCopy (self->s.origin, daoldorigin); //Knightmare- copy old orgin for reference
		if (self->spawnflags & TRAIN_ORIGIN)	// Knightmare- func_train_origin support
			VectorCopy (ent->s.origin, self->s.origin);
		else
			VectorSubtract (adjusted_pathpoint, self->mins, self->s.origin); // was ent->s.origin
		VectorCopy (self->s.origin, self->s.old_origin);
		self->s.event = EV_OTHER_TELEPORT;
		gi.linkentity (self);

		// Knightmare- move movewith children
		if (self->targetname)
		{
			edict_t	*e = NULL;
			vec3_t	dir;
			int i;

			VectorSubtract (self->s.origin, daoldorigin, dir);
			e = g_edicts+1; // skip the worldspawn
			// cycle through all ents to find ones to move with
			for (i = 1; i < globals.num_edicts; i++, e++)
			{
				if (!e->classname)
					continue;
				if (!e->movewith)
					continue;
				if (!strcmp(e->movewith, self->targetname))
				{
					VectorAdd(dir, e->s.origin, e->s.origin);
					VectorCopy (e->s.origin, e->s.old_origin);
					e->s.event = EV_OTHER_TELEPORT;
					gi.linkentity (e);
				}
			}
		}
		goto again;
	}

// PGM
// DHW: This shouldn't go here... we're one path_corner behind.
//      Set speed before train_next is called
// Knightmare- this is only used for Rogue maps
// train speed changes are handled differently
	if ((ent->speed) && IsRogueMap())
	{
		self->speed = ent->speed;
		self->moveinfo.speed = ent->speed;
		if(ent->accel)
			self->moveinfo.accel = ent->accel;
		else
			self->moveinfo.accel = ent->speed;
		if(ent->decel)
			self->moveinfo.decel = ent->decel;
		else
			self->moveinfo.decel = ent->speed;
		self->moveinfo.current_speed = 0;
	}
//PGM

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
		VectorSubtract (adjusted_pathpoint, self->mins, dest); // was ent->s.origin
	self->moveinfo.state = STATE_TOP;
	VectorCopy (self->s.origin, self->moveinfo.start_origin);
	VectorCopy (dest, self->moveinfo.end_origin);

	if (self->spawnflags & TRAIN_SPLINE)
	{
		float	speed;
		int		frames;

		self->from = self->to;
		self->to   = ent;
		self->moveinfo.bezier_ratio = 0.0;

		VectorSubtract(dest,self->s.origin,v);
		self->moveinfo.distance = VectorLength(v);
		frames = (int)(10 * self->moveinfo.distance/self->speed);
		if (frames < 1) frames = 1;
		speed = (10*self->moveinfo.distance)/(float)frames;
		self->moveinfo.speed = speed;
		self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;
		train_move_children (self);
	}

	// Rroff rotating
	if (self->spawnflags & TRAIN_ROTATE && !(ent->spawnflags & 2))
	{
//		VectorSubtract (ent->s.origin, self->s.origin, v);
//		self->ideal_yaw = vectoyaw(v);
//		self->ideal_pitch = vectopitch(v);			
		if (self->spawnflags & TRAIN_ORIGIN) {	// Knightmare- func_train_origin support
			VectorSubtract(ent->s.origin, self->s.origin, v);
		}
		else {
			VectorAdd(self->s.origin, self->mins, v);
			VectorSubtract(adjusted_pathpoint, v, v); // was ent->s.origin
		}
		vectoangles2(v, angles);
		self->ideal_pitch = angles[PITCH];
		self->ideal_yaw = angles[YAW];
		if (self->ideal_pitch < 0)
			self->ideal_pitch += 360;
		self->ideal_roll = ent->roll;

		VectorClear(self->movedir);
		self->movedir[1] = 1.0;
	}
	// Knightmare- add angular speed for ROT_CONST trains
	if (self->spawnflags & TRAIN_ROT_CONST)
	{
		self->avelocity[PITCH] = self->pitch_speed;
		self->avelocity[YAW] = self->yaw_speed;
		self->avelocity[ROLL] = self->roll_speed;
	}

	Move_Calc (self, dest, train_wait);
	self->spawnflags |= TRAIN_START_ON;

//PGM
	if (self->team && IsRogueMap())
	{
		edict_t *e;
		vec3_t	dir, dst;

		VectorSubtract (dest, self->s.origin, dir);
		for (e=self->teamchain; e; e = e->teamchain)
		{	// Knightmare- don't move other func_trains with team- 
			// this fixes the broken conveyors
		//	if (strcmp(e->classname, "func_train") && strcmp(e->classname, "model_train"))
			{
				VectorAdd(dir, e->s.origin, dst);
				VectorCopy(e->s.origin, e->moveinfo.start_origin);
				VectorCopy(dst, e->moveinfo.end_origin);

				e->moveinfo.state = STATE_TOP;
				e->speed = self->speed;
				e->moveinfo.speed = self->moveinfo.speed;
				e->moveinfo.accel = self->moveinfo.accel;
				e->moveinfo.decel = self->moveinfo.decel;
				e->movetype = MOVETYPE_PUSH;
				Move_Calc (e, dst, train_piece_wait);
			}
		}
	
	}
//PGM
}

void train_resume (edict_t *self)
{
	edict_t		*ent;
	vec3_t		dest;
	vec3_t		adjusted_pathpoint;
	vec3_t		corner_offset = {1,1,1};

	ent = self->target_ent;
	//Knightmare- calc the real target for the train's mins,
	//since that is 1 unit below the corner of the bmodel in all 3 dimensions
	if (adjust_train_corners->value)
		VectorSubtract(ent->s.origin, corner_offset, adjusted_pathpoint);
	else
		VectorCopy(ent->s.origin, adjusted_pathpoint);

	if (self->spawnflags & TRAIN_ORIGIN)	// Knightmare- func_train_origin support
		VectorCopy (ent->s.origin, dest);
	else
		VectorSubtract (adjusted_pathpoint, self->mins, dest); //was ent->s.origin
	self->moveinfo.state = STATE_TOP;
	VectorCopy (self->s.origin, self->moveinfo.start_origin);
	VectorCopy (dest, self->moveinfo.end_origin);

	Move_Calc (self, dest, train_wait);
	self->spawnflags |= TRAIN_START_ON;
	// Lazarus:
	if(self->spawnflags & TRAIN_ROT_CONST)
	{
		self->avelocity[0] = self->pitch_speed;
		self->avelocity[1]   = self->yaw_speed;
		self->avelocity[2]  = self->roll_speed;
	}
}

#define MODEL_TRAIN_ROTATE 32
#define MODEL_TRAIN_ROT_CONST 64

void func_train_find (edict_t *self)
{
	edict_t		*ent;
	vec3_t		daoldorigin;
	// Knightmare added
	vec3_t		adjusted_pathpoint;
	vec3_t		corner_offset = {1,1,1};

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

	// Lazarus: trains can change speed at path_corners
	if (ent->speed)
	{
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

	self->target = ent->target;

	// Knightmare- calc the real target for the train's mins,
	//since that is 1 unit below the corner of the bmodel in all 3 dimensions
	if (adjust_train_corners->value)
		VectorSubtract(ent->s.origin, corner_offset, adjusted_pathpoint);
	else
		VectorCopy(ent->s.origin, adjusted_pathpoint);
	
	// Rroff
	// Knightmare- don't think_rotate if that flag isn't set
	// This fixes a bug with trains becoming invisble in some maps
	if (self->spawnflags & TRAIN_ROTATE)
	{
		ent->think = train_rotate;
		ent->enemy = self;
		ent->nextthink = level.time + FRAMETIME;
	}
	//end Rroff
	else if (self->spawnflags & TRAIN_SPLINE)
	{
		ent->think = Train_Move_Spline;
		ent->enemy = self;
		ent->nextthink = level.time + FRAMETIME;
	}
	else
	{
		ent->think = train_children_think;
		ent->enemy = self;
		ent->nextthink = level.time + FRAMETIME;
	}
	self->postthink = train_move_children; // Knightmare- supports movewith

	VectorCopy (self->s.origin, daoldorigin); // Knightmare- copy old orgin for reference
	if (self->spawnflags & TRAIN_ORIGIN)	// Knightmare- func_train_origin support
		VectorCopy (ent->s.origin, self->s.origin);
	else
		VectorSubtract (adjusted_pathpoint, self->mins, self->s.origin); // was ent->s.origin
	VectorCopy (self->s.origin, self->s.old_origin);
	gi.linkentity (self);

	// Knightmare- move movewith pieces to spawning point
	if (self->targetname)
	{
		edict_t	*e = NULL;
		vec3_t	dir;
		int i;

		VectorSubtract (self->s.origin, daoldorigin, dir);
		e = g_edicts+1; // skip the worldspawn
		// cycle through all ents to find ones to move with
		for (i = 1; i < globals.num_edicts; i++, e++)
		{
			if (!e->classname)
				continue;
			if (!e->movewith)
				continue;
			if (!strcmp(e->movewith, self->targetname))
			{
				// Knightmare- save distance moved for turret_breach to add to its firing point
				if (!strcmp(e->classname, "turret_breach") || !strcmp(e->classname, "model_turret"))
					VectorCopy (dir, e->aim_point);

				VectorAdd(dir, e->s.origin, e->s.origin);
				VectorCopy (e->s.origin, e->s.old_origin);
				//This is now the child's original position
				if ( (e->solid == SOLID_BSP) && strcmp(e->classname,"func_rotating")
					&& strcmp(e->classname,"func_door_rotating"))
					ReInitialize_Entity(e);
				gi.linkentity (e);
			}
		}
	}

	// Knightmare- no sound or animation if we're not moving yet
	// sound is turned on in train_next
	self->s.sound = 0;
	self->s.effects &= ~(EF_ANIM_ALL | EF_ANIM_ALLFAST);

	// if not triggered, start immediately
	if (!self->targetname)
		self->spawnflags |= TRAIN_START_ON;

	if (self->spawnflags & TRAIN_START_ON)
	{
		// Lazarus: animated trains
		if (!strcmp(self->classname, "func_train"))
		{
			if (self->spawnflags & TRAIN_ANIM)
				self->s.effects |= EF_ANIM_ALL;
			else if (self->spawnflags & TRAIN_ANIM_FAST)
				self->s.effects |= EF_ANIM_ALLFAST;
		}
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
		VectorClear (self->avelocity); // Knightmare added
		if (!strcmp(self->classname, "func_train"))
			self->s.effects &= ~(EF_ANIM_ALL | EF_ANIM_ALLFAST);
		self->nextthink = 0;
	}
	else
	{
		if (!strcmp(self->classname, "func_train"))
		{
			if (self->spawnflags & TRAIN_ANIM)
				self->s.effects |= EF_ANIM_ALL;
			else if (self->spawnflags & TRAIN_ANIM_FAST)
				self->s.effects |= EF_ANIM_ALLFAST;
		}
		if (self->spawnflags & TRAIN_SPLINE)
		{
			// Back up a step
			self->moveinfo.bezier_ratio -= self->moveinfo.speed * FRAMETIME / self->moveinfo.distance;
			if(self->moveinfo.bezier_ratio < 0.)
				self->moveinfo.bezier_ratio = 0.;
		}

		if (self->target_ent)
			train_resume(self);
		else
			train_next(self);
	}
}

void SP_target_speaker (edict_t *ent);

void SP_func_train (edict_t *self)
{
	self->class_id = ENTITY_FUNC_TRAIN;

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

	// usermodel (for alias model vehicles, etc) goes on modelindex2
	if (self->usermodel && strlen(self->usermodel)) {
		char	modelname[256];
		// check for "models/" already in path
		if ( !strncmp(self->usermodel, "models/", 7) )
			Com_sprintf(modelname, sizeof(modelname), "%s", self->usermodel);
		else
			Com_sprintf(modelname, sizeof(modelname), "models/%s", self->usermodel);
		self->s.modelindex2 = gi.modelindex (modelname);
	}

	if (self->spawnflags & TRAIN_ROTATE) //Lazarus move_origin code
	{
		self->flags |= FL_ROTTRAIN;
		if ( (!VectorLength(self->bleft)) && (!VectorLength(self->tright)) )
		{
			VectorSet(self->bleft, -16, -16, -16);
			VectorSet(self->tright, 16, 16, 16);
		}
		VectorAdd(self->bleft,self->tright,self->move_origin);
		VectorScale(self->move_origin,0.5,self->move_origin);
	}

	if (self->spawnflags & TRAIN_SMOOTH)
		self->smooth_movement = 1;
	else
		self->smooth_movement = 0;

	//David Hyde's code for the origin offset
	VectorAdd(self->absmin, self->absmax, self->origin_offset);
	VectorScale(self->origin_offset, 0.5, self->origin_offset);
	VectorSubtract(self->origin_offset, self->s.origin, self->origin_offset);

	if (self->attenuation <= 0)
		self->attenuation = ATTN_IDLE;

	if (st.noise)
	{	// create a speaker entity at center of bmodel
		edict_t *speaker;

		self->noise_index = gi.soundindex  (st.noise);
		speaker = G_Spawn();
		speaker->classname = "moving_speaker";
		speaker->s.sound     = 0;
		speaker->volume = 1;
		speaker->attenuation = self->attenuation; // was 3
		speaker->owner = self;
		speaker->think       = moving_speaker_think;
		speaker->nextthink   = level.time + 2*FRAMETIME;
		speaker->spawnflags = 7; //looped on
		self->speaker = speaker;
	//	VectorAdd (self->s.origin, self->origin_offset, speaker->s.origin);
		if (VectorLength(self->s.origin))
			VectorCopy(self->s.origin, speaker->s.origin);
		else
		{
			VectorAdd(self->absmin,self->absmax, speaker->s.origin);
			VectorScale(speaker->s.origin, 0.5, speaker->s.origin);
		}
		VectorSubtract (speaker->s.origin, self->s.origin, speaker->offset);
	//	gi.dprintf ("moving_speaker offset %s from func_train\n", vtos(speaker->offset));
	}

	if (!self->speed)
		self->speed = 100;

	if ((self->spawnflags & TRAIN_ROTATE) && (self->spawnflags &TRAIN_ROT_CONST))
	{
		self->spawnflags &= ~TRAIN_ROTATE;
		self->spawnflags &= ~TRAIN_ROT_CONST;
		self->spawnflags |= TRAIN_SPLINE;
	}
	self->moveinfo.speed = self->speed;
	// Knightmare- auto detect accel and decel values
	if (self->accel)
		self->moveinfo.accel = self->accel;
	else
		self->moveinfo.accel = self->moveinfo.speed;

	if (self->decel)
		self->moveinfo.decel = self->decel;
	else
		self->moveinfo.decel = self->moveinfo.speed;

	self->use = train_use;

	if (self->health && self->health > 0) //Knightmare- detect health
	{
		//the func_explosive is handled exactly the same way. (look in g_misc.c)
		self->die = func_explosive_die;	//what to call when we die (in g_misc.c)
		self->takedamage = DAMAGE_YES;	//we take damage hits (spark, bleed, etc)
	}

	if (self->spawnflags & TRAIN_ANIM)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & TRAIN_ANIM_FAST)
		self->s.effects |= EF_ANIM_ALLFAST;

	// Mappack
	gi.linkentity (self);

	if (self->target)
	{
		// start trains on the second frame, to make sure their targets have had
		// a chance to spawn
		self->nextthink = level.time +  FRAMETIME;
		self->think = func_train_find;
	}
	else
		gi.dprintf ("func_train without a target at %s\n", vtos(self->absmin));
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
	self->class_id = ENTITY_TRIGGER_ELEVATOR;

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
	self->class_id = ENTITY_FUNC_TIMER;

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

TOGGLE	this allows the conveyor to be turned on and off, starts off by default

START_ON the conveyor will initially be present, only valid for TOGGLE walls

"speed"	default 102
"angle" direction in which to move stuff
*/

void func_conveyor_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & 1)
	{
		self->spawnflags &= ~1;
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}
	else
	{
		self->spawnflags |= 1;
		self->solid = SOLID_BSP;
		self->svflags &= ~SVF_NOCLIENT;
		KillBox (self);
	}
	gi.linkentity(self);

	if (!(self->spawnflags & 2))
		self->use = NULL;
/*	if (self->spawnflags & 1)
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
		self->count = 0;*/
}

void SP_func_conveyor (edict_t *self)
{
	self->class_id = ENTITY_FUNC_CONVEYOR;

	if (!self->speed)
		self->speed = 102;

	self->use = func_conveyor_use;
	gi.setmodel (self, self->model);

	self->movetype = MOVETYPE_CONVEYOR;
	if (!VectorLength(self->movedir))
		G_SetMovedir (self->s.angles, self->movedir);
/*	// just a static conveyor
	if (!(self->spawnflags & 1) && !(self->spawnflags & 2))
	{
		self->solid = SOLID_BSP;
		self->use = NULL;
		gi.linkentity (self);
		return;
	}
	// yell if the spawnflags are odd
	if (self->spawnflags & 1)
	{
		if (!(self->spawnflags & 2))
		{
			gi.dprintf("func_conveyor START_ON without TOGGLE\n");
			self->spawnflags |= 2;
		}
	}*/
	if (self->spawnflags & 1)
		self->solid = SOLID_BSP;
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}
	gi.linkentity (self);
}


/*QUAKED func_door_secret (0 .5 .8) ? always_shoot 1st_left 1st_down
A secret door.  Slide back and then to the side.

open_once		doors never closes
1st_left		1st move is left of arrow
1st_down		1st move is down from arrow
always_shoot	door is shootable even if targeted

"angle"		determines the direction
"dmg"		damage to inflic when blocked (default 2)
"wait"		how long to hold in the open position (default 5, -1 means hold)
"sounds"
0)		default
1)		silent
2-4		not used
5-99	custom sound- e.g. doors/dr05_strt.wav, doors/dr05_mid.wav, doors/dr05_end.wav
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
	if ((self->moveinfo.state != STATE_LOWEST) && (self->moveinfo.state != STATE_TOP))
		return;

	//added sound
	if (self->moveinfo.sound_start)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
	if (self->moveinfo.sound_middle) {
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}
	if (self->moveinfo.state == STATE_LOWEST)
	{
		self->moveinfo.state = STATE_DOWN;
		Move_Calc (self, self->pos1, door_secret_move1);
		door_use_areaportals (self, true);

	}
	else //Knightmare added
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
    //added sound
	self->s.sound = 0;
	if (self->moveinfo.sound_end)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC
}

void door_secret_move2 (edict_t *self)
{
    //added sound
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
    //added sound
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
    //added sound
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
    //added sound
	self->s.sound = 0;
	if (self->moveinfo.sound_end)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC
}

void door_secret_move6 (edict_t *self)
{
    //added sound
	if (self->moveinfo.sound_start)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
	if (self->moveinfo.sound_middle) {
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}
	self->moveinfo.state = STATE_DOWN;
	Move_Calc (self, self->pos0, door_secret_done);
}

void door_secret_done (edict_t *self)
{
	if (!(self->targetname) || (self->spawnflags & SECRET_ALWAYS_SHOOT))
	{
		self->health = 0;
		self->takedamage = DAMAGE_YES;
	}
	self->moveinfo.state = STATE_LOWEST;
    //added sound
	self->s.sound = 0;
	if (self->moveinfo.sound_end)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC

	door_use_areaportals (self, false);
}

void door_secret_blocked  (edict_t *self, edict_t *other)
{
	if (!(other->svflags & SVF_MONSTER) && (!other->client) )
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, self->pos0, other->s.origin, self->pos0, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other && other->inuse)
			BecomeExplosion1 (other);
		return;
	}

	if (level.time < self->touch_debounce_time)
		return;
	self->touch_debounce_time = level.time + 0.5;

	T_Damage (other, self, self, self->pos0, other->s.origin, self->pos0, self->dmg, 1, 0, MOD_CRUSH);
}

void door_secret_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	door_secret_use (self, attacker, attacker);
}

void SP_func_door_secret (edict_t *ent)
{
	vec3_t	forward, right, up;

	ent->class_id = ENTITY_FUNC_DOOR_SECRET;

	if (ent->sounds > 4 && ent->sounds < 100) // custom sounds
	{
		ent->moveinfo.sound_start = gi.soundindex  (va("doors/dr%02i_strt.wav", ent->sounds));
		ent->moveinfo.sound_middle = gi.soundindex  (va("doors/dr%02i_mid.wav", ent->sounds));
		ent->moveinfo.sound_end = gi.soundindex  (va("doors/dr%02i_end.wav", ent->sounds));
	}
	else if (ent->sounds != 1)
	{
		ent->moveinfo.sound_start = gi.soundindex  ("doors/dr1_strt.wav");
		ent->moveinfo.sound_middle = gi.soundindex  ("doors/dr1_mid.wav");
		ent->moveinfo.sound_end = gi.soundindex  ("doors/dr1_end.wav");
	}
	else
	{
		ent->moveinfo.sound_start = 0;
		ent->moveinfo.sound_middle = 0;
		ent->moveinfo.sound_end = 0;
	}

	if (ent->attenuation <= 0)
		ent->attenuation = ATTN_STATIC;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);
	
	ent->use = door_secret_use;
	ent->blocked = door_secret_blocked; // Knightmare- this was missing
	ent->moveinfo.state = STATE_LOWEST;

	if (!(ent->targetname) || (ent->spawnflags & SECRET_ALWAYS_SHOOT))
	{
		ent->health = 0;
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

	// calculate positions
	VectorCopy (ent->s.origin, ent->pos0);
	AngleVectors (ent->s.angles, forward, right, up);
	VectorClear (ent->s.angles);
	//ent->side = 1.0 - (ent->spawnflags & SECRET_1ST_LEFT);

	if (ent->spawnflags & SECRET_1ST_LEFT)
		ent->side = -1.0;
	else
		ent->side = 1.0;
	if (!ent->width)
	{
		if (ent->spawnflags & SECRET_1ST_DOWN)
			ent->width = fabs(DotProduct(up, ent->size)) - 2;
		else
			ent->width = fabs(DotProduct(right, ent->size)) - 2;
	}
	if (!ent->length)
		ent->length = fabs(DotProduct(forward, ent->size)) - 2;
	if (ent->spawnflags & SECRET_1ST_DOWN)
		VectorMA (ent->s.origin, -1 * ent->width, up, ent->pos1);
	else
		VectorMA (ent->s.origin, ent->side * ent->width, right, ent->pos1);
	VectorMA (ent->pos1, ent->length, forward, ent->pos2);

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
	ent->classname = "func_door";
	ent->postthink = train_move_children; //Knightmare- now supports movewith

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
	ent->class_id = ENTITY_FUNC_KILLBOX;

	gi.setmodel (ent, ent->model);
	ent->use = use_killbox;
	ent->svflags = SVF_NOCLIENT;
}


//===================================================================
// LAZARUS additions
//===================================================================
//
// func_pushable
// Moveable crate. Similar to misc_explobox, but:
// 1) Uses a brush model rather than a .md2 model. Model can be ANY
//    shape, but the func_pushable uses the bounding box to determine
//    whether it is being touched or not, so cubes are generally best.
// 2) Can be pushed off a ledge and damaged by falling
// 3) Default dmg = 0 (no fireball) and health = 0 (indestructible)
// 4) Plays a sound when moving
// 
// targetname - If triggered, pushable object self-destructs, throwing
//              debris chunks and (if dmg>0) exploding
// health     - Damage sustained by object before it "dies". On death,
//              object throws debris and (if dmg>0) explodes. func_pushable
//              can be damaged by shooting (inapplicable in Tremor) or
//              by falling damage or by being crushed by a func_door,
//              func_train, etc. Set health=0 for a func_pushable that
//              cannot be damaged by falling or weapon fire. Set health=-1
//              for func_pushable that will block MOVETYPE_PUSH entities
//              (func_door, etc.)
// dmg        - Radius damage due to object explosion. Set to 0 for
//              no fireball
// mass       - Weight of the object. Heavier objects are harder to push.
//              Default = 400.
// sounds     - Sound played when moved.
//              0 = none
//              1 = tank/thud.wav
//              2 = weapons/rg_hum.wav
//              3 = weapons/rockfly.wav
//             
// SF=1       - Trigger spawn. Func_pushable is invisible and non-solid until triggered.
//    2       - No knockback. Not moved by weapon fire (invulnerable func_pushables aren't
//              affected by weapon fire in either case)
//
//=============
// box_movestep
//
// Similar to SV_movestep in monster code, but handles falling
// objects, and doesn't balk at water/lava/slime
//=============
//

qboolean box_movestep (edict_t *ent, vec3_t move, qboolean relink)
{
	vec3_t		oldorg, neworg, end;
	trace_t		trace;
	float		stepsize;

	vec3_t		maxs, mins, origin;

// try the move	
	VectorAdd (ent->s.origin, ent->origin_offset, origin);
	VectorCopy (origin, oldorg);
	VectorAdd (origin, move, neworg);
	VectorCopy (ent->size, maxs);
	VectorScale (maxs, 0.5, maxs);
	VectorNegate (maxs, mins);


// push down from a step height above the wished position
	stepsize = 1;

	neworg[2] += stepsize;
	VectorCopy (neworg, end);
	end[2] -= stepsize*2;

	trace = gi.trace (neworg, mins, maxs, end, ent, MASK_MONSTERSOLID);

	if (trace.allsolid)
		return false;

	if (trace.startsolid)
	{
		neworg[2] -= stepsize;
		trace = gi.trace (neworg, mins, maxs, end, ent, MASK_MONSTERSOLID);
		if (trace.allsolid || trace.startsolid)
			return false;
	}

	if (trace.fraction == 1)
	{
	// if box had the ground pulled out, go ahead and fall
		VectorAdd (ent->s.origin, move, ent->s.origin);
		if (relink)
		{
			gi.linkentity (ent);
			G_TouchTriggers (ent);
		}
		ent->groundentity = NULL;
		return true;
	}

	VectorCopy (trace.endpos, origin);
	VectorSubtract (origin, ent->origin_offset, ent->s.origin);

	ent->groundentity = trace.ent;
	ent->groundentity_linkcount = trace.ent->linkcount;

// the move is ok
	if (relink)
	{
		gi.linkentity (ent);
		G_TouchTriggers (ent);
	}
	return true;
}

void box_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if(self->deathtarget)
	{
		self->target = self->deathtarget;
		if(self->activator)
			G_UseTargets (self, self->activator);
		else
			G_UseTargets (self, attacker);
		self->target = NULL;
	}
	func_explosive_die (self, inflictor, attacker, damage, point);
}

void box_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;
	box_die(self, self, other, self->health, vec3_origin);
}

//
//===============
//box_walkmove
// similar to M_walkmove, but:
// 1) works with entities not on the ground
// 2) calls moving box-specific box_movestep, which allows
//    boxes to fall
//===============
//

qboolean box_walkmove (edict_t *ent, float yaw, float dist)
{
	vec3_t	move;
	
	if (!ent->groundentity && !(ent->flags & (FL_FLY|FL_SWIM)))
		return false;

	yaw = yaw*M_PI*2 / 360;
	
	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

	return box_movestep(ent, move, true);
}

void box_water_friction(edict_t *ent)
{
	int i;
	float		speed, newspeed, control;

	if (!(ent->flags & FL_SWIM)) return;	// should not be here
	if (ent->waterlevel==0) return;			// likewise
	if (ent->crane_control) return;			// currently under control of a crane
	if ((ent->velocity[0]==0) && (ent->velocity[1]==0))
	{
		ent->nextthink = 0;
		return;
	}
	for(i=0; i<2; i++)
	{
		if(ent->velocity[i] != 0)
		{
			speed = fabs(ent->velocity[i]);
			control = speed < 100 ? 100 : speed;
			newspeed = speed - (FRAMETIME * control * ent->waterlevel);
			if (newspeed < 0)
				newspeed = 0;
			newspeed /= speed;
			ent->velocity[i] *= newspeed;
		}
	}
	ent->nextthink = level.time + FRAMETIME;
	gi.linkentity(ent);
}

void box_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float	e;   // coefficient of restitution
	float   m;
	float	ratio;
	float	v11, v12, v21, v22;
	int     axis;
	vec3_t	v1, v2, v;
	vec3_t	origin;
	edict_t *bottom, *top;

	//Knightmare- ignore prox mines attached to self
	if (!strcmp(other->classname, "prox") && other->movewith_ent && other->movewith_ent == self)
		return;

	// if other is another func_pushable, AND self is in
	// water, move in proportion to relative masses
	if (other->movetype == MOVETYPE_PUSHABLE)
	{
		int		damage;
		float	delta;
		float	vslide;
		vec3_t	dir, impact_v;

		// Check for impact damage first
		if(self->health > 0)
		{
			VectorSubtract(other->velocity,self->velocity,impact_v);
			delta = VectorLength(impact_v);
			delta = delta*delta*0.0001;
			if (delta > 30)
			{
				damage = (float)(other->mass)/(float)(self->mass) * ( (delta-30)/2 );
				if (damage > 0)
				{
					VectorSubtract(self->s.origin,other->s.origin,dir);
					VectorNormalize(dir);
					T_Damage (self, other, other, dir, self->s.origin, vec3_origin, damage, 0, 0, MOD_FALLING);
					if(self->health <= 0) return;
				}
			}
		}
		if(self->waterlevel==0) return;

		// 06/03/00 change: If either func_pushable is currently being moved 
		// by crane, bail out.
		if(self->crane_control) return;
		if(other->crane_control) return;

		// Since func_pushables have a bounding box, impact will ALWAYS be on one of the
		// planes of the bounding box. The "plane" argument isn't always used, but since 
		// all entities involved use a parallelepiped bounding box we can rely on offsets
		// to centers to figure out which side the impact is on.
		VectorAdd (self->absmax,self->absmin,v1);
		VectorScale(v1,0.5,v1);
		VectorAdd (other->absmax,other->absmin,v2);
		VectorScale(v2,0.5,v2);
		VectorSubtract(v1,v2,v);
		VectorNormalize(v);
		axis = 0;
		if(fabs(v[1]) > fabs(v[axis])) axis = 1;
		if(fabs(v[2]) > fabs(v[axis])) axis = 2;

		e = 0.5; // coefficient of restitution
		m = (float)(other->mass)/(float)(self->mass);
		v11 = self->velocity[axis];
		v21 = other->velocity[axis];
		v22 = ( e*(v11-v21) + v11 + m*v21 ) / (1.0 + m);
		v12 = v22 + e*(v21-v11);
		self->velocity[axis] = v12;
		other->velocity[axis] = v22;

		// Assuming frictionless surfaces, momentum of crate is conserved in 
		// other two directions (so velocity doesn't change)... BUT we want 
		// to get the bottom crate out from underneath the other one,
		// so we're gonna be a little "creative"
		if(axis==2)
		{
			if(v[2] > 0)
			{
				bottom = other;
				top = self;
				VectorNegate(v,v);
			} else {
				bottom = self;
				top = other;
			}
			v[2] = 0;
			VectorNormalize(v);
			if(!VectorLength(v))
			{
				v[0] = crandom();
				v[1] = sqrt(1.0 - v[0]*v[0]);
			}
			vslide = 10;
			if(fabs(bottom->velocity[0]) < 50)
				bottom->velocity[0] += v[0] * vslide;
			if(fabs(bottom->velocity[1]) < 50)
				bottom->velocity[1] += v[1] * vslide;
			top->velocity[0] = -bottom->velocity[0]/2;
			top->velocity[1] = -bottom->velocity[1]/2;
			other->think     = box_water_friction;
			other->nextthink = level.time + 0.2;
			gi.linkentity(other);
			self->think      = box_water_friction;
			self->nextthink  = level.time + 0.2;
		}
		else
		{
			// Frictionless horizontal motion for 1 second
			self->think = box_water_friction;
			self->nextthink = level.time + 1.0;
		}

		// Override oldvelocity
		VectorCopy(self->velocity,self->oldvelocity);
		gi.linkentity(self);
		return;
	}
	// if other is a monster or a player and box is on other's head and moving down,
	// do impact damage
	VectorAdd(self->s.origin,self->origin_offset,origin);
	if( other->client || (other->svflags & SVF_MONSTER) )
	{
		VectorAdd (self->absmax,self->absmin,v1);
		VectorScale(v1,0.5,v1);
		VectorSubtract(v1,other->s.origin,v);
		VectorNormalize(v);
		axis = 0;
		if(fabs(v[1]) > fabs(v[axis])) axis = 1;
		if(fabs(v[2]) > fabs(v[axis])) axis = 2;
		if(axis == 2 && v[axis] > 0) {
			v11 = VectorLength(self->velocity);
			VectorCopy(self->velocity,v);
			VectorNormalize(v);
			if(!other->groundentity)
			{
				other->velocity[2] = self->velocity[2];
				gi.linkentity(other);
			}
			else if((v11 > 0) && (v[2] < -0.7))
			{
				int		damage;
				float	delta;
				vec3_t	dir, impact_v;
				VectorSubtract(other->velocity,self->velocity,impact_v);
				delta = VectorLength(impact_v);
				delta = delta*delta*0.001;
				damage = (float)(self->mass)/(float)(other->mass) * delta;
				// always do some minimum amount of damage, or we get boxes on
				// heads, which looks awfully damn odd
				damage = max(2,damage);
				VectorSubtract(self->s.origin,other->s.origin,dir);
				VectorNormalize(dir);
				T_Damage (other, self, self, dir, other->s.origin, vec3_origin, damage, 0, 0, MOD_CRUSH);
				self->bounce_me = 1;
				return;
			}
		}
		else if( (other->groundentity == self) && (self->velocity[2] > 0))
		{
			self->bounce_me = 2;
			other->velocity[2] = self->velocity[2];
			gi.linkentity(other);
			return;
		}
	}

	// if not a player return
	if (!other->client) return;

	// if func_pushable is on ground and has a mass > 1000, go away
	if (self->groundentity && self->mass > 1000) return;

	// if player not pressing use key, do nothing
	if (!other->client->use)
	{
		if (self->activator == other) self->activator = NULL;
		return;
	}

	// if player in contact with this func_pushable is already pushing
	// something else, return
	if ((other->client->push != NULL) && (other->client->push != self))
	{
		if(self->activator == other) self->activator = NULL;
		return;
	}

	// if another player got here first, he maintains control
	if (self->activator)
	{
		if (self->activator->client)
		{
			if (self->activator != other)
			{
				return;
			}
		}
	}

	VectorAdd (self->absmax,self->absmin,v1);
	VectorScale(v1,0.5,v1);

	// if func_pushable isn't in front of pusher, do nothing
	if (!point_infront(other,v1))
	{
		if(self->activator == other) self->activator = NULL;
		return;
	}

	// if player isn't on solid ground AND object isn't in water,
	// OR if player is standing on this box, do nothing
	if ( ((!other->groundentity) && (self->waterlevel==0)) || (other->groundentity == self) )
	{
		if(self->activator == other) self->activator = NULL;
		return;
	}

	// if this box has another box stacked on top, balk
	if( CrateOnTop (NULL, self) )
	{
		if(self->activator == other) self->activator = NULL;
		return;
	}

	// Give object a little nudge to give us some clearance
	VectorSubtract (v1, other->s.origin, v);
	box_walkmove (self, vectoyaw(v), 4);
	// Now get the offset from the player to the object, 
	//   and preserve that offset in ClientThink by shifting
	//   object as needed.
	VectorSubtract(other->s.origin,self->s.origin,self->offset);
	self->offset[2] = 0;
	self->activator = other;
	other->client->push = self;
	ratio = (float)other->mass / (float)self->mass;
	other->client->maxvelocity = 20. * ratio;
	// Knightmare- don't autoswitch if client-side chasecam is on
	if(tpp_auto->value && (!cl_thirdperson->value || deathmatch->value || coop->value)
		&& !other->client->chasetoggle && !other->client->chaseactive)
	{
	//	Cmd_Chasecam_Toggle(other);
		ChasecamStart (other);
		other->client->chasetoggle = 0;
	}

// test crap
//	self->s.renderfx = RF_TRANSLUCENT;
//	gi.linkentity(self);


}

void func_pushable_spawn (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid     = SOLID_BSP;
	self->movetype  = MOVETYPE_PUSHABLE;
	self->svflags  &= ~SVF_NOCLIENT;
	self->use       = box_use;
	self->clipmask  = MASK_PLAYERSOLID|MASK_MONSTERSOLID;
	self->touch     = box_touch;
}

void SP_func_pushable (edict_t *self)
{
//	vec3_t border = {2,2,2};
	vec3_t border = {1,1,1};

	self->class_id = ENTITY_FUNC_PUSHABLE;

	//Knightmare- precache different gib types
	PrecacheDebris(self->gib_type);

	gi.setmodel (self, self->model);

	// usermodel (for alias model pushables) goes on modelindex2
	if (self->usermodel && strlen(self->usermodel)) {
		char	modelname[256];
		// check for "models/" already in path
		if ( !strncmp(self->usermodel, "models/", 7) )
			Com_sprintf(modelname, sizeof(modelname), "%s", self->usermodel);
		else
			Com_sprintf(modelname, sizeof(modelname), "models/%s", self->usermodel);
		self->s.modelindex2 = gi.modelindex (modelname);
	}

	// Game places a 2 unit border around brush model absmin and absmax
	VectorAdd(self->mins,border,self->mins);
	VectorSubtract(self->maxs,border,self->maxs);
	VectorAdd(self->absmin,border,self->absmin);
	VectorSubtract(self->absmax,border,self->absmax);

	if (!self->mass)
		self->mass = 400;

	if (st.item) //Knightmare- item support
	{
		self->item = FindItemByClassname (st.item);
		if (!self->item)
			gi.dprintf("%s at %s has bad item: %s\n", self->classname, vtos(self->s.origin), st.item);
	}

	self->flags = FL_SWIM;

	if (self->health > 0)
	{
		self->die = box_die;
		self->takedamage = DAMAGE_YES;
	}
	else
	{
		self->die = NULL;
		self->takedamage = DAMAGE_NO;
	}

	if (self->spawnflags & 2)
	{
		// trigger spawn
		self->solid    = SOLID_NOT;
		self->movetype = MOVETYPE_NONE;
		self->use      = func_pushable_spawn;
		self->svflags |= SVF_NOCLIENT;
	}
	else
	{
		self->solid     = SOLID_BSP;
		self->movetype  = MOVETYPE_PUSHABLE;
		self->use       = box_use;
		self->clipmask  = MASK_PLAYERSOLID|MASK_MONSTERSOLID;
		self->touch     = box_touch;
		self->think     = M_droptofloor;
		self->nextthink = level.time + 2 * FRAMETIME;
	}

	if (self->spawnflags & 4)
		self->flags |= FL_NO_KNOCKBACK;

	switch (self->sounds)
	{
	case 1:
		self->noise_index = gi.soundindex ("tank/thud.wav");
		break;
	case 2:
		self->noise_index = gi.soundindex ("weapons/rg_hum.wav");
		break;
	case 3:
		self->noise_index = gi.soundindex ("weapons/rockfly.wav");
		break;
	}

	if(self->sounds && !VectorLength(self->s.origin) )
	{
		edict_t *speaker;

		speaker = G_Spawn();
		speaker->classname   = "moving_speaker";
		speaker->s.sound     = 0;
		speaker->volume      = 1;
		speaker->attenuation = ATTN_STATIC; // was 1
		speaker->owner       = self;
		speaker->noise_index = self->noise_index;
		speaker->think       = moving_speaker_think;
		speaker->nextthink   = level.time + 2*FRAMETIME;
		speaker->spawnflags  = 11;       // owner must be moving and on ground to play
		self->speaker        = speaker;
		VectorAdd(self->absmin,self->absmax,speaker->s.origin);
		VectorScale(speaker->s.origin,0.5,speaker->s.origin);
		VectorSubtract(speaker->s.origin, self->s.origin, speaker->offset);
	//	gi.dprintf ("moving_speaker offset %s from pushable\n", vtos(speaker->offset));
	}

	// Move up 1 unit so that func_pushable sitting directly on a func_train
	// won't block it's movement (since train has a 1 unit border)
	self->s.origin[2] += 1;
	gi.linkentity (self);

}

/*QUAKED func_door_swinging (0 .5 .9) ? IGNORE REVOLVE CRUSHER NO_MONSTER ANIMATED TOGLLE X_AXIS Y_AXIS
A 2-way rotating door, identical to func_door_rotating,
but will always open AWAY from whoever opens it

Spawnflags:
IGNORE: (Ignore Activator) determines what side of the door the activator is on is disabled.
REVOLVING: The door will never close, but instead can be repeatedly triggered to rotate distance degrees again, in either direction.

"distance"		deg/rotation (default=90)
"dmg"			damage when blocked (default=2)
"health"		if activated by shooting at it
"followtarget"	info_notnull opp. origin
"killtarget"	entity to attack
"message"		to print when trigger is inactive
"movewith"		make it part of a train or other mover
"pathtarget"	Targetname of the entity whose origin the func_door_swinging is to use as a spawning location at runtime. Normally this locating entity should be an info_notnull.
"sounds"
     0 = Audible (default)
     1 = Silent
"speed"			in degrees/sec door swings (Default=100)
"target"		Targetname of the entity to be triggered when the door opens. If the target is a func_areaportal, the areaportal will be triggered again when the door closes.
"targetname"	Name of the specific door. A door with a targetname cannot be opened without another triggering entity which targets it, but it can be shot (if health>0).
"team"			Team name of the specific door. Doors with a identical team names will move together, and will all stop if one is blocked.
"turn_rider"	If non-zero, a player or other entity riding on the door will rotate as the door rotates. (Default=0)
"wait"			If REVOLVING is not set, this is the time in seconds for the door to wait before returning to its closed position. If wait=-1, the door will never return. For non-REVOLVING doors, the default=3. If REVOLVING is set, this is the time in seconds to wait before the door will accept another trigger input after it completes its move. If wait=-1, the door cannot be triggered again. For REVOLVING doors, the default=0.
*/

//
// func_door_swinging is identical to func_door_rotating, but will always open AWAY
// from whoever opens it. If opened by a trigger the normal func_door_rotating rotation
// is used.
//
void swinging_door_killed (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t	*ent;
	edict_t	*master;

	for (ent = self->teammaster ; ent ; ent = ent->teamchain)
	{
		ent->health = ent->max_health;
		ent->takedamage = DAMAGE_NO;
	}
	master = self->teammaster;

	if (master->spawnflags & DOOR_TOGGLE)
	{
		if (master->moveinfo.state == STATE_UP || master->moveinfo.state == STATE_TOP)
		{
			// trigger all paired doors
			for (ent = master ; ent ; ent = ent->teamchain)
			{
				ent->message = NULL;
				ent->touch = NULL;
				door_go_down (ent);
			}
			return;
		}
	}
	
	// trigger all paired doors
	for (ent = master ; ent ; ent = ent->teamchain)
	{
		ent->message = NULL;
		ent->touch = NULL;
		ent->do_not_rotate = 1;
		if (ent->moveinfo.state == STATE_UP)
			continue;		// already going up
		if (ent->moveinfo.state == STATE_TOP)
		{	// reset top wait time
			if (ent->moveinfo.wait >= 0)
				ent->nextthink = level.time + ent->moveinfo.wait;
			return;
		}
		check_reverse_rotation(ent,point);
		if (!(ent->flags & FL_TEAMSLAVE))
		{
			if (ent->moveinfo.sound_start)
				gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ent->attenuation, 0); // was ATTN_STATIC
			ent->s.sound = ent->moveinfo.sound_middle;
	#ifdef LOOP_SOUND_ATTENUATION
			ent->s.attenuation = ent->attenuation;
	#endif
		}
		ent->moveinfo.state = STATE_UP;
		AngleMove_Calc (ent, door_hit_top);
		G_UseTargets (ent, attacker);
		door_use_areaportals (ent, true);
	}
}

void func_door_swinging_init (edict_t *self)
{
	edict_t		*new_origin;
	edict_t		*follow;

	follow = G_Find (NULL, FOFS(targetname), self->followtarget);
	if(!follow)
	{
		gi.dprintf("func_door_swinging at %s, followtarget not found\n",vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}
	VectorSubtract(follow->s.origin,self->s.origin,self->move_origin);
	VectorNormalize(self->move_origin);
	G_FreeEdict(follow);
	if(self->pathtarget)
	{
		new_origin = G_Find (NULL, FOFS(targetname), self->pathtarget);
		if(new_origin)
		{
			VectorCopy (new_origin->s.origin,self->s.origin);
			VectorCopy (self->s.origin, self->moveinfo.start_origin);
			VectorCopy (self->s.origin, self->moveinfo.end_origin);
			gi.linkentity(self);
		}
	}
	self->nextthink = level.time + FRAMETIME;
	if (self->health || self->targetname)
		self->think = Think_CalcMoveSpeed;
	else
		self->think = Think_SpawnDoorTrigger;
}
void SP_func_door_swinging (edict_t *self)
{
	int	pivot;

	self->class_id = ENTITY_FUNC_PIVOT;

	pivot = self->spawnflags & 1;	// 1 means "start open" for normal doors, so turn it
	self->spawnflags &= ~1;			// off temporarily until normal door initialization
									// is done

	if(self->spawnflags & DOOR_REVERSE)
	{
		self->spawnflags &= ~DOOR_REVERSE;
		self->flags |= FL_REVOLVING;
	}
	if(!self->followtarget)
	{
		gi.dprintf("func_door_swinging with no followtarget at %s\n",vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}
	SP_func_door_rotating (self);
	self->spawnflags |= pivot;
	if( pivot && (self->health > 0) )
		self->die = swinging_door_killed;

	self->postthink = train_move_children; //supports movewith

	self->flags |= FL_REVERSIBLE;
//	strcpy(self->classname, "func_door_rotating");
	self->classname = "func_door_rotating";

	// Wait a few frames so that we're sure pathtarget has been parsed.
	self->think = func_door_swinging_init;
	self->nextthink = level.time + 2*FRAMETIME;
	gi.linkentity(self);
}


/*QUAKED func_bobbingwater (0 .5 .8) ? START_ON MUD
Moveable, bobbing water
The water only bobs when triggered. When allowed to "close" on its own, it will not bob during movement to its original position. If you want the func_bobbingwater to bob at both ends of its motion, then don't set its wait to >=0. Instead, use wait=-1 and re-trigger it to "close"

START_OPEN causes the water to move to its destination when spawned and operate in reverse..
MUD        turns the water to mud - difficult to move through, swallows players


"angle" Direction the bobbingwater will move on the XY plane. Default=0.
"angles" Direction the bobbingwater will move in 3 dimensions, specified by pitch and yaw. Roll is ignored. Syntax is pitch yaw 0. Default=0 0 0.
"speed" in units/second that the bobbingwater moves. Default=25.
"targetname" Name of the specific bobbingwater.
"team" name of the specific bobbingwater. Bobbingwaters with a identical team names will move together, and if solid, will all stop if one is blocked. A func_bobbingwater may be teamed with a func_water.
"wait" in seconds for the bobbingwater to wait before it returns to its "closed" position. If wait=-1, the bobbingwater must be retriggered in order for it to close. This will also result in the bob effect taking place on both ends of the bobbingwater's motion, rather than just its "opening" motion. Default=-1
"wait" Number of seconds until reset. Continuous = -1
"lip" Lip units visible after move
"sounds" Default= 1
   0 :No Sounds
   1 :Water (Default)
   2 :Lava
"bob" Amplitude of motion" Default= 16
"duration" Time between bobbing cycles. Default= 8
*/

//
// Bobbing water - identical to func_water, but bobs up and down with
// amplitude specified by "bob" value and duration of one cycle specified by
// "duration"
//
void bob_think (edict_t *self)
{
	float	delta;
	int 	time = self->duration*10;
	float	t1, t0, z0, z1;

	t0 = self->bobframe%time;
	t1 = (self->bobframe+1)%time;

	z0 = sin(2*M_PI*t0/time);
	z1 = sin(2*M_PI*t1/time);
	delta = self->bob/2 * (z1-z0);
	self->velocity[2] = delta/FRAMETIME;
	self->nextthink = level.time + FRAMETIME;
	self->bobframe  = (self->bobframe+1)%time;
	gi.linkentity(self);
}

void bob_init (edict_t *self)
{
	self->bobframe  = 0;
	self->think     = bob_think;
	self->nextthink = level.time + FRAMETIME;
}

void SP_func_bobbingwater(edict_t *self)
{
	vec3_t	abs_movedir;

	if (!VectorLength(self->movedir))
		G_SetMovedir (self->s.angles, self->movedir);
	self->movetype = MOVETYPE_PUSH;
	self->solid = SOLID_BSP;
	gi.setmodel (self, self->model);

	if(self->spawnflags & 2)
	{
		level.mud_puddles++;
		self->svflags |= SVF_MUD;
	}

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

	if (self->lip)
		st.lip = self->lip;
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
	self->flags |= FL_BOB;

	if(!self->bob) self->bob = 16;
	if(!self->duration) self->duration = 8;
	self->think     = bob_init;
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
}

//
// func_pivot - works like a see-saw
//
void pivot_blocked (edict_t *self, edict_t *other)
{
	VectorCopy(vec3_origin,self->avelocity);
	gi.linkentity(self);
}

void pivot_stop(edict_t *ent)
{
	VectorClear(ent->avelocity);
	gi.linkentity(ent);
}

void pivot_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float	time;
	vec3_t	offset;
	vec3_t	avelocity;
	vec3_t	delta;

	if(!other->mass) return;               // other is weightless
	if(!other->groundentity) return;       // other is in air
	if(other->groundentity != ent) return; // other is not standing on ent

	VectorSubtract(ent->s.origin,other->s.origin,offset);
	offset[2] = 0.; // z offset is irrelevant
	VectorCopy(ent->avelocity,avelocity);
	if(ent->spawnflags & 1)
	{
		avelocity[PITCH] = -other->mass*offset[0]/400;
//		if(avelocity[PITCH] = ent->avelocity[PITCH]) return;
		if(offset[0] > 0)
			ent->move_angles[PITCH] = ent->pos2[PITCH];
		else
			ent->move_angles[PITCH] = ent->pos1[PITCH];
		VectorSubtract(ent->move_angles,ent->s.angles,delta);
		time = delta[PITCH]/avelocity[PITCH];
	}
	else
	{
		avelocity[ROLL]  = other->mass*offset[1]/400;
//		if(avelocity[ROLL] = ent->avelocity[ROLL]) return;
		if(offset[1] > 0)
			ent->move_angles[ROLL] = ent->pos1[ROLL];
		else
			ent->move_angles[ROLL] = ent->pos2[ROLL];
		VectorSubtract(ent->move_angles,ent->s.angles,delta);
		time = delta[ROLL]/avelocity[ROLL];
	}
	gi.dprintf("time=%f, v=%f %f %f\n",time,avelocity[0],avelocity[1],avelocity[2]);
	if(time > 0)
	{
		VectorCopy(avelocity,ent->avelocity);
		ent->think = pivot_stop;
		ent->nextthink = level.time + time;
		gi.linkentity(ent);
	}
	else
	{
		VectorClear(ent->avelocity);
		ent->nextthink = 0;
	}
}
void pivot_init (edict_t *ent)
{
	float	zmin;
	trace_t	tr;
	vec3_t	start, end;

	VectorClear(ent->pos1);
	VectorClear(ent->pos2);
	if(ent->spawnflags & 1)
	{
		end[0] = start[0] = ent->absmin[0];
		end[1] = start[1] = (ent->absmin[1]+ent->absmax[1])/2;
		start[2] = ent->absmin[2];
		end[2]   = ent->absmin[2] - (ent->s.origin[0]-ent->absmin[0]);
		tr=gi.trace(start,NULL,NULL,end,ent,MASK_SOLID);
		if(tr.fraction < 1.0)
			zmin = tr.endpos[2];
		else
			zmin = end[2];
		ent->pos2[PITCH] = asin((ent->absmin[2]-zmin)/(ent->s.origin[0]-ent->absmin[0]));

		end[0] = start[0] = ent->absmax[0];
		end[2] = ent->absmin[2] - (ent->absmax[0]-ent->s.origin[0]);
		tr=gi.trace(start,NULL,NULL,end,ent,MASK_SOLID);
		if(tr.fraction < 1.0)
			zmin = tr.endpos[2];
		else
			zmin = end[2];
		ent->pos1[PITCH] = asin((ent->absmin[2]-zmin)/(ent->absmax[0]-ent->s.origin[0]));

		ent->pos1[PITCH] *= 180/M_PI;
		ent->pos2[PITCH] *= -180/M_PI;
	}
	else
	{
		end[0] = start[0] = (ent->absmin[0]+ent->absmax[0])/2;
		end[1] = start[1] = ent->absmin[1];
		start[2] = ent->absmin[2];
		end[2]   = ent->absmin[2] - (ent->s.origin[1]-ent->absmin[1]);
		tr=gi.trace(start,NULL,NULL,end,ent,MASK_SOLID);
		if(tr.fraction < 1.0)
			zmin = tr.endpos[2];
		else
			zmin = end[2];
		ent->pos1[ROLL] = asin((ent->absmin[2]-zmin)/(ent->s.origin[1]-ent->absmin[1]));

		end[1] = start[1] = ent->absmax[1];
		end[2] = ent->absmin[2] - (ent->absmax[1]-ent->s.origin[1]);
		tr=gi.trace(start,NULL,NULL,end,ent,MASK_SOLID);
		if(tr.fraction < 1.0)
			zmin = tr.endpos[2];
		else
			zmin = end[2];
		ent->pos2[ROLL] = asin((ent->absmin[2]-zmin)/(ent->absmax[1]-ent->s.origin[1]));

		ent->pos1[ROLL] *= 180/M_PI;
		ent->pos2[ROLL] *= -180/M_PI;
	}

	VectorClear(ent->move_angles);
	gi.linkentity(ent);
}

void SP_func_pivot (edict_t *ent)
{
	ent->solid = SOLID_BSP;
	ent->movetype = MOVETYPE_PUSH;

	if (!ent->speed)
		ent->speed = 100;
	if (!ent->dmg)
		ent->dmg = 2;

	ent->touch = pivot_touch;
	ent->blocked = pivot_blocked;
	ent->gravity = 0;
	ent->think = pivot_init;
	ent->nextthink = level.time + FRAMETIME;
	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);
}
