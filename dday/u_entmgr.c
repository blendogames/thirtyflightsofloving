/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/u_entmgr.c,v $
 *   $Revision: 1.7 $
 *   $Date: 2002/06/04 19:49:50 $
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
 
 /*
  u_entmgr.c

  vjj

  This file contains the functions that a user would call to insert their 
  items into the itemlist[] and entities into the spawns[].

*/

#include "g_local.h"


#define EMPTY_NAME   "*none*"


//these are the Entity functions
//These functions allow the user to insert and remove types of entities from 
//the spawns[].
//we need to be able to access the spawns[]

extern spawn_t spawns[];




//add an item to the itemlist array. Note that once you request a spot, it
//forever increases the number of items in the list,  even if you remove the 
//weapon.
//Note that most of the items in the item struct are pointers. No copy of the
//data is performed. You must maintain a copy of the data in your dll! 
//You also have to pass in a spawn_t struct that tells how to spawn your
//item.
gitem_t *InsertItem(gitem_t *it, spawn_t *spawnInfo)
{
    int i, inc_items;
    gitem_t *spot;
    spawn_t *spspot, *s;

    inc_items = 0;
    spot = NULL;
      //first, we want to find a place for the item.
    for(i=1;i<game.num_items && !spot;i++)
        if(itemlist[i].classname && !Q_stricmp(itemlist[i].classname,EMPTY_NAME))
            spot = &itemlist[i];


      //if we didn't find an empty slot, see if we can create one
    if(!spot && i < MAX_ITEMS)
        {
        spot = &itemlist[i];

                inc_items = 1;

        }

    if(spot)
    {
          //found a place in the item list, need to see if we can insert
          //the spawn function befrore we can insert item
        spspot = NULL;

        for(s=spawns, i=0; i<MAX_EDICTS && s->name;i++,s++)
            if(s->name && !Q_stricmp(s->name,EMPTY_NAME))
                spspot = s;

          //if we didn't find an empty slot, see if we can create one
        if(!spspot && !s->name && i < (MAX_EDICTS - 1)) //want to leave {NULL,NULL}
            spspot = s;


      //OK, fill spot in with the stuff the user sent in
        if(spspot)
        {
            *spspot = *spawnInfo;

            *spot = *it;            //OK, fill spot in with the stuff the user sent in
                        if (inc_items) game.num_items++;
        }
    }
    
	//gi.dprintf("InsertItem: %s -> %s\n", it->classname, it->pickup_name);

	PrecacheItem (it); //pbowens: precache team items
    return spot;
}




