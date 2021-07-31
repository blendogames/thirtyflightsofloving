/*==============================================================================

Quake AD FREDDIE (Edie from Quoth - Kell/Necros/Preach)
Reimplemented by Knightmare and MikeM

==============================================================================
*/

#include "g_local.h"
#include "m_q1freddie.h"

static int	sound_pain1;
static int	sound_pain2;
static int	sound_death;
static int	sound_death2;
static int	sound_idle;
static int	sound_sight;
static int	sound_saw;
static int	sound_melee;
static int	sound_atk_spinup;
static int	sound_atk_nail;
static int	sound_atk_laser;
static int	sound_step1;
static int	sound_step2;
static int	sound_step3;

#define FREDDIE_MELEE_DISTANCE 64

void freddie_fireweapon (edict_t *self);
void freddie_attack(edict_t *self);
void freddie_check_refire (edict_t *self);

void freddie_idle_sound (edict_t *self)
{
	if (random() < 0.2)
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void freddie_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	freddie_attack (self);
}

void freddie_sound_footstep (edict_t *self)
{
	static int	stepNum = 0;

	switch (stepNum)
	{
	case 0:
	default:
		gi.sound (self, CHAN_BODY, sound_step1, 1, ATTN_NORM, 0);
		break;
	case 1:
		gi.sound (self, CHAN_BODY, sound_step2, 1, ATTN_NORM, 0);
		break;
	case 2:
		gi.sound (self, CHAN_BODY, sound_step1, 1, ATTN_NORM, 0);
		break;
	case 3:
		gi.sound (self, CHAN_BODY, sound_step2, 1, ATTN_NORM, 0);
		break;
	case 4:
		gi.sound (self, CHAN_BODY, sound_step3, 1, ATTN_NORM, 0);
		break;
	}
	stepNum++;
	if (stepNum > 4)
		stepNum = 0;
}

//
// stand
//

void freddie_stand (edict_t *self);

mframe_t freddie_frames_stand [] =
{
	ai_stand, 0, freddie_idle_sound,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
};
mmove_t	freddie_move_stand = {FRAME_stand1, FRAME_stand8, freddie_frames_stand, freddie_stand};

void freddie_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &freddie_move_stand;
}

//
// walk
//

void freddie_walk (edict_t *self);

mframe_t freddie_frames_walk [] =
{
	ai_walk, 20, freddie_idle_sound,
	ai_walk, 16, freddie_sound_footstep,
	ai_walk, 1, NULL,
	ai_walk, 2, NULL,
	ai_walk, 4, NULL,
	ai_walk, 8, NULL,
	ai_walk, 4, NULL,
	ai_walk, 16, NULL,
	ai_walk, 20, NULL,
	ai_walk, 16, freddie_sound_footstep,
	ai_walk, 1, NULL,
	ai_walk, 2, NULL,
	ai_walk, 4, NULL,
	ai_walk, 8, NULL,
	ai_walk, 4, NULL,
	ai_walk, 16, NULL,
};
mmove_t freddie_move_walk = {FRAME_run1, FRAME_run6, freddie_frames_walk, NULL};

void freddie_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &freddie_move_walk;
}

//
// run
//

void freddie_run (edict_t *self);
void freddie_run_frame (edict_t *self);

mframe_t freddie_frames_run [] =
{
	ai_run, 24,	freddie_run_frame,	// freddie_idle_sound,
	ai_run, 19,	freddie_run_frame,	// freddie_sound_footstep,
	ai_run, 1,	freddie_run_frame,
	ai_run, 2,	freddie_run_frame,
	ai_run, 5,	freddie_run_frame,
	ai_run, 10,	freddie_run_frame,
	ai_run, 5,	freddie_run_frame,
	ai_run, 5,	freddie_run_frame,
	ai_run, 24,	freddie_run_frame,
	ai_run, 19,	freddie_run_frame,
	ai_run, 1,	freddie_run_frame,	// freddie_sound_footstep,
	ai_run, 2,	freddie_run_frame,
	ai_run, 5,	freddie_run_frame,
	ai_run, 10,	freddie_run_frame,
	ai_run, 5,	freddie_run_frame,
	ai_run, 5,	freddie_run_frame,
};
mmove_t freddie_move_run = {FRAME_run1, FRAME_run16, freddie_frames_run, NULL};

void freddie_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &freddie_move_stand;
	else
		self->monsterinfo.currentmove = &freddie_move_run;
}

//
// run + firing
//
#if 0
mframe_t freddie_frames_rfire [] =
{
	ai_run, 24,	freddie_run_frame,	// freddie_idle_sound,
	ai_run, 19,	freddie_run_frame,	// freddie_sound_footstep,
	ai_run, 1,	freddie_run_frame,
	ai_run, 2,	freddie_run_frame,
	ai_run, 5,	freddie_run_frame,
	ai_run, 10,	freddie_run_frame,
	ai_run, 5,	freddie_run_frame,
	ai_run, 5,	freddie_run_frame,
	ai_run, 24,	freddie_run_frame,
	ai_run, 19,	freddie_run_frame,
	ai_run, 1,	freddie_run_frame,	// freddie_sound_footstep,
	ai_run, 2,	freddie_run_frame,
	ai_run, 5,	freddie_run_frame,
	ai_run, 10,	freddie_run_frame,
	ai_run, 5,	freddie_run_frame,
	ai_run, 5,	freddie_run_frame,
};
//mmove_t freddie_move_rfire = {FRAME_rfire1, FRAME_rfire16, freddie_frames_rfire, NULL};

void freddie_rfire (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &freddie_move_stand;
	else
		self->monsterinfo.currentmove = &freddie_move_rfire;
}
#endif

void freddie_run_frame (edict_t *self)
{
	int		startFrame = 0;

	if ( !self || (self->health <= 0) )
		return;

	// If enemy is dead or missing no firing
//	if ( !self->enemy || (self->enemy->health <= 0) )
//		self->fog_model = 0;

//	if (self->monsterinfo.currentmove == &freddie_move_run)
		startFrame = FRAME_run1;
//	else if (self->monsterinfo.currentmove == &freddie_move_rfire)
//		startFrame = FRAME_rfire1;

	switch (self->s.frame - startFrame)
	{
	case 0:
		VectorSet (self->muzzle, 50, -20, 32);	freddie_idle_sound (self);
		break;
	case 1:
		VectorSet (self->muzzle, 43, -18, 33);	freddie_sound_footstep (self);
		break;
	case 2:
		VectorSet (self->muzzle, 53, -20, 27);
		break;
	case 3:
		VectorSet (self->muzzle, 47, -19, 31);
		break;
	case 4:
		VectorSet (self->muzzle, 54, -17, 29);
		break;
	case 5:
		VectorSet (self->muzzle, 47, -17, 35);
		break;
	case 6:
		VectorSet (self->muzzle, 53, -15, 32);
		break;
	case 7:
		VectorSet (self->muzzle, 47, -15, 36);
		break;
	case 8:
		VectorSet (self->muzzle, 50, -13, 32);
		break;
	case 9:
		VectorSet (self->muzzle, 46, -15, 33);
		break;
	case 10:
		VectorSet (self->muzzle, 51, -14, 33);	freddie_sound_footstep (self);
		break;
	case 11:
		VectorSet (self->muzzle, 47, -14, 32);
		break;
	case 12:
		VectorSet (self->muzzle, 52, -14, 33);
		break;
	case 13:
		VectorSet (self->muzzle, 47, -14, 36);
		break;
	case 14:
		VectorSet (self->muzzle, 49, -14, 35);
		break;
	case 15:
		VectorSet (self->muzzle, 45, -15, 34);
		break;
	default:
		VectorSet (self->muzzle, 0, 0, 0);
		break;
	}

	// Run + Fire time?
/*	if (self->fog_model > 0)
	{
		self->fog_index++;
		if (self->fog_index > 2)
			self->fog_model = 0;
		else
			freddie_fireweapon (self);
	}*/
}


//
// pain
//

void freddie_pain (edict_t *self, edict_t *other, float kick, int damage);

mframe_t freddie_frames_pain1 [] =
{
	ai_move, 0, NULL,
	ai_move, 10, NULL,
	ai_move, 9, NULL,
	ai_move, 4, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t freddie_move_pain1 = {FRAME_pain1, FRAME_pain12, freddie_frames_pain1, freddie_run};

mframe_t freddie_frames_pain2 [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t freddie_move_pain2 = {FRAME_painb1, FRAME_painb5, freddie_frames_pain2, freddie_run};


void freddie_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	float r;

	if (level.time < self->pain_debounce_time)
		return;

	r = random();

	if (r < 0.5f)
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);

	self->pain_debounce_time = level.time + 3 + random() * 2;

	if (skill->value == 3) {
		self->pain_debounce_time = level.time + 3 + random() * 2;
		return;		// no pain anims in nightmare
	}

	if (damage <= 50) {
		self->monsterinfo.currentmove = &freddie_move_pain2;
		self->pain_debounce_time = level.time + 2 + random() * 2;
	}
	else {
		self->monsterinfo.currentmove = &freddie_move_pain1;
		self->pain_debounce_time = level.time + 3 + random() * 2;
	}
}

//
// fire (standing)
//

void freddie_attack (edict_t *self);
void freddie_do_sfire (edict_t *self);
void freddie_attack_spindown (edict_t *self);

void freddie_attack_spinupsound (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_atk_spinup, 1.0, ATTN_NORM, 0);
}

void freddie_attack_prefire (edict_t *self)
{
	self->fog_index = 0;
	self->fogclip = 4 + (int)skill->value * 2;
}

void freddie_fireweapon (edict_t *self)
{
	vec3_t	start, forward, right, dir, vec;	// attack_track
	float	speed, scale;

	if ( !self || (self->health <= 0) )
		return;
	if ( !self->enemy || (self->enemy->health < 1) )	// If no enemy or enemy dead then stop attack
		return;

	// Nail and laser use the same firing speed
	speed = 500 + (int)skill->value * 150;
	// Rotate offset vector
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, self->muzzle, forward, right, start);

/*	if ( (self->fog_model != 0) && ((int)skill->value > 1) )	// The mini burst mode (run+fire) can do tracking
	{
		if (self->fog_index == 1)
			Freddie_Tracking (self->enemy->s.origin, self->muzzle, self->enemy, speed, attack_track);
		else
			Freddie_Tracking (attack_track, self->muzzle, self->enemy, speed, attack_track);
		VectorSubtract (attack_track, start, dir);
		VectorNormalize (dir);
	}
	else*/	// Standard mode: spray bullets at player
	{
		scale = crandom() * (20 + (random() * 50));
		VectorScale (right, scale, vec);
		VectorAdd (self->enemy->s.origin, vec, dir);
		VectorSubtract (dir, start, dir);
		VectorNormalize (dir);
	}

	if (self->sounds == 1) {
		gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, sound_atk_laser, 1.0, ATTN_NORM, 0);
		q1_fire_laser (self, start, dir, 15, speed);
	}
	else {
		gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, sound_atk_nail, 1.0, ATTN_NORM, 0);
		q1_fire_nail (self, start, dir, 9, speed, true);
	}
}

void freddie_stand_fire (edict_t *self)
{
	qboolean	quitNext = false;

	// Easy = 7 shots, Normal = 11 shots, Hard/NM = 15 shots
	switch (self->s.frame)
	{
	case FRAME_sfire9:
		VectorSet (self->muzzle, 45, -10, 20);
		break;
	case FRAME_sfire10:
		VectorSet (self->muzzle, 45, -10, 20);
		break;
	case FRAME_sfire11:
		VectorSet (self->muzzle, 45, -10, 22);
		break;
	case FRAME_sfire12:
		VectorSet (self->muzzle, 45, -10, 22);
		break;
	case FRAME_sfire13:
		VectorSet (self->muzzle, 45, -12, 23);
		break;
	case FRAME_sfire14:
		VectorSet (self->muzzle, 42, -12, 25);
		break;
	case FRAME_sfire15:
		VectorSet (self->muzzle, 42, -14, 23);
		if ((int)skill->value == 0)
			quitNext = true;
		break;
	case FRAME_sfire16:
		VectorSet (self->muzzle, 38, -14, 27);
		break;
	case FRAME_sfire17:
		VectorSet (self->muzzle, 38, -17, 26);
		break;
	case FRAME_sfire18:
		VectorSet (self->muzzle, 36, -17, 28);
		break;
	case FRAME_sfire19:
		VectorSet (self->muzzle, 36, -17, 26);
		if ((int)skill->value == 1)
			quitNext = true;
		break;
	case FRAME_sfire20:
		VectorSet (self->muzzle, 30, -17, 28);
		break;
	case FRAME_sfire21:
		VectorSet (self->muzzle, 30, -18, 25);
		break;
	case FRAME_sfire22:
		VectorSet (self->muzzle, 25, -18, 27);
		break;
	case FRAME_sfire23:
		VectorSet (self->muzzle, 25, -18, 27);
		break;
	default:
		VectorSet (self->muzzle, 0, 0, 0);
		break;
	}

	if ( !self->enemy || (self->enemy->health < 1) )	// If no enemy or enemy dead then stop attack
		self->fog_index = self->fogclip + 1;
	else if ( visible(self, self->enemy) )				// Check if enemy is visible to weapon
		self->fog_index++;
	else												// Reset counter and keep firing
		self->fog_index = 0;	

	if (self->fog_index > self->fogclip)
		freddie_attack_spindown (self);
	else
		freddie_fireweapon (self);

	if (quitNext)	// this is our last firing frame based on skill level
		self->fog_index = self->fogclip + 1;
}

mframe_t freddie_frames_sfire_start [] =
{
	ai_charge, 0, freddie_attack_spinupsound,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, freddie_sound_footstep,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, freddie_attack_prefire
};
mmove_t freddie_move_sfire_start = {FRAME_sfire1, FRAME_sfire8, freddie_frames_sfire_start, freddie_do_sfire};

mframe_t freddie_frames_sfire [] =
{
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire,
	ai_charge, 0, freddie_stand_fire
};
mmove_t freddie_move_sfire = {FRAME_sfire9, FRAME_sfire23, freddie_frames_sfire, freddie_attack_spindown};

mframe_t freddie_frames_sfire_stop [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t freddie_move_sfire_stop = {FRAME_sfire24, FRAME_sfire27, freddie_frames_sfire_stop, freddie_run};

void freddie_do_sfire (edict_t *self)
{
	self->monsterinfo.currentmove = &freddie_move_sfire;
}

void freddie_attack_spindown (edict_t *self)
{
	self->monsterinfo.currentmove = &freddie_move_sfire_stop;
}

void freddie_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &freddie_move_sfire_start;
}

//
// melee
//

void freddie_melee (edict_t *self);
void freddie_melee_repeat (edict_t *self);

void freddie_saw_sound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_saw, 1, ATTN_NORM, 0);
}

void freddie_blade (edict_t *self, float side)
{
	float	damage;
	vec3_t	aim;

	if (!self->enemy)
		return;
	if (self->enemy->health <= 0)
		return;

	// 1-45 (3x15) damage
	damage = (random() + random() + random()) * 15;
	damage = max (1, damage);

	VectorSet (aim, FREDDIE_MELEE_DISTANCE, self->mins[0], 8);
	if ( fire_hit (self, aim, damage, 100) )
		gi.sound (self, CHAN_VOICE, sound_melee, 1, ATTN_NORM, 0);
}

void freddie_blade_right (edict_t *self)
{
	freddie_blade (self, -200);
}

void freddie_blade_left (edict_t *self)
{
	freddie_blade (self, 200);
}

mframe_t freddie_frames_fswing [] =
{
	ai_charge, 8, freddie_saw_sound,
	ai_charge, 16, freddie_sound_footstep,
	ai_charge, 10, NULL,
	ai_charge, 4, freddie_sound_footstep,
	ai_charge, 2, NULL,
	ai_charge, 1, NULL,
	ai_charge, 1, NULL,
	ai_charge, 2, freddie_blade_right,
	ai_charge, 2, NULL,
	ai_charge, 2, NULL,
	ai_charge, 0, NULL,
};
mmove_t freddie_move_fswing = {FRAME_fswing1, FRAME_fswing11, freddie_frames_fswing, freddie_melee_repeat};

mframe_t freddie_frames_bswing [] =
{
	ai_charge, 20, freddie_saw_sound,
	ai_charge, 12, NULL,
	ai_charge, 4, NULL,
	ai_charge, 2, freddie_sound_footstep,
	ai_charge, 6, freddie_blade_left,
	ai_charge, 4, NULL,
	ai_charge, 4, NULL,
	ai_charge, 8, NULL,
	ai_charge, 8, freddie_sound_footstep,
	ai_charge, 8, NULL,
	ai_charge, 8, NULL,
	ai_charge, 12, NULL,
	ai_charge, 10, NULL,
	ai_charge, 10, NULL,
};
mmove_t freddie_move_bswing = {FRAME_bswing1, FRAME_bswing14, freddie_frames_bswing, freddie_run};

mframe_t freddie_frames_fswinge [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, freddie_sound_footstep,
};
mmove_t freddie_move_fswinge = {FRAME_fswinge1, FRAME_fswinge10, freddie_frames_fswinge, freddie_run};

void freddie_melee_repeat  (edict_t *self)
{
	int r;

	if (!self->enemy)
		return;
	if (self->enemy->health <= 0)
		return;

	// Check if within range to do a second (backward) swing attack
	r = range(self, self->enemy);
	if (r == RANGE_MELEE)
	{
		self->monsterinfo.currentmove = &freddie_move_bswing;
	}
	else
		self->monsterinfo.currentmove = &freddie_move_fswinge;
}

void freddie_melee (edict_t *self)
{
	int r;
	
	if (!self->enemy)
		return;
	if (self->enemy->health <= 0)
		return;

	r = range(self, self->enemy);
	if (r == RANGE_MELEE)
	{
		self->monsterinfo.currentmove = &freddie_move_fswing;
	}
	else if (visible(self, self->enemy) && infront(self,self->enemy)
		&& (r < RANGE_FAR) && !(self->monsterinfo.aiflags & AI_SOUND_TARGET))
	{
		self->monsterinfo.currentmove = &freddie_move_sfire_start;
	}
	else
		self->monsterinfo.currentmove = &freddie_move_run;
}

//
// death
//

void freddie_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

void freddie_nogib (edict_t *self)
{
	self->gib_health = -10000;
}

mframe_t freddie_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 freddie_nogib,
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
	ai_move, 0,	 NULL
};
mmove_t freddie_move_death = {FRAME_death1, FRAME_death35, freddie_frames_death, freddie_dead};

void freddie_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n, type, bloodTypeBak;

	// check for gib
	if ( (self->health <= self->gib_health) && !(self->spawnflags & SF_MONSTER_NOGIB) )
	{
		gi.sound (self, CHAN_VOICE|CHAN_RELIABLE, sound_death2, 1, ATTN_NORM, 0);

	//	T_RadiusDamage (self, self, 40, NULL, 40, MOD_EXPLOSIVE);	// this would cause a crash because inflictor entity is freed

		gi.WriteByte (svc_temp_entity);
		if (self->waterlevel)
			type = TE_ROCKET_EXPLOSION_WATER;
		else
			type = TE_EXPLOSION1_BIG;
		gi.WriteByte (type);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);
		
		for (n = 0; n < 6; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", 0, 0, damage, GIB_ORGANIC);
		for (n = 0; n < 6; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib3/tris.md2", 0, 0, damage, GIB_ORGANIC);

		// change blood type to mechanical to throw mech gibs
		bloodTypeBak = self->blood_type;
		self->blood_type = 2;
		for (n = 0; n < 3; n++)
			ThrowGib (self, "models/monsters/q1freddie/gib_junk/tris.md2", (int)(random()*3), 0, damage, GIB_METALLIC);
		for (n = 0; n < 4; n++)
			ThrowGib (self, "models/monsters/q1freddie/gib_metal1/tris.md2", 0, 0, damage, GIB_METALLIC);
		for (n = 0; n < 4; n++)
			ThrowGib (self, "models/monsters/q1freddie/gib_metal3/tris.md2", 0, 0, damage, GIB_METALLIC);
		self->blood_type = bloodTypeBak;

		ThrowHead (self, "models/monsters/q1ogre/head/tris.md2", 0, 0, damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &freddie_move_death;
}


/*QUAKED monster_q1_freddie (1 0 0) (-32 -32 -24) (32 32 64) Ambush Trigger_Spawn Sight
model="models/monsters/q1freddie/tris.md2"
*/
void SP_monster_q1_freddie (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_pain1 =		gi.soundindex ("q1freddie/pain.wav");
	sound_pain2 =		gi.soundindex ("q1freddie/painshrt.wav");
	sound_death =		gi.soundindex ("q1freddie/death.wav");
	sound_death2 =		gi.soundindex ("q1freddie/explode_major.wav");
	sound_idle =		gi.soundindex ("q1freddie/idle1.wav");
	sound_sight =		gi.soundindex ("q1freddie/sight.wav");
	sound_saw  =		gi.soundindex ("q1freddie/sawstart.wav");
	sound_melee  =		gi.soundindex ("q1freddie/mangle.wav");
	sound_atk_spinup =	gi.soundindex ("q1freddie/stfire.wav");
	sound_atk_nail =	gi.soundindex ("q1weapons/rocket1i.wav");
	sound_atk_laser =	gi.soundindex ("q1enforcer/enfire.wav");
	sound_step1  =		gi.soundindex ("q1freddie/step.wav");
	sound_step2  =		gi.soundindex ("q1freddie/step2.wav");
	sound_step3  =		gi.soundindex ("q1freddie/step3.wav");

	// precache gibs
	gi.modelindex ("models/monsters/q1ogre/head/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib1/tris.md2");
	gi.modelindex ("models/objects/q1gibs/q1gib3/tris.md2");
	gi.modelindex ("models/monsters/q1freddie/gib_junk/tris.md2");
	gi.modelindex ("models/monsters/q1freddie/gib_metal1/tris.md2");
	gi.modelindex ("models/monsters/q1freddie/gib_metal3/tris.md2");
	// precache nail/laser
	if (self->sounds == 1)
		q1_laser_precache ();
	else
		q1_nail_precache ();

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	// Lazarus: special purpose skins
	if ( self->style )
	{
		PatchMonsterModel("models/monsters/q1freddie/tris.md2");
		self->s.skinnum = self->style;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/q1freddie/tris.md2");
	VectorSet (self->mins, -24, -24, -24);
	VectorSet (self->maxs, 24, 24, 64);

	if (!self->health)
		self->health = 500;
	if (!self->gib_health)
		self->gib_health = -80;
	if (!self->mass)
		self->mass = 500;

	self->pain = freddie_pain;
	self->die = freddie_die;

	self->flags |= FL_Q1_MONSTER;

	self->monsterinfo.stand = freddie_stand;
	self->monsterinfo.walk = freddie_walk;
	self->monsterinfo.run = freddie_run;
	self->monsterinfo.walk = freddie_walk;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = freddie_attack;
	self->monsterinfo.melee = freddie_melee;
	self->monsterinfo.sight = freddie_sight;
	self->monsterinfo.search = freddie_stand;

	VectorSet (self->muzzle, 0.0f, 0.0f, 0.0f);	// attack_offset from original Quake C
	self->fog_model = 0;						// attack_timer  from original Quake C

	if (!self->monsterinfo.flies)
		self->monsterinfo.flies = 0.25;

	// Lazarus
	if (self->powerarmor)
	{
		if (self->powerarmortype == 1)
			self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
		else
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		self->monsterinfo.power_armor_power = self->powerarmor;
	}

	self->common_name = "Freddie";
	self->class_id = ENTITY_MONSTER_Q1_FREDDIE;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &freddie_move_stand;	
	if (self->health < 0)
	{
		mmove_t	*deathmoves[] = {&freddie_move_death,
								 NULL};
		M_SetDeath (self, (mmove_t **)&deathmoves);
	}
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
