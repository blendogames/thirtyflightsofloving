#include "g_local.h"

#define MONSTER_TRIGGER_SPAWN 2


void InitiallyDead (edict_t *self);

// Lazarus: If worldspawn CORPSE_SINK effects flag is set,
//          monsters/actors fade out and sink into the floor
//          30 seconds after death

#define SINKAMT			1

#ifdef KMQUAKE2_ENGINE_MOD
void FadeSink (edict_t *ent)
{
	ent->count++;
	if ((ent->count % 2) == 0)
		ent->s.origin[2] -= SINKAMT;
	ent->s.alpha -= 0.05;
	if (ent->s.alpha < 0.0)
		ent->s.alpha = 0.0;
	if (ent->s.alpha <= 0.0)
	{
		G_FreeEdict (ent);
		return;
	}
	if (ent->count == 20)
		ent->think = G_FreeEdict;
	ent->think = FadeSink;
	ent->nextthink = level.time + 2*FRAMETIME;
}

void FadeDieSink (edict_t *ent)
{
	ent->takedamage = DAMAGE_NO;	// can't gib 'em once they start sinking
	ent->s.effects &= ~EF_FLIES;
	ent->s.sound = 0;
	ent->s.origin[2] -= SINKAMT;
	if (ent->s.renderfx & RF_TRANSLUCENT)
		ent->s.alpha = 0.70;
	else if (ent->s.effects & EF_SPHERETRANS)
		ent->s.alpha = 0.30;
	else if (!(ent->s.alpha)  || ent->s.alpha <= 0.0)
		ent->s.alpha = 1.00;
	ent->think = FadeSink;
	ent->nextthink = level.time + FRAMETIME;
	ent->count = 0;
}

#else

void FadeSink (edict_t *ent)
{
	ent->count++;
	ent->s.origin[2] -= SINKAMT;
	ent->think = FadeSink;
	if (ent->count == 5)
	{
		ent->s.renderfx &= ~RF_TRANSLUCENT;
		ent->s.effects |= EF_SPHERETRANS;
	}
	if (ent->count == 10)
		ent->think = G_FreeEdict;
	ent->nextthink = level.time+FRAMETIME;
}

void FadeDieSink (edict_t *ent)
{
	ent->takedamage = DAMAGE_NO;	// can't gib 'em once they start sinking
	ent->s.effects &= ~EF_FLIES;
	ent->s.sound = 0;
	ent->s.origin[2] -= SINKAMT;
	ent->s.renderfx = RF_TRANSLUCENT;
	ent->think = FadeSink;
	ent->nextthink = level.time + FRAMETIME;
	ent->count = 0;
}
#endif

// Lazarus: M_SetDeath is used to restore the death movement,
//          bounding box, and a few other parameters for dead
//          monsters that change levels with a trigger_transition

qboolean M_SetDeath (edict_t *self, mmove_t **deathmoves)
{
	mmove_t	*move=NULL;
	mmove_t *dmove;

	if (self->health > 0)
		return false;

	while (*deathmoves && !move)
	{
		dmove = *deathmoves;
		if ( (self->s.frame >= dmove->firstframe) &&
			(self->s.frame <= dmove->lastframe)     )
			move = dmove;
		else
			deathmoves++;
	}
	if (move)
	{
		self->monsterinfo.currentmove = move;
		if (self->monsterinfo.currentmove->endfunc)
			self->monsterinfo.currentmove->endfunc(self);
		self->s.frame = move->lastframe;
		// Gekk has 2 pain skins, requires special handling
		if ( strcmp(self->classname, "monster_gekk") == 0 )	
		{
			if (self->style)
				self->s.skinnum = self->style * 3 + 2;
			else
				self->s.skinnum = 2;
		}
		// Exclude fixbot, vulture, and Q1 monsters, as they have no pain skins
		else if ( (strcmp(self->classname, "monster_fixbot") != 0) && (strcmp(self->classname, "monster_vulture") != 0)
				&& !(self->flags & FL_Q1_MONSTER) )
		{
			self->s.skinnum |= 1;
		}
		return true;
	}
	return false;
}

//
// monster weapons
//

//FIXME mosnters should call these with a totally accurate direction
// and we can mess it up based on skill.  Spread should be for normal
// and we can tighten or loosen based on skill.  We could muck with
// the damages too, but I'm not sure that's such a good idea.
void monster_fire_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int kick, int hspread, int vspread, int flashtype)
{
	fire_bullet (self, start, dir, damage, kick, hspread, vspread, MOD_UNKNOWN);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

void monster_fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int flashtype)
{
	fire_shotgun (self, start, aimdir, damage, kick, hspread, vspread, count, MOD_UNKNOWN);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

void monster_fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect, int color)
{
	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_blaster (self, start, dir, damage, speed, effect, false, color);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (color - 1);	// orange = 0, green = 1, blue = 2, red = 3
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

void monster_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int flashtype, qboolean contact)
{
	fire_grenade (self, start, aimdir, damage, speed, 2.5, damage+40, contact);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

void monster_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, edict_t *homing_target)
{
	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_rocket (self, start, dir, damage, speed, damage+20, damage, homing_target);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

// Knightmare added
void monster_fire_missile (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, edict_t *homing_target)
{
	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_missile (self, start, dir, damage, speed, damage+20, damage, homing_target);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

void monster_fire_railgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int flashtype)
{
	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	// PMM
	if (!(gi.pointcontents (start) & MASK_SOLID))
		fire_rail (self, start, aimdir, damage, kick, false, 0, 0, 0);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

void monster_fire_bfg (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int kick, float damage_radius, int flashtype)
{
	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_bfg (self, start, aimdir, damage, speed, damage_radius);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

// RAFAEL
void monster_fire_blueblaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect)
{
	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_blueblaster (self, start, dir, damage, speed, effect);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (2);	// blue = 2
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}	

// RAFAEL
void monster_fire_ionripper (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect)
{
	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_ionripper (self, start, dir, damage, speed, effect);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

// Knightmare- this should have been added by Xatrix!
void monster_fire_phalanx (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, int flashtype)
{
	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_phalanx_plasma (self, start, dir, damage, speed, damage_radius, radius_damage);

	// Sends new MZ2_GLADBETA_PHALANX_1 for KMQ2 builds, or MZ2_GLADIATOR_RAILGUN_1 for non-KMQ2
	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

// RAFAEL
void monster_fire_rocket_heat (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype)
{
	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_rocket_heat (self, start, dir, damage, speed, damage, damage);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

// RAFAEL
void dabeam_hit (edict_t *self)
{
	edict_t	*ignore;
	vec3_t	start;
	vec3_t	end;
	trace_t	tr;
	int		count;
	static	vec3_t	lmins = {-4, -4, -4};
	static	vec3_t	lmaxs = {4, 4, 4};

	if (self->spawnflags & 0x80000000)
		count = 8;
	else
		count = 4;

	ignore = self;
	VectorCopy (self->s.origin, start);
	VectorMA (start, 2048, self->movedir, end);
	
	while (1)
	{	// Knightmare- double trace here, needed to make sure clipping works- must be compiler weirdness
		tr = gi.trace (start, NULL, NULL, end, ignore, (CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER));
		tr = gi.trace (start, NULL, NULL, end, ignore, (CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER));
		if (!tr.ent)
			break;

		// hurt it if we can
		if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
			T_Damage (tr.ent, self, self->owner, self->movedir, tr.endpos, vec3_origin, self->dmg, skill->value, DAMAGE_ENERGY, MOD_TARGET_LASER);

		if (self->dmg < 0) // healer ray
		{
			// when player is at 100 health
			// just undo health fix
			// keeping fx
			if (tr.ent->client && tr.ent->health > 100)
				tr.ent->health += self->dmg; 
		}
		
		// if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		{
			if (self->spawnflags & 0x80000000)
			{
				self->spawnflags &= ~0x80000000;
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (10);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self->s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
			}
			break;
		}

		ignore = tr.ent;
		VectorCopy (tr.endpos, start);
	}


	VectorCopy (tr.endpos, self->s.old_origin);
	self->nextthink = level.time + 0.1;
	self->think = G_FreeEdict;
  
}

// RAFAEL
void monster_fire_dabeam (edict_t *self)
{
	vec3_t last_movedir;
	vec3_t point;
	
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1;
		
	self->s.frame = 2;
	
	if (self->owner->monsterinfo.aiflags & AI_MEDIC)
		self->s.skinnum = 0xf3f3f1f1;
	else
		self->s.skinnum = 0xf2f2f0f0;

	if (self->enemy)
	{
		VectorCopy (self->movedir, last_movedir);
		VectorMA (self->enemy->absmin, 0.5, self->enemy->size, point);
		if (self->owner->monsterinfo.aiflags & AI_MEDIC)
			point[0] += sin (level.time) * 8;
		VectorSubtract (point, self->s.origin, self->movedir);
		VectorNormalize (self->movedir);
		if (!VectorCompare(self->movedir, last_movedir))
			self->spawnflags |= 0x80000000;
	}
	else
		G_SetMovedir (self->s.angles, self->movedir);

	self->think = dabeam_hit;
	self->nextthink = level.time + 0.1;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	gi.linkentity (self);
 
	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
}

// ROGUE
void monster_fire_blaster2 (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect)
{
	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_blaster2 (self, start, dir, damage, speed, effect, false);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

// FIXME -- add muzzle flash
void monster_fire_tracker (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, edict_t *enemy, int flashtype)
{
	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_tracker (self, start, dir, damage, speed, enemy);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

void monster_fire_heat (edict_t *self, vec3_t start, vec3_t dir, vec3_t offset, int damage, int kick, int flashtype)
{
	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_heat (self, start, dir, offset, damage, kick, true);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}
// ROGUE

// Knightmare added
void monster_fire_flechette (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, int flashtype)
{
	fire_flechette (self, start, dir, damage, speed, damage_radius, radius_damage);

#ifdef KMQUAKE2_ENGINE_MOD	// Knightmare- client muzzle flash for monster ETF Rifle is only in KMQ2
	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
#endif	// KMQUAKE2_ENGINE_MOD
}

void monster_fire_prox (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int damage_multiplier, int speed, int health, float timer, float damage_radius, int flashtype)
{
	fire_prox (self, start, aimdir, damage, damage_multiplier, speed, health, timer, damage_radius);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}
// end Knightmare

// SKWiD MOD
void monster_fire_plasma_rifle (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, qboolean spread)
{
	// Zaero add
	if (EMPNukeCheck(self, start))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	fire_plasma_rifle (self, start, dir, damage, speed, spread);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if (flashtype >= MZ2_SEND_SHORT) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte ((spread) ? 1 : 0);	// 1 = spread fire sound
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}


//
// Monster utility functions
//

void M_FliesOff (edict_t *self)
{
	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
}

void M_FliesOn (edict_t *self)
{
	if (self->waterlevel)
		return;

	self->s.effects |= EF_FLIES;
	self->s.sound = gi.soundindex ("infantry/inflies1.wav");
	self->think = M_FliesOff;
	self->nextthink = level.time + 60;
}

void M_FlyCheck (edict_t *self)
{	
	// Knightmare- keep running lava check
	self->postthink = deadmonster_think;

	if (self->monsterinfo.flies > 1.0)
	{
		// should ALREADY have flies
		self->think = M_FliesOff;
		self->nextthink = level.time + 60;
		return;
	}

	if (self->waterlevel)
		return;

	if (random() > self->monsterinfo.flies) // was 0.33
		return;

	if (world->effects & FX_WORLDSPAWN_CORPSEFADE)
		return;

	self->think = M_FliesOn;
	self->nextthink = level.time + 5 + 10 * random();
}

void AttackFinished (edict_t *self, float time)
{
	self->monsterinfo.attack_finished = level.time + time;
}

void M_CheckGround (edict_t *ent)
{
	vec3_t		point;
	trace_t		trace;

	if (level.time < ent->gravity_debounce_time)
		return;

	if (ent->flags & (FL_SWIM|FL_FLY))
		return;

#ifdef ROGUE_GRAVITY
	if ((ent->velocity[2] * ent->gravityVector[2]) < -100)		// PGM
#else
	if (ent->velocity[2] > 100)
#endif
	{
		ent->groundentity = NULL;
		return;
	}

// if the hull point one-quarter unit down is solid the entity is on ground
	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
#ifdef ROGUE_GRAVITY
	point[2] = ent->s.origin[2] + (0.25 * ent->gravityVector[2]);	//PGM
#else
	point[2] = ent->s.origin[2] - 0.25;
#endif

	trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, point, ent, MASK_MONSTERSOLID);

	// check steepness
#ifdef ROGUE_GRAVITY
//PGM
	if ( ent->gravityVector[2] < 0)		// normal gravity
	{
		if ( trace.plane.normal[2] < 0.7 && !trace.startsolid)
		{
			ent->groundentity = NULL;
			return;
		}
	}
	else								// inverted gravity
	{
		if ( trace.plane.normal[2] > -0.7 && !trace.startsolid)
		{
			ent->groundentity = NULL;
			return;
		}
	}
//PGM
#else
	if ( trace.plane.normal[2] < 0.7 && !trace.startsolid)
	{
		ent->groundentity = NULL;
		return;
	}
#endif

	// Lazarus: The following 2 lines were in the original code and commented out
	//          by id. However, the effect of this is that a player walking over
	//          a dead monster who is laying on a brush model will cause the 
	//          dead monster to drop through the brush model. This change *may*
	//          have other consequences, though, so watch out for this.
	// Knightmare- this wrecks aiming for many/most monster_turrets in a map!
	// Leave this commented out, or disable it for turrets!
//	ent->groundentity = trace.ent;
//	ent->groundentity_linkcount = trace.ent->linkcount;

//	if (!trace.startsolid && !trace.allsolid)
//		VectorCopy (trace.endpos, ent->s.origin);
	if (!trace.startsolid && !trace.allsolid)
	{
		VectorCopy (trace.endpos, ent->s.origin);
		ent->groundentity = trace.ent;
		ent->groundentity_linkcount = trace.ent->linkcount;
		ent->velocity[2] = 0;
	}
}


void M_CatagorizePosition (edict_t *ent)
{
	vec3_t		point;
	int			cont;

//
// get waterlevel
//
	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
	point[2] = ent->s.origin[2] + ent->mins[2] + 1;
	cont = gi.pointcontents (point);

	if (!(cont & MASK_WATER))
	{
		ent->waterlevel = 0;
		ent->watertype = 0;
		return;
	}

	ent->watertype = cont;
	ent->waterlevel = 1;
	point[2] += 26;
	cont = gi.pointcontents (point);
	if (!(cont & MASK_WATER))
		return;

	ent->waterlevel = 2;
	point[2] += 22;
	cont = gi.pointcontents (point);
	if (cont & MASK_WATER)
		ent->waterlevel = 3;
}


void M_WorldEffects (edict_t *ent)
{
	int		dmg;

	if (ent->health > 0)
	{
		if (!(ent->flags & FL_SWIM))
		{
			if (ent->waterlevel < 3)
			{
				ent->air_finished = level.time + 12;
			}
			else if (ent->air_finished < level.time)
			{	// drown!
				if (ent->pain_debounce_time < level.time)
				{
					dmg = 2 + 2 * floor(level.time - ent->air_finished);
					if (dmg > 15)
						dmg = 15;
					T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
					ent->pain_debounce_time = level.time + 1;
				}
			}
		}
		else
		{
			if (ent->waterlevel > 0)
			{
				ent->air_finished = level.time + 9;
			}
			else if (ent->air_finished < level.time)
			{	// suffocate!
				if (ent->pain_debounce_time < level.time)
				{
					dmg = 2 + 2 * floor(level.time - ent->air_finished);
					if (dmg > 15)
						dmg = 15;
					T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
					ent->pain_debounce_time = level.time + 1;
				}
			}
		}
	}

	// Knightmare added- all monsters are IR visible
	ent->s.renderfx |= RF_IR_VISIBLE;

	if (ent->waterlevel == 0)
	{
		if (ent->flags & FL_INWATER)
		{
			if (ent->watertype & CONTENTS_MUD)
				gi.sound (ent, CHAN_BODY, gi.soundindex("mud/mud_out1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_BODY, gi.soundindex("player/watr_out.wav"), 1, ATTN_NORM, 0);
			ent->flags &= ~FL_INWATER;
		}
		return;
	}

	if ((ent->watertype & CONTENTS_LAVA) && !(ent->flags & FL_IMMUNE_LAVA))
	{
		if (ent->damage_debounce_time < level.time)
		{
			ent->damage_debounce_time = level.time + 0.2;
			T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 10*ent->waterlevel, 0, 0, MOD_LAVA);
		}
	}
	// No slime damage for dead monsters
	if ((ent->watertype & CONTENTS_SLIME) && !(ent->flags & FL_IMMUNE_SLIME) && !(ent->svflags & SVF_DEADMONSTER))
	{
		if (ent->damage_debounce_time < level.time)
		{
			ent->damage_debounce_time = level.time + 1;
			T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 4*ent->waterlevel, 0, 0, MOD_SLIME);
		}
	}

	if ( !(ent->flags & FL_INWATER) )
	{
		if (!(ent->svflags & SVF_DEADMONSTER))
		{
			if ((ent->watertype & CONTENTS_LAVA) && (ent->health > 0))
			{
				if (random() <= 0.5)
					gi.sound (ent, CHAN_BODY, gi.soundindex("player/lava1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (ent, CHAN_BODY, gi.soundindex("player/lava2.wav"), 1, ATTN_NORM, 0);
			}
			else if (ent->watertype & CONTENTS_SLIME)
				gi.sound (ent, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
			else if (ent->watertype & CONTENTS_MUD)
				gi.sound (ent, CHAN_BODY, gi.soundindex("mud/mud_in2.wav"), 1, ATTN_NORM, 0);
			else if (ent->watertype & CONTENTS_WATER)
				gi.sound (ent, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
		}

		ent->flags |= FL_INWATER;
		ent->old_watertype = ent->watertype;
		ent->damage_debounce_time = 0;
	}
}


void M_droptofloor (edict_t *ent)
{
	vec3_t		end;
	trace_t		trace;

#ifdef ROGUE_GRAVITY
//PGM
	if (ent->gravityVector[2] < 0)
	{
		ent->s.origin[2] += 1;
		VectorCopy (ent->s.origin, end);
		end[2] -= 256;
	}
	else
	{
		ent->s.origin[2] -= 1;
		VectorCopy (ent->s.origin, end);
		end[2] += 256;
	}
//PGM
#else
	ent->s.origin[2] += 1;
	VectorCopy (ent->s.origin, end);
	end[2] -= 256;
#endif

	trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

	if (trace.fraction == 1 || trace.allsolid)
		return;

	VectorCopy (trace.endpos, ent->s.origin);

	gi.linkentity (ent);
	M_CheckGround (ent);
	M_CatagorizePosition (ent);
}


void M_SetEffects (edict_t *ent)
{
	int remaining;

	ent->s.effects &= ~(EF_COLOR_SHELL|EF_POWERSCREEN|EF_DOUBLE|EF_QUAD|EF_PENT);
	ent->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE|RF_SHELL_DOUBLE);

	if (ent->monsterinfo.aiflags & AI_RESURRECTING)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_RED;
	}

	if (ent->health <= 0)
		return;

	if (ent->powerarmor_time > level.time)
	{
		if (ent->monsterinfo.power_armor_type == POWER_ARMOR_SCREEN)
		{
			ent->s.effects |= EF_POWERSCREEN;
		}
		else if (ent->monsterinfo.power_armor_type == POWER_ARMOR_SHIELD)
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_GREEN;
		}
	}
	// PMM - new monster powerups
	if (ent->monsterinfo.quad_framenum > level.framenum)
	{
		remaining = ent->monsterinfo.quad_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			ent->s.effects |= EF_QUAD;
	}
	else
		ent->s.effects &= ~EF_QUAD;

	if (ent->monsterinfo.double_framenum > level.framenum)
	{
		remaining = ent->monsterinfo.double_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			ent->s.effects |= EF_DOUBLE;
	}
	else
		ent->s.effects &= ~EF_DOUBLE;

	if (ent->monsterinfo.invincible_framenum > level.framenum)
	{
		remaining = ent->monsterinfo.invincible_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			ent->s.effects |= EF_PENT;
	}
	else
		ent->s.effects &= ~EF_PENT;

	// PMM
	// PMM - testing
//	ent->s.effects |= EF_COLOR_SHELL;
//	ent->s.renderfx |= RF_SHELL_HALF_DAM;
/*
	if (fmod (level.time, 4.0) > 2.0)
	{
		gi.dprintf ("invulnerable ");
		ent->s.renderfx |= RF_SHELL_RED;
	}
	else
		ent->s.renderfx &= ~RF_SHELL_RED;

	if (fmod (level.time, 8.0) > 4.0)
	{
		gi.dprintf ("shield ");
		ent->s.renderfx |= RF_SHELL_GREEN;
	}
	else
		ent->s.renderfx &= ~RF_SHELL_GREEN;

	if (fmod (level.time, 16.0) > 8.0)
	{
		gi.dprintf ("quad ");
		ent->s.renderfx |= RF_SHELL_BLUE;\
	}
	else
		ent->s.renderfx &= ~RF_SHELL_BLUE;

	if (fmod (level.time, 32.0) > 16.0)
	{
		gi.dprintf ("double ");
		ent->s.renderfx |= RF_SHELL_DOUBLE;
	}
	else
		ent->s.renderfx &= ~RF_SHELL_DOUBLE;

	if (fmod (level.time, 64.0) > 32.0)
	{
		gi.dprintf ("half ");
		ent->s.renderfx |= RF_SHELL_HALF_DAM;
	}
	else
		ent->s.renderfx &= ~RF_SHELL_HALF_DAM;

	gi.dprintf ("\n");
*/
}
char	*G_CopyString (char *in);
void	stuffcmd(edict_t *ent,char *command);
float	*tv (float x, float y, float z);
char	*vtos (vec3_t v);
float vectoyaw (vec3_t vec);
void vectoangles (vec3_t vec, vec3_t angles);
qboolean point_infront (edict_t *self, vec3_t point);
void AnglesNormalize(vec3_t vec);
float SnapToEights(float x);
//ROGUE


void M_MoveFrame (edict_t *self)
{
	mmove_t	*move;
	int		index;

	// Lazarus: For live monsters weaker than the tank who aren't already running from
	//          something, evade live grenades on the ground.
/*	if ((self->health > 0) && (self->max_health < 750) && !(self->monsterinfo.aiflags & AI_CHASE_THING) && self->monsterinfo.run	// was self->max_health < 400
		&& !((Q_stricmp(self->classname, "misc_insane") == 0) && (self->moreflags & FL2_CRUCIFIED)) )	// Knightmare- crucified insanes don't evade
		Grenade_Evade (self);
*/
	move = self->monsterinfo.currentmove;
	self->nextthink = level.time + FRAMETIME;

	if ((self->monsterinfo.nextframe) && (self->monsterinfo.nextframe >= move->firstframe) && (self->monsterinfo.nextframe <= move->lastframe))
	{
		self->s.frame = self->monsterinfo.nextframe;
		self->monsterinfo.nextframe = 0;
	}
	else
	{
		if (self->s.frame == move->lastframe)
		{
			if (move->endfunc)
			{
				move->endfunc (self);

				// regrab move, endfunc is very likely to change it
				move = self->monsterinfo.currentmove;

				// check for death
				if (self->svflags & SVF_DEADMONSTER)
					return;
			}
		}

		if (self->s.frame < move->firstframe || self->s.frame > move->lastframe)
		{
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
			self->s.frame = move->firstframe;
		}
		else
		{
			if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
			{
				self->s.frame++;
				if (self->s.frame > move->lastframe)
					self->s.frame = move->firstframe;
			}
		}
	}

	index = self->s.frame - move->firstframe;

	if (move->frame[index].aifunc)
		if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
			move->frame[index].aifunc (self, move->frame[index].dist * self->monsterinfo.scale);
		else
			move->frame[index].aifunc (self, 0);

	if (move->frame[index].thinkfunc)
		move->frame[index].thinkfunc (self);
}


void monster_think (edict_t *self)
{
	M_MoveFrame (self);
	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);

	// Zaero add
	// decrease blindness
	if (self->monsterinfo.flashTime > 0)
		self->monsterinfo.flashTime--;
	// end Zaero
}

// Knightmare- for dead monsters to check
// if they've fallen into lava, etc.
void deadmonster_think (edict_t *self)
{
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);
}


/*
================
monster_use

Using a monster makes it angry at the current activator
================
*/

void monster_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->enemy)
		return;
	if (self->health <= 0)
		return;
	if (activator->flags & FL_NOTARGET)
		return;
	if (!(activator->client) && !(activator->monsterinfo.aiflags & AI_GOOD_GUY))
		return;
	if (activator->flags & FL_DISGUISED)		// PGM
		return;									// PGM

	// if monster is "used" by player, turn off good guy stuff
	if (activator->client)
	{	// Knightmare- gekks and stalkers use different spawnflag
		if (UseSpecialGoodGuyFlag(self)) {
			self->spawnflags &= ~16;
		}
		else if (UseRegularGoodGuyFlag(self)) {
			self->spawnflags &= ~SF_MONSTER_GOODGUY;
		}
		// Knightmare- don't include goodguy monsters turned bad in body count
		if (self->monsterinfo.aiflags & AI_GOOD_GUY)
			self->monsterinfo.monsterflags |= MFL_DO_NOT_COUNT;
		self->monsterinfo.aiflags &= ~(AI_GOOD_GUY + AI_FOLLOW_LEADER);
		if (self->dmgteam && !Q_stricmp(self->dmgteam,"player"))
			self->dmgteam = NULL;
	}

// delay reaction so if the monster is teleported, its sound is still heard
	self->enemy = activator;
	FoundTarget (self);
}


void monster_start_go (edict_t *self);


void monster_triggered_spawn (edict_t *self)
{
	self->s.origin[2] += 1;
	KillBox (self);

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_STEP;
	self->svflags &= ~SVF_NOCLIENT;
	self->air_finished = level.time + 12;

	// Knightmare- teleport effect for Q1 monsters
	if (self->flags & FL_Q1_MONSTER) {
#ifdef KMQUAKE2_ENGINE_MOD
		self->s.event = EV_PLAYER_TELEPORT2;
#else
		self->s.event = EV_PLAYER_TELEPORT;
		Q1TeleportSounds (self);
#endif
	}
	// end Knightmare

	// Zaero add
/*	if (level.maptype == MAPTYPE_ZAERO) {
		self->s.event = EV_PLAYER_TELEPORT;
		MonsterPlayerKillBox (self);
	}*/
	// end Zaero

	gi.linkentity (self);
	monster_start_go (self);

	if (self->enemy && !(self->spawnflags & 1) && !(self->enemy->flags & FL_NOTARGET))
	{
		if (!(self->enemy->flags & FL_DISGUISED))		// PGM
			FoundTarget (self);
		else // PMM - just in case, make sure to clear the enemy so FindTarget doesn't get confused
			self->enemy = NULL;
	}
	else
	{
		self->enemy = NULL;
	}
}

void monster_triggered_spawn_use (edict_t *self, edict_t *other, edict_t *activator)
{
	// we have a one frame delay here so we don't telefrag the guy who activated us
	self->think = monster_triggered_spawn;
	self->nextthink = level.time + FRAMETIME;

	// Knightmare- good guy monsters shouldn't have an enemy from this
	if (activator->client && !(self->monsterinfo.aiflags & AI_GOOD_GUY))
		self->enemy = activator;

	// Lazarus: Add 'em up
//	if (!(self->monsterinfo.aiflags & AI_GOOD_GUY))
//		level.total_monsters++;
	self->use = monster_use;
}

void monster_triggered_start (edict_t *self)
{
	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
	self->use = monster_triggered_spawn_use;
}


/*
================
monster_death_use

When a monster dies, it fires all of its targets with the current
enemy as activator.
================
*/
void monster_death_use (edict_t *self)
{
	edict_t	*player;
	int		i;

	if (!strcmp(self->classname, "monster_turret"))
		gi.dprintf("monster_turret firing targets\n");

	self->flags &= ~(FL_FLY|FL_SWIM);
	self->monsterinfo.aiflags &= AI_GOOD_GUY;

	//Knightmare- remove powerscreen and other effects from dead monsters
	self->s.effects &= ~(EF_COLOR_SHELL|EF_POWERSCREEN|EF_DOUBLE|EF_QUAD|EF_PENT);
	self->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE|RF_SHELL_DOUBLE);

	// Lazarus: If actor/monster is being used as a camera by a player,
	// turn camera off for that player
	for(i=0,player=g_edicts+1; i<maxclients->value; i++, player++)
	{
		if (player->client && player->client->spycam == self)
			camera_off(player);
	}

	if (self->item)
	{
		Drop_Item (self, self->item);
		self->item = NULL;
	}

	if (self->deathtarget)
		self->target = self->deathtarget;

	if (!self->target)
		return;

	G_UseTargets (self, self->enemy);
}


//============================================================================

qboolean monster_start (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return false;
	}

	// Lazarus: Already gibbed monsters passed across levels via trigger_transition:
	if ( (self->max_health > 0) && (self->health <= self->gib_health) && !(self->spawnflags & SF_MONSTER_NOGIB) )
	{
		void	SP_gibhead(edict_t *);

		SP_gibhead(self);
		return true;
	}

	// Lazarus: Good guys
	// Knightmare- gekks and stalkers use different spawnflag
	if ( (UseRegularGoodGuyFlag(self) && (self->spawnflags & SF_MONSTER_GOODGUY))	
		|| (UseSpecialGoodGuyFlag(self) && (self->spawnflags & 16)) )
	{
		self->monsterinfo.aiflags |= AI_GOOD_GUY;
		if (!self->dmgteam)
		{
			size_t	dmgSize = 8*sizeof(char);
			self->dmgteam = gi.TagMalloc(dmgSize, TAG_LEVEL);
			Com_strcpy (self->dmgteam, dmgSize, "player");
		}
	}

	// Lazarus: Max range for sight/attack
	if (self->distance)
		st.distance = self->distance;
	if (st.distance)
		self->monsterinfo.max_range = max(500,st.distance);
	else
		self->monsterinfo.max_range = 1280;	// Q2 default is 1000. We're mean.

	// Lazarus: We keep SIGHT to mean what old AMBUSH does, and AMBUSH additionally
	//          now means don't play idle sounds
/*	if ((self->spawnflags & MONSTER_SIGHT) && !(self->monsterinfo.aiflags & AI_GOOD_GUY))
	{
		self->spawnflags &= ~MONSTER_SIGHT;
		self->spawnflags |= MONSTER_AMBUSH;
	} */
	if ( (self->spawnflags & SF_MONSTER_AMBUSH) && !(self->monsterinfo.aiflags & AI_GOOD_GUY) )
		self->spawnflags |= SF_MONSTER_SIGHT;

	// Zaero- spawnflag 16 = do not count
	//	if ( !(self->monsterinfo.aiflags & AI_GOOD_GUY) && !(self->monsterinfo.aiflags & AI_DO_NOT_COUNT) )
	//	if ( !(self->monsterinfo.aiflags & AI_GOOD_GUY) && !(self->monsterinfo.monsterflags & MFL_DO_NOT_COUNT)/* && !(self->spawnflags & SF_MONSTER_TRIGGER_SPAWN*/ )
	// Zaero- spawnflag 16 = do not count
	if ( !(self->monsterinfo.aiflags & AI_GOOD_GUY) && !(self->monsterinfo.monsterflags & MFL_DO_NOT_COUNT) && !( (level.maptype == MAPTYPE_ZAERO) && (self->spawnflags & 16) ) )
		level.total_monsters++;

	self->nextthink = level.time + FRAMETIME;
	self->svflags |= SVF_MONSTER;
	self->s.renderfx |= RF_FRAMELERP;
	self->takedamage = DAMAGE_AIM;
	self->air_finished = level.time + 12;
	self->use = monster_use;

	// Lazarus - don't reset max_health unnecessarily
	if (!self->max_health)
		self->max_health = self->health;

	// Reset skinnum for revived monsters
	// Gekk has 2 pain skins, requires special handling
	if ( strcmp(self->classname, "monster_gekk") == 0 )
	{
		if (self->health < (self->max_health /4))
		{
			if (self->style)
				self->s.skinnum = self->style * 3 + 2;
			else
				self->s.skinnum = 2;
		}
		else if (self->health < (self->max_health / 2))
		{
			if (self->style)
				self->s.skinnum = self->style * 3 + 1;
			else
				self->s.skinnum = 1;
		}
		else
		{
			if (self->style)
				self->s.skinnum = self->style * 3;
			else
				self->s.skinnum = 0;
		}
	}
	// Exclude fixbot, vulture, and Q1 monsters, as they have no pain skins
	else if ( (strcmp(self->classname, "monster_fixbot") != 0) && (strcmp(self->classname, "monster_vulture") != 0)
			&& !(self->flags & FL_Q1_MONSTER) )
	{
		if (self->health < (self->max_health / 2))
			self->s.skinnum |= 1;
		else
			self->s.skinnum &= ~1;
	}
	// catch negative skinnum
	if (self->s.skinnum < 1)
		self->s.skinnum = 0;

	self->clipmask = MASK_MONSTERSOLID;
	self->deadflag = DEAD_NO;
	self->svflags &= ~SVF_DEADMONSTER;

	if (self->monsterinfo.flies > 1.0)
	{
		self->s.effects |= EF_FLIES;
		self->s.sound = gi.soundindex ("infantry/inflies1.wav");
	}

	// Lazarus
	if (self->health <= 0)
	{
		self->svflags |= SVF_DEADMONSTER;
		self->movetype = MOVETYPE_TOSS;
		self->takedamage = DAMAGE_YES;
		self->monsterinfo.pausetime = 100000000;
		self->monsterinfo.aiflags &= ~AI_RESPAWN_FINDPLAYER;
		if (self->max_health > 0)
		{
			// This must be a dead monster who changed levels
			// via trigger_transition
			self->nextthink = 0;
			self->deadflag = DEAD_DEAD;
		}
		if (self->s.effects & EF_FLIES && self->monsterinfo.flies <= 1.0)
		{
			self->think = M_FliesOff;
			self->nextthink = level.time + 1 + random()*60;
		}
		return true;
	}
	else
	{
		// make sure red shell is turned off in case medic got confused:
		self->monsterinfo.aiflags &= ~AI_RESURRECTING;
		self->svflags &= ~SVF_DEADMONSTER;
		self->takedamage = DAMAGE_AIM;
	}

	if (!self->monsterinfo.checkattack)
		self->monsterinfo.checkattack = M_CheckAttack;
	VectorCopy (self->s.origin, self->s.old_origin);

	if (st.item)
	{
		self->item = FindItemByClassname (st.item);
		if (!self->item)
			gi.dprintf("%s at %s has bad item: %s\n", self->classname, vtos(self->s.origin), st.item);
	}

	// randomize what frame they start on
	if (self->monsterinfo.currentmove)
		self->s.frame = self->monsterinfo.currentmove->firstframe + (rand() % (self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1));

	// PMM - get this so I don't have to do it in all of the monsters
	self->monsterinfo.base_height = self->maxs[2];

	// PMM - clear these
	self->monsterinfo.quad_framenum = 0;
	self->monsterinfo.double_framenum = 0;
	self->monsterinfo.invincible_framenum = 0;

	return true;
}

void monster_start_go (edict_t *self)
{
	vec3_t	v;

	if (self->health <= 0)
	{
		if (self->max_health <= 0)
			InitiallyDead(self);
		return;
	}
	
	// Lazarus: move_origin for func_monitor
	if (!VectorLength(self->move_origin))
		VectorSet(self->move_origin, 0, 0, self->viewheight);

	// check for target to combat_point and change to combattarget
	if (self->target)
	{
		qboolean	notcombat;
		qboolean	fixup;
		edict_t		*target;

		target = NULL;
		notcombat = false;
		fixup = false;
		while ((target = G_Find (target, FOFS(targetname), self->target)) != NULL)
		{
			if (strcmp(target->classname, "point_combat") == 0)
			{
				self->combattarget = self->target;
				fixup = true;
			}
			else
			{
				notcombat = true;
			}
		}
		if (notcombat && self->combattarget)
			gi.dprintf("%s at %s has target with mixed types\n", self->classname, vtos(self->s.origin));
		if (fixup)
			self->target = NULL;
	}

	// validate combattarget
	if (self->combattarget)
	{
		edict_t		*target;

		target = NULL;
		while ((target = G_Find (target, FOFS(targetname), self->combattarget)) != NULL)
		{
			if (strcmp(target->classname, "point_combat") != 0)
			{
				gi.dprintf("%s at (%i %i %i) has a bad combattarget %s : %s at (%i %i %i)\n",
					self->classname, (int)self->s.origin[0], (int)self->s.origin[1], (int)self->s.origin[2],
					self->combattarget, target->classname, (int)target->s.origin[0], (int)target->s.origin[1],
					(int)target->s.origin[2]);
			}
		}
	}

	if (self->target)
	{
		self->goalentity = self->movetarget = G_PickTarget(self->target);
		if (!self->movetarget)
		{
			gi.dprintf ("%s can't find target %s at %s\n", self->classname, self->target, vtos(self->s.origin));
			self->target = NULL;
			self->monsterinfo.pausetime = 100000000;
			self->monsterinfo.stand (self);
		}
		else if (strcmp (self->movetarget->classname, "path_corner") == 0)
		{
			// Lazarus: Don't wipe out target for trigger spawned monsters
			// that aren't triggered yet
			if ( !(self->spawnflags & MONSTER_TRIGGER_SPAWN) )
			{
				VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
				self->ideal_yaw = self->s.angles[YAW] = vectoyaw(v);
				self->monsterinfo.walk (self);
				self->target = NULL;
			}
		}
		else
		{
			self->goalentity = self->movetarget = NULL;
			self->monsterinfo.pausetime = 100000000;
			self->monsterinfo.stand (self);
		}
	}
	else
	{
		self->monsterinfo.pausetime = 100000000;
	}

	self->think = monster_think;
	self->nextthink = level.time + FRAMETIME;
}


void walkmonster_start_go (edict_t *self)
{
	if (!(self->spawnflags & 2) && level.time < 1)
	{
		M_droptofloor (self);

		if (self->groundentity)
			if (!M_walkmove (self, 0, 0))
				gi.dprintf ("%s in solid at %s\n", self->classname, vtos(self->s.origin));
	}

	if (!self->yaw_speed)
		self->yaw_speed = 20;
	// PMM - stalkers are too short for this
	if (strcmp(self->classname, "monster_stalker") == 0)
		self->viewheight = 15;
	// Knightmare- vultures are also too short
	else if (strcmp(self->classname, "monster_vulture") == 0)
		self->viewheight = 8;
	else
		self->viewheight = 25;

	monster_start_go (self);

	if (self->spawnflags & MONSTER_TRIGGER_SPAWN)
		monster_triggered_start (self);
}

void walkmonster_start (edict_t *self)
{
	self->think = walkmonster_start_go;
	monster_start (self);
}


void flymonster_start_go (edict_t *self)
{
	if (!M_walkmove (self, 0, 0))
		gi.dprintf ("%s in solid at %s\n", self->classname, vtos(self->s.origin));

	if (!self->yaw_speed)
		self->yaw_speed = 10;
	// Knightmare- vultures are too short for this
	if (strcmp(self->classname, "monster_vulture") == 0)
		self->viewheight = 8;
	else
		self->viewheight = 25;

	monster_start_go (self);

	if (self->spawnflags & MONSTER_TRIGGER_SPAWN)
		monster_triggered_start (self);
}


void flymonster_start (edict_t *self)
{
	self->flags |= FL_FLY;
	self->think = flymonster_start_go;
	monster_start (self);
}


void swimmonster_start_go (edict_t *self)
{
	if (!self->yaw_speed)
		self->yaw_speed = 10;
	self->viewheight = 10;

	monster_start_go (self);

	if (self->spawnflags & MONSTER_TRIGGER_SPAWN)
		monster_triggered_start (self);
}

void swimmonster_start (edict_t *self)
{
	self->flags |= FL_SWIM;
	self->think = swimmonster_start_go;
	monster_start (self);
}

//ROGUE

void stationarymonster_start_go (edict_t *self);

void stationarymonster_triggered_spawn (edict_t *self)
{
	KillBox (self);

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_NONE;
	self->svflags &= ~SVF_NOCLIENT;
	self->air_finished = level.time + 12;
	gi.linkentity (self);

	// FIXME - why doesn't this happen with real monsters?
	self->spawnflags &= ~2;

	stationarymonster_start_go (self);

	if (self->enemy && !(self->spawnflags & 1) && !(self->enemy->flags & FL_NOTARGET))
	{
		if (!(self->enemy->flags & FL_DISGUISED))		// PGM
			FoundTarget (self);
		else // PMM - just in case, make sure to clear the enemy so FindTarget doesn't get confused
			self->enemy = NULL;
	}
	else
	{
		self->enemy = NULL;
	}
}

void stationarymonster_triggered_spawn_use (edict_t *self, edict_t *other, edict_t *activator)
{
	// we have a one frame delay here so we don't telefrag the guy who activated us
	self->think = stationarymonster_triggered_spawn;
	self->nextthink = level.time + FRAMETIME;
	// Knightmare- good guy monsters shouldn't have an enemy from this
	if (activator->client && !(self->monsterinfo.aiflags & AI_GOOD_GUY))
		self->enemy = activator;
	// Lazarus: Add 'em up
//	if (!(self->monsterinfo.aiflags & AI_GOOD_GUY))
//		level.total_monsters++;
	self->use = monster_use;
}

void stationarymonster_triggered_start (edict_t *self)
{
	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
	self->use = stationarymonster_triggered_spawn_use;
}

void stationarymonster_start_go (edict_t *self)
{
// PGM - only turrets use this, so remove the error message. They're supposed to be in solid.

//	if (!M_walkmove (self, 0, 0))
//		gi.dprintf ("%s in solid at %s\n", self->classname, vtos(self->s.origin));

	if (!self->yaw_speed)
		self->yaw_speed = 20;
//	self->viewheight = 25;

	monster_start_go (self);

	if (self->spawnflags & MONSTER_TRIGGER_SPAWN)
		stationarymonster_triggered_start (self);
}

void stationarymonster_start (edict_t *self)
{
	self->think = stationarymonster_start_go;
	monster_start (self);
}

void monster_done_dodge (edict_t *self)
{
//	if ((g_showlogic) && (g_showlogic->value))
//		gi.dprintf ("%s done dodging\n", self->classname);
	self->monsterinfo.aiflags &= ~AI_DODGING;
}

// Following functions unique to Lazarus

void InitiallyDead (edict_t *self)
{
	int	damage;

	if (self->max_health > 0)
		return;

//	gi.dprintf("InitiallyDead on %s at %s\n",self->classname,vtos(self->s.origin));
	
	// initially dead bad guys shouldn't count against totals
	if ((self->max_health <= 0) && !(self->monsterinfo.aiflags & AI_GOOD_GUY))
	{
		level.total_monsters--;
		if (self->deadflag != DEAD_DEAD)
			level.killed_monsters--;
	}
	if (self->deadflag != DEAD_DEAD)
	{
		damage = 1 - self->health;
		self->health = 1;
		T_Damage (self, world, world, vec3_origin, self->s.origin, vec3_origin, damage, 0, DAMAGE_NO_ARMOR, 0);
		if (self->svflags & SVF_MONSTER)
		{
			self->svflags |= SVF_DEADMONSTER;
			self->think = monster_think;
			self->nextthink = level.time + FRAMETIME;
		}
	}
	gi.linkentity(self);
}

#define MAX_SKINS		32 // max is 32, and we need all of them!
#define MAX_SKINNAME	64

#include "pak.h"

int PatchMonsterModel (char *modelname)
{
	cvar_t		*gamedir;
	int			j;
	int			numskins;			// number of skin entries
	char		skins[MAX_SKINS][MAX_SKINNAME];	// skin entries
	char		infilename[MAX_OSPATH];
	char		outfilename[MAX_OSPATH];
	char		tempname[MAX_OSPATH];
	char		*p;
	FILE		*infile;
	FILE		*outfile;
	dmdl_t		model;				// model header
	byte		*data;				// model data
	int			datasize;			// model data size (bytes)
	int			newoffset;			// model data offset (after skins)
	qboolean	is_tank = false;
	qboolean	is_soldier = false;
	// Knightmare added
	qboolean	is_brain = false;
	qboolean	is_gekk = false;
	qboolean	is_fixbot = false;
	qboolean	is_chick = false;
	qboolean	is_gunner = false;
	qboolean	is_soldierh = false;
	qboolean	is_carrier = false;
	qboolean	is_hover = false;
	qboolean	is_medic = false;
	qboolean	is_turret = false;
	qboolean	is_vulture = false;
	qboolean	is_zboss_mech = false;
	qboolean	is_zboss_pilot = false;
	qboolean	is_q1monster = false;

	qboolean	gamedirpakfile = false;

	// get game (moddir) name
	gamedir = gi.cvar("game", "", 0);
	if (!*gamedir->string)
		return 0;	// we're in baseq2

//	Com_sprintf (outfilename, sizeof(outfilename), "%s/%s", gamedir->string, modelname);
	Com_sprintf (tempname, sizeof(tempname), modelname);
	SavegameDirRelativePath (tempname, outfilename, sizeof(outfilename));
	if (outfile = fopen (outfilename, "rb"))
	{
		// output file already exists, move along
		fclose (outfile);
	//	gi.dprintf ("PatchMonsterModel: Could not save %s, file already exists\n", outfilename);
		return 0;
	}


	numskins = 8;
	// special cases
	if (!strcmp(modelname, "models/monsters/tank/tris.md2"))
	{
		is_tank = true;
		numskins = 16;
	}
	else if (!strcmp(modelname, "models/monsters/soldier/tris.md2"))
	{
		is_soldier = true;
		numskins = 32;	// was 24
	}
	// Knightmare added
#ifdef CITADELMOD_FEATURES
	else if (!strcmp(modelname, "models/monsters/brain/tris.md2"))
	{
		is_brain = true;
		numskins = 16;
	}
#endif
	else if (!strcmp(modelname, "models/monsters/gekk/tris.md2"))
	{
		is_gekk = true;
		numskins = 12;
	}
	else if (!strcmp(modelname, "models/monsters/fixbot/tris.md2"))
	{
		is_fixbot = true;
		numskins = 4;
	}
	else if (!strcmp(modelname, "models/monsters/bitch/tris.md2")
		|| !strcmp(modelname, "models/monsters/bitch2/tris.md2"))
	{
		is_chick = true;
		numskins = 16;
	}
	else if (!strcmp(modelname, "models/monsters/gunner/tris.md2"))
	{
		is_gunner = true;
		numskins = 16;
	}
	else if (!strcmp(modelname, "models/monsters/soldierh/tris.md2"))
	{
		is_soldierh = true;
		numskins = 24;
	}
	else if (!strcmp(modelname, "models/monsters/carrier/tris.md2"))
	{
		is_carrier = true;
		numskins = 8;
	}
	else if (!strcmp(modelname, "models/monsters/hover/tris.md2"))
	{
		is_hover = true;
		numskins = 16;
	}
	else if (!strcmp(modelname, "models/monsters/medic/tris.md2"))
	{
		is_medic = true;
		numskins = 16;
	}
	else if (!strcmp(modelname, "models/monsters/turret/tris.md2"))
	{
		is_turret = true;
		numskins = 12;
	}
	else if (!strcmp(modelname, "models/monsters/vulture/tris.md2"))
	{
		is_vulture = true;
		numskins = 4;
	}
	else if (!strcmp(modelname, "models/monsters/bossz/mech/tris.md2"))
	{
		is_zboss_mech = true;
		numskins = 12;
	}
	else if (!strcmp(modelname, "models/monsters/bossz/pilot/tris.md2"))
	{
		is_zboss_pilot = true;
		numskins = 12;
	}
	else if ( !strcmp(modelname, "models/monsters/q1dog/tris.md2") ||  !strcmp(modelname, "models/monsters/q1enforcer/tris.md2")
			|| !strcmp(modelname, "models/monsters/q1fiend/tris.md2") || !strcmp(modelname, "models/monsters/q1freddie/tris.md2")
			|| !strcmp(modelname, "models/monsters/q1grunt/tris.md2") || !strcmp(modelname, "models/monsters/q1hknight/tris.md2")
			|| !strcmp(modelname, "models/monsters/q1knight/tris.md2") || !strcmp(modelname, "models/monsters/q1ogre/tris.md2")
			|| !strcmp(modelname, "models/monsters/q1rotfish/tris.md2") || !strcmp(modelname, "models/monsters/q1scrag/tris.md2")
			|| !strcmp(modelname, "models/monsters/q1shalrath/tris.md2") || !strcmp(modelname, "models/monsters/q1shambler/tris.md2")
			|| !strcmp(modelname, "models/monsters/q1tarbaby/tris.md2") || !strcmp(modelname, "models/monsters/q1zombie/tris.md2") )
	{
		is_q1monster = true;
		numskins = 4;
	}

	// end Knightmare

	for (j=0; j<numskins; j++)
	{
		memset (skins[j], 0, MAX_SKINNAME);
		Com_strcpy( skins[j], sizeof(skins[j]), modelname );
		p = strstr( skins[j], "tris.md2" );
		if (!p)
		{
			fclose (outfile);
			gi.dprintf( "Error patching %s\n",modelname);
			return 0;
		}
		*p = 0;
		if (is_soldier)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin_lt.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "skin_ltp.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "skin_ss.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "skin_ssp.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "skin_pl.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "skin_plp.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_lt.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1_lt.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_ss.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1_ss.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_pl.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1_pl.pcx"); break;
			case 16:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_lt.pcx"); break;
			case 17:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2_lt.pcx"); break;
			case 18:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 19:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 20:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_ss.pcx"); break;
			case 21:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2_ss.pcx"); break;
			case 22:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_pl.pcx"); break;
			case 23:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2_pl.pcx"); break;
			case 24:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_lt.pcx"); break;
			case 25:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3_lt.pcx"); break;
			case 26:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 27:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 28:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_ss.pcx"); break;
			case 29:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3_ss.pcx"); break;
			case 30:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_pl.pcx"); break;
			case 31:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3_pl.pcx"); break;
			}
		}
		else if (is_tank)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/skin.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/pain.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/custom1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/custompain1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/custom2.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/custompain2.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/custom3.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "../ctank/custompain3.pcx"); break;
			}
		}
		// Knightmare added
#ifdef CITADELMOD_FEATURES
		else if (is_brain)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "beta.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "betapain.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta2.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p2.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta3.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p3.pcx"); break;
			}
		}
#endif
		else if (is_gekk)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "gekk.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "gekpain1.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "gekpain2.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1pain1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1pain2.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2pain1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2pain2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3pain1.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3pain2.pcx"); break;
			}
		}
		else if (is_fixbot)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "droid.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			}
		}
		else if (is_chick)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Com_strcpy (skins[j], sizeof(skins[j]), "models/monsters/bitch/bi_sk3.pcx"); break;
			case 3:
				Com_strcpy (skins[j], sizeof(skins[j]), "models/monsters/bitch/bi_pain.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta2.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p2.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta3.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p3.pcx"); break;
			}
		}
		else if (is_gunner)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Com_strcpy (skins[j], sizeof(skins[j]), "tact_skin.pcx"); break;
			case 3:
				Com_strcpy (skins[j], sizeof(skins[j]), "tact_pain.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta2.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p2.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta3.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p3.pcx"); break;
			}
		}
		else if (is_soldierh)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "sold01.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "sold01_p.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "sold02.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "sold02_p.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "sold03.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "sold03_p.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1sold1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1sold1_p.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1sold2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1sold2_p.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1sold3.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1sold3_p.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2sold1.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2sold1_p.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2sold2.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2sold2_p.pcx"); break;
			case 16:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2sold3.pcx"); break;
			case 17:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2sold3_p.pcx"); break;
			case 18:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3sold1.pcx"); break;
			case 19:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3sold1_p.pcx"); break;
			case 20:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3sold2.pcx"); break;
			case 21:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3sold2_p.pcx"); break;
			case 22:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3sold3.pcx"); break;
			case 23:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3sold3_p.pcx"); break;
			}
		}
		else if (is_carrier)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain2.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			}
		}
		else if (is_hover || is_medic)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "rskin.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "rpain.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta2.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p2.pcx"); break;
			case 12:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 13:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			case 14:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta2.pcx"); break;
			case 15:
				Com_strcat (skins[j], sizeof(skins[j]), "custombeta_p2.pcx"); break;
			}
		}
		else if (is_turret)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "skin1.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "skin2.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_1.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_2.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_3.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_1.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_2.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_3.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_1.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_2.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_3.pcx"); break;
			}
		}
		else if (is_vulture)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "vulture.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			}
		}
		else if (is_zboss_mech || is_zboss_pilot)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain1.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "pain2.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_p1.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1_p2.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_p1.pcx"); break;
			case 8:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2_p2.pcx"); break;
			case 9:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 10:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_p1.pcx"); break;
			case 11:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3_p2.pcx"); break;
			}
		}
		else if (is_q1monster)
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin0.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			}
		}
		// end Knightmare
		else
		{
			switch (j)
			{
			case 0:
				Com_strcat (skins[j], sizeof(skins[j]), "skin.pcx"); break;
			case 1:
				Com_strcat (skins[j], sizeof(skins[j]), "pain.pcx"); break;
			case 2:
				Com_strcat (skins[j], sizeof(skins[j]), "custom1.pcx"); break;
			case 3:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain1.pcx"); break;
			case 4:
				Com_strcat (skins[j], sizeof(skins[j]), "custom2.pcx"); break;
			case 5:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain2.pcx"); break;
			case 6:
				Com_strcat (skins[j], sizeof(skins[j]), "custom3.pcx"); break;
			case 7:
				Com_strcat (skins[j], sizeof(skins[j]), "custompain3.pcx"); break;
			}
		}
	}

	// load original model from baseq2
	Com_sprintf (infilename, sizeof(infilename), "baseq2/%s", modelname);
	// Knightmare- if not in baseq2, look in current gamedir
	// can't do this- the output file already exists
	//if ( !(infile = fopen (infilename, "rb")) )
	//	GameDirRelativePath (modelname, infilename, sizeof(infilename));

	// If file doesn't exist on user's hard disk, it must be in 
	// a pak file
	if ( !(infile = fopen (infilename, "rb")) )
	{
		pak_header_t	pakheader;
		pak_item_t		pakitem;
		FILE			*fpak;
		int				i, k, numitems;
		char			pakfile[MAX_OSPATH];

		// Knightmare- look in all pak files in baseq2
		for (i=0; i<10; i++)
		{
			Com_sprintf (pakfile, sizeof(pakfile), "baseq2/pak%i.pak", i);
			fpak = fopen(pakfile, "rb");
			if (!fpak && (i == 0)) // if pak0.pak isn't on hard disk, try CD
			{
				cvar_t	*cddir;

				cddir = gi.cvar("cddir", "", 0);
				Com_sprintf(pakfile, sizeof(pakfile), "%s/baseq2/pak0.pak",cddir->string);
				fpak = fopen(pakfile,"rb");
				if (!fpak)
				{
					gi.dprintf("PatchMonsterModel: Cannot find pak0.pak on CD\n");
				//	return 0;
					continue;
				}
			}
			else if (!fpak) // this pak not found, go on to next
				continue;
			fread(&pakheader,1,sizeof(pak_header_t),fpak);
			numitems = pakheader.dsize/sizeof(pak_item_t);
			fseek(fpak,pakheader.dstart,SEEK_SET);
			data = NULL;
			for (k=0; k<numitems && !data; k++)
			{
				fread(&pakitem,1,sizeof(pak_item_t),fpak);
				if (!Q_stricmp(pakitem.name,modelname))
				{
					fseek(fpak,pakitem.start,SEEK_SET);
					fread(&model, sizeof(dmdl_t), 1, fpak);
					datasize = model.ofs_end - model.ofs_skins;
					if ( !(data = malloc (datasize)) )	// make sure freed locally
					{
						fclose(fpak);
						gi.dprintf ("PatchMonsterModel: Could not allocate memory for model\n");
						return 0;
					}
					fread (data, sizeof (byte), datasize, fpak);
				}
			}
			fclose(fpak);
			if (data) // we found it, so stop searching
				break;
		}
		if (!data) // if not in baseq2 pak file, check pakfiles in current gamedir
		{
			char		pakname[MAX_OSPATH];

			// check all pakfiles in current gamedir
			for (i=0; i<10; i++)
			{
				Com_sprintf (pakname, sizeof(pakname), "pak%i.pak", i);
				GameDirRelativePath (pakname, pakfile, sizeof(pakfile));
				fpak = fopen(pakfile,"rb");
				if (!fpak) // this pak not found, go on to next
					continue;
				fread(&pakheader,1,sizeof(pak_header_t),fpak);
				numitems = pakheader.dsize/sizeof(pak_item_t);
				fseek(fpak,pakheader.dstart,SEEK_SET);
				data = NULL;
				for (k=0; k<numitems && !data; k++)
				{
					fread(&pakitem,1,sizeof(pak_item_t),fpak);
					if (!Q_stricmp(pakitem.name,modelname))
					{
						fseek(fpak,pakitem.start,SEEK_SET);
						fread(&model, sizeof(dmdl_t), 1, fpak);
						datasize = model.ofs_end - model.ofs_skins;
						if ( !(data = malloc (datasize)) )	// make sure freed locally
						{
							fclose(fpak);
							gi.dprintf ("PatchMonsterModel: Could not allocate memory for model\n");
							return 0;
						}
						fread (data, sizeof (byte), datasize, fpak);
					}
				}
				fclose(fpak);
				if (data) // we found it, so stop searching
				{
					gamedirpakfile = true;
					break;
				}
			}
			if (!data)
			{
				gi.dprintf("PatchMonsterModel: Could not find %s in pak file(s)\n",modelname);
				return 0;
			}
		}
	}
	else
	{
		fread (&model, sizeof (dmdl_t), 1, infile);
	
		datasize = model.ofs_end - model.ofs_skins;
		if ( !(data = malloc (datasize)) )	// make sure freed locally
		{
			gi.dprintf ("PatchMonsterModel: Could not allocate memory for model\n");
			return 0;
		}
		fread (data, sizeof (byte), datasize, infile);
	
		fclose (infile);
	}
	
	// update model info
	model.num_skins = numskins;
	
	newoffset = numskins * MAX_SKINNAME;
	model.ofs_st     += newoffset;
	model.ofs_tris   += newoffset;
	model.ofs_frames += newoffset;
	model.ofs_glcmds += newoffset;
	model.ofs_end    += newoffset;
	
	// save new model
/*	Com_sprintf (outfilename, sizeof(outfilename), "%s/models", gamedir->string);	// make some dirs if needed
	_mkdir (outfilename);
	Com_strcat (outfilename, sizeof(outfilename), "/monsters");
	_mkdir (outfilename);
	Com_sprintf (outfilename, sizeof(outfilename), "%s/%s", gamedir->string, modelname);
	p = strstr(outfilename,"/tris.md2");
	*p = 0;
	_mkdir (outfilename);
	Com_sprintf (outfilename, sizeof(outfilename), "%s/%s", gamedir->string, modelname);
*/	
	Com_sprintf (tempname, sizeof(tempname), modelname);
	SavegameDirRelativePath (tempname, outfilename, sizeof(outfilename));
	CreatePath (outfilename);

	if ( !(outfile = fopen (outfilename, "wb")) )
	{
		// file couldn't be created for some other reason
		gi.dprintf ("PatchMonsterModel: Could not save %s\n", outfilename);
		free (data);
		return 0;
	}
	
	fwrite (&model, sizeof (dmdl_t), 1, outfile);
	fwrite (skins, sizeof (char), newoffset, outfile);
	fwrite (data, sizeof (byte), datasize, outfile);
	
	fclose (outfile);

	// Knightmare- if we loaded the model from a pak file in the same gamedir,
	// then we need to insert it into a higher-numbered pakfile, otherwise it won't be loaded

	gi.dprintf ("PatchMonsterModel: Saved %s\n", outfilename);
	free (data);
	return 1;
}


void HintTestNext (edict_t *self, edict_t *hint)
{
	edict_t		*next=NULL;
	edict_t		*e;
	vec3_t		dir;

	self->monsterinfo.aiflags2 &= ~AI2_HINT_TEST;
	if (self->goalentity == hint)
		self->goalentity = NULL;
	if (self->movetarget == hint)
		self->movetarget = NULL;
	if (self->monsterinfo.pathdir == 1)
	{
		if (hint->hint_chain)
			next = hint->hint_chain;
		else
			self->monsterinfo.pathdir = -1;
	}
	if (self->monsterinfo.pathdir == -1)
	{
	//	e = hint_chain_starts[hint->hint_chain_id];
		e = hint_path_start[hint->hint_chain_id];
		while (e)
		{
			if (e->hint_chain == hint)
			{
				next = e;
				break;
			}
			e = e->hint_chain;
		}
	}
	if (!next)
	{
		self->monsterinfo.pathdir = 1;
		next = hint->hint_chain;
	}
	if (next)
	{
		self->hint_chain_id = next->hint_chain_id;
		VectorSubtract(next->s.origin, self->s.origin, dir);
		self->ideal_yaw = vectoyaw(dir);
		self->goalentity = self->movetarget = next;
		self->monsterinfo.pausetime = 0;
		self->monsterinfo.aiflags2 |= AI2_HINT_TEST;
		// run for it
		self->monsterinfo.run (self);
		gi.dprintf("%s (%s): Reached hint_path %s,\nsearching for hint_path %s at %s. %s\n",
			self->classname, (self->targetname ? self->targetname : "<noname>"),
			(hint->targetname ? hint->targetname : "<noname>"),
			(next->targetname ? next->targetname : "<noname>"),
			vtos(next->s.origin),
			(visible(self,next) ? "I see it." : "I don't see it."));
	}
	else
	{
		self->monsterinfo.pausetime = level.time + 100000000;
		self->monsterinfo.stand (self);
		gi.dprintf("%s (%s): Error finding next/previous hint_path from %s at %s.\n",
			self->classname, (self->targetname ? self->targetname : "<noname>"),
			(hint->targetname ? hint->targetname : "<noname>"),
			vtos(hint->s.origin));
	}
}


int HintTestStart (edict_t *self)
{
	edict_t	*e;
	edict_t	*hint=NULL;
	float	dist;
	vec3_t	dir;
	int		i;
	float	bestdistance=99999;

//	if (!hint_chains_exist)
	if (!hint_paths_present)
		return 0;

	for (i=game.maxclients+1; i<globals.num_edicts; i++)
	{
			e = &g_edicts[i];
			if (!e->inuse)
				continue;
			if (Q_stricmp(e->classname, "hint_path"))
				continue;
			if (!visible(self, e))
				continue;
			if (!canReach(self, e))
				continue;
			VectorSubtract (e->s.origin, self->s.origin, dir);
			dist = VectorLength(dir);
			if (dist < bestdistance)
			{
				hint = e;
				bestdistance = dist;
			}
	}
	if (hint)
	{
		self->hint_chain_id = hint->hint_chain_id;
		if (!self->monsterinfo.pathdir)
			self->monsterinfo.pathdir = 1;
		VectorSubtract(hint->s.origin, self->s.origin, dir);
		self->ideal_yaw = vectoyaw(dir);
		self->enemy = self->oldenemy = NULL;
		self->goalentity = self->movetarget = hint;
		self->monsterinfo.pausetime = 0;
		self->monsterinfo.aiflags2 |= AI2_HINT_TEST;
		// run for it
		self->monsterinfo.run (self);
		return 1;
	}
	else
		return -1;
}
