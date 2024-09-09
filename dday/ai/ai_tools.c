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
// AIDebug_ToogleBotDebug
//==========================================
void AIDebug_ToogleBotDebug(void)
{
/*	if (AIDevel.debugMode || !sv_cheats->integer )
	{
//		G_Printf ("BOT: Debug Mode Off\n");
		AIDevel.debugMode = false;
		return;
	}

	//Activate debug mode
	G_Printf ("\n======================================\n");
	G_Printf ("--==[ D E B U G ]==--\n");
	G_Printf ("======================================\n");
	G_Printf ("'addnode [nodetype]' -- Add [specified] node to players current location\n");
	G_Printf ("'movenode [node] [x y z]' -- Move [node] to [x y z] coordinates\n");
	G_Printf ("'findnode' -- Finds closest node\n");
	G_Printf ("'removelink [node1 node2]' -- Removes link between two nodes\n");
	G_Printf ("'addlink [node1 node2]' -- Adds a link between two nodes\n");
	G_Printf ("======================================\n\n");

	G_Printf ("BOT: Debug Mode On\n");

	AIDevel.debugMode = true;
*/
}


//==========================================
// AIDebug_SetChased
// Theorically, only one client
//	at the time will chase. Otherwise it will
//	be a really fucked up situation.
//==========================================
void AIDebug_SetChased(edict_t *ent)
{
/*	int i;
	AIDevel.chaseguy = NULL;
	AIDevel.debugChased = false;

	if (!AIDevel.debugMode || !sv_cheats->integer)
		return;

	//find if anyone is chasing this bot
	for(i=0;i<game.maxclients+1;i++)
	{
//		AIDevel.chaseguy = game.edicts + i + 1;
		AIDevel.chaseguy = g_edicts + i + 1;
		if(AIDevel.chaseguy->inuse && AIDevel.chaseguy->client){
			if( AIDevel.chaseguy->client->chase_target &&
				AIDevel.chaseguy->client->chase_target == ent)
				break;
		}
		AIDevel.chaseguy = NULL;
	}

	if (!AIDevel.chaseguy)
		return;

	AIDevel.debugChased = true;
*/
}



//=======================================================================
//							NODE TOOLS	
//=======================================================================



//==========================================
// AITools_DrawLine
// Just so I don't hate to write the event every time
//==========================================
void AITools_DrawLine(vec3_t origin, vec3_t dest)
{
/*
	edict_t		*event;
	
	event = G_SpawnEvent ( EV_BFG_LASER, 0, origin );
	event->svflags = SVF_FORCEOLDORIGIN;
	VectorCopy ( dest, event->s.origin2 );
*/
}


//==========================================
// AITools_DrawPath
// Draws the current path (floods as hell also)
//==========================================
static int	drawnpath_timeout;
void AITools_DrawPath(edict_t *self, int node_from, int node_to)
{
/*
	int			count = 0;
	int			pos = 0;

	//don't draw it every frame (flood)
	if (level.time < drawnpath_timeout)
		return;
	drawnpath_timeout = level.time + 4*FRAMETIME;

	if( self->ai.path->goalNode != node_to )
		return;

	//find position in stored path
	while( self->ai.path->nodes[pos] != node_from )
	{
		pos++;
		if( self->ai.path->goalNode == self->ai.path->nodes[pos] )
			return;	//failed
	}

	// Now set up and display the path
	while( self->ai.path->nodes[pos] != node_to && count < 32)
	{
		edict_t		*event;
		
		event = G_SpawnEvent ( EV_BFG_LASER, 0, nodes[self->ai.path->nodes[pos]].origin );
		event->svflags = SVF_FORCEOLDORIGIN;
		VectorCopy ( nodes[self->ai.path->nodes[pos+1]].origin, event->s.origin2 );		
		pos++;
		count++;
	}
*/
}

//==========================================
// AITools_ShowPlinks
// Draws lines from the current node to it's plinks nodes
//==========================================
static int	debugdrawplinks_timeout;
void AITools_ShowPlinks( void )
{
/*	int		current_node;
	int		plink_node;
	int		i;

	if (AIDevel.plinkguy == NULL)
		return;

	//don't draw it every frame (flood)
	if (level.time < debugdrawplinks_timeout)
		return;
	debugdrawplinks_timeout = level.time + 4*FRAMETIME;

	//do it
	current_node = AI_FindClosestReachableNode(AIDevel.plinkguy,NODE_DENSITY*3,NODE_ALL);
	if (!pLinks[current_node].numLinks)
		return;

	for (i=0; i<nav.num_items;i++){
		if (nav.items[i].node == current_node){
			if( !nav.items[i].ent->classname )
				G_CenterPrintMsg(AIDevel.plinkguy, "no classname");
			else
				G_CenterPrintMsg(AIDevel.plinkguy, "%s", nav.items[i].ent->classname);
			break;
		}
	}

	for (i=0; i<pLinks[current_node].numLinks; i++) 
	{
		plink_node = pLinks[current_node].nodes[i];
		AITools_DrawLine(nodes[current_node].origin, nodes[plink_node].origin);
	}
*/
}


//=======================================================================
//=======================================================================


//==========================================
// AITools_Frame
// Gives think time to the debug tools found
// in this archive (those witch need it)
//==========================================
void AITools_Frame(void)
{
	//debug
	if( AIDevel.showPLinks )
		AITools_ShowPlinks();
}



//removes nearest node
void SV_Remove_Node (edict_t *ent)
{
	float	dist;
	vec3_t	v;
	int	nearest;
	float nearest_distance;
	int i,j;

	if (sv_cheats->value == 0)
		return;

	nearest_distance = 9999999999;
	nearest = -1;
    for (i=1; i<MAX_NODES; i++)
	{
		VectorSubtract(ent->s.origin, nodes[i].origin, v); 
		dist = VectorLength(v);
		if (dist < nearest_distance)
		{
			nearest = i;
			nearest_distance = dist;
		}

	}

	if (nearest == -1)
		return;

	//replace this node with the last one and lower nav.num_nodes
/*	nodes[nearest].area = nodes[nav.num_nodes].area;
	nodes[nearest].flags = nodes[nav.num_nodes].flags;
	VectorCopy (nodes[nav.num_nodes].origin ,nodes[nearest].origin);
	if (nearest != nav.num_nodes)
	{
		AI_UpdateNodeEdge (nav.num_nodes, nearest);
		nav.num_nodes--;
	}
	else gi.dprintf ("can't remove the very last node\n");*/

	for (j=0; j<pLinks[nearest].numLinks; j++)
	{
			gi.dprintf("KILLING LINK\n");
			pLinks[nearest].moveType[j] = LINK_INVALID;
	}
	VectorSet(nodes[nearest].origin, -9999,-9999,-9999);

/*	for (i=0; i< MAX_NODES; i++)
	{
		for (j=0; j<pLinks[i].numLinks; j++)
		{
			if( pLinks[i].nodes[j] == nearest )
			{
				gi.dprintf("KILLING LINK\n");
				pLinks[i].moveType[j] = LINK_INVALID;
			}
		}
	}
*/

}



void Show_Nodes_Think (edict_t *ent)
{
	vec3_t	last_movedir;

	vec3_t	start,end;
	float	dist;
	vec3_t	v;
	int	nearest;
	float nearest_distance;
	int i;
	edict_t	*head;

	nearest_distance = 999999;
	nearest=-1;
	for (i=0; i<MAX_NODES; i++)
	{
		VectorSubtract(ent->owner->s.origin, nodes[i].origin, v);
		dist = VectorLength(v);
		if (dist < nearest_distance)
		{
			nearest = i;
			nearest_distance = dist;
		}

	}

	if (nearest == -1)
	{
		ent->think =G_FreeEdict;
		ent->nextthink = level.time+.1;
		gi.dprintf("No nodes!\n");
	}
	

	if (ent->mass >= pLinks[nearest].numLinks)	
	{
		ent->mass = 0;
	}

	if (pLinks[nearest].moveType[ent->mass]== LINK_INVALID)
		ent->s.skinnum = 0xf2f2f0f0; // red
	else if (pLinks[nearest].moveType[ent->mass] == LINK_JUMP)
		ent->s.skinnum = 0xf3f3f1f1; //blue
	else
		ent->s.skinnum = 0xd0d1d2d3; //green

	VectorCopy (nodes[nearest].origin, start);
	VectorCopy (nodes[pLinks[nearest].nodes[ent->mass]].origin, end);

//	gi.dprintf ("%i\n", pLinks[nearest].moveType[ent->mass]);


	//put a head at the end of the link.
	head = G_Spawn();
	head->s.skinnum = 0;
	VectorCopy (end, head->s.origin);
	head->s.frame = 0;
	gi.setmodel (head, "models/mapmodels/faf/skull2.md2");
	VectorSet (head->mins, -16, -16, 0);
	VectorSet (head->maxs, 16, 16, 16);
	head->takedamage = DAMAGE_NO;
	head->solid = SOLID_NOT;
	head->s.effects = EF_GIB;
	head->s.sound = 0;
	head->flags |= FL_NO_KNOCKBACK;
	head->movetype = MOVETYPE_NONE;
	head->think = G_FreeEdict;
	head->nextthink = level.time + .5;
	gi.linkentity (head);




	//gi.dprintf("%i %s, %s %s\n", ent->mass, vtos(start),vtos(end), vtos(nodes[pLinks[nearest].nodes[ent->mass]].origin)); 

/* slowly goes through every link on map	
//mass will be link # for current node
	if (!ent->mass)
		ent->mass = 1;

	if (ent->mass > pLinks[ent->count].numLinks || VectorCompare(nodes[pLinks[ent->count].nodes[ent->mass]].origin, vec3_origin))	{
		ent->count++;
		ent->mass = 1;
	}
	//count will be node #
	if (!ent->count || ent->count > nav.num_nodes)	{
		ent->count = 1;
		ent->mass = 1;
	}

	gi.dprintf("%i, %i, %s\n", ent->count, ent->mass, vtos(nodes[pLinks[ent->count].nodes[ent->mass]].origin)); 


	VectorCopy (nodes[ent->count].origin, start);
	VectorCopy (nodes[pLinks[ent->count].nodes[ent->mass]].origin, end);*/


	VectorCopy (start, ent->s.origin);

	VectorCopy (ent->movedir, last_movedir);
//	VectorMA (ent->mins, 0.5, ent->size, point);
	VectorSubtract (start, end, ent->movedir);
	VectorNormalize (ent->movedir);
//	if (!VectorCompare(ent->movedir, last_movedir))
		ent->spawnflags |= 0x80000000;



	VectorCopy (end, ent->s.old_origin);
	gi.linkentity (ent);
	ent->nextthink = level.time +.2;

	ent->mass++;



	

}



void Bot_Laser_Think (edict_t *ent)
{
	vec3_t	last_movedir;
	vec3_t	start,end;


	if (!ent->owner || !ent->owner->ai  || !ent->owner->client->resp.laser)
		return;

	if (ent->owner->ai->camp_targ < 0)
		VectorCopy (ent->owner->s.origin, start);
	else
		VectorCopy (camp_spots[ent->owner->ai->camp_targ].origin, start);


	VectorCopy (ent->owner->s.origin, end);


	VectorCopy (start, ent->s.origin);

	VectorCopy (ent->movedir, last_movedir);
	VectorSubtract (start, end, ent->movedir);
	VectorNormalize (ent->movedir);

	ent->spawnflags |= 0x80000000;

	VectorCopy (end, ent->s.old_origin);
	gi.linkentity (ent);
	ent->nextthink = level.time +.1;

	if (ent->owner->ai->state == BOT_STATE_WANDER)
		ent->s.skinnum = 0xf2f2f0f0;
	else
		ent->s.skinnum = 0xdcdddedf;

}



void ShowLinks(edict_t *ent)
{
	edict_t	*laser, *thrower, *blaser, *e;
	int i;

	if (sv_cheats->value == 0)
		return;

	if (ent->client->resp.laser)
		return;

	laser = G_Spawn();


	laser->movetype = MOVETYPE_NONE;
	laser->solid = SOLID_NOT;
	laser->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	laser->s.modelindex = 1;			// must be non-zero

	laser->s.frame = 4;

	laser->s.skinnum = 0xf2f2f0f0; // red

	VectorSet (laser->mins, -8, -8, -8);
	VectorSet (laser->maxs, 8, 8, 8);
	laser->spawnflags |= 0x80000001;
	laser->svflags &= ~SVF_NOCLIENT;
	VectorCopy (ent->s.origin,laser->s.origin);

	gi.linkentity (laser);

	laser->think = Show_Nodes_Think;
	laser->nextthink = level.time +.1;

	ent->client->resp.laser = laser;
	laser->owner = ent;



	//put a team flag over every campspot
	for (i=0; i< total_camp_spots; i++)
	{
		thrower = G_Spawn();
		thrower->s.skinnum = 0;
		VectorCopy (camp_spots[i].origin, thrower->s.origin);
		thrower->s.frame = 0;
		if (camp_spots[i].team == 1)
			//gi.setmodel (thrower, "models/weapons/grm/g_masher/tris.md2");
			gi.setmodel (thrower, "models/objects/banner2/tris.md2");
		else
			gi.setmodel (thrower, "models/objects/banner4/tris.md2");


		thrower->s.angles[1] = camp_spots[i].angle;

		VectorSet (thrower->mins, -16, -16, 0);
		VectorSet (thrower->maxs, 16, 16, 16);
		thrower->takedamage = DAMAGE_NO;
		thrower->solid = SOLID_NOT;
		thrower->s.effects = EF_GIB;
		thrower->s.sound = 0;
		thrower->flags |= FL_NO_KNOCKBACK;
		thrower->movetype = MOVETYPE_NONE;
		//thrower->think = G_FreeEdict;
		//thrower->nextthink = level.time + .5;
		gi.linkentity (thrower);

		if (camp_spots[i].type == CAMP_OBJECTIVE)
		{//gi.dprintf("ljldf\n");
			thrower->s.renderfx = RF_TRANSLUCENT;
		}
	}





	//give every bot a laser that points to its campspot
	for (i=0 ; i< maxclients->value ; i++)
	{
		e = g_edicts + 1 + i;
		if (!e->client)
			continue;
		if (!e->inuse || !e->ai)
			continue;
		if (e->client->resp.laser)
			continue;
	

		blaser = G_Spawn();

	 	blaser->movetype = MOVETYPE_NONE;
		blaser->solid = SOLID_NOT;
		blaser->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
		blaser->s.modelindex = 1;			// must be non-zero

		blaser->s.frame = 4;

		blaser->s.skinnum = 0xe0e1e2e3;

		VectorSet (blaser->mins, -8, -8, -8);
		VectorSet (blaser->maxs, 8, 8, 8);
		blaser->spawnflags |= 0x80000001;
		blaser->svflags &= ~SVF_NOCLIENT;
		VectorCopy (e->s.origin,blaser->s.origin);

		gi.linkentity (blaser);

		blaser->think = Bot_Laser_Think;
		blaser->nextthink = level.time +.1;

		e->client->resp.laser = blaser;
		blaser->owner = e;
	}

}