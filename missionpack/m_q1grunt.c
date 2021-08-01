/*
==============================================================================

Q1 SOLDIER

==============================================================================
*/

#include "g_local.h"
#include "m_q1grunt.h"

static int	sound_idle;
static int	sound_sight;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_death;
static int	sound_gib;
static int	sound_attack;


void q1grunt_idle (edict_t *self)
{
	if (!self->enemy)
	if (random() > 0.8)
	{
		gi.sound (self, CHAN_VOICE, sound_idle, 0.8, ATTN_STATIC, 0);
	}
}



// STAND

void q1grunt_stand (edict_t *self);

mframe_t q1grunt_frames_stand [] =
{
	ai_stand, 0, q1grunt_idle,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t q1grunt_move_stand = {FRAME_stand1, FRAME_stand8, q1grunt_frames_stand, q1grunt_stand};

void q1grunt_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &q1grunt_move_stand;
}


//
// WALK
//

mframe_t q1grunt_frames_walk [] =
{
	ai_walk, 1,  q1grunt_idle,
	ai_walk, 1,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 1,  NULL
};
mmove_t q1grunt_move_walk = {FRAME_prowl_1, FRAME_prowl_24, q1grunt_frames_walk, NULL};

void q1grunt_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &q1grunt_move_walk;
}


//
// RUN
//

void q1grunt_run (edict_t *self);

mframe_t q1grunt_frames_run [] =
{
	ai_run, 11, NULL, //q1grunt_idle,
	ai_run, 15, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 8,  NULL,
	ai_run, 15, NULL,
	ai_run, 10, NULL,
	ai_run, 15, NULL
};
mmove_t q1grunt_move_run = {FRAME_run1, FRAME_run8, q1grunt_frames_run, q1grunt_run};

void q1grunt_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &q1grunt_move_stand;
		return;
	}
	else
		self->monsterinfo.currentmove = &q1grunt_move_run;
}

//
// PAIN
//

mframe_t q1grunt_frames_pain1 [] =
{
	ai_move, -2, NULL,
	ai_move, 3,  NULL,
	ai_move, 1,  NULL,
	ai_move, 1,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL
};
mmove_t q1grunt_move_pain1 = {FRAME_pain1, FRAME_pain6, q1grunt_frames_pain1, q1grunt_run};

mframe_t q1grunt_frames_pain2 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,  NULL,
	ai_move, 13,   NULL,
	ai_move, 9,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, -2,   NULL,
	ai_move, 0,   NULL
};
mmove_t q1grunt_move_pain2 = {FRAME_painb1, FRAME_painb14, q1grunt_frames_pain2, q1grunt_run};

mframe_t q1grunt_frames_pain3 [] =
{
	ai_move, 0, NULL,
	ai_move, -1, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 1, NULL,
	ai_move, 1,  NULL,
	ai_move, 0,  NULL,
	ai_move, -1,  NULL,
	ai_move, 4,  NULL,
	ai_move, 3,  NULL,
	ai_move, 6,  NULL,
	ai_move, 8,  NULL,
	ai_move, 0,  NULL
};
mmove_t q1grunt_move_pain3 = {FRAME_painc1, FRAME_painc13, q1grunt_frames_pain3, q1grunt_run};


void q1grunt_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	float	r;
	
	if (level.time < self->pain_debounce_time)
		return;
	
	if (self->health <= 0)
		return;

	r = random();

	if (r < 0.2)
	{
		self->pain_debounce_time = level.time + 0.6;
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &q1grunt_move_pain1;
	}
	else if (r < 0.6)
	{
		self->pain_debounce_time = level.time + 1.1;
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &q1grunt_move_pain2;
	}
	else
	{
		self->pain_debounce_time = level.time + 1.1;
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &q1grunt_move_pain3;
	}
}


//
// ATTACK
//

void q1grunt_fire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	float	r, u;
		
	if (!self->enemy)
		return;
	
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_SOLDIER_SHOTGUN_1], forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);

	r = crandom()*1000;
	u = crandom()*500;
	VectorMA (start, 8192, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	VectorSubtract (end, start, aim);
	VectorNormalize (aim);
	
	gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, sound_attack, 1, ATTN_NORM, 0);
	monster_fire_shotgun (self, start, aim, 2, 1, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SHOTGUN_COUNT, MZ2_SOLDIER_SHOTGUN_1);
}


void q1grunt_attack_refire (edict_t *self)
{
	if (!self->enemy || self->enemy->health <= 0)
		return;

	if ( ((skill->value == 3) && (random() < 0.2)) || (range(self, self->enemy) == RANGE_MELEE) )
		self->monsterinfo.nextframe = FRAME_shoot1;
	else
		self->monsterinfo.nextframe = FRAME_shoot8;
}


mframe_t q1grunt_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, q1grunt_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, q1grunt_attack_refire,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t q1grunt_move_attack = {FRAME_shoot1, FRAME_shoot9, q1grunt_frames_attack, q1grunt_run};

void q1grunt_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &q1grunt_move_attack;
}


//
// SIGHT
//

void q1grunt_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	
	if ((skill->value > 0) && (range(self, self->enemy) < RANGE_FAR)
		&& !(self->monsterinfo.aiflags & AI_SOUND_TARGET))
	{
		self->monsterinfo.currentmove = &q1grunt_move_attack;
	}
}


//
// DEATH
//
void q1grunt_dropshells (edict_t *self)
{
	edict_t	*backpack;

	if (self->health <= self->gib_health)
		return;

	backpack = Drop_Q1Backpack (self, FindItemByClassname("ammo_shells"), 5);
/*	backpack = Drop_Item(self, FindItemByClassname("item_q1_backpack"));
	backpack->item = FindItemByClassname("ammo_shells");
	backpack->count = 5;
	backpack->touch = Touch_Item;
	backpack->nextthink = level.time + 1800;
	backpack->think = G_FreeEdict;
*/
	self->gib_health = -10000;
}

void q1grunt_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t q1grunt_frames_death1 [] =
{
	ai_move, 0,   NULL,
	ai_move, -10, NULL,
	ai_move, -10, q1grunt_dropshells,
	ai_move, -10, NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t q1grunt_move_death1 = {FRAME_death1, FRAME_death10, q1grunt_frames_death1, q1grunt_dead};

mframe_t q1grunt_frames_death2 [] =
{
	ai_move, 0,   NULL,
	ai_move, -5,  NULL,
	ai_move, -4,  q1grunt_dropshells,
	ai_move, -13, NULL,
	ai_move, -3,  NULL,
	ai_move, -4,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t q1grunt_move_death2 = {FRAME_deathc1, FRAME_deathc11, q1grunt_frames_death2, q1grunt_dead};

void q1grunt_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// check for gib
	if (self->health <= self->gib_health && !(self->spawnflags & SF_MONSTER_NOGIB))
	{
		gi.sound (self, CHAN_VOICE|CHAN_RELIABLE, sound_gib, 1, ATTN_NORM, 0);

		// if dead body, don't drop pack when gibbed
	//	if (self->deadflag != DEAD_DEAD)
	//		q1grunt_dropshells(self);

		for (n = 0; n < 2; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/q1gibs/q1gib2/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/q1grunt/head/tris.md2", 0, 0, damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	
	if (random() < 0.5)
		self->monsterinfo.currentmove = &q1grunt_move_death1;
	else 
		self->monsterinfo.currentmove = &q1grunt_move_death2;
}


//
// SPAWN
//

/*QUAKED monster_q1_grunt (1 .5 0) (-16 -16 -24) (16 16 40) Ambush Trigger_Spawn Sight
model="models/monsters/q1grunt/tris.md2"
*/
void SP_monster_q1_grunt (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	
	sound_idle =	gi.soundindex ("q1grunt/idle.wav");
	sound_sight =	gi.soundindex ("q1grunt/sight1.wav");
	sound_pain1 =	gi.soundindex ("q1grunt/pain1.wav");
	sound_pain2 =   gi.soundindex ("q1grunt/pain2.wav");
	sound_death =	gi.soundindex ("q1grunt/death1.wav");
	sound_gib =		gi.soundindex ("q1player/udeath.wav");
	sound_attack =  gi.soundindex ("q1grunt/sattck1.wav");

	// precache backpack
	gi.modelindex ("models/items/q1backpack/tris.md2");
//	gi.soundindex ("q1weapons/lock4.wav");
	// precache gibs
	gi.modelindex ("models/monsters/q1grunt/head/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib1/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib2/tris.md2");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	// Lazarus: special purpose skins
	if ( self->style )
	{
		PatchMonsterModel("models/monsters/q1grunt/tris.md2");
		self->s.skinnum = self->style;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/q1grunt/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 40);
	
	if (!self->health)
		self->health = 35;
	if (!self->gib_health)
		self->gib_health = -35;
	if (!self->mass)
		self->mass = 100;

	self->pain = q1grunt_pain;
	self->die = q1grunt_die;

	self->flags |= FL_Q1_MONSTER;

	self->monsterinfo.stand = q1grunt_stand;
	self->monsterinfo.walk = q1grunt_walk;
	self->monsterinfo.run = q1grunt_run;
	self->monsterinfo.attack = q1grunt_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = q1grunt_sight;

	if (!self->monsterinfo.flies)
		self->monsterinfo.flies = 0.40;

	// Lazarus
	if (self->powerarmor)
	{
		if (self->powerarmortype == 1)
			self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
		else
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		self->monsterinfo.power_armor_power = self->powerarmor;
	}

	self->common_name = "Grunt";
	self->class_id = ENTITY_MONSTER_Q1_GRUNT;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &q1grunt_move_stand;
	if (self->health < 0)
	{
		mmove_t	*deathmoves[] = {&q1grunt_move_death1,
			                     &q1grunt_move_death1,
								 NULL};
		M_SetDeath (self, (mmove_t **)&deathmoves);
	}
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}