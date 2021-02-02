// g_main.c

#include "g_local.h"

game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals;
spawn_temp_t	st;

edict_t			*g_edicts;

//CW++
teamgame_t		teamgame;
asltgame_t		asltgame;
voteinfo_t		g_vote;

level_locals_t	g_oldmaps[MAX_OLDMAPS];
edict_t			*g_slots[MAX_CLIENTS];
int				g_reserve_used = 0;
int				g_public_used = 0;
int				g_round = 0;

int				hook_index;
int				spike_index;
int				r_explode_index;
int				tracer_index;
//CW--

int				sm_meat_index;
int				snd_fry;
int				meansOfDeath;


// Console variables.

cvar_t	*deathmatch;
cvar_t	*dmflags;
cvar_t	*fraglimit;
cvar_t	*timelimit;

//ZOID++
cvar_t	*capturelimit;
//ZOID--

cvar_t	*password;
cvar_t	*maxclients;
cvar_t	*maxentities;
cvar_t	*g_select_empty;
cvar_t	*dedicated;

cvar_t	*filterban;

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

cvar_t	*flood_msgs;
cvar_t	*flood_persecond;
cvar_t	*flood_waitdelay;

//CW++
cvar_t	*sv_gametype;
cvar_t	*sv_respawn_invuln_time;
cvar_t	*sv_rnd_powerup_timeout;
cvar_t	*sv_rnd_mystery_ratio;
cvar_t	*sv_show_leader;
cvar_t	*sv_quad_factor;
cvar_t	*sv_reject_blank_ip;
cvar_t	*sv_map_random;

cvar_t	*sv_op_password;
cvar_t	*sv_op_ignore_maplist;

cvar_t	*sv_reserved;
cvar_t	*sv_rsv_password;

cvar_t	*sv_map_file;
cvar_t	*sv_config_file;
cvar_t	*sv_motd_file;

cvar_t	*sv_agm_drop;
cvar_t	*sv_agm_drop_file;

cvar_t	*sv_disc_drop;
cvar_t	*sv_disc_drop_file;

cvar_t	*competition;
cvar_t	*matchstarttime;

cvar_t	*sv_team1_name;
cvar_t	*sv_team2_name;
cvar_t	*sv_team1_model;
cvar_t	*sv_team2_model;
cvar_t	*sv_team1_skin;
cvar_t	*sv_team2_skin;

cvar_t	*sv_allow_bots;
cvar_t	*sv_bots_camp;
cvar_t	*sv_bots_insult;
cvar_t	*sv_bots_taunt;
cvar_t	*sv_bots_chat;
cvar_t	*sv_bots_random;
cvar_t	*sv_bots_use_file;
cvar_t	*sv_bots_config_file;

cvar_t	*chedit;

cvar_t	*sv_allow_gausspistol;
cvar_t	*sv_allow_mac10;
cvar_t	*sv_allow_jackhammer;
cvar_t	*sv_allow_traps;
cvar_t	*sv_allow_c4;
cvar_t	*sv_allow_spikegun;
cvar_t	*sv_allow_rocketlauncher;
cvar_t	*sv_allow_flamethrower;
cvar_t	*sv_allow_railgun;
cvar_t	*sv_allow_shockrifle;
cvar_t	*sv_allow_agm;
cvar_t	*sv_allow_disclauncher;
cvar_t	*sv_allow_hook;

cvar_t	*sv_allow_invuln;
cvar_t	*sv_allow_quad;
cvar_t	*sv_allow_siphon;
cvar_t	*sv_allow_d89;
cvar_t	*sv_allow_haste;
cvar_t	*sv_allow_tele;
cvar_t	*sv_allow_antibeam;
cvar_t	*sv_allow_enviro;
cvar_t	*sv_allow_silencer;
cvar_t	*sv_allow_breather;

cvar_t	*sv_allow_bullets;
cvar_t	*sv_allow_shells;
cvar_t	*sv_allow_rockets;
cvar_t	*sv_allow_cells;
cvar_t	*sv_allow_slugs;

cvar_t	*sv_chainsaw_damage;

cvar_t	*sv_deserteagle_damage;
cvar_t	*sv_deserteagle_hspread;
cvar_t	*sv_deserteagle_vspread;

cvar_t	*sv_jackhammer_damage;
cvar_t	*sv_jackhammer_hspread;
cvar_t	*sv_jackhammer_vspread;

cvar_t	*sv_mac10_damage;
cvar_t	*sv_mac10_hspread;
cvar_t	*sv_mac10_vspread;

cvar_t	*sv_gauss_damage_base;
cvar_t	*sv_gauss_damage_rate;
cvar_t	*sv_gauss_damage_step;
cvar_t	*sv_gauss_damage_max;
cvar_t	*sv_gauss_damage_particle;
cvar_t	*sv_gauss_scan_range;

cvar_t	*sv_c4_damage;
cvar_t	*sv_c4_radius;
cvar_t	*sv_c4_touchbang;
cvar_t	*sv_c4_proximity;
cvar_t	*sv_c4_proximity_range;
cvar_t	*sv_c4_proximity_delay;
cvar_t	*sv_c4_proximity_life;
cvar_t	*sv_c4_min_speed;
cvar_t	*sv_c4_max_speed;
cvar_t	*sv_c4_hold_accel;
cvar_t	*sv_c4_timelimit;
cvar_t	*sv_c4_spawn_range;

cvar_t	*sv_trap_hook_damage;
cvar_t	*sv_trap_hook_speed;
cvar_t	*sv_trap_beam_damage;
cvar_t	*sv_trap_beam_power;
cvar_t	*sv_trap_proximity_range;
cvar_t	*sv_trap_activate_delay;
cvar_t	*sv_trap_min_speed;
cvar_t	*sv_trap_max_speed;
cvar_t	*sv_trap_hold_accel;
cvar_t	*sv_trap_thru_tele;
cvar_t	*sv_traps_max_active;
cvar_t	*sv_traps_timelimit;

cvar_t	*sv_spike_damage;
cvar_t	*sv_spike_bang_damage;
cvar_t	*sv_spike_bang_radius;
cvar_t	*sv_spike_bang_delay;
cvar_t	*sv_spike_speed;

cvar_t	*sv_rocket_damage;
cvar_t	*sv_rocket_radius;
cvar_t	*sv_rocket_radius_damage;
cvar_t	*sv_rocket_speed;
cvar_t	*sv_rocket_kick_scale;

cvar_t	*sv_flame_damage;
cvar_t	*sv_flame_small_damage;
cvar_t	*sv_flame_speed;
cvar_t	*sv_firebomb_damage;
cvar_t	*sv_firebomb_radius;
cvar_t	*sv_firebomb_speed;
cvar_t	*sv_flame_thru_tele;

cvar_t	*sv_railgun_damage;

cvar_t	*sv_shock_homing_damage;
cvar_t	*sv_shock_homing_range;
cvar_t	*sv_shock_speed;
cvar_t	*sv_shock_homing_speed;
cvar_t	*sv_shock_radius_damage;
cvar_t	*sv_shock_radius;
cvar_t	*sv_shock_freeze_time;
cvar_t	*sv_shock_live_time;

cvar_t	*sv_agm_mode;
cvar_t	*sv_agm_charge_rate;
cvar_t	*sv_agm_fire_rate;
cvar_t	*sv_agm_shot_cells;
cvar_t	*sv_agm_beam_cells;
cvar_t	*sv_agm_fling_power;
cvar_t	*sv_agm_cross_damage;
cvar_t	*sv_agm_cross_timeout;
cvar_t	*sv_agm_reflect_damage;
cvar_t	*sv_agm_reflect_timeout;
cvar_t	*sv_agm_invuln_cells;
cvar_t	*sv_agm_disrupt_damage;
cvar_t	*sv_agm_disrupt_siphon;

cvar_t	*sv_disc_damage;
cvar_t	*sv_disc_speed;
cvar_t	*sv_disc_live_time;

cvar_t	*sv_plasma_damage;
cvar_t	*sv_plasma_speed;

cvar_t	*sv_hook_damage;
cvar_t	*sv_hook_speed;
cvar_t	*sv_hook_pull_speed;
cvar_t	*sv_hook_pull_agm;
cvar_t	*sv_hook_offhand;

cvar_t	*sv_health_initial;
cvar_t	*sv_health_max;
cvar_t	*sv_health_max_siphon;
cvar_t	*sv_health_max_bonus;

cvar_t	*sv_have_deserteagle;
cvar_t	*sv_have_gausspistol;
cvar_t	*sv_have_mac10;
cvar_t	*sv_have_jackhammer;
cvar_t	*sv_have_c4;
cvar_t	*sv_have_traps;
cvar_t	*sv_have_spikegun;
cvar_t	*sv_have_rocketlauncher;
cvar_t	*sv_have_flamethrower;
cvar_t	*sv_have_railgun;
cvar_t	*sv_have_shockrifle;
cvar_t	*sv_have_agm;
cvar_t	*sv_have_disclauncher;

cvar_t	*sv_initial_weapon;

cvar_t	*sv_initial_bullets;
cvar_t	*sv_initial_shells;
cvar_t	*sv_initial_c4;
cvar_t	*sv_initial_traps;
cvar_t	*sv_initial_rockets;
cvar_t	*sv_initial_cells;
cvar_t	*sv_initial_slugs;

cvar_t	*sv_initial_armor;
cvar_t	*sv_initial_armortype;

cvar_t	*sv_max_bullets;
cvar_t	*sv_max_shells;
cvar_t	*sv_max_c4;
cvar_t	*sv_max_traps;
cvar_t	*sv_max_rockets;
cvar_t	*sv_max_cells;
cvar_t	*sv_max_slugs;

cvar_t	*sv_max_band_bullets;
cvar_t	*sv_max_band_shells;
cvar_t	*sv_max_band_cells;
cvar_t	*sv_max_band_slugs;

cvar_t	*sv_max_pack_bullets;
cvar_t	*sv_max_pack_shells;
cvar_t	*sv_max_pack_c4;
cvar_t	*sv_max_pack_traps;
cvar_t	*sv_max_pack_rockets;
cvar_t	*sv_max_pack_cells;
cvar_t	*sv_max_pack_slugs;

cvar_t	*sv_mystery_invuln;
cvar_t	*sv_mystery_quad;
cvar_t	*sv_mystery_d89;
cvar_t	*sv_mystery_haste;
cvar_t	*sv_mystery_siphon;
cvar_t	*sv_mystery_antibeam;
cvar_t	*sv_mystery_enviro;
cvar_t	*sv_mystery_tele;

cvar_t	*sv_vote_percentage;
cvar_t	*sv_vote_time;
cvar_t	*sv_vote_minclients;

cvar_t	*sv_vote_allow_map;
cvar_t	*sv_vote_allow_config;
cvar_t	*sv_vote_allow_match;
cvar_t	*sv_vote_allow_timelimit;
cvar_t	*sv_vote_allow_fraglimit;
cvar_t	*sv_vote_allow_capturelimit;
cvar_t	*sv_vote_allow_hook;
cvar_t	*sv_vote_allow_hook_offhand;

cvar_t	*sv_vote_allow_bots;

cvar_t	*sv_vote_allow_gausspistol;
cvar_t	*sv_vote_allow_mac10;
cvar_t	*sv_vote_allow_jackhammer;
cvar_t	*sv_vote_allow_c4;
cvar_t	*sv_vote_allow_traps;
cvar_t	*sv_vote_allow_esg;
cvar_t	*sv_vote_allow_rocketlauncher;
cvar_t	*sv_vote_allow_flamethrower;
cvar_t	*sv_vote_allow_railgun;
cvar_t	*sv_vote_allow_shockrifle;
cvar_t	*sv_vote_allow_agm;
cvar_t	*sv_vote_allow_disclauncher;

cvar_t	*sv_vote_allow_invuln;
cvar_t	*sv_vote_allow_quad;
cvar_t	*sv_vote_allow_siphon;
cvar_t	*sv_vote_allow_d89;
cvar_t	*sv_vote_allow_haste;
cvar_t	*sv_vote_allow_tele;
cvar_t	*sv_vote_allow_antibeam;
cvar_t	*sv_vote_allow_enviro;
cvar_t	*sv_vote_allow_silencer;
cvar_t	*sv_vote_allow_breather;

cvar_t	*sv_vote_allow_nohealth;
cvar_t	*sv_vote_allow_noitems;
cvar_t	*sv_vote_allow_weaponsstay;
cvar_t	*sv_vote_allow_nofalling;
cvar_t	*sv_vote_allow_noarmor;
cvar_t	*sv_vote_allow_infiniteammo;
cvar_t	*sv_vote_allow_quaddrop;
cvar_t	*sv_vote_allow_fastswitch;
cvar_t	*sv_vote_allow_extraitems;
cvar_t	*sv_vote_allow_noreplacements;
cvar_t	*sv_vote_allow_notech;
cvar_t	*sv_vote_allow_ctfspawns;

cvar_t	*sv_vote_timelimit_max;
cvar_t	*sv_vote_timelimit_step;
cvar_t	*sv_vote_fraglimit_max;
cvar_t	*sv_vote_fraglimit_step;
cvar_t	*sv_vote_capturelimit_max;
cvar_t	*sv_vote_capturelimit_step;
//CW--
cvar_t	*footstep_sounds;	// Knightmare added
cvar_t	*sv_step_fraction;	// Knightmare- this was a define in p_view.c

cvar_t	*turn_rider;	// Knightmare added

void SpawnEntities(char *mapname, char *entities, char *spawnpoint);
void ClientThink(edict_t *ent, usercmd_t *cmd);
qboolean ClientConnect(edict_t *ent, char *userinfo);
void ClientUserinfoChanged(edict_t *ent, char *userinfo);
void ClientDisconnect(edict_t *ent);
void ClientBegin(edict_t *ent);
void ClientCommand(edict_t *ent);
void RunEntity(edict_t *ent);
void WriteGame(char *filename, qboolean autosave);
void ReadGame(char *filename);
void WriteLevel(char *filename);
void ReadLevel(char *filename);
void InitGame(void);
void G_RunFrame(void);


//===================================================================

void ShutdownGame(void)
{
	gi.dprintf("==== ShutdownGame ====\n");

	gi.FreeTags(TAG_LEVEL);
	gi.FreeTags(TAG_GAME);
}


/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points and global variables.
=================
*/
game_export_t *GetGameAPI(game_import_t *import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;

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
void Sys_Error(char *error, ...)
{
	va_list	argptr;
	char	text[1024];

	va_start(argptr, error);
//	vsprintf(text, error, argptr);
	Q_vsnprintf(text, sizeof(text), error, argptr);
	va_end(argptr);

	gi.error(ERR_FATAL, "%s", text);
}

void Com_Printf(char *msg, ...)
{
	va_list	argptr;
	char	text[1024];

	va_start(argptr, msg);
//	vsprintf(text, msg, argptr);
	Q_vsnprintf(text, sizeof(text), msg, argptr);
	va_end(argptr);

	gi.dprintf("%s", text);
}
#endif

//======================================================================

/*
=================
ClientEndServerFrames
=================
*/
void ClientEndServerFrames(void)
{
	edict_t	*ent;
	int		i;

	// calc the player views now that all pushing and damage has been added
	for (i = 0; i < (int)maxclients->value; ++i)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;

//CW++
		if (!ent->isabot && !(level.framenum & (50 + i)))		// reduce performance hits
		{

//			Catch llamas and newbies.

			if (strlen(ent->client->pers.netname) == 0)
			{
				StuffCmd(ent, "name Llama\n");
				gi_centerprintf(ent, "Blank names are lame.\n");
			}
			else if (strstr(ent->client->pers.netname, "[BOT]") != NULL)
			{
				StuffCmd(ent, "name Llama\n");
				gi_centerprintf(ent, "AwakenBot spoofing is lame.\n");
			}

//			Notify everyone of name changes, and update old name.

			else if (Q_stricmp(ent->client->pers.netname, ent->client->pers.old_name))
			{
				gi_bprintf(PRINT_CHAT, "%s is now known as %s\n", ent->client->pers.old_name, ent->client->pers.netname);
				strncpy(ent->client->pers.old_name, ent->client->pers.netname, sizeof(ent->client->pers.netname)-1);
			}
		}
//CW--

		ClientEndServerFrame(ent);
	}
}

/*
=================
CreateTargetChangeLevel

Returns the created target changelevel
=================
*/
edict_t *CreateTargetChangeLevel(char *map)
{
	edict_t *ent;

	ent = G_Spawn();
	ent->classname = "target_changelevel";
	Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;

	return ent;
}

//CW++
/*
=================
FindNextMap

Returns the BSP name of the next map in the server's map list after the current level.
=================
*/
char *FindNextMap(FILE *iostream)
{
	char		bspname[MAX_QPATH];
	char		*buf = NULL;
	qboolean	finished = false;
	qboolean	found_first;
	int			fnum;
	int			nmaps = 0;

	while (!finished)
	{
		fnum = fscanf(iostream, "%s", bspname);
		if (fnum != EOF)
		{
			if ((bspname[0] == '/') && (bspname[1] == '/'))
				continue;

			++nmaps;
			if (!Q_stricmp(level.mapname, bspname))
			{
				finished = true;	
				fnum = fscanf(iostream, "%s", bspname);
				if ((fnum == EOF) || ((bspname[0] == '/') && (bspname[1] == '/')))		//current map is last in list
				{
					rewind(iostream);
					found_first = false;
					while (!found_first)
					{
						fscanf(iostream, "%s", bspname);
						if ((bspname[0] == '/') && (bspname[1] == '/'))
							continue;
						else
							found_first = true;
					}
				}
			}
		}
		else	//current map not in list, or map list empty
		{
			finished = true;
			if (nmaps > 0)		//there's at least one map in the list, so select the first
			{
				rewind(iostream);
				found_first = false;
				while (!found_first)
				{
					fscanf(iostream, "%s", bspname);
					if ((bspname[0] == '/') && (bspname[1] == '/'))
						continue;
					else
						found_first = true;
				}
			}
			else
			{
				gi.dprintf("** Map list is empty\n");
				Com_sprintf(bspname, sizeof(bspname), "q2dm1");
			}
		}
	}

	if (strlen(bspname))
		buf = G_CopyString(bspname);
		
	return buf;
}

/*
=================
FindRandomMap

Returns a BSP name from the server's map list (apart from the current level).
=================
*/
char *FindRandomMap(FILE *iostream)
{
	char		bspname[MAX_QPATH];
	char		*buf = NULL;
	qboolean	finished = false;
	qboolean	match_found;
	int			fnum;
	int			nmaps = 0;
	int			nmaps_diff = 0;
	int			count;
	int			iterations = 0;
	int			i;

//	Count the number of bsp names, and the number of bsp names that are different to the current 
//	one, in the server's map list.

	while (!finished)
	{
		fnum = fscanf(iostream, "%s", bspname);
		if (fnum != EOF)
		{
			if ((bspname[0] == '/') && (bspname[1] == '/'))
				continue;

			++nmaps;
			if (Q_stricmp(level.mapname, bspname))
				++nmaps_diff;
		}
		else
			finished = true;
	}

//	If there's at least one bsp name that's different to the current one, randomly select one 
//	from the map list.

	if (nmaps > 0)
	{
		if (nmaps_diff > 0)
		{
			finished = false;
			while (!finished)
			{
				rewind(iostream);
				count = (rand() % nmaps) + 1;
				while (count > 0)
				{
					if ((bspname[0] == '/') && (bspname[1] == '/'))
						continue;

					--count;
					fnum = fscanf(iostream, "%s", bspname);
					if (fnum == EOF)	// shouldn't happen!
					{
						finished = true;
						break;
					}
				}

//				Compare it to the list of the last MAX_OLDMAPS bsps played.
//				Choose again if it's in this recent-bsp list, unless the number of unique maps 
//				in the server's map list is <= MAX_OLDMAPS (ie. we're going to get repetitions 
//				within the recent-bsp list). Don't replay the last map if possible.

				if (nmaps_diff > MAX_OLDMAPS)
				{
					match_found = false;
					for (i = 0; i < MAX_OLDMAPS; ++i)
					{
						if (!Q_stricmp(g_oldmaps[i].mapname, bspname))
							match_found = true;
					}
					if (!match_found)
						finished = true;
				}
				else
				{
					if (Q_stricmp(level.mapname, bspname))
						finished = true;
				}

//				If we've done a few iterations without producing a valid selection, it's likely 
//				that the server's map list is full of repeated bsp names (which b0rks the 
//				selection algorithm). In this case, just use the last bsp name chosen.

				if (++iterations > 10)
					finished = true;
			}
			
		}
		else	// no different maps, so replay the current one
		{
			gi.dprintf("** Map list has no different maps\n");
			Com_sprintf(bspname, sizeof(bspname), level.mapname);
		}
	}
	else
	{
		gi.dprintf("** Map list is empty\n");
		Com_sprintf(bspname, sizeof(bspname), "q2dm1");
	}

	if (strlen(bspname))
		buf = G_CopyString(bspname);
		
	return buf;
}
//CW--

/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded.
=================
*/
void EndDMLevel(void)
{
	edict_t	*ent;

//CW++
	FILE	*mapstream;
	char	bspname[MAX_QPATH];
	char	endmsg[130];

	++g_round;
	level.starttime = 0.0;
	level.leader = NULL;

//	Display the appropriate victory message for Assault games.

	if (sv_gametype->value == G_ASLT)
	{
		if (asltgame.victory)
		{
			if (asltgame.msg_attack)
			{
				if (strlen(asltgame.msg_attack) > 129)
					Com_sprintf(endmsg, sizeof(endmsg), "The Attackers have won.");
				else
					Com_sprintf(endmsg, sizeof(endmsg), "%s", asltgame.msg_attack);

				free(asltgame.msg_attack);
			}
			else
				Com_sprintf(endmsg, sizeof(endmsg), "The Attackers have won.");

			gi_bprintf(PRINT_CHAT, "%s\n", endmsg);
		}
		else
		{
			if (asltgame.msg_defend)
			{
				if (strlen(asltgame.msg_defend) > 129)
					Com_sprintf(endmsg, sizeof(endmsg), "The Defenders have won.");
				else
					Com_sprintf(endmsg, sizeof(endmsg), "%s", asltgame.msg_defend);

				free(asltgame.msg_defend);
			}
			else
				Com_sprintf(endmsg, sizeof(endmsg), "The Defenders have won.");

			gi_bprintf(PRINT_CHAT, "%s\n", endmsg);
		}
	}

//	Write the players' scores to the console.

	if (sv_gametype->value == G_FFA)
		PrintFFAScores();
	else
		PrintTeamScores();

//	For Assault games, restart the level if we've only played one round on it so far.

	if ((sv_gametype->value == G_ASLT) && (g_round % 2))
	{
		BeginIntermission(CreateTargetChangeLevel(level.mapname));
		return;
	}
//CW--

	// stay on same level flag
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		BeginIntermission(CreateTargetChangeLevel(level.mapname));
		return;
	}

	if (*level.forcemap)
	{
//CW++
//		Check that the bsp exists. If not, then reload the current level.

		if (!FileExists(level.forcemap, FILE_MAP))
		{
			gi_bprintf(PRINT_CHAT, "** Cannot open bsp: \"%s\"\n   (Restarting current level).\n\n", level.forcemap);
			BeginIntermission(CreateTargetChangeLevel(level.mapname));
		}
		else
//CW--
			BeginIntermission(CreateTargetChangeLevel(level.forcemap));

		return;
	}

//CW++
//	Select a map from the list (either randomly or sequentially, as flagged).

	if ((mapstream = OpenMaplistFile(true)) != NULL)
	{
		if ((int)sv_map_random->value)
			Com_sprintf(bspname, sizeof(bspname), "%s", FindRandomMap(mapstream));
		else
			Com_sprintf(bspname, sizeof(bspname), "%s", FindNextMap(mapstream));
		
		fclose(mapstream);

//		Check that the bsp exists. If not, then reload the current level.

		if (!FileExists(bspname, FILE_MAP))
		{
			gi_bprintf(PRINT_CHAT, "** Cannot open bsp: \"%s\"\n   (Restarting current level).\n\n", bspname);
			BeginIntermission(CreateTargetChangeLevel(level.mapname));
			return;
		}

		gi.dprintf(">> Loading bsp: \"%s\"\n\n", bspname);
		BeginIntermission(CreateTargetChangeLevel(bspname));	
		return;
	}
//CW--

	if (level.nextmap[0])			// go to a specific map
		BeginIntermission(CreateTargetChangeLevel(level.nextmap) );
	else							// search for a changelevel
	{
		ent = G_Find(NULL, FOFS(classname), "target_changelevel");
		if (!ent)
		{	// the map designer didn't include a changelevel, so create a fake ent that goes back to the same level
			BeginIntermission(CreateTargetChangeLevel(level.mapname));
			return;
		}
		BeginIntermission(ent);
	}
}

/*
=================
CheckDMRules
=================
*/
void CheckDMRules(void)
{
	gclient_t	*cl;
	int			i;
//CW++
	edict_t		*e;
	char		text[10];
	int			t;
//CW--

	if (level.intermissiontime)
		return;

//CW++							
	if ((g_vote.vote != VOTE_NONE) && (g_vote.votetime <= level.time))
	{
		gi_bprintf(PRINT_CHAT, "Requested vote timed out and has been cancelled.\n");
		g_vote.vote = VOTE_NONE;
	}

//	Display the time remaining in non-match games (time display code is elsewhere for matches).

	if (timelimit->value && (level.time - (int)level.time == 0.0))
	{
		if (!teamgame.match)
		{
			t = (int)(((timelimit->value * 60.0) + level.starttime) - level.time);
			Com_sprintf(text, sizeof(text), "%02d:%02d", (int)(t / 60), (int)(t % 60));
			gi.configstring(CONFIG_CTF_MATCH, text);
		}
		else
			t = (int)(teamgame.matchtime - level.time);

//		Warn everyone at the 5- and 1-minute(s)-remaining points (matches, too).

		if ((t == 300) || (t == 60))
		{
			for (i = 1; i <= (int)maxclients->value; ++i)
			{
				e = g_edicts + i;
				if (e->inuse)
				{
					gi_centerprintf(e, "%s remaining.\n", (t == 300)?"Five minutes":"One minute");
					gi.sound(e, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NORM, 0);
				}
			}
		}

//		Give a voice countdown at the 10-second mark.

		if ((t == 10) && (teamgame.match == MATCH_NONE))
			gi.positioned_sound(world->s.origin, world, (CHAN_AUTO|CHAN_RELIABLE), gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);
	}
//CW--

//ZOID++
	if (((sv_gametype->value == G_CTF) || (sv_gametype->value == G_TDM)) && CTFCheckRules())		//CW
	{
		EndDMLevel();
		return;
	}

//CW++
	else if ((sv_gametype->value == G_ASLT) && ASLTCheckRules())
	{
		EndDMLevel();
		return;
	}
//CW--

	if (CTFInMatch())
		return;
//ZOID--

	if (timelimit->value)
	{
		if (level.time >= timelimit->value * 60.0)
		{
			gi_bprintf(PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel();
			return;
		}
	}

	if ((int)fraglimit->value && (sv_gametype->value != G_ASLT))									//CW
	{
		for (i = 0; i < (int)maxclients->value; i++)
		{
			cl = game.clients + i;
			if (!g_edicts[i+1].inuse)
				continue;

			if (cl->resp.score >= (int)fraglimit->value)
			{
				gi_bprintf(PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel();
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
void ExitLevel(void)
{
	edict_t	*ent;
	char	command[256];
	int		i;

	level.exitintermission = 0;
	level.intermissiontime = 0;

	if (CTFNextMap())
		return;

	Com_sprintf(command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString(command);
	ClientEndServerFrames();

	level.changemap = NULL;

	// clear some things before going to next level
	for (i = 0; i < (int)maxclients->value; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;

		if (ent->health > ent->client->pers.max_health)
			ent->health = ent->client->pers.max_health;
	}
}


/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/
void G_RunFrame(void)
{
	edict_t	*ent;
	int		i;

	level.framenum++;
	level.time = level.framenum * FRAMETIME;
	
//CW++
	if (level.starttime == 0.0)
		level.starttime = level.time;
//CW--

//	Exit intermissions.

	if (level.exitintermission)
	{
		ExitLevel();
		return;
	}

//	Treat each object in turn; even the world gets a chance to think.

	ent = &g_edicts[0];
	for (i = 0; i < globals.num_edicts; ++i, ++ent)
	{
		if (!ent->inuse)
			continue;

		level.current_entity = ent;
		VectorCopy(ent->s.origin, ent->s.old_origin);

//		If the ground entity moved, make sure we are still on it.

		if ((ent->groundentity) && (ent->groundentity->linkcount != ent->groundentity_linkcount))
			ent->groundentity = NULL;

		if ((i > 0) && (i <= (int)maxclients->value))
		{
			ClientBeginServerFrame(ent);
			continue;
		}

		G_RunEntity(ent);
	}

//	See if it is time to end a deathmatch.

	CheckDMRules();

//	Build the playerstate_t structures for all players.

	ClientEndServerFrames();
}

//CW++
// if (!kill(self)) self->strength++;
//CW--
