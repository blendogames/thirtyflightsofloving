// g_turret.c

//CW: This turret code is taken from the Lazarus mod source (version 2.1),
//    which David Hyde released on 22-Sep-01 for public use.
//    See 'http://planetquake.com/lazarus' for more information.


#include "g_local.h"

#define SF_TURRETDRIVER_REMOTE_DRIVER	1
#define SF_TURRET_PLAYER_CONTROLLABLE	1
#define SF_TURRET_TRIGGER_SPAWN			2
#define SF_TURRET_TRACKING				4
#define SF_TURRET_GOODGUY				8
#define SF_TURRET_INACTIVE				16
#define SF_TURRET_MD2					32

//CW++
#define SF_TURRET_TEAM1					64
#define SF_TURRET_TEAM2					128
//CW--

void NoAmmoWeaponChange (edict_t *ent);

// DH++
// Added TurretTarget to scan the player's view for a damageable target.
// Used with homing rockets
edict_t	*TurretTarget(edict_t *self)
{
	edict_t	*who;
	edict_t	*best;
	trace_t	tr;
	float	bd;
	float	d;
	int		i;
	vec3_t	dir;
	vec3_t	end;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	up;
	vec3_t	start;

	AngleVectors(self->s.angles, forward, right, up);
	VectorMA(self->s.origin, self->move_origin[0], forward, start);
	VectorMA(start,          self->move_origin[1], right,   start);
	VectorMA(start,          self->move_origin[2], up,      start);
	VectorMA(start, WORLD_SIZE, forward, end);	// was 8192

	/* Check for aiming directly at a damageable entity */
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent->takedamage != DAMAGE_NO) && (tr.ent->solid != SOLID_NOT))
		return tr.ent;

	/* Check for damageable entity within a tolerance of view angle */
	bd = 0;
	best = NULL;
	for (i = 1, who = g_edicts+1; i < globals.num_edicts; i++, who++)
	{
		if (!who->inuse)
			continue;
		if (who->takedamage == DAMAGE_NO)
			continue;
		if (who->solid == SOLID_NOT)
			continue;

		VectorMA(who->absmin,0.5,who->size,end);
		tr = gi.trace (start, vec3_origin, vec3_origin, end, self, MASK_OPAQUE);
		if (tr.fraction < 1.0)
			continue;

		VectorSubtract(end, self->s.origin, dir);
		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if (d > bd)
		{
			bd = d;
			best = who;
		}
	}
	if (bd > 0.90)
		return best;

	return NULL;
}
//DH--

void turret_blocked(edict_t *self, edict_t *other)
{
	edict_t	*attacker;
	edict_t	*ent;
	edict_t	*master;

	if (other == world)
	{
		// world brush - stop
		self->avelocity[YAW] = 0;
		if (self->team)
		{
			for (ent = self->teammaster; ent; ent = ent->teamchain)
				ent->avelocity[YAW] = 0;
		}
		if (self->owner)
			self->owner->avelocity[YAW] = 0;

		gi.linkentity(self);
	}

	if (other->takedamage)
	{
		vec3_t	dir;
		VectorSubtract(other->s.origin,self->s.origin,dir);
		VectorNormalize(dir);

		if (self->teammaster)
			master = self->teammaster;
		else
			master = self;

		if (self->teammaster)
		{
			if (self->teammaster->owner)
				attacker = self->teammaster->owner;
			else
				attacker = self->teammaster;
		}
		else if(self->owner)
			attacker = self->owner;
		else
			attacker = self;

		// give a big kickback to help prevent getting stuck
		T_Damage(other, self, attacker, dir, other->s.origin, vec3_origin, master->dmg, 50, 0, MOD_CRUSH);
	}
	if (!(other->svflags & SVF_MONSTER) && (!other->client))
	{
		T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		if (other)
			BecomeExplosion1(other);

		return;
	}
}

void turret_breach_fire(edict_t *self)
{
	edict_t	*owner;
	vec3_t	forward;
	vec3_t	right;
	vec3_t	up;
	vec3_t	start;

	AngleVectors(self->s.angles, forward, right, up);
	VectorMA(self->s.origin, self->move_origin[0], forward, start);
	VectorMA(start, self->move_origin[1], right, start);
	VectorMA(start, self->move_origin[2], up, start);

	// DWH: automated turrets have no driver, so use self
	if (self->owner && !(self->owner->spawnflags & SF_TURRETDRIVER_REMOTE_DRIVER))
		owner = self->owner;
	else
		owner = self;
	/*
	ed - self->teammaster->owner causes quake 2 to crash when the player uses BUTTON_SHOOT
	     its been changed to self->owner incase anything weird happens.
	*/
	//FIXME : only use the normal damages if self->owner (turret_driver) doesn't have one

	if (self->delay <= level.time + 0.001)			//CW: Was just < , but this screwed up the RoF
	{												//CW: Also, added 0.001 to account for fp error
		switch (self->sounds)
		{
			case 1: // Rocket Launcher
				Fire_Rocket (owner, start, forward, self->mass, sv_rocket_speed->value, 150, self->mass, false);
				gi.positioned_sound(start, self, CHAN_WEAPON, gi.soundindex("weapons/rocklf1a.wav"), 1, ATTN_NORM, 0);
				self->delay = level.time + self->wait;
				break;

			case 2: // Railgun
				Fire_Rail (owner, start, forward, self->mass, 2, false, 0, 0, 0);
				gi.positioned_sound(start, self, CHAN_WEAPON, gi.soundindex("weapons/railgf1a.wav"), 1, ATTN_NORM, 0);
				gi.WriteByte(svc_muzzleflash);
				gi.WriteShort(self-g_edicts);
				gi.WriteByte(MZ_RAILGUN);
				gi.multicast(start, MULTICAST_PVS);
				self->delay = level.time + self->wait;
				break;

			case 3: // Machinegun
				if (++self->style > 4)
				{
					vec3_t  t_start;
					trace_t tr;

					VectorMA(start, 300.0, forward, t_start);
					tr = gi.trace(start, NULL, NULL, t_start, self, MASK_SHOT);
					if (tr.fraction == 1.0)
						Fire_Tracer(owner, t_start, forward, 2000.0, 0.3);

					self->style = 0;
				}
				Fire_Bullet(owner, start, forward, self->mass, 4, DEFAULT_MGTURRET_HSPREAD, DEFAULT_MGTURRET_VSPREAD, MOD_MACHINEGUN);
				gi.positioned_sound(start, self, CHAN_WEAPON, gi.soundindex("turret/mchngun.wav"), 1, ATTN_NORM, 0);
				self->delay = level.time + FRAMETIME;		// no delay on machinegun (=> fires every server frame)
				break;
//CW++
			case 4: // E.S.G.
				Fire_Spike(owner, start, forward, self->mass, sv_spike_speed->value, 2, sv_spike_bang_radius->value, (int)sv_spike_bang_damage->value);
				gi.positioned_sound(start, self, CHAN_WEAPON, gi.soundindex("weapons/esg/fire.wav"), 1, ATTN_NORM, 0);
				self->delay = level.time + self->wait;
				break;

			case 5: // Shock Rifle
				Fire_Shock(self, start, forward, 0, sv_shock_speed->value, 2, self->mass, sv_shock_radius->value, false);
				gi.WriteByte(svc_muzzleflash);
				gi.WriteShort(self-g_edicts);
				gi.WriteByte(MZ_BFG);
				gi.multicast(self->s.origin, MULTICAST_PVS);
				self->delay = level.time + self->wait;
				break;

			case 6:	// Plasma Launcher
				Fire_Plasma(owner, start, forward, self->mass, sv_plasma_speed->value);
				gi.positioned_sound(start, self, CHAN_WEAPON, gi.soundindex("floater/fltatck1.wav"), 1, ATTN_NORM, 0);
				self->delay = level.time + self->wait;
				break;
//CW--
			default: // Rocket Launcher
				Fire_Rocket(owner, start, forward, self->mass, sv_rocket_speed->value, 150, self->mass, false);
				gi.positioned_sound(start, self, CHAN_WEAPON, gi.soundindex("weapons/rocklf1a.wav"), 1, ATTN_NORM, 0);
				self->delay = level.time + self->wait;
				break;
		}
	}
}

void turret_disengage(edict_t *self)
{
	edict_t *ent;
	vec3_t  forward;
	int     i;

	// level the gun
	self->move_angles[0] = 0.0;
				
	ent = self->owner;
				
	//ed - to keep remove tracking of the entity
	ent->turret = NULL;
				
	// throw them back from turret
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorScale(forward, -300.0, forward);
	forward[2] = forward[2] + 150.0;
	if (forward[2] < 80.0)
		forward[2] = 80.0;
				
	for (i = 0; i < 3; i++)
		ent->velocity[i] = forward[i];
		
	ent->s.origin[2] = ent->s.origin[2] + 1.0;
	ent->movetype = MOVETYPE_WALK;
	ent->gravity = 1.0;
				
	ent->flags &= ~FL_TURRET_OWNER;
				
	// turn ON client side prediction for this player
	ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				
	gi.linkentity(ent);

	self->owner = NULL;
}

void turret_turn(edict_t *self)
{
	vec3_t		current_angles;
	vec3_t		delta;
	qboolean	yaw_restrict;

	VectorCopy(self->s.angles, current_angles);
	AnglesNormalize(current_angles);

	if (self->viewer && self->viewer->client)
	{
		gclient_t *client = self->viewer->client;

		if ((client->old_owner_angles[0] != client->ucmd.angles[0]) || (client->old_owner_angles[1] != client->ucmd.angles[1]))
		{
			// Give game a bit of time to catch up after player causes ucmd pitch angle to roll over... 
			// otherwise we'll hit on the above test even though player hasn't hit +lookup/+lookdown
			float	delta;

			delta = level.time - self->touch_debounce_time;
			if ((delta < 0.0) || (delta > 1.0))
			{
				float	delta_angle;
				float	fastest = self->speed * FRAMETIME;
				
				delta_angle = SHORT2ANGLE(client->ucmd.angles[0]-client->old_owner_angles[0]);
				if (delta_angle < -180.0)
					delta_angle += 360.0;
				if (delta_angle > 180.0)
					delta_angle -= 360.0;
				if (delta_angle > fastest)
					delta_angle = fastest;
				if (delta_angle < -fastest)
					delta_angle = -fastest;
				self->move_angles[0] += delta_angle;
				
				delta_angle = SHORT2ANGLE(client->ucmd.angles[1]-client->old_owner_angles[1]);
				if (delta_angle < -180.0)
					delta_angle += 360.0;
				if (delta_angle > 180.0)
					delta_angle -= 360.0;
				if (delta_angle > fastest)
					delta_angle = fastest;
				if (delta_angle < -fastest)
					delta_angle = -fastest;
				self->move_angles[1] += delta_angle;

				client->old_owner_angles[0] = client->ucmd.angles[0];
				client->old_owner_angles[1] = client->ucmd.angles[1];
			}
			self->touch_debounce_time = level.time + 5.0;
		}
	}
	
//=======	
	AnglesNormalize(self->move_angles);

	// clamp angles to mins & maxs
	if (self->move_angles[PITCH] > self->pos1[PITCH])
		self->move_angles[PITCH] = self->pos1[PITCH];
	else if (self->move_angles[PITCH] < self->pos2[PITCH])
		self->move_angles[PITCH] = self->pos2[PITCH];

	// Lazarus: Special case - if there are no constraints on YAW, don't adjust angle
	if ((self->pos1[YAW] != 0.0) || (self->pos2[YAW] != 360.0))
		yaw_restrict = true;
	else
		yaw_restrict = false;

	if (yaw_restrict)
	{
		float	yaw_range;
		float	yaw_base;

		yaw_range = self->pos2[YAW] - self->pos1[YAW];
		if (yaw_range < 0.0)
			yaw_range += 360.0;

		yaw_base  = self->move_angles[YAW] - self->pos1[YAW];
		if (yaw_base < 0.0)
			yaw_base += 360.0;

		if (yaw_base > yaw_range)
		{
			float dmin;
			float dmax;

			dmin = fabs(self->pos1[YAW] - self->move_angles[YAW]);
			if (dmin < 0.0)
				dmin += 360.0;
			else if (dmin > 360.0)
				dmin -= 360.0;
			
			dmax = fabs(self->pos2[YAW] - self->move_angles[YAW]);
			if (dmax < 0.0)
				dmax += 360.0;
			else if (dmax > 360.0)
				dmax -= 360.0;

			if (fabs(dmin) < fabs(dmax))
				self->move_angles[YAW] = self->pos1[YAW];
			else
				self->move_angles[YAW] = self->pos2[YAW];
		}
	}

	VectorSubtract(self->move_angles, current_angles, delta);
	if (delta[0] < -180.0)
		delta[0] += 360.0;
	else if (delta[0] > 180.0)
		delta[0] -= 360.0;

	if (delta[1] < -180.0)
		delta[1] += 360.0;
	else if (delta[1] > 180.0)
		delta[1] -= 360.0;
	
	delta[2] = 0.0;

	VectorScale(delta, 1.0/FRAMETIME, delta);
	if (delta[0] >  self->speed)
		delta[0] =  self->speed;
	if (delta[0] < -self->speed)
		delta[0] = -self->speed;
	if (delta[1] >  self->speed)
		delta[1] =  self->speed;
	if (delta[1] < -self->speed)
		delta[1] = -self->speed;
	VectorCopy(delta, self->avelocity);

	if (self->team)
	{
		edict_t	*ent;
		for (ent = self->teammaster; ent; ent = ent->teamchain)
		{
			ent->avelocity[1] = self->avelocity[1];
			if (ent->solid == SOLID_NOT)
				ent->avelocity[0] = self->avelocity[0];
		}
	}
}

void turret_breach_think(edict_t *self)
{
	edict_t		*ent;
	edict_t		*victim;
	trace_t		tr;
	vec3_t		dir, angles;
	vec3_t		target;
	qboolean	remote_monster;
	qboolean	yaw_restrict;
	float		yaw_r;
	float		yaw_0 = 0;																			//CW
	int			i;

	turret_turn(self);
	yaw_r = self->pos2[YAW] - self->pos1[YAW];
	if (yaw_r < 0.0)
		yaw_r += 360.0;

	if ((self->pos1[YAW] != 0.0) || (self->pos2[YAW] != 360.0))
		yaw_restrict = true;
	else
		yaw_restrict = false;

	self->nextthink = level.time + FRAMETIME;

	if (self->deadflag == DEAD_DEAD)
		return;

	remote_monster = false;

	if (self->owner || self->viewer)
	{
		if (!(self->spawnflags & SF_TURRET_MD2))
		{
			self->s.effects &= ~EF_ANIM23;
			self->s.effects |= EF_ANIM01;
		}
	}
	else
	{
		if (!(self->spawnflags & SF_TURRET_MD2))
		{
			self->s.effects &= ~EF_ANIM01;
			self->s.effects |= EF_ANIM23;
		}
	}

	if (self->team)
	{
		for (ent = self->teammaster; ent; ent = ent->teamchain)
		{
			if (ent != self->owner)
			{
				if (ent->solid != SOLID_NOT)
					ent->s.effects = self->s.effects;
			}
		}
	}

	// if we have a driver, adjust his velocities
	if (self->owner && !remote_monster)
	{
		if (self->owner->target_ent == self)
		{
			float	angle;
			float	diff;
			float	target_z;
			vec3_t	dir;
			
			// angular is easy, just copy ours
			self->owner->avelocity[0] = self->avelocity[0];
			self->owner->avelocity[1] = self->avelocity[1];
			
			// x & y
			angle = self->s.angles[1] + self->owner->move_origin[1];
//CW++
			angle = DEG2RAD(angle);
//CW--
			target[0] = SnapToEights(self->s.origin[0] + cos(angle) * self->owner->move_origin[0]);
			target[1] = SnapToEights(self->s.origin[1] + sin(angle) * self->owner->move_origin[0]);
			target[2] = self->owner->s.origin[2];
			
			VectorSubtract (target, self->owner->s.origin, dir);
			self->owner->velocity[0] = dir[0] * 1.0 / FRAMETIME;
			self->owner->velocity[1] = dir[1] * 1.0 / FRAMETIME;
			
			// z
//CW++
			angle = DEG2RAD(self->s.angles[PITCH]);
//CW--
			target_z = SnapToEights(self->s.origin[2] + self->owner->move_origin[0] * tan(angle) + self->owner->move_origin[2]);
			
			diff = target_z - self->owner->s.origin[2];
			self->owner->velocity[2] = diff * 1.0 / FRAMETIME;
			
			if (self->spawnflags & 65536)
			{
				turret_breach_fire(self);
				self->spawnflags &= ~65536;
			}
			return;
		}
		else if (self->spawnflags & SF_TURRET_PLAYER_CONTROLLABLE)
		{	// a player is controlling the turret, move towards view angles
			vec3_t	target, forward;

			for (i = 0; i < 3; i++)
				self->move_angles[i] = self->owner->client->v_angle[i];

			// FIXME: do a tracebox from up and behind towards the turret, to try and keep them from
			// getting stuck inside the rotating turre
			// x & y
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorScale(forward, 32.0, forward);
			VectorSubtract(self->s.origin, forward, target);
			VectorAdd(target, tv(0.0,0.0,8.0), self->owner->s.origin);
			gi.linkentity(self->owner);

			if (self->owner->client->ps.pmove.velocity[2] > 15) 
				turret_disengage(self);
		}
	}
	else if ((self->spawnflags & SF_TURRET_PLAYER_CONTROLLABLE) && !remote_monster)
	{	// check if a player has mounted the turret

		edict_t	*ent;
		vec3_t	target;
		vec3_t	forward;
		vec3_t	dir;
		int		i;

		// find a player
		ent = &g_edicts[0];
		ent++;
		for (i = 0; i < (int)maxclients->value; i++, ent++)
		{
			if (!ent->inuse)
				continue;
			if (ent->solid == SOLID_NOT)
				continue;

			// determine distance from turret seat location

			// x & y
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorScale(forward, 32.0, forward);
			VectorSubtract(self->s.origin, forward, target);

			VectorSubtract(target, ent->s.origin, dir);
			if (fabs(dir[2]) < 64.0)
				dir[2] = 0.0;

			if (VectorLength(dir) < 16.0)
			{	// player has taken control of turret
				self->owner = ent;
				ent->movetype = MOVETYPE_PUSH;	// don't let them move, or they'll get stuck
				ent->gravity = 0.0;

				//ed - to keep track of the entity
				ent->turret = self;

				// turn off client side prediction for this player
				ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;

				gi.linkentity(ent);

				//ed - set the flag on the client so that when they shoot the 
				//     turret shoots instead of "using" it
				ent->flags |= FL_TURRET_OWNER;
			}
		}
	}

	if ((self->spawnflags & SF_TURRET_TRACKING) && (!self->owner || remote_monster))
	{
		// TRACK - automated turret
		edict_t	*gomer;
		vec3_t	f;
		vec3_t	forward;
		vec3_t	right;
		vec3_t	up;
		vec3_t	start;
		vec3_t	t_start;
		float	best_dist = WORLD_SIZE;	// was 8192.0
		float	dist;

		if (self->viewer && (level.time < self->touch_debounce_time))
			return;

		AngleVectors(self->s.angles, forward, right, up);
		VectorMA(self->s.origin,self->move_origin[0], forward, start);
		VectorMA(start,         self->move_origin[1], right,   start);
		VectorMA(start,         self->move_origin[2], up,      start);

		self->oldenemy = self->enemy;

		if (self->enemy)
		{
			// Check that current enemy is valid. if so, find
			// distance. Don't switch enemies unless another
			// monster is at least 100 units closer to the camera
			if (self->enemy->inuse)
			{
				if ((self->enemy->health > self->enemy->gib_health) && !(self->enemy->svflags & SVF_NOCLIENT) && 
					!(self->enemy->flags & FL_NOTARGET))
				{
					if (gi.inPVS(self->s.origin,self->enemy->s.origin))
					{
						VectorMA(self->enemy->absmin,0.5,self->enemy->size,target);
						VectorSubtract(target,self->s.origin,dir);
						vectoangles(dir,angles);
						AnglesNormalize(angles);
						if (yaw_restrict)
						{
							yaw_0 = angles[YAW] - self->pos1[YAW];
							if (yaw_0 < 0.0)
								yaw_0 += 360.0;
						}

						if ((angles[PITCH] > self->pos1[PITCH]) || (angles[PITCH] < self->pos2[PITCH]) || (yaw_restrict && (yaw_0 > yaw_r)))
							self->enemy = NULL;
						else
						{
							VectorCopy(self->s.origin,t_start);
							VectorCopy(dir,f);
							VectorNormalize(f);
							VectorMA(t_start, self->teammaster->base_radius, f, t_start);
							tr = gi.trace(t_start, vec3_origin, vec3_origin, target, self, MASK_SHOT);
							if (tr.ent == self->enemy)
							{
								VectorSubtract(target, self->s.origin, dir);
								best_dist = VectorLength(dir) - 100.0;
							}
							else
								self->enemy = NULL;
						}
					}
					else
						self->enemy = NULL;
				}
				else
					self->enemy = NULL;
			}
			else
				self->enemy = NULL;
		}

		// for GOODGUY weapon-firing turrets, if current enemy is a player or GOODGUY monster,
		// reset best_dist so that bad monsters will be selected if found, regardless of distance.
		if ((self->enemy) && (self->sounds >= 0) && (self->spawnflags & SF_TURRET_GOODGUY))
		{
			if (self->enemy->client || (self->enemy->monsterinfo.aiflags & AI_GOOD_GUY))
				best_dist = WORLD_SIZE;	// was 8192.0
		}
			
		// for weapon-firing turrets, if GOODGUY is set and we already have an enemy, we're done.
		if ((self->sounds >= 0) && (self->spawnflags & SF_TURRET_GOODGUY) && self->enemy)
			goto good_enemy;

		// for non-GOODGUY weapon-firing turrets, reset best_dist so that players will
		// ALWAYS be selected if found
		if ((self->sounds >= 0) && !(self->spawnflags & SF_TURRET_GOODGUY))
			best_dist = WORLD_SIZE;	// was 8192.0

		// hunt for closest player - hunt ALL entities since we want to view fake players using camera
		for (i = 1; i < globals.num_edicts; i++)
		{
			gomer = g_edicts + i;
			if (!gomer->inuse)
				continue;
			if (!gomer->client)
				continue;
			if (gomer->svflags & SVF_NOCLIENT)
				continue;
			if (gomer->health < gomer->gib_health)
				continue;
			if (gomer->flags & FL_NOTARGET)
				continue;
			if (!gi.inPVS(self->s.origin, gomer->s.origin))
				continue;

//CW++
//			Don't fire at team-mates.

			if ((sv_gametype->value == G_CTF) || (sv_gametype->value == G_TDM))
			{
				if ((self->spawnflags & SF_TURRET_TEAM1) && (gomer->client->resp.ctf_team == CTF_TEAM1))
					continue;

				if ((self->spawnflags & SF_TURRET_TEAM2) && (gomer->client->resp.ctf_team == CTF_TEAM2))
					continue;
			}
			else if (sv_gametype->value == G_ASLT)
			{
				if (self->spawnflags & SF_TURRET_TEAM1)
				{
					if (gomer->client->resp.ctf_team == asltgame.t_attack)
						continue;
				}

				if (self->spawnflags & SF_TURRET_TEAM2)
				{
					if (gomer->client->resp.ctf_team != asltgame.t_attack)
						continue;
				}
			}
//CW--

			VectorMA(gomer->absmin, 0.5, gomer->size, target);
			VectorCopy(self->s.origin, t_start);
			VectorSubtract(target, self->s.origin, dir);
			VectorCopy(dir, f);
			VectorNormalize(f);
			VectorMA(t_start, self->teammaster->base_radius, f, t_start);
			tr = gi.trace(t_start, vec3_origin, vec3_origin, target, self, MASK_SHOT);
			if (tr.ent == gomer)
			{
				vectoangles(dir, angles);
				AnglesNormalize(angles);
				if (yaw_restrict)
				{
					yaw_0 = angles[YAW] - self->pos1[YAW];
					if (yaw_0 < 0.0)
						yaw_0 += 360.0;
				}
				if ((angles[PITCH] <= self->pos1[PITCH]) && (angles[PITCH] >= self->pos2[PITCH]) &&
					(!yaw_restrict || (yaw_0 <= yaw_r)))
				{
					dist = VectorLength(dir);
					if (dist < best_dist)
					{
						self->enemy = gomer;
						best_dist = dist;
					}
				}
			}
		}

good_enemy:
		if (self->enemy)
		{
			if (self->enemy != self->oldenemy)
			{
				self->monsterinfo.trail_time = level.time;
				self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
			}

			VectorCopy(self->enemy->s.origin, target);
			if (self->enemy->deadflag)
				target[2] -= 16.0;

			VectorSubtract(target, self->s.origin, dir);
			VectorNormalize(dir);
			vectoangles(dir, self->move_angles);

			// decide if we should shoot
			victim = NULL;
			if (self->spawnflags & SF_TURRET_GOODGUY)
			{
				if ((self->enemy->svflags & SVF_MONSTER) && !(self->enemy->monsterinfo.aiflags & AI_GOOD_GUY))
					victim = self->enemy;
			}
			else
			{
				if (self->enemy->client)
					victim = self->enemy;
			}

			if (victim && (self->sounds >= 0) && (DotProduct(forward,dir) > 0.99))
			{
				// never automatically fire a turret remotely controlled by a player
				if (!self->viewer || (self->viewer && !self->viewer->client))
				{
//CW++
					if (level.time < self->monsterinfo.trail_time)
//CW--
					{
						turret_turn(self);
						return;
					}

					self->monsterinfo.attack_finished = level.time;									//CW
					if (self->sounds != 3)															//CW
						self->monsterinfo.attack_finished += self->wait;
//CW++
					else
						self->monsterinfo.attack_finished += FRAMETIME;
//CW--
					turret_breach_fire(self);
					turret_turn(self);
				}
			}
			else
				turret_turn(self);
		}
	}
	// If turret has no enemy and isn't controlled by a player or monster, check for "followtarget"
	if ((!self->enemy) && ((!self->owner) || remote_monster))
	{
		if (self->followtarget)
		{
			self->enemy = G_Find(NULL, FOFS(targetname), self->followtarget);
			if (self->enemy)
			{
				VectorMA(self->enemy->absmin, 0.5, self->enemy->size, target);
				VectorSubtract(target, self->s.origin, dir);
				vectoangles(dir, self->move_angles);
				turret_turn(self);
			}
		}
	}
}

void turret_breach_finish_init(edict_t *self)
{
	// get and save info for muzzle location
	if (!self->target)
		gi.dprintf("%s at %s needs a target\n", self->classname, vtos(self->s.origin));
	else
	{
		self->target_ent = G_PickTarget(self->target);
		if (!self->target_ent)
		{
			gi.dprintf("%s at %s, target %s does not exist\n", self->classname, vtos(self->s.origin), self->target);
			G_FreeEdict(self);
			return;
		}
		VectorSubtract(self->target_ent->s.origin, self->s.origin, self->move_origin);
		G_FreeEdict(self->target_ent);
	}

	if (!self->team)
		self->teammaster = self;

	self->teammaster->dmg = self->dmg;

	if (!(self->spawnflags & (SF_TURRET_TRIGGER_SPAWN | SF_TURRET_GOODGUY | SF_TURRET_INACTIVE)))
	{
		self->think = turret_breach_think;
		self->think(self);
	}
	else
	{
		self->think = NULL;
		self->nextthink = 0.0;
	}
}

void turret_die_temp_think(edict_t *self)
{
	edict_t	*target;

	target = G_Find(NULL, FOFS(targetname), self->destroytarget);
	while (target)
	{
		if (target && target->use)
			target->use(target, self->target_ent, self->target_ent);

		target = G_Find(target, FOFS(targetname), self->destroytarget);
	}

	G_FreeEdict(self);
}

void turret_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t	*ent;
	edict_t	*player;
	int		i;

	// ensure turret_base stops rotating
	if (self->team)
	{
		for (ent = self->teammaster; ent; ent = ent->teamchain)
		{
			if (ent != self)
			{
				ent->avelocity[1] = 0.0;
				gi.linkentity(ent);
			}
		}
	}

	if (self->deadflag != DEAD_DEAD)
	{
		// if turret has a driver, kill him too unless he's a "remote" driver
		if (self->owner && (self->owner->target_ent == self))
			T_Damage(self->owner, inflictor, attacker, vec3_origin, self->owner->s.origin, vec3_origin, 100000, 1, 0, 0);

		// if turret is being used as a camera by a player, turn camera off for that player
		for (i = 0, player = g_edicts+1; i < (int)maxclients->value; i++, player++)
		{
			if (player->client && (player->client->spycam == self))
				camera_off(player);
		}
		if (self->deathtarget)
		{
			edict_t	*target;
			target = G_Find(NULL, FOFS(targetname), self->deathtarget);
			while (target)
			{
				if (target && target->use)
					target->use(target, attacker, attacker);
				target = G_Find(target, FOFS(targetname), self->deathtarget);
			}
		}
	}

	if (self->health <= self->gib_health)
	{
		if (self->destroytarget)
		{
			if (self->deadflag == DEAD_DEAD)
			{
				// we were already dead, so deathtarget has been fired
				edict_t	*target;
				target = G_Find(NULL, FOFS(targetname), self->destroytarget);
				while (target)
				{
					if (target && target->use)
						target->use(target,attacker,attacker);

					target = G_Find(target, FOFS(targetname), self->destroytarget);
				}
			}
			else
			{
				// we were killed and gibbed in the same frame. postpone destroytarget just a bit
				edict_t *temp;
				temp = G_Spawn();
				temp->solid = SOLID_NOT;
				temp->svflags = SVF_NOCLIENT;
				temp->think = turret_die_temp_think;
				temp->nextthink = level.time + (2.0 * FRAMETIME);
				temp->destroytarget = self->destroytarget;
				temp->target_ent = attacker;
//CW++
				temp->classname = "temp_turret_die";
//CW--
				gi.linkentity(temp);
			}
			self->nextthink = 0.0;
			gi.linkentity(self);
		}
		if (self->dmg > 0)
			BecomeExplosion1(self);
		else
			G_FreeEdict(self);

		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	// slow turret down and level it... or for MD2 turrets set to minpitch
	self->speed *= 0.25;																			//CW
	if (self->spawnflags & SF_TURRET_MD2)
		self->move_angles[0] = self->pos1[0];
	else
		self->move_angles[0] = 0.0;
}

void toggle_turret_breach(edict_t *self, edict_t *other, edict_t *activator)
{
	if (!(self->spawnflags & SF_TURRET_INACTIVE))
	{
		self->spawnflags |= SF_TURRET_INACTIVE;
		VectorCopy(self->s.angles, self->move_angles);
		if (self->team)
		{
			edict_t	*ent;
			for (ent = self->teammaster; ent; ent = ent->teamchain)
			{
				VectorClear(ent->avelocity);
				gi.linkentity(ent);
			}
		}
		self->think = NULL;
		self->nextthink = 0.0;
	}
	else
	{
		self->spawnflags &= ~SF_TURRET_INACTIVE;
		self->think = turret_breach_think;
		self->nextthink = level.time + FRAMETIME;
	}
}

void use_turret_breach(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & SF_TURRET_TRIGGER_SPAWN)
	{
		self->spawnflags &= ~SF_TURRET_TRIGGER_SPAWN;
		self->svflags &= ~SVF_NOCLIENT;

		if(self->spawnflags & SF_TURRET_MD2)
			self->solid = SOLID_BBOX;
		else
			self->solid = SOLID_BSP;
		
		self->think = turret_breach_think;
		self->think(self);
	}
}
void turret_breach_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// This added for Lazarus to help prevent player from becoming stuck when 
	// jumping onto a TRACK turret.

	// We only care about TRACK turrets. For monster controlled turrets the angles
	// should of course be controlled by the monster.
	if (!(self->spawnflags & SF_TURRET_TRACKING))
		return;

	// We only care about players... everybody else knows better than to
	// get tangled up with turret :-)
	if (!other->client)
		return;

	// Do nothing for turrets that already have an enemy
	if (self->enemy)
		return;

	if ((other->client) && (other->absmin[2] > self->s.origin[2]))
	{
		if (fabs(self->s.angles[PITCH] - self->pos1[PITCH]) < fabs(self->s.angles[PITCH] - self->pos2[PITCH]))
			self->move_angles[PITCH] = self->pos2[PITCH];
		else
			self->move_angles[PITCH] = self->pos1[PITCH];
		
		if (fabs(self->s.angles[YAW] - self->pos1[YAW]) < fabs(self->s.angles[YAW] - self->pos2[YAW]))
			self->move_angles[YAW] = self->pos2[YAW];
		else
			self->move_angles[YAW] = self->pos1[YAW];
	}
}

void SP_turret_breach(edict_t *self)
{
	// Good guy turrets shoot at monsters, not players. Turn TRACK on if it ain't already.
	if (self->spawnflags & SF_TURRET_GOODGUY)
		self->spawnflags |= (SF_TURRET_TRACKING | SF_TURRET_INACTIVE);

	if (self->spawnflags & SF_TURRET_MD2)
	{
		char modelname[256];
		if (!self->usermodel)
		{
			gi.dprintf("%s w/o a model and MD2 spawnflag set at %s\n", self->classname, vtos(self->s.origin));
			G_FreeEdict(self);
			return;
		}
		Com_sprintf(modelname, sizeof(modelname), "models/%s", self->usermodel);
		self->s.modelindex = gi.modelindex (modelname);

		if ((VectorLength(self->bleft) == 0.0) && (VectorLength(self->tright) == 0.0))
		{
			VectorSet(self->bleft, -16.0, -16.0, -16.0);
			VectorSet(self->tright, 16.0, 16.0, 16.0);
		}
		VectorCopy(self->bleft, self->mins);
		VectorCopy(self->tright, self->maxs);

		if (self->spawnflags & SF_TURRET_TRIGGER_SPAWN)
		{
			self->svflags |= SVF_NOCLIENT;
			self->solid = SOLID_NOT;
			self->use = use_turret_breach;
		}
		else
		{
			self->solid = SOLID_BBOX;
			if (self->spawnflags & SF_TURRET_TRACKING)
				self->use = toggle_turret_breach;
		}
	}
	else
	{
		if (self->spawnflags & SF_TURRET_TRIGGER_SPAWN)
		{
			self->svflags |= SVF_NOCLIENT;
			self->solid = SOLID_NOT;
			self->use = use_turret_breach;
		}
		else
		{
			self->solid = SOLID_BSP;
			if (self->spawnflags & SF_TURRET_TRACKING)
				self->use = toggle_turret_breach;
		}
		gi.setmodel(self, self->model);
	}
	self->movetype = MOVETYPE_PUSH;

	if (!self->speed)
		self->speed = 50.0;

	if (!self->dmg)
		self->dmg = 10;

	if (!st.minpitch)
		st.minpitch = -30.0;
	if (!st.maxpitch)
		st.maxpitch = 30.0;
	if (!st.maxyaw)
		st.maxyaw = 360.0;

	if (!self->wait)
		self->wait = 2.0;

	if (self->health)
	{
		self->die = turret_die;
		self->takedamage = DAMAGE_YES;
	}
	else
	{
		self->die = NULL;
		self->takedamage = DAMAGE_NO;
	}

//CW++
//	Use the 'mass' field to specify damage done by turret's shot; if zero, use the 
//	default server setting for that weapon type.

	if (self->mass < 1)
	{
		switch (self->sounds)
		{
			case 1: // Rocket Launcher
				self->mass = (int)sv_rocket_damage->value;
				break;

			case 2: // Railgun
				self->mass = (int)sv_railgun_damage->value;
				break;

			case 3: // Machinegun
				if (self->wait < 1.0)								// take old-style turrets into account
					self->mass = (int)sv_mac10_damage->value;
				else
					self->mass = (int)self->wait;
				break;

			case 4: // E.S.G.
				self->mass = (int)sv_spike_damage->value;
				break;

			case 5: // Shock Rifle
				self->mass = (int)sv_shock_radius_damage->value;
				break;

			case 6:	// Plasma
				self->mass = (int)sv_plasma_damage->value;
				break;

			default: // Rocket Launcher
				self->mass = (int)sv_rocket_damage->value;
				break;
		}
	}

//	Avert your eyes - it's ugly hack time!

	if (!Q_stricmp(level.mapname, "awaken2_assault2") && self->targetname)
	{
		if (!Q_stricmp(self->targetname, "turret111"))
		{
			self->viewmessage = "[Turret #1]:\n\nMachinegun";
			self->s.angles[YAW] = 0.0;
			st.minpitch = -35.0;
		}
		else if (!Q_stricmp(self->targetname, "turret1"))
		{
			self->viewmessage = "[Turret #2]:\n\nShockbolt";
			st.minpitch = -35.0;
			self->wait = 1.0;
		}
	}
//CW--

	// Added touch routine to help prevent player from getting stuck after jumping on turret barrel
	self->touch = turret_breach_touch;

	self->pos1[PITCH] = -1.0 * st.minpitch;
	self->pos1[YAW]   = st.minyaw;
	self->pos2[PITCH] = -1.0 * st.maxpitch;
	self->pos2[YAW]   = st.maxyaw;

	if (self->pos1[YAW] < 0.0)
		self->pos1[YAW] += 360.0;
	if (self->pos2[YAW] < 0.0)
		self->pos2[YAW] += 360.0;

	self->ideal_yaw = self->s.angles[YAW];
	self->move_angles[YAW] = self->ideal_yaw;

	self->blocked = turret_blocked;

	self->think = turret_breach_finish_init;
	self->nextthink = level.time + FRAMETIME;

//CW++
	self->delay = level.time;

	if (!self->count)
		self->count = 1;
//CW--

	gi.linkentity(self);
}

void SP_model_turret(edict_t *self)
{
	self->spawnflags |= SF_TURRET_MD2;
	SP_turret_breach(self);
}

/*QUAKED turret_base (0 0 0) ?
This portion of the turret changes yaw only. MUST be teamed with a turret_breach.
*/
void turret_base_finish(edict_t *self)
{
	vec_t	radius;

	if (self->team)
	{
		// should ALWAYS have a team, but we're being pessimistic here
		radius = (self->maxs[0] - self->mins[0])*(self->maxs[0] - self->mins[0]) +
			     (self->maxs[1] - self->mins[1])*(self->maxs[1] - self->mins[1]) +
				 (self->maxs[2] - self->mins[2])*(self->maxs[2] - self->mins[2]);
		self->teammaster->base_radius = sqrt(radius);
	}
}

void use_turret_base(edict_t *self, edict_t *other, edict_t *activator)
{
	self->svflags &= ~SVF_NOCLIENT;
	self->solid = SOLID_BSP;
}

void SP_turret_base(edict_t *self)
{
	if (self->spawnflags & SF_TURRET_TRIGGER_SPAWN)
	{
		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->use = use_turret_base;
	}
	else
		self->solid = SOLID_BSP;

	self->movetype = MOVETYPE_PUSH;
	gi.setmodel(self, self->model);
	self->blocked = turret_blocked;

	// DWH
	self->s.angles[PITCH] = self->s.angles[ROLL] = 0.0;
	self->think = turret_base_finish;
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity(self);
}
