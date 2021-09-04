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

// ui_mouse.c -- mouse support code

#include <string.h>
#include <ctype.h>

#include "../client/client.h"
#include "ui_local.h"

cursor_t ui_mousecursor;

/*
=======================================================================

Menu Mouse Cursor - psychospaz

=======================================================================
*/

/*
=================
UI_RefreshCursorButtons
From Q2max
=================
*/
void UI_RefreshCursorButtons (void)
{
	ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
	ui_mousecursor.buttondown[MOUSEBUTTON1] = false;
	ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 0;
	ui_mousecursor.buttonused[MOUSEBUTTON2] = true;
	ui_mousecursor.buttondown[MOUSEBUTTON2] = false;
	ui_mousecursor.buttonclicks[MOUSEBUTTON2] = 0;
}


/*
=================
UI_RefreshCursorMenu
=================
*/
void UI_RefreshCursorMenu (void)
{
	ui_mousecursor.menu = NULL;
}


/*
=================
UI_RefreshCursorLink
=================
*/
void UI_RefreshCursorLink (void)
{
	ui_mousecursor.menuitem = NULL;
}


/*
=================
UI_Slider_CursorPositionX
=================
*/
int UI_Slider_CursorPositionX (menuslider_s *s)
{
	float range;

	if (!s)
		return 0;

	range = (float)s->curPos / (float)s->maxPos;

	if (range < 0)
		range = 0;
	if (range > 1)
		range = 1;

//	return (int)(s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET + MENU_FONT_SIZE + SLIDER_RANGE*MENU_FONT_SIZE*range);
	return (int)(s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET + SLIDER_ENDCAP_WIDTH + SLIDER_RANGE*SLIDER_SECTION_WIDTH*range);
}


/*
=================
UI_SliderValueForX
=================
*/
int UI_SliderValueForX (menuslider_s *s, int x)
{
	float	newValue, sliderbase;
	int		newValueInt;
	int		pos;

	if (!s)
		return 0;

//	sliderbase = s->generic.x + s->generic.parent->x + MENU_FONT_SIZE + RCOLUMN_OFFSET;
	sliderbase = s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET + SLIDER_ENDCAP_WIDTH;
	SCR_ScaleCoords (&sliderbase, NULL, NULL, NULL, ALIGN_CENTER);
	pos = x - sliderbase;
//	pos = x - SCR_ScaledScreen(s->generic.x + s->generic.parent->x + MENU_FONT_SIZE + RCOLUMN_OFFSET);

//	newValue = ((float)pos) / ((SLIDER_RANGE-1) * SCR_ScaledScreen(MENU_FONT_SIZE));
	newValue = ((float)pos) / (SCR_ScaledScreen(SLIDER_RANGE*SLIDER_SECTION_WIDTH));
	newValue = min(newValue, 1.0f);
	newValueInt = newValue * (float)(s->maxPos);

	return newValueInt;
}


/*
=================
UI_Slider_CheckSlide
=================
*/
void UI_Slider_CheckSlide (menuslider_s *s)
{
	if (!s)
		return;

	s->curPos = min(max(s->curPos, 0), s->maxPos);

	if (s->generic.callback)
		s->generic.callback (s);
}


/*
=================
UI_DragSlideItem
=================
*/
void UI_DragSlideItem (menuframework_s *menu, void *menuitem)
{
	menuslider_s *slider;

	if (!menu || !menuitem)
		return;

	slider = (menuslider_s *) menuitem;

	slider->curPos = UI_SliderValueForX(slider, ui_mousecursor.x);
	UI_Slider_CheckSlide (slider);
}


/*
=================
UI_ClickSlideItem
=================
*/
void UI_ClickSlideItem (menuframework_s *menu, void *menuitem)
{
	int				min, max;
	float			x, w;
	menuslider_s	*slider;
	
	if (!menu || !menuitem)
		return;

	slider = (menuslider_s *)menuitem;

//	x = menu->x + item->x + UI_Slider_CursorPositionX(slider) - 4;
//	w = 8;
	x = UI_Slider_CursorPositionX(slider) - (SLIDER_KNOB_WIDTH/2);
	w = SLIDER_KNOB_WIDTH;
	SCR_ScaleCoords (&x, NULL, &w, NULL, ALIGN_CENTER);
	min = x;	max = x + w;

	if (ui_mousecursor.x < min)
		UI_SlideMenuItem (menu, -1);
	if (ui_mousecursor.x > max)
		UI_SlideMenuItem (menu, 1);
}


/*
=================
UI_CheckSlider_Mouseover
=================
*/
qboolean UI_CheckSlider_Mouseover (menuframework_s *menu, void *menuitem)
{
	int				min[2], max[2];
	float			x1, y1, x2, y2;
	menuslider_s	*s;

	if (!menu || !menuitem)
		return false;

	s = (menuslider_s *)menuitem;

	x1 = s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET;
	y1 = s->generic.y + s->generic.parent->y;
	x2 = x1 + SLIDER_ENDCAP_WIDTH + SLIDER_RANGE*SLIDER_SECTION_WIDTH + SLIDER_ENDCAP_WIDTH;
	y2 = y1 + SLIDER_HEIGHT;

	SCR_ScaleCoords (&x1, &y1, NULL, NULL, ALIGN_CENTER);
	SCR_ScaleCoords (&x2, &y2, NULL, NULL, ALIGN_CENTER);
	min[0] = x1;	max[0] = x2;
	min[1] = y1;	max[1] = y2;

	if ( ui_mousecursor.x >= min[0] && ui_mousecursor.x <= max[0] 
		&& ui_mousecursor.y >= min[1] &&  ui_mousecursor.y <= max[1] )
		return true;
	else
		return false;
}


/*
=================
UI_Mouseover_Check
=================
*/
void UI_Mouseover_Check (menuframework_s *menu)
{
	int				i;
	menucommon_s	*item, *lastitem;

	ui_mousecursor.menu = menu;

	if (ui_mousecursor.mouseaction)
	{
		lastitem = ui_mousecursor.menuitem;
		UI_RefreshCursorLink();

		for (i = menu->nitems; i >= 0 ; i--)
		{
			int		type, len;
			int		min[2], max[2];
			float	x1, y1, w1, h1;

			item = ((menucommon_s * )menu->items[i]);

			if (!item || item->type == MTYPE_SEPARATOR)
				continue;

			x1 = menu->x + item->x + RCOLUMN_OFFSET; // + 2 chars for space + cursor
			y1 = menu->y + item->y;
			w1 = 0;			h1 = item->textSize;	// MENU_FONT_SIZE
			SCR_ScaleCoords (&x1, &y1, &w1, &h1, ALIGN_CENTER);
			min[0] = x1;	max[0] = x1 + w1;
			min[1] = y1;	max[1] = y1 + h1;
		//	max[0] = min[0] = SCR_ScaledScreen(menu->x + item->x + RCOLUMN_OFFSET); //+ 2 chars for space + cursor
		//	max[1] = min[1] = SCR_ScaledScreen(menu->y + item->y);
		//	max[1] += SCR_ScaledScreen(MENU_FONT_SIZE);

			switch (item->type)
			{
				case MTYPE_ACTION:
					{
						len = (int)strlen(item->name);
						
						if (item->flags & QMF_LEFT_JUSTIFY)
						{
							min[0] += SCR_ScaledScreen(LCOLUMN_OFFSET*2);
						//	max[0] = min[0] + SCR_ScaledScreen(len*MENU_FONT_SIZE);
							max[0] = min[0] + SCR_ScaledScreen(len*item->textSize);
						}
						else
						//	min[0] -= SCR_ScaledScreen(len*MENU_FONT_SIZE + MENU_FONT_SIZE*3);
							min[0] -= SCR_ScaledScreen(len*item->textSize + item->textSize*3);

						type = MENUITEM_ACTION;
					}
					break;
				case MTYPE_SLIDER:
					{
						if (item->name)
						{
							len = (int)strlen(item->name);
						//	min[0] -= SCR_ScaledScreen(len*MENU_FONT_SIZE - LCOLUMN_OFFSET*2);
							min[0] -= SCR_ScaledScreen(len*item->textSize - LCOLUMN_OFFSET*2);
						}
						else
							min[0] -= SCR_ScaledScreen(16);
					//	max[0] += SCR_ScaledScreen((SLIDER_RANGE + 4) * MENU_FONT_SIZE);
						max[0] += SCR_ScaledScreen((SLIDER_RANGE + 4) * item->textSize);
						type = MENUITEM_SLIDER;
					}
					break;
				case MTYPE_LIST:
				case MTYPE_SPINCONTROL:
					{
						int len;
						menulist_s *spin = menu->items[i];


						if (item->name)
						{
							len = (int)strlen(item->name);
						//	min[0] -= SCR_ScaledScreen(len*MENU_FONT_SIZE - LCOLUMN_OFFSET*2);
							min[0] -= SCR_ScaledScreen(len*item->textSize - LCOLUMN_OFFSET*2);
						}

						len = (int)strlen(spin->itemNames[spin->curValue]);
					//	max[0] += SCR_ScaledScreen(len*MENU_FONT_SIZE);
						max[0] += SCR_ScaledScreen(len*item->textSize);

						type = MENUITEM_ROTATE;
					}
					break;
				case MTYPE_FIELD:
					{
						menufield_s *text = menu->items[i];

						len = text->visible_length + 2;

					//	max[0] += SCR_ScaledScreen(len*MENU_FONT_SIZE);
						max[0] += SCR_ScaledScreen(len*item->textSize);
						type = MENUITEM_TEXT;
					}
					break;
				default:
					continue;
			}

			if (ui_mousecursor.x >= min[0] && 
				ui_mousecursor.x <= max[0] &&
				ui_mousecursor.y >= min[1] && 
				ui_mousecursor.y <= max[1])
			{
				// new item
				if (lastitem!=item)
				{
					int j;

					for (j=0; j<MENU_CURSOR_BUTTON_MAX; j++)
					{
						ui_mousecursor.buttonclicks[j] = 0;
						ui_mousecursor.buttontime[j] = 0;
					}
				}

				ui_mousecursor.menuitem = item;
				ui_mousecursor.menuitemtype = type;
				
				menu->cursor = i;

				break;
			}
		}
	}

	ui_mousecursor.mouseaction = false;
}


/*
=================
UI_MouseCursor_Think
=================
*/
void UI_MouseCursor_Think (void)
{
	char * sound = NULL;
	menuframework_s *m = (menuframework_s *)ui_mousecursor.menu;

	if (m_drawfunc == Menu_Main_Draw) // have to hack for main menu :p
	{
		UI_CheckMainMenuMouse ();
		return;
	}
	if (m_drawfunc == Menu_Credits_Draw) // have to hack for credits :p
	{
		if (ui_mousecursor.buttonclicks[MOUSEBUTTON2])
		{
			ui_mousecursor.buttonused[MOUSEBUTTON2] = true;
			ui_mousecursor.buttonclicks[MOUSEBUTTON2] = 0;
			ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
			ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 0;
			S_StartLocalSound (menu_out_sound);
			if (creditsBuffer)
				FS_FreeFile (creditsBuffer);
			UI_PopMenu();
			return;
		}
	}

/*	// clicking on the player model menu...
	if (m_drawfunc == Menu_PlayerConfig_Draw)
		Menu_PlayerConfig_MouseClick ();
	// clicking on the screen menu
	if (m_drawfunc == Menu_Options_Screen_Draw)
		Menu_Options_Screen_Crosshair_MouseClick ();
*/
	if (!m)
		return;

	// Exit with double click 2nd mouse button

	if (ui_mousecursor.menuitem)
	{
		// MOUSE1
		if (ui_mousecursor.buttondown[MOUSEBUTTON1])
		{
			if (ui_mousecursor.menuitemtype == MENUITEM_SLIDER && !ui_mousecursor.buttonused[MOUSEBUTTON1])
			{
				if ( UI_CheckSlider_Mouseover(m, ui_mousecursor.menuitem) ) {
					UI_DragSlideItem (m, ui_mousecursor.menuitem);
					sound = menu_drag_sound;
				}
				else {
					UI_SlideMenuItem (m, 1);
					sound = menu_move_sound;
					ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
				}
			}
			else if (!ui_mousecursor.buttonused[MOUSEBUTTON1] && ui_mousecursor.buttonclicks[MOUSEBUTTON1])
			{
				if (ui_mousecursor.menuitemtype == MENUITEM_ROTATE)
				{
					if (ui_item_rotate->integer)					
						UI_SlideMenuItem (m, -1);
					else			
						UI_SlideMenuItem (m, 1);

					sound = menu_move_sound;
					ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
				}
				else
				{
					ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
					UI_MouseSelectItem (ui_mousecursor.menuitem);
					sound = menu_move_sound;
				}
			}
		}
		// MOUSE2
		if (ui_mousecursor.buttondown[MOUSEBUTTON2] && ui_mousecursor.buttonclicks[MOUSEBUTTON2])
		{
			if (ui_mousecursor.menuitemtype == MENUITEM_SLIDER && !ui_mousecursor.buttonused[MOUSEBUTTON2])
			{
				if ( UI_CheckSlider_Mouseover(m, ui_mousecursor.menuitem) ) {
					UI_ClickSlideItem (m, ui_mousecursor.menuitem);
				}
				else {
					UI_SlideMenuItem (m, -1);
				}
				sound = menu_move_sound;
				ui_mousecursor.buttonused[MOUSEBUTTON2] = true;
			}
			else if (!ui_mousecursor.buttonused[MOUSEBUTTON2])
			{
				if (ui_mousecursor.menuitemtype == MENUITEM_ROTATE)
				{
					if (ui_item_rotate->integer)					
						UI_SlideMenuItem (m, 1);
					else			
						UI_SlideMenuItem (m, -1);

					sound = menu_move_sound;
					ui_mousecursor.buttonused[MOUSEBUTTON2] = true;
				}
			}
		}
	}
	else if (!ui_mousecursor.buttonused[MOUSEBUTTON2] && (ui_mousecursor.buttonclicks[MOUSEBUTTON2] == 2)
		&& ui_mousecursor.buttondown[MOUSEBUTTON2])
	{
		// We need to manually save changes for playerconfig menu here
		if (m_drawfunc == Menu_PlayerConfig_Draw)
			Menu_PConfigSaveChanges ();

		UI_PopMenu ();

		sound = menu_out_sound;
		ui_mousecursor.buttonused[MOUSEBUTTON2] = true;
		ui_mousecursor.buttonclicks[MOUSEBUTTON2] = 0;
		ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
		ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 0;
	}

	// clicking on the player model menu...
	if (m_drawfunc == Menu_PlayerConfig_Draw)
		Menu_PlayerConfig_MouseClick ();
	// clicking on the screen menu
	if (m_drawfunc == Menu_Options_Screen_Draw)
		Menu_Options_Screen_Crosshair_MouseClick ();

	if ( sound )
		S_StartLocalSound ( sound );
}
