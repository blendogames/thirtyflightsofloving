/*
==============================================================================

GUNNER

==============================================================================
*/

#include "g_local.h"
#include "m_gunner.h"

static int	sound_pain;
static int	sound_pain2;
static int	sound_death;
static int	sound_idle;
static int	sound_open;
static int	sound_search;
static int	sound_sight;

// Knightmare- Tactician Gunner sounds
#ifndef KMQUAKE2_ENGINE_MOD
static int	tactician_sound_fire_flechette;
#endif	// KMQUAKE2_ENGINE_MOD
/*
static int	tactician_sound_pain;
static int	tactician_sound_pain2;
static int	tactician_sound_death;
static int	tactician_sound_idle;
static int	tactician_sound_open;
static int	tactician_sound_search;
static int	tactician_sound_sight;
*/
// end Knightmare

// NOTE: Original gunner grenade velocity was 600 units/sec, but then 
//       fire_grenade added 200 units/sec in a direction perpendicular
//       to the aim direction. We've removed that from fire_grenade 
//       (for the gunner, not for players) since the gunner now shoots 
//       smarter, and adjusted things so that the initial velocity out 
//       of the barrel is the same.
#define GRENADE_VELOCITY			632.4555320337f
#define GRENADE_VELOCITY_SQUARED	400000.0f
// Knightmare- placement spread for Tactician Gunner prox mines
#define	GUNNER_PROX_SPREAD			48.0f
#define	HALF_GUNNER_PROX_SPREAD		(GUNNER_PROX_SPREAD * 0.5f)

void gunner_idlesound (edict_t *self)
{
	if ( !(self->spawnflags & SF_MONSTER_AMBUSH) ) {
	/*	if (self->moreflags & FL2_COMMANDER)
			gi.sound (self, CHAN_VOICE, tactician_sound_idle, 1, ATTN_IDLE, 0);
		else */
			gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
	}
}

void gunner_sight (edict_t *self, edict_t *other)
{
/*	if (self->moreflags & FL2_COMMANDER)
		gi.sound (self, CHAN_VOICE, tactician_sound_sight, 1, ATTN_NORM, 0);
	else */
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void gunner_search (edict_t *self)
{
/*	if (self->moreflags & FL2_COMMANDER)
		gi.sound (self, CHAN_VOICE, tactician_sound_search, 1, ATTN_NORM, 0);
	else */
		gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}


qboolean visible (edict_t *self, edict_t *other);
void GunnerGrenade (edict_t *self);
void GunnerFire (edict_t *self);
void gunner_fire_chain(edict_t *self);
void gunner_refire_chain(edict_t *self);


void gunner_stand (edict_t *self);

mframe_t gunner_frames_fidget [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, gunner_idlesound,
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
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	gunner_move_fidget = {FRAME_stand31, FRAME_stand70, gunner_frames_fidget, gunner_stand};

void gunner_fidget (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		return;
	if (random() <= 0.05)
		self->monsterinfo.currentmove = &gunner_move_fidget;
}

mframe_t gunner_frames_stand [] =
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
	ai_stand, 0, gunner_fidget,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, gunner_fidget,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, gunner_fidget
};
mmove_t	gunner_move_stand = {FRAME_stand01, FRAME_stand30, gunner_frames_stand, NULL};

void gunner_stand (edict_t *self)
{
		self->monsterinfo.currentmove = &gunner_move_stand;
}

mframe_t gunner_frames_walk [] =
{
	ai_walk, 0, NULL,
	ai_walk, 3, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 7, NULL,
	ai_walk, 2, NULL,
	ai_walk, 6, NULL,
	ai_walk, 4, NULL,
	ai_walk, 2, NULL,
	ai_walk, 7, NULL,
	ai_walk, 5, NULL,
	ai_walk, 7, NULL,
	ai_walk, 4, NULL
};
mmove_t gunner_move_walk = {FRAME_walk07, FRAME_walk19, gunner_frames_walk, NULL};

void gunner_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &gunner_move_walk;
}

mframe_t gunner_frames_run [] =
{
	ai_run, 26, NULL,
	ai_run, 9,  NULL,
	ai_run, 9,  NULL,
	ai_run, 9,  monster_done_dodge,
	ai_run, 15, NULL,
	ai_run, 10, NULL,
	ai_run, 13, NULL,
	ai_run, 6,  NULL
};

mmove_t gunner_move_run = {FRAME_run01, FRAME_run08, gunner_frames_run, NULL};

void gunner_run (edict_t *self)
{
	monster_done_dodge(self);
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &gunner_move_stand;
	else
		self->monsterinfo.currentmove = &gunner_move_run;
}

mframe_t gunner_frames_runandshoot [] =
{
	ai_run, 32, NULL,
	ai_run, 15, NULL,
	ai_run, 10, NULL,
	ai_run, 18, NULL,
	ai_run, 8,  NULL,
	ai_run, 20, NULL
};

mmove_t gunner_move_runandshoot = {FRAME_runs01, FRAME_runs06, gunner_frames_runandshoot, NULL};

void gunner_runandshoot (edict_t *self)
{
	self->monsterinfo.currentmove = &gunner_move_runandshoot;
}

mframe_t gunner_frames_pain3 [] =
{
	ai_move, -3, NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 1,	 NULL
};
mmove_t gunner_move_pain3 = {FRAME_pain301, FRAME_pain305, gunner_frames_pain3, gunner_run};

mframe_t gunner_frames_pain2 [] =
{
	ai_move, -2, NULL,
	ai_move, 11, NULL,
	ai_move, 6,	 NULL,
	ai_move, 2,	 NULL,
	ai_move, -1, NULL,
	ai_move, -7, NULL,
	ai_move, -2, NULL,
	ai_move, -7, NULL
};
mmove_t gunner_move_pain2 = {FRAME_pain201, FRAME_pain208, gunner_frames_pain2, gunner_run};

mframe_t gunner_frames_pain1 [] =
{
	ai_move, 2,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -5, NULL,
	ai_move, 3,	 NULL,
	ai_move, -1, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 2,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -2, NULL,
	ai_move, -2, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t gunner_move_pain1 = {FRAME_pain101, FRAME_pain118, gunner_frames_pain1, gunner_run};

void gunner_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		smallDamage, bigDamage;

	if (self->health < (self->max_health / 2))
		self->s.skinnum |= 1;

	monster_done_dodge (self);

	// Tactician Gunner shrugs off small damage
	if ( (self->moreflags & FL2_COMMANDER) && (damage < 10) )
		return;

	if (!self->groundentity)
	{
	//	if ((g_showlogic) && (g_showlogic->value))
	//		gi.dprintf ("gunner: pain avoided due to no ground\n");
		return;
	}

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if (rand() & 1) {
	/*	if (self->moreflags & FL2_COMMANDER)
			gi.sound (self, CHAN_VOICE, tactician_sound_pain, 1, ATTN_NORM, 0);
		else */
			gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	}
	else {
	/*	if (self->moreflags & FL2_COMMANDER)
			gi.sound (self, CHAN_VOICE, tactician_sound_pain2, 1, ATTN_NORM, 0);
		else */
			gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	}

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (self->moreflags & FL2_COMMANDER) {
		smallDamage = 20;
		bigDamage = 50;
	}
	else {
		smallDamage = 10;
		bigDamage = 25;
	}

//	if (damage <= 10)
	if (damage <= smallDamage)
		self->monsterinfo.currentmove = &gunner_move_pain3;
//	else if (damage <= 25)
	else if (damage <= bigDamage)
		self->monsterinfo.currentmove = &gunner_move_pain2;
	else
		self->monsterinfo.currentmove = &gunner_move_pain1;

	self->monsterinfo.aiflags &= ~AI_MANUAL_STEERING;

	// PMM - clear duck flag
	if (self->monsterinfo.aiflags & AI_DUCKED)
		monster_duck_up (self);
}

void gunner_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
	M_FlyCheck (self);

	// Lazarus monster fade
	if (world->effects & FX_WORLDSPAWN_CORPSEFADE)
	{
		self->think = FadeDieSink;
		self->nextthink = level.time+corpse_fadetime->value;
	}
}

mframe_t gunner_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -7, NULL,
	ai_move, -3, NULL,
	ai_move, -5, NULL,
	ai_move, 8,	 NULL,
	ai_move, 6,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t gunner_move_death = {FRAME_death01, FRAME_death11, gunner_frames_death, gunner_dead};

void gunner_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	self->monsterinfo.power_armor_type = POWER_ARMOR_NONE;

	// check for gib
	if (self->health <= self->gib_health && !(self->spawnflags & SF_MONSTER_NOGIB))
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n = 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", 0, 0, damage, GIB_ORGANIC);
		for (n = 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", 0, 0, damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
/*	if (self->moreflags & FL2_COMMANDER)
		gi.sound (self, CHAN_VOICE, tactician_sound_death, 1, ATTN_NORM, 0);
	else */
		gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);

	self->s.skinnum |= 1;
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &gunner_move_death;
}

// Knightmare added
// This calcs horizontal spread of prox mines based on distance.
// Then it does a short-range trace at blast radius, to ensure we won't clip a wall.
#define GUNNER_PROX_DANGER_RANGE	256.0f
qboolean gunner_prox_safety_check (edict_t *self, vec3_t start, vec3_t target)
{
	trace_t		tr;
	vec3_t		closeCheckMins, closeCheckMaxs, dir, dangerOffset, dangerTarget;
	float		dist, dangerSpread;

	// get dist to target
	VectorSubtract (target, start, dir);
	dist = VectorLength (dir);

	// get spread at damger range
	dangerSpread = (HALF_GUNNER_PROX_SPREAD / dist) * GUNNER_PROX_DANGER_RANGE;
	dangerSpread += PROX_TEST_SIZE;	// add bounds of prox mine + 1
	VectorSet (closeCheckMins, -dangerSpread, -dangerSpread, -12.0f);
	VectorSet (closeCheckMaxs, dangerSpread, dangerSpread, 12.0f);

//	if ((g_showlogic) && (g_showlogic->value))
//		gi.dprintf ("Tactician Gunner: perfoming close-range safety check with radius of %5.2f- ", dangerSpread);

	// extrapolate point on path to target at danger range
	VectorNormalize (dir);
	VectorScale (dir, GUNNER_PROX_DANGER_RANGE, dangerOffset);
	VectorAdd (start, dangerOffset, dangerTarget);

	tr = gi.trace(start, closeCheckMins, closeCheckMaxs, dangerTarget, self, MASK_SHOT);
	if (tr.fraction < 1.0) {
	//	if (g_showlogic && g_showlogic->value)
	//		gi.dprintf ("failed!\n");
		return false;
	}
//	if (g_showlogic && g_showlogic->value)
//		gi.dprintf ("succeeded!\n");
	return true;
}

// Knightmare added
// This does a short-range trace at blast radius, to ensure we won't clip a wall.
#define GUNNER_CTGRENADE_DANGER_RANGE	128.0f
qboolean gunner_ctgrenade_safety_check (edict_t *self, vec3_t start, vec3_t target)
{
	trace_t		tr;
	vec3_t		dir, dangerOffset, dangerTarget;
	float		dist;

	// get dist to target
	VectorSubtract (target, start, dir);
	dist = VectorLength (dir);

//	if ((g_showlogic) && (g_showlogic->value))
//		gi.dprintf ("Gunner: perfoming close-range contactgrenade safety check- ");

	// extrapolate point on path to target at danger range
	VectorNormalize (dir);
	VectorScale (dir, GUNNER_CTGRENADE_DANGER_RANGE, dangerOffset);
	VectorAdd (start, dangerOffset, dangerTarget);

	tr = gi.trace(start, vec3_origin, vec3_origin, dangerTarget, self, MASK_SHOT);
	if (tr.fraction < 1.0) {
	//	if (g_showlogic && g_showlogic->value)
	//		gi.dprintf ("failed!\n");
		return false;
	}
//	if (g_showlogic && g_showlogic->value)
//		gi.dprintf ("succeeded!\n");
	return true;
}


qboolean gunner_grenade_check (edict_t *self)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		target;
	trace_t		tr;
	vec3_t		dir;
	vec3_t		vhorz;
	float		horz, vertmax, dangerClose;
	qboolean	isProx = (self->moreflags & FL2_COMMANDER);
	qboolean	isContact = (self->spawnflags & SF_MONSTER_SPECIAL);
	// Knightmare- Tactician Gunner fires prox mines in a spread, 
	// so we need a wider safety bounds check
	vec3_t		checkMins, checkMaxs;
	vec3_t		proxMins = {-PROX_TEST_SIZE, -PROX_TEST_SIZE, -PROX_TEST_SIZE};
	vec3_t		proxMaxs = {PROX_TEST_SIZE, PROX_TEST_SIZE, PROX_TEST_SIZE};

	if (!self->enemy)
		return false;

	// if the player is above my head, use machinegun.

//	if (self->absmax[2] <= self->enemy->absmin[2])
//		return false;

	// Lazarus: We can do better than that... see below

	// Knightmare- use appropriate trace mins/maxs based on projectile
	if (isProx) {
		VectorCopy (proxMins, checkMins);
		VectorCopy (proxMaxs, checkMaxs);
	}
	else {
		VectorCopy (vec3_origin, checkMins);
		VectorCopy (vec3_origin, checkMaxs);
	}

	// check to see that we can trace to the player before we start
	// tossing grenades around.
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_GUNNER_GRENADE_1], forward, right, start);

	// see if we're too close
	// Knightmare- Tactician Gunner's prox mines stick around, so only use at longer range
	// Also use longer range for contact grenades
	if (isProx)
		dangerClose = 320.0f;
	else if (isContact)
		dangerClose = 128.0f;
	else
		dangerClose = 100.0f;
	VectorSubtract (self->enemy->s.origin, self->s.origin, dir);
	if (VectorLength(dir) < dangerClose)
		return false;

	// Lazarus: Max vertical distance - this is approximate and conservative
	VectorCopy (dir, vhorz);
	vhorz[2] = 0;
	horz = VectorLength (vhorz);
	vertmax = (GRENADE_VELOCITY_SQUARED) / (2 * sv_gravity->value) -
		0.5 * sv_gravity->value * horz * horz / GRENADE_VELOCITY_SQUARED;
	if (dir[2] > vertmax) 
		return false;

	// Lazarus: Make sure there's a more-or-less clear flight path to target
	// Rogue checked target origin, but if target is above gunner then the trace
	// would almost always hit the platform the target was standing on
	VectorCopy (self->enemy->s.origin, target);
	target[2] = self->enemy->absmax[2];
	tr = gi.trace(start, checkMins, checkMaxs, target, self, MASK_SHOT);
	if (tr.ent == self->enemy || tr.fraction == 1)
	{	
		VectorCopy (target, self->aim_point);	// save this aim location in case later safety check fails
		// Knightmare- added close-range prox and contact grenade safety checks
		if (isProx) {
			if ( gunner_prox_safety_check(self, start, target) )
				return true;
		}
		else if (isContact) {
			if ( gunner_ctgrenade_safety_check(self, start, target) )
				return true;
		}
		else
			return true;
	}
	// Repeat for feet... in case we're looking down at a target standing under,
	// for example, a short doorway
	target[2] = self->enemy->absmin[2];
	tr = gi.trace(start, checkMins, checkMaxs, target, self, MASK_SHOT);
	if (tr.ent == self->enemy || tr.fraction == 1)
	{
		VectorCopy (target, self->aim_point);	// save this aim location in case later safety check fails
		// Knightmare- added close-range prox and contact grenade safety checks
		if (isProx) {
			if ( gunner_prox_safety_check(self, start, target) )
				return true;
		}
		else if (isContact) {
			if ( gunner_ctgrenade_safety_check(self, start, target) )
				return true;
		}
		else
			return true;
	}

	return false;
}

// PMM - changed to duck code for new dodge

//
// this is specific to the gunner, leave it be
//
void gunner_duck_down (edict_t *self)
{
//	if (self->monsterinfo.aiflags & AI_DUCKED)
//		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	if (skill->value >= 2)
	{
		// Lazarus: Added check for goodness of grenade firing
		if (random() > 0.5 && gunner_grenade_check(self))
			GunnerGrenade (self);
	}

//	self->maxs[2] -= 32;
	self->maxs[2] = self->monsterinfo.base_height - 32;
	self->takedamage = DAMAGE_YES;
	if (self->monsterinfo.duck_wait_time < level.time)
		self->monsterinfo.duck_wait_time = level.time + 1;
	gi.linkentity (self);
}

mframe_t gunner_frames_duck [] =
{
	ai_move, 1,  gunner_duck_down,
	ai_move, 1,  NULL,
	ai_move, 1,  monster_duck_hold,
	ai_move, 0,  NULL,
	ai_move, -1, NULL,
	ai_move, -1, NULL,
	ai_move, 0,  monster_duck_up,
	ai_move, -1, NULL
};
mmove_t	gunner_move_duck = {FRAME_duck01, FRAME_duck08, gunner_frames_duck, gunner_run};

// PMM - gunner dodge moved below so I know about attack sequences

void gunner_opengun (edict_t *self)
{
/*	if (self->moreflags & FL2_COMMANDER)
		gi.sound (self, CHAN_VOICE, tactician_sound_open, 1, ATTN_IDLE, 0);
	else */
		gi.sound (self, CHAN_VOICE, sound_open, 1, ATTN_IDLE, 0);
}

void GunnerFire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	target;
	vec3_t	aim;
	vec3_t	targ_vel;
	int		flash_number;
	float	dist, time, flechetteSpeed = 850.0f;

	if (!self->enemy || !self->enemy->inuse)		//PGM
		return;									//PGM

#ifdef KMQUAKE2_ENGINE_MOD	// Knightmare- unique muzzle flash for Tactician Gunner's flechettes
	if (self->moreflags & FL2_COMMANDER)
		flash_number = MZ2_GUNNER_ETF_RIFLE_1 + (self->s.frame - FRAME_attak216);
	else
#endif	// KMQUAKE2_ENGINE_MOD
		flash_number = MZ2_GUNNER_MACHINEGUN_1 + (self->s.frame - FRAME_attak216);

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	// project enemy back a bit and target there
	VectorCopy (self->enemy->s.origin, target);
	if ( !(self->moreflags & FL2_COMMANDER) )
	{	// Tactician Gunner fires projectiles, so no backward projection
		VectorMA (target, -0.2, self->enemy->velocity, target);
	}
	target[2] += self->enemy->viewheight;

	// Lazarus fog reduction of accuracy
	if (self->monsterinfo.visibility < FOG_CANSEEGOOD)
	{
		target[0] += crandom() * 640 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
		target[1] += crandom() * 640 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
		target[2] += crandom() * 320 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
	}

	// Knightmare- Tactician Gunner leads the target
	if ( (self->moreflags & FL2_COMMANDER) && (self->monsterinfo.aiflags2 & AI2_LEAD_TARGET) )
	{
		VectorSubtract (target, start, aim);
		dist = VectorLength(aim);
		time = dist / flechetteSpeed;	// was 1000.0f
		VectorCopy (self->enemy->velocity, targ_vel);
		targ_vel[2] = min(targ_vel[2], 0.0f);	// ignore z-velocity of player jumping
		VectorMA (target, time, targ_vel, target);
	}

	VectorSubtract (target, start, aim);
	VectorNormalize (aim);

	// Knightmare- Tactician Gunner fires flechettes
	if (self->moreflags & FL2_COMMANDER) {
#ifndef KMQUAKE2_ENGINE_MOD	// Knightmare- silent muzzleflash and play fire sound directly for vanilla Q2
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (self-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (self->s.origin, MULTICAST_PVS);

		gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, tactician_sound_fire_flechette, 1.0, ATTN_NORM, 0);
#endif	// KMQUAKE2_ENGINE_MOD
		monster_fire_flechette (self, start, aim, 4, flechetteSpeed, 30, 8, flash_number);	// was damage_radius 75, reduced to limit self-damage
	}
	else
		monster_fire_bullet (self, start, aim, 3, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
}

void GunnerGrenade (edict_t *self)
{
	vec3_t		start;
	vec3_t		forward, right, up;
	vec3_t		aim;
	vec_t		monster_speed;
	int			flash_number;
	float		spread;
//	float		pitch;
	// PMM
	vec3_t		target, leadTarget;	
	qboolean	blindfire = false;
	qboolean	leadingTarget = false;
	qboolean	targetSafe = false;
	qboolean	leadSafe = false;
	qboolean	isProx = (self->moreflags & FL2_COMMANDER);
	qboolean	isContact = (self->spawnflags & SF_MONSTER_SPECIAL);

	//PGM
	if (!self->enemy || !self->enemy->inuse)
		return;

	// pmm
	if (self->monsterinfo.aiflags & AI_MANUAL_STEERING)
		blindfire = true;

	if (self->s.frame == FRAME_attak105)
	{
		spread = .02;
		flash_number = MZ2_GUNNER_GRENADE_1;
	}
	else if (self->s.frame == FRAME_attak108)
	{
		spread = .05;
		flash_number = MZ2_GUNNER_GRENADE_2;
	}
	else if (self->s.frame == FRAME_attak111)
	{
		spread = .08;
		flash_number = MZ2_GUNNER_GRENADE_3;
	}
	else // (self->s.frame == FRAME_attak114)
	{
		self->monsterinfo.aiflags &= ~AI_MANUAL_STEERING;
		spread = .11;
		flash_number = MZ2_GUNNER_GRENADE_4;
	}

	//	pmm
	// if we're shooting blind and we still can't see our enemy
	if ( (blindfire) && (!visible(self, self->enemy)) )
	{
		// and we have a valid blind_fire_target
		if (VectorCompare (self->monsterinfo.blind_fire_target, vec3_origin))
			return;
		
	//	gi.dprintf ("blind_fire_target = %s\n", vtos (self->monsterinfo.blind_fire_target));
	//	gi.dprintf ("GunnerGrenade: ideal yaw is %f\n", self->ideal_yaw);
		VectorCopy (self->monsterinfo.blind_fire_target, target);
	}
	else
		VectorCopy (self->s.origin, target);
	// pmm

	AngleVectors (self->s.angles, forward, right, up);	// PGM
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

//PGM
	if (self->enemy)
	{
		float	range;

		VectorSubtract (target, self->s.origin, aim);
		range = VectorLength (aim);

		// aim at enemy's feet if he's at same elevation or lower, otherwise aim at origin
		VectorCopy (self->enemy->s.origin, target);
		if (self->enemy->absmin[2] <= self->absmax[2])
			target[2] = self->enemy->absmin[2];

		// lead target... 20, 35, 50, 65 chance of leading
		if ( random() < (0.2 + skill->value * 0.15) )
		{
			float	dist, time;

			VectorSubtract (target, start, aim);
			dist = VectorLength (aim);
			time = dist / GRENADE_VELOCITY;  // Not correct, but better than nothin'
			VectorMA (target, time, self->enemy->velocity, leadTarget);
			if (!isProx && !isContact)	// delay copying for prox/ctgrenade safety check
				VectorCopy (leadTarget, target);
			leadingTarget = true;
		}

		// Knightmare- run another safety check before firing, so players can't trick us into self-damage
		if (isProx)
		{
			if ( gunner_prox_safety_check(self, start, target) ) {
				VectorCopy (target, self->aim_point);	// save this target point
				targetSafe = true;
			}
			if ( leadingTarget && gunner_prox_safety_check(self, start, leadTarget) ) {
				VectorCopy (leadTarget, target);	// copy lead point over target
				leadSafe = true;
			}
			if ( !targetSafe && !leadSafe ) {
				VectorCopy (self->aim_point, target);	// revert to prev target point
			}
		}
		else if (isContact)
		{
			if ( gunner_ctgrenade_safety_check(self, start, target) ) {
				VectorCopy (target, self->aim_point);	// save this target point
				targetSafe = true;
			}
			if ( leadingTarget && gunner_ctgrenade_safety_check(self, start, leadTarget) ) {
				VectorCopy (leadTarget, target);	// copy lead point over target
				leadSafe = true;
			}
			if ( !targetSafe && !leadSafe ) {
				VectorCopy (self->aim_point, target);	// revert to prev target point
			}
		}
	/*	if ( (isProx || isContact) && (g_showlogic) && (g_showlogic->value) )
		{
			if ( targetSafe && leadSafe )
				gi.dprintf ("GunnerGrenade: safe to fire at and lead target, saving target point.\n");
			else if ( targetSafe && leadingTarget && !leadSafe )
				gi.dprintf ("GunnerGrenade: safe to fire at but not lead target, saving target point.\n");
			else if ( targetSafe && !leadingTarget )
				gi.dprintf ("GunnerGrenade: safe to fire at target, saving target point.\n");
			else if ( !targetSafe && leadSafe )
				gi.dprintf ("GunnerGrenade: safe to lead target only, not saving target point.\n");
			else if ( !targetSafe && !leadSafe && leadingTarget )
				gi.dprintf ("GunnerGrenade: NOT safe to fire at or lead target, reverting to prev target point.\n");
			else if ( !targetSafe && !leadSafe && !leadingTarget )
				gi.dprintf ("GunnerGrenade: NOT safe to fire at target, reverting to prev target point.\n");
		} */

		// Knightmare- spread out Tactician Gunner's prox mines so they don't collide
		if (isProx)
		{
			target[0] += crandom() * GUNNER_PROX_SPREAD;
			target[1] += crandom() * GUNNER_PROX_SPREAD;
		}

		// Lazarus fog reduction of accuracy
		if ( self->monsterinfo.visibility < FOG_CANSEEGOOD )
		{
			target[0] += crandom() * 640 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
			target[1] += crandom() * 640 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
			target[2] += crandom() * 320 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
		}

		// Leading code was here

		// aim up if they're on the same level as me and far away.
	//	if ((range > 512) && (aim[2] < 64) && (aim[2] > -64))
	//		aim[2] += (range - 512);

	/*	VectorNormalize (aim);
		pitch = aim[2];
		if (pitch > 0.4)
			pitch = 0.4;
		else if (pitch < -0.5)
			pitch = -0.5; */
	}
//PGM

	AimGrenade (self, start, target, GRENADE_VELOCITY, aim, (self->moreflags & FL2_COMMANDER));
	// Lazarus - take into account (sort of) feature of adding shooter's velocity to
	// grenade velocity
	monster_speed = VectorLength(self->velocity);
	if (monster_speed > 0)
	{
		vec3_t	v1;
		vec_t	delta;

		VectorCopy (self->velocity, v1);
		VectorNormalize (v1);
		delta = -monster_speed / GRENADE_VELOCITY;
		VectorMA (aim, delta, v1, aim);
		VectorNormalize (aim);
	}

	// FIXME : do a spread -225 -75 75 225 degrees around forward
//	VectorCopy (forward, aim);
//	VectorMA (forward, spread, right, aim);
//	VectorMA (aim, pitch, up, aim);

	if (isProx)	// Knightmare- Tactician Gunner fires prox mines
	{
		float	prox_timer = (blindfire) ? 60.0f : 30.0f;
		monster_fire_prox (self, start, aim, 90, 1, GRENADE_VELOCITY, 20, prox_timer, 192, flash_number);
	}
	else
		monster_fire_grenade (self, start, aim, 50, GRENADE_VELOCITY, flash_number, isContact);
}

mframe_t gunner_frames_attack_chain [] =
{
	/*
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	*/
	ai_charge, 0, gunner_opengun,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t gunner_move_attack_chain = {FRAME_attak209, FRAME_attak215, gunner_frames_attack_chain, gunner_fire_chain};

mframe_t gunner_frames_fire_chain [] =
{
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire,
	ai_charge,   0, GunnerFire
};
mmove_t gunner_move_fire_chain = {FRAME_attak216, FRAME_attak223, gunner_frames_fire_chain, gunner_refire_chain};

mframe_t gunner_frames_endfire_chain [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t gunner_move_endfire_chain = {FRAME_attak224, FRAME_attak230, gunner_frames_endfire_chain, gunner_run};

void gunner_blind_check (edict_t *self)
{
	vec3_t	aim;

	if (self->monsterinfo.aiflags & AI_MANUAL_STEERING)
	{
		VectorSubtract(self->monsterinfo.blind_fire_target, self->s.origin, aim);
		self->ideal_yaw = vectoyaw(aim);
		
	//	gi.dprintf ("blind_fire_target = %s\n", vtos (self->monsterinfo.blind_fire_target));
	//	gi.dprintf ("gunner_attack: ideal yaw is %f\n", self->ideal_yaw);
	}
}

mframe_t gunner_frames_attack_grenade [] =
{
	ai_charge, 0, gunner_blind_check,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, GunnerGrenade,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, GunnerGrenade,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, GunnerGrenade,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, GunnerGrenade,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t gunner_move_attack_grenade = {FRAME_attak101, FRAME_attak121, gunner_frames_attack_grenade, gunner_run};

void gunner_attack (edict_t *self)
{
	float chance, r;

	monster_done_dodge(self);

	// PMM 
	if (self->monsterinfo.attack_state == AS_BLIND)
	{
		// setup shot probabilities
		if (self->monsterinfo.blind_fire_delay < 1.0)
			chance = 1.0;
		else if (self->monsterinfo.blind_fire_delay < 7.5)
			chance = 0.4;
		else
			chance = 0.1;

		r = random();

		// minimum of 2 seconds, plus 0-3, after the shots are done
		self->monsterinfo.blind_fire_delay += 2.1 + 2.0 + random()*3.0;

		// don't shoot at the origin
		if (VectorCompare (self->monsterinfo.blind_fire_target, vec3_origin))
			return;

		// don't shoot if the dice say not to
		if (r > chance)
		{
		//	if ((g_showlogic) && (g_showlogic->value))
		//		gi.dprintf ("blindfire - NO SHOT\n");
			return;
		}

		// turn on manual steering to signal both manual steering and blindfire
	//	self->monsterinfo.aiflags |= AI_MANUAL_STEERING;
		self->monsterinfo.monsterflags |= AI_MANUAL_STEERING;
		if ( gunner_grenade_check(self) )
		{
			// if the check passes, go for the attack
			self->monsterinfo.currentmove = &gunner_move_attack_grenade;
			self->monsterinfo.attack_finished = level.time + 2 * random();
		}
		// pmm - should this be active?
	//	else
	//		self->monsterinfo.currentmove = &gunner_move_attack_chain;
	//	if ((g_showlogic) && (g_showlogic->value))
	//		gi.dprintf ("blind grenade check failed, doing nothing\n");

		// turn off blindfire flag
		self->monsterinfo.aiflags &= ~AI_MANUAL_STEERING;
		return;
	}
	// pmm

	self->monsterinfo.aiflags2 &= ~AI2_LEAD_TARGET;	// Knightmare- reset Tactican Gunner leading target flag

	// PGM - gunner needs to use his chaingun if he's being attacked by a tesla.
	if ((range (self, self->enemy) == RANGE_MELEE) || self->bad_area)
	{
		self->monsterinfo.currentmove = &gunner_move_attack_chain;
	}
	else
	{
		if (random() <= 0.5 && gunner_grenade_check(self)) {
			self->monsterinfo.currentmove = &gunner_move_attack_grenade;
		}
		else
		{
			// Knightmare- Tactician Gunner leads the target
			if (self->moreflags & FL2_COMMANDER)
			{
				vec3_t	forward, right, start, target, aim;
				float	dist, chance;

				AngleVectors (self->s.angles, forward, right, NULL);
				G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_GUNNER_MACHINEGUN_1], forward, right, start);
				VectorCopy (self->enemy->s.origin, target);
				target[2] += self->enemy->viewheight;

				// Lazarus fog reduction of accuracy
				if (self->monsterinfo.visibility < FOG_CANSEEGOOD)
				{
					target[0] += crandom() * 640 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
					target[1] += crandom() * 640 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
					target[2] += crandom() * 320 * (FOG_CANSEEGOOD - self->monsterinfo.visibility);
				}

				VectorSubtract (target, start, aim);
				dist = VectorLength(aim);
				if (dist < 640.0f)	// chance for leading fire if distance is less than 640
				{
				//	if ((g_showlogic) && (g_showlogic->value))
				//		gi.dprintf ("Tactician Gunner: target in range, rolling for chance to lead- ");
					chance = random();	// chance = 50% easy, 60% medium, 70% hard, 80% hard+
					chance += (3.0f - skill->value) * 0.1f;
					if (chance < 0.8f)
					{
					//	if ((g_showlogic) && (g_showlogic->value))
					//		gi.dprintf ("chance passed, leading target\n");
						self->monsterinfo.aiflags2 |= AI2_LEAD_TARGET;
					}
				//	else if ((g_showlogic) && (g_showlogic->value))
				//		gi.dprintf ("chance failed, not leading target\n");
				}
			}
			// end Knightmare
			self->monsterinfo.currentmove = &gunner_move_attack_chain;
		}
	}
}

void gunner_fire_chain (edict_t *self)
{
	self->monsterinfo.currentmove = &gunner_move_fire_chain;
}

void gunner_refire_chain (edict_t *self)
{
	if (self->enemy->health > 0)
		if ( visible (self, self->enemy) )
			if (random() <= 0.5)
			{
				self->monsterinfo.currentmove = &gunner_move_fire_chain;
				return;
			}
	self->monsterinfo.currentmove = &gunner_move_endfire_chain;
}
/*
void gunner_dodge (edict_t *self, edict_t *attacker, float eta, trace_t *tr)
{
// original quake2 dodge code

	if (random() > 0.25)
		return;

	if (!self->enemy)
		self->enemy = attacker;

	self->monsterinfo.currentmove = &gunner_move_duck;

//===========
//PMM - rogue rewrite of gunner dodge code.
	float	r;
	float	height;
	int		shooting = 0;

	if (!self->enemy)
	{
		self->enemy = attacker;
		FoundTarget (self);
	}

	// PMM - don't bother if it's going to hit anyway; fix for weird in-your-face etas (I was
	// seeing numbers like 13 and 14)
	if ((eta < 0.1) || (eta > 5))
		return;

	r = random();
	if (r > (0.25*((skill->value)+1)))
		return;

	if ((self->monsterinfo.currentmove == &gunner_move_attack_chain) ||
		(self->monsterinfo.currentmove == &gunner_move_fire_chain) ||
		(self->monsterinfo.currentmove == &gunner_move_attack_grenade)
		)
	{
		shooting = 1;
	}
	if (self->monsterinfo.aiflags & AI_DODGING)
	{
		height = self->absmax[2];
	}
	else
	{
		height = self->absmax[2]-32-1;  // the -1 is because the absmax is s.origin + maxs + 1
	}

	// check to see if it makes sense to duck
	if (tr->endpos[2] <= height)
	{
		vec3_t right, diff;
		if (shooting)
		{
			self->monsterinfo.attack_state = AS_SLIDING;
			return;
		}
		AngleVectors (self->s.angles, NULL, right, NULL);
		VectorSubtract (tr->endpos, self->s.origin, diff);
		if (DotProduct (right, diff) < 0)
		{
			self->monsterinfo.lefty = 1;
		}
		// if it doesn't sense to duck, try to strafe away
		monster_done_dodge (self);
		self->monsterinfo.currentmove = &gunner_move_run;
		self->monsterinfo.attack_state = AS_SLIDING;
		return;
	}

	if (skill->value == 0)
	{
		self->monsterinfo.currentmove = &gunner_move_duck;
		// PMM - stupid dodge
		self->monsterinfo.duck_wait_time = level.time + eta + 1;
		self->monsterinfo.aiflags |= AI_DODGING;
		return;
	}

	if (!shooting)
	{
		self->monsterinfo.currentmove = &gunner_move_duck;
		self->monsterinfo.duck_wait_time = level.time + eta + (0.1 * (3 - skill->value));
		self->monsterinfo.aiflags |= AI_DODGING;
	}
	return;
//PMM
//===========
}
*/
//===========
//PGM
void gunner_jump_now (edict_t *self)
{
	vec3_t	forward,up;

	monster_jump_start (self);

	AngleVectors (self->s.angles, forward, NULL, up);
	VectorMA(self->velocity, 100, forward, self->velocity);
	VectorMA(self->velocity, 300, up, self->velocity);
}

void gunner_jump2_now (edict_t *self)
{
	vec3_t	forward,up;

	monster_jump_start (self);

	AngleVectors (self->s.angles, forward, NULL, up);
	VectorMA(self->velocity, 150, forward, self->velocity);
	VectorMA(self->velocity, 400, up, self->velocity);
}

void gunner_jump_wait_land (edict_t *self)
{
	if (self->groundentity == NULL)
	{
		self->monsterinfo.nextframe = self->s.frame;

		if (monster_jump_finished (self))
			self->monsterinfo.nextframe = self->s.frame + 1;
	}
	else 
		self->monsterinfo.nextframe = self->s.frame + 1;
}

mframe_t gunner_frames_jump [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, gunner_jump_now,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, gunner_jump_wait_land,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t gunner_move_jump = { FRAME_jump01, FRAME_jump10, gunner_frames_jump, gunner_run };

mframe_t gunner_frames_jump2 [] =
{
	ai_move, -8, NULL,
	ai_move, -4, NULL,
	ai_move, -4, NULL,
	ai_move, 0, gunner_jump_now,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, gunner_jump_wait_land,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t gunner_move_jump2 = { FRAME_jump01, FRAME_jump10, gunner_frames_jump2, gunner_run };

void gunner_jump (edict_t *self)
{
	if (!self->enemy)
		return;

	monster_done_dodge (self);

	if (self->enemy->s.origin[2] > self->s.origin[2])
		self->monsterinfo.currentmove = &gunner_move_jump2;
	else
		self->monsterinfo.currentmove = &gunner_move_jump;
}

//===========
//PGM
qboolean gunner_blocked (edict_t *self, float dist)
{
	if (blocked_checkshot (self, 0.25 + (0.05 * skill->value) ))
		return true;

	if (blocked_checkplat (self, dist))
		return true;

	if (blocked_checkjump (self, dist, 192, 40))
	{
		gunner_jump (self);
		return true;
	}

	return false;
}
//PGM
//===========

// PMM - new duck code
void gunner_duck (edict_t *self, float eta)
{
	if ((self->monsterinfo.currentmove == &gunner_move_jump2) ||
		(self->monsterinfo.currentmove == &gunner_move_jump))
	{
		return;
	}

	if ((self->monsterinfo.currentmove == &gunner_move_attack_chain) ||
		(self->monsterinfo.currentmove == &gunner_move_fire_chain) ||
		(self->monsterinfo.currentmove == &gunner_move_attack_grenade)
		)
	{
		// if we're shooting, and not on easy, don't dodge
		if (skill->value)
		{
			self->monsterinfo.aiflags &= ~AI_DUCKED;
			return;
		}
	}

	if (skill->value == 0)
		// PMM - stupid dodge
		self->monsterinfo.duck_wait_time = level.time + eta + 1;
	else
		self->monsterinfo.duck_wait_time = level.time + eta + (0.1 * (3 - skill->value));

	// has to be done immediately otherwise he can get stuck
	gunner_duck_down(self);

	self->monsterinfo.nextframe = FRAME_duck01;
	self->monsterinfo.currentmove = &gunner_move_duck;
	return;
}

void gunner_sidestep (edict_t *self)
{
	if ((self->monsterinfo.currentmove == &gunner_move_jump2) ||
		(self->monsterinfo.currentmove == &gunner_move_jump))
	{
		return;
	}

	if ((self->monsterinfo.currentmove == &gunner_move_attack_chain) ||
		(self->monsterinfo.currentmove == &gunner_move_fire_chain) ||
		(self->monsterinfo.currentmove == &gunner_move_attack_grenade)
		)
	{
		// if we're shooting, and not on easy, don't dodge
		if (skill->value)
		{
			self->monsterinfo.aiflags &= ~AI_DODGING;
			return;
		}
	}

	if (self->monsterinfo.currentmove != &gunner_move_run)
		self->monsterinfo.currentmove = &gunner_move_run;
}


/*QUAKED monster_gunner (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight GoodGuy NoGib ContactGrenades
*/
/*QUAKED monster_gunner_tactician (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight GoodGuy NoGib
*/
void SP_monster_gunner (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_death = gi.soundindex ("gunner/death1.wav");	
	sound_pain = gi.soundindex ("gunner/gunpain2.wav");	
	sound_pain2 = gi.soundindex ("gunner/gunpain1.wav");	
	sound_idle = gi.soundindex ("gunner/gunidle1.wav");	
	sound_open = gi.soundindex ("gunner/gunatck1.wav");	
	sound_search = gi.soundindex ("gunner/gunsrch1.wav");	
	sound_sight = gi.soundindex ("gunner/sight1.wav");	

//	gi.soundindex ("gunner/gunatck2.wav");	// not used by Tactician Gunner
	gi.soundindex ("gunner/gunatck3.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	// Lazarus: special purpose skins
	if (strcmp(self->classname, "monster_gunner_tactician") == 0)
	{
		self->s.skinnum = 2;
		self->moreflags |= FL2_COMMANDER;
	}
	if ( self->style )
	{
		PatchMonsterModel("models/monsters/gunner/tris.md2");
		self->s.skinnum += self->style * 4;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/gunner/tris.md2");

	{ // Coconut Monkey 2 sombrero
		cvar_t	*gamedir = gi.cvar("gamedir", "", 0);
		if (strlen(gamedir->string) && !strcmp(gamedir->string, "coconut2"))
			self->s.modelindex2 = gi.modelindex ("models/monsters/gunner/gear.md2");
	}

	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	if (strcmp(self->classname, "monster_gunner_tactician") == 0)
	{	// precache
		gi.modelindex ("models/weapons/g_prox/tris.md2");
		gi.modelindex ("models/proj/flechette/tris.md2");
		gi.soundindex ("weapons/proxopen.wav");
		gi.soundindex ("weapons/proxwarn.wav");
#ifdef KMQUAKE2_ENGINE_MOD
		gi.soundindex ("weapons/nail1.wav");
#else
		tactician_sound_fire_flechette = gi.soundindex ("weapons/nail1.wav");	
#endif	// KMQUAKE2_ENGINE_MOD
	/*
		tactician_sound_death = gi.soundindex ("tactician_gunner/death1.wav");	
		tactician_sound_pain = gi.soundindex ("tactician_gunner/gunpain2.wav");	
		tactician_sound_pain2 = gi.soundindex ("tactician_gunner/gunpain1.wav");	
		tactician_sound_idle = gi.soundindex ("tactician_gunner/gunidle1.wav");	
		tactician_sound_open = gi.soundindex ("tactician_gunner/gunatck1.wav");	
		tactician_sound_search = gi.soundindex ("tactician_gunner/gunsrch1.wav");	
		tactician_sound_sight = gi.soundindex ("tactician_gunner/sight1.wav");	
	*/

		if (!self->health)
			self->health = 400;
		if (!self->gib_health)
			self->gib_health = -250;
		if (!self->mass)
			self->mass = 300;

		// Lazarus
		if (self->powerarmor)
		{
			if (self->powerarmortype == 1)
				self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
			else
				self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
			self->monsterinfo.power_armor_power = self->powerarmor;
		}
		else
		{
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
			self->monsterinfo.power_armor_power = 300;
		}

		self->common_name = "Tactician Gunner";
		self->class_id = ENTITY_MONSTER_GUNNER_TACTICIAN;

		self->monsterinfo.monsterflags |= MFL_KNOWS_PROX_MINES;	// Tactician Gunner avoids prox mines
	}
	else
	{	// precache
		gi.modelindex ("models/objects/grenade/tris.md2");
		gi.soundindex ("gunner/gunatck2.wav");

		if (!self->health)
			self->health = 175;
		if (!self->gib_health)
			self->gib_health = -150;
		if (!self->mass)
			self->mass = 200;

		// Lazarus
		if (self->powerarmor)
		{
			if (self->powerarmortype == 1)
				self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
			else
				self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
			self->monsterinfo.power_armor_power = self->powerarmor;
		}

		self->common_name = "Gunner";
		self->class_id = ENTITY_MONSTER_GUNNER;
	}

	self->pain = gunner_pain;
	self->die = gunner_die;

	self->monsterinfo.stand = gunner_stand;
	self->monsterinfo.walk = gunner_walk;
	self->monsterinfo.run = gunner_run;
	// pmm
	self->monsterinfo.dodge = M_MonsterDodge;
	self->monsterinfo.duck = gunner_duck;
	self->monsterinfo.unduck = monster_duck_up;
	self->monsterinfo.sidestep = gunner_sidestep;
//	self->monsterinfo.dodge = gunner_dodge;
	// pmm
	self->monsterinfo.attack = gunner_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = gunner_sight;
	self->monsterinfo.search = gunner_search;
	self->monsterinfo.blocked = gunner_blocked;		//PGM
	
	if (!self->blood_type)
		self->blood_type = 3; // sparks and blood

	if ( !self->monsterinfo.flies && strcmp(self->classname, "monster_gunner_tactician") == 0 )
		self->monsterinfo.flies = 0.20;
	else if (!self->monsterinfo.flies)
		self->monsterinfo.flies = 0.30;

	if (monsterjump->value)
	{
		self->monsterinfo.jump = gunner_jump;
		self->monsterinfo.jumpup = 48;
		self->monsterinfo.jumpdn = 64;
	}

	gi.linkentity (self);

	self->monsterinfo.currentmove = &gunner_move_stand;	
	if (self->health < 0)
	{
		mmove_t	*deathmoves[] = {&gunner_move_death,
								 NULL};
		M_SetDeath (self, (mmove_t **)&deathmoves);
	}
	self->monsterinfo.scale = MODEL_SCALE;

	// PMM
	self->monsterinfo.blindfire = true;

	walkmonster_start (self);
}
