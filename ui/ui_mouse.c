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

#include "../client/client.h"
#include "ui_local.h"

cursor_t ui_mousecursor;

#ifndef NOTTHIRTYFLIGHTS
qboolean hasitem;
char *menu_click		= "misc/click.wav";
static char *creditsBuffer; // doesn't exist in newer engine releases - Brad
#endif

/*
=======================================================================

Menu Mouse Cursor - psychospaz

=======================================================================
*/

/*
=================
UI_RefreshCursorButtons
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
UI_Mouseover_CheckItem
=================
*/
qboolean UI_Mouseover_CheckItem (menuFramework_s *menu, int i, menuCommon_s	*lastitem)
{
	int				itemType, min[2], max[2];
	float			x1, y1, x2, y2;
	menuCommon_s	*item;

	item = ((menuCommon_s * )menu->items[i]);

	if ( !UI_ItemHasMouseBounds(item) )
		return false;
	if ( item->isHidden )
		return false;

	x1 = item->topLeft[0];
	y1 = item->topLeft[1];
	x2 = item->botRight[0];
	y2 = item->botRight[1];

	// add length and height of current item
	UI_SetMenuItemDynamicSize (menu->items[i]);
	x2 += item->dynamicWidth;
	y2 += item->dynamicHeight;

	SCR_ScaleCoords (&x1, &y1, NULL, NULL, item->scrAlign);
	SCR_ScaleCoords (&x2, &y2, NULL, NULL, item->scrAlign);
	min[0] = x1;	max[0] = x2;
	min[1] = y1;	max[1] = y2;

	itemType = UI_GetItemMouseoverType(item);
	if (itemType == MENUITEM_NONE)
		return false;

	if ( (ui_mousecursor.x >= min[0]) &&  (ui_mousecursor.x <= max[0])
		&& (ui_mousecursor.y >= min[1]) && (ui_mousecursor.y <= max[1]) )
	{	// new item
		if (lastitem != item)
		{
			int j;
			for (j=0; j<MENU_CURSOR_BUTTON_MAX; j++) {
				ui_mousecursor.buttonclicks[j] = 0;
				ui_mousecursor.buttontime[j] = 0;
			}
		}
		ui_mousecursor.menuitem = item;
		ui_mousecursor.menuitemtype = itemType;
		// don't set menu cursor for mouse-only items
		if (!(item->flags & QMF_MOUSEONLY))
			menu->cursor = i;
		return true;
	}
	return false;
}


/*
=================
UI_Mouseover_Check
=================
*/
void UI_Mouseover_Check (menuFramework_s *menu)
{
	int				i;
	menuCommon_s	*lastitem;

	ui_mousecursor.menu = menu;

	// don't allow change in item focus if waiting to grab a key
	if ( UI_HasValidGrabBindItem(menu) ) 
		return;

	if (ui_mousecursor.mouseaction)
	{
		lastitem = ui_mousecursor.menuitem;
		UI_RefreshCursorLink ();

		for (i=0; i<=menu->nitems; i++)
		{
			if ( UI_Mouseover_CheckItem (menu, i, lastitem) )
				break;
		}
	}
	ui_mousecursor.mouseaction = false;
}

// This whole section was forward ported from the older code; expect possible bugs - Brad
/*
=================
UI_Slider_CursorPositionX
=================
*/
int UI_Slider_CursorPositionX (menuSlider_s *s)
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
int UI_SliderValueForX (menuSlider_s *s, int x)
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
void UI_Slider_CheckSlide (menuSlider_s *s)
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
void UI_DragSlideItem (menuFramework_s *menu, void *menuitem)
{
	menuSlider_s *slider;

	if (!menu || !menuitem)
		return;

	slider = (menuSlider_s *) menuitem;

	slider->curPos = UI_SliderValueForX(slider, ui_mousecursor.x);
	UI_Slider_CheckSlide (slider);
}


/*
=================
UI_ClickSlideItem
=================
*/
void UI_ClickSlideItem (menuFramework_s *menu, void *menuitem)
{
	int				min, max;
	float			x, w;
	menuSlider_s	*slider;
	
	if (!menu || !menuitem)
		return;

	slider = (menuSlider_s *)menuitem;

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
qboolean UI_CheckSlider_Mouseover (menuFramework_s *menu, void *menuitem)
{
	int				min[2], max[2];
	float			x1, y1, x2, y2;
	menuSlider_s	*s;

	if (!menu || !menuitem)
		return false;

	s = (menuSlider_s *)menuitem;

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
UI_MouseCursor_Think
=================
*/
void UI_MouseCursor_Think (void)
{
	char * sound = NULL;
	menuFramework_s *m = (menuFramework_s *)ui_mousecursor.menuitem;
    void *m_drawfunc = m->drawFunc;

	if (m_drawfunc == Menu_Main_f) // have to hack for main menu :p
	{
		UI_CheckMainMenuMouse ();
		return;
	}
	if (m_drawfunc == Menu_Credits_f) // have to hack for credits :p
	{
#ifdef NOTTHIRTYFLIGHTS
		if (ui_mousecursor.buttonclicks[MOUSEBUTTON2])
#else
		if ((ui_mousecursor.buttonclicks[MOUSEBUTTON1])||(ui_mousecursor.buttonclicks[MOUSEBUTTON2]))
#endif
		{
			ui_mousecursor.buttonused[MOUSEBUTTON2] = true;
			ui_mousecursor.buttonclicks[MOUSEBUTTON2] = 0;
			ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
			ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 0;
			S_StartLocalSound (ui_menu_out_sound);
			if (creditsBuffer)
				FS_FreeFile (creditsBuffer);
			UI_PopMenu();
			return;
		}
	}
#ifndef NOTTHIRTYFLIGHTS
	else if (m_drawfunc == Menu_Quit_f) //hack for quit menu
	{
		if (ui_mousecursor.buttonclicks[MOUSEBUTTON2])
		{
			ui_mousecursor.buttonused[MOUSEBUTTON2] = true;
			ui_mousecursor.buttonclicks[MOUSEBUTTON2] = 0;
			ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
			ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 0;

			S_StartLocalSound ("world/cheer.wav");



			UI_PopMenu ();
			
			return;
		}
	}
#endif

/*	// clicking on the player model menu...
	if (m_drawfunc == Menu_PlayerConfig_f)
		Menu_PlayerConfig_MouseClick ();
	// clicking on the screen menu
	if (m_drawfunc == Menu_Options_Screen_f)
		Menu_Options_Screen_Crosshair_MouseClick ();
*/
	if (!m)
		return;

	// Exit with double click 2nd mouse button

	if (ui_mousecursor.menuitem)
	{
#ifndef NOTTHIRTYFLIGHTS
		if (!hasitem)
		{
			sound = ui_menu_move_sound;
			hasitem=true;
		}
#endif
		// MOUSE1
		if (ui_mousecursor.buttondown[MOUSEBUTTON1])
		{
			if (ui_mousecursor.menuitemtype == MENUITEM_SLIDER && !ui_mousecursor.buttonused[MOUSEBUTTON1])
			{
				if ( UI_CheckSlider_Mouseover(m, ui_mousecursor.menuitem) ) {
					UI_DragSlideItem (m, ui_mousecursor.menuitem);
					sound = ui_menu_drag_sound;
				}
				else {
					UI_SlideMenuItem (m, 1);
#ifndef NOTTHIRTYFLIGHTS
					sound = ui_menu_move_sound;
#else
					sound = menu_click;
#endif
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

					sound = ui_menu_move_sound;
					ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
				}
				else
				{
					ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
					UI_MouseSelectItem (ui_mousecursor.menuitem);
					sound = ui_menu_move_sound;
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
				sound = ui_menu_move_sound;
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

					sound = ui_menu_move_sound;
					ui_mousecursor.buttonused[MOUSEBUTTON2] = true;
				}
			}
		}
	}
	else if (!ui_mousecursor.buttonused[MOUSEBUTTON2] && (ui_mousecursor.buttonclicks[MOUSEBUTTON2] == 2)
		&& ui_mousecursor.buttondown[MOUSEBUTTON2])
	{
		// We need to manually save changes for playerconfig menu here
		if (m_drawfunc == Menu_PlayerConfig_f)
			Menu_PConfigSaveChanges ();

		UI_PopMenu ();

		sound = ui_menu_out_sound;
		ui_mousecursor.buttonused[MOUSEBUTTON2] = true;
		ui_mousecursor.buttonclicks[MOUSEBUTTON2] = 0;
		ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
		ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 0;
	}
#ifndef NOTTHIRTYFLIGHTS
	else
	{
		if (hasitem==true)
			hasitem=false;
	}
#endif

	// clicking on the player model menu...
// Is this needed? - Brad
#ifdef NOTTHIRTYFLIGHTS
	if (m_drawfunc == Menu_PlayerConfig_f)
		Menu_PlayerConfig_MouseClick ();
#endif
	// clicking on the screen menu
	if (m_drawfunc == Menu_Options_Screen_f)
		Menu_Options_Screen_Crosshair_MouseClick ();

	if ( sound )
		S_StartLocalSound ( sound );
}
