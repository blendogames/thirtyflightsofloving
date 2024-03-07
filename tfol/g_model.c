/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2000-2002 Mr. Hyde and Mad Dog

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "g_local.h"
//
// mappack stuff by mr. ed, modified extensively for Tremor by dwh
//

//Spawns a user defined model, you can specify whether its solid, if so how big the box is, and apply nearly
//any effect to the entity.
//
//	PLAYER		set this if you want to use a player model
//
//	Note : These flags override any frame settings you may have enterered
//	ANIM01		cycle between frames 0 and 1 at 2 hz
//	ANIM23		cycle between frames 2 and 3 at 2 hz
//	ANIM_ALL		cycle through all frames at 2hz
//	ANIM_ALLFAST	cycle through all frames at 10hz
//
//	START_OFF		Start inactive, when triggered display the model
//	TOGGLE		Start active, when triggered become inactive
//	NO_MODEL		Don't use a model. Usefull for placeing particle effects and
//				dynamic lights on their own
//
//	"usermodel" = The model to load (models/ is already coded)
//	"startframe" = The starting frame : default 0
//	"userframes" = The number of frames you want to display after startframe
//	"solidstate" = 1 : SOLID_NOT - not solid at all
//			       2 : SOLID_BBOX - solid and affected by gravity
//			       3 : NO DROP - solid but not affected by gravity
//                 4 : SOLID_NOT, but affect by gravity
//
//	NOTE : if you want the model to be solid then you must enter vector values into the following fields :
//	"bleft" = the point that is at the bottom left of the models bounding box in a model editor
//	"tright" = the point that is at the top left of the models bounding box in a model editor
//

#define	TOGGLE		    2
#define	PLAYER_MODEL	8
#define	NO_MODEL		16
#define ANIM_ONCE		32

void model_spawn_use (edict_t *self, edict_t *other, edict_t *activator);
void modelspawn_think (edict_t *self)
{
	self->s.frame++;
	if (self->s.frame >= self->framenumbers)
	{
		self->s.frame = self->startframe;
		if(self->spawnflags & ANIM_ONCE)
		{
			model_spawn_use(self,world,world);
			return;
		}
	}
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity(self);
}

void model_spawn_use (edict_t *self, edict_t *other, edict_t *activator)
{


	if (self->delay) //we started off
	{
		self->svflags &= ~SVF_NOCLIENT;
		self->delay = 0;
		if(self->framenumbers > 1)
		{
			self->think = modelspawn_think;
			self->nextthink = level.time + FRAMETIME;
		}
		self->s.sound = self->noise_index;
	}
	else             //we started active
	{
		self->svflags |= SVF_NOCLIENT;
		self->delay = 1;
		self->use = model_spawn_use;
		self->think = NULL;
		self->nextthink = 0;
		self->s.sound = 0;
	}
}

void model_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t	*e, *next;

	e = self->movewith_next;
	while(e) {
		next = e->movewith_next;
		if(e->solid == SOLID_NOT) {
			e->nextthink = 0;
			G_FreeEdict(e);
		} else
			BecomeExplosion1 (e);
		e = next;
	}

	BecomeExplosion1(self);
}

#define ANIM_MASK (EF_ANIM01|EF_ANIM23|EF_ANIM_ALL|EF_ANIM_ALLFAST)

void SP_model_spawn (edict_t *ent)
{
	char	modelname[256];

	//paranoia checks
	if ((!ent->usermodel) && !(ent->spawnflags & NO_MODEL) && !(ent->spawnflags & PLAYER_MODEL))
	{
		gi.dprintf("%s without a model at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict(ent);
		return;
	}

	switch (ent->solidstate)
	{
		case 1 : ent->solid = SOLID_NOT; ent->movetype = MOVETYPE_NONE; break;
		case 2 : ent->solid = SOLID_BBOX; ent->movetype = MOVETYPE_TOSS; break;
		case 3 : ent->solid = SOLID_BBOX; ent->movetype = MOVETYPE_NONE; break;
		case 4 : ent->solid = SOLID_NOT; ent->movetype = MOVETYPE_TOSS; break;
		default: ent->solid = SOLID_NOT; ent->movetype = MOVETYPE_NONE; break;
	}
	if (ent->solid != SOLID_NOT ) {
		if (ent->health > 0) {
			ent->die = model_die;
			ent->takedamage = DAMAGE_YES;
		}
	}

	switch (ent->style)
	{
		case 1 : ent->s.effects |= EF_ANIM01; break;
		case 2 : ent->s.effects |= EF_ANIM23; break;
		case 3 : ent->s.effects |= EF_ANIM_ALL; break;
		case 4 : ent->s.effects |= EF_ANIM_ALLFAST; break;
	}

	// DWH: Rather than use one value (renderfx) we use the
	//      actual values for effects and renderfx. All may
	//      be combined.
	ent->s.effects  |= ent->effects;
	ent->s.renderfx |= ent->renderfx; //broken?


	ent->s.renderfx |= RF_NOSHADOW;
	ent->s.renderfx |= RF_MINLIGHT;


	if (ent->startframe < 0)
		ent->startframe = 0;
	if (!ent->framenumbers)
		ent->framenumbers = 1;
	// Change framenumbers to last frame to play
	ent->framenumbers += ent->startframe;

	if (ent->bleft)
	{
		VectorCopy (ent->bleft, ent->mins);
	}
	else
	{
		if (ent->solid != SOLID_NOT)
		{
			gi.dprintf("%s solid with no bleft vector at %s\n", ent->classname, vtos(ent->s.origin));
			ent->solid = SOLID_NOT;
		}
	}

	if (ent->tright)
	{
		VectorCopy (ent->tright, ent->maxs);
	}
	else
	{
		if (ent->solid != SOLID_NOT)
		{
			gi.dprintf("%s solid with no tright vector at %s\n", ent->classname, vtos(ent->s.origin));
			ent->solid = SOLID_NOT;
		}
	}

//	if(ent->movewith && (ent->solid == SOLID_BBOX))
	if(ent->movewith)
		ent->movetype = MOVETYPE_PUSH;

	if (ent->solid != SOLID_NOT)
		ent->clipmask = CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER;

	if (ent->spawnflags & NO_MODEL)
	{	// For rendering effects to work, we MUST use a model
		ent->s.modelindex = gi.modelindex ("sprites/point.sp2");
		ent->movetype = MOVETYPE_NOCLIP;
	}
	else
	{
		if (ent->spawnflags & PLAYER_MODEL) {
			if(!ent->usermodel || !strlen(ent->usermodel))
				ent->s.modelindex = MAX_MODELS-1;
			else
			{
				if(strstr(ent->usermodel,"tris.md2"))
					sprintf(modelname,"players/%s", ent->usermodel);
				else
					sprintf(modelname,"players/%s/tris.md2", ent->usermodel);
				ent->s.modelindex = gi.modelindex(modelname);
			}
		}
		else
		{
			if(strstr(ent->usermodel,".sp2"))
				sprintf(modelname, "sprites/%s", ent->usermodel);
			else
				sprintf(modelname, "models/%s", ent->usermodel);
			ent->s.modelindex = gi.modelindex (modelname);
		}
		ent->s.frame = ent->startframe;

		//BC let the startframe be any arbitrary frame.
		if (ent->count > 0)
			ent->s.frame = ent->count;
	}

	if (st.noise)
		ent->noise_index = gi.soundindex  (st.noise);
	ent->s.sound = ent->noise_index;

	if (ent->spawnflags & ANIM_ONCE)
		ent->spawnflags |= TOGGLE;

	if (ent->spawnflags & TOGGLE)
	{
		ent->delay = 0;
		ent->use = model_spawn_use;
	}

	if(!(ent->s.effects & ANIM_MASK) && (ent->framenumbers > 1))
	{
		ent->think = modelspawn_think;
		ent->nextthink = level.time + 2*FRAMETIME;
	}

	if (ent->attenuation >= 1)
	{
		ent->svflags |= SVF_NOCLIENT;
	}


	//BC baboo
	if (skill->value >= 2 && ent->usermodel)
	{
		if (strcmp(ent->usermodel,"monsters/npc/tris.md2")==0)
		{
			//spawn mathman audio dummy.
			edict_t *mathEnt;			
			mathEnt = G_Spawn();
			VectorCopy (ent->s.origin, mathEnt->s.origin);
			mathEnt->s.origin[2] += 1;
			SP_target_mathaudio(mathEnt);

			ent->s.skinnum = 7; //mathman.
		}


		if (strcmp(ent->usermodel,"monsters/cat/tris.md2")==0)
		{
			gi.dprintf("mathcat\n");
			ent->s.skinnum = 1; //mathcat.
		}
	}



	gi.linkentity (ent);
}
