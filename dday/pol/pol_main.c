/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/pol/pol_main.c,v $
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

#include "pol_main.h"
#include "pol_classes.h"

void POL_UserPrecache(void) 
{
	gi.imageindex("scope_pol");
	gi.imageindex("victory_pol");

	gi.soundindex("pol/victory.wav");

	gi.modelindex("players/pol/tris.md2");

	//
	// V-WEPS
	//

	// POL 
	gi.modelindex("players/pol/w_vis.md2");
	gi.modelindex("players/pol/w_svt.md2");
//	gi.modelindex("players/pol/w_sten.md2");
	gi.modelindex("players/pol/w_mors.md2");
	gi.modelindex("players/pol/w_rkm.md2");
	gi.modelindex("players/pol/w_mg34.md2");
	gi.modelindex("players/pol/w_piat.md2");
	gi.modelindex("players/pol/w_m98ks.md2");
	gi.modelindex("players/pol/a_masher.md2");
	gi.modelindex("players/pol/w_sabre.md2");


	//faf:  airstrike plane
	gi.modelindex("models/ships/polplane/tris.md2");
//	gi.soundindex("airstrike/stuka.wav");

}

