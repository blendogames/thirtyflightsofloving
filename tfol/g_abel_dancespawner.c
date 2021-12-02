
#include "g_local.h"


void dancer_think (edict_t *self)
{
	self->count++;

	if (self->count >= 30)
	{
		self->think = G_FreeEdict;		
		self->nextthink = level.time + FRAMETIME;

		self->svflags = SVF_NOCLIENT;

		return;
	}

	self->s.frame++;
	if (self->s.frame >= self->startframe + self->framenumbers)
		self->s.frame = self->startframe;

	//FLOAT.
	self->s.origin[2] += self->avelocity[2];

	//turn.
	self->s.angles[YAW] += 20;

	self->think = dancer_think;
	self->nextthink = level.time + FRAMETIME;

	self->svflags &= ~SVF_NOCLIENT;
}

void SP_dancer(edict_t *ent)
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

	ent->think = dancer_think;
	ent->nextthink = level.time + FRAMETIME;
	gi.linkentity (ent);
}



// ============================== DANCE SPAWNER

void dancespawner_spawnOne(edict_t *ent)
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


	if (random() < 0.5 && (ent->spawnflags & 2))
	{
		person->avelocity[2] = 2 + random() * 8;
		spawnpos[2] +=  random() * 16;
	}
	


	VectorCopy (spawnpos, person->s.origin);
	VectorCopy (spawnpos, person->s.old_origin);

	//person animation.
	person->startframe = ent->startframe;
	person->framenumbers = ent->framenumbers;
	person->s.frame = person->startframe + random() * person->framenumbers;

	person->s.angles[YAW] = random() * 359;

	person->svflags = SVF_NOCLIENT;

	


	SP_dancer( person );
}

void dancespawner_DoSpawn(edict_t *self)
{
	int i;

	if (self->density <= 0)
		return;

	for ( i = 0 ; i < self->count ; i++ )
	{
		dancespawner_spawnOne(self);
	}
	
	self->think = dancespawner_DoSpawn;
	self->nextthink = level.time + self->wait;	
}

void use_dancespawner(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->density <= 0)
	{
		self->density = 1;
		self->think = dancespawner_DoSpawn;
		self->nextthink = level.time + 1;
	}
	else
	{
		self->density = 0;
	}
}


void SP_func_dancespawner(edict_t *self)
{
	edict_t *pathent;

	if (!self->count)
		self->count = 4;

	if (!self->startframe)
		self->startframe = 330;

	if (!self->framenumbers)
		self->framenumbers = 19;

	if (!self->wait)
		self->wait = 0.2;

	self->use = use_dancespawner;

	self->svflags = SVF_NOCLIENT;
	self->solid = SOLID_NOT;
	gi.setmodel (self, self->model);	

	if (self->spawnflags & 1)
	{
		self->density = 1; //On.
		self->think = dancespawner_DoSpawn;
		self->nextthink = level.time + 1;
	}
}