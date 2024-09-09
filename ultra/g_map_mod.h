//
// Map Mod
// Jeremy Mappus - Jerm a.k.a DarkTheties
//

#ifndef MAP_MOD_H
#define MAP_MOD_H

extern int	map_mod_;							// 1 if maps.txt was found and loaded 0 if not

void		map_mod_set_up (void);				// Attempts to find and load maps.txt
char		*map_mod_next_map (void);			// Retrieves name of next level in the list

#endif


/*
*** NOTE: You will have to #include "g_map_mod.h" at the top of g_save.c

This code goes at the end of the InitGame function in g_save.c

    // MAP MOD
	map_mod_set_up();
	// MAP MOD

*/

/*
*** NOTE: You will have to #include "g_map_mod.h" at the top of g_main.c

This code goes in the EndDMLevel function in g_main.c at the top of the function

	// MAP MOD
	if (map_mod_)
	{
		Com_strcpy(level.nextmap, sizeof(level.nextmap), map_mod_next_map());
	}
	// MAP MOD


*/

/*
	This will NOT load the first map in maps.txt upon server startup and there seemed no
	easy way to get it to do that. If you are having problems with a map repeating, try
	using spaces or tabs to seperate maps in the maps.txt instead of return characters.

	Enjoy,
		Jerm...
*/

