/*
==============================================================================

QUAKE ROTFISH

==============================================================================
*/

#include "g_local.h"
#include "m_q1rotfish.h"

#define Q1_FISH_RUN_SPEED		12
#define Q1_FISH_WALK_SPEED		8
#define Q1_FISH_MELEE_DISTANCE	60

static int	sound_chomp;
static int	sound_death;
static int	sound_idle;

void q1rotfish_idle (edict_t *self)
{
	if (random() < 0.5)
		gi.sound (self, CHAN_AUTO, sound_idle, 1, ATTN_IDLE, 0);
}

void q1rotfish_stand (edict_t *self);

mframe_t q1rotfish_frames_stand [] =
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
mmove_t	q1rotfish_move_stand = {FRAME_swim1, FRAME_swim18, q1rotfish_frames_stand, NULL};

void q1rotfish_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &q1rotfish_move_stand;
}

mframe_t q1rotfish_frrun_loop [] =
{
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL,
	ai_run, Q1_FISH_RUN_SPEED, NULL
};	
mmove_t	q1rotfish_move_run_loop = {FRAME_swim1, FRAME_swim18, q1rotfish_frrun_loop, NULL};

void q1rotfish_run (edict_t *self)
{
	self->monsterinfo.currentmove = &q1rotfish_move_run_loop;
}

/* Standard Swimming */ 
mframe_t q1rotfish_walk_loop [] =
{
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL,
	ai_walk, Q1_FISH_WALK_SPEED, NULL
};	
mmove_t	q1rotfish_move_walk_loop = {FRAME_swim1, FRAME_swim18, q1rotfish_walk_loop, NULL};

void q1rotfish_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &q1rotfish_move_walk_loop;
}

mframe_t q1rotfish_frames_pain1 [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0, NULL
};
mmove_t q1rotfish_move_pain1 = {FRAME_pain1, FRAME_pain9, q1rotfish_frames_pain1, q1rotfish_run};

void q1rotfish_bite (edict_t *self)
{
	vec3_t	aim;

	if (!self->enemy)
		return;

	VectorSet (aim, Q1_FISH_MELEE_DISTANCE, 0, 0);

	gi.sound (self, CHAN_WEAPON, sound_chomp, 1, ATTN_NORM, 0);

	fire_hit (self, aim, (random() + random()) * 3, 0);
}

mframe_t q1rotfish_frames_attack [] =
{
	ai_charge, 10,	NULL,
	ai_charge, 10,	NULL,
	ai_charge, 0,	q1rotfish_bite,
	ai_charge, 10,	NULL,
	ai_charge, 10,	NULL,
	ai_charge, 10,	NULL,
	ai_charge, 10,	NULL,
	ai_charge, 10,	NULL,
	ai_charge, 0,	q1rotfish_bite,
	ai_charge, 10,	NULL,
	ai_charge, 10,	NULL,
	ai_charge, 10,	NULL,
	ai_charge, 10,	NULL,
	ai_charge, 10,	NULL,
	ai_charge, 0,	q1rotfish_bite,
	ai_charge, 10,	NULL,
	ai_charge, 10,	NULL,
	ai_charge, 10,	NULL,
};
mmove_t q1rotfish_move_attack = {FRAME_attack1, FRAME_attack18, q1rotfish_frames_attack, q1rotfish_run};

void q1rotfish_melee(edict_t *self)
{
	self->monsterinfo.currentmove = &q1rotfish_move_attack;
}

void q1rotfish_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 1.1;

	self->monsterinfo.currentmove = &q1rotfish_move_pain1;
}

void q1rotfish_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->gib_health = -10000;
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

mframe_t q1rotfish_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
};
mmove_t q1rotfish_move_death = {FRAME_death1, FRAME_death21, q1rotfish_frames_death, q1rotfish_dead};

void q1rotfish_sight (edict_t *self, edict_t *other)
{
}

void q1rotfish_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->monsterinfo.currentmove = &q1rotfish_move_death;
}


//
// SPAWN
//

/*QUAKED monster_q1_rotfish (1 .5 0) (-16 -16 -24) (16 16 24) Ambush Trigger_Spawn Sight
model="models/monsters/q1rotfish/tris.md2"
*/
void SP_monster_q1_rotfish (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_death		= gi.soundindex ("q1rotfish/death.wav");	
	sound_chomp		= gi.soundindex ("q1rotfish/bite.wav");
	sound_idle		= gi.soundindex ("q1rotfish/idle.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	// Lazarus: special purpose skins
	if ( self->style )
	{
		PatchMonsterModel("models/monsters/q1rotfish/tris.md2");
		self->s.skinnum = self->style;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/q1rotfish/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 24);

	if (!self->health)
		self->health = 25;
//	if (!self->gib_health)
		self->gib_health = 0;
	if (!self->mass)
		self->mass = 100;

	self->pain = q1rotfish_pain;
	self->die = q1rotfish_die;

	self->flags |= FL_Q1_MONSTER;

	self->monsterinfo.stand = q1rotfish_stand;
	self->monsterinfo.walk = q1rotfish_walk;
	self->monsterinfo.run = q1rotfish_run;
	self->monsterinfo.melee = q1rotfish_melee;
	self->monsterinfo.sight = q1rotfish_sight;

	if (!self->monsterinfo.flies)
		self->monsterinfo.flies = 0.95;

	// Lazarus
	if (self->powerarmor)
	{
		if (self->powerarmortype == 1)
			self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
		else
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		self->monsterinfo.power_armor_power = self->powerarmor;
	}

	self->common_name = "Rotfish";
	self->class_id = ENTITY_MONSTER_Q1_ROTFISH;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &q1rotfish_move_stand;	
	if (self->health < 0)
	{
		mmove_t	*deathmoves[] = {&q1rotfish_move_death,
								 NULL};
		M_SetDeath (self, (mmove_t **)&deathmoves);
	}
	self->monsterinfo.scale = MODEL_SCALE;

	swimmonster_start (self);
}
