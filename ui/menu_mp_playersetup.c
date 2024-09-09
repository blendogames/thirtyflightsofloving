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

// menu_mp_playersetup.c -- the player setup menu 

#include "../client/client.h"
#include "ui_local.h"

#ifdef NOTTHIRTYFLIGHTS
/*
=============================================================================

PLAYER CONFIG MENU

=============================================================================
*/

static menuFramework_s	s_player_config_menu;
static menuImage_s		s_playerconfig_banner;
static menuField_s		s_playerconfig_name_field;

//static menuLabel_s		s_playerconfig_model_title;
//static menuPicker_s		s_playerconfig_model_box;
//static menuLabel_s		s_playerconfig_skin_title;
//static menuPicker_s		s_playerconfig_skin_box;
//static menuLabel_s		s_playerconfig_hand_title;
//static menuPicker_s		s_playerconfig_handedness_box;
//static menuLabel_s		s_playerconfig_rate_title;
//static menuPicker_s		s_playerconfig_rate_box;
static menuComboBox_s	s_playerconfig_model_box;
static menuComboBox_s	s_playerconfig_skin_box;
static menuComboBox_s	s_playerconfig_handedness_box;
static menuComboBox_s	s_playerconfig_rate_box;

static menuLabel_s		s_playerconfig_railcolor_title;
static menuRectangle_s	s_playerconfig_railcolor_background;
static menuImage_s		s_playerconfig_railcolor_display[2];
static menuSlider_s		s_playerconfig_railcolor_slider[3];
static menuListView_s	s_playerconfig_skin_display;
static menuAction_s		s_playerconfig_back_action;
static menuModelView_s	s_playerconfig_model_display;

//=======================================================================

static void Menu_LoadPlayerRailColor (void)
{
	color_t	railColor;

	if ( Com_ParseColorString(Cvar_VariableString("color1"), railColor) ) {
		Cvar_SetInteger ("ui_player_railred", railColor[0]);
		Cvar_SetInteger ("ui_player_railgreen", railColor[1]);
		Cvar_SetInteger ("ui_player_railblue", railColor[2]);
		s_playerconfig_railcolor_display[1].imageColor[0] = min(max(railColor[0], 0), 255);
		s_playerconfig_railcolor_display[1].imageColor[1] = min(max(railColor[1], 0), 255);
		s_playerconfig_railcolor_display[1].imageColor[2] = min(max(railColor[2], 0), 255);
	}
}


static void Menu_SavePlayerRailColor (void *unused)
{
	Cvar_Set ( "color1", va("%02X%02X%02X",
			min(max(Cvar_VariableInteger("ui_player_railred"), 0), 255),
			min(max(Cvar_VariableInteger("ui_player_railgreen"), 0), 255),
			min(max(Cvar_VariableInteger("ui_player_railblue"), 0), 255)) );
	s_playerconfig_railcolor_display[1].imageColor[0] = min(max(Cvar_VariableInteger("ui_player_railred"), 0), 255);
	s_playerconfig_railcolor_display[1].imageColor[1] = min(max(Cvar_VariableInteger("ui_player_railgreen"), 0), 255);
	s_playerconfig_railcolor_display[1].imageColor[2] = min(max(Cvar_VariableInteger("ui_player_railblue"), 0), 255);
}


static void Menu_PlayerModelCallback (void *unused)
{
	int		mNum, sNum;

	mNum = s_playerconfig_model_box.curValue;

	// update skin picker
	s_playerconfig_skin_box.itemNames = ui_pmi[mNum].skinDisplayNames;
	s_playerconfig_skin_box.curValue = 0;
	UI_InitMenuItem (&s_playerconfig_skin_box);

	// update skin listview
	s_playerconfig_skin_display.itemNames = ui_pmi[mNum].skinDisplayNames;
	s_playerconfig_skin_display.imageNames = ui_pmi[mNum].skinIconNames;
	s_playerconfig_skin_display.curValue = 0;
	UI_InitMenuItem (&s_playerconfig_skin_display);

	sNum = s_playerconfig_skin_box.curValue;
	UI_UpdatePlayerModelInfo (mNum, sNum);

	// update player model display
	UI_InitMenuItem (&s_playerconfig_model_display);
}


static void Menu_PlayerSkinCallback (void *unused)
{
	int		mNum, sNum;

	mNum = s_playerconfig_model_box.curValue;
	sNum = s_playerconfig_skin_box.curValue;
	UI_UpdatePlayerSkinInfo (mNum, sNum);

	// update skin listview
	s_playerconfig_skin_display.curValue = sNum;
	UI_InitMenuItem (&s_playerconfig_skin_display);

	// update player model display
	UI_InitMenuItem (&s_playerconfig_model_display);
}


static void Menu_PlayerSkinViewCallback (void *unused)
{
	int		mNum, sNum;

	mNum = s_playerconfig_model_box.curValue;
	sNum = s_playerconfig_skin_display.curValue;
	UI_UpdatePlayerSkinInfo (mNum, sNum);

	// update skin picker
	s_playerconfig_skin_box.curValue = sNum;

	// update player model display
	UI_InitMenuItem (&s_playerconfig_model_display);
}


static void Menu_PlayerHandednessCallback (void *unused)
{
	int			i;
	qboolean	lefthand;

	// update player model display
	lefthand = (Cvar_VariableValue("hand") == 1);
	s_playerconfig_model_display.isMirrored = lefthand;
	for (i=0; i<2; i++)
		VectorSet (s_playerconfig_model_display.modelRotation[i], 0, (lefthand ? -0.1 : 0.1), 0);
}


void Menu_PConfigSaveChanges (void *unused)
{
	int		mNum, sNum;
	char	scratch[1024];

	UI_SaveMenuItemValue (&s_playerconfig_name_field);

	mNum = s_playerconfig_model_box.curValue;
	sNum = s_playerconfig_skin_box.curValue;
	Com_sprintf (scratch, sizeof( scratch ), "%s/%s", 
		ui_pmi[mNum].directory, ui_pmi[mNum].skinDisplayNames[sNum]);
	Cvar_Set ("skin", scratch);
}

//=======================================================================

void Menu_PlayerConfig_Init (void)
{
	int			i, x, y, mNum = 0, sNum = 0;
	qboolean	lefthand = (Cvar_VariableValue("hand") == 1);

	static const char *handedness_names[] = { "right", "left", "center", 0 };

	static const char *rate_names[] =
	{
		"28.8 Modem",
		"33.6 Modem",
		"56K/Single ISDN",
		"Dual ISDN",
		"Cable/DSL",
		"T1/LAN",
		"User defined",
		0
	};
	static const char *rate_values[] =
	{
		"2500",
		"3200",
		"5000",
		"10000",
		"15000",
		"25000",
		UI_ITEMVALUE_WILDCARD,
		0
	};

	// get model and skin index and precache them
	UI_InitPlayerModelInfo (&mNum, &sNum);

	// menu.x = 38, menu.y = 170
	x = 4.75*MENU_FONT_SIZE;	// SCREEN_WIDTH*0.5 - 282
	y = 17*MENU_LINE_SIZE;		// SCREEN_HEIGHT*0.5 - 7*MENU_LINE_SIZE

	s_player_config_menu.x					= 0;
	s_player_config_menu.y					= 0;
	s_player_config_menu.nitems				= 0;
	s_player_config_menu.isPopup			= false;
	s_player_config_menu.background			= NULL;
	s_player_config_menu.drawFunc			= UI_DefaultMenuDraw;
	s_player_config_menu.keyFunc			= UI_DefaultMenuKey;
	s_player_config_menu.canOpenFunc		= UI_HaveValidPlayerModels;
	s_player_config_menu.cantOpenMessage	= "No valid player models found";
	s_player_config_menu.onExitFunc			= Menu_PConfigSaveChanges;
	
	s_playerconfig_banner.generic.type		= MTYPE_IMAGE;
	s_playerconfig_banner.generic.x			= 0;
	s_playerconfig_banner.generic.y			= 9*MENU_LINE_SIZE;
	s_playerconfig_banner.width				= 275;
	s_playerconfig_banner.height			= 32;
	s_playerconfig_banner.imageName			= "/pics/m_banner_plauer_setup.pcx";
	s_playerconfig_banner.alpha				= 255;
	s_playerconfig_banner.border			= 0;
	s_playerconfig_banner.hCentered			= true;
	s_playerconfig_banner.vCentered			= false;
	s_playerconfig_banner.useAspectRatio	= false;
	s_playerconfig_banner.generic.isHidden	= false;

	s_playerconfig_name_field.generic.type			= MTYPE_FIELD;
	s_playerconfig_name_field.generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_name_field.generic.header		= "name";
	s_playerconfig_name_field.generic.name			= 0;	// "name"
	s_playerconfig_name_field.generic.x				= x;
	s_playerconfig_name_field.generic.y				= y;
	s_playerconfig_name_field.generic.flags			= 0;
	s_playerconfig_name_field.generic.callback		= 0;
	s_playerconfig_name_field.length				= 23;
	s_playerconfig_name_field.visible_length		= 23;
	s_playerconfig_name_field.generic.cvar			= "name";
	s_playerconfig_name_field.generic.cvarNoSave	= true;
/*
	s_playerconfig_model_title.generic.type		= MTYPE_LABEL;
	s_playerconfig_model_title.generic.textSize	= MENU_FONT_SIZE;
	s_playerconfig_model_title.generic.flags	= QMF_LEFT_JUSTIFY;
	s_playerconfig_model_title.generic.name		= "model";
	s_playerconfig_model_title.generic.x		= x + 4*MENU_FONT_SIZE;
	s_playerconfig_model_title.generic.y		= y += 3*MENU_LINE_SIZE;

	s_playerconfig_model_box.generic.type			= MTYPE_PICKER;
	s_playerconfig_model_box.generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_model_box.generic.x				= x;
	s_playerconfig_model_box.generic.y				= y += MENU_LINE_SIZE;
	s_playerconfig_model_box.generic.callback		= Menu_PlayerModelCallback;
	s_playerconfig_model_box.generic.cursor_offset	= -1*MENU_FONT_SIZE;
	s_playerconfig_model_box.curValue				= mNum;
	s_playerconfig_model_box.itemNames				= ui_pmnames;
	
	s_playerconfig_skin_title.generic.type		= MTYPE_LABEL;
	s_playerconfig_skin_title.generic.textSize	= MENU_FONT_SIZE;
	s_playerconfig_skin_title.generic.flags		= QMF_LEFT_JUSTIFY;
	s_playerconfig_skin_title.generic.name		= "skin";
	s_playerconfig_skin_title.generic.x			= x + 4*MENU_FONT_SIZE;
	s_playerconfig_skin_title.generic.y			= y += 2*MENU_LINE_SIZE;
	
	s_playerconfig_skin_box.generic.type			= MTYPE_PICKER;
	s_playerconfig_skin_box.generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_skin_box.generic.x				= x;
	s_playerconfig_skin_box.generic.y				= y += MENU_LINE_SIZE;
	s_playerconfig_skin_box.generic.name			= 0;
	s_playerconfig_skin_box.generic.callback		= Menu_PlayerSkinCallback;
	s_playerconfig_skin_box.generic.cursor_offset	= -1*MENU_FONT_SIZE;
	s_playerconfig_skin_box.curValue				= sNum;
	s_playerconfig_skin_box.itemNames				= ui_pmi[mNum].skinDisplayNames;
	s_playerconfig_skin_box.generic.flags			|= QMF_NOLOOP;
	
	s_playerconfig_hand_title.generic.type		= MTYPE_LABEL;
	s_playerconfig_hand_title.generic.textSize	= MENU_FONT_SIZE;
	s_playerconfig_hand_title.generic.flags		= QMF_LEFT_JUSTIFY;
	s_playerconfig_hand_title.generic.name		= "handedness";
	s_playerconfig_hand_title.generic.x			= x + 4*MENU_FONT_SIZE;
	s_playerconfig_hand_title.generic.y			= y += 2*MENU_LINE_SIZE;
	
	s_playerconfig_handedness_box.generic.type			= MTYPE_PICKER;
	s_playerconfig_handedness_box.generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_handedness_box.generic.x				= x;
	s_playerconfig_handedness_box.generic.y				= y += MENU_LINE_SIZE;
	s_playerconfig_handedness_box.generic.name			= 0;
	s_playerconfig_handedness_box.generic.cursor_offset	= -1*MENU_FONT_SIZE;
	s_playerconfig_handedness_box.itemNames				= handedness_names;
	s_playerconfig_handedness_box.generic.callback		= Menu_PlayerHandednessCallback;
	s_playerconfig_handedness_box.generic.cvar			= "hand";
	s_playerconfig_handedness_box.generic.cvarClamp		= true;
	s_playerconfig_handedness_box.generic.cvarMin		= 0;
	s_playerconfig_handedness_box.generic.cvarMax		= 2;
			
	s_playerconfig_rate_title.generic.type			= MTYPE_LABEL;
	s_playerconfig_rate_title.generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_rate_title.generic.flags			= QMF_LEFT_JUSTIFY;
	s_playerconfig_rate_title.generic.name			= "connect speed";
	s_playerconfig_rate_title.generic.x				= x + 4*MENU_FONT_SIZE;
	s_playerconfig_rate_title.generic.y				= y += 2*MENU_LINE_SIZE;
		
	s_playerconfig_rate_box.generic.type			= MTYPE_PICKER;
	s_playerconfig_rate_box.generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_rate_box.generic.x				= x;
	s_playerconfig_rate_box.generic.y				= y += MENU_LINE_SIZE;
	s_playerconfig_rate_box.generic.name			= 0;
	s_playerconfig_rate_box.generic.cursor_offset	= -1*MENU_FONT_SIZE;
	s_playerconfig_rate_box.itemNames				= rate_names;
	s_playerconfig_rate_box.itemValues				= rate_values;
	s_playerconfig_rate_box.generic.cvar			= "rate";
	s_playerconfig_rate_box.generic.cvarClamp		= false;
*/

	s_playerconfig_model_box.generic.type		= MTYPE_COMBOBOX;
	s_playerconfig_model_box.generic.header		= "model";
	s_playerconfig_model_box.generic.name		= 0;
	s_playerconfig_model_box.generic.x			= x;
	s_playerconfig_model_box.generic.y			= y += 3*MENU_LINE_SIZE;
	s_playerconfig_model_box.items_y			= 10;
	s_playerconfig_model_box.itemWidth			= 16;
	s_playerconfig_model_box.itemSpacing		= 1;
	s_playerconfig_model_box.itemTextSize		= 8;
	s_playerconfig_model_box.border				= 1;
	s_playerconfig_model_box.borderColor[0]		= 60;
	s_playerconfig_model_box.borderColor[1]		= 60;
	s_playerconfig_model_box.borderColor[2]		= 60;
	s_playerconfig_model_box.borderColor[3]		= 255;
	s_playerconfig_model_box.backColor[0]		= 0;
	s_playerconfig_model_box.backColor[1]		= 0;
	s_playerconfig_model_box.backColor[2]		= 0;
	s_playerconfig_model_box.backColor[3]		= 224;
	s_playerconfig_model_box.curValue			= mNum;
	s_playerconfig_model_box.itemNames			= ui_pmnames;
	s_playerconfig_model_box.generic.callback	= Menu_PlayerModelCallback;

	s_playerconfig_skin_box.generic.type		= MTYPE_COMBOBOX;
	s_playerconfig_skin_box.generic.header		= "skin";
	s_playerconfig_skin_box.generic.name		= 0;
	s_playerconfig_skin_box.generic.x			= x;
	s_playerconfig_skin_box.generic.y			= y += 3*MENU_LINE_SIZE;
	s_playerconfig_skin_box.items_y				= 10;
	s_playerconfig_skin_box.itemWidth			= 16;
	s_playerconfig_skin_box.itemSpacing			= 1;
	s_playerconfig_skin_box.itemTextSize		= 8;
	s_playerconfig_skin_box.border				= 1;
	s_playerconfig_skin_box.borderColor[0]		= 60;
	s_playerconfig_skin_box.borderColor[1]		= 60;
	s_playerconfig_skin_box.borderColor[2]		= 60;
	s_playerconfig_skin_box.borderColor[3]		= 255;
	s_playerconfig_skin_box.backColor[0]		= 0;
	s_playerconfig_skin_box.backColor[1]		= 0;
	s_playerconfig_skin_box.backColor[2]		= 0;
	s_playerconfig_skin_box.backColor[3]		= 224;
	s_playerconfig_skin_box.curValue			= sNum;
	s_playerconfig_skin_box.itemNames			= ui_pmi[mNum].skinDisplayNames;
	s_playerconfig_skin_box.generic.callback	= Menu_PlayerSkinCallback;

	s_playerconfig_handedness_box.generic.type		= MTYPE_COMBOBOX;
	s_playerconfig_handedness_box.generic.header	= "handedness";
	s_playerconfig_handedness_box.generic.name		= 0;
	s_playerconfig_handedness_box.generic.x			= x;
	s_playerconfig_handedness_box.generic.y			= y += 3*MENU_LINE_SIZE;
	s_playerconfig_handedness_box.items_y			= 3;
	s_playerconfig_handedness_box.itemWidth			= 7;
	s_playerconfig_handedness_box.itemSpacing		= 1;
	s_playerconfig_handedness_box.itemTextSize		= 8;
	s_playerconfig_handedness_box.border			= 1;
	s_playerconfig_handedness_box.borderColor[0]	= 60;
	s_playerconfig_handedness_box.borderColor[1]	= 60;
	s_playerconfig_handedness_box.borderColor[2]	= 60;
	s_playerconfig_handedness_box.borderColor[3]	= 255;
	s_playerconfig_handedness_box.backColor[0]		= 0;
	s_playerconfig_handedness_box.backColor[1]		= 0;
	s_playerconfig_handedness_box.backColor[2]		= 0;
	s_playerconfig_handedness_box.backColor[3]		= 224;
	s_playerconfig_handedness_box.itemNames			= handedness_names;
	s_playerconfig_handedness_box.generic.callback	= Menu_PlayerHandednessCallback;
	s_playerconfig_handedness_box.generic.cvar		= "hand";
	s_playerconfig_handedness_box.generic.cvarClamp	= true;
	s_playerconfig_handedness_box.generic.cvarMin	= 0;
	s_playerconfig_handedness_box.generic.cvarMax	= 2;
		
	s_playerconfig_rate_box.generic.type		= MTYPE_COMBOBOX;
	s_playerconfig_rate_box.generic.header		= "connect speed";
	s_playerconfig_rate_box.generic.name		= 0;
	s_playerconfig_rate_box.generic.x			= x;
	s_playerconfig_rate_box.generic.y			= y += 3*MENU_LINE_SIZE;
	s_playerconfig_rate_box.items_y				= 7;
	s_playerconfig_rate_box.itemWidth			= 16;
	s_playerconfig_rate_box.itemSpacing			= 1;
	s_playerconfig_rate_box.itemTextSize		= 8;
	s_playerconfig_rate_box.border				= 1;
	s_playerconfig_rate_box.borderColor[0]		= 60;
	s_playerconfig_rate_box.borderColor[1]		= 60;
	s_playerconfig_rate_box.borderColor[2]		= 60;
	s_playerconfig_rate_box.borderColor[3]		= 255;
	s_playerconfig_rate_box.backColor[0]		= 0;
	s_playerconfig_rate_box.backColor[1]		= 0;
	s_playerconfig_rate_box.backColor[2]		= 0;
	s_playerconfig_rate_box.backColor[3]		= 224;
	s_playerconfig_rate_box.itemNames			= rate_names;
	s_playerconfig_rate_box.itemValues			= rate_values;
	s_playerconfig_rate_box.generic.cvar		= "rate";
	s_playerconfig_rate_box.generic.cvarClamp	= false;

	s_playerconfig_railcolor_title.generic.type		= MTYPE_LABEL;
	s_playerconfig_railcolor_title.generic.textSize	= MENU_FONT_SIZE;
	s_playerconfig_railcolor_title.generic.flags	= QMF_LEFT_JUSTIFY;
	s_playerconfig_railcolor_title.generic.name		= "railgun effect color";
	s_playerconfig_railcolor_title.generic.x		= x + 4*MENU_FONT_SIZE;
	s_playerconfig_railcolor_title.generic.y		= y += 3*MENU_LINE_SIZE;

	s_playerconfig_railcolor_background.generic.type		= MTYPE_RECTANGLE;
	s_playerconfig_railcolor_background.generic.x			= x + 2*MENU_FONT_SIZE;
	s_playerconfig_railcolor_background.generic.y			= y += 1.5*MENU_LINE_SIZE;
	s_playerconfig_railcolor_background.width				= 160;
	s_playerconfig_railcolor_background.height				= 40;
	s_playerconfig_railcolor_background.color[0]			= 0;
	s_playerconfig_railcolor_background.color[1]			= 0;
	s_playerconfig_railcolor_background.color[2]			= 0;
	s_playerconfig_railcolor_background.color[3]			= 255;
	s_playerconfig_railcolor_background.border				= 2;
	s_playerconfig_railcolor_background.borderColor[0]		= 60;
	s_playerconfig_railcolor_background.borderColor[1]		= 60;
	s_playerconfig_railcolor_background.borderColor[2]		= 60;
	s_playerconfig_railcolor_background.borderColor[3]		= 255;
	s_playerconfig_railcolor_background.hCentered			= false;
	s_playerconfig_railcolor_background.vCentered			= false;
	s_playerconfig_railcolor_background.generic.isHidden	= false;

	s_playerconfig_railcolor_display[0].generic.type		= MTYPE_IMAGE;
	s_playerconfig_railcolor_display[0].generic.x			= x + 2*MENU_FONT_SIZE;
	s_playerconfig_railcolor_display[0].generic.y			= y;
	s_playerconfig_railcolor_display[0].width				= 160;
	s_playerconfig_railcolor_display[0].height				= 40;
	s_playerconfig_railcolor_display[0].imageName			= UI_RAILCORE_PIC;
	s_playerconfig_railcolor_display[0].alpha				= 254;
	s_playerconfig_railcolor_display[0].border				= 0;
	s_playerconfig_railcolor_display[0].hCentered			= false;
	s_playerconfig_railcolor_display[0].vCentered			= false;
	s_playerconfig_railcolor_display[0].useAspectRatio		= false;
	s_playerconfig_railcolor_display[0].generic.isHidden	= false;

	s_playerconfig_railcolor_display[1].generic.type		= MTYPE_IMAGE;
	s_playerconfig_railcolor_display[1].generic.x			= x + 2*MENU_FONT_SIZE;
	s_playerconfig_railcolor_display[1].generic.y			= y;
	s_playerconfig_railcolor_display[1].width				= 160;
	s_playerconfig_railcolor_display[1].height				= 40;
	s_playerconfig_railcolor_display[1].imageName			= UI_RAILSPIRAL_PIC;
	s_playerconfig_railcolor_display[1].alpha				= 254;
	s_playerconfig_railcolor_display[1].overrideColor		= true;
	s_playerconfig_railcolor_display[1].imageColor[0]		= 255;
	s_playerconfig_railcolor_display[1].imageColor[1]		= 255;
	s_playerconfig_railcolor_display[1].imageColor[2]		= 255;
	s_playerconfig_railcolor_display[1].imageColor[3]		= 254;
	s_playerconfig_railcolor_display[1].border				= 0;
	s_playerconfig_railcolor_display[1].hCentered			= false;
	s_playerconfig_railcolor_display[1].vCentered			= false;
	s_playerconfig_railcolor_display[1].useAspectRatio		= false;
	s_playerconfig_railcolor_display[1].generic.isHidden	= false;

	s_playerconfig_railcolor_slider[0].generic.type			= MTYPE_SLIDER;
	s_playerconfig_railcolor_slider[0].generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_railcolor_slider[0].generic.x			= x + 9*MENU_FONT_SIZE;
	s_playerconfig_railcolor_slider[0].generic.y			= y += 4.5*MENU_LINE_SIZE;
	s_playerconfig_railcolor_slider[0].generic.name			= "red";
	s_playerconfig_railcolor_slider[0].generic.callback		= Menu_SavePlayerRailColor;
	s_playerconfig_railcolor_slider[0].maxPos				= 64;
	s_playerconfig_railcolor_slider[0].baseValue			= 0.0f;
	s_playerconfig_railcolor_slider[0].increment			= 4.0f;
	s_playerconfig_railcolor_slider[0].displayAsPercent		= false;
	s_playerconfig_railcolor_slider[0].generic.cvar			= "ui_player_railred";
	s_playerconfig_railcolor_slider[0].generic.cvarClamp	= true;
	s_playerconfig_railcolor_slider[0].generic.cvarMin		= 0;
	s_playerconfig_railcolor_slider[0].generic.cvarMax		= 256;
	s_playerconfig_railcolor_slider[0].generic.statusbar	= "changes player's railgun particle effect red component";

	s_playerconfig_railcolor_slider[1].generic.type			= MTYPE_SLIDER;
	s_playerconfig_railcolor_slider[1].generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_railcolor_slider[1].generic.x			= x + 9*MENU_FONT_SIZE;
	s_playerconfig_railcolor_slider[1].generic.y			= y += MENU_LINE_SIZE;
	s_playerconfig_railcolor_slider[1].generic.name			= "green";
	s_playerconfig_railcolor_slider[1].generic.callback		= Menu_SavePlayerRailColor;
	s_playerconfig_railcolor_slider[1].maxPos				= 64;
	s_playerconfig_railcolor_slider[1].baseValue			= 0.0f;
	s_playerconfig_railcolor_slider[1].increment			= 4.0f;
	s_playerconfig_railcolor_slider[1].displayAsPercent		= false;
	s_playerconfig_railcolor_slider[1].generic.cvar			= "ui_player_railgreen";
	s_playerconfig_railcolor_slider[1].generic.cvarClamp	= true;
	s_playerconfig_railcolor_slider[1].generic.cvarMin		= 0;
	s_playerconfig_railcolor_slider[1].generic.cvarMax		= 256;
	s_playerconfig_railcolor_slider[1].generic.statusbar	= "changes player's railgun particle effect green component";

	s_playerconfig_railcolor_slider[2].generic.type			= MTYPE_SLIDER;
	s_playerconfig_railcolor_slider[2].generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_railcolor_slider[2].generic.x			= x + 9*MENU_FONT_SIZE;
	s_playerconfig_railcolor_slider[2].generic.y			= y += MENU_LINE_SIZE;
	s_playerconfig_railcolor_slider[2].generic.name			= "blue";
	s_playerconfig_railcolor_slider[2].generic.callback		= Menu_SavePlayerRailColor;
	s_playerconfig_railcolor_slider[2].maxPos				= 64;
	s_playerconfig_railcolor_slider[2].baseValue			= 0.0f;
	s_playerconfig_railcolor_slider[2].increment			= 4.0f;
	s_playerconfig_railcolor_slider[2].displayAsPercent		= false;
	s_playerconfig_railcolor_slider[2].generic.cvar			= "ui_player_railblue";
	s_playerconfig_railcolor_slider[2].generic.cvarClamp	= true;
	s_playerconfig_railcolor_slider[2].generic.cvarMin		= 0;
	s_playerconfig_railcolor_slider[2].generic.cvarMax		= 256;
	s_playerconfig_railcolor_slider[2].generic.statusbar	= "changes player's railgun particle effect blue component";

	s_playerconfig_skin_display.generic.type		= MTYPE_LISTVIEW;
//	s_playerconfig_skin_display.generic.header		= "skin";
	s_playerconfig_skin_display.generic.x			= SCREEN_WIDTH*0.5 - 3*MENU_FONT_SIZE;
	s_playerconfig_skin_display.generic.y			= SCREEN_HEIGHT - 110;
	s_playerconfig_skin_display.generic.name		= 0;
	s_playerconfig_skin_display.listViewType		= LISTVIEW_IMAGE;
	s_playerconfig_skin_display.items_x				= 7;
	s_playerconfig_skin_display.items_y				= 1;
	s_playerconfig_skin_display.scrollDir			= 0;
	s_playerconfig_skin_display.itemWidth			= 36;
	s_playerconfig_skin_display.itemHeight			= 36;
	s_playerconfig_skin_display.itemSpacing			= 0;
	s_playerconfig_skin_display.itemPadding			= 2;
	s_playerconfig_skin_display.itemTextSize		= 4;
	s_playerconfig_skin_display.border				= 1;
	s_playerconfig_skin_display.borderColor[0]		= 0;
	s_playerconfig_skin_display.borderColor[1]		= 0;
	s_playerconfig_skin_display.borderColor[2]		= 0;
	s_playerconfig_skin_display.borderColor[3]		= 255;
	s_playerconfig_skin_display.backColor[0]		= 0;
	s_playerconfig_skin_display.backColor[1]		= 0;
	s_playerconfig_skin_display.backColor[2]		= 0;
	s_playerconfig_skin_display.backColor[3]		= 255;
	s_playerconfig_skin_display.background			= "/gfx/ui/widgets/listbox_background.pcx";
	s_playerconfig_skin_display.curValue			= sNum;
	s_playerconfig_skin_display.itemNames			= ui_pmi[mNum].skinDisplayNames;
	s_playerconfig_skin_display.imageNames			= ui_pmi[mNum].skinIconNames;
	s_playerconfig_skin_display.generic.callback	= Menu_PlayerSkinViewCallback;

	s_playerconfig_back_action.generic.type			= MTYPE_ACTION;
	s_playerconfig_back_action.generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_back_action.generic.name			= "Back to Multiplayer";
	s_playerconfig_back_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_playerconfig_back_action.generic.x			= x + 4*MENU_FONT_SIZE;
	s_playerconfig_back_action.generic.y			= y += 3*MENU_LINE_SIZE;
	s_playerconfig_back_action.generic.statusbar	= NULL;
	s_playerconfig_back_action.generic.callback		= UI_BackMenu;

	s_playerconfig_model_display.generic.type	= MTYPE_MODELVIEW;
	s_playerconfig_model_display.generic.x		= 0;
	s_playerconfig_model_display.generic.y		= 0;
	s_playerconfig_model_display.generic.name	= 0;
	s_playerconfig_model_display.width			= SCREEN_WIDTH;
	s_playerconfig_model_display.height			= SCREEN_HEIGHT;
	s_playerconfig_model_display.fov			= 50;
	s_playerconfig_model_display.isMirrored		= lefthand;
	s_playerconfig_model_display.modelName[0]	= ui_playerconfig_playermodelname;
	s_playerconfig_model_display.skinName[0]	= ui_playerconfig_playerskinname;
	s_playerconfig_model_display.modelName[1]	= ui_playerconfig_weaponmodelname;
	for (i=0; i<2; i++)
	{
		VectorSet (s_playerconfig_model_display.modelOrigin[i], 150, -25, 0);	// -8
		VectorSet (s_playerconfig_model_display.modelBaseAngles[i], 0, 0, 0);
		VectorSet (s_playerconfig_model_display.modelRotation[i], 0, (lefthand ? -0.1 : 0.1), 0);
		s_playerconfig_model_display.modelFrame[i]			= 0;
		s_playerconfig_model_display.modelFrameNumbers[i]	= 0;
	//	s_playerconfig_model_display.modelFrameTime[i]		= 0.01;
		s_playerconfig_model_display.entFlags[i]			= /*RF_FULLBRIGHT|*/RF_NOSHADOW|RF_DEPTHHACK;
	}
	s_playerconfig_model_display.generic.isHidden	= false;

	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_banner);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_name_field);
//	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_model_title);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_model_box);
	if ( s_playerconfig_skin_box.itemNames )
	{
	//	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_skin_title);
		UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_skin_box);
	}
//	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_hand_title);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_handedness_box);
//	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_rate_title);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_rate_box);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_railcolor_title);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_railcolor_background);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_railcolor_display[0]);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_railcolor_display[1]);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_railcolor_slider[0]);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_railcolor_slider[1]);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_railcolor_slider[2]);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_skin_display);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_back_action);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_model_display);

	// get color components from color1 cvar
	Menu_LoadPlayerRailColor ();
}


void Menu_PlayerConfig_f (void)
{
	UI_RefreshPlayerModels ();	// Reload player models if we recently downloaded anything
	Menu_PlayerConfig_Init ();
	UI_PushMenu (&s_player_config_menu);
}
#endif
