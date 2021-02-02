// g_misc.c

#include "g_local.h"


/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.
*/

//=====================================================

void Use_Areaportal(edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->count ^= 1;
	gi.SetAreaPortalState(ent->style, ent->count);
}

/*QUAKED func_areaportal (0 0 0) ?

This is a non-visible object that divides the world into
areas that are seperated when this portal is not activated.
Usually enclosed in the middle of a door.
*/
void SP_func_areaportal(edict_t *ent)
{
	ent->use = Use_Areaportal;
	ent->count = 0;						// always start closed
}

//=====================================================


/*
=================
Misc functions
=================
*/
void VelocityForDamage(int damage, vec3_t v)
{
	v[0] = 100.0 * crandom();
	v[1] = 100.0 * crandom();
	v[2] = 200.0 + (100.0 * random());

	if (damage < 50)
		VectorScale(v, 0.7, v);
	else 
		VectorScale(v, 1.2, v);
}

void ClipGibVelocity(edict_t *ent)
{
	if (ent->velocity[0] < -300.0)
		ent->velocity[0] = -300.0;
	else if (ent->velocity[0] > 300.0)
		ent->velocity[0] = 300.0;

	if (ent->velocity[1] < -300.0)
		ent->velocity[1] = -300.0;
	else if (ent->velocity[1] > 300.0)
		ent->velocity[1] = 300.0;

	if (ent->velocity[2] < 200.0)
		ent->velocity[2] = 200.0;			// always some upwards
	else if (ent->velocity[2] > 500.0)
		ent->velocity[2] = 500.0;
}


/*
=================
gibs
=================
*/
void gib_think(edict_t *self)
{
	self->s.frame++;
	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame == 10)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 1.0;															//CW: was 8 + random()*10
	}
}

void gib_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	normal_angles;
	vec3_t	right;

	if (!self->groundentity)
		return;

	self->touch = NULL;

	if (plane)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex ("misc/fhit3.wav"), 1, ATTN_NORM, 0);

		vectoangles(plane->normal, normal_angles);
		AngleVectors(normal_angles, NULL, right, NULL);
		vectoangles(right, self->s.angles);

		if (self->s.modelindex == sm_meat_index)
		{
			self->s.frame++;
			self->think = gib_think;
			self->nextthink = level.time + FRAMETIME;
		}
	}
}

void gib_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict(self);
}

void ThrowGib(edict_t *self, char *gibname, int damage, int type, float livetime)
{
	edict_t *gib;
	vec3_t	vd;
	vec3_t	origin;
	vec3_t	size;
	float	vscale;

	gib = G_Spawn();

	VectorScale(self->size, 0.5, size);
	VectorAdd(self->absmin, size, origin);
	gib->s.origin[0] = origin[0] + (crandom() * size[0]);
	gib->s.origin[1] = origin[1] + (crandom() * size[1]);
	gib->s.origin[2] = origin[2] + (crandom() * size[2]);

	gi.setmodel(gib, gibname);
	gib->solid = SOLID_NOT;
	gib->s.effects |= EF_GIB;
	gib->flags |= FL_NO_KNOCKBACK;
	gib->svflags |= SVF_GIB; // Knightmare- added gib flag
	gib->takedamage = DAMAGE_YES;
	gib->die = gib_die;

	if (type == GIB_ORGANIC)
	{
		gib->movetype = MOVETYPE_TOSS;
		gib->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		gib->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	VelocityForDamage(damage, vd);
	VectorMA(self->velocity, vscale, vd, gib->velocity);
	ClipGibVelocity(gib);
	gib->avelocity[0] = random() * 600.0;
	gib->avelocity[1] = random() * 600.0;
	gib->avelocity[2] = random() * 600.0;

//CW++
	if (livetime < FRAMETIME)
		livetime = 2.0 + (random() * 4.0);
//CW--

	gib->think = G_FreeEdict;
	gib->nextthink = level.time + livetime;															//CW

	gi.linkentity(gib);
}

void ThrowHead(edict_t *self, char *gibname, int damage, int type)
{
	vec3_t	vd;
	float	vscale;

	self->s.skinnum = 0;
	self->s.frame = 0;
	VectorClear(self->mins);
	VectorClear(self->maxs);
	self->s.modelindex2 = 0;
//CW++
	self->s.modelindex3 = 0;
	self->s.modelindex4 = 0;
//CW--

	gi.setmodel(self, gibname);
	self->solid = SOLID_NOT;
	self->s.effects |= EF_GIB;
	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;
	self->svflags |= SVF_GIB; // Knightmare- added gib flag
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

	VelocityForDamage(damage, vd);
	VectorMA(self->velocity, vscale, vd, self->velocity);
	ClipGibVelocity(self);
	self->avelocity[YAW] = crandom() * 600.0;

	self->think = G_FreeEdict;
	self->nextthink = level.time + 10.0 + (random()*10.0);

	gi.linkentity(self);
}


void ThrowClientHead(edict_t *self, int damage)
{
	vec3_t	vd;
	char	*gibname;

//CW++
	if (self->disintegrated)
		gibname = "sprites/point.sp2";
	else
	{
//CW--
		if (rand() & 1)
		{
			gibname = "models/objects/gibs/head2/tris.md2";
			self->s.skinnum = 1;		// second skin is player
		}
		else
		{
			gibname = "models/objects/gibs/skull/tris.md2";
			self->s.skinnum = 0;
		}
	}
	
	self->s.origin[2] += 32.0;
	self->s.frame = 0;
	gi.setmodel(self, gibname);
	VectorSet(self->mins, -16.0, -16.0, 0.0);
	VectorSet(self->maxs, 16.0, 16.0, 16.0);

	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->s.effects = EF_GIB;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;
	self->svflags |= SVF_GIB; // Knightmare- added gib flag

//CW++
	if (self->disintegrated)
	{
		self->movetype = MOVETYPE_TOSS;
		VectorClear(self->velocity);
		self->disintegrated = false;
	}
	else
	{
//CW--
		self->movetype = MOVETYPE_BOUNCE;
		VelocityForDamage(damage, vd);
		VectorAdd(self->velocity, vd, self->velocity);
	}

	if (self->client)	// bodies in the queue don't have a client anymore
	{
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = self->s.frame;
	}

	gi.linkentity(self);
}


/*
=================
debris
=================
*/
void debris_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict(self);
}

void ThrowDebris(edict_t *self, char *modelname, float speed, vec3_t origin)
{
	edict_t	*chunk;
	vec3_t	v;

	chunk = G_Spawn();
	VectorCopy(origin, chunk->s.origin);
	gi.setmodel(chunk, modelname);
	v[0] = 100.0 * crandom();
	v[1] = 100.0 * crandom();
	v[2] = 100.0 + (100.0 * crandom());
	VectorMA(self->velocity, speed, v, chunk->velocity);
	chunk->movetype = MOVETYPE_BOUNCE;
	chunk->solid = SOLID_NOT;
	chunk->avelocity[0] = random() * 600;
	chunk->avelocity[1] = random() * 600;
	chunk->avelocity[2] = random() * 600;
	chunk->think = G_FreeEdict;
	chunk->nextthink = level.time + 1.0;															//CW
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "debris";
	chunk->svflags |= SVF_GIB; // Knightmare- gib flag
	chunk->takedamage = DAMAGE_YES;
	chunk->die = debris_die;
	gi.linkentity(chunk);
}


void BecomeExplosion1(edict_t *self)
{
//ZOID++
	//flags are important
	if (strcmp(self->classname, "item_flag_team1") == 0)
	{
		CTFResetFlag(CTF_TEAM1); // this will free self!
		gi_bprintf(PRINT_HIGH, "The %s flag has returned!\n", CTFTeamName(CTF_TEAM1));
		return;
	}
	if (strcmp(self->classname, "item_flag_team2") == 0)
	{
		CTFResetFlag(CTF_TEAM2); // this will free self!
		gi_bprintf(PRINT_HIGH, "The %s flag has returned!\n", CTFTeamName(CTF_TEAM1));
		return;
	}
	// techs are important too
	if (self->item && (self->item->flags & IT_TECH))
	{
		CTFRespawnTech(self); // this will free self!
		return;
	}
//ZOID--

//CW++
//	Entities that are dead player bodies should not explode or be freed.

	if (self->classname && !Q_stricmp(self->classname, "bodyque"))
		return;
//CW--

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_EXPLOSION1);
	gi.WritePosition(self->s.origin);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	G_FreeEdict(self);
}

void BecomeExplosion2(edict_t *self)
{
//CW++
//	Entities that are dead player bodies should not explode or be freed.

	if (self->classname && !Q_stricmp(self->classname, "bodyque"))
		return;
//CW--

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_EXPLOSION2);
	gi.WritePosition(self->s.origin);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	G_FreeEdict(self);
}


/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
Target: next path corner
Pathtarget: gets used when an entity that has
	this path_corner targeted touches it
*/

void path_corner_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		G_UseTargets(self, other);
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
		VectorCopy(v, other->s.origin);
		next = G_PickTarget(next->target);
	}

	other->goalentity = other->movetarget = next;

	if (self->wait)
	{
		other->monsterinfo.pausetime = level.time + self->wait;
		other->monsterinfo.stand(other);
		return;
	}

	if (!other->movetarget)
	{
		other->monsterinfo.pausetime = level.time + 100000000.0;
		other->monsterinfo.stand(other);
	}
	else
	{
		VectorSubtract(other->goalentity->s.origin, other->s.origin, v);
		other->ideal_yaw = vectoyaw(v);
	}
}

void SP_path_corner(edict_t *self)
{
	if (!self->targetname)
	{
		gi.dprintf("path_corner with no targetname at %s\n", vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	self->solid = SOLID_TRIGGER;
	self->touch = path_corner_touch;
	VectorSet(self->mins, -8.0, -8.0, -8.0);
	VectorSet(self->maxs, 8.0, 8.0, 8.0);
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity(self);
}


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for spotlights, etc.
*/
void SP_info_null(edict_t *self)
{
	G_FreeEdict(self);
};


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for lightning, turrets, etc.
*/
void SP_info_notnull(edict_t *self)
{
	VectorCopy(self->s.origin, self->absmin);
	VectorCopy(self->s.origin, self->absmax);

//CW++
	self->svflags |= SVF_NOCLIENT;

//	Avert your eyes - it's ugly hack time!

	if (!Q_stricmp(level.mapname, "awaken2_assault2") && self->targetname)
	{
		if (!Q_stricmp(self->targetname, "turret1_target"))
			self->s.origin[2] = 298;
		else if (!Q_stricmp(self->targetname, "turret111_target"))
			self->s.origin[2] = 292;

		gi.linkentity(self);
	}
//CW--
};

//CW++
/*QUAKED info_timelimit (0 0.5 0.5) (-8 -8 -8) (8 8 8)
Used to set the timelimit in an Assault map.
*/
void SP_info_timelimit(edict_t *self)
{
	if (self->wait <= 0.0)
		gi.dprintf ("info_timelimit with invalid 'wait' value at %s\n", vtos(self->s.origin));
	else
		gi.cvar_forceset("timelimit", va("%f", self->wait));
	
	G_FreeEdict(self);
};

/*QUAKED info_mission (0 1 0.5) (-8 -8 -8) (8 8 8)
Describes a mission objective for an Assault map.
*/
void SP_info_mission(edict_t *self)
{
	if (sv_gametype->value != G_ASLT)
	{
		G_FreeEdict(self);
		return;
	}

	if (!self->message)
	{
		gi.dprintf("%s with no message at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	self->svflags |= SVF_NOCLIENT;

//	Avert your eyes - it's ugly hack time!

	if (!Q_stricmp(level.mapname, "awaken2_assault3"))
	{
		if (self->targetname && !Q_stricmp(self->targetname, "obj3") && self->target && !Q_stricmp(self->target, "obj3"))
			self->target = NULL;
	}
}

/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used to override the current server setting and disable the
use of the grappling hook in the level.
*/
void SP_info_nohook(edict_t *self)
{
	level.nohook = true;
	G_FreeEdict(self);
};
//CW--


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) START_OFF
Non-displayed light.
Default light value is 300.
Default style is 0.
If targeted, will toggle between on and off.
Default _cone value is 10 (used to set size of light for spotlights)
*/
void SP_light(edict_t *self)
{
//	No targeted lights in deathmatch, because they cause global messages.

	G_FreeEdict(self);
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

void func_wall_use(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
	{
		self->solid = SOLID_BSP;
		self->svflags &= ~SVF_NOCLIENT;
		KillBox(self);
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}

	gi.linkentity(self);

	if (!(self->spawnflags & 2))
		self->use = NULL;
}

void SP_func_wall(edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel(self, self->model);

	if (self->spawnflags & 8)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 16)
		self->s.effects |= EF_ANIM_ALLFAST;

	// just a wall
	if ((self->spawnflags & 7) == 0)
	{
		self->solid = SOLID_BSP;
		gi.linkentity(self);
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
		self->solid = SOLID_BSP;
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}

	gi.linkentity(self);
}


/*QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
This is solid bmodel that will fall if it's support it removed.
*/

void func_object_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// only squash thing we fall on top of
	if (!plane)
		return;
	if (plane->normal[2] < 1.0)
		return;

	if (other->takedamage == DAMAGE_NO)
		return;

	T_Damage(other, self, self, vec3_origin, self->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void func_object_release(edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->touch = func_object_touch;
}

void func_object_use(edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox(self);
	func_object_release(self);
}

void SP_func_object(edict_t *self)
{
	gi.setmodel(self, self->model);

	self->mins[0] += 1.0;
	self->mins[1] += 1.0;
	self->mins[2] += 1.0;
	self->maxs[0] -= 1.0;
	self->maxs[1] -= 1.0;
	self->maxs[2] -= 1.0;

	if (!self->dmg)
		self->dmg = 100;

	if (self->spawnflags == 0)
	{
		self->solid = SOLID_BSP;
		self->movetype = MOVETYPE_PUSH;
		self->think = func_object_release;
		self->nextthink = level.time + (2.0 * FRAMETIME);
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

	gi.linkentity(self);
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
void func_explosive_explode(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t	origin;
	vec3_t	chunkorigin;
	vec3_t	size;
	int		count;
	int		mass;

	// bmodel origins are (0 0 0), we need to adjust that here
	VectorScale(self->size, 0.5, size);
	VectorAdd(self->absmin, size, origin);
	VectorCopy(origin, self->s.origin);

	self->takedamage = DAMAGE_NO;

	if (self->dmg)
		T_RadiusDamage(self, attacker, self->dmg, NULL, self->dmg+40, MOD_EXPLOSIVE);

	VectorSubtract(self->s.origin, inflictor->s.origin, self->velocity);
	VectorNormalize(self->velocity);
	VectorScale(self->velocity, 150.0, self->velocity);

	// start chunks towards the center
	VectorScale(size, 0.5, size);

	mass = self->mass;
	if (!mass)
		mass = 75;

	// big chunks
	if (mass >= 100)
	{
		count = (int)(0.01 * mass);
		if (count > 8)
			count = 8;
		while (count--)
		{
			chunkorigin[0] = origin[0] + crandom() * size[0];
			chunkorigin[1] = origin[1] + crandom() * size[1];
			chunkorigin[2] = origin[2] + crandom() * size[2];
			ThrowDebris(self, "models/objects/debris1/tris.md2", 1, chunkorigin);
		}
	}

	// small chunks
	count = (int)(0.04 * mass);
	if (count > 16)
		count = 16;
	while (count--)
	{
		chunkorigin[0] = origin[0] + crandom() * size[0];
		chunkorigin[1] = origin[1] + crandom() * size[1];
		chunkorigin[2] = origin[2] + crandom() * size[2];
		ThrowDebris(self, "models/objects/debris2/tris.md2", 2, chunkorigin);
	}

	G_UseTargets(self, attacker);

	if (self->dmg)
		BecomeExplosion1(self);
	else
		G_FreeEdict(self);
}

void func_explosive_use(edict_t *self, edict_t *other, edict_t *activator)
{
	func_explosive_explode(self, self, other, self->health, vec3_origin);
}

void func_explosive_spawn(edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox(self);
	gi.linkentity(self);
}

void SP_func_explosive(edict_t *self)
{
	if (sv_gametype->value != G_ASLT)																//CW: OK for Assault games
	{
		G_FreeEdict(self);
		return;
	}

	self->movetype = MOVETYPE_PUSH;

	gi.modelindex("models/objects/debris1/tris.md2");
	gi.modelindex("models/objects/debris2/tris.md2");
	gi.setmodel(self, self->model);

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

	gi.linkentity(self);
}


/*QUAKED misc_explobox (0 .5 .8) (-16 -16 0) (16 16 40)
Large exploding box.  You can override its mass (100),
health (80), and dmg (150).
*/
void barrel_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float	ratio;
	vec3_t	v;

	if (!other->groundentity || (other->groundentity == self))
		return;

	ratio = (float)other->mass / (float)self->mass;
	VectorSubtract(self->s.origin, other->s.origin, v);
	M_WalkMove(self, vectoyaw(v), 20.0 * ratio * FRAMETIME);
}

void barrel_explode(edict_t *self)
{
	vec3_t	org;
	vec3_t	save;
	float	spd;

	T_RadiusDamage(self, self->activator, self->dmg, NULL, self->dmg+40, MOD_BARREL);

	VectorCopy(self->s.origin, save);
	VectorMA(self->absmin, 0.5, self->size, self->s.origin);

	// a few big chunks
	spd = 1.5 * (float)self->dmg / 200.0;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org);

	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org);

	// bottom corners
	spd = 0.00875 * (float)self->dmg;
	VectorCopy(self->absmin, org);
	ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);

	VectorCopy(self->absmin, org);
	org[0] += self->size[0];
	ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);

	VectorCopy(self->absmin, org);
	org[1] += self->size[1];
	ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);

	VectorCopy(self->absmin, org);
	org[0] += self->size[0];
	org[1] += self->size[1];
	ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);

	// a bunch of little chunks
	spd = 0.01 * self->dmg;
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

	VectorCopy(save, self->s.origin);
	if (self->groundentity)
		BecomeExplosion2(self);
	else
		BecomeExplosion1(self);
}

void barrel_delay(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + (2.0 * FRAMETIME);
	self->think = barrel_explode;
	self->activator = attacker;
}

void SP_misc_explobox(edict_t *self)
{
	if (sv_gametype->value != G_ASLT)																//CW: OK for Assault games
	{
		G_FreeEdict(self);
		return;
	}

	gi.modelindex("models/objects/debris1/tris.md2");
	gi.modelindex("models/objects/debris2/tris.md2");
	gi.modelindex("models/objects/debris3/tris.md2");

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_STEP;
	self->model = "models/objects/barrels/tris.md2";
	self->s.modelindex = gi.modelindex (self->model);
	VectorSet(self->mins, -16.0, -16.0, 0.0);
	VectorSet(self->maxs, 16.0, 16.0, 40.0);

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
	self->think = M_DropToFloor;
	self->nextthink = level.time + (2.0 * FRAMETIME);

	gi.linkentity(self);
}


//
// miscellaneous specialty items
//

/*QUAKED misc_blackhole (1 .5 0) (-8 -8 -8) (8 8 8)
*/
void misc_blackhole_use(edict_t *ent, edict_t *other, edict_t *activator)
{
	G_FreeEdict(ent);
}

void misc_blackhole_think(edict_t *self)
{
	if (++self->s.frame < 19)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 0;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_blackhole(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	VectorSet(ent->mins, -64.0, -64.0, 0.0);
	VectorSet(ent->maxs, 64.0, 64.0, 8.0);
	ent->s.modelindex = gi.modelindex("models/objects/black/tris.md2");
	ent->s.renderfx = RF_TRANSLUCENT;
	ent->use = misc_blackhole_use;
	ent->think = misc_blackhole_think;
	ent->nextthink = level.time + (2.0 * FRAMETIME);

	gi.linkentity(ent);
}

/*QUAKED misc_eastertank (1 .5 0) (-32 -32 -16) (32 32 32)
*/
void misc_eastertank_think(edict_t *self)
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
	VectorSet(ent->mins, -32.0, -32.0, -16.0);
	VectorSet(ent->maxs, 32.0, 32.0, 32.0);
	ent->s.modelindex = gi.modelindex("models/monsters/tank/tris.md2");
	ent->s.frame = 254;
	ent->think = misc_eastertank_think;
	ent->nextthink = level.time + (2.0 * FRAMETIME);

	gi.linkentity(ent);
}

/*QUAKED misc_easterchick (1 .5 0) (-32 -32 0) (32 32 32)
*/
void misc_easterchick_think(edict_t *self)
{
	if (++self->s.frame < 247)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 208;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_easterchick(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet(ent->mins, -32.0, -32.0, 0.0);
	VectorSet(ent->maxs, 32.0, 32.0, 32.0);
	ent->s.modelindex = gi.modelindex("models/monsters/bitch/tris.md2");
	ent->s.frame = 208;
	ent->think = misc_easterchick_think;
	ent->nextthink = level.time + (2.0 * FRAMETIME);

	gi.linkentity(ent);
}

/*QUAKED misc_easterchick2 (1 .5 0) (-32 -32 0) (32 32 32)
*/
void misc_easterchick2_think(edict_t *self)
{
	if (++self->s.frame < 287)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 248;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_easterchick2(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet(ent->mins, -32.0, -32.0, 0.0);
	VectorSet(ent->maxs, 32.0, 32.0, 32.0);
	ent->s.modelindex = gi.modelindex("models/monsters/bitch/tris.md2");
	ent->s.frame = 248;
	ent->think = misc_easterchick2_think;
	ent->nextthink = level.time + (2.0 * FRAMETIME);

	gi.linkentity(ent);
}


/*QUAKED monster_commander_body (1 .5 0) (-32 -32 0) (32 32 48)
Not really a monster, this is the Tank Commander's decapitated body.
There should be a item_commander_head that has this as it's target.
*/
void commander_body_think(edict_t *self)
{
	if (++self->s.frame < 24)
		self->nextthink = level.time + FRAMETIME;
	else
		self->nextthink = 0.0;

	if (self->s.frame == 22)
		gi.sound(self, CHAN_BODY, gi.soundindex("tank/thud.wav"), 1, ATTN_NORM, 0);
}

void commander_body_use(edict_t *self, edict_t *other, edict_t *activator)
{
	self->think = commander_body_think;
	self->nextthink = level.time + FRAMETIME;
	gi.sound(self, CHAN_BODY, gi.soundindex("tank/pain.wav"), 1, ATTN_NORM, 0);
}

void commander_body_drop(edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->s.origin[2] += 2.0;
}

void SP_monster_commander_body(edict_t *self)
{
	gi.soundindex("tank/thud.wav");
	gi.soundindex("tank/pain.wav");

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	self->model = "models/monsters/commandr/tris.md2";
	self->s.modelindex = gi.modelindex (self->model);
	VectorSet (self->mins, -32.0, -32.0, 0.0);
	VectorSet (self->maxs, 32.0, 32.0, 48.0);
	self->takedamage = DAMAGE_YES;
	self->flags = FL_GODMODE;
	self->s.renderfx |= RF_FRAMELERP;
	self->use = commander_body_use;
	self->think = commander_body_drop;
	self->nextthink = level.time + (5.0 * FRAMETIME);

	gi.linkentity (self);
}


/*QUAKED misc_banner (1 .5 0) (-4 -4 -4) (4 4 4)
The origin is the bottom of the banner.
The banner is 128 tall.
*/
void misc_banner_think(edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_banner(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex("models/objects/banner/tris.md2");
	ent->s.frame = rand() % 16;
	ent->think = misc_banner_think;
	ent->nextthink = level.time + FRAMETIME;

	gi.linkentity(ent);
}


/*QUAKED misc_viper (1 .5 0) (-16 -16 0) (16 16 32)
This is the Viper for the flyby bombing.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"		How fast the Viper should fly
*/

extern void train_use(edict_t *self, edict_t *other, edict_t *activator);
extern void func_train_find(edict_t *self);

// Knightmare added
void viper_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point )
{
//	edict_t	*e, *next;

	if (self->deathtarget)
	{
		self->target = self->deathtarget;
		G_UseTargets (self, attacker);
	}

/*	e = self->movewith_next;
	while(e) {
		next = e->movewith_next;
		if (e->solid == SOLID_NOT) {
			e->nextthink = 0;
			G_FreeEdict(e);
		} else
			BecomeExplosion1 (e);
		e = next;
	}*/

	self->enemy = inflictor;
	self->activator = attacker;
	func_explosive_explode (self, inflictor, attacker, damage, point);
}
// end Knightmare

void misc_viper_use(edict_t *self, edict_t *other, edict_t *activator)
{
	self->svflags &= ~SVF_NOCLIENT;
	self->use = train_use;
	train_use(self, other, activator);
}

void SP_misc_viper(edict_t *ent)
{
	if (!ent->target)
	{
		gi.dprintf("misc_viper without a target at %s\n", vtos(ent->absmin));
		G_FreeEdict(ent);
		return;
	}

	if (!ent->speed)
		ent->speed = 300.0;

	ent->movetype = MOVETYPE_PUSH;
//	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex("models/ships/viper/tris.md2");


	// Knightmare added
	// Lazarus - allow ship to be destroyed if positive health value
	if (ent->health > 0)
	{
		ent->solid = SOLID_BBOX;
		VectorSet (ent->mins, -32, -24, -12);
		VectorSet (ent->maxs,  32,  24,  16);
		ent->takedamage = DAMAGE_YES;
		ent->die = viper_die;
		if (!ent->dmg)
			ent->dmg = 200;
		if (!ent->mass)
			ent->mass = 800;
	}
	else
	{
		ent->solid = SOLID_NOT;
		VectorSet(ent->mins, -16.0, -16.0, 0.0);
		VectorSet(ent->maxs, 16.0, 16.0, 32.0);
	}

	// Lazarus - TRAIN_SMOOTH forces trains to go directly to Move_Done from
	//           Move_Final rather than slowing down (if necessary) for one
	//           frame.
	if (ent->spawnflags & TRAIN_SMOOTH)
		ent->smooth_movement = true;
	else
		ent->smooth_movement = false;
	// end Knightmare

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;

	// Knightmare added
	if (ent->spawnflags & TRAIN_START_ON)
		ent->use = train_use;
	else {
		ent->use = misc_viper_use;
		ent->svflags |= SVF_NOCLIENT;
	}

	// Knightmare- change both rotate flags to spline flag
	if ((ent->spawnflags & TRAIN_ROTATE) && (ent->spawnflags & TRAIN_ROTATE_CONSTANT))
	{
		ent->spawnflags &= ~(TRAIN_ROTATE | TRAIN_ROTATE_CONSTANT);
		ent->spawnflags |= TRAIN_SPLINE;
	}
	// end Knightmare

	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	gi.linkentity(ent);
}


/*QUAKED misc_bigviper (1 .5 0) (-176 -120 -24) (176 120 72) 
This is a large stationary viper as seen in Paul's intro
*/
void SP_misc_bigviper(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet(ent->mins, -176.0, -120.0, -24.0);
	VectorSet(ent->maxs, 176.0, 120.0, 72.0);
	ent->s.modelindex = gi.modelindex("models/ships/bigviper/tris.md2");

	gi.linkentity(ent);
}

void SP_misc_viper_origin (edict_t *ent)
{
	ent->spawnflags |= TRAIN_ORIGIN;
	SP_misc_viper (ent);
}

/*QUAKED misc_viper_bomb (1 0 0) (-8 -8 -8) (8 8 8)
"dmg"	how much boom should the bomb make?
*/
void misc_viper_bomb_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	G_UseTargets(self, self->activator);

	self->s.origin[2] = self->absmin[2] + 1.0;
	T_RadiusDamage(self, self, self->dmg, NULL, self->dmg+40, MOD_BOMB);
	BecomeExplosion2(self);
}

void misc_viper_bomb_prethink(edict_t *self)
{
	vec3_t	v;
	float	diff;

	self->groundentity = NULL;

	diff = self->timestamp - level.time;
	if (diff < -1.0)
		diff = -1.0;

	VectorScale(self->moveinfo.dir, 1.0 + diff, v);
	v[2] = diff;

	diff = self->s.angles[2];
	vectoangles(v, self->s.angles);
	self->s.angles[2] = diff + 10.0;
}

void misc_viper_bomb_use(edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*viper;

	self->solid = SOLID_BBOX;
	self->svflags &= ~SVF_NOCLIENT;
	self->s.effects |= EF_ROCKET;
	self->use = NULL;
	self->movetype = MOVETYPE_TOSS;
	self->prethink = misc_viper_bomb_prethink;
	self->touch = misc_viper_bomb_touch;
	self->activator = activator;

	viper = G_Find(NULL, FOFS(classname), "misc_viper");
	VectorScale(viper->moveinfo.dir, viper->moveinfo.speed, self->velocity);

	self->timestamp = level.time;
	VectorCopy(viper->moveinfo.dir, self->moveinfo.dir);
}

void SP_misc_viper_bomb(edict_t *self)
{
	if (!self->dmg)
		self->dmg = 1000;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	VectorSet(self->mins, -8.0, -8.0, -8.0);
	VectorSet(self->maxs, 8.0, 8.0, 8.0);
	self->s.modelindex = gi.modelindex("models/objects/bomb/tris.md2");
	self->use = misc_viper_bomb_use;
	self->svflags |= SVF_NOCLIENT;

	gi.linkentity(self);
}


/*QUAKED misc_strogg_ship (1 .5 0) (-16 -16 0) (16 16 32)
This is a Storgg ship for the flybys.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"		How fast it should fly
*/
extern void train_use (edict_t *self, edict_t *other, edict_t *activator);
extern void func_train_find (edict_t *self);

void misc_strogg_ship_use(edict_t *self, edict_t *other, edict_t *activator)
{
	self->svflags &= ~SVF_NOCLIENT;
	self->use = train_use;
	train_use(self, other, activator);
}

void SP_misc_strogg_ship(edict_t *ent)
{
	if (!ent->target)
	{
		gi.dprintf("%s without a target at %s\n", ent->classname, vtos(ent->absmin));
		G_FreeEdict(ent);
		return;
	}

	if (!ent->speed)
		ent->speed = 300.0;

	ent->movetype = MOVETYPE_PUSH;
//	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex("models/ships/strogg1/tris.md2");

	// Knightmare added
	// Lazarus - allow ship to be destroyed if positive health value
	if (ent->health > 0)
	{
		ent->solid = SOLID_BBOX;
		VectorSet (ent->mins, -58, -60, -40);
		VectorSet (ent->maxs,  72,  60,  38);
		ent->takedamage = DAMAGE_YES;
		ent->die = viper_die;
		if (!ent->dmg)
			ent->dmg = 200;
		if (!ent->mass)
			ent->mass = 1200;
	}
	else
	{
		ent->solid = SOLID_NOT;
		VectorSet(ent->mins, -16.0, -16.0, 0.0);
		VectorSet(ent->maxs, 16.0, 16.0, 32.0);
	}

	// Lazarus - TRAIN_SMOOTH forces trains to go directly to Move_Done from
	//           Move_Final rather than slowing down (if necessary) for one
	//           frame.
	if (ent->spawnflags & TRAIN_SMOOTH)
		ent->smooth_movement = true;
	else
		ent->smooth_movement = false;
	// end Knightmare

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;

	// Knightmare added
	if (ent->spawnflags & TRAIN_START_ON)
		ent->use = train_use;
	else
	{
		ent->use = misc_strogg_ship_use;
		ent->svflags |= SVF_NOCLIENT;
	}

	// Knightmare- change both rotate flags to spline flag
	if ((ent->spawnflags & TRAIN_ROTATE) && (ent->spawnflags & TRAIN_ROTATE_CONSTANT))
	{
		ent->spawnflags &= ~(TRAIN_ROTATE | TRAIN_ROTATE_CONSTANT);
		ent->spawnflags |= TRAIN_SPLINE;
	}
	// end Knightmare

	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	gi.linkentity(ent);
}

void SP_misc_strogg_ship_origin (edict_t *ent)
{
	ent->spawnflags |= TRAIN_ORIGIN;
	SP_misc_strogg_ship (ent);
}

/*QUAKED misc_satellite_dish (1 .5 0) (-64 -64 0) (64 64 128)
*/
void misc_satellite_dish_think(edict_t *self)
{
	self->s.frame++;
	if (self->s.frame < 38)
		self->nextthink = level.time + FRAMETIME;
}

void misc_satellite_dish_use(edict_t *self, edict_t *other, edict_t *activator)
{
	self->s.frame = 0;
	self->think = misc_satellite_dish_think;
	self->nextthink = level.time + FRAMETIME;
}

void SP_misc_satellite_dish(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet(ent->mins, -64.0, -64.0, 0.0);
	VectorSet(ent->maxs, 64.0, 64.0, 128.0);
	ent->s.modelindex = gi.modelindex("models/objects/satellite/tris.md2");
	ent->use = misc_satellite_dish_use;

	gi.linkentity(ent);
}


/*QUAKED light_mine1 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine1(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex("models/objects/minelite/light1/tris.md2");

	gi.linkentity(ent);
}


/*QUAKED light_mine2 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine2(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex("models/objects/minelite/light2/tris.md2");

	gi.linkentity(ent);
}


/*QUAKED misc_gib_arm (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_arm(edict_t *ent)
{
	gi.setmodel(ent, "models/objects/gibs/arm/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random() * 200.0;
	ent->avelocity[1] = random() * 200.0;
	ent->avelocity[2] = random() * 200.0;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30.0;

	gi.linkentity(ent);
}

/*QUAKED misc_gib_leg (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_leg(edict_t *ent)
{
	gi.setmodel(ent, "models/objects/gibs/leg/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random() * 200.0;
	ent->avelocity[1] = random() * 200.0;
	ent->avelocity[2] = random() * 200.0;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30.0;

	gi.linkentity(ent);
}

/*QUAKED misc_gib_head (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_head(edict_t *ent)
{
	gi.setmodel(ent, "models/objects/gibs/head/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random() * 200.0;
	ent->avelocity[1] = random() * 200.0;
	ent->avelocity[2] = random() * 200.0;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30.0;

	gi.linkentity(ent);
}

//=====================================================

/*QUAKED target_character (0 0 1) ?
used with target_string (must be on same "team")
"count" is position in the string (starts at 1)
*/

void SP_target_character(edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel(self, self->model);
	self->solid = SOLID_BSP;
	self->s.frame = 12;

	gi.linkentity(self);
}


/*QUAKED target_string (0 0 1) (-8 -8 -8) (8 8 8)
*/

void target_string_use(edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *e;
	int		n;
	int		l;
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
		if ((c >= '0') && (c <= '9'))
			e->s.frame = c - '0';
		else if (c == '-')
			e->s.frame = 10;
		else if (c == ':')
			e->s.frame = 11;
		else
			e->s.frame = 12;
	}
}

void SP_target_string(edict_t *self)
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

static void func_clock_reset(edict_t *self)
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
		self->wait = 0.0;
	}
}

static void func_clock_format_countdown(edict_t *self)
{
	if (self->style == 0)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i", self->health);
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

void func_clock_think(edict_t *self)
{
	if (!self->enemy)
	{
		self->enemy = G_Find(NULL, FOFS(targetname), self->target);
		if (!self->enemy)
			return;
	}

	if (self->spawnflags & 1)
	{
		func_clock_format_countdown(self);
		self->health++;
	}
	else if (self->spawnflags & 2)
	{
		func_clock_format_countdown(self);
		self->health--;
	}
	else
	{
		struct tm	*ltime;
		time_t		gmtime;

		time(&gmtime);
		ltime = localtime(&gmtime);
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		if (self->message[6] == ' ')
			self->message[6] = '0';
	}

	self->enemy->message = self->message;
	self->enemy->use(self->enemy, self, self);

	if (((self->spawnflags & 1) && (self->health > self->wait)) || ((self->spawnflags & 2) && (self->health < self->wait)))
	{
		if (self->pathtarget)
		{
			char *savetarget;
			char *savemessage;

			savetarget = self->target;
			savemessage = self->message;
			self->target = self->pathtarget;
			self->message = NULL;
			G_UseTargets(self, self->activator);
			self->target = savetarget;
			self->message = savemessage;
		}

		if (!(self->spawnflags & 8))
			return;

		func_clock_reset (self);

		if (self->spawnflags & 4)
			return;
	}

	self->nextthink = level.time + 1.0;
}

void func_clock_use(edict_t *self, edict_t *other, edict_t *activator)
{
	if (!(self->spawnflags & 8))
		self->use = NULL;

	if (self->activator)
		return;

	self->activator = activator;
	self->think(self);
}

void SP_func_clock(edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("%s with no target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	if ((self->spawnflags & 2) && (!self->count))
	{
		gi.dprintf("%s with no count at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	if ((self->spawnflags & 1) && (!self->count))
		self->count = 60 * 60;

	func_clock_reset(self);

	self->message = gi.TagMalloc(CLOCK_MESSAGE_SIZE, TAG_LEVEL);
	self->think = func_clock_think;

	if (self->spawnflags & 4)
		self->use = func_clock_use;
	else
		self->nextthink = level.time + 1.0;
}

//=================================================================================

void teleporter_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*dest;
	int		i;

//CW++
	edict_t *ent;
//CW--

	if (!other->client)
		return;

	dest = G_Find(NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf("Couldn't find destination for misc_teleporter at %s\n", vtos(self->s.origin));	//CW
		return;
	}

//Pon++
//	Route update.

	if ((int)chedit->value && (CurrentIndex < MAXNODES) && (other == &g_edicts[1]))
	{
		gi.bprintf(PRINT_HIGH, "Teleport!\n");
		VectorCopy(self->s.origin, Route[CurrentIndex].Pt);
		Route[CurrentIndex].ent = NULL;
		Route[CurrentIndex].state = GRS_TELEPORT;

		if (++CurrentIndex < MAXNODES)
		{
			memset(&Route[CurrentIndex], 0, sizeof(route_t));
			Route[CurrentIndex].index = Route[CurrentIndex - 1].index + 1;
		}
	}
//Pon--

//Maj++
	ForceRouteReset(other);
//Maj--

//ZOID++
	CTFPlayerResetGrapple(other);
//ZOID--

//CW++
//	Break Trap tractor beam(s) if the player is held (and flagged to do so).

	if (other->tractored && !((int)sv_trap_thru_tele->value))
	{
		other->tractored = false;
		for (i = 0; i < globals.num_edicts; ++i)
		{
			ent = &g_edicts[i];
			if (!ent->inuse)
				continue;
			if (ent->client)
				continue;
			if (ent->wep_proj)
				continue;

			if ((ent->die == Trap_DieFromDamage) && (ent->enemy == other))
			{
				ent->think = Trap_Die;
				ent->nextthink = level.time + FRAMETIME;
			}
		}
	}

//	Break AGM stream if the player is being manipulated.

	if (other->client->agm_enemy != NULL)
	{
		other->client->thrown_by_agm = false;
		other->client->flung_by_agm = false;
		other->client->held_by_agm = false;
		
		other->client->agm_enemy->client->agm_on = false;
		other->client->agm_enemy->client->agm_push = false;
		other->client->agm_enemy->client->agm_pull = false;
		other->client->agm_enemy->client->agm_charge = 0;
		other->client->agm_enemy->client->agm_target = NULL;
		other->client->agm_enemy = NULL;
	}

//	Snuff out the flame if the player is on fire (and flagged to do so).

	if (other->burning && !((int)sv_flame_thru_tele->value))
	{
		other->burning = false;
		if (other->flame)
		{
			other->flame->touch = NULL;
			Flame_Expire(other->flame);
		}
	}
//CW--

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity(other);

	VectorCopy(dest->s.origin, other->s.origin);
	VectorCopy(dest->s.origin, other->s.old_origin);
	other->s.origin[2] += 10.0;

	// clear the velocity and hold them in place briefly
	VectorClear(other->velocity);
	other->client->ps.pmove.pm_time = 160>>3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->owner->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i = 0; i < 3; i++)
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

	VectorClear(other->s.angles);
	VectorClear(other->client->ps.viewangles);
	VectorClear(other->client->v_angle);

	// kill anything at the destination
	KillBox(other);
	gi.linkentity(other);
}

/*QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.
*/
void SP_misc_teleporter(edict_t *ent)
{
	edict_t *trig;

	if (!ent->target)
	{
		gi.dprintf("misc_teleporter without a target at %s\n", vtos(ent->s.origin));				//CW
		G_FreeEdict(ent);
		return;
	}

//CW++
	if (!(ent->spawnflags & SPAWNFLAG_NOMODEL))
	{
//CW--
		gi.setmodel(ent, "models/objects/dmspot/tris.md2");
		ent->s.skinnum = 1;
		ent->solid = SOLID_BBOX;
		VectorSet(ent->mins, -32.0, -32.0, -24.0);
		VectorSet(ent->maxs, 32.0, 32.0, -16.0);
	}

//CW++
	if (!(ent->spawnflags & SPAWNFLAG_NOFOUNTAIN))
	{
//CW--
		ent->s.effects = EF_TELEPORTER;
		ent->s.sound = gi.soundindex("world/amb10.wav");
	}
	gi.linkentity(ent);

//	Create a trigger field for the teleporter.

	trig = G_Spawn();
	trig->touch = teleporter_touch;
	trig->solid = SOLID_TRIGGER;
	trig->target = ent->target;
	trig->owner = ent;
	VectorCopy(ent->s.origin, trig->s.origin);
	VectorSet(trig->mins, -8.0, -8.0, 8.0);
	VectorSet(trig->maxs, 8.0, 8.0, 24.0);

//CW++
	trig->classname = "tele_trigger";
//CW--

	gi.linkentity(trig);	
}

/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
*/
void SP_misc_teleporter_dest(edict_t *ent)
{
	if (!(ent->spawnflags & SPAWNFLAG_NOMODEL))
	{
		gi.setmodel(ent, "models/objects/dmspot/tris.md2");
		ent->s.skinnum = 0;
		ent->solid = SOLID_BBOX;
		VectorSet(ent->mins, -32.0, -32.0, -24.0);
		VectorSet(ent->maxs, 32.0, 32.0, -16.0);

		gi.linkentity(ent);
	}

//CW++
	ent->style = ENT_ID_TELE_DEST;
//CW--
}
