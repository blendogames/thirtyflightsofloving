/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/rus/rus_main.c,v $
 *   $Revision: 1.9 $
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
  German.c

  vjj  03/29/98

  We are using the dll template that was created for the multiple dll 
 modification.
*/

#include "rus_main.h"
#include "rus_classes.h"

void RUS_UserPrecache(void) 
{
	gi.imageindex("scope_rus");
	gi.imageindex("victory_rus");

	gi.soundindex("rus/victory.wav");

	gi.modelindex("players/rus/tris.md2");

	//
	// V-WEPS
	//

	
	// rus 
	gi.modelindex("players/rus/w_tt33.md2");
	gi.modelindex("players/rus/w_m9130.md2");
	gi.modelindex("players/rus/w_ppsh41.md2");
	gi.modelindex("players/rus/w_pps43.md2");
	gi.modelindex("players/rus/w_dpm.md2");
	gi.modelindex("players/rus/w_panzer.md2");
	gi.modelindex("players/rus/w_m9130s.md2");
	gi.modelindex("players/rus/a_f1grenade.md2");

}

