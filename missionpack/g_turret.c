// g_turret.c

#include "g_local.h"

#define SF_TURRETDRIVER_REMOTE_DRIVER 1
#define SF_TURRET_PLAYER_CONTROLLABLE 1
#define SF_TURRET_TRIGGER_SPAWN       2
#define SF_TURRET_TRACKING            4
#define SF_TURRET_GOODGUY             8
#define SF_TURRET_INACTIVE           16
#define SF_TURRET_MD2                32

// Added Musashi's double barreled turret code
//CW++
//CW--

#define TURRET_GRENADE_SPEED 800

void SpawnTargetingSystem (edict_t *turret);	// PGM

// Lazarus - Added TurretTarget to scan the turret driver's view for a
// damageable target. Used with homing rockets
edict_t	*TurretTarget(edict_t *self)
{
	float       bd, d;
	int			i;
	edict_t	    *who, *best;
	trace_t     tr;
	vec3_t      dir, end, forward, right, up, start;

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
	for (i=1, who=g_edicts+1; i<globals.num_edicts; i++, who++)
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
		else if (self->owner)
		{
			attacker = self->owner;
		}
		else
		{
			attacker = self;
		}
		// give a big kickback to help prevent getting stuck
		T_Damage (other, self, attacker, dir, other->s.origin, vec3_origin, master->dmg, 50, 0, MOD_CRUSH);
		//T_Damage (other, self, attacker, vec3_origin, other->s.origin, vec3_origin, self->teammaster->dmg, 10, 0, MOD_CRUSH);
	}
	if (!(other->svflags & SVF_MONSTER) && (!other->client))
	{
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		if (other)
			BecomeExplosion1 (other);
		return;
	}

}

/*QUAKED turret_breach (0 0 0) ? OCCUPIABLE TRIGGER_SPAWN TRACK
This portion of the turret can change both pitch and yaw.
The model  should be made with a flat pitch.
It (and the associated base) need to be oriented towards 0.
Use "angle" to set the starting angle.

OCCUPIABLE - player can jump on this turret and use it
TRIGGER_SPAWN- Spawns upon being triggered
TRACK-		It will attack the player without having a driver.

"speed"		default 50
"dmg"		default 10
"distance"	initial velocity of fired grenades in units/sec
"angle"		point this forward
"target"	point this at an info_notnull at the muzzle tip
"minpitch"	min acceptable pitch angle : default -30
"maxpitch"	max acceptable pitch angle : default 30
"minyaw"	min acceptable yaw angle   : default 0
"maxyaw"	max acceptable yaw angle   : default 360
"target"	the info_notnull (rocket origin)
"combattarget" the 2nd info_notnull for a 2nd barrel
"style"		set to 1 to enable alternate firing of dual barrels
"targetname"	name of this breach
"team"		give this the same value as the turret_base "team" key
"health"	if set, it explodes like a func_explosive- default to 100 health, 75 mass, etc
			It will not destroy the turret base.

"sounds"	- weapon to use -
		1) Railgun
		2) Rocket Launcher
		3) BFG
		4) Homing Rockets
		5) Machine Gun
		6) Hyperblaster
		7) Grenade
"effects"	Sets hyperblaster color. 0 = Orange, 1 = Blue, 2 = Green, 3 = Red
"wait" The meaning of "wait" is dependent on the weapon type (sounds) used. Firing rate: When sounds=1, 2, 3, 4, or 7 (RG, RL, BFG, HomingRL, GL), "wait" sets the number of seconds to wait between weapon firing events. Note: An additional skill-level dependent "reaction time" delay is added to this, as described by the formula:

   [reaction time]=[2-skill]/2

So if a RL turret uses wait=1, on normal skill the actual time between firings would be 1.5 seconds. Damage level: When sounds=5 or 6 (MG or HB), "wait" sets the amount of damage points each shot will do (firing rate is fixed at 10 shots/second). Ignored when sounds=-1. Default=2.
*/

//CW++
void hrocket_turret_fire (edict_t *self, edict_t *owner, vec3_t start, vec3_t dir, vec3_t start2, vec3_t dir2, int damage, int speed, float damage_radius, int radius_damage, edict_t *home_target)
{
	if (self->moreflags & FL2_TURRET_DOUBLE)
	{
		if (self->moreflags & FL2_TURRET_DOUBLE_ALT)
		{
			if (self->moreflags & FL2_TURRET_DOUBLE_ALT_FIRING)
			{
				fire_rocket (owner, start2, dir2, damage, speed, damage_radius, radius_damage, home_target);
				self->moreflags &= ~FL2_TURRET_DOUBLE_ALT_FIRING;
			}
			else
			{
				fire_rocket (owner, start, dir, damage, speed, damage_radius, radius_damage, home_target);
				self->moreflags |= FL2_TURRET_DOUBLE_ALT_FIRING;
			}
		}
		else
		{
			fire_rocket (owner, start, dir, damage, speed, damage_radius, radius_damage, home_target);
			fire_rocket (owner, start2, dir2, damage, speed, damage_radius, radius_damage, home_target);
		}
	}
	else
		fire_rocket (owner, start, dir, damage, speed, 150, damage, home_target);
}
//CW--

static unsigned int /*long*/ HB_Shots;

void turret_breach_fire (edict_t *self)
{
	edict_t	*owner;
	vec3_t	forward, right, up;
	vec3_t	start;
	int		damage, speed, radius;
//CW++
	vec3_t	forward2, right2, up2;
	vec3_t	start2;
//CW--

	AngleVectors (self->s.angles, forward, right, up);
	VectorMA (self->s.origin, self->move_origin[0], forward, start);
	VectorMA (start, self->move_origin[1], right, start);
	VectorMA (start, self->move_origin[2], up, start);

//CW++
	if (self->moreflags & FL2_TURRET_DOUBLE)
	{ 
		AngleVectors (self->s.angles, forward2, right2, up2);
		VectorMA (self->s.origin, self->muzzle2[0], forward2, start2);
		VectorMA (start2, self->muzzle2[1], right2, start2);
		VectorMA (start2, self->muzzle2[2], up2, start2);
	}
//CW--

	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	damage = 100 + random() * 50;
	speed = 550 + 50 * skill->value;

	// Lazarus: automated turrets have no driver, so use self
	if (self->owner && !(self->owner->spawnflags & SF_TURRETDRIVER_REMOTE_DRIVER))
		owner = self->owner;
	else
		owner = self;

//	fire_rocket (self->teammaster->owner, start, f, damage, speed, 150, damage);
//	gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("weapons/rocklf1a.wav"), 1, ATTN_NORM, 0);
	/*
	Mappaack - self->teammaster->owner causes quake 2 to crash when the player uses BUTTON_SHOOT
	     its been changed to self->owner incase anything weird happens.
	*/

	// FIXME : only use the normal damages if self->owner (turret_driver) doesn't have one

	if (self->delay < level.time)
	{
		switch (self->sounds)
		{
			case 1: // railgun
			{
				damage = 150;
//CW++
				if (self->moreflags & FL2_TURRET_DOUBLE)
				{
					if (self->moreflags & FL2_TURRET_DOUBLE_ALT)
					{
						if (self->moreflags & FL2_TURRET_DOUBLE_ALT_FIRING)
						{
							fire_rail(owner, start2, forward2, damage, 0);
							self->moreflags &= ~FL2_TURRET_DOUBLE_ALT_FIRING;
						}
						else
						{
							fire_rail(owner, start, forward, damage, 0);
							self->moreflags |= FL2_TURRET_DOUBLE_ALT_FIRING;
						}
					}
					else
					{
						fire_rail(owner, start, forward, damage, 0);
						fire_rail(owner, start2, forward2, damage, 0);
					}
				}
				else
//CW--
					fire_rail (owner, start, forward, damage, 0);
				gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("weapons/railgf1a.wav"), 1, ATTN_NORM, 0);
				// was level.time + 1.3
				self->delay = level.time + max((self->wait + ((2 - skill->value) / 2)), self->wait); 

				// Mappack - Muzzleflash?  On a turret?  Yeah baby!
				gi.WriteByte (svc_muzzleflash);
				gi.WriteShort (self-g_edicts);
				gi.WriteByte (MZ_RAILGUN);
				gi.multicast (start, MULTICAST_PVS);
				break;
			}
			case 2: // rocket
			{
				damage = 100 + random() * 50;
//CW++
				if (self->moreflags & FL2_TURRET_DOUBLE)
				{
					if (self->moreflags & FL2_TURRET_DOUBLE_ALT)
					{
						if (self->moreflags & FL2_TURRET_DOUBLE_ALT_FIRING)
						{
							fire_rocket(owner, start2, forward2, damage, speed, 150, damage, NULL);
							self->moreflags &= ~FL2_TURRET_DOUBLE_ALT_FIRING;
						}
						else
						{
							fire_rocket(owner, start, forward, damage, speed, 150, damage, NULL);
							self->moreflags |= FL2_TURRET_DOUBLE_ALT_FIRING;
						}
					}
					else
					{
						fire_rocket(owner, start, forward, damage, speed, 150, damage, NULL);
						fire_rocket(owner, start2, forward2, damage, speed, 150, damage, NULL);
					}
				}
				else
//CW--
					fire_rocket (owner, start, forward, damage, speed, 150, damage, NULL);
				gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("weapons/rocklf1a.wav"), 1, ATTN_NORM, 0);
				// was level.time + 0.45
				self->delay = level.time + max((self->wait + ((2 - skill->value) / 2)), self->wait); 

				break;
			}
			case 3: // BFG
			{
				damage = 500;
//CW++
				if (self->moreflags & FL2_TURRET_DOUBLE)
				{
					if (self->moreflags & FL2_TURRET_DOUBLE_ALT)
					{
						if (self->moreflags & FL2_TURRET_DOUBLE_ALT_FIRING)
						{
							fire_bfg(owner, start2, forward2, damage, speed, 1000);
							self->moreflags &= ~FL2_TURRET_DOUBLE_ALT_FIRING;
						}
						else
						{
							fire_bfg(owner, start, forward, damage, speed, 1000);
							self->moreflags |= FL2_TURRET_DOUBLE_ALT_FIRING;
						}
					}
					else
					{
						fire_bfg(owner, start, forward, damage, speed, 1000);
						fire_bfg(owner, start2, forward2, damage, speed, 1000);
					}
				}
				else
//CW--
					fire_bfg (owner, start, forward, damage, speed, 1000);
				gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("makron/bfg_fire.wav"), 1, ATTN_NORM, 0);
				// was level.time + 3;
				self->delay = level.time + max((self->wait + ((2 - skill->value) / 2)), self->wait); 
				break;
			}
			case 4: // Homing rocket
			{
				damage = 100 + random() * 50;
				if (owner->target_ent == self || owner == self)
				{
					// monster-controlled or automated turret
				//	fire_rocket (owner, start, forward, damage, speed, 150, damage, owner->enemy);
					hrocket_turret_fire (self, owner, start, forward, start2, forward2, damage, speed, 150, damage, owner->enemy);	//CW
				}
				else if (self->spawnflags & SF_TURRET_PLAYER_CONTROLLABLE
					|| allow_player_use_abandoned_turret->value)
				{
					// what is player aiming at?
					edict_t *target;
					target = TurretTarget(self);
				//	fire_rocket (owner, start, forward, damage, speed, 150, damage, target);
					hrocket_turret_fire (self, owner, start, forward, start2, forward2, damage, speed, 150, damage, target);		//CW
				}
				else
				{
					// shouldn't be possible to get here
				//	fire_rocket (owner, start, forward, damage, speed, 150, damage, NULL);
					hrocket_turret_fire (self, owner, start, forward, start2, forward2, damage, speed, 150, damage, NULL);			//CW
				}
				gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("weapons/rocklf1a.wav"), 1, ATTN_NORM, 0);
				// was level.time + 0.45
				self->delay = level.time + max((self->wait + ((2 - skill->value) / 2)), self->wait); 
				break;
			}
			case 5: // Machine Gun
			{
				if (!self->wait)
					damage = 4;
				else
					damage = self->wait;
//CW++
				if (self->moreflags & FL2_TURRET_DOUBLE)
				{
					if (self->moreflags & FL2_TURRET_DOUBLE_ALT)
					{
						if (self->moreflags & FL2_TURRET_DOUBLE_ALT_FIRING)
						{
							fire_bullet(owner, start2, forward2, damage, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
							self->moreflags &= ~FL2_TURRET_DOUBLE_ALT_FIRING;
						}
						else
						{
							fire_bullet(owner, start, forward, damage, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
							self->moreflags |= FL2_TURRET_DOUBLE_ALT_FIRING;
						}
					}
					else
					{
						fire_bullet(owner, start, forward, damage, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
						fire_bullet(owner, start2, forward2, damage, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
					}
				}
				else
//CW--
					fire_bullet (owner, start, forward, damage, 2, 100, 100, MOD_MACHINEGUN);
			//	gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("infantry/infatck1.wav"), 1, ATTN_NORM, 0);
				self->delay = level.time; // no delay

				gi.WriteByte (svc_muzzleflash);
				gi.WriteShort (self-g_edicts);
				gi.WriteByte (MZ_MACHINEGUN);
				gi.multicast (start, MULTICAST_PVS);
				break;
			}
			case 6: // Hyperblaster
			{
				unsigned int	effect, color;
				if (!self->wait)
					damage = 5;
				else
					damage = self->wait;

				if (self->effects == 1)	// Blue
				{	effect = EF_BLUEHYPERBLASTER; color = BLASTER_BLUE;	}
				else if (self->effects == 2)	// Green
				{	effect = EF_HYPERBLASTER|EF_TRACKER; color = BLASTER_GREEN;	}
#ifdef KMQUAKE2_ENGINE_MOD
				else if (self->effects == 3)	// Red
				{	effect = EF_HYPERBLASTER|EF_IONRIPPER; color = BLASTER_RED;	}
#endif	// KMQUAKE2_ENGINE_MOD
				else // (self->effects == 0) // Orange
				{	effect = EF_HYPERBLASTER; color = BLASTER_ORANGE;	}

				HB_Shots++;
//CW++
				if (self->moreflags & FL2_TURRET_DOUBLE)
				{
					if (self->moreflags & FL2_TURRET_DOUBLE_ALT)
					{
						if (self->moreflags & FL2_TURRET_DOUBLE_ALT_FIRING)
						{
							fire_blaster (owner, start2, forward2, self->wait, 1000, (!(HB_Shots % 4))?effect:0, true, color);
							self->moreflags &= ~FL2_TURRET_DOUBLE_ALT_FIRING;
						}
						else
						{
							fire_blaster (owner, start, forward, self->wait, 1000, (!(HB_Shots % 4))?effect:0, true, color);
							self->moreflags |= FL2_TURRET_DOUBLE_ALT_FIRING;
						}
					}
					else
					{
						fire_blaster (owner, start, forward, self->wait, 1000, (!(HB_Shots % 4))?effect:0, true, color);
						fire_blaster (owner, start2, forward2, self->wait, 1000, 0, true, color);
					}
				}
				else
//CW--
					fire_blaster (owner, start, forward, damage, 1000, (!(HB_Shots % 4))?effect:0, true, color);
			//	fire_blaster (owner, start, forward, damage, 1000, EF_HYPERBLASTER, true, BLASTER_ORANGE);
				gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("makron/blaster.wav"), 1, ATTN_NORM, 0);
				self->delay = level.time; // no delay
				break;
			}
			case 7: // Grenade
			{
				damage = 100 + random() * 50;
				if (self->distance > 0)
					speed = self->distance;
				radius = 90;
//CW++
				if (self->moreflags & FL2_TURRET_DOUBLE)
				{
					if (self->moreflags & FL2_TURRET_DOUBLE_ALT)
					{
						if (self->moreflags & FL2_TURRET_DOUBLE_ALT_FIRING)
						{
							fire_grenade (owner, start2, forward2, damage, speed, 2.5, radius, false);
							self->moreflags &= ~FL2_TURRET_DOUBLE_ALT_FIRING;
						}
						else
						{
							fire_grenade (owner, start, forward, damage, speed, 2.5, radius, false);
							self->moreflags |= FL2_TURRET_DOUBLE_ALT_FIRING;
						}
					}
					else
					{
						fire_grenade (owner, start, forward, damage, speed, 2.5, radius, false);
						fire_grenade (owner, start2, forward2, damage, speed, 2.5, radius, false);
					}
				}
				else
//CW--
					fire_grenade (owner, start, forward, damage, speed, 2.5, radius, false);
				// was level.time + 0.45;
				self->delay = level.time + max((self->wait + ((2 - skill->value) / 2)), self->wait);

				gi.WriteByte (svc_muzzleflash2);
				gi.WriteShort (self - g_edicts);
				gi.WriteByte (MZ2_GUNNER_GRENADE_1);
				gi.multicast (start, MULTICAST_PVS);
				break;
			}
			/*case 8: // Blue Hyperblaster
			{
				if (!self->wait)
					damage = 5;
				else
					damage = self->wait;
			//	fire_blaster (owner, start, forward, damage, 1000, EF_BLUEHYPERBLASTER, true, BLASTER_BLUE);
				fire_blueblaster (owner, start, forward, damage, 1000, EF_BLUEHYPERBLASTER);
				gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("makron/blaster.wav"), 1, ATTN_NORM, 0);
				self->delay = level.time; // no delay
				break;
			}
			case 9: // Green Hyperblaster
			{
				if (!self->wait)
					damage =  5;
				else
					damage = self->wait;
			//	fire_blaster (owner, start, forward, damage, 1000, EF_HYPERBLASTER|EF_TRACKER, true, BLASTER_GREEN);
				fire_blaster2 (owner, start, forward, damage, 1000, EF_HYPERBLASTER, true);
				gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("makron/blaster.wav"), 1, ATTN_NORM, 0);
				self->delay = level.time; // no delay
				break;
			}*/
			default:
			{
				damage = 100;
//CW++
				if (self->moreflags & FL2_TURRET_DOUBLE)
				{
					if (self->moreflags & FL2_TURRET_DOUBLE_ALT)
					{
						if (self->moreflags & FL2_TURRET_DOUBLE_ALT_FIRING)
						{
							fire_rocket(owner, start2, forward2, damage, speed, 150, damage, NULL);
							self->moreflags &= ~FL2_TURRET_DOUBLE_ALT_FIRING;
						}
						else
						{
							fire_rocket(owner, start, forward, damage, speed, 150, damage, NULL);
							self->moreflags |= FL2_TURRET_DOUBLE_ALT_FIRING;
						}
					}
					else
					{
						fire_rocket(owner, start, forward, damage, speed, 150, damage, NULL);
						fire_rocket(owner, start2, forward2, damage, speed, 150, damage, NULL);
					}
				}
				else
//CW--
					fire_rocket (owner, start, forward, damage, speed, 150, damage, NULL);
				gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("weapons/rocklf1a.wav"), 1, ATTN_NORM, 0);
				self->delay = level.time + 1;
				break;
			}
		}
	}
}

void turret_disengage (edict_t *self)
{
	int     i;
	edict_t *ent;
	vec3_t  forward;

	// level the gun
	self->move_angles[0] = 0;
				
	ent = self->owner;
				
	//ed - to keep remove tracking of the entity
	ent->turret = NULL;
				
	// throw them back from turret
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorScale(forward, -300, forward);
	forward[2] = forward[2] + 150;
	if (forward[2] < 80)
		forward[2] = 80;
				
	for (i=0; i<3; i++)
		ent->velocity[i] = forward[i];
		
	ent->s.origin[2] = ent->s.origin[2] + 1;
	ent->movetype = MOVETYPE_WALK;
	ent->gravity = 1;
				
	ent->flags &= ~FL_TURRET_OWNER;
				
	// turn ON client side prediction for this player
	ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

	gi.linkentity (ent);
	
	self->owner = NULL;
}

void turret_turn (edict_t *self)
{
	vec3_t		current_angles;
	vec3_t		delta;
	qboolean	yaw_restrict;

	VectorCopy (self->s.angles, current_angles);
	AnglesNormalize(current_angles);

	if (self->viewer && self->viewer->client)
	{
		gclient_t	*client = self->viewer->client;

		if ( (client->old_owner_angles[0] != client->ucmd.angles[0]) ||
			(client->old_owner_angles[1] != client->ucmd.angles[1])   )
		{
			// Give game a bit of time to catch up after player
			// causes ucmd pitch angle to roll over... otherwise
			// we'll hit on the above test even though player
			// hasn't hit +lookup/+lookdown
			float	delta;
			delta = level.time - self->touch_debounce_time;
			if ( delta < 0 || delta > 1.0)
			{
				float	delta_angle;
				float	fastest = self->speed * FRAMETIME;
				
				delta_angle = SHORT2ANGLE(client->ucmd.angles[0]-client->old_owner_angles[0]);
				if (delta_angle < -180)
					delta_angle += 360;
				if (delta_angle > 180)
					delta_angle -= 360;
				if (delta_angle > fastest)
					delta_angle = fastest;
				if (delta_angle < -fastest)
					delta_angle = -fastest;
				self->move_angles[0] += delta_angle;
				
				delta_angle = SHORT2ANGLE(client->ucmd.angles[1]-client->old_owner_angles[1]);
				if (delta_angle < -180)
					delta_angle += 360;
				if (delta_angle > 180)
					delta_angle -= 360;
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

//	if (self->move_angles[PITCH] > 180)
//		self->move_angles[PITCH] -= 360;

	// clamp angles to mins & maxs
	if (self->move_angles[PITCH] > self->pos1[PITCH])
		self->move_angles[PITCH] = self->pos1[PITCH];
	else if (self->move_angles[PITCH] < self->pos2[PITCH])
		self->move_angles[PITCH] = self->pos2[PITCH];

	// Lazarus: Special case - if there are no constraints on YAW, don't
	// adjust angle
	if (self->pos1[YAW] != 0 || self->pos2[YAW] != 360)
		yaw_restrict = true;
	else
		yaw_restrict = false;

	if ( yaw_restrict )
	{
		float	yaw_range;
		float	yaw_base;
		yaw_range = self->pos2[YAW] - self->pos1[YAW];
		if (yaw_range < 0)
			yaw_range += 360;
		yaw_base  = self->move_angles[YAW] - self->pos1[YAW];
		if (yaw_base < 0)
			yaw_base += 360;
		if (yaw_base > yaw_range)
		if ((self->move_angles[YAW] < self->pos1[YAW]) || (self->move_angles[YAW] > self->pos2[YAW]))
		{
			float	dmin, dmax;

			dmin = fabs(self->pos1[YAW] - self->move_angles[YAW]);
			if (dmin < 0 )
				dmin += 360;
			else if (dmin > 360)
				dmin -= 360;
			dmax = fabs(self->pos2[YAW] - self->move_angles[YAW]);
			if (dmax < 0)
				dmax += 360;
			else if (dmax > 360)
				dmax -= 360;
			if (fabs(dmin) < fabs(dmax))
				self->move_angles[YAW] = self->pos1[YAW];
			else
				self->move_angles[YAW] = self->pos2[YAW];
		}
	}

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

//	gi.dprintf("move=%g, current=%g, delta=%g, pos1=%g, pos2=%g\n",
//		self->move_angles[PITCH],current_angles[PITCH],delta[PITCH],self->pos1[PITCH],self->pos2[PITCH]);

/*	if (delta[0] > self->speed * FRAMETIME)
		delta[0] = self->speed * FRAMETIME;
	if (delta[0] < -1 * self->speed * FRAMETIME)
		delta[0] = -1 * self->speed * FRAMETIME;
	if (delta[1] > self->speed * FRAMETIME)
		delta[1] = self->speed * FRAMETIME;
	if (delta[1] < -1 * self->speed * FRAMETIME)
		delta[1] = -1 * self->speed * FRAMETIME;
	VectorScale (delta, 1.0/FRAMETIME, self->avelocity);
*/
	VectorScale (delta, 1.0/FRAMETIME, delta);
	if (delta[0] >  self->speed)
		delta[0] =  self->speed;
	if (delta[0] < -self->speed)
		delta[0] = -self->speed;
	if (delta[1] >  self->speed)
		delta[1] =  self->speed;
	if (delta[1] < -self->speed)
		delta[1] = -self->speed;
	VectorCopy (delta, self->avelocity);

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

qboolean FindTarget (edict_t *self);

void turret_breach_think (edict_t *self)
{
	edict_t		*ent;
	edict_t		*victim;
	trace_t		tr;
	vec3_t		dir, angles;
	vec3_t		target;
	qboolean	remote_monster;
	qboolean	yaw_restrict;
	float		yaw_r, yaw_0;
	int		i;

	turret_turn(self);
	yaw_r = self->pos2[YAW] - self->pos1[YAW];
	if (yaw_r < 0)
		yaw_r += 360;

	if (self->pos1[YAW] != 0 || self->pos2[YAW] != 360)
		yaw_restrict = true;
	else
		yaw_restrict = false;

	self->nextthink = level.time + FRAMETIME;

	if (self->deadflag == DEAD_DEAD)
		return;

	if ( (self->owner) && (self->owner->target_ent == self) &&
		 (self->owner->spawnflags & SF_TURRETDRIVER_REMOTE_DRIVER))
		remote_monster = true;
	else
		remote_monster = false;

	// Lazarus: If turret is in the control of somebody/something, 
	//          animate it. "viewer" is a player using the turret 
	//          as a camera.
	if (self->owner || self->viewer)
	{
		if (!(self->spawnflags & SF_TURRET_MD2))
		{
			self->s.effects &= ~EF_ANIM23;
			self->s.effects |= EF_ANIM01;
		}
		self->do_not_rotate = true;
	}
	else
	{
		if (!(self->spawnflags & SF_TURRET_MD2))
		{
			self->s.effects &= ~EF_ANIM01;
			self->s.effects |= EF_ANIM23;
		}
		self->do_not_rotate = false;
	}
	if (self->team)
	{
		for (ent = self->teammaster; ent; ent = ent->teamchain)
		{
			if (ent != self->owner)
			{
				if (ent->solid != SOLID_NOT)
					ent->s.effects = self->s.effects;
				ent->do_not_rotate = self->do_not_rotate;
			}
		}
	}

	// if we have a driver, adjust his velocities
	if (self->owner && !remote_monster)
	{
		if (self->owner->target_ent == self) 
		{
			float	angle;
			float	target_z;
			float	diff;
		//	vec3_t	target;
			vec3_t	dir;

			// angular is easy, just copy ours
			self->owner->avelocity[0] = self->avelocity[0];
			self->owner->avelocity[1] = self->avelocity[1];

			// x & y
			angle = self->s.angles[1] + self->owner->move_origin[1];
			angle *= (M_PI*2 / 360);
			target[0] = SnapToEights(self->s.origin[0] + cos(angle) * self->owner->move_origin[0]);
			target[1] = SnapToEights(self->s.origin[1] + sin(angle) * self->owner->move_origin[0]);
			target[2] = self->owner->s.origin[2];

			VectorSubtract (target, self->owner->s.origin, dir);
			self->owner->velocity[0] = dir[0] * 1.0 / FRAMETIME;
			self->owner->velocity[1] = dir[1] * 1.0 / FRAMETIME;

			// z
			angle = self->s.angles[PITCH] * (M_PI*2 / 360);
			target_z = SnapToEights(self->s.origin[2] + self->owner->move_origin[0] * tan(angle) + self->owner->move_origin[2]);

			diff = target_z - self->owner->s.origin[2];
			self->owner->velocity[2] = diff * 1.0 / FRAMETIME;

			if (self->spawnflags & 65536)
			{
				turret_breach_fire (self);
				self->spawnflags &= ~65536;
			}
			return;
		}
		else if ((self->spawnflags & SF_TURRET_PLAYER_CONTROLLABLE
			|| allow_player_use_abandoned_turret->value))// && !remote_monster)
		{	// a player is controlling the turret, move towards view angles
			vec3_t	target, forward;

			for (i=0; i<3; i++)
				self->move_angles[i] = self->owner->client->v_angle[i];
			AnglesNormalize(self->move_angles);

			// FIXME: do a tracebox from up and behind towards the turret, to try and keep them from
			// getting stuck inside the rotating turret
			// x & y
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorScale(forward, 32, forward);
			VectorSubtract(self->s.origin, forward, target);

			VectorAdd(target, tv(0,0,8), self->owner->s.origin);

			gi.linkentity(self->owner);

			// should the turret shoot now?
			if ((self->owner->client->buttons & BUTTON_ATTACK) && (self->delay < level.time))	// 1 second break between shots
			{
				turret_breach_fire (self);
			//	self->delay = level.time + 1;
			}

			// has the player abondoned the turret?
			// jump button disables turret

		//	if (self->owner->client->ps.pmove.velocity[2] > 150) 
		//		turret_disengage(self);
			else if (self->owner->client->ucmd.upmove >= 20)
				turret_disengage(self);
		}
	}
	// Mappack
	else if ((self->spawnflags & SF_TURRET_PLAYER_CONTROLLABLE
		|| allow_player_use_abandoned_turret->value) && !remote_monster)
	{	// check if a player has mounted the turret
		int		i;
		edict_t	*ent;
		vec3_t	target, forward;
		vec3_t	dir;

		// find a player
		ent = &g_edicts[0];
		ent++;
		for (i=0 ; i < maxclients->value ; i++, ent++)
		{
retry:
			if (!ent->inuse)
				continue;
			if (ent->solid == SOLID_NOT)
				continue;
			// Knightmare- if it's an oldplayer, switch to client entity
			if (ent->svflags & SVF_OLDPLAYER && ent->to && ent->to->client && ent->to->client->oldplayer)
			{
				ent = ent->to;
				goto retry;
			}
			// determine distance from turret seat location

			// x & y
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorScale(forward, 32, forward);
			VectorSubtract(self->s.origin, forward, target);

			VectorSubtract (target, ent->s.origin, dir);
			if (fabs(dir[2]) < 64)
				dir[2] = 0;

			if (VectorLength(dir) < 16)
			{	// player has taken control of turret

				// Knightmare- leave thirdperson mode
				if (ent->client->chaseactive)
					ChasecamRemove (ent);

				self->owner = ent;
				ent->movetype = MOVETYPE_PUSH;	// don't let them move, or they'll get stuck
				//ent->solid = SOLID_NOT; //Knightmare- don't let them get pinched by angled turret
				ent->gravity = 0;

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

	if ((self->spawnflags & SF_TURRET_TRACKING) && ((!self->owner) || remote_monster))
	{
		// Lazarus: Automated turret that is not under the control of anybody
		//          (except possibly a "remote" turret_driver)
		float	reaction_time;
		vec3_t	f, forward, right, up, start, t_start;
		edict_t	*gomer;
		float	best_dist = WORLD_SIZE;	// was 8192
		float	dist;

		if (self->viewer && level.time < self->touch_debounce_time)
			return;

		AngleVectors(self->s.angles, forward, right, up);
		VectorMA(self->s.origin,self->move_origin[0],forward,start);
		VectorMA(start,         self->move_origin[1],right,  start);
		VectorMA(start,         self->move_origin[2],up,     start);

		self->oldenemy = self->enemy;
		if (self->enemy)
		{	// check that current enemy is valid. if so, find
			// distance. don't switch enemies unless another
			// monster is at least 100 units closer to the camera
			if (self->enemy->inuse)
			{
				if ((self->enemy->health > 0 /*self->enemy->gib_health*/) &&
					!(self->enemy->svflags & SVF_NOCLIENT) &&
					!(self->enemy->flags & FL_NOTARGET))
				{
					if (gi.inPVS(self->s.origin,self->enemy->s.origin))
					{
						VectorMA(self->enemy->absmin,0.5,self->enemy->size,target);
						VectorSubtract(target,self->s.origin,dir);
						vectoangles(dir,angles);
						AnglesNormalize(angles);
						if ( yaw_restrict )
						{
							yaw_0  = angles[YAW] - self->pos1[YAW];
							if (yaw_0 < 0)
								yaw_0 += 360;
						}
						if ( (angles[PITCH] > self->pos1[PITCH]) || (angles[PITCH] < self->pos2[PITCH]) ||
							( yaw_restrict && (yaw_0 > yaw_r) )	)
						{
							self->enemy = NULL;
						}
						else
						{
							VectorCopy(self->s.origin,t_start);
							VectorCopy(dir,f);
							VectorNormalize(f);
							VectorMA(t_start,self->teammaster->base_radius,f,t_start);
							tr = gi.trace(t_start,vec3_origin,vec3_origin,target,self,MASK_SHOT);
							if (tr.ent == self->enemy)
							{
								VectorSubtract(target,self->s.origin,dir);
								best_dist = VectorLength(dir) - 100;
							}
							else
								self->enemy = NULL;
						}
					}
					else
					{
						self->enemy = NULL;
					}
				}
				else
					self->enemy = NULL;
			}
			else
				self->enemy = NULL;
		}

		// for GOODGUY weapon-firing turrets, if current enemy is a player or GOODGUY monster,
		// reset best_dist so that bad monsters will be selected if found, regardless of distance.
		if ( (self->enemy) && (self->sounds >= 0) && (self->spawnflags & SF_TURRET_GOODGUY)) {
			if ((self->enemy->client) || (self->enemy->monsterinfo.aiflags & AI_GOOD_GUY))
				best_dist = WORLD_SIZE;	// was 8192
		}

		// hunt for monster
		if (!remote_monster)
		{
			for (i=maxclients->value+1; i<globals.num_edicts; i++)
			{
				gomer = g_edicts + i;
				if (gomer == self->enemy) continue; // no need to re-check this guy
				if (!gomer->inuse) continue;
				if (!(gomer->svflags & SVF_MONSTER)) continue;
				if (gomer->health < gomer->gib_health) continue;
				if (gomer->svflags & SVF_NOCLIENT) continue;
				if (!gi.inPVS(self->s.origin,gomer->s.origin)) continue;
				VectorMA(gomer->absmin,0.5,gomer->size,target);
				VectorCopy(self->s.origin,t_start);
				VectorSubtract(target,self->s.origin,dir);
				VectorCopy(dir,f);
				VectorNormalize(f);
				VectorMA(t_start,self->teammaster->base_radius,f,t_start);
				tr = gi.trace(t_start,vec3_origin,vec3_origin,target,self,MASK_SHOT);
				if (tr.ent == gomer)
				{
					vectoangles(dir,angles);
					AnglesNormalize(angles);
					if ( yaw_restrict )
					{
						yaw_0  = angles[YAW] - self->pos1[YAW];
						if (yaw_0 < 0)
							yaw_0 += 360;
					}
					if ( (angles[PITCH] <= self->pos1[PITCH]) && (angles[PITCH] >= self->pos2[PITCH]) &&
						( !yaw_restrict || (yaw_0 <= yaw_r) ))
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
		}
		// for weapon-firing turrets, if GOODGUY is set and we already have an enemy, we're
		// done.
		if ( (self->sounds >= 0) && (self->spawnflags & SF_TURRET_GOODGUY) && self->enemy)
			goto good_enemy;

		// for non-GOODGUY weapon-firing turrets, reset best_dist so that players will
		// ALWAYS be selected if found
		if ( (self->sounds >= 0) && !(self->spawnflags & SF_TURRET_GOODGUY))
			best_dist = WORLD_SIZE;	// was 8192

		// hunt for closest player - hunt ALL entities since
		// we want to view fake players using camera
		for (i=1; i<globals.num_edicts; i++)
		{
			gomer = g_edicts + i;
			if (!gomer->inuse) continue;
			if (!gomer->client) continue;
			if (gomer->svflags & SVF_NOCLIENT) continue;
			if (gomer->health < gomer->gib_health) continue;
			if (gomer->flags & FL_NOTARGET) continue;
			if (!gi.inPVS(self->s.origin,gomer->s.origin)) continue;
			VectorMA(gomer->absmin,0.5,gomer->size,target);

			VectorCopy(self->s.origin,t_start);
			VectorSubtract(target,self->s.origin,dir);
			VectorCopy(dir,f);
			VectorNormalize(f);
			VectorMA(t_start,self->teammaster->base_radius,f,t_start);
			tr = gi.trace(t_start,vec3_origin,vec3_origin,target,self,MASK_SHOT);
			if (tr.ent == gomer)
			{
				vectoangles(dir,angles);
				AnglesNormalize(angles);
				if ( yaw_restrict )
				{
					yaw_0  = angles[YAW] - self->pos1[YAW];
					if (yaw_0 < 0)
						yaw_0 += 360;
				}
				if ( (angles[PITCH] <= self->pos1[PITCH]) && (angles[PITCH] >= self->pos2[PITCH]) &&
					( !yaw_restrict || (yaw_0 <= yaw_r) ) )
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
			VectorCopy (self->enemy->s.origin, target);
			if (self->enemy->deadflag)
				target[2] -= 16;
			if (skill->value >= 2)
			{
				VectorMA(target,FRAMETIME,self->enemy->velocity,target);

/* May add some variant of this for skill 3 later. For now, the following is virtually 
   indistinguishable from skill 2 for most normal setups. Trouble is, it is sometimes
   EASIER than skill 2.

				if (skill->value > 2)
				{
					float	t;
					VectorSubtract(target,self->s.origin,dir);
					VectorNormalize(dir);
					vectoangles(dir,dir);
					VectorSubtract(dir,self->s.angles,dir);
					AnglesNormalize(dir);
					dir[2] = max( fabs(dir[0]), fabs(dir[1]) );
					if (dir[2] > 0)
					{
						t = dir[2]/self->speed;
						VectorMA(target,t,self->enemy->velocity,target);
					}
				} */
			}
			if (self->sounds == 7)
			{
				if (!AimGrenade (self, start, target, self->fog_far, dir, false))
				{
					// Can't get a grenade to target. Correct yaw but
					// not pitch

					vec_t pitch = self->move_angles[PITCH];
					vectoangles (dir, self->move_angles);
					self->move_angles[PITCH] = pitch;
					if (skill->value > 0)
						turret_turn(self);
					return;
				}
			}
			else
				VectorSubtract (target, self->s.origin, dir);
			VectorNormalize(dir);
			vectoangles (dir, self->move_angles);
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
			if ( victim && self->sounds >= 0 && DotProduct(forward,dir) > 0.99f )
			{
				// never automatically fire a turret remotely controlled by 
				// a player
				if (!self->viewer || (self->viewer && !self->viewer->client))
				{
					// don't fire rockets or homing rockets if remote turret_driver is
					// too close to target
					if (remote_monster)
					{
						vec3_t	range;
						vec_t	r;
						VectorSubtract(self->enemy->s.origin,self->owner->s.origin,range);
						r = VectorLength(range);
						if (r < 128) return;
					}
					if (level.time < self->monsterinfo.attack_finished)
					{
						if (skill->value > 0)
							turret_turn(self);
						return;
					}
					if ( (self->sounds == 5) || (self->sounds == 6) )
						reaction_time = 0;
					else
						reaction_time = max(0., 0.5*(2-skill->value));
					if ((level.time - self->monsterinfo.trail_time) < reaction_time)
					{
						if (skill->value > 0)
							turret_turn(self);
						return;
					}
					self->monsterinfo.attack_finished = level.time + reaction_time;
					if ( (self->sounds != 5) && (self->sounds != 6) )
						self->monsterinfo.attack_finished += self->wait;
					turret_breach_fire (self);
					if (skill->value > 0)
						turret_turn(self);
				}
			}
			else
			{
				if (skill->value > 0)
					turret_turn(self);
			}
		}
	}
	// If turret has no enemy and isn't controlled by a player or monster,
	// check for "followtarget"
	if ((!self->enemy) && ((!self->owner) || remote_monster))
	{
		if (self->followtarget)
		{
			self->enemy = G_Find(NULL,FOFS(targetname),self->followtarget);
			if (self->enemy)
			{
				VectorMA (self->enemy->absmin, 0.5, self->enemy->size, target);
				VectorSubtract (target, self->s.origin, dir);
				vectoangles (dir, self->move_angles);
				if (skill->value > 0)
					turret_turn(self);
			}
		}
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
		if (!self->target_ent)
		{
			gi.dprintf("%s at %s, target %s does not exist\n",
				self->classname, vtos(self->s.origin), self->target);
			G_FreeEdict(self);
			return;
		}
		VectorSubtract (self->target_ent->s.origin, self->s.origin, self->move_origin);
		// Knightmare- if we've been moved by a func_train before initializing,
		// shift firing point by the distance moved
		if (VectorLength(self->aim_point))
			VectorAdd (self->move_origin, self->aim_point, self->move_origin);

		G_FreeEdict(self->target_ent);

//CW++	Double-barrelled turrets.
		self->target_ent = NULL;
		self->target_ent = G_PickTarget(self->combattarget);
		if (self->target_ent)
		{
			VectorSubtract(self->target_ent->s.origin, self->s.origin, self->muzzle2);
			// Knightmare- if we've been moved by a func_train before initializing,
			// shift firing point by the distance moved
			if (VectorLength(self->aim_point))
				VectorAdd (self->muzzle2, self->aim_point, self->muzzle2);

			self->moreflags |= FL2_TURRET_DOUBLE;
			if (self->style > 0)
				self->moreflags |= FL2_TURRET_DOUBLE_ALT;

			G_FreeEdict(self->target_ent);
		}
//CW--
	}

	if (!self->team)
		self->teammaster = self;
	self->teammaster->dmg = self->dmg;

	if (!(self->spawnflags & (SF_TURRET_TRIGGER_SPAWN | SF_TURRET_GOODGUY | SF_TURRET_INACTIVE) ))
	{
		self->think = turret_breach_think;
		self->think (self);
	}
	else
	{
		self->think = NULL;
		self->nextthink = 0;
	}

}

void turret_breach_die_temp_think(edict_t *self)
{
	edict_t	*target;
	target = G_Find(NULL,FOFS(targetname),self->destroytarget);
	while (target)
	{
		if (target && target->use)
			target->use(target,self->target_ent,self->target_ent);
		target = G_Find(target,FOFS(targetname),self->destroytarget);
	}
	G_FreeEdict(self);
}

void turret_driver_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void SP_monster_infantry (edict_t *self);
void monster_start_go(edict_t *self);

void turret_breach_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	i;
	edict_t	*ent;
	edict_t	*player;

	// ensure turret_base stops rotating
	if (self->team)
	{
		for (ent = self->teammaster; ent; ent = ent->teamchain)
		{
			if (ent != self)
			{
				ent->avelocity[1] = 0;
				gi.linkentity(ent);
			}
		}
	}

	if (self->deadflag != DEAD_DEAD)
	{	// if turret has a driver, kill him too unless he's a "remote" driver
		if (self->owner && (self->owner->target_ent == self))
		{
			if (self->owner->spawnflags & SF_TURRETDRIVER_REMOTE_DRIVER)
			{
				// remote driver - remove and replace with normal infantry
				edict_t	*monster;
				monster = self->owner->child;
				if (monster)
				{
					monster->health = self->owner->health;
					monster->enemy  = self->owner->enemy;
					G_FreeEdict(self->owner);
					monster->solid = SOLID_BBOX;
					monster->movetype = MOVETYPE_STEP;
					monster->svflags &= ~SVF_NOCLIENT;
					monster_start_go (monster);
					gi.linkentity (monster);
					if (monster->enemy) FoundTarget(monster);
				}
			}
			else
			{
				T_Damage(self->owner, inflictor, attacker, vec3_origin, self->owner->s.origin, vec3_origin, 100000, 1, 0, 0);
			}
		}
		// if turret is being used as a camera by a player, turn camera off for that player
		for (i=0,player=g_edicts+1; i<maxclients->value; i++, player++)
		{
			if (player->client && player->client->spycam == self)
				camera_off(player);
		}
		if (self->deathtarget)
		{
			edict_t	*target;
			target = G_Find(NULL,FOFS(targetname),self->deathtarget);
			while (target)
			{
				if (target && target->use)
					target->use(target,attacker,attacker);
				target = G_Find(target,FOFS(targetname),self->deathtarget);
			}
		}
	}
	if (self->health <= self->gib_health)
	{
		if (self->destroytarget)
		{
			if (self->deadflag == DEAD_DEAD)
			{
				// we were already dead, so destroytarget has been fired
				edict_t	*target;
				target = G_Find(NULL,FOFS(targetname),self->destroytarget);
				while (target)
				{
					if (target && target->use)
						target->use(target,attacker,attacker);
					target = G_Find(target,FOFS(targetname),self->destroytarget);
				}
			}
			else
			{
				// we were killed and gibbed in the same frame. postpone
				// destroytarget just a bit
				edict_t *temp;
				temp = G_Spawn();
				temp->solid = SOLID_NOT;
				temp->svflags = SVF_NOCLIENT;
				temp->think = turret_breach_die_temp_think;
				temp->nextthink = level.time + 2*FRAMETIME;
				temp->destroytarget = self->destroytarget;
				temp->target_ent = attacker;
				gi.linkentity(temp);
			}
			self->nextthink = 0;
			gi.linkentity(self);
		}
		if (self->dmg > 0)
		{
			if (self->mass >= 400)
				BecomeExplosion3 (self);
			else
				BecomeExplosion1(self);
		}
		else
			G_FreeEdict(self);
		return;
	}
	if (self->deadflag == DEAD_DEAD)
		return;
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	// slow turret down and level it... or for MD2 turrets set to minpitch
	self->speed /= 4;
	if (self->spawnflags & SF_TURRET_MD2)
		self->move_angles[0] = self->pos1[0];
	else
		self->move_angles[0] = 0;
}

void toggle_turret_breach (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!(self->spawnflags & SF_TURRET_INACTIVE))
	{
		self->spawnflags |= SF_TURRET_INACTIVE;
		VectorCopy(self->s.angles,self->move_angles);
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
		self->nextthink = 0;
	}
	else
	{
		self->spawnflags &= ~SF_TURRET_INACTIVE;
		self->think = turret_breach_think;
		self->nextthink = level.time + FRAMETIME;
	}
}

void turret_breach_use (edict_t *self, edict_t *activator, edict_t *other)
{
/*	self->use = NULL;
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->think = turret_breach_finish_init;
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
*/
	if (self->spawnflags & SF_TURRET_TRIGGER_SPAWN)
	{
		self->spawnflags &= ~SF_TURRET_TRIGGER_SPAWN;
		self->svflags &= ~SVF_NOCLIENT;
		if (self->spawnflags & SF_TURRET_MD2)
			self->solid = SOLID_BBOX;
		else
			self->solid = SOLID_BSP;
		self->think = turret_breach_think;
		self->think(self);
	}
}

void turret_breach_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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
	if ( (other->client) && (other->absmin[2] > self->s.origin[2]) )
	{
		if ( fabs(self->s.angles[PITCH] - self->pos1[PITCH]) <
			fabs(self->s.angles[PITCH] - self->pos2[PITCH])  )
			self->move_angles[PITCH] = self->pos2[PITCH];
		else
			self->move_angles[PITCH] = self->pos1[PITCH];
		
		if ( fabs(self->s.angles[YAW] - self->pos1[YAW]) <
			fabs(self->s.angles[YAW] - self->pos2[YAW])  )
			self->move_angles[YAW] = self->pos2[YAW];
		else
			self->move_angles[YAW] = self->pos1[YAW];
	}
}

void SP_turret_breach (edict_t *self)
{
	self->class_id = ENTITY_TURRET_BREACH;

	//Knightmare- no goodguy flag on level02 of Zorius
	if (/*Q_stricmp(level.mapname, "level02") == 0
		||*/ Q_stricmp(level.mapname, "cm3pt1") == 0
		|| Q_stricmp(level.mapname, "cm3pt3") == 0)
	{
		gi.dprintf("Removing goodguy flag from turret\n");
		self->spawnflags &= ~SF_TURRET_GOODGUY;
	}

	// Good guy turrets shoot at monsters, not players. Turn TRACK on if it ain't already
	if (self->spawnflags & SF_TURRET_GOODGUY)
		self->spawnflags |= (SF_TURRET_TRACKING | SF_TURRET_INACTIVE);

//	self->solid = SOLID_BSP;
//	self->movetype = MOVETYPE_PUSH;
//	gi.setmodel (self, self->model);

	if (self->spawnflags & SF_TURRET_MD2)
	{
		char modelname[256];
		if (!self->usermodel)
		{
			gi.dprintf("%s w/o a model and MD2 spawnflag set at %s\n",
				self->classname,vtos(self->s.origin));
			G_FreeEdict(self);
			return;
		}
		// Knightmare- check for "models/" already in path
		if ( !strncmp(self->usermodel, "models/", 7) )
			Com_sprintf(modelname, sizeof(modelname), "%s", self->usermodel);
		else
			Com_sprintf(modelname, sizeof(modelname), "models/%s", self->usermodel);
		self->s.modelindex = gi.modelindex (modelname);

		if ( (VectorLength(self->bleft) == 0) &&
			 (VectorLength(self->tright) == 0)   )
		{
			VectorSet(self->bleft,-16,-16,-16);
			VectorSet(self->tright,16, 16, 16);
		}
		VectorCopy(self->bleft,self->mins);
		VectorCopy(self->tright,self->maxs);

		if (self->spawnflags & SF_TURRET_TRIGGER_SPAWN)
		{
			self->svflags |= SVF_NOCLIENT;
			self->solid = SOLID_NOT;
			self->use = turret_breach_use;
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
			self->use = turret_breach_use;
		}
		else
		{
			self->solid = SOLID_BSP;
			if (self->spawnflags & SF_TURRET_TRACKING)
				self->use = toggle_turret_breach;
		}
		gi.setmodel (self, self->model);
	}
	self->movetype = MOVETYPE_PUSH;

	// grenade speed. Yes I know this is a misuse of this. Sue me.
	if (self->distance)
		st.distance = self->distance;
	if (st.distance)
		self->fog_far = st.distance;
	else
		self->fog_far = TURRET_GRENADE_SPEED;

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
	if (!self->wait && (self->sounds != 5) && (self->sounds != 6) )
		self->wait = 1;
	if (self->health > 0)
	{
		self->die = turret_breach_die;
		self->takedamage = DAMAGE_YES;
	}
	// Added touch routine to help prevent player from getting stuck after
	// jumping on turret barrel
	self->touch = turret_breach_touch;

	self->pos1[PITCH] = -1 * st.minpitch;
	self->pos1[YAW]   = st.minyaw;
	self->pos2[PITCH] = -1 * st.maxpitch;
	self->pos2[YAW]   = st.maxyaw;

	if (self->pos1[YAW] < 0)
		self->pos1[YAW] += 360;
	if (self->pos2[YAW] < 0)
		self->pos2[YAW] += 360;

	self->ideal_yaw = self->s.angles[YAW];
	self->move_angles[YAW] = self->ideal_yaw;

	self->blocked = turret_blocked;

	self->think = turret_breach_finish_init;
	// Knightmare- wait 2 serverframes for all entities to spawn
	self->nextthink = level.time + 2 * FRAMETIME;

	// Lazarus: Added so monsters will attack turrets that
	//          fire at them
	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (self);
}

void SP_model_turret(edict_t *self)
{
	self->spawnflags |= SF_TURRET_MD2;
	SP_turret_breach(self);

	self->class_id = ENTITY_MODEL_TURRET;
}


/*QUAKED turret_base (0 0 0) ? x TRIGGER_SPAWN
This portion of the turret changes yaw only.
MUST be teamed with a turret_breach.

TRIGGER_SPAWN- Spawns upon being triggered

"health"	if set, it explodes like a func_explosive- default to 100 health, 75 mass, etc
		Will also destroy the turret's breach.
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

void turret_base_use (edict_t *self, edict_t *activator, edict_t *other)
{
	self->use = NULL;
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	gi.linkentity (self);
}

void SP_turret_base (edict_t *self)
{
	self->class_id = ENTITY_TURRET_BASE;

	if (self->spawnflags & SF_TURRET_TRIGGER_SPAWN)
	{
		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->use   = turret_base_use;
	}
	else
		self->solid = SOLID_BSP;

	self->movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self->model);
	self->blocked = turret_blocked;
	//Knightmare- explode option
	if (self->health > 0)
	{
		//the func_explosive is handled exactly the same way. (look in g_misc.c)
		self->die = func_explosive_die;	//what to call when we die (in g_misc.c)
		self->takedamage = DAMAGE_YES;	//we take damage hits (spark, bleed, etc)
	}
	// Lazarus - added turret_base_finish to find the circle that encloses
	// the base. That radius is used to look past the turret_base when
	// searching for targets.
	self->s.angles[PITCH] = self->s.angles[ROLL] = 0;
	self->think = turret_base_finish;
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
}


/*QUAKED turret_driver (1 .5 0) (-16 -16 -24) (16 16 32) REMOTE_DRIVER
Must NOT be on the team with the rest of the turret parts.
Instead it must target the turret_breach.
*/

void infantry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage);
void infantry_stand (edict_t *self);
void monster_use (edict_t *self, edict_t *other, edict_t *activator);

void turret_driver_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t	*ent;

	if (self->target_ent->inuse)
	{
		// level the gun
		self->target_ent->move_angles[0] = 0;

		if (self->spawnflags & SF_TURRETDRIVER_REMOTE_DRIVER)
			// "remote" driver... turn off TRACK for turret
			self->target_ent->spawnflags &= ~SF_TURRET_TRACKING;
		else
		{

			// remove the driver from the end of them team chain
			for (ent = self->target_ent->teammaster; ent->teamchain != self; ent = ent->teamchain)
				;
			ent->teamchain = NULL;
			self->teammaster = NULL;
			self->flags &= ~FL_TEAMSLAVE;
			self->target_ent->owner = NULL;
			self->target_ent->teammaster->owner = NULL;
		}
		self->target_ent->owner = NULL;
	}
	infantry_die (self, inflictor, attacker, damage);
}

void turret_driver_think (edict_t *self)
{
	vec3_t	target;
	vec3_t	dir;
	float	reaction_time;

	self->nextthink = level.time + FRAMETIME;

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

void turret_driver_link (edict_t *self)
{
	vec3_t	vec;
	edict_t	*ent;

	self->think = turret_driver_think;
	self->nextthink = level.time + FRAMETIME;

	self->target_ent = G_PickTarget (self->target);
	self->target_ent->owner = self;
//	self->target_ent->teammaster->owner = self;
//	VectorCopy (self->target_ent->s.angles, self->s.angles);

	// DWH: We no longer require turret_breach to be teamed with a turret_base UNLESS
	//      there's a turret_driver
	if (!self->target_ent->team)
	{
		gi.dprintf("turret_driver at %s targets a no-team turret_breach\n",
			vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}
	// DWH: REMOTE (=1) drivers aren't part of the turret team, and don't think
	//      (the turret_breach thinks for them)
	if (!(self->spawnflags & SF_TURRETDRIVER_REMOTE_DRIVER))
	{
		self->think = turret_driver_think;
		self->nextthink = level.time + FRAMETIME;
		self->target_ent->teammaster->owner = self;
		VectorCopy (self->target_ent->s.angles, self->s.angles);
	}

	vec[0] = self->target_ent->s.origin[0] - self->s.origin[0];
	vec[1] = self->target_ent->s.origin[1] - self->s.origin[1];
	vec[2] = 0;
	self->move_origin[0] = VectorLength(vec);

	VectorSubtract (self->s.origin, self->target_ent->s.origin, vec);
	vectoangles (vec, vec);
	AnglesNormalize(vec);
	self->move_origin[1] = vec[1];

	self->move_origin[2] = self->s.origin[2] - self->target_ent->s.origin[2];

	// add the driver to the end of the team chain
	// DWH: REMOTE (=1) drivers don't move with turret, and the turret tracks
	//      players
	if (self->spawnflags & SF_TURRETDRIVER_REMOTE_DRIVER)
		self->target_ent->spawnflags |= SF_TURRET_TRACKING;
	else
	{
		for (ent = self->target_ent->teammaster; ent->teamchain; ent = ent->teamchain)
			;
		ent->teamchain = self;
		self->teammaster = self->target_ent->teammaster;
		self->flags |= FL_TEAMSLAVE;
	}
}

void SP_turret_driver (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->class_id = ENTITY_TURRET_DRIVER;

	self->movetype = MOVETYPE_PUSH;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/infantry/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->health = 100;
	self->gib_health = 0;
	self->mass = 200;
	self->viewheight = 24;

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
	//Knightmare- set max visible range
	self->monsterinfo.max_range = 1280;

	if (st.item)
	{
		self->item = FindItemByClassname (st.item);
		if (!self->item)
			gi.dprintf("%s at %s has bad item: %s\n", self->classname, vtos(self->s.origin), st.item);
	}

	self->think = turret_driver_link;
	self->nextthink = level.time + FRAMETIME;

	if (self->spawnflags & SF_TURRETDRIVER_REMOTE_DRIVER)
	{
		// remote turret driver - go ahead and create his "real" infantry replacement
		// NOW so the switch won't be so time-consuming
		edict_t	*infantry;
		infantry = G_Spawn();
		infantry->spawnflags = SF_TURRET_TRIGGER_SPAWN;
		VectorCopy(self->s.angles,infantry->s.angles);
		VectorCopy(self->s.origin,infantry->s.origin);
		infantry->s.origin[2] += 1.0;
		infantry->health = self->health;
		SP_monster_infantry(infantry);
		self->child = infantry;
	}

	gi.linkentity (self);
}

//============
// ROGUE

// invisible turret drivers so we can have unmanned turrets.
// originally designed to shoot at func_trains and such, so they
// fire at the center of the bounding box, rather than the entity's
// origin.

void turret_brain_think (edict_t *self)
{
	vec3_t	target;
	vec3_t	dir;
	vec3_t	endpos;
	float	reaction_time;
	trace_t	trace;

	self->nextthink = level.time + FRAMETIME;

	if (self->enemy)
	{
		if (!self->enemy->inuse)
			self->enemy = NULL;
		else if (self->enemy->takedamage && self->enemy->health <= 0)
			self->enemy = NULL;
	}

	if (!self->enemy)
	{
		if (!FindTarget (self))
			return;
		self->monsterinfo.trail_time = level.time;
		self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
	}
	else
	{
		VectorAdd (self->enemy->absmax, self->enemy->absmin, endpos);
		VectorScale (endpos, 0.5, endpos);

		trace = gi.trace (self->target_ent->s.origin, vec3_origin, vec3_origin, endpos, self->target_ent, MASK_SHOT);
		if (trace.fraction == 1 || trace.ent == self->enemy)
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
	VectorCopy (endpos, target);
	VectorSubtract (target, self->target_ent->s.origin, dir);
	vectoangles (dir, self->target_ent->move_angles);

	// decide if we should shoot
	if (level.time < self->monsterinfo.attack_finished)
		return;

	if (self->delay)
		reaction_time = self->delay;
	else
		reaction_time = (3 - skill->value) * 1.0;

	if ((level.time - self->monsterinfo.trail_time) < reaction_time)
		return;
	self->monsterinfo.attack_finished = level.time + reaction_time + 1.0;
	//FIXME how do we really want to pass this along?
	self->target_ent->spawnflags |= 65536;
	
}

// =================
// =================
void turret_brain_link (edict_t *self)
{
	vec3_t	vec;
	edict_t	*ent;

	if (self->killtarget)
	{
		self->enemy = G_PickTarget (self->killtarget);
	}

	self->think = turret_brain_think;
	self->nextthink = level.time + FRAMETIME;

	self->target_ent = G_PickTarget (self->target);
	self->target_ent->owner = self;
	self->target_ent->teammaster->owner = self;
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
}

// =================
// =================
void turret_brain_activate (edict_t *self, edict_t *other, edict_t *activator);

void turret_brain_deactivate (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & 2) //Knightmare- allow reactivation with toggle flag
		self->use = turret_brain_activate;
	self->think = NULL;
	self->nextthink = 0;
}

// =================
// =================
void turret_brain_activate (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!self->enemy)
	{
		self->enemy = activator;
	}

	// wait at least 3 seconds to fire.
	self->monsterinfo.attack_finished = level.time + 3;
	self->use = turret_brain_deactivate;

	self->think = turret_brain_link;
	self->nextthink = level.time + FRAMETIME;
}

/*QUAKED turret_invisible_brain (1 .5 0) (-16 -16 -16) (16 16 16) x TOGGLE
Invisible brain to drive the turret.

Does not search for targets. If targeted, can only be turned on once
and then off once. After that they are completely disabled, unless
the toggle flag is checked.

"delay" the delay between firing (default ramps for skill level)
"target" the turret breach
"killtarget" the item you want it to attack.
Target the brain if you want it activated later, instead of immediately. It will wait 3 seconds
before firing to acquire the target.
*/
void SP_turret_invisible_brain (edict_t *self)
{
	if (!self->killtarget && !(self->spawnflags & 1)) //Knightmare- exception for attack player mode
	{
		gi.dprintf("turret_invisible_brain with no killtarget!\n");
		G_FreeEdict (self);
		return;
	}
	if (!self->target)
	{
		gi.dprintf("turret_invisible_brain with no target!\n");
		G_FreeEdict (self);
		return;
	}

	if (self->targetname)
	{
		self->use = turret_brain_activate;
	}
	else
	{
		self->think = turret_brain_link;
		self->nextthink = level.time + FRAMETIME;
	}

	self->class_id = ENTITY_TURRET_INVISIBLE_BRAIN;

	self->movetype = MOVETYPE_PUSH;
	gi.linkentity (self);
}

// ROGUE
//============
