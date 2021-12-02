
#include "g_local.h"

void npc_think (edict_t *self)
{
	if (self->style == 0) // gun
	{
		if (self->s.frame < 13)
			self->s.frame++;
		else
			self->s.frame = 0;
	}
	else if (self->style == 1) // sit on chair
	{
		if (self->s.frame < 23)
			self->s.frame++;
		else
			self->s.frame = 14;
	}
	else if (self->style == 2)
	{
		if (self->s.frame < 27)
			self->s.frame++;
		else
			self->s.frame = 24;
	}
	else if (self->style == 3)
	{
		if (self->s.frame < 31)
			self->s.frame++;
		else
			self->s.frame = 28;
	}

	self->nextthink=level.time+FRAMETIME;
}

void npc_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!activator->client || !self->target)
		return;


	
	//gi.dprintf("using npc\n");

	G_UseTargets (self, activator);
	

	//self->client->ps.stats[STAT_GRENADEABLE] = "abba\n";

	//gi.dprintf("%s\n",self->client->ps.stats[STAT_GRENADEABLE]);
}

void npc_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	//self->monsterinfo.currentmove = &flipper_move_pain1;
	gi.dprintf("ouch\n");
}

void SP_monster_npc (edict_t *self)
{
	gi.setmodel (self, "models/monsters/charlie/tris.md2");
	self->model = "models/monsters/charlie/tris.md2";
	self->solid = SOLID_BBOX;

	if ((self->style != 2) && (self->style != 3))
	{
		VectorSet(self->mins, -8, -8, 0);
		VectorSet(self->maxs, 8, 8, 52);
	}
	else
	{
		VectorSet(self->mins, -16, -16, 0);
		VectorSet(self->maxs, 16, 16, 37);
	}

	

	self->think=npc_think;
	self->nextthink=level.time+FRAMETIME;

	if (self->style == 2)
		self->s.modelindex2 = gi.modelindex ("models/weapons/g_machn/tris.md2");
		//self->modelindex2 = "models/weapons/g_machn/tris.md2";


	self->takedamage = DAMAGE_YES;
	self->pain = npc_pain;
	
	gi.linkentity (self);
}


// ========== MONSTER_STARER

void starer_think(edict_t *self)
{
	edict_t	*player;
	player = &g_edicts[1];

	if (player)
	{
		//found player.
        //face the player.
        vec3_t playervec;
		float ymin,ymax;
		vec3_t	forward, right;
		
		VectorCopy (player->s.origin, playervec);
		playervec[2] += self->attenuation; //they focus on your head.		

        VectorSubtract (playervec, self->pos1, playervec);

		//only stare if within radius.
		if (VectorLength(playervec) > self->radius)
		{
			VectorCopy(self->pos2,playervec);
		}
		else
		{
			vectoangles(playervec,playervec);

			ymin = self->pos2[1] - 89;
			ymax = self->pos2[1] + 89;

			if (playervec[YAW] > ymax || playervec[YAW] < ymin)
			{
				//player out of fov. snap back to default position.
				VectorCopy(self->pos2,playervec);
			}
		}

		VectorCopy (playervec, self->s.angles);
		
		
		AngleVectors (playervec, forward, right, NULL);
		G_ProjectSource (self->pos1, self->offset, forward, right, self->s.origin);

		if (self->spawnflags & 1)
		{
			self->s.origin[0] += crandom() * 0.1;
			self->s.origin[1] += crandom() * 0.1;
			self->s.origin[2] += crandom() * 0.4;
		}
	}	

	self->think = starer_think;
	self->nextthink = level.time + FRAMETIME;
}



void SP_monster_starer(edict_t *self)
{
	if (!self->usermodel)
		self->s.modelindex = gi.modelindex ("models/monsters/npc/head.md2");
	else
		self->s.modelindex = gi.modelindex (self->usermodel);

	if (!self->attenuation)
		self->attenuation = 14;

	if (self->spawnflags & 2)
		self->svflags = SVF_NOCLIENT;

	if (!self->radius)
		self->radius = 160;

	self->s.renderfx |= RF_NOSHADOW;
	VectorCopy(self->s.origin, self->pos1);
	self->solid			= SOLID_NOT;
	self->takedamage	= DAMAGE_NO;
	VectorCopy(self->s.angles, self->pos2);
	self->think = starer_think;
	self->nextthink = level.time + FRAMETIME;

	//BC baboo	
	if (skill->value >= 2 		&& !self->usermodel)
	{
		//the head model.
		self->s.skinnum = 7;
	}



	gi.linkentity (self);
}