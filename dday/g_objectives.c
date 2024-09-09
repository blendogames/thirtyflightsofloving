/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_objectives.c,v $
 *   $Revision: 1.8 $
 *   $Date: 2002/06/04 19:49:46 $
 * 
 ***********************************

Copyright (C) 2002 Vipersoft

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
//#include "p_menus.h"

// g_objectives.c
// D-Day: Normandy Objective Entities

#if 0

	char *obj_name;
	float obj_area;
	float obj_time;
	int	obj_owner;	//entity that owns this item
	int	obj_gain;
	int	obj_loss;
	int obj_count;

#endif // 0

/*
========================
objective_area
========================
*/
void objective_area_think (edict_t *self) {

	edict_t *ent  = NULL;
	int count = 0;
	int i = 0;
	int newteam = 0;
	int delay;

	self->nextthink = level.time + FRAMETIME;

	if (self->delay) // if there's a counter running
	{
	}

	while ((ent = findradius(ent, self->s.origin, self->obj_area)) != NULL)
	{
		if (!ent->inuse)
			continue;
		if (!IsValidPlayer(ent))
			continue;

		newteam = ent->client->resp.team_on->index;

		if (newteam != self->obj_owner)
			count++;

		//gi.dprintf("Found %d players\n", count);		
	}

	if (count >= self->obj_count) {

		team_list[self->obj_owner]->score -= self->obj_loss;

		self->obj_owner = team_list[newteam]->index;
		team_list[self->obj_owner]->score += self->obj_gain;

		if (team_list[self->obj_owner]->time_to_win) // If there already is a counter somwhere else
		{
			if (team_list[self->obj_owner]->time_to_win > (self->obj_time + level.time) )
			// If the counter is longer, shorten it up to this one
				team_list[self->obj_owner]->time_to_win = (self->obj_time + level.time);
		} else // there is no counter
			team_list[self->obj_owner]->time_to_win = (self->obj_time + level.time);

		delay = (int)(team_list[self->obj_owner]->time_to_win - level.time);

		if ((delay/60) >= 1)
			safe_bprintf(PRINT_HIGH, "Team %s has %i minutes before they win the battle.\n", team_list[self->obj_owner]->teamname, (delay/60));
		else
			safe_bprintf(PRINT_HIGH, "Team %s has %i seconds before they win the battle.\n", team_list[self->obj_owner]->teamname, delay);

		gi.sound(self, CHAN_NO_PHS_ADD, gi.soundindex(va("%s/objectives/area_cap.wav", team_list[self->obj_owner]->teamid)), 1, 0, 0);

		if (dedicated->value)
			safe_cprintf(NULL, PRINT_HIGH, "Objective %s taken by team %s!\n",  self->obj_name,  team_list[self->obj_owner]->teamname);

		centerprintall("Objective %s taken\n by team %s!\n", 
			self->obj_name, 
			team_list[self->obj_owner]->teamname);
	}
}

void SP_objective_area(edict_t *self) {	

	if (!self->obj_name)
		 self->obj_name = "Objective";
	if (!self->obj_area)
		 self->obj_area = 100.0;
//	if (!self->obj_time)
		 self->obj_time = 120;
	if (!self->obj_count)
		 self->obj_count = 3;

	gi.dprintf("\n\nobjective_area spawned belonging to team %i (%s) as \"%s\"\n",
		self->obj_owner,
        team_list[self->obj_owner]->teamname,
        self->obj_name);

	gi.dprintf("distance: %f\n", 
		self->obj_area);

	gi.dprintf("award: %i, loss: %i\n", 
		self->obj_gain,
		self->obj_loss);
	   
	gi.dprintf("required persons: %i\n", self->obj_count);
	gi.dprintf("must hold for %i seconds.\n\n",	(int)self->obj_time);
	
	gi.dprintf(" mins: %s\n maxs: %s\n\n",
		vtos(self->mins),
		vtos(self->maxs) );

	self->think=objective_area_think;	
	self->nextthink = level.time + FRAMETIME;

	self->movetype = MOVETYPE_NONE;
//	self->svflags |= SVF_NOCLIENT;
	gi.setmodel (self, self->model);
	self->solid = SOLID_NOT;
	gi.linkentity (self);

	gi.dprintf(" mins: %s\n maxs: %s\n\n",
		vtos(self->mins),
		vtos(self->maxs) );

}


/*
========================
objective_touch
========================
*/
void objective_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {

	int otherteam;
	//edict_t *entC = NULL;

	if (!IsValidPlayer(other))  
		return;

	//with this type, will only be recapped by obj_perm_owner when they respawn
	if (self->style%3 == 2 && self->obj_perm_owner && self->obj_perm_owner%2 == other->client->resp.team_on->index &&
		other->client->respawn_time < level.time -.5)
		return;

	//gi.dprintf("touch %i:%i (%i)\n", level.framenum, self->obj_count, (level.framenum - self->obj_count));

	if (other->client->resp.team_on->index != self->obj_owner) 
	{
		if (self->style%5 == 3)//hill fix
		{
			if ((level.framenum - self->obj_count) <= 100) //team recaps after 10 seconds even if other team still in area
				return;
		}
		else if ((level.framenum - self->obj_count) <= self->delay)//15) // its been at least a frame since own team touched it
			return;

		if (self->obj_perm_owner)
		{
//			if (team_list[self->obj_owner])
//				team_list[self->obj_owner]->score -= self->dmg;

			self->obj_owner = other->client->resp.team_on->index;
			if (self->obj_perm_owner%2 != other->client->resp.team_on->index)
				if (team_list[self->obj_owner])
				{
					team_list[self->obj_owner]->score += self->health;
				}
		}
		else
		{
			if (team_list[self->obj_owner])
				team_list[self->obj_owner]->score -= self->dmg;

			self->obj_owner = other->client->resp.team_on->index;
			team_list[self->obj_owner]->score += self->health;

		}
		
		otherteam = (self->obj_owner);
		if (!team_list[otherteam]->need_points ||
			(!team_list[otherteam]->kills_and_points && team_list[otherteam]->score < team_list[otherteam]->need_points) ||
			(team_list[otherteam]->kills_and_points && 
				team_list[otherteam]->kills < team_list[otherteam]->need_kills))
			gi.sound(self, CHAN_NO_PHS_ADD, gi.soundindex(va("%s/objectives/touch_cap.wav", team_list[self->obj_owner]->teamid)), 1, 0, 0);

		if (dedicated->value)
			safe_cprintf(NULL, PRINT_HIGH, "%s taken by %s [%s]\n", 
				self->message, 
				other->client->pers.netname,
				team_list[self->obj_owner]->teamname);

		centerprintall("%s taken by:\n\n%s\n%s",
				self->message, 
				other->client->pers.netname,
				team_list[self->obj_owner]->teamname);
		
		self->obj_count = level.framenum; // reset the touch count

		G_UseTargets (self, other); //faf

		if (self->delay == -1)
			self->touch = NULL;
		
	} 
	else  // own team touched it
	{
		//gi.dprintf("%s deadflag: %i\n", other->client->pers.netname, other->deadflag);

		if (self->style%5==3)return; //HILL FIX

		if (other->deadflag == DEAD_NO)
			self->obj_count = level.framenum; // update the last time team touched it
	}

}




/*
========================
objective_area
========================
*/

/// exactly like the one above, except 1 person needs to touch the thing.
void SP_objective_touch(edict_t *self) 
{	
	vec3_t min;
	vec3_t max;

	int i;

	self->classname = "objective_touch";
	self->classnameb = OBJECTIVE_TOUCH;

	self->touch=objective_touch;
	//self->index=st.obj_owner;
	self->movetype = MOVETYPE_NONE;	
	self->solid = SOLID_TRIGGER;

	if (!self->delay)
		self->delay = 15;

	if (self->model)
	{
		gi.setmodel (self, self->model);

		if (VectorCompare (self->obj_origin, vec3_origin))
		{
			//gi.dprintf("xxx%s\n", self->classname);
			VectorSet (self->obj_origin, (self->absmax[0] + self->absmin[0])/2,
			(self->absmax[1] + self->absmin[1])/2,
			(self->absmax[2] + self->absmin[2])/2);
		}

	}
	else if (self->move_origin && self->move_angles)
	{ 
		VectorCopy (self->move_origin, min);
		VectorCopy (self->move_angles, max);

		//make sure mins are really less than maxs
		for (i=0; i< 3; i++)
		{
			if (min[i] > max [i])
			{
				self->move_origin[i] = max[i];
				self->move_angles[i] = min[i];
			}
		}

		VectorSet (self->s.origin, (self->move_angles[0] + self->move_origin[0])/2,
			(self->move_angles[1] + self->move_origin[1])/2,
			(self->move_angles[2] + self->move_origin[2])/2);
		VectorSet (self->mins, self->move_origin[0] - self->s.origin[0],
			self->move_origin[1] - self->s.origin[1],
			self->move_origin[2] - self->s.origin[2]);
		VectorSet (self->maxs, self->move_angles[0]- self->s.origin[0],
			self->move_angles[1]- self->s.origin[1],
			self->move_angles[2]- self->s.origin[2]);

		VectorCopy (self->s.origin, self->obj_origin);

	}



	//so bots, to find nearest campspot.  either mapper sets it or it goes to the center


	gi.linkentity (self);
	
}

void timed_objective_touch_think (edict_t *self) 
{
	self->nextthink = level.time + FRAMETIME;

	if (!self->wait)
		return;

	if (self->wait)
	{
//		safe_bprintf (PRINT_HIGH, "%i \n", (int)(self->wait + self->obj_time - level.time)); 
		level.obj_time =(int)(self->wait + self->obj_time - level.time + 1);
		level.obj_team = self->obj_owner;
	}

	if (level.intermissiontime)	{
		G_FreeEdict(self);
		return;
	}


	if (level.obj_time <= 0)
	{
		if (dedicated->value)
			safe_cprintf(NULL, PRINT_HIGH, "%s has been Successfully Held by the %s!\n", 
				self->message, 
				team_list[self->obj_owner]->teamname);

		safe_bprintf (PRINT_HIGH, "%s Has been Successfully Held by the %s!\n", 
				self->message, 
				team_list[self->obj_owner]->teamname);

		team_list[self->obj_owner]->score += self->health;
		
		level.obj_time = 0;
		
//		self->obj_count = level.framenum; // reset the touch count
		G_FreeEdict(self);
	}


}


void timed_objective_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) 
{
	int otherteam;
	

	//edict_t *entC = NULL;

	if (!IsValidPlayer(other))  
		return;

//	gi.dprintf("touch %i:%i (%i)\n", level.framenum, self->obj_count, (level.framenum - self->obj_count));



	if (self->obj_owner ==-1 ||
		other->client->resp.team_on->index != self->obj_owner) 
	{
		if ((level.framenum - self->obj_count) <= 15) // its been at least a frame since own team touched it
			return;

		self->wait =level.time;

//		if (self->obj_owner < MAX_TEAMS) // undefined teams
//			team_list[self->obj_owner]->score -= self->dmg;
		
		self->obj_owner = other->client->resp.team_on->index;
//		team_list[self->obj_owner]->score += self->health;

		otherteam = (self->obj_owner+1)%2;
		if (!team_list[otherteam]->kills_and_points && team_list[otherteam]->score < team_list[otherteam]->need_points ||
			(team_list[otherteam]->kills_and_points && 
				team_list[otherteam]->kills < team_list[otherteam]->need_kills))
		gi.sound(self, CHAN_NO_PHS_ADD, gi.soundindex(va("%s/objectives/touch_cap.wav", team_list[self->obj_owner]->teamid)), 1, 0, 0);
		

		if (dedicated->value)
			safe_cprintf(NULL, PRINT_HIGH, "%s taken by %s [%s]\n", 
				self->message, 
				other->client->pers.netname,
				team_list[self->obj_owner]->teamname);

		centerprintall("%s taken by:\n\n%s\n%s",
				self->message, 
				other->client->pers.netname,
				team_list[self->obj_owner]->teamname);
		
		self->obj_count = level.framenum; // reset the touch count
		
		/*
		if (self->obj_owner == 1 && self->style == 2)//hack so axis don't trigger flag on first cap
			self->style = 0;
		else		{
			self->style = 0;
			G_UseTargets (self, other); //faf
		} */

		if (self->obj_owner == 1 && self->style == 2)
		{
			edict_t *t;

			self->style = 0;
			//trigger spawn_toggle only
			t = NULL;
			while ((t = G_Find (t, FOFS(targetname), self->target)))
			{
				if (t->use)
				{
					if (!strcmp(t->classname,"spawn_toggle"))
						t->use (t,self, other);
				}
			}

		}
		else		
		{
			self->style = 0;
			G_UseTargets (self, other); //faf
		}



	} 
	else  // own team touched it
	{
		//gi.dprintf("%s deadflag: %i\n", other->client->pers.netname, other->deadflag);

		if (other->deadflag == DEAD_NO)
			self->obj_count = level.framenum; // update the last time team touched it
	}

}

/// exactly like the one above, except 1 person needs to touch the thing.
void SP_timed_objective_touch(edict_t *self) 
{	
	vec3_t min;
	vec3_t max;

	int i;

	self->classname = "objective_touch";
	self->classnameb = OBJECTIVE_TOUCH;

	self->obj_owner =-1;
	self->touch=timed_objective_touch;
	self->think=timed_objective_touch_think;
	self->nextthink = level.time + FRAMETIME;
//self->index=st.obj_owner;
	self->movetype = MOVETYPE_NONE;	
	self->solid = SOLID_TRIGGER;


	if (self->model)
		gi.setmodel (self, self->model);
	else if (self->move_origin && self->move_angles)
	{ 
		VectorCopy (self->move_origin, min);
		VectorCopy (self->move_angles, max);

		//make sure mins are really less than maxs
		for (i=0; i< 3; i++)
		{
			if (min[i] > max [i])
			{
				self->move_origin[i] = max[i];
				self->move_angles[i] = min[i];
			}
		}

		VectorSet (self->s.origin, (self->move_angles[0] + self->move_origin[0])/2,
			(self->move_angles[1] + self->move_origin[1])/2,
			(self->move_angles[2] + self->move_origin[2])/2);
		VectorSet (self->mins, self->move_origin[0] - self->s.origin[0],
			self->move_origin[1] - self->s.origin[1],
			self->move_origin[2] - self->s.origin[2]);
		VectorSet (self->maxs, self->move_angles[0]- self->s.origin[0],
			self->move_angles[1]- self->s.origin[1],
			self->move_angles[2]- self->s.origin[2]);

	}
	else
	{
		G_FreeEdict(self);
		return;
	}

	//so bots, to find nearest campspot.  either mapper sets it or it goes to the center
	if (VectorCompare (self->obj_origin, vec3_origin))
	{
		VectorCopy (self->s.origin, self->obj_origin);
	}

	self->obj_perm_owner = -1; //for bots to work correctly

	gi.linkentity (self);
	
}


/*
========================
func_explosive_objective
========================
*/
void func_explosive_objective_explode (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t	origin;
	vec3_t	chunkorigin;
	vec3_t	size;
	int		count;
	int		mass;
	int		enemy;
	int		otherteam;

	//gi.dprintf("self: %s\ninflictor: %s\n attacker: %s\n",
	//	self->classname, inflictor->classname, attacker->classname);

	if (!attacker->client ||
		!attacker->client->resp.mos)
		return;

	// bmodel origins are (0 0 0), we need to adjust that here
	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);
	VectorCopy (origin, self->s.origin);

	self->takedamage = DAMAGE_NO;

	if (self->dmg)
		T_RadiusDamage (self, attacker, self->dmg, NULL, self->dmg+40, MOD_EXPLOSIVE);

	VectorSubtract (self->s.origin, inflictor->s.origin, self->velocity);
	VectorNormalize (self->velocity);
	VectorScale (self->velocity, 150, self->velocity);

	// start chunks towards the center
	VectorScale (size, 0.5, size);

	mass = self->mass;
	if (!mass)
		mass = 75;

	// big chunks
	if (mass >= 100)
	{
		count = mass / 100;
		if (count > 8)
			count = 8;
		while(count--)
		{
			chunkorigin[0] = origin[0] + crandom() * size[0];
			chunkorigin[1] = origin[1] + crandom() * size[1];
			chunkorigin[2] = origin[2] + crandom() * size[2];
			ThrowDebris (self, "models/objects/debris1/tris.md2", 1, chunkorigin);
		}
	}

	// small chunks
	count = mass / 25;
	if (count > 16)
		count = 16;
	while(count--)
	{
		chunkorigin[0] = origin[0] + crandom() * size[0];
		chunkorigin[1] = origin[1] + crandom() * size[1];
		chunkorigin[2] = origin[2] + crandom() * size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", 2, chunkorigin);
	}

	G_UseTargets (self, attacker);

	// hack for 2 team games

	if (self->obj_owner != 99) {
		team_list[self->obj_owner]->score -= self->obj_loss;
		enemy = (self->obj_owner) ? 0 : 1;
	} else
		enemy = 99;

	if (self->obj_owner != attacker->client->resp.team_on->index)
		team_list[attacker->client->resp.team_on->index]->score += self->obj_gain;
	else if (self->obj_owner == attacker->client->resp.team_on->index && enemy != 99)
		team_list[enemy]->score += self->obj_gain;

	if (dedicated->value)
		safe_cprintf(NULL, PRINT_HIGH, "%s destroyed by %s [%s]\n", 
			self->obj_name, 
			attacker->client->pers.netname,
			team_list[attacker->client->resp.team_on->index]->teamname);

	centerprintall("%s destroyed by:\n\n%s\n%s",
		self->obj_name, 
		attacker->client->pers.netname,
		team_list[attacker->client->resp.team_on->index]->teamname);


	otherteam = (self->obj_owner+1)%2;
		if (!team_list[otherteam]->kills_and_points && team_list[otherteam]->score < team_list[otherteam]->need_points ||
			(team_list[otherteam]->kills_and_points && 
				team_list[otherteam]->kills < team_list[otherteam]->need_kills))
			gi.sound(self, CHAN_NO_PHS_ADD, gi.soundindex(va("%s/objectives/touch_cap.wav", team_list[otherteam]->teamid)), 1, 0, 0);

//		gi.dprintf ("pts:%i  ndpts:%i  kills:%i  ndkills:%i\n",team_list[(self->obj_owner+1)%2]->score,team_list[(self->obj_owner+1)%2]->need_points,
//team_list[(self->obj_owner+1)%2]->kills,team_list[(self->obj_owner+1)%2]->need_kills);

	if (self->deathtarget)
	{	
		self->target = self->deathtarget;
		if (self->target)
			G_UseTargets (self, attacker);
	}



	if (self->dmg)
		BecomeExplosion1 (self);
	else
		G_FreeEdict (self);
}

void func_explosive_objective_use(edict_t *self, edict_t *other, edict_t *activator)
{
	func_explosive_objective_explode (self, self, other, self->health, vec3_origin);
}

void func_explosive_objective_spawn (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox (self);
	gi.linkentity (self);
}

void SP_func_explosive_objective (edict_t *self)
{
	self->classnameb = FUNC_EXPLOSIVE_OBJECTIVE;

	self->movetype = MOVETYPE_PUSH;


	gi.modelindex ("models/objects/debris1/tris.md2");
	gi.modelindex ("models/objects/debris2/tris.md2");

	gi.setmodel (self, self->model);

	if (self->spawnflags & 1)
	{
		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->use = func_explosive_objective_spawn;
	}
	else
	{
		self->solid = SOLID_BSP;
		if (self->targetname)
			self->use = func_explosive_objective_use;
	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	if (self->use != func_explosive_objective_use)
	{
		if (!self->health)
			self->health = 500;
		self->die = func_explosive_objective_explode;
		self->takedamage = DAMAGE_YES;
	}

	if (!self->obj_name)
		self->obj_name = "Objective";
	if (!self->obj_gain)
		self->obj_gain = 5;
//	if (!self->obj_loss)
//		self->obj_loss = 5;




	//so bots can aim at center.  either mapper sets it or it goes to the center
	if (VectorCompare (self->obj_origin, vec3_origin))
	{
		VectorSet (self->obj_origin, (self->absmax[0] + self->absmin[0])/2,
		(self->absmax[1] + self->absmin[1])/2,
		(self->absmax[2] + self->absmin[2])/2);
	}


	gi.linkentity (self);
}

void GetMapObjective (void)
{
	FILE	*map_file;
	char	filename[256];
	
/*	Q_strncpyz (filename, sizeof(filename), GAMEVERSION "/pics/objectives/");		
	Q_strncatz (filename, sizeof(filename), level.mapname);
	Q_strncatz (filename, sizeof(filename),".pcx"); */
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/pics/objectives/%s.pcx", GameDir(), level.mapname);

	gi.dprintf ("Loading map objective pic %s...", filename);
	if (map_file = fopen(filename, "r")) 
	{
		fclose (map_file);
		level.objectivepic = filename;
		gi.dprintf ("done.\n");
	} 
	else
		gi.dprintf ("error.\n");
}

//faf:  ctb code

qboolean briefcase_respawn_needed;

void SP_briefcase (edict_t *self)
{
	if (!self->count)
		level.ctb_time = 180;
	else
		level.ctb_time = self->count;

    SpawnItem(self,FindItemByClassname("briefcase"));
}


void DoRespawn(edict_t * ent ); 
void briefcase_spawn_think(edict_t *ent)
{
	if (briefcase_respawn_needed)
	{
		ent->think = DoRespawn;
		ent->nextthink = level.time + 1;
		briefcase_respawn_needed = false;
	}
	else
		ent->nextthink = level.time + 10; //check every 10 seconds
}

void Set_Briefcase_Respawn (edict_t *ent)
{
	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->nextthink = level.time + 10;//level.time + delay;
	ent->think = briefcase_spawn_think;//DoRespawn;
	gi.linkentity (ent);
}



qboolean Pickup_Briefcase (edict_t *ent, edict_t *other)
{
	int			index;
	gitem_t		*item;
	index = ITEM_INDEX(ent->item);

	item = ent->item;

	other->client->pers.inventory[index]++;
	other->s.modelindex3 = gi.modelindex ("models/objects/briefcase/w_briefcase.md2");
	gi.bprintf (PRINT_HIGH, "%s picked up the briefcase for team %s!\n", other->client->pers.netname, other->client->resp.team_on->teamname);

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		Set_Briefcase_Respawn (ent);

	briefcase_respawn_needed = false;

	other->client->has_briefcase = true;//used to display icon in hud
	
	return true;
}

void Drop_Briefcase (edict_t *ent, gitem_t *item)
{
	if (!item)
		return; // out of ammo, switched before frame?

	Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	
	ent->s.modelindex3 = 0;
	gi.cprintf(ent, PRINT_HIGH, "You dropped the briefcase!\n");

	ent->client->has_briefcase = false;//used to display icon in hud

}

void briefcase_respawn (edict_t *ent)
{
//	edict_t *check;

	G_FreeEdict (ent);

	briefcase_respawn_needed = true;
	/*
	for (check = g_edicts; check < &g_edicts[globals.num_edicts]; check++)
	{
		if (!check->inuse)
			continue;

		//the briefcase spawn point
		if (!strcmp(check->classname, "briefcase"))
		{
			SpawnItem(check, FindItemByClassname("briefcase"));
		}
	}*/
}





void briefcase_warn (edict_t *ent)
{
	edict_t *e;
	int i;

	if (ent->owner &&
		ent->owner->client)
	{
		for (i=0 ; i < game.maxclients ; i++)
		{

			e = g_edicts + 1 + i;
			if (!e->inuse || !e->client)
				continue;
			
			gi.cprintf(e, PRINT_HIGH, "The briefcase has not been touched in 1 minute.  It will be respawned in 30 seconds if it's not picked up!\n");
		}
	}

	ent->think = briefcase_respawn;
	ent->nextthink = level.time + 30;
}

void SP_briefcase (edict_t *self);
void SP_usa_base (edict_t *ent);
void SP_grm_base (edict_t *ent);

//faf: ctb code
void Create_CTB_Entities (edict_t *self)
{
	edict_t	*spot = NULL;
//	spot = NULL;

	return;

#if 0	// Knightmare- unreachable code
	if ( !Q_stricmp(level.mapname, "dday3") )  
	{
		spot = G_Spawn();
		spot->classname = "usa_base";
		spot->s.origin[0] = -1519.5;
		spot->s.origin[1] = 1397.3;
		spot->s.origin[2] = 86.5;
		SP_usa_base(spot);

		spot = G_Spawn();
		spot->classname = "grm_base";
		spot->s.origin[0] = 1605.7;
		spot->s.origin[1] = 176.9;
		spot->s.origin[2] = 199.3;
		SP_grm_base(spot);

		spot = G_Spawn();
		spot->classname = "briefcase";
		spot->sounds = 0;
		spot->s.origin[0] = -1085.5;
		spot->s.origin[1] = 27.4;
		spot->s.origin[2] = 345.3;
//		spot->s.angles[1] = 90;
		SP_briefcase (spot);
	}
	else if ( !Q_stricmp(level.mapname, "dday2") )  
	{

		spot = G_Spawn();
		spot->classname = "usa_base";
		spot->s.origin[0] = 1288.2;
		spot->s.origin[1] = 1080.6;
		spot->s.origin[2] = -60;
		SP_usa_base(spot);

		spot = G_Spawn();
		spot->classname = "grm_base";
		spot->s.origin[0] = -2262.5;
		spot->s.origin[1] = 280;
		spot->s.origin[2] = -100.2;
		SP_grm_base(spot);

		spot = G_Spawn();
		spot->classname = "briefcase";
		spot->s.origin[0] = -768.4;
		spot->s.origin[1] = 1017.6;
		spot->s.origin[2] = 91.9;
		SP_briefcase (spot);		
		gi.dprintf("dfjklsdfjklsdfjklsdfjkl\n");
		
	}
	else if ( !Q_stricmp(level.mapname, "invade2") )  
	{

		spot = G_Spawn();
		spot->classname = "usa_base";
		spot->s.origin[0] = -2498.4;
		spot->s.origin[1] = 2213.1;
		spot->s.origin[2] = -510.5;
		SP_usa_base(spot);

		spot = G_Spawn();
		spot->classname = "grm_base";
		spot->s.origin[0] = 580.9;
		spot->s.origin[1] = -931.2;
		spot->s.origin[2] = -508.5;
		SP_grm_base(spot);


		spot = G_Spawn();
		spot->classname = "briefcase";
		spot->sounds = 2;
		spot->s.origin[0] = -1501.4;
		spot->s.origin[1] = 691.5;
		spot->s.origin[2] = -467.9;
		SP_briefcase (spot);
		
	}
	else if ( !Q_stricmp(level.mapname, "mp1dday2") )  
	{

		spot = G_Spawn();
		spot->classname = "usa_base";
		spot->s.origin[0] = 3326.5;
		spot->s.origin[1] = -2496;
		spot->s.origin[2] = -310.8;
		SP_usa_base(spot);

		spot = G_Spawn();
		spot->classname = "grm_base";
		spot->s.origin[0] = 3126.4;
		spot->s.origin[1] = 852.6;
		spot->s.origin[2] = 122.4;
		SP_grm_base(spot);

		spot = G_Spawn();
		spot->classname = "briefcase";
		spot->sounds = 0;
		spot->s.origin[0] = 145.5;
		spot->s.origin[1] = -923.9;
		spot->s.origin[2] = 94.3;
		SP_briefcase (spot);
		
	}
	else if ( !Q_stricmp(level.mapname, "inland4") )  
	{

		spot = G_Spawn();
		spot->classname = "usa_base";
		spot->s.origin[0] = -2445.6;
		spot->s.origin[1] = -164.9;
		spot->s.origin[2] = -130.1;
		SP_usa_base(spot);

		spot = G_Spawn();
		spot->classname = "grm_base";
		spot->s.origin[0] = 2092.4;
		spot->s.origin[1] = 303;
		spot->s.origin[2] = -153.5;
		SP_grm_base(spot);

	
		spot = G_Spawn();
		spot->classname = "briefcase";
		spot->sounds = 1;
		spot->s.origin[0] = 7.5;
		spot->s.origin[1] = 1321.7;
		spot->s.origin[2] = -75.6;
		spot->s.angles[1] = 270;
		SP_briefcase (spot);
			
	}
	else if ( !Q_stricmp(level.mapname, "dunkirk") )  
	{

		spot = G_Spawn();
		spot->classname = "usa_base";
		spot->s.origin[0] = -399.7;
		spot->s.origin[1] = 2840.4;
		spot->s.origin[2] = -190.8;
		spot->s.angles[1] = 90;
		SP_usa_base(spot);
	
		spot = G_Spawn();
		spot->classname = "grm_base";
		spot->s.origin[0] = -646.3;
		spot->s.origin[1] = -218;
		spot->s.origin[2] = -190.8;
		spot->s.angles[1] = 90;
		SP_grm_base(spot);

		spot = G_Spawn();
		spot->classname = "briefcase";
		spot->sounds = 2;
		spot->s.origin[0] = 1401.3;
		spot->s.origin[1] = -260.7;
		spot->s.origin[2] = -165.7;
		spot->s.angles[1] = 315;
		SP_briefcase (spot);
		
	}	  
	else if ( !Q_stricmp(level.mapname, "inland1") )  
	{
		spot = G_Spawn();
		spot->classname = "usa_base";
		spot->s.origin[0] = 373;
		spot->s.origin[1] = 1044;
		spot->s.origin[2] = -280;
		spot->s.angles[1] = 90;
		SP_usa_base(spot);
	
		spot = G_Spawn();
		spot->classname = "grm_base";
		spot->s.origin[0] = 50;
		spot->s.origin[1] = -1179;
		spot->s.origin[2] = 7;
		spot->s.angles[1] = 90;
		SP_grm_base(spot);

		spot = G_Spawn();
		spot->classname = "briefcase";
		spot->sounds = 2;
		spot->s.origin[0] = -373;
		spot->s.origin[1] = -131;
		spot->s.origin[2] = 56;
		spot->s.angles[1] = 315;
		SP_briefcase (spot);
	}	  
	else if ( !Q_stricmp(level.mapname, "inland2") )  
	{
		spot = G_Spawn();
		spot->classname = "usa_base";
		spot->s.origin[0] = 329;
		spot->s.origin[1] = -352;
		spot->s.origin[2] = -93;
		spot->s.angles[1] = 90;
		SP_usa_base(spot);

		spot = G_Spawn();
		spot->classname = "grm_base";
		spot->s.origin[0] = -2513;
		spot->s.origin[1] = 912;
		spot->s.origin[2] = -65;
		spot->s.angles[1] = 90;
		SP_grm_base(spot);

		spot = G_Spawn();
		spot->classname = "briefcase";
		spot->sounds = 2;
		spot->s.origin[0] = -1021;
		spot->s.origin[1] = 213;
		spot->s.origin[2] = 74;
		spot->s.angles[1] = 10;
		SP_briefcase (spot);
	}
	else if ( !Q_stricmp(level.mapname, "invade6") )
	{
		spot = G_Spawn();
		spot->classname = "usa_base";
		spot->s.origin[0] = 914;
		spot->s.origin[1] = -370;
		spot->s.origin[2] = -510;
		spot->s.angles[1] = 90;
		SP_usa_base(spot);

		spot = G_Spawn();
		spot->classname = "grm_base";
		spot->s.origin[0] = -1428;
		spot->s.origin[1] = 3190;
		spot->s.origin[2] = -510;
		spot->s.angles[1] = 90;
		SP_grm_base(spot);

		spot = G_Spawn();
		spot->classname = "briefcase";
		spot->sounds = 2;
		spot->s.origin[0] = 525;
		spot->s.origin[1] = 2543;
		spot->s.origin[2] = -423;
		spot->s.angles[1] = 10;
		SP_briefcase (spot);
	}	  

#endif	// Knightmare- end unreachable code
}



//faf:  ctb code
void base_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 13; //faf
	ent->nextthink = level.time + FRAMETIME;
}

void SP_usa_base (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/objects/usaflag/tris.md2");
//	ent->s.frame = rand() % 16;
//	ent->s.frame = 1;
	gi.linkentity (ent);

	ent->think = base_think;
	ent->nextthink = level.time + FRAMETIME;
	ent->s.sound = gi.soundindex("faf/flag.wav");
	
}
void SP_grm_base (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/objects/grmflag/tris.md2");
//	ent->s.frame = rand() % 16;
//	ent->s.frame = 1;
	gi.linkentity (ent);

	ent->think = base_think;
	ent->nextthink = level.time + FRAMETIME;
	ent->s.sound = gi.soundindex("faf/flag.wav");

}
//end faf



