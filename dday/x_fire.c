/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/x_fire.c,v $
 *   $Revision: 1.13 $
 *   $Date: 2002/06/04 19:49:50 $
 *
 ***********************************

Copyright (C) 2002 Vipersoft

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
 
 /*==========================================================================
//  x_fire.c -- by Patrick Martin               Last updated:  8-31-1998
//--------------------------------------------------------------------------
//  This file contains code that produces fire that burns entities.
//
//  Fire inflicts damage to entities near it.  In addition, there is a
//  chance that an affected entity will catch on fire.  If this happens,
//  the fire will burn the target for some time and inflict more damage.
//  Fire will be extinguished if the burning entity enters at least
//  waist-deep into liquid.  Fire can also spread from entity to entity.
//  However, all frags (and anger) are given only to the entity that
//  originally spawned the fire.
//
//  NOTES:  The following edicts fields defined in 'g_local.h'
//  are used differently here than in other files:
//
//  * count = Chance that a flame will be created and dropped
//      from an exploding fireball.
//  * dmg = Chance that a dropped flame will explode on impact.
//  * pos1 = Fire damage vector for direct hits.  {die, base, chance}.
//      damage is 1d(die) + base.  Target has chance% of igniting.
//  * pos2 = Same as pos1 except this is used for splash damage.
//  * timestamp = Time when burning flames will damage nearby entities.
//
//  This code uses my custom flame model.  It is similar to
//  (but NOT the same as) the flame2.mdl in Quake 1.
//
//  Frames:
//     0 -  2  Small flame expansion (used for fireball creation)
//     3 - 14  Small fire (used for fireballs and the like)
//    15 - 20  Large flame expansion (used for burning entities)
//    21 - 32  Large fire (used for burning entities)
//    33 - 38  Huge flare (NOT used)
//         39  Tiny spark (near invisible; used for smoke)
//         40  Base frame (NOT used)
//
//  Skins:
//     0  Orange fire
//     1  Blue fire (NOT used)
//
//========================================================================*/

#include "g_local.h"
#include "x_fire.h"


/*============================/  Fire Dodge  /============================*/

/*-------------------------------------------------------- Mirror Code -----
//  This is a copy of 'check_dodge' found in 'g_weapons.c'.
//  Because that function is static, and I do not want to
//  modify that file, a copy is placed here.
//------------------------------------------------------------------------*/
void check_firedodge (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	vec3_t	end;
	vec3_t	v;
	trace_t	tr;
	float	eta;

	/* easy mode only ducks one quarter the time */
	if (skill->value == 0)
	{
		if (random() > 0.25)
			return;
	}
	VectorMA (start, 8192, dir, end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterinfo.dodge) && infront(tr.ent, self))
	{
		VectorSubtract (tr.endpos, start, v);
		eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
		tr.ent->monsterinfo.dodge (tr.ent, self, eta);
	}
}


/*========================/  Custom Explosions  /========================*/

/*------------------------------------------------------/ New Code /--------
//  This transforms the fire into smoke.
//------------------------------------------------------------------------*/
void PBM_BecomeSmoke (edict_t *self)
{
/* Make some steam. */
	self->s.modelindex = MD2_FIRE;
	self->s.frame      = FRAME_FLAMEOUT;
        self->s.skinnum    = 0;
	self->touch        = NULL;
        self->solid        = SOLID_NOT;
        self->takedamage   = DAMAGE_NO;
        self->clipmask     = 0;
	self->s.effects    = EF_GRENADE;
	self->movetype     = MOVETYPE_FLY;
	VectorSet(self->velocity, 0, 0, 400);

        gi.linkentity (self);

	self->nextthink    = level.time + 0.1;
	self->think        = G_FreeEdict;
}

/*------------------------------------------------------/ New Code /--------
//  This transforms the fire into steam.
//------------------------------------------------------------------------*/
void PBM_BecomeSteam (edict_t *self)
{
/* world/steam*.wav are standard Q2 sounds. */
	if (rand() & 1)
                gi.sound(self, CHAN_AUTO, gi.soundindex("world/steam1.wav"), 1, ATTN_NORM, 0);
	else
                gi.sound(self, CHAN_AUTO, gi.soundindex("world/steam2.wav"), 1, ATTN_NORM, 0);

	PBM_BecomeSmoke(self);
}

/*------------------------------------------------------/ New Code /--------
//  This animates the small explosion.  The magic numbers in this
//  function represent the frame numbers in id's r_explode md2.
//------------------------------------------------------------------------*/
void PBM_SmallExplodeThink (edict_t *self)
{
        if (++self->s.frame > 7)
	{	G_FreeEdict (self);
		return;
	}

	self->s.skinnum++;

        if (self->s.frame == 7)
                self->s.renderfx |= RF_TRANSLUCENT;
	else if (self->s.frame == 3)
		self->s.effects &= ~EF_HYPERBLASTER;

	self->nextthink = level.time + FRAMETIME;
}

/*------------------------------------------------------/ New Code /--------
//  This transforms an entity into a small explosion.
//------------------------------------------------------------------------*/
void PBM_StartSmallExplosion (edict_t *self)
{
        self->s.modelindex  = gi.modelindex("models/objects/r_explode/tris.md2");
        self->s.frame       = 1;
        self->s.skinnum     = 0;
        self->touch         = NULL;
        self->solid         = SOLID_NOT;
        self->takedamage    = DAMAGE_NO;
        self->clipmask      = 0;
        self->s.effects     = EF_HYPERBLASTER;
        self->s.renderfx    = RF_FULLBRIGHT;
        self->movetype      = MOVETYPE_NONE;
        VectorClear (self->velocity);
        VectorClear (self->s.angles);
        self->s.angles[YAW] = rand() % 360;
        self->nextthink     = level.time + FRAMETIME;
        self->think         = PBM_SmallExplodeThink;

        gi.linkentity (self);
}

/*------------------------------------------------------/ New Code /--------
//  This tells an entity to become a small explosion.  If entity
//  is underwater, steam is produced instead.
//------------------------------------------------------------------------*/
void PBM_BecomeSmallExplosion (edict_t *self)
{
        if (gi.pointcontents(self->s.origin) & MASK_WATER)
        {       PBM_BecomeSteam (self);
                return;
        }

/* Play explosion sound at half volume. */
        gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/grenlx1a.wav"), 0.5, ATTN_NORM, 0);

        PBM_StartSmallExplosion (self);
}


/*========================/  General Fire Code  /========================*/

/*-------------------------------------------------------- New Code --------
//  This removes every fire from all burning entities.  This
//  should always be called whenever a client changes levels.
//
//  NOTE:  This causes the flames to simply disappear.
//------------------------------------------------------------------------*/
void PBM_KillAllFires (void)
{
        edict_t *ent;

        for (ent = g_edicts; ent < &g_edicts[globals.num_edicts]; ent++)
        {
                if (!ent->inuse)
			continue;

                /* Kill any fires. */
                ent->burnout = 0;
                if (ent->burner)
                {       ent->burner->enemy = NULL;
                        G_FreeEdict (ent->burner);
                        ent->burner = NULL;
                }
        }
}

/*------------------------------------------------------/ New Code /--------
//  This checks if 'ent' (the entity on fire) found some water.
//
//  NOTE:  Checking the entity rather than the flame itself
//         works to the entity's benefit, because the flame
//         is (usually) set higher than ent->s.origin.
//------------------------------------------------------------------------*/
qboolean PBM_InWater (edict_t *ent)
{
/* For players and monsters. */
	if (ent->waterlevel >= 2)
		return true;

/* All-purpose check. */
	if (gi.pointcontents(ent->s.origin) & MASK_WATER)
		return true;

        return false;
}

/*------------------------------------------------------/ New Code /--------
//  This checks if 'ent' is impervious to fire.  Entities that are
//  immune to fire return true.  Entities that can burn return false.
//------------------------------------------------------------------------*/
qboolean PBM_Inflammable (edict_t *ent)
{
/* Thou canst not burn what doth not exist. */
        if (!ent)  return true;

/* Any entity that is impervious to damage is inflammable. */
        if (!ent->takedamage)  return true;

/* Dead and gibbed entities cannot burn. */
        if (ent->health <= ent->gib_health)  return true;

/* Any entity that is immune to lava cannot burn. */
        if (ent->flags & FL_IMMUNE_LAVA)  return true;

/* If entity is a client, check for powerups. */
	if (ent->client)
	{
	/* Invulnerability protects entity from burning. */
		if (ent->client->invincible_framenum > level.framenum)
                        return true;

	/* Bio-suit offers partial immunity to fire. */
                if ((ent->client->enviro_framenum > level.framenum) && (random() < 0.8))
                        return true;

	}

/* The entity is flammable. */
        return false;
}


/*------------------------------------------------------/ New Code /--------
//  This checks if 'ent' can be set on fire.  If so, false is returned.
//------------------------------------------------------------------------*/
qboolean PBM_FireResistant (edict_t *ent, vec3_t point)
{
/* An entity immune to damage (or fire) obviously can resist fire. */
        if (PBM_Inflammable(ent))
		return true;

/* Check if entity is resistant to fire.  (True by default.) */
        if (!(ent->fireflags & FIREFLAG_IGNITE))
		return true;


/* Entity can be set on fire. */
	return false;
}

/*------------------------------------------------------/ New Code /--------
//  This damages a single entity and possibly sets it on fire.
//
//  NOTE(1):  Damage = 1d(die) + base.  (This is D&D notation)
//            Chance is expressed as a percentange.  (e.g., 20 = 20%.)
//
//  NOTE(2):  All fire damage uses the field '(edict)->master'.
//            That field cannot be NULL, or Q2 may crash.
//------------------------------------------------------------------------*/
void PBM_BurnDamage
(edict_t *victim, edict_t *fire, vec3_t point, vec3_t damage, int dflags, int mod)
{
	int   points;
	int   die = (int)damage[0];
	int   base = (int)damage[1];
	int   chance;
	
	chance = (int)damage[2];

    if (PBM_Inflammable(victim))
		return;

/* Calculate damage. */
	if (die > 0)
		points = (rand() % die) + base + 1;
	else
		points = base;

/* Check if entity will catch on fire. */
//	if ((rand() % 100) < chance)
			if (fire->master != fire);
                PBM_Ignite (victim, fire->master, point);

/* Inflict some burn damage. */

        T_Damage (victim, fire, fire->master, vec3_origin, point, vec3_origin, points, 0, dflags, mod);
}

/*------------------------------------------------------/ New Code /--------
//  This damages entities within a certain radius.  There is
//  also a chance that affected entities will catch on fire.
//
//  NOTE:  Damage = 1d(die) + base.  Chance is expressed as percentange.
//------------------------------------------------------------------------*/
void PBM_BurnRadius
(edict_t *fire, float radius, vec3_t damage, edict_t *ignore)
{
        edict_t *ent = NULL;
        vec3_t  point;

	while ((ent = findradius(ent, fire->s.origin, radius)) != NULL)
	{
                if (!CanDamage (ent, fire))
                        continue;

                if (PBM_Inflammable(ent))
                        continue;

                if (ent == ignore)
                        continue;

                /* Locate damage point. */
                VectorSubtract (ent->s.origin, fire->s.origin, point);
                VectorNormalize (point);
                VectorMA (ent->s.origin, -4, point, point);

                /* Burn the target. */
                PBM_BurnDamage (ent, fire, point, damage, DAMAGE_RADIUS, MOD_FIRE_SPLASH);
	}
}

/*------------------------------------------------------/ New Code /--------
//  This checks if the fire's master (owner) is in Quake2.  If not,
//  this changes the fire's master to itself.  This check is needed
//  so that Quake2 does not crash when a fire tries to burn something
//  after its master is removed from Quake2.
//------------------------------------------------------------------------*/
void PBM_CheckMaster (edict_t *fire)
{
        if (!fire->master)
        {       fire->master = fire;
                fire->owner = fire;
        }
}

/*------------------------------------------------------/ New Code /--------
//  This returns the best location on the entity for where the fire
//  should burn.
//------------------------------------------------------------------------*/
void PBM_FireSpot (vec3_t spot, edict_t *ent)
{
        vec3_t  forward;
        int     delta = 0;

/* Get the entity's forward direction. */
        AngleVectors (ent->s.angles, forward, NULL, NULL);
        VectorNormalize (forward);
        VectorScale (forward, 4, forward);

/* Put the flame at the best spot on an entity. */
        VectorCopy (ent->s.origin, spot);
        VectorAdd (spot, forward, spot);

/* Adjust elevation of flame depending on the entity. */
        if ((ent->health > 0) || (!(ent->fireflags & FIREFLAG_DELTA_ALIVE)))
        {
                delta = ent->fireflags & FIREFLAG_SHIFT_Z;
                if (ent->fireflags & FIREFLAG_DOWN)
                        delta = 0 - delta;
        }

        if (ent->fireflags & FIREFLAG_DELTA_BASE)
        {       /* Dead entities are lower than living ones. */
                if (ent->health > 0)
                        delta += 4;
                else
                        delta -= 18;
        }

        if (ent->fireflags & FIREFLAG_DELTA_VIEW)
                delta += ent->viewheight;

        spot[2] += delta;
}

/*------------------------------------------------------/ New Code /--------
//  This checks if the fire burning on an entity should die.
//------------------------------------------------------------------------*/
qboolean PBM_FlameOut (edict_t *self)
{
/* If entity suddenly disappears from Quake2, remove the fire. */
	if (!self->enemy)
        {       PBM_BecomeSmallExplosion (self);
		return true;
	}

/* If no burnout time is specified, entity should not be burning. */
	if (!self->enemy->burnout)
        {       PBM_BecomeSmallExplosion (self);
		return true;
	}

/* Explode as soon as the entity is gibbed. */
	if (self->enemy->health <= self->enemy->gib_health)
	{	self->enemy->burnout = 0;
                PBM_BecomeSmallExplosion (self);
		return true;
	}

/* The fire burns itself out after a period of time. */
	if (self->enemy->burnout < level.time)
	{	self->enemy->burnout = 0;
                PBM_BecomeSmallExplosion (self);
		return true;
	}

/* Check if the victim found some water. */
	if (PBM_InWater(self->enemy))
	{	self->enemy->burnout = 0;
		PBM_BecomeSteam (self);
		return true;
	}

/* Gaining invulnerability kills the fire immediately. */
	if (self->enemy->client)
        {       if (self->enemy->client->invincible_framenum > level.framenum)
		{	self->enemy->burnout = 0;
                        PBM_BecomeSmallExplosion (self);
			return true;
		}
        }

/* Keep on burning! */
        return false;
}

/*------------------------------------------------------/ New Code /--------
//  This keeps the fire on an entity burning.
//
//  NOTE:  Because of client prediction, the client's POV (point of view)
//         is ahead of its body.  So, if the flames burn on the client's
//         body where they should, the flames will lag behind the POV.
//         Giving the fire the client's velocity will often place the
//         fire ahead of the client's body, where it will likely be
//         closer to the POV.  With all this in mind, we give flames
//         velocity only in single-player games, and if the victim is
//         a client with prediction on.
//------------------------------------------------------------------------*/
void PBM_Burn (edict_t *self)
{
	vec3_t   spot;

/* Check if fire should be extinguished. */
        if (PBM_FlameOut (self))
		return;

/* Burn 'em all!  Make fire stick to target. */
        PBM_FireSpot (spot, self->enemy);
        VectorCopy (spot, self->s.origin);
        if (self->enemy->client)
        { //      if ( deathmatch->value || coop->value || (self->enemy->client->ps.pmove.pm_flags & PMF_NO_PREDICTION) )
                        VectorClear (self->velocity);
  //              else
     //                   VectorCopy (self->enemy->velocity, self->velocity);
						if (self->enemy->stanceflags == STANCE_CRAWL)
							self->s.origin[2]+=20;
        }
        else
                VectorClear (self->velocity);

        gi.linkentity (self);

/* Animate the fire. */
	if (++self->s.frame > FRAME_LAST_LARGEFIRE)
		self->s.frame = FRAME_FIRST_LARGEFIRE;

/* Spread the fire! */
	if (self->timestamp < level.time)
        {       PBM_CheckMaster (self);
                PBM_BurnRadius (self, 70, self->pos2, self->enemy);
                PBM_BurnDamage (self->enemy, self, self->enemy->s.origin, self->pos1, 0, MOD_ON_FIRE);
		self->timestamp = level.time + 1;
	}

	self->nextthink  = level.time + FRAMETIME;
}

void Drop_Flamed (edict_t *ent);
/*------------------------------------------------------/ New Code /--------
//  This sets the victim on fire.
//------------------------------------------------------------------------*/
void PBM_Ignite (edict_t *victim, edict_t *attacker, vec3_t point)
{
        edict_t *fire;
        vec3_t  spot;
        vec3_t  burn_damage = {10, 5, 0};
	vec3_t	radius_damage = {0, 5, 50};


	if (victim->client)
	{
		if (level.time < victim->client->spawntime + invuln_spawn->value)
			return;
	}
/* Some entities vulnerable to fire damage can resist burning. */
        if (PBM_FireResistant(victim, point))  return;

		if (OnSameTeam(attacker, victim))
			return;

/* If entity is already burning, extend time and switch owners. */
        if (victim->burnout > level.time)
        {       victim->burnout         = level.time + BURN_TIME;
                victim->burner->owner   = attacker;
                victim->burner->master  = attacker;
                return;
        }
		
		//not using this
       // if(victim->client)
		//Drop_Flamed(victim);

/* Entity will burn for a period of time. */
	victim->burnout = level.time + BURN_TIME;
	
	
	//stop cheaters
	if (victim->client)
	{
		stuffcmd (victim, "set gl_polyblend 1");

		gi.sound(victim, CHAN_AUTO, gi.soundindex("jpn/molotov/flareup.wav"), 1, ATTN_NORM, 0);
	}





/* Create the fire. */
	PBM_FireSpot(spot, victim);

	fire = G_Spawn();

	fire->s.renderfx   = RF_FULLBRIGHT;


	fire->s.modelindex = MD2_FIRE;
        fire->s.frame      = FRAME_FIRST_LARGEIGNITE;
        fire->s.skinnum    = SKIN_FIRE_RED;
	VectorClear (fire->mins);
	VectorClear (fire->maxs);
	VectorCopy (spot, fire->s.origin);
        VectorClear (fire->s.angles);
        VectorClear (fire->velocity);
	fire->solid        = SOLID_NOT;
	fire->takedamage   = DAMAGE_NO;
        fire->movetype     = MOVETYPE_FLY;
	fire->clipmask     = 0;
	fire->s.effects    = EF_ROCKET;
	fire->s.renderfx   = RF_FULLBRIGHT;
	fire->owner        = attacker;
        fire->master       = attacker;
	fire->enemy        = victim;
	fire->classname    = "fire";
        fire->timestamp    = level.time + 1;
	fire->nextthink    = level.time + FRAMETIME;
	fire->think        = PBM_Burn;
	VectorCopy (burn_damage, fire->pos1);
	VectorCopy (radius_damage, fire->pos2);


	//faf:
	fire->s.sound = gi.soundindex("inland/fire.wav");



        gi.linkentity (fire);

/* Link victim to fire. */
	victim->burner = fire;
}


/*===========================/  Small Flames  /===========================*/

/*------------------------------------------------------/ New Code /--------
//  This checks when to put out the fire.  The fire is put out when
//  the burning entity finds water or when the fire is done burning.
//  The fire can (but does not need to) damage/ignite nearby entities.
//  This is called on every frame.  Used for small flames only.
//------------------------------------------------------------------------*/
void PBM_CheckFire (edict_t *self)
{
/* Put out the fire if it hits water. */
	if (PBM_InWater(self))
        {       PBM_BecomeSteam(self);
		return;
	}

/* The fire burns itself out after a period of time. */
	if (self->burnout < level.time)
        {       PBM_BecomeSmoke(self);
		return;
	}

	/* Animate the fire. */	
		if (self->s.frame > 19)
		{
			self->s.frame ++;
			if (self->s.frame == 33)
				self->s.frame = 20;
			
			if (self->movetype == MOVETYPE_NONE && !self->groundentity && rand()%100 == 1)
				PBM_FireDrop (self->master, self->s.origin, self->pos1, self->pos2, self->dmg, vec3_origin);

			if (self->burnout - 2 < level.time)
			{
				self->s.frame -= 12; //become small flame
				self->s.sound = gi.soundindex("inland/fire.wav");

			}
		}
		else if (++self->s.frame > FRAME_LAST_SMALLFIRE)
		self->s.frame = FRAME_FIRST_SMALLFIRE;

/* The fire may inflict a small amount of burn damage. */
	if (self->timestamp)
		if (self->timestamp < level.time)
		{
			vec3_t	damage = {3, 0, 5};

                        PBM_CheckMaster (self);
                        PBM_BurnRadius (self, 50, damage, NULL);
			self->timestamp = level.time + random();
		}

/* Fire is still burning, check again. */
	self->nextthink  = level.time + FRAMETIME;
}

/*------------------------------------------------------/ New Code /--------
//  Fire that drops onto an entity may set it on fire (if possible)
//  and explode to damage and possibly ignite nearby targets.
//------------------------------------------------------------------------*/
void PBM_FireDropTouch
(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
/* Check if floor is the sky.  If so, disappear. */
	if (surf && (surf->flags & SURF_SKY))
        {       G_FreeEdict (self);
		return;
	}

/* If in water, dissolve into steam. */
	if (PBM_InWater(self))
        {       PBM_BecomeSteam(self);
		return;
	}

/* Check if it explodes. */
        if ( (other->solid != SOLID_BSP) || ((rand() % 100) < self->dmg) )
        {
                if (other->takedamage)
                {
                        if (!(other->flags & FL_IMMUNE_LAVA))
                                PBM_BurnDamage(other, self, self->s.origin, self->pos1, 0, MOD_FIREBALL);
                }
                PBM_BurnRadius (self, self->dmg_radius, self->pos2, other);

        /* Calculate position for the explosion entity. */
    /*            VectorNormalize (self->velocity);
                VectorMA (self->s.origin, -8, self->velocity, self->s.origin);

                PBM_BecomeSmallExplosion (self);
		return; */ //why?
	}

/* The fire remains burning. */
	 VectorClear (self->velocity);
	 self->solid = SOLID_NOT;
	 VectorSet (self->mins, -4, -4, 0);
	 VectorSet (self->maxs, 4, 4, 24);
         gi.linkentity (self);
}

/*------------------------------------------------------/ New Code /--------
//  This creates a flame that drops straight down.
//------------------------------------------------------------------------*/
void PBM_FireDrop
(edict_t *attacker, vec3_t spot, vec3_t damage, vec3_t radius_damage, int blast_chance, vec3_t vel)
{
	edict_t *fire;

	fire = G_Spawn();
	fire->s.modelindex = MD2_FIRE;
        fire->s.frame      = FRAME_FIRST_SMALLIGNITE;
        fire->s.skinnum    = SKIN_FIRE_RED;
	VectorClear (fire->mins);
	VectorClear (fire->maxs);
        VectorCopy (spot, fire->s.origin);
	VectorClear (fire->s.angles);

	//VectorClear (fire->velocity);
	VectorCopy (vel, fire->velocity);

	fire->movetype     = MOVETYPE_TOSS;
	fire->clipmask     = MASK_SHOT;
	fire->solid        = SOLID_BBOX;
	fire->takedamage   = DAMAGE_NO;
	fire->s.effects    = 0;
	fire->s.renderfx   = RF_FULLBRIGHT;
        fire->owner        = fire;
        fire->master       = attacker;
	fire->classname    = "fire";
	fire->touch        = PBM_FireDropTouch;
	fire->burnout      = level.time + 2 + random() * 3;
	fire->timestamp    = level.time;
	fire->nextthink    = level.time + FRAMETIME;
	fire->think        = PBM_CheckFire;
        VectorCopy (damage, fire->pos1);
        VectorCopy (radius_damage, fire->pos2);
        fire->dmg_radius   = RADIUS_FIRE_SPLASH;
        fire->dmg          = blast_chance;

	//faf:
	fire->s.sound = gi.soundindex("inland/fire.wav");


	gi.linkentity (fire);
}


void FireWood
(edict_t *attacker, vec3_t spot, vec3_t damage, vec3_t radius_damage, int blast_chance, qboolean stick)
{
	edict_t *fire;

	fire = G_Spawn();
	fire->s.modelindex = MD2_FIRE;
        fire->s.frame      = 21;//FRAME_FIRST_SMALLIGNITE;
        fire->s.skinnum    = SKIN_FIRE_RED;
	VectorClear (fire->mins);
	VectorClear (fire->maxs);
        VectorCopy (spot, fire->s.origin);
	VectorClear (fire->s.angles);
	VectorClear (fire->velocity);
	
	if (stick)
		fire->movetype = MOVETYPE_NONE;
	else
		fire->movetype = MOVETYPE_TOSS;

	fire->clipmask     = MASK_SHOT;
	fire->solid        = SOLID_BBOX;
	fire->takedamage   = DAMAGE_NO;
	fire->s.effects    = 0;
	fire->s.renderfx   = RF_FULLBRIGHT;
        fire->owner        = fire;
        fire->master       = attacker;
	fire->classname    = "fire";
	fire->touch        = PBM_FireDropTouch;
	fire->burnout      = level.time + 8 + random() * 2;
	fire->timestamp    = level.time;
	fire->nextthink    = level.time + FRAMETIME;
	fire->think        = PBM_CheckFire;
        VectorCopy (damage, fire->pos1);
        VectorCopy (radius_damage, fire->pos2);
        fire->dmg_radius   = RADIUS_FIRE_SPLASH;
        fire->dmg          = blast_chance;

	//faf:
	//fire->s.sound = gi.soundindex("inland/fire.wav");
	fire->s.sound = gi.soundindex("faf/fire1.wav");


	gi.linkentity (fire);
}

/*------------------------------------------------------/ New Code /--------
//  This is a support function for 'PBM_FireDrop()'.
//------------------------------------------------------------------------*/
void PBM_EasyFireDrop (edict_t *self)
{
        PBM_FireDrop (self->owner, self->s.origin, self->pos1, self->pos2, self->dmg,  vec3_origin);
}


/*===========================/  Flame Clouds  /===========================*/

/*------------------------------------------------------/ New Code /--------
//  This causes part of the flame cloud to explode and drop flames.
//------------------------------------------------------------------------*/
void PBM_CloudBurst (edict_t *self)
{
	if ((rand() % 100) < self->count)
                PBM_EasyFireDrop (self);

        PBM_BecomeSmallExplosion (self);
}

/*------------------------------------------------------/ New Code /--------
//  This causes part of the flame cloud to damage as it erupts.
//------------------------------------------------------------------------*/
void PBM_CloudBurstDamage (edict_t *self)
{
        PBM_BurnRadius (self, self->dmg_radius, self->pos2, NULL);
        PBM_CloudBurst (self);
}

/*------------------------------------------------------/ New Code /--------
//  This creates a flaming cloud that can rain fire.
//
//  NOTE:  'cloud' vector fields --
//         [0] = Number of mini-explosions.
//         [1] = X and Y dimensions of cloud.
//         [2] = Z dimension of cloud.
//
//         'timer' vector fields --
//         [0] = Delay before any explosion erupts.
//         [1] = Extra random delay after first delay per explosion.
//         [2] = Unused!
//------------------------------------------------------------------------*/
void PBM_FlameCloud
(edict_t *attacker, vec3_t start, vec3_t cloud, vec3_t timer, qboolean deadly, vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance)
{
	vec3_t  spot;
	edict_t  *smoke;
	int  quota = (int)cloud[0];
        int  check = quota * 3;      /* Use correction after this #. */
        int  limit = check + quota;  /* Max. number of attempts. */
	int  tally = 0;           /* Number of flames created. */
	int  trials;              /* Number of attempts. */

	for (trials = 0; (trials < limit) && (tally < quota); trials++)
	{
        /* Pick any spot within cloud space. */
		VectorCopy (start, spot);
		spot[0] += ((random() - 0.5) * cloud[1]);
		spot[1] += ((random() - 0.5) * cloud[1]);
		spot[2] += ((random() - 0.5) * cloud[2]);

        /* If spot is in a solid, then either the fire is killed or
           placed back into empty space.  Unless the loop is near the
           counter limit, the flame is killed.
        */
                if ((trials >= check) && (gi.pointcontents(spot) & MASK_SOLID))
                {
                        trace_t tr;
                        vec3_t dir;

                        tr = gi.trace (start, NULL, NULL, spot, NULL, MASK_SHOT);

                        VectorCopy (tr.endpos, spot);
                        VectorSubtract (start, spot, dir);
                        VectorNormalize (dir);
                        VectorMA (spot, 10, dir, spot);
                }

        /* Create a fireball-shaped cloud if in empty space. */
                if (!(gi.pointcontents(spot) & MASK_SOLID))
		{
			smoke = G_Spawn();
                        smoke->s.modelindex = gi.modelindex("");
			VectorClear (smoke->mins);
			VectorClear (smoke->maxs);
			VectorCopy (spot, smoke->s.origin);
			smoke->movetype   = MOVETYPE_NONE;
			smoke->solid      = SOLID_NOT;
			smoke->takedamage = DAMAGE_NO;
			smoke->clipmask   = 0;
			smoke->owner      = attacker;
                        smoke->master     = attacker;
			smoke->enemy      = NULL;
			smoke->classname  = "fire";
			smoke->touch      = NULL;
			smoke->nextthink  = level.time + timer[0] + random() * timer[1];
			if (deadly)
				smoke->think      = PBM_CloudBurstDamage;
			else
				smoke->think      = PBM_CloudBurst;
			VectorCopy (damage, smoke->pos1);
			VectorCopy (radius_damage, smoke->pos2);
                        smoke->dmg_radius = RADIUS_FIRE_SPLASH;

			smoke->count      = rain_chance;
			smoke->dmg        = blast_chance;

			gi.linkentity (smoke);

			tally++;

			if (PBM_InWater(smoke))
				PBM_BecomeSteam (smoke);
		}
	}
}


/*===========================/  Projectiles  /===========================*/

/*------------------------------------------------------/ New Code /--------
//  This adjusts a vector's angles with the spread given.
//
//  NOTE:  'spread' vector fields --
//         [0] = PITCH = Vertical spread.
//         [1] = YAW   = Horizontal spread.
//         [2] = ROLL  = No effect (should always be 0).
//------------------------------------------------------------------------*/
void PBM_FireAngleSpread (vec3_t spread, vec3_t dir)
{
        vec3_t  v;

        vectoangles (dir, v);

        v[PITCH] += (random() - 0.5) * spread[PITCH];
        v[YAW]   += (random() - 0.5) * spread[YAW];
        v[ROLL]  += (random() - 0.5) * spread[ROLL];

        AngleVectors (v, dir, NULL, NULL);
        VectorNormalize (dir);
}

/*------------------------------------------------------/ New Code /--------
//  The fireball impacts and delivers pain and flames.
//------------------------------------------------------------------------*/
void PBM_FireballTouch
(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
        {       G_FreeEdict (self);
		return;
	}

	if (PBM_InWater(self))
        {       PBM_BecomeSteam(self);
		return;
	}

/* Burn! */
        PBM_BurnRadius (self, self->dmg_radius, self->pos2, other);

/* Calculate position for the explosion entity. */
        VectorNormalize (self->velocity);
        VectorMA (self->s.origin, -8, self->velocity, self->s.origin);
        gi.linkentity (self);

	if (other->takedamage)
	{
		if (other->flags & FL_IMMUNE_LAVA)
		{	vec3_t	reduced = {0, 0, 0};
			reduced[1] = self->pos1[1];
                        PBM_BurnDamage (other, self, self->s.origin, reduced, 0, MOD_FIREBALL);
		}
		else
                        PBM_BurnDamage (other, self, self->s.origin, self->pos1, 0, MOD_FIREBALL);
	}
	else
        {       if ((rand() % 100) < self->count)
                        PBM_EasyFireDrop(self);
	}

        PBM_BecomeSmallExplosion (self);
}

/*------------------------------------------------------/ New Code /--------
//  Create and launch a fireball.
//------------------------------------------------------------------------*/
void PBM_FireFlamer
(edict_t *self, vec3_t start, vec3_t spread, vec3_t dir, int speed, vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance)
{
	edict_t  *fireball;

/* Adjust fireball direction with 'spread'. */
        PBM_FireAngleSpread (spread, dir);

/* Create the fireball. */
	fireball = G_Spawn();
        fireball->s.modelindex    = MD2_FIRE;
        fireball->s.frame         = FRAME_FIRST_SMALLIGNITE;
        fireball->s.skinnum       = SKIN_FIRE_RED;
	VectorClear (fireball->mins);
	VectorClear (fireball->maxs);
	VectorCopy (start, fireball->s.origin);
	vectoangles (dir, fireball->s.angles);
	VectorScale (dir, speed, fireball->velocity);
        fireball->s.angles[PITCH] -= 90;
        fireball->movetype        = MOVETYPE_FLYMISSILE;
        fireball->clipmask        = MASK_SHOT;
        fireball->solid           = SOLID_BBOX;
        fireball->takedamage      = DAMAGE_NO;
        fireball->s.effects       = EF_GRENADE;
        fireball->s.renderfx      = RF_FULLBRIGHT;
        fireball->owner           = self;
        fireball->master          = self;
        fireball->classname       = "fire";
        fireball->touch           = PBM_FireballTouch;
        fireball->burnout         = level.time + 15000/speed;
        fireball->timestamp       = 0;
        fireball->nextthink       = level.time + FRAMETIME;
        fireball->think           = PBM_CheckFire;
	VectorCopy(damage, fireball->pos1);
	VectorCopy(radius_damage, fireball->pos2);
        fireball->dmg_radius      = RADIUS_FIRE_SPLASH;
        fireball->count           = rain_chance;
        fireball->dmg             = blast_chance;
	gi.linkentity (fireball);

/* If fireball is spawned in liquid, it dissolves. */
        if (PBM_InWater (fireball))
        {       PBM_BecomeSteam (fireball);
		return;
	}

/* Check for monster dodge routine. */
	if (self->client)
		check_firedodge (self, fireball->s.origin, dir, speed);
}

/*------------------------------------------------------/ New Code /--------
//  The fat fireball impacts and burns stuff up.
//------------------------------------------------------------------------*/
void PBM_FlameThrowerTouch
(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean wood = false;

	if (surf && (surf->flags & SURF_SKY))
        {       G_FreeEdict (self);
		return;
	}

	if (PBM_InWater(self))
        {       PBM_BecomeSteam(self);
		return;
	}

	if (surf && Surface(surf->name, SURF_WOOD))
		wood = true;

	//Wheaty: Don't let it go through solid objects
	if (gi.pointcontents(self->s.origin) & MASK_SOLID)
	{
		G_FreeEdict (self);
		return;
	}

/* Burn! */
        PBM_BurnRadius (self, self->dmg_radius, self->pos2, other);

/* Calculate position for the explosion entity. */
        VectorNormalize (self->velocity);
        VectorMA (self->s.origin, -8, self->velocity, self->s.origin);
        gi.linkentity (self);

	if (other->takedamage)
	{
		if (other->flags & FL_IMMUNE_LAVA){	
			vec3_t	reduced = {0, 0, 0};
			reduced[1] = self->pos1[1];
            PBM_BurnDamage (other, self, self->s.origin, reduced, 0, MOD_FIREBALL);
		}
		else
			PBM_BurnDamage (other, self, self->s.origin, self->pos1, 0, MOD_FIREBALL);
	}
	else {
		if (wood && rand()%4==1){
			FireWood (self->owner, self->s.origin, self->pos1, self->pos2, self->dmg, !strcmp(other->classname,"worldspawn"));
		}	
		else if ((rand() % 100) < self->count)
                        PBM_EasyFireDrop (self);
	}

/* Stop the flame. */
	self->touch        = NULL;
        self->solid        = SOLID_NOT;
        self->takedamage   = DAMAGE_NO;
        self->clipmask     = 0;
	self->movetype     = MOVETYPE_NONE;
        VectorClear (self->velocity);
        gi.linkentity (self);
}

/*------------------------------------------------------/ New Code /--------
//  This animates the fireball/explosion.  The magic numbers in this
//  function represent the frame numbers in id's r_explode md2.
//  While thinking, the shot will burn nearby entities, except its
//  owner; thus making the shot an area effect weapon.
//------------------------------------------------------------------------*/
void PBM_FlameThrowerThink (edict_t *self)
{
        if (++self->s.frame > 7)
	{	G_FreeEdict (self);
		return;
	}

/* Put out the fire if it hits water. */
	if (gi.pointcontents(self->s.origin) & MASK_WATER)
        {       PBM_BecomeSteam(self);
		return;
	}

	//Wheaty: Put out the fire if it is INSIDE a solid object :p
	if (gi.pointcontents(self->s.origin) & MASK_SOLID)
	{
		G_FreeEdict (self);
		return;
	}

        PBM_BurnRadius (self, self->dmg_radius, self->pos2, self->owner);

	//self->s.skinnum++;
		self->s.skinnum += 3;
	self->nextthink = level.time + FRAMETIME;


		if (rand()% 100 ==1)
        PBM_FireDrop (self->master, self->s.origin, self->pos1, self->pos2, self->dmg, self->velocity);


}


//faf:  flame starts invisible at players origin, then becomes visible here
void Become_Flame (edict_t *fireball)
{


///	//bcass start - gibola
//	if (self->client->gibmachine == flame_normal)
//	{
		fireball->s.modelindex = gi.modelindex("sprites/s_explod.sp2");
		//faf: cut down lag
		if (random() < 0.3)
		{ 
			fireball->s.effects       = EF_ROCKET;  //EF_GRENADE;
		}
			fireball->s.effects       = EF_ROCKET;  //EF_GRENADE;
fireball->s.renderfx = RF_TRANSLUCENT;

//	}
//	else
//	{
//		fireball->s.modelindex = gi.modelindex("sprites/null.sp2");
//		fireball->s.effects       = EF_GIB;  //EF_GRENADE
//	}
//		
	//bcass end

//        fireball->s.frame      = 2;
        fireball->s.skinnum    = 5;

	    fireball->nextthink       = level.time + FRAMETIME * 2;
        fireball->think           = PBM_FlameThrowerThink;
}




/*------------------------------------------------------/ New Code /--------
//  Create and launch a fat fireball.
//------------------------------------------------------------------------*/
void PBM_FireFlameThrower
(edict_t *self, vec3_t start, vec3_t spread, vec3_t dir, int speed, vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance)
{
	edict_t  *fireball;



/* Adjust fireball direction with 'spread'. */
     PBM_FireAngleSpread (spread, dir);

/* Create the fireball. */
	fireball = G_Spawn();
//        fireball->s.modelindex = gi.modelindex("models/objects/r_explode/tris.md2");


	VectorClear (fireball->mins);
	VectorClear (fireball->maxs);


//	VectorSet (fireball->mins, -8, -8, -8);
//	VectorSet (fireball->maxs, 8,8,8);



	VectorCopy (start, fireball->s.origin);
	vectoangles (dir, fireball->s.angles);
	VectorScale (dir, speed, fireball->velocity);

	VectorAdd (fireball->velocity, self->velocity, fireball->velocity);


        fireball->s.angles[PITCH] -= 90;
        fireball->s.angles[YAW]   += rand() % 360;
        fireball->movetype        = MOVETYPE_FLYMISSILE;  //MOVETYPE_TOSS;
        fireball->clipmask        = MASK_SHOT;
        fireball->solid           = SOLID_BBOX;
        fireball->takedamage      = DAMAGE_NO;
        fireball->s.renderfx      = RF_FULLBRIGHT;
        fireball->owner           = self;
        fireball->master          = self;
        fireball->classname       = "fire";
        fireball->touch           = PBM_FlameThrowerTouch;
//        fireball->nextthink       = level.time + FRAMETIME * 2;
//        fireball->think           = PBM_FlameThrowerThink;

		fireball->think = Become_Flame;
		fireball->nextthink = level.time + FRAMETIME * 2;

	VectorCopy(damage, fireball->pos1);
	VectorCopy(radius_damage, fireball->pos2);
        fireball->dmg_radius      = RADIUS_FIRE_SPLASH;
        fireball->count           = rain_chance;
        fireball->dmg             = blast_chance;
	gi.linkentity (fireball);

/* If fireball is spawned in liquid, it dissolves. */
	if (PBM_InWater (fireball))
	{       
		PBM_BecomeSteam (fireball);
		return;
	}







/* Flamethrower is an area effect weapon, so don't bother dodging. */
}


/*===========================/  END OF FILE  /===========================*/




