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


#include "ai_nodes_shared.h"

//=============================================================
//
//					GAME NODES LOCAL
//
//=============================================================


typedef struct nav_item_s
{
	int			item;
	float		weight;
	edict_t		*ent;
	int			node;

} nav_item_t;

typedef struct nav_ents_s
{
	edict_t		*ent;
	int			node;

} nav_ents_t;

typedef struct nav_botroam_s
{
	int			node;
	float		weight;

} nav_broam_t;

typedef struct nav_path_s
{
	int		next;		//next node
	int		cost;
	int		moveTypes;	//type of movements required to run along this path (flags)

} nav_path_t;

// Knightmare- made these vars extern to fix compile on GCC
extern nav_plink_t pLinks[MAX_NODES];		// pLinks array
extern nav_node_t nodes[MAX_NODES];		// nodes array

typedef struct
{
//	qboolean	loaded;
	int			loaded;
	int			num_nodes;			// total number of nodes
	
	int			num_items;			// number of items known to navigation code
	nav_item_t	items[MAX_EDICTS];	//keeps track of items related to nodes

	int			num_ents;
	nav_ents_t	ents[MAX_EDICTS];	//plats, etc

	int			num_broams;
	nav_broam_t	broams[MAX_BOT_ROAMS];	//list of nodes wich are botroams

} ai_navigation_t;

// Knightmare- made this var extern to fix compile on GCC
extern ai_navigation_t	nav;

