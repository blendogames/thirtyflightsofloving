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
cvar_t	*cl_autoskins;
cvar_t	*cl_footsteps;
cvar_t	*cl_timeout;
cvar_t	*cl_predict;
//cvar_t	*cl_minfps;
cvar_t	*cl_maxfps;

#ifdef CLIENT_SPLIT_NETFRAME
cvar_t	*cl_async;
cvar_t	*net_maxfps;
cvar_t	*r_maxfps;
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
cvar_t	*menu_sensitivity;
cvar_t	*menu_rotate;
cvar_t	*menu_alpha;

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

client_static_t	cls;
client_state_t	cl;

centity_t		cl_entities[MAX_EDICTS];

entity_state_t	cl_parse_entities[MAX_PARSE_ENTITIES];


float ClampCvar( float min, float max, float value )
{
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}


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

	//	strncpy( buffer, Cmd_Argv(1) );
	//	strncat( buffer, "=" );
		Q_strncpyz( buffer, Cmd_Argv(1), sizeof(buffer) );
		Q_strncatz( buffer, "=", sizeof(buffer) );

		for ( i = 2; i < argc; i++ )
		{
		//	strncat( buffer, Cmd_Argv( i ) );
		//	strncat( buffer, " " );
			Q_strncatz( buffer, Cmd_Argv( i ), sizeof(buffer) );
			Q_strncatz( buffer, " ", sizeof(buffer) );
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

	Cvar_SetValue ("paused", !cl_paused->value);
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
	int		port;

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
	if (cl_servertrick->value && strcmp(cls.servername, "localhost"))
		Netchan_OutOfBandPrint (NS_CLIENT, adr, "connect %i %i %i \"%s\"\n",
			OLD_PROTOCOL_VERSION, port, cls.challenge, Cvar_Userinfo() );
	else
		Netchan_OutOfBandPrint (NS_CLIENT, adr, "connect %i %i %i \"%s\"\n",
			PROTOCOL_VERSION, port, cls.challenge, Cvar_Userinfo() );
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
	Q_strncatz (message, "rcon ", sizeof(message));
	Q_strncatz (message, rcon_client_password->string, sizeof(message));
	Q_strncatz (message, " ", sizeof(message));

	for (i=1 ; i<Cmd_Argc() ; i++)
	{
	//	strncat (message, Cmd_Argv(i));
	//	strncat (message, " ");
		Q_strncatz (message, Cmd_Argv(i), sizeof(message));
		Q_strncatz (message, " ", sizeof(message));
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
	
	NET_SendPacket (NS_CLIENT, strlen(message)+1, message, to);
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
extern	char	*currentweaponmodel;
void CL_Disconnect (void)
{
	byte	final[32];

	if (cls.state == ca_disconnected)
		return;

	if (cl_timedemo && cl_timedemo->value)
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
	Q_strncpyz ((char *)final+1, "disconnect", sizeof(final)-1);
	Netchan_Transmit (&cls.netchan, strlen(final), final);
	Netchan_Transmit (&cls.netchan, strlen(final), final);
	Netchan_Transmit (&cls.netchan, strlen(final), final);

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
	currentweaponmodel = NULL;
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

	l = strlen (in);
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
	int			i;
	netadr_t	adr;
	char		name[32];
	char		*adrstring;
	cvar_t		*noudp;
	cvar_t		*noipx;

	NET_Config (true);		// allow remote

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

		// if the server is using the old protocol,
		// lie to it about this client's protocol
		if (cl_servertrick->value)
			Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", OLD_PROTOCOL_VERSION));
		else
	        Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", PROTOCOL_VERSION));
	}

	noudp = Cvar_Get ("noudp", "0", CVAR_NOSET);
	if (!noudp->value)
	{
        global_udp_server_time = Sys_Milliseconds() ;
		adr.type = NA_BROADCAST;
		adr.port = BigShort(PORT_SERVER);

		// if the server is using the old protocol,
		// lie to it about this client's protocol
		if (cl_servertrick->value)
			Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", OLD_PROTOCOL_VERSION));
		else
			Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", PROTOCOL_VERSION));
	}

	noipx = Cvar_Get ("noipx", "0", CVAR_NOSET);
	if (!noipx->value)
	{
        global_ipx_server_time = Sys_Milliseconds() ;
		adr.type = NA_BROADCAST_IPX;
		adr.port = BigShort(PORT_SERVER);
		// if the server is using the old protocol,
		// lie to it about this client's protocol
		if (cl_servertrick->value)
			Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", OLD_PROTOCOL_VERSION));
		else
			Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", PROTOCOL_VERSION));
	}
}
//</<serverping>>
#else
void CL_PingServers_f (void)
{
	int			i;
	netadr_t	adr;
	char		name[32];
	char		*adrstring;
	cvar_t		*noudp;
	cvar_t		*noipx;

	NET_Config (true);		// allow remote

	// send a broadcast packet
	Com_Printf ("pinging broadcast...\n");

	noudp = Cvar_Get ("noudp", "0", CVAR_NOSET);
	if (!noudp->value)
	{
		adr.type = NA_BROADCAST;
		adr.port = BigShort(PORT_SERVER);
		// if the server is using the old protocol,
		// lie to it about this client's protocol
		if (cl_servertrick->value)
			Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", OLD_PROTOCOL_VERSION));
		else
			Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", PROTOCOL_VERSION));
	}

	noipx = Cvar_Get ("noipx", "0", CVAR_NOSET);
	if (!noipx->value)
	{
		adr.type = NA_BROADCAST_IPX;
		adr.port = BigShort(PORT_SERVER);
		// if the server is using the old protocol,
		// lie to it about this client's protocol
		if (cl_servertrick->value)
			Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", OLD_PROTOCOL_VERSION));
		else
			Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", PROTOCOL_VERSION));
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
		// if the server is using the old protocol,
		// lie to it about this client's protocol
		if (cl_servertrick->value)
			Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", OLD_PROTOCOL_VERSION));
		else
			Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", PROTOCOL_VERSION));
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

	if (gender_auto->value)
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

//
// register our variables
//
	cl_stereo_separation = Cvar_Get( "cl_stereo_separation", "0.4", CVAR_ARCHIVE );
	cl_stereo = Cvar_Get( "cl_stereo", "0", 0 );

	cl_add_blend = Cvar_Get ("cl_blend", "1", 0);
	cl_add_lights = Cvar_Get ("cl_lights", "1", 0);
	cl_add_particles = Cvar_Get ("cl_particles", "1", 0);
	cl_add_entities = Cvar_Get ("cl_entities", "1", 0);
	cl_gun = Cvar_Get ("cl_gun", "1", 0);
	cl_weapon_shells = Cvar_Get ("cl_weapon_shells", "1", CVAR_ARCHIVE);
	cl_footsteps = Cvar_Get ("cl_footsteps", "1", 0);

	// reduction factor for particle effects
	cl_particle_scale = Cvar_Get ("cl_particle_scale", "1", CVAR_ARCHIVE);

	// whether to adjust fov for wide aspect rattio
	cl_widescreen_fov = Cvar_Get ("cl_widescreen_fov", "1", CVAR_ARCHIVE);

	cl_noskins = Cvar_Get ("cl_noskins", "0", 0);
	cl_autoskins = Cvar_Get ("cl_autoskins", "0", 0);
	cl_predict = Cvar_Get ("cl_predict", "1", 0);
//	cl_minfps = Cvar_Get ("cl_minfps", "5", 0);
	cl_maxfps = Cvar_Get ("cl_maxfps", "90", 0);

#ifdef CLIENT_SPLIT_NETFRAME
	cl_async = Cvar_Get ("cl_async", "1", CVAR_ARCHIVE);
	net_maxfps = Cvar_Get ("net_maxfps", "60", 0);
	r_maxfps = Cvar_Get ("r_maxfps", "125", 0);
#endif

	cl_sleep = Cvar_Get ("cl_sleep", "1", 0); 

	// whether to trick version 34 servers that this is a version 34 client
	cl_servertrick = Cvar_Get ("cl_servertrick", "0", CVAR_ARCHIVE);

	// Psychospaz's chasecam
	cg_thirdperson = Cvar_Get ("cg_thirdperson", "0", CVAR_ARCHIVE);
	cg_thirdperson_angle = Cvar_Get ("cg_thirdperson_angle", "10", CVAR_ARCHIVE);
	cg_thirdperson_dist = Cvar_Get ("cg_thirdperson_dist", "50", CVAR_ARCHIVE);
	cg_thirdperson_offset = Cvar_Get ("cg_thirdperson_offset", "16", CVAR_ARCHIVE);
	cg_thirdperson_alpha = Cvar_Get ("cg_thirdperson_alpha", "0", CVAR_ARCHIVE);
	cg_thirdperson_chase = Cvar_Get ("cg_thirdperson_chase", "1", CVAR_ARCHIVE);
	cg_thirdperson_adjust = Cvar_Get ("cg_thirdperson_adjust", "1", CVAR_ARCHIVE);
	cg_thirdperson_indemo = Cvar_Get ("cg_thirdperson_indemo", "0", CVAR_ARCHIVE);
	cg_thirdperson_overhead = Cvar_Get ("cg_thirdperson_overhead", "0", CVAR_ARCHIVE);
	cg_thirdperson_overhead_dist = Cvar_Get ("cg_thirdperson_overhead_dist", "192", CVAR_ARCHIVE);

	cl_blood = Cvar_Get ("cl_blood", "2", CVAR_ARCHIVE);

	// Option for old explosions
	cl_old_explosions = Cvar_Get ("cl_old_explosions", "0", CVAR_ARCHIVE);
	// Option for unique plasma explosion sound
	cl_plasma_explo_sound = Cvar_Get ("cl_plasma_explo_sound", "0", CVAR_ARCHIVE);
	cl_item_bobbing = Cvar_Get ("cl_item_bobbing", "0", CVAR_ARCHIVE);

	// Psychospaz's changeable rail code
	cl_railred = Cvar_Get ("cl_railred", "20", CVAR_ARCHIVE);
	cl_railgreen = Cvar_Get ("cl_railgreen", "50", CVAR_ARCHIVE);
	cl_railblue = Cvar_Get ("cl_railblue", "175", CVAR_ARCHIVE);
	cl_railtype = Cvar_Get ("cl_railtype", "0", CVAR_ARCHIVE);
	cl_rail_length = Cvar_Get ("cl_rail_length", va("%i", DEFAULT_RAIL_LENGTH), CVAR_ARCHIVE);
	cl_rail_space = Cvar_Get ("cl_rail_space", va("%i", DEFAULT_RAIL_SPACE), CVAR_ARCHIVE);

	// whether to use texsurfs.txt footstep sounds
	cl_footstep_override = Cvar_Get ("cl_footstep_override", "1", CVAR_ARCHIVE);

	// decal control
	r_decals = Cvar_Get ("r_decals", "500", CVAR_ARCHIVE);
	r_decal_life = Cvar_Get ("r_decal_life", "1000", CVAR_ARCHIVE);

	con_font_size = Cvar_Get ("con_font_size", "8", CVAR_ARCHIVE);
	alt_text_color = Cvar_Get ("alt_text_color", "2", CVAR_ARCHIVE);

	// whether to try to play OGGs instead of CD tracks
	cl_ogg_music = Cvar_Get ("cl_ogg_music", "1", CVAR_ARCHIVE);
	cl_rogue_music = Cvar_Get ("cl_rogue_music", "0", CVAR_ARCHIVE);
	cl_xatrix_music = Cvar_Get ("cl_xatrix_music", "0", CVAR_ARCHIVE);

	cl_upspeed = Cvar_Get ("cl_upspeed", "200", 0);
	cl_forwardspeed = Cvar_Get ("cl_forwardspeed", "200", 0);
	cl_sidespeed = Cvar_Get ("cl_sidespeed", "200", 0);
	cl_yawspeed = Cvar_Get ("cl_yawspeed", "140", 0);
	cl_pitchspeed = Cvar_Get ("cl_pitchspeed", "150", 0);
	cl_anglespeedkey = Cvar_Get ("cl_anglespeedkey", "1.5", 0);

	cl_run = Cvar_Get ("cl_run", "0", CVAR_ARCHIVE);
	freelook = Cvar_Get( "freelook", "1", CVAR_ARCHIVE ); // Knightmare changed, was 0
	lookspring = Cvar_Get ("lookspring", "0", CVAR_ARCHIVE);
	lookstrafe = Cvar_Get ("lookstrafe", "0", CVAR_ARCHIVE);
	sensitivity = Cvar_Get ("sensitivity", "3", CVAR_ARCHIVE);
	menu_sensitivity = Cvar_Get ("menu_sensitivity", "1", CVAR_ARCHIVE);
	menu_rotate = Cvar_Get ("menu_rotate", "0", CVAR_ARCHIVE);
	menu_alpha = Cvar_Get ("menu_alpha", "0.6", CVAR_ARCHIVE);

	m_pitch = Cvar_Get ("m_pitch", "0.022", CVAR_ARCHIVE);
	m_yaw = Cvar_Get ("m_yaw", "0.022", 0);
	m_forward = Cvar_Get ("m_forward", "1", 0);
	m_side = Cvar_Get ("m_side", "1", 0);

	cl_shownet = Cvar_Get ("cl_shownet", "0", 0);
	cl_showmiss = Cvar_Get ("cl_showmiss", "0", 0);
	cl_showclamp = Cvar_Get ("showclamp", "0", 0);
	cl_timeout = Cvar_Get ("cl_timeout", "120", 0);
	cl_paused = Cvar_Get ("paused", "0", CVAR_CHEAT);
	cl_timedemo = Cvar_Get ("timedemo", "0", CVAR_CHEAT);

	rcon_client_password = Cvar_Get ("rcon_password", "", 0);
	rcon_address = Cvar_Get ("rcon_address", "", 0);

	cl_lightlevel = Cvar_Get ("r_lightlevel", "0", 0);

	//
	// userinfo
	//
	info_password = Cvar_Get ("password", "", CVAR_USERINFO);
	info_spectator = Cvar_Get ("spectator", "0", CVAR_USERINFO);
	name = Cvar_Get ("name", "unnamed", CVAR_USERINFO | CVAR_ARCHIVE);
	skin = Cvar_Get ("skin", "male/grunt", CVAR_USERINFO | CVAR_ARCHIVE);
	rate = Cvar_Get ("rate", "25000", CVAR_USERINFO | CVAR_ARCHIVE);	// FIXME
	msg = Cvar_Get ("msg", "1", CVAR_USERINFO | CVAR_ARCHIVE);
	hand = Cvar_Get ("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	fov = Cvar_Get ("fov", "90", CVAR_USERINFO | CVAR_ARCHIVE);
	gender = Cvar_Get ("gender", "male", CVAR_USERINFO | CVAR_ARCHIVE);
	gender_auto = Cvar_Get ("gender_auto", "1", CVAR_ARCHIVE);
	gender->modified = false; // clear this so we know when user sets it manually

	cl_vwep = Cvar_Get ("cl_vwep", "1", CVAR_ARCHIVE);

	// for the server to tell which version the client is
	cl_engine = Cvar_Get ("cl_engine", "KMQuake2", CVAR_USERINFO | CVAR_NOSET | CVAR_LATCH);
	cl_engine_version = Cvar_Get ("cl_engine_version", va("%4.2f",VERSION), CVAR_USERINFO | CVAR_NOSET | CVAR_LATCH);

#ifdef LOC_SUPPORT	// Xile/NiceAss LOC
	cl_drawlocs =		Cvar_Get("cl_drawlocs", "0", 0);
	loc_here =			Cvar_Get("loc_here", "", CVAR_NOSET);
	loc_there =			Cvar_Get("loc_there", "", CVAR_NOSET);
#endif // LOC_SUPPORT

#ifdef USE_CURL	// HTTP downloading from R1Q2
	cl_http_proxy = Cvar_Get ("cl_http_proxy", "", 0);
	cl_http_filelists = Cvar_Get ("cl_http_filelists", "1", 0);
	cl_http_downloads = Cvar_Get ("cl_http_downloads", "1", CVAR_ARCHIVE);
	cl_http_max_connections = Cvar_Get ("cl_http_max_connections", "4", 0);
//	cl_http_max_connections->changed = _cl_http_max_connections_changed;
	cl_http_fallback = Cvar_Get ("cl_http_fallback", "1", CVAR_ARCHIVE);
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
	if (net_maxfps->value < 10)
		Cvar_SetValue("net_maxfps", 10);
	if (net_maxfps->value > 90)
		Cvar_SetValue("net_maxfps", 90);
	if (r_maxfps->value < 10)
		Cvar_SetValue("r_maxfps", 10);
	if (r_maxfps->value > 1000)
		Cvar_SetValue("r_maxfps", 1000);

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

	if (!cl_timedemo->value)
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
			if (cl_sleep->value)
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
		if (host_speeds->value)
			time_before_ref = Sys_Milliseconds ();
		SCR_UpdateScreen ();
		if (host_speeds->value)
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

		if (log_stats->value)
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

	if (dedicated->value)
		return;

#ifdef CLIENT_SPLIT_NETFRAME
	if (cl_async->value && !cl_timedemo->value)
	{
		CL_Frame_Async (msec);
		return;
	}
#endif

	extratime += msec;

	// don't allow setting maxfps too low (or game could stop responding)
	// don't allow too high, either
	if (cl_maxfps->value < 10)
		Cvar_SetValue("cl_maxfps", 10);
	if (cl_maxfps->value > 500)
		Cvar_SetValue("cl_maxfps", 500);

	if (!cl_timedemo->value)
	{
		if (cls.state == ca_connected && extratime < 100)
			return;			// don't flood packets out while connecting
		if (extratime < 1000.0 / cl_maxfps->value)
		{	
			// Pooy's CPU usage fix
			if (cl_sleep->value)
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
		Cvar_SetValue("cl_particle_scale", 1);

	// clamp this to acceptable minimum length
	if (cl_rail_length->value < MIN_RAIL_LENGTH)
		Cvar_SetValue("cl_rail_length", MIN_RAIL_LENGTH);

	// clamp this to acceptable minimum duration
	if (r_decal_life->value < MIN_DECAL_LIFE)
		Cvar_SetValue("r_decal_life", MIN_DECAL_LIFE);

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
	if (host_speeds->value)
		time_before_ref = Sys_Milliseconds ();
	SCR_UpdateScreen ();
	if (host_speeds->value)
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

	if ( log_stats->value )
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
	if (dedicated->value)
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

	S_Shutdown();

	// added delay
	sec = base = Sys_Milliseconds();
	while ((sec - base) < 200)
		sec = Sys_Milliseconds();
	// end delay

	IN_Shutdown ();
	VID_Shutdown();
}
