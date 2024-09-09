/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/gbr/gbr_main.c,v $
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

#include "gbr_main.h"
#include "gbr_classes.h"

void GBR_UserPrecache(void) 
{

	gi.imageindex("scope_gbr");
	gi.imageindex("victory_gbr");

	gi.soundindex("gbr/victory.wav");

	gi.modelindex("players/gbr/tris.md2");

	//
	// V-WEPS
	//

	// GBR
	gi.modelindex("players/gbr/w_webley.md2");
	gi.modelindex("players/gbr/w_303.md2");
	gi.modelindex("players/gbr/w_sten.md2");
	gi.modelindex("players/gbr/w_bren.md2");
	gi.modelindex("players/gbr/w_vickers.md2");
	gi.modelindex("players/gbr/w_piat.md2");
	gi.modelindex("players/gbr/w_303s.md2");
	gi.modelindex("players/gbr/a_mills_bomb.md2");

	// GENERIC
	gi.modelindex("players/gbr/w_flame.md2");

	//faf:  airstrike plane
	gi.modelindex("models/ships/gbrplane/tris.md2");

}

