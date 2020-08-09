// g_misc.c

#include "g_local.h"
#include "m_gekk.h"

int	gibsthisframe = 0;
int lastgibframe = 0;

#define GIB_METAL   1
#define GIB_GLASS   2
#define GIB_BARREL  3
#define GIB_CRATE   4
#define GIB_ROCK    5
#define GIB_CRYSTAL 6
#define GIB_MECH    7
#define GIB_WOOD    8
#define GIB_TECH    9

extern void M_WorldEffects (edict_t *ent);

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

//Knightmare- gib fade
#ifdef KMQUAKE2_ENGINE_MOD
void gib_fade2 (edict_t *self);

void gib_fade (edict_t *self)
{
	if (self->s.effects & EF_BLASTER) // Remove glow from gekk gibs
	{
		self->s.effects &= ~EF_BLASTER;
		self->s.renderfx &= ~RF_NOSHADOW;
	}
	if (self->s.renderfx & RF_TRANSLUCENT)
		self->s.alpha = 0.70f;
	else if (self->s.effects & EF_SPHERETRANS)
		self->s.alpha = 0.30f;
	else if ( !(self->s.alpha) || (self->s.alpha <= 0.0f) || (self->s.alpha > 1.0f) )
		self->s.alpha = 1.00f;
//	gib_fade2 (self);
	self->nextthink = level.time + 0.2;
	self->think = gib_fade2;
}

void gib_fade2 (edict_t *self)
{
	self->s.alpha -= 0.05f;
	self->s.alpha = max(self->s.alpha, 1/255);
	if (self->s.alpha <= 1/255)
	{
		G_FreeEdict (self);
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
	if (self->s.effects & EF_BLASTER) //Remove glow from gekk gibs
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
	self->think = G_FreeEdict;
	gi.linkentity(self);
}
#endif

void gib_think (edict_t *self)
{
	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame == 10)
	{
		self->think = gib_fade; //was G_FreeEdict
		self->nextthink = level.time + 8 + random()*10;
	}
}

void gib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	normal_angles, right;

// RAFAEL 24-APR-98
// removed acid damage
#if 0
// RAFAEL
	if (other->takedamage && (self->s.effects & EF_GREENGIB))
	{
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
		G_FreeEdict (self);
	}
#endif

	if (!self->groundentity)
		return;

//	self->touch = NULL;

	if (plane)
	{
	//	gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/fhit3.wav"), 1, ATTN_NORM, 0);

		vectoangles (plane->normal, normal_angles);
		AngleVectors (normal_angles, NULL, right, NULL);
		vectoangles (right, self->s.angles);

		if (self->s.modelindex == sm_meat_index)
		{
			self->s.frame++;
			self->think = gib_think;
			self->nextthink = level.time + FRAMETIME;
		}
	}
}

void gib_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict (self);
}

void ThrowGib (edict_t *self, char *gibname, int damage, int type)
{
	edict_t *gib;
	vec3_t	vd;
	vec3_t	origin;
	vec3_t	size;
	float	vscale;
	char	modelname[256];
	char	*p;

	// Lazarus: Prevent gib showers (generally due to firing BFG in a crowd) from
	// causing SZ_GetSpace: overflow
	if(level.framenum > lastgibframe)
	{
		gibsthisframe = 0;
		lastgibframe = level.framenum;
	}
	gibsthisframe++;
	if (gibsthisframe > sv_maxgibs->value)
		return;

	gib = G_Spawn();

	//gib->classname = "gib";
	gib->classname = gi.TagMalloc (4, TAG_LEVEL);
	strcpy(gib->classname, "gib");

	// Lazarus: mapper-definable gib class
	Com_strcpy(modelname, sizeof(modelname), gibname);
	p = strstr(modelname, "models/objects/gibs/");
	if(p && self->gib_type)
	{
		p += 18;
		p[0] = (char)((int)'0' + (self->gib_type % 10));
	}

	// Knightmare- this causes a crash when saving game
	// Save gibname and type for level transition gibs
	//gib->key_message = gi.TagMalloc (strlen(modelname)+1,TAG_LEVEL);
	//strcpy(gib->key_message, modelname);
	gib->style = type;

	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);
	gib->s.origin[0] = origin[0] + crandom() * size[0];
	gib->s.origin[1] = origin[1] + crandom() * size[1];
	gib->s.origin[2] = origin[2] + crandom() * size[2];

	//gi.setmodel (gib, gibname);
	gib->s.modelindex = gi.modelindex (gibname);
	gib->clipmask = MASK_SHOT;
	VectorSet (gib->mins, -4, -4, -4);
	VectorSet (gib->maxs, 4, 4, 4);
	gib->solid = SOLID_TRIGGER;	// Knightmare- was SOLID_NOT

	if (self->blood_type == 1)
	{
		gib->s.effects |= EF_GREENGIB|EF_BLASTER;
		gib->s.renderfx |= RF_NOSHADOW;
	}
	else if(self->blood_type == 2)
		gib->s.effects |= EF_GRENADE;
	else
		gib->s.effects |= EF_GIB;

	// inherit translucency flags from parent entity
	if (self->s.renderfx & RF_TRANSLUCENT)
		gib->s.renderfx |= RF_TRANSLUCENT;
	if (self->s.effects & EF_SPHERETRANS)
		gib->s.effects |= EF_SPHERETRANS;

#ifdef KMQUAKE2_ENGINE_MOD
	// Knightmare- translucent monsters throw translucent gibs
	if ( (self->s.alpha) && (self->s.alpha > 0.0f) && (self->s.alpha < 1.0f) )
		gib->s.alpha = self->s.alpha;
#endif
	gib->flags |= FL_NO_KNOCKBACK;
	gib->svflags |= SVF_GIB; // Knightmare- gib flag
	gib->takedamage = DAMAGE_NO;
	gib->die = gib_die;
	if(self->blood_type == 1)
		gib->dmg = 2;

	if (type == GIB_ORGANIC)
	{
		gib->movetype = MOVETYPE_BOUNCE; // was MOVETYPE_TOSS
		gib->touch = gib_touch;
		if(self->blood_type == 1)
			vscale = 3.0;
		else
			vscale = 1.0;
	}
	else
	{
		gib->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	VelocityForDamage (damage, vd);
	VectorMA (self->velocity, vscale, vd, gib->velocity);
	ClipGibVelocity (gib);
	gib->avelocity[0] = random()*600;
	gib->avelocity[1] = random()*600;
	gib->avelocity[2] = random()*600;

	gib->think = gib_fade; //Knightmare- gib fade, was G_FreeEdict
	gib->nextthink = level.time + 10 + random()*10;

//PGM
	gib->s.renderfx |= RF_IR_VISIBLE;
//PGM

	gi.linkentity (gib);
}

// NOTE: SP_gib is ONLY intended to be used for gibs that change maps
//       via trigger_transition. It should NOT be used for map entities.

void gib_delayed_start (edict_t *gib)
{
	if(g_edicts[1].linkcount)
	{
		if(gib->count > 0)
		{
			gib->think = gib_fade;
			gib->nextthink = level.time + FRAMETIME;
		}
		else
		{
			gib->think = gib_fade;
			gib->nextthink = level.time + 8 + random()*10;
		}
	}
	else
	{
		gib->nextthink = level.time + FRAMETIME;
	}
}

void SP_gib (edict_t *gib)
{
	if(gib->key_message)
		gi.setmodel (gib, gib->key_message);
	else
		gi.setmodel (gib, "models/objects/gibs/sm_meat/tris.md2");
	gib->die = gib_die;
	if (gib->style == GIB_ORGANIC)
		gib->touch = gib_touch;
	gib->think = gib_delayed_start;
	gib->nextthink = level.time + FRAMETIME;
	gi.linkentity (gib);
}

void ThrowHead (edict_t *self, char *gibname, int damage, int type)
{
	vec3_t	vd;
	float	vscale;
	char	modelname[256];
	char	*p;

	// Lazarus reflections
	DeleteReflection (self,-1);

	if (self->movewith) // Knightmare- remove stuff for movewith monsters
	{
		if (self->oldmovetype) // restore movetype
			self->movetype = self->oldmovetype;
		self->movewith = NULL;
		VectorClear(self->movewith_offset);
		self->movewith_set = 0;
	}
	self->s.skinnum = 0;
	self->s.frame = 0;
	VectorClear (self->mins);
	VectorClear (self->maxs);

	Com_strcpy(modelname, sizeof(modelname), gibname);
	p = strstr(modelname, "models/objects/gibs/");
	if(p && self->gib_type)
	{
		p += 18;
		p[0] = (char)((int)'0' + (self->gib_type % 10));
	}

	// Knightmare- this causes a crash when saving game
	// Save gibname and type for level transition gibs
//	self->key_message = gi.TagMalloc (strlen(modelname)+1,TAG_LEVEL);
//	strcpy(self->key_message, modelname);
	self->style = type;

	self->s.modelindex2 = 0;
	gi.setmodel (self, gibname);

	self->clipmask = MASK_SHOT;
	if (self->blood_type == 1)
	{
		VectorSet (self->mins, -4, -4, -4);
		VectorSet (self->maxs, 4, 4, 4);
	}
	else if (self->flags & FL_Q1_MONSTER) // Q1 monster heads have bigger bbox
	{
		VectorSet (self->mins, -16, -16, 0);
		VectorSet (self->maxs, 16, 16, 32);
	}
	else
	{
		VectorSet (self->mins, -4, -4, 0);
		VectorSet (self->maxs, 4, 4, 8);
	}
	self->solid = SOLID_TRIGGER;	// Knightmare- was SOLID_NOT
	if(self->blood_type == 1)
	{
		self->s.effects |= EF_GREENGIB|EF_BLASTER;
		self->s.renderfx |= RF_NOSHADOW;
	}
	else if(self->blood_type == 2)
		self->s.effects |= EF_GRENADE;
	else
		self->s.effects |= EF_GIB;
	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;
	self->svflags &= ~SVF_MONSTER;
	self->svflags |= SVF_GIB; // Knightmare- gib flag
	self->takedamage = DAMAGE_NO;
	// Lazarus: Disassociate this head with its monster
	self->targetname = NULL;
	self->die = gib_die;
	self->dmgteam = NULL; // Prevent gibs from becoming angry if their buddies are hurt
	self->postthink = NULL;	// Knightmare- stop lava check

	if (type == GIB_ORGANIC)
	{
		self->movetype = MOVETYPE_BOUNCE; //was MOVETYPE_TOSS
		self->touch = gib_touch;
		if(self->blood_type == 1)
			vscale = 3.0;
		else
			vscale = 1.0; //was 0.5
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

	if (self->flags & FL_Q1_MONSTER) // Q1 monster heads don't disappear
	{
		self->think = NULL;
		self->nextthink = 0;
	}
	else
	{
		self->think = gib_fade; // Knightmare- gib fade, was G_FreeEdict
		self->nextthink = level.time + 10 + random()*10;
	}

//PGM
	self->s.renderfx |= RF_IR_VISIBLE;
//PGM

	gi.linkentity (self);
}

// NOTE: SP_gibhead is ONLY intended to be used for gibs that change maps
//       via trigger_transition. It should NOT be used for map entities.

void SP_gibhead (edict_t *gib)
{
	if(gib->key_message)
		gi.setmodel (gib, gib->key_message);
	else
		gi.setmodel (gib, "models/objects/gibs/head2/tris.md2");

	if (gib->style == GIB_ORGANIC)
		gib->touch = gib_touch;

	gib->think = gib_delayed_start;
	gib->nextthink = level.time + FRAMETIME;
	gi.linkentity (gib);
}

void ThrowClientHead (edict_t *self, int damage)
{
	vec3_t	vd;
	char	*gibname;

	if (self->movewith) // Knightmare- remove stuff for movewith monsters
	{
		if (self->oldmovetype) //restore movetype
			self->movetype = self->oldmovetype;
		self->movewith = NULL;
		VectorClear(self->movewith_offset);
		self->movewith_set = 0;
	}
	if (rand()&1)
	{
		gibname = "models/objects/gibs/head2/tris.md2";
		self->s.skinnum = 1;		// second skin is player
	}
	else
	{
		gibname = "models/objects/gibs/skull/tris.md2";
		self->s.skinnum = 0;
	}
	self->s.origin[2] += 32;
	self->s.frame = 0;
	gi.setmodel (self, gibname);
	VectorSet (self->mins, -4, -4, -4);
	VectorSet (self->maxs, 4, 4, 4);

	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_TRIGGER;	// Knightmare- was SOLID_NOT
	self->s.effects = EF_GIB;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;
	self->svflags |= SVF_GIB; // Knightmare- gib flag
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
		self->think = NULL;
		self->nextthink = 0;
	}

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

void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin, int skin, int effects)
{
	edict_t	*chunk;
	vec3_t	v;

	// Lazarus: Prevent gib showers (generally due to firing BFG in a crowd) from
	// causing SZ_GetSpace: overflow
	if(level.framenum > lastgibframe)
	{
		gibsthisframe = 0;
		lastgibframe = level.framenum;
	}
	gibsthisframe++;
	if (gibsthisframe > sv_maxgibs->value)
		return;

	chunk = G_Spawn();
	VectorCopy (origin, chunk->s.origin);
	gi.setmodel (chunk, modelname);

	// inherit translucency flags from parent entity
	if (self->s.renderfx & RF_TRANSLUCENT)
		chunk->s.renderfx |= RF_TRANSLUCENT;
	if (self->s.effects & EF_SPHERETRANS)
		chunk->s.effects |= EF_SPHERETRANS;

#ifdef KMQUAKE2_ENGINE_MOD
	// Knightmare- translucent entities throw translucent debris
	if ( (self->s.alpha) && (self->s.alpha > 0.0f) && (self->s.alpha < 1.0f) )
		chunk->s.alpha = self->s.alpha;
#endif
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	VectorMA (self->velocity, speed, v, chunk->velocity);
	chunk->movetype = MOVETYPE_BOUNCE;
	VectorSet (chunk->mins, -1, -1, -1);
	VectorSet (chunk->maxs, 1, 1, 1);
	chunk->solid = SOLID_TRIGGER;	// Knightmare- was SOLID_NOT
	chunk->avelocity[0] = random()*600;
	chunk->avelocity[1] = random()*600;
	chunk->avelocity[2] = random()*600;
	chunk->think = gib_fade; // Knightmare- gib fade, was G_FreeEdict
	chunk->nextthink = level.time + 8 + random()*10;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "debris";
	chunk->svflags |= SVF_GIB; // Knightmare- gib flag
	chunk->takedamage = DAMAGE_NO; // was DAMAGE_YES
	chunk->die = debris_die;

	// Lazarus: skin number and effects
	chunk->s.skinnum = skin;
	chunk->s.effects |= effects;

	gi.linkentity (chunk);
}

// NOTE: SP_debris is ONLY intended to be used for debris chunks that change maps
//       via trigger_transition. It should NOT be used for map entities.

void debris_delayed_start (edict_t *debris)
{
	if(g_edicts[1].linkcount)
	{
		debris->think = gib_fade;
		debris->nextthink = level.time + 8 + random()*5;
	}
	else
	{
		debris->nextthink = level.time + FRAMETIME;
	}
}

void SP_debris (edict_t *debris)
{
	if(debris->message)
		gi.setmodel (debris, debris->message);
	else
		gi.setmodel (debris, "models/objects/debris2/tris.md2");
	debris->think = debris_delayed_start;
	debris->nextthink = level.time + FRAMETIME;
	debris->die = debris_die;
	gi.linkentity (debris);
}

void BecomeExplosion1 (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// Lazarus explosions
	if (level.num_reflectors)
		ReflectExplosion (TE_EXPLOSION1, self->s.origin);

	G_FreeEdict (self);
}


void BecomeExplosion2 (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION2);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// Lazarus explosions
	if (level.num_reflectors)
		ReflectExplosion (TE_EXPLOSION1, self->s.origin);

	G_FreeEdict (self);
}

void BecomeExplosion3 (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1_BIG);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// Lazarus explosions
	if (level.num_reflectors)
		ReflectExplosion (TE_EXPLOSION1, self->s.origin);

	G_FreeEdict (self);
}

/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT NO_ROTATE
TELEPORT: The enity will teleport to this location
NO_ROTATE: a train with ROTATE spawnflag will not angle toward this waypoint

"target"	next path corner
"pathtarget" gets used when an entity that has this path_corner targeted touches it
"wait"		number of seconds before continuing; -1 = wait for retrigger
"speed"		speed of train
"accel"		acceleration of train
"decel"		deceleration of train
"pitch_speed" (Nose up & Down) in degrees per second (additive for ROT_CONST trains)
"yaw_speed" (Side-to-side "wiggle") in degrees per second (additive for ROT_CONST trains)
"roll_speed" (Banking) in degrees per second (additive for ROT_CONST trains)
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

	// Lazarus: Distinguish between path_corner and target_actor, or target_actor
	//          with JUMP spawnflag will result in teleport. Ack!
	if ((next) && (next->spawnflags & 1) && !Q_stricmp(next->classname,"path_corner"))
	{
		VectorCopy (next->s.origin, v);
		v[2] += next->mins[2];
		v[2] -= other->mins[2];
		VectorCopy (v, other->s.origin);
		next = G_PickTarget(next->target);
		other->s.event = EV_OTHER_TELEPORT;
	}

	other->goalentity = other->movetarget = next;

	if (self->wait > 0)
	{
		// Zaero
		if ( IsZaeroMap() && (other->goalentity) )
		{
			VectorSubtract (other->goalentity->s.origin, other->s.origin, v);
			other->ideal_yaw = vectoyaw (v);
		}
		// end Zaero

		other->monsterinfo.pausetime = level.time + self->wait;
		other->monsterinfo.stand (other);
		return;
	}
	else if (self->wait < 0)
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		other->monsterinfo.stand (other);
	}
	else
	{
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
	self->count--;
	if (self->count == 0)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 1;
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


/*QUAKED point_combat (.5 .3 0) (-8 -8 -8) (8 8 8) Hold DriveTrain
Makes this the target of a monster and it will head here when first activated before going after the activator. If hold is selected, it will stay here.

"count" number of times it can be used
"pathtarget" targetname of entity to be triggered when the point_combat is used.
"target" targetname of the next point_combat in the path
"targetname" name of the specific point_combat
*/
void tracktrain_drive(edict_t *train, edict_t *other);
void point_combat_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*activator;

	if (other->movetarget != self)
		return;

	self->count--;
	if(self->count == 0)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + self->delay + 1;
	}

	if (self->target)
	{
		other->target = self->target;
		other->goalentity = other->movetarget = G_PickTarget(other->target);
		if (!other->goalentity)
		{
			gi.dprintf("%s at %s target %s does not exist\n", self->classname, vtos(self->s.origin), self->target);
			other->movetarget = self;
		}
		// Lazarus: NO NO NO! This makes the pc's target accessible once and only once
		//self->target = NULL;
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
		if (self->spawnflags & 2)
		{
			edict_t	*train;
			train = G_PickTarget(self->pathtarget);
			if(train)
				tracktrain_drive(train,other);
		}
		else
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

/*static*/ void light_use (edict_t *self, edict_t *other, edict_t *activator)
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
		self->count--;
		if (self->count == 0)
		{
			self->think = G_FreeEdict;
			self->nextthink = level.time + 1;
		}
	}
}

void SP_light (edict_t *self)
{
	// no targeted lights in deathmatch, because they cause global messages
	if (!self->targetname || deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

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
		self->count--;
		if (self->count == 0)
		{
			self->think = G_FreeEdict;
			self->nextthink = level.time + 1;
			return;
		}
	}
	gi.linkentity (self);

	if (!(self->spawnflags & 2))
		self->use = NULL;
}

void SP_func_wall (edict_t *self)
{
	self->class_id = ENTITY_FUNC_WALL;

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
	//	gi.dprintf("func_wall missing TRIGGER_SPAWN\n");
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

#define	TRAIN_ROTATE		8

/*QUAKED func_explosive (0 .5 .8) ? Trigger_Spawn Animated Animated_Fast Inactive Explosive_Only
Any brush that you want to explode or break apart.  If you want an
ex0plosion, set dmg and it will do a radius explosion of that amount
at the center of the bursh.

If targeted it will not be shootable, unless the inactive flag is set.

INACTIVE - Specifies that the entity is not explodable until triggered. If you use this you must
target the entity you want to trigger it. This is the only entity approved to activate it.

health defaults to 100.

mass defaults to 75.  This determines how much debris is emitted when
it explodes.  You get one large chunk per 100 of mass (up to 8) and
one small chunk per 25 of mass (up to 16).  So 800 gives the most.

delay- How long to wait before exploding, useful for a chain of func_explosives.
gib_type- Set to 1 for metal, 2 for glass, 3 for barrel, 4 for crate, 5 for rock, 6 for crystal
			7 for mechanical, 8 for wood, 9 for tech gibs.
skinnum- Skin number for the gibs to use.
*/
void train_kill_children (edict_t *self);
void turret_breach_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

void func_explosive_use(edict_t *self, edict_t *other, edict_t *activator);
void func_explosive_respawn(edict_t *self)
{
    KillBox(self);
    self->solid = SOLID_BSP;
    self->svflags &= ~SVF_NOCLIENT;
    self->health = self->max_health;
    self->takedamage = DAMAGE_YES;
	self->use = func_explosive_use;
    gi.linkentity(self);
}

void func_explosive_explode (edict_t *self)
{
	vec3_t	origin;
	vec3_t	chunkorigin;
	vec3_t	size;
	int		count;
	int		mass;
	edict_t	*master;
	qboolean	done = false;

	//Knightmare- kill exploding train's pieces
	/*if (!strcmp(self->classname, "func_train") && IsRogueMap())
	{
		if(self->team)
		{
			edict_t *e;
			for (e=self->teamchain; e ; e = e->teamchain)
			{	//don't kill other synched trains
				if (strcmp(e->classname, "func_train") != 0)
					G_FreeEdict(e);
			}
		}
	}*/

	//Knightmare- kill turret base's breach
	if (!strcmp(self->classname, "turret_base"))
	{
		if(self->team)
		{
			edict_t *e;
			int i;

			e = g_edicts+1; // skip the worldspawn
			for (i = 1; i < globals.num_edicts; i++, e++)
			{
				if (!e->classname)
					continue;
				//only destroy the breach
				if (!strcmp(e->classname, "turret_breach") && !strcmp(e->team, self->team))
				{
					e->health = -1;
					e->gib_health = 0;
					turret_breach_die (e, self, self, 1000, self->s.origin);
					break;
				}
			}
		}
	}

	//Knightmare- remove train's children if it is destroyed
	train_kill_children(self); 

	//Knightmare- don't manipulate origin of rotating train- it already has it in the right place
	if ( strcmp(self->classname, "func_train") || !self->spawnflags & TRAIN_ROTATE )
	{ 	// bmodel origins are (0 0 0), we need to adjust that here
		VectorScale (self->size, 0.5, size);
		VectorAdd (self->absmin, size, origin);
		VectorCopy (origin, self->s.origin);
	}
	else
		VectorCopy (self->s.origin, origin);

	self->takedamage = DAMAGE_NO;

	if (self->dmg)
		T_RadiusDamage (self, self->activator, self->dmg, NULL, self->dmg+40, MOD_EXPLOSIVE);

	if (self->item) //Knightmare- added item dropping
	{
		Drop_Item (self, self->item);
		self->item = NULL;
	}

	VectorSubtract (self->s.origin, self->enemy->s.origin, self->velocity);
	VectorNormalize (self->velocity);
	VectorScale (self->velocity, 150, self->velocity);

	// start chunks towards the center
	VectorScale (size, 0.5, size);

	mass = self->mass;
	if (!mass)
		mass = 75;

	if( (self->gib_type > 0) && (self->gib_type < 10))
	{
		int	r;

		count = mass/25;
		if(count > 128)
			count = 128;
		while(count--)
		{
			r = (rand() % 5) + 1;
			chunkorigin[0] = origin[0] + crandom() * size[0];
			chunkorigin[1] = origin[1] + crandom() * size[1];
			chunkorigin[2] = origin[2] + crandom() * size[2];
			switch(self->gib_type) {
			case GIB_METAL:
				ThrowDebris (self, va("models/objects/metal_gibs/gib%i.md2",r),   2, chunkorigin, self->skinnum, 0); break;
			case GIB_GLASS:
				ThrowDebris (self, va("models/objects/glass_gibs/gib%i.md2",r),   2, chunkorigin, self->skinnum, EF_SPHERETRANS); break;
			case GIB_BARREL:
				ThrowDebris (self, va("models/objects/barrel_gibs/gib%i.md2",r),  2, chunkorigin, self->skinnum, 0); break;
			case GIB_CRATE:
				ThrowDebris (self, va("models/objects/crate_gibs/gib%i.md2",r),   2, chunkorigin, self->skinnum, 0); break;
			case GIB_ROCK:
				ThrowDebris (self, va("models/objects/rock_gibs/gib%i.md2",r),    2, chunkorigin, self->skinnum, 0); break;
			case GIB_CRYSTAL:
				ThrowDebris (self, va("models/objects/crystal_gibs/gib%i.md2",r), 2, chunkorigin, self->skinnum, 0); break;
			case GIB_MECH:
				ThrowDebris (self, va("models/objects/mech_gibs/gib%i.md2",r),    2, chunkorigin, self->skinnum, 0); break;
			case GIB_WOOD:
				ThrowDebris (self, va("models/objects/wood_gibs/gib%i.md2",r),    2, chunkorigin, self->skinnum, 0); break;
			case GIB_TECH:
				ThrowDebris (self, va("models/objects/tech_gibs/gib%i.md2",r),    2, chunkorigin, self->skinnum, 0); break;
			}
		}
	}
	else
	{
		// big chunks
		if (mass >= 100)
		{
			count = mass / 100;
			if (count > 128) //Knightmare- changed to 32
				count = 128;
			while(count--)
			{
				chunkorigin[0] = origin[0] + crandom() * size[0];
				chunkorigin[1] = origin[1] + crandom() * size[1];
				chunkorigin[2] = origin[2] + crandom() * size[2];
				ThrowDebris (self, "models/objects/debris1/tris.md2", 1, chunkorigin, 0, 0);
			}
		}

		// small chunks
		count = mass / 25;
		if (count > 256) //Knightmare- changed to 64
			count = 256;
		while(count--)
		{
			chunkorigin[0] = origin[0] + crandom() * size[0];
			chunkorigin[1] = origin[1] + crandom() * size[1];
			chunkorigin[2] = origin[2] + crandom() * size[2];
			ThrowDebris (self, "models/objects/debris2/tris.md2", 2, chunkorigin, 0, 0);
		}
	}
	// PMM - if we're part of a train, clean ourselves out of it
	if (self->flags & FL_TEAMSLAVE && !self->movewith_set)
	{
//		if ((g_showlogic) && (g_showlogic->value))
//			gi.dprintf ("Removing func_explosive from train!\n");

		if (self->teammaster)
		{
			master = self->teammaster;
			if(master && master->inuse)		// because mappers (other than jim (usually)) are stupid....
			{
				while (!done)
				{
					if (master->teamchain == self)
					{
						master->teamchain = self->teamchain;
						done = true;
					}
					master = master->teamchain;
					if (!master)
					{
//						if ((g_showlogic) && (g_showlogic->value))
//							gi.dprintf ("Couldn't find myself in master's chain, ignoring!\n");
					}
				}
			}
		}
		else
		{
//			if ((g_showlogic) && (g_showlogic->value))
//				gi.dprintf ("No master to free myself from, ignoring!\n");
		}
	}

	G_UseTargets (self, self->activator);

	//Knightmare- support for deathtarget
	if(self->deathtarget)
	{
		self->target = self->deathtarget;
		G_UseTargets (self, self->activator);
	}

	//Knightmare- support for destroytarget
	if(self->destroytarget)
	{
		self->target = self->destroytarget;
		G_UseTargets (self, self->activator);
	}

// Respawnable func_explosive. If desired, replace these lines....

	if (self->dmg)
	{
		if (mass >= 400)
			BecomeExplosion3 (self);
		else
			BecomeExplosion1 (self);
	}
	else
		G_FreeEdict (self);

	/*  with these:
    if (self->dmg)
    {
        gi.WriteByte (svc_temp_entity);	
		if (mass >= 400)
			gi.WriteByte (TE_EXPLOSION1_BIG);
		else
			gi.WriteByte (TE_EXPLOSION1);
        gi.WritePosition (self->s.origin);
        gi.multicast (self->s.origin, MULTICAST_PVS);
    }
	VectorClear(self->s.origin);
	VectorClear(self->velocity);
    self->solid = SOLID_NOT;
    self->svflags |= SVF_NOCLIENT;
    self->think = func_explosive_respawn;
    self->nextthink = level.time + 10; // substitute whatever value you want here
	self->use = NULL;
	gi.linkentity(self); */
}

void func_explosive_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->enemy = inflictor;
	self->activator = attacker;
	if(self->delay > 0)
	{	//Knightmare- clean up child movement stuff here
		// Is this really needed anymore?
	/*	VectorClear(self->avelocity);
		self->movewith = "";
		self->movewith_set = 0;
		self->movewith_ent = NULL;
		self->postthink = NULL;*/
		self->think = func_explosive_explode;
		self->nextthink = level.time + self->delay;
	}
	else
		func_explosive_explode(self);
}

void func_explosive_use(edict_t *self, edict_t *other, edict_t *activator)
{
	func_explosive_die (self, self, other, self->health, vec3_origin);
}

//PGM
void func_explosive_activate(edict_t *self, edict_t *other, edict_t *activator)
{
	int approved;

	approved = 0;
	// PMM - looked like target and targetname were flipped here
	if (other != NULL && other->target)
	{
		if(!strcmp(other->target, self->targetname))
			approved = 1;
	}
	if (!approved && activator!=NULL && activator->target)
	{
		if(!strcmp(activator->target, self->targetname))
			approved = 1;
	}

	if (!approved)
	{
//		gi.dprintf("func_explosive_activate: incorrect activator\n");
		return;
	}

	// PMM - according to mappers, they don't need separate cases for blowupable and triggerable
//	if (self->target)
//	{
		self->use = func_explosive_use;
//	}
//	else
//	{
		if (!self->health)
			self->health = 100;
		self->die = func_explosive_die;
		self->takedamage = DAMAGE_YES;
//	}
}
//PGM

void func_explosive_spawn (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
    self->svflags |= SVF_DAMAGEABLE;
	self->use = NULL;
	KillBox (self);
	gi.linkentity (self);
}

// Lazarus func_explosive can be damaged by impact
void func_explosive_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		damage;
	float	delta;
	float	mass;
	vec3_t	dir, impact_v;

	if (!self->health) return;
	if (other->mass <= 200) return;
	if (VectorLength(other->velocity)==0) return;
	// Check for impact damage
	if (self->health > 0)
	{
		VectorSubtract(other->velocity,self->velocity,impact_v);
		delta = VectorLength(impact_v);
		delta = delta*delta*0.0001;
		mass = self->mass;
		if (!mass) mass = 200;
		delta *= (float)(other->mass)/mass;
		if (delta > 30)
		{
			damage = (delta-30)/2;
			if (damage > 0)
			{
				VectorSubtract(self->s.origin,other->s.origin,dir);
				VectorNormalize(dir);
				T_Damage (self, other, other, dir, self->s.origin, vec3_origin, damage, 0, 0, MOD_FALLING);
			}
		}
	}
}

void SP_func_explosive (edict_t *self)
{
	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_PUSH;

	PrecacheDebris(self->gib_type);

	gi.modelindex ("models/objects/debris1/tris.md2");
	gi.modelindex ("models/objects/debris2/tris.md2");

	gi.setmodel (self, self->model);

	if (self->spawnflags & 1)
	{
		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->use = func_explosive_spawn;
	}
//PGM
	else if(self->spawnflags & 8)
	{
		self->solid = SOLID_BSP;
		if (self->targetname)
			self->use = func_explosive_activate;
	}
//PGM
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

//PGM
	if ((self->use != func_explosive_use) && (self->use != func_explosive_activate))
//PGM
	{
		if (!self->health)
			self->health = 100;
		self->die = func_explosive_die;
		self->takedamage = DAMAGE_YES;
	}

	if (st.item) //Knightmare- item support
	{
		self->item = FindItemByClassname (st.item);
		if (!self->item)
			gi.dprintf("%s at %s has bad item: %s\n", self->classname, vtos(self->s.origin), st.item);
	}

	// Lazarus: added touch function for impact damage
	self->touch = func_explosive_touch;

	// Lazarus: for respawning func_explosives
	self->max_health = self->health;

	gi.linkentity (self);
}

void PrecacheDebris(int type)
{
	int	i;

	switch(type)
	{
	case 0:
		gi.modelindex ("models/objects/debris1/tris.md2");
		gi.modelindex ("models/objects/debris2/tris.md2");
		gi.modelindex ("models/objects/debris3/tris.md2");
		break;
	case GIB_METAL:
		for(i=1;i<=5;i++)
			gi.modelindex(va("models/objects/metal_gibs/gib%i.md2",i));
		break;
	case GIB_GLASS:
		for(i=1;i<=5;i++)
			gi.modelindex(va("models/objects/glass_gibs/gib%i.md2",i));
		break;
	case GIB_BARREL:
		for(i=1;i<=5;i++)
			gi.modelindex(va("models/objects/barrel_gibs/gib%i.md2",i));
		break;
	case GIB_CRATE:
		for(i=1;i<=5;i++)
			gi.modelindex(va("models/objects/crate_gibs/gib%i.md2",i));
		break;
	case GIB_ROCK:
		for(i=1;i<=5;i++)
			gi.modelindex(va("models/objects/rock_gibs/gib%i.md2",i));
		break;
	case GIB_CRYSTAL:
		for(i=1;i<=5;i++)
			gi.modelindex(va("models/objects/crystal_gibs/gib%i.md2",i));
		break;
	case GIB_MECH:
		for(i=1;i<=5;i++)
			gi.modelindex(va("models/objects/mech_gibs/gib%i.md2",i));
		break;
	case GIB_WOOD:
		for(i=1;i<=5;i++)
			gi.modelindex(va("models/objects/wood_gibs/gib%i.md2",i));
		break;
	case GIB_TECH:
		for(i=1;i<=5;i++)
			gi.modelindex(va("models/objects/tech_gibs/gib%i.md2",i));
		break;
	}
}

// Knightmare- func_breakaway is a brush model that breaks away from its
//		surroundings (masonry, boulders, etc.), and changes from
//		SOLID_BSP MOVETYPE_PUSH to SOLID_BBOX MOVETYPE_DEBRIS.

#define BREAKAWAY_TRIGGER_SPAWN	1
#define BREAKAWAY_ANIM			2
#define BREAKAWAY_ANIM_FAST		4
#define BREAKAWAY_INACTIVE		8
#define BREAKAWAY_FADE			16
#define BREAKAWAY_ALWAYS_SHOOT	32
#define BREAKAWAY_TARGET_ANGLE	64

/*QUAKED func_breakaway (0 .5 .8) ? Trigger_Spawn Animated Animated_Fast Inactive Fade AlwaysShoot TargAngle
Any brush model that you want to break away from its surroundings (masonry, boulders, etc.).
Needs an origin brush unless its center is at the map origin.

If targeted it will not be shootable, unless the inactive flag is set.

Inactive -	Specifies that the entity is not shootable until triggered.
Fade -		Specifies that the entity will fade away after coming to a rest.
				Time to fade is specified by "fadeout".  Default: 30 seconds.
AlwaysShoot-	Damagable even if targeted.
TargAngle-		Will rotate toward "move_angles" when dislodged.  Useful for objects that fall over flat.  Use "duration" for how many seconds the rotation takes.  Default: 5, will also always be > "wait"

"health"	Defaults to 100.
"health2"	How much health to destroy the brush after it has come to a stop.
"mass"		Defaults to 500.  This determines how much damage the falling boulder does.
"speed"		Defaults to 100.  This sets how fast the brush will move (only if triggered).
"angles"	Sets the initial angle that the brush will move at.  Use pitch, yaw, and roll.
"delay"		How long to wait before breaking away, default 0.
"wait"		How long before it becomes SOLID_BBOX after being dislodged, default 0.3.
"fadeout"	If the Fade flag is set, this is how long until it fades away.
"noise"		Sound to play when it hits.
"count"		Number of times to play impact sound

"target"	Will fire all entities with this targetname when it breaks free.
"deathtarget"	Will fire all entities with this targetname when it breaks free.
"destroytarget"	Will fire all entities with this targetname when it is destroyed after coming to a rest.

"gib_type"	Set to 1 for metal, 2 for glass, 3 for barrel, 4 for crate, 5 for rock, 6 for crystal
			7 for mechanical, 8 for wood, 9 for tech gibs.

"bleft"		Min b-box coords XYZ. Default = -16 -16 -16
"tright"	Max b-box coords XYZ. Default = 16 16 16

To have other entities move with the brush, set the door pieces' movewith values to the same as the brush's targetname and they will move in
unison, and will also fade away in unison if the Fade spawnflag is set.  Be sure to use the AlwaysShoot spawnflag if you want it to be shootable.
NOTE: All the pieces must be created after the brush entity, otherwise they will move ahead of it.
*/

void func_breakaway_hit (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}
	if (plane && plane->normal)
	{
		//roll in direction of movement
		if (!(self->spawnflags & BREAKAWAY_TARGET_ANGLE))
		{
			vectoangles (self->velocity, self->avelocity);
			VectorNormalize (self->avelocity);
			VectorScale (self->avelocity, ((400+random()*20* self->speed) / (self->mass * 0.05)), self->avelocity);
		}
		if (self->noise_index && !(self->waterlevel))
		{
			gi.sound (self, CHAN_AUTO, self->noise_index, 1.0, ATTN_NORM, 0);
			self->count--;
			if (self->count == 0)
				self->noise_index = 0;
		}
	}
}

void func_breakaway_think (edict_t *self)
{
	vec3_t	move;

	if (self->spawnflags & BREAKAWAY_TARGET_ANGLE)
	{
		if (level.time >= self->timestamp) //we should be done rotating by now
		{
			if (self->style & 1) //if done rotating
			{
				VectorClear (self->avelocity);
				VectorCopy (self->move_angles, self->s.angles);
			}
			else //set up final rotation
			{
				VectorSubtract (self->move_angles, self->s.angles, move);
				VectorScale (move, 1.0/FRAMETIME, self->avelocity);
				self->style |= 1;
			}
		}
		else
			VectorCopy (self->relative_avelocity, self->avelocity);
	}
	// Don't become immobile until we're on solid ground
	if (!VectorLength(self->velocity) && self->groundentity && self->groundentity == world)
	{
		VectorClear (self->velocity);
		VectorClear (self->avelocity);
		if (self->spawnflags & BREAKAWAY_FADE)
		{
			self->think = gib_fade;
			self->nextthink = level.time + (int)self->fadeout;
			return;
		}
		self->movetype = MOVETYPE_PUSH;
		VectorClear (self->mins);
		VectorClear (self->maxs);
		gi.setmodel (self, self->model);
		self->solid = SOLID_BSP;
		self->nextthink = 0;
		gi.linkentity (self);
	}
	else
		self->nextthink = level.time + FRAMETIME;
}

void func_breakaway_makesolid (edict_t *self)
{
	self->solid = SOLID_BBOX;
	VectorCopy (self->bleft, self->mins);
	VectorCopy (self->tright, self->maxs);

	if (self->health2)
	{
		self->health = self->health2;
		self->die = func_explosive_die;
		self->takedamage = DAMAGE_YES;
		self->delay = 0;
	}

	self->think = func_breakaway_think;
	self->nextthink = level.time + FRAMETIME;
}

void func_breakaway_fall (edict_t *self)
{
	vec_t var = self->speed/5;
	//int i;

	self->use = NULL;

	self->velocity[0] = self->speed * self->movedir[0] + var * crandom();
	self->velocity[1] = self->speed * self->movedir[1] + var * crandom();
	self->velocity[2] = self->speed * self->movedir[2] + var * crandom();

	if (!(self->spawnflags & BREAKAWAY_TARGET_ANGLE))
	{
		vectoangles (self->movedir, self->avelocity);
		VectorNormalize (self->avelocity);
		VectorScale (self->avelocity, ((400+random()*20*self->speed) / (self->mass * 0.05)), self->avelocity);
		/*for (i = 0; i < 3; i++)
		{
			self->avelocity[i] = (random()*600) / (self->mass * 0.05);
			if (random() > 0.5)
				self->avelocity[i] *= -1;
		}*/
	}
	else //get difference in current and target angles, divide by time to get avelocity
	{
		VectorSubtract (self->move_angles, self->s.angles, self->avelocity);
		VectorScale (self->avelocity, (1/self->duration), self->avelocity);
		VectorCopy (self->avelocity, self->relative_avelocity);
		self->timestamp = level.time + self->duration; //save when we started rotating
		self->style &= ~1; //flag for whether to stop movement
	}

	self->attenuation = 0.5;
	self->movetype = MOVETYPE_DEBRIS;
	self->solid = SOLID_NOT; //make not solid to not clip with world
	self->touch = func_breakaway_hit;

	self->think = func_breakaway_makesolid; //make solid again after wait
	self->nextthink = level.time + self->wait;

	G_UseTargets (self, self->activator);

	if(self->deathtarget)
	{
		self->target = self->deathtarget;
		G_UseTargets (self, self->activator);
	}
	self->target = NULL;

	gi.linkentity (self);
}

void func_breakaway_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->enemy = inflictor;
	self->activator = attacker;
	self->speed = max(damage, self->speed);
	self->takedamage = DAMAGE_NO;
	self->die = NULL;
	if (self->delay > 0)
	{	//Knightmare- clean up child movement stuff here
		VectorClear(self->avelocity);
		self->movewith = "";
		self->movewith_set = 0;
		self->movewith_ent = NULL;
		self->postthink = NULL;
		self->think = func_breakaway_fall;
		self->nextthink = level.time + self->delay;
	}
	else
		func_breakaway_fall (self);
}

void func_breakaway_use(edict_t *self, edict_t *other, edict_t *activator)
{
	func_breakaway_die (self, self, other, self->speed, vec3_origin);
}

void func_breakaway_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		damage;
	float	delta;
	float	mass;
	vec3_t	dir, impact_v;

	if (!self->health) return;
	if (other->mass <= 200) return;
	if (VectorLength(other->velocity)==0) return;
	// Check for impact damage
	if (self->health > 0)
	{
		VectorSubtract(other->velocity,self->velocity,impact_v);
		delta = VectorLength(impact_v);
		delta = delta*delta*0.0001;
		mass = self->mass;
		if (!mass) mass = 200;
		delta *= (float)(other->mass)/mass;
		if (delta > 30)
		{
			damage = (delta-30)/2;
			if (damage > 0)
			{
				VectorSubtract(self->s.origin,other->s.origin,dir);
				VectorNormalize(dir);
				T_Damage (self, other, other, dir, self->s.origin, vec3_origin, damage, 0, 0, MOD_FALLING);
			}
		}
	}
}

void func_breakaway_spawn (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
      self->svflags |= SVF_DAMAGEABLE;
	self->use = func_breakaway_use;
	KillBox (self);
	gi.linkentity (self);
}

void func_breakaway_activate(edict_t *self, edict_t *other, edict_t *activator)
{
	if (!self->health)
		self->health = 100;
	self->die = func_breakaway_die;
	self->takedamage = DAMAGE_YES;
	self->use = func_breakaway_use;
}

void SP_func_breakaway (edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self->model);
	G_SetMovedir (self->s.angles, self->movedir);

	// precache debris if destroyable
	if (self->health2)
		PrecacheDebris(self->gib_type);

	if ( (!VectorLength(self->bleft)) && (!VectorLength(self->tright)) )
	{
		VectorSet(self->bleft, -16, -16, -16);
		VectorSet(self->tright, 16, 16, 16);
	}

	if (!self->mass)
		self->mass = 500;
	if (!self->speed)
		self->speed = 100;
	if ((self->spawnflags & BREAKAWAY_FADE) && !self->fadeout)
		self->fadeout = 30;
	if (!self->wait)
		self->wait = 0.3;

	if (self->spawnflags & BREAKAWAY_TARGET_ANGLE)
	{
		if (!self->duration)
			self->duration = 5.0;
		//must stop rotating AFTER becoming solid
		if (self->duration <= self->wait)
			self->duration = self->wait + FRAMETIME;
	}
	if (st.noise)
		self->noise_index = gi.soundindex  (st.noise);

	if (self->spawnflags & BREAKAWAY_TRIGGER_SPAWN)
	{
		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->use = func_breakaway_spawn;
	}
	else if (self->spawnflags & BREAKAWAY_INACTIVE)
	{
		self->solid = SOLID_BSP;
		if(self->targetname)
			self->use = func_breakaway_activate;
	}
	else
	{
		self->solid = SOLID_BSP;
		if (self->targetname)
			self->use = func_breakaway_use;
	}

	if (self->spawnflags & BREAKAWAY_ANIM)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & BREAKAWAY_ANIM_FAST)
		self->s.effects |= EF_ANIM_ALLFAST;

	if ((self->spawnflags & BREAKAWAY_ALWAYS_SHOOT) || ((self->use != func_breakaway_use) && (self->use != func_breakaway_activate)))
	{
		if (!self->health)
			self->health = 100;
		self->die = func_breakaway_die;
		self->takedamage = DAMAGE_YES;
	}

	self->postthink = train_move_children; //supports movewith

	//touch function for impact damage
	self->touch = func_breakaway_touch;

	gi.linkentity (self);
}

/*QUAKED misc_explobox (0 .5 .8) (-16 -16 0) (16 16 40)
Large exploding box.  You can override its mass (100),
health (80), and dmg (150).
gib_type- Set to 3 for barrel-specific gibs.
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
	vec3_t	size;
	int i;
	
	/*if(self->gib_type == GIB_BARREL)
	{	//Knightmare- cap mass at 399 to avoid large explosion
		if (self->mass >= 400)
			self->mass = 399;

		func_explosive_explode (self);
		return;
	}*/
	
	VectorCopy (self->s.origin, save);
	VectorMA (self->absmin, 0.5, self->size, self->s.origin);

	T_RadiusDamage (self, self->activator, self->dmg, NULL, self->dmg+40, MOD_BARREL);

	// Lazarus: Nice catch by Argh! For years now, explobox chunks have
	//          gone through the floor. This is very noticeable if the floor
	//          is transparent. Like func_explosive, shrink size box so that
	//          debris has an initial standoff from the floor.
	VectorScale(self->size,0.5,size);

	//Knightmare- new gib code
	if (self->gib_type && self->gib_type == GIB_BARREL)
	{
		// top
		spd = 1.5 * (float)self->dmg / 200.0;
		VectorCopy (self->s.origin, org);
		org[2] = self->absmax[2];
		ThrowDebris (self, "models/objects/barrel_gibs/gib2.md2", spd, org, 0, 0);
		
		// side pieces
		for (i = 0; i < 8; i++)
		{
			org[0] = self->s.origin[0] + crandom() * size[0];
			org[1] = self->s.origin[1] + crandom() * size[1];
			org[2] = self->s.origin[2] + crandom() * size[2];
			ThrowDebris (self, "models/objects/barrel_gibs/gib4.md2",  spd, org, 0, 0);
		}

		// bottom corners
		spd = 1.75 * (float)self->dmg / 200.0;
		VectorCopy (self->absmin, org);
		org[1] += self->size[1];
		ThrowDebris (self, "models/objects/barrel_gibs/gib1.md2",  spd, org, 0, 0);
		org[0] += self->size[0] * 2;
		ThrowDebris (self, "models/objects/barrel_gibs/gib1.md2",  spd, org, 0, 0);

		// top corners
		VectorCopy (self->absmax, org);
		org[1] += self->size[1];
		ThrowDebris (self, "models/objects/barrel_gibs/gib3.md2",  spd, org, 0, 0);
		org[0] += self->size[0] * 2;
		ThrowDebris (self, "models/objects/barrel_gibs/gib3.md2",  spd, org, 0, 0);

		// a bunch of little chunks
		spd = 2 * self->dmg / 200;
		for (i = 0; i < 8; i++)
		{
			org[0] = self->s.origin[0] + crandom() * size[0];
			org[1] = self->s.origin[1] + crandom() * size[1];
			org[2] = self->s.origin[2] + crandom() * size[2];
			ThrowDebris (self, "models/objects/barrel_gibs/gib5.md2",  spd, org, 0, 0);
		}
	}
	else
	{
		// a few big chunks
		spd = 1.5 * (float)self->dmg / 200.0;
		for (i = 0; i < 2; i++)
		{
			org[0] = self->s.origin[0] + crandom() * size[0];
			org[1] = self->s.origin[1] + crandom() * size[1];
			org[2] = self->s.origin[2] + crandom() * size[2];
			ThrowDebris (self, "models/objects/debris1/tris.md2", spd, org, 0, 0);
		}

		// bottom corners
		spd = 1.75 * (float)self->dmg / 200.0;
		VectorCopy (self->absmin, org);
		ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org, 0, 0);
		VectorCopy (self->absmin, org);
		org[0] += self->size[0];
		ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org, 0, 0);
		VectorCopy (self->absmin, org);
		org[1] += self->size[1];
		ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org, 0, 0);
		VectorCopy (self->absmin, org);
		org[0] += self->size[0];
		org[1] += self->size[1];
		ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org, 0, 0);

		// a bunch of little chunks
		spd = 2 * self->dmg / 200;
		for (i = 0; i < 8; i++)
		{
			org[0] = self->s.origin[0] + crandom() * size[0];
			org[1] = self->s.origin[1] + crandom() * size[1];
			org[2] = self->s.origin[2] + crandom() * size[2];
			ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org, 0, 0);
		}
	}

	// Lazarus: use targets
	G_UseTargets (self, self->activator);

	// Lazarus: added case for no damage
	if(self->dmg)
	{
		VectorCopy (save, self->s.origin);
		if (self->groundentity)
		{
			self->s.origin[2] = self->absmin[2] + 2;
			BecomeExplosion2 (self);
		}
		else
			BecomeExplosion1 (self);
	}
	else
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("tank/thud.wav"), 1, ATTN_NORM, 0);
		G_FreeEdict (self);
	}
}

void barrel_delay (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + 2 * FRAMETIME;
	self->think = barrel_explode;
	self->activator = attacker;
}

//=========
//PGM  - change so barrels will think and hence, blow up
void barrel_think (edict_t *self)
{
	// the think needs to be first since later stuff may override.
	self->think = barrel_think;
	self->nextthink = level.time + FRAMETIME;

	M_CatagorizePosition (self);
	self->flags |= FL_IMMUNE_SLIME;
	self->air_finished = level.time + 100;
	M_WorldEffects (self);
}

void barrel_start (edict_t *self)
{
	M_droptofloor(self);
	self->think = barrel_think;
	self->nextthink = level.time + FRAMETIME;
}
//PGM
//=========

void SP_misc_explobox (edict_t *self)
{
	int i;

	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (self);
		return;
	}

	//Knightmare- barrel-specific gibs, since we have the higher limit
#ifdef KMQUAKE2_ENGINE_MOD
	self->gib_type = GIB_BARREL;
	for (i=1; i<=5; i++)
		gi.modelindex(va("models/objects/barrel_gibs/gib%i.md2",i));
#else
	if (self->gib_type && self->gib_type == GIB_BARREL)
	{
		for (i=1; i<=5; i++)
			gi.modelindex(va("models/objects/barrel_gibs/gib%i.md2",i));
	}
	else
	{
		gi.modelindex ("models/objects/debris1/tris.md2");
		gi.modelindex ("models/objects/debris2/tris.md2");
		gi.modelindex ("models/objects/debris3/tris.md2");
	}
#endif

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_STEP;

	self->model = "models/objects/barrels/tris.md2";
	self->s.modelindex = gi.modelindex (self->model);
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 40);

	// Knightmare added- make IR visible
	self->s.renderfx |= RF_IR_VISIBLE;

	if (!self->mass)
		self->mass = 400;

	if (!self->health)
		self->health = 10;
	if (!self->dmg)
		self->dmg = 150;

	self->die = barrel_delay;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.aiflags = AI_NOSTEP;

	self->touch = barrel_touch;

//PGM - change so barrels will think and hence, blow up
	self->think = barrel_start;
	self->nextthink = level.time + 2 * FRAMETIME;
//PGM

	self->common_name = "Exploding Barrel";
	gi.linkentity (self);
}


//
// miscellaneous specialty items
//

/*QUAKED misc_blackhole (1 .5 0) (-8 -8 -8) (8 8 8)
*/

//David Hyde's prethink function
void misc_blackhole_transparent (edict_t *ent)
{
	// Lazarus: This avoids the problem mentioned below.
	ent->s.renderfx = RF_TRANSLUCENT|RF_NOSHADOW;
	ent->prethink = NULL;
	gi.linkentity(ent);
}

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
//	ent->s.renderfx = RF_TRANSLUCENT; David Hyde's fix for this
	ent->use = misc_blackhole_use;
	ent->think = misc_blackhole_think;
	ent->prethink = misc_blackhole_transparent;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (ent);
}

/*QUAKED misc_eastertank (1 .5 0) (-32 -32 -16) (32 32 32) NOT_SOLID COMMANDER
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
	if (ent->spawnflags & 1)	 //Knightmare- not solid flag
		ent->solid = SOLID_NOT;
	else
		ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -32, -32, -16);
	VectorSet (ent->maxs, 32, 32, 32);
	ent->s.modelindex = gi.modelindex ("models/monsters/tank/tris.md2");
	ent->s.frame = 254;
	if (ent->spawnflags & 2)	 //Knightmare- commander skin
		ent->s.skinnum = 2;
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
	ent->s.renderfx |= RF_NOSHADOW;
	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_deadsoldier (1 .5 0) (-16 -16 0) (16 16 16) ON_BACK ON_STOMACH BACK_DECAP FETAL_POS SIT_DECAP IMPALED
This is the dead player model. Comes in 6 exciting different poses!
*/
void misc_deadsoldier_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	if (self->health > -100)
//	if (self->health > -30)
		return;

	gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
	for (n= 0; n < 4; n++)
		ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
	ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
}

void misc_deadsoldier_flieson(edict_t *self)
{
	if (!(self->spawnflags & 64))
		return;
	if (self->waterlevel)
		return;
	self->s.effects |= EF_FLIES;
	self->s.sound = gi.soundindex ("infantry/inflies1.wav");
}

int PatchDeadSoldier (void);
void SP_misc_deadsoldier (edict_t *ent)
{
	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (ent);
		return;
	}

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex ("models/deadbods/dude/tris.md2");

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

		// Lazarus
	if (ent->spawnflags & 64)
	{
		// postpone turning on flies till we figure out whether dude is submerged
		ent->think = misc_deadsoldier_flieson;
		ent->nextthink = level.time + FRAMETIME;
	}

	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 16);
	ent->deadflag = DEAD_DEAD;
	ent->takedamage = DAMAGE_YES;
	ent->svflags |= SVF_MONSTER|SVF_DEADMONSTER;
	// Knightmare added- make IR visible
	ent->s.renderfx |= RF_IR_VISIBLE; 
	ent->die = misc_deadsoldier_die;
	ent->monsterinfo.aiflags |= AI_GOOD_GUY;

	if (ent->style)
	{
		PatchDeadSoldier();
		ent->s.skinnum = ent->style;
	}
	ent->common_name = "Dead Marine";

	gi.linkentity (ent);
}

#define	TRAIN_ROTATE		8
#define	TRAIN_ROT_CONST		16
#define TRAIN_SMOOTH		128
#define TRAIN_SPLINE        8192

/*QUAKED misc_viper (1 .5 0) (-16 -16 0) (16 16 32) START_ON BIG_VIPER x ROTATE ROT_CONST x x SMOOTH
This is the Viper for the flyby bombing.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"			How fast the Viper should fly
"pitch_speed"	degrees per second
"yaw_speed"		degrees per second
"roll_speed"	degrees per second
"accel"		Acceleration
"decel"		Deceleration
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

	// Zaero
	if ( IsZaeroMap() && (ent->spawnflags & 1) )
	{
		ent->s.effects |= EF_ROCKET;
		ent->spawnflags &= ~1; // turn this off so that it doesn't mess up the trains
	}
	// end Zaero

	ent->class_id = ENTITY_MISC_VIPER;

	if (!ent->speed)
		ent->speed = 300;

	ent->movetype = MOVETYPE_PUSH;

	// Zaero
	if ( IsZaeroMap() && (ent->spawnflags & 2) ) {
		ent->solid = SOLID_BBOX;
	}
	else {
		ent->solid = SOLID_NOT;
	}
	// end Zaero

	// Zaero
	if ( IsZaeroMap() )
	{
		if (ent->model) {
			ent->s.modelindex = gi.modelindex (ent->model);
		}
		else {
			ent->s.modelindex = gi.modelindex ("models/ships/viper/tris.md2");
		}
		if (ent->model2) {
			ent->s.modelindex2 = gi.modelindex (ent->model2);
		}
		if (ent->model3) {
			ent->s.modelindex3 = gi.modelindex (ent->model3);
		}
		if (ent->model4) {
			ent->s.modelindex4 = gi.modelindex (ent->model4);
		}
		ent->spawnflags |= TRAIN_SMOOTH|TRAIN_ORIGIN;	// set smooth movement and origin flag for zdef4
	}
	else
	// end Zaero
	// Mappack
	if (ent->spawnflags & 2)
	{
		ent->solid = SOLID_BBOX;
		ent->s.modelindex = gi.modelindex ("models/ships/bigviper/tris.md2");
		VectorSet (ent->mins, -176, -120, -24);
		VectorSet (ent->maxs, 176, 120, 72);
	}	// Mappack
	else
	{
		ent->s.modelindex = gi.modelindex ("models/ships/viper/tris.md2");
		VectorSet (ent->mins, -16, -16, 0);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	ent->s.renderfx |= RF_IR_VISIBLE;

	if (st.noise) // Knightmare- added movement sound
		ent->noise_index = gi.soundindex  (st.noise);

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_viper_use;
	if (!ent->spawnflags & 1)
		ent->svflags |= SVF_NOCLIENT;

	if (ent->spawnflags & TRAIN_SMOOTH)
		ent->smooth_movement = 1;
	else
		ent->smooth_movement = 0;

	// Knightmare- change both rotate flags to spline flag
	if ( (ent->spawnflags & TRAIN_ROTATE) && (ent->spawnflags & TRAIN_ROT_CONST) )
	{
		ent->spawnflags &= ~TRAIN_ROTATE;
		ent->spawnflags &= ~TRAIN_ROT_CONST;
		ent->spawnflags |= TRAIN_SPLINE;
	}

	// Mappack
	if(!ent->accel)
		ent->moveinfo.accel = ent->speed;
	else
		ent->moveinfo.accel = ent->accel;
	if(!ent->decel)
		ent->moveinfo.decel = ent->speed;
	else
		ent->moveinfo.decel = ent->decel;
	ent->moveinfo.speed = ent->speed;
	// Mappack

//	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	gi.linkentity (ent);
}

void SP_misc_viper_origin (edict_t *ent)
{
	ent->spawnflags |= TRAIN_ORIGIN;
	SP_misc_viper (ent);
}

/*QUAKED misc_crashviper (1 .5 0) (-176 -120 -24) (176 120 72) START_ON SMALL_VIPER x ROTATE ROT_CONST x x SMOOTH
This is a large viper about to crash
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"		How fast the Viper should fly
"pitch_speed"	degrees per second
"yaw_speed"		degrees per second
"roll_speed"	degrees per second
"accel"		Acceleration
"decel"		Deceleration
*/
void SP_misc_crashviper (edict_t *ent)
{
	if (!ent->target)
	{
		gi.dprintf ("misc_viper without a target at %s\n", vtos(ent->absmin));
		G_FreeEdict (ent);
		return;
	}

	ent->class_id = ENTITY_MISC_CRASHVIPER;

	if (!ent->speed)
		ent->speed = 300;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_NOT;
	//Mappack
	if(ent->spawnflags & 2)
	{
		ent->s.modelindex = gi.modelindex ("models/ships/viper/tris.md2");
		VectorSet (ent->mins, -16, -16, 0);
		VectorSet (ent->maxs, 16, 16, 32);

	} //Mappack
	else
	{
		ent->s.modelindex = gi.modelindex ("models/ships/bigviper/tris.md2");
		VectorSet (ent->mins, -176, -120, -24);
		VectorSet (ent->maxs, 176, 120, 72);
	}
	ent->s.renderfx |= RF_IR_VISIBLE;

	if (st.noise) // Kngihtmare added- movement sound
		ent->noise_index = gi.soundindex  (st.noise);

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_viper_use;
	if (!ent->spawnflags & 1)
		ent->svflags |= SVF_NOCLIENT;

	if (ent->spawnflags & TRAIN_SMOOTH)
		ent->smooth_movement = 1;
	else
		ent->smooth_movement = 0;

	//Knightmare- change both rotate flags to spline flag
	if ((ent->spawnflags & TRAIN_ROTATE) && (ent->spawnflags &TRAIN_ROT_CONST))
	{
		ent->spawnflags &= ~TRAIN_ROTATE;
		ent->spawnflags &= ~TRAIN_ROT_CONST;
		ent->spawnflags |= TRAIN_SPLINE;
	}

	//Mappack
	if(!ent->accel)
		ent->moveinfo.accel = ent->speed;
	else
		ent->moveinfo.accel = ent->accel;
	if(!ent->decel)
		ent->moveinfo.decel = ent->speed;
	else
		ent->moveinfo.decel = ent->decel;
	ent->moveinfo.speed = ent->speed;
	//Mappack

//	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	gi.linkentity (ent);
}

void SP_misc_crashviper_origin (edict_t *ent)
{
	ent->spawnflags |= TRAIN_ORIGIN;
	SP_misc_crashviper (ent);
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
	ent->s.renderfx |= RF_IR_VISIBLE;
	gi.linkentity (ent);
}


/*QUAKED misc_viper_bomb (1 0 0) (-8 -8 -8) (8 8 8)
"dmg"	how much boom should the bomb make?
*/
void misc_viper_bomb_use (edict_t *, edict_t *, edict_t *);
void misc_viper_bomb_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	G_UseTargets (self, self->activator);

	self->s.origin[2] = self->absmin[2] + 1;
	T_RadiusDamage (self, self, self->dmg, NULL, self->dmg+40, MOD_BOMB);

	self->count--;
	if(self->count == 0)
		self->spawnflags &= ~1;

	if (self->spawnflags & 1)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION2);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);

		// Lazarus reflections
		if (level.num_reflectors)
			ReflectExplosion (TE_EXPLOSION2, self->s.origin);
		
		self->svflags   |= SVF_NOCLIENT;
		self->solid      = SOLID_NOT;
		self->use        = misc_viper_bomb_use;
		self->s.effects  = 0;
		self->movetype   = MOVETYPE_NONE;
		self->nextthink  = 0;
		self->think      = NULL;
		VectorCopy(self->pos1,self->s.origin);
		VectorCopy(self->pos2,self->s.angles);
		VectorClear(self->velocity);
		gi.linkentity(self);
	}
	else
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

void viper_bomb_think (edict_t *self)
{
	if(readout->value)
	{
		gi.dprintf("bomb velocity=%g,%g,%g\n",self->velocity[0],self->velocity[1],self->velocity[2]);
		gi.dprintf("bomb position=%g,%g,%g\n",self->s.origin[0],self->s.origin[1],self->s.origin[2]);
	}
	self->nextthink = level.time + FRAMETIME;
}

void misc_viper_bomb_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*viper;

	if(self->solid != SOLID_NOT)  // Already in use
		return;

	self->solid = SOLID_BBOX;
	self->svflags &= ~SVF_NOCLIENT;
	self->s.effects |= EF_ROCKET;
	self->use = NULL;
	self->movetype = MOVETYPE_TOSS;
	self->prethink = misc_viper_bomb_prethink;
	self->touch = misc_viper_bomb_touch;
	self->activator = activator;

	VectorCopy(self->pos1,self->s.origin);
	VectorCopy(self->pos2,self->s.angles);

	if(self->pathtarget)
	{
		if(!Q_stricmp(self->pathtarget,self->targetname))
		{
			VectorScale(self->movedir,self->speed,self->velocity);
			VectorCopy(self->movedir,self->moveinfo.dir);
		}
		else
		{
			viper = G_Find(NULL, FOFS(targetname), self->pathtarget);
			if(!viper)
				return;
			VectorScale (viper->moveinfo.dir, viper->moveinfo.speed, self->velocity);
			VectorCopy (viper->moveinfo.dir, self->moveinfo.dir);
		}
	}
	else
	{
		viper = G_Find (NULL, FOFS(classname), "misc_viper");
		if(!viper)
			return;
		VectorScale (viper->moveinfo.dir, viper->moveinfo.speed, self->velocity);
		VectorCopy (viper->moveinfo.dir, self->moveinfo.dir);
	}
	self->timestamp = level.time;

	self->think = viper_bomb_think;
	self->nextthink = level.time + FRAMETIME;
}

void SP_misc_viper_bomb (edict_t *self)
{
	vec3_t	angles;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);

	self->s.modelindex = gi.modelindex ("models/objects/bomb/tris.md2");
	self->s.renderfx |=	RF_IR_VISIBLE;

	// Lazarus: Preserve angles in movedir and initial placement
	//          for case of targeting self
	VectorCopy(self->s.angles,angles);
	G_SetMovedir (angles,self->movedir);
	VectorCopy(self->s.origin,self->pos1);
	VectorCopy(self->s.angles,self->pos2);

	if (!self->dmg)
		self->dmg = 1000;

	self->use = misc_viper_bomb_use;
	self->svflags |= SVF_NOCLIENT;
	self->common_name = "Bomb";
	gi.linkentity (self);
}

// RAFAEL
/*QUAKED misc_viper_missile (1 0 0) (-8 -8 -8) (8 8 8)
"dmg"	how much boom should the bomb make? the default value is 250
*/
void misc_viper_missile_use (edict_t *self, edict_t *other, edict_t *activator)
{
	
	vec3_t	forward, right, up;
	vec3_t	start, dir;
	vec3_t	vec;

	AngleVectors (self->s.angles, forward, right, up);
	
	self->enemy = G_Find (NULL, FOFS(targetname), self->target);
	if (!self->enemy)
	{	//Knightmare- set movedir here, allowing angles to be updated by movewith code
		G_SetMovedir2 (self->s.angles, dir);
	}
	else
	{
		VectorCopy (self->enemy->s.origin, vec);
		vec[2] += 16;
		VectorCopy (self->s.origin, start);
		VectorSubtract (vec, start, dir);
		VectorNormalize (dir);
	}
	
	monster_fire_rocket (self, start, dir, self->dmg, 500, MZ2_CHICK_ROCKET_1, NULL);
//	monster_fire_missile (self, start, dir, self->dmg, 500, MZ2_CHICK_ROCKET_1, NULL);

	self->nextthink = level.time + 0.1;
	self->think = G_FreeEdict;
}

void SP_misc_viper_missile (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);

	//self->s.modelindex = gi.modelindex ("models/objects/bomb/tris.md2");

	if (!self->dmg)
		self->dmg = 250;

	self->use = misc_viper_missile_use;
	self->svflags |= SVF_NOCLIENT;
	self->common_name = "Missile Shooter";
	gi.linkentity (self);
}

/*QUAKED misc_strogg_ship (1 .5 0) (-16 -16 0) (16 16 32) START_ON x x ROTATE ROT_CONST x CRASH GIB
This is a Storgg ship for the flybys.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

CRASH (flag 64) 	Crash into the ground when killed
GIB   (flag 128)	Explode in mid air

"speed"			How fast it should fly
"pitch_speed"	degrees per second
"yaw_speed"		degrees per second
"roll_speed"	degrees per second
"accel"		Acceleration
"decel"		Deceleration
"health"	How much damage it takes before blowing up
*/

#define STROGG_CRASH	64
#define STROGG_GIB		128

extern void train_use (edict_t *self, edict_t *other, edict_t *activator);
extern void func_train_find (edict_t *self);
void misc_strogg_ship_explode (edict_t *self);

void misc_strogg_ship_remove (edict_t *self)
{
	vec3_t	org;
	int		i, n;

	for (i=0; i<5; i++)
	{
		VectorCopy (self->s.origin, org);
		org[2] += 12 + (rand()&32);
		switch (i)
		{
			case 0:
				org[0] -= 16 + (rand()&32);
				org[1] -= 16 + (rand()&32);
				break;
			case 1:
				org[0] += 16 + (rand()&32);
				org[1] += 16 + (rand()&32);
				break;
			case 2:
				org[0] += 16 + (rand()&32);
				org[1] -= 16 + (rand()&32);
				break;
			case 3:
				org[0] -= 16 + (rand()&32);
				org[1] += 16 + (rand()&32);
				break;
			case 4:
				self->s.sound = 0;

				ThrowGib (self, "models/dead/ships/strogg/front.md2", 5000, GIB_METALLIC);
				ThrowGib (self, "models/dead/ships/strogg/hub.md2", 5000, GIB_METALLIC);
				ThrowGib (self, "models/dead/ships/strogg/hub.md2", 500, GIB_METALLIC);
				ThrowGib (self, "models/dead/ships/strogg/l_wing.md2", 500, GIB_METALLIC);
				ThrowGib (self, "models/dead/ships/strogg/r_wing.md2", 500, GIB_METALLIC);

				for (n= 0; n < 5; n++)
					ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2", 500, GIB_METALLIC);

				G_FreeEdict(self);
				break;
		}

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (org);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}
}

void misc_strogg_ship_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (strcmp(other->classname, "world") == 1.0)
	{
	//	gi.dprintf("Strogg ship hit the world\n");
		
		self->prethink = NULL;
		self->think = misc_strogg_ship_remove;
		self->nextthink = level.time + 1;
	}
}

void misc_strogg_ship_explode (edict_t *self)
{
	vec3_t	org;
	int		n;

	self->think = misc_strogg_ship_explode;
	VectorCopy (self->s.origin, org);
	org[2] += 24 + (rand()&15);
	switch (self->count++)
	{
	case 0:
		org[0] -= 24;
		org[1] -= 24;
		break;
	case 1:
		org[0] += 24;
		org[1] += 24;
		break;
	case 2:
		org[0] += 24;
		org[1] -= 24;
		break;
	case 3:
		org[0] -= 24;
		org[1] += 24;
		break;
	case 4:
		self->s.sound = 0;
		for (n= 0; n < 20; n++)
			ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2", 500, GIB_METALLIC);

		self->think = NULL;
		self->nextthink = 0;
		return;
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (org);
	gi.multicast (self->s.origin, MULTICAST_PVS);

//	self->nextthink = level.time + 0.1;
}

void misc_strogg_ship_prethink (edict_t *self)
{
	vec3_t	v, org;
	float		diff;

	//self->groundentity = NULL;

	diff = self->timestamp - level.time;
	if (diff < -1.0)
		diff = -1.0;

	VectorScale (self->moveinfo.dir, 1.0 + diff, v);
	v[2] = diff;

	diff = self->s.angles[2];

	if (diff < 11)
	{
		vectoangles (v, self->s.angles);
		self->s.angles[2] = diff + 2;
	}

	if ((random() > 0.6) && (!self->groundentity))
	{
	//	gi.dprintf ("Doing random explosion\n");

		VectorCopy (self->s.origin, org);
		if (random() > 0.8)
		{
			org[0] -= 24;
			org[1] -= 24;
		}
		else
		{
			org[0] += 24;
			org[1] += 24;
		}

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (org);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}
}

void misc_strogg_ship_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;

	if (self->spawnflags & STROGG_GIB)
	{
		misc_strogg_ship_remove(self);
	//	return;
	}

	misc_strogg_ship_explode(self);

	self->s.effects |= (EF_ROCKET|EF_GRENADE|EF_GIB);
	self->use = NULL;
	self->movetype = MOVETYPE_TOSS;
	VectorSet (self->mins, -32, -32, 0);
	VectorSet (self->maxs, 32, 32, 48);
	self->prethink = misc_strogg_ship_prethink;

	self->touch = misc_strogg_ship_touch;
	//Knightmare- remove spline flag
	self->spawnflags &= ~TRAIN_SPLINE;

//	self->nextthink = level.time + 1;
//	self->touch = misc_viper_bomb_touch;

//	viper = G_Find (NULL, FOFS(classname), "misc_viper");
	VectorScale (self->moveinfo.dir, self->moveinfo.speed, self->velocity);

	self->timestamp = level.time;
}

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

	ent->class_id = ENTITY_MISC_STROGG_SHIP;

	if (!ent->speed)
		ent->speed = 300;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_NOT;
	//Mappack
	if ((ent->spawnflags && STROGG_CRASH) || (ent->spawnflags && STROGG_GIB))
	{
		if (!ent->health)
		{
			gi.dprintf ("misc_strogg_ship is killable with no health set at %s\n", vtos(ent->s.origin));
			ent->health = 40;
		}
		ent->solid = SOLID_BBOX;
		ent->takedamage = DAMAGE_YES;
		ent->die = misc_strogg_ship_die;
	}
	//Mappack
	ent->s.modelindex = gi.modelindex ("models/ships/strogg1/tris.md2");
	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 32);
	ent->s.renderfx |= RF_IR_VISIBLE;

	if (st.noise) // Kngihtmare added- movement sound
		ent->noise_index = gi.soundindex  (st.noise);

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_strogg_ship_use;
	if (!ent->spawnflags & 1)
		ent->svflags |= SVF_NOCLIENT;

	ent->blood_type = 2; //for smoking gibs

	// Knightmare- change both rotate flags to spline flag
	if ( (ent->spawnflags & TRAIN_ROTATE) && (ent->spawnflags & TRAIN_ROT_CONST) )
	{
		ent->spawnflags &= ~TRAIN_ROTATE;
		ent->spawnflags &= ~TRAIN_ROT_CONST;
		ent->spawnflags |= TRAIN_SPLINE;
	}

	// Mappack
	if(!ent->accel)
		ent->moveinfo.accel = ent->speed;
	else
		ent->moveinfo.accel = ent->accel;
	if(!ent->decel)
		ent->moveinfo.decel = ent->speed;
	else
		ent->moveinfo.decel = ent->decel;
	ent->moveinfo.speed = ent->speed;
	// Mappack
//	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	gi.linkentity (ent);
}

void SP_misc_strogg_ship_origin (edict_t *ent)
{
	ent->spawnflags |= TRAIN_ORIGIN;
	SP_misc_strogg_ship (ent);
}

// RAFAEL 17-APR-98
/*QUAKED misc_transport (1 0 0) (-256 -256 -32) (256 256 128) START_ON x x ROTATE ROT_CONST x x SMOOTH
Maxx's transport at end of game
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"			How fast it should fly
"pitch_speed"	degrees per second
"yaw_speed"		degrees per second
"roll_speed"	degrees per second
"accel"		Acceleration
"decel"		Deceleration
*/

void SP_misc_transport (edict_t *ent)
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
	ent->s.modelindex = gi.modelindex ("models/objects/ship/tris.md2");

	VectorSet (ent->mins, -256, -256, -32);
	VectorSet (ent->maxs, 256, 256, 128);

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_strogg_ship_use;
	if (!ent->spawnflags & 1)
		ent->svflags |= SVF_NOCLIENT;

	if (ent->spawnflags & TRAIN_SMOOTH)
		ent->smooth_movement = 1;
	else
		ent->smooth_movement = 0;

	// Knightmare- change both rotate flags to spline flag
	if ( (ent->spawnflags & TRAIN_ROTATE) && (ent->spawnflags & TRAIN_ROT_CONST) )
	{
		ent->spawnflags &= ~TRAIN_ROTATE;
		ent->spawnflags &= ~TRAIN_ROT_CONST;
		ent->spawnflags |= TRAIN_SPLINE;
	}

	if(!ent->accel)
		ent->moveinfo.accel = ent->speed;
	else
		ent->moveinfo.accel = ent->accel;
	if(!ent->decel)
		ent->moveinfo.decel = ent->speed;
	else
		ent->moveinfo.decel = ent->decel;
	ent->moveinfo.speed = ent->speed;
	
	gi.linkentity (ent);
}

void SP_misc_transport_origin (edict_t *ent)
{
	ent->spawnflags |= TRAIN_ORIGIN;
	SP_misc_transport (ent);
}
// END 17-APR-98


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
	ent->s.renderfx |= RF_NOSHADOW;
	gi.linkentity (ent);
}


/*QUAKED light_mine2 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine2 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex ("models/objects/minelite/light2/tris.md2");
	ent->s.renderfx |= RF_NOSHADOW;
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

/*static*/ void func_clock_reset (edict_t *self)
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

// Skuller's hack to fix crash on exiting biggun
typedef struct zhead_s {
   struct zhead_s   *prev, *next;
   short   magic;
   short   tag;         // for group free
   int      size;
} zhead_t; 

/*static*/ void func_clock_format_countdown (edict_t *self)
{
	zhead_t *z = ( zhead_t * )self->message - 1;
	int size = z->size - sizeof (zhead_t);

	if (size < CLOCK_MESSAGE_SIZE) {
		gi.TagFree (self->message);
		self->message = gi.TagMalloc (CLOCK_MESSAGE_SIZE, TAG_LEVEL);
		//gi.dprintf ("WARNING: func_clock_format_countdown: self->message is too small: %i\n", size);
	} 
	// end Skuller's hack

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

/*==================================================================
 Lazarus changes to misc_teleporter:

 START_OFF (=1)  If set, misc_teleporter must be targeted to work
 TOGGLE    (=2)  If set, misc_teleporter is toggled on/off each time
                 it is targeted
 NO_MODEL  (=4)  If set, teleporter does not use the normal teleporter
                 base or splash effect
 LANDMARK  (=64) If set, player angles and speed are preserved

 misc_teleporter selects a random pick from up to 8 targets for a 
 destination

 trigger_transition with same name as teleporter can be used to 
 teleport multiple non-player entities
================================================================== */

void teleport_transition_ents (edict_t *transition, edict_t *teleporter, edict_t *destination)
{
	extern entlist_t DoNotMove;
	int			i, j;
	int			total=0;
	qboolean	nogo=false;
	edict_t		*ent;
	entlist_t	*p;
	vec3_t		angles, forward, right, v;
	vec3_t		start;

	angles[YAW] = destination->s.angles[YAW] - teleporter->s.angles[YAW];
	angles[PITCH] = angles[ROLL] = 0;
	AngleVectors(angles,forward,right,NULL);
	VectorNegate(right,right);
	if(teleporter->solid == SOLID_TRIGGER)
		VectorMA(teleporter->absmin,0.5,teleporter->size,start);
	else
		VectorCopy(teleporter->s.origin,start);

	for(i=game.maxclients+1; i<globals.num_edicts; i++)
	{
		ent = &g_edicts[i];
		ent->id = 0;
		if(!ent->inuse) continue;
		// Pass up owned entities not owned by the player on this pass...
		// get 'em next pass so we'll know whether owner is in our list
		if(ent->owner && !ent->owner->client) continue;
		if(ent->movewith) continue;
		if(ent->solid == SOLID_BSP) continue;
		if((ent->solid == SOLID_TRIGGER) && !FindItemByClassname(ent->classname)) continue;
		// Do not under any circumstances move these entities:
		for(p=&DoNotMove, nogo=false; p->name && !nogo; p++)
			if(!Q_stricmp(ent->classname,p->name))
				nogo = true;
		if(nogo) continue;
		if(!HasSpawnFunction(ent)) continue;
		if(ent->s.origin[0] > transition->maxs[0]) continue;
		if(ent->s.origin[1] > transition->maxs[1]) continue;
		if(ent->s.origin[2] > transition->maxs[2]) continue;
		if(ent->s.origin[0] < transition->mins[0]) continue;
		if(ent->s.origin[1] < transition->mins[1]) continue;
		if(ent->s.origin[2] < transition->mins[2]) continue;
		total++;
		ent->id = total;
		if(ent->owner)
			ent->owner_id = -(ent->owner - g_edicts);
		else
			ent->owner_id = 0;
		if(angles[YAW])
		{
			vec3_t	spawn_offset;
		
			VectorSubtract(ent->s.origin,start,spawn_offset);
			VectorCopy(spawn_offset,v);
			G_ProjectSource (vec3_origin, v, forward, right, spawn_offset);
			VectorCopy(spawn_offset,ent->s.origin);
			VectorCopy(ent->velocity,v);
			G_ProjectSource (vec3_origin, v, forward, right, ent->velocity);
			ent->s.angles[YAW] += angles[YAW];
		}
		else
		{
			VectorSubtract(ent->s.origin,teleporter->s.origin,ent->s.origin);
		}
		VectorAdd(ent->s.origin,destination->s.origin,ent->s.origin);
		gi.linkentity(ent);
	}
	// Repeat, ONLY for ents owned by non-players
	for(i=game.maxclients+1; i<globals.num_edicts; i++)
	{
		ent = &g_edicts[i];
		if(!ent->inuse) continue;
		if(!ent->owner) continue;
		if(ent->owner->client) continue;
		if(ent->movewith) continue;
		if(ent->solid == SOLID_BSP) continue;
		if((ent->solid == SOLID_TRIGGER) && !FindItemByClassname(ent->classname)) continue;
		// Do not under any circumstances move these entities:
		for(p=&DoNotMove, nogo=false; p->name && !nogo; p++)
			if(!Q_stricmp(ent->classname,p->name))
				nogo = true;
		if(nogo) continue;
		if(!HasSpawnFunction(ent)) continue;
		if(ent->s.origin[0] > transition->maxs[0]) continue;
		if(ent->s.origin[1] > transition->maxs[1]) continue;
		if(ent->s.origin[2] > transition->maxs[2]) continue;
		if(ent->s.origin[0] < transition->mins[0]) continue;
		if(ent->s.origin[1] < transition->mins[1]) continue;
		if(ent->s.origin[2] < transition->mins[2]) continue;
		ent->owner_id = 0;
		for(j=game.maxclients+1; j<globals.num_edicts && !ent->owner_id; j++)
		{
			if(ent->owner == &g_edicts[j])
				ent->owner_id = g_edicts[j].id;
		}
		if(!ent->owner_id) continue;
		total++;
		ent->id = total;
		if(angles[YAW])
		{
			vec3_t	spawn_offset;
		
			VectorSubtract(ent->s.origin,start,spawn_offset);
			VectorCopy(spawn_offset,v);
			G_ProjectSource (vec3_origin, v, forward, right, spawn_offset);
			VectorCopy(spawn_offset,ent->s.origin);
			VectorCopy(ent->velocity,v);
			G_ProjectSource (vec3_origin, v, forward, right, ent->velocity);
			ent->s.angles[YAW] += angles[YAW];
		}
		else
		{
			VectorSubtract(ent->s.origin,teleporter->s.origin,ent->s.origin);
		}
		VectorAdd(ent->s.origin,destination->s.origin,ent->s.origin);
		gi.linkentity(ent);
	}
}

// G_PickDestination is identical to G_PickTarget, but w/o the 
// obnoxious error message.

#define MAXCHOICES 8
static edict_t *G_PickDestination (char *targetname)
{
	edict_t	*ent = NULL;
	int		num_choices = 0;
	edict_t	*choice[MAXCHOICES];

	if (!targetname)
	{
		gi.dprintf("G_PickDestination called with NULL targetname\n");
		return NULL;
	}

	while(1)
	{
		ent = G_Find (ent, FOFS(targetname), targetname);
		if (!ent)
			break;
		choice[num_choices++] = ent;
		if (num_choices == MAXCHOICES)
			break;
	}

	if (!num_choices)
		return NULL;

	return choice[rand() % num_choices];
}

//=================================================================================
#define TELEPORTER_START_OFF	1
#define TELEPORTER_TOGGLE		2
#define TELEPORTER_NO_MODEL		4
#define TELEPORTER_MONSTER		8
#define TELEPORTER_LANKMARK		64

void teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;
	edict_t		*teleporter;
	qboolean	landmark;
	int			i;

	// Lazarus: teleporters with MONSTER spawnflag can be used by monsters
	if (!other->client && (!(self->spawnflags & TELEPORTER_MONSTER) || !(other->svflags & SVF_MONSTER)))
		return;
	// Lazarus: Use G_PickDestination instead so we can have multiple destinations
	//dest = G_Find (NULL, FOFS(targetname), self->target);
	dest = G_PickDestination(self->target);
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

	if(self->owner)
		teleporter = self->owner;
	else
		teleporter = self;

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	VectorCopy (dest->s.origin, other->s.old_origin);
	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	landmark = (teleporter->spawnflags & 64 ? true : false);

	if(landmark)
	{
		vec3_t	angles, forward, right, v;

		VectorSubtract(dest->s.angles,teleporter->s.angles,angles);
		AngleVectors(angles,forward,right,NULL);
		VectorNegate(right,right);
		VectorCopy(other->velocity,v);
		G_ProjectSource (vec3_origin,
					     v, forward, right,
						 other->velocity);
	}
	else
		VectorClear (other->velocity);

	if(other->client)
	{
		other->client->ps.pmove.pm_time = 160>>3;		// hold time
		other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
	}

	// draw the teleport splash at source and on the player
	if(self->owner)
		self->owner->s.event = EV_PLAYER_TELEPORT;
	else
	{
		// This must be a trigger_teleporter
		vec3_t	origin;
		VectorMA(self->mins,0.5,self->size,origin);

		if(!(self->spawnflags & 16))
		{
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_TELEPORT_EFFECT);
			gi.WritePosition(origin);
			gi.multicast(origin, MULTICAST_PHS);
		}
		gi.positioned_sound(origin,self,CHAN_AUTO,self->noise_index,1,1,0);
	}
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	if(landmark)
	{
		edict_t	*transition;

		// LANDMARK
		if(other->client)
		{
			other->s.angles[ROLL] = 0;
			for (i=0 ; i<3 ; i++)
				other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - teleporter->s.angles[i] + other->s.angles[i] - other->client->resp.cmd_angles[i]);
			VectorClear (other->client->ps.viewangles);
			VectorClear (other->client->v_angle);
			VectorClear (other->s.angles);
		}
		else
		{
			VectorSubtract(other->s.angles,teleporter->s.angles,other->s.angles);
			VectorAdd(other->s.angles,dest->s.angles,other->s.angles);
		}

		transition = G_Find(NULL,FOFS(classname),"trigger_transition");
		while(transition)
		{
			if(!Q_stricmp(transition->targetname,teleporter->targetname))
				teleport_transition_ents(transition,teleporter,dest);
			transition = G_Find(transition,FOFS(classname),"trigger_transition");
		}

	}
	else
	{
		if(other->client)
		{
			for (i=0 ; i<3 ; i++)
				other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);
			VectorClear (other->s.angles);
			VectorClear (other->client->ps.viewangles);
			VectorClear (other->client->v_angle);
		}
		else
			VectorCopy (dest->s.angles,other->s.angles);
	}
	// kill anything at the destination
	KillBox (other);

	if(!self->owner)
	{
		// this is a trigger_teleporter
		self->count--;
		if (self->count == 0)
			G_FreeEdict(self);
	}
	gi.linkentity (other);
}

void teleporter_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if(!(self->spawnflags & TELEPORTER_START_OFF))
	{
		if(!(self->spawnflags & TELEPORTER_TOGGLE))
			return;
		self->spawnflags |= TELEPORTER_START_OFF;
		self->s.effects &= ~EF_TELEPORTER;
		self->target_ent->solid = SOLID_NOT;
		self->s.sound = 0;
	}
	else
	{
		self->spawnflags &= ~TELEPORTER_START_OFF;
		if(!(self->spawnflags & TELEPORTER_NO_MODEL))
			self->s.effects |= EF_TELEPORTER;
		self->target_ent->solid = SOLID_TRIGGER;
		self->s.sound = gi.soundindex ("world/amb10.wav");
	}
	gi.linkentity(self);
	gi.linkentity(self->target_ent);
}

/*QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16) START_OFF TOGGLE NO_MODEL MONSTER x x LANDMARK
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.

"target" name of misc_teleporter_dest
"targetname" to trigger it on/off
"angles"
"movewith" if you want it on a train or other moving ent!
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
	// yell if the spawnflags are odd
	if (ent->spawnflags & TELEPORTER_START_OFF)
	{
		if (!(ent->spawnflags & TELEPORTER_TOGGLE))
		{
			gi.dprintf("misc_teleporter START_OFF without TOGGLE\n");
			ent->spawnflags |= TELEPORTER_TOGGLE;
		}
	}
	if (!(ent->spawnflags & TELEPORTER_NO_MODEL))
	{
		gi.setmodel (ent, "models/objects/dmspot/tris.md2");
		ent->s.skinnum = 1;
		if(!(ent->spawnflags & TELEPORTER_START_OFF))
		{
			ent->s.effects = EF_TELEPORTER;
			ent->s.sound = gi.soundindex ("world/amb10.wav");
		}
	}
	if (ent->spawnflags & 3)
		ent->use = teleporter_use;

	if (ent->spawnflags & TELEPORTER_NO_MODEL)
	{
		ent->solid = SOLID_NOT;
	}
	else
	{
		ent->solid = SOLID_BBOX;
		VectorSet (ent->mins, -32, -32, -24);
		VectorSet (ent->maxs, 32, 32, -16);
	}
	// Knightmare added- make IR visible
	ent->s.renderfx |= RF_IR_VISIBLE; 

	ent->common_name = "Teleporter";
	gi.linkentity (ent);

	trig = G_Spawn ();
	ent->target_ent = trig;
	trig->touch = teleporter_touch;
	if(ent->spawnflags & TELEPORTER_START_OFF)
		trig->solid = SOLID_NOT;
	else
		trig->solid = SOLID_TRIGGER;
	trig->target = ent->target;
	trig->owner = ent;
	// Lazarus
	trig->movewith = ent->movewith;
	VectorCopy (ent->s.origin, trig->s.origin);
	VectorSet (trig->mins, -8, -8, 8);
	VectorSet (trig->maxs, 8, 8, 24);
	gi.linkentity (trig);

}

void trigger_teleporter_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if(self->solid == SOLID_NOT)
		self->solid = SOLID_TRIGGER;
	else
		self->solid = SOLID_NOT;
	gi.linkentity(self);
}

// trigger_teleporter
// spawnflags
//  2 = TRIGGERED
//  8 = MONSTER
// 16 = NO EFFECT
// 32 = CUSTOM
/*QUAKED trigger_teleporter (.5 .5 .5) ? x Triggered x Monster NoEffects Sound Landmark
Teleporter field that can transport monsters or players

Use the 'SOUND' spawnflag and then choose your own custom sound.
"noise" (path/file.wav)
"target" destination
*/
void SP_trigger_teleporter (edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf ("trigger_teleporter without a target.\n");
		G_FreeEdict (self);
		return;
	}
	if (!(self->spawnflags & 48))
		self->noise_index = gi.soundindex("misc/tele1.wav");
	else if(self->spawnflags & 32)
	{
		if(st.noise)
			self->noise_index = gi.soundindex(st.noise);
		else
			self->noise_index = 0;
	}
	else
		self->noise_index = 0;

	gi.setmodel (self, self->model);
	self->touch   = teleporter_touch;
	self->use = trigger_teleporter_use;
	self->svflags = SVF_NOCLIENT;
	if(self->spawnflags & 2)
		self->solid = SOLID_NOT;
	else
		self->solid = SOLID_TRIGGER;

	gi.linkentity (self);
}

/*QUAKED trigger_teleporter_bbox (.5 .5 .5) (-8 -8 -8) (8 8 8) x Triggered x Monster NoEffects Sound Landmark
Teleporter field that can transport monsters or players

Use the 'SOUND' spawnflag and then choose your own custom sound.
"noise" (path/file.wav)
"target" destination

bleft Min b-box coords XYZ. Default = -16 -16 -16
tright Max b-box coords XYZ. Default = 16 16 16
*/
void SP_trigger_teleporter_bbox (edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf ("trigger_teleporter_bbox without a target at %s\n", vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}
	if (!(self->spawnflags & 48))
		self->noise_index = gi.soundindex("misc/tele1.wav");
	else if(self->spawnflags & 32)
	{
		if(st.noise)
			self->noise_index = gi.soundindex(st.noise);
		else
			self->noise_index = 0;
	}
	else
		self->noise_index = 0;

	if ( (!VectorLength(self->bleft)) && (!VectorLength(self->tright)) )
	{
		VectorSet(self->bleft, -16, -16, -16);
		VectorSet(self->tright, 16, 16, 16);
	}
	VectorCopy (self->bleft, self->mins);
	VectorCopy (self->tright, self->maxs);
	self->touch   = teleporter_touch;
	self->use = trigger_teleporter_use;
	self->svflags = SVF_NOCLIENT;
	if(self->spawnflags & 2)
		self->solid = SOLID_NOT;
	else
		self->solid = SOLID_TRIGGER;

	gi.linkentity (self);
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
	ent->s.renderfx |= RF_IR_VISIBLE; // Knightmare added- make IR visible
	VectorSet (ent->mins, -32, -32, -24);
	VectorSet (ent->maxs, 32, 32, -16);
	gi.linkentity (ent);
}

/*QUAKED misc_amb4 (1 0 0) (-8 -8 -8) (8 8 8)
Mal's amb4 loop entity
*/
static int amb4sound;

void amb4_think (edict_t *ent)
{
	ent->nextthink = level.time + 2.7;
	gi.sound(ent, CHAN_VOICE, amb4sound, 1, ATTN_NONE, 0);
}

void SP_misc_amb4 (edict_t *ent)
{
	ent->think = amb4_think;
	ent->nextthink = level.time + 1;
	amb4sound = gi.soundindex ("world/amb4.wav");
	gi.linkentity (ent);
}

//======================
//ROGUE
void misc_nuke_core_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if(self->svflags & SVF_NOCLIENT)
		self->svflags &= ~SVF_NOCLIENT;
	else
		self->svflags |= SVF_NOCLIENT;
}

/*QUAKED misc_nuke_core (1 0 0) (-16 -16 -16) (16 16 16)
toggles visible/not visible. starts visible.
*/
void SP_misc_nuke_core (edict_t *ent)
{
	gi.setmodel (ent, "models/objects/core/tris.md2");
	ent->s.renderfx |= RF_IR_VISIBLE;
	gi.linkentity (ent);

	ent->use = misc_nuke_core_use;
}
//ROGUE
//======================

/*QUAKED misc_nuke (1 0 0) (-16 -16 -16) (16 16 16)
*/

void use_nuke (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *from = g_edicts;

	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (from == self)
			continue;
		if (from->client)
		{
			T_Damage (from, self, self, vec3_origin, from->s.origin, vec3_origin, 100000, 1, 0, MOD_TRAP);
		}
		else if (from->svflags & SVF_MONSTER)
		{
			G_FreeEdict (from);
		}
	}

	self->use = NULL;
}

void SP_misc_nuke (edict_t *ent)
{
	ent->use = use_nuke;
}

//Knightmare- moved Xatrix rotating light and repair object here

/*QUAKED rotating_light (0 1 0) (-8 -8 -8) (8 8 8) START_OFF ALARM
"health"	if set, the light may be killed.
*/

// RAFAEL 
// note to self
// the lights will take damage from explosions
// this could leave a player in total darkness very bad
 
#define START_OFF	1

void rotating_light_alarm (edict_t *self)
{
	if (self->spawnflags & START_OFF)
	{
		self->think = NULL;
		self->nextthink = 0;	
	}
	else
	{
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0);
		self->nextthink = level.time + 1;
	}
}

void rotating_light_killed (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WELDING_SPARKS);
	gi.WriteByte (30);
	gi.WritePosition (self->s.origin);
	gi.WriteDir (vec3_origin);
	gi.WriteByte (0xe0 + (rand()&7));
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->s.effects &= ~EF_SPINNINGLIGHTS;
	self->use = NULL;

	self->think = G_FreeEdict;	
	self->nextthink = level.time + 0.1;	
}

/*static*/ void rotating_light_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & START_OFF)
	{
		self->spawnflags &= ~START_OFF;
		self->s.effects |= EF_SPINNINGLIGHTS;

		if (self->spawnflags & 2)
		{
			self->think = rotating_light_alarm;
			self->nextthink = level.time + 0.1;
		}
	}
	else
	{
		self->spawnflags |= START_OFF;
		self->s.effects &= ~EF_SPINNINGLIGHTS;
	}
}
	
void SP_rotating_light (edict_t *self)
{

	self->movetype = MOVETYPE_STOP;
	self->solid = SOLID_BBOX;
	
	self->s.modelindex = gi.modelindex ("models/objects/light/tris.md2");
	
	self->s.frame = 0;
		
	self->use = rotating_light_use;
	
	if (self->spawnflags & START_OFF)
		self->s.effects &= ~EF_SPINNINGLIGHTS;
	else
	{
		self->s.effects |= EF_SPINNINGLIGHTS;
	}
	self->s.renderfx |=	RF_IR_VISIBLE;
	if (!self->speed)
		self->speed = 32;
	// this is a real cheap way
	// to set the radius of the light
	// self->s.frame = self->speed;

	if (!self->health)
	{
		self->health = 10;
		self->max_health = self->health;
		self->die = rotating_light_killed;
		self->takedamage = DAMAGE_YES;
	}
	else
	{
		self->max_health = self->health;
		self->die = rotating_light_killed;
		self->takedamage = DAMAGE_YES;
	}
	
	if (self->spawnflags & 2)
	{
		self->moveinfo.sound_start = gi.soundindex ("misc/alarm.wav");	
	}
	
	gi.linkentity (self);

}

/*QUAKED func_object_repair (1 .5 0) (-8 -8 -8) (8 8 8) 
object to be repaired.
The default delay is 1 second
"delay" the delay in seconds for spark to occur
*/

void object_repair_fx (edict_t *ent)
{
	ent->nextthink = level.time + ent->delay;

	if (ent->health <= 100)
		ent->health++;
 	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_WELDING_SPARKS);
		gi.WriteByte (10);
		gi.WritePosition (ent->s.origin);
		gi.WriteDir (vec3_origin);
		gi.WriteByte (0xe0 + (rand()&7));
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
}


void object_repair_dead (edict_t *ent)
{
	G_UseTargets (ent, ent);
	ent->nextthink = level.time + 0.1;
	ent->think = object_repair_fx;
}

void object_repair_sparks (edict_t *ent)
{
	if (ent->health < 0)
	{
		ent->nextthink = level.time + 0.1;
		ent->think = object_repair_dead;
		return;
	}

	ent->nextthink = level.time + ent->delay;
	
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WELDING_SPARKS);
	gi.WriteByte (10);
	gi.WritePosition (ent->s.origin);
	gi.WriteDir (vec3_origin);
	gi.WriteByte (0xe0 + (rand()&7));
	gi.multicast (ent->s.origin, MULTICAST_PVS);	
}

void SP_object_repair (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->classname = "object_repair";
	VectorSet (ent->mins, -8, -8, 8);
	VectorSet (ent->maxs, 8, 8, 8);
	ent->think = object_repair_sparks;
	ent->nextthink = level.time + 1.0;
	ent->health = 100;
	if (!ent->delay)
		ent->delay = 1.0;	
}

//Knightmare
/*QUAKED misc_sick_guard (1 0 0) (-24 -32 -26) (8 12 -2)
model="models/monsters/sick_guard/"
*/
void misc_sick_guard_think (edict_t *ent)
{
	float r1, r2;

	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
	r1 = random();
	r2 = random();
	if (r1 <= .033)
	{
		if (r2 <= 0.33)
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("monsters/sick_guard/patient1b.wav"), 1, ATTN_IDLE, 0);
		else if (r2 <= 0.67)
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("monsters/sick_guard/patient2.wav"), 1, ATTN_IDLE, 0);
		else
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("monsters/sick_guard/patient3.wav"), 1, ATTN_IDLE, 0);
	}

}

void SP_misc_sick_guard (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	VectorSet (self->mins, -24, -32, -26);
	VectorSet (self->maxs, 8, 12, -2);
	self->s.modelindex = gi.modelindex ("models/monsters/sick_guard/tris.md2");
	self->s.renderfx |=	RF_IR_VISIBLE;
	self->s.frame = rand() % 16;
	gi.linkentity (self);

	self->think = misc_sick_guard_think;
	self->nextthink = level.time + FRAMETIME;
}

void SP_model_spawn (edict_t *ent);


/*QUAKED misc_gekk_writhe (1 .5 0) (-24 -24 -24) (24 24 24)
Not solid due to model not being aligned with bbox- you must put a clip brush around it.
Will gib when targeted.
model="models/monsters/gekk_writhe/"
*/
void misc_gekk_writhe_think (edict_t *self)
{
	if (++self->s.frame < FRAME_death4_35) //was FRAME_death4_07
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = FRAME_death4_01; //was FRAME_death4_03
		self->nextthink = level.time + FRAMETIME;
	}
}

// gib function
void misc_gekk_writhe_use (edict_t *self, edict_t *other, edict_t *activator)
{
	int damage = 20;

	gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);

	ThrowGib (self, "models/objects/gekkgib/pelvis/tris.md2", damage, GIB_ORGANIC);
	ThrowGib (self, "models/objects/gekkgib/arm/tris.md2", damage, GIB_ORGANIC);
	ThrowGib (self, "models/objects/gekkgib/arm/tris.md2", damage, GIB_ORGANIC);
	ThrowGib (self, "models/objects/gekkgib/torso/tris.md2", damage, GIB_ORGANIC);
	ThrowGib (self, "models/objects/gekkgib/claw/tris.md2", damage, GIB_ORGANIC);
	ThrowGib (self, "models/objects/gekkgib/leg/tris.md2", damage, GIB_ORGANIC);
	ThrowGib (self, "models/objects/gekkgib/leg/tris.md2", damage, GIB_ORGANIC);
	ThrowHead (self, "models/objects/gekkgib/head/tris.md2", damage, GIB_ORGANIC);

	G_UseTargets (self, activator);
}

void SP_misc_gekk_writhe (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	VectorSet (self->mins, -24, -24, -24);
	VectorSet (self->maxs, 24, 24, 24);
	self->s.modelindex = gi.modelindex ("models/monsters/gekk/tris.md2");
	self->s.renderfx |=	RF_IR_VISIBLE;
	self->s.frame = FRAME_death4_01; //was FRAME_death4_03
	self->use = misc_gekk_writhe_use;
	self->blood_type = 1; //Knightmare- set this for blood color
	gi.linkentity (self);

	self->think = misc_gekk_writhe_think;
	self->nextthink = level.time + FRAMETIME;
}


/*
=============================================================

Coconut Monkey 3 Flame entities

=============================================================
*/
#define FLAME_START_OFF	1
void hurt_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

/*static*/ void light_flame_use (edict_t *self, edict_t *other, edict_t *activator)
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
	self->s.modelindex = gi.modelindex ("sprites/s_flame1.sp2");
	VectorSet(self->mins,-48,-48,-32);
	VectorSet(self->maxs, 48, 48, 64);
	light_flame_spawn (self);
}

void SP_light_flame1s (edict_t *self)
{
	self->s.modelindex = gi.modelindex ("sprites/s_flame1s.sp2");
	VectorSet(self->mins,-16,-16,-16);
	VectorSet(self->maxs, 16, 16, 32);
	light_flame_spawn (self);
}

void SP_light_flame2 (edict_t *self)
{
	self->s.modelindex = gi.modelindex ("sprites/s_flame2.sp2");
	VectorSet(self->mins,-48,-48,-32);
	VectorSet(self->maxs, 48, 48, 64);
	light_flame_spawn (self);
}

void SP_light_flame2s (edict_t *self)
{
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

	gi.linkentity (self);
}

/*
=============================================================

Lazarus new entities

=============================================================
*/

void misc_light_think (edict_t *self)
{
	if(self->spawnflags & START_OFF)
		return;
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_FLASHLIGHT);
	gi.WritePosition (self->s.origin);
	gi.WriteShort (self - g_edicts);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	self->nextthink = level.time + FRAMETIME;
}

void misc_light_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & START_OFF)
	{
		self->spawnflags &= ~START_OFF;
		self->nextthink = level.time + FRAMETIME;
	}
	else
		self->spawnflags |= START_OFF;
}

void SP_misc_light (edict_t *self)
{
	self->use = misc_light_use;
	if(self->movewith)
		self->movetype = MOVETYPE_PUSH;
	else
		self->movetype = MOVETYPE_NONE;
	self->think = misc_light_think;
	if (!(self->spawnflags & START_OFF))
		self->nextthink = level.time + 2*FRAMETIME;
}

/*
=============================================================

TARGET_PRECIPITATION

=============================================================
*/

#define SF_WEATHER_STARTON        1
#define SF_WEATHER_SPLASH         2
#define SF_WEATHER_GRAVITY_BOUNCE 4
#define SF_WEATHER_FIRE_ONCE      8
#define SF_WEATHER_START_FADE     16
#define STYLE_WEATHER_RAIN        0
#define STYLE_WEATHER_BIGRAIN     1
#define STYLE_WEATHER_SNOW        2
#define STYLE_WEATHER_LEAF        3
#define STYLE_WEATHER_USER        4

void drop_add_to_chain(edict_t *drop)
{
	edict_t	*owner = drop->owner;
	edict_t *parent;

	if(!owner || !owner->inuse || !(owner->spawnflags & SF_WEATHER_STARTON))
	{
		G_FreeEdict(drop);
		return;
	}
	parent = owner;
	while(parent->child)
		parent = parent->child;
	parent->child = drop;
	drop->child = NULL;
	drop->svflags |= SVF_NOCLIENT;
	drop->s.effects &= ~EF_SPHERETRANS;
	drop->s.renderfx &= ~RF_TRANSLUCENT;
	VectorClear(drop->velocity);
	VectorClear(drop->avelocity);
	gi.linkentity(drop);
}

void drop_splash(edict_t *drop)
{
	vec3_t	up = {0,0,1};
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_LASER_SPARKS);
	gi.WriteByte (drop->owner->mass2);
	gi.WritePosition (drop->s.origin);
	gi.WriteDir (up);
	gi.WriteByte (drop->owner->sounds);
	gi.multicast (drop->s.origin, MULTICAST_PVS);
	drop_add_to_chain(drop);
}

void leaf_fade2(edict_t *ent)
{
	ent->count++;
	if (ent->count == 1)
	{
		ent->s.effects |= EF_SPHERETRANS;
		ent->nextthink=level.time+0.5;
		gi.linkentity(ent);
	}
	else
		drop_add_to_chain(ent);
}

void leaf_fade (edict_t *ent)
{
	ent->s.renderfx   = RF_TRANSLUCENT;
	ent->think        = leaf_fade2;
	ent->nextthink    = level.time+0.5;
	ent->count        = 0;
	gi.linkentity(ent);
}

void drop_touch(edict_t *drop, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if(drop->owner->spawnflags & SF_WEATHER_START_FADE)
		return;
	else if(drop->fadeout > 0)
	{
		if( (drop->spawnflags & SF_WEATHER_GRAVITY_BOUNCE) && (drop->owner->gravity > 0))
		{
			drop->movetype = MOVETYPE_DEBRIS;
			drop->gravity  = drop->owner->gravity;
		}
		drop->think     = leaf_fade;
		drop->nextthink = level.time + drop->fadeout;
	}
	else if(drop->spawnflags & SF_WEATHER_SPLASH)
		drop_splash(drop);
	else
		drop_add_to_chain(drop);
}

void spawn_precipitation(edict_t *self, vec3_t org, vec3_t dir, float speed)
{
	edict_t *drop;

	if(self->child)
	{
		// Then we already have a currently unused, invisible drop available
		drop = self->child;
		self->child = drop->child;
		drop->child = NULL;
		drop->svflags &= ~SVF_NOCLIENT;
		drop->groundentity = NULL;
	}
	else
	{
		drop = G_Spawn();
		if(self->style == STYLE_WEATHER_BIGRAIN)
			drop->s.modelindex = gi.modelindex ("models/objects/drop/heavy.md2");
		else if(self->style == STYLE_WEATHER_SNOW)
			drop->s.modelindex = gi.modelindex ("models/objects/snow/tris.md2");
		else if(self->style == STYLE_WEATHER_LEAF)
		{
			float	r=random();
			if(r < 0.33)
				drop->s.modelindex = gi.modelindex ("models/objects/leaf1/tris.md2");
			else if(r < 0.66)
				drop->s.modelindex = gi.modelindex ("models/objects/leaf2/tris.md2");
			else
				drop->s.modelindex = gi.modelindex ("models/objects/leaf3/tris.md2");
			VectorSet(drop->mins,-1,-1,-1);
			VectorSet(drop->maxs, 1, 1, 1);
		}
		else if(self->style == STYLE_WEATHER_USER)
			drop->s.modelindex = gi.modelindex(self->usermodel);
		else
			drop->s.modelindex = gi.modelindex ("models/objects/drop/tris.md2");
		drop->classname = "rain drop";
	}
	if(self->gravity > 0. || self->attenuation > 0 )
		drop->movetype = MOVETYPE_DEBRIS;
	else
		drop->movetype = MOVETYPE_RAIN;

	drop->touch    = drop_touch;
	if(self->style == STYLE_WEATHER_USER)
		drop->clipmask = MASK_MONSTERSOLID;
	else if((self->fadeout > 0) && (self->gravity == 0.))
		drop->clipmask = MASK_SOLID | CONTENTS_WATER;
	else
		drop->clipmask = MASK_MONSTERSOLID | CONTENTS_WATER;
	drop->solid    = SOLID_BBOX;
	drop->svflags  = SVF_DEADMONSTER;
	VectorSet(drop->mins,-1,-1,-1);
	VectorSet(drop->maxs, 1, 1, 1);

	if(self->spawnflags & SF_WEATHER_GRAVITY_BOUNCE)
		drop->gravity = self->gravity;
	else
		drop->gravity = 0.;
	drop->attenuation = self->attenuation;
	drop->mass        = self->mass;
	drop->spawnflags  = self->spawnflags;
	drop->fadeout     = self->fadeout;
	drop->owner       = self;

	VectorCopy (org, drop->s.origin);
	vectoangles(dir, drop->s.angles);
	drop->s.angles[PITCH] -= 90;
	VectorScale (dir, speed, drop->velocity);
	if(self->style == STYLE_WEATHER_LEAF)
	{
		drop->avelocity[PITCH] = crandom() * 360;
		drop->avelocity[YAW] = crandom() * 360;
		drop->avelocity[ROLL] = crandom() * 360;
	}
	else if(self->style == STYLE_WEATHER_USER)
	{
		drop->s.effects = self->effects;
		drop->s.renderfx = self->renderfx;
		drop->avelocity[PITCH] = crandom() * self->pitch_speed;
		drop->avelocity[YAW]   = crandom() * self->yaw_speed;
		drop->avelocity[ROLL]  = crandom() * self->roll_speed;
	}
	else
	{
		drop->s.effects |= EF_SPHERETRANS;
		drop->avelocity[YAW] = self->yaw_speed;
	}
	if(self->spawnflags & SF_WEATHER_START_FADE)
	{
		drop->think     = leaf_fade;
		drop->nextthink = level.time + self->fadeout;
	}
	gi.linkentity(drop);
}

void target_precipitation_think (edict_t *self)
{
	vec3_t	center;
	vec3_t	org;
	int		r, i;
	float	u, v, z;
	float	temp;
	qboolean	can_see_me;

	self->nextthink = level.time + FRAMETIME;

	// Don't start raining until player is in the game. The following 
	// takes care of both initial map load conditions and restored saved games.
	// This is a gross abuse of groundentity_linkcount. Sue me.
	if(g_edicts[1].linkcount == self->groundentity_linkcount)
		return;
	else
		self->groundentity_linkcount = g_edicts[1].linkcount;
	// Don't spawn drops if player can't see us. This SEEMS like an obvious
	// thing to do, but can cause visual problems if mapper isn't careful.
	// For example, placing target_precipitation where it isn't in the PVS
	// of the player's current position, but the result (rain) IS in the
	// PVS. In any case, this step is necessary to prevent overflows when
	// player suddenly encounters rain.
	can_see_me = false;
	for(i=1; i<=game.maxclients && !can_see_me; i++)
	{
		if(!g_edicts[i].inuse) continue;
		if(gi.inPVS(g_edicts[i].s.origin,self->s.origin))
			can_see_me = true;
	}
	if(!can_see_me) return;

	// Count is models/second. We accumulate a probability of a model
	// falling this frame in ->density. Yeah its a misnomer but density isn't 
	// used for anything else so it works fine.

	temp = 0.1*(self->density + crandom()*self->random);
	r = (int)(temp);
	if(r > 0)
		self->density = self->count + (temp-(float)r)*10;
	else
		self->density += (temp*10);
	if(r < 1) return;

	VectorAdd(self->bleft,self->tright,center);
	VectorMA(self->s.origin,0.5,center,center);

	for(i=0; i<r; i++)
	{
		u = crandom() * (self->tright[0] - self->bleft[0])/2;
		v = crandom() * (self->tright[1] - self->bleft[1])/2;
		z = crandom() * (self->tright[2] - self->bleft[2])/2;
		
		VectorCopy(center, org);
		
		org[0] += u;
		org[1] += v;
		org[2] += z;

		spawn_precipitation(self, org, self->movedir, self->speed);
	}
}

void target_precipitation_use (edict_t *ent, edict_t *other, edict_t *activator)
{
	if(ent->spawnflags & SF_WEATHER_STARTON)
	{
		// already on; turn it off
		ent->nextthink = 0;
		ent->spawnflags &= ~SF_WEATHER_STARTON;
		if(ent->child)
		{
			edict_t	*child, *parent;
			child = ent->child;
			ent->child = NULL;
			while(child)
			{
				parent = child;
				child = parent->child;
				G_FreeEdict(parent);
			}
		}
	}
	else
	{
		ent->density = ent->count;
		ent->think = target_precipitation_think;
		ent->spawnflags |= SF_WEATHER_STARTON;
		ent->think(ent);
	}
}

void target_precipitation_delayed_use (edict_t *self)
{
	// Since target_precipitation tends to be a processor hog,
	// for START_ON we wait until the player has spawned into the
	// game to ease the startup burden somewhat
	if(g_edicts[1].linkcount)
	{
		self->think = target_precipitation_think;
		self->think(self);
	}
	else
		self->nextthink = level.time + FRAMETIME;
}
void SP_target_precipitation (edict_t *ent)
{
	if(deathmatch->value || coop->value)
	{
		G_FreeEdict(ent);
		return;
	}

	ent->class_id = ENTITY_TARGET_PRECIPITATION;

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;

	if(ent->spawnflags & SF_WEATHER_STARTON)
	{
		ent->think = target_precipitation_delayed_use;
		ent->nextthink = level.time + 1;
	}

	if(ent->style == STYLE_WEATHER_USER)
	{
		char	*buffer;
		if(!ent->usermodel)
		{
			gi.dprintf("target_precipitation style=user\nwith no usermodel.\n");
			G_FreeEdict(ent);
			return;
		}

		// Knightmare- check for "models/" or "sprites/" already in path
		if ( strncmp(ent->usermodel, "models/", 7) && strncmp(ent->usermodel, "sprites/", 8) )
		{
			buffer = gi.TagMalloc(strlen(ent->usermodel)+10,TAG_LEVEL);
			if (strstr(ent->usermodel,".sp2"))
				sprintf(buffer, "sprites/%s", ent->usermodel);
			else
				sprintf(buffer, "models/%s", ent->usermodel);
			ent->usermodel = buffer;
		}

		if(st.gravity)
			ent->gravity = atof(st.gravity);
		else
			ent->gravity = 0.;
	}
	else
	{
		ent->gravity = 0.;
		ent->attenuation = 0.;
	}

	// If not rain or "user", turn off splash. Yeah I know goofy mapper
	// might WANT splash, but we're enforcing good taste here :)
	if(ent->style > STYLE_WEATHER_BIGRAIN && ent->style != STYLE_WEATHER_USER)
		ent->spawnflags &= ~SF_WEATHER_SPLASH;

	ent->use = target_precipitation_use;
	
	if(!ent->count)
		ent->count = 1;

	if(!ent->sounds)
		ent->sounds = 2;	// blue splash

	if(!ent->mass2)
		ent->mass2 = 8;		// 8 particles in splash

	if((ent->style < STYLE_WEATHER_RAIN) || (ent->style > STYLE_WEATHER_USER))
		ent->style = STYLE_WEATHER_RAIN;		// single rain drop model

	if(ent->speed <= 0)
	{
		switch(ent->style)
		{
		case STYLE_WEATHER_SNOW: ent->speed = 50; break;
		case STYLE_WEATHER_LEAF: ent->speed = 50; break;
		default: ent->speed = 300;
		}
	}

	if((VectorLength(ent->bleft) == 0.) && (VectorLength(ent->tright) == 0.))
	{
		// Default distribution places raindrops vertically for
		// full coverage, to help avoid "lumps"
		VectorSet(ent->bleft,-512,-512, -ent->speed*0.05);
		VectorSet(ent->tright,512, 512,  ent->speed*0.05);
	}

	if(VectorLength(ent->s.angles) > 0)
		G_SetMovedir(ent->s.angles,ent->movedir);
	else
		VectorSet(ent->movedir,0,0,-1);

	ent->density = ent->count;

	gi.linkentity (ent);
}

//=============================================================================
// TARGET_FOUNTAIN is identical to TARGET_PRECIPITATION, with these exceptions:
// ALL styles are "user-defined" (no predefined rain, snow, etc.)
// Models are spawned from a point source, and bleft/tright form a box within
// which the target point is found.
//=============================================================================
void target_fountain_think (edict_t *self)
{
	vec3_t	center;
	vec3_t	org;
	vec3_t	dir;
	int		r, i;
	float	u, v, z;
	float	temp;
	qboolean	can_see_me;

	if(!(self->spawnflags & SF_WEATHER_FIRE_ONCE))
		self->nextthink = level.time + FRAMETIME;

	// Don't start raining until player is in the game. The following 
	// takes care of both initial map load conditions and restored saved games.
	// This is a gross abuse of groundentity_linkcount. Sue me.
	if(g_edicts[1].linkcount == self->groundentity_linkcount)
		return;
	else
		self->groundentity_linkcount = g_edicts[1].linkcount;
	// Don't spawn drops if player can't see us. This SEEMS like an obvious
	// thing to do, but can cause visual problems if mapper isn't careful.
	// For example, placing target_precipitation where it isn't in the PVS
	// of the player's current position, but the result (rain) IS in the
	// PVS. In any case, this step is necessary to prevent overflows when
	// player suddenly encounters rain.
	can_see_me = false;
	for(i=1; i<=game.maxclients && !can_see_me; i++)
	{
		if(!g_edicts[i].inuse) continue;
		if(gi.inPVS(g_edicts[i].s.origin,self->s.origin))
			can_see_me = true;
	}
	if(!can_see_me) return;

	// Count is models/second. We accumulate a probability of a model
	// falling this frame in ->density. Yeah its a misnomer but density isn't 
	// used for anything else so it works fine.

	temp = 0.1*(self->density + crandom()*self->random);
	r = (int)(temp);
	if(r > 0)
		self->density = self->count;
	else
		self->density += (temp*10);
	if(r < 1) return;

	VectorAdd(self->bleft,self->tright,center);
	VectorMA(self->s.origin,0.5,center,center);

	for(i=0; i<r; i++)
	{
		u = crandom() * (self->tright[0] - self->bleft[0])/2;
		v = crandom() * (self->tright[1] - self->bleft[1])/2;
		z = crandom() * (self->tright[2] - self->bleft[2])/2;
		
		VectorCopy(center, org);
		
		org[0] += u;
		org[1] += v;
		org[2] += z;
		VectorSubtract(org,self->s.origin,dir);
		VectorNormalize(dir);

		spawn_precipitation(self, self->s.origin, dir, self->speed);
	}
}

void target_fountain_use (edict_t *ent, edict_t *other, edict_t *activator)
{
	if((ent->spawnflags & SF_WEATHER_STARTON) && !(ent->spawnflags & SF_WEATHER_FIRE_ONCE))
	{
		// already on; turn it off
		ent->nextthink = 0;
		ent->spawnflags &= ~SF_WEATHER_STARTON;
		if(ent->child)
		{
			edict_t	*child, *parent;
			child = ent->child;
			ent->child = NULL;
			while(child)
			{
				parent = child;
				child = parent->child;
				G_FreeEdict(parent);
			}
		}
	}
	else
	{
		ent->density = ent->count;
		ent->think = target_fountain_think;
		ent->spawnflags |= SF_WEATHER_STARTON;
		ent->think(ent);
	}
}

void target_fountain_delayed_use (edict_t *self)
{
	// Since target_fountain tends to be a processor hog,
	// for START_ON we wait until the player has spawned into the
	// game to ease the startup burden somewhat
	if(g_edicts[1].linkcount)
	{
		self->think = target_fountain_think;
		self->think(self);
	}
	else
		self->nextthink = level.time + FRAMETIME;
}
void SP_target_fountain (edict_t *ent)
{
	char	*buffer;

	if(deathmatch->value || coop->value)
	{
		G_FreeEdict(ent);
		return;
	}

	ent->class_id = ENTITY_TARGET_FOUNTAIN;

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;

	if(ent->spawnflags & SF_WEATHER_STARTON)
	{
		ent->think = target_fountain_delayed_use;
		ent->nextthink = level.time + 1;
	}

	ent->style = STYLE_WEATHER_USER;
	if(!ent->usermodel)
	{
		gi.dprintf("target_fountain with no usermodel.\n");
		G_FreeEdict(ent);
		return;
	}

	// Knightmare- check for "models/" or "sprites/" already in path
	if ( strncmp(ent->usermodel, "models/", 7) && strncmp(ent->usermodel, "sprites/", 8) )
	{
		buffer = gi.TagMalloc(strlen(ent->usermodel)+10,TAG_LEVEL);
		if(strstr(ent->usermodel,".sp2"))
			sprintf(buffer, "sprites/%s", ent->usermodel);
		else
			sprintf(buffer, "models/%s", ent->usermodel);
		ent->usermodel = buffer;
	}

	if(st.gravity)
		ent->gravity = atof(st.gravity);
	else
		ent->gravity = 0.;

	ent->use = target_fountain_use;
	
	if(!ent->count)
		ent->count = 1;

	if(!ent->sounds)
		ent->sounds = 2;	// blue splash

	if(!ent->mass2)
		ent->mass2 = 8;		// 8 particles in splash

	if(ent->speed <= 0)
		ent->speed = 300;

	if((VectorLength(ent->bleft) == 0.) && (VectorLength(ent->tright) == 0.))
	{
		// Default distribution places raindrops vertically for
		// full coverage, to help avoid "lumps"
		VectorSet(ent->bleft,-32, -32, 64);
		VectorSet(ent->tright,32,  32,128);
	}

	ent->density = ent->count;

	gi.linkentity (ent);
}

//
/*=============================================================================

MISC_DEADSOLDIER MODEL PATCH

==============================================================================*/

#define NUM_SKINS		16
#define MAX_SKINNAME	64
#define DEADSOLDIER_MODEL "models/deadbods/dude/tris.md2"

#include "pak.h"

int PatchDeadSoldier (void)
{
	cvar_t		*gamedir;
	char		skins[NUM_SKINS][MAX_SKINNAME];	// skin entries
	char		infilename[MAX_OSPATH];
	char		outfilename[MAX_OSPATH];
	char		tempname[MAX_OSPATH];
	int			j;
//	char		*p;
	FILE		*infile;
	FILE		*outfile;
	dmdl_t		model;				// model header
	byte		*data;				// model data
	int			datasize;			// model data size (bytes)
	int			newoffset;			// model data offset (after skins)

	// get game (moddir) name
	gamedir = gi.cvar("game", "", 0);
	if (!*gamedir->string)
		return 0;	// we're in baseq2

//	Com_sprintf (outfilename, sizeof(outfilename), "%s/%s", gamedir->string,DEADSOLDIER_MODEL);
	Com_sprintf (tempname, sizeof(tempname), DEADSOLDIER_MODEL);
	SavegameDirRelativePath (tempname, outfilename, sizeof(outfilename));
	if (outfile = fopen (outfilename, "rb"))
	{
		// output file already exists, move along
		fclose (outfile);
	//	gi.dprintf ("PatchDeadSoldier: Could not save %s, file already exists\n", outfilename);
		return 0;
	}

	for (j = 0; j < NUM_SKINS; j++)
		memset (skins[j], 0, MAX_SKINNAME);

	Com_sprintf (skins[0],  sizeof(skins[0]), "models/deadbods/dude/dead1.pcx");
	Com_sprintf (skins[1],	sizeof(skins[1]), "players/male/cipher.pcx");
	Com_sprintf (skins[2],	sizeof(skins[2]), "players/male/claymore.pcx");
	Com_sprintf (skins[3],	sizeof(skins[3]), "players/male/flak.pcx");
	Com_sprintf (skins[4],	sizeof(skins[4]), "players/male/grunt.pcx");
	Com_sprintf (skins[5],	sizeof(skins[5]), "players/male/howitzer.pcx");
	Com_sprintf (skins[6],	sizeof(skins[6]), "players/male/major.pcx");
	Com_sprintf (skins[7],	sizeof(skins[7]), "players/male/nightops.pcx");
	Com_sprintf (skins[8],	sizeof(skins[8]), "players/male/pointman.pcx");
	Com_sprintf (skins[9],	sizeof(skins[9]), "players/male/psycho.pcx");
	Com_sprintf (skins[10],	sizeof(skins[10]), "players/male/rampage.pcx");
	Com_sprintf (skins[11], sizeof(skins[11]), "players/male/razor.pcx");
	Com_sprintf (skins[12], sizeof(skins[12]), "players/male/recon.pcx");
	Com_sprintf (skins[13], sizeof(skins[13]), "players/male/scout.pcx");
	Com_sprintf (skins[14], sizeof(skins[14]), "players/male/sniper.pcx");
	Com_sprintf (skins[15], sizeof(skins[15]), "players/male/viper.pcx");


	// load original model
	Com_sprintf (infilename, sizeof(infilename), "baseq2/%s", DEADSOLDIER_MODEL);
	if ( !(infile = fopen (infilename, "rb")) )
	{
		// If file doesn't exist on user's hard disk, it must be in 
		// pak0.pak

		pak_header_t	pakheader;
		pak_item_t		pakitem;
		FILE			*fpak;
		int				k, numitems;

		fpak = fopen("baseq2/pak0.pak","rb");
		if(!fpak)
		{
			cvar_t	*cddir;
			char	pakfile[MAX_OSPATH];

			cddir = gi.cvar("cddir", "", 0);
			Com_sprintf(pakfile, sizeof(pakfile), "%s/baseq2/pak0.pak",cddir->string);
			fpak = fopen(pakfile,"rb");
			if(!fpak)
			{
				gi.dprintf("PatchDeadSoldier: Cannot find pak0.pak\n");
				return 0;
			}
		}
		fread(&pakheader,1,sizeof(pak_header_t),fpak);
		numitems = pakheader.dsize/sizeof(pak_item_t);
		fseek(fpak,pakheader.dstart,SEEK_SET);
		data = NULL;
		for(k=0; k<numitems && !data; k++)
		{
			fread(&pakitem,1,sizeof(pak_item_t),fpak);
			if(!Q_stricmp(pakitem.name,DEADSOLDIER_MODEL))
			{
				fseek(fpak,pakitem.start,SEEK_SET);
				fread(&model, sizeof(dmdl_t), 1, fpak);
				datasize = model.ofs_end - model.ofs_skins;
				if ( !(data = malloc (datasize)) )	// make sure freed locally
				{
					fclose(fpak);
					gi.dprintf ("PatchDeadSoldier: Could not allocate memory for model\n");
					return 0;
				}
				fread (data, sizeof (byte), datasize, fpak);
			}
		}
		fclose(fpak);
		if(!data)
		{
			gi.dprintf("PatchDeadSoldier: Could not find %s in baseq2/pak0.pak\n",DEADSOLDIER_MODEL);
			return 0;
		}
	}
	else
	{
		fread (&model, sizeof (dmdl_t), 1, infile);
	
		datasize = model.ofs_end - model.ofs_skins;
		if ( !(data = malloc (datasize)) )	// make sure freed locally
		{
			gi.dprintf ("PatchMonsterModel: Could not allocate memory for model\n");
			return 0;
		}
		fread (data, sizeof (byte), datasize, infile);
	
		fclose (infile);
	}
	
	// update model info
	model.num_skins = NUM_SKINS;
	
	// Already had 1 skin, so new offset doesn't include that one
//	newoffset = (model.num_skins-1) * MAX_SKINNAME;
	newoffset = model.num_skins * MAX_SKINNAME;
	model.ofs_st     += newoffset;
	model.ofs_tris   += newoffset;
	model.ofs_frames += newoffset;
	model.ofs_glcmds += newoffset;
	model.ofs_end    += newoffset;
	
	// save new model
/*	Com_sprintf (outfilename, sizeof(outfilename), "%s/models", gamedir->string);	// make some dirs if needed
	_mkdir (outfilename);
	Com_strcat (outfilename, sizeof(outfilename), "/deadbods");
	_mkdir (outfilename);
	Com_strcat (outfilename, sizeof(outfilename), "/dude");
	_mkdir (outfilename);
	Com_sprintf (outfilename, sizeof(outfilename), "%s/%s", gamedir->string, DEADSOLDIER_MODEL);
	p = strstr(outfilename,"/tris.md2");
	*p = 0;
	_mkdir (outfilename);
	Com_sprintf (outfilename, sizeof(outfilename), "%s/%s", gamedir->string, DEADSOLDIER_MODEL);
*/	
	Com_sprintf (tempname, sizeof(tempname), DEADSOLDIER_MODEL);
	SavegameDirRelativePath (tempname, outfilename, sizeof(outfilename));
	CreatePath (outfilename);

	if ( !(outfile = fopen (outfilename, "wb")) )
	{
		// file couldn't be created for some other reason
		gi.dprintf ("PatchDeadSoldier: Could not save %s\n", outfilename);
		free (data);
		return 0;
	}
	
	fwrite (&model, sizeof (dmdl_t), 1, outfile);
//	fwrite (skins, sizeof (char), model.num_skins*MAX_SKINNAME, outfile);
	fwrite (skins, sizeof (char), newoffset, outfile);
//	data += MAX_SKINNAME;
	fwrite (data, sizeof (byte), datasize, outfile);
	
	fclose (outfile);
	gi.dprintf ("PatchDeadSoldier: Saved %s\n", outfilename);
	free (data);	// crashes here
	return 1;
}
