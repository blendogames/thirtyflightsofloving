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

edict_t		*LINKS_PASSENT = NULL;

//==========================================
// AI_LinkString
//==========================================
char *AI_LinkString( int linktype )
{
	char *s = NULL;

	if ( linktype == LINK_MOVE )
		s = "LINK_MOVE";
	else if ( linktype == LINK_STAIRS )
		s = "LINK_STAIRS";
	else if ( linktype == LINK_FALL )
		s = "LINK_FALL";
	else if ( linktype == LINK_CLIMB )
		s = "LINK_CLIMB";
	else if ( linktype == LINK_TELEPORT )
		s = "LINK_TELEPORT";
	else if ( linktype == LINK_PLATFORM )
		s = "LINK_PLATFORM";
	else if ( linktype == LINK_JUMPPAD )
		s = "LINK_JUMPAD";
	else if ( linktype == LINK_WATER )
		s = "LINK_WATER";
	else if ( linktype == LINK_WATERJUMP )
		s = "LINK_WATERJUMP";
	else if ( linktype == LINK_LADDER )
		s = "LINK_LADDER";
	else if ( linktype == LINK_INVALID )
	{
		gi.positioned_sound (vec3_origin, g_edicts, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NONE, 0);
		s = "LINK_INVALID";
	}
	else if ( linktype == LINK_JUMP )
		s = "LINK_JUMP";
	else if ( linktype )
		s = "UNKNOWN";

	return s;
}

//==========================================
// AI_VisibleOrigins
// same as is visible, but doesn't need ents
//==========================================
qboolean AI_VisibleOrigins (vec3_t spot1, vec3_t spot2)
{
	trace_t	trace;

//	AILink_Trace( &trace, spot1, vec3_origin, vec3_origin, spot2, NULL, MASK_NODESOLID );
	trace = gi.trace( spot1, vec3_origin, vec3_origin, spot2, LINKS_PASSENT, MASK_NODESOLID );
	if (trace.fraction == 1.0 && !trace.startsolid)
		return true;
	//Com_Printf("Bloqued");
	return false;
}


//==========================================
// AI_FindDistance
//==========================================
float AI_Distance( vec3_t o1, vec3_t o2 )
{
	vec3_t	distVec;

	VectorSubtract( o2, o1, distVec );
	return VectorLength(distVec);
}


//=================
//AI_findNodeInRadius
//
// Copy of findradius to act with nodes instead of entities
// Setting up ignoreHeight uses a cilinder instead of a sphere (used to catch fall links)
//=================
int AI_findNodeInRadius (int from, vec3_t org, float rad, qboolean ignoreHeight)
{
	vec3_t	eorg;
	int		j;

	if ( from < 0 )
		return -1;
	else if ( from > nav.num_nodes )
		return -1;
	else if ( !nav.num_nodes )
		return -1;
	else
		from++;

	for( ; from < nav.num_nodes; from++)
	{

		for( j=0 ; j<3 ; j++ )
			eorg[j] = org[j] - nodes[from].origin[j];

		if ( ignoreHeight )
			eorg[2] = 0;

		if ( VectorLength(eorg) > rad )
			continue;

		return from;
	}

	return -1;
}


//==========================================
// AI_FindLinkDistance
// returns world distance between both nodes
//==========================================
float AI_FindLinkDistance(int n1, int n2)
{
	// Teleporters exception: JALFIXME: add check for Destiny being teleporter's target
	if (nodes[n1].flags & NODEFLAGS_TELEPORTER_IN && nodes[n2].flags & NODEFLAGS_TELEPORTER_OUT )
		return NODE_DENSITY; //not 0, just because teleporting has a strategical cost

	return AI_Distance( nodes[n1].origin, nodes[2].origin );
}


//==================================================================
//
//		PLINKS (nodes linking. 1 hop only. Used later for pathfinding)
//
//==================================================================


//==========================================
// AI_AddLink
// force-add of a link
//==========================================
qboolean AI_AddLink( int n1, int n2, int linkType )
{
	//never store self-link
	if ( n1 == n2 )
		return false;

	//already referenced
	if ( AI_PlinkExists(n1, n2) )
		return false;

	if (linkType == LINK_INVALID)
		return false;

	//add the link
	if (pLinks[n1].numLinks > NODES_MAX_PLINKS)
	{
//		G_Printf("MaxPlinks Reached! node:%i numPlinks:%i\n", n1, pLinks[n1].numLinks);
		return false;
	}

	pLinks[n1].nodes[pLinks[n1].numLinks] = n2;
	pLinks[n1].dist[pLinks[n1].numLinks] = (int)AI_FindLinkDistance(n1, n2);
	pLinks[n1].moveType[pLinks[n1].numLinks] = linkType;

	pLinks[n1].numLinks++;

	return true;
}


//==========================================
// AI_PlinkExists
// Just check if the plink is already stored
//==========================================
qboolean AI_PlinkExists(int n1, int n2)
{
	int		i;

	if ( n1 == n2 )
		return false;

	for (i=0; i<pLinks[n1].numLinks; i++) {
		if (pLinks[n1].nodes[i] == n2)
			return true;
	}

	return false;
}

//==========================================
// AI_PlinkMoveType
// return moveType of an already stored link
//==========================================
int AI_PlinkMoveType(int n1, int n2)
{
	int		i;

	if ( !nav.loaded )
		return LINK_INVALID;

	if ( n1 == n2 )
		return LINK_INVALID;

	for (i=0; i<pLinks[n1].numLinks; i++) {
		if ( pLinks[n1].nodes[i] == n2 )
			return pLinks[n1].moveType[i];
	}

	return LINK_INVALID;
}


//==========================================
// AI_IsWaterJumpLink
// check against the world if we can link it
//==========================================
#define AI_WATERJUMP_HEIGHT 24
int	AI_IsWaterJumpLink( int n1, int n2 )
{
	vec3_t	waterorigin;
	vec3_t	solidorigin;
	trace_t	trace;
	float	heightdiff;

	//find n2 floor
	trace = gi.trace( nodes[n2].origin, tv(-15,-15,0), tv(15,15,0), tv(nodes[n2].origin[0], nodes[n2].origin[1], nodes[n2].origin[2] - AI_JUMPABLE_HEIGHT), NULL, MASK_NODESOLID );
	if ( trace.startsolid || trace.fraction == 1.0 )
		return LINK_INVALID;

	VectorCopy( trace.endpos, solidorigin );

	if ( gi.pointcontents(nodes[n1].origin) & MASK_WATER )
		VectorCopy( nodes[n1].origin, waterorigin );
	else {
		//try finding water below, isn't it a node_water?
		return LINK_INVALID;
	}

	if ( solidorigin[2] >= waterorigin[2])
		heightdiff = solidorigin[2] - waterorigin[2];
	else
		heightdiff = waterorigin[2] - solidorigin[2];

	if ( heightdiff > AI_WATERJUMP_HEIGHT )	//jalfixme: find true waterjump size
		return LINK_INVALID;

	//now find if bloqued
	waterorigin[2] = nodes[n2].origin[2];
	trace = gi.trace( nodes[n1].origin, tv(-15,-15,0), tv(15,15,0), waterorigin, NULL, MASK_NODESOLID );
	if ( trace.fraction < 1.0 )
		return LINK_INVALID;

	trace = gi.trace( waterorigin, tv(-15,-15,0), tv(15,15,0), nodes[n2].origin, NULL, MASK_NODESOLID );
	if ( trace.fraction < 1.0 )
		return LINK_INVALID;

	return LINK_WATERJUMP;
}


//==========================================
// AI_GravityBoxStep
// move the box one step for walk movetype
//==========================================
int AI_GravityBoxStep( vec3_t origin, float scale, vec3_t destvec, vec3_t neworigin, vec3_t mins, vec3_t maxs )
{
	trace_t	trace;
	vec3_t	v1, v2, forward, up, angles, movedir;
	int		movemask = 0;
	int		eternalfall = 0;
	float	xzdist, xzscale;
	float	ydist, yscale;
	float	dist;

	trace = gi.trace( origin, mins, maxs, origin, LINKS_PASSENT, MASK_NODESOLID );
	if ( trace.startsolid )
	{
	//	gi.dprintf ("AI_GravityBoxStep startsolid\n");
		return LINK_INVALID;
	}

	VectorSubtract( destvec, origin, movedir);
	VectorNormalize( movedir );
	vectoangles ( movedir, angles );

	//remaining distance in planes
	if ( scale < 1 )
		scale = 1;

	dist = AI_Distance( origin, destvec );
	if ( scale > dist )
		scale = dist;

	xzscale = scale;
	xzdist = AI_Distance( tv(origin[0], origin[1], destvec[2]), destvec );
	if ( xzscale > xzdist )
		xzscale = xzdist;

	yscale = scale;
	ydist = AI_Distance( tv(0,0,origin[2]), tv(0,0,destvec[2]) );
	if ( yscale > ydist )
		yscale = ydist;


	//float move step
	if ( gi.pointcontents( origin ) & MASK_WATER )
	{
		angles[ROLL] = 0;
		AngleVectors( angles, forward, NULL, up );


		VectorMA( origin, scale, movedir, neworigin );
		trace = gi.trace( origin, mins, maxs, neworigin, LINKS_PASSENT, MASK_NODESOLID);
		if ( trace.startsolid || trace.fraction < 1.0 )
			VectorCopy( origin, neworigin );	//update if valid

		if ( VectorCompare(origin, neworigin) )
		{
		//	gi.dprintf ("AI_GravityBoxStep origin = neworigin\n");
			return LINK_INVALID;
		}

		if ( gi.pointcontents( neworigin ) & MASK_WATER )
			return LINK_WATER;

		//jal: Actually GravityBox can't leave water.
		//return INVALID and WATERJUMP, so it can validate the rest outside
		//gi.dprintf ("AI_GravityBoxStep  can't leave water  ");
		return (LINK_INVALID|LINK_WATERJUMP);
	}

	//gravity step

	angles[PITCH] = 0;
	angles[ROLL] = 0;
	AngleVectors( angles, forward, NULL, NULL );
	VectorNormalize( forward );

	// try moving forward
	VectorMA( origin, xzscale, forward, neworigin );
	trace = gi.trace( origin, mins, maxs, neworigin, LINKS_PASSENT, MASK_NODESOLID );
	if ( trace.fraction == 1.0 ) //moved
	{
		movemask |= LINK_MOVE;
		goto droptofloor;

	}
	else
	{	//bloqued, try stepping up
		VectorCopy( origin, v1 );
		VectorMA( v1, xzscale, forward, v2 );

		//gi.dprintf ("%s %s\n", vtos(v1), vtos(v2));
		for( ; v1[2] < origin[2] + AI_JUMPABLE_HEIGHT; v1[2] += scale, v2[2] += scale )
		{
			//gi.dprintf("hi\n");
			trace = gi.trace( v1, mins, maxs, v2, LINKS_PASSENT, MASK_NODESOLID|MASK_AISOLID );
			if ( !trace.startsolid && trace.fraction == 1.0 )
			{
				VectorCopy( v2, neworigin );
				if ( origin[2] + AI_STEPSIZE > v2[2] )
					movemask |= LINK_STAIRS;
				else
					movemask |= LINK_JUMP;

				goto droptofloor;
			}
		}

		//still failed, try slide move
		VectorMA( origin, xzscale, forward, neworigin );
		trace = gi.trace( origin, mins, maxs, neworigin, LINKS_PASSENT, MASK_NODESOLID);
		//	gi.dprintf ("%f %f %f \n", trace.plane.normal[0], trace.plane.normal[1], trace.plane.normal[2]);
		if ( trace.plane.normal[2] < 0.5 && trace.plane.normal[2] >= -0.4 )
		{
			VectorCopy( trace.endpos, neworigin );
			VectorCopy( trace.plane.normal, v1 );
			v1[2] = 0;
			VectorNormalize( v1 );
			VectorMA( neworigin, xzscale, v1, neworigin );

			//if new position is closer to destiny, might be valid
			if ( AI_Distance( origin, destvec ) > AI_Distance( neworigin, destvec ) )
			{
				trace = gi.trace( trace.endpos, mins, maxs, neworigin, LINKS_PASSENT, MASK_NODESOLID );
				if ( !trace.startsolid && trace.fraction == 1.0 )
					goto droptofloor;
			}
		}

		VectorCopy( origin, neworigin );
	//	gi.dprintf ("AI_GravityBoxStep  can't step up \n");
		return LINK_INVALID;
	}

droptofloor:

	while(eternalfall < 20000)
	{
		if ( gi.pointcontents(neworigin) & MASK_WATER ) {

			if ( origin[2] > neworigin[2] + AI_JUMPABLE_HEIGHT )
				movemask |= LINK_FALL;
			else if ( origin[2] > neworigin[2] + AI_STEPSIZE )
				movemask |= LINK_STAIRS;

			return movemask;
		}

		trace = gi.trace( neworigin, mins, maxs, tv(neworigin[0], neworigin[1], (neworigin[2] - AI_STEPSIZE)), LINKS_PASSENT, MASK_NODESOLID );
		if ( trace.startsolid )
		{
		//	gi.dprintf ("AI_GravityBoxStep: Trace startsolid in droptofloor\n");
			return LINK_INVALID;
		//	gi.error("AI_GravityBoxStep: Trace startsolid in droptofloor\n");
		}

		VectorCopy( trace.endpos, neworigin );
		if ( trace.fraction < 1.0 )
		{
			if ( origin[2] > neworigin[2] + AI_JUMPABLE_HEIGHT )
				movemask |= LINK_FALL;
			else if ( origin[2] > neworigin[2] + AI_STEPSIZE )
				movemask |= LINK_STAIRS;

			if ( VectorCompare( origin, neworigin ) )
			{
			//	gi.dprintf ("AI_GravityBoxStep origin = neworigin\n");
				return LINK_INVALID;
			}

			return movemask;
		}

		eternalfall++;
	}

//	gi.dprintf ("AI_GravityBoxStep: ETERNAL FALL\n");
//	gi.error ("ETERNAL FALL\n");
	return LINK_INVALID;	//jabot092
}

//==========================================
// AI_RunGravityBox
// move a box along the link
//==========================================
int AI_RunGravityBox( int n1, int n2 )
{
	int			move;
	int			movemask = 0;
	float		movescale = 4; //faf: was 8
	trace_t		trace;
	vec3_t		boxmins, boxmaxs;
	vec3_t		o1;
	vec3_t		v1;
	//qboolean	crouched = false;
	int			eternalcount = 0;


	if ( n1 == n2 )
		return LINK_INVALID;

	//set up box
	VectorSet( boxmins, -15, -15, -24 );
	VectorSet( boxmaxs, 15, 15, 32 );

	//try some shortcuts before

	//water link shortcut
	if ( gi.pointcontents(nodes[n1].origin) & MASK_WATER &&
		gi.pointcontents(nodes[n2].origin) & MASK_WATER &&
		AI_VisibleOrigins(nodes[n1].origin, nodes[n2].origin) )
		return LINK_WATER;
	//waterjump link
	if ( gi.pointcontents(nodes[n1].origin) & MASK_WATER &&
		!(gi.pointcontents(nodes[n2].origin) & MASK_WATER) &&
		AI_Distance(nodes[n1].origin, nodes[n2].origin) < NODE_DENSITY )
	{
		if ( AI_IsWaterJumpLink( n1, n2 ) == LINK_WATERJUMP )
			return LINK_WATERJUMP;
	}

	//proceed

	//put box at first node
	VectorCopy( nodes[n1].origin, o1 );
	trace = gi.trace( o1, boxmins, boxmaxs, o1, LINKS_PASSENT, MASK_NODESOLID );
	if ( trace.startsolid )
	{
		//try crouched
		boxmaxs[2] = 8;
		trace = gi.trace( o1, boxmins, boxmaxs, o1, LINKS_PASSENT, MASK_NODESOLID );
		if ( trace.startsolid )
			return LINK_INVALID;


		//crouched = true;
		movemask |= LINK_CROUCH;
	}

	//start moving the box to o2
	while(eternalcount < 20000000)
	{
		//gi.dprintf ("%s   %s\n", vtos(o1), vtos(v1));
		move = AI_GravityBoxStep( o1, movescale, nodes[n2].origin, v1, boxmins, boxmaxs);
		if ( move & LINK_INVALID && !(movemask & LINK_CROUCH)/*!crouched*/ )
		{
			//retry crouched
			boxmaxs[2] = 8;
			//crouched = true;
			movemask |= LINK_CROUCH;
			move = AI_GravityBoxStep( o1, movescale, nodes[n2].origin, v1, boxmins, boxmaxs);
		}

		if ( move & LINK_INVALID )
		{
			//gravitybox can't reach waterjump links. So, check them here
			if ( move & LINK_WATERJUMP ) {
				if ( AI_IsWaterJumpLink( n1, n2 ) == LINK_WATERJUMP )
					return LINK_WATERJUMP;
			}
			return ( movemask|move );
		}

		//next
		movemask |= move;
		VectorCopy( v1, o1 );

		//check for reached
		if ( AI_Distance( o1, nodes[n2].origin ) < 24 && AI_VisibleOrigins( o1, nodes[n2].origin ) )
		{
			movemask |= move;
			return movemask;
		}

		eternalcount++;
	}

	gi.error ("ETERNAL COUNT\n"); //should never get here
	return 0;
}

//==========================================
// AI_GravityBoxToLink
// move a box along the link
//==========================================
int AI_GravityBoxToLink(int n1, int n2)
{
	int		link;

	//find out the link move type against the world
	link = AI_RunGravityBox( n1, n2 );

	//don't fall to JUMPAD nodes, or will be sent up again
	if ( (nodes[n2].flags & NODEFLAGS_JUMPPAD) && (link & LINK_FALL) )
		return LINK_INVALID;

	//simplify
	if ( link & LINK_INVALID )
		return LINK_INVALID;

	if ( link & LINK_CLIMB )
		return LINK_INVALID; // No actual bot is able to climb

	if ( link & LINK_WATERJUMP )
		return LINK_WATERJUMP;

	if ( link == LINK_WATER || link == (LINK_WATER|LINK_CROUCH) )	//only pure flags
		return LINK_WATER;

	if ( link & LINK_CROUCH )
		return LINK_CROUCH;

	if ( link & LINK_JUMP )
		return LINK_JUMP;	//there are simple ledge jumps only

	if ( link & LINK_FALL )
		return LINK_FALL;

	if ( link & LINK_STAIRS )
		return LINK_STAIRS;

	return LINK_MOVE;
}


//==========================================
// AI_FindFallOrigin
// helper for AI_IsJumpLink
//==========================================
int	AI_FindFallOrigin( int n1, int n2, vec3_t fallorigin )
{
	int			move;
	int			movemask = 0;
	float		movescale = 8;
	trace_t		trace;
	vec3_t		boxmins, boxmaxs;
	vec3_t		o1;
	vec3_t		v1;
	int			eternalcount = 0;


	if ( n1 == n2 )
		return LINK_INVALID;

	//set up box
	VectorSet( boxmins, -15, -15, -24 );
	VectorSet( boxmaxs, 15, 15, 32 );

	//put box at first node
	VectorCopy( nodes[n1].origin, o1 );
	trace = gi.trace( o1, boxmins, boxmaxs, o1, LINKS_PASSENT, MASK_NODESOLID );
	if ( trace.startsolid )
		return LINK_INVALID;

	//moving the box to o2 until falls. Keep last origin before falling
	while(1)
	{
		move = AI_GravityBoxStep( o1, movescale, nodes[n2].origin, v1, boxmins, boxmaxs);

		if ( move & LINK_INVALID )
			return LINK_INVALID;

		movemask |= move;

		if ( move & LINK_FALL ) {
			VectorCopy( o1, fallorigin );
			return LINK_FALL;
		}

		VectorCopy( v1, o1 );	//next step

		//check for reached ( which is invalid in this case )
		if ( AI_Distance( o1, nodes[n2].origin ) < 24 && AI_VisibleOrigins( o1, nodes[n2].origin ) ) {
			return LINK_INVALID;
		}

		eternalcount++;
		if (eternalcount > 200000000 )
			gi.error ("ETERNAL COUNT\n");
	}

	return LINK_INVALID;
}


//==========================================
// AI_LadderLink_FindUpperNode
// finds upper node in same ladder, if any
//==========================================
int AI_LadderLink_FindUpperNode( int node )
{
	int		i = INVALID;
	int		j;
	vec3_t	eorg;
	float	xzdist;
	int		candidate = INVALID;

	for( i=0; i<nav.num_nodes; i++ )
	{
		if ( i == node )
			continue;

		if ( !(nodes[i].flags & NODEFLAGS_LADDER) )
			continue;

		//same ladder
		for (j=0 ; j<2 ; j++)
			eorg[j] = nodes[i].origin[j] - nodes[node].origin[j];
		eorg[2] = 0; //ignore height
		xzdist = VectorLength(eorg);
		if (xzdist > 8)	//not in our ladder
			continue;

		if ( nodes[node].origin[2] > nodes[i].origin[2] )	//below
			continue;

		if ( candidate == INVALID ) {	//first found
			candidate = i;
			continue;
		}

		//shorter is better
		if ( nodes[i].origin[2] - nodes[node].origin[2] < nodes[candidate].origin[2] - nodes[node].origin[2] )
			candidate = i;
	}

//	if ( candidate != -1 )
//		Com_Printf( "LADDER: FOUND upper node in ladder\n");

	return candidate;
}

//==========================================
// AI_LadderLink_FindLowerNode
// finds lower node in same ladder, if any
//==========================================
int AI_LadderLink_FindLowerNode( int node )
{
	int		i = INVALID;
	int		j;
	vec3_t	eorg;
	float	xzdist;
	int		candidate = INVALID;

	for( i=0; i<nav.num_nodes; i++ )
	{
		if ( i == node )
			continue;

		if ( !(nodes[i].flags & NODEFLAGS_LADDER) )
			continue;

		//same ladder
		for (j=0 ; j<2 ; j++)
			eorg[j] = nodes[i].origin[j] - nodes[node].origin[j];
		eorg[2] = 0; //ignore height
		xzdist = VectorLength(eorg);
		if (xzdist > 8)	//not in our ladder
			continue;

		if ( nodes[i].origin[2] > nodes[node].origin[2] )	//above
			continue;

		if ( candidate == INVALID ) {	//first found
			candidate = i;
			continue;
		}

		//shorter is better
		if ( nodes[node].origin[2] - nodes[i].origin[2] < nodes[node].origin[2] - nodes[candidate].origin[2] )
			candidate = i;
	}

//	if ( candidate != -1 )
//		Com_Printf( "LADDER: FOUND lower node in ladder\n");

	return candidate;
}

//==========================================
// AI_IsLadderLink
// interpretation of this link
//==========================================
int AI_IsLadderLink( int n1, int n2 )
{
	int	j;
	vec3_t	eorg;
	float	xzdist;

	for (j=0 ; j<2 ; j++)
		eorg[j] = nodes[n2].origin[j] - nodes[n1].origin[j];
	eorg[2] = 0; //ignore height

	xzdist = VectorLength(eorg);

	if (xzdist < 0)
		xzdist = -xzdist;

	//if both are ladder nodes
	if ( nodes[n1].flags & NODEFLAGS_LADDER && nodes[n2].flags & NODEFLAGS_LADDER )
	{
		int	candidate = AI_LadderLink_FindUpperNode( n1 );
		if ( candidate != n2 )
			return LINK_INVALID;

		return LINK_LADDER;
	}

	//if only second is ladder node
	if ( !(nodes[n1].flags & NODEFLAGS_LADDER) && nodes[n2].flags & NODEFLAGS_LADDER )
	{
		int candidate;

		if ( nodes[n1].flags & NODEFLAGS_WATER )
		{
			if ( AI_VisibleOrigins(nodes[n1].origin, nodes[n2].origin) ) {
				if ( nodes[n2].flags & NODEFLAGS_WATER )
					return LINK_WATER;
				else
					return LINK_LADDER;
			}

			return LINK_INVALID;
		}

		//only allow linking to the bottom ladder node from outside the ladder
		candidate = AI_LadderLink_FindLowerNode( n2 );
		if ( candidate == -1 ) {
			int link;
			if ( nodes[n2].flags & NODEFLAGS_WATER ) {
				link = AI_RunGravityBox( n1, n2 );
				if ( link & LINK_INVALID )
					return LINK_INVALID;
				return LINK_WATER;
			} else {
				return AI_GravityBoxToLink( n1, n2 );
			}
		}

		return LINK_INVALID;
	}

	//if only first is ladder node
	if ( nodes[n1].flags & NODEFLAGS_LADDER && !(nodes[n2].flags & NODEFLAGS_LADDER) )
	{
		int	candidate;
		int	link;

		//if it has a upper ladder node, it can only link to it
		candidate = AI_LadderLink_FindUpperNode( n1 );
		if ( candidate != INVALID )
			return LINK_INVALID;

		if ( AI_Distance(nodes[n1].origin, nodes[n2].origin) > (NODE_DENSITY*0.8) )
			return LINK_INVALID;

		link = AI_RunGravityBox( n2, n1 );	//try to reach backwards
		if ( link & LINK_INVALID || link & LINK_FALL )
			return LINK_INVALID;

		return LINK_LADDER;
	}

	return LINK_INVALID;
}

//==========================================
// AI_FindLinkType
// interpretation of this link
//==========================================
int AI_FindLinkType(int n1, int n2)
{
	if ( n1 == n2 || n1 == INVALID || n2 == INVALID )
		return LINK_INVALID;

	if ( AI_PlinkExists( n1, n2 ))
		return LINK_INVALID; //already saved


	//ignore server links
	if ( nodes[n1].flags & NODEFLAGS_SERVERLINK || nodes[n2].flags & NODEFLAGS_SERVERLINK )
		return LINK_INVALID; // they are added only by the server at dropping entity nodes

	//LINK_LADDER
	if ( nodes[n1].flags & NODEFLAGS_LADDER || nodes[n2].flags & NODEFLAGS_LADDER )
		return AI_IsLadderLink( n1, n2 );

	//find out the link move type against the world
	return AI_GravityBoxToLink( n1, n2 );
}


//==========================================
// AI_IsJumpLink
// interpretation of this link
//==========================================
int AI_IsJumpLink(int n1, int n2)
{
	int		link;
	int climblink = 0;

	if ( n1 == n2 || n1 == INVALID || n2 == INVALID )
		return LINK_INVALID;

	//ignore server links
	if ( nodes[n1].flags & NODEFLAGS_SERVERLINK || nodes[n2].flags & NODEFLAGS_SERVERLINK )
		return LINK_INVALID; // they are added only by the server at dropping entity nodes

	//LINK_LADDER
	if ( nodes[n1].flags & NODEFLAGS_LADDER || nodes[n2].flags & NODEFLAGS_LADDER )
		return LINK_INVALID;

	//can't jump if begins inside water
	if ( nodes[n1].flags & NODEFLAGS_WATER )
		return LINK_INVALID;

	//find out the link move type against the world
	link = AI_RunGravityBox( n1, n2 );

	if ( !(link & LINK_INVALID) )	// recheck backwards for climb links
		return LINK_INVALID;

	if ( AI_PlinkExists( n2, n1 ))
		climblink = AI_PlinkMoveType( n2, n1 );
	else
		climblink = AI_RunGravityBox( n2, n1 );

	if ( climblink & LINK_FALL ) {
		climblink &= ~LINK_FALL;
		link &= ~LINK_INVALID;
		link = (link|climblink|LINK_CLIMB);
	}

	//see if we can jump it
	if ( link & LINK_CLIMB && link & LINK_FALL )
	{
		vec3_t fo1, fo2;
		int		link;
		float	dist;
		float	heightdiff;

		VectorSet( fo1, 0,0,0 );
		VectorSet( fo2, 0,0,0 );

		link = AI_FindFallOrigin( n1, n2, fo1 );
		if ( !(link & LINK_FALL) )
			return LINK_INVALID;

		link = AI_FindFallOrigin( n2, n1, fo2 );
		if ( !(link & LINK_FALL) )
			return LINK_INVALID;

		//reachable? (faster)
		if ( !AI_VisibleOrigins( fo1, fo2 ) )
			return LINK_INVALID;

		if ( fo2[2] > fo1[2] + AI_JUMPABLE_HEIGHT )	//n1 is just too low
			return LINK_INVALID;

		heightdiff = fo2[2] - fo1[2];	//if n2 is higher, shorten xzplane dist
		if ( heightdiff < 0 )
			heightdiff = 0;

		//xzplane dist is valid?
		fo2[2] = fo1[2];
		dist = AI_Distance( fo1, fo2 );
		if ( (dist + heightdiff) < AI_JUMPABLE_DISTANCE && dist > 24 )
			return LINK_JUMP;
	}

	return LINK_INVALID;
}


//==========================================
// AI_LinkCloseNodes_JumpPass
// extended radius for jump links.
// Standard movetypes nodes must be stored before calling this one
//==========================================
int AI_LinkCloseNodes_JumpPass( int start )
{
	int			n1, n2;
	int			count = 0;
	float		pLinkRadius = NODE_DENSITY*2;
	qboolean	ignoreHeight = true;
	int			linkType;

	if ( nav.num_nodes < 1 )
		return 0;

	//do it for everynode in the list
	for( n1 = start; n1<nav.num_nodes; n1++ )
	{
		n2 = 0;
		n2 = AI_findNodeInRadius ( 0, nodes[n1].origin, pLinkRadius, ignoreHeight);

		while (n2 != -1)
		{
			if ( n1 != n2 && !AI_PlinkExists( n1, n2 ) )
			{
				linkType = AI_IsJumpLink( n1, n2 );
				if ( linkType == LINK_JUMP && pLinks[n1].numLinks < NODES_MAX_PLINKS )
				{
					int cost;
					//make sure there isn't a good 'standard' path for it
					cost = AI_FindCost( n1, n2, (LINK_MOVE|LINK_STAIRS|LINK_FALL|LINK_WATER|LINK_WATERJUMP|LINK_CROUCH) );
					if ( cost == -1 || cost > 4 ) {
						if ( AI_AddLink(n1, n2, LINK_JUMP) )
							count++;
					}
				}
			}
			//next
			n2 = AI_findNodeInRadius ( n2, nodes[n1].origin, pLinkRadius, ignoreHeight);
		}
	}

	return count;
}


//==========================================
// AI_LinkCloseNodes
// track the nodes list and find close nodes around. Link them if possible
//==========================================
int AI_LinkCloseNodes( void )
{
	int			n1, n2;
	int			count = 0;
	float		pLinkRadius = NODE_DENSITY*1.5;
	qboolean	ignoreHeight = true;

	//do it for everynode in the list
	for( n1=0; n1<nav.num_nodes; n1++ )
	{
		n2 = 0;
		n2 = AI_findNodeInRadius ( 0, nodes[n1].origin, pLinkRadius, ignoreHeight);

		while (n2 != -1)
		{
			if ( AI_AddLink( n1, n2, AI_FindLinkType(n1, n2) ))
				count++;

			n2 = AI_findNodeInRadius ( n2, nodes[n1].origin, pLinkRadius, ignoreHeight);
		}
	}
	return count;
}

