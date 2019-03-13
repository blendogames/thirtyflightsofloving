/*
Copyright (C) 1997-2001 Id Software, Inc.

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

// ui_startserver.c -- the start server menu 

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"


/*
=============================================================================

START SERVER MENU

=============================================================================
*/
static menuframework_s s_startserver_menu;

static menuaction_s	s_startserver_start_action;
static menuaction_s	s_startserver_dmoptions_action;
static menufield_s	s_timelimit_field;
static menufield_s	s_fraglimit_field;
static menufield_s	s_maxclients_field;
static menufield_s	s_hostname_field;
static menulist_s	s_startmap_list;
menulist_s			s_rules_box;
static menuaction_s	s_startserver_back_action;

#define M_UNSET 0
#define	M_MISSING 1
#define	M_FOUND 2
static	byte *ui_svr_mapshotvalid; // levelshot truth table

typedef enum {
	MAP_DM,
	MAP_COOP,
	MAP_CTF,
	MAP_3TCTF,
	NUM_MAPTYPES
} maptype_t;

typedef struct {
	int		index;
	char	*tokens;
} gametype_names_t;

gametype_names_t gametype_names[] = 
{
	{MAP_DM, "dm ffa team teamdm"},
	{MAP_COOP, "coop"},
	{MAP_CTF, "ctf"},
	{MAP_3TCTF, "3tctf"},
};

#define MAX_ARENAS 1024
#define MAX_ARENAS_TEXT 8192

static maptype_t ui_svr_maptype;
static int	ui_svr_nummaps;
static char **ui_svr_mapnames;
static int	ui_svr_listfile_nummaps;
static char **ui_svr_listfile_mapnames;
static int	ui_svr_arena_nummaps[NUM_MAPTYPES];
static char **ui_svr_arena_mapnames[NUM_MAPTYPES];

//=============================================================================

/*
===============
UI_ParseArenaFromFile
Partially from Q3 source
===============
*/
qboolean UI_ParseArenaFromFile (char *filename, char *shortname, char *longname, char *gametypes, size_t bufSize)
{
	int				len;
	fileHandle_t	f;
	char			buf[MAX_ARENAS_TEXT];
	char			*s, *token, *dest;

	len = FS_FOpenFile (filename, &f, FS_READ);
	if (!f) {
		Com_Printf (S_COLOR_RED "UI_ParseArenaFromFile: file not found: %s\n", filename);
		return false;
	}
	if (len >= MAX_ARENAS_TEXT) {
		Com_Printf (S_COLOR_RED "UI_ParseArenaFromFile: file too large: %s is %i, max allowed is %i", filename, len, MAX_ARENAS_TEXT);
		FS_FCloseFile (f);
		return false;
	}

	FS_Read (buf, len, f);
	buf[len] = 0;
	FS_FCloseFile (f);

	s = buf;
	// get the opening curly brace
	token = COM_Parse (&s);
	if (!token) {
		Com_Printf ("UI_ParseArenaFromFile: unexpected EOF\n");
		return false;
	}
	if (token[0] != '{') {
		Com_Printf ("UI_ParseArenaFromFile: found %s when expecting {\n", token);
		return false;
	}

	// go through all the parms
	while (s < (buf + len))
	{
		dest = NULL;
		token = COM_Parse (&s);
		if (token && (token[0] == '}')) break;
		if (!token || !s) {
			Com_Printf ("UI_ParseArenaFromFile: EOF without closing brace\n");
			break;
		}

		if (!Q_strcasecmp(token, "map"))
			dest = shortname;
		else if (!Q_strcasecmp(token, "longname"))
			dest = longname;
		else if (!Q_strcasecmp(token, "type"))
			dest = gametypes;
		if (dest)
		{
			token = COM_Parse (&s);
			if (!token) {
				Com_Printf ("UI_ParseArenaFromFile: unexpected EOF\n");
				return false;
			}
			if (token[0] == '}') {
				Com_Printf ("UI_ParseArenaFromFile: closing brace without data\n");
				break;
			}
			if (!s) {
				Com_Printf ("UI_ParseArenaFromFile: EOF without closing brace\n");
				break;
			}
		//	strncpy( dest, token );
			Q_strncpyz( dest, token, bufSize );
		}
	}
	if (!shortname || !strlen(shortname)) {
		Com_Printf (S_COLOR_RED "UI_ParseArenaFromFile: %s: map field not found\n", filename);
		return false;
	}
	if (!strlen(longname))
		longname = shortname;
	return true;
}


/*
===============
UI_SortArenas
===============
*/
void UI_SortArenas (char **list, int len)
{
	int			i, j;
	char		*temp, *s1, *s2;
	qboolean	moved;

	if (!list || len < 2)
		return;

	for (i=(len-1); i>0; i--)
	{
		moved = false;
		for (j=0; j<i; j++)
		{
			if (!list[j]) break;
			s1 = strchr(list[j], '\n')+1;
			s2 = strchr(list[j+1], '\n')+1;
			if (Q_stricmp(s1, s2) > 0)
			{
				temp = list[j];
				list[j] = list[j+1];
				list[j+1] = temp;
				moved = true;
			}
		}
		if (!moved) break; // done sorting
	}
}


/*
===============
UI_LoadArenas
===============
*/
void UI_LoadArenas (void)
{
	char		*p, *s, *s2, *tok, *tok2;
	char		**arenafiles;
	char		**tmplist = 0;
	int			i, j, narenas = 0, narenanames = 0;
	qboolean	type_supported[NUM_MAPTYPES];

	//
	// free existing lists and malloc new ones
	//
	for (i=0; i<NUM_MAPTYPES; i++)
	{
		if (ui_svr_arena_mapnames[i])
			FS_FreeFileList (ui_svr_arena_mapnames[i], ui_svr_arena_nummaps[i]);
		ui_svr_arena_nummaps[i] = 0;
		ui_svr_arena_mapnames[i] = malloc( sizeof( char * ) * MAX_ARENAS );
		memset( ui_svr_arena_mapnames[i], 0, sizeof( char * ) * MAX_ARENAS );
	}

	tmplist = malloc( sizeof( char * ) * MAX_ARENAS );
	memset( tmplist, 0, sizeof( char * ) * MAX_ARENAS );

	//
	// check in paks for .arena files
	//
	if (arenafiles = FS_ListPak("scripts/", &narenas))
	{
		for (i=0; i<narenas && narenanames<MAX_ARENAS; i++)
		{
			if (!arenafiles || !arenafiles[i])
				continue;

			p = arenafiles[i];

			if (!strstr(p, ".arena"))
				continue;

			if (!FS_ItemInList(p, narenanames, tmplist)) // check if already in list
			{
				char	shortname[MAX_TOKEN_CHARS];
				char	longname[MAX_TOKEN_CHARS];
				char	gametypes[MAX_TOKEN_CHARS];
				char	scratch[200];
				if (UI_ParseArenaFromFile (p, shortname, longname, gametypes, MAX_TOKEN_CHARS))
				{
					Com_sprintf(scratch, sizeof(scratch), "%s\n%s", longname, shortname);
					
					for (j=0; j<NUM_MAPTYPES; j++)
						type_supported[j] = false;
					s = gametypes;
					tok = strdup(COM_Parse (&s));
					while (s != NULL)
					{
						for (j=0; j<NUM_MAPTYPES; j++)
						{
							s2 = gametype_names[j].tokens;
							tok2 = COM_Parse (&s2);
							while (s2 != NULL) {
								if ( !Q_strcasecmp(tok, tok2) )
									type_supported[j] = true;
								tok2 = COM_Parse (&s2);
							}
						}
						if (tok)	free (tok);
						tok = strdup(COM_Parse(&s));
					}
					if (tok)	free (tok);

					for (j=0; j<NUM_MAPTYPES; j++)
						if (type_supported[j]) {
							ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]] = malloc(strlen( scratch ) + 1);
						//	strncpy(ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]], scratch);
							Q_strncpyz(ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]], scratch, strlen( scratch ) + 1);
							ui_svr_arena_nummaps[j]++;
						}

					//Com_Printf ("UI_LoadArenas: successfully loaded arena file %s: mapname: %s levelname: %s gametypes: %s\n", p, shortname, longname, gametypes);
					narenanames++;
					FS_InsertInList(tmplist, strdup(p), narenanames, 0); // add to list
				}
			}
		}
	}
	if (narenas)
		FS_FreeFileList (arenafiles, narenas);

	if (narenanames)
		FS_FreeFileList (tmplist, narenanames);

	for (i=0; i<NUM_MAPTYPES; i++)
		UI_SortArenas (ui_svr_arena_mapnames[i], ui_svr_arena_nummaps[i]);

//	Com_Printf ("UI_LoadArenas: loaded %i arena file(s)\n", narenanames);
//	for (i=0; i<NUM_MAPTYPES; i++)
//		Com_Printf ("%s: %i arena file(s)\n", gametype_names[i].tokens, ui_svr_arena_nummaps[i]);
}


/*
===============
UI_LoadMapList
===============
*/
void UI_LoadMapList (void)
{
	char	*buffer, *s;
	char	mapsname[1024];
	int		i, length;
	FILE	*fp;

	//
	// free existing list
	//
	if (ui_svr_listfile_mapnames)
		FS_FreeFileList (ui_svr_listfile_mapnames, ui_svr_listfile_nummaps);
	ui_svr_listfile_nummaps = 0;

	//
	// load the list of map names
	//
	Com_sprintf( mapsname, sizeof( mapsname ), "%s/maps.lst", FS_Gamedir() );
	if ( ( fp = fopen( mapsname, "rb" ) ) == 0 )
	{
		if ( ( length = FS_LoadFile( "maps.lst", ( void ** ) &buffer ) ) == -1 )
			Com_Error( ERR_DROP, "couldn't find maps.lst\n" );
	}
	else
	{
#ifdef _WIN32
		length = filelength( fileno( fp  ) );
#else
		fseek(fp, 0, SEEK_END);
		length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
#endif
		buffer = malloc( length );
		fread( buffer, length, 1, fp );
	}

	s = buffer;

	i = 0;
	while (i < length)
	{
		if (s[i] == '\r')
			ui_svr_listfile_nummaps++;
		i++;
	}

	if (ui_svr_listfile_nummaps == 0)
	{	// hack in a default map list
		ui_svr_listfile_nummaps = 1;
		buffer = "base1 \"Outer Base\"\n";
	}

	ui_svr_listfile_mapnames = malloc( sizeof( char * ) * ( ui_svr_listfile_nummaps + 1 ) );
	memset( ui_svr_listfile_mapnames, 0, sizeof( char * ) * ( ui_svr_listfile_nummaps + 1 ) );

	s = buffer;

	for (i = 0; i < ui_svr_listfile_nummaps; i++)
	{
		char	shortname[MAX_TOKEN_CHARS];
		char	longname[MAX_TOKEN_CHARS];
		char	scratch[200];

	//	strncpy( shortname, COM_Parse( &s ) );
	//	strncpy( longname, COM_Parse( &s ) );
		Q_strncpyz( shortname, COM_Parse( &s ), sizeof(shortname) );
		Q_strncpyz( longname, COM_Parse( &s ), sizeof(longname) );
		Com_sprintf( scratch, sizeof( scratch ), "%s\n%s", longname, shortname );
		ui_svr_listfile_mapnames[i] = malloc( strlen( scratch ) + 1 );
	//	strncpyz( ui_svr_listfile_mapnames[i], scratch );
		Q_strncpyz( ui_svr_listfile_mapnames[i], scratch, strlen( scratch ) + 1 );
	}
	ui_svr_listfile_mapnames[ui_svr_listfile_nummaps] = 0;

	if ( fp != 0 )
	{
		fp = 0;
		free( buffer );
	}
	else
		FS_FreeFile( buffer );

	UI_LoadArenas ();

	ui_svr_maptype = MAP_DM; // init maptype
}


/*
===============
UI_BuildMapList
===============
*/
void UI_BuildMapList (maptype_t maptype)
{
	int		i;

	if (ui_svr_mapnames)	free (ui_svr_mapnames);
	ui_svr_nummaps = ui_svr_listfile_nummaps + ui_svr_arena_nummaps[maptype];
	ui_svr_mapnames = malloc( sizeof( char * ) * ( ui_svr_nummaps + 1 ) );
	memset( ui_svr_mapnames, 0, sizeof( char * ) * ( ui_svr_nummaps + 1 ) );

	for (i = 0; i < ui_svr_nummaps; i++)
	{
		if (i < ui_svr_listfile_nummaps)
			ui_svr_mapnames[i] = ui_svr_listfile_mapnames[i];
		else
			ui_svr_mapnames[i] = ui_svr_arena_mapnames[maptype][i-ui_svr_listfile_nummaps];
	}
	ui_svr_mapnames[ui_svr_nummaps] = 0;
	ui_svr_maptype = maptype;

	if (s_startmap_list.curvalue >= ui_svr_nummaps) // paranoia
		s_startmap_list.curvalue = 0;
}	


/*
===============
UI_RefreshMapList
===============
*/
void UI_RefreshMapList (maptype_t maptype)
{
	int		i;

	if (maptype == ui_svr_maptype) // no change
		return;

	// reset startmap if it's in the part of the list that changed
	if (s_startmap_list.curvalue >= ui_svr_listfile_nummaps)
		s_startmap_list.curvalue = 0;

	UI_BuildMapList (maptype);
	s_startmap_list.itemnames = ui_svr_mapnames;
	for (i=0; s_startmap_list.itemnames[i]; i++);
	s_startmap_list.numitemnames = i;

	// levelshot found table
	if (ui_svr_mapshotvalid)	free(ui_svr_mapshotvalid);
	ui_svr_mapshotvalid = malloc( sizeof( byte ) * ( ui_svr_nummaps + 1 ) );
	memset( ui_svr_mapshotvalid, 0, sizeof( byte ) * ( ui_svr_nummaps + 1 ) );
	// register null levelshot
	if (ui_svr_mapshotvalid[ui_svr_nummaps] == M_UNSET) {	
		if (R_DrawFindPic("/gfx/ui/noscreen.pcx"))
			ui_svr_mapshotvalid[ui_svr_nummaps] = M_FOUND;
		else
			ui_svr_mapshotvalid[ui_svr_nummaps] = M_MISSING;
	}
}



//=============================================================================


void DMOptionsFunc (void *self)
{
	if (s_rules_box.curvalue == 1)
		return;
	M_Menu_DMOptions_f();
}

void RulesChangeFunc (void *self)
{
	if (s_rules_box.curvalue == 0) 	// DM
	{
		s_maxclients_field.generic.statusbar = NULL;
		if (atoi(s_maxclients_field.buffer) <= 8) // set default of 8
		//	strncpy( s_maxclients_field.buffer, "8" );
			Q_strncpyz( s_maxclients_field.buffer, "8", sizeof(s_maxclients_field.buffer) );
		s_startserver_dmoptions_action.generic.statusbar = NULL;
		UI_RefreshMapList (MAP_DM);
	}
	else if (s_rules_box.curvalue == 1)		// coop				// PGM
	{
		s_maxclients_field.generic.statusbar = "4 maximum for cooperative";
		if (atoi(s_maxclients_field.buffer) > 4)
		//	strncpy( s_maxclients_field.buffer, "4" );
			Q_strncpyz( s_maxclients_field.buffer, "4", sizeof(s_maxclients_field.buffer) );
		s_startserver_dmoptions_action.generic.statusbar = "N/A for cooperative";
		UI_RefreshMapList (MAP_COOP);
	}
	else if (s_rules_box.curvalue == 2) // CTF
	{
		s_maxclients_field.generic.statusbar = NULL;
		if (atoi(s_maxclients_field.buffer) <= 12) // set default of 12
		//	strncpy( s_maxclients_field.buffer, "12" );
			Q_strncpyz( s_maxclients_field.buffer, "12", sizeof(s_maxclients_field.buffer) );
		s_startserver_dmoptions_action.generic.statusbar = NULL;
		UI_RefreshMapList (MAP_CTF);
	}
	else if (s_rules_box.curvalue == 3) // 3Team CTF
	{
		s_maxclients_field.generic.statusbar = NULL;
		if (atoi(s_maxclients_field.buffer) <= 18) // set default of 18
		//	strncpy( s_maxclients_field.buffer, "18" );
			Q_strncpyz( s_maxclients_field.buffer, "18", sizeof(s_maxclients_field.buffer) );
		s_startserver_dmoptions_action.generic.statusbar = NULL;
		UI_RefreshMapList (MAP_3TCTF);
	}
	// ROGUE GAMES
	else if (roguepath() && s_rules_box.curvalue == 4) // tag	
	{
		s_maxclients_field.generic.statusbar = NULL;
		if (atoi(s_maxclients_field.buffer) <= 8) // set default of 8
		//	strncpy( s_maxclients_field.buffer, "8" );
			Q_strncpyz( s_maxclients_field.buffer, "8", sizeof(s_maxclients_field.buffer) );
		s_startserver_dmoptions_action.generic.statusbar = NULL;
		UI_RefreshMapList (MAP_DM);
	}
}

void StartServerActionFunc (void *self)
{
	char	startmap[1024];
	int		timelimit;
	int		fraglimit;
	int		maxclients;
	char	*spot;

//	strncpy( startmap, strchr( ui_svr_mapnames[s_startmap_list.curvalue], '\n' ) + 1 );
	Q_strncpyz( startmap, strchr( ui_svr_mapnames[s_startmap_list.curvalue], '\n' ) + 1, sizeof(startmap) );

	maxclients  = atoi( s_maxclients_field.buffer );
	timelimit	= atoi( s_timelimit_field.buffer );
	fraglimit	= atoi( s_fraglimit_field.buffer );

	Cvar_SetValue( "maxclients", ClampCvar( 0, maxclients, maxclients ) );
	Cvar_SetValue ("timelimit", ClampCvar( 0, timelimit, timelimit ) );
	Cvar_SetValue ("fraglimit", ClampCvar( 0, fraglimit, fraglimit ) );
	Cvar_Set("hostname", s_hostname_field.buffer );

	Cvar_SetValue ("deathmatch", s_rules_box.curvalue != 1);
	Cvar_SetValue ("coop", s_rules_box.curvalue == 1);
	Cvar_SetValue ("ctf", s_rules_box.curvalue == 2);
	Cvar_SetValue ("ttctf", s_rules_box.curvalue == 3);
	Cvar_SetValue ("gamerules", roguepath() ? ((s_rules_box.curvalue == 4) ? 2 : 0) : 0);

	spot = NULL;
	if (s_rules_box.curvalue == 1)		// PGM
	{
 		if(Q_stricmp(startmap, "bunk1") == 0)
  			spot = "start";
 		else if(Q_stricmp(startmap, "mintro") == 0)
  			spot = "start";
 		else if(Q_stricmp(startmap, "fact1") == 0)
  			spot = "start";
 		else if(Q_stricmp(startmap, "power1") == 0)
  			spot = "pstart";
 		else if(Q_stricmp(startmap, "biggun") == 0)
  			spot = "bstart";
 		else if(Q_stricmp(startmap, "hangar1") == 0)
  			spot = "unitstart";
 		else if(Q_stricmp(startmap, "city1") == 0)
  			spot = "unitstart";
 		else if(Q_stricmp(startmap, "boss1") == 0)
			spot = "bosstart";
	}

	if (spot)
	{
		if (Com_ServerState())
			Cbuf_AddText ("disconnect\n");
		Cbuf_AddText (va("gamemap \"*%s$%s\"\n", startmap, spot));
	}
	else
	{
		Cbuf_AddText (va("map %s\n", startmap));
	}

	UI_ForceMenuOff ();
}

void StartServer_MenuInit (void)
{
	static const char *dm_coop_names[] =
	{
		"deathmatch",
		"cooperative",
		"CTF",
		"3Team CTF",
		0
	};

	static const char *dm_coop_names_rogue[] =
	{
		"deathmatch",
		"cooperative",
		"CTF",
		"3Team CTF",
		"tag",
		0
	};

	int y = 0;
	
	//UI_LoadMapList (); // moved to UI_Init
	UI_BuildMapList (ui_svr_maptype); // was MAP_DM

	// levelshot found table
	if (ui_svr_mapshotvalid)	free(ui_svr_mapshotvalid);
	ui_svr_mapshotvalid = malloc( sizeof( byte ) * ( ui_svr_nummaps + 1 ) );
	memset( ui_svr_mapshotvalid, 0, sizeof( byte ) * ( ui_svr_nummaps + 1 ) );
	// register null levelshot
	if (ui_svr_mapshotvalid[ui_svr_nummaps] == M_UNSET) {	
		if (R_DrawFindPic("/gfx/ui/noscreen.pcx"))
			ui_svr_mapshotvalid[ui_svr_nummaps] = M_FOUND;
		else
			ui_svr_mapshotvalid[ui_svr_nummaps] = M_MISSING;
	}

	//
	// initialize the menu stuff
	//
	s_startserver_menu.x = SCREEN_WIDTH*0.5 - 140;
	//s_startserver_menu.y = 0;
	s_startserver_menu.nitems = 0;

	s_startmap_list.generic.type	= MTYPE_SPINCONTROL;
	s_startmap_list.generic.x		= 0;
	s_startmap_list.generic.y		= y;
	s_startmap_list.generic.name	= "initial map";
	s_startmap_list.itemnames		= ui_svr_mapnames;

	s_rules_box.generic.type	= MTYPE_SPINCONTROL;
	s_rules_box.generic.x		= 0;
	s_rules_box.generic.y		= y += 2*MENU_LINE_SIZE;
	s_rules_box.generic.name	= "rules";
//PGM - rogue games only available with rogue DLL.
	if (roguepath())
		s_rules_box.itemnames	= dm_coop_names_rogue;
	else
		s_rules_box.itemnames	= dm_coop_names;
//PGM
	if (Cvar_VariableValue("ttctf"))
		s_rules_box.curvalue = 3;
	else if (Cvar_VariableValue("ctf"))
		s_rules_box.curvalue = 2;
	else if (roguepath() && Cvar_VariableValue("gamerules") == 2)
		s_rules_box.curvalue = 4;
	else if (Cvar_VariableValue("coop"))
		s_rules_box.curvalue = 1;
	else
		s_rules_box.curvalue = 0;
	s_rules_box.generic.callback = RulesChangeFunc;

	s_timelimit_field.generic.type		= MTYPE_FIELD;
	s_timelimit_field.generic.name		= "time limit";
	s_timelimit_field.generic.flags		= QMF_NUMBERSONLY;
	s_timelimit_field.generic.x			= 0;
	s_timelimit_field.generic.y			= y += 2*MENU_FONT_SIZE;
	s_timelimit_field.generic.statusbar	= "0 = no limit";
	s_timelimit_field.length			= 4;
	s_timelimit_field.visible_length	= 4;
//	strncpy( s_timelimit_field.buffer, Cvar_VariableString("timelimit") );
	Q_strncpyz( s_timelimit_field.buffer, Cvar_VariableString("timelimit"), sizeof(s_timelimit_field.buffer) );
	s_timelimit_field.cursor			= strlen( s_timelimit_field.buffer );

	s_fraglimit_field.generic.type		= MTYPE_FIELD;
	s_fraglimit_field.generic.name		= "frag limit";
	s_fraglimit_field.generic.flags		= QMF_NUMBERSONLY;
	s_fraglimit_field.generic.x			= 0;
	s_fraglimit_field.generic.y			= y += 2.25*MENU_FONT_SIZE;
	s_fraglimit_field.generic.statusbar	= "0 = no limit";
	s_fraglimit_field.length			= 4;
	s_fraglimit_field.visible_length	= 4;
//	strncpy( s_fraglimit_field.buffer, Cvar_VariableString("fraglimit") );
	Q_strncpyz( s_fraglimit_field.buffer, Cvar_VariableString("fraglimit"), sizeof(s_fraglimit_field.buffer) );
	s_fraglimit_field.cursor			= strlen( s_fraglimit_field.buffer );

	/*
	** maxclients determines the maximum number of players that can join
	** the game.  If maxclients is only "1" then we should default the menu
	** option to 8 players, otherwise use whatever its current value is. 
	** Clamping will be done when the server is actually started.
	*/
	s_maxclients_field.generic.type			= MTYPE_FIELD;
	s_maxclients_field.generic.name			= "max players";
	s_maxclients_field.generic.flags		= QMF_NUMBERSONLY;
	s_maxclients_field.generic.x			= 0;
	s_maxclients_field.generic.y			= y += 2.25*MENU_FONT_SIZE;
	s_maxclients_field.generic.statusbar	= NULL;
	s_maxclients_field.length				= 3;
	s_maxclients_field.visible_length		= 3;
	if ( Cvar_VariableValue( "maxclients" ) == 1 )
	//	strncpy( s_maxclients_field.buffer, "8" );
		Q_strncpyz( s_maxclients_field.buffer, "8", sizeof(s_maxclients_field.buffer) );
	else 
	//	strncpy( s_maxclients_field.buffer, Cvar_VariableString("maxclients") );
		Q_strncpyz( s_maxclients_field.buffer, Cvar_VariableString("maxclients"), sizeof(s_maxclients_field.buffer) );
	s_maxclients_field.cursor				= strlen( s_maxclients_field.buffer );

	s_hostname_field.generic.type			= MTYPE_FIELD;
	s_hostname_field.generic.name			= "hostname";
	s_hostname_field.generic.flags			= 0;
	s_hostname_field.generic.x				= 0;
	s_hostname_field.generic.y				= y += 2.25*MENU_FONT_SIZE;
	s_hostname_field.generic.statusbar		= NULL;
	s_hostname_field.length					= 12;
	s_hostname_field.visible_length			= 12;
//	strncpy( s_hostname_field.buffer, Cvar_VariableString("hostname") );
	Q_strncpyz( s_hostname_field.buffer, Cvar_VariableString("hostname"), sizeof(s_hostname_field.buffer) );
	s_hostname_field.cursor					= strlen( s_hostname_field.buffer );

	s_startserver_dmoptions_action.generic.type = MTYPE_ACTION;
	s_startserver_dmoptions_action.generic.name	= " deathmatch flags";
	s_startserver_dmoptions_action.generic.flags= QMF_LEFT_JUSTIFY;
	s_startserver_dmoptions_action.generic.x	= 24;
	s_startserver_dmoptions_action.generic.y	= y += 2.25*MENU_FONT_SIZE;
	s_startserver_dmoptions_action.generic.statusbar = NULL;
	s_startserver_dmoptions_action.generic.callback = DMOptionsFunc;

	s_startserver_start_action.generic.type = MTYPE_ACTION;
	s_startserver_start_action.generic.name	= " begin";
	s_startserver_start_action.generic.flags= QMF_LEFT_JUSTIFY;
	s_startserver_start_action.generic.x	= 24;
	s_startserver_start_action.generic.y	= y += 2*MENU_LINE_SIZE;
	s_startserver_start_action.generic.callback = StartServerActionFunc;

	s_startserver_back_action.generic.type = MTYPE_ACTION;
	s_startserver_back_action.generic.name	= " back to multiplayer";
	s_startserver_back_action.generic.flags= QMF_LEFT_JUSTIFY;
	s_startserver_back_action.generic.x	= 24;
	s_startserver_back_action.generic.y	= y += 3*MENU_LINE_SIZE;
	s_startserver_back_action.generic.callback = UI_BackMenu;

	Menu_AddItem( &s_startserver_menu, &s_startmap_list );
	Menu_AddItem( &s_startserver_menu, &s_rules_box );
	Menu_AddItem( &s_startserver_menu, &s_timelimit_field );
	Menu_AddItem( &s_startserver_menu, &s_fraglimit_field );
	Menu_AddItem( &s_startserver_menu, &s_maxclients_field );
	Menu_AddItem( &s_startserver_menu, &s_hostname_field );
	Menu_AddItem( &s_startserver_menu, &s_startserver_dmoptions_action );
	Menu_AddItem( &s_startserver_menu, &s_startserver_start_action );
	Menu_AddItem( &s_startserver_menu, &s_startserver_back_action );

	Menu_Center( &s_startserver_menu );

	// call this now to set proper inital state
	RulesChangeFunc ( NULL );
}

void DrawStartSeverLevelshot (void)
{	
	char startmap[MAX_QPATH];
	char mapshotname [MAX_QPATH];
	int i = s_startmap_list.curvalue;
//	strncpy( startmap, strchr( ui_svr_mapnames[i], '\n' ) + 1 );
	Q_strncpyz( startmap, strchr( ui_svr_mapnames[i], '\n' ) + 1, sizeof(startmap) );

	SCR_DrawFill (SCREEN_WIDTH/2+44, SCREEN_HEIGHT/2-70, 244, 184, ALIGN_CENTER, 60,60,60,255);

	if ( ui_svr_mapshotvalid[i] == M_UNSET) { // init levelshot
		Com_sprintf(mapshotname, sizeof(mapshotname), "/levelshots/%s.pcx", startmap);
		if (R_DrawFindPic(mapshotname))
			ui_svr_mapshotvalid[i] = M_FOUND;
		else
			ui_svr_mapshotvalid[i] = M_MISSING;
	}

	if ( ui_svr_mapshotvalid[i] == M_FOUND) {
		Com_sprintf(mapshotname, sizeof(mapshotname), "/levelshots/%s.pcx", startmap);

		SCR_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, mapshotname, 1.0);
	}
	else if (ui_svr_mapshotvalid[ui_svr_nummaps] == M_FOUND)
		SCR_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, "/gfx/ui/noscreen.pcx", 1.0);
	else
		SCR_DrawFill (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, 0,0,0,255);
}

void StartServer_MenuDraw (void)
{
	Menu_DrawBanner( "m_banner_start_server" ); // Knightmare added
	Menu_Draw( &s_startserver_menu );
	DrawStartSeverLevelshot (); // added levelshots
}

const char *StartServer_MenuKey (int key)
{
	/*if ( key == K_ESCAPE )
	{
		if (ui_svr_mapnames)
			FS_FreeFileList (ui_svr_mapnames, ui_svr_nummaps);
	}*/

	return Default_MenuKey( &s_startserver_menu, key );
}

void M_Menu_StartServer_f (void)
{
	StartServer_MenuInit();
	UI_PushMenu( StartServer_MenuDraw, StartServer_MenuKey );
}
