#include "g_local.h"

//PGM - some of these are mine, some id's. I added the define's.
#define TRIGGER_MONSTER		1
#define TRIGGER_NOT_PLAYER	2
#define TRIGGER_TRIGGERED	4
#define TRIGGER_START_OFF   4
#define TRIGGER_TOGGLE		8
//PGM
#define TRIGGER_CAMOWNER    16
#define TRIGGER_LOOKTARGET	32
#define TRIGGER_TARGET_OFF  64

void InitTrigger (edict_t *self)
{
	if (!VectorCompare (self->s.angles, vec3_origin))
		G_SetMovedir (self->s.angles, self->movedir);

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	gi.setmodel (self, self->model);
	self->svflags = SVF_NOCLIENT;
}

//Knightmare- same as above, but for bbox triggers
void InitTriggerBbox (edict_t *self)
{
	if (!VectorCompare (self->s.angles, vec3_origin))
		G_SetMovedir (self->s.angles, self->movedir);

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	if ( (!VectorLength(self->bleft)) && (!VectorLength(self->tright)) )
	{
		VectorSet(self->bleft, -16, -16, -16);
		VectorSet(self->tright, 16, 16, 16);
	}
	VectorCopy (self->bleft, self->mins);
	VectorCopy (self->tright, self->maxs);

	self->svflags = SVF_NOCLIENT;
}


// the wait time has passed, so set back up for another activation
void multi_wait (edict_t *ent)
{
	ent->nextthink = 0;
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void multi_trigger (edict_t *ent)
{
	if (ent->nextthink)
		return;		// already been triggered

	G_UseTargets (ent, ent->activator);

	if (ent->wait > 0)	
	{
		ent->think = multi_wait;
		ent->nextthink = level.time + ent->wait;
		ent->count--;
		if (ent->count == 0)
		{
			ent->touch = NULL;
			ent->nextthink = level.time + FRAMETIME;
			ent->think = G_FreeEdict;
		}
		return;
	}
	else
	{	// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		ent->touch = NULL;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = G_FreeEdict;
	}
}

void Use_Multi (edict_t *ent, edict_t *other, edict_t *activator)
{
//PGM
	if (ent->spawnflags & TRIGGER_TOGGLE)
	{
		if(ent->solid == SOLID_TRIGGER)
			ent->solid = SOLID_NOT;
		else
			ent->solid = SOLID_TRIGGER;
		gi.linkentity (ent);
	}
	else
	{
		ent->activator = activator;
		multi_trigger (ent);
	}
//PGM
}

void Touch_Multi (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if(other->client || (other->flags & FL_ROBOT))
	{
		if (self->spawnflags & TRIGGER_NOT_PLAYER)
			return;
	}
	else if (other->svflags & SVF_MONSTER)
	{
		if (!(self->spawnflags & TRIGGER_MONSTER))
			return;
	}
	else
		return;

	if( (self->spawnflags & TRIGGER_CAMOWNER) && (!other->client || !other->client->spycam))
		return;

	if (!VectorCompare(self->movedir, vec3_origin))
	{
		vec3_t	forward;

		AngleVectors(other->s.angles, forward, NULL, NULL);
		if (_DotProduct(forward, self->movedir) < 0)
			return;
	}

	self->activator = other;
	multi_trigger (self);
}

/*QUAKED trigger_multiple (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED TOGGLE
Variable sized repeatable trigger.  Must be targeted at one or more entities.
If "delay" is set, the trigger waits some time after activating before firing.
"wait" : Seconds between triggerings. (.2 default)

TOGGLE - using this trigger will activate/deactivate it. trigger will begin inactive.

sounds
1)	secret
2)	beep beep
3)	large switch

set "message" to text string

count -	how many times it can be used before being auto-killtargeted
*/

void trigger_enable (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_TRIGGER;
	self->use = Use_Multi;
	gi.linkentity (self);
}

void SP_trigger_multiple (edict_t *ent)
{
	if (ent->sounds == 1)
		ent->noise_index = gi.soundindex ("misc/secret.wav");
	else if (ent->sounds == 2)
		ent->noise_index = gi.soundindex ("misc/talk.wav");
	else if (ent->sounds == 3)
// DWH - should be silent
//		ent->noise_index = gi.soundindex ("misc/trigger1.wav");
		ent->noise_index = -1;

	if (!ent->wait)
		ent->wait = 0.2;
	ent->touch = Touch_Multi;
	ent->movetype = MOVETYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;

	if (ent->spawnflags & TRIGGER_CAMOWNER)
		ent->svflags |= SVF_TRIGGER_CAMOWNER;

//PGM
	if (ent->spawnflags & (TRIGGER_TRIGGERED | TRIGGER_TOGGLE))
	{
		ent->solid = SOLID_NOT;
		ent->use = trigger_enable;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->use = Use_Multi;
	}
//PGM

	if (!VectorCompare(ent->s.angles, vec3_origin))
		G_SetMovedir (ent->s.angles, ent->movedir);

	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);
}


/*QUAKED trigger_once (.5 .5 .5) ? x x TRIGGERED
Triggers once, then removes itself.
You must set the key "target" to the name of another object in the level that has a matching "targetname".

If TRIGGERED, this trigger must be triggered before it is live.

sounds
 1)	secret
 2)	beep beep
 3)	large switch

"message"	string to be displayed when triggered
*/

void SP_trigger_once(edict_t *ent)
{
	// make old maps work because I messed up on flag assignments here
	// triggered was on bit 1 when it should have been on bit 4
	if (ent->spawnflags & 1)
	{
		vec3_t	v;

		VectorMA (ent->mins, 0.5, ent->size, v);
		ent->spawnflags &= ~1;
		ent->spawnflags |= 4;
		gi.dprintf("fixed TRIGGERED flag on %s at %s\n", ent->classname, vtos(v));
	}

	ent->wait = -1;
	SP_trigger_multiple (ent);
}

//Knightmare 
/*QUAKED trigger_bbox (.5 .5 .5) (-8 -8 -8) (8 8 8) MONSTER NOT_PLAYER TRIGGERED TOGGLE TURRET
TOGGLE - Using this trigger will activate/deactivate it. Trigger will begin inactive.

sounds
1)	secret
2)	beep beep
3)	large switch
set "message" to text string

bleft Min b-box coords XYZ. Default = -16 -16 -16
tright Max b-box coords XYZ. Default = 16 16 16

wait -	seconds between triggers. Default=0

count -	how many times it can be used before being auto-killtargeted
*/

void SP_trigger_bbox (edict_t *self)
{
	if (self->sounds == 1)
		self->noise_index = gi.soundindex ("misc/secret.wav");
	else if (self->sounds == 2)
		self->noise_index = gi.soundindex ("misc/talk.wav");
	else if (self->sounds == 3)
		self->noise_index = gi.soundindex ("misc/trigger1.wav");
	
	if (!self->wait)
		self->wait = 0.2;
	self->touch = Touch_Multi;

	InitTriggerBbox (self);

//PGM
	if (self->spawnflags & (TRIGGER_TRIGGERED | TRIGGER_TOGGLE))
	{
		self->solid = SOLID_NOT;
		self->use = trigger_enable;
	}
	else
	{
		self->solid = SOLID_TRIGGER;
		self->use = Use_Multi;
	}
//PGM
	gi.linkentity (self);
}

/*QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
This fixed size trigger cannot be touched, it can only be fired by other events.

count -	how many times it can be used before being auto-killtargeted
*/
void trigger_relay_use (edict_t *self, edict_t *other, edict_t *activator)
{
	G_UseTargets (self, activator);
	//Knightmare- decrement count and auto-killtarget
	self->count--;
	if (self->count == 0)
	{
		self->use = NULL;
		self->nextthink = level.time + FRAMETIME;
		self->think = G_FreeEdict;
	}
}

void SP_trigger_relay (edict_t *self)
{
// DWH - gives trigger_relay same message-displaying, sound-playing capabilities
//       as trigger_multiple and trigger_once
	if (self->sounds == 1)
		self->noise_index = gi.soundindex ("misc/secret.wav");
	else if (self->sounds == 2)
		self->noise_index = gi.soundindex ("misc/talk.wav");
	else if (self->sounds == 3)
		self->noise_index = -1;
	if(!self->count) self->count=-1;
// end DWH
	self->use = trigger_relay_use;
}


/*
==============================================================================

trigger_key

 Lazarus additions:
Spawnflags
1 = Multi-use. If set, item is required EVERY time trigger_key is targeted
2 = Keep key. If set, player doesn't give up the key when trigger_key is used.
4 = Silent. If set, neither the "You need" message and sound nor keyuse.wav
    are played. This is useful for trigger_keys used only to remove items from
	the player.

Lazarus also removes non-multi-use trigger_keys once used, to free up space in
the edicts array.

==============================================================================
*/

/*QUAKED trigger_key (.5 .5 .5) (-8 -8 -8) (8 8 8) MultiUse KeepKey Silent
A relay trigger that only fires it's targets if player has the proper key.
If the Silent spawnflag is set, neither the success or failure sounds are played, and the "You need the..." message is not displayed. Useful for removing items from players.

"item" specifies the key; for example "key_data_cd"
"message" printed when successful
"key_message" printed when player doesn't have key
*/
void trigger_key_use (edict_t *self, edict_t *other, edict_t *activator)
{
	int			index;

	if (!self || !self->item)
		return;
	if (!activator || !activator->client)
		return;

	index = ITEM_INDEX(self->item);
	if (!activator->client->pers.inventory[index])
	{
		if (level.time < self->touch_debounce_time)
			return;
		self->touch_debounce_time = level.time + 5.0;
		if (!(self->spawnflags & 4))
		{
			if (self->key_message && strlen(self->key_message))
				gi.centerprintf (activator, "%s", self->key_message);
			else
				gi.centerprintf (activator, "You need the %s", self->item->pickup_name);
			// Skid added		
			if (self->item->flags & IT_Q1)
			{
				if (index == key_q1_med_gold_index || index == key_q1_med_silver_index)
					gi.sound (activator, CHAN_AUTO, gi.soundindex ("q1world/doors/medtry.wav"), 1, ATTN_NORM, 0);
				else if (index == key_q1_rune_gold_index || index == key_q1_rune_silver_index)
					gi.sound (activator, CHAN_AUTO, gi.soundindex ("q1world/doors/runetry.wav"), 1, ATTN_NORM, 0);
				else if (index == key_q1_base_gold_index || index == key_q1_base_silver_index)
					gi.sound (activator, CHAN_AUTO, gi.soundindex ("q1world/doors/basetry.wav"), 1, ATTN_NORM, 0);
			}
			else
			// end Skid
			gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/keytry.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	if (!(self->spawnflags & 4))
	{
		// Skid added		
		if (self->item->flags & IT_Q1)
		{
			if (index == key_q1_med_gold_index || index == key_q1_med_silver_index)
				gi.sound (activator, CHAN_AUTO, gi.soundindex ("q1world/doors/meduse.wav"), 1, ATTN_NORM, 0);
			else if (index == key_q1_rune_gold_index || index == key_q1_rune_silver_index)
				gi.sound (activator, CHAN_AUTO, gi.soundindex ("q1world/doors/runeuse.wav"), 1, ATTN_NORM, 0);
			else if (index == key_q1_base_gold_index || index == key_q1_base_silver_index)
				gi.sound (activator, CHAN_AUTO, gi.soundindex ("q1world/doors/baseuse.wav"), 1, ATTN_NORM, 0);
		}
		else
		// end Skid	
		gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/keyuse.wav"), 1, ATTN_NORM, 0);
	}
	if (coop->value)
	{
		int		player;
		edict_t	*ent;

		if (strcmp(self->item->classname, "key_power_cube") == 0)
		{
			int	cube;

			for (cube = 0; cube < 8; cube++)
				if (activator->client->pers.power_cubes & (1 << cube))
					break;
			for (player = 1; player <= game.maxclients; player++)
			{
				ent = &g_edicts[player];
				if (!ent->inuse)
					continue;
				if (!ent->client)
					continue;
				// DWH: keep key
				if (!(self->spawnflags & 2))
				{
					if (ent->client->pers.power_cubes & (1 << cube))
					{
						ent->client->pers.inventory[index]--;
						ent->client->pers.power_cubes &= ~(1 << cube);
					}
				}
			}
		}
		else
		{
			for (player = 1; player <= game.maxclients; player++)
			{
				ent = &g_edicts[player];
				if (!ent->inuse)
					continue;
				if (!ent->client)
					continue;
				// DWH: keep key
				if(!(self->spawnflags & 2))
					ent->client->pers.inventory[index] = 0;
			}
		}
	}
	// DWH: keep key
	else if(!(self->spawnflags & 2))
	{
		activator->client->pers.inventory[index]--;
	}

	G_UseTargets (self, activator);

	// DWH - multi-use
	if(!(self->spawnflags & 1))
	{
		self->use       = NULL;
		self->think     = G_FreeEdict;
		self->nextthink = level.time + FRAMETIME;
		gi.linkentity(self);
	}
}

void SP_trigger_key (edict_t *self)
{
	if (!st.item)
	{
		gi.dprintf("no key item for trigger_key at %s\n", vtos(self->s.origin));
		return;
	}
	self->item = FindItemByClassname (st.item);

	if (!self->item)
	{
		gi.dprintf("item %s not found for trigger_key at %s\n", st.item, vtos(self->s.origin));
		return;
	}

	if (!self->target)
	{
		gi.dprintf("%s at %s has no target\n", self->classname, vtos(self->s.origin));
		return;
	}
	//Knightmare- don't load these sounds if it is silent
	if(!(self->spawnflags & 4))
	{
		gi.soundindex ("misc/keytry.wav");
		gi.soundindex ("misc/keyuse.wav");
	}
	self->use = trigger_key_use;
}


/*
==============================================================================

trigger_counter

==============================================================================
*/

/*QUAKED trigger_counter (.5 .5 .5) (-8 -8 -8) (8 8 8) NoMessage
Acts as an intermediary for an action that takes multiple inputs.

If NoMessage is not set, it will print "1 more...2 more..." etc, when triggered and "sequence complete" when finished. After the counter has been triggered "count" times (default 2), it will fire all of it's targets and remove itself.

"killtarget" ent to remove when triggered
"message" to print when triggered
"sounds"
   0 Beep beep (Default)
   1 Secret
   2 F1 prompt
   3 Silent"
"targetname" when triggered
"target"
"delay" trigger activates in X seconds
"count" triggers before firing; default=2
*/

void trigger_counter_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->count == 0)
		return;
	
	self->count--;

	if (self->count)
	{
		if (! (self->spawnflags & 1))
		{
			gi.centerprintf(activator, "%i more to go...", self->count);
			gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}
	
	if (! (self->spawnflags & 1))
	{
		gi.centerprintf(activator, "Sequence completed!");
		gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}
	self->activator = activator;
	multi_trigger (self);
}

void SP_trigger_counter (edict_t *self)
{
	self->wait = -1;
	if (!self->count)
		self->count = 2;

	self->use = trigger_counter_use;
}


/*
==============================================================================

trigger_always

==============================================================================
*/

/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
void SP_trigger_always (edict_t *ent)
{
	// we must have some delay to make sure our use targets are present
	if (ent->delay < 0.2)
		ent->delay = 0.2;
	G_UseTargets(ent, ent);
}


/*
==============================================================================

trigger_push

==============================================================================
*/

// PGM
#define PUSH_ONCE		1
#define PUSH_START_OFF	2
#define PUSH_CUSTOM_SND	4
// PGM

static int windsound;

void trigger_push_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (strcmp(other->classname, "grenade") == 0)
	{
		VectorScale (self->movedir, self->speed * 10, other->velocity);
	}
	// Lazarus
	else if (other->movetype == MOVETYPE_PUSHABLE)
	{
		vec3_t v;
		VectorScale (self->movedir, self->speed * 2000 / (float)(other->mass), v); 
		VectorAdd(other->velocity,v,other->velocity);
	}
	else if (other->health > 0)
	{
		VectorScale (self->movedir, self->speed * 10, other->velocity);

		if (other->client)
		{
			// don't take falling damage immediately from this
			VectorCopy (other->velocity, other->client->oldvelocity);
			if (other->fly_sound_debounce_time < level.time)
			{
				other->fly_sound_debounce_time = level.time + 1.5;
				if(self->spawnflags & PUSH_CUSTOM_SND)
				{
					if(self->noise_index)
						gi.sound(other, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
				}
				else
					gi.sound (other, CHAN_AUTO, windsound, 1, ATTN_NORM, 0);
			}
		}
	}
	if (self->spawnflags & PUSH_ONCE)
		G_FreeEdict (self);
}

//======
//PGM
void trigger_push_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
		self->solid = SOLID_TRIGGER;
	else
		self->solid = SOLID_NOT;
	gi.linkentity (self);
}

//PGM
//======

/*QUAKED trigger_push (.5 .5 .5) ? PUSH_ONCE START_OFF CUSTOM_SOUND
Pushes the player and other ents.                                 
If targeted, it will toggle on and off when used.

PUSH_ONCE - Push the player once then do nothing
START_OFF - Toggled trigger_push begins in off setting
SILENT - Doesn't make wind noise

"speed"		defaults to 1000
"noise" (path/file.wav) 
*/

void SP_trigger_push (edict_t *self)
{
	InitTrigger (self);
	if (self->spawnflags & PUSH_CUSTOM_SND)
	{
		if (st.noise)
			self->noise_index = gi.soundindex(st.noise);
		else
			self->noise_index = 0;
	}
	else if (self->targetname || !(self->spawnflags & PUSH_START_OFF))
	{
		windsound = gi.soundindex ("misc/windfly.wav");
	}
	self->touch = trigger_push_touch;
	if  (!self->speed)
		self->speed = 1000;
//PGM
	if (self->targetname)		// toggleable
	{
		self->use = trigger_push_use;
		if (self->spawnflags & PUSH_START_OFF)
			self->solid = SOLID_NOT;
	}
	else if (self->spawnflags & PUSH_START_OFF)
	{   //Knightmare- they must be using the custom sound spawngflag from Lazarus
		gi.dprintf ("trigger_push is START_OFF but not targeted.\n");
	//	self->svflags = 0;
	//	self->touch = NULL;
	//	self->solid = SOLID_BSP;
	//	self->movetype = MOVETYPE_PUSH;
		self->spawnflags &= ~PUSH_START_OFF;
		self->spawnflags |= PUSH_CUSTOM_SND;
		if (st.noise)
			self->noise_index = gi.soundindex(st.noise);
		else
			self->noise_index = 0;
	}
//PGM
	gi.linkentity (self);
}

/*QUAKED trigger_push_bbox (.5 .5 .5) (-8 -8 -8) (8 8 8) PUSH_ONCE START_OFF CUSTOM_SOUND
Pushes the player and other ents.
Same as trigger_push, except it doesn't use a model.                               
If targeted, it will toggle on and off when used.

PUSH_ONCE - Push the player once then do nothing
START_OFF - Toggled trigger_push begins in off setting
SILENT - Doesn't make wind noise

"speed"		defaults to 1000
"noise" (path/file.wav) 
"bleft" Min b-box coords XYZ. Default = -16 -16 -16
"tright" Max b-box coords XYZ. Default = 16 16 16
*/

void SP_trigger_push_bbox (edict_t *self)
{
	InitTriggerBbox (self);
	if (self->spawnflags & PUSH_CUSTOM_SND)
	{
		if (st.noise)
			self->noise_index = gi.soundindex(st.noise);
		else
			self->noise_index = 0;
	}
	else if (self->targetname || !(self->spawnflags & PUSH_START_OFF))
	{
		windsound = gi.soundindex ("misc/windfly.wav");
	}
	self->touch = trigger_push_touch;
	if (!self->speed)
		self->speed = 1000;
//PGM
	if (self->targetname)		// toggleable
	{
		self->use = trigger_push_use;
		if (self->spawnflags & PUSH_START_OFF)
			self->solid = SOLID_NOT;
	}
	else if (self->spawnflags & PUSH_START_OFF)
	{	//Knightmare- they must be using the custom sound spawngflag from Lazarus
		gi.dprintf ("trigger_push_bbox is START_OFF but not targeted.\n");
	//	self->svflags = 0;
	//	self->touch = NULL;
	//	self->solid = SOLID_NOT;
	//	self->movetype = MOVETYPE_PUSH;
		self->spawnflags &= ~PUSH_START_OFF;
		self->spawnflags |= PUSH_CUSTOM_SND;
		if (st.noise)
			self->noise_index = gi.soundindex(st.noise);
		else
			self->noise_index = 0;
	}
//PGM

	gi.linkentity (self);
}

/*
==============================================================================

trigger_hurt

==============================================================================
*/

/*QUAKED trigger_hurt (.5 .5 .5) ? START_OFF TOGGLE SILENT NO_PROTECTION SLOW
Any entity that touches this will be hurt.

It does dmg points of damage each server frame

SILENT			supresses playing the sound
SLOW			changes the damage rate to once per second
NO_PROTECTION	*nothing* stops the damage

"dmg"			default 5 (whole numbers only)
*/
void hurt_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
		self->solid = SOLID_TRIGGER;
	else
		self->solid = SOLID_NOT;
	gi.linkentity (self);

	if (!(self->spawnflags & 2))
		self->use = NULL;
}


void hurt_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		dflags;

	if (!other->takedamage)
		return;

	if (self->timestamp > level.time)
		return;

	if (self->spawnflags & 16)
		self->timestamp = level.time + 1;
	else
		self->timestamp = level.time + FRAMETIME;

	if (!(self->spawnflags & 4))
	{
		if ((level.framenum % 10) == 0)
			gi.sound (other, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
	}

	if (self->spawnflags & 8)
		dflags = DAMAGE_NO_PROTECTION;
	else
		dflags = 0;
	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, self->dmg, dflags, MOD_TRIGGER_HURT);
}

void SP_trigger_hurt (edict_t *self)
{
	InitTrigger (self);

	self->noise_index = gi.soundindex ("world/electro.wav");
	self->touch = hurt_touch;

	if (!self->dmg)
		self->dmg = 5;

	if (self->spawnflags & 1)
		self->solid = SOLID_NOT;
	else
		self->solid = SOLID_TRIGGER;

	if (self->spawnflags & 2)
		self->use = hurt_use;

	gi.linkentity (self);
}

//Knightmare 
/*QUAKED trigger_hurt_bbox (.5 .5 .5) (-8 -8 -8) (8 8 8) START_OFF TOGGLE SILENT NO_PROTECTION SLOW
Any entity that touches this will be hurt.
Same as trigger_hurt, except it doesn't use a model.

It does dmg points of damage each server frame

SILENT			supresses playing the sound
SLOW			changes the damage rate to once per second
NO_PROTECTION	*nothing* stops the damage

"dmg"			default 5 (whole numbers only)

bleft Min b-box coords XYZ. Default = -16 -16 -16
tright Max b-box coords XYZ. Default = 16 16 16
*/

void SP_trigger_hurt_bbox (edict_t *self)
{
	InitTriggerBbox (self);

	self->noise_index = gi.soundindex ("world/electro.wav");
	self->touch = hurt_touch;

	if (!self->dmg)
		self->dmg = 5;

	if (self->spawnflags & 1)
		self->solid = SOLID_NOT;
	else
		self->solid = SOLID_TRIGGER;

	if (self->spawnflags & 2)
		self->use = hurt_use;

	gi.linkentity (self);
}

/*
==============================================================================

trigger_gravity

==============================================================================
*/

//PGM
void trigger_gravity_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
		self->solid = SOLID_TRIGGER;
	else
		self->solid = SOLID_NOT;
	gi.linkentity (self);
}
//PGM

void trigger_gravity_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	other->gravity = self->gravity;
}

/*QUAKED trigger_gravity (.5 .5 .5) ? TOGGLE START_OFF
Changes the touching entites gravity to
the value of "gravity".  1.0 is standard
gravity for the level.

TOGGLE - trigger_gravity can be turned on and off
START_OFF - trigger_gravity starts turned off (implies TOGGLE)
*/
void SP_trigger_gravity (edict_t *self)
{
	if (st.gravity == 0)
	{
		gi.dprintf("trigger_gravity without gravity set at %s\n", vtos(self->s.origin));
		G_FreeEdict  (self);
		return;
	}

	InitTrigger (self);

//PGM
//	self->gravity = atoi(st.gravity);
	self->gravity = atof(st.gravity);

	if (self->spawnflags & 1)				// TOGGLE
		self->use = trigger_gravity_use;

	if (self->spawnflags & 2)				// START_OFF
	{
		self->use = trigger_gravity_use;
		self->solid = SOLID_NOT;
	}

	self->touch = trigger_gravity_touch;
//PGM

	gi.linkentity (self);
}

/*QUAKED trigger_gravity_bbox (.5 .5 .5) (-8 -8 -8) (8 8 8) TOGGLE START_OFF
Changes the touching entites gravity to the value of "gravity".  1.0 is standard gravity for the level.
Same as trigger_gravity, except that it doesn't use a model.

TOGGLE - trigger_gravity can be turned on and off
START_OFF - trigger_gravity starts turned off (implies TOGGLE)

bleft Min b-box coords XYZ. Default = -16 -16 -16
tright Max b-box coords XYZ. Default = 16 16 16
*/

void SP_trigger_gravity_bbox (edict_t *self)
{
	if (st.gravity == 0)
	{
		gi.dprintf("trigger_gravity without gravity set at %s\n", vtos(self->s.origin));
		G_FreeEdict  (self);
		return;
	}

	InitTriggerBbox (self);

//PGM
//	self->gravity = atoi(st.gravity);
	self->gravity = atof(st.gravity);

	if(self->spawnflags & 1)				// TOGGLE
		self->use = trigger_gravity_use;

	if(self->spawnflags & 2)				// START_OFF
	{
		self->use = trigger_gravity_use;
		self->solid = SOLID_NOT;
	}

	self->touch = trigger_gravity_touch;
//PGM

	gi.linkentity (self);
}

/*
==============================================================================

trigger_monsterjump

==============================================================================
*/

/*QUAKED trigger_monsterjump (.5 .5 .5) ?
Walking monsters that touch this will jump in the direction of the trigger's angle.
"speed" default to 200, the speed thrown forward
"height" default to 200, the speed thrown upwards
*/

void trigger_monsterjump_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->flags & (FL_FLY | FL_SWIM) )
		return;
	if (other->svflags & SVF_DEADMONSTER)
		return;
	if ( !(other->svflags & SVF_MONSTER))
		return;

// set XY even if not on ground, so the jump will clear lips
	other->velocity[0] = self->movedir[0] * self->speed;
	other->velocity[1] = self->movedir[1] * self->speed;
	
	if (!other->groundentity)
		return;
	
	other->groundentity = NULL;
	other->velocity[2] = self->movedir[2];
}


void SP_trigger_monsterjump (edict_t *self)
{
	if (!self->speed)
		self->speed = 200;

	if (self->height)
		st.height = self->height;
	if (!st.height)
		st.height = 200;
	if (self->s.angles[YAW] == 0)
		self->s.angles[YAW] = 360;
	InitTrigger (self);
	self->touch = trigger_monsterjump_touch;
	self->movedir[2] = st.height;
}

void SP_trigger_monsterjump_bbox (edict_t *self)
{
	if (!self->speed)
		self->speed = 200;
	if (self->height)
		st.height = self->height;
	if (!st.height)
		st.height = 200;
	if (self->s.angles[YAW] == 0)
		self->s.angles[YAW] = 360;
	InitTriggerBbox (self);

	self->touch = trigger_monsterjump_touch;
	self->movedir[2] = st.height;
}

/*QUAKED trigger_monsterjump_bbox (.5 .5 .5) (-8 -8 -8) (8 8 8)
Walking monsters that touch this will jump in the direction of the trigger's angle.
Same as trigger_monsterjump, except that it doesn't use a model.

"speed" default to 200, the speed thrown forward
"height" default to 200, the speed thrown upwards

bleft Min b-box coords XYZ. Default = -16 -16 -16
tright Max b-box coords XYZ. Default = 16 16 16
*/

//=========================================================================================
// TRIGGER_MASS - triggers its targets when touched by any entity with mass >= mass value
//                of trigger
//=========================================================================================
void trigger_mass_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->mass < self->mass)
		return;
	self->activator = other;
	multi_trigger (self);
}

void SP_trigger_mass (edict_t *self)
{
	// Fires its target if touched by an entity weighing at least
	// self->mass
	if (self->sounds == 1)
		self->noise_index = gi.soundindex ("misc/secret.wav");
	else if (self->sounds == 2)
		self->noise_index = gi.soundindex ("misc/talk.wav");
	else if (self->sounds == 3)
// DWH - should be silent
//		self->noise_index = gi.soundindex ("misc/trigger1.wav");
		self->noise_index = -1;

	if(!self->wait)
		self->wait = 0.2;
	self->touch = trigger_mass_touch;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	if(self->spawnflags & TRIGGER_TRIGGERED)
	{
		self->solid = SOLID_NOT;
		self->use = trigger_enable;
	}
	else
	{
		self->solid = SOLID_TRIGGER;
		self->use = Use_Multi;
	}
	if(!self->mass)
		self->mass = 100;

	gi.setmodel (self, self->model);
	gi.linkentity (self);
}

//Knightmare 
/*QUAKED trigger_mass_bbox (.5 .5 .5) (-8 -8 -8) (8 8 8) x x TRIGGERED
A "weight limit" trigger that fires at its targets when the mass of the activator is equal to or greater than the mass value for the trigger.
Same as trigger_mass, except that it doesn't use a model.

"wait" Seconds between triggers. Default=0
"mass" Minimum mass to activate. Default=100

bleft Min b-box coords XYZ. Default = -16 -16 -16
tright Max b-box coords XYZ. Default = 16 16 16
*/

void SP_trigger_mass_bbox (edict_t *self)
{
	// Fires its target if touched by an entity weighing at least
	// self->mass
	if (self->sounds == 1)
		self->noise_index = gi.soundindex ("misc/secret.wav");
	else if (self->sounds == 2)
		self->noise_index = gi.soundindex ("misc/talk.wav");
	else if (self->sounds == 3)
// DWH - should be silent
//		self->noise_index = gi.soundindex ("misc/trigger1.wav");
		self->noise_index = -1;

	if(!self->wait) self->wait = 0.2;
	self->touch = trigger_mass_touch;

	InitTriggerBbox (self);

	if(self->spawnflags & TRIGGER_TRIGGERED)
	{
		self->solid = SOLID_NOT;
		self->use = trigger_enable;
	}
	else
	{
		self->solid = SOLID_TRIGGER;
		self->use = Use_Multi;
	}
	if(!self->mass)
		self->mass = 100;

	gi.linkentity (self);
}

//=======================================================================================
// TRIGGER_INSIDE - triggers its targets when the bounding box for its pathtarget is
//                  completely inside the trigger field
//=======================================================================================
void trigger_inside_think (edict_t *self)
{
	int		i, num;
	edict_t	*touch[MAX_EDICTS], *hit;

	num = gi.BoxEdicts (self->absmin, self->absmax, touch, MAX_EDICTS, AREA_SOLID);
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse) continue;
		if (!hit->targetname) continue;
		if (Q_stricmp(self->pathtarget, hit->targetname)) continue;
		// must be COMPLETELY inside
		if (hit->absmin[0] < self->absmin[0]) continue;
		if (hit->absmin[1] < self->absmin[1]) continue;
		if (hit->absmin[2] < self->absmin[2]) continue;
		if (hit->absmax[0] > self->absmax[0]) continue;
		if (hit->absmax[1] > self->absmax[1]) continue;
		if (hit->absmax[2] > self->absmax[2]) continue;
		G_UseTargets (self, hit);
		if (self->wait > 0)	
			self->nextthink = level.time + self->wait;
		else
		{
			self->nextthink = level.time + FRAMETIME;
			self->think = G_FreeEdict;
		}
		gi.linkentity(self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity(self);
}

void SP_trigger_inside (edict_t *self)
{
	vec3_t v;

	VectorMA (self->mins, 0.5, self->size, v);
	if(!self->target)
	{
		gi.dprintf("trigger_inside with no target at %s.\n",vtos(v));
		G_FreeEdict(self);
		return;
	}
	if(!self->pathtarget)
	{
		gi.dprintf("trigger_inside with no pathtarget at %s.\n",vtos(v));
		G_FreeEdict(self);
		return;
	}
	self->movetype = MOVETYPE_NONE;
	self->svflags  |= SVF_NOCLIENT;
	self->solid    = SOLID_TRIGGER;
	if(!self->wait)
		self->wait = 0.2;

	gi.setmodel (self,self->model);
	self->think     = trigger_inside_think;
	self->nextthink = level.time + 1.0;
	gi.linkentity(self);
}

/*QUAKED trigger_inside_bbox (.5 .5 .5) (-8 -8 -8) (8 8 8) x x TRIGGERED
A multiple fire trigger that fires its targets when the bounding box of the pathtarget entity is completely inside the trigger field.
Same as trigger_inside, except that it doesn't use a model.

"movewith" ent to move with
"targetname" when triggered
"wait" Seconds between triggers. Default=0
"pathtarget" Name of activator

bleft Min b-box coords XYZ. Default = -16 -16 -16
tright Max b-box coords XYZ. Default = 16 16 16
*/

void SP_trigger_inside_bbox (edict_t *self)
{
	vec3_t v;

	VectorMA (self->mins, 0.5, self->size, v);
	if(!self->target)
	{
		gi.dprintf("trigger_inside_bbox with no target at %s.\n",vtos(v));
		G_FreeEdict(self);
		return;
	}
	if(!self->pathtarget)
	{
		gi.dprintf("trigger_inside_bbox with no pathtarget at %s.\n",vtos(v));
		G_FreeEdict(self);
		return;
	}
	InitTriggerBbox (self);

	if(!self->wait)
		self->wait = 0.2;

	self->think     = trigger_inside_think;
	self->nextthink = level.time + 1.0;
	gi.linkentity(self);
}

//==================================================================================
// TRIGGER_SCALES - coupled with target_characters, displays the weight of all
//                  entities that are "standing on" the trigger.
//==================================================================================
float weight_on_top(edict_t *ent)
{
	float weight;
	int i;
	edict_t *e;
	weight = 0.0;
	for(i=1, e=g_edicts+i; i<globals.num_edicts; i++, e++)
	{
		if(!e->inuse) continue;
		if(e->groundentity == ent)
		{
			weight += e->mass;
			weight += weight_on_top(e);
		}
	}
	return weight;
}

void trigger_scales_think (edict_t *self)
{
	float	f, fx, fy;
	int		i, num;
	int		weight;
	edict_t	*e, *touch[MAX_EDICTS], *hit;

	num = gi.BoxEdicts (self->absmin, self->absmax, touch, MAX_EDICTS, AREA_SOLID);
	weight = 0;
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse) continue;
		if (!hit->mass) continue;
		fx = fy = 0.0;
		if(hit->absmin[0] < self->absmin[0])
			fx += (self->absmin[0] - hit->absmin[0])/hit->size[0];
		if(hit->absmax[0] > self->absmax[0])
			fx += (hit->absmax[0] - self->absmax[0])/hit->size[0];
		if(hit->absmin[1] < self->absmin[1])
			fy += (self->absmin[1] - hit->absmin[1])/hit->size[1];
		if(hit->absmax[1] > self->absmax[1])
			fy += (hit->absmax[1] - self->absmax[1])/hit->size[1];
		f = (1.0 - fx - fy + fx*fy);
		if(f > 0) weight += f * hit->mass;
		weight += f*weight_on_top(hit);
	}
	if(weight != self->mass)
	{
		self->mass = weight;
		for (e = self->teammaster; e; e = e->teamchain)
		{
			if (!e->count)
				continue;
			num = e->count;
			if(weight < pow(10,num-1))
				e->s.frame = 12;
			else
				e->s.frame = ( weight % (int)pow(10,num) ) / ( pow(10,num-1) );
		}
	}
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity(self);
}

void SP_trigger_scales (edict_t *self)
{
	vec3_t v;

	VectorMA (self->mins, 0.5, self->size, v);
	if(!self->team)
	{
		gi.dprintf("trigger_scales with no team at %s.\n",vtos(v));
		G_FreeEdict(self);
		return;
	}
	self->movetype = MOVETYPE_NONE;
	self->svflags  |= SVF_NOCLIENT;
	self->solid    = SOLID_TRIGGER;
	gi.setmodel (self,self->model);
	self->think     = trigger_scales_think;
	self->nextthink = level.time + 1.0;
	self->mass = 0;
	gi.linkentity(self);
}

void SP_trigger_scales_bbox (edict_t *self)
{
	vec3_t v;

	VectorMA (self->mins, 0.5, self->size, v);
	if(!self->team)
	{
		gi.dprintf("trigger_scales_bbox with no team at %s.\n",vtos(v));
		G_FreeEdict(self);
		return;
	}

	InitTriggerBbox (self);

	self->think     = trigger_scales_think;
	self->nextthink = level.time + 1.0;
	self->mass = 0;
	gi.linkentity(self);
}

//============================================================================================
// TRIGGER_LOOK
// Serves the same function as trigger_multiple, but:
// 1) Is only usable by players
// 2) Player must be looking at a point within bleft-tright of the origin of the trigger_look
// 3) If USE spawnflag (=8) is set, player must be pressing +use to trigger

void trigger_look_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	trace_t	tr;
	vec_t	dist;
	vec3_t	dir, forward, left, up, end, start;

	if(!other->client)
		return;

	if (self->nextthink)
		return;		// already been triggered

	if( (self->spawnflags & TRIGGER_TOGGLE) && !(other->client->use))
		return;

	if( (self->spawnflags & TRIGGER_CAMOWNER) && !other->client->spycam)
		return;

	if( self->spawnflags & 32 )
	{
		// Then trigger only fires if looking at TARGET, not trigger bbox
		edict_t	*target;
		int		num_triggered=0;
		edict_t	*what;
		vec3_t	endpos;

		target = G_Find(NULL,FOFS(targetname),self->target);
		while(target && !num_triggered)
		{
			what = LookingAt(other,0,endpos,NULL);
			if(target->inuse && (LookingAt(other,0,NULL,NULL) == target))
			{
				num_triggered++;
				self->activator = other;
				G_UseTarget (self, other, target);
			}
			else
				target = G_Find(target,FOFS(targetname),self->target);
		}
		if(!num_triggered)
			return;
	}
	else
	{
		if(other->client->spycam)
		{
			vec3_t	f1, l1, u1;

			AngleVectors(other->client->spycam->s.angles, forward, left, up);
			VectorScale(forward, other->client->spycam->move_origin[0],f1);
			VectorScale(left,   -other->client->spycam->move_origin[1],l1);
			VectorScale(up,      other->client->spycam->move_origin[2],u1);
			VectorAdd(other->client->spycam->s.origin,f1,start);
			VectorAdd(start,l1,start);
			VectorAdd(start,u1,start);
		}
		else
		{
			AngleVectors(other->client->v_angle, forward, NULL, NULL);
			VectorCopy(other->s.origin,start);
			start[2] += other->viewheight;
		}
		VectorSubtract(self->s.origin,start,dir);
		dist = VectorLength(dir);
		VectorMA(start,dist,forward,end);
		
		tr = gi.trace(start,vec3_origin,vec3_origin,end,other,MASK_OPAQUE);
		
		// See if we're looking at origin, within bleft, tright
		// FIXME: The following is more or less accurate if the
		// bleft-tright box is roughly a cube. If it's considerably
		// longer in one direction we'll get false misses.
		
		if(end[0] < self->s.origin[0] + self->bleft[0])
			return;
		if(end[1] < self->s.origin[1] + self->bleft[1])
			return;
		if(end[2] < self->s.origin[2] + self->bleft[2])
			return;
		if(end[0] > self->s.origin[0] + self->tright[0])
			return;
		if(end[1] > self->s.origin[1] + self->tright[1])
			return;
		if(end[2] > self->s.origin[2] + self->tright[2])
			return;
		
		self->activator = other;
		G_UseTargets (self, other);
	}

	if (self->wait > 0)	
	{
		self->think = multi_wait;
		self->nextthink = level.time + self->wait;
	}
	else
	{	// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		self->touch = NULL;
		self->nextthink = level.time + FRAMETIME;
		self->think = G_FreeEdict;
	}

}

void trigger_look_enable (edict_t *self, edict_t *other, edict_t *activator);
void trigger_look_disable (edict_t *self, edict_t *other, edict_t *activator)
{
	self->count--;
	if(self->count == 0)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 1;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->use = trigger_look_enable;
		gi.linkentity (self);
	}
}

void trigger_look_enable (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_TRIGGER;
	self->use = trigger_look_disable;
	gi.linkentity (self);
}

void SP_trigger_look (edict_t *self)
{
	InitTriggerBbox (self);

	if (self->sounds == 1)
		self->noise_index = gi.soundindex ("misc/secret.wav");
	else if (self->sounds == 2)
		self->noise_index = gi.soundindex ("misc/talk.wav");
	else if (self->sounds == 3)
		self->noise_index = -1;
	
	if (!self->wait)
		self->wait = 0.2;

	if (self->spawnflags & TRIGGER_TRIGGERED)
	{
		self->solid = SOLID_NOT;
		self->use = trigger_look_enable;
	}
	else
	{
		self->solid = SOLID_TRIGGER;
		self->use = trigger_look_disable;
	}

	gi.setmodel (self, self->model);

	if (self->spawnflags & TRIGGER_CAMOWNER)
		self->svflags |= SVF_TRIGGER_CAMOWNER;

	self->touch = trigger_look_touch;
}

void trigger_speaker_think (edict_t *self)
{
	int			i;
	edict_t		*touching;
	edict_t		*player;

	touching = NULL;
	for (i = 1; i <= maxclients->value && !touching; i++) {
		player = &g_edicts[i];
		if(!player->inuse) continue;
		if(player->s.origin[0] < self->s.origin[0] + self->bleft[0]) continue;
		if(player->s.origin[1] < self->s.origin[1] + self->bleft[1]) continue;
		if(player->s.origin[2] < self->s.origin[2] + self->bleft[2]) continue;
		if(player->s.origin[0] > self->s.origin[0] + self->tright[0]) continue;
		if(player->s.origin[1] > self->s.origin[1] + self->tright[1]) continue;
		if(player->s.origin[2] > self->s.origin[2] + self->tright[2]) continue;
		touching = player;
	}
	if(touching)
		gi.sound (touching, CHAN_VOICE, self->noise_index, 1, ATTN_NORM, 0);
	self->nextthink = level.time + FRAMETIME;
}

void trigger_speaker_enable (edict_t *self, edict_t *other, edict_t *activator);
void trigger_speaker_disable (edict_t *self, edict_t *other, edict_t *activator)
{
	self->use   = trigger_speaker_enable;
	self->think = NULL;
	self->nextthink = 0;
}

void trigger_speaker_enable (edict_t *self, edict_t *other, edict_t *activator)
{
	self->use   = trigger_speaker_disable;
	self->think = trigger_speaker_think;
	self->think(self);
}

void SP_trigger_speaker (edict_t *self)
{
	char	buffer[MAX_QPATH];

	if(!st.noise)
	{
		gi.dprintf("trigger_speaker with no noise set at %s\n", vtos(self->s.origin));
		return;
	}
	if (!strstr (st.noise, ".wav"))
		Com_sprintf (buffer, sizeof(buffer), "%s.wav", st.noise);
	else
	//	strncpy (buffer, st.noise, sizeof(buffer));
		Com_strcpy (buffer, sizeof(buffer), st.noise);
	self->noise_index = gi.soundindex (buffer);

	if (self->spawnflags & 1)
	{
		self->use       = trigger_speaker_disable;
		self->think     = trigger_speaker_think;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->use       = trigger_speaker_enable;
	}

	if ( (!VectorLength(self->bleft)) && (!VectorLength(self->tright)) )
	{
		VectorSet(self->bleft,-16,-16,-16);
		VectorSet(self->tright,16, 16, 16);
	}
}


//==============================================================================
// trigger_transition is a HL-like box that defines what entities will be
// moved from one map to another when a target_changelevel with the same
// targetname is fired. Brush models may NOT be moved.
//==============================================================================
void WriteEdict (FILE *f, edict_t *ent);

qboolean HasSpawnFunction(edict_t *ent)
{
	spawn_t	*s;
	gitem_t	*item;
	int		i;

	if(!ent->classname)
		return false;

	// check item spawn functions
	for (i=0,item=itemlist ; i<game.num_items ; i++,item++)
	{
		if (!item->classname)
			continue;
		if (!strcmp(item->classname, ent->classname))
			return true;
	}
	// check normal spawn functions
	for (s=spawns ; s->name ; s++)
	{
		if (!strcmp(s->name, ent->classname))
			return true;
	}
	return false;
}

void WriteTransitionEdict (FILE *f, edict_t *changelevel, edict_t *ent)
{
	byte		*temp;
	edict_t		e;
	field_t		*field;
	void		*p;

	memcpy(&e,ent,sizeof(edict_t));
	if (!Q_stricmp(e.classname,"target_laser") ||
		!Q_stricmp(e.classname,"target_blaster")  )
		vectoangles(e.movedir,e.s.angles);

	if (!Q_stricmp(e.classname,"target_speaker"))
		e.spawnflags |= 8;  // indicates that "message" contains noise

	if(changelevel->s.angles[YAW])
	{
		vec3_t	angles;
		vec3_t	forward, right, v;
		vec3_t	spawn_offset;
		
		VectorSubtract(e.s.origin,changelevel->s.origin,spawn_offset);
		angles[PITCH] = angles[ROLL] = 0.;
		angles[YAW] = changelevel->s.angles[YAW];
		AngleVectors(angles,forward,right,NULL);
		VectorNegate(right,right);
		VectorCopy(spawn_offset,v);
		G_ProjectSource (vec3_origin, v, forward, right, spawn_offset);
		VectorCopy(spawn_offset,e.s.origin);
		VectorCopy(e.velocity,v);
		G_ProjectSource (vec3_origin, v, forward, right, e.velocity);
		e.s.angles[YAW] += angles[YAW];
	}
	else
	{
		VectorSubtract(e.s.origin,changelevel->s.origin,e.s.origin);
	}
	// wipe out all edict_t and function members, since
	// they won't be valid in the next map and might otherwise
	// cause.... umm... big crash
	temp = (byte *)&e;
	for (field=fields ; field->name ; field++)
	{
		if((field->type == F_EDICT) || (field->type == F_FUNCTION))
		{
			p = (void *)(temp + field->ofs);
			*(edict_t **)p = NULL;
		}
	}
	// Clean out a few more things
	e.s.number = 0;
	memset (&e.moveinfo,   0, sizeof(moveinfo_t));
	memset (&e.area, 0, sizeof(e.area));
	e.linkcount = 0;
	e.nextthink = 0;
	e.groundentity_linkcount = 0;
	e.s.modelindex  = 0;
	e.s.modelindex2 = 0;
	e.s.modelindex3 = 0;
	e.s.modelindex4 = 0;
#ifdef KMQUAKE2_ENGINE_MOD //Knightmare- more modelindices
	e.s.modelindex5 = 0;
	e.s.modelindex6 = 0;
#endif
	e.noise_index = 0;
	// If the ent is a live bad guy monster, remove him from the total
	// monster count. He'll be added back in in the new map.
	if( (e.svflags & SVF_MONSTER) && !(e.monsterinfo.aiflags & AI_GOOD_GUY)
		&& !(e.monsterinfo.monsterflags & MFL_DO_NOT_COUNT) )
	{
		if(e.health > 0)
			level.total_monsters--;
		else
			e.max_health = -1;
	}
	// Enemy isn't preserved... let's try a new flag for
	// single-player only that tells monster to find
	// the player again at startup
	if(!coop->value && !deathmatch->value)
	{
		if(ent->enemy == &g_edicts[1] && ent->health > 0)
			e.monsterinfo.aiflags = AI_RESPAWN_FINDPLAYER;
	}
	if(e.classname &&
	   ( !Q_stricmp(e.classname,"misc_actor") || strstr(e.classname,"monster_") ) &&
	   //Knightmare- changed this from a gib_health check, to take into account no_gib monsters
	   (e.svflags & SVF_GIB) )
		e.classname = "gibhead";
	WriteEdict(f,&e);
}

entlist_t DoNotMove[] = {
	{"crane_reset"},
	{"func_clock"},
	{"func_timer"},
	{"hint_path"},
	{"info_player_coop"},
	{"info_player_coop_lava"},
	{"info_player_deathmatch"},
	{"info_player_intermission"},
	{"info_player_start"},
	{"light"},
	{"light_mine1"},
	{"light_mine2"},
	{"misc_strogg_ship"},
	{"misc_transport"},
	{"misc_viper"},
	{"misc_crashviper"},
	{"misc_viper_bomb"},
	{"misc_viper_missile"},
	{"model_train"},
	{"path_corner"},
	{"path_track"},
	{"point_combat"},
	{"target_actor"},
	{"target_anger"},
	{"target_changelevel"},
	{"target_character"},
	{"target_crosslevel_target"},
	{"target_crosslevel_trigger"},
	{"target_goal"},
	{"target_help"},
	{"target_lightramp"},
	{"target_locator"},
	{"target_lock"},
	{"target_lock_clue"},
	{"target_lock_code"},
	{"target_lock_digit"},
	{"target_rotation"},
	{"target_secret"},
	{"target_speaker"}, //Knightmare- don't move speakers
	{"target_splash"},
	{"target_steam"},
	{"target_string"},
	{"trigger_always"},
	{"trigger_counter"},
	{"trigger_elevator"},
	{"trigger_key"},
	{"trigger_relay"},
	{"turret_driver"},
	{"bad_area"},
	{"prox_field"},
	{NULL}};

void trans_ent_filename (char *filename, size_t filenameSize)
{
#if defined (_M_X64) || defined (_M_AMD64) || defined (__x86_64__)
	SavegameDirRelativePath("save_x64/trans.ent", filename, filenameSize);
#else
	SavegameDirRelativePath("save/trans.ent", filename, filenameSize);
#endif
}

int trigger_transition_ents (edict_t *changelevel, edict_t *self)
{
	char		t_file[MAX_QPATH];
	int			i, j;
	int			total=0;
	qboolean	nogo;
	edict_t		*ent;
	entlist_t	*p;
	FILE		*f;

	if(developer->value)
		gi.dprintf("==== WriteTransitionEnts ====\n");

	trans_ent_filename (t_file, sizeof(t_file));
	f = fopen(t_file,"wb");
	if(!f)
	{
		gi.dprintf("Error opening %s for writing\n",t_file);
		return 0;
	}
	// First scan entities for brush models that SHOULD change levels, e.g. func_tracktrain,
	// which had better have a partner train in the next map... or we'll bitch loudly
	for(i=game.maxclients+1; i<globals.num_edicts; i++)
	{
		ent = &g_edicts[i];
		if(!ent->inuse) continue;
		if(ent->solid != SOLID_BSP) continue;
		if(ent->s.origin[0] > self->maxs[0]) continue;
		if(ent->s.origin[1] > self->maxs[1]) continue;
		if(ent->s.origin[2] > self->maxs[2]) continue;
		if(ent->s.origin[0] < self->mins[0]) continue;
		if(ent->s.origin[1] < self->mins[1]) continue;
		if(ent->s.origin[2] < self->mins[2]) continue;
		if(!Q_stricmp(ent->classname,"func_tracktrain") && !(ent->spawnflags & 8) && ent->targetname)
		{
			edict_t	*e;

			e = G_Spawn();
			e->classname = gi.TagMalloc(17,TAG_LEVEL);
			strcpy(e->classname, "info_train_start");
			e->targetname = gi.TagMalloc(strlen(ent->targetname)+1,TAG_LEVEL);
			strcpy(e->targetname, ent->targetname);
			e->target = gi.TagMalloc(strlen(ent->target)+1,TAG_LEVEL);
			strcpy(e->target, ent->target);
			e->spawnflags = ent->spawnflags;
			VectorCopy(ent->s.origin,e->s.origin);
			VectorCopy(ent->s.angles,e->s.angles);
			VectorCopy(ent->offset,  e->offset);
			VectorCopy(ent->bleft,   e->bleft);
			VectorCopy(ent->tright,  e->tright);
			e->sounds = ent->sounds;
			e->viewheight = ent->viewheight;
			e->speed = ent->moveinfo.speed;
			// misuse/abuse a couple of entries to copy moveinfo stuff:
			e->count = ent->moveinfo.state;
			e->radius = ent->moveinfo.distance;
			e->solid = SOLID_NOT;
			e->svflags |= SVF_NOCLIENT;
			if(ent->owner)
				e->style = ent->owner - g_edicts;
			else
				e->style = 0;
			gi.linkentity(e);

			ent->owner = NULL;
			ent->spawnflags |= 24;	// SF_TRACKTRAIN_OTHERMAP | SF_TRACKTRAIN_DISABLED
			VectorClear(ent->velocity);
			VectorClear(ent->avelocity);
			ent->moveinfo.state = ent->moveinfo.prevstate = 0;	// STOP
			gi.linkentity(ent);
		}
	}

	for(i=game.maxclients+1; i<globals.num_edicts; i++)
	{
		ent = &g_edicts[i];
		ent->id = 0;
		if(!ent->inuse) continue;
		// Pass up owned entities not owned by the player on this pass...
		// get 'em next pass so we'll know whether owner is in our list
		if(ent->owner && !ent->owner->client) continue;
		if(ent->movewith) continue;
		if(ent->s.origin[0] > self->maxs[0]) continue;
		if(ent->s.origin[1] > self->maxs[1]) continue;
		if(ent->s.origin[2] > self->maxs[2]) continue;
		if(ent->s.origin[0] < self->mins[0]) continue;
		if(ent->s.origin[1] < self->mins[1]) continue;
		if(ent->s.origin[2] < self->mins[2]) continue;
		if(ent->solid == SOLID_BSP) continue;
		if((ent->solid == SOLID_TRIGGER) && !FindItemByClassname(ent->classname)) continue;
		// Do not under any circumstances move these entities:
		for(p=DoNotMove, nogo=false; p->name && !nogo; p++)
			if(!Q_stricmp(ent->classname,p->name))
				nogo = true;
		if(nogo) continue;
		if(!HasSpawnFunction(ent)) continue;
		total++;
		ent->id = total;
		if(ent->owner)
			ent->owner_id = -(ent->owner - g_edicts);
		else
			ent->owner_id = 0;
		WriteTransitionEdict(f,changelevel,ent);
		gi.unlinkentity(ent);
		ent->inuse = false;
	}
	// Repeat, ONLY for ents owned by non-players
	for(i=game.maxclients+1; i<globals.num_edicts; i++)
	{
		ent = &g_edicts[i];
		if(!ent->inuse) continue;
		if(!ent->owner) continue;
		if(ent->owner->client) continue;
		if(ent->movewith) continue;
		if(ent->solid == SOLID_BSP) continue;
		if((ent->solid == SOLID_TRIGGER) && !FindItemByClassname(ent->classname)) continue;
		// Do not under any circumstances move these entities:
		for(p=DoNotMove, nogo=false; p->name && !nogo; p++)
			if(!Q_stricmp(ent->classname,p->name))
				nogo = true;
		if(nogo) continue;
		if(!HasSpawnFunction(ent)) continue;
		if(ent->s.origin[0] > self->maxs[0]) continue;
		if(ent->s.origin[1] > self->maxs[1]) continue;
		if(ent->s.origin[2] > self->maxs[2]) continue;
		if(ent->s.origin[0] < self->mins[0]) continue;
		if(ent->s.origin[1] < self->mins[1]) continue;
		if(ent->s.origin[2] < self->mins[2]) continue;
		ent->owner_id = 0;
		for(j=game.maxclients+1; j<globals.num_edicts && !ent->owner_id; j++)
		{
			if(ent->owner == &g_edicts[j])
				ent->owner_id = g_edicts[j].id;
		}
		if(!ent->owner_id) continue;
		total++;
		ent->id = total;
		WriteTransitionEdict(f,changelevel,ent);
		gi.unlinkentity(ent);
		ent->inuse = false;
	}

	fflush(f);
	fclose(f);
	return total;
}

void SP_trigger_transition (edict_t *self)
{
	if(!self->targetname)
	{
		gi.dprintf("trigger_transition w/o a targetname\n");
		G_FreeEdict(self);
	}
	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	gi.setmodel (self, self->model);
	self->svflags = SVF_NOCLIENT;
}

void SP_trigger_transition_bbox (edict_t *self)
{
	if(!self->targetname)
	{
		gi.dprintf("trigger_transition_bbox w/o a targetname\n");
		G_FreeEdict(self);
	}
	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	if ( (!VectorLength(self->bleft)) && (!VectorLength(self->tright)) )
	{
		VectorSet(self->bleft, -16, -16, -16);
		VectorSet(self->tright, 16, 16, 16);
	}
	VectorCopy (self->bleft, self->mins);
	VectorCopy (self->tright, self->maxs);
	self->svflags = SVF_NOCLIENT;
}


/* ============================================================================
  TRIGGER_SWITCH

  Identical to trigger_multiple in just about every way, except that
  it only turns entities ON if they're currently OFF (default operation) or
  OFF if they're currently ON and TRIGGER_TARGET_OFF spawnflag is set.

==============================================================================*/

void trigger_switch_usetargets (edict_t *ent, edict_t *activator);
void trigger_switch_delay (edict_t *ent)
{
	trigger_switch_usetargets (ent, ent->activator);
	G_FreeEdict (ent);
}

void trigger_switch_usetargets (edict_t *ent, edict_t *activator)
{
	edict_t		*t;

//
// check for a delay
//
	if (ent->delay)
	{
	// create a temp object to fire at a later time
		t = G_Spawn();
		t->classname = "DelayedUse";
		t->nextthink = level.time + ent->delay;
		t->think = trigger_switch_delay;
		t->activator = activator;
		if (!activator)
			gi.dprintf ("Delay with no activator\n");
		t->message = ent->message;
		t->target = ent->target;
		t->killtarget = ent->killtarget;
		t->noise_index = ent->noise_index;
		return;
	}
	
	
//
// print the message
//
	if ((ent->message) && !(activator->svflags & SVF_MONSTER))
	{
//		Lazarus - change so that noise_index < 0 means no sound
		gi.centerprintf (activator, "%s", ent->message);
		if (ent->noise_index > 0)
			gi.sound (activator, CHAN_AUTO, ent->noise_index, 1, ATTN_NORM, 0);
		else if (ent->noise_index == 0)
			gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}

//
// kill killtargets
//
	if (ent->killtarget)
	{
		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), ent->killtarget)))
		{
			// Lazarus: remove LIVE killtargeted monsters from total_monsters
			if((t->svflags & SVF_MONSTER) && (t->deadflag == DEAD_NO))
			{
				if(!t->dmgteam || strcmp(t->dmgteam,"player"))
					if(!(t->monsterinfo.aiflags & AI_GOOD_GUY))
						level.total_monsters--;
			}
			// and decrement secret count if target_secret is removed
			else if(t->class_id == ENTITY_TARGET_SECRET)
				level.total_secrets--;
			// same deal with target_goal, but also turn off CD music if applicable
			else if(t->class_id == ENTITY_TARGET_GOAL)
			{
				level.total_goals--;
				if (level.found_goals >= level.total_goals)
				gi.configstring (CS_CDTRACK, "0");
			}
			G_FreeEdict (t);
			if (!ent->inuse)
			{
				gi.dprintf("entity was removed while using killtargets\n");
				return;
			}
		}
	}

//
// fire targets
//
	if (ent->target)
	{
		int	on;

		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), ent->target)))
		{
			if (t == ent)
			{
				gi.dprintf ("WARNING: Entity used itself.\n");
			}
			else if(t->use)
			{
				on = 0;
				switch(t->class_id)
				{
				case ENTITY_FUNC_CONVEYOR:
				case ENTITY_FUNC_FORCE_WALL:
				case ENTITY_FUNC_WALL:
					if(t->solid == SOLID_BSP)
						on=1;
					break;
				case ENTITY_FUNC_PENDULUM:
				case ENTITY_FUNC_TRAIN:
				case ENTITY_MISC_STROGG_SHIP:
				case ENTITY_MISC_VIPER:
				case ENTITY_MISC_CRASHVIPER:
				case ENTITY_MODEL_TRAIN:
				case ENTITY_TARGET_ATTRACTOR:
				case ENTITY_TARGET_FOG:
				case ENTITY_TARGET_FOUNTAIN:
				case ENTITY_TARGET_LASER:
				case ENTITY_TARGET_MAL_LASER:
				case ENTITY_TARGET_PRECIPITATION:
					if(t->spawnflags & 1)
						on=1;
					break;
				case ENTITY_FUNC_REFLECT:
					if(!(t->spawnflags & 1))
						on=1;
					break;
				case ENTITY_FUNC_ROTATING:
					on = VectorCompare (t->avelocity, vec3_origin);
					break;
				case ENTITY_FUNC_TIMER:
					if(t->nextthink)
						on=1;
					break;
				case ENTITY_FUNC_TRACKTRAIN:
					if(!(t->spawnflags & 128))
						on=1;
					break;
				case ENTITY_MODEL_TURRET:
				case ENTITY_TURRET_BREACH:
					if(t->spawnflags & 16)
						on=1;
					break;
				case ENTITY_TARGET_EFFECT:
					if(t->spawnflags & 3)
					{
						if(t->spawnflags & 1)
							on=1;
					}
					else
						on = -1;
					break;
				case ENTITY_TARGET_SPEAKER:
					if(t->spawnflags & 3)
					{
						if(t->s.sound)
							on=1;
					}
					else
						on=-1;
					break;
				default:
					on=-1;
				}


				if(ent->spawnflags & TRIGGER_TARGET_OFF)
				{
					// Only use target if it is currently ON
					if(on==1)
						t->use (t, ent, activator);
				}
				else if(on==0)
				{
					// Only use target if it is currently OFF
					t->use (t, ent, activator);
				}
			}
			if (!ent->inuse)
			{
				gi.dprintf("entity was removed while using targets\n");
				return;
			}
		}
	}
}

void trigger_switch (edict_t *ent)
{
	if (ent->nextthink)
		return;		// already been triggered

	trigger_switch_usetargets (ent, ent->activator);

	if (ent->wait > 0)	
	{
		ent->think = multi_wait;
		ent->nextthink = level.time + ent->wait;
	}
	else
	{	// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		ent->touch = NULL;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = G_FreeEdict;
	}
}

void touch_trigger_switch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if(other->client || (other->flags & FL_ROBOT))
	{
		if (self->spawnflags & TRIGGER_NOT_PLAYER)
			return;
	}
	else if (other->svflags & SVF_MONSTER)
	{
		if (!(self->spawnflags & TRIGGER_MONSTER))
			return;
	}
	else
		return;

	if( (self->spawnflags & TRIGGER_CAMOWNER) && (!other->client || !other->client->spycam))
		return;

	if (!VectorCompare(self->movedir, vec3_origin))
	{
		vec3_t	forward;

		AngleVectors(other->s.angles, forward, NULL, NULL);
		if (_DotProduct(forward, self->movedir) < 0)
			return;
	}

	self->activator = other;
	trigger_switch (self);
}

void use_trigger_switch (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->activator = activator;
	trigger_switch (ent);
}

void SP_trigger_switch (edict_t *ent)
{
	ent->class_id = ENTITY_TRIGGER_SWITCH;
	if (ent->sounds == 1)
		ent->noise_index = gi.soundindex ("misc/secret.wav");
	else if (ent->sounds == 2)
		ent->noise_index = gi.soundindex ("misc/talk.wav");
	else if (ent->sounds == 3)
		ent->noise_index = -1;
	
	if (!ent->wait)
		ent->wait = 0.2;
	ent->touch = touch_trigger_switch;
	ent->movetype = MOVETYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;

	if (ent->spawnflags & TRIGGER_CAMOWNER)
		ent->svflags |= SVF_TRIGGER_CAMOWNER;

	if (ent->spawnflags & TRIGGER_START_OFF)
	{
		ent->solid = SOLID_NOT;
		ent->use = trigger_enable;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->use = use_trigger_switch;
	}

	if (!VectorCompare(ent->s.angles, vec3_origin))
		G_SetMovedir (ent->s.angles, ent->movedir);

	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);
}

// end DWH
