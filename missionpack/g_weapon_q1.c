// g_weapon_q1.c

#include "g_local.h"

void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed);

/*
=================
q1_fire_nail

Fires a Nail, Used by Q1 NG and SNG
=================
*/
void q1_nail_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		if (self->spawnflags & 1)
			mod = MOD_Q1_SNG;
		else
			mod = MOD_Q1_NG;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 0, DAMAGE_BULLET, mod);
	}
	else
	{
#ifdef KMQUAKE2_ENGINE_MOD
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SPIKEIMPACT_Q1);
		gi.WritePosition (self->s.origin);
		gi.WriteDir ((!plane) ? vec3_origin : plane->normal);
		if (self->spawnflags == 1)
			gi.WriteByte (1);	// subtype 1 specifies superspike impact
		else
			gi.WriteByte (0);	// subtype 0 specifies spike impact
		gi.multicast (self->s.origin, MULTICAST_PVS);	

#else	// KMQUAKE2_ENGINE_MOD

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SHOTGUN);
		gi.WritePosition (self->s.origin);
		gi.WriteDir ((!plane) ? vec3_origin : plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);	

		// not if online, too laggy
		if (!deathmatch->value)
		{
			float sound = random();
			if (sound < 0.3)
			{
				if (sound < 0.1)
					gi.sound (self, CHAN_WEAPON, gi.soundindex ("q1weapons/ric3.wav"), 1, ATTN_STATIC, 0);
				else if (sound < 0.2)
					gi.sound (self, CHAN_WEAPON, gi.soundindex ("q1weapons/ric2.wav"), 1, ATTN_STATIC, 0);
				else 
					gi.sound (self, CHAN_WEAPON, gi.soundindex ("q1weapons/ric1.wav"), 1, ATTN_STATIC, 0);
			}
			else if (sound < 0.5)
				gi.sound (self, CHAN_WEAPON, gi.soundindex ("q1weapons/tink1.wav"), 1, ATTN_STATIC, 0);
		}
#endif	// KMQUAKE2_ENGINE_MOD
	}
	G_FreeEdict (self);
}


void q1_fire_nail (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, qboolean sng)
{
	edict_t	*nail;
	trace_t	tr;

	VectorNormalize (dir);

	nail = G_Spawn();
	VectorCopy (start, nail->s.origin);
	VectorCopy (start, nail->s.old_origin);
	vectoangles (dir, nail->s.angles);
	VectorScale (dir, speed, nail->velocity);
	nail->movetype = MOVETYPE_FLYMISSILE;
	nail->clipmask = MASK_SHOT;
	nail->solid = SOLID_BBOX;
	nail->svflags = SVF_DEADMONSTER;
	nail->s.renderfx = RF_FULLBRIGHT;
	VectorClear (nail->mins);
	VectorClear (nail->maxs);
	nail->s.modelindex = gi.modelindex ("models/objects/q1nail/tris.md2");
	nail->owner = self;
	nail->touch = q1_nail_touch;
	nail->nextthink = level.time + 8000/speed; 
	nail->think = G_FreeEdict;
	nail->dmg = damage;
	nail->classname = "q1_nail";
	if (sng)
		nail->spawnflags = 1;

	nail->common_name = "Nail";
	nail->class_id = ENTITY_Q1_NAIL;

	gi.linkentity (nail);

	if (self->client)
		check_dodge (self, nail->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, nail->s.origin, nail, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (nail->s.origin, -10, dir, nail->s.origin);
		nail->touch (nail, tr.ent, NULL, NULL);
	}
}


void q1_nail_precache (void)
{
	gi.modelindex ("models/objects/q1nail/tris.md2");
#ifndef KMQUAKE2_ENGINE_MOD
	gi.soundindex ("q1weapons/tink1.wav");
	gi.soundindex ("q1weapons/ric1.wav");
	gi.soundindex ("q1weapons/ric2.wav");
	gi.soundindex ("q1weapons/ric3.wav");
#endif	// KMQUAKE2_ENGINE_MOD
}

// NOTE: SP_q1_nail should ONLY be used to spawn nails that change
//       maps via trigger_transition. It should NOT be used for map entities.
void q1_nail_delayed_start (edict_t *nail)
{
//	if (g_edicts[1].linkcount)
	if ( AnyPlayerSpawned() )	// Knightmare- function handles multiple players
	{
		VectorScale (nail->movedir, nail->moveinfo.speed, nail->velocity);
		nail->nextthink = level.time + 2;
		nail->think = G_FreeEdict;
		gi.linkentity (nail);
	}
	else
		nail->nextthink = level.time + FRAMETIME;
}

void SP_q1_nail (edict_t *nail)
{
	nail->s.skinnum = 0;
	nail->s.modelindex = gi.modelindex ("models/objects/q1nail/tris.md2");
	nail->touch = q1_nail_touch;
	VectorCopy (nail->velocity, nail->movedir);
	VectorNormalize (nail->movedir);
	nail->moveinfo.speed = VectorLength(nail->velocity);
	VectorClear (nail->velocity);
	nail->think = q1_nail_delayed_start;
	nail->nextthink = level.time + FRAMETIME;
	gi.linkentity (nail);
}


/*
=================
q1_fire_laser

Fires a laser bolt, used by Q1 enforcer and laser trap.
=================
*/
void q1_laser_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage) {
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 0, DAMAGE_ENERGY, MOD_Q1_LASER);
	}
	else
	{
#ifdef KMQUAKE2_ENGINE_MOD
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SPIKEIMPACT_Q1);
		gi.WritePosition (self->s.origin);
		gi.WriteDir ((!plane) ? vec3_origin : plane->normal);
		gi.WriteByte (2);	// subtype 2 specifies enforcer laser impact
		gi.multicast (self->s.origin, MULTICAST_PVS);

#else	// KMQUAKE2_ENGINE_MOD

	/*	gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SHOTGUN);
		gi.WritePosition (self->s.origin);
		gi.WriteDir ((!plane) ? vec3_origin : plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS); */

		// From Decino's Q2Infighter mod
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_WELDING_SPARKS);
		gi.WriteByte (15);
		gi.WritePosition (self->s.origin);
		gi.WriteDir ((!plane) ? vec3_origin : plane->normal);
		gi.WriteByte (226);
		gi.multicast (self->s.origin, MULTICAST_PVS);	

		// not if online, too laggy
		if (!deathmatch->value)
		{
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("q1enforcer/enfstop.wav"), 1, ATTN_NORM, 0);	// was ATTN_STATIC
		}
#endif	// KMQUAKE2_ENGINE_MOD
	}
	G_FreeEdict (self);
}

void q1_fire_laser (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*laser;
	trace_t	tr;

	VectorNormalize (dir);

	laser = G_Spawn();
	VectorCopy (start, laser->s.origin);
	VectorCopy (start, laser->s.old_origin);
	vectoangles (dir, laser->s.angles);
	VectorScale (dir, speed, laser->velocity);
	laser->movetype = MOVETYPE_FLYMISSILE;
	laser->clipmask = MASK_SHOT;
	laser->solid = SOLID_BBOX;
	laser->svflags = SVF_DEADMONSTER;
	laser->s.renderfx = RF_FULLBRIGHT;
	VectorClear (laser->mins);
	VectorClear (laser->maxs);
	laser->s.modelindex = gi.modelindex ("models/monsters/q1enforcer/laser/tris.md2");
	laser->owner = self;
	laser->touch = q1_laser_touch;
	laser->nextthink = level.time + 8000/speed; 
	laser->think = G_FreeEdict;
	laser->dmg = damage;
	laser->classname = "q1_laser";

	laser->common_name = "Laser";
	laser->class_id = ENTITY_Q1_LASER;

	gi.linkentity (laser);

	if (self->client)
		check_dodge (self, laser->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, laser->s.origin, laser, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (laser->s.origin, -10, dir, laser->s.origin);
		laser->touch (laser, tr.ent, NULL, NULL);
	}
}

void q1_laser_precache (void)
{
	gi.modelindex ("models/monsters/q1enforcer/laser/tris.md2");
#ifndef KMQUAKE2_ENGINE_MOD
	gi.soundindex ("q1enforcer/enfstop.wav");
#endif	// KMQUAKE2_ENGINE_MOD
}


// NOTE: SP_q1_laser should ONLY be used to spawn lasers that change
//       maps via trigger_transition. It should NOT be used for map entities.
void q1_laser_delayed_start (edict_t *laser)
{
//	if (g_edicts[1].linkcount)
	if ( AnyPlayerSpawned() )	// Knightmare- function handles multiple players
	{
		VectorScale (laser->movedir, laser->moveinfo.speed, laser->velocity);
		laser->nextthink = level.time + 2;
		laser->think = G_FreeEdict;
		gi.linkentity (laser);
	}
	else
		laser->nextthink = level.time + FRAMETIME;
}

void SP_q1_laser (edict_t *laser)
{
	laser->s.skinnum = 0;
	laser->s.modelindex = gi.modelindex ("models/monsters/q1enforcer/laser/tris.md2");
	laser->touch = q1_laser_touch;
	VectorCopy (laser->velocity, laser->movedir);
	VectorNormalize (laser->movedir);
	laser->moveinfo.speed = VectorLength(laser->velocity);
	VectorClear (laser->velocity);
	laser->think = q1_laser_delayed_start;
	laser->nextthink = level.time + FRAMETIME;
	gi.linkentity (laser);
}


/*
=================
q1_fire_flame

Fires a flame bolt.  Used by the Hell Knight.
=================
*/

void q1_flame_splash (int type, int count, int color, vec3_t start, vec3_t movdir, vec3_t origin) 
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (type);
	gi.WriteByte (count);
	gi.WritePosition (start);
	gi.WriteDir (movdir);
	gi.WriteByte (color);
	gi.multicast (origin, MULTICAST_PVS);
}

void q1_flame_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_Q1_FLAMEBOLT);
	}
	else
	{
#ifdef KMQUAKE2_ENGINE_MOD
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SPIKEIMPACT_Q1);
		gi.WritePosition (self->s.origin);
		gi.WriteDir ((!plane) ? vec3_origin : plane->normal);
		gi.WriteByte (4);	// subtype 4 specifies hell knight flame impact
		gi.multicast (self->s.origin, MULTICAST_PVS);

#else	// KMQUAKE2_ENGINE_MOD

	//	q1_flame_splash (TE_TUNNEL_SPARKS, 16, 71, self->s.origin, plane->normal, self->s.origin);

		// From Decino's Q2Infighter mod
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_WELDING_SPARKS);
		gi.WriteByte (15);
		gi.WritePosition (self->s.origin);
		gi.WriteDir ((!plane) ? vec3_origin : plane->normal);
	//	gi.WriteByte (66);
		gi.WriteByte (228);
		gi.multicast (self->s.origin, MULTICAST_PVS);

		gi.sound (self, CHAN_WEAPON, gi.soundindex("q1hknight/hit.wav"), 1, ATTN_NORM, 0);
#endif	// KMQUAKE2_ENGINE_MOD
	}

//	gi.sound (self, CHAN_WEAPON, gi.soundindex("q1hknight/hit.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict (self);
}


void q1_fire_flame (edict_t *self, vec3_t start, vec3_t dir, float leftrightoff)
{
	edict_t	*bolt;
	trace_t	tr;
	int damage, speed;
//	vec3_t up;

	damage = 9;
	speed = 300;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags |= SVF_DEADMONSTER;
	// yes, I know it looks weird that projectiles are deadmonsters
	// what this means is that when prediction is used against the object
	// (blaster/hyperblaster shots), the player won't be solid clipped against
	// the object.  Right now trying to run into a firing hyperblaster
	// is very jerky since you are predicted 'against' the shots.
	
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
#ifdef KMQUAKE2_ENGINE_MOD
	bolt->s.effects |= (EF_IONRIPPER|EF_TRACKER);
#else	// KMQUAKE2_ENGINE_MOD
	bolt->s.effects |= EF_IONRIPPER;
#endif	// KMQUAKE2_ENGINE_MOD
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/monsters/q1hknight/k_spike/tris.md2");
	bolt->owner = self;
	bolt->touch = q1_flame_touch;
	bolt->nextthink = level.time + 6;	// Knightmare- was 2
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "q1_flame";

	bolt->common_name = "Hell Knight Flame";
	bolt->class_id = ENTITY_Q1_HKNIGHT_FLAME;

	gi.linkentity (bolt);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}


void q1_flame_precache (void)
{
	gi.modelindex ("models/monsters/q1hknight/k_spike/tris.md2");
#ifndef KMQUAKE2_ENGINE_MOD
	gi.soundindex ("q1hknight/hit.wav");
#endif	// KMQUAKE2_ENGINE_MOD
}


// NOTE: SP_q1_flame should ONLY be used to spawn flames that change
//       maps via trigger_transition. It should NOT be used for map entities.
void q1_flame_delayed_start (edict_t *bolt)
{
//	if (g_edicts[1].linkcount)
	if ( AnyPlayerSpawned() )	// Knightmare- function handles multiple players
	{
		VectorScale (bolt->movedir, bolt->moveinfo.speed, bolt->velocity);
		bolt->nextthink = level.time + 2;
		bolt->think = G_FreeEdict;
		gi.linkentity (bolt);
	}
	else
		bolt->nextthink = level.time + FRAMETIME;
}

void SP_q1_flame (edict_t *bolt)
{
	bolt->s.skinnum = 0;
	bolt->s.modelindex = gi.modelindex ("models/monsters/q1hknight/k_spike/tris.md2");
	bolt->touch = q1_flame_touch;
	VectorCopy (bolt->velocity, bolt->movedir);
	VectorNormalize (bolt->movedir);
	bolt->moveinfo.speed = VectorLength(bolt->velocity);
	VectorClear (bolt->velocity);
	bolt->think = q1_flame_delayed_start;
	bolt->nextthink = level.time + FRAMETIME;
	gi.linkentity (bolt);
}


/*
=================
q1_fire_grenade

Fires a grenade.  Used by the Ogre.
=================
*/
void q1_explode (edict_t *self)
{
	if (self->s.frame == 5)
	{
		G_FreeEdict (self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
}

void q1_grenade_explode (edict_t *ent)
{
	vec3_t		origin;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		int		points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - (int)(0.5 * VectorLength(v));
		points = max(0, points);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, points, points, DAMAGE_RADIUS, MOD_Q1_GL);
	}

	T_RadiusDamage (ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, MOD_Q1_GL_SPLASH);
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

#ifdef KMQUAKE2_ENGINE_MOD
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION_Q1);
	gi.WritePosition (origin);
	gi.WriteByte (0);	// subtype 0 specifies default explosion
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	G_FreeEdict (ent);
#else	// KMQUAKE2_ENGINE_MOD
	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("q1weapons/r_exp3.wav"), 1.0, ATTN_NORM, 0);	

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	// explosion sprite
	gi.unlinkentity (ent);
	ent->solid = SOLID_NOT;
	ent->touch = NULL;
	VectorCopy (origin, ent->s.origin);
	VectorCopy (origin, ent->s.old_origin);
	VectorClear (ent->velocity);
	ent->s.modelindex = gi.modelindex ("sprites/s_explod.sp2"); 
	ent->s.frame = 0; 
	ent->s.sound = 0;
	ent->s.effects &= ~EF_ANIM_ALLFAST; 
	ent->think = q1_explode; 
	ent->nextthink = level.time + FRAMETIME;
	gi.linkentity (ent);
#endif	// KMQUAKE2_ENGINE_MOD
}


void q1_grenade_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;
	
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}
	
	if (!other->takedamage || 
		(other->solid == SOLID_BSP))
	{
		gi.sound (ent, CHAN_RELIABLE|CHAN_WEAPON, gi.soundindex ("q1weapons/bounce.wav"), 1, ATTN_NORM, 0);
		return;
	}

	ent->enemy = other;
	q1_grenade_explode (ent);
}


void q1_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	// Lazarus - keep same vertical boost for players, but monsters do a better job
	//           of calculating aim direction, so throw that out
//	if (self->client)
		VectorMA (grenade->velocity, 200 + (random() - 0.5) * 20.0, up, grenade->velocity);
//	else
//		VectorMA (grenade->velocity, (random() - 0.5) * 20.0, up, grenade->velocity);

	VectorMA (grenade->velocity, (random() - 0.5) * 20.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	grenade->s.renderfx |= RF_IR_VISIBLE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/q1grenade/tris.md2"); 
	grenade->owner = self;
	grenade->touch = q1_grenade_touch;
	grenade->nextthink = level.time + timer;
	grenade->think = q1_grenade_explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "q1_grenade";

	grenade->common_name = "Quake Grenade";
	grenade->class_id = ENTITY_Q1_GRENADE;

//	Grenade_Add_To_Chain (grenade);

	gi.linkentity (grenade);
}

void q1_grenade_precache (void)
{
	gi.modelindex ("models/objects/q1grenade/tris.md2");
	gi.soundindex ("q1weapons/bounce.wav");
#ifndef KMQUAKE2_ENGINE_MOD
	gi.modelindex ("sprites/s_explod.sp2");
	gi.soundindex ("q1weapons/r_exp3.wav");
#endif	// KMQUAKE2_ENGINE_MOD
}


// NOTE: SP_q1_grenade should ONLY be used to spawn grenades that change
//       maps via trigger_transition. It should NOT be used for map entities.
void q1_grenade_delayed_start (edict_t *grenade)
{
//	if (g_edicts[1].linkcount)
	if ( AnyPlayerSpawned() )	// Knightmare- function handles multiple players
	{
		VectorScale (grenade->movedir, grenade->moveinfo.speed, grenade->velocity);
		grenade->nextthink = level.time + 2;
		grenade->think = G_FreeEdict;
		gi.linkentity (grenade);
	}
	else
		grenade->nextthink = level.time + FRAMETIME;
}

void SP_q1_grenade (edict_t *grenade)
{
	grenade->s.skinnum = 0;
	grenade->s.modelindex = gi.modelindex ("models/objects/q1grenade/tris.md2");
	grenade->touch = q1_grenade_touch;

	// For SP, freeze grenade until player spawns in
	if (game.maxclients == 1)
	{
		grenade->movetype  = MOVETYPE_NONE;
		VectorCopy (grenade->velocity, grenade->movedir);
		VectorNormalize (grenade->movedir);
		grenade->moveinfo.speed = VectorLength(grenade->velocity);
		VectorClear (grenade->velocity);
		grenade->think = q1_grenade_delayed_start;
		grenade->nextthink = level.time + FRAMETIME;
	}
	else
	{
		grenade->movetype  = MOVETYPE_BOUNCE;
		grenade->nextthink = level.time + 2.5;
		grenade->think     = q1_grenade_explode;
	}
	gi.linkentity (grenade);
}


/*
=================
q1_fire_rocket

Fires a q1 rocket.  Used by ?
=================
*/
void q1_rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

	if (other == ent->owner)
		return;

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
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_Q1_RL);
	}
	T_RadiusDamage (ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_Q1_RL_SPLASH);

#ifdef KMQUAKE2_ENGINE_MOD
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION_Q1);
	gi.WritePosition (origin);
	gi.WriteByte (0);	// subtype 0 specifies default explosion
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	G_FreeEdict (ent);
#else	// KMQUAKE2_ENGINE_MOD
	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("q1weapons/r_exp3.wav"), 1.0, ATTN_NORM, 0);	

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION); 
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

//	if (ent->target_ent)
//		G_FreeEdict (ent->target_ent);

	// explosion sprite
	gi.unlinkentity (ent);
	ent->solid = SOLID_NOT;
	ent->touch = NULL;
	VectorCopy (origin, ent->s.origin);
	VectorCopy (origin, ent->s.old_origin);
	VectorClear (ent->velocity);
	ent->s.modelindex = gi.modelindex ("sprites/s_explod.sp2"); 
	ent->s.frame = 0; 
	ent->s.sound = 0;
	ent->s.effects &= ~EF_ANIM_ALLFAST; 
	ent->think = q1_explode; 
	ent->nextthink = level.time + FRAMETIME;
	gi.linkentity (ent);
#endif	// KMQUAKE2_ENGINE_MOD
}


void q1rocketTrail_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	G_FreeEdict (ent);
}

// Rocket Trails 
// needed to give the yellow part of the Rocket trail

void q1rocket_trail (edict_t *self, vec3_t start, vec3_t dir)
{
	edict_t	*bolt;
	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, 950, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects = EF_BLASTER;
	bolt->svflags |= SVF_DEADMONSTER;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->owner = self;
	bolt->touch = q1rocketTrail_touch;
	bolt->nextthink = level.time + 8000/950;
	bolt->think = G_FreeEdict;
	bolt->classname = "bolt";
	gi.linkentity (bolt);

}

	
void q1_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;
	
	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_GRENADE;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/q1rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = q1_rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "q1_rocket";
	
	q1rocket_trail (rocket, start, dir);
	
	rocket->common_name = "Quake Rocket";
	rocket->class_id = ENTITY_Q1_ROCKET;

	gi.linkentity (rocket);
}


void q1_rocket_precache (void)
{
	gi.modelindex ("models/objects/q1rocket/tris.md2");
	gi.soundindex ("weapons/rockfly.wav");
#ifndef KMQUAKE2_ENGINE_MOD
	gi.modelindex ("sprites/s_explod.sp2");
	gi.soundindex ("q1weapons/r_exp3.wav");
#endif	// KMQUAKE2_ENGINE_MOD
}


// NOTE: SP_rocket should ONLY be used to spawn rockets that change maps
//       via a trigger_transition. It should NOT be used for map entities.

void q1_rocket_delayed_start (edict_t *rocket)
{
//	if (g_edicts[1].linkcount)
	if ( AnyPlayerSpawned() )	// Knightmare- function handles multiple players
	{
		VectorScale (rocket->movedir, rocket->moveinfo.speed, rocket->velocity);
		rocket->nextthink = level.time + 8000/rocket->moveinfo.speed;
		rocket->think = G_FreeEdict;
		gi.linkentity (rocket);
	}
	else
		rocket->nextthink = level.time + FRAMETIME;
}

void SP_q1_rocket (edict_t *rocket)
{
	vec3_t	dir;

	rocket->s.modelindex = gi.modelindex ("models/objects/q1rocket/tris.md2");
	rocket->s.sound      = gi.soundindex ("weapons/rockfly.wav");
	rocket->touch = q1_rocket_touch;
	AngleVectors (rocket->s.angles, dir, NULL, NULL);
	VectorCopy (dir, rocket->movedir);
	rocket->moveinfo.speed = VectorLength(rocket->velocity);
	if (rocket->moveinfo.speed <= 0)
		rocket->moveinfo.speed = 650;

	// For SP, freeze rocket until player spawns in
	if (game.maxclients == 1)
	{
		VectorClear (rocket->velocity);
		rocket->think = q1_rocket_delayed_start;
		rocket->nextthink = level.time + FRAMETIME;
	}
	else
	{
		rocket->think = G_FreeEdict;
		rocket->nextthink = level.time + 8000/rocket->moveinfo.speed;
	}
	gi.linkentity (rocket);
}


/*
=================
q1_fire_lightning

Fires a lighting bolt.  Used by ?
=================
*/
void q1_fire_lightning (edict_t *self, vec3_t start, vec3_t dir, int damage)
{
	vec3_t end;
	trace_t  tr;

	
	VectorNormalize (dir);
	VectorMA (start, 600, dir, end);

	// Initial Trace - damage if close enough
	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		if (tr.ent && (tr.ent != self) && (tr.ent->takedamage))
		T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, 0, DAMAGE_ENERGY, MOD_Q1_LG);
	    return;
	}
	else
	{
		// trace 2
		tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	
		if (self->client->chasetoggle)
		{
			gi.WriteByte (svc_temp_entity);
#ifdef KMQUAKE2_ENGINE_MOD
			gi.WriteByte (TE_LIGHTNING_ATTACK);
			gi.WriteShort (self->client->oldplayer - g_edicts);
			gi.WritePosition (start);
			gi.WritePosition (tr.endpos); 
			gi.WriteByte (2);	// model 2 specifies player bolt model 
#else
			gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
			gi.WriteShort (self->client->oldplayer - g_edicts);
			gi.WritePosition (start);
			gi.WritePosition (tr.endpos); 
#endif	// KMQUAKE2_ENGINE_MOD
			gi.multicast (self->client->oldplayer->s.origin, MULTICAST_PVS);
		//	gi.multicast (self->client->oldplayer->s.origin, MULTICAST_ALL);
		}
		else
		{
			gi.WriteByte (svc_temp_entity);
#ifdef KMQUAKE2_ENGINE_MOD
			gi.WriteByte (TE_LIGHTNING_ATTACK);
			gi.WriteShort (self - g_edicts);
			gi.WritePosition (start);
			gi.WritePosition (tr.endpos);
			gi.WriteByte (2);	// model 2 specifies player bolt model 
#else
			gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
			gi.WriteShort (self - g_edicts);
			gi.WritePosition (start);
			gi.WritePosition (tr.endpos);
#endif	// KMQUAKE2_ENGINE_MOD
			gi.multicast (self->s.origin, MULTICAST_PVS);
		//	gi.multicast (self->s.origin, MULTICAST_ALL);
		}

		if ((tr.ent != self) && (tr.ent->takedamage))
			T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, 0, DAMAGE_ENERGY, MOD_Q1_LG);
	}
}


/*
=================
q1_fire_firepod

Fires a homing fire pod.  Used by the Vore (shalrath).
=================
*/
void q1_firepod_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	int		mod;
	vec3_t origin;

	if (other == self->owner)
		return;

	if ( (Q_stricmp(other->classname, "q1_monster_zombie") == 0) || (Q_stricmp(other->classname, "monster_q1_zombie") == 0) )
		T_Damage (other, self, self->owner, vec3_origin, other->s.origin, vec3_origin, 200, 0, 0, 0);
	T_RadiusDamage (self, self->owner, 40, NULL, 40, MOD_Q1_FIREPOD);

	VectorMA (self->s.origin, -0.02, self->velocity, origin);

#ifdef KMQUAKE2_ENGINE_MOD
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION_Q1);
	gi.WritePosition (origin);
	gi.WriteByte (0);	// subtype 0 specifies default explosion
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
#else	// KMQUAKE2_ENGINE_MOD
	gi.sound (self, CHAN_WEAPON, gi.soundindex ("q1weapons/r_exp3.wav"), 1.0, ATTN_NORM, 0);	

//	gi.WriteByte (svc_temp_entity);
//	gi.WriteByte (TE_Q1_EXPLOSION); 
	// FX Paril
//	gi.WriteByte (TE_ROCKET_EXPLOSION); 
//	gi.WritePosition (origin);
//	gi.multicast (self->s.origin, MULTICAST_PVS);

	gi.unlinkentity (self);
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorCopy (origin, self->s.origin);
	VectorCopy (origin, self->s.old_origin);
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/s_explod.sp2");
	self->s.frame = 0; 
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST; 
	self->think = q1_explode; 
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);

//	q1_explo_particles (self, origin);
//	if (Q_stricmp(self->classname, "freed"))
//		G_FreeEdict (self);
#endif	// KMQUAKE2_ENGINE_MOD
}


void q1_firepod_home (edict_t *self)
{
	vec3_t dir, vtemp;
	VectorClear (self->velocity);
	VectorCopy (self->enemy->s.origin, vtemp);
	VectorSet (dir, 0, 0, 10);
	VectorAdd (vtemp, dir, vtemp);
	VectorClear (dir);
	if (self->enemy->health < 1)
	{
		G_FreeEdict(self);
		return;
	}
	if (self->enemy->client && (self->enemy->client->ps.pmove.pm_flags & PMF_DUCKED))
		vtemp[2] -= 10;
	VectorSubtract (vtemp, self->s.origin, dir);
	VectorNormalize (dir);
	if (skill->value >= 2)
		VectorMA (self->velocity, 350, dir, self->velocity);
	else
		VectorMA (self->velocity, 250, dir, self->velocity);
	self->nextthink = level.time + 0.2;
}


void q1_fire_firepod (edict_t *self, vec3_t dir)
{
	edict_t	*pod;
	trace_t	tr;
	vec3_t start, temp;

	VectorCopy (self->s.origin, start);
	VectorSet (temp, 0, 0,  10);
	VectorAdd (start, temp, start);

	VectorNormalize (dir);

	pod = G_Spawn();
	pod->svflags |= SVF_DEADMONSTER;
	// yes, I know it looks weird that projectiles are deadmonsters
	// what this means is that when prediction is used against the object
	// (blaster/hyperblaster shots), the player won't be solid clipped against
	// the object.  Right now trying to run into a firing hyperblaster
	// is very jerky since you are predicted 'against' the shots.
	
	VectorCopy (start, pod->s.origin);
	VectorCopy (start, pod->s.old_origin);
	vectoangles (dir, pod->s.angles);
	VectorScale (dir, 400, pod->velocity);
	pod->movetype = MOVETYPE_FLYMISSILE;
	pod->clipmask = MASK_SHOT;
	pod->solid = SOLID_BBOX;
	VectorSet (pod->avelocity, 300, 300, 300);
#ifdef KMQUAKE2_ENGINE_MOD
	pod->s.effects |= EF_FLAG1|EF_BLUEHYPERBLASTER;
#else	// KMQUAKE2_ENGINE_MOD
	pod->s.effects |= EF_FLAG1|EF_FLAG2;
#endif	// KMQUAKE2_ENGINE_MOD
	VectorClear (pod->mins);
	VectorClear (pod->maxs);
	pod->s.modelindex = gi.modelindex ("models/monsters/q1shalrath/v_spike/tris.md2");
//	pod->s.sound = gi.soundindex ("misc/lasfly.wav");
	pod->touch = q1_firepod_touch;
	pod->nextthink = level.time +.1;
	pod->think = q1_firepod_home;
	pod->classname = "q1_firepod";
	pod->enemy = self->enemy;
	pod->owner = self;

	pod->common_name = "Firepod";
	pod->class_id = ENTITY_Q1_FIREPOD;

	gi.linkentity (pod);

//	if (self->client)
//		check_dodge (self, pod->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, pod->s.origin, pod, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (pod->s.origin, -10, dir, pod->s.origin);
		pod->touch (pod, tr.ent, NULL, NULL);
	}
}


void q1_firepod_precache (void)
{
	gi.modelindex ("models/monsters/q1shalrath/v_spike/tris.md2");
#ifndef KMQUAKE2_ENGINE_MOD
	gi.modelindex ("sprites/s_explod.sp2");
	gi.soundindex ("q1weapons/r_exp3.wav");
#endif	// KMQUAKE2_ENGINE_MOD
}


// NOTE: SP_q1_firepod should ONLY be used to spawn firepods that change maps
//       via a trigger_transition. It should NOT be used for map entities.

void q1_firepod_delayed_start (edict_t *pod)
{
//	if (g_edicts[1].linkcount)
	if ( AnyPlayerSpawned() )	// Knightmare- function handles multiple players
	{
		VectorScale (pod->movedir, pod->moveinfo.speed, pod->velocity);
		pod->nextthink = level.time + 8000/pod->moveinfo.speed;
		pod->think = G_FreeEdict;
		gi.linkentity (pod);
	}
	else
		pod->nextthink = level.time + FRAMETIME;
}

void SP_q1_firepod (edict_t *pod)
{
	vec3_t	dir;

	pod->s.modelindex = gi.modelindex ("models/monsters/q1shalrath/v_spike/tris.md2");
	pod->touch = q1_firepod_touch;
	AngleVectors (pod->s.angles, dir, NULL, NULL);
	VectorCopy (dir, pod->movedir);
	pod->moveinfo.speed = VectorLength(pod->velocity);
	if (pod->moveinfo.speed <= 0)
		pod->moveinfo.speed = 400;

	// For SP, freeze firepod until player spawns in
	if (game.maxclients == 1)
	{
		VectorClear (pod->velocity);
		pod->think = q1_firepod_delayed_start;
		pod->nextthink = level.time + FRAMETIME;
	}
	else
	{
		pod->think = G_FreeEdict;
		pod->nextthink = level.time + 8000/pod->moveinfo.speed;
	}
	gi.linkentity (pod);
}


/*
=================
q1_fire_lavaball

Fires a lavaball.  Used by Chthon.
=================
*/
/*static*/ void q1_lavaball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, (ent->dmg*2), 0, MOD_Q1_RL);
	}

	T_RadiusDamage (ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_Q1_RL_SPLASH);

#ifdef KMQUAKE2_ENGINE_MOD
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION_Q1);
	gi.WritePosition (origin);
	gi.WriteByte (0);	// subtype 0 specifies default explosion
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	G_FreeEdict (ent);
#else	// KMQUAKE2_ENGINE_MOD
	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("q1weapons/r_exp3.wav"), 1.0, ATTN_NORM, 0);	

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
	// explosion sprite 
	gi.unlinkentity (ent);
	ent->solid = SOLID_NOT;
	ent->touch = NULL;
	VectorCopy(ent->s.origin, origin);
	VectorCopy(ent->s.origin, ent->s.old_origin);
	VectorClear (ent->velocity);
	ent->s.modelindex = gi.modelindex ("sprites/s_explod.sp2"); 
	ent->s.frame = 0;
	ent->s.sound = 0;
	ent->s.effects &= ~EF_ANIM_ALLFAST; 
	ent->think = q1_explode; 
	ent->nextthink = level.time + FRAMETIME;
	ent->enemy = other;
	gi.linkentity (ent);
#endif	// KMQUAKE2_ENGINE_MOD
}


void q1_fire_lavaball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*lavaball;

	lavaball = G_Spawn();
	VectorCopy (start, lavaball->s.origin);
	VectorCopy (dir, lavaball->movedir);
	vectoangles (dir, lavaball->s.angles);
	VectorScale (dir, speed, lavaball->velocity);
	lavaball->movetype = MOVETYPE_FLYMISSILE;
	lavaball->clipmask = MASK_SHOT;
	lavaball->solid = SOLID_BBOX;
	lavaball->s.effects |= EF_ROCKET;	// EF_FLAG1
	lavaball->s.renderfx |= RF_GLOW;

	VectorClear (lavaball->mins);
	VectorClear (lavaball->maxs);
	lavaball->s.modelindex = gi.modelindex ("models/monsters/q1chthon/lavaball/tris.md2");
	lavaball->owner = self;
	lavaball->touch = q1_lavaball_touch;
	lavaball->nextthink = level.time + 8000/speed;
	lavaball->think = G_FreeEdict;
	lavaball->dmg = damage;
	lavaball->radius_dmg = radius_damage;
	lavaball->dmg_radius = damage_radius;
	lavaball->s.sound = gi.soundindex ("weapons/rockfly.wav");
	lavaball->classname = "chthon_lavaball";

	lavaball->common_name = "Chthon Lavaball";
	lavaball->class_id = ENTITY_Q1_LAVABALL;

	gi.linkentity (lavaball);
}


void q1_fire_lavaball_precache (void)
{
	gi.modelindex ("models/monsters/q1chthon/lavaball/tris.md2");
#ifndef KMQUAKE2_ENGINE_MOD
	gi.modelindex ("sprites/s_explod.sp2");
	gi.soundindex ("q1weapons/r_exp3.wav");
#endif	// KMQUAKE2_ENGINE_MOD
}


// NOTE: SP_q1_lavaball should ONLY be used to spawn lavaballs that change maps
//       via a trigger_transition. It should NOT be used for map entities.

void q1_lavaball_delayed_start (edict_t *lavaball)
{
//	if (g_edicts[1].linkcount)
	if ( AnyPlayerSpawned() )	// Knightmare- function handles multiple players
	{
		VectorScale (lavaball->movedir, lavaball->moveinfo.speed, lavaball->velocity);
		lavaball->nextthink = level.time + 8000/lavaball->moveinfo.speed;
		lavaball->think = G_FreeEdict;
		gi.linkentity (lavaball);
	}
	else
		lavaball->nextthink = level.time + FRAMETIME;
}

void SP_q1_lavaball (edict_t *lavaball)
{
	vec3_t	dir;

	lavaball->s.modelindex = gi.modelindex ("models/monsters/q1chthon/lavaball/tris.md2");
	lavaball->s.sound = gi.soundindex ("weapons/rockfly.wav");
	lavaball->touch = q1_lavaball_touch;
	AngleVectors (lavaball->s.angles, dir, NULL, NULL);
	VectorCopy (dir, lavaball->movedir);
	lavaball->moveinfo.speed = VectorLength(lavaball->velocity);
	if (lavaball->moveinfo.speed <= 0)
		lavaball->moveinfo.speed = 650;

	// For SP, freeze lavaball until player spawns in
	if (game.maxclients == 1)
	{
		VectorClear (lavaball->velocity);
		lavaball->think = q1_lavaball_delayed_start;
		lavaball->nextthink = level.time + FRAMETIME;
	}
	else
	{
		lavaball->think = G_FreeEdict;
		lavaball->nextthink = level.time + 8000/lavaball->moveinfo.speed;
	}
	gi.linkentity (lavaball);
}


/*
=================
q1_acidbolt_touch

Fires a bolt of acid spit.  Used by the Scrag.
=================
*/
/*static*/ void q1_acidbolt_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (other->takedamage)
	{
		// FIX MOD
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_Q1_RL);
	}
	else
	{
#ifdef KMQUAKE2_ENGINE_MOD
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SPIKEIMPACT_Q1);
		gi.WritePosition (ent->s.origin);
		gi.WriteDir ((!plane) ? vec3_origin : plane->normal);
		gi.WriteByte (3);	// subtype 3 specifies scrag acid impact
		gi.multicast (ent->s.origin, MULTICAST_PVS);

#else	// KMQUAKE2_ENGINE_MOD

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_WELDING_SPARKS);
		gi.WriteByte (15);
		gi.WritePosition (ent->s.origin);
		gi.WriteDir ((!plane) ? vec3_origin : plane->normal);
	//	gi.WriteByte (209);
		gi.WriteByte (222);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		gi.sound (ent, CHAN_VOICE, gi.soundindex ("q1scrag/hit.wav"), 1, ATTN_NORM, 0);
#endif	// KMQUAKE2_ENGINE_MOD
	}

	G_FreeEdict (ent);
}

/*
void G_Spawn_Trails(int type, vec3_t start, vec3_t endpos, vec3_t origin )
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (type);
	gi.WritePosition (start);
	gi.WritePosition (endpos);
	gi.multicast (origin, MULTICAST_PVS);
} */

void q1_fire_acidspit (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*acidbolt;

	acidbolt = G_Spawn();
	VectorCopy (start, acidbolt->s.origin);
	VectorCopy (dir, acidbolt->movedir);
	vectoangles (dir, acidbolt->s.angles);
	VectorScale (dir, speed, acidbolt->velocity);
	acidbolt->movetype = MOVETYPE_FLYMISSILE;
	acidbolt->clipmask = MASK_SHOT;
	acidbolt->solid = SOLID_BBOX;
#ifdef KMQUAKE2_ENGINE_MOD
	acidbolt->s.effects |= (EF_GREENGIB|EF_TRACKER);
#else	// KMQUAKE2_ENGINE_MOD
	acidbolt->s.effects |= EF_GREENGIB;		// EF_HYPERBLASTER EF_BFG EF_GREENTRAIL
#endif	// KMQUAKE2_ENGINE_MOD
	acidbolt->s.renderfx |= RF_TRANSLUCENT;	// FULLBRIGHT

	VectorClear (acidbolt->mins);
	VectorClear (acidbolt->maxs);
	acidbolt->s.modelindex = gi.modelindex ("models/monsters/q1scrag/w_spike/tris.md2");
	acidbolt->owner = self;
	acidbolt->touch = q1_acidbolt_touch;
	acidbolt->nextthink = level.time + 8000/speed;
	acidbolt->think = G_FreeEdict;
	acidbolt->dmg = damage;
//	acidbolt->s.sound = gi.soundindex ("weapons/rockfly.wav");
	acidbolt->classname = "q1_acidbolt";

	acidbolt->common_name = "Acid Bolt";
	acidbolt->class_id = ENTITY_Q1_ACIDBOLT;

	gi.linkentity (acidbolt);
}


void q1_acidspit_precache (void)
{
	gi.modelindex ("models/monsters/q1scrag/w_spike/tris.md2");
#ifndef KMQUAKE2_ENGINE_MOD
	gi.soundindex ("q1scrag/hit.wav");
#endif	// KMQUAKE2_ENGINE_MOD
}


// NOTE: SP_q1_acidspit should ONLY be used to spawn acid bolts that change
//       maps via trigger_transition. It should NOT be used for map entities.
void q1_acidspit_delayed_start (edict_t *acidbolt)
{
//	if (g_edicts[1].linkcount)
	if ( AnyPlayerSpawned() )	// Knightmare- function handles multiple players
	{
		VectorScale (acidbolt->movedir, acidbolt->moveinfo.speed, acidbolt->velocity);
		acidbolt->nextthink = level.time + 2;
		acidbolt->think = G_FreeEdict;
		gi.linkentity (acidbolt);
	}
	else
		acidbolt->nextthink = level.time + FRAMETIME;
}

void SP_q1_acidspit (edict_t *acidbolt)
{
	acidbolt->s.skinnum = 0;
	acidbolt->s.modelindex = gi.modelindex ("models/monsters/q1scrag/w_spike/tris.md2");
	acidbolt->touch = q1_acidbolt_touch;
	VectorCopy (acidbolt->velocity, acidbolt->movedir);
	VectorNormalize (acidbolt->movedir);
	acidbolt->moveinfo.speed = VectorLength(acidbolt->velocity);
	VectorClear (acidbolt->velocity);
	acidbolt->think = q1_acidspit_delayed_start;
	acidbolt->nextthink = level.time + FRAMETIME;
	gi.linkentity (acidbolt);
}


/*
=================
q1_fire_gib

Fires a gib projectile.  Used by the Zombie.
=================
*/
/*static*/ void q1_zombiegib_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (other->takedamage)
	{
		// FIX MOD
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_Q1_RL); 
		gi.sound (ent, CHAN_RELIABLE|CHAN_WEAPON, gi.soundindex ("q1zombie/z_hit.wav"), 1.0, ATTN_NORM, 0);	
	}
	else
	{
		gi.sound (ent, CHAN_RELIABLE|CHAN_WEAPON, gi.soundindex ("q1zombie/z_miss.wav"), 1.0, ATTN_NORM, 0);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLOOD);
		gi.WritePosition (ent->s.origin);
		gi.WriteDir (plane->normal);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
		
//	no more touches	
	ent->touch = NULL;

	ent->nextthink = level.time + 3;
	ent->think = G_FreeEdict;
//	G_FreeEdict (ent);
}


void q1_fire_gib (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
{
	edict_t	*gib;
	vec3_t	dir;
	vec3_t	up; //forward, right,

	vectoangles (aimdir, dir);
	AngleVectors (dir, NULL, NULL, up);

	gib = G_Spawn();
	VectorCopy (start, gib->s.origin);
	VectorScale (aimdir, speed, gib->velocity);
	VectorMA (gib->velocity, 200, up, gib->velocity);
//	VectorMA (gib->velocity, 20, right, gib->velocity);
	
	VectorSet (gib->avelocity, 3000, 1000, 2000);
	gib->movetype = MOVETYPE_BOUNCE;
	gib->clipmask = MASK_SHOT;
	gib->solid = SOLID_BBOX;
	gib->s.effects |= EF_GIB;
	VectorClear (gib->mins);
	VectorClear (gib->maxs);
	gib->s.modelindex = gi.modelindex ("models/monsters/q1zombie/gib/tris.md2"); 
	gib->owner = self;
	gib->touch = q1_zombiegib_touch;
	gib->nextthink = level.time + 2.5;
//	gib->think = G_FreeEdict;
	gib->think = gib_fade;	// use gib_fade() instead of directly removing
	gib->dmg = damage;
	gib->classname = "q1_zombiegib";

	gib->common_name = "Gib Projectile";
	gib->class_id = ENTITY_Q1_ZOMBIE_GIB;

	gi.linkentity (gib);
}


void q1_zombiegib_precache (void)
{
	gi.modelindex ("models/monsters/q1zombie/gib/tris.md2");
	gi.soundindex ("q1zombie/z_hit.wav");
	gi.soundindex ("q1zombie/z_miss.wav");
}


// NOTE: SP_q1_zombiegib should ONLY be used to spawn zombie gibs that change
//       maps via trigger_transition. It should NOT be used for map entities.
void q1_zombiegib_delayed_start (edict_t *gib)
{
//	if (g_edicts[1].linkcount)
	if ( AnyPlayerSpawned() )	// Knightmare- function handles multiple players
	{
		VectorScale (gib->movedir, gib->moveinfo.speed, gib->velocity);
		gib->nextthink = level.time + 2;
		gib->think = G_FreeEdict;
		gi.linkentity (gib);
	}
	else
		gib->nextthink = level.time + FRAMETIME;
}

void SP_q1_zombiegib (edict_t *gib)
{
	gib->s.skinnum = 0;
	gib->s.modelindex = gi.modelindex ("models/monsters/q1scrag/w_spike/tris.md2");
	gib->touch = q1_zombiegib_touch;
	VectorCopy (gib->velocity, gib->movedir);
	VectorNormalize (gib->movedir);
	gib->moveinfo.speed = VectorLength(gib->velocity);
	VectorClear (gib->velocity);
	gib->think = q1_zombiegib_delayed_start;
	gib->nextthink = level.time + FRAMETIME;
	gi.linkentity (gib);
}
