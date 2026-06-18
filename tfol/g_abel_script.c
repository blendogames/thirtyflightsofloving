
#include "g_local.h"

//======================= SCRIPT_CART


//oh my this is chock full of hacky ugliness!!!!!!!!!!!

static int	sound_cartdrag;
static int	sound_carthit;
static int	sound_cartpush;

void think_script_cart (edict_t *self)
{
	if (self->count > 0)
		self->count--;

	if (self->count <= 0 && self->density >= 1)
	{
		vec3_t sparkpos;
		int i;

		self->count = 2;

		for (i = 0; i < 3; i++)
		{
			VectorCopy(self->goalentity->s.origin, sparkpos);
			sparkpos[2] -= 14;
			sparkpos[0] -= random() * 32;

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_SHOTGUN);
			gi.WritePosition (sparkpos);		
			gi.WriteDir (self->goalentity->s.angles);
			gi.multicast (sparkpos, MULTICAST_PVS);	
		}
	}

	//gi.dprintf("%f %f \n", self->goalentity->s.origin[0], self->goalentity->s.origin[1]);

	if (self->goalentity->s.origin[1] > 3880)
	{
		//stop!
		vec3_t sparkpos;
		int i;

		VectorClear(self->target_ent->velocity);
		VectorClear(self->goalentity->velocity);
		//gi.dprintf("%f %f \n", self->goalentity->s.origin[0], self->goalentity->s.origin[1]);

		self->goalentity->s.origin[1] = 3895;
		self->target_ent->s.origin[1] = 3895;

		gi.sound (self->goalentity, CHAN_VOICE, sound_carthit, 1, ATTN_NORM, 0);//ATTN_IDLE


		G_UseTargets (self, self);

		/*
		VectorCopy(self->s.origin, sparkpos);
		sparkpos[2] += 22;
		sparkpos[1] -= 8;
		sparkpos[0] += 16;

		for (i = 0; i < 3; i++)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BLASTER);
			gi.WritePosition (sparkpos);
			gi.WriteDir (vec3_origin);
			gi.multicast (sparkpos, MULTICAST_PVS);

			sparkpos[0] -= 16;
		}
*/
		return;
	}
	else if (self->goalentity->s.origin[1] > 3600 && self->density <= 0)
	{
		self->density = 1;

		self->goalentity->s.angles[2] = 90;
		self->target_ent->s.angles[2] = 90;

		self->goalentity->s.origin[2] += 15;
		self->target_ent->s.origin[2] -= 2;
		self->target_ent->s.origin[0] -= 8;

		gi.sound (self->goalentity, CHAN_VOICE, sound_cartdrag, 1, ATTN_NORM, 0);//ATTN_IDLE
	}
	

	self->think = think_script_cart;
	self->nextthink = level.time + FRAMETIME;
}

void use_script_cart(edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*player;	
	vec3_t playerpos, facing;

	if (self->density >= 1)
	{
		G_FreeEdict(self->goalentity);
		G_FreeEdict(self->target_ent);
		G_FreeEdict(self);
		return;
	}


	player = &g_edicts[1];	// Gotta be, since this is SP only

	VectorCopy(player->s.origin, playerpos);
	playerpos[2] = self->s.origin[2];

	VectorSubtract (self->s.origin, playerpos, facing);
	
	vectoangles(facing, facing);

	VectorCopy(facing, self->goalentity->s.angles);
	VectorCopy(facing, self->target_ent->s.angles);


	AngleVectors (facing, facing, NULL, NULL);
	VectorNormalize(facing);
	VectorScale (facing, 320, self->goalentity->velocity);
	VectorScale (facing, 320, self->target_ent->velocity);

	playerpos[1] += 32;

	VectorCopy(playerpos, self->goalentity->s.origin);
	VectorCopy(playerpos, self->goalentity->s.old_origin);

	playerpos[2] += 8;

	VectorCopy(playerpos, self->target_ent->s.origin);
	VectorCopy(playerpos, self->target_ent->s.old_origin);

	self->goalentity->svflags &= ~SVF_NOCLIENT;
	self->target_ent->svflags &= ~SVF_NOCLIENT;

	gi.sound (self, CHAN_VOICE, sound_cartpush, 1, ATTN_NONE, 0);//ATTN_IDLE

	self->think = think_script_cart;
	self->nextthink = level.time + FRAMETIME;
}

void SP_script_cart (edict_t *self)
{
	edict_t *frame;
	edict_t *cart;

	sound_cartdrag = gi.soundindex ("world/cartdrag.wav");
	sound_carthit = gi.soundindex ("world/metalhit.wav");
	sound_cartpush = gi.soundindex ("world/cartpush.wav");
	
	cart = G_Spawn();
	cart->s.modelindex = gi.modelindex ("models/objects/cart/tris.md2");	
	cart->solid			= SOLID_NOT;
	cart->takedamage	= DAMAGE_NO;
	cart->movetype = MOVETYPE_NOCLIP;
	cart->svflags = SVF_NOCLIENT;
	cart->s.renderfx = RF_MINLIGHT;
	gi.linkentity (cart);
	

	frame = G_Spawn();
	frame->s.modelindex = gi.modelindex ("models/monsters/npc/tris.md2");	
	frame->solid			= SOLID_NOT;
	frame->takedamage	= DAMAGE_NO;
	frame->svflags = SVF_NOCLIENT;
	frame->s.frame = 269;
	frame->s.skinnum = 2;
	frame->s.renderfx = RF_MINLIGHT;
	frame->movetype = MOVETYPE_NOCLIP;
	VectorCopy(self->s.angles, frame->s.angles);
	VectorCopy(self->s.origin, frame->s.origin);
	frame->s.origin[2] += 8;
	gi.linkentity (frame);
	
	self->target_ent = frame;
	self->goalentity = cart;
	self->use = use_script_cart;
	gi.linkentity (self);
}
