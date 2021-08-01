/*
==============================================================================

QUAKE ENFORCER

==============================================================================
*/

#include "g_local.h"
#include "m_q1enforcer.h"

static int	sound_pain1;
static int	sound_pain2;
static int	sound_death;
static int	sound_gib;
static int	sound_idle;
static int	sound_sight1;
static int	sound_sight2;
static int	sound_sight3;
static int	sound_sight4;
static int	sound_fire;


void q1enforcer_fire (edict_t *self);
void q1enforcer_stand (edict_t *self);

/*int q1enforcer_rndidle ()
{
	char str[256];

	Com_sprintf (str, sizeof(str), "q1enforcer/sight%i.wav", rand()%3+1);

	return gi.soundindex(str);
}*/

void q1enforcer_idlesound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void q1enforcer_sight (edict_t *self, edict_t *other)
{
	float r = random();

	if (r < 0.25)
		gi.sound (self, CHAN_VOICE, sound_sight1, 1, ATTN_NORM, 0);
	else if (r < 0.5)
		gi.sound (self, CHAN_VOICE, sound_sight2, 1, ATTN_NORM, 0);
	else if (r < 0.75)
		gi.sound (self, CHAN_VOICE, sound_sight3, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_sight4, 1, ATTN_NORM, 0);
}

void q1enforcer_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
//	gi.sound (self, CHAN_VOICE, q1enforcer_rndidle(), 1, ATTN_IDLE, 0);
}

mframe_t q1enforcer_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	q1enforcer_move_stand = {FRAME_stand1, FRAME_stand7, q1enforcer_frames_stand, NULL};

void q1enforcer_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &q1enforcer_move_stand;
}

mframe_t q1enforcer_frames_walk [] =
{
	ai_walk, 3, NULL,
	ai_walk, 4, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 4, NULL,
	ai_walk, 3, NULL,
	ai_walk, 2, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 2, NULL,
	ai_walk, 3, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 4, NULL,
	ai_walk, 3, NULL
};
mmove_t q1enforcer_move_walk = {FRAME_walk1, FRAME_walk16, q1enforcer_frames_walk, NULL};

void q1enforcer_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &q1enforcer_move_walk;
}

mframe_t q1enforcer_frames_run [] =
{
	ai_run, 22, NULL,
	ai_run, 8,  NULL,
	ai_run, 7,  NULL,
	ai_run, 8,  NULL,
	ai_run, 13, NULL,
	ai_run, 12, NULL,
	ai_run, 11, NULL,
	ai_run, 7,  NULL
};

mmove_t q1enforcer_move_run = {FRAME_run1, FRAME_run8, q1enforcer_frames_run, NULL};

void q1enforcer_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &q1enforcer_move_stand;
	else
		self->monsterinfo.currentmove = &q1enforcer_move_run;
}

mframe_t q1enforcer_frames_pain3 [] =
{
	ai_move, 0, NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL
};
mmove_t q1enforcer_move_pain3 = {FRAME_paina1, FRAME_paina4, q1enforcer_frames_pain3, q1enforcer_run};

mframe_t q1enforcer_frames_pain2 [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0, NULL
};
mmove_t q1enforcer_move_pain2 = {FRAME_painb1, FRAME_painb5, q1enforcer_frames_pain2, q1enforcer_run};

mframe_t q1enforcer_frames_pain1 [] =
{
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0, NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0, NULL
};
mmove_t q1enforcer_move_pain1 = {FRAME_painc1, FRAME_painc8, q1enforcer_frames_pain1, q1enforcer_run};

void q1enforcer_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if (rand()&1)
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);

	if (damage <= 10)
		self->monsterinfo.currentmove = &q1enforcer_move_pain3;
	else if (damage <= 25)
		self->monsterinfo.currentmove = &q1enforcer_move_pain2;
	else
		self->monsterinfo.currentmove = &q1enforcer_move_pain1;
}

void q1enforcer_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

void q1enforcer_throw_backpack (edict_t *self) 
{
	edict_t	*backpack;

	if (self->health <= self->gib_health)
		return;

	backpack = Drop_Q1Backpack (self, FindItemByClassname("ammo_cells"), 5);
/*	backpack = Drop_Item(self, FindItemByClassname("item_q1_backpack"));
	backpack->item = FindItemByClassname("ammo_cells");
	backpack->count = 5;
	backpack->touch = Touch_Item;
	backpack->nextthink = level.time + 1800;
	backpack->think = G_FreeEdict;
*/
	self->gib_health = -10000;
}

mframe_t q1enforcer_frames_death1 [] =
{
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	q1enforcer_throw_backpack,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0, NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL
};
mmove_t q1enforcer_move_death1 = {FRAME_death1, FRAME_death14, q1enforcer_frames_death1, q1enforcer_dead};

mframe_t q1enforcer_frames_death2 [] =
{
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	q1enforcer_throw_backpack,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0, NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL
};
mmove_t q1enforcer_move_death2 = {FRAME_fdeath1, FRAME_fdeath11, q1enforcer_frames_death2, q1enforcer_dead};


void q1enforcer_whichway (edict_t *self);

void q1enforcer_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// check for gib
	if (self->health <= self->gib_health && !(self->spawnflags & SF_MONSTER_NOGIB))
	{
		gi.sound (self, CHAN_VOICE|CHAN_RELIABLE, sound_gib, 1, ATTN_NORM, 0);

		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/q1gibs/q1gib2/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/q1enforcer/head/tris.md2", 0, 0, damage, GIB_ORGANIC);
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
		self->monsterinfo.currentmove = &q1enforcer_move_death1;
	else
		self->monsterinfo.currentmove = &q1enforcer_move_death2;
}

void q1enforcer_fire (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start, target, aim;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_SOLDIER_BLASTER_1], forward, right, start);

	// project enemy back a bit and target there
	VectorCopy (self->enemy->s.origin, target);
	VectorMA (target, -0.2, self->enemy->velocity, target);
	target[2] += self->enemy->viewheight;

	VectorSubtract (target, start, aim);
	VectorNormalize (aim);

	gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, sound_fire, 1, ATTN_NORM, 0);
	q1_fire_laser (self, start, aim, 15, 600);
}

void q1enforcer_secondfire (edict_t *self)
{
	if (!self->radius_dmg)
	{
		self->monsterinfo.nextframe = 36;
		self->radius_dmg = 1;
	}
	else
		self->radius_dmg = 0;
}

mframe_t q1enforcer_frames_start_chain [] =
{
	ai_charge,   0, NULL,
	ai_charge,   1, NULL,
	ai_charge,   1, NULL,
	ai_charge,   0, NULL,
	ai_charge,   0, NULL,
	ai_charge, 0, q1enforcer_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, q1enforcer_secondfire,
	ai_charge, 0, NULL
};
mmove_t q1enforcer_move_start_fire = {FRAME_attack1, FRAME_attack10, q1enforcer_frames_start_chain, q1enforcer_run};

void q1enforcer_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &q1enforcer_move_start_fire;
}


//
// SPAWN
//

/*QUAKED SP_monster_q1_enforcer (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
model="models/monsters/q1enforcer/tris.md2"
*/
void SP_monster_q1_enforcer (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_pain1		= gi.soundindex ("q1enforcer/pain1.wav");
	sound_pain2		= gi.soundindex ("q1enforcer/pain2.wav");
	sound_death		= gi.soundindex ("q1enforcer/death1.wav");
	sound_gib		= gi.soundindex ("q1player/udeath.wav");
	sound_idle		= gi.soundindex ("q1enforcer/idle1.wav");
	sound_sight1	= gi.soundindex ("q1enforcer/sight1.wav");
	sound_sight2	= gi.soundindex ("q1enforcer/sight2.wav");
	sound_sight3	= gi.soundindex ("q1enforcer/sight3.wav");
	sound_sight4	= gi.soundindex ("q1enforcer/sight4.wav");
	sound_fire		= gi.soundindex ("q1enforcer/enfire.wav");

	// precache backpack
	gi.modelindex ("models/items/q1backpack/tris.md2");
//	gi.soundindex ("q1weapons/lock4.wav");
	// precache gibs
	gi.modelindex ("models/monsters/q1enforcer/head/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib1/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib3/tris.md2");
	// precache laser
	q1_laser_precache ();

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	// Lazarus: special purpose skins
	if ( self->style )
	{
		PatchMonsterModel("models/monsters/q1enforcer/tris.md2");
		self->s.skinnum = self->style;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/q1enforcer/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 40);

	if (!self->health)
		self->health = 80;
	if (!self->gib_health)
		self->gib_health = -35;
	if (!self->mass)
		self->mass = 175;

	self->flags |= FL_Q1_MONSTER;

	self->pain = q1enforcer_pain;
	self->die = q1enforcer_die;


	self->monsterinfo.stand = q1enforcer_stand;
	self->monsterinfo.walk = q1enforcer_walk;
	self->monsterinfo.run = q1enforcer_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = q1enforcer_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = q1enforcer_sight;
	self->monsterinfo.search = q1enforcer_search;

	if (!self->monsterinfo.flies)
		self->monsterinfo.flies = 0.10;

	// Lazarus
	if (self->powerarmor)
	{
		if (self->powerarmortype == 1)
			self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
		else
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		self->monsterinfo.power_armor_power = self->powerarmor;
	}

	self->common_name = "Enforcer";
	self->class_id = ENTITY_MONSTER_Q1_ENFORCER;

	gi.linkentity (self);

	gi.linkentity (self);

	self->monsterinfo.currentmove = &q1enforcer_move_stand;	
	if (self->health < 0)
	{
		mmove_t	*deathmoves[] = {&q1enforcer_move_death1,
			                     &q1enforcer_move_death2,
								 NULL};
		M_SetDeath (self, (mmove_t **)&deathmoves);
	}
	self->monsterinfo.scale = MODEL_SCALE;
	
	self->radius_dmg = 0;

	walkmonster_start (self);
}
