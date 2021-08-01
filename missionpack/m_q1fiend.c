/*
==============================================================================

QUAKE FIEND

==============================================================================
*/

#include "g_local.h"
#include "m_q1fiend.h"


static int	sound_hit;
static int	sound_jump;
static int	sound_death;
static int	sound_gib;
static int	sound_idle;
static int	sound_pain;
static int	sound_sight;
static int  sound_land;



//
// SOUNDS
//


void fiend_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0); //was ATTN_IDLE
}

void fiend_idle_sound (edict_t *self)
{
	if (random() < 0.2)
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_STATIC, 0);
}

void fiend_idle_sound2 (edict_t *self)
{
	if (random() < 0.2)
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_NORM, 0); //was ATTN_STATIC
}


//
// STAND
//

//void fiend_stand (edict_t *self);

mframe_t fiend_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, fiend_idle_sound,		

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL		
	
};
mmove_t fiend_move_stand = {FRAME_stand1, FRAME_stand13, fiend_frames_stand, NULL}; //stand

void fiend_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &fiend_move_stand;
}


//
// WALK
//

//void fiend_walk (edict_t *self);

mframe_t fiend_frames_walk [] =
{
	ai_walk,	8,		fiend_idle_sound2,
	ai_walk,	6,		NULL,
	ai_walk,	6,		NULL,
	ai_walk,	7,		NULL,
	ai_walk,	4,		NULL,
	ai_walk,	6,		NULL,
	ai_walk,	10,		NULL,
	ai_walk,	10,		NULL
};
mmove_t fiend_move_walk = {FRAME_walk1, FRAME_walk8, fiend_frames_walk, NULL}; //walk

void fiend_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &fiend_move_walk;
}


//
// RUN
//

mframe_t fiend_frames_run [] =
{
	ai_run,	20,		fiend_idle_sound2,
	ai_run,	15,		NULL,
	ai_run,	36,		NULL,
	ai_run,	20,		NULL,
	ai_run,	15,		NULL,
	ai_run,	36,		NULL
};
mmove_t fiend_move_run = {FRAME_run1, FRAME_run6, fiend_frames_run, NULL};

void fiend_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &fiend_move_stand;
	else
		self->monsterinfo.currentmove = &fiend_move_run;
}



//
// MELEE
//

/*static*/ void fiend_hit_left (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 8);
	if (fire_hit (self, aim, (10 + (rand() %5)), 100))
		gi.sound (self, CHAN_WEAPON, sound_hit, 1, ATTN_NORM, 0);
}

/*static*/ void fiend_hit_right (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->maxs[0], 8);
	if (fire_hit (self, aim, (10 + (rand() %5)), 100))
		gi.sound (self, CHAN_WEAPON, sound_hit, 1, ATTN_NORM, 0);
}


/*static*/ void fiend_check_refire (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		return;

	if ( ((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE) )
		self->monsterinfo.nextframe = FRAME_attacka1;
}

mframe_t fiend_frames_attack [] =
{
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	fiend_hit_left,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	fiend_hit_right,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	fiend_check_refire
};
mmove_t fiend_move_attack = {FRAME_attacka1, FRAME_attacka15, fiend_frames_attack, fiend_run};


void fiend_melee (edict_t *self)
{
	self->monsterinfo.currentmove = &fiend_move_attack;
}


//
// ATTACK
//

void fiend_jump_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (self->health <= 0)
	{
		self->touch = NULL;
		return;
	}

	if (other->takedamage)
	{
		if (VectorLength(self->velocity) > 400)
		{
			vec3_t	point;
			vec3_t	normal;
			int		damage;

			VectorCopy (self->velocity, normal);
			VectorNormalize(normal);
			VectorMA (self->s.origin, self->maxs[0], normal, point);
			damage = 40 + 10 * random();
			T_Damage (other, self, self, self->velocity, point, normal, damage, damage, 0, MOD_UNKNOWN);
		}
	}

	if (!M_CheckBottom (self))
	{
		if (self->groundentity)
		{
			self->monsterinfo.nextframe = FRAME_attacka1;
			self->touch = NULL;
		}
		return;
	}

	self->touch = NULL;
}

void fiend_jump_takeoff (edict_t *self)
{
	vec3_t	forward;

	gi.sound (self, CHAN_VOICE, sound_jump, 1, ATTN_NORM, 0);
	AngleVectors (self->s.angles, forward, NULL, NULL);
	self->s.origin[2] += 1;
	VectorScale (forward, 600, self->velocity);
	self->velocity[2] = 250;
	self->groundentity = NULL;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->monsterinfo.attack_finished = level.time + 3;
	self->touch = fiend_jump_touch;
}

void fiend_check_landing (edict_t *self)
{
	if (self->groundentity)
	{
		self->monsterinfo.attack_finished = 0;
		self->monsterinfo.aiflags &= ~AI_DUCKED;
		return;
	}

	if (level.time > self->monsterinfo.attack_finished)
		self->monsterinfo.nextframe = FRAME_attacka1;
	else
		self->monsterinfo.nextframe = FRAME_attacka8;
}

mframe_t fiend_frames_jump [] =
{
	ai_charge,	 5,	NULL,
	ai_charge,	 5,	NULL,
	ai_charge,	 5,	fiend_jump_takeoff,
	ai_charge,	10,	NULL,
	ai_charge,	10,	NULL,
	ai_charge,	15,	NULL,
	ai_charge,	15,	NULL,
	ai_charge,	10,	NULL,
	ai_charge,	10,	NULL,
	ai_charge,	 5, NULL,
	ai_charge,	 5,	fiend_check_landing,
	ai_charge,	 5,	NULL
};
mmove_t fiend_move_jump = {FRAME_leap1, FRAME_leap12, fiend_frames_jump, fiend_run};

void fiend_jump (edict_t *self)
{
	self->monsterinfo.currentmove = &fiend_move_jump;
}


//
// CHECKATTACK
//

static qboolean fiend_check_melee (edict_t *self)
{
	if (range (self, self->enemy) == RANGE_MELEE)
		return true;
	return false;
}

static qboolean fiend_check_jump (edict_t *self)
{
	vec3_t	v;
	float	distance;

	if (self->absmin[2] > (self->enemy->absmin[2] + 0.75 * self->enemy->size[2]))
		return false;

	if (self->absmax[2] < (self->enemy->absmin[2] + 0.25 * self->enemy->size[2]))
		return false;

	v[0] = self->s.origin[0] - self->enemy->s.origin[0];
	v[1] = self->s.origin[1] - self->enemy->s.origin[1];
	v[2] = 0;
	distance = VectorLength(v);

	if (distance < 100)
		return false;
	if (distance > 100)
	{
		if (random() < 0.8)
			return false;
	}

	return true;
}

qboolean fiend_checkattack (edict_t *self)
{
	if (!self->enemy || self->enemy->health <= 0)
		return false;

	if (fiend_check_melee(self))
	{
		self->monsterinfo.attack_state = AS_MELEE;
		return true;
	}

	if (fiend_check_jump(self))
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		// FIXME play a jump sound here
		return true;
	}
	return false;
}


//
// PAIN
//

mframe_t fiend_frames_pain [] =
{
	ai_move,	4,	NULL,
	ai_move,	-3,	NULL,
	ai_move,	-8,	NULL,
	ai_move,	-3,	NULL,
	ai_move,	2,	NULL,
	ai_move,	5,	NULL
};
mmove_t fiend_move_pain = {FRAME_pain1, FRAME_pain6, fiend_frames_pain, fiend_run};


void fiend_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
	return;

	self->pain_debounce_time = level.time + 1.5;

	if (random()*200 > damage)
		return;

	if (self->health > 0)
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);

	self->monsterinfo.currentmove = &fiend_move_pain;
}


//
// DEATH
//

void fiend_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

void fiend_nogib (edict_t *self)
{
	self->gib_health = -10000;
}

mframe_t fiend_frames_death [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	fiend_nogib,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t fiend_move_death = {FRAME_death1, FRAME_death9, fiend_frames_death, fiend_dead};


void fiend_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// check for gib
	if (self->health <= self->gib_health && !(self->spawnflags & SF_MONSTER_NOGIB))
	{
		gi.sound (self, CHAN_VOICE|CHAN_RELIABLE, sound_gib, 1, ATTN_NORM, 0);
			
		for (n = 0; n < 2; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", 0, 0, damage, GIB_ORGANIC);
		for (n = 0; n < 2; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib3/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/q1fiend/head/tris.md2", 0, 0, damage, GIB_ORGANIC);

		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &fiend_move_death;
}


//
// SPAWN
//

/*QUAKED monster_q1_fiend (1 .5 0) (-32 -32 -24) (32 32 48) Ambush Trigger_Spawn Sight
model="models/monsters/q1fiend/tris.md2"
*/
void SP_monster_q1_fiend (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_hit =		gi.soundindex ("q1fiend/dhit2.wav");
	sound_jump =	gi.soundindex ("q1fiend/djump.wav");
	sound_death =	gi.soundindex ("q1fiend/ddeath.wav");
	sound_gib =		gi.soundindex ("q1player/udeath.wav");
	sound_idle =	gi.soundindex ("q1fiend/idle1.wav");
	sound_pain =	gi.soundindex ("q1fiend/dpain1.wav");
	sound_sight =	gi.soundindex ("q1fiend/sight2.wav");
	sound_land =	gi.soundindex ("q1fiend/dland2.wav");

	// precache gibs
	gi.modelindex ("models/monsters/q1fiend/head/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib1/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib3/tris.md2");
	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	// Lazarus: special purpose skins
	if ( self->style )
	{
		PatchMonsterModel("models/monsters/q1fiend/tris.md2");
		self->s.skinnum = self->style;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/q1fiend/tris.md2");
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 48);

	if (!self->health)
		self->health = 300;
	if (!self->gib_health)
		self->gib_health = -80;
	if (!self->mass)
		self->mass = 300;

	self->pain = fiend_pain;
	self->die = fiend_die;

	self->flags |= FL_Q1_MONSTER;

	self->monsterinfo.stand = fiend_stand;
	self->monsterinfo.walk = fiend_walk;
	self->monsterinfo.run = fiend_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = fiend_jump;
	self->monsterinfo.melee = fiend_melee;
	self->monsterinfo.sight = fiend_sight;
	self->monsterinfo.search = fiend_stand;
	self->monsterinfo.idle = fiend_stand;
	self->monsterinfo.checkattack = fiend_checkattack;

	if (!self->monsterinfo.flies)
		self->monsterinfo.flies = 0.70;

	// Lazarus
	if (self->powerarmor)
	{
		if (self->powerarmortype == 1)
			self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
		else
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		self->monsterinfo.power_armor_power = self->powerarmor;
	}

	self->common_name = "Fiend";
	self->class_id = ENTITY_MONSTER_Q1_FIEND;

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &fiend_move_stand;
	if (self->health < 0)
	{
		mmove_t	*deathmoves[] = {&fiend_move_death,
								 NULL};
		M_SetDeath (self, (mmove_t **)&deathmoves);
	}
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
