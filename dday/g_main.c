/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_main.c,v $
 *   $Revision: 1.15 $
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

#include "g_local.h"
#include "g_cmds.h"

game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals;
spawn_temp_t	st;

int	sm_meat_index;
int	snd_fry;
int meansOfDeath;

edict_t		*g_edicts;

cvar_t	*deathmatch;
cvar_t	*coop;
cvar_t	*dmflags;
cvar_t	*skill;
cvar_t	*fraglimit;
cvar_t	*timelimit;
cvar_t	*password;
cvar_t	*maxclients;
cvar_t	*maxentities;
cvar_t	*g_select_empty;
cvar_t	*dedicated;

cvar_t	*sv_maxvelocity;
cvar_t	*sv_gravity;

cvar_t	*sv_rollspeed;
cvar_t	*sv_rollangle;
cvar_t	*gun_x;
cvar_t	*gun_y;
cvar_t	*gun_z;

cvar_t	*run_pitch;
cvar_t	*run_roll;
cvar_t	*bob_up;
cvar_t	*bob_pitch;
cvar_t	*bob_roll;

cvar_t	*sv_cheats;

//Flyingnun Password
cvar_t  *flyingnun_password;

//KMM: 9/15  CVARS
//cvar_t	*scope_setting;				// Sniperscope FOV
//cvar_t	*auto_reload;				// change magazine automaticly?
//cvar_t	*auto_weapon_change;		// change weapon when out of ammo?
//cvar_t	*observer_on_death;			// if set, go to observer when dead
//cvar_t	*heal_wounds;			// if set, health fixes chest and stomach wounds
//cvar_t	*flame_shots;				// the number of shots a flamethrower shoots
cvar_t  *RI;						// reinforcement interval -- the length of time before reinforcements are spawned.
cvar_t	*team_kill;					// friendly fire toggle (default off)
cvar_t	*class_limits;				// enable class limits
cvar_t	*spawn_camp_check;			// enable spawn camping checks
cvar_t	*spawn_camp_time;			// the amount of time after spawn to check for spawn camps
//cvar_t	*teamgren;					// enables "team grenade mode" where the grenades are team-conscience, a soldier can not pick up his own team's grenade
cvar_t	*death_msg;					// when and how the death msgs are displayed
cvar_t	*level_wait;				// pause time at beginning and end of games
cvar_t	*invuln_spawn;				// how long player is invulnerable after spawn

cvar_t	*arty_delay;				// seconds for artillary to position
cvar_t	*arty_time;					// seconds between each volley
cvar_t  *arty_max;					// number of shots to be fired in each volley

//bcass start - easter_egg cvar, AGAIN
cvar_t	*easter_egg;				//turns easter eggs on and off
//bcass end

cvar_t	*flood_msgs;
cvar_t	*flood_persecond;
cvar_t	*flood_waitdelay;

//faf
cvar_t  *player_scores;
cvar_t  *max_gibs;
cvar_t  *extra_anims;//faf
cvar_t  *force_auto_select;

cvar_t  *allied_password;
cvar_t  *axis_password;

cvar_t  *objective_protect;
cvar_t  *ent_files;

cvar_t *mauser_only;  //ddaylife
cvar_t *sniper_only;  //ddaylife
cvar_t *no_nades; //ddaylife
cvar_t *airstrikes;

cvar_t *ctc;//capture the church

cvar_t *stats;

cvar_t *fast_knife;
cvar_t *chile;
cvar_t *mashup;
cvar_t *sv_maplist;  //faf,  putting this back in from the original quake2 code
cvar_t	*swords;
cvar_t	*nohud;
cvar_t	*mapvoting;
cvar_t	*constant_play;
cvar_t	*serverimg;

//cvar_t	*crosshair;

cvar_t *bots;
cvar_t *botchat;
cvar_t *botchatfreq;
//cvar_t *alliedbots;
//cvar_t *axisbots;
cvar_t *alliedlevel;
cvar_t *axislevel;
//cvar_t *playerminforbots;
cvar_t *playermaxforbots;

cvar_t *knifefest;
cvar_t *fullbright;

cvar_t	*campaign;

cvar_t *sandbaglimit;

cvar_t *afk_time;

// Knightmare- moved these vars here to fix compile on GCC
byte		is_silenced;

TeamS_t *team_list[MAX_TEAMS];

mapclasslimits_t mapclasslimits[MAX_TEAMS][10];

int	usa_index;
int	grm_index;
int	rus_index;
int	gbr_index;
int	pol_index;
int	ita_index;
int	jpn_index;
int	usm_index;

camp_spots_t camp_spots[128];
int  total_camp_spots;
int	num_clients;
qboolean qbots;

campaign_spots_t campaign_spots[50];

int alliedplatoons;
int	axisplatoons;
int	alliedneedspots;
int	axisneedspots;

int campaign_winner;

char	*last_maps_played[20];

qboolean no_objectives_left;

qboolean dropnodes;

int	allied_sandbags;
int axis_sandbags;

char	*votemaps[5];
int		mapvotes[5];

char botchat_taunt[MAX_TEAMS][200][150];
int  botchat_taunt_count[MAX_TEAMS];
char botchat_sorry[MAX_TEAMS][200][150];
int  botchat_sorry_count[MAX_TEAMS];
char botchat_brag[MAX_TEAMS][200][150];
int  botchat_brag_count[MAX_TEAMS];
char botchat_tked[MAX_TEAMS][200][150];
int  botchat_tked_count[MAX_TEAMS];
char botchat_insult[MAX_TEAMS][200][150];
int  botchat_insult_count[MAX_TEAMS];
char botchat_forgive[MAX_TEAMS][200][150];
int  botchat_forgive_count[MAX_TEAMS];
char botchat_random[MAX_TEAMS][200][150];
int  botchat_random_count[MAX_TEAMS];
char botchat_killed[MAX_TEAMS][200][150];
int  botchat_killed_count[MAX_TEAMS];
char botchat_self[MAX_TEAMS][200][150];
int  botchat_self_count[MAX_TEAMS];


char user_shouts[20][100];
int user_shout_count;
// end Knightmare


void SpawnEntities (char *mapname, char *entities, char *spawnpoint);
void ClientThink (edict_t *ent, usercmd_t *cmd);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void ClientDisconnect (edict_t *ent);
void ClientBegin (edict_t *ent);
void ClientCommand (edict_t *ent);
void RunEntity (edict_t *ent);
void WriteGame (char *filename, qboolean autosave);
void ReadGame (char *filename);
void WriteLevel (char *filename);
void ReadLevel (char *filename);
void InitGame (void);
void G_RunFrame (void);
void CleanUpCmds();
void PBM_KillAllFires (void);



//===================================================================


//=============================================================
// So the Server can execute a console command for the Client
// as if the client typed it into their console themselves.
//============================================================
void stuffcmd(edict_t *ent, char *s)
{
	//JABot[start]
	if (ent->ai || !ent->inuse)
		return;
	//[end]


	gi.WriteByte(11);
	gi.WriteString(s);
	gi.unicast(ent, true);
}


void ShutdownGame (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->client)
			continue;
		if (!ent->inuse)
			continue;
		PMenu_Close(ent);
	}
	gi.dprintf ("==== ShutdownGame ====\n");

	CleanUpCmds();
	//ClearUserDLLs();

	gi.FreeTags (TAG_LEVEL);
	gi.FreeTags (TAG_GAME);
}

void SpawnEntities2 (char *mapname, char *entstring, char *spawnpoint);
/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
game_export_t *GetGameAPI (game_import_t *import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities2;

	globals.WriteGame = WriteGame;
	globals.ReadGame = ReadGame;
	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;

	globals.ServerCommand = ServerCommand;

	globals.edict_size = sizeof(edict_t);

	return &globals;
}

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
//	vsprintf (text, error, argptr);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	gi.error (ERR_FATAL, "%s", text);
}

void Com_Printf (char *msg, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
//	vsprintf (text, msg, argptr);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	gi.dprintf ("%s", text);
}

#endif

//======================================================================


/*
=================
ClientEndServerFrames
=================
*/
void ClientEndServerFrames (void)
{
	int		i;
	edict_t	*ent;

	// calc the player views now that all pushing
	// and damage has been added
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		ClientEndServerFrame (ent);
	}

}



/*        //faf:  from original q2 code
=================
CreateTargetChangeLevel

Returns the created target changelevel
=================
*/
edict_t *CreateTargetChangeLevel(char *map)
{
	edict_t *ent;

	ent = G_Spawn ();
	ent->classname = "target_changelevel";
	Com_sprintf (level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;
	return ent;
}



qboolean MapExists (char *map)
{
	FILE	*check;
	char	filename[256];
	cvar_t	*basedir;	// Knightmare added

	basedir = gi.cvar("basedir", "", 0);	// Knightmare added

/*	Q_strncpyz (filename, sizeof(filename), GAMEVERSION "/maps/");
	Q_strncatz (filename, sizeof(filename), map);
	Q_strncatz (filename, sizeof(filename), ".bsp"); */
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/maps/%s.bsp", GameDir(), map);

	if (check = fopen(filename, "r") )
	{
		fclose (check);
		return true;
	}

/*	Q_strncpyz (filename, sizeof(filename), "baseq2/maps/");
	Q_strncatz (filename, sizeof(filename), map);
	Q_strncatz (filename, sizeof(filename), ".bsp"); */
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/baseq2/maps/%s.bsp", basedir->string, map);

	if (check = fopen(filename, "r") )
	{
		fclose (check);
		return true;
	}

/*	Q_strncpyz (filename, sizeof(filename), GAMEVERSION "/maps/");
	Q_strncatz (filename, sizeof(filename), map);
	Q_strncatz (filename, sizeof(filename), ".bsp.override"); */
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/maps/%s.bsp.override", GameDir(), map);

	if (check = fopen(filename, "r") )
	{
		fclose (check);
		return true;
	}

	return false;

}


void Write_Last_Maps (void)
{
	char	filename[256];
	FILE	*fp;
	int		i;

//	fp = fopen ("dday/lastmaps.txt", "w");
	// Knightmare- use SavegameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/lastmaps.txt", SavegameDir());
	fp = fopen (filename, "w");

	if ( !fp )
	//	gi.error ("Couldn't open dday/lastmaps.txt");
		gi.error ("Couldn't open %s", filename);

	for (i=0; i<20 && last_maps_played[i]; i++)	{
		fprintf (fp, "%s\n", last_maps_played[i]);
	}
	fclose (fp);
}


void Read_Last_Maps (void)
{
	int		i,c;
	char	*s, *f = NULL;
	char	*lastmaps;
 	char	filename[MAX_OSPATH] = "";	// Knightmare added
	FILE	*check;						// Knightmare added

//	lastmaps = ReadEntFile("dday/lastmaps.txt");

	// Knightmare- use  SavegameDir() / GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/lastmaps.txt", SavegameDir());

	// fall back to GameDir() if not found in SavegameDir()
	check = fopen(filename, "r");
	if ( !check )
		Com_sprintf (filename, sizeof(filename), "%s/lastmaps.txt", GameDir());
	else
		fclose (check);
	// end Knightmare

	lastmaps = ReadEntFile(filename);

	if (lastmaps)
	{
		c = 0;
	//	f = strdup (lastmaps);
		f = G_CopyString (lastmaps);	// Knightmare- use G_CopyString instead
		s = strtok(f, "\n");
		for (i=1; i<20; i++) {
			if (s != NULL) {
				last_maps_played[i]= s;
				s = strtok (NULL, "\n");
			}
		}
	}

	// Knightmare- free temp buffer
	if (f != NULL) {
		gi.TagFree (f);
		f = NULL;
	}
}


char *Get_Next_MaplistTxt_Map (void)
{
	char	*maps;
	int		i,j,c;
	char	*s, *f=NULL;
	int		mapcount;
	int		newmapcount;
	int		x;
	int		randnum;
	int		removed;
	char	*possible_maps[300];
	char	*maplisttxt[300];
 	char	filename[MAX_OSPATH] = "";	// Knightmare added

//	maps = ReadEntFile("dday/maplist.txt");
	// Knightmare- use GameDir() instead for compatibility on all platforms
	Com_sprintf (filename, sizeof(filename), "%s/maplist.txt", GameDir());
	maps = ReadEntFile(filename);

	mapcount = 0;

	if (maps)
	{
		c = 0;
	//	f = strdup (maps);
		f = G_CopyString (maps);	// Knightmare- use G_CopyString instead
		s = strtok(f, "\n");
		while (c < 300)
		{
			if (s != NULL)
			{
				if (MapExists(s))
				{
					maplisttxt[c] = s;
					mapcount++;
					c++;
				}
				else
					gi.dprintf("WARNING: Map '%s' in maplist.txt not found on server!\n",s);



				s = strtok (NULL, "\n");
			}
			else
			{maplisttxt[c] = "";
			c++;}

		}

	}
	else
	{
		return NULL;
	}

	removed = 0;
	//remove most recently played maps
	for (i = 0; i<20 && removed != mapcount-1 && last_maps_played[i]; i++) //for last_maps_played
	{
		for (j=0; j < mapcount; j++)
		{
			if (!strcmp (maplisttxt[j],""))
				continue;




			if (!strcmp (last_maps_played[i], maplisttxt[j]))
			{
				maplisttxt[j]= "";
				//gi.dprintf("EEEEEEEE Removing %s\n",campaign_spots[checknum].bspname);
				removed++;
				break;
			}
		}
	}


	newmapcount = 0;
	//sort possible_maps
	for (x=0; x < 300; x++)
	{
		if (!strcmp (maplisttxt[x],""))
			continue;
		possible_maps[newmapcount] = maplisttxt[x];

		newmapcount++;
	}

/*	for (j = 0; j<50; j++)	{
		if (possible_maps[j] > -1)	{
			gi.dprintf ("%s\n",campaign_spots[possible_maps[j]].bspname);
		}
	}	*/
	randnum = (int)(random()*newmapcount);

	// Knightmare- free temp buffer
	if (f != NULL) {
		gi.TagFree (f);
		f = NULL;
	}

	return possible_maps[randnum];
}

char *Get_Next_Campaign_Map ();
/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
void EndDMLevel (void)
{
	char		*s=NULL, *t=NULL, *f=NULL, *sb=NULL, *tb=NULL;// *fb;
	static const char	*seps = " ,\n\r";
	char		*mapname=NULL, *mapCheck=NULL;
	char		mapCheckBuf[MAX_OSPATH];	// Knightmare added
	int			i, axiscount=0,alliedcount=0;
	char		*nextmap;
	edict_t		*ent = NULL;

	i = 0;

	mapname = level.mapname;

	if (level.campaign)
	{

		for (i=0; campaign_spots[i].bspname;i++)
		{
			if (!strcmp (level.mapname, campaign_spots[i].bspname))
			{
				Update_Campaign_Info();

				if (alliedplatoons == 0)
				{	campaign_winner = 1;}
				else if (axisplatoons == 0)
					campaign_winner = 0;

				else
				{
					for (i=0; campaign_spots[i].bspname; i++)
					{
						if (campaign_spots[i].owner == 0)
							alliedcount++;
						if (campaign_spots[i].owner == 1)
							axiscount++;
					}
					if  (alliedneedspots > 0 &&	alliedcount >= alliedneedspots)
					{campaign_winner = 0;}
					else if (axisneedspots > 0 && axiscount >= axisneedspots)
					{	campaign_winner = 1;}
				}
				break;
			}
		}

	WriteCampaignTxt();

	}


//	if (last_maps_played[0])
//		gi.dprintf ("xxx%s\n", last_maps_played[0]);
	for (i=20; i>0; i--)
	{
		if (last_maps_played[i-1])
		{
			last_maps_played[i]=last_maps_played[i-1];
//			gi.dprintf("%s\n", last_maps_played[i]);
		}
	}
	if (level.botfiles)
		last_maps_played[0] = level.botfiles;
	else
		last_maps_played[0] = level.mapname;

	Read_Last_Maps ();
	Write_Last_Maps ();

	if (level.campaign && campaign_winner < 0)
	{
		nextmap = Get_Next_Campaign_Map();
		if (nextmap != NULL)
		{
			ent = G_Spawn ();
			ent->classname = "target_changelevel";
			ent->map = nextmap;
			safe_bprintf (PRINT_HIGH, "Next map: %s \n", nextmap);
			BeginIntermission (ent);
			return;
		}
		else
		{	//hack job! winning team has no place to go, so pretend the losing team won, get
			//next map, then set everything back again
			Last_Team_Winner = (Last_Team_Winner+1)%2;
			nextmap = Get_Next_Campaign_Map();
			if (nextmap != NULL)
			{
				Last_Team_Winner = (Last_Team_Winner+1)%2;
				ent = G_Spawn ();
				ent->classname = "target_changelevel";
				ent->map = nextmap;
				safe_bprintf (PRINT_HIGH, "Next map: %s \n", nextmap);
				BeginIntermission (ent);
				return;
			}
			else (Last_Team_Winner = (Last_Team_Winner+1)%2);

		}
	}




	// stay on same level flag
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = mapname;
		safe_bprintf (PRINT_HIGH, "Next map: %s \n", level.mapname);
	}


	// faf:  maplist code, based on baseq2 version
	if (*sv_maplist->string)
	{
		if (!strcmp(sv_maplist->string, "txt"))
		{
			nextmap = Get_Next_MaplistTxt_Map();
			if (nextmap)
			{
				safe_bprintf (PRINT_HIGH, "Next map: %s \n", nextmap);
				BeginIntermission (CreateTargetChangeLevel (nextmap) );
				//last_maplist_map_played = team_list[0]->nextmap;
				//first_non_maplist_map = NULL;
				return;
			}
			else
			{
				safe_bprintf (PRINT_HIGH, "Next map: %s \n", nextmap);
				BeginIntermission (CreateTargetChangeLevel ("hill") );
				return;
			}
		}
		else
		{
		//	s = strdup(sv_maplist->string);
			s = G_CopyString(sv_maplist->string);	// Knightmare- use G_CopyString instead
			f = NULL;
			t = strtok(s, seps);
			while (t != NULL)
			{
				//add campaigns to maplist
				if (strstr (mapname,t) && strcmp(mapname,t))
				{
					//gi.dprintf ("rwef %s %s\n",mapname, t);
					if (team_list[0] && strstr(team_list[0]->nextmap,t))
					{
						//if campaign points back to first map, move on in maplist
						char ck[20];
						Com_sprintf (ck, sizeof(ck), "%s", t);
						Q_strncatz (ck, sizeof(ck), "1");
						//gi.dprintf ("crere %s %s\n",ck, t);
						if ( strcmp(ck, team_list[0]->nextmap) )
						{
							safe_bprintf (PRINT_HIGH, "Next map: %s \n", team_list[0]->nextmap);
							BeginIntermission (CreateTargetChangeLevel (team_list[0]->nextmap) );
							//last_maplist_map_played = team_list[0]->nextmap;
							//first_non_maplist_map = NULL;
							return;
						}
						else
						{
							//last_maplist_map_played = mapname;
							//first_non_maplist_map = NULL;
							mapname = t;//pretend we just left map 'invade' (for example) so it continues maplist
						}
					}
				}
				if (!f)
				f = t;
				t = strtok(NULL, seps);
			}

			// Knightmare- free temp buffer
			if (s != NULL) {
				gi.TagFree (s);
				s = NULL;
			}

		//	s = strdup(sv_maplist->string);
			s = G_CopyString(sv_maplist->string);	// Knightmare- use G_CopyString instead
			f = NULL;
			t = strtok(s, seps);
			while (t != NULL)
			{
				if (Q_stricmp(t, mapname) == 0)   //if the running map is on maplist
				{
					//safe_bprintf (PRINT_HIGH,"sdf %s %s\n",mapname, t);
					// it's in the list, go to the next one
					t = strtok(NULL, seps);
					if (t == NULL) // end of list, go to first one
					{
						if (f == NULL) // there isn't a first one, same level
						{
							if (MapExists(mapname))
							{
								safe_bprintf (PRINT_HIGH, "Next map: %s \n", mapname);
								BeginIntermission (CreateTargetChangeLevel (mapname) );
								//last_maplist_map_played = mapname;
								//first_non_maplist_map = NULL;
								return;
							}
						}
						else
						{
							if (MapExists(f))
							{
								safe_bprintf (PRINT_HIGH, "Next map: %s \n", f);
								BeginIntermission (CreateTargetChangeLevel (f) );
								return;
							}
							else
							{
							//	mapCheck = t;
							//	strncat (mapCheck, "1");
								// Knightmare- use safe buffer method instead
								Q_strncpyz (mapCheckBuf, sizeof(mapCheckBuf), t);
								Q_strncatz (mapCheckBuf, sizeof(mapCheckBuf), "1");
								mapCheck = mapCheckBuf;
								if ( MapExists(mapCheck) )
								{
									safe_bprintf (PRINT_HIGH, "Next map: %s \n", mapCheck);
									BeginIntermission (CreateTargetChangeLevel (mapCheck));
									return;
								}
							}


						}
					}
					else
					{
						if (MapExists(t))
						{
							safe_bprintf (PRINT_HIGH, "Next map: %s \n", t);
							BeginIntermission (CreateTargetChangeLevel (t) );
							return;
						}
						else
						{
							// last_maplist_map_played = t;
							// first_non_maplist_map = NULL;
						//	mapCheck = t;
						//	strncat (mapCheck, "1");
							// Knightmare- use safe buffer method instead
							Q_strncpyz (mapCheckBuf, sizeof(mapCheckBuf), t);
							Q_strncatz (mapCheckBuf, sizeof(mapCheckBuf), "1");
							mapCheck = mapCheckBuf;
						//	gi.dprintf("%s kljfdsjlk\n", mapCheck);
							if ( MapExists(mapCheck) )
							{
								safe_bprintf (PRINT_HIGH, "Next map: %s \n", mapCheck);
								BeginIntermission (CreateTargetChangeLevel (mapCheck) );
								return;
							}
							else
								gi.dprintf("map doesn't exist for sv_maplist.\n");

						}

					}
				}

				if (!f)
					f = t;
				t = strtok(NULL, seps);

			}

			// Knightmare- free temp buffer
			if (s != NULL) {
				gi.TagFree (s);
				s = NULL;
			}

				// if last_maplist_map_played == 0, then start at first maplist map
				// if it has a value, play the map after that
				// if it has a value and map after that is null, play first map in maplist

			// t becomes tb
			// f becomes fb
			// s becomes sb
			// seps stays same


			if (t == NULL) //faf:  happens when running a map thats not on maplist and map is to change
			{
				//let it stay off maplist until a map on the maplist is changed to
				if (team_list[0]->nextmap && MapExists(team_list[0]->nextmap))
				{
					safe_bprintf (PRINT_HIGH, "Next map: %s \n", team_list[0]->nextmap);
					BeginIntermission (CreateTargetChangeLevel (team_list[0]->nextmap));
					return;
				}
				else
				{
					//restart maplist
				//	sb = strdup(sv_maplist->string);
					sb = G_CopyString(sv_maplist->string);	// Knightmare- use G_CopyString instead
					tb = strtok(s, seps);

					if (MapExists(tb))
					{
						safe_bprintf (PRINT_HIGH, "Next map: %s \n", tb);
						BeginIntermission (CreateTargetChangeLevel (tb));
						return;
					}
					else
					{
					//	mapCheck = tb;
					//	strncat (mapCheck, "1");
						// Knightmare- use safe buffer method instead
						Q_strncpyz (mapCheckBuf, sizeof(mapCheckBuf), tb);
						Q_strncatz (mapCheckBuf, sizeof(mapCheckBuf), "1");
						mapCheck = mapCheckBuf;
						if ( MapExists(mapCheck) )
						{
							safe_bprintf (PRINT_HIGH, "Next map: %s \n", mapCheck);
							BeginIntermission (CreateTargetChangeLevel (mapCheck) );
							return;
						}
					}
				}
			}
		}
	}

	// Knightmare- free temp buffer
	if (sb != NULL) {
		gi.TagFree (sb);
		sb = NULL;
	}
	// end Knightmare

// alternate maplist, not as good

//	else if ((int)dmflags->value & DF_MAP_LIST)  // maplist active?
	else if ((int)dmflags->value & DF_MAP_LIST  && maplist.nummaps > 0)  // faf: fixes crash
	{
		switch (maplist.rotationflag)        // choose next map in list
		{
		case ML_ROTATE_SEQ:        // sequential rotation
			i = (maplist.currentmap + 1) % maplist.nummaps;
			break;

		case ML_ROTATE_RANDOM:     // random rotation
			i = (int) (random() * maplist.nummaps);
			break;

		default:       // should never happen, but set to first map if it does
			i = 0;
		} // end switch


		maplist.currentmap = i;

		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		if (maplist.mapnames[i] && !level.nextmap[0])
			ent->map = maplist.mapnames[i];
		else if (level.nextmap[0])
			ent->map = level.nextmap;
		else
			ent->map = level.mapname;

		safe_bprintf (PRINT_HIGH, "Next map: %s \n", ent->map);

	}

	else if (Last_Team_Winner <= 1 && Last_Team_Winner > -1 && team_list[Last_Team_Winner] && team_list[Last_Team_Winner]->nextmap)
	{
		ent= G_Spawn();
		ent->classname = "target_changelevel";
		//ent->map = team_list[Last_Team_Winner]->nextmap;
		//faf: f campaign mode
		ent->map = team_list[0]->nextmap;
		safe_bprintf (PRINT_HIGH, "Next map: %s \n", ent->map);
	}
	//tie game, advance to next map
	else if (Last_Team_Winner == -1 && team_list[0] && team_list[0]->nextmap)
	{
		ent= G_Spawn();
		ent->classname = "target_changelevel";
		//ent->map = team_list[Last_Team_Winner]->nextmap;
		//faf: f campaign mode
		ent->map = team_list[0]->nextmap;
		safe_bprintf (PRINT_HIGH, "Next map: %s \n", ent->map);
	}
	else if (level.nextmap[0])
	{	// go to a specific map
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = level.nextmap;
		safe_bprintf (PRINT_HIGH, "Next map: %s \n", ent->map);
	}
	else
	{	// search for a changeleve
		ent = G_Find (NULL, FOFS(classname), "target_changelevel");
		if (!ent)
		{	// the map designer didn't include a changelevel,
			// so create a fake ent that goes back to the same level
			ent = G_Spawn ();
			ent->classname = "target_changelevel";
			ent->map = level.mapname;
			safe_bprintf (PRINT_HIGH, "Next map: %s \n", ent->map);
		}
	}

	if (ent)
		BeginIntermission (ent);
}

/*
=================
CheckDMRules
=================
*/
void CheckDMRules (void)
{
	int			i=0, tempscore=0;
	float		delay=0.0;
	qboolean Is_Game_Over=false;
	gclient_t	*cl;

	int allies_kills_win = 0;
	int axis_kills_win = 0;
	int allies_points_win = 0;
	int axis_points_win = 0;


	if (level.intermissiontime)
		return;

	if (!deathmatch->value)
		return;


	//faf: ctb code
	if (level.ctb_time)
	{
		vec3_t		w;	// faf
		float		range;	// faf
		edict_t		*check = NULL;
		edict_t		*usaflag = NULL;
		edict_t     *grmflag = NULL;
		edict_t     *e = NULL;

		if (level.time == level.ctb_time)
			gi.bprintf (PRINT_HIGH, "Timelimit hit!\n");

		if (level.time == level.ctb_time + 1)
			gi.bprintf (PRINT_HIGH, "Next team to bring the briefcase to their base wins!\n");

		if (level.time >= level.ctb_time)
		{
//			gi.bprintf (PRINT_HIGH, "Timelimit hit!\n");

			for (check = g_edicts; check < &g_edicts[globals.num_edicts]; check++)
			{
				if (!check->inuse)
					continue;

				if (!strcmp(check->classname, "usa_base"))
				{
					usaflag = check;
				}

			}

			for (check = g_edicts; check < &g_edicts[globals.num_edicts]; check++)
			{
				if (!check->inuse)
					continue;

				if (!strcmp(check->classname, "grm_base"))
				{
					grmflag = check;
				}
			}


			if (grmflag && usaflag)
			{
				for (check = g_edicts; check < &g_edicts[globals.num_edicts]; check++)
				{
					if (!check->inuse)
						continue;

					if (check->deadflag)
						continue;


					if (!strcmp(check->classname, "briefcase"))
					{

						VectorSubtract (check->s.origin, usaflag->s.origin, w);
						range = VectorLength (w);

						if (range < 40)  //briefcase is near usa flag at end of map
						{
							team_list[0]->score	+= 100;
						}

						VectorSubtract (check->s.origin, grmflag->s.origin, w);
						range = VectorLength (w);

						if (range < 40)  //briefcase is near grm flag at end of map
						{
							team_list[1]->score	+= 100;

						}
					}

				}
				//see if anyone's carrying a briefcase near the flag
				for (i=0 ; i < game.maxclients ; i++)
				{
					e = g_edicts + 1 + i;
					if (!e->inuse || e->flyingnun || !e->client)
						continue;

					if(e->client->pers.inventory[ITEM_INDEX(FindItem("briefcase"))])
					{
						VectorSubtract (e->s.origin, usaflag->s.origin, w);
						range = VectorLength (w);

						if (range < 40)  //briefcase is near usa flag at end of map
						{
							team_list[0]->score	+= 100;
						}

						VectorSubtract (e->s.origin, grmflag->s.origin, w);
						range = VectorLength (w);

						if (range < 40)  //briefcase is near grm flag at end of map
						{
							team_list[1]->score	+= 100;

						}
					}

				}
			}
		}
	}
//faf end


	for(i=0; i < MAX_TEAMS;i++)
	{
		if (!team_list[i])
			break;

		if (team_list[i]->time_to_win)
		{

			delay = (team_list[i]->time_to_win - level.time);

		/*	gi.dprintf("time_to_win [%i] = %f\n", i, team_list[i]->time_to_win);
			gi.dprintf("delay       [%i] = %f\n", i, delay);
			gi.dprintf("level.time  [%i] = %f\n", i, level.time); */

			if (delay <= 0) {
				safe_bprintf( PRINT_HIGH, "Team %s is victorious!\n", team_list[i]->teamname);
				Last_Team_Winner=i;
				EndDMLevel ();
				break;
			}

			else if (delay == 30)
				safe_bprintf ( PRINT_HIGH, "30 seconds left before team %s wins the battle!\n", team_list[i]->teamname);
			else if (delay == 60)
				safe_bprintf ( PRINT_HIGH, "1 minute left before team %s wins the battle!\n", team_list[i]->teamname);
			else if (delay == 300)
				safe_bprintf ( PRINT_HIGH, "5 minutes left before team %s wins the battle!\n", team_list[i]->teamname);
		}
	}
	//faf:  rewrite this so tie games are announced correctly
	if (team_list[0] && team_list[1])
	{
		if (team_list[0]->need_kills > 0 &&
			team_list[0]->kills >= team_list[0]->need_kills)
			allies_kills_win++;

		if (team_list[1]->need_kills > 0 &&
			team_list[1]->kills >= team_list[1]->need_kills)
			axis_kills_win++;

		if (team_list[0]->need_points > 0 &&
			team_list[0]->score >= team_list[0]->need_points)
			allies_points_win++;

		if (team_list[1]->need_points > 0 &&
			team_list[1]->score >= team_list[1]->need_points)
			axis_points_win++;

		if (team_list[0]->kills_and_points)
		{
			if (allies_kills_win + allies_points_win < 2)
			{
				allies_kills_win = 0;
				allies_points_win = 0;
			}
		}
		if (team_list[1]->kills_and_points)
		{
			if (axis_kills_win + axis_points_win < 2)
			{
				axis_kills_win = 0;
				axis_points_win = 0;
			}
		}

		if (allies_kills_win || allies_points_win ||
			axis_kills_win || axis_points_win)
		{
			if (allies_kills_win + allies_points_win >
				axis_kills_win + axis_points_win)
			{
				if (allies_kills_win)
				{
					safe_bprintf( PRINT_HIGH, "Team %s is victorious (%i / %i kills)!\n",
					team_list[0]->teamname,
					team_list[0]->kills,
					team_list[0]->need_kills);

					Last_Team_Winner=0;
					EndDMLevel ();
					return;
				}
				else
				{
					safe_bprintf( PRINT_HIGH, "Team %s is victorious (%i / %i points)!\n",
					team_list[0]->teamname,
					team_list[0]->score,
					team_list[0]->need_points);

					Last_Team_Winner=0;
					EndDMLevel ();
					return;
				}
			}
			else if (allies_kills_win + allies_points_win <
				axis_kills_win + axis_points_win)
			{
				if (axis_kills_win)
				{
					safe_bprintf( PRINT_HIGH, "Team %s is victorious (%i / %i kills)!\n",
					team_list[1]->teamname,
					team_list[1]->kills,
					team_list[1]->need_kills);

					Last_Team_Winner=1;
					EndDMLevel ();
					return;
				}
				else
				{
				safe_bprintf( PRINT_HIGH, "Team %s is victorious (%i / %i points)!\n",
					team_list[1]->teamname,
					team_list[1]->score,
					team_list[1]->need_points);

					Last_Team_Winner=1;
					EndDMLevel ();
					return;
				}
			}
			else //faf:  for tie games
			{
//lol				centerprintall("TTTTTT III EEEE   GGG   AA  M   M EEEE !!!\n  TT    I  E     G     A  A MM MM E    !!!\n  TT    I  EEE   G  GG AAAA M M M EEE  !!!\n  TT    I  E     G   G A  A M   M E       \n  TT   III EEEE   GGG  A  A M   M EEEE !!!\n");
				centerprintall("T I E   G A M E !");
				 Last_Team_Winner = -1;

				EndDMLevel ();
				return;
			}
		}
	}


//faf: this is not used below
	if(Is_Game_Over)
	{
		for(i=0;i<MAX_TEAMS;i++)
		{
			if(team_list[i]->score > tempscore)
			{
				tempscore=team_list[i]->score;
				Last_Team_Winner=i;
			}
		}
		safe_bprintf (PRINT_HIGH, "%s is victorius.\n",team_list[Last_Team_Winner]->teamname);
		EndDMLevel ();
		return;
	}



	if (timelimit->value)
	{
		if (level.time >= timelimit->value*60)
		{
			safe_bprintf (PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel ();
			return;
		}
	}

	if (fraglimit->value)
	{
		for (i=0 ; i<maxclients->value ; i++)
		{
			cl = game.clients + i;
			if (!g_edicts[i+1].inuse)
				continue;

			if (cl->resp.score >= fraglimit->value)
			{
				safe_bprintf (PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel ();
				return;
			}
		}
	}
}


/*
=============
ExitLevel
=============
*/
void ExitLevel (void)
{
	int		i;
	edict_t	*ent;
	char	command [256];
	char	campaignfilename[MAX_QPATH] = "";

	WriteCampaignTxt();


if (campaign_winner>-1)
{
	Com_sprintf (campaignfilename, sizeof(campaignfilename), "dday/campaigns/%s.campaign", level.campaign);
/*	won't work  if (remove (campaignfilename))
		gi.dprintf("removed %s\n",campaignfilename); */
	SetupCampaign (true);
	gi.cvar_set("campaign", "");

}

	//JABot[start] (Disconnect all bots before changing map)
	BOT_RemoveBot("all", NULL);
	//[end]






	Com_sprintf (command, sizeof(command), "map \"%s\"\n", level.changemap);
//	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames ();
/*-----/ PM /-----/ NEW:  Remove all flames before leaving level. /-----*/
    PBM_KillAllFires ();
/*----------------------------------------------------------------------*/


	// clear some things before going to next level
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->health > ent->client->pers.max_health)
			ent->health = ent->client->pers.max_health;
	}

	//LevelExitUserDLLs();
}



int HumanPlayerCount(void)
{
	int i;
	int playercount = 0;
    edict_t *check_ent;


	for (i = 1; i <= maxclients->value; i++)
    {
         check_ent = g_edicts + i;
         if (!check_ent->inuse)
			 continue;
		 if (!check_ent->client ||
			 !check_ent->client->resp.team_on)
			 continue;
		 if (check_ent->ai)
			 continue;

		 playercount++;

	}
	return playercount;

}





/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/
void G_RunFrame (void)
{
	int		i;
	edict_t	*ent,*check_ent;
	int axiscount, alliedcount;

	level.framenum++;
	level.time = level.framenum*FRAMETIME;

	// choose a client for monsters to target this frame
	//AI_SetSightClient ();

	// exit intermissions

	if (level.exitintermission)
	{
		ExitLevel ();
		return;
	}


	//Add bots (JABOT)
/*	if (bots->value && !qbots && level.framenum > 30 &&
		playerminforbots->value < num_clients
		)
		/*&&		playerminforbots->value > num_clients &&
		(!playermaxforbots->value || playermaxforbots->value > num_clients)) */
/*	{
		int allied, axis;

		qbots = true;

		allied = alliedbots->value;
		axis = axisbots->value;
		while (allied + axis > 0)		{
			if (allied >0)
			{
				BOT_SpawnBot ( 0, "", "", NULL);
				allied --;			}
			if (axis > 0)
			{
				BOT_SpawnBot ( 1, "", "", NULL);
				axis --;			}
		}
	}*/

	if ((level.framenum%40 == 1 && qbots && bots->value == 0) ||
		(bots->value &&
		//num_clients >= playermaxforbots->value
		HumanPlayerCount() >= playermaxforbots->value

		))
	{

		qbots = false;
		for (i = 1; i <= maxclients->value; i++)
		{
			check_ent = g_edicts + i;
			if (!check_ent->inuse)
				continue;
			if (!check_ent->client)
				continue;

			if (check_ent->ai)
				BOT_RemoveBot("",check_ent);

		}
	}

	//maintain certain player/bot levels per team according to cvars
	else if (level.framenum > 40 && bots->value &&
		((level.framenum < 300 && level.framenum%10 == 1) ||
		(level.framenum%40 == 1))) //check every 4 seconds, except in beginning
	{
		alliedcount = PlayerCountForTeam(0, true);
		axiscount = PlayerCountForTeam(1, true);
		if (alliedcount > alliedlevel->value)
		{
			for (i = 1; i <= maxclients->value; i++)
			{
				check_ent = g_edicts + i;
				if (!check_ent->inuse)
					continue;
				if (!check_ent->client ||
					!check_ent->client->resp.team_on)
					continue;

				if (check_ent->client->resp.team_on->index != 0)
					continue;
				if (check_ent->ai)
				{
					BOT_RemoveBot("",check_ent);
					if (PlayerCountForTeam(0, true) <= alliedlevel->value)
						break;
				}

			}
		}
		else if (level.allied_cmps && alliedcount < alliedlevel->value &&
			 ( level.framenum > 1800 || (HumanPlayerCount() <= playermaxforbots->value - 1 || playermaxforbots->value == 0) ))
		{
				BOT_SpawnBot ( 0, "", "", NULL);
		}

		if (axiscount > axislevel->value)
		{
			for (i = 1; i <= maxclients->value; i++)
			{
				check_ent = g_edicts + i;
				if (!check_ent->inuse)
					continue;
				if (!check_ent->client ||
					!check_ent->client->resp.team_on)
					continue;

				if (check_ent->client->resp.team_on->index != 1)
					continue;
				if (check_ent->ai)
				{
					BOT_RemoveBot("",check_ent);
					if (PlayerCountForTeam(1, true) <= axislevel->value)
						break;
				}

			}
		}
		else if (level.axis_cmps && axislevel->value && axiscount < axislevel->value &&
		 ( level.framenum > 1800 || (HumanPlayerCount() <= playermaxforbots->value - 1 || playermaxforbots->value == 0) ))
		{
			BOT_SpawnBot ( 1, "", "", NULL);
		}
	}

	//
	// treat each object in turn
	// even the world gets a chance to think
	//
	ent = &g_edicts[0];
	for (i=0 ; i<globals.num_edicts ; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		level.current_entity = ent;

		VectorCopy (ent->s.origin, ent->s.old_origin);

		// if the ground entity moved, make sure we are still on it
		/*if ((ent->groundentity) && (ent->groundentity->linkcount != ent->groundentity_linkcount))
		{
			ent->groundentity = NULL;
			if ( !(ent->flags & (FL_SWIM|FL_FLY)) && (ent->svflags & SVF_MONSTER) )
			{
				M_CheckGround (ent);
			}
		}*/

		if (i > 0 && i <= maxclients->value)
		{
			ClientBeginServerFrame (ent);
			//JABot[start]
			if( !ent->ai ) //jabot092(2)
			//[end]
				continue;
		}

		G_RunEntity (ent);
	}

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();

	//JABot[start]
	AITools_Frame();	//give think time to AI debug tools
	//[end]

	if (nohud->value && level.framenum %100 == 1)
			gi.configstring (CS_STATUSBAR, " if 21 xr 1 0 yb 0 xv  0 yv  0 pic 21 endif "); //just crosshair


}

