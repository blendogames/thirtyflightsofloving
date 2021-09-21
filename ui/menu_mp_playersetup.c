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

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"


/*
=============================================================================

PLAYER CONFIG MENU

=============================================================================
*/
extern menuframework_s	s_multiplayer_menu;

static menuframework_s	s_player_config_menu;
static menufield_s		s_playerconfig_name_field;
static menulist_s		s_playerconfig_model_box;
static menulist_s		s_playerconfig_skin_box;
static menulist_s		s_playerconfig_handedness_box;
static menulist_s		s_playerconfig_rate_box;
static menuseparator_s	s_playerconfig_skin_title;
static menuseparator_s	s_playerconfig_model_title;
static menuseparator_s	s_playerconfig_hand_title;
static menuseparator_s	s_playerconfig_rate_title;
static menuaction_s		s_playerconfig_back_action;

//=======================================================================

#define	NUM_SKINBOX_ITEMS 7

static void Menu_PlayerHandednessCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_playerconfig_handedness_box, "hand");
}


static void Menu_PlayerRateCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_playerconfig_rate_box, "rate");
}


static void Menu_PlayerModelCallback (void *unused)
{
	int		mNum, sNum;

	mNum = s_playerconfig_model_box.curValue;
	s_playerconfig_skin_box.itemNames = ui_pmi[mNum].skinDisplayNames;
	s_playerconfig_skin_box.curValue = 0;
	sNum = s_playerconfig_skin_box.curValue;

	UI_UpdatePlayerModelInfo (mNum, sNum);
}


static void Menu_PlayerSkinCallback (void *unused)
{
	int		mNum, sNum;

	mNum = s_playerconfig_model_box.curValue;
	sNum = s_playerconfig_skin_box.curValue;
	UI_UpdatePlayerSkinInfo (mNum, sNum);
}

//=======================================================================

qboolean Menu_PlayerConfig_Init (void)
{
	int		y;
	int		mNum = 0, sNum = 0;

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

	if ( !UI_HaveValidPlayerModels(NULL) )
		return false;

	// get model and skin index and precache them
	UI_InitPlayerModelInfo (&mNum, &sNum);

	y = 0;

	s_player_config_menu.x = SCREEN_WIDTH*0.5 - 210;
	s_player_config_menu.y = SCREEN_HEIGHT*0.5 - 70;
	s_player_config_menu.nitems = 0;
	
	s_playerconfig_name_field.generic.type		= MTYPE_FIELD;
	s_playerconfig_name_field.generic.textSize	= MENU_FONT_SIZE;
	s_playerconfig_name_field.generic.flags		= QMF_LEFT_JUSTIFY;
	s_playerconfig_name_field.generic.name		= "name";
	s_playerconfig_name_field.generic.callback	= 0;
	s_playerconfig_name_field.generic.x			= -MENU_FONT_SIZE;
	s_playerconfig_name_field.generic.y			= y;
	s_playerconfig_name_field.length			= 20;
	s_playerconfig_name_field.visible_length	= 20;
	Q_strncpyz (s_playerconfig_name_field.buffer, sizeof(s_playerconfig_name_field.buffer), Cvar_VariableString("name"));
	s_playerconfig_name_field.cursor = (int)strlen(s_playerconfig_name_field.buffer);

	s_playerconfig_model_title.generic.type		= MTYPE_SEPARATOR;
	s_playerconfig_model_title.generic.textSize	= MENU_FONT_SIZE;
	s_playerconfig_model_title.generic.flags	= QMF_LEFT_JUSTIFY;
	s_playerconfig_model_title.generic.name		= "model";
	s_playerconfig_model_title.generic.x		= -2*MENU_FONT_SIZE;
	s_playerconfig_model_title.generic.y		= y += 3*MENU_LINE_SIZE;
	
	s_playerconfig_model_box.generic.type			= MTYPE_SPINCONTROL;
	s_playerconfig_model_box.generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_model_box.generic.x				= -8*MENU_FONT_SIZE;
	s_playerconfig_model_box.generic.y				= y += MENU_LINE_SIZE;
	s_playerconfig_model_box.generic.callback		= Menu_PlayerModelCallback;
	s_playerconfig_model_box.generic.cursor_offset	= -1*MENU_FONT_SIZE;
	s_playerconfig_model_box.curValue				= mNum;
	s_playerconfig_model_box.itemNames				= ui_pmnames;
	
	s_playerconfig_skin_title.generic.type		= MTYPE_SEPARATOR;
	s_playerconfig_skin_title.generic.textSize	= MENU_FONT_SIZE;
	s_playerconfig_skin_title.generic.flags		= QMF_LEFT_JUSTIFY;
	s_playerconfig_skin_title.generic.name		= "skin";
	s_playerconfig_skin_title.generic.x			= -3*MENU_FONT_SIZE;
	s_playerconfig_skin_title.generic.y			= y += 2*MENU_LINE_SIZE;
	
	s_playerconfig_skin_box.generic.type			= MTYPE_SPINCONTROL;
	s_playerconfig_skin_box.generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_skin_box.generic.x				= -8*MENU_FONT_SIZE;
	s_playerconfig_skin_box.generic.y				= y += MENU_LINE_SIZE;
	s_playerconfig_skin_box.generic.name			= 0;
	s_playerconfig_skin_box.generic.callback		= Menu_PlayerSkinCallback; // Knightmare added, was 0
	s_playerconfig_skin_box.generic.cursor_offset	= -1*MENU_FONT_SIZE;
	s_playerconfig_skin_box.curValue				= sNum;
	s_playerconfig_skin_box.itemNames				= ui_pmi[mNum].skinDisplayNames;
	s_playerconfig_skin_box.generic.flags			|= QMF_SKINLIST;
	
	s_playerconfig_hand_title.generic.type		= MTYPE_SEPARATOR;
	s_playerconfig_hand_title.generic.textSize	= MENU_FONT_SIZE;
	s_playerconfig_hand_title.generic.flags		= QMF_LEFT_JUSTIFY;
	s_playerconfig_hand_title.generic.name		= "handedness";
	s_playerconfig_hand_title.generic.x			= 3*MENU_FONT_SIZE;
	s_playerconfig_hand_title.generic.y			= y += 2*MENU_LINE_SIZE;
	
	s_playerconfig_handedness_box.generic.type			= MTYPE_SPINCONTROL;
	s_playerconfig_handedness_box.generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_handedness_box.generic.x				= -8*MENU_FONT_SIZE;
	s_playerconfig_handedness_box.generic.y				= y += MENU_LINE_SIZE;
	s_playerconfig_handedness_box.generic.name			= 0;
	s_playerconfig_handedness_box.generic.cursor_offset	= -1*MENU_FONT_SIZE;
	s_playerconfig_handedness_box.generic.callback		= Menu_PlayerHandednessCallback;
	s_playerconfig_handedness_box.itemNames				= handedness_names;
	UI_MenuSpinControl_SetValue (&s_playerconfig_handedness_box, "hand", 0, 2, true);
			
	s_playerconfig_rate_title.generic.type		= MTYPE_SEPARATOR;
	s_playerconfig_rate_title.generic.textSize	= MENU_FONT_SIZE;
	s_playerconfig_rate_title.generic.flags		= QMF_LEFT_JUSTIFY;
	s_playerconfig_rate_title.generic.name		= "connect speed";
	s_playerconfig_rate_title.generic.x			= 6*MENU_FONT_SIZE;
	s_playerconfig_rate_title.generic.y			= y += 2*MENU_LINE_SIZE;
		
	s_playerconfig_rate_box.generic.type			= MTYPE_SPINCONTROL;
	s_playerconfig_rate_box.generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_rate_box.generic.x				= -8*MENU_FONT_SIZE;
	s_playerconfig_rate_box.generic.y				= y += MENU_LINE_SIZE;
	s_playerconfig_rate_box.generic.name			= 0;
	s_playerconfig_rate_box.generic.cursor_offset	= -1*MENU_FONT_SIZE;
	s_playerconfig_rate_box.generic.callback		= Menu_PlayerRateCallback;
	s_playerconfig_rate_box.itemNames				= rate_names;
	s_playerconfig_rate_box.itemValues				= rate_values;
	UI_MenuSpinControl_SetValue (&s_playerconfig_rate_box, "rate", 0, 0, false);

	s_playerconfig_back_action.generic.type			= MTYPE_ACTION;
	s_playerconfig_back_action.generic.textSize		= MENU_FONT_SIZE;
	s_playerconfig_back_action.generic.name			= "Back to Multiplayer";
	s_playerconfig_back_action.generic.flags		= QMF_LEFT_JUSTIFY;
	s_playerconfig_back_action.generic.x			= -5*MENU_FONT_SIZE;
	s_playerconfig_back_action.generic.y			= y += 2*MENU_LINE_SIZE;
	s_playerconfig_back_action.generic.statusbar	= NULL;
	s_playerconfig_back_action.generic.callback		= UI_BackMenu;

	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_name_field);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_model_title);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_model_box);
	if ( s_playerconfig_skin_box.itemNames )
	{
		UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_skin_title);
		UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_skin_box);
	}
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_hand_title);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_handedness_box);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_rate_title);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_rate_box);
	UI_AddMenuItem (&s_player_config_menu, &s_playerconfig_back_action);

	return true;
}

//=======================================================================

qboolean Menu_PlayerConfig_CheckIncrement (int dir, float x, float y, float w, float h)
{
	float min[2], max[2], x1, y1, w1, h1;
	char *sound = NULL;

	x1 = x;	y1 = y;	w1 = w;	h1 = h;
	SCR_ScaleCoords (&x1, &y1, &w1, &h1, ALIGN_CENTER);
	min[0] = x1;	max[0] = x1 + w1;
	min[1] = y1;	max[1] = y1 + h1;

	if ( (ui_mousecursor.x >= min[0]) && (ui_mousecursor.x <= max[0]) &&
		(ui_mousecursor.y >= min[1]) && (ui_mousecursor.y <= max[1]) &&
		!ui_mousecursor.buttonused[MOUSEBUTTON1] &&
		ui_mousecursor.buttonclicks[MOUSEBUTTON1]==1)
	{
		if (dir) // dir == 1 is left
		{
			if (s_playerconfig_skin_box.curValue > 0)
				s_playerconfig_skin_box.curValue--;
		}
		else
		{
			if (s_playerconfig_skin_box.curValue < ui_pmi[s_playerconfig_model_box.curValue].nskins)
				s_playerconfig_skin_box.curValue++;
		}

		sound = ui_menu_move_sound;
		ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
		ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 0;

		if ( sound )
			S_StartLocalSound( sound );
		Menu_PlayerSkinCallback (NULL);

		return true;
	}
	return false;
}


void Menu_PlayerConfig_MouseClick (void)
{
	float	icon_x = SCREEN_WIDTH*0.5 - 5, // width - 325
			icon_y = SCREEN_HEIGHT - 108,
			icon_offset = 0;
	int		i, count;
	char	*sound = NULL;
	buttonmenuobject_t buttons[NUM_SKINBOX_ITEMS];

	for (i=0; i<NUM_SKINBOX_ITEMS; i++)
		buttons[i].index =- 1;

	if ( (ui_pmi[s_playerconfig_model_box.curValue].nskins < NUM_SKINBOX_ITEMS) || (s_playerconfig_skin_box.curValue < 4) )
		i = 0;
	else if (s_playerconfig_skin_box.curValue > ui_pmi[s_playerconfig_model_box.curValue].nskins-4)
		i = ui_pmi[s_playerconfig_model_box.curValue].nskins-NUM_SKINBOX_ITEMS;
	else
		i = s_playerconfig_skin_box.curValue-3;

	if (i > 0)
		if (Menu_PlayerConfig_CheckIncrement (1, icon_x-39, icon_y, 32, 32))
			return;

	for (count=0; count<NUM_SKINBOX_ITEMS; i++,count++)
	{
		if ( (i < 0) || (i >= ui_pmi[s_playerconfig_model_box.curValue].nskins) )
			continue;

		UI_AddButton (&buttons[count], i, icon_x+icon_offset, icon_y, 32, 32);
		icon_offset += 34;
	}

	icon_offset = NUM_SKINBOX_ITEMS*34;
	if (ui_pmi[s_playerconfig_model_box.curValue].nskins-i > 0)
		if (Menu_PlayerConfig_CheckIncrement (0, icon_x+icon_offset+5, icon_y, 32, 32))
			return;

	for (i=0; i<NUM_SKINBOX_ITEMS; i++)
	{
		if (buttons[i].index == -1)
			continue;

		if ( (ui_mousecursor.x >= buttons[i].min[0]) && (ui_mousecursor.x <= buttons[i].max[0]) &&
			(ui_mousecursor.y >= buttons[i].min[1]) && (ui_mousecursor.y <= buttons[i].max[1]) )
		{
			if (!ui_mousecursor.buttonused[MOUSEBUTTON1] && ui_mousecursor.buttonclicks[MOUSEBUTTON1]==1)
			{
				s_playerconfig_skin_box.curValue = buttons[i].index;

				sound = ui_menu_move_sound;
				ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
				ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 0;

				if (sound)
					S_StartLocalSound (sound);
				Menu_PlayerSkinCallback (NULL);

				return;
			}
			break;
		}
	}
}


void Menu_PlayerConfig_DrawSkinSelection (void)
{
	char		scratch[MAX_QPATH];
	float		icon_x = SCREEN_WIDTH*0.5 - 5; // width - 325
	float		icon_y = SCREEN_HEIGHT - 108;
	float		icon_offset = 0;
	float		x, y, w, h;
	int			i, count, color[3];
	color_t		arrowColor;
	vec4_t		arrowTemp[2];

	CL_TextColor ((int)Cvar_VariableValue("alt_text_color"), &color[0], &color[1], &color[2]);
	Vector4Copy (stCoord_arrow_left, arrowTemp[0]);
	Vector4Copy (stCoord_arrow_right, arrowTemp[1]);

	if ( (ui_pmi[s_playerconfig_model_box.curValue].nskins < NUM_SKINBOX_ITEMS) || (s_playerconfig_skin_box.curValue < 4) )
		i = 0;
	else if ( s_playerconfig_skin_box.curValue > (ui_pmi[s_playerconfig_model_box.curValue].nskins - 4) )
		i = ui_pmi[s_playerconfig_model_box.curValue].nskins-NUM_SKINBOX_ITEMS;
	else
		i = s_playerconfig_skin_box.curValue - 3;

	// left arrow
	if (i > 0) {
		Vector4Set (arrowColor, color[0], color[1], color[2], 255);
	//	Com_sprintf (scratch, sizeof(scratch), "/gfx/ui/arrows/arrow_left.pcx");
	}
	else {
		Vector4Set (arrowColor, 150, 150, 150, 255);
		arrowTemp[0][1] += 0.25;
		arrowTemp[0][3] += 0.25;
	//	Com_sprintf (scratch, sizeof(scratch), "/gfx/ui/arrows/arrow_left_d.pcx");
	}
	UI_DrawPicST (icon_x-39, icon_y+2, 32, 32, arrowTemp[0], ALIGN_CENTER, false, arrowColor, UI_ARROWS_PIC);
//	UI_DrawPic (icon_x-39, icon_y+2, 32, 32,  ALIGN_CENTER, false, scratch, 1.0);

	// background
	UI_DrawFill (icon_x-3, icon_y-3, NUM_SKINBOX_ITEMS*34+4, 38, ALIGN_CENTER, false, 0, 0, 0, 255);
	if (R_DrawFindPic("/gfx/ui/widgets/listbox_background.pcx")) {
		x = icon_x-2;	y = icon_y-2;	w = NUM_SKINBOX_ITEMS * 34 + 2;	h = 36;
		UI_DrawTiledPic (x, y, w, h, ALIGN_CENTER, true, "/gfx/ui/widgets/listbox_background.pcx", 255);
	//	SCR_ScaleCoords (&x, &y, &w, &h, ALIGN_CENTER);
	//	R_DrawTileClear ((int)x, (int)y, (int)w, (int)h, "/gfx/ui/widgets/listbox_background.pcx");
	}
	else
		UI_DrawFill (icon_x-2, icon_y-2, NUM_SKINBOX_ITEMS*34+2, 36, ALIGN_CENTER, false, 60, 60, 60, 255);
		
	for (count=0; count<NUM_SKINBOX_ITEMS; i++,count++)
	{
		if ( (i < 0) || (i >= ui_pmi[s_playerconfig_model_box.curValue].nskins) )
			continue;

		Com_sprintf (scratch, sizeof(scratch), "/players/%s/%s_i.pcx", 
			ui_pmi[s_playerconfig_model_box.curValue].directory,
			ui_pmi[s_playerconfig_model_box.curValue].skinDisplayNames[i] );

		if (i == s_playerconfig_skin_box.curValue)
			UI_DrawFill (icon_x + icon_offset-1, icon_y-1, 34, 34, ALIGN_CENTER, false, color[0], color[1] ,color[2], 255);
		UI_DrawPic (icon_x + icon_offset, icon_y, 32, 32,  ALIGN_CENTER, false, scratch, 1.0);
		icon_offset += 34;
	}

	// right arrow
	icon_offset = NUM_SKINBOX_ITEMS*34;
	if ( ui_pmi[s_playerconfig_model_box.curValue].nskins-i > 0 ) {
		Vector4Set (arrowColor, color[0], color[1], color[2], 255);
	//	Com_sprintf (scratch, sizeof(scratch), "/gfx/ui/arrows/arrow_right.pcx");
	}
	else {
		Vector4Set (arrowColor, 150, 150, 150, 255);
		arrowTemp[1][1] += 0.25;
		arrowTemp[1][3] += 0.25;
	//	Com_sprintf (scratch, sizeof(scratch), "/gfx/ui/arrows/arrow_right_d.pcx");
	}
	UI_DrawPicST (icon_x+icon_offset+5, icon_y+2, 32, 32, arrowTemp[1], ALIGN_CENTER, false, arrowColor, UI_ARROWS_PIC);
//	UI_DrawPic (icon_x+icon_offset+5, icon_y+2, 32, 32,  ALIGN_CENTER, false, scratch, 1.0);
}


void Menu_PlayerConfig_Draw (void)
{
	refdef_t	refdef;
	float		rx, ry, rw, rh;
	qboolean	lefthand = (Cvar_VariableInteger("hand") == 1);

	UI_DrawBanner ("m_banner_plauer_setup"); // typo for image name is id's fault

	memset(&refdef, 0, sizeof(refdef));

	rx = 0;							ry = 0;
	rw = SCREEN_WIDTH;				rh = SCREEN_HEIGHT;
	SCR_ScaleCoords (&rx, &ry, &rw, &rh, ALIGN_CENTER);
	refdef.x = rx;		refdef.y = ry;
	refdef.width = rw;	refdef.height = rh;
	refdef.fov_x = 50;
	refdef.fov_y = CalcFov (refdef.fov_x, refdef.width, refdef.height);
	refdef.time = cls.realtime*0.001;
 
	if ( ui_pmi[s_playerconfig_model_box.curValue].skinDisplayNames )
	{
		int			yaw;
		int			maxframe = 29;
		vec3_t		modelOrg;
		// Psychopspaz's support for showing weapon model
		entity_t	entity[2], *ent;

		refdef.num_entities = 0;
		refdef.entities = entity;

		yaw = anglemod(cl.time/10);

		VectorSet (modelOrg, 150, -25, 0); // was 80, 0, 0

		// Setup player model
		ent = &entity[0];
		memset (&entity[0], 0, sizeof(entity[0]));

		// moved registration code to init and change only
		ent->model = ui_playermodel;
		ent->skin = ui_playerskin;

		ent->flags = RF_FULLBRIGHT|RF_NOSHADOW|RF_DEPTHHACK;
		if (lefthand)
			ent->flags |= RF_MIRRORMODEL;

		ent->origin[0] = modelOrg[0];
		ent->origin[1] = modelOrg[1];
		ent->origin[2] = modelOrg[2];

		VectorCopy( ent->origin, ent->oldorigin );
		ent->frame = 0;
		ent->oldframe = 0;
		ent->backlerp = 0.0;
		ent->angles[1] = yaw;
		
		refdef.num_entities++;

		// Setup weapon model
		ent = &entity[1];
		memset (&entity[1], 0, sizeof(entity[1]));

		// moved registration code to init and change only
		ent->model = ui_weaponmodel;

		if (ent->model)
		{
			ent->skinnum = 0;

			ent->flags = RF_FULLBRIGHT|RF_NOSHADOW|RF_DEPTHHACK;
			if (lefthand)
				ent->flags |= RF_MIRRORMODEL;

			ent->origin[0] = modelOrg[0];
			ent->origin[1] = modelOrg[1];
			ent->origin[2] = modelOrg[2];

			VectorCopy( ent->origin, ent->oldorigin );
			ent->frame = 0;
			ent->oldframe = 0;
			ent->backlerp = 0.0;
			ent->angles[1] = yaw;
			
			refdef.num_entities++;
		}


		refdef.areabits = 0;
		refdef.lightstyles = 0;
		refdef.rdflags = RDF_NOWORLDMODEL;

		UI_DrawMenu (&s_player_config_menu);

		// skin selection preview
		Menu_PlayerConfig_DrawSkinSelection ();

		R_RenderFrame (&refdef);
	}
}


void Menu_PConfigSaveChanges (void)
{
	int		mNum, sNum;
	char	scratch[1024];

	Cvar_Set ("name", s_playerconfig_name_field.buffer);

	mNum = s_playerconfig_model_box.curValue;
	sNum = s_playerconfig_skin_box.curValue;
	Com_sprintf (scratch, sizeof( scratch ), "%s/%s", 
		ui_pmi[mNum].directory, ui_pmi[mNum].skinDisplayNames[sNum]);
	Cvar_Set ("skin", scratch);
}

//=======================================================================

const char *Menu_PlayerConfig_Key (int key)
{
	if ( key == K_ESCAPE )
		Menu_PConfigSaveChanges ();

	return UI_DefaultMenuKey (&s_player_config_menu, key);
}


void Menu_PlayerConfig_f (void)
{
	UI_RefreshPlayerModels ();	// Reload player models if we recently downloaded anything
	if ( !Menu_PlayerConfig_Init() )
	{
		UI_SetMenuStatusBar (&s_multiplayer_menu, "No valid player models found");
		return;
	}
	UI_SetMenuStatusBar (&s_multiplayer_menu, NULL);
	UI_PushMenu (Menu_PlayerConfig_Draw, Menu_PlayerConfig_Key);
}
