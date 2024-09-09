/////////////////////////////////////////////////////////////////////////////
// Map Mod
// by Jeremy Mappus - Jerm a.k.a. DarkTheties
//
// This mod will load a file called maps.txt in the game directory. It will
// take any names separated by white spaces and list them as seperate name
// entries into the map_mod_names_ table. It also provides a function for
// checking the change level.
//
// NIQ changes by Mike Fox (a.k.a. Artful Dodger)
// 	Skip rest of line after any '#'.
//  Support for using all maps  before starting over if in random mode and
// 	niq_allmaps=1.

#include "g_local.h"
#include "g_map_mod.h"
//#include "g_niq.h"

#include <stdio.h>

#define	MAPMOD_MAXLEVELS	256

int			map_mod_ = 0;
int			map_mod_current_level_ = -1;
int			map_mod_n_levels_ = 0;
char		map_mod_names_[MAPMOD_MAXLEVELS][64];

// niq hack:
qboolean	map_used[MAPMOD_MAXLEVELS];
int			unused_maps=0;

cvar_t *mapmod_random;

/////////////////////////////////////////////////////////////////////////////

void map_mod_set_up (void)
{
	FILE	*file;
	char	file_name[256];
	cvar_t	*game_dir, *basedir;

	mapmod_random = gi.cvar ("mapmod_random", "0", CVAR_ARCHIVE);

	game_dir = gi.cvar ("game", "", 0);
	basedir = gi.cvar("basedir", ".", 0);

// AJ - added extra parameter to allow the maps.txt to be changed
#if defined(linux)
	Com_sprintf (file_name, sizeof(file_name), "%s/%s/%s", basedir->string, game_dir->string, mapqueue->string);
#else
	Com_sprintf (file_name, sizeof(file_name), "%s\\%s\\%s", basedir->string, game_dir->string, mapqueue->string);
#endif
// end AJ

	file = fopen(file_name, "r");
	map_mod_ = 0;
	map_mod_current_level_ = -1;
	map_mod_n_levels_ = 0;
	if (file != NULL)
	{
		long file_size;
		char *p_buffer;
		char *p_name;
		long counter = 0;
		int n_chars = 0;

		file_size = 0;
		while (!feof(file))
		{
			fgetc(file);
			file_size++;
		}
		rewind (file);

		p_buffer = malloc(file_size);
		memset (p_buffer,0,file_size);

		fread ((void *)p_buffer, sizeof(char), file_size, file);

		gi.dprintf ("\n==== Map Mod v1.01 set up ====\n");
		gi.dprintf ("Adding maps to cycle: ");

		p_name = p_buffer;
		do
		{
			// niq: skip rest of line after a '#' (works with Unix?)
			if (*p_name == '#')
			{
				while ((*p_name != '\n') && (*p_name != '\r') && counter < file_size)
					{
					p_name++;
					counter++;
					}
			}
			else
			{
				while ((((*p_name >= 'a') && (*p_name <= 'z')) || ((*p_name >= 'A') && (*p_name <= 'Z')) || ((*p_name >= '0') && (*p_name <= '9')) || (*p_name == '_') || (*p_name == '-') || (*p_name == '/') || (*p_name == '\\')) && counter < file_size)
				{
					n_chars++;
					counter++;
					p_name++;
				}
			}

			if (n_chars)
			{
				memcpy(&map_mod_names_[map_mod_n_levels_][0], p_name - n_chars, n_chars);
				memset(&map_mod_names_[map_mod_n_levels_][n_chars], 0, 1);

				if (map_mod_n_levels_ > 0)
					gi.dprintf(", ");
				gi.dprintf("%s", map_mod_names_[map_mod_n_levels_]);

				map_mod_n_levels_++;
				n_chars = 0;

				if (map_mod_n_levels_ >= MAPMOD_MAXLEVELS)
				{
					gi.dprintf ("\nMAPMOD_MAXLEVELS exceeded\nUnable to add more levels.\n");
					break;
				}

				// skip rest of the line...
				while ((*p_name != '\n') && (*p_name != '\r') && counter < file_size)
				{
					p_name++;
					counter++;
				}
			}

			// next mapname
			counter++;
			p_name++;

			// eat up non-characters (niq: except #)
			while (!((*p_name == '#') || ((*p_name >= 'a') && (*p_name <= 'z')) || ((*p_name >= 'A') && (*p_name <= 'Z')) || ((*p_name >= '0') && (*p_name <= '9')) || (*p_name == '_') || (*p_name == '-') || (*p_name == '/') || (*p_name == '\\')) && counter < file_size)
			{
				counter++;
				p_name++;
			}

		} while (counter < file_size);

		gi.dprintf ("\n\n");

		free (p_buffer);
		fclose (file);

		if (map_mod_n_levels_)
		{
			map_mod_ = true;
		}
	}
	else
	{
// AJ - changed message slightly to allow for different filenames
		gi.dprintf ("==== Map Mod v1.01 - missing map-list file ====\n");
	}

	unused_maps=0;
}

/////////////////////////////////////////////////////////////////////////////

char* map_mod_next_map (void)
{
	int i;

	if (map_mod_)
	{
		if (mapmod_random->value)
		{
			// NIQ hack start
			if (map_mod_n_levels_ >= 2)
			{
				// niq: hack to mapmode code to make sure we try all maps
				// before starting over.
				int map;
				int skipped;

				if (!unused_maps)
				{
					// reset random maps
					for (map=0; map<map_mod_n_levels_; map++)
						map_used[map] = false;

					if (map_mod_current_level_ == -1 && level.mapname)
					{
						// no current MapMod map:
						// if there is a current map make sure we don't
						// pick it again right away if it is in the list
						for (i=0; (i<map_mod_n_levels_ && map_mod_current_level_== -1); i++)
							if ( !Q_stricmp(level.mapname, map_mod_names_[i]) )
								map_mod_current_level_ = i;

					}

					if (map_mod_current_level_ != -1)
					{
						// zap the map
						map_used[map_mod_current_level_] = true;

						// one less unused map to choose from
						unused_maps = map_mod_n_levels_ - 1;
					}
					else
					{
						// can choose any map in list
						unused_maps = map_mod_n_levels_;
					}
				}

				// pick number of unsued maps to skip (less clustering likely)
				i = (int) floor(random() * ((float)(unused_maps)));

				// skip to first unused map (has to find one)
				map	= 0;
				while (map_used[map])
					map++;

				// skip over i unused maps (has to find them)
				skipped	= 0;
				while (skipped<i)
				{
					if (!map_used[map++])
						skipped++;
				}

				// skip to unused map if necessary (e.g. if last skip skipped to used one)
				while (map_used[map])
					map++;

				map_mod_current_level_ = map;
				map_used[map] = true;
				unused_maps--;
			}
			// NIQ hack end
		else
		{
			map_mod_current_level_ = -1;

			i = (int) floor(random() * ((float)(map_mod_n_levels_)));

			if ( !Q_stricmp(level.mapname, map_mod_names_[i]) )
			{
				if (++i >= map_mod_n_levels_)
					i = 0;
			}
			map_mod_current_level_ = i;
		}
		}
		else
		{
			map_mod_current_level_ = -1;

			for (i=0; i < map_mod_n_levels_; i++)
				if ( !Q_stricmp(level.mapname, map_mod_names_[i]) )
					map_mod_current_level_ = i+1;
		}

		if (map_mod_current_level_ >= map_mod_n_levels_)
		{
			map_mod_current_level_ = 0;
		}

		if (map_mod_current_level_ > -1)
		{
			return map_mod_names_[map_mod_current_level_];
		}

	}

	return NULL;
}

