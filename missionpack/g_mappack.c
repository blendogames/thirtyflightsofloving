#include "g_local.h"

/*
=============================
Q1 Torch

=============================
*/
/*QUAKED light_torch (0 1 0) (-8 -8 -8) (8 8 8)
The torches from Quake 1 (no model yet)

"light" = The amount of light emitted from the torch
"style" = The style of the light : default 0
*/
void torch_think (edict_t *self)
{
	self->s.frame++;
	if (self->s.frame == 5)
		self->s.frame = 0;

	self->nextthink = level.time + FRAMETIME;
}

void SP_light_torch (edict_t *self) // New function by Beel.
{
	self->class_id = ENTITY_LIGHT_TORCH;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.modelindex = gi.modelindex ("models/torch/tris.md2");
	self->s.frame = 0;
	self->think = torch_think;
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
}

/*
=============================
Small flame

=============================
*/
/*QUAKED light_flame (0 1 0) (-8 -8 0) (8 8 16) START_OFF
Small flame from Q1
Default light value is 300.
Default style is 0.
model=models/objects/fire/"
*/
void smallflame_think (edict_t *self)
{
	if (self->s.frame >= 10) // was 5
		self->s.frame = 0;
	else
		self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}

void bigflame_think (edict_t *self)
{
	self->s.frame++;
	if (self->s.frame == 16)
		self->s.frame = 6;
	
	self->nextthink = level.time + FRAMETIME;
}

void SP_light_flame (edict_t *self) // New function by Beel. (small flame)
{
	self->class_id = ENTITY_LIGHT_FLAME;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	VectorSet (self->mins, -8, -8, 0);
	VectorSet (self->maxs, 8, 8, 16);
	self->s.modelindex = gi.modelindex ("models/objects/fire/tris.md2"); // was models/flame/tris.md2
	self->s.frame = 0;
	self->s.renderfx |=	RF_IR_VISIBLE | RF_FULLBRIGHT;		// PGM
	self->s.effects |= EF_PLASMA;
	// Knightmare- only one size for this model
	//ed - spawnflag of 2 gives a big flame
//	if (self->spawnflags & 2)
//		self->think = bigflame_think;
//	else
	self->nextthink = level.time + FRAMETIME;
	self->think = smallflame_think;
	gi.linkentity (self);
}

/*
==========================
Trigger entities with key presses

==========================
*/
//#define	START_OFF		1

extern void Think_Delay (edict_t *ent);
extern void target_laser_on (edict_t *self);
extern void target_laser_off (edict_t *self);

void Cmd_Trigger_f (edict_t *ent)
{
	char		*targetname;
	edict_t	*t;
	int		arg2;

	arg2 = atoi (gi.argv(2));
	targetname = gi.argv(1);
	t = G_Find (NULL, FOFS(targetname), targetname);

	switch (arg2)
	{
	/*
	=============
	Turn target off
	=============
	*/
		case 1:
			if (!t)
				return;
			if (t->classname == "target_mappack_laser")
			{
				if (t->spawnflags & 1)
					target_laser_off (t);			
			}
			if (t->classname == "mappack_light")
			{
				if (!t->spawnflags & 1)
				{
					gi.configstring (CS_LIGHTS+t->style, "a");
					t->spawnflags |= 1;
				}
			}
			break;
	/*
	=============
	Turn target on
	=============
	*/
		case 2: 
			if (!t)
				return;
			if (t->classname == "target_satan_laser")
			{
				if (t->spawnflags & 1)
				{
				}
				else
					target_laser_on (t);
			}
			if (t->classname == "satan_light")
			{
				if (t->spawnflags & 1)
				{
						gi.configstring (CS_LIGHTS+t->style, "m");
						t->spawnflags &= ~1;
				}
			}
			break;

	/*
	=============
	Just swap em around, or trigger em if not toggleable
	=============
	*/
		default:
			if (!t)
				return;

			t = G_Spawn();
			t->classname = "DelayedUse";
			t->nextthink = level.time + 0.01;
			t->think = Think_Delay;
			t->activator = ent;
			t->target = gi.argv(1);
			break;
	}
}
