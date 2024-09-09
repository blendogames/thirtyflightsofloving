/*
==============================================================================

CHTHON !!!!!! HE LIVES !

==============================================================================
*/

#include "g_local.h"
#include "m_q1chthon.h"

#define SF_CHTHON_LIGHTNING_ONLY	32
#define SF_CHTHON_MOBILE			64

void monster_use (edict_t *self, edict_t *other, edict_t *activator);
void chthon_run (edict_t *self);
void chthon_set_dead (edict_t *self);
void chthon_check_attack (edict_t *self);

static int	sound_pain;
static int	sound_death;
static int	sound_sight;
static int  sound_throw;
static int  sound_rise;


void chthon_lava_splash (edict_t *self)
{
#ifdef KMQUAKE2_ENGINE_MOD
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_LAVASPLASH_Q1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
#else	// KMQUAKE2_ENGINE_MOD
	int count = 60;
	int i;
	float rx, ry, r1, r2;
	vec3_t splashorigin;
	vec3_t MOVEDIR_UP	= {0, 0, 1};
//	vec3_t MOVEDIR_DOWN	= {0, 0, -1};

	for (i = 0; i < 80; i++)
	{
		rx = random() * 128;
		ry = random() * 128;
		r1 = random();
		r2 = random();
		VectorCopy (self->s.origin, splashorigin);
		if (r1 > 0.5)
			splashorigin[0] += rx;
		else
			splashorigin[0] -= rx;
		if (r2 > 0.5)
			splashorigin[1] += ry;
		else
			splashorigin[1] -= ry;
		splashorigin[2] += 8;
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SPLASH);
		gi.WriteByte (count);
		gi.WritePosition (splashorigin);
		gi.WriteDir (MOVEDIR_UP);
		gi.WriteByte (SPLASH_LAVA);
		gi.multicast (splashorigin, MULTICAST_PVS);
	}
#endif	// KMQUAKE2_ENGINE_MOD
}

/*static*/ void chthon_rise_sound (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_rise, 1, ATTN_NONE, 0);
}

void chthon_sink (edict_t *self)
{
	chthon_rise_sound (self);
	chthon_lava_splash (self);
}

/*static*/ void chthon_sight_sound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

/*static*/ void chthon_sight_sound2 (edict_t *self)
{
	if (random() < 0.1)
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

mframe_t chthon_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, chthon_check_attack,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, chthon_check_attack,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, chthon_check_attack,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, chthon_sight_sound,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, chthon_check_attack,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, chthon_check_attack,
	ai_stand, 0, NULL
};
mmove_t chthon_move_stand = {FRAME_walk1, FRAME_walk31, chthon_frames_stand, NULL};

void chthon_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &chthon_move_stand;
}

void chthon_frameend (edict_t *self)
{
//	gi.dprintf ("RISE:%d\n", self->s.frame);
}
void chthon_framemid( edict_t *self)
{
//	gi.dprintf ("RISE:%d\n", self->s.frame);
}
void chthon_framestart (edict_t *self)
{
//	gi.dprintf ("RISE:%d\n", self->s.frame);
}

mframe_t chthon_frames_rise [] =
{
	ai_move, 0, NULL,
	ai_move, 0, chthon_rise_sound,
	ai_move, 0, chthon_sight_sound,
	ai_move, 0, NULL,	// was chthon_framestart
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,	// was chthon_framemid
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,	// was chthon_frameend
	ai_move, 0, NULL	// was chthon_stand
};

mmove_t	chthon_move_rise = {FRAME_rise1, FRAME_rise17, chthon_frames_rise, chthon_stand};

void chthon_rise (edict_t *self)
{
//	gi.dprintf ("RISE chthon!\n");
	chthon_lava_splash (self); // added lava splash
	self->monsterinfo.currentmove = &chthon_move_rise;
}

mframe_t chthon_frames_walk_mobile [] =
{
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, chthon_sight_sound2,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, chthon_check_attack,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, chthon_check_attack
};
mmove_t chthon_move_walk_mobile = {FRAME_walk1, FRAME_walk31, chthon_frames_walk_mobile, NULL};

mframe_t chthon_frames_walk [] =
{
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, chthon_sight_sound2,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, chthon_check_attack,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, NULL,
	ai_walk, 0, chthon_check_attack
};
mmove_t chthon_move_walk = {FRAME_walk1, FRAME_walk31, chthon_frames_walk, NULL};

void chthon_walk (edict_t *self)
{
	if (self->spawnflags & SF_CHTHON_MOBILE)	// Chthon actually moves
		self->monsterinfo.currentmove = &chthon_move_walk_mobile;
	else
		self->monsterinfo.currentmove = &chthon_move_walk;
}

mframe_t chthon_frames_run_mobile [] =
{
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, chthon_sight_sound2,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, chthon_check_attack,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, chthon_check_attack,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, NULL,
	ai_charge, 6, chthon_check_attack
};
mmove_t chthon_move_run_mobile = {FRAME_walk1, FRAME_walk31, chthon_frames_walk_mobile, NULL};

mframe_t chthon_frames_run [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, chthon_sight_sound2,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, chthon_check_attack,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, chthon_check_attack,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, chthon_check_attack
};
mmove_t chthon_move_run = {FRAME_walk1, FRAME_walk31, chthon_frames_walk, NULL};

void chthon_run (edict_t *self)
{
	if (self->spawnflags & SF_CHTHON_MOBILE)	// Chthon actually moves
		self->monsterinfo.currentmove = &chthon_move_run_mobile;
	else
		self->monsterinfo.currentmove = &chthon_move_run;
}

mframe_t chthon_frames_shock1 [] =
{
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 chthon_check_attack
};
mmove_t chthon_frames_move_shock1 = {FRAME_shocka1, FRAME_shocka10, chthon_frames_shock1, chthon_walk};

mframe_t chthon_frames_shock2 [] =
{
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,  chthon_check_attack
};
mmove_t chthon_frames_move_shock2 = {FRAME_shockb1, FRAME_shockb6, chthon_frames_shock2, chthon_walk};

mframe_t chthon_frames_shock3 [] =
{
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 chthon_check_attack
};
mmove_t chthon_frames_move_shock3 = {FRAME_shockc1, FRAME_shockc10, chthon_frames_shock3, chthon_walk};

mframe_t chthon_frames_shock3_die [] =
{
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t chthon_frames_move_shock3_die = {FRAME_shockc1, FRAME_shockc10, chthon_frames_shock3_die, chthon_set_dead};

void chthon_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		 health_1_4, health_2_4, health_3_4;

	if (level.time < self->pain_debounce_time)
		return;

	health_1_4 = self->max_health * (1.0f/4.0f);
	health_2_4 = self->max_health * (2.0f/4.0f);
	health_3_4 = self->max_health * (3.0f/4.0f);

/*	if ( (Q_stricmp(level.mapname, "qe1m7") == 0) && (stricmp(other->classname, "target_q1_bolt") == 0) ) {
	//	gi.dprintf("HIT BY BOLT");
		self->pain_debounce_time = level.time + 5;
	} */

	// no pain sound or anim for small amounts of damage
	if (damage >= 100)
	{
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);

		if (self->health <= health_1_4) {
			self->monsterinfo.currentmove = &chthon_frames_move_shock3;
			self->pain_debounce_time = level.time + 6;
		}
		else if (self->health <= health_2_4) {
			self->monsterinfo.currentmove = &chthon_frames_move_shock2;
			self->pain_debounce_time = level.time + 6;
		}
		else if (self->health <= health_3_4) {
			self->monsterinfo.currentmove = &chthon_frames_move_shock1;
			self->pain_debounce_time = level.time + 6;
		}
	}
}


void chthon_dead (edict_t *self)
{
	// Fire Chthon's targets after he sinks back in
	monster_death_use (self);
	VectorSet (self->mins, -64, -64, -64);
	VectorSet (self->maxs, 64, 64, 64);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
}

mframe_t chthon_frames_death [] =
{
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	chthon_sink	// was chthon_rise_sound
};
mmove_t chthon_move_death = {FRAME_death1, FRAME_death9, chthon_frames_death, chthon_dead};


void chthon_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("q1player/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 8; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", 0, 0, damage, GIB_ORGANIC);
		for (n= 0; n < 8; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib3/tris.md2", 0, 0, damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
		ThrowHead (self, "models/objects/q1gibs/q1gib2/tris.md2", 0, 0, damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NONE, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;

	self->monsterinfo.currentmove = &chthon_move_death;
}

void chthon_set_dead (edict_t *self)
{
	if ( !(self->monsterinfo.aiflags & AI_GOOD_GUY) && !(self->monsterinfo.monsterflags & MFL_DO_NOT_COUNT) )
		level.killed_monsters++;

	self->health = -50;
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NONE, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->monsterinfo.currentmove = &chthon_move_death;
}

// Returns true if Chthon is killed
qboolean chthon_bolt (edict_t *self, int num)
{
	int		 health_1_3, health_2_3;
	
	// This can't be called multiple times in one frame
	// (in the case of multiple target_q1_bolt emitters)
	if (level.time < self->pain_debounce_time)
		return false;

	health_1_3 = self->max_health * (1.0f/3.0f);
	health_2_3 = self->max_health * (2.0f/3.0f);

	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);	
	self->health -= (self->max_health / self->bossFireCount);
	self->pain_debounce_time = level.time + 6;

	if (self->health >= health_2_3)
		self->monsterinfo.currentmove = &chthon_frames_move_shock1;
	else if (self->health >= health_1_3)
		self->monsterinfo.currentmove = &chthon_frames_move_shock2;
	else if (self->health > 0)
		self->monsterinfo.currentmove = &chthon_frames_move_shock3;
	else
		self->monsterinfo.currentmove = &chthon_frames_move_shock3_die;

//	gi.dprintf ("chthon_bolt: hit #%i, health = %i\n", num, self->health);

	return (self->health <= 0);
}

void chthon_attack_left (edict_t *self)
{
	vec3_t	forward, right, start;
	vec3_t	dir;
	vec3_t	vec;
	vec3_t  offset;
	int		dist;
	qboolean high=false;

	if (!self->enemy)
		return;

	dist = range (self, self->enemy);

	AngleVectors (self->s.angles, forward, right, NULL); //was NULL
//	gi.dprintf ("ENEMY:%f\nSELF:%f\nDIFF:%f\n",self->enemy->s.origin[2],self->s.origin[2],
//	self->enemy->s.origin[2] - (self->s.origin[2] + 50.00));	
	
	if ((self->enemy->s.origin[2] - (self->s.origin[2] + 50.00)) > 100)
		high=true;

	VectorSet (offset, 36, 160, 200);
	G_ProjectSource (self->s.origin, offset, forward, right, start); // monster_flash_offset[MZ2_BOSS2_ROCKET_1]
	VectorCopy (self->enemy->s.origin, vec);

	switch (dist)
	{
		case RANGE_MELEE:
		{
		//	vec[0] += self->enemy->velocity[0] * 0.12;  
		//	vec[1] += self->enemy->velocity[1] * 0.12;
			if (!high)
				vec[2] -= self->enemy->viewheight;
			break;
		}
		case RANGE_NEAR:
		{
		//	vec[0] += self->enemy->velocity[0] * 0.25;  
		//	vec[1] += self->enemy->velocity[1] * 0.25; 
			if (!high)
				vec[2] -= self->enemy->viewheight;
			break;
		}
		case RANGE_MID:
		{
		//	vec[0] += self->enemy->velocity[0] * 0.4;  
		//	vec[1] += self->enemy->velocity[1] * 0.4; 
			if (!high)
				vec[2] -= (self->enemy->viewheight*(4 * random()));
			else
				vec[2] += self->enemy->viewheight;

			break;
		}
		case RANGE_FAR:
		default:
		{
		//	vec[0] += self->enemy->velocity[0];  
		//	vec[1] += self->enemy->velocity[1];
			if (!high)
				vec[2] -= (self->enemy->viewheight*(2 * random()));
			else
				vec[2] += self->enemy->viewheight;
			break;
		}
	}

	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_BOSS2_ROCKET_1);
	gi.multicast (start, MULTICAST_PVS);

	q1_fire_lavaball (self, start, dir, 100, 750, 120, 100);
}	

void chthon_attack_right (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	vec3_t  offset;
	int		dist;
	qboolean high = false;
	
	if (!self->enemy)
		return;

	dist = range (self, self->enemy);
	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet (offset, 36, -160, 200);
	G_ProjectSource (self->s.origin,offset, forward, right, start);//monster_flash_offset[MZ2_BOSS2_ROCKET_4]

	if ((self->enemy->s.origin[2] - (self->s.origin[2] + 50.00)) > 100)
		high=true;

	VectorCopy (self->enemy->s.origin, vec);
	
	switch (dist)
	{
		case RANGE_MELEE:
		{
		//	vec[0] += self->enemy->velocity[0] * 0.12;  
		//	vec[1] += self->enemy->velocity[1] * 0.12;
			if (!high)
				vec[2] -= self->enemy->viewheight;
			break;
		}
		case RANGE_NEAR:
		{
		//	vec[0] += self->enemy->velocity[0] * 0.25;  
		//	vec[1] += self->enemy->velocity[1] * 0.25; 
			if (!high)
				vec[2] -= self->enemy->viewheight;
			break;
		}
		case RANGE_MID:
		{
		//	vec[0] += self->enemy->velocity[0] * 0.4;  
		//	vec[1] += self->enemy->velocity[1] * 0.4; 
			if (!high)
				vec[2] -= (self->enemy->viewheight*(4 * random()));
			else
				vec[2] += self->enemy->viewheight;
			break;
		}
		case RANGE_FAR:
		default:
		{
		//	vec[0] += self->enemy->velocity[0];  
		//	vec[1] += self->enemy->velocity[1];
			if (!high)
				vec[2] -= (self->enemy->viewheight*(2 * random()));
			else
				vec[2] += self->enemy->viewheight;
			break;
		}
	}

	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	
	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_BOSS2_ROCKET_4);
	gi.multicast (start, MULTICAST_PVS);

	q1_fire_lavaball (self,start, dir, 100, 750, 120, 100);
}	


mframe_t chthon_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, chthon_sight_sound2,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, chthon_attack_left,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, chthon_attack_right,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, chthon_check_attack
};
mmove_t chthon_move_attack = {FRAME_attack1, FRAME_attack23, chthon_frames_attack, chthon_walk};

void chthon_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &chthon_move_attack;
}

void chthon_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_IDLE, 0);

/*	if (!self->enemy)
		gi.dprintf("chthon SEES FIRST ENEMY\n");
	else
		gi.dprintf("chthon SEES OLD ENEMY\n");
*/
//	self->monsterinfo.currentmove = &chthon_move_rise;
//	chthon_attack (self);
}

/*static*/ void chthon_check_attack (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		return;

	self->monsterinfo.currentmove = &chthon_move_attack;
}

qboolean chthon_checkattack (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		return false;

	else if (visible(self,self->enemy))
		return true;
	else
		return false;
}

// Chthon is really set up here
void chthon_awake (edict_t *self, edict_t *other, edict_t *activator)
{
//	if ( !Q_stricmp(level.mapname, "qe1m7") )
//		self->movetype = MOVETYPE_NONE;
//	else
		self->movetype = MOVETYPE_STEP;

	self->svflags &= ~SVF_NOCLIENT;

	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/q1chthon/tris.md2");
	self->use = monster_use;
/*	if ( !Q_stricmp(level.mapname, "qe1m7") ) {
		self->s.origin[2] -= 24;
	} */

	VectorSet (self->mins, -128, -128, -24);
	VectorSet (self->maxs, 128, 128, 226);

	// get number of times to be shocked
	if ( !self->count )
		self->bossFireCount = 3;
	else
		self->bossFireCount = max(self->count, 1);

	if (self->spawnflags & SF_CHTHON_LIGHTNING_ONLY) {
		self->health = self->bossFireCount * 2000;
		self->takedamage = DAMAGE_LIGHTING_ONLY;
	}
	else if (!self->health)
	{
		if (skill->value == 0)
			self->health = 3000;
		else if (skill->value == 1)
			self->health = 4000;
		else if (skill->value == 2)
			self->health = 5000;
		else
			self->health = 6000;
	}

	if (!self->gib_health)
		self->gib_health = -500;
	if (!self->mass)
		self->mass = 1500;
//	self->style = 0;

	self->flags |= (FL_Q1_MONSTER|FL_IMMUNE_LAVA);

	self->pain = chthon_pain;
	self->die = chthon_die;

	self->monsterinfo.stand = chthon_stand; 
	self->monsterinfo.walk = chthon_walk;
	self->monsterinfo.run = chthon_run; 
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = chthon_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;	// was chthon_sight
	self->monsterinfo.search = NULL;
	self->monsterinfo.checkattack = chthon_checkattack;

	// Lazarus
	if (self->powerarmor)
	{
		if (self->powerarmortype == 1)
			self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
		else
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		self->monsterinfo.power_armor_power = self->powerarmor;
	}

	self->s.renderfx |= RF_FULLBRIGHT;

	self->monsterinfo.scale = MODEL_SCALE;
	chthon_lava_splash (self); // added lava splash
	self->monsterinfo.currentmove = &chthon_move_rise;

	walkmonster_start (self);
}

#define CHTHON_TRIGGER_SPAWN 2


// Knightmare- added soundcache function
void monster_q1_chthon_soundcache (edict_t *self)
{
	sound_pain = gi.soundindex ("q1chthon/pain.wav");	
	sound_death = gi.soundindex ("q1chthon/death.wav");
	sound_sight = gi.soundindex ("q1chthon/sight1.wav");	
	sound_throw = gi.soundindex ("q1chthon/throw.wav");	
	sound_rise = gi.soundindex ("q1chthon/out1.wav");	
}


//
// SPAWN
//

/*QUAKED monster_q1_chthon (1 .5 0) (-128 -128 -24) (128 128 226) Ambush Trigger_Spawn Sight GoodGuy x LightningOnly Mobile
LightningOnly: specifies orignal lightning-kill only mode
Mobile: Chthon moves around
"count"	Number of times needed to shock Chthon
model="models/monsters/q1chthon/tris.md2"
frame="17"
*/
void SP_monster_q1_chthon (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	// Knightmare- use soundcache function
	monster_q1_chthon_soundcache (self);

	if (self->spawnflags & CHTHON_TRIGGER_SPAWN)
		self->spawnflags &= ~CHTHON_TRIGGER_SPAWN;

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;

	// Lazarus: special purpose skins
	if ( self->style )
	{
		PatchMonsterModel("models/monsters/q1chthon/tris.md2");
		self->s.skinnum = self->style * 2;
	}

	self->use = chthon_awake;

	self->common_name = "Chthon";
	self->class_id = ENTITY_MONSTER_Q1_CHTHON;

	gi.linkentity (self);
}
