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


//=============================================================
//
//					NODES SHARED (game/cgame)
//
//=============================================================

#define MAX_NODES			2048		//jalToDo: needs dynamic alloc (big terrain maps)
#define NODE_DENSITY		128			// Density setting for nodes
#define INVALID				-1
#define	NODES_MAX_PLINKS	16
#define	NAV_FILE_VERSION	11
#define NAV_FILE_EXTENSION	"nav"
#define AI_MOD_FOLDER		"dday"
#define AI_NODES_FOLDER		"navigation"

#define MASK_NODESOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW)
#define MASK_AISOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_MONSTER|CONTENTS_DEADMONSTER|CONTENTS_MONSTERCLIP)

#define	AI_STEPSIZE				18
#define AI_JUMPABLE_HEIGHT		60//120//34//60//120//34
#define AI_JUMPABLE_DISTANCE	140

// node flags
#define	NODEFLAGS_WATER				0x00000001
#define	NODEFLAGS_LADDER			0x00000002 
#define NODEFLAGS_SERVERLINK		0x00000004	//plats, doors, teles. Only server can link 2 nodes with this flag
#define	NODEFLAGS_FLOAT				0x00000008	//don't drop node to floor ( air & water )
//#define	NODEFLAGS_ITEM			0x00000010	//jal remove me
#define	NODEFLAGS_BOTROAM			0x00000020
#define NODEFLAGS_JUMPPAD			0x00000040	// jalfixme: add NODEFLAGS_REACHATTOUCH
#define NODEFLAGS_JUMPPAD_LAND		0x00000080
#define	NODEFLAGS_PLATFORM			0x00000100
#define	NODEFLAGS_TELEPORTER_IN		0x00000200	// jalfixme: add NODEFLAGS_REACHATTOUCH
#define NODEFLAGS_TELEPORTER_OUT	0x00000400
#define NODEFLAGS_REACHATTOUCH		0x00000800

#define NODE_ALL					0x00001000

// links types (movetypes required to run node links)
#define	LINK_MOVE				0x00000001
#define	LINK_STAIRS				0x00000002 
#define LINK_FALL				0x00000004	
#define	LINK_CLIMB				0x00000008	
#define	LINK_TELEPORT			0x00000010
#define	LINK_PLATFORM			0x00000020
#define LINK_JUMPPAD			0x00000040
#define LINK_WATER				0x00000080
#define	LINK_WATERJUMP			0x00000100
#define	LINK_LADDER				0x00000200
#define LINK_JUMP				0x00000400
#define LINK_CROUCH				0x00000800

#define LINK_INVALID			0x00001000


typedef struct nav_plink_s
{
	int			numLinks;
	int			nodes[NODES_MAX_PLINKS];
	int			dist[NODES_MAX_PLINKS];
	int			moveType[NODES_MAX_PLINKS];

} nav_plink_t;


typedef struct nav_node_s
{
	vec3_t		origin;
	int			flags;
	int			area;

} nav_node_t;
