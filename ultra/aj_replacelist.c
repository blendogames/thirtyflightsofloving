/* item replacement */
#include "g_local.h"
#include "aj_replacelist.h"
#include "aj_startmax.h"

replace_details *replace_list = NULL;

void aj_replace (char *from, char *to)
{
	replace_details	*current = replace_list;
	replace_details *new_rep = NULL;

/*	if ( !FindItemByClassname(from) )
	{
		gi.dprintf ("\nreplace: invalid item name: \n");
		gi.dprintf (from);
		gi.dprintf ("\n");
		return;
	}

	if ( !FindItemByClassname(to) )
	{
		gi.dprintf ("\nreplace: invalid item name: \n");
		gi.dprintf (to);
		gi.dprintf ("\n");
		return;
	} */

	if ( !FindItemByClassname(from) || !FindItemByClassname(to) )
	{
		gi.dprintf ("replace: invalid item name in mapping: \n");
		gi.dprintf (from);
		gi.dprintf (" -> ");
		gi.dprintf (to);
		gi.dprintf ("\n");
		return;
	}

	while (current)
	{
		if ( !strcmp(current->from, from) )
			new_rep = current;
		current = current->next;
	}

	if ( !new_rep )
	{
		current = replace_list;

		if ( !current )
		{
			current = (replace_details *) gi.TagMalloc (sizeof(replace_details), TAG_GAME);
		//	current=(replace_details *) malloc (sizeof(replace_details));
			if ( !current )
				return; // should signal an error, but wtf...
			current->next = NULL;
		}
		else
		{
			current = (replace_details *) gi.TagMalloc (sizeof(replace_details), TAG_GAME);
		//	current=(replace_details *) malloc (sizeof(replace_details));
			if ( !current )
				return; // should signal an error, but wtf...
			current->next = replace_list;
		}
		replace_list = current;
		new_rep = replace_list;
	}
	else
	{
		free (new_rep->from);
		free (new_rep->to);
	}

	new_rep->from = strdup(from);
	new_rep->to = strdup(to);
}

gitem_t *lithium_replace_item (gitem_t *item)
{
	replace_details *current = replace_list;

	while (current)
	{
		if (item == FindItemByClassname(current->from))
			return (FindItemByClassname(current->to));
		current = current->next;
	}

	return item;
}

void Svcmd_Replace (void)
{
	aj_replace (gi.argv(2), gi.argv(3));
}

void Clcmd_Replace (void)
{
	aj_replace (gi.argv(1), gi.argv(2));
}


void clear_replacelist (void)
{
	replace_details *current = replace_list, *next_node;

	while (current)
	{
		next_node = current->next;

		free (current->from);
		free (current->to);
		free (current);

		current = next_node;
	}

	replace_list = NULL;
}


void read_replacelist (void)
{
	FILE	*f;
	int		i, mappings = 0;
	char	filename[256];
	char	strbuf[256], from_weapon[256], to_weapon[256];
	int		newline;
	size_t	fromLen, toLen;
/*	cvar_t	*game_dir;

	game_dir = gi.cvar ("game", "", 0);

#ifdef	_WIN32
//	i =  sprintf(filename, ".\\");
//	i += sprintf(filename + i, game_dir->string);
//	i += sprintf(filename + i, "\\");
//	i += sprintf(filename + i, replace_file->string);
	// Knightmare- use safe string functions
	Com_strcpy (filename, sizeof(filename), ".\\");
	Com_strcat (filename, sizeof(filename), game_dir->string);
	Com_strcat (filename, sizeof(filename), "\\");
	Com_strcat (filename, sizeof(filename), replace_file->string);
#else
	Com_strcpy (filename, sizeof(filename), "./");
	Com_strcat (filename, sizeof(filename), game_dir->string);
	Com_strcat (filename, sizeof(filename), "/");
	Com_strcat (filename, sizeof(filename), replace_file->string);
#endif */
	// Knightmare- use GameDir() for all platforms
	Com_strcpy (filename, sizeof(filename), va("%s/", GameDir()));
	Com_strcat (filename, sizeof(filename), replace_file->string);

	f = fopen (filename, "r");
	if ( !f ) {
	//	gi.error("Unable to read the replace file.\n");
		return;
	}

	// clear existing list first
	clear_replacelist ();

	// initialise the teams
	for (i=0; i<MAX_TEAMS; i++)
		bot_teams[i] = NULL;

	gi.dprintf ("\nReading %s...\n", replace_file->string);

	fscanf (f, "%c", &strbuf[0]);

	do
	{
		if ( feof(f) )
			break;

		if (strbuf[0] == '#')		// commented line
		{
			do {
				fscanf (f, "%c", &strbuf[0]);
			}
			while ( !feof(f) && (strbuf[0] != '\n') );
		}
		else if ( (strbuf[0] == '\n') || (strbuf[0] == ' ') || (strbuf[0] == '\t') )		// blank line
		{
			do {
				fscanf (f, "%c", &strbuf[0]);
			}
			while ( !feof(f) && ( (strbuf[0] == '\n') || (strbuf[0] == ' ') || (strbuf[0] == '\t') ) );
		}
		else // start of some data
		{
			i = 0;
			do
			{
				i++;
				fscanf (f, "%c", &strbuf[i]);

				if ( (strbuf[i] == '\n') || feof(f) )
					goto fail_line;
			}
			while ( (strbuf[i] != ' ') && (strbuf[i] != '\t') && (i < 255) );

			strbuf[i] = '\0';	// terminate the string
			Com_strcpy (from_weapon, sizeof(from_weapon), strbuf);

			do
			{
				fscanf (f, "%c", &strbuf[0]);

				if ( feof(f) )
					goto fail_line;
			}
			while ( (strbuf[0] == ' ') || (strbuf[i] == '\t') );

			i = 0;
			do
			{
				i++;
				fscanf (f, "%c", &strbuf[i]);
			}
			while ( (strbuf[i] != ' ') && (strbuf[i] != '\t') && (strbuf[i] != '\n') && (i < 255) && !feof(f) );

			if (strbuf[i] != '\n')
				newline = 1;
			else
				newline = 0;
			strbuf[i] = '\0';

			// Knightmare- zero out carriage returns on Linux
#ifndef _WIN32
			if ( (i > 2) && (strbuf[i-1] == '\r') ) {
				strbuf[i-1] = '\0';
			}
#endif // _WIN32

			Com_strcpy (to_weapon, sizeof(to_weapon), strbuf);

			fromLen = strlen(from_weapon);
			toLen = strlen(to_weapon);
			if ( (fromLen > 1) && (toLen > 1) )	// Knightmare- don't add 0 or 1-length item names
			{
			//	gi.dprintf ("Replacing %s with %s\n", from_weapon, to_weapon);

				aj_replace (from_weapon, to_weapon); // try adding this weapon

				mappings++;
			}
			else {
				if (fromLen < 2)
					gi.dprintf ("from name in replacement mapping has %i length\n", fromLen);
				else if (toLen < 2)
					gi.dprintf ("to name in replacement mapping has %i length\n", toLen);
			}

			if ( !newline )
				do {
					fscanf (f, "%c", &strbuf[0]);
				}
				while ( ( (strbuf[0] == ' ') || (strbuf[i] == '\t') ) && !feof(f) );

		//	mappings++;
fail_line:;
		}

	}
	while ( !feof(f) );

	gi.dprintf ("Read %i item mappings.\n", mappings);

	fclose (f);
}


void aj_onegun (int weap)
{
	char	*ammo;
	char	*weapon;
	float	grens = sk_start_grenades->value;
	char	number[4];
	int		newdmflags;

	gi.cvar_forceset ("start_grenades", "0");
	gi.cvar_forceset ("start_blaster", "0");
	gi.cvar_forceset ("start_shotgun", "0");
	gi.cvar_forceset ("start_sshotgun", "0");
	gi.cvar_forceset ("start_machinegun", "0");
	gi.cvar_forceset ("start_chaingun", "0");
	gi.cvar_forceset ("start_grenadelauncher", "0");
	gi.cvar_forceset ("start_rocketlauncher", "0");
	gi.cvar_forceset ("start_hyperblaster", "0");
	gi.cvar_forceset ("start_railgun", "0");
	gi.cvar_forceset ("start_bfg", "0");

	newdmflags = ~DF_WEAPONS_STAY & (int)dmflags->value;
	gi.cvar_forceset("dmflags", va("%d", newdmflags));

	if (weap == 1) // 1=special case RANDOM
	{
		weap = (rand() % 8) + 2;
	}

	// force to the chosen weapon
	Com_sprintf (number, sizeof(number), "%d", weap);
	gi.cvar_forceset ("start_weapon", number);


	switch (weap)
	{
	case 2:
		ammo = "ammo_shells";
		weapon = "weapon_shotgun";
		gi.cvar_forceset ("start_shotgun", "1");
		break;
	case 3:
		ammo = "ammo_shells";
		weapon = "weapon_supershotgun";
		gi.cvar_forceset ("start_sshotgun", "1");
		break;
	case 4:
		ammo = "ammo_bullets";
		weapon = "weapon_machinegun";
		gi.cvar_forceset ("start_machinegun", "1");
		break;
	case 5:
		ammo = "ammo_bullets";
		weapon = "weapon_chaingun";
		gi.cvar_forceset ("start_chaingun", "1");
		break;
	case 6:
		ammo = "ammo_grenades";
		weapon = "weapon_grenadelauncher";
		gi.cvar_forceset ("start_grenadelauncher", "1");
		gi.cvar_forceset ("start_grenades", va("%d", grens));
		break;
	case 7:
		ammo = "ammo_rockets";
		weapon = "weapon_rocketlauncher";
		gi.cvar_forceset ("start_rocketlauncher", "1");
		break;
	case 8:
		ammo = "ammo_cells";
		weapon = "weapon_hyperblaster";
		gi.cvar_forceset ("start_hyperblaster", "1");
		break;
	case 9:
		ammo = "ammo_slugs";
		weapon = "weapon_railgun";
		gi.cvar_forceset ("start_railgun", "1");
		break;
	case 10:
		ammo = "ammo_cells";
		weapon = "weapon_bfg";
		gi.cvar_forceset ("start_bfg", "1");
		break;
	default:
		ammo = "bugger";
		weapon = "crap";
		break;
	}

	aj_replace ("weapon_shotgun", weapon);
	aj_replace ("weapon_supershotgun", weapon);
	aj_replace ("weapon_machinegun", weapon);
	aj_replace ("weapon_chaingun", weapon);
	aj_replace ("weapon_grenadelauncher", weapon);
	aj_replace ("weapon_rocketlauncher", weapon);
	aj_replace ("weapon_hyperblaster", weapon);
	aj_replace ("weapon_railgun", weapon);
	aj_replace ("weapon_bfg", weapon);

	aj_replace ("ammo_shells", ammo);
	aj_replace ("ammo_bullets", ammo);
	aj_replace ("ammo_grenades", ammo);
	aj_replace ("ammo_rockets", ammo);
	aj_replace ("ammo_cells", ammo);
	aj_replace ("ammo_slugs", ammo);
}

void Svcmd_OneGun (void)
{
	aj_onegun ( atoi( gi.argv(2) ) );
}
