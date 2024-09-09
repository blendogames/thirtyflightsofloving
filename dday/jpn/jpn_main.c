/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/jpn/jpn_main.c,v $
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

  vjj  03/29/98

  We are using the dll template that was created for the multiple dll 
 modification.
*/

#include "jpn_main.h"
#include "jpn_classes.h"


void JPN_UserPrecache(void) 
{
	gi.imageindex("scope_jpn");
	gi.imageindex("victory_jpn");

	gi.soundindex("jpn/victory.wav");

	gi.modelindex("players/jpn/tris.md2");

	//
	// V-WEPS
	//

	// JPN 
	gi.modelindex("players/jpn/w_nambu.md2");
	gi.modelindex("players/jpn/w_arisaka.md2");
	gi.modelindex("players/jpn/w_type_100.md2");
	gi.modelindex("players/jpn/w_type_99.md2");
	gi.modelindex("players/jpn/w_panzer.md2");
	gi.modelindex("players/jpn/w_arisakas.md2");
	gi.modelindex("players/jpn/a_97_grenade.md2");
	gi.modelindex("players/jpn/w_katana.md2");
}

