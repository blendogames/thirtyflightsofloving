// g_camera.c

//CW: This camera code is taken from the Lazarus mod source (version 2.1),
//    which David Hyde released on 22-Sep-2001 for public use.
//    See 'http://planetquake.com/lazarus' for more information.

#include "g_local.h"

void camera_off(edict_t *ent)
{
	int		i;

//CW++
	edict_t	*t_ent;
//CW--

	if (!ent->client)
		return;

    if (!ent->client->spycam)
		return;

	if (ent->client->spycam->viewer == ent)
		ent->client->spycam->viewer = NULL;

	if (ent->client->spycam->svflags & SVF_MONSTER)
		ent->client->spycam->svflags &= ~SVF_NOCLIENT;

//CW++
	ent->client->frozen_framenum = ent->client->camplayer->client->frozen_framenum;

//	If the fake entity is on fire, transfer the flame back to the player.

	if (ent->client->camplayer->burning)
	{
		ent->burning = true;
		ent->flame = ent->client->camplayer->flame;
		ent->flame->enemy = ent;

		ent->client->camplayer->flame = NULL;
		ent->client->camplayer->burning = false;
	}

//	If the fake entity has been caught by a Trap, transfer the tractor beam to the player.

	if (ent->client->camplayer->tractored)
	{
		ent->tractored = true;
		ent->client->camplayer->tractored = false;

		for (i = 0; i < globals.num_edicts; ++i)
		{
			t_ent = &g_edicts[i];
			if ((t_ent->die == Trap_DieFromDamage) && (t_ent->enemy == ent->client->camplayer))
			{
				if (!t_ent->inuse)
					continue;

				t_ent->enemy = ent;
			}
		}
	}

	VectorCopy(ent->client->camplayer->s.origin, ent->s.old_origin);
//CW--

	VectorCopy(ent->client->camplayer->s.origin, ent->s.origin);
	gi.TagFree(ent->client->camplayer->client);
	G_FreeEdict(ent->client->camplayer); 

	// set angles
	ent->movetype = MOVETYPE_WALK;
	ent->client->ps.pmove.pm_type = PM_NORMAL;
	for (i = 0; i < 3; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->org_viewangles[i] - ent->client->resp.cmd_angles[i]);

	VectorCopy(ent->client->org_viewangles, ent->client->resp.cmd_angles);
	VectorCopy(ent->client->org_viewangles, ent->s.angles);
	VectorCopy(ent->client->org_viewangles, ent->client->ps.viewangles);
	VectorCopy(ent->client->org_viewangles, ent->client->v_angle);
	
	ent->client->ps.gunindex        = gi.modelindex(ent->client->pers.weapon->view_model);
	ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	ent->svflags                   &= ~SVF_NOCLIENT;
	ent->clipmask                   = MASK_PLAYERSOLID;
	ent->solid                      = SOLID_BBOX;
	ent->client->camplayer          = NULL;
	ent->client->spycam             = NULL;
	gi.linkentity(ent);
}

void faker_animate(edict_t *self)
{
	if ((self->s.frame < 0) || (self->s.frame > 39))
		self->s.frame = 0;
	else
	{
		self->s.frame++;
		if (self->s.frame > 39)
			self->s.frame = 0;
	}
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity(self);
}

void camera_on(edict_t *ent)
{
    gclient_t	*cl; 
	edict_t		*faker;
	edict_t		*camera;
	vec3_t		forward;
	vec3_t		left;
	vec3_t		up;

	if (!ent->client)
		return;

    if (!ent->client->spycam)
		return;

	// "viewer" can control camera aim (2nd player to come along and use camera cannot)
	camera = ent->client->spycam;
	
	if (!camera->viewer)
		camera->viewer = ent;

	// save current viewangles and restore them with camera_off
	VectorCopy(ent->client->v_angle, ent->client->org_viewangles);

    // copy over all important player data to fake player
	ent->client->camplayer = G_Spawn();
	faker = ent->client->camplayer;
	faker->s.frame = ent->s.frame; 
	VectorCopy(ent->s.origin, faker->s.origin); 
	VectorCopy(ent->velocity, faker->velocity); 
	VectorCopy(ent->s.angles, faker->s.angles); 
	faker->s = ent->s;
	faker->takedamage   = DAMAGE_AIM;
	faker->movetype     = MOVETYPE_WALK;
	faker->groundentity = ent->groundentity;
	faker->viewheight   = ent->viewheight;
	faker->inuse        = true;
	faker->classname    = "camplayer";
	faker->mass         = ent->mass;
	faker->solid        = SOLID_BBOX;
	faker->deadflag     = DEAD_NO;
	faker->clipmask     = MASK_PLAYERSOLID;
	faker->health       = ent->health;
	faker->light_level  = ent->light_level;
	faker->think        = faker_animate;
	faker->nextthink    = level.time + FRAMETIME;

//CW++
	faker->style = ENT_ID_FAKER;
//CW--

	VectorCopy(ent->mins, faker->mins);
	VectorCopy(ent->maxs, faker->maxs);

    // create a client so you can pick up items/be shot/etc while in camera
	cl = (gclient_t *)gi.TagMalloc(sizeof(gclient_t), TAG_LEVEL);									//CW
	memset(cl, 0, sizeof(gclient_t));
	ent->client->camplayer->client = cl; 
	ent->client->camplayer->target_ent = ent;
	gi.linkentity(faker); 

	AngleVectors(camera->s.angles,forward,left,up);

	VectorMA(camera->s.origin, camera->move_origin[0], forward, ent->s.origin);
	VectorMA(ent->s.origin,   -camera->move_origin[1], left,    ent->s.origin);
	VectorMA(ent->s.origin,    camera->move_origin[2], up,      ent->s.origin);

	ent->movetype = MOVETYPE_NOCLIP;
	ent->clipmask = 0;
	ent->solid    = SOLID_NOT;
	VectorClear(ent->velocity);
	ent->client->ps.gunindex = 0; 
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	ent->svflags |= SVF_NOCLIENT;

	if (ent->client->spycam->viewmessage)
		gi_centerprintf(ent, ent->client->spycam->viewmessage);

//CW++
	faker->delay = ent->delay;

//	If the player is on fire, transfer the flame to the fake entity.

	if (ent->burning)
	{
		faker->burning = true;
		faker->flame = ent->flame;
		faker->flame->enemy = faker;

		ent->flame = NULL;
		ent->burning = false;
	}

//	Mark the fake entity as being on the same team as the player.

	faker->client->resp.ctf_team = ent->client->resp.ctf_team;
//CW--
}

edict_t *G_FindNextCamera(edict_t *camera, edict_t *monitor)
{
	edict_t	*next;

	if (!monitor->target)
		return NULL;

//	If we already have a camera that's a monster, make it visible now.

	if (camera && (camera->svflags & SVF_MONSTER))
	{
		camera->svflags &= ~SVF_NOCLIENT;
		gi.linkentity(camera);
	}

//	First, determine if we're going to use the "count" to get the next camera,
//	or just scan through the list of entities. If count for the first camera
//	in the map is 0, then we'll just use the map order.

	next = G_Find(NULL, FOFS(targetname), monitor->target);
	if (!next)
		return NULL;

	if (!next->count)
	{	
		if (camera)
		{
			next = camera;
			next++;
		}
		else
			next = g_edicts;
		
		for ( ; next < &g_edicts[globals.num_edicts] ; next++)
		{
			if (next == camera)
				continue;
			if (!next->inuse)
				continue;
			if (next->deadflag == DEAD_DEAD)
				continue;
			if (!next->targetname)
				continue;
			// don't select "inactive" cameras
			if (!Q_stricmp(next->classname, "turret_breach") && (next->spawnflags & 16))
				continue;

			if (!Q_stricmp(next->targetname, monitor->target))
				goto found_one;
		}
		next = g_edicts;
		for ( ; next < camera ; next++)
		{
			if (next == camera)
				continue;
			if (!next->inuse)
				continue;
			if (next->deadflag == DEAD_DEAD)
				continue;
			if (!next->targetname)
				continue;
			// don't select "inactive" cameras
			if (!Q_stricmp(next->classname, "turret_breach") && (next->spawnflags & 16))
				continue;

			if (!Q_stricmp(next->targetname, monitor->target))
				goto found_one;
		}
	}
	else
	{
		int which;
		int start;

		if (camera)
		{
			which = camera->count + 1;
			if (which > monitor->count)
				which = 1;
		}
		else
			which = 1;

		start = which;
		next = g_edicts + 1;
		while (1)
		{
			if (next->targetname)
			{
				if (!Q_stricmp(next->targetname, monitor->target))
				{
					if (next->count == which)
					{
						if (!next->inuse || (next->deadflag == DEAD_DEAD) ||
							(!Q_stricmp(next->classname, "turret_breach") && (next->spawnflags & 16)))
						{
							next = g_edicts;
							which++;
							if (which > monitor->count)
								which = 1;

							if (which == start)
								return NULL;
						}
						else
							goto found_one;
					}
				}
			}
			if (next == &g_edicts[globals.num_edicts-1])
			{
				next = g_edicts;
				which++;
				if (which > monitor->count)
					which = 1;

				if (which == start)
					return NULL;
			}
			next++;
		}
	}
	return NULL;

found_one:
	if (!(monitor->spawnflags & 32) && (next->svflags & SVF_MONSTER))
		next->svflags |= SVF_NOCLIENT;

	return next;
}

edict_t *G_FindPrevCamera(edict_t *camera, edict_t *monitor)
{
	edict_t	*prev;
	edict_t	*newcamera = NULL;																		//CW

	if (!monitor->target)
		return NULL;
	
//	If we already have a camera that's a monster, make it visible now.

	if (camera && (camera->svflags & SVF_MONSTER))
	{
		camera->svflags &= ~SVF_NOCLIENT;
		gi.linkentity(camera);
	}

//	First, determine if we're going to use the "count" to get the next camera,
//	or just scan through the list of entities. If count for the first camera
//	in the map is 0, then we'll just use the map order.

	prev = G_Find(NULL, FOFS(targetname), monitor->target);
	if (!prev)
		return NULL;

	if (!prev->count)
	{
		newcamera = NULL;
		for (prev = g_edicts ; prev < &g_edicts[globals.num_edicts] ; prev++)
		{
			if (prev == camera)
			{
				if (newcamera)
					goto found_one;

				continue;
			}
			if (!prev->inuse)
				continue;
			if (prev->deadflag == DEAD_DEAD)
				continue;
			if (!prev->targetname)
				continue;
			// don't select "inactive" cameras
			if (!Q_stricmp(prev->classname, "turret_breach") && (prev->spawnflags & 16))
				continue;

			if (!Q_stricmp(prev->targetname, monitor->target))
				newcamera = prev;
		}
		goto found_one;
	}
	else
	{
		int which;
		int start;

		if (camera)
		{
			which = camera->count - 1;
			if (which <= 0)
				which = monitor->count;
		}
		else 
			which = monitor->count;

		start = which;
		prev = g_edicts + 1;
		while (1)
		{
			if (prev->targetname)
			{
				if (!Q_stricmp(prev->targetname, monitor->target))
				{
					if (prev->count == which)
					{
						if (!prev->inuse || (prev->deadflag == DEAD_DEAD) ||
							(!Q_stricmp(prev->classname, "turret_breach") && (prev->spawnflags & 16)))
						{
							prev = g_edicts;
							which--;
							if (which <= 0)
								which = monitor->count;

							if (which == start)
								return NULL;
						}
						else
						{
							newcamera = prev;
							goto found_one;
						}
					}
				}
			}
			if (prev == &g_edicts[globals.num_edicts-1])
			{
				prev = g_edicts;
				which--;
				if (which <= 0)
					which = monitor->count;

				if (which == start)
					return NULL;
			}
			prev++;
		}
	}

found_one:
	if (!(monitor->spawnflags & 32) && (newcamera->svflags & SVF_MONSTER))
		newcamera->svflags |= SVF_NOCLIENT;

	return newcamera;
}

void use_camera(edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*target;

	if (!activator->client)
		return;

	if (activator->client->spycam)		// already using camera
		return;

	target = G_FindNextCamera(NULL, self);
	if (!target)
		return;

    activator->client->spycam = target; 
	activator->client->monitor = self;
	camera_on(activator);
}

void func_monitor_init(edict_t *self)
{
	edict_t	*camera;

	self->count = 0;
	camera = NULL;
	while ((camera = G_Find(camera, FOFS(targetname), self->target)) != NULL)
		self->count++;

	if (!self->count)
		self->s.effects = 0; // don't animate a func_monitor that has no cameras
}

void SP_func_monitor(edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("func_monitor without a target at %s\n", vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	if (self->spawnflags & 8)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 16)
		self->s.effects |= EF_ANIM_ALLFAST;

	gi.setmodel(self, self->model);
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BSP;
	self->use = use_camera;
	self->think = func_monitor_init;
	self->nextthink = level.time + (2.0 * FRAMETIME);
	gi.linkentity(self);
}
