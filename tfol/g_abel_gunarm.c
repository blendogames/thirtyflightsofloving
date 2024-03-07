
#include "g_local.h"

#define GUNARM_FIRERATE		4

#define BASEDAMAGE			40
#define RANDDAMAGE			30

#define	BASEDISTANCE		2048

// ============================= TARGET_GUNARMS

void firetracer (edict_t *self, vec3_t pos, vec3_t dir)
{
	edict_t *gib;
	vec3_t forward;


	edict_t *player;
	player = &g_edicts[1];	// Gotta be, since this is SP only	

	gib = G_Spawn();

	gi.setmodel (gib, "models/objects/tracer/tris.md2");	
	gib->solid = SOLID_NOT;

	VectorCopy(pos, gib->s.origin);	


	gib->movetype = MOVETYPE_NOCLIP;

	VectorNormalize(dir);
	vectoangles(dir, gib->s.angles);

	
	AngleVectors(dir, forward, NULL, NULL);

	
	

	//gib->s.angles[PITCH] = player->client->ps.stats[STAT_GUNARM_PITCH] ;
	//gib->s.angles[YAW] = player->client->ps.stats[STAT_GUNARM_YAW] + 90;

	//vectoangles(forward, forward);

	VectorScale (dir, 1536, gib->velocity);

	//add player velocity
	//VectorAdd (gib->velocity, self->velocity, gib->velocity);

	
	gib->think = G_FreeEdict;
	gib->nextthink = level.time + 0.4;

	gi.linkentity (gib);
}

void gunarm_fire(edict_t *self, edict_t *badguy)
{
	edict_t *player;

	vec3_t playervec, viewang, aimdir, forward, right, armforward, muzzledir;
	vec3_t hitpos;

	//right 5
	//forward 23

	player = &g_edicts[1];	// Gotta be, since this is SP only	

	//left gun or right gun.
	if (self->style <= 0)
		self->style = 1;
	else
		self->style = 0;	
	
	AngleVectors (player->client->ps.viewangles, forward, right, NULL);
	VectorCopy (player->s.origin, playervec);	
	forward[2] = 0;
	VectorMA(playervec, 23, forward, playervec);

	if (self->style <= 0)
		VectorMA(playervec, 7, right, playervec);
	else
		VectorMA(playervec, -7, right, playervec);

	playervec[2] += 5;


	
	
	aimdir[PITCH] = player->client->ps.stats[STAT_GUNARM_PITCH] ;
	aimdir[YAW] = player->client->ps.stats[STAT_GUNARM_YAW] +  player->s.angles[YAW];
	aimdir[ROLL] = 0;	

	AngleVectors (aimdir, armforward, NULL, NULL);
	VectorMA(playervec, 29 , armforward, playervec);  //distance from shoulder to gun muzzle.
	


	//muzzle.
	VectorSubtract ( badguy->s.origin,playervec, muzzledir);



	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SHOTGUN);
	gi.WritePosition (playervec);		
	gi.WriteDir (muzzledir);
	gi.multicast (playervec, MULTICAST_PVS);	




	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (player - g_edicts);
	gi.WriteByte (MZ_MACHINEGUN);
	gi.multicast (playervec, MULTICAST_PVS);

	
	VectorCopy(badguy->s.origin, hitpos);

	hitpos[2] += random() * 30;

	
	
	firetracer(self, playervec, muzzledir);

	T_Damage (badguy, player, player, vec3_origin, hitpos, vec3_origin, BASEDAMAGE + rand() % RANDDAMAGE, -10, DAMAGE_BULLET, MOD_UNKNOWN);

	//advance the counter
	self->count++;

	if (self->count >= 56 && self->crane_dir <= 0)
	{
		self->crane_dir = 1;
		G_UseTargets (self, self);
	}
}

void gunarm_aimat(edict_t *self, edict_t *badguy)
{
	if (self->density <= 0)
	{
		//fire gun.
		self->density = GUNARM_FIRERATE;
		
		gunarm_fire(self, badguy);
	}
}

qboolean CheckBadguy(edict_t *self, edict_t *player, edict_t *badguy)
{
	float dist;
	vec3_t playervec;
	float yawang, pitchang;
	trace_t tr;

	vec3_t playereyes;

	dist = realrange(player,badguy);

	if (dist > BASEDISTANCE)
		return false; //too far.

	VectorCopy (player->s.origin, playervec);
	playervec[2] += 24;
	VectorSubtract (playervec, badguy->s.origin, playervec);

	vectoangles(playervec,playervec);

	yawang = playervec[YAW] - player->s.angles[YAW] - 180;

	if (yawang < -60 || yawang > 60)
		return false;

	pitchang = playervec[PITCH] + 360;

	if (pitchang < -85 || pitchang > 85)
		return false;

	VectorCopy(player->s.origin, playereyes);
	playereyes[2] += 24;

	tr = gi.trace (
		playereyes,
		vec3_origin,
		vec3_origin,
		badguy->s.origin, //dest
		player, //ignore
		MASK_SOLID);//MASK_SOLID

	if (tr.startsolid || tr.allsolid )
		return false;

	//gi.dprintf("%f\n", tr.fraction);

	if (tr.fraction < 1)
		return false;
	

	player->client->ps.stats[STAT_GUNARM_YAW] = yawang;
	player->client->ps.stats[STAT_GUNARM_PITCH] = -pitchang;
	return true;
}

edict_t *FindBadguy(edict_t *self)
{
	edict_t *player;
	edict_t *badguy;

	player = &g_edicts[1];	// Gotta be, since this is SP only	

	badguy = NULL;
	badguy = G_Find(NULL, FOFS(classname), "monster_floater");
	while(badguy)
	{
		if (CheckBadguy(self, player, badguy) == true)
			return badguy;			

		badguy = G_Find(badguy, FOFS(classname), "monster_floater");
	}

	player->client->ps.stats[STAT_GUNARM_YAW] = sin(level.time / 2) * 30;
	player->client->ps.stats[STAT_GUNARM_PITCH] = -12 + sin(level.time / 3) * 8;

	return NULL;
}

void think_gunarms(edict_t *self)
{
	edict_t *player;
	edict_t *badguy;

	player = &g_edicts[1];	// Gotta be, since this is SP only

	//find a badguy
	badguy = FindBadguy(self);

	if (self->density > 0)
		self->density--;

	if (badguy)
	{
		//aim at bad guy.
		gunarm_aimat(self, badguy);
	}

	self->think = think_gunarms;
	self->nextthink = level.time + FRAMETIME;
}

void use_gunarms(edict_t *self, edict_t *other, edict_t *activator)
{
	int modelIndex;
	edict_t *player;

	player = &g_edicts[1];	// Gotta be, since this is SP only

	if (self->spawnflags & 1)
	{
		//turn off.
		player->client->ps.stats[STAT_GUNARMS] = 0;
		player->client->ps.stats[STAT_GUNARM_YAW] = 0;
		player->client->ps.stats[STAT_GUNARM_PITCH] = 0;

		return;
	}

	modelIndex = gi.modelindex (self->usermodel);

	if (self->spawnflags & 2)
		player->client->ps.stats[STAT_GUNARMS] = 1;
	else
	{
		player->client->ps.stats[STAT_GUNARMS] = modelIndex;		

		self->think = think_gunarms;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_target_gunarms(edict_t *self)
{
	if (!self->usermodel && !(self->spawnflags & 1))
	{
		gi.dprintf("%s without a model at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	self->use = use_gunarms;
	self->svflags = SVF_NOCLIENT;
}


// =========== target_gunarmhide

void use_gunarmhide(edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *player;
	int modelindex;

	player = &g_edicts[1];	// Gotta be, since this is SP only	

	if (self->spawnflags & 1)
	{
		//hide.
		player->client->ps.stats[STAT_GUNARMS] = 0;
		return;
	}

	//unhide.	
	modelindex = gi.modelindex (self->message);
	player->client->ps.stats[STAT_GUNARMS] = modelindex;	
}

void SP_target_gunarmhide(edict_t *self)
{
	if (!self->message && !(self->spawnflags & 1))
	{
		gi.dprintf("%s without a message at %s. Using default: models/objects/gunarm/tris.md2\n", self->classname, vtos(self->s.origin));
		self->message = "models/objects/gunarm/tris.md2";
	}

	self->use = use_gunarmhide;
	self->svflags = SVF_NOCLIENT;
}