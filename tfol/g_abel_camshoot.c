#include "g_local.h"

// =================== TARGET_CAMSHOOT

#define CAMBULLET_SPEED    512

void FireTarget2(edict_t *ent)
{
	edict_t		*t;

	if (ent->target2)
	{
		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), ent->target2)))
		{
			// doors fire area portals in a specific way
			if (!Q_stricmp(t->classname, "func_areaportal") &&
				(!Q_stricmp(ent->classname, "func_door") || !Q_stricmp(ent->classname, "func_door_rotating") 
				/*DWH*/ || !Q_stricmp(ent->classname,"func_door_rot_dh")))
				continue;

			if (t == ent)
			{
				gi.dprintf ("WARNING: Entity used itself.\n");
			}
			else
			{
				if (t->use)
					t->use (t, ent, ent);
			}
			if (!ent->inuse)
			{
				gi.dprintf("entity was removed while using targets\n");
				return;
			}
		}
	}
}

void FirePathTarget(edict_t *ent)
{
	edict_t		*t;

	if (ent->pathtarget)
	{
		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), ent->pathtarget)))
		{
			// doors fire area portals in a specific way
			if (!Q_stricmp(t->classname, "func_areaportal") &&
				(!Q_stricmp(ent->classname, "func_door") || !Q_stricmp(ent->classname, "func_door_rotating") 
				/*DWH*/ || !Q_stricmp(ent->classname,"func_door_rot_dh")))
				continue;

			if (t == ent)
			{
				gi.dprintf ("WARNING: Entity used itself.\n");
			}
			else
			{
				if (t->use)
				{
					t->use (t, ent, ent);
				}
			}
			if (!ent->inuse)
			{
				gi.dprintf("entity was removed while using targets\n");
				return;
			}
		}
	}
}

//this is the physical bullet model.
edict_t *CreateCamBullet(edict_t *self, edict_t *ent)
{
	edict_t *player;
	edict_t *gib;

	vec3_t forward, up, right, v, randAng;

	player = &g_edicts[1];	// Gotta be, since this is SP only

	gib = G_Spawn();

	gi.setmodel (gib, "models/objects/bullet/tris.md2");
	gib->solid = SOLID_NOT;

	VectorCopy(self->s.origin, gib->s.origin);
	//gib->s.origin[2] += 24;

	gib->movetype = MOVETYPE_FLY;



	if (ent->message)
	{
		edict_t *steerEnt;
		steerEnt = G_Find(NULL,FOFS(targetname), ent->message);

		if (steerEnt)
		{
			vec3_t angToSteerEnt;
			VectorSubtract (steerEnt->s.origin, gib->s.origin, angToSteerEnt);
			vectoangles(angToSteerEnt,angToSteerEnt);
			VectorCopy(angToSteerEnt, gib->s.angles);
		}

	}
	else
	{
		VectorCopy(player->client->ps.viewangles, gib->s.angles);
	}

	


	if (gib->s.angles[PITCH] > 0)
		gib->s.angles[PITCH] = 0;

	gib->s.angles[ROLL] = crandom() * 180;
	gib->avelocity[ROLL] = -512;
	
	AngleVectors(gib->s.angles, forward, NULL, up);

	VectorMA(gib->s.origin, -44.5, forward, gib->s.origin);	
	VectorMA(gib->s.origin, 1.8, up, gib->s.origin);	

	VectorScale (forward, CAMBULLET_SPEED, gib->velocity);
	
	gib->s.renderfx |= RF_MINLIGHT;

	gi.linkentity (gib);

	return gib;
}


float Lerp2(float value1, float value2, float amount)
{
    float delta = value2 - value1;
    return value1 + amount * delta;
}

void DeactivateCam(edict_t *self)
{
	edict_t *player;
	player = &g_edicts[1];	// Gotta be, since this is SP only	
	camera_off(player);	
}

void WarpToNextPoint(edict_t *self, edict_t *nextTarget);

void trackcam_think(edict_t *self)
{
	vec3_t finalPos;

	finalPos[0] = Lerp2(self->pos1[0], self->pos2[0], self->moveinfo.remaining_distance);
	finalPos[1] = Lerp2(self->pos1[1], self->pos2[1], self->moveinfo.remaining_distance);
	finalPos[2] = Lerp2(self->pos1[2], self->pos2[2], self->moveinfo.remaining_distance);

	VectorCopy(finalPos, self->s.origin);


	self->moveinfo.remaining_distance += self->moveinfo.accel;
	if (self->moveinfo.remaining_distance >= 1)
	{
		if (!self->target_ent->target2)
		{
			//fire any target2
			if (self->target_ent->target)
			{
				edict_t *aimpoint;

				aimpoint = G_Find(NULL,FOFS(targetname), self->target_ent->target);

				if (aimpoint)
				{
					FireTarget2(aimpoint);
				}
			}

			DeactivateCam(self);
			return;
		}
		else
		{
			edict_t *nextTarget;
			nextTarget = G_Find(NULL,FOFS(targetname), self->target_ent->target2);

			if (nextTarget)
			{
				FirePathTarget(nextTarget);

				WarpToNextPoint(self, nextTarget);
			}
			else
			{
				DeactivateCam(self);
				return;
			}
		}

		return;
	}

	self->think = trackcam_think;
	self->nextthink = level.time + FRAMETIME;
}

void WarpToNextPoint(edict_t *self, edict_t *nextTarget)
{
	self->target_ent = nextTarget;

	self->s.event = EV_OTHER_TELEPORT;	

	VectorClear(self->avelocity);
	VectorClear(self->velocity);
	VectorCopy(nextTarget->s.origin, self->s.old_origin);
	VectorCopy(nextTarget->s.origin, self->s.origin);
	VectorCopy(nextTarget->s.angles, self->s.angles);
	self->touch = NULL;

	if (nextTarget->move_angles)
	{
		self->avelocity[0] = nextTarget->move_angles[0];
		self->avelocity[1] = nextTarget->move_angles[1];
		self->avelocity[2] = nextTarget->move_angles[2];
	}

	

	if (!nextTarget->speed)
		self->speed = 16;
	else
		self->speed = nextTarget->speed;

	if (nextTarget->target)
	{
		edict_t *aimTarget;
		vec3_t  v;

		aimTarget = G_Find(NULL,FOFS(targetname), nextTarget->target);

		if (!aimTarget)
			return;
		
		VectorSubtract (nextTarget->s.origin, aimTarget->s.origin, v);
		self->moveinfo.distance = VectorLength(v);

		//this is the 0-1 transition.
		self->moveinfo.remaining_distance = 0;

		//this is the step to increase each frame.
		self->moveinfo.accel =  self->speed / self->moveinfo.distance;

		VectorCopy(nextTarget->s.origin, self->pos1 );
		VectorCopy(aimTarget->s.origin, self->pos2 );

		trackcam_think(self);
	}
	else
	{
		DeactivateCam(self);
	}
}




void camTrack_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t *player;
	edict_t *nextTarget;

	if (!plane)
		return;
	
	player = &g_edicts[1];	// Gotta be, since this is SP only

	//delete bullet model.
	G_FreeEdict(self->child);


	
	

	//hit wall.

	//if no next target, then abort.
	if (!self->owner->target)
	{
		camera_off(player);
		//fire targets.
		FireTarget2(self->owner);
		return;
	}

	//fire targets.
	FireTarget2(self->owner);

	nextTarget = G_Find(NULL,FOFS(targetname), self->owner->target);

	if (!nextTarget)
	{
		camera_off(player);
		return;
	}	

	FirePathTarget(nextTarget);
	WarpToNextPoint(self, nextTarget);
}

edict_t *camTrack(edict_t *self)
{
	edict_t *player;
	edict_t *gib;

	vec3_t forward, right, v, randAng;

	player = &g_edicts[1];	// Gotta be, since this is SP only

	gib = G_Spawn();

	gib->touch = camTrack_touch;

	//gi.setmodel (gib, self->usermodel);
	gib->solid = SOLID_TRIGGER;

	VectorCopy(player->s.origin, gib->s.origin);
	gib->s.origin[2] += 24;

	gib->movetype = MOVETYPE_FLY;	



	if (self->message)
	{
		edict_t *steerEnt;
		steerEnt = G_Find(NULL,FOFS(targetname), self->message);

		if (steerEnt)
		{
			vec3_t angToSteerEnt;
			VectorSubtract (steerEnt->s.origin, gib->s.origin, angToSteerEnt);
			vectoangles(angToSteerEnt,angToSteerEnt);
			VectorCopy(angToSteerEnt, gib->s.angles);
		}
	}
	else
	{
		VectorCopy(player->client->ps.viewangles, gib->s.angles);
	}
	
	
	if (gib->s.angles[PITCH] > 0)
		gib->s.angles[PITCH] = 0;

	gib->avelocity[ROLL] = 20; //camera roll.
	
	AngleVectors(gib->s.angles, forward, NULL, NULL);	

	VectorScale (forward, CAMBULLET_SPEED, gib->velocity);	
	
	gi.linkentity (gib);

	return gib;
}

void use_camshoot(edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *player;
	edict_t	*target;

	player = &g_edicts[1];	// Gotta be, since this is SP only

	if(!player->client)
		return;

	if(player->client->spycam)	// already using camera
		return;

	target = camTrack(self);

	if(!target) 
		return;	

	target->owner = self;
	player->client->ps.stats[STAT_USEABLE] = 0;
	player->client->ps.stats[STAT_HINTTITLE] = 1;
	VectorClear (player->velocity);

	if (!(self->spawnflags & 1))
	{
		target->child = CreateCamBullet(target, self);
	}

	player->client->spycam = target; 
	player->client->monitor = self;
	camera_on(player);

	
	//player->client->ps.pmove.pm_time = 160>>3;		// hold time
	//player->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	if (self->spawnflags & 1)
	{
		//skip the bulletcam, go straight to the tracks.
		edict_t *nextTarget;
		nextTarget = NULL;
		nextTarget = G_Find(NULL,FOFS(targetname), self->target);

		if (!nextTarget)
		{
			camera_off(player);
			return;
		}	

		FirePathTarget(nextTarget);
		WarpToNextPoint(target, nextTarget);
	}
}

void SP_target_camshoot(edict_t *self)
{
	self->use = use_camshoot;
	//self->svflags = SVF_NOCLIENT;
	gi.linkentity (self);
}