//====================================================================
//
// SWINGING GRAPPLING HOOK for Quake2
// by: Perecli Manole AKA Bort
//
//====================================================================
// Aside from this new file, the following are the modifications
// done to id's original source files:
//--------------------------------------------------------------------
// File: g_cmds.c
// Location: on top after the #includes
// Added: void Cmd_FunHook_f (edict_t *ent);
//--------------------------------------------------------------------
// File: g_cmds.c
// Procedure: ClientCommand
// Location: after line "if (level.intermissiontime) return;"
// Added: if (Q_stricmp (cmd, "funhook") == 0)
//			Cmd_FunHook_f (ent);
//        else
//--------------------------------------------------------------------
// File: p_view.c
// Procedure: P_FallingDamage
// Location: after line "if (ent->waterlevel == 3) return;"
// Added: if (!(ent->client->ps.pmove.pm_flags & PMF_ON_GROUND))
//          return;
//--------------------------------------------------------------------
// File: g_local.h
// Structure: gclient_s 
// Location: after line "weaponstate_t weaponstate;"
// Added: int hookstate;
//        int hook_vampire_time;
//--------------------------------------------------------------------
// File: g_local.h
// Location: Existing "means of death" messages
// Added: 3 new #defines
//--------------------------------------------------------------------
// File: g_local.h
// Location: "means of death" messages (MOD_*)
// Added: 3 new #defines
//--------------------------------------------------------------------
// File: g_local.h
// Location: cvar prototypes
// Added: 12 new cvar prototypes (hook_*)
//--------------------------------------------------------------------
// File: q_shared.h
// Location: sound channel definitions
// Added: #define CHAN_HOOK 5
//--------------------------------------------------------------------

#include "g_local.h"


// edict->hookstate bit constants
#define HOOK_ON		0x00000001 // set if hook command is active
#define HOOK_IN		0x00000002 // set if hook has attached
#define SHRINK_ON	0x00000004 // set if shrink chain is active 
#define GROW_ON		0x00000008 // set if grow chain is active

// edict->sounds constants
#define MOTOR_OFF	0	   // motor sound has not been triggered
#define MOTOR_START	1	   // motor start sound has been triggered
#define MOTOR_ON	2	   // motor running sound has been triggered


// this is the same as function P_ProjectSource in p_weapons.c except it 
// projects the offset distance in reverse since hook is launched with 
// player's free hand
void P_ProjectSource_Reverse (edict_t *client_ent, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	gclient_t	*client = NULL;
	vec3_t		_distance;

	if ( !client_ent || !client_ent->client )
		return;

	client = client_ent->client;

	VectorCopy (distance, _distance);
	if (client->pers.hand == RIGHT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;

	G_ProjectSource (point, _distance, forward, right, result);

	// Yamagi Q2/Berserker: fix - now the projectile hits exactly where the scope is pointing.
	if (g_aimfix->value)
	{
		vec3_t		start, end, rangeVec, forward2;
		vec_t		range, featherFrac;
		trace_t		tr;

		VectorSet (start, client_ent->s.origin[0], client_ent->s.origin[1], client_ent->s.origin[2] + client_ent->viewheight);
		VectorMA (start, WORLD_SIZE, forward, end);

		tr = gi.trace(start, NULL, NULL, end, client_ent, MASK_SHOT);
		// Knightmare- check against minimum range for aimfix so we can still shoot around corners
		VectorSubtract (start, tr.endpos, rangeVec);
		range = VectorLength(rangeVec);
		if ( (tr.fraction < 1) && (range >= g_aimfix_min_dist->value) )
		{
			if (range < (g_aimfix_min_dist->value + g_aimfix_taper_dist->value) )
			{	// within feathering range
				featherFrac = (range - g_aimfix_min_dist->value) / g_aimfix_taper_dist->value;
				VectorSubtract (tr.endpos, result, forward2);
				VectorNormalize (forward2);
				VectorScale (forward2, featherFrac, forward2);
				VectorScale (forward, 1.0f - featherFrac, forward);
				VectorAdd (forward, forward2, forward);
				VectorNormalize (forward);
			}
			else {	// regular aimfix
				VectorSubtract (tr.endpos, result, forward);
				VectorNormalize (forward);
			}
		}
	}
}


void DropHook (edict_t *ent)
{
	// remove all hook flags
	ent->owner->client->hookstate = 0;

	gi.sound (ent->owner, CHAN_HOOK, gi.soundindex("hook/retract.wav"), 1, ATTN_IDLE, 0);

	// removes hook
	G_FreeEdict (ent);
}


void MaintainLinks (edict_t *ent)
{
	float  multiplier;	 // prediction multiplier
	vec3_t pred_hookpos;	 // predicted future hook origin
	vec3_t norm_hookvel;	 // normalized hook velocity
	vec3_t offset, start;
	vec3_t forward, right;
	vec3_t chainvec;	 // vector of the chain 
	vec3_t norm_chainvec;	 // vector of chain with distance of 1


	// predicts hook's future position since chain links fall behind
	multiplier = VectorLength(ent->velocity) / 22; 
	VectorNormalize2 (ent->velocity, norm_hookvel); 
	VectorMA (ent->s.origin, multiplier, norm_hookvel, pred_hookpos);

	// derive start point of chain
	AngleVectors (ent->owner->client->v_angle, forward, right, NULL);
	VectorSet (offset, 8, 8, ent->owner->viewheight-8);
	P_ProjectSource_Reverse (ent->owner, ent->owner->s.origin, offset, forward, right, start);	// Knightmare- changed parms for aimfix

	// get info about chain
	_VectorSubtract (pred_hookpos,start,chainvec);
	VectorNormalize2 (chainvec, norm_chainvec);

	// shorten ends of chain
	VectorMA (start, 10, norm_chainvec, start);
	VectorMA (pred_hookpos, -20, norm_chainvec, pred_hookpos);

	// create temp entity chain
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (ent - g_edicts);
	gi.WritePosition (pred_hookpos);
	gi.WritePosition (start);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
}


void HookBehavior (edict_t *ent)
{
	vec3_t offset, start;
	vec3_t forward, right;

	qboolean chain_moving;
	vec3_t chainvec;   // chain's vector
	float  chainlen;   // length of extended chain
	vec3_t velpart;    // player's velocity component moving to or away from hook
	float f1, f2;	     // restrainment forces


	// decide when to disconnect hook
	if ((!(ent->owner->client->hookstate & HOOK_ON)) ||
		(ent->enemy->solid == SOLID_NOT)             || 
		(ent->owner->deadflag)                       ||
		(ent->owner->s.event == EV_PLAYER_TELEPORT))
	{
		DropHook(ent);
		return;
	}

	// gives hook same velocity as the entity it is stuck in
	if (ent->enemy->client)
	{
		if ((ent->enemy->deadflag) || (ent->enemy->s.event == EV_PLAYER_TELEPORT))
		{
			DropHook(ent);
			return;
		}

		VectorCopy (ent->enemy->s.origin, ent->s.origin);
		ent->s.origin[2] += 8.0;
	}
	else {
		VectorCopy (ent->enemy->velocity, ent->velocity);
	}

	// chain sizing 
	chain_moving = false;

	// grow the length of the chain
	if ((ent->owner->client->hookstate & GROW_ON) && (ent->angle < sk_hook_max_length->value))
	{
		ent->angle += sk_hook_pull_speed->value;
		if (ent->angle > sk_hook_max_length->value) 
			ent->angle = sk_hook_max_length->value;
		chain_moving = true;
	}

	// shrink the length of the chain
	if ((ent->owner->client->hookstate & SHRINK_ON) && 
		(ent->angle > sk_hook_min_length->value))
	{
		ent->angle -= sk_hook_pull_speed->value;
		if (ent->angle < sk_hook_min_length->value) 
			ent->angle = sk_hook_min_length->value;
		chain_moving = true;
	}

	// determine sound play if climbing or sliding
	if (chain_moving)
	{
		if (ent->sounds == MOTOR_OFF) {
			// play start of climb sound
			gi.sound (ent->owner, CHAN_HOOK, gi.soundindex("hook/motor1.wav"), 1, 
				ATTN_IDLE, 0);
			ent->sounds = MOTOR_START;

		}
		else if (ent->sounds == MOTOR_START) {
			// play repetitive climb sound
			gi.sound (ent->owner, CHAN_HOOK, gi.soundindex("hook/motor2.wav"), 1, 
				ATTN_IDLE, 0);
			ent->sounds = MOTOR_ON;
		}
	}
	else if (ent->sounds != MOTOR_OFF) {
		gi.sound (ent->owner, CHAN_HOOK, gi.soundindex("hook/motor3.wav"), 1, 
			ATTN_IDLE, 0);
		ent->sounds = MOTOR_OFF;
	}

	// chain physics

	// derive start point of chain
	AngleVectors (ent->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->owner->viewheight-8);
	P_ProjectSource_Reverse (ent->owner, ent->owner->s.origin, offset, forward, right, start);	// Knightmare- changed parms for aimfix

	// get info about chain
	_VectorSubtract (ent->s.origin, start, chainvec);
	chainlen = VectorLength (chainvec);

	// if player's location is beyond the chain's reach
	if (chainlen > ent->angle)
	{	 

		// determine player's velocity component of chain vector
		VectorScale (chainvec, _DotProduct (ent->owner->velocity, chainvec) / 
			_DotProduct (chainvec, chainvec), velpart);

		// restrainment default force 
		f2 = (chainlen - ent->angle) * 5;

		// if player's velocity heading is away from the hook
		if (_DotProduct (ent->owner->velocity, chainvec) < 0)
		{

			// if chain has streched for 25 units
			if (chainlen > ent->angle + 25) {
				// remove player's velocity component moving away from hook
				_VectorSubtract(ent->owner->velocity, velpart, ent->owner->velocity);
			}
			f1 = f2;
		}
		else {
			// if player's velocity heading is towards the hook	      
			if (VectorLength (velpart) < f2)
				f1 = f2 - VectorLength (velpart);
			else		
				f1 = 0;
		}
	}
	else {
		f1 = 0;
	}

	// applys chain restrainment 
	VectorNormalize (chainvec);

	if (ent->enemy->client)
	{
		if (ent->owner->groundentity)
		{
			VectorMA (ent->enemy->velocity, -f1, chainvec, ent->enemy->velocity);
		}
		else
		{
			f1 /= 2.0;

			VectorMA (ent->enemy->velocity, -f1, chainvec,
				ent->enemy->velocity);
			VectorMA (ent->owner->velocity,  f1, chainvec,
				ent->owner->velocity);
		}

		// vampire hook stuff
		if (((level.time - ent->owner->client->hook_vampire_time) >= 1.0) &&  sk_hook_vampirism->value)
		{
				ent->owner->client->hook_vampire_time = level.time;
				if (sk_hook_vampirism->value > 0)
				{
					ent->owner->health += 
						sk_hook_vampirism->value * sk_hook_vampire_ratio->value;

					if ((ent->enemy->health -= sk_hook_vampirism->value) <= 0)
					{
						T_Damage (ent->enemy, ent, ent->owner, ent->velocity, 
							ent->s.origin, chainvec, sk_hook_vampirism->value, 0, 
							DAMAGE_NO_KNOCKBACK, MOD_HOOK_VAMPIRE1);
					}
				}
				else
				{
					ent->enemy->health -= sk_hook_vampirism->value * sk_hook_vampire_ratio->value;

					if ((ent->owner->health += sk_hook_vampirism->value) <= 0) {

						T_Damage (ent->owner, ent, ent->enemy, ent->velocity, 
							ent->s.origin, chainvec, -(sk_hook_vampirism->value), 0, 
							DAMAGE_NO_KNOCKBACK, MOD_HOOK_VAMPIRE2);
					}
				}
		}
	}
	else {
		VectorMA (ent->owner->velocity, f1, chainvec, ent->owner->velocity);
	}

	MaintainLinks (ent);

	// prep for next think
	ent->nextthink = level.time + FRAMETIME;
}


void HookTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t offset, start;
	vec3_t forward, right;
	vec3_t chainvec;	  // chain's vector


	// derive start point of chain
	AngleVectors (ent->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->owner->viewheight-8);
	P_ProjectSource_Reverse (ent->owner, ent->owner->s.origin, offset, forward, right, start);	// Knightmare- changed parms for aimfix
  
	// member angle is used to store the length of the chain
	_VectorSubtract(ent->s.origin,start,chainvec);
	ent->angle = VectorLength (chainvec);	

	// don't attach hook to sky
	if ( !sk_hook_sky->value && (surf && (surf->flags & SURF_SKY)) ) {
		DropHook(ent);
		return;
	}

  // inflict damage on damageable items
	if (other->takedamage) {
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, 
				plane->normal, ent->dmg, 100, 0, MOD_HOOK_DAMAGE1);
	}

	if ((other->solid == SOLID_BBOX))
	{
		if ((other->svflags & SVF_MONSTER) || (other->client)) 
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex("hook/smack.wav"), 1, ATTN_IDLE, 0);

			ent->owner->client->hook_vampire_time = level.time;
      
			if (other->client && sk_hook_messages->value && sk_hook_players->value)
			{
//				if (!other->bot_client) 
//					gi.centerprintf(other, "%s has hooked you!", ent->owner->client->pers.netname);
//				if (!ent->bot_client) 
//					gi.centerprintf(ent->owner, "You've hooked %s!", other->client->pers.netname);
			}
		}
    
		if (!(other->client && sk_hook_players->value)) 
		{
			DropHook(ent);
			return;
		}
	}
	
	if (other->client) {
		ent->solid = SOLID_NOT; // so that it does not interfere with 
								// other's movements
	}

	if (other->solid == SOLID_BSP)
	{
		// create puff of smoke
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SHOTGUN);
		gi.WritePosition (ent->s.origin);

		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);

		gi.multicast (ent->s.origin, MULTICAST_PVS);
		gi.sound (ent, CHAN_VOICE, gi.soundindex("hook/hit.wav"), 1, ATTN_IDLE, 0);
		VectorClear (ent->avelocity);
	}
	else if (other->solid == SOLID_TRIGGER) {
		// debugging line; don't know if this will ever happen 
		gi.cprintf (ent->owner, PRINT_HIGH, "Hook touched a SOLID_TRIGGER\n");
	}
	
  // hook gets the same velocity as the item it attached to
  VectorCopy (other->velocity,ent->velocity);

  // flags hook as being attached to something
  ent->owner->client->hookstate |= HOOK_IN;
  
  ent->enemy = other;
  ent->touch = NULL;
  ent->think = HookBehavior;
  ent->nextthink = level.time + FRAMETIME;

}


void HookAirborne (edict_t *ent)
{
	vec3_t	chainvec;	// chain's vector
	float	chainlen;	// length of extended chain

	edict_t	*target = NULL; // the homee ;)
	edict_t	*blip = NULL;   // potential homee
	vec3_t	targetdir, blipdir;
	vec_t	speed;

	// get info about chain
	_VectorSubtract (ent->s.origin, ent->owner->s.origin, chainvec);
	chainlen = VectorLength (chainvec);

	// check to see if we're at the end of our rope
	if ( (!(ent->owner->client->hookstate & HOOK_ON)) || (chainlen > sk_hook_max_length->value) )
	{
		DropHook (ent);
		return;
	}

	MaintainLinks (ent);	

	if (sk_hook_is_homing->value && sk_hook_players->value) 
	{

		// make the hook "homing"
		while ((blip = findradius(blip, ent->s.origin, sk_hook_homing_radius->value)) != NULL) 
		{
			if (!(blip->svflags & SVF_MONSTER) && !blip->client)
				continue;
			if (blip == ent->owner)
				continue;
			if (!blip->takedamage)
				continue;
			if (blip->health <= 0)
				continue;
			if (!visible(ent, blip))
				continue;
			if (!infront(ent, blip))
				continue;
			if (!infront(ent->owner, blip))
				continue;

			VectorSubtract (blip->s.origin, ent->s.origin, blipdir);
			blipdir[2] += 16.0;

			if ((target == NULL) || 
				(VectorLength(blipdir) < VectorLength(targetdir))) 
			{
				target = blip;
				VectorCopy (blipdir, targetdir);
			}
		}


		if (target != NULL) 
		{
			VectorNormalize (targetdir);
			VectorScale (targetdir, sk_hook_homing_factor->value, targetdir);
			VectorAdd (targetdir, ent->movedir, targetdir);
			VectorNormalize (targetdir);
			VectorCopy (targetdir, ent->movedir);
			vectoangles (targetdir, ent->s.angles);
			speed = VectorLength(ent->velocity);
			VectorScale (targetdir, speed, ent->velocity);
		}

	}
	// end homing hook

	ent->nextthink = level.time + FRAMETIME;
}


void FireHook (edict_t *ent)
{
	edict_t *newhook;
	vec3_t   offset, start;
	vec3_t   forward, right;
	int	   damage;

	// determine the damage the hook will inflict
	damage = 10;
	if (ent->client->quad_framenum > level.framenum)
		damage *= 4;

	// derive point of hook origin
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource_Reverse (ent, ent->s.origin, offset, forward, right, start);	// Knightmare- changed parms for aimfix

	// spawn hook
	newhook = G_Spawn();
	VectorCopy (start, newhook->s.origin);
	VectorCopy (forward, newhook->movedir);
	vectoangles (forward, newhook->s.angles);
	VectorScale (forward, sk_hook_fire_speed->value, newhook->velocity);
	VectorSet(newhook->avelocity,0,0,-800);
	newhook->movetype = MOVETYPE_FLYMISSILE;
	newhook->clipmask = MASK_SHOT;
	newhook->solid = SOLID_BBOX;
	VectorClear (newhook->mins);
	VectorClear (newhook->maxs);
	newhook->s.modelindex = gi.modelindex ("models/items/hook/tris.md2");
	newhook->owner = ent;
	newhook->dmg = damage;

	// keeps track of motor chain sound played 
	newhook->sounds = 0;   

	// play hook launching sound
	gi.sound (ent, CHAN_HOOK, gi.soundindex ("medic/medatck2.wav"), 1, 
		ATTN_IDLE, 0);

	// specify actions to follow 
	newhook->touch = HookTouch;
	newhook->think = HookAirborne;
	newhook->nextthink = level.time + FRAMETIME;

	gi.linkentity (newhook);
}


void Cmd_FunHook_f (edict_t *ent)
{
	char *s;
	int *hookstate;

	// get the first hook argument
	s = gi.argv(1);

	// create intermediate value
	hookstate = &ent->client->hookstate;

	if ((!(*hookstate & HOOK_ON)) && (Q_stricmp(s, "action") == 0)) {
		// flags hook as being active 
		*hookstate = HOOK_ON;    
		FireHook (ent);
		return;
	}

	if  (*hookstate & HOOK_ON) {

		// release hook	
		if (Q_stricmp(s, "action") == 0) {
			*hookstate = 0;
			return;
		}

		// deactivate chain growth or shrink
		if (Q_stricmp(s, "stop") == 0) {
			*hookstate -= *hookstate & (GROW_ON | SHRINK_ON);
			return;
		}

		// activate chain growth
		if (Q_stricmp(s, "grow") == 0) {
			*hookstate |= GROW_ON;
			*hookstate -= *hookstate & SHRINK_ON;
			return;
		}

		// activate chain shrinking
		if (Q_stricmp(s, "shrink") == 0) {

			*hookstate |= SHRINK_ON;		
			*hookstate -= *hookstate & GROW_ON;
		}
	}
}
