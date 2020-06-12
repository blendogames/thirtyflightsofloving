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

// ui_subsystem.c -- menu support/handling functions

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

void	(*m_drawfunc) (void);
const char *(*m_keyfunc) (int key);

qboolean hasitem;

/*
=======================================================================

SUPPORT ROUTINES

=======================================================================
*/

/*
=================
FreeFileList
=================
*/
void FreeFileList (char **list, int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		if (list && list[i])
		{
			free(list[i]);
			list[i] = 0;
		}
	}
	free(list);
}

/*
=================
ItemInList
=================
*/
qboolean ItemInList (char *check, int num, char **list)
{
	int i;
	for (i=0;i<num;i++)
		if (!Q_strcasecmp(check, list[i]))
			return true;
	return false;
}

/*
=================
InsertInList
=================
*/
void InsertInList (char **list, char *insert, int len, int start)
{
	int i;
	if (!list) return;

	for (i=start; i<len; i++)
	{
		if (!list[i])
		{
			list[i] = strdup(insert);
			return;
		}
	}
	list[len] = strdup(insert);
}


/*
=======================================================================

MENU SUBSYSTEM

=======================================================================
*/

#define	MAX_MENU_DEPTH	8


typedef struct
{
	void	(*draw) (void);
	const char *(*key) (int k);
} menulayer_t;


menulayer_t	m_layers[MAX_MENU_DEPTH];
int		m_menudepth;


/*
=================
UI_AddButton
From Q2max
=================
*/
void UI_AddButton (buttonmenuobject_t *thisObj, int index, float x, float y, float w, float h)
{
	float x1, y1, w1, h1;

	x1 = x;	y1 = y;	w1 = w;	h1 = h;
	SCR_AdjustFrom640 (&x1, &y1, &w1, &h1, ALIGN_CENTER);
	thisObj->min[0] = x1;	thisObj->max[0] = x1 + w1;
	thisObj->min[1] = y1;	thisObj->max[1] = y1 + h1;
	thisObj->index = index;
}


/*
=============
UI_AddMainButton
From Q2max
=============
*/
void UI_AddMainButton (mainmenuobject_t *thisObj, int index, int x, int y, char *name)
{
	int		w, h;
	float	x1, y1, w1, h1;

	R_DrawGetPicSize (&w, &h, name);
	
	x1 = x; y1 = y; w1 = w; h1 = h;
	SCR_AdjustFrom640 (&x1, &y1, &w1, &h1, ALIGN_CENTER);
	thisObj->min[0] = x1;	thisObj->max[0] = x1 + w1;
	thisObj->min[1] = y1;	thisObj->max[1] = y1 + h1;

	switch (index)
	{
	case 0:
		thisObj->OpenMenu = M_Menu_Game_f;
	case 1:
		thisObj->OpenMenu = M_Menu_Multiplayer_f;
	case 2:
		thisObj->OpenMenu = M_Menu_Options_f;
	case 3:
		thisObj->OpenMenu = M_Menu_Video_f;
	case 4:
		thisObj->OpenMenu = M_Menu_Quit_f;
	}
}


/*
=================
UI_RefreshCursorButtons
From Q2max
=================
*/
void UI_RefreshCursorButtons(void)
{
	cursor.buttonused[MOUSEBUTTON2] = true;
	cursor.buttondown[MOUSEBUTTON1] = false;
	cursor.buttonclicks[MOUSEBUTTON2] = 0;
	cursor.buttonused[MOUSEBUTTON1] = true;
	cursor.buttondown[MOUSEBUTTON2] = false;
	cursor.buttonclicks[MOUSEBUTTON1] = 0;
}

/*
=================
UI_PushMenu
=================
*/
void UI_PushMenu ( void (*draw) (void), const char *(*key) (int k) )
{
	int		i;

	if (Cvar_VariableValue ("maxclients") == 1 && Com_ServerState () && !cls.consoleActive) // Knightmare added
		Cvar_Set ("paused", "1");

	// Knightmare- if just opened menu, and ingame and not DM, grab screen first
	if (cls.key_dest != key_menu && !Cvar_VariableValue("deathmatch")
		&& Com_ServerState() == 2) //ss_game
		//&& !cl.cinematictime && Com_ServerState())
		R_GrabScreen();

	// if this menu is already present, drop back to that level
	// to avoid stacking menus by hotkeys
	for (i=0 ; i<m_menudepth ; i++)
		if (m_layers[i].draw == draw &&
			m_layers[i].key == key)
		{
			m_menudepth = i;
		}

	if (i == m_menudepth)
	{
		if (m_menudepth >= MAX_MENU_DEPTH)
			Com_Error (ERR_FATAL, "UI_PushMenu: MAX_MENU_DEPTH");
		m_layers[m_menudepth].draw = m_drawfunc;
		m_layers[m_menudepth].key = m_keyfunc;
		m_menudepth++;
	}

	m_drawfunc = draw;
	m_keyfunc = key;

	m_entersound = true;

	// Knightmare- added Psychospaz's mouse support
	UI_RefreshCursorLink();
	UI_RefreshCursorButtons();

	cls.key_dest = key_menu;
}

/*
=================
UI_ForceMenuOff
=================
*/
void UI_ForceMenuOff (void)
{
	// Knightmare- added Psychospaz's mouse support
	UI_RefreshCursorLink();
	m_drawfunc = 0;
	m_keyfunc = 0;
	cls.key_dest = key_game;
	m_menudepth = 0;
	Key_ClearStates ();
	if (!cls.consoleActive && Cvar_VariableValue ("maxclients") == 1 && Com_ServerState()) // Knightmare added
		Cvar_Set ("paused", "0");
}

/*
=================
UI_PopMenu
=================
*/
void UI_PopMenu (void)
{
	S_StartLocalSound( menu_out_sound );
	if (m_menudepth < 1)
		Com_Error (ERR_FATAL, "UI_PopMenu: depth < 1");
	m_menudepth--;

	m_drawfunc = m_layers[m_menudepth].draw;
	m_keyfunc = m_layers[m_menudepth].key;

	// Knightmare- added Psychospaz's mouse support
	UI_RefreshCursorLink();
	UI_RefreshCursorButtons();

	if (!m_menudepth)
		UI_ForceMenuOff ();
}

/*
=================
UI_BackMenu
=================
*/
void UI_BackMenu (void *unused)
{
	UI_PopMenu();
}

/*
=================
Default_MenuKey
=================
*/
const char *Default_MenuKey ( menuframework_s *m, int key )
{
	const char *sound = NULL;
	menucommon_s *item;

	if ( m )
	{
		if ( ( item = Menu_ItemAtCursor( m ) ) != 0 )
		{
			if ( item->type == MTYPE_FIELD )
			{
				if ( Field_Key( ( menufield_s * ) item, key ) )
					return NULL;
			}
		}
	}

	switch ( key )
	{

	case K_JOY2: //BC B button.
	case K_BACKSPACE:
	case K_ESCAPE:
		UI_PopMenu();
		return menu_out_sound;

	case K_AUX29: //BC dpad up.
	case K_KP_UPARROW:
	case K_UPARROW:
		if ( m )
		{
			m->cursor--;
			// Knightmare- added Psychospaz's mouse support
			UI_RefreshCursorLink();

			Menu_AdjustCursor( m, -1 );
			sound = menu_move_sound;
		}
		break;

	case K_AUX31: //BC dpad down.
	case K_TAB:
	case K_KP_DOWNARROW:
	case K_DOWNARROW:
		if ( m )
		{
			m->cursor++;
			// Knightmare- added Psychospaz's mouse support

			UI_RefreshCursorLink();
			Menu_AdjustCursor( m, 1 );
			sound = menu_move_sound;
		}
		break;

	case K_AUX32: //BC dpad left.
	case K_KP_LEFTARROW:
	case K_LEFTARROW:
		if ( m )
		{
			Menu_SlideItem( m, -1 );
			sound = menu_move_sound;
		}
		break;

	case K_AUX30: //BC dpad right.
	case K_KP_RIGHTARROW:
	case K_RIGHTARROW:
		if ( m )
		{
			Menu_SlideItem( m, 1 );
			sound = menu_move_sound;
		}
		break;

	/*case K_MOUSE1:
	case K_MOUSE2:
	case K_MOUSE3:
	//Knightmare 12/22/2001
	case K_MOUSE4:
	case K_MOUSE5:*/
	//end Knightmare
	case K_JOY1:
	//case K_JOY2:
	case K_JOY3:
	case K_JOY4:
	case K_AUX1:
	case K_AUX2:
	case K_AUX3:
	case K_AUX4:
	case K_AUX5:
	case K_AUX6:
	case K_AUX7:
	case K_AUX8:
	case K_AUX9:
	case K_AUX10:
	case K_AUX11:
	case K_AUX12:
	case K_AUX13:
	case K_AUX14:
	case K_AUX15:
	case K_AUX16:
	case K_AUX17:
	case K_AUX18:
	case K_AUX19:
	case K_AUX20:
	case K_AUX21:
	case K_AUX22:
	case K_AUX23:
	case K_AUX24:
	case K_AUX25:
	case K_AUX26:
	case K_AUX27:
	case K_AUX28:
	//case K_AUX29:
	//case K_AUX30:
	//case K_AUX31:
	//case K_AUX32:

	case K_SPACE:		
	case K_KP_ENTER:
	case K_ENTER:
		if ( m )
		{
			if ( item->type == MTYPE_CHECKBOX )
			{
				
				Menu_SlideItem( m, 1 );
				sound = menu_click;
			}
			else 
			{
				
				Menu_SelectItem( m );

				if ( item->type == MTYPE_ACTION )
					sound = menu_move_sound;
				
			}
		}
			
		
		break;
	}

	return sound;
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

	UI_LoadMapList();	// load map list
	
	Cmd_AddCommand ("menu_main", M_Menu_Main_f);
	Cmd_AddCommand ("menu_game", M_Menu_Game_f);
		Cmd_AddCommand ("menu_loadgame", M_Menu_LoadGame_f);
		Cmd_AddCommand ("menu_savegame", M_Menu_SaveGame_f);
		Cmd_AddCommand ("menu_credits", M_Menu_Credits_f );
	Cmd_AddCommand ("menu_multiplayer", M_Menu_Multiplayer_f );
		Cmd_AddCommand ("menu_joinserver", M_Menu_JoinServer_f);
			Cmd_AddCommand ("menu_addressbook", M_Menu_AddressBook_f);
		Cmd_AddCommand ("menu_startserver", M_Menu_StartServer_f);
			Cmd_AddCommand ("menu_dmoptions", M_Menu_DMOptions_f);
		Cmd_AddCommand ("menu_playerconfig", M_Menu_PlayerConfig_f);
		Cmd_AddCommand ("menu_downloadoptions", M_Menu_DownloadOptions_f);
	Cmd_AddCommand ("menu_video", M_Menu_Video_f);
		Cmd_AddCommand ("menu_video_advanced", M_Menu_Video_Advanced_f);
	Cmd_AddCommand ("menu_options", M_Menu_Options_f);
		Cmd_AddCommand ("menu_sound", M_Menu_Options_Sound_f);
		Cmd_AddCommand ("menu_controls", M_Menu_Options_Controls_f);
			Cmd_AddCommand ("menu_keys", M_Menu_Keys_f);
		Cmd_AddCommand ("menu_ingame", M_Menu_Options_Ingame_f);
		Cmd_AddCommand ("menu_interface", M_Menu_Options_Interface_f);
	Cmd_AddCommand ("menu_quit", M_Menu_Quit_f);
}



/*
=======================================================================

Menu Mouse Cursor - psychospaz

=======================================================================
*/

/*
=================
UI_RefreshCursorMenu
=================
*/
void UI_RefreshCursorMenu (void)
{
	cursor.menu = NULL;
}

/*
=================
UI_RefreshCursorLink
=================
*/
void UI_RefreshCursorLink (void)
{
	cursor.menuitem = NULL;
}

/*
=================
Slider_CursorPositionX
=================
*/
int Slider_CursorPositionX (menuslider_s *s)
{
	float range;

	range = (s->curvalue - s->minvalue) / (float)(s->maxvalue - s->minvalue);

	if (range < 0)
		range = 0;
	if (range > 1)
		range = 1;

	return (int)(SCR_ScaledVideo(MENU_FONT_SIZE) + RCOLUMN_OFFSET + (SLIDER_RANGE)*SCR_ScaledVideo(MENU_FONT_SIZE) * range);
}

/*
=================
NewSliderValueForX
=================
*/
int NewSliderValueForX (int x, menuslider_s *s)
{
	float	newValue, sliderbase;
	int		newValueInt;
	int		pos;
	
	sliderbase = s->generic.x + s->generic.parent->x + MENU_FONT_SIZE + RCOLUMN_OFFSET;
	SCR_AdjustFrom640 (&sliderbase, NULL, NULL, NULL, ALIGN_CENTER);
	pos = x - sliderbase;
//	pos = x - SCR_ScaledVideo(s->generic.x + s->generic.parent->x + MENU_FONT_SIZE + RCOLUMN_OFFSET);

	newValue = ((float)pos)/((SLIDER_RANGE-1)*SCR_ScaledVideo(MENU_FONT_SIZE));
	newValueInt = s->minvalue + newValue * (float)(s->maxvalue - s->minvalue);

	return newValueInt;
}

/*
=================
Slider_CheckSlide
=================
*/
void Slider_CheckSlide (menuslider_s *s)
{
	if (s->curvalue > s->maxvalue)
		s->curvalue = s->maxvalue;
	else if (s->curvalue < s->minvalue)
		s->curvalue = s->minvalue;

	if (s->generic.callback)
		s->generic.callback( s );
}

/*
=================
Menu_DragSlideItem
=================
*/
void Menu_DragSlideItem (menuframework_s *menu, void *menuitem)
{
	menucommon_s *item = (menucommon_s *) menuitem;
	menuslider_s *slider = (menuslider_s *) menuitem;

	slider->curvalue = NewSliderValueForX(cursor.x, slider);
	Slider_CheckSlide (slider);
}

/*
=================
Menu_ClickSlideItem
=================
*/
void Menu_ClickSlideItem (menuframework_s *menu, void *menuitem)
{
	int				min, max;
	float			x, w;
	menucommon_s	*item = (menucommon_s *) menuitem;
	menuslider_s	*slider = (menuslider_s *) menuitem;

	x = menu->x + item->x + Slider_CursorPositionX(slider) - 4;
	w = 8;
	SCR_AdjustFrom640 (&x, NULL, &w, NULL, ALIGN_CENTER);
	min = x;	max = x + w;
//	min = SCR_ScaledVideo(menu->x + item->x + Slider_CursorPositionX(slider) - 4);
//	max = SCR_ScaledVideo(menu->x + item->x + Slider_CursorPositionX(slider) + 4);

	if (cursor.x < min)
		Menu_SlideItem( menu, -1 );
	if (cursor.x > max)
		Menu_SlideItem( menu, 1 );
}


/*
=================
UI_Think_MouseCursor
=================
*/
void UI_Think_MouseCursor (void)
{
	char * sound = NULL;
	menuframework_s *m = (menuframework_s *)cursor.menu;

	if (m_drawfunc == M_Main_Draw) //have to hack for main menu :p
	{
		UI_CheckMainMenuMouse();
		return;
	}
	if (m_drawfunc == M_Credits_MenuDraw) //have to hack for credits :p
	{
		if ((cursor.buttonclicks[MOUSEBUTTON1])||(cursor.buttonclicks[MOUSEBUTTON2]))
		{
			cursor.buttonused[MOUSEBUTTON2] = true;
			cursor.buttonclicks[MOUSEBUTTON2] = 0;
			cursor.buttonused[MOUSEBUTTON1] = true;
			cursor.buttonclicks[MOUSEBUTTON1] = 0;
			S_StartLocalSound( menu_out_sound );
			if (creditsBuffer)
				FS_FreeFile (creditsBuffer);
			UI_PopMenu();
			return;
		}
	}
	else if (m_drawfunc == M_Quit_Draw) //hack for quit menu
	{
		if (cursor.buttonclicks[MOUSEBUTTON2])
		{
			cursor.buttonused[MOUSEBUTTON2] = true;
			cursor.buttonclicks[MOUSEBUTTON2] = 0;
			cursor.buttonused[MOUSEBUTTON1] = true;
			cursor.buttonclicks[MOUSEBUTTON1] = 0;

			S_StartLocalSound ("world/cheer.wav");



			UI_PopMenu ();
			
			return;
		}
	}



	//mouse clicking on the player model menu...
	if (m_drawfunc == PlayerConfig_MenuDraw)
		PlayerConfig_MouseClick();
	if (m_drawfunc == Options_Ingame_MenuDraw)
		MenuCrosshair_MouseClick();

	if (!m)
		return;

	//Exit with double click 2nd mouse button

	if (cursor.menuitem)
	{
		
		if (!hasitem)
		{
			sound = menu_move_sound;
			hasitem=true;
		}

		//MOUSE1
		if (cursor.buttondown[MOUSEBUTTON1])
		{
			if (cursor.menuitemtype == MENUITEM_SLIDER)
			{
				Menu_DragSlideItem(m, cursor.menuitem);
			}
			else if (!cursor.buttonused[MOUSEBUTTON1] && cursor.buttonclicks[MOUSEBUTTON1])
			{
				if (cursor.menuitemtype == MENUITEM_ROTATE)
				{
					if (menu_rotate->value)					
						Menu_SlideItem( m, -1 );
					else			
						Menu_SlideItem( m, 1 );


					//zzzzzz
					//if ( item->type == MTYPE_CHECKBOX )
					//sound = menu_move_sound;
					sound = menu_click;

					cursor.buttonused[MOUSEBUTTON1] = true;
				}
				else
				{
					cursor.buttonused[MOUSEBUTTON1] = true;
					Menu_MouseSelectItem( cursor.menuitem );
					sound = menu_move_sound;
				}
			}
		}
		//MOUSE2
		if (cursor.buttondown[MOUSEBUTTON2] && cursor.buttonclicks[MOUSEBUTTON2])
		{
			if (cursor.menuitemtype == MENUITEM_SLIDER && !cursor.buttonused[MOUSEBUTTON2])
			{
				Menu_ClickSlideItem(m, cursor.menuitem);
				sound = menu_move_sound;
				cursor.buttonused[MOUSEBUTTON2] = true;
			}
			else if (!cursor.buttonused[MOUSEBUTTON2])
			{
				if (cursor.menuitemtype == MENUITEM_ROTATE)
				{
					if (menu_rotate->value)					
						Menu_SlideItem( m, 1 );
					else			
						Menu_SlideItem( m, -1 );

					sound = menu_move_sound;
					cursor.buttonused[MOUSEBUTTON2] = true;
				}
			}
		}
	}
	else if (!cursor.buttonused[MOUSEBUTTON2] && cursor.buttonclicks[MOUSEBUTTON2]==2 && cursor.buttondown[MOUSEBUTTON2])
	{
		if (m_drawfunc==PlayerConfig_MenuDraw)
			PConfigAccept();

		UI_PopMenu();

		sound = menu_out_sound;
		cursor.buttonused[MOUSEBUTTON2] = true;
		cursor.buttonclicks[MOUSEBUTTON2] = 0;
		cursor.buttonused[MOUSEBUTTON1] = true;
		cursor.buttonclicks[MOUSEBUTTON1] = 0;
	}
	else
	{
		if (hasitem==true)
			hasitem=false;
	}

	if ( sound )
		S_StartLocalSound( sound );
}


/*
=================
UI_Draw_Cursor
=================
*/
void UI_Draw_Cursor (void)
{
	float alpha = 1, scale = SCR_ScaledVideo(1)*0.5;
	int w,h;
	char *overlay = NULL;
	char *cur_img = NULL;

	if (m_drawfunc == M_Main_Draw)
	{
		if (MainMenuMouseHover)
		{
			if ((cursor.buttonused[0] && cursor.buttonclicks[0])
				|| (cursor.buttonused[1] && cursor.buttonclicks[1]))
			{
				cur_img = "/gfx/m_cur_click.pcx";
				alpha = 0.85 + 0.15*sin(anglemod(cl.time*0.005));
			}
			else
			{
				cur_img = "/gfx/m_cur_hover.pcx";
				alpha = 0.85 + 0.15*sin(anglemod(cl.time*0.005));
			}
		}
		else
			cur_img = "/gfx/m_cur_main.pcx";

		//overlay = "/gfx/m_cur_over.pcx";
	}
	else
	{
		if (cursor.menuitem)
		{
			if (cursor.menuitemtype == MENUITEM_TEXT)
			{
				cur_img = "/gfx/m_cur_text.pcx";
			}
			else
			{
				if ((cursor.buttonused[0] && cursor.buttonclicks[0])
					|| (cursor.buttonused[1] && cursor.buttonclicks[1]))
				{
					cur_img = "/gfx/m_cur_click.pcx";
					alpha = 0.85 + 0.15*sin(anglemod(cl.time*0.005));
				}
				else
				{
					cur_img = "/gfx/m_cur_hover.pcx";
					alpha = 0.85 + 0.15*sin(anglemod(cl.time*0.005));
				}
				//overlay = "/gfx/m_cur_over.pcx";
			}
		}
		else
		{
			cur_img = "/gfx/m_cur_main.pcx";
			//overlay = "/gfx/m_cur_over.pcx";
		}
	}
	
	if (cur_img)
	{
		R_DrawGetPicSize( &w, &h, cur_img );
		R_DrawScaledPic( cursor.x - scale*w/2, cursor.y - scale*h/2, scale, 1.0, cur_img);

		/*
		if (overlay)
		{
			R_DrawGetPicSize( &w, &h, overlay );
			R_DrawScaledPic( cursor.x - scale*w/2, cursor.y - scale*h/2, scale, 1, overlay);
		}
		*/
	}
}


/*void UI_Draw_Cursor (void)
{
	int w,h;

	//get sizing vars
	R_DrawGetPicSize( &w, &h, "m_mouse_cursor" );
	w = SCR_ScaledVideo(w)*0.5;
	h = SCR_ScaledVideo(h)*0.5;
	R_DrawStretchPic (cursor.x-w/2, cursor.y-h/2, w, h, "m_mouse_cursor", 1.0);
}*/


/*
=================
UI_Draw
=================
*/
void UI_Draw (void)
{
	if (cls.key_dest != key_menu)
		return;

	// scaled menu stuff
	//SCR_InitScreenScale();

	// repaint everything next frame
	SCR_DirtyScreen ();

	// dim everything behind it down
	if (cl.cinematictime > 0 || cls.state == ca_disconnected)
	{
		//bc removing this background!!!
		/*
		if (R_DrawFindPic("/gfx/menu_background.pcx")) {
			R_DrawStretchPic (0, 0, viddef.width, viddef.height, "/gfx/menu_background.pcx", 1.0);
			//R_DrawFadeScreen ();
		}
		else*/
			R_DrawFill2 (0,0,viddef.width, viddef.height, 0,0,0,255);
	}
	// ingame menu uses alpha
	//else if (R_DrawFindPic("/gfx/menu_background.pcx"))
	//	R_DrawStretchPic (0, 0, viddef.width, viddef.height, "/gfx/menu_background.pcx", menu_alpha->value);
	else
		R_DrawFadeScreen ();

	// Knigthmare- added Psychospaz's mouse support
	UI_RefreshCursorMenu();

	m_drawfunc ();

	// delay playing the enter sound until after the
	// menu has been drawn, to avoid delay while
	// caching images
	if (m_entersound)
	{
		S_StartLocalSound( menu_in_sound );
		m_entersound = false;
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
