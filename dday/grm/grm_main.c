/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/grm/grm_main.c,v $
 *   $Revision: 1.11 $
 *   $Date: 2002/07/23 21:11:37 $
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
  German.c

  vjj  03/29/98

  We are using the dll template that was created for the multiple dll 
 modification.
*/

#include "grm_main.h"
#include "grm_classes.h"

void GRM_UserPrecache(void) 
{
	gi.imageindex("scope_grm");
	gi.imageindex("victory_grm");

	gi.soundindex("grm/victory.wav");

	gi.modelindex("players/grm/tris.md2");

	//
	// V-WEPS
	//

	// GRM 
	gi.modelindex("players/grm/w_p38.md2");
	gi.modelindex("players/grm/w_m98k.md2");
	gi.modelindex("players/grm/w_mp40.md2");
	gi.modelindex("players/grm/w_mp43.md2");
	gi.modelindex("players/grm/w_mg42.md2");
	gi.modelindex("players/grm/w_panzer.md2");
	gi.modelindex("players/grm/w_m98ks.md2");
	gi.modelindex("players/grm/a_masher.md2");


	//faf:  airstrike plane
	gi.modelindex("models/ships/grmplane/tris.md2");
	gi.soundindex("airstrike/stuka.wav");


}
