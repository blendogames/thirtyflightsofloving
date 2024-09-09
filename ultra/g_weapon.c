#include "g_local.h"
#include "aj_weaponbalancing.h" // AJ

#define	NUKE_QUAKE_TIME		3
#define NUKE_QUAKE_STRENGTH	100

/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/
// static void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed)
void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed)
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
	VectorMA (start, 512, dir, end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterinfo.dodge) && infront(tr.ent, self))
	{
		VectorSubtract (tr.endpos, start, v);
		eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
		tr.ent->monsterinfo.dodge (tr.ent, self, eta);
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
static void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
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

		// long traceline's eat CPU
		if (self->bot_client)
		{
			VectorMA (start, 1000, forward, end);
			r = (1/8) * r;
			r = (1/8) * u;
		}
		else {
			// Knightmare- adjust spread for expanded world size
#ifdef KMQUAKE2_ENGINE_MOD
			r *= (WORLD_SIZE / 8192);
			u *= (WORLD_SIZE / 8192);
#endif
			VectorMA (start, WORLD_SIZE, forward, end);	// was 8192
		}

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
				if (tr.contents & CONTENTS_WATER)
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

	if (self->owner->client)
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
		// Knightmare- blaster colors
		if (self->style == BLASTER_GREEN) //green
			tempevent = TE_BLASTER2;
		else if (self->style == BLASTER_BLUE) //blue
	#ifdef KMQUAKE2_ENGINE_MOD // Knightmare- looks better than flechette
			tempevent =  TE_BLUEHYPERBLASTER;
	#else
			tempevent = TE_FLECHETTE;
	#endif
	#ifdef KMQUAKE2_ENGINE_MOD
		else if (self->style == BLASTER_RED) //red
			tempevent =  TE_REDBLASTER;
	#endif
		else //standard yellow
			tempevent = TE_BLASTER;

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (tempevent);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void blue_blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

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
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_FLECHETTE);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper, int color)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
#ifdef KMQUAKE2_ENGINE_MOD
	bolt->s.renderfx |= RF_NOSHADOW;
#endif
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);

	// Knightmare- blaster colors
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
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	if (hyper)
		bolt->spawnflags = 1;
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
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
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);

	bolt->s.modelindex = gi.modelindex ("models/objects/blaser/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = blue_blaster_touch;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);

	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}

}

/*
=================
fire_grenade
=================
*/
void Grenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
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
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void ContactGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (other->takedamage)
		ent->enemy = other;

	Grenade_Explode (ent);
}

static void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
	{
		int	i;

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

		// look for bot's to alarm
		for (i=0; i<num_players; i++)
		{
			if ((players[i]->bot_client) && (players[i]->health > 0) && (entdist(players[i], ent) < 256))
			{
				players[i]->avoid_ent = ent;
			}
		}

		return;
	}

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
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
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
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
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
		gi.linkentity (grenade);
	}
}


/*
=================
fire_rocket
=================
*/
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
#ifdef KMQUAKE2_ENGINE_MOD
		if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
		{
			n = rand() % 5;
			while(n--)
				ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
		}
#else
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
#endif
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

void RocketInformDanger (edict_t *self)
{
	int	i;
	edict_t	*bot;
	vec3_t	org2, vec;
	float	dist;

	if (self->timestamp < (level.time - 5))
	{
		G_FreeEdict(self);
		return;
	}

	VectorMA(self->s.origin, 0.25, self->velocity, org2);
	// scan for bot's to inform
	for (i=0; i<num_players; i++)
	{
		bot = players[i];

		if (bot->bot_client && (bot->bot_stats->combat < 4))
			continue;

		if ((fabs(bot->s.origin[0] - self->s.origin[0]) > 300) ||
			(fabs(bot->s.origin[1] - self->s.origin[1]) > 300))
			continue;

		// make sure rocket is heading somewhat towards this bot
		VectorSubtract(org2, bot->s.origin, vec);
		if (((dist=entdist(bot, self)) - VectorLength(vec)) > 75)
		{	// yup, inform bot of danger
			bot->avoid_ent = self;
		}
	}
	self->nextthink = level.time + 0.3;
}


void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	rocket->classname = "rocket";
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET;
#ifdef KMQUAKE2_ENGINE_MOD
	rocket->s.renderfx |= RF_NOSHADOW;
#endif
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;

	// Eraser: Modified to scan for Bots to inform of danger
	rocket->timestamp = level.time;
	rocket->nextthink = level.time + 0.1;
	rocket->think = RocketInformDanger;

	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}


/*
=================
fire_rail
=================
*/
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, qboolean useColor, int red, int green, int blue)
{
	vec3_t		from, end, endpos = {0};
	trace_t		tr;
	edict_t		*ignore;
	int			mask, tempevent, i=0;
	qboolean	water;
	float		dist, inc=128;

	// Knightmare- changeable trail color
#ifdef KMQUAKE2_ENGINE_MOD
	if (useColor)
		tempevent = TE_RAILTRAIL_COLORED;
	else
#endif
		tempevent = TE_RAILTRAIL;

	// long tracelines chew up HUGE amounts of CPU, only do for client
	VectorMA (start, inc, aimdir, end);
	dist = inc;

	VectorCopy (start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
//	while (ignore || (dist < 2048))
	while ( (ignore || (dist < 4096)) && (i<256) )
	{
		tr = gi.trace (from, NULL, NULL, end, ignore, mask);

		if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
			water = true;
		}
		else
		{
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
				ignore = tr.ent;
			else
			{
				ignore = NULL;
				if (tr.fraction < 1)
					dist = 10000;
			}

			if ((tr.ent != self) && (tr.ent->takedamage))
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_RAILGUN);
		}

		VectorCopy (tr.endpos, from);
		VectorCopy (tr.endpos, endpos);

		VectorMA (from, inc, aimdir, end);
		dist += inc;
		i++;
	}

//	VectorSubtract (tr.endpos, start, end);
	VectorSubtract (endpos, start, end);
#ifndef KMQUAKE2_ENGINE_MOD
	if (VectorLength(end) > 2048)
	{
		VectorNormalize2 (end, end);
		VectorScale (end, 2048, end);
	}
#endif
	VectorAdd (start, end, end);

	// send gun puff / flash
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (tempevent);
	gi.WritePosition (start);
	gi.WritePosition (end);
#ifdef KMQUAKE2_ENGINE_MOD
	if (tempevent == TE_RAILTRAIL_COLORED) {
		gi.WriteByte (red);
		gi.WriteByte (green);
		gi.WriteByte (blue);
	}
#endif
	gi.multicast (self->s.origin, MULTICAST_PHS);
//	gi.multicast (start, MULTICAST_PHS);
	if (water)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (tempevent);
		gi.WritePosition (start);
		gi.WritePosition (end);
#ifdef KMQUAKE2_ENGINE_MOD
		if (tempevent == TE_RAILTRAIL_COLORED) {
			gi.WriteByte (red);
			gi.WriteByte (green);
			gi.WriteByte (blue);
		}
#endif
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
			if (ent == self->owner)
				points = points * 0.5;

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_EXPLOSION);
			gi.WritePosition (ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PHS);
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
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 200, 0, 0, MOD_BFG_BLAST);
	T_RadiusDamage(self, self->owner, 200, other, 100, MOD_BFG_BLAST);

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
		dmg = 5;
	else
		dmg = 10;

	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 256)) != NULL)
	{
		if (ent == self)
			continue;

		if (ent == self->owner)
			continue;

		if (!ent->takedamage)
			continue;

		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;

//ZOID
		//don't target players in CTF
		if (ctf->value && ent->client &&
			self->owner->client &&
			ent->client->resp.ctf_team == self->owner->client->resp.ctf_team)
			continue;
//ZOID

		VectorMA (ent->absmin, 0.5, ent->size, point);

		VectorSubtract (point, self->s.origin, dir);
		VectorNormalize (dir);

		ignore = self;
		VectorCopy (self->s.origin, start);
		VectorMA (start, 2048, dir, end);
		while(1)
		{
			tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

			if (!tr.ent)
				break;

			// hurt it if we can
			if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
				T_Damage (tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER);

			// if we hit something that's not a monster or player we're done
			if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
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
	bfg->nextthink = level.time + 8000/speed;
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
#ifdef NEW_ENTITY_STATE_MEMBERS
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
#ifdef NEW_ENTITY_STATE_MEMBERS
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
#ifdef NEW_ENTITY_STATE_MEMBERS
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
#ifdef NEW_ENTITY_STATE_MEMBERS
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
	int	i;
	edict_t	*bot;
	vec3_t	org2, vec;
	float	dist;

	sphere->avelocity[PITCH] = 80;
	sphere->avelocity[ROLL] = 80;

	VectorMA(sphere->s.origin, 0.25, sphere->velocity, org2);
	// scan for bots to inform
	for (i=0; i<num_players; i++)
	{
		bot = players[i];

		if (bot->bot_client && (bot->bot_stats->combat < 4))
			continue;

		if ((fabs(bot->s.origin[0] - sphere->s.origin[0]) > 300) ||
			(fabs(bot->s.origin[1] - sphere->s.origin[1]) > 300))
			continue;

		// make sure rocket is heading somewhat towards this bot
		VectorSubtract(org2, bot->s.origin, vec);
		if (((dist=entdist(bot, sphere)) - VectorLength(vec)) > 75)
		{	// yup, inform bot of danger
			bot->avoid_ent = sphere;
		}
	}
	sphere->nextthink = level.time + FRAMETIME;
}

void fire_shock_sphere (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*sphere;

	sphere = G_Spawn();
	sphere->classname = "shocksphere";
//	sphere->class_id = ENTITY_SHOCK_SPHERE;
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

#ifdef USE_AM_ROCKETS
/*
=================
fire_am_rocket
=================
*/
void am_rocket_quake (edict_t *self)
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
		e->velocity[0] += crandom()* 150;
		e->velocity[1] += crandom()* 150;
		e->velocity[2] = self->speed * (100.0 / e->mass);
	}

	G_FreeEdict (self);

}

void am_rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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

//	if (ent->owner->client)
//		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_AM_ROCKET);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}
	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_AM_ROCKET);

	gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/grenlx1a.wav"), 1, ATTN_NONE, 0);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1_BIG);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_NUKEBLAST);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_ALL);

	// become a quake
	ent->svflags |= SVF_NOCLIENT;
	ent->noise_index = gi.soundindex ("world/rumble.wav");
	ent->think = am_rocket_quake;
	ent->speed = NUKE_QUAKE_STRENGTH;
	ent->timestamp = level.time + NUKE_QUAKE_TIME;
	ent->nextthink = level.time + FRAMETIME;
	ent->last_move_time = 0;

//	gi.WritePosition (origin);
//	gi.multicast (ent->s.origin, MULTICAST_PHS);

//	G_FreeEdict (ent);
}

void fire_am_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	rocket->classname = "am_rocket";
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
//	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->s.modelindex = gi.modelindex ("models/objects/bomb/tris.md2");
	rocket->owner = self;
	rocket->touch = am_rocket_touch;

	// Eraser: Modified to scan for Bots to inform of danger
	rocket->timestamp = level.time;
	rocket->nextthink = level.time + 0.1;
	rocket->think = RocketInformDanger;

	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}
#endif	// USE_AM_ROCKETS
