
#include "g_local.h"

void think_targetlook (edict_t *ent)
{
	trace_t     tr;
	vec3_t      end, forward, start;
	edict_t *player;

	player = &g_edicts[1];	// Gotta be, since this is SP only
	
	VectorCopy(player->s.origin, start);
	start[2] += player->viewheight;

	AngleVectors(player->client->v_angle, forward, NULL, NULL);

	VectorMA(start, 8192, forward, end);

	tr = gi.trace (start, NULL, NULL, end, player, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

	if (tr.ent)
	{
		if (tr.ent == ent)
		{
			//hit!
			if (ent->target)
				G_UseTargets (ent, ent);

			if (ent->wait <= 0)
			{
				return;
			}
			else
			{
				ent->think = think_targetlook;
				ent->nextthink = level.time + ent->wait;
				return;
			}
		}
	}

	ent->think = think_targetlook;
	ent->nextthink = level.time + 0.2;
}

void use_targetlook (edict_t *ent, edict_t *other, edict_t *activator)
{
	//ent->think = playereffect_think;
	//ent->nextthink = level.time + FRAMETIME;
}

void SP_trigger_looker (edict_t *self)
{
	if (!self->wait)
		self->wait = -1;

	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_NONE;	

	self->use = use_targetlook;

	self->think = think_targetlook;
	self->nextthink = level.time + 1;

	gi.setmodel (self, self->model);
	gi.linkentity (self);
}