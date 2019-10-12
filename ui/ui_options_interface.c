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

// ui_options_interface.c -- the interface options menu

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

/*
=======================================================================

INTERFACE MENU

=======================================================================
*/

static menuframework_s	s_options_interface_menu;
static menuseparator_s	s_options_interface_header;
static menuslider_s		s_options_interface_conalpha_slider;
//static menuslider_s		s_options_interface_conheight_slider;
static menuslider_s		s_options_interface_menumouse_slider;
static menuslider_s		s_options_interface_menualpha_slider;
static menulist_s		s_options_interface_font_box;
static menuslider_s		s_options_interface_fontsize_slider;
static menulist_s		s_options_interface_alt_text_color_box;
static menulist_s		s_options_interface_simple_loadscreen_box;
static menulist_s		s_options_interface_newconback_box;
static menulist_s		s_options_interface_noalttab_box;
static menuaction_s		s_options_interface_defaults_action;
static menuaction_s		s_options_interface_back_action;


static void MouseMenuFunc( void *unused )
{
	Cvar_SetValue( "menu_sensitivity", s_options_interface_menumouse_slider.curvalue / 4.0F );
}

// menu alpha option
static void MenuAlphaFunc( void *unused )
{
	Cvar_SetValue( "menu_alpha", s_options_interface_menualpha_slider.curvalue / 20.0F);
}

static void AltTextColorFunc( void *unused )
{
	Cvar_SetValue( "alt_text_color", s_options_interface_alt_text_color_box.curvalue);
}

// Psychospaz's transparent console
static void ConAlphaFunc( void *unused )
{
	Cvar_SetValue( "con_alpha", s_options_interface_conalpha_slider.curvalue * 0.05 );
}

// variable console height
/*static void ConHeightFunc( void *unused )
{
	Cvar_SetValue( "con_height", 0.25 + (s_options_interface_conheight_slider.curvalue * 0.05) );
}*/

static void SimpleLoadscreenFunc( void *unused )
{
	Cvar_SetValue( "scr_simple_loadscreen", s_options_interface_simple_loadscreen_box.curvalue );
}

static void NewConbackFunc( void *unused )
{
	Cvar_SetValue( "con_newconback", s_options_interface_newconback_box.curvalue );
}

static void NoAltTabFunc( void *unused )
{
	Cvar_SetValue( "win_noalttab", s_options_interface_noalttab_box.curvalue );
}

/*
=======================================================================
Font loading
=======================================================================
*/
cvar_t *con_font;
#define MAX_FONTS 32
char **font_names = NULL;
int	numfonts = 0;

static void FontSizeFunc( void *unused )
{
	Cvar_SetValue( "con_font_size", s_options_interface_fontsize_slider.curvalue * 2 );
}

static void FontFunc( void *unused )
{
	Cvar_Set( "con_font", font_names[s_options_interface_font_box.curvalue] );
}

void SetFontCursor (void)
{
	int i;
	s_options_interface_font_box.curvalue = 0;

	if (!con_font)
		con_font = Cvar_Get ("con_font", "default", CVAR_ARCHIVE);

	if (numfonts > 1)
		for (i=0; font_names[i]; i++)
		{
			if (!Q_strcasecmp(con_font->string, font_names[i]))
			{
				s_options_interface_font_box.curvalue = i;
				return;
			}
		}
}


void insertFont (char **list, char *insert, int len )
{
	int i, j;
	if (!list) return;

	// i=1 so default stays first!
	for (i=1; i<len; i++)
	{
		if (!list[i])
			break;

		if (strcmp( list[i], insert ))
		{
			for (j=len; j>i ;j--)
				list[j] = list[j-1];

			list[i] = strdup(insert);
			return;
		}
	}
	list[len] = strdup(insert);
}

char **SetFontNames (void)
{
	char *curFont;
	char **list = 0, *p;//, *s;
//	char findname[1024];
	int nfonts = 0, nfontnames;
	char **fontfiles;
	char *path = NULL;
	int i;//, j;

	list = malloc( sizeof( char * ) * MAX_FONTS );
	memset( list, 0, sizeof( char * ) * MAX_FONTS );

	list[0] = strdup("default");

	nfontnames = 1;

#if 1
	fontfiles = FS_GetFileList("fonts/*.*", NULL, &nfonts);
	for (i=0;i<nfonts && nfontnames<MAX_FONTS;i++)
	{
		int num;

		if (!fontfiles || !fontfiles[i])	// Knightmare added array base check
			continue;

		if ( !IsValidImageFilename(fontfiles[i]) )
			continue;

		p = strrchr(fontfiles[i], '/'); p++;

		num = strlen(p)-4;
		p[num] = 0; // NULL

		curFont = p;

		if (!FS_ItemInList(curFont, nfontnames, list))
		{
			// insertFont not needed due to sorting in FS_GetFileList()
		//	insertFont (list, strdup(curFont), nfontnames);
			FS_InsertInList(list, strdup(curFont), nfontnames, 1);	// start=1 so default stays first!
			nfontnames++;
		}
		
		// set back so whole string get deleted.
		p[num] = '.';
	}
#else
	path = FS_NextPath( path );
	while (path) 
	{
		Com_sprintf( findname, sizeof(findname), "%s/fonts/*.*", path );
		fontfiles = FS_ListFiles( findname, &nfonts, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM );

		for (i=0;i<nfonts && nfontnames<MAX_FONTS;i++)
		{
			int num;

			if (!fontfiles || !fontfiles[i])	// Knightmare added array base check
				continue;

			if ( !IsValidImageFilename(fontfiles[i]) )
				continue;

			p = strrchr(fontfiles[i], '/'); p++;

			num = strlen(p)-4;
			p[num] = 0;//NULL;

			curFont = p;

			if (!FS_ItemInList(curFont, nfontnames, list))
			{
				insertFont (list, strdup(curFont), nfontnames);
				nfontnames++;
			}
			
			// set back so whole string get deleted.
			p[num] = '.';
		}
		if (nfonts)
			FS_FreeFileList( fontfiles, nfonts );
		
		path = FS_NextPath( path );
	}

	// check pak after
	if (fontfiles = FS_ListPak("fonts/", &nfonts))
	{
		for (i=0;i<nfonts && nfontnames<MAX_FONTS;i++)
		{
			int num;

			if (!fontfiles || !fontfiles[i])	// Knightmare added array base check
				continue;

			if ( !IsValidImageFilename(fontfiles[i]) )
				continue;

			p = strrchr(fontfiles[i], '/'); p++;

			num = strlen(p)-4;
			p[num] = 0; // NULL

			curFont = p;

			if (!FS_ItemInList(curFont, nfontnames, list))
			{
				insertFont(list, strdup(curFont), nfontnames);
				nfontnames++;
			}
			
			// set back so whole string get deleted.
			p[num] = '.';
		}
	}
#endif

	if (nfonts)
		FS_FreeFileList( fontfiles, nfonts );

	numfonts = nfontnames;

	return list;		
}

//=======================================================================

static void InterfaceSetMenuItemValues( void )
{
	SetFontCursor();

	s_options_interface_menumouse_slider.curvalue		= ( Cvar_VariableValue("menu_sensitivity") ) * 4;
	s_options_interface_menualpha_slider.curvalue		= ( Cvar_VariableValue("menu_alpha") ) * 20;
	s_options_interface_fontsize_slider.curvalue		= ( Cvar_VariableValue("con_font_size") ) * 0.5;

	Cvar_SetValue( "alt_text_color", ClampCvar( 0, 9, Cvar_VariableValue("alt_text_color") ) );
	s_options_interface_alt_text_color_box.curvalue	= Cvar_VariableValue("alt_text_color");

	Cvar_SetValue( "con_alpha", ClampCvar( 0, 1, Cvar_VariableValue("con_alpha") ) );
	s_options_interface_conalpha_slider.curvalue		= ( Cvar_VariableValue("con_alpha") ) * 20;

	//Cvar_SetValue( "con_height", ClampCvar( 0.25, 0.75, Cvar_VariableValue("con_height") ) );
	//s_options_interface_conheight_slider.curvalue		= 20 * (Cvar_VariableValue("con_height") - 0.25);

	Cvar_SetValue( "scr_simple_loadscreen", ClampCvar( 0, 1, Cvar_VariableValue("scr_simple_loadscreen") ) );
	s_options_interface_simple_loadscreen_box.curvalue = Cvar_VariableValue("scr_simple_loadscreen");

	Cvar_SetValue( "con_newconback", ClampCvar( 0, 1, Cvar_VariableValue("con_newconback") ) );
	s_options_interface_newconback_box.curvalue = Cvar_VariableValue("con_newconback");

	s_options_interface_noalttab_box.curvalue			= Cvar_VariableValue("win_noalttab");
}

static void InterfaceResetDefaultsFunc ( void *unused )
{
	Cvar_SetToDefault ("menu_sensitivity");	
	Cvar_SetToDefault ("menu_alpha");	
	Cvar_SetToDefault ("con_font");	
	Cvar_SetToDefault ("con_font_size");	
	Cvar_SetToDefault ("alt_text_color");	
	Cvar_SetToDefault ("con_alpha");	
//	Cvar_SetToDefault ("con_height");	
	Cvar_SetToDefault ("scr_simple_loadscreen");
	Cvar_SetToDefault ("con_newconback");
	Cvar_SetToDefault ("win_noalttab");	

	InterfaceSetMenuItemValues();
}

void Options_Interface_MenuInit ( void )
{
	static const char *yesno_names[] =
	{
		"no",
		"yes",
		0
	};

	static const char *textcolor_names[] =
	{
		"gray",
		"red",
		"green",
		"yellow",
		"blue",
		"cyan",
		"magenta",
		"white",
		"black",
		"orange",
		0
	};

	int y = 3*MENU_LINE_SIZE;

	s_options_interface_menu.x = SCREEN_WIDTH*0.5;
	s_options_interface_menu.y = SCREEN_HEIGHT*0.5 - 58;
	s_options_interface_menu.nitems = 0;

	s_options_interface_header.generic.type		= MTYPE_SEPARATOR;
	s_options_interface_header.generic.name		= "Interface";
	s_options_interface_header.generic.x		= MENU_FONT_SIZE/2 * strlen(s_options_interface_header.generic.name);
	s_options_interface_header.generic.y		= 0;

	// Knightmare- Psychospaz's menu mouse support
	s_options_interface_menumouse_slider.generic.type		= MTYPE_SLIDER;
	s_options_interface_menumouse_slider.generic.x			= 0;
	s_options_interface_menumouse_slider.generic.y			= y;
	s_options_interface_menumouse_slider.generic.name		= "mouse speed";
	s_options_interface_menumouse_slider.generic.callback	= MouseMenuFunc;
	s_options_interface_menumouse_slider.minvalue			= 1;
	s_options_interface_menumouse_slider.maxvalue			= 8;
	s_options_interface_menumouse_slider.generic.statusbar	= "changes sensitivity of mouse in menus";

	s_options_interface_menualpha_slider.generic.type		= MTYPE_SLIDER;
	s_options_interface_menualpha_slider.generic.x			= 0;
	s_options_interface_menualpha_slider.generic.y			= y+=MENU_LINE_SIZE;
	s_options_interface_menualpha_slider.generic.name		= "ingame menu transparency";
	s_options_interface_menualpha_slider.generic.callback	= MenuAlphaFunc;
	s_options_interface_menualpha_slider.minvalue			= 0;
	s_options_interface_menualpha_slider.maxvalue			= 20;
	s_options_interface_menualpha_slider.generic.statusbar	= "changes opacity of menu background";

	// free any loaded fonts to prevent memory leak
	if (numfonts > 0) {
		FS_FreeFileList (font_names, numfonts);
	}
	numfonts = 0;
	font_names = SetFontNames ();
	s_options_interface_font_box.generic.type				= MTYPE_SPINCONTROL;
	s_options_interface_font_box.generic.x					= 0;
	s_options_interface_font_box.generic.y					= y+=2*MENU_LINE_SIZE;
	s_options_interface_font_box.generic.name				= "font";
	s_options_interface_font_box.generic.callback			= FontFunc;
	s_options_interface_font_box.itemnames					= font_names;
	s_options_interface_font_box.generic.statusbar			= "changes console and menu text font";

	s_options_interface_fontsize_slider.generic.type		= MTYPE_SLIDER;
	s_options_interface_fontsize_slider.generic.x			= 0;
	s_options_interface_fontsize_slider.generic.y			= y+=MENU_LINE_SIZE;
	s_options_interface_fontsize_slider.generic.name		= "console font size";
	s_options_interface_fontsize_slider.generic.callback	= FontSizeFunc;
	s_options_interface_fontsize_slider.minvalue			= 3;
	s_options_interface_fontsize_slider.maxvalue			= 8;
	s_options_interface_fontsize_slider.generic.statusbar	= "changes size of console text";

	s_options_interface_alt_text_color_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_interface_alt_text_color_box.generic.x		= 0;
	s_options_interface_alt_text_color_box.generic.y		= y+=MENU_LINE_SIZE;
	s_options_interface_alt_text_color_box.generic.name		= "alt text color";
	s_options_interface_alt_text_color_box.generic.callback	= AltTextColorFunc;
	s_options_interface_alt_text_color_box.itemnames		= textcolor_names;
	s_options_interface_alt_text_color_box.generic.statusbar	= "changes color of highlighted text";

	s_options_interface_conalpha_slider.generic.type		= MTYPE_SLIDER;
	s_options_interface_conalpha_slider.generic.x			= 0;
	s_options_interface_conalpha_slider.generic.y			= y+=2*MENU_LINE_SIZE;
	s_options_interface_conalpha_slider.generic.name		= "console transparency";
	s_options_interface_conalpha_slider.generic.callback	= ConAlphaFunc;
	s_options_interface_conalpha_slider.minvalue			= 0;
	s_options_interface_conalpha_slider.maxvalue			= 20;
	s_options_interface_conalpha_slider.generic.statusbar	= "changes opacity of console";

	/*
	s_options_interface_conheight_slider.generic.type	= MTYPE_SLIDER;
	s_options_interface_conheight_slider.generic.x		= 0;
	s_options_interface_conheight_slider.generic.y		= y+=MENU_LINE_SIZE;
	s_options_interface_conheight_slider.generic.name	= "console height";
	s_options_interface_conheight_slider.generic.callback = ConHeightFunc;
	s_options_interface_conheight_slider.minvalue		= 0;
	s_options_interface_conheight_slider.maxvalue		= 10;
	*/

	s_options_interface_simple_loadscreen_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_interface_simple_loadscreen_box.generic.x			= 0;
	s_options_interface_simple_loadscreen_box.generic.y			= y+=2*MENU_LINE_SIZE;
	s_options_interface_simple_loadscreen_box.generic.name		= "simple load screens";
	s_options_interface_simple_loadscreen_box.generic.callback	= SimpleLoadscreenFunc;
	s_options_interface_simple_loadscreen_box.itemnames			= yesno_names;
	s_options_interface_simple_loadscreen_box.generic.statusbar	= "toggles simple map load screen";

	s_options_interface_newconback_box.generic.type			= MTYPE_SPINCONTROL;
	s_options_interface_newconback_box.generic.x			= 0;
	s_options_interface_newconback_box.generic.y			= y+=MENU_LINE_SIZE;
	s_options_interface_newconback_box.generic.name			= "new console background";
	s_options_interface_newconback_box.generic.callback		= NewConbackFunc;
	s_options_interface_newconback_box.itemnames			= yesno_names;
	s_options_interface_newconback_box.generic.statusbar	= "enables Q3-style console background";

	s_options_interface_noalttab_box.generic.type		= MTYPE_SPINCONTROL;
	s_options_interface_noalttab_box.generic.x			= 0;
	s_options_interface_noalttab_box.generic.y			= y+=2*MENU_LINE_SIZE;
	s_options_interface_noalttab_box.generic.name		= "disable alt-tab";
	s_options_interface_noalttab_box.generic.callback	= NoAltTabFunc;
	s_options_interface_noalttab_box.itemnames			= yesno_names;
	s_options_interface_noalttab_box.generic.statusbar	= "disables alt-tabbing to desktop";

	s_options_interface_defaults_action.generic.type		= MTYPE_ACTION;
	s_options_interface_defaults_action.generic.x			= MENU_FONT_SIZE;
	s_options_interface_defaults_action.generic.y			= 18*MENU_LINE_SIZE;
	s_options_interface_defaults_action.generic.name		= "reset defaults";
	s_options_interface_defaults_action.generic.callback	= InterfaceResetDefaultsFunc;
	s_options_interface_defaults_action.generic.statusbar	= "resets all interface settings to internal defaults";

	s_options_interface_back_action.generic.type		= MTYPE_ACTION;
	s_options_interface_back_action.generic.x			= MENU_FONT_SIZE;
	s_options_interface_back_action.generic.y			= 20*MENU_LINE_SIZE;
	s_options_interface_back_action.generic.name		= "back to options";
	s_options_interface_back_action.generic.callback	= UI_BackMenu;

	Menu_AddItem( &s_options_interface_menu, ( void * ) &s_options_interface_header );
	Menu_AddItem( &s_options_interface_menu, ( void * ) &s_options_interface_menumouse_slider );
	Menu_AddItem( &s_options_interface_menu, ( void * ) &s_options_interface_menualpha_slider );
	Menu_AddItem( &s_options_interface_menu, ( void * ) &s_options_interface_font_box );
	Menu_AddItem( &s_options_interface_menu, ( void * ) &s_options_interface_fontsize_slider );
	Menu_AddItem( &s_options_interface_menu, ( void * ) &s_options_interface_alt_text_color_box );
	Menu_AddItem( &s_options_interface_menu, ( void * ) &s_options_interface_conalpha_slider );
	//Menu_AddItem( &s_options_interface_menu, ( void * ) &s_options_interface_conheight_slider );
	Menu_AddItem( &s_options_interface_menu, ( void * ) &s_options_interface_simple_loadscreen_box );
	Menu_AddItem( &s_options_interface_menu, ( void * ) &s_options_interface_newconback_box );
	Menu_AddItem( &s_options_interface_menu, ( void * ) &s_options_interface_noalttab_box );
	Menu_AddItem( &s_options_interface_menu, ( void * ) &s_options_interface_defaults_action );
	Menu_AddItem( &s_options_interface_menu, ( void * ) &s_options_interface_back_action );

	InterfaceSetMenuItemValues();
}

void Options_Interface_MenuDraw (void)
{
	Menu_DrawBanner( "m_banner_options" );

	Menu_AdjustCursor( &s_options_interface_menu, 1 );
	Menu_Draw( &s_options_interface_menu );
}

const char *Options_Interface_MenuKey( int key )
{
	return Default_MenuKey( &s_options_interface_menu, key );
}

void M_Menu_Options_Interface_f (void)
{
	Options_Interface_MenuInit();
	UI_PushMenu ( Options_Interface_MenuDraw, Options_Interface_MenuKey );
}
