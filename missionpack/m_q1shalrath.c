/*
==============================================================================

QUAKE SHALRATH

==============================================================================
*/

#include "g_local.h"
#include "m_q1shalrath.h"

static int	sound_death;
static int	sound_gib;
static int	sound_sight;
static int	sound_pain1;
static int	sound_attack;
static int	sound_attack2;
static int	sound_idle;


// STAND
void q1shalrath_idle (edict_t *self)
{
	if (random() < 0.2)
		gi.sound (self, CHAN_BODY, sound_idle, 1, ATTN_IDLE, 0);
}

void q1shalrath_stand (edict_t *self);

mframe_t q1shalrath_frames_stand [] =
{
	ai_stand, 0, NULL
};
mmove_t q1shalrath_move_stand = {FRAME_walk1, FRAME_walk1, q1shalrath_frames_stand, q1shalrath_stand};

void q1shalrath_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &q1shalrath_move_stand;
}


//
// WALK
//

mframe_t q1shalrath_frames_walk [] =
{
	ai_walk, 6,  q1shalrath_idle,
	ai_walk, 4,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 4,  NULL,
	ai_walk, 5,  NULL
};
mmove_t q1shalrath_move_walk = {FRAME_walk1, FRAME_walk12, q1shalrath_frames_walk, NULL};

void q1shalrath_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &q1shalrath_move_walk;
}


//
// RUN
//

void q1shalrath_run (edict_t *self);

mframe_t q1shalrath_frames_run [] =
{
	ai_run, 6,  q1shalrath_idle,
	ai_run, 4,  NULL,
	ai_run, 0,  NULL,
	ai_run, 0,  NULL,
	ai_run, 0,  NULL,
	ai_run, 0,  NULL,
	ai_run, 5,  NULL,
	ai_run, 6,  NULL,
	ai_run, 5,  NULL,
	ai_run, 0,  NULL,
	ai_run, 4,  NULL,
	ai_run, 5,  NULL
};
mmove_t q1shalrath_move_run = {FRAME_walk1, FRAME_walk12, q1shalrath_frames_run, q1shalrath_run};

void q1shalrath_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &q1shalrath_move_stand;
		return;
	}
	else
		self->monsterinfo.currentmove = &q1shalrath_move_run;
}

//
// PAIN
//

mframe_t q1shalrath_frames_pain1 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL
};
mmove_t q1shalrath_move_pain1 = {FRAME_pain1, FRAME_pain5, q1shalrath_frames_pain1, q1shalrath_run};

void q1shalrath_pain (edict_t *self, edict_t *other, float kick, int damage)
{
//	float	r;
	
	if (level.time < self->pain_debounce_time)
		return;
	
//	r = random();

	self->pain_debounce_time = level.time + 3;

	gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);

	self->monsterinfo.currentmove = &q1shalrath_move_pain1;
}


//
// ATTACK
//

void q1shalrath_fire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	float	r, u;
		
	if(!self->enemy)
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
	
	gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, sound_attack2, 1, ATTN_NORM, 0);
	q1_fire_firepod (self, aim);
}

void q1shalrath_attack_snd (edict_t *self)
{
	gi.sound (self, CHAN_AUTO, sound_attack, 1, ATTN_NORM, 0);
}

mframe_t q1shalrath_frames_attack [] =
{
	ai_charge, 0, q1shalrath_attack_snd,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, q1shalrath_fire,
	ai_charge, 0, NULL
};
mmove_t q1shalrath_move_attack = {FRAME_attack1, FRAME_attack11, q1shalrath_frames_attack, q1shalrath_run};

void q1shalrath_attack(edict_t *self)
{
	self->monsterinfo.currentmove = &q1shalrath_move_attack;
}


//
// SIGHT
//

void q1shalrath_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}


//
// DEATH
//

void q1shalrath_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	gi.linkentity (self);
}

void q1shalrath_nogib (edict_t *self)
{
	self->gib_health = -10000;
}

mframe_t q1shalrath_frames_death1 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   q1shalrath_nogib,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t q1shalrath_move_death = {FRAME_death1, FRAME_death7, q1shalrath_frames_death1, q1shalrath_dead};

void q1shalrath_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// check for gib
	if (self->health <= self->gib_health && !(self->spawnflags & SF_MONSTER_NOGIB))
	{
		gi.sound (self, CHAN_VOICE|CHAN_RELIABLE, sound_gib, 1, ATTN_NORM, 0);

		for (n = 0; n < 2; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/q1gibs/q1gib2/tris.md2", 0, 0, damage, GIB_ORGANIC);
		for (n = 0; n < 2; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib3/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/q1shalrath/head/tris.md2", 0, 0, damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &q1shalrath_move_death;
}


//
// SPAWN
//

/*QUAKED monster_q1_shalrath (1 .5 0) (-16 -16 -24) (16 16 40) Ambush Trigger_Spawn Sight
model="models/monsters/q1shalrath/tris.md2"
*/
void SP_monster_q1_shalrath (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	
	sound_sight		= gi.soundindex ("q1shalrath/sight.wav");
	sound_pain1		= gi.soundindex ("q1shalrath/pain.wav");
	sound_gib		= gi.soundindex ("q1player/udeath.wav");
	sound_death		= gi.soundindex ("q1shalrath/death.wav");
	sound_attack	= gi.soundindex ("q1shalrath/attack.wav");
	sound_attack2	= gi.soundindex ("shalrath/attack2.wav");
	sound_idle		= gi.soundindex ("q1shalrath/idle.wav");

	// precache gibs
	gi.modelindex ("models/monsters/q1shalrath/head/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib1/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib2/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib3/tris.md2");
	// precache firepod
	q1_firepod_precache ();

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	// Lazarus: special purpose skins
	if ( self->style )
	{
		PatchMonsterModel("models/monsters/q1shalrath/tris.md2");
		self->s.skinnum = self->style;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/q1shalrath/tris.md2");
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 48);
	
	if (!self->health)
		self->health = 400;
	if (!self->gib_health)
		self->gib_health = -90;
	if (!self->mass)
		self->mass = 125;

	self->pain = q1shalrath_pain;
	self->die = q1shalrath_die;

	self->flags |= FL_Q1_MONSTER;

	self->monsterinfo.stand = q1shalrath_stand;
	self->monsterinfo.walk = q1shalrath_walk;
	self->monsterinfo.run = q1shalrath_run;
	self->monsterinfo.attack = q1shalrath_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = q1shalrath_sight;

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

	self->common_name = "Vore";
	self->class_id = ENTITY_MONSTER_Q1_SHALRATH;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &q1shalrath_move_stand;
	if (self->health < 0)
	{
		mmove_t	*deathmoves[] = {&q1shalrath_move_death,
								 NULL};
		M_SetDeath (self, (mmove_t **)&deathmoves);
	}
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}