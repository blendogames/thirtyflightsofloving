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

#define MAPLIST_ARENAS


/*
=============================================================================

START SERVER MENU

=============================================================================
*/
static menuframework_s s_startserver_menu;
static char **mapnames;
static int	nummaps;
static int	arenaNum = 0;

#define M_UNSET 0
#define	M_MISSING 1
#define	M_FOUND 2
static	int *svr_mapshotvalid; // levelshot truth table

static menuaction_s	s_startserver_start_action;
static menuaction_s	s_startserver_dmoptions_action;
static menufield_s	s_timelimit_field;
static menufield_s	s_fraglimit_field;
static menufield_s	s_maxclients_field;
static menufield_s	s_hostname_field;
static menulist_s	s_startmap_list;
menulist_s	s_rules_box;
static menuaction_s	s_startserver_back_action;


//=============================================================================
#ifdef MAPLIST_ARENAS

#define MAX_ARENAS 1024
#define MAX_ARENAS_TEXT 8192

static char **arenaList;

/*
===============
UI_ParseArenaFromFile
Partially from Q3 source
===============
*/
qboolean UI_ParseArenaFromFile (char *filename, char *shortname, char *longname, char *gametypes)
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
			strcpy( dest, token );
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
UI_LoadArenas
===============
*/
void UI_LoadArenas (void)
{
	char	*p, *path = NULL;
	char	**arenafiles;
	char	**tmplist = 0;
	char	findname[1024];
	int		i, narenas = 0, narenanames = 0;

	arenaList = malloc( sizeof( char * ) * MAX_ARENAS );
	memset( arenaList, 0, sizeof( char * ) * MAX_ARENAS );

	tmplist = malloc( sizeof( char * ) * MAX_ARENAS );
	memset( tmplist, 0, sizeof( char * ) * MAX_ARENAS );

	path = FS_NextPath( path );
	while (path) 
	{
		Com_sprintf( findname, sizeof(findname), "%s/scripts/*.arena", path );
		arenafiles = FS_ListFiles( findname, &narenas, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM );

		for (i=0; i<narenas && narenanames<MAX_ARENAS; i++)
		{
			if (!arenafiles || !arenafiles[i])
				continue;

			p = strstr(arenafiles[i], "/scripts/"); p++;

			if (!strstr(p, ".arena"))
				continue;

			if (!ItemInList(p, narenanames, tmplist)) // check if already in list
			{
				char	shortname[MAX_TOKEN_CHARS];
				char	longname[MAX_TOKEN_CHARS];
				char	gametypes[MAX_TOKEN_CHARS];
				char	scratch[200];
				if (UI_ParseArenaFromFile (p, shortname, longname, gametypes))
				{
					Com_sprintf(scratch, sizeof(scratch), "%s\n%s", longname, shortname);
					arenaList[narenanames] = malloc(strlen( scratch ) + 1);
					strcpy(arenaList[narenanames], scratch);
					narenanames++;

					//Com_Printf ("UI_LoadArenas: successfully loaded arena file %s: mapname: %s levelname: %s\n", p, shortname, longname);

					InsertInList(tmplist, strdup(p), narenanames, 0); // add to list
				}
			}
		}
		if (narenas)
			FreeFileList (arenafiles, narenas);
		path = FS_NextPath( path );
	}

	//check pak after
	if (arenafiles = FS_ListPak("scripts/", &narenas))
	{
		for (i=0; i<narenas && narenanames<MAX_ARENAS; i++)
		{
			if (!arenafiles || !arenafiles[i])
				continue;

			p = arenafiles[i];

			if (!strstr(p, ".arena"))
				continue;

			if (!ItemInList(p, narenanames, tmplist)) // check if already in list
			{
				char	shortname[MAX_TOKEN_CHARS];
				char	longname[MAX_TOKEN_CHARS];
				char	gametypes[MAX_TOKEN_CHARS];
				char	scratch[200];
				if (UI_ParseArenaFromFile (p, shortname, longname, gametypes))
				{
					Com_sprintf(scratch, sizeof(scratch), "%s\n%s", longname, shortname);
					arenaList[narenanames] = malloc(strlen( scratch ) + 1);
					strcpy(arenaList[narenanames], scratch);
					narenanames++;

					//Com_Printf ("UI_LoadArenas: successfully loaded arena file %s: mapname: %s levelname: %s\n", p, shortname, longname);

					InsertInList(tmplist, strdup(p), narenanames, 0); // add to list
				}
			}
		}
	}
	if (narenas)
		FreeFileList (arenafiles, narenas);

	if (narenanames)
		FreeFileList (tmplist, narenanames);
	arenaNum = narenanames;
	Com_Printf ("UI_LoadArenas: loaded %i arena file(s)\n", arenaNum);
}

#endif // MAPLIST_ARENAS

/*
===============
UI_LoadMapList
===============
*/
void UI_LoadMapList (void)
{
	char *buffer;
	char  mapsname[1024];
	char *s;
	int length;
	int i;
	FILE *fp;

	/*
	** load the list of map names
	*/
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
	while ( i < length )
	{
		if ( s[i] == '\r' )
			nummaps++;
		i++;
	}

	// add .arena files to count
#ifdef MAPLIST_ARENAS
	UI_LoadArenas();	// load arena names
#endif
	nummaps += arenaNum;

	if (nummaps == 0)
	//	Com_Error( ERR_DROP, "no maps in maps.lst\n" );
	{	// dropping to console won't avoid anything now, so hack in a default map list
		nummaps = 1;
		buffer = "base1 \"Outer Base\"\n";
	}

	mapnames = malloc( sizeof( char * ) * ( nummaps + 1 ) );
	memset( mapnames, 0, sizeof( char * ) * ( nummaps + 1 ) );

	s = buffer;

	for (i = 0; i < nummaps; i++)
	{
		char	shortname[MAX_TOKEN_CHARS];
		char	longname[MAX_TOKEN_CHARS];
		char	scratch[200];
		//int		j, l;
		if (i < (nummaps - arenaNum))
		{
			strcpy( shortname, COM_Parse( &s ) );
			/*l = strlen(shortname);
			for (j=0 ; j<l ; j++)
				shortname[j] = toupper(shortname[j]);*/
			strcpy( longname, COM_Parse( &s ) );
			Com_sprintf( scratch, sizeof( scratch ), "%s\n%s", longname, shortname );
			mapnames[i] = malloc( strlen( scratch ) + 1 );
			strcpy( mapnames[i], scratch );
		}
#ifdef MAPLIST_ARENAS
		else // map names from .arena files at end of list
		{
			int offset = nummaps - arenaNum;
			mapnames[i] = malloc( strlen( arenaList[i-offset] ) + 1 );
			strcpy( mapnames[i], arenaList[i-offset] );
		}
#endif
	}
	mapnames[nummaps] = 0;

	if ( fp != 0 )
	{
		fp = 0;
		free( buffer );
	}
	else
		FS_FreeFile( buffer );
}
//=============================================================================


void DMOptionsFunc( void *self )
{
	if (s_rules_box.curvalue == 1)
		return;
	M_Menu_DMOptions_f();
}

void RulesChangeFunc ( void *self )
{
	if (s_rules_box.curvalue == 0) 	// DM
	{
		s_maxclients_field.generic.statusbar = NULL;
		s_startserver_dmoptions_action.generic.statusbar = NULL;
	}
	else if(s_rules_box.curvalue == 1)		// coop				// PGM
	{
		s_maxclients_field.generic.statusbar = "4 maximum for cooperative";
		if (atoi(s_maxclients_field.buffer) > 4)
			strcpy( s_maxclients_field.buffer, "4" );
		s_startserver_dmoptions_action.generic.statusbar = "N/A for cooperative";
	}
	// ROGUE GAMES
	else if (roguepath() && s_rules_box.curvalue == 2) // tag	
	{
		s_maxclients_field.generic.statusbar = NULL;
		s_startserver_dmoptions_action.generic.statusbar = NULL;
	}
	else if ( (roguepath() && s_rules_box.curvalue == 3) // CTF
		|| (!roguepath() && s_rules_box.curvalue == 2) )
	{
		s_maxclients_field.generic.statusbar = NULL;
		if (atoi(s_maxclients_field.buffer) <= 12) // set default of 8
			strcpy( s_maxclients_field.buffer, "12" );
		s_startserver_dmoptions_action.generic.statusbar = NULL;
	}
	else if ( (roguepath() && s_rules_box.curvalue == 4) // 3Team CTF
		|| (!roguepath() && s_rules_box.curvalue == 3) )
	{
		s_maxclients_field.generic.statusbar = NULL;
		if (atoi(s_maxclients_field.buffer) <= 18) // set default of 8
			strcpy( s_maxclients_field.buffer, "18" );
		s_startserver_dmoptions_action.generic.statusbar = NULL;
	}
}

void StartServerActionFunc( void *self )
{
	char	startmap[1024];
	int		timelimit;
	int		fraglimit;
	int		maxclients;
	char	*spot;

	strcpy( startmap, strchr( mapnames[s_startmap_list.curvalue], '\n' ) + 1 );

	maxclients  = atoi( s_maxclients_field.buffer );
	timelimit	= atoi( s_timelimit_field.buffer );
	fraglimit	= atoi( s_fraglimit_field.buffer );

	Cvar_SetValue( "maxclients", ClampCvar( 0, maxclients, maxclients ) );
	Cvar_SetValue ("timelimit", ClampCvar( 0, timelimit, timelimit ) );
	Cvar_SetValue ("fraglimit", ClampCvar( 0, fraglimit, fraglimit ) );
	Cvar_Set("hostname", s_hostname_field.buffer );

	// Knightmare- redid this
	if (roguepath()) {
		Cvar_SetValue ("deathmatch", s_rules_box.curvalue != 1);
		Cvar_SetValue ("coop", s_rules_box.curvalue == 1);
		Cvar_SetValue ("gamerules", s_rules_box.curvalue == 2 );
		Cvar_SetValue ("ctf", s_rules_box.curvalue >= 3 );
		Cvar_SetValue ("ttctf", s_rules_box.curvalue == 4 );
	} else {
		Cvar_SetValue ("deathmatch", s_rules_box.curvalue != 1);
		Cvar_SetValue ("coop", s_rules_box.curvalue == 1);
		Cvar_SetValue ("gamerules", 0 );
		Cvar_SetValue ("ctf", s_rules_box.curvalue >= 2 );
		Cvar_SetValue ("ttctf", s_rules_box.curvalue == 3 );
	}

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

void StartServer_MenuInit ( void )
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
		"tag",
		"CTF",
		"3Team CTF",
	//	"deathball",
		0
	};

	int y = 0;
	
	//UI_LoadMapList(); // moved to separate function

	// levelshot found table
	svr_mapshotvalid = malloc( sizeof( int ) * ( nummaps + 1 ) );
	memset( svr_mapshotvalid, 0, sizeof( int ) * ( nummaps + 1 ) );

	// register null levelshot
	if (svr_mapshotvalid[nummaps] == M_UNSET) {	
		if (R_DrawFindPic("/gfx/noscreen.pcx"))
			svr_mapshotvalid[nummaps] = M_FOUND;
		else
			svr_mapshotvalid[nummaps] = M_MISSING;
	}

	//
	// initialize the menu stuff
	//
	s_startserver_menu.x = SCREEN_WIDTH*0.5 - 140;
//	s_startserver_menu.x = viddef.width / 2 - SCR_ScaledVideo(140);
//	s_startserver_menu.y = 0;
	s_startserver_menu.nitems = 0;

	s_startmap_list.generic.type = MTYPE_SPINCONTROL;
	s_startmap_list.generic.x	= 0;
	s_startmap_list.generic.y	= y;
	s_startmap_list.generic.name	= "initial map";
	s_startmap_list.itemnames = mapnames;

	s_rules_box.generic.type = MTYPE_SPINCONTROL;
	s_rules_box.generic.x	= 0;
	s_rules_box.generic.y	= y += 2*MENU_LINE_SIZE;
	s_rules_box.generic.name	= "rules";
//PGM - rogue games only available with rogue DLL.
	if (roguepath())
		s_rules_box.itemnames = dm_coop_names_rogue;
	else
		s_rules_box.itemnames = dm_coop_names;
//PGM
	if (roguepath()) {
		if (Cvar_VariableValue("ttctf"))
			s_rules_box.curvalue = 4;
		else if (Cvar_VariableValue("ctf"))
			s_rules_box.curvalue = 3;
		else if (Cvar_VariableValue("gamerules"))
			s_rules_box.curvalue = 2;
		else if (Cvar_VariableValue("coop"))
			s_rules_box.curvalue = 1;
		else
			s_rules_box.curvalue = 0;
	} else {
		if (Cvar_VariableValue("ttctf"))
			s_rules_box.curvalue = 3;
		else if (Cvar_VariableValue("ctf"))
			s_rules_box.curvalue = 2;
		else if (Cvar_VariableValue("coop"))
			s_rules_box.curvalue = 1;
		else
			s_rules_box.curvalue = 0;
	}
	s_rules_box.generic.callback = RulesChangeFunc;

	s_timelimit_field.generic.type = MTYPE_FIELD;
	s_timelimit_field.generic.name = "time limit";
	s_timelimit_field.generic.flags = QMF_NUMBERSONLY;
	s_timelimit_field.generic.x	= 0;
	s_timelimit_field.generic.y	= y += 2*MENU_FONT_SIZE;
	s_timelimit_field.generic.statusbar = "0 = no limit";
	s_timelimit_field.length = 4;
	s_timelimit_field.visible_length = 4;
	strcpy( s_timelimit_field.buffer, Cvar_VariableString("timelimit") );
	s_timelimit_field.cursor = strlen( s_timelimit_field.buffer );

	s_fraglimit_field.generic.type = MTYPE_FIELD;
	s_fraglimit_field.generic.name = "frag limit";
	s_fraglimit_field.generic.flags = QMF_NUMBERSONLY;
	s_fraglimit_field.generic.x	= 0;
	s_fraglimit_field.generic.y	= y += 2.25*MENU_FONT_SIZE;
	s_fraglimit_field.generic.statusbar = "0 = no limit";
	s_fraglimit_field.length = 4;
	s_fraglimit_field.visible_length = 4;
	strcpy( s_fraglimit_field.buffer, Cvar_VariableString("fraglimit") );
	s_fraglimit_field.cursor = strlen( s_fraglimit_field.buffer );

	/*
	** maxclients determines the maximum number of players that can join
	** the game.  If maxclients is only "1" then we should default the menu
	** option to 8 players, otherwise use whatever its current value is. 
	** Clamping will be done when the server is actually started.
	*/
	s_maxclients_field.generic.type = MTYPE_FIELD;
	s_maxclients_field.generic.name = "max players";
	s_maxclients_field.generic.flags = QMF_NUMBERSONLY;
	s_maxclients_field.generic.x	= 0;
	s_maxclients_field.generic.y	= y += 2.25*MENU_FONT_SIZE;
	s_maxclients_field.generic.statusbar = NULL;
	s_maxclients_field.length = 3;
	s_maxclients_field.visible_length = 3;
	if ( Cvar_VariableValue( "maxclients" ) == 1 )
		strcpy( s_maxclients_field.buffer, "8" );
	else 
		strcpy( s_maxclients_field.buffer, Cvar_VariableString("maxclients") );
	s_maxclients_field.cursor = strlen( s_maxclients_field.buffer );

	s_hostname_field.generic.type = MTYPE_FIELD;
	s_hostname_field.generic.name = "hostname";
	s_hostname_field.generic.flags = 0;
	s_hostname_field.generic.x	= 0;
	s_hostname_field.generic.y	= y += 2.25*MENU_FONT_SIZE;
	s_hostname_field.generic.statusbar = NULL;
	s_hostname_field.length = 12;
	s_hostname_field.visible_length = 12;
	strcpy( s_hostname_field.buffer, Cvar_VariableString("hostname") );
	s_hostname_field.cursor = strlen( s_hostname_field.buffer );

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
	strcpy( startmap, strchr( mapnames[i], '\n' ) + 1 );

	SCR_DrawFill2 (SCREEN_WIDTH/2+44, SCREEN_HEIGHT/2-70, 244, 184, ALIGN_CENTER, 60,60,60,255);

	if ( svr_mapshotvalid[i] == M_UNSET) { // init levelshot
		Com_sprintf(mapshotname, sizeof(mapshotname), "/levelshots/%s.pcx", startmap);
		if (R_DrawFindPic(mapshotname))
			svr_mapshotvalid[i] = M_FOUND;
		else
			svr_mapshotvalid[i] = M_MISSING;
	}

	if ( svr_mapshotvalid[i] == M_FOUND) {
		Com_sprintf(mapshotname, sizeof(mapshotname), "/levelshots/%s.pcx", startmap);

		SCR_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, mapshotname, 1.0);
	}
	else if (svr_mapshotvalid[nummaps] == M_FOUND)
		SCR_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, "/gfx/noscreen.pcx", 1.0);
	else
		SCR_DrawFill2 (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-68, 240, 180, ALIGN_CENTER, 0,0,0,255);
}

void StartServer_MenuDraw(void)
{
	Menu_Banner( "m_banner_start_server" ); // Knightmare added
	Menu_Draw( &s_startserver_menu );
	DrawStartSeverLevelshot (); // added levelshots
}

const char *StartServer_MenuKey (int key)
{
	/*if ( key == K_ESCAPE )
	{
		if ( mapnames )
		{
			int i;

			for ( i = 0; i < nummaps; i++ )
				free( mapnames[i] );
			free( mapnames );
		}
		mapnames = 0;
		nummaps = 0;
	}*/

	return Default_MenuKey( &s_startserver_menu, key );
}

void M_Menu_StartServer_f (void)
{
	StartServer_MenuInit();
	UI_PushMenu( StartServer_MenuDraw, StartServer_MenuKey );
}
