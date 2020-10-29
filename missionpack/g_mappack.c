#include "g_local.h"

/*
=============================
Q1 Torch

=============================
*/
/*QUAKED light_torch (0 1 0) (-8 -8 -8) (8 8 8)
The torches from Quake 1 (no model yet)

"light" = The amount of light emitted from the torch
"style" = The style of the light : default 0
*/
void torch_think (edict_t *self)
{
	self->s.frame++;
	if (self->s.frame == 5)
		self->s.frame = 0;

	self->nextthink = level.time + FRAMETIME;
}

void SP_light_torch (edict_t *self) // New function by Beel.
{
	self->class_id = ENTITY_LIGHT_TORCH;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.modelindex = gi.modelindex ("models/torch/tris.md2");
	self->s.frame = 0;
	self->think = torch_think;
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
}

/*
=============================
Small flame

=============================
*/
/*QUAKED light_flame (0 1 0) (-8 -8 0) (8 8 16) START_OFF
Small flame from Q1
Default light value is 300.
Default style is 0.
model=models/objects/fire/"
*/
void smallflame_think (edict_t *self)
{
	if (self->s.frame >= 10) // was 5
		self->s.frame = 0;
	else
		self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}

void bigflame_think (edict_t *self)
{
	self->s.frame++;
	if (self->s.frame == 16)
		self->s.frame = 6;
	
	self->nextthink = level.time + FRAMETIME;
}

void SP_light_flame (edict_t *self) // New function by Beel. (small flame)
{
	self->class_id = ENTITY_LIGHT_FLAME;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	VectorSet (self->mins, -8, -8, 0);
	VectorSet (self->maxs, 8, 8, 16);
	self->s.modelindex = gi.modelindex ("models/objects/fire/tris.md2"); // was models/flame/tris.md2
	self->s.frame = 0;
	self->s.renderfx |=	RF_IR_VISIBLE | RF_FULLBRIGHT;		// PGM
	self->s.effects |= EF_PLASMA;
	// Knightmare- only one size for this model
	//ed - spawnflag of 2 gives a big flame
//	if (self->spawnflags & 2)
//		self->think = bigflame_think;
//	else
	self->nextthink = level.time + FRAMETIME;
	self->think = smallflame_think;
	gi.linkentity (self);
}

/*
==========================
Trigger entities with key presses

==========================
*/
//#define	START_OFF		1

extern void Think_Delay (edict_t *ent);
extern void target_laser_on (edict_t *self);
extern void target_laser_off (edict_t *self);

void Cmd_Trigger_f (edict_t *ent)
{
	char		*targetname;
	edict_t	*t;
	int		arg2;

	arg2 = atoi (gi.argv(2));
	targetname = gi.argv(1);
	t = G_Find (NULL, FOFS(targetname), targetname);

	switch (arg2)
	{
	/*
	=============
	Turn target off
	=============
	*/
		case 1:
			if (!t)
				return;
			if (t->classname == "target_mappack_laser")
			{
				if (t->spawnflags & 1)
					target_laser_off (t);			
			}
			if (t->classname == "mappack_light")
			{
				if (!t->spawnflags & 1)
				{
					gi.configstring (CS_LIGHTS+t->style, "a");
					t->spawnflags |= 1;
				}
			}
			break;
	/*
	=============
	Turn target on
	=============
	*/
		case 2: 
			if (!t)
				return;
			if (t->classname == "target_satan_laser")
			{
				if (t->spawnflags & 1)
				{
				}
				else
					target_laser_on (t);
			}
			if (t->classname == "satan_light")
			{
				if (t->spawnflags & 1)
				{
						gi.configstring (CS_LIGHTS+t->style, "m");
						t->spawnflags &= ~1;
				}
			}
			break;

	/*
	=============
	Just swap em around, or trigger em if not toggleable
	=============
	*/
		default:
			if (!t)
				return;

			t = G_Spawn();
			t->classname = "DelayedUse";
			t->nextthink = level.time + 0.01;
			t->think = Think_Delay;
			t->activator = ent;
			t->target = gi.argv(1);
			break;
	}
}

/*
=============================
Spawning a user defined model

=============================
*/

//ed - added all these spawnflags and stuff

/*QUAKED model_spawn (1 0 0) (-8 -8 -8) (8 8 8) x TOGGLE NOT_IR PLAYER NO_MODEL ANIM_ONCE
Spawns a user defined model, you can specify whether its solid, if so how big the box is, and apply nearly
any effect to the entity.

Spawnflags:
	TOGGLE		Start active, when triggered become inactive
	NOT_IR		The model won't be tinted red when the player has IR goggles on
	PLAYER		Set this if you want to use a player model
	NO_MODEL	Don't use a model. Usefull for placing particle effects and dynamic lights on their own
	ANIM__ONCE	Only play the animation once
-----------------------
Key/Value pairs:
"style" Specifies the animation type to use.
   0: None (unless startframe and framenumbers are used)
   1: ANIM01 - cycle between frames 0 and 1 at 2 hz
   2: ANIM23 - cycle between frames 2 and 3 at 2 hz
   3: ANIM_ALL - cycle through all frames at 2 hz
   4: ANIM_ALLFAST - cycle through all frames at 10 hz

Note: The animation flags override startframe and framenumbers settings you may have enterered. ANIM_ALL and ANIM_ALLFAST don't do what you might think - rather than setting a framerate, these apparently cause the model to cycle through its ENTIRE sequence of animations in 0.5 or 0.1 seconds... which for monster and player models is of course not possible. Looks exceptionally goofy. 

"usermodel"		The model to load (models/ is already coded)
"startframe"	The starting frame : default 0
"framenumbers"  The number of frames you want to display after startframe
"skinnum"		The skin number to use, default 0
"health"		If non-zero and solidstate is 3 or 4 (solid), the entity will be shootable. When destroyed, it blows up with a no-damage explosion. 
"solidstate" 
	1 - not solid at all. These models do not obey any sort of physics. If you place them up in the air or embedded in a wall they will stay there and be perfectly happy about it. 
	2 - solid. These models will "droptofloor" when spawned. If the health value is set they may be damaged. 
	3 - solid. Same as above but not affected by gravity. Model will remain in the same location. 
	4 - not solid but affected by gravity. Model will "droptofloor" when spawned. 
NOTE : if you want the model to be solid then you must enter vector values into the following fields :
"bleft" = the point that is at the bottom left of the models bounding box in a model editor
"tright" = the point that is at the top left of the models bounding box in a model editor

"effects" 
         1: ROTATE Rotate like a weapon
         2: GIB
         8: BLASTER Yellowish orange glow plus particles
        16: ROCKET Rocket trail
        32: GRENADE Grenade trail
        64: HYPERBLASTER BLASTER w/o the particles
       128: BFG Big green ball
       256: COLOR_SHELL
       512: POWERSCREEN Green power shield
     16384: FLIES Ewwww
     32768: QUAD Blue shell
     65536: PENT Red shell
    131072: TELEPORTER Teleporter particles
    262144: FLAG1 Red glow
	524288: FLAG2 Blue glow
   1048576: IONRIPPER 
   2097152: GREENGIB
   4194304: BLUE_HB Blue hyperblaster glow
   8388608: SPINNING_LIGHTS Red spinning lights
  16777216: PLASMA
  33554432: TRAP
  67108864: TRACKER
 134217728: DOUBLE Yellow shell
 268435456: SPHERETRANS Transparent
 536870912: TAGTRAIL
1073741824: HALF_DAMAGE
2147483648: TRACKER_TRAIL

"renderfx" 
     1: MINLIGHT Never completely dark
     2: VIEWERMODEL
     4: WEAPONMODEL
     8: FULLBRIGHT
    16: DEPTHHACK
    32: TRANSLUCENT Transparent
    64: FRAMELERP
   128: BEAM
   512: GLOW Pulsating glow of normal Q2 pickup items
  1024: SHELL_RED
  2048: SHELL_GREEN
  4096: SHELL_BLUE
 32768: IR_VISIBLE
 65536: SHELL_DOUBLE
131072: SHELL_HALF_DAMAGE White shell
262144: USE_DISGUISE 
"movewith" Targetname of the entity to move with 
*/

#define	TOGGLE			2
#define	NOT_IR			4 
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
		if (self->spawnflags & ANIM_ONCE)
		{
			model_spawn_use(self,world,world);
			return;
		}
	}
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity(self);
	if (!strcmp(self->classname, "model_train"))
		train_move_children (self);
}

void model_spawn_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->delay) //we started off
	{
		self->svflags &= ~SVF_NOCLIENT;
		self->delay = 0;
		if (self->framenumbers > 1)
		{
			self->think = modelspawn_think;
			self->nextthink = level.time + FRAMETIME;
		}
		self->s.sound = self->noise_index;
#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
#endif
	}
	else	//we started active
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
	if (self->deathtarget)
	{
		self->target = self->deathtarget;
		G_UseTargets (self, attacker);
	}
	train_kill_children(self);
	BecomeExplosion1(self);
}

#define ANIM_MASK (EF_ANIM01|EF_ANIM23|EF_ANIM_ALL|EF_ANIM_ALLFAST)

void SP_model_spawn (edict_t *ent)
{
	char modelname[256];

	// paranoia check
	if ((!ent->usermodel) && (!ent->spawnflags & NO_MODEL) && !(ent->spawnflags & PLAYER_MODEL))
	{
		gi.dprintf("%s without a model and without NO_MODEL spawnflag at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict(ent);
		return;
	}

	ent->class_id = ENTITY_MODEL_SPAWN;

	switch (ent->solidstate)
	{
		case 1 : ent->solid = SOLID_NOT; ent->movetype = MOVETYPE_NONE; break;
		case 2 : ent->solid = SOLID_BBOX; ent->movetype = MOVETYPE_TOSS; break;
		case 3 : ent->solid = SOLID_BBOX; ent->movetype = MOVETYPE_NONE; break;
		case 4 : ent->solid = SOLID_NOT; ent->movetype = MOVETYPE_TOSS; break;
		default: ent->solid = SOLID_NOT; ent->movetype = MOVETYPE_NONE; break;
	}

	if (ent->solid != SOLID_NOT )
	{
		if (ent->health > 0)
		{
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
	ent->s.renderfx |= ent->renderfx;

	if (ent->startframe < 0)
		ent->startframe = 0;
	if (!ent->framenumbers)
		ent->framenumbers = 1;
	// Change framenumbers to last frame to play
	ent->framenumbers += ent->startframe;

	if (!VectorLength(ent->bleft) && ent->solid == SOLID_BBOX)
	{
		gi.dprintf("%s solid with no bleft vector at %s, using default (-16,-16,-16)\n", ent->classname, vtos(ent->s.origin));
		VectorSet(ent->bleft, -16, -16, -16);
	}
	VectorCopy (ent->bleft, ent->mins);

	if (!VectorLength(ent->tright) && ent->solid == SOLID_BBOX)
	{
		gi.dprintf("%s solid with no tright vector at %, using default (16,16,16)\n", ent->classname, vtos(ent->s.origin));
		VectorSet(ent->tright, 16, 16, 16);
	}
	VectorCopy (ent->tright, ent->maxs);

	if (ent->solid != SOLID_NOT)
		ent->clipmask = CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER;

	if (ent->spawnflags & NO_MODEL)
	{	// For rendering effects to work, we MUST use a model
		ent->s.modelindex = gi.modelindex ("sprites/point.sp2");
		ent->movetype = MOVETYPE_NOCLIP;
	}
	else
	{
		if (ent->spawnflags & PLAYER_MODEL)
		{
			if (!ent->usermodel || !strlen(ent->usermodel))
				ent->s.modelindex = MAX_MODELS-1; //was 255
			else
			{
				if (strstr(ent->usermodel,"tris.md2"))
					Com_sprintf(modelname, sizeof(modelname), "players/%s", ent->usermodel);
				else
					Com_sprintf(modelname, sizeof(modelname), "players/%s/tris.md2", ent->usermodel);
				ent->s.modelindex = gi.modelindex(modelname);
			}
		}
		else
		{
			if (strstr(ent->usermodel,".sp2")) {
				// Knightmare- check for "sprites/" already in path
				if ( !strncmp(ent->usermodel, "sprites/", 8) )
					Com_sprintf(modelname, sizeof(modelname), "%s", ent->usermodel);
				else
					Com_sprintf(modelname, sizeof(modelname), "sprites/%s", ent->usermodel);
			}
			else {
				// Knightmare- check for "models/" already in path
				if ( !strncmp(ent->usermodel, "models/", 7) )
					Com_sprintf(modelname, sizeof(modelname), "%s", ent->usermodel);
				else
					Com_sprintf(modelname, sizeof(modelname), "models/%s", ent->usermodel);
			}
			ent->s.modelindex = gi.modelindex (modelname);
		}
		if (ent->startframe < 0)
		{	
			gi.dprintf("model_spawn with startframe less than 0 at %s\n", vtos(ent->s.origin));
			ent->startframe = 0;
		}
		ent->s.frame = ent->startframe;
	}

	if (st.noise)
		ent->noise_index = gi.soundindex  (st.noise);
	//ent->s.sound = ent->noise_index;
#ifdef LOOP_SOUND_ATTENUATION
	ent->s.attenuation = ent->attenuation;
#endif

	if (ent->skinnum) // Knightmare- selectable skin
		ent->s.skinnum = ent->skinnum;

	if (ent->spawnflags & ANIM_ONCE)
		ent->spawnflags |= TOGGLE;

	if (ent->spawnflags & TOGGLE)
	{
		ent->delay = 0;
		ent->use = model_spawn_use;
	}

	if (!(ent->s.effects & ANIM_MASK) && (ent->framenumbers > 1))
	{
		ent->think = modelspawn_think;
		ent->nextthink = level.time + 2*FRAMETIME;
	}

	if (ent->spawnflags & NOT_IR)
		ent->s.renderfx &= ~RF_IR_VISIBLE;
	else
		ent->s.renderfx |= RF_IR_VISIBLE;

	gi.linkentity (ent);
}

/*QUAKED model_train (1 0 0) (-8 -8 -8) (8 8 8) START_ON TOGGLE BLOCK_STOPS PLAYER NO_MODEL ROTATE ROT_CONST SMOOTH
A moving model. The "team" key allows you to team train entities together.

Spawnflags:
	PLAYER		Set this if you want to use a player model
	NO_MODEL	Don't use a model. Usefull for placing particle effects and	dynamic lights on their own
	SPLINE: If set, the func_train will follow a spline curve between path_corners. What this means is you can create near perfectly smooth curvilinear paths with a handful of path_corners. The train will constantly turn to face the direction it is moving (unless yaw_speed and pitch_speed are negative values). For a couple of examples, see the rottrain and lcraft example maps (available on the downloads page.) The shape of the spline curve is controlled by the location and pitch and yaw angles of the train's path_corners. Train roll angle will vary linearly between path_corner roll angle values (the third component of the angles vector). 
-----------------------
Key/Value pairs:
"style" Specifies the animation type to use.
   0: None (unless startframe and framenumbers are used)
   1: ANIM01 - cycle between frames 0 and 1 at 2 hz
   2: ANIM23 - cycle between frames 2 and 3 at 2 hz
   3: ANIM_ALL - cycle through all frames at 2 hz
   4: ANIM_ALLFAST - cycle through all frames at 10 hz

Note: The animation flags override startframe and framenumbers settings you may have enterered. ANIM_ALL and ANIM_ALLFAST don't do what you might think - rather than setting a framerate, these apparently cause the model to cycle through its ENTIRE sequence of animations in 0.5 or 0.1 seconds... which for monster and player models is of course not possible. Looks exceptionally goofy. 

"usermodel"		The model to load (models/ is already coded)
"startframe"	The starting frame : default 0
"framenumbers"  The number of frames you want to display after startframe
"skinnum"		The skin number to use, default 0
"health"		If non-zero and solidstate is 3 or 4 (solid), the entity will be shootable. When destroyed, it blows up with a no-damage explosion. 
"target"		first path_corner"
"team"			func_train or func_rotating
"solidstate"
     1 = Not solid (default)
     2 = Bounding box
NOTE : if you want the model to be solid then you must enter vector values into the following fields :
"bleft" = the point that is at the bottom left of the models bounding box in a model editor
"tright" = the point that is at the top left of the models bounding box in a model editor

"speed"		How fast the model should move
"accel"		Acceleration
"decel"		Deceleration

"pitch_speed"	(Nose up & Down) in degrees per second (defualt 20)
"yaw_speed"		(Side-to-side "wiggle") in degrees per second (default 20)
"roll_speed"	(Banking) in degrees per second toward the next path corner's set roll
"dmg"			default	2, damage to inflict when blocked
"noise"			Sound model makes while moving(path/file.wav)
"effects" 
         1: ROTATE Rotate like a weapon
         2: GIB
         8: BLASTER Yellowish orange glow plus particles
        16: ROCKET Rocket trail
        32: GRENADE Grenade trail
        64: HYPERBLASTER BLASTER w/o the particles
       128: BFG Big green ball
       256: COLOR_SHELL
       512: POWERSCREEN Green power shield
     16384: FLIES Ewwww
     32768: QUAD Blue shell
     65536: PENT Red shell
    131072: TELEPORTER Teleporter particles
    262144: FLAG1 Red glow
    524288: FLAG2 Blue glow
   1048576: IONRIPPER 
   2097152: GREENGIB
   4194304: BLUE_HB Blue hyperblaster glow
   8388608: SPINNING_LIGHTS Red spinning lights
  16777216: PLASMA
  33554432: TRAP
  67108864: TRACKER
 134217728: DOUBLE Yellow shell
 268435456: SPHERETRANS Transparent
 536870912: TAGTRAIL
1073741824: HALF_DAMAGE
2147483648: TRACKER_TRAIL

"renderfx" 
     1: MINLIGHT Never completely dark
     2: VIEWERMODEL
     4: WEAPONMODEL
     8: FULLBRIGHT
    16: DEPTHHACK
    32: TRANSLUCENT Transparent
    64: FRAMELERP
   128: BEAM
   512: GLOW Pulsating glow of normal Q2 pickup items
  1024: SHELL_RED
  2048: SHELL_GREEN
  4096: SHELL_BLUE
 32768: IR_VISIBLE
 65536: SHELL_DOUBLE
131072: SHELL_HALF_DAMAGE White shell
262144: USE_DISGUISE 

To have other entities move with the model train, set the door pieces' movewith values to the same as the train's targetname and they will move in
unison, and also still be able to move relative to the train themselves, and can be attached and detached using target_movewith.
NOTE: All the pieces must be created after the model train entity, otherwise they will move ahead of it.
*/

#define MODEL_TRAIN_START_ON	1
#define MODEL_TRAIN_TOGGLE		2
#define MODEL_TRAIN_BLOCK_STOPS 4
#define MODEL_TRAIN_ROTATE		32
#define MODEL_TRAIN_ROT_CONST	64
#define	TRAIN_ANIM			32
#define	TRAIN_ANIM_FAST		64
#define MODEL_TRAIN_SMOOTH		128

#define	TRAIN_ROTATE		8
#define	TRAIN_ROT_CONST		16
#define TRAIN_SPLINE        8192

void model_train_animator(edict_t *animator)
{
	edict_t	*train;

	train = animator->owner;
	if (!train || !train->inuse)
	{
		G_FreeEdict(animator);
		return;
	}
	if (Q_stricmp(train->classname, "model_train"))
	{
		G_FreeEdict(animator);
		return;
	}
	animator->nextthink = level.time + FRAMETIME;
	if (VectorLength(train->velocity) == 0)
		return;
	train->s.frame++;
	if (train->s.frame >= train->framenumbers)
		train->s.frame = train->startframe;
	gi.linkentity(train);
}

void SP_model_train (edict_t *self)
{
	SP_model_spawn (self);

	self->class_id = ENTITY_MODEL_TRAIN;

	// Reset s.sound, which SP_model_spawn may have turned on
	self->moveinfo.sound_middle = self->s.sound;
	self->s.sound = 0;

	if (!self->inuse) return;

	// Reset some things from SP_model_spawn
	self->delay = 0;
	self->think = NULL;
	self->nextthink = 0;

	self->s.sound = self->noise_index;

	if (self->health)
	{
		self->die = model_die;
		self->takedamage = DAMAGE_YES;
	}

	if (self->framenumbers > self->startframe+1)
	{
		edict_t	*animator;
		animator = G_Spawn();
		animator->owner = self;
		animator->think = model_train_animator;
		animator->nextthink = level.time + FRAMETIME;
	}
	self->s.frame = self->startframe;
	self->movetype = MOVETYPE_PUSH;

	// Really gross stuff here... translate model_spawn spawnflags
	// to func_train spawnflags. PLAYER_MODEL and NO_MODEL have
	// already been checked in SP_model_spawn and are never re-used,
	// so it's OK to overwrite those.
	if (self->spawnflags & MODEL_TRAIN_ROTATE)
	{
		self->spawnflags &= ~MODEL_TRAIN_ROTATE;
		self->spawnflags |= TRAIN_ROTATE;

	}
	if (self->spawnflags & MODEL_TRAIN_ROT_CONST)
	{
		self->spawnflags &= ~MODEL_TRAIN_ROT_CONST;
		self->spawnflags |= TRAIN_ROT_CONST;
	}
	//Knightmare- change both rotate flags to spline flag
	if ((self->spawnflags & TRAIN_ROTATE) && (self->spawnflags &TRAIN_ROT_CONST))
	{
		self->spawnflags &= ~TRAIN_ROTATE;
		self->spawnflags &= ~TRAIN_ROT_CONST;
		self->spawnflags |= TRAIN_SPLINE;
	}
	if (self->style == 3)
		self->spawnflags |= TRAIN_ANIM;		// 32
	if (self->style == 4)
		self->spawnflags |= TRAIN_ANIM_FAST;	// 64

	// TRAIN_SMOOTH forces trains to go directly to Move_Done from
	// Move_Final rather than slowing down (if necessary) for one
	// frame.
	if (self->spawnflags & MODEL_TRAIN_SMOOTH)
		self->smooth_movement = 1;
	else
		self->smooth_movement = 0;

	self->blocked = train_blocked;
	if (self->spawnflags & MODEL_TRAIN_BLOCK_STOPS)
		self->dmg = 0;
	else
	{
		if (!self->dmg)
			self->dmg = 100;
	}

	if (!self->speed)
		self->speed = 100;

	// Mappack
	if (!self->accel)
		self->moveinfo.accel = self->speed;
	else
		self->moveinfo.accel = self->accel;
	if (!self->decel)
		self->moveinfo.decel = self->speed;
	else
		self->moveinfo.decel = self->decel;
	self->moveinfo.speed = self->speed;

	self->use = train_use;

	gi.linkentity (self);

	if (self->target)
	{
		// start trains on the second frame, to make sure their targets have had
		// a chance to spawn
		self->nextthink = level.time + FRAMETIME;
		self->think = func_train_find;
	}
	else
		gi.dprintf ("model_train without a target at %s\n", vtos(self->s.origin));

}

void SP_model_train_origin (edict_t *self)
{
	self->spawnflags |= TRAIN_ORIGIN;
	SP_model_train (self);
}
