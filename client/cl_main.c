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

// Copyright (C) 2001-2003 pat@aftermoon.net for modif flanked by <serverping>

// cl_main.c  -- client main loop

#include "client.h"
#include "../ui/ui_local.h"

#ifdef _WIN32
#include "../win32/winquake.h"
#endif

cvar_t	*freelook;

cvar_t	*adr0;
cvar_t	*adr1;
cvar_t	*adr2;
cvar_t	*adr3;
cvar_t	*adr4;
cvar_t	*adr5;
cvar_t	*adr6;
cvar_t	*adr7;
cvar_t	*adr8;
cvar_t	*adr9;
cvar_t	*adr10;
cvar_t	*adr11;

cvar_t	*cl_stereo_separation;
cvar_t	*cl_stereo;

cvar_t	*rcon_client_password;
cvar_t	*rcon_address;

cvar_t	*cl_noskins;
//cvar_t	*cl_autoskins;	// unused
cvar_t	*cl_footsteps;
cvar_t	*cl_timeout;
cvar_t	*cl_predict;
//cvar_t	*cl_minfps;
cvar_t	*cl_maxfps;

#ifdef CLIENT_SPLIT_NETFRAME
cvar_t	*cl_async;
cvar_t	*net_maxfps;
cvar_t	*r_maxfps;
cvar_t	*r_maxfps_autoset;
#endif

cvar_t	*cl_sleep; 
// whether to trick version 34 servers that this is a version 34 client
cvar_t	*cl_servertrick;

cvar_t	*cl_gun;
cvar_t	*cl_weapon_shells;

// reduction factor for particle effects
cvar_t	*cl_particle_scale;

// whether to adjust fov for wide aspect rattio
cvar_t	*cl_widescreen_fov;

// Psychospaz's chasecam
cvar_t	*cg_thirdperson;
cvar_t	*cg_thirdperson_angle;
cvar_t	*cg_thirdperson_chase;
cvar_t	*cg_thirdperson_dist;
cvar_t	*cg_thirdperson_offset;
cvar_t	*cg_thirdperson_alpha;
cvar_t	*cg_thirdperson_adjust;
cvar_t	*cg_thirdperson_indemo;
cvar_t	*cg_thirdperson_overhead;
cvar_t	*cg_thirdperson_overhead_dist;

cvar_t	*cl_blood;
cvar_t	*cl_old_explosions;	// Option for old explosions
cvar_t	*cl_plasma_explo_sound;	// Option for unique plasma explosion sound
cvar_t	*cl_item_bobbing;	// Option for bobbing items

// Psychospaz's rail code
cvar_t	*cl_railred;
cvar_t	*cl_railgreen;
cvar_t	*cl_railblue;
cvar_t	*cl_railtype;
cvar_t	*cl_rail_length;
cvar_t	*cl_rail_space;

// whether to use texsurfs.txt footstep sounds
cvar_t	*cl_footstep_override;

cvar_t	*r_decals;		// decal quantity
cvar_t	*r_decal_life;  // decal duration in seconds

cvar_t	*con_font_size;
cvar_t	*alt_text_color;

// whether to try to play OGGs instead of CD tracks
cvar_t	*cl_ogg_music;
cvar_t	*cl_rogue_music; // whether to play Rogue tracks
cvar_t	*cl_xatrix_music; // whether to play Xatrix tracks


cvar_t	*cl_add_particles;
cvar_t	*cl_add_lights;
cvar_t	*cl_add_entities;
cvar_t	*cl_add_blend;

cvar_t	*cl_shownet;
cvar_t	*cl_showmiss;
cvar_t	*cl_showclamp;

cvar_t	*cl_paused;
cvar_t	*cl_timedemo;


cvar_t	*lookspring;
cvar_t	*lookstrafe;
cvar_t	*sensitivity;

//cvar_t	*menu_sensitivity;
//cvar_t	*menu_rotate;
//cvar_t	*menu_alpha;

cvar_t	*m_pitch;
cvar_t	*m_yaw;
cvar_t	*m_forward;
cvar_t	*m_side;

cvar_t	*cl_lightlevel;

//
// userinfo
//
cvar_t	*info_password;
cvar_t	*info_spectator;
cvar_t	*name;
cvar_t	*skin;
cvar_t	*rate;
cvar_t	*fov;
cvar_t	*msg;
cvar_t	*hand;
cvar_t	*gender;
cvar_t	*gender_auto;

cvar_t	*cl_vwep;

// for the server to tell which version the client is
cvar_t *cl_engine;
cvar_t *cl_engine_version;

#ifdef USE_CURL	// HTTP downloading from R1Q2
cvar_t	*cl_http_downloads;
cvar_t	*cl_http_filelists;
cvar_t	*cl_http_proxy;
cvar_t	*cl_http_max_connections;
cvar_t	*cl_http_fallback;
#endif	// USE_CURL

#ifdef LOC_SUPPORT	// Xile/NiceAss LOC
cvar_t	*cl_drawlocs;
cvar_t	*loc_here;
cvar_t	*loc_there;
#endif	// LOC_SUPPORT

// Chat Ignore from R1Q2/Q2Pro
chatIgnore_t	cl_chatNickIgnores;
chatIgnore_t	cl_chatTextIgnores;
// end R1Q2/Q2Pro Chat Ignore

client_static_t	cls;
client_state_t	cl;

centity_t		cl_entities[MAX_EDICTS];

entity_state_t	cl_parse_entities[MAX_PARSE_ENTITIES];

qboolean		local_initialized = false;

//======================================================================


/*
====================
CL_WriteDemoMessage

Dumps the current net message, prefixed by the length
====================
*/
void CL_WriteDemoMessage (void)
{
	int		len, swlen;

	// the first eight bytes are just packet sequencing stuff
	len = net_message.cursize-8;
	swlen = LittleLong(len);
	fwrite (&swlen, 4, 1, cls.demofile);
	fwrite (net_message.data+8,	len, 1, cls.demofile);
}


/*
====================
CL_Stop_f

stop recording a demo
====================
*/
void CL_Stop_f (void)
{
	int		len;

	if (!cls.demorecording)
	{
		Com_Printf ("Not recording a demo.\n");
		return;
	}

// finish up
	len = -1;
	fwrite (&len, 4, 1, cls.demofile);
	fclose (cls.demofile);
	cls.demofile = NULL;
	cls.demorecording = false;
	Com_Printf ("Stopped demo.\n");
}

/*
====================
CL_Record_f

record <demoname>

Begins recording a demo from the current position
====================
*/
void CL_Record_f (void)
{
	char	name[MAX_OSPATH];
	char	buf_data[MAX_MSGLEN];
	sizebuf_t	buf;
	int		i;
	int		len;
	entity_state_t	*ent;
	entity_state_t	nullstate;

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("record <demoname>\n");
		return;
	}

	if (cls.demorecording)
	{
		Com_Printf ("Already recording.\n");
		return;
	}

	if (cls.state != ca_active)
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
	cls.demofile = fopen (name, "wb");
	if (!cls.demofile)
	{
		Com_Printf ("ERROR: couldn't open.\n");
		return;
	}
	cls.demorecording = true;

	// don't start saving messages until a non-delta compressed message is received
	cls.demowaiting = true;

	//
	// write out messages to hold the startup information
	//
	SZ_Init (&buf, buf_data, sizeof(buf_data));

	// send the serverdata
	MSG_WriteByte (&buf, svc_serverdata);
	MSG_WriteLong (&buf, PROTOCOL_VERSION);
	MSG_WriteLong (&buf, 0x10000 + cl.servercount);
	MSG_WriteByte (&buf, 1);	// demos are always attract loops
	MSG_WriteString (&buf, cl.gamedir);
	MSG_WriteShort (&buf, cl.playernum);

	MSG_WriteString (&buf, cl.configstrings[CS_NAME]);

	// configstrings
	for (i=0 ; i<MAX_CONFIGSTRINGS ; i++)
	{
		if (cl.configstrings[i][0])
		{
			if (buf.cursize + strlen (cl.configstrings[i]) + 32 > buf.maxsize)
			{	// write it out
				len = LittleLong (buf.cursize);
				fwrite (&len, 4, 1, cls.demofile);
				fwrite (buf.data, buf.cursize, 1, cls.demofile);
				buf.cursize = 0;
			}

			MSG_WriteByte (&buf, svc_configstring);
			MSG_WriteShort (&buf, i);
			MSG_WriteString (&buf, cl.configstrings[i]);
		}

	}

	// baselines
	memset (&nullstate, 0, sizeof(nullstate));
	for (i=0; i<MAX_EDICTS ; i++)
	{
		ent = &cl_entities[i].baseline;
		if (!ent->modelindex)
			continue;

		if (buf.cursize + 64 > buf.maxsize)
		{	// write it out
			len = LittleLong (buf.cursize);
			fwrite (&len, 4, 1, cls.demofile);
			fwrite (buf.data, buf.cursize, 1, cls.demofile);
			buf.cursize = 0;
		}

		MSG_WriteByte (&buf, svc_spawnbaseline);		
		MSG_WriteDeltaEntity (&nullstate, &cl_entities[i].baseline, &buf, true, true);
	}

	MSG_WriteByte (&buf, svc_stufftext);
	MSG_WriteString (&buf, "precache\n");

	// write it to the demo file

	len = LittleLong (buf.cursize);
	fwrite (&len, 4, 1, cls.demofile);
	fwrite (buf.data, buf.cursize, 1, cls.demofile);

	// the rest of the demo file will be individual frames
}

//======================================================================
// Chat Ignore from R1Q2/Q2Pro
//======================================================================

/*
===================
CL_FindChatIgnore
===================
*/
chatIgnore_t *CL_FindChatIgnore (chatIgnore_t *ignoreList, const char *match)
{
	chatIgnore_t	*cur=NULL;

	if (!ignoreList || !ignoreList->next)	// no list to search
		return NULL;
	if ( !match || (strlen(match) < 1) )	// no search string
		return NULL;

	for (cur = ignoreList->next; cur != NULL; cur = cur->next)
	{
		if ( !cur->text || (strlen(cur->text) < 1) )
			continue;
		if (!strcmp(cur->text, match))
			return cur;
	}

	return NULL;
}


/*
===================
CL_AddChatIgnore
===================
*/
qboolean CL_AddChatIgnore (chatIgnore_t *ignoreList, const char *add)
{
	chatIgnore_t	*next=NULL, *newEntry=NULL;
	size_t			textLen;

	if (!ignoreList)	// nothing to remove
		return false;
	if ( !add || (strlen(add) < 1) )	// no string to add
		return false;

	// Don't add the same ignore twice
	if ( CL_FindChatIgnore (ignoreList, add) ) {
		Com_Printf ("%s is already in ignore list.\n", add);
		return false;
	}

	next = ignoreList->next;	// should be NULL for first entry
	textLen = strlen(Cmd_Argv(1))+1;
	newEntry = Z_Malloc (sizeof(chatIgnore_t));
	newEntry->numHits = 0;
	newEntry->text = Z_Malloc (textLen);
	Q_strncpyz (newEntry->text, textLen, Cmd_Argv(1));
	newEntry->next = next;
	ignoreList->next = newEntry;

	return true;
}


/*
===================
CL_RemoveChatIgnore
===================
*/
qboolean CL_RemoveChatIgnore (chatIgnore_t *ignoreList, const char *match)
{
	chatIgnore_t	*cur=NULL, *last=NULL, *next=NULL;

	if (!ignoreList || !ignoreList->next)	// nothing to remove
		return false;
	if ( !match || (strlen(match) < 1) )	// no search string
		return false;

	for (last = ignoreList, cur = ignoreList->next; cur != NULL; last = cur, cur = cur->next)
	{
		if ( !cur->text || (strlen(cur->text) < 1) )
			continue;
		if ( !strcmp(match, cur->text) )
		{
			next = cur->next;
			last->next = next;

			Z_Free (cur->text);
			cur->text = NULL;
			Z_Free (cur);

			return true;
		}
	}

	Com_Printf ("Can't find ignore filter \"%s\"\n", match);
	return false;
}


/*
===================
CL_RemoveAllChatIgnores
===================
*/
void CL_RemoveAllChatIgnores (chatIgnore_t *ignoreList)
{
	chatIgnore_t	*cur=NULL, *next=NULL;
	int				count = 0;

	if (!ignoreList || !ignoreList->next)	// nothing to remove
		return;

	cur = ignoreList->next;
	next = cur->next;
	do
	{
		if (cur->text != NULL) {
			Z_Free (cur->text);
			cur->text = NULL;
		}
		next = cur->next;
		Z_Free (cur);
		cur = next;
		count++;
	} while (cur != NULL);

	ignoreList->next = NULL;

	Com_Printf ("Removed %i ignore filter(s).\n", count);
}


/*
===================
CL_ListChatIgnores
===================
*/
void CL_ListChatIgnores (chatIgnore_t *ignoreList)
{
	chatIgnore_t	*cur=NULL;

	if (!ignoreList || !ignoreList->next)	// no list to output
		return;

	Com_Printf ("Current ignore filters:\n");
	for (cur = ignoreList->next; cur != NULL; cur = cur->next)
	{
		if ( !cur->text || (strlen(cur->text) < 1) )
			continue;
		Com_Printf ("\"%s\" (%i hits)\n", cur->text, cur->numHits);
	}
}


/*
===================
CL_IgnoreChatNick_f
===================
*/
void CL_IgnoreChatNick_f (void)
{
	qboolean		added;

	if (Cmd_Argc() < 2) {
		Com_Printf ("Usage: ignorenick <nick>\n");
		CL_ListChatIgnores (&cl_chatNickIgnores);	// output list if no param
		return;
	}

	added = CL_AddChatIgnore (&cl_chatNickIgnores, Cmd_Argv(1));

	if (added)
		Com_Printf ("%s added to nick ignore list.\n", Cmd_Argv(1));
}


/*
===================
CL_UnIgnoreChatNick_f
===================
*/
void CL_UnIgnoreChatNick_f (void)
{
	qboolean		removed;

	if (Cmd_Argc() < 2) {
		Com_Printf ("Usage: unignorenick <nick>\n");
		CL_ListChatIgnores (&cl_chatNickIgnores);	// output list if no param
		return;
	}

	if ( (Cmd_Argc() == 2) && !strcmp(Cmd_Argv(1), "all") ) {
		 CL_RemoveAllChatIgnores (&cl_chatNickIgnores);
		 return;
	}

	removed = CL_RemoveChatIgnore (&cl_chatNickIgnores, Cmd_Argv(1));

	if (removed)
		Com_Printf ("%s removed from nick ignore list.\n", Cmd_Argv(1));
}


/*
===================
CL_IgnoreChatText_f
===================
*/
void CL_IgnoreChatText_f (void)
{
	qboolean		added;

	if (Cmd_Argc() < 2) {
		Com_Printf ("Usage: ignoretext <text>\n");
		CL_ListChatIgnores (&cl_chatTextIgnores);	// output list if no param
		return;
	}

	added = CL_AddChatIgnore (&cl_chatTextIgnores, Cmd_Argv(1));

	if (added)
		Com_Printf ("%s added to text ignore list.\n", Cmd_Argv(1));
}


/*
===================
CL_UnIgnoreChatText_f
===================
*/
void CL_UnIgnoreChatText_f (void)
{
	qboolean		removed;

	if (Cmd_Argc() < 2) {
		Com_Printf ("Usage: unignoretext <text>\n");
		CL_ListChatIgnores (&cl_chatTextIgnores);	// output list if no param
		return;
	}

	if ( (Cmd_Argc() == 2) && !strcmp(Cmd_Argv(1), "all") ) {
		 CL_RemoveAllChatIgnores (&cl_chatTextIgnores);
		 return;
	}

	removed = CL_RemoveChatIgnore (&cl_chatTextIgnores, Cmd_Argv(1));

	if (removed)
		Com_Printf ("%s removed from text ignore list.\n", Cmd_Argv(1));
}


/*
===================
CL_ChatMatchIgnoreNick
===================
*/
qboolean CL_ChatMatchIgnoreNick (const char *buf, size_t bufSize, const char *nick)
{
	size_t		nickLen = strlen(nick);
	char		*string = (char *)buf, *p = NULL;
	int			idx = 0;
	qboolean	clanTag;

//	Com_Printf ("CL_ChatMatchIgnoreNick: Searching for nick %s in chat message %s\n", nick, buf);

	do
	{
		clanTag = false;
		idx++;

		// catch nick with ": " following
		if ( !strncmp(string, nick, nickLen) && !strncmp(string + nickLen, ": ", 2) )
			return true;
		
		if (*string == '(')	// catch nick in parenthesis
		{
			if (!strncmp(string + 1, nick, nickLen) && !strncmp(string + 1 + nickLen, "): ", 3) )
				return true;
		}

		// skip over clan tag in []
		if (*string == '[') {
			p = strstr(string + 1, "] ");
			if (p) {
			//	Com_Printf ("CL_ChatMatchIgnoreNick: skipping over clan tag\n");
				clanTag = true;
				string = p + 2;
			}
		}
	}
	while ( clanTag && (idx < 2) && (string < (buf + bufSize)) );

	return false;
}


/*
===================
CL_CheckforChatIgnore
===================
*/
qboolean CL_CheckForChatIgnore (const char *string)
{
	char			chatBuf[MSG_STRING_SIZE];
	chatIgnore_t	*compare=NULL;

	if (!cl_chatNickIgnores.next && !cl_chatTextIgnores.next)	// nothing in lists to compare
		return false;

	Q_strncpyz (chatBuf, sizeof(chatBuf), unformattedString(string));
//	Com_Printf ("CL_CheckForChatIgnore: scanning chat message \"%s\" for ignore nicks and text\n", chatBuf);

	if (cl_chatNickIgnores.next != NULL)
	{
		for (compare = cl_chatNickIgnores.next; compare != NULL; compare = compare->next)
		{
			if ( (compare->text != NULL) && (strlen(compare->text) > 0) ) {
				if ( CL_ChatMatchIgnoreNick(chatBuf, sizeof(chatBuf), compare->text) ) {
				//	Com_Printf ("CL_CheckForChatIgnore: filtered nick %s in chat message\n", compare->text);
					compare->numHits++;
					return true;
				}
			}
		}
	}

	if (cl_chatTextIgnores.next != NULL)
	{
		for (compare = cl_chatTextIgnores.next; compare != NULL; compare = compare->next)
		{
			if ( (compare->text != NULL) && (strlen(compare->text) > 0) ) {
				if ( Q_StrScanToken (chatBuf, compare->text, false) ) {
				//	Com_Printf ("CL_CheckForChatIgnore: filtered text %s in chat message\n", compare->text);
					compare->numHits++;
					return true;
				}
			}
		}
	}

	return false;
}

//======================================================================
// end R1Q2/Q2Pro Chat Ignore
//======================================================================

/*
===================
Cmd_ForwardToServer

adds the current command line as a clc_stringcmd to the client message.
things like godmode, noclip, etc, are commands directed to the server,
so when they are typed in at the console, they will need to be forwarded.
===================
*/
void Cmd_ForwardToServer (void)
{
	char	*cmd;

	cmd = Cmd_Argv(0);
	if (cls.state <= ca_connected || *cmd == '-' || *cmd == '+')
	{
		Com_Printf ("Unknown command \"%s\"\n", cmd);
		return;
	}

	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	SZ_Print (&cls.netchan.message, cmd);
	if (Cmd_Argc() > 1)
	{
		SZ_Print (&cls.netchan.message, " ");
		SZ_Print (&cls.netchan.message, Cmd_Args());
	}
	cls.forcePacket = true;
}

void CL_Setenv_f( void )
{
	int argc = Cmd_Argc();

	if ( argc > 2 )
	{
		char buffer[1000];
		int i;

	//	strncpy(buffer, Cmd_Argv(1));
	//	strncat(buffer, "=");
		Q_strncpyz (buffer, sizeof(buffer), Cmd_Argv(1));
		Q_strncatz (buffer, sizeof(buffer), "=");

		for ( i = 2; i < argc; i++ )
		{
		//	strncat(buffer, Cmd_Argv( i ));
		//	strncat(buffer, " ");
			Q_strncatz (buffer, sizeof(buffer), Cmd_Argv( i ));
			Q_strncatz (buffer, sizeof(buffer), " ");
		}

		putenv( buffer );
	}
	else if ( argc == 2 )
	{
		char *env = getenv( Cmd_Argv(1) );

		if ( env )
		{
			Com_Printf( "%s=%s\n", Cmd_Argv(1), env );
		}
		else
		{
			Com_Printf( "%s undefined\n", Cmd_Argv(1), env );
		}
	}
}


/*
==================
CL_ForwardToServer_f
==================
*/
void CL_ForwardToServer_f (void)
{
	if (cls.state != ca_connected && cls.state != ca_active)
	{
		Com_Printf ("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}
	
	// don't forward the first argument
	if (Cmd_Argc() > 1)
	{
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		SZ_Print (&cls.netchan.message, Cmd_Args());
		cls.forcePacket = true;
	}
}


/*
==================
CL_Pause_f
==================
*/
void CL_Pause_f (void)
{
	// never pause in multiplayer
	if (Cvar_VariableValue ("maxclients") > 1 || !Com_ServerState ())
	{
		Cvar_SetValue ("paused", 0);
		return;
	}

//	Cvar_SetValue ("paused", !cl_paused->value);
	Cvar_SetValue ("paused", !cl_paused->integer);
}

/*
==================
CL_Quit_f
==================
*/
void CL_Quit_f (void)
{
	CL_Disconnect ();
	Com_Quit ();
}

/*
================
CL_Drop

Called after an ERR_DROP was thrown
================
*/
void CL_Drop (void)
{
	if (cls.state == ca_uninitialized)
		return;

	// if an error occurs during initial load
	// or during game start, drop loading plaque
	if ( (cls.disable_servercount != -1) || (cls.key_dest == key_game) )
		SCR_EndLoadingPlaque ();	// get rid of loading plaque

	if (cls.state == ca_disconnected)
		return;

	CL_Disconnect ();
}


/*
=======================
CL_SendConnectPacket

We have gotten a challenge from the server, so try and
connect.
======================
*/
void CL_SendConnectPacket (void)
{
	netadr_t	adr;
	int			port, sendProtocolVersion;

	if (!NET_StringToAdr (cls.servername, &adr))
	{
		Com_Printf ("Bad server address\n");
		cls.connect_time = 0;
		return;
	}
	if (adr.port == 0)
		adr.port = BigShort (PORT_SERVER);

	port = Cvar_VariableValue ("qport");
	userinfo_modified = false;

	// if in compatibility mode, lie to server about this
	// client's protocol, but exclude localhost for this.
	sendProtocolVersion = ((cl_servertrick->integer != 0) && strcmp(cls.servername, "localhost")) ? OLD_PROTOCOL_VERSION : PROTOCOL_VERSION;

	Netchan_OutOfBandPrint (NS_CLIENT, adr, "connect %i %i %i \"%s\"\n", sendProtocolVersion, port, cls.challenge, Cvar_Userinfo() );
}


/*
==================
CL_ForcePacket
==================
*/
void CL_ForcePacket (void)
{
	cls.forcePacket = true;
}


/*
=================
CL_CheckForResend

Resend a connect message if the last one has timed out
=================
*/
void CL_CheckForResend (void)
{
	netadr_t	adr;

	// if the local server is running and we aren't
	// then connect
	if (cls.state == ca_disconnected && Com_ServerState() )
	{
		cls.state = ca_connecting;
		strncpy (cls.servername, "localhost", sizeof(cls.servername)-1);
		// we don't need a challenge on the localhost
		CL_SendConnectPacket ();
		return;
//		cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately
	}

	// resend if we haven't gotten a reply yet
	if (cls.state != ca_connecting)
		return;

	if (cls.realtime - cls.connect_time < 3000)
		return;

	if (!NET_StringToAdr (cls.servername, &adr))
	{
		Com_Printf ("Bad server address\n");
		cls.state = ca_disconnected;
		return;
	}
	if (adr.port == 0)
		adr.port = BigShort (PORT_SERVER);

	cls.connect_time = cls.realtime;	// for retransmit requests

	Com_Printf ("Connecting to %s...\n", cls.servername);

	Netchan_OutOfBandPrint (NS_CLIENT, adr, "getchallenge\n");
}


/*
================
CL_Connect_f

================
*/
void CL_Connect_f (void)
{
	char		*server, *p;
	netadr_t	serverAdr;

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("usage: connect <server>\n");
		return;	
	}
	
	if (Com_ServerState ())
	{	// if running a local server, kill it and reissue
	//	SV_Shutdown (va("Server quit\n", msg), false);
		SV_Shutdown ("Server quit.\n", false);
	}
	else
	{
		CL_Disconnect ();
	}

	server = Cmd_Argv (1);

	// start quake2:// support
	if (!strncmp (server, "quake2://", 9))
		server += 9;

	p = strchr (server, '/');	// remove trailing slash
	if (p)
		p[0] = '\0';
	// end quake2:// support 

	NET_Config (true);		// allow remote

	// validate server address
	if (!NET_StringToAdr (server, &serverAdr))
	{
		Com_Printf ("Bad server address: %s\n", server);
		return;
	}

	CL_Disconnect ();

	cls.state = ca_connecting;
	strncpy (cls.servername, server, sizeof(cls.servername)-1);
	cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately
}


/*
=====================
CL_Rcon_f

  Send the rest of the command line over as
  an unconnected command.
=====================
*/
void CL_Rcon_f (void)
{
	char	message[1024];
	int		i;
	netadr_t	to;

	if (!rcon_client_password->string)
	{
		Com_Printf ("You must set 'rcon_password' before\n"
					"issuing an rcon command.\n");
		return;
	}

	message[0] = (char)255;
	message[1] = (char)255;
	message[2] = (char)255;
	message[3] = (char)255;
	message[4] = 0;

	NET_Config (true);		// allow remote

//	strncat (message, "rcon ");
//	strncat (message, rcon_client_password->string);
//	strncat (message, " ");
	Q_strncatz (message, sizeof(message), "rcon ");
	Q_strncatz (message, sizeof(message), rcon_client_password->string);
	Q_strncatz (message, sizeof(message), " ");

	for (i=1 ; i<Cmd_Argc() ; i++)
	{
	//	strncat (message, Cmd_Argv(i));
	//	strncat (message, " ");
		Q_strncatz (message, sizeof(message), Cmd_Argv(i));
		Q_strncatz (message, sizeof(message), " ");
	}

	if (cls.state >= ca_connected)
		to = cls.netchan.remote_address;
	else
	{
		if (!strlen(rcon_address->string))
		{
			Com_Printf ("You must either be connected,\n"
						"or set the 'rcon_address' cvar\n"
						"to issue rcon commands\n");

			return;
		}
		NET_StringToAdr (rcon_address->string, &to);
		if (to.port == 0)
			to.port = BigShort (PORT_SERVER);
	}
	
	NET_SendPacket (NS_CLIENT, (int)strlen(message)+1, message, to);
}


/*
=====================
CL_ClearState

=====================
*/
void CL_ClearState (void)
{
	S_StopAllSounds ();
	CL_ClearEffects ();
	CL_ClearTEnts ();
	V_ClearFogInfo ();

	R_ClearState ();

// wipe the entire cl structure
	memset (&cl, 0, sizeof(cl));
	memset (&cl_entities, 0, sizeof(cl_entities));

	cl.maxclients = MAX_CLIENTS;	// from R1Q2
	SZ_Clear (&cls.netchan.message);
}

/*
=====================
CL_Disconnect

Goes from a connected state to full screen console state
Sends a disconnect message to the server
This is also called on Com_Error, so it shouldn't cause any errors
=====================
*/
extern	char	*ui_currentweaponmodel;
void CL_Disconnect (void)
{
	byte	final[32];

	if (cls.state == ca_disconnected)
		return;

//	if (cl_timedemo && cl_timedemo->value)
	if (cl_timedemo && cl_timedemo->integer)
	{
		int	time;
		
		time = Sys_Milliseconds () - cl.timedemo_start;
		if (time > 0)
			Com_Printf ("%i frames, %3.1f seconds: %3.1f fps\n", cl.timedemo_frames,
			time/1000.0, cl.timedemo_frames*1000.0 / time);
	}

	VectorClear (cl.refdef.blend);
	//R_SetPalette(NULL);

	UI_ForceMenuOff ();

	cls.connect_time = 0;

	SCR_StopCinematic ();

	if (cls.demorecording)
		CL_Stop_f ();

	// send a disconnect message to the server
	final[0] = clc_stringcmd;
//	strncpy ((char *)final+1, "disconnect");
	Q_strncpyz ((char *)final+1, sizeof(final)-1, "disconnect");
	Netchan_Transmit (&cls.netchan, (int)strlen(final), final);
	Netchan_Transmit (&cls.netchan, (int)strlen(final), final);
	Netchan_Transmit (&cls.netchan, (int)strlen(final), final);

	CL_ClearState ();

	// stop download
	if (cls.download) {
		fclose(cls.download);
		cls.download = NULL;
	}

#ifdef USE_CURL	// HTTP downloading from R1Q2
	CL_CancelHTTPDownloads (true);
	cls.downloadReferer[0] = 0;
	cls.downloadname[0] = 0;
	cls.downloadposition = 0;
#endif	// USE_CURL

	cls.state = ca_disconnected;

	// reset current weapon model
	ui_currentweaponmodel = NULL;
}

void CL_Disconnect_f (void)
{
	Com_Error (ERR_DROP, "Disconnected from server");
}


/*
====================
CL_Packet_f

packet <destination> <contents>

Contents allows \n escape character
====================
*/
void CL_Packet_f (void)
{
	char	send[2048];
	int		i, l;
	char	*in, *out;
	netadr_t	adr;

	if (Cmd_Argc() != 3)
	{
		Com_Printf ("packet <destination> <contents>\n");
		return;
	}

	NET_Config (true);		// allow remote

	if (!NET_StringToAdr (Cmd_Argv(1), &adr))
	{
		Com_Printf ("Bad address\n");
		return;
	}
	if (!adr.port)
		adr.port = BigShort (PORT_SERVER);

	in = Cmd_Argv(2);
	out = send+4;
	send[0] = send[1] = send[2] = send[3] = (char)0xff;

	l = (int)strlen (in);
	for (i=0 ; i<l ; i++)
	{
		if (in[i] == '\\' && in[i+1] == 'n')
		{
			*out++ = '\n';
			i++;
		}
		else
			*out++ = in[i];
	}
	*out = 0;

	NET_SendPacket (NS_CLIENT, out-send, send, adr);
}

/*
=================
CL_Changing_f

Just sent as a hint to the client that they should
drop to full console
=================
*/
void CL_Changing_f (void)
{
	//ZOID
	//if we are downloading, we don't change!  This so we don't suddenly stop downloading a map
	SCR_BeginLoadingPlaque (); // Knightmare moved here

	if (cls.download)
		return;

	//SCR_BeginLoadingPlaque ();
	cls.state = ca_connected;	// not active anymore, but not disconnected
	Com_Printf ("\nChanging map...\n");

#ifdef USE_CURL
	// FS: Added because Whale's Weapons HTTP server rejects you after a lot of 404s.  Then you lose HTTP until a hard reconnect.
	if (cls.downloadServerRetry[0] != 0) {
		CL_SetHTTPServer(cls.downloadServerRetry);
	}
#endif
}


/*
=================
CL_Reconnect_f

The server is changing levels
=================
*/
void CL_Reconnect_f (void)
{
	//ZOID
	//if we are downloading, we don't change!  This so we don't suddenly stop downloading a map
	if (cls.download)
		return;

	S_StopAllSounds ();
	if (cls.state == ca_connected)
	{
		Com_Printf ("reconnecting...\n");
		cls.state = ca_connected;
		MSG_WriteChar (&cls.netchan.message, clc_stringcmd);
		MSG_WriteString (&cls.netchan.message, "new");		
		cls.forcePacket = true;
		return;
	}

	if (*cls.servername)
	{
		if (cls.state >= ca_connected)
		{
			CL_Disconnect();
			cls.connect_time = cls.realtime - 1500;
		} else
			cls.connect_time = -99999; // fire immediately

		cls.state = ca_connecting;
		Com_Printf ("reconnecting...\n");
	}
}

/*
=================
CL_ParseStatusMessage

Handle a reply from a ping
=================
*/
void CL_ParseStatusMessage (void)
{
	char	*s;

	s = MSG_ReadString(&net_message);

	Com_Printf ("%s\n", s);
	UI_AddToServerList (net_from, s);
}


/*
=================
CL_PingServers_f
=================
*/
#if 1
//<serverping> Added code for compute ping time of server broadcasted
extern int      global_udp_server_time;
extern int      global_ipx_server_time;
extern int      global_adr_server_time[16];
extern netadr_t global_adr_server_netadr[16];

void CL_PingServers_f (void)
{
	int			i, sendProtocolVersion;
	netadr_t	adr;
	char		name[32];
	char		*adrstring;
	cvar_t		*noudp;
	cvar_t		*noipx;

	NET_Config (true);		// allow remote

	// if the server is using the old protocol,
	// lie to it about this client's protocol
	sendProtocolVersion = (cl_servertrick->integer != 0) ? OLD_PROTOCOL_VERSION : PROTOCOL_VERSION;

	// send a broadcast packet
	Com_Printf ("pinging broadcast...\n");

	// send a packet to each address book entry
	for (i=0 ; i<16 ; i++)
	{
        memset(&global_adr_server_netadr[i], 0, sizeof(global_adr_server_netadr[0]));
        global_adr_server_time[i] = Sys_Milliseconds() ;

        Com_sprintf (name, sizeof(name), "adr%i", i);
		adrstring = Cvar_VariableString (name);
		if (!adrstring || !adrstring[0])
			continue;

		Com_Printf ("pinging %s...\n", adrstring);
		if (!NET_StringToAdr (adrstring, &adr))
		{
			Com_Printf ("Bad address: %s\n", adrstring);
			continue;
		}
		if (!adr.port)
			adr.port = BigShort(PORT_SERVER);
        
        memcpy(&global_adr_server_netadr[i], &adr, sizeof(global_adr_server_netadr));

		Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", sendProtocolVersion));
	}

	noudp = Cvar_Get ("noudp", "0", CVAR_NOSET);
	if (!noudp->integer)
	{
        global_udp_server_time = Sys_Milliseconds() ;
		adr.type = NA_BROADCAST;
		adr.port = BigShort(PORT_SERVER);
		Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", sendProtocolVersion));
	}

	noipx = Cvar_Get ("noipx", "0", CVAR_NOSET);
	if (!noipx->integer)
	{
        global_ipx_server_time = Sys_Milliseconds() ;
		adr.type = NA_BROADCAST_IPX;
		adr.port = BigShort(PORT_SERVER);
		Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", sendProtocolVersion));
	}
}
//</<serverping>>
#else
void CL_PingServers_f (void)
{
	int			i, sendProtocolVersion;
	netadr_t	adr;
	char		name[32];
	char		*adrstring;
	cvar_t		*noudp;
	cvar_t		*noipx;

	NET_Config (true);		// allow remote

	// send a broadcast packet
	Com_Printf ("pinging broadcast...\n");

	// if the server is using the old protocol,
	// lie to it about this client's protocol
	sendProtocolVersion = (cl_servertrick->integer != 0) ? OLD_PROTOCOL_VERSION : PROTOCOL_VERSION;

	noudp = Cvar_Get ("noudp", "0", CVAR_NOSET);
	if (!noudp->integer)
	{
		adr.type = NA_BROADCAST;
		adr.port = BigShort(PORT_SERVER);
		Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", sendProtocolVersion));
	}

	noipx = Cvar_Get ("noipx", "0", CVAR_NOSET);
	if (!noipx->integer)
	{
		adr.type = NA_BROADCAST_IPX;
		adr.port = BigShort(PORT_SERVER);
		Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", sendProtocolVersion));
	}

	// send a packet to each address book entry
	for (i=0; i<16; i++)
	{
		Com_sprintf (name, sizeof(name), "adr%i", i);
		adrstring = Cvar_VariableString (name);
		if (!adrstring || !adrstring[0])
			continue;

		Com_Printf ("pinging %s...\n", adrstring);
		if (!NET_StringToAdr (adrstring, &adr))
		{
			Com_Printf ("Bad address: %s\n", adrstring);
			continue;
		}
		if (!adr.port)
			adr.port = BigShort(PORT_SERVER);

		Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", sendProtocolVersion));
	}
}
#endif

/*
=================
CL_Skins_f

Load or download any custom player skins and models
=================
*/
void CL_Skins_f (void)
{
	int		i;

	for (i=0 ; i<MAX_CLIENTS ; i++)
	{
		// BIG UGLY HACK for old connected to server using old protocol
		// Changed config strings require different parsing
		if ( LegacyProtocol() )
		{
			if (!cl.configstrings[OLD_CS_PLAYERSKINS+i][0])
				continue;
			Com_Printf ("client %i: %s\n", i, cl.configstrings[OLD_CS_PLAYERSKINS+i]); 

		} else {
			if (!cl.configstrings[CS_PLAYERSKINS+i][0])
				continue;
			Com_Printf ("client %i: %s\n", i, cl.configstrings[CS_PLAYERSKINS+i]); 
		}
		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();	// pump message loop
		CL_ParseClientinfo (i);
	}
}


/*
=================
CL_AACSkey_f
=================
*/
void CL_AACSkey_f (void)
{
	Com_Printf ("AACS processing keys: 09 F9 11 02 9D 74 E3 5B D8 41 56 C5 63 56 88 C0\n");
	Com_Printf ("                      45 5F E1 04 22 CA 29 C4 93 3F 95 05 2B 79 2A B2\n");
}


/*
=================
CL_ConnectionlessPacket

Responses to broadcasts, etc
=================
*/
void CL_ConnectionlessPacket (void)
{
	char	*s, *c;
	// HTTP downloading from R1Q2
	char	*buff, *p;
	int		i;
	// end HTTP downloading from R1Q2
	
	MSG_BeginReading (&net_message);
	MSG_ReadLong (&net_message);	// skip the -1

	s = MSG_ReadStringLine (&net_message);

	Cmd_TokenizeString (s, false);

	c = Cmd_Argv(0);

	Com_Printf ("%s: %s\n", NET_AdrToString (net_from), c);

	// server connection
	if (!strcmp(c, "client_connect"))
	{
		if (cls.state == ca_connected)
		{
			Com_Printf ("Dup connect received.  Ignored.\n");
			return;
		}
		Netchan_Setup (NS_CLIENT, &cls.netchan, net_from, cls.quakePort);

		// HTTP downloading from R1Q2
		buff = NET_AdrToString(cls.netchan.remote_address);
		for (i = 1; i < Cmd_Argc(); i++)
		{
			p = Cmd_Argv(i);
			if ( !strncmp (p, "dlserver=", 9) )
			{
#ifdef USE_CURL
				p += 9;
				Com_sprintf (cls.downloadReferer, sizeof(cls.downloadReferer), "quake2://%s", buff);
				CL_SetHTTPServer (p);
				if ( cls.downloadServer[0] )
					Com_Printf ("HTTP downloading enabled, URL: %s\n", cls.downloadServer);
#else
				Com_Printf ("HTTP downloading supported by server but this client was built without USE_CURL, too bad.\n");
#endif	// USE_CURL
			}
		}
		// end HTTP downloading from R1Q2

		MSG_WriteChar (&cls.netchan.message, clc_stringcmd);
		MSG_WriteString (&cls.netchan.message, "new");	
		cls.forcePacket = true;
		cls.state = ca_connected;
		return;
	}

	// server responding to a status broadcast
	if (!strcmp(c, "info"))
	{
		CL_ParseStatusMessage ();
		return;
	}

	// remote command from gui front end
	if (!strcmp(c, "cmd"))
	{
		if (!NET_IsLocalAddress(net_from))
		{
			Com_Printf ("Command packet from remote host.  Ignored.\n");
			return;
		}
		Sys_AppActivate ();
		s = MSG_ReadString (&net_message);
		Cbuf_AddText (s);
		Cbuf_AddText ("\n");
		return;
	}
	// print command from somewhere
	if (!strcmp(c, "print"))
	{
		s = MSG_ReadString (&net_message);
		Com_Printf ("%s", s);
		return;
	}

	// ping from somewhere
	if (!strcmp(c, "ping"))
	{
		Netchan_OutOfBandPrint (NS_CLIENT, net_from, "ack");
		return;
	}

	// challenge from the server we are connecting to
	if (!strcmp(c, "challenge"))
	{
		cls.challenge = atoi(Cmd_Argv(1));
		CL_SendConnectPacket ();
		return;
	}

	// echo request from server
	if (!strcmp(c, "echo"))
	{
		Netchan_OutOfBandPrint (NS_CLIENT, net_from, "%s", Cmd_Argv(1) );
		return;
	}

	Com_Printf ("Unknown command.\n");
}


/*
=================
CL_DumpPackets

A vain attempt to help bad TCP stacks that cause problems
when they overflow
=================
*/
void CL_DumpPackets (void)
{
	while (NET_GetPacket (NS_CLIENT, &net_from, &net_message))
	{
		Com_Printf ("dumnping a packet\n");
	}
}

/*
=================
CL_ReadPackets
=================
*/
void CL_ReadPackets (void)
{
	while (NET_GetPacket (NS_CLIENT, &net_from, &net_message))
	{
//	Com_Printf ("packet\n");
		//
		// remote command packet
		//
		if (*(int *)net_message.data == -1)
		{
			CL_ConnectionlessPacket ();
			continue;
		}

		if (cls.state == ca_disconnected || cls.state == ca_connecting)
			continue;		// dump it if not connected

		if (net_message.cursize < 8)
		{
			Com_Printf ("%s: Runt packet\n",NET_AdrToString(net_from));
			continue;
		}

		//
		// packet from server
		//
		if (!NET_CompareAdr (net_from, cls.netchan.remote_address))
		{
			Com_DPrintf ("%s:sequenced packet without connection\n"
				,NET_AdrToString(net_from));
			continue;
		}
		if (!Netchan_Process(&cls.netchan, &net_message))
			continue;		// wasn't accepted for some reason
		CL_ParseServerMessage ();
	}

	//
	// check timeout
	//
	if (cls.state >= ca_connected
	 && cls.realtime - cls.netchan.last_received > cl_timeout->value*1000)
	{
		if (++cl.timeoutcount > 5)	// timeoutcount saves debugger
		{
			Com_Printf ("\nServer connection timed out.\n");
			CL_Disconnect ();
			return;
		}
	}
	else
		cl.timeoutcount = 0;
	
}


//=============================================================================

/*
==============
CL_FixUpGender_f
==============
*/
void CL_FixUpGender(void)
{
	char *p;
	char sk[80];

//	if (gender_auto->value)
	if (gender_auto->integer)
	{
		if (gender->modified)
		{
			// was set directly, don't override the user
			gender->modified = false;
			return;
		}
		strncpy(sk, skin->string, sizeof(sk) - 1);
		if ((p = strchr(sk, '/')) != NULL)
			*p = 0;
		if (Q_stricmp(sk, "male") == 0 || Q_stricmp(sk, "cyborg") == 0)
			Cvar_Set ("gender", "male");
		else if (Q_stricmp(sk, "female") == 0 || Q_stricmp(sk, "crackhor") == 0)
			Cvar_Set ("gender", "female");
		else
			Cvar_Set ("gender", "none");
		gender->modified = false;
	}
}

/*
==============
CL_Userinfo_f
==============
*/
void CL_Userinfo_f (void)
{
	Com_Printf ("User info settings:\n");
	Info_Print (Cvar_Userinfo());
}

/*
=================
CL_Snd_Restart_f

Restart the sound subsystem so it can pick up
new parameters and flush all sounds
=================
*/
void CL_Snd_Restart_f (void)
{
	S_Shutdown ();
	S_Init ();
	CL_RegisterSounds ();
}


extern	int precache_check; // for autodownload of precache items
extern	int precache_spawncount;
//extern	int precache_tex;
extern	int precache_model_skin;
extern	byte *precache_model; // used for skin checking in alias models
extern	int	precache_pak;	// Knightmare added


/*
=================
CL_ResetPrecacheCheck
=================
*/
void CL_ResetPrecacheCheck (void)
{
//	precache_start_time = Sys_Milliseconds();

	precache_check = CS_MODELS;
//	precache_spawncount = atoi(Cmd_Argv(1));
	precache_model = 0;
	precache_model_skin = 0;
	precache_pak = 0;	// Knightmare added
}


/*
=================
CL_Precache_f

The server will send this command right
before allowing the client into the server
=================
*/
void CL_Precache_f (void)
{
	// Yet another hack to let old demos work
	// the old precache sequence
	if (Cmd_Argc() < 2)
	{
		unsigned	map_checksum;		// for detecting cheater maps

		CM_LoadMap (cl.configstrings[CS_MODELS+1], true, &map_checksum);
		CL_RegisterSounds ();
		CL_PrepRefresh ();
		return;
	}

	precache_check = CS_MODELS;
	precache_spawncount = atoi(Cmd_Argv(1));
	precache_model = 0;
	precache_model_skin = 0;
	precache_pak = 0;	// Knightmare added

#ifdef USE_CURL	// HTTP downloading from R1Q2
	CL_HTTP_ResetMapAbort ();	// Knightmare- reset the map abort flag
#endif	// USE_CURL

	CL_RequestNextDownload();
}


#ifdef LOC_SUPPORT // Xile/NiceAss LOC
/*
=================
CL_AddLoc_f
=================
*/
void CL_AddLoc_f (void)
{
	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: loc_add <label/description>\n");
		return;
	}

	CL_LocAdd (Cmd_Argv(1));
}


/*
=================
CL_DeleteLoc_f
=================
*/
void CL_DeleteLoc_f (void)
{
	CL_LocDelete ();
}


/*
=================
CL_SaveLoc_f
=================
*/
void CL_SaveLoc_f (void)
{
	CL_LocWrite ();
}
#endif	// LOC_SUPPORT


/*
=================
CL_InitLocal
=================
*/
void CL_InitLocal (void)
{
	cls.state = ca_disconnected;
	cls.realtime = Sys_Milliseconds ();

	CL_InitInput ();

	adr0 = Cvar_Get( "adr0", "", CVAR_ARCHIVE );
	adr1 = Cvar_Get( "adr1", "", CVAR_ARCHIVE );
	adr2 = Cvar_Get( "adr2", "", CVAR_ARCHIVE );
	adr3 = Cvar_Get( "adr3", "", CVAR_ARCHIVE );
	adr4 = Cvar_Get( "adr4", "", CVAR_ARCHIVE );
	adr5 = Cvar_Get( "adr5", "", CVAR_ARCHIVE );
	adr6 = Cvar_Get( "adr6", "", CVAR_ARCHIVE );
	adr7 = Cvar_Get( "adr7", "", CVAR_ARCHIVE );
	adr8 = Cvar_Get( "adr8", "", CVAR_ARCHIVE );
	adr9 = Cvar_Get( "adr9", "", CVAR_ARCHIVE );
	adr10 = Cvar_Get( "adr10", "", CVAR_ARCHIVE );
	adr11 = Cvar_Get( "adr11", "", CVAR_ARCHIVE );
	Cvar_SetDescription ("adr0", "Address book entry for Join Server menu.");
	Cvar_SetDescription ("adr1", "Address book entry for Join Server menu.");
	Cvar_SetDescription ("adr2", "Address book entry for Join Server menu.");
	Cvar_SetDescription ("adr3", "Address book entry for Join Server menu.");
	Cvar_SetDescription ("adr4", "Address book entry for Join Server menu.");
	Cvar_SetDescription ("adr5", "Address book entry for Join Server menu.");
	Cvar_SetDescription ("adr6", "Address book entry for Join Server menu.");
	Cvar_SetDescription ("adr7", "Address book entry for Join Server menu.");
	Cvar_SetDescription ("adr8", "Address book entry for Join Server menu.");
	Cvar_SetDescription ("adr9", "Address book entry for Join Server menu.");
	Cvar_SetDescription ("adr10", "Address book entry for Join Server menu.");
	Cvar_SetDescription ("adr11", "Address book entry for Join Server menu.");

	//
	// register our variables
	//
	cl_stereo_separation = Cvar_Get( "cl_stereo_separation", "0.4", CVAR_ARCHIVE );
	Cvar_SetDescription ("cl_stereo_separation", "Stereo separation used when cl_stereo is enabled.");
	cl_stereo = Cvar_Get( "cl_stereo", "0", 0 );
	Cvar_SetDescription ("cl_stereo", "Enables stereo mode for 3D glasses.");

	cl_add_blend = Cvar_Get ("cl_blend", "1", 0);
	Cvar_SetDescription ("cl_blend", "Enables screen blend effects.");
	cl_add_lights = Cvar_Get ("cl_lights", "1", 0);
	Cvar_SetDescription ("cl_lights", "Enables drawing of dynamic light effects.");
	cl_add_particles = Cvar_Get ("cl_particles", "1", 0);
	Cvar_SetDescription ("cl_particles", "Enables drawing of particles.");
	cl_add_entities = Cvar_Get ("cl_entities", "1", 0);
	Cvar_SetDescription ("cl_entities", "Enables drawing of entities.");
	cl_gun = Cvar_Get ("cl_gun", "1", 0);
	Cvar_SetDescription ("cl_gun", "Enables drawing of first-person gun model.  Set to 0 when taking screenshots.");
	cl_weapon_shells = Cvar_Get ("cl_weapon_shells", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_weapon_shells", "Enables drawing of powerup shells on first-person gun model.");
	cl_footsteps = Cvar_Get ("cl_footsteps", "1", 0);
	Cvar_SetDescription ("cl_footsteps", "Enables player footstep sounds.");

	// reduction factor for particle effects
	cl_particle_scale = Cvar_Get ("cl_particle_scale", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_particle_scale", "Scales down particle effects for better performance.  Higher value = fewer particles.");

	// whether to adjust fov for wide aspect rattio
	cl_widescreen_fov = Cvar_Get ("cl_widescreen_fov", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_widescreen_fov", "Enables automatic scaling of FOV for widescreen video modes.");

	cl_noskins = Cvar_Get ("cl_noskins", "0", 0);
	Cvar_SetDescription ("cl_noskins", "Forces all skins to male/grunt when enabled.");
//	cl_autoskins = Cvar_Get ("cl_autoskins", "0", 0);	// unused
	cl_predict = Cvar_Get ("cl_predict", "1", 0);
	Cvar_SetDescription ("cl_predict", "Enables client-side movement prediction.  Recommended to leave enabled.");
//	cl_minfps = Cvar_Get ("cl_minfps", "5", 0);	// unused
	cl_maxfps = Cvar_Get ("cl_maxfps", "90", 0);
	Cvar_SetDescription ("cl_maxfps", "Shared framerate cap when when cl_async (asynchronous frames) is set to 0.");

#ifdef CLIENT_SPLIT_NETFRAME
	cl_async = Cvar_Get ("cl_async", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_async", "Enables asynchronous frame rendering.  Network frames and renderer frames are separated.  Uses r_maxfps and net_maxfps cvars.");
	net_maxfps = Cvar_Get ("net_maxfps", "60", 0);
	Cvar_SetDescription ("net_maxfps", "Framerate cap for network frames when cl_async (asynchronous frames) is set to 1.");
	r_maxfps = Cvar_Get ("r_maxfps", "125", 0);
	Cvar_SetDescription ("r_maxfps", "Framerate cap for video frames when cl_async (asynchronous frames) is set to 1.");
	r_maxfps_autoset = Cvar_Get ("r_maxfps_autoset", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("r_maxfps_autoset", "Enables automatic setting of framerate cap (r_maxfps) based on refresh rate.  Does nothing when refresh rate is left at default.");
#endif

	cl_sleep = Cvar_Get ("cl_sleep", "1", 0); 
	Cvar_SetDescription ("cl_sleep", "Enables application sleep between render and network frames.  Reduces CPU usage when enabled.");

	// whether to trick version 34 servers that this is a version 34 client
	cl_servertrick = Cvar_Get ("cl_servertrick", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_servertrick", "Enables sending of Vanilla Q2 protocol version to servers.  Set to 1 to connect to non-KMQ2 servers.");

	// Psychospaz's chasecam
	cg_thirdperson = Cvar_Get ("cg_thirdperson", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("cg_thirdperson", "Enables third-person mode.");
	cg_thirdperson_angle = Cvar_Get ("cg_thirdperson_angle", "10", CVAR_ARCHIVE);
	Cvar_SetDescription ("cg_thirdperson_angle", "Sets angle for third-person camera.");
	cg_thirdperson_dist = Cvar_Get ("cg_thirdperson_dist", "50", CVAR_ARCHIVE);
	Cvar_SetDescription ("cg_thirdperson_dist", "Sets distance for third-person camera.");
	cg_thirdperson_offset = Cvar_Get ("cg_thirdperson_offset", "16", CVAR_ARCHIVE);
	Cvar_SetDescription ("cg_thirdperson_offset", "Sets horizontal offset for third-person camera.");
	cg_thirdperson_alpha = Cvar_Get ("cg_thirdperson_alpha", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("cg_thirdperson_alpha", "Enables close-distance alpha fade of player model for third-person camera.");
	cg_thirdperson_chase = Cvar_Get ("cg_thirdperson_chase", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("cg_thirdperson_chase", "Enables chasecam-type third-person camera.");
	cg_thirdperson_adjust = Cvar_Get ("cg_thirdperson_adjust", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("cg_thirdperson_adjust", "Enables view angle adjustment for third-person camera.");
	cg_thirdperson_indemo = Cvar_Get ("cg_thirdperson_indemo", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("cg_thirdperson_indemo", "Enables third-person camera in demos.");
	cg_thirdperson_overhead = Cvar_Get ("cg_thirdperson_overhead", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("cg_thirdperson_overhead", "Enables overhead third-person camera mode.  EXPERIMENTAL!");
	cg_thirdperson_overhead_dist = Cvar_Get ("cg_thirdperson_overhead_dist", "192", CVAR_ARCHIVE);
	Cvar_SetDescription ("cg_thirdperson_overhead_dist", "Sets camera distance for overhead third-person camera mode.");

	cl_blood = Cvar_Get ("cl_blood", "2", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_blood", "Sets blood effect type.  0 = none, 1 = puff, 2 = splat,  3 = bleed, 4 = gore.");

	// Option for old explosions
	cl_old_explosions = Cvar_Get ("cl_old_explosions", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_old_explosions", "Enables old model explosions.");
	// Option for unique plasma explosion sound
	cl_plasma_explo_sound = Cvar_Get ("cl_plasma_explo_sound", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_plasma_explo_sound", "Enables unique explosion sound for Phalanx Cannon.");
	cl_item_bobbing = Cvar_Get ("cl_item_bobbing", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_item_bobbing", "Enables Quake3-style item bobbing.");

	// Psychospaz's changeable rail code
	cl_railred = Cvar_Get ("cl_railred", "20", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_railred", "Sets red color component of railgun trail.  Values range 0-255.");
	cl_railgreen = Cvar_Get ("cl_railgreen", "48", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_railgreen", "Sets green color component of railgun trail.  Values range 0-255.");
	cl_railblue = Cvar_Get ("cl_railblue", "176", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_railblue", "Sets blue color component of railgun trail.  Values range 0-255.");
	cl_railtype = Cvar_Get ("cl_railtype", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_railtype", "Sets type of railgun trail.  0 = original spiral, 1 = solid beam, 2 = devrail.");
	cl_rail_length = Cvar_Get ("cl_rail_length", va("%i", DEFAULT_RAIL_LENGTH), CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_rail_length", "Sets maximum length of railgun trails.");
	cl_rail_space = Cvar_Get ("cl_rail_space", va("%i", DEFAULT_RAIL_SPACE), CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_rail_space", "Sets space between railgun particles.");

	// whether to use texsurfs.txt footstep sounds
	cl_footstep_override = Cvar_Get ("cl_footstep_override", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_footstep_override", "Enables use of texsurfs.txt file for texture-based footstep definitions.");

	// decal control
	r_decals = Cvar_Get ("r_decals", "500", CVAR_ARCHIVE);
	Cvar_SetDescription ("r_decals", "Sets maximum number of decals.");
	r_decal_life = Cvar_Get ("r_decal_life", "1000", CVAR_ARCHIVE);
	Cvar_SetDescription ("r_decal_life", "Sets duration in seconds of decals.");

	con_font_size = Cvar_Get ("con_font_size", "8", CVAR_ARCHIVE);
	Cvar_SetDescription ("con_font_size", "Sets size of console font.  Values > 8 are larger than default, while values < 8 are smaller.");
	alt_text_color = Cvar_Get ("alt_text_color", "2", CVAR_ARCHIVE);
	Cvar_SetDescription ("alt_text_color", "Sets color of high-bit highlighted text.");

	// whether to try to play OGGs instead of CD tracks
	cl_ogg_music = Cvar_Get ("cl_ogg_music", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_ogg_music", "Enables playing of Ogg Vorbis files instead of CD audio tracks.");
	cl_rogue_music = Cvar_Get ("cl_rogue_music", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_rogue_music", "Forces remapping of Ogg Vorbs tracks for Ground Zero mission pack.");
	cl_xatrix_music = Cvar_Get ("cl_xatrix_music", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_xatrix_music", "Forces remapping of Ogg Vorbs tracks for The Reckoning mission pack.");

	cl_upspeed = Cvar_Get ("cl_upspeed", "200", 0);
	Cvar_SetDescription ("cl_upspeed", "Scalar adjustment for jumping/upward swimming sensitiviy.");
	cl_forwardspeed = Cvar_Get ("cl_forwardspeed", "200", 0);
	Cvar_SetDescription ("cl_forwardspeed", "Scalar adjustment for forward movement sensitiviy.");
	cl_sidespeed = Cvar_Get ("cl_sidespeed", "200", 0);
	Cvar_SetDescription ("cl_sidespeed", "Scalar adjustment for sideways movement sensitiviy.");
	cl_yawspeed = Cvar_Get ("cl_yawspeed", "140", 0);
	Cvar_SetDescription ("cl_yawspeed", "Scalar adjustment for view yaw speed.");
	cl_pitchspeed = Cvar_Get ("cl_pitchspeed", "150", 0);
	Cvar_SetDescription ("cl_pitchspeed", "Scalar adjustment for view pitch speed.");
	cl_anglespeedkey = Cvar_Get ("cl_anglespeedkey", "1.5", 0);
	Cvar_SetDescription ("cl_anglespeedkey", "Scalar adjustment for keyboard-based camera control.");

	cl_run = Cvar_Get ("cl_run", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_run", "Enables always-run movement.");
	freelook = Cvar_Get( "freelook", "1", CVAR_ARCHIVE ); // Knightmare changed, was 0
	Cvar_SetDescription ("freelook", "Enables use of mouse for looking around instead of player movement.");
	lookspring = Cvar_Get ("lookspring", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("lookspring", "Enables automatic centering of view when freelook is disabled.");
	lookstrafe = Cvar_Get ("lookstrafe", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("lookstrafe", "Enables lookstrafe mode (horizontal mouse movement as strafing).");
	sensitivity = Cvar_Get ("sensitivity", "3", CVAR_ARCHIVE);
	Cvar_SetDescription ("sensitivity", "Sets in-game mouse sensitivity.");

	m_pitch = Cvar_Get ("m_pitch", "0.022", CVAR_ARCHIVE);
	Cvar_SetDescription ("m_pitch", "Vertical sensitvity of mouse input for freelook.  Is negative for inverted mode.");
	m_yaw = Cvar_Get ("m_yaw", "0.022", 0);
	Cvar_SetDescription ("m_yaw", "Horizontal sensitvity of mouse input for freelook.");
	m_forward = Cvar_Get ("m_forward", "1", 0);
	Cvar_SetDescription ("m_forward", "Forward mouse movement value when freelook is disabled.");
	m_side = Cvar_Get ("m_side", "1", 0);
	Cvar_SetDescription ("m_side", "Sideways mouse movement value when freelook is disabled.");

	cl_shownet = Cvar_Get ("cl_shownet", "0", 0);
	Cvar_SetDescription ("cl_shownet", "Shows verbose output about server packets.  1 will show current message sizes.  2 will show svc_xxx packets as they are parsed.  3 will show verbose information about delta encoding from packet entities.");
	cl_showmiss = Cvar_Get ("cl_showmiss", "0", 0);
	Cvar_SetDescription ("cl_showmiss", "Shows misses on movement prediction.");
	cl_showclamp = Cvar_Get ("showclamp", "0", 0);
	Cvar_SetDescription ("showclamp", "Shows time skews from clients timer versus the servers timer.");
	cl_timeout = Cvar_Get ("cl_timeout", "120", 0);
	Cvar_SetDescription ("cl_timeout", "Timeout (in seconds) for connecting to servers.");
	cl_paused = Cvar_Get ("paused", "0", CVAR_CHEAT);
	Cvar_SetDescription ("paused", "Value that determines if the game is paused.  Considered a client cheat in multiplayer.");
	cl_timedemo = Cvar_Get ("timedemo", "0", CVAR_CHEAT);
	Cvar_SetDescription ("timedemo", "Set to 1 for timing playback of demos.  Useful for old-school bencmarking.");

	rcon_client_password = Cvar_Get ("rcon_password", "", 0);
	Cvar_SetDescription ("rcon_password", "Sets password for rcon commands.");
	rcon_address = Cvar_Get ("rcon_address", "", 0);
	Cvar_SetDescription ("rcon_address", "Sets server address for rcon commands.");

	cl_lightlevel = Cvar_Get ("r_lightlevel", "0", 0);

	//
	// userinfo
	//
	info_password = Cvar_Get ("password", "", CVAR_USERINFO);
	Cvar_SetDescription ("password", "Sets password for multiplayer games.");
	info_spectator = Cvar_Get ("spectator", "0", CVAR_USERINFO);
	Cvar_SetDescription ("spectator", "Sets spectator mode for multiplayer games.");
	name = Cvar_Get ("name", "unnamed", CVAR_USERINFO | CVAR_ARCHIVE);
	Cvar_SetDescription ("name", "Sets player name.");
	skin = Cvar_Get ("skin", "male/grunt", CVAR_USERINFO | CVAR_ARCHIVE);
	Cvar_SetDescription ("skin", "Sets player model and skin, e.g. \"male/grunt\".");
	rate = Cvar_Get ("rate", "25000", CVAR_USERINFO | CVAR_ARCHIVE);	// FIXME
	Cvar_SetDescription ("rate", "Sets network connect rate.  Higher values are recommended for faster connections.");
	msg = Cvar_Get ("msg", "1", CVAR_USERINFO | CVAR_ARCHIVE);
	hand = Cvar_Get ("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	Cvar_SetDescription ("hand", "Sets handedness of player.  0 = right, 1 = left, 2 = center.");
	fov = Cvar_Get ("fov", "90", CVAR_USERINFO | CVAR_ARCHIVE);
	Cvar_SetDescription ("fov", "Field of view of player.  Will be automatically scaled up for widescreen when cl_widescreen_fov is set to 1.");
	gender = Cvar_Get ("gender", "male", CVAR_USERINFO | CVAR_ARCHIVE);
	Cvar_SetDescription ("gender", "Sex of player model.");
	gender_auto = Cvar_Get ("gender_auto", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("gender_auto", "Enables automatic setting of gender based on player model.");
	gender->modified = false; // clear this so we know when user sets it manually

	cl_vwep = Cvar_Get ("cl_vwep", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_vwep", "Enables visible player weapons.");

	// for the server to tell which version the client is
	cl_engine = Cvar_Get ("cl_engine", "KMQuake2", /*CVAR_USERINFO |*/ CVAR_NOSET | CVAR_LATCH);
	Cvar_SetDescription ("cl_engine", "Identifies the client engine.");
	cl_engine_version = Cvar_Get ("cl_engine_version", va("%4.2f",VERSION), /*CVAR_USERINFO |*/ CVAR_NOSET | CVAR_LATCH);
	Cvar_SetDescription ("cl_engine_version", "Identifies the client engine version.");

#ifdef LOC_SUPPORT	// Xile/NiceAss LOC
	cl_drawlocs =		Cvar_Get("cl_drawlocs", "0", 0);
	Cvar_SetDescription ("cl_drawlocs", "Enables drawing of null models for location points.");
	loc_here =			Cvar_Get("loc_here", "", CVAR_NOSET);
	Cvar_SetDescription ("loc_here", "Internal value for loc-based chat.  This is a NOSET value.");
	loc_there =			Cvar_Get("loc_there", "", CVAR_NOSET);
	Cvar_SetDescription ("loc_there", "Internal value for loc-based chat.  This is a NOSET value.");
#endif // LOC_SUPPORT

#ifdef USE_CURL	// HTTP downloading from R1Q2
	cl_http_downloads = Cvar_Get ("cl_http_downloads", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_http_downloads", "Enables HTTP-based downloading.");
	cl_http_filelists = Cvar_Get ("cl_http_filelists", "1", 0);
	Cvar_SetDescription ("cl_http_filelists", "Enables use of filelists for HTTP downloading.");
	cl_http_proxy = Cvar_Get ("cl_http_proxy", "", 0);
	Cvar_SetDescription ("cl_http_proxy", "Sets proxy server for HTTP downloading.");
	cl_http_max_connections = Cvar_Get ("cl_http_max_connections", "4", 0);
	Cvar_SetDescription ("cl_http_max_connections", "Sets max connections for HTTP downloading.");
//	cl_http_max_connections->changed = _cl_http_max_connections_changed;
	cl_http_fallback = Cvar_Get ("cl_http_fallback", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("cl_http_fallback", "Enables fallback to Q2Pro paths and UDP for HTTP downloading.");
#endif	// USE_CURL

	//
	// register our commands
	//
	Cmd_AddCommand ("cmd", CL_ForwardToServer_f);
	Cmd_AddCommand ("pause", CL_Pause_f);
	Cmd_AddCommand ("pingservers", CL_PingServers_f);
	Cmd_AddCommand ("skins", CL_Skins_f);

	Cmd_AddCommand ("userinfo", CL_Userinfo_f);
	Cmd_AddCommand ("snd_restart", CL_Snd_Restart_f);

	Cmd_AddCommand ("changing", CL_Changing_f);
	Cmd_AddCommand ("disconnect", CL_Disconnect_f);
	Cmd_AddCommand ("record", CL_Record_f);
	Cmd_AddCommand ("stop", CL_Stop_f);

	Cmd_AddCommand ("quit", CL_Quit_f);

	Cmd_AddCommand ("connect", CL_Connect_f);
	Cmd_AddCommand ("reconnect", CL_Reconnect_f);

	Cmd_AddCommand ("rcon", CL_Rcon_f);

// 	Cmd_AddCommand ("packet", CL_Packet_f); // this is dangerous to leave in

	Cmd_AddCommand ("setenv", CL_Setenv_f );

	Cmd_AddCommand ("precache", CL_Precache_f);

	Cmd_AddCommand ("download", CL_Download_f);

	Cmd_AddCommand ("writeconfig", CL_WriteConfig_f);

	Cmd_AddCommand ("aacskey", CL_AACSkey_f);

	// Chat Ignore from R1Q2/Q2Pro
	Cmd_AddCommand ("ignorenick", CL_IgnoreChatNick_f);
	Cmd_AddCommand ("unignorenick", CL_UnIgnoreChatNick_f);
	Cmd_AddCommand ("ignoretext", CL_IgnoreChatText_f);
	Cmd_AddCommand ("unignoretext", CL_UnIgnoreChatText_f);
	// end R1Q2/Q2Pro Chat Ignore

#ifdef LOC_SUPPORT	// Xile/NiceAss LOC
	Cmd_AddCommand ("loc_add", CL_AddLoc_f);
	Cmd_AddCommand ("loc_del", CL_DeleteLoc_f);
	Cmd_AddCommand ("loc_save", CL_SaveLoc_f);
	Cmd_AddCommand ("loc_help", CL_LocHelp_f);
#endif	// LOC_SUPPORT

	//
	// forward to server commands
	//
	// the only thing this does is allow command completion
	// to work -- all unknown commands are automatically
	// forwarded to the server
	Cmd_AddCommand ("wave", NULL);
	Cmd_AddCommand ("inven", NULL);
	Cmd_AddCommand ("kill", NULL);
	Cmd_AddCommand ("use", NULL);
	Cmd_AddCommand ("drop", NULL);
	Cmd_AddCommand ("say", NULL);
	Cmd_AddCommand ("say_team", NULL);
	Cmd_AddCommand ("info", NULL);
	Cmd_AddCommand ("prog", NULL);
	Cmd_AddCommand ("give", NULL);
	Cmd_AddCommand ("god", NULL);
	Cmd_AddCommand ("notarget", NULL);
	Cmd_AddCommand ("noclip", NULL);
	Cmd_AddCommand ("invuse", NULL);
	Cmd_AddCommand ("invprev", NULL);
	Cmd_AddCommand ("invnext", NULL);
	Cmd_AddCommand ("invdrop", NULL);
	Cmd_AddCommand ("weapnext", NULL);
	Cmd_AddCommand ("weapprev", NULL);

	// Chat Ignore from R1Q2/Q2Pro
	// Init list pointers
	cl_chatNickIgnores.next = NULL;
	cl_chatNickIgnores.text = NULL;
	cl_chatTextIgnores.next = NULL;
	cl_chatTextIgnores.text = NULL;
	// end R1Q2/Q2Pro Chat Ignore

	local_initialized = true;
}


/*
=================
CL_ShutdownLocal
=================
*/
void CL_ShutdownLocal (void)
{
	if (!local_initialized)
		return;

	Cmd_RemoveCommand ("cmd");
	Cmd_RemoveCommand ("pause");
	Cmd_RemoveCommand ("pingservers");
	Cmd_RemoveCommand ("skins");

	Cmd_RemoveCommand ("userinfo");
	Cmd_RemoveCommand ("snd_restart");

	Cmd_RemoveCommand ("changing");
	Cmd_RemoveCommand ("disconnect");
	Cmd_RemoveCommand ("record");
	Cmd_RemoveCommand ("stop");

	Cmd_RemoveCommand ("quit");

	Cmd_RemoveCommand ("connect");
	Cmd_RemoveCommand ("reconnect");

	Cmd_RemoveCommand ("rcon");

// 	Cmd_RemoveCommand ("packet"); // this is dangerous to leave in

	Cmd_RemoveCommand ("setenv");

	Cmd_RemoveCommand ("precache");

	Cmd_RemoveCommand ("download");

	Cmd_RemoveCommand ("writeconfig");

	Cmd_RemoveCommand ("aacskey");

	// Chat Ignore from R1Q2/Q2Pro
	Cmd_RemoveCommand ("ignorenick");
	Cmd_RemoveCommand ("unignorenick");
	Cmd_RemoveCommand ("ignoretext");
	Cmd_RemoveCommand ("unignoretext");
	// end R1Q2/Q2Pro Chat Ignore

#ifdef LOC_SUPPORT	// Xile/NiceAss LOC
	Cmd_RemoveCommand ("loc_add");
	Cmd_RemoveCommand ("loc_del");
	Cmd_RemoveCommand ("loc_save");
	Cmd_RemoveCommand ("loc_help");
#endif	// LOC_SUPPORT

	Cmd_RemoveCommand ("wave");
	Cmd_RemoveCommand ("inven");
	Cmd_RemoveCommand ("kill");
	Cmd_RemoveCommand ("use");
	Cmd_RemoveCommand ("drop");
	Cmd_RemoveCommand ("say");
	Cmd_RemoveCommand ("say_team");
	Cmd_RemoveCommand ("info");
	Cmd_RemoveCommand ("prog");
	Cmd_RemoveCommand ("give");
	Cmd_RemoveCommand ("god");
	Cmd_RemoveCommand ("notarget");
	Cmd_RemoveCommand ("noclip");
	Cmd_RemoveCommand ("invuse");
	Cmd_RemoveCommand ("invprev");
	Cmd_RemoveCommand ("invnext");
	Cmd_RemoveCommand ("invdrop");
	Cmd_RemoveCommand ("weapnext");
	Cmd_RemoveCommand ("weapprev");

	// Chat Ignore from R1Q2/Q2Pro
	CL_RemoveAllChatIgnores (&cl_chatNickIgnores);
	CL_RemoveAllChatIgnores (&cl_chatTextIgnores);
	// end R1Q2/Q2Pro Chat Ignore

	local_initialized = false;
}


/*
===============
CL_WriteConfiguration

Writes key bindings and archived cvars to config.cfg
===============
*/
qboolean CL_WriteConfiguration (char *cfgName)
{
	FILE	*f;
	char	path[MAX_QPATH];

	if (!cfgName)
		return false;

	if (cls.state == ca_uninitialized)
		return false;

	// Knightmare changed- use separate config for better cohabitation
//	Com_sprintf (path, sizeof(path),"%s/kmq2config.cfg", FS_Savegamedir());
	Com_sprintf (path, sizeof(path),"%s/%s.cfg", FS_Savegamedir(), cfgName);	// was FS_Gamedir()
	f = fopen (path, "w");
	if (!f)
	{	// Knightmare changed- use separate config for better cohabitation
		//Com_Printf ("Couldn't write config.cfg.\n");
	//	Com_Printf ("Couldn't write kmq2config.cfg.\n");
		Com_Printf ("Couldn't write %s.cfg.\n", cfgName);
		return false;
	}

	fprintf (f, "// This file is generated by KMQuake2, do not modify.\n");
	fprintf (f, "// Use autoexec.cfg for adding custom settings.\n");
	Key_WriteBindings (f);
	fclose (f);

	Cvar_WriteVariables (path);

	return true;
}


/*
===============
CL_WriteConfig_f

===============
*/
void CL_WriteConfig_f (void)
{
	char cfgName[MAX_QPATH];

	if (Cmd_Argc() == 1 || Cmd_Argc() == 2)
	{
		if (Cmd_Argc() == 1)
			Com_sprintf (cfgName, sizeof(cfgName), "kmq2config");
		else // if (Cmd_Argc() == 2)
			strncpy (cfgName, Cmd_Argv(1), sizeof(cfgName));

		if (CL_WriteConfiguration (cfgName))
			Com_Printf ("Wrote config file %s/%s.cfg.\n", FS_Savegamedir(), cfgName);	// was FS_Gamedir()
	}
	else
		Com_Printf ("Usage: writeconfig <name>\n");
}

//============================================================================

/*
==================
CL_FixCvarCheats
==================
*/
void CL_FixCvarCheats (void)
{
	if ( !cl.configstrings[CS_MAXCLIENTS][0]
		|| !strcmp(cl.configstrings[CS_MAXCLIENTS], "1") )
	{	// single player can cheat
		Cvar_FixCheatVars (true);
		return;
	}

	// don't allow cheats in multiplayer
	Cvar_FixCheatVars (false);
}


/*
==================
CL_AdvertiseVersion

Adapted from R1Q2
==================
*/
void CL_AdvertiseVersion (void)
{
	char adBuf[128];

	Com_sprintf (adBuf, sizeof(adBuf), "say \"KMQuake2 %4.2f %s %s %s %s [www.markshan.com/knightmare]\"\n",
										VERSION, CPUSTRING, OS_STRING, COMPILETYPE_STRING, __DATE__);
	Cbuf_AddText (adBuf);
	cls.lastAdvertiseTime = cls.realtime;
	cls.advertiseTime = 0;
}


#ifdef CLIENT_SPLIT_NETFRAME
/*
==================
CL_SetFramerateCap

Auto-sets r_maxfps based on r_displayrefresh.
Does nothing if r_displayrefresh is not set.
==================
*/
void CL_SetFramerateCap (void)
{
	int		displayFreq = Cvar_VariableInteger("r_displayrefresh");
	cvar_t	*autoSet = Cvar_Get ("r_maxfps_autoset", "1", CVAR_ARCHIVE);

	if (!autoSet->integer)
		return;

	// if no refresh set, leave framerate cap at default
	if (displayFreq <= 0) {
	//	Cvar_SetInteger ("r_maxfps", 125);	// 8ms frame interval
		return;
	}
	
	// surely refresh rates will never go over 500Hz, right?
	if (displayFreq > 334)
		Cvar_SetInteger ("r_maxfps", 500);	// 2ms frame interval
	else if (displayFreq > 250)
		Cvar_SetInteger ("r_maxfps", 334);	// 3ms frame interval
	else if (displayFreq > 200)
		Cvar_SetInteger ("r_maxfps", 250);	// 4ms frame interval
	else if (displayFreq > 167)
		Cvar_SetInteger ("r_maxfps", 200);	// 5ms frame interval
	else if (displayFreq > 125)
		Cvar_SetInteger ("r_maxfps", 167);	// 6ms frame interval
	else	// 125 fps is default cap
		Cvar_SetInteger ("r_maxfps", 125);	// 8ms frame interval
}


/*
==================
CL_RefreshInputs
==================
*/
static void CL_RefreshInputs (void)
{
	// fetch results from server
	CL_ReadPackets ();

	// get new key events
	Sys_SendKeyEvents ();

	// allow mice or other external controllers to add commands
	IN_Commands ();

	// process console commands
	Cbuf_Execute ();

	// fix any cheating cvars
	CL_FixCvarCheats ();

	// fetch results from server
//	CL_ReadPackets ();

	// Update usercmd state
	if (cls.state > ca_connecting)
		CL_RefreshCmd ();
	else
		CL_RefreshMove ();
}

/*
==================
CL_SendCommand_Async
==================
*/
static void CL_SendCommand_Async (void)
{
	// send intentions now
	CL_SendCmd_Async ();

	// resend a connection request if necessary
	CL_CheckForResend ();
}


/*
==================
CL_Frame_Async
==================
*/
#define FRAMETIME_MAX 0.5 // was 0.2
void CL_Frame_Async (int msec)
{
	static int	packetDelta = 0;
	static int	renderDelta = 0;
	static int	miscDelta = 0;
	static int  lasttimecalled;
	qboolean	packetFrame = true;
	qboolean	renderFrame = true;
	qboolean	miscFrame = true;

	// Don't allow setting maxfps too low or too high
	if (net_maxfps->integer < 10)
		Cvar_SetInteger ("net_maxfps", 10);
	if (net_maxfps->integer > 100)
		Cvar_SetInteger ("net_maxfps", 100);
	if (r_maxfps->integer < 10)
		Cvar_SetInteger ("r_maxfps", 10);
	if (r_maxfps->integer > 1000)
		Cvar_SetInteger ("r_maxfps", 1000);

	packetDelta += msec;
	renderDelta += msec;
	miscDelta += msec;

	// decide the simulation time
	cls.netFrameTime = packetDelta * 0.001f;
	cls.renderFrameTime = renderDelta * 0.001f;
	cl.time += msec;
	cls.realtime = curtime;

	// Don't extrapolate too far ahead
	if (cls.netFrameTime > FRAMETIME_MAX)
		cls.netFrameTime = FRAMETIME_MAX;
	if (cls.renderFrameTime > FRAMETIME_MAX)
		cls.renderFrameTime = FRAMETIME_MAX;

	// If in the debugger last frame, don't timeout
	if (msec > 5000)
		cls.netchan.last_received = Sys_Milliseconds ();

//	if (!cl_timedemo->value)
	if (!cl_timedemo->integer)
	{	// Don't flood packets out while connecting
		if (cls.state == ca_connected && packetDelta < 100)
			packetFrame = false;

		if (packetDelta < 1000.0 / net_maxfps->value)
			packetFrame = false;
		else if (cls.netFrameTime == cls.renderFrameTime)
			packetFrame = false;

		if (renderDelta < 1000.0 / r_maxfps->value)
			renderFrame = false;

		// Stuff that only needs to run at 10FPS
		if (miscDelta < 1000.0 / 10)
			miscFrame = false;
		
		if (!packetFrame && !renderFrame && !cls.forcePacket && !userinfo_modified)
		{	// Pooy's CPU usage fix
		//	if (cl_sleep->value)
			if (cl_sleep->integer)
			{
				int temptime = min( (1000.0 / net_maxfps->value - packetDelta), (1000.0 / r_maxfps->value - renderDelta) );
				if (temptime > 1)
					Sys_Sleep (1);
			} // end CPU usage fix
			return;
		}
		
	}
	else if (msec < 1)	// don't exceed 1000 fps in timedemo mode (fixes hang)
	{
		return;
	}

#ifdef USE_CURL	// HTTP downloading from R1Q2
	if (cls.state == ca_connected) {
		// downloads run full speed when connecting
		CL_RunHTTPDownloads ();
	}
#endif	// USE_CURL

	// Update the inputs (keyboard, mouse, console)
	if (packetFrame || renderFrame)
		CL_RefreshInputs ();

	if (cls.forcePacket || userinfo_modified)
	{
		packetFrame = true;
		cls.forcePacket = false;
	}

	// Send a new command message to the server
	if (packetFrame)
	{
		packetDelta = 0;
		CL_SendCommand_Async ();

#ifdef USE_CURL	// HTTP downloading from R1Q2
		// downloads run less often in game
		CL_RunHTTPDownloads ();
#endif	// USE_CURL
	}
	
	if (renderFrame)
	{
		renderDelta = 0;

		if (miscFrame)
		{
			miscDelta = 0;

			// Let the mouse activate or deactivate
			IN_Frame ();

			// Allow rendering DLL change
			VID_CheckChanges ();
		}
		// Predict all unacknowledged movements
		CL_PredictMovement ();

		if (!cl.refresh_prepped && cls.state == ca_active)
			CL_PrepRefresh ();

		// Predict all unacknowledged movements
	//	CL_PredictMovement ();

		// update the screen
	//	if (host_speeds->value)
		if (host_speeds->integer)
			time_before_ref = Sys_Milliseconds ();
		SCR_UpdateScreen ();
	//	if (host_speeds->value)
		if (host_speeds->integer)
			time_after_ref = Sys_Milliseconds ();

		// Update audio
		S_Update (cl.refdef.vieworg, cl.v_forward, cl.v_right, cl.v_up);
		
		if (miscFrame)
		{
			CDAudio_Update();

			// Advertise engine version, from R1Q2
			if ( (cls.advertiseTime != 0) && (cls.advertiseTime < cls.realtime) )
				CL_AdvertiseVersion ();
		}

		// Advance local effects for next frame
		CL_RunDLights ();
		CL_RunLightStyles ();
		SCR_RunCinematic ();
		SCR_RunConsole ();
		SCR_RunLetterbox ();

		cls.framecount++;

	//	if (log_stats->value)
		if (log_stats->integer)
		{
			if (cls.state == ca_active)
			{
				if (!lasttimecalled)
				{
					lasttimecalled = Sys_Milliseconds();
					if (log_stats_file)
						fprintf( log_stats_file, "0\n" );
				}
				else
				{
					int now = Sys_Milliseconds();

					if (log_stats_file)
						fprintf( log_stats_file, "%d\n", now - lasttimecalled );
					lasttimecalled = now;
				}
			}
		}
	}
}
#endif // CLIENT_SPLIT_NETFRAME


/*
==================
CL_SendCommand

==================
*/
void CL_SendCommand (void)
{
	// get new key events
	Sys_SendKeyEvents ();

	// allow mice or other external controllers to add commands
	IN_Commands ();

	// process console commands
	Cbuf_Execute ();

	// fix any cheating cvars
	CL_FixCvarCheats ();

	// send intentions now
	CL_SendCmd ();

	// resend a connection request if necessary
	CL_CheckForResend ();
}


/*
==================
CL_Frame

==================
*/
void CL_Frame (int msec)
{
	static int	extratime;
	static int  lasttimecalled;

//	if (dedicated->value)
	if (dedicated->integer)
		return;

#ifdef CLIENT_SPLIT_NETFRAME
//	if (cl_async->value && !cl_timedemo->value)
	if (cl_async->integer && !cl_timedemo->value)
	{
		CL_Frame_Async (msec);
		return;
	}
#endif

	extratime += msec;

	// don't allow setting maxfps too low (or game could stop responding)
	// don't allow too high, either
	if (cl_maxfps->integer < 10)
		Cvar_SetInteger ("cl_maxfps", 10);
	if (cl_maxfps->integer > 500)
		Cvar_SetInteger ("cl_maxfps", 500);

//	if (!cl_timedemo->value)
	if (!cl_timedemo->integer)
	{
		if (cls.state == ca_connected && extratime < 100)
			return;			// don't flood packets out while connecting
		if (extratime < 1000.0 / cl_maxfps->value)
		{	
			// Pooy's CPU usage fix
		//	if (cl_sleep->value)
			if (cl_sleep->integer)
			{
				int temptime = 1000 / cl_maxfps->value - extratime;
				if (temptime > 1)
					Sys_Sleep (1);
			} // end CPU usage fix
			return;			// framerate is too high
		}
	}
	else if (extratime < 1)	// don't exceed 1000 fps in timedemo mode (fixes hang)
	{
		return;
	}

	// let the mouse activate or deactivate
	IN_Frame ();

	// decide the simulation time
	cls.netFrameTime = extratime/1000.0;
	cl.time += extratime;
	cls.realtime = curtime;

	extratime = 0;
#if 0
	if (cls.netFrameTime > (1.0 / cl_minfps->value))
		cls.netFrameTime = (1.0 / cl_minfps->value);
#else
	if (cls.netFrameTime > (1.0 / 5))
		cls.netFrameTime = (1.0 / 5);
#endif
	cls.renderFrameTime = cls.netFrameTime;
		
	// clamp this to acceptable values (don't allow infinite particles)
	if (cl_particle_scale->value < 1.0f)
		Cvar_SetValue ("cl_particle_scale", 1);

	// clamp this to acceptable minimum length
	if (cl_rail_length->value < MIN_RAIL_LENGTH)
		Cvar_SetValue ("cl_rail_length", MIN_RAIL_LENGTH);

	// clamp this to acceptable minimum duration
	if (r_decal_life->value < MIN_DECAL_LIFE)
		Cvar_SetValue ("r_decal_life", MIN_DECAL_LIFE);

	// if in the debugger last frame, don't timeout
	if (msec > 5000)
		cls.netchan.last_received = Sys_Milliseconds ();

#ifdef USE_CURL	// HTTP downloading from R1Q2
	CL_RunHTTPDownloads ();
#endif	// USE_CURL

	// fetch results from server
	CL_ReadPackets ();

	// send a new command message to the server
	CL_SendCommand ();

	// predict all unacknowledged movements
	CL_PredictMovement ();

	// allow rendering DLL change
	VID_CheckChanges ();
	if (!cl.refresh_prepped && cls.state == ca_active)
		CL_PrepRefresh ();

	// update the screen
//	if (host_speeds->value)
	if (host_speeds->integer)
		time_before_ref = Sys_Milliseconds ();
	SCR_UpdateScreen ();
//	if (host_speeds->value)
	if (host_speeds->integer)
		time_after_ref = Sys_Milliseconds ();

	// update audio
	S_Update (cl.refdef.vieworg, cl.v_forward, cl.v_right, cl.v_up);
	
	CDAudio_Update();

	// Advertise engine version, from R1Q2
	if ( (cls.advertiseTime != 0) && (cls.advertiseTime < cls.realtime) )
		CL_AdvertiseVersion ();

	// advance local effects for next frame
	CL_RunDLights ();
	CL_RunLightStyles ();
	SCR_RunCinematic ();
	SCR_RunConsole ();
	SCR_RunLetterbox ();

	cls.framecount++;

//	if ( log_stats->value )
	if ( log_stats->integer )
	{
		if ( cls.state == ca_active )
		{
			if ( !lasttimecalled )
			{
				lasttimecalled = Sys_Milliseconds();
				if ( log_stats_file )
					fprintf( log_stats_file, "0\n" );
			}
			else
			{
				int now = Sys_Milliseconds();

				if ( log_stats_file )
					fprintf( log_stats_file, "%d\n", now - lasttimecalled );
				lasttimecalled = now;
			}
		}
	}
}


//============================================================================

/*
====================
CL_Init
====================
*/
void CL_Init (void)
{
//	if (dedicated->value)
	if (dedicated->integer)
		return;		// nothing running on the client

	// all archived variables will now be loaded

	Con_Init ();	
#if defined __linux__ || defined __sgi
	S_Init ();	
	VID_Init ();
#else
	VID_Init ();
	S_Init ();	// sound must be initialized after window is created
#endif
	
	V_Init ();
	
	net_message.data = net_message_buffer;
	net_message.maxsize = sizeof(net_message_buffer);

	UI_Init ();	
	
	SCR_Init ();
	cls.disable_screen = true;	// don't draw yet

	CDAudio_Init ();
	CL_InitLocal ();
	IN_Init ();

#ifdef USE_CURL	// HTTP downloading from R1Q2
	CL_InitHTTPDownloads ();
#endif	// USE_CURL

	//Cbuf_AddText ("exec autoexec.cfg\n");
	FS_ExecAutoexec ();
	Cbuf_Execute ();

}


/*
===============
CL_Shutdown

FIXME: this is a callback from Sys_Quit and Com_Error.  It would be better
to run quit through here before the final handoff to the sys code.
===============
*/
void CL_Shutdown (void)
{
	static qboolean isdown = false;
	int sec, base; 	// zaphster's delay variables

	if (isdown)
	{
		printf ("recursive shutdown\n");
		return;
	}
	isdown = true;

#ifdef USE_CURL	// HTTP downloading from R1Q2
	CL_HTTP_Cleanup (true);
#endif	// USE_CURL

	CL_WriteConfiguration ("kmq2config"); 

	CDAudio_Shutdown ();

	// added delay
	sec = base = Sys_Milliseconds();
	while ((sec - base) < 200)
		sec = Sys_Milliseconds();
	// end delay

	S_Shutdown ();

	// added delay
	sec = base = Sys_Milliseconds();
	while ((sec - base) < 200)
		sec = Sys_Milliseconds();
	// end delay

	V_Shutdown ();

	CL_ShutdownLocal ();	// added Local shutdown
	IN_Shutdown ();
	VID_Shutdown();

	SCR_Shutdown ();		// added SCR shutdown
	UI_Shutdown ();			// added UI shutdown
	Con_Shutdown ();		// added Con shutdown
}
