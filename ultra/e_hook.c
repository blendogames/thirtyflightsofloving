#include "g_local.h"
#include "m_player.h"

#include "e_hook.h"

//void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent, qboolean altfire));



// Function name	: hook_laser_think
// Description	    : move the two ends of the laser beam to the proper positions
// Return type		: void 
// Argument         : edict_t *self
void hook_laser_think (edict_t *self)
{
	vec3_t	forward, right, offset, start;
	
	// stupid check for NULL pointers ...
	if (!self->owner || !self->owner->owner || !self->owner->owner->client) {
		// ok, something's screwy
		G_FreeEdict(self);		// go away
		return;	
	}

	// put start position into start
	AngleVectors (self->owner->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, self->owner->owner->viewheight-8);
	P_ProjectSource (self->owner->owner, self->owner->owner->s.origin, offset, forward, right, start);	// Knightmare- changed parms for aimfix

	// move the two ends
//	gi.unlinkentity(self);
	VectorCopy (start, self->s.origin);
	VectorCopy (self->owner->s.origin, self->s.old_origin);
	gi.linkentity(self);

	// set up to go again
	self->nextthink = level.time + FRAMETIME;
	return;
}



// Function name	: *hook_laser_start
// Description	    : create a laser and return a pointer to it
// Return type		: edict_t 
// Argument         : edict_t *ent
edict_t *abandon_hook_laser_start (edict_t *ent)
{
	edict_t *self;
	
	char		*info;
	
	int			randyhook;
	
	self = G_Spawn();
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1;			// must be non-zero
	self->owner = ent;

	// set the beam diameter
	self->s.frame = 2;


	info = Info_ValueForKey (ent->owner->client->pers.userinfo, "hook_color");
	//set the color
	//gi.dprintf("Hook color selected it \"%s\".\n",info);
	
	randyhook = random() * 4294967295;

	// ADJ CTF team colours on the hook
	if (ctf->value)
	{
		switch (ent->owner->client->resp.ctf_team)
		{
		case CTF_TEAM1:
			self->s.skinnum = 0xf2f2f0f0; // red
			break;
		case CTF_TEAM2:
			self->s.skinnum = 0xf3f3f1f1; // blue
			break;
		case CTF_TEAM3:
			self->s.skinnum = 0xd0d1d2d3; // green
			break;
		}

	}
	else if (Q_stricmp (info, "red") == 0)
	{
		self->s.skinnum = 0xf2f2f0f0;
	}
	else if (Q_stricmp (info, "green") == 0)
	{
		self->s.skinnum = 0xd0d1d2d3;
	}
	else if (Q_stricmp (info, "purple") == 0)
	{
		self->s.skinnum = 0xf3f3f1f1;
	}
	else if (Q_stricmp (info, "brown") == 0)
	{
		self->s.skinnum = 0xdcdddedf;
	}
	else if (Q_stricmp (info, "random") == 0)
	{
		self->s.skinnum = (randyhook);
//		gi.dprintf("Using hook color %32i.\n",randyhook);

	} 
	else if (Q_stricmp (info, "blue") == 0)
		self->s.skinnum = 0xf3f3f1f1;		// sorta blue
	else
		self->s.skinnum = 0xf2f2f0f0; // default back to red (most common)


	//} else gi.dprintf("The string was TOO short!");

		
	


//	if (self->spawnflags & 2)
//		self->s.skinnum = 0xf2f2f0f0;		// red
//	else if (self->spawnflags & 4)
//		self->s.skinnum = 0xd0d1d2d3;		// green ?
//	else if (self->spawnflags & 8)
//		self->s.skinnum = 0xf3f3f1f1;		// red+blue = purple
//	else if (self->spawnflags & 16)
//		self->s.skinnum = 0xdcdddedf;		// brown
//	else if (self->spawnflags & 32)
//		self->s.skinnum = 0xe0e1e2e3;		// red+brown


	self->think = hook_laser_think;

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	gi.linkentity (self);

	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
	hook_laser_think (self);
	return(self);
}





// Function name	: hook_reset
// Description	    : reset the hook.  pull all entities out of the world and reset
//					  the clients weapon state
// Return type		: void 
// Argument         : edict_t *rhook
void abandon_hook_reset (edict_t *rhook)
{
	// start with NULL pointer checks
	if (!rhook) return;
	if (rhook->owner)	// && rhook->owner != world) {
	{
		if (rhook->owner->client)
		{
			// client's hook is no longer out (duh)
			rhook->owner->client->hook_out = false;
			rhook->owner->client->hook_on = false;
			rhook->owner->client->hook = NULL;
			// if they have the hook selected, reset for another firing
			if (rhook->owner->client->pers.weapon &&
				strcmp(rhook->owner->client->pers.weapon->pickup_name, "Hook") == 0) {
				rhook->owner->client->ps.gunframe++;				// = 9;	// unlock animation
				rhook->owner->client->weaponstate = WEAPON_READY;	// can fire again immediately
			}
			// avoid all falling damage
			// seems to work, but i'm not quite sure if its correct
//			VectorClear(rhook->owner->client->oldvelocity);
		}
	}
	// this should always be true and free the laser beam
	if (rhook->laser) G_FreeEdict(rhook->laser);
	// delete ourself
	G_FreeEdict(rhook);
};



// Function name	: hook_cond_reset
// Description	    : resets the hook if it needs to be
// Return type		: qboolean 
// Argument         : edict_t *self
qboolean hook_cond_reset (edict_t *self)
{
	// this should never be true
	if (!self->enemy || !self->owner) {
            abandon_hook_reset (self);
            return (true);
    }

	// drop the hook if either party dies/leaves the game/etc.
    if ((!self->enemy->inuse) || (!self->owner->inuse) ||
		(self->enemy->client && self->enemy->health <= 0) || 
		(self->owner->health <= 0))
    {
            abandon_hook_reset (self);
            return (true);
    }

    // drop the hook if player lets go of button
	// and has the hook as current weapon
    if (!((self->owner->client->latched_buttons|self->owner->client->buttons) & BUTTON_ATTACK)
		&& (strcmp(self->owner->client->pers.weapon->pickup_name, "Hook") == 0))
    {
            abandon_hook_reset (self);
			return (true);
    }

	return (false);
}


// Function name	: hook_service
// Description	    : Do all the service hook crap (move client, release etc)
// Return type		: void 
// Argument         : edict_t *self
void abandon_hook_service (edict_t *self)
{
    vec3_t	hook_dir;

	// if hook should be dropped, just return
	if (hook_cond_reset(self)) return;

	// give the client some velocity ...
	if (self->enemy->client)
		_VectorSubtract(self->enemy->s.origin, self->owner->s.origin, hook_dir);
	else
		_VectorSubtract(self->s.origin, self->owner->s.origin, hook_dir);
    VectorNormalize(hook_dir);

	VectorScale(hook_dir, sk_hook_pullspeed->value, self->owner->velocity);

	// avoid "falling" damage JMC
	//VectorCopy(self->owner->velocity, self->owner->client->oldvelocity);
}



// Function name	: hook_track
// Description	    : keeps the invisible hook entity on hook->enemy (can be world or an entity)
// Return type		: void 
// Argument         : edict_t *self
void abandon_hook_track (edict_t *self)
{
	vec3_t	normal;

	// if hook should be dropped, just return
	if (hook_cond_reset(self)) return;

	// bring the pAiN!
    if (self->enemy->client)
    {
		// move the hook along with the player.  It's invisible, but
		// we need this to make the sound come from the right spot
//			gi.unlinkentity(self);
		VectorCopy(self->enemy->s.origin, self->s.origin);
//			gi.linkentity(self);
		
		_VectorSubtract(self->owner->s.origin, self->enemy->s.origin, normal);

		
		T_Damage (self->enemy, self, self->owner, vec3_origin, self->enemy->s.origin, normal, sk_hook_damage->value, 0, DAMAGE_NO_KNOCKBACK, MOD_GRAPPLE);
    }
	else {

        // If the hook is not attached to the player, constantly copy
	    // copy the target's velocity. Velocity copying DOES NOT work properly
	    // for a hooked client. 
        VectorCopy(self->enemy->velocity, self->velocity);
	}

	gi.linkentity(self);
    self->nextthink = level.time + 0.1;
};



// Function name	: hook_touch
// Description	    : the hook has hit something.  what could it be? :)
// Return type		: void 
// Argument         : edict_t *self
// Argument         : edict_t *other
// Argument         : cplane_t *plane
// Argument         : csurface_t *surf
void hook_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	dir, normal;

	/*---- JMC MOD 1/28/99 11:04:08 PM ----*/


	if ((surf && (surf->flags & SURF_SKY)) && (!sk_hook_sky->value))
	{
		abandon_hook_reset(self);
		return;
	}

	/*---- JMC MOD 1/28/99 11:04:10 PM ----*/


	
	// ignore hitting the person who launched us
	if (other == self->owner)
		return;
	
	// ignore hitting items/projectiles/etc.
	if (other->solid == SOLID_NOT || other->solid == SOLID_TRIGGER || other->movetype == MOVETYPE_FLYMISSILE)
		return;
	
	if (other->client)	// we hit a player
	{
		// ignore hitting a teammate
		if (OnSameTeam(other, self->owner)) return;
		// we hit an enemy, so do a bit of damage
		_VectorSubtract(other->s.origin, self->owner->s.origin, dir);
		_VectorSubtract(self->owner->s.origin, other->s.origin, normal);
		T_Damage(other, self, self->owner, dir, self->s.origin, normal, sk_hook_damage->value, sk_hook_damage->value, 0, MOD_GRAPPLE);
	}
	else	// we hit something thats not a player
	{	
		// if we can hurt it, then do a bit of damage
		if (other->takedamage) {
			_VectorSubtract(other->s.origin, self->owner->s.origin, dir);
			_VectorSubtract(self->owner->s.origin, other->s.origin, normal);
			T_Damage(other, self, self->owner, dir, self->s.origin, normal, 1, 1, 0, MOD_HIT);
		}
		// stop moving
		VectorClear(self->velocity);
		
		// gi.sound() doesnt work because the origin of an entity with no model is not 
		// transmitted to clients or something.  hoped this would be fixed in Q2 ...
		gi.positioned_sound(self->s.origin, self, CHAN_WEAPON, gi.soundindex("flyer/Flyatck2.wav"), 1, ATTN_NORM, 0);
	}
	

// handled in hook_cond_reset()
	// check to see if we already let up on the fire button
//	if ( !((self->owner->client->latched_buttons|self->owner->client->buttons) & BUTTON_ATTACK) ) {
//		abandon_hook_reset(self);
//		return;
//	}

	// remember who/what we hit
	// this must be set before hook_cond_reset() is called
	self->enemy = other;

	// if hook should be dropped, just return
	if (hook_cond_reset(self)) return;

	// pull us off the ground (figuratively)
//	self->owner->groundentity = NULL;

	// we are now anchored
	self->owner->client->hook_on = true;
	
	// keep up with that thing
	self->think = abandon_hook_track;
	self->nextthink = level.time + 0.1;
	
	self->solid = SOLID_NOT;
}



// Function name	: fire_hook
// Description	    : creates the invisible hook entity and sends it on its way attaches a laser to it
// Return type		: void 
// Argument         : edict_t *owner
// Argument         : vec3_t start
// Argument         : vec3_t forward
void abandon_fire_hook(edict_t *owner, vec3_t start, vec3_t forward)
{
		edict_t	*hook;
		trace_t tr;

        hook = G_Spawn();
        hook->movetype = MOVETYPE_FLYMISSILE;
        hook->solid = SOLID_BBOX;
		hook->clipmask = MASK_SHOT;
        hook->owner = owner;			// this hook belongs to me
		owner->client->hook = hook;		// this is my hook
        hook->classname = "hook";		// this is a hook
		
		
		vectoangles (forward, hook->s.angles);
        VectorScale(forward, sk_hook_speed->value, hook->velocity);

        hook->touch = hook_touch;

		hook->think = abandon_hook_reset;
		hook->nextthink = level.time + 5;
//		hook->noblock = true;

		gi.setmodel(hook, "");

        VectorCopy(start, hook->s.origin);
		VectorCopy(hook->s.origin, hook->s.old_origin);

		VectorClear(hook->mins);
		VectorClear(hook->maxs);

		// start up the laser
		hook->laser = abandon_hook_laser_start(hook);

		// put it in the world
		gi.linkentity(hook);


// from id's code.  I don't question these things...		
		tr = gi.trace (owner->s.origin, NULL, NULL, hook->s.origin, hook, MASK_SHOT);
		if (tr.fraction < 1.0)
		{
			VectorMA (hook->s.origin, -10, forward, hook->s.origin);
			hook->touch (hook, tr.ent, NULL, NULL);
		}

}



// Function name	: hook_fire
// Description	    : a call has been made to fire the hook
// Return type		: void 
// Argument         : edict_t *ent
void abandon_hook_fire (edict_t *ent, qboolean altfire)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	// due to the way Weapon_Generic was written up, if weaponstate
	// is not WEAPON_FIRING, then we can switch away
	// since we don't want to be in any other real state,
	// we just set it to some 'invalid' state and everything works
	// fine :)
	if (ent->client->pers.weapon &&
		strcmp(ent->client->pers.weapon->pickup_name, "Hook") == 0)
			ent->client->weaponstate = -1;	// allow weapon change

	if (ent->deadflag == DEAD_DEAD)
		return;

#ifdef	USEABANDON
	if (!ent->client->resp.ingame)
#else
	if (ent->client->resp.spectator) 
#endif
		return;

	/*---- JMC MOD 1/28/99 10:27:25 PM ----*/

#ifdef	USEABANDON

	if ((int)ABFLAGS->value & AB_NO_HOOK)
		return;

#endif

	/*---- JMC MOD 1/28/99 10:27:27 PM ----*/

	if (ent->client->hook_out)		// reject subsequent calls from Weapon_Generic
		return;

    ent->client->hook_out = true;

	// calculate start position and forward direction
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	P_ProjectSource (ent, ent->s.origin, offset, forward, right, start);	// Knightmare- changed parms for aimfix

	// kick back??
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	// actually launch the hook off
	abandon_fire_hook (ent, start, forward);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex("flyer/Flyatck3.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);

}

/*
void Weapon_Hook (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (!ent->client->hook_out) {
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent->s.frame = FRAME_crattak1-1;
					ent->client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent->s.frame = FRAME_attack1-1;
					ent->client->anim_end = FRAME_attack8;
				}
			}


		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			{
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (pause_frames)
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
						if (rand()&15)
							return;
					}
				}
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

//		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
//			ent->client->weaponstate = WEAPON_READY;
	}
}
*/



// Function name	: Weapon_Hook
// Description	    : boring service routine
// Return type		: void 
// Argument         : edict_t *ent
void abandon_Weapon_Hook (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

//	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, abandon_hook_fire);
}

