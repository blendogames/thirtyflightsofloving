/*
===========================================================================
Copyright (C) 1997-2001 Id Software, Inc.

This file is part of Quake 2 source code.

Quake 2 source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake 2 source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake 2 source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "server.h"
#include "../client/ref.h"


/*
===============================================================================

OPERATOR CONSOLE ONLY COMMANDS

These commands can only be entered from stdin or by a remote operator datagram
===============================================================================
*/

/*
====================
SV_SetMaster_f

Specify a list of master servers
====================
*/
void SV_SetMaster_f (void)
{
	int		i, slot;

	// only dedicated servers send heartbeats
	if (!dedicated->value)
	{
		Com_Printf ("Only dedicated servers use masters.\n");
		return;
	}

	// make sure the server is listed public
	Cvar_Set ("public", "1"); // Vic's fix

	for (i=1 ; i<MAX_MASTERS ; i++)
		memset (&master_adr[i], 0, sizeof(master_adr[i]));

	slot = 1;		// slot 0 will always contain the id master
	for (i=1 ; i<Cmd_Argc() ; i++)
	{
		if (slot == MAX_MASTERS)
			break;

		if (!NET_StringToAdr (Cmd_Argv(i), &master_adr[i]))
		{
			Com_Printf ("Bad address: %s\n", Cmd_Argv(i));
			continue;
		}
		if (master_adr[slot].port == 0)
			master_adr[slot].port = BigShort (PORT_MASTER);

		Com_Printf ("Master server at %s\n", NET_AdrToString (master_adr[slot]));

		Com_Printf ("Sending a ping.\n");

		Netchan_OutOfBandPrint (NS_SERVER, master_adr[slot], "ping");

		slot++;
	}

	svs.last_heartbeat = -9999999;
}



/*
==================
SV_SetPlayer

Sets sv_client and sv_player to the player with idnum Cmd_Argv(1)
==================
*/
qboolean SV_SetPlayer (void)
{
	client_t	*cl;
	int			i;
	int			idnum;
	char		*s;

	if (Cmd_Argc() < 2)
		return false;

	s = Cmd_Argv(1);

	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9')
	{
		idnum = atoi(Cmd_Argv(1));
		if (idnum < 0 || idnum >= maxclients->value)
		{
			Com_Printf ("Bad client slot: %i\n", idnum);
			return false;
		}

		sv_client = &svs.clients[idnum];
		sv_player = sv_client->edict;
		if (!sv_client->state)
		{
			Com_Printf ("Client %i is not active\n", idnum);
			return false;
		}
		return true;
	}

	// check for a name match
	for (i=0,cl=svs.clients ; i<maxclients->value; i++,cl++)
	{
		if (!cl->state)
			continue;
		if (!strcmp(cl->name, s))
		{
			sv_client = cl;
			sv_player = sv_client->edict;
			return true;
		}
	}

	Com_Printf ("Userid %s is not on the server\n", s);
	return false;
}


/*
===============================================================================

SAVEGAME FILES

===============================================================================
*/
void	R_GrabScreen (void); // Knightmare- screenshots for savegames
void	R_ScaledScreenshot (char *name); // Knightmare- screenshots for savegames
void	R_FreePic (char *name); // Knightmare- unregisters an image

/*
=====================
SV_WipeSavegame

Delete save/<XXX>/
=====================
*/
void SV_WipeSavegame (char *savename)
{
	char	name[MAX_OSPATH];
	char	*s;

	Com_DPrintf("SV_WipeSaveGame(%s)\n", savename);

//	Com_sprintf (name, sizeof(name), "%s/save/%s/server.ssv", FS_Savegamedir (), savename);	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/%s/server.ssv", FS_Savegamedir (), savename);	// was FS_Gamedir()
	remove (name);
//	Com_sprintf (name, sizeof(name), "%s/save/%s/game.ssv", FS_Savegamedir (), savename);	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/%s/game.ssv", FS_Savegamedir (), savename);	// was FS_Gamedir()
	remove (name);
	// Knightmare- delete screenshot
//	Com_sprintf (name, sizeof(name), "%s/save/%s/shot.jpg", FS_Savegamedir (), savename);	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/%s/shot.jpg", FS_Savegamedir (), savename);	// was FS_Gamedir()
	remove (name);

//	Com_sprintf (name, sizeof(name), "%s/save/%s/*.sav", FS_Savegamedir (), savename);	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/%s/*.sav", FS_Savegamedir (), savename);	// was FS_Gamedir()
	s = Sys_FindFirst( name, 0, 0 );
	while (s)
	{
		remove (s);
		s = Sys_FindNext( 0, 0 );
	}
	Sys_FindClose ();
//	Com_sprintf (name, sizeof(name), "%s/save/%s/*.sv2", FS_Savegamedir (), savename);	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/%s/*.sv2", FS_Savegamedir (), savename);	// was FS_Gamedir()
	s = Sys_FindFirst(name, 0, 0 );
	while (s)
	{
		remove (s);
		s = Sys_FindNext( 0, 0 );
	}
	Sys_FindClose ();
//	Com_sprintf (name, sizeof(name), "%s/save/%s/*.savz", FS_Savegamedir(), savename);	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/%s/*.savz", FS_Savegamedir(), savename);	// was FS_Gamedir()
	s = Sys_FindFirst(name, 0, 0 );
	while (s)
	{
		remove (s);
		s = Sys_FindNext( 0, 0 );
	}
	Sys_FindClose ();
}


/*
================
SV_CopySaveGame
================
*/
void SV_CopySaveGame (char *src, char *dst)
{
	char	name[MAX_OSPATH], name2[MAX_OSPATH];
	int		l, len, i, extLen;
	char	*found;
	static const char *saveExtensions[] =
	{
		"sav",
		"savz",
		0
	};

	Com_DPrintf("SV_CopySaveGame(%s, %s)\n", src, dst);

	SV_WipeSavegame (dst);

	// copy the savegame over
//	Com_sprintf (name, sizeof(name), "%s/save/%s/server.ssv", FS_Savegamedir(), src);	// was FS_Gamedir()
//	Com_sprintf (name2, sizeof(name2), "%s/save/%s/server.ssv", FS_Savegamedir(), dst);	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/%s/server.ssv", FS_Savegamedir(), src);	// was FS_Gamedir()
	Com_sprintf (name2, sizeof(name2), "%s/"SAVEDIRNAME"/%s/server.ssv", FS_Savegamedir(), dst);	// was FS_Gamedir()
	FS_CreatePath (name2);
	FS_CopyFile (name, name2);

//	Com_sprintf (name, sizeof(name), "%s/save/%s/game.ssv", FS_Savegamedir(), src);	// was FS_Gamedir()
//	Com_sprintf (name2, sizeof(name2), "%s/save/%s/game.ssv", FS_Savegamedir(), dst);	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/%s/game.ssv", FS_Savegamedir(), src);	// was FS_Gamedir()
	Com_sprintf (name2, sizeof(name2), "%s/"SAVEDIRNAME"/%s/game.ssv", FS_Savegamedir(), dst);	// was FS_Gamedir()
	FS_CopyFile (name, name2);

	// Knightmare- copy screenshot
	if (strcmp(dst, "kmq2save0")) // no screenshot for start of level autosaves
	{
	//	Com_sprintf (name, sizeof(name), "%s/save/%s/shot.jpg", FS_Savegamedir(), src);	// was FS_Gamedir()
	//	Com_sprintf (name2, sizeof(name2), "%s/save/%s/shot.jpg", FS_Savegamedir(), dst);	// was FS_Gamedir()
		Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/%s/shot.jpg", FS_Savegamedir(), src);	// was FS_Gamedir()
		Com_sprintf (name2, sizeof(name2), "%s/"SAVEDIRNAME"/%s/shot.jpg", FS_Savegamedir(), dst);	// was FS_Gamedir()
		FS_CopyFile (name, name2);
	}

//	Com_sprintf (name, sizeof(name), "%s/save/%s/", FS_Savegamedir(), src);	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/%s/", FS_Savegamedir(), src);	// was FS_Gamedir()
	len = (int)strlen(name);
//	Com_sprintf (name, sizeof(name), "%s/save/%s/*.sav", FS_Savegamedir(), src);	// was FS_Gamedir()
	for (i = 0; i < 2; i++)
	{
	//	Com_sprintf (name, sizeof(name), "%s/save/%s/*.%s", FS_Savegamedir(), src, saveExtensions[i]);	// was FS_Gamedir()
		Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/%s/*.%s", FS_Savegamedir(), src, saveExtensions[i]);	// was FS_Gamedir()
		extLen = (int)strlen (saveExtensions[i]);
		found = Sys_FindFirst (name, 0, 0);
		while (found)
		{
		//	strncpy (name+len, found+len);
			Q_strncpyz (name+len, found+len, sizeof(name)-len);
		//	Com_sprintf (name2, sizeof(name2), "%s/save/%s/%s", FS_Savegamedir(), dst, found+len);	// was FS_Gamedir()
			Com_sprintf (name2, sizeof(name2), "%s/"SAVEDIRNAME"/%s/%s", FS_Savegamedir(), dst, found+len);	// was FS_Gamedir()
			FS_CopyFile (name, name2);

			if (i == 0)
			{
				// change sav to sv2
				l = (int)strlen(name);
			//	strncpy (name+l-3, "sv2");
				Q_strncpyz (name+l-3, "sv2", sizeof(name)-l+3);
				l = (int)strlen(name2);
			//	strncpy (name2+l-3, "sv2");
				Q_strncpyz (name2+l-3, "sv2", sizeof(name2)-l+3);
				FS_CopyFile (name, name2);
			}
			found = Sys_FindNext( 0, 0 );
		}
		Sys_FindClose ();
	}
}

#ifdef COMPRESSED_SAVEGAMES
void		CM_WritePortalState (fileHandle_t f);
#else // COMPRESSED_SAVEGAMES
void		CM_WritePortalState (FILE *f);
#endif // COMPRESSED_SAVEGAMES
/*
==============
SV_WriteLevelFile

==============
*/
void SV_WriteLevelFile (void)
{
#ifdef COMPRESSED_SAVEGAMES
	char			name[MAX_OSPATH], zipName[MAX_QPATH], intName[MAX_QPATH];
	fileHandle_t	f;

	Com_DPrintf("SV_WriteLevelFile()\n");

//	Com_sprintf (zipName, sizeof(zipName), "/save/current/%s.savz", sv.name);
	Com_sprintf (zipName, sizeof(zipName), "/"SAVEDIRNAME"/current/%s.savz", sv.name);
	Com_sprintf (intName, sizeof(intName), "%s.sv2", sv.name);
	FS_FOpenCompressedFile (zipName, intName, &f, FS_WRITE);
	if (!f)
	{
		Com_Printf ("Failed to open %s\n", zipName);
		return;
	}
	FS_Write (sv.configstrings, sizeof(sv.configstrings), f);
	CM_WritePortalState (f);
	FS_FCloseFile (f);
#else // COMPRESSED_SAVEGAMES
	char	name[MAX_OSPATH];
	FILE	*f;

	Com_DPrintf("SV_WriteLevelFile()\n");

//	Com_sprintf (name, sizeof(name), "%s/save/current/%s.sv2", FS_Savegamedir(), sv.name);	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/current/%s.sv2", FS_Savegamedir(), sv.name);	// was FS_Gamedir()
	f = fopen(name, "wb");
	if (!f)
	{
		Com_Printf ("Failed to open %s\n", name);
		return;
	}
	fwrite (sv.configstrings, sizeof(sv.configstrings), 1, f);
	CM_WritePortalState (f);
	fclose (f);
#endif // COMPRESSED_SAVEGAMES

//	Com_sprintf (name, sizeof(name), "%s/save/current/%s.sav", FS_Savegamedir(), sv.name);	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/current/%s.sav", FS_Savegamedir(), sv.name);	// was FS_Gamedir()
	ge->WriteLevel (name);
#ifdef COMPRESSED_SAVEGAMES
	// compress .sav into .savz
//	Com_sprintf (zipName, sizeof(zipName), "/save/current/%s.savz", sv.name);
	Com_sprintf (zipName, sizeof(zipName), "/"SAVEDIRNAME"/current/%s.savz", sv.name);
	Com_sprintf (intName, sizeof(intName), "%s.sav", sv.name);
	FS_CompressFile (name, zipName, intName);

	// delete .sav
	remove (name);
#endif // COMPRESSED_SAVEGAMES
}

void	CM_ReadPortalState (fileHandle_t f);
/*
==============
SV_ReadLevelFile

==============
*/
void SV_ReadLevelFile (void)
{
	char			name[MAX_OSPATH];
	fileHandle_t	f;
#ifdef COMPRESSED_SAVEGAMES // check for compressed .savz file here
	char			zipName[MAX_QPATH], intName[MAX_QPATH];
	FILE			*fp;
#endif // COMPRESSED_SAVEGAMES

	Com_DPrintf("SV_ReadLevelFile()\n");

#ifdef COMPRESSED_SAVEGAMES // check for compressed .savz file here
//	Com_sprintf (zipName, sizeof(zipName), "save/current/%s.savz", sv.name);
	Com_sprintf (zipName, sizeof(zipName), SAVEDIRNAME"/current/%s.savz", sv.name);
	Com_sprintf (intName, sizeof(intName), "%s.sv2", sv.name);
	
	FS_FOpenCompressedFile (zipName, intName, &f, FS_READ);
	if (f)
	{
		FS_Read (sv.configstrings, sizeof(sv.configstrings), f);
		CM_ReadPortalState (f);
		FS_FCloseFile(f);
	}
	else 
#endif // COMPRESSED_SAVEGAMES
	{
	//	Com_sprintf (name, sizeof(name), "save/current/%s.sv2", sv.name);
		Com_sprintf (name, sizeof(name), SAVEDIRNAME"/current/%s.sv2", sv.name);
		FS_FOpenFile (name, &f, FS_READ);
		if (!f)
		{
			Com_Printf ("Failed to open %s\n", name);
			return;
		}
		FS_Read (sv.configstrings, sizeof(sv.configstrings), f);
		CM_ReadPortalState (f);
		FS_FCloseFile(f);
	}

//	Com_sprintf (name, sizeof(name), "%s/save/current/%s.sav", FS_Savegamedir(), sv.name);	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/current/%s.sav", FS_Savegamedir(), sv.name);	// was FS_Gamedir()
#ifdef COMPRESSED_SAVEGAMES
	// check for .sav; if not present, decompress from .savz
	fp = fopen (name, "rb");	
	if (!fp) {
	//	Com_sprintf (zipName, sizeof(zipName), "save/current/%s.savz", sv.name);
		Com_sprintf (zipName, sizeof(zipName), SAVEDIRNAME"/current/%s.savz", sv.name);
		Com_sprintf (intName, sizeof(intName), "%s.sav", sv.name);
		FS_DecompressFile (name, zipName, intName);
	}
	else {
		fclose (fp);
	}
#endif // COMPRESSED_SAVEGAMES
	ge->ReadLevel (name);
}


/*
==============
SV_WriteScreenshot
==============
*/
void SV_WriteScreenshot (void)
{
	char	name[MAX_OSPATH];

	if (dedicated->value) // can't do this in dedicated mode
		return;

	Com_DPrintf("SV_WriteScreenshot()\n");

//	Com_sprintf (name, sizeof(name), "%s/save/current/shot.jpg", FS_Savegamedir());	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/current/shot.jpg", FS_Savegamedir());	// was FS_Gamedir()

	R_ScaledScreenshot(name);
}


/*
==============
SV_WriteServerFile

==============
*/
void SV_WriteServerFile (qboolean autosave, qboolean quicksave)
{
	FILE	*f;
	cvar_t	*var;
	char	fileName[MAX_OSPATH], varName[128], string[128];
	char	comment[32], infoHeader[10], mapname[32], dummy[18];
	time_t	aclock;
	struct tm	*newtime;
	int		i;

	Com_DPrintf("SV_WriteServerFile(%s)\n", autosave ? "true" : "false");

//	Com_sprintf (fileName, sizeof(fileName), "%s/save/current/server.ssv", FS_Savegamedir());	// was FS_Gamedir()
	Com_sprintf (fileName, sizeof(fileName), "%s/"SAVEDIRNAME"/current/server.ssv", FS_Savegamedir());	// was FS_Gamedir()
	f = fopen (fileName, "wb");
	if (!f)
	{
		Com_Printf ("Couldn't write %s\n", fileName);
		return;
	}
	// write the comment field
	memset (comment, 0, sizeof(comment));

	time (&aclock);
	newtime = localtime (&aclock);

	if (!autosave)
	{
		Com_sprintf (comment,sizeof(comment), "%2i:%i%i %2i/%2i  ", newtime->tm_hour
			, newtime->tm_min/10, newtime->tm_min%10,
			newtime->tm_mon+1, newtime->tm_mday);
		strncat (comment, sv.configstrings[CS_NAME], sizeof(comment)-1-strlen(comment) );
	}
	else
	{	// autosaved
		Com_sprintf (comment, sizeof(comment), "ENTERING %s", sv.configstrings[CS_NAME]);
	}

	fwrite (comment, 1, sizeof(comment), f);

	// write the mapcmd
	fwrite (svs.mapcmd, 1, sizeof(svs.mapcmd), f);

	// write extra save info in first cvar slot
	memset (infoHeader, 0, sizeof(infoHeader));
	memset (mapname, 0, sizeof(mapname));
	memset (comment, 0, sizeof(comment));
	memset (dummy, 0, sizeof(dummy));
	memset (string, 0, sizeof(string));

	Com_sprintf (infoHeader, sizeof(infoHeader), "KMQ2SSV01");
	Com_sprintf (mapname, sizeof(mapname), "%s", sv.configstrings[CS_NAME]);

	// replace newline in mapname with space
	for (i = 0; i < sizeof(mapname)-1; i++) {
		if ( (mapname[i] == '\n') || (mapname[i] == '\r') )
			mapname[i] = ' ';
	}
	
	if (autosave)
		Com_sprintf (comment, sizeof(comment), "AUTO SAVE");
	else if (quicksave)
		Com_sprintf (comment, sizeof(comment), "QUICK SAVE");

	fwrite (infoHeader, 1, sizeof(infoHeader), f);
	fwrite (newtime, 1, sizeof(struct tm), f);
	fwrite (mapname, 1, sizeof(mapname), f);
	fwrite (comment, 1, sizeof(comment), f);
	fwrite (dummy, 1, sizeof(dummy), f);
	fwrite (string, 1, sizeof(string), f);

	// write all CVAR_LATCH cvars
	// these will be things like coop, skill, deathmatch, etc
	for (var = cvar_vars ; var ; var=var->next)
	{
		if (!(var->flags & CVAR_LATCH))
			continue;
		if (var->flags & CVAR_SAVE_IGNORE)	// latched vars that are not saved (game, etc)
			continue;
		if (strlen(var->name) >= sizeof(varName)-1
			|| strlen(var->string) >= sizeof(string)-1)
		{
			Com_Printf ("Cvar too long: %s = %s\n", var->name, var->string);
			continue;
		}
		memset (varName, 0, sizeof(varName));
		memset (string, 0, sizeof(string));
	//	strncpy (varName, var->name);
	//	strncpy (string, var->string);
		Q_strncpyz (varName, var->name, sizeof(varName));
		Q_strncpyz (string, var->string, sizeof(string));
		fwrite (varName, 1, sizeof(varName), f);
		fwrite (string, 1, sizeof(string), f);
	}

	fclose (f);

	// write game state
//	Com_sprintf (fileName, sizeof(fileName), "%s/save/current/game.ssv", FS_Savegamedir());	// was FS_Gamedir()
	Com_sprintf (fileName, sizeof(fileName), "%s/"SAVEDIRNAME"/current/game.ssv", FS_Savegamedir());	// was FS_Gamedir()
	ge->WriteGame (fileName, autosave);
}


/*
==============
SV_ReadServerFile

==============
*/
void SV_ReadServerFile (void)
{
	fileHandle_t	f;
	char	fileName[MAX_OSPATH], varName[128], string[128];
	char	comment[32];
	char	mapcmd[MAX_TOKEN_CHARS];

	Com_DPrintf("SV_ReadServerFile()\n");

//	Com_sprintf (fileName, sizeof(fileName), "save/current/server.ssv");
	Com_sprintf (fileName, sizeof(fileName), SAVEDIRNAME"/current/server.ssv");
	FS_FOpenFile (fileName, &f, FS_READ);
	if (!f)
	{
		Com_Printf ("Couldn't read %s\n", fileName);
		return;
	}
	// read the comment field
	FS_Read (comment, sizeof(comment), f);

	// read the mapcmd
	FS_Read (mapcmd, sizeof(mapcmd), f);

	// read all CVAR_LATCH cvars
	// these will be things like coop, skill, deathmatch, etc
	while (1)
	{
		if (!FS_FRead (varName, 1, sizeof(varName), f))
			break;
		FS_Read (string, sizeof(string), f);

		// skip first cvar slot in KMQ2 0.21 and later saves (embedded extra save info)
		if ( !strncmp(varName, "KMQ2SSV", 7) ) {
		//	Com_DPrintf ("Skipping extra save info\n");
			continue;
		}

		// don't load game, basegame, engine name/version, or sys_* cvars from savegames
		if ( strcmp(varName, "game") && strncmp(varName, "basegame", 8)
			&& strncmp(varName, "sv_engine", 9) && strncmp(varName, "cl_engine", 9)
			&& strncmp(varName, "sys_", 4) ) {
			Com_DPrintf ("Set %s = %s\n", varName, string);
			Cvar_ForceSet (varName, string);
		}
	}

	FS_FCloseFile(f);

	// start a new game fresh with new cvars
	SV_InitGame ();

//	strncpy (svs.mapcmd, mapcmd);
	Q_strncpyz (svs.mapcmd, mapcmd, sizeof(svs.mapcmd));

	// read game state
//	Com_sprintf (fileName, sizeof(fileName), "%s/save/current/game.ssv", FS_Savegamedir());	// was FS_Gamedir()
	Com_sprintf (fileName, sizeof(fileName), "%s/"SAVEDIRNAME"/current/game.ssv", FS_Savegamedir());	// was FS_Gamedir()
	ge->ReadGame (fileName);
}


//=========================================================




/*
==================
SV_DemoMap_f

Puts the server in demo mode on a specific map/cinematic
==================
*/
void SV_DemoMap_f (void)
{
	// Knightmare- force off DM, CTF mode
	Cvar_SetValue( "ttctf", 0);
	Cvar_SetValue( "ctf", 0);
	Cvar_SetValue( "deathmatch", 0);

	SV_Map (true, Cmd_Argv(1), false );
}

/*
==================
SV_GameMap_f

Saves the state of the map just being exited and goes to a new map.

If the initial character of the map string is '*', the next map is
in a new unit, so the current savegame directory is cleared of
map files.

Example:

*inter.cin+jail

Clears the archived maps, plays the inter.cin cinematic, then
goes to map jail.bsp.
==================
*/
void SV_GameMap_f (void)
{
	char		*map;
	int			i, l;
	client_t	*cl;
	qboolean	*savedInuse;

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("USAGE: gamemap <map>\n");
		return;
	}

	Com_DPrintf("SV_GameMap(%s)\n", Cmd_Argv(1));

//	FS_CreatePath (va("%s/save/current/", FS_Savegamedir()));	// was FS_Gamedir()
	FS_CreatePath (va("%s/"SAVEDIRNAME"/current/", FS_Savegamedir()));	// was FS_Gamedir()

	// check for clearing the current savegame
	map = Cmd_Argv(1);
	if (map[0] == '*')
	{
		// wipe all the *.sav files
		SV_WipeSavegame ("current");
	}
	else
	{
		// save the map just exited
		if (sv.state == ss_game)
		{
			// clear all the client inuse flags before saving so that
			// when the level is re-entered, the clients will spawn
			// at spawn points instead of occupying body shells
			savedInuse = malloc(maxclients->value * sizeof(qboolean));
			for (i=0,cl=svs.clients ; i<maxclients->value; i++,cl++)
			{
				savedInuse[i] = cl->edict->inuse;
				cl->edict->inuse = false;
			}

			SV_WriteLevelFile ();

			// we must restore these for clients to transfer over correctly
			for (i=0,cl=svs.clients ; i<maxclients->value; i++,cl++)
				cl->edict->inuse = savedInuse[i];
			free (savedInuse);
		}
	}

	// start up the next map
	SV_Map (false, Cmd_Argv(1), false );

	// archive server state
	strncpy (svs.mapcmd, Cmd_Argv(1), sizeof(svs.mapcmd)-1);

	// copy off the level to the autosave slot
	// Knightmare- don't do this in deathmatch or for cinematics
	l = (int)strlen(map);
	//l = strcspn(map, "+");
	if (!dedicated->value && !Cvar_VariableValue("deathmatch")
		&& Q_strcasecmp (map+l-4, ".cin") && Q_strcasecmp (map+l-4, ".roq")
		&& Q_strcasecmp (map+l-4, ".pcx"))
	{
		SV_WriteServerFile (true, false);
		SV_CopySaveGame ("current", "kmq2save0");
	}
}

/*
==================
SV_Map_f

Goes directly to a given map without any savegame archiving.
For development work
==================
*/
void SV_Map_f (void)
{
	char	*map;
	char	expanded[MAX_QPATH];

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("USAGE: map <mapname>\n");
		return;
	}

	// if not a pcx, demo, or cinematic, check to make sure the level exists
	map = Cmd_Argv(1);
	if (!strstr (map, "."))
	{
		Com_sprintf (expanded, sizeof(expanded), "maps/%s.bsp", map);

		if (FS_LoadFile (expanded, NULL) == -1)
		{
			Com_Printf ("Can't find %s\n", expanded);
			//if (!dedicated->value && cls.state != ca_connected) // Knightmare added
			//	CL_Drop ();
			return;
		}
	}

	sv.state = ss_dead;		// don't save current level when changing
	SV_WipeSavegame("current");
	SV_GameMap_f ();
}

/*
=====================================================================

  SAVEGAMES

=====================================================================
*/
extern	char *load_saveshot;
char sv_loadshotname[MAX_QPATH];

/*
==============
SV_Loadgame_f

==============
*/
void SV_Loadgame_f (void)
{
	char	name[MAX_OSPATH];
	FILE	*f;
	char	*dir;

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("USAGE: loadgame <directory>\n");
		return;
	}

	Com_Printf ("Loading game...\n");

	dir = Cmd_Argv(1);
	if (strstr (dir, "..") || strstr (dir, "/") || strstr (dir, "\\") )
	{
		Com_Printf ("Bad savedir.\n");
	}

	// make sure the server.ssv file exists
//	Com_sprintf (name, sizeof(name), "%s/save/%s/server.ssv", FS_Savegamedir(), Cmd_Argv(1));	// was FS_Gamedir()
	Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/%s/server.ssv", FS_Savegamedir(), Cmd_Argv(1));	// was FS_Gamedir()
	f = fopen (name, "rb");
	if (!f)
	{
		Com_Printf ("No such savegame: %s\n", name);
		return;
	}
	fclose (f);

	// Knightmare- set saveshot name
	if ( !dedicated->value && (!strcmp(Cmd_Argv(1), "quick") || !strcmp(Cmd_Argv(1), "quik")) )
	{
	//	Com_sprintf(sv_loadshotname, sizeof(sv_loadshotname), "save/%s/shot.jpg", Cmd_Argv(1));
		Com_sprintf(sv_loadshotname, sizeof(sv_loadshotname), SAVEDIRNAME"/%s/shot.jpg", Cmd_Argv(1));
		R_FreePic (sv_loadshotname);
	//	Com_sprintf(sv_loadshotname, sizeof(sv_loadshotname), "/save/%s/shot.jpg", Cmd_Argv(1));
		Com_sprintf(sv_loadshotname, sizeof(sv_loadshotname), "/"SAVEDIRNAME"/%s/shot.jpg", Cmd_Argv(1));
		load_saveshot = sv_loadshotname;
	}

	SV_CopySaveGame (Cmd_Argv(1), "current");

	SV_ReadServerFile ();

	// go to the map
	sv.state = ss_dead;		// don't save current level when changing
	SV_Map (false, svs.mapcmd, true);
}



/*
==============
SV_Savegame_f

==============
*/
//extern	char fs_gamedir[MAX_OSPATH];

void SV_Savegame_f (void)
{
	char	*dir;
	qboolean	quicksave = false;

	if (sv.state != ss_game)
	{
		Com_Printf ("You must be in a game to save.\n");
		return;
	}

	// Knightmare- fs_gamedir may be getting messed up, causing it to occasinally save in the root dir,
	// thus leading to a hang on game loads, so we reset it here.
	// Root cause of this was fixed in FS_SetGamedir(), this fix is no longer needed
/*	if (!fs_gamedir[0])
	{
		if (fs_gamedirvar->string[0])
			Com_sprintf (fs_gamedir, sizeof(fs_gamedir), "%s/%s", fs_basedir->string, fs_gamedirvar->string);
	}
*/

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("USAGE: savegame <directory>\n");
		return;
	}

	if (Cvar_VariableValue("deathmatch"))
	{
		Com_Printf ("Can't savegame in a deathmatch\n");
		return;
	}

	if (!strcmp (Cmd_Argv(1), "current"))
	{
		Com_Printf ("Can't save to 'current'\n");
		return;
	}

	quicksave = ( !dedicated->value && (!strcmp(Cmd_Argv(1), "quick") || !strcmp(Cmd_Argv(1), "quik")) );

	// Knightmare- grab screen for quicksave
	if ( !dedicated->value && (!strcmp(Cmd_Argv(1), "quick") || !strcmp(Cmd_Argv(1), "quik")) )
		R_GrabScreen();

	if (maxclients->value == 1 && svs.clients[0].edict->client->ps.stats[STAT_HEALTH] <= 0)
	{
		Com_Printf ("\nCan't savegame while dead!\n");
		return;
	}

	dir = Cmd_Argv(1);
	if (strstr (dir, "..") || strstr (dir, "/") || strstr (dir, "\\") )
	{
		Com_Printf ("Bad savedir.\n");
	}

	Com_Printf (S_COLOR_CYAN"Saving game \"%s\"...\n", dir);

	// archive current level, including all client edicts.
	// when the level is reloaded, they will be shells awaiting
	// a connecting client
	SV_WriteLevelFile ();

	// save server state
	SV_WriteServerFile (false, quicksave);

	// take screenshot
	SV_WriteScreenshot ();

	// copy it off
	SV_CopySaveGame ("current", dir);

	Com_Printf (S_COLOR_CYAN"Done.\n");
}

//===============================================================

/*
==================
SV_Kick_f

Kick a user off of the server
==================
*/
void SV_Kick_f (void)
{
	if (!svs.initialized)
	{
		Com_Printf ("No server running.\n");
		return;
	}

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("Usage: kick <userid>\n");
		return;
	}

	if (!SV_SetPlayer ())
		return;

	SV_BroadcastPrintf (PRINT_HIGH, "%s was kicked\n", sv_client->name);
	// print directly, because the dropped client won't get the
	// SV_BroadcastPrintf message
	SV_ClientPrintf (sv_client, PRINT_HIGH, "You were kicked from the game\n");
	SV_DropClient (sv_client);
	sv_client->lastmessage = svs.realtime;	// min case there is a funny zombie
}


/*
================
SV_Status_f
================
*/
void SV_Status_f (void)
{
	int			i, j, l;
	client_t	*cl;
	char		*s;
	int			ping;
	if (!svs.clients)
	{
		Com_Printf ("No server running.\n");
		return;
	}
	Com_Printf ("map              : %s\n", sv.name);

	Com_Printf ("num score ping name            lastmsg address               qport \n");
	Com_Printf ("--- ----- ---- --------------- ------- --------------------- ------\n");
	for (i=0,cl=svs.clients ; i<maxclients->value; i++,cl++)
	{
		if (!cl->state)
			continue;
		Com_Printf ("%3i ", i);
		Com_Printf ("%5i ", cl->edict->client->ps.stats[STAT_FRAGS]);

		if (cl->state == cs_connected)
			Com_Printf ("CNCT ");
		else if (cl->state == cs_zombie)
			Com_Printf ("ZMBI ");
		else
		{
			ping = cl->ping < 9999 ? cl->ping : 9999;
			Com_Printf ("%4i ", ping);
		}

		Com_Printf ("%s", cl->name);
		l = 16 - (int)strlen(cl->name);
		for (j=0 ; j<l ; j++)
			Com_Printf (" ");

		Com_Printf ("%7i ", svs.realtime - cl->lastmessage );

		s = NET_AdrToString ( cl->netchan.remote_address);
		Com_Printf ("%s", s);
		l = 22 - (int)strlen(s);
		for (j=0 ; j<l ; j++)
			Com_Printf (" ");
		
		Com_Printf ("%5i", cl->netchan.qport);

		Com_Printf ("\n");
	}
	Com_Printf ("\n");
}

/*
==================
SV_ConSay_f
==================
*/
void SV_ConSay_f(void)
{
	client_t *client;
	int		j;
	char	*p;
	char	text[1024];

	if (Cmd_Argc () < 2)
		return;

//	strncpy (text, "console: ");
	Q_strncpyz (text, "console: ", sizeof(text));
	p = Cmd_Args();

	if (*p == '"')
	{
		p++;
		p[strlen(p)-1] = 0;
	}

//	strncat(text, p);
	Q_strncatz(text, p, sizeof(text));

	for (j = 0, client = svs.clients; j < maxclients->value; j++, client++)
	{
		if (client->state != cs_spawned)
			continue;
		SV_ClientPrintf(client, PRINT_CHAT, "%s\n", text);
	}
}


/*
==================
SV_Heartbeat_f
==================
*/
void SV_Heartbeat_f (void)
{
	svs.last_heartbeat = -9999999;
}


/*
===========
SV_Serverinfo_f

  Examine or change the serverinfo string
===========
*/
void SV_Serverinfo_f (void)
{
	Com_Printf ("Server info settings:\n");
	Info_Print (Cvar_Serverinfo());
}


/*
===========
SV_DumpUser_f

Examine all a users info strings
===========
*/
void SV_DumpUser_f (void)
{
	if (Cmd_Argc() != 2)
	{
		Com_Printf ("Usage: info <userid>\n");
		return;
	}

	if (!SV_SetPlayer ())
		return;

	Com_Printf ("userinfo\n");
	Com_Printf ("--------\n");
	Info_Print (sv_client->userinfo);

}


/*
===================
SV_StartMod
===================
*/
void SV_StartMod (char *mod)
{
	// killserver, start mod, unbind keys, exec configs, and start demos
	Cbuf_AddText ("killserver\n");
	Cbuf_AddText (va("game %s\n", mod));
	Cbuf_AddText ("unbindall\n");
	Cbuf_AddText ("exec default.cfg\n");
	Cbuf_AddText ("exec kmq2config.cfg\n");
	Cbuf_AddText ("exec autoexec.cfg\n");
	Cbuf_AddText ("d1\n");
}

/*
===================
SV_ChangeGame_f

switch to a different mod
===================
*/
void SV_ChangeGame_f (void)
{
	if (Cmd_Argc() < 2)
	{
		Com_Printf ("changegame <gamedir> : change game directory\n");
		return;
	}
	SV_StartMod (Cmd_Argv(1));
}


/*
==============
SV_ServerRecord_f

Begins server demo recording.  Every entity and every message will be
recorded, but no playerinfo will be stored.  Primarily for demo merging.
==============
*/
void SV_ServerRecord_f (void)
{
	char	name[MAX_OSPATH];
	char	buf_data[32768];
	sizebuf_t	buf;
	int		len;
	int		i;

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("serverrecord <demoname>\n");
		return;
	}

	if (svs.demofile)
	{
		Com_Printf ("Already recording.\n");
		return;
	}

	if (sv.state != ss_game)
	{
		Com_Printf ("You must be in a level to record.\n");
		return;
	}

	//
	// open the demo file
	//
	Com_sprintf (name, sizeof(name), "%s/demos/%s.dm2", FS_Savegamedir(), Cmd_Argv(1));	// was FS_Gamedir()

	Com_Printf ("recording to %s.\n", name);
	FS_CreatePath (name);
	svs.demofile = fopen (name, "wb");
	if (!svs.demofile)
	{
		Com_Printf ("ERROR: couldn't open.\n");
		return;
	}

	// setup a buffer to catch all multicasts
	SZ_Init (&svs.demo_multicast, svs.demo_multicast_buf, sizeof(svs.demo_multicast_buf));

	//
	// write a single giant fake message with all the startup info
	//
	SZ_Init (&buf, buf_data, sizeof(buf_data));

	//
	// serverdata needs to go over for all types of servers
	// to make sure the protocol is right, and to set the gamedir
	//
	// send the serverdata
	MSG_WriteByte (&buf, svc_serverdata);
	MSG_WriteLong (&buf, PROTOCOL_VERSION);
	MSG_WriteLong (&buf, svs.spawncount);
	// 2 means server demo
	MSG_WriteByte (&buf, 2);	// demos are always attract loops
	MSG_WriteString (&buf, Cvar_VariableString ("gamedir"));
	MSG_WriteShort (&buf, -1);
	// send full levelname
	MSG_WriteString (&buf, sv.configstrings[CS_NAME]);

	for (i=0 ; i<MAX_CONFIGSTRINGS ; i++)
		if (sv.configstrings[i][0])
		{
			MSG_WriteByte (&buf, svc_configstring);
			MSG_WriteShort (&buf, i);
			MSG_WriteString (&buf, sv.configstrings[i]);
		}

	// write it to the demo file
	Com_DPrintf ("signon message length: %i\n", buf.cursize);
	len = LittleLong (buf.cursize);
	fwrite (&len, 4, 1, svs.demofile);
	fwrite (buf.data, buf.cursize, 1, svs.demofile);

	// the rest of the demo file will be individual frames
}


/*
==============
SV_ServerStop_f

Ends server demo recording
==============
*/
void SV_ServerStop_f (void)
{
	if (!svs.demofile)
	{
		Com_Printf ("Not doing a serverrecord.\n");
		return;
	}
	fclose (svs.demofile);
	svs.demofile = NULL;
	Com_Printf ("Recording completed.\n");
}


/*
===============
SV_KillServer_f

Kick everyone off, possibly in preparation for a new game

===============
*/
void SV_KillServer_f (void)
{
	if (!svs.initialized)
		return;

	SV_Shutdown ("Server was killed.\n", false);
	NET_Config ( false );	// close network sockets
}


/*
===============
SV_DumpEntities_f

Dumps entity sting to a text file with .ent extension
===============
*/
void SV_DumpEntities_f (void)
{
	FILE	*entFile;
	char	name[MAX_OSPATH];
//	char	outBuf[MAX_MAP_ENTSTRING];

	if (!svs.initialized)
	{
		Com_Printf ("SV_DumpEntities_f: No map loaded.\n");
		return;
	}

	Com_sprintf(name, sizeof(name), "%s/maps/%s.ent", FS_Savegamedir(), sv.name);	// was FS_Gamedir()
	FS_CreatePath(name);
	entFile = fopen(name, "w");

	if (!entFile) {
		Com_Printf ("SV_DumpEntities_f: Couldn't open %s to write.\n", name);
	}
	else {
		Com_Printf ("SV_DumpEntities_f: Dumping entstring to %s.\n", name);
	//	Q_strncpyz(outBuf, CM_EntityString(), sizeof (outBuf));
	//	fputs(outBuf, entFile);
		fputs(CM_EntityString(), entFile);
		fclose(entFile);
	}
}


/*
===============
SV_ServerCommand_f

Let the game dll handle a command
===============
*/
void SV_ServerCommand_f (void)
{
	if (!ge)
	{
		Com_Printf ("No game loaded.\n");
		return;
	}

	ge->ServerCommand();
}

//===========================================================

/*
==================
SV_InitOperatorCommands
==================
*/
void SV_InitOperatorCommands (void)
{
	Cmd_AddCommand ("heartbeat", SV_Heartbeat_f);
	Cmd_AddCommand ("kick", SV_Kick_f);
	Cmd_AddCommand ("status", SV_Status_f);
	Cmd_AddCommand ("serverinfo", SV_Serverinfo_f);
	Cmd_AddCommand ("dumpuser", SV_DumpUser_f);

	Cmd_AddCommand ("changegame", SV_ChangeGame_f); // Knightmare added

	Cmd_AddCommand ("map", SV_Map_f);
	Cmd_AddCommand ("demomap", SV_DemoMap_f);
	Cmd_AddCommand ("gamemap", SV_GameMap_f);
	Cmd_AddCommand ("setmaster", SV_SetMaster_f);

	if ( dedicated->value )
		Cmd_AddCommand ("say", SV_ConSay_f);

	Cmd_AddCommand ("serverrecord", SV_ServerRecord_f);
	Cmd_AddCommand ("serverstop", SV_ServerStop_f);

	Cmd_AddCommand ("save", SV_Savegame_f);
	Cmd_AddCommand ("load", SV_Loadgame_f);

	Cmd_AddCommand ("killserver", SV_KillServer_f);

	Cmd_AddCommand ("sv_dumpentities", SV_DumpEntities_f);

	Cmd_AddCommand ("sv", SV_ServerCommand_f);
}
