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

// ui_joinserver.c -- the join server menu 
// Copyright (C) 2001-2003 pat@aftermoon.net for modif flanked by <serverping>

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"


/*
=============================================================================

JOIN SERVER MENU

=============================================================================
*/
#define MAX_LOCAL_SERVERS 12

static menuframework_s	s_joinserver_menu;
static menuseparator_s	s_joinserver_server_title;

//Knightmare- client compatibility option
static menulist_s		s_joinserver_compatibility_box;
static menuseparator_s	s_joinserver_compat_title;

static menuaction_s		s_joinserver_search_action;
static menuaction_s		s_joinserver_address_book_action;
static menuaction_s		s_joinserver_server_actions[MAX_LOCAL_SERVERS];
static menuaction_s		s_joinserver_back_action;

int		m_num_servers;
#define	NO_SERVER_STRING	"<no server>"

// user readable information
static char local_server_names[MAX_LOCAL_SERVERS][80];

// network address
static netadr_t local_server_netadr[MAX_LOCAL_SERVERS];


void SearchLocalGames( void );

//Knightmare- client compatibility option
static void ClientCompatibilityFunc( void *unused )
{
	Cvar_SetValue( "cl_servertrick", s_joinserver_compatibility_box.curvalue );
}


#if 1
//<serverping> Added code for compute ping time of server broadcasted
// The server is displayed like : 
// "protocol ping hostname mapname nb players/max players"
// "udp 100ms Pat  q2dm1 2/8"

int      global_udp_server_time;
int      global_ipx_server_time;
int      global_adr_server_time[16];
netadr_t global_adr_server_netadr[16];

void UI_AddToServerList (netadr_t adr, char *info)
{
	int		i;
    int     iPing;
    char    *pszProtocol;

	if (m_num_servers == MAX_LOCAL_SERVERS)
		return;
	while ( *info == ' ' )
		info++;

	// Ignore if duplicated
	for (i=0 ; i<m_num_servers ; i++)
        if ( strncmp(info, &local_server_names[i][11], sizeof(local_server_names[0])-10)==0 )
			return;

    iPing = 0 ;
	for (i=0 ; i<MAX_LOCAL_SERVERS ; i++)
    {
        if (    memcmp(&adr.ip, &global_adr_server_netadr[i].ip, sizeof(adr.ip))==0 
             && adr.port == global_adr_server_netadr[i].port )
        {
            // bookmark server
            iPing = Sys_Milliseconds() - global_adr_server_time[i] ;
            pszProtocol = "bkm" ;
            break;
        }
    }
    if ( i == MAX_LOCAL_SERVERS )
    {
        if ( adr.ip[0] > 0 )    // udp server
        {
            iPing = Sys_Milliseconds() - global_udp_server_time ;
            pszProtocol = "UDP" ;
        }
        else                    // ipx server
        {
            iPing = Sys_Milliseconds() - global_ipx_server_time ;
            pszProtocol = "IPX" ;
        }
    }

    Com_sprintf( local_server_names[m_num_servers], 
                 sizeof(local_server_names[0]), 
                 "%s %4dms %s", pszProtocol, iPing, info ) ;

    local_server_netadr[m_num_servers] = adr;

    m_num_servers++;
}
// </serverping>
#else
void UI_AddToServerList (netadr_t adr, char *info)
{
	int		i;

	if (m_num_servers == MAX_LOCAL_SERVERS)
		return;
	while ( *info == ' ' )
		info++;

	// ignore if duplicated
	for (i=0 ; i<m_num_servers ; i++)
		if (!strcmp(info, local_server_names[i]))
			return;

	local_server_netadr[m_num_servers] = adr;
	strncpy (local_server_names[m_num_servers], info, sizeof(local_server_names[0])-1);
	m_num_servers++;
}
#endif

void JoinServerFunc( void *self )
{
	char	buffer[128];
	int		index;

	index = ( menuaction_s * ) self - s_joinserver_server_actions;

	if ( Q_stricmp( local_server_names[index], NO_SERVER_STRING ) == 0 )
		return;

	if (index >= m_num_servers)
		return;

	Com_sprintf (buffer, sizeof(buffer), "connect %s\n", NET_AdrToString (local_server_netadr[index]));
	Cbuf_AddText (buffer);
	UI_ForceMenuOff ();
	cls.disable_screen = 1; // Knightmare- show loading screen
}

void AddressBookFunc( void *self )
{
	M_Menu_AddressBook_f();
}

//Knightmare- init client compatibility menu option
static void JoinserverSetMenuItemValues( void )
{
	Cvar_SetValue( "cl_servertrick", ClampCvar( 0, 1, Cvar_VariableValue("cl_servertrick") ) );
	s_joinserver_compatibility_box.curvalue		= Cvar_VariableValue("cl_servertrick");
}

void NullCursorDraw( void *self )
{
}

void SearchLocalGames( void )
{
	int		i;

	m_num_servers = 0;
	for (i=0 ; i<MAX_LOCAL_SERVERS ; i++)
		strcpy (local_server_names[i], NO_SERVER_STRING);

	Menu_DrawTextBox (168, 192, 36, 3);
	SCR_DrawString (188, 192+MENU_FONT_SIZE, ALIGN_CENTER, S_COLOR_ALT"Searching for local servers, this", 255);
	SCR_DrawString (188, 192+MENU_FONT_SIZE*2, ALIGN_CENTER, S_COLOR_ALT"could take up to a minute, so", 255);
	SCR_DrawString (188, 192+MENU_FONT_SIZE*3, ALIGN_CENTER, S_COLOR_ALT"please be patient.", 255);

	// the text box won't show up unless we do a buffer swap
	GLimp_EndFrame();

	// send out info packets
	CL_PingServers_f();
}

void SearchLocalGamesFunc( void *self )
{
	SearchLocalGames();
}


void JoinServer_MenuInit( void )
{
	int i;
	int y = 0;

	static const char *compatibility_names[] =
	{
		"version 35 (KMQuake2)",
		"version 34 (stock Quake2)",
		0
	};

	JoinserverSetMenuItemValues(); // init item values

	s_joinserver_menu.x = SCREEN_WIDTH*0.5 - 120;
//	s_joinserver_menu.x = viddef.width * 0.50 - SCR_ScaledVideo(120);
	s_joinserver_menu.y = SCREEN_HEIGHT*0.5 - 80;
	s_joinserver_menu.nitems = 0;

	// init client compatibility menu option
	s_joinserver_compat_title.generic.type = MTYPE_SEPARATOR;
	s_joinserver_compat_title.generic.name = "client protocol compatibility";
	s_joinserver_compat_title.generic.x    = 200;
	s_joinserver_compat_title.generic.y	   = y;

	s_joinserver_compatibility_box.generic.type = MTYPE_SPINCONTROL;
	s_joinserver_compatibility_box.generic.name	= "";
	s_joinserver_compatibility_box.generic.x	= -32;
	s_joinserver_compatibility_box.generic.y	= y += MENU_LINE_SIZE;
	s_joinserver_compatibility_box.generic.cursor_offset = -24;
	s_joinserver_compatibility_box.generic.callback = ClientCompatibilityFunc;
	s_joinserver_compatibility_box.generic.statusbar = "set to version 34 to ping non-KMQuake2 servers";
	s_joinserver_compatibility_box.itemnames = compatibility_names;

	s_joinserver_address_book_action.generic.type	= MTYPE_ACTION;
	s_joinserver_address_book_action.generic.name	= "address book";
	s_joinserver_address_book_action.generic.flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_address_book_action.generic.x		= 0;
	s_joinserver_address_book_action.generic.y		= y += 2*MENU_LINE_SIZE;
	s_joinserver_address_book_action.generic.callback = AddressBookFunc;

	s_joinserver_search_action.generic.type = MTYPE_ACTION;
	s_joinserver_search_action.generic.name	= "refresh server list";
	s_joinserver_search_action.generic.flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_search_action.generic.x	= 0;
	s_joinserver_search_action.generic.y	= y += MENU_LINE_SIZE;
	s_joinserver_search_action.generic.callback = SearchLocalGamesFunc;
	s_joinserver_search_action.generic.statusbar = "search for servers";

	s_joinserver_server_title.generic.type = MTYPE_SEPARATOR;
	s_joinserver_server_title.generic.name = "connect to...";
	s_joinserver_server_title.generic.x    = 80;
	s_joinserver_server_title.generic.y	   = y += 2*MENU_LINE_SIZE;

	y += MENU_LINE_SIZE;
	for ( i = 0; i < MAX_LOCAL_SERVERS; i++ )
	{
		s_joinserver_server_actions[i].generic.type	= MTYPE_ACTION;
		strcpy (local_server_names[i], NO_SERVER_STRING);
		s_joinserver_server_actions[i].generic.name	= local_server_names[i];
		s_joinserver_server_actions[i].generic.flags	= QMF_LEFT_JUSTIFY;
		s_joinserver_server_actions[i].generic.x		= 0;
		s_joinserver_server_actions[i].generic.y		= y + i*MENU_LINE_SIZE;
		s_joinserver_server_actions[i].generic.callback = JoinServerFunc;
		s_joinserver_server_actions[i].generic.statusbar = "press ENTER to connect";
	}

	s_joinserver_back_action.generic.type = MTYPE_ACTION;
	s_joinserver_back_action.generic.name	= "back to multiplayer";
	s_joinserver_back_action.generic.flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_back_action.generic.x	= 0;
	s_joinserver_back_action.generic.y	= y += (MAX_LOCAL_SERVERS+2)*MENU_LINE_SIZE;
	s_joinserver_back_action.generic.callback = UI_BackMenu;

	Menu_AddItem( &s_joinserver_menu, &s_joinserver_compat_title );
	Menu_AddItem( &s_joinserver_menu, &s_joinserver_compatibility_box );

	Menu_AddItem( &s_joinserver_menu, &s_joinserver_address_book_action );
	Menu_AddItem( &s_joinserver_menu, &s_joinserver_server_title );
	Menu_AddItem( &s_joinserver_menu, &s_joinserver_search_action );

	for ( i = 0; i < MAX_LOCAL_SERVERS; i++ )
		Menu_AddItem( &s_joinserver_menu, &s_joinserver_server_actions[i] );

	Menu_AddItem( &s_joinserver_menu, &s_joinserver_back_action );

//	Menu_Center( &s_joinserver_menu );

	// skip over compatibility title
	if (s_joinserver_menu.cursor == 0)
		s_joinserver_menu.cursor = 1;

	SearchLocalGames();
}

void JoinServer_MenuDraw(void)
{
	Menu_Banner( "m_banner_join_server" );
	Menu_Draw( &s_joinserver_menu );
}


const char *JoinServer_MenuKey( int key )
{
	return Default_MenuKey( &s_joinserver_menu, key );
}

void M_Menu_JoinServer_f (void)
{
	JoinServer_MenuInit();
	UI_PushMenu( JoinServer_MenuDraw, JoinServer_MenuKey );
}
