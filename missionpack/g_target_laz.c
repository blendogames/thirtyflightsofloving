// g_target_laz.c
// target entities for the Lazarus mod

#include "g_local.h"

qboolean FindTarget (edict_t *self);

/*
===============================================

	MAPPACK AND LAZARUS ADDITIONS

===============================================
*/

// target_monsterbattle serves the same purpose as target_anger, but 
// ends up turning a dmgteam group of monsters against another dmgteam

void use_target_monsterbattle (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *grouch, *grouchmate;
	edict_t *target, *targetmate;

	grouch = G_Find(NULL,FOFS(targetname),self->target);
	if (!grouch) return;
	if (!grouch->inuse) return;
	target = G_Find(NULL,FOFS(targetname),self->killtarget);
	if (!target) return;
	if (!target->inuse) return;
	if (grouch->dmgteam)
	{
		grouchmate = G_Find(NULL,FOFS(dmgteam),grouch->dmgteam);
		while (grouchmate)
		{
			grouchmate->monsterinfo.aiflags2 |= AI2_FREEFORALL;
			grouchmate = G_Find(grouchmate,FOFS(dmgteam),grouch->dmgteam);
		}
	}
	if (target->dmgteam)
	{
		targetmate = G_Find(NULL,FOFS(dmgteam),target->dmgteam);
		while (targetmate)
		{
			targetmate->monsterinfo.aiflags2 |= AI2_FREEFORALL;
			targetmate = G_Find(targetmate,FOFS(dmgteam),target->dmgteam);
		}
	}
	grouch->enemy = target;
	grouch->monsterinfo.aiflags |= AI_TARGET_ANGER;
	FoundTarget(grouch);

	self->count--;
	if (self->count == 0)
	{ 
		self->think = G_FreeEdict;
		self->nextthink = level.time + 1;
	}
}
void SP_target_monsterbattle (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict(self);
		return;
	}
	if (!self->target)
	{
		gi.dprintf("target_monsterbattle with no target set at %s\n",vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}
	if (!self->killtarget)
	{
		gi.dprintf("target_monsterbattle with no killtarget set at %s\n",vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}
	
	self->class_id = ENTITY_TARGET_MONSTERBATTLE;

	self->use = use_target_monsterbattle;
}


/*QUAKED target_command (1 0 0) (-8 -8 -8) (8 8 8)
Sends a command to the console which gets executed imediately,
unless it requires a restart of the server.

message - command to send
*/
void target_command_use (edict_t *self, edict_t *activator, edict_t *other)
{
   	gi.WriteByte (11);	        
	gi.WriteString (self->message);
	gi.unicast (self, true);
}

void SP_target_command (edict_t *self)
{
	if (!self->message)
	{
		gi.dprintf("target_command with no command, target name is %s at %s", self->targetname, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	self->class_id = ENTITY_TARGET_COMMAND;

	self->use = target_command_use;
	self->svflags = SVF_NOCLIENT;
	gi.linkentity (self);
}

#define	ON		1
#define	OFF		2
#define	TOGGLE	4

/*QUAKED target_set_effect (1 0 0) (-8 -8 -8) (8 8 8)
Changes the effects of the targeted entities.
"target"	- set the properties to this entity

"style" -
 0 = Set
 1 = Remove
 2 = XOR (toggle)

"alpha"	:  Change the alpha value of the target (between 0 and 1)
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
*/

void target_set_effect_use (edict_t *self, edict_t *activator, edict_t *other)
{
	edict_t	*target;

	target = G_Find (NULL, FOFS(targetname), self->target);
	while (target)
	{
		if (self->style == 1)
		{
			target->s.effects &= ~self->effects;
			target->s.renderfx &= ~self->renderfx;
		}
		else if (self->style == 2)
		{
			target->s.effects ^= self->effects;
			target->s.renderfx ^= self->renderfx;
		}
		else
		{
			target->s.effects = self->effects;
			target->s.renderfx = self->renderfx;
		}
#ifdef KMQUAKE2_ENGINE_MOD //Knightmare added
		if ((self->alpha >= 0.0) && (self->alpha <= 1.0))
			target->s.alpha = self->alpha;
#endif
		gi.linkentity(target);
		target = G_Find(target,FOFS(targetname),self->target);
	}
}

void SP_target_set_effect (edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("target_set_effect w/o a target at %s\n",vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	self->class_id = ENTITY_TARGET_SET_EFFECT;

	self->use = target_set_effect_use;
	self->svflags = SVF_NOCLIENT;
	gi.linkentity (self);
}

/*QUAKED target_global_text (1 0 0) (-8 -8 -8) (8 8 8)
Send a string to all clients to be printed.

message - what to print.
*/

void target_global_text_use (edict_t *self, edict_t *activator, edict_t *other)
{
	//FIXME : Send this to all clients notjust the activator.
	//gi.centerprintf (activator, "%s", self->message);

	gi.bprintf (PRINT_CHAT, "%s\n", self->message);
}

void SP_target_global_text (edict_t *self)
{
	if (!self->message)
	{
		gi.dprintf("target_global_text at %s with no message\n", vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}
	if (!self->targetname)
	{
		gi.dprintf("target_global_text at %s with no targetname\n", vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	self->class_id = ENTITY_TARGET_GLOBAL_TEXT;

	self->use = target_global_text_use;
	self->svflags = SVF_NOCLIENT;
	gi.linkentity (self);
}

/*QUAKED target_ignore_player (1 0 0) (-8 -8 -8) (8 8 8) OFF
Switches monsters ignoring the player on and off.

OFF		- switch the effect off.
target	- monster to switch
*/

/*#define	IGNORE_CLIENT	64

void target_ignore_player_use (edict_t *self, edict_t *activator, edict_t *other)
{
	if (self->spawnflags & 1)
		self->enemy->monsterinfo.aiflags &= ~IGNORE_CLIENT;
	else
		self->enemy->monsterinfo.aiflags |= IGNORE_CLIENT;
}

void SP_target_ignore_player (edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("target_ignore_player with out target at %s", vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	self->svflags = SVF_NOCLIENT;

	self->think = VerifyTarget;
	self->nextthink = 1;

	self->use = target_ignore_player_use;
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity (self);
}*/


/*QUAKED target_effect (1 0 0) (-8 -8 -8) (8 8 8) LoopOn LoopOff
Calls an effect when used

"target" ent aimed at
"sounds" splash or pallete index
"count" pixels/splash (1-255)
"wait" steam duration
"speed" steam speed
"style" Select from the list below

0 : Gunshot
1 : Blood
2 : Blaster
3 : Railtrail
4 : Shotgun
5 : Explosion1
6 : Explosion2
7 : Rocket explosion
8 : Grenade explosion
9 : Sparks
10 : Splash
11 : Bubbletrail
12 : Screen sparks
13 : Shield sparks
14 : Bullet sparks
15 : Laser sparks
16 : Parasite attack
17 : Rocket expl (water)
18 : Grenade expl (water)
19 : Medic cable attack
20 : BFG explosion
21 : BFG big explosion
22 : BossTport
23 : BFG laser
24 : Grapple cable
25 : Welding sparks
26 : GreenBlood
28 : Plasma explosion
29 : Tunnel sparks
30 : Blaster2
33 : Lightning
34 : Debugtrail
35 : Plain explosion
36 : Flashlight
38 : Heatbeam
39 : Monster heatbeam
40 : Steam
41 : Bubbletrail2
42 : MoreBlood
43 : Heatbeam sparks
44 : Heatbeam steam
45 : Chainfist smoke
46 : Electric sparks
47 : Tracker explosion
48 : Teleport effect
49 : DBall goal
50 : WidowBeamOut
51 : NukeBlast
52 : WidowSplash
53 : Explosion1 Big
54 : Explosion1 NP
55 : Flechette
*/


/*====================================================================================
   TARGET_EFFECT
======================================================================================*/

/* Unknowns or not supported
TE_FLAME,              32  Rogue flamethrower, never implemented
TE_FORCEWALL,          37  ??
*/

//=========================================================================
/* Spawns an effect at the entity origin
  TE_FLASHLIGHT         36
*/
void target_effect_at (edict_t *self, edict_t *activator)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (self->style);
	gi.WritePosition (self->s.origin);
	gi.WriteShort (self - g_edicts);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}
/* Poor man's target_steam
  TE_STEAM           40
*/
void target_effect_steam (edict_t *self, edict_t *activator)
{
	static int nextid;
	int	wait;

	if (self->wait)
		wait = self->wait*1000;
	else
		wait = 0;

	if (nextid > 20000)
		nextid = nextid %20000;
	nextid++;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (self->style);
	gi.WriteShort (nextid);
	gi.WriteByte (self->count);
	gi.WritePosition (self->s.origin);
	gi.WriteDir (self->movedir);
	gi.WriteByte (self->sounds&0xff);
	gi.WriteShort ( (int)(self->speed) );
	gi.WriteLong ( (int)(wait) );
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectSteam (self->s.origin,self->movedir,self->count,self->sounds,(int)(self->speed),wait,nextid);
}
//=========================================================================
/*
Spawns (style) Splash with (count) particles of (sounds) color at (origin)
moving in (movedir) direction.

  TE_SPLASH             10 Randomly shaded shower of particles
  TE_LASER_SPARKS       15 Splash particles obey gravity
  TE_WELDING_SPARKS     25 Splash particles with flash of light at {origin}
*/
//=========================================================================
void target_effect_splash (edict_t *self, edict_t *activator)
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(self->style);
	gi.WriteByte(self->count);
	gi.WritePosition(self->s.origin);
	gi.WriteDir(self->movedir);
	gi.WriteByte(self->sounds);
	gi.multicast(self->s.origin, MULTICAST_PVS);
}
//======================================================
/*
Spawns a trail of (type) from (start) to (end) and Broadcasts to all
in Potentially Visible Set from vector (origin)

  TE_RAILTRAIL             3 Spawns a blue spiral trail filled with white smoke
  TE_BUBBLETRAIL          11 Spawns a trail of bubbles
  TE_PARASITE_ATTACK      16
  TE_MEDIC_CABLE_ATTACK   19
  TE_BFG_LASER            23 Spawns a green laser
  TE_GRAPPLE_CABLE        24
  TE_RAILTRAIL2           31 NOT IMPLEMENTED IN ENGINE
  TE_DEBUGTRAIL           34
  TE_HEATBEAM,            38 Requires Rogue model
  TE_MONSTER_HEATBEAM,    39 Requires Rogue model
  TE_BUBBLETRAIL2         41
*/
//======================================================
void target_effect_trail (edict_t *self, edict_t *activator)
{
	edict_t	*target;

	if (!self->target) return;
	target = G_Find(NULL,FOFS(targetname),self->target);
	if (!target) return;

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(self->style);
	if ((self->style == TE_PARASITE_ATTACK) || (self->style==TE_MEDIC_CABLE_ATTACK) ||
	   (self->style == TE_HEATBEAM)        || (self->style==TE_MONSTER_HEATBEAM)   ||
	   (self->style == TE_GRAPPLE_CABLE)                                             )
		gi.WriteShort(self-g_edicts);
	gi.WritePosition(self->s.origin);
	gi.WritePosition(target->s.origin);
	if (self->style == TE_GRAPPLE_CABLE) {
		gi.WritePosition(vec3_origin);
	}
	gi.multicast(self->s.origin, MULTICAST_PVS);

	// Lazarus reflections
	if (level.num_reflectors)
	{
		if ((self->style == TE_RAILTRAIL) || (self->style == TE_BUBBLETRAIL) ||
		   (self->style == TE_BFG_LASER) || (self->style == TE_DEBUGTRAIL)  ||
		   (self->style == TE_BUBBLETRAIL2))
		   ReflectTrail (self->style, self->s.origin, target->s.origin, 0, 0, 0);
	}
}
//===========================================================================
/* TE_LIGHTNING   33 Lightning bolt

  Similar but slightly different syntax to trail stuff */
void target_effect_lightning(edict_t *self, edict_t *activator)
{
	edict_t	*target;

	if (!self->target)
		return;
	target = G_Find(NULL,FOFS(targetname),self->target);
	if (!target)
		return;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (self->style);
	gi.WriteShort (target - g_edicts);		// destination entity
	gi.WriteShort (self - g_edicts);		// source entity
	gi.WritePosition (target->s.origin);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}
//===========================================================================
/*
Spawns sparks of (type) from (start) in direction of (movdir) and
Broadcasts to all in Potentially Visible Set from vector (origin)

  TE_GUNSHOT           0 Spawns a grey splash of particles, with a bullet puff
  TE_BLOOD             1 Spawns a spurt of red blood
  TE_BLASTER           2 Spawns a blaster sparks
  TE_SHOTGUN           4 Spawns a small grey splash of spark particles, with a bullet puff
  TE_SPARKS            9 Spawns a red/gold splash of spark particles
  TE_SCREEN_SPARKS    12 Spawns a large green/white splash of sparks
  TE_SHIELD_SPARKS    13 Spawns a large blue/violet splash of sparks
  TE_BULLET_SPARKS    14 Same as TE_SPARKS, with a bullet puff and richochet sound
  TE_GREENBLOOD       26 Spurt of green (actually kinda yellow) blood
  TE_BLUEHYPERBLASTER 27 NOT IMPLEMENTED
  TE_BLASTER2         30 Green/white sparks with a yellow/white flash
  TE_MOREBLOOD        42 
  TE_HEATBEAM_SPARKS  43
  TE_HEATBEAM_STEAM   44
  TE_CHAINFIST_SMOKE  45 
  TE_ELECTRIC_SPARKS  46
  TE_FLECHETTE        55
*/
//======================================================
void target_effect_sparks (edict_t *self, edict_t *activator)
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(self->style);
	gi.WritePosition(self->s.origin);
	if (self->style != TE_CHAINFIST_SMOKE) 
		gi.WriteDir(self->movedir);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectSparks(self->style,self->s.origin,self->movedir);
}
//======================================================
/*
Spawns a (type) effect at (start} and Broadcasts to all in the
Potentially Hearable set from vector (origin)

  TE_EXPLOSION1               5 airburst
  TE_EXPLOSION2               6 ground burst
  TE_ROCKET_EXPLOSION         7 rocket explosion
  TE_GRENADE_EXPLOSION        8 grenade explosion
  TE_ROCKET_EXPLOSION_WATER  17 underwater rocket explosion
  TE_GRENADE_EXPLOSION_WATER 18 underwater grenade explosion
  TE_BFG_EXPLOSION           20 BFG explosion sprite
  TE_BFG_BIGEXPLOSION        21 BFG particle explosion
  TE_BOSSTPORT               22 
  TE_PLASMA_EXPLOSION        28
  TE_PLAIN_EXPLOSION         35
  TE_TRACKER_EXPLOSION       47
  TE_TELEPORT_EFFECT	     48
  TE_DBALL_GOAL              49 Identical to TE_TELEPORT_EFFECT?
  TE_NUKEBLAST               51 
  TE_WIDOWSPLASH             52
  TE_EXPLOSION1_BIG          53  Works, but requires Rogue models/objects/r_explode2
  TE_EXPLOSION1_NP           54
*/
//==============================================================================
void target_effect_explosion (edict_t *self, edict_t *activator)
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(self->style);
	gi.WritePosition(self->s.origin);
	gi.multicast(self->s.origin, MULTICAST_PHS);

	// Lazarus reflections
	if (level.num_reflectors)
		ReflectExplosion (self->style, self->s.origin);
}
//===============================================================================
/*  TE_TUNNEL_SPARKS    29 
    Similar to other splash effects, but Xatrix does some funky things with
	the origin so we'll do the same */

void target_effect_tunnel_sparks (edict_t *self, edict_t *activator)
{
	vec3_t	origin;
	int		i;

	VectorCopy(self->s.origin,origin);
	for (i=0; i<self->count; i++)
	{
		origin[2] += (self->speed * 0.01) * (i + random());
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (self->style);
		gi.WriteByte (1);
		gi.WritePosition (origin);
		gi.WriteDir (vec3_origin);
		gi.WriteByte (self->sounds + (rand()&7));  // color
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}
}
//===============================================================================
/*  TE_WIDOWBEAMOUT     50
*/
void target_effect_widowbeam(edict_t *self, edict_t *activator)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WIDOWBEAMOUT);
	gi.WriteShort (20001);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}
//===============================================================================

void target_effect_use(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & 1) {
		// currently looped on - turn it off
		self->spawnflags &= ~1;
		self->spawnflags |= 2;
		self->nextthink = 0;
		return;
	}
	if (self->spawnflags & 2) {
		// currently looped off - turn it on
		self->spawnflags &= ~2;
		self->spawnflags |= 1;
		self->nextthink = level.time + self->wait;
	}
	if (self->spawnflags & 4) {
		// "if_moving" set. If movewith target isn't moving,
		// don't play
		edict_t	*mover;
		if (!self->movewith) return;
		mover = G_Find(NULL,FOFS(targetname), self->movewith);
		if (!mover) return;
		if (!VectorLength(mover->velocity)) return;
	}
	self->play(self,activator);
}

void target_effect_think(edict_t *self)
{
	self->play(self,NULL);
	self->nextthink = level.time + self->wait;
}
//===============================================================================
void SP_target_effect (edict_t *self)
{
	self->class_id = ENTITY_TARGET_EFFECT;

	if (self->movewith)
		self->movetype = MOVETYPE_PUSH;
	else
		self->movetype = MOVETYPE_NONE;

	switch (self->style )
	{
	case TE_FLASHLIGHT:
		self->play = target_effect_at;
		break;
	case TE_STEAM:
		self->play = target_effect_steam;
		G_SetMovedir (self->s.angles, self->movedir);
		if (!self->count)
			self->count = 32;
		if (!self->sounds)
			self->sounds = 8;
		if (!self->speed)
			self->speed = 75;
		break;
	case TE_SPLASH:
	case TE_LASER_SPARKS:
	case TE_WELDING_SPARKS:
		self->play = target_effect_splash;
		G_SetMovedir (self->s.angles, self->movedir);
		if (!self->count)
			self->count = 32;
		break;
	case TE_RAILTRAIL:
	case TE_BUBBLETRAIL:
	case TE_PARASITE_ATTACK:
	case TE_MEDIC_CABLE_ATTACK:
	case TE_BFG_LASER:
	case TE_GRAPPLE_CABLE:
	case TE_DEBUGTRAIL:
	case TE_HEATBEAM:
	case TE_MONSTER_HEATBEAM:
	case TE_BUBBLETRAIL2:
		if (!self->target)
		{
			gi.dprintf("%s at %s with style=%d needs a target\n",self->classname,vtos(self->s.origin),self->style);
			G_FreeEdict(self);
		} else
			self->play = target_effect_trail;
		break;
	case TE_LIGHTNING:
		if (!self->target)
		{
			gi.dprintf("%s at %s with style=%d needs a target\n",self->classname,vtos(self->s.origin),self->style);
			G_FreeEdict(self);
		}
		else
			self->play = target_effect_lightning;
		break;
	case TE_GUNSHOT:
	case TE_BLOOD:
	case TE_BLASTER:
	case TE_SHOTGUN:
	case TE_SPARKS:
	case TE_SCREEN_SPARKS:
	case TE_SHIELD_SPARKS:
	case TE_BULLET_SPARKS:
	case TE_GREENBLOOD:
	case TE_BLASTER2:
	case TE_MOREBLOOD:
	case TE_HEATBEAM_SPARKS:
	case TE_HEATBEAM_STEAM:
	case TE_CHAINFIST_SMOKE:
	case TE_ELECTRIC_SPARKS:
	case TE_FLECHETTE:
		self->play = target_effect_sparks;
		G_SetMovedir (self->s.angles, self->movedir);
		break;
	case TE_EXPLOSION1:
	case TE_EXPLOSION2:
	case TE_ROCKET_EXPLOSION:
	case TE_GRENADE_EXPLOSION:
	case TE_ROCKET_EXPLOSION_WATER:
	case TE_GRENADE_EXPLOSION_WATER:
	case TE_BFG_EXPLOSION:
	case TE_BFG_BIGEXPLOSION:
	case TE_BOSSTPORT:
	case TE_PLASMA_EXPLOSION:
	case TE_PLAIN_EXPLOSION:
	case TE_TRACKER_EXPLOSION:
	case TE_TELEPORT_EFFECT:
	case TE_DBALL_GOAL:
	case TE_NUKEBLAST:
	case TE_WIDOWSPLASH:
	case TE_EXPLOSION1_BIG:
	case TE_EXPLOSION1_NP:
		self->play = target_effect_explosion;
		break;
	case TE_TUNNEL_SPARKS:
		if (!self->count)
			self->count = 32;
		if (!self->sounds)
			self->sounds = 116; // Light blue, same color used by Xatrix
		self->play = target_effect_tunnel_sparks;
		break;
	case TE_WIDOWBEAMOUT:
		self->play = target_effect_widowbeam;
		G_SetMovedir (self->s.angles, self->movedir);
		break;
	default:
		gi.dprintf("%s at %s: bad style %d\n",self->classname,vtos(self->s.origin),self->style);
	}
	self->use = target_effect_use;
	self->think = target_effect_think;
	if (self->spawnflags & 1)
		self->nextthink = level.time + 1;
}


/*QUAKED target_movewith (1 0 0) (-8 -8 -8) (8 8 8) DETACH
Change or remove an entity's movewith field.  Useful for
attaching or detaching it from a parent entity (usually  a train).

DETACH: Just remove its movewith field

target		: Targetname of entity to attach/detach.
pathtarget	: Targetname of parent entity to attach to.  Not needed to detach.
count		: Number of times it can be used
*/

#if 0
void movewith_detach (edict_t *child)
{
	edict_t	*e;
	edict_t	*parent = NULL;
	int		i;
				
	for (i=1; i<globals.num_edicts && !parent; i++) {
		e = g_edicts + i;
		if (e->movewith_next == child) parent=e;
	}
	if (parent) parent->movewith_next = child->movewith_next;
		
	child->movewith_next = NULL;
	child->movewith = NULL;
	child->movetype = child->oldmovetype;
	// if monster, give 'em a small vertical boost
	if (child->svflags & SVF_MONSTER)
		child->s.origin[2] += 2;
	gi.linkentity(child);
}

void target_movewith_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*target;

	if (!self->target)
		return;
	target = G_Find(NULL,FOFS(targetname),self->target);

	if (self->spawnflags & 1)
	{
		// Detach
		while (target)
		{
			if (target->movewith_ent)
				movewith_detach(target);
			target = G_Find(target,FOFS(targetname), self->target);
		}
	}
	else
	{
		// Attach
		edict_t	*parent;
		edict_t	*e;
		edict_t	*previous;

		parent = G_Find(NULL, FOFS(targetname), self->pathtarget);
		if (!parent || !parent->inuse)
			return;
		while (target)
		{
			if (!target->movewith_ent || (target->movewith_ent != parent) )
			{
				if (target->movewith_ent)
					movewith_detach (target);
		
				target->movewith_ent = parent;
				VectorCopy (parent->s.angles, target->parent_attach_angles);
				VectorCopy (target->s.angles, target->child_attach_angles);
				if (target->oldmovetype < 0)
					target->oldmovetype = target->movetype;
				if (target->movetype != MOVETYPE_NONE)
					target->movetype = MOVETYPE_PUSH;
				VectorCopy (target->mins, target->org_mins);
				VectorCopy (target->maxs, target->org_maxs);
				VectorSubtract (target->s.origin, parent->s.origin, target->movewith_offset);
				e = parent->movewith_next;
				previous = parent;
				while (e)
				{
					previous = e;
					e = previous->movewith_next;
				}
				previous->movewith_next = target;
				gi.linkentity(target);
			}
			target = G_Find(target,FOFS(targetname),self->target);
		}
	}

	self->count--;
	if (!self->count) {
		self->think = G_FreeEdict;
		self->nextthink = level.time + 1;
	}
}
#endif

void target_movewith_use (edict_t *self, edict_t *activator, edict_t *other)
{
	edict_t	*t = NULL;

	if (!self->target)
		return;
	while ((t = G_Find (t, FOFS(targetname), self->target)))
	{
		if (self->spawnflags & 1)
		{
			if (t->oldmovetype) // restore movetype
				t->movetype = t->oldmovetype;
			t->movewith_ent = NULL;
			t->movewith = NULL;
			VectorClear(t->movewith_offset);
			t->movewith_set = 0;
			if (t->svflags & SVF_MONSTER) // toss monsters up a little bit so they won't be stuck
				t->s.origin[2] += 2;
		}
		else if (self->pathtarget)
			t->movewith = self->pathtarget;
		gi.linkentity(t);
	}
	self->count--;
	if (self->count == 0)
	{
		self->nextthink = level.time + FRAMETIME;
		self->think = G_FreeEdict;
	}
}

void SP_target_movewith (edict_t *self)
{
	self->class_id = ENTITY_TARGET_MOVEWITH;

	if (!self->targetname)
		gi.dprintf("target_movewith without a targetname at %s\n", vtos(self->s.origin));
	if (!self->target)
		gi.dprintf("target_movewith without a target at %s\n", vtos(self->s.origin));
	if (!self->pathtarget && !(self->spawnflags & 1))
		gi.dprintf("target_movewith without a pathtarget at %s\n", vtos(self->s.origin));

	self->svflags |= SVF_NOCLIENT;
	self->use = target_movewith_use;

	gi.linkentity (self);
}

/*QUAKED target_change (1 0 0) (-8 -8 -8) (8 8 8) Spawnflags Spawnflags Spawnflags Spawnflags Spawnflags Spawnflags Spawnflags Spawnflags
An entity changer

"newtargetname" The new targetname value you may wish to assign to the targeted entity.

"target" Two values are valid here; the first is the targetname of the entity whose keyvalue(s) you wish to alter,
and the second (optional) value is the new value for "target" that you may wish to assign to the targeted entity.
If two values are used, then they should be separated by a comma.
Syntax: targeted_entity_name,new_target_value.
  
"targetname" The name of the specific target_change.
  
SPAWNFLAGS Virtually any other keyvalue may be assigned, assuming the targeted entity can use it.
*/

void target_change_use (edict_t *self, edict_t *activator, edict_t *other)
{
	char	*buffer;
	char	*target;
	char	*newtarget;
	int		L;
	size_t	bufSize;
	int		newteams=0;
	edict_t	*target_ent;

	if (!self->target)
		return;

	L = (int)strlen(self->target);
	bufSize = L+1;
	buffer = (char *)gi.TagMalloc(bufSize, TAG_LEVEL);
	Com_strcpy (buffer, bufSize, self->target);
	newtarget = strstr(buffer,",");
	if (newtarget)
	{
		*newtarget = 0;
		newtarget++;
	}
	target = buffer;
	target_ent = G_Find(NULL,FOFS(targetname),target);
	while (target_ent)
	{
		if ( newtarget && (strlen(newtarget) > 0) )
			target_ent->target = G_CopyString(newtarget);
		if ( self->newtargetname && (strlen(self->newtargetname) > 0) )
			target_ent->targetname = G_CopyString(self->newtargetname);
		if ( self->team && (strlen(self->team) > 0) )
		{
			target_ent->team = G_CopyString(self->team);
			newteams++;
		}
		if (VectorLength(self->s.angles))
		{
			VectorCopy (self->s.angles, target_ent->s.angles);
			if (target_ent->solid == SOLID_BSP)
				G_SetMovedir (target_ent->s.angles, target_ent->movedir);
		}
		if ( self->deathtarget && (strlen(self->deathtarget) > 0) )
			target_ent->deathtarget = G_CopyString(self->deathtarget);
		if ( self->pathtarget && (strlen(self->pathtarget) > 0) )
			target_ent->pathtarget = G_CopyString(self->pathtarget);
		if ( self->killtarget && (strlen(self->killtarget) > 0) )
			target_ent->killtarget = G_CopyString(self->killtarget);
		if ( self->message && (strlen(self->message) > 0) )
			target_ent->message = G_CopyString(self->message);
		if (self->delay > 0)
			target_ent->delay = self->delay;
		if (self->dmg > 0)
			target_ent->dmg = self->dmg;
		if (self->health > 0)
			target_ent->health = self->health;
		if (self->mass > 0)
			target_ent->mass = self->mass;
		if (self->pitch_speed > 0)
			target_ent->pitch_speed = self->pitch_speed;
		if (self->random > 0)
			target_ent->random = self->random;
		if (self->roll_speed > 0)
			target_ent->roll_speed = self->roll_speed;
		if (self->wait > 0)
			target_ent->wait = self->wait;
		if (self->yaw_speed > 0)
			target_ent->yaw_speed = self->yaw_speed;
		if (self->noise_index)
		{
			if (target_ent->s.sound == target_ent->noise_index)
				target_ent->s.sound = target_ent->noise_index = self->noise_index;
			else
				target_ent->noise_index = self->noise_index;
		}
#ifdef LOOP_SOUND_ATTENUATION
		if (self->attenuation)
		{
			if (target_ent->s.attenuation == target_ent->attenuation)
				target_ent->s.attenuation = target_ent->attenuation = self->attenuation;
			else
				target_ent->attenuation = self->attenuation;
		}
#endif
		if (self->spawnflags)
		{
			target_ent->spawnflags = self->spawnflags;
			// special cases:
			if ( !Q_stricmp(target_ent->classname, "model_train") )
			{
				if (target_ent->spawnflags & 32)
				{
					target_ent->spawnflags &= ~32;
					target_ent->spawnflags |= 8;
				}
				if (target_ent->spawnflags & 64)
				{
					target_ent->spawnflags &= ~64;
					target_ent->spawnflags |= 16;
				}
			}
		}

		// Knightmare- set usermodel and skinnum only for model_spawn/train/turret
		if ( !Q_stricmp(target_ent->classname, "model_spawn") || !Q_stricmp(target_ent->classname, "model_train") || !Q_stricmp(target_ent->classname, "model_turret") )
		{
			char	modelname[256];	// Knightmare added

			if ( self->usermodel  && (strlen(self->usermodel) > 0) )
			{
				if (strstr(self->usermodel,".sp2")) {
					// check for "sprites/" already in path
					if ( !strncmp(self->usermodel, "sprites/", 8) )
						Com_sprintf(modelname, sizeof(modelname), "%s", self->usermodel);
					else
						Com_sprintf(modelname, sizeof(modelname), "sprites/%s", self->usermodel);
				}
				else {
					// check for "models/" already in path
					if ( !strncmp(self->usermodel, "models/", 7) )
						Com_sprintf(modelname, sizeof(modelname), "%s", self->usermodel);
					else
						Com_sprintf(modelname, sizeof(modelname), "models/%s", self->usermodel);
				}
				target_ent->s.modelindex = gi.modelindex (modelname);
			}
			// Set skinnum, -1 = 0
			if (self->skinnum != 0) {
				target_ent->skinnum = self->skinnum;
				target_ent->skinnum = max(0, target_ent->skinnum);
				target_ent->s.skinnum = target_ent->skinnum;
			}
		}
		// Knightmare- set solidstate, style, effects, renderfx, startframe, and framenumbers for model_spawn/train
		if ( !Q_stricmp(target_ent->classname, "model_spawn") || !Q_stricmp(target_ent->classname, "model_train") )
		{
			int		effects = 0;

			if (self->solidstate > 0)
			{
				switch (self->solidstate)
				{
					case 1 : target_ent->solid = SOLID_NOT; target_ent->movetype = MOVETYPE_NONE; break;
					case 2 : target_ent->solid = SOLID_BBOX; target_ent->movetype = MOVETYPE_TOSS; break;
					case 3 : target_ent->solid = SOLID_BBOX; target_ent->movetype = MOVETYPE_NONE; break;
					case 4 : target_ent->solid = SOLID_NOT; target_ent->movetype = MOVETYPE_TOSS; break;
					default: target_ent->solid = SOLID_NOT; target_ent->movetype = MOVETYPE_NONE; break;
				}
				if ( (target_ent->solid != SOLID_NOT) && (target_ent->health > 0) ) {
					target_ent->die = model_die;
					target_ent->takedamage = DAMAGE_YES;
				}
				else {
					target_ent->die = NULL;
					target_ent->takedamage = DAMAGE_NO;
				}
			}

			if (self->style > 0)
			{
				switch (self->style)
				{
					case 1 : effects |= EF_ANIM01; break;
					case 2 : effects |= EF_ANIM23; break;
					case 3 : effects |= EF_ANIM_ALL; break;
					case 4 : effects |= EF_ANIM_ALLFAST; break;
				}
			}
			// Set effects, -1 = 0
			if (self->effects != 0) {
				target_ent->effects = self->effects;
				target_ent->effects = max(0, target_ent->effects);
			}
			if ( (self->style > 0) || (self->effects != 0) ) {
				target_ent->s.effects = (effects | target_ent->effects);
			}
			// Set renderfx, -1 = 0
			if (self->renderfx != 0) {
				target_ent->renderfx = self->renderfx;
				target_ent->renderfx = max(0, target_ent->renderfx);
				target_ent->s.renderfx = target_ent->renderfx;
			}

			// Set startframe, -1 = 0
			if (self->startframe != 0) {
				target_ent->startframe = self->startframe;
				target_ent->startframe = max(0, target_ent->startframe);
			}
			// Set framenumbers, -1 = 0
			if (self->framenumbers != 0) {
				target_ent->framenumbers = self->framenumbers;
				target_ent->framenumbers = max(1, target_ent->framenumbers);
			}
			if ( (self->startframe != 0) || (self->framenumbers != 0) ) {
				// Change framenumbers to last frame to play
				target_ent->framenumbers += target_ent->startframe;
				target_ent->s.frame = target_ent->startframe;
			}
		}
		// end Knightmare

		gi.linkentity(target_ent);
		target_ent = G_Find(target_ent,FOFS(targetname),target);
	}
	gi.TagFree(buffer);
	if (newteams)
		G_FindTeams();
}

void SP_target_change (edict_t *self)
{
	self->class_id = ENTITY_TARGET_CHANGE;

	if (!self->targetname)
		gi.dprintf("target_change without a targetname at %s\n", vtos(self->s.origin));
	if (!self->target)
		gi.dprintf("target_change without a target at %s\n", vtos(self->s.origin));

	self->svflags |= SVF_NOCLIENT;
	self->use = target_change_use;
	if (st.noise) //David Hyde's code
		self->noise_index = gi.soundindex(st.noise);
	gi.linkentity (self);
}

#define ROTATION_NO_LOOP 1
#define ROTATION_RANDOM 2
/*QUAKED target_rotation (.5 .5 .5) (-8 -8 -8) (8 8 8) NO_LOOP RANDOM
A target "cycler." Every time you trigger it, it picks from among its string of targets, either in order, or randomly depending on the spawnflag setting.

"target" Comma-separated targets to choose from, e.g. "targ1,targ2,targ3" (Do not include the quotation marks)
"count" how many times it can be used
*/

void target_rotation_use (edict_t *self, edict_t *activator, edict_t *other)
{
	edict_t	*target;
	int		i, pick;
	char	*p1, *p2;
	char	targetname[256];

	if (self->spawnflags & 2) {
		// random pick
		pick = self->sounds * random();
		if (pick == self->sounds) pick--;
	} else {
		pick = self->mass;
		if (pick == self->sounds) {
			if (self->spawnflags & 1)    // no loop
				return;
			else
				pick = 0;
		}
		self->mass = pick+1;
	}
	p1 = self->target;
	p2 = targetname;
	memset(targetname,0,sizeof(targetname));
	// skip over pick commas
	for (i=0; i<pick; i++) {
		p1 = strstr(p1,",");
		if (p1)
			p1++;
		else
			return;	// should never happen
	}
	while (*p1 != 0 && *p1 != ',') {
		*p2 = *p1;
		p1++;
		p2++;
	}
	target = G_Find(NULL,FOFS(targetname),targetname);
	while (target) {
		if (target->inuse && target->use)
			target->use(target,other,activator);
		target = G_Find(target,FOFS(targetname),targetname);
	}

	self->count--;
	if (self->count == 0)
	{
		self->nextthink = level.time + FRAMETIME;
		self->think = G_FreeEdict;
	}
/*	char	*target_ent = NULL;
	char	*savetarget;
	int		i = 0;

	if (self->spawnflags & ROTATION_RANDOM)	//random cycling
	{
		int j = rand() % self->dmg + 1;
		for ( target_ent = strtok(self->target,","); target_ent != NULL; target_ent = strtok(NULL, ",") )
		{
			i++;
			if (i == j) break;
		}
	}
	else //fire target enumerated by self->health
	{
		for ( target_ent = strtok(self->target,","); target_ent != NULL; target_ent = strtok(NULL, ",") )
		{
			i++;
			if (i == self->health) break;
		}
		self->health++;
		if (self->health > self->dmg) //if at end of string
		{
			if (self->spawnflags & ROTATION_NO_LOOP) //if no looping, remove
			{
				self->use = NULL;
				self->think = G_FreeEdict;
				self->nextthink = level.time + FRAMETIME;
			}
			else //start over
				self->health = 1;
		}
	}

	//now fire selected target
	savetarget = self->target;
	self->target = target_ent;
	G_UseTargets (self, self->activator);
	self->target = savetarget;*/
}

void SP_target_rotation (edict_t *self)
{
	char	*p;

	if (!self->target)
	{
		gi.dprintf("target_rotation without a target at %s\n",vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	self->class_id = ENTITY_TARGET_ROTATION;

	if ( (self->spawnflags & 3) == 3)
	{
		gi.dprintf("target_rotation at %s: NO_LOOP and RANDOM are mutually exclusive.\n");
		self->spawnflags = 2;
	}

	self->use = target_rotation_use;
	self->svflags = SVF_NOCLIENT;
	self->mass = 0;  // index of currently selected target
	self->sounds = 0; // number of comma-delimited targets in target string
	p = self->target;
	while ( (p = strstr(p,",")) != NULL)
	{
		self->sounds++;
		p++;
	}
	self->sounds++;
	
/*	if (!self->targetname)
		gi.dprintf("target_rotation without a targetname at %s\n", vtos(self->s.origin));
	if (!self->target)
		gi.dprintf("target_rotation without a target at %s\n", vtos(self->s.origin));
	if (!strstr(self->target, ","))
	{
		gi.dprintf("target_rotation with less than 2 targets at %s\n", vtos(self->s.origin));
		return;
	}
	else //Count how many different targets we have
	{
		char	*target1 = NULL;
		self->dmg = 0; //dmg is number of targets
		self->health = 1; //start at target 1
		for ( target1 = strtok(self->target,","); target1 != NULL; target1 = strtok(NULL, ",") )
		{
			self->dmg++;
		}
	}

	self->svflags |= SVF_NOCLIENT;
	self->use = target_rotation_use;

	gi.linkentity (self);*/
}

/*QUAKED target_cd (1 0 0) (-8 -8 -8) (8 8 8)
A CD/OGG track player

"count"		number of times it can be used
"sounds"	CD track number; default = 2
"musictrack"	name of OGG track or CD track number, overrides "sounds"
"dmg"		times to loop; default=1
*/

void target_cd_use (edict_t *self, edict_t *activator, edict_t *other)
{
	if (self->musictrack && strlen(self->musictrack))
		gi.configstring (CS_CDTRACK, self->musictrack);
	else
		gi.configstring (CS_CDTRACK, va("%d", self->sounds) );
	if ((self->dmg > 0) && (!deathmatch->value) && (!coop->value))
		stuffcmd(&g_edicts[1],va("cd_loopcount %d\n",self->dmg)); 
	self->count--;
	if (self->count == 0)
	{
		self->nextthink = level.time + 1;
		self->think = G_FreeEdict;
	}
}

void SP_target_cd (edict_t *self)
{
	self->class_id = ENTITY_TARGET_CD;

	if (!self->targetname)
		gi.dprintf("target_cd without a targetname at %s\n", vtos(self->s.origin));
	if (!self->sounds)
		self->sounds = 2;
	if (!self->dmg)
		self->dmg = lazarus_cd_loop->value;

	self->svflags |= SVF_NOCLIENT;
	self->use = target_cd_use;

	gi.linkentity (self);
}

/*QUAKED target_skill (1 0 0) (-8 -8 -8) (8 8 8)
Change skill level on the fly

"targetname" when triggered
"count" number of times it can be used
"style"
0 : Easy
1 : Normal (Default)
2 : Hard
3 : Nightmare
*/

void use_target_skill (edict_t *self, edict_t *other, edict_t *activator)
{
	level.next_skill = self->style + 1;
	self->count--;
	if (self->count == 0)
		G_FreeEdict(self);
}

void SP_target_skill (edict_t *self)
{
	self->class_id = ENTITY_TARGET_SKILL;

	self->use = use_target_skill;
}

/*QUAKED target_sky (1 0 0) (-8 -8 -8) (8 8 8)
Change the level's environment map

"sky" env map name
"count" number of times it can be used
*/

void target_sky_use (edict_t *self, edict_t *activator, edict_t *other)
{
	gi.configstring(CS_SKY,self->pathtarget);
	stuffcmd(&g_edicts[1],va("sky %s\n",self->pathtarget));
	self->count--;
	if (self->count == 0)
	{
		self->nextthink = level.time + FRAMETIME;
		self->think = G_FreeEdict;
	}
}

void SP_target_sky (edict_t *self)
{
	size_t	pathSize;

	if (!st.sky || !*st.sky)
	{
		gi.dprintf("Target_sky with no sky string at %s\n",vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}
	self->class_id = ENTITY_TARGET_SKY;

	pathSize = strlen(st.sky)+1;
	self->pathtarget = gi.TagMalloc(pathSize, TAG_LEVEL);
	Com_strcpy (self->pathtarget, pathSize, st.sky);

	self->svflags |= SVF_NOCLIENT;
	self->use = target_sky_use;

	gi.linkentity (self);
}


/*=============================================================================
 TARGET_FADE fades the screen to a color
 color    = R,G,B components of fade color, 0-1
 alpha    = opacity of fade. 0=no effect, 1=solid color
 fadein   = time in seconds from trigger until full alpha
 fadeout  = time in seconds after fadein+holdtime from full alpha to clear screen
 holdtime = time to hold the effect at full alpha value. -1 = permanent
=============================================================================*/
void use_target_fade (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!activator->client)
		return;

	activator->client->fadestart= level.framenum;
	activator->client->fadein   = level.framenum + self->fadein*10;
	activator->client->fadehold = activator->client->fadein + self->holdtime*10;
	activator->client->fadeout  = activator->client->fadehold + self->fadeout*10;
	activator->client->fadecolor[0] = self->color[0];
	activator->client->fadecolor[1] = self->color[1];
	activator->client->fadecolor[2] = self->color[2];
	activator->client->fadealpha    = self->alpha;

	self->count--;
	if (self->count == 0)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 1;
	}
}

void SP_target_fade (edict_t *self)
{
	self->class_id = ENTITY_TARGET_FADE;

	self->use = use_target_fade;
	if (self->fadein < 0)
		self->fadein = 0;
	if (self->holdtime < 0)
	{
		self->count = 1;
		self->holdtime = 10000;
	}
	if (self->fadeout < 0)
		self->fadeout = 0;
}

/*QUAKED target_rocks (.5 .5 .5) (-8 -8 -8) (8 8 8)
Causes a rock slide when fired
"count"		Number of times you can use it.
"angles"	Angle to throw rocks
"mass"		Debris amount. Default=500
"speed"		How fast the rocks come down in units/sec. Default=400
*/


/*void rock_touch (edict_t *rock, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int damage;

	damage = rock->mass * VectorLength(rock->velocity) * 0.0001;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (rock);
		return;
	}

	if (plane->normal && rock->sounds == 1)
		gi.sound (rock, CHAN_AUTO, gi.soundindex("zer/brik_hit.wav"), 1.0, ATTN_NORM, 0);

	//if (other->client || other->svflags & SVF_MONSTER)
	if (other->takedamage)
		T_Damage (other, rock, rock, rock->velocity, rock->s.origin, plane->normal, damage, 0, 0, MOD_FALLING_ROCKS);
}*/

//void spawn_rock1 (edict_t *self, float speed)
void ThrowRock (edict_t *self, char *modelname, float speed, vec3_t origin, vec3_t size, int mass)
{
	edict_t	*rock;
	vec_t var = speed/5;

	rock = G_Spawn();
	VectorCopy (origin, rock->s.origin);
	gi.setmodel (rock, modelname);
	VectorCopy(size,rock->maxs);
	VectorScale(rock->maxs,0.5,rock->maxs);
	VectorNegate(rock->maxs,rock->mins);
	rock->velocity[0] = speed * self->movedir[0] + var * crandom();
	rock->velocity[1] = speed * self->movedir[1] + var * crandom();
	rock->velocity[2] = speed * self->movedir[2] + var * crandom();
	rock->avelocity[0] = random()*600;
	rock->avelocity[1] = random()*600;
	rock->avelocity[2] = random()*600;
	rock->movetype = MOVETYPE_DEBRIS;
	rock->attenuation = 0.5;
	rock->solid = SOLID_NOT;
	rock->mass = mass;
//	rock->touch = rock_touch;
	rock->nextthink = level.time + 15 + random()*15;
	rock->think = gib_fade;
	rock->classname = "debris";

	gi.linkentity (rock);
}

/*void spawn_rock2 (edict_t *self, float speed)
{
	edict_t	*rock;
	vec3_t	dir;
	vec3_t	forward, right, up;
	vec_t var = speed/5;

	vectoangles (self->s.angles, dir);
	AngleVectors (dir, forward, right, up);

	rock = G_Spawn();
	VectorCopy (self->s.origin, rock->s.origin);
//	VectorScale (self->movedir, speed, rock->velocity);
	rock->velocity[0] = speed * self->movedir[0] + var * crandom();
	rock->velocity[1] = speed * self->movedir[1] + var * crandom();
	rock->velocity[2] = speed * self->movedir[2] + var * crandom();
//	VectorMA (rock->velocity, crandom() * 10.0, forward, rock->velocity);
//	VectorMA (rock->velocity, crandom() * 10.0, right, rock->velocity);

//	VectorSet (rock->avelocity, 300, 300, 300);
	rock->avelocity[0] = random()*600;
	rock->avelocity[1] = random()*600;
	rock->avelocity[2] = random()*600;
	rock->movetype = MOVETYPE_DEBRIS;
	rock->clipmask = MASK_SHOT;
	rock->solid = SOLID_BBOX;
	VectorClear (rock->mins);
	VectorClear (rock->maxs);
	rock->mass = 25;
	rock->sounds = self->sounds;
	rock->s.modelindex = gi.modelindex ("models/objects/rock2/tris.md2");
	rock->touch = rock_touch;
	rock->nextthink = level.time + 10 + random()*10;
	rock->think = gib_fade;
	rock->classname = "rock";

	gi.linkentity (rock);
	SV_AddGravity (rock);
}*/

void target_rocks_use (edict_t *self, edict_t *activator, edict_t *other)
{
	vec3_t	chunkorigin;
	vec3_t	size, source;
	vec_t	mass;
	int		count;
	char	modelname[64];
//	int r1, r2, i;

//	r1 = min (16, (self->mass / 100));
//	r2 = min (16, (self->mass / 25));
	VectorSet(source,8,8,8);
	mass = self->mass;

	// set movedir here- if we're 
	G_SetMovedir2 (self->s.angles, self->movedir);
//	if (self->sounds == 1)
//		gi.sound (self, CHAN_AUTO, gi.soundindex("zer/wall01.wav"), 1.0, ATTN_NORM, 0);
	// big chunks
	if (mass >= 100)
	{
		Com_sprintf(modelname, sizeof(modelname), "models/objects/rock%d/tris.md2",self->style*2+1);
		count = mass / 100;
		if (count > 16)
			count = 16;
		VectorSet(size,8,8,8);
		while (count--)
		{
			chunkorigin[0] = self->s.origin[0] + crandom() * source[0];
			chunkorigin[1] = self->s.origin[1] + crandom() * source[1];
			chunkorigin[2] = self->s.origin[2] + crandom() * source[2];
			ThrowRock (self, modelname, self->speed, chunkorigin, size, 100);
		}
	}
	// small chunks
	count = mass / 25;
	Com_sprintf(modelname, sizeof(modelname), "models/objects/rock%d/tris.md2",self->style*2+2);
	if (count > 16)
		count = 16;
	VectorSet(size,4,4,4);
	while (count--)
	{
		chunkorigin[0] = self->s.origin[0] + crandom() * source[0];
		chunkorigin[1] = self->s.origin[1] + crandom() * source[1];
		chunkorigin[2] = self->s.origin[2] + crandom() * source[2];
		ThrowRock (self, modelname, self->speed, chunkorigin, size, 25);
	}
	if (self->dmg)
		T_RadiusDamage (self, activator, self->dmg, NULL, self->dmg+40, MOD_SPLASH);

/*	for (i = 0; i < r1; i++)
		spawn_rock1 (self, self->speed);
	for (i = 0; i < r2; i++)
		spawn_rock2 (self, self->speed);*/

	self->count--;
	if (self->count == 0)
	{
		self->nextthink = level.time + FRAMETIME;
		self->think = G_FreeEdict;
	}
}

void SP_target_rocks (edict_t *self)
{
	self->class_id = ENTITY_TARGET_ROCKS;

	// precache
	gi.modelindex ("models/objects/rock1/tris.md2");
	gi.modelindex ("models/objects/rock2/tris.md2");

	if (!self->targetname)
		gi.dprintf("target_rocks without a targetname at %s\n", vtos(self->s.origin));
	if (!self->mass)
		self->mass = 500;
	if (!self->speed)
		self->speed = 400;

//	G_SetMovedir  (self->s.angles, self->movedir);

	self->svflags |= SVF_NOCLIENT;
	self->use = target_rocks_use;

	gi.linkentity (self);
}

/*QUAKED target_clone (1 0 0) (-8 -8 -8) (8 8 8) START_ON SET_MOVEDIR
Clones entities, works the same as target_bmodel_spawner

START_ON:		It will spawn the entity clone at map load.

SET_MOVEDIR:	Set the door, button, etc's movedir using move_angles instead of copying it from the parent entity.

"targetname"	Name of the specific target_bmodel_spawner.
"source"		Targetname of the brush model entity which is to be used as a model reference.
"count"			When non-zero, specifies the number of times the target_bmodel_spawner will be called before being auto-killtargeted. Default=0.

"angle"			facing angle of the spawned brush entity on the XY plane, relative to its referenced parent model. Default=0.
"angles"		facing angle of the spawned brush entity in 3 dimensions, relative to its referenced parent model, as defined by pitch, yaw, and roll. Default=0 0 0.
"move_angles"	Movement angle for the spawned brush entity (pitch, yaw, roll[ignored])
"alpha"			Alpha value for the spawned entity (between 0 and 1)
"target"		value which will be inherited by the spawned clone. If no value is given to this key, then the clone will not have a target.
"followtarget"	Followtarget for new bmodel, specific to func_door_swinging. If no value is given to this key, then the clone will not have a followtarget.
"deathtarget"	value which will be inherited by the spawned clone. If no value is given to this key, then the clone will not have a deathtarget.
"pathtarget"	value which will be inherited by the spawned clone. If no value is given to this key, then the clone will not have a pathtarget.
"killtarget"	value which will be inherited by the spawned clone. If no value is given to this key, then the clone will not have a killtarget.
"newtargetname"	Targetname value which will be inherited by the spawned clone. If no value is given to this key, then the clone will not have a targetname.
"newteam"		Team value which will be inherited by the spawned clone. If no value is given to this key, then the clone will not be on a team. (This should be used with START_ON, so that there's no chance one teammate can be in the process of moving while another teammate is being spawned).
"team"			value which will be inherited by the spawned clone. If no value is given to this key, then the clone will not be on a team. (This should be used with START_ON, so that there's no chance one teammate can be in the process of moving while another teammate is being spawned).
*/

//void button_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
//void Think_CalcMoveSpeed (edict_t *self);
//void Think_SpawnDoorTrigger (edict_t *ent);
//void func_train_find (edict_t *self); 

void clone (edict_t *self)
{
	edict_t	*ent;
	edict_t	*source;
	size_t	classSize;

	self->nextthink = 0;

	source = G_Find (NULL, FOFS(targetname), self->source);
	if (!source)
	{
		gi.dprintf("%s at %s, source not found\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}
	if (!source->classname)
	{
		gi.dprintf("%s at %s, source at %s has no classname\n",
			self->classname, vtos(self->s.origin), vtos(source->s.origin));
		G_FreeEdict(self);
		return;
	}
	if (!source->s.modelindex)
	{
		gi.dprintf("%s at %s, source %s at %s not a model entity\n",
			self->classname, vtos(self->s.origin), source->classname, vtos(source->s.origin));
		G_FreeEdict(self);
		return;
	}
	// spawn and copy origin and angles
	ent = G_Spawn();
	VectorCopy(self->s.origin, ent->s.origin);
	VectorCopy(self->s.angles, ent->s.angles);
	if (self->spawnflags & 2)
		G_SetMovedir2 (self->move_angles, ent->movedir);
	else
		VectorCopy(source->movedir,ent->movedir);

	ent->model = source->model;
	ent->s.modelindex = source->s.modelindex;
	
	// copy fields from source
	classSize = strlen(source->classname)+1;
	ent->classname = gi.TagMalloc(classSize, TAG_LEVEL);
	Com_strcpy (ent->classname, classSize, source->classname);
//	ent->classname = source->classname;
	ent->spawnflags = source->spawnflags;
	ent->svflags = source->svflags;
	VectorCopy(source->mins,ent->mins);
	VectorCopy(source->maxs,ent->maxs);
	VectorCopy(source->size,ent->size);
	ent->solid = source->solid;
	ent->clipmask = source->clipmask;
	ent->movetype = source->movetype;
	ent->mass = source->mass;
	ent->speed = source->speed;
	ent->accel = source->accel;
	ent->decel = source->decel;
	ent->lip = source->lip;
	ent->distance = source->distance;
	ent->health = source->health;
	ent->max_health = source->max_health;
	ent->takedamage = source->takedamage;
	ent->dmg = source->dmg;
	ent->count = source->count;
	ent->sounds = source->sounds; 
	ent->noise_index = source->noise_index;
	ent->noise_index2 = source->noise_index2;
	ent->wait = source->wait;
	ent->delay = source->delay;
	ent->random = source->random;
	ent->style = source->style;
	ent->flags = source->flags;
	ent->blocked = source->blocked;
	ent->touch = source->touch;
	ent->use = source->use;
	ent->pain = source->pain;
	ent->die = source->die;
	ent->renderfx = source->renderfx;
	ent->s.effects = source->s.effects;
#ifdef KMQUAKE2_ENGINE_MOD //Knightmare added
	ent->s.alpha = source->s.alpha;
#endif
	ent->s.skinnum = source->s.skinnum;
	ent->skinnum = source->skinnum;
	ent->gib_type = source->gib_type;
	ent->item = source->item;
	ent->moveinfo.sound_start = source->moveinfo.sound_start;
	ent->moveinfo.sound_middle = source->moveinfo.sound_middle;
	ent->moveinfo.sound_end = source->moveinfo.sound_end; 
	ent->pitch_speed = source->pitch_speed;
	ent->yaw_speed = source->yaw_speed;
	ent->roll_speed = source->roll_speed;

	if (VectorLength(ent->s.angles) != 0)
	{
		if (ent->s.angles[YAW] == 90 || ent->s.angles[YAW] == 270)
		{ // We're correct for these angles, not even gonna bother with others
			vec_t temp;
			temp = ent->size[0];
			ent->size[0] = ent->size[1];
			ent->size[1] = temp;
			temp = ent->mins[0];
			if (ent->s.angles[YAW] == 90)
			{
				ent->mins[0] = -ent->maxs[1]; 
				ent->maxs[1] = ent->maxs[0];
				ent->maxs[0] = -ent->mins[1];
				ent->mins[1] = temp;
			}
			else
			{
				ent->mins[0] = ent->mins[1];
				ent->mins[1] = -ent->maxs[0];
				ent->maxs[0] = ent->maxs[1]; 
				ent->maxs[1] = -temp;
			}
		} 
		vectoangles(ent->movedir,ent->movedir);
		ent->movedir[PITCH] += ent->s.angles[PITCH];
		ent->movedir[YAW] += ent->s.angles[YAW];
		ent->movedir[ROLL] += ent->s.angles[ROLL];
		if (ent->movedir[PITCH] > 360)
			ent->movedir[PITCH] -= 360;
		if (ent->movedir[YAW]   > 360)
			ent->movedir[YAW] -= 360;
		if (ent->movedir[ROLL]  > 360)
			ent->movedir[ROLL] -= 360;
		AngleVectors(ent->movedir,ent->movedir,NULL,NULL);
	}
	VectorAdd(ent->s.origin,ent->mins,ent->absmin);
	VectorAdd(ent->s.origin,ent->maxs,ent->absmax); 

	//fields from bmodel spawner
	if (self->target)
		ent->target = G_CopyString(self->target);
	if (self->followtarget)
		ent->followtarget = G_CopyString(self->followtarget);
	if (self->deathtarget)
		ent->deathtarget = G_CopyString(self->deathtarget);
	if (self->pathtarget)
		ent->pathtarget = G_CopyString(self->pathtarget);
	if (self->killtarget)
		ent->killtarget = G_CopyString(self->killtarget);
	if (self->newtargetname)
		ent->targetname = G_CopyString(self->newtargetname); 
	if (self->team)
		ent->team = G_CopyString(self->team);
	if (self->newteam)
		ent->team = G_CopyString(self->newteam);
#ifdef KMQUAKE2_ENGINE_MOD //Knightmare added
	if ((self->alpha >= 0.0) && (self->alpha <= 1.0))
		ent->s.alpha = self->alpha;
#endif
	ent->svflags |= SVF_CLONED; //mark this entity as cloned
	ReInitialize_Entity(ent); //call its spawn function

	//set up orgin offset
	VectorAdd(ent->absmin, ent->absmax, ent->origin_offset);
	VectorScale(ent->origin_offset, 0.5, ent->origin_offset);
	VectorSubtract(ent->origin_offset, ent->s.origin, ent->origin_offset);
	//Kill anything in the way
	gi.unlinkentity(ent);
	//Knightmare- only killbox if spawned entity is solid
	if (ent->solid == SOLID_BSP || ent->solid == SOLID_BBOX)
		KillBox(ent);
	gi.linkentity(ent);

	self->count--;
	if (self->count == 0)
	{
		self->nextthink = level.time + FRAMETIME;
		self->think = G_FreeEdict;
	}
}

void target_clone_use (edict_t *self, edict_t *activator, edict_t *other)
{
	clone (self);
}

void SP_target_clone (edict_t *self)
{
	if (!self->targetname && !(self->spawnflags & 1))
	{
		gi.dprintf("%s without a targetname and without start_on at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}
	if (!self->source)
	{
		gi.dprintf("%s without a source at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	self->class_id = ENTITY_TARGET_CLONE;

	self->svflags |= SVF_NOCLIENT;
	self->use = target_clone_use;
	gi.linkentity (self);
	if (self->spawnflags & 1) //START_ON
	{
		self->think = clone;
		self->nextthink = level.time + FRAMETIME;
	}
}

/*=====================================================================================
 TARGET_ATTRACTOR - pulls target entity towards its origin
 target     - Targetname of entity to attract. Ignored if PLAYER spawnflag is set
 pathtarget - Entity or entities to "use" when distance criteria is met.
 speed      - Minimum speed to pull target with. Must use a value > sv_gravity/10 
              to overcome gravity when pulling up.
 distance   - When target is within "distance" units of target_attractor, attraction
              is shut off. Use a value < 0 to hold target in place. 0 will be reset
              to 1.
 sounds     - effect to use. ONLY VALID for PLAYER or MONSTER, not target. If sounds
              is non-zero, SINGLE_TARGET and SIGHT are automatically set
      0 = none
      1 = medic cable
      2 = green laser
              
 Spawnflags:   1 - START_ON
               2 - PLAYER (attract player, ignore "target"
               4 - NO_GRAVITY - turns off gravity for target. W/O this flag you'll
                   get an annoying jitter when pulling players up.
               8 - MONSTER - attract ALL monsters, ignore "target"
              16 - SIGHT - must have LOS to target to attract it
              32 - SINGLE_TARGET - will select best target
              64 - PATHTARGET_FIRE - used internally only
=======================================================================================*/
#define ATTRACTOR_ON          1
#define ATTRACTOR_PLAYER      2
#define ATTRACTOR_NO_GRAVITY  4
#define ATTRACTOR_MONSTER     8
#define ATTRACTOR_SIGHT      16
#define ATTRACTOR_SINGLE     32
#define ATTRACTOR_PATHTARGET 64

void target_attractor_think_single (edict_t *self)
{
	edict_t	*ent, *target, *previous_target;
	trace_t	tr;
	vec3_t	dir, targ_org;
	vec_t	dist, speed;
	vec_t	best_dist;
	vec3_t	forward, right;
	int		i;
	int		num_targets = 0;
	
	if ( !(self->spawnflags & ATTRACTOR_ON) ) return;

	previous_target = self->target_ent;
	target      = NULL;
	best_dist   = WORLD_SIZE;	// was 8192

	if (self->spawnflags & ATTRACTOR_PLAYER)
	{
		for (i=1, ent=&g_edicts[i]; i<=game.maxclients; i++, ent++)
		{
			if (!ent->inuse) continue;
			if (ent->health <= 0) continue;
			num_targets++;
			VectorSubtract(self->s.origin,ent->s.origin,dir);
			dist = VectorLength(dir);
			if (dist > self->moveinfo.distance) continue;
			if (self->spawnflags & ATTRACTOR_SIGHT)
			{
				tr = gi.trace(self->s.origin,vec3_origin,vec3_origin,ent->s.origin,NULL,MASK_OPAQUE | MASK_SHOT);
				if (tr.ent != ent) continue;
			}
			if (dist < best_dist)
			{
				best_dist = dist;
				target = ent;
			}
		}
	}
	if (self->spawnflags & ATTRACTOR_MONSTER)
	{
		for (i=1, ent=&g_edicts[i]; i<=globals.num_edicts; i++, ent++)
		{
			if (!ent->inuse) continue;
			if (ent->health <= 0) continue;
			if (!(ent->svflags & SVF_MONSTER)) continue;
			num_targets++;
			VectorSubtract(self->s.origin,ent->s.origin,dir);
			dist = VectorLength(dir);
			if (dist > self->moveinfo.distance) continue;
			if (self->spawnflags & ATTRACTOR_SIGHT)
			{
				tr = gi.trace(self->s.origin,vec3_origin,vec3_origin,ent->s.origin,NULL,MASK_OPAQUE | MASK_SHOT);
				if (tr.ent != ent) continue;
			}
			if (dist < best_dist)
			{
				best_dist = dist;
				target = ent;
			}
		}
	}
	if (!(self->spawnflags & (ATTRACTOR_PLAYER | ATTRACTOR_MONSTER)))
	{
		ent = G_Find(NULL,FOFS(targetname),self->target);
		while (ent)
		{
			if (!ent->inuse) continue;
			num_targets++;
			VectorAdd(ent->s.origin,ent->origin_offset,targ_org);
			VectorSubtract(self->s.origin,targ_org,dir);
			dist = VectorLength(dir);
			if (dist > self->moveinfo.distance) continue;
			if (self->spawnflags & ATTRACTOR_SIGHT)
			{
				tr = gi.trace(self->s.origin,vec3_origin,vec3_origin,targ_org,NULL,MASK_OPAQUE | MASK_SHOT);
				if (tr.ent != ent) continue;
			}
			if (dist < best_dist)
			{
				best_dist = dist;
				target = ent;
			}
			ent = G_Find(ent,FOFS(targetname),self->target);
		}
	}
	self->target_ent = target;
	if (!target)
	{
		if (num_targets > 0) self->nextthink = level.time + FRAMETIME;
		return;
	}

	if (target != previous_target)
		self->moveinfo.speed = 0;

	if (self->moveinfo.speed != self->speed)
	{
		if (self->speed > 0)
			self->moveinfo.speed = min(self->speed, self->moveinfo.speed + self->accel);
		else
			self->moveinfo.speed = max(self->speed, self->moveinfo.speed + self->accel);
	}

	VectorAdd(target->s.origin,target->origin_offset,targ_org);
	VectorSubtract(self->s.origin,targ_org,dir);
	dist = VectorLength(dir);
	if (readout->value) gi.dprintf("distance=%g, pull speed=%g\n",dist,self->moveinfo.speed);

	if ((self->pathtarget) && (self->spawnflags & ATTRACTOR_PATHTARGET))
	{
		if (dist == 0)
		{
			// fire pathtarget when close
			ent = G_Find(NULL,FOFS(targetname),self->pathtarget);
			while (ent) {
				if (ent->use)
					ent->use(ent,self,self);
				ent = G_Find(ent,FOFS(targetname),self->pathtarget);
			}
			self->spawnflags &= ~ATTRACTOR_PATHTARGET;
		}
	}
	VectorNormalize(dir);
	speed = VectorNormalize(target->velocity);
	speed = max(fabs(self->moveinfo.speed),speed);
	if (self->moveinfo.speed < 0) speed = -speed;
	if (speed > dist*10)
	{
		speed = dist*10;
		VectorScale(dir,speed,target->velocity);
		// if NO_GRAVITY is NOT set, and target would normally be affected by gravity,
		// counteract gravity during the last move
		if ( !(self->spawnflags & ATTRACTOR_NO_GRAVITY) )
		{
			if ( (target->movetype == MOVETYPE_BOUNCE  ) ||
				(target->movetype == MOVETYPE_PUSHABLE) ||
				(target->movetype == MOVETYPE_STEP    ) ||
				(target->movetype == MOVETYPE_TOSS    ) ||
				(target->movetype == MOVETYPE_DEBRIS  )   )
			{
				target->velocity[2] += target->gravity * sv_gravity->value * FRAMETIME;
			}
		}
	}
	else
		VectorScale(dir,speed,target->velocity);
	// Add attractor velocity in case it's a movewith deal
	VectorAdd(target->velocity,self->velocity,target->velocity);
	if (target->client)
	{
		float	scale;
		if (target->groundentity || target->waterlevel > 1)
		{
			if (target->groundentity)
				scale = 0.75;
			else
				scale = 0.375;
			// Players - add movement stuff so he MAY be able to escape
			AngleVectors (target->client->v_angle, forward, right, NULL);
			for (i=0 ; i<3 ; i++)
				target->velocity[i] += scale * forward[i] * target->client->ucmd.forwardmove +
				scale * right[i]   * target->client->ucmd.sidemove; 
			target->velocity[2] += scale * target->client->ucmd.upmove;
		}
	}
	// If target is on the ground and attractor is overhead, give 'em a little nudge.
	// This is only really necessary for players
	if (target->groundentity && (self->s.origin[2] > target->absmax[2]))
	{
		target->s.origin[2] += 1;
		target->groundentity = NULL;
	}

	if (self->sounds)
	{
		vec3_t	new_origin;

		if (target->client)
			VectorCopy(target->s.origin,new_origin);
		else
			VectorMA(targ_org,FRAMETIME,target->velocity,new_origin);
		
		switch(self->sounds)
		{
		case 1:
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_MEDIC_CABLE_ATTACK);
			gi.WriteShort(self-g_edicts);
			gi.WritePosition(self->s.origin);
			gi.WritePosition(new_origin);
			gi.multicast(self->s.origin, MULTICAST_PVS);
			break;
		case 2:
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BFG_LASER);
			gi.WritePosition(self->s.origin);
			gi.WritePosition(new_origin);
			gi.multicast(self->s.origin, MULTICAST_PVS);
		}
	}

	
	if (self->spawnflags & ATTRACTOR_NO_GRAVITY)
		target->gravity_debounce_time = level.time + 2*FRAMETIME;
	gi.linkentity(target);
	
	if (!num_targets)
	{
		// shut 'er down
		self->spawnflags &= ~ATTRACTOR_ON;
	}
	else
	{
		self->nextthink = level.time + FRAMETIME;
	}
}

void target_attractor_think (edict_t *self)
{
	edict_t	*ent, *target;
	trace_t	tr;
	vec3_t	dir, targ_org;
	vec_t	dist, speed;
	vec3_t	forward, right;
	int		i;
	int		ent_start;
	int		num_targets = 0;

	if ( !(self->spawnflags & ATTRACTOR_ON) ) return;

	if (self->moveinfo.speed != self->speed)
	{
		if (self->speed > 0)
			self->moveinfo.speed = min(self->speed, self->moveinfo.speed + self->accel);
		else
			self->moveinfo.speed = max(self->speed, self->moveinfo.speed + self->accel);
	}

	target = NULL;
	ent_start = 1;
	while (true)
	{
		if (self->spawnflags & (ATTRACTOR_PLAYER | ATTRACTOR_MONSTER))
		{
			target = NULL;
			for (i=ent_start, ent=&g_edicts[ent_start];i<globals.num_edicts && !target; i++, ent++)
			{
				if ((self->spawnflags & ATTRACTOR_PLAYER) && ent->client && ent->inuse)
				{
					target = ent;
					ent_start = i+1;
					continue;
				}
				if ((self->spawnflags & ATTRACTOR_MONSTER) && (ent->svflags & SVF_MONSTER) && (ent->inuse))
				{
					target = ent;
					ent_start = i+1;
				}
			}
		}
		else
			target = G_Find(target,FOFS(targetname),self->target);
		if (!target) break;
		if (!target->inuse) continue;
		if ( ((target->client) || (target->svflags & SVF_MONSTER)) && (target->health <= 0)) continue;
		num_targets++;
		
		VectorAdd(target->s.origin,target->origin_offset,targ_org);
		VectorSubtract(self->s.origin,targ_org,dir);
		dist = VectorLength(dir);

		if (self->spawnflags & ATTRACTOR_SIGHT)
		{
			tr = gi.trace(self->s.origin,vec3_origin,vec3_origin,target->s.origin,NULL,MASK_OPAQUE | MASK_SHOT);
			if (tr.ent != target) continue;
		}

		if (readout->value) gi.dprintf("distance=%g, pull speed=%g\n",dist,self->moveinfo.speed);
		if (dist > self->moveinfo.distance)
			continue;
		
		if ((self->pathtarget) && (self->spawnflags & ATTRACTOR_PATHTARGET))
		{
			if (dist == 0)
			{
				// fire pathtarget when close
				ent = G_Find(NULL,FOFS(targetname),self->pathtarget);
				while (ent)
				{
					if (ent->use)
						ent->use(ent,self,self);
					ent = G_Find(ent,FOFS(targetname),self->pathtarget);
				}
				self->spawnflags &= ~ATTRACTOR_PATHTARGET;
			}
		}
		VectorNormalize(dir);
		speed = VectorNormalize(target->velocity);
		speed = max(fabs(self->moveinfo.speed),speed);
		if (self->moveinfo.speed < 0) speed = -speed;
		if (speed > dist*10)
		{
			speed = dist*10;
			VectorScale(dir,speed,target->velocity);
			// if NO_GRAVITY is NOT set, and target would normally be affected by gravity,
			// counteract gravity during the last move
			if ( !(self->spawnflags & ATTRACTOR_NO_GRAVITY) )
			{
				if ( (target->movetype == MOVETYPE_BOUNCE  ) ||
					(target->movetype == MOVETYPE_PUSHABLE) ||
					(target->movetype == MOVETYPE_STEP    ) ||
					(target->movetype == MOVETYPE_TOSS    ) ||
					(target->movetype == MOVETYPE_DEBRIS  )   )
				{
					target->velocity[2] += target->gravity * sv_gravity->value * FRAMETIME;
				}
			}
		}
		else
			VectorScale(dir,speed,target->velocity);
		// Add attractor velocity in case it's a movewith deal
		VectorAdd(target->velocity,self->velocity,target->velocity);
		if (target->client)
		{
			float	scale;
			if (target->groundentity || target->waterlevel > 1)
			{
				if (target->groundentity)
					scale = 0.75;
				else
					scale = 0.375;
				// Players - add movement stuff so he MAY be able to escape
				AngleVectors (target->client->v_angle, forward, right, NULL);
				for (i=0 ; i<3 ; i++)
					target->velocity[i] += scale * forward[i] * target->client->ucmd.forwardmove +
					                       scale * right[i]   * target->client->ucmd.sidemove; 
				target->velocity[2] += scale * target->client->ucmd.upmove;
			}
		}
		// If target is on the ground and attractor is overhead, give 'em a little nudge.
		// This is only really necessary for players
		if (target->groundentity && (self->s.origin[2] > target->absmax[2]))
		{
			target->s.origin[2] += 1;
			target->groundentity = NULL;
		}
		if (self->spawnflags & ATTRACTOR_NO_GRAVITY)
			target->gravity_debounce_time = level.time + 2*FRAMETIME;
		gi.linkentity(target);
	}
	if (!num_targets)
	{
		// shut 'er down
		self->spawnflags &= ~ATTRACTOR_ON;
	}
	else
	{
		self->nextthink = level.time + FRAMETIME;
	}
}

void use_target_attractor(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & ATTRACTOR_ON)
	{
		self->count--;
		if (self->count == 0)
		{
			self->think = G_FreeEdict;
			self->nextthink = level.time + 1;
		}
		else
		{
			self->spawnflags &= ~ATTRACTOR_ON;
			self->s.sound = 0;
			self->target_ent  = NULL;
			self->nextthink   = 0;
		}
	}
	else
	{
		self->spawnflags |= (ATTRACTOR_ON + ATTRACTOR_PATHTARGET);
		self->s.sound = self->noise_index;
	#ifdef LOOP_SOUND_ATTENUATION
		self->s.attenuation = self->attenuation;
	#endif
		if (self->spawnflags & ATTRACTOR_SINGLE)
			self->think = target_attractor_think_single;
		else
			self->think = target_attractor_think;
		self->moveinfo.speed = 0;
		gi.linkentity(self);
		self->think(self);
	}
}

void SP_target_attractor (edict_t *self)
{
	if (!self->target && !(self->spawnflags & ATTRACTOR_PLAYER) &&
		!(self->spawnflags & ATTRACTOR_MONSTER))
	{
		gi.dprintf("target_attractor without a target at %s\n",vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}
	self->class_id = ENTITY_TARGET_ATTRACTOR;

	if (self->sounds)
	{
//		if ((self->spawnflags & ATTRACTOR_PLAYER) || (self->spawnflags & ATTRACTOR_MONSTER)) {
			self->spawnflags |= (ATTRACTOR_SIGHT | ATTRACTOR_SINGLE);
//		} else {
//			gi.dprintf("Target_attractor sounds key is only valid\n"
//				       "for PLAYER or MONSTER. Setting sounds=0\n");
//		}
	}
	if (self->distance)
		st.distance = self->distance;
	if (st.distance)
		self->moveinfo.distance = st.distance;
	else
		self->moveinfo.distance = WORLD_SIZE;	// was 8192

	self->solid = SOLID_NOT;
	if (self->movewith)
		self->movetype = MOVETYPE_PUSH;
	else
		self->movetype = MOVETYPE_NONE;
	self->use = use_target_attractor;

	if (st.noise)
		self->noise_index = gi.soundindex(st.noise);
	else
		self->noise_index = 0;

	if (!self->speed)
		self->speed = 100;

	if (!self->accel)
		self->accel = self->speed;
	else
	{
		self->accel *= 0.1;
		if (self->accel > self->speed)
			self->accel = self->speed;
	}

	if (self->spawnflags & ATTRACTOR_ON)
	{
		if (self->spawnflags & ATTRACTOR_SINGLE)
			self->think = target_attractor_think_single;
		else
			self->think = target_attractor_think;
		if (self->sounds)
			self->nextthink = level.time + 2*FRAMETIME;
		else
			self->think(self);
	}
}

/*QUAKED target_text (1 0 0) (-8 -8 -8) (8 8 8) FILE
General text display. You can enter text using the "message" key, or you can refer to a text file. 

"message" string or path/file.txt
*/
//Note: this is a temporary implementation
/*void target_text_use (edict_t *self, edict_t *activator, edict_t *other)
{
	if ((self->message) && !(activator->svflags & SVF_MONSTER))
	{
		gi.centerprintf (activator, "%s", self->message);
		gi.dprintf ("%s", self->message);
		gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}
}

void SP_target_text (edict_t *self)
{
	self->svflags |= SVF_NOCLIENT;
	self->use = target_text_use;
	gi.linkentity (self);
}*/

/*===================================================================
 TARGET_MONITOR
 Move the player's viewpoint to the target_monitor origin, 
 gives the target_monitor angles to the player, and freezes him
 for "wait" seconds (default=3). If wait < 0, target_monitor
 must be targeted a second time to free the player.
===================================================================*/
#define SF_MONITOR_CHASECAM       1
#define SF_MONITOR_EYEBALL        2		// Same as CHASECAM, but viewpoint is at the target
										// entity's viewheight, and target entity is made
										// invisible while in use
#define SF_MONITOR_CAMERAEFFECT   4		// Knightmare- camera effect
#define SF_MONITOR_LETTERBOX      8		// Knightmare- letterboxing

void target_monitor_off (edict_t *self)
{
	int		i;
	edict_t	*faker;
	edict_t	*player;

	player = self->child;
	if (!player) return;

	if (self->spawnflags & SF_MONITOR_EYEBALL)
	{
		if (self->target_ent)
			self->target_ent->svflags &= ~SVF_NOCLIENT;
	}
	faker = player->client->camplayer;
	VectorCopy (faker->s.origin, player->s.origin);
	gi.TagFree (faker->client); 
	G_FreeEdict (faker); 
	player->client->ps.pmove.origin[0] = player->s.origin[0]*8;
	player->client->ps.pmove.origin[1] = player->s.origin[1]*8;
	player->client->ps.pmove.origin[2] = player->s.origin[2]*8;
	for (i=0 ; i<3 ; i++)
		player->client->ps.pmove.delta_angles[i] = 
			ANGLE2SHORT(player->client->org_viewangles[i] - player->client->resp.cmd_angles[i]);
	VectorCopy (player->client->org_viewangles, player->client->resp.cmd_angles);
	VectorCopy (player->client->org_viewangles, player->s.angles);
	VectorCopy (player->client->org_viewangles, player->client->ps.viewangles);
	VectorCopy (player->client->org_viewangles, player->client->v_angle);
	
	player->client->ps.gunindex        = gi.modelindex(player->client->pers.weapon->view_model);
	player->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	player->client->ps.pmove.pm_type   = PM_NORMAL;
#ifdef KMQUAKE2_ENGINE_MOD
	player->client->ps.rdflags		  &= ~(RDF_CAMERAEFFECT|RDF_LETTERBOX); // Knightmare- letterboxing
#endif
	player->svflags                   &= ~SVF_NOCLIENT;
	player->clipmask                   = MASK_PLAYERSOLID;
	player->solid                      = SOLID_BBOX;
	player->viewheight                 = 22;
	player->client->camplayer          = NULL;
	player->target_ent                 = NULL;
	gi.unlinkentity(player);
	KillBox(player);
	gi.linkentity(player);

	if (self->noise_index)
		gi.sound (player, CHAN_VOICE, self->noise_index, 1, ATTN_NORM, 0);

	// if we were previously in third person view, restore it
	if (tpp->value)
		Cmd_Chasecam_Toggle (player);

	self->child = NULL;
	gi.linkentity(self);

	self->count--;
	if (self->count == 0)
	{
		self->use = NULL;
		self->think = G_FreeEdict;
		self->nextthink = level.time + 1;
	}
}

void target_monitor_move (edict_t *self)
{
	// "chase cam"
	trace_t	trace;
	vec3_t	forward, o, goal;

	if (!self->target_ent || !self->target_ent->inuse)
	{
		if (self->wait)
		{
			self->think = target_monitor_off;
			self->nextthink = self->monsterinfo.attack_finished;
		}
		return;
	}

	if ( (self->monsterinfo.attack_finished > 0) &&
		(level.time > self->monsterinfo.attack_finished))
	{
		target_monitor_off(self);
		return;
	}

	AngleVectors (self->target_ent->s.angles, forward, NULL, NULL);
	VectorMA (self->target_ent->s.origin, -self->moveinfo.distance, forward, o);

	o[2] += self->viewheight;

	VectorSubtract (o, self->s.origin, o);
	VectorMA (self->s.origin, 0.2, o, o);

	trace = gi.trace(self->target_ent->s.origin, NULL, NULL, o, self, MASK_SOLID);
	VectorCopy (trace.endpos, goal);
	VectorMA (goal, 2, forward, goal);

	// pad for floors and ceilings
	VectorCopy(goal, o);
	o[2] += 6;
	trace = gi.trace(goal, NULL, NULL, o, self, MASK_SOLID);
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] -= 6;
	}

	VectorCopy(goal, o);
	o[2] -= 6;
	trace = gi.trace(goal, NULL, NULL, o, self, MASK_SOLID);
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] += 6;
	}

	VectorCopy(goal, self->s.origin);
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity(self);
}

void use_target_monitor (edict_t *self, edict_t *other, edict_t *activator)
{
	int			i;
	edict_t		*faker;
	edict_t		*monster;
	gclient_t	*cl;

	if (!activator->client)
		return;

	if (self->child)
	{
		if (self->wait < 0)
			target_monitor_off (self);
		return;
	}

	if (self->target)
		self->target_ent = G_Find(NULL, FOFS(targetname), self->target);

	// if this is a CHASE_CAM target_monitor and the target no longer
	// exists, remove this target_monitor and exit
	if (self->spawnflags & SF_MONITOR_CHASECAM)
	{
		if (!self->target_ent || !self->target_ent->inuse)
		{
			G_FreeEdict(self);
			return;
		}
	}
	// save current viewangles
	VectorCopy (activator->client->v_angle, activator->client->org_viewangles);

	// create a fake player to stand in real player's position
	faker = activator->client->camplayer = G_Spawn();
	faker->s.frame = activator->s.frame; 
	VectorCopy (activator->s.origin, faker->s.origin); 
	VectorCopy (activator->velocity, faker->velocity); 
	VectorCopy (activator->s.angles, faker->s.angles); 
	faker->s = activator->s;
	faker->takedamage   = DAMAGE_NO;		// so monsters won't attack
	faker->flags       |= FL_NOTARGET;      // ... just to make sure
//	faker->movetype     = MOVETYPE_WALK;
	faker->movetype     = MOVETYPE_TOSS;
	faker->groundentity = activator->groundentity;
	faker->viewheight   = activator->viewheight;
	faker->inuse        = true;
	faker->classname    = "camplayer";
	faker->mass         = activator->mass;
	faker->solid        = SOLID_BBOX;
	faker->deadflag     = DEAD_NO;
	faker->clipmask     = MASK_PLAYERSOLID;
	faker->health       = 100000;			// invulnerable
	faker->light_level  = activator->light_level;
	faker->think        = faker_animate;
	faker->nextthink    = level.time + FRAMETIME;
	VectorCopy (activator->mins, faker->mins);
	VectorCopy (activator->maxs, faker->maxs);
    // create a client so you can pick up items/be shot/etc while in camera
	cl = (gclient_t *) gi.TagMalloc(sizeof(gclient_t), TAG_LEVEL); 
	faker->client = cl; 
	faker->target_ent = activator;
	gi.linkentity (faker); 

	if (self->target_ent && self->target_ent->inuse)
	{
		if (self->spawnflags & SF_MONITOR_EYEBALL)
			VectorCopy (self->target_ent->s.angles, activator->client->ps.viewangles);
		else
		{
			vec3_t	dir;
			VectorSubtract (self->target_ent->s.origin, self->s.origin, dir);
			vectoangles (dir, activator->client->ps.viewangles);
		}
	}
	else
		VectorCopy (self->s.angles, activator->client->ps.viewangles);

	VectorCopy (self->s.origin, activator->s.origin);
	activator->client->ps.pmove.origin[0] = self->s.origin[0]*8;
	activator->client->ps.pmove.origin[1] = self->s.origin[1]*8;
	activator->client->ps.pmove.origin[2] = self->s.origin[2]*8;
	activator->client->ps.pmove.pm_type = PM_FREEZE;
#ifdef KMQUAKE2_ENGINE_MOD
	// Knightmare- camera effect and letterboxing
	if (self->spawnflags & SF_MONITOR_CAMERAEFFECT)
		activator->client->ps.rdflags |= RDF_CAMERAEFFECT;
	if (self->spawnflags & SF_MONITOR_LETTERBOX)
		activator->client->ps.rdflags |= RDF_LETTERBOX;
#endif
	activator->svflags                 |= SVF_NOCLIENT;
	activator->solid                    = SOLID_NOT;
	activator->viewheight               = 0;
	if (activator->client->chasetoggle)
	{
		Cmd_Chasecam_Toggle (activator);
		activator->client->chasetoggle = 1;
	}
	else
		activator->client->chasetoggle = 0;
	activator->clipmask = 0;
	VectorClear (activator->velocity);
	activator->client->ps.gunindex = 0; 
	activator->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	gi.linkentity (activator);

	gi.unlinkentity(faker);
	KillBox(faker);
	gi.linkentity(faker);

	// check to see if player is the enemy of any monster.
	for (i=maxclients->value+1, monster=g_edicts+i; i<globals.num_edicts; i++, monster++)
	{
		if (!monster->inuse) continue;
		if (!(monster->svflags & SVF_MONSTER)) continue;
		if (monster->enemy == activator)
		{
			monster->enemy = NULL;
			monster->oldenemy = NULL;
			if (monster->goalentity == activator)
				monster->goalentity = NULL;
			if (monster->movetarget == activator)
				monster->movetarget = NULL;
			monster->monsterinfo.attack_finished = level.time + 1;
			FindTarget (monster);
		}
	}

	activator->target_ent = self;
	self->child = activator;

	if (self->noise_index)
		gi.sound (activator, CHAN_VOICE, self->noise_index, 1, ATTN_NORM, 0);

	if (self->spawnflags & SF_MONITOR_CHASECAM)
	{
		if (self->wait > 0)
			self->monsterinfo.attack_finished = level.time + self->wait;
		else
			self->monsterinfo.attack_finished = 0;

		if (self->spawnflags & SF_MONITOR_EYEBALL)
		{
			self->viewheight = self->target_ent->viewheight;
			self->target_ent->svflags |= SVF_NOCLIENT;
		}
		VectorCopy (self->target_ent->s.origin, self->s.origin);
		self->think = target_monitor_move;
		self->think(self);
	}
	else if (self->wait > 0)
	{
		self->think = target_monitor_off;
		self->nextthink = level.time + self->wait;
	}
}

void SP_target_monitor (edict_t *self)
{
	char	buffer[MAX_QPATH];

	self->class_id = ENTITY_TARGET_MONITOR;

	if (!self->wait)
		self->wait = 3;
	self->use = use_target_monitor;
	self->movetype = MOVETYPE_NOCLIP;
	if (st.noise)
	{
		if (!strstr (st.noise, ".wav"))
			Com_sprintf (buffer, sizeof(buffer), "%s.wav", st.noise);
		else
		//	strncpy (buffer, st.noise, sizeof(buffer));
			Com_strcpy (buffer, sizeof(buffer), st.noise);
		self->noise_index = gi.soundindex (buffer);
	}

	if (self->spawnflags & SF_MONITOR_EYEBALL)
		self->spawnflags |= SF_MONITOR_CHASECAM;

	if (self->spawnflags & SF_MONITOR_CHASECAM)
	{	// chase cam
		if (self->spawnflags & SF_MONITOR_EYEBALL)
		{
			self->moveinfo.distance = 0;
			self->viewheight = 0;
		}
		else
		{
			if (self->distance)
				st.distance = self->distance;
			if (st.distance)
				self->moveinfo.distance = st.distance;
			else
				self->moveinfo.distance = 128;

			if (self->height)
				st.height = self->height;
			if (st.height)
				self->viewheight = st.height;
			else
				self->viewheight = 16;
		}

		// MUST have target
		if (!self->target)
		{
			gi.dprintf("CHASECAM target_monitor with no target at %s\n",vtos(self->s.origin));
			self->spawnflags &= ~(SF_MONITOR_CHASECAM | SF_MONITOR_EYEBALL);
		}
		else if (self->movewith)
		{
			gi.dprintf("CHASECAM target_monitor cannot use 'movewith'\n");
			self->spawnflags &= ~(SF_MONITOR_CHASECAM | SF_MONITOR_EYEBALL);
		}
	}

	gi.linkentity(self);
}

/*====================================================================================
 TARGET_ANIMATION causes the target entity to use the animation frames
 "startframe" through "startframe" + "framenumbers" - 1.

 Spawnflags:
 ACTIVATOR = 1 - target_animation acts on it's activator rather than
                 it's target

 "message" - specifies allowable classname to animate. This prevents 
             animating entities with inapplicable frame numbers
=====================================================================================*/

mmove_t	g_custom_anims[MAX_CUSTOM_ANIMS];	// array of custom anins, saved to level file

void target_animate (edict_t *ent)
{
	if ( (ent->s.frame <  ent->monsterinfo.currentmove->firstframe) ||
		(ent->s.frame >= ent->monsterinfo.currentmove->lastframe )    )
	{
		if (ent->monsterinfo.currentmove->endfunc)
		{
			ent->think     = ent->monsterinfo.currentmove->endfunc;
			ent->nextthink = level.time + FRAMETIME;
		}
		else if (ent->svflags & SVF_MONSTER)
		{
			// Hopefully we don't get here, but if we DO then we definitely 
			// need for monsters/actors to turn their brains back on.
			ent->think     = monster_think;
			ent->nextthink = level.time + FRAMETIME;
		}
		else
		{
			ent->think     = NULL;
			ent->nextthink = 0;
		}
		ent->monsterinfo.currentmove = ent->monsterinfo.savemove;
		return;
	}
	ent->s.frame++;
	ent->nextthink = level.time + FRAMETIME;
	gi.linkentity(ent);
}

void target_animation_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*target = NULL;

	if (level.time < self->touch_debounce_time)
		return;
	if (self->spawnflags & 1)
	{
		if (activator && activator->client)
			return;
		if (self->message && Q_stricmp(self->message, activator->classname))
			return;
		if (!self->target)
			target = activator;
	}
	if (!target)
	{
		if (!self->target)
			return;
		target = G_Find(NULL, FOFS(targetname), self->target);
		if (!target)
			return;
	}
	// Don't allow target to be animated if ALREADY under influence of
	// another target_animation
	if (target->think == target_animate)
		return;
	self->monsterinfo.currentmove->firstframe = self->startframe;
	self->monsterinfo.currentmove->lastframe  = self->startframe + self->framenumbers - 1;
	self->monsterinfo.currentmove->frame      = NULL;
	self->monsterinfo.currentmove->endfunc    = target->think;
	target->s.frame = self->startframe;
	target->think   = target_animate;
	target->monsterinfo.savemove    = target->monsterinfo.currentmove;
	target->monsterinfo.currentmove = self->monsterinfo.currentmove;
	target->nextthink = level.time + FRAMETIME;
	gi.linkentity(target);

	self->count--;
	if (self->count == 0)
		G_FreeEdict(self);
	else
		self->touch_debounce_time = level.time + (self->framenumbers+1)*FRAMETIME;
}

void SP_target_animation (edict_t *self)
{
#if 0
	gi.dprintf("Target_animation is currently not implemented.\n");
	G_FreeEdict(self);
	return;
#else
	mmove_t	*move;

	self->class_id = ENTITY_TARGET_ANIMATION;

	if (!self->target && !(self->spawnflags & 1))
	{
		gi.dprintf("target_animation w/o a target at %s\n",vtos(self->s.origin));
		G_FreeEdict(self);
	}
	switch(self->sounds)
	{
	case 1:
		// actor jump
		self->startframe = 66;
		self->framenumbers = 6;
		break;
	case 2:
		// actor flip
		self->startframe = 72;
		self->framenumbers = 12;
		break;
	case 3:
		// actor salute
		self->startframe = 84;
		self->framenumbers = 11;
		break;
	case 4:
		// actor taunt
		self->startframe = 95;
		self->framenumbers = 17;
		break;
	case 5:
		// actor wave
		self->startframe = 112;
		self->framenumbers = 11;
		break;
	case 6:
		// actor point
		self->startframe = 123;
		self->framenumbers = 12;
		break;
	default:
		if (!self->framenumbers)
			self->framenumbers = 1;
	}
	self->use = target_animation_use;
//	move = gi.TagMalloc(sizeof(mmove_t), TAG_LEVEL);
	move = G_NewCustomAnim();
	if (!move) {
		gi.dprintf("target_animation: no more custom anims available!\n");
		G_FreeEdict(self);
		return;
	}

	self->monsterinfo.currentmove = move;
#endif
}

/*===================================================================================
 TARGET_FAILURE - Halts the game, fades the screen to black and displays
                  a message explaining to the player how he screwed up.
                  Optionally plays a sound.
====================================================================================*/
void target_failure_wipe (edict_t *self)
{
	edict_t	*player;

	player = &g_edicts[1];	// Gotta be, since this is SP only
	if (player->client->textdisplay) Text_Close(player);
}

void target_failure_player_die (edict_t *player)
{
	int		n;

	// player_die w/o... umm... dying

	if (player->client->chaseactive)
	{
		ChasecamRemove (player);
		player->client->chasetoggle = 1;
	}
	else
		player->client->chasetoggle = 0;
	player->client->pers.spawn_landmark = false; // paranoia check
	player->client->pers.spawn_levelchange = false;
	player->client->zooming = 0;
	player->client->zoomed = false;
	SetSensitivities(player,true);
	if (player->client->spycam)
		camera_off(player);
	VectorClear (player->avelocity);
	player->takedamage = DAMAGE_NO;
	player->movetype = MOVETYPE_NONE;
	player->s.modelindex2 = 0;	// remove linked weapon model
	player->s.sound = 0;
	player->client->weapon_sound = 0;
	player->svflags |= SVF_DEADMONSTER;
	player->client->respawn_time = level.time + 1.0;
	player->client->ps.gunindex = 0;
	// clear inventory
	for (n = 0; n < game.num_items; n++)
	{
		player->client->pers.inventory[n] = 0;
	}
	// remove powerups
	player->client->quad_framenum = 0;
	player->client->invincible_framenum = 0;
	player->client->breather_framenum = 0;
	player->client->enviro_framenum = 0;
	player->flags &= ~(FL_POWER_SHIELD|FL_POWER_SCREEN);
	player->client->flashlight_active = false;
	player->deadflag = DEAD_FROZEN;
	gi.linkentity (player);
}

void target_failure_think (edict_t *self)
{
	target_failure_player_die(self->target_ent);
	self->target_ent = NULL;
	self->think = target_failure_wipe;
	self->nextthink = level.time + 10;
}

void target_failure_fade_lights (edict_t *self)
{
	char lightvalue[2];
	char values[] = "abcdefghijklm";

	lightvalue[0] = values[self->flags];
	lightvalue[1] = 0;
	gi.configstring(CS_LIGHTS+0, lightvalue);
	if (self->flags)
	{
		self->flags--;
		self->nextthink = level.time + 0.2;
	}
	else
	{
		target_failure_player_die(self->target_ent);
		self->target_ent = NULL;
		self->think = target_failure_wipe;
		self->nextthink = level.time + 10;
	}
}

void Use_Target_Text(edict_t *self, edict_t *other, edict_t *activator);
void use_target_failure (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!activator->client)
		return;

	if (self->target_ent)
		return;

	if (self->message && strlen(self->message))
		Use_Target_Text (self,other,activator);

	if (self->noise_index)
		gi.sound (activator, CHAN_VOICE|CHAN_RELIABLE, self->noise_index, 1, ATTN_NORM, 0);

	self->target_ent = activator;
	if (Q_stricmp(vid_ref->string,"gl") && Q_stricmp(vid_ref->string,"kmgl"))
	{
		self->flags = 12;
		self->think = target_failure_fade_lights;
		self->nextthink = level.time + FRAMETIME;
	}
	else
	{
		activator->client->fadestart = level.framenum;
		activator->client->fadein   = level.framenum + 40;
		activator->client->fadehold = activator->client->fadein + 100000;
		activator->client->fadeout  = 0;
		activator->client->fadecolor[0] = 0;
		activator->client->fadecolor[1] = 0;
		activator->client->fadecolor[2] = 0;
		activator->client->fadealpha    = 1.0;
		self->think = target_failure_think;
		self->nextthink = level.time + 4;
	}
	activator->deadflag = DEAD_FROZEN;
	gi.linkentity(activator);
}

void SP_target_failure (edict_t *self)
{
	if (deathmatch->value || coop->value)
	{	// SP only
		G_FreeEdict (self);
		return;
	}

	self->class_id = ENTITY_TARGET_FAILURE;

	self->use = use_target_failure;
	if (st.noise)
		self->noise_index = gi.soundindex(st.noise);
}

// DWH
//
// Tremor stuff follows
//
//
// target_locator can be used to move entities to a random selection
// from a series of path_corners. Move takes place at level start ONLY.
//
void target_locator_init (edict_t *self)
{
	int num_points=0;
	int i, N, nummoves;
	qboolean looped;
	edict_t *tgt0, *tgtlast, *target, *next;
	edict_t *move;

	move = NULL;
	move = G_Find(move,FOFS(targetname),self->target);

	if (!move)
	{
		gi.dprintf("Target of target_locator (%s) not found.\n",
			self->target);
		G_FreeEdict(self);
		return;
	}
	target = G_Find(NULL,FOFS(targetname),self->pathtarget);
	if (!target)
	{
		gi.dprintf("Pathtarget of target_locator (%s) not found.\n",
			self->pathtarget);
		G_FreeEdict(self);
		return;
	}

	srand(time(NULL));
	tgt0 = target;
	next = NULL;
	target->spawnflags &= 0x7FFE;
	while (next != tgt0)
	{
		if (target->target)
		{
			next = G_Find(NULL,FOFS(targetname),target->target);
			if ((!next) || (next==tgt0)) tgtlast = target;
			if (!next)
			{
				gi.dprintf("Target %s of path_corner at %s not found.\n",
					target->target,vtos(target->s.origin));
				break;
			}
			target = next;
			target->spawnflags &= 0x7FFE;
			num_points++;
		}
		else
		{
			next = tgt0;
			tgtlast = target;
		}
	}
	if (!num_points) num_points=1;
	
	nummoves = 1;
	while (move)
	{
		if (nummoves > num_points) break;  // more targets than path_corners

		N = rand() % num_points;
		i = 0;
		next   = tgt0;
		looped = false;
		while (i<=N)
		{
			target = next;
			if (!(target->spawnflags & 1)) i++;
			if (target==tgtlast)
			{
				// We've looped thru all path_corners, but not
				// reached the target number yet. This can only
				// happen in the case of multiple targets. Use the
				// next available path_corner.
				looped = true;
			}
			if (looped && !(target->spawnflags & 1)) i = N+1;
			next = G_Find(NULL,FOFS(targetname),target->target);
		}
		target->spawnflags |= 1;

		// Assumptions here: SOLID_BSP entities are assumed to be brush models,
		// all others are point ents
		if (move->solid == SOLID_BSP)
		{
			vec3_t origin;
			VectorAdd(move->absmin,move->absmax,origin);
			VectorScale(origin,0.5,origin);
			VectorSubtract(target->s.origin,origin,move->s.origin);
		}
		else {
			VectorCopy(target->s.origin,move->s.origin);
			VectorCopy(target->s.angles,move->s.angles);
		}
		M_droptofloor(move);
		gi.linkentity(move);
		move = G_Find(move,FOFS(targetname),self->target);
		nummoves++;
	}
	// All done, go away
	G_FreeEdict(self);
}

void SP_target_locator (edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("target_locator w/o target at %s\n",vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}
	if (!self->pathtarget)
	{
		gi.dprintf("target_locator w/o pathtarget at %s\n",vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	self->class_id = ENTITY_TARGET_LOCATOR;

	self->think = target_locator_init;
	self->nextthink = level.time + 2*FRAMETIME;
	gi.linkentity(self);
}
