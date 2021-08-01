#include "g_local.h"

#define NUKE_QUAKE_TIME		3
#define NUKE_QUAKE_STRENGTH	100

/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/
//static void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed)
void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed)		//PGM
{
	vec3_t	end;
	vec3_t	v;
	trace_t	tr;
	float	eta;

	// easy mode only ducks one quarter the time
	if (skill->value == 0)
	{
		if (random() > 0.25)
			return;
	}
	VectorMA (start, WORLD_SIZE, dir, end);	// was 8192
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterinfo.dodge) && infront(tr.ent, self))
	{
			VectorSubtract (tr.endpos, start, v);
			eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
//			tr.ent->monsterinfo.dodge (tr.ent, self, eta);
			tr.ent->monsterinfo.dodge (tr.ent, self, eta, &tr);
	}
}


/*
=================
fire_hit

Used for all impact (hit/punch/slash) attacks
=================
*/
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick)
{
	trace_t		tr;
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	float		range;
	vec3_t		dir;

	//see if enemy is in range
	VectorSubtract (self->enemy->s.origin, self->s.origin, dir);
	range = VectorLength(dir);
	if (range > aim[0])
		return false;

	if (aim[1] > self->mins[0] && aim[1] < self->maxs[0])
	{
		// the hit is straight on so back the range up to the edge of their bbox
		range -= self->enemy->maxs[0];
	}
	else
	{
		// this is a side hit so adjust the "right" value out to the edge of their bbox
		if (aim[1] < 0)
			aim[1] = self->enemy->mins[0];
		else
			aim[1] = self->enemy->maxs[0];
	}

	VectorMA (self->s.origin, range, dir, point);

	tr = gi.trace (self->s.origin, NULL, NULL, point, self, MASK_SHOT);
	if (tr.fraction < 1)
	{
		if (!tr.ent->takedamage)
			return false;
		// if it will hit any client/monster then hit the one we wanted to hit
		if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
			tr.ent = self->enemy;
	}

	AngleVectors(self->s.angles, forward, right, up);
	VectorMA (self->s.origin, range, forward, point);
	VectorMA (point, aim[1], right, point);
	VectorMA (point, aim[2], up, point);
	VectorSubtract (point, self->enemy->s.origin, dir);

	// do the damage
	T_Damage (tr.ent, self, self, dir, point, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, MOD_HIT);

	if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		return false;

	// do our special form of knockback here
	VectorMA (self->enemy->absmin, 0.5, self->enemy->size, v);
	VectorSubtract (v, point, v);
	VectorNormalize (v);
	VectorMA (self->enemy->velocity, kick, v, self->enemy->velocity);
	if (self->enemy->velocity[2] > 0)
		self->enemy->groundentity = NULL;
	return true;
}


/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
/*static*/ void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;

	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		r = crandom()*hspread; 
		u = crandom()*vspread;
		// Knightmare- adjust spread for expanded world size
#ifdef KMQUAKE2_ENGINE_MOD
		r *= (WORLD_SIZE / 8192);
		u *= (WORLD_SIZE / 8192);
#endif
		VectorMA (start, WORLD_SIZE, forward, end);	// was 8192
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, NULL, NULL, end, self, content_mask);

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int		color;

			water = true;
			VectorCopy (tr.endpos, water_start);

			if (!VectorCompare (start, tr.endpos))
			{
				if (tr.ent->svflags & SVF_MUD)
					color = SPLASH_BROWN_WATER;
				else if (tr.contents & CONTENTS_WATER)
				{
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				// Knightmare- adjust spread for expanded world size
#ifdef KMQUAKE2_ENGINE_MOD
				r *= (WORLD_SIZE / 8192);
				u *= (WORLD_SIZE / 8192);
#endif
				VectorMA (water_start, WORLD_SIZE, forward, end);	// was 8192
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			// re-trace ignoring water this time
			tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (te_impact);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);

					// Lazarus reflections
					if (level.num_reflectors)
						ReflectSparks (te_impact, tr.endpos, tr.plane.normal);

					if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}
			}
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}
}


/*
=================
fire_bullet

Fires a single round.  Used for machinegun and chaingun.  Would be fine for
pistols, rifles, etc....
=================
*/
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
	fire_lead (self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
}


/*
=================
fire_shotgun

Shoots shotgun pellets.  Used by shotgun and super shotgun.
=================
*/
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
	int		i;

	for (i = 0; i < count; i++)
		fire_lead (self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
}


/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;
	int		tempevent;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	// PMM - crash prevention
	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		if (self->spawnflags & 1)
			mod = MOD_HYPERBLASTER;
		else
			mod = MOD_BLASTER;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		if (self->style == BLASTER_GREEN) //green
			tempevent = TE_BLASTER2;
		else if (self->style == BLASTER_BLUE) //blue
//	#ifdef KMQUAKE2_ENGINE_MOD
	#if defined (KMQUAKE2_ENGINE_MOD) || defined (Q2E_ENGINE_MOD)
			tempevent =  TE_BLUEHYPERBLASTER; // Knightmare- looks better than flechette
	#else
			tempevent = TE_FLECHETTE;
	#endif
//	#ifdef KMQUAKE2_ENGINE_MOD
	#if defined (KMQUAKE2_ENGINE_MOD) || defined (Q2E_ENGINE_MOD)
		else if (self->style == BLASTER_RED) //red
			tempevent =  TE_REDBLASTER;
	#endif
		else //standard orange
			tempevent = TE_BLASTER;

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (tempevent);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);

		// Lazarus reflections
		if (level.num_reflectors)
		{
			if (!plane)
				ReflectSparks (tempevent, self->s.origin, vec3_origin);
			else
				ReflectSparks (tempevent, self->s.origin, plane->normal);
		}
	}
	G_FreeEdict (self);
}

void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper, int color)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER;
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
	bolt->s.effects |= effect;
	bolt->s.renderfx |= RF_NOSHADOW; // Knightmare- no shadow
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);

	if (color == BLASTER_GREEN) // green
		bolt->s.skinnum = 1;
	else if (color == BLASTER_BLUE) // blue
		bolt->s.skinnum = 2;
	else if (color == BLASTER_RED) // red
		bolt->s.skinnum = 3;
	else // standard orange
		bolt->s.skinnum = 0;
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->style = color;

	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");

	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 4;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	if (hyper)
		bolt->spawnflags = 1;
	bolt->classname = "bolt";
	bolt->class_id = ENTITY_BOLT;
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0 && !(self->flags & FL_TURRET_OWNER))
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}

// RAFAEL
void fire_blueblaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t *bolt;
	trace_t tr;

	VectorNormalize (dir);

	bolt = G_Spawn ();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	bolt->s.renderfx |= RF_NOSHADOW; //Knightmare- no shadow
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);

	bolt->s.modelindex = gi.modelindex ("models/objects/blaser/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->style = BLASTER_BLUE;
	bolt->spawnflags |= 1;
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 4;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	bolt->class_id = ENTITY_BOLT;
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);

	if (tr.fraction < 1.0 && !(self->flags & FL_TURRET_OWNER))
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}

// NOTE: SP_bolt should ONLY be used for blaster/hyperblaster bolts that have
//       changed maps via trigger_transition. It should NOT be used for map
//       entities.
void bolt_delayed_start (edict_t *bolt)
{
	if (g_edicts[1].linkcount)
	{
		VectorScale(bolt->movedir,bolt->moveinfo.speed,bolt->velocity);
		bolt->nextthink = level.time + 2;
		bolt->think = G_FreeEdict;
		gi.linkentity(bolt);
	}
	else
		bolt->nextthink = level.time + FRAMETIME;
}

void SP_bolt (edict_t *bolt)
{
	if (bolt->style == BLASTER_GREEN) // green bolt
		bolt->s.skinnum = 1;
	else if (bolt->style == BLASTER_BLUE) // blue bolt
		bolt->s.skinnum = 2;
	else if (bolt->style == BLASTER_RED) // red bolt
		bolt->s.skinnum = 3;
	else // orange bolt
		bolt->s.skinnum = 0;
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->touch = blaster_touch;
	VectorCopy(bolt->velocity,bolt->movedir);
	VectorNormalize(bolt->movedir);
	bolt->moveinfo.speed = VectorLength(bolt->velocity);
	VectorClear(bolt->velocity);
	bolt->think = bolt_delayed_start;
	bolt->nextthink = level.time + FRAMETIME;
	gi.linkentity(bolt);
}

/*
=================
fire_grenade
=================
*/
// Lazarus additions: next_grenade and prev_grenade linked lists facilitate checking
// for grenades near monsters, so that monsters can evade w/o bogging down the game

void Grenade_Evade (edict_t *monster)
{
	edict_t	*grenade;
	vec3_t	grenade_vec;
	float	grenade_dist, best_r, best_yaw, r;
	float	yaw;
	int		i;
	vec3_t	forward;
	vec3_t	pos, best_pos;
	trace_t	tr;

	// We assume on entry here that monster is alive and that he's not already
	// AI_CHASE_THING
	grenade = world->next_grenade;
	while (grenade)
	{
		// we only care about grenades on the ground
		if (grenade->inuse && grenade->groundentity)
		{
			// if it ain't in the PVS, it can't hurt us (I think?)
			if (gi.inPVS(grenade->s.origin,monster->s.origin))
			{
				VectorSubtract (grenade->s.origin, monster->s.origin, grenade_vec);
				grenade_dist = VectorNormalize(grenade_vec);
				if (grenade_dist <= grenade->dmg_radius)
					break;
			}
		}
		grenade = grenade->next_grenade;
	}
	if (!grenade)
		return;
	// Find best escape route.
	best_r = 9999;
	for (i=0; i<8; i++)
	{
		yaw = anglemod( i*45 );
		forward[0] = cos( DEG2RAD(yaw) );
		forward[1] = sin( DEG2RAD(yaw) );
		forward[2] = 0;
		// Estimate of required distance to run. This is conservative.
		r = grenade->dmg_radius + grenade_dist*DotProduct(forward, grenade_vec) + monster->size[0] + 16;
		if ( r < best_r )
		{
			VectorMA (monster->s.origin, r, forward,pos);
			tr = gi.trace(monster->s.origin, monster->mins, monster->maxs, pos, monster, MASK_MONSTERSOLID);
			if (tr.fraction < 1.0)
				continue;
			best_r = r;
			best_yaw = yaw;
			VectorCopy (tr.endpos, best_pos);
		}
	}
	if (best_r < 9000)
	{
		edict_t	*thing = SpawnThing();
		VectorCopy(best_pos,thing->s.origin);
		thing->touch_debounce_time = grenade->nextthink;
		thing->target_ent = monster;
		ED_CallSpawn(thing);
		monster->ideal_yaw  = best_yaw;
		monster->movetarget = monster->goalentity = thing;
		monster->monsterinfo.aiflags &= ~AI_SOUND_TARGET;
		monster->monsterinfo.aiflags |= (AI_CHASE_THING | AI_EVADE_GRENADE);
		monster->monsterinfo.run(monster);
		monster->next_grenade = grenade;
	}
}

void Grenade_Add_To_Chain (edict_t *grenade)
{
	edict_t	*ancestor;

	ancestor = world;
	while (ancestor->next_grenade && ancestor->next_grenade->inuse)
		ancestor = ancestor->next_grenade;
	ancestor->next_grenade = grenade;
	grenade->prev_grenade = ancestor;
}

void Grenade_Remove_From_Chain (edict_t *grenade)
{
	if (grenade->prev_grenade)
	{
		// "prev_grenade" should always be valid for other than player-thrown
		// grenades that explode in player's hand
		grenade->prev_grenade->next_grenade = grenade->next_grenade;
		if (grenade->next_grenade)
			grenade->next_grenade->prev_grenade = grenade->prev_grenade;
	}
}

void Grenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;
	int			type;	// Knightmare added

//	Grenade_Remove_From_Chain (ent);

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		if (ent->spawnflags & 1)
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

	if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else if (ent->spawnflags & 1)
		mod = MOD_HG_SPLASH;
	else
		mod = MOD_G_SPLASH;
	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
	{
		if (ent->groundentity)
			type = TE_GRENADE_EXPLOSION_WATER;
		else
			type = TE_ROCKET_EXPLOSION_WATER;
	}
	else
	{
		if (ent->groundentity)
			type = TE_GRENADE_EXPLOSION;
		else
			type = TE_ROCKET_EXPLOSION;
	}
	gi.WriteByte (type);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectExplosion (type, origin);

	G_FreeEdict (ent);
}

void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
	//	Grenade_Remove_From_Chain (ent);
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
	Grenade_Explode (ent);
}

void ContactGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
	//	Grenade_Remove_From_Chain (ent);
		G_FreeEdict (ent);
		return;
	}

	if (other->takedamage)
		ent->enemy = other;

	Grenade_Explode (ent);
}

void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean contact)
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
	if (self->client)
		VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	else
		VectorMA (grenade->velocity, crandom() * 10.0, up, grenade->velocity);

	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	// Lazarus: Add owner velocity
//	VectorAdd (grenade->velocity, self->velocity, grenade->velocity);
	// NO. This is too unrealistic. Instead, if owner is riding a moving entity,
	//     add velocity of the thing he's riding

	// Knightmare- add player's base velocity to grenade
	if (add_velocity_throw->value && self->client)
		VectorAdd (grenade->velocity, self->velocity, grenade->velocity);
	else if (self->groundentity)
		VectorAdd (grenade->velocity, self->groundentity->velocity, grenade->velocity);

	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	grenade->s.renderfx |= RF_IR_VISIBLE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	if (contact)
		grenade->touch = ContactGrenade_Touch;
	else
		grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";
	grenade->class_id = ENTITY_GRENADE;

//	Grenade_Add_To_Chain (grenade);

	gi.linkentity (grenade);
}

void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	// Knightmare- add player's base velocity to thrown grenade
	if (add_velocity_throw->value && self->client)
		VectorAdd (grenade->velocity, self->velocity, grenade->velocity);
	else if (self->groundentity)
		VectorAdd (grenade->velocity, self->groundentity->velocity, grenade->velocity);

	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	grenade->s.renderfx |= RF_IR_VISIBLE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	grenade->class_id = ENTITY_HANDGRENADE;
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		Grenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
	//	Grenade_Add_To_Chain (grenade);
		gi.linkentity (grenade);
	}
}

// NOTE: SP_grenade and SP_handgrenade should ONLY be used to spawn grenades that change
//       maps via a trigger_transition. They should NOT be used for map entities.

void grenade_delayed_start (edict_t *grenade)
{
	if (g_edicts[1].linkcount)
	{
		VectorScale(grenade->movedir,grenade->moveinfo.speed,grenade->velocity);
		grenade->movetype  = MOVETYPE_BOUNCE;
		grenade->nextthink = level.time + 2.5;
		grenade->think     = Grenade_Explode;
		gi.linkentity(grenade);
	}
	else
		grenade->nextthink = level.time + FRAMETIME;
}

void SP_grenade (edict_t *grenade)
{
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->touch = Grenade_Touch;

	// For SP, freeze grenade until player spawns in
	if (game.maxclients == 1)
	{
		grenade->movetype  = MOVETYPE_NONE;
		VectorCopy(grenade->velocity,grenade->movedir);
		VectorNormalize(grenade->movedir);
		grenade->moveinfo.speed = VectorLength(grenade->velocity);
		VectorClear(grenade->velocity);
		grenade->think     = grenade_delayed_start;
		grenade->nextthink = level.time + FRAMETIME;
	}
	else
	{
		grenade->movetype  = MOVETYPE_BOUNCE;
		grenade->nextthink = level.time + 2.5;
		grenade->think     = Grenade_Explode;
	}
	gi.linkentity (grenade);
}

void handgrenade_delayed_start (edict_t *grenade)
{
	if (g_edicts[1].linkcount)
	{
		VectorScale(grenade->movedir,grenade->moveinfo.speed,grenade->velocity);
		grenade->movetype  = MOVETYPE_BOUNCE;
		grenade->nextthink = level.time + 2.5;
		grenade->think     = Grenade_Explode;
		if (grenade->owner)
			gi.sound (grenade->owner, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity(grenade);
	}
	else
		grenade->nextthink = level.time + FRAMETIME;
}

void SP_handgrenade (edict_t *grenade)
{
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->touch = Grenade_Touch;

	// For SP, freeze grenade until player spawns in
	if (game.maxclients == 1)
	{
		grenade->movetype  = MOVETYPE_NONE;
		VectorCopy(grenade->velocity,grenade->movedir);
		VectorNormalize(grenade->movedir);
		grenade->moveinfo.speed = VectorLength(grenade->velocity);
		VectorClear(grenade->velocity);
		grenade->think     = handgrenade_delayed_start;
		grenade->nextthink = level.time + FRAMETIME;
	}
	else
	{
		grenade->movetype  = MOVETYPE_BOUNCE;
		grenade->nextthink = level.time + 2.5;
		grenade->think     = Grenade_Explode;
	}
	gi.linkentity (grenade);
}


//==========================================================================================
//
// AimGrenade finds the correct aim vector to get a grenade from start to target at initial
// velocity = speed. Returns false if grenade can't make it to target.
//
//==========================================================================================
qboolean AimGrenade (edict_t *self, vec3_t start, vec3_t target, vec_t speed, vec3_t aim, qboolean isProx)
{
	vec3_t		angles, forward, right, up;
	vec3_t		from_origin, from_muzzle;
	vec3_t		aim_point;
	vec_t		xo, yo;
	vec_t		x;
	float		cosa, t, vx, y;
	float		drop;
	float		last_error, v_error;
	int			i;
	vec3_t		last_aim;
	// Knightmare- wider bounds to avoid prox collision
	vec3_t		proxMins = {-8, -8, -8};
	vec3_t		proxMaxs = {8, 8, 8};

	VectorCopy (target, aim_point);
	VectorSubtract (aim_point, self->s.origin, from_origin);
	VectorSubtract (aim_point, start, from_muzzle);

	if (self->svflags & SVF_MONSTER)
	{
		VectorCopy (from_muzzle, aim);
		VectorNormalize (aim);
		yo = from_muzzle[2];
		xo = sqrt (from_muzzle[0] * from_muzzle[0] + from_muzzle[1] * from_muzzle[1]);
	}
	else
	{
		VectorCopy (from_origin, aim);
		VectorNormalize (aim);
		yo = from_origin[2];
		xo = sqrt (from_origin[0] * from_origin[0] + from_origin[1] * from_origin[1]);
	}

	// If resulting aim vector is looking straight up or straight down, we're 
	// done. Actually now that I write this down and think about it... should
	// probably check straight up to make sure grenade will actually reach the
	// target.
	if ( (aim[2] == 1.0) || (aim[2] == -1.0))
		return true;

	// horizontal distance to target from muzzle
	x = sqrt (from_muzzle[0] * from_muzzle[0] + from_muzzle[1] * from_muzzle[1]);
	cosa = sqrt (aim[0] * aim[0] + aim[1] * aim[1]);
	// constant horizontal velocity (since grenades don't have drag)
	vx = speed * cosa;
	// time to reach target x
	t = x / vx;
	// if flight time is less than one frame, no way grenade will drop much,
	// shoot the sucker now.
	if (t < FRAMETIME)
		return true;
	// in that time, grenade will drop this much:
	drop = 0.5 * sv_gravity->value * t * t;
	y = speed * aim[2] * t - drop;
	v_error = target[2] - start[2] - y;

	// if we're fairly close and we'll hit target at current angle,
	// no need for all this, just shoot it
	if ( (x < 128) && (fabs(v_error) < 16) )
		return true;

	last_error = 100000.;
	VectorCopy (aim, last_aim);

	// Unfortunately there is no closed-form solution for this problem,
	// so we creep up on an answer and balk if it takes more than 
	// 10 iterations to converge to the tolerance we'll accept.
	for (i=0; i<10 && fabs(v_error) > 4 && fabs(v_error) < fabs(last_error); i++)
	{
		last_error = v_error;
		aim[2] = cosa * (yo + drop) / xo;
		VectorNormalize (aim);
		if (!(self->svflags & SVF_MONSTER))
		{
			vectoangles (aim, angles);
			AngleVectors (angles, forward, right, up);
			G_ProjectSource2 (self->s.origin, self->move_origin, forward, right, up, start);
			VectorSubtract (aim_point, start, from_muzzle);
			x = sqrt(from_muzzle[0] * from_muzzle[0] + from_muzzle[1] * from_muzzle[1]);
		}
		cosa = sqrt(aim[0] * aim[0] + aim[1] * aim[1]);
		vx = speed * cosa;
		t = x / vx;
		drop = 0.5 * sv_gravity->value * t * t;
		y = speed * aim[2] * t - drop;
		v_error = target[2] - start[2] - y;
		if (fabs(v_error) < fabs(last_error))
			VectorCopy (aim, last_aim);
	}
	
	if (i >= 10 || v_error > 64)
		return false;

	if (fabs(v_error) > fabs(last_error))
	{
		VectorCopy (last_aim, aim);
		if (!(self->svflags & SVF_MONSTER))
		{
			vectoangles (aim, angles);
			AngleVectors (angles, forward, right, up);
			G_ProjectSource2 (self->s.origin, self->move_origin, forward, right, up, start);
			VectorSubtract (aim_point, start, from_muzzle);
		}
	}
	
	// Sanity check... if launcher is at the same elevation or a bit above the 
	// target entity, check to make sure he won't bounce grenades off the 
	// top of a doorway or other obstruction. If he WOULD do that, then figure out 
	// the max elevation angle that will get the grenade through the door, and 
	// hope we get a good bounce.
	if ( (start[2] - target[2] < 160) &&
		(start[2] - target[2] > -16)   )
	{
		trace_t	tr;
		vec3_t	dist;
		
		if (isProx)
			tr = gi.trace(start, proxMins, proxMaxs, aim_point, self, MASK_SOLID);
		else
			tr = gi.trace(start, vec3_origin, vec3_origin, aim_point, self, MASK_SOLID);
		if ( (tr.fraction < 1.0) && (!self->enemy || (tr.ent != self->enemy) ))
		{
			// OK... the aim vector hit a solid, but would the grenade actually hit?
			int		contents;
			cosa = sqrt(aim[0] * aim[0] + aim[1] * aim[1]);
			vx = speed * cosa;
			VectorSubtract (tr.endpos, start, dist);
			dist[2] = 0;
			x = VectorLength (dist);
			t = x / vx;
			drop = 0.5 * sv_gravity->value * t * (t + FRAMETIME);
			tr.endpos[2] -= drop;
			// move just a bit in the aim direction
			tr.endpos[0] += aim[0];
			tr.endpos[1] += aim[1];
			contents = gi.pointcontents(tr.endpos);
			while ((contents & MASK_SOLID) && (aim_point[2] > target[2]))
			{
				aim_point[2] -= 8.0;
				VectorSubtract (aim_point, self->s.origin, from_origin);
				VectorCopy (from_origin, aim);
				VectorNormalize (aim);
				if (!(self->svflags & SVF_MONSTER))
				{
					vectoangles (aim, angles);
					AngleVectors (angles, forward, right, up);
					G_ProjectSource2 (self->s.origin, self->move_origin, forward, right, up, start);
					VectorSubtract (aim_point, start, from_muzzle);
				}
				if (isProx)
					tr = gi.trace(start, proxMins, proxMaxs, aim_point, self, MASK_SOLID);
				else
					tr = gi.trace(start, vec3_origin, vec3_origin, aim_point, self, MASK_SOLID);
				if (tr.fraction < 1.0)
				{
					cosa = sqrt(aim[0] * aim[0] + aim[1] * aim[1]);
					vx = speed * cosa;
					VectorSubtract (tr.endpos, start, dist);
					dist[2] = 0;
					x = VectorLength(dist);
					t = x / vx;
					drop = 0.5 * sv_gravity->value * t * (t + FRAMETIME);
					tr.endpos[2] -= drop;
					tr.endpos[0] += aim[0];
					tr.endpos[1] += aim[1];
					contents = gi.pointcontents(tr.endpos);
				}
			}
		}
	}
	return true;
}


/*
=================
fire_missile
=================
*/
void missile_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	origin;
	int		n;
	int		type;

	if (other == ent->owner)
		return;

	if (ent->owner->client && (ent->owner->client->homing_rocket == ent))
		ent->owner->client->homing_rocket = NULL;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner && ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_MISSILE);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while (n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin, 0, 0, 0);
			}
		}
	}

	// Lazarus: bad monsters have a large damage radius
	if (ent->owner && (ent->owner->svflags & SVF_MONSTER) && !(ent->owner->monsterinfo.aiflags & AI_GOOD_GUY))
	//	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius + 17.5*skill->value, MOD_MISSILE_SPLASH, -2.0/(4.0+skill->value) );
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius + 17.5*skill->value, MOD_MISSILE_SPLASH);
	else
	//	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_MISSILE_SPLASH, -0.5);
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_MISSILE_SPLASH);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		type = TE_ROCKET_EXPLOSION_WATER;
	else
		type = TE_ROCKET_EXPLOSION;
	gi.WriteByte (type);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectExplosion (type, origin);

	G_FreeEdict (ent);
}

/*static*/ void missile_explode (edict_t *ent)
{
	vec3_t	origin;
	int		type;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

//	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_MISSILE_SPLASH, -0.5);
	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_MISSILE_SPLASH);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		type = TE_ROCKET_EXPLOSION_WATER;
	else
		type = TE_EXPLOSION1_BIG;
	gi.WriteByte (type);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectExplosion (type, origin);

	G_FreeEdict (ent);
}

/*static*/ void missile_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + FRAMETIME;
	self->think = missile_explode;
}

void homing_think (edict_t *self);
void Rocket_Evade (edict_t *rocket, vec3_t	dir, float speed);
void fire_missile (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, edict_t *home_target)
{
	edict_t	*missile;

	missile = G_Spawn();
	VectorCopy (start, missile->s.origin);
	VectorCopy (dir, missile->movedir);
	vectoangles (dir, missile->s.angles);
	VectorScale (dir, speed, missile->velocity);
	// Lazarus: add shooter's lateral velocity
	if (rocket_strafe->value)
	{
		vec3_t	right, up;
		vec3_t	lateral_speed;

		AngleVectors(self->s.angles,NULL,right,up);
		VectorCopy(self->velocity,lateral_speed);
		lateral_speed[0] *= fabs(right[0]);
		lateral_speed[1] *= fabs(right[1]);
		lateral_speed[2] *= fabs(up[2]);
		VectorAdd(missile->velocity,lateral_speed,missile->velocity);
	}
	missile->movetype = MOVETYPE_FLYMISSILE;
	missile->clipmask = MASK_SHOT;
	missile->solid = SOLID_BBOX;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	missile->s.effects |= EF_ROCKET;
	missile->s.renderfx |= RF_NOSHADOW; //Knightmare- no shadow
	missile->s.renderfx |= RF_IR_VISIBLE;
	VectorClear (missile->mins);
	VectorClear (missile->maxs);
	missile->s.modelindex = gi.modelindex ("models/objects/bomb/tris.md2");
	missile->owner = self;
	missile->touch = missile_touch;
	missile->dmg = damage;
	missile->radius_dmg = radius_damage;
	missile->dmg_radius = damage_radius;
	missile->s.sound = gi.soundindex ("weapons/rockfly.wav");

	if (home_target)
	{
		// homers are shootable
		VectorSet(missile->mins, -10, -3, 0);
		VectorSet(missile->maxs,  10,  3, 6);
		missile->mass = 10;
		missile->health = 5;
		missile->die = missile_die;
		missile->takedamage = DAMAGE_YES;
		missile->monsterinfo.aiflags = AI_NOSTEP;

		missile->enemy     = home_target;
		missile->classname = "homing rocket";
		missile->class_id = ENTITY_ROCKET;
		missile->nextthink = level.time + FRAMETIME;
		missile->think = homing_think;
		missile->starttime = level.time + 0.3; // play homing sound on 3rd frame
		missile->endtime   = level.time + 8000.0f/speed;
		if (self->client)
		{
			self->client->homing_rocket = missile;
			check_dodge (self, missile->s.origin, dir, speed);
		}
		Rocket_Evade (missile, dir, speed);
	}
	else
	{
		missile->classname = "missile";
		missile->class_id = ENTITY_ROCKET;
		missile->nextthink = level.time + 8000.0f/speed;
		missile->think = G_FreeEdict;
		Rocket_Evade (missile, dir, speed);
	}
	gi.linkentity (missile);
}

// NOTE: SP_missile should ONLY be used to spawn missiles that change maps
//       via a trigger_transition. It should NOT be used for map entities.

void missile_delayed_start (edict_t *missile)
{
	if (g_edicts[1].linkcount)
	{
		VectorScale(missile->movedir,missile->moveinfo.speed,missile->velocity);
		missile->nextthink = level.time + 8000/missile->moveinfo.speed;
		missile->think = G_FreeEdict;
		gi.linkentity(missile);
	}
	else
		missile->nextthink = level.time + FRAMETIME;
}

void SP_missile (edict_t *missile)
{
	vec3_t	dir;

	missile->s.modelindex = gi.modelindex ("models/objects/bomb/tris.md2");
	missile->s.sound      = gi.soundindex ("weapons/rockfly.wav");
	missile->touch = missile_touch;
	AngleVectors(missile->s.angles,dir,NULL,NULL);
	VectorCopy (dir, missile->movedir);
	missile->moveinfo.speed = VectorLength(missile->velocity);
	if (missile->moveinfo.speed <= 0)
		missile->moveinfo.speed = 650;

	// For SP, freeze missile until player spawns in
	if (game.maxclients == 1)
	{
		VectorClear(missile->velocity);
		missile->think = missile_delayed_start;
		missile->nextthink = level.time + FRAMETIME;
	}
	else
	{
		missile->think = G_FreeEdict;
		missile->nextthink = level.time + 8000/missile->moveinfo.speed;
	}
	gi.linkentity (missile);
}

/*
=================
fire_rocket
=================
*/
// Lazarus: homing rocket
void homing_think (edict_t *self)
{
	trace_t	tr;
	vec3_t	dir, target;
	vec_t	speed;

	if (level.time > self->endtime)
	{
		if (self->owner->client && (self->owner->client->homing_rocket == self))
			self->owner->client->homing_rocket = NULL;
		BecomeExplosion1 (self);
		return;
	}
	if (self->enemy && self->enemy->inuse)
	{
		VectorMA (self->enemy->absmin, 0.5, self->enemy->size, target);
		tr = gi.trace(self->s.origin, vec3_origin, vec3_origin, target, self, MASK_OPAQUE);
		if (tr.fraction == 1)
		{
			// target in view; apply correction
			VectorSubtract (target, self->s.origin, dir);
			VectorNormalize (dir);
			if (self->enemy->client)
				VectorScale (dir, 0.8+0.1*skill->value, dir);
			else
				VectorScale (dir, 1.0, dir);  // 0=no correction, 1=turn on a dime
			VectorAdd (dir, self->movedir, dir);
			VectorNormalize (dir);
			VectorCopy (dir, self->movedir);
			vectoangles (dir, self->s.angles);
			speed = VectorLength(self->velocity);
			VectorScale (dir, speed, self->velocity);

			if (level.time >= self->starttime && self->starttime > 0)
			{
				if (level.time > self->owner->fly_sound_debounce_time)
				{
					// this prevents multiple lockon sounds resulting from
					// monsters firing multiple rockets in quick succession
				#ifdef KMQUAKE2_ENGINE_MOD // extra sound only with engine mod
					if (self->enemy->client)
						gi.sound (self->enemy, CHAN_AUTO, gi.soundindex ("weapons/homing/lockon.wav"), 1, ATTN_NORM, 0);
				#endif
					self->owner->fly_sound_debounce_time = level.time + 2.0;
				}
				self->starttime = 0;
			}
		}
	}
	self->nextthink = level.time + FRAMETIME;
}

// Lazarus: Rocket_Evade tells monsters to get da hell outta da way

void Rocket_Evade (edict_t *rocket, vec3_t	dir, float speed)
{
	float	rocket_dist, best_r, best_yaw, dist, r;
	float	time;
	float	dot;
	float	yaw;
	int		i;
	edict_t	*ent=NULL;
	trace_t	tr;
	vec3_t	hitpoint;
	vec3_t	forward, pos, best_pos;
	vec3_t	rocket_vec, vec;

	// Find out what rocket will hit, assuming everything remains static
	VectorMA (rocket->s.origin, WORLD_SIZE, dir, rocket_vec);	// was 8192
	tr = gi.trace(rocket->s.origin, rocket->mins, rocket->maxs, rocket_vec, rocket,MASK_SHOT);
	VectorCopy (tr.endpos, hitpoint);
	VectorSubtract (hitpoint, rocket->s.origin, vec);
	dist = VectorLength (vec);
	time = dist / speed;

	while ((ent = findradius(ent, hitpoint, rocket->dmg_radius)) != NULL)
	{
		if (!ent->inuse)
			continue;
		if (!(ent->svflags & SVF_MONSTER))
			continue;
		if (!ent->takedamage)
			continue;
		if (ent->health <= 0)
			continue;
		if (!ent->monsterinfo.run)	// takes care of turret_driver
			continue;
		if (rocket->owner == ent)
			continue;

		VectorSubtract (hitpoint, ent->s.origin, rocket_vec);
		rocket_dist = VectorNormalize(rocket_vec);

		// Not much hope in evading if distance is < 1K or so.
		if (rocket_dist < 1024)
			continue;

		// Find best escape route.
		best_r = 9999;
		for (i=0; i<8; i++)
		{
			yaw = anglemod( i*45 );
			forward[0] = cos( DEG2RAD(yaw) );
			forward[1] = sin( DEG2RAD(yaw) );
			forward[2] = 0;
			dot = DotProduct(forward,dir);
			if ((dot > 0.96) || (dot < -0.96))
				continue;
			// Estimate of required distance to run. This is conservative.
			r = rocket->dmg_radius + rocket_dist*DotProduct(forward, rocket_vec) + ent->size[0] + 16;
			if ( r < best_r )
			{
				VectorMA (ent->s.origin, r, forward ,pos);
				tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, pos, ent, MASK_MONSTERSOLID);
				if (tr.fraction < 1.0)
					continue;
				best_r = r;
				best_yaw = yaw;
				VectorCopy (tr.endpos, best_pos);
			}
		}
		if (best_r < 9000)
		{
			edict_t	*thing = SpawnThing();
			VectorCopy (best_pos, thing->s.origin);
			thing->touch_debounce_time = level.time + time;
			thing->target_ent = ent;
			ED_CallSpawn (thing);
			ent->ideal_yaw  = best_yaw;
			ent->movetarget = ent->goalentity = thing;
			ent->monsterinfo.aiflags &= ~AI_SOUND_TARGET;
			ent->monsterinfo.aiflags |= (AI_CHASE_THING | AI_EVADE_GRENADE);
			ent->monsterinfo.run(ent);
		}
	}
}


void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	origin;
	int		n;
	int		type;

	if (other == ent->owner)
		return;

	if (ent->owner->client && (ent->owner->client->homing_rocket == ent))
		ent->owner->client->homing_rocket = NULL;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner && ent->owner->client)
		PlayerNoise (ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while (n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin, 0, 0, 0);
			}
		}
	}

	// Lazarus: bad monsters have a large damage radius
	if (ent->owner && (ent->owner->svflags & SVF_MONSTER) && !(ent->owner->monsterinfo.aiflags & AI_GOOD_GUY))
	//	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius + 17.5*skill->value, MOD_R_SPLASH, -2.0/(4.0+skill->value) );
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius + 17.5*skill->value, MOD_R_SPLASH);
	else
	//	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH, -0.5);
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		type = TE_ROCKET_EXPLOSION_WATER;
	else
		type = TE_ROCKET_EXPLOSION;
	gi.WriteByte (type);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectExplosion (type, origin);

	G_FreeEdict (ent);
}

/*static*/ void rocket_explode (edict_t *ent)
{
	vec3_t	origin;
	int		type;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

//	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_R_SPLASH, -0.5);
	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_R_SPLASH);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		type = TE_ROCKET_EXPLOSION_WATER;
	else
		type = TE_ROCKET_EXPLOSION;
	gi.WriteByte (type);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectExplosion (type, origin);

	G_FreeEdict (ent);
}

/*static*/ void rocket_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + FRAMETIME;
	self->think = rocket_explode;
}

void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, edict_t *home_target)
{
	edict_t	*rocket;
	qboolean homing = false;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	// Lazarus: add shooter's lateral velocity
	if (rocket_strafe->value)
	{
		vec3_t	right, up;
		vec3_t	lateral_speed;

		AngleVectors (self->s.angles, NULL, right, up);
		VectorCopy (self->velocity, lateral_speed);
		lateral_speed[0] *= fabs(right[0]);
		lateral_speed[1] *= fabs(right[1]);
		lateral_speed[2] *= fabs(up[2]);
		VectorAdd(rocket->velocity,lateral_speed,rocket->velocity);
	}

	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET;
	rocket->s.renderfx |=	RF_IR_VISIBLE|RF_NOSHADOW; // Knightmare- no shadow
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
//	if (home_target)
//		rocket->s.modelindex = gi.modelindex ("models/objects/hrocket/tris.md2");
//	else
		rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");

	if (strstr(self->classname,"monster_") && (self->spawnflags & SF_MONSTER_SPECIAL)
		&& strcmp(self->classname, "monster_turret"))
		homing = true;
	// Knightmare- use different skin, not different model, for homing rocket
	if (home_target || (self->client && self->client->pers.fire_mode) || homing)
	{
		rocket->s.skinnum = 1;
		rocket->classname = "homing rocket";
	}
	else
		rocket->classname = "rocket";
	rocket->class_id = ENTITY_ROCKET;

	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");

	if (home_target)
	{
		// homers are shootable
		VectorSet(rocket->mins, -10, -3, 0);
		VectorSet(rocket->maxs,  10,  3, 6);
		rocket->mass = 10;
		rocket->health = 5;
		rocket->die = rocket_die;
		rocket->takedamage = DAMAGE_YES;
		rocket->monsterinfo.aiflags = AI_NOSTEP;

		rocket->enemy     = home_target;
		rocket->nextthink = level.time + FRAMETIME;
		rocket->think = homing_think;
		rocket->starttime = level.time + 0.3; // play homing sound on 3rd frame
		rocket->endtime   = level.time + 8000.0f/speed;

		if (self->client)
		{
			self->client->homing_rocket = rocket;
//			check_dodge (self, rocket->s.origin, dir, speed);
		}
		Rocket_Evade (rocket, dir, speed);
	}
	else
	{
		rocket->nextthink = level.time + 8000.0f/speed;
		rocket->think = G_FreeEdict;
		Rocket_Evade (rocket, dir, speed);
	}

	gi.linkentity (rocket);
}

// NOTE: SP_rocket should ONLY be used to spawn rockets that change maps
//       via a trigger_transition. It should NOT be used for map entities.

void rocket_delayed_start (edict_t *rocket)
{
	if (g_edicts[1].linkcount)
	{
		VectorScale(rocket->movedir,rocket->moveinfo.speed,rocket->velocity);
		rocket->nextthink = level.time + 8000/rocket->moveinfo.speed;
		rocket->think = G_FreeEdict;
		gi.linkentity(rocket);
	}
	else
		rocket->nextthink = level.time + FRAMETIME;
}

void SP_rocket (edict_t *rocket)
{
	vec3_t	dir;

	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	if (!strcmp(rocket->classname, "homing rocket"))
		rocket->s.skinnum = 1;
	rocket->s.sound      = gi.soundindex ("weapons/rockfly.wav");
	rocket->touch = rocket_touch;
	AngleVectors(rocket->s.angles,dir,NULL,NULL);
	VectorCopy (dir, rocket->movedir);
	rocket->moveinfo.speed = VectorLength(rocket->velocity);
	if (rocket->moveinfo.speed <= 0)
		rocket->moveinfo.speed = 650;

	// For SP, freeze rocket until player spawns in
	if (game.maxclients == 1)
	{
		VectorClear(rocket->velocity);
		rocket->think = rocket_delayed_start;
		rocket->nextthink = level.time + FRAMETIME;
	}
	else
	{
		rocket->think = G_FreeEdict;
		rocket->nextthink = level.time + 8000/rocket->moveinfo.speed;
	}
	gi.linkentity (rocket);
}

/*void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while (n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin, 0, 0, 0);
			}
		}
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
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
	rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 8000.0f/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}*/

/*
=================
fire_rail
=================
*/
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	vec3_t		from, end;
	trace_t		tr;
	edict_t		*ignore;
	int			mask, tempevent, i=0;
	qboolean	water;

	// Knightmare- changeable trail color
//#ifdef KMQUAKE2_ENGINE_MOD
#if defined (KMQUAKE2_ENGINE_MOD) || defined (Q2E_ENGINE_MOD)
	if ( (self->client) && (sk_rail_color->value == 2) )
		tempevent = TE_RAILTRAIL2;
	else
#endif
		tempevent = TE_RAILTRAIL;

	VectorMA (start, WORLD_SIZE, aimdir, end);	// was 8192
	VectorCopy (start, from);
	ignore = self;
	water = false;

	// Zaero- hack for certain maps to shoot through windows
	if ( (self->client) && level.isZaeroRailgunHackMap )
		mask = MASK_SHOT_NO_WINDOW|CONTENTS_SLIME|CONTENTS_LAVA;
	else
		mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	// end Zaero

	while (ignore && i<256)
	{
		tr = gi.trace (from, NULL, NULL, end, ignore, mask);

		if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
			water = true;
		}
		else
		{
			// ZOID--added so rail goes through SOLID_BBOX entities (gibs, etc)
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client) ||
				(tr.ent->svflags & SVF_DAMAGEABLE) ||
				(tr.ent->solid == SOLID_BBOX))
				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (tr.ent->takedamage))
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_RAILGUN);
		}

		VectorCopy (tr.endpos, from);
		i++;
	}

	// send gun puff / flash
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (tempevent);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PHS);
//	gi.multicast (start, MULTICAST_PHS);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectTrail (tempevent, start, tr.endpos);

	if (water)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (tempevent);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (tr.endpos, MULTICAST_PHS);
	}

	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}


/*
=================
fire_bfg
=================
*/
void bfg_explode (edict_t *self)
{
	edict_t	*ent;
	float	points;
	vec3_t	v;
	float	dist;

	if (self->s.frame == 0)
	{
		// the BFG effect
		ent = NULL;
		while ((ent = findradius(ent, self->s.origin, self->dmg_radius)) != NULL)
		{
			if (!ent->takedamage)
				continue;
			if (ent == self->owner)
				continue;
			if (!CanDamage (ent, self))
				continue;
			if (!CanDamage (ent, self->owner))
				continue;

			VectorAdd (ent->mins, ent->maxs, v);
			VectorMA (ent->s.origin, 0.5, v, v);
			VectorSubtract (self->s.origin, v, v);
			dist = VectorLength(v);
			points = self->radius_dmg * (1.0 - sqrt(dist/self->dmg_radius));
// PMM - happened to notice this copy/paste bug
//			if (ent == self->owner)
//				points = points * 0.5;

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_EXPLOSION);
			gi.WritePosition (ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PHS);

			// Lazarus reflections
			if (level.num_reflectors)
				ReflectExplosion (TE_BFG_EXPLOSION, ent->s.origin);

			T_Damage (ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY, MOD_BFG_EFFECT);
		}
	}

	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 5)
		self->think = G_FreeEdict;
}

void bfg_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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

	// core explosion - prevents firing it into the wall/floor
	if (other->takedamage)
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, sk_bfg_rdamage->value, 0, 0, MOD_BFG_BLAST); //was 200
	T_RadiusDamage(self, self->owner, sk_bfg_rdamage->value, other, 100, MOD_BFG_BLAST); //was 200

	gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorMA (self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/s_bfg3.sp2");
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST;
	self->think = bfg_explode;
	self->nextthink = level.time + FRAMETIME;
	self->enemy = other;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectExplosion (TE_BFG_BIGEXPLOSION, self->s.origin);
}


void bfg_think (edict_t *self)
{
	edict_t	*ent;
	edict_t	*ignore;
	vec3_t	point;
	vec3_t	dir;
	vec3_t	start;
	vec3_t	end;
	int		dmg;
	trace_t	tr;

	if (deathmatch->value)
		dmg = sk_bfg_damage2_dm->value; //was 5
	else
		dmg = sk_bfg_damage2->value; //was 10

	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 256)) != NULL)
	{
		if (ent == self)
			continue;

		if (ent == self->owner)
			continue;

		if (!ent->takedamage)
			continue;

		//ROGUE - make tesla hurt by bfg
		if (!(ent->svflags & SVF_MONSTER) && !(ent->svflags & SVF_DAMAGEABLE) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
//		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0)
//			&& (strcmp(ent->classname, "tesla") != 0))
			continue;

		VectorMA (ent->absmin, 0.5, ent->size, point);

		VectorSubtract (point, self->s.origin, dir);
		VectorNormalize (dir);

		ignore = self;
		VectorCopy (self->s.origin, start);
		VectorMA (start, 2048, dir, end);
		while (1)
		{
			tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

			if (!tr.ent)
				break;

			// hurt it if we can
			if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
				T_Damage (tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER);

			// if we hit something that's not a monster or player we're done
//			if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
			if (!(tr.ent->svflags & SVF_MONSTER) && !(tr.ent->svflags & SVF_DAMAGEABLE) && (!tr.ent->client))
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (4);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self->s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
				break;
			}

			ignore = tr.ent;
			VectorCopy (tr.endpos, start);
		}

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);

		// Lazarus reflections
		if (level.num_reflectors)
			ReflectTrail (TE_BFG_LASER, self->s.origin, tr.endpos);
	}

	self->nextthink = level.time + FRAMETIME;
}


void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	edict_t	*bfg;

	bfg = G_Spawn();
	VectorCopy (start, bfg->s.origin);
	VectorCopy (dir, bfg->movedir);
	vectoangles (dir, bfg->s.angles);
	VectorScale (dir, speed, bfg->velocity);
	bfg->movetype = MOVETYPE_FLYMISSILE;
	bfg->clipmask = MASK_SHOT;
	bfg->solid = SOLID_BBOX;
	bfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
	VectorClear (bfg->mins);
	VectorClear (bfg->maxs);
	bfg->s.modelindex = gi.modelindex ("sprites/s_bfg1.sp2");
	bfg->owner = self;
	bfg->touch = bfg_touch;
	bfg->nextthink = level.time + 8000.0f/speed;
	bfg->think = G_FreeEdict;
	bfg->radius_dmg = damage;
	bfg->dmg_radius = damage_radius;
	bfg->classname = "bfg blast";
	bfg->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");

	bfg->think = bfg_think;
	bfg->nextthink = level.time + FRAMETIME;
	bfg->teammaster = bfg;
	bfg->teamchain = NULL;

	if (self->client)
		check_dodge (self, bfg->s.origin, dir, speed);

	gi.linkentity (bfg);
}

// RAFAEL

/*
	fire_ionripper
*/

void ionripper_sparks (edict_t *self)
{
	byte	count, color;

	count = 0;
	color = 0xe4 + (rand()&3);

	// Knightmare- explode sound
	if (ion_ripper_extra_sounds->value)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/ionexp.wav"), 1, ATTN_NONE, 0);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WELDING_SPARKS);
	gi.WriteByte (count);	// 0
	gi.WritePosition (self->s.origin);
	gi.WriteDir (vec3_origin);
	gi.WriteByte (color);	// 0xe4 + (rand()&3)
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectWeldingSparks (count, color, self->s.origin, vec3_origin);

	G_FreeEdict (self);
}

// RAFAEL
void ionripper_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
			PlayerNoise (self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
	//Knightmare- hit sound
		if (ion_ripper_extra_sounds->value)
#ifdef KMQUAKE2_ENGINE_MOD
		{
			float r = random();
			if (r < 0.3333)
				gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/ionhit1.wav"), 1, ATTN_NONE, 0);
			else if (r < 0.6666)
				gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/ionhit2.wav"), 1, ATTN_NONE, 0);
			else
				gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/ionhit3.wav"), 1, ATTN_NONE, 0);
		}
#else
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/ionhit1.wav"), 1, ATTN_NONE, 0);
#endif
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_RIPPER);
	}
	else
	{
		return;
	}

	G_FreeEdict (self);
}


// RAFAEL
void fire_ionripper (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t *ion;
	trace_t tr;

	VectorNormalize (dir);

	ion = G_Spawn ();
	ion->classname = "ion";
	ion->class_id = ENTITY_ION;
	VectorCopy (start, ion->s.origin);
	VectorCopy (start, ion->s.old_origin);
	vectoangles (dir, ion->s.angles);
	VectorScale (dir, speed, ion->velocity);

	ion->movetype = MOVETYPE_WALLBOUNCE;
	ion->clipmask = MASK_SHOT;
	ion->solid = SOLID_BBOX;
	ion->s.effects |= effect;

	ion->s.renderfx |= RF_FULLBRIGHT | RF_NOSHADOW; // Knightmare- no shadow

	VectorClear (ion->mins);
	VectorClear (ion->maxs);
	ion->s.modelindex = gi.modelindex ("models/objects/boomrang/tris.md2");
	ion->s.sound = gi.soundindex ("misc/lasfly.wav");
	ion->owner = self;
	ion->touch = ionripper_touch;
	ion->nextthink = level.time + 3;
	ion->think = ionripper_sparks;
	ion->dmg = damage;
	ion->dmg_radius = 100;
	gi.linkentity (ion);

	if (self->client)
		check_dodge (self, ion->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, ion->s.origin, ion, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (ion->s.origin, -10, dir, ion->s.origin);
		ion->touch (ion, tr.ent, NULL, NULL);
	}

}

// NOTE: SP_ion should ONLY be used for ION Ripper projectiles that have
//       changed maps via trigger_transition. It should NOT be used for map
//       entities.
void ion_delayed_start (edict_t *ion)
{
	if (g_edicts[1].linkcount)
	{
		VectorScale(ion->movedir,ion->moveinfo.speed,ion->velocity);
		ion->nextthink = level.time + 3;
		ion->think = ionripper_sparks;
		gi.linkentity(ion);
	}
	else
		ion->nextthink = level.time + FRAMETIME;
}

void SP_ion (edict_t *ion)
{
	ion->s.modelindex = gi.modelindex ("models/objects/boomrang/tris.md2");
	ion->s.sound = gi.soundindex ("misc/lasfly.wav");
	ion->touch = ionripper_touch;
	VectorCopy(ion->velocity,ion->movedir);
	VectorNormalize(ion->movedir);
	ion->moveinfo.speed = VectorLength(ion->velocity);
	VectorClear(ion->velocity);
	ion->think = ion_delayed_start;
	ion->nextthink = level.time + FRAMETIME;
	gi.linkentity(ion);
}

// NOTE: the new Rogue fire_heat is in g_newweap.c
/*
fire_rocket_heat
*/
void rocket_heat_think (edict_t *self)
{
	edict_t		*target = NULL;
	edict_t		*aquire = NULL;
	vec3_t		vec;
	vec3_t		oldang;
	int			len;
	int			oldlen = 0;

	VectorClear (vec);

	// aquire new target
	while (( target = findradius (target, self->s.origin, 1024)) != NULL)
	{
		
		if (self->owner == target)
			continue;
		if (!target->svflags & SVF_MONSTER)
			continue;
		//If player fires this, do track monsters
		if (!self->owner->client && !target->client)
			continue;
		if (target->health <= 0)
			continue;
		if (!visible (self, target))
			continue;
		
		// if we need to reduce the tracking cone
		/*
		{
			vec3_t	vec;
			float	dot;
			vec3_t	forward;
	
			AngleVectors (self->s.angles, forward, NULL, NULL);
			VectorSubtract (target->s.origin, self->s.origin, vec);
			VectorNormalize (vec);
			dot = DotProduct (vec, forward);
	
			if (dot > 0.6)
				continue;
		}
		*/

		if (!infront (self, target))
			continue;

		VectorSubtract (self->s.origin, target->s.origin, vec);
		len = VectorLength (vec);

		if (aquire == NULL || len < oldlen)
		{
			aquire = target;
			self->target_ent = aquire;
			oldlen = len;
		}
	}

	if (aquire != NULL)
	{
		VectorCopy (self->s.angles, oldang);
		VectorSubtract (aquire->s.origin, self->s.origin, vec);
		
		vectoangles (vec, self->s.angles);
		
		VectorNormalize (vec);
		VectorCopy (vec, self->movedir);
		VectorScale (vec, 500, self->velocity);
	}

	self->nextthink = level.time + 0.1;
}

// RAFAEL
void fire_rocket_heat (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t *heat;

	heat = G_Spawn();
	heat->classname = "rocket";
	heat->class_id = ENTITY_ROCKET;
	VectorCopy (start, heat->s.origin);
	VectorCopy (dir, heat->movedir);
	vectoangles (dir, heat->s.angles);
	VectorScale (dir, speed, heat->velocity);
	heat->movetype = MOVETYPE_FLYMISSILE;
	heat->clipmask = MASK_SHOT;
	heat->solid = SOLID_BBOX;
	heat->s.effects |= EF_ROCKET;
	heat->s.renderfx |= RF_NOSHADOW; //Knightmare- no shadow
	VectorClear (heat->mins);
	VectorClear (heat->maxs);
//	heat->s.modelindex = gi.modelindex ("models/objects/hrocket/tris.md2");
	heat->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	heat->s.skinnum = 1;
	heat->owner = self;
	heat->touch = rocket_touch;

	heat->nextthink = level.time + 0.1;
	heat->think = rocket_heat_think;
	
	heat->dmg = damage;
	heat->radius_dmg = radius_damage;
	heat->dmg_radius = damage_radius;
	heat->s.sound = gi.soundindex ("weapons/rockfly.wav");

	if (self->client)
		check_dodge (self, heat->s.origin, dir, speed);

	gi.linkentity (heat);
}

// RAFAEL
/*
fire_phalanx_plasma
*/
void phalanx_plasma_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_PHALANX);
	}
	
	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_PHALANX_SPLASH);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_PLASMA_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
	// Lazarus reflections
	if (level.num_reflectors)
		ReflectExplosion (TE_PLASMA_EXPLOSION, ent->s.origin);

	G_FreeEdict (ent);
}

// Knightmare- remove this after the next savegame version increment!
void plasma_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	phalanx_plasma_touch (ent, other, plane, surf);
}

// RAFAEL
void fire_phalanx_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t *ph_plasma;

	ph_plasma = G_Spawn();
	ph_plasma->classname = "phalanx_plasma";
	ph_plasma->class_id = ENTITY_PLASMA;
	VectorCopy (start, ph_plasma->s.origin);
	VectorCopy (dir, ph_plasma->movedir);
	vectoangles (dir, ph_plasma->s.angles);
	VectorScale (dir, speed, ph_plasma->velocity);
	ph_plasma->movetype = MOVETYPE_FLYMISSILE;
	ph_plasma->clipmask = MASK_SHOT;
	ph_plasma->solid = SOLID_BBOX;

	VectorClear (ph_plasma->mins);
	VectorClear (ph_plasma->maxs);
	
	ph_plasma->owner = self;
	ph_plasma->touch = phalanx_plasma_touch;
	ph_plasma->nextthink = level.time + 8000.0f/speed;
	ph_plasma->think = G_FreeEdict;
	ph_plasma->dmg = damage;
	ph_plasma->radius_dmg = radius_damage;
	ph_plasma->dmg_radius = damage_radius;
	ph_plasma->s.sound = gi.soundindex ("weapons/rockfly.wav");
	
	ph_plasma->s.modelindex = gi.modelindex ("sprites/s_photon.sp2");
	ph_plasma->s.effects |= EF_PLASMA | EF_ANIM_ALLFAST;
	ph_plasma->s.renderfx |= RF_FULLBRIGHT;

	if (self->client)
		check_dodge (self, ph_plasma->s.origin, dir, speed);

	gi.linkentity (ph_plasma);
}

// NOTE: SP_phalanx_plasma should ONLY be used to spawn phalanx magslugs that change maps
//       via a trigger_transition. It should NOT be used for map entities.

void phalanx_plasma_delayed_start (edict_t *ph_plasma)
{
	if (g_edicts[1].linkcount)
	{
		VectorScale(ph_plasma->movedir, ph_plasma->moveinfo.speed, ph_plasma->velocity);
		ph_plasma->nextthink = level.time + 8000.0f / ph_plasma->moveinfo.speed;
		ph_plasma->think = G_FreeEdict;
		gi.linkentity(ph_plasma);
	}
	else
		ph_plasma->nextthink = level.time + FRAMETIME;
}

void SP_phalanx_plasma (edict_t *ph_plasma)
{
	vec3_t	dir;

	ph_plasma->s.modelindex = gi.modelindex ("sprites/s_photon.sp2");
	ph_plasma->s.effects |= EF_PLASMA | EF_ANIM_ALLFAST;
	ph_plasma->s.sound      = gi.soundindex ("weapons/rockfly.wav");
	ph_plasma->touch = phalanx_plasma_touch;
	AngleVectors(ph_plasma->s.angles, dir, NULL, NULL);
	VectorCopy (dir, ph_plasma->movedir);
	ph_plasma->moveinfo.speed = VectorLength(ph_plasma->velocity);
	if (ph_plasma->moveinfo.speed <= 0)
		ph_plasma->moveinfo.speed = 650;

	// For SP, freeze plasma until player spawns in
	if (game.maxclients == 1)
	{
		VectorClear(ph_plasma->velocity);
		ph_plasma->think = phalanx_plasma_delayed_start;
		ph_plasma->nextthink = level.time + FRAMETIME;
	}
	else
	{
		ph_plasma->think = G_FreeEdict;
		ph_plasma->nextthink = level.time + 8000.0f / ph_plasma->moveinfo.speed;
	}
	gi.linkentity (ph_plasma);
}

// RAFAEL
extern void SP_item_foodcube (edict_t *best);
// RAFAEL

extern int	gibsthisframe;
extern int lastgibframe;

/*static*/ void Trap_Think (edict_t *ent)
{
	edict_t	*target = NULL;
	edict_t	*best = NULL;
	vec3_t	vec;
	int		len, i;
	int		oldlen = 8000;
	//vec3_t	forward, right, up;
	
	if (ent->timestamp < level.time)
	{
		ent->s.frame = 6;
		//BecomeExplosion1(ent);
		// note to self
		// cause explosion damage???
		return;
	}
	
	ent->nextthink = level.time + 0.1;
	
	if (!ent->groundentity)
		return;

	// ok lets do the blood effect
	if (ent->s.frame > 4)
	{
		if (ent->s.frame == 5)
		{
			if (ent->wait == 64)
				gi.sound(ent, CHAN_VOICE, gi.soundindex ("weapons/trapdown.wav"), 1, ATTN_IDLE, 0);

			ent->wait -= 2;
			ent->delay += level.time;

			for (i=0; i<3; i++)
			{
				
				// Knightmare- forget this, enough gibs are spawned already
				// Lazarus: Prevent gib showers from causing SZ_GetSpace: overflow
			/*	if (level.framenum > lastgibframe)
				{
					gibsthisframe = 0;
					lastgibframe = level.framenum;
				}
				gibsthisframe++;
				if (gibsthisframe <= sv_maxgibs->value)
				{
					best = G_Spawn();

					//if (strcmp (ent->enemy->classname, "monster_gekk") == 0)
					if (ent->enemy->blood_type == 1)
					{
						best->s.modelindex = gi.modelindex ("models/objects/gekkgib/torso/tris.md2");	
					//	best->s.effects |= TE_GREENBLOOD;
						best->s.effects |= EF_GREENGIB|EF_BLASTER;
					}
					// Kngihtmare- added 
					else if (strcmp (ent->enemy->classname, "monster_fixbot") == 0)
					{
						best->s.modelindex = gi.modelindex ("models/objects/debris1/tris.md2");	
						best->s.effects |= EF_GRENADE;
					}
					else if (ent->enemy->mass > 200)
					{
						best->s.modelindex = gi.modelindex ("models/objects/gibs/chest/tris.md2");	
					//	best->s.effects |= TE_BLOOD;
						best->s.effects |= EF_GIB;
					}
					else
					{
						best->s.modelindex = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");	
					//	best->s.effects |= TE_BLOOD;
						best->s.effects |= EF_GIB;
					}

					AngleVectors (ent->s.angles, forward, right, up);
				
					RotatePointAroundVector( vec, up, right, ((360.0/3)* i)+ent->delay);
					VectorMA (vec, ent->wait/2, vec, vec);
					VectorAdd(vec, ent->s.origin, vec);
					VectorAdd(vec, forward, best->s.origin);
  
					best->s.origin[2] = ent->s.origin[2] + ent->wait;
				
					VectorCopy (ent->s.angles, best->s.angles);
  
					best->solid = SOLID_NOT;
					best->s.effects |= EF_GIB;
					best->takedamage = DAMAGE_YES;
		
					best->movetype = MOVETYPE_TOSS;
					best->svflags |= SVF_MONSTER;
					best->deadflag = DEAD_DEAD;
			  
					VectorClear (best->mins);
					VectorClear (best->maxs);

					best->watertype = gi.pointcontents(best->s.origin);
					if (best->watertype & MASK_WATER)
						best->waterlevel = 1;

					//best->nextthink = level.time + 0.1;
					best->nextthink = level.time + 10 + random()*10;
					//best->think = G_FreeEdict;
					best->think = gib_fade;
					gi.linkentity (best);
				} */

				best = G_Spawn ();
				VectorCopy (ent->s.origin, best->s.origin);
				best->s.origin[2]+= 32;
				SP_item_foodcube (best);
				best->velocity[2] = 400;
				best->count = 20; //was best->mass
				gi.linkentity (best);
				if (ent->timestamp < (level.time - 1)) //close if time is just about up
					ent->s.frame++;
				else //go back to previous state
					ent->s.frame = 4;
			}
				
			//if (ent->wait < 19)
			//	ent->s.frame++;

			return;
		}
		ent->s.frame ++;
		if (ent->s.frame == 8)
		{
			ent->nextthink = level.time + 1.0;
			ent->think = G_FreeEdict;

/*			best = G_Spawn ();
			SP_item_foodcube (best);
			VectorCopy (ent->s.origin, best->s.origin);
			best->s.origin[2]+= 16;
			best->velocity[2] = 400;
			best->count = ent->mass;
			gi.linkentity (best);
*/
			return;
		}
		return;
	}
	
	ent->s.effects &= ~EF_TRAP;
	if (ent->s.frame >= 4)
	{
		ent->s.effects |= EF_TRAP;
		VectorClear (ent->mins);
		VectorClear (ent->maxs);

	}

	if (ent->s.frame < 4)
		ent->s.frame++;

	while ((target = findradius(target, ent->s.origin, 256)) != NULL)
	{
		if (target == ent)
			continue;
		if (!(target->svflags & SVF_MONSTER) && !target->client)
			continue;
		// if (target == ent->owner)
		//	continue;
		if (target->health <= 0)
			continue;
		if (!visible (ent, target))
		 	continue;
		if (!best)
		{
			best = target;
			continue;
		}
		VectorSubtract (ent->s.origin, target->s.origin, vec);
		len = VectorLength (vec);
		if (len < oldlen)
		{
			oldlen = len;
			best = target;
		}
	}

	// pull the enemy in
	if (best)
	{
		vec3_t	forward;

		if (best->groundentity)
		{
			best->s.origin[2] += 1;
			best->groundentity = NULL;
		}
		VectorSubtract (ent->s.origin, best->s.origin, vec);
		len = VectorLength (vec);
		if (best->client)
		{
			VectorNormalize (vec);
			VectorMA (best->velocity, 250, vec, best->velocity);
		}
		else
		{
			best->ideal_yaw = vectoyaw(vec);	
			M_ChangeYaw (best);
			AngleVectors (best->s.angles, forward, NULL, NULL);
			VectorScale (forward, 256, best->velocity);
		}

		gi.sound(ent, CHAN_VOICE, gi.soundindex ("weapons/trapsuck.wav"), 1, ATTN_IDLE, 0);
		
		if (len < 32)
		{	
			if (best->mass <= 400)
			{
				T_Damage (best, ent, ent->owner, vec3_origin, best->s.origin, vec3_origin, 100000, 1, 0, MOD_TRAP);
				ent->enemy = best;
				ent->wait = 64;
				VectorCopy (ent->s.origin, ent->s.old_origin);
				ent->timestamp = level.time + 600;
				if (deathmatch->value)
					ent->mass = best->mass/4;
				else
					ent->mass = best->mass/10;
				// ok spawn the food cube
				ent->s.frame = 5;
			}
			else
			{
				BecomeExplosion1(ent);
				// note to self
				// cause explosion damage???
				return;
			}	
		}
	}	
}


// RAFAEL
void fire_trap (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*trap;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	trap = G_Spawn();
	trap->classname = "trap";
	VectorCopy (start, trap->s.origin);
	VectorScale (aimdir, speed, trap->velocity);
	VectorMA (trap->velocity, 200 + crandom() * 10.0, up, trap->velocity);
	VectorMA (trap->velocity, crandom() * 10.0, right, trap->velocity);
	// Knightmare- add player's base velocity to thrown trap
	if (add_velocity_throw->value && self->client)
		VectorAdd (trap->velocity, self->velocity, trap->velocity);
	else if (self->groundentity)
		VectorAdd (trap->velocity, self->groundentity->velocity, trap->velocity);

	VectorSet (trap->avelocity, 0, 300, 0);
	trap->movetype = MOVETYPE_BOUNCE;
	trap->clipmask = MASK_SHOT;
	trap->solid = SOLID_BBOX;
//	VectorClear (trap->mins);
//	VectorClear (trap->maxs);
	VectorSet (trap->mins, -4, -4, 0);
	VectorSet (trap->maxs, 4, 4, 8);
	trap->s.modelindex = gi.modelindex ("models/weapons/z_trap/tris.md2");
	trap->s.renderfx |= RF_IR_VISIBLE;
	trap->owner = self;
	trap->nextthink = level.time + 1.0;
	trap->think = Trap_Think;
	trap->dmg = damage;
	trap->dmg_radius = damage_radius;
	trap->classname = "htrap";
	trap->class_id = ENTITY_TRAP;
	// RAFAEL 16-APR-98
	trap->s.sound = gi.soundindex ("weapons/traploop.wav");
	// END 16-APR-98
	if (held)
		trap->spawnflags = 3;
	else
		trap->spawnflags = 1;
	
	trap->health = sk_trap_health->value;  
	trap->takedamage = DAMAGE_YES;
	trap->die = Trap_Die;

	if (timer <= 0.0)
		Grenade_Explode (trap);
	else
	{
		// gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/trapdown.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (trap);
	}
	trap->timestamp = level.time + sk_trap_life->value;
}

// NOTE: SP_trap should ONLY be used to spawn traps that change maps
//       via a trigger_transition. They should NOT be used for map entities.

void trap_delayed_start (edict_t *trap)
{
	if (g_edicts[1].linkcount)
	{
		VectorScale(trap->movedir,trap->moveinfo.speed,trap->velocity);
		trap->movetype  = MOVETYPE_BOUNCE;
		trap->nextthink = level.time + 1.0;
		trap->think = Trap_Think;
		gi.linkentity(trap);
	}
	else
		trap->nextthink = level.time + FRAMETIME;
}


void SP_trap (edict_t *trap)
{
	trap->s.modelindex = gi.modelindex ("models/weapons/z_trap/tris.md2");
	trap->s.sound = gi.soundindex ("weapons/traploop.wav");
	// For SP, freeze trap until player spawns in
	if (game.maxclients == 1)
	{
		trap->movetype  = MOVETYPE_NONE;
		VectorCopy(trap->velocity,trap->movedir);
		VectorNormalize(trap->movedir);
		trap->moveinfo.speed = VectorLength(trap->velocity);
		VectorClear(trap->velocity);
		trap->think     = trap_delayed_start;
		trap->nextthink = level.time + FRAMETIME;
	}
	else
	{
		trap->movetype  = MOVETYPE_BOUNCE;
		trap->nextthink = level.time + 1.0;
		trap->think = Trap_Think;
	}
	gi.linkentity (trap);
}


void Trap_Die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	Trap_Explode (self);
}


void Cmd_KillTrap_f (edict_t *ent)
{
	edict_t *blip = NULL;

	while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
	{
		if (!strcmp(blip->classname, "htrap") && blip->owner == ent)
		{
			blip->think = Trap_Explode;
			blip->nextthink = level.time + 0.1;
		}
	}
}


void Trap_Explode (edict_t *ent)
{
	vec3_t	origin;

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_GRENADE_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

//	Grenade_Explode(ent);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectExplosion (TE_GRENADE_EXPLOSION, origin);

	G_FreeEdict (ent);
}


/*
==============================================================================

Shockwave

==============================================================================
*/

void Nuke_Quake (edict_t *self);

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
	//remove after 5 secs
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
		{	//put origin of impact effect 16*i+1 units away from last hit surface
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
	impact->think = Nuke_Quake;
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


//==============================================================================
/*
 * M82 Plasma Rifle Source
 * Copyright (C) 1999  Team HOSTILE
 * Copyright (C) 1999  LMCTF 5.0
 * created by James "SWKiD" Tomaschke
 */

/*
==============================================================================
plasma_rifle_bounce_touch

If touched a hurtable object, hurt it.  Otherwise bounce with splash damage.
==============================================================================
*/
void plasma_rifle_bounce_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) 
{
	// sanity check
	if (!self || !other) {
		G_FreeEdict(self);
		return;
	}
	
	// If hit the sky, remove from world
	if (surf && (surf->flags & SURF_SKY)) {
		G_FreeEdict(self);
		return;
	}

	if ( self->owner->client )
		PlayerNoise (self->owner, self->s.origin, PNOISE_IMPACT);

	if ( other->takedamage ) 
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_PLASMA);	// Knightmare- use parm damage
			  
		// play hit sound
		gi.sound( self, CHAN_BODY, gi.soundindex(PLASMA_SOUND_HIT), 1, ATTN_IDLE, 0 );

		self->solid = SOLID_NOT;
		self->touch = NULL;
		VectorMA (self->s.origin, -1*FRAMETIME, self->velocity, self->s.origin);
		VectorClear (self->velocity);

		// Run Plasma Hit Animation
		self->s.modelindex = gi.modelindex (PLASMA_SPRITE_HIT);
		self->s.frame = 0;
		self->s.sound = 0;
		self->think = G_FreeEdict;
		self->nextthink = level.time + 0.1;
	}
	else
	{	// fx to do when it bounces off something
		// play reflection sound
		gi.sound ( self, CHAN_BODY, gi.soundindex(PLASMA_SOUND_BOUNCE), 1, ATTN_STATIC, 0 );

		// Draw blue sparks
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_LASER_SPARKS);
		gi.WriteByte (32);		// ammount
		gi.WritePosition (self->s.origin);
		gi.WriteDir (plane->normal);
		gi.WriteByte (176);		// stecki's choice of id's blue
		gi.multicast (self->s.origin, MULTICAST_PVS);

		// -bat
		T_RadiusDamage (self, self->owner, self->dmg, NULL, self->dmg+sk_plasma_rifle_radius->value, MOD_PLASMA);	// Knightmare- use parm damage and radius cvar
	}
}


/*
==============================================================================
plasma_rifle_spread_touch

If hits an entity, do some damage, otherwise do some splash damage.
==============================================================================
*/
void plasma_rifle_spread_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) 
{
	// sanity check
	if (!self || !other) {
		G_FreeEdict(self);
		return;
	}

	// If hit the sky, remove from world
	if (surf && (surf->flags & SURF_SKY)) {
		G_FreeEdict(self);
		return;
	}

	// Don't collide with other plasma goops
	if ( Q_stricmp(other->classname, "goop") == 0 )
		return;
	
	if ( self->owner->client )
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	// If can damage, hurt it
	if ( other->takedamage )
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_PLASMA);	// Knightmare- use parm damage
	}		  
	else	// otherwise, splash damage
	{		
		//-bat added sparks to this too.
		// Draw blue sparks
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_LASER_SPARKS);
		gi.WriteByte (32);		// ammount
		gi.WritePosition (self->s.origin);
		gi.WriteDir (plane->normal);
		gi.WriteByte (176);		// stecki's choice of id's blue
		gi.multicast (self->s.origin, MULTICAST_PVS);

		T_RadiusDamage (self, self->owner, self->dmg, NULL, self->dmg+sk_plasma_rifle_radius->value, MOD_PLASMA);	// Knightmare- use parm damage and radius cvar
	}		  
	// play hit sound
	gi.sound ( self, CHAN_BODY, gi.soundindex(PLASMA_SOUND_HIT), 1, ATTN_IDLE,0 );	// idle static none

	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorMA (self->s.origin, -1*FRAMETIME, self->velocity, self->s.origin);
	VectorClear (self->velocity);

	// Run Plasma Hit Animation
	self->s.modelindex = gi.modelindex (PLASMA_SPRITE_HIT);
	self->s.frame = 0;
	self->s.sound = 0;
	self->think = G_FreeEdict;
	self->nextthink = level.time + 0.1;
}


/*
==============================================================================
Spawn_Goop

Spawns the plasma entities, and defines values global to both weapon modes.
==============================================================================
*/
edict_t *Spawn_Goop (edict_t *ent, vec3_t start)
{
	edict_t *goop;	// = G_Spawn();

	// sanity check
	if (!ent) {
		return NULL;
	}

	goop = G_Spawn();

	goop->owner = ent;
	goop->clipmask = MASK_SHOT;
	goop->solid = SOLID_BBOX;
	goop->svflags = SVF_DEADMONSTER;
	
	VectorCopy (start, goop->s.origin);
	goop->classname = "goop";
	goop->class_id = ENTITY_GOOP;

	goop->s.effects |= EF_BLUEHYPERBLASTER | EF_ANIM_ALLFAST;
	// bat to get rid of the blue flag effect
//	goop->s.effects |= EF_IONRIPPER | EF_ANIM_ALLFAST;
	goop->s.renderfx = RF_TRANSLUCENT;
	goop->s.modelindex = gi.modelindex(PLASMA_SPRITE_FLY);
	goop->s.sound = gi.soundindex(PLASMA_SOUND_FLYBY);

	// give it some thickness for the bounce
//	VectorSet (goop->mins, -12, -12, -12);
//	VectorSet (goop->maxs, 12, 12, 12);
	VectorSet (goop->mins, -6, -6, -6);
	VectorSet (goop->maxs, 6, 6, 6);

	return goop;
}


/*
==============================================================================
fire_plasma_rifle_bounce

Unique code to fire a bouncy plasma goob.
Uses MOVETYPE_WALLBOUNCE.
==============================================================================
*/
void fire_plasma_rifle_bounce (edict_t *ent, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t *goop = NULL;

	// sanity check
	if (!ent) {
		return;
	}

	goop = Spawn_Goop (ent, start);
	if (!goop) {
		return;
	}

	goop->movetype = MOVETYPE_WALLBOUNCE;	// Knightmare- use same movetype as ION Ripper projectiles

	VectorScale (dir, speed, goop->velocity);		// Knightmare- use parm speed
	VectorCopy (goop->velocity, goop->s.angles);		// needed for post touch
	
	//-bat
	goop->dmg = damage;		// Knightmare- use parm damage
	goop->touch = plasma_rifle_bounce_touch;
	
	goop->think = G_FreeEdict;				// change this to handle
//	goop->nextthink = level.time + 3.0;		// sprite animation?
//	goop->nextthink = level.time + 1.5;
	goop->nextthink = level.time + sk_plasma_rifle_life_bounce->value;

	gi.linkentity (goop);

	// Knightmare- added missing check_dodge() call
	if (ent->client)
		check_dodge (ent, goop->s.origin, dir, speed);
}


/*
==============================================================================
fire_plasma_rifle_spread

Unique code to fire a spread of three bullets, each with 1/3 the damage of
one initial bouncy bullet.
==============================================================================
*/
void fire_plasma_rifle_spread (edict_t *ent, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*goop_l = NULL;
	edict_t *goop_c = NULL;
	edict_t	*goop_r = NULL;
	vec3_t	dir_r, dir_l, angles;

	// sanity check
	if (!ent) {
		return;
	}

	goop_l = Spawn_Goop (ent, start);
	goop_c = Spawn_Goop (ent, start);
	goop_r = Spawn_Goop (ent, start);
	if (!goop_l || !goop_c || !goop_r) {
		return;
	}

	goop_l->movetype = MOVETYPE_FLYMISSILE;
	goop_c->movetype = MOVETYPE_FLYMISSILE;
	goop_r->movetype = MOVETYPE_FLYMISSILE;

	VectorClear (goop_l->mins);
	VectorClear (goop_l->maxs);
	VectorClear (goop_c->mins);
	VectorClear (goop_c->maxs);
	VectorClear (goop_r->mins);
	VectorClear (goop_r->maxs);

	// Knightmare- use parm damage
	goop_l->dmg = damage;
	goop_c->dmg = damage;
	goop_r->dmg = damage;

	// center spread, line of sight
	VectorScale (dir, speed, goop_c->velocity);		// Knightmare- use parm speed
	vectoangles (dir, angles);

	// right spread, has 10+ in yaw
	angles[YAW] -= 10;
	AngleVectors (angles, dir_r, NULL, NULL);
	VectorScale (dir_r, speed, goop_r->velocity);		// Knightmare- use parm speed

	// left spread, has 10- in yaw
	angles[YAW] += 20;
	AngleVectors (angles, dir_l, NULL, NULL);
	VectorScale (dir_l, speed, goop_l->velocity);		// Knightmare- use parm speed

	goop_l->touch = plasma_rifle_spread_touch;
	goop_c->touch = plasma_rifle_spread_touch;
	goop_r->touch = plasma_rifle_spread_touch;
	
	goop_l->think = G_FreeEdict;
	goop_c->think = G_FreeEdict;
	goop_r->think = G_FreeEdict;
//	goop_l->nextthink = level.time + 3.0;
//	goop_c->nextthink = level.time + 3.0;
//	goop_r->nextthink = level.time + 3.0;
	goop_l->nextthink = level.time + sk_plasma_rifle_life_spread->value;
	goop_c->nextthink = level.time + sk_plasma_rifle_life_spread->value;
	goop_r->nextthink = level.time + sk_plasma_rifle_life_spread->value;

	gi.linkentity (goop_l);
	gi.linkentity (goop_c);
	gi.linkentity (goop_r);

	// Knightmare- added missing check_dodge() calls
	if (ent->client) {
		check_dodge (ent, goop_c->s.origin, dir, speed);
		check_dodge (ent, goop_r->s.origin, dir_r, speed);
		check_dodge (ent, goop_l->s.origin, dir_l, speed);
	}
}


/*
==============================================================================
fire_plasma_rifle

If "reflect" is > 0, it will fire a bouncy shot, else, it will fire a spread.
"start" and "dir" are not set here, but in 'p_weapon.c', where it is setup to
handle weapon firing.
==============================================================================
*/
void fire_plasma_rifle (edict_t *ent, vec3_t start, vec3_t dir, int damage, int speed, qboolean spread)
{
	if (spread) {
		fire_plasma_rifle_spread (ent, start, dir, damage, speed);
	}
	else {
		fire_plasma_rifle_bounce (ent, start, dir, damage, speed);
	}
}

// NOTE: SP_goop should ONLY be used to spawn plasma rifle shots that change maps
//       via a trigger_transition. It should NOT be used for map entities.

void goop_delayed_start (edict_t *goop)
{
	if (g_edicts[1].linkcount)
	{
		VectorScale(goop->movedir, goop->moveinfo.speed, goop->velocity);
		goop->nextthink = level.time + 8000/goop->moveinfo.speed;
		goop->think = G_FreeEdict;
		gi.linkentity(goop);
	}
	else
		goop->nextthink = level.time + FRAMETIME;
}

void SP_goop (edict_t *goop)
{
	vec3_t	dir;

	goop->s.modelindex = gi.modelindex(PLASMA_SPRITE_FLY);
	goop->s.effects |= EF_BLUEHYPERBLASTER | EF_ANIM_ALLFAST;
	goop->s.sound = gi.soundindex(PLASMA_SOUND_FLYBY);

	// set to the spread variant as we don't need a high speed bouncing projectile just after a map transition
	goop->touch = plasma_rifle_spread_touch;
	AngleVectors(goop->s.angles, dir, NULL, NULL);
	VectorCopy (dir, goop->movedir);
	goop->moveinfo.speed = VectorLength(goop->velocity);
	if (goop->moveinfo.speed <= 0)
		goop->moveinfo.speed = sk_plasma_rifle_speed_spread->value;	//	1200

	// For SP, freeze goop until player spawns in
	if (game.maxclients == 1)
	{
		VectorClear(goop->velocity);
		goop->think = goop_delayed_start;
		goop->nextthink = level.time + FRAMETIME;
	}
	else
	{
		goop->think = G_FreeEdict;
		goop->nextthink = level.time + 8000/goop->moveinfo.speed;
	}
	gi.linkentity (goop);
}
// end M82 Plasma Rifle