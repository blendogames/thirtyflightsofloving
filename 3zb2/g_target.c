#include "g_local.h"

/*QUAKED target_temp_entity (1 0 0) (-8 -8 -8) (8 8 8)
Fire an origin based temp entity event to the clients.
"style"		type byte
*/
void Use_Target_Tent (edict_t *ent, edict_t *other, edict_t *activator)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (ent->style);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
}

void SP_target_temp_entity (edict_t *ent)
{
	ent->use = Use_Target_Tent;
}


//==========================================================

//==========================================================

/*QUAKED target_speaker (1 0 0) (-8 -8 -8) (8 8 8) looped-on looped-off reliable
"noise"		wav file to play
"attenuation"
-1 = none, send to whole level
1 = normal fighting sounds
2 = idle sound level
3 = ambient sound level
"volume"	0.0 to 1.0

Normal sounds play each time the target is used.  The reliable flag can be set for crucial voiceovers.

Looped sounds are always atten 3 / vol 1, and the use function toggles it on/off.
Multiple identical looping sounds will just increase volume without any speed cost.
*/
void Use_Target_Speaker (edict_t *ent, edict_t *other, edict_t *activator)
{
	int		chan;

	if (ent->spawnflags & 3)
	{	// looping sound toggles
		if (ent->s.sound)
			ent->s.sound = 0;	// turn it off
		else
			ent->s.sound = ent->noise_index;	// start it
#ifdef LOOP_SOUND_ATTENUATION	// Knightmare added
			ent->s.attenuation = ent->attenuation;
#endif
	}
	else
	{	// normal sound
		if (ent->spawnflags & 4)
			chan = CHAN_VOICE|CHAN_RELIABLE;
		else
			chan = CHAN_VOICE;
		// use a positioned_sound, because this entity won't normally be
		// sent to any clients because it is invisible
		gi.positioned_sound (ent->s.origin, ent, chan, ent->noise_index, ent->volume, ent->attenuation, 0);
	}
}

void SP_target_speaker (edict_t *ent)
{
	char	buffer[MAX_QPATH];

	if(!st.noise)
	{
		gi.dprintf("target_speaker with no noise set at %s\n", vtos(ent->s.origin));
		return;
	}
	if (!strstr (st.noise, ".wav"))
		Com_sprintf (buffer, sizeof(buffer), "%s.wav", st.noise);
	else
		strncpy (buffer, st.noise, sizeof(buffer));
	ent->noise_index = gi.soundindex (buffer);

	if (!ent->volume)
		ent->volume = 1.0;

	if (!ent->attenuation)
		ent->attenuation = 1.0;
	else if (ent->attenuation == -1)	// use -1 so 0 defaults to 1
		ent->attenuation = 0;

	// check for prestarted looping sound
	if (ent->spawnflags & 1) {
		ent->s.sound = ent->noise_index;
#ifdef LOOP_SOUND_ATTENUATION	// Knightmare added
		ent->s.attenuation = ent->attenuation;
#endif
	}

	ent->use = Use_Target_Speaker;

	// must link the entity so we get areas and clusters so
	// the server can determine who to send updates to
	gi.linkentity (ent);
}


//==========================================================

void Use_Target_Help (edict_t *ent, edict_t *other, edict_t *activator)
{
	if (ent->spawnflags & 1)
		strncpy (game.helpmessage1, ent->message, sizeof(game.helpmessage2)-1);
	else
		strncpy (game.helpmessage2, ent->message, sizeof(game.helpmessage1)-1);

	game.helpchanged++;
}

/*QUAKED target_help (1 0 1) (-16 -16 -24) (16 16 24) help1
When fired, the "message" key becomes the current personal computer string, and the message light will be set on all clients status bars.
*/
void SP_target_help(edict_t *ent)
{
	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (ent);
		return;
	}

	if (!ent->message)
	{
		gi.dprintf ("%s with no message at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}
	ent->use = Use_Target_Help;
}

//==========================================================

/*QUAKED target_secret (1 0 1) (-8 -8 -8) (8 8 8)
Counts a secret found.
These are single use targets.
*/
void use_target_secret (edict_t *ent, edict_t *other, edict_t *activator)
{
	gi.sound (ent, CHAN_VOICE, ent->noise_index, 1, ATTN_NORM, 0);

	level.found_secrets++;

	G_UseTargets (ent, activator);
	G_FreeEdict (ent);
}

void SP_target_secret (edict_t *ent)
{
	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (ent);
		return;
	}

	ent->use = use_target_secret;
	if (!st.noise)
		st.noise = "misc/secret.wav";
	ent->noise_index = gi.soundindex (st.noise);
	ent->svflags = SVF_NOCLIENT;
	level.total_secrets++;
	// map bug hack
	if (!Q_stricmp(level.mapname, "mine3") && ent->s.origin[0] == 280 && ent->s.origin[1] == -2048 && ent->s.origin[2] == -624)
		ent->message = "You have found a secret area.";
}

//==========================================================

/*QUAKED target_goal (1 0 1) (-8 -8 -8) (8 8 8)
Counts a goal completed.
These are single use targets.
*/
void use_target_goal (edict_t *ent, edict_t *other, edict_t *activator)
{
	gi.sound (ent, CHAN_VOICE, ent->noise_index, 1, ATTN_NORM, 0);

	level.found_goals++;

	if (level.found_goals == level.total_goals)
		gi.configstring (CS_CDTRACK, "0");

	G_UseTargets (ent, activator);
	G_FreeEdict (ent);
}

void SP_target_goal (edict_t *ent)
{
	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (ent);
		return;
	}

	ent->use = use_target_goal;
	if (!st.noise)
		st.noise = "misc/secret.wav";
	ent->noise_index = gi.soundindex (st.noise);
	ent->svflags = SVF_NOCLIENT;
	level.total_goals++;
}

//==========================================================


/*QUAKED target_explosion (1 0 0) (-8 -8 -8) (8 8 8)
Spawns an explosion temporary entity when used.

"delay"		wait this long before going off
"dmg"		how much radius damage should be done, defaults to 0
*/
void target_explosion_explode (edict_t *self)
{
	float		save;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	T_RadiusDamage (self, self->activator, self->dmg, NULL, self->dmg+40, MOD_EXPLOSIVE);

	save = self->delay;
	self->delay = 0;
	G_UseTargets (self, self->activator);
	self->delay = save;
}

void use_target_explosion (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;

	if (!self->delay)
	{
		target_explosion_explode (self);
		return;
	}

	self->think = target_explosion_explode;
	self->nextthink = level.time + self->delay;
}

void SP_target_explosion (edict_t *ent)
{
	ent->use = use_target_explosion;
	ent->svflags = SVF_NOCLIENT;
}


//==========================================================

/*QUAKED target_changelevel (1 0 0) (-8 -8 -8) (8 8 8)
Changes level to "map" when fired
*/
void use_target_changelevel (edict_t *self, edict_t *other, edict_t *activator)
{
	if (level.intermissiontime)
		return;		// already activated

	if (!deathmatch->value && !coop->value)
	{
		if (g_edicts[1].health <= 0)
			return;
	}

	// if noexit, do a ton of damage to other
	if (deathmatch->value && !( (int)dmflags->value & DF_ALLOW_EXIT) && other != world)
	{
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 10 * other->max_health, 1000, 0, MOD_EXIT);
		return;
	}

	// if multiplayer, let everyone know who hit the exit
	if (deathmatch->value)
	{
		if (activator && activator->client)
			gi.bprintf (PRINT_HIGH, "%s exited the level.\n", activator->client->pers.netname);
	}

	// if going to a new unit, clear cross triggers
	if (strstr(self->map, "*"))	
		game.serverflags &= ~(SFL_CROSS_TRIGGER_MASK);

	BeginIntermission (self);
}

void SP_target_changelevel (edict_t *ent)
{
	if (!ent->map)
	{
		gi.dprintf("target_changelevel with no map at %s\n", vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	// ugly hack because *SOMEBODY* screwed up their map
   if((Q_stricmp(level.mapname, "fact1") == 0) && (Q_stricmp(ent->map, "fact3") == 0))
	   ent->map = "fact3$secret1";

	ent->use = use_target_changelevel;
	ent->svflags = SVF_NOCLIENT;
}


//==========================================================

/*QUAKED target_splash (1 0 0) (-8 -8 -8) (8 8 8)
Creates a particle splash effect when used.

Set "sounds" to one of the following:
  1) sparks
  2) blue water
  3) brown water
  4) slime
  5) lava
  6) blood

"count"	how many pixels in the splash
"dmg"	if set, does a radius damage at this location when it splashes
		useful for lava/sparks
*/

void use_target_splash (edict_t *self, edict_t *other, edict_t *activator)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPLASH);
	gi.WriteByte (self->count);
	gi.WritePosition (self->s.origin);
	gi.WriteDir (self->movedir);
	gi.WriteByte (self->sounds);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (self->dmg)
		T_RadiusDamage (self, activator, self->dmg, NULL, self->dmg+40, MOD_SPLASH);
}

void SP_target_splash (edict_t *self)
{
	self->use = use_target_splash;
	G_SetMovedir (self->s.angles, self->movedir);

	if (!self->count)
		self->count = 32;

	self->svflags = SVF_NOCLIENT;
}


//==========================================================

/*QUAKED target_spawner (1 0 0) (-8 -8 -8) (8 8 8)
Set target to the type of entity you want spawned.
Useful for spawning monsters and gibs in the factory levels.

For monsters:
	Set direction to the facing you want it to have.

For gibs:
	Set direction if you want it moving and
	speed how fast it should be moving otherwise it
	will just be dropped
*/
void ED_CallSpawn (edict_t *ent);

void use_target_spawner (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*ent;

	ent = G_Spawn();
	ent->classname = self->target;
	VectorCopy (self->s.origin, ent->s.origin);
	VectorCopy (self->s.angles, ent->s.angles);
	ED_CallSpawn (ent);
	gi.unlinkentity (ent);
	KillBox (ent);
	gi.linkentity (ent);
	if (self->speed)
		VectorCopy (self->movedir, ent->velocity);
}

void SP_target_spawner (edict_t *self)
{
	self->use = use_target_spawner;
	self->svflags = SVF_NOCLIENT;
	if (self->speed)
	{
		G_SetMovedir (self->s.angles, self->movedir);
		VectorScale (self->movedir, self->speed, self->movedir);
	}
}

//==========================================================

/*QUAKED target_blaster (1 0 0) (-8 -8 -8) (8 8 8) NOTRAIL NOEFFECTS
Fires a blaster bolt in the set direction when triggered.

dmg		default is 15
speed	default is 1000
*/

void use_target_blaster (edict_t *self, edict_t *other, edict_t *activator)
{
	int effect;

	if (self->spawnflags & 2)
		effect = 0;
	else if (self->spawnflags & 1)
		effect = EF_HYPERBLASTER;
	else
		effect = EF_BLASTER;

	fire_blaster (self, self->s.origin, self->movedir, self->dmg, self->speed, EF_BLASTER, MOD_TARGET_BLASTER);
	gi.sound (self, CHAN_VOICE, self->noise_index, 1, ATTN_NORM, 0);
}

void SP_target_blaster (edict_t *self)
{
	self->use = use_target_blaster;
	G_SetMovedir (self->s.angles, self->movedir);
	self->noise_index = gi.soundindex ("weapons/laser2.wav");

	if (!self->dmg)
		self->dmg = 15;
	if (!self->speed)
		self->speed = 1000;

	self->svflags = SVF_NOCLIENT;
}


//==========================================================

/*QUAKED target_crosslevel_trigger (.5 .5 .5) (-8 -8 -8) (8 8 8) trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8
Once this trigger is touched/used, any trigger_crosslevel_target with the same trigger number is automatically used when a level is started within the same unit.  It is OK to check multiple triggers.  Message, delay, target, and killtarget also work.
*/
void trigger_crosslevel_trigger_use (edict_t *self, edict_t *other, edict_t *activator)
{
	game.serverflags |= self->spawnflags;
	G_FreeEdict (self);
}

void SP_target_crosslevel_trigger (edict_t *self)
{
	self->svflags = SVF_NOCLIENT;
	self->use = trigger_crosslevel_trigger_use;
}

/*QUAKED target_crosslevel_target (.5 .5 .5) (-8 -8 -8) (8 8 8) trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8
Triggered by a trigger_crosslevel elsewhere within a unit.  If multiple triggers are checked, all must be true.  Delay, target and
killtarget also work.

"delay"		delay before using targets if the trigger has been activated (default 1)
*/
void target_crosslevel_target_think (edict_t *self)
{
	if (self->spawnflags == (game.serverflags & SFL_CROSS_TRIGGER_MASK & self->spawnflags))
	{
		G_UseTargets (self, self);
		G_FreeEdict (self);
	}
}

void SP_target_crosslevel_target (edict_t *self)
{
	if (! self->delay)
		self->delay = 1;
	self->svflags = SVF_NOCLIENT;

	self->think = target_crosslevel_target_think;
	self->nextthink = level.time + self->delay;
}

//==========================================================

/*QUAKED target_laser (0 .5 .8) (-8 -8 -8) (8 8 8) START_ON RED GREEN BLUE YELLOW ORANGE FAT
When triggered, fires a laser.  You can either set a target
or a direction.
*/

void target_laser_think (edict_t *self)
{
	edict_t	*ignore;
	vec3_t	start;
	vec3_t	end;
	trace_t	tr;
	vec3_t	point;
	vec3_t	last_movedir;
	int		count;

	if (self->spawnflags & 0x80000000)
		count = 8;
	else
		count = 4;

	if (self->enemy)
	{
		VectorCopy (self->movedir, last_movedir);
		VectorMA (self->enemy->absmin, 0.5, self->enemy->size, point);
		VectorSubtract (point, self->s.origin, self->movedir);
		VectorNormalize (self->movedir);
		if (!VectorCompare(self->movedir, last_movedir))
			self->spawnflags |= 0x80000000;
	}

	ignore = self;
	VectorCopy (self->s.origin, start);
	VectorMA (start, 2048, self->movedir, end);
	while(1)
	{
		tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		VectorCopy (tr.endpos,self->moveinfo.end_origin);

		if (!tr.ent)
			break;

		// hurt it if we can
		if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER))
			T_Damage (tr.ent, self, self->activator, self->movedir, tr.endpos, vec3_origin, self->dmg, 1, DAMAGE_ENERGY, MOD_TARGET_LASER);

		// if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		{
			if (self->spawnflags & 0x80000000)
			{
				self->spawnflags &= ~0x80000000;
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (count);
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

	self->nextthink = level.time + FRAMETIME;
}

void target_laser_on (edict_t *self)
{
	if (!self->activator)
		self->activator = self;
	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
	target_laser_think (self);
}

void target_laser_off (edict_t *self)
{
	self->spawnflags &= ~1;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
}

void target_laser_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;
	if (self->spawnflags & 1)
		target_laser_off (self);
	else
		target_laser_on (self);
}

void target_laser_start (edict_t *self)
{
	edict_t *ent;

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1;			// must be non-zero

	// set the beam diameter
	if (self->spawnflags & 64)
		self->s.frame = 16;
	else
		self->s.frame = 4;

	// set the color
	if (self->spawnflags & 2)
		self->s.skinnum = 0xf2f2f0f0;
	else if (self->spawnflags & 4)
		self->s.skinnum = 0xd0d1d2d3;
	else if (self->spawnflags & 8)
		self->s.skinnum = 0xf3f3f1f1;
	else if (self->spawnflags & 16)
		self->s.skinnum = 0xdcdddedf;
	else if (self->spawnflags & 32)
		self->s.skinnum = 0xe0e1e2e3;

	if (!self->enemy)
	{
		if (self->target)
		{
			ent = G_Find (NULL, FOFS(targetname), self->target);
			if (!ent)
				gi.dprintf ("%s at %s: %s is a bad target\n", self->classname, vtos(self->s.origin), self->target);
			self->enemy = ent;
		}
		else
		{
			G_SetMovedir (self->s.angles, self->movedir);
		}
	}
	self->use = target_laser_use;
	self->think = target_laser_think;

	if (!self->dmg)
		self->dmg = 1;

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	gi.linkentity (self);

	if (self->spawnflags & 1)
		target_laser_on (self);
	else
		target_laser_off (self);
}

void SP_target_laser (edict_t *self)
{
	// let everything else get spawned before we start firing
	self->think = target_laser_start;
	self->nextthink = level.time + 1;
}

//==========================================================
// RAFAEL 15-APR-98
/*QUAKED target_mal_laser (1 0 0) (-4 -4 -4) (4 4 4) START_ON RED GREEN BLUE YELLOW ORANGE FAT
Mal's laser
*/
void target_mal_laser_on (edict_t *self)
{
	if (!self->activator)
		self->activator = self;
	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
	// target_laser_think (self);
	self->nextthink = level.time + self->wait + self->delay;
}

void target_mal_laser_off (edict_t *self)
{
	self->spawnflags &= ~1;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
}

void target_mal_laser_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;
	if (self->spawnflags & 1)
		target_mal_laser_off (self);
	else
		target_mal_laser_on (self);
}

void mal_laser_think (edict_t *self)
{
	target_laser_think (self);
	self->nextthink = level.time + self->wait + 0.1;
	self->spawnflags |= 0x80000000;
}

void SP_target_mal_laser (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1;			// must be non-zero

	// set the beam diameter
	if (self->spawnflags & 64)
		self->s.frame = 16;
	else
		self->s.frame = 4;

	// set the color
	if (self->spawnflags & 2)
		self->s.skinnum = 0xf2f2f0f0;
	else if (self->spawnflags & 4)
		self->s.skinnum = 0xd0d1d2d3;
	else if (self->spawnflags & 8)
		self->s.skinnum = 0xf3f3f1f1;
	else if (self->spawnflags & 16)
		self->s.skinnum = 0xdcdddedf;
	else if (self->spawnflags & 32)
		self->s.skinnum = 0xe0e1e2e3;

	G_SetMovedir (self->s.angles, self->movedir);
	
	if (!self->delay)
		self->delay = 0.1;

	if (!self->wait)
		self->wait = 0.1;

	if (!self->dmg)
		self->dmg = 5;

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	
	self->nextthink = level.time + self->delay;
	self->think = mal_laser_think;

	self->use = target_mal_laser_use;

	gi.linkentity (self);

	if (self->spawnflags & 1)
		target_mal_laser_on (self);
	else
		target_mal_laser_off (self);
}
// END	15-APR-98
//==========================================================
/*QUAKED target_lightramp (0 .5 .8) (-8 -8 -8) (8 8 8) TOGGLE
speed		How many seconds the ramping will take
message		two letters; starting lightlevel and ending lightlevel
*/

void target_lightramp_think (edict_t *self)
{
	char	style[2];

	style[0] = 'a' + self->movedir[0] + (level.time - self->timestamp) / FRAMETIME * self->movedir[2];
	style[1] = 0;
	gi.configstring (CS_LIGHTS+self->enemy->style, style);

	if ((level.time - self->timestamp) < self->speed)
	{
		self->nextthink = level.time + FRAMETIME;
	}
	else if (self->spawnflags & 1)
	{
		char	temp;

		temp = self->movedir[0];
		self->movedir[0] = self->movedir[1];
		self->movedir[1] = temp;
		self->movedir[2] *= -1;
	}
}

void target_lightramp_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!self->enemy)
	{
		edict_t		*e;

		// check all the targets
		e = NULL;
		while (1)
		{
			e = G_Find (e, FOFS(targetname), self->target);
			if (!e)
				break;
			if (strcmp(e->classname, "light") != 0)
			{
				gi.dprintf("%s at %s ", self->classname, vtos(self->s.origin));
				gi.dprintf("target %s (%s at %s) is not a light\n", self->target, e->classname, vtos(e->s.origin));
			}
			else
			{
				self->enemy = e;
			}
		}

		if (!self->enemy)
		{
			gi.dprintf("%s target %s not found at %s\n", self->classname, self->target, vtos(self->s.origin));
			G_FreeEdict (self);
			return;
		}
	}

	self->timestamp = level.time;
	target_lightramp_think (self);
}

void SP_target_lightramp (edict_t *self)
{
	if (!self->message || strlen(self->message) != 2 || self->message[0] < 'a' || self->message[0] > 'z' || self->message[1] < 'a' || self->message[1] > 'z' || self->message[0] == self->message[1])
	{
		gi.dprintf("target_lightramp has bad ramp (%s) at %s\n", self->message, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if (!self->target)
	{
		gi.dprintf("%s with no target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	self->svflags |= SVF_NOCLIENT;
	self->use = target_lightramp_use;
	self->think = target_lightramp_think;

	self->movedir[0] = self->message[0] - 'a';
	self->movedir[1] = self->message[1] - 'a';
	self->movedir[2] = (self->movedir[1] - self->movedir[0]) / (self->speed / FRAMETIME);
}

//==========================================================

/*QUAKED target_earthquake (1 0 0) (-8 -8 -8) (8 8 8)
When triggered, this initiates a level-wide earthquake.
All players and monsters are affected.
"speed"		severity of the quake (default:200)
"count"		duration of the quake (default:5)
*/

void target_earthquake_think (edict_t *self)
{
	int		i;
	edict_t	*e;

	if (self->last_move_time < level.time)
	{
		gi.positioned_sound (self->s.origin, self, CHAN_AUTO, self->noise_index, 1.0, ATTN_NONE, 0);
		self->last_move_time = level.time + 0.5;
	}

	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		if (!e->groundentity)
			continue;

		e->groundentity = NULL;
		e->velocity[0] += crandom()* 150;
		e->velocity[1] += crandom()* 150;
		e->velocity[2] = self->speed * (100.0 / e->mass);
	}

	if (level.time < self->timestamp)
		self->nextthink = level.time + FRAMETIME;
}

void target_earthquake_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->timestamp = level.time + self->count;
	self->nextthink = level.time + FRAMETIME;
	self->activator = activator;
	self->last_move_time = 0;
}

void SP_target_earthquake (edict_t *self)
{
	if (!self->targetname)
		gi.dprintf("untargeted %s at %s\n", self->classname, vtos(self->s.origin));

	if (!self->count)
		self->count = 5;

	if (!self->speed)
		self->speed = 200;

	self->svflags |= SVF_NOCLIENT;
	self->think = target_earthquake_think;
	self->use = target_earthquake_use;

	self->noise_index = gi.soundindex ("world/quake.wav");
}

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

//	if(level.num_reflectors)
//		ReflectSteam (self->s.origin,self->movedir,self->count,self->sounds,(int)(self->speed),wait,nextid);
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

/*	if(level.num_reflectors)
	{
		if((self->style == TE_RAILTRAIL) || (self->style == TE_BUBBLETRAIL) ||
		   (self->style == TE_BFG_LASER) || (self->style == TE_DEBUGTRAIL)  ||
		   (self->style == TE_BUBBLETRAIL2))
		   ReflectTrail(self->style,self->s.origin,target->s.origin);
	}	*/
}
//===========================================================================
/* TE_LIGHTNING   33 Lightning bolt

  Similar but slightly different syntax to trail stuff */
void target_effect_lightning (edict_t *self, edict_t *activator)
{
	edict_t	*target;

	if (!self->target) return;
	target = G_Find(NULL,FOFS(targetname),self->target);
	if (!target) return;

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

//	if (level.num_reflectors)
//		ReflectSparks(self->style,self->s.origin,self->movedir);
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

//	if (level.num_reflectors)
//		ReflectExplosion (self->style, self->s.origin);
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
void target_effect_widowbeam (edict_t *self, edict_t *activator)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WIDOWBEAMOUT);
	gi.WriteShort (20001);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}
//===============================================================================

void target_effect_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & 1)
	{
		// currently looped on - turn it off
		self->spawnflags &= ~1;
		self->spawnflags |= 2;
		self->nextthink = 0;
		return;
	}
	if (self->spawnflags & 2)
	{
		// currently looped off - turn it on
		self->spawnflags &= ~2;
		self->spawnflags |= 1;
		self->nextthink = level.time + self->wait;
	}
/*	if (self->spawnflags & 4)
	{
		// "if_moving" set. If movewith target isn't moving,
		// don't play
		edict_t	*mover;
		if (!self->movewith) return;
		mover = G_Find(NULL,FOFS(targetname), self->movewith);
		if (!mover) return;
		if (!VectorLength(mover->velocity)) return;
	}*/
	self->play(self,activator);
}

void target_effect_think (edict_t *self)
{
	self->play(self,NULL);
	self->nextthink = level.time + self->wait;
}

//===============================================================================
void SP_target_effect (edict_t *self)
{
//	self->class_id = ENTITY_TARGET_EFFECT;

/*	if (self->movewith)
		self->movetype = MOVETYPE_PUSH;
	else*/
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
		if(!self->count)
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
		if (!self->target) {
			gi.dprintf("%s at %s with style=%d needs a target\n",self->classname,vtos(self->s.origin),self->style);
			G_FreeEdict(self);
		} else
			self->play = target_effect_trail;
		break;
	case TE_LIGHTNING:
		if (!self->target) {
			gi.dprintf("%s at %s with style=%d needs a target\n",self->classname,vtos(self->s.origin),self->style);
			G_FreeEdict(self);
		} else
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
		gi.dprintf ("%s at %s: bad style %d\n",self->classname,vtos(self->s.origin),self->style);
	}
	self->use = target_effect_use;
	self->think = target_effect_think;
	if (self->spawnflags & 1)
		self->nextthink = level.time + 1;
}
