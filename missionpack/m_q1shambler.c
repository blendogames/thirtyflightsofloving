/*
==============================================================================

QUAKE SHAMBLER

==============================================================================
*/

#include "g_local.h"
#include "m_q1shambler.h"


static int  sound_melee1;
static int  sound_melee2;
static int	sound_melee3;
static int	sound_attack;
static int  sound_boom;  // ?
static int	sound_pain;
static int	sound_death;
static int	sound_gib;
static int	sound_idle;
static int	sound_sight;

void shambler_attack(edict_t *self);

void shambler_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	shambler_attack(self);
}

/*static*/ void shambler_idle_sound (edict_t *self)
{
	if (!self->enemy && random() > 0.8)
	gi.sound (self, CHAN_VOICE, sound_idle, 0.8, ATTN_IDLE, 0);
}

/*static*/ void shambler_melee_sound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_melee1, 1, ATTN_NORM, 0);
}

/*static*/ void shambler_melee_sound2 (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_melee2, 1, ATTN_NORM, 0);
}

/*static*/ void shambler_attack_sound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_attack, 1, ATTN_NORM, 0);
}

// STAND

void shambler_stand (edict_t *self);

mframe_t shambler_frames_stand [] =
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
	ai_stand, 0, NULL
};

mmove_t	shambler_move_stand = {FRAME_stand1, FRAME_stand17, shambler_frames_stand, shambler_stand};

void shambler_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &shambler_move_stand;
}

// WALK

void shambler_walk (edict_t *self);

mframe_t shambler_frames_walk [] =
{
	ai_walk, 10, shambler_idle_sound,
	ai_walk, 9, NULL,
	ai_walk, 9, NULL,
	ai_walk, 5, NULL,
	ai_walk, 6, NULL,
	ai_walk, 12, NULL,
	ai_walk, 8, NULL,
	ai_walk, 3, NULL,
	ai_walk, 13, NULL,
	ai_walk, 9, NULL,
	ai_walk, 7, NULL,
	ai_walk, 7, NULL
};
mmove_t shambler_move_walk = {FRAME_walk1, FRAME_walk12, shambler_frames_walk, shambler_walk};

void shambler_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &shambler_move_walk;
}

//RUN

void shambler_run (edict_t *self);

mframe_t shambler_frames_run [] =
{
	ai_run, 20, shambler_idle_sound,
	ai_run, 24, NULL,
	ai_run, 20, NULL,
	ai_run, 20, NULL,
	ai_run, 24, NULL,
	ai_run, 20, shambler_attack
};

mmove_t shambler_move_run = {FRAME_run1, FRAME_run6, shambler_frames_run, shambler_run};

void shambler_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &shambler_move_stand;
	else
		self->monsterinfo.currentmove = &shambler_move_run;
}



mframe_t shambler_frames_pain [] =
{
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t shambler_move_pain = {FRAME_pain1, FRAME_pain6, shambler_frames_pain, shambler_run};


void shambler_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	if (self->health <= 0)
		return;

	if (self->health > 0)
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);	

	if (random() * 400 > damage)
		return;

	self->monsterinfo.currentmove = &shambler_move_pain;
	self->pain_debounce_time = level.time + 2;
}



void shambler_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

void shambler_nogib (edict_t *self)
{
	self->gib_health = -10000;
}

mframe_t shambler_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 shambler_nogib,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t shambler_move_death = {FRAME_death1, FRAME_death11, shambler_frames_death, shambler_dead};


void shambler_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// check for gib
	if (self->health <= self->gib_health && !(self->spawnflags & SF_MONSTER_NOGIB))
	{
		gi.sound (self, CHAN_VOICE|CHAN_RELIABLE, sound_gib, 1, ATTN_NORM, 0);
		for (n = 0; n < 2; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", 0, 0, damage, GIB_ORGANIC);
		for (n = 0; n < 4; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib3/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/q1gibs/q1gib2/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/q1shambler/head/tris.md2", 0, 0, damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &shambler_move_death;
}



// ATTACK


void shambler_swing_left (edict_t *self)
{
	vec3_t	aim;

	if (!self->enemy)
		return;

	VectorSet (aim, MELEE_DISTANCE*1.2, self->mins[0], 8);
	if (fire_hit (self, aim, ((random() + random() + random()) * 20), 100))
		gi.sound (self, CHAN_VOICE, sound_melee3, 1, ATTN_NORM, 0); //WAS ATTN_IDLE
}


mframe_t shambler_frames_swingL [] =
{
	ai_charge, 5, shambler_melee_sound2,
	ai_charge, 3, NULL,
	ai_charge, 7, NULL,
	ai_charge, 3, NULL,
	ai_charge, 7, NULL,
	ai_charge, 9, NULL,
	ai_charge, 5, shambler_swing_left,
	ai_charge, 4, NULL,
	ai_charge, 8, shambler_attack
};
mmove_t shambler_move_swingL_attack = {FRAME_swingl1, FRAME_swingl9, shambler_frames_swingL, shambler_run};

void shambler_swing_right (edict_t *self)
{
	vec3_t	aim;

	if (!self->enemy)
		return;

	VectorSet (aim, MELEE_DISTANCE*1.2, self->maxs[0], 8);
	if (fire_hit (self, aim, ((random() + random() + random()) * 20), 100))
		gi.sound (self, CHAN_VOICE, sound_melee3, 1, ATTN_NORM, 0); //WAS ATTN_IDLE
}

mframe_t shambler_frames_swingR [] =
{
	ai_charge, 1, shambler_melee_sound,
	ai_charge, 8, NULL,
	ai_charge, 14,NULL,
	ai_charge, 7,  NULL,
	ai_charge, 3, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, shambler_swing_right,
	ai_charge, 3, NULL,
	ai_charge, 10,shambler_attack
};
mmove_t shambler_move_swingR_attack = {FRAME_swingr1, FRAME_swingr9, shambler_frames_swingR, shambler_run};


void shambler_smash (edict_t *self)
{
	vec3_t	aim;

	if (!self->enemy)
		return;

	VectorSet (aim, MELEE_DISTANCE*1.2, self->maxs[0], self->maxs[2]);
	if (fire_hit (self, aim, ((random() + random() + random()) * 40), 100))
		gi.sound (self, CHAN_WEAPON, sound_melee3, 1, ATTN_NORM, 0);
}

mframe_t shambler_frames_smash [] =
{
	ai_charge, 2, shambler_melee_sound,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 5, NULL,
	ai_charge, 4, NULL,
	ai_charge, 1, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, shambler_smash,
	ai_charge, 5, NULL,
	ai_charge, 4, shambler_attack
};
mmove_t shambler_move_smash_attack = {FRAME_smash1, FRAME_smash12, shambler_frames_smash, shambler_run};


static qboolean shambler_magic_attack_ok (vec3_t start, vec3_t end)
{
	vec3_t	dir, angles;

	// check for max distance
	VectorSubtract (start, end, dir);
	if (VectorLength(dir) > 600)
		return false;

	// check for min/max pitch
	vectoangles (dir, angles);
	if (angles[0] < -180)
		angles[0] += 360;
	if (fabs(angles[0]) > 30)
		return false;

	return true;
}

// Added from Decino's Q2Infighter code
void shambler_sparks (edict_t *self)
{
	vec3_t	spark_pt;

	VectorCopy (self->s.origin, spark_pt);
	spark_pt[2] += 80.0f;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WELDING_SPARKS);
	gi.WriteByte (15);
	gi.WritePosition (spark_pt);
	gi.WriteDir (vec3_origin);
	gi.WriteByte (15);
	gi.multicast (spark_pt, MULTICAST_PVS);
}


void shambler_magic_attack (edict_t *self)
{
	vec3_t	offset, start, f, r, end, dir;
	trace_t	tr;
	int		damage = 10;

	if (!self->enemy)
		return;

	// Skip last lighting frame if not on nightmare skill
	if ( (self->s.frame == 74) && (skill->value < 3) )
		return;

	AngleVectors (self->s.angles, f, r, NULL);
	VectorSet (offset, 28, 0, 40);
	G_ProjectSource (self->s.origin, offset, f, r, start);

	VectorCopy (self->enemy->s.origin, end);
	if (!shambler_magic_attack_ok(start, end))
	{
		end[2] = self->enemy->s.origin[2] + self->enemy->maxs[2] - 8;
		if (!shambler_magic_attack_ok(start, end))
		{
			end[2] = self->enemy->s.origin[2] + self->enemy->mins[2] + 8;
			if (!shambler_magic_attack_ok(start, end))
			{
				shambler_run(self);
				return;
			}
		}
	}
	VectorCopy (self->enemy->s.origin, end);
	
	tr = gi.trace (start, NULL, NULL, end, self, (MASK_SHOT|CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA));
	if (tr.ent != self->enemy)
		return;

	if (self->s.frame == FRAME_magic6)
		gi.sound (self, CHAN_WEAPON, sound_boom, 1, ATTN_NORM, 0);

#if 1	// From Decino's Q2Infighter mod
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_LIGHTNING);
	gi.WriteShort (tr.ent - g_edicts);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (end);
	gi.WritePosition (start);
	gi.multicast (self->s.origin, MULTICAST_PVS);
#else
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_PARASITE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (start);
	gi.WritePosition (end);
	gi.multicast (self->s.origin, MULTICAST_PVS);
#endif

	VectorSubtract (start, end, dir);
	T_Damage (self->enemy, self, self, dir, self->enemy->s.origin, vec3_origin, damage, 0, 0, 0);
}


void shambler_skill3 (edict_t *self)
{
	if (skill->value == 3)
		shambler_attack(self);
}


mframe_t shambler_frames_magic_attack[] =
{
	ai_charge, 0, shambler_attack_sound,
	ai_charge, 0, shambler_sparks,	// NULL,
	ai_charge, 0, shambler_sparks,	// NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, shambler_magic_attack,
	ai_charge, 0, shambler_magic_attack,
	ai_charge, 0, shambler_magic_attack,
	ai_charge, 0, shambler_magic_attack,
	ai_charge, 0, shambler_magic_attack,
	ai_charge, 0, shambler_skill3,
	ai_charge, 0, shambler_attack
};
mmove_t shambler_move_magic_attack = {FRAME_magic1, FRAME_magic12, shambler_frames_magic_attack, shambler_run};


void shambler_attack (edict_t *self)
{
	float len;
	vec3_t v;
	
	//int i;
	//i = range (self, self->enemy);
	
	if (level.intermissiontime && !Q_stricmp(level.mapname, "bbelief9")) // don't attack intermission cam
		return;

	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		return;
	
	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	len = VectorLength (v);
	
	if (len < 128)
	{
		if (self->health > 500 && random() > 0.6)
			self->monsterinfo.currentmove = &shambler_move_smash_attack;
		else
		{
			if (random() > 0.5)
				self->monsterinfo.currentmove = &shambler_move_swingL_attack;
			else
				self->monsterinfo.currentmove = &shambler_move_swingR_attack;
		}
	}
	else if ((len > 196 && len < 1024) && infront(self,self->enemy))
	{
		self->monsterinfo.currentmove = &shambler_move_magic_attack;
	}
	else
	{
		self->monsterinfo.currentmove = &shambler_move_run;
	}
}

void shambler_melee(edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
	{
	//	if (random() > 0.5)
	//		self->monsterinfo.currentmove = &shambler_move_walk;
		return;
	}
	
	if (range (self, self->enemy) == RANGE_MELEE)
	{
		if (self->health > 500 && random() > 0.6)
			self->monsterinfo.currentmove = &shambler_move_smash_attack;
		else
		{
			if (random() > 0.5)
				self->monsterinfo.currentmove = &shambler_move_swingL_attack;
			else
				self->monsterinfo.currentmove = &shambler_move_swingR_attack;
		}
	}
}


//
// SPAWN
//

/*QUAKED monster_q1_shambler (1 .5 0) (-32 -32 -24) (32 32 64) Ambush Trigger_Spawn Sight
model="models/monsters/q1shambler/tris.md2"
*/
void SP_monster_q1_shambler (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_melee1 =	gi.soundindex ("q1shambler/melee1.wav");	
	sound_melee2 =	gi.soundindex ("q1shambler/melee1.wav");	
	sound_melee3 =	gi.soundindex ("q1shambler/smack.wav");	
	sound_attack =	gi.soundindex ("q1shambler/sattck1.wav");			
	sound_boom =	gi.soundindex ("q1shambler/sboom.wav");
	sound_pain =	gi.soundindex ("q1shambler/shurt2.wav");	
	sound_death =	gi.soundindex ("q1shambler/sdeath.wav");			
	sound_gib =		gi.soundindex ("q1player/udeath.wav");
	sound_idle =	gi.soundindex ("q1shambler/sidle.wav");			
	sound_sight =	gi.soundindex ("q1shambler/ssight.wav");

	// precache gibs
	gi.modelindex ("models/monsters/q1shambler/head/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib1/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib2/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib3/tris.md2");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	// Lazarus: special purpose skins
	if ( self->style )
	{
		PatchMonsterModel("models/monsters/q1shambler/tris.md2");
		self->s.skinnum = self->style;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/q1shambler/tris.md2");
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 64);

	if (!self->health)
		self->health = 600;
	if (!self->gib_health)
		self->gib_health = -60;
	if (!self->mass)
		self->mass = 600;

	self->pain = shambler_pain;
	self->die = shambler_die;

	self->flags |= FL_Q1_MONSTER;

	self->monsterinfo.stand = shambler_stand;
	self->monsterinfo.walk = shambler_walk;
	self->monsterinfo.run = shambler_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = shambler_attack;
	self->monsterinfo.melee = shambler_melee;
	self->monsterinfo.sight = shambler_sight;
	self->monsterinfo.search = shambler_stand;

	if (!self->monsterinfo.flies)
		self->monsterinfo.flies = 0.50;

	// Lazarus
	if (self->powerarmor)
	{
		if (self->powerarmortype == 1)
			self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
		else
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		self->monsterinfo.power_armor_power = self->powerarmor;
	}

	self->common_name = "Shambler";
	self->class_id = ENTITY_MONSTER_Q1_SHAMBLER;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &shambler_move_stand;	
	if (self->health < 0)
	{
		mmove_t	*deathmoves[] = {&shambler_move_death,
								 NULL};
		M_SetDeath (self, (mmove_t **)&deathmoves);
	}
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
