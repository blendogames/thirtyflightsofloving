#include "g_local.h"


//
// monster weapons
//

//FIXME mosnters should call these with a totally accurate direction
// and we can mess it up based on skill.  Spread should be for normal
// and we can tighten or loosen based on skill.  We could muck with
// the damages too, but I'm not sure that's such a good idea.
void monster_fire_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int kick, int hspread, int vspread, int flashtype, int mod)
{
	fire_bullet (self, start, dir, damage, kick, hspread, vspread, mod);

	if (self->map && (flashtype != MZ2_ACTOR_MACHINEGUN_1))	// bot
		gi.WriteByte (svc_muzzleflash);
	else
		gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if ( !self->map && (flashtype >= MZ2_SEND_SHORT) ) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

void monster_fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int flashtype, int mod)
{
	fire_shotgun (self, start, aimdir, damage, kick, hspread, vspread, count, mod);

	if (self->map)	// bot
		gi.WriteByte (svc_muzzleflash);
	else
		gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if ( !self->map && (flashtype >= MZ2_SEND_SHORT) ) {
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
	fire_blaster (self, start, dir, damage, speed, effect, false, color);

	if (self->map)	// bot
		gi.WriteByte (svc_muzzleflash);
	else
		gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if ( !self->map && (flashtype >= MZ2_SEND_SHORT) ) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}	

void monster_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int flashtype)
{
	fire_grenade (self, start, aimdir, damage, speed, 2.5, damage+40, false);

	if (self->map)	// bot
		gi.WriteByte (svc_muzzleflash);
	else
		gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if ( !self->map && (flashtype >= MZ2_SEND_SHORT) ) {
		gi.WriteByte (MZ2_SEND_SHORT);
		gi.WriteShort (flashtype);
		gi.WriteByte (0);
	}
	else
#endif
		gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

void monster_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype)
{
	fire_rocket (self, start, dir, damage, speed, damage+20, damage);

	if (self->map)	// bot
		gi.WriteByte (svc_muzzleflash);
	else
		gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if ( !self->map && (flashtype >= MZ2_SEND_SHORT) ) {
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
	fire_rail (self, start, aimdir, damage, kick, false, 0, 0, 0);

	if (self->map)	// bot
		gi.WriteByte (svc_muzzleflash);
	else
		gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if ( !self->map && (flashtype >= MZ2_SEND_SHORT) ) {
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
	fire_bfg (self, start, aimdir, damage, speed, damage_radius);

	if (self->map)	// bot
		gi.WriteByte (svc_muzzleflash);
	else
		gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if ( !self->map && (flashtype >= MZ2_SEND_SHORT) ) {
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
//	fire_blueblaster (self, start, dir, damage, speed, effect);
	fire_blaster (self, start, dir, damage, speed, effect, false, BLASTER_BLUE);

	if (self->map)	// bot
		gi.WriteByte (svc_muzzleflash);
	else
		gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if ( !self->map && (flashtype >= MZ2_SEND_SHORT) ) {
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
	fire_ionripper (self, start, dir, damage, speed, effect);

	if (self->map)	// bot
		gi.WriteByte (svc_muzzleflash);
	else
		gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if ( !self->map && (flashtype >= MZ2_SEND_SHORT) ) {
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
	fire_phalanx_plasma (self, start, dir, damage, speed, damage_radius, radius_damage);

	if (self->map)	// bot
		gi.WriteByte (svc_muzzleflash);
	else
		gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if ( !self->map && (flashtype >= MZ2_SEND_SHORT) ) {
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
	fire_rocket_heat (self, start, dir, damage, speed, damage, damage);

	if (self->map)	// bot
		gi.WriteByte (svc_muzzleflash);
	else
		gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if ( !self->map && (flashtype >= MZ2_SEND_SHORT) ) {
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
	
	while(1)
	{
		tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
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
void monster_dabeam (edict_t *self)
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

// SKWiD MOD
void monster_fire_plasma_rifle (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, qboolean spread)
{
	fire_plasma_rifle (self, start, dir, damage, speed, spread);

	if (self->map)	// bot
		gi.WriteByte (svc_muzzleflash);
	else
		gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
#ifdef KMQUAKE2_ENGINE_MOD
	if ( !self->map && (flashtype >= MZ2_SEND_SHORT) ) {
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

static void M_FliesOff (edict_t *self)
{
	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
}

static void M_FliesOn (edict_t *self)
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
	if (self->waterlevel)
		return;

	if (random() > 0.5)
		return;

	self->think = M_FliesOn;
	self->nextthink = level.time + 5 + 10 * random();
}

void AttackFinished (edict_t *self, float time)
{
	self->monsterinfo.attack_finished = level.time + time;
}

int	CanJump(edict_t *ent);

void M_CheckGround (edict_t *ent)
{
	vec3_t		point;
	trace_t		trace;

	if (ent->health <= 0)
		return;

	if (ent->velocity[2] > 210)
	{
		ent->groundentity = NULL;
		return;
	}

// if the hull point one-quarter unit down is solid the entity is on ground
	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
	point[2] = ent->s.origin[2] - 1;

	trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, point, ent, MASK_PLAYERSOLID);

	if ((trace.startsolid) && !(trace.ent->client) && !(trace.ent->health <= 0))
	{	// we're stuck
		// find a safe position?
//		int x, y;
		qboolean safe=false;

		ent->maxs[2] = 4;		// duck
		ent->crouch_attack_time = level.time + 2;
/*
		for (x=-16; x<=16; x+=8)
			for (y=-16; y<=16; y+=8)
			{
				if (!x && !y)
					continue;

				point[0] = ent->s.origin[0] + x;
				point[1] = ent->s.origin[1] + y;
				point[2] = ent->s.origin[2];

				trace = gi.trace(point, ent->mins, ent->maxs, point, ent, MASK_PLAYERSOLID);
				
				if (!trace.startsolid)
				{
					safe = true;
					goto done;
				}
			}
*/
		// still stuck, try moving up
		ent->s.origin[2] += 26;

		point[0] = ent->s.origin[0];
		point[1] = ent->s.origin[1];
		point[2] = ent->s.origin[2];

		trace = gi.trace(point, ent->mins, ent->maxs, point, ent, MASK_PLAYERSOLID);
		
		if (!trace.startsolid)
		{
			safe = true;
			goto done;
		}

		ent->s.origin[2] -= 26;

done:

		if (!safe)
		{
			if (ent->maxs[2] < 32)
			{
				T_Damage(ent, ent, ent, VEC_ORIGIN, ent->s.origin, VEC_ORIGIN, 100000, 0, DAMAGE_NO_PROTECTION, MOD_SUICIDE);
			}
			else	// try ducking
			{
				ent->maxs[2] = 4;
				ent->crouch_attack_time = level.time + 2;
			}
		}

		return;
	}

	// check steepness
	if ( (trace.fraction < 1) && (trace.ent == world) && (trace.plane.normal[2] <= 0.4))
	{

		VectorScale(trace.plane.normal, 300, ent->velocity);
		VectorCopy(ent->velocity, ent->jump_velocity);

//		if (CanJump(ent))
//			ent->s.origin[2] += 1;

		ent->groundentity = NULL;
		gi.linkentity(ent);

		return;

	}

	if (trace.fraction < 1)
		ent->groundentity = trace.ent;
	else
		ent->groundentity = NULL;

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
	
	if (ent->waterlevel == 0)
	{
		if (ent->flags & FL_INWATER)
		{	
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
// AJ hack to give bots less damage from lava
			if (ent->bot_client)
				T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, bot_lava_mod->value*10*ent->waterlevel, 0, 0, MOD_LAVA);
			else
				T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, lava_mod->value*10*ent->waterlevel, 0, 0, MOD_LAVA);
// end AJ
		}
	}
	if ((ent->watertype & CONTENTS_SLIME) && !(ent->flags & FL_IMMUNE_SLIME))
	{
		if (ent->damage_debounce_time < level.time)
		{
			ent->damage_debounce_time = level.time + 1;
// AJ hack to give bots less damage from slime
			if (ent->bot_client)
				T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, bot_slime_mod->value*4*ent->waterlevel, 0, 0, MOD_SLIME);
			else
				T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, slime_mod->value*4*ent->waterlevel, 0, 0, MOD_SLIME);
// end AJ
		}
	}
	
	if ( !(ent->flags & FL_INWATER) )
	{	
		if (!(ent->svflags & SVF_DEADMONSTER))
		{
			if (ent->watertype & CONTENTS_LAVA)
				if (random() <= 0.5)
					gi.sound (ent, CHAN_BODY, gi.soundindex("player/lava1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (ent, CHAN_BODY, gi.soundindex("player/lava2.wav"), 1, ATTN_NORM, 0);
			else if (ent->watertype & CONTENTS_SLIME)
				gi.sound (ent, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
			else if (ent->watertype & CONTENTS_WATER)
				gi.sound (ent, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
		}

		ent->flags |= FL_INWATER;
		ent->damage_debounce_time = 0;
	}
}


void M_droptofloor (edict_t *ent)
{
	vec3_t		end;
	trace_t		trace;

	ent->s.origin[2] += 1;
	VectorCopy (ent->s.origin, end);
	end[2] -= 256;
	
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
	ent->s.effects &= ~(EF_COLOR_SHELL|EF_POWERSCREEN);
	ent->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);

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
}


void M_MoveFrame (edict_t *self)
{
	mmove_t	*move;
	int		index;

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

extern edict_t		*current_player;
extern gclient_t	*current_client;
void P_WorldEffects (void);

void monster_think (edict_t *self)
{

	// Yes, this is a hack. This is the result of starting such a project
	// without knowing what anything does. Idealistically, bot's should have
	// their own totally unique think, and shouldn't interfer with the normal
	// monster code, but the Eraser started out as a normal monster, so I had
	// to use this code. It is possible to put back the original code and 
	// use a check for self->bot_client to disperse bot's and normal monsters.
	// That way, a coop bot would be possible (but not probable, given the
	// amount of work required).

	if (!self->bot_client)
	{
		gi.error("\nCannot play Eraser in single player mode.\n\nType \"deathmatch 1\" to play the Eraser Bot.\n\n");
		return;
	}

	if (self->last_think_time == level.time)
		return;

	if (self->client->showscores)	// must be in intermission
		return;
	if (level.intermissiontime)
		return;		// already activated

	bot_frametime = 0.1;

	// check X/Y jumping velocity, make sure we continue moving forwards when obstructed by a stair
	// or other solids before landing
	if (!self->groundentity && (self->waterlevel < 2))
	{
//		trace_t tr;
//		vec3_t	dest;

//		if (self->jump_velocity[0] && self->jump_velocity[1])
//		{
			self->velocity[0] = self->jump_velocity[0];
			self->velocity[1] = self->jump_velocity[1];
//		}
/*
		VectorCopy(self->s.origin, dest);
		dest[2] -= 8;

		tr = gi.trace(self->s.origin, self->mins, self->maxs, dest, self, MASK_PLAYERSOLID);

		// only really inair if way above ground
		if (tr.fraction == 1)
		{
			self->last_inair = level.time;
		}
*/
		self->last_inair = level.time;

		if (!self->client->ctf_grapple || (self->client->ctf_grapplestate <= CTF_GRAPPLE_STATE_FLY))
		{
			// this is some really ugly hacks to fix in-air velocity stuff
			if (self->velocity[2] > self->wait)
				self->wait = self->velocity[2];

			if ((self->velocity[2] > -300) && (self->velocity[2] < 0))
				self->velocity[2] = self->wait - sv_gravity->value*FRAMETIME;
		}
	}
	else
	{
		if (self->jump_velocity[0] || self->jump_velocity[1])
		{
			self->jump_velocity[0] = self->jump_velocity[1] = 0;;
		}
	
		if (!self->last_inair || (self->last_inair > level.time))
			self->last_inair = 0;
		else	// make sure we remove it next frame
			self->last_inair = level.time + 1;
	}

	self->wait = self->velocity[2];

	self->monsterinfo.run(self);
	self->last_think_time = level.time;

//	if (self->linkcount != self->monsterinfo.linkcount)
//	{
//		self->monsterinfo.linkcount = self->linkcount;
	if (!self->client->ctf_grapple)
		M_CheckGround (self);
//	}
	M_CatagorizePosition (self);

	current_player = self;
	current_client = self->client;

	P_WorldEffects ();
	M_SetEffects (self);

	self->nextthink = level.time + FRAMETIME;
//self->nextthink = -1;
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
	gi.linkentity (self);

	monster_start_go (self);

	// RAFAEL
	if (strcmp (self->classname, "monster_fixbot") == 0)
	{
		if (self->spawnflags &16 || self->spawnflags &8 || self->spawnflags &4)
		{
			self->enemy = NULL;
			return;
		}
	}

	if (self->enemy && !(self->spawnflags & 1) && !(self->enemy->flags & FL_NOTARGET))
	{
		FoundTarget (self);
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
	if (activator->client)
		self->enemy = activator;
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
	self->flags &= ~(FL_FLY|FL_SWIM);
	self->monsterinfo.aiflags &= AI_GOOD_GUY;

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

void monster_start (edict_t *self)
{
/*
	if ((deathmatch->value || nomonsters->value) && (!strcmp(self->classname, "bot")))
	{
		G_FreeEdict (self);
		return false;
	}
*/

	if ((self->spawnflags & 4) && !(self->monsterinfo.aiflags & AI_GOOD_GUY))
	{
		self->spawnflags &= ~4;
		self->spawnflags |= 1;
//		gi.dprintf("fixed spawnflags on %s at %s\n", self->classname, vtos(self->s.origin));
	}

	if (!(self->monsterinfo.aiflags & AI_GOOD_GUY))
		level.total_monsters++;

	self->nextthink = level.time + FRAMETIME;
	self->svflags |= SVF_MONSTER;
	self->s.renderfx |= RF_FRAMELERP;
	self->takedamage = DAMAGE_AIM;
	self->air_finished = level.time + 12;
//	self->use = monster_use;
	self->max_health = self->health;
	self->clipmask = MASK_MONSTERSOLID;

//	self->s.skinnum = 0;
	self->deadflag = DEAD_NO;
	self->svflags &= ~SVF_DEADMONSTER;

	if (!self->monsterinfo.checkattack)
		self->monsterinfo.checkattack = M_CheckAttack;
	VectorCopy (self->s.origin, self->s.old_origin);

/*
	if (st.item)
	{
		self->item = FindItemByClassname (st.item);
		if (!self->item)
			gi.dprintf("%s at %s has bad item: %s\n", self->classname, vtos(self->s.origin), st.item);
	}

	// randomize what frame they start on
	if (self->monsterinfo.currentmove)
		self->s.frame = self->monsterinfo.currentmove->firstframe + (rand() % (self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1));

	return true;
*/
}

void monster_start_go (edict_t *self)
{
	vec3_t	v;

	if (self->health <= 0)
		return;

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
			VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
			self->ideal_yaw = self->s.angles[YAW] = vectoyaw(v);
			self->monsterinfo.walk (self);
			self->target = NULL;
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
		self->monsterinfo.stand (self);
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
	self->viewheight = 25;

	monster_start_go (self);

	if (self->spawnflags & 2)
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
	self->viewheight = 25;

	monster_start_go (self);

	if (self->spawnflags & 2)
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

	if (self->spawnflags & 2)
		monster_triggered_start (self);
}

void swimmonster_start (edict_t *self)
{
	self->flags |= FL_SWIM;
	self->think = swimmonster_start_go;
	monster_start (self);
}
