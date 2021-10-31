// g_weapon.c

#include "g_local.h"

//CW++
void Fireball_CheckEnv(edict_t *self);
//CW--


/*
=================
Fire_Lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
static void Fire_Lead(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward;
	vec3_t		right;
	vec3_t		up;
	vec3_t		end;
	vec3_t		water_start;
	float		r;
	float		u;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;

	tr = gi.trace(self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles(aimdir, dir);
		AngleVectors(dir, forward, right, up);

		r = crandom() * hspread;
		u = crandom() * vspread;
		// Knightmare- adjust spread for expanded world size
#ifdef KMQUAKE2_ENGINE_MOD
		r *= (WORLD_SIZE / 8192);
		u *= (WORLD_SIZE / 8192);
#endif
		VectorMA(start, WORLD_SIZE, forward, end);	// was 8192.0
		VectorMA(end, r, right, end);
		VectorMA(end, u, up, end);

		if (gi.pointcontents(start) & MASK_WATER)
		{
			water = true;
			VectorCopy(start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace(start, NULL, NULL, end, self, content_mask);

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int color;

			water = true;
			VectorCopy(tr.endpos, water_start);

			if (!VectorCompare(start, tr.endpos))
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
					gi.WriteByte(svc_temp_entity);
					gi.WriteByte(TE_SPLASH);
					gi.WriteByte(8);
					gi.WritePosition(tr.endpos);
					gi.WriteDir(tr.plane.normal);
					gi.WriteByte(color);
					gi.multicast(tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				VectorSubtract(end, start, dir);
				vectoangles(dir, dir);
				AngleVectors(dir, forward, right, up);
				r = crandom() * hspread * 2.0;
				u = crandom() * vspread * 2.0;
				// Knightmare- adjust spread for expanded world size
#ifdef KMQUAKE2_ENGINE_MOD
				r *= (WORLD_SIZE / 8192);
				u *= (WORLD_SIZE / 8192);
#endif
				VectorMA(water_start, WORLD_SIZE, forward, end);	// was 8192.0
				VectorMA(end, r, right, end);
				VectorMA(end, u, up, end);
			}

			// re-trace ignoring water this time
			tr = gi.trace(water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if (!(tr.surface && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
//CW++
//			Destroy player's nearby Traps and C4 bundles (as gi.trace will ignore player's own entities).

			if (self->next_node)
			{
				edict_t		*check;
				edict_t		*index;
				qboolean	finished = false;

				index = self->next_node;
				while (index && !finished)
				{
					check = index;
					if (index->next_node)
						index = index->next_node;
					else
						finished = true;

					if (VecRange(tr.endpos, check->s.origin) < 10.0)
					{
						if (check->die == C4_DieFromDamage)
							C4_Die(check);
						else if (check->die == Trap_DieFromDamage)
							Trap_Die(check);
						else
							gi.dprintf("BUG: Invalid next_node pointer in Fire_Lead().\nPlease contact musashi@planetquake.com\n");
					}
				}
			}
//CW--
			if (tr.ent->takedamage)
				T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			else
			{
//CW++
				if (tr.surface && !(tr.surface->flags & SURF_SKY))
//CW--
				{
					gi.WriteByte(svc_temp_entity);
					gi.WriteByte(te_impact);
					gi.WritePosition(tr.endpos);
					gi.WriteDir(tr.plane.normal);
					gi.multicast(tr.endpos, MULTICAST_PVS);
//CW++
					if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
//CW--
				}
			}
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t pos;

		VectorSubtract(tr.endpos, water_start, dir);
		VectorNormalize(dir);
		VectorMA(tr.endpos, -2.0, dir, pos);
		if (gi.pointcontents(pos) & MASK_WATER)
			VectorCopy(pos, tr.endpos);
		else
			tr = gi.trace(pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd(water_start, tr.endpos, pos);
		VectorScale(pos, 0.5, pos);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BUBBLETRAIL2);																//CW
		gi.WritePosition(water_start);
		gi.WritePosition(tr.endpos);
		gi.multicast(pos, MULTICAST_PVS);
	}
}


/*
=================
Fire_Bullet

Fires a single round.
=================
*/
void Fire_Bullet(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
	Fire_Lead(self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
}


/*
=================
Fire_Shotgun

Shoots shotgun pellets.
=================
*/
void Fire_Shotgun(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
	int i;

	for (i = 0; i < count; i++)
		Fire_Lead(self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
}


/*
=================
Fire_Blaster

Fires a single blaster bolt.
=================
*/
void Blaster_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

//CW++
	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);
//CW--

	if (other->takedamage)
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_TARGET_BLASTER);	//CW
	else
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BLASTER);
		gi.WritePosition(self->s.origin);
		if (!plane)
			gi.WriteDir(vec3_origin);
		else
			gi.WriteDir(plane->normal);
		gi.multicast(self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict(self);
}

void Fire_Blaster(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect)	//CW
{
	edict_t	*bolt;
	trace_t	tr;

	bolt = G_Spawn();
	bolt->solid = SOLID_BBOX;
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->svflags = SVF_DEADMONSTER | SVF_PROJECTILE;												//CW
	bolt->s.effects |= effect;
	bolt->s.sound = gi.soundindex("misc/lasfly.wav");
	bolt->owner = self;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	bolt->touch = Blaster_Touch;
	bolt->nextthink = level.time + BLASTER_LIVETIME;												//CW
	bolt->think = G_FreeEdict;

	bolt->s.modelindex = gi.modelindex("models/objects/laser/tris.md2");
	VectorCopy(start, bolt->s.origin);
	VectorCopy(start, bolt->s.old_origin);
	vectoangles(aimdir, bolt->s.angles);
	VectorScale(aimdir, speed, bolt->velocity);

	gi.linkentity(bolt);

	tr = gi.trace(self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA(bolt->s.origin, -10.0, aimdir, bolt->s.origin);
		bolt->touch(bolt, tr.ent, NULL, NULL);
	}
}	


/*
=================
Fire_Rocket
=================
*/
//CW++
void Guided_Rocket_Think(edict_t *self)
{
//	Based on code by David Hyde.

	trace_t	tr;
	vec3_t	dir;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	end;
	float	speed;

//	Trace a line along player's current viewing direction; if it intersects a solid, reset
//	the target to be this endpoint. Otherwise, the target is the end of the line.

	AngleVectors(self->owner->client->v_angle, forward, right, NULL);
	VectorMA(self->owner->s.origin, WORLD_SIZE, forward, end);	// was 8192.0
	tr = gi.trace(self->owner->s.origin, NULL, NULL, end, self, MASK_SHOT);

//	Adjust rocket's velocity and angles to aim towards the target point.

	VectorSubtract(tr.endpos, self->s.origin, dir);
	VectorNormalize(dir);
	VectorAdd(dir, self->movedir, dir);
	VectorNormalize(dir);
	VectorCopy(dir, self->movedir);
	vectoangles(dir, self->s.angles);
	speed = VectorLength(self->velocity);
	VectorScale(dir, speed, self->velocity);
	gi.linkentity(self);

	self->nextthink = level.time + FRAMETIME;
}
//CW--

void Rocket_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	origin;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

	// calculate position for the explosion entity
	VectorMA(self->s.origin, -0.02, self->velocity, origin);

	if (other->takedamage)
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 0, 0, MOD_ROCKET);

	T_RadiusDamage(self, self->owner, self->radius_dmg, other, self->dmg_radius, MOD_R_SPLASH);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte((self->waterlevel)?TE_ROCKET_EXPLOSION_WATER:TE_ROCKET_EXPLOSION);
	gi.WritePosition(origin);
	gi.multicast(self->s.origin, MULTICAST_PHS);

//CW++
	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);
//CW--

	G_FreeEdict(self);
}

void Fire_Rocket(edict_t *self, vec3_t start, vec3_t aimdir, int damage, float speed, float damage_radius, int radius_damage, qboolean guided)	//CW
{
	edict_t	*rocket;

	rocket = G_Spawn();
	rocket->solid = SOLID_BBOX;
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->s.effects |= EF_ROCKET;
	rocket->owner = self;
	rocket->touch = Rocket_Touch;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex("weapons/rockfly.wav");
	rocket->classname = "rocket";

	rocket->s.modelindex = gi.modelindex("models/objects/rocket/tris.md2");
	VectorCopy(start, rocket->s.origin);
	VectorCopy(aimdir, rocket->movedir);
	vectoangles(aimdir, rocket->s.angles);
	VectorScale(aimdir, speed, rocket->velocity);

//CW++
	rocket->svflags = SVF_DEADMONSTER;
	rocket->wep_proj = true;

	if (!guided)
	{
		rocket->spawnflags = 0;
		rocket->svflags |= SVF_PROJECTILE;
		rocket->nextthink = level.time + (8000.0 / speed);
		rocket->think = G_FreeEdict;
	}
	else
	{
		rocket->spawnflags = 1;
		rocket->nextthink = level.time + FRAMETIME;
		rocket->think = Guided_Rocket_Think;
	}
//CW--

	gi.linkentity(rocket);
}


/*
=================
Fire_Rail
=================
*/
void Fire_Rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, qboolean useColor, int red, int green, int blue)
{
	vec3_t		from;
	vec3_t		end;
	trace_t		tr;
	edict_t		*ignore;
	int			mask, tempevent, i=0;
	qboolean	water;

	// Knightmare- changeable trail color
#ifdef KMQUAKE2_ENGINE_MOD
	if (useColor)
		tempevent = TE_RAILTRAIL_COLORED;
	else
#endif
		tempevent = TE_RAILTRAIL;

	VectorMA (start, WORLD_SIZE, aimdir, end);	// was 8192.0
	VectorCopy (start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT | CONTENTS_SLIME | CONTENTS_LAVA;
	while (ignore && i<256)
	{
		tr = gi.trace(from, NULL, NULL, end, ignore, mask);
		if (tr.contents & (CONTENTS_SLIME | CONTENTS_LAVA))
		{
			mask &= ~(CONTENTS_SLIME | CONTENTS_LAVA);
			water = true;
		}
		else
		{
			//ZOID--added so rail goes through SOLID_BBOX entities (gibs, etc)
			if (tr.ent->client || (tr.ent->solid == SOLID_BBOX) || (tr.ent->svflags & SVF_MONSTER))
				ignore = tr.ent;
			else
				ignore = NULL;

			if (tr.ent->takedamage && (tr.ent != self))
				T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_RAILGUN);
		}

		VectorCopy(tr.endpos, from);
		i++;
	}

//CW++
//	Destroy player's nearby Traps and C4 bundles (as gi.trace will ignore player's own entities).

	if (self->next_node)
	{
		edict_t		*check;
		edict_t		*index;
		qboolean	finished = false;

		index = self->next_node;
		while (index && !finished)
		{
			check = index;
			if (index->next_node)
				index = index->next_node;
			else
				finished = true;

			if (VecRange(tr.endpos, check->s.origin) < 10.0)
			{
				if (check->die == C4_DieFromDamage)
					C4_Die(check);
				else if (check->die == Trap_DieFromDamage)
					Trap_Die(check);
				else
					gi.dprintf("BUG: Invalid next_node pointer in Fire_Rail().\nPlease contact musashi@planetquake.com\n");
			}
		}
	}
//CW--

	// send gun puff / flash
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(tempevent);
	gi.WritePosition(start);
	gi.WritePosition(tr.endpos);
#ifdef KMQUAKE2_ENGINE_MOD
	if (tempevent == TE_RAILTRAIL_COLORED) {
		gi.WriteByte (red);
		gi.WriteByte (green);
		gi.WriteByte (blue);
	}
#endif
	gi.multicast(self->s.origin, MULTICAST_PHS);

	if (water)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(tempevent);
		gi.WritePosition(start);
		gi.WritePosition(tr.endpos);
#ifdef KMQUAKE2_ENGINE_MOD
		if (tempevent == TE_RAILTRAIL_COLORED) {
			gi.WriteByte (red);
			gi.WriteByte (green);
			gi.WriteByte (blue);
		}
#endif
		gi.multicast(tr.endpos, MULTICAST_PHS);
	}

//CW++
	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
//CW--
}


//CW++
//-------------------
// AWAKENING WEAPONS
//-------------------

/*
======================================================================
C4 subroutines

spawnflags (internal use only):
 1  = held too long by player
 4  = stuck to a rotating brush model
 8  = stuck to a moving brush model
16  = proximity fuse has been activated
32  = proximity fuse lifetime was exceeded
======================================================================
*/
void C4_Explode(edict_t *self)
{
	vec3_t	origin;
	int		mod;

	if (self->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd(self->enemy->mins, self->enemy->maxs, v);
		VectorMA(self->enemy->s.origin, 0.5, v, v);
		VectorSubtract(self->s.origin, v, v);
		points = self->dmg - (0.5 * VectorLength(v));
		VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
		T_Damage(self->enemy, self, self->owner, dir, self->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, MOD_C4);
	}

	if (self->spawnflags & 1)
		mod = MOD_C4_HELD;
	else if (self->spawnflags & 16)
		mod = MOD_C4_PROXIMITY;
	else if (self->spawnflags & 32)
		mod = MOD_C4_LIFETIME;
	else
		mod = MOD_C4;

	T_RadiusDamage(self, self->owner, self->dmg, self->enemy, self->dmg_radius, mod);

	VectorMA(self->s.origin, -0.02, self->velocity, origin);
	gi.WriteByte(svc_temp_entity);
	if (self->waterlevel)
		gi.WriteByte((self->groundentity)?TE_GRENADE_EXPLOSION_WATER:TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte((self->groundentity)?TE_GRENADE_EXPLOSION:TE_ROCKET_EXPLOSION);
	gi.WritePosition(origin);
	gi.multicast(self->s.origin, MULTICAST_PHS);

	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	TList_DelNode(self);
	G_FreeEdict(self);
}

void C4_ProximityCheck(edict_t *self)
{
	edict_t	*cl_ent = NULL;

//	If the proximity fuse lifetime has been exceeded, and hasn't yet been triggered, go BOOM!

	if (self->wait && (level.time >= self->wait) && !self->delay)
	{
		self->spawnflags = 32;
		C4_Explode(self);
		return;
	}

	self->nextthink = level.time + FRAMETIME;

//	Valid targets are live players who aren't our owner, or team members of our owner, within our 
//	detection range.

	while ((cl_ent = FindRadius(cl_ent, self->s.origin, sv_c4_proximity_range->value)) != NULL)
	{
		if (!cl_ent->client)
			continue;
		if (cl_ent == self->owner)
			continue;
		if (cl_ent->health < 1)
			continue;
		if (!visible(self, cl_ent))
			continue;
		if ((int)sv_gametype->value > G_FFA)
		{
			if (self->owner->client && (cl_ent->client->resp.ctf_team == self->owner->client->resp.ctf_team))
				continue;
		}

//		Target is valid, so trigger the fuse.
//		NB: Explosion-timing must be done differently if stuck to a moving brush model, as the think 
//		function is already taken by C4_MoveWithEnt().

		self->spawnflags |= 16;
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/c4/timer.wav"), 1, ATTN_NORM, 0);
		if (!(self->spawnflags & 4) && !(self->spawnflags & 8))
		{
			self->think = C4_Explode;
			self->nextthink = level.time + sv_c4_proximity_delay->value;
		}
		else
			self->delay = level.time + sv_c4_proximity_delay->value;

		break;
	}
}

void C4_Die(edict_t *self)
{
	int	n;

	if (self == NULL)
	{
		gi_centerprintf(self->owner, "BUG: C4_Die() called with null edict.\nPlease contact musashi@planetquake.com\n");
		return;
	}

	if (self->die != C4_DieFromDamage)
	{
		gi.dprintf("BUG: C4_Die() called for a non-trap edict.\n");
		gi.dprintf("     classname = %s\n", self->classname);
		if (self->owner && self->owner->client)
			gi.dprintf("     owner->name = %s\n", self->owner->client->pers.netname);

		return;
	}

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SPARKS);
	gi.WritePosition(self->s.origin);
	gi.WriteDir(vec3_origin);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	n = rand() % 4;
	while (n--)
		ThrowDebris(self, "models/objects/debris2/tris.md2", 2, self->s.origin);

	TList_DelNode(self);
	G_FreeEdict(self);
}

void C4_DieFromDamage(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	C4_Die(self);
}

void C4_CheckEnv(edict_t *self)
{
	vec3_t	point;
	vec3_t	puff_start;
	int		cont;

//	Pop if we detect slime or lava just below us (this makes the puff more visible compared with
//	checking the pointcontents at our actual origin).

	point[0] = self->s.origin[0];
	point[1] = self->s.origin[1];
	point[2] = self->s.origin[2] + self->mins[2];

	cont = gi.pointcontents(point);
	if (cont & (CONTENTS_LAVA | CONTENTS_SLIME))
	{
		puff_start[0] = self->s.origin[0];
		puff_start[1] = self->s.origin[1];
		puff_start[2] = self->s.origin[2] + 20.0;
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_HEATBEAM_SPARKS);
		gi.WritePosition(puff_start);
		gi.WriteDir(vec3_up);
		gi.multicast(puff_start, MULTICAST_PVS);
		C4_Die(self);
		return;
	}
	else if (cont & CONTENTS_SOLID)
	{
		TList_DelNode(self);
		G_FreeEdict(self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
}

void C4_RotateWithEnt(edict_t *self)
{
	edict_t	*t;
	float	vec1;
	float	vec2;
	float	radius;
	float	angle;

//	Sanity check.

	if (!self->target_ent)
	{
		gi_centerprintf(self->owner, "BUG: C4_RotateWithEnt() called with no target_ent.\nPlease contact musashi@planetquake.com\n");
		C4_Die(self);
		return;
	}
	
//	Adjust origin based on the axis of rotation.

	t = self->target_ent;
	if (t->movedir[0])			// y-axis
	{
		vec1 = self->s.origin[0] - t->s.origin[0];
		vec2 = self->s.origin[2] - t->s.origin[2];
		radius = sqrt(vec1*vec1 + vec2*vec2);
		angle = atan2(vec2, vec1) - DEG2RAD(t->avelocity[0] * FRAMETIME);
		self->s.origin[0] = t->s.origin[0] + (radius * cos(angle));
		self->s.origin[2] = t->s.origin[2] + (radius * sin(angle));
		self->s.angles[0] = -RAD2DEG(angle);
	}
	else if (t->movedir[1])		// z-axis
	{
		vec1 = self->s.origin[0] - t->s.origin[0];
		vec2 = self->s.origin[1] - t->s.origin[1];
		radius = sqrt(vec1*vec1 + vec2*vec2);
		angle = atan2(vec2, vec1) + DEG2RAD(t->avelocity[1] * FRAMETIME);
		self->s.origin[0] = t->s.origin[0] + (radius * cos(angle));
		self->s.origin[1] = t->s.origin[1] + (radius * sin(angle));
		self->s.angles[1] = RAD2DEG(angle);
	}
	else						// x-axis
	{
		vec1 = self->s.origin[1] - t->s.origin[1];
		vec2 = self->s.origin[2] - t->s.origin[2];
		radius = sqrt(vec1*vec1 + vec2*vec2);
		angle = atan2(vec2, vec1) + DEG2RAD(t->avelocity[2] * FRAMETIME);
		self->s.origin[1] = t->s.origin[1] + (radius * cos(angle));
		self->s.origin[2] = t->s.origin[2] + (radius * sin(angle));
		self->s.angles[2] = -RAD2DEG(angle);
	}
}

void C4_MoveWithEnt(edict_t *self)
{

//	Sanity check.

	if (!self->target_ent)
	{
		gi_centerprintf(self->owner, "BUG: C4_MoveWithEnt() called with no target_ent.\nPlease contact musashi@planetquake.com\n");
		C4_Die(self);
		return;
	}

//	If the proximity fuse has been activated, and the time is right, go BOOM!

	if ((self->spawnflags & 16) && (level.time >= self->delay))
	{
		C4_Explode(self);
		return;
	}

//	If the proximity fuse lifetime has been exceeded, and hasn't yet been triggered, go BOOM!

	if ((int)sv_c4_proximity->value && self->wait && (level.time >= self->wait) && !self->delay)
	{
		self->spawnflags = 32;
		C4_Explode(self);
		return;
	}

//	Handle C4 movement for the case that it's stuck to a moving brush model.

	if (self->spawnflags & 4)			// rotating brush model
		C4_RotateWithEnt(self);
	else if (self->spawnflags & 8)		// non-rotating brush model
		VectorAdd(self->target_ent->s.origin, self->move_origin, self->s.origin);

//	Destroy ourself if we're inside a solid (eg. scraped against the side of a lift shaft),
//	or in slime/lava.

	if (gi.pointcontents(self->s.origin) & (CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME))
	{
		C4_Die(self);
		return;
	}

//	Perform proximity detection if flagged to do so.

	if ((int)sv_c4_proximity->value && !(self->spawnflags & 16))
		C4_ProximityCheck(self);

	self->nextthink = level.time + FRAMETIME;
	gi.linkentity(self);
}

void C4_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*ent = NULL;

//	If the other entity has been freed (eg. as happens when two touchbang C4s touch each other), 
//	then don't bother running the rest of the touch function (see next code fragment).

	if (!other->inuse)
		return;

//	Pop if we hit another C4.

	if (other->die == C4_DieFromDamage)
	{
		C4_Die(self);
		return;
	}

//	Ignore weapon projectiles.

	if (other->wep_proj)
		return;

//	Don't explode if it's our owner who is doing the touching.

	if (other == self->owner)
		return;

//	Don't explode if we're a touchbang C4 and it's a team-mate of our owner who is doing the 
//	touching, as long as we've already landed on the ground.

	if ((int)sv_c4_touchbang->value && self->show_hostile && ((int)sv_gametype->value > G_FFA))
	{
		if (self->owner->client && other->client && (other->client->resp.ctf_team == self->owner->client->resp.ctf_team))
			return;
	}

//	Vanish if we hit a sky surface.

	if (surf && (surf->flags & SURF_SKY))
	{
		TList_DelNode(self);
		G_FreeEdict(self);
		return;
	}

//	Don't explode if we hit something that can't be damaged (eg. the floor). Instead, stick to it.

	if (!other->takedamage)
	{

//		First, a final check that we're not in slime or lava.

		if (gi.pointcontents(self->s.origin) & (CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME))
		{
			C4_Die(self);
			return;
		}

//		Next, check that we're not too close to a spawn point or teleporter destination
//		(just for proximity and touch-sensitive C4s).

		if (((int)sv_c4_proximity->value || (int)sv_c4_touchbang->value) && ((int)sv_c4_spawn_range->value > 0))
		{
			while ((ent = FindPointRadius(ent, self->s.origin, sv_c4_spawn_range->value)) != NULL)
			{
				if (ent->client)
					continue;
				if (ent->wep_proj)
					continue;
				if (!ent->classname)
					continue;
				if (ent->style == ENT_ID_INTERMISSION)
					continue;

				if ((ent->style == ENT_ID_PLAYER_SPAWN) || (ent->style == ENT_ID_TELE_DEST))
				{
					C4_Die(self);
					return;
				}
			}
		}

//		Seems OK, so let's get on with the sticking!

		self->s.effects = 0;
		VectorMA(self->s.origin, -0.001, self->velocity, self->s.origin);
		VectorClear(self->velocity);
		VectorClear(self->avelocity);
		self->movetype = MOVETYPE_NONE;
		self->show_hostile = true;			// abuse flag

		VectorClear(self->s.angles);
		if (plane->normal)
		{
			if ((plane->normal[2] > 0.8) || (plane->normal[2] < -0.8))
				self->s.angles[0] += 90.0;
		}

//		If we hit a brush model that might be in motion, then move with it.

		if ((other->blocked == plat_blocked) || (other->blocked == door_blocked) || (other->blocked == train_blocked) || (other->blocked == rotating_blocked))
		{
			VectorSubtract(self->s.origin, other->s.origin, self->move_origin);
			self->target_ent = other;
			self->think = C4_MoveWithEnt;
			self->nextthink = level.time + FRAMETIME;

			if ((other->blocked == rotating_blocked) || (other->style == ENT_ID_DOOR_ROTATING))
			{
				self->spawnflags |= 4;
				if (other->movedir[0] || other->movedir[2])
					self->s.angles[0] = 0;
			}
			else
				self->spawnflags |= 8;
		}

//		Start scanning for nearby enemies if proximity detection is flagged, unless stuck
//		to a moving brush model (function will be called separately).

		if ((int)sv_c4_proximity->value)
		{
			if (!(self->spawnflags & 4) && !(self->spawnflags & 8))
			{
				self->think = C4_ProximityCheck;
				self->nextthink = level.time + FRAMETIME;
			}

//			Proximity C4 bundles have a limited lifetime, if flagged.

			if (sv_c4_proximity_life->value)
				self->wait = level.time + sv_c4_proximity_life->value;
		}

		gi.linkentity(self);

		if (random() > 0.5)
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/c4/bounce1.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/c4/bounce2.wav"), 1, ATTN_NORM, 0);
	}

//	else go BOOM!

	else
	{
		self->enemy = other;
		C4_Explode(self);
	}

//	A stickied C4 bundle will now only explode on contact if flagged to do so.

	if (!(int)sv_c4_touchbang->value)
		self->touch = NULL;
}

void Fire_C4(edict_t *self, vec3_t start, vec3_t aimdir, int damage, float speed, float damage_radius, qboolean held)
{
	edict_t	*c4;
	vec3_t	dir;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	up;

	c4 = G_Spawn();
	c4->solid = SOLID_BBOX;
	c4->movetype = MOVETYPE_TOSS;
	c4->takedamage = DAMAGE_YES;
	c4->clipmask = MASK_SHOT;
	c4->monsterinfo.aiflags = AI_NOSTEP;
	c4->health = 1;
	c4->mass = 2;
	c4->s.effects |= EF_GRENADE;
	c4->dmg = damage;
	c4->dmg_radius = damage_radius;
	c4->classname = "c4";
	c4->owner = self;
	c4->die = C4_DieFromDamage;
	c4->touch = C4_Touch;
	c4->think = C4_CheckEnv;
	c4->nextthink = level.time + FRAMETIME;

	c4->model = "models/objects/grenade/tris.md2";
	c4->s.modelindex = gi.modelindex(c4->model);
	VectorSet(c4->mins, -3.0, -3.0, -3.0);
	VectorSet(c4->maxs, 3.0, 3.0, 3.0);

	if ((int)sv_gametype->value > G_FFA)
	{
		if (self->client->resp.ctf_team == CTF_TEAM1)
			c4->s.skinnum = 1;
		else if (self->client->resp.ctf_team == CTF_TEAM2)
			c4->s.skinnum = 2;
	}

	vectoangles(aimdir, dir);
	AngleVectors(dir, forward, right, up);
	VectorCopy(start, c4->s.origin);
	VectorScale(aimdir, speed, c4->velocity);
	VectorMA(c4->velocity, 200.0, up, c4->velocity);
	VectorSet(c4->avelocity, 100.0, 100.0, 100.0);

	TList_AddNode(c4);

	if (held)
	{
		c4->spawnflags = 1;
		C4_Explode(c4);
	}
	else
		gi.linkentity(c4);
}

/*
======================================================================
TRAPS subroutines

spawnflags (internal use only):
 1  = held too long by player, so auto-tractoring them
 2  = tractor beam pulling power is zero (apply damage only)
 4  = stuck to a rotating brush model [*]
 8  = stuck to a moving brush model
16  = proximity-detection has been activated
32  = hook has been fired at enemy
64  = tractor beam has been activated
128 = [*] on positive radial surface
256 = [*] on negative radial surface
512 = [*] on circumferential surface
======================================================================
*/
void Trap_CheckEnv(edict_t *self);

void Trap_TractorBeam(edict_t *self)
{
	vec3_t	start;
	vec3_t	end;
	vec3_t	dir;
	int		power;
	int		mod;

//	Sanity checks.

	if (!self->enemy)
	{
		gi_centerprintf(self->owner, "BUG: Trap_TractorBeam() called with no enemy.\nPlease contact musashi@planetquake.com\n");
		Trap_Die(self);
		return;
	}

	if (!self->enemy->inuse)
	{
		Trap_Die(self);
		return;
	}

//	Draw a laser beam between the Trap and the player who's caught. Apply damage every frame, but only
//	play the ripping sound once per second. Apply a momentum change to the target if flagged to do so.

	VectorCopy(self->s.origin, start);
	VectorCopy(self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight - 8.0;

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_BFG_LASER);
	gi.WritePosition(start);
	gi.WritePosition(end);
	gi.multicast(start, MULTICAST_PHS);

	VectorSubtract(start, end, dir);
	mod = (self->spawnflags & 1) ? MOD_TRAP_HELD : MOD_TRAP;
	power = (self->spawnflags & 2) ? 0 : self->count;

	T_Damage(self->enemy, self, self->owner, dir, end, vec3_origin, self->radius_dmg, power, 0, mod);
	if (level.time > self->wait)
	{
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/hook/rip.wav"), 1, ATTN_NORM, 0);
		self->wait = level.time + TRAP_DMG_RATE;
	}

	self->nextthink = level.time + FRAMETIME;

	if (self->spawnflags & 1)
		Trap_CheckEnv(self);
}

void Trap_Hook_Die(edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/hook/hit_wall.wav"), 1, ATTN_NORM, 0);
	Trap_Die(self->owner);
	self->touch = NULL;
	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
}

void Trap_Hook_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean hit_fake = false;

//	Ignore the Trap that fired us.

	if (other == self->owner)
		return;

//	Ignore weapon projectiles.

	if (other->wep_proj)
		return;

//	Destroy ourself and our parent Trap if...

//	...our parent Trap has been destroyed in the meantime.

	if (!self->owner->inuse)		// prevents server crashes
	{
		if (other->takedamage)
		{
			T_Damage(other, self, self->goalentity, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_TRAP);
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/hook/hit_body.wav"), 1, ATTN_NORM, 0);
		}
		else
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/hook/hit_wall.wav"), 1, ATTN_NORM, 0);

		self->think = G_FreeEdict;
		self->nextthink = level.time + FRAMETIME;
		self->touch = NULL;
		return;
	}

//	...we hit someone who's invulnerable.

	if (other->client && (other->client->invincible_framenum > level.framenum))
	{
		Trap_Hook_Die(self);
		return;
	}

//	...we hit a sky surface.

	if (surf && (surf->flags & SURF_SKY))
	{
		Trap_Hook_Die(self);
		return;
	}

//	...we hit something that can't be damaged (eg. the floor).

	if (!other->takedamage)
	{
		Trap_Hook_Die(self);
		return;
	}

//	...we hit a dead body.

	if (other->health < 1)
	{
		T_Damage(other, self, self->goalentity, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_TRAP);
		Trap_Hook_Die(self);
		return;
	}

//	... we hit team members of our parent Trap's owner in CTF/TDM/ASLT.

	if ((int)sv_gametype->value > G_FFA)
	{
		if (other->client && self->owner->owner->client && (other->client->resp.ctf_team == self->owner->owner->client->resp.ctf_team))
		{
			Trap_Hook_Die(self);
			return;
		}
	}
	
//	Mark the entity as being the target of the parent Trap's tractor beam if they're a player.

	if (other->client)
	{
		other->tractored = true;
		self->owner->enemy = other;
		self->owner->wait = level.time + TRAP_DMG_RATE;
		self->owner->nextthink = level.time + FRAMETIME;

		if (!(self->owner->spawnflags & 4) && !(self->owner->spawnflags & 8))
			self->owner->think = Trap_TractorBeam;
		else
			self->owner->spawnflags |= 64;

		if (other->target_ent && other->target_ent->client && other->target_ent->client->spycam)
			hit_fake = true;
	}

	T_Damage(other, self, self->goalentity, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_TRAP);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/hook/hit_body.wav"), 1, ATTN_NORM, 0);

//	If we've hit something damage-able that isn't a player, pop our parent Trap.

	if (!other->client && !hit_fake)
		Trap_Die(self->owner);

//	Vanish ("like an old oak table").

	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
	self->touch = NULL;
}

void Fire_Trap_Hook(edict_t *self, vec3_t start, vec3_t aimdir, int damage, float speed)
{
	edict_t	*hook;

	hook = G_Spawn();
	hook->solid = SOLID_BBOX;
	hook->movetype = MOVETYPE_FLYMISSILE;
	hook->clipmask = MASK_SHOT;
	hook->svflags = SVF_DEADMONSTER | SVF_PROJECTILE;
	hook->s.effects |= EF_GREENGIB;
	hook->s.renderfx = RF_FULLBRIGHT;
	hook->dmg = damage;
	hook->classname = "trap_hook";
	hook->wep_proj = true;
	hook->owner = self;
	hook->goalentity = self->owner;
	hook->touch = Trap_Hook_Touch;

	hook->model = "models/objects/hook/tris.md2";
	hook->s.modelindex = hook_index;

	VectorCopy(start, hook->s.origin);
	VectorCopy(start, hook->s.old_origin);
	vectoangles(aimdir, hook->s.angles);
	VectorScale(aimdir, speed, hook->velocity);

	gi.linkentity(hook);
}

void Trap_ShootHook(edict_t *self)
{
	vec3_t	end;
	vec3_t	dir;
	float	time;

	VectorCopy(self->enemy->s.origin, end);

//	Lead the target.

	VectorSubtract(end, self->s.origin, dir);
	time = VectorLength(dir) / sv_trap_hook_speed->value;
	end[0] += (time * self->enemy->velocity[0]);
	end[1] += (time * self->enemy->velocity[1]);
	VectorSubtract(end, self->s.origin, dir);
	VectorNormalize(dir);

	gi.sound(self, CHAN_WEAPON, gi.soundindex("makron/blaster.wav"), 1, ATTN_NORM, 0);
	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_WEAPON);

	Fire_Trap_Hook(self, self->s.origin, dir, self->dmg, sv_trap_hook_speed->value);
}

void Trap_ProximityCheck(edict_t *self)
{
	edict_t	*ent = NULL;

	while ((ent = FindRadius(ent, self->s.origin, sv_trap_proximity_range->value)) != NULL)
	{
//		Don't target ourself.

		if (ent == self)
			continue;

//		Don't target noclipped players.

		if (ent->movetype == MOVETYPE_NOCLIP)
			continue;

//		Don't target team members of our owner in CTF/TDM/ASLT, or their Traps and C4 bundles,
//		or our owner's.

		if ((int)sv_gametype->value > G_FFA)
		{
			if (ent->client && self->owner->client && (ent->client->resp.ctf_team == self->owner->client->resp.ctf_team))
				continue;
		}

		if ((ent->die == Trap_DieFromDamage) || (ent->die == C4_DieFromDamage))
		{
			if (ent->owner == self->owner)
				continue;

			if ((int)sv_gametype->value > G_FFA)
			{
				if (ent->owner->client && self->owner->client && (ent->owner->client->resp.ctf_team == self->owner->client->resp.ctf_team))
					continue;
			}
		}

//		Target any visible enemy players, Traps or C4s (apart from ourself or our owner).

		if (visible(self, ent) && ent->takedamage && (ent != self->owner) && (ent->health > 0))
		{
			if (!ent->client && (ent->die != Trap_DieFromDamage) && (ent->die != C4_DieFromDamage))
				continue;

			self->enemy = ent;
			if (!(self->spawnflags & 4) && !(self->spawnflags & 8))
				self->think = Trap_ShootHook;

			break;
		}
	}
	self->nextthink = level.time + FRAMETIME;
}

void Trap_Arm_ProximityCheck(edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/trap/timer.wav"), 1, ATTN_NORM, 0);
	self->think = Trap_ProximityCheck;
	self->nextthink = level.time + FRAMETIME;
}

void Trap_Die(edict_t *self)
{
	int	n;

	if (self == NULL)
	{
		gi_centerprintf(self->owner, "BUG: Trap_Die() called with null edict.\nPlease contact musashi@planetquake.com\n");
		return;
	}

	if (self->die != Trap_DieFromDamage)
	{
		gi.dprintf("BUG: Trap_Die() called for a non-trap edict.\n");
		gi.dprintf("     classname = %s\n", self->classname);
		if (self->owner && self->owner->client)
			gi.dprintf("     owner->name = %s\n", self->owner->client->pers.netname);

		return;
	}

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SPARKS);
	gi.WritePosition(self->s.origin);
	gi.WriteDir(vec3_origin);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	n = rand() % 4;
	while (n--)
		ThrowDebris(self, "models/objects/debris2/tris.md2", 2, self->s.origin);

	if (self->enemy)
		self->enemy->tractored = false;

	TList_DelNode(self);
	G_FreeEdict(self);
}

void Trap_DieFromDamage(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	Trap_Die(self);
}

void Trap_CheckEnv(edict_t *self)
{
	vec3_t	point;
	vec3_t	puff_start;
	int		cont;

//	Pop if we detect slime or lava just below us (this makes the puff more visible compared with
//	checking the pointcontents at our actual origin).

	point[0] = self->s.origin[0];
	point[1] = self->s.origin[1];
	point[2] = self->s.origin[2] + self->mins[2];

	cont = gi.pointcontents(point);
	if (cont & (CONTENTS_LAVA | CONTENTS_SLIME))
	{
		puff_start[0] = self->s.origin[0];
		puff_start[1] = self->s.origin[1];
		puff_start[2] = self->s.origin[2] + 20.0;
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_HEATBEAM_SPARKS);
		gi.WritePosition(puff_start);
		gi.WriteDir(vec3_up);
		gi.multicast(puff_start, MULTICAST_PVS);
		Trap_Die(self);
		return;
	}
	else if (cont & CONTENTS_SOLID)
	{
		TList_DelNode(self);
		G_FreeEdict(self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
}

void Trap_RotateWithEnt(edict_t *self)
{
	edict_t	*t;
	float	vec1;
	float	vec2;
	float	radius;
	float	angle;

//	Sanity check.

	if (!self->target_ent)
	{
		gi_centerprintf(self->owner, "BUG: Trap_RotateWithEnt() called with no target_ent.\nPlease contact musashi@planetquake.com\n");
		Trap_Die(self);
		return;
	}

//	Adjust origin based on the axis of rotation.

	t = self->target_ent;

	if (t->movedir[0])			// y-axis
	{
		vec1 = self->s.origin[0] - t->s.origin[0];
		vec2 = self->s.origin[2] - t->s.origin[2];
		radius = sqrt(vec1*vec1 + vec2*vec2);
		angle = atan2(vec2, vec1) - DEG2RAD(t->avelocity[0] * FRAMETIME);
		self->s.origin[0] = t->s.origin[0] + (radius * cos(angle));
		self->s.origin[2] = t->s.origin[2] + (radius * sin(angle));

		if (self->spawnflags & 128)
			self->s.angles[2] = RAD2DEG(angle);
		else if (self->spawnflags & 256)
			self->s.angles[2] = -RAD2DEG(angle);
		else if (self->spawnflags & 512)
		{
			trace_t tr = gi.trace(self->s.origin, NULL, NULL, t->s.origin, self, MASK_SOLID);
			if (tr.surface)
				vectoangles(tr.plane.normal, self->s.angles);
		}
	}
	else if (t->movedir[1])		// z-axis
	{
		vec1 = self->s.origin[0] - t->s.origin[0];
		vec2 = self->s.origin[1] - t->s.origin[1];
		radius = sqrt(vec1*vec1 + vec2*vec2);
		angle = atan2(vec2, vec1) + DEG2RAD(t->avelocity[1] * FRAMETIME);
		self->s.origin[0] = t->s.origin[0] + (radius * cos(angle));
		self->s.origin[1] = t->s.origin[1] + (radius * sin(angle));

		if (self->spawnflags & 128)
			self->s.angles[1] = RAD2DEG(angle);
		else if (self->spawnflags & 256)
			self->s.angles[1] = RAD2DEG(angle);
		else if (self->spawnflags & 512)
		{
			trace_t tr = gi.trace(self->s.origin, NULL, NULL, t->s.origin, self, MASK_SOLID);
			if (tr.surface)
				vectoangles(tr.plane.normal, self->s.angles);
		}
	}
	else						// x-axis
	{
		vec1 = self->s.origin[1] - t->s.origin[1];
		vec2 = self->s.origin[2] - t->s.origin[2];
		radius = sqrt(vec1*vec1 + vec2*vec2);
		angle = atan2(vec2, vec1) + DEG2RAD(t->avelocity[2] * FRAMETIME);
		self->s.origin[1] = t->s.origin[1] + (radius * cos(angle));
		self->s.origin[2] = t->s.origin[2] + (radius * sin(angle));

		if (self->spawnflags & 128)
			self->s.angles[2] = -RAD2DEG(angle);
		else if (self->spawnflags & 256)
			self->s.angles[2] = RAD2DEG(angle);
		else if (self->spawnflags & 512)
		{
			trace_t tr = gi.trace(self->s.origin, NULL, NULL, t->s.origin, self, MASK_SOLID);
			if (tr.surface)
				vectoangles(tr.plane.normal, self->s.angles);
		}
	}
}

void Trap_MoveWithEnt(edict_t *self)
{
//	Sanity check.

	if (!self->target_ent)
	{
		gi_centerprintf(self->owner, "BUG: Trap_MoveWithEnt() called with no target_ent.\nPlease contact musashi@planetquake.com\n");
		Trap_Die(self);
		return;
	}

//	Handle Trap movement for the case that it's stuck to a moving brush model.

	if (self->spawnflags & 4)			// rotating brush model
		Trap_RotateWithEnt(self);
	else if (self->spawnflags & 8)		// non-rotating brush model 
		VectorAdd(self->target_ent->s.origin, self->move_origin, self->s.origin);

//	Destroy ourself if we're inside a solid (eg. scraped against the side of a lift shaft),
//	or in slime/lava.

	if (gi.pointcontents(self->s.origin) & (CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME))
	{
		Trap_Die(self);
		return;
	}

	gi.linkentity(self);

//	Apply tractor beam effects if flagged to do so.

	if (self->spawnflags & 64)
	{
		Trap_TractorBeam(self);
		return;
	}

	if (self->spawnflags & 32)
		return;

//	Check for nearby enemies if proximity detection has been activated, otherwise call 
//	Trap_Arm_ProximityCheck() at the appropriate time.

	if (self->spawnflags & 16)
		Trap_ProximityCheck(self);
	else
	{
		if (level.time >= self->delay)
		{
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/trap/timer.wav"), 1, ATTN_NORM, 0);
			self->spawnflags |= 16;
		}
	}

//	Launch hook if a target has been acquired (and the hook hasn't already been fired).

	if (self->enemy && !(self->spawnflags & 32))
	{
		Trap_ShootHook(self);
		self->spawnflags |= 32;
	}

	self->nextthink = level.time + FRAMETIME;
}

void Trap_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{

//	Pop if we hit another Trap or C4 bundle.

	if ((other->die == Trap_DieFromDamage) || (other->die == C4_DieFromDamage))
	{
		Trap_Die(self);
		return;
	}

//	Ignore weapon projectiles.

	if (other->wep_proj)
		return;

//	Don't stick to our owner.

	if (other == self->owner)
		return;

//	Vanish if we hit a sky surface.

	if (surf && (surf->flags & SURF_SKY))
	{
		TList_DelNode(self);
		G_FreeEdict(self);
		return;
	}

//	If we hit something that can't be damaged (eg. the floor), stick to it.

	if (!other->takedamage)
	{

//		First, a final check that we're not in slime or lava.

		if (gi.pointcontents(self->s.origin) & (CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME))
		{
			Trap_Die(self);
			return;
		}

//		Seems OK, so let's get on with the sticking!

		self->s.effects = 0;
		VectorClear(self->velocity);
		VectorClear(self->avelocity);
		self->movetype = MOVETYPE_NONE;

		if (plane->normal)
			vectoangles(plane->normal, self->s.angles);
		else
			VectorClear(self->s.angles);

//		If we hit a brush model that might be in motion, then move with it.

		if ((other->blocked == plat_blocked) || (other->blocked == door_blocked) || (other->blocked == train_blocked) || (other->blocked == rotating_blocked))
		{
			VectorSubtract(self->s.origin, other->s.origin, self->move_origin);
			self->target_ent = other;
			self->think = Trap_MoveWithEnt;
			self->nextthink = level.time + FRAMETIME;

			if ((other->blocked == rotating_blocked) || (other->style == ENT_ID_DOOR_ROTATING))
			{
				self->spawnflags |= 4;
				if (other->movedir[0])				// y-axis
				{
					if (plane->normal[1] > 0.9)				// +ve radial
						self->spawnflags |= 128;
					else if (plane->normal[1] < -0.9)		// -ve radial
						self->spawnflags |= 256;
					else									// circumferential
						self->spawnflags |= 512;
				}
				else if (other->movedir[1])			// z-axis
				{
					if (plane->normal[2] > 0.9)				// +ve radial
						self->spawnflags |= 128;
					else if (plane->normal[2] < -0.9)		// -ve radial
						self->spawnflags |= 256;
					else									// circumferential
						self->spawnflags |= 512;
				}
				else								// x-axis
				{
					if (plane->normal[0] > 0.9)			// +ve radial
						self->spawnflags |= 128;
					else if (plane->normal[0] < -0.9)	// -ve radial
						self->spawnflags |= 256;
					else								// circumferential
						self->spawnflags |= 512;
				}
			}
			else
				self->spawnflags |= 8;
		}

//		If we're flagged as having been held too long, start tractoring our owner on full power...

		if (self->spawnflags & 1)
		{
			self->spawnflags &= ~1;			// stops Trap_CheckEnv() call in Trap_TractorBeam()
			self->spawnflags &= ~2;
			if ((self->spawnflags & 4) || (self->spawnflags & 8))	// keep tractoring
				self->spawnflags |= 64;
		}

//		...otherwise, start scanning for enemies.
//		NB: Proximity-checking must be handled differently if moving with a bmodel, as the think 
//		function is already taken by Trap_MoveWithEnt().

		else
		{
			if (!(self->spawnflags & 4) && !(self->spawnflags & 8))
			{
				self->think = Trap_Arm_ProximityCheck;
				self->nextthink = level.time + sv_trap_activate_delay->value;
				self->svflags = SVF_DEADMONSTER | SVF_PROJECTILE;
			}
			else
				self->delay = level.time + sv_trap_activate_delay->value;
		}

		self->touch = NULL;
		gi.linkentity(self);

		if (random() > 0.5)
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/trap/bounce1.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/trap/bounce2.wav"), 1, ATTN_NORM, 0);
	}

//	Otherwise, pop.

	else
		Trap_Die(self);
}

void Fire_Trap(edict_t *self, vec3_t start, vec3_t aimdir, int hook_damage, float speed, int beam_damage, int beam_power, qboolean held)
{
	edict_t	*trap;
	vec3_t	dir;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	up;

	trap = G_Spawn();
	trap->solid = SOLID_BBOX;
	trap->movetype = MOVETYPE_TOSS;
	trap->takedamage = DAMAGE_YES;
	trap->clipmask = MASK_SHOT;
	trap->monsterinfo.aiflags = AI_NOSTEP;
	trap->health = 1;
	trap->mass = 10;
	trap->dmg = hook_damage;
	trap->radius_dmg = beam_damage;
	trap->count = beam_power;
	trap->classname = "trap";
	trap->owner = self;
	trap->die = Trap_DieFromDamage;
	trap->touch = Trap_Touch;

	trap->model = "models/objects/trap/tris.md2";
	trap->s.modelindex = gi.modelindex(trap->model);
	VectorSet(trap->mins, -6.0, -6.0, -6.0);
	VectorSet(trap->maxs, 6.0, 6.0, 6.0);

	if ((int)sv_gametype->value > G_FFA)
	{
		if (self->client->resp.ctf_team == CTF_TEAM1)
			trap->s.skinnum = 1;
		else if (self->client->resp.ctf_team == CTF_TEAM2)
			trap->s.skinnum = 2;
	}

	vectoangles(aimdir, dir);
	AngleVectors(dir, forward, right, up);
	VectorCopy(start, trap->s.origin);
	VectorScale(aimdir, speed, trap->velocity);
	VectorMA(trap->velocity, 200.0, up, trap->velocity);
	VectorSet(trap->avelocity, 300.0, 300.0, 300.0);

	gi.linkentity(trap);
	TList_AddNode(trap);

	trap->nextthink = level.time + FRAMETIME;
	if (held)
	{
		trap->spawnflags = 3;	//bit 0 => held;  bit 1 => don't pull
		trap->think = Trap_TractorBeam;
		trap->enemy = self;
		self->tractored = true;
	}
	else
		trap->think = Trap_CheckEnv;
}
/*
======================================================================
EXPLOSIVE SPIKE GUN subroutines

spawnflags (internal use only):
 4  = stuck to a rotating brush model [*]
 8  = stuck to a moving brush model
128 = [*] on positive radial surface
256 = [*] on negative radial surface
512 = [*] on circumferential surface
======================================================================
*/
void Spike_Explode(edict_t *self)
{
	if (self->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd(self->enemy->mins, self->enemy->maxs, v);
		VectorMA(self->enemy->s.origin, 0.5, v, v);
		VectorSubtract(self->s.origin, v, v);
		points = self->radius_dmg - (0.5 * VectorLength(v));
		VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
		T_Damage(self->enemy, self, self->owner, dir, self->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, MOD_SPIKE_SPLASH);
	}

	T_RadiusDamage(self, self->owner, self->radius_dmg, self->enemy, self->dmg_radius, MOD_SPIKE_SPLASH);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte((self->waterlevel)?TE_ROCKET_EXPLOSION_WATER:TE_ROCKET_EXPLOSION);
	gi.WritePosition(self->s.origin);
	gi.multicast(self->s.origin, MULTICAST_PHS);

	G_FreeEdict(self);
}

void Spike_RotateWithEnt(edict_t *self)
{
	edict_t	*t;
	vec3_t	normal;
	float	vec1;
	float	vec2;
	float	radius;
	float	angle;

//	Sanity check.

	if (!self->enemy)
	{
		gi_centerprintf(self->owner, "BUG: Spike_RotateWithEnt() called with no enemy.\nPlease contact musashi@planetquake.com\n");
		G_FreeEdict(self);
		return;
	}

//	Adjust origin based on the axis of rotation.

	t = self->enemy;
	if (t->movedir[0])			// y-axis
	{
		vec1 = self->s.origin[0] - t->s.origin[0];
		vec2 = self->s.origin[2] - t->s.origin[2];
		radius = sqrt(vec1*vec1 + vec2*vec2);
		angle = atan2(vec2, vec1) - DEG2RAD(t->avelocity[0] * FRAMETIME);
		self->s.origin[0] = t->s.origin[0] + (radius * cos(angle));
		self->s.origin[2] = t->s.origin[2] + (radius * sin(angle));

		if (self->spawnflags & 128)
			self->s.angles[2] = RAD2DEG(angle);
		else if (self->spawnflags & 256)
			self->s.angles[2] = -RAD2DEG(angle);
		else if (self->spawnflags & 512)
		{
			trace_t tr = gi.trace(self->s.origin, NULL, NULL, t->s.origin, self, MASK_SOLID);
			if (tr.surface)
			{
				VectorNegate(tr.plane.normal, normal);
				vectoangles(normal, self->s.angles);
			}
		}
	}
	else if (t->movedir[1])		// z-axis
	{
		vec1 = self->s.origin[0] - t->s.origin[0];
		vec2 = self->s.origin[1] - t->s.origin[1];
		radius = sqrt(vec1*vec1 + vec2*vec2);
		angle = atan2(vec2, vec1) + DEG2RAD(t->avelocity[1] * FRAMETIME);
		self->s.origin[0] = t->s.origin[0] + (radius * cos(angle));
		self->s.origin[1] = t->s.origin[1] + (radius * sin(angle));

		if (self->spawnflags & 128)
			self->s.angles[1] = RAD2DEG(angle);
		else if (self->spawnflags & 256)
			self->s.angles[1] = RAD2DEG(angle);
		else if (self->spawnflags & 512)
		{
			trace_t tr = gi.trace(self->s.origin, NULL, NULL, t->s.origin, self, MASK_SOLID);
			if (tr.surface)
			{
				VectorNegate(tr.plane.normal, normal);
				vectoangles(normal, self->s.angles);
			}
		}
	}
	else						// x-axis
	{
		vec1 = self->s.origin[1] - t->s.origin[1];
		vec2 = self->s.origin[2] - t->s.origin[2];
		radius = sqrt(vec1*vec1 + vec2*vec2);
		angle = atan2(vec2, vec1) + DEG2RAD(t->avelocity[2] * FRAMETIME);
		self->s.origin[1] = t->s.origin[1] + (radius * cos(angle));
		self->s.origin[2] = t->s.origin[2] + (radius * sin(angle));

		if (self->spawnflags & 128)
			self->s.angles[2] = -RAD2DEG(angle);
		else if (self->spawnflags & 256)
			self->s.angles[2] = RAD2DEG(angle);
		else if (self->spawnflags & 512)
		{
			trace_t tr = gi.trace(self->s.origin, NULL, NULL, t->s.origin, self, MASK_SOLID);
			if (tr.surface)
			{
				VectorNegate(tr.plane.normal, normal);
				vectoangles(normal, self->s.angles);
			}
		}
	}
}

void Spike_MoveWithEnt(edict_t *self)
{
	edict_t	*t;
	float	z_offset;

	if (level.time >= self->delay)
		self->think = Spike_Explode;

	self->nextthink = level.time + FRAMETIME;

//	Sanity check.

	if (!self->enemy)
	{
		gi_centerprintf(self->owner, "BUG: Spike_MoveWithEnt() called with no enemy.\nPlease contact musashi@planetquake.com\n");
		G_FreeEdict(self);
		return;
	}

//	Handle spike movement for the case that it's stuck to a player. Note that the
//	z-position needs to be adjusted if the player is crouching or jumping.

	t = self->enemy;
	if (t->client)
	{
		if ((t->health < 1) || !t->inuse)
		{
			self->movetype = MOVETYPE_TOSS;
			self->think = Spike_Explode;
			self->nextthink = self->delay;
			self->enemy = NULL;
		}
		else
		{
			VectorAdd(t->s.origin, self->move_origin, self->s.origin);
			if (t->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				z_offset = 32.0 * ((self->s.origin[2] - (t->s.origin[2] - 24.0)) / 56.0);
				self->s.origin[2] = t->s.origin[2] - 24.0 + z_offset;
			}
			else if (t->client->anim_priority == ANIM_JUMP)
			{
				z_offset = 32.0 * (((t->s.origin[2] + 32.0) - self->s.origin[2]) / 56.0);
				self->s.origin[2] = t->s.origin[2] + 32.0 - z_offset;
			}
		}
	}
	else
	{

//		Handle spike movement for the case that it's stuck to a moving brush model.

		if (self->spawnflags & 4)			// rotating brush model
			Spike_RotateWithEnt(self);
		else if (self->spawnflags & 8)		// non-rotating brush model 
			VectorAdd(t->s.origin, self->move_origin, self->s.origin);
	}

	gi.linkentity(self);
}

void Spike_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

//	Vanish if we hit a sky surface.

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

//	Ignore weapon projectiles.

	if (other->wep_proj)
		return;

//	Stick to whatever we hit; move in step with those entities which may be in motion themselves.

	self->movetype = MOVETYPE_NONE;
	self->s.effects = 0;
	self->solid = SOLID_NOT;
	self->touch = NULL;

	if (!other->takedamage)
	{
		if ((other->blocked == plat_blocked) || (other->blocked == door_blocked) || (other->blocked == train_blocked) || (other->blocked == rotating_blocked))
		{
			VectorSubtract(self->s.origin, other->s.origin, self->move_origin);
			self->enemy = other;
			self->delay = level.time + sv_spike_bang_delay->value;
			self->think = Spike_MoveWithEnt;
			self->nextthink = level.time + FRAMETIME;

			if ((other->blocked == rotating_blocked) || (other->style == ENT_ID_DOOR_ROTATING))
			{
				self->spawnflags |= 4;
				if (other->movedir[0])				// y-axis
				{
					if (plane->normal[1] > 0.9)				// +ve radial
						self->spawnflags |= 128;
					else if (plane->normal[1] < -0.9)		// -ve radial
						self->spawnflags |= 256;
					else									// circumferential
						self->spawnflags |= 512;
				}
				else if (other->movedir[1])			// z-axis
				{
					if (plane->normal[2] > 0.9)				// +ve radial
						self->spawnflags |= 128;
					else if (plane->normal[2] < -0.9)		// -ve radial
						self->spawnflags |= 256;
					else									// circumferential
						self->spawnflags |= 512;
				}
				else								// x-axis
				{
					if (plane->normal[0] > 0.9)			// +ve radial
						self->spawnflags |= 128;
					else if (plane->normal[0] < -0.9)	// -ve radial
						self->spawnflags |= 256;
					else								// circumferential
						self->spawnflags |= 512;
				}
			}
			else
				self->spawnflags |= 8;
		}
		else
		{
			self->nextthink = level.time + sv_spike_bang_delay->value;
			self->think = Spike_Explode;
		}
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/esg/hit_wall.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/esg/hit_body.wav"), 1, ATTN_NORM, 0);
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, self->count, 0, MOD_SPIKE);
		
		if (other->health > 0)						// could be a player or a func_plat/door/train
		{
			self->enemy = other;
			self->delay = level.time + sv_spike_bang_delay->value;
			self->think = Spike_MoveWithEnt;
			self->nextthink = level.time + FRAMETIME;

			if (other->client)
				VectorMA(self->s.origin, 0.1, self->velocity, self->s.origin);

			VectorSubtract(self->s.origin, other->s.origin, self->move_origin);
		}
		else										// assume it's a deady body
		{
			self->movetype = MOVETYPE_TOSS;
			self->think = Spike_Explode;
			self->nextthink = level.time + sv_spike_bang_delay->value;
		}
	}

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SPARKS);
	gi.WritePosition(self->s.origin);
	gi.WriteDir(vec3_origin);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	VectorClear(self->velocity);
	VectorClear(self->avelocity);
	gi.linkentity(self);
}

void Fire_Spike(edict_t *self, vec3_t start, vec3_t aimdir, int damage, float speed, int kick, float damage_radius, int radius_damage)
{
	edict_t	*spike;
	trace_t	tr;

	spike = G_Spawn();
	spike->solid = SOLID_BBOX;
	spike->movetype = MOVETYPE_FLYMISSILE;
	spike->clipmask = MASK_SHOT;
	spike->svflags = SVF_DEADMONSTER;
	spike->s.effects |= EF_IONRIPPER;
	spike->s.renderfx = RF_FULLBRIGHT;
	spike->dmg = damage;
	spike->count = kick;
	spike->radius_dmg = radius_damage;
	spike->dmg_radius = damage_radius;
	spike->classname = "spike";
	spike->wep_proj = true;
	spike->owner = self;
	spike->touch = Spike_Touch;
	spike->think = Spike_Explode;
	spike->nextthink = level.time + SPIKE_LIVETIME;

	spike->model = "models/objects/spike/tris.md2";
	spike->s.modelindex = spike_index;

	VectorCopy(start, spike->s.origin);
	VectorCopy(start, spike->s.old_origin);
	vectoangles(aimdir, spike->s.angles);
	VectorScale(aimdir, speed, spike->velocity);

	gi.linkentity(spike);

	tr = gi.trace(self->s.origin, NULL, NULL, spike->s.origin, spike, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA(spike->s.origin, -10.0, aimdir, spike->s.origin);
		spike->touch(spike, tr.ent, NULL, NULL);
	}
}

/*
======================================================================
FLAMETHROWER subroutines

spawnflags (internal use only):
1 = fireball has already halted on something
2 = fireball has already done damage to something
======================================================================
*/
//
// Flame_* subroutines are for the larger fires, for burning players.
//
void Flame_Expire(edict_t *self)
{
	vec3_t	puff_start;

	if (Q_stricmp(self->classname, "flame"))
	{
		gi.dprintf("BUG: Flame_Expire() called for a non-flame edict.\n");
		gi.dprintf("     classname = %s\n", self->classname);
		if (self->owner && self->owner->client)
			gi.dprintf("     owner->name = %s\n", self->owner->client->pers.netname);

		return;
	}

	if (self->s.frame < 34)
		self->s.frame = 34;	
	else if (self->s.frame < 38)
		++self->s.frame;
	else
	{
		puff_start[0] = self->s.origin[0];
		puff_start[1] = self->s.origin[1];
		puff_start[2] = self->s.origin[2] + 32;
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_HEATBEAM_SPARKS);
		gi.WritePosition(puff_start);
		gi.WriteDir(vec3_up);
		gi.multicast(puff_start, MULTICAST_PVS);

		G_FreeEdict(self);
		return;
	}

	self->touch = NULL;
	self->think = Flame_Expire;
	self->nextthink = level.time + FRAMETIME;
}

void Flame_Burn(edict_t *self)
{
	vec3_t	offset;

	++self->s.frame;
	if (self->s.frame == 34)
		self->s.frame = 21;

	self->nextthink = level.time + FRAMETIME;

	if (!self->enemy)
	{
		gi_centerprintf(self->owner, "BUG: Flame_Burn() called with no enemy.\nPlease contact musashi@planetquake.com\n");
		G_FreeEdict(self);
		return;
	}
	
//	Sanity check - if our enemy isn't flagged as being on fire (eg. they've jumped into water),
//	then extinguish ourself.

	if (!self->enemy->burning)
	{
		self->touch = NULL;
		self->think = Flame_Expire;
		return;
	}

//	If the enemy is gibbed, snuff out.

	if (self->enemy->health < -40)
	{
		self->touch = NULL;
		self->think = Flame_Expire;
		return;
	}

//	Otherwise, follow them around and apply burning damage every second.

	VectorCopy(self->enemy->s.origin, self->s.origin);
	if (self->enemy->health <= 0)
	{
		VectorSet(offset, 0.0, 0.0, -18.0);
		VectorAdd(self->s.origin, offset, self->s.origin);
	}
	gi.linkentity(self);

	if (level.time >= self->wait)
	{
		T_Damage(self->enemy, self, self->owner, self->velocity, self->s.origin, vec3_origin, self->dmg, 0, 0, MOD_FLAME);
		self->wait = level.time + FLAME_DMG_RATE;
	}
}

void Flame_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;

//	Don't ignite someone who is already nicely warm and toasty.

	if (other->burning)
		return;

//	The environment suit and Invulnerability will protect against fire damage and personal ignition.

	if (other->client->enviro_framenum > level.framenum)
		return;

	if (other->client->invincible_framenum > level.framenum)
		return;

//	Don't ignite team members of our owner in CTF or TDM.

	if (CheckTeamDamage(other, self->owner))
		return;

//	Otherwise, let them burn!

	other->burning = true;
	T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 0, 0, MOD_FLAME);
	Spawn_Flame(self, other, other->s.origin, self->dmg);
}

void Spawn_Flame(edict_t *self, edict_t *other, vec3_t start, int damage)
{
	edict_t	*flame;

	flame = G_Spawn();
	flame->solid = SOLID_TRIGGER;
	flame->movetype = MOVETYPE_NONE;
	flame->dmg = damage;
	flame->classname = "flame";
	flame->wep_proj = true;
	flame->takedamage = DAMAGE_NO;
	flame->s.effects |= EF_PLASMA | EF_HYPERBLASTER;
	flame->s.renderfx = RF_FULLBRIGHT;
	flame->owner = self->owner;
	flame->think = Flame_Burn;
	flame->touch = Flame_Touch;
	flame->enemy = other;
	other->flame = flame;
	flame->nextthink = level.time + FRAMETIME;
	flame->wait = level.time + FLAME_DMG_RATE;

	VectorSet(flame->mins, -18, -18, 0);
	VectorSet(flame->maxs, 18, 18, 24);
	flame->model = "models/fire/tris.md2";
	flame->s.modelindex = gi.modelindex(flame->model);
	flame->s.frame = 16;
	flame->s.sound = gi.soundindex("weapons/flamer/fire1.wav");

	VectorCopy(start, flame->s.origin);
	VectorCopy(start, flame->s.old_origin);
	VectorClear(flame->velocity);

	gi.linkentity(flame);
}
//
// Flame_Small_* subroutines are for the smaller fires that burn on the ground.
//
void Flame_Small_Expire(edict_t *self)
{
	vec3_t	puff_start;

	puff_start[0] = self->s.origin[0];
	puff_start[1] = self->s.origin[1];
	puff_start[2] = self->s.origin[2] + 16;
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_HEATBEAM_SPARKS);
	gi.WritePosition(puff_start);
	gi.WriteDir(vec3_up);
	gi.multicast(puff_start, MULTICAST_PVS);

	G_FreeEdict(self);
}

void Flame_Small_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	point;
	int		cont;

//	Vanish if we hit a sky surface.

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

//	Fizzle out if we hit water.

	point[0] = self->s.origin[0];
	point[1] = self->s.origin[1];
	point[2] = self->s.origin[2] + self->mins[2];	
	if ((cont = gi.pointcontents(point)) & MASK_WATER)
	{
		self->think = Flame_Small_Expire;
		self->nextthink = level.time + FRAMETIME;
		self->touch = NULL;
		return;
	}

//	If a player touches the flame, damage them and then snuff out. There is a 20% chance of igniting the 
//	player if they're not already burning.

	if (other->client)
	{
		self->touch = NULL;
		self->think = Flame_Small_Expire;
		self->nextthink = level.time + FRAMETIME;

//		The environment suit and Invulnerability protect against small fire damage and personal ignition.

		if (other->client && (other->client->enviro_framenum > level.framenum))
			return;

		if (other->client && (other->client->invincible_framenum > level.framenum))
			return;

//		Don't ignite team members of our owner in CTF or TDM.

		if (CheckTeamDamage(other, self->owner))
			return;
		
//		If it's a non-burning player, and they fail their saving throw vs combustion, make them burn!

		if (other->client && !other->burning && (random() < FT_SMALL_IGNITE_PROB))
		{
			other->burning = true;
			Spawn_Flame(self, other, other->s.origin, self->dmg);
		}

		T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->radius_dmg, 0, 0, MOD_FLAME);
	}
}

void Flame_Small_Burn(edict_t *self)
{
	++self->s.frame;
	if (self->s.frame == 15)
		self->s.frame = 3;

	if (level.time >= self->wait)
		self->think = Flame_Small_Expire;

	self->nextthink = level.time + FRAMETIME;

	Fireball_CheckEnv(self);
}

void Spawn_Flame_Small(edict_t *self, vec3_t start, int damage, int damage_smallflame)
{
	edict_t	*flame;

	flame = G_Spawn();
	flame->solid = SOLID_TRIGGER;
	flame->svflags = SVF_DEADMONSTER;
	flame->movetype = MOVETYPE_TOSS;
	flame->clipmask = MASK_SHOT;
	flame->takedamage = DAMAGE_NO;
	flame->s.effects |= EF_PLASMA;
	flame->s.renderfx = RF_FULLBRIGHT;
	flame->dmg = damage;
	flame->radius_dmg = damage_smallflame;
	flame->classname = "flame_small";
	flame->wep_proj = true;
	flame->owner = self->owner;
	flame->touch = Flame_Small_Touch;
	flame->think = Flame_Small_Burn;
	flame->nextthink = level.time + FRAMETIME;
	flame->wait = level.time + FLAME_LIVETIME_0 + (FLAME_LIVETIME_R * random());

	VectorSet(flame->mins, -2.0, -2.0, 0.0);
	VectorSet(flame->maxs, 2.0, 2.0, 4.0);
	flame->model = "models/fire/tris.md2";
	flame->s.modelindex = gi.modelindex(flame->model);
	flame->s.frame = 0;

	VectorCopy(start, flame->s.origin);
	VectorCopy(start, flame->s.old_origin);
	VectorClear(flame->velocity);

	gi.linkentity(flame);
}
//
// Fireball_* subroutines are for the burning globs emitted by the Flamethrower.
//
void Fireball_Expire(edict_t *self)
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_HEATBEAM_SPARKS);
	gi.WritePosition(self->s.origin);
	gi.WriteDir(vec3_up);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	G_FreeEdict(self);
}

void Fireball_CheckBurn(edict_t *self)
{
	edict_t	*cl_ent = NULL;

	while ((cl_ent = FindClientRadius(cl_ent, self->s.origin, FT_IGNITE_RADIUS)) != NULL)
	{
		if (cl_ent == self->owner)
			continue;
		if (cl_ent->burning)
			continue;
		if (CheckTeamDamage(cl_ent, self->owner))	
			continue;
		if (cl_ent->client->enviro_framenum > level.framenum)
			continue;
		if (cl_ent->client->invincible_framenum > level.framenum)
			continue;

		if (random() < FT_IGNITE_PROB)
		{
			cl_ent->burning = true;
			Spawn_Flame(self, cl_ent, cl_ent->s.origin, self->dmg);
		}
	}
}

void Fireball_CheckEnv(edict_t *self)
{	//NB: could free self, so be careful how you call this function!
	vec3_t	point;
	vec3_t	puff_start;

//	Fizzle out if we're in liquid.

	point[0] = self->s.origin[0];
	point[1] = self->s.origin[1];
	point[2] = self->s.origin[2];

	if (gi.pointcontents(point) & MASK_WATER)
	{
		puff_start[0] = self->s.origin[0];
		puff_start[1] = self->s.origin[1];
		puff_start[2] = self->s.origin[2] + 20.0;

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_HEATBEAM_SPARKS);
		gi.WritePosition(puff_start);
		gi.WriteDir(vec3_up);
		gi.multicast(puff_start, MULTICAST_PVS);

		G_FreeEdict(self);
	}
}

void Fireball_Think(edict_t *self)
{
	self->nextthink = level.time + FRAMETIME;

	if (level.time < self->delay)
	{
		Fireball_CheckBurn(self);
		Fireball_CheckEnv(self);
		return;
	}

	if (self->s.skinnum < 6)
		++self->s.skinnum;

	if (self->s.frame == 0)
		self->s.frame = 15;
	else
		++self->s.frame;

	if (self->s.frame == 18)
		self->s.effects |= EF_SPHERETRANS;

	if (self->s.frame == 21)
		self->think = G_FreeEdict;

	Fireball_CheckBurn(self);
	Fireball_CheckEnv(self);
}

void Fireball_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*ent;
	vec3_t	point;
	int		cont;

	if (other == self)
		return;

//	Don't ignite owner.

	if (other == self->owner)
		return;

//	Vanish if we hit a sky surface.

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}
	
//	Fizzle out if we hit water.

	point[0] = self->s.origin[0];
	point[1] = self->s.origin[1];
	point[2] = self->s.origin[2] - 2.0;
	if ((cont = gi.pointcontents(point)) & MASK_WATER)
	{
		self->think = Fireball_Expire;
		self->nextthink = level.time + FRAMETIME;
		self->touch = NULL;
		return;
	}

	if (other->wep_proj)
		return;

//	There's a 10% chance that a small burning globule will be produced.

	if (!other->takedamage)
	{	
		if (self->spawnflags & 1)	// already halted (and maybe spawned a small flame)
			return;
		else
		{
			VectorMA(self->s.origin, -0.001, self->velocity, self->s.origin);
			VectorClear(self->velocity);
			self->spawnflags |= 1;
			gi.linkentity(self);
			if (random() < FT_SMALLFLAME_PROB)
				Spawn_Flame_Small(self, self->s.origin, self->dmg, self->radius_dmg);

//			Destroy nearby Traps and C4 bundles (beyond fireball's bounding box).

			ent = NULL;
			while ((ent = FindRadius(ent, self->s.origin, 40.0)) != NULL)
			{
				if ((ent->die == C4_DieFromDamage) || (ent->die == Trap_DieFromDamage))
					T_Damage(ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, 999, 0, 0, MOD_FLAME);
			}
		}
		return;
	}

	if (self->spawnflags & 2)	// already done damage
		return;

	if (other->client && (other->client->invincible_framenum > level.framenum))
		return;

	if (CheckTeamDamage(other, self->owner))
		return;

//	Otherwise, cause damage; there is a 50% chance of igniting the player if they're not already burning.
//	NB:	The environment suit reduces flamethrower damage, and prevents personal ignition.

	if (other->client && (other->client->enviro_framenum > level.framenum))
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, vec3_origin, (int)(0.2*self->dmg), 0, 0, MOD_FLAMETHROWER);
	else
	{		
		if (other->client && !other->burning && (random() < FT_IGNITE_PROB))
		{
			other->burning = true;
			Spawn_Flame(self, other, other->s.origin, self->dmg);
		}

		T_Damage(other, self, self->owner, self->velocity, self->s.origin, vec3_origin, self->dmg, 0, 0, MOD_FLAMETHROWER);
	}
	self->spawnflags |= 2;
}

void Fire_Fireball(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int damage_smallflame, float speed, qboolean glow)
{
	edict_t	*fireball;
	trace_t	tr;

	fireball = G_Spawn();
	fireball->svflags = SVF_DEADMONSTER;
	fireball->solid = SOLID_BBOX;
	fireball->movetype = MOVETYPE_FLYMISSILE;
	fireball->clipmask = MASK_SHOT;
	fireball->takedamage = DAMAGE_NO;
	fireball->s.effects |= EF_PLASMA | ((glow)?EF_HYPERBLASTER:0);
	fireball->s.renderfx = RF_FULLBRIGHT;
	fireball->dmg = damage;
	fireball->radius_dmg = damage_smallflame;
	fireball->classname = "fireball";
	fireball->owner = self;
	fireball->touch = Fireball_Touch;
	fireball->think = Fireball_Think;
	fireball->nextthink = level.time + FRAMETIME;
	fireball->spawnflags = 0;
	fireball->wep_proj = true;
	fireball->delay = level.time + 0.2;

	fireball->model = "models/objects/r_explode/tris.md2";
	fireball->s.modelindex = r_explode_index;
	fireball->s.frame = 0;
	fireball->s.skinnum = 1;

	VectorCopy(start, fireball->s.origin);
	VectorCopy(start, fireball->s.old_origin);
	vectoangles(aimdir, fireball->s.angles);
	VectorScale(aimdir, speed, fireball->velocity);

	gi.linkentity(fireball);

	tr = gi.trace(self->s.origin, NULL, NULL, fireball->s.origin, fireball, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA(fireball->s.origin, -10.0, aimdir, fireball->s.origin);
		fireball->touch(fireball, tr.ent, NULL, NULL);
	}
}

//------- Firebomb-sepcific functions -------------

void Firebomb_Explode(edict_t *self)
{
	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame < 4)
	{
		++self->s.frame;
		++self->s.skinnum;

		if (self->s.frame == 1)
			self->s.effects |= EF_PLASMA;
		else if (self->s.frame == 3)
			self->s.effects |= EF_SPHERETRANS;
	}
	else
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_HEATBEAM_SPARKS);
		gi.WritePosition(self->s.origin);
		gi.WriteDir(vec3_up);
		gi.multicast(self->s.origin, MULTICAST_PVS);

		G_FreeEdict(self);
	}
}

void Firebomb_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	point;
	int		cont;

	if (other == self)
		return;

//	Don't ignite owner.

	if (other == self->owner)
		return;

//	Vanish if we hit a sky surface.

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}
	
//	Ignore other weapon projectiles.

	if (other->wep_proj)
		return;

//	Fizzle out if we hit water.

	point[0] = self->s.origin[0];
	point[1] = self->s.origin[1];
	point[2] = self->s.origin[2] + self->mins[2];	
	if ((cont = gi.pointcontents(point)) & MASK_WATER)
	{
		self->think = Fireball_Expire;
		self->nextthink = level.time + FRAMETIME;
		self->touch = NULL;
		return;
	}

//	Stop and explode.

	VectorMA(self->s.origin, -0.001, self->velocity, self->s.origin);
	VectorClear(self->velocity);
	self->think = Firebomb_Explode;
	self->nextthink = level.time + FRAMETIME;
	self->touch = NULL;
	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	gi.linkentity(self);

	if (other->takedamage)
	{

	//	Otherwise, cause damage; ignite the player if they're not already burning.
	//	NB:	The environment suit prevents ignition.

		if (other->client && !other->burning)
		{
			if (!CheckTeamDamage(other, self->owner) && (other->client->enviro_framenum <= level.framenum) &&
				                                        (other->client->invincible_framenum <= level.framenum))
			{
				other->burning = true;
				Spawn_Flame(self, other, other->s.origin, self->radius_dmg);
			}
		}

		T_Damage(other, self, self->owner, self->velocity, self->s.origin, vec3_origin, self->dmg, self->count, 0, MOD_FIREBOMB);
	}

	T_RadiusDamage(self, self->owner, self->dmg, other, self->dmg_radius, MOD_FIREBOMB_SPLASH);

	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/rocklx1a.wav"), 1, ATTN_NORM, 0);
	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);
}

void Firebomb_Think(edict_t *self)
{
	self->nextthink = level.time + FRAMETIME;
	Fireball_CheckEnv(self);
}

void Fire_Firebomb(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int damage_minor, int kick, float damage_radius, float speed)
{
	edict_t	*firebomb;
	trace_t	tr;

	firebomb = G_Spawn();
	firebomb->svflags = SVF_PROJECTILE | SVF_DEADMONSTER;
	firebomb->solid = SOLID_BBOX;
	firebomb->movetype = MOVETYPE_FLYMISSILE;
	firebomb->clipmask = MASK_SHOT;
	firebomb->s.effects = EF_ROCKET;
	firebomb->s.renderfx = RF_FULLBRIGHT;
	firebomb->dmg = damage;
	firebomb->radius_dmg = damage_minor;		// actually flame burning dmg, not radius damage
	firebomb->dmg_radius = damage_radius;
	firebomb->count = kick;
	firebomb->classname = "firebomb";
	firebomb->wep_proj = true;
	firebomb->owner = self;
	firebomb->touch = Firebomb_Touch;
	firebomb->think = Firebomb_Think;
	firebomb->nextthink = level.time + FRAMETIME;

	firebomb->model = "models/objects/firebomb/tris.md2";
	firebomb->s.modelindex = gi.modelindex(firebomb->model);
	firebomb->s.frame = 0;
	firebomb->s.skinnum = 0;

	VectorCopy(start, firebomb->s.origin);
	VectorCopy(start, firebomb->s.old_origin);
	vectoangles(aimdir, firebomb->s.angles);
	VectorScale(aimdir, speed, firebomb->velocity);

	gi.linkentity(firebomb);

	tr = gi.trace(self->s.origin, NULL, NULL, firebomb->s.origin, firebomb, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA(firebomb->s.origin, -10.0, aimdir, firebomb->s.origin);
		firebomb->touch(firebomb, tr.ent, NULL, NULL);
	}
}

/*
======================================================================
SHOCKRIFLE subroutines
======================================================================
*/
void Shock_SeekEnemy(edict_t *self);

void Shock_HomeIn(edict_t *self)
{
	vec3_t	end;
	vec3_t	dir;
	vec3_t	deltav;
	float	range;

	self->nextthink = level.time + FRAMETIME;

	if (level.time > self->wait)
	{
		self->think = G_FreeEdict;
		self->touch = NULL;
		return;
	}

//	Sanity check.

	if (!self->enemy)
	{
		gi_centerprintf(self->owner, "BUG: Shock_HomeIn() called with no enemy.\nPlease contact musashi@planetquake.com\n");
		G_FreeEdict(self);
		return;
	}

//	Set our target to be the enemy player's cold, black heart.

	end[0] = self->enemy->s.origin[0];
	end[1] = self->enemy->s.origin[1];
	end[2] = self->enemy->s.origin[2] + (self->enemy->viewheight - 8.0);

//	Obtain an attack vector that points from us to the target. If the length of this vector is greater than
//	our detection range, lose the current target and search for another one.

	VectorSubtract(end, self->s.origin, dir);
	range = VectorLength(dir);
	if (range > sv_shock_homing_range->value)
	{
		self->think = Shock_SeekEnemy;
		self->enemy = NULL;
		return;
	}

//	Otherwise, keep homing! Adjust our orientation to point towards the target.

	VectorNormalize(dir);
	VectorCopy(dir, self->movedir);
	vectoangles(dir, self->s.angles);

//	Apply a delta-V to our current velocity based on the attack vector.

	VectorScale(dir, self->speed, deltav);
	VectorAdd(deltav, self->velocity, self->velocity);
	VectorNormalize(self->velocity);
	VectorScale(self->velocity, self->speed, self->velocity);

	gi.linkentity(self);
}

void Shock_SeekEnemy(edict_t *self)
{
	edict_t	*cl_ent = NULL;

	self->nextthink = level.time + FRAMETIME;

	if (level.time > self->wait)
	{
		self->think = G_FreeEdict;
		self->touch = NULL;
		return;
	}

//	Valid targets are live players who aren't our owner, or team members of our owner, within our 
//	detection range.

	while ((cl_ent = FindRadius(cl_ent, self->s.origin, sv_shock_homing_range->value)) != NULL)
	{
		if (!cl_ent->client)
			continue;
		if (cl_ent == self->owner)
			continue;
		if (cl_ent->health < 1)
			continue;
		if (cl_ent->client->antibeam_framenum > level.framenum)
			continue;
		if ((int)sv_gametype->value > G_FFA)
		{
			if (self->owner->client && (cl_ent->client->resp.ctf_team == self->owner->client->resp.ctf_team))
				continue;
		}

//		Target is valid, so home in on them.

		self->enemy = cl_ent;
		self->think = Shock_HomeIn;
		break;
	}
}

void Shock_HomingTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	origin;

//	Vanish if we hit a sky surface.

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

//	Ignore weapon projectiles.

	if (other->wep_proj)
		return;

//	Explode and do damage.

	VectorMA(self->s.origin, -0.02, self->velocity, origin);
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_BFG_BIGEXPLOSION);
	gi.WritePosition(origin);
	gi.multicast(origin, MULTICAST_PVS);

	gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/shock/shockhit.wav"), 1, ATTN_NORM, 0);
	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, self->count, DAMAGE_ENERGY, MOD_SR_HOMING);

	T_RadiusDamage(self, self->owner, self->dmg, other, self->dmg_radius, MOD_SR_HOMING);
	G_FreeEdict(self);
}

void Shock_DisintegratorTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

//	Vanish if we hit a sky surface.

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

//	Ignore weapon projectiles.

	if (other->wep_proj)
		return;

//	Explode and do damage.

	VectorMA(self->s.origin, -0.02, self->velocity, origin);
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_NUKEBLAST);
	gi.WritePosition(origin);
	gi.multicast(origin, MULTICAST_PVS);

	gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/shock/shockhit.wav"), 1, ATTN_NORM, 0);
	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		if (other->client)
			other->disintegrated = true;

		T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, 10000, self->count, DAMAGE_ENERGY, MOD_SR_DISINT);
	}		

	T_RadiusDamage(self, self->owner, self->radius_dmg, other, self->dmg_radius, MOD_SR_DISINT_WAVE);
	G_FreeEdict(self);
}

void Fire_Shock(edict_t *self, vec3_t start, vec3_t aimdir, int damage_plasma, float speed, int kick, int damage_shockbolt, float damage_radius, qboolean homing)
{
	edict_t	*shock;
	trace_t	tr;

	shock = G_Spawn();
	shock->solid = SOLID_BBOX;
	shock->movetype = MOVETYPE_FLYMISSILE;
	shock->svflags = SVF_DEADMONSTER;
	shock->clipmask = MASK_SHOT;
	shock->s.renderfx = RF_FULLBRIGHT;
	shock->dmg = damage_plasma;
	shock->count = kick;
	shock->speed = speed;
	shock->radius_dmg = damage_shockbolt;
	shock->dmg_radius = damage_radius;
	shock->owner = self;
	shock->s.sound = gi.soundindex("weapons/shock/shockfly.wav");
	shock->wep_proj = true;

	shock->model = "models/objects/shock/tris.md2";
	shock->s.modelindex = gi.modelindex(shock->model);

	if (homing)
	{
		shock->classname = "shock_homing";
		shock->touch = Shock_HomingTouch;
		shock->think = Shock_SeekEnemy;
		shock->nextthink = level.time + FRAMETIME;
		shock->wait = level.time + sv_shock_live_time->value;
		shock->s.skinnum = 1;
		shock->s.effects |= EF_BFG | EF_COLOR_SHELL;
		shock->s.effects |= EF_BLASTER | EF_TRACKER;
		shock->s.renderfx |= RF_SHELL_GREEN;
	}
	else
	{
		shock->svflags |= SVF_PROJECTILE;
		shock->classname = "shock_disint";
		shock->touch = Shock_DisintegratorTouch;
		shock->think = G_FreeEdict;
		shock->nextthink = level.time + sv_shock_live_time->value;			
		shock->s.skinnum = 0;
		shock->s.effects |= EF_BLUEHYPERBLASTER | EF_QUAD | EF_FLAG2;
	}

	VectorCopy(start, shock->s.origin);
	VectorCopy(start, shock->s.old_origin);
	vectoangles(aimdir, shock->s.angles);
	VectorScale(aimdir, speed, shock->velocity);

	gi.linkentity(shock);

	tr = gi.trace(self->s.origin, NULL, NULL, shock->s.origin, shock, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA(shock->s.origin, -10.0, aimdir, shock->s.origin);
		shock->touch(shock, tr.ent, NULL, NULL);
	}
}

/*
======================================================================
GAUSS PISTOL subroutines
======================================================================
*/
void Draw_Beam(edict_t *self)
{
	if (level.time > self->wait)
	{
		G_FreeEdict(self);
		return;
	}

	self->nextthink = level.time + FRAMETIME;

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_BFG_LASER);
	gi.WritePosition(self->s.origin);
	gi.WritePosition(self->pos1);
	gi.multicast(self->s.origin, MULTICAST_PHS);
}

qboolean Particle_Can_Hit(vec3_t start, vec3_t aimdir, edict_t *targ)
{
	vec3_t		vec;
	vec3_t		end[3];
	float		range;
	float		dotp;
	float		tolerance;
	qboolean	hit = false;
	int			i;

//	Take into account the target entity's bounding box - generate two endpoints.

	VectorCopy(targ->s.origin, end[0]);
	VectorAdd(targ->s.origin, targ->mins, end[1]);
	VectorAdd(targ->s.origin, targ->maxs, end[2]);

//	Compare the muzzle->endpoint vectors with the aiming vector; if they're close enough together,
//	call it a hit.
//	NB: the numbers used here were arrived at after playing around to see what felt best in-game.

	for (i = 0; i < 3; ++i)
	{
		VectorSubtract(end[i], start, vec);
		range = VectorLength(vec);
		if (range < 40.0)
			tolerance = 0.5;
		else if (range < 200.0)
			tolerance = 0.5 + (0.00309375 * (range - 40.0));
		else
			tolerance = 0.995;

		VectorNormalize(vec);
		dotp = DotProduct(vec, aimdir);
		if (dotp >= tolerance)
		{
			hit = true;
			break;
		}
	}
	return hit;
}

void Fire_Particle (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	trace_t		tr;
	edict_t		*ignore = self;
	edict_t		*index;
	edict_t		*check;
	edict_t		*refkiller = NULL;
	edict_t		*beam;
	vec3_t		from;
	vec3_t		end;
	vec3_t		beam_mins;
	vec3_t		beam_maxs;
	qboolean	finished = false;
	qboolean	reflected = false;
	float		scale = WORLD_SIZE;	// was 8192.0
	int			i;

//	Extend a long line from the muzzle, and clip it against the world cube boundary; this is the
//	endpoint for the particle beam.

	VectorMA(start, scale, aimdir, end);
	for (i = 0; i < 3; ++i)
	{
		if (end[i] >= MAX_WORLD_COORD)	// was 4095.0
		{
			scale *= (MAX_WORLD_COORD - start[i]) / (end[i] - start[i]);	// was 4095.0
			VectorMA(start, scale, aimdir, end);
		}
		else if (end[i] <= MIN_WORLD_COORD)	// was -4095.0
		{
			scale *= (MAX_WORLD_COORD + start[i]) / (start[i] - end[i]);	// was 4095.0
			VectorMA(start, scale, aimdir, end);
		}
	}

//	Trace a line from the muzzle to the endpoint, and apply damage to those entities that can be
//	damaged. (The contents mask for gi.trace will ensure that the line will pass through world brushes).

	VectorSet(beam_mins, -16.0, -16.0, -16.0);
	VectorSet(beam_maxs, 16.0, 16.0, 16.0);
	VectorCopy(start, from);

	while (ignore)
	{
		tr = gi.trace(from, beam_mins, beam_maxs, end, ignore, CONTENTS_MONSTER | CONTENTS_DEADMONSTER);
		if (tr.ent->client || (tr.ent->solid == SOLID_BBOX))
			ignore = tr.ent;
		else
			ignore = NULL;

		if ((tr.ent != self) && (tr.ent->takedamage))
		{
			T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_ENERGY, MOD_GAUSS_BEAM);
			if (tr.ent->client && (tr.ent->client->antibeam_framenum > level.framenum))
			{
				reflected = true;
				refkiller = tr.ent;
				VectorCopy(tr.endpos, from);
				break;
			}
		}
		
		if (VectorCompare(tr.endpos, from))
			break;
		else
			VectorCopy(tr.endpos, from);
	}

//	Search through the player's linked list of Trap and C4 entities, and destroy them if they are
//	within the particle beam's cone of effect (as they will have been ignored by the above trace).

	if (self->next_node)
	{
		index = self->next_node;
		while (index && !finished)
		{
			check = index;
			if (index->next_node)
				index = index->next_node;
			else
				finished = true;

			if (Particle_Can_Hit(start, aimdir, check))
			{
				if (check->die == C4_DieFromDamage)
					C4_Die(check);
				else if (check->die == Trap_DieFromDamage)
					Trap_Die(check);
				else
					gi.dprintf("BUG: Invalid next_node pointer in Fire_Particle().\nPlease contact musashi@planetquake.com\n");
			}
		}
	}

//	Draw the particle beam.

	for (i = 0; i < 2; ++i)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(i?TE_BUBBLETRAIL:TE_BFG_LASER);
		gi.WritePosition(start);
		gi.WritePosition((reflected)?from:end);
		gi.multicast(start, MULTICAST_PHS);
	}

//	If the beam has been reflected, we're doomed!

	if (reflected && (refkiller != NULL))
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(refkiller->s.origin);
		gi.WritePosition(start);
		gi.multicast(start, MULTICAST_PHS);

		T_Damage(self, refkiller, refkiller, aimdir, self->s.origin, vec3_origin, damage, kick, DAMAGE_ENERGY, MOD_GAUSS_BEAM_REF);
	}
	else

//	Spawn a temporary entity that will continue to draw the beam for a few more frames.
	{
		beam = G_Spawn();
		beam->svflags |= SVF_NOCLIENT | SVF_PROJECTILE;
		beam->classname = "particle_beam";
		beam->think = Draw_Beam;
		beam->nextthink = level.time + FRAMETIME;
		beam->wait = level.time + 0.3;
		VectorCopy(start, beam->s.origin);
		VectorCopy(end, beam->pos1);
		gi.linkentity(beam);
	}
}

void Fire_Instabolt(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	edict_t		*check;
	edict_t		*index;
	vec3_t		end;
	trace_t		tr;
	qboolean	finished = false;

//	Trace a line until we reach something that can be shot, and damage it if possible.

	VectorMA(start, WORLD_SIZE, aimdir, end);	// was 8192.0
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent != self) && (tr.ent->takedamage))
		T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_ENERGY, MOD_GAUSS_BLASTER);

//	Destroy nearby Traps and C4 bundles (as gi.trace will ignore player's own entities).

	if (self->next_node)
	{
		index = self->next_node;
		while (index && !finished)
		{
			check = index;
			if (index->next_node)
				index = index->next_node;
			else
				finished = true;

			if (VecRange(tr.endpos, check->s.origin) < 10.0)
			{
				if (check->die == C4_DieFromDamage)
					C4_Die(check);
				else if (check->die == Trap_DieFromDamage)
					Trap_Die(check);
				else
					gi.dprintf("BUG: Invalid next_node pointer in Fire_Instabolt().\nPlease contact musashi@planetquake.com\n");
			}
		}
	}

//	Don't generate an impact effect if the endpoint is on a sky surface, otherwise generate
//	a blaster bolt puff.

	if (tr.surface && (tr.surface->flags & SURF_SKY))
		return;

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_FLECHETTE);
	gi.WritePosition(tr.endpos);
	if (!tr.plane.normal)
		gi.WriteDir(vec3_origin);
	else
		gi.WriteDir(tr.plane.normal);
	gi.multicast(tr.endpos, MULTICAST_PVS);

	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);
}

/*
======================================================================
Anti-Grav Manipulator subroutines
======================================================================
*/
void Move_AGM(edict_t *self, vec3_t start, vec3_t aimdir)
{
	gclient_t	*cl = self->client;
	trace_t		tr;
	vec3_t		end;
	vec3_t		dir;
	float		speed;

	if (cl->agm_target == NULL)
	{
		gi.dprintf("BUG: Move_AGM() called with null target\n");
		return;
	}

//	Move our target towards the beam endpoint (make the delta-v proportional to target's 
//	distance from the endpoint).
//	Note that we want to do it here, rather than after the checks for beam-breakage, to make
//	it easier for players to throw their target into a hard surface (this way, it gives the 
//	target a last velocity boost before shutting off the beam).

	VectorMA(start, cl->agm_range, aimdir, end);
	VectorSubtract(end, cl->agm_target->s.origin, dir);
	speed = AGM_MOVE_SCALE * VectorLength(dir);
	VectorNormalize(dir);
	VectorScale(dir, speed, cl->agm_target->velocity);
	gi.linkentity(cl->agm_target);

//	Trace a line from the player to the fixed end of the AGM beam. If the trace meets something that isn't
//	the intended client, disconnect the beam.

	cl->agm_target->client->flung_by_agm = false;
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

	if (tr.ent && (tr.ent != cl->agm_target) && Q_stricmp(tr.ent->classname, "worldspawn"))
	{
		cl->agm_target->client->held_by_agm = false;
		cl->agm_target->client->thrown_by_agm = true;
		cl->agm_target = NULL;
		return;
	}

	if ((tr.fraction < 1.0) && !Q_stricmp(tr.ent->classname, "worldspawn"))
	{
		cl->agm_target->client->held_by_agm = false;
		cl->agm_target->client->thrown_by_agm = true;
		cl->agm_target = NULL;
		return;
	}

	cl->agm_target->client->held_by_agm = true;
	cl->agm_target->client->thrown_by_agm = false;

//	If the target is being held against a wall, make sure we can't screw up the range calcs by
//	extending the beam endpoint into the wall.

	tr = gi.trace(cl->agm_target->s.origin, NULL, NULL, end, cl->agm_target, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorSubtract(tr.endpos, self->s.origin, dir);
		self->client->agm_range = VectorLength(dir);
	}
}

void Fire_AGM(edict_t *self, vec3_t start, vec3_t aimdir, qboolean disrupt)
{
	trace_t	tr;
	vec3_t	end;
	vec3_t	vbeam;
	float	dist;
	int		damage;
	int		kick;

//	Trace a line to try and find something that can be manipulated.

	self->client->agm_target = NULL;
	VectorMA(start, WORLD_SIZE, aimdir, end);	// was 8192.0
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

	if (tr.startsolid && !tr.ent->client)
		return;

//	As long as we're not firing point-blank at something, draw the tracer beam.

	if (!tr.startsolid)
	{
		edict_t	*beam = G_Spawn();

		beam->movetype = MOVETYPE_NOCLIP;
		beam->solid = SOLID_NOT;
		beam->s.renderfx |= RF_BEAM; 
		beam->s.modelindex = 1;
		beam->s.frame = 2;
		beam->s.skinnum = (disrupt)?0xf0f2f2f2:0xf1f3f1f1;
		beam->think = G_FreeEdict;
		beam->nextthink = level.time + (1.1 * FRAMETIME);		// give it enough time to be seen

		VectorCopy(start, beam->s.origin);
        VectorSubtract(tr.endpos, start, vbeam);
		dist = 10.0 * VectorLength(vbeam);
		VectorScale(aimdir, dist, beam->velocity);

		gi.linkentity(beam);
	}

//	A living player is a valid target.

	if (tr.ent->client && tr.ent->inuse && (tr.ent->health > 0))	
	{
		VectorSubtract(tr.endpos, start, vbeam);

//		If the target is currently using an AGM on us, cut our AGM beam and get damaged.

		if (tr.ent->client->agm_target && (tr.ent->client->agm_target == self))
		{
			damage = (int)sv_agm_cross_damage->value;
			if (self->client->quad_framenum > level.framenum)
				damage *= (int)sv_quad_factor->value;
			T_Damage(self, tr.ent, tr.ent, vbeam, start, vec3_origin, damage, -damage, 0, MOD_AGM_FEEDBACK);
			self->client->agm_target = NULL;
			self->client->agm_charge = 0;
			gi.sound(self, CHAN_ITEM, gi.soundindex("weapons/agm/agm_cross.wav"), 1, ATTN_NORM, 0);
		}

//		The beam also cuts out if our target is currently being held by someone else's AGM, 
//		and we get damaged if the AGM wielder is not on our team.

		else if (tr.ent->client->held_by_agm)
		{
			if (!CheckTeamDamage(tr.ent->client->agm_enemy, self))
			{
				damage = (int)sv_agm_cross_damage->value;
				if (self->client->quad_framenum > level.framenum)
					damage *= (int)sv_quad_factor->value;
				T_Damage(self, tr.ent->client->agm_enemy, tr.ent->client->agm_enemy, vbeam, start, vec3_origin, damage, -damage, 0, MOD_AGM_FEEDBACK);
			}
			self->client->agm_target = NULL;
			self->client->agm_charge = 0;
			gi.sound(self, CHAN_ITEM, gi.soundindex("weapons/agm/agm_cross.wav"), 1, ATTN_NORM, 0);
		}

//		If our target has a Beam Reflector, we get bounced back and hurt if they're not on our team.

		else if (tr.ent->client->antibeam_framenum > level.framenum)
		{
			if (!CheckTeamDamage(tr.ent, self))
			{
				damage = (int)sv_agm_reflect_damage->value;
				if (self->client->quad_framenum > level.framenum)
					damage *= (int)sv_quad_factor->value;
				T_Damage(self, tr.ent, tr.ent, vbeam, start, vec3_origin, damage, -200, 0, MOD_AGM_BEAM_REF);
			}
			self->client->agm_target = NULL;
			self->client->agm_charge = 0;
			gi.sound(tr.ent, CHAN_ITEM, gi.soundindex("ctf/tech1.wav"), 1, ATTN_NORM, 0);
		}
		else
		{

//			Do cellular disruption effect rather than manipulation, if flagged.
//			NB: armour doesn't protect against this effect.

			if (disrupt)
			{
				if (!CheckTeamDamage(tr.ent, self))
				{
					damage = (int)sv_agm_disrupt_damage->value;
					kick = -20;

//					Apply powerup and Tech effects.

					if (self->client->quad_framenum > level.framenum)
					{
						damage *= (int)sv_quad_factor->value;
						kick *= (int)sv_quad_factor->value;
					}

					if ((self->client->haste_framenum > level.framenum) || CTFApplyHaste(self))
					{
						damage *= 2;
						kick *= 2;
					}

					T_Damage(tr.ent, self, self, vbeam, tr.endpos, tr.plane.normal, damage, kick, (DAMAGE_ENERGY | DAMAGE_NO_ARMOR), MOD_AGM_DISRUPT);
				}

				return;
			}

//			As long as they don't have the Invulnerability, or are on our team, they're ours to play with
//			like a cheap toy.

			if ((tr.ent->client->invincible_framenum > level.framenum) && ((int)sv_agm_invuln_cells->value < 0))
				return;

			if (CheckTeamDamage(tr.ent, self))
			{
				self->client->agm_target = NULL;
				return;
			}

			if (tr.ent->target_ent && tr.ent->target_ent->client && tr.ent->target_ent->client->spycam)
			{
				tr.ent = tr.ent->target_ent;
				camera_off(tr.ent);
			}

			self->client->agm_target = tr.ent;
			self->client->agm_range = VectorLength(vbeam);
			if ((self->client->agm_range < AGM_RANGE_MIN) || tr.startsolid)
				self->client->agm_range = AGM_RANGE_MIN;

			tr.ent->client->agm_enemy = self;
			tr.ent->client->held_by_agm = true;
			tr.ent->client->flung_by_agm = false;
			tr.ent->client->thrown_by_agm = false;
							
			if (tr.ent->isabot)
				tr.ent->client->movestate |= STS_AGMMOVE;

			gi.sound(self, CHAN_WEAPON, gi.soundindex("brain/melee3.wav"), 1, ATTN_NORM, 0);
			gi.sound(tr.ent, CHAN_BODY, gi.soundindex("brain/melee3.wav"), 1, ATTN_NORM, 0);
			gi_centerprintf(tr.ent, "You're being manipulated by %s\n", self->client->pers.netname);
		}
	}
}


/*
======================================================================
DISC LAUNCHER subroutines
======================================================================
*/
void Disc_Pop(edict_t *self)
{
	int n;

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SPARKS);
	gi.WritePosition(self->s.origin);
	gi.WriteDir(vec3_origin);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	n = rand() % 4;
	while (n--)
		ThrowDebris(self, "models/objects/debris2/tris.md2", 2, self->s.origin);

	G_FreeEdict(self);
}

void Disc_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	Vanish if we hit a sky surface.

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

//	Ignore weapon projectiles.

	if (other->wep_proj)
		return;

//	If it's a player then damage them, otherwise ricochet.

	if (!other->takedamage)
	{
		int r = 1 + (rand() % 3);
		gi.sound(self, CHAN_VOICE, gi.soundindex(va("weapons/disc/ric%i.wav", r)), 1, ATTN_NORM, 0);

		if (self->owner && self->owner->client)
			PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_SPARKS);
		gi.WritePosition(self->s.origin);
		if (!plane)
			gi.WriteDir(vec3_origin);
		else
			gi.WriteDir(plane->normal);
		gi.multicast(self->s.origin, MULTICAST_PVS);

		self->owner = self;		// reset for collision detection
	}
	else
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/disc/hit.wav"), 1, ATTN_NORM, 0);
		if (self->owner && self->owner->client)
			PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

		T_Damage(other, self, self->oldenemy, self->velocity, self->s.origin, plane->normal, self->dmg, self->count, 0, MOD_DISC);
		G_FreeEdict(self);
	}
}

void Fire_Disc(edict_t *self, vec3_t start, vec3_t aimdir, int damage, float speed, int kick)
{
	edict_t	*disc;

	disc = G_Spawn();
	disc->solid = SOLID_BBOX;
	disc->movetype = MOVETYPE_FLYBOUNCE;
	disc->clipmask = MASK_SHOT;
	disc->svflags = SVF_DEADMONSTER;
	disc->s.effects |= EF_GRENADE;
	disc->s.renderfx = RF_FULLBRIGHT;
	disc->dmg = damage;
	disc->count = kick;
	disc->classname = "disc";
	disc->wep_proj = true;
	disc->oldenemy = self;			// used for T_Damage() call
	disc->owner = self;
	disc->touch = Disc_Touch;
	disc->think = Disc_Pop;
	disc->nextthink = level.time + sv_disc_live_time->value;

	disc->model = "models/objects/disc/tris.md2";
	disc->s.modelindex = gi.modelindex(disc->model);
	VectorSet(disc->mins, -8.0, -8.0, -2.0);
	VectorSet(disc->maxs, 8.0, 8.0, 2.0);

	VectorCopy(start, disc->s.origin);
	VectorCopy(start, disc->s.old_origin);
	vectoangles(aimdir, disc->s.angles);
	VectorScale(aimdir, speed, disc->velocity);

	gi.linkentity(disc);
}


/*
======================================================================
CHAINSAW subroutines
======================================================================
*/
void Fire_Chainsaw(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	trace_t		tr;
	vec3_t		end;
	vec3_t		dmgdir;

//	Trace a line from the start point to an imaginary chainsaw tip (it actually extends further
//	than a chainsaw would, in order to make the weapon useful). If any entities are detected
//	then damage them if possible, otherwise draw a shower of sparks.

	VectorMA(start, RANGE_CHAINSAW, aimdir, end);
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		if (!tr.ent->takedamage)
		{
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_SPARKS);
			gi.WritePosition(tr.endpos);
			if (!tr.plane.normal)
				gi.WriteDir(vec3_origin);
			else
				gi.WriteDir(tr.plane.normal);
			gi.multicast(tr.endpos, MULTICAST_PVS);
		}
		else
		{

//			Prevent dead bodies from being pulled off the ground.

			if (tr.ent->die == body_die)
				kick = 0;

//			Set damage direction to be towards the attacking player, and apply damage.

			VectorNegate(aimdir, dmgdir);
			T_Damage(tr.ent, self, self, dmgdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_CHAINSAW);
			
//			If it's other players or dead bodies that are being chainsawed, produce lots of blood and gibs!

			if (tr.ent->client || (tr.ent->die == body_die))
			{
				gi.WriteByte(svc_temp_entity);
				gi.WriteByte(TE_MOREBLOOD);
				gi.WritePosition(tr.ent->s.origin);
				gi.WriteDir(vec3_up);
				gi.multicast(tr.ent->s.origin, MULTICAST_PVS);

				if (random() < 0.5)
					ThrowGib(tr.ent, "models/objects/gibs/sm_meat/tris.md2", 50, GIB_ORGANIC, 1.0);
			}
		}
	}

//	Destroy nearby Traps and C4 bundles (as gi.trace will ignore the player's own entities).

	if (self->next_node)
	{
		edict_t		*check;
		edict_t		*index;
		vec3_t		entdir;
		qboolean	finished = false;

		index = self->next_node;
		while (index && !finished)
		{
			check = index;
			if (index->next_node)
				index = index->next_node;
			else
				finished = true;

			VectorSubtract(check->s.origin, start, entdir);
			VectorNormalize(entdir);
			if ((VecRange(start, check->s.origin) < RANGE_CHAINSAW) && (DotProduct(entdir, aimdir) > 0.9))
			{
				if (check->die == C4_DieFromDamage)
					C4_Die(check);
				else if (check->die == Trap_DieFromDamage)
					Trap_Die(check);
				else
					gi.dprintf("BUG: Invalid next_node pointer in Fire_Chainsaw().\nPlease contact musashi@planetquake.com\n");
			}
		}
	}
}


/*
======================================================================
MG TRACER ROUND subroutines
======================================================================
*/
void Tracer_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (self == other)
		return;

	if (other->classname && (!Q_stricmp(other->classname, "worldspawn") || !Q_stricmp(other->classname, "player")))
		G_FreeEdict(self);
}

void Fire_Tracer(edict_t *self, vec3_t start, vec3_t aimdir, float speed, float lifetime)
{
	edict_t	*tracer;

	tracer = G_Spawn();
	tracer->svflags = SVF_DEADMONSTER | SVF_PROJECTILE;
	tracer->solid = SOLID_TRIGGER;
	tracer->movetype = MOVETYPE_FLY;
	tracer->clipmask = 0;
	tracer->classname = "tracer";
	tracer->wep_proj = true;
	tracer->s.renderfx |= RF_FULLBRIGHT;
	tracer->touch = Tracer_Touch;
	tracer->think = G_FreeEdict;
	tracer->nextthink = level.time + lifetime;

	tracer->model = "models/objects/tracer/tris.md2";
	tracer->s.modelindex = tracer_index;

	VectorCopy(start, tracer->s.origin);
	VectorCopy(start, tracer->s.old_origin);
	vectoangles(aimdir, tracer->s.angles);
	VectorScale(aimdir, speed, tracer->velocity);

	gi.linkentity(tracer);
}


/*
=================
PLASMA BOLT subroutines
=================
*/
void Plasma_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{

//	Ignore ourself and our owner.

	if (other == self)
		return;

	if (other == self->owner)
		return;

//	Ignore weapon projectiles.

	if (other->wep_proj)
		return;

//	Vanish if we hit a sky surface.

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

//	Apply damage if appropriate.

	if (other->takedamage)
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 5, DAMAGE_ENERGY, MOD_PLASMA);
	else
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_SHIELD_SPARKS);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir(vec3_origin);
		else
			gi.WriteDir(plane->normal);
		gi.multicast(self->s.origin, MULTICAST_PVS);
	}

	gi.sound(self, CHAN_WEAPON, gi.soundindex("world/explod2.wav"), 1, ATTN_NORM, 0);
	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	G_FreeEdict(self);
}

void Fire_Plasma(edict_t *self, vec3_t start, vec3_t aimdir, int damage, float speed)
{
	edict_t	*bolt;
	trace_t	tr;

	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER | SVF_PROJECTILE;
	bolt->solid = SOLID_TRIGGER;
	bolt->movetype = MOVETYPE_FLY;
	bolt->clipmask = MASK_SHOT;
	bolt->s.effects = EF_BLUEHYPERBLASTER;
	bolt->s.renderfx = RF_FULLBRIGHT;
	bolt->dmg = damage;
	bolt->classname = "plasma_bolt";
	bolt->owner = self;
	bolt->touch = Plasma_Touch;
	bolt->think = G_FreeEdict;
	bolt->nextthink = level.time + PLASMA_LIVETIME;

	bolt->s.modelindex = gi.modelindex("sprites/s_plasma.sp2");
	bolt->s.sound = gi.soundindex("misc/lasfly.wav");
	bolt->s.frame = 0;
	bolt->s.skinnum = 0;

	VectorCopy(start, bolt->s.origin);
	VectorCopy(start, bolt->s.old_origin);
	vectoangles(aimdir, bolt->s.angles);
	VectorScale(aimdir, speed, bolt->velocity);

	gi.linkentity(bolt);

	tr = gi.trace(self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA(bolt->s.origin, -10.0, aimdir, bolt->s.origin);
		bolt->touch(bolt, tr.ent, NULL, NULL);
	}
}
//CW--
