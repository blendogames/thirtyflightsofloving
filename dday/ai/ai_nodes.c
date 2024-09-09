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


//ACE



//===========================================================
//
//				NODES
//
//===========================================================



//==========================================
// AI_DropNodeOriginToFloor
//==========================================
qboolean AI_DropNodeOriginToFloor( vec3_t origin, edict_t *passent )
{
	trace_t	trace;

	//trap_Trace ( &trace, origin, tv(-15, -15, 0), tv(15, 15, 0), tv(origin[0], origin[1], world->mins[2]), NULL, MASK_NODESOLID );
	trace = gi.trace( origin, tv(-15, -15, 0), tv(15, 15, 0), tv(origin[0], origin[1], origin[2]-2048), passent, MASK_NODESOLID );
	if( trace.startsolid )
		return false;

	origin[0] = trace.endpos[0];
	origin[1] = trace.endpos[1];
	origin[2] = trace.endpos[2] + 24;

	return true;
}


//==========================================
// AI_FlagsForNode
// check the world and set up node flags
//==========================================
int AI_FlagsForNode( vec3_t origin, edict_t *passent )
{
	trace_t	trace;
	int		flagsmask = 0;

	//water
	if( gi.pointcontents(origin) & MASK_WATER )
		flagsmask |= NODEFLAGS_WATER;

	//floor
	trace = gi.trace( origin, tv(-15,-15,0), tv(15,15,0), tv(origin[0], origin[1], origin[2] - AI_JUMPABLE_HEIGHT), passent, MASK_NODESOLID );
	if( trace.fraction < 1.0 )
		flagsmask &= ~NODEFLAGS_FLOAT;	//ok, it wasn't set, I know...
	else
		flagsmask |= NODEFLAGS_FLOAT;

	//ladder
//	trace = gi.trace( origin, tv(-18, -18, -16), tv(18, 18, 16), origin, passent, MASK_ALL );
//	if( trace.startsolid && trace.contents & CONTENTS_LADDER )
//		flagsmask |= NODEFLAGS_LADDER;

	return flagsmask;
}


//#define SHOW_JUMPAD_GUESS
#ifdef SHOW_JUMPAD_GUESS
//==========================================
// just a debug tool
//==========================================
void AI_JumpadGuess_ShowPoint( vec3_t origin, char *modelname )
{
	edict_t	*ent;

	ent = G_Spawn();
	VectorCopy ( origin, ent->s.origin);
	VectorClear ( ent->movedir );
	ent->movetype = MOVETYPE_NONE;
	ent->clipmask = MASK_WATER;
	ent->solid = SOLID_NOT;
//	ent->s.type = ET_GENERIC;
//	ent->s.renderfx |= RF_NOSHADOW;
	VectorClear ( ent->mins );
	VectorClear ( ent->maxs );
	ent->s.modelindex = gi.modelindex ( modelname );
	ent->nextthink = level.time + 20000;
	ent->think = G_FreeEdict;
	ent->classname = "checkent";

	gi.linkentity (ent);
}
#endif

//==========================================
// AI_PredictJumpadDestity
// Make a guess on where a jumpad will send
// the player.
//==========================================
qboolean AI_PredictJumpadDestity( edict_t *ent, vec3_t out )
{
	int		i;
	edict_t *target;
	trace_t	trace;
	vec3_t	pad_origin, v1, v2;
	float	htime, vtime, tmpfloat, player_factor;	//player movement guess
	vec3_t	floor_target_origin, target_origin;
	vec3_t	floor_dist_vec, floor_movedir;

	VectorClear( out );

	if( !ent->target )	//jabot092
		return false;

	// get target entity
	target = G_Find ( NULL, FOFS(targetname), ent->target );
	if (!target)
		return false;

	// find pad origin
	VectorCopy( ent->maxs, v1 );
	VectorCopy( ent->mins, v2 );
	pad_origin[0] = (v1[0] - v2[0]) / 2 + v2[0];
	pad_origin[1] = (v1[1] - v2[1]) / 2 + v2[1];
	pad_origin[2] = ent->maxs[2];

	//make a projection 'on floor' of target origin
	VectorCopy( target->s.origin, target_origin );
	VectorCopy( target->s.origin, floor_target_origin );
	floor_target_origin[2] = pad_origin[2];	//put at pad's height

	//make a guess on how player movement will affect the trajectory
	tmpfloat = AI_Distance( pad_origin, floor_target_origin );
	htime = sqrt ((tmpfloat));
	vtime = sqrt ((target->s.origin[2] - pad_origin[2]));
	if(!vtime) return false;
	htime *= 4;vtime *= 4;
	if( htime > vtime )
		htime = vtime;
	player_factor = vtime - htime;

	// find distance vector, on floor, from pad_origin to target origin.
	for ( i=0 ; i<3 ; i++ )
		floor_dist_vec[i] = floor_target_origin[i] - pad_origin[i];

	// movement direction on floor
	VectorCopy( floor_dist_vec, floor_movedir );
	VectorNormalize( floor_movedir );

	// move both target origin and target origin on floor by player movement factor.
	VectorMA ( target_origin, player_factor, floor_movedir, target_origin);
	VectorMA ( floor_target_origin, player_factor, floor_movedir, floor_target_origin);

	// move target origin on floor by floor distance, and add another player factor step to it
	VectorMA ( floor_target_origin, 1, floor_dist_vec, floor_target_origin);
	VectorMA ( floor_target_origin, player_factor, floor_movedir, floor_target_origin);

#ifdef SHOW_JUMPAD_GUESS
	// this is our top of the curve point, and the original target
	AI_JumpadGuess_ShowPoint( target_origin, "models/objects/grenade2/tris.md2" );
	AI_JumpadGuess_ShowPoint( target->s.origin, "models/powerups/health/large_cross.md3" );
#endif

	//trace from target origin to endPoint.
//	trap_Trace ( &trace, target_origin, tv(-15, -15, -8), tv(15, 15, 8), floor_target_origin, NULL, MASK_NODESOLID);
	trace = gi.trace(  target_origin, tv(-15, -15, -8), tv(15, 15, 8), floor_target_origin, NULL, MASK_NODESOLID);
	if (trace.fraction == 1.0 && trace.startsolid || trace.allsolid && trace.startsolid){
//		G_Printf("JUMPAD LAND: ERROR: trace was in solid.\n"); //started inside solid (target should never be inside solid, this is a mapper error)
		return false;
	} else if ( trace.fraction == 1.0 ) {

		//didn't find solid. Extend Down (I have to improve this part)
		vec3_t	target_origin2, extended_endpoint, extend_dist_vec;

		VectorCopy( floor_target_origin, target_origin2 );
		for ( i=0 ; i<3 ; i++ )
			extend_dist_vec[i] = floor_target_origin[i] - target_origin[i];

		VectorMA ( target_origin2, 1, extend_dist_vec, extended_endpoint);
		//repeat tracing
//		trap_Trace ( &trace, target_origin2, tv(-15, -15, -8), tv(15, 15, 8), extended_endpoint, NULL, MASK_NODESOLID);
		trace = gi.trace(  target_origin2, tv(-15, -15, -8), tv(15, 15, 8), extended_endpoint, NULL, MASK_NODESOLID);
		if ( trace.fraction == 1.0 )
			return false;//still didn't find solid
	}

#ifdef SHOW_JUMPAD_GUESS
	// destiny found
	AI_JumpadGuess_ShowPoint( trace.endpos, "models/objects/grenade2/tris.md2" );
#endif

	VectorCopy ( trace.endpos, out );
	return true;
}


//==========================================
// AI_AddNode_JumpPad
// Drop two nodes, one at jump pad and other
// at predicted destity
//==========================================
int AI_AddNode_JumpPad( edict_t *ent )
{
	vec3_t	v1,v2;
	vec3_t	out;

	if (nav.num_nodes + 1 > MAX_NODES)
		return INVALID;

	if( !AI_PredictJumpadDestity( ent, out ))
		return INVALID;

	// jumpad node
	nodes[nav.num_nodes].flags = (NODEFLAGS_JUMPPAD|NODEFLAGS_SERVERLINK|NODEFLAGS_REACHATTOUCH);

	// find the origin
	VectorCopy( ent->maxs, v1 );
	VectorCopy( ent->mins, v2 );
	nodes[nav.num_nodes].origin[0] = (v1[0] - v2[0]) / 2 + v2[0];
	nodes[nav.num_nodes].origin[1] = (v1[1] - v2[1]) / 2 + v2[1];
	nodes[nav.num_nodes].origin[2] = ent->maxs[2] + 16;	//raise it up a bit

	nodes[nav.num_nodes].flags |= AI_FlagsForNode( nodes[nav.num_nodes].origin, NULL );

	nav.num_nodes++;

	// Destiny node
	nodes[nav.num_nodes].flags = (NODEFLAGS_JUMPPAD_LAND|NODEFLAGS_SERVERLINK);
	nodes[nav.num_nodes].origin[0] = out[0];
	nodes[nav.num_nodes].origin[1] = out[1];
	nodes[nav.num_nodes].origin[2] = out[2];	//raise it up a bit
	AI_DropNodeOriginToFloor( nodes[nav.num_nodes].origin, NULL );

	nodes[nav.num_nodes].flags |= AI_FlagsForNode( nodes[nav.num_nodes].origin, NULL );

	// link jumpad to dest
	AI_AddLink( nav.num_nodes-1 , nav.num_nodes, LINK_JUMPPAD );

	nav.num_nodes++;
	return nav.num_nodes -1;
}


//==========================================
// AI_AddNode_Door -  //jabot092(2)
// Drop a node at each side of the door
// and force them to link. Only typical
// doors are covered.
//==========================================
int AI_AddNode_Door( edict_t *ent )
{
	edict_t		*other;
	vec3_t		mins, maxs;
	vec3_t		door_origin;
	vec3_t		crossdir;
	vec3_t		MOVEDIR_UP		= {0, 0, 1};
	vec3_t		MOVEDIR_DOWN	= {0, 0, -1};

	if (ent->flags & FL_TEAMSLAVE)
		return INVALID;		//only team master will drop the nodes

	//make box formed by all team members boxes
	VectorCopy (ent->absmin, mins);
	VectorCopy (ent->absmax, maxs);

	for (other = ent->teamchain ; other ; other=other->teamchain)
	{
		AddPointToBounds (other->absmin, mins, maxs);
		AddPointToBounds (other->absmax, mins, maxs);
	}

	door_origin[0] = (maxs[0] - mins[0]) / 2 + mins[0];
	door_origin[1] = (maxs[1] - mins[1]) / 2 + mins[1];
	door_origin[2] = (maxs[2] - mins[2]) / 2 + mins[2];


	//if it moves in y axis we don't know if it's walked to north or east, so
	// we must try dropping nodes in both directions and check for solids
	if (VectorCompare(MOVEDIR_UP, ent->movedir) || VectorCompare(MOVEDIR_DOWN, ent->movedir) )
	{
		//now find the crossing angle
		AngleVectors( ent->s.angles, crossdir, NULL, NULL );
		VectorNormalize( crossdir );

		//add node
		nodes[nav.num_nodes].flags = 0;
		VectorMA( door_origin, 32, crossdir, nodes[nav.num_nodes].origin);
		if( AI_DropNodeOriginToFloor( nodes[nav.num_nodes].origin, NULL ) )
		{
			nodes[nav.num_nodes].flags |= AI_FlagsForNode( nodes[nav.num_nodes].origin, NULL );
#ifdef SHOW_JUMPAD_GUESS
			AI_JumpadGuess_ShowPoint( nodes[nav.num_nodes].origin, "models/objects/grenade2/tris.md2" );
#endif
			nav.num_nodes++;
		}

		//add node 2
		nodes[nav.num_nodes].flags = 0;
		VectorMA( door_origin, -32, crossdir, nodes[nav.num_nodes].origin);
		if( AI_DropNodeOriginToFloor( nodes[nav.num_nodes].origin, NULL ) )
		{
			nodes[nav.num_nodes].flags |= AI_FlagsForNode( nodes[nav.num_nodes].origin, NULL );
#ifdef SHOW_JUMPAD_GUESS
			AI_JumpadGuess_ShowPoint( nodes[nav.num_nodes].origin, "models/objects/grenade2/tris.md2" );
#endif
			//add links in both directions
			AI_AddLink( nav.num_nodes, nav.num_nodes-1, LINK_MOVE );
			AI_AddLink( nav.num_nodes-1, nav.num_nodes, LINK_MOVE );

			nav.num_nodes++;
		}

	}

	//find the crossing angle
	AngleVectors( ent->s.angles, NULL, crossdir, NULL ); //jabot092(2)
	VectorNormalize( crossdir );

	//add node
	nodes[nav.num_nodes].flags = 0;
	VectorMA( door_origin, 32, crossdir, nodes[nav.num_nodes].origin);
	if( AI_DropNodeOriginToFloor( nodes[nav.num_nodes].origin, NULL ) )
	{
		nodes[nav.num_nodes].flags |= AI_FlagsForNode( nodes[nav.num_nodes].origin, NULL );
#ifdef SHOW_JUMPAD_GUESS
		AI_JumpadGuess_ShowPoint( nodes[nav.num_nodes].origin, "models/objects/grenade2/tris.md2" );
#endif
		nav.num_nodes++;
	}

	//add node 2
	nodes[nav.num_nodes].flags = 0;
	VectorMA( door_origin, -32, crossdir, nodes[nav.num_nodes].origin);
	if( AI_DropNodeOriginToFloor( nodes[nav.num_nodes].origin, NULL ) )
	{
		nodes[nav.num_nodes].flags |= AI_FlagsForNode( nodes[nav.num_nodes].origin, NULL );
#ifdef SHOW_JUMPAD_GUESS
		AI_JumpadGuess_ShowPoint( nodes[nav.num_nodes].origin, "models/objects/grenade2/tris.md2" );
#endif
		//add links in both directions
		AI_AddLink( nav.num_nodes, nav.num_nodes-1, LINK_MOVE );
		AI_AddLink( nav.num_nodes-1, nav.num_nodes, LINK_MOVE );

		nav.num_nodes++;
	}

	return nav.num_nodes-1;
}


//==========================================
// AI_AddNode_Platform -  //jabot092(2)
// drop two nodes one at top, one at bottom
//==========================================
int AI_AddNode_Platform( edict_t *ent )
{
	vec3_t		v1,v2;
	float		plat_dist;

	if (nav.num_nodes + 2 > MAX_NODES)
		return INVALID;

	if (ent->flags & FL_TEAMSLAVE)
		return INVALID;		//only team master will drop the nodes

	plat_dist = ent->pos1[2] - ent->pos2[2]; //jabot092(2)

	// Upper node
	nodes[nav.num_nodes].flags = (NODEFLAGS_PLATFORM|NODEFLAGS_SERVERLINK|NODEFLAGS_FLOAT);
	VectorCopy( ent->maxs, v1 );
	VectorCopy( ent->mins, v2 );
	nodes[nav.num_nodes].origin[0] = (v1[0] - v2[0]) / 2 + v2[0];
	nodes[nav.num_nodes].origin[1] = (v1[1] - v2[1]) / 2 + v2[1];
	nodes[nav.num_nodes].origin[2] = ent->mins[2] + plat_dist + 16;//jabot092(2)
#ifdef SHOW_JUMPAD_GUESS
	AI_JumpadGuess_ShowPoint( nodes[nav.num_nodes].origin, "models/objects/grenade2/tris.md2" );
#endif
	nodes[nav.num_nodes].flags |= AI_FlagsForNode( nodes[nav.num_nodes].origin, NULL );

	//put into ents table
	nav.ents[nav.num_ents].ent = ent;
	nav.ents[nav.num_ents].node = nav.num_nodes;
	nav.num_ents++;

	nav.num_nodes++;

	// Lower node
	nodes[nav.num_nodes].flags = (NODEFLAGS_PLATFORM|NODEFLAGS_SERVERLINK|NODEFLAGS_FLOAT);
	nodes[nav.num_nodes].origin[0] = nodes[nav.num_nodes-1].origin[0];
	nodes[nav.num_nodes].origin[1] = nodes[nav.num_nodes-1].origin[1];
	nodes[nav.num_nodes].origin[2] = ent->mins[2] + (AI_JUMPABLE_HEIGHT - 1); //jabot092(2)
#ifdef SHOW_JUMPAD_GUESS
	AI_JumpadGuess_ShowPoint( nodes[nav.num_nodes].origin, "models/objects/grenade2/tris.md2" );
#endif
	nodes[nav.num_nodes].flags |= AI_FlagsForNode( nodes[nav.num_nodes].origin, NULL );

	//put into ents table
	nav.ents[nav.num_ents].ent = ent;
	nav.ents[nav.num_ents].node = nav.num_nodes;
	nav.num_ents++;

	// link lower to upper
	AI_AddLink( nav.num_nodes, nav.num_nodes-1, LINK_PLATFORM );

	//next
	nav.num_nodes++;
	return nav.num_nodes-1;
}


//==========================================
// AI_AddNode_Teleporter
// Drop two nodes, one at trigger and other
// at target entity
//==========================================
int AI_AddNode_Teleporter( edict_t *ent )
{
	vec3_t		v1,v2;
	edict_t		*dest;

	if (nav.num_nodes + 1 > MAX_NODES)
		return INVALID;

	dest = G_Find ( NULL, FOFS(targetname), ent->target );
	if (!dest)
		return INVALID;

	//NODE_TELEPORTER_IN
	nodes[nav.num_nodes].flags = (NODEFLAGS_TELEPORTER_IN|NODEFLAGS_SERVERLINK);

	VectorCopy( ent->maxs, v1 );
	VectorCopy( ent->mins, v2 );
	nodes[nav.num_nodes].origin[0] = (v1[0] - v2[0]) / 2 + v2[0];
	nodes[nav.num_nodes].origin[1] = (v1[1] - v2[1]) / 2 + v2[1];
	nodes[nav.num_nodes].origin[2] = ent->mins[2]+32;

	nodes[nav.num_nodes].flags |= AI_FlagsForNode( nodes[nav.num_nodes].origin, ent );

	nav.num_nodes++;

	//NODE_TELEPORTER_OUT
	nodes[nav.num_nodes].flags = (NODEFLAGS_TELEPORTER_OUT|NODEFLAGS_SERVERLINK);
	VectorCopy( dest->s.origin, nodes[nav.num_nodes].origin );
	if ( ent->spawnflags & 1 ) // droptofloor
		nodes[nav.num_nodes].flags |= NODEFLAGS_FLOAT;
	else
		AI_DropNodeOriginToFloor( nodes[nav.num_nodes].origin, NULL );

	nodes[nav.num_nodes].flags |= AI_FlagsForNode( nodes[nav.num_nodes].origin, ent );

	// link from teleport_in
	AI_AddLink( nav.num_nodes-1, nav.num_nodes, LINK_TELEPORT );

	nav.num_nodes++;
	return nav.num_nodes -1;
}


//==========================================
// AI_AddNode_BotRoam
// add nodes from bot roam entities
//==========================================
int AI_AddNode_BotRoam( edict_t *ent )
{
	if( nav.num_nodes + 1 > MAX_NODES )
		return INVALID;

	nodes[nav.num_nodes].flags = (NODEFLAGS_BOTROAM);//bot roams are not NODEFLAGS_NOWORLD

	// Set location
	VectorCopy( ent->s.origin, nodes[nav.num_nodes].origin );
	if ( ent->spawnflags & 1 ) // floating items
		nodes[nav.num_nodes].flags |= NODEFLAGS_FLOAT;
	else
		if( !AI_DropNodeOriginToFloor( nodes[nav.num_nodes].origin, NULL ) )
			return INVALID;//spawned inside solid

	nodes[nav.num_nodes].flags |= AI_FlagsForNode( nodes[nav.num_nodes].origin, NULL );

	//count into bot_roams table
	nav.broams[nav.num_broams].node = nav.num_nodes;

	if( ent->count )
		nav.broams[nav.num_broams].weight = ent->count * 0.01;//count is a int with a value in between 0 and 100
	else
		nav.broams[nav.num_broams].weight = 0.3;

	nav.num_broams++;
	nav.num_nodes++;
	return nav.num_nodes-1; // return the node added
}


//==========================================
// AI_AddNode_ItemNode
// Used to add nodes from items
//==========================================
int AI_AddNode_ItemNode( edict_t *ent )
{
	if (nav.num_nodes + 1 > MAX_NODES)
		return INVALID;

	VectorCopy( ent->s.origin, nodes[nav.num_nodes].origin );
	if ( ent->spawnflags & 1 ) // floating items
		nodes[nav.num_nodes].flags |= NODEFLAGS_FLOAT;
	else
		if( !AI_DropNodeOriginToFloor( nodes[nav.num_nodes].origin, NULL ) )
			return INVALID;	//spawned inside solid

	nodes[nav.num_nodes].flags |= AI_FlagsForNode( nodes[nav.num_nodes].origin, NULL );

	nav.num_nodes++;
	return nav.num_nodes-1; // return the node added
}


//==========================================
// AI_CreateNodesForEntities
//
// Entities aren't saved into nodes files anymore.
// They generate and link its nodes at map load.
//==========================================
void AI_CreateNodesForEntities ( void )
{
	edict_t *ent;
	int		node;

	nav.num_ents = 0;
	memset( nav.ents, 0, sizeof(nav_ents_t) * MAX_EDICTS );

	// Add special entities
//	for( ent = game.edicts; ent < &game.edicts[game.numentities]; ent++ )
	for( ent = g_edicts; ent < &g_edicts[game.maxentities]; ent++ )
	{
		if( !ent->classname )
			continue;

		// platforms
		if( !strcmp( ent->classname,"func_plat" ) )
		{
			AI_AddNode_Platform( ent );
		}
		// teleporters
		else if( !strcmp( ent->classname,"trigger_teleport" ) )
		//else if( !strcmp( ent->classname,"misc_teleport" ) ) //should be this? needs testing though
		{
			AI_AddNode_Teleporter( ent );
		}
		// jump pads
		else if( !strcmp( ent->classname,"trigger_push" ) )
		{
			AI_AddNode_JumpPad( ent );
		}
		// doors
		else if(ent->classnameb == FUNC_DOOR)
		{
			AI_AddNode_Door( ent );
		}
		else if (ent->classnameb == FUNC_DOOR_ROTATING)//faf: probably fubared, but better than nothing
		{
			AI_AddNode_Door( ent );
		}
	}

	// bot roams
	nav.num_broams = 0;
	memset( nav.broams, 0, sizeof(nav_broam_t) * MAX_BOT_ROAMS );

	//visit world nodes first, and put in list what we find in there
	for( node=0; node < nav.num_nodes; node++ )
	{
//		if( nodes[node].flags & NODEFLAGS_BOTROAM && nav.num_broams < MAX_BOT_ROAMS)
//		{
//			nav.broams[nav.num_broams].node = node;
//			nav.broams[nav.num_broams].weight = 0.3;
//			nav.num_broams++;
//		}
	}

	//now add bot roams from entities
//	for(ent = game.edicts; ent < &game.edicts[game.numentities]; ent++)
	for( ent = g_edicts; ent < &g_edicts[game.maxentities]; ent++ )
	{
		if( !ent->classname )
			continue;

		if( !strcmp( ent->classname,"item_botroam" ) )
		{
			//if we have a available node close enough to the item, use it instead of dropping a new node
			node = AI_FindClosestReachableNode( ent->s.origin, NULL, 48, NODE_ALL );
			if( node != -1 &&
				!(nodes[node].flags & NODEFLAGS_SERVERLINK) &&
				!(nodes[node].flags & NODEFLAGS_LADDER) )
			{
				float heightdiff = 0;
				heightdiff = ent->s.origin[2] - nodes[node].origin[2];
				if( heightdiff < 0 ) heightdiff = -heightdiff;

				if( heightdiff < AI_STEPSIZE && nav.num_broams < MAX_BOT_ROAMS ) //near enough
				{
					nodes[node].flags |= NODEFLAGS_BOTROAM;
					//add node to botroam list
					if( ent->count )
						nav.broams[nav.num_broams].weight = ent->count * 0.01;//count is a int with a value in between 0 and 100
					else
						nav.broams[nav.num_broams].weight = 0.3; //jalfixme: add cmd to weight (dropped by console cmd, self is player)

					nav.broams[nav.num_broams].node = node;
					nav.num_broams++;
					continue;
				}
			}

			//drop a new node
			if( nav.num_broams < MAX_BOT_ROAMS ){
				AI_AddNode_BotRoam( ent );
			}
		}
	}

	// game items (I want all them on top of the nodes array)
	nav.num_items = 0;
	memset( nav.items, 0, sizeof(nav_item_t) * MAX_EDICTS );

//	for( ent = game.edicts; ent < &game.edicts[game.numentities]; ent++ )
	for( ent = g_edicts; ent < &g_edicts[game.maxentities]; ent++ )
	{
		int	item_index;

		if( !ent->classname || !ent->item )
			continue;

		item_index = ITEM_INDEX( ent->item );
		if(item_index == INVALID)
			continue;

		//if we have a available node close enough to the item, use it
		node = AI_FindClosestReachableNode( ent->s.origin, NULL, 48, NODE_ALL );
		if( node != INVALID )
		{
			if( nodes[node].flags & NODEFLAGS_SERVERLINK ||
				nodes[node].flags & NODEFLAGS_LADDER )
				node = INVALID;
			else
			{
				float heightdiff = 0;
				heightdiff = ent->s.origin[2] - nodes[node].origin[2];
				if( heightdiff < 0 ) heightdiff = -heightdiff;

				if( heightdiff > AI_STEPSIZE )	//not near enough
					node = INVALID;
			}
		}

		//drop a new node
		if( node == INVALID )
			node = AI_AddNode_ItemNode( ent );

		if( node != INVALID )
		{
			nav.items[nav.num_items].node = node;
			nav.items[nav.num_items].ent = ent;
			nav.items[nav.num_items].item = item_index;
			nav.num_items++;
		}
	}
}


//==========================================
// AI_LoadPLKFile
// load nodes and plinks from file
//==========================================
qboolean AI_LoadPLKFile( char *mapname )
{
	FILE		*pIn;
	int			i;
	char		filename[MAX_OSPATH];
	int			version;

//	Com_sprintf (filename, sizeof(filename), "%s/%s/%s.%s", AI_MOD_FOLDER, AI_NODES_FOLDER, mapname, NAV_FILE_EXTENSION);
//	pIn = fopen( filename, "rb" );
	// Knightmare- use SavegameDir() / GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/%s/%s.%s", SavegameDir(), AI_NODES_FOLDER, mapname, NAV_FILE_EXTENSION);
	pIn = fopen(filename, "rb");
	if ( pIn  == NULL ) {
	//	gi.dprintf ("Couldn't load PLK file %s from SavegameDir, trying GameDir\n", filename);
		Com_sprintf (filename, sizeof(filename), "%s/%s/%s.%s", GameDir(), AI_NODES_FOLDER, mapname, NAV_FILE_EXTENSION);
		pIn = fopen(filename, "rb");
	}
	// end Knightmare
	if ( pIn  == NULL ) {
	//	gi.dprintf ("Couldn't load PLK file %s from Gamedir\n", filename);
		return false;
	}

	// check version
	fread (&version, sizeof(int), 1, pIn);

	if ( version != NAV_FILE_VERSION )
	{
		fclose (pIn);
		return false;
	}

	fread (&nav.num_nodes, sizeof(int), 1, pIn);

	for (i=0; i<nav.num_nodes; i++)
		fread (&nodes[i], sizeof(nav_node_t), 1, pIn);

	for (i=0; i<nav.num_nodes; i++)
		fread (&pLinks[i], sizeof(nav_plink_t), 1, pIn);

	fclose (pIn);

	return true;
}


//==========================================
// AI_IsPlatformLink
// interpretation of this link type
//==========================================
int AI_IsPlatformLink( int n1, int n2 )
{
	int	i;
	if( nodes[n1].flags & NODEFLAGS_PLATFORM && nodes[n2].flags & NODEFLAGS_PLATFORM )
	{
		//the link was added by it's dropping function or it's invalid
		return LINK_INVALID;
	}

	//if first is plat but not second
	if( nodes[n1].flags & NODEFLAGS_PLATFORM && !(nodes[n2].flags & NODEFLAGS_PLATFORM) )
	{
		edict_t *n1ent = NULL;
		int		othernode = 0;

		// find ent
		for(i=0;i<nav.num_ents;i++) {
			if( nav.ents[i].node == n1 )
				n1ent = nav.ents[i].ent;
		}
		// find the other node from that ent
		for(i=0;i<nav.num_ents;i++){
			if( nav.ents[i].node != n1 && nav.ents[i].ent == n1ent)
				othernode = nav.ents[i].node;
		}

		if( othernode == -1 || !n1ent )
			return LINK_INVALID;

		//find out if n1 is the upper or the lower plat node
		if( nodes[n1].origin[2] < nodes[othernode].origin[2] )
		{
			//n1 is plat lower: it can't link TO anything but upper plat node
			return LINK_INVALID;

		} else {

			trace_t	trace;
			float	heightdiff;
			//n1 is plat upper: it can link to visibles at same height
			trace = gi.trace( nodes[n1].origin, vec3_origin, vec3_origin, nodes[n2].origin, n1ent, MASK_NODESOLID );
			if (trace.fraction == 1.0 && !trace.startsolid)
			{
				heightdiff = nodes[n1].origin[2] - nodes[n2].origin[2];
				if( heightdiff < 0 )
					heightdiff = -heightdiff;

				if( heightdiff < AI_JUMPABLE_HEIGHT )
					return LINK_MOVE;

				return LINK_INVALID;
			}
		}
	}

	//only second is plat node
	if( !(nodes[n1].flags & NODEFLAGS_PLATFORM) && nodes[n2].flags & NODEFLAGS_PLATFORM )
	{
		edict_t *n2ent = NULL;
		int		othernode = 0;

		// find ent
		for(i=0;i<nav.num_ents;i++) {
			if( nav.ents[i].node == n2 )
				n2ent = nav.ents[i].ent;
		}
		// find the other node from that ent
		for(i=0;i<nav.num_ents;i++){
			if( nav.ents[i].node != n2 && nav.ents[i].ent == n2ent)
				othernode = nav.ents[i].node;
		}

		if( othernode == -1 || !n2ent )
			return LINK_INVALID;

		//find out if n2 is the upper or the lower plat node
		if( nodes[n2].origin[2] < nodes[othernode].origin[2] )
		{
			trace_t	trace;
			float	heightdiff;

			//n2 is plat lower: other's can link to it when visible and good height
			trace = gi.trace( nodes[n1].origin, vec3_origin, vec3_origin, nodes[n2].origin, n2ent, MASK_NODESOLID );
			if (trace.fraction == 1.0 && !trace.startsolid)
			{
				heightdiff = nodes[n1].origin[2] - nodes[n2].origin[2];
				if( heightdiff < 0 )
					heightdiff = -heightdiff;

				if( heightdiff < AI_JUMPABLE_HEIGHT )
					return LINK_MOVE;

				return LINK_INVALID;
			}

		} else {

			//n2 is plat upper: others can't link to plat upper nodes
			return LINK_INVALID;
		}
	}

	return LINK_INVALID;
}

//==========================================
// AI_IsJumpPadLink
// interpretation of this link type
//==========================================
int AI_IsJumpPadLink( int n1, int n2 )
{
	if( nodes[n1].flags & NODEFLAGS_JUMPPAD )
		return LINK_INVALID; //only can link TO jumppad land, and it's linked elsewhere

	if( nodes[n2].flags & NODEFLAGS_JUMPPAD_LAND )
		return LINK_INVALID; //linked as TO only from it's jumppad. Handled elsewhere

	return AI_GravityBoxToLink( n1, n2 );
}

//==========================================
// AI_IsTeleporterLink
// interpretation of this link type
//==========================================
int AI_IsTeleporterLink( int n1, int n2 )
{
	if( nodes[n1].flags & NODEFLAGS_TELEPORTER_IN )
		return LINK_INVALID;

	if (nodes[n2].flags & NODEFLAGS_TELEPORTER_OUT)
		return LINK_INVALID;

	//find out the link move type against the world
	return AI_GravityBoxToLink( n1, n2 );
}

//==========================================
// AI_FindServerLinkType
// determine what type of link it is
//==========================================
int AI_FindServerLinkType( int n1, int n2 )
{
	if( AI_PlinkExists( n1, n2 ))
		return LINK_INVALID;	//already saved

	if( nodes[n1].flags & NODEFLAGS_PLATFORM || nodes[n2].flags & NODEFLAGS_PLATFORM )
	{
		return AI_IsPlatformLink(n1, n2);
	}
	else if( nodes[n2].flags & NODEFLAGS_TELEPORTER_IN || nodes[n1].flags & NODEFLAGS_TELEPORTER_OUT )
	{
		return AI_IsTeleporterLink(n1, n2);
	}
	else if( nodes[n2].flags & NODEFLAGS_JUMPPAD || nodes[n1].flags & NODEFLAGS_JUMPPAD_LAND )
	{
		return AI_IsJumpPadLink(n1, n2);
	}
	return LINK_INVALID;
}

//==========================================
// AI_LinkServerNodes
// link the new nodes to&from those loaded from disk
//==========================================
int AI_LinkServerNodes( int start )
{
	int			n1, n2;
	int			count = 0;
	float		pLinkRadius = NODE_DENSITY*1.2;
	qboolean	ignoreHeight = true;

	if( start >= nav.num_nodes )
		return 0;

	for( n1=start; n1<nav.num_nodes; n1++ )
	{
		n2 = 0;
		n2 = AI_findNodeInRadius ( 0, nodes[n1].origin, pLinkRadius, ignoreHeight);

		while (n2 != -1)
		{
			if( nodes[n1].flags & NODEFLAGS_SERVERLINK || nodes[n2].flags & NODEFLAGS_SERVERLINK )
			{
				if( AI_AddLink( n1, n2, AI_FindServerLinkType(n1, n2) ) )
					count++;

				if( AI_AddLink( n2, n1, AI_FindServerLinkType(n2, n1) ) )
					count++;
			}
			else
			{
				if( AI_AddLink( n1, n2, AI_FindLinkType(n1, n2) ) )
					count++;

				if( AI_AddLink( n2, n1, AI_FindLinkType(n2, n1) ) )
					count++;
			}

			n2 = AI_findNodeInRadius ( n2, nodes[n1].origin, pLinkRadius, ignoreHeight);
		}
	}
	return count;
}


int AI_LinkCloseNodes_JumpPass( int start );
//==========================================
// AI_InitNavigationData
// Setup nodes & links for this map
//==========================================
void AI_InitNavigationData (void)
{
	int	i;
	int newlinks;
	int newjumplinks;
	int linkscount;
	int	servernodesstart = 0;

	// Init nodes arrays
	nav.num_nodes = 0;
	memset (nodes, 0, sizeof(nav_node_t) * MAX_NODES);
	memset (pLinks, 0, sizeof(nav_plink_t) * MAX_NODES);

	// Load nodes from file

//	if (level.botfiles)
//		nav.loaded = AI_LoadPLKFile( level.mapname );
//	else
		nav.loaded = AI_LoadPLKFile( level.mapname );

	if (!nav.loaded)
	{	nav.loaded = -1;
	//	if( !nav.loaded ) {
		Com_Printf ("AI: FAILED to load nodes file.\n");
		return;
	}

	servernodesstart = nav.num_nodes;

	for (linkscount = 0, i = 0; i< nav.num_nodes; i++)
		linkscount += pLinks[i].numLinks;

	// create nodes for map entities
	AI_CreateNodesForEntities ();
	newlinks = AI_LinkServerNodes( servernodesstart );
	newjumplinks = AI_LinkCloseNodes_JumpPass( servernodesstart );

	Com_Printf( "-------------------------------------\n" );
	Com_Printf ("       : AI: Nodes Initialized.\n" );
	Com_Printf ("       : loaded nodes: %i.\n", servernodesstart );
	Com_Printf ("       : added nodes: %i.\n", nav.num_nodes - servernodesstart );
	Com_Printf ("       : total nodes: %i.\n", nav.num_nodes );
	Com_Printf ("       : loaded links: %i.\n", linkscount );
	Com_Printf ("       : added links: %i.\n", newlinks );
	Com_Printf ("       : added jump links: %i.\n", newjumplinks );
}
