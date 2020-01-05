#include "g_local.h"

//==========================================================================================
//
// AimGrenade finds the correct aim vector to get a grenade from start to target at initial
// velocity = speed. Returns false if grenade can't make it to target.
//
//==========================================================================================
qboolean AimGrenade (edict_t *self, vec3_t start, vec3_t target, vec_t speed, vec3_t aim)
{
	vec3_t		angles, forward, right, up;
	vec3_t		from_origin, from_muzzle;
	vec3_t		aim_point;
	vec_t		xo, yo;
	vec_t		x;
	float		cosa, t, vx, y;
	float		drop;
	float		last_error, v_error;
	int			i;
	vec3_t		last_aim;

	VectorCopy(target,aim_point);
	VectorSubtract(aim_point,self->s.origin,from_origin);
	VectorSubtract(aim_point, start, from_muzzle);

	if(self->svflags & SVF_MONSTER)
	{
		VectorCopy(from_muzzle,aim);
		VectorNormalize(aim);
		yo = from_muzzle[2];
		xo = sqrt(from_muzzle[0]*from_muzzle[0] + from_muzzle[1]*from_muzzle[1]);
	}
	else
	{
		VectorCopy(from_origin,aim);
		VectorNormalize(aim);
		yo = from_origin[2];
		xo = sqrt(from_origin[0]*from_origin[0] + from_origin[1]*from_origin[1]);
	}

	// If resulting aim vector is looking straight up or straight down, we're 
	// done. Actually now that I write this down and think about it... should
	// probably check straight up to make sure grenade will actually reach the
	// target.
	if( (aim[2] == 1.0) || (aim[2] == -1.0))
		return true;

	// horizontal distance to target from muzzle
	x = sqrt( from_muzzle[0]*from_muzzle[0] + from_muzzle[1]*from_muzzle[1]);
	cosa = sqrt(aim[0]*aim[0] + aim[1]*aim[1]);
	// constant horizontal velocity (since grenades don't have drag)
	vx = speed * cosa;
	// time to reach target x
	t = x/vx;
	// if flight time is less than one frame, no way grenade will drop much,
	// shoot the sucker now.
	if(t < FRAMETIME)
		return true;
	// in that time, grenade will drop this much:
	drop = 0.5*sv_gravity->value*t*t;
	y = speed*aim[2]*t - drop;
	v_error = target[2] - start[2] - y;

	// if we're fairly close and we'll hit target at current angle,
	// no need for all this, just shoot it
	if( (x < 128) && (fabs(v_error) < 16) )
		return true;

	last_error = 100000.;
	VectorCopy(aim,last_aim);

	// Unfortunately there is no closed-form solution for this problem,
	// so we creep up on an answer and balk if it takes more than 
	// 10 iterations to converge to the tolerance we'll accept.
	for(i=0; i<10 && fabs(v_error) > 4 && fabs(v_error) < fabs(last_error); i++)
	{
		last_error = v_error;
		aim[2] = cosa * (yo + drop)/xo;
		VectorNormalize(aim);
		if(!(self->svflags & SVF_MONSTER))
		{
			vectoangles(aim,angles);
			AngleVectors(angles, forward, right, up);
			G_ProjectSource2(self->s.origin,self->move_origin,forward,right,up,start);
			VectorSubtract(aim_point,start,from_muzzle);
			x = sqrt(from_muzzle[0]*from_muzzle[0] + from_muzzle[1]*from_muzzle[1]);
		}
		cosa = sqrt(aim[0]*aim[0] + aim[1]*aim[1]);
		vx = speed * cosa;
		t = x/vx;
		drop = 0.5*sv_gravity->value*t*t;
		y = speed*aim[2]*t - drop;
		v_error = target[2] - start[2] - y;
		if(fabs(v_error) < fabs(last_error))
			VectorCopy(aim,last_aim);
	}
	
	if(i >= 10 || v_error > 64)
		return false;
	if(fabs(v_error) > fabs(last_error))
	{
		VectorCopy(last_aim,aim);
		if(!(self->svflags & SVF_MONSTER))
		{
			vectoangles(aim,angles);
			AngleVectors(angles, forward, right, up);
			G_ProjectSource2(self->s.origin,self->move_origin,forward,right,up,start);
			VectorSubtract(aim_point,start,from_muzzle);
		}
	}
	
	// Sanity check... if launcher is at the same elevation or a bit above the 
	// target entity, check to make sure he won't bounce grenades off the 
	// top of a doorway or other obstruction. If he WOULD do that, then figure out 
	// the max elevation angle that will get the grenade through the door, and 
	// hope we get a good bounce.
	if( (start[2] - target[2] < 160) &&
		(start[2] - target[2] > -16)   )
	{
		trace_t	tr;
		vec3_t	dist;
		
		tr = gi.trace(start,vec3_origin,vec3_origin,aim_point,self,MASK_SOLID);
		if( (tr.fraction < 1.0) && (!self->enemy || (tr.ent != self->enemy) )) {
			// OK... the aim vector hit a solid, but would the grenade actually hit?
			int		contents;
			cosa = sqrt(aim[0]*aim[0] + aim[1]*aim[1]);
			vx = speed * cosa;
			VectorSubtract(tr.endpos,start,dist);
			dist[2] = 0;
			x = VectorLength(dist);
			t = x/vx;
			drop = 0.5*sv_gravity->value*t*(t+FRAMETIME);
			tr.endpos[2] -= drop;
			// move just a bit in the aim direction
			tr.endpos[0] += aim[0];
			tr.endpos[1] += aim[1];
			contents = gi.pointcontents(tr.endpos);
			while((contents & MASK_SOLID) && (aim_point[2] > target[2])) {
				aim_point[2] -= 8.0;
				VectorSubtract(aim_point,self->s.origin,from_origin);
				VectorCopy(from_origin,aim);
				VectorNormalize(aim);
				if(!(self->svflags & SVF_MONSTER))
				{
					vectoangles(aim,angles);
					AngleVectors(angles, forward, right, up);
					G_ProjectSource2(self->s.origin,self->move_origin,forward,right,up,start);
					VectorSubtract(aim_point,start,from_muzzle);
				}
				tr = gi.trace(start,vec3_origin,vec3_origin,aim_point,self,MASK_SOLID);
				if(tr.fraction < 1.0) {
					cosa = sqrt(aim[0]*aim[0] + aim[1]*aim[1]);
					vx = speed * cosa;
					VectorSubtract(tr.endpos,start,dist);
					dist[2] = 0;
					x = VectorLength(dist);
					t = x/vx;
					drop = 0.5*sv_gravity->value*t*(t+FRAMETIME);
					tr.endpos[2] -= drop;
					tr.endpos[0] += aim[0];
					tr.endpos[1] += aim[1];
					contents = gi.pointcontents(tr.endpos);
				}
			}
		}
	}
	return true;
}
