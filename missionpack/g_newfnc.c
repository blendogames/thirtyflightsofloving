#include "g_local.h"

//void plat_CalcMove (edict_t *ent, vec3_t dest, void(*func)(edict_t*));

void fd_secret_move1(edict_t *self);
void fd_secret_move2(edict_t *self);
void fd_secret_move3(edict_t *self);
void fd_secret_move4(edict_t *self);
void fd_secret_move5(edict_t *self);
void fd_secret_move6(edict_t *self);
void fd_secret_done(edict_t *self);

/*
=============================================================================

SECRET DOORS

=============================================================================
*/

#define SEC_OPEN_ONCE		1          // stays open
#define SEC_1ST_LEFT		2          // 1st move is left of arrow
#define SEC_1ST_DOWN		4          // 1st move is down from arrow
#define SEC_NO_SHOOT		8          // only opened by trigger
#define SEC_YES_SHOOT		16		   // shootable even if targeted
#define SEC_MOVE_RIGHT		32
#define SEC_MOVE_FORWARD	64

#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3
#define STATE_LOWEST		4

void fd_secret_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *ent;

//	gi.dprintf("fd_secret_use\n");
	if (self->flags & FL_TEAMSLAVE)
		return;

	// added sound
	if (self->moveinfo.sound_start)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
	if (self->moveinfo.sound_middle) {
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}

	// trigger all paired doors
	for (ent = self; ent; ent = ent->teamchain)
	{
		if (self->moveinfo.state == STATE_LOWEST)
		{
			ent->moveinfo.state = STATE_DOWN;
			Move_Calc (ent, ent->pos1, fd_secret_move1);
			door_use_areaportals (self, true);
		}
		else if (self->moveinfo.state == STATE_TOP) // Knightmare added
		{
			ent->moveinfo.state = STATE_UP;
			Move_Calc (ent, ent->pos1, fd_secret_move5);
		}
	}
}

void fd_secret_killed (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	gi.dprintf("fd_secret_killed\n");
//	self->health = self->max_health;
	self->takedamage = DAMAGE_NO;

	if (self->flags & FL_TEAMSLAVE && self->teammaster && self->teammaster->takedamage != DAMAGE_NO)
		fd_secret_killed (self->teammaster, inflictor, attacker, damage, point);
	else
		fd_secret_use (self, inflictor, attacker);
}

// Wait after first movement...
void fd_secret_move1 (edict_t *self) 
{
//	gi.dprintf("fd_secret_move1\n");
	self->nextthink = level.time + 1.0;
	self->think = fd_secret_move2;
	self->moveinfo.state = STATE_BOTTOM;

	// added sound
	self->s.sound = 0;
	if (self->moveinfo.sound_end)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC
}

// Start moving sideways w/sound...
void fd_secret_move2 (edict_t *self)
{
//	gi.dprintf("fd_secret_move2\n");
    // added sound
	if (self->moveinfo.sound_start)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
	if (self->moveinfo.sound_middle) {
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}

	self->moveinfo.state = STATE_UP;
	Move_Calc (self, self->pos2, fd_secret_move3);
}

// Wait here until time to go back...
void fd_secret_move3 (edict_t *self)
{
//	gi.dprintf("fd_secret_move3\n");
	if (!(self->spawnflags & SEC_OPEN_ONCE))
	{
		self->nextthink = level.time + self->wait;
		self->think = fd_secret_move4;
	}
	self->moveinfo.state = STATE_TOP;

	// added sound
	self->s.sound = 0;
	if (self->moveinfo.sound_end)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC
}

// Move backward...
void fd_secret_move4 (edict_t *self)
{
//	gi.dprintf("fd_secret_move4\n");
	// added sound
	if (self->moveinfo.sound_start)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
	if (self->moveinfo.sound_middle) {
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}

	self->moveinfo.state = STATE_UP;
	Move_Calc (self, self->pos1, fd_secret_move5);          
}

// Wait 1 second...
void fd_secret_move5 (edict_t *self)
{
//	gi.dprintf("fd_secret_move5\n");
	self->nextthink = level.time + 1.0;
	self->think = fd_secret_move6;
	self->moveinfo.state = STATE_BOTTOM;

	// added sound
	self->s.sound = 0;
	if (self->moveinfo.sound_end)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC
}

void fd_secret_move6 (edict_t *self)
{
//	gi.dprintf("fd_secret_move6\n");
    // added sound
	if (self->moveinfo.sound_start)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, self->attenuation, 0); // was ATTN_STATIC
	if (self->moveinfo.sound_middle) {
		self->s.sound = self->moveinfo.sound_middle;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}

	self->moveinfo.state = STATE_DOWN;
	Move_Calc (self, self->pos0, fd_secret_done);
}

void fd_secret_done (edict_t *self)
{
//	gi.dprintf("fd_secret_done\n");
	if (!self->targetname || self->spawnflags & SEC_YES_SHOOT)
	{
		self->health = 1;
		self->takedamage = DAMAGE_YES;
		self->die = fd_secret_killed;   
	}
	self->moveinfo.state = STATE_LOWEST;

	// added sound
	self->s.sound = 0;
	if (self->moveinfo.sound_end)
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, self->attenuation, 0); // was ATTN_STATIC

	door_use_areaportals (self, false);
}

void secret_blocked (edict_t *self, edict_t *other)
{
	// Remove dead Q1 monsters, as they can't be gibbed
	if ( (other->svflags & SVF_DEADMONSTER) && (other->flags & FL_Q1_MONSTER) )
	{
		G_FreeEdict(other);
		return;
	}

	if (!(self->flags & FL_TEAMSLAVE))
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 0, 0, MOD_CRUSH);

//	if (time < self->attack_finished)
//		return;
//	self->attack_finished = time + 0.5;
//	T_Damage (other, self, self, self->dmg);
}

/*
================
secret_touch

Prints messages
================
*/
void secret_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->health <= 0)
		return;

//	if (!(other->client))
	// Lazarus: Allows robot usage
	if (!other->client && !(other->flags & FL_ROBOT))
		return;

	if (self->monsterinfo.attack_finished > level.time)
		return;
	self->monsterinfo.attack_finished = level.time + 2;
	
	if (self->message)
	{
		gi.centerprintf (other, self->message);
		gi.sound (other, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
	//	fixme - put this sound back??
	//	gi.sound (other, CHAN_BODY, "misc/talk.wav", 1, ATTN_NORM);
	}
}


/*QUAKED func_door_secret2 (0 .5 .8) ? OPEN_ONCE 1st_LEFT 1st_DOWN NO_SHOOT ALWAYS_SHOOT SLIDE_RIGHT SLIDE_FORWARD
Basic secret door. Slides back, then to the left. Angle determines direction.

FLAGS:
OPEN_ONCE = Stay open, never close
1st_LEFT = 1st move is left/right of arrow
1st_DOWN = 1st move is forwards/backwards
NO_SHOOT = not implemented yet
ALWAYS_SHOOT = even if targeted, keep shootable
SLIDE_RIGHT = the sideways move will be to right of arrow
SLIDE_FORWARD = the to/fro move will be forward

VALUES:
wait = # of seconds before coming back (5 default)
dmg  = damage to inflict when blocked (2 default)
width- override how far the door pops out
length- override how far the door slides
*/

void SP_func_door_secret2 (edict_t *ent)
{
	vec3_t	forward, right, up;
//	float	lrSize, fbSize;

	ent->class_id = ENTITY_FUNC_DOOR_SECRET2;

	if ( (level.maptype == MAPTYPE_CUSTOM) && (ent->sounds > 4) && (ent->sounds < 100) ) // custom sounds
	{
		ent->moveinfo.sound_start = gi.soundindex  (va("doors/dr%02i_strt.wav", ent->sounds));
		ent->moveinfo.sound_middle = gi.soundindex  (va("doors/dr%02i_mid.wav", ent->sounds));
		ent->moveinfo.sound_end = gi.soundindex  (va("doors/dr%02i_end.wav", ent->sounds));
	}
	else if (ent->sounds != 1)
	{
		ent->moveinfo.sound_start = gi.soundindex  ("doors/dr1_strt.wav");
		ent->moveinfo.sound_middle = gi.soundindex  ("doors/dr1_mid.wav");
		ent->moveinfo.sound_end = gi.soundindex  ("doors/dr1_end.wav");
	}
	else
	{
		ent->moveinfo.sound_start = 0;
		ent->moveinfo.sound_middle = 0;
		ent->moveinfo.sound_end = 0;
	}

	if (ent->attenuation <= 0)
		ent->attenuation = ATTN_STATIC;

	if (!ent->dmg)
		ent->dmg = 2;
		
	AngleVectors (ent->s.angles, forward, right, up);
	VectorCopy (ent->s.origin, ent->pos0);
	VectorCopy (ent->s.angles, ent->move_angles);
	G_SetMovedir (ent->s.angles, ent->movedir);

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);

	if (ent->move_angles[1] == 0 || ent->move_angles[1] == 180)
	{
		if (!ent->width)
			ent->width = ent->size[1] - 2; // was lrSize
		if (!ent->length)
			ent->length = ent->size[0] - 2; // was fbSize
	}		
	else if (ent->move_angles[1] == 90 || ent->move_angles[1] == 270)
	{
		if (!ent->width)
			ent->width = ent->size[0] - 2; // was lrSize
		if (!ent->length)
			ent->length = ent->size[1] - 2; // was fbSize
	}		
	else {
		gi.dprintf("func_door_secret2 angles not set at 0, 90, 180, 270!\n");
	}

	if (ent->spawnflags & SEC_MOVE_FORWARD)
		VectorScale (forward, ent->length, forward);
	else
		VectorScale (forward, ent->length * -1 , forward);

	if (ent->spawnflags & SEC_MOVE_RIGHT)
		VectorScale (right, ent->width, right);
	else
		VectorScale (right, ent->width * -1, right);

	if (ent->spawnflags & SEC_1ST_DOWN)
	{
		VectorAdd (ent->s.origin, forward, ent->pos1); // was ent->moveinfo.start_origin
		VectorAdd (ent->pos1, right, ent->pos2); // was ent->moveinfo.end_origin
	}
	else
	{
		VectorAdd (ent->s.origin, right, ent->pos1); // was ent->moveinfo.start_origin
		VectorAdd (ent->pos1, forward, ent->pos2); // was ent->moveinfo.end_origin
	}

	ent->touch = secret_touch;
	ent->blocked = secret_blocked;
	ent->use = fd_secret_use;

	ent->moveinfo.speed = 50;
	ent->moveinfo.accel = 50;
	ent->moveinfo.decel = 50;
	ent->moveinfo.state = STATE_LOWEST;

	if (!ent->targetname || ent->spawnflags & SEC_YES_SHOOT)
	{
		ent->health = 1;
		ent->max_health = ent->health;
		ent->takedamage = DAMAGE_YES;
		ent->die = fd_secret_killed;
	}
	if (!ent->wait)
		ent->wait = 5;          // 5 seconds before closing
	ent->postthink = train_move_children;	// Knightmare- now supports movewith

	gi.linkentity(ent);
}

// ==================================================

#define FWALL_START_ON		1

void force_wall_think(edict_t *self)
{
	if (self->solid == SOLID_BSP)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_FORCEWALL);
		gi.WritePosition (self->pos1);
		gi.WritePosition (self->pos2);
		gi.WriteByte  (self->style);
		gi.multicast (self->offset, MULTICAST_PVS);
	}

	self->think = force_wall_think;
	self->nextthink = level.time + FRAMETIME;
}

void force_wall_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
	{
		self->wait = 0;
		self->think = force_wall_think;
		self->nextthink = level.time + FRAMETIME;
		self->solid = SOLID_BSP;
		KillBox(self);		// Is this appropriate?
		gi.linkentity (self);
	}
	else
	{
		self->wait = 1;
		self->think = NULL;
		self->nextthink = 0;
		self->solid = SOLID_NOT;
		gi.linkentity( self );
	}
}

void force_wall_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!self->dmg)
		return;

	if (!other->takedamage)
		return;

	if (self->timestamp > level.time)
		return;

	self->timestamp = level.time + 1;

	if (!(self->spawnflags & 4))
	{
		if ((level.framenum % 10) == 0)
			gi.sound (other, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
	}

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, self->dmg, 0, MOD_TRIGGER_HURT);
}

/*QUAKED func_force_wall (1 0 1) ? START_ON
A vertical particle force wall. Turns on and solid when triggered.
If someone is in the force wall when it turns on, they're telefragged.

START_ON - forcewall begins activated. triggering will turn it off.
style - color of particles to use.
	  0: black
	208: green
	215: white
	220: yellow
	240: red
	241: blue
	224: orange
*/
void SP_func_force_wall(edict_t *ent)
{
	ent->class_id = ENTITY_FUNC_FORCE_WALL;

	gi.setmodel (ent, ent->model);

	ent->offset[0] = (ent->absmax[0] + ent->absmin[0]) / 2;
	ent->offset[1] = (ent->absmax[1] + ent->absmin[1]) / 2;
	ent->offset[2] = (ent->absmax[2] + ent->absmin[2]) / 2;

	ent->pos1[2] = ent->absmax[2];
	ent->pos2[2] = ent->absmax[2];
	if (ent->size[0] > ent->size[1])
	{
		ent->pos1[0] = ent->absmin[0];
		ent->pos2[0] = ent->absmax[0];
		ent->pos1[1] = ent->offset[1];
		ent->pos2[1] = ent->offset[1];
	}
	else
	{
		ent->pos1[0] = ent->offset[0];
		ent->pos2[0] = ent->offset[0];
		ent->pos1[1] = ent->absmin[1];
		ent->pos2[1] = ent->absmax[1];
	}
	
	if (!ent->style)
		ent->style = 208;

	ent->movetype = MOVETYPE_NONE;
	ent->wait = 1;

	if (ent->spawnflags & FWALL_START_ON)
	{
		ent->solid = SOLID_BSP;
		ent->think = force_wall_think;
		ent->nextthink = level.time + FRAMETIME;
	}
	else
		ent->solid = SOLID_NOT;

	ent->use = force_wall_use;
	ent->touch = force_wall_touch;

	ent->svflags = SVF_NOCLIENT;
	
	gi.linkentity(ent);
}

/*======================================================================================*/

/*QUAKED func_dm_wall (1 0 1) ?
When the number of clients that connect to the server is less than
the value in maxclient, the wall will appear and cut off areas of
the map. This only applies when the level is first loaded, so if
2 players start, the map will be sized to 2 player. If another 16
connect then it'll be a bit frantic (haha)

count	- number of clients before disappearing
*/

/*
FIXME : Make a spawnflag to check if the designer wants to use an effect.
	  actually make this a point entity so ANY entity can be set up like this.
	  i.e. one could target func_walls, weapons, ammo and health and have a different
		 setup depending on the number of clients conncted.
*/

void SP_func_dm_wall (edict_t *self)
{
	qboolean	spawn = false;

	self->class_id = ENTITY_FUNC_DM_WALL;

	// if it was lower than this something went very wrong.
	if (self->count > 2)
	{
		if (self->count > game.maxclients)
		{
			spawn = true;
		}
	}
	else
	{
		gi.dprintf("func_dm_wall with a count less than 2 at %s");

		G_FreeEdict(self);
		return;
	}

	if (spawn)
	{
		self->movetype = MOVETYPE_PUSH;
		gi.setmodel (self, self->model);
		self->solid = SOLID_BSP;
		gi.linkentity (self);
	}
	else
	{
		G_FreeEdict(self);
		return;
	}
}
