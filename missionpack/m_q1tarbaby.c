/*
==============================================================================

QUAKE TARBABY (BLOB)

==============================================================================
*/

#include "g_local.h"
#include "m_q1tarbaby.h"

static int	sound_death;
static int	sound_hit;
static int	sound_land;
static int	sound_sight;


void tarbaby_stand (edict_t *self);
void tarbaby_jump_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void tarbaby_check_landing (edict_t *self);
void tarbaby_fly (edict_t *self);
void tarbaby_takeoff (edict_t *self);


void tarbaby_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}


mframe_t tarbaby_frames_stand [] =
{
	ai_stand, 0, NULL
};
mmove_t tarbaby_move_stand = {FRAME_walk1, FRAME_walk1, tarbaby_frames_stand, tarbaby_stand};

void tarbaby_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &tarbaby_move_stand;
}


mframe_t tarbaby_frames_walk [] =
{
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
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL
};
mmove_t tarbaby_move_walk = {FRAME_walk1, FRAME_walk25, tarbaby_frames_walk, NULL};

void tarbaby_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &tarbaby_move_walk;
}


mframe_t tarbaby_frames_run [] =
{
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 0, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL,
	ai_run, 2, NULL
};
mmove_t tarbaby_move_run = {FRAME_run1, FRAME_run25, tarbaby_frames_run, NULL};

void tarbaby_run (edict_t *self)
{
	self->monsterinfo.currentmove = &tarbaby_move_run;
}


mframe_t tarbaby_frames_fly [] =
{
	ai_charge, 0,	 NULL,
	ai_charge, 0,	 NULL,
	ai_charge, 0,	 NULL,
	ai_charge, 0,	 tarbaby_check_landing
};
mmove_t tarbaby_move_fly = {FRAME_fly1, FRAME_fly4, tarbaby_frames_fly, tarbaby_fly};

void tarbaby_fly (edict_t *self)
{
	self->monsterinfo.currentmove = &tarbaby_move_fly;
}

mframe_t tarbaby_frames_jump [] =
{
	ai_charge, 0,	 NULL,
	ai_charge, 0,	 NULL,
	ai_charge, 0,	 NULL,
	ai_charge, 0,	 NULL,
	ai_charge, 0,	 tarbaby_takeoff,
	ai_charge, 0,	 NULL
};
mmove_t tarbaby_move_jump = {FRAME_jump1, FRAME_jump6, tarbaby_frames_jump, tarbaby_fly};

void tarbaby_jump_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (self->health <= 0) {
		self->touch = NULL;
		return;
	}

	if (other->takedamage && strcmp(other->classname, self->classname))
	{
		if (VectorLength(self->velocity) > 400)
		{
			vec3_t	point, normal;
			int		damage;

			damage = 10 + 10 * random();
			VectorCopy (self->velocity, normal);
			VectorNormalize (normal);
			VectorMA (self->s.origin, self->maxs[0], normal, point);
			T_Damage (other, self, self, self->velocity, point, normal, damage, damage, 0, MOD_UNKNOWN);
			gi.sound (self, CHAN_VOICE, sound_hit, 1, ATTN_NORM, 0);
		}
	}
	else
		gi.sound (self, CHAN_VOICE, sound_land, 1, ATTN_NORM, 0);

	if ( !M_CheckBottom (self) )
	{
		if (self->groundentity) {
			self->touch = NULL;
			self->monsterinfo.currentmove = &tarbaby_move_run;
			self->movetype = MOVETYPE_STEP;
			self->monsterinfo.attack_finished = 0;
			self->monsterinfo.aiflags &= ~AI_DUCKED;
		}
		return;	// not landed yet
	}
	self->touch = NULL;
	self->monsterinfo.currentmove = &tarbaby_move_jump;
}

void tarbaby_check_landing (edict_t *self)
{
	vec3_t	temp;

	VectorCopy (self->velocity, temp);
	temp[2] = 0.0f;

	if (self->groundentity)
	{
		self->touch = NULL;
		self->monsterinfo.currentmove = &tarbaby_move_run;
		self->movetype = MOVETYPE_STEP;
		self->monsterinfo.attack_finished = 0;
		self->monsterinfo.aiflags &= ~AI_DUCKED;
		return;
	}
	else  if (VectorLength(temp) == 0)	// random jump to un-stick
	{
		self->velocity[0] = (random() -0.5) * 600;
		self->velocity[1] = (random() -0.5) * 600;
		self->velocity[2] = 200;
		self->groundentity = NULL;
		return;
	}

	self->count++;
	if ( self->count == 4 )	// continue flying
	{
		self->monsterinfo.currentmove = &tarbaby_move_jump;
		self->monsterinfo.nextframe = FRAME_jump5;
		tarbaby_takeoff (self);
	}
}

void tarbaby_takeoff (edict_t *self)
{
	vec3_t forward, /*temp,*/ add;

	self->movetype = MOVETYPE_BOUNCE;
	self->s.origin[2] += 1;
	AngleVectors (self->s.angles, forward, NULL, NULL);
	VectorScale (forward, 600, self->velocity);
	VectorSet (add, 0, 0, 200);
	VectorAdd (self->velocity, add, self->velocity);
	self->monsterinfo.attack_finished = level.time + 3;
	self->touch = tarbaby_jump_touch;
	self->groundentity = NULL;
	self->count = 0;
}

void tarbaby_jump (edict_t *self)
{
	if (!self->enemy)
		return;

	self->monsterinfo.currentmove = &tarbaby_move_jump;
}


void tarbaby_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// The tarbaby has no pain frames, so nothing goes here.
}


void tarbaby_dead (edict_t *self)
{
	edict_t	*explode;
	vec3_t	temp;

	T_RadiusDamage (self, self, 120, NULL, 120, MOD_UNKNOWN);

	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);

	VectorCopy (self->velocity, temp);
	VectorNormalize (temp);
	VectorScale (temp, -8, temp);
	VectorAdd (self->s.origin, temp, temp);

	explode = G_Spawn ();
	VectorCopy (temp, explode->s.origin);
	VectorCopy (temp, explode->s.old_origin);
	explode->solid = SOLID_NOT;
	explode->movetype = MOVETYPE_NONE;
	explode->touch = NULL;
	VectorClear (explode->velocity);
	explode->s.modelindex = gi.modelindex ("sprites/s_explod.sp2"); 
	explode->s.frame = 0; 
	explode->s.sound = 0;
	explode->s.effects &= ~EF_ANIM_ALLFAST; 
	explode->think = q1_explode; 
	explode->nextthink = level.time + FRAMETIME;
	gi.linkentity (explode);

	gi.unlinkentity (self);
	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->touch = NULL;
	self->svflags |= SVF_NOCLIENT;
	self->think = G_FreeEdict;
	self->nextthink = level.time + 1;
	gi.linkentity (self);
}

mframe_t tarbaby_frames_explode [] =
{
	ai_move, 0,	 NULL
};
mmove_t tarbaby_move_explode = {FRAME_exp, FRAME_exp, tarbaby_frames_explode, tarbaby_dead};

void tarbaby_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->gib_health = -10000;

	self->monsterinfo.currentmove = &tarbaby_move_explode;
}


//
// SPAWN
//

/*QUAKED monster_q1_tarbaby (1 .5 0) (-16 -16 -24) (16 16 24) Ambush Trigger_Spawn Sight
model="models/monsters/q1tarbaby/tris.md2"
*/
void SP_monster_q1_tarbaby (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_death	= gi.soundindex ("q1blob/death1.wav");
	sound_hit	= gi.soundindex ("q1blob/hit1.wav");
	sound_land	= gi.soundindex ("q1blob/land1.wav");
	sound_sight	= gi.soundindex ("q1blob/sight1.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	// Lazarus: special purpose skins
	if ( self->style )
	{
		PatchMonsterModel("models/monsters/q1tarbaby/tris.md2");
		self->s.skinnum = self->style;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/q1tarbaby/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 24);

	if (!self->health)
		self->health = 80;
//	if (!self->gib_health)
		self->gib_health = 0;
	if (!self->mass)
		self->mass = 100;

	self->pain = tarbaby_pain;
	self->die = tarbaby_die;

	self->flags |= FL_Q1_MONSTER;

	self->monsterinfo.stand = tarbaby_stand;
	self->monsterinfo.walk = tarbaby_walk;
	self->monsterinfo.run = tarbaby_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = tarbaby_jump;
	self->monsterinfo.melee = tarbaby_jump;
	self->monsterinfo.sight = tarbaby_sight;
	self->monsterinfo.search = NULL;

	// Lazarus
	if (self->powerarmor)
	{
		if (self->powerarmortype == 1)
			self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
		else
			self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		self->monsterinfo.power_armor_power = self->powerarmor;
	}

	self->common_name = "Spawn";
	self->class_id = ENTITY_MONSTER_Q1_TARBABY;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &tarbaby_move_stand;	
	if (self->health < 0)
	{	// Knightmare- this just goes thru a single-frame animation and explodes
		mmove_t	*deathmoves[] = {&tarbaby_move_explode,
								 NULL};
		M_SetDeath (self, (mmove_t **)&deathmoves);
	}
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
