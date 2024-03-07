
#include "g_local.h"

void bulletbox_use (edict_t *self, edict_t *other, edict_t *activator)
{
	qboolean boxopen = false;

	if (!activator->client)
		return;

	

	if (self->count < 9)	
	{
		self->s.frame = self->count + 1;

		if (self->count > 0)
			gi.sound(self, CHAN_ITEM, gi.soundindex("misc/am_pkup.wav"), 1, ATTN_NORM, 0);
		else
			boxopen = true;

		self->count++;
	}
	else
	{
		if (self->s.frame == 9)
			self->s.frame = 10;
		else
			self->s.frame = 9;

		boxopen = true;
	}

	if (boxopen == true)
		gi.sound(self, CHAN_ITEM, gi.soundindex("world/boxopen.wav"), 1, ATTN_NORM, 0);

	//gi.dprintf("%d\n",self->count);
}

void SP_item_bulletbox (edict_t *self)
{
	VectorSet(self->mins, -10, -10, 0);
	VectorSet(self->maxs, 10, 10, 2.2);

	gi.setmodel (self, "models/objects/bulletbox/tris.md2");
	self->model = "models/objects/bulletbox/tris.md2";
	self->svflags          |= SVF_DEADMONSTER;

	self->count = 0;

	self->solid = SOLID_BBOX;
	//self->s.alpha = 0.99;

	gi.linkentity (self);
}


void pickup_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!activator->client)
		return;

	if (self->pathtarget)
	{
		//gi.dprintf("%s\n",st.noise);
		gi.sound(self, CHAN_ITEM, gi.soundindex(self->pathtarget), 1, ATTN_NORM, 0);
	}

	if (self->target)
		G_UseTargets (self, activator);

	//gi.dprintf("%d\n",self->count);
	G_FreeEdict (self);
}

void SP_item_pickup (edict_t *self)
{
	char	modelname[256];

	if (!self->usermodel)
	{
		gi.dprintf("%s without a model at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	if (st.noise)
	{
		self->pathtarget = gi.TagMalloc(strlen(st.noise)+1,TAG_LEVEL);
		strcpy(self->pathtarget,st.noise);
		//sprintf(self->message,"%s.wav", st.noise);
	}


	if (self->tright)
	{
		VectorCopy (self->tright, self->maxs);
	}

	if (self->bleft)
	{
		VectorCopy (self->bleft, self->mins);
	}

	if (self->startframe < 0)
		self->startframe = 0;
	else if (self->startframe > 0)
		self->s.frame = self->startframe;

	sprintf(modelname, "models/%s", self->usermodel);
	self->s.modelindex = gi.modelindex (modelname);

	//gi.setmodel (self, "models/objects/bulletbox/tris.md2");
	//self->model = "models/objects/bulletbox/tris.md2";
	self->svflags          |= SVF_DEADMONSTER;


	//if (st.noise)
	{
		//self->noise_index = gi.soundindex  (st.noise);
		//gi.dprintf("%s\n",st.noise);
	}

	//if (self->noise_index)
		//self->s.sound = self->noise_index;


	//self->count = 0;

	self->solid = SOLID_BBOX;
	//self->s.alpha = 0.99;

	//gi.dprintf("%f %f %f / %f %f %f\n",self->

	gi.linkentity (self);
}






// ====================== RADIO
void radio_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!activator->client)
		return;



	gi.sound(self, CHAN_ITEM, gi.soundindex("misc/menu2.wav"), 1, ATTN_NORM, 0);


	if (self->s.sound)
	{
		self->s.sound = 0;	// turn it off
		self->s.skinnum = 0;
		
		self->nextthink = 0;
	}
	else
	{
		self->s.sound = self->noise_index;	// start it
		self->s.skinnum = 1;
	}

}

void SP_item_radio (edict_t *self)
{
	VectorSet(self->mins, -7, -7, 0);
	VectorSet(self->maxs, 7, 7, 7);

	gi.setmodel (self, "models/objects/radio/tris.md2");
	self->model = "models/objects/radio/tris.md2";
	self->svflags          |= SVF_DEADMONSTER;

	self->solid = SOLID_BBOX;

	self->use = radio_use;


	if (st.noise)
		self->noise_index = gi.soundindex (st.noise);

	if (!self->volume)
		self->volume = 1.0;

	if (!self->attenuation)
		self->attenuation = 1.0;




	gi.linkentity (self);
}



// ================== item_partyguest2

void think_guest2(edict_t *self)
{
	self->think = think_guest2;

	if ((self->s.frame == 414 && !(self->spawnflags & 1)) || (self->s.frame == 416 && self->spawnflags & 1))
	{
		//DRINK.
		if (self->spawnflags & 1)
			self->s.frame =  417;
		else
			self->s.frame =  415;

		self->child->s.frame = 2;
		self->nextthink = level.time + 0.5 + random() * 1.5;
	}
	else
	{
		//STOP DRINK.
		
		if (self->spawnflags & 1)
			self->s.frame =  416;
		else
			self->s.frame =  414;

		self->child->s.frame = 1;
		self->nextthink = level.time + 3 + random() * 3.5;
	}	
}

void SP_item_partyguest2(edict_t *self)
{
	edict_t *gib;


	self->s.modelindex = gi.modelindex ("models/monsters/npc/tris.md2");

	
	self->solid			= SOLID_NOT;
	//VectorSet(self->mins, -1, -1, 0);
	//VectorSet(self->maxs, 1, 1, 52);
	self->s.skinnum = 13 + random() * 5;	


	//BC baboo
	if (skill->value >= 2)
	{
		self->s.skinnum = 7;
	}


	self->takedamage = DAMAGE_NO;

	if (self->spawnflags & 1)
		self->s.frame =  416;
	else
		self->s.frame =  414;

	self->think = think_guest2;
	self->nextthink = level.time + 1 + random() * 6;

	


	gib = G_Spawn();
	gib->solid = SOLID_NOT;
	gib->s.modelindex = gi.modelindex ("models/objects/wineglass/tris.md2");
	VectorCopy(self->s.origin, gib->s.origin);
	VectorCopy(self->s.angles, gib->s.angles);
	gib->s.frame = 1;
	gi.linkentity (gib);
	self->child = gib;


	gi.linkentity (self);
}









// ============== item_deck
static int sound_deckhum;

void deck_think (edict_t *self)
{
	if (self->count <= 0)
	{
		//close up!
		self->s.frame--;

		if (self->s.frame <= 0)
		{
			self->s.sound = 0;
			return;
		}
	}
	else
	{
		self->s.frame++;

		if (self->s.frame >= 10)
			return;
	}

	self->think = deck_think;
	self->nextthink = level.time + FRAMETIME;
}

void deck_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->s.frame <= 0)
	{
		self->count = 1; //open it.
		self->s.sound = sound_deckhum;
	}
	else
		self->count = 0; //close it.

	gi.sound(self, CHAN_ITEM, gi.soundindex("world/deckpanel.wav"), 1, ATTN_NORM, 0);

	self->think = deck_think;
	self->nextthink = level.time + FRAMETIME;
}

void SP_item_deck (edict_t *self)
{
	sound_deckhum = gi.soundindex ("world/deckhum.wav");

	VectorSet(self->mins, -8, -8, 0);
	VectorSet(self->maxs, 8, 8, 3.5);

	gi.setmodel (self, "models/objects/deck/tris.md2");
	self->model = "models/objects/deck/tris.md2";
	self->svflags          |= SVF_DEADMONSTER;

	self->count = 0;
	self->solid = SOLID_BBOX;

	gi.linkentity (self);
}