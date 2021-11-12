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

	// Don't collide with other plasma balls
	if ( Q_stricmp(other->classname, "plasmaball") == 0 )
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
Spawn_Plasmaball

Spawns the plasma entities, and defines values global to both weapon modes.
==============================================================================
*/
edict_t *Spawn_Plasmaball (edict_t *ent, vec3_t start)
{
	edict_t *plasmaball;	// = G_Spawn();

	// sanity check
	if (!ent) {
		return NULL;
	}

	plasmaball = G_Spawn();

	plasmaball->owner = ent;
	plasmaball->clipmask = MASK_SHOT;
	plasmaball->solid = SOLID_BBOX;
	plasmaball->svflags = SVF_DEADMONSTER;
	
	VectorCopy (start, plasmaball->s.origin);
	plasmaball->classname = "plasmaball";
	plasmaball->class_id = ENTITY_PLASMABALL;

	plasmaball->s.effects |= EF_BLUEHYPERBLASTER | EF_ANIM_ALLFAST;
	// bat to get rid of the blue flag effect
//	plasmaball->s.effects |= EF_IONRIPPER | EF_ANIM_ALLFAST;
	plasmaball->s.renderfx = RF_TRANSLUCENT;
	plasmaball->s.modelindex = gi.modelindex(PLASMA_SPRITE_FLY);
	plasmaball->s.sound = gi.soundindex(PLASMA_SOUND_FLYBY);

	// give it some thickness for the bounce
//	VectorSet (plasmaball->mins, -12, -12, -12);
//	VectorSet (plasmaball->maxs, 12, 12, 12);
	VectorSet (plasmaball->mins, -6, -6, -6);
	VectorSet (plasmaball->maxs, 6, 6, 6);

	return plasmaball;
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
	edict_t *plasmaball = NULL;

	// sanity check
	if (!ent) {
		return;
	}

	plasmaball = Spawn_Plasmaball (ent, start);
	if (!plasmaball) {
		return;
	}

	plasmaball->movetype = MOVETYPE_WALLBOUNCE;	// Knightmare- use same movetype as ION Ripper projectiles

	VectorScale (dir, speed, plasmaball->velocity);		// Knightmare- use parm speed
	VectorCopy (plasmaball->velocity, plasmaball->s.angles);		// needed for post touch
	
	//-bat
	plasmaball->dmg = damage;		// Knightmare- use parm damage
	plasmaball->touch = plasma_rifle_bounce_touch;
	
	plasmaball->think = G_FreeEdict;				// change this to handle
//	plasmaball->nextthink = level.time + 3.0;		// sprite animation?
//	plasmaball->nextthink = level.time + 1.5;
	plasmaball->nextthink = level.time + sk_plasma_rifle_life_bounce->value;

	gi.linkentity (plasmaball);

	// Knightmare- added missing check_dodge() call
	if (ent->client)
		check_dodge (ent, plasmaball->s.origin, dir, speed);
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
	edict_t	*plasmaball_l = NULL;
	edict_t *plasmaball_c = NULL;
	edict_t	*plasmaball_r = NULL;
	vec3_t	dir_r, dir_l, angles;

	// sanity check
	if (!ent) {
		return;
	}

	plasmaball_l = Spawn_Plasmaball (ent, start);
	plasmaball_c = Spawn_Plasmaball (ent, start);
	plasmaball_r = Spawn_Plasmaball (ent, start);
	if (!plasmaball_l || !plasmaball_c || !plasmaball_r) {
		return;
	}

	plasmaball_l->movetype = MOVETYPE_FLYMISSILE;
	plasmaball_c->movetype = MOVETYPE_FLYMISSILE;
	plasmaball_r->movetype = MOVETYPE_FLYMISSILE;

	VectorClear (plasmaball_l->mins);
	VectorClear (plasmaball_l->maxs);
	VectorClear (plasmaball_c->mins);
	VectorClear (plasmaball_c->maxs);
	VectorClear (plasmaball_r->mins);
	VectorClear (plasmaball_r->maxs);

	// Knightmare- use parm damage
	plasmaball_l->dmg = damage;
	plasmaball_c->dmg = damage;
	plasmaball_r->dmg = damage;

	// center spread, line of sight
	VectorScale (dir, speed, plasmaball_c->velocity);		// Knightmare- use parm speed
	vectoangles (dir, angles);

	// right spread, has 10+ in yaw
	angles[YAW] -= 10;
	AngleVectors (angles, dir_r, NULL, NULL);
	VectorScale (dir_r, speed, plasmaball_r->velocity);		// Knightmare- use parm speed

	// left spread, has 10- in yaw
	angles[YAW] += 20;
	AngleVectors (angles, dir_l, NULL, NULL);
	VectorScale (dir_l, speed, plasmaball_l->velocity);		// Knightmare- use parm speed

	plasmaball_l->touch = plasma_rifle_spread_touch;
	plasmaball_c->touch = plasma_rifle_spread_touch;
	plasmaball_r->touch = plasma_rifle_spread_touch;
	
	plasmaball_l->think = G_FreeEdict;
	plasmaball_c->think = G_FreeEdict;
	plasmaball_r->think = G_FreeEdict;
//	plasmaball_l->nextthink = level.time + 3.0;
//	plasmaball_c->nextthink = level.time + 3.0;
//	plasmaball_r->nextthink = level.time + 3.0;
	plasmaball_l->nextthink = level.time + sk_plasma_rifle_life_spread->value;
	plasmaball_c->nextthink = level.time + sk_plasma_rifle_life_spread->value;
	plasmaball_r->nextthink = level.time + sk_plasma_rifle_life_spread->value;

	gi.linkentity (plasmaball_l);
	gi.linkentity (plasmaball_c);
	gi.linkentity (plasmaball_r);

	// Knightmare- added missing check_dodge() calls
	if (ent->client) {
		check_dodge (ent, plasmaball_c->s.origin, dir, speed);
		check_dodge (ent, plasmaball_r->s.origin, dir_r, speed);
		check_dodge (ent, plasmaball_l->s.origin, dir_l, speed);
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

// NOTE: SP_plasmaball should ONLY be used to spawn plasma rifle shots that change maps
//       via a trigger_transition. It should NOT be used for map entities.

void plasmaball_delayed_start (edict_t *plasmaball)
{
	if (g_edicts[1].linkcount)
	{
		VectorScale(plasmaball->movedir, plasmaball->moveinfo.speed, plasmaball->velocity);
		plasmaball->nextthink = level.time + 8000/plasmaball->moveinfo.speed;
		plasmaball->think = G_FreeEdict;
		gi.linkentity(plasmaball);
	}
	else
		plasmaball->nextthink = level.time + FRAMETIME;
}

void SP_plasmaball (edict_t *plasmaball)
{
	vec3_t	dir;

	plasmaball->s.modelindex = gi.modelindex(PLASMA_SPRITE_FLY);
	plasmaball->s.effects |= EF_BLUEHYPERBLASTER | EF_ANIM_ALLFAST;
	plasmaball->s.sound = gi.soundindex(PLASMA_SOUND_FLYBY);

	// set to the spread variant as we don't need a high speed bouncing projectile just after a map transition
	plasmaball->touch = plasma_rifle_spread_touch;
	AngleVectors(plasmaball->s.angles, dir, NULL, NULL);
	VectorCopy (dir, plasmaball->movedir);
	plasmaball->moveinfo.speed = VectorLength(plasmaball->velocity);
	if (plasmaball->moveinfo.speed <= 0)
		plasmaball->moveinfo.speed = sk_plasma_rifle_speed_spread->value;	//	1200

	// For SP, freeze plasmaball until player spawns in
	if (game.maxclients == 1)
	{
		VectorClear(plasmaball->velocity);
		plasmaball->think = plasmaball_delayed_start;
		plasmaball->nextthink = level.time + FRAMETIME;
	}
	else
	{
		plasmaball->think = G_FreeEdict;
		plasmaball->nextthink = level.time + 8000/plasmaball->moveinfo.speed;
	}
	gi.linkentity (plasmaball);
}
// end M82 Plasma Rifle
