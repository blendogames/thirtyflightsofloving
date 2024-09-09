/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/usa/usa_main.c,v $
 *   $Revision: 1.9 $
 *   $Date: 2002/07/23 19:12:49 $
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

#include "usa_main.h"
#include "usa_classes.h"




/* 
   Initialization Function - called to set up the dll. This is usually
   called to set up mod specific global data.
*/

// pbowens: this is a new function initiated within the DLL for those team specific
//			game elements that arent items yet still need precaching (models)
void USA_UserPrecache(void) 
{

	gi.imageindex("scope_usa");
	gi.imageindex("victory_usa");

	gi.soundindex("usa/victory.wav");

	gi.modelindex("players/usa/tris.md2");



	gi.modelindex("players/usa/w_colt45.md2");
	gi.modelindex("players/usa/w_m1.md2");
	gi.modelindex("players/usa/w_thompson.md2");
	gi.modelindex("players/usa/w_bar.md2");
	gi.modelindex("players/usa/w_bhmg.md2");
	gi.modelindex("players/usa/w_bazooka.md2");
	gi.modelindex("players/usa/w_m1903.md2");
	gi.modelindex("players/usa/a_grenade.md2");

	// GENERIC
	gi.modelindex("players/usa/w_flame.md2");
	gi.modelindex("players/usa/w_morphine.md2");
	gi.modelindex("players/usa/w_knife.md2");
	gi.modelindex("players/usa/w_binoc.md2");


	//faf:  airstrike plane
	gi.modelindex("models/ships/usaplane/tris.md2");

}


