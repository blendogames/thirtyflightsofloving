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

// menu_mods.c -- the mods menu
 
#include "../client/client.h"
#include "ui_local.h"

/*
=======================================================================

MODS MENU

=======================================================================
*/

#define UI_MODS_LIST_HEIGHT 18
#define UI_MODS_LIST_WIDTH 48

menuFramework_s			s_mods_menu;
static menuImage_s		s_mods_banner;
static menuListBox_s	s_mods_list;
static menuLabel_s		s_mods_unsupported_label;
static menuAction_s		s_loadmod_action;
static menuAction_s		s_mods_back_action;

//=======================================================================

static void M_LoadModFunc (void *unused)
{
//	UI_LoadMod ((char *)s_mods_list.itemValues[s_mods_list.curValue]);
	UI_LoadModFromList (s_mods_list.curValue);
}

//=======================================================================

void Menu_Mods_Init (void)
{
	int  x, y;

	x = SCREEN_WIDTH*0.5 - (UI_MODS_LIST_WIDTH*MENU_FONT_SIZE + LIST_SCROLLBAR_CONTROL_SIZE) / 2;
	y = SCREEN_HEIGHT*0.5 - 58;

	s_mods_menu.x					= 0;
	s_mods_menu.y					= 0;
	s_mods_menu.nitems				= 0;
	s_mods_menu.isPopup				= false;
	s_mods_menu.background			= NULL;
	s_mods_menu.drawFunc			= UI_DefaultMenuDraw;
	s_mods_menu.keyFunc				= UI_DefaultMenuKey;
	s_mods_menu.canOpenFunc			= NULL;

	s_mods_banner.generic.type		= MTYPE_IMAGE;
	s_mods_banner.generic.x			= 0;
	s_mods_banner.generic.y			= 9*MENU_LINE_SIZE;
	s_mods_banner.width				= 275;
	s_mods_banner.height			= 32;
	s_mods_banner.imageName			= "/pics/m_banner_mods.pcx";
	s_mods_banner.alpha				= 255;
	s_mods_banner.border			= 0;
	s_mods_banner.hCentered			= true;
	s_mods_banner.vCentered			= false;
	s_mods_banner.useAspectRatio	= false;
	s_mods_banner.generic.isHidden	= false;

	s_mods_list.generic.type			= MTYPE_LISTBOX;
	s_mods_list.generic.name			= "";
	s_mods_list.generic.header			= "";
	s_mods_list.generic.x				= x - 2*MENU_FONT_SIZE;
	s_mods_list.generic.y				= y;
	s_mods_list.itemNames				= ui_mod_names;
//	s_mods_list.itemValues				= ui_mod_values;
	s_mods_list.itemWidth				= UI_MODS_LIST_WIDTH;
	s_mods_list.itemHeight				= 1;
	s_mods_list.items_y					= UI_MODS_LIST_HEIGHT;
	s_mods_list.itemSpacing				= 0;
	s_mods_list.itemTextSize			= 8;
	s_mods_list.border					= 2;
	s_mods_list.borderColor[0]			= 60;
	s_mods_list.borderColor[1]			= 60;
	s_mods_list.borderColor[2]			= 60;
	s_mods_list.borderColor[3]			= 255;
	s_mods_list.backColor[0]			= 0;
	s_mods_list.backColor[1]			= 0;
	s_mods_list.backColor[2]			= 0;
	s_mods_list.backColor[3]			= 192;
	s_mods_list.altBackColor[0]			= 10;
	s_mods_list.altBackColor[1]			= 10;
	s_mods_list.altBackColor[2]			= 10;
	s_mods_list.altBackColor[3]			= 192;
//	s_mods_list.generic.dblClkCallback	= M_LoadModFunc;
	s_mods_list.generic.statusbar		= "select a mod and click Load Mod";

	s_mods_unsupported_label.generic.type		= MTYPE_LABEL;
	s_mods_unsupported_label.generic.textSize	= MENU_SUBTEXT_FONT_SIZE;
#ifdef WIN32
	s_mods_unsupported_label.generic.name		= S_COLOR_ORANGE"Orange text indicates mod with unsupported game DLL";
#elif defined(__APPLE__) || defined(MACOSX)
	s_mods_unsupported_label.generic.name		= S_COLOR_ORANGE"Orange text indicates mod with unsupported game plugin";
#else // Linux / Unix  / Irix / etc
	s_mods_unsupported_label.generic.name		= S_COLOR_ORANGE"Orange text indicates mod with unsupported game SO";
#endif
	s_mods_unsupported_label.generic.flags		= QMF_LEFT_JUSTIFY|QMF_ALTCOLOR;
	s_mods_unsupported_label.generic.x			= x + 2*MENU_FONT_SIZE;
	s_mods_unsupported_label.generic.y			= y += (UI_MODS_LIST_HEIGHT)*MENU_LINE_SIZE + MENU_LINE_SIZE*0.5;

	s_loadmod_action.generic.type		= MTYPE_ACTION;
	s_loadmod_action.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_loadmod_action.generic.name		= "Load Mod";
	s_loadmod_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_loadmod_action.generic.x			= x + 2*MENU_FONT_SIZE;
	s_loadmod_action.generic.y			= y += 2*MENU_LINE_SIZE;	// (UI_MODS_LIST_HEIGHT+1)*MENU_LINE_SIZE;
	s_loadmod_action.generic.callback	= M_LoadModFunc;
	s_loadmod_action.generic.statusbar	= "click to load selected mods";

	s_mods_back_action.generic.type		= MTYPE_ACTION;
	s_mods_back_action.generic.textSize	= MENU_HEADER_FONT_SIZE;
	s_mods_back_action.generic.flags	= QMF_LEFT_JUSTIFY;
	s_mods_back_action.generic.name		= "Back to Main";
	s_mods_back_action.generic.x		= x + 2*MENU_FONT_SIZE;
	s_mods_back_action.generic.y		= y += 3*MENU_LINE_SIZE;
	s_mods_back_action.generic.callback	= UI_BackMenu;

	UI_AddMenuItem (&s_mods_menu, (void *) &s_mods_banner);
	UI_AddMenuItem (&s_mods_menu, (void *) &s_mods_list);
	UI_AddMenuItem (&s_mods_menu, (void *) &s_mods_unsupported_label);
	UI_AddMenuItem (&s_mods_menu, (void *) &s_loadmod_action);
	UI_AddMenuItem (&s_mods_menu, (void *) &s_mods_back_action);
}

void Menu_Mods_f (void)
{
	Menu_Mods_Init ();
	UI_PushMenu (&s_mods_menu);
}
