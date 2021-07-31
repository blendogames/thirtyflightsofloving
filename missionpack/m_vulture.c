/*
==============================================================================

vulture
Based on model by Andrew Eglington and sounds by Magnus Jansén
Reimplemented by Knightmare

==============================================================================
*/

#include "g_local.h"
#include "m_vulture.h"

#define SF_VULTURE_IN_AIR	8

qboolean visible (edict_t *self, edict_t *other);

static int	nextmove;			// Used for start/stop frames

//static int	sound_sight;
static int	sound_perch_idle1;
static int	sound_perch_idle2;
static int	sound_perch_idle3;
static int	sound_soar_idle1;
static int	sound_soar_idle2;
static int	sound_soar_idle3;
static int	sound_flap1;
static int	sound_flap2;
static int	sound_flap3;
static int	sound_peck1;
static int	sound_peck2;
static int	sound_peck3;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_death;

void vulture_run (edict_t *self);

void vulture_sight (edict_t *self, edict_t *other)
{
//	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void vulture_perch_idle (edict_t *self)
{
	float r;

	if (random() < 0.3f)
	{
		r = random();
		if (r < 0.33f)
			gi.sound (self, CHAN_VOICE, sound_perch_idle1, 1, ATTN_IDLE, 0);
		else if (r < 0.67f)
			gi.sound (self, CHAN_VOICE, sound_perch_idle2, 1, ATTN_IDLE, 0);
		else
			gi.sound (self, CHAN_VOICE, sound_perch_idle3, 1, ATTN_IDLE, 0);
	}
}

void vulture_soar_idle (edict_t *self)
{
	float r;

	if (random() < 0.3f)
	{
		r = random();
		if (r < 0.33f)
			gi.sound (self, CHAN_VOICE, sound_soar_idle1, 1, ATTN_IDLE, 0);
		else if (r < 0.67f)
			gi.sound (self, CHAN_VOICE, sound_soar_idle2, 1, ATTN_IDLE, 0);
		else
			gi.sound (self, CHAN_VOICE, sound_soar_idle3, 1, ATTN_IDLE, 0);
	}
}

void vulture_idle (edict_t *self)
{
	if (self->flags & FL_FLY)
		vulture_soar_idle (self);
	else
		vulture_perch_idle (self);
}


void vulture_flap (edict_t *self)
{
	float r;

	if (random() < 0.67f)
	{
		r = random();
		if (r < 0.33f)
			gi.sound (self, CHAN_VOICE, sound_flap1, 1, ATTN_IDLE, 0);
		else if (r < 0.67f)
			gi.sound (self, CHAN_VOICE, sound_flap2, 1, ATTN_IDLE, 0);
		else
			gi.sound (self, CHAN_VOICE, sound_flap3, 1, ATTN_IDLE, 0);
	}
}

//
// perch
//

mframe_t vulture_frames_perch [] =
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
	ai_stand, 0, NULL
};
mmove_t	vulture_move_perch = {FRAME_perch1, FRAME_perch30, vulture_frames_perch, NULL};

void vulture_perch (edict_t *self)
{
	self->monsterinfo.currentmove = &vulture_move_perch;
}

//
// walk
//

mframe_t vulture_frames_walk [] =
{
	ai_walk, 1, NULL,
	ai_walk, 1, NULL,
	ai_walk, 1, NULL,
	ai_walk, 1, NULL,
	ai_walk, 1, NULL,
	ai_walk, 1, NULL,
	ai_walk, 1, NULL,
	ai_walk, 1, NULL,
	ai_walk, 1, NULL,
	ai_walk, 1, NULL
};
mmove_t	vulture_move_walk = {FRAME_walk1, FRAME_walk10, vulture_frames_walk, NULL};

//
// soar
//

mframe_t vulture_frames_soar [] =
{
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,

	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,

	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,

	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL
};
mmove_t	vulture_move_soar = {FRAME_soar1, FRAME_soar42, vulture_frames_soar, NULL};
/*
void vulture_soar (edict_t *self)
{
	self->monsterinfo.currentmove = &vulture_move_soar;
}
*/

#if 0
//
// bank right
//

mframe_t vulture_frames_bankright [] =
{
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL
};
//mmove_t	vulture_move_bankright = {FRAME_bankR1, FRAME_bankR10, vulture_frames_bankright, NULL};

void vulture_bankright (edict_t *self)
{
	self->monsterinfo.currentmove = &vulture_move_bankright;
}

//
// bank left
//

mframe_t vulture_frames_bankleft [] =
{
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL,
	ai_move, 10, NULL
};
//mmove_t	vulture_move_bankleft = {FRAME_bankL1, FRAME_bankL10, vulture_frames_bankleft, NULL};

void vulture_bankleft (edict_t *self)
{
	self->monsterinfo.currentmove = &vulture_move_bankleft;
}
#endif

void vulture_walk (edict_t *self)
{
	if (self->flags & FL_FLY) {
		self->monsterinfo.currentmove = &vulture_move_soar;
	}
	else {
		self->monsterinfo.currentmove = &vulture_move_walk;
	}
}

//
// fly
//

mframe_t vulture_frames_fly [] =
{
	ai_run, 10, NULL,
	ai_run, 10, vulture_flap,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL
};
mmove_t	vulture_move_fly = {FRAME_fly1, FRAME_fly5, vulture_frames_fly, NULL};
/*
void vulture_fly (edict_t *self)
{
	self->monsterinfo.currentmove = &vulture_move_fly;
}
*/

//
// takeoff
//

void vulture_airborne (edict_t *self)
{
	self->flags |= FL_FLY;

//	if (self->flags & FL_FLY)
//		gi.dprintf ("successfully converted monster_vulture to flying\n");
}

mframe_t vulture_frames_takeoff [] =
{
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, vulture_airborne
};
mmove_t	vulture_move_takeoff = {FRAME_takeoff1, FRAME_takeoff6, vulture_frames_takeoff, vulture_run};
/*
void vulture_takeoff (edict_t *self)
{
	self->monsterinfo.currentmove = &vulture_move_takeoff;
}
*/

//
// land
//

void vulture_landed (edict_t *self)
{
	self->flags &= ~FL_FLY;

//	if ( !(self->flags & FL_FLY) )
//		gi.dprintf ("successfully converted monster_vulture to ground\n");
}
/*
mframe_t vulture_frames_land [] =
{
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, NULL,
	ai_move, 5, vulture_landed
};
mmove_t	vulture_move_land = {FRAME_land1, FRAME_land13, vulture_frames_land, vulture_walk};

void vulture_land (edict_t *self)
{
	self->monsterinfo.currentmove = &vulture_move_land;
}
*/

void vulture_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		if (self->flags & FL_FLY) {
			self->monsterinfo.currentmove = &vulture_move_soar;
		}
		else {
			self->monsterinfo.currentmove = &vulture_move_perch;
		}
	}
	else
	{
		if (self->flags & FL_FLY) {
			self->monsterinfo.currentmove = &vulture_move_fly;
		}
		else {
			// takeoff if we're on ground
			self->monsterinfo.currentmove = &vulture_move_takeoff;
		}
	}
}

//
// melee
//

void vulture_peck (edict_t *self)
{
	vec3_t	aim;
	float	r;
	
	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 0);
	if ( fire_hit (self, aim, (int)(10 + (random() * 5)), 0) )
	{
		r = random();
		if (r < 0.33f)
			gi.sound (self, CHAN_WEAPON, sound_peck1, 1, ATTN_NORM, 0);
		else if (r < 0.67f)
			gi.sound (self, CHAN_WEAPON, sound_peck2, 1, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_WEAPON, sound_peck3, 1, ATTN_NORM, 0);
	}
}

mframe_t vulture_frames_melee [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, vulture_peck,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t	vulture_move_melee = {FRAME_melee1, FRAME_melee4, vulture_frames_melee, vulture_run};

void vulture_melee (edict_t *self)
{
	if (level.time < self->touch_debounce_time)
		return;

	self->touch_debounce_time = level.time + 0.5f;

	self->monsterinfo.currentmove = &vulture_move_melee;
}

/*
void vulture_attack (edict_t *self)
{
	int r;
	
	if (!self->enemy)
		return;

	r = range(self, self->enemy);

	if (r == RANGE_MELEE)
	{
		self->monsterinfo.currentmove = &vulture_move_melee;
	}
}
*/

//
// pain
//

mframe_t vulture_frames_pain [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t	vulture_move_pain = {FRAME_pain1, FRAME_pain4, vulture_frames_pain, vulture_run};

//
// soar pain
//

mframe_t vulture_frames_soarpain [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t	vulture_move_soarpain = {FRAME_soarpain1, FRAME_soarpain4, vulture_frames_soarpain, vulture_run};

void vulture_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if (rand() & 1)
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (self->flags & FL_FLY)
		self->monsterinfo.currentmove = &vulture_move_soarpain;
	else
		self->monsterinfo.currentmove = &vulture_move_pain;
}

//
// death
//

void vulture_dead (edict_t *self)
{
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
	M_FlyCheck (self);

	// Lazarus monster fade
	if (world->effects & FX_WORLDSPAWN_CORPSEFADE)
	{
		self->think = FadeDieSink;
		self->nextthink = level.time + corpse_fadetime->value;
	}
}

mframe_t vulture_frames_death [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t	vulture_move_death = {FRAME_die1, FRAME_die4, vulture_frames_death, vulture_dead};

//
// soar death
//

mframe_t vulture_frames_soardeath [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,

	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,

	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t	vulture_move_soardeath = {FRAME_soardie1, FRAME_soardie30, vulture_frames_soardeath, vulture_dead};

void vulture_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// check for gib
	if ( (self->health <= self->gib_health) && !(self->spawnflags & 32) )
	{
		gi.sound (self, CHAN_VOICE|CHAN_RELIABLE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		
		for (n = 0; n < 4; n++)
			ThrowGib (self, "models/monsters/vulture/feather1.md2", 0, 0, damage, GIB_FEATHER);
		for (n = 0; n < 4; n++)
			ThrowGib (self, "models/monsters/vulture/feather2.md2", 0, 0, damage, GIB_FEATHER);
		ThrowGib (self, "models/objects/gibs/bone/tris.md2", 0, 0, damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/bone2/tris.md2", 0, 0, damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if (self->flags & FL_FLY)
		self->monsterinfo.currentmove = &vulture_move_soardeath;
	else
		self->monsterinfo.currentmove = &vulture_move_death;
}


/*QUAKED monster_vulture (1 .5 0) (-8 -8 -8) (8 8 8) Ambush Trigger_Spawn Sight InAir GoodGuy NoGib
*/
void SP_monster_vulture (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

//	sound_sight =		gi.soundindex ("vulture/sight1.wav");
	sound_perch_idle1 =	gi.soundindex ("vulture/perch_idle1.wav");
	sound_perch_idle2 =	gi.soundindex ("vulture/perch_idle2.wav");
	sound_perch_idle3 =	gi.soundindex ("vulture/perch_idle3.wav");
	sound_soar_idle1 =	gi.soundindex ("vulture/soar_idle1.wav");
	sound_soar_idle2 =	gi.soundindex ("vulture/soar_idle2.wav");
	sound_soar_idle3 =	gi.soundindex ("vulture/soar_idle3.wav");
	sound_flap1 =	gi.soundindex ("vulture/flap1.wav");
	sound_flap2 =	gi.soundindex ("vulture/flap2.wav");
	sound_flap3 =	gi.soundindex ("vulture/flap3.wav");
	sound_peck1 =	gi.soundindex ("vulture/peck1.wav");
	sound_peck2 =	gi.soundindex ("vulture/peck2.wav");
	sound_peck3 =	gi.soundindex ("vulture/peck3.wav");
	sound_pain1 =	gi.soundindex ("vulture/pain1.wav");
	sound_pain2 =	gi.soundindex ("vulture/pain2.wav");
	sound_death =	gi.soundindex ("vulture/death1.wav");

	// precache feather gibs
	gi.modelindex ("models/monsters/vulture/feather1.md2");
	gi.modelindex ("models/monsters/vulture/feather2.md2");

	// Lazarus: special purpose skins
	if ( self->style )
	{
		PatchMonsterModel("models/monsters/vulture/tris.md2");
		self->s.skinnum = self->style;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/vulture/tris.md2");
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);	// was 8, 8, 10
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	// if spawned as landed, bump origin so we don't start in a brush
/*	if ( !(self->spawnflags & SF_VULTURE_IN_AIR) ) {
		self->s.origin[2] += 8;
	} */

	if (!self->health)
		self->health = 50;
	if (!self->gib_health)
		self->gib_health = -20;
	if (!self->mass)
		self->mass = 50;

	self->pain = vulture_pain;
	self->die = vulture_die;

	self->monsterinfo.stand = vulture_perch;
	self->monsterinfo.walk = vulture_walk;
	self->monsterinfo.run = vulture_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = NULL;
	self->monsterinfo.melee = vulture_melee;
	self->monsterinfo.sight = vulture_sight;
	self->monsterinfo.idle = vulture_idle;
	self->monsterinfo.search = NULL;
	self->monsterinfo.blocked = NULL;

	if (!self->monsterinfo.flies)
		self->monsterinfo.flies = 0.05;

	// Lazarus
	if (self->powerarmor)
	{
		if (self->powerarmortype == 1)
			self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
		else
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		self->monsterinfo.power_armor_power = self->powerarmor;
	}
	self->common_name = "Vulture";
	self->class_id = ENTITY_MONSTER_VULTURE;
	self->spawnflags |= SF_MONSTER_KNOWS_MIRRORS;

	gi.linkentity (self);

//	self->monsterinfo.currentmove = &vulture_move_perch;

	if (self->spawnflags & SF_VULTURE_IN_AIR) {
		self->monsterinfo.currentmove = &vulture_move_soar;	
		flymonster_start (self);
	}
	else {
		self->monsterinfo.currentmove = &vulture_move_perch;	
		walkmonster_start (self);
	}
	if (self->health < 0)
	{
		mmove_t	*deathmoves[] = {&vulture_move_death,
								 NULL};
		M_SetDeath (self, (mmove_t **)&deathmoves);
	}
	self->monsterinfo.scale = MODEL_SCALE;

}
