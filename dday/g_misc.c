/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_misc.c,v $
 *   $Revision: 1.26 $
 *   $Date: 2002/07/25 01:44:49 $
 *
 ***********************************

Copyright (C) 2002 Vipersoft

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// g_misc.c

#include "g_local.h"
#include "x_fire.h"

void Spawn_Chute(edict_t *ent);

//faf
void Remove_Gib (edict_t *ent)
{
	level.gib_count--;
	G_FreeEdict (ent);

	if (level.gib_count < 0)
		level.gib_count = 0;

}

//Knightmare- gib fade
#ifdef KMQUAKE2_ENGINE_MOD
void gib_fade2 (edict_t *self);

void gib_fade (edict_t *self)
{
	if (self->s.effects & EF_BLASTER) //Remove glow from gekk gibs
	{
		self->s.effects &= ~EF_BLASTER;
		self->s.renderfx &= ~RF_NOSHADOW;
	}
	if (self->s.renderfx & RF_TRANSLUCENT)
		self->s.alpha = 0.70f;
	else if (self->s.effects & EF_SPHERETRANS)
		self->s.alpha = 0.30f;
	else if (!(self->s.alpha) || self->s.alpha <= 0.0F || self->s.alpha > 1.0F)
		self->s.alpha = 1.00f;
	gib_fade2 (self);
}

void gib_fade2 (edict_t *self)
{
	self->s.alpha -= 0.05f;
	self->s.alpha = max(self->s.alpha, 1/255);
	if (self->s.alpha <= 1/255)
	{
		Remove_Gib (self);
		return;
	}
	self->nextthink = level.time + 0.2;
	self->think = gib_fade2;
	gi.linkentity (self);
}

#else
void gib_fade2 (edict_t *self);

void gib_fade (edict_t *self)
{
	if (self->s.effects & EF_BLASTER)  //Remove glow from gekk gibs
		self->s.effects &= ~EF_BLASTER;
	self->s.renderfx = RF_TRANSLUCENT;
	self->nextthink = level.time + 2;
	self->think = gib_fade2;
	gi.linkentity(self);
}

void gib_fade2 (edict_t *self)
{
	self->s.effects |= EF_SPHERETRANS;
	self->s.renderfx &= ~RF_TRANSLUCENT;
	self->nextthink = level.time + 2;
	self->think = Remove_Gib;
	gi.linkentity(self);
}
#endif



/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.
*/

//=====================================================

void Use_Areaportal (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->count ^= 1;		// toggle state
//	gi.dprintf ("portalstate: %i = %i\n", ent->style, ent->count);
	gi.SetAreaPortalState (ent->style, ent->count);
}

/*QUAKED func_areaportal (0 0 0) ?

This is a non-visible object that divides the world into
areas that are seperated when this portal is not activated.
Usually enclosed in the middle of a door.
*/
void SP_func_areaportal (edict_t *ent)
{
	ent->use = Use_Areaportal;
	ent->count = 0;		// always start closed;
}

//=====================================================


/*
=================
Misc functions
=================
*/
void VelocityForDamage (int damage, vec3_t v)
{
	v[0] = 100.0 * crandom();
	v[1] = 100.0 * crandom();
	v[2] = 200.0 + 100.0 * random();

	if (damage < 50)
		VectorScale (v, 0.7, v);
	else
		VectorScale (v, 1.2, v);
}

void ClipGibVelocity (edict_t *ent)
{
	if (ent->velocity[0] < -300)
		ent->velocity[0] = -300;
	else if (ent->velocity[0] > 300)
		ent->velocity[0] = 300;
	if (ent->velocity[1] < -300)
		ent->velocity[1] = -300;
	else if (ent->velocity[1] > 300)
		ent->velocity[1] = 300;
	if (ent->velocity[2] < 200)
		ent->velocity[2] = 200;	// always some upwards
	else if (ent->velocity[2] > 500)
		ent->velocity[2] = 500;
}


/*
=================
gibs
=================
*/
void gib_think (edict_t *self)
{
	self->s.frame++;
	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame == 10)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 8 + random()*10;
	}
}

void head_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	vec3_t	normal_angles, right;

	float speed = abs (self->velocity[0]) + abs (self->velocity[1]) + abs (self->velocity[2]);
	if (speed > 350)
		speed = 350;


	if (self->avelocity[0] + self->avelocity[1] + self->avelocity[2] != 0 && // still bouncing/spinning
		(!other->takedamage || !other->client) && (other != self) ) //GaP prevent stupid sound
	{


		if (random() < .5)
			gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/gibs1.wav"), speed/350, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/gibs2.wav"), speed/350, ATTN_NORM, 0);
		self->s.angles[2] = 0;


	}
}

void gib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	vec3_t	normal_angles, right;

	float speed = abs (self->velocity[0]) + abs (self->velocity[1]) + abs (self->velocity[2]);

	if (speed > 350)
		speed = 350;

	if (!self->count)
		self->count = 0;

	self->count++;

	//gi.dprintf ("%f\n", speed);

    if (self->count >3 || (surf && (surf->flags & SURF_SKY)))
	{
		level.gib_count--;

		G_FreeEdict (self);
        return;
	}


	if (self->avelocity[0] + self->avelocity[1] + self->avelocity[2] != 0 && // still bouncing/spinning
		(!other->takedamage || !other->client) && (other != self) ) //GaP prevent stupid sound
	{


		if (random() < .5)
			gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/gibs1.wav"), speed/350, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/gibs2.wav"), speed/350, ATTN_NORM, 0);
		self->s.angles[2] = 0;


	}


/*
	self->touch = NULL;

	if (plane)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/fhit3.wav"), 1, ATTN_NORM, 0);

		vectoangles (plane->normal, normal_angles);
		AngleVectors (normal_angles, NULL, right, NULL);
		vectoangles (right, self->s.angles);

		if (self->s.modelindex == sm_meat_index)
		{
			self->s.frame++;
			self->think = gib_think;
			self->nextthink = level.time + FRAMETIME;
		}
	}*/
}

void gib_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict (self);

	//faf
	level.gib_count--;

	//faf
	if (level.gib_count < 0)
		level.gib_count = 0;


}

void ThrowGib (edict_t *self, char *gibname, int damage, int type)
{
	edict_t *gib;
	vec3_t	vd;
	vec3_t	origin;
	vec3_t	size;
	float	vscale;


	//faf
	if (level.gib_count > max_gibs->value)
		return;

	level.gib_count++;//faf
	gib = G_Spawn();

	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);

	gib->s.origin[0] = origin[0] + crandom() * size[0];
	gib->s.origin[1] = origin[1] + crandom() * size[1];
	gib->s.origin[2] = origin[2] + crandom() * size[2];

	gib->s.origin[2] += 5;//faf 32;
	gib->s.frame = 0;
	gi.setmodel (gib, gibname);

//	VectorSet (self->mins, -16, -16, 0);
//	VectorSet (self->maxs, 16, 16, 16);

	gib->solid = SOLID_NOT;
	gib->s.effects |= EF_GIB;
	gib->flags |= FL_NO_KNOCKBACK;
	gib->takedamage = DAMAGE_NO;//DAMAGE_YES;
	gib->die = gib_die;


	gib->clipmask = MASK_SHOT;
	gib->solid = SOLID_BBOX;


	if (type == GIB_ORGANIC)
	{
		gib->movetype = MOVETYPE_BOUNCE;//MOVETYPE_TOSS;
		gib->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		gib->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	VelocityForDamage (damage, vd);
	VectorMA (self->velocity, vscale, vd, gib->velocity);
	gib->velocity[0] = random()*100 -50;
	gib->velocity[1] = random()*100 -50;
	gib->velocity[2] = random()*75 - 25;


	ClipGibVelocity (gib);
	gib->avelocity[0] = random()*100;
	gib->avelocity[1] = random()*100;
//	gib->avelocity[2] = random()*400;

	gib->think = gib_fade;	// was Remove_Gib //faf G_FreeEdict;
	gib->nextthink = level.time + 5 + random()*3;

	gi.linkentity (gib);
}

void ThrowHead (edict_t *self, char *gibname, int damage, int type)
{
	vec3_t	vd;

	float	vscale;



	//faf
	if (level.gib_count > max_gibs->value)
		return;

	level.gib_count++;//faf




	self->s.skinnum = 0;

	self->s.frame = 0;

	VectorClear (self->mins);

	VectorClear (self->maxs);



	self->s.modelindex2 = 0;

	gi.setmodel (self, gibname);

	self->solid = SOLID_NOT;

	self->s.effects |= EF_GIB;

	self->s.effects &= ~EF_FLIES;

	self->s.sound = 0;

	self->flags |= FL_NO_KNOCKBACK;

	self->svflags &= ~SVF_MONSTER;

	self->takedamage = DAMAGE_YES;

	self->die = gib_die;



	if (type == GIB_ORGANIC)

	{

		self->movetype = MOVETYPE_TOSS;

		self->touch = gib_touch;

		vscale = 0.5;

	}

	else

	{

		self->movetype = MOVETYPE_BOUNCE;

		vscale = 1.0;

	}



	VelocityForDamage (damage, vd);

	VectorMA (self->velocity, vscale, vd, self->velocity);

	ClipGibVelocity (self);



	self->avelocity[YAW] = crandom()*600;



	self->think = gib_fade; // was Remove_Gib //faf G_FreeEdict;

	self->nextthink = level.time + 10 + random()*10;



	gi.linkentity (self);

}




void ThrowClientHead (edict_t *self, int damage)
{
	vec3_t	vd;
	char	*gibname;

//	if (rand()&1)
//	{
//		gibname = "models/objects/gibs/skull/tris.md2";
//		self->s.skinnum = 1;		// second skin is player
//	}
//	else
//	{
		gibname = "models/objects/gibs/head2/tris.md2";
		self->s.skinnum = 0;
//	}

	self->s.origin[2] += 32;
	self->s.frame = 0;
	gi.setmodel (self, gibname);

	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 16);

	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->s.effects = EF_GIB;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;
	self->movetype = MOVETYPE_BOUNCE;

	VelocityForDamage (damage, vd);
	VectorAdd (self->velocity, vd, self->velocity);


	if (self->client)	// bodies in the queue don't have a client anymore
	{
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = self->s.frame;
	}
	else
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 45;
	}

	self->clipmask = MASK_SHOT;
	self->solid = SOLID_BBOX;
//doesnt work	self->touch = head_touch;

	gi.linkentity (self);
}


/*
=================
debris
=================
*/
void debris_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict (self);
}

void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin)
{
	edict_t	*chunk;
	vec3_t	v;

	//faf
	if (level.gib_count > max_gibs->value)
		return;

	level.gib_count++;//faf



	chunk = G_Spawn();
	VectorCopy (origin, chunk->s.origin);
	gi.setmodel (chunk, modelname);
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	VectorMA (self->velocity, speed, v, chunk->velocity);
	chunk->movetype = MOVETYPE_BOUNCE;
	chunk->solid = SOLID_NOT;
	chunk->avelocity[0] = random()*600;
	chunk->avelocity[1] = random()*600;
	chunk->avelocity[2] = random()*600;
	chunk->think = gib_fade;	// was Remove_Gib //faf G_FreeEdict;
	chunk->nextthink = level.time + 5 + random()*5;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "debris";
	chunk->takedamage = DAMAGE_YES;
	chunk->die = debris_die;
	gi.linkentity (chunk);
}


void ThrowDebris2 (int soundtype, vec3_t origin)

{
	edict_t	*chunk;
	vec3_t	v;



	chunk = G_Spawn();
	VectorCopy (origin, chunk->s.origin);

	gi.setmodel (chunk, "models/objects/debris/tris.md2");

	if (soundtype ==  SOUND_CONCRETE)
		chunk->s.skinnum = (int)(random()*2);
	else
		chunk->s.skinnum = 2 + (int)(random()*2);

	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
//	VectorMA (vec3_origin, speed, v, chunk->velocity);

	chunk->s.frame = (int)(random()*6);
	VectorCopy (v, chunk->velocity);
	chunk->movetype = MOVETYPE_BOUNCE;
	chunk->solid = SOLID_NOT;
	chunk->avelocity[0] = random()*600;
	chunk->avelocity[1] = random()*600;
	chunk->avelocity[2] = random()*600;
	chunk->think = G_FreeEdict;
	chunk->nextthink = level.time + 1 + random()*2;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "debris";
	chunk->takedamage = DAMAGE_NO;
	//chunk->die = debris_die;
	gi.linkentity (chunk);
}


void BecomeExplosion1 (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
}


void BecomeExplosion2 (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION2);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
}


/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
Target: next path corner
Pathtarget: gets used when an entity that has
	this path_corner targeted touches it
*/

void path_corner_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		v;

	edict_t		*next;


	if (other->movetarget != self)
		return;

	if (other->enemy)
		return;

	if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		G_UseTargets (self, other);
		self->target = savetarget;
	}

	if (self->target)

		next = G_PickTarget(self->target);

	else

		next = NULL;



	if ((next) && (next->spawnflags & 1))

	{

		VectorCopy (next->s.origin, v);

		v[2] += next->mins[2];

		v[2] -= other->mins[2];

		VectorCopy (v, other->s.origin);

		next = G_PickTarget(next->target);

	}



	other->goalentity = other->movetarget = next;

	if (self->wait)
	{
		other->monsterinfo.pausetime = level.time + self->wait;
		other->monsterinfo.stand (other);
		return;
	}

	if (!other->movetarget)
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		other->monsterinfo.stand (other);
	}
	else
	{
		VectorSubtract (other->goalentity->s.origin, other->s.origin, v);
		other->ideal_yaw = vectoyaw (v);
	}
}

void SP_path_corner (edict_t *self)
{
	if (!self->targetname)
	{
		gi.dprintf ("path_corner with no targetname at %s\n", vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	self->solid = SOLID_TRIGGER;
	self->touch = path_corner_touch;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	self->svflags |= SVF_NOCLIENT;

	gi.linkentity (self);
}


/*QUAKED point_combat (0.5 0.3 0) (-8 -8 -8) (8 8 8) Hold
Makes this the target of a monster and it will head here
when first activated before going after the activator.  If
hold is selected, it will stay here.
*/
void point_combat_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*activator;

	if (other->movetarget != self)
		return;

	if (self->target)
	{
		other->target = self->target;
		other->goalentity = other->movetarget = G_PickTarget(other->target);
		if (!other->goalentity)
		{
			gi.dprintf("%s at %s target %s does not exist\n", self->classname, vtos(self->s.origin), self->target);
			other->movetarget = self;
		}
		self->target = NULL;
	}
	else if ((self->spawnflags & 1) && !(other->flags & (FL_SWIM|FL_FLY)))
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		other->monsterinfo.aiflags |= AI_STAND_GROUND;
		other->monsterinfo.stand (other);
	}

	if (other->movetarget == self)
	{
		other->target = NULL;
		other->movetarget = NULL;
		other->goalentity = other->enemy;
		other->monsterinfo.aiflags &= ~AI_COMBAT_POINT;

	}

	if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		if (other->enemy && other->enemy->client)
			activator = other->enemy;
		else if (other->oldenemy && other->oldenemy->client)
			activator = other->oldenemy;
		else if (other->activator && other->activator->client)
			activator = other->activator;
		else
			activator = other;
		G_UseTargets (self, activator);
		self->target = savetarget;
	}
}

void SP_point_combat (edict_t *self)
{
	if (deathmatch->value)

	{

		G_FreeEdict (self);

		return;

	}

	self->solid = SOLID_TRIGGER;
	self->touch = point_combat_touch;
	VectorSet (self->mins, -8, -8, -16);
	VectorSet (self->maxs, 8, 8, 16);
	self->svflags = SVF_NOCLIENT;
	gi.linkentity (self);
};


/*QUAKED viewthing (0 .5 .8) (-8 -8 -8) (8 8 8)
Just for the debugging level.  Don't use
*/
void TH_viewthing(edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 7;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_viewthing(edict_t *ent)
{
	gi.dprintf ("viewthing spawned\n");

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.renderfx = RF_FRAMELERP;
	VectorSet (ent->mins, -16, -16, -24);
	VectorSet (ent->maxs, 16, 16, 32);
	ent->s.modelindex = gi.modelindex ("models/objects/banner/tris.md2");
	gi.linkentity (ent);
	ent->nextthink = level.time + 0.5;
	ent->think = TH_viewthing;
	return;
}


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for spotlights, etc.
*/
void SP_info_null (edict_t *self)
{
	G_FreeEdict (self);
};


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for lightning.
*/
void SP_info_notnull (edict_t *self)
{
	VectorCopy (self->s.origin, self->absmin);
	VectorCopy (self->s.origin, self->absmax);
};


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) START_OFF
Non-displayed light.
Default light value is 300.
Default style is 0.
If targeted, will toggle between on and off.
Default _cone value is 10 (used to set size of light for spotlights)
*/

#define START_OFF	1

static void light_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & START_OFF)
	{
		gi.configstring (CS_LIGHTS+self->style, "m");
		self->spawnflags &= ~START_OFF;
	}
	else
	{
		gi.configstring (CS_LIGHTS+self->style, "a");
		self->spawnflags |= START_OFF;
	}
}

void SP_light (edict_t *self)
{

	// no targeted lights in deathmatch, because they cause global messages
//	if (!self->targetname || deathmatch->value)
//	{
//		G_FreeEdict (self);
//		return;
//	}

	if (self->style >= 32)
	{
		self->use = light_use;
		if (self->spawnflags & START_OFF)
			gi.configstring (CS_LIGHTS+self->style, "a");
		else
			gi.configstring (CS_LIGHTS+self->style, "m");
	}
}


/*QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED ANIMATED_FAST
This is just a solid wall if not inhibited

TRIGGER_SPAWN	the wall will not be present until triggered
				it will then blink in to existance; it will
				kill anything that was in it's way

TOGGLE			only valid for TRIGGER_SPAWN walls
				this allows the wall to be turned on and off

START_ON		only valid for TRIGGER_SPAWN walls
				the wall will initially be present
*/

void func_wall_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
	{
		self->solid = SOLID_BSP;
		self->svflags &= ~SVF_NOCLIENT;

		KillBox (self);

	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;

	}
	gi.linkentity (self);

	if (!(self->spawnflags & 2))
		self->use = NULL;
}

void SP_func_wall (edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self->model);

	if (self->spawnflags & 8)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 16)
		self->s.effects |= EF_ANIM_ALLFAST;

	// just a wall
	if ((self->spawnflags & 7) == 0)
	{
		self->solid = SOLID_BSP;
		gi.linkentity (self);
		return;
	}

	// it must be TRIGGER_SPAWN
	if (!(self->spawnflags & 1))
	{
//		gi.dprintf("func_wall missing TRIGGER_SPAWN\n");
		self->spawnflags |= 1;
	}

	// yell if the spawnflags are odd
	if (self->spawnflags & 4)
	{
		if (!(self->spawnflags & 2))
		{
			gi.dprintf("func_wall START_ON without TOGGLE\n");
			self->spawnflags |= 2;
		}
	}

	self->use = func_wall_use;
	if (self->spawnflags & 4)
	{
		self->solid = SOLID_BSP;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;

	}
	gi.linkentity (self);
}


/*QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
This is solid bmodel that will fall if it's support it removed.
*/

void func_object_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// only squash thing we fall on top of
	if (!plane)
		return;
	if (plane->normal[2] < 1.0)
		return;
	if (other->takedamage == DAMAGE_NO)
		return;
	T_Damage (other, self, self, vec3_origin, self->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void func_object_release (edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->touch = func_object_touch;
}

void func_object_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;

	self->use = NULL;
	KillBox (self);
	func_object_release (self);
}

void SP_func_object (edict_t *self)
{
	gi.setmodel (self, self->model);

	self->mins[0] += 1;
	self->mins[1] += 1;
	self->mins[2] += 1;
	self->maxs[0] -= 1;
	self->maxs[1] -= 1;
	self->maxs[2] -= 1;

	if (!self->dmg)
		self->dmg = 100;

	if (self->spawnflags == 0)
	{
		self->solid = SOLID_BSP;
		self->movetype = MOVETYPE_PUSH;
		self->think = func_object_release;
		self->nextthink = level.time + 2 * FRAMETIME;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->movetype = MOVETYPE_PUSH;
		self->use = func_object_use;
		self->svflags |= SVF_NOCLIENT;

	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	self->clipmask = MASK_MONSTERSOLID;



	gi.linkentity (self);
}


/*QUAKED func_explosive (0 .5 .8) ? Trigger_Spawn ANIMATED ANIMATED_FAST
Any brush that you want to explode or break apart.  If you want an
ex0plosion, set dmg and it will do a radius explosion of that amount
at the center of the bursh.

If targeted it will not be shootable.

health defaults to 100.

mass defaults to 75.  This determines how much debris is emitted when
it explodes.  You get one large chunk per 100 of mass (up to 8) and
one small chunk per 25 of mass (up to 16).  So 800 gives the most.
*/
void func_explosive_explode (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t	origin;
	vec3_t	chunkorigin;
	vec3_t	size;
	int		count;
	int		mass;

	// bmodel origins are (0 0 0), we need to adjust that here
	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);
	VectorCopy (origin, self->s.origin);


	self->takedamage = DAMAGE_NO;


	if (self->dmg)
		T_RadiusDamage (self, attacker, self->dmg, NULL, self->dmg+40, MOD_EXPLOSIVE);


	VectorSubtract (self->s.origin, inflictor->s.origin, self->velocity);

	VectorNormalize (self->velocity);

	VectorScale (self->velocity, 150, self->velocity);



	// start chunks towards the center
	VectorScale (size, 0.5, size);

	mass = self->mass;
	if (!mass)
		mass = 75;

	// big chunks
	if (mass >= 100)
	{
		count = mass / 100;
		if (count > 8)
			count = 8;
		while(count--)
		{
			chunkorigin[0] = origin[0] + crandom() * size[0];
			chunkorigin[1] = origin[1] + crandom() * size[1];
			chunkorigin[2] = origin[2] + crandom() * size[2];
			ThrowDebris (self, "models/objects/debris1/tris.md2", 1, chunkorigin);
		}
	}

	// small chunks
	count = mass / 25;
	if (count > 16)
		count = 16;
	while(count--)
	{
		chunkorigin[0] = origin[0] + crandom() * size[0];
		chunkorigin[1] = origin[1] + crandom() * size[1];
		chunkorigin[2] = origin[2] + crandom() * size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", 2, chunkorigin);
	}

	G_UseTargets (self, attacker);

	if (self->dmg)
		BecomeExplosion1 (self);
	else
		G_FreeEdict (self);
}

void func_explosive_use(edict_t *self, edict_t *other, edict_t *activator)
{
	func_explosive_explode (self, self, other, self->health, vec3_origin);
}

void func_explosive_spawn (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;

	self->use = NULL;
	KillBox (self);
	gi.linkentity (self);
}

void SP_func_explosive (edict_t *self)
{
	self->classnameb = FUNC_EXPLOSIVE;
	//if (deathmatch->value)
	//{	// auto-remove for deathmatch
	//	G_FreeEdict (self);
	//	return;
	//}

	self->movetype = MOVETYPE_PUSH;

	gi.modelindex ("models/objects/debris1/tris.md2");
	gi.modelindex ("models/objects/debris2/tris.md2");

	gi.setmodel (self, self->model);

	if (self->spawnflags & 1)
	{
		self->svflags |= SVF_NOCLIENT;

		self->solid = SOLID_NOT;
		self->use = func_explosive_spawn;
	}
	else
	{
		self->solid = SOLID_BSP;
		if (self->targetname)
			self->use = func_explosive_use;
	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	if (self->use != func_explosive_use)
	{
		if (!self->health)
			self->health = 100;
		self->die = func_explosive_explode;
		self->takedamage = DAMAGE_YES;
	}



	//quick hack so bots don't pointlessly shoot stuff on these maps
	if (!strcmp(level.mapname,"dday2") ||
		!strcmp(level.mapname,"feb_bridge")
)
	{
		self->style = -1;
	}
	else if (!strcmp(level.mapname,"outpost"))
	{
		if (!self->target)
			self->style = -1;
	}



	//can't be destroyed if there are bots (for bridges and stuff)
	if (self->style == -2 && bots->value != 0)
		self->takedamage = DAMAGE_NO;




	//so bots can aim at center.  either mapper sets it or it goes to the center
	if (VectorCompare (self->obj_origin, vec3_origin))
	{
		VectorSet (self->obj_origin, (self->absmax[0] + self->absmin[0])/2,
		(self->absmax[1] + self->absmin[1])/2,
		(self->absmax[2] + self->absmin[2])/2);
	}




	gi.linkentity (self);
}


/*QUAKED misc_explobox (0 .5 .8) (-16 -16 0) (16 16 40)
Large exploding box.  You can override its mass (100),
health (80), and dmg (150).
*/

void barrel_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)

{
	float	ratio;
	vec3_t	v;

	if ((!other->groundentity) || (other->groundentity == self))
		return;

	ratio = (float)other->mass / (float)self->mass;
	VectorSubtract (self->s.origin, other->s.origin, v);
	M_walkmove (self, vectoyaw(v), 20 * ratio * FRAMETIME);
}

void barrel_explode (edict_t *self)
{
	vec3_t	org;
	float	spd;
	vec3_t	save;

	T_RadiusDamage (self, self->activator, self->dmg, NULL, self->dmg+40, MOD_BARREL);

	VectorCopy (self->s.origin, save);
	VectorMA (self->absmin, 0.5, self->size, self->s.origin);

	// a few big chunks
	spd = 1.5 * (float)self->dmg / 200.0;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris1/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris1/tris.md2", spd, org);

	// bottom corners
	spd = 1.75 * (float)self->dmg / 200.0;
	VectorCopy (self->absmin, org);
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[0] += self->size[0];
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[1] += self->size[1];
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[0] += self->size[0];
	org[1] += self->size[1];
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);

	// a bunch of little chunks
	spd = 2 * self->dmg / 200;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);

	VectorCopy (save, self->s.origin);
	if (self->groundentity)
		BecomeExplosion2 (self);
	else
		BecomeExplosion1 (self);
}

void barrel_delay (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + 2 * FRAMETIME;
	self->think = barrel_explode;
	self->activator = attacker;
}

void SP_misc_explobox (edict_t *self)
{
	if (deathmatch->value)

	{	// auto-remove for deathmatch

		G_FreeEdict (self);

		return;

	}



	gi.modelindex ("models/objects/debris1/tris.md2");
	gi.modelindex ("models/objects/debris2/tris.md2");
	gi.modelindex ("models/objects/debris3/tris.md2");

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_STEP;

	if (!strncmp (self->model, "models/", 7))
	{
		gi.setmodel(self, self->model);
		VectorSet (self->mins, -16, -16, 0);
		VectorSet (self->maxs, 16, 16, 40);
	}
	else
	{
		gi.setmodel (self, self->model);
	}


//	self->model = "models/objects/barrels/tris.md2";
//	self->s.modelindex = gi.modelindex (self->model);

	if (!self->mass)
		self->mass = 400;
	if (!self->health)
		self->health = 10;
	if (!self->dmg)
		self->dmg = 150;

/*-----/ PM /-----/ NEW:  Set gib health and fireflags. /-----*/

        self->gib_health = 0;

        self->fireflags  = 36 | FIREFLAG_IGNITE;

/*------------------------------------------------------------*/



	self->die = barrel_delay;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.aiflags = AI_NOSTEP;

	self->touch = barrel_touch;


	self->think = M_droptofloor;

	self->nextthink = level.time + 2 * FRAMETIME;


	gi.linkentity (self);
}


//
// miscellaneous specialty items
//


/*QUAKED misc_blackhole (1 .5 0) (-8 -8 -8) (8 8 8)

*/



void misc_blackhole_use (edict_t *ent, edict_t *other, edict_t *activator)

{

	/*

	gi.WriteByte (svc_temp_entity);

	gi.WriteByte (TE_BOSSTPORT);

	gi.WritePosition (ent->s.origin);

	gi.multicast (ent->s.origin, MULTICAST_PVS);

	*/

	G_FreeEdict (ent);

}



void misc_blackhole_think (edict_t *self)

{

	if (++self->s.frame < 19)

		self->nextthink = level.time + FRAMETIME;

	else

	{

		self->s.frame = 0;

		self->nextthink = level.time + FRAMETIME;

	}

}



void SP_misc_blackhole (edict_t *ent)

{

	ent->movetype = MOVETYPE_NONE;

	ent->solid = SOLID_NOT;

	VectorSet (ent->mins, -64, -64, 0);

	VectorSet (ent->maxs, 64, 64, 8);

	ent->s.modelindex = gi.modelindex ("models/objects/black/tris.md2");

	ent->s.renderfx = RF_TRANSLUCENT;

	ent->use = misc_blackhole_use;

	ent->think = misc_blackhole_think;

	ent->nextthink = level.time + 2 * FRAMETIME;

	gi.linkentity (ent);

}



/*QUAKED misc_eastertank (1 .5 0) (-32 -32 -16) (32 32 32)

*/



void misc_eastertank_think (edict_t *self)

{

	if (++self->s.frame < 293)

		self->nextthink = level.time + FRAMETIME;

	else

	{

		self->s.frame = 254;

		self->nextthink = level.time + FRAMETIME;

	}

}



void SP_misc_eastertank (edict_t *ent)

{

	ent->movetype = MOVETYPE_NONE;

	ent->solid = SOLID_BBOX;

	VectorSet (ent->mins, -32, -32, -16);

	VectorSet (ent->maxs, 32, 32, 32);

	ent->s.modelindex = gi.modelindex ("models/monsters/tank/tris.md2");

	ent->s.frame = 254;

	ent->think = misc_eastertank_think;

	ent->nextthink = level.time + 2 * FRAMETIME;

	gi.linkentity (ent);

}



/*QUAKED misc_easterchick (1 .5 0) (-32 -32 0) (32 32 32)

*/





void misc_easterchick_think (edict_t *self)

{

	if (++self->s.frame < 247)

		self->nextthink = level.time + FRAMETIME;

	else

	{

		self->s.frame = 208;

		self->nextthink = level.time + FRAMETIME;

	}

}



void SP_misc_easterchick (edict_t *ent)

{

	ent->movetype = MOVETYPE_NONE;

	ent->solid = SOLID_BBOX;

	VectorSet (ent->mins, -32, -32, 0);

	VectorSet (ent->maxs, 32, 32, 32);

	ent->s.modelindex = gi.modelindex ("models/monsters/bitch/tris.md2");

	ent->s.frame = 208;

	ent->think = misc_easterchick_think;

	ent->nextthink = level.time + 2 * FRAMETIME;

	gi.linkentity (ent);

}



/*QUAKED misc_easterchick2 (1 .5 0) (-32 -32 0) (32 32 32)

*/





void misc_easterchick2_think (edict_t *self)

{

	if (++self->s.frame < 287)

		self->nextthink = level.time + FRAMETIME;

	else

	{

		self->s.frame = 248;

		self->nextthink = level.time + FRAMETIME;

	}

}



void SP_misc_easterchick2 (edict_t *ent)

{

	ent->movetype = MOVETYPE_NONE;

	ent->solid = SOLID_BBOX;

	VectorSet (ent->mins, -32, -32, 0);

	VectorSet (ent->maxs, 32, 32, 32);

	ent->s.modelindex = gi.modelindex ("models/monsters/bitch/tris.md2");

	ent->s.frame = 248;

	ent->think = misc_easterchick2_think;

	ent->nextthink = level.time + 2 * FRAMETIME;

	gi.linkentity (ent);

}




/*QUAKED monster_commander_body (1 .5 0) (-32 -32 0) (32 32 48)
Not really a monster, this is the Tank Commander's decapitated body.
There should be a item_commander_head that has this as it's target.
*/

void commander_body_think (edict_t *self)
{
	if (++self->s.frame < 24)
		self->nextthink = level.time + FRAMETIME;
	else
		self->nextthink = 0;

	if (self->s.frame == 22)
		gi.sound (self, CHAN_BODY, gi.soundindex ("tank/thud.wav"), 1, ATTN_NORM, 0);
}

void commander_body_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->think = commander_body_think;
	self->nextthink = level.time + FRAMETIME;
	gi.sound (self, CHAN_BODY, gi.soundindex ("tank/pain.wav"), 1, ATTN_NORM, 0);
}

void commander_body_drop (edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->s.origin[2] += 2;
}

void SP_monster_commander_body (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	self->model = "models/monsters/commandr/tris.md2";
	self->s.modelindex = gi.modelindex (self->model);
	VectorSet (self->mins, -32, -32, 0);
	VectorSet (self->maxs, 32, 32, 48);
	self->use = commander_body_use;
	self->takedamage = DAMAGE_YES;
	self->flags = FL_GODMODE;
	self->s.renderfx |= RF_FRAMELERP;
	gi.linkentity (self);

	gi.soundindex ("tank/thud.wav");
	gi.soundindex ("tank/pain.wav");

	self->think = commander_body_drop;
	self->nextthink = level.time + 5 * FRAMETIME;
}


/*QUAKED misc_banner (1 .5 0) (-4 -4 -4) (4 4 4)
The origin is the bottom of the banner.
The banner is 128 tall.
*/
void misc_banner_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/objects/banner/tris.md2");
	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}



void SP_misc_banner_generic (edict_t *ent, char *model)

{

	ent->movetype = MOVETYPE_NONE;

	ent->solid = SOLID_NOT;

	ent->s.modelindex = gi.modelindex(model);

	ent->s.frame = rand() % 16;

	gi.linkentity (ent);



	ent->think = misc_banner_think;

	ent->nextthink = level.time + FRAMETIME;

}

//bcass start - banner thing - this next part is ripped from above, yo'
/*QUAKED misc_banner (1 .5 0) (-4 -4 -4) (4 4 4)
The origin is the bottom of the banner.
The banner is 128 tall.
*/
void misc_banner_x_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_banner_x (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;


	ent->s.modelindex = gi.modelindex ("models/objects/bannerx/tris.md2");
	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_banner_x_think;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_banner_1 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;


	ent->s.modelindex = gi.modelindex ("models/objects/banner1/tris.md2");
	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_banner_2 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;


	ent->s.modelindex = gi.modelindex ("models/objects/banner2/tris.md2");
	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_banner_3 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;


	ent->s.modelindex = gi.modelindex ("models/objects/banner3/tris.md2");
	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_banner_4 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;


	ent->s.modelindex = gi.modelindex ("models/objects/banner4/tris.md2");
	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}


void SP_misc_banner_x_generic (edict_t *ent, char *model)

{

	ent->movetype = MOVETYPE_NONE;

	ent->solid = SOLID_NOT;

	ent->s.modelindex = gi.modelindex(model);

	ent->s.frame = rand() % 16;

	gi.linkentity (ent);



	ent->think = misc_banner_x_think;

	ent->nextthink = level.time + FRAMETIME;

}
//bcass end

//bcass start - skeleton model thing
/*QUAKED misc_skeleton (1 .5 0) (-4 -4 -4) (4 4 4)
*/
void misc_skeleton_think (edict_t *ent)
{
//	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->s.frame = (ent->s.frame);
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_skeleton (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/deadbods/dude/tris.md2");
//	ent->s.frame = rand() % 16;
//	ent->s.frame = 1;
	gi.linkentity (ent);

	ent->think = misc_skeleton_think;
	ent->nextthink = level.time + FRAMETIME;
}


//faf: mappers can put "frames" or "mass" value to equal total frame numbers for animated md2s.
void misc_md2_think (edict_t *ent)
{
    if (ent->mass)
		ent->s.frame = (ent->s.frame + 1) % ent->mass; //faf
	else
		ent->s.frame = (ent->s.frame);

	ent->nextthink = level.time + FRAMETIME;
}

void misc_md2_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->svflags & SVF_NOCLIENT)
	{
		self->svflags &= ~SVF_NOCLIENT;
	}
	else
	{
		self->svflags |= SVF_NOCLIENT;
	}
	gi.linkentity (self);
}

void SP_misc_md2 (edict_t *ent)
{
     // Knightmare added
    char    modelName[MAX_QPATH] = {0};
    int     i;
    // end Knightmare

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;

//	ent->s.modelindex = gi.modelindex (ent->model);
	// Knightmare- convert modelname to all lowercase (for Linux)
	Q_strncpyz (modelName, sizeof(modelName), ent->model);
	for (i = 0; modelName[i]; i++)
		modelName[i] = tolower(modelName[i]);
	ent->s.modelindex = gi.modelindex (modelName);
    // end Knightmare

//	ent->s.frame = rand() % 16;
//	ent->s.frame = 1;

	ent->think = misc_md2_think;
	ent->nextthink = level.time + FRAMETIME;


	ent->use = misc_md2_use;


	if (ent->style == 1)
		ent->s.renderfx = RF_TRANSLUCENT;

	//start off
	if (ent->style == 2)
		ent->svflags |= SVF_NOCLIENT;



	if (!VectorCompare (ent->move_origin,vec3_origin) && !VectorCompare (ent->move_angles,vec3_origin))
	{
		VectorCopy (ent->move_origin, ent->mins);
		VectorCopy (ent->move_angles, ent->maxs);
		ent->movetype = MOVETYPE_NONE;
		ent->solid = SOLID_BBOX;
		ent->mass = 400;
	}

	gi.linkentity (ent);

}

void objective_flag_think(edict_t *ent)
{
	edict_t  *e;
	int i;

	if (ent->mass)
		ent->s.frame = (ent->s.frame + 1) % ent->mass; //faf
	else
		ent->s.frame = (ent->s.frame);


	if (!ent->owner)
	{
		if (!ent->targetname)
		{
			ent->think = G_FreeEdict;
			ent->nextthink = level.time +.1;
			gi.dprintf ("objective_flag needs targetname\n");
			return;
		}
		for (i=0 ; i<globals.num_edicts ; i++)
		{
			e = &g_edicts[i];

			if (!e->inuse)
				continue;

			if (!e->classnameb)
				continue;
			if (e->classnameb != OBJECTIVE_TOUCH)
				continue;

			if (!e->target)
				continue;

			if (!strcmp(e->target, ent->targetname))
			{
				ent->owner = e;
			}

		}
	}
	else{
		if (ent->owner->obj_owner == 0)
			ent->s.skinnum = 1;
		else if (ent->owner->obj_owner == 1)
			ent->s.skinnum = 2;
		else
			ent->s.skinnum = 0;
	}

	ent->nextthink = level.time +.1;

}


void SP_objective_flag (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex (ent->model);
	gi.linkentity (ent);

	ent->think = objective_flag_think;
	ent->nextthink = level.time + FRAMETIME;

}



void SP_misc_crate (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex ("models/objects/crate/tris.md2");
//	ent->s.frame = rand() % 16;
//	ent->s.frame = 1;

	ent->mass = 400;

	if (ent->style)
		ent->s.skinnum = ent->style;

	if (ent->count == 0)
	{
		ent->s.frame = 0;
		VectorSet (ent->mins, -8, -8, -10);
		VectorSet (ent->maxs, 8, 8, 6);
	}
	else if (ent->count == 1)
	{
		ent->s.frame = 1;
		VectorSet (ent->mins, -24, -24, -10);
		VectorSet (ent->maxs, 24, 24, 36);
	}
	else if (ent->count == 2)
	{
		ent->s.frame = 2;
		VectorSet (ent->mins, -32, -32, -10);
		VectorSet (ent->maxs, 32, 32, 54);
	}
	gi.linkentity (ent);

}



void SP_misc_skeleton_generic (edict_t *ent, char *model)

{

	ent->movetype = MOVETYPE_NONE;

	ent->solid = SOLID_NOT;

	ent->s.modelindex = gi.modelindex(model);

	ent->s.frame = rand() % 16;

	gi.linkentity (ent);



	ent->think = misc_skeleton_think;

	ent->nextthink = level.time + FRAMETIME;

}
//bcass end

/*QUAKED misc_deadsoldier (1 .5 0) (-16 -16 0) (16 16 16) ON_BACK ON_STOMACH BACK_DECAP FETAL_POS SIT_DECAP IMPALED
This is the dead player model. Comes in 6 exciting different poses!
*/
void misc_deadsoldier_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;


/*-----/ PM /-----/ MODIFIED:  Check for gib. /-----*/

    if (self->health > self->gib_health)

/*--------------------------------------------------*/

	return;

	gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
	for (n= 0; n < 4; n++)
		ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
	ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
}

void SP_misc_deadsoldier (edict_t *ent)
{
	if (deathmatch->value)

	{	// auto-remove for deathmatch

		G_FreeEdict (ent);

		return;

	}



	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex=gi.modelindex ("models/deadbods/dude/tris.md2");

	// Defaults to frame 0
	if (ent->spawnflags & 2)
		ent->s.frame = 1;
	else if (ent->spawnflags & 4)
		ent->s.frame = 2;
	else if (ent->spawnflags & 8)
		ent->s.frame = 3;
	else if (ent->spawnflags & 16)
		ent->s.frame = 4;
	else if (ent->spawnflags & 32)
		ent->s.frame = 5;
	else
		ent->s.frame = 0;

	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 16);
	ent->deadflag = DEAD_DEAD;
	ent->takedamage = DAMAGE_YES;
	ent->svflags |= SVF_MONSTER|SVF_DEADMONSTER;
	ent->die = misc_deadsoldier_die;
	ent->monsterinfo.aiflags |= AI_GOOD_GUY;



/*-----/ PM /-----/ NEW:  Set gib health, mass, and fireflags. /-----*/

        ent->gib_health = -80;

        ent->mass       = 200;

        ent->fireflags  = 4 | FIREFLAG_IGNITE;

/*-------------------------------------------------------------------*/


	gi.linkentity (ent);
}

/*QUAKED misc_viper (1 .5 0) (-16 -16 0) (16 16 32)
This is the Viper for the flyby bombing.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"		How fast the Viper should fly
*/

extern void train_use (edict_t *self, edict_t *other, edict_t *activator);
extern void func_train_find (edict_t *self);

void misc_viper_use  (edict_t *self, edict_t *other, edict_t *activator)
{
	self->svflags &= ~SVF_NOCLIENT;

	self->use = train_use;
	train_use (self, other, activator);
}

void SP_misc_viper (edict_t *ent)
{
	if (!ent->target)
	{
		gi.dprintf ("misc_viper without a target at %s\n", vtos(ent->absmin));
		G_FreeEdict (ent);
		return;
	}

	if (!ent->speed)
	     ent->speed = 300;

	if (ent->model &&
		!strncmp (ent->model, "models/", 7))
	{
		gi.setmodel(ent, ent->model);
	}
	else
	{
	ent->s.modelindex = gi.modelindex ("models/ships/viper/tris.md2");
	}
	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_NOT;

	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 32);

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_viper_use;
	ent->svflags |= SVF_NOCLIENT;

	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	gi.linkentity (ent);
}


/*QUAKED misc_bigviper (1 .5 0) (-176 -120 -24) (176 120 72)
This is a large stationary viper as seen in Paul's intro
*/
void SP_misc_bigviper (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -176, -120, -24);
	VectorSet (ent->maxs, 176, 120, 72);
	ent->s.modelindex = gi.modelindex ("models/ships/bigviper/tris.md2");
	gi.linkentity (ent);
}


/*QUAKED misc_viper_bomb (1 0 0) (-8 -8 -8) (8 8 8)
"dmg"	how much boom should the bomb make?
*/
void misc_viper_bomb_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	G_UseTargets (self, self->activator);

	self->s.origin[2] = self->absmin[2] + 1;
	T_RadiusDamage (self, self, self->dmg, NULL, self->dmg+40, MOD_BOMB);
	BecomeExplosion2 (self);
}

void misc_viper_bomb_prethink (edict_t *self)
{
	vec3_t	v;
	float	diff;

	self->groundentity = NULL;

	diff = self->timestamp - level.time;
	if (diff < -1.0)
		diff = -1.0;

	VectorScale (self->moveinfo.dir, 1.0 + diff, v);
	v[2] = diff;

	diff = self->s.angles[2];
	vectoangles (v, self->s.angles);
	self->s.angles[2] = diff + 10;
}

void misc_viper_bomb_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*viper;

	self->solid = SOLID_BBOX;
	self->svflags &= ~SVF_NOCLIENT;

//	self->s.effects |= EF_ROCKET;  //Take out cheesy flare behind
	self->use = NULL;
	self->movetype = MOVETYPE_TOSS;
	self->prethink = misc_viper_bomb_prethink;
	self->touch = misc_viper_bomb_touch;
	self->activator = activator;

	viper = G_Find (NULL, FOFS(classname), "misc_viper");
	VectorScale (viper->moveinfo.dir, viper->moveinfo.speed, self->velocity);

	self->timestamp = level.time;
	VectorCopy (viper->moveinfo.dir, self->moveinfo.dir);
}

void SP_misc_viper_bomb (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);

	self->s.modelindex = gi.modelindex ("models/objects/bomb/tris.md2");

	if (!self->dmg)
		self->dmg = 1000;

	self->use = misc_viper_bomb_use;
	self->svflags |= SVF_NOCLIENT;


	gi.linkentity (self);
}


/*QUAKED misc_strogg_ship (1 .5 0) (-16 -16 0) (16 16 32)

This is a Storgg ship for the flybys.

It is trigger_spawned, so you must have something use it for it to show up.

There must be a path for it to follow once it is activated.



"speed"		How fast it should fly

*/



extern void train_use (edict_t *self, edict_t *other, edict_t *activator);

extern void func_train_find (edict_t *self);



void misc_strogg_ship_use  (edict_t *self, edict_t *other, edict_t *activator)

{

	self->svflags &= ~SVF_NOCLIENT;

	self->use = train_use;

	train_use (self, other, activator);

}



void SP_misc_strogg_ship (edict_t *ent)

{

	if (!ent->target)

	{

		gi.dprintf ("%s without a target at %s\n", ent->classname, vtos(ent->absmin));

		G_FreeEdict (ent);

		return;

	}



	if (!ent->speed)

		ent->speed = 300;



	ent->movetype = MOVETYPE_PUSH;

	ent->solid = SOLID_NOT;

	ent->s.modelindex = gi.modelindex ("models/ships/strogg1/tris.md2");

	VectorSet (ent->mins, -16, -16, 0);

	VectorSet (ent->maxs, 16, 16, 32);



	ent->think = func_train_find;

	ent->nextthink = level.time + FRAMETIME;

	ent->use = misc_strogg_ship_use;

	ent->svflags |= SVF_NOCLIENT;

	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;



	gi.linkentity (ent);

}





/*QUAKED misc_satellite_dish (1 .5 0) (-64 -64 0) (64 64 128)
*/
void misc_satellite_dish_think (edict_t *self)
{
	self->s.frame++;
	if (self->s.frame < 38)
		self->nextthink = level.time + FRAMETIME;
}

void misc_satellite_dish_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->s.frame = 0;
	self->think = misc_satellite_dish_think;
	self->nextthink = level.time + FRAMETIME;
}

void SP_misc_satellite_dish (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -64, -64, 0);
	VectorSet (ent->maxs, 64, 64, 128);
	ent->s.modelindex = gi.modelindex ("models/objects/satellite/tris.md2");
	ent->use = misc_satellite_dish_use;
	gi.linkentity (ent);
}


/*QUAKED light_mine1 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine1 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex ("models/objects/minelite/light1/tris.md2");
	gi.linkentity (ent);
}


/*QUAKED light_mine2 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine2 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex ("models/objects/minelite/light2/tris.md2");
	gi.linkentity (ent);
}


/*QUAKED misc_gib_arm (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_arm (edict_t *ent)
{
	gi.setmodel (ent, "models/objects/gibs/arm/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random()*200;
	ent->avelocity[1] = random()*200;
	ent->avelocity[2] = random()*200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	gi.linkentity (ent);
}

/*QUAKED misc_gib_leg (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_leg (edict_t *ent)
{
	gi.setmodel (ent, "models/objects/gibs/leg/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random()*200;
	ent->avelocity[1] = random()*200;
	ent->avelocity[2] = random()*200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	gi.linkentity (ent);
}

/*QUAKED misc_gib_head (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_head (edict_t *ent)
{
	gi.setmodel (ent, "models/objects/gibs/head/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random()*200;
	ent->avelocity[1] = random()*200;
	ent->avelocity[2] = random()*200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	gi.linkentity (ent);
}

//=====================================================

/*QUAKED target_character (0 0 1) ?
used with target_string (must be on same "team")
"count" is position in the string (starts at 1)
*/

void SP_target_character (edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self->model);
	self->solid = SOLID_BSP;
	self->s.frame = 12;
	gi.linkentity (self);
	return;
}


/*QUAKED target_string (0 0 1) (-8 -8 -8) (8 8 8)
*/

void target_string_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *e;
	int		n, l;
	char	c;

	l = (int)strlen(self->message);
	for (e = self->teammaster; e; e = e->teamchain)
	{
		if (!e->count)
			continue;
		n = e->count - 1;
		if (n > l)
		{
			e->s.frame = 12;
			continue;
		}

		c = self->message[n];
		if (c >= '0' && c <= '9')
			e->s.frame = c - '0';
		else if (c == '-')
			e->s.frame = 10;
		else if (c == ':')
			e->s.frame = 11;
		else
			e->s.frame = 12;
	}
}

void SP_target_string (edict_t *self)
{
	if (!self->message)
		self->message = "";
	self->use = target_string_use;
}


/*QUAKED func_clock (0 0 1) (-8 -8 -8) (8 8 8) TIMER_UP TIMER_DOWN START_OFF MULTI_USE
target a target_string with this

The default is to be a time of day clock

TIMER_UP and TIMER_DOWN run for "count" seconds and the fire "pathtarget"
If START_OFF, this entity must be used before it starts

"style"		0 "xx"
			1 "xx:xx"
			2 "xx:xx:xx"
*/


#define	CLOCK_MESSAGE_SIZE	16



// don't let field width of any clock messages change, or it

// could cause an overwrite after a game load


static void func_clock_reset (edict_t *self)
{
	self->activator = NULL;
	if (self->spawnflags & 1)
	{
		self->health = 0;
		self->wait = self->count;
	}
	else if (self->spawnflags & 2)
	{
		self->health = self->count;
		self->wait = 0;
	}
}

static void func_clock_format_countdown (edict_t *self)
{
	if (self->style == 0)
	{
		Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%2i", self->health);
		return;
	}

	if (self->style == 1)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i", self->health / 60, self->health % 60);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		return;
	}

	if (self->style == 2)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", self->health / 3600, (self->health - (self->health / 3600) * 3600) / 60, self->health % 60);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		if (self->message[6] == ' ')
			self->message[6] = '0';
		return;
	}
}

void func_clock_think (edict_t *self)
{
	if (!self->enemy)
	{
		self->enemy = G_Find (NULL, FOFS(targetname), self->target);
		if (!self->enemy)
			return;
	}

	if (self->spawnflags & 1)
	{
		func_clock_format_countdown (self);
		self->health++;
	}
	else if (self->spawnflags & 2)
	{
		func_clock_format_countdown (self);
		self->health--;
	}
	else
	{
		struct tm	*ltime;

		time_t		gmtime;



		time(&gmtime);

		ltime = localtime(&gmtime);

		Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

		if (self->message[3] == ' ')

			self->message[3] = '0';

		if (self->message[6] == ' ')

			self->message[6] = '0';

	}

	self->enemy->message = self->message;
	self->enemy->use (self->enemy, self, self);

	if (((self->spawnflags & 1) && (self->health > self->wait)) ||
		((self->spawnflags & 2) && (self->health < self->wait)))
	{
		if (self->pathtarget)
		{
			char *savetarget;
			char *savemessage;

			savetarget = self->target;
			savemessage = self->message;
			self->target = self->pathtarget;
			self->message = NULL;
			G_UseTargets (self, self->activator);
			self->target = savetarget;
			self->message = savemessage;
		}

		if (!(self->spawnflags & 8))
			return;

		func_clock_reset (self);

		if (self->spawnflags & 4)
			return;
	}

	self->nextthink = level.time + 1;
}

void func_clock_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!(self->spawnflags & 8))
		self->use = NULL;
	if (self->activator)
		return;
	self->activator = activator;
	self->think (self);
}

void SP_func_clock (edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("%s with no target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	if ((self->spawnflags & 2) && (!self->count))
	{
		gi.dprintf("%s with no count at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	if ((self->spawnflags & 1) && (!self->count))
		self->count = 60*60;;

	func_clock_reset (self);

	self->message = gi.TagMalloc (CLOCK_MESSAGE_SIZE, TAG_LEVEL);


	self->think = func_clock_think;

	if (self->spawnflags & 4)
		self->use = func_clock_use;
	else
		self->nextthink = level.time + 1;
}


//=================================================================================



void teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)

{

	edict_t		*dest = NULL, *t = NULL;

	int			i, randnum;

	int			total;

	if (!other->client)

		return;
//	dest = G_Find (NULL, FOFS(targetname), self->target);


	total = 0;
	t = NULL;
	while ((t = G_Find (t, FOFS(targetname), self->target)))
	{
		if (t->inuse)
			total++;
	}

	randnum = (int) (random() * total) + 1;

	i = 0;

	t=NULL;

	while ((t = G_Find (t, FOFS(targetname), self->target)))
	{
		if (t->inuse)
			i++;

		if (i == randnum)
			dest = t;
	}



	if (!dest)

	{

		gi.dprintf ("Couldn't find destination\n");

		return;

	}



	// unlink to make sure it can't possibly interfere with KillBox

	gi.unlinkentity (other);



	VectorCopy (dest->s.origin, other->s.origin);

	VectorCopy (dest->s.origin, other->s.old_origin);

	other->s.origin[2] += 10;



	// clear the velocity and hold them in place briefly

	VectorClear (other->velocity);

	other->client->ps.pmove.pm_time = 160>>3;		// hold time

	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;



	// draw the teleport splash at source and on the player

//	self->owner->s.event = EV_PLAYER_TELEPORT;

//	other->s.event = EV_PLAYER_TELEPORT;



	// set angles

	for (i=0 ; i<3 ; i++)

		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);



	VectorClear (other->s.angles);

	VectorClear (other->client->ps.viewangles);

	VectorClear (other->client->v_angle);



	// kill anything at the destination

//faf	KillBox (other);


	//faf:  this fixes getting stuck in other players when teleporting:
	other->solid = SOLID_TRIGGER;
	//it works just like when spawning

	if (1)//other->client->resp.mos == SPECIAL || other->client->resp.team_on->chute == true)
	{
		other->client->landed = false;
		Spawn_Chute(other);
	}



	gi.linkentity (other);

}



/*QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)

Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.

*/

void SP_misc_teleporter (edict_t *ent)

{

	edict_t		*trig;



	if (!ent->target)

	{

		gi.dprintf ("teleporter without a target.\n");

		G_FreeEdict (ent);

		return;

	}



	gi.setmodel (ent, "models/objects/dmspot/tris.md2");

	ent->s.skinnum = 1;

//	ent->s.effects = EF_TELEPORTER;

	ent->s.sound = gi.soundindex ("world/amb10.wav");

	ent->solid = SOLID_BBOX;



	VectorSet (ent->mins, -32, -32, -24);

	VectorSet (ent->maxs, 32, 32, -16);

	gi.linkentity (ent);



	trig = G_Spawn ();

	trig->touch = teleporter_touch;

	trig->solid = SOLID_TRIGGER;

	trig->target = ent->target;

	trig->owner = ent;

	VectorCopy (ent->s.origin, trig->s.origin);

	VectorSet (trig->mins, -8, -8, 8);

	VectorSet (trig->maxs, 8, 8, 24);

	gi.linkentity (trig);



}



/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)

Point teleporters at these.

*/

void SP_misc_teleporter_dest (edict_t *ent)

{

	gi.setmodel (ent, "models/objects/dmspot/tris.md2");

	ent->s.skinnum = 0;

	ent->solid = SOLID_BBOX;

//	ent->s.effects |= EF_FLIES;

	VectorSet (ent->mins, -32, -32, -24);

	VectorSet (ent->maxs, 32, 32, -16);

	gi.linkentity (ent);

}

/*
HeadShotGib() by Wheaty
Used for when someone gets shot in the headz0r
*/
void HeadShotGib (edict_t *ent, vec3_t head_pos, vec3_t forward, vec3_t right, vec3_t player_vel)
{
	edict_t *gib;
	vec3_t s_up={1.0,0.0,3.0};


	level.gib_count++;//faf
	gib = G_Spawn();

	gi.setmodel (gib, "models/objects/gibs/brain/tris.md2");

	VectorNormalize(s_up);

	gib->solid = SOLID_NOT;
	gib->s.effects |= EF_GIB;
	gib->flags |= FL_NO_KNOCKBACK;
	gib->takedamage = DAMAGE_YES;
	gib->die = gib_die;

	gib->movetype = MOVETYPE_BOUNCE;
	gib->touch = gib_touch;

	// Brain will go up and to the right
	VectorCopy (head_pos, gib->s.origin);
	VectorScale (s_up, 300, gib->velocity);


	gib->avelocity[0] = random()*300;
	gib->avelocity[1] = random()*300;
	gib->avelocity[2] = random()*300;


	gib->clipmask = MASK_SHOT;
	gib->solid = SOLID_BBOX;


	gib->think = G_FreeEdict;
	gib->nextthink = level.time + 10;//45.0; //45 seconds

	gi.linkentity (gib);

}


// pbowens: surface types (based on psychospaz)

qboolean strcmpwld (char *give, char *check)
{
	int i, j, givenlength=0, checklength=0;

	givenlength = (int)strlen(give);
	checklength = (int)strlen(check);

	for (i = 0; i < givenlength; i++)
	{
		char checked[100];
		checked[0] = 0;

		for (j=i; j<checklength+i; j++)
		{
			// pbowens: use tolower() for editors like worldcraft
			Com_sprintf (checked, sizeof(checked), "%s%c", &checked, tolower(give[j]));
		}
		if (!strcmp(check, (char *)(&checked)))
			return true;
	}

	return false;
}

qboolean Surface(char *name, int type)
{

	switch (type)
	{
	case SURF_METAL:
		if (strcmpwld (name, "metal"))
			return true;
		if (strcmpwld (name, "hedgehog"))
			return true;
		if (strcmpwld (name, "pantheriv"))
			return true;
		if (strcmpwld (name, "base_bkup"))
			return true;
		if (strcmpwld (name, "trk_bed"))
			return true;
/*		if (strcmpwld (name, "tile10"))
			return true;
		if (strcmpwld (name, "tile11"))
			return true;   */
		if (strcmpwld (name, "duct"))
			return true;
/*		if (strcmpwld (name, "glider"))
			return true;*/
		if (strcmpwld (name, "barrel"))
			return true;
		if (strcmpwld (name, "colpiron"))
			return true;
		if (strcmpwld (name, "lcp"))
			return true;
		if (strcmpwld (name, "ammo"))
			return true;
		if (strcmpwld (name, "door1"))
			return true;
		if (strcmpwld (name, "trk"))
			return true;
		if (strcmpwld (name, "pipe"))
			return true;
		if (strcmpwld (name, "tile10"))
			return true;
		if (strcmpwld (name, "boat"))
			return true;
		if (strcmpwld (name, "mtlpanel"))
			return true;
		if (strcmpwld (name, "lcvp"))
			return true;
		break;

	case SURF_GRASS:
		if (strcmpwld (name, "grass"))
			return true;
		if (strcmpwld (name, "ground1"))
			return true;
		if (strcmpwld (name, "hedge"))
			return true;
		if (strcmpwld (name, "redcarp"))
			return true;
		if (strcmpwld (name, "carpet"))
			return true;
		if (strcmpwld (name, "rug"))
			return true;
		if (strcmpwld (name, "bush"))
			return true;
		break;

	case SURF_WOOD:
		if (strcmpwld (name, "wood"))
			return true;
		if (strcmpwld (name, "wdfence"))
			return true;
		if (strcmpwld (name, "k_roof01"))
			return true;
		if (strcmpwld (name, "roof1"))
			return true;
		if (strcmpwld (name, "floor3"))
			return true;
		if (strcmpwld (name, "mr_floor10"))
			return true;
		if (strcmpwld (name, "a3"))
			return true;
		if (strcmpwld (name, "wdv256c"))
			return true;
		if (strcmpwld (name, "base_bkend"))
			return true;
		if (strcmpwld (name, "sandyrockboard"))
			return true;
		if (strcmpwld (name, "rf_sr_mw1"))
			return true;
		if (strcmpwld (name, "crate"))
			return true;
		if (strcmpwld (name, "ladder"))
			return true;
		if (strcmpwld (name, "bark"))
			return true;
		if (strcmpwld (name, "glider"))
			return true;
		if (strcmpwld (name, "box"))
			return true;
		if (strcmpwld (name, "shutter"))
			return true;
		if (strcmpwld (name, "k2x_woo"))
			return true;
		if (strcmpwld (name, "door") &&
			!strcmpwld (name, "dday3/door1"))
			return true;
		break;



	case SURF_SAND:
		if (strcmpwld (name, "sand"))
			return true;
		if (strcmpwld (name, "cf_m_asphalt1"))
			return true;
		if (strcmpwld (name, "mr_rock3"))
			return true;
		if (strcmpwld (name, "rubble"))
			return true;
		if (strcmpwld (name, "dirt"))
			return true;
		if (strcmpwld (name, "mud"))
			return true;
		if (strcmpwld (name, "white"))
			return true;
		if (strcmpwld (name, "snow"))
			return true;
		if (strcmpwld (name, "leave"))
			return true;
		break;

	case SURF_GLASS:
		if (strcmpwld (name, "window") &&
			!(strcmpwld (name, "dday2/window2")))
			return true;
		if (strcmpwld (name, "glass"))
			return true;
		if (strcmpwld (name, "staing"))
			return true;
		if (strcmpwld (name, "winlight"))
			return true;
		if (strcmpwld (name, "dday2/a10"))
			return true;
		break;

	}
	return false;
}

int Surface2(char *name)
{
	if (strcmpwld (name, "metal"))
		return SURF_METAL;
	if (strcmpwld (name, "hedgehog"))
		return SURF_METAL;
	if (strcmpwld (name, "pantheriv"))
		return SURF_METAL;
	if (strcmpwld (name, "base_bkup"))
		return SURF_METAL;
	if (strcmpwld (name, "trk_bed"))
		return SURF_METAL;
/*		if (strcmpwld (name, "tile10"))
		return SURF_METAL;
	if (strcmpwld (name, "tile11"))
		return SURF_METAL;   */
	if (strcmpwld (name, "duct"))
		return SURF_METAL;
/*		if (strcmpwld (name, "glider"))
		return SURF_METAL;*/
	if (strcmpwld (name, "barrel"))
		return SURF_METAL;
	if (strcmpwld (name, "colpiron"))
		return SURF_METAL;
	if (strcmpwld (name, "lcp"))
		return SURF_METAL;
	if (strcmpwld (name, "ammo"))
		return SURF_METAL;
	if (strcmpwld (name, "door1"))
		return SURF_METAL;
	if (strcmpwld (name, "trk"))
		return SURF_METAL;
	if (strcmpwld (name, "pipe"))
		return SURF_METAL;
	if (strcmpwld (name, "tile10"))
		return SURF_METAL;
	if (strcmpwld (name, "boat"))
		return SURF_METAL;
	if (strcmpwld (name, "mtlpanel"))
		return SURF_METAL;
	if (strcmpwld (name, "lcvp"))
		return SURF_METAL;



	if (strcmpwld (name, "grass"))
		return SURF_GRASS;
	if (strcmpwld (name, "ground1"))
		return SURF_GRASS;
	if (strcmpwld (name, "hedge"))
		return SURF_GRASS;
	if (strcmpwld (name, "redcarp"))
		return SURF_GRASS;
	if (strcmpwld (name, "carpet"))
		return SURF_GRASS;
	if (strcmpwld (name, "rug"))
		return SURF_GRASS;
	if (strcmpwld (name, "bush"))
		return SURF_GRASS;



	if (strcmpwld (name, "wood"))
		return SURF_WOOD;
	if (strcmpwld (name, "wdfence"))
		return SURF_WOOD;
	if (strcmpwld (name, "k_roof01"))
		return SURF_WOOD;
	if (strcmpwld (name, "roof1"))
		return SURF_WOOD;
	if (strcmpwld (name, "floor3"))
		return SURF_WOOD;
	if (strcmpwld (name, "mr_floor10"))
		return SURF_WOOD;
	if (strcmpwld (name, "a3"))
		return SURF_WOOD;
	if (strcmpwld (name, "wdv256c"))
		return SURF_WOOD;
	if (strcmpwld (name, "base_bkend"))
		return SURF_WOOD;
	if (strcmpwld (name, "sandyrockboard"))
		return SURF_WOOD;
	if (strcmpwld (name, "rf_sr_mw1"))
		return SURF_WOOD;
	if (strcmpwld (name, "crate"))
		return SURF_WOOD;
	if (strcmpwld (name, "ladder"))
		return SURF_WOOD;
	if (strcmpwld (name, "bark"))
		return SURF_WOOD;
	if (strcmpwld (name, "glider"))
		return SURF_WOOD;
	if (strcmpwld (name, "box"))
		return SURF_WOOD;
	if (strcmpwld (name, "shutter"))
		return SURF_WOOD;
	if (strcmpwld (name, "k2x_woo"))
		return SURF_WOOD;
	if (strcmpwld (name, "door") &&
		!strcmpwld (name, "dday3/door1"))
		return SURF_WOOD;



	if (strcmpwld (name, "sand"))
		return SURF_SAND;
	if (strcmpwld (name, "cf_m_asphalt1"))
		return SURF_SAND;
	if (strcmpwld (name, "mr_rock3"))
		return SURF_SAND;
	if (strcmpwld (name, "rubble"))
		return SURF_SAND;
	if (strcmpwld (name, "dirt"))
		return SURF_SAND;
	if (strcmpwld (name, "mud"))
		return SURF_SAND;
	if (strcmpwld (name, "white"))
		return SURF_SAND;
	if (strcmpwld (name, "snow"))
		return SURF_SAND;
	if (strcmpwld (name, "leave"))
		return SURF_SAND;



	if (strcmpwld (name, "window") &&
		!(strcmpwld (name, "dday2/window2")))
		return SURF_GLASS;
	if (strcmpwld (name, "glass"))
		return SURF_GLASS;
	if (strcmpwld (name, "staing"))
		return SURF_GLASS;
	if (strcmpwld (name, "winlight"))
		return SURF_GLASS;
	if (strcmpwld (name, "dday2/a10"))
		return SURF_GLASS;

	return -1;
}


void spawn_toggle_use (edict_t *self, edict_t *other, edict_t *activator)
{
    edict_t *x;

	//gi.dprintf("%s %s\n",activator->classname, other->classname);

	if (self->style == 2 && activator->client)
	{
		//bulgef thing: only switch if the activating person's nearest i_r_s belongs to opposite team.
		edict_t *e = NULL, *nearest = NULL;
		float temp_distance,nearest_distance;
		vec3_t dist;

		nearest_distance = 9999999999;
		for (e = g_edicts; e < &g_edicts[globals.num_edicts]; e++)
		{
			if (!e->inuse)
				continue;
			if (strcmp(e->classname, "info_reinforcements_start"))
				continue;

			VectorSubtract (e->s.origin, other->s.origin, dist);

			temp_distance = VectorLength(dist);
			//gi.dprintf("%f\n",temp_distance);

			if (temp_distance < nearest_distance)
			{
				nearest_distance = temp_distance;
				nearest = e;
			}
		}
		if (nearest)
		{
			if (activator->client->resp.team_on &&
				nearest->obj_owner == activator->client->resp.team_on->index)
				return;
			//gi.dprintf ("%s\n",vtos(nearest->s.origin));

		}
	}


	for (x = g_edicts; x < &g_edicts[globals.num_edicts]; x++)
    {
       if (!x->inuse)
		   continue;

	   if (!x->classname)
		   continue;

	   if (!strcmp(x->classname, "info_reinforcements_start") || !strcmp(x->classname, "bot_spawn"))
	   {
			if (self->style !=2)
			{
				if (x->obj_owner > -1)
					x->obj_owner = -99;
			}
	   }

	   if (!x->targetname)
		   continue;

	   if (self->style == 2)
	   {
			if (x->obj_owner == 1)
				x->obj_owner = 0;
			else if (x->obj_owner == 0)
				x->obj_owner = 1;
	   }



		if (!strcmp(x->targetname, self->target))
		{
			if (x->obj_owner == -2)
				x->obj_owner = 1;
			else if (x->obj_owner == -1)
				x->obj_owner = 0;
		}
	}

}

void SP_Spawn_Toggle (edict_t *self)
{
	self->use = spawn_toggle_use;
}



void Medic_Screen (edict_t *ent)
{
	int			i,j,y;
	char	string[1024];
	char	entry[1024];
	int	stringlength;
	char  *bp;
	edict_t *e;

	vec3_t	vec;
	float	dot,xdot;
	vec3_t	forward,right;
	vec3_t	a,b;
	int iconpos;

	qboolean qleft,qforward,qback,qright;
	qleft = qforward = qback = qright = false;

	//JABot[start]
	if (ent->ai || !ent->inuse || !ent->client)
		return;
	//[end]


	if (!ent->client->resp.team_on)
		return;

	if (ent->client->menu) {
		PMenu_Close(ent);
		return;
	}



	string[0] = 0;

	// send the layout
//	Com_sprintf (string, sizeof(string),
//		"xv 32 yv 8 picn scorehead ");

    Com_sprintf (string, sizeof(entry),	"xr -200 160 yt 120  string \" Wounded Teammates!\" xr -200 160 yt 130  string \" ------------------\" ");


	stringlength = (int)strlen(string);

	y=2;
	for (i=0 ; i < game.maxclients ; i++)
	{
		e = g_edicts + 1 + i;
		if (!e->client)
			continue;
		if (!e->inuse)
			continue;
		if (e->health > 99 || e->health < 1)
			continue;
		if (e == ent)
			continue;
		if (!e->client->resp.team_on)
			continue;
		if (e->client->resp.team_on->index != ent->client->resp.team_on->index)
			continue;


		//put * next to bleeders
		if (e->die_time)
		{ bp ="*";}
		else
		{ bp = " ";}


		VectorCopy (e->s.origin, a);
		VectorCopy (ent->s.origin, b);
		a[2]=0;
		b[2]=0;

		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorSubtract (a, b, vec);
		VectorNormalize (vec);

		dot = DotProduct (vec, forward);
		if (dot > 0.9)
			qforward = true;
		else if (dot < .1)
			qback = true;
/*		else
		{
			dot = DotProduct (vec, right);
			if (dot >.1)
				qright = true;
			else if (dot < -.1)
				qleft = true;
		}*/
		//gi.dprintf("%f\n",dot);

		xdot = DotProduct (vec, right);

		if (dot > .92)
		{
			iconpos = 1228 * xdot + 160;
			Com_sprintf (entry, sizeof(entry),	"xr -200 yt %i string \"%s%-22.22s\" 	xv %i yb -150 picn i_medic ",120+(y*10),bp,e->client->pers.netname, iconpos);
		}
		else if (xdot > 0) //wounded dude is offscreen to right
			Com_sprintf (entry, sizeof(entry),	"xr -200 yt %i string \"%s%-22.22s\" 	xr -30 yb -150 picn i_medic ",120+(y*10),bp,e->client->pers.netname);
		else
			Com_sprintf (entry, sizeof(entry),	"xr -200 yt %i string \"%s%-22.22s\" 	xl 10 yb -150 picn i_medic ",120+(y*10),bp,e->client->pers.netname);


		//if (lookingtoward(ent,e))
//			Com_sprintf (entry, sizeof(entry),	"xr -200 yt %i string \"%s%-22.22s\" ",120+(y*10),bp,e->client->pers.netname);
		//else
		//	Com_sprintf (entry, sizeof(entry),	"xr -200 yt %i string2 \"%s%-22.22s\" ",120+(y*10),bp,e->client->pers.netname);

		j = (int)strlen(entry);
		if (stringlength + j > 1024)
			break;
	//	strncpy (string + stringlength, entry);
		Q_strncpyz (string + stringlength, sizeof(string) - stringlength, entry);
		stringlength += j;
		y++;

	}


	/*

	if (qforward)
	{
		Com_sprintf (entry, sizeof(entry),	"xv 160 yt 100 picn i_medic ");
		j = strlen(entry);
		if (stringlength + j < 1024)
		{
		//	strncpy (string + stringlength, entry);
			Q_strncpyz (string + stringlength, sizeof(string) - stringlength, entry);
			stringlength += j;
		}

	}
	if (qback)
	{
		Com_sprintf (entry, sizeof(entry),	"xv 160 yb -100 picn i_medic ");
		j = strlen(entry);
		if (stringlength + j < 1024)
		{
		//	strncpy (string + stringlength, entry);
			Q_strncpyz (string + stringlength, sizeof(string) - stringlength, entry);
			stringlength += j;
		}

	}
	if (qright)
	{
		Com_sprintf (entry, sizeof(entry),	"xr -100 yv 100 picn i_medic ");
		j = strlen(entry);
		if (stringlength + j < 1024)
		{
		//	strncpy (string + stringlength, entry);
			Q_strncpyz (string + stringlength, sizeof(string) - stringlength, entry);
			stringlength += j;
		}
	}
	if (qleft)
	{
		Com_sprintf (entry, sizeof(entry),	"xl 100 yv 100 picn i_medic ");
		j = strlen(entry);
		if (stringlength + j < 1024)
		{
		//	strncpy (string + stringlength, entry);
			Q_strncpyz (string + stringlength, sizeof(string) - stringlength, entry);
			stringlength += j;
		}

	}
*/

	if (y == 2)// no wounded teammates
	{
	//	ent->client->layout_type = SHOW_NONE;
	//	return;
		Com_sprintf (string, sizeof(string),	" ");

	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
