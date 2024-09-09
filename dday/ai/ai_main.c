/*
Copyright (C) 1997-2001 Id Software, Inc.

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

#include "../g_local.h"
#include "ai_local.h"
#include "stddef.h"

// Knightmare- moved these vars here to fix compile on GCC
int	num_AIEnemies;
edict_t *AIEnemies[MAX_EDICTS];		// pointers to all players in the game

ai_devel_t	AIDevel;

nav_plink_t pLinks[MAX_NODES];		// pLinks array
nav_node_t nodes[MAX_NODES];		// nodes array

ai_navigation_t	nav;

ai_weapon_t		AIWeapons[WEAP_TOTAL];
// end Knightmare

int AI_ClosestNodeToSpotx (vec3_t origin);
int AI_ClosestNodeToSpot (vec3_t origin, edict_t *passent, qboolean visible);

//ACE


//set the nearest camp spot to each objective to be used only for objective capping/destroying
void Set_Up_CampSpots_For_Ents (void)
{
	edict_t		*e;
	int			i, j, team, obj_camp, tm;
	float		nearest_distance;
	vec3_t		distv;
	qboolean	allset;

	for (i=0; i<globals.num_edicts; i++)
	{
		team = -999;

		e = &g_edicts[i];

		if (!e->inuse)
			continue;

		if (!e->classnameb)
			continue;

		if (!(e->classnameb == OBJECTIVE_TOUCH ||
			e->classnameb == FUNC_EXPLOSIVE_OBJECTIVE ||
			e->classnameb == OBJECTIVE_VIP ||
			e->classnameb == FUNC_TRAIN))
			continue;


		if (!e->obj_gain && !e->health)
			continue;


		if (VectorCompare (vec3_origin, e->obj_origin))
			continue;

		if (e->obj_owner > -99)
			team = e->obj_owner;


		//set a campspot to CAMP_OBJECTIVE, once for each team (if needed)
		allset = false;
		for (tm = 0; tm < 2; tm++)
		{
			nearest_distance = 99999999999;
			obj_camp = -1;
			if (allset)
				break;

			if ( e->classnameb == OBJECTIVE_TOUCH ||
				(e->classnameb == FUNC_EXPLOSIVE_OBJECTIVE && team == (tm+1)%2) ||
				(e->classnameb == OBJECTIVE_VIP && team == (tm+1)%2) ||
				(e->classnameb == FUNC_TRAIN && team == (tm+1)%2) ) //other team
			{
				for (j = 0; j< total_camp_spots; j++)
				{
					if (camp_spots[j].type == CAMP_OBJECTIVE) //already taken
						continue;
					if (e->classnameb == OBJECTIVE_TOUCH &&
						camp_spots[j].team == (tm+1)%2) //no axis campspots
						continue;
					if (e->classnameb == FUNC_EXPLOSIVE_OBJECTIVE &&
						camp_spots[j].team == team)
						continue;
					if (e->classnameb == OBJECTIVE_VIP &&
						camp_spots[j].team == team)
						continue;
					if (e->classnameb == OBJECTIVE_TOUCH &&
						e->obj_perm_owner == tm) //can't recap
						continue;


					VectorSubtract (e->obj_origin, camp_spots[j].origin, distv);

					if (VectorLength (distv) < nearest_distance)
					{
						nearest_distance = VectorLength (distv);
						obj_camp = j;

					}
				}
				if (obj_camp > -1)
				{
					camp_spots[obj_camp].type = CAMP_OBJECTIVE;

					if (e->classnameb == OBJECTIVE_VIP || e->classnameb == FUNC_EXPLOSIVE_OBJECTIVE || e->classnameb == FUNC_TRAIN)
						allset = true;

					//gi.dprintf("camptarg #%i set to CAMP_OBJ for Team#%i for %s/%s\n", obj_camp, tm, e->obj_name, e->message);

				}
			}
		}

	}


}


//==========================================
// AI_Init
// Inits global parameters
//==========================================
void AI_Init (void)
{
	// Init developer mode
	AIDevel.debugMode = false;
	AIDevel.debugChased = false;
	AIDevel.chaseguy = NULL;
	AIDevel.showPLinks = false;
	AIDevel.plinkguy = NULL;
}

//==========================================
// AI_NewMap
// Inits Map local parameters
//==========================================
void AI_NewMap (void)
{
	// Load nodes
	AI_InitNavigationData ();

	AI_InitAIWeapons ();

	Set_Up_CampSpots_For_Ents ();
}

//==========================================
// G_FreeAI
// removes the AI handle from memory
//==========================================
void G_FreeAI (edict_t *ent)
{
	if ( !ent->ai ) return;

	gi.TagFree (ent->ai);
	ent->ai = NULL;
}

//==========================================
// G_SpawnAI
// allocate ai_handle_t for this entity
//==========================================
void G_SpawnAI (edict_t *ent)
{
	if ( !ent->ai )
		ent->ai = gi.TagMalloc (sizeof(ai_handle_t), TAG_LEVEL);

	memset( ent->ai, 0, sizeof(ai_handle_t));
}

//==========================================
// AI_SetUpMoveWander
//==========================================
void AI_SetUpMoveWander (edict_t *ent)
{
	ent->ai->state = BOT_STATE_WANDER;
	ent->ai->wander_timeout = level.time + 1.0;
	ent->ai->nearest_node_tries = 0;

	ent->ai->next_move_time = level.time;
	ent->ai->bloqued_timeout = level.time + 15.0;

	ent->ai->goal_node = INVALID;
	ent->ai->current_node = INVALID;
	ent->ai->next_node = INVALID;
}


//==========================================
// AI_ResetWeights
// Init bot weights from bot-class weights.
//==========================================
void AI_ResetWeights (edict_t *ent)
{
	//restore defaults from bot persistant
//	memset(ent->ai->status.inventoryWeights, 0, sizeof (ent->ai->status.inventoryWeights));
//	memcpy(ent->ai->status.inventoryWeights, ent->ai->pers.inventoryWeights, sizeof(ent->ai->pers.inventoryWeights));
}


//==========================================
// AI_ResetNavigation
// Init bot navigation. Called at first spawn & each respawn
//==========================================
void AI_ResetNavigation (edict_t *ent)
{
	int		i;

	ent->enemy = NULL;
	ent->movetarget = NULL;
	ent->ai->state_combat_timeout = 0.0;

	ent->ai->state = BOT_STATE_WANDER;
	ent->ai->wander_timeout = level.time;
	ent->ai->nearest_node_tries = 0;

	ent->ai->next_move_time = level.time;
	ent->ai->bloqued_timeout = level.time + 15.0;

	ent->ai->goal_node = INVALID;
	ent->ai->current_node = INVALID;
	ent->ai->next_node = INVALID;

	VectorSet( ent->ai->move_vector, 0, 0, 0 );

	//reset bot_roams timeouts
	for( i=0; i<nav.num_broams; i++)
		ent->ai->status.broam_timeouts[i] = 0.0;

	ent->client->aim = false;
}


//==========================================
// AI_BotRoamForLRGoal
//
// Try assigning a bot roam node as LR Goal
//==========================================
qboolean AI_BotRoamForLRGoal (edict_t *self, int current_node)
{
	int		i;
	float	cost;
	float	weight, best_weight = 0.0;
	int		goal_node = INVALID;
	int		best_broam = INVALID;
	float	dist;

	if (!nav.num_broams)
		return false;

	for( i=0; i<nav.num_broams; i++)
	{
		if ( self->ai->status.broam_timeouts[i] > level.time)
			continue;

		//limit cost finding by distance
		dist = AI_Distance( self->s.origin, nodes[nav.broams[i].node].origin );
		if ( dist > 10000 )
			continue;

		//find cost
		cost = AI_FindCost(current_node, nav.broams[i].node, self->ai->pers.moveTypesMask);
		if (cost == INVALID || cost < 3) // ignore invalid and very short hops
			continue;

		cost *= random(); // Allow random variations for broams
		weight = nav.broams[i].weight / cost;	// Check against cost of getting there

		if (weight > best_weight)
		{
			best_weight = weight;
			goal_node = nav.broams[i].node;
			best_broam = i;
		}
	}

	if (best_weight == 0.0 || goal_node == INVALID)
		return false;

	//set up the goal
	self->ai->state = BOT_STATE_MOVE;
	self->ai->tries = 0;	// Reset the count of how many times we tried this goal

//	if (AIDevel.debugChased && bot_showlrgoal->value)
//		G_PrintMsg (AIDevel.chaseguy, PRINT_HIGH, "%s: selected a bot roam of weight %f at node %d for LR goal.\n",self->ai->pers.netname, nav.broams[best_broam].weight, goal_node);

	AI_SetGoal(self,goal_node);

	return true;
}



int Closest_CMP_to_Ent (edict_t *self, edict_t *obj)
{
	int j;
	float nearest_distance;
	vec3_t obj_orig;
	vec3_t distv;
	int obj_camp = -1;


	nearest_distance = 99999999999;
	for (j = 0; j<total_camp_spots; j++)
	{
		if (!camp_spots[j].type)
			continue;

		if (camp_spots[j].team != self->client->resp.team_on->index)
			continue;
		if (camp_spots[j].owner && obj->classnameb != OBJECTIVE_TOUCH)
			continue;

		else if (!VectorCompare (obj->obj_origin, vec3_origin))
			VectorCopy (obj->obj_origin, obj_orig);
		else //func_train
			VectorSet (obj_orig, (obj->absmax[0] + obj->absmin[0])/2,(obj->absmax[1] + obj->absmin[1])/2,(obj->absmax[2] + obj->absmin[2])/2);

		VectorSubtract (obj_orig, camp_spots[j].origin, distv);


		//gi.dprintf("%s    %i VL %f\n", vtos(obj_orig), j, VectorLength(distv));

		if (VectorLength (distv) < nearest_distance)
		{
			nearest_distance = VectorLength (distv);
			obj_camp = j;

		}
	}
	return obj_camp;


}


//==========================================
// AI_PickLongRangeGoal
//
// Evaluate the best long range goal and send the bot on
// its way. This is a good time waster, so use it sparingly.
// Do not call it for every think cycle.
//
// jal: I don't think there is any problem by calling it,
// now that we have stored the costs at the nav.costs table (I don't do it anyway)
//==========================================
void AI_PickLongRangeGoal (edict_t *self)
{
	int		i;
	int		node;
	float	weight,best_weight=0.0;
	int		current_node, goal_node = INVALID;
	edict_t *goal_ent = NULL;
	float	cost;
	int		nearest = -1;
	float	temp_distance, nearest_distance = 9999999,nearest_distanceb = 9999999;
	vec3_t	distb;

	int		j, randseed, k;

	edict_t *closest = NULL;
	edict_t *e = NULL;

	int obj_camp = -1;
	edict_t *obj = NULL;
	int	obj_count = 0;
	int rand_obj_num;

	self->ai->objective = NULL;
	self->ai->reached_obj_time = 0;

	if (self->ai->camp_targ > -1)
	{
		camp_spots[self->ai->camp_targ].owner = NULL;
		self->ai->camp_targ = -1;
	}

	// look for a target
	//current_node = AI_FindClosestReachableNode(self->s.origin, self,((1+self->ai->nearest_node_tries)*NODE_DENSITY),NODE_ALL);
	current_node = AI_FindClosestReachableNode(self->s.origin, self,((10+self->ai->nearest_node_tries)*NODE_DENSITY),NODE_ALL);
	self->ai->current_node = current_node;

	if (current_node == -1)	//failed. Go wandering :(
	{
//		if (AIDevel.debugChased && bot_showlrgoal->value)
//			G_PrintMsg (AIDevel.chaseguy, PRINT_HIGH, "%s: LRGOAL: Closest node not found. Tries:%i\n", self->ai->pers.netname, self->ai->nearest_node_tries);

		//gi.dprintf("NO NEARBY NODE\n");
		if ( self->ai->state != BOT_STATE_WANDER )
			AI_SetUpMoveWander( self );

		self->ai->wander_timeout = level.time + 1.0;
		self->ai->nearest_node_tries++;	//extend search radius with each try
		return;
	}
	self->ai->nearest_node_tries = 0;

	// Players: This should be its own function and is for now just finds a player to set as the goal.
	for( i=0; i<num_AIEnemies; i++ )
	{
		//ignore self & spectators
		if ( AIEnemies[i] == self || AIEnemies[i]->svflags & SVF_NOCLIENT)
			continue;

		//ignore zero weighted players
		if ( self->ai->status.playersWeights[i] == 0.0f )
			continue;

		node = AI_FindClosestReachableNode( AIEnemies[i]->s.origin, AIEnemies[i], NODE_DENSITY, NODE_ALL);
		cost = AI_FindCost(current_node, node, self->ai->pers.moveTypesMask);

		if (cost == INVALID || cost < 4) // ignore invalid and very short hops
			continue;

		//precomputed player weights
		weight = self->ai->status.playersWeights[i];

		//weight *= random(); // Allow random variations
		weight /= cost; // Check against cost of getting there

		if (weight > best_weight)
		{
			best_weight = weight;
			goal_node = node;
			goal_ent = AIEnemies[i];
		}
	}

	//look for objectives

	if (!no_objectives_left && self->ai->defend_bot == false)
	{
		no_objectives_left = true;
		//count objectives
		for (i=0 ; i<game.maxentities ; i++)
		{
			e = &g_edicts[i];
			if (!e->inuse)
				continue;
			if (!e->classnameb)
				continue;
			//if (!e->obj_owner)
			//	continue;
			if (!e->obj_gain && !e->health)
				continue;
			if (! (e->classnameb == FUNC_EXPLOSIVE_OBJECTIVE ||
				e->classnameb == FUNC_TRAIN ||
				e->classnameb == OBJECTIVE_VIP ||
				e->classnameb == OBJECTIVE_TOUCH) )
				continue;

			no_objectives_left = false;

			if (e->obj_owner == self->client->resp.team_on->index)
				continue;


			obj_count++;
		}

	//	gi.dprintf("obj_count: %i\n",obj_count);

		if (obj_count > 0)
		{

			rand_obj_num = 1 + rand()%obj_count;

			nearest_distance = 999999999;

			obj_count = 1;
			for (i=0 ; i<game.maxentities ; i++)
			{
				e = &g_edicts[i];

				if (!e->inuse)
					continue;
				if (!e->classnameb)
					continue;
				//if (!e->obj_owner)
				//	continue;
				if (!e->obj_gain && !e->health)
					continue;

				if (! (e->classnameb == FUNC_EXPLOSIVE_OBJECTIVE ||
					e->classnameb == FUNC_TRAIN ||
					e->classnameb == OBJECTIVE_VIP ||
					e->classnameb == OBJECTIVE_TOUCH) )
					continue;

				if (e->obj_owner == self->client->resp.team_on->index)
					continue;

				if (obj_count != rand_obj_num)
				{
					obj_count++;
					continue;
				}

				obj = e;
				break;
			}

			//assign closest camping spot to objective to player.
			if (obj)
			{
				self->ai->objective = obj;
				obj_camp = Closest_CMP_to_Ent(self, obj);
			}

			if (obj_camp > -1)
			{
					camp_spots[obj_camp].owner = self;
					self->ai->camp_targ = obj_camp;
					self->ai->start_camp_time = level.time;
					//gi.dprintf ("cto %i %s\n", obj_camp, vtos(camp_spots[self->ai->camp_targ].origin));
					//gi.dprintf("cs %i\n",self->ai->camp_targ);

			}
			else //shouldn't really happen (but probably will)
			{
				self->ai->objective = NULL;
				self->ai->reached_obj_time = 0;
			}
		}
	}

	// select camping spot
	if (!self->ai->objective)
	{
		nearest_distance = 99999999999;
		randseed = (int)rand()%total_camp_spots;

		//hacky stuff: if ai->camp_targ == -2, that means they want to find another camp spot,
		//but one that is nearby, otherwise it is random

		//actually make it 50% of the time go to closest

		for (j = 0; j<total_camp_spots; j++)
		{
			k = j + randseed%total_camp_spots;
			if (camp_spots[k].type != CAMP_NORMAL)
				continue;
			if (camp_spots[k].team != self->client->resp.team_on->index)
				continue;
			if (camp_spots[k].owner)
				continue;
			if (self->client->resp.mos == H_GUNNER &&
				camp_spots[k].stance == STANCE_STAND)
				continue;

			if (self->ai->camp_targ == -2 || rand()%2 == 1)
			{
				VectorSubtract (camp_spots[k].origin, self->s.origin, distb);

				temp_distance = VectorLength(distb);
				if (temp_distance < nearest_distance)
				{
					nearest_distance = temp_distance;
					nearest = k;
				}
			}
			else
			{
				nearest = k;
				break;
			}
		}

		if (nearest > -1)
		{
			camp_spots[nearest].owner = self;
			self->ai->camp_targ = nearest;
			self->ai->start_camp_time = level.time;
		}
	}



	if (self->ai->camp_targ > -1)
	{
		goal_node = AI_ClosestNodeToSpot(camp_spots[self->ai->camp_targ].origin, self, true);
		//VectorCopy (camp_spots[self->ai->camp_targ].origin, self->s.origin);
	}
	if (goal_node)
		best_weight = .5;



	// If do not find a goal, go wandering....
	if (best_weight == 0.0 || goal_node == INVALID)
	{
		//BOT_ROAMS
		if (!AI_BotRoamForLRGoal(self, current_node))
		{
			self->ai->goal_node = INVALID;
			self->ai->state = BOT_STATE_WANDER;
			self->ai->wander_timeout = level.time + 1.0;
//			if (AIDevel.debugChased && bot_showlrgoal->value)
//				G_PrintMsg (AIDevel.chaseguy, PRINT_HIGH, "%s: did not find a LR goal, wandering.\n",self->ai->pers.netname);
		}
		return; // no path?
	}

	// OK, everything valid, let's start moving to our goal.
	self->ai->state = BOT_STATE_MOVE;
	self->ai->tries = 0;	// Reset the count of how many times we tried this goal

//	if (goal_ent != NULL && AIDevel.debugChased && bot_showlrgoal->value)
//		G_PrintMsg (AIDevel.chaseguy, PRINT_HIGH, "%s: selected a %s at node %d for LR goal.\n",self->ai->pers.netname, goal_ent->classname, goal_node);

	AI_SetGoal (self, goal_node);
}


//==========================================
// AI_PickShortRangeGoal
// Pick best goal based on importance and range. This function
// overrides the long range goal selection for items that
// are very close to the bot and are reachable.
//==========================================
void AI_PickShortRangeGoal (edict_t *self)
{
	edict_t *target;
	float	best_weight=0.0;
	edict_t *best = NULL;

	vec3_t	ent_orig;
	float nearest_distance = 9999999;
	vec3_t dist, distv;
	edict_t	*e = NULL, *closest = NULL;
	int i;

	if ( !self->client )
		return;


	if (!self->ai->defend_bot)
	{
		//look for objectives
		for (i=0 ; i<game.maxentities ; i++)
		{
			e = &g_edicts[i];

			if (!e->inuse)
				continue;
			if (!e->classname)
				continue;
			if (!(e->classnameb == FUNC_EXPLOSIVE_OBJECTIVE || e->classnameb == OBJECTIVE_VIP || e->classnameb == FUNC_TRAIN))
				continue;
			if (!e->obj_gain)
				continue;
	//		if (!e->obj_owner)
	//			continue;
			if (e->obj_owner == self->client->resp.team_on->index)
				continue;

			//inland1 save these 2 for last
	/*		if (closest && (!strcmp (closest->obj_name, "Axis Light Supplies") || !strcmp (closest->obj_name, "Axis Heavy Machine")))
			{
				if (!strcmp (e->obj_name, "Axis Light Supplies"))
					continue;
				if (!strcmp (e->obj_name, "Axis Heavy Machine"))
					continue;
			} */

			else if (!VectorCompare (e->obj_origin, vec3_origin))
				VectorCopy (e->obj_origin, ent_orig);
			else
				VectorSet (ent_orig, (e->absmax[0] + e->absmin[0])/2,(e->absmax[1] + e->absmin[1])/2,(e->absmax[2] + e->absmin[2])/2);

			if (!objective_hittable(self, e, ent_orig))
				continue;

			VectorSubtract (ent_orig, self->s.origin, distv);
			if (VectorLength (distv) < nearest_distance //||
				//(closest && !strcmp (closest->obj_name, "Bridge")) // save bridge for last (dday4)
				)
			{

				nearest_distance = VectorLength (distv);
				closest = e;
			}
		}
				//gi.dprintf("\n\n");

		if (closest)// && nearest_distance < 100)
		{
			//	VectorSet (ent_orig,
			//		(closest->absmax[0] + closest->absmin[0])/2,
			//		(closest->absmax[1] + closest->absmin[1])/2,
			//		(closest->absmax[2] + closest->absmin[2])/2);

			//gi.dprintf("shortrange:  %s %s\n",closest->classname, vtos(ent_orig));
			//if (BOT_DMclass_CheckShot(self, ent_orig) &&
				//objective_hittable(self, closest, ent_orig))
				self->enemy = closest;

				self->ai->bloqued_timeout = level.time + 10.0;

				self->ai->objective = closest;

				if (self->ai->camp_targ > -1)
				{
					camp_spots[self->ai->camp_targ].owner = NULL;
				}
				self->ai->camp_targ = Closest_CMP_to_Ent (self, closest);


				if (self->client->resp.mos == FLAMER && self->client->pers.weapon && self->client->pers.weapon->position != LOC_PISTOL)
				{
					gitem_t	*it;
					int	index;

					it = FindNextPickup(self, LOC_PISTOL);
					index = ITEM_INDEX(it);
					if (self->client->pers.inventory[index])
						it->use (self, it);
				}



				if (self->ai->camp_targ >-1)
				{
					int goal_node;

					camp_spots[self->ai->camp_targ].owner = self;
					self->ai->start_camp_time = level.time;
					goal_node = AI_ClosestNodeToSpot(camp_spots[self->ai->camp_targ].origin, self, true);
					self->ai->state = BOT_STATE_MOVE;
					self->ai->tries = 0;
					AI_SetGoal(self,goal_node);
					return;
				}
		}
	}


/*  done elsewhere

	if (self->ai->objective)
	{
		VectorSet (ent_orig, (self->ai->objective->absmax[0] + self->ai->objective->absmin[0])/2,
			(self->ai->objective->absmax[1] + self->ai->objective->absmin[1])/2,
			(self->ai->objective->absmax[2] + self->ai->objective->absmin[2])/2);

		if (!strcmp(level.mapname,"dday4")&&
			self->enemy && self->enemy->obj_name &&
			!strcmp (self->enemy->obj_name, "Bridge"))
		{
			VectorSet (ent_orig, -211, -544, 352);
		}

		if (BOT_DMclass_CheckShot(self, ent_orig) &&
			objective_hittable(self, self->ai->objective, ent_orig))
		{
			self->enemy = self->ai->objective;
			self->movetarget = best;
			self->goalentity = best;
			return;
		}
		else
		{
			//self->enemy = NULL;
			//self->movetarget =NULL;
			//self->goalentity = NULL;
		}

	}
*/

	if (!self->enemy)
	{
		for (i=0 ; i<game.maxentities ; i++)
		{
			e = &g_edicts[i];

			if (!e->inuse)
				continue;
			if (!e->classnameb)
				continue;
			if (!(e->classnameb == FUNC_EXPLOSIVE || e->classnameb == SANDBAGS))// && e->obj_owner != self->client->resp.team_on->index))
				continue;
			if (e->style < 0)
				continue;

		//	if (e->classnameb== SANDBAGS)
			//	gi.dprintf("sdfljksdfjkl\n");

		//		VectorAdd (e->absmin, e->absmax, ent_orig);
			//VectorSet (ent_orig, (e->absmax[0] + e->absmin[0])/2,(e->absmax[1] + e->absmin[1])/2,(e->absmax[2] + e->absmin[2])/2);
			VectorCopy (e->obj_origin , ent_orig);
			//gi.dprintf("%s\n",vtos(e->obj_origin));

			if (!objective_hittable(self, e, ent_orig))
				continue;

			VectorSubtract (ent_orig, self->s.origin, dist);
			if (VectorLength (dist) < nearest_distance)
			{
				//gi.dprintf("%s  %s\n", vtos(self->s.origin), vtos (ent_orig));
				//gi.dprintf("%s ",e->classname);
			//gi.dprintf("%f ",VectorLength (dist));
				nearest_distance = VectorLength (dist);
				closest = e;
			}
		}
				//gi.dprintf("\n\n");

		if (closest && nearest_distance < 100)
		{
			//gi.dprintf("%s\n",vtos(ent_orig));
			self->enemy = closest;
			return;
		}
	}


	// look for a target (should make more efficent later)
	target = findradius(NULL, self->s.origin, AI_GOAL_SR_RADIUS);

	while(target)
	{
		if (target->classname == NULL)
			return;

		// Missile detection code
		if (target->classnameb == ROCKET || target->classnameb == HGRENADE)
		{
			//if player who shoot is a potential enemy
			if (!OnSameTeam (self,target->owner) && self->ai->status.playersWeights[target->owner->s.number-1])
			{
//				if (AIDevel.debugChased && bot_showcombat->value)
//					G_PrintMsg (AIDevel.chaseguy, PRINT_HIGH, "%s: ROCKET ALERT!\n", self->ai->pers.netname);

				self->enemy = target->owner;	// set who fired the rocket as enemy
				return;
			}
		}


		// next target
		target = findradius(target, self->s.origin, AI_GOAL_SR_RADIUS);
	}






	//jalfixme (what's goalentity doing here?)
	if (best_weight)
	{
		self->movetarget = best;
		self->goalentity = best;
//		if (AIDevel.debugChased && bot_showsrgoal->value && (self->goalentity != self->movetarget))
//			G_PrintMsg (AIDevel.chaseguy, PRINT_HIGH, "%s: selected a %s for SR goal.\n",self->ai->pers.netname, self->movetarget->classname);
	}
}

//===================
//  AI_CategorizePosition
//  Categorize waterlevel and groundentity/stepping
//===================
void AI_CategorizePosition (edict_t *ent)
{
	qboolean stepping = AI_IsStep(ent);

	ent->was_swim = ent->is_swim;
	ent->was_step = ent->is_step;

	ent->is_ladder = AI_IsLadder( ent->s.origin, ent->s.angles, ent->mins, ent->maxs, ent );

	M_CatagorizePosition(ent);
	if (ent->waterlevel > 2 || ent->waterlevel && !stepping) {
		ent->is_swim = true;
		ent->is_step = false;
		return;
	}

	ent->is_swim = false;
	ent->is_step = stepping;
}


void ShowSpot (vec3_t orig, qboolean blah)
{
		edict_t	*head;


			head = G_Spawn();
			head->s.skinnum = 0;
			VectorCopy (orig, head->s.origin);

			head->s.frame = 0;
			if (blah)
				gi.setmodel (head, "models/mapmodels/faf/plant.MD2");
			else
				gi.setmodel (head, "models/objects/debris2/tris.md2");
			VectorSet (head->mins, -16, -16, 0);
			VectorSet (head->maxs, 16, 16, 16);
			head->takedamage = DAMAGE_NO;
			head->solid = SOLID_NOT;
			head->s.effects = EF_GIB;
			head->s.sound = 0;
			head->flags |= FL_NO_KNOCKBACK;
			head->movetype = MOVETYPE_NONE;
			head->think = G_FreeEdict;
			head->nextthink = level.time + 2;
			gi.linkentity (head);


}



void ParseBotChat (char *text, edict_t *attacker)
{
	static char buf[10240], infobuf[10240];    // Knightmare- was unsigned
	char *p,	*pbuf;
	size_t		bufLen;		// Knightmare added

	p = text;
	pbuf = buf;
	*pbuf = 0;
	bufLen = pbuf - &buf[0];	// Knightmare added

	while (*p != 0)
	{
		if (((ptrdiff_t)pbuf - (ptrdiff_t)buf) > 150)
		{
			break;
		}
		if (*p == '#')
		{
			switch (*(p+1))
			{
			case 'K':
				Com_sprintf (infobuf, sizeof(infobuf),  "%s", attacker->client->pers.netname);
			//	infobuf = attacker->client->pers.netname;
			//	strncpy (pbuf, infobuf);
				Q_strncpyz (pbuf, sizeof(buf) - bufLen, infobuf);
				pbuf = SeekBufEnd(pbuf);
				bufLen = pbuf - &buf[0];	// Knightmare added
				p += 2;
				continue;
			}
		}
		*pbuf++ = *p++;
	}

	*pbuf = 0;

	strncpy (text, buf, 150);
	if (strlen(text) > 149)
		text[150] = 0; // in case it's 150
}


float infrontdegree (edict_t *self, edict_t *other);
edict_t *FindOverlap (edict_t *ent, edict_t *last_overlap);
//==========================================
// AI_Think
// think funtion for AIs
//==========================================
void AI_Think (edict_t *self)
{
	int camp_targ_save;
	char soundfile[50];

	edict_t *e;
	vec3_t	vdist;
	float	nearlength;
	float	checkdist;
	int i, nearest;

	float tdist;
	float closest = 5000;
	edict_t *teammatedodge = NULL;
	float dist;


	if ( !self->ai )	//jabot092(2)
		return;


	if (self->ai->chatdelay)
	{


		self->ai->chatdelay--;
		if (self->ai->chatdelay == 0)
		{
			edict_t		*entR = NULL;
			edict_t		*entG = NULL;
			char teamname[5];
			int j;

			if (self->client->resp.team_on)
			{
				Com_sprintf (teamname, sizeof(teamname), "%s ",self->client->resp.team_on->playermodel);
			}
			else
				Com_sprintf (teamname, sizeof(teamname), "");

			if (dedicated->value){
				safe_cprintf(NULL, PRINT_HIGH, "%s", teamname);
				safe_cprintf(NULL, PRINT_HIGH, "%s: ", self->client->pers.netname);
				safe_cprintf(NULL, PRINT_HIGH, "%s\n", self->ai->chat);
			}

			for (j = 1; j <= game.maxclients; j++)
			{
				entR=&g_edicts[j];
				if (!entR->inuse || !entR->client)
					continue;
				safe_cprintf(entR, PRINT_HIGH, "%s", teamname);
				safe_cprintf(entR, PRINT_HIGH, "\2%s: ", self->client->pers.netname);
				safe_cprintf(entR, PRINT_HIGH, "\2%s\n", self->ai->chat);
				stuffcmd(entR, "play misc/talk1.wav");
			}
		}



	}

//gi.dprintf("%s \n",self->client->pers.userinfo);

//	if (self->ai->teammatedodge)
//	gi.dprintf("%s %s \n", self->client->pers.netname, self->ai->teammatedodge->client->pers.netname);

//	gi.dprintf("%f %f\n",self->ai->actual_camp_start, level.time);


	//had to duck due to map obstacle, stand them back up
	if (self->ai->ducktime &&
		self->ai->ducktime < level.time - 4)
	{
		if (self->stanceflags != STANCE_STAND)
			if (self->oldstance == self->stanceflags)
				change_stance (self, STANCE_STAND);

		self->ai->ducktime = 0;
	}

//ShowCampTarg (self);


//	if (self->ai->objective)
//		gi.dprintf ("%s %s\n", self->ai->objective->obj_name, self->ai->objective->message);
//	if (self->ai->camp_targ)
//      gi.dprintf ("%s\n", vtos(camp_spots[self->ai->camp_targ].origin));
//	gi.dprintf ("%i\n\n", self->ai->state);




//prevent telefrag spawn
	if (self->solid == SOLID_TRIGGER)
	{
		edict_t *overlap;


		if ((overlap = FindOverlap(self, NULL)) == NULL)
		{
			self->solid = SOLID_BBOX;
			gi.linkentity(self);
		}
		else
		{
			do
			{
				if (overlap->solid == SOLID_BBOX)
				{
					overlap->solid = SOLID_TRIGGER;
					gi.linkentity(overlap);
				}
				overlap = FindOverlap(self, overlap);

			} while (overlap != NULL);
		}
	}





	if (self->ai->teammatedodgetime &&
		self->ai->teammatedodgetime< level.time - 1.5)
	{
		self->ai->teammatedodgetime = 0;
		self->ai->teammatedodge = NULL;
		self->ai->dodge_node = -1;
		self->ai->state = BOT_STATE_MOVE;

//		AI_PickLongRangeGoal(self);
	}

// don't bunch up with teammates
	if (level.framenum %30 == 1 &&
		//!self->ai->teammatedodge &&
		//self->ai->teammatedodgetime < level.time - 3 &&
		self->ai->state != BOT_STATE_CAMP)
	{

		for (i=0 ; i<game.maxclients ; i++)
		{
			e = g_edicts + 1 + i;


			if (!e->inuse)
				continue;
			if (!e->client)
				continue;
			if (e == self)
				continue;
			if (e->health < 1)
				continue;
			if (e->client->resp.team_on != self->client->resp.team_on)
				continue;
			if (!e->s.origin)
				continue;
		//	if (e->ai &&
		//		e->ai->state == BOT_STATE_CAMP)
		//		continue;
			if (e->ai && e->ai->teammatedodge && e->ai->teammatedodge == self)
				continue;
			if (infrontdegree(self,e)<.8)
				continue;


//			if (infront (self,e))// + infront (e,self) == 1)
//				continue;
//			if (!e->client->movement)
//				continue;

			VectorSubtract (e->s.origin, self->s.origin, vdist);

			tdist = VectorLength (vdist);
			if (tdist > 200)
				continue;

			if (infront (self,e) && infront (e,self) && e->client)
				Cmd_Wave_f (self, 1);


			if (tdist < closest)
			{
				closest = tdist;
				teammatedodge = e;

			}
		}


		if (teammatedodge)// != self->ai->teammatedodge)//already dodging this dude, ignore
		{
				//gi.dprintf("%f \n", closest);
			self->ai->teammatedodge = teammatedodge;
			self->ai->teammatedodgetime = level.time;
//
		}
	}


	/* do this differently
	//find the closest node that is more than x distance away from teammate
	if (self->ai->teammatedodge && self->ai->teammatedodge->s.origin
		&& self->ai->teammatedodgetime > level.time - 3)
	{
		nearest = -1;
		nearlength = 999999999;
		for (i=0; i < nav.num_nodes; i++)
		{
			VectorSubtract (self->ai->teammatedodge->s.origin, nodes[i].origin, vdist);//crashed here once, don't know why
			checkdist = VectorLength (vdist);

			if (!pointinfront(self, nodes[i].origin))
				continue;

			if (checkdist > 600 &&
				checkdist < nearlength )
			{
				nearlength = checkdist;
				nearest = i;
			}
		}
		if (nearest)
		{
			self->ai->dodge_node = nearest;
			self->ai->state = BOT_STATE_DODGE;
		//	self->ai->camp_targ = -1;
		//	self->ai->objective = NULL;
        }
	}
*/

// look for nades & tnt
	if (level.framenum%5 == 1)
	{
		for (i=0 ; i<game.maxentities ; i++)
		{
			e = &g_edicts[i];

			if (!e->inuse)
				continue;
			if (!e->classname)
				continue;
			if (!( (e->classnameb == HGRENADE && e->nextthink && level.time > e->nextthink -2) ||
				e->classnameb == BOTWARN ||
				e->classnameb == FIRE ||
				e->classnameb == TNT) )
				continue;

			if (e->classnameb == TNT &&
				e->delay > level.time + 5)
				continue;

			if (e->classnameb == FIRE &&
				e->enemy &&
				e->enemy == self)
				continue;


			VectorSubtract (e->s.origin, self->s.origin, vdist);

			//gi.dprintf("%f\n", VectorLength(vdist));

			dist = VectorLength (vdist);

			if (e->classnameb == BOTWARN &&
				dist > 500)
				continue;
			else if (dist > 800)
				continue;

			self->ai->nadedodge = e;
			self->ai->bloqued_timeout = level.time + 10.0;

		}
	}

	//find the closest node that is more than x distance away from nade;
	if (self->ai->nadedodge)
	{
		nearest = -1;
		nearlength = 999999999;
		for (i=0; i < nav.num_nodes; i++)
		{
			VectorSubtract (self->ai->nadedodge->s.origin, nodes[i].origin, vdist);
			checkdist = VectorLength (vdist);
			if (checkdist > 1000 &&
				checkdist < nearlength )
			{
				nearlength = checkdist;
				nearest = i;
			}
		}
		if (nearest)
		{
			self->ai->dodge_node = nearest;
			self->ai->state = BOT_STATE_DODGE;

			if (self->ai->camp_targ > -1)
				camp_spots[self->ai->camp_targ].owner = NULL;

			self->ai->camp_targ = -1;
			self->ai->objective = NULL;

			self->ai->teammatedodgetime = -1;
			self->ai->teammatedodge = NULL;
        }
	}

	//nade blew up
	if (self->ai->dodge_node > 0 &&
		(self->ai->nadedodge == NULL ||
		self->ai->nadedodge->inuse == false))
	{
		//gi.dprintf("xxx\n");
		self->ai->nadedodge = NULL;
		self->ai->dodge_node = -1;

		if (self->ai->camp_targ > -1)
			camp_spots[self->ai->camp_targ].owner = NULL;
		self->ai->camp_targ = -1;

		self->ai->state = BOT_STATE_WANDER;
		self->ai->objective = NULL;
		self->movetarget = NULL;
		self->goalentity = NULL;
		self->enemy = NULL;
		AI_PickLongRangeGoal(self);
		//AI_PickShortRangeGoal(self);
	}

//	if (self->ai->objective)
//		self->ai->state = BOT_STATE_MOVE;

//
//	if (self->ai->objective)
  //    gi.dprintf("%s\n", self->ai->objective->obj_name);
  //        gi.dprintf("%i\n", self->ai->state);

	//gi.dprintf("%i\n", self->ai->current_node);

/*	if (self->ai->current_node > -1 && self->ai->goal_node &&
			self->ai->state == BOT_STATE_WANDER)
	{
		AI_FollowPath (self);
	}*/

/*	if (level.framenum%30 == 1)
	{
		if (self->ai->current_node == -1)
		{
			gi.dprintf ("oiwrewe\n");
			self->ai->camp_targ = -2;
			AI_PickLongRangeGoal(self);
		}
	}*/

	//bot's objective was just destroyed or captured
	if ( self->ai->objective &&
		(!self->ai->objective->health ||
		self->ai->objective->obj_owner == self->client->resp.team_on->index) )
	{
		self->client->aim = false;

		self->ai->reached_obj_time = 0;
		//gi.dprintf("OBJ DESTROYED\n");

		if (self->ai->camp_targ > -1)
			camp_spots[self->ai->camp_targ].owner = NULL;
		self->ai->camp_targ = -2;

		self->ai->objective = NULL;
		self->movetarget = NULL;
		self->goalentity = NULL;
		self->enemy = NULL;
		AI_PickLongRangeGoal(self);
		//AI_PickShortRangeGoal(self);

	}

	//keep flamer moving between camp spots
	if ((self->client->resp.mos == ENGINEER || self->client->resp.mos == FLAMER || (self->client->pers.weapon && self->client->pers.weapon->position == LOC_KNIFE))
		&&
		self->ai->state == BOT_STATE_CAMP &&
		self->ai->actual_camp_start > 0)
	{
		if (self->ai->objective)
		{
			if (self->client->pers.weapon && self->client->pers.weapon->position != LOC_PISTOL)
			{
				gitem_t	*it;
				int	index;

				it = FindNextPickup(self, LOC_PISTOL);
				index = ITEM_INDEX(it);
				if (self->client->pers.inventory[index])
					it->use (self, it);
			}
		}
		else
		{
			camp_targ_save = self->ai->camp_targ;
			self->ai->camp_targ = -1;
			AI_PickLongRangeGoal(self);
			if (self->ai->camp_targ != camp_targ_save)
				camp_spots[camp_targ_save].owner = NULL;
			if (self->oldstance == self->stanceflags)
				change_stance(self, STANCE_STAND);
		}
	}

//	gi.dprintf ("time: %i   rt: %i   acs: %i   lft: %i\n", (int)level.time, (int)self->client->respawn_time, (int)self->ai->actual_camp_start, (int)self->client->last_fire_time);

	//bot is at a camp_spot, but not seeing any action, move him to a different, random camp spot
	else if (self->ai->state == BOT_STATE_CAMP &&
		self->client->respawn_time < level.time - 90 &&
		self->ai->actual_camp_start > 0 &&
		self->ai->actual_camp_start < level.time - 90 &&
		(self->client->last_fire_time < level.time - 90))
	{
//		gi.dprintf ("HELLO\n");
		camp_targ_save = self->ai->camp_targ;
		self->ai->camp_targ = -1;
		AI_PickLongRangeGoal(self);
		if (self->ai->camp_targ != camp_targ_save)
			camp_spots[camp_targ_save].owner = NULL;
		if (self->oldstance == self->stanceflags)
			change_stance(self, STANCE_STAND);
		self->client->aim = false;
	}

	//bot saw action at this camp spot recently,
	//maybe it's time to find a different, close by camp spot to keep
	//the enemy guessing
	else if (self->ai->state == BOT_STATE_CAMP &&
		self->ai->actual_camp_start > 0 &&
		self->ai->actual_camp_start < level.time - 5 &&
		self->client->last_fire_time > level.time - 10 //&&
		//self->client->last_fire_time < level.time - 3
		)
	{
			//gi.dprintf("CHANGING?\n");
		if (rand()%3 > 1)
		{
			//gi.dprintf("CHANGING\n");
			self->ai->actual_camp_start = -1;
			camp_targ_save = self->ai->camp_targ;
			self->ai->camp_targ = -2;
			AI_PickLongRangeGoal(self);
			if (self->ai->camp_targ != camp_targ_save)
				camp_spots[camp_targ_save].owner = NULL;

			self->ai->bloqued_timeout = level.time + 10.0;
			if (self->oldstance == self->stanceflags)
				change_stance(self, STANCE_STAND);
			self->client->aim = false;
		}
		else
			self->ai->actual_camp_start = level.time; //treat camp_spot like new and don't move
	}

//	if (!self->ai->current_node)
//		self->ai->current_node = AI_FindClosestReachableNode(self->s.origin, self,((1+self->ai->nearest_node_tries)*NODE_DENSITY),NODE_ALL);

	//was blocking door, now unblock
	if (self->health > 0 && self->solid == SOLID_NOT)
	{
		if (self->ai->door_block_time < level.time - 2)
		{
			self->solid = SOLID_TRIGGER;
		}
	}

	//SHOUTS
	if (!level.intermissiontime && self->health > 0 && level.last_bot_shout_time < level.time - 1)
	{
		if (self->ai->med_heal_time &&
			self->ai->med_heal_time < level.time -1.25)
		{
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("%s/shout/thanks1.wav",self->client->resp.team_on->teamid)), 1, ATTN_NORM, 0);
			self->ai->med_heal_time = 0;
			Cmd_Wave_f (self, 1);
		}
		else if (self->client->respawn_time > level.time - 3)
		{
			if ((int)rand()%50 == 1)
			{
				if ((int)rand()%2 == 1)
				{
					Q_strncpyz (soundfile, sizeof(soundfile), va("%s/shout/move%i.wav", self->client->resp.team_on->teamid,1+(int)rand()%2));
					Cmd_Wave_f (self, 4);

				}
				else
				{
					Q_strncpyz (soundfile, sizeof(soundfile), va("%s/shout/attack%i.wav", self->client->resp.team_on->teamid,1+(int)rand()%2));
					Cmd_Wave_f (self, 3);

				}
				gi.sound (self, CHAN_VOICE, gi.soundindex(soundfile), 1, ATTN_NORM, 0);
				level.last_bot_shout_time = level.time;

				//sndfixcheck
				//gi.dprintf("AI_Think respawn_time %s\n",  soundfile);


			}
		}
		if (self->ai->tktime > level.time - 1)
		{
			if (rand()%3 == 1)
			{
				//if ((int)rand()%2 == 1)
					Q_strncpyz (soundfile, sizeof(soundfile), va("%s/shout/cease%i.wav", self->client->resp.team_on->teamid,1+(int)rand()%2));
				gi.sound (self, CHAN_VOICE, gi.soundindex(soundfile), 1, ATTN_NORM, 0);
				level.last_bot_shout_time = level.time;
				Cmd_Wave_f (self, 2);

								//sndfixcheck
				//gi.dprintf("AI_Think cease %s\n",  soundfile);


			}
			self->ai->tktime = -1;
		}
		else if (self->health < 100  && level.framenum%100 == 1 && (int)rand()%5 == 1 && self->client && self->client->resp.mos && self->client->resp.mos != MEDIC)
		{
		//	gi.dprintf("MEDIC\n");
			Q_strncpyz (soundfile, sizeof(soundfile), va("%s/shout/medic.wav", self->client->resp.team_on->teamid));
			gi.sound (self, CHAN_VOICE, gi.soundindex(soundfile), 1, ATTN_NORM, 0);
			level.last_bot_shout_time = level.time;
			Cmd_Wave_f (self, 0);

			//sndfixcheck
				//gi.dprintf("AI_Think medic %s\n",  soundfile);


		}
		else if (self->ai->sniperspot < level.time - 1 &&
			self->ai->sniperspot > level.time - 2)
		{
			if (rand()%4 == 1)
			{
				char soundfile2[50];

				Q_strncpyz (soundfile2, sizeof(soundfile2), va("%s/shout/sniper%i.wav", self->client->resp.team_on->teamid,1+(int)rand()%2));
				gi.sound (self, CHAN_VOICE, gi.soundindex(soundfile2), 1, ATTN_NORM, 0);
				level.last_bot_shout_time = level.time;
				Cmd_Wave_f (self, 4);
			//	gi.dprintf("SNIPER\n");
				self->ai->sniperspot = -1;
							//sndfixcheck
			//	gi.dprintf("AI_Think sniper %s\n",  soundfile2);
			}
			else
				self->ai->sniperspot = -1;
		}
		if (level.framenum % 600 == 300 && rand() % 10 == 1)
		{
				if ((int)rand()%4 > 1)
					Q_strncpyz (soundfile, sizeof(soundfile), va("%s/shout/funny%i.wav", self->client->resp.team_on->teamid,1+(int)rand()%3));
				else
					Q_strncpyz (soundfile, sizeof(soundfile), va("%s/shout/smoke1.wav", self->client->resp.team_on->teamid));
				gi.sound (self, CHAN_VOICE, gi.soundindex(soundfile), 1, ATTN_NORM, 0);

				// sndfixcheck
			//	gi.dprintf("AI_Think funny %s\n",  soundfile);

				level.last_bot_shout_time = level.time;
				Cmd_Wave_f (self, 0);
		}
	}

	AIDebug_SetChased(self);	//jal:debug shit
	AI_CategorizePosition(self);

	//freeze AI when dead
	if ( self->deadflag ) {
		self->ai->pers.deadFrame(self);
		return;
	}

	//if we're close to our target camping spot, then camp there.
	if (self->ai->state != BOT_STATE_CAMP && self->ai->camp_targ > -1)
	{
		if (AI_FindClosestReachableNode( self->s.origin, NULL, 48, NODE_ALL ) == AI_ClosestNodeToSpotx(camp_spots[self->ai->camp_targ].origin))
		//if (AI_FindClosestReachableNode( self->s.origin, self, 48, NODE_ALL ) == AI_FindClosestReachableNode( camp_spots[self->ai->camp_targ].origin, self, 48, NODE_ALL ))
		{
			if (self->ai->objective && !self->ai->defend_bot)
			{
				vec3_t obj_orig;

				self->ai->actual_camp_start = level.time;
				self->ai->state = BOT_STATE_CAMP;

				if (!self->ai->reached_obj_time)
					self->ai->reached_obj_time = level.time;
				else if (VectorCompare (self->ai->objective->obj_origin, vec3_origin))
					VectorCopy (self->ai->objective->obj_origin, obj_orig);
				else
					VectorSet (obj_orig, (self->ai->objective->absmax[0] + self->ai->objective->absmin[0])/2,
					(self->ai->objective->absmax[1] + self->ai->objective->absmin[1])/2,
					(self->ai->objective->absmax[2] + self->ai->objective->absmin[2])/2);

               if (objective_hittable(self, self->ai->objective, obj_orig))
				{
					//gi.dprintf ("SHOOT OBJECTIVE!\n");
					self->enemy = self->ai->objective;
				}
			   else if (self->ai->reached_obj_time > 0 && self->ai->reached_obj_time < level.time - 5)
				{
					//gi.dprintf ("SWITCHING OBJECTIVES!\n");
					camp_targ_save = self->ai->camp_targ;
					self->ai->camp_targ = -1;
					AI_PickLongRangeGoal(self);
					if (self->ai->camp_targ != camp_targ_save)
						camp_spots[camp_targ_save].owner = NULL;

				}
			}
			else
			{
				self->ai->actual_camp_start = level.time;
				self->ai->state = BOT_STATE_CAMP;
				//if (self->stanceflags != camp_spots[self->ai->camp_targ].stance)
				//	change_stance(self,camp_spots[self->ai->camp_targ].stance);
			}

		}
	}

/*
	//been moving towards camp spot for a while and can't find/get to it
	if (//!self->ai->objective &&
		self->ai->camp_targ > -1 &&
		self->ai->state == BOT_STATE_CAMP &&
		self->client->last_fire_time < level.time - 20 &&
		self->ai->start_camp_time > 0 && self->ai->start_camp_time < level.time - 30)
	{
		camp_targ_save = self->ai->camp_targ;
		self->ai->camp_targ = -1;
		AI_PickLongRangeGoal(self);
		if (self->ai->camp_targ != camp_targ_save)
			camp_spots[camp_targ_save].owner = NULL;

	}

*/


	if (self->client->limbo_mode)
		self->ai->bloqued_timeout = level.time + 10.0;


	if (self->client->resp.AlreadySpawned &&
		!level.intermissiontime &&
		(self->ai->state != BOT_STATE_CAMP &&
		!(self->client->resp.mos == H_GUNNER &&	self->stanceflags != STANCE_STAND))  ||
		(self->client->respawn_time < level.time - 60 &&
		self->client->last_fire_time < level.time - 60 &&
		self->ai->actual_camp_start < level.time - 60)
		)
	{
		//if completely stuck somewhere
		if (VectorLength(self->velocity) > 37)
			self->ai->bloqued_timeout = level.time + 10.0;

		if ( self->ai->bloqued_timeout < level.time && self->client->respawn_time < level.time - 5) {
			self->ai->pers.bloquedTimeout(self);
			return;
		}
	}


	//update status information to feed up ai
	self->ai->pers.UpdateStatus(self);

	//update position in path, set up move vector
	if ( self->ai->state == BOT_STATE_MOVE ) {

		if ( !AI_FollowPath(self) )
		{


			AI_SetUpMoveWander( self );
			self->ai->wander_timeout = level.time - 1;	//do it now
		}
	}

	//check if something's caused em to go wayward (fall off ramp/ladder etc.) and reset their path to goal if so
	if ( self->ai->state == BOT_STATE_MOVE &&
		self->ai->next_node )
	{
		vec3_t v;
		float	dist;

		VectorSubtract (self->s.origin, nodes[self->ai->next_node].origin, v);
		dist = VectorLength (v);

		if (self->ai->current_node == self->ai->checknode &&
			self->ai->last_checknode_distance)
		{
			//gi.dprintf("%f %f %i %i\n", self->ai->last_checknode_distance, dist, self->ai->current_node, self->ai->checknode);
			if (self->ai->last_checknode_distance /*+ 50*/ < dist )
			{
			//	gi.dprintf("going wrong way!");
			//	AI_SetGoal(self,self->ai->goal_node);

			}

		}
			self->ai->checknode = self->ai->current_node;
			self->ai->last_checknode_distance = dist;
	}

	if (self->ai->unduck_stance && self->ai->ducktime < level.time -3)
	{
		if (self->oldstance == self->stanceflags && self->ai->unduck_stance != self->stanceflags)
				change_stance(self,self->ai->unduck_stance);

		self->ai->unduck_stance = 0;
	}
	else if (self->client->aim && self->client->p_rnd && *self->client->p_rnd == 0 && self->ai->last_reload_try < level.time - 5)
	{
		vec3_t	crawlorig,enemyview;

		self->client->buttons &= ~BUTTON_ATTACK;
		Cmd_Reload_f (self);
		self->ai->last_reload_try = level.time;

		//gi.dprintf("RELOAD %f\n",level.time);
		VectorCopy (self->s.origin, crawlorig);
		crawlorig[2]-= 17;

		//duck to reload
		if (self->ai->last_enemy && self->ai->last_enemy->client)
		{
	 		VectorCopy (self->ai->last_enemy->s.origin, enemyview);
			enemyview[2]+= self->ai->last_enemy->viewheight;
			if (!AI_VisibleOrigins (crawlorig, enemyview))
			{
				if (self->oldstance == self->stanceflags)
				{
					self->ai->unduck_stance = self->stanceflags;
					change_stance (self,STANCE_CRAWL);
					self->ai->ducktime = level.time;
					//gi.dprintf("duck!\n");
				}
			}

		}

	}

	//bolt sniper whenever needed
	if (!self->client->newweapon && self->client->resp.team_on && self->client->pers.weapon && self->client->pers.weapon->position == LOC_SNIPER &&
		self->client->sniper_loaded[self->client->resp.team_on->index] == false)
	{
		Cmd_Scope_f(self);
	}

	if (!self->enemy && self->ai->last_enemy_time < level.time -5 && self->ai->last_reload_try < level.time - 5)
	{
		if (self->client->pers.weapon && self->client->p_rnd && *self->client->p_rnd)	{
			if (self->client->pers.weapon->position == LOC_SUBMACHINEGUN ||
				self->client->pers.weapon->position == LOC_SUBMACHINEGUN2 ||
				self->client->pers.weapon->position == LOC_L_MACHINEGUN){
				if (*self->client->p_rnd < 8)		{
					Cmd_Reload_f (self);
					self->ai->last_reload_try = level.time;
				}
			}
			else if (self->client->pers.weapon->position == LOC_RIFLE){
				if (*self->client->p_rnd < 3){
					Cmd_Reload_f (self);
					self->ai->last_reload_try = level.time;
				}
			}
			else if (self->client->pers.weapon->position == LOC_H_MACHINEGUN){
				if (*self->client->p_rnd < 10){
					Cmd_Reload_f (self);
					self->ai->last_reload_try = level.time;
				}
			}
		}
	}

	//pick a new long range goal
	if ( self->ai->state == BOT_STATE_WANDER && self->ai->wander_timeout < level.time)
	{
		if (self->ai->camp_targ > -1)
			camp_spots[self->ai->camp_targ].owner = NULL;

		self->ai->camp_targ = -1;
		AI_PickLongRangeGoal(self);
	}

	//Find any short range goal
	AI_PickShortRangeGoal(self);

	//run class based states machine
	self->ai->pers.RunFrame(self);

	//if (self->ai->state == BOT_STATE_WANDER && !self->enemy && self->ai->last_enemy_time < level.time - 1)
	if (self->ai->state == BOT_STATE_CAMP)
	{
		if (self->stanceflags != camp_spots[self->ai->camp_targ].stance)
		{
			if (self->oldstance == self->stanceflags)
				change_stance(self, camp_spots[self->ai->camp_targ].stance);
		}
	}
	else if (!self->enemy && self->ai->last_enemy_time < level.time - 2)
	{
//		self->client->aim = false;
		if (self->stanceflags != STANCE_STAND)
			if (self->oldstance == self->stanceflags)
				change_stance (self, STANCE_STAND);
	}
	else
	{
	/*	if (self->client->resp.mos == H_GUNNER)
		{
			if (self->stanceflags != STANCE_DUCK)
				change_stance (self, STANCE_DUCK);
		}*/
	}

//	self->client->reloadtry = true;

	if (self->client->weaponstate != WEAPON_RELOADING)
	{

			//if (self->ai->state != BOT_STATE_CAMP && self->stanceflags == STANCE_STAND && self->client->resp.mos == H_GUNNER &&
				//self->client->last_fire_time < level.time - .5)
				//change_stance(self, STANCE_DUCK);


			if (self->client->aim && self->ai->state != BOT_STATE_CAMP &&    // && self->ai->aimtime < level.time - 4 && self->client->last_fire_time < level.time - 4)
				 !self->enemy && self->ai->last_enemy_time < level.time - 2)
			{
				if ((self->client->resp.mos == H_GUNNER || self->client->resp.mos == ENGINEER) && self->ai->state != BOT_STATE_CAMP && self->stanceflags != STANCE_STAND)
				{
					if (self->oldstance == self->stanceflags)
						change_stance(self, STANCE_STAND);
				}

				self->client->aim		 = false;
			}

	}
	//if (self->ai->path.goalNode)
	//gi.dprintf("%s\n",vtos(nodes[self->ai->path.goalNode].origin));

//	if (self->enemy && self->enemy->obj_name)
//		gi.dprintf("%s\n",self->enemy->obj_name);
//	else if (self->enemy)
//		gi.dprintf("%s\n",self->enemy->classname);
//	else
//		gi.dprintf("xx\n");
}


float infrontdegree (edict_t *self, edict_t *other)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;

	AngleVectors (self->s.angles, forward, NULL, NULL);
	VectorSubtract (other->s.origin, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);

	return dot;
}


qboolean pointinfront (edict_t *self, vec3_t point)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;

	AngleVectors (self->s.angles, forward, NULL, NULL);
	VectorSubtract (point, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);

	if (dot > 0.3)
		return true;

	return false;
}


qboolean toright (edict_t *self, vec3_t point)
{
	vec3_t	vec;
	float	dot;
	vec3_t	right;

	AngleVectors (self->s.angles, NULL, right, NULL);
	VectorSubtract (point, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, right);

	if (dot > 0)
		return true;

	return false;
}
