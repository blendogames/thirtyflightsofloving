/*
e_grapple.c
Rich 'Publius' Tollerton
Charles 'Myrkul' Kendrick

Code for handling the Expert offhand grappling hook
and for Expert Pogo, which is roughly an offhand
rocket jump with no damage.
*/

#include "g_local.h"
#include "e_grapple.h" // AJ

#define HOOK_TIME		4		// Number of seconds that hook can live
#define HOOK_SPEED		1900	// Velocity of the hook itself
#define THINK_TIME		0.3		// Time between hook thinks
#define HOOK_DAMAGE		10		// Damage done by hook
#define GRAPPLE_REFIRE	5		// Refire delay for grapple, in frames

// NOTE: 650 appears to be the fastest the player can be pulled,
// without jitter (or without anti-jitter hacks).
#define PULL_SPEED		420		// How fast the player a pulled 
								// once the hook is anchored

#define POGO_SPEED		500		// What velocity the player gets on a pogo
#define POGO_REFIRE		8		// Refire delay for pogo, in frames

// Ended_Grappling: Returns true if the client just stopped grappling.
qboolean Ended_Grappling (gclient_t *client)
{
	return (!(client->buttons & BUTTON_USE) && client->oldbuttons & BUTTON_USE);
}

// Is_Grappling: Returns true if the client is grappling at the moment.
qboolean Is_Grappling (gclient_t *client)
{
	return (client->hook == NULL) ? false : true;
}

/*
Called when the hook touches something.

If that something is a projectile: Ignore it.
If it is a player or monster: damage it and release
If it is a solid object: anchor to it and pull the player.
*/
void Grapple_Touch(edict_t *hook, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// Release if hitting its owner
	if (other == hook->owner)
		return;

	// Release if already released or has been voided via health
	// (this is a carryover from a QW bug..)
	if (!Is_Grappling(hook->owner->client) && hook->health == 0) {
		return;
	}

	hook->health = 0;

	// Whether to stick to sky
	// AJ changed exp_flags & EXPERT_SKY_HOOK to sk_hook_sky->value
	if (!sk_hook_sky->value) {
		if (surf && surf->flags & SURF_SKY)
		{
			Release_Grapple(hook);
			return;
		}
	}

	// ignore weapon projectiles (rockets, BFG blasts, &c)
	if (other != g_edicts && other->clipmask == MASK_SHOT)
		return;
	
	gi.sound(hook, CHAN_ITEM, gi.soundindex("world/land.wav"), 1, ATTN_NORM, 0);

	if (other != NULL) {
		// Do damage to those that need it
// AJ changed constant to HOOK_DAMAGE to cvar of the same name
		T_Damage(other, hook, hook->owner, hook->velocity, hook->s.origin, plane->normal, sk_hook_damage->value, 0, 0, MOD_GRAPPLE);
	}

	// The hook hit a hook/monster/maybe explobox. Release.
	if (other != g_edicts && other->health && other->solid == SOLID_BBOX) {
		Release_Grapple(hook);
		return;
	}

	if (other != g_edicts && other->inuse &&
		(other->movetype == MOVETYPE_PUSH || other->movetype == MOVETYPE_STOP))
	{
		// Use a spare edict field on the object to store the hook in
		other->mynoise2 = hook;

		// Save what is being anchored to match velocities
		hook->owner->client->hook_touch = other;
		hook->enemy = other;
		hook->groundentity = NULL;

		hook->flags |= FL_TEAMSLAVE;
		
		// Turn off client prediction during the pull
		// Note: does not appear to be an improvement over
		// prediction with no gravity.
		//hook->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	}

	VectorClear(hook->velocity);
	VectorClear(hook->avelocity);
	hook->solid = SOLID_NOT;
	hook->touch = NULL;
	hook->movetype = MOVETYPE_NONE;

	// Sets how long a grapple can be anchored
	// changed constant HOOK_TIME to cvar of a similar name
	hook->delay = level.time + sk_hook_maxtime->value;

	// Show that hook is being pulled. Pull_Grapple handles movement from
	// Client_Think in p_client.c
	hook->owner->client->on_hook = true;

	// The server needs to be told that the player's off the ground
	hook->owner->groundentity = NULL;

	// do initial pull
	Pull_Grapple(hook->owner);

/*
	// Experimental: faster hooking if the player is already moving in the
	// direction of the hook
	{
	vec3_t hookPull;
	float directionMatch;

	// unit vector along hook's pull
	VectorSubtract(hook->s.origin, hook->owner->s.origin, hookPull);
	VectorNormalize(hookPull);

	// vector of hook's pull
	VectorScale(hookPull, PULL_SPEED, hookPull);

	directionMatch = DotProduct(hookPull, hook->owner->velocity) / (PULL_SPEED * PULL_SPEED);

	if (hook->owner->scale == 0) {
		hook->owner->scale = 1;
	}

	if (directionMatch > 0.55) {
		hook->owner->scale += 0.2;
		if (hook->owner->scale > 1.5) {
			hook->owner->scale = 1.5;
		}
	} else {
		hook->owner->scale = 1;
	}
	gi.bprintf(PRINT_HIGH, "DirectionMatch is %.1f, scale is %.1f\n", directionMatch, hook->owner->scale);
	}
*/

}

// Think_Grapple: Run any sounds, update any models, etc.
void Think_Grapple(edict_t *hook)
{
	// Hook has been out for too long. Release
	if (level.time > hook->delay)
		hook->prethink = Release_Grapple;
	else
	{
		if (hook->owner->client->hook_touch) {
			edict_t *obj = hook->owner->client->hook_touch;

			if (obj == g_edicts)
			{
				Release_Grapple(hook);
				return;
			}

			// Release if the edict was freed
			if (obj->inuse == false) {
				Release_Grapple(hook);
				return;
			}

			// Release if the object is dead
			if (obj->deadflag == DEAD_DEAD)
			{
				Release_Grapple(hook);
				return;
			}

			// Movement code is handled with the MOVETYPE_PUSH stuff in g_phys.c

			// Do damage to the touched entity - we're certain it's not a player anyway..
			// AJ: changed the constant HOOK_DAMAGE to the cvar of the same name
			T_Damage(obj, hook, hook->owner, hook->velocity, hook->s.origin, vec3_origin, sk_hook_damage->value, 0, 0, MOD_GRAPPLE);
		}

		hook->nextthink += THINK_TIME;
	}
}

// Make_Hook: Create and set up the hook object
void Make_Hook (edict_t *ent)
{
	edict_t *hook;
	vec3_t forward, right, start, offset;

	// Make the hook entity
	hook = G_Spawn();

	AngleVectors(ent->client->v_angle, forward, right, NULL);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent, ent->s.origin, offset, forward, right, start);	// Knightmare- changed parms for aimfix

	VectorCopy(start, hook->s.origin);
	VectorCopy(forward, hook->movedir);
	vectoangles(forward, hook->s.angles);
	// AJ changed constant HOOK_SPEED to the cvar of the same name
	VectorScale(forward, sk_hook_speed->value, hook->velocity);
	VectorSet(hook->avelocity, 0, 0, 500);

	hook->classname = "hook";
	hook->movetype = MOVETYPE_FLYMISSILE;
	hook->clipmask = MASK_SHOT;
	hook->solid = SOLID_BBOX;
	// no misprediction on hit
	hook->svflags |= SVF_DEADMONSTER;
	hook->s.effects = EF_GREENGIB;
	hook->s.renderfx = RF_FULLBRIGHT;
	VectorClear (hook->mins);
	VectorClear (hook->maxs);

	// In CTF, use Threewave grapple.  Otherwise, use activated grenade model
	// FIXME change to Expert custom hook model when auto-downloading is ready
// AJ changed to controlled via a CVAR
		hook->s.modelindex = gi.modelindex (grapple_mdl->string);
// end AJ

	hook->owner = ent;
	hook->touch = Grapple_Touch;

	// Delay is used for when the hook is automatically released.
	// Think is used for sound/model updates
	// AJ: changed the constant HOOK_TIME to the cvar of a similar name
	hook->delay = level.time + sk_hook_maxtime->value;
	hook->nextthink = level.time;
	hook->think = Think_Grapple;
	
	// Code to check if the hook has already touched something.
	// (QW had a double-touch bug; assuming Q2 does too)
	hook->health = 100;

	// So that doors and others don't try to blow the hook up
	hook->svflags = SVF_MONSTER;
	
	ent->client->hook = hook;
	gi.linkentity(hook);
}

// Throw_Grapple: Initially starts the grappling hook
void Throw_Grapple (edict_t *player)
{
	vec3_t		forward, traceTo;
	trace_t		trace;

	// Refire delay, for both pogo and grapple
	if (player->client->pogo_time > level.framenum) {
		return;
	}

	// Expert Pogo: like a rocket jump, but separate from weapons,
	// with the same control mechanism as the Expert Hook
	// AJ changed (expflags & EXPERT_POGO) to (0) as I dont like/want it
	if (0) {

		// the point along the player's view angle, POGO_SPEED away
		AngleVectors (player->client->v_angle, forward, NULL, NULL);
		VectorMA(player->s.origin, POGO_SPEED, forward, traceTo);

		// trace to it
		trace = gi.trace(player->s.origin, NULL, NULL, traceTo, player, MASK_PLAYERSOLID|MASK_WATER);

		// pogo gives velocity in the reverse of facing direction
		VectorMA(forward, -2, forward, forward);

		// thrust proportional to distance to wall
		VectorMA(player->velocity, (1 - trace.fraction) * POGO_SPEED, forward, player->velocity);

		// underwater explosion sound
		// FIXME: better sound?
		gi.sound(player, CHAN_ITEM, gi.soundindex("weapons/xpld_wat.wav"), 1, ATTN_NORM, 0);

		// Refire delay
		player->client->pogo_time = level.framenum + POGO_REFIRE;
		return;

	}

	if (player->client->hook) {
		// player already hooking
		return;
	}

	// FIXME: Possibly change channel
	gi.sound(player, CHAN_ITEM, gi.soundindex("plats/pt1_mid.wav"), 0.5, ATTN_NORM, 0);

	// Mask off BUTTON_USE so that this function isn't called twice
	//player->client->latched_buttons ^= BUTTON_USE;

	player->client->hook_touch = NULL;
	
	Make_Hook(player);

	// Refire delay
	player->client->pogo_time = level.framenum + GRAPPLE_REFIRE;

//AJ - Eraser: record this position, so we drop a grapple node here, rather than where the player is when they leave the ground
	if (!player->bot_client)
		VectorCopy(player->s.origin, player->animate_org);
// end AJ
}

// Release_Grapple: Delete hook and perform any sounds/etc needed
void Release_Grapple (edict_t *hook)
{
	edict_t *owner = hook->owner;
	gclient_t *client = hook->owner->client;
	edict_t *link = hook->teamchain;

	client->on_hook = false;
	client->hook_touch = NULL;

	// If hook is already NULL, then the hook was already released,
	// so ignore this attempt to release 
	if (client->hook != NULL) {
		client->hook = NULL;
		gi.sound(owner, CHAN_ITEM, gi.soundindex("misc/menu3.wav"), 1, ATTN_NORM, 0);
		VectorClear(client->oldvelocity);

		hook->think = NULL;

		// If connected to something, take the hook out of that something's teamchain
		if (hook->enemy) {
//			hook->enemy->teamchain = link;
			hook->enemy->mynoise2 = NULL;
		}

		// Turn prediction back on for the player
		// Note: does not appear to be an improvement over
		// prediction with no gravity.
		//client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

		G_FreeEdict(hook);
	}
}

// Pull_Grapple: Pulls player to hook. Called from ClientThink
void Pull_Grapple (edict_t *player)
{
	vec3_t hookDir;
	vec_t length;

	// unit vector in the direction of the hook from the player
	VectorSubtract(player->client->hook->s.origin, player->s.origin, hookDir); //ScarFace- this was where the the hook and teleport crash happened
	length = VectorNormalize(hookDir);

	// make the player's velocity a vector toward the hook
// AJ: changed constant PULL_SPEED to cvar of the same name
	VectorScale(hookDir, /*player->scale * */ sk_hook_pullspeed->value, player->velocity);
	VectorCopy(hookDir, player->movedir);

/*
	// Experimental: hook pulls momentarily and then releases.  zero-g?
	VectorMA(player->velocity, PULL_SPEED, hookDir, player->velocity); 
	VectorCopy(player->velocity, player->movedir);
	VectorNormalize(player->movedir);

	Release_Grapple(player->client->hook);
*/
}



void Cmd_Hook_f (edict_t *ent)
{
	if (!sk_hook_offhand->value || ent->deadflag || ent->client->hook_frame > level.framenum)
		return;

	Throw_Grapple (ent);
}