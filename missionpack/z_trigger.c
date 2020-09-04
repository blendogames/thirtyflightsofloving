#include "g_local.h"

/*
======================================================================

trigger_laser

======================================================================
*/

/*QUAKED trigger_laser (1 0 0) (-16 -16 -16) (16 16 16) TRIGGER_MULTIPLE
Laser-type trigger
"wait"       "x" where x is the delay before reactivation
"target"     target to trigger
"message"    message to center print
"delay"      delay before trigger
*/

#define TRIGGER_MULTIPLE	1
void trigger_laser_on (edict_t *self);
void trigger_laser_think (edict_t *self)
{
	vec3_t	start;
	vec3_t	end;
	trace_t	tr;
	int		count = 8;

	self->nextthink = level.time + FRAMETIME;
	
	VectorCopy (self->s.origin, start);
	VectorMA (start, 2048, self->movedir, end);
	tr = gi.trace (start, NULL, NULL, end, self, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

	if (!tr.ent)
		return;

	// if we hit something that's not a monster or player
	if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
	{
		if (self->spawnflags & 0x80000000)
		{
			self->spawnflags &= ~0x80000000;
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_LASER_SPARKS);
			gi.WriteByte (count);
			gi.WritePosition (tr.endpos);
			gi.WriteDir (tr.plane.normal);
			gi.WriteByte (self->s.skinnum);
			gi.multicast (tr.endpos, MULTICAST_PVS);
		}
	}
	else
	{
		// trigger
		G_UseTargets (self, tr.ent);

		if (self->spawnflags & TRIGGER_MULTIPLE)
		{
			// hide for a time
			self->svflags |= SVF_NOCLIENT;
			self->nextthink = level.time + self->wait;
			self->think = trigger_laser_on;
		}
		else
		{
			// remove self
			G_FreeEdict(self);
		}
	}

	VectorCopy (tr.endpos, self->s.old_origin);
}

void trigger_laser_on (edict_t *self)
{
	self->svflags &= ~SVF_NOCLIENT;
	self->think = trigger_laser_think;
	trigger_laser_think(self);
}

void SP_trigger_laser (edict_t *self)
{
	// if no target
	if (!self->target)
	{
		gi.dprintf("trigger_laser without target\n");
		G_FreeEdict(self);
		return;
	}

	self->class_id = ENTITY_TRIGGER_LASER;

	// if no wait, set default
	if (!self->wait)
	{
		self->wait = 4;
	}
	G_SetMovedir (self->s.angles, self->movedir);
	self->s.skinnum = 0xf2f2f0f0;	// colour
	self->s.frame = 2;				// diameter
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1;
	self->spawnflags |= 0x80000000;
	self->think = trigger_laser_on;
	self->nextthink = level.time + 0.1;
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity (self);
}
