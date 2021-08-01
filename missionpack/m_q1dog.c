/*
==============================================================================

QUAKE DOG

==============================================================================
*/

#include "g_local.h"
#include "m_q1dog.h"


static int	sound_pain;
static int	sound_death;
static int	sound_gib;
static int	sound_attack;
static int	sound_sight;
static int	sound_idle;


void q1dog_idle (edict_t *self)
{
	if (random() < 0.2)
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void q1dog_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

//STAND

mframe_t q1dog_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	q1dog_move_stand = {FRAME_stand1, FRAME_stand9, q1dog_frames_stand, NULL};

void q1dog_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &q1dog_move_stand;
}

// WALK

mframe_t q1dog_frames_walk [] =
{
	ai_walk, 7, q1dog_idle,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL
};
mmove_t q1dog_move_walk = {FRAME_walk1, FRAME_walk8, q1dog_frames_walk, NULL};

void q1dog_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &q1dog_move_walk;
}

// RUN

mframe_t q1dog_frames_run [] =
{
	ai_run, 12, q1dog_idle,
	ai_run, 20, NULL,
	ai_run, 20, NULL,
	ai_run, 16, NULL,
	ai_run, 22, NULL,
	ai_run, 20, NULL,
	ai_run, 12, NULL,
	ai_run, 20, NULL,
	ai_run, 20, NULL,
	ai_run, 16, NULL,
	ai_run, 22, NULL,
	ai_run, 20, NULL
};
mmove_t q1dog_move_run = {FRAME_run1, FRAME_run12, q1dog_frames_run, NULL};

void q1dog_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &q1dog_move_walk;
	else
		self->monsterinfo.currentmove = &q1dog_move_run;
}

// PAIN

mframe_t q1dog_frames_pain1 [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL
};
mmove_t q1dog_move_pain1 = {FRAME_pain1, FRAME_pain6, q1dog_frames_pain1, q1dog_run};

mframe_t q1dog_frames_pain2 [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, -4, NULL,
	ai_move, -12,NULL,
	ai_move, -12,NULL,
	ai_move, -2, NULL,
	ai_move, 0,	NULL,
	ai_move, -4,	NULL,
	ai_move, 0,	NULL,
	ai_move, -10,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL
};
mmove_t q1dog_move_pain2 = {FRAME_painb1, FRAME_painb16, q1dog_frames_pain2, q1dog_run};

void q1dog_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 1;
	
	if (self->health > 0)
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0); //WAS ATTN_IDLE
	
	if (random() < 0.5)
		self->monsterinfo.currentmove = &q1dog_move_pain1;
	else
		self->monsterinfo.currentmove = &q1dog_move_pain2;
}

// MELEE

void q1dog_bite (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->maxs[0], 8);
	if (fire_hit (self, aim, (random() + random() + random())*8, 100))
		gi.sound (self, CHAN_WEAPON, sound_attack, 1, ATTN_NORM, 0);
}


void q1dog_checkrefire (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		return;

	if ( ((skill->value == 2) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE) )
		self->monsterinfo.nextframe = FRAME_attack1;
}


mframe_t q1dog_frames_attack []=
{
	ai_charge, 10, NULL,
	ai_charge, 10, NULL,
	ai_charge, 10, NULL,
	ai_charge, 10, q1dog_bite,
	ai_charge, 10, NULL,
	ai_charge, 10, NULL,
	ai_charge, 10, NULL,
	ai_charge, 10, q1dog_checkrefire
};
mmove_t q1dog_move_attack = {FRAME_attack1, FRAME_attack8, q1dog_frames_attack, q1dog_run};

void q1dog_melee (edict_t *self)
{
	self->monsterinfo.currentmove = &q1dog_move_attack;
}		


// LEAP

void q1dog_jump_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (self->health <= 0)
	{
		self->touch = NULL;
		return;
	}

	if (other->takedamage)
	{
		if (VectorLength(self->velocity) > 300)
		{
			vec3_t	point, normal;
			int		damage;

			VectorCopy (self->velocity, normal);
			VectorNormalize(normal);
			VectorMA (self->s.origin, self->maxs[0], normal, point);
			damage = 10 + 10 * random();
			T_Damage (other, self, self, self->velocity, point, normal, damage, damage, 0, MOD_UNKNOWN);
		}
	}

	if (!M_CheckBottom (self))
	{
		if (self->groundentity)
		{
			self->monsterinfo.nextframe = FRAME_attack1;
			self->touch = NULL;
		}
		return;
	}

	self->touch = NULL;
}


void q1dog_takeoff (edict_t *self)
{
	vec3_t	forward;

	gi.sound (self, CHAN_VOICE, sound_attack, 1, ATTN_NORM, 0);
	AngleVectors (self->s.angles, forward, NULL, NULL);
	self->s.origin[2] += 1;
	VectorScale (forward, 350, self->velocity);
	self->velocity[2] = 220;
	self->groundentity = NULL;
	self->monsterinfo.attack_finished = level.time + 2;
	self->touch = q1dog_jump_touch;
}

void q1dog_checklanding (edict_t *self)
{
	if (self->groundentity)
	{
		self->monsterinfo.attack_finished = 0;
		return;
	}

	if (level.time > self->monsterinfo.attack_finished)
		self->monsterinfo.nextframe = FRAME_attack1;
}


mframe_t q1dog_frames_leap [] =
{
	ai_charge,	0,	NULL,
	ai_charge,	0,	q1dog_takeoff,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	q1dog_checklanding
};
mmove_t q1dog_move_leap = {FRAME_leap1, FRAME_leap9, q1dog_frames_leap, q1dog_run};

void q1dog_leap (edict_t *self)
{
	self->monsterinfo.currentmove = &q1dog_move_leap;
}

//
// CHECKATTACK
//

static qboolean q1dog_check_melee (edict_t *self)
{
	if (range (self, self->enemy) == RANGE_MELEE)
		return true;
	return false;
}

static qboolean q1dog_check_jump (edict_t *self)
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

	if (distance < 80)
		return false;
	if (distance > 80)
	{
		vec3_t offset,end,dir;
		vec3_t forward,right;
		int cont;
		VectorSet(offset, 80, 0, -2);
		AngleVectors (self->s.angles, forward, right, NULL);
		G_ProjectSource (self->s.origin, offset, forward, right, dir);
		VectorMA (self->s.origin, 100 , dir, end);  
		cont = gi.pointcontents (end);
		if (!(cont & MASK_SOLID))
		{
		//	gi.dprintf("DOG TARGET LANDING IS NOT SOLID\n");
			return false;
		}
			
		if (random() < 0.7)
			return false;
	}
	return true;
}

qboolean q1dog_checkattack (edict_t *self)
{
	if (!self->enemy || self->enemy->health <= 0)
		return false;

	if (q1dog_check_melee(self))
	{
		self->monsterinfo.attack_state = AS_MELEE;
		return true;
	}

	if (q1dog_check_jump(self))
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		return true;
	}
	return false;
}


// DEAD

void q1dog_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

void q1dog_nogib (edict_t *self)
{
	self->gib_health = -10000;
}

mframe_t q1dog_frames_death1 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	q1dog_nogib,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t q1dog_move_death1 = {FRAME_death1, FRAME_death9, q1dog_frames_death1, q1dog_dead};

mframe_t q1dog_frames_death2 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	q1dog_nogib,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t q1dog_move_death2 = {FRAME_deathb1, FRAME_deathb9, q1dog_frames_death2, q1dog_dead};

void q1dog_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// check for gib
	if (self->health <= self->gib_health && !(self->spawnflags & SF_MONSTER_NOGIB))
	{
		gi.sound (self, CHAN_VOICE|CHAN_RELIABLE, sound_gib, 1, ATTN_NORM, 0);
		
		ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", 0, 0, damage, GIB_ORGANIC);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib3/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/q1dog/head/tris.md2", 0, 0, damage, GIB_ORGANIC);

		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if (random() < 0.5)
		self->monsterinfo.currentmove = &q1dog_move_death1;
	else
		self->monsterinfo.currentmove = &q1dog_move_death2;
}		


//
// SPAWN
//

/*QUAKED monster_q1_dog (1 .5 0) (-32 -32 -24) (32 32 32) Ambush Trigger_Spawn Sight
model="models/monsters/q1dog/tris.md2"
*/
void SP_monster_q1_dog (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	sound_pain = gi.soundindex ("q1dog/dpain1.wav");
	sound_death = gi.soundindex ("q1dog/ddeath.wav");
	sound_gib =		gi.soundindex ("q1player/udeath.wav");
	sound_attack = gi.soundindex ("q1dog/dattack1.wav");
	sound_sight = gi.soundindex ("q1dog/dsight.wav");
	sound_idle = gi.soundindex ("q1dog/idle.wav");

	// precache gibs
	gi.modelindex ("models/monsters/q1dog/head/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib1/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib3/tris.md2");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	// Lazarus: special purpose skins
	if ( self->style )
	{
		PatchMonsterModel("models/monsters/q1dog/tris.md2");
		self->s.skinnum = self->style;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/q1dog/tris.md2");
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 32);

	if (!self->health)
		self->health = 30;
	if (!self->gib_health)
		self->gib_health = -35;
	if (!self->mass)
		self->mass = 80;

	self->pain = q1dog_pain;
	self->die = q1dog_die;

	self->flags |= FL_Q1_MONSTER;

	self->monsterinfo.stand = q1dog_stand;
	self->monsterinfo.walk = q1dog_walk;
	self->monsterinfo.run = q1dog_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = q1dog_leap;
	self->monsterinfo.melee = q1dog_melee;
	self->monsterinfo.sight = q1dog_sight;
	self->monsterinfo.search = q1dog_stand;
	self->monsterinfo.idle = q1dog_stand;
	self->monsterinfo.checkattack = q1dog_checkattack;

	if (!self->monsterinfo.flies)
		self->monsterinfo.flies = 0.60;

	// Lazarus
	if (self->powerarmor)
	{
		if (self->powerarmortype == 1)
			self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
		else
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		self->monsterinfo.power_armor_power = self->powerarmor;
	}

	self->common_name = "Rottweiler";
	self->class_id = ENTITY_MONSTER_Q1_DOG;

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &q1dog_move_stand;
	if (self->health < 0)
	{
		mmove_t	*deathmoves[] = {&q1dog_move_death1,
			                     &q1dog_move_death2,
								 NULL};
		M_SetDeath (self, (mmove_t **)&deathmoves);
	}
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
