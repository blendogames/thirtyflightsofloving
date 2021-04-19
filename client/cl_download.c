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

// cl_download.c  -- client autodownload code
// moved from cl_main.c and cl_parse.c

#include "client.h"

extern	cvar_t *allow_download;
extern	cvar_t *allow_download_players;
extern	cvar_t *allow_download_models;
extern	cvar_t *allow_download_sounds;
extern	cvar_t *allow_download_maps;
// Knightmare- whether to allow downloading 24-bit textures
extern	cvar_t *allow_download_textures_24bit;

int precache_check; // for autodownload of precache items
int precache_spawncount;
int precache_tex;
int precache_model_skin;
int precache_pak;	// Knightmare added

byte *precache_model; // used for skin checking in alias models

#ifdef USE_CURL
qboolean	precache_forceUDP = false;	// force all downloads to UDP
qboolean	filelistUseGamedir = false;

// HTTP Fallback handling
// First we're trying to download all files over HTTP with r1q2-style URLs.
// If we encountered errors, we reset the complete precacher state and retry with HTTP and q2pro-style URLs.
// If we still got errors we're falling back to UDP.
// Iteration 1: Initial state, R1Q2-style URLs.
// Iteration 2: Q2Pro-style URLs.
// Iteration 3: UDP downloads.
typedef enum {
	ITER_HTTP_R1Q2 = 0,
	ITER_HTTP_Q2PRO = 1,
	ITER_UDP = 2,
} precacheIteration_t;

static precacheIteration_t precache_iteration = ITER_HTTP_R1Q2;
#endif	// USE_CURL

#define PLAYER_MULT 5

// ENV_CNT is map load, ENV_CNT+1 is first env map
#define ENV_CNT (CS_PLAYERSKINS + MAX_CLIENTS * PLAYER_MULT)
#define TEXTURE_CNT (ENV_CNT+13)

// Knightmare- old configstrings for version 34 client compatibility
#define OLD_ENV_CNT (OLD_CS_PLAYERSKINS + MAX_CLIENTS * PLAYER_MULT)
#define OLD_TEXTURE_CNT (OLD_ENV_CNT+13)


static const char *env_suf[6] = {"rt", "bk", "lf", "ft", "up", "dn"};

void CL_InitFailedDownloadList (void);


/*
=================
CL_RequestNextDownload
=================
*/
void CL_RequestNextDownload (void)
{
	unsigned	map_checksum;		// for detecting cheater maps
	char		fn[MAX_OSPATH];
	dmd2_t		*md2header;
	dmd3_t		*md3header;
	dmd3mesh_t	*md3mesh;
	dspr2_t		*sp2header;
	char		*skinname;
	int			cs_sounds, cs_playerskins, cs_images;
	int			max_models, max_sounds, max_images;
	int			env_cnt, texture_cnt;

	if (cls.state != ca_connected)
		return;

	// clear failed download list
	if (precache_check == CS_MODELS)
		CL_InitFailedDownloadList ();

	// Knightmare- hack for connected to server using old protocol
	// Changed config strings require different parsing
	if ( LegacyProtocol() )
	{
		cs_sounds		= OLD_CS_SOUNDS;
		cs_playerskins	= OLD_CS_PLAYERSKINS;
		cs_images		= OLD_CS_IMAGES;
		max_models		= OLD_MAX_MODELS;
		max_sounds		= OLD_MAX_SOUNDS;
		max_images		= OLD_MAX_IMAGES;
		env_cnt			= OLD_ENV_CNT;
		texture_cnt		= OLD_TEXTURE_CNT;
	}
	else
	{
		cs_sounds		= CS_SOUNDS;
		cs_playerskins	= CS_PLAYERSKINS;
		cs_images		= CS_IMAGES;
		max_models		= MAX_MODELS;
		max_sounds		= MAX_SOUNDS;
		max_images		= MAX_IMAGES;
		env_cnt			= ENV_CNT;
		texture_cnt		= TEXTURE_CNT;
	}

	// YQ2 Q2pro download addition
#ifdef USE_CURL
	if (precache_iteration == ITER_HTTP_R1Q2) {
		CL_HTTP_SetDownloadGamedir (cl.gamedir);
	}
	else if (precache_iteration == ITER_HTTP_Q2PRO)
	{
		if (cl.gamedir[0] == '\0')
			CL_HTTP_SetDownloadGamedir (BASEDIRNAME);
		else
			CL_HTTP_SetDownloadGamedir (cl.gamedir);
	}
	else {	// if (precache_iteration == ITER_UDP)
		precache_forceUDP = true;
	}
#endif	// USE_CURL

	// Skip to loading map if downloading disabled or on local server
	if ( (Com_ServerState() || !allow_download->integer) && precache_check < env_cnt)
		precache_check = env_cnt;

	// Try downloading pk3 file for current map from server, hack by Jay Dolan
	if ( !LegacyProtocol() && precache_check == CS_MODELS && precache_pak == 0 )
	{
		precache_pak++;
		if (strlen(cl.configstrings[CS_PAKFILE])) {
			if (!CL_CheckOrDownloadFile(cl.configstrings[CS_PAKFILE]))
				return;  // started a download
		}
	}

	// ZOID
	if (precache_check == CS_MODELS) { // confirm map
		precache_check = CS_MODELS+2; // 0 isn't used
		if (allow_download_maps->integer) {
			if (!CL_CheckOrDownloadFile(cl.configstrings[CS_MODELS+1]))
				return; // started a download
		}
	}
	if (precache_check >= CS_MODELS && precache_check < CS_MODELS+max_models)
	{
		if (allow_download_models->integer)
		{
			while (precache_check < CS_MODELS+max_models &&
				cl.configstrings[precache_check][0])
			{
				if (cl.configstrings[precache_check][0] == '*' ||
					cl.configstrings[precache_check][0] == '#') {
					precache_check++;
					continue;
				}
				if (precache_model_skin == 0)
				{
					if (!CL_CheckOrDownloadFile(cl.configstrings[precache_check])) {
						precache_model_skin = 1;
						return; // started a download
					}
					precache_model_skin = 1;
				}

#ifdef USE_CURL	// HTTP downloading from R1Q2
				// pending downloads (models), let's wait here before we can check skins.
				if ( CL_PendingHTTPDownloads() ) {
					return;
				}
#endif	// USE_CURL

				// checking for skins in the model
				if (!precache_model)
				{
					FS_LoadFile (cl.configstrings[precache_check], (void **)&precache_model);
					if (!precache_model) {
						precache_model_skin = 0;
						precache_check++;
						continue; // couldn't load it
					}
					if (LittleLong(*(unsigned *)precache_model) != IDMD2HEADER)
					{	// is it an md3?
						if (LittleLong(*(unsigned *)precache_model) != IDMD3HEADER)
						{	// is it a sprite?
							if (LittleLong(*(unsigned *)precache_model) != IDSP2HEADER)
							{
								// not a recognized model
								FS_FreeFile(precache_model);
								precache_model = 0;
								precache_model_skin = 0;
								precache_check++;
								continue;
							}
							else
							{	// get sprite header
								sp2header = (dspr2_t *)precache_model;
								if (LittleLong (sp2header->version) != SP2_VERSION)
								{	// not a recognized sprite
									FS_FreeFile(precache_model);
									precache_model = 0;
									precache_check++;
									precache_model_skin = 0;
									continue; // couldn't load it
								}
							}
						}
						else
						{	// get md3 header
							md3header = (dmd3_t *)precache_model;
							if (LittleLong (md3header->version) != MD3_ALIAS_VERSION)
							{	// not a recognized md3
								FS_FreeFile(precache_model);
								precache_model = 0;
								precache_check++;
								precache_model_skin = 0;
								continue; // couldn't load it
							}
						}
					}
					else
					{	// get md2 header
						md2header = (dmd2_t *)precache_model;
						if (LittleLong (md2header->version) != MD2_ALIAS_VERSION)
						{	// not a recognized md2
							FS_FreeFile(precache_model);
							precache_model = 0;
							precache_check++;
							precache_model_skin = 0;
							continue; // couldn't load it
						}
					}
				}

				if (LittleLong(*(unsigned *)precache_model) == IDMD2HEADER) // md2
				{
					md2header = (dmd2_t *)precache_model;
					while (precache_model_skin - 1 < LittleLong(md2header->num_skins))
					{
						skinname = (char *)precache_model + LittleLong(md2header->ofs_skins) + 
									(precache_model_skin - 1)*MD2_MAX_SKINNAME;

						// r1ch: spam warning for models that are broken
						if (strchr (skinname, '\\'))
							Com_Printf ("Warning, model %s with incorrectly linked skin: %s\n", cl.configstrings[precache_check], skinname);
						else if (strlen(skinname) > MD2_MAX_SKINNAME-1)
							Com_Error (ERR_DROP, "Model %s has too long a skin path: %s", cl.configstrings[precache_check], skinname);

						if (!CL_CheckOrDownloadFile(skinname))
						{
							precache_model_skin++;
							return; // started a download
						}
						precache_model_skin++;
					}
				}
				else if (LittleLong(*(unsigned *)precache_model) == IDMD3HEADER) // md3
				{
					md3header = (dmd3_t *)precache_model;
					while (precache_model_skin - 1 < LittleLong(md3header->num_skins))
					{
						int	i;
						md3mesh = (dmd3mesh_t *)((byte *)md3header + LittleLong(md3header->ofs_meshes));
						for ( i = 0; i < md3header->num_meshes; i++)
						{
							if (precache_model_skin - 1 >= LittleLong(md3header->num_skins))
								break;
							skinname = (char *)precache_model + LittleLong(md3mesh->ofs_skins) + 
										(precache_model_skin - 1)*MD3_MAX_PATH;

							// r1ch: spam warning for models that are broken
							if (strchr (skinname, '\\'))
								Com_Printf ("Warning, model %s with incorrectly linked skin: %s\n", cl.configstrings[precache_check], skinname);
							else if (strlen(skinname) > MD3_MAX_PATH-1)
								Com_Error (ERR_DROP, "Model %s has too long a skin path: %s", cl.configstrings[precache_check], skinname);

							if (!CL_CheckOrDownloadFile(skinname))
							{
								precache_model_skin++;
								return; // started a download
							}
							precache_model_skin++;

							md3mesh = (dmd3mesh_t *)((byte *)md3mesh + LittleLong (md3mesh->meshsize));
						}
					}
				}
				else	// if (LittleLong(*(unsigned *)precache_model) == IDSP2HEADER)	// sp2
				{
					sp2header = (dspr2_t *)precache_model;
					while (precache_model_skin - 1 < LittleLong(sp2header->numframes))
					{
						skinname = sp2header->frames[(precache_model_skin - 1)].name;

						// r1ch: spam warning for models that are broken
						if (strchr (skinname, '\\'))
							Com_Printf ("Warning, sprite %s with incorrectly linked skin: %s\n", cl.configstrings[precache_check], skinname);
						else if (strlen(skinname) > MD2_MAX_SKINNAME-1)
							Com_Error (ERR_DROP, "Sprite %s has too long a skin path: %s", cl.configstrings[precache_check], skinname);

						if (!CL_CheckOrDownloadFile(skinname))
						{
							precache_model_skin++;
							return; // started a download
						}
						precache_model_skin++;
					}
				}

				if (precache_model) { 
					FS_FreeFile(precache_model);
					precache_model = 0;
				}
				precache_model_skin = 0;
				precache_check++;
			}
		}
		precache_check = cs_sounds;
	}
	if (precache_check >= cs_sounds && precache_check < cs_sounds+max_sounds)
	{ 
		if (allow_download_sounds->integer) {
			if (precache_check == cs_sounds)
				precache_check++; // zero is blank
			while (precache_check < cs_sounds+max_sounds &&
				cl.configstrings[precache_check][0]) {
				if (cl.configstrings[precache_check][0] == '*') {
					precache_check++;
					continue;
				}
				Com_sprintf(fn, sizeof(fn), "sound/%s", cl.configstrings[precache_check++]);
				if (!CL_CheckOrDownloadFile(fn))
					return; // started a download
			}
		}
		precache_check = cs_images;
	}
	if (precache_check >= cs_images && precache_check < CS_IMAGES+max_images)
	{
		if (precache_check == cs_images)
			precache_check++; // zero is blank
		while (precache_check < cs_images+max_images &&
			cl.configstrings[precache_check][0]) {
			Com_sprintf(fn, sizeof(fn), "pics/%s.pcx", cl.configstrings[precache_check++]);
			if (!CL_CheckOrDownloadFile(fn))
				return; // started a download
		}
		precache_check = cs_playerskins;
	}
	// skins are special, since a player has three things to download:
	// model, weapon model and skin
	// so precache_check is now *3
	if (precache_check >= cs_playerskins && precache_check < cs_playerskins + MAX_CLIENTS * PLAYER_MULT)
	{
		if (allow_download_players->integer)
		{
			while (precache_check < cs_playerskins + MAX_CLIENTS * PLAYER_MULT)
			{
				int			i, j, n, len;
				char		model[MAX_QPATH], skin[MAX_QPATH], *p;
				qboolean	badSkinName = false;

				i = (precache_check - cs_playerskins)/PLAYER_MULT;
				n = (precache_check - cs_playerskins)%PLAYER_MULT;

				// from R1Q2- skip invalid player skins data
				if (i >= cl.maxclients) {
					precache_check = env_cnt;
					continue;
				}

				if (!cl.configstrings[cs_playerskins+i][0]) {
					precache_check = cs_playerskins + (i + 1) * PLAYER_MULT;
					continue;
				}

				if ((p = strchr(cl.configstrings[cs_playerskins+i], '\\')) != NULL) {
					p++;
				}
				else {
					p = cl.configstrings[cs_playerskins+i];
				}

			//	strncpy(model, p);
				Q_strncpyz (model, sizeof(model), p);
				p = strchr(model, '/');
				if (!p)
					p = strchr(model, '\\');

				if (p)
				{
					*p++ = 0;
					if (!p[0] || !model[0]) {
						precache_check = cs_playerskins + (i + 1) * PLAYER_MULT;
						continue;
					}
					else {
					//	strncpy(skin, p);
						Q_strncpyz (skin, sizeof(skin), p);
					}
				}
				else {
				//	*skin = 0;
					precache_check = cs_playerskins + (i + 1) * PLAYER_MULT;
					continue;
				}

				// from R1Q2: check model and skin name for invalid chars
				len = (int)strlen(model);
				for (j = 0; j < len; j++)
				{
					if ( !IsValidChar(model[j]) ) {
						Com_Printf (S_COLOR_YELLOW"Bad character '%c' in player model '%s'\n", model[j], model);
						badSkinName = true;
					}
				}

				len = (int)strlen(skin);
				for (j = 0; j < len; j++)
				{
					if ( !IsValidChar(skin[j]) ) {
						Com_Printf (S_COLOR_YELLOW"Bad character '%c' in player skin '%s'\n", skin[j], skin);
						badSkinName = true;
					}
				}
				if (badSkinName) {
					precache_check = cs_playerskins + (i + 1) * PLAYER_MULT;
					continue;
				}
				// end R1Q2 name check

				switch (n)
				{
				case 0: // model
					Com_sprintf(fn, sizeof(fn), "players/%s/tris.md2", model);
					if (!CL_CheckOrDownloadFile(fn)) {
						precache_check = cs_playerskins + i * PLAYER_MULT + 1;
						return; // started a download
					}
					n++;
					/*FALL THROUGH*/

				case 1: // weapon model
					Com_sprintf(fn, sizeof(fn), "players/%s/weapon.md2", model);
					if (!CL_CheckOrDownloadFile(fn)) {
						precache_check = cs_playerskins + i * PLAYER_MULT + 2;
						return; // started a download
					}
					n++;
					/*FALL THROUGH*/

				case 2: // weapon skin
					Com_sprintf(fn, sizeof(fn), "players/%s/weapon.pcx", model);
					if (!CL_CheckOrDownloadFile(fn)) {
						precache_check = cs_playerskins + i * PLAYER_MULT + 3;
						return; // started a download
					}
					n++;
					/*FALL THROUGH*/

				case 3: // skin
					Com_sprintf(fn, sizeof(fn), "players/%s/%s.pcx", model, skin);
					if (!CL_CheckOrDownloadFile(fn)) {
						precache_check = cs_playerskins + i * PLAYER_MULT + 4;
						return; // started a download
					}
					n++;
					/*FALL THROUGH*/

				case 4: // skin_i
					Com_sprintf(fn, sizeof(fn), "players/%s/%s_i.pcx", model, skin);
					if (!CL_CheckOrDownloadFile(fn)) {
						precache_check = cs_playerskins + i * PLAYER_MULT + 5;
						return; // started a download
					}
				//	n = 5;	// from R1Q2

					// move on to next model
				//	precache_check = cs_playerskins + (i + 1) * PLAYER_MULT;
				}
				// move on to next model
				precache_check = cs_playerskins + (i + 1) * PLAYER_MULT;
			}
		}
		// precache phase completed
		precache_check = env_cnt;
	}

#ifdef USE_CURL	// HTTP downloading from R1Q2
	// pending downloads (possibly the map), let's wait here.
	if ( CL_PendingHTTPDownloads() ) {
		return;
	}
	// YQ2 UDP fallback addition
	if ( CL_CheckHTTPError() && cl_http_fallback->integer )	// Download errors detected, so start over with next iteration
	{
		precache_iteration++;
		if (precache_iteration == ITER_HTTP_Q2PRO) {
			Com_Printf ("[HTTP] One or more HTTP downloads failed on R1Q2 path, falling back to Q2Pro path.\n");
			// try filelist again with a different path
			filelistUseGamedir = true;
			CL_HTTP_EnableGenericFilelist ();
		}
		else	// if (precache_iteration == ITER_UDP)
			Com_Printf ("[HTTP] One or more HTTP downloads failed on both R1Q2 and Q2Pro paths, falling back to UDP.\n");
		CL_ResetPrecacheCheck ();

		CL_RequestNextDownload ();
		return;
	}
#endif	// USE_CURL

	if (precache_check == env_cnt)
	{
		if ( Com_ServerState() )	// if on local server, skip checking textures
			precache_check = texture_cnt+999;
		else
			precache_check = env_cnt + 1;

		CM_LoadMap (cl.configstrings[CS_MODELS+1], true, &map_checksum);

		if (map_checksum != atoi(cl.configstrings[CS_MAPCHECKSUM])) {
			Com_Error (ERR_DROP, "Local map version differs from server: %i != '%s'\n",
				map_checksum, cl.configstrings[CS_MAPCHECKSUM]);
			return;
		}
	}

	if (precache_check > env_cnt && precache_check < texture_cnt)
	{
		if (allow_download->integer && allow_download_maps->integer)
		{
			while (precache_check < texture_cnt)
			{
				int n = precache_check++ - env_cnt - 1;

				if (n & 1)
					Com_sprintf(fn, sizeof(fn), "env/%s%s.pcx", 
						cl.configstrings[CS_SKY], env_suf[n/2]);
				else
					Com_sprintf(fn, sizeof(fn), "env/%s%s.tga", 
						cl.configstrings[CS_SKY], env_suf[n/2]);
				if (!CL_CheckOrDownloadFile(fn))
					return; // started a download
			}
		}
		precache_check = texture_cnt;
	}

	if (precache_check == texture_cnt) {
		precache_check = texture_cnt+1;
		precache_tex = 0;
	}

	// confirm existance of .wal textures, download any that don't exist
	if (precache_check == texture_cnt+1)
	{	// from qcommon/cmodel.c
		extern int			numtexinfo;
		extern mapsurface_t	map_surfaces[];

		if (allow_download->integer && allow_download_maps->integer)
		{
			while (precache_tex < numtexinfo)
			{
				char fn[MAX_OSPATH];

				Com_sprintf(fn, sizeof(fn), "textures/%s.wal", map_surfaces[precache_tex++].rname);
				if (!CL_CheckOrDownloadFile(fn))
					return; // started a download
			}
		}
	//	precache_check = texture_cnt+999;
		precache_check = texture_cnt+2;
		precache_tex = 0;
	}

	// confirm existance of .tga textures, try to download any that don't exist
	if (precache_check == texture_cnt+2)
	{	// from qcommon/cmodel.c
		extern int			numtexinfo;
		extern mapsurface_t	map_surfaces[];

		if (allow_download->integer && allow_download_maps->integer
			&& allow_download_textures_24bit->integer)
		{
			while (precache_tex < numtexinfo)
			{
				char fn[MAX_OSPATH];

				Com_sprintf(fn, sizeof(fn), "textures/%s.tga", map_surfaces[precache_tex++].rname);
				if (!CL_CheckOrDownloadFile(fn))
					return; // started a download
			}
		}
		precache_check = texture_cnt+3;
		precache_tex = 0;
	}

#ifdef PNG_SUPPORT
	// confirm existance of .png textures, try to download any that don't exist
	if (precache_check == texture_cnt+3)
	{	// from qcommon/cmodel.c
		extern int			numtexinfo;
		extern mapsurface_t	map_surfaces[];

		if (allow_download->integer && allow_download_maps->integer
			&& allow_download_textures_24bit->integer)
		{
			while (precache_tex < numtexinfo)
			{
				char fn[MAX_OSPATH];

				Com_sprintf(fn, sizeof(fn), "textures/%s.png", map_surfaces[precache_tex++].rname);
				if (!CL_CheckOrDownloadFile(fn))
					return; // started a download
			}
		}
		precache_check = texture_cnt+4;
		precache_tex = 0;
	}

	// confirm existance of .jpg textures, try to download any that don't exist
	if (precache_check == texture_cnt+4)
#else	// PNG_SUPPORT
	// confirm existance of .jpg textures, try to download any that don't exist
	if (precache_check == texture_cnt+3)
#endif	// PNG_SUPPORT
	{	// from qcommon/cmodel.c
		extern int			numtexinfo;
		extern mapsurface_t	map_surfaces[];

		if (allow_download->integer && allow_download_maps->integer
			&& allow_download_textures_24bit->integer)
		{
			while (precache_tex < numtexinfo)
			{
				char fn[MAX_OSPATH];

				Com_sprintf(fn, sizeof(fn), "textures/%s.jpg", map_surfaces[precache_tex++].rname);
				if (!CL_CheckOrDownloadFile(fn))
					return; // started a download
			}
		}
		precache_check = texture_cnt+999;
	}
// ZOID

#ifdef USE_CURL	// HTTP downloading from R1Q2
	// pending downloads (possibly textures), let's wait here.
	if ( CL_PendingHTTPDownloads() ) {
		return;
	}

	// This map is done downloading, reset HTTP for next map
	precache_forceUDP = false;
	filelistUseGamedir = false;
	precache_iteration = ITER_HTTP_R1Q2;

	CL_HTTP_EnableGenericFilelist ();	// re-enable generic filelists for next map
#endif	// USE_CURL

	CL_RegisterSounds ();
	CL_PrepRefresh ();

	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	MSG_WriteString (&cls.netchan.message, va("begin %i\n", precache_spawncount) );
	cls.forcePacket = true;
}


//=============================================================================

/*
===============
CL_DownloadFileName
===============
*/
void CL_DownloadFileName (char *dest, int destlen, const char *fn)
{
	if ( !stricmp(FS_Downloaddir(), FS_Gamedir()) )	// use basedir/gamedir if fs_downloaddir is the same as fs_gamedir
	{
		if (strncmp(fn, "players", 7) == 0)
			Com_sprintf (dest, destlen, "%s/%s", BASEDIRNAME, fn);
		else
			Com_sprintf (dest, destlen, "%s/%s", FS_Gamedir(), fn);	// was FS_Gamedir()
	}
	else
		Com_sprintf (dest, destlen, "%s/%s", FS_Downloaddir(), fn);
}


// Knightmare- store the names of last downloads that failed
#define NUM_FAIL_DLDS 64
char lastFailedDownload[NUM_FAIL_DLDS][MAX_OSPATH];
static unsigned failedDlListIndex;

/*
===============
CL_InitFailedDownloadList
===============
*/
void CL_InitFailedDownloadList (void)
{
	int		i;

	for (i=0; i<NUM_FAIL_DLDS; i++)
		Com_sprintf(lastFailedDownload[i], sizeof(lastFailedDownload[i]), "\0");

	failedDlListIndex = 0;
}


/*
===============
CL_CheckDownloadFailed
===============
*/
qboolean CL_CheckDownloadFailed (const char *name)
{
	int		i;

	for (i=0; i<NUM_FAIL_DLDS; i++)
		if ( (strlen(lastFailedDownload[i]) > 0) && !strcmp(name, lastFailedDownload[i]) )
		{	// we already tried downlaoding this, server didn't have it
			return true;
		}

	return false;
}


/*
===============
CL_AddToFailedDownloadList
===============
*/
void CL_AddToFailedDownloadList (const char *name)
{
	int			i;
	qboolean	found = false;

	// check if this name is already in the table
	for (i=0; i<NUM_FAIL_DLDS; i++)
		if ( (strlen(lastFailedDownload[i]) > 0) && !strcmp(name, lastFailedDownload[i]) )
		{
			found = true;
			break;
		}

	// if it isn't already in the table, then we need to add it
	if (!found)
	{
		Com_sprintf(lastFailedDownload[failedDlListIndex], sizeof(lastFailedDownload[failedDlListIndex]), "%s", name);
		failedDlListIndex++;

		// wrap around to start of list
		if (failedDlListIndex >= NUM_FAIL_DLDS)
			failedDlListIndex = 0;
	}	
}


/*
===============
CL_CheckOrDownloadFile

Returns true if the file exists, otherwise it attempts
to start a download from the server.
===============
*/
qboolean CL_CheckOrDownloadFile (const char *filename)
{
	FILE	*fp;
	char	name[MAX_OSPATH];
	int		len; // Knightmare added
	char	s[128];
	//int	i;

	if (strstr (filename, ".."))
	{
		Com_Printf ("Refusing to download a path with ..\n");
		return true;
	}

	if (FS_LoadFile (filename, NULL) != -1)
	{	// it exists, no need to download
		return true;
	}

	// don't try again to download a file that just failed
//	if (CL_CheckDownloadFailed(filename))
//		return true;

#ifdef PNG_SUPPORT
	// don't download a .png texture which already has a .tga counterpart
	len = (int)strlen(filename); 
//	strncpy(s, filename); 
	Q_strncpyz (s, sizeof(s), filename); 
	if (strstr(s, "textures/") && !strcmp(s+len-4, ".png")) // look if we have a .png texture 
	{ 
		s[len-3]='t'; s[len-2]='g'; s[len-1]='a'; // replace extension 
		if (FS_LoadFile (s, NULL) != -1)	// check for .tga counterpart
			return true;
	}
#endif	// PNG_SUPPORT

	// don't download a .jpg texture which already has a .tga or .png counterpart
	len = (int)strlen(filename); 
//	strncpy(s, filename); 
	Q_strncpyz (s, sizeof(s), filename); 
	if (strstr(s, "textures/") && !strcmp(s+len-4, ".jpg")) // look if we have a .jpg texture 
	{ 
		s[len-3]='t'; s[len-2]='g'; s[len-1]='a'; // replace extension 
		if (FS_LoadFile (s, NULL) != -1)	// check for .tga counterpart
			return true;
#ifdef PNG_SUPPORT
		s[len-3]='p'; s[len-2]='n'; s[len-1]='g'; // replace extension 
		if (FS_LoadFile (s, NULL) != -1)	// check for .png counterpart
			return true;
#endif	// PNG_SUPPORT
	}

#ifdef USE_CURL	// HTTP downloading from R1Q2
	if (!precache_forceUDP)	// YQ2 UDP fallback additon
	{
		if ( CL_QueueHTTPDownload(filename, filelistUseGamedir) )
		{
			// We return true so that the precache check keeps feeding us more files.
			// Since we have multiple HTTP connections we want to minimize latency
			// and be constantly sending requests, not one at a time.
			return true;
		}
	}
	else	// YQ2 UDP fallback additon
	{
	/*	There are 2 cases that get us here:
		1. forceUDP was set after a 404. In this case we want to retry that
			single file over UDP and all later files over HTTP.
		2. forceUDP was set after another error code.  In that case the HTTP
			code aborts all HTTP downloads and CL_QueueHTTPDownload() returns false.
	*/
		precache_forceUDP = false;
	/*	We might be connected to an r1q2-style HTTP server that missed just one file.
		So reset the precacher iteration counter to start over.
	*/
		precache_iteration = ITER_HTTP_R1Q2;
	}
//	else
//	{
#endif	// USE_CURL

	// don't try again to download a file that just failed
	if (CL_CheckDownloadFailed(filename))
		return true;

//	strncpy (cls.downloadname, filename);
	Q_strncpyz (cls.downloadname, sizeof(cls.downloadname), filename);

	// download to a temp name, and only rename
	// to the real name when done, so if interrupted
	// a runt file wont be left
	COM_StripExtension (cls.downloadname, cls.downloadtempname, sizeof(cls.downloadtempname));
//	strncat (cls.downloadtempname, ".tmp");
	Q_strncatz (cls.downloadtempname, sizeof(cls.downloadtempname), ".tmp");

//ZOID
	// check to see if we already have a tmp for this file, if so, try to resume
	// open the file if not opened yet
	CL_DownloadFileName (name, sizeof(name), cls.downloadtempname);

//	FS_CreatePath (name);

	fp = fopen (name, "r+b");
	if (fp)
	{	// it exists
		int len;
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);

		cls.download = fp;

		// give the server an offset to start the download
		Com_Printf ("Resuming %s\n", cls.downloadname);
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		MSG_WriteString (&cls.netchan.message,
			va("download %s %i", cls.downloadname, len));
	}
	else {
		Com_Printf ("Downloading %s\n", cls.downloadname);
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		MSG_WriteString (&cls.netchan.message, va("download %s", cls.downloadname));
	}

	cls.downloadnumber++;
	cls.forcePacket = true;

	return false;

#ifdef USE_CURL	// HTTP downloading from R1Q2
//	}
#endif	// USE_CURL
}


/*
===============
CL_Download_f

Request a download from the server
===============
*/
void CL_Download_f (void)
{
	char filename[MAX_OSPATH];

	if (Cmd_Argc() != 2) {
		Com_Printf("Usage: download <filename>\n");
		return;
	}

	Com_sprintf(filename, sizeof(filename), "%s", Cmd_Argv(1));

	if (strstr (filename, ".."))
	{
		Com_Printf ("Refusing to download a path with ..\n");
		return;
	}

	if (FS_LoadFile (filename, NULL) != -1)
	{	// it exists, no need to download
		Com_Printf("File already exists.\n");
		return;
	}

//	strncpy (cls.downloadname, filename);
	Q_strncpyz (cls.downloadname, sizeof(cls.downloadname), filename);
	Com_Printf ("Downloading %s\n", cls.downloadname);

	// download to a temp name, and only rename
	// to the real name when done, so if interrupted
	// a runt file wont be left
	COM_StripExtension (cls.downloadname, cls.downloadtempname, sizeof(cls.downloadtempname));
//	strncat (cls.downloadtempname, ".tmp");
	Q_strncatz (cls.downloadtempname, sizeof(cls.downloadtempname), ".tmp");

	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	MSG_WriteString (&cls.netchan.message,
		va("download %s", cls.downloadname));

	cls.downloadnumber++;
}

//=============================================================================

/*
=====================
CL_ParseDownload

A download message has been received from the server
=====================
*/
void CL_ParseDownload (void)
{
	int		size, percent;
	char	name[MAX_OSPATH];
	int		r;	// i

	// read the data
	size = MSG_ReadShort (&net_message);
	percent = MSG_ReadByte (&net_message);
	if (size == -1)
	{
		Com_Printf ("Server does not have this file.\n");

		if (cls.downloadname)	// Knightmare- save name of failed download
			CL_AddToFailedDownloadList (cls.downloadname);

		if (cls.download)
		{
			// if here, we tried to resume a file but the server said no
			fclose (cls.download);
			cls.download = NULL;
		}
		CL_RequestNextDownload ();
		return;
	}

	// open the file if not opened yet
	if (!cls.download)
	{
		CL_Download_Reset_KBps_counter ();	// Knightmare- for KB/s counter

		CL_DownloadFileName (name, sizeof(name), cls.downloadtempname);

		FS_CreatePath (name);

		cls.download = fopen (name, "wb");
		if (!cls.download)
		{
			net_message.readcount += size;
			Com_Printf ("Failed to open %s\n", cls.downloadtempname);
			CL_RequestNextDownload ();
			return;
		}
	}

	fwrite (net_message.data + net_message.readcount, 1, size, cls.download);
	net_message.readcount += size;

	if (percent != 100)
	{
		// request next block
// change display routines by zoid
#if 0
		Com_Printf (".");
		if (10*(percent/10) != cls.downloadpercent)
		{
			cls.downloadpercent = 10*(percent/10);
			Com_Printf ("%i%%", cls.downloadpercent);
		}
#endif
		CL_Download_Calculate_KBps (size, 0);	// Knightmare- for KB/s counter
		cls.downloadpercent = percent;

		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		SZ_Print (&cls.netchan.message, "nextdl");
		cls.forcePacket = true;
	}
	else
	{
		char	oldn[MAX_OSPATH];
		char	newn[MAX_OSPATH];

	//	Com_Printf ("100%%\n");

		fclose (cls.download);

		// rename the temp file to it's final name
		CL_DownloadFileName (oldn, sizeof(oldn), cls.downloadtempname);
		CL_DownloadFileName (newn, sizeof(newn), cls.downloadname);
		r = rename (oldn, newn);
		if (r)
			Com_Printf ("failed to rename.\n");

		cls.download = NULL;
		cls.downloadpercent = 0;

		// add new pk3s to search paths, hack by Jay Dolan
		if (strstr(newn, ".pk3")) 
			FS_AddPK3File (newn, false);

		// get another file if needed

		CL_RequestNextDownload ();
	}
}

//=============================================================================

// Download speed counter

typedef struct {
	int		prevTime;
	int		bytesRead;
	int		byteCount;
	float	timeCount;
	float	prevTimeCount;
	float	startTime;
} dlSpeedInfo_t;

dlSpeedInfo_t	dlSpeedInfo;

/*
=====================
CL_Download_Reset_KBps_counter
=====================
*/
void CL_Download_Reset_KBps_counter (void)
{
	dlSpeedInfo.timeCount = dlSpeedInfo.prevTime = dlSpeedInfo.prevTimeCount = dlSpeedInfo.bytesRead = dlSpeedInfo.byteCount = 0;
	dlSpeedInfo.startTime = (float)cls.realtime;
	cls.downloadrate = 0;
}

/*
=====================
CL_Download_Calculate_KBps
=====================
*/
void CL_Download_Calculate_KBps (int byteDistance, int totalSize)
{
	float	timeDistance = (float)(cls.realtime - dlSpeedInfo.prevTime);
	float	totalTime = (dlSpeedInfo.timeCount - dlSpeedInfo.startTime) / 1000.0f;

	dlSpeedInfo.timeCount += timeDistance;
	dlSpeedInfo.byteCount += byteDistance;
	dlSpeedInfo.bytesRead += byteDistance;

	if (totalTime >= 1.0f)
	{
		cls.downloadrate = (float)dlSpeedInfo.byteCount / 1024.0f;
		Com_DPrintf ("Rate: %4.2fKB/s, Downloaded %4.2fKB of %4.2fKB\n", cls.downloadrate, (float)dlSpeedInfo.bytesRead/1024.0, (float)totalSize/1024.0);
		dlSpeedInfo.byteCount = 0;
		dlSpeedInfo.startTime = (float)cls.realtime;
	}
	dlSpeedInfo.prevTime = cls.realtime;
}
