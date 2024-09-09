/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_turret.c,v $
 *   $Revision: 1.5 $
 *   $Date: 2002/06/04 19:49:47 $
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

// g_turret.c

#include "g_local.h"
#include "x_fire.h"
void turret_off (edict_t *self);
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean tracers_on);
void PBM_FireFlameThrower
(edict_t *self, vec3_t start, vec3_t spread, vec3_t dir, int speed, vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance);
void fire_hmg_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int mod);





void ExplosionShake (edict_t *inflictor, float damage) 
{
	float	effect_radius;
	vec3_t	v;
	edict_t	*dmgef_ent = NULL;

	effect_radius = damage + 500; //500 = radius

	while ((dmgef_ent = findradius(dmgef_ent, inflictor->s.origin, effect_radius)) != NULL)
	{
		if (!dmgef_ent->client)
			continue;

		dmgef_ent->client->dmgef_startframe = level.framenum;
		dmgef_ent->client->dmgef_sway_value = 999; // start the sway count
		
		VectorAdd (dmgef_ent->mins, dmgef_ent->maxs, v);
		VectorMA (dmgef_ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		dmgef_ent->client->dmgef_intensity = effect_radius - VectorLength (v);
	}
}



void AnglesNormalize(vec3_t vec)
{
	while(vec[0] > 360)
		vec[0] -= 360;
	while(vec[0] < 0)
		vec[0] += 360;
	while(vec[1] > 360)
		vec[1] -= 360;
	while(vec[1] < 0)
		vec[1] += 360;
}

float SnapToEights(float x)
{
	x *= 8.0;
	if (x > 0.0)
		x += 0.5;
	else
		x -= 0.5;
	return 0.125 * (int)x;
}


void turret_blocked(edict_t *self, edict_t *other)
{
	edict_t	*attacker;

//	if (other->takedamage &&
//		!other->client)
	if (!other->client)
	{
		if (self->teammaster->owner)
			attacker = self->teammaster->owner;
		else
			attacker = self->teammaster;
		T_Damage (other, self, attacker, vec3_origin, other->s.origin, vec3_origin, self->teammaster->dmg, 10000, 0, MOD_CRUSH);


		if (other)
			BecomeExplosion1 (other);

	}
}

/*QUAKED turret_breach (0 0 0) ?
This portion of the turret can change both pitch and yaw.
The model  should be made with a flat pitch.
It (and the associated base) need to be oriented towards 0.
Use "angle" to set the starting angle.

"speed"		default 50
"dmg"		default 10
"angle"		point this forward
"target"	point this at an info_notnull at the muzzle tip
"minpitch"	min acceptable pitch angle : default -30
"maxpitch"	max acceptable pitch angle : default 30
"minyaw"	min acceptable yaw angle   : default 0
"maxyaw"	max acceptable yaw angle   : default 360
*/
//void fire_gun(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean calcv);


void shell_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_R_SPLASH);
	}
	else
	{
		// don't throw any debris in net games
		//if (!deathmatch->value && !coop->value)
		//{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		//}
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


void explo_think (edict_t *ent)
{
	if (ent->s.frame > -1)
	{
		ent->s.frame++;
			if (ent->s.frame%2 == 3)
				ent->s.skinnum++;
		if (ent->s.frame == 14)
		{
			//ent->s.skinnum - 3;
			G_FreeEdict(ent);
			return;
		}
		ent->nextthink = level.time + .1;
		return;
	}
	//type 1
	ent->s.frame++;
	ent->s.skinnum++;
	if (ent->s.skinnum > 4)
	{
		G_FreeEdict(ent);
		return;
	}
	ent->nextthink = level.time + .1;
}


void explosion_effect (vec3_t start, int type)
{
	edict_t *explo;

	explo = G_Spawn();

	VectorCopy (start, explo->s.origin);
	explo->movetype = MOVETYPE_NONE;
	explo->solid = SOLID_NOT;
	explo->s.modelindex = gi.modelindex ("models/objects/r_explode/tris.md2");
	
	explo->s.renderfx |= RF_TRANSLUCENT; // <--
	
	explo->think = explo_think;
	explo->nextthink = level.time + FRAMETIME;

	if (type == 2)
		explo->s.frame = 0;

	gi.linkentity (explo);

}



void fire_shell (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*shell, *trail;

//	gi.WriteByte (svc_temp_entity);
//	gi.WriteByte (TE_EXPLOSION1);
//	gi.WritePosition (start);
//	gi.multicast (self->s.origin, MULTICAST_PVS);

	explosion_effect (start, 1);


	trail = G_Spawn();
	VectorCopy (start, trail->s.origin);
	VectorCopy (dir, trail->movedir);
	vectoangles (dir, trail->s.angles);
	VectorScale (dir, 100, trail->velocity);

    trail->s.modelindex = gi.modelindex ("sprites/null.sp2");

	trail->movetype = MOVETYPE_FLYMISSILE;
	trail->clipmask = 0;
	trail->solid = SOLID_NOT;
	VectorClear (trail->mins);
	VectorClear (trail->maxs);
	trail->s.effects |= EF_GRENADE;
	trail->nextthink = level.time + .5;
	trail->think = G_FreeEdict;
	gi.linkentity (trail);




	shell = G_Spawn();
	VectorCopy (start, shell->s.origin);
	VectorCopy (dir, shell->movedir);
	vectoangles (dir, shell->s.angles);
	VectorScale (dir, speed, shell->velocity);
	shell->movetype = MOVETYPE_FLYMISSILE;
	shell->clipmask = MASK_SHOT;
	shell->solid = SOLID_BBOX;
	VectorClear (shell->mins);
	VectorClear (shell->maxs);
	shell->owner = self;
	shell->touch = shell_touch;

//	shell->s.effects |= EF_GRENADE;
	shell->nextthink = shell->nextthink = level.time + 8000/speed;
	shell->think = G_FreeEdict;


	shell->dmg = damage;
	shell->radius_dmg = radius_damage;
	shell->dmg_radius = damage_radius;
//	shell->s.sound = gi.soundindex ("weapons/rockfly.wav");
	shell->classname = "shell";


	gi.linkentity (shell);
}

void turret_breach_fire (edict_t *self)
{
	vec3_t	f, r, u;
	vec3_t	start;
	int		damage;
	int		speed;
	vec3_t	radius_damage = {1, 1, 1};
    vec3_t  spread = {5, 5, 0};


	//touchdt = last fire time.  paindt = firing rate
	if (self->touch_debounce_time > level.time - self->pain_debounce_time)
		return;

	if (self->count == 0)
	{
//		gi.positioned_sound (self->s.origin, self, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);

		//		turret_off (self->teammaster->owner);
		self->touch_debounce_time = level.time;//last fire time
		self->pain_debounce_time = 1;
		return;
	}


	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self->teammaster->owner-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (self->s.origin, MULTICAST_PVS);




//	safe_bprintf (PRINT_HIGH, "%i.\n", self->count); 
	self->count--;
	self->touch_debounce_time = level.time;//last fire time

	AngleVectors (self->s.angles, f, r, u);
	VectorMA (self->s.origin, self->move_origin[0], f, start);
	VectorMA (start, self->move_origin[1], r, start);
	VectorMA (start, self->move_origin[2], u, start);


//	gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex(self->obj_name), 1, ATTN_NORM, 0);
gi.sound(self, CHAN_WEAPON, gi.soundindex(self->obj_name), 1, ATTN_NORM, 0);
	if (!strcmp(self->killtarget, "shell")) //ammo_type
	{
		ExplosionShake(self, 80);

		damage = self->dmg;
		if (self->numfired)
			speed = self->numfired;
		else
			speed = 1000;
		fire_shell (self->teammaster->owner, start, f, damage, speed, 150, damage);
	}
	else if (!strcmp(self->killtarget, "rocket")) //ammo_type
	{
		damage = self->dmg;
		if (self->numfired)
			speed = self->numfired;
		else
			speed = 600;
		fire_rocket (self->teammaster->owner, start, f, damage, speed, 150, damage);
	}
	else if (!strcmp(self->killtarget, "grenade")) //ammo_type
	{
		edict_t	*trail;

		ExplosionShake(self, 40);

		damage = self->dmg;
		if (self->numfired)
			speed = self->numfired;
		else
			speed = 50;
		fire_grenade (self->teammaster->owner, start, f, damage, speed, 3, 50);

		trail = G_Spawn();
		VectorCopy (start, trail->s.origin);
		VectorCopy (f, trail->movedir);
		vectoangles (f, trail->s.angles);
		VectorScale (f, 100, trail->velocity);

		trail->s.modelindex = gi.modelindex ("sprites/null.sp2");

		trail->movetype = MOVETYPE_FLYMISSILE;
		trail->clipmask = 0;
		trail->solid = SOLID_NOT;
		VectorClear (trail->mins);
		VectorClear (trail->maxs);
		trail->s.effects |= EF_GRENADE;
		trail->nextthink = level.time + .5;
		trail->think = G_FreeEdict;
		gi.linkentity (trail);

	}
	else if (!strcmp(self->killtarget, "fire")) //ammo_type
	{
		if (self->numfired)
			speed = self->numfired;
		else
			speed = 100;
    
		PBM_FireFlameThrower (self->teammaster->owner, start, spread, f, speed, radius_damage, radius_damage, 10, 0);
	}
	else if (!strcmp(self->killtarget, "tracer"))
	{
		if (self->numfired)
			speed = self->numfired;
		else
			speed = 1000;

		if (self->count%5 ==0)//tracer
			fire_tracer(self, start, f, self->dmg, MOD_SHOTGUN);
		else
			fire_hmg_bullet(self, start, f, self->dmg, MOD_SHOTGUN);

	}
	else//bullet
	{
		if (self->pain_debounce_time == -1)//double firing rate
		{
			fire_bullet(self, start, f, self->dmg, 0, (int)(crandom() * 200), (int)(crandom() * 200), MOD_SHOTGUN, 0);
			if (self->count != 0)
			{
				fire_bullet(self, start, f, self->dmg, 0,(int)(crandom() * 200), (int)(crandom() * 200), MOD_SHOTGUN, 0);
				self->count--;
			}
		}
		else
			fire_bullet(self, start, f, self->dmg, 0, 100, 100, MOD_SHOTGUN, 0);
	}
}

void turret_breach_think (edict_t *self)
{
	edict_t	*ent;
	vec3_t	current_angles;
	vec3_t	delta;



	if (self->owner &&
		self->owner->client &&
		!self->owner->deadflag)
		VectorCopy (self->owner->client->ps.viewangles, self->move_angles);



	VectorCopy (self->s.angles, current_angles);
	AnglesNormalize(current_angles);

	AnglesNormalize(self->move_angles);
	if (self->move_angles[PITCH] > 180)
		self->move_angles[PITCH] -= 360;



	// clamp angles to mins & maxs
	if (self->move_angles[PITCH] > self->pos1[PITCH])
		self->move_angles[PITCH] = self->pos1[PITCH];
	else if (self->move_angles[PITCH] < self->pos2[PITCH])
		self->move_angles[PITCH] = self->pos2[PITCH];

//	if (self->owner)
//		safe_bprintf (PRINT_HIGH, "%f\n", self->move_angles[YAW]); 

	if (self->pos1[YAW] < 0 && self->move_angles[YAW] > 180)
	{
		self->move_angles[YAW] -= 360;
	}

	if ((self->move_angles[YAW] < self->pos1[YAW]) || (self->move_angles[YAW] > self->pos2[YAW]))
	{
		float	dmin, dmax;

		dmin = fabs(self->pos1[YAW] - self->move_angles[YAW]);
		if (dmin < -180)
			dmin += 360;
		else if (dmin > 180)
			dmin -= 360;
		dmax = fabs(self->pos2[YAW] - self->move_angles[YAW]);
		if (dmax < -180)
			dmax += 360;
		else if (dmax > 180)
			dmax -= 360;
		if (fabs(dmin) < fabs(dmax))
			self->move_angles[YAW] = self->pos1[YAW];
		else
			self->move_angles[YAW] = self->pos2[YAW];
	}

	if (self->move_angles[YAW] <0)
		self->move_angles[YAW]+= 360;





	VectorSubtract (self->move_angles, current_angles, delta);
	if (delta[0] < -180)
		delta[0] += 360;
	else if (delta[0] > 180)
		delta[0] -= 360;
	if (delta[1] < -180)
		delta[1] += 360;
	else if (delta[1] > 180)
		delta[1] -= 360;
	delta[2] = 0;

	if (delta[0] > self->speed * FRAMETIME)
		delta[0] = self->speed * FRAMETIME;
	if (delta[0] < -1 * self->speed * FRAMETIME)
		delta[0] = -1 * self->speed * FRAMETIME;
	if (delta[1] > self->speed * FRAMETIME)
		delta[1] = self->speed * FRAMETIME;
	if (delta[1] < -1 * self->speed * FRAMETIME)
		delta[1] = -1 * self->speed * FRAMETIME;

	VectorScale (delta, 1.0/FRAMETIME, self->avelocity);

	self->nextthink = level.time + FRAMETIME;

	for (ent = self->teammaster; ent; ent = ent->teamchain)
		ent->avelocity[1] = self->avelocity[1];



	if (self->map)//(turret_sound)
	{
		if (self->avelocity[0]!=0 ||
			self->avelocity[1]!=0 ||
			self->avelocity[2]!=0)
			self->s.sound = gi.soundindex(self->map);
		else
			self->s.sound = 0;
	}



	// if we have adriver, adjust his velocities
	if (self->owner)
	{
		float	angle;
		float	target_z;
		float	diff;
		vec3_t	target;
		vec3_t	dir;

		// angular is easy, just copy ours
		self->owner->avelocity[0] = self->avelocity[0];
		self->owner->avelocity[1] = self->avelocity[1];

		// x & y
		angle = self->s.angles[1] + self->owner->move_origin[1];
		angle *= (M_PI*2 / 360);
//		target[0] = SnapToEights(self->s.origin[0] + cos(angle) * self->owner->move_origin[0]);
//		target[1] = SnapToEights(self->s.origin[1] + sin(angle) * self->owner->move_origin[0]);
//		target[2] = self->owner->s.origin[2];
		target[0] = self->s.origin[0] + cos(angle) * self->owner->move_origin[0];
		target[1] = self->s.origin[1] + sin(angle) * self->owner->move_origin[0];
		target[2] = self->owner->s.origin[2];

		VectorSubtract (target, self->owner->s.origin, dir);
		self->owner->velocity[0] = dir[0] * 1.0 / FRAMETIME;
		self->owner->velocity[1] = dir[1] * 1.0 / FRAMETIME;

		// z
		angle = self->s.angles[PITCH] * (M_PI*2 / 360);
//		target_z = SnapToEights(self->s.origin[2] + self->owner->move_origin[0] * tan(angle) + self->owner->move_origin[2]);
		target_z = self->s.origin[2] + self->owner->move_origin[0] * tan(angle) + self->owner->move_origin[2];

		diff = target_z - self->owner->s.origin[2];
		self->owner->velocity[2] = diff * 1.0 / FRAMETIME;




//		if (self->spawnflags & 65536)
//		{
//			turret_breach_fire (self);
//			self->spawnflags &= ~65536;
//		}
	}
}

void turret_breach_finish_init (edict_t *self)
{
	// get and save info for muzzle location
	if (!self->target)
	{
		gi.dprintf("%s at %s needs a target\n", self->classname, vtos(self->s.origin));
	}
	else
	{
		self->target_ent = G_PickTarget (self->target);
		VectorSubtract (self->target_ent->s.origin, self->s.origin, self->move_origin);
		G_FreeEdict(self->target_ent);
	}

	self->teammaster->dmg = self->dmg;
	self->think = turret_breach_think;
	self->think (self);
}




void turret_breach_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t *t;

	t = NULL;

	if (self->owner)
	{
		if (self->owner->solid == SOLID_NOT)
		{
			turret_off(self->owner);
//crash			T_Damage (self->owner, attacker, attacker, vec3_origin, self->owner->s.origin, vec3_origin, 300, 0, DAMAGE_NO_PROTECTION, MOD_R_SPLASH);
		}
		else
			turret_off(self->owner);
	}
	while ((t = G_Find (t, FOFS(classname), "turret_range")))
	{
		if (!t->inuse)
			continue;

		if (!strcmp(self->targetname, t->target))
			G_FreeEdict(t);
	}

	t = NULL;
	while ((t = G_Find (t, FOFS(classname), "turret_base")))
	{
		if (!t->inuse)
			continue;

		if (!strcmp(self->team, t->team))
			BecomeExplosion1(t);
	}


	if (self->deathtarget)
		self->target = self->deathtarget;

	if (self->target)
		G_UseTargets (self, inflictor);


	BecomeExplosion1 (self);
}
void turret_base_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t *t;

	t = NULL;

	if (self->owner)
		turret_off(self->owner);

/*	while ((t = G_Find (t, FOFS(classname), "turret_range")))
	{
		if (!t->inuse)
			continue;

		if (!strcmp(self->targetname, t->target))
			G_FreeEdict(t);
	}
	t = NULL;*/
	while ((t = G_Find (t, FOFS(classname), "turret_breach")))
	{
		if (!t->inuse)
			continue;

		if (!strcmp(self->team, t->team))
		{
			t->takedamage = DAMAGE_YES;
			t->health = 1;
			T_Damage (t, inflictor, self, vec3_origin, t->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
			
		}
	}



//	BecomeExplosion1 (self);
}



void SP_turret_breach (edict_t *self)
{
	if (self->pathtarget)
	{
		gi.setmodel(self, self->pathtarget);
	}
	else
	{
		self->solid = SOLID_BSP;
		gi.setmodel (self, self->model);
	}

		self->movetype = MOVETYPE_PUSH;

	if (self->health > 0)
		self->takedamage   = DAMAGE_YES;


	self->die = turret_breach_die;


	if (!self->speed)
		self->speed = 50;
	if (!self->dmg)
		self->dmg = 10;

	if (!st.minpitch)
		st.minpitch = -30;
	if (!st.maxpitch)
		st.maxpitch = 30;
	if (!st.maxyaw)
		st.maxyaw = 360;

	self->pos1[PITCH] = -1 * st.minpitch;
	self->pos1[YAW]   = st.minyaw;
	self->pos2[PITCH] = -1 * st.maxpitch;
	self->pos2[YAW]   = st.maxyaw;

	self->ideal_yaw = self->s.angles[YAW];
	self->move_angles[YAW] = self->ideal_yaw;

	self->blocked = turret_blocked;

	self->think = turret_breach_finish_init;
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
}


/*QUAKED turret_base (0 0 0) ?
This portion of the turret changes yaw only.
MUST be teamed with a turret_breach.
*/

void SP_turret_base (edict_t *self)
{
	if (self->pathtarget)
	{
		gi.setmodel(self, self->pathtarget);
	}
	else
	{
		self->solid = SOLID_BSP;
		gi.setmodel (self, self->model);
	}
		self->movetype = MOVETYPE_PUSH;
	self->blocked = turret_blocked;


	if (self->health > 0)
		self->takedamage   = DAMAGE_YES;

	self->die = turret_base_die;




	gi.linkentity (self);
}


/*QUAKED turret_driver (1 .5 0) (-16 -16 -24) (16 16 32)
Must NOT be on the team with the rest of the turret parts.
Instead it must target the turret_breach.
*/

//void infantry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage);
//void infantry_stand (edict_t *self);
void monster_use (edict_t *self, edict_t *other, edict_t *activator);

void turret_driver_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t	*ent;

	// level the gun
	self->target_ent->move_angles[0] = 0;

	// remove the driver from the end of them team chain
	for (ent = self->target_ent->teammaster; ent->teamchain != self; ent = ent->teamchain)
		;
	ent->teamchain = NULL;
	self->teammaster = NULL;
	self->flags &= ~FL_TEAMSLAVE;

	self->target_ent->owner = NULL;
	self->target_ent->teammaster->owner = NULL;

//	infantry_die (self, inflictor, attacker, damage);
}



void turret_off (edict_t *self)
{
	edict_t	*ent;


	if (!self->client)
		return;

	if (!self->client->turret)
		return;

	if (self->client->turret->obj_gain)
		self->solid = SOLID_TRIGGER;//keep from getting stuck in other players
	else
		self->solid = SOLID_BBOX;

	self->svflags &= ~SVF_NOCLIENT; 

	self->movetype = MOVETYPE_WALK;


	VectorCopy (self->s.origin, self->client->turret->last_turret_driver_spot);

	self->client->ps.gunindex = gi.modelindex(self->client->pers.weapon->view_model);
	self->client->ps.fov = STANDARD_FOV;
	self->movetype = MOVETYPE_WALK;

	// level the gun
	self->target_ent->move_angles[0] = 0;

	// remove the driver from the end of them team chain
	for (ent = self->target_ent->teammaster; ent->teamchain != self; ent = ent->teamchain)
		;
	ent->teamchain = NULL;
	self->teammaster = NULL;
	self->flags &= ~FL_TEAMSLAVE;


	self->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;



	self->target_ent->owner = NULL;
	self->target_ent->teammaster->owner = NULL;

	self->client->turret = NULL;

//	infantry_die (self, inflictor, attacker, damage);
}


qboolean FindTarget (edict_t *self);

void turret_driver_think (edict_t *self)
{
	vec3_t	target;
	vec3_t	dir;
	float	reaction_time;

//	self->nextthink = level.time + FRAMETIME;

	if (self->enemy && (!self->enemy->inuse || self->enemy->health <= 0))
		self->enemy = NULL;

	if (!self->enemy)
	{
		if (!FindTarget (self))
			return;
		self->monsterinfo.trail_time = level.time;
		self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
	}
	else
	{
		if (visible (self, self->enemy))
		{
			if (self->monsterinfo.aiflags & AI_LOST_SIGHT)
			{
				self->monsterinfo.trail_time = level.time;
				self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
			}
		}
		else
		{
			self->monsterinfo.aiflags |= AI_LOST_SIGHT;
			return;
		}
	}

	// let the turret know where we want it to aim
	VectorCopy (self->enemy->s.origin, target);
	target[2] += self->enemy->viewheight;
	VectorSubtract (target, self->target_ent->s.origin, dir);
	vectoangles (dir, self->target_ent->move_angles);

	// decide if we should shoot
	if (level.time < self->monsterinfo.attack_finished)
		return;

	reaction_time = (3 - skill->value) * 1.0;
	if ((level.time - self->monsterinfo.trail_time) < reaction_time)
		return;

	self->monsterinfo.attack_finished = level.time + reaction_time + 1.0;
	//FIXME how do we really want to pass this along?
	self->target_ent->spawnflags |= 65536;
}
void change_stance(edict_t *self, int stance);
void turret_driver_link (edict_t *self)
{
	vec3_t	vec;
	edict_t	*ent;

//	self->think = turret_driver_think;
//	self->nextthink = level.time + FRAMETIME;


	edict_t *t;

	int i;
	
	if (self->client->turret->obj_gain && 
		self->client->turret->obj_gain == 1) //(mapper set ("tank" to 1))
	{
		self->svflags |= SVF_NOCLIENT; 
		self->solid = SOLID_NOT;
		self->movetype = MOVETYPE_NOCLIP; 
	}

	if (self->client->turret->sounds)
	{

		change_stance (self, self->client->turret->sounds);
//		gi.dprintf ("%s\n",self->client->turret->sounds);
	}
		self->movetype = MOVETYPE_FLYMISSILE; 





	VectorClear(self->velocity);

	self->client->ps.pmove.pm_type = PM_FREEZE;

		for (i=0 ; i<3 ; i++)
			self->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(self->client->turret->s.angles[i] - self->client->resp.cmd_angles[i]);

		VectorCopy(self->client->turret->s.angles, self->client->ps.viewangles);
		VectorCopy(self->client->turret->s.angles, self->client->v_angle);




//	self->movetype = MOVETYPE_NOCLIP;
self->movetype = MOVETYPE_NOCLIP;

	self->client->ps.fov = 70;

	self->client->ps.gunindex = 0; 

	self->target_ent = self->client->turret;

	self->target_ent->owner = self;
	self->target_ent->teammaster->owner = self;


	self->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;



	t = NULL;
			
	while ((t = G_Find (t, FOFS(classname), "turret_driver")))
	{
		if (!strcmp(t->target, self->target_ent->targetname))
		{
//			safe_bprintf (PRINT_HIGH, "%s   %s\n", t->target, vtos(t->s.origin));
			VectorCopy (t->s.origin, self->s.origin);
		}
	}

	VectorCopy (self->target_ent->s.angles, self->s.angles);

	vec[0] = self->target_ent->s.origin[0] - self->s.origin[0];
	vec[1] = self->target_ent->s.origin[1] - self->s.origin[1];
	vec[2] = 0;

	self->move_origin[0] = VectorLength(vec);

	VectorSubtract (self->s.origin, self->target_ent->s.origin, vec);
	vectoangles (vec, vec);
	AnglesNormalize(vec);
	self->move_origin[1] = vec[1];

	self->move_origin[2] = self->s.origin[2] - self->target_ent->s.origin[2];

	// add the driver to the end of them team chain
	for (ent = self->target_ent->teammaster; ent->teamchain; ent = ent->teamchain)
		;
	ent->teamchain = self;
	self->teammaster = self->target_ent->teammaster;
	self->flags |= FL_TEAMSLAVE;

	if (!VectorCompare (self->client->turret->last_turret_driver_spot,vec3_origin))
	{
		VectorCopy (self->client->turret->last_turret_driver_spot, self->s.origin);
	}



}

void SP_turret_driver (edict_t *self)
{
}
/*
void SP_turret_driver (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_PUSH;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/infantry/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->health = 100;
	self->gib_health = 0;
	self->mass = 200;
	self->viewheight = 24;
/*-----/ PM /-----/ NEW:  Set fireflags. /-----*/
  //  self->fireflags = FIREFLAG_DELTA_BASE | FIREFLAG_IGNITE;
/*---------------------------------------------*/
/*
	self->die = turret_driver_die;
	self->monsterinfo.stand = infantry_stand;

	self->flags |= FL_NO_KNOCKBACK;

	level.total_monsters++;

	self->svflags |= SVF_MONSTER;
	self->s.renderfx |= RF_FRAMELERP;
	self->takedamage = DAMAGE_AIM;
	self->use = monster_use;
	self->clipmask = MASK_MONSTERSOLID;
	VectorCopy (self->s.origin, self->s.old_origin);
	self->monsterinfo.aiflags |= AI_STAND_GROUND|AI_DUCKED;

	if (st.item)
	{
		self->item = FindItemByClassname (st.item);
		if (!self->item)
			gi.dprintf("%s at %s has bad item: %s\n", self->classname, vtos(self->s.origin), st.item);
	}

	self->think = turret_driver_link;
	self->nextthink = level.time + FRAMETIME;

	gi.linkentity (self);
}
*/

void Turret_Touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->health <= 0)
		return;

	if (other->client &&
		other->client->resp.team_on &&
		!other->deadflag &&
		!other->flyingnun)
	{
		self->oldenemy = other; //indicates what player is in range
		self->obj_time = level.time; //last time player touched
	}
}

void SP_turret_range (edict_t *self)
{
	self->solid = SOLID_TRIGGER;
	self->touch = Turret_Touch;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	if (!VectorCompare(self->s.angles, vec3_origin))
		G_SetMovedir (self->s.angles, self->movedir);

	gi.setmodel (self, self->model);
	gi.linkentity (self);
}


qboolean CheckForTurret(edict_t *ent)
{
	edict_t *t, *turret;

	t = NULL;
			
	while ((t = G_Find (t, FOFS(classname), "turret_range")))
	{
		if (t->oldenemy == ent &&
			t->obj_time > level.time - .2 &&
			!t->owner)
		{
			turret = G_PickTarget(t->target);
			if (!turret->owner)
			{
				ent->client->turret = G_PickTarget(t->target);
//			ent->target_ent = ent->client->turret;
				turret_driver_link (ent);
				return true;
			}
			else
				return false;
		
		}
	}
	return false;
}


void Weapon_Turret_Fire (edict_t *ent)
{
	if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
	{
		turret_breach_fire (ent->client->turret);
	}
}

