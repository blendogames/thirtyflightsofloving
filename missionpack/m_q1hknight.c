/*
==============================================================================

QUAKE HELL KNIGHT

==============================================================================
*/

#include "g_local.h"
#include "m_q1hknight.h"


static int	sound_sword1;
static int	sound_sword2;
static int	sound_flame;
static int	sound_death;
static int	sound_gib;
static int	sound_idle;
static int	sound_pain;
static int	sound_sight;

#define SWORDSOUND(self) gi.sound (self, CHAN_WEAPON, sound_sword1, 1, ATTN_NORM, 0.1f);

void hknight_stand (edict_t *self);
void hknight_walk (edict_t *self);
void hknight_check_dist (edict_t *self);
void hknight_melee (edict_t *self);

//
// SOUNDS
//
void hknight_sight_sound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void hknight_sight (edict_t *self, edict_t *other)
{
	hknight_sight_sound(self);

	hknight_check_dist(self);
}

void hknight_idle_sound (edict_t *self)
{
	if (random() < 0.1)
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void hknight_sword_sound (edict_t *self)
{
	//if (self->style)
	//	return;

	if (random() < 0.5)
		gi.sound (self, CHAN_VOICE, sound_sword1, 1, ATTN_IDLE, 0);
	//self->style = 1;
}


//
// STAND
//

mframe_t hknight_frames_stand [] =
{
	ai_stand, 0, hknight_idle_sound,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL		

};
mmove_t hknight_move_stand = {FRAME_stand1, FRAME_stand9, hknight_frames_stand, hknight_stand};

void hknight_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &hknight_move_stand;
}


//
// WALK
//

mframe_t hknight_frames_walk [] =
{
	ai_walk,	5,		NULL,
	ai_walk,	5,		NULL,
	ai_walk,	4,		NULL,
	ai_walk,	4,		NULL,
	ai_walk,	2,		NULL,
	ai_walk,	2,		NULL,
	ai_walk,	3,		NULL,
	ai_walk,	3,		NULL,
	ai_walk,	4,		NULL,
	ai_walk,	3,		NULL,
	ai_walk,	4,		NULL,
	ai_walk,	6,		NULL,
	ai_walk,	2,		NULL,
	ai_walk,	2,		NULL,
	ai_walk,	4,		NULL,
	ai_walk,	3,		NULL,
	ai_walk,	3,		NULL,
	ai_walk,	3,		NULL,
	ai_walk,	3,		NULL,
	ai_walk,	2,		NULL
};
mmove_t hknight_move_walk = {FRAME_walk1, FRAME_walk20, hknight_frames_walk, hknight_walk};

void hknight_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &hknight_move_walk;
}


//
// RUN
//
// can be improved

void hknight_check_dist (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		return;

	if (range(self, self->enemy) == RANGE_MELEE)
		hknight_melee(self);
	//else if (random() < 0.5)
	//	hknight_runatk(self);
}


mframe_t hknight_frames_run [] =
{
	ai_run,	16,		NULL,
	ai_run,	20,		NULL,
	ai_run,	13,		NULL,
	ai_run,	7,		NULL,
	ai_run,	16,		NULL,
	ai_run,	20,		NULL,
	ai_run,	14,		NULL,
	ai_run,	6,		hknight_check_dist
};
mmove_t hknight_move_run = {FRAME_run1, FRAME_run8, hknight_frames_run, NULL};


void hknight_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &hknight_move_stand;
	else
		self->monsterinfo.currentmove = &hknight_move_run;
}

/*mframe_t hknight_frames_attack_run [] =
{
	ai_charge,	20,		NULL,
	ai_charge,	20,		hknight_sword_sound,
	ai_charge,	13,		NULL,
	ai_charge,	7,		NULL,
	ai_charge,	16,		NULL,
	ai_charge,	20,		hknight_attack,
	ai_charge,	14,		NULL,
	ai_charge,	14,		NULL,
	ai_charge,	14,		NULL,
	ai_charge,	14,		NULL,
	ai_charge,	6,		hknight_check_dist
};
mmove_t hknight_move_attack_run = {FRAME_runattack1, FRAME_runattack11, hknight_frames_attack_run, hknight_run};

void hknight_attack_run (edict_t *self)
{
	self->monsterinfo.currentmove = &hknight_move_attack_run;
}*/

void hknight_melee_damage (edict_t *self)
{
	float 	ldmg;

	if (!self->enemy)
		return;		// removed before stroke
		
	if (PointDist(self->enemy->s.origin, self->s.origin) > 60)
		return;
		
	ldmg = (random() + random() + random()) * 3;
	T_Damage (self->enemy, self, self, vec3_origin, self->enemy->s.origin, vec3_origin, ldmg, 0, 0, MOD_UNKNOWN);
}

mframe_t hknight_frames_smash [] =
{
	ai_charge,	1,	NULL,
	ai_charge,	13,	NULL,
	ai_charge,	9,	NULL,
	ai_charge,	11,	NULL,
	ai_charge,	10,	hknight_melee_damage,
	ai_charge,	7,	hknight_melee_damage,
	ai_charge,	12,	hknight_melee_damage,
	ai_charge,	2,	hknight_melee_damage,
	ai_charge,	3,	hknight_melee_damage,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL
};
mmove_t hknight_smash_attack = {FRAME_smash1, FRAME_smash11, hknight_frames_smash, hknight_run};

void hknight_sword_swing (edict_t *self) 
{
	SWORDSOUND(self);
}

mframe_t hknight_frames_watk [] =
{
	ai_charge,	2,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	1,	NULL,
	ai_charge,	4,	NULL,
	ai_charge,	5,	NULL,
	ai_charge,	3,	hknight_melee_damage,
	ai_charge,	2,	hknight_melee_damage,
	ai_charge,	2,	hknight_melee_damage,
	ai_charge,	0,	hknight_sword_swing,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	1,	NULL,
	ai_charge,	1,	hknight_melee_damage,
	ai_charge,	3,	hknight_melee_damage,
	ai_charge,	4,	hknight_melee_damage,
	ai_charge,	6,	NULL,
	ai_charge,	7,	NULL,
	ai_charge,	3,	NULL
};
mmove_t hknight_watk_attack = {FRAME_w_attack1, FRAME_w_attack22, hknight_frames_watk, hknight_run};

mframe_t hknight_frames_srice [] =
{
	ai_charge,	9,	NULL,
	ai_charge,	6,	NULL,
	ai_charge,	13,	NULL,
	ai_charge,	4,	NULL,
	ai_charge,	7,	hknight_melee_damage,
	ai_charge,	15,	hknight_melee_damage,
	ai_charge,	8,	hknight_melee_damage,
	ai_charge,	2,	hknight_melee_damage,
	ai_charge,	0,	hknight_melee_damage,
	ai_charge,	3,	NULL
};
mmove_t hknight_slice_attack = {FRAME_slice1, FRAME_slice10, hknight_frames_srice, hknight_run};

void hknight_melee (edict_t *self)
{
//	int hknight_type = (self->s.frame >= 121 ? (self->s.frame >= 133 ? 3 : 2) : 1) - 1;
	
	SWORDSOUND(self);

	if (self->dmg == 0)
	{
		self->monsterinfo.currentmove = &hknight_slice_attack;
		self->dmg++;
	}
	else if (self->dmg == 1)
	{
		self->monsterinfo.currentmove = &hknight_smash_attack;
		self->dmg++;
	}
	else if (self->dmg == 2)
	{
		self->monsterinfo.currentmove = &hknight_watk_attack;
		self->dmg = 0;
	}
}


//
// PAIN
//

mframe_t hknight_frames_pain1 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t hknight_move_pain1 = {FRAME_pain1, FRAME_pain5, hknight_frames_pain1, hknight_run};

void hknight_pain (edict_t *self, edict_t *other, float kick, int damage)
{	
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 1.1;

	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);

	self->monsterinfo.currentmove = &hknight_move_pain1;
}


//
// DEATH
//

void hknight_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

void hknight_nogib (edict_t *self)
{
	self->gib_health = -10000;
}

mframe_t hknight_frames_death1 [] =
{
	ai_move,	10,	NULL,
	ai_move,	8,	NULL,
	ai_move,	7,	hknight_nogib,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	10,	NULL,
	ai_move,	11,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t hknight_move_death1 = {FRAME_death1, FRAME_death12, hknight_frames_death1, hknight_dead};


mframe_t hknight_frames_death2 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	hknight_nogib,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t hknight_move_death2 = {FRAME_deathb1, FRAME_deathb9, hknight_frames_death2, hknight_dead};

void hknight_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// check for gib
	if (self->health <= self->gib_health && !(self->spawnflags & SF_MONSTER_NOGIB))
	{
		gi.sound (self, CHAN_VOICE|CHAN_RELIABLE, sound_gib, 1, ATTN_NORM, 0);
		
		for (n = 0; n < 2; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib3/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowHead(self, "models/monsters/q1hknight/head/tris.md2", 0, 0, damage, GIB_ORGANIC);
		
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
		self->monsterinfo.currentmove = &hknight_move_death1;
	else
		self->monsterinfo.currentmove = &hknight_move_death2;
}

#define enemy_vis visible(self, self->enemy)

mframe_t knight_charge1_frames [] =
{
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL
};
mmove_t knight_charge1_attack = {FRAME_char_a1, FRAME_char_a16, knight_charge1_frames, hknight_run};

void hknight_check_charge (edict_t *self)
{
	if (!enemy_vis)
		return;
	if (fabs(self->s.origin[2] - self->enemy->s.origin[2]) > 20)
		return;
	if (PointDist(self->s.origin, self->enemy->s.origin) < 80)
		return;

	// Charge
	self->monsterinfo.currentmove = &knight_charge1_attack;
}

void hknight_fire_flame (edict_t *self)
{
	int		lawl[] = {-2, -1, 0, 1, 2, 3};
	int		rem = 165;
	int		arraynum = rem-self->s.frame;
	float	offang_y = 0;
	vec3_t	vec, forward, right, start;
	vec3_t	offang, v_forward, temp;

	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet (temp, 0, 0, 14);
	G_ProjectSource (self->s.origin, temp, forward, right, start);

	VectorSet (temp, 0, lawl[arraynum], 0);
	VectorSubtract (self->s.angles, temp, vec);
//	gi.dprintf("%i, %i\n", lawl[arraynum], arraynum);
	AngleVectors (vec, forward, NULL, NULL);

	VectorSubtract (self->enemy->s.origin, self->s.origin, offang);
	vectoangles (offang, offang);
	offang_y = self->s.angles[1] + -lawl[arraynum] * 6;
	offang[1] = offang_y;

	AngleVectors (offang, v_forward, NULL, NULL);

	//vec = VectorNormalizeFastf(

	gi.sound (self, CHAN_WEAPON, sound_flame, 1, ATTN_NORM, 0);

	q1_fire_flame (self, start, v_forward, 0);
}

mframe_t hknight_frames_magic [] =
{
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	hknight_fire_flame,
	ai_charge,	0,	hknight_fire_flame,
	ai_charge,	0,	hknight_fire_flame,
	ai_charge,	0,	hknight_fire_flame,
	ai_charge,	0,	hknight_fire_flame,
	ai_charge,	0,	hknight_fire_flame
};
mmove_t hknight_magic_attack = { FRAME_magicc1, FRAME_magicc11, hknight_frames_magic, hknight_run};

void hknight_magic (edict_t *self)
{
	self->monsterinfo.currentmove = &hknight_magic_attack;
}


//
// SPAWN
//

/*QUAKED SP_monster_q1_hknight (1 .5 0) (-16 -16 -24) (16 16 40) Ambush Trigger_Spawn Sight
model="models/monsters/q1hknight/tris.md2"
*/
void SP_monster_q1_hknight (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_death =	gi.soundindex ("q1hknight/death1.wav");
	sound_gib =		gi.soundindex ("q1player/udeath.wav");
	sound_pain =	gi.soundindex ("q1hknight/pain1.wav");
	sound_idle =	gi.soundindex ("q1hknight/idle.wav");
	sound_sight =	gi.soundindex ("q1hknight/sight1.wav");
	sound_sword1 =	gi.soundindex ("q1hknight/slash1.wav");
	sound_sword2 =	gi.soundindex ("q1hknight/slash1.wav");
	sound_flame =	gi.soundindex ("q1hknight/attack1.wav");

	// precache gibs
	gi.modelindex ("models/monsters/q1hknight/head/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib1/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib3/tris.md2");
	// precache flame
	q1_flame_precache ();
	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	// Lazarus: special purpose skins
	if ( self->style )
	{
		PatchMonsterModel("models/monsters/q1hknight/tris.md2");
		self->s.skinnum = self->style;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/q1hknight/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 40);

	if (!self->health)
		self->health = 250;
	if (!self->gib_health)
		self->gib_health = -40;
	if (!self->mass)
		self->mass = 175;

	self->pain = hknight_pain;
	self->die = hknight_die;

	self->flags |= FL_Q1_MONSTER;

	self->monsterinfo.stand = hknight_stand;
	self->monsterinfo.walk = hknight_walk;
	self->monsterinfo.run = hknight_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = hknight_magic; 
	self->monsterinfo.melee = hknight_melee;
	self->monsterinfo.sight = hknight_sight;
	self->monsterinfo.search = hknight_stand;
	self->monsterinfo.idle = hknight_stand;

	self->dmg = rand()%2;

	if (!self->monsterinfo.flies)
		self->monsterinfo.flies = 0.30;

	// Lazarus
	if (self->powerarmor)
	{
		if (self->powerarmortype == 1)
			self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
		else
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		self->monsterinfo.power_armor_power = self->powerarmor;
	}

	self->common_name = "Hell Knight";
	self->class_id = ENTITY_MONSTER_Q1_HKNIGHT;

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &hknight_move_stand;
	if (self->health < 0)
	{
		mmove_t	*deathmoves[] = {&hknight_move_death1,
			                     &hknight_move_death2,
								 NULL};
		M_SetDeath (self, (mmove_t **)&deathmoves);
	}
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
