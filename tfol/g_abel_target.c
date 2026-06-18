
#include "g_local.h"

#define SPAWNFLAG_TELE_VELOCITY		1
#define SPAWNFLAG_TELE_ANGLE_YAW	2
#define SPAWNFLAG_TELE_ANGLE_PITCH	4

void use_target_tele (edict_t *ent, edict_t *other, edict_t *activator)
{
	vec3_t	angles, forward, right, v;
	edict_t	*player;
	edict_t	*dest;
	int i;

	
	player = &g_edicts[1];	// Gotta be, since this is SP only


	if(!ent->target)
	{
		gi.dprintf("tele no target");
		return;
	}
	
	dest = G_Find(NULL,FOFS(targetname),ent->target);
	
	if(!dest)
	{
		gi.dprintf("***** %s cannot find target: %s\n", ent->targetname, ent->target);
		return;
	}



	//do the teleport.
	VectorCopy (dest->s.origin, player->s.origin);
	VectorCopy (dest->s.origin, player->s.old_origin);
	
	if (ent->spawnflags & SPAWNFLAG_TELE_VELOCITY)
	{
		//continue the velocity. Speedy thing goes in, speedy thing goes out.
		VectorSubtract(dest->s.angles, player->s.angles,angles);
			AngleVectors(angles,forward,right,NULL);
			VectorNegate(right,right);
			VectorCopy(player->velocity,v);
			G_ProjectSource (vec3_origin,
							 v, forward, right,
							 player->velocity);
	}
	else
	{
		VectorClear (player->velocity);

		
		if(player->client)
		{
			player->client->ps.pmove.pm_time = 160>>3;		// hold time
			player->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
		}

		//VectorSet(player->velocity,		0,0, 10);
	}


		


	//view angles.

	
	if(player->client)
	{
		for (i=0 ; i<3 ; i++)
		{
			if ((ent->spawnflags & SPAWNFLAG_TELE_ANGLE_YAW) && i == YAW)
				continue;

			if ((ent->spawnflags & SPAWNFLAG_TELE_ANGLE_PITCH) && i == PITCH)
				continue;

			player->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - player->client->resp.cmd_angles[i]);
		}

		VectorClear (player->client->ps.viewangles);
		VectorClear (player->client->v_angle);
		VectorClear (player->s.angles);
	}
	else
		VectorCopy (dest->s.angles,player->s.angles);
	

}

void SP_target_teleporter (edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("%s without a target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	self->use = use_target_tele;
	self->svflags = SVF_NOCLIENT;
}



//====================== TARGET_SPEWER

void use_spewer (edict_t *ent, edict_t *other, edict_t *activator)
{
	//launch a thing.

	float randSpeed;
	edict_t *gib;
	vec3_t forward, right, v, randAng;

	gib = G_Spawn();

	gi.setmodel (gib, ent->usermodel);
	gib->solid = SOLID_NOT;

	VectorCopy(ent->s.origin, gib->s.origin);

	gib->movetype = MOVETYPE_BOUNCE;

	gib->avelocity[0] = -400 + random()*800;
	gib->avelocity[1] = -400 + random()*800;
	gib->avelocity[2] = -400 + random()*800;

	VectorCopy(ent->s.angles, randAng);
	randAng[0] += crandom() * ent->offset[0];
	randAng[1] += crandom() * ent->offset[1];
	AngleVectors(randAng, forward, NULL, NULL);

	randSpeed = ent->speed  + crandom() * ent->random;

	VectorScale (forward, randSpeed, gib->velocity);

	gib->s.renderfx |= RF_MINLIGHT;

	gib->think = G_FreeEdict;
	gib->nextthink = level.time  + 20 + random() * 10;//lifetime.
	
	gi.linkentity (gib);
}

void SP_target_spewer (edict_t *self)
{
	if (!self->speed)
		self->speed = 256;

	self->use = use_spewer;
	self->svflags = SVF_NOCLIENT;
}





//================== TARGET_CAMTURNER

void camturn_think (edict_t *self)
{
	edict_t *player;

	player = &g_edicts[1];	// Gotta be, since this is SP only

	if (player->client->ps.stats[STAT_CAMTURN] > 0)
	{
		player->client->ps.stats[STAT_CAMTURN] -= self->speed;

		if (player->client->ps.stats[STAT_CAMTURN] < 0)
		{
			player->client->ps.stats[STAT_CAMTURN] = 0;
			self->count = 0;

			if (self->target)
				G_UseTargets (self, self);
		}

		self->think = camturn_think;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		self->count = 0;

		if (self->target)
			G_UseTargets (self, self);
	}
}

void use_camturner (edict_t *ent, edict_t *other, edict_t *activator)
{
	if (ent->count <= 0)
	{
		//turn it ON.
		activator->client->ps.stats[STAT_CAMTURN] = ent->radius;
		ent->count++;
	}
	else if (ent->count == 1)
	{
		ent->count++;

		ent->think = camturn_think;
		ent->nextthink = level.time + FRAMETIME;
	}
	else
	{
		//reset it off.
		ent->count = 0;
		activator->client->ps.stats[STAT_CAMTURN] = 0;
	}
}

void SP_target_camturner(edict_t *self)
{
	if (!self->speed)
		self->speed = 2;

	if (!self->radius)
		self->radius = 30;

	self->use = use_camturner;
	self->svflags = SVF_NOCLIENT;
}


// =================================== TARGET_ATTACHER
void use_attacher (edict_t *ent, edict_t *other, edict_t *activator)
{
	edict_t	*player;
	int statIndex;
	int modelIndex;

	//gi.dprintf("idx %d \n", gi.modelindex ("models/monsters/npc/tris.md2"));

	player = &g_edicts[1];	// Gotta be, since this is SP only

	if (!player || !player->client)
		return;

	if (ent->spawnflags & 1)
	{
		//turn ON.
		modelIndex = gi.modelindex (ent->usermodel);
	}
	else
		modelIndex = 0;

	if (ent->style <= 0)
	{
		player->client->ps.stats[STAT_ATTACHMENT1] = modelIndex;
		player->client->ps.stats[STAT_ATTACHMENT1FRAME] = ent->startframe;
		player->client->ps.stats[STAT_ATTACHMENT1SKIN] = ent->s.skinnum;
		player->client->ps.stats[STAT_ATTACHMENT1HEIGHT] = ent->radius;

	}
	else
	{
		player->client->ps.stats[STAT_ATTACHMENT2] = modelIndex;
		player->client->ps.stats[STAT_ATTACHMENT2FRAME] = ent->startframe;
		player->client->ps.stats[STAT_ATTACHMENT2SKIN] = ent->s.skinnum;
		player->client->ps.stats[STAT_ATTACHMENT2HEIGHT] = ent->radius;
	}
}

void SP_target_attacher(edict_t *self)
{
	if (!self->usermodel && self->spawnflags & 1)
	{
		gi.dprintf("%s without a model at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	if (!self->radius)
		self->radius = -42;

	//BC baboo
	if (skill->value >= 2 && self->usermodel)
	{
		if (strcmp(self->usermodel,"models/monsters/npc/tris.md2")==0)
			self->s.skinnum = 7;
	}


	self->use = use_attacher;
	self->svflags = SVF_NOCLIENT;
}




// ======================= TARGET_CAMDIPPER

void camdipper_riseup_think (edict_t *self)
{
	edict_t *player;

	player = &g_edicts[1];	// Gotta be, since this is SP only

	player->client->ps.stats[STAT_CAMDIP] -= self->speed;


	if (player->client->ps.stats[STAT_CAMDIP] <= 0)
	{
		player->client->ps.stats[STAT_CAMDIP] = 0;

		if (self->target)
			G_UseTargets (self, self);

		self->count = 0;
		return;
	}

	self->think = camdipper_riseup_think;
	self->nextthink = level.time + FRAMETIME;	
}

void camdipper_dipdown_think (edict_t *self)
{
	edict_t *player;
	player = &g_edicts[1];	// Gotta be, since this is SP only

	player->client->ps.stats[STAT_CAMDIP] += self->speed;

	if (player->client->ps.stats[STAT_CAMDIP] >= self->radius)
	{
		player->client->ps.stats[STAT_CAMDIP] = self->radius;

		if(self->pathtarget)
		{
			edict_t *e;
			e = NULL;

			while ((e = G_Find (e, FOFS(targetname), self->pathtarget)))
			{
				if (!e->use)
					continue;

				e->use(e,NULL, NULL);
			}
		}

		self->think = camdipper_riseup_think;
		//self->nextthink = level.time + (self->wait * 1000);		
		self->nextthink = level.time + self->wait;

		return;
	}

	self->think = camdipper_dipdown_think;
	self->nextthink = level.time + FRAMETIME;	
}

void use_camdipper (edict_t *ent, edict_t *other, edict_t *activator)
{
	if (ent->count > 0)
		return;

	ent->count = 1;

	ent->think = camdipper_dipdown_think;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_target_camdipper(edict_t *self)
{
	if (!self->radius)
		self->radius = 35;

	if (!self->wait)
		self->wait = 0.5;

	if (!self->speed)
		self->speed = 5;

	self->use = use_camdipper;
	self->svflags = SVF_NOCLIENT;
}


// ========================= TARGET_PLAYEREFFECT


void playereffect_think (edict_t *self)
{
	//spew the particle.
	edict_t *player;
	vec3_t forward, right, start, angle;

	player = &g_edicts[1];	// Gotta be, since this is SP only

	VectorCopy (player->client->v_angle, angle);
	angle[0] = 0; //pitch is zero.
	AngleVectors(angle,forward,right,NULL);

	VectorCopy (player->s.origin, start);
	VectorMA(start, self->offset[1], forward, start);
	VectorMA(start, self->offset[0], right, start);
	start[2] += self->offset[2];

	//VectorCopy (start, self->s.origin);
	//VectorCopy (start, self->s.old_origin);

	//G_SetMovedir (self->s.angles, self->movedir);
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(self->style);
	
	gi.WritePosition(start);
	gi.WriteDir (player->velocity);

	gi.multicast(start, MULTICAST_PHS);

//	gi.dprintf("%f %f %f\n", player->velocity[0], player->velocity[1], player->velocity[2]);


	if (self->count > 0)
	{
		self->count--;

		self->think = playereffect_think;
		self->nextthink = level.time + self->wait;
	}
}


void use_playereffect (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->think = playereffect_think;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_target_playereffect(edict_t *self)
{
	if (!self->wait)
		self->wait = 0.2;

	if (!self->count)
		self->count = 1;

	self->use = use_playereffect;
	self->svflags = SVF_NOCLIENT;
}

// ============= TARGET_PLAYERANGLE

void playerangle_think(edict_t *self)
{
	edict_t	*player;
	float adjustedYaw = 0;

	if (self->count <= 0)
		return;

	player = &g_edicts[1];

	//gi.dprintf("playerAng  %d    \n", abs(self->s.angles[1] - player->client->v_angle[1]));

	adjustedYaw = player->client->v_angle[1];

	if (adjustedYaw < 0)
	{
		adjustedYaw = abs(adjustedYaw);
		adjustedYaw = 180 - adjustedYaw;
		adjustedYaw += 180;
	}


	//gi.dprintf("playerAng  %f  %f    \n", self->s.angles[1] , adjustedYaw);

	if (abs(self->s.angles[1] - adjustedYaw) <= self->radius)
	{
		G_UseTargets (self, self);

		if (self->wait > 0)
		{
			self->think = playerangle_think;
			self->nextthink = level.time + self->wait;
		}
		return;
	}

	self->think = playerangle_think;
	self->nextthink = level.time + FRAMETIME;
}

void use_playerangle (edict_t *ent, edict_t *other, edict_t *activator)
{
	if (ent->count <= 0)
		ent->count = 1;
	else
		ent->count = 0;

	if (ent->count >= 1)
	{
		ent->think = playerangle_think;
		ent->nextthink = level.time + FRAMETIME;
	}
}

void SP_target_playerangle(edict_t *self)
{
	if (!self->radius)
		self->radius = 20;

	if (!self->wait)
		self->wait = -1;

	if (self->spawnflags & 1)
	{
		//start on.
		self->count = 1;
		self->think = playerangle_think;
		self->nextthink = level.time + FRAMETIME;
	}

	self->use = use_playerangle;
	self->svflags = SVF_NOCLIENT;
}


void think_audiofollow(edict_t *ent)
{
	edict_t *player;

	if (ent->count <= 0)
		return;

	ent->think = think_audiofollow;
	ent->nextthink = level.time + FRAMETIME;

	player = &g_edicts[1];	// Gotta be, since this is SP only

	if (ent->offset[0] == player->s.origin[0] && ent->offset[1] == player->s.origin[1])
	{
		ent->s.sound = 0;
		return;
	}
	else if (ent->s.sound <= 0)
	{
		ent->s.sound = ent->noise_index;
	}

	ent->offset[0] = player->s.origin[0];
	ent->offset[1] = player->s.origin[1];

	VectorCopy (player->s.origin, ent->s.origin);	
	VectorCopy (player->s.origin, ent->s.old_origin);

	//gi.dprintf("%f %f %f\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
}

void use_audiofollow(edict_t *ent, edict_t *other, edict_t *activator)
{
	if (ent->count <= 0)
		ent->count = 1;
	else
	{
		ent->count = 0;
		ent->s.sound = 0;
	}

	if (ent->count >= 1)
	{
		//turn on.
		//ent->s.sound = ent->noise_index;
		edict_t *player;

		player = &g_edicts[1];	// Gotta be, since this is SP only
		ent->offset[0] = player->s.origin[0];
		ent->offset[1] = player->s.origin[1];

		
		ent->think = think_audiofollow;
		ent->nextthink = level.time + FRAMETIME;
	}
}

void SP_target_audiofollow(edict_t *ent)
{
	if(!st.noise)
	{
		gi.dprintf("target_speaker with no noise set at %s\n", vtos(ent->s.origin));
		G_FreeEdict(ent);
		return;
	}
	// DWH: Use "message" key to store noise for speakers that change levels
	//      via trigger_transition
	if (!strstr (st.noise, ".wav"))
	{
		ent->message = gi.TagMalloc(strlen(st.noise)+5,TAG_LEVEL);
		sprintf(ent->message,"%s.wav", st.noise);
	}
	else
	{
		ent->message = gi.TagMalloc(strlen(st.noise)+1,TAG_LEVEL);
		strcpy(ent->message,st.noise);
	}

	ent->noise_index = gi.soundindex (ent->message);

	ent->movetype = MOVETYPE_FLY;
	ent->attenuation = 1;
	ent->volume      = 1;
	ent->spawnflags  = 0;       // plays constantly

	ent->use = use_audiofollow;
	gi.linkentity (ent);
}


// ============== TARGET_SKINNER

void use_skinner (edict_t *ent, edict_t *other, edict_t *activator)
{
	edict_t *dest;
	dest = G_Find(NULL,FOFS(targetname),ent->target);
	
	if(!dest)
	{
		gi.dprintf("***** %s cannot find target: %s\n", ent->targetname, ent->target);
		return;
	}

	if (skill->value >= 2 && dest->usermodel != NULL)
	{
		if (strcmp(dest->usermodel,"monsters/npc/tris.md2")==0)
		{
			return;
		}
	}

	//found the entity.
	dest->s.skinnum = ent->s.skinnum;
}

void SP_target_skinner(edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("%s without a target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	self->use = use_skinner;
	self->svflags = SVF_NOCLIENT;
}

// ============================= TARGET_HIDER



void hider_do(edict_t *ent)
{
	edict_t *dest;
	dest = NULL;
	dest = G_Find(NULL,FOFS(targetname),ent->target);
	
	if(!dest)
	{
		gi.dprintf("***** %s cannot find target: %s\n", ent->targetname, ent->target);
		return;
	}

	while (dest)
	{
		//found the entity.
		if (ent->style >= 1) //HIDE IT
			dest->svflags |= SVF_NOCLIENT;
		else if (ent->style < 1)
			dest->svflags &= ~SVF_NOCLIENT;

		dest = G_Find(dest,FOFS(targetname),ent->target);
	}
}

void use_hider(edict_t *self, edict_t *other, edict_t *activator)
{
	if (!self->delay)
	{
		hider_do(self);
		return;
	}

	self->think = hider_do;
	self->nextthink = level.time + self->delay;	
}

void SP_target_hider(edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("%s without a target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	self->use = use_hider;
	self->svflags = SVF_NOCLIENT;
}



// ============================= TARGET_FRAMER

void frame_do(edict_t *ent)
{
	edict_t *dest;
	dest = NULL;
	dest = G_Find(NULL,FOFS(targetname),ent->target);
	
	if(!dest)
	{
		gi.dprintf("***** %s cannot find target: %s\n", ent->targetname, ent->target);
		return;
	}

	
	//found the entity.

	while (dest)
	{
		dest->s.frame = ent->startframe;
		dest->startframe = ent->startframe;
		dest->framenumbers = ent->startframe + ent->framenumbers;

		dest = G_Find(dest,FOFS(targetname),ent->target);
	}
}

void use_framer(edict_t *self, edict_t *other, edict_t *activator)
{
	if (!self->delay)
	{
		frame_do(self);
		return;
	}

	self->think = frame_do;
	self->nextthink = level.time + self->delay;	
}

void SP_target_framer(edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("%s without a target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	if (!self->framenumbers)
		self->framenumbers = 1;

	self->use = use_framer;
	self->svflags = SVF_NOCLIENT;
}



// ============================= TARGET_BLOOMER

void think_bloomdown(edict_t *self)
{
	edict_t *player;

	player = &g_edicts[1];	// Gotta be, since this is SP only

	if (!player)
		return;

	if (player->client->ps.stats[STAT_BLOOMINTENSITY] > 0)
		player->client->ps.stats[STAT_BLOOMINTENSITY] -= 20;	

	if (player->client->ps.stats[STAT_BLOOMINTENSITY] < 0)
		player->client->ps.stats[STAT_BLOOMINTENSITY] = 0;

	if (player->client->ps.stats[STAT_BLOOMALPHA] > 0)
		player->client->ps.stats[STAT_BLOOMALPHA] -= 10;

	if (player->client->ps.stats[STAT_BLOOMALPHA] < 0)
		player->client->ps.stats[STAT_BLOOMALPHA] = 0;


	if (player->client->ps.stats[STAT_BLOOMINTENSITY] > 0 || player->client->ps.stats[STAT_BLOOMALPHA] > 0)
	{
		self->think = think_bloomdown;
		self->nextthink = level.time + FRAMETIME;
	}
}

void think_bloomer(edict_t *self)
{
	edict_t *player;

	if (self->count >= 80)
		return;

	player = &g_edicts[1];	// Gotta be, since this is SP only

	if (!player)
		return;

	self->count++;


	player->client->ps.stats[STAT_BLOOMINTENSITY] +=  7;	
	player->client->ps.stats[STAT_BLOOMALPHA] += 3;

	//player->client->ps.stats[STAT_BLOOMDARKEN] = 12;



	self->think = think_bloomer;
	self->nextthink = level.time + FRAMETIME;

	//gi.dprintf("%f %f\n", self->density, player->client->ps.stats[STAT_BLOOMINTENSITY]);
	
}

void use_bloomer(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & 1)
	{
		//turn off.
		edict_t *player;
		player = &g_edicts[1];	// Gotta be, since this is SP only


		if (self->spawnflags & 2)
		{
			//instant turnoff, p.u.!!!
			player->client->ps.stats[STAT_BLOOMINTENSITY] = 0;
			player->client->ps.stats[STAT_BLOOMALPHA] = 0;
			return;
		}

		self->think = think_bloomdown;
		self->nextthink = level.time + FRAMETIME;	

		return;
	}

	if (self->spawnflags & 2)
	{
		edict_t *player;
		player = &g_edicts[1];	// Gotta be, since this is SP only

		player->client->ps.stats[STAT_BLOOMINTENSITY] = 560;
		player->client->ps.stats[STAT_BLOOMALPHA] = 240;
		return;
	}

	self->think = think_bloomer;
	self->nextthink = level.time + FRAMETIME;	
}

void SP_target_bloomer(edict_t *self)
{
	self->use = use_bloomer;
	self->svflags = SVF_NOCLIENT;
}



// ============================= TARGET_GODMODE

void use_godmode(edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *player;
	player = &g_edicts[1];	// Gotta be, since this is SP only

	if (self->spawnflags & 1)
	{
		//turn off.
		player->client->ps.stats[STAT_GODMODE] = 0;
		return;
	}

	player->client->ps.stats[STAT_GODMODE] = 1;	
}

void SP_target_godmode(edict_t *self)
{
	self->use = use_godmode;
	self->svflags = SVF_NOCLIENT;
}

// ============================= TARGET_MOVESLOW

void use_moveslow(edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *player;
	player = &g_edicts[1];	// Gotta be, since this is SP only

	if (self->spawnflags & 1)
	{
		//turn off.
		player->client->ps.stats[STAT_MOVESLOW] = 0;
		return;
	}

	player->client->ps.stats[STAT_MOVESLOW] = 1;	
}

void SP_target_moveslow(edict_t *self)
{
	self->use = use_moveslow;
	self->svflags = SVF_NOCLIENT;
}




// ============================= TARGET_POSTER

#define POSTERSKINS  13

void think_poster(edict_t *self)
{
	self->s.frame++;

	if (self->s.frame >= 50)
		self->s.frame = 0;

	if (self->s.frame == 44)
	{
		self->s.skinnum++;

		if (self->s.skinnum >= POSTERSKINS)
			self->s.skinnum = 1;
	}

	self->think = think_poster;
	self->nextthink = level.time + FRAMETIME;
}


int sound_plink;

void think_wantedposter(edict_t *self)
{
	self->s.frame++;

	if (self->s.frame >= 50)
		self->s.frame = 0;

	if (self->s.frame == 44)
		self->s.skinnum = 0;

	if (self->s.frame == 1 && self->s.skinnum == 0)
	{
		gi.sound (self, CHAN_VOICE, sound_plink, 1, ATTN_NORM, 0);//ATTN_IDLE
		return;
	}
	else
	{
		self->think = think_wantedposter;
		self->nextthink = level.time + FRAMETIME;
	}
}

void use_poster(edict_t *self, edict_t *other, edict_t *activator)
{
	self->think = think_wantedposter;
	self->nextthink = level.time + (random() * 3);
}



void SP_target_poster(edict_t *self)
{
	edict_t *frame;
	
	if (self->attenuation > 0)
		self->svflags = SVF_NOCLIENT;

	sound_plink = gi.soundindex ("world/plink.wav");

	self->s.modelindex = gi.modelindex ("models/objects/poster/tris.md2");	

	self->s.renderfx |= RF_NOSHADOW;
	self->solid			= SOLID_NOT;
	self->takedamage	= DAMAGE_NO;

	self->s.frame = random() * 49;
	self->s.skinnum = 1 + (random() * (POSTERSKINS - 1));
	
	self->think = think_poster;
	self->nextthink = level.time + FRAMETIME;

	self->use = use_poster;

	gi.linkentity (self);
	
	frame = G_Spawn();
	frame->s.modelindex = gi.modelindex ("models/objects/poster/frame.md2");	
	frame->s.renderfx |= RF_NOSHADOW;
	frame->solid			= SOLID_NOT;
	frame->takedamage	= DAMAGE_NO;
	VectorCopy(self->s.angles, frame->s.angles);
	VectorCopy(self->s.origin, frame->s.origin);
	gi.linkentity (frame);
}

// ====================== TARGET_GLASSGIB

void glassgib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int rand1;

	if (!plane)
		return;	
	
	if (self->density <= 0 || self->groundentity)
	{
		self->touch = NULL;
		return;
	}

	rand1 = random() * 10;
	gi.sound (self, CHAN_ITEM, gi.soundindex( va("world/tinkle0%i.wav",rand1)), self->density, ATTN_NORM, 0);	

	//with each bounce, the tinkle sound lowers volume.
	self->density -= 0.2;
}

void spawnglassgib (edict_t *self)
{
	float randSpeed;
	edict_t *gib;
	vec3_t forward, right, v, randAng;

	int rand1;

	gib = G_Spawn();	

	rand1 =  random() * 5;
	gi.setmodel (gib, va("models/objects/glass/glass0%i.md2",rand1));	

	//gib->solid = SOLID_NOT;
	gib->solid = SOLID_TRIGGER;

	VectorCopy(self->s.origin, gib->s.origin);
	gib->s.origin[1] += crandom() * 32;
	gib->s.origin[0] += crandom() * 32;


	gib->gravity  = 0.4 + random() * 0.3;
	gib->density = 1;
	

	gib->movetype = MOVETYPE_BOUNCE;

	if (random() < 0.2)
		gib->touch = glassgib_touch;

	gib->avelocity[0] = crandom()*128;
	gib->avelocity[1] = crandom()*128;
	gib->avelocity[2] = crandom()*128;

	VectorSet(forward,
		crandom() * 32,
		crandom() * 32,
		1);
	
	randSpeed = 0.5;

	VectorScale (forward, randSpeed, gib->velocity);

	//gib->s.renderfx |= RF_MINLIGHT;

	gib->think = G_FreeEdict;
	gib->nextthink = level.time + 5 + random() * 5;

	gi.linkentity (gib);
}

void use_glassgib(edict_t *self, edict_t *other, edict_t *activator)
{
	int i;
	int rand1;

	for (i = 0; i < 2; i++)
	{
		rand1 =  1 + random() * 5;
		gi.sound (self, CHAN_ITEM, gi.soundindex( va("world/glass0%i.wav",rand1)), 1, ATTN_NORM, 0);//ATTN_IDLE
	}

	//spawn 3d models.
	for (i = 0; i < 8; i++)
		spawnglassgib(self);


	//spawn the particles.
	
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_LOBBYGLASS);
	gi.WritePosition (self->s.origin);		
	gi.WriteDir (vec3_origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);	
	
}

void SP_target_glassgib(edict_t *self)
{
	self->use = use_glassgib;
	self->svflags = SVF_NOCLIENT;
	gi.linkentity (self);
}



// ============================ FUNC_INSPECT

void think_inspect(edict_t *self)
{
	trace_t     tr;
	vec3_t      end, forward, start;
	edict_t *player;

	player = &g_edicts[1];	// Gotta be, since this is SP only


	if (player->client->ps.fov >= 90)
	{
		self->count = 0;
		return;
	}

	VectorCopy(player->s.origin, start);
	start[2] += player->viewheight;

	AngleVectors(player->client->v_angle, forward, NULL, NULL);

	VectorMA(start, 64, forward, end);

	tr = gi.trace (start, NULL, NULL, end, player, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

	if (tr.ent)
	{
		if (tr.ent != self)
		{
			gi.sound (self, CHAN_VOICE, gi.soundindex( "world/snap2.wav"), 1, ATTN_NONE, 0);

			player->client->ps.fov = 90;

			self->count = 0;
			return;
		}
	}

	self->think = think_inspect;
	self->nextthink = level.time + FRAMETIME;

}

void use_inspect(edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *player;
	player = &g_edicts[1];	// Gotta be, since this is SP only

	//turn on the zoom.
	player->client->ps.fov = self->dmg;
	self->count = 1;  //disable fingercursor.

	gi.sound (self, CHAN_VOICE, gi.soundindex( "world/snap.wav"), 1, ATTN_NONE, 0);

	//start tracking when the player no longer looks at it.
	self->think = think_inspect;
	self->nextthink = level.time + FRAMETIME;
}

void SP_func_inspect(edict_t *self)
{
	if (!self->dmg)
		self->dmg = 20;

	self->classname = "func_inspect";
	self->solid = SOLID_BSP;
	self->use = use_inspect;
	gi.setmodel (self, self->model);	
	gi.linkentity (self);
}



// ============== target fovzoom

#define FOVZOOM_MIN  20

void think_fovzoom(edict_t *self)
{
	edict_t *player;
	player = &g_edicts[1];	// Gotta be, since this is SP only

	if (self->count <= 0)
	{


		//return to normal fov.		
		player->client->ps.fov += 1;

		if (player->client->ps.fov >= 90)
		{
			player->client->ps.fov = 90;
			return;
		}
	}
	else
	{
		//zoom in
		if (player->client->ps.fov > FOVZOOM_MIN)
			player->client->ps.fov -= 2;

		if (player->client->ps.fov <= FOVZOOM_MIN)
		{
			return;
		}
	}

	self->think = think_fovzoom;
	self->nextthink = level.time + FRAMETIME;
}

void use_fovzoom(edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *player;
	player = &g_edicts[1];	// Gotta be, since this is SP only

	if (player->client->ps.fov >= 90)
	{
		self->count = 1;
	}
	else
	{
		self->count = 0;
	}


	//start tracking when the player no longer looks at it.
	self->think = think_fovzoom;
	self->nextthink = level.time + FRAMETIME;
}

void SP_target_fovzoom(edict_t *self)
{
	self->use = use_fovzoom;
	self->svflags = SVF_NOCLIENT;
}



// ============ target_bernpaper

void think_bernpaper(edict_t *self)
{
	if (self->count <= 0)
	{
		//return to idle state.
		self->s.frame++;

		if (self->s.frame >= 40)
		{
			self->s.frame = 0;
			return;
		}		
	}
	else
	{
		self->s.frame++;

		if (self->s.frame >= 29)
			self->s.frame = 9;
	}

	self->think = think_bernpaper;
	self->nextthink = level.time + FRAMETIME;
}

void use_bernpaper(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->count <= 0)
		self->count = 1; //turn on.
	else
	{
		self->count = 0;
		self->s.frame = 30;
	}

	self->think = think_bernpaper;
	self->nextthink = level.time + FRAMETIME;
}

void SP_target_bernpaper(edict_t *self)
{
	edict_t *frame;

	if (!self->usermodel)
	{
		G_FreeEdict(self);
		return;
	}

	self->s.modelindex = gi.modelindex ( self->usermodel );	

	self->s.renderfx |= RF_NOSHADOW;
	self->solid			= SOLID_NOT;
	self->takedamage	= DAMAGE_NO;

	self->use = use_bernpaper;

	gi.linkentity (self);
	
}

// killfloaters

int FloaterCount(edict_t *self)
{
	int total;
	edict_t *item;

	total = 0;
	item = NULL;
	item = G_Find(NULL, FOFS(classname), "monster_floater");

	while (item)
	{
		total++;
		item = G_Find(item, FOFS(classname), "monster_floater");
	}

	return total;
}

void use_killfloaters(edict_t *self, edict_t *other, edict_t *activator)
{
	int i;
	int total = FloaterCount(self);

	if (total <= 3)
		return;

	for (i = 0; i  < total - 2; i++)
	{
		edict_t *item;

		item = NULL;
		item = G_Find(NULL, FOFS(classname), "monster_floater");

		if (item)
			G_FreeEdict(item);
	}
}

void SP_target_killfloaters(edict_t *self)
{
	self->use = use_killfloaters;
	self->svflags = SVF_NOCLIENT;	
}



// ========== TARGET_MATHAUDIO



void think_mathaudio(edict_t *self)
{
	int rand1;
	float vol;

	rand1 = random() * 20;
	vol = 0.5 + random() * 0.5;


	gi.sound (self, CHAN_ITEM, self->mathaudio[rand1], vol, ATTN_STATIC, 0);//ATTN_IDLE

	/*
	if (rand1 >= 10)
		gi.sound (self, CHAN_ITEM, gi.soundindex( va("mathman/math%i.wav",rand1)), vol, ATTN_STATIC, 0);
	else
		gi.sound (self, CHAN_ITEM, gi.soundindex( va("mathman/math0%i.wav",rand1)), vol, ATTN_STATIC, 0);
*/
	

	self->think = think_mathaudio;
	self->nextthink = level.time + 4 + (random() * 4);
}

void SP_target_mathaudio(edict_t *self)
{
	int i;
	i = 0;

	for (i = 0; i < 19; i++)
	{
		if (i < 10)
			self->mathaudio[i] =  gi.soundindex (va("mathman/math0%i.wav", i));
		else
			self->mathaudio[i] =  gi.soundindex (va("mathman/math%i.wav", i));
	}

	self->think = think_mathaudio;
	self->nextthink = level.time + 1 + random() * 6;
	self->svflags = SVF_NOCLIENT;	
}