/*
===========================================================================
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2000-2002 Mr. Hyde and Mad Dog

This file is part of Lazarus Quake 2 Mod source code.

Lazarus Quake 2 Mod source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Lazarus Quake 2 Mod source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lazarus Quake 2 Mod source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "g_local.h"

/*
===============================================

  HINT_PATH CODE

===============================================
*/

#define HINTPATH_ENDPT		1

int	hint_chains_exist;
int	hint_chain_count;
edict_t *hint_chain_starts[MAX_HINT_CHAINS];

/*
=====================
SetupHintPaths

Called from SpawnEntities in g_spawn.c.
Initializes all hint_path chains in a map.
=====================
*/
void SetupHintPaths (void)
{
	int		i, keyofs;
	edict_t	*thisPath, *ent;

	// check if there are any hint_paths in this map first
	hint_chains_exist = 0;
	keyofs = FOFS(classname);
	ent = G_Find(NULL, keyofs, "hint_path");
	if (!ent) return; // get outta here

	hint_chains_exist = 1;
	hint_chain_count = 0;
	memset (hint_chain_starts, 0, MAX_HINT_CHAINS*sizeof (edict_t *));
	while (ent)
	{
		if ((ent->spawnflags & HINTPATH_ENDPT) && ent->target) // start point
		{
			if (!ent->targetname) // must not be targeted
			{
				if (hint_chain_count >= MAX_HINT_CHAINS) {
					gi.dprintf ("SetupHintPaths: Limit of %d hint_path chains reached.  Merge some.\n", MAX_HINT_CHAINS);
					break;
				}
				hint_chain_starts[hint_chain_count++] = ent;
			}
			else // Node in middle of chain has end flag set, ignore.
				gi.dprintf ("SetupHintPaths: Hint_path node at %s has endpoint flag set with both target (%s) and targetname fields set (%s).\n",
					vtos (ent->s.origin), ent->target, ent->targetname);
		}
		ent = G_Find(ent, keyofs, "hint_path");
	}

	keyofs = FOFS(targetname);
	for (i=0; i< hint_chain_count; i++)
	{
		thisPath = hint_chain_starts[i];
		thisPath->hint_chain_id = i;
		ent = G_Find(NULL, keyofs, thisPath->target); // find next node
		if (G_Find(ent, keyofs, thisPath->target)) // checked for branched chain
		{
			gi.dprintf ("SetupHintPaths: Branched hint_path node found in chain %i at %s, double target: %s\n", 
				i, vtos (thisPath->s.origin), thisPath->target);
			hint_chain_starts[i]->hint_chain = NULL;
			continue;
		}
		while (ent)
		{	
			if (!ent->hint_chain) // make sure we haven't looped back
			{
				thisPath->hint_chain = ent;
				thisPath = ent;
				thisPath->hint_chain_id = i;
				// If we've reached the end of the chain, get outta here!
				if (!thisPath->target) break; 
				ent = G_Find(NULL, keyofs, thisPath->target); // find next node
				if (G_Find(ent, keyofs, thisPath->target)) // checked for branched chain
				{
					gi.dprintf ("SetupHintPaths: Branched hint_path node found in chain %i at %s, double target: %s\n", 
						i, vtos (thisPath->s.origin), thisPath->target);
					hint_chain_starts[i]->hint_chain = NULL;
					break;
				}
			}
			else // we've encountered a circular chain and have come back to the start
			{
				gi.dprintf ("\nCircular hint_path  found in chain %i at %s, targetname: %s\n", 
					i, vtos (ent->s.origin), ent->targetname);
				hint_chain_starts[i]->hint_chain = NULL;
				break;

			}
		}
	}
}

/*
=====================
hintpath_start

Makes a monster go towards a hintpath spot,
and clears inhibiting AI flags.
=====================
*/
void hintpath_start (edict_t *monster, edict_t *spot)
{
	vec3_t	goDir, goAngles;

	VectorSubtract(spot->s.origin, monster->s.origin, goDir);
	vectoangles2 (goDir, goAngles);

	monster->monsterinfo.aiflags &= ~(AI_SOUND_TARGET | AI_PURSUIT_LAST_SEEN | AI_PURSUE_NEXT | AI_PURSUE_TEMP);
	monster->monsterinfo.aiflags |= AI_HINT_PATH;
	monster->monsterinfo.pausetime = 0;
	monster->ideal_yaw = goAngles[YAW];
	monster->movetarget = monster->goalentity = spot;
	// get moovin!
	monster->monsterinfo.search_time = level.time;
	monster->monsterinfo.run (monster);
}

/*
=====================
hintpath_stop

Makes a monster stop following hint_paths.
=====================
*/
void hintpath_stop (edict_t *monster)
{
	monster->movetarget = monster->goalentity = NULL;
	monster->monsterinfo.last_hint_time = level.time;
	monster->monsterinfo.goal_hint = NULL;
	monster->monsterinfo.aiflags &= ~AI_HINT_PATH;

	// If we don't have an enemy to get mad at, just stand around like an unactivated monster.
	if (!has_valid_enemy(monster))
	{
		monster->enemy = NULL;
		monster->monsterinfo.pausetime = level.time + 100000000;
		monster->monsterinfo.stand (monster);
	}
	else if (visible(monster, monster->enemy)) 	// attack if we can see our foe
		FoundTarget (monster);
	else // keep pursuing
		HuntTarget (monster);
}

/*
=====================
hintcheck_monsterlost

Has a monster look for a valid hintpath that will lead to its enemy.
One endpoint of a valid path must be visible to the monster, and the other to the monster's enemy.
=====================
*/
#define HINT_NODE_RANGE 512
qboolean hintcheck_monsterlost (edict_t *monster)
{
	edict_t		*ent;
	edict_t		*monster_node_list, *target_node_list, *prev_node;
	edict_t		*closest_node, *start_node, *dest_node;
	int			i;
	float		node_dist, closest_dist;
	qboolean	monster_pathchains_listed[MAX_HINT_CHAINS], target_pathchains_listed[MAX_HINT_CHAINS];
	qboolean	nodes_found = false;

	// If monster is standing at post, has no enemy,
	// or there are no hint_path chains in this map, get outta here.
	if ((monster->monsterinfo.aiflags & AI_STAND_GROUND)
		|| !monster->enemy || !hint_chains_exist)
		return false;

	monster_node_list = NULL;
	// Create linked list of all hint_path nodes in level
	for (i=0; i < hint_chain_count; i++)
	{
		ent = hint_chain_starts[i];
		while (ent)
		{	// Clean up previous monster_hint_chain pointers
			if (ent->monster_hint_chain)
				ent->monster_hint_chain = NULL;
			if (!monster_node_list)  // add first node
			{
				monster_node_list = ent;
				prev_node = ent;
			}
			else
			{
				prev_node->monster_hint_chain = ent;
				prev_node = ent;
			}
			ent = ent->hint_chain;
		}
	}
	
	// Remove inaccessible to monster nodes from monster_node_list linked list.
	ent = monster_node_list;
	prev_node = NULL;
	while (ent)
	{
		node_dist = realrange (monster, ent);
		if (node_dist > HINT_NODE_RANGE || !visible(monster, ent))
		{
			if (!prev_node)
			{
				edict_t *temp = ent;
				ent = ent->monster_hint_chain;
				temp->monster_hint_chain = NULL;
				// Since there is no previous valid node, move start pointer up to our new position.
				monster_node_list = ent;
				continue;
			}
			else
			{
				prev_node->monster_hint_chain = ent->monster_hint_chain;
				ent->monster_hint_chain = NULL;
				ent = prev_node->monster_hint_chain;
				continue;
			}
		}
		nodes_found = true;
		prev_node = ent;
		ent = ent->monster_hint_chain;
	}

	// If no hint_path nodes are accessible to the monster, get outta here.
	if (!nodes_found) return false;

/*
	We now have a linked list of all hint_path nodes accessible to the monster.

	The next step is to create a list of pathchains that the monster can access.
*/

	// Go through all monster- accessible nodes to see which pathchains have nodes in the linked list.
	for (i=0; i < hint_chain_count; i++)
		monster_pathchains_listed[i] = false;
	ent = monster_node_list;
	while (ent)
	{	// catch errors
		if ((ent->hint_chain_id < 0) || (ent->hint_chain_id > hint_chain_count))
			return false;
		monster_pathchains_listed[ent->hint_chain_id] = true;
		ent = ent->monster_hint_chain;
	}

	// Build a linked list of all nodes in the pathchains accessible to the monster.
	// This will be used to find nodes that are accessible to the monster's enemy.
	target_node_list = NULL;
	prev_node = NULL;
	for (i=0; i < hint_chain_count; i++)
	{
		if (monster_pathchains_listed[i]) // If pathchain is listed, add it to our new linked list.
		{
			ent = hint_chain_starts[i];
			while (ent)
			{
				if (!target_node_list) // add first node
				{
					target_node_list = ent;
					prev_node = ent;
				}
				else
				{
					prev_node->target_hint_chain = ent;
					prev_node = ent;
				}
				ent = ent->hint_chain;
			}
		}
	}

	// Remove inaccessible to monster's enemy nodes from target_node_list linked list.
	ent = target_node_list;
	prev_node = NULL;
	nodes_found = false;
	while (ent)
	{
		node_dist = realrange (monster->enemy, ent);
		if (node_dist > HINT_NODE_RANGE || !visible(monster->enemy, ent))
		{
			if (!prev_node)
			{
				edict_t *temp = ent;
				ent = ent->target_hint_chain;
				temp->target_hint_chain = NULL;
				// Since there is no previous valid node, move start pointer up to our new position.
				target_node_list = ent;
				continue;
			}
			else
			{
				prev_node->target_hint_chain = ent->target_hint_chain;
				ent->target_hint_chain = NULL;
				ent = prev_node->target_hint_chain;
				continue;
			}
		}
		nodes_found = true;
		prev_node = ent;
		ent = ent->target_hint_chain;
	}
	
	// If no hint_path nodes would bring us to our enemy, get outta here.
	if (!nodes_found) return false;

/*
	We now have two linked lists- one of hint_path nodes accessible to the monster, and one of hint_path nodes
	near the monster's enemy that are accessible from nodes near the monster.
	
	The next step is to find the closest node near the monster that will lead it to its enemy,
*/

	// Go through all monster's enemy-accessible nodes to see which pathchains have nodes in the linked list.
	for (i=0; i < hint_chain_count; i++)
		target_pathchains_listed[i] = false;
	ent = target_node_list;
	while (ent)
	{	// catch errors
		if ((ent->hint_chain_id < 0) || (ent->hint_chain_id > hint_chain_count))
			return false;
		target_pathchains_listed[ent->hint_chain_id] = true;
		ent = ent->target_hint_chain;
	}
	
	// Go through monster_node_list linked list to find the closest node to us
	// that leads to the monster's enemy (on the list).
	closest_dist = HINT_NODE_RANGE;
	closest_node = NULL;
	ent = monster_node_list;
	while (ent)
	{
		if (target_pathchains_listed[ent->hint_chain_id])
		{
			node_dist = realrange(monster, ent);
			if (node_dist < closest_dist)
			{
				closest_node = ent;
				closest_dist = node_dist;
			}
		}
		ent = ent->monster_hint_chain;
	}

	// If there are no nodes close enough that take us to our enemy, get outta here.
	if (!closest_node) return false;
	start_node = closest_node;

	// Finally we go through target_node_list linked list to find the node closest to
	// our target that is on the pathchain the monster will be using.
	closest_dist = HINT_NODE_RANGE;
	closest_node = NULL;
	ent = target_node_list;
	while (ent)
	{
		if (ent->hint_chain_id == start_node->hint_chain_id)
		{
			node_dist = realrange(monster->enemy, ent);
			if (node_dist < closest_dist)
			{
				closest_node = ent;
				closest_dist = node_dist;
			}
		}
		ent = ent->target_hint_chain;
	}

	// If there is no node close enough to our enemy, get outta here.
	if (!closest_node) return false;
	dest_node = closest_node;

	monster->monsterinfo.goal_hint = dest_node;
	hintpath_start (monster, start_node);
	return true;
}


/*
=====================
touch_hint_path

A monster has touched a hint_path node
=====================
*/
void touch_hint_path (edict_t *hintpath, edict_t *monster, cplane_t *plane, csurface_t *surf)
{
	qboolean	parsedGoal = false;
	edict_t		*ent, *goalPath, *nextPath;

	if (monster->monsterinfo.aiflags & AI_MEDIC_PATROL) 
	{
		if (monster->movetarget == hintpath)
			medic_NextPatrolPoint(monster, hintpath);
		return;
	}

	if (monster->monsterinfo.aiflags & AI_HINT_TEST)
	{
		if (monster->movetarget == hintpath)
			HintTestNext(monster, hintpath);
		return;
	}

	// check that this hint path is monster's movetarget
	if (hintpath != monster->movetarget) return;

	goalPath = monster->monsterinfo.goal_hint;
		
	if (hintpath == goalPath) // stop if monster has reached destination
	{
		hintpath_stop (monster);
		return;
	}

	// get next hint_path for monster to travel to
	ent = hint_chain_starts[hintpath->hint_chain_id];
	while (ent)
	{
		// If we encounter this node first (before destination),
		// then send monster to the next node on the chain.
		if (hintpath == ent)
		{
			nextPath = ent->hint_chain;
			break;
		}

		// If we encounter the destination node first, raise flag.
		if (goalPath == ent) parsedGoal = true;

		// If we found the destination node before the current node,
		// then we are going in the opposite direction of the chain.
		// So send the monster to the node preceeding this one.
		if (parsedGoal && (hintpath == ent->hint_chain))
		{
			nextPath = ent;
			break;
		}
		ent = ent->hint_chain;
	}

	// If for some reason we couldn't find the next node, get outta here.
	if(!nextPath)
	{
		hintpath_stop(monster);
		return;
	}

	// Otherwise, proceed to the next hint_path.
	hintpath_start (monster, nextPath);

	// If this hint_path has a wait value set, pause here for set time.
	if (hintpath->wait)
		monster->nextthink = level.time + hintpath->wait;
}


/*QUAKED hint_path (.5 .3 0) (-8 -8 -8) (8 8 8) ENDPT
ENDPT - set this for nodes at the start and end of each string

"target"		: next hint path
"targetname"	: name of this hint_path
"wait"			: delay for monster at this point
*/
void SP_hint_path (edict_t *hintpath)
{
	// singleplayer-only
	if (deathmatch->value) {
		G_FreeEdict (hintpath); return;
	}
	// check if not connected
	if (!hintpath->targetname && !hintpath->target) {
		gi.dprintf ("unconnected hint_path at %s\n", vtos(hintpath->s.origin));
		G_FreeEdict (hintpath); return;
	}

	// Lazarus: Corrections for mappers that can't follow instructions :-)
	if (!hintpath->targetname || !hintpath->target)
		hintpath->spawnflags |= HINTPATH_ENDPT;

	VectorSet (hintpath->mins, -8, -8, -8);
	VectorSet (hintpath->maxs, 8, 8, 8);
	hintpath->solid = SOLID_TRIGGER;
	hintpath->svflags |= SVF_NOCLIENT;
	hintpath->touch = touch_hint_path;
	gi.linkentity (hintpath);
}


/*
===============================

Blocked Logic

===============================
*/

// func_plat states
#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3

qboolean face_wall (edict_t *self);
void HuntTarget (edict_t *self);
qboolean parasite_drain_attack_ok (vec3_t start, vec3_t end);

/*
=====================
check_shot_blocked

chance_attack: 0-1, probability that monster will attack if it has a clear shot
=====================
*/
qboolean check_shot_blocked (edict_t *monster, float chance_attack)
{
	// only check for players, and random check
	if (!monster->enemy || !monster->enemy->client || (random() < chance_attack))
		return false;

	// special case for parasite
	if (strcmp(monster->classname, "monster_parasite") == 0)
	{
		trace_t	trace;
		vec3_t	forward, right, start, end, probeOffset;
		int		i;

		VectorSet (probeOffset, 24, 0, 6);
		VectorCopy (monster->enemy->s.origin, end);
		AngleVectors (monster->s.angles, forward, right, NULL);
		G_ProjectSource (monster->s.origin, probeOffset, forward, right, start);
		for (i=0; i<3; i++)
		{
			switch(i) {
			case 0: break;
			case 1:
				end[2] = monster->enemy->s.origin[2] + monster->enemy->maxs[2] - 8; break;
			case 2:
				end[2] = monster->enemy->s.origin[2] + monster->enemy->mins[2] + 8; break;
			}
			if (parasite_drain_attack_ok(start, end))
				break;
			if (i == 2)
				return false;
		}
		VectorCopy (monster->enemy->s.origin, end);

		trace = gi.trace (start, NULL, NULL, end, monster, MASK_SHOT);
		if (trace.ent != monster->enemy) {
			monster->monsterinfo.aiflags |= AI_BLOCKED;
			if (monster->monsterinfo.attack) monster->monsterinfo.attack(monster);
			monster->monsterinfo.aiflags &= ~AI_BLOCKED;
			return true;
		}
	}
	return false;
}

/*
=====================
check_plat_blocked

moveDist: how far monster is trying to move
=====================
*/
qboolean check_plat_blocked (edict_t *monster, float moveDist)
{ 
	edict_t		*platform = NULL;
	trace_t		stepTrace;
	int			enemy_relHeight;
	vec3_t		point1, point2, forward;

	if (!monster->enemy) return false;

	// check our enemy's comparative elevation
	if (monster->enemy->absmax[2] <= monster->absmin[2])
		enemy_relHeight = -1;
	else if (monster->enemy->absmin[2] >= monster->absmax[2])
		enemy_relHeight = 1;
	else
		enemy_relHeight = 0;

	// if monster is close to its enemy in elevation, don't bother with using a lift
	if (!enemy_relHeight) return false;

	// check if monster is already on a plat
	if(monster->groundentity && monster->groundentity != world
		&& strcmp(monster->groundentity->classname, "func_plat") == 0)
		platform = monster->groundentity;

	// if monster isn't, check to see if it will step onto one with this move
	if (!platform)
	{
		AngleVectors (monster->s.angles, forward, NULL, NULL);
		VectorMA(monster->s.origin, moveDist, forward, point1);
		VectorCopy (point1, point2);
		point2[2] -= 384;

		stepTrace = gi.trace(point1, vec3_origin, vec3_origin, point2, monster, MASK_MONSTERSOLID);
		if (stepTrace.fraction < 1 && !stepTrace.startsolid && !stepTrace.allsolid
			&& strcmp(stepTrace.ent->classname, "func_plat") == 0)
			platform = stepTrace.ent;
	}

	// if monster has found a lift, activate it
	if (platform && platform->use)
	{
		if (enemy_relHeight == -1)
		{
			if ((monster->groundentity == platform && platform->moveinfo.state == STATE_TOP) ||
				(monster->groundentity != platform && platform->moveinfo.state == STATE_BOTTOM))
			{
				platform->use (platform, monster, monster);
				return true;
			}
		}
		else if (enemy_relHeight == 1)
		{
			if ((monster->groundentity == platform && platform->moveinfo.state == STATE_BOTTOM) ||
				(monster->groundentity != platform && platform->moveinfo.state == STATE_TOP))
			{
				platform->use (platform, monster, monster);
				return true;			
			}
		}
	}
	return false;
}


/*
=====================
check_jump_blocked

jumpDist: distance monster is attempting to advance
downLimit/upLimit: max altitude to approve jump for; 0 is for no change
=====================
*/
qboolean check_jump_blocked (edict_t *monster, float jumpDist, float downLimit, float upLimit)
{
	edict_t		*target;
	trace_t		jumpTrace;
	int			enemy_relHeight;
	vec3_t		point1, point2, forward, up;
	vec_t		d0, d1;

	// Lazarus: Rogue only did this for enemies. We do it for enemies or
	//          movetargets

	if (!monster->monsterinfo.jump) return false;
	if (monster->enemy) target = monster->enemy;
	else if (monster->movetarget) target = monster->movetarget;	
	else return false;

	VectorSubtract(target->s.origin, monster->s.origin, point1);
	d0 = VectorLength(point1);

	AngleVectors (monster->s.angles, forward, NULL, up);
	VectorMA(monster->s.origin, 48, forward, point1);
	VectorCopy (point1, point2);

	// check our enemy's comparative elevation
	if ((target->absmin[2] + 16) < monster->absmin[2])
		enemy_relHeight = -1;
	else if ((target->absmin[2] - 16) > monster->absmin[2])
		enemy_relHeight = 1;
	else enemy_relHeight = 0;

	if (enemy_relHeight == -1 && downLimit)
	{
		// make sure jump off location is accessible
		jumpTrace = gi.trace(monster->s.origin, monster->mins, monster->maxs, point1, monster, MASK_MONSTERSOLID);
		if (jumpTrace.fraction < 1) return false;

		point2[2] = monster->mins[2] - downLimit - 1;
		jumpTrace = gi.trace(point1, vec3_origin, vec3_origin, point2, monster, MASK_MONSTERSOLID | MASK_WATER);
		if (jumpTrace.fraction < 1 && !jumpTrace.startsolid && !jumpTrace.allsolid
			&& (monster->absmin[2] - jumpTrace.endpos[2]) >= 24 && jumpTrace.contents & MASK_SOLID)
		{
			if ( (target->absmin[2] - jumpTrace.endpos[2]) > 32) return false;
			if (jumpTrace.plane.normal[2] < 0.9) return false;

			VectorSubtract(target->s.origin, jumpTrace.endpos, point1);
			d1 = VectorLength(point1);
			if (d0 < d1)
				return false;

			monster->velocity[0] = forward[0]*jumpDist*10;
			monster->velocity[1] = forward[1]*jumpDist*10;
			monster->velocity[2] = max(monster->velocity[2],100);

			gi.linkentity(monster);
			return true;
		}
	}
	else if (enemy_relHeight == 1 && upLimit)
	{
		point1[2] = monster->absmax[2] + upLimit;
		jumpTrace = gi.trace(point1, vec3_origin, vec3_origin, point2, monster, MASK_MONSTERSOLID | MASK_WATER);
		if (jumpTrace.fraction < 1 && !jumpTrace.startsolid && !jumpTrace.allsolid
			&& (jumpTrace.endpos[2] - monster->absmin[2]) <= upLimit && jumpTrace.contents & MASK_SOLID)
		{
			VectorSubtract(target->s.origin, jumpTrace.endpos, point1);
			d1 = VectorLength(point1);
			if (d0 < d1)
				return false;

			face_wall(monster);
			monster->monsterinfo.jump(monster);
			monster->velocity[0] = forward[0]*jumpDist*10;
			monster->velocity[1] = forward[1]*jumpDist*10;
			monster->velocity[2] = max(monster->velocity[2],200);

			gi.linkentity(monster);
			return true;
		}
	}
	return false;
}

/*
===============================================

  MISC CODE

===============================================
*/

float realrange (edict_t *this, edict_t *that)
{
	vec3_t offset;
	VectorSubtract (this->s.origin, that->s.origin, offset);
	return VectorLength(offset);
}

qboolean face_wall (edict_t *monster)
{
	trace_t	trace;
	vec3_t	point, fwd, angles;

	AngleVectors (monster->s.angles, fwd, NULL, NULL);
	VectorMA (monster->s.origin, 64, fwd, point);
	trace = gi.trace(monster->s.origin, vec3_origin, vec3_origin, point, monster, MASK_MONSTERSOLID);
	if (trace.fraction < 1 && !trace.startsolid && !trace.allsolid)
	{
		vectoangles2(trace.plane.normal, angles);
		monster->ideal_yaw = angles[YAW] + 180;
		if (monster->ideal_yaw > 360) monster->ideal_yaw -= 360;
		M_ChangeYaw(monster);
		return true;
	}
	return false;
}

qboolean has_valid_enemy (edict_t *monster)
{
	if (!monster->enemy || !monster->enemy->inuse)
		return false;

	// Lazarus: first take into account medics pursuing dead monsters
	if ((monster->monsterinfo.aiflags & AI_MEDIC) && (monster->enemy->health > 0))
		return false;
	else if (monster->enemy->health < 1)
		return false;

	return true;
}

