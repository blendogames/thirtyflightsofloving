#include "g_local.h"

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
