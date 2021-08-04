/*
==============================================================================

TURRET

==============================================================================
*/

#include "g_local.h"
#include "m_turret.h"

#define SPAWN_BLASTER			8
#define SPAWN_RAILGUN			16
#define SPAWN_ROCKET			32
#define SPAWN_HEATBEAM			64
#define SPAWN_WEAPONCHOICE		120
#define SPAWN_INSTANT_WEAPON	80
#define SPAWN_WALL_UNIT			128

#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3
#define STATE_LOWEST		4

extern qboolean FindTarget (edict_t *self);

void turret_run (edict_t *self);
void TurretAim (edict_t *self);
void turret_sight (edict_t *self, edict_t *other);
void turret_search (edict_t *self);
void turret_stand (edict_t *self);
void turret_wake (edict_t *self);
void turret_ready_gun (edict_t *self);
void turret_run (edict_t *self);

void turret_attack (edict_t *self);
mmove_t turret_move_fire;
mmove_t turret_move_fire_blind;


void TurretAim (edict_t *self)
{
	vec3_t		end, dir;
	vec3_t		ang;
	float		move, idealPitch, idealYaw, current, speed, turn_yaw=0.0f;
	int			orientation;
	qboolean	is_spinning = false;
	qboolean	is_turning = false;

//	gi.dprintf("turret_aim: %d %d\n", self->s.frame, self->monsterinfo.nextframe);

	if (self->movewith_set && self->movewith_ent && self->movewith_ent->inuse
		&& (!strcmp(self->movewith_ent->classname, "func_rotating")
			|| !strcmp(self->movewith_ent->classname, "func_rotating_dh")
			|| !strcmp(self->movewith_ent->classname, "func_trackchange") )
			&& (self->movewith_ent->avelocity[YAW])) {
		is_spinning = true;
		turn_yaw = self->movewith_ent->avelocity[YAW] * FRAMETIME;
	}

	if (self->movewith_set && self->movewith_ent && self->movewith_ent->inuse
		&& (!strcmp(self->movewith_ent->classname, "func_train")
			|| !strcmp(self->movewith_ent->classname, "func_tracktrain")
			|| !strcmp(self->movewith_ent->classname, "model_train")
			|| !strcmp(self->movewith_ent->classname, "func_door_rotating")
			|| !strcmp(self->movewith_ent->classname, "func_door_swinging") )
			&& (self->movewith_ent->avelocity[YAW])) {
		is_turning = true;
		turn_yaw = self->movewith_ent->avelocity[YAW] * FRAMETIME;
	}

	if (!self->enemy || self->enemy == world)
	{
		if (!FindTarget (self))
			return;
	}

	if (!self->enemy->classname)
		return;
	if (!strcmp(self->enemy->classname, "gib") || !strcmp(self->enemy->classname, "debris"))
		return;

	// if turret is still in inactive mode, ready the gun, but don't aim
	if (self->s.frame < FRAME_active01)
	{
		turret_ready_gun (self);
		return;
	}
	// if turret is still readying, don't aim.
	if (self->s.frame < FRAME_run01)
		return;

	// PMM - blindfire aiming here
	if (self->monsterinfo.currentmove == &turret_move_fire_blind)	// && !is_spinning)
	{
		VectorCopy (self->monsterinfo.blind_fire_target, end);
		if (self->enemy->s.origin[2] < self->monsterinfo.blind_fire_target[2])
			end[2] += self->enemy->viewheight + 10;
		else
			end[2] += self->enemy->mins[2] - 10;
	}
	else
	{
		VectorCopy (self->enemy->s.origin, end);
		if (self->enemy->client)
			end[2] += self->enemy->viewheight;
	}

	VectorSubtract (end, self->s.origin, dir);
	vectoangles2 (dir, ang);

	//
	// Clamp first
	//

	idealPitch = ang[PITCH];
	idealYaw = ang[YAW];

	orientation = self->offset[1];
	switch (orientation)
	{
		case -1:			// up		pitch: 0 to 90
			if (idealPitch < -90)
				idealPitch += 360;
			if (idealPitch > -5)
				idealPitch = -5;
			break;

		case -2:			// down		pitch: -180 to -360
			if (idealPitch > -90)
				idealPitch -= 360;
			if (idealPitch < -355)
				idealPitch = -355;
			else if (idealPitch > -185)
				idealPitch = -185;
			break;

 		// Knightmare- support odd yaw angles
		default:
			if (idealPitch < -180)
				idealPitch += 360;

			if (idealPitch > 85)
				idealPitch = 85;
			else if (idealPitch < -85)
				idealPitch = -85;
		
			if ( idealYaw > (orientation+180) )
				idealYaw -= 360;
			if ( idealYaw < (orientation-180) )
				idealYaw += 360;
		
			if (is_turning)	// Knightmare- factor in avelocity of parent entity
			{
			//	gi.dprintf("turret_aim: clamping yaw for turning parent entity\n");
				if ( idealYaw > (orientation + turn_yaw + 85) )
					idealYaw = orientation + turn_yaw + 85;
				else if ( idealYaw < (orientation + turn_yaw - 85) )
					idealYaw = orientation + turn_yaw - 85;
			}
			else if (is_spinning)	// Knightmare- if out of range, point to front
			{
			//	gi.dprintf("turret_aim: clamping yaw for spinning parent entity\n");
				if ( idealYaw > (orientation + 85) )
					idealYaw = orientation;
				else if ( idealYaw < (orientation - 85) )
					idealYaw = orientation;
			}
			else
			{
				if ( idealYaw > (orientation + 85) )
					idealYaw = orientation + 85;
				else if ( idealYaw < (orientation - 85) )
					idealYaw = orientation - 85;
			}
			break;
		// end Knightmare
	}

	//
	// adjust pitch
	//
	current = self->s.angles[PITCH];
	speed = self->yaw_speed;

	if (idealPitch != current)
	{
		move = idealPitch - current;

		while (move >= 360)
			move -= 360;
		if (move >= 90)
		{
			move = move - 360;
		}

		while (move <= -360)
			move += 360;
		if (move <= -90)
		{
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

		self->s.angles[PITCH] = anglemod (current + move);
	}

	//
	// adjust yaw
	//
	current = self->s.angles[YAW];
	speed = self->yaw_speed;

	if (idealYaw != current)
	{
		move = idealYaw - current;

		if (move >= 180) 
			move -= 360;

		if (move <= -180)
			move += 360;

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
		self->s.angles[YAW] = anglemod (current + move);
	}

}

void turret_sight (edict_t *self, edict_t *other)
{
}

void turret_search (edict_t *self)
{
}

mframe_t turret_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t turret_move_stand = {FRAME_stand01, FRAME_stand02, turret_frames_stand, NULL};

void turret_stand (edict_t *self)
{
//	gi.dprintf("turret_stand\n");
	self->monsterinfo.currentmove = &turret_move_stand;
}

mframe_t turret_frames_ready_gun [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	
	ai_stand, 0, NULL
};
mmove_t turret_move_ready_gun = { FRAME_active01, FRAME_run01, turret_frames_ready_gun, turret_run };

void turret_ready_gun (edict_t *self)
{
	self->monsterinfo.currentmove = &turret_move_ready_gun;
}

mframe_t turret_frames_seek [] =
{
	ai_walk, 0, TurretAim,
	ai_walk, 0, TurretAim
};
mmove_t turret_move_seek = {FRAME_run01, FRAME_run02, turret_frames_seek, NULL};

void turret_walk (edict_t *self)
{
	if (self->s.frame < FRAME_run01)
		turret_ready_gun(self);
	else
		self->monsterinfo.currentmove = &turret_move_seek;
}


mframe_t turret_frames_run [] =
{
	ai_run, 0, TurretAim,
	ai_run, 0, TurretAim
};
mmove_t turret_move_run = {FRAME_run01, FRAME_run02, turret_frames_run, turret_run};

void turret_run (edict_t *self)
{
	if (self->s.frame < FRAME_run01)
		turret_ready_gun(self);
	else
		self->monsterinfo.currentmove = &turret_move_run;
}

// **********************
//  ATTACK
// **********************

#define TURRET_BULLET_DAMAGE	4
#define TURRET_HEAT_DAMAGE		4


void Turret_Railgun_Fire (edict_t *self)
{
	vec3_t	start, dir;

	TurretAim (self);

	VectorCopy (self->s.origin, start);
	// trail the target....
//	VectorMA(end, -0.2, self->enemy->velocity, end);
	VectorSubtract (self->aim_point, start, dir);
	VectorNormalize(dir);

#ifdef KMQUAKE2_ENGINE_MOD
	monster_fire_railgun (self, start, dir, 50, 0, MZ2_TURRET_RAILGUN_1);	// new muzzleflash for KMQ2
#else
	monster_fire_railgun (self, start, dir, 50, 0, MZ2_GLADIATOR_RAILGUN_1);
#endif	// KMQUAKE2_ENGINE_MOD

	self->think = monster_think;
	self->nextthink = level.time + FRAMETIME;
}

void Turret_Railgun_Aim (edict_t *self)
{
	vec3_t	end;

	TurretAim (self);

	if (!self->enemy || !self->enemy->inuse)
		return;

	VectorCopy (self->enemy->s.origin, end);
	// aim for the head.
	if ((self->enemy) && (self->enemy->client))
		end[2]+=self->enemy->viewheight;
	else
		end[2]+=22;
	VectorCopy (end, self->aim_point);	// save for aiming the shot

	self->think = Turret_Railgun_Fire;
	self->nextthink = level.time + 2 * FRAMETIME;
}

void TurretFire (edict_t *self)
{
	vec3_t	forward;
	vec3_t	start, end, dir;
	float	time, dist, chance;
	trace_t	trace;
	int		rocketSpeed;

	TurretAim (self);

	if (!self->enemy || !self->enemy->inuse)
		return;

	VectorSubtract (self->enemy->s.origin, self->s.origin, dir);
	VectorNormalize(dir);
	AngleVectors(self->s.angles, forward, NULL, NULL);
	chance = DotProduct(dir, forward);
	if (chance < 0.98)
	{
	//	gi.dprintf("off-angle\n");
		return;
	}

	chance = random();

	// rockets fire less often than the others do.
	if (self->spawnflags & SPAWN_ROCKET)
	{
		chance = chance * 3;

		rocketSpeed = 550;
		if (skill->value == 2)
		{
			rocketSpeed += 200 * random();
		}
		else if (skill->value == 3)
		{
			rocketSpeed += 100 + (200 * random());
		}
	}
	else if (self->spawnflags & SPAWN_BLASTER)
	{
		if (skill->value == 0)
			rocketSpeed = 600;
		else if (skill->value == 1)
			rocketSpeed = 800;
		else
			rocketSpeed = 1000;
		chance = chance * 2;
	}
	
	// up the fire chance 20% per skill level.
	chance = chance - (0.2 * skill->value);

	if (/*chance < 0.5 && */visible(self, self->enemy))
	{
		VectorCopy (self->s.origin, start);
		VectorCopy (self->enemy->s.origin, end);
		
		// aim for the head.
		if ((self->enemy) && (self->enemy->client))
			end[2] += self->enemy->viewheight;
		else
			end[2] += 22;

		// Lazarus fog reduction of accuracy
		if (self->monsterinfo.visibility < FOG_CANSEEGOOD)
		{
			end[0] += crandom() * 640 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
			end[1] += crandom() * 640 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
			end[2] += crandom() * 320 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
		}

		VectorSubtract (end, start, dir);
		dist = VectorLength(dir);
		
		// check for predictive fire if distance less than 512
		if (!(self->spawnflags & SPAWN_INSTANT_WEAPON) && (dist<512))
		{
			chance = random();
			// ramp chance easy - 50%, med - 60%, hard - 70%, nightmare - 80%
			chance += (3 - skill->value) * 0.1;
			if (chance < 0.8)
			{
				// lead the target....
				time = dist / 1000;
				VectorMA(end, time, self->enemy->velocity, end);

				// Lazarus fog reduction of accuracy
				if (self->monsterinfo.visibility < FOG_CANSEEGOOD)
				{
					end[0] += crandom() * 640 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
					end[1] += crandom() * 640 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
					end[2] += crandom() * 320 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
				}

				VectorSubtract (end, start, dir);
			}
		}

		VectorNormalize(dir);
		trace = gi.trace(start, vec3_origin, vec3_origin, end, self, MASK_SHOT);
		if (trace.ent == self->enemy || trace.ent == world || (self->monsterinfo.visibility < FOG_CANSEEGOOD))
		{
			if (self->spawnflags & SPAWN_BLASTER)
				monster_fire_blaster (self, start, dir, 20, rocketSpeed, MZ2_TURRET_BLASTER, EF_BLASTER, BLASTER_ORANGE);
			else if (self->spawnflags & SPAWN_RAILGUN && self->last_fire_time <= level.time)	// was SPAWN_MACHINEGUN
			{
			//	monster_fire_bullet (self, start, dir, TURRET_BULLET_DAMAGE, 0, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ2_TURRET_MACHINEGUN);
				gi.sound (self, CHAN_WEAPON, gi.soundindex ("gladiator/railgun.wav"), 1, ATTN_NORM, 0);
				self->think = Turret_Railgun_Aim;
				self->nextthink = level.time + 2 * FRAMETIME;
				self->last_fire_time = level.time + 1.5;
			}
			else if (self->spawnflags & SPAWN_ROCKET)
			{
				if (dist * trace.fraction > 72)
					monster_fire_rocket (self, start, dir, 50, rocketSpeed, MZ2_TURRET_ROCKET, NULL);
			}
		}	
	}
}

// PMM
void TurretFireBlind (edict_t *self)
{
	vec3_t	forward;
	vec3_t	start, end, dir;
	float	dist, chance;
	int		rocketSpeed;

	TurretAim (self);

	if (!self->enemy || !self->enemy->inuse)
		return;

	VectorSubtract (self->monsterinfo.blind_fire_target, self->s.origin, dir);
	VectorNormalize(dir);
	AngleVectors(self->s.angles, forward, NULL, NULL);
	chance = DotProduct(dir, forward);
	if (chance < 0.98)
	{
//		gi.dprintf("off-angle\n");
		return;
	}

	if (self->spawnflags & SPAWN_ROCKET)
	{
		rocketSpeed = 550;
		if (skill->value == 2)
		{
			rocketSpeed += 200 * random();
		}
		else if (skill->value == 3)
		{
			rocketSpeed += 100 + (200 * random());
		}
	}

	VectorCopy (self->s.origin, start);
	VectorCopy (self->monsterinfo.blind_fire_target, end);
		
	if (self->enemy->s.origin[2] < self->monsterinfo.blind_fire_target[2])
		end[2] += self->enemy->viewheight + 10;
	else
		end[2] += self->enemy->mins[2] - 10;

	VectorSubtract (end, start, dir);
	dist = VectorLength(dir);
		
	VectorNormalize(dir);

	if (self->spawnflags & SPAWN_BLASTER)
		monster_fire_blaster(self, start, dir, 20, 1000, MZ2_TURRET_BLASTER, EF_BLASTER, BLASTER_ORANGE);
	else if (self->spawnflags & SPAWN_ROCKET)
		monster_fire_rocket (self, start, dir, 50, rocketSpeed, MZ2_TURRET_ROCKET, NULL);
}
//pmm

mframe_t turret_frames_fire [] =
{
	ai_run,   0, TurretFire,
	ai_run,   0, TurretAim,
	ai_run,   0, TurretAim,
	ai_run,   0, TurretAim
};
mmove_t turret_move_fire = {FRAME_pow01, FRAME_pow04, turret_frames_fire, turret_run};

//PMM

// the blind frames need to aim first
mframe_t turret_frames_fire_blind [] =
{
	ai_run,   0, TurretAim,
	ai_run,   0, TurretAim,
	ai_run,   0, TurretAim,
	ai_run,   0, TurretFireBlind
};
mmove_t turret_move_fire_blind = {FRAME_pow01, FRAME_pow04, turret_frames_fire_blind, turret_run};
//pmm

void turret_attack(edict_t *self)
{
	float r, chance;

	if (self->s.frame < FRAME_run01)
		turret_ready_gun(self);
	// PMM
	else if (self->monsterinfo.attack_state != AS_BLIND)
	{
		self->monsterinfo.nextframe = FRAME_pow01;
		self->monsterinfo.currentmove = &turret_move_fire;
	}
	else
	{
		// setup shot probabilities
		if (self->monsterinfo.blind_fire_delay < 1.0)
			chance = 1.0;
		else if (self->monsterinfo.blind_fire_delay < 7.5)
			chance = 0.4;
		else
			chance = 0.1;

		r = random();

		// minimum of 3 seconds, plus 0-4, after the shots are done - total time should be max less than 7.5
		self->monsterinfo.blind_fire_delay += 0.4 + 3.0 + random()*4.0;
		// don't shoot at the origin
		if (VectorCompare (self->monsterinfo.blind_fire_target, vec3_origin))
			return;

		// don't shoot if the dice say not to
		if (r > chance)
			return;

		self->monsterinfo.nextframe = FRAME_pow01;
		self->monsterinfo.currentmove = &turret_move_fire_blind;
	}
	// pmm
}

// **********************
//  PAIN
// **********************

void turret_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	return;
}

// **********************
//  DEATH
// **********************

void turret_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t		forward;
	vec3_t		start;
	edict_t		*base;

	gi.WriteByte (svc_temp_entity);
	//gi.WriteByte (TE_PLAIN_EXPLOSION);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, 1, forward, start);

	ThrowDebris (self, "models/objects/debris1/tris.md2", 1, start, 0, 0, 0);
	ThrowDebris (self, "models/objects/debris1/tris.md2", 2, start, 0, 0, 0);
	ThrowDebris (self, "models/objects/debris1/tris.md2", 1, start, 0, 0, 0);
	ThrowDebris (self, "models/objects/debris1/tris.md2", 2, start, 0, 0, 0);

	if (self->teamchain)
	{
		base = self->teamchain;
		base->solid = SOLID_BBOX;
		base->takedamage = DAMAGE_NO;
		base->movetype = MOVETYPE_NONE;
		gi.linkentity (base);
	}

	if (self->target)
	{
		if (self->enemy && self->enemy->inuse)
			G_UseTargets (self, self->enemy);
		else
			G_UseTargets (self, self);
	}

	G_FreeEdict(self);
}

// **********************
//  WALL SPAWN
// **********************

void turret_wall_spawn (edict_t *turret)
{
	edict_t		*ent;
	int			angle;
	vec3_t		forward;

	ent = G_Spawn();
	ent->classname = "turret_wall";
	ent->class_id = ENTITY_TURRET_WALL;
	VectorCopy (turret->s.origin, ent->s.origin);
	VectorCopy (turret->s.angles, turret->deploy_angles);
	VectorCopy (turret->s.angles, ent->s.angles);
	VectorCopy (ent->s.angles, ent->deploy_angles);

	angle = ent->s.angles[1];
	if (ent->s.angles[0] == 90)
		angle = -1;
	else if (ent->s.angles[0] == 270)
		angle = -2;
	switch (angle)
	{
		case -1:
			VectorSet(ent->mins, -16, -16, -8);
			VectorSet(ent->maxs, 16, 16, 0);
			break;
		case -2:
			VectorSet(ent->mins, -16, -16, 0);
			VectorSet(ent->maxs, 16, 16, 8);
			break;
		case 0:
			VectorSet(ent->mins, -8, -16, -16);
			VectorSet(ent->maxs, 0, 16, 16);
			break;
		case 90:
			VectorSet(ent->mins, -16, -8, -16);
			VectorSet(ent->maxs, 16, 0, 16);
			break;
		case 180:
			VectorSet(ent->mins, 0, -16, -16);
			VectorSet(ent->maxs, 8, 16, 16);
			break;
		case 270:
			VectorSet(ent->mins, -16, 0, -16);
			VectorSet(ent->maxs, 16, 8, 16);
			break;
		default:
			if (angle > 315 || angle <= 45)
			{
				VectorSet(ent->mins, -8, -16, -16);
				VectorSet(ent->maxs, 0, 16, 16);
			}
			else if (angle > 45 && angle <= 135)
			{
				VectorSet(ent->mins, -16, -8, -16);
				VectorSet(ent->maxs, 16, 0, 16);
			}
			else if (angle > 135 && angle <= 225)
			{
				VectorSet(ent->mins, 0, -16, -16);
				VectorSet(ent->maxs, 8, 16, 16);
			}
			else if (angle > 225 && angle <= 315)
			{
				VectorSet(ent->mins, -16, 0, -16);
				VectorSet(ent->maxs, 16, 8, 16);
			}
			break;
	}

/*	if (turret->s.angles[0] == 270)
		VectorSet (forward, 0, 0, 1);
	else if (turret->s.angles[0] == 90)
		VectorSet (forward, 0, 0, -1);
	else if (turret->s.angles[1] == 0)
		VectorSet (forward, 1, 0, 0);
	else if (turret->s.angles[1] == 90)
		VectorSet (forward, 0, 1, 0);
	else if (turret->s.angles[1] == 180)
		VectorSet (forward, -1, 0, 0);
	else if (turret->s.angles[1] == 270)
		VectorSet (forward, 0, -1, 0);
*/
	// Knightmare- set up the turret's movement positions here
	AngleVectors (turret->s.angles, forward, NULL, NULL);
	// Move turret wall backward a bit so it will be flush with surrounding wall
	VectorMA (ent->s.origin, -2, forward, ent->s.origin);

	VectorCopy (turret->s.origin, turret->pos1);
	VectorMA (turret->pos1, 32, forward, turret->pos2);
	VectorCopy (ent->s.origin, ent->pos1);
	VectorMA (ent->pos1, 32, forward, ent->pos2);

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_NOT;
	ent->moveinfo.state = STATE_BOTTOM;

	ent->teammaster = turret;
	turret->teammaster = turret;
	turret->teamchain = ent;
	turret->moveinfo.state = STATE_BOTTOM;

	ent->teamchain = NULL;
	ent->flags |= FL_TEAMSLAVE;
	ent->owner = turret;

	// Knightmare- set movewith if applicable and backup original angles
	if (turret->movewith)
		ent->movewith = turret->movewith;

	VectorCopy (ent->s.angles, ent->org_angles);

	ent->s.modelindex = gi.modelindex("models/monsters/turretbase/tris.md2");
	ent->s.renderfx |= RF_NOSHADOW;	// Knightmare added

	gi.linkentity (ent);
}

void turret_wake (edict_t *self)
{
	// The wall section will call this when it stops moving.
	// just return without doing anything. Easiest way to have a null function.
	self->moveinfo.state = STATE_TOP;

	if (self->flags & FL_TEAMSLAVE)
		return;

	self->monsterinfo.stand = turret_stand;
	self->monsterinfo.walk = turret_walk;
	self->monsterinfo.run = turret_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = turret_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = turret_sight;
	self->monsterinfo.search = turret_search;
	self->monsterinfo.currentmove = &turret_move_stand;
	self->takedamage = DAMAGE_AIM;
	self->movetype = MOVETYPE_NONE;
	// prevent counting twice
	//self->monsterinfo.aiflags |= AI_DO_NOT_COUNT;
	self->monsterinfo.monsterflags |= MFL_DO_NOT_COUNT;

	gi.linkentity (self);

	stationarymonster_start (self);
	
	if (self->spawnflags & SPAWN_RAILGUN) //was SPAWN_MACHINEGUN
		self->s.skinnum = 1;
	else if (self->spawnflags & SPAWN_ROCKET)
		self->s.skinnum = 2;

	// but we do want the death to count
	//self->monsterinfo.aiflags &= ~AI_DO_NOT_COUNT;
	self->monsterinfo.monsterflags &= ~MFL_DO_NOT_COUNT;
}

extern void Move_Calc (edict_t *ent, vec3_t dest, void(*func)(edict_t*));

void turret_activate (edict_t *self, edict_t *other, edict_t *activator)
{
//	vec3_t		endpos;
	edict_t		*base;

	self->movetype = MOVETYPE_PUSH;
	self->use = NULL; // Knightmare added
	if (!self->speed)
		self->speed = 15;
	self->moveinfo.speed = self->speed;
	self->moveinfo.accel = self->speed;
	self->moveinfo.decel = self->speed;

	// start up the turret
	self->moveinfo.state = STATE_UP;
	Move_Calc(self, self->pos2, turret_wake);

	base = self->teamchain;
	if (base)
	{
		base->movetype = MOVETYPE_PUSH;
		base->speed = self->speed;
		base->moveinfo.speed = base->speed;
		base->moveinfo.accel = base->speed;
		base->moveinfo.decel = base->speed;

		// start up the wall section
		self->teamchain->moveinfo.state = STATE_UP;
		Move_Calc(self->teamchain, self->teamchain->pos2, turret_wake);
	}

	gi.sound (self, CHAN_VOICE, gi.soundindex ("world/dr_short.wav"), 1, ATTN_NORM, 0);
}

// PMM
// checkattack .. ignore range, just attack if available
qboolean turret_checkattack (edict_t *self)
{
	vec3_t	spot1, spot2;
	float	chance, nexttime;
	trace_t	tr;
	int		enemy_range;

	if (self->enemy->health > 0)
	{
	// see if any entities are in the way of the shot
		VectorCopy (self->s.origin, spot1);
		spot1[2] += self->viewheight;
		VectorCopy (self->enemy->s.origin, spot2);
		spot2[2] += self->enemy->viewheight;

		tr = gi.trace (spot1, NULL, NULL, spot2, self, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_SLIME|CONTENTS_LAVA|CONTENTS_WINDOW);

		// do we have a clear shot?
		if (tr.ent != self->enemy)
		{	
			// PGM - we want them to go ahead and shoot at info_notnulls if they can.
			if (self->enemy->solid != SOLID_NOT || tr.fraction < 1.0)		//PGM
			{
				// PMM - if we can't see our target, and we're not blocked by a monster, go into blind fire if available
				if ((!(tr.ent->svflags & SVF_MONSTER)) && (!visible(self, self->enemy)))
				{
					if ((self->monsterinfo.blindfire) && (self->monsterinfo.blind_fire_delay <= 10.0))
					{
						if (level.time < self->monsterinfo.attack_finished)
						{
							return false;
						}
						if (level.time < (self->monsterinfo.trail_time + self->monsterinfo.blind_fire_delay))
						{
							// wait for our time
							return false;
						}
						else
						{
							// make sure we're not going to shoot something we don't want to shoot
							tr = gi.trace (spot1, NULL, NULL, self->monsterinfo.blind_fire_target, self, CONTENTS_MONSTER);
							if (tr.allsolid || tr.startsolid || ((tr.fraction < 1.0) && (tr.ent != self->enemy)))
							{
								return false;
							}

							self->monsterinfo.attack_state = AS_BLIND;
							self->monsterinfo.attack_finished = level.time + 0.5 + 2*random();
							return true;
						}
					}
				}
				// pmm
				return false;
			}
		}
	}
	
	if (level.time < self->monsterinfo.attack_finished)
		return false;

	enemy_range = range(self, self->enemy);

	if (enemy_range == RANGE_MELEE)
	{
		// don't always melee in easy mode
		if (skill->value == 0 && (rand()&3) )
			return false;
		self->monsterinfo.attack_state = AS_MISSILE;
		return true;
	}
	
	if (self->spawnflags & SPAWN_ROCKET)
	{
		chance = 0.10;
		nexttime = (1.8 - (0.2 * skill->value));
	}
	else if (self->spawnflags & SPAWN_BLASTER)
	{
		chance = 0.35;
		nexttime = (1.2 - (0.2 * skill->value));
	}
	else
	{
		chance = 0.50;
		nexttime = (0.8 - (0.1 * skill->value));
	}

	if (skill->value == 0)
		chance *= 0.5;
	else if (skill->value > 1)
		chance *= 2;

	// PGM - go ahead and shoot every time if it's a info_notnull
	// PMM - added visibility check
	if ( ((random () < chance) && (visible(self, self->enemy))) || (self->enemy->solid == SOLID_NOT))
	{
		self->monsterinfo.attack_state = AS_MISSILE;
//		self->monsterinfo.attack_finished = level.time + 0.3 + 2*random();
		self->monsterinfo.attack_finished = level.time + nexttime;
		return true;
	}

	self->monsterinfo.attack_state = AS_STRAIGHT;

	return false;
}


// **********************
//  SPAWN
// **********************

/*QUAKED monster_turret (1 .5 0) (-16 -16 -16) (16 16 16) Ambush Trigger_Spawn Sight Blaster RailGun Rocket Heatbeam WallUnit

The automated defense turret that mounts on walls. 
Check the weapon you want it to use: blaster, railgun, rocket, heatbeam.
Default weapon is blaster.
When activated, wall units move 32 units in the direction they're facing.
*/
void SP_monster_turret (edict_t *self)
{
	int		angle;

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	// VERSIONING
//	if (g_showlogic && g_showlogic->value)
//		gi.dprintf ("%s\n", ROGUE_VERSION_STRING);

//	self->plat2flags = ROGUE_VERSION_ID;
	// versions

	// pre-caches
	gi.soundindex ("world/dr_short.wav");
	gi.modelindex ("models/objects/debris1/tris.md2");


	if (self->style)
	{
		PatchMonsterModel("models/monsters/turret/tris.md2");
		self->s.skinnum = self->style * 3;
	}

	self->s.modelindex = gi.modelindex("models/monsters/turret/tris.md2");
	self->s.renderfx |= RF_NOSHADOW; // Knightmare added

	VectorSet (self->mins, -12, -12, -12);
	VectorSet (self->maxs, 12, 12, 12);
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;

	if (!self->health)
		self->health = 240;
	if (!self->gib_health)
		self->gib_health = -100;
	if (!self->mass)
		self->mass = 250;
	if (!self->yaw_speed)
		self->yaw_speed = 45;

	self->flags |= FL_MECHANICAL;

	self->pain = turret_pain;
	self->die = turret_die;

	// map designer didn't specify weapon type. set it now.
	if (!(self->spawnflags & SPAWN_WEAPONCHOICE))
	{
		self->spawnflags |= SPAWN_BLASTER;
	//	self->spawnflags |= SPAWN_MACHINEGUN;
	//	self->spawnflags |= SPAWN_ROCKET;
	//	self->spawnflags |= SPAWN_HEATBEAM;
	}

	if (self->spawnflags & SPAWN_HEATBEAM)
	{
		self->spawnflags &= ~SPAWN_HEATBEAM;
		self->spawnflags |= SPAWN_BLASTER;
	}

	if (!(self->spawnflags & SPAWN_WALL_UNIT))
	{
		self->monsterinfo.stand = turret_stand;
		self->monsterinfo.walk = turret_walk;
		self->monsterinfo.run = turret_run;
		self->monsterinfo.dodge = NULL;
		self->monsterinfo.attack = turret_attack;
		self->monsterinfo.melee = NULL;
		self->monsterinfo.sight = turret_sight;
		self->monsterinfo.search = turret_search;
		self->monsterinfo.currentmove = &turret_move_stand;
	}

	// PMM
	self->monsterinfo.checkattack = turret_checkattack;

	self->monsterinfo.aiflags |= AI_MANUAL_STEERING;
	self->monsterinfo.scale = MODEL_SCALE;
	self->gravity = 0;
	self->last_fire_time = 0;

	VectorCopy (self->s.angles, self->offset);

	angle = (int)self->s.angles[1];
	switch (angle)
	{
		case -1:					// up
			self->s.angles[0] = 270;
			self->s.angles[1] = 0;
			self->s.origin[2] += 2;
			break;
		case -2:					// down
			self->s.angles[0] = 90;
			self->s.angles[1] = 0;
			self->s.origin[2] -= 2;
			break;
		case 0:
			self->s.origin[0] += 2;
			break;
		case 90:
			self->s.origin[1] += 2;
			break;
		case 180:
			self->s.origin[0] -= 2;
			break;
		case 270:
			self->s.origin[1] -= 2;
			break;
		default:
			if (angle > 338 || angle <= 22) // near 0
				self->s.origin[0] += 2;
			else if (angle > 22 && angle <= 68) { // near 45
				self->s.origin[0] += 1;
				self->s.origin[1] += 1; }
			else if (angle > 68 && angle <= 112) // near 90
				self->s.origin[1] += 2;
			else if (angle > 112 && angle <= 158) { // near 135
				self->s.origin[1] += 1;
				self->s.origin[0] -= 1; }
			else if (angle > 158 && angle <= 202) // near 180
				self->s.origin[0] -= 2;
			else if (angle > 202 && angle <= 248) { // near 225
				self->s.origin[0] -= 1;
				self->s.origin[1] -= 1; }
			else if (angle > 248 && angle <= 292) // near 270
				self->s.origin[1] -= 2;
			else if (angle > 292 && angle <= 338) { // near 315
				self->s.origin[1] -= 1;
				self->s.origin[0] += 1; }
			break;
	}

	gi.linkentity (self);


	if (self->spawnflags & SPAWN_WALL_UNIT)
	{
	//	vec3_t		forward;

		if (!self->targetname)
		{
		//	gi.dprintf("Wall Unit Turret without targetname! %s\n", vtos(self->s.origin));
			G_FreeEdict(self);
			return;
		}
		self->takedamage = DAMAGE_NO;
		self->use = turret_activate;
		turret_wall_spawn(self);
		//if ((!(self->monsterinfo.aiflags & AI_GOOD_GUY)) && (!(self->monsterinfo.aiflags & AI_DO_NOT_COUNT)))
		if ((!(self->monsterinfo.aiflags & AI_GOOD_GUY)) && (!(self->monsterinfo.monsterflags & MFL_DO_NOT_COUNT)))
			level.total_monsters++;
	}
	else
	{
		stationarymonster_start (self);
	}

	if (self->spawnflags & SPAWN_BLASTER)
	{
		gi.modelindex ("models/objects/laser/tris.md2");
		gi.soundindex ("misc/lasfly.wav");
		gi.soundindex ("soldier/solatck2.wav");
		self->spawnflags &= ~SPAWN_RAILGUN;
		self->spawnflags &= ~SPAWN_ROCKET;
		self->s.skinnum = 0;
	}
	else if (self->spawnflags & SPAWN_RAILGUN) //was SPAWN_MACHINEGUN
	{	
		//gi.soundindex ("infantry/infatck1.wav");
		gi.soundindex ("gladiator/railgun.wav");
		self->spawnflags &= ~SPAWN_ROCKET;
		self->s.skinnum = 1;
	}
	else if (self->spawnflags & SPAWN_ROCKET)
	{
		gi.soundindex ("weapons/rockfly.wav");
		gi.modelindex ("models/objects/rocket/tris.md2");
		gi.soundindex ("chick/chkatck2.wav");
		self->s.skinnum = 2;
	}
	else
	{
		self->spawnflags |= SPAWN_BLASTER;
		gi.modelindex ("models/objects/laser/tris.md2");
		gi.soundindex ("misc/lasfly.wav");
		gi.soundindex ("soldier/solatck2.wav");
		self->s.skinnum = 0;
	}

	// Lazarus
	if (self->powerarmor)
	{
		if (self->powerarmortype == 1)
			self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
		else
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		self->monsterinfo.power_armor_power = self->powerarmor;
	}
	self->common_name = "Sentry Turret";
	self->class_id = ENTITY_MONSTER_TURRET;

	// PMM  - turrets don't get mad at monsters, and visa versa
	self->monsterinfo.aiflags |= AI_IGNORE_SHOTS;
	// PMM - blindfire
	if (self->spawnflags & (SPAWN_ROCKET|SPAWN_BLASTER))
		self->monsterinfo.blindfire = true;
}
