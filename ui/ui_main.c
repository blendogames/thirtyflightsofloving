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

// ui_main.c -- menu subsystem functions

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

cvar_t	*ui_sensitivity;
cvar_t	*ui_background_alpha;
cvar_t	*ui_item_rotate;
cvar_t	*ui_cursor_scale;

// moved these here to avoid redundancy
char *menu_null_sound		= "null";
char *menu_in_sound			= "misc/menu1.wav";
char *menu_move_sound		= "misc/menu2.wav";
char *menu_out_sound		= "misc/menu3.wav";
char *menu_drag_sound		= "drag";

qboolean	ui_entersound;		// play after drawing a frame, so caching
								// won't disrupt the sound
qboolean	ui_initialized = false;	// whether UI subsystem has been initialized


/*
=======================================================================

MENU SUBSYSTEM

=======================================================================
*/

/*
=================
UI_Draw
=================
*/
void UI_Draw (void)
{
	if (cls.key_dest != key_menu)
		return;

	// dim everything behind it down
	if (cl.cinematictime > 0 || cls.state == ca_disconnected)
	{
		if (R_DrawFindPic(UI_BACKGROUND_NAME))
		{
		//	R_DrawStretchPic (0, 0, viddef.width, viddef.height, UI_BACKGROUND_NAME, 1.0f);
			R_DrawFill (0, 0, viddef.width, viddef.height, 0, 0, 0, 255);
			UI_DrawPic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ALIGN_CENTER, false, UI_BACKGROUND_NAME, 1.0f);
		}
		else
			R_DrawFill (0,0,viddef.width, viddef.height, 0, 0, 0, 255);
	}
	// ingame menu uses alpha
	else if (R_DrawFindPic(UI_BACKGROUND_NAME))
	//	R_DrawStretchPic (0, 0, viddef.width, viddef.height, UI_BACKGROUND_NAME, ui_background_alpha->value);
		UI_DrawPic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ALIGN_CENTER, false, UI_BACKGROUND_NAME, ui_background_alpha->value);
	else
		R_DrawFill (0, 0, viddef.width, viddef.height, 0, 0, 0, (int)(ui_background_alpha->value*255.0f));

	// Knigthmare- added Psychospaz's mouse support
	UI_RefreshCursorMenu();

	m_drawfunc ();

	// delay playing the enter sound until after the
	// menu has been drawn, to avoid delay while
	// caching images
	if (ui_entersound)
	{
		S_StartLocalSound( menu_in_sound );
		ui_entersound = false;
	}

	// Knigthmare- added Psychospaz's mouse support
	//menu cursor for mouse usage :)
	UI_Draw_Cursor();
}


/*
=================
UI_Keydown
=================
*/
void UI_Keydown (int key)
{
	const char *s;

	if (m_keyfunc)
		if ( ( s = m_keyfunc( key ) ) != 0 )
			S_StartLocalSound( ( char * ) s );
}


/*
=================
UI_RootMenu
=================
*/
void UI_RootMenu (void)
{
	Menu_Main_f ();
}


/*
=================
UI_Precache
=================
*/
void UI_Precache (void)
{
	int		i;
	char	scratch[80];

	// general images
	R_DrawFindPic (LOADSCREEN_NAME); 
	R_DrawFindPic (UI_BACKGROUND_NAME); 
	R_DrawFindPic (UI_NOSCREEN_NAME); 

	// loadscreen images
	R_DrawFindPic ("/pics/loading.pcx");
	R_DrawFindPic ("/pics/loading_bar.pcx");
	R_DrawFindPic ("/pics/downloading.pcx");
	R_DrawFindPic ("/pics/downloading_bar.pcx");
	R_DrawFindPic ("/pics/loading_led1.pcx");

	// cursors
//	R_DrawFindPic (UI_MOUSECURSOR_MAIN_PIC);
//	R_DrawFindPic (UI_MOUSECURSOR_HOVER_PIC);
//	R_DrawFindPic (UI_MOUSECURSOR_CLICK_PIC);
//	R_DrawFindPic (UI_MOUSECURSOR_OVER_PIC);
//	R_DrawFindPic (UI_MOUSECURSOR_TEXT_PIC);
	R_DrawFindPic (UI_MOUSECURSOR_PIC);
	R_DrawFindPic (UI_ITEMCURSOR_DEFAULT_PIC);
	R_DrawFindPic (UI_ITEMCURSOR_KEYBIND_PIC);
	R_DrawFindPic (UI_ITEMCURSOR_BLINK_PIC);

	for (i = 0; i < NUM_MAINMENU_CURSOR_FRAMES; i++) {
		Com_sprintf (scratch, sizeof(scratch), "/pics/m_cursor%d.pcx", i);
		R_DrawFindPic (scratch);
	}

	// main menu items
	R_DrawFindPic ("/pics/m_main_game.pcx");
	R_DrawFindPic ("/pics/m_main_game_sel.pcx");
	R_DrawFindPic ("/pics/m_main_multiplayer.pcx");
	R_DrawFindPic ("/pics/m_main_multiplayer_sel.pcx");
	R_DrawFindPic ("/pics/m_main_options.pcx");
	R_DrawFindPic ("/pics/m_main_options_sel.pcx");
	R_DrawFindPic ("/pics/m_main_video.pcx");
	R_DrawFindPic ("/pics/m_main_video_sel.pcx");
//	R_DrawFindPic ("/pics/m_main_mods.pcx");
//	R_DrawFindPic ("/pics/m_main_mods_sel.pcx");
	R_DrawFindPic ("/pics/m_main_quit.pcx");
	R_DrawFindPic ("/pics/m_main_quit_sel.pcx");
	R_DrawFindPic ("/pics/m_main_plaque.pcx");
	R_DrawFindPic ("/pics/m_main_logo.pcx");
	R_RegisterModel ("models/ui/quad_cursor.md2");

	// menu banners
	R_DrawFindPic ("/pics/m_banner_game.pcx");
	R_DrawFindPic ("/pics/m_banner_load_game.pcx");
	R_DrawFindPic ("/pics/m_banner_save_game.pcx");
	R_DrawFindPic ("/pics/m_banner_multiplayer.pcx");
	R_DrawFindPic ("/pics/m_banner_join_server.pcx");
	R_DrawFindPic ("/pics/m_banner_addressbook.pcx");
	R_DrawFindPic ("/pics/m_banner_start_server.pcx");
	R_DrawFindPic ("/pics/m_banner_plauer_setup.pcx"); // typo for image name is id's fault
	R_DrawFindPic ("/pics/m_banner_options.pcx");
	R_DrawFindPic ("/pics/m_banner_customize.pcx");
	R_DrawFindPic ("/pics/m_banner_video.pcx");
//	R_DrawFindPic ("/pics/m_banner_mods.pcx");
	R_DrawFindPic ("/pics/quit.pcx");
//	R_DrawFindPic ("/pics/areyousure.pcx");
//	R_DrawFindPic ("/pics/yn.pcx");

	// GUI elements
	R_DrawFindPic ("/gfx/ui/widgets/listbox_background.pcx");
//	R_DrawFindPic (UI_CHECKBOX_ON_PIC);
//	R_DrawFindPic (UI_CHECKBOX_OFF_PIC);
	R_DrawFindPic (UI_FIELD_PIC);
	R_DrawFindPic (UI_TEXTBOX_PIC);
	R_DrawFindPic (UI_SLIDER_PIC);
	R_DrawFindPic (UI_ARROWS_PIC);

//	R_DrawFindPic ("/gfx/ui/listbox_background.pcx");
//	R_DrawFindPic ("/gfx/ui/arrows/arrow_left.pcx");
//	R_DrawFindPic ("/gfx/ui/arrows/arrow_left_d.pcx");
//	R_DrawFindPic ("/gfx/ui/arrows/arrow_right.pcx");
//	R_DrawFindPic ("/gfx/ui/arrows/arrow_right_d.pcx"); 
}


/*
=================
UI_Init
=================
*/
void UI_Init (void)
{
	// init this cvar here so M_Print can use it
	if (!alt_text_color)
		alt_text_color = Cvar_Get ("alt_text_color", "2", CVAR_ARCHIVE);

	ui_sensitivity = Cvar_Get ("ui_sensitivity", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("ui_sensitivity", "Sets sensitvity of mouse in menus.");
	ui_background_alpha = Cvar_Get ("ui_background_alpha", "0.6", CVAR_ARCHIVE);
	Cvar_SetDescription ("ui_background_alpha", "Sets opacity of background menu image when ingame.");
	ui_item_rotate = Cvar_Get ("ui_item_rotate", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("ui_item_rotate", "Reverses direction of mouse click rotation for menu lists.");
	ui_cursor_scale = Cvar_Get ("ui_cursor_scale", "0.4", 0);
	Cvar_SetDescription ("ui_cursor_scale", "Sets scale for drawing the menu mouse cursor.");

	UI_LoadFontNames ();	// load font list
//	UI_LoadHudNames ();		// load hud list
	UI_LoadCrosshairs ();	// load crosshairs
	UI_InitServerList ();	// init join server list
	UI_LoadMapList();		// load map list
	UI_LoadPlayerModels (); // load player models
	UI_InitSavegameData ();	// load savegame data

	UI_Precache ();		// precache images

	Cmd_AddCommand ("menu_main", Menu_Main_f);
	Cmd_AddCommand ("menu_game", Menu_Game_f);
		Cmd_AddCommand ("menu_loadgame", Menu_LoadGame_f);
		Cmd_AddCommand ("menu_savegame", Menu_SaveGame_f);
		Cmd_AddCommand ("menu_credits", Menu_Credits_f );
	Cmd_AddCommand ("menu_multiplayer", Menu_Multiplayer_f );
		Cmd_AddCommand ("menu_joinserver", Menu_JoinServer_f);
			Cmd_AddCommand ("menu_addressbook", Menu_AddressBook_f);
		Cmd_AddCommand ("menu_startserver", Menu_StartServer_f);
			Cmd_AddCommand ("menu_dmoptions", Menu_DMOptions_f);
		Cmd_AddCommand ("menu_playerconfig", Menu_PlayerConfig_f);
		Cmd_AddCommand ("menu_downloadoptions", Menu_DownloadOptions_f);
	Cmd_AddCommand ("menu_video", Menu_Video_f);
		Cmd_AddCommand ("menu_video_advanced", Menu_Video_Advanced_f);
	Cmd_AddCommand ("menu_options", Menu_Options_f);
		Cmd_AddCommand ("menu_sound", Menu_Options_Sound_f);
		Cmd_AddCommand ("menu_controls", Menu_Options_Controls_f);
			Cmd_AddCommand ("menu_keys", Menu_Keys_f);
		Cmd_AddCommand ("menu_screen", Menu_Options_Screen_f);
		Cmd_AddCommand ("menu_effects", Menu_Options_Effects_f);
		Cmd_AddCommand ("menu_interface", Menu_Options_Interface_f);
	Cmd_AddCommand ("menu_quit", Menu_Quit_f);

	ui_initialized = true;
}


/*
=================
UI_Shutdown
=================
*/
void UI_Shutdown (void)
{
	// Don't shutdown if not initialized
	// Fixes errors in dedicated console
	if (!ui_initialized)
		return;

	UI_FreeFontNames ();
//	UI_FreeHudNames ();
	UI_FreeCrosshairs ();
	UI_FreeMapList ();
	UI_FreePlayerModels ();

	Cmd_RemoveCommand ("menu_main");
	Cmd_RemoveCommand ("menu_game");
	Cmd_RemoveCommand ("menu_loadgame");
	Cmd_RemoveCommand ("menu_savegame");
	Cmd_RemoveCommand ("menu_credits");
	Cmd_RemoveCommand ("menu_multiplayer");
	Cmd_RemoveCommand ("menu_joinserver");
	Cmd_RemoveCommand ("menu_addressbook");
	Cmd_RemoveCommand ("menu_startserver");
	Cmd_RemoveCommand ("menu_dmoptions");
	Cmd_RemoveCommand ("menu_playerconfig");
	Cmd_RemoveCommand ("menu_downloadoptions");
	Cmd_RemoveCommand ("menu_video");
	Cmd_RemoveCommand ("menu_video_advanced");
	Cmd_RemoveCommand ("menu_options");
	Cmd_RemoveCommand ("menu_sound");
	Cmd_RemoveCommand ("menu_controls");
	Cmd_RemoveCommand ("menu_keys");
	Cmd_RemoveCommand ("menu_screen");
	Cmd_RemoveCommand ("menu_effects");
	Cmd_RemoveCommand ("menu_interface");
	Cmd_RemoveCommand ("menu_quit");

	ui_initialized = false;
}
