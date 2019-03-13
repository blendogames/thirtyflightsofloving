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

// ui_game.c -- the single player menu and credits

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

/*
=============================================================================

GAME MENU

=============================================================================
*/

static int		m_game_cursor;

static menuframework_s	s_game_menu;
static menuaction_s		s_easy_game_action;
static menuaction_s		s_medium_game_action;
static menuaction_s		s_hard_game_action;
static menuaction_s		s_nitemare_game_action;
static menuaction_s		s_load_game_action;
static menuaction_s		s_save_game_action;
static menuaction_s		s_credits_action;
static menuseparator_s	s_blankline;
static menuaction_s		s_game_back_action;

static void StartGame( void )
{
	// disable updates and start the cinematic going
	cl.servercount = -1;
	UI_ForceMenuOff ();
	Cvar_SetValue( "deathmatch", 0 );
	Cvar_SetValue( "coop", 0 );
	Cvar_SetValue( "gamerules", 0 );		//PGM

//	Cbuf_AddText ("loading ; killserver ; wait ; newgame\n");
	if (cls.state != ca_disconnected) // don't force loadscreen if disconnected
		Cbuf_AddText ("loading ; killserver ; wait\n");
	Cbuf_AddText ("newgame\n");
	cls.key_dest = key_game;
}

static void EasyGameFunc( void *data )
{
	Cvar_ForceSet( "skill", "0" );
	StartGame();
}

static void MediumGameFunc( void *data )
{
	Cvar_ForceSet( "skill", "1" );
	StartGame();
}

static void HardGameFunc( void *data )
{
	Cvar_ForceSet( "skill", "2" );
	StartGame();
}

static void NitemareGameFunc( void *data )
{
	Cvar_ForceSet( "skill", "3" );
	StartGame();
}

static void LoadGameFunc( void *unused )
{
	M_Menu_LoadGame_f ();
}

static void SaveGameFunc( void *unused )
{
	M_Menu_SaveGame_f();
}

static void CreditsFunc( void *unused )
{
	M_Menu_Credits_f();
}

void Game_MenuInit( void )
{
	static const char *difficulty_names[] =
	{
		"easy",
		"medium",
		"hard",
		0
	};
	int y = 0;

	s_game_menu.x = SCREEN_WIDTH*0.5 - 24;
	//s_game_menu.y = 0;
	s_game_menu.nitems = 0;

	s_easy_game_action.generic.type	= MTYPE_ACTION;
	s_easy_game_action.generic.flags  = QMF_LEFT_JUSTIFY;
	s_easy_game_action.generic.x		= 0;
	s_easy_game_action.generic.y		= y; // 0
	s_easy_game_action.generic.name	= " easy";
	s_easy_game_action.generic.callback = EasyGameFunc;

	s_medium_game_action.generic.type	= MTYPE_ACTION;
	s_medium_game_action.generic.flags  = QMF_LEFT_JUSTIFY;
	s_medium_game_action.generic.x		= 0;
	s_medium_game_action.generic.y		= y += MENU_LINE_SIZE;
	s_medium_game_action.generic.name	= " medium";
	s_medium_game_action.generic.callback = MediumGameFunc;

	s_hard_game_action.generic.type	= MTYPE_ACTION;
	s_hard_game_action.generic.flags  = QMF_LEFT_JUSTIFY;
	s_hard_game_action.generic.x		= 0;
	s_hard_game_action.generic.y		= y += MENU_LINE_SIZE;
	s_hard_game_action.generic.name	= " hard";
	s_hard_game_action.generic.callback = HardGameFunc;

	s_nitemare_game_action.generic.type	= MTYPE_ACTION;
	s_nitemare_game_action.generic.flags  = QMF_LEFT_JUSTIFY;
	s_nitemare_game_action.generic.x		= 0;
	s_nitemare_game_action.generic.y		= y += MENU_LINE_SIZE;
	s_nitemare_game_action.generic.name	= " nightmare";
	s_nitemare_game_action.generic.callback = NitemareGameFunc;


	s_blankline.generic.type = MTYPE_SEPARATOR;

	s_load_game_action.generic.type	= MTYPE_ACTION;
	s_load_game_action.generic.flags  = QMF_LEFT_JUSTIFY;
	s_load_game_action.generic.x		= 0;
	s_load_game_action.generic.y		= y += 2*MENU_LINE_SIZE;
	s_load_game_action.generic.name	= " load game";
	s_load_game_action.generic.callback = LoadGameFunc;

	s_save_game_action.generic.type	= MTYPE_ACTION;
	s_save_game_action.generic.flags  = QMF_LEFT_JUSTIFY;
	s_save_game_action.generic.x		= 0;
	s_save_game_action.generic.y		= y += MENU_LINE_SIZE;
	s_save_game_action.generic.name	= " save game";
	s_save_game_action.generic.callback = SaveGameFunc;

	s_credits_action.generic.type	= MTYPE_ACTION;
	s_credits_action.generic.flags  = QMF_LEFT_JUSTIFY;
	s_credits_action.generic.x		= 0;
	s_credits_action.generic.y		= y += MENU_LINE_SIZE;
	s_credits_action.generic.name	= " credits";
	s_credits_action.generic.callback = CreditsFunc;

	s_game_back_action.generic.type	= MTYPE_ACTION;
	s_game_back_action.generic.flags  = QMF_LEFT_JUSTIFY;
	s_game_back_action.generic.x		= 0;
	s_game_back_action.generic.y		= y += 2*MENU_LINE_SIZE;
	s_game_back_action.generic.name	= " back to main";
	s_game_back_action.generic.callback = UI_BackMenu;

	Menu_AddItem( &s_game_menu, ( void * ) &s_easy_game_action );
	Menu_AddItem( &s_game_menu, ( void * ) &s_medium_game_action );
	Menu_AddItem( &s_game_menu, ( void * ) &s_hard_game_action );
	Menu_AddItem( &s_game_menu, ( void * ) &s_nitemare_game_action );

	Menu_AddItem( &s_game_menu, ( void * ) &s_blankline );

	Menu_AddItem( &s_game_menu, ( void * ) &s_load_game_action );
	Menu_AddItem( &s_game_menu, ( void * ) &s_save_game_action );

	Menu_AddItem( &s_game_menu, ( void * ) &s_blankline );
	Menu_AddItem( &s_game_menu, ( void * ) &s_credits_action );

	Menu_AddItem( &s_game_menu, ( void * ) &s_game_back_action );

	Menu_Center( &s_game_menu );
}

void Game_MenuDraw( void )
{
	Menu_DrawBanner( "m_banner_game" );
	Menu_AdjustCursor( &s_game_menu, 1 );
	Menu_Draw( &s_game_menu );
}

const char *Game_MenuKey( int key )
{
	return Default_MenuKey( &s_game_menu, key );
}

void M_Menu_Game_f (void)
{
	Game_MenuInit();
	UI_PushMenu( Game_MenuDraw, Game_MenuKey );
	m_game_cursor = 1;
}
