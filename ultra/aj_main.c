/*
*
* Lithium simulation
*
* Coded by Anthony Jacques, June 1998
*
*/

#include "g_local.h"
#include "bot_procs.h" // for lithium_motd (Eraser version)
#include <stdio.h> // for lithium_motd()

// for directory creation
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include "aj_runes.h"
#include "aj_startmax.h"
#include "aj_statusbars.h"
#include "aj_replacelist.h"
#include "aj_banning.h"


gitem_t *item_phalanx, *item_ionripper, *item_trap, *item_quadfire, *item_magslugs, *item_ir_goggles, *item_double, *item_sphere_vengeance, *item_sphere_hunter, *item_sphere_defender, *item_doppleganger;


// set ammo item lookups
void set_item_lookups (void)
{
	item_shells = FindItem("shells");
	item_cells = FindItem("cells");
	item_bullets = FindItem("bullets");
	item_rockets = FindItem("rockets");
	item_slugs = FindItem("slugs");
	item_grenades = FindItem("grenades");

	item_blaster	= FindItem("blaster");
	item_shotgun	= FindItem("shotgun");
	item_supershotgun	= FindItem("super shotgun");
	item_machinegun	= FindItem("machinegun");
	item_chaingun	= FindItem("chaingun");
	item_grenadelauncher	= FindItem("grenade launcher");
	item_rocketlauncher	= FindItem("rocket launcher");
	item_railgun	= FindItem("railgun");
	item_hyperblaster	= FindItem("hyperblaster");
	item_bfg10k	= FindItem("bfg10k");

	item_phalanx	= FindItem("Phalanx");
	item_ionripper	= FindItem("Ionripper");
	item_trap	= FindItem("Trap");
	item_quadfire	= FindItem("DualFire Damage");
	item_ir_goggles = FindItem("IR Goggles");
	item_double = FindItem("Double Damage");
	item_sphere_vengeance = FindItem("Vengeance sphere");
	item_sphere_hunter = FindItem("Hunter sphere");
	item_sphere_defender = FindItem("Defender sphere");
	item_doppleganger = FindItem("Doppleganger");

	item_magslugs	= FindItem("Mag Slug");

	item_chainfist	= FindItem("Chainfist");
	item_disruptor	= FindItem("Disruptor");
	item_etfrifle	= FindItem("ETF Rifle");
	item_plasmabeam	= FindItem("Plasma Beam");
	item_proxlauncher	= FindItem("Prox Launcher");
	item_prox = FindItem("Prox");
	item_tesla = FindItem("Tesla");
	item_rounds	= FindItem("Rounds");
	item_flechettes	= FindItem("Flechettes");

	item_plasmarifle	= FindItem(PLASMA_PICKUP_NAME);
	item_shockwave		= FindItem("Shockwave");
}


void lithium_init (void)
{
	char	none[6]="none";
	char	*c=none;
	char	dir[256];
//	int		i;
//	cvar_t	*game_dir;

	while (*c)
	{
		*c += 128;
		c++;
	}

	gi.configstring(CS_TECHNONE, none);
	gi.configstring(CS_SAFETY, "Safety Mode");
	gi.configstring(CS_OBSERVER, "Observer Mode");

	if (use_runes->value)
	{
		lithium_setuprunes();
		CTFSetupTechSpawn();
	}

	if (use_replacelist->value)
		read_replacelist();

	read_banlist();

	if (use_onegun->value)
		aj_onegun((int)use_onegun->value);

/*	game_dir = gi.cvar ("game", "", 0);

#ifdef  _WIN32
//	i =  sprintf(dir, ".\\");
//	i += sprintf(dir + i, game_dir->string);
//	i += sprintf(dir + i, "\\routes");
	// Knightmare- use safe string functions
	Com_strcpy (dir, sizeof(dir), ".\\");
	Com_strcat (dir, sizeof(dir), game_dir->string);
	Com_strcat (dir, sizeof(dir), "\\routes");
	mkdir (dir);
#else
	Com_strcpy (dir, sizeof(dir), "./");
	Com_strcat (dir, sizeof(dir), game_dir->string);
	Com_strcat (dir, sizeof(dir), "/routes");
	mkdir (dir, 755);
#endif */

	// Knightmare- use SavegameDir() for all platforms
	Com_strcpy (dir, sizeof(dir), SavegameDir());
	Com_strcat (dir, sizeof(dir), "/routes");
#ifdef  _WIN32
	mkdir (dir);
#else
	mkdir (dir, 755);
#endif

	aj_choosebar ();
}


void lithium_initclient (gclient_t *client)
{
	if (!item_shells)
		set_item_lookups();
// (set the start_* values)
// FIXME: can all these ITEM_INDEX(find_item()) calls be replaced by item_shells etc?
	client->pers.inventory[ITEM_INDEX(item_shells)] = sk_start_shells->value;	//crashes here
	client->pers.inventory[ITEM_INDEX(item_bullets)] = sk_start_bullets->value;
	client->pers.inventory[ITEM_INDEX(item_cells)] = sk_start_cells->value;
	client->pers.inventory[ITEM_INDEX(item_rockets)] = sk_start_rockets->value;
	client->pers.inventory[ITEM_INDEX(item_slugs)] = sk_start_slugs->value;
	client->pers.inventory[ITEM_INDEX(item_grenades)] = sk_start_grenades->value;
	client->pers.inventory[ITEM_INDEX(item_magslugs)] = sk_start_magslugs->value;
	client->pers.inventory[ITEM_INDEX(item_trap)] = sk_start_traps->value;
	client->pers.inventory[ITEM_INDEX(item_flechettes)] = sk_start_flechettes->value;
	client->pers.inventory[ITEM_INDEX(item_rounds)] = sk_start_rounds->value;

	client->pers.inventory[ITEM_INDEX(item_blaster)] = sk_start_blaster->value;
	client->pers.inventory[ITEM_INDEX(item_shotgun)] = sk_start_shotgun->value;
	client->pers.inventory[ITEM_INDEX(item_supershotgun)] = sk_start_sshotgun->value;
	client->pers.inventory[ITEM_INDEX(item_machinegun)] = sk_start_machinegun->value;
	client->pers.inventory[ITEM_INDEX(item_chaingun)] = sk_start_chaingun->value;
	client->pers.inventory[ITEM_INDEX(item_grenadelauncher)] = sk_start_grenadelauncher->value;
	client->pers.inventory[ITEM_INDEX(item_rocketlauncher)] = sk_start_rocketlauncher->value;
	client->pers.inventory[ITEM_INDEX(item_hyperblaster)] = sk_start_hyperblaster->value;
	client->pers.inventory[ITEM_INDEX(item_railgun)] = sk_start_railgun->value;
	client->pers.inventory[ITEM_INDEX(item_bfg10k)] = sk_start_bfg->value;
	client->pers.inventory[ITEM_INDEX(item_ionripper)] = sk_start_ionripper->value;
	client->pers.inventory[ITEM_INDEX(item_phalanx)] = sk_start_phalanx->value;
	client->pers.inventory[ITEM_INDEX(item_etfrifle)] = sk_start_etfrifle->value;
	client->pers.inventory[ITEM_INDEX(item_proxlauncher)] = sk_start_proxlauncher->value;
	client->pers.inventory[ITEM_INDEX(item_plasmabeam)] = sk_start_plasmabeam->value;
	client->pers.inventory[ITEM_INDEX(item_disruptor)] = sk_start_disruptor->value;
	client->pers.inventory[ITEM_INDEX(item_chainfist)] = sk_start_chainfist->value;
	client->pers.inventory[ITEM_INDEX(item_plasmarifle)] = sk_start_plasmarifle->value;
	client->pers.inventory[ITEM_INDEX(item_shockwave)] = sk_start_shockwave->value;


	switch ((int)sk_start_rune->value)
	{
	case 1:
		client->pers.inventory[ITEM_INDEX(item_tech1)] = 1;
		client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH1;
		break;
	case 2:
		client->pers.inventory[ITEM_INDEX(item_tech2)] = 1;
		client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH2;
		break;
	case 3:
		client->pers.inventory[ITEM_INDEX(item_tech3)] = 1;
		client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH3;
		break;
	case 4:
		client->pers.inventory[ITEM_INDEX(item_tech4)] = 1;
		client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH4;
		break;
	case 5:
		client->pers.inventory[ITEM_INDEX(item_tech5)] = 1;
		client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH5;
		break;
	case 6:
		client->pers.inventory[ITEM_INDEX(item_tech6)] = 1;
		client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH6;
		break;
	}

	client->pers.health			= sk_start_health->value;

// set the max_* values
	client->pers.max_health		= sk_max_health->value;
	client->pers.max_bullets	= sk_max_bullets->value;
	client->pers.max_shells		= sk_max_shells->value;
	client->pers.max_rockets	= sk_max_rockets->value;
	client->pers.max_grenades	= sk_max_grenades->value;
	client->pers.max_cells		= sk_max_cells->value;
	client->pers.max_slugs		= sk_max_slugs->value;
	client->pers.max_magslug	= sk_max_magslugs->value;
	client->pers.max_trap		= sk_max_traps->value;
	client->pers.max_prox		= sk_max_prox->value;
	client->pers.max_tesla		= sk_max_tesla->value;
	client->pers.max_flechettes = sk_max_flechettes->value;
#ifndef KILL_DISRUPTOR
	client->pers.max_rounds     = sk_max_rounds->value;
#endif
	client->pers.max_armor		= sk_max_armor->value;

	client->resp.id_state = def_id->value;
	client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECHNONE;
}


void lithium_choosestartweap(edict_t *ent)
{
	gclient_t *client=ent->client;

	client->newweapon=0;

	if ( ((sk_start_weapon->value == 0) || (sk_start_weapon->value == 1))
		&& client->pers.inventory[ITEM_INDEX(item_blaster)] )
		client->newweapon = item_blaster;

	if ( client->pers.inventory[ITEM_INDEX(item_shotgun)] &&
		( (client->pers.inventory[ITEM_INDEX(item_shells)] && (sk_start_weapon->value == 0))
		 || (sk_start_weapon->value == 2)) )
	{
		client->newweapon = item_shotgun;
	}

	if ( client->pers.inventory[ITEM_INDEX(item_supershotgun)] &&
		( (client->pers.inventory[ITEM_INDEX(item_shells)] && (sk_start_weapon->value == 0))
		|| (sk_start_weapon->value == 3)) )
	{
		client->newweapon = item_supershotgun;
	}

	if ( client->pers.inventory[ITEM_INDEX(item_machinegun)] &&
		( (client->pers.inventory[ITEM_INDEX(item_bullets)] && (sk_start_weapon->value == 0))
		|| (sk_start_weapon->value == 4)) )
	{
		client->newweapon = item_machinegun;
	}

	if ( client->pers.inventory[ITEM_INDEX(item_chaingun)] &&
		( (client->pers.inventory[ITEM_INDEX(item_bullets)] && (sk_start_weapon->value == 0))
		|| (sk_start_weapon->value == 5)) )
	{
		client->newweapon = item_chaingun;
	}

	if ( client->pers.inventory[ITEM_INDEX(item_grenadelauncher)] &&
		( (client->pers.inventory[ITEM_INDEX(item_grenades)] && (sk_start_weapon->value == 0))
		|| (sk_start_weapon->value == 6)) )
	{
		client->newweapon = item_grenadelauncher;
	}

	if ( client->pers.inventory[ITEM_INDEX(item_rocketlauncher)] &&
		( (client->pers.inventory[ITEM_INDEX(item_rockets)] && (sk_start_weapon->value == 0))
		|| (sk_start_weapon->value == 7)) )
	{
		client->newweapon = item_rocketlauncher;
	}

	if ( client->pers.inventory[ITEM_INDEX(item_hyperblaster)] &&
		( (client->pers.inventory[ITEM_INDEX(item_cells)] && (sk_start_weapon->value == 0 ))
		|| (sk_start_weapon->value == 8)) )
	{
		client->newweapon = item_hyperblaster;
	}

	if ( client->pers.inventory[ITEM_INDEX(item_railgun)] &&
		( (client->pers.inventory[ITEM_INDEX(item_slugs)] && (sk_start_weapon->value == 0))
		|| (sk_start_weapon->value == 9)) )
	{
		client->newweapon = item_railgun;
	}

	if ( client->pers.inventory[ITEM_INDEX(item_bfg10k)] &&
		( (client->pers.inventory[ITEM_INDEX(item_cells)] && (sk_start_weapon->value == 0))
		|| (sk_start_weapon->value == 10)) )
	{
		client->newweapon = item_bfg10k;
	}

	if (ent->bot_client)
		botPickBestWeapon(ent);
	else
	{
		client->pers.weapon = client->newweapon;
		client->pers.lastweapon = client->newweapon;
	}
}

// function returns 1 if the item should be spawned
int lithium_weaponbanning(edict_t *ent)
{
//	if (bot_calc_nodes->value)
//		return 1; // NEVER ban an item if calc'ing nodes as the route will be incomplete if loaded with items un-banned

	if ((sk_no_shotgun->value && (strcmp(ent->classname, "weapon_shotgun") == 0))
		|| (sk_no_sshotgun->value && (strcmp(ent->classname, "weapon_supershotgun") == 0))
		|| (sk_no_machinegun->value && (strcmp(ent->classname, "weapon_machinegun") == 0))
		|| (sk_no_chaingun->value && (strcmp(ent->classname, "weapon_chaingun") == 0))
		|| (sk_no_grenadelauncher->value && (strcmp(ent->classname, "weapon_grenadelauncher") == 0))
		|| (sk_no_rocketlauncher->value && (strcmp(ent->classname, "weapon_rocketlauncher") == 0))
		|| (sk_no_hyperblaster->value && (strcmp(ent->classname, "weapon_hyperblaster") == 0))
		|| (sk_no_railgun->value && (strcmp(ent->classname, "weapon_railgun") == 0))
		|| (sk_no_bfg->value && (strcmp(ent->classname, "weapon_bfg") == 0))
		|| (sk_no_quad->value && (strcmp(ent->classname, "item_quad") == 0))
		|| (sk_no_pent->value && (strcmp(ent->classname, "item_invulnerability") == 0))
		|| (sk_no_ionripper->value && (strcmp(ent->classname, "weapon_boomer") == 0))
		|| (sk_no_phalanx->value && (strcmp(ent->classname, "weapon_phalanx") == 0))
		|| (sk_no_traps->value && (strcmp(ent->classname, "ammo_trap") == 0))
		|| (sk_no_etfrifle->value && (strcmp(ent->classname, "weapon_etf_rifle") == 0))
		|| (sk_no_proxlauncher->value && (strcmp(ent->classname, "weapon_proxlauncher") == 0))
		|| (sk_no_plasmabeam->value && (strcmp(ent->classname, "weapon_plasmabeam") == 0))
		|| (sk_no_disruptor->value && (strcmp(ent->classname, "weapon_disintegrator") == 0))
		|| (sk_no_chainfist->value && (strcmp(ent->classname, "weapon_chainfist") == 0))
		|| (sk_no_plasmarifle->value && (strcmp(ent->classname, "weapon_plasma") == 0)))
	{
		G_FreeEdict(ent);
		return 0;
	}
	return 1;
}


void lithium_motd (edict_t *ent)
{
	FILE	*fh = NULL;
	char	output[1024], line[257];
	char	filename[256];
	int		/*line_count = 0,*/ i = 0;
#ifndef _WIN32
	int		j;
	size_t  len;
#endif // _WIN32
//	cvar_t	*game_dir;

	if ( !motd->string )
	{
		gi.centerprintf (ent, "\n\n=====================================\nEraser Ultra v%1.3f BETA\nby Anthony Jacques\nBased upon Eraser %1.3f by Ryan Feltrin\n\nNo MOTD defined.\n\nwww.users.zetnet.co.uk/jacquesa/q2/\n\n",
						ULTRA_VERSION , ERASER_VERSION);
		return;
	}

/*	game_dir = gi.cvar ("game", "", 0);

#ifdef	_WIN32
	// Knightmare- use safe string functions
	Com_strcpy (filename, sizeof(filename), ".\\");
	Com_strcat (filename, sizeof(filename), game_dir->string);
	Com_strcat (filename, sizeof(filename), "\\");
	Com_strcat (filename, sizeof(filename), motd->string);
#else
	Com_strcpy (filename, sizeof(filename), "./");
	Com_strcat (filename, sizeof(filename), game_dir->string);
	Com_strcat (filename, sizeof(filename), "/");
	Com_strcat (filename, sizeof(filename), motd->string);
#endif */
	// Knightmare- use GameDir() for all platforms
	Com_strcpy (filename, sizeof(filename), va("%s/", GameDir()));
	Com_strcat (filename, sizeof(filename), motd->string);

	fh = fopen(filename, "r");
	if (fh == NULL)
	{
		gi.centerprintf (ent, "\n\n=====================================\nEraser Ultra v%1.3f BETA\nby Anthony Jacques\nBased upon Eraser %1.3f by Ryan Feltrin\n\nMOTD file not found.\n\nwww.users.zetnet.co.uk/jacquesa/q2/\n\n",
                        ULTRA_VERSION , ERASER_VERSION);
		return;
	}

//	gi.dprintf ("reading %s\n", filename);

	output[0] = 0;
	while ( fgets(line, 256, fh) && (i < 1024) )
	{
		// Knightmare- replace carriage returns on Linux
#ifndef _WIN32
		len = strlen(line);
		for (j = 0; j < len; j++) {
			if (line[j] == '\r')
				line[j] = ' ';
		}
#endif // _WIN32

		Com_strcat (output, sizeof(output), line);
		i += (int)strlen(line);
	}

	if (i > 0)
		output[i] = '\0';

	fclose (fh);

	gi.centerprintf (ent, output);
}




void lithium_observer(edict_t *ent, pmenu_t *menu)
{
	if (use_observer->value && !ent->bot_client)
	{
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->observer_mode = true;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);
		ent->client->ps.stats[STAT_LITHIUM_MODE] = CS_OBSERVER;
		if (ent->client->bIsCamera)
			ent->client->bIsCamera=0;

		if (gamerules && gamerules->value) //ScarFace added
		{
			if(DMGame.PlayerDisconnect)
				DMGame.PlayerDisconnect(ent);
		}
	}
}
