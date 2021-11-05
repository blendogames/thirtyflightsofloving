// m_actor.c
//
// Lazarus 1.4: Adopted Mappack misc_actor code
//
#include "g_local.h"
#include "m_actor.h"
#include "pak.h"

#define ACTOR_WEAP_BLASTER		1
#define ACTOR_WEAP_SHOTGUN		2
#define ACTOR_WEAP_SSHOTGUN		3
#define ACTOR_WEAP_MACHINEGUN	4
#define ACTOR_WEAP_CHAINGUN		5
#define ACTOR_WEAP_GLAUNCHER	6
#define ACTOR_WEAP_RLAUCHER		7
#define ACTOR_WEAP_HYPERBLASTER	8
#define ACTOR_WEAP_RAILGUN		9
#define ACTOR_WEAP_BFG			10
#define ACTOR_WEAP_IONRIPPER	11
#define ACTOR_WEAP_PHALANX		12
#define ACTOR_WEAP_ETFRIFLE		13
#define ACTOR_WEAP_PLASMABEAM	14
#define ACTOR_WEAP_DISRUPTOR	15
#define ACTOR_WEAP_PLASMARIFLE	16

#define ACTOR_WEAP_FIRST		1
#define ACTOR_WEAP_LAST			16

static char wavname[NUM_ACTOR_SOUNDS][32] = 
{ "jump1.wav",
  "pain25_1.wav",
  "pain25_2.wav",
  "pain50_1.wav",
  "pain50_2.wav",
  "pain75_1.wav",
  "pain75_2.wav",
  "pain100_1.wav",
  "pain100_2.wav",
  "death1.wav",
  "death2.wav",
  "death3.wav",
  "death4.wav"
};

#define ACTOR_SOUND_JUMP        0		// Do NOT change this one
#define ACTOR_SOUND_PAIN_25_1   1
#define ACTOR_SOUND_PAIN_25_2   2
#define ACTOR_SOUND_PAIN_50_1   3
#define ACTOR_SOUND_PAIN_50_2   4
#define ACTOR_SOUND_PAIN_75_1   5
#define ACTOR_SOUND_PAIN_75_2   6
#define ACTOR_SOUND_PAIN_100_1  7
#define ACTOR_SOUND_PAIN_100_2  8
#define ACTOR_SOUND_DEATH1      9
#define ACTOR_SOUND_DEATH2     10
#define ACTOR_SOUND_DEATH3     11
#define ACTOR_SOUND_DEATH4     12

mframe_t actor_frames_stand [] =
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

mmove_t actor_move_stand = {FRAME_stand01, FRAME_stand40, actor_frames_stand, NULL};

void actor_stand (edict_t *self)
{
	self->s.sound = 0;
	if (self->monsterinfo.aiflags & AI_CROUCH)
		self->monsterinfo.currentmove = &actor_move_crouch;
	else
		self->monsterinfo.currentmove = &actor_move_stand;

	// randomize on startup
	if (level.time < 1.0)
		self->s.frame = self->monsterinfo.currentmove->firstframe + (rand() % (self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1));
}

mframe_t actor_frames_walk [] =
{
/*	ai_walk, 4,  NULL,
	ai_walk, 15, NULL,
	ai_walk, 15, NULL,
	ai_walk, 8,  NULL,
	ai_walk, 20, NULL,
	ai_walk, 15, NULL,
	ai_walk, 8,  NULL,
	ai_walk, 17, NULL,
	ai_walk, 12, NULL,
	ai_walk, -2, NULL,
	ai_walk, -2, NULL,
	ai_walk, -1, NULL */
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL
};
mmove_t actor_move_walk = {FRAME_run1, FRAME_run6, actor_frames_walk, NULL};

mmove_t actor_move_run;
void actor_walk (edict_t *self)
{
	// prevent foolishness:
	if (self->monsterinfo.aiflags & AI_FOLLOW_LEADER)
	{
		if (!self->movetarget || !self->movetarget->inuse || (self->movetarget == world))
			self->movetarget = self->monsterinfo.leader;
	}

	if ( (self->monsterinfo.aiflags & AI_FOLLOW_LEADER) &&
		(self->movetarget) &&
		(self->movetarget->inuse) &&
		(self->movetarget->health > 0) )
	{
		float	R;

		R = realrange(self,self->movetarget);
		if (R > ACTOR_FOLLOW_RUN_RANGE || self->enemy)
		{
			self->monsterinfo.currentmove = &actor_move_run;
			if (self->monsterinfo.aiflags & AI_CROUCH)
			{
				self->monsterinfo.aiflags &= ~AI_CROUCH;
				self->maxs[2] += 28;
				self->viewheight += 28;
				self->move_origin[2] += 28;
			}
		}
		else if (R <= ACTOR_FOLLOW_STAND_RANGE && self->movetarget->client)
		{
			self->monsterinfo.pausetime = level.time + 0.5;
			if (self->monsterinfo.aiflags & AI_CROUCH)
				self->monsterinfo.currentmove = &actor_move_crouch;
			else
				self->monsterinfo.currentmove = &actor_move_stand;
		}
		else
		{
			if (self->monsterinfo.aiflags & AI_CROUCH)
				self->monsterinfo.currentmove = &actor_move_crouchwalk;
			else
				self->monsterinfo.currentmove = &actor_move_walk;
		}
	}
	else
	{
		if (self->monsterinfo.aiflags & AI_CROUCH)
			self->monsterinfo.currentmove = &actor_move_crouchwalk;
		else
			self->monsterinfo.currentmove = &actor_move_walk;
	}
}

mframe_t actor_frames_walk_back [] =
{
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL
};
mmove_t actor_move_walk_back = {FRAME_run1, FRAME_run6, actor_frames_walk_back, NULL};

mframe_t actor_frames_crouchwalk_back [] =
{
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL,
	ai_walk, -10, NULL
};
mmove_t actor_move_crouchwalk_back = {FRAME_crwalk1, FRAME_crwalk6, actor_frames_crouchwalk_back, NULL};

void actor_walk_back (edict_t *self)
{
	// prevent foolishness:
	if (self->monsterinfo.aiflags & AI_FOLLOW_LEADER)
	{
		if (!self->movetarget || !self->movetarget->inuse || (self->movetarget == world))
			self->movetarget = self->monsterinfo.leader;
	}

	if ( (self->monsterinfo.aiflags & AI_FOLLOW_LEADER) &&
		(self->movetarget) &&
		(self->movetarget->inuse) &&
		(self->movetarget->health > 0) )
	{
		float	R;

		R = realrange(self,self->movetarget);
		if (R <= ACTOR_FOLLOW_STAND_RANGE && self->movetarget->client)
		{
			self->monsterinfo.pausetime = level.time + 0.5;
			if (self->monsterinfo.aiflags & AI_CROUCH)
				self->monsterinfo.currentmove = &actor_move_crouch;
			else
				self->monsterinfo.currentmove = &actor_move_stand;
		}
		else
		{
			if (self->monsterinfo.aiflags & AI_CROUCH)
				self->monsterinfo.currentmove = &actor_move_crouchwalk_back;
			else
				self->monsterinfo.currentmove = &actor_move_walk_back;
		}
	}
	else
	{
		if (self->monsterinfo.aiflags & AI_CROUCH)
			self->monsterinfo.currentmove = &actor_move_crouchwalk_back;
		else
			self->monsterinfo.currentmove = &actor_move_walk_back;
	}
}
mframe_t actor_frames_crouch [] =
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
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t actor_move_crouch = {FRAME_crstnd01, FRAME_crstnd19, actor_frames_crouch, NULL};

mframe_t actor_frames_crouchwalk [] =
{
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL
};
mmove_t actor_move_crouchwalk = {FRAME_crwalk1, FRAME_crwalk6, actor_frames_crouchwalk, NULL};

// DWH: Changed running speed to a constant (equal to player running speed) for normal
//      misc_actor and 2/3 that for "bad guys". Also eliminated excess frames.
mframe_t actor_frames_run [] =
{
	ai_run, 40, NULL,
	ai_run, 40, NULL,
	ai_run, 40, NULL,
	ai_run, 40, NULL,
	ai_run, 40, NULL,
	ai_run, 40, NULL
};
mmove_t actor_move_run = {FRAME_run1, FRAME_run6, actor_frames_run, NULL};

mframe_t actor_frames_run_bad [] =
{
	ai_run, 30, NULL,
	ai_run, 30, NULL,
	ai_run, 30, NULL,
	ai_run, 30, NULL,
	ai_run, 30, NULL,
	ai_run, 30, NULL
};

mmove_t actor_move_run_bad = {FRAME_run1, FRAME_run6, actor_frames_run_bad, NULL};

void actor_run (edict_t *self)
{
	// prevent foolishness
	if (self->monsterinfo.aiflags & AI_FOLLOW_LEADER)
	{
		if (!self->movetarget || !self->movetarget->inuse || (self->movetarget == world))
			self->movetarget = self->monsterinfo.leader;
	}
	if ((level.time < self->pain_debounce_time) && (!self->enemy))
	{
		if (self->movetarget)
			actor_walk(self);
		else
			actor_stand(self);
		return;
	}

	if ( self->monsterinfo.aiflags & AI_STAND_GROUND )
	{
		actor_stand(self);
		return;
	}

	if ( self->monsterinfo.aiflags & AI_CROUCH)
	{
		self->monsterinfo.aiflags &= ~AI_CROUCH;
		self->maxs[2] += 28;
		self->viewheight += 28;
		self->move_origin[2] += 28;
	}

	if ( self->monsterinfo.aiflags & AI_GOOD_GUY ) {
		self->monsterinfo.currentmove = &actor_move_run;
	}
	else
	{
		self->monsterinfo.currentmove = &actor_move_run_bad;
	}
}

mframe_t actor_frames_run_back [] =
{
	ai_run, -40, NULL,
	ai_run, -40, NULL,
	ai_run, -40, NULL,
	ai_run, -40, NULL,
	ai_run, -40, NULL,
	ai_run, -40, NULL
};
mmove_t actor_move_run_back = {FRAME_run1, FRAME_run6, actor_frames_run_back, NULL};

void actor_run_back (edict_t *self)
{
	// prevent foolishness:
	if (self->monsterinfo.aiflags & AI_FOLLOW_LEADER)
	{
		if (!self->movetarget || !self->movetarget->inuse || (self->movetarget == world))
			self->movetarget = self->monsterinfo.leader;
	}
	if ((level.time < self->pain_debounce_time) && (!self->enemy))
	{
		if (self->movetarget)
			actor_walk_back(self);
		else
			actor_stand(self);
		return;
	}

	if ( self->monsterinfo.aiflags & AI_STAND_GROUND )
	{
		actor_stand(self);
		return;
	}

	if ( self->monsterinfo.aiflags & AI_CROUCH)
	{
		self->monsterinfo.aiflags &= ~AI_CROUCH;
		self->maxs[2] += 28;
		self->viewheight += 28;
		self->move_origin[2] += 28;
	}

	self->monsterinfo.currentmove = &actor_move_run_back;
}
mframe_t actor_frames_pain1 [] =
{
	ai_move, -5, NULL,
	ai_move, 4,  NULL,
	ai_move, 1,  NULL,
	ai_move, 1,  NULL
};
mmove_t actor_move_pain1 = {FRAME_pain101, FRAME_pain104, actor_frames_pain1, actor_run};

mframe_t actor_frames_pain2 [] =
{
	ai_move, -4, NULL,
	ai_move, 4,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL
};
mmove_t actor_move_pain2 = {FRAME_pain201, FRAME_pain204, actor_frames_pain2, actor_run};

mframe_t actor_frames_pain3 [] =
{
	ai_move, -1, NULL,
	ai_move, 1,  NULL,
	ai_move, 0,  NULL,
	ai_move, 1,  NULL
};
mmove_t actor_move_pain3 = {FRAME_pain301, FRAME_pain304, actor_frames_pain3, actor_run};

mframe_t actor_frames_flipoff [] =
{
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL
};
mmove_t actor_move_flipoff = {FRAME_flip01, FRAME_flip12, actor_frames_flipoff, actor_run};

mframe_t actor_frames_taunt [] =
{
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL
};
mmove_t actor_move_taunt = {FRAME_taunt01, FRAME_taunt17, actor_frames_taunt, actor_run};

void actor_ideal_range(edict_t *self)
{
	int	weapon;

	weapon = self->actor_weapon[self->actor_current_weapon];

	switch (weapon)
	{
	case ACTOR_WEAP_SHOTGUN:
		self->monsterinfo.ideal_range[0] = 0;
		self->monsterinfo.ideal_range[1] = 270;
		break;
	case ACTOR_WEAP_SSHOTGUN:
		self->monsterinfo.ideal_range[0] = 0;
		self->monsterinfo.ideal_range[1] = 90;
		break;
	case ACTOR_WEAP_MACHINEGUN:
	case ACTOR_WEAP_CHAINGUN:
		self->monsterinfo.ideal_range[0] = 0;
		self->monsterinfo.ideal_range[1] = 450;
		break;
	case ACTOR_WEAP_GLAUNCHER:
		self->monsterinfo.ideal_range[0] = 200;
		self->monsterinfo.ideal_range[1] = 450;
		break;
	case ACTOR_WEAP_RLAUCHER:
		self->monsterinfo.ideal_range[0] = 300;
		self->monsterinfo.ideal_range[1] = 1000;
		break;
	case ACTOR_WEAP_HYPERBLASTER:
		self->monsterinfo.ideal_range[0] = 200;
		self->monsterinfo.ideal_range[1] = 500;
		break;
	case ACTOR_WEAP_RAILGUN:
	case ACTOR_WEAP_BFG:
		self->monsterinfo.ideal_range[0] = 300;
		self->monsterinfo.ideal_range[1] = 1000;
		break;
	// Knightmare- added mission pack weapon support
	case ACTOR_WEAP_IONRIPPER:
		self->monsterinfo.ideal_range[0] = 0;
		self->monsterinfo.ideal_range[1] = 600;
		break;
	case ACTOR_WEAP_PHALANX:
		self->monsterinfo.ideal_range[0] = 300;
		self->monsterinfo.ideal_range[1] = 1000;
		break;
	case ACTOR_WEAP_ETFRIFLE:
		self->monsterinfo.ideal_range[0] = 100;
		self->monsterinfo.ideal_range[1] = 1000;
		break;
	case ACTOR_WEAP_PLASMABEAM:
		self->monsterinfo.ideal_range[0] = 0;
		self->monsterinfo.ideal_range[1] = 1500;
		break;
	case ACTOR_WEAP_DISRUPTOR:
		self->monsterinfo.ideal_range[0] = 0;
		self->monsterinfo.ideal_range[1] = 1000;
		break;
	case ACTOR_WEAP_PLASMARIFLE:
		self->monsterinfo.ideal_range[0] = 0;
		self->monsterinfo.ideal_range[1] = 1000;
		break;
	// end Knightmare
	default:
		self->monsterinfo.ideal_range[0] = 0;
		self->monsterinfo.ideal_range[1] = 0;
	}
}

void actor_attack(edict_t *self);
void actor_switch (edict_t *self)
{
	self->actor_current_weapon = 1 - self->actor_current_weapon;
	self->s.modelindex2 = self->actor_model_index[self->actor_current_weapon];
	actor_ideal_range (self);
	gi.linkentity(self);
}
mframe_t actor_frames_switch [] =
{
	ai_run, 0, actor_switch,
	ai_run, 0, NULL,
	ai_run, 0, NULL
};
mmove_t actor_move_switch = {FRAME_jump4, FRAME_jump6, actor_frames_switch, actor_attack};

void actor_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;
	int		r, l;

	// DWH: Players don't have pain skins!
//	if (self->health < (self->max_health / 2))
//		self->s.skinnum = 1;

	// Stop weapon sound, if any
	self->s.sound = 0;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 1;

	// DWH: Use same scheme used for player pain sounds
	if (!(self->flags & FL_GODMODE))
	{
		r = (rand()&1);
		if (self->health < 25)
			l = 0;
		else if (self->health < 50)
			l = 2;
		else if (self->health < 75)
			l = 4;
		else
			l = 6;
		gi.sound (self, CHAN_VOICE, self->actor_sound_index[ACTOR_SOUND_PAIN_25_1 + l + r],
			1, ATTN_NORM, 0);
	}

	// Lazarus: Removed printed message, but keep taunt (not for monster actors, though)
	if ((other->client) && (random() < 0.4) && (self->monsterinfo.aiflags & AI_GOOD_GUY))
	{
		vec3_t	v;
		VectorSubtract (other->s.origin, self->s.origin, v);
		self->ideal_yaw = vectoyaw (v);
		if (random() < 0.5) {
			self->monsterinfo.currentmove = &actor_move_flipoff;
		}
		else
		{
			self->monsterinfo.currentmove = &actor_move_taunt;
		}
		return;
	}


	n = rand() % 3;
	if (n == 0)
		self->monsterinfo.currentmove = &actor_move_pain1;
	else if (n == 1)
		self->monsterinfo.currentmove = &actor_move_pain2;
	else
		self->monsterinfo.currentmove = &actor_move_pain3;

}

//
// Attack code moved to m_actor_weap.c
//

void actor_dead (edict_t *self)
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
		self->think=FadeDieSink;
		self->nextthink=level.time+corpse_fadetime->value;
	}
}

mframe_t actor_frames_death1 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, -13, NULL,
	ai_move, 14,  NULL,
	ai_move, 3,   NULL,
	ai_move, -2,  NULL
};
mmove_t actor_move_death1 = {FRAME_death101, FRAME_death106, actor_frames_death1, actor_dead};

mframe_t actor_frames_death2 [] =
{
	ai_move, 0,   NULL,
	ai_move, 7,   NULL,
	ai_move, -6,  NULL,
	ai_move, -5,  NULL,
	ai_move, 1,   NULL,
	ai_move, 0,   NULL
};
mmove_t actor_move_death2 = {FRAME_death201, FRAME_death206, actor_frames_death2, actor_dead};

void actor_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// Remove the weapon model and turn off weapon sound, if any
	self->s.modelindex2 = 0;
	self->s.sound = 0;

	// check for gib
	if (self->health <= self->gib_health && !(self->spawnflags & SF_MONSTER_NOGIB))
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n = 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", 0, 0, damage, GIB_ORGANIC);
		for (n = 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", 0, 0, damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// Knightmare- hack to prevent falling through floor
	self->s.origin[2] += 1;

	// regular death
	gi.sound (self, CHAN_VOICE, 
		self->actor_sound_index[ACTOR_SOUND_DEATH1 + (rand()%4)], 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if (self->monsterinfo.aiflags & AI_CHASE_THING)
	{
		if (self->movetarget && !Q_stricmp(self->movetarget->classname, "thing"))
		{
			G_FreeEdict(self->movetarget);
			self->movetarget = NULL;
		}
	}
	self->monsterinfo.aiflags &= ~(AI_FOLLOW_LEADER | AI_CHASE_THING | AI_CHICKEN | AI_EVADE_GRENADE);
	if (random() > 0.5)
		self->monsterinfo.currentmove = &actor_move_death1;
	else
		self->monsterinfo.currentmove = &actor_move_death2;
}

void actor_fire (edict_t *self)
{
	int	weapon;

	weapon = self->actor_weapon[self->actor_current_weapon];

	switch (weapon)
	{
	case ACTOR_WEAP_BLASTER:
		actorBlaster (self);
		break;
	case ACTOR_WEAP_SHOTGUN:
		actorShotgun (self);
		break;
	case ACTOR_WEAP_SSHOTGUN:
		actorSuperShotgun (self);
		break;
	case ACTOR_WEAP_MACHINEGUN:
		actorMachineGun (self);
		if (level.time >= self->monsterinfo.pausetime)
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		else
			self->monsterinfo.aiflags |= AI_HOLD_FRAME;
		break;
	case ACTOR_WEAP_CHAINGUN:
		actorChaingun (self);
		if (level.time >= self->monsterinfo.pausetime)
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		else
			self->monsterinfo.aiflags |= AI_HOLD_FRAME;
		break;
	case ACTOR_WEAP_GLAUNCHER:
		actorGrenadeLauncher (self);
		break;
	case ACTOR_WEAP_RLAUCHER:
		actorRocket (self);
		break;
	case ACTOR_WEAP_HYPERBLASTER:
		actorHyperblaster(self);
		if (level.time >= self->monsterinfo.pausetime)
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		else
			self->monsterinfo.aiflags |= AI_HOLD_FRAME;
		break;
	case ACTOR_WEAP_RAILGUN:
		actorRailGun (self);
		break;
	case ACTOR_WEAP_BFG:
		actorBFG (self);
		if (level.time >= self->monsterinfo.pausetime)
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		else
			self->monsterinfo.aiflags |= AI_HOLD_FRAME;
		break;
	case ACTOR_WEAP_IONRIPPER:
		actorIonripper(self);
		break;
	case ACTOR_WEAP_PHALANX:
		actorPhalanx (self);
		if (level.time >= self->monsterinfo.pausetime)
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		else
			self->monsterinfo.aiflags |= AI_HOLD_FRAME;
		break;
	case ACTOR_WEAP_ETFRIFLE:
		actorETF_Rifle (self);
		if (level.time >= self->monsterinfo.pausetime)
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		else
			self->monsterinfo.aiflags |= AI_HOLD_FRAME;
		break;
	case ACTOR_WEAP_PLASMABEAM:
		actorPlasmaBeam (self);
		if (level.time >= self->monsterinfo.pausetime)
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
		else
			self->monsterinfo.aiflags |= AI_HOLD_FRAME;
		break;
	case ACTOR_WEAP_DISRUPTOR:
		actorDisintegrator (self);
		break;
	case ACTOR_WEAP_PLASMARIFLE:
		actorPlasmaRifle (self);
		break;
	}
}

void actor_no_weapon_sound (edict_t *self)
{
	self->s.sound = 0;
	gi.linkentity(self);
}

static int chase_angle[] = {360, 315, 405, 270, 450, 225, 495, 540};
void actor_seekcover (edict_t *self)
{
	int		i;
	edict_t	*thing;
	vec3_t	atk, dir, best_dir, end, forward;
	vec_t	travel, yaw;
	vec3_t	mins, maxs;
	vec3_t	testpos;
	vec_t	best_dist=0;
	trace_t	trace1, trace2;

	// No point in hiding from enemy if.. we don't have an enemy
	if ( !self->enemy || !self->enemy->inuse )
	{
		actor_run (self);
		return;
	}
	if (!actorscram->value)
	{
		actor_run (self);
		return;
	}
	// Don't hide from non-humanoid stuff
	if ( !self->enemy->client && !(self->enemy->svflags & SVF_MONSTER) )
	{
		actor_run(self);
		return;
	}
	// This shouldn't happen, we're just being cautious. Quit now if
	// already chasing a "thing"
	if ( self->movetarget && !Q_stricmp(self->movetarget->classname, "thing") )
	{
		actor_run (self);
		return;
	}
	// Don't bother finding cover if we're within melee range of enemy
	VectorSubtract (self->enemy->s.origin, self->s.origin, atk);
	if (VectorLength(atk) < 80)
	{
		actor_run (self);
		return;
	}
	VectorCopy (self->mins, mins);
	mins[2] += 18;
	if (mins[2] > 0)
		mins[2] = 0;
	VectorCopy (self->maxs, maxs);

	// Find a vector that will hide the actor from his enemy
	VectorCopy (self->enemy->s.origin, atk);
	atk[2] += self->enemy->viewheight;
	VectorClear(best_dir);
	AngleVectors (self->s.angles, forward, NULL, NULL);
	dir[2] = 0;
	for (travel=64; travel<257 && best_dist == 0; travel *= 2)
	{
		for (i=0; i<8 && best_dist == 0; i++)
		{
			yaw = self->s.angles[YAW] + chase_angle[i];
			yaw = (int)(yaw/45)*45;
			yaw = anglemod(yaw);
			yaw *= M_PI/180;
			dir[0] = cos(yaw);
			dir[1] = sin(yaw);
			VectorMA (self->s.origin, travel, dir, end);
			trace1 = gi.trace(self->s.origin, mins, maxs, end, self, MASK_MONSTERSOLID);
			// Test whether proposed position can be seen by enemy. Test
			// isn't foolproof - tests against 1) new origin, 2-5) each corner of top
			// of bounding box.
			trace2 = gi.trace(trace1.endpos, NULL, NULL, atk, self, MASK_SOLID);
			if (trace2.fraction == 1.0) continue;

			VectorAdd (trace1.endpos, self->maxs, testpos);
			trace2 = gi.trace(testpos,NULL,NULL,atk,self,MASK_SOLID);
			if (trace2.fraction == 1.0) continue;

			testpos[0] = trace1.endpos[0] + self->mins[0];
			trace2 = gi.trace(testpos, NULL, NULL, atk, self, MASK_SOLID);
			if (trace2.fraction == 1.0) continue;

			testpos[1] = trace1.endpos[1] + self->mins[1];
			trace2 = gi.trace(testpos, NULL, NULL, atk, self, MASK_SOLID);
			if (trace2.fraction == 1.0) continue;

			testpos[0] = trace1.endpos[0] + self->maxs[0];
			trace2 = gi.trace(testpos, NULL, NULL, atk, self, MASK_SOLID);
			if (trace2.fraction == 1.0) continue;

			best_dist = trace1.fraction * travel;
			if (best_dist < 32) // not much point to this move
				continue;
			VectorCopy(dir,best_dir);
		}
	}
	if (best_dist < 32)
	{
		actor_run (self);
		return;
	}
	// This snaps the angles, which may not be all that good but it sure
	// is quicker than turning in place
	vectoangles (best_dir, self->s.angles);
	thing = SpawnThing();
	VectorMA (self->s.origin, best_dist, best_dir, thing->s.origin);
	thing->touch_debounce_time = level.time + 3.0;
	thing->target_ent = self;
	ED_CallSpawn (thing);
	self->movetarget = self->goalentity = thing;
	self->monsterinfo.aiflags &= ~(AI_SOUND_TARGET | AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
	self->monsterinfo.aiflags |= (AI_SEEK_COVER | AI_CHASE_THING);
	gi.linkentity (self);
	actor_run (self);
}

mframe_t actor_frames_attack [] =
{
	ai_charge, 0,  actor_fire,
	ai_charge, 0,  actor_no_weapon_sound,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL
};
mmove_t actor_move_attack = {FRAME_attack1, FRAME_attack8, actor_frames_attack, actor_seekcover};

mframe_t actor_frames_crattack [] =
{
	ai_charge, 0,  actor_fire,
	ai_charge, 0,  actor_no_weapon_sound,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL
};
mmove_t actor_move_crattack = {FRAME_crattak1, FRAME_crattak9, actor_frames_crattack, actor_run};

void actor_attack (edict_t *self)
{
	int		n;
	int		weapon, w_select;
	mmove_t	*attackmove;
	vec3_t	v;

	w_select = self->actor_current_weapon;
	weapon   = self->actor_weapon[w_select];

	if (self->enemy)
	{
		if (w_select == 0 && self->actor_weapon[1] > 0)
		{
			VectorSubtract(self->s.origin,self->enemy->s.origin,v);
			if (VectorLength(v) < 200)
			{
				self->monsterinfo.currentmove = &actor_move_switch;
				return;
			}
		}
		else if (w_select == 1 && self->actor_weapon[0] > 0)
		{
			VectorSubtract(self->s.origin,self->enemy->s.origin,v);
			if (VectorLength(v) > 300)
			{
				self->monsterinfo.currentmove = &actor_move_switch;
				return;
			}
		}
	}

	self->actor_gunframe = 0;

	// temporary deal to toggle crouch
/*	if (self->actor_crouch_time < level.time)
		self->actor_crouch_time = level.time + 5;
	else
		self->actor_crouch_time = 0; */
	// end temp

	if (self->actor_crouch_time < level.time)
		attackmove = &actor_move_attack;
	else
		attackmove = &actor_move_crattack;

	switch (weapon)
	{
	case ACTOR_WEAP_BLASTER:
		self->monsterinfo.currentmove = attackmove;
		self->monsterinfo.pausetime = level.time + 2 * FRAMETIME;
		break;
	case ACTOR_WEAP_SHOTGUN:
		self->monsterinfo.currentmove = attackmove;
		self->monsterinfo.pausetime = level.time + 6 * FRAMETIME;
		break;
	case ACTOR_WEAP_SSHOTGUN:
		self->monsterinfo.currentmove = attackmove;
		self->monsterinfo.pausetime = level.time + 10 * FRAMETIME;
		break;
	case ACTOR_WEAP_MACHINEGUN:
		self->monsterinfo.currentmove = attackmove;
		n = (rand() & 15) + 10;
		self->monsterinfo.pausetime = level.time + n * FRAMETIME;
		break;
	case ACTOR_WEAP_CHAINGUN:
		self->monsterinfo.currentmove = attackmove;
		n = (rand() & 20) + 20;
		self->monsterinfo.pausetime = level.time + n * FRAMETIME;
		break;
	case ACTOR_WEAP_GLAUNCHER:
	case  ACTOR_WEAP_RLAUCHER:
		self->monsterinfo.currentmove = attackmove;
		if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		{ // if hes just standing there refire rate is normal
			self->monsterinfo.pausetime = level.time + 7;
		}
		else
		{ // otherwise, allow the target to fire back
			self->monsterinfo.pausetime = level.time + 2;
		}
		break;
	case ACTOR_WEAP_HYPERBLASTER:
		self->monsterinfo.currentmove = attackmove;
		n = (rand() & 15) + 3 + 7;
		self->monsterinfo.pausetime = level.time + n * FRAMETIME;
		break;
	case ACTOR_WEAP_RAILGUN:
		self->monsterinfo.currentmove = attackmove;
		self->monsterinfo.pausetime = level.time + 3;
		break;
	case ACTOR_WEAP_BFG:
		if (level.time > self->endtime)
		{
			self->monsterinfo.currentmove = attackmove;
			self->monsterinfo.pausetime = level.time + 1.5;
		}
		else
			self->monsterinfo.currentmove = &actor_move_stand;
		break;
	case ACTOR_WEAP_IONRIPPER:
		self->monsterinfo.currentmove = attackmove;
		self->monsterinfo.pausetime = level.time + 6 * FRAMETIME;
		break;
	case ACTOR_WEAP_PHALANX:
		self->monsterinfo.currentmove = attackmove;
	//	self->monsterinfo.pausetime = level.time + 4;
		if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		{ // if hes just standing there refire rate is normal
			self->monsterinfo.pausetime = level.time + 5;
		}
		else
		{ // otherwise, allow the target to fire back
			self->monsterinfo.pausetime = level.time + 2;
		}
		break;
	case ACTOR_WEAP_ETFRIFLE:
		self->monsterinfo.currentmove = attackmove;
		n = (rand() & 11) + 7;
		self->monsterinfo.pausetime = level.time + n * FRAMETIME;
		break;
	case ACTOR_WEAP_PLASMABEAM:
		self->monsterinfo.currentmove = attackmove;
		n = (rand() & 10) + 5;
		self->monsterinfo.pausetime = level.time + n * FRAMETIME;
		break;
	case ACTOR_WEAP_DISRUPTOR:
		self->monsterinfo.currentmove = attackmove;
		self->monsterinfo.pausetime = level.time + 9 * FRAMETIME;
		break;
	case ACTOR_WEAP_PLASMARIFLE:
		self->monsterinfo.currentmove = attackmove;
		self->monsterinfo.pausetime = level.time + 18 * FRAMETIME;
		break;
	}
}

void actor_use (edict_t *self, edict_t *other, edict_t *activator)
{
	vec3_t	v;

	self->goalentity = self->movetarget = G_PickTarget(self->target);
	if ( (!self->movetarget) || (strcmp(self->movetarget->classname, "target_actor") != 0) )
	{
		gi.dprintf ("%s has bad target %s at %s\n", self->classname, self->target, vtos(self->s.origin));
		self->target = NULL;
		self->monsterinfo.pausetime = 100000000;
		self->monsterinfo.stand (self);
		return;
	}
	VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
	self->ideal_yaw = self->s.angles[YAW] = vectoyaw(v);
	self->monsterinfo.walk (self);
	self->target = NULL;
	if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		G_UseTargets (self, other);
		self->target = savetarget;
	}
}

// Lazarus: checkattack - higher probabilities than normal monsters,
//          also weapon-based

static float chancefar[16] =  { 0.0, 0.2, 0.0, 0.0, 0.0, 0.0, 0.2, 0.4, 0.2, 0.5, 0.8, 0.3, 0.4, 0.4, 0.5, 0.3 };
static float chancenear[16] = { 0.0, 0.4, 0.0, 0.0, 0.0, 0.0, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4 };
qboolean actor_checkattack (edict_t *self)
{
	vec3_t		v;
	vec3_t		forward, right, start, end;
	float		chance;
	float		range;
	float		goodchance, poorchance, lorange, hirange;
	trace_t		tr;
	int			weapon;

	// Paranoia check
	if (!self->enemy)
		return false;

	// If running to "thing", never attack
	if (self->monsterinfo.aiflags & AI_CHASE_THING)
		return false;

	weapon = self->actor_weapon[self->actor_current_weapon];
	// If actor has no weapon, well then of course he should not attack
	if ( (weapon < ACTOR_WEAP_FIRST) || (weapon > ACTOR_WEAP_LAST) ) // Knightmare- mission pack weapon support, was > 10
			return false;

	if (self->enemy->health > 0)
	{
		// see if any entities are in the way of the shot
		AngleVectors (self->s.angles, forward, right, NULL);
		G_ProjectSource (self->s.origin, self->muzzle, forward, right, start);
		VectorCopy (self->enemy->s.origin, end);

		tr = gi.trace (start, NULL, NULL, end, self, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_SLIME|CONTENTS_LAVA|CONTENTS_WINDOW);

 		// do we have a clear shot?
		if (tr.ent != self->enemy)
			return false;
	}

	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	range = VectorLength (v);

	// melee attack
	if (range <= MELEE_DISTANCE)
	{
		// don't always melee in easy mode
		if (skill->value == 0 && (rand()&3) )
			return false;
		self->monsterinfo.attack_state = AS_MISSILE;
		return true;
	}
	
	// missile attack
	if (!self->monsterinfo.attack)
		return false;

	if (level.time < self->monsterinfo.attack_finished)
		return false;
		
	if (range > self->monsterinfo.max_range)
		return false;
 
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		chance = 0.4;
	}
	else
	{
		if ( (weapon >= ACTOR_WEAP_SHOTGUN) && (weapon <= ACTOR_WEAP_CHAINGUN) )
		{
			// Scatter guns - probability of firing based on percentage of rounds
			// that will hit target at a given range.
			if (skill->value == 1)
				goodchance = 0.6;
			else if (skill->value > 1)
				goodchance = 0.9;
			else
				goodchance = 0.3;
			poorchance = 0.01;
			switch (weapon)
			{
			case ACTOR_WEAP_SHOTGUN:	lorange = 270;	hirange = 500;	break;
			case ACTOR_WEAP_SSHOTGUN:	lorange = 90;	hirange = 200;	break;
			case ACTOR_WEAP_MACHINEGUN:	lorange = 450;	hirange = 628;	break;
			case ACTOR_WEAP_CHAINGUN:	lorange = 450;	hirange = 628;	break;
			}
			if (range <= lorange)
				chance = goodchance;
			else if (range > hirange)
				chance = poorchance;
			else
				chance = goodchance + (range - lorange) / (hirange - lorange) * (poorchance - goodchance);
		}
		else
		{
			if (range <= 500)
				chance = chancenear[weapon];
			else
				chance = chancefar[weapon];
			if (self->monsterinfo.aiflags & AI_GOOD_GUY)
			{
				if (skill->value == 0)
					chance *= 2;
				else if (skill->value == 2)
					chance *= 0.5;
				else if (skill->value == 3)
					chance *= 0.25;
			}
			else
			{
				if (skill->value == 0)
					chance *= 0.5;
				else if (skill->value == 2)
					chance *= 2;
				else if (skill->value == 3)
					chance *= 4;
			}
		}
	}

	if (random () < chance)
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		self->monsterinfo.attack_finished = level.time + 2 * random();
		return true;
	}

	return false;
}

mmove_t actor_move_jump;
void actor_end_jump (edict_t *self)
{
	if (self->flags & FL_ROBOT)
	{
		if (self->monsterinfo.savemove)
		{
			actor_run(self);
		//	self->monsterinfo.currentmove = self->monsterinfo.savemove;
		/*	gi.dprintf("savemove=%d\n", self->monsterinfo.currentmove);
			gi.dprintf("actor_move_jump=%d\n", &actor_move_jump);
			gi.dprintf("actor_move_run=%d\n", &actor_move_run);
			gi.dprintf("actor_move_walk=%d\n", &actor_move_walk);
			gi.dprintf("actor_move_stand=%d\n", &actor_move_stand); */
		}
		else if (self->enemy)
			actor_run (self);
		else if (self->movetarget)
			actor_walk (self);
		else
			actor_stand (self);
	}
	else
		actor_run (self);
}

mframe_t actor_frames_jump [] =
{
	ai_move,  0, NULL,
	ai_move,  0, NULL,
	ai_move,  0, NULL,
	ai_move,  0, NULL,
	ai_move,  0, NULL,
	ai_move,  0, actor_end_jump
};

mmove_t actor_move_jump = {FRAME_jump1, FRAME_jump6, actor_frames_jump, actor_jump};

void actor_jump (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, self->actor_sound_index[ACTOR_SOUND_JUMP], 1, ATTN_NORM, 0);
	self->monsterinfo.currentmove = &actor_move_jump;
}
qboolean actor_blocked (edict_t *self, float dist)
{
	if (blocked_checkshot (self, 0.25 + (0.05 * skill->value) ))
		return true;

	if (blocked_checkjump (self, dist, self->monsterinfo.jumpdn, self->monsterinfo.jumpup))
		return true;

	if (blocked_checkplat (self, dist))
		return true;

	return false;
}

mframe_t actor_frames_salute [] =
{
	ai_turn, 0, NULL,
	ai_turn, 0, NULL,
	ai_turn, 0, NULL,
	ai_turn, 0, NULL,
	ai_turn, 0, NULL,
	ai_turn, 0, NULL,
	ai_turn, 0, NULL,
	ai_turn, 0, NULL,
	ai_turn, 0, NULL,
	ai_turn, 0, NULL,
	ai_turn, 0, NULL
};
mmove_t actor_move_salute = {FRAME_salute01, FRAME_salute11, actor_frames_salute, actor_run};

void actor_salute (edict_t *self)
{
	self->monsterinfo.currentmove = &actor_move_salute;
}

/*QUAKED misc_actor (1 .5 0) (-16 -16 -24) (16 16 32) Ambush TrigSpawn Sight GoodGuy NoGib Homing BeMonster IgnoreFire
"angle" Specifies the facing angle of the actor on the XY plane. Default=0.
"angles" Specifies the facing angle of the actor in 3 dimensions as defined by pitch, yaw, and roll. Default=0 0 0.
"bleft" Specifies bottom-left (minY/minX/minZ) bounding box coordinates for the actor relative to the actor's origin.
"combattarget" targetname of the point_combat the actor will move to when angered.
"deathtarget" targetname of the entity to be triggered upon the actor's death.
"dmgteam" An actor with a dmgteam value set will treat all attacks upon other actor(s) or monster(s) with the same dmgteam value as an attack on himself. May also be used to trigger a trigger_relay.
"flies" Specifies the probability that the actor's corpse will produce flies. When they appear, they will have a duration of 60 seconds before they expire. Flies will never appear if the actor's corpse is in a liquid. Normal probability values range from 0.00-1.00; 0.00=0% probability (never produce flies); 1.00=100% probability (always produce flies). If flies>1, the actor will have flies buzzing about him even while alive. Default=0.
Ignored if health<0, unless flies=1. In that event, the initially dead actor will have flies which never expire.
"gib_health" Specifies the number of hit points required to gib the actor's corpse. Ignored when NO_GIB is set. Default=40.
"health" Specifies the number of hit points required to kill the actor. Default=100
   If health=0 it will be reset to the default.
   If health<0 the actor will be dead on startup.
   If health=100000 or more the actor is invulnerable, and will ignore attacks.
"item" entity to be spawned by the actor upon his death.
"killtarget" targetname of the entity to be removed from the map upon the actor's death.
"mass" weight of the actor. Default=200.
"movewith" targetname of the parent entity the actor is to movewith.
"muzzle" Specifies weapon firing origin, offset from the actor origin.
"muzzle2" Specifies an optional 2nd weapon firing origin, offset from the actor origin.
"powerarmor" Specifies amount of optional power shield armor the actor has. Default=0. Experiment with settings using the weapons the actor will face - it looks odd for a actor to die before his power shield runs out.
"sounds" weapon to use. Default=0
     0 : None
   100 : Blaster
   200 : Shotgun
   300 : Super Shotgun
   400 : Machinegun
   500 : Chaingun
   603 : GL/SSG
   703 : RL/SSG
   800 : Hyperblaster
   900 : Railgun
  1003 : BFG/SSG
  1100 : ION Ripper
  1203 : Phalanx/SSG
  1300 : ETF Rifle
  1400 : Plasma beam
  1500 : Disintegrator
  1600 : Plasma Rifle (LMSP)
"style" Specifies the 0-based index of the skin the actor will use. Default=0. The valid range for style is 0 to the number of skins referenced from within the model itself minus one. What number references what skin is dependent on the individual model. If the style value +1 exceeds the number of referenced skins, skin 0 will be used. The number of available skins for all player models distributed with Lazarus are given in the ActorPak documentation. 
"target" targetname of the path_corner the actor will move to.
"targetname" Name of the specific actor.
"tright" Specifies top-right (maxY/maxX/maxZ) bounding box coordinates for the actor relative to the actor's origin. If no value is specified for either tright or bleft, and the value of usermodel is a player model in the Lazarus ActorPak, then the suggested bounding box as listed on the ActorPak page for that model will be used. 
"usermodel" Specifies the player model used. Value is the name of the model's "player" folder. No default is specified for usermodel. If the model folder contains custom sounds, misc_actor will use them; otherwise it will fall back to using male model sounds.
*/

#define ACTOR_ALIEN       1
#define ACTOR_HUNTER      2
#define ACTOR_PARANOID    3
#define ACTOR_RATAMAHATTA 4
#define ACTOR_RHINO       5
#define ACTOR_SAS         6
#define ACTOR_SLITH       7
#define ACTOR_TERRAN      8
#define ACTOR_WALKER      9
#define ACTOR_WASTE      10
#define ACTOR_XENOID     11
#define ACTOR_ZUMLIN     12

#define NUM_ACTORPAK_ACTORS 12
char ActorNames[NUM_ACTORPAK_ACTORS][32] = 
{ "alien",      "hunter",     "paranoid","ratamahatta",
  "rhino",      "sas",        "slith",   "terran",
  "walker",     "waste",      "xenoid",  "zumlin" };

void SP_misc_actor (edict_t *self)
{
	char	modelpath[256];
	char	*p;
	int		i, weapon;
	int		ActorID = 0;
	size_t	modelSize;

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->class_id = ENTITY_MISC_ACTOR;
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	if (self->usermodel)
	{
		p = strstr(self->usermodel, "/tris.md2");
		if (p) *p = 0;
	}
	else
	{
		modelSize = 5;
		self->usermodel = gi.TagMalloc(modelSize, TAG_LEVEL);
		Com_strcpy (self->usermodel, modelSize, "male");
	}

	if ( (!Q_stricmp(self->usermodel, "male")) ||
		(!Q_stricmp(self->usermodel, "female")) ||
		(!Q_stricmp(self->usermodel, "cyborg")) )
	{
		self->actor_id_model = true;
		if ( PatchPlayerModels(self->usermodel) )
			level.restart_for_actor_models = true;
	}
	else
		self->actor_id_model = false;

	Com_sprintf (modelpath, sizeof(modelpath), "players/%s/tris.md2", self->usermodel);
	self->s.modelindex = gi.modelindex(modelpath);

	for (i=0; i<NUM_ACTORPAK_ACTORS && !ActorID; i++)
	{
		if (!Q_stricmp(self->usermodel, ActorNames[i]))
			ActorID = i+1;
	}

	if ( !VectorLength(self->bleft) && !VectorLength(self->tright) )
	{
		switch (ActorID)
		{
		case ACTOR_ALIEN:
			VectorSet (self->mins, -28, -28, -24);
			VectorSet (self->maxs,  28,  28,  32);
			break;
		case ACTOR_HUNTER:
			VectorSet (self->mins, -24, -24, -24);
			VectorSet (self->maxs,  24,  24,  32);
			break;
		case ACTOR_RATAMAHATTA:
		case ACTOR_TERRAN:
			VectorSet (self->mins, -20, -20, -24);
			VectorSet (self->maxs,  20,  20,  32);
			break;
		case ACTOR_RHINO:
			VectorSet (self->mins, -30, -30, -24);
			VectorSet (self->maxs,  30,  30,  32);
			break;
		case ACTOR_SAS:
		case ACTOR_XENOID:
		case ACTOR_ZUMLIN:
			VectorSet (self->mins, -18, -18, -24);
			VectorSet (self->maxs,  18,  18,  32);
			break;
		case ACTOR_WALKER:
			VectorSet (self->mins, -24, -24, -24);
			VectorSet (self->maxs,  24,  24,  30);
			break;
		default:
			VectorSet (self->mins, -16, -16, -24);
			VectorSet (self->maxs,  16,  16,  32);
		}
	}
	else
	{
		VectorCopy (self->bleft,  self->mins);
		VectorCopy (self->tright, self->maxs);
	}

	if (!self->health)
		self->health = 100;
	if (!self->gib_health)
		self->gib_health = -40;
	if (!self->mass)
		self->mass = 200;

	if (self->sounds < 0)
	{
		self->actor_weapon[0] = 0;
		self->actor_weapon[1] = -self->sounds;
	}
	else if (self->sounds <= ACTOR_WEAP_LAST)	// Knightmare- added mission pack weapon support, was <= 10
	{
		self->actor_weapon[0] = self->sounds;
		self->actor_weapon[1] = 0;
	}
	else
	{
		self->actor_weapon[0] = self->sounds / 100;
		self->actor_weapon[1] = self->sounds % 100;
	}

	if ( !VectorLength(self->muzzle) )
	{
		switch (ActorID)
		{
		case ACTOR_ALIEN:
			VectorSet (self->muzzle, 42, 5, 15);
			break;
		case ACTOR_HUNTER:
			switch (self->actor_weapon[0])
			{
			case ACTOR_WEAP_BLASTER:		VectorSet (self->muzzle, 32, 5, 15);	break;
			case ACTOR_WEAP_SHOTGUN:		VectorSet (self->muzzle, 36, 5, 15);	break;
			case ACTOR_WEAP_SSHOTGUN:		VectorSet (self->muzzle, 36, 5, 15);	break;
			case ACTOR_WEAP_MACHINEGUN:		VectorSet (self->muzzle, 38, 4, 19);	break;
			case ACTOR_WEAP_CHAINGUN:		VectorSet (self->muzzle, 45, 4.5, 15);	break;
			case ACTOR_WEAP_GLAUNCHER:		VectorSet (self->muzzle, 32, 5, 15);	break;
			case ACTOR_WEAP_RLAUCHER:		VectorSet (self->muzzle, 40, 5, 15);	break;
			case ACTOR_WEAP_HYPERBLASTER:	VectorSet (self->muzzle, 41, 4, 19);	break;
			case ACTOR_WEAP_RAILGUN:		VectorSet (self->muzzle, 40, 4, 19);	break;
			case ACTOR_WEAP_BFG:			VectorSet (self->muzzle, 42, 5, 20);	break;
			default:						VectorSet (self->muzzle, 40, 4, 19);	break;
			}
			break;
		case ACTOR_PARANOID:
			switch (self->actor_weapon[0])
			{
			case ACTOR_WEAP_BLASTER:		VectorSet (self->muzzle, 18, 7, 10);	break;
			case ACTOR_WEAP_SHOTGUN:		VectorSet (self->muzzle, 22, 7, 10);	break;
			case ACTOR_WEAP_SSHOTGUN:		VectorSet (self->muzzle, 22, 7, 10);	break;
			case ACTOR_WEAP_MACHINEGUN:		VectorSet (self->muzzle, 18, 7, 12);	break;
			case ACTOR_WEAP_CHAINGUN:		VectorSet (self->muzzle, 26, 7, 16);	break;
			case ACTOR_WEAP_GLAUNCHER:		VectorSet (self->muzzle, 24, 7, 10);	break;
			case ACTOR_WEAP_RLAUCHER:		VectorSet (self->muzzle, 26, 7, 10);	break;
			case ACTOR_WEAP_HYPERBLASTER:	VectorSet (self->muzzle, 18, 7, 14);	break;
			case ACTOR_WEAP_RAILGUN:		VectorSet (self->muzzle, 28, 7, 10);	break;
			case ACTOR_WEAP_BFG:			VectorSet (self->muzzle, 28, 7, 10);	break;
			default:						VectorSet (self->muzzle, 28, 7, 10);	break;
			}
			break;
		case ACTOR_RATAMAHATTA:
			VectorSet (self->muzzle, 24, 13, 10);
			break;
		case ACTOR_RHINO:
			VectorSet (self->muzzle, 29, 7, 10);
			break;
		case ACTOR_SAS:
			VectorSet (self->muzzle, 17, 6.5, 17);
			break;
		case ACTOR_SLITH:
			switch (self->actor_weapon[0])
			{
			case ACTOR_WEAP_BLASTER:		VectorSet (self->muzzle, 32, 7, 10);	break;
			case ACTOR_WEAP_SHOTGUN:		VectorSet (self->muzzle, 32, 7, 10);	break;
			case ACTOR_WEAP_SSHOTGUN:		VectorSet (self->muzzle, 32, 7, 10);	break;
			case ACTOR_WEAP_MACHINEGUN:		VectorSet (self->muzzle, 25, 5, -1);	break;
			case ACTOR_WEAP_CHAINGUN:		VectorSet (self->muzzle, 25, 5, -1);	break;
			case ACTOR_WEAP_GLAUNCHER:		VectorSet (self->muzzle, 32, 7, 10);	break;
			case ACTOR_WEAP_RLAUCHER:		VectorSet (self->muzzle, 32, 7, 10);	break;
			case ACTOR_WEAP_HYPERBLASTER:	VectorSet (self->muzzle, 12, 6, -1);	break;
			case ACTOR_WEAP_RAILGUN:		VectorSet (self->muzzle, 32, 7, 10);	break;
			case ACTOR_WEAP_BFG:			VectorSet (self->muzzle, 20, 5, -1);	break;
			default:						VectorSet (self->muzzle, 32, 7, 10);	break;
			}
			break;
		case ACTOR_TERRAN:
			VectorSet (self->muzzle, 42, 7, 11.5);
			break;
		case ACTOR_WALKER:
			VectorSet (self->muzzle, 9, 16, 7);
			break;
		case ACTOR_WASTE:
			switch (self->actor_weapon[0])
			{
			case ACTOR_WEAP_BLASTER:		VectorSet (self->muzzle, 12, 9, 9);		break;
			case ACTOR_WEAP_SHOTGUN:		VectorSet (self->muzzle, 22, 9, 9);		break;
			case ACTOR_WEAP_SSHOTGUN:		VectorSet (self->muzzle, 20, 9, 9);		break;
			case ACTOR_WEAP_MACHINEGUN:		VectorSet (self->muzzle, 11, 11, 7);	break;
			case ACTOR_WEAP_CHAINGUN:		VectorSet (self->muzzle, 26, 8, 8);		break;
			case ACTOR_WEAP_GLAUNCHER:		VectorSet (self->muzzle, 18, 9, 7);		break;
			case ACTOR_WEAP_RLAUCHER:		VectorSet (self->muzzle, 26, 9, 7);		break;
			case ACTOR_WEAP_HYPERBLASTER:	VectorSet (self->muzzle, 26, 7.5, 8);	break;
			case ACTOR_WEAP_RAILGUN:		VectorSet (self->muzzle, 26, 9, 7);		break;
			case ACTOR_WEAP_BFG:			VectorSet (self->muzzle, 22, 11, 7);	break;
			default:						VectorSet (self->muzzle, 26, 9, 7);		break;
			}
			break;
		case ACTOR_XENOID:
			VectorSet (self->muzzle, 20, 12, 7);
			break;
		case ACTOR_ZUMLIN:
			switch (self->actor_weapon[0])
			{
			case ACTOR_WEAP_BLASTER:		VectorSet (self->muzzle, 22, 3, 8);		break;
			case ACTOR_WEAP_SHOTGUN:		VectorSet (self->muzzle, 20, 2, 9);		break;
			case ACTOR_WEAP_SSHOTGUN:		VectorSet (self->muzzle, 20, 2, 9);		break;
			case ACTOR_WEAP_MACHINEGUN:		VectorSet (self->muzzle, 8, 5, 4);		break;
			case ACTOR_WEAP_CHAINGUN:		VectorSet (self->muzzle, 22, 2, 4);		break;
			case ACTOR_WEAP_GLAUNCHER:		VectorSet (self->muzzle, 20, 2, 7);		break;
			case ACTOR_WEAP_RLAUCHER:		VectorSet (self->muzzle, 30, 2, 9);		break;
			case ACTOR_WEAP_HYPERBLASTER:	VectorSet (self->muzzle, 20, 3, 2);		break;
			case ACTOR_WEAP_RAILGUN:		VectorSet (self->muzzle, 26, 2, 9);		break;
			case ACTOR_WEAP_BFG:			VectorSet (self->muzzle, 16, 5, -2);	break;
			default:						VectorSet (self->muzzle, 26, 2, 9);		break;
			}
			break;
		default:
			switch (self->actor_weapon[0])
			{
			case ACTOR_WEAP_MACHINEGUN:		VectorSet (self->muzzle, 6, 9, 6);			break;
			case ACTOR_WEAP_CHAINGUN:		VectorSet (self->muzzle, 20, 9, 8);			break;
			case ACTOR_WEAP_HYPERBLASTER:	VectorSet (self->muzzle, 18, 8, 6);			break;
			default:						VectorSet (self->muzzle, 18.4, 7.4, 9.6);	break;
			}
		}
	}

	if ( !VectorLength(self->muzzle2) )
	{
		switch (ActorID)
		{
		case ACTOR_RHINO:
			VectorSet (self->muzzle2, 27, -15, 13);
			break;
		case ACTOR_WALKER:
			VectorSet (self->muzzle2, 9, -11, 7);
			break;
		}
	}
	if ( VectorLength(self->muzzle2) )
		self->monsterinfo.aiflags |= AI_TWO_GUNS;

	self->pain = actor_pain;
	self->die = actor_die;

	self->monsterinfo.stand = actor_stand;
	self->monsterinfo.walk = actor_walk;
	self->monsterinfo.run = actor_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = actor_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;
	self->monsterinfo.idle = NULL;
	self->monsterinfo.checkattack = actor_checkattack;
	if (actorjump->value)
	{
		self->monsterinfo.jump = actor_jump;
		self->monsterinfo.jumpup = 48;
		self->monsterinfo.jumpdn = 160;
	}
//	self->monsterinfo.blocked = actor_blocked;

	// There are several actions (mainly following a player leader) that
	// are only applicable to misc_actor (not other monsters)
	self->monsterinfo.aiflags |= AI_ACTOR;
	if ( !(self->spawnflags & SF_ACTOR_BAD_GUY) || (self->spawnflags & SF_MONSTER_GOODGUY) )
		self->monsterinfo.aiflags |= AI_GOOD_GUY;

	if (self->powerarmor)
	{
		if (self->powerarmortype == 1)
			self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
		else
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		self->monsterinfo.power_armor_power = self->powerarmor;
	}

	// Minimum distance
	if (self->actor_weapon[1])
		self->monsterinfo.min_range = 0;
	else
	{
		weapon = self->actor_weapon[0];
		if ( (weapon == ACTOR_WEAP_GLAUNCHER) || (weapon == ACTOR_WEAP_RLAUCHER)
			|| (weapon == ACTOR_WEAP_BFG) || (weapon == ACTOR_WEAP_PHALANX) ) // Knightmare- added phalanx support
			self->monsterinfo.min_range = 200;
		else if (weapon == ACTOR_WEAP_ETFRIFLE)		// Knightamre- min range for ETF rifle
			self->monsterinfo.min_range = 100;
		else
			self->monsterinfo.min_range = 0;
	}
	// Ideal range
	actor_ideal_range(self);

	gi.linkentity (self);

	self->monsterinfo.currentmove = &actor_move_stand;
	if (self->health < 0)
	{
		mmove_t	*deathmoves[] = {&actor_move_death1,
			                     &actor_move_death2,
								 NULL};
		M_SetDeath (self,(mmove_t **)&deathmoves);
	}
	self->monsterinfo.scale = 0.8;
	walkmonster_start (self);

	// We've built the misc_actor model to include the standard
	// Q2 male skins, specified with the style key. Default = grunt
	self->s.skinnum = self->style;

	// actors always start in a dormant state, they *must* be used to get going
	self->use = actor_use;

	// If health > 100000, actor is invulnerable
	if (self->health >= 100000)
		self->takedamage = DAMAGE_NO;

	self->common_name = "Actor";

	// Muzzle flash
	self->flash = G_Spawn();
	self->flash->classname = "muzzleflash";
	self->flash->model = "models/objects/flash/tris.md2";
	gi.setmodel(self->flash,self->flash->model);
	self->flash->solid = SOLID_NOT;
	self->flash->s.skinnum = 0;
	self->flash->s.effects = EF_PLASMA;
	self->flash->s.renderfx = RF_FULLBRIGHT;
	self->flash->svflags |= SVF_NOCLIENT;
	VectorCopy (self->s.origin, self->flash->s.origin);
	gi.linkentity(self->flash);
}


/*QUAKED target_actor (.5 .3 0) (-8 -8 -8) (8 8 8) JUMP SHOOT ATTACK x HOLD BRUTAL
JUMP			jump in set direction upon reaching this target
SHOOT			take a single shot at the pathtarget
ATTACK		    attack pathtarget until it or actor is dead 

"target"		next target_actor
"pathtarget"	target of any action to be taken at this point
"wait"		amount of time actor should pause at this point

for JUMP only:
"speed"			speed thrown forward (default 200)
"height"		speed thrown upwards (default 200)
*/

void target_actor_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	v;

	if (other->movetarget != self)
		return;
	
	if (other->enemy)
		return;

	other->goalentity = other->movetarget = NULL;

	if (self->spawnflags & 1)		// jump
	{
		other->velocity[0] = self->movedir[0] * self->speed;
		other->velocity[1] = self->movedir[1] * self->speed;
		
		if (other->groundentity)
		{
			other->groundentity = NULL;
			other->velocity[2] = self->movedir[2];
			if (other->monsterinfo.aiflags & AI_ACTOR)
				gi.sound (self, CHAN_VOICE, other->actor_sound_index[ACTOR_SOUND_JUMP], 1, ATTN_NORM, 0);
		}
		// NOTE: The jump animation won't work UNLESS this target_actor has a target. If this
		// is the last target_actor in a sequence, the actor's run takes over and prevents
		// the jump.
	//	if (!Q_stricmp(other->classname,"misc_actor"))
	//		other->monsterinfo.currentmove = &actor_move_jump;
	}

	if (self->spawnflags & 2)	// shoot
	{
		if (self->pathtarget)
		{
			if ( G_Find(NULL,FOFS(targetname),self->pathtarget) != NULL )
			{
				other->enemy = G_PickTarget(self->pathtarget);
				if (self->spawnflags & 8) {
					other->monsterinfo.aiflags |= AI_STAND_GROUND;
					actor_stand (other);
				}
				else
					actor_attack(other);
			} 
			else
				other->enemy = NULL;
		}
		else
		{
			other->enemy = NULL;
		}
	}
	else if (self->spawnflags & 4)	// attack
	{
		if (self->pathtarget)
		{
			if ( G_Find(NULL,FOFS(targetname),self->pathtarget) != NULL )
				other->enemy = G_PickTarget(self->pathtarget);
			else
				other->enemy = NULL;
		}
		else {
			other->enemy = NULL;
		}

		if (other->enemy)
		{
			other->goalentity = other->enemy;
			if (self->spawnflags & 32)
				other->monsterinfo.aiflags |= AI_BRUTAL;
			if (self->spawnflags & 16) {
				other->monsterinfo.aiflags |= AI_STAND_GROUND;
				actor_stand (other);
			}
			else {
				actor_run (other);
			}
		}
	}

	if (!(self->spawnflags & 6) && (self->pathtarget))
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		G_UseTargets (self, other);
		self->target = savetarget;
	}

	// DWH: Allow blank target field
	if (self->target)
		other->movetarget = G_PickTarget(self->target);
	else
		other->movetarget = NULL;

	if (!other->goalentity)
		other->goalentity = other->movetarget;

	if (self->wait)
	{
		other->monsterinfo.pausetime = level.time + self->wait;
		other->monsterinfo.stand (other);
	}
	else
	{
		
		if (!other->movetarget && !other->enemy)
		{
			other->monsterinfo.pausetime = level.time + 100000000;
			other->monsterinfo.stand (other);
		}
		else if (other->movetarget == other->goalentity)
		{
			// DWH: Bug fix here... possible to get here with NULL movetarget and goalentity
			if (other->movetarget)
			{
				VectorSubtract (other->movetarget->s.origin, other->s.origin, v);
				other->ideal_yaw = vectoyaw (v);
			}
		}
	}

	self->count--;
	if (!self->count)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 1;
	}

}

void SP_target_actor (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict(self);
		return;
	}
	self->class_id = ENTITY_TARGET_ACTOR;

	if (!self->targetname)
		gi.dprintf ("%s with no targetname at %s\n", self->classname, vtos(self->s.origin));

	self->solid = SOLID_TRIGGER;
	self->touch = target_actor_touch;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	self->svflags = SVF_NOCLIENT;

	if (self->spawnflags & 1)
	{
		if (!self->speed)
			self->speed = 200;

		if (self->height)
			st.height = self->height;
		if (!st.height)
			st.height = 200;
		if (self->s.angles[YAW] == 0)
			self->s.angles[YAW] = 360;
		G_SetMovedir (self->s.angles, self->movedir);
		self->movedir[2] = st.height;
	}
	gi.linkentity (self);
}


qboolean InPak (char *basedir, char *gamedir, char *filename)
{
	char			pakfile[256];
	int				k, kk;
	int				num, numitems;
	FILE			*f;
	pak_header_t	pakheader;
	pak_item_t		pakitem;
	qboolean		found = false;
	
#ifdef KMQUAKE2_ENGINE_MOD // *.pak/pk3 support
	char	*file_data;
	int		file_size = 0;

	file_size = gi.LoadFile(filename, (void **)&file_data);

	if (file_data)
		gi.FreeFile(file_data);

	if (file_size > 2)
		return true;
#endif

	// Search paks in game folder
	for (k=9; k>=0 && !found; k--)
	{
		strncpy(pakfile, basedir, sizeof(pakfile));
		if (strlen(gamedir))
		{
			Com_strcat (pakfile, sizeof(pakfile), "/");
			Com_strcat (pakfile, sizeof(pakfile), gamedir);
		}
		Com_strcat (pakfile, sizeof(pakfile), va("/pak%d.pak", k));
		if (NULL != (f = fopen(pakfile, "rb")))
		{
			num = (int)fread(&pakheader, 1, sizeof(pak_header_t), f);
			if (num >= sizeof(pak_header_t))
			{
				if ( pakheader.id[0] == 'P' &&
					pakheader.id[1] == 'A' &&
					pakheader.id[2] == 'C' &&
					pakheader.id[3] == 'K'     )
				{
					numitems = pakheader.dsize/sizeof(pak_item_t);
					fseek(f, pakheader.dstart, SEEK_SET);
					for (kk=0; kk<numitems && !found; kk++)
					{
						fread(&pakitem, 1, sizeof(pak_item_t), f);
						if (!Q_stricmp(pakitem.name, filename))
							found = true;
					}
				}
			}
			fclose(f);
		}
	}
	return found;
}

typedef struct
{
	int		index;
} actorlist;

void actor_files (void)
{
	char				path[256];
	char				filename[256];
	int					s_match, w_match[2];
	int					i, j, k;
	int					num_actors = 0;
	static actorlist	actors[MAX_EDICTS];	// Knightmare- made static due to stack size
	cvar_t				*basedir, *cddir, *gamedir;
	edict_t				*e, *e0;
	FILE				*f;

	if (deathmatch->value)
		return;

	basedir = gi.cvar("basedir", "", 0);
	cddir   = gi.cvar("cddir",   "", 0);
	gamedir = gi.cvar("gamedir", "", 0);

	memset (&actors, 0, MAX_EDICTS*sizeof(actorlist));

	for (i=game.maxclients+1; i<globals.num_edicts; i++)
	{
		e = &g_edicts[i];
		if (!e->inuse) continue;
		if (!e->classname) continue;
		if (!(e->monsterinfo.aiflags & AI_ACTOR)) continue;

		for (j=0; j<NUM_ACTOR_SOUNDS; j++)
			e->actor_sound_index[j] = 0;

		s_match    = 0;
		w_match[0] = 0;
		w_match[1] = 0;
		if (num_actors > 0)
		{
			for (j=0; j<num_actors && (s_match == 0 || w_match[0] == 0 || w_match[1] == 0); j++)
			{
				e0 = &g_edicts[actors[j].index];
				if (!Q_stricmp(e->usermodel, e0->usermodel))
				{
					s_match = j+1;
					if (e->actor_weapon[0] == e0->actor_weapon[0])
						w_match[0] = j*2+1;
					else if (e->actor_weapon[0] == e0->actor_weapon[1])
						w_match[0] = j*2+2;
					if (e->actor_weapon[1] == e0->actor_weapon[0])
						w_match[1] = j*2+1;
					else if (e->actor_weapon[1] == e0->actor_weapon[1])
						w_match[1] = j*2+2;
				}
			}
			if (s_match)
			{
				// copy sound indices from previous actor
				e0 = &g_edicts[actors[s_match-1].index];
				for (j=0; j<NUM_ACTOR_SOUNDS; j++)
					e->actor_sound_index[j] = e0->actor_sound_index[j];
			}
			if (w_match[0])
			{
				k = (w_match[0]-1) % 2;
				e0 = &g_edicts[actors[ (w_match[0]-k-1)/2 ].index];
				e->s.modelindex2 = e->actor_model_index[0] = e0->actor_model_index[k];
			}
			if (w_match[1])
			{
				k = (w_match[1]-1) % 2;
				e0 = &g_edicts[actors[ (w_match[1]-k-1)/2 ].index];
				e->actor_model_index[1] = e0->actor_model_index[k];
			}
		}
		if (!s_match)
		{
			// search for sounds on hard disk and in paks
			actors[num_actors].index  = i;
			num_actors++;
			
			if (!Q_stricmp(e->usermodel, "male") || !Q_stricmp(e->usermodel, "female")) {
				Com_sprintf(path, sizeof(path), "player/%s/", e->usermodel);
			}
			else {
				Com_sprintf(path, sizeof(path), "../players/%s/", e->usermodel);
			}
			
			for (j=0; j<NUM_ACTOR_SOUNDS; j++)
			{
				if (e->actor_sound_index[j])
					continue;

				// If it's NOT a custom model, start by looking in game folder
				if (strlen(gamedir->string))
				{
					Com_sprintf(filename, sizeof(filename), "%s/%s/sound/%s%s", basedir->string, gamedir->string, path, wavname[j]);
					f = fopen(filename, "r");
					if (f) {
						fclose(f);
						Com_strcpy (filename, sizeof(filename), path);
						Com_strcat (filename, sizeof(filename), wavname[j]);
						e->actor_sound_index[j] = gi.soundindex(filename);
						continue;
					}
					// Search paks in game folder
					Com_sprintf(filename, sizeof(filename), "sound/%s%s", path, wavname[j]);
					if ( InPak(basedir->string, gamedir->string, filename) ) {
						Com_strcpy (filename, sizeof(filename), path);
						Com_strcat (filename, sizeof(filename), wavname[j]);
						e->actor_sound_index[j] = gi.soundindex(filename);
						continue;
					}
				}
				
				// Search in baseq2 for external file
				Com_sprintf(filename, sizeof(filename), "%s/baseq2/sound/%s%s", basedir->string, path, wavname[j]);
				f = fopen(filename, "r");
				if (f) {
					fclose(f);
					Com_strcpy (filename, sizeof(filename), path);
					Com_strcat (filename, sizeof(filename), wavname[j]);
					e->actor_sound_index[j] = gi.soundindex(filename);
					continue;
				}
				// Search paks in baseq2
				Com_sprintf(filename, sizeof(filename), "sound/%s%s", path, wavname[j]);
				if ( InPak(basedir->string, "baseq2", filename) ) {
					Com_strcpy (filename, sizeof(filename), path);
					Com_strcat (filename, sizeof(filename), wavname[j]);
					e->actor_sound_index[j] = gi.soundindex(filename);
					continue;
				}
				if (strlen(cddir->string))
				{
					// Search in cddir (minimal installation)
					Com_sprintf(filename, sizeof(filename), "%s/baseq2/sound/%s%s", cddir->string, path, wavname[j]);
					f = fopen(filename, "r");
					if (f) {
						fclose(f);
						Com_strcpy (filename, sizeof(filename), path);
						Com_strcat (filename, sizeof(filename), wavname[j]);
						e->actor_sound_index[j] = gi.soundindex(filename);
						continue;
					}
					// Search paks in baseq2
					Com_sprintf(filename, sizeof(filename), "sound/%s%s", path, wavname[j]);
					if ( InPak(cddir->string, "baseq2", filename) ) {
						Com_strcpy (filename, sizeof(filename), path);
						Com_strcat (filename, sizeof(filename), wavname[j]);
						e->actor_sound_index[j] = gi.soundindex(filename);
						continue;
					}
				}
				// If sound is STILL not found, use normal male sounds
				Com_sprintf(filename, sizeof(filename), "player/male/%s", wavname[j]);
				e->actor_sound_index[j] = gi.soundindex(filename);
			}
		}

		// repeat this WHOLE DAMN THING for weapons

		for (k=0; k<2; k++)
		{
			if (w_match[k]) continue;
			if (!e->actor_weapon[k]) continue;
			if ((k==1) && (e->actor_weapon[0] == e->actor_weapon[1]))
			{
				e->actor_model_index[1] = e->actor_model_index[0];
				continue;
			}
			if (s_match)
			{
				// Wasn't added to table on account of sounds
				if (k==0 || w_match[0] > 0)
				{
					// Either this is weapon 0, or weapon 0 was a match. Either
					// way, this guy has something unique and hasn't been added
					// to the table
					actors[num_actors].index  = i;
					num_actors++;
				}
			}

			Com_sprintf (filename, sizeof(filename), "players/%s/", e->usermodel);
			switch (e->actor_weapon[k])
			{
			case ACTOR_WEAP_SHOTGUN:		Com_strcat (filename, sizeof(filename), "w_shotgun.md2");		break;
			case ACTOR_WEAP_SSHOTGUN:		Com_strcat (filename, sizeof(filename), "w_sshotgun.md2");		break;
			case ACTOR_WEAP_MACHINEGUN:		Com_strcat (filename, sizeof(filename), "w_machinegun.md2");	break;
			case ACTOR_WEAP_CHAINGUN:		Com_strcat (filename, sizeof(filename), "w_chaingun.md2");		break;
			case ACTOR_WEAP_GLAUNCHER:		Com_strcat (filename, sizeof(filename), "w_glauncher.md2");		break;
			case ACTOR_WEAP_RLAUCHER:		Com_strcat (filename, sizeof(filename), "w_rlauncher.md2");		break;
			case ACTOR_WEAP_HYPERBLASTER:	Com_strcat (filename, sizeof(filename), "w_hyperblaster.md2");	break;
			case ACTOR_WEAP_RAILGUN:		Com_strcat (filename, sizeof(filename), "w_railgun.md2");		break;
			case ACTOR_WEAP_BFG:			Com_strcat (filename, sizeof(filename), "w_bfg.md2");			break;
			// Knightmare- mission pack weapon support
			case ACTOR_WEAP_IONRIPPER:		Com_strcat (filename, sizeof(filename), "w_ripper.md2");		break;
			case ACTOR_WEAP_PHALANX:		Com_strcat (filename, sizeof(filename), "w_phalanx.md2");		break;
			case ACTOR_WEAP_ETFRIFLE:		Com_strcat (filename, sizeof(filename), "w_etfrifle.md2");		break;
			case ACTOR_WEAP_PLASMABEAM:		Com_strcat (filename, sizeof(filename), "w_plasma.md2");		break;
			case ACTOR_WEAP_DISRUPTOR:		Com_strcat (filename, sizeof(filename), "w_disrupt.md2");		break;
			// LMSP plasma rifle
			case ACTOR_WEAP_PLASMARIFLE:	Com_strcat (filename, sizeof(filename), "w_plasmarifle.md2");	break;
			// end Knightmare
			default:						Com_strcat (filename, sizeof(filename), "w_blaster.md2");		break;
			}

			if (strlen(gamedir->string))
			{	// Start in game folder
				Com_sprintf(path, sizeof(path), "%s/%s/%s", basedir->string, gamedir->string, filename);
				f = fopen(path, "r");
				if (f) {
					fclose(f);
					e->actor_model_index[k] = gi.modelindex(filename);
					continue;
				}
				// Search pak files in gamedir
				if ( InPak(basedir->string, gamedir->string, filename) ) {
					e->actor_model_index[k] = gi.modelindex(filename);
					continue;
				}
			}
			// Search in baseq2 for external file
			Com_sprintf(path, sizeof(path), "%s/baseq2/%s", basedir->string, filename);
			f = fopen(path, "r");
			if (f) {
				fclose(f);
				e->actor_model_index[k] = gi.modelindex(filename);
				continue;
			}
			// Search paks in baseq2
			if ( InPak(basedir->string, "baseq2", filename) ) {
				e->actor_model_index[k] = gi.modelindex(filename);
				continue;
			}

			if (strlen(cddir->string))
			{
				// Search CD for minimal installations
				Com_sprintf(path, sizeof(path), "%s/baseq2/%s", cddir->string, filename);
				f = fopen(path, "r");
				if (f) {
					fclose(f);
					e->actor_model_index[k] = gi.modelindex(filename);
					continue;
				}
				
				// Search paks in baseq2
				if ( InPak(cddir->string, "baseq2", filename) ) {
					e->actor_model_index[k] = gi.modelindex(filename);
					continue;
				}
			}

			// If sound is STILL not found, start the fuck over and look for weapon.md2
			Com_sprintf(filename, sizeof(filename), "players/%s/weapon.md2", e->usermodel);

			if (strlen(gamedir->string))
			{
				// Start in game folder
				Com_sprintf(path, sizeof(path), "%s/%s/%s", basedir->string, gamedir->string, filename);
				f = fopen(path, "r");
				if (f) {
					fclose(f);
					e->actor_model_index[k] = gi.modelindex(filename);
					continue;
				}
				// Search pak files in gamedir
				if ( InPak(basedir->string, gamedir->string, filename) ) {
					e->actor_model_index[k] = gi.modelindex(filename);
					continue;
				}
			}	
			// Search in baseq2 for external file
			Com_sprintf (path, sizeof(path), "%s/baseq2/%s", basedir->string, filename);
			f = fopen(path, "r");
			if (f) {
				fclose(f);
				e->actor_model_index[k] = gi.modelindex(filename);
				continue;
			}
			// Search paks in baseq2
			if ( InPak(basedir->string, "baseq2", filename) ) {
				e->actor_model_index[k] = gi.modelindex(filename);
				continue;
			}

			if (strlen(cddir->string))
			{
				// Search CD for minimal installations
				Com_sprintf(path, sizeof(path), "%s/baseq2/%s", cddir->string, filename);
				f = fopen(path, "r");
				if (f) {
					fclose(f);
					e->actor_model_index[k] = gi.modelindex(filename);
					continue;
				}
				// Search paks in baseq2
				if ( InPak(cddir->string, "baseq2", filename) ) {
					e->actor_model_index[k] = gi.modelindex(filename);
					continue;
				}
			}
			// And if it's STILL not found, use 
			Com_sprintf(filename, sizeof(filename), "players/male/weapon.md2");
			e->actor_model_index[k] = gi.modelindex(filename);
		}
		if (e->health > 0)
			e->s.modelindex2 = e->actor_model_index[e->actor_current_weapon];
		else
			e->s.modelindex2 = 0;
		gi.linkentity(e);
	}
}

void actor_moveit (edict_t *player, edict_t *actor)
{
	edict_t	*thing;
	trace_t	tr;
	vec3_t	dir, end;
	vec_t	d[3];
	vec_t	temp;
	vec_t	travel;
	int		best=0;

	if (!(actor->monsterinfo.aiflags & AI_FOLLOW_LEADER))
		return;
	if (actor->enemy)
		return;
	if (actor->health <= 0)
		return;
	travel = 256 + 128 * crandom();
	thing  = actor->vehicle;
	if ( !thing || !thing->inuse || Q_stricmp(thing->classname, "thing") )
		thing = actor->vehicle = SpawnThing();
	VectorSubtract (actor->s.origin, player->s.origin, dir);
	dir[2] = 0;
	VectorNormalize (dir);
	if ( !VectorLength(dir) )
		VectorSet (dir, 1.0f, 0.0f, 0.0f);
	VectorMA (actor->s.origin, travel, dir, end);
	tr = gi.trace(actor->s.origin, NULL, NULL, end, actor, MASK_MONSTERSOLID);
	d[best] = tr.fraction * travel;
	if (d[best] < 64)
	{
		temp   = dir[0];
		dir[0] = -dir[1];
		dir[1] = temp;
		VectorMA (actor->s.origin, travel, dir, end);
		tr = gi.trace(actor->s.origin, NULL, NULL, end, actor, MASK_MONSTERSOLID);
		best = 1;
		d[best] = tr.fraction * travel;
		if (d[best] < 64)
		{
			dir[0] = -dir[0];
			dir[1] = -dir[1];
			VectorMA (actor->s.origin, travel, dir, end);
			tr = gi.trace(actor->s.origin, NULL, NULL, end, actor, MASK_MONSTERSOLID);
			best = 2;
			d[best] = tr.fraction * travel;
			if (d[best] < 64)
			{
				if (d[0] > d[1] && d[0] > d[2])
					best = 0;
				else if (d[1] > d[0] && d[1] > d[2])
					best = 1;
				if (best == 1)
				{
					dir[0] = -dir[0];
					dir[1] = -dir[1];
				}
				else if (best == 0)
				{
					temp   = -dir[1];
					dir[1] = dir[0];
					dir[0] = temp;
				}
			}
		}
	}
	VectorCopy (tr.endpos, thing->s.origin);
	thing->touch_debounce_time = level.time + max(5.0f, d[best]/50.0f);
	thing->target_ent = actor;
	ED_CallSpawn (thing);
	actor->monsterinfo.aiflags |= AI_CHASE_THING;
	actor->movetarget = actor->goalentity = thing;
	actor->monsterinfo.old_leader = player;
	actor->monsterinfo.leader = thing;
	VectorSubtract (thing->s.origin, actor->s.origin, dir);
	actor->ideal_yaw = vectoyaw(dir);
	actor->monsterinfo.run (actor);
}
