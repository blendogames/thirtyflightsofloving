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
--------------------------------------------------------------
The ACE Bot is a product of Steve Yeager, and is available from
the ACE Bot homepage, at http://www.axionfx.com/ace.

This program is a modification of the ACE Bot, and is therefore
in NO WAY supported by Steve Yeager.
*/

#include "../g_local.h"
#include "ai_local.h"



//==========================================
// AI_FindCost
// Determine cost of moving from one node to another
//==========================================
int AI_FindCost(int from, int to, int movetypes)
{
	astarpath_t	path;

	if( !AStar_GetPath( from, to, movetypes, &path ) )
		return -1;

	return path.numNodes;
}


//==========================================
// AI_FindClosestReachableNode
// Find the closest node to the player within a certain range
//==========================================
int AI_FindClosestReachableNode( vec3_t origin, edict_t *passent, int range, int flagsmask )
{
	int			i;
	float		closest = 99999;
	float		dist;
	int			node=-1;
	vec3_t		v;
	trace_t		tr;
	float		rng;
	vec3_t		maxs,mins;

	VectorSet( mins, -15, -15, -15 );
	VectorSet( maxs, 15, 15, 15 );

	// For Ladders, do not worry so much about reachability
	if( flagsmask & NODEFLAGS_LADDER )
	{
		VectorCopy(vec3_origin,maxs);
		VectorCopy(vec3_origin,mins);
	}

	rng = (float)(range * range); // square range for distance comparison (eliminate sqrt)

	for(i=0;i<nav.num_nodes;i++)
	{
		if( flagsmask == NODE_ALL || nodes[i].flags & flagsmask )
		{
			VectorSubtract( nodes[i].origin, origin, v );

			dist = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];

			if(dist < closest && dist < rng)
			{
				// make sure it is visible
				tr = gi.trace( origin, mins, maxs, nodes[i].origin, passent, MASK_AISOLID);
				if(tr.fraction == 1.0)
				{
					node = i;
					closest = dist;
				}
			}
		}
	}
	return node;
}


//==========================================
// AI_SetGoal
// set the goal //jabot092
//==========================================
void AI_SetGoal(edict_t *self, int goal_node)
{
	int			node;

	self->ai->goal_node = goal_node;
	node = AI_FindClosestReachableNode( self->s.origin, self, NODE_DENSITY*3, NODE_ALL );

	if(node == -1) {
		AI_SetUpMoveWander(self);
		return;
	}

	//------- ASTAR -----------
	//do this twice, 1 time randomly dropping some nodes out so bots take alternate routes.  Second time, normal.
	dropnodes = true;
	if( !AStar_GetPath( node, goal_node, self->ai->pers.moveTypesMask, &self->ai->path ) )
	{	
		dropnodes = false;
		if( !AStar_GetPath( node, goal_node, self->ai->pers.moveTypesMask, &self->ai->path ) )
		{
			AI_SetUpMoveWander(self);
			return;
		}
	}
	dropnodes = false;
	self->ai->current_node = self->ai->path.nodes[self->ai->path.numNodes];
	//-------------------------

//	if(AIDevel.debugChased && bot_showlrgoal->value)
//		G_PrintMsg (AIDevel.chaseguy, PRINT_HIGH, "%s: GOAL: new START NODE selected %d\n", self->ai->pers.netname, node);

	self->ai->next_node = self->ai->current_node; // make sure we get to the nearest node first
	self->ai->node_timeout = 0;
}


//==========================================
// AI_FollowPath
// Move closer to goal by pointing the bot to the next node
// that is closer to the goal //jabot092 (path-> to path.)
//==========================================
qboolean AI_FollowPath( edict_t *self )
{
	vec3_t			v;
	float			dist;


	if( self->ai->goal_node == INVALID )
		return false;

	// Try again?
	if(self->ai->node_timeout++ > 30)
	{
		if(self->ai->tries++ > 3)
			return false;
		else
			AI_SetGoal( self, self->ai->goal_node );
	}

	// Are we there yet?
	VectorSubtract( self->s.origin, nodes[self->ai->next_node].origin, v );
	dist = VectorLength(v);

	//special lower plat reached check
	if( dist < 64
		&& nodes[self->ai->current_node].flags & NODEFLAGS_PLATFORM
		&& nodes[self->ai->next_node].flags & NODEFLAGS_PLATFORM
		&& self->groundentity && self->groundentity->use == Use_Plat)
		dist = 16;

	if( (dist < 32 && nodes[self->ai->next_node].flags != NODEFLAGS_JUMPPAD && nodes[self->ai->next_node].flags != NODEFLAGS_TELEPORTER_IN)
		|| (self->ai->status.jumpadReached && nodes[self->ai->next_node].flags & NODEFLAGS_JUMPPAD)
		|| (self->ai->status.TeleportReached && nodes[self->ai->next_node].flags & NODEFLAGS_TELEPORTER_IN) )
	{
		// reset timeout
		self->ai->node_timeout = 0;

		if( self->ai->next_node == self->ai->goal_node )
		{
			//if(AIDevel.debugChased && bot_showlrgoal->value)
			//	G_PrintMsg (AIDevel.chaseguy, PRINT_HIGH, "%s: GOAL REACHED!\n", self->ai->pers.netname);
			
			//if botroam, setup a timeout for it
			if( nodes[self->ai->goal_node].flags & NODEFLAGS_BOTROAM )
			{
				int		i;
				for( i=0; i<nav.num_broams; i++) {	//find the broam
					if( nav.broams[i].node != self->ai->goal_node )
						continue;

					//if(AIDevel.debugChased && bot_showlrgoal->integer)
					//	G_PrintMsg (AIDevel.chaseguy, PRINT_HIGH, "%s: BotRoam Time Out set up for node %i\n", self->ai->pers.netname, nav.broams[i].node);
					//Com_Printf( "%s: BotRoam Time Out set up for node %i\n", self->ai->pers.netname, nav.broams[i].node);
					self->ai->status.broam_timeouts[i] = level.time + 15.0;
					break;
				}
			}
		
			// Pick a new goal
			if (self->ai->state != BOT_STATE_CAMP)
				AI_PickLongRangeGoal(self);

		//	if (self->ai->next_node == self->ai->current_node)
		//	{	//gi.dprintf ("next_node\n");
				//AI_PickLongRangeGoal(self);
		//	}
				
		}
		else
		{
			self->ai->current_node = self->ai->next_node;
			if( self->ai->path.numNodes )
				self->ai->path.numNodes--;
			self->ai->next_node = self->ai->path.nodes[self->ai->path.numNodes];
		}
	}

	if(self->ai->current_node == -1 || self->ai->next_node == -1)
		return false;

	// Set bot's movement vector
	VectorSubtract( nodes[self->ai->next_node].origin, self->s.origin , self->ai->move_vector );
	return true;
}


//==========================================
// AI_FindClosestReachableNode
// Find the closest node to a given origin
//==========================================
int AI_ClosestNodeToSpotx(vec3_t origin)
{
	int			i;
	float		closest = 99999;
	float		dist;
	int			node=-1;
	vec3_t		v;


	//gi.dprintf("%s\n",vtos(origin));

	for(i=0;i<nav.num_nodes;i++)
	{
		VectorSubtract( nodes[i].origin, origin, v );

		dist = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];

		if(dist < closest)
		{
			node = i;
			closest = dist;
		}
	}


	return node;
}

//==========================================
// AI_FindClosestReachableNode
// Find the closest node to the player within a certain range
//==========================================
int AI_ClosestNodeToSpot( vec3_t origin, edict_t *passent, qboolean visible)
{
	int			i;
	float		closest = 99999;
	float		dist;
	int			node=-1;
	vec3_t		v;
	trace_t		tr;
	float		rng;
	vec3_t		maxs,mins;

	int range = 200;

	VectorSet( mins, -15, -15, -15 );
	VectorSet( maxs, 15, 15, 15 );


	rng = (float)(range * range); // square range for distance comparison (eliminate sqrt)

	for(i=0;i<nav.num_nodes;i++)
	{
			VectorSubtract( nodes[i].origin, origin, v );

			dist = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];

			if(dist < closest && dist < rng)
			{
				if (visible == true)
				{
					// make sure it is visible
					tr = gi.trace( origin, mins, maxs, nodes[i].origin, passent, MASK_AISOLID);
					if(tr.fraction == 1.0)
					{
						node = i;
						closest = dist;
					}
				}
				else
				{
					node = i;
					closest = dist;
				}
			}
	}
	return node;
}




//==========================================
// AI_ClosestNodeToReachableSpot
// Find the closest node to a given origin that is reachable by player
//==========================================
int AI_ClosestNodeToReachableSpot(edict_t *self, vec3_t origin)
{
	int			i;
	float		closest = 999999999;
	float		dist;
	int			node=-1;
	vec3_t		v;

	int			check[3];
	int			c;


	//gi.dprintf("%s\n",vtos(origin));
	check[0]=-1;
	check[1]=-1;
	check[2]=-1;
	check[3]=-1;

	c = 0;
	while (c < 4)
	{
		closest = 999999999;
		node=-1;
		
		for(i=0;i<nav.num_nodes;i++)
		{
			//gi.dprintf("%s\n", vtos(nodes[i].origin));

			VectorSubtract( nodes[i].origin, origin, v );

			dist = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
			//gi.dprintf("%f\n", dist);
			if(dist < closest)
			{
//				if (i != check[0] && i != check[1] &&
//					i != check[2] && i != check[3])
//				{
					node = i;
					closest = dist;
//				}
			}
		}
		//gi.dprintf("check:  %i\n", node);
		if( AStar_GetPath( self->ai->current_node, node, self->ai->pers.moveTypesMask, &self->ai->path ) )
			return node;

		check [c] = node;

		//gi.dprintf("%s\n", vtos(nodes[node].origin));

		c++;
	}	
	//gi.dprintf("no node found\n");
	return -1;
}

