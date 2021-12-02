
#include "g_local.h"




float Lerp(float value1, float value2, float amount)
{
    float delta = value2 - value1;
    return value1 + amount * delta;
}

int civwalk_SetPathData(edict_t *self)
{
	vec3_t v;
	vec3_t facingVec;

	VectorSubtract (self->s.origin, self->target_ent->s.origin, v);
	self->moveinfo.distance = VectorLength(v);

	//this is the 0-1 transition.
	self->moveinfo.remaining_distance = 0;

	//this is the step to increase each frame.
	self->moveinfo.accel =  self->speed / self->moveinfo.distance;

	VectorCopy(self->s.origin, self->pos1 );
	VectorCopy(self->target_ent->s.origin, self->pos2 );

	//random path offset.
	self->pos2[0] += -48 + random() * 96;
	self->pos2[1] += -48 + random() * 96;

	//turn to face the next path.

	VectorSubtract (self->pos2, self->pos1, facingVec);
	vectoangles(facingVec,facingVec);
	self->s.angles[YAW] = facingVec[YAW];

	if (self->target_ent->spawnflags & 1)
	{
		self->s.event = EV_OTHER_TELEPORT; //so the ent teleports over w/o interpolation.

		self->moveinfo.remaining_distance = 1;		
		VectorCopy(self->pos2, self->s.origin);		
		VectorCopy(self->pos2, self->s.old_origin);
		return 0;
	}

	return 1;
}

void civwalk_think(edict_t *self)
{
	vec3_t finalPos;

	self->think = civwalk_think;
	self->nextthink = level.time + FRAMETIME;

	self->moveinfo.remaining_distance += self->moveinfo.accel;
	if (self->moveinfo.remaining_distance >= 1)
	{
		if (!self->target_ent->target)
			return;

		//reached the path end. continue to next pathcorner.
		self->target_ent = G_Find(NULL, FOFS(targetname), self->target_ent->target);

		//no next target? then abort.
		if (self->target_ent == NULL)
			return;

		if (civwalk_SetPathData(self) <= 0)
			return;
	}

	//animation.
	self->s.frame++;
	if (self->s.frame >= self->startframe + self->framenumbers)
		self->s.frame = self->startframe;

	finalPos[0] = Lerp(self->pos1[0], self->pos2[0], self->moveinfo.remaining_distance);
	finalPos[1] = Lerp(self->pos1[1], self->pos2[1], self->moveinfo.remaining_distance);
	finalPos[2] = Lerp(self->pos1[2], self->pos2[2], self->moveinfo.remaining_distance);

	VectorCopy(finalPos, self->s.origin);
}



void SP_civwalker(edict_t *ent)
{
	ent->s.modelindex = gi.modelindex ("models/monsters/npc/tris.md2");

	ent->solid			= SOLID_NOT;
	ent->takedamage	= DAMAGE_NO;
	ent->movetype = MOVETYPE_NOCLIP;

	ent->s.renderfx |= RF_MINLIGHT;


	

	//TODO: select random skin.
	ent->s.skinnum = 13 + random() * 5;


	//BC baboo
	if (skill->value >= 2 )
	{
		ent->s.skinnum = 7;
	}

	//find target path.
	ent->target_ent = G_Find(NULL, FOFS(targetname), ent->target);
	civwalk_SetPathData(ent);



	ent->think = civwalk_think;
	ent->nextthink = level.time + FRAMETIME;
	gi.linkentity (ent);
}

void civspawner_spawnOne(edict_t *ent)
{
	edict_t *person;
	vec3_t spawnpos;
	vec3_t dest;
	trace_t tr;
	int width, height, length;

	width = abs(ent->maxs[0] - ent->mins[0]);
	length = abs(ent->maxs[1] - ent->mins[1]);
	height = abs(ent->maxs[2] - ent->mins[2]);


	//drop to ground.
	spawnpos[0] = ent->mins[0] + random() * width;
	spawnpos[1] = ent->mins[1] + random() * length;
	spawnpos[2] = ent->maxs[2];

	VectorSet( dest, spawnpos[0], spawnpos[1], spawnpos[2] - 4096 );
	tr = gi.trace (
			spawnpos,
			vec3_origin,
			vec3_origin,
			dest, //dest
			ent, //ignore
			MASK_SOLID);//MASK_SOLID

	//if in solid, then abort.
	if ( tr.startsolid )
		return;

	person = G_Spawn();

	//position.
	spawnpos[2] = tr.endpos[2];
	VectorCopy (spawnpos, person->s.origin);
	VectorCopy (spawnpos, person->s.old_origin);

	//target path.
	person->target = ent->target;

	//person animation.
	person->startframe = ent->startframe;
	person->framenumbers = ent->framenumbers;
	person->s.frame = person->startframe + random() * person->framenumbers;

	//random speed.
	person->speed = ent->speed  + random() * ent->random;

	SP_civwalker( person );
}

void civspawner_DoSpawn(edict_t *ent)
{
	int i;

	for ( i = 0 ; i < ent->count ; i++ )
	{
		civspawner_spawnOne(ent);
	}

	//remove this ent from world.
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + FRAMETIME;
	
}

void use_civspawner(edict_t *self)
{
	self->think = civspawner_DoSpawn;
	self->nextthink = level.time + FRAMETIME;
}

void SP_func_civspawner(edict_t *self)
{
	edict_t *pathent;

	if (!self->target)
	{
		gi.dprintf ("func_civspawner without a target at %s\n", vtos(self->absmin));
		G_FreeEdict (self);
		return;
	}

	pathent = G_Find(NULL, FOFS(targetname), self->target);
	if (pathent == NULL)
	{
		gi.dprintf ("func_civspawner can't find target %s\n", self->target);
		G_FreeEdict (self);
		return;
	}

	if (!self->speed)
		self->speed = 5;

	if (!self->count)
		self->count = 20;

	if (!self->startframe)
		self->startframe = 59;

	if (!self->framenumbers)
		self->framenumbers = 10;

	if (!self->random)
		self->random = 15;

	self->use = use_civspawner;

	self->svflags = SVF_NOCLIENT;
	self->solid = SOLID_NOT;
	gi.setmodel (self, self->model);	

	if (!(self->spawnflags & 1))
	{
		self->think = civspawner_DoSpawn;
		self->nextthink = level.time + 1;
	}
}