/*****************************************************************

	Eraser Bot source code - by Ryan Feltrin

	..............................................................

	This file is Copyright(c) 1998, Ryan Feltrin, All Rights Reserved.

	..............................................................

	All other files are Copyright(c) Id Software, Inc.

	Please see liscense.txt in the source directory for the copyright
	information regarding those files belonging to Id Software, Inc.

	..............................................................

	Should you decide to release a modified version of Eraser, you MUST
	include the following text (minus the BEGIN and END lines) in the
	documentation for your modification.

	--- BEGIN ---

	The Eraser Bot is a product of Ryan Feltrin, and is available from
	the Eraser Bot homepage, at http://impact.frag.com.

	This program is a modification of the Eraser Bot, and is therefore
	in NO WAY supported by Ryan Feltrin.

	This program MUST NOT be sold in ANY form. If you have paid for
	this product, you should contact Ryan Feltrin immediately, via
	the Eraser Bot homepage.

	--- END ---

	..............................................................

	You will find p_trail.c has not been included with the Eraser
	source code release. This is NOT an error. I am unable to
	distribute this file because it contains code that is bound by
	legal documents, and signed by myself, never to be released
	to the public. Sorry guys, but law is law.

	I have therefore include the compiled version of these files
	in .obj form in the src\Release and src\Debug directories.
	So while you cannot edit and debug code within these files,
	you can still compile this source as-is. Although these will only
	work in MSVC v5.0, linux versions can be made available upon
	request.

	NOTE: When compiling this source, you will get a warning
	message from the compiler, regarding the missing p_trail.c
	file. Just ignore it, it will still compile fine.

	..............................................................

	I, Ryan Feltrin, hold no responsibility for any harm caused by the
	use of this source code. I also am NOT willing to provide any form
	of help or support for this source code. It is provided as-is,
	as a service by me, with no documentation, other then the comments
	contained within the code. If you have any queries, I suggest you
	visit the "official" Eraser source web-board, at
	http://www.telefragged.com/epidemic/. I will stop by there from
	time to time, to answer questions and help with any problems that
	may arise.

	Otherwise, have fun, and I look forward to seeing what can be done
	with this.

	-Ryan Feltrin

 *****************************************************************/

#include "g_local.h"
#include "bot_procs.h"
#include "m_player.h"
#include "g_items.h"

#include <sys/timeb.h>

#include "aj_weaponbalancing.h"	// AJ
#include "aj_statusbars.h" // AJ
byte P_DamageModifier (edict_t *ent);

int	aborted_fire;

int	bot_is_quad;
int	bot_is_double;
int bot_is_quadfire;

void bot_FireWeapon(edict_t	*self)
{
	if (!self->enemy)
		return;

	// set Quad/double flag
	P_DamageModifier(self);
	bot_is_quad = (self->client->quad_framenum > level.framenum);
	bot_is_double = (self->client->double_framenum > level.framenum);
	bot_is_quadfire = (self->client->quadfire_framenum > level.framenum);

	aborted_fire = false;
	self->bot_fire(self);

	if (!aborted_fire)
	{
		if (!CTFApplyStrengthSound(self))
		{
			if (bot_is_quad)
				gi.sound(self, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
			if (bot_is_double)
				gi.sound(self, CHAN_ITEM, gi.soundindex("items/ddamage3.wav"), 1, ATTN_NORM, 0);
			if (bot_is_quadfire)
				gi.sound(self, CHAN_ITEM, gi.soundindex("items/quadfire3.wav"), 1, ATTN_NORM, 0);
		}

		self->last_fire = level.time;

		if (self->maxs[2] == 4)
			self->s.frame = FRAME_crattak1;
		else
			self->s.frame = FRAME_attack1;

		if (self->bot_fire != botBlaster)
			self->last_reached_trail = level.time;
	}

// AJ - end of safety as firing...
	if (self->client->safety_mode)
		{
			self->takedamage = DAMAGE_YES;
			self->client->safety_mode = false;
//			if (!ent->bot_client)
//				gi.centerprintf(ent, "End of safety.");
			self->client->ps.stats[STAT_LITHIUM_MODE] = 0;
//			self->s.effects &= 0xF7FFFFFF; // clear the yellow shell
			self->s.effects &= EF_HALF_DAMAGE; //ScarFace- clear green shell
		}
// end AJ


	// check for no ammo
	if ((self->client->pers.weapon != item_blaster) && (!self->client->pers.inventory[self->client->ammo_index]))
	{
		botPickBestWeapon(self);
	}

	if ((self->bot_fire == botBlaster) && (self->enemy) && !CarryingFlag(self->enemy))
	{	// abort attacking enemy?
		gclient_t	*client;

		client = self->enemy->client;

		if ((	(self->health < 20) &&
				(self->enemy->health > 15) &&
				((self->enemy->health > self->health) || client->pers.weapon != item_blaster /*ClientHasAnyWeapon(client)*/)))
		{	// abort the attack
			self->enemy = NULL;
		}
	}
}

void	bot_Attack(edict_t *self)
{
	float	strafe_speed=BOT_STRAFE_SPEED;

	if (!self->enemy || (self->enemy->health <= 0) || (self->health <= 0))
		return;

	// see if the enemy is visible
	if (	(self->last_enemy_sight > (level.time - 0.2))
		||	(	(visible(self, self->enemy) && gi.inPVS(self->s.origin, self->enemy->s.origin))
			 &&	(self->last_enemy_sight = level.time)))
	{
		trace_t	trace;

		// make sure we don't hit a friend
		if (!((int)(dmflags->value) & DF_NO_FRIENDLY_FIRE) && (ctf->value || self->client->team))
		{
			trace = gi.trace (self->s.origin, vec3_origin, vec3_origin, self->enemy->s.origin, self, MASK_PLAYERSOLID);

			if (trace.ent && trace.ent->client && SameTeam(trace.ent,self))
			{	// we might hit a good guy!
				return;
			}
		}

		// BFG delayed firing
		if ((self->bot_fire == botBFG) && (self->last_fire > level.time) && (self->last_fire <= (level.time+0.1)))
		{
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/bfg__f1y.wav"), 1, 2, 0);
			self->last_fire = level.time - FIRE_INTERVAL_BFG + 0.5;
		}

		if (	((self->last_fire + self->fire_interval) < level.time)
			&&	(self->sight_enemy_time < (level.time - (SIGHT_FIRE_DELAY * ((5 - self->bot_stats->combat*0.5)/5)))))
		{
			bot_FireWeapon(self);

			if (CTFApplyHaste(self))
				CTFApplyHasteSound(self);
		}

		if (self->bot_stats->combat == 1)	// can't strafe if bad Combat skills
			strafe_speed = 0;

		if (self->enemy != self->last_movegoal)
		{	// only strafe slowly, so we don't go too far off course
			strafe_speed = 0;
		}
		else if (self->maxs[2] == 4)
		{
			strafe_speed *= 0.5;
		}

		// do attack movements, like strafing
		if ((strafe_speed > 0) && self->enemy && self->groundentity && (self->strafe_changedir_time < level.time) && (self->bot_stats->combat > 1))
		{
			self->strafe_dir = !self->strafe_dir;
			self->strafe_changedir_time = level.time + 0.5 + random() * 1.5;

			// check for ducking or jumping
			if (self->crouch_attack_time < level.time)
			{
				float	rnd, dist;

				dist = entdist(self, self->enemy);
				rnd = random()*4;

				// if low combat, then skip jumping for a bit
				if (self->bot_stats->combat < rnd)
				{
					self->crouch_attack_time = level.time + 1;
					goto nojump;
				}

				if ((self->maxs[2] > 4) && (dist > 400) && (rnd < 3))	// crouch if far away
				{
					if (self->bot_stats->combat > 4)
					{
						self->crouch_attack_time = level.time + random()*0.5 + 0.5;
						self->maxs[2] = 4;
					}
				}
				else if (	(dist < 700)
						 &&	(	(self->last_seek_enemy < level.time)
							 ||	(entdist(self->last_movegoal, self) > 256))
						 &&	(CanJump(self)))	// jump
				{
					vec3_t	right, dest, mins;
					trace_t	trace;

					if ((self->bot_stats->combat >= 3) || (random() < 0.3))	// if combat = 2, jump less frequently
					{
						vec3_t	rvec;

						AngleVectors(self->s.angles, NULL, right, NULL);
						VectorCopy(right, rvec);
						VectorScale(right, ((self->strafe_dir * 2) - 1), right);
						VectorScale(right, BOT_STRAFE_SPEED, right);

						// check that the jump will be safe
//						VectorAdd(self->mins, tv(0,0,12), mins);
						VectorAdd(self->s.origin, right, dest);
						trace = gi.trace(self->s.origin, mins, self->maxs, dest, self, MASK_SOLID);
						VectorSubtract(trace.endpos, rvec, trace.endpos);
						VectorAdd(trace.endpos, tv(0,0,-256), dest);
						trace = gi.trace(trace.endpos, VEC_ORIGIN, VEC_ORIGIN, dest, self, MASK_SOLID | MASK_WATER);

						if ((trace.fraction < 1) && !(trace.contents & (CONTENTS_LAVA | CONTENTS_SLIME)))
						{
							VectorCopy(right, self->velocity);

							self->velocity[2] = 300;
							self->groundentity = NULL;
//							self->s.origin[2] += 1;

							gi.linkentity(self);

							VectorCopy(self->velocity, self->jump_velocity);

							gi.sound(self, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, 2, 0);
						}
						else	// jump straight up
						{
							VectorClear(self->velocity);

							self->velocity[2] = 300;
							self->groundentity = NULL;
//							self->s.origin[2] += 1;

							gi.linkentity(self);

							VectorCopy(self->velocity, self->jump_velocity);

							gi.sound(self, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, 2, 0);
						}
					}
				}

				if (	(rnd >= 1) && (self->maxs[2] == 4)
					&&	(!self->goalentity || (self->goalentity->maxs[2] > 4))
					&&	(CanStand(self)))
				{	// resume standing
					self->maxs[2] = 32;
				}
			}
		}

nojump:

		if (self->groundentity && (strafe_speed > 0))
		{
			if (!M_walkmove(self,
							self->s.angles[YAW] + (90 * ((self->strafe_dir * 2) - 1)),
							strafe_speed * bot_frametime ))
			{
				self->strafe_dir = !self->strafe_dir;
				self->strafe_changedir_time = level.time + 0.5 + (random() * 0.5);
			}
		}

//		self->last_enemy_sight = level.time;
	}
/*
	else if ((self->last_enemy_sight + BOT_SEARCH_LONG) < level.time)
	{
//gi.dprintf("Sight time expired\n");
		bot_roam(self, false);
	}
*/
	else	// once we sight them again, don't fire instantaneously (super-human powers)
	{
		self->sight_enemy_time = level.time;

		// abort chasing a RL welding human, with enough health
		if (self->enemy->client) //ScarFace
		if (!self->enemy->bot_client && (self->enemy->client->pers.weapon == item_rocketlauncher) && !CarryingFlag(self->enemy)
			&& ((self->enemy->health > 25) || (self->bot_fire == botBlaster || self->bot_fire == botShotgun)))
		{	// abort the attack
			// move away
			if (self->goalentity)
			{
				self->goalentity->ignore_time = level.time + 1;
				self->goalentity = NULL;
			}

			if (self->enemy->client->pers.weapon == item_rocketlauncher)
				self->enemy->ignore_time = level.time + 2;

			self->enemy = NULL;
		}
	}
}

void botBlaster (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	float	dist, tf;
	int		damage;
	int		effect;
	int		color;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);
	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	dist = entdist(self, self->enemy);

	if (self->enemy && infront(self, self->enemy))
	{

		if ((self->enemy->health > 0) && (skill->value > 1) && (self->enemy->client) && (dist > 64))
		{
			VectorMA (self->enemy->s.origin, dist * (1/1000), self->enemy->velocity, target);
			target[2] += self->enemy->viewheight - 8;
		}
		else if (!self->enemy->client) //ScarFace
		{
			if ( (!strcmp(self->enemy->classname, "doppleganger")) && (self->enemy->health > 0)
				&& (skill->value > 1) && (dist > 64) )
			{
				VectorMA (self->enemy->s.origin, dist * (1/1000), self->enemy->velocity, target);
				target[2] += self->enemy->viewheight - 8;
			}
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= 0.5 + (VectorLength(self->enemy->velocity)/600);

			else if (!self->enemy->client) //ScarFace
				if ( (!strcmp(self->enemy->classname, "doppleganger")) && !self->enemy->bot_client)
					tf *= 0.5 + (VectorLength(self->enemy->velocity)/600);

			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

// AJ changed constant (15) to cvar
	damage = sk_bot_blaster_damage->value;
	if (bot_is_quad)
		damage *= 4;
	if (bot_is_double)	// ScarFace
		damage *= 2;

	// Knightmare- blaster colors
	color = sk_blaster_color->value;
	// blaster_color could be any other value, so clamp it
	if ( (sk_blaster_color->value < 1) || (sk_blaster_color->value > 4) )
		color = BLASTER_ORANGE;
	// CTF color override
	if (ctf->value && ctf_blastercolors->value && self->client)
		color = (5 - self->client->resp.ctf_team);
#ifndef KMQUAKE2_ENGINE_MOD
	if (color == BLASTER_RED) color = BLASTER_ORANGE;
#endif

	if (color == BLASTER_GREEN)
		effect = (EF_BLASTER|EF_TRACKER);
	else if (color == BLASTER_BLUE)
#ifdef KMQUAKE2_ENGINE_MOD
		effect = EF_BLASTER|EF_BLUEHYPERBLASTER;
#else
		effect = EF_BLUEHYPERBLASTER;
#endif
	else if (color == BLASTER_RED)
		effect = EF_BLASTER|EF_IONRIPPER;
	else // standard orange
		effect = EF_BLASTER;

// AJ changed constant 1000 for cvar sk_bot_blaster_speed->value
	monster_fire_blaster (self, start, forward, damage, sk_bot_blaster_speed->value, MZ_BLASTER, effect, color);
}


void botMachineGun (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int	damage;

	if ((int)(level.time*10) & 1)	// only calculate every other frame
	{
		AngleVectors (self->s.angles, forward, right, NULL);
		G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

		VectorScale(forward, 8, ofs);
		VectorAdd(self->s.origin, ofs, start);
		start[2] += self->viewheight - 8;

		if (self->enemy && infront(self, self->enemy))
		{
			float	dist, tf;

			dist = entdist(self, self->enemy);

			if (self->enemy->health > 0)
			{
				VectorCopy (self->enemy->s.origin, target);

				if (skill->value <= 1)
				{	// trail the player's velocity
					VectorMA(target, -0.2, self->enemy->velocity, target);
				}

				target[2] += self->enemy->viewheight - 8;
			}
			else
			{
				VectorCopy (self->enemy->s.origin, target);
				target[2] += self->enemy->viewheight - 8;
			}

			if (self->bot_stats->accuracy < 5)
			{
				tf = (dist < 256) ? dist/2 : 256;
				tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
				if (self->enemy->client && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));

				else if (!self->enemy->client) //ScarFace
					if ( (!strcmp(self->enemy->classname, "doppleganger")) && !self->enemy->bot_client)
						tf *= (1 - (VectorLength(self->enemy->velocity)/600));

				VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.1), target);
			}

			VectorSubtract (target, start, forward);
			VectorNormalize (forward);

			vectoangles(forward, self->s.angles);
			if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
				self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
		}
		else
		{
			aborted_fire = true;
			return;
		}

		VectorCopy(forward, self->last_forward);
		VectorCopy(start, self->last_start);
	}
	else
	{
		VectorCopy(self->last_forward, forward);
		VectorCopy(self->last_start, start);
	}


// AJ changed constant (3) to cvar
	damage = sk_bot_machinegun_damage->value;
	if (bot_is_quad)
		damage *= 4;
	if (bot_is_double)	//ScarFace
		damage *= 2;
// AJ changed constants to cvars (hspread, vspread)
	//ScarFace- changed to player muzzle flash
	monster_fire_bullet (self, start, forward, damage, 4, sk_bot_machinegun_hspread->value, sk_bot_machinegun_vspread->value, MZ_MACHINEGUN, MOD_MACHINEGUN); //MZ2_ACTOR_MACHINEGUN_1

	self->client->pers.inventory[self->client->ammo_index]--;

/*
#ifdef	_WIN32
	_ftime(&self->lastattack_time);
#else
	ftime(&self->lastattack_time);
#endif
*/
}

void botShotgun (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int	damage;
	float	dist=0, tf;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{

		dist = entdist(self, self->enemy);

		if (self->enemy->health > 0)
		{
			VectorCopy (self->enemy->s.origin, target);

			if (skill->value <= 1)
			{	// trail the player's velocity
				VectorMA(target, -0.2, self->enemy->velocity, target);
			}
			target[2] += self->enemy->viewheight - 8;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));

			else if (!self->enemy->client) //ScarFace
				if ( (!strcmp(self->enemy->classname, "doppleganger")) && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));

			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

// AJ changed constant (3) to cvar
	damage = sk_bot_shotgun_damage->value;
	if (bot_is_quad)
		damage *= 4;
	if (bot_is_double)	//ScarFace
		damage *= 2;

// AJ changed constants to cvar's (hspread, vspread, count)
	monster_fire_shotgun (self, start, forward, damage, 4, sk_bot_shotgun_vspread->value, sk_bot_shotgun_vspread->value, sk_bot_shotgun_count->value, MZ_SHOTGUN, MOD_SHOTGUN);
	self->client->pers.inventory[self->client->ammo_index]--;

	if (dist > 700)
	{	// check for a better long distance weapon
		botPickBestFarWeapon(self);
	}
}

void botSuperShotgun (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	vec3_t	angles;
	int damage;
	float	dist=0, tf;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{

		dist = entdist(self, self->enemy);

		if (self->enemy->health > 0)
		{
			VectorCopy (self->enemy->s.origin, target);

			if (skill->value <= 1)
			{	// trail the player's velocity
				VectorMA(target, -0.2, self->enemy->velocity, target);
			}

			target[2] += self->enemy->viewheight - 8;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));

			else if (!self->enemy->client) //ScarFace
				if ( (!strcmp(self->enemy->classname, "doppleganger")) && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));

			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

// AJ changed constant (3*2) to cvar
	damage = sk_bot_sshotgun_damage->value; // OPTIMIZE: increase damage, decrease number of bullets
	if (bot_is_quad)
		damage *= 4;
	if (bot_is_double)	//ScarFace
		damage *= 2;

	vectoangles(forward, angles);

	angles[YAW] += 5;
	AngleVectors(angles, forward, NULL, NULL);
// AJ changed constants to cvars. un-did Ridah's optimisation (/4 changed to /2)
	monster_fire_shotgun (self, start, forward, damage, 4, sk_bot_sshotgun_hspread->value, sk_bot_sshotgun_vspread->value, sk_bot_sshotgun_count->value/2, MZ_SSHOTGUN, MOD_SSHOTGUN);

	angles[YAW] -= 10;
	AngleVectors(angles, forward, NULL, NULL);
// AJ changed constants to cvars. un-did Ridah's optimisation (/4 changed to /2)
	monster_fire_shotgun (self, start, forward, damage, 4, sk_bot_sshotgun_hspread->value, sk_bot_sshotgun_vspread->value, sk_bot_sshotgun_count->value/2, MZ_SSHOTGUN | MZ_SILENCED, MOD_SSHOTGUN);

	self->client->pers.inventory[self->client->ammo_index] -= 2;
	if (self->client->pers.inventory[self->client->ammo_index] < 0)
		self->client->pers.inventory[self->client->ammo_index] = 0;

	if (dist > 700)
	{	// check for a better long distance weapon
		botPickBestFarWeapon(self);
	}
}

void botChaingun (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int		shots, damage, kick;

	if ((int)(level.time*10) & 1)	// only calculate every other frame
	{
		AngleVectors (self->s.angles, forward, right, NULL);
		G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

		VectorScale(forward, 8, ofs);
		VectorAdd(self->s.origin, ofs, start);
		start[2] += self->viewheight - 8;

		if (self->enemy && infront(self, self->enemy))
		{
			float	dist, tf;

			dist = entdist(self, self->enemy);

			if (self->enemy->health > 0)
			{
				VectorCopy (self->enemy->s.origin, target);

				if (skill->value <= 1)
				{	// trail the player's velocity
					VectorMA(target, -0.2, self->enemy->velocity, target);
				}

				target[2] += self->enemy->viewheight - 8;
			}
			else
			{
				VectorCopy (self->enemy->s.origin, target);
				target[2] += self->enemy->viewheight - 8;
			}

			if (self->bot_stats->accuracy < 5)
			{
				tf = (dist < 256) ? dist/2 : 256;
				tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
				if (self->enemy->client && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));

				else if (!self->enemy->client) //ScarFace
					if ( (!strcmp(self->enemy->classname, "doppleganger")) && !self->enemy->bot_client)
						tf *= (1 - (VectorLength(self->enemy->velocity)/600));

				if (tf > 0)
					VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
			}

			VectorSubtract (target, start, forward);
			VectorNormalize (forward);

			vectoangles(forward, self->s.angles);
			if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
				self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
		}
		else
		{
			aborted_fire = true;
			return;
		}

		VectorCopy(forward, self->last_forward);
		VectorCopy(start, self->last_start);
	}
	else
	{
		VectorCopy(self->last_forward, forward);
		VectorCopy(self->last_start, start);
	}

	if (self->client->killer_yaw < (level.time - 0.3))
	{
		if (self->client->killer_yaw < (level.time - 0.5))
		{	// must have stopped firing, so need to restart wind-up
			self->client->machinegun_shots = 0;
		}

		self->client->killer_yaw = level.time;
		self->client->machinegun_shots++;
	}

	shots = self->client->machinegun_shots;
	if (shots > 3)
		shots = self->client->machinegun_shots = 3;

	// optimize, simulate more shots by increasing the damage, but still only firing one shot
	// chaingun is responsible for a LOT of cpu usage
// AJ changed constant (3) to cvar
	damage = sk_bot_chaingun_damage->value * shots;
	kick = 2;

	if (bot_is_quad)
		damage *= 4;
	if (bot_is_double)	//ScarFace
		damage *= 2;

//	for (i=0 ; i<shots ; i++)
//	{
		// get start / end positions
// AJ changed constants (*0.5!!) to cvars (hspread, vspread)
		fire_bullet (self, start, forward, damage, kick, sk_bot_chaingun_hspread->value, sk_bot_chaingun_hspread->value, MOD_CHAINGUN);
//	}

	if ((self->client->pers.inventory[self->client->ammo_index] -= shots) < 0)
	{
		self->client->pers.inventory[self->client->ammo_index] = 0;
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1));
	gi.multicast (self->s.origin, MULTICAST_PVS);

/*
#ifdef	_WIN32
	_ftime(&self->lastattack_time);
#else
	ftime(&self->lastattack_time);
#endif
*/
}

void botRailgun (edict_t *self)
{
	vec3_t		start, target;
	vec3_t		forward, right, ofs;
	int			damage, kick;
	// Knightmare added
	int			red=20, green=48, blue=176;
	qboolean	useColor=false;

	// CTF color override
	if ( (int)ctf->value && (int)ctf_railcolors->value && self->client )
	{
		useColor = true;
		if (self->client->resp.ctf_team == CTF_TEAM1) {
			red = 176;	green = 20;	blue = 20;
		}
		else if (self->client->resp.ctf_team == CTF_TEAM2) {
			red = 20;	green = 20;	blue = 176;
		}
		else if (self->client->resp.ctf_team == CTF_TEAM3) {
			red = 20;	green = 176;	blue = 20;
		}
	}
	// custom client color
	else if ( self->client && (self->client->pers.color1[3] != 0) )
	{
		useColor = true;
		red = self->client->pers.color1[0];
		green = self->client->pers.color1[1];
		blue = self->client->pers.color1[2];
	}

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8, 8, self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{
		float	dist, tf=0;

		dist = entdist(self, self->enemy);

		if (self->enemy->health > 0)
		{
			VectorCopy (self->enemy->s.origin, target);

//			if (skill->value <= 1)
			{	// trail the player's velocity
// AJ changed constant -0.2 to "0.0 - bot_railtime->value"
				VectorMA(target, 0.0 - bot_railtime->value, self->enemy->velocity, target);
			}

			target[2] += self->enemy->viewheight - 8;

			if (self->enemy->client && !self->enemy->bot_client)
				tf = (VectorLength(self->enemy->velocity) / 300) * 100;

			else if (!self->enemy->client) // ScarFace
				if ( (!strcmp(self->enemy->classname, "doppleganger")) && !self->enemy->bot_client)
						tf = (VectorLength(self->enemy->velocity) / 300) * 100;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		tf = 32;

		if (self->bot_stats->accuracy < 5)
		{
			tf += (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));

			else if (!self->enemy->client) //ScarFace
				if ( (!strcmp(self->enemy->classname, "doppleganger")) && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));
		}

		if (tf > 0)
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.1), target);

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

// AJ changed constant (100) to cvar
	damage	= sk_bot_railgun_damage->value;
	kick	= 200;

	if (bot_is_quad)
		damage *= 4;
	if (bot_is_double)	//ScarFace
		damage *= 2;

//	fire_rail (self, start, forward, damage, kick, false, 0, 0, 0);
	fire_rail (self, start, forward, damage, kick, useColor, red, green, blue);
	self->client->pers.inventory[self->client->ammo_index]--;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_RAILGUN);
	gi.multicast (self->s.origin, MULTICAST_PVS);

}

void botRocketLauncher (edict_t *self)
{
	vec3_t	start, target, end_trace;
	vec3_t	forward, right, ofs;
	vec3_t	oldorg, vel;
	int		damage;
	float	damage_radius;
	int		radius_damage;
	float	dist=0, tf;
	trace_t	trace;

	// fire at peak of jump
	if ((self->bot_stats->combat > 3) && !self->groundentity && (self->velocity[2] > 50))
	{
		aborted_fire = true;
		return;
	}

// AJ changed constant (100) to bot_rocket_damage, 20.0 to bot_rocket_damage2
	damage = sk_bot_rocket_damage->value + (int)(random() * sk_bot_rocket_damage2->value);
// AJ changed constant (120) to bot_rocket_rdamage
	radius_damage = sk_bot_rocket_rdamage->value;
// AJ changed constant (12) to bot_rocket_radius
	damage_radius = sk_bot_rocket_radius->value;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{
		dist = entdist(self, self->enemy);

		if ((skill->value > 1) && (self->enemy->health > 0) && ( (self->enemy->client && !self->enemy->bot_client)
				|| (!self->enemy->client && !strcmp(self->enemy->classname, "doppleganger") && !self->enemy->bot_client) )
			&& (dist > 64))
		{
			VectorCopy(self->enemy->velocity, vel);
			if (vel[2] > 0)
				vel[2] = 0;

			VectorMA (self->enemy->s.origin, (float) dist / 650, vel, target);
			target[2] += self->enemy->viewheight - 8;

			if (self->bot_stats->combat > 3)
			{	// aim towards the ground?
				trace_t tr;

				VectorCopy(target, end_trace);
				end_trace[2] -= 64;
				tr = gi.trace(target, NULL, NULL, end_trace, self->enemy, CONTENTS_SOLID);

				if (tr.fraction < 1)
				{
					vec3_t end, org;

					VectorCopy(tr.endpos, end);

					VectorCopy(self->s.origin, org);
					org[2] += self->viewheight;

					tr = gi.trace(org, NULL, NULL, end, self, CONTENTS_SOLID);

					if (tr.fraction == 1)
					{
						VectorCopy(end, target);
					}
				}
			}

		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));

			else if (!self->enemy->client) //ScarFace
				if ( (!strcmp(self->enemy->classname, "doppleganger")) && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));

			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	// check to make sure the rocket won't explode in our face

	if (self->enemy->client && !self->enemy->bot_client)
	{
		// move the enemy to the predicted position
		VectorCopy(self->enemy->s.origin, oldorg);
		VectorMA (self->enemy->s.origin, (float) dist / 650, self->enemy->velocity, self->enemy->s.origin);
		gi.linkentity(self->enemy);
	}
	else if (!self->enemy->client) //ScarFace
	{
		if ( (!strcmp(self->enemy->classname, "doppleganger")) && !self->enemy->bot_client)
		{
			// move the enemy to the predicted position
			VectorCopy(self->enemy->s.origin, oldorg);
			VectorMA (self->enemy->s.origin, (float) dist / 650, self->enemy->velocity, self->enemy->s.origin);
			gi.linkentity(self->enemy);
		}
	}
	VectorScale(forward, 130, end_trace);
	VectorAdd(start, end_trace, end_trace);
	trace = gi.trace(start, tv(-12,-12,-4), tv(12,12,4), end_trace, self, MASK_PLAYERSOLID);

	if (self->enemy->client && !self->enemy->bot_client)
	{
		// move the enemy back to their correct position
		VectorCopy(oldorg, self->enemy->s.origin);
		gi.linkentity(self->enemy);
	}
	else if (!self->enemy->client) //ScarFace
	{
		if ( (!strcmp(self->enemy->classname, "doppleganger")) && !self->enemy->bot_client)
		{
			// move the enemy back to their correct position
			VectorCopy(oldorg, self->enemy->s.origin);
			gi.linkentity(self->enemy);
		}
	}
	if (	(trace.fraction < 1)
		&&	(	(self->health > 15)
			 ||	(!trace.ent)))		// dangerous situation, only fire if almost dead and obstacle is another player
	{
		// walk backwards
		if (!M_walkmove(self, self->s.angles[YAW] + 180, BOT_RUN_SPEED * bot_frametime))
		{
			// FIXME: can't walk backwards, select a good close-range weapon
			botPickBestCloseWeapon(self);
		}

		aborted_fire = true;
		return;
	}

	if (bot_is_quad)
		damage *= 4;
	if (bot_is_double)	//ScarFace
		damage *= 2;

// AJ changed constant 650 for cvar sk_bot_rocket_speed->value
	fire_rocket (self, start, forward, damage, sk_bot_rocket_speed->value, damage_radius, radius_damage);
	self->client->pers.inventory[self->client->ammo_index]--;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (dist > 700)
	{	// check for a better long distance weapon
		botPickBestFarWeapon(self);
	}

}

void botGrenadeLauncher (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs, angles;
	int		damage;
	int		radius;
	float	dist=0, tf;

// AJ changed constant (120) to cvar
	damage = sk_bot_grenade_damage->value;
// AJ - changed algorithm for calculating grenade radius (cvar)
//	radius = damage+40;
	radius = sk_bot_grenade_radius->value;
// end AJ

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	dist = entdist(self, self->enemy);

	if (self->enemy && infront(self, self->enemy))
	{
		if ((self->enemy->health > 0) && (self->enemy->client && !self->enemy->bot_client) && (dist > 64))
		{
			VectorMA (self->enemy->s.origin, dist / 550, self->enemy->velocity, target);
			target[2] += self->enemy->viewheight - 8;
		}
		else if (!self->enemy->client) //ScarFace
		{
			if ( (!strcmp(self->enemy->classname, "doppleganger")) && (self->enemy->health > 0) && (!self->enemy->bot_client) && (dist > 64) )
			{
				VectorMA (self->enemy->s.origin, dist / 550, self->enemy->velocity, target);
				target[2] += self->enemy->viewheight - 8;
			}
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));

			else if (!self->enemy->client) //ScarFace
				if ( (!strcmp(self->enemy->classname, "doppleganger")) && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));

			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	if (bot_is_quad)
		damage *= 4;
	if (bot_is_double)	//ScarFace
		damage *= 2;

	vectoangles(forward, angles);

	// angle upwards a bit
	angles[PITCH] -= 15 * ((dist < 384) ? ((dist / 384) * 2) - 1: 1);
	AngleVectors(angles, forward, NULL, NULL);

//	fire_grenade (self, start, forward, damage, 600, 2.5, radius, false);
	fire_grenade (self, start, forward, damage, (int)sk_bot_grenade_speed->value, 2.5, radius, false);
	self->client->pers.inventory[self->client->ammo_index]--;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_GRENADE);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (dist > 700)
	{	// check for a better long distance weapon
		botPickBestFarWeapon(self);
	}
	else if (dist < radius)
	{
		botPickBestCloseWeapon(self);
	}
}

void botHyperblaster (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	float	dist=0, tf;
	int		damage, effect, color, muzzleflash;

// AJ - changed damage constant from 15 to a cvar
	damage = sk_bot_hyperblaster_damage->value;
	if (bot_is_quad)
		damage *= 4;
	if (bot_is_double)	//ScarFace
		damage *= 2;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{
		dist = entdist(self, self->enemy);

		if ((skill->value > 1) && (self->enemy->health > 0) && (self->enemy->client) && (dist > 64))
		{
			VectorMA (self->enemy->s.origin, dist/1000, self->enemy->velocity, target);
			target[2] += self->enemy->viewheight - 8;
		}
		else if (!self->enemy->client) //ScarFace
		{
			if ( (!strcmp(self->enemy->classname, "doppleganger")) && (skill->value > 1) && (self->enemy->health > 0) && (dist > 64) )
			{
				VectorMA (self->enemy->s.origin, dist/1000, self->enemy->velocity, target);
				target[2] += self->enemy->viewheight - 8;
			}
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
// AJ added the "1 -"... this resolves the hyperblaster accuracy issue
//				tf *= (VectorLength(self->enemy->velocity)/600);
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));

			else if (!self->enemy->client) //ScarFace
				if ( (!strcmp(self->enemy->classname, "doppleganger")) && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));

			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	// Knightmare- select color
	color = sk_hyperblaster_color->value;
	// hyperblaster_color could be any other value, so clamp it
	if ( (sk_hyperblaster_color->value < 2) || (sk_hyperblaster_color->value > 4) )
		color = BLASTER_ORANGE;
	// CTF color override
	if (ctf->value && ctf_blastercolors->value && self->client)
		color = (5 - self->client->resp.ctf_team);
#ifndef KMQUAKE2_ENGINE_MOD
	if (color == BLASTER_RED) color = BLASTER_ORANGE;
#endif

	if ((random() * 3) < 1)
	{
		// Knightmare- blaster colors
		if (color == BLASTER_GREEN)
			effect = (EF_HYPERBLASTER|EF_TRACKER);
		else if (color == BLASTER_BLUE)
			effect = EF_BLUEHYPERBLASTER;
		else if (color == BLASTER_RED)
			effect = EF_HYPERBLASTER|EF_IONRIPPER;
		else // standard orange
			effect = EF_HYPERBLASTER;
	}
	else
		effect = 0;

// AJ changed constant 1000 to cvar sk_bot_hyperblaster_speed->value
	fire_blaster (self, start, forward, damage, sk_bot_hyperblaster_speed->value, effect, true, color);
	self->client->pers.inventory[self->client->ammo_index]--;

	// Knightmare- blaster colors
	if (color == BLASTER_GREEN)
#ifdef KMQUAKE2_ENGINE_MOD
		muzzleflash = MZ_GREENHYPERBLASTER;
#else
		muzzleflash = MZ_HYPERBLASTER;
#endif
	else if (color == BLASTER_BLUE)
		muzzleflash = MZ_BLUEHYPERBLASTER;
#ifdef KMQUAKE2_ENGINE_MOD
	else if (color == BLASTER_RED)
		muzzleflash = MZ_REDHYPERBLASTER;
#endif
	else //standard orange
		muzzleflash = MZ_HYPERBLASTER;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (muzzleflash);
	gi.multicast (self->s.origin, MULTICAST_PVS);

}

void botBFG (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int		damage;
	float	damage_radius;
	float	dist=0, tf;

// AJ changed constant (500) to cvar
	damage = sk_bot_bfg_damage->value;
// AJ changed constant (1000) to cvar
	damage_radius = sk_bot_bfg_radius->value;
	if (bot_is_quad)
		damage *= 4;
	if (bot_is_double)	//ScarFace
		damage *= 2;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	dist = entdist(self, self->enemy);

	if (self->enemy && infront(self, self->enemy))
	{

		if ((self->enemy->health > 0) && (self->enemy->client && !self->enemy->bot_client) && (dist > 64))
		{
			VectorMA (self->enemy->s.origin, entdist(self, self->enemy) * (1/550), self->enemy->velocity, target);
			target[2] += self->enemy->viewheight - 8;

			if ((dist > 200) && self->enemy->groundentity)	// aim towards ground
				target[2] -= (4 * self->bot_stats->combat);
		}
		else if (!self->enemy->client) //ScarFace
		{
			if ( (!strcmp(self->enemy->classname, "doppleganger")) && (self->enemy->health > 0) && !self->enemy->bot_client && (dist > 64) )
			{
				VectorMA (self->enemy->s.origin, entdist(self, self->enemy) * (1/550), self->enemy->velocity, target);
				target[2] += self->enemy->viewheight - 8;
				if ((dist > 200) && self->enemy->groundentity)	// aim towards ground
					target[2] -= (4 * self->bot_stats->combat);
			}
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));

			else if (!self->enemy->client) //ScarFace
				if ( (!strcmp(self->enemy->classname, "doppleganger")) && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));

			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

//	fire_bfg (self, start, forward, damage, 400, damage_radius);
	fire_bfg (self, start, forward, damage, (int)sk_bot_bfg_speed->value, damage_radius);
	if ((self->client->pers.inventory[self->client->ammo_index] -= 60) < 0)
		self->client->pers.inventory[self->client->ammo_index] = 0;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_BFG);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (dist > 1000)
	{	// check for a better long distance weapon
		botPickBestFarWeapon(self);
	}
}

/*
===================
botPickBestWeapon

  called everytime a weapon/ammo is picked up, or ammo runs out
===================
*/
void	botPickBestWeapon(edict_t *self)
{
	gclient_t	*client;
	gitem_t		*oldweapon;

	client = self->client;

	oldweapon = client->pers.weapon;

	// check favourite weapon
	if ( client->pers.inventory[self->bot_stats->fav_weapon->tag]
		&&  client->pers.inventory[ITEM_INDEX(self->bot_stats->fav_weapon)] )
	{
botDebugPrint("%s picked up favourite weapon\n", self->client->pers.netname);
		client->newweapon = self->bot_stats->fav_weapon;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		GetBotFireForWeapon(self->bot_stats->fav_weapon, &self->bot_fire);

		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;

		if (client->newweapon == item_shotgun)
			self->fire_interval = FIRE_INTERVAL_SHOTGUN;
		else if (client->newweapon == item_supershotgun)
			self->fire_interval = FIRE_INTERVAL_SSHOTGUN;
		else if (client->newweapon == item_rocketlauncher)
			self->fire_interval = FIRE_INTERVAL_ROCKETLAUNCHER;
		else if (client->newweapon == item_grenadelauncher)
			self->fire_interval = FIRE_INTERVAL_GRENADELAUNCHER;
		else if (client->newweapon == item_railgun)
			self->fire_interval = FIRE_INTERVAL_RAILGUN;
		else if (client->newweapon == item_hyperblaster)
			self->fire_interval = FIRE_INTERVAL_HYPERBLASTER;
		else if (client->newweapon == item_chaingun)
			self->fire_interval = FIRE_INTERVAL_CHAINGUN;
		else if (client->newweapon == item_machinegun)
			self->fire_interval = FIRE_INTERVAL_MACHINEGUN;
		else if (client->newweapon == item_bfg10k)
			self->fire_interval = FIRE_INTERVAL_BFG;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = self->bot_stats->fav_weapon->tag;
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}

	// only use BFG first if not Quad Damage
//	if (client->quad_framenum < level.framenum)
//	{
		if ( client->pers.inventory[ITEM_INDEX(item_cells)]
			&&  client->pers.inventory[ITEM_INDEX(item_bfg10k)] )
		{
			client->newweapon = item_bfg10k;

			if (client->pers.weapon == client->newweapon)
				return;		// already using this weapon

			self->bot_fire = botBFG;
			self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
			self->fire_interval = FIRE_INTERVAL_BFG;

			if (CTFApplyHaste(self))
				self->fire_interval *= 0.5;
			if (client->quadfire_framenum > level.framenum)
				self->fire_interval *= 0.5;

			client->ammo_index = ITEM_INDEX(item_cells);
			client->pers.weapon = client->newweapon;

			self->client->pers.weapon = client->newweapon;
			goto found;
		}
//	}

	// do other weapons
	if ( client->pers.inventory[ITEM_INDEX(item_bullets)]
		&&  client->pers.inventory[ITEM_INDEX(item_chaingun)] )
	{
		client->newweapon = item_chaingun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botChaingun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_CHAINGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_bullets);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_cells)]
		&&  client->pers.inventory[ITEM_INDEX(item_hyperblaster)] )
	{
		client->newweapon = item_hyperblaster;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botHyperblaster;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_HYPERBLASTER;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_cells);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_rockets)]
		&&  client->pers.inventory[ITEM_INDEX(item_rocketlauncher)] )
	{
		client->newweapon = item_rocketlauncher;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botRocketLauncher;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_ROCKETLAUNCHER;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_rockets);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_slugs)]
		&&  client->pers.inventory[ITEM_INDEX(item_railgun)] )
	{
		client->newweapon = item_railgun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botRailgun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_RAILGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_slugs);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_bullets)]
		&&  client->pers.inventory[ITEM_INDEX(item_machinegun)] )
	{
		client->newweapon = item_machinegun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botMachineGun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_MACHINEGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_bullets);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
/*
	// if Quad Damage, then use BFG if it's available
	if (client->quad_framenum >= level.framenum)
	{
		if ( client->pers.inventory[ITEM_INDEX(item_cells)]
			&&  client->pers.inventory[ITEM_INDEX(item_bfg10k)] )
		{
			client->newweapon = item_bfg10k;

			if (client->pers.weapon == client->newweapon)
				return;		// already using this weapon

			self->bot_fire = botBFG;
			self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
			self->fire_interval = FIRE_INTERVAL_BFG;

			if (CTFApplyHaste(self))
				self->fire_interval *= 0.5;
			if (client->quadfire_framenum > level.framenum)
				self->fire_interval *= 0.5;

			client->ammo_index = ITEM_INDEX(item_cells);
			client->pers.weapon = client->newweapon;

			self->client->pers.weapon = client->newweapon;
			goto found;
		}
	}
*/
	if ( client->pers.inventory[ITEM_INDEX(item_shells)]
		&&  client->pers.inventory[ITEM_INDEX(item_supershotgun)] )
	{
		client->newweapon = item_supershotgun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botSuperShotgun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_SSHOTGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_shells);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_grenades)]
		&&  client->pers.inventory[ITEM_INDEX(item_grenadelauncher)] )
	{
		client->newweapon = item_grenadelauncher;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botGrenadeLauncher;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_GRENADELAUNCHER;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_grenades);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_shells)]
		&&  client->pers.inventory[ITEM_INDEX(item_shotgun)] )
	{
		client->newweapon = item_shotgun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botShotgun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_SHOTGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_shells);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}

	// must use blaster
// AJ check that the bot HAS the blaster...
	if ( client->pers.inventory[ITEM_INDEX(item_blaster)] )
	{
// end AJ
		client->newweapon = item_blaster;
		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botBlaster;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_BLASTER;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = 0;
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
// AJ
	}
// end AJ

found:

	if (oldweapon != client->pers.weapon)
		ShowGun(self);

};

int botHasWeaponForAmmo (gclient_t *client, gitem_t *item)
{
	switch (item->tag)
	{
		case (AMMO_SHELLS) :
		{
			return (client->pers.inventory[ITEM_INDEX(item_supershotgun)] ||
					client->pers.inventory[ITEM_INDEX(item_shotgun)]);
		}

		case (AMMO_ROCKETS) :
		{
			return (client->pers.inventory[ITEM_INDEX(item_rocketlauncher)]);
		}

		case (AMMO_CELLS) :
		{
			return (client->pers.inventory[ITEM_INDEX(item_hyperblaster)] ||
					client->pers.inventory[ITEM_INDEX(item_bfg10k)]);
		}

		case (AMMO_BULLETS) :
		{
			return (client->pers.inventory[ITEM_INDEX(item_chaingun)] ||
					client->pers.inventory[ITEM_INDEX(item_machinegun)]);
		}

		case (AMMO_SLUGS) :
		{
			return client->pers.inventory[ITEM_INDEX(item_railgun)];
		}

		case (AMMO_GRENADES) :
		{
			return (client->pers.inventory[ITEM_INDEX(item_grenadelauncher)]);
		}

		case (AMMO_MAGSLUG) :
		{
			return (client->pers.inventory[ITEM_INDEX(item_phalanx)]);
		}
		case (AMMO_FLECHETTES):
		{
			return (client->pers.inventory[ITEM_INDEX(item_trap)]);
		}
		case (AMMO_TESLA):
		{
			return (client->pers.inventory[ITEM_INDEX(item_trap)]);
		}
		case (AMMO_TRAP):
		{
			return (client->pers.inventory[ITEM_INDEX(item_trap)]);
		}
		case (AMMO_PROX):
		{
			return (client->pers.inventory[ITEM_INDEX(item_trap)]);
		}
		case (AMMO_DISRUPTOR):
		{
			return (client->pers.inventory[ITEM_INDEX(item_trap)]);
		}
		default :
		{
			gi.dprintf("botHasWeaponForAmmo: unkown ammo type - %i\n", item->ammo);
			return false;
		}

	}
}

int	ClientHasAnyWeapon(gclient_t	*client)
{
	if (client->pers.weapon != item_blaster)
		return true;

	if (client->pers.inventory[ITEM_INDEX(item_shotgun)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_supershotgun)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_machinegun)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_chaingun)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_grenadelauncher)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_rocketlauncher)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_railgun)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_hyperblaster)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_bfg10k)])
		return true;

	return false;
}

int	botCanPickupAmmo (gclient_t *client, gitem_t *item)
{
	int max, index;

	switch (item->tag)
	{
		case AMMO_BULLETS	: max = client->pers.max_bullets; break;
		case AMMO_SHELLS	: max = client->pers.max_shells; break;
		case AMMO_ROCKETS	: max = client->pers.max_rockets; break;
		case AMMO_GRENADES	: max = client->pers.max_grenades; break;
		case AMMO_CELLS		: max = client->pers.max_cells; break;
		case AMMO_SLUGS		: max = client->pers.max_slugs; break;
		default				: return false;
	}

//gi.dprintf("Max ammo set\n");

	index = ITEM_INDEX(item);

	if (client->pers.inventory[index] == max)
		return false;

//gi.dprintf("Can pickup ammo\n");

	return true;
}

void GetBotFireForWeapon(gitem_t	*weapon,	void (**bot_fire)(edict_t	*self))
{
	if (weapon == item_rocketlauncher)
		*bot_fire = botRocketLauncher;
	else if (weapon == item_chaingun)
		*bot_fire = botChaingun;
	else if (weapon == item_supershotgun)
		*bot_fire = botSuperShotgun;
	else if (weapon == item_grenadelauncher)
		*bot_fire = botGrenadeLauncher;
	else if (weapon == item_railgun)
		*bot_fire = botRailgun;
	else if (weapon == item_hyperblaster)
		*bot_fire = botHyperblaster;
	else if (weapon == item_bfg10k)
		*bot_fire = botBFG;
	else if (weapon == item_shotgun)
		*bot_fire = botShotgun;
	else if (weapon == item_machinegun)
		*bot_fire = botMachineGun;

}

/*
===================
botPickBestCloseWeapon

  called when close to enemy, don't use RL, GL, BFG
===================
*/
void	botPickBestCloseWeapon(edict_t *self)
{
	gclient_t	*client;
	gitem_t		*oldweapon;

	client = self->client;

	oldweapon = client->pers.weapon;

	if ( client->pers.inventory[ITEM_INDEX(item_bullets)]
		&&  client->pers.inventory[ITEM_INDEX(item_chaingun)] )
	{
		client->newweapon = item_chaingun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botChaingun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_CHAINGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_bullets);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_shells)]
		&&  client->pers.inventory[ITEM_INDEX(item_supershotgun)] )
	{
		client->newweapon = item_supershotgun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botSuperShotgun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_SSHOTGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_shells);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_cells)]
		&&  client->pers.inventory[ITEM_INDEX(item_hyperblaster)] )
	{
		client->newweapon = item_hyperblaster;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botHyperblaster;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_HYPERBLASTER;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_cells);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_bullets)]
		&&  client->pers.inventory[ITEM_INDEX(item_machinegun)] )
	{
		client->newweapon = item_machinegun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botMachineGun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_MACHINEGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_bullets);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_cells)]
		&&  client->pers.inventory[ITEM_INDEX(item_bfg10k)] )
	{
		client->newweapon = item_bfg10k;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botBFG;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_BFG;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_cells);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;

		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_slugs)]
		&&  client->pers.inventory[ITEM_INDEX(item_railgun)] )
	{
		client->newweapon = item_railgun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botRailgun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_RAILGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_slugs);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_grenades)]
		&&  client->pers.inventory[ITEM_INDEX(item_grenadelauncher)] )
	{
		client->newweapon = item_grenadelauncher;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botGrenadeLauncher;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_GRENADELAUNCHER;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_grenades);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_shells)]
		&&  client->pers.inventory[ITEM_INDEX(item_shotgun)] )
	{
		client->newweapon = item_shotgun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botShotgun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_SHOTGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_shells);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_rockets)]
		&&  client->pers.inventory[ITEM_INDEX(item_rocketlauncher)] )
	{
		client->newweapon = item_rocketlauncher;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botRocketLauncher;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_ROCKETLAUNCHER;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_rockets);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
		goto found;
	}

	// must use blaster
// AJ check to see if the bot HAS the blaster
	if ( client->pers.inventory[ITEM_INDEX(item_blaster)] )
	{
// end AJ
		client->newweapon = item_blaster;
		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botBlaster;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_BLASTER;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = 0;
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
// AJ
	}
// end AJ

found:

	if (oldweapon != client->pers.weapon)
		ShowGun(self);
};

/*
===================
botPickBestFarWeapon

  called when close to enemy, don't use RL, GL, BFG
===================
*/
void	botPickBestFarWeapon(edict_t *self)
{
	gclient_t	*client;
	gitem_t		*oldweapon;

	client = self->client;

	oldweapon = client->pers.weapon;

	if ( client->pers.inventory[ITEM_INDEX(item_bullets)]
		&&  client->pers.inventory[ITEM_INDEX(item_chaingun)] )
	{
		client->newweapon = item_chaingun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botChaingun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_CHAINGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_bullets);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;

		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_slugs)]
		&&  client->pers.inventory[ITEM_INDEX(item_railgun)] )
	{
		client->newweapon = item_railgun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botRailgun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_RAILGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_slugs);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;

		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_bullets)]
		&&  client->pers.inventory[ITEM_INDEX(item_machinegun)] )
	{
		client->newweapon = item_machinegun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botMachineGun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_MACHINEGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_bullets);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;

		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_cells)]
		&&  client->pers.inventory[ITEM_INDEX(item_hyperblaster)] )
	{
		client->newweapon = item_hyperblaster;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botHyperblaster;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_HYPERBLASTER;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_cells);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;

		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_cells)]
		&&  client->pers.inventory[ITEM_INDEX(item_bfg10k)] )
	{
		client->newweapon = item_bfg10k;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botBFG;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_BFG;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_cells);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;

		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_rockets)]
		&&  client->pers.inventory[ITEM_INDEX(item_rocketlauncher)] )
	{
		client->newweapon = item_rocketlauncher;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botRocketLauncher;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_ROCKETLAUNCHER;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_rockets);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;

		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_shells)]
		&&  client->pers.inventory[ITEM_INDEX(item_shotgun)] )
	{
		client->newweapon = item_shotgun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botShotgun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_SHOTGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_shells);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;

		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_shells)]
		&&  client->pers.inventory[ITEM_INDEX(item_supershotgun)] )
	{
		client->newweapon = item_supershotgun;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botSuperShotgun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_SSHOTGUN;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_shells);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;

		goto found;
	}
	if ( client->pers.inventory[ITEM_INDEX(item_grenades)]
		&&  client->pers.inventory[ITEM_INDEX(item_grenadelauncher)] )
	{
		client->newweapon = item_grenadelauncher;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botGrenadeLauncher;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_GRENADELAUNCHER;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = ITEM_INDEX(item_grenades);
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;

		goto found;
	}

	// must use blaster
// AJ added check to see if the bot HAS the blaster
	if ( client->pers.inventory[ITEM_INDEX(item_blaster)] )
	{
// end AJ
		client->newweapon = item_blaster;
		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		self->bot_fire = botBlaster;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_BLASTER;

		if (CTFApplyHaste(self))
			self->fire_interval *= 0.5;
		if (client->quadfire_framenum > level.framenum)
			self->fire_interval *= 0.5;

		client->ammo_index = 0;
		client->pers.weapon = client->newweapon;

		self->client->pers.weapon = client->newweapon;
// AJ
	}
// end AJ

found:

	if (oldweapon != client->pers.weapon)
		ShowGun(self);
};
