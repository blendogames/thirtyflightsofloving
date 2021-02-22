// g_misc_nm.c
// misc entities for Neil Manke's Q2 maps

#include "g_local.h"

/*
=============================================================

Coconut Monkey 3 Flame entities

=============================================================
*/
#define FLAME_START_OFF	1
void hurt_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

void light_flame_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
	{
		self->svflags &= ~SVF_NOCLIENT;
		gi.configstring (CS_LIGHTS+self->style, "m");
		self->spawnflags &= ~FLAME_START_OFF;
		self->solid = SOLID_TRIGGER;
	}
	else
	{
		self->svflags |= SVF_NOCLIENT;
		gi.configstring (CS_LIGHTS+self->style, "a");
		self->spawnflags |= FLAME_START_OFF;
		self->solid = SOLID_NOT;
	}
	gi.linkentity (self);
}

void light_flame_spawn (edict_t *self)
{
	self->s.effects = EF_ANIM_ALLFAST;
	self->s.renderfx |= RF_NOSHADOW;
	self->movetype = MOVETYPE_NONE;
	self->touch = hurt_touch;
	if (!self->dmg)
		self->dmg = 5;

	self->noise_index = gi.soundindex ("world/electro.wav");

	if (self->style >= 32)
	{
		if (self->spawnflags & FLAME_START_OFF)
		{
			gi.configstring (CS_LIGHTS+self->style, "a");
			self->svflags |= SVF_NOCLIENT;
			self->solid = SOLID_NOT;

		}
		else
		{
			gi.configstring (CS_LIGHTS+self->style, "m");
			self->solid = SOLID_TRIGGER;
		}
	}
	self->use = light_flame_use;

	gi.linkentity (self);
}

void SP_light_flame1 (edict_t *self)
{
	self->class_id = ENTITY_LIGHT_FLAME_CM;

	self->s.modelindex = gi.modelindex ("sprites/s_flame1.sp2");
	VectorSet(self->mins,-48,-48,-32);
	VectorSet(self->maxs, 48, 48, 64);
	light_flame_spawn (self);
}

void SP_light_flame1s (edict_t *self)
{
	self->class_id = ENTITY_LIGHT_FLAME_CM;

	self->s.modelindex = gi.modelindex ("sprites/s_flame1s.sp2");
	VectorSet(self->mins,-16,-16,-16);
	VectorSet(self->maxs, 16, 16, 32);
	light_flame_spawn (self);
}

void SP_light_flame2 (edict_t *self)
{
	self->class_id = ENTITY_LIGHT_FLAME_CM;

	self->s.modelindex = gi.modelindex ("sprites/s_flame2.sp2");
	VectorSet(self->mins,-48,-48,-32);
	VectorSet(self->maxs, 48, 48, 64);
	light_flame_spawn (self);
}

void SP_light_flame2s (edict_t *self)
{
	self->class_id = ENTITY_LIGHT_FLAME_CM;

	self->s.modelindex = gi.modelindex ("sprites/s_flame2s.sp2");
	VectorSet(self->mins,-16,-16,-16);
	VectorSet(self->maxs, 16, 16, 32);
	light_flame_spawn (self);
}

/*
=============================================================

Coconut Monkey

=============================================================
*/

void monster_coco_monkey_think (edict_t *self)
{
	if (++self->s.frame > 19)
		self->s.frame = 0;
	self->nextthink = level.time + FRAMETIME;
}


void SP_monster_coco_monkey (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	self->movetype = MOVETYPE_TOSS;
	self->solid = SOLID_BBOX;

	self->s.modelindex = gi.modelindex ("models/monsters/coco/tris.md2");
	self->s.renderfx |= RF_IR_VISIBLE;
	VectorSet(self->mins,-16,-16,-40);
	VectorSet(self->maxs, 16, 16, 48);
	self->s.origin[2] += 10;

	self->nextthink = level.time + FRAMETIME;
	self->think = monster_coco_monkey_think;
	self->common_name = "Coconut Monkey";

	self->class_id = ENTITY_MONSTER_COCO_MONKEY;

	gi.linkentity (self);
}
