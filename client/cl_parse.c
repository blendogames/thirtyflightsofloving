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

// cl_parse.c  -- parse a message received from the server

#include "client.h"

char *svc_strings[256] =
{
	"svc_bad",

	"svc_muzzleflash",
	"svc_muzzlflash2",
	"svc_temp_entity",
	"svc_layout",
	"svc_inventory",

	"svc_nop",
	"svc_disconnect",
	"svc_reconnect",
	"svc_sound",
	"svc_print",
	"svc_stufftext",
	"svc_serverdata",
	"svc_configstring",
	"svc_spawnbaseline",	
	"svc_centerprint",
	"svc_download",
	"svc_playerinfo",
	"svc_packetentities",
	"svc_deltapacketentities",
	"svc_frame",
	"svc_lazarus_fog"			// Knightmare added
};

//=============================================================================

/*
======================
CL_RegisterSounds
======================
*/
void CL_RegisterSounds (void)
{
	int		i, cs_sounds, max_sounds;

	// Knightmare- hack for connected to server using old protocol
	// Changed config strings require different parsing
	if ( LegacyProtocol() ) {
		max_sounds = OLD_MAX_SOUNDS;
		cs_sounds = OLD_CS_SOUNDS;
	}
	else {
		max_sounds = MAX_SOUNDS;
		cs_sounds = CS_SOUNDS;
	}

	S_BeginRegistration ();
	CL_RegisterTEntSounds ();

	for (i=1; i < max_sounds; i++)
	{
		if (!cl.configstrings[cs_sounds+i][0])
			break;
		cl.sound_precache[i] = S_RegisterSound (cl.configstrings[cs_sounds+i]);
		Sys_SendKeyEvents ();	// pump message loop
	}

	S_EndRegistration ();
}

/*
=====================================================================

  SERVER CONNECTING MESSAGES

=====================================================================
*/

/*
==================
CL_ParseServerData
==================
*/
void CL_ParseServerData (void)
{
	extern cvar_t	*fs_gamedirvar;
	char	*str;
	int		i;
	
	Com_DPrintf ("Serverdata packet received.\n");
//
// wipe the client_state_t struct
//
	CL_ClearState ();
	cls.state = ca_connected;

// parse protocol version number
	i = MSG_ReadLong (&net_message);
	cls.serverProtocol = i;

	// BIG HACK to let demos from release work with the 3.0x patch!!!
	// Knightmare- also allow connectivity with servers using the old protocol
	if ( LegacyProtocol() ) { } // do nothing
	else if ( (i != PROTOCOL_VERSION) && (i != OLD_PROTOCOL_VERSION) ) {
		Com_Error (ERR_DROP, "Server returned version %i, not %i or %i", i, PROTOCOL_VERSION, OLD_PROTOCOL_VERSION);
	}

	cl.servercount = MSG_ReadLong (&net_message);
	cl.attractloop = MSG_ReadByte (&net_message);

	// game directory
	str = MSG_ReadString (&net_message);
	strncpy (cl.gamedir, str, sizeof(cl.gamedir)-1);

	// set gamedir
	if ( ( (*str && (!fs_gamedirvar->string || !*fs_gamedirvar->string || strcmp(fs_gamedirvar->string, str)))
		|| (!*str && (fs_gamedirvar->string && *fs_gamedirvar->string)) ) // was fs_gamedirvar->string || *fs_gamedirvar->string
		&& !cl.attractloop ) // Knightmare- don't allow demos to change this
		Cvar_Set("game", str);

	// parse player entity number
	cl.playernum = MSG_ReadShort (&net_message);

	// get the full level name
	str = MSG_ReadString (&net_message);

	if (cl.playernum == -1)
	{	// playing a cinematic or showing a pic, not a level
		SCR_PlayCinematic (str);
	}
	else
	{
		// seperate the printfs so the server message can have a color
		Com_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
		con.ormask = 128;
		Com_Printf ("%c"S_COLOR_SHADOW S_COLOR_ALT"%s\n", 2, str);
		con.ormask = 0;

		// need to prep refresh at next oportunity
		cl.refresh_prepped = false;
	}
}

/*
==================
CL_ParseBaseline
==================
*/
void CL_ParseBaseline (void)
{
	centity_state_t	*es;
	int				bits, bits2;
	int				newnum;
	centity_state_t	nullstate;

	memset (&nullstate, 0, sizeof(nullstate));

	newnum = CL_ParseEntityBits (&bits, &bits2);
	es = &cl_entities[newnum].baseline;
	CL_ParseDelta (&nullstate, es, newnum, bits, bits2);
}


/*
================
CL_LoadClientinfo

================
*/
void CL_LoadClientinfo (clientinfo_t *ci, char *s)
{
	int i;
	char		*t;
	char		model_name[MAX_QPATH];
	char		skin_name[MAX_QPATH];
	char		model_filename[MAX_QPATH];
	char		skin_filename[MAX_QPATH];
	char		weapon_filename[MAX_QPATH];

	strncpy(ci->cinfo, s, sizeof(ci->cinfo));
	ci->cinfo[sizeof(ci->cinfo)-1] = 0;

	// isolate the player's name
	strncpy(ci->name, s, sizeof(ci->name));
	ci->name[sizeof(ci->name)-1] = 0;
	t = strstr (s, "\\");
	if (t)
	{
		ci->name[t-s] = 0;
		s = t+1;
	}

	if (cl_noskins->integer || *s == 0)
	{
		Com_sprintf (model_filename, sizeof(model_filename), "players/male/tris.md2");
		Com_sprintf (weapon_filename, sizeof(weapon_filename), "players/male/weapon.md2");
		Com_sprintf (skin_filename, sizeof(skin_filename), "players/male/grunt.pcx");
		Com_sprintf (ci->iconname, sizeof(ci->iconname), "/players/male/grunt_i.pcx");
		ci->model = R_RegisterModel (model_filename);
		memset(ci->weaponmodel, 0, sizeof(ci->weaponmodel));
		ci->weaponmodel[0] = R_RegisterModel (weapon_filename);
		ci->skin = R_RegisterSkin (skin_filename);
		ci->icon = R_DrawFindPic (ci->iconname);
	}
	else
	{
		// isolate the model name
	//	strncpy (model_name, s);
		Q_strncpyz (model_name, sizeof(model_name), s);
		t = strstr(model_name, "/");
		if (!t)
			t = strstr(model_name, "\\");
		if (!t)
			t = model_name;
		*t = 0;

		// isolate the skin name
//		strncpy (skin_name, s + strlen(model_name) + 1);
		Q_strncpyz (skin_name, sizeof(skin_name), s + strlen(model_name) + 1);

		// model file
		Com_sprintf (model_filename, sizeof(model_filename), "players/%s/tris.md2", model_name);
		ci->model = R_RegisterModel (model_filename);
		if (!ci->model)
		{
		//	strncpy(model_name, "male");
			Q_strncpyz(model_name, sizeof(model_name), "male");
			Com_sprintf (model_filename, sizeof(model_filename), "players/male/tris.md2");
			ci->model = R_RegisterModel (model_filename);
		}

		// skin file
		Com_sprintf (skin_filename, sizeof(skin_filename), "players/%s/%s.pcx", model_name, skin_name);
		ci->skin = R_RegisterSkin (skin_filename);

		// if we don't have the skin and the model wasn't male,
		// see if the male has it (this is for CTF's skins)
 		if (!ci->skin && Q_stricmp(model_name, "male"))
		{
			// change model to male
		//	strncpy(model_name, "male");
			Q_strncpyz(model_name, sizeof(model_name), "male");
			Com_sprintf (model_filename, sizeof(model_filename), "players/male/tris.md2");
			ci->model = R_RegisterModel (model_filename);

			// see if the skin exists for the male model
			Com_sprintf (skin_filename, sizeof(skin_filename), "players/%s/%s.pcx", model_name, skin_name);
			ci->skin = R_RegisterSkin (skin_filename);
		}

		// if we still don't have a skin, it means that the male model didn't have
		// it, so default to grunt
		if (!ci->skin) {
			// see if the skin exists for the male model
			Com_sprintf (skin_filename, sizeof(skin_filename), "players/%s/grunt.pcx", model_name, skin_name);
			ci->skin = R_RegisterSkin (skin_filename);
		}

		// weapon file
		for (i = 0; i < num_cl_weaponmodels; i++) {
			Com_sprintf (weapon_filename, sizeof(weapon_filename), "players/%s/%s", model_name, cl_weaponmodels[i]);
			ci->weaponmodel[i] = R_RegisterModel(weapon_filename);
			if (!ci->weaponmodel[i] && strcmp(model_name, "cyborg") == 0) {
				// try male
				Com_sprintf (weapon_filename, sizeof(weapon_filename), "players/male/%s", cl_weaponmodels[i]);
				ci->weaponmodel[i] = R_RegisterModel(weapon_filename);
			}
			if (!cl_vwep->integer)
				break; // only one when vwep is off
		}

		// icon file
		Com_sprintf (ci->iconname, sizeof(ci->iconname), "/players/%s/%s_i.pcx", model_name, skin_name);
		ci->icon = R_DrawFindPic (ci->iconname);
	}

	// must have loaded all data types to be valud
	if (!ci->skin || !ci->icon || !ci->model || !ci->weaponmodel[0])
	{
		ci->skin = NULL;
		ci->icon = NULL;
		ci->model = NULL;
		ci->weaponmodel[0] = NULL;
		return;
	}
}

/*
================
CL_ParseClientinfo

Load the skin, icon, and model for a client
================
*/
void CL_ParseClientinfo (int player)
{
	char			*s;
	clientinfo_t	*ci;

	// Knightmare- 1/2/2002- GROSS HACK for old demos or
	// connected to server using old protocol
	// Changed config strings require different offsets
	if ( LegacyProtocol() )
		s = cl.configstrings[player+OLD_CS_PLAYERSKINS];
	else
		s = cl.configstrings[player+CS_PLAYERSKINS];
	//end Knightmare

	ci = &cl.clientinfo[player];

	CL_LoadClientinfo (ci, s);
}


/*
================
CL_MissionPackCDTrack
Returns correct OGG track number for mission packs.
This assumes that the standard Q2 CD was ripped
as track02-track11, and the Rogue CD as track12-track21.
================
*/
int CL_MissionPackCDTrack (int tracknum)
{
	if ( FS_ModType("rogue") || cl_rogue_music->integer )
	{
		if ( (tracknum >= 2) && (tracknum <= 11) )
			return tracknum + 10;
		else
			return tracknum;
	}
	// an out-of-order mix from Q2 and Rogue CDs
	else if ( FS_ModType("xatrix") || cl_xatrix_music->integer )
	{
		switch (tracknum)
		{
			case 2: return 9;	break;
			case 3: return 13;	break;
			case 4: return 14;	break;
			case 5: return 7;	break;
			case 6: return 16;	break;
			case 7: return 2;	break;
			case 8: return 15;	break;
			case 9: return 3;	break;
			case 10: return 4;	break;
			case 11: return 18; break;
			default: return tracknum; break;
		}
	}
	else
		return tracknum;
}


/*
=================
CL_PlayBackgroundTrack
=================
*/
#ifdef OGG_SUPPORT

#include "snd_ogg.h"

void CL_PlayBackgroundTrack (void)
{
	char	name[MAX_QPATH], name2[MAX_QPATH];
	int		track;

#ifdef NOTTHIRTYFLIGHTS
	if (!cl.refresh_prepped)
		return;
#endif

	// using a named audio track intead of numbered
	if (strlen(cl.configstrings[CS_CDTRACK]) > 2)
	{
		Com_sprintf (name, sizeof(name), "music/%s.ogg", cl.configstrings[CS_CDTRACK]);
		Com_sprintf (name2, sizeof(name2), "music/%s.ogg_json", cl.configstrings[CS_CDTRACK]);	// Also check .ogg_json file
		if ( (FS_LoadFile(name, NULL) != -1) || (FS_LoadFile(name2, NULL) != -1) )
		{
#ifndef NOTTHIRTYFLIGHTS
			//BC add looping track.
			//check if the intro name exists. If not, then just use the looping track.
			char	introname[MAX_QPATH];
			sprintf(introname, va("music/%s_intro.ogg", cl.configstrings[CS_CDTRACK]) );
			if (FS_LoadFile(introname, NULL) <= -1)
				strcpy (introname, name);
#endif
			CDAudio_Stop ();
#ifdef NOTTHIRTYFLIGHTS
			S_StartBackgroundTrack(name, name);
#else
			S_StartBackgroundTrack(introname, name);
#endif
			return;
		}
	}

	track = atoi(cl.configstrings[CS_CDTRACK]);

	if (track == 0)
	{	// Stop any playing track
		CDAudio_Stop ();
		S_StopBackgroundTrack ();
		return;
	}

	// If an OGG file exists play it, otherwise fall back to CD audio
	Com_sprintf (name, sizeof(name), "music/track%02i.ogg", CL_MissionPackCDTrack(track));
	if ( (FS_LoadFile(name, NULL) != -1) && cl_ogg_music->integer )
		S_StartBackgroundTrack (name, name);
	else
		CDAudio_Play (track, true);
}

#else

void CL_PlayBackgroundTrack (void)
{
	CDAudio_Play (atoi(cl.configstrings[CS_CDTRACK]), true);
}

#endif // OGG_SUPPORT

/*
================
CL_ParseConfigString
================
*/
void CL_ParseConfigString (void)
{
	int		i;
	int		max_models, max_sounds, max_images, cs_lights, cs_sounds, cs_images, cs_playerskins;
	char	*s, *dest;
	char	olds[MAX_QPATH];
	size_t	length, maxLength;

	// Knightmare- hack for connected to server using old protocol
	// Changed config strings require different parsing
	if ( LegacyProtocol() )
	{
		max_models = OLD_MAX_MODELS;
		max_sounds = OLD_MAX_SOUNDS;
		max_images = OLD_MAX_IMAGES;
		cs_lights = OLD_CS_LIGHTS;
		cs_sounds = OLD_CS_SOUNDS;
		cs_images = OLD_CS_IMAGES;
		cs_playerskins = OLD_CS_PLAYERSKINS;
	}
	else
	{
		max_models = MAX_MODELS;
		max_sounds = MAX_SOUNDS;
		max_images = MAX_IMAGES;
		cs_lights = CS_LIGHTS;
		cs_sounds = CS_SOUNDS;
		cs_images = CS_IMAGES;
		cs_playerskins = CS_PLAYERSKINS;
	}

	i = MSG_ReadShort (&net_message);
	if (i < 0 || i >= MAX_CONFIGSTRINGS)
		Com_Error (ERR_DROP, "configstring > MAX_CONFIGSTRINGS");
	s = MSG_ReadString(&net_message);

	Q_strncpyz (olds, sizeof(olds), cl.configstrings[i]);

	// check length
	length = strlen(s);
	if ( length >= (sizeof(cl.configstrings[0]) * (MAX_CONFIGSTRINGS - i)) - 1 )
		Com_Error (ERR_DROP, "CL_ParseConfigString: string %d exceeds available buffer space!", i);

/*	if ( (i >= CS_STATUSBAR) && (i < CS_AIRACCEL) ) {	// allow writes to statusbar strings to overflow
		strncpy (cl.configstrings[i], s, (sizeof(cl.configstrings[i]) * (CS_AIRACCEL - i))-1 );
		cl.configstrings[CS_AIRACCEL-1][MAX_QPATH-1] = 0;	// null terminate end of section
	}
	else {
		if (length >= MAX_QPATH)
			Com_Printf(S_COLOR_YELLOW"CL_ParseConfigString: string %d of length %d exceeds MAX_QPATH.\n", i, (int)length);
		Q_strncpyz (cl.configstrings[i], sizeof(cl.configstrings[i]), s);
	} */

	// Allow writes to statusbar strings to overflow
	if ( (i >= CS_STATUSBAR) && (i < CS_AIRACCEL) ) {
		maxLength =  MAX_QPATH * (CS_AIRACCEL - i);
		if (length >= maxLength)
			Com_Printf(S_COLOR_YELLOW"CL_ParseConfigString: string %d in CS_STATUSBAR of length %d exceeds %d.\n", i, (int)length, (int)maxLength);
	//	Com_Printf("CL_ParseConfigString: CS_STATUSBAR %i: '%s', maxlen=%i\n", i, s, (sizeof(cl.configstrings[i]) * (CS_AIRACCEL - i))-1);
	}
	// Allow writes to general strings to overflow
	else if ( LegacyProtocol() && ( (i >= OLD_CS_GENERAL) && (i < OLD_MAX_CONFIGSTRINGS) ) ) {
		maxLength =  MAX_QPATH * (OLD_MAX_CONFIGSTRINGS - i);
		if (length >= maxLength)
			Com_Printf(S_COLOR_YELLOW"CL_ParseConfigString: string %d in OLD_CS_GENERAL of length %d exceeds %d.\n", i, (int)length, (int)maxLength);
	//	Com_Printf("CL_ParseConfigString: CS_GENERAL %i: '%s', maxlen=%i\n", i, s, (sizeof(cl.configstrings[i]) * (OLD_MAX_CONFIGSTRINGS - i))-1);
	}
	else if ( !LegacyProtocol() && ( (i >= CS_GENERAL) && (i < CS_HUDVARIANT) ) ) {
		maxLength =  MAX_QPATH * (CS_HUDVARIANT - i);
		if (length >= maxLength)
			Com_Printf(S_COLOR_YELLOW"CL_ParseConfigString: string %d in CS_GENERAL of length %d exceeds %d.\n", i, (int)length, (int)maxLength);
	//	Com_Printf("CL_ParseConfigString: CS_GENERAL %i: '%s', maxlen=%i\n", i, s, (sizeof(cl.configstrings[i]) * (CS_HUDVARIANT - i))-1);	
	}
	else {
		maxLength = MAX_QPATH;
		if (length >= maxLength)
			Com_Printf(S_COLOR_YELLOW"CL_ParseConfigString: string %d of length %d exceeds %d.\n", i, (int)length, (int)maxLength);
	}

	length = min(length, (maxLength - 1));
	dest = cl.configstrings[i];
	memcpy(dest, s, length);
	dest[length] = 0;	// null terminate string

	// do something apropriate 
	if ( (i >= cs_lights) && (i < cs_lights+MAX_LIGHTSTYLES) )
		CL_SetLightstyle (i - cs_lights);
	else if (i == CS_CDTRACK)
	{
		if (cl.refresh_prepped)
			CL_PlayBackgroundTrack ();
	}
	else if (i == CS_MAXCLIENTS)	// from R1Q2
	{
		if (!cl.attractloop)
			cl.maxclients = atoi(cl.configstrings[CS_MAXCLIENTS]);
	}
	else if ( (i >= CS_MODELS) && (i < CS_MODELS+max_models) )
	{
		if (cl.refresh_prepped)
		{
			cl.model_draw[i-CS_MODELS] = R_RegisterModel (cl.configstrings[i]);
			if (cl.configstrings[i][0] == '*')
				cl.model_clip[i-CS_MODELS] = CM_InlineModel (cl.configstrings[i]);
			else
				cl.model_clip[i-CS_MODELS] = NULL;
		}
	}
	else if ( (i >= cs_sounds) && (i < cs_sounds+max_sounds) ) // Knightmare- was MAX_MODELS
	{
		if (cl.refresh_prepped)
			cl.sound_precache[i-cs_sounds] = S_RegisterSound (cl.configstrings[i]);
	}
	else if ( (i >= cs_images) && (i < cs_images+max_images) ) // Knightmare- was MAX_IMAGES
	{
		if (cl.refresh_prepped)
			cl.image_precache[i-cs_images] = R_DrawFindPic (cl.configstrings[i]);
	}
	else if ( (i >= cs_playerskins) && (i < cs_playerskins+MAX_CLIENTS) )
	{
		// from R1Q2- a hack to avoid parsing non-skins from mods that overload CS_PLAYERSKINS
		if ( (i-cs_playerskins) < cl.maxclients ) {
			if (cl.refresh_prepped && strcmp(olds, s))
				CL_ParseClientinfo (i-cs_playerskins);
		}
		else {
			Com_DPrintf ("CL_ParseConfigString: Ignoring out-of-range playerskin %d (%s)\n", i, MakePrintable(s, 0));
		}
	}
	else if (i == CS_HUDVARIANT)	// update cl_hudVariant when CS_HUDVARIANT changes
	{
		CL_SetHudVariant ();
	}
}


/*
=====================================================================

ACTION MESSAGES

=====================================================================
*/

/*
==================
CL_ParseStartSoundPacket
==================
*/
void CL_ParseStartSoundPacket (void)
{
    vec3_t  pos_v;
	float	*pos;
    int 	channel, ent;
    int 	sound_num;
    float 	volume;
    float 	attenuation;  
	int		flags;
	float	ofs;

	flags = MSG_ReadByte (&net_message);

	// Knightmare- 12/23/2001
	// read sound indices as bytes only if playing old demos or
	// connected to server using old protocol; otherwise, read as shorts
	if ( LegacyProtocol() )
		sound_num = MSG_ReadByte (&net_message);
	else
		sound_num = MSG_ReadShort (&net_message);
	// end Knightmare

    if (flags & SND_VOLUME)
		volume = MSG_ReadByte (&net_message) / 255.0;
	else
		volume = DEFAULT_SOUND_PACKET_VOLUME;
	
    if (flags & SND_ATTENUATION)
		attenuation = MSG_ReadByte (&net_message) / 64.0;
	else
		attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;	

    if (flags & SND_OFFSET)
		ofs = MSG_ReadByte (&net_message) / 1000.0;
	else
		ofs = 0;

	if (flags & SND_ENT)	// entity relative
	{
		// Knightmare 8/2/21- read channel and ent as a combined short only if playing old demos or
		// connected to server using old protocol; otherwise, read as a byte and a short
		if ( LegacyProtocol() )
		{
			channel = MSG_ReadShort(&net_message); 
			ent = channel >> 3;
			channel &= 7;
		}
		else
		{
			channel = MSG_ReadByte(&net_message); 
			ent = (unsigned short)MSG_ReadShort(&net_message); 	// make sure this doesn't turn negative!
		}
		if (ent > MAX_EDICTS)
			Com_Error (ERR_DROP,"CL_ParseStartSoundPacket: ent = %i", ent);
		// end Knightmare
	}
	else
	{
		ent = 0;
		channel = 0;
	}

	if (flags & SND_POS)
	{	// positioned in space
		MSG_ReadPos (&net_message, pos_v);
 
		pos = pos_v;
	}
	else	// use entity number
		pos = NULL;

	if (!cl.sound_precache[sound_num])
		return;

	S_StartSound (pos, ent, channel, cl.sound_precache[sound_num], volume, attenuation, ofs);
}       


/*
==================
CL_ParsePrint
==================
*/
void CL_ParsePrint (void)
{
	int		i;
	char	*s;

	i = MSG_ReadByte (&net_message);
	s = MSG_ReadString (&net_message);

	// Chat Ignore from R1Q2/Q2Pro
	// Also filter private messages
	if ( (i == PRINT_CHAT) || (strstr(s, "(private message)") != NULL) ) {
		if ( CL_CheckForChatIgnore(s) )
			return;
	}

	if (i == PRINT_CHAT)
	{
		// Chat Ignore from R1Q2/Q2Pro
	//	if ( CL_CheckForChatIgnore(s) )
	//		return;

		S_StartLocalSound ("misc/talk.wav");
	//	con.ormask = 128;	// made redundant by color code

		// Set advertise timer for !version, from R1Q2
		if ( (strstr(s, "!kmq2_version") || strstr(s, "!version")) &&
			( (cls.lastAdvertiseTime == 0) || (cls.realtime > cls.lastAdvertiseTime + 300000) ) )
			cls.advertiseTime = cls.realtime + (int)(random() * 1500);

		Com_Printf (S_COLOR_ALT"%s", s); // add green flag
	}
	else
		Com_Printf ("%s", s);
	con.ormask = 0;
}


void SHOWNET (char *s)
{
	if (cl_shownet->integer >= 2)
		Com_Printf ("%3i:%s\n", net_message.readcount-1, s);
}


/*
=====================
CL_FilterStuffText
Catches malicious stuffed commands from the server.
Simply disconnects when the stuffed command is quit
or error, same effect as kicking the player.
Uses list of malicious commands from xian.
=====================
*/
qboolean CL_FilterStuffText (char *stufftext, size_t textSize)
{
	int			i, quotes, stuffLen, execLen, cmdLen, textLen;
	char		*parsetext = stufftext;
	char		*s, *execname, *p;
	char		*bad_stuffcmds[] =
	{
		"sensitivity",
		"unbindall",
		"unbind",
		"bind",
	//	"exec",
		"kill",
		"rate",
		"cl_maxfps",
		"r_maxfps",
		"net_maxfps",
	//	"quit",
	//	"error",
		0
	};

	// catch null pointer
	if ( !stufftext )
		return true;

	stuffLen = (int)strlen(stufftext);

	// nothing to filter?
	if (stuffLen == 0)
		return true;

	do
	{
		if ( ((parsetext - stufftext) >= (textSize-1)) || ((parsetext - stufftext) >= stuffLen) )
			break;

		// skip ;
		if (*parsetext == ';') parsetext++;

		if ( ((parsetext - stufftext) >= (textSize-1)) || ((parsetext - stufftext) >= stuffLen) )
			break;

		// skip leading spaces
		while ( (*parsetext == ' ') && ((parsetext - stufftext) < (textSize-1)) && ((parsetext - stufftext) < stuffLen) )
			parsetext++;

		if ( ((parsetext - stufftext) >= (textSize-1)) || ((parsetext - stufftext) >= stuffLen) )
			break;

		// handle quit and error stuffs specially
		if (!strncmp(parsetext, "quit", 4) || !strncmp(parsetext, "error", 5))
		{
			Com_Printf(S_COLOR_YELLOW"CL_FilterStuffText: Server stuffed 'quit' or 'error' command, disconnecting...\n");
			CL_Disconnect ();
			return false;
		}

		// don't allow stuffing of renderer cvars
		if ( !strncmp(parsetext, "gl_", 3) || !strncmp(parsetext, "r_", 2) )    	
			return false;

		// the Generations mod stuffs exec g*.cfg  for classes, so limit exec stuffs to .cfg files
		if ( !strncmp(parsetext, "exec", 4) )
		{
			s = parsetext;
			execname = COM_Parse (&s);
			if (!s) {
				Com_Printf(S_COLOR_YELLOW"CL_FilterStuffText: Server stuffed 'exec' command with no file\n");
				return false;	// catch case of no text after 'exec'
			}

			execname = COM_Parse (&s);
			execLen = (int)strlen(execname);

			if ( (execLen > 1) && (execname[execLen-1] == ';') )	// catch token ending with ;
				execLen--;

			if ( (execLen < 5) || (strncmp(execname+execLen-4, ".cfg", 4) != 0) ) {
				Com_Printf(S_COLOR_YELLOW"CL_FilterStuffText: Server stuffed 'exec' command for non-cfg file\n");
				return false;
			}
			return true;
		}

		// code by xian- cycle through list of malicious commands
		i = 0;
		while (bad_stuffcmds[i] != NULL)
		{
			cmdLen = (int)strlen(bad_stuffcmds[i]);
			if ( Q_strncmp(parsetext, bad_stuffcmds[i], cmdLen) == 0 )
				return false;
			i++;
		}

		// find a ; for next pass
		quotes = 0;
		textLen = (int)strlen(parsetext);
		p = NULL;
		for (i = 0; i < textLen; i++)
		{
			if (parsetext[i] == '"')
				quotes++;
			// don't break if in a quoted string
			if ( !(quotes & 1) && (parsetext[i] == ';') )
				p = &parsetext[i];
		}
		parsetext = p;
	} while (parsetext != NULL);

	return true;
}


// Knightmare- server-controlled fog
/*
=====================
CL_ParseLazarusFog
=====================
*/
// Fog is sent like this:
// gi.WriteByte (svc_lazarus_fog);	// svc_lazarus_fog = 21
// gi.WriteByte (fog_enable);		// 1 = on, 0 = off
// gi.WriteByte (fog_model);		// 0, 1, or 2
// gi.WriteByte (fog_density);		// 1-100
// gi.WriteShort (fog_near);		// >0, < fog_far
// gi.WriteShort (fog_far);			// >fog_near-64, < 10000
// gi.WriteByte (fog_red);			// 0-255
// gi.WriteByte (fog_green);		// 0-255
// gi.WriteByte (fog_blue);			// 0-255
// gi.unicast (player_ent, true); 

void CL_ParseLazarusFog (void)
{
	qboolean fogenable;
	int model, density, start, end,
			red, green, blue, temp;

	temp = MSG_ReadByte (&net_message);
	fogenable = (temp > 0) ? true:false;
	model = MSG_ReadByte (&net_message);
	density = MSG_ReadByte (&net_message);
	start = MSG_ReadShort (&net_message);
	end = MSG_ReadShort (&net_message);
	red = MSG_ReadByte (&net_message);
	green = MSG_ReadByte (&net_message);
	blue = MSG_ReadByte (&net_message);

	V_SetFogInfo (fogenable, model, density, start, end, red, green, blue);
}


/*
=====================
CL_ParseServerMessage
=====================
*/
void CL_ParseServerMessage (void)
{
	int			cmd;
	char		*s;
//	int			i;

//
// if recording demos, copy the message out
//
	if (cl_shownet->integer == 1)
		Com_Printf ("%i ",net_message.cursize);
	else if (cl_shownet->integer >= 2)
		Com_Printf ("------------------\n");


//
// parse the message
//
	while (1)
	{
		if (net_message.readcount > net_message.cursize)
		{
			Com_Error (ERR_DROP,"CL_ParseServerMessage: Bad server message");
			break;
		}

		cmd = MSG_ReadByte (&net_message);

		if (cmd == -1)
		{
			SHOWNET("END OF MESSAGE");
			break;
		}

		if (cl_shownet->integer >= 2)
		{
			if (!svc_strings[cmd])
				Com_Printf ("%3i:BAD CMD %i\n", net_message.readcount-1,cmd);
			else
				SHOWNET(svc_strings[cmd]);
		}
	
	// other commands
		switch (cmd)
		{
		default:
			Com_Error (ERR_DROP,"CL_ParseServerMessage: Illegible server message\n");
			break;
			
		case svc_nop:
//			Com_Printf ("svc_nop\n");
			break;
			
		case svc_disconnect:
			Com_Error (ERR_DISCONNECT,"Server disconnected\n");
			break;

		case svc_reconnect:
			Com_Printf ("Server disconnected, reconnecting\n");
			if (cls.download) {
				//ZOID, close download
				fclose (cls.download);
				cls.download = NULL;
			}
			cls.state = ca_connecting;
			cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately
			break;

		case svc_print:
		/*	i = MSG_ReadByte (&net_message);
			if (i == PRINT_CHAT)
			{
				S_StartLocalSound ("misc/talk.wav");
			//	con.ormask = 128;	// made redundant by color code
				Com_Printf (S_COLOR_ALT"%s", MSG_ReadString (&net_message)); // add green flag
			}
			else
				Com_Printf ("%s", MSG_ReadString (&net_message));
			con.ormask = 0;*/
			CL_ParsePrint ();
			break;
			
		case svc_centerprint:
			SCR_CenterPrint (MSG_ReadString (&net_message));
			break;
			
		case svc_stufftext:
			s = MSG_ReadString (&net_message);
			// Knightmare- filter malicious stufftext
			if ( !CL_FilterStuffText(s, MSG_STRING_SIZE) ) {
				Com_Printf(S_COLOR_YELLOW"CL_ParseServerMessage: Malicious stufftext from server:  %s\n", s);
				break;
			}
			Com_DPrintf ("stufftext: %s\n", s);
			Cbuf_AddText (s);
			break;
			
		case svc_serverdata:
			Cbuf_Execute ();		// make sure any stuffed commands are done
			CL_ParseServerData ();
			break;
			
		case svc_configstring:
			CL_ParseConfigString ();
			break;
			
		case svc_sound:
			CL_ParseStartSoundPacket();
			break;
			
		case svc_spawnbaseline:
			CL_ParseBaseline ();
			break;

		case svc_temp_entity:
			CL_ParseTEnt ();
			break;

		case svc_muzzleflash:
			CL_ParseMuzzleFlash ();
			break;

		case svc_muzzleflash2:
			CL_ParseMuzzleFlash2 ();
			break;

		case svc_download:
			CL_ParseDownload ();
			break;

		case svc_frame:
			CL_ParseFrame ();
			break;

		case svc_inventory:
			CL_ParseInventory ();
			break;

		case svc_lazarus_fog:	// Knightmare added
			CL_ParseLazarusFog ();
			break;

		case svc_layout:
			s = MSG_ReadString (&net_message);
			strncpy (cl.layout, s, sizeof(cl.layout)-1);
			break;

		case svc_playerinfo:
		case svc_packetentities:
		case svc_deltapacketentities:
			Com_Error (ERR_DROP, "Out of place frame data");
			break;
		}
	}

	CL_AddNetgraph ();

	//
	// we don't know if it is ok to save a demo message until
	// after we have parsed the frame
	//
	if (cls.demorecording && !cls.demowaiting)
		CL_WriteDemoMessage ();

}
