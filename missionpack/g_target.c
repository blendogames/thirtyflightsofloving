#include "./g_local.h"

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
	ent->class_id = ENTITY_TARGET_TEMP_ENTITY;

	ent->use = Use_Target_Tent;
}


//==========================================================

//==========================================================

/*QUAKED target_speaker (1 0 0) (-8 -8 -8) (8 8 8) looped-on looped-off reliable
"noise"		wav file to play
"attenuation"
DWH
-2 = only played (full volume) for player who triggered the target_speaker
end DWH

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
		if (ent->s.sound) {
			ent->s.sound = 0;	// turn it off
		}
		else {
			ent->s.sound = ent->noise_index;	// start it
#ifdef LOOP_SOUND_ATTENUATION
			ent->s.attenuation = ent->attenuation;
#endif
		}
	}
	else
	{
		if (ent->attenuation == -2)
		{
			if (ent->spawnflags & 4)
				chan = CHAN_VOICE|CHAN_RELIABLE;
			else
				chan = CHAN_VOICE;
			gi.sound (activator, chan, ent->noise_index, 1, ATTN_NORM, 0);
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

		ent->count--;
		if (!ent->count) {
			ent->think = G_FreeEdict;
			ent->nextthink = level.time + 1;
		}
	}
}

void SP_target_speaker (edict_t *ent)
{
	size_t		msgSize;

	ent->class_id = ENTITY_TARGET_SPEAKER;

	if (!(ent->spawnflags & 8))
	{
		if (!st.noise)
		{
			gi.dprintf("target_speaker with no noise set at %s\n", vtos(ent->s.origin));
			G_FreeEdict(ent);
			return;
		}
		// DWH: Use "message" key to store noise for speakers that change levels
		//      via trigger_transition
		if (!strstr (st.noise, ".wav"))
		{
			msgSize = strlen(st.noise)+5;
			ent->message = gi.TagMalloc(msgSize, TAG_LEVEL);
			Com_sprintf(ent->message, msgSize, "%s.wav", st.noise);
		}
		else
		{
			msgSize = strlen(st.noise)+1;
			ent->message = gi.TagMalloc(msgSize, TAG_LEVEL);
			Com_strcpy (ent->message, msgSize, st.noise);
		}
	}

	ent->noise_index = gi.soundindex (ent->message);
	ent->spawnflags &= ~8;

	if (!ent->volume)
		ent->volume = 1.0;

	if (!ent->attenuation)
		ent->attenuation = (ent->spawnflags & 1) ? 3.0 : 1.0;
	else if (ent->attenuation == -1)	// use -1 so 0 defaults to 1
		ent->attenuation = 0;

	// check for prestarted looping sound
	if (ent->spawnflags & 1) {
		ent->s.sound = ent->noise_index;
#ifdef LOOP_SOUND_ATTENUATION
		ent->s.attenuation = ent->attenuation;
#endif
	}

	ent->use = Use_Target_Speaker;

/*	if (!strcmp(ent->classname, "moving_speaker"))
	{
		ent->think= moving_speaker_think;
		ent->nextthink = level.time + FRAMETIME;
	}*/
	// must link the entity so we get areas and clusters so
	// the server can determine who to send updates to
	gi.linkentity (ent);
}


//==========================================================

void Use_Target_Help (edict_t *ent, edict_t *other, edict_t *activator)
{
	if (ent->message)
	{
		if (ent->spawnflags & 1)
		//	strncpy (game.helpmessage1, ent->message, sizeof(game.helpmessage2)-1);
			Com_strcpy (game.helpmessage1, sizeof(game.helpmessage2), ent->message);
		else
		//	strncpy (game.helpmessage2, ent->message, sizeof(game.helpmessage1)-1);
			Com_strcpy (game.helpmessage2, sizeof(game.helpmessage1), ent->message);
	}

	game.helpchanged++;

	ent->count--;
	if (!ent->count) {
		ent->think = G_FreeEdict;
		ent->nextthink = level.time + 1;
	}
}

/*QUAKED target_help (1 0 1) (-16 -16 -24) (16 16 24) help1
When fired, the "message" key becomes the current personal computer string, and the message light will be set on all clients status bars.
*/
void SP_target_help (edict_t *ent)
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

	ent->class_id = ENTITY_TARGET_HELP;

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

	ent->class_id = ENTITY_TARGET_SECRET;

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

//Knightmare- keep playing music after goals are completed
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

	ent->class_id = ENTITY_TARGET_GOAL;

	ent->use = use_target_goal;
	if (!st.noise)
		st.noise = "misc/secret.wav";
	ent->noise_index = gi.soundindex (st.noise);
	ent->svflags = SVF_NOCLIENT;
	level.total_goals++;
}

//==========================================================


/*QUAKED target_explosion (1 0 0) (-8 -8 -8) (8 8 8) BIG
Spawns an explosion temporary entity when used.

BIG			Do you want a larger explosion model?
"delay"		wait this long before going dff
"dmg"		how much radius damage should be done, defaults to 0
*/
void target_explosion_explode (edict_t *self)
{
	float		save;
	int			eventNum;

	if (self->spawnflags & 1)	 // Knightmare- big explosion
		eventNum = TE_EXPLOSION1_BIG;
	else
		eventNum = TE_EXPLOSION1;

	gi.WriteByte (svc_temp_entity);
//	if (self->spawnflags & 1)	 //Knightmare- big explosion
//		gi.WriteByte (TE_EXPLOSION1_BIG);
//	else
//		gi.WriteByte (TE_EXPLOSION1);
	gi.WriteByte (eventNum);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	// Lazarus reflections
	if (level.num_reflectors)
	//	ReflectExplosion (TE_EXPLOSION1,self->s.origin);
		ReflectExplosion (eventNum, self->s.origin);

	T_RadiusDamage (self, self->activator, self->dmg, NULL, self->dmg+40, MOD_EXPLOSIVE);

	save = self->delay;
	self->delay = 0;
	G_UseTargets (self, self->activator);
	self->delay = save;

	self->count--;
	if (!self->count)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 1;
	}
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
	ent->class_id = ENTITY_TARGET_EXPLOSION;

	ent->use = use_target_explosion;
	ent->svflags = SVF_NOCLIENT;
}


//==========================================================

/*QUAKED target_changelevel (1 0 0) (-8 -8 -8) (8 8 8) Clr_Inven Landmark NoGun Easy Normal Hard Nightmare 
Changes level to "map" when fired

Clr_Inven: Causes the player to drop all items, keys, weapons, etc upon entering new level.
Landmark:  Allows you to have Half-Life-style transition zones from one map to the next.
No_gun:    Sets cl_gun 0 and crosshair 0 for the next map/demo only
Easy:      Sets skill 0 for next map
Normal:    Sets skill 1 for next map
Hard:      Sets skill 2 for next map
Nightmare: Sets skill 3 for next map

"map" The name of the next map. The value formula is entered as such: e1m1$123.
      "e1m1" is the name of the actual bsp file; the $ sign is a separator; "123" is the targetname of the info_player_start located in the next map.
*/


void use_target_changelevel (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*transition;
	extern	int	nostatus;

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

	FMOD_Stop();

	// if multiplayer, let everyone know who hit the exit
	if (deathmatch->value)
	{
		if (activator && activator->client)
			gi.bprintf (PRINT_HIGH, "%s exited the level.\n", activator->client->pers.netname);
	}

	if (activator->client)
	{
		if (!activator->vehicle)
			activator->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	}

	// if going to a new unit, clear cross triggers
	if (strstr(self->map, "*"))
	{
		// Knightmare- game.serverflags is ONLY used for cross-level trigger bits, so we can just zero it.
	//	game.serverflags &= ~(SFL_CROSS_TRIGGER_MASK);
		game.serverflags = 0;
		game.lock_code[0] = 0;
		game.lock_revealed = 0;
		game.lock_hud = 0;
		game.transition_ents = 0;
		if (activator->client)
		{
			activator->client->pers.spawn_landmark = false;
			activator->client->pers.spawn_levelchange = false;
		}
	}
	else
	{
		if (self->spawnflags & 2 && activator->client)
		{
			activator->client->pers.spawn_landmark = true;
			VectorSubtract(activator->s.origin,self->s.origin,
				activator->client->pers.spawn_offset);
			VectorCopy(activator->velocity,activator->client->pers.spawn_velocity);
			VectorCopy(activator->s.angles,activator->client->pers.spawn_angles);
			activator->client->pers.spawn_angles[ROLL] = 0;
			VectorCopy(activator->client->ps.viewangles,activator->client->pers.spawn_viewangles);
			activator->client->pers.spawn_pm_flags = activator->client->ps.pmove.pm_flags;
			if (self->s.angles[YAW])
			{
				vec3_t	angles;
				vec3_t	forward, right, v;

				angles[PITCH] = angles[ROLL] = 0.;
				angles[YAW] = self->s.angles[YAW];
				AngleVectors(angles,forward,right,NULL);
				VectorNegate(right,right);
				VectorCopy(activator->client->pers.spawn_offset,v);
				G_ProjectSource (vec3_origin,
					             v, forward, right,
								 activator->client->pers.spawn_offset);
				VectorCopy(activator->client->pers.spawn_velocity,v);
				G_ProjectSource (vec3_origin,
					             v, forward, right,
								 activator->client->pers.spawn_velocity);
				activator->client->pers.spawn_angles[YAW] += angles[YAW];
				activator->client->pers.spawn_viewangles[YAW] += angles[YAW];
			}
		}		
		else if (activator && activator->client) //Knightmare- paranoia
		{	
				activator->client->pers.spawn_landmark = false;
		}

		if ((self->spawnflags & 4) && activator->client && !deathmatch->value && !coop->value)
		{
			nostatus = 1;
#ifdef KMQUAKE2_ENGINE_MOD // engine has zoom mode and autosensitivity
			stuffcmd (activator, "cl_zoommode 1\n");
#else
			stuffcmd (activator, "cl_gun 0;crosshair 0\n");
#endif
			activator->client->pers.hand = 2;
		}
		if (activator && activator->client) //Knightmare- paranoia
		{
			activator->client->pers.spawn_gunframe    = activator->client->ps.gunframe;
			activator->client->pers.spawn_modelframe  = activator->s.frame;
			activator->client->pers.spawn_anim_end    = activator->client->anim_end;
		}
	}

	if (level.next_skill > 0)
	{
		gi.cvar_forceset("skill", va("%d",level.next_skill-1));
		level.next_skill = 0;	// reset
	}
	else if (self->spawnflags & 8)
		gi.cvar_forceset("skill", "0");
	else if (self->spawnflags & 16)
		gi.cvar_forceset("skill", "1");
	else if (self->spawnflags & 32)
		gi.cvar_forceset("skill", "2");
	else if (self->spawnflags & 64)
		gi.cvar_forceset("skill", "3");

	// Knightmare- some of id's stock Q2 maps have this spawnflag
	// set on their trigger_changelevels, so only allow this on custom maps
	if ( (self->spawnflags & 1) && (level.maptype == MAPTYPE_CUSTOM) && (int)allow_clear_inventory->value )
	{
		int n;
		if (activator && activator->client)
		{
			for (n = 0; n < MAX_ITEMS; n++)
			{
				// Keep blaster
				if (!(itemlist[n].flags & IT_WEAPON) || itemlist[n].weapmodel != WEAP_BLASTER )
					activator->client->pers.inventory[n] = 0;
			}
			//Knightmare- always have null weapon
			if (!deathmatch->value)
				activator->client->pers.inventory[ITEM_INDEX(FindItem("No Weapon"))] = 1;
			// Switch to blaster
			if ( activator->client->pers.inventory[ITEM_INDEX(FindItem("blaster"))] )
				activator->client->newweapon = FindItem ("blaster");
			else
				activator->client->newweapon = FindItem ("No Weapon");
			ChangeWeapon(activator);
			activator->client->pers.health = activator->health = activator->client->pers.max_health; //was 100
		}
	}
	game.transition_ents = 0;
	if (self->spawnflags & 2 && activator->client)
	{
		transition = G_Find(NULL,FOFS(classname), "trigger_transition");
		while (transition)
		{
			if (!Q_stricmp(transition->targetname,self->targetname))
			{
				game.transition_ents = trigger_transition_ents(self,transition);
				if (developer->value)
					gi.dprintf("Number of transition ents saved: %i\n", game.transition_ents);
				break;
			}
			transition = G_Find(transition,FOFS(classname), "trigger_transition");
		}
	}

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

	ent->class_id = ENTITY_TARGET_CHANGELEVEL;

	if ((deathmatch->value || coop->value) && (ent->spawnflags & 2))
	{
		gi.dprintf("target_changelevel at %s\nLANDMARK only valid in single-player\n",
			vtos(ent->s.origin));
		ent->spawnflags &= ~2;
	}

	// ugly hack because *SOMEBODY* screwed up their map
	if ((Q_stricmp(level.mapname, "fact1") == 0) && (Q_stricmp(ent->map, "fact3") == 0))
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
	self->class_id = ENTITY_TARGET_SPLASH;

	self->use = use_target_splash;
	G_SetMovedir (self->s.angles, self->movedir);

	if (!self->count)
		self->count = 32;

	self->svflags = SVF_NOCLIENT;
}


//==========================================================

/*QUAKED target_spawner (1 0 0) (-8 -8 -8) (8 8 8) 1 2 3 4 5 6 7 8
Set target to the type of entity you want spawned.
Set count to the number of times it can be used.
Useful for spawning monsters and gibs in the factory levels.
Spawnflags 1-128 will be duplicated for the entity spawned.

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
	ent->flags = self->flags;
	ent->spawnflags = self->spawnflags;
	VectorCopy (self->s.origin, ent->s.origin);
	VectorCopy (self->s.angles, ent->s.angles);
	ED_CallSpawn (ent);
	gi.unlinkentity (ent);
	KillBox (ent);
	gi.linkentity (ent);
	if (self->speed)
		VectorCopy (self->movedir, ent->velocity);

	ent->s.renderfx |= RF_IR_VISIBLE;		//PGM
	//Knightmare- auto-remove code
	self->count--;
	if (self->count == 0)
	{
		self->nextthink = level.time + 1;
		self->think = G_FreeEdict;
	}
}

void SP_target_spawner (edict_t *self)
{
	self->class_id = ENTITY_TARGET_SPEAKER;

	self->use = use_target_spawner;
	self->svflags = SVF_NOCLIENT;
	if (self->speed)
	{
		G_SetMovedir (self->s.angles, self->movedir);
		VectorScale (self->movedir, self->speed, self->movedir);
	}
}

//==========================================================

/*QUAKED target_blaster (1 0 0) (-8 -8 -8) (8 8 8) NO_TRAIL NO_EFFECTS START_ON IF_VISIBLE x x x SEEKPLAYER
Fires a blaster bolt in the set direction when triggered.

"count" number of times it can be used (wait>0 only)
"dmg" Default= 15
"speed"	Default= 1000
"wait" firing rate per second; default=0
"movewith" targetname of train or other ent to move with
"sounds"
   0 = Blaster
   1 = Railgun
   2 = Rocket
   3 = BFG
   4 = Homing rockets

*/

#define BLASTER_START_ON 4
#define BLASTER_IF_VISIBLE 8
#define BLASTER_SILENT		16
#define BLASTER_SEEK_PLAYER 128

void use_target_blaster (edict_t *self, edict_t *other, edict_t *activator)
{
	vec3_t	movedir, start, target;
	int effect;

	// Zaero add
	if (EMPNukeCheck(self, self->s.origin))
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("items/empnuke/emp_missfire.wav"), 1, ATTN_NORM, 0);
		return;
	}
	// end Zaero

	VectorCopy(self->s.origin,start);
	if (self->enemy)
	{
		if (self->enemy->health < 0)
		{
			self->enemy = NULL;
			return;
		}
		if (self->sounds == 6)
		{
			if (!AimGrenade (self, start, self->enemy->s.origin, self->speed, movedir, false))
				return;
		}
		else
		{
			VectorMA(self->enemy->absmin,0.5,self->enemy->size,target);
			VectorSubtract(target,start,movedir);
			VectorNormalize(movedir);
		}
	}
	else
	{	// Knightmare- set movedir here, allowing angles to be updated by movewith code
		G_SetMovedir2 (self->s.angles, self->movedir);
		VectorCopy(self->movedir,movedir);
	}

	if (self->spawnflags & 2)
		effect = 0;
	else if (self->spawnflags & 1)
		effect = EF_HYPERBLASTER;
	else
		effect = EF_BLASTER;

	// Lazarus: weapon choices
	if (self->sounds == 1)
	{
		fire_rail (self, start, movedir, self->dmg, 0, false, 0, 0, 0);
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (self-g_edicts);
		gi.WriteByte (MZ_RAILGUN);
		gi.multicast (start, MULTICAST_PVS);
	}
	else if (self->sounds == 2)
	{
		fire_rocket(self, start, movedir, self->dmg, self->speed, self->dmg, self->dmg, NULL);
		gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("weapons/rocklf1a.wav"), 1, ATTN_NORM, 0);
	}
	else if (self->sounds == 3)
	{
		fire_bfg(self, start, movedir, self->dmg, self->speed, self->dmg);
		gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("weapons/laser2.wav"), 1, ATTN_NORM, 0);
	}
	else if (self->sounds == 4)
	{
		fire_rocket(self, start, movedir, self->dmg, self->speed, self->dmg, self->dmg, self->enemy);
		gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("weapons/rocklf1a.wav"), 1, ATTN_NORM, 0);
	}
	else if (self->sounds == 5)
	{
		fire_bullet(self, start, movedir, self->dmg, 2, 0, 0, MOD_TARGET_BLASTER);
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_CHAINFIST_SMOKE);
		gi.WritePosition(start);
		gi.multicast(start, MULTICAST_PVS);
		gi.positioned_sound(start,self,CHAN_WEAPON,gi.soundindex(va("weapons/machgf%db.wav",rand() % 5 + 1)),1,ATTN_NORM,0);
	}
	else if (self->sounds == 6)
	{
		fire_grenade(self, start, movedir, self->dmg, self->speed, 2.5, self->dmg+40, false);
		gi.WriteByte (svc_muzzleflash2);
		gi.WriteShort (self - g_edicts);
		gi.WriteByte (MZ2_GUNNER_GRENADE_1);
		gi.multicast (start, MULTICAST_PVS);
	}
	else
	{
		fire_blaster (self, start, movedir, self->dmg, self->speed, effect, BLASTER_ORANGE, MOD_TARGET_BLASTER);
		if (self->noise_index)
			gi.sound (self, CHAN_VOICE, self->noise_index, 1, ATTN_NORM, 0);
	}

}

void target_blaster_think (edict_t *self)
{
	edict_t *ent;
	edict_t	*player;
	trace_t	tr;
	vec3_t	target;
	int		i;

	if (self->spawnflags & BLASTER_SEEK_PLAYER)
	{
		// this takes precedence over everything else

		// If we are currently targeting a non-player, reset and look for
		// a player
		if (self->enemy && !self->enemy->client)
			self->enemy = NULL;

		// Is currently targeted player alive and not using notarget?
		if (self->enemy)
		{
			if (self->enemy->flags & FL_NOTARGET)
				self->enemy = NULL;
			else if (!self->enemy->inuse || self->enemy->health < 0)
				self->enemy = NULL;
		}

		// We have a live not-notarget player as target. If IF_VISIBLE is 
		// set, see if we can see him
		if (self->enemy && (self->spawnflags & BLASTER_IF_VISIBLE) )
		{
			VectorMA(self->enemy->absmin,0.5,self->enemy->size,target);
			tr = gi.trace(self->s.origin,vec3_origin,vec3_origin,target,self,MASK_OPAQUE);
			if (tr.fraction != 1.0)
				self->enemy = NULL;
		}

		// If we STILL have an enemy, then he must be a good player target. Frag him
		if (self->enemy)
		{
			use_target_blaster(self,self,self);
			if (self->wait)
				self->nextthink = level.time + self->wait;
			return;
		}
	
		// Find a player - note that we search the entire entity list so we'll
		// also hit on func_monitor-viewing fake players
		for (i=1, player=g_edicts+1; i<globals.num_edicts && !self->enemy; i++, player++) {
			if (!player->inuse) continue;
			if (!player->client) continue;
			if (player->svflags & SVF_NOCLIENT) continue;
			if (player->health >= 0 && !(player->flags & FL_NOTARGET) )
			{
				if (self->spawnflags & BLASTER_IF_VISIBLE)
				{
					// player must be seen to shoot
					VectorMA(player->s.origin,0.5,player->size,target);
					tr = gi.trace(self->s.origin,vec3_origin,vec3_origin,target,self,MASK_OPAQUE);
					if (tr.fraction == 1.0)
						self->enemy = player;
				}
				else
				{
					// we don't care whether he can be seen
					self->enemy = player;
				}
			}
		}
		// If we have an enemy, shoot
		if (self->enemy)
		{
			use_target_blaster(self,self,self);
			if (self->wait)
				self->nextthink = level.time + self->wait;
			return;
		}
	}

	// If we get to this point, then either SEEK_PLAYER wasn't set or we couldn't find
	// a live, notarget player.

	if (self->target)
	{
		if (!(self->spawnflags & BLASTER_IF_VISIBLE))
		{
			// have a target, don't care whether it's visible; cannot be a gibbed monster
			self->enemy = NULL;
			ent = G_Find (NULL, FOFS(targetname), self->target);
			while (ent && !self->enemy)
			{
				// if target is not a monster, we're done
				if ( !(ent->svflags & SVF_MONSTER))
				{
					self->enemy = ent;
					break;
				}
				ent = G_Find(ent, FOFS(targetname), self->target);
			}
		}
		else
		{
			// has a target, but must be visible and not a monster
			self->enemy = NULL;
			ent = G_Find (NULL, FOFS(targetname), self->target);
			while (ent && !self->enemy)
			{
				// if the target isn't a monster, we don't care whether 
				// it can be seen or not.
				if ( !(ent->svflags & SVF_MONSTER) )
				{
					self->enemy = ent;
					break;
				}
				if ( ent->health > ent->gib_health)
				{
					// Not a gibbed monster
					VectorMA(ent->absmin,0.5,ent->size,target);
					tr = gi.trace(self->s.origin,vec3_origin,vec3_origin,target,self,MASK_OPAQUE);
					if (tr.fraction == 1.0)
					{
						self->enemy = ent;
						break;
					}
				}
				ent = G_Find(ent, FOFS(targetname), self->target);
			}
		}
	}
	if (self->enemy || !(self->spawnflags & BLASTER_IF_VISIBLE) )
	{
		use_target_blaster(self,self,self);
		if (self->wait)
			self->nextthink = level.time + self->wait;
	}
	else if (self->wait)
		self->nextthink = level.time + FRAMETIME;
}

void find_target_blaster_target(edict_t *self, edict_t *other, edict_t *activator)
{
	target_blaster_think(self);
}

void toggle_target_blaster (edict_t *self, edict_t *other, edict_t *activator)
{
	// used for target_blasters with a "wait" value
	self->activator = activator;
	if (self->spawnflags & 4)
	{
		self->count--;
		if (self->count == 0)
		{
			self->think = G_FreeEdict;
			self->nextthink = level.time + 1;
		}
		else
		{
			self->spawnflags &= ~4;
			self->nextthink = 0;
		}
	}
	else
	{
		self->spawnflags |= 4;
		self->think (self);
	}
}

void target_blaster_init (edict_t *self)
{
	if (self->target)
	{
		edict_t	*ent;
		ent = G_Find (NULL, FOFS(targetname), self->target);
		if (!ent)
			gi.dprintf("%s at %s: %s is a bad target\n", self->classname, vtos(self->s.origin), self->target);
		self->enemy = ent;
	}
}

void SP_target_blaster (edict_t *self)
{
//	self->use = use_target_blaster;
//	G_SetMovedir (self->s.angles, self->movedir);
	// Knightmare- added silent option
	if (!self->spawnflags & BLASTER_SILENT)
		self->noise_index = gi.soundindex ("weapons/laser2.wav");

	if (!self->dmg)
		self->dmg = 15;
	if (!self->speed)
		self->speed = 1000;

	// If SEEK_PLAYER is not set and there's no target, then
	// IF_VISIBLE is meaningless
	if (!(self->spawnflags & BLASTER_SEEK_PLAYER) && !self->target)
		self->spawnflags &= ~16;
	if (self->wait)
	{
		// toggled target_blaster
		self->use = toggle_target_blaster;
		self->enemy = NULL; // for now
		self->think = target_blaster_think;
		if (self->spawnflags & 4)
			self->nextthink = level.time + 1;
		else
			self->nextthink = 0;
	}
	else if (self->target || (self->spawnflags & BLASTER_SEEK_PLAYER))
	{
		self->use = find_target_blaster_target;
		if (self->target)
		{
			self->think = target_blaster_init;
			self->nextthink = level.time + 2*FRAMETIME;
		}
	}
	else
	{
		// normal targeted target_blaster
		self->use = use_target_blaster;
	}

	self->class_id = ENTITY_TARGET_BLASTER;

	gi.linkentity(self);
	self->svflags = SVF_NOCLIENT;
}


//==========================================================

/*QUAKED target_crosslevel_trigger (.5 .5 .5) (-8 -8 -8) (8 8 8) trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8
Once this trigger is touched/used, any trigger_crosslevel_target with the same trigger number is automatically used when a level is started within the same unit.  It is OK to check multiple triggers.  Message, delay, target, and killtarget also work.
*/
void trigger_crosslevel_trigger_use (edict_t *self, edict_t *other, edict_t *activator)
{
	// Knightmare- use moreflags to override spawnflags, allows use of 32 trigger bits
	if (self->moreflags != 0) {
		game.serverflags |= self->moreflags;
	}
	else {
		game.serverflags |= self->spawnflags;
	}
	G_FreeEdict (self);
}

void SP_target_crosslevel_trigger (edict_t *self)
{
	self->class_id = ENTITY_TARGET_CROSSLEVEL_TRIGGER;

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
/*	if (self->spawnflags == (game.serverflags & SFL_CROSS_TRIGGER_MASK & self->spawnflags))
	{
		G_UseTargets (self, self);
		G_FreeEdict (self);
	}*/
	// Knightmare- use moreflags to override spawnflags, allows use of 32 trigger bits
	qboolean triggered = false;

	if (self->moreflags != 0) {
		triggered = (self->moreflags == (game.serverflags & self->moreflags));
	}
	else {
		triggered = (self->spawnflags == (game.serverflags & SFL_CROSS_TRIGGER_MASK & self->spawnflags));
	}

	if (triggered) {
		G_UseTargets (self, self);
		G_FreeEdict (self);
	}
}

void SP_target_crosslevel_target (edict_t *self)
{
	self->class_id = ENTITY_TARGET_CROSSLEVEL_TARGET;

	if (! self->delay)
		self->delay = 1;
	self->svflags = SVF_NOCLIENT;

	self->think = target_crosslevel_target_think;
	self->nextthink = level.time + self->delay;
}

//==========================================================

/*QUAKED target_laser (0 .5 .8) (-8 -8 -8) (8 8 8) START_ON RED GREEN BLUE YELLOW ORANGE FAT WINDOWSTOP x x x x x SEEK_PLAYER
When triggered, fires a laser.  You can either set a target
or a direction.

WINDOWSTOP - stops at CONTENTS_WINDOW

"angle" aiming direction on the XY plane. Default=0. Ignored if target is used, and/or SEEK_PLAYER is set.
  
"angles" aiming direction in 3 dimensions, defined by pitch and yaw (roll is ignored). Default=0 0 0. Ignored if target is used, and/or SEEK_PLAYER is set.
  
"count" When non-zero, specifies the number of times the laser will be turned off before it is auto-killtargeted (see this page for details). Default=0.

"wait" If non-zero, specifies that the laser will pulse on and off automatically. The value specified here sets the amount of time (in seconds) between pulse cycles. Default=0. Also requires the use of delay. (Please see the notes below regarding timing issues when using pulse lasers).

"delay" amount of time, in seconds, that a laser pulse will be on. This value must be less than the value of wait. Default=0. Ignored if wait=0.
  
"dmg" the number of damage hit points the laser will do every 0.1 seconds. Default=100. If set to a negative value, the laser will give health, but will not cause pain effects. Ignored when style>0.
  
"mass" Sets the width of the laser. When 0, uses the default internal value of 4 for normal lasers, or 16 for fat lasers. When non-zero, FAT is ignored. Default=0.
 
"style" Sets the behavior of the laser. When non-zero, dmg is ignored. Choices are:
   0: Affects health, monsters avoid, creates sparks (normal, default laser).
   1: No damage; monsters avoid; creates sparks.
   2: No damage; monsters ignore; creates sparks.
   3: No damage; monsters ignore; no sparks.

"movewith" Targetname of parent entity the laser is to movewith.

"target" Targetname of the entity the laser will fire at. If SEEK_PLAYER is set, the player is a higher priority target, and therefore the entity specified by "target" will be ignored, assuming the laser has a clear shot at the player. In the event SEEK_PLAYER is set and the laser has no clear shot at him, it will fall back to firing at its "target".
  
"targetname" the specific target_laser.
*/

//======
// PGM
#define LASER_ON			1
//#define LASER_RED			0x0002
//#define LASER_GREEN		0x0004
//#define LASER_BLUE		0x0008
//#define LASER_YELLOW		0x0010
//#define LASER_ORANGE		0x0020
#define LASER_FAT			64
#define LASER_STOPWINDOW	128
#define	LASER_SEEK_PLAYER	8192
// PGM
//======

// DWH - player-seeking laser stuff
void target_laser_ps_think (edict_t *self)
{
	edict_t	*ignore;
	edict_t *player;
	trace_t	tr;
	vec3_t	start;
	vec3_t	end;
	vec3_t	point;
	vec3_t	last_movedir;
	vec3_t	target;
	int		count;
	int		i;

	if ( self->wait > 0) {
		if ( level.time >= self->starttime )
		{
			self->starttime = level.time + self->wait;
			self->endtime   = level.time + self->delay;
		}
		else if ( level.time >= self->endtime )
		{
			self->nextthink = level.time + FRAMETIME;
			return;
		}
	}

	if (self->spawnflags & 0x80000000)
		count = 8;    // spark count
	else
		count = 4;

	if (self->enemy) {
		if (self->enemy->flags & FL_NOTARGET || (self->enemy->health < self->enemy->gib_health) )
			self->enemy = NULL;
		else
		{
			// first make sure laser can see the center of the enemy
			VectorMA(self->enemy->absmin,0.5,self->enemy->size,target);
			tr = gi.trace(self->s.origin,vec3_origin,vec3_origin,target,self,MASK_OPAQUE);
			if (tr.fraction != 1.0)
				self->enemy = NULL;
		}
	}
	if (!self->enemy)
	{
		// find a player - as with target_blaster, search entire entity list so
		// we'll pick up fake players representing camera-viewers
		for (i=1, player=g_edicts+1; i<globals.num_edicts && !self->enemy; i++, player++)
		{
			if (!player->inuse) continue;
			if (!player->client) continue;
			if (player->svflags & SVF_NOCLIENT) continue;
			if ((player->health >= player->gib_health) && !(player->flags & FL_NOTARGET) )
			{
				VectorMA(player->absmin,0.5,player->size,target);
				tr = gi.trace(self->s.origin,vec3_origin,vec3_origin,target,self,MASK_OPAQUE);
				if (tr.fraction == 1.0) {
					self->enemy = player;
					self->spawnflags |= 0x80000001;
					count = 8;
				}
			}
		}
	}
	if (!self->enemy)
	{
		self->svflags |= SVF_NOCLIENT;
		self->nextthink = level.time + FRAMETIME;
		return;
	}

	self->svflags &= ~SVF_NOCLIENT;
	VectorCopy (self->movedir, last_movedir);
	VectorMA (self->enemy->absmin, 0.5, self->enemy->size, point);
	VectorSubtract (point, self->s.origin, self->movedir);
	VectorNormalize (self->movedir);
	if (!VectorCompare(self->movedir, last_movedir))
		self->spawnflags |= 0x80000000;

	ignore = self;
	VectorCopy (self->s.origin, start);
	VectorMA (start, 2048, self->movedir, end);
	while (1)
	{
		tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		if (!tr.ent)
			break;

		// hurt it if we can
		if (tr.ent->takedamage && !self->style)
		{
			if (!(tr.ent->flags & FL_IMMUNE_LASER) && (self->dmg > 0) )
				T_Damage (tr.ent, self, self->activator, self->movedir, tr.endpos, vec3_origin, self->dmg, 1, DAMAGE_ENERGY, MOD_TARGET_LASER);
			else if (self->dmg < 0)
			{
				tr.ent->health -= self->dmg;
				if (tr.ent->health > tr.ent->max_health)
					tr.ent->health = tr.ent->max_health;
			}
		}

		// if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		{
			if ((self->spawnflags & 0x80000000) && (self->style != 3))
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

void target_laser_ps_on (edict_t *self)
{
	if (!self->activator)
		self->activator = self;
	self->spawnflags |= 0x80000001;
//	self->svflags &= ~SVF_NOCLIENT;
	if (self->wait > 0)
	{
		self->starttime = level.time + self->wait;
		self->endtime = level.time + self->delay;
	}
	target_laser_ps_think (self);
}

void target_laser_ps_off (edict_t *self)
{
	self->spawnflags &= ~1;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
}

void target_laser_ps_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;
	if (self->spawnflags & 1)
	{
		target_laser_ps_off (self);
		self->count--;
		if (!self->count)
		{
			self->think = G_FreeEdict;
			self->nextthink = level.time + 1;
		}
	}
	else
		target_laser_ps_on (self);
}

void target_laser_think (edict_t *self)
{
	edict_t	*ignore;
	vec3_t	start;
	vec3_t	end;
	trace_t	tr; // trace;
	vec3_t	point;
	vec3_t	last_movedir;
	int		count;
	vec3_t	realmin;

	// DWH
	if ( self->wait > 0)
	{
		// pulsed laser
		if ( level.time >= self->starttime )
		{
			self->starttime = level.time + self->wait;
			self->endtime   = level.time + self->delay;
		}
		else if ( level.time >= self->endtime )
		{
			self->nextthink = level.time + FRAMETIME;
			return;
		}
	}
	// end DWH

	if (self->spawnflags & 0x80000000)
		count = 8;
	else
		count = 4;

	if (self->enemy)
	{
		//Knightmare- calc min coordinate, don't use absmin
		VectorAdd(self->enemy->s.origin, self->enemy->mins, realmin);

		VectorCopy (self->movedir, last_movedir);
	//	VectorMA (self->enemy->absmin, 0.5, self->enemy->size, point);
		VectorMA (realmin, 0.5, self->enemy->size, point);
		VectorSubtract (point, self->s.origin, self->movedir);
		VectorNormalize (self->movedir);
		if (!VectorCompare(self->movedir, last_movedir))
			self->spawnflags |= 0x80000000;
	}
	else // Knightmare- set movedir here so if our angles get updated...
		G_SetMovedir2 (self->s.angles, self->movedir);

	ignore = self;
	VectorCopy (self->s.origin, start);

	// Knightmare- if started in solid, move forward
	//while (gi.pointcontents(start) & CONTENTS_SOLID)
	//	VectorMA (start, 1, self->movedir, start);

	VectorMA (start, 2048, self->movedir, end);
	while (1)
	{
//======
// PGM
		if (self->spawnflags & LASER_STOPWINDOW)
			tr = gi.trace (start, NULL, NULL, end, ignore, MASK_SHOT);
		else
			tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
// PGM
//======
		if (!tr.ent)
			break;
 
		// hurt it if we can
		if (tr.ent->takedamage && !self->style)
		{
			if (!(tr.ent->flags & FL_IMMUNE_LASER) && (self->dmg > 0))
				T_Damage (tr.ent, self, self->activator, self->movedir, tr.endpos, vec3_origin, self->dmg, 1, DAMAGE_ENERGY, MOD_TARGET_LASER);
			else
			{
				tr.ent->health -= self->dmg;
				if (tr.ent->health > tr.ent->max_health)
					tr.ent->health = tr.ent->max_health;
			}
		}

		//PMM added SVF_DAMAGEABLE
		// if we hit something that's not a monster or player or is immune to lasers, we're done
		//if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client) && !(tr.ent->svflags & SVF_DAMAGEABLE))
		{
			if (self->spawnflags & 0x80000000 && (self->style != 3))
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
	if (self->wait > 0)
	{
		self->starttime = level.time + self->wait;
		self->endtime = level.time + self->delay;
	}
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
	{
		target_laser_off (self);
		self->count--;
		if (!self->count)
		{
			self->think = G_FreeEdict;
			self->nextthink = level.time + 1;
		}
	}
	else
		target_laser_on (self);
}

void target_laser_start (edict_t *self)
{
	edict_t *ent;
	vec3_t	forward;
	vec3_t	xhangar2laspoint1 = {572,-2848,164};
	vec3_t	xhangar2laspoint2 = {572,-2848,200};
	vec3_t	xhangar2laspoint3 = {572,-2848,236};
	vec3_t	xcompnd2laspoint1 = {768,2088,40};

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1;		// must be non-zero

	//Knightmare- horrendously ugly hack for the 3 lasers on xhangar2
	if (!Q_stricmp(level.mapname, "xhangar2")
		&& ( VectorCompare(self->s.origin, xhangar2laspoint1)
		||   VectorCompare(self->s.origin, xhangar2laspoint2)
		|| VectorCompare(self->s.origin, xhangar2laspoint3) ))
	{
		//gi.dprintf("Moving target_laser origin backward 4 units\n");
		VectorSet (forward, 0, 1, 0);
		VectorMA (self->s.origin, -4, forward, self->s.origin);
	}

	//Knightmare- another horrendously ugly hack for the laser on xcompnd2
	if (!Q_stricmp(level.mapname, "xcompnd2")
		&& VectorCompare(self->s.origin, xcompnd2laspoint1) )
	{
		//gi.dprintf("Moving target_laser origin backward 1 unit\n");
		VectorSet (forward, 1, 0, 0);
		VectorMA (self->s.origin, -1, forward, self->s.origin);
	}

	// set the beam diameter
	if (self->mass > 1)
		self->s.frame = self->mass;
	else if (self->spawnflags & 64)
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

	if (!self->dmg)
		self->dmg = 1;

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);

	// DWH

	// pulsed laser
	if (self->wait > 0)
	{
		if (self->delay >= self->wait)
		{
			gi.dprintf("target_laser at %s, delay must be < wait.\n",
				vtos(self->s.origin));
			self->wait = 0;
		} else if (self->delay == 0.)
		{
			gi.dprintf("target_laser at %s, wait > 0 but delay = 0\n",
				vtos(self->s.origin));
			self->wait = 0;
		}
	}

	if (self->spawnflags & 256)
	{
		// player-seeking laser
		self->enemy = NULL;
		self->use = target_laser_ps_use;
		self->think = target_laser_ps_think;
		gi.linkentity(self);
		if (self->spawnflags & 1)
			target_laser_ps_on(self);
		else
			target_laser_ps_off(self);
		return;
	}
	// end DWH


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
			G_SetMovedir2 (self->s.angles, self->movedir);
		}
	}

	self->use = target_laser_use;
	self->think = target_laser_think;

	gi.linkentity (self);

	if (self->spawnflags & 1)
		target_laser_on (self);
	else
		target_laser_off (self);
}

void SP_target_laser (edict_t *self)
{
	self->class_id = ENTITY_TARGET_LASER;

	// let everything else get spawned before we start firing
	self->think = target_laser_start;
	self->nextthink = level.time + 1;
}

// RAFAEL 15-APR-98
/*QUAKED target_mal_laser (0 .5 .8) (-8 -8 -8) (8 8 8) START_ON RED GREEN BLUE YELLOW ORANGE FAT WINDOWSTOP
Mal's laser
Same as the normal laser entity but it flashes on and off.

WINDOWSTOP - stops at CONTENTS_WINDOW

"angle" aiming direction on the XY plane. Default=0. Ignored if target is used, and/or SEEK_PLAYER is set.
  
"angles" aiming direction in 3 dimensions, defined by pitch and yaw (roll is ignored). Default=0 0 0. Ignored if target is used, and/or SEEK_PLAYER is set.
  
"count" When non-zero, specifies the number of times the laser will be turned off before it is auto-killtargeted (see this page for details). Default=0.

"wait" If non-zero, specifies that the laser will pulse on and off automatically. The value specified here sets the amount of time (in seconds) between pulse cycles. Default=0. Also requires the use of delay. (Please see the notes below regarding timing issues when using pulse lasers).

"delay" amount of time, in seconds, that a laser pulse will be on. This value must be less than the value of wait. Default=0. Ignored if wait=0.
  
"dmg" the number of damage hit points the laser will do every 0.1 seconds. Default=100. If set to a negative value, the laser will give health, but will not cause pain effects. Ignored when style>0.
  
"mass" Sets the width of the laser. When 0, uses the default internal value of 4 for normal lasers, or 16 for fat lasers. When non-zero, FAT is ignored. Default=0.

"style" Sets the behavior of the laser. When non-zero, dmg is ignored. Choices are:
   0: Affects health, monsters avoid, creates sparks (normal, default laser).
   1: No damage; monsters avoid; creates sparks.
   2: No damage; monsters ignore; creates sparks.
   3: No damage; monsters ignore; no sparks.

"movewith" Targetname of parent entity the laser is to movewith.

"target" Targetname of the entity the laser will fire at. If SEEK_PLAYER is set, the player is a higher priority target, and therefore the entity specified by "target" will be ignored, assuming the laser has a clear shot at the player. In the event SEEK_PLAYER is set and the laser has no clear shot at him, it will fall back to firing at its "target".
  
"targetname" the specific target_laser.
*/

void old_target_laser_think (edict_t *self)
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
	else // Knightmare- set movedir here so if our angles get updated...
		G_SetMovedir2 (self->s.angles, self->movedir);

	ignore = self;
	VectorCopy (self->s.origin, start);
	VectorMA (start, 2048, self->movedir, end);
	while (1)
	{
		tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

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

void target_mal_laser_on (edict_t *self)
{
	if (!self->activator)
		self->activator = self;
	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
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
	{
		target_mal_laser_off (self);
		self->count--;
		if (!self->count)
		{
			self->think = G_FreeEdict;
			self->nextthink = level.time + 1;
		}
	}
	else
		target_mal_laser_on (self);
}

void mal_laser_think (edict_t *self)
{
	old_target_laser_think (self);
	self->nextthink = level.time + self->wait + 0.1;
	self->spawnflags |= 0x80000000;
}

// Knightmare- added delayed start
void target_mal_laser_start (edict_t *self)
{

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1;			// must be non-zero

	// set the beam diameter
	if (self->mass > 1)
		self->s.frame = self->mass;
	else if (self->spawnflags & 64)
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

	G_SetMovedir2 (self->s.angles, self->movedir);
	
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

void SP_target_mal_laser (edict_t *self)
{
	self->class_id = ENTITY_TARGET_MAL_LASER;

	// let everything else get spawned before we start firing
	self->think = target_mal_laser_start;
	self->nextthink = level.time + 1;
}

// END	15-APR-98

//==========================================================

/*QUAKED target_lightramp (0 .5 .8) (-8 -8 -8) (8 8 8) TOGGLE
speed		How many seconds the ramping will take
message		two letters; starting lightlevel and ending lightlevel
*/

#define LIGHTRAMP_TOGGLE 1
#define LIGHTRAMP_CUSTOM 2
#define LIGHTRAMP_LOOP	 4
#define LIGHTRAMP_ACTIVE 128

void target_lightramp_think (edict_t *self)
{
	char	style[2];

	if (self->spawnflags & LIGHTRAMP_CUSTOM)
	{
		if (self->movedir[2] > 0)
			style[0] = self->message[(int)self->movedir[0]];
		else
			style[0] = self->message[(int)(self->movedir[1]-self->movedir[0])];
		self->movedir[0]++;
	}
	else
	{
		style[0] = 'a' + self->movedir[0] + (level.time - self->timestamp) / FRAMETIME * self->movedir[2];
	}
	style[1] = 0;
	gi.configstring (CS_LIGHTS+self->enemy->style, style);

	if (self->spawnflags & LIGHTRAMP_CUSTOM)
	{
		if ((self->movedir[0] <= self->movedir[1]) ||
		   ((self->spawnflags & LIGHTRAMP_LOOP) && (self->spawnflags & LIGHTRAMP_ACTIVE)) ) {
			self->nextthink = level.time + FRAMETIME;
			if (self->movedir[0] > self->movedir[1])
			{
				self->movedir[0] = 0;
				if (self->spawnflags & LIGHTRAMP_TOGGLE)
					self->movedir[2] *= -1;
			}
		}
		else
		{
			self->movedir[0] = 0;
			if (self->spawnflags & LIGHTRAMP_TOGGLE)
				self->movedir[2] *= -1;
			//Knightmare- auto-remove code
			self->count--;
			if (self->count == 0)
			{
				self->nextthink = level.time + 1;
				self->think = G_FreeEdict;
			}
		}
	}
	else
	{
		if ( (level.time - self->timestamp) < self->speed)
		{
			self->nextthink = level.time + FRAMETIME;
		}
		else if (self->spawnflags & LIGHTRAMP_TOGGLE)
		{
			char	temp;
			
			temp = self->movedir[0];
			self->movedir[0] = self->movedir[1];
			self->movedir[1] = temp;
			self->movedir[2] *= -1;
			if ( (self->spawnflags & LIGHTRAMP_LOOP) && (self->spawnflags & LIGHTRAMP_ACTIVE) )
			{
				self->timestamp = level.time;
				self->nextthink = level.time + FRAMETIME;
			}
		}
		else if ((self->spawnflags & LIGHTRAMP_LOOP) && (self->spawnflags & LIGHTRAMP_ACTIVE)) {
			// Not toggled, looping. Start sequence over
			self->timestamp = level.time;
			self->nextthink = level.time + FRAMETIME;
		}
		else
		{
			//Knightmare- auto-remove code
			self->count--;
			if (self->count == 0)
			{
				self->nextthink = level.time + 1;
				self->think = G_FreeEdict;
			}
		}
	}
}


void target_lightramp_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & LIGHTRAMP_LOOP)
	{
		if (self->spawnflags & LIGHTRAMP_ACTIVE)
		{
			self->spawnflags &= ~LIGHTRAMP_ACTIVE;	// already on, turn it off
			target_lightramp_think(self);
			return;
		}
		else
		{
			self->spawnflags |= LIGHTRAMP_ACTIVE;
		}
	}

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
	self->class_id = ENTITY_TARGET_LIGHTRAMP;

	// DWH: CUSTOM spawnflag allows custom light switching, speed is ignored
	if (self->spawnflags & LIGHTRAMP_CUSTOM)
	{
		if (!self->message || strlen(self->message) < 2)
		{
			gi.dprintf("custom target_lightramp has bad ramp (%s) at %s\n", self->message, vtos(self->s.origin));
			G_FreeEdict (self);
			return;
		}
	}
	else
	{
		if (!self->message || strlen(self->message) != 2 || self->message[0] < 'a' || self->message[0] > 'z' || self->message[1] < 'a' || self->message[1] > 'z' || self->message[0] == self->message[1])
		{
			gi.dprintf("target_lightramp has bad ramp (%s) at %s\n", self->message, vtos(self->s.origin));
			G_FreeEdict (self);
			return;
		}
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

	if (self->spawnflags & LIGHTRAMP_CUSTOM)
	{
		self->movedir[0] = 0;                        // index into message
		self->movedir[1] = strlen(self->message)-1;  // position of last character
		self->movedir[2] = 1;                        // direction = start->end
	}
	else
	{
		self->movedir[0] = self->message[0] - 'a';
		self->movedir[1] = self->message[1] - 'a';
		self->movedir[2] = (self->movedir[1] - self->movedir[0]) / (self->speed / FRAMETIME);
	}
}

//==========================================================

/*QUAKED target_earthquake (1 0 0) (-8 -8 -8) (8 8 8) SILENT
When triggered, this initiates a level-wide earthquake.
All players and monsters are affected.
"speed"		severity of the quake (default:200)
"accel"		horizontal shaking speed for players
"count"		duration of the quake (default:5)
*/

void target_earthquake_think (edict_t *self)
{
	int		i;
	edict_t	*e;

	if (!(self->spawnflags & 1))					// PGM
	{											// PGM
		if (self->last_move_time < level.time)
		{
			gi.positioned_sound (self->s.origin, self, CHAN_AUTO, self->noise_index, 1.0, ATTN_NONE, 0);
			self->last_move_time = level.time + 0.5;
		}
	}											// PGM
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		if (!e->groundentity)
			continue;
		// Lazarus: special case for tracktrain riders - 
		// earthquakes hurt 'em too bad, so don't shake 'em
		if ((e->groundentity->flags & FL_TRACKTRAIN) && (e->groundentity->moveinfo.state))
			continue;
		// Knightmare- if groundentity is a train or tracktrain, shake less
		if ( e->groundentity
			&& (!strcmp(e->groundentity->classname, "func_train") || !strcmp(e->groundentity->classname, "func_tracktrain")) )
		{
			if (!e->groundentity->velocity[2])
			{
			// Knightmare- added accel for horizontal shaking
			//	e->velocity[0] += crandom() * 37;
			//	e->velocity[1] += crandom() * 37;
				e->velocity[0] += crandom() * self->accel * 0.25f;
				e->velocity[1] += crandom() * self->accel * 0.25f;
			}
			//e->groundentity = NULL;
			e->velocity[2] = self->speed * (100.0 / e->mass);
		}
		else
		{
			e->groundentity = NULL;
		// Knightmare- added accel for horizontal shaking
		//	e->velocity[0] += crandom() * 150;
		//	e->velocity[1] += crandom() * 150;
			e->velocity[0] += crandom() * self->accel;
			e->velocity[1] += crandom() * self->accel;
			e->velocity[2] = self->speed * (100.0 / e->mass);
		}
	}
	if (level.time < self->timestamp)
		self->nextthink = level.time + FRAMETIME;
}

void target_earthquake_use (edict_t *self, edict_t *other, edict_t *activator)
{
	// PGM
//	if (g_showlogic && g_showlogic->value)
//		gi.dprintf("earthquake: %0.1f\n", self->speed);
	// PGM
	self->timestamp = level.time + self->count;
	self->nextthink = level.time + FRAMETIME;
	self->activator = activator;
	self->last_move_time = 0;
}

void SP_target_earthquake (edict_t *self)
{
	self->class_id = ENTITY_TARGET_EARTHQUAKE;

	if (!self->targetname)
		gi.dprintf("untargeted %s at %s\n", self->classname, vtos(self->s.origin));

	if (!self->count)
		self->count = 5;

	if (!self->speed)
		self->speed = 200;

	// Knightmare- added accel for horizontal shaking
	if (!self->accel)
		self->accel = 150;

	self->svflags |= SVF_NOCLIENT;
	self->think = target_earthquake_think;
	self->use = target_earthquake_use;

	if (!(self->spawnflags & 1))									// PGM
		self->noise_index = gi.soundindex ("world/quake.wav");
}
