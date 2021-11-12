#include "g_local.h"

// RAFAEL

/*
==============================================================================

ION RIPPER

==============================================================================
*/

void ionripper_sparks (edict_t *self)
{
	byte	count, color;

	count = 0;
	color = 0xe4 + (rand()&3);

	// Knightmare- explode sound
#ifdef KMQUAKE2_ENGINE_MOD
	if (sk_ionripper_extra_sounds->value)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/ionexp.wav"), 1, ATTN_NONE, 0);
#endif

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
	// Knightmare- hit sound
#ifdef KMQUAKE2_ENGINE_MOD
		if (sk_ionripper_extra_sounds->value)
		{
			float r = random();
			if (r < 0.3333)
				gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/ionhit1.wav"), 1, ATTN_NONE, 0);
			else if (r < 0.6666)
				gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/ionhit2.wav"), 1, ATTN_NONE, 0);
			else
				gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex ("weapons/ionhit3.wav"), 1, ATTN_NONE, 0);
		}
#endif
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_RIPPER);
	}
	else
	{
		return;
	}

	G_FreeEdict (self);
}


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


/*
==============================================================================

HEAT-SEEKING ROCKETS

==============================================================================
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
		// If player fires this, do track monsters
		if (!self->owner->client && !target->client)
			continue;
		if (target->health <= 0)
			continue;
		if (!visible (self, target))
			continue;
		
		// if we need to reduce the tracking cone
	/*	{
			vec3_t	vec;
			float	dot;
			vec3_t	forward;
	
			AngleVectors (self->s.angles, forward, NULL, NULL);
			VectorSubtract (target->s.origin, self->s.origin, vec);
			VectorNormalize (vec);
			dot = DotProduct (vec, forward);
	
			if (dot > 0.6)
				continue;
		} */

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


// NOTE: the new Rogue fire_heat is in g_newweap.c
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


/*
==============================================================================

PHALANX PARTICLE CANNON

==============================================================================
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

/*
==============================================================================

TRAP

==============================================================================
*/

extern void SP_item_foodcube (edict_t *best);

extern int	gibsthisframe;
extern int lastgibframe;

void Trap_Think (edict_t *ent)
{
	edict_t	*target = NULL;
	edict_t	*best = NULL;
	vec3_t	vec;
	int		len, i;
	int		oldlen = 8000;
//	vec3_t	forward, right, up;
	
	if (ent->timestamp < level.time)
	{
		ent->s.frame = 6;
	//	BecomeExplosion1(ent);
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
				
				// Knightmare- forget this, enough gibs are spawned already.
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

				//	if (strcmp (ent->enemy->classname, "monster_gekk") == 0)
					if (ent->enemy->blood_type == 1)
					{
						best->s.modelindex = gi.modelindex ("models/objects/gekkgib/torso/tris.md2");	
					//	best->s.effects |= TE_GREENBLOOD;
						best->s.effects |= EF_GREENGIB|EF_BLASTER;
					}
					// Knightmare- added 
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

				//	best->nextthink = level.time + 0.1;
					best->nextthink = level.time + 10 + random()*10;
				//	best->think = G_FreeEdict;
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
				
		//	if (ent->wait < 19)
		//		ent->s.frame++;

			return;
		}
		ent->s.frame ++;
		if (ent->s.frame == 8)
		{
			ent->nextthink = level.time + 1.0;
			ent->think = G_FreeEdict;

		/*	best = G_Spawn ();
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

//	Grenade_Explode (ent);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectExplosion (TE_GRENADE_EXPLOSION, origin);

	G_FreeEdict (ent);
}
