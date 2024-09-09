// g_misc_q1.c

#include "g_local.h"

void door_go_down (edict_t *self);
qboolean chthon_bolt (edict_t *self, int no);

//======================================================================
// CRUCIFIED ZOMBIE
//======================================================================

void misc_zombie_crucified_think (edict_t *self)
{
	float r1;

	if (++self->s.frame < 198) // was FRAME_death4_22
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 192;
		self->nextthink = level.time + FRAMETIME;
	}
	r1 = random();
	if (r1 <= .017)
	{
			gi.sound (self, CHAN_VOICE, gi.soundindex ("q1zombie/idle_w2.wav"), 1, ATTN_IDLE, 0);
	}
}

/*QUAKED misc_q1_zombie_crucified (1 .5 0) (-16 -16 -24) (16 16 32)
model="models/monsters/q1zombie/"
frame="192"
*/
void SP_misc_q1_zombie_crucified (edict_t *self)
{
	// precache
	gi.soundindex ("q1zombie/idle_w2.wav");

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->s.modelindex = gi.modelindex ("models/monsters/q1zombie/tris.md2");
	self->s.frame = 192;

	self->common_name = "Crucified Zombie";
	self->class_id = ENTITY_MISC_Q1_ZOMBIE_CRUCIFIED;

	gi.linkentity (self);

	self->think = misc_zombie_crucified_think;
	self->nextthink = level.time + FRAMETIME;
}

//======================================================================
// AIR BUBBLES
//======================================================================

void bubble_bob (edict_t *bubble);
void bubble_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!strcmp (other->classname, ent->classname))
	{
	//	gi.dprintf ("bump\n");
		return;
	}
	G_FreeEdict (ent);
}

void bubble_split (edict_t *bubble)
{
	edict_t *newbubble;
	float r1 = random();
	float r2 = random();
	int i, j;

	newbubble = G_Spawn();
	newbubble->classname = "bubble";
	newbubble->s.modelindex = gi.modelindex ("sprites/s_bubble.sp2");
	VectorCopy (bubble->s.origin, newbubble->s.origin);

	if (r1 > 0.5) i = 1;
	else i = 0;

	if (r2 > 0.5) j = 1;
	else j = -1;

	newbubble->s.origin[i] += (6 * j);

	newbubble->movetype = MOVETYPE_FLYMISSILE;
	newbubble->solid = SOLID_TRIGGER;
	VectorCopy (bubble->velocity, newbubble->velocity);
	newbubble->nextthink = level.time + 0.5;
	newbubble->think = bubble_bob;
	newbubble->touch = bubble_touch;
	newbubble->s.frame = 1;
	newbubble->count = 10;
	VectorSet (newbubble->mins, -2, -2, -2);
	VectorSet (newbubble->maxs, 2, 2, 2);
	gi.linkentity (newbubble);

	bubble->s.frame = 1;
	bubble->count = 10;
	if (bubble->waterlevel != 3)
		G_FreeEdict (bubble);
}

void bubble_bob (edict_t *bubble)
{
	float	rnd1, rnd2, rnd3;
//	vec3_t	vtmp1, modi;

	bubble->count++;
	if (bubble->count == 4)
		bubble_split (bubble);
	if (bubble->count == 20)
		G_FreeEdict (bubble);

	rnd1 = bubble->velocity[0] + (-10 + (random() * 20));
	rnd2 = bubble->velocity[1] + (-10 + (random() * 20));
	rnd3 = bubble->velocity[2] + 10 + random() * 10;

	if (rnd1 > 10)
		rnd1 = 5;
	if (rnd1 < -10)
		rnd1 = -5;
		
	if (rnd2 > 10)
		rnd2 = 5;
	if (rnd2 < -10)
		rnd2 = -5;
		
	if (rnd3 < 10)
		rnd3 = 15;
	if (rnd3 > 30)
		rnd3 = 25;
	
	bubble->velocity[0] = rnd1;
	bubble->velocity[1] = rnd2;
	bubble->velocity[2] = rnd3;
		
	bubble->nextthink = level.time + 0.5;
	bubble->think = bubble_bob;
}

void make_bubbles (edict_t *self)
{

	edict_t *bubble;

	bubble = G_Spawn();
	bubble->classname = "bubble";
	bubble->s.modelindex = gi.modelindex ("sprites/s_bubble.sp2");
	VectorCopy (self->s.origin, bubble->s.origin);
	bubble->movetype = MOVETYPE_FLYMISSILE;
	bubble->solid = SOLID_TRIGGER;
	VectorSet (bubble->mins, -2, -2, -2);
	VectorSet (bubble->maxs, 2, 2, 2);
	VectorSet (bubble->velocity, 0, 0, 15);
	bubble->nextthink = level.time + 0.5;
	bubble->think = bubble_bob;
	bubble->touch = bubble_touch;
	bubble->s.frame = 0;
	bubble->count = 0;

	bubble->common_name = "Bubble";
	bubble->class_id = ENTITY_Q1_BUBBLE;

	gi.linkentity (bubble);

	self->nextthink = level.time + random() + 0.5;
	self->think = make_bubbles;
}

/*QUAKED misc_q1_air_bubbles (1 0 0) (-8 -8 -8) (8 8 8)
This entity gives off sprite air bubbles, put it at the bottom of the water where the air should start bubbling up.
*/
void SP_misc_q1_air_bubbles (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->class_id = ENTITY_MISC_Q1_BUBBLE_SPAWNER;

	gi.modelindex ("sprites/s_bubble.sp2");
	self->nextthink = level.time + 1;
	self->think = make_bubbles;
	gi.linkentity (self);
}


//======================================================================
// GLOBE
//======================================================================

/*QUAKED misc_q1_globe (0 1 0) (-4 -4 -4) (4 4 4)
This is a sprite, is not solid
*/
void SP_misc_q1_globe (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.modelindex = gi.modelindex ("sprites/s_light.sp2");
	self->s.frame = 0;
	self->s.sound = 0;

	self->common_name = "Globe Light";
	self->class_id = ENTITY_MISC_Q1_GLOBE;

	gi.linkentity (self);
}

//======================================================================
// SMALL FLAME
//======================================================================

void q1_small_flame_think (edict_t *self)
{
	if (self->s.frame >= 5)
		self->s.frame = 0;
	else
		self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}	

/*QUAKED misc_q1_small_flame (0 1 0) (-4 -4 -4) (4 4 4)
not solid
model="models/objects/q1flame/"
*/
void SP_misc_q1_small_flame (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/objects/q1flame/tris.md2");
	ent->s.frame = 0;
	ent->s.sound = gi.soundindex("q1world/fire1.wav");
	ent->s.renderfx = RF_FULLBRIGHT | RF_NOSHADOW;
	ent->nextthink = level.time + FRAMETIME;
	ent->think = q1_small_flame_think;

	ent->common_name = "Small Flame";
	ent->class_id = ENTITY_MISC_Q1_FLAME_SMALL;

	gi.linkentity (ent);
}

//======================================================================
// LARGE FLAME
//======================================================================

void q1_large_flame_think (edict_t *self)
{
	if (self->s.frame >= 16)
		self->s.frame = 6;
	else
		self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}
	
/*QUAKED misc_q1_large_flame (0 1 0) (-4 -4 -4) (4 4 4)
not solid
model="models/objects/q1flame/"
frame="6"
*/
void SP_misc_q1_large_flame (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/objects/q1flame/tris.md2");
	ent->s.frame = 6;
	ent->s.sound = gi.soundindex("q1world/fire1.wav");
	ent->nextthink = level.time + FRAMETIME;
	ent->think = q1_large_flame_think;
	ent->s.renderfx = RF_FULLBRIGHT | RF_NOSHADOW;

	ent->common_name = "Large Flame";
	ent->class_id = ENTITY_MISC_Q1_FLAME_LARGE;

	gi.linkentity (ent);
}

//======================================================================
// TORCH
//======================================================================

void q1_torch_think (edict_t *self)
{
	if (self->s.frame >= 5)
		self->s.frame = 0;
	else
		self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_q1_torch (0 1 0) (-4 -4 -4) (4 4 4)
model="models/objects/q1torch/"
not solid
*/
void SP_misc_q1_torch (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/objects/q1torch/tris.md2");
	ent->s.frame = 0;
	ent->s.sound = gi.soundindex("q1world/fire1.wav");
	ent->s.renderfx = RF_FULLBRIGHT | RF_NOSHADOW;
	ent->nextthink = level.time + FRAMETIME;
	ent->think = q1_torch_think;

	ent->common_name = "Torch";
	ent->class_id = ENTITY_MISC_Q1_TORCH;

	gi.linkentity (ent);
}

/*
==============
target_q1_trap

spawnflag 1 = superspike
spawnflag 2 = laser
defualt speed 500, 600 for laser
damage 9 for spike, 15 for superspike, 15 for laser
==============
*/

void q1_use_target_trapshooter (edict_t *self, edict_t *other, edict_t *activator)
{
	qboolean super = (self->spawnflags & 1);

	gi.sound (self, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
	if (self->spawnflags & 2) {
		q1_fire_laser (self, self->s.origin, self->movedir, self->dmg, self->speed);
	}
	else {
		q1_fire_nail (self, self->s.origin, self->movedir, self->dmg, self->speed, super);
#ifndef KMQUAKE2_ENGINE_MOD
		gi.sound (self, CHAN_AUTO, gi.soundindex("q1weapons/s_end.wav"), 1.0, ATTN_NORM, 0);
#endif
	}
}

/*QUAKED target_q1_trap (1 0 0) (-8 -8 -8) (8 8 8) superspike laser
Fires a spike in the set direction when triggered.
default speed 500, 600 for laser
damage 9 for spike, 15 for superspike, 15 for laser
*/
void SP_target_q1_trap (edict_t *self)
{
	self->class_id = ENTITY_TARGET_Q1_TRAPSHOOTER;

	self->use = q1_use_target_trapshooter;
	G_SetMovedir (self->s.angles, self->movedir);
	if (self->spawnflags & 2)
	{
		self->noise_index = gi.soundindex ("q1enforcer/enfire.wav");
		q1_laser_precache ();
		if (!self->speed)
			self->speed = 600;
	}
	else {
#ifdef KMQUAKE2_ENGINE_MOD
		self->noise_index = gi.soundindex ("q1weapons/spike2.wav");
#else
		self->noise_index = gi.soundindex ("q1weapons/spike.wav");
		gi.soundindex("q1weapons/s_end.wav");
#endif
		q1_nail_precache ();
		if (!self->speed)
			self->speed = 500;
	}

	if ( (self->spawnflags & 1) || (self->spawnflags & 2) ) {
		if (!self->dmg)
			self->dmg = 15;
	}
	else {
		if (!self->dmg)
			self->dmg = 9;
	}
	self->svflags = SVF_NOCLIENT;
}

//======================================================================
// LIGHTNING BOLT
//======================================================================

/*static*/ void target_fire_lightning (edict_t *self, vec3_t start, vec3_t dir, int damage)
{
	vec3_t	end;
	trace_t	tr;

	VectorNormalize (dir);
	VectorMA (start, 600, dir, end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	
	gi.WriteByte (svc_temp_entity);
#ifdef KMQUAKE2_ENGINE_MOD
	gi.WriteByte (TE_LIGHTNING_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos); 
	gi.WriteByte (3);	// model 3 specifies trap bolt model 
#else
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos); 
#endif	// KMQUAKE2_ENGINE_MOD
	gi.multicast (start, MULTICAST_PVS);
		
	if ( (tr.ent != self) && (tr.ent->takedamage) )
		T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, 0, DAMAGE_ENERGY, MOD_Q1_LIGHTNING_TRAP);
}

void think_targetbolt (edict_t *self)
{
	qboolean	electrodes_found = false;
	qboolean	discharge_done = false;
	edict_t		*le1 = NULL;
	edict_t		*le2 = NULL;

	// find our electrodes
	if ( self->pathtarget && (self->pathtarget[0] != 0) )
		le1 = G_Find (NULL, FOFS(targetname), self->pathtarget);
	if ( self->followtarget && (self->followtarget[0] != 0) )
		le2 = G_Find (NULL, FOFS(targetname), self->followtarget);

	// check if electrodes exist and are the correct entity type
	if ( (le1 != NULL) && (le2 != NULL) )
	{
		if ( ( !Q_stricmp(le1->classname, "func_door") ||
			!Q_stricmp(le1->classname, "func_door_rotating") ||
			!Q_stricmp(le1->classname, "func_door_rot_dh") ) &&
			( !Q_stricmp(le2->classname, "func_door") ||
			!Q_stricmp(le2->classname, "func_door_rotating") ||
			!Q_stricmp(le2->classname, "func_door_rot_dh") ) )
		{
			electrodes_found = true;
		}
	}

	// bail out if on qe1m7 and electrodes not found
/*	if ( (Q_stricmp(level.mapname, "qe1m7") == 0) && !electrodes_found ) {
		self->nextthink = 0;
		self->think = NULL;
		self->delay = 0;
		return;
	} */

	// if discnarge is complete, clear think pointer and timer
	if (level.time >= self->delay) {
		self->nextthink = 0;
		self->think = NULL;
		self->delay = 0;
		discharge_done = true;
	}
	// if not done, fire lightning bolt again
	else {
		target_fire_lightning (self, self->s.origin, self->movedir, 0);	// damage is 0 because we're just extending the effect
	//	gi.sound (self, CHAN_AUTO, gi.soundindex ("q1weapons/lhit.wav"), 1.0, ATTN_NORM, 0);
		self->nextthink = level.time + 0.2;
		self->think = think_targetbolt;
	}

	// if discnarge is complete, retract electrodes
	if ( electrodes_found && discharge_done ) {
		door_go_down (le1);
		door_go_down (le2);
	}
}

void use_target_bolt (edict_t *self, edict_t *other, edict_t *activator)
{
	int			bstate1 = 0, bstate2 = 0;
	vec3_t		dir, start, end;
	trace_t		tr;
	qboolean	electrodes_found = false;
	qboolean	electrodes_aligned = false;
	qboolean	chthon_shocked = false;
	edict_t		*le1 = NULL;
	edict_t		*le2 = NULL;
	edict_t		*t = NULL;
	edict_t		*chthon = NULL;

	// find our electrodes
	if ( self->pathtarget && (self->pathtarget[0] != 0) )
		le1 = G_Find (NULL, FOFS(targetname), self->pathtarget);
	if ( self->followtarget && (self->followtarget[0] != 0) )
		le2 = G_Find (NULL, FOFS(targetname), self->followtarget);
		
	// check if electrodes exist and are the correct entity type
	// also check if they are aligned
	if ( (le1 != NULL) && (le2 != NULL) )
	{
		if ( ( !Q_stricmp(le1->classname, "func_door") ||
			!Q_stricmp(le1->classname, "func_door_rotating") ||
			!Q_stricmp(le1->classname, "func_door_rot_dh") ) &&
			( !Q_stricmp(le2->classname, "func_door") ||
			!Q_stricmp(le2->classname, "func_door_rotating") ||
			!Q_stricmp(le2->classname, "func_door_rot_dh") ) )
		{
			electrodes_found = true;
			bstate1 = le1->moveinfo.state;
			bstate2 = le2->moveinfo.state;
		//	if (bstate1 == bstate2)
			if ( ( (bstate1 == 0) && (bstate2 == 0) ) || ( (bstate1 == 1) && (bstate2 == 1) ) )
				electrodes_aligned = true;
		}
	}
		
	// bail out if electrodes found and not aligned
	if ( electrodes_found && !electrodes_aligned ) {
		return;
	}

	// if both electrodes are up, fire lightning from our target ent target_q1_bolt 
	if ( electrodes_aligned && bstate1 )
	{
		t = G_Find (NULL, FOFS(targetname), self->target);
		if ( t && !Q_stricmp(t->classname, "target_q1_bolt") )
		{
			target_fire_lightning (t, t->s.origin, t->movedir, t->dmg);
		//	gi.sound (t, CHAN_AUTO, gi.soundindex("q1weapons/lstart.wav"), 1.0, ATTN_NORM, 0);
			gi.sound (t, CHAN_AUTO, t->noise_index, 1.0, ATTN_NORM, 0);
			t->nextthink = level.time + 0.2;
			t->think = think_targetbolt;
			t->delay = level.time + 1.8;
			return;
		}
	}
	// if both electrodes are down, find Chthon and shock him
	else if ( electrodes_aligned && (bstate2 == 0) && (bstate1 == 0) )
	{
		// prevent electrodes from retracting until shock is finished
		le1->nextthink = 0;
		le2->nextthink = 0;

		// trace forward according to movedir and see if we hit Chthon
		VectorCopy (self->movedir, dir);
		VectorNormalize (dir);
		VectorCopy (self->s.origin, start);
		VectorMA (start, 600, dir, end);
		tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
		if ( (tr.ent != self) && tr.ent->classname && (tr.ent->classname[0] != 0) &&
			( !Q_stricmp(tr.ent->classname, "q1_monster_chton") || !Q_stricmp(tr.ent->classname, "monster_q1_chthon") ) )
		{
			chthon = tr.ent;
			if ( !chthon->deadflag && chthon->enemy ) {
				chthon_shocked = true;
				self->count++;
				if ( chthon_bolt (chthon, self->count) )
					self->count = 0;
			}
		//	else
		//		gi.dprintf ("Chthon does not have an enemy\n");
		}
	//	else
	//		gi.dprintf ("Could not find Chthon\n");
	}

	// do no extra damage to Chthon to avoid causing early death sequence
	if (chthon_shocked)
		target_fire_lightning (self, self->s.origin, self->movedir, 0);
	else
		target_fire_lightning (self, self->s.origin, self->movedir, self->dmg);
//	gi.sound (self, CHAN_AUTO, gi.soundindex("q1weapons/lstart.wav"), 1.0, ATTN_NORM, 0);
	gi.sound (self, CHAN_AUTO, self->noise_index, 1.0, ATTN_NORM, 0);
	self->nextthink = level.time + 0.2;
	self->think = think_targetbolt;
	self->delay = level.time + 1.8;
}

/*QUAKED q1_target_bolt (1 0 0) (-8 -8 -8) (8 8 8)
Fires lightning bolt in the set direction when triggered.
*/

void SP_target_q1_bolt (edict_t *self)
{
	self->use = use_target_bolt;
	self->count = 0;
	G_SetMovedir (self->s.angles, self->movedir);
//	self->noise_index = gi.soundindex ("q1weapons/lhit.wav"); 
//	gi.soundindex ("q1weapons/lstart.wav");
	self->noise_index = gi.soundindex ("q1misc/power.wav"); 
	
	// check pathtarget and followtarget, set default values if unset
/*	if (Q_stricmp(level.mapname, "qe1m7") == 0)	// qe1m7 uses hardcoded targetnames for electrodes
	{
		if ( !self->pathtarget || (self->pathtarget[0] == 0) || !self->followtarget || (self->followtarget[0] == 0) ) {
			gi.dprintf ("target_q1_bolt without a pathtarget/followtarget at %s\n", vtos(self->s.origin));
			self->pathtarget = G_CopyString ("t12");
			self->followtarget = G_CopyString ("t13");
		}
	}
	else
	{ */
		if ( !self->pathtarget || (strlen(self->pathtarget) == 0) ) {
			gi.dprintf ("target_q1_bolt without a pathtarget at %s\n", vtos(self->s.origin));
			self->pathtarget = G_CopyString ("foo_1");
		}
		if ( !self->followtarget || (strlen(self->followtarget) == 0) ) {
			gi.dprintf ("target_q1_bolt without a followtarget at %s\n", vtos(self->s.origin));
			self->followtarget = G_CopyString ("foo_2");
		}
//	}

	if (!self->dmg)
		self->dmg = 50;
	self->svflags = SVF_NOCLIENT;
}

//======================================================================
// Q1 EXPLOBOX
//======================================================================

void q1_barrel_explode (edict_t *self)
{
	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 5)
		G_FreeEdict(self);
}

/*static*/ void q1_explobox_delay (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + 2 * FRAMETIME;
	self->activator = attacker;
	
	T_RadiusDamage (self, self->activator, self->dmg, NULL, self->dmg+30, MOD_BARREL);

	if (!deathmatch->value)
	{
		vec3_t	org;
		float	spd;
		vec3_t	save;
	
		VectorCopy (self->s.origin, save);
		VectorMA (self->absmin, 0.5, self->size, self->s.origin);

		// a few big chunks
		spd = 1.5 * (float)self->dmg / 200.0;
		org[0] = self->s.origin[0] + crandom() * self->size[0];
		org[1] = self->s.origin[1] + crandom() * self->size[1];
		org[2] = self->s.origin[2] + crandom() * self->size[2];
		ThrowDebris (self, "models/objects/debris1/tris.md2", spd, org, 0, 0, 0);
		org[0] = self->s.origin[0] + crandom() * self->size[0];
		org[1] = self->s.origin[1] + crandom() * self->size[1];
		org[2] = self->s.origin[2] + crandom() * self->size[2];
		ThrowDebris (self, "models/objects/debris1/tris.md2", spd, org, 0, 0, 0);

		// bottom corners
		spd = 1.75 * (float)self->dmg / 200.0;
		VectorCopy (self->absmin, org);
		ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org, 0, 0, 0);
		VectorCopy (self->absmin, org);
		org[0] += self->size[0];
		ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org, 0, 0, 0);
		VectorCopy (self->absmin, org);
		org[1] += self->size[1];
		ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org, 0, 0, 0);
		VectorCopy (self->absmin, org);
		org[0] += self->size[0];
		org[1] += self->size[1];
		ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org, 0, 0, 0);

		// a bunch of little chunks
		spd = 2 * self->dmg / 200;
		org[0] = self->s.origin[0] + crandom() * self->size[0];
		org[1] = self->s.origin[1] + crandom() * self->size[1];
		org[2] = self->s.origin[2] + crandom() * self->size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org, 0, 0, 0);
		org[0] = self->s.origin[0] + crandom() * self->size[0];
		org[1] = self->s.origin[1] + crandom() * self->size[1];
		org[2] = self->s.origin[2] + crandom() * self->size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org, 0, 0, 0);
		org[0] = self->s.origin[0] + crandom() * self->size[0];
		org[1] = self->s.origin[1] + crandom() * self->size[1];
		org[2] = self->s.origin[2] + crandom() * self->size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org, 0, 0, 0);
		org[0] = self->s.origin[0] + crandom() * self->size[0];
		org[1] = self->s.origin[1] + crandom() * self->size[1];
		org[2] = self->s.origin[2] + crandom() * self->size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org, 0, 0, 0);
		org[0] = self->s.origin[0] + crandom() * self->size[0];
		org[1] = self->s.origin[1] + crandom() * self->size[1];
		org[2] = self->s.origin[2] + crandom() * self->size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org, 0, 0, 0);
		org[0] = self->s.origin[0] + crandom() * self->size[0];
		org[1] = self->s.origin[1] + crandom() * self->size[1];
		org[2] = self->s.origin[2] + crandom() * self->size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org, 0, 0, 0);
		org[0] = self->s.origin[0] + crandom() * self->size[0];
		org[1] = self->s.origin[1] + crandom() * self->size[1];
		org[2] = self->s.origin[2] + crandom() * self->size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org, 0, 0, 0);
		org[0] = self->s.origin[0] + crandom() * self->size[0];
		org[1] = self->s.origin[1] + crandom() * self->size[1];
		org[2] = self->s.origin[2] + crandom() * self->size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org, 0, 0, 0);

		VectorCopy (save, self->s.origin);
	}
	
	gi.sound (self, CHAN_AUTO, gi.soundindex ("q1weapons/r_exp3.wav"), 1.0, ATTN_NORM, 0);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// explosion sprite
	gi.unlinkentity (self);
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/s_explod.sp2"); 
	self->s.frame = 0; 
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST; 
	self->think = q1_barrel_explode; 
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
}

/*QUAKED misc_q1_explobox (0 .5 .8) (-16 -16 -16) (16 16 48) small
If small is checked, it is only 40, not 64 units tall.
health defaults to 20
dmg defaults to 160
mass defaults to 400
model="models/objects/q1explo/big/"
*/
void SP_misc_q1_explobox (edict_t *self)
{
	// precache
	gi.modelindex ("sprites/s_explod.sp2");
	gi.soundindex ("q1weapons/r_exp3.wav");

	self->solid = SOLID_BBOX;
	//self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags &= ~SVF_NOCLIENT;
	
	if (self->spawnflags & 1)
	{
		self->model = "models/objects/q1explo/small/tris.md2";
		VectorSet (self->mins, -16, -16, -16);
		VectorSet (self->maxs, 16, 16, 24);
	}
	else
	{
		self->model = "models/objects/q1explo/big/tris.md2";
		VectorSet (self->mins, -16, -16, -16);
		VectorSet (self->maxs, 16, 16, 48);
	}
		
	self->s.modelindex = gi.modelindex (self->model);

	if (!self->mass)
		self->mass = 400;
	if (!self->health)
		self->health = 20;
	if (!self->dmg)
		self->dmg = 160;

	self->die = q1_explobox_delay;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.aiflags = AI_NOSTEP;
	self->touch = NULL;
	self->think = M_droptofloor;
	self->nextthink = level.time + 2 * FRAMETIME;

	self->common_name = "Exploding Box";
	self->class_id = ENTITY_MISC_Q1_EXPLOBOX;

	gi.linkentity (self);
}

//======================================================================
// FIREBALL
//======================================================================

void q1_fireball_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 0, 0, 0);
	G_FreeEdict (self);
}
	
void q1_fireball_fly (edict_t *self)
{
	edict_t *lavaball;

//	gi.dprintf ("Launching Fireball\n");
	
	lavaball = G_Spawn();
	lavaball->speed = 200;
	VectorCopy (self->s.origin, lavaball->s.origin);
	VectorCopy (self->movedir, lavaball->movedir);
	vectoangles (self->movedir, lavaball->s.angles);
	VectorSet(lavaball->velocity,((random() * 100) - 50),
								   ((random() * 100) - 50),
								   (self->speed + (random() * 150)));

//	lavaball->movetype = MOVETYPE_FLYMISSILE;
	lavaball->movetype = MOVETYPE_TOSS;
	lavaball->clipmask = MASK_SHOT;
	lavaball->solid = SOLID_BBOX;
	lavaball->s.effects |= EF_ROCKET; // EF_FLAG1
	lavaball->s.renderfx |= RF_GLOW;
	VectorClear (lavaball->mins);
	VectorClear (lavaball->maxs);
	VectorSet (lavaball->mins, -8, -8, -8);
	VectorSet (lavaball->maxs, 8, 8, 8);
	lavaball->s.modelindex = gi.modelindex ("models/monsters/q1chthon/lavaball/tris.md2");
	lavaball->owner = self;
	lavaball->touch = q1_fireball_touch;
	lavaball->nextthink = level.time + 7;
	lavaball->think = G_FreeEdict;
	lavaball->dmg = 20;
	lavaball->classname = "lavaball";

	lavaball->common_name = "Lavaball";
	lavaball->class_id = ENTITY_Q1_FIREBALL;

	gi.linkentity (lavaball);

	self->nextthink = level.time + (4 + (random() * 3.0));
	self->think = q1_fireball_fly;
}

/*QUAKED misc_q1_fireball (.5 .5 .5) (-8 -8 -8) (8 8 8) 
model="models/monsters/q1chthon/lavaball/"
*/
void SP_misc_q1_fireball (edict_t *self)
{
	// precache
	gi.modelindex ("models/monsters/q1chthon/lavaball/tris.md2");

	self->nextthink = level.time + (4 + (random() * 3.0));
	self->think = q1_fireball_fly;
	self->classname = "fireball";

	self->common_name = "Lavaball Spawner";
	self->class_id = ENTITY_MISC_Q1_FIREBALL_SPAWNER;

	gi.linkentity(self);
}
