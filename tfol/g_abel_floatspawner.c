
#include "g_local.h"

#define MAXFLOATERCOUNT  30

int GetFloaterCount(edict_t *self)
{
	int total;
	edict_t *item;

	total = 0;
	item = NULL;
	item = G_Find(NULL, FOFS(classname), "monster_floater");

	while (item)
	{
		if (item->id != 777)
			continue;

		total++;

		item = G_Find(item, FOFS(classname), "monster_floater");
	}

	return total;
}

void floatspawner_spawnone(edict_t *self)
{
	int width, length, height;
	vec3_t spawnpos;
	edict_t *person;

	if (GetFloaterCount(self) >= MAXFLOATERCOUNT)
		return;

	width = abs(self->maxs[0] - self->mins[0]);
	length = abs(self->maxs[1] - self->mins[1]);
	height = abs(self->maxs[2] - self->mins[2]);

	//drop to ground.
	spawnpos[0] = self->mins[0] + random() * width;
	spawnpos[1] = self->mins[1] + random() * length;
	spawnpos[2] = self->mins[2] + random() * height;


	person = G_Spawn();

	VectorCopy (spawnpos, person->s.origin);
	VectorCopy (spawnpos, person->s.old_origin);
	person->s.angles[YAW] = random() * 359;
	person->classname = "monster_floater";	
	
	person->id = 777;

	SP_monster_floater( person );

	person->svflags |= SVF_DEADMONSTER;
}

void think_floatspawner(edict_t *self)
{
	if (self->count <= 0)
		return;

	floatspawner_spawnone( self );

	self->think = think_floatspawner;
	self->nextthink = level.time + 0.5;
}

void use_floatspawner(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->count <= 0)
	{
		self->think = think_floatspawner;
		self->nextthink = level.time + FRAMETIME;
		self->count = 1;
	}
	else
	{
		self->count = 0;
	}
}

void SP_func_floatspawner(edict_t *self)
{
	self->use = use_floatspawner;
	self->svflags = SVF_NOCLIENT;
	self->solid = SOLID_NOT;

	gi.setmodel (self, self->model);
}