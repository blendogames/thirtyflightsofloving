#include "g_local.h"

/*
==============================================================================

SHOCKWAVE

==============================================================================
*/

void ShockQuake (edict_t *self)
{
	int		i;
	edict_t	*ent;

	for (i = 1, ent = g_edicts+1; i < globals.num_edicts; i++, ent++)
	{
		if ( !ent->inuse || !ent->client || !ent->groundentity )
			continue;

		ent->groundentity = NULL;
		ent->velocity[0] += crandom() * 125.0f;
		ent->velocity[1] += crandom() * 125.0f;
		ent->velocity[2] += self->speed * (150.0f / ent->mass);
	}

/*	if (!strcmp(self->classname, "shocksphere")) // remove shock sphere after x bounces
	{
		if (self->count > sk_shockwave_bounces->value)
			G_FreeEdict (self);
		return;  //don't loop
	}*/

	if (level.time < self->timestamp)
		self->nextthink = level.time + FRAMETIME;
	else
		G_FreeEdict (self);
}

void shock_effect_think (edict_t *self)
{
	if (++self->s.frame < 19)
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 0;
		self->nextthink = level.time + FRAMETIME;
	}
	self->count--;

	// fade out
#ifdef KMQUAKE2_ENGINE_MOD
	if (self->count <= 6)
		self->s.alpha -= 0.10;
	if (self->s.alpha < 0.10)
		self->s.alpha = 0.10;
#else
	if (self->count == 5)
	{
		self->s.effects |= EF_SPHERETRANS;
		self->s.renderfx &= ~RF_TRANSLUCENT;
	}
#endif
	// remove after 6 secs
	if (self->count == 0)
		G_FreeEdict (self);
	// inflict field damage on surroundings
	T_RadiusDamage(self, self->owner, self->radius_dmg, NULL, self->dmg_radius, MOD_SHOCK_SPLASH);
}

void shock_effect_center_think (edict_t *self)
{
	self->nextthink = level.time + FRAMETIME;
	if ((self->count % 5) == 0)
	{
		if (self->count > 10) // double effect for first 40 seconds
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_NUKEBLAST);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_ALL);
		}
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_NUKEBLAST);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_ALL);
	}
	// fade out
#ifdef KMQUAKE2_ENGINE_MOD
	if (self->count <= 10)
		self->s.alpha -= 0.10;
	self->s.alpha = max(self->s.alpha, 1/255);
	// remove after 5 secs
	self->count--;
	if (self->count == 0 || self->s.alpha <= 1/255)
		G_FreeEdict (self);
#else
	if (self->count == 10)
		self->s.renderfx |= RF_TRANSLUCENT;
	if (self->count == 5)
	{
		self->s.effects |= EF_SPHERETRANS;
		self->s.renderfx &= ~RF_TRANSLUCENT;
	}
	// remove after 5 secs
	self->count--;
	if (self->count == 0)
		G_FreeEdict (self);
#endif
}

void ShockEffect (edict_t *source, edict_t *attacker, float damage, float radius, cplane_t *plane)
{
	edict_t	*ent;
	edict_t	*center;
	vec3_t	hit_point;

	if (plane->normal)
	{	// put origin of effect 32 units away from last hit surface
		VectorMA (source->s.origin, 32.0, plane->normal, hit_point);
	}

	ent = G_Spawn();
	// same origin as exploding shock sphere
	if (plane->normal)
		VectorCopy (hit_point, ent->s.origin);
	else
		VectorCopy (source->s.origin, ent->s.origin);
	ent->radius_dmg = sk_shockwave_effect_damage->value;
	ent->dmg_radius = sk_shockwave_effect_radius->value;
	ent->owner = attacker;
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	VectorSet (ent->mins, -8, -8, 8);
	VectorSet (ent->maxs, 8, 8, 8);
	ent->s.modelindex = gi.modelindex ("models/objects/shockfield/tris.md2");
#ifdef KMQUAKE2_ENGINE_MOD
	ent->s.alpha = 0.70;
#else
	ent->s.renderfx |= RF_TRANSLUCENT;
#endif
	ent->s.renderfx |= RF_NOSHADOW|RF_FULLBRIGHT;
	ent->s.effects = EF_FLAG2;
	ent->count = 60;  // lasts 6 seconds
	ent->think = shock_effect_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (ent);

	// center light burst effect
	center = G_Spawn();
	// same origin as exploding shock sphere
	if (plane->normal)
		VectorCopy (hit_point, center->s.origin);
	else
		VectorCopy (source->s.origin, center->s.origin);
	center->movetype = MOVETYPE_NONE;
	center->solid = SOLID_NOT;
	VectorSet (center->mins, -8, -8, 8);
	VectorSet (center->maxs, 8, 8, 8);
	center->s.modelindex = gi.modelindex ("sprites/s_trap.sp2");
	center->s.effects |= EF_FLAG2 | EF_ANIM_ALLFAST;
#ifdef KMQUAKE2_ENGINE_MOD
	center->s.alpha = 0.90;
#endif
	center->count = 50;  // lasts 5 seconds
	center->think = shock_effect_center_think;
	center->nextthink = level.time + 2 * FRAMETIME;
	ent->teamchain = center;
	gi.linkentity (center);
}

#ifndef KMQUAKE2_ENGINE_MOD
void ShockSplashThink (edict_t *self)
{
	self->s.frame++;
	if (self->s.frame > 8)
	{
		G_FreeEdict(self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
}
#else
// Gotta have this for extractfuncs...
void ShockSplashThink (edict_t *self)
{
}
#endif

void ShockSplash(edict_t *self, cplane_t *plane)
{
#ifdef KMQUAKE2_ENGINE_MOD	// use new client-side effect
	vec3_t shockdir;

	if (!plane->normal)
		AngleVectors(self->s.angles, shockdir, NULL, NULL);
	else
		VectorCopy (plane->normal, shockdir);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SHOCKSPLASH);
	gi.WritePosition (self->s.origin);
	gi.WriteDir (shockdir);
	gi.multicast (self->s.origin, MULTICAST_ALL);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectSparks (TE_SHOCKSPLASH, self->s.origin, shockdir);

#else
	edict_t	*shockring;
	int i;

	gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/shockhit.wav"), 1, ATTN_NONE, 0);

	for (i = 0; i < 5; i++)
	{
		shockring = G_Spawn();
		shockring->classname = "shock_ring";
		if (plane->normal)
		{	// put origin of impact effect 16*i+1 units away from last hit surface
			VectorMA (self->s.origin, 16.0*(i+1), plane->normal, shockring->s.origin);
			vectoangles(plane->normal, shockring->s.angles);
		}
		else
		{
			VectorCopy (self->s.origin, shockring->s.origin);
			VectorCopy (self->s.angles, shockring->s.angles);
		}
		shockring->solid = SOLID_NOT;
		shockring->movetype = MOVETYPE_NONE;
		shockring->owner = self;
		shockring->s.modelindex = gi.modelindex("models/objects/shocksplash/tris.md2");
		shockring->s.frame = (4-i);
		shockring->s.effects |= EF_SPHERETRANS;
		shockring->s.renderfx |= (RF_FULLBRIGHT | RF_NOSHADOW);
		shockring->nextthink = level.time + FRAMETIME;
		shockring->think = ShockSplashThink;
		gi.linkentity (shockring);
	}
#endif
}

void shock_sphere_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	origin;
	edict_t	*impact;

	// ignore other projectiles
	if ((other->movetype == MOVETYPE_FLYMISSILE) || (other->movetype == MOVETYPE_BOUNCE)
		|| (other->movetype == MOVETYPE_WALLBOUNCE))
		return;

	ent->count++; // add to count
	ent->movetype = MOVETYPE_BOUNCE;

	if (other == ent->owner)
		return;

	// detonate if hit a monster
	if (other->takedamage)
		ent->count = sk_shockwave_bounces->value + 1;

	if ( (ent->velocity[0] < 20) && (ent->velocity[0] > -20)
	   && (ent->velocity[1] < 20) && (ent->velocity[1] > -20)
	   && (ent->velocity[2] < 20) && (ent->velocity[2] > -20) )
		ent->count = sk_shockwave_bounces->value + 1;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_SHOCK_SPHERE);
	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_SHOCK_SPLASH);

	// spawn impact
	impact = G_Spawn();
	impact->classname = "shock_impact";
	VectorCopy (ent->s.origin, impact->s.origin);
	gi.linkentity (impact);
	impact->think = ShockQuake;
	impact->speed = 250;
	impact->nextthink = level.time + FRAMETIME;

	if ((ent->count <= sk_shockwave_bounces->value) && !other->takedamage) //no shock rings if hit a monster
	{
		ShockSplash (ent, plane); // Spawn shock rings

		if (impact)
			impact->timestamp = level.time + 3;
	}
	else // don't exploode until final hit or hit a monster
	{ 
		gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/shockexp.wav"), 1, ATTN_NONE, 0);

		ShockEffect (ent, ent->owner, ent->radius_dmg, ent->dmg_radius, plane);

		if (impact)
			impact->timestamp = level.time + 6;

		// remove after x bounces
		G_FreeEdict (ent);
	}
}

void shock_sphere_think (edict_t *sphere)
{
	sphere->avelocity[PITCH] = 80;
	sphere->avelocity[ROLL] = 80;
	sphere->nextthink = level.time + FRAMETIME;
}

void fire_shock_sphere (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*sphere;

	sphere = G_Spawn();
	sphere->classname = "shocksphere";
	sphere->class_id = ENTITY_SHOCK_SPHERE;
	VectorCopy (start, sphere->s.origin);
	VectorCopy (dir, sphere->movedir);
	vectoangles (dir, sphere->s.angles);
	VectorScale (dir, speed, sphere->velocity);
	sphere->avelocity[PITCH] = 80;
	sphere->avelocity[ROLL] = 80;
	sphere->movetype = MOVETYPE_WALLBOUNCE;
	sphere->count = 0;
	sphere->clipmask = MASK_SHOT;
	sphere->solid = SOLID_BBOX;
	VectorClear (sphere->mins);
	VectorClear (sphere->maxs);
	VectorSet (sphere->mins, -14, -14, -14);
	VectorSet (sphere->maxs, 14, 14, 14);
	sphere->s.modelindex = gi.modelindex ("models/objects/shocksphere/tris.md2");
	sphere->s.renderfx |= RF_IR_VISIBLE;
	sphere->owner = self;
	sphere->touch = shock_sphere_touch;
	sphere->timestamp = level.time;
	sphere->nextthink = level.time + 0.1;
	sphere->think = shock_sphere_think;
	sphere->dmg = damage;
	sphere->radius_dmg = radius_damage;
	sphere->dmg_radius = damage_radius;

	if (self->client)
		check_dodge (self, sphere->s.origin, dir, speed);
	gi.linkentity (sphere);
}

// NOTE: SP_shocksphere should ONLY be used to spawn shockspheres that change maps
//       via a trigger_transition. It should NOT be used for map entities.

void shocksphere_delayed_start (edict_t *shocksphere)
{
	if (g_edicts[1].linkcount)
	{
		VectorScale(shocksphere->movedir,shocksphere->moveinfo.speed,shocksphere->velocity);
		if (shocksphere->count > 0)
			shocksphere->movetype  = MOVETYPE_BOUNCE;
		else
			shocksphere->movetype  = MOVETYPE_WALLBOUNCE;
		shocksphere->nextthink = level.time + FRAMETIME;
		shocksphere->think     = shock_sphere_think;
		gi.linkentity(shocksphere);
	}
	else
		shocksphere->nextthink = level.time + FRAMETIME;
}

void SP_shocksphere (edict_t *shocksphere)
{
	shocksphere->s.modelindex = gi.modelindex ("models/objects/shocksphere/tris.md2");
	shocksphere->touch = shock_sphere_touch;

	// For SP, freeze shocksphere until player spawns in
	if (game.maxclients == 1)
	{
		shocksphere->movetype  = MOVETYPE_NONE;
		VectorCopy(shocksphere->velocity,shocksphere->movedir);
		VectorNormalize(shocksphere->movedir);
		shocksphere->moveinfo.speed = VectorLength(shocksphere->velocity);
		VectorClear(shocksphere->velocity);
		shocksphere->think     = shocksphere_delayed_start;
		shocksphere->nextthink = level.time + FRAMETIME;
	}
	else
	{
		if (shocksphere->count > 0)
			shocksphere->movetype  = MOVETYPE_BOUNCE;
		else
			shocksphere->movetype  = MOVETYPE_WALLBOUNCE;
		shocksphere->nextthink = level.time + FRAMETIME;
		shocksphere->think     = shock_sphere_think;
	}
	gi.linkentity (shocksphere);
}


/*
==============================================================================

BLU-86 (aka NEUTRON BOMB)

==============================================================================
*/
#define NBOMB_DELAY			4
#define NBOMB_TIME_TO_LIVE	6
//#define NBOMB_TIME_TO_LIVE	40
#define NBOMB_RADIUS			256
#define NBOMB_DAMAGE			5000
#define NBOMB_QUAKE_TIME		3
#define NBOMB_QUAKE_STRENGTH	100

void Nbomb_Quake (edict_t *self)
{
	int		i;
	edict_t	*e;

	if (self->last_move_time < level.time)
	{
		gi.positioned_sound (self->s.origin, self, CHAN_AUTO, self->noise_index, 0.75, ATTN_NONE, 0);
		self->last_move_time = level.time + 0.5;
	}

	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		if (!e->groundentity)
			continue;

		e->groundentity = NULL;
		e->velocity[0] += crandom()* 150.0f;
		e->velocity[1] += crandom()* 150.0f;
		e->velocity[2] = self->speed * (100.0f / e->mass);
	}

	if (level.time < self->timestamp)
		self->nextthink = level.time + FRAMETIME;
	else
		G_FreeEdict (self);
}


void Nbomb_Explode (edict_t *ent)
{
	if (ent->teammaster->client)
		PlayerNoise(ent->teammaster, ent->s.origin, PNOISE_IMPACT);

	T_RadiusNukeDamage(ent, ent->teammaster, ent->dmg, ent, ent->dmg_radius, MOD_NBOMB);

	if (ent->dmg > sk_nbomb_damage->value)
	{
		if (ent->dmg < (4 * sk_nbomb_damage->value)) // double sound
				gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/ddamage3.wav"), 1, ATTN_NORM, 0);
		else // quad sound
				gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
	}

	gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/grenlx1a.wav"), 1, ATTN_NONE, 0);


	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1_BIG);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_NUKEBLAST);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_ALL);

	// Lazarus reflections
	if (level.num_reflectors)
	{
		ReflectExplosion (TE_EXPLOSION1_BIG, ent->s.origin);
	//	ReflectExplosion (TE_NUKEBLAST, ent->s.origin);
	}

	// become a quake
	ent->svflags |= SVF_NOCLIENT;
	ent->noise_index = gi.soundindex ("world/rumble.wav");
	ent->think = Nbomb_Quake;
	ent->speed = NBOMB_QUAKE_STRENGTH;
	ent->timestamp = level.time + NBOMB_QUAKE_TIME;
	ent->nextthink = level.time + FRAMETIME;
	ent->last_move_time = 0;
}

void nbomb_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	if ( (attacker) && !(strcmp(attacker->classname, "nbomb")) )
	{
	//	if ((g_showlogic) && (g_showlogic->value))
	//		gi.dprintf ("nbomb nuked by a nbomb, not nuking\n");
		G_FreeEdict (self);
		return;
	}
	Nbomb_Explode(self);
}

void Nbomb_Think (edict_t *ent)
{
	float attenuation, default_atten = 1.8;
	int	muzzleflash;

	attenuation = default_atten/3.0;
	muzzleflash = MZ_NUKE4;

	if (ent->wait < level.time)
		Nbomb_Explode(ent);
	else if (level.time >= (ent->wait - sk_nbomb_life->value))
	{
		if (gi.pointcontents (ent->s.origin) & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			Nbomb_Explode (ent);
			return;
		}

		// Knightmare- remove smoke trail if we've stopped moving
		if ( ent->groundentity && !VectorLength(ent->velocity) )
			ent->s.effects &= ~EF_GRENADE;
		// but restore it if we go flying
		else if ( !ent->groundentity )
			ent->s.effects |= EF_GRENADE;

		ent->think = Nbomb_Think;
		ent->nextthink = level.time + 0.1;
		ent->health = 1;
		ent->owner = NULL;

		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (muzzleflash);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		if (ent->timestamp <= level.time)
		{
	/*		gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/nukewarn.wav"), 1, ATTN_NORM, 0);
			ent->timestamp += 10.0;
		}
	*/

			if ((ent->wait - level.time) <= (sk_nbomb_life->value / 2.0))
			{
			//	gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/nukewarn2.wav"), 1, ATTN_NORM, 0);
				gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/nukewarn2.wav"), 1, attenuation, 0);
			//	gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/nukewarn2.wav"), 1, ATTN_NORM, 0);
				ent->timestamp = level.time + 0.3;
			}
			else
			{
				gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/nukewarn2.wav"), 1, attenuation, 0);
			//	gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/nukewarn2.wav"), 1, ATTN_NORM, 0);
				ent->timestamp = level.time + 0.5;
			}
		}
	}
	else
	{
		if (ent->timestamp <= level.time)
		{
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/nukewarn2.wav"), 1, attenuation, 0);
		//	gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/nukewarn2.wav"), 1, ATTN_NORM, 0);
		//		gi.dprintf ("time %2.2f\n", ent->wait-level.time);
			ent->timestamp = level.time + 1.0;
		}
		ent->nextthink = level.time + FRAMETIME;
	}
}

void nbomb_bounce (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (random() > 0.5)
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
}


extern byte P_DamageModifier(edict_t *ent);

void fire_nbomb (edict_t *self, vec3_t start, vec3_t aimdir, int speed)
{
	edict_t	*nbomb;
	vec3_t	dir;
	vec3_t	forward, right, up;
	int		damage_modifier;

	damage_modifier = (int)P_DamageModifier (self);

	vectoangles2 (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	nbomb = G_Spawn();
	VectorCopy (start, nbomb->s.origin);
	VectorScale (aimdir, speed, nbomb->velocity);

	VectorMA (nbomb->velocity, 200 + crandom() * 10.0, up, nbomb->velocity);
	VectorMA (nbomb->velocity, crandom() * 10.0, right, nbomb->velocity);
	// Knightmare- add player's base velocity to nbomb
	if (self->groundentity)
		VectorAdd (nbomb->velocity, self->groundentity->velocity, nbomb->velocity);

	VectorClear (nbomb->avelocity);
	VectorClear (nbomb->s.angles);
	nbomb->movetype = MOVETYPE_BOUNCE;
	nbomb->clipmask = MASK_SHOT;
	nbomb->solid = SOLID_BBOX;
	nbomb->s.effects |= EF_GRENADE;
	nbomb->s.renderfx |= RF_IR_VISIBLE;
	VectorSet (nbomb->mins, -8, -8, -16);
	VectorSet (nbomb->maxs, 8, 8, 14);
	nbomb->s.modelindex = gi.modelindex ("models/items/ammo/nbomb/tris.md2");
	nbomb->owner = self;
	nbomb->teammaster = self;
	nbomb->wait = level.time + sk_nbomb_delay->value + sk_nbomb_life->value;
	nbomb->nextthink = level.time + FRAMETIME;
	nbomb->think = Nbomb_Think;
	nbomb->touch = nbomb_bounce;

	nbomb->health = 10000;
	nbomb->takedamage = DAMAGE_YES;
	nbomb->svflags |= SVF_DAMAGEABLE;
	nbomb->dmg = sk_nbomb_damage->value * damage_modifier;
	if (damage_modifier == 1)
		nbomb->dmg_radius = sk_nbomb_radius->value;
	else
		nbomb->dmg_radius = sk_nbomb_radius->value + sk_nbomb_radius->value * (0.25*(float)damage_modifier);
	// this yields 1.0, 1.5, 2.0, 3.0 times radius

//	if ((g_showlogic) && (g_showlogic->value))
//		gi.dprintf ("nbomb modifier = %d, damage = %d, radius = %f\n", damage_modifier, nbomb->dmg, nbomb->dmg_radius);

	nbomb->classname = "nbomb";
	nbomb->class_id = ENTITY_NBOMB;
	nbomb->die = nbomb_die;

	gi.linkentity (nbomb);
}
