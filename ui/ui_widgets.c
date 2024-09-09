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

// ui_widgets.c -- supporting code for menu widgets

#include "../client/client.h"
#include "ui_local.h"

static int	scrollBarColor[4] = {0, 0, 0, 192};

vec4_t		stCoord_arrow_left = {0.0, 0.0, 0.25, 0.25};
vec4_t		stCoord_arrow_right = {0.25, 0.0, 0.5, 0.25};
vec4_t		stCoord_arrow_up = {0.5, 0.0, 0.75, 0.25};
vec4_t		stCoord_arrow_down = {0.75, 0.0, 1, 0.25};
vec4_t		stCoord_scrollKnob_h = {0.0, 0.75, 0.25, 1.0};
vec4_t		stCoord_scrollKnob_v = {0.25, 0.75, 0.5, 1.0};

vec4_t		stCoord_scrollKnob_h_left = {0.0, 0.0, 0.125, 0.25};
vec4_t		stCoord_scrollKnob_h_seg = {0.125, 0.0, 0.25, 0.25};
vec4_t		stCoord_scrollKnob_h_center = {0.25, 0.0, 0.375, 0.25};
vec4_t		stCoord_scrollKnob_h_center_dbl = {0.625, 0.0, 0.875, 0.25};
vec4_t		stCoord_scrollKnob_h_right = {0.375, 0.0, 0.5, 0.25};
vec4_t		stCoord_scrollKnob_v_top = {0.0, 0.5, 0.25, 0.625};
vec4_t		stCoord_scrollKnob_v_seg = {0.0, 0.625, 0.25, 0.75};
vec4_t		stCoord_scrollKnob_v_center = {0.0, 0.75, 0.25, 0.875};
vec4_t		stCoord_scrollKnob_v_center_dbl = {0.5, 0.625, 0.75, 0.875};
vec4_t		stCoord_scrollKnob_v_bottom = {0.0, 0.875, 0.25, 1.0};

vec4_t		stCoord_checkbox_off = {0.0, 0.0, 0.5, 1.0};
vec4_t		stCoord_checkbox_on = {0.5, 0.0, 1.0, 1.0};
vec4_t		stCoord_field_left = {0.0, 0.0, 0.25, 1.0};
vec4_t		stCoord_field_center = {0.25, 0.0, 0.5, 1.0};
vec4_t		stCoord_field_right = {0.5, 0.0, 0.75, 1.0};
vec4_t		stCoord_slider_left = {0.0, 0.0, 0.125, 1.0};
vec4_t		stCoord_slider_center = {0.125, 0.0, 0.375, 1.0};
vec4_t		stCoord_slider_right = {0.375, 0.0, 0.5, 1.0};
vec4_t		stCoord_slider_knob = {0.5, 0.0, 0.625, 1.0};

#ifndef NOTTHIRTYFLIGHTS
// Forward ported from old engine - Brad

void MenuAction_DoEnter (menuAction_s *a)
{
	if (!a) return;

	if (a->generic.callback)
		a->generic.callback(a);
}

void MenuKeyBind_DoEnter (menuKeyBind_s *k)
{
	menuFramework_s	*menu = k->generic.parent;

	if (!menu)	return;

	UI_FindKeysForCommand (k->commandName, k->keys);

//	if (k->keys[1] != -1)
//		UI_UnbindCommand (k->commandName);

	UI_SetGrabBindItem (menu, (menuCommon_s *)k);
		
	if (k->generic.callback)
		k->generic.callback (k);
}

qboolean MenuField_DoEnter (menuField_s *f)
{
	if (!f) return false;

	if (f->generic.callback)
	{
		f->generic.callback(f);
		return true;
	}
	return false;
}

/*
=================
UI_MouseSelectItem
=================
*/
qboolean UI_MouseSelectItem (menuCommon_s *item)
{
	if (!item)	return false;

	if (item)
	{
		switch (item->type)
		{
		case MTYPE_FIELD:
			return MenuField_DoEnter ( (menuField_s *)item ) ;
		case MTYPE_ACTION:
			MenuAction_DoEnter ( (menuAction_s *)item );
			return true;
		case MTYPE_KEYBIND:
			MenuKeyBind_DoEnter ( (menuKeyBind_s *)item );
			return true;
		case MTYPE_LISTBOX:
		case MTYPE_SPINCONTROL:
			return false;
		default:
			break;
		}
	}
	return false;
}
#endif

//=========================================================

void UI_MenuScrollBar_SetPos (widgetScroll_s *scroll, int visibleItems, int numItems, int curValue)
{
	if (!scroll)	return;
	if (!scroll->scrollEnabled || curValue < 0) {
		scroll->scrollPos = 0;
		return;
	}

	scroll->scrollPos = max( min( (curValue-(visibleItems/2)), (numItems-visibleItems) ), 0);
}

qboolean UI_MenuScrollBar_Increment (widgetScroll_s *scroll, int dir)
{
	int oldPos;

	if (!scroll)
		return false;

	oldPos = scroll->scrollPos;
	scroll->scrollPos = min(max(scroll->scrollPos+dir, scroll->scrollMin), scroll->scrollMax);
	return (scroll->scrollPos != oldPos);
}

void UI_MenuScrollBar_ClickPos (widgetScroll_s *scroll, menuCommon_s *item, float barStart, float barEnd)
{
	float	clickPos, barLength;

	if (!scroll)	return;

	if (scroll->scrollType == SCROLL_X) {
		SCR_ScaleCoords (&barStart, NULL, NULL, NULL, item->scrAlign);
		SCR_ScaleCoords (&barEnd, NULL, NULL, NULL, item->scrAlign);
	}
	else { // SCROLL_Y
		SCR_ScaleCoords (NULL, &barStart, NULL, NULL, item->scrAlign);
		SCR_ScaleCoords (NULL, &barEnd, NULL, NULL, item->scrAlign);
	}
	barLength = barEnd - barStart;
	clickPos = ((scroll->scrollType == SCROLL_X) ? ui_mousecursor.x : ui_mousecursor.y) - barStart;
	scroll->scrollPos = (clickPos / barLength) * (scroll->scrollMax - scroll->scrollMin + 1);
	scroll->scrollPos = min(max(scroll->scrollPos, scroll->scrollMin), scroll->scrollMax);
}

qboolean UI_MenuScrollBar_Click (widgetScroll_s *scroll, menuCommon_s *item)
{
	int		button_size = LIST_SCROLLBAR_CONTROL_SIZE;
	int		x, y, w, h;

	if (!scroll)	return false;

	x = scroll->scrollTopLeft[0];
	y = scroll->scrollTopLeft[1];
	w = scroll->scrollBotRight[0] - scroll->scrollTopLeft[0];
	h = scroll->scrollBotRight[1] - scroll->scrollTopLeft[1];

	if ( UI_MouseOverSubItem(x, y, w, h, item->scrAlign) )
	{
		if (!scroll->scrollEnabled) // check if enabled
			return true;

		if (scroll->scrollType == SCROLL_X)
		{
			// left arrow
			if ( UI_MouseOverSubItem(scroll->scrollTopLeft[0], scroll->scrollTopLeft[1], button_size, button_size, item->scrAlign) )
				UI_MenuScrollBar_Increment (scroll, -1);
			// right arrow
			else if ( UI_MouseOverSubItem(scroll->scrollBotRight[0]-button_size, scroll->scrollTopLeft[1], button_size, button_size, item->scrAlign) )
				UI_MenuScrollBar_Increment (scroll, 1);
			else // on knob area
				UI_MenuScrollBar_ClickPos (scroll, item, scroll->scrollTopLeft[0]+button_size, scroll->scrollBotRight[0]-button_size);
		}
		else // SCROLL_Y
		{
			// up arrow
			if ( UI_MouseOverSubItem(scroll->scrollTopLeft[0], scroll->scrollTopLeft[1], button_size, button_size, item->scrAlign) )
				UI_MenuScrollBar_Increment (scroll, -1);
			// down arrow
			else if ( UI_MouseOverSubItem(scroll->scrollTopLeft[0], scroll->scrollBotRight[1]-button_size, button_size, button_size, item->scrAlign) )
				UI_MenuScrollBar_Increment (scroll, 1);
			else // on knob area
				UI_MenuScrollBar_ClickPos (scroll, item, scroll->scrollTopLeft[1]+button_size, scroll->scrollBotRight[1]-button_size);
		}
		return true;
	}
	return false;
}

qboolean UI_ItemHasScrollBar (menuCommon_s *item)
{
	if (!item)	return false;

	switch (item->type)
	{
		case MTYPE_KEYBINDLIST:
			return ( ((menuKeyBindList_s *)item)->scrollState.scrollEnabled );
		case MTYPE_LISTBOX:
			return ( ((menuListBox_s *)item)->scrollState.scrollEnabled );
		case MTYPE_LISTVIEW:
			return ( ((menuListView_s *)item)->scrollState.scrollEnabled );
		case MTYPE_COMBOBOX:
			return ( ((menuCommon_s *)item)->isExtended && ((menuComboBox_s *)item)->scrollState.scrollEnabled );
		default:
			return false;
	}
}

/*qboolean UI_MouseOverScrollableItem (menuFramework_s *m)
{
	menuCommon_s *item;

	if (!ui_mousecursor.menuitem)
		return false;

	item = (menuCommon_s *)ui_mousecursor.menuitem;

	return UI_ItemHasScrollBar (item);
} */

qboolean UI_MouseOverScrollKnob (menuCommon_s *item)
{
	widgetScroll_s	*scroll;
	int				button_size = LIST_SCROLLBAR_CONTROL_SIZE;
	int				x, y, w, h;

	if (!item)	return false;

	switch (item->type)
	{
		case MTYPE_KEYBINDLIST:
			scroll = &((menuKeyBindList_s *)item)->scrollState;
			break;
		case MTYPE_LISTBOX:
			scroll = &((menuListBox_s *)item)->scrollState;
			break;
		case MTYPE_LISTVIEW:
			scroll = &((menuListView_s *)item)->scrollState;
			break;
		case MTYPE_COMBOBOX:
			scroll = &((menuComboBox_s *)item)->scrollState;
			break;
		default:
			return false;
	}

	if (!scroll)	return false;
	if (!scroll->scrollEnabled)	return false;

	if (scroll->scrollType == SCROLL_X)
	{
		x = scroll->scrollTopLeft[0] + button_size;
		y = scroll->scrollTopLeft[1];
		w = scroll->scrollBotRight[0] - scroll->scrollTopLeft[0] - button_size*2;
		h = scroll->scrollBotRight[1] - scroll->scrollTopLeft[1];

	}
	else // SCROLL_Y
	{
		x = scroll->scrollTopLeft[0];
		y = scroll->scrollTopLeft[1] + button_size;
		w = scroll->scrollBotRight[0] - scroll->scrollTopLeft[0];
		h = scroll->scrollBotRight[1] - scroll->scrollTopLeft[1] - button_size*2;
	}
	if ( UI_MouseOverSubItem(x, y, w, h, item->scrAlign) )
		return true;
	else
		return false;
}

void UI_ClickItemScrollBar (menuCommon_s *item)
{
	widgetScroll_s	*scroll;

	if (!item)	return;

	switch (item->type)
	{
	case MTYPE_KEYBINDLIST:
		scroll = &((menuKeyBindList_s *)item)->scrollState;
		break;
	case MTYPE_LISTBOX:
		scroll = &((menuListBox_s *)item)->scrollState;
		break;
	case MTYPE_LISTVIEW:
		scroll = &((menuListView_s *)item)->scrollState;
		break;
	case MTYPE_COMBOBOX:
		scroll = &((menuComboBox_s *)item)->scrollState;
		break;
	default:
		return;
	}

	if (!scroll)	return;
	if (!scroll->scrollEnabled)	return;

	UI_MenuScrollBar_Click (scroll, item);
}

void UI_MenuScrollBar_Draw (menuCommon_s *item, widgetScroll_s *scroll, int box_x, int box_y, int boxWidth, int boxHeight)
{
	int			button_size = LIST_SCROLLBAR_CONTROL_SIZE;
	int			sk_segment_size = LIST_SCROLLBAR_CONTROL_SIZE / 2;
	int			i, barWidth, barHeight, sliderPos,red, green, blue, hoverAlpha;
	int			knob_x, knob_y, segmentPos;
	int			scrollTotal, knobWidth, knobHeight,  sliderPos2, numKnobSegments, centerKnobSegment;
	float		scroll_proportion, t_ofs[2];
	color_t		arrowColor;
	vec4_t		arrowTemp[2], sbknobTemp[5];
	qboolean	mouseClick, mouseOverArrow1, mouseOverArrow2, arrow1_pulse, arrow2_pulse, knobPulse, useDblKnobCenter;

	if (!scroll)	return;

	hoverAlpha = UI_MouseOverAlpha(ui_mousecursor.menuitem);
	mouseClick = ( ui_mousecursor.buttonused[MOUSEBUTTON1] && ui_mousecursor.buttonclicks[MOUSEBUTTON1] );
	UI_TextColor (alt_text_color->integer, true, &red, &green, &blue);

	if (scroll->scrollEnabled) {
		Vector4Set (arrowColor, red, green, blue, 255);
		t_ofs[0] = t_ofs[1] = 0;
	}
	else {
		Vector4Set (arrowColor, 150, 150, 150, 255);
		t_ofs[0] = t_ofs[1] = 0.25;
	}

	if (scroll->scrollType == SCROLL_X)
	{
		mouseOverArrow1 = UI_MouseOverSubItem(scroll->scrollTopLeft[0], scroll->scrollTopLeft[1], button_size, button_size, item->scrAlign);
		mouseOverArrow2 = UI_MouseOverSubItem(scroll->scrollBotRight[0]-button_size, scroll->scrollTopLeft[1], button_size, button_size, item->scrAlign);
		// left arrow glow
		arrow1_pulse = scroll->scrollEnabled && mouseOverArrow1 && !mouseClick;
		if (scroll->scrollEnabled && mouseOverArrow1 && mouseClick)
			t_ofs[0] = 0.5;
		// right arrow glow
		arrow2_pulse = scroll->scrollEnabled && mouseOverArrow2 && !mouseClick;
		if (scroll->scrollEnabled && mouseOverArrow2 && mouseClick)
			t_ofs[1] = 0.5;
		// scroll knob pulse
		knobPulse = UI_MouseOverScrollKnob(item);

		Vector4Copy (stCoord_arrow_left, arrowTemp[0]);
		Vector4Copy (stCoord_arrow_right, arrowTemp[1]);
		Vector4Copy (stCoord_scrollKnob_h_left, sbknobTemp[0]);
		Vector4Copy (stCoord_scrollKnob_h_seg, sbknobTemp[1]);
		Vector4Copy (stCoord_scrollKnob_h_center, sbknobTemp[2]);
		Vector4Copy (stCoord_scrollKnob_h_center_dbl, sbknobTemp[3]);
		Vector4Copy (stCoord_scrollKnob_h_right, sbknobTemp[4]);
		for (i=0; i<2; i++) {
			arrowTemp[i][1] += t_ofs[i];
			arrowTemp[i][3] += t_ofs[i];
		}
		barWidth = boxWidth - (2 * button_size);

		// scrolling area
		UI_DrawFill (box_x, box_y+boxHeight-LIST_SCROLLBAR_SIZE,
			boxWidth, LIST_SCROLLBAR_SIZE, item->scrAlign, false, scrollBarColor[0], scrollBarColor[1], scrollBarColor[2], scrollBarColor[3]);

		// left arrow
		arrowColor[3] = arrow1_pulse ? hoverAlpha : 255;
		UI_DrawPicST (box_x, box_y+boxHeight-button_size, button_size, button_size,
					arrowTemp[0], item->scrAlign, false, arrowColor, UI_ARROWS_PIC);

		// right arrow
		arrowColor[3] = arrow2_pulse ? hoverAlpha : 255;
		UI_DrawPicST (box_x+boxWidth-button_size, box_y+boxHeight-button_size, button_size, button_size,
					arrowTemp[1], item->scrAlign, false, arrowColor, UI_ARROWS_PIC);
		arrowColor[3] = knobPulse ? hoverAlpha : 255;

		// scroll knob
		if (scroll->scrollEnabled)
		{
			sliderPos = (barWidth-button_size) * ((float)scroll->scrollPos / (float)(scroll->scrollMax - scroll->scrollMin));
			scrollTotal = scroll->scrollNumVisible + (scroll->scrollMax - scroll->scrollMin);
			scroll_proportion = (float)scroll->scrollNumVisible / (float)scrollTotal;
			knobWidth = (int)(scroll_proportion * (float)barWidth);
			knobWidth -= knobWidth % sk_segment_size;

			if (knobWidth <= button_size) {
				UI_DrawPicST (box_x+button_size+sliderPos, box_y+boxHeight-button_size, button_size, button_size,
							stCoord_scrollKnob_h, item->scrAlign, false, arrowColor, UI_ARROWS_PIC);
			}
			else
			{
				sliderPos2 = (barWidth-knobWidth) * ((float)scroll->scrollPos / (float)(scroll->scrollMax - scroll->scrollMin));
				numKnobSegments = knobWidth / sk_segment_size;
				centerKnobSegment = (numKnobSegments / 2);
				useDblKnobCenter = (numKnobSegments % 2 == 0);
				if (useDblKnobCenter)
					centerKnobSegment -= 1;
				knob_x = box_x + button_size + sliderPos2;
				knob_y = box_y + boxHeight - button_size;
			//	Com_Printf ("scroll_prop: %f barWidth: %i knobWidth: %i numKnobSegments: %i centerKnobSegment: %i sliderPos2: %i\n",
			//				scroll_proportion, barWidth, knobWidth, numKnobSegments, centerKnobSegment, sliderPos2);
			//	UI_DrawFill (knob_x, knob_y, knobWidth, button_size, ALIGN_CENTER, false, arrowColor[0], arrowColor[1], arrowColor[2], 127);
				// left
				UI_DrawPicST (knob_x, knob_y, sk_segment_size, button_size,
							stCoord_scrollKnob_h_left, item->scrAlign, false, arrowColor, UI_SCROLLKNOB_PIC);
				// center segments
				for (i = 1, segmentPos = sk_segment_size; i < numKnobSegments - 1; i++, segmentPos += sk_segment_size)
				{
					if (i == centerKnobSegment)
					{
						if (useDblKnobCenter) {
							UI_DrawPicST (knob_x+segmentPos, knob_y, button_size, button_size,
										stCoord_scrollKnob_h_center_dbl, item->scrAlign, false, arrowColor, UI_SCROLLKNOB_PIC);
							i++;
							segmentPos += sk_segment_size;
						}
						else {
							UI_DrawPicST (knob_x+segmentPos, knob_y, sk_segment_size, button_size,
										stCoord_scrollKnob_h_center, item->scrAlign, false, arrowColor, UI_SCROLLKNOB_PIC);
						}
					}
					else {
						UI_DrawPicST (knob_x+segmentPos, knob_y, sk_segment_size, button_size,
									stCoord_scrollKnob_h_seg, item->scrAlign, false, arrowColor, UI_SCROLLKNOB_PIC);
					}
				}
				// right
				UI_DrawPicST (knob_x+segmentPos, knob_y, sk_segment_size, button_size,
							stCoord_scrollKnob_h_right, item->scrAlign, false, arrowColor, UI_SCROLLKNOB_PIC);
			}
		}
	}
	else // SCROLL_Y
	{
		mouseOverArrow1 = UI_MouseOverSubItem(scroll->scrollTopLeft[0], scroll->scrollTopLeft[1], button_size, button_size, item->scrAlign);
		mouseOverArrow2 = UI_MouseOverSubItem(scroll->scrollTopLeft[0], scroll->scrollBotRight[1]-button_size, button_size, button_size, item->scrAlign);
		// up arrow glow
		arrow1_pulse = scroll->scrollEnabled && mouseOverArrow1 && !mouseClick;
		if (scroll->scrollEnabled && mouseOverArrow1 && mouseClick)
			t_ofs[0] = 0.5;
		// down arrow glow
		arrow2_pulse = scroll->scrollEnabled && mouseOverArrow2 && !mouseClick;
		if (scroll->scrollEnabled && mouseOverArrow2 && mouseClick)
			t_ofs[1] = 0.5;
		// scroll knob pulse
		knobPulse = UI_MouseOverScrollKnob(item);

		Vector4Copy (stCoord_arrow_up, arrowTemp[0]);
		Vector4Copy (stCoord_arrow_down, arrowTemp[1]);
		Vector4Copy (stCoord_scrollKnob_v_top, sbknobTemp[0]);
		Vector4Copy (stCoord_scrollKnob_v_seg, sbknobTemp[1]);
		Vector4Copy (stCoord_scrollKnob_v_center, sbknobTemp[2]);
		Vector4Copy (stCoord_scrollKnob_v_center_dbl, sbknobTemp[3]);
		Vector4Copy (stCoord_scrollKnob_v_bottom, sbknobTemp[4]);
		for (i=0; i<2; i++) {
			arrowTemp[i][1] += t_ofs[i];
			arrowTemp[i][3] += t_ofs[i];
		}
		barHeight = boxHeight - (2 * button_size);

		// scrolling area
		UI_DrawFill (box_x+boxWidth-LIST_SCROLLBAR_SIZE, box_y,
					LIST_SCROLLBAR_SIZE, boxHeight, item->scrAlign, false, scrollBarColor[0], scrollBarColor[1], scrollBarColor[2], scrollBarColor[3]);

		// up arrow
		arrowColor[3] = arrow1_pulse ? hoverAlpha : 255;
		UI_DrawPicST (box_x+boxWidth-button_size, box_y, button_size, button_size,
					arrowTemp[0], item->scrAlign, false, arrowColor, UI_ARROWS_PIC);

		// down arrow
		arrowColor[3] = arrow2_pulse ? hoverAlpha : 255;
		UI_DrawPicST (box_x+boxWidth-button_size, box_y+boxHeight-button_size, button_size, button_size,
					arrowTemp[1], item->scrAlign, false, arrowColor, UI_ARROWS_PIC);
		arrowColor[3] = knobPulse ? hoverAlpha : 255;

		// scroll knob
		if (scroll->scrollEnabled)
		{
			sliderPos = (barHeight-button_size) * ((float)scroll->scrollPos / (float)(scroll->scrollMax - scroll->scrollMin));
			scrollTotal = scroll->scrollNumVisible + (scroll->scrollMax - scroll->scrollMin);
			scroll_proportion = (float)scroll->scrollNumVisible / (float)scrollTotal;
			knobHeight = (int)(scroll_proportion * (float)barHeight);
			knobHeight -= knobHeight % sk_segment_size;

			if (knobHeight <= button_size) {
				UI_DrawPicST (box_x+boxWidth-button_size, box_y+button_size+sliderPos, button_size, button_size,
							stCoord_scrollKnob_v, item->scrAlign, false, arrowColor, UI_ARROWS_PIC);
			}
			else
			{
				sliderPos2 = (barHeight - knobHeight) * ((float)scroll->scrollPos / (float)(scroll->scrollMax - scroll->scrollMin));
				numKnobSegments = knobHeight / sk_segment_size;
				centerKnobSegment = (numKnobSegments / 2);
				useDblKnobCenter = (numKnobSegments % 2 == 0);
				if (useDblKnobCenter)
					centerKnobSegment -= 1;
				knob_x = box_x + boxWidth - button_size;
				knob_y = box_y + button_size + sliderPos2;
			//	Com_Printf ("scroll_prop: %f barHeight: %i knobHeight: %i numKnobSegments: %i centerKnobSegment: %i sliderPos2: %i\n",
			//				scroll_proportion, barHeight, knobHeight, numKnobSegments, centerKnobSegment, sliderPos2);
			//	UI_DrawFill (knob_x, knob_y, button_size, knobHeight, ALIGN_CENTER, false, arrowColor[0], arrowColor[1], arrowColor[2], 127);
				// top
				UI_DrawPicST (knob_x, knob_y, button_size, sk_segment_size,
							stCoord_scrollKnob_v_top, item->scrAlign, false, arrowColor, UI_SCROLLKNOB_PIC);
				// center segments
				for (i = 1, segmentPos = sk_segment_size; i < numKnobSegments - 1; i++, segmentPos += sk_segment_size)
				{
					if (i == centerKnobSegment)
					{
						if (useDblKnobCenter) {
							UI_DrawPicST (knob_x, knob_y+segmentPos, button_size, button_size,
										stCoord_scrollKnob_v_center_dbl, item->scrAlign, false, arrowColor, UI_SCROLLKNOB_PIC);
							i++;
							segmentPos += sk_segment_size;
						}
						else {
							UI_DrawPicST (knob_x, knob_y+segmentPos, button_size, sk_segment_size,
										stCoord_scrollKnob_v_center, item->scrAlign, false, arrowColor, UI_SCROLLKNOB_PIC);
						}
					}
					else {
						UI_DrawPicST (knob_x, knob_y+segmentPos, button_size, sk_segment_size,
									stCoord_scrollKnob_v_seg, item->scrAlign, false, arrowColor, UI_SCROLLKNOB_PIC);
					}
				}
				// bottom
				UI_DrawPicST (knob_x, knob_y+segmentPos, button_size, sk_segment_size,
							stCoord_scrollKnob_v_bottom, item->scrAlign, false, arrowColor, UI_SCROLLKNOB_PIC);
			}
		}
	}

	if (ui_debug_itembounds->integer)
	{
		int		x, y, w, h;
		x = scroll->scrollTopLeft[0];
		y = scroll->scrollTopLeft[1];
		w = scroll->scrollBotRight[0] - scroll->scrollTopLeft[0];
		h = scroll->scrollBotRight[1] - scroll->scrollTopLeft[1];
		UI_DrawFill (x, y, w, h, item->scrAlign, false, 255,0,0,128);
	}

}

//======================================================

void UI_MenuCommon_DrawItemName (menuCommon_s *c, int nameX, int nameY, int headerX, int headerY, int hoverAlpha)
{
	qboolean	usesName = (c->name && strlen(c->name) > 0);
	qboolean	usesHeader = (c->header && strlen(c->header) > 0);

	if (usesName)
		UI_DrawMenuString (c->topLeft[0] + nameX, c->topLeft[1] +nameY,
							MENU_FONT_SIZE, c->scrAlign, c->name, hoverAlpha, true, true);

	if (usesHeader)
		UI_DrawMenuString (c->topLeft[0] + headerX, c->topLeft[1] + headerY,
							MENU_FONT_SIZE, c->scrAlign, c->header, (!usesName) ? hoverAlpha : 255, false, true);
}

//======================================================

void UI_MenuAction_DoEnter (menuAction_s *a)
{
	if (a->generic.callback)
		a->generic.callback (a);
}

char *UI_MenuAction_Click (menuAction_s *a, qboolean mouse2)
{
	// return if it's just a mouse2 click
	if (!a->usesMouse2 && mouse2)
		return ui_menu_null_sound;

	if ( mouse2 && a->generic.mouse2Callback )
		a->generic.mouse2Callback (a);
	else if (a->generic.callback)
		a->generic.callback (a);

	return ui_menu_move_sound;
}

void UI_MenuAction_Draw (menuAction_s *a)
{
	menuFramework_s	*menu = a->generic.parent;
	int				alpha = UI_MouseOverAlpha(&a->generic);

	UI_DrawMenuString (menu->x + a->generic.x + LCOLUMN_OFFSET, menu->y + a->generic.y,
					a->generic.textSize, a->generic.scrAlign, a->generic.name, alpha,
					!(a->generic.flags & QMF_LEFT_JUSTIFY), (a->generic.flags & QMF_ALTCOLOR));

//	if (a->generic.ownerdraw)
//		a->generic.ownerdraw (a);
}

void UI_MenuAction_Setup (menuAction_s *a)
{
	menuFramework_s	*menu = a->generic.parent;
	int				len;

	len = (a->generic.name) ? (int)strlen(a->generic.name) : 0;
	// set min and max coords
	if (a->generic.flags & QMF_LEFT_JUSTIFY) {
		a->generic.topLeft[0] = menu->x + a->generic.x + LCOLUMN_OFFSET;
		a->generic.botRight[0] = a->generic.topLeft[0] + len*a->generic.textSize;
	}
	else {
		a->generic.topLeft[0] = menu->x + a->generic.x - (len+2)*a->generic.textSize;
		a->generic.botRight[0] = a->generic.topLeft[0] + (len+2)*a->generic.textSize;
	}
	a->generic.topLeft[1] = menu->y + a->generic.y;
	a->generic.botRight[1] = a->generic.topLeft[1] + a->generic.textSize;
	a->generic.dynamicWidth = 0;
	a->generic.dynamicHeight = 0;
	a->generic.isExtended = false;
	a->generic.valueChanged = false;
}

//=========================================================
// Forward ported from old engine - Brad
void UI_MenuSpinControl_SetValue (menuPicker_s *s, const char *varName, float cvarMin, float cvarMax, qboolean clamp)
{
	if (!s || !varName || varName[0] == '\0')
		return;

	if (clamp) {
		UI_ClampCvar (varName, cvarMin, cvarMax);
	}
	if (s->itemValues) {
		s->curValue = UI_GetIndexForStringValue(s->itemValues, Cvar_VariableString((char *)varName));
	}
	else
	{
		if (s->invertValue) {
			s->curValue = (Cvar_VariableValue((char *)varName) < 0);
		}
		else {
			s->curValue = (int)min(max(Cvar_VariableValue((char *)varName), cvarMin), cvarMax);
		}
	}
}

void UI_MenuSpinControl_SaveValue (menuPicker_s *s, const char *varName)
{
	if (!s || !varName || varName[0] == '\0')
		return;
	if (!s->numItems) {
		Com_Printf (S_COLOR_YELLOW"UI_MenuSpinControl_SaveValue: not initialized!\n");
		return;
	}
	if ( (s->curValue < 0) || (s->curValue >= s->numItems) ) {
		Com_Printf (S_COLOR_YELLOW"UI_MenuSpinControl_SaveValue: curvalue out of bounds!\n");
		return;
	}

	if (s->itemValues) {
		// Don't save to cvar if this itemvalue is the wildcard
		if ( Q_stricmp(va("%s", s->itemValues[s->curValue]), UI_ITEMVALUE_WILDCARD) != 0 )
			Cvar_Set ((char *)varName, va("%s", s->itemValues[s->curValue]));
	}
	else
	{
		if (s->invertValue) {
			Cvar_SetValue ((char *)varName, Cvar_VariableValue((char *)varName) * -1 );
		}
		else {
			Cvar_SetInteger ((char *)varName, s->curValue);
		}
	}
}

const char *UI_MenuSpinControl_GetValue (menuPicker_s *s)
{
	const char *value;

	if (!s)
		return NULL;

	if (!s->numItems) {
		Com_Printf (S_COLOR_YELLOW"UI_MenuSpinControl_GetValue: not initialized!\n");
		return NULL;
	}
	if ( (s->curValue < 0) || (s->curValue >= s->numItems) ) {
		Com_Printf (S_COLOR_YELLOW"UI_MenuSpinControl_GetValue: curvalue out of bounds!\n");
		return NULL;
	}

	if (s->itemValues) {
		value = s->itemValues[s->curValue];
	}
	else {
		value = va("%d", s->curValue);
	}

	return value;
}

void MenuSpinControl_DoEnter (menuPicker_s *s)
{
	if (!s || !s->itemNames || !s->numItems)
		return;

	s->curValue++;
	if (s->itemNames[s->curValue] == 0)
		s->curValue = 0;

	if (s->generic.callback)
		s->generic.callback(s);
}

void MenuSpinControl_DoSlide (menuPicker_s *s, int dir)
{
	if (!s || !s->itemNames || !s->numItems)
		return;

	s->curValue += dir;

	if (s->generic.flags & QMF_SKINLIST) // don't allow looping around for skin lists
	{
		if (s->curValue < 0)
			s->curValue = 0;
		else if (s->itemNames[s->curValue] == 0)
			s->curValue--;
	}
	else {
		if (s->curValue < 0)
			s->curValue = s->numItems-1; // was 0
		else if (s->itemNames[s->curValue] == 0)
			s->curValue = 0; // was --
	}

	if (s->generic.callback)
		s->generic.callback(s);
}
 
void MenuSpinControl_Draw (menuPicker_s *s)
{
	int		alpha;
	char	buffer[100];

	if (!s)	return;

	alpha = UI_MouseOverAlpha(&s->generic);

	if (s->generic.name)
	{
		UI_DrawStringR2LDark (s->generic.x + s->generic.parent->x + LCOLUMN_OFFSET,
								s->generic.y + s->generic.parent->y, s->generic.textSize, ALIGN_CENTER, s->generic.name, FONT_UI, alpha);
	}
	if (!strchr(s->itemNames[s->curValue], '\n'))
	{
#ifdef NOTTHIRTYFLIGHTS
		UI_DrawString (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET,
#else
		UI_DrawString (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET + (6),
#endif
						s->generic.y + s->generic.parent->y, s->generic.textSize, ALIGN_CENTER, s->itemNames[s->curValue], FONT_UI, alpha);
	}
	else
	{
	//	strncpy(buffer, s->itemnames[s->curvalue]);
		Q_strncpyz (buffer, sizeof(buffer), s->itemNames[s->curValue]);
		*strchr(buffer, '\n') = 0;
		UI_DrawString (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET,
						s->generic.y + s->generic.parent->y, s->generic.textSize, ALIGN_CENTER, buffer, FONT_UI, alpha);
	//	strncpy(buffer, strchr( s->itemnames[s->curvalue], '\n' ) + 1 );
		Q_strncpyz (buffer, sizeof(buffer), strchr( s->itemNames[s->curValue], '\n' ) + 1);
		UI_DrawString (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET,
						s->generic.y + s->generic.parent->y + MENU_LINE_SIZE, s->generic.textSize, ALIGN_CENTER, buffer, FONT_UI, alpha);
	}
}

//=========================================================

void UI_MenuKeyBind_DoEnter (menuKeyBind_s *k)
{
	menuFramework_s	*menu = k->generic.parent;

	if (!menu)	return;

	UI_FindKeysForCommand (k->commandName, k->keys);

//	if (k->keys[1] != -1)
//		UI_UnbindCommand (k->commandName);

	UI_SetGrabBindItem (menu, (menuCommon_s *)k);
		
	if (k->generic.callback)
		k->generic.callback (k);
}

char *UI_MenuKeyBind_Click (menuKeyBind_s *k, qboolean mouse2)
{
	// return if it's just a mouse2 click
	if (mouse2)
		return ui_menu_null_sound;

	UI_MenuKeyBind_DoEnter (k);

	return ui_menu_move_sound;
}

void UI_MenuKeyBind_Draw (menuKeyBind_s *k)
{
	menuFramework_s	*menu = k->generic.parent;
	int				x, alpha = UI_MouseOverAlpha(&k->generic);
	const char		*keyName1, *keyName2;

	if (!menu)	return;

	UI_DrawMenuString (menu->x + k->generic.x + LCOLUMN_OFFSET,
						menu->y + k->generic.y, MENU_FONT_SIZE, k->generic.scrAlign, k->generic.name, alpha,
						!(k->generic.flags & QMF_LEFT_JUSTIFY), (k->generic.flags & QMF_ALTCOLOR));

	if (k->commandName)
	{
		UI_FindKeysForCommand (k->commandName, k->keys);

		if (k->keys[0] == -1)
		{
			UI_DrawMenuString (menu->x + k->generic.x + RCOLUMN_OFFSET,
								menu->y + k->generic.y, MENU_FONT_SIZE, k->generic.scrAlign, "???", alpha, false, false);
		}
		else
		{
			keyName1 = Key_KeynumToString (k->keys[0]);
			UI_DrawMenuString (menu->x + k->generic.x + RCOLUMN_OFFSET,
								menu->y + k->generic.y, MENU_FONT_SIZE, k->generic.scrAlign, keyName1, alpha, false, false);
			if (k->keys[1] != -1)
			{
				x = (int)strlen(keyName1) * MENU_FONT_SIZE;
				keyName2 = Key_KeynumToString (k->keys[1]);
				UI_DrawMenuString (menu->x + k->generic.x + MENU_FONT_SIZE*3 + x,
									menu->y + k->generic.y, MENU_FONT_SIZE, k->generic.scrAlign, "or", alpha, false, false);
				UI_DrawMenuString (menu->x + k->generic.x + MENU_FONT_SIZE*6 + x,
									menu->y + k->generic.y, MENU_FONT_SIZE, k->generic.scrAlign, keyName2, alpha, false, false);
			}
		}
	}
	else
		Com_Printf ("UI_MenuKeyBind_Draw: keybind has no commandName!\n");

//	if (k->generic.ownerdraw)
//		k->generic.ownerdraw(k);
}

void UI_MenuKeyBind_SetDynamicSize (menuKeyBind_s *k)
{
	k->generic.dynamicWidth = 0;
	if (k->commandName)
	{
		UI_FindKeysForCommand (k->commandName, k->keys);
		k->generic.dynamicWidth += RCOLUMN_OFFSET;

		if (k->keys[0] == -1)
			k->generic.dynamicWidth += MENU_FONT_SIZE*3; // "???"
		else {
			k->generic.dynamicWidth += MENU_FONT_SIZE*strlen(Key_KeynumToString(k->keys[0])); // key 1
			if (k->keys[1] != -1) // " or " + key2
				k->generic.dynamicWidth += MENU_FONT_SIZE*4 + MENU_FONT_SIZE*strlen(Key_KeynumToString(k->keys[1]));
		}
	}
}

void UI_MenuKeyBind_Setup (menuKeyBind_s *k)
{
	menuFramework_s	*menu = k->generic.parent;
	int				len;

	if (!menu)	return;

	len = (k->generic.name) ? (int)strlen(k->generic.name) : 0;
	// set min and max coords
	if (k->generic.flags & QMF_LEFT_JUSTIFY) {
		k->generic.topLeft[0] = menu->x + k->generic.x + LCOLUMN_OFFSET;
		k->generic.botRight[0] = k->generic.topLeft[0] + len*MENU_FONT_SIZE;
	}
	else {
		k->generic.topLeft[0] = menu->x + k->generic.x - (len+2)*MENU_FONT_SIZE;
		k->generic.botRight[0] = k->generic.topLeft[0] + (len+2)*MENU_FONT_SIZE;
	}
	k->generic.topLeft[1] = menu->y + k->generic.y;
	k->generic.botRight[1] = k->generic.topLeft[1] + MENU_FONT_SIZE;
	k->generic.dynamicWidth = 0;
	k->generic.dynamicHeight = 0;
	k->generic.isExtended = false;
	k->generic.valueChanged = false;

	UI_MenuKeyBind_SetDynamicSize (k);
	k->grabBind = false;
}

const char *UI_MenuKeyBind_Key (menuKeyBind_s *k, int key)
{
	menuFramework_s	*menu = k->generic.parent;

	// pressing mouse1 to pick a new bind wont force bind/unbind itself - spaz
	if (UI_HasValidGrabBindItem(menu) && k->grabBind
		&& !(ui_mousecursor.buttonused[MOUSEBUTTON1] && key == K_MOUSE1))
	{
		// grab key here
		if (key != K_ESCAPE && key != '`')
		{
			char cmd[1024];

			if (k->keys[1] != -1)	// if two keys are already bound to this, clear them
				UI_UnbindCommand (k->commandName);

			Com_sprintf (cmd, sizeof(cmd), "bind \"%s\" \"%s\"\n", Key_KeynumToString(key), k->commandName);
			Cbuf_InsertText (cmd);
		}

		// don't let selecting with mouse buttons screw everything up
		UI_RefreshCursorButtons();
		if (key == K_MOUSE1)
			ui_mousecursor.buttonclicks[MOUSEBUTTON1] = -1;

		if (menu)
			UI_ClearGrabBindItem (menu);

		return ui_menu_out_sound;
	}

	switch (key)
	{
	case K_ESCAPE:
		UI_CheckAndPopMenu (menu);
		return ui_menu_out_sound;
	case K_ENTER:
	case K_KP_ENTER:
		UI_MenuKeyBind_DoEnter (k);
		return ui_menu_in_sound;
	case K_BACKSPACE:
	case K_DEL:
	case K_KP_DEL:
		UI_UnbindCommand (k->commandName); // delete bindings
		return ui_menu_out_sound;
	default:
		return ui_menu_null_sound;
	}
}

//=========================================================

static keyBindSubitem_t ui_binds_null[] = 
{
{ "null",	"No Bind Commands Found",	0, 0 },
{ 0, 0,	0, 0 }
};

void UI_MenuKeyBindList_DoEnter (menuKeyBindList_s *k)
{
	menuFramework_s	*menu = k->generic.parent;

	if (!menu)	return;

	// catch invalid selected index
	if ( (k->curValue < 0) && (k->curValue >= k->numItems) ) {
		return;
	}

	UI_FindKeysForCommand ((char *)k->bindList[k->curValue].commandName, k->bindList[k->curValue].keys);

//	if (k->keys[1] != -1)
//		UI_UnbindCommand (k->commandName);

	UI_SetGrabBindItem (menu, (menuCommon_s *)k);
		
	if (k->generic.dblClkCallback)
		k->generic.dblClkCallback (k);
}

void UI_MenuKeyBindList_DoSlide (menuKeyBindList_s *k, int dir)
{
	int		visibleLines, startItem;

	if (!k->bindList || k->numItems < 1)
		return;

	k->curValue += dir;

	if (k->generic.flags & QMF_NOLOOP) // don't allow looping around
	{
		if (k->curValue < 0)
			k->curValue = 0;
		else if (k->curValue >= k->numItems)
			k->curValue = k->numItems-1;
	}
	else {
		if (k->curValue < 0)
			k->curValue = k->numItems-1;
		else if (k->curValue >= k->numItems)
			k->curValue = 0;
	}

	// scroll if needed
	startItem = k->scrollState.scrollPos;
	visibleLines = k->items_y;
	while (k->curValue < startItem) {
		UI_MenuScrollBar_Increment (&k->scrollState, -1);
		startItem = k->scrollState.scrollPos;
	}
	while ( k->curValue > (startItem+visibleLines-1) ) {
		UI_MenuScrollBar_Increment (&k->scrollState, 1);
		startItem = k->scrollState.scrollPos;
	}

	if (k->generic.callback)
		k->generic.callback (k);
}

char *UI_MenuKeyBindList_Click (menuKeyBindList_s *k, qboolean mouse2)
{
	int			i, oldCurValue, y, itemWidth, itemHeight, visibleLines, startItem, itemIncrement;
	char		*s = ui_menu_null_sound;

	// return if it's just a mouse2 click
	if (mouse2)
		return s;

	// clicked on scroll bar
	if ( UI_MenuScrollBar_Click(&k->scrollState, &k->generic) ) {
		s = ui_menu_move_sound;
	}
	else // clicked on one of the items
	{
		itemWidth = k->lineWidth * MENU_FONT_SIZE;
		itemHeight = k->itemHeight * MENU_LINE_SIZE;
		startItem = k->scrollState.scrollPos;
		visibleLines = k->items_y;
		itemIncrement = itemHeight + k->itemSpacing;
		for (i=startItem; i<k->numItems && i<(startItem+visibleLines); i++)
		{
			y = k->generic.topLeft[1] + (i-startItem)*itemIncrement;
			if ( UI_MouseOverSubItem(k->generic.topLeft[0], y, itemWidth, itemIncrement, k->generic.scrAlign) )
			{
				oldCurValue = k->curValue;
				k->curValue = i;
				s = ui_menu_move_sound;

				if (oldCurValue != k->curValue) {	// new subitem
					ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 1;	// was 0
					if (k->generic.callback)
						k->generic.callback (k);
				}

				if (ui_mousecursor.buttonclicks[MOUSEBUTTON1] == 2) {	// && k->generic.dblClkCallback)
					UI_MenuKeyBindList_DoEnter (k);
				}

				break;
			}
		}
	}
	return s;
}

void UI_MenuKeyBindList_Draw (menuKeyBindList_s *k)
{
//	menuFramework_s	*menu = k->generic.parent;
	int				i, x, xofs, y, itemWidth, itemHeight, boxWidth, boxHeight, visibleLines, startItem, itemIncrement;
	int				hc[3];
	int				hoverAlpha;
	char			buf[512];
	const char		*keyName1, *keyName2;
	qboolean		itemPulse;

	UI_TextColorHighlight (Cvar_VariableInteger("alt_text_color"), &hc[0], &hc[1], &hc[2]);
	hoverAlpha	= UI_MouseOverAlpha(&k->generic);
	itemWidth	= k->lineWidth * MENU_FONT_SIZE;
	itemHeight	= k->itemHeight * MENU_LINE_SIZE;
	boxWidth	= k->generic.botRight[0] - k->generic.topLeft[0];
	boxHeight	= k->generic.botRight[1] - k->generic.topLeft[1];

	// name and header
	UI_MenuCommon_DrawItemName (&k->generic, -(2*RCOLUMN_OFFSET + k->border), 0, 0, -(k->border + MENU_LINE_SIZE), hoverAlpha);

	// border and background
	if (k->border > 0)
	{
		if (k->backColor[3] == 255) // just fill whole area for border if not trans
			UI_DrawFill (k->generic.topLeft[0]-k->border, k->generic.topLeft[1]-k->border,
							boxWidth+(k->border*2), boxHeight+(k->border*2), k->generic.scrAlign, true, k->borderColor[0], k->borderColor[1], k->borderColor[2], k->borderColor[3]);
		else // have to do each side
			UI_DrawBorder ((float)k->generic.topLeft[0], (float)k->generic.topLeft[1], (float)boxWidth, (float)boxHeight,
							(float)k->border, k->generic.scrAlign, true, k->borderColor[0], k->borderColor[1], k->borderColor[2], k->borderColor[3]);
	}
	if ( !(k->altBackColor[3] > 0) )
		UI_DrawFill (k->generic.topLeft[0], k->generic.topLeft[1], boxWidth, boxHeight,
						k->generic.scrAlign, true, k->backColor[0], k->backColor[1], k->backColor[2], k->backColor[3]);

	// scrollbar
	UI_MenuScrollBar_Draw (&k->generic, &k->scrollState, k->generic.topLeft[0], k->generic.topLeft[1], boxWidth, boxHeight); 

	// items
	x = k->generic.topLeft[0];
	startItem = k->scrollState.scrollPos;
	visibleLines = k->items_y;
	itemIncrement = itemHeight + k->itemSpacing;
	for (i=startItem; i<k->numItems && i<(startItem+visibleLines); i++)
	{
		y = k->generic.topLeft[1]+(i-startItem)*itemIncrement;
		itemPulse = UI_MouseOverSubItem(x, y, itemWidth, itemIncrement, k->generic.scrAlign);

		if (i == k->curValue)
			UI_DrawFill (x, y, itemWidth, itemHeight, k->generic.scrAlign, false, hc[0], hc[1], hc[2], hoverAlpha);
		else if (k->altBackColor[3] > 0) {
			if (i % 2 == 1)
				UI_DrawFill (x, y, itemWidth, itemHeight, k->generic.scrAlign, true, k->altBackColor[0], k->altBackColor[1], k->altBackColor[2], k->altBackColor[3]);
			else
				UI_DrawFill (x, y, itemWidth, itemHeight, k->generic.scrAlign, true, k->backColor[0], k->backColor[1], k->backColor[2], k->backColor[3]);
		}
		// bind name
		if (k->bindList[i].commandName)
		{
			UI_FindKeysForCommand ((char *)k->bindList[i].commandName, k->bindList[i].keys);

			xofs = LISTBOX_ITEM_PADDING;
			UI_DrawMenuString (x+xofs, y+LISTBOX_ITEM_PADDING,
								MENU_FONT_SIZE, k->generic.scrAlign, k->bindList[i].displayName, itemPulse ? hoverAlpha : 255, false, false);

			xofs = LISTBOX_ITEM_PADDING + (k->itemNameWidth + 1) * MENU_FONT_SIZE;
			if ( (i == k->curValue) && k->grabBind ) {
				Com_sprintf (buf, sizeof(buf), " = ");
			}
			else
				Com_sprintf (buf, sizeof(buf), "   ");

			if (k->bindList[i].keys[0] == -1) {
				Q_strncatz (buf, sizeof(buf), "???");
			}
			else
			{
				keyName1 = Key_KeynumToString (k->bindList[i].keys[0]);
				Q_strncatz (buf, sizeof(buf), keyName1);

				if (k->bindList[i].keys[1] != -1) {
					keyName2 = Key_KeynumToString (k->bindList[i].keys[1]);
					Q_strncatz (buf, sizeof(buf), " or ");
					Q_strncatz (buf, sizeof(buf), keyName2);
				}
			}
			UI_DrawMenuString (x+xofs, y+LISTBOX_ITEM_PADDING, MENU_FONT_SIZE, k->generic.scrAlign, buf, itemPulse ? hoverAlpha : 255, false, false);
		}
		else
			Com_Printf ("UI_MenuKeyBindList_Draw: keyBindSubitem has no commandName!\n");

	}
}

void UI_MenuKeyBindList_Setup (menuKeyBindList_s *k)
{
	menuFramework_s	*menu = k->generic.parent;
	int				i, boxWidth, boxHeight;

	if (!menu)	return;

	// use bind list from external file if loaded
	if ( k->useCustomBindList && ui_customKeyBindList.bindList && (ui_customKeyBindList.numKeyBinds > 0) ) {
		k->bindListBackup = k->bindList;
		k->bindList = ui_customKeyBindList.bindList;
	}

	// catch null bindlist
	if (!k->bindList) {
		Com_Printf ("UI_MenuKeyBindList_Setup: keyBindList has no bindList!\n");
		k->bindList = ui_binds_null;
		k->bindListIsFromFile = false;
	}

	// count # of items
	k->numItems = 0;
	for (i=0; k->bindList[i].commandName; i++)
		k->numItems++;

	// clamp curValue
	k->curValue = min(max(k->curValue, 0), k->numItems-1);

	k->items_y		= max(k->items_y, 3);		// must be at least 3 lines tall
	k->lineWidth	= min(max(k->lineWidth, 20), 100);	// must be at least 20 chars wide
	k->itemNameWidth =  min(max(k->itemNameWidth, k->lineWidth/4), k->lineWidth*0.75f);	// name must be at least 25% of width, max 75%
//	k->itemHeight	= max(k->itemHeight, 1);	// must be at least 1 line high
	k->itemHeight	= 1;						// item height is always 1 line
	k->itemSpacing	= max(k->itemSpacing, 0);
	k->itemTextSize	= max(k->itemTextSize, 4);
	k->border		= max(k->border, 0);

	boxWidth	= k->lineWidth * MENU_FONT_SIZE + LIST_SCROLLBAR_SIZE;
	boxHeight	= k->items_y * (k->itemHeight*MENU_LINE_SIZE + k->itemSpacing) + k->itemSpacing;

	// set min and max coords
	k->generic.topLeft[0] = menu->x + k->generic.x + RCOLUMN_OFFSET + k->border;
	k->generic.topLeft[1] = menu->y + k->generic.y;
	k->generic.botRight[0] = k->generic.topLeft[0] + boxWidth;
	k->generic.botRight[1] = k->generic.topLeft[1] + boxHeight;
	k->generic.dynamicWidth = 0;
	k->generic.dynamicHeight = 0;
	k->generic.isExtended = false;
	k->generic.valueChanged = false;

	k->scrollState.scrollType = SCROLL_Y; // always scrolls vertically
	k->scrollState.scrollEnabled = (k->numItems > k->items_y);
	k->scrollState.scrollMin = 0;
	k->scrollState.scrollMax = max((k->numItems - k->items_y), 0);
	k->scrollState.scrollNumVisible = k->items_y;
	k->scrollState.scrollTopLeft[0] = k->generic.topLeft[0] + boxWidth - LIST_SCROLLBAR_SIZE;
	k->scrollState.scrollTopLeft[1] = k->generic.topLeft[1];
	k->scrollState.scrollBotRight[0] = k->generic.botRight[0];
	k->scrollState.scrollBotRight[1] = k->generic.botRight[1];
	UI_MenuScrollBar_SetPos (&k->scrollState, k->items_y, k->numItems, k->curValue);
//	k->generic.flags |= QMF_MOUSEONLY;

	k->grabBind = false;
}

const char *UI_MenuKeyBindList_Key (menuKeyBindList_s *k, int key)
{
	menuFramework_s	*menu = k->generic.parent;

	if (!menu)	return ui_menu_null_sound;

	// catch invalid selected index
	if ( (k->curValue < 0) && (k->curValue >= k->numItems) ) {
		return ui_menu_null_sound;
	}

	// pressing mouse1 to pick a new bind wont force bind/unbind itself - spaz
	if ( UI_HasValidGrabBindItem(menu) && k->grabBind
		&& !(ui_mousecursor.buttonused[MOUSEBUTTON1] && key == K_MOUSE1) )
	{
		// grab key here
		if (key != K_ESCAPE && key != '`')
		{
			char cmd[1024];
			
			if (k->bindList[k->curValue].keys[1] != -1)	// if two keys are already bound to this, clear them
				UI_UnbindCommand ((char *)k->bindList[k->curValue].commandName);
			Com_sprintf (cmd, sizeof(cmd), "bind \"%s\" \"%s\"\n", Key_KeynumToString(key), k->bindList[k->curValue].commandName);
			Cbuf_InsertText (cmd);
		}

		// don't let selecting with mouse buttons screw everything up
		UI_RefreshCursorButtons ();
		if (key == K_MOUSE1)
			ui_mousecursor.buttonclicks[MOUSEBUTTON1] = -1;

		if (menu)
			UI_ClearGrabBindItem (menu);

		return ui_menu_out_sound;
	}

	switch (key)
	{
	case K_ESCAPE:
		UI_CheckAndPopMenu (menu);
		return ui_menu_out_sound;
	case K_ENTER:
	case K_KP_ENTER:
		UI_MenuKeyBindList_DoEnter (k);
		return ui_menu_in_sound;
	case K_BACKSPACE:
	case K_DEL:
	case K_KP_DEL:
		UI_UnbindCommand ((char *)k->bindList[k->curValue].commandName); // delete bindings
		return ui_menu_out_sound;
	default:
		return ui_menu_null_sound;
	}
}

//=========================================================

char *UI_MenuField_GetValue (menuField_s *f)
{
	if ( (f->generic.flags & QMF_NUMBERSONLY) && f->generic.cvarClamp 
		&& !(f->generic.cvarMin == 0 && f->generic.cvarMax == 0) )
		return va("%f", ClampCvar(f->generic.cvarMin, f->generic.cvarMax, atof(f->buffer)) );
	else
		return f->buffer;
}

void UI_MenuField_SetValue (menuField_s *f)
{
	if (f->generic.cvar && strlen(f->generic.cvar) > 0)
	{
		if (f->generic.flags & QMF_NUMBERSONLY)
			UI_ClampCvarForControl (&f->generic);

	//	strncpy (f->buffer, Cvar_VariableString(f->generic.cvar), sizeof(f->buffer));
		Q_strncpyz (f->buffer, sizeof(f->buffer), Cvar_VariableString(f->generic.cvar));
		f->cursor = (int)strlen(Cvar_VariableString(f->generic.cvar));
	}
}

void UI_MenuField_SaveValue (menuField_s *f)
{
	if (f->generic.cvar && strlen(f->generic.cvar) > 0)
	{
		// clamp numeric values if needed
		if ( (f->generic.flags & QMF_NUMBERSONLY) && f->generic.cvarClamp 
			&& !(f->generic.cvarMin == 0 && f->generic.cvarMax == 0) )
			Cvar_SetValue ( f->generic.cvar, ClampCvar(f->generic.cvarMin, f->generic.cvarMax, atof(f->buffer)) );
		else
			Cvar_Set (f->generic.cvar, f->buffer);
	}
	f->generic.valueChanged = false;
}

qboolean UI_MenuField_ValueChanged (menuField_s *f)
{
	if (!f->generic.cvar || !strlen(f->generic.cvar))	// must have a valid cvar
		return false;
	if (!f->generic.cvarNoSave)	// only for cvarNoSave items
		return false;

	return ( strcmp(Cvar_VariableString(f->generic.cvar), f->buffer) != 0 );
}

qboolean UI_MenuField_DoEnter (menuField_s *f)
{
	if (!f) return false;

	if (f->generic.callback)
	{
		f->generic.callback (f);
		return true;
	}
	return false;
}

char *UI_MenuField_Click (menuField_s *f, qboolean mouse2)
{
	// return if it's just a mouse2 click
	if (mouse2)
		return ui_menu_null_sound;

	if (f->generic.callback)
	{
		f->generic.callback (f);
		return ui_menu_move_sound;
	}
	return ui_menu_null_sound;
}

void UI_MenuField_Draw (menuField_s *f)
{
	menuFramework_s	*menu = f->generic.parent;
	int				i, hoverAlpha = UI_MouseOverAlpha(&f->generic), xtra;
	char			tempbuffer[128]="";
	int				offset;

	// name
	UI_MenuCommon_DrawItemName (&f->generic, -(2*RCOLUMN_OFFSET), 0, 0, -(FIELD_VOFFSET+MENU_LINE_SIZE), hoverAlpha);

	if (xtra = stringLengthExtra(f->buffer))
	{
		strncpy( tempbuffer, f->buffer + f->visible_offset, f->visible_length );
		offset = (int)strlen(tempbuffer) - xtra;

		if (offset > f->visible_length)
		{
			f->visible_offset = offset - f->visible_length;
			strncpy (tempbuffer, f->buffer + f->visible_offset - xtra, f->visible_length + xtra);
			offset = f->visible_offset;
		}
	}
	else
	{
		strncpy (tempbuffer, f->buffer + f->visible_offset, f->visible_length);
		offset = (int)strlen(tempbuffer);
	}

	// draw left
	if (ui_new_textfield->integer) {
		UI_DrawPicST (menu->x + f->generic.x + RCOLUMN_OFFSET, menu->y + f->generic.y - FIELD_VOFFSET,
					f->generic.textSize, f->generic.textSize*2, stCoord_field_left, f->generic.scrAlign, true, color_identity, UI_FIELD_PIC);
	}
	else {
		UI_DrawChar (menu->x + f->generic.x + RCOLUMN_OFFSET, menu->y + f->generic.y - FIELD_VOFFSET,
					f->generic.textSize, f->generic.scrAlign, 18, 255, 255, 255, 255, false, false);
		UI_DrawChar (menu->x + f->generic.x + RCOLUMN_OFFSET, menu->y + f->generic.y + FIELD_VOFFSET,
					f->generic.textSize, f->generic.scrAlign, 24, 255, 255, 255, 255, false, false);
	}

	// draw center
	for (i = 0; i < f->visible_length; i++)
	{
		if (ui_new_textfield->integer) {
			UI_DrawPicST (menu->x + f->generic.x + (3+i)*f->generic.textSize, menu->y + f->generic.y - FIELD_VOFFSET,
						f->generic.textSize, f->generic.textSize*2, stCoord_field_center, f->generic.scrAlign, true, color_identity, UI_FIELD_PIC);
		}
		else {
			UI_DrawChar (menu->x + f->generic.x + (3+i)*f->generic.textSize, menu->y + f->generic.y - FIELD_VOFFSET,
						f->generic.textSize, f->generic.scrAlign, 19, 255, 255, 255, 255, false, false);
			UI_DrawChar (menu->x + f->generic.x + (3+i)*f->generic.textSize, menu->y + f->generic.y + FIELD_VOFFSET,
						f->generic.textSize, f->generic.scrAlign, 25, 255, 255, 255, 255, false, false);
		}
	}

	// draw right
	if (ui_new_textfield->integer) {
		UI_DrawPicST (menu->x + f->generic.x + (3+f->visible_length)*f->generic.textSize, menu->y + f->generic.y - FIELD_VOFFSET,
					f->generic.textSize, f->generic.textSize*2, stCoord_field_right, f->generic.scrAlign, true, color_identity, UI_FIELD_PIC);
	}
	else {
		UI_DrawChar (menu->x + f->generic.x + (3+f->visible_length)*f->generic.textSize, menu->y + f->generic.y - FIELD_VOFFSET,
					f->generic.textSize, f->generic.scrAlign, 20, 255, 255, 255, 255, false, false);
		UI_DrawChar (menu->x + f->generic.x + (3+f->visible_length)*f->generic.textSize, menu->y + f->generic.y + FIELD_VOFFSET,
					f->generic.textSize, f->generic.scrAlign, 26, 255, 255, 255, 255, false, true);
	}

	// add cursor thingie
	if ( (UI_ItemAtMenuCursor(menu) == f)  && (((int)(Sys_Milliseconds()/250)) & 1) )
		Com_sprintf (tempbuffer, sizeof(tempbuffer), "%s%c", tempbuffer, 11);

	UI_DrawMenuString (menu->x + f->generic.x + (int)(f->generic.textSize*2.5),
						menu->y + f->generic.y, f->generic.textSize, f->generic.scrAlign, tempbuffer, hoverAlpha, false, false);
}

void UI_MenuField_Setup (menuField_s *f)
{
	menuFramework_s	*menu = f->generic.parent;

	// set min and max coords
	f->generic.topLeft[0] = menu->x + f->generic.x + RCOLUMN_OFFSET;
	f->generic.topLeft[1] = menu->y + f->generic.y;
	f->generic.botRight[0] = f->generic.topLeft[0] + (f->visible_length+2)*MENU_FONT_SIZE;
	f->generic.botRight[1] = f->generic.topLeft[1] + MENU_FONT_SIZE;
	f->generic.dynamicWidth = 0;
	f->generic.dynamicHeight = 0;
	f->generic.isExtended = false;
	f->generic.valueChanged = false;
}

qboolean UI_MenuField_Key (menuField_s *f, int key)
{
	extern int keydown[];

	if (!f) return false;

	key = Key_ParseKeypad (key);

	// mxd- This blocked Shift-Ins combo in the next block.
	// Knightmare- allow only the INS key thru, otherwise mouse events end up as text input!
	if ( (key > 127) && (key != K_INS) && (key != K_KP_INS) )
		return false;

	// support pasting from the clipboard
	if ( ( toupper(key) == 'V' && keydown[K_CTRL] ) ||
		 ( ( (key == K_INS) || (key == K_KP_INS) ) && keydown[K_SHIFT] ) )
	{
		char *cbd;
		
		if ( ( cbd = Sys_GetClipboardData() ) != 0 )
		{
			strtok( cbd, "\n\r\b" );

			strncpy( f->buffer, cbd, f->length - 1 );
			f->cursor = (int)strlen( f->buffer );
			f->visible_offset = f->cursor - f->visible_length;
			if ( f->visible_offset < 0 )
				f->visible_offset = 0;

			free( cbd );
		}
	}
	else
	{
		switch ( key )
		{
		case K_KP_LEFTARROW:
		case K_LEFTARROW:
		case K_BACKSPACE:
			if ( f->cursor > 0 )
			{
				memmove( &f->buffer[f->cursor-1], &f->buffer[f->cursor], strlen( &f->buffer[f->cursor] ) + 1 );
				f->cursor--;

				if (f->visible_offset)
					f->visible_offset--;
			}
			break;

		case K_KP_DEL:
		case K_DEL:
			memmove( &f->buffer[f->cursor], &f->buffer[f->cursor+1], strlen( &f->buffer[f->cursor+1] ) + 1 );
			break;

		case K_KP_ENTER:
		case K_ENTER:
		case K_ESCAPE:
		case K_TAB:
			return false;

		case K_SPACE:
		default:
			if ( !isdigit(key) && (f->generic.flags & QMF_NUMBERSONLY) )
				return false;

			if (f->cursor < f->length)
			{
				f->buffer[f->cursor++] = key;
				f->buffer[f->cursor] = 0;

				if (f->cursor > f->visible_length)
					f->visible_offset++;
			}
			break;
		}
	}

	if (!f->generic.cvarNoSave)
		UI_MenuField_SaveValue (f);
	else
		f->generic.valueChanged = UI_MenuField_ValueChanged (f);

	return true;
}

//=========================================================

void UI_MenuLabel_Draw (menuLabel_s *l)
{
	menuFramework_s	*menu = l->generic.parent;
	int				alpha = UI_MouseOverAlpha(&l->generic);
	qboolean		left = (l->generic.flags & QMF_LEFT_JUSTIFY);

	if (!l->generic.name)
		return;

	UI_DrawMenuString (menu->x + l->generic.x + (left ? LCOLUMN_OFFSET : 0), menu->y + l->generic.y,
						l->generic.textSize, l->generic.scrAlign, l->generic.name, alpha, !left, true);
}

void UI_MenuLabel_Setup (menuLabel_s *l)
{
	menuFramework_s	*menu = l->generic.parent;
	int				nameWidth;

	nameWidth = (l->generic.name) ? (int)strlen(l->generic.name)*l->generic.textSize : 0;

	// set min and max coords
	if (l->generic.flags & QMF_LEFT_JUSTIFY) {
		l->generic.topLeft[0] = menu->x + l->generic.x + LCOLUMN_OFFSET;
		l->generic.botRight[0] = l->generic.topLeft[0] + nameWidth;
	}
	else {
		l->generic.topLeft[0] = menu->x + l->generic.x - nameWidth;
		l->generic.botRight[0] = l->generic.topLeft[0] + nameWidth;
	}
	l->generic.topLeft[1] = menu->y + l->generic.y;
	l->generic.botRight[1] = l->generic.topLeft[1] + l->generic.textSize;
	l->generic.dynamicWidth = 0;
	l->generic.dynamicHeight = 0;
	l->generic.isExtended = false;
	l->generic.valueChanged = false;
	l->generic.flags |= QMF_NOINTERACTION;
}

//=========================================================

char *UI_MenuSlider_GetValue (menuSlider_s *s)
{
	return va("%f", ((float)s->curPos * s->increment) + s->baseValue);
}

void UI_MenuSlider_SetValue (menuSlider_s *s)
{
	if (s->generic.cvar && strlen(s->generic.cvar) > 0)
	{
		UI_ClampCvarForControl (&s->generic);
		s->curPos	= (int)ceil((Cvar_VariableValue(s->generic.cvar) - s->baseValue) / s->increment);
		s->curPos = min(max(s->curPos, 0), s->maxPos);
	}
}

void UI_MenuSlider_SaveValue (menuSlider_s *s)
{
	if (s->generic.cvar && strlen(s->generic.cvar) > 0)
	{
		Cvar_SetValue (s->generic.cvar, ((float)s->curPos * s->increment) + s->baseValue);
	}
	s->generic.valueChanged = false;
}

qboolean UI_MenuSlider_ValueChanged (menuSlider_s *s)
{
	if (!s->generic.cvar || !strlen(s->generic.cvar))	// must have a valid cvar
		return false;
	if (!s->generic.cvarNoSave)	// only for cvarNoSave items
		return false;

	return (s->curPos != (int)ceil((Cvar_VariableValue(s->generic.cvar) - s->baseValue) / s->increment));
}

void UI_MenuSlider_CheckSlide (menuSlider_s *s)
{
	if (!s) return;

	s->curPos = min(max(s->curPos, 0), s->maxPos);

	if (!s->generic.cvarNoSave)
		UI_MenuSlider_SaveValue (s);
	else
		s->generic.valueChanged = UI_MenuSlider_ValueChanged (s);

	if (s->generic.callback)
		s->generic.callback (s);
}

void UI_MenuSlider_DoSlide (menuSlider_s *s, int dir)
{
	if (!s) return;

	s->curPos += dir;
	UI_MenuSlider_CheckSlide (s);
}

void UI_MenuSlider_ClickPos (menuFramework_s *menu, void *menuitem)
{
	menuSlider_s	*slider = (menuSlider_s *)menuitem;
	menuCommon_s	*item = (menuCommon_s *)menuitem;
	int				sliderPos, min, max;
	float			x, w, range;

	range = min(max((float)slider->curPos / (float)slider->maxPos, 0), 1);
	sliderPos = (int)(RCOLUMN_OFFSET + SLIDER_ENDCAP_WIDTH
				+ (float)SLIDER_RANGE*(float)SLIDER_SECTION_WIDTH*range);
	x = menu->x + item->x + sliderPos;
	w = SLIDER_KNOB_WIDTH;
	SCR_ScaleCoords (&x, NULL, &w, NULL, item->scrAlign);
	min = x - w/2;
	max = x + w/2;

	if (ui_mousecursor.x < min)
		UI_MenuSlider_DoSlide (slider, -1);
	if (ui_mousecursor.x > max)
		UI_MenuSlider_DoSlide (slider, 1);
}

void UI_MenuSlider_DragPos (menuFramework_s *menu, void *menuitem)
{
	menuSlider_s	*slider = (menuSlider_s *)menuitem;
	float			newValue, sliderbase;

	sliderbase = menu->x + slider->generic.x + RCOLUMN_OFFSET + SLIDER_ENDCAP_WIDTH;
	SCR_ScaleCoords (&sliderbase, NULL, NULL, NULL, slider->generic.scrAlign);
	newValue = (float)(ui_mousecursor.x - sliderbase)
				/ (SLIDER_RANGE * SCR_ScaledScreen(SLIDER_SECTION_WIDTH));
	slider->curPos = newValue * (float)slider->maxPos;

	UI_MenuSlider_CheckSlide (slider);
}

qboolean UI_MenuSlider_Check_Mouseover (menuSlider_s *s)
{
	int				min[2], max[2];
	float			x1, y1, x2, y2;

	x1 = s->barTopLeft[0];
	y1 = s->barTopLeft[1];
	x2 = s->generic.botRight[0];
	y2 = s->generic.botRight[1];

	SCR_ScaleCoords (&x1, &y1, NULL, NULL, s->generic.scrAlign);
	SCR_ScaleCoords (&x2, &y2, NULL, NULL, s->generic.scrAlign);
	min[0] = x1;	max[0] = x2;
	min[1] = y1;	max[1] = y2;

	if ( ui_mousecursor.x >= min[0] && ui_mousecursor.x <= max[0] 
		&& ui_mousecursor.y >= min[1] &&  ui_mousecursor.y <= max[1] )
		return true;
	else
		return false;
}

char *UI_MenuSlider_Click (menuSlider_s *s, qboolean mouse2)
{
	if (mouse2)
	{
		if ( UI_MenuSlider_Check_Mouseover(s) )
			UI_MenuSlider_ClickPos (s->generic.parent, s);
		else
			UI_MenuSlider_DoSlide (s, -1);
		return ui_menu_move_sound;
	}
	else
	{
		if ( UI_MenuSlider_Check_Mouseover(s) ) {
			UI_MenuSlider_DragPos (s->generic.parent, s);
			return ui_menu_drag_sound;
		}
		else {
			UI_MenuSlider_DoSlide (s, 1);
			return ui_menu_move_sound;
		}
	}
}

void UI_MenuSlider_Draw (menuSlider_s *s)
{
//	menuFramework_s	*menu = s->generic.parent;
	int		i, x, y, hoverAlpha = UI_MouseOverAlpha(&s->generic);
	float	tmpValue;
	char	valueText[8];

	// name and header
	UI_MenuCommon_DrawItemName (&s->generic, LCOLUMN_OFFSET, 0, RCOLUMN_OFFSET, -MENU_LINE_SIZE, hoverAlpha);

	if (!s->maxPos)
		s->maxPos = 1;
	if (!s->increment)
		s->increment = 1.0f;

	s->range = (float)s->curPos / (float)s->maxPos;

	if (s->range < 0)
		s->range = 0;
	if (s->range > 1)
		s->range = 1;

	x = s->generic.topLeft[0] + RCOLUMN_OFFSET;
	y = s->generic.topLeft[1] - SLIDER_V_OFFSET;

	// draw left
#ifdef NOTTHIRTYFLIGHTS
	UI_DrawPicST (x, y, SLIDER_ENDCAP_WIDTH, SLIDER_HEIGHT,
					stCoord_slider_left, s->generic.scrAlign, true, color_identity, UI_SLIDER_PIC);
//	UI_DrawChar (s->generic.topLeft[0] + RCOLUMN_OFFSET,
//				s->generic.topLeft[1], s->generic.textSize, s->generic.scrAlign, 128, 255,255,255,255, false, false);
#else
    UI_DrawChar (
            x, y,
            ALIGN_CENTER,
            s->generic.textSize,
            FONT_UI,
            255,255,255,255,
            false, false);
#endif

	// draw center
	x += SLIDER_ENDCAP_WIDTH;
	for (i = 0; i < SLIDER_RANGE; i++) {
#ifdef NOTTHIRTYFLIGHTS
		UI_DrawPicST (x + i*SLIDER_SECTION_WIDTH, y, SLIDER_SECTION_WIDTH, SLIDER_HEIGHT,
						stCoord_slider_center, s->generic.scrAlign, true, color_identity, UI_SLIDER_PIC);
	//	UI_DrawChar (s->generic.topLeft[0] + (i+1)*s->generic.textSize + RCOLUMN_OFFSET,
	//				s->generic.topLeft[1], s->generic.textSize, s->generic.scrAlign, 129, 255,255,255,255, false, false);
#else
        UI_DrawChar (
            s->generic.topLeft[0] + (i+1)*s->generic.textSize + RCOLUMN_OFFSET, 
                y, ALIGN_CENTER,
                s->generic.textSize,
                FONT_UI,
                255,255,255,255,
                false, false);
#endif        
	}

	// draw right
#ifdef NOTTHIRTYFLIGHTS
	UI_DrawPicST (x + i*SLIDER_SECTION_WIDTH, y, SLIDER_ENDCAP_WIDTH, SLIDER_HEIGHT,
					stCoord_slider_right, s->generic.scrAlign, true, color_identity, UI_SLIDER_PIC);
//	UI_DrawChar (s->generic.topLeft[0] + (i+1)*s->generic.textSize + RCOLUMN_OFFSET,
//				s->generic.topLeft[1], s->generic.textSize, s->generic.scrAlign, 130, 255,255,255,255, false, false);
#else
    UI_DrawChar (
            s->generic.topLeft[0] + (i+1)*s->generic.textSize + RCOLUMN_OFFSET, 
            y, ALIGN_CENTER,
            s->generic.textSize,
            FONT_UI,
            255,255,255,255,
            false, false);
#endif

	// draw knob
#ifdef NOTTHIRTYFLIGHTS
	UI_DrawPicST (x + SLIDER_RANGE*SLIDER_SECTION_WIDTH*s->range - (SLIDER_KNOB_WIDTH/2), y, SLIDER_KNOB_WIDTH, SLIDER_HEIGHT,
					stCoord_slider_knob, s->generic.scrAlign, true, color_identity, UI_SLIDER_PIC);
//	UI_DrawChar (s->generic.topLeft[0] + s->generic.textSize*((SLIDER_RANGE-1)*s->range+1) + RCOLUMN_OFFSET,
//				s->generic.topLeft[1], s->generic.textSize, s->generic.scrAlign, 131, 255,255,255,255, false, true);
#else
    UI_DrawChar (
            s->generic.topLeft[0] + s->generic.textSize*((SLIDER_RANGE-1)*s->range+1) + RCOLUMN_OFFSET,
            y, ALIGN_CENTER,
            s->generic.textSize,
            FONT_UI,
            255,255,255,255,
            false, true);
#endif

	// draw value
	tmpValue = s->curPos * s->increment + s->baseValue;
	if (s->displayAsPercent) {
		tmpValue *= 100.0f;
		Com_sprintf (valueText, sizeof(valueText), "%.0f%%", tmpValue);
	}
	else {
		if (fabs((int)tmpValue - tmpValue) < 0.01f)
			Com_sprintf (valueText, sizeof(valueText), "%i", (int)tmpValue);
		else
			Com_sprintf (valueText, sizeof(valueText), "%4.2f", tmpValue);
	}
	UI_DrawMenuString (s->generic.topLeft[0] + RCOLUMN_OFFSET + 2*SLIDER_ENDCAP_WIDTH + i*SLIDER_SECTION_WIDTH + MENU_FONT_SIZE/2,
						s->generic.topLeft[1] + SLIDER_V_OFFSET, MENU_FONT_SIZE-2, s->generic.scrAlign, valueText, hoverAlpha, false, false);
//	UI_DrawMenuString (s->generic.topLeft[0] + s->generic.textSize*SLIDER_RANGE + RCOLUMN_OFFSET + 2.5*MENU_FONT_SIZE,
//						s->generic.topLeft[1] + SLIDER_V_OFFSET, MENU_FONT_SIZE-2, s->generic.scrAlign, valueText, hoverAlpha, false, false);
}

void UI_MenuSlider_Setup (menuSlider_s *s)
{
	menuFramework_s	*menu = s->generic.parent;
//	int				nameWidth;

//	nameWidth = (s->generic.name) ? (int)strlen(s->generic.name)*s->generic.textSize : 0;
	if (!s->baseValue)	s->baseValue = 0.0f;
	if (!s->increment)	s->increment = 1.0f;

	// set min and max coords
	s->generic.topLeft[0] = menu->x + s->generic.x;
	s->generic.topLeft[1] = menu->y + s->generic.y;
	s->generic.botRight[0] = s->generic.topLeft[0] + RCOLUMN_OFFSET + SLIDER_RANGE*SLIDER_SECTION_WIDTH + 2*SLIDER_ENDCAP_WIDTH;
	s->generic.botRight[1] = s->generic.topLeft[1] + SLIDER_HEIGHT;
	s->barTopLeft[0] = s->generic.topLeft[0] + RCOLUMN_OFFSET;
	s->barTopLeft[1] = s->generic.topLeft[1];
	s->generic.dynamicWidth = 0;
	s->generic.dynamicHeight = 0;
	s->generic.isExtended = false;
	s->generic.valueChanged = false;
}

//=========================================================

char *UI_MenuPicker_GetValue (menuPicker_s *p)
{
	if (p->bitFlag) {
		return va("%i", (p->curValue ? (!p->invertValue) : p->invertValue) ? p->bitFlag : 0);
	}
	else if (p->bitFlags != NULL) {
		return va("%i", (p->curValue ? p->bitFlags[p->curValue] : 0));
	}
	else {
		if (p->itemValues)
			return va("%s", p->itemValues[p->curValue]);
		else
			return va("%i", p->curValue);
	}
}

void UI_MenuPicker_SetValue (menuPicker_s *p)
{
	if (p->generic.cvar && strlen(p->generic.cvar) > 0)
	{
		if (p->itemValues) {
			p->curValue = UI_GetIndexForStringValue(p->itemValues, Cvar_VariableString(p->generic.cvar));
		}
		else
		{
			if (p->invertValue) {
				p->curValue = (Cvar_VariableValue(p->generic.cvar) < 0);
			}
			else {
				UI_ClampCvarForControl (&p->generic);
				p->curValue = Cvar_VariableInteger(p->generic.cvar);
			}
		}
	}
	else if (p->bitFlag)
	{
		menuFramework_s	*menu = p->generic.parent;

		p->curValue = ( p->invertValue ? !(menu->bitFlags & p->bitFlag) : (menu->bitFlags & p->bitFlag) ) ? 1 : 0;
	}
	else if (p->bitFlags != NULL)
	{
		menuFramework_s	*menu = p->generic.parent;
		int				i, foundBit=0;

		for (i=1; i<p->numItems; i++)
			if (menu->bitFlags & p->bitFlags[i]) {
				foundBit = i;
				break;
			}
		p->curValue = foundBit;
	}
}

void UI_MenuPicker_SaveValue (menuPicker_s *p)
{
	if (p->generic.cvar && strlen(p->generic.cvar) > 0)
	{
		if (p->itemValues) {
			// Don't save to cvar if this itemvalue is the wildcard
			if ( Q_stricmp(va("%s", p->itemValues[p->curValue]), UI_ITEMVALUE_WILDCARD) != 0 )
				Cvar_Set (p->generic.cvar, va("%s", p->itemValues[p->curValue]));
		}
		else
		{
			if (p->invertValue) {
				Cvar_SetValue (p->generic.cvar, Cvar_VariableValue(p->generic.cvar) * -1 );
			}
			else {
				Cvar_SetInteger (p->generic.cvar, p->curValue);
			}
		}
	}
	else if (p->bitFlag)
	{
		menuFramework_s	*menu = p->generic.parent;

		UI_SetMenuBitFlags (menu, p->bitFlag, (p->curValue ? (!p->invertValue) : p->invertValue));
	}
	else if (p->bitFlags != NULL)
	{
		menuFramework_s	*menu = p->generic.parent;
		int				i, clearBits=0;

		for (i=1; i<p->numItems; i++)
			clearBits |= p->bitFlags[i];

		UI_SetMenuBitFlags (menu, clearBits, false);
		UI_SetMenuBitFlags (menu, p->bitFlags[p->curValue], true);
	}
	p->generic.valueChanged = false;
}

qboolean UI_MenuPicker_ValueChanged (menuPicker_s *p)
{
	if ( (p->bitFlag != 0) || (p->bitFlags != NULL) )	// doesn't apply to bitflag/bitflags
		return false;
	if (!p->generic.cvar || !strlen(p->generic.cvar))	// must have a valid cvar
		return false;
	if (!p->generic.cvarNoSave)	// only for cvarNoSave items
		return false;
		
	if (p->itemValues)
		return ( p->curValue != UI_GetIndexForStringValue(p->itemValues, Cvar_VariableString(p->generic.cvar)) );
	else {
		UI_ClampCvarForControl (&p->generic);
		return ( p->curValue != Cvar_VariableInteger(p->generic.cvar) );
	}
}

void UI_MenuPicker_DoEnter (menuPicker_s *p)
{
	if (!p || !p->itemNames || !p->numItems)
		return;

	p->curValue++;
	if (p->itemNames[p->curValue] == 0)
		p->curValue = 0;

	if (!p->generic.cvarNoSave)
		UI_MenuPicker_SaveValue (p);
	else
		p->generic.valueChanged = UI_MenuPicker_ValueChanged (p);

	if (p->generic.callback)
		p->generic.callback (p);
}

void UI_MenuPicker_DoSlide (menuPicker_s *p, int dir)
{
	if (!p || !p->itemNames || !p->numItems)
		return;

	p->curValue += dir;

	if (p->generic.flags & QMF_NOLOOP) // don't allow looping around
	{
		if (p->curValue < 0)
			p->curValue = 0;
		else if (p->itemNames[p->curValue] == 0)
			p->curValue--;
	}
	else {
		if (p->curValue < 0)
			p->curValue = p->numItems-1; // was 0
		else if (p->itemNames[p->curValue] == 0)
			p->curValue = 0; // was --
	}

	if (!p->generic.cvarNoSave)
		UI_MenuPicker_SaveValue (p);
	else
		p->generic.valueChanged = UI_MenuPicker_ValueChanged (p);

	if (p->generic.callback)
		p->generic.callback (p);
}

char *UI_MenuPicker_Click (menuPicker_s *p, qboolean mouse2)
{
	if (!p || !p->itemNames || !p->numItems)
		return ui_menu_null_sound;

	if (mouse2) {
		UI_MenuPicker_DoSlide (p, -1);
	}
	else {
		UI_MenuPicker_DoSlide (p, 1);
	}

	return ui_menu_move_sound;
}

void UI_MenuPicker_Draw (menuPicker_s *p)
{
	int		alpha;
	char	buffer[100];

	if (!p)	return;

	alpha = UI_MouseOverAlpha(&p->generic);

	if (p->generic.name)
	{
		UI_DrawMenuString (p->generic.x + p->generic.parent->x + LCOLUMN_OFFSET,
							p->generic.y + p->generic.parent->y, p->generic.textSize, p->generic.scrAlign, p->generic.name, alpha, true, true);
	}
	if (!strchr(p->itemNames[p->curValue], '\n'))
	{
		UI_DrawMenuString (p->generic.x + p->generic.parent->x + RCOLUMN_OFFSET,
							p->generic.y + p->generic.parent->y, p->generic.textSize, p->generic.scrAlign, p->itemNames[p->curValue], alpha, false, false);
	}
	else
	{
		Q_strncpyz (buffer, sizeof(buffer), p->itemNames[p->curValue]);
		*strchr(buffer, '\n') = 0;
		UI_DrawMenuString (p->generic.x + p->generic.parent->x + RCOLUMN_OFFSET,
						p->generic.y + p->generic.parent->y, p->generic.textSize, p->generic.scrAlign, buffer, alpha, false, false);
		Q_strncpyz (buffer, sizeof(buffer), strchr( p->itemNames[p->curValue], '\n' ) + 1);
		UI_DrawMenuString (p->generic.x + p->generic.parent->x + RCOLUMN_OFFSET,
						p->generic.y + p->generic.parent->y + MENU_LINE_SIZE, p->generic.textSize, p->generic.scrAlign, buffer, alpha, false, false);
	}
}

void UI_MenuPicker_SetDynamicSize (menuPicker_s *p)
{
	if (!p)	return;

	p->generic.dynamicWidth = 0;
	p->generic.dynamicWidth += RCOLUMN_OFFSET;
	if ( (p->curValue < p->numItems) && p->itemNames[p->curValue] && (strlen(p->itemNames[p->curValue]) > 0) )
	{
		p->generic.dynamicWidth += (int)strlen(p->itemNames[p->curValue])*p->generic.textSize;
	}
}

void UI_MenuPicker_Setup (menuPicker_s *p)
{
	menuFramework_s	*menu = p->generic.parent;
	int				i, j, len;

	for (i=0; p->itemNames[i]; i++);
	p->numItems = i;

	if (p->itemValues)	// Check if itemvalues count matches itemnames
	{
		for (j=0; p->itemValues[j]; j++);
		if (j != i) {
			Com_Printf (S_COLOR_YELLOW"UI_MenuPicker_Setup: itemvalues size mismatch for %s!\n",
						(p->generic.name && (p->generic.name[0] != 0)) ? p->generic.name : "<noname>");
		}
	}

	// set min and max coords
	len = (p->generic.name) ? (int)strlen(p->generic.name) : 0;
	p->generic.topLeft[0] = menu->x + p->generic.x - (len+2)*p->generic.textSize;
	p->generic.topLeft[1] = menu->y + p->generic.y;
	p->generic.botRight[0] = p->generic.topLeft[0] + (len+2)*p->generic.textSize;
	p->generic.botRight[1] = p->generic.topLeft[1] + p->generic.textSize;
	p->generic.dynamicWidth = 0;
	p->generic.dynamicHeight = 0;
	p->generic.isExtended = false;
	p->generic.valueChanged = false;

	UI_MenuPicker_SetDynamicSize (p);
}

//=========================================================

char *UI_MenuCheckBox_GetValue (menuCheckBox_s *c)
{
	if (c->bitFlag) {
		return va("%i", (c->curValue ? (!c->invertValue) : c->invertValue) ? c->bitFlag : 0);
	}
	else {
		if (c->invertValue)
			return va("%i", c->curValue ? -1 : 1 );
		else
			return va("%f", c->baseValue + ((float)c->curValue * c->increment));
	}
}

void UI_MenuCheckBox_SetValue (menuCheckBox_s *c)
{
	if (c->generic.cvar && strlen(c->generic.cvar) > 0)
	{
		if (c->invertValue) {
			c->curValue	= (Cvar_VariableValue(c->generic.cvar) < 0);
		}
		else {
			UI_ClampCvarForControl (&c->generic);
			c->curValue	= (Cvar_VariableValue(c->generic.cvar) != c->baseValue);
		}
	}
	else if (c->bitFlag)
	{
		menuFramework_s	*menu = c->generic.parent;

		c->curValue = ( c->invertValue ? !(menu->bitFlags & c->bitFlag) : (menu->bitFlags & c->bitFlag) ) ? 1 : 0;
	}
}

void UI_MenuCheckBox_SaveValue (menuCheckBox_s *c)
{
	if (c->generic.cvar && strlen(c->generic.cvar) > 0)
	{
		if (c->invertValue)
			Cvar_SetValue (c->generic.cvar, Cvar_VariableValue(c->generic.cvar) * -1 );
		else
			Cvar_SetValue (c->generic.cvar, c->baseValue + ((float)c->curValue * c->increment));
	}
	else if (c->bitFlag)
	{
		menuFramework_s	*menu = c->generic.parent;

		UI_SetMenuBitFlags (menu, c->bitFlag, (c->curValue ? (!c->invertValue) : c->invertValue));
	}
	c->generic.valueChanged = false;
}

qboolean UI_MenuCheckBox_ValueChanged (menuCheckBox_s *c)
{
	if (c->bitFlag)				// doesn't apply to bitflags
		return false;
	if (!c->generic.cvar || !strlen(c->generic.cvar))	// must have a valid cvar
		return false;
	if (!c->generic.cvarNoSave)	// only for cvarNoSave items
		return false;

	if (c->invertValue)
		return ( c->curValue != (Cvar_VariableValue(c->generic.cvar) < 0) );
	else {
		UI_ClampCvarForControl (&c->generic);
		return ( c->curValue != (Cvar_VariableValue(c->generic.cvar) != c->baseValue) );
	}
}

void UI_MenuCheckBox_DoEnter (menuCheckBox_s *c)
{
	c->curValue = !c->curValue;

	if (!c->generic.cvarNoSave)
		UI_MenuCheckBox_SaveValue (c);
	else
		c->generic.valueChanged = UI_MenuCheckBox_ValueChanged (c);

	if (c->generic.callback)
		c->generic.callback (c);
}

char *UI_MenuCheckBox_Click (menuCheckBox_s *c, qboolean mouse2)
{
	UI_MenuCheckBox_DoEnter (c);

	return ui_menu_move_sound;
}

void UI_MenuCheckBox_DoSlide (menuCheckBox_s *c, int unused)
{
	UI_MenuCheckBox_DoEnter (c);
}

void UI_MenuCheckBox_Draw (menuCheckBox_s *c)
{
	menuFramework_s	*menu = c->generic.parent;
	int				alpha = UI_MouseOverAlpha(&c->generic);
	qboolean		left = (c->generic.flags & QMF_LEFT_JUSTIFY);

	if (c->generic.name)
		UI_DrawMenuString (menu->x + c->generic.x + (left ? (2*RCOLUMN_OFFSET + MENU_FONT_SIZE + 2) : LCOLUMN_OFFSET),
							menu->y + c->generic.y, MENU_FONT_SIZE, c->generic.scrAlign, c->generic.name, alpha, !left, true);

	UI_DrawPicST (menu->x + c->generic.x + RCOLUMN_OFFSET,
				menu->y + c->generic.y-CHECKBOX_V_OFFSET, CHECKBOX_WIDTH, CHECKBOX_HEIGHT,
				(c->curValue) ? stCoord_checkbox_on : stCoord_checkbox_off, c->generic.scrAlign, false, color_identity, UI_CHECKBOX_PIC);	
}

void UI_MenuCheckBox_Setup (menuCheckBox_s *c)
{
	menuFramework_s	*menu = c->generic.parent;
	int				nameWidth;

	nameWidth = (c->generic.name) ? (int)strlen(c->generic.name)*MENU_FONT_SIZE : 0;
	if (!c->baseValue)	c->baseValue = 0.0f;
	if (!c->increment)	c->increment = 1.0f;

	// set min and max coords
	c->generic.topLeft[0] = menu->x + c->generic.x;
	c->generic.topLeft[1] = menu->y + c->generic.y;
	c->generic.botRight[0] = c->generic.topLeft[0] + RCOLUMN_OFFSET + CHECKBOX_WIDTH;
	c->generic.botRight[1] = c->generic.topLeft[1] + MENU_FONT_SIZE;
	c->generic.dynamicWidth = 0;
	c->generic.dynamicHeight = 0;
	c->generic.isExtended = false;
	c->generic.valueChanged = false;

	if (c->generic.name) {
		if (c->generic.flags & QMF_LEFT_JUSTIFY)
			c->generic.botRight[0] += (nameWidth + RCOLUMN_OFFSET);	
		else
			c->generic.topLeft[0] -= (nameWidth + RCOLUMN_OFFSET);
	}
}

//=========================================================

void UI_MenuImage_Draw (menuImage_s *i)
{
//	menuFramework_s	*menu = i->generic.parent;
	int				curAnimFrame;
	int				oscillate[2] = {0};
	char			*animPicName;
	byte			*bc = i->borderColor;

	// set oscillation if any
	if ( (i->oscillate_amplitude[0] != 0.0f) && (i->oscillate_timeScale[0] != 0.0f) )
		oscillate[0] = (int)(i->oscillate_amplitude[0] * sin(anglemod(cl.time * i->oscillate_timeScale[0])));
	if ( (i->oscillate_amplitude[1] != 0.0f) && (i->oscillate_timeScale[1] != 0.0f) )
		oscillate[1] = (int)(i->oscillate_amplitude[1] * sin(anglemod(cl.time * i->oscillate_timeScale[1])));

	if (i->border > 0)
	{
		if (i->alpha == 255) // just fill whole area for border if not trans
			UI_DrawFill (i->generic.topLeft[0] - i->border + oscillate[0], i->generic.topLeft[1] - i->border + oscillate[1],
							i->drawWidth+(i->border*2), i->height+(i->border*2), i->generic.scrAlign, false, bc[0],bc[1],bc[2],bc[3]);
		else // have to do each side
			UI_DrawBorder ((float)(i->generic.topLeft[0] + oscillate[0]), (float)(i->generic.topLeft[1] + oscillate[1]), (float)i->drawWidth, (float)i->height,
							(float)i->border, i->generic.scrAlign, false, bc[0],bc[1],bc[2],bc[3]);
	}
	if (i->isAnimated) {
		curAnimFrame = (int)((cls.realtime - i->start_time) * i->animTimeScale) % i->numAnimFrames;
		animPicName = va(i->animTemplate, curAnimFrame);
		UI_DrawPic (i->generic.topLeft[0] + oscillate[0], i->generic.topLeft[1] + oscillate[1], i->drawWidth, i->height, i->generic.scrAlign, false, animPicName, i->alpha);
	}
	else if ( i->imageName && (strlen(i->imageName) > 0) ) {
		if (i->overrideColor)
			UI_DrawColoredPic (i->generic.topLeft[0] + oscillate[0], i->generic.topLeft[1] + oscillate[1], i->drawWidth, i->height, i->generic.scrAlign, false, i->imageColor, i->imageName);
		else
			UI_DrawPic (i->generic.topLeft[0] + oscillate[0], i->generic.topLeft[1] + oscillate[1], i->drawWidth, i->height, i->generic.scrAlign, false, i->imageName, i->alpha);
	}
	else
		UI_DrawFill (i->generic.topLeft[0] + oscillate[0], i->generic.topLeft[1] + oscillate[1], i->drawWidth, i->height, i->generic.scrAlign, false, 0,0,0,255);
}

void UI_MenuImage_UpdateCoords (menuImage_s *i)
{
	menuFramework_s	*menu = i->generic.parent;

	// aspect ratio-based width
	if ( i->useAspectRatio && (i->aspectRatio > 0) )
		i->drawWidth = i->height * i->aspectRatio;
	else
		i->drawWidth = i->width;

	i->generic.topLeft[0] = menu->x + i->generic.x;
	i->generic.topLeft[1] = menu->y + i->generic.y;
	i->generic.botRight[0] = i->generic.topLeft[0] + i->drawWidth;
	i->generic.botRight[1] = i->generic.topLeft[1] + i->height;
}

void UI_MenuImage_Setup (menuImage_s *i)
{
	int				idx;
	menuFramework_s	*menu = i->generic.parent;

	// automatic sizing
	if (i->width == -1 || i->height == -1)
	{
		int		w, h;
		if ( i->animTemplate && (strlen(i->animTemplate) > 0) )
			R_DrawGetPicSize (&w, &h, va(i->animTemplate, 0));
		else
			R_DrawGetPicSize (&w, &h, i->imageName);
		if (i->width == -1)
			i->width = w;
		if (i->height == -1)
			i->height = h;
	}

	i->width = max(i->width, 1);
	i->height = max(i->height, 1);
	i->border = max(i->border, 0);

	// aspect ratio-based width
	if ( i->useAspectRatio && (i->aspectRatio > 0) )
		i->drawWidth = i->height * i->aspectRatio;
	else
		i->drawWidth = i->width;

	// automatic centering
	if (i->hCentered)
		i->generic.x = (SCREEN_WIDTH/2 - i->drawWidth/2) - menu->x;
	if (i->vCentered)
		i->generic.y = (SCREEN_HEIGHT/2 - i->height/2) - menu->y;

	// setup animation
	if ( i->animTemplate && (strlen(i->animTemplate) > 0) )
	{
		i->numAnimFrames = max(i->numAnimFrames, 1);
		for (idx = 0; idx < i->numAnimFrames; idx++)
			R_DrawFindPic (va(i->animTemplate, idx));
		i->animTimeScale = min(max(i->animTimeScale, 0.01f), 0.25f);		// anim speed must be between 100 and 4 fps
		i->start_time = cls.realtime;
		i->isAnimated = true;
	}
	else {
		i->isAnimated = false;
	}

	// set min and max coords
	UI_MenuImage_UpdateCoords (i);
	i->generic.dynamicWidth = 0;
	i->generic.dynamicHeight = 0;
	i->generic.isExtended = false;
	i->generic.valueChanged = false;
	i->generic.flags |= QMF_NOINTERACTION;

/*	if (R_DrawFindPic(i->imageName))
		i->imageValid = true;
	else
		i->imageValid = false;*/
}

//=========================================================

void UI_MenuButton_DoEnter (menuButton_s *b)
{
	if (b->generic.flags & QMF_MOUSEONLY)
		return;

	if (b->generic.callback)
		b->generic.callback (b);
}

char *UI_MenuButton_Click (menuButton_s *b, qboolean mouse2)
{
	// skip if it's not mouse2-enabled and mouse1 wasn't clicked
	if (!b->usesMouse2 && mouse2)
		return ui_menu_null_sound;

	if ( mouse2 && b->generic.mouse2Callback )
		b->generic.mouse2Callback (b);
	else if (b->generic.callback)
		b->generic.callback (b);

	return ui_menu_move_sound;
}

void UI_MenuButton_Draw (menuButton_s *b)
{
	menuFramework_s	*menu = b->generic.parent;
	int				oscillate[2] = {0};
	byte			*bc = b->borderColor;

	// set oscillation if any
	if ( (b == ui_mousecursor.menuitem) || (b == UI_ItemAtMenuCursor(menu)) ) {
		if ( (b->hoverOscillate_amplitude[0] != 0.0f) && (b->hoverOscillate_timeScale[0] != 0.0f) )
			oscillate[0] = (int)(b->hoverOscillate_amplitude[0] * sin(anglemod(cl.time * b->hoverOscillate_timeScale[0])));
		if ( (b->hoverOscillate_amplitude[1] != 0.0f) && (b->hoverOscillate_timeScale[1] != 0.0f) )
			oscillate[1] = (int)(b->hoverOscillate_amplitude[1] * sin(anglemod(cl.time * b->hoverOscillate_timeScale[1])));
	}
	else {
		if ( (b->oscillate_amplitude[0] != 0.0f) && (b->oscillate_timeScale[0] != 0.0f) )
			oscillate[0] = (int)(b->oscillate_amplitude[0] * sin(anglemod(cl.time * b->oscillate_timeScale[0])));
		if ( (b->oscillate_amplitude[1] != 0.0f) && (b->oscillate_timeScale[1] != 0.0f) )
			oscillate[1] = (int)(b->oscillate_amplitude[1] * sin(anglemod(cl.time * b->oscillate_timeScale[1])));
	}

	if (b->border > 0)
	{
		if (b->alpha == 255) // just fill whole area for border if not trans
			UI_DrawFill (b->generic.topLeft[0] - b->border + oscillate[0], b->generic.topLeft[1] - b->border + oscillate[1],
							b->drawWidth+(b->border*2), b->height+(b->border*2), b->generic.scrAlign, false, bc[0],bc[1],bc[2],bc[3]);
		else // have to do each side
			UI_DrawBorder ((float)(b->generic.topLeft[0] + oscillate[0]), (float)(b->generic.topLeft[1] + oscillate[1]), (float)b->drawWidth, (float)b->height,
							(float)b->border, b->generic.scrAlign, false, bc[0],bc[1],bc[2],bc[3]);
	}
	if ( ((b == ui_mousecursor.menuitem) || (b == UI_ItemAtMenuCursor(menu)) )
		&& (b->hoverImageName && strlen(b->hoverImageName) > 0) ) {
		if (b->overrideColor)
			UI_DrawColoredPic (b->generic.topLeft[0] + oscillate[0], b->generic.topLeft[1] + oscillate[1], b->drawWidth, b->height, b->generic.scrAlign, false, b->imageColor, b->hoverImageName);
		else
			UI_DrawPic (b->generic.topLeft[0] + oscillate[0], b->generic.topLeft[1] + oscillate[1], b->drawWidth, b->height, b->generic.scrAlign, false, b->hoverImageName, b->alpha);
	}
	else if ( b->imageName && (strlen(b->imageName) > 0) ) {
		if (b->overrideColor)
			UI_DrawColoredPic (b->generic.topLeft[0] + oscillate[0], b->generic.topLeft[1] + oscillate[1], b->drawWidth, b->height, b->generic.scrAlign, false, b->imageColor, b->imageName);
		else
			UI_DrawPic (b->generic.topLeft[0] + oscillate[0], b->generic.topLeft[1] + oscillate[1], b->drawWidth, b->height, b->generic.scrAlign, false, b->imageName, b->alpha);
	}
	else
		UI_DrawFill (b->generic.topLeft[0] + oscillate[0], b->generic.topLeft[1] + oscillate[1], b->drawWidth, b->height, b->generic.scrAlign, false, 0,0,0,255);
}

void UI_MenuButton_UpdateCoords (menuButton_s *b)
{
	menuFramework_s	*menu = b->generic.parent;

	// aspect ratio-based width
	if ( b->useAspectRatio && (b->aspectRatio > 0) )
		b->drawWidth = b->height * b->aspectRatio;
	else
		b->drawWidth = b->width;

	b->generic.topLeft[0] = menu->x + b->generic.x;
	b->generic.topLeft[1] = menu->y + b->generic.y;
	b->generic.botRight[0] = b->generic.topLeft[0] + b->drawWidth;
	b->generic.botRight[1] = b->generic.topLeft[1] + b->height;
}

void UI_MenuButton_Setup (menuButton_s *b)
{
	menuFramework_s	*menu = b->generic.parent;

	// automatic sizing
	if (b->width == -1 || b->height == -1)
	{
		int w, h;
		R_DrawGetPicSize (&w, &h, b->imageName);
		if (b->width == -1)
			b->width = w;
		if (b->height == -1)
			b->height = h;
	}

	b->width = max(b->width, 1);
	b->height = max(b->height, 1);
	b->border = max(b->border, 0);

	// aspect ratio-based width
	if ( b->useAspectRatio && (b->aspectRatio > 0) )
		b->drawWidth = b->height * b->aspectRatio;
	else
		b->drawWidth = b->width;

	// automatic centering
	if (b->hCentered)
		b->generic.x = (SCREEN_WIDTH/2 - b->drawWidth/2) - menu->x;
	if (b->vCentered)
		b->generic.y = (SCREEN_HEIGHT/2 - b->height/2) - menu->y;

	// set min and max coords
	UI_MenuButton_UpdateCoords (b);
	b->generic.dynamicWidth = 0;
	b->generic.dynamicHeight = 0;
	b->generic.isExtended = false;
	b->generic.valueChanged = false;
}

//=========================================================

void UI_MenuRectangle_Draw (menuRectangle_s *r)
{
//	menuFramework_s	*menu = r->generic.parent;
	byte			*bc = r->borderColor;
	byte			*c = r->color;

	if (r->border > 0)
	{
		if (c[3] == 255) // just fill whole area for border if not trans
			UI_DrawFill (r->generic.topLeft[0]-r->border, r->generic.topLeft[1]-r->border,
							r->width+(r->border*2), r->height+(r->border*2), r->generic.scrAlign, false, bc[0],bc[1],bc[2],bc[3]);
		else // have to do each side
			UI_DrawBorder ((float)r->generic.topLeft[0], (float)r->generic.topLeft[1], (float)r->width, (float)r->height,
							(float)r->border, r->generic.scrAlign, false, bc[0],bc[1],bc[2],bc[3]);
	}
	UI_DrawFill (r->generic.topLeft[0], r->generic.topLeft[1], r->width, r->height, r->generic.scrAlign, false, c[0],c[1],c[2],c[3]);
}

void UI_MenuRectangle_UpdateCoords (menuRectangle_s *r)
{
	menuFramework_s	*menu = r->generic.parent;

	r->generic.topLeft[0] = menu->x + r->generic.x;
	r->generic.topLeft[1] = menu->y + r->generic.y;
	r->generic.botRight[0] = r->generic.topLeft[0] + r->width;
	r->generic.botRight[1] = r->generic.topLeft[1] + r->height;
}

void UI_MenuRectangle_Setup (menuRectangle_s *r)
{
	menuFramework_s	*menu = r->generic.parent;

	r->width =	max(r->width, 1);
	r->height =	max(r->height, 1);
	r->border = max(r->border, 0);

	// automatic centering
	if (r->hCentered)
		r->generic.x = (SCREEN_WIDTH/2 - r->width/2) - menu->x;
	if (r->vCentered)
		r->generic.y = (SCREEN_HEIGHT/2 - r->height/2) - menu->y;

	// set min and max coords
	UI_MenuRectangle_UpdateCoords (r);
	r->generic.dynamicWidth = 0;
	r->generic.dynamicHeight = 0;
	r->generic.isExtended = false;
	r->generic.valueChanged = false;
	r->generic.flags |= QMF_NOINTERACTION;
}

//=========================================================

char *UI_MenuListBox_GetValue (menuListBox_s *l)
{
	if (l->itemValues)
		return va("%s", l->itemValues[l->curValue]);
	else
		return va("%i", l->curValue);
}

void UI_MenuListBox_SetValue (menuListBox_s *l)
{
	if (l->generic.cvar && strlen(l->generic.cvar) > 0)
	{
		if (l->itemValues)
			l->curValue = UI_GetIndexForStringValue(l->itemValues, Cvar_VariableString(l->generic.cvar));
		else {
			UI_ClampCvarForControl (&l->generic);
			l->curValue = Cvar_VariableInteger(l->generic.cvar);
		}
		UI_MenuScrollBar_SetPos (&l->scrollState, l->items_y, l->numItems, l->curValue);
	}
}

void UI_MenuListBox_SaveValue (menuListBox_s *l)
{
	if (l->generic.cvar && strlen(l->generic.cvar) > 0)
	{
		if (l->itemValues) {
			// Don't save to cvar if this itemvalue is the wildcard
			if ( Q_stricmp(va("%s", l->itemValues[l->curValue]), UI_ITEMVALUE_WILDCARD) != 0 )
				Cvar_Set (l->generic.cvar, va("%s", l->itemValues[l->curValue]));
		}
		else
			Cvar_SetInteger (l->generic.cvar, l->curValue);
	}
	l->generic.valueChanged = false;
}

qboolean UI_MenuListBox_ValueChanged (menuListBox_s *l)
{
	if (!l->generic.cvar || !strlen(l->generic.cvar))	// must have a valid cvar
		return false;
	if (!l->generic.cvarNoSave)	// only for cvarNoSave items
		return false;
		
	if (l->itemValues)
		return ( l->curValue != UI_GetIndexForStringValue(l->itemValues, Cvar_VariableString(l->generic.cvar)) );
	else {
		UI_ClampCvarForControl (&l->generic);
		return ( l->curValue != Cvar_VariableInteger(l->generic.cvar) );
	}
}

void UI_MenuListBox_DoEnter (menuListBox_s *l)
{
	if (l->generic.dblClkCallback)
		l->generic.dblClkCallback (l);
}

void UI_MenuListBox_DoSlide (menuListBox_s *l, int dir)
{
	int		visibleLines, startItem;

	if (!l->itemNames || !l->numItems)
		return;

	l->curValue += dir;

	if (l->generic.flags & QMF_NOLOOP) // don't allow looping around
	{
		if (l->curValue < 0)
			l->curValue = 0;
		else if (l->curValue >= l->numItems)
			l->curValue = l->numItems-1;
	}
	else {
		if (l->curValue < 0)
			l->curValue = l->numItems-1;
		else if (l->curValue >= l->numItems)
			l->curValue = 0;
	}

	// scroll if needed
	startItem = l->scrollState.scrollPos;
	visibleLines = l->items_y;
	while (l->curValue < startItem) {
		UI_MenuScrollBar_Increment (&l->scrollState, -1);
		startItem = l->scrollState.scrollPos;
	}
	while ( l->curValue > (startItem+visibleLines-1) ) {
		UI_MenuScrollBar_Increment (&l->scrollState, 1);
		startItem = l->scrollState.scrollPos;
	}

	if (!l->generic.cvarNoSave)
		UI_MenuListBox_SaveValue (l);
	else
		l->generic.valueChanged = UI_MenuListBox_ValueChanged (l);

	if (l->generic.callback)
		l->generic.callback (l);
}

char *UI_MenuListBox_Click (menuListBox_s *l, qboolean mouse2)
{
	int			i, oldCurValue, y, itemWidth, itemHeight, visibleLines, startItem, itemIncrement;
	char		*s = ui_menu_null_sound;

	// return if it's just a mouse2 click
	if (mouse2)
		return s;

	// clicked on scroll bar
	if ( UI_MenuScrollBar_Click(&l->scrollState, &l->generic) ) {
		s = ui_menu_move_sound;
	}
	else // clicked on one of the items
	{
		itemWidth = l->itemWidth * MENU_FONT_SIZE;
		itemHeight = l->itemHeight * MENU_LINE_SIZE;
		startItem = l->scrollState.scrollPos;
		visibleLines = l->items_y;
		itemIncrement = itemHeight + l->itemSpacing;
		for (i=startItem; i<l->numItems && i<(startItem+visibleLines); i++)
		{
			y = l->generic.topLeft[1] + (i-startItem)*itemIncrement;
			if ( UI_MouseOverSubItem(l->generic.topLeft[0], y, itemWidth, itemIncrement, l->generic.scrAlign) )
			{
				oldCurValue = l->curValue;
				l->curValue = i;
				s = ui_menu_move_sound;

				if (!l->generic.cvarNoSave)
					UI_MenuListBox_SaveValue (l);
				else
					l->generic.valueChanged = UI_MenuListBox_ValueChanged (l);

				if (oldCurValue != l->curValue) {	// new subitem
					ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 1;	// was 0
					if (l->generic.callback)
						l->generic.callback (l);
				}

				if ( (ui_mousecursor.buttonclicks[MOUSEBUTTON1] == 2) && l->generic.dblClkCallback )
					l->generic.dblClkCallback (l);

				break;
			}
		}
	}
	return s;
}

void UI_MenuListBox_Draw (menuListBox_s *l)
{
//	menuFramework_s	*menu = l->generic.parent;
	int				i, j, x, y, itemWidth, itemHeight, boxWidth, boxHeight, visibleLines, startItem, itemIncrement;
	int				hc[3];
	int				hoverAlpha;
	char			*p, buf1[512], buf2[100];
	qboolean		img_background = (l->background && strlen(l->background) > 0);
	qboolean		itemPulse;

	UI_TextColorHighlight (Cvar_VariableInteger("alt_text_color"), &hc[0], &hc[1], &hc[2]);
	hoverAlpha	= UI_MouseOverAlpha(&l->generic);
	itemWidth	= l->itemWidth * MENU_FONT_SIZE;
	itemHeight	= l->itemHeight * MENU_LINE_SIZE;
	boxWidth	= l->generic.botRight[0] - l->generic.topLeft[0];
	boxHeight	= l->generic.botRight[1] - l->generic.topLeft[1];

	// name and header
	UI_MenuCommon_DrawItemName (&l->generic, -(2*RCOLUMN_OFFSET+l->border), 0, 0, -(l->border + MENU_LINE_SIZE), hoverAlpha);

	// border and background
	if (l->border > 0)
	{
		if (l->backColor[3] == 255 || img_background) // just fill whole area for border if not trans
			UI_DrawFill (l->generic.topLeft[0]-l->border, l->generic.topLeft[1]-l->border,
							boxWidth+(l->border*2), boxHeight+(l->border*2), l->generic.scrAlign, true, l->borderColor[0], l->borderColor[1], l->borderColor[2], l->borderColor[3]);
		else // have to do each side
			UI_DrawBorder ((float)l->generic.topLeft[0], (float)l->generic.topLeft[1], (float)boxWidth, (float)boxHeight,
							(float)l->border, l->generic.scrAlign, true, l->borderColor[0], l->borderColor[1], l->borderColor[2], l->borderColor[3]);
	}
	if (img_background)
		UI_DrawTiledPic (l->generic.topLeft[0], l->generic.topLeft[1], boxWidth, boxHeight,
						l->generic.scrAlign, true, (char *)l->background, (float)(l->backColor[3]/255.0f));
	else if ( !(l->altBackColor[3] > 0) )
		UI_DrawFill (l->generic.topLeft[0], l->generic.topLeft[1], boxWidth, boxHeight,
						l->generic.scrAlign, true, l->backColor[0], l->backColor[1], l->backColor[2], l->backColor[3]);

	// scrollbar
	UI_MenuScrollBar_Draw (&l->generic, &l->scrollState, l->generic.topLeft[0], l->generic.topLeft[1], boxWidth, boxHeight); 

	// items
	x = l->generic.topLeft[0];
	startItem = l->scrollState.scrollPos;
	visibleLines = l->items_y;
	itemIncrement = itemHeight + l->itemSpacing;
	for (i=startItem; i<l->numItems && i<(startItem+visibleLines); i++)
	{
		y = l->generic.topLeft[1]+(i-startItem)*itemIncrement;
		itemPulse = UI_MouseOverSubItem(x, y, itemWidth, itemIncrement, l->generic.scrAlign);

		if (i == l->curValue)
			UI_DrawFill (x, y, itemWidth, itemHeight, l->generic.scrAlign, false, hc[0], hc[1], hc[2], hoverAlpha);
		else if (l->altBackColor[3] > 0) {
			if (i % 2 == 1)
				UI_DrawFill (x, y, itemWidth, itemHeight, l->generic.scrAlign, true, l->altBackColor[0], l->altBackColor[1], l->altBackColor[2], l->altBackColor[3]);
			else
				UI_DrawFill (x, y, itemWidth, itemHeight, l->generic.scrAlign, true, l->backColor[0], l->backColor[1], l->backColor[2], l->backColor[3]);
		}	

		if (l->itemHeight > 1)
		{
		//	strncpy(buf1, l->itemNames[i]);
			Q_strncpyz(buf1, sizeof(buf1), l->itemNames[i]);
			p = strtok(buf1, "\n");
			for (j=0; j<l->itemHeight; j++)
			{
				if (!p)	break;
				strncpy(buf2, va("%s\0", p), l->itemWidth);
				UI_DrawMenuString (x+LISTBOX_ITEM_PADDING, y+LISTBOX_ITEM_PADDING+j*MENU_LINE_SIZE,
									MENU_FONT_SIZE, l->generic.scrAlign, buf2, itemPulse ? hoverAlpha : 255, false, false);
				p = strtok(NULL, "\n");
			}
		}
		else {
			UI_DrawMenuString (x+LISTBOX_ITEM_PADDING, y+LISTBOX_ITEM_PADDING,
								MENU_FONT_SIZE, l->generic.scrAlign, l->itemNames[i], itemPulse ? hoverAlpha : 255, false, false);
			// debug output
		//	if ( !strncmp(l->itemNames[i], "^9", 2) )
		//		Com_Printf("UI_MenuListBox_Draw: drawing orange item text %s\n", l->itemNames[i]);
		}
	}
}

void UI_MenuListBox_Setup (menuListBox_s *l)
{
	menuFramework_s	*menu = l->generic.parent;
	int				i, /*numValues,*/ boxWidth, boxHeight;

	// count # of items
	for (i=0; l->itemNames[i]; i++);
		l->numItems = i;

	// clamp curValue
	l->curValue = min(max(l->curValue, 0), l->numItems-1);

	l->items_y		= max(l->items_y, 3);		// must be at least 3 lines tall
	l->itemWidth	= min(max(l->itemWidth, 6), 100);	// must be at least 6 chars wide
	l->itemHeight	= max(l->itemHeight, 1);	// must be at least 1 line high
	l->itemSpacing	= max(l->itemSpacing, 0);
	l->itemTextSize	= max(l->itemTextSize, 4);
	l->border		= max(l->border, 0);

	boxWidth	= l->itemWidth * MENU_FONT_SIZE + LIST_SCROLLBAR_SIZE;
	boxHeight	= l->items_y * (l->itemHeight*MENU_LINE_SIZE + l->itemSpacing) + l->itemSpacing;

	// set min and max coords
	l->generic.topLeft[0] = menu->x + l->generic.x + RCOLUMN_OFFSET + l->border;
	l->generic.topLeft[1] = menu->y + l->generic.y;
	l->generic.botRight[0] = l->generic.topLeft[0] + boxWidth;
	l->generic.botRight[1] = l->generic.topLeft[1] + boxHeight;
	l->generic.dynamicWidth = 0;
	l->generic.dynamicHeight = 0;
	l->generic.isExtended = false;
	l->generic.valueChanged = false;

	l->scrollState.scrollType = SCROLL_Y; // always scrolls vertically
	l->scrollState.scrollEnabled = (l->numItems > l->items_y);
	l->scrollState.scrollMin = 0;
	l->scrollState.scrollMax = max((l->numItems - l->items_y), 0);
	l->scrollState.scrollNumVisible = l->items_y;
	l->scrollState.scrollTopLeft[0] = l->generic.topLeft[0] + boxWidth - LIST_SCROLLBAR_SIZE;
	l->scrollState.scrollTopLeft[1] = l->generic.topLeft[1];
	l->scrollState.scrollBotRight[0] = l->generic.botRight[0];
	l->scrollState.scrollBotRight[1] = l->generic.botRight[1];
	UI_MenuScrollBar_SetPos (&l->scrollState, l->items_y, l->numItems, l->curValue);

//	l->generic.flags |= QMF_MOUSEONLY;
}

//=========================================================

char *UI_MenuComboBox_GetValue (menuComboBox_s *c)
{
	if (c->itemValues)
		return va("%s", c->itemValues[c->curValue]);
	else
		return va("%i", c->curValue);
}

void UI_MenuComboBox_SetValue (menuComboBox_s *c)
{
	if (c->generic.cvar && strlen(c->generic.cvar) > 0)
	{
		if (c->itemValues)
			c->curValue = UI_GetIndexForStringValue(c->itemValues, Cvar_VariableString(c->generic.cvar));
		else {
			UI_ClampCvarForControl (&c->generic);
			c->curValue = Cvar_VariableInteger(c->generic.cvar);
		}
	}
	else if (c->bitFlags != NULL)
	{
		menuFramework_s	*menu = c->generic.parent;
		int				i, foundBit=0;

		for (i=1; i<c->numItems; i++)
			if (menu->bitFlags & c->bitFlags[i]) {
				foundBit = i;
				break;
			}
		c->curValue = foundBit;
	}
}

void UI_MenuComboBox_SaveValue (menuComboBox_s *c)
{
	if (c->generic.cvar && strlen(c->generic.cvar) > 0)
	{
		if (c->itemValues) {
			// Don't save to cvar if this itemvalue is the wildcard
			if ( Q_stricmp(va("%s", c->itemValues[c->curValue]), UI_ITEMVALUE_WILDCARD) != 0 )
				Cvar_Set (c->generic.cvar, va("%s", c->itemValues[c->curValue]));
		}
		else
			Cvar_SetInteger (c->generic.cvar, c->curValue);
	}
	else if (c->bitFlags != NULL)
	{
		menuFramework_s	*menu = c->generic.parent;
		int				i, clearBits=0;

		for (i=1; i<c->numItems; i++)
			clearBits |= c->bitFlags[i];

		UI_SetMenuBitFlags (menu, clearBits, false);
		UI_SetMenuBitFlags (menu, c->bitFlags[c->curValue], true);
	}
	c->generic.valueChanged = false;
}

qboolean UI_MenuComboBox_ValueChanged (menuComboBox_s *c)
{
	if (c->bitFlags != NULL)	// doesn't apply to bitflags
		return false;
	if (!c->generic.cvar || !strlen(c->generic.cvar))	// must have a valid cvar
		return false;
	if (!c->generic.cvarNoSave)	// only for cvarNoSave items
		return false;
		
	if (c->itemValues)
		return ( c->curValue != UI_GetIndexForStringValue(c->itemValues, Cvar_VariableString(c->generic.cvar)) );
	else {
		UI_ClampCvarForControl (&c->generic);
		return ( c->curValue != Cvar_VariableInteger(c->generic.cvar) );
	}
}

void UI_MenuComboBox_DoSlide (menuComboBox_s *c, int dir)
{
	if (!c->itemNames || !c->numItems)
		return;

	c->curValue += dir;

	if (c->generic.flags & QMF_NOLOOP) // don't allow looping around
	{
		if (c->curValue < 0)
			c->curValue = 0;
		else if (c->itemNames[c->curValue] == 0)
			c->curValue--;
	}
	else {
		if (c->curValue < 0)
			c->curValue = c->numItems-1;
		else if (c->itemNames[c->curValue] == 0)
			c->curValue = 0;
	}

	if (c->generic.isExtended)
		UI_MenuScrollBar_SetPos (&c->scrollState, c->items_y, c->numItems, c->curValue);

	if (!c->generic.cvarNoSave)
		UI_MenuComboBox_SaveValue (c);
	else
		c->generic.valueChanged = UI_MenuComboBox_ValueChanged (c);

	if (c->generic.callback)
		c->generic.callback (c);
}

char *UI_MenuComboBox_Click (menuComboBox_s *c, qboolean mouse2)
{
	int			i, itemWidth, visibleLines, startItem, itemIncrement;
	int			x, y, w, h;
	char		*s = ui_menu_null_sound;

	// return if it's just a mouse2 click
	if (mouse2)
		return s;

	if (c->generic.isExtended && c->scrollState.scrollEnabled)
	{	// clicked on scroll bar
		if ( UI_MenuScrollBar_Click(&c->scrollState, &c->generic) )
			return ui_menu_move_sound;
	}
	x = c->generic.topLeft[0];
	y = c->generic.topLeft[1];
	w = c->generic.botRight[0] - c->generic.topLeft[0];
	h = c->generic.botRight[1] - c->generic.topLeft[1];
	if ( UI_MouseOverSubItem(x, y, w, h, c->generic.scrAlign) ) // clicked on the top
	{
		c->generic.isExtended = !c->generic.isExtended;
		if (c->generic.isExtended)
			UI_MenuScrollBar_SetPos (&c->scrollState, c->items_y, c->numItems, c->curValue);
		s = ui_menu_move_sound;
	}
	else if (c->generic.isExtended) // clicked on one of the items
	{
		itemWidth = c->generic.botRight[0] - c->generic.topLeft[0] - ((c->scrollState.scrollEnabled) ? LIST_SCROLLBAR_SIZE : 0);
		startItem = c->scrollState.scrollPos;
		visibleLines = c->items_y;
		itemIncrement = MENU_LINE_SIZE + c->itemSpacing;
		for (i=startItem; i<c->numItems && i<(startItem+visibleLines); i++)
		{
			y = c->generic.botRight[1] + c->border + (i-startItem)*itemIncrement;
			if ( UI_MouseOverSubItem(c->generic.topLeft[0], y, itemWidth, itemIncrement, c->generic.scrAlign) )
			{
				c->curValue = i;
				c->generic.isExtended = false; // retract box once item is selected
				s = ui_menu_move_sound;

				if (!c->generic.cvarNoSave)
					UI_MenuComboBox_SaveValue (c);
				else
					c->generic.valueChanged = UI_MenuComboBox_ValueChanged (c);

				if (c->generic.callback)
					c->generic.callback (c);
				break;
			}
		}
	}
	return s;
}

void UI_MenuComboBox_Draw (menuComboBox_s *c)
{
	menuFramework_s	*menu = c->generic.parent;
	int				x, y, itemWidth, fieldWidth, fieldHeight;
	int				button_size = LIST_SCROLLBAR_CONTROL_SIZE;
	int				red, green, blue, hoverAlpha;
	float			t_ofs;
	color_t			arrowColor;
	vec4_t			arrowTemp;
	qboolean		mouseClick, mouseOver, itemPulse;

	hoverAlpha		= UI_MouseOverAlpha(&c->generic);
	itemWidth		= c->itemWidth * MENU_FONT_SIZE;
	fieldWidth		= c->generic.botRight[0] - c->generic.topLeft[0];
	fieldHeight		= c->generic.botRight[1] - c->generic.topLeft[1];
//	UI_TextColor (alt_text_color->value, true, &red, &green, &blue);
	UI_TextColor (alt_text_color->integer, true, &red, &green, &blue);
	Vector4Set (arrowColor, red, green, blue, hoverAlpha);

	// name and header
	UI_MenuCommon_DrawItemName (&c->generic, -(2*RCOLUMN_OFFSET+c->border), 0, 0, -(c->border + MENU_LINE_SIZE), hoverAlpha);

	// border and background
	if (c->border > 0)
	{
		if (c->backColor[3] == 255) // just fill whole area for border if not trans
			UI_DrawFill (c->generic.topLeft[0]-c->border, c->generic.topLeft[1]-c->border, fieldWidth+(c->border*2), fieldHeight+(c->border*2),
							c->generic.scrAlign, true, c->borderColor[0],c->borderColor[1],c->borderColor[2],c->borderColor[3]);
		else // have to do each side
			UI_DrawBorder ((float)c->generic.topLeft[0], (float)c->generic.topLeft[1], (float)fieldWidth, (float)fieldHeight,
							(float)c->border, c->generic.scrAlign, true, c->borderColor[0],c->borderColor[1],c->borderColor[2],c->borderColor[3]);
	}
	UI_DrawFill (c->generic.topLeft[0], c->generic.topLeft[1], fieldWidth, fieldHeight,
				c->generic.scrAlign, true, c->backColor[0],c->backColor[1],c->backColor[2],c->backColor[3]);

	// current item
	x = c->generic.topLeft[0];
	y = c->generic.topLeft[1];
	mouseClick	= ( ui_mousecursor.buttonused[MOUSEBUTTON1] && ui_mousecursor.buttonclicks[MOUSEBUTTON1] );
	mouseOver = UI_MouseOverSubItem(x, y, fieldWidth, fieldHeight, c->generic.scrAlign);
	itemPulse = mouseOver && !mouseClick;
	if (mouseOver && mouseClick)
		t_ofs = 0.5;
	else
		t_ofs = 0;

	UI_DrawMenuString (x+LISTBOX_ITEM_PADDING, y+LISTBOX_ITEM_PADDING+c->itemSpacing,
						MENU_FONT_SIZE, c->generic.scrAlign, c->itemNames[c->curValue], itemPulse ? hoverAlpha : 255, false, false);
	
	Vector4Copy (stCoord_arrow_down, arrowTemp);
	arrowTemp[1] += t_ofs;
	arrowTemp[3] += t_ofs;
	// drop down arrow
	UI_DrawFill (x+itemWidth, y, button_size, button_size, c->generic.scrAlign, false, scrollBarColor[0], scrollBarColor[1], scrollBarColor[2], scrollBarColor[3]);
	UI_DrawPicST (x+itemWidth, y, button_size, button_size, arrowTemp, c->generic.scrAlign, false, arrowColor, UI_ARROWS_PIC);

	// catch loss of focus
	if ( ((ui_mousecursor.menuitem != NULL) && (ui_mousecursor.menuitem != c))
		|| ((menuCommon_s *)c) != UI_ItemAtMenuCursor(menu) )
		c->generic.isExtended = false;
}

void UI_MenuComboBox_DrawExtension (menuComboBox_s *c)
{
//	menuFramework_s	*menu = c->generic.parent;
	int				x, y, itemWidth, fieldWidth, fieldHeight, boxTop, boxHeight;
	int				i, visibleLines, startItem, itemIncrement;
	int				hc[3];
	int				hoverAlpha;
//	byte			*borderColor = c->borderColor;
//	byte			*backColor = c->backColor;
	qboolean		itemPulse;

	UI_TextColorHighlight (Cvar_VariableInteger("alt_text_color"), &hc[0], &hc[1], &hc[2]);
	hoverAlpha	= UI_MouseOverAlpha(&c->generic);
	itemWidth	= c->itemWidth * MENU_FONT_SIZE + ((!c->scrollState.scrollEnabled) ? LIST_SCROLLBAR_SIZE : 0);
	fieldWidth	= c->generic.botRight[0] - c->generic.topLeft[0];
	fieldHeight = c->generic.botRight[1] - c->generic.topLeft[1];
	boxTop		= c->generic.topLeft[1] + fieldHeight + c->border;
	boxHeight	= c->dropHeight - c->border;

	// border and background
	if (c->border > 0)
	{
		if (c->backColor[3] == 255)	// just fill whole area for border if not trans
			UI_DrawFill (c->generic.topLeft[0]-c->border, boxTop, fieldWidth+(c->border*2), boxHeight+c->border,
							c->generic.scrAlign, true, c->borderColor[0],c->borderColor[1],c->borderColor[2],c->borderColor[3]);
		else	// have to draw actual border
			UI_DrawBorder ((float)c->generic.topLeft[0], (float)boxTop, (float)fieldWidth, (float)boxHeight,
							(float)c->border, c->generic.scrAlign, true, c->borderColor[0],c->borderColor[1],c->borderColor[2],c->borderColor[3]);
	}
	UI_DrawFill (c->generic.topLeft[0], boxTop, fieldWidth, boxHeight,
					c->generic.scrAlign, true, c->backColor[0],c->backColor[1],c->backColor[2],c->backColor[3]);

	// scrollbar
	if (c->scrollState.scrollEnabled)
		UI_MenuScrollBar_Draw (&c->generic, &c->scrollState, c->generic.topLeft[0], boxTop, fieldWidth, boxHeight); 

	// items
	x = c->generic.topLeft[0];
	startItem = c->scrollState.scrollPos;
	visibleLines = min(c->items_y, c->numItems);	
	itemIncrement = MENU_LINE_SIZE + c->itemSpacing;
	for (i=startItem; i<c->numItems && i<(startItem+visibleLines); i++)
	{
		y = boxTop+(i-startItem)*itemIncrement;
		itemPulse = UI_MouseOverSubItem(x, y, itemWidth, itemIncrement, c->generic.scrAlign);

		if (i == c->curValue)
			UI_DrawFill (x, y, itemWidth, MENU_LINE_SIZE, c->generic.scrAlign, false, hc[0], hc[1], hc[2], hoverAlpha);

		UI_DrawMenuString (x+LISTBOX_ITEM_PADDING, y+LISTBOX_ITEM_PADDING,
							MENU_FONT_SIZE, c->generic.scrAlign, c->itemNames[i], itemPulse ? hoverAlpha : 255, false, false);
	}
}

void UI_MenuComboBox_SetDynamicSize (menuComboBox_s *c)
{
	c->generic.dynamicHeight = (c->generic.isExtended) ? c->dropHeight : 0;
}

void UI_MenuComboBox_Setup (menuComboBox_s *c)
{
	menuFramework_s	*menu = c->generic.parent;
	int				i, fieldWidth, fieldHeight;

	// count # of items
	for (i=0; c->itemNames[i]; i++);
		c->numItems = i;

	// clamp curValue
	c->curValue = min(max(c->curValue, 0), c->numItems-1);

	c->items_y		= max(c->items_y, 3);		// must be at least 3 lines tall
	c->items_y		= min(c->items_y, c->numItems);	// must not be taller than # of items
	c->itemWidth	= min(max(c->itemWidth, 4), 100);	// must be at least 4 chars wide
	c->itemSpacing	= max(c->itemSpacing, 0);
	c->itemTextSize	= max (c->itemTextSize, 4);
	c->border		= max(c->border, 0);

	fieldWidth		= c->itemWidth * MENU_FONT_SIZE + LIST_SCROLLBAR_SIZE;
	fieldHeight		= max((MENU_LINE_SIZE + c->itemSpacing*2), LIST_SCROLLBAR_CONTROL_SIZE);
	c->dropHeight	= c->border + c->items_y * (MENU_LINE_SIZE + c->itemSpacing) - c->itemSpacing;

	// set min and max coords
	c->generic.topLeft[0] = menu->x + c->generic.x + RCOLUMN_OFFSET + c->border;
	c->generic.topLeft[1] = menu->y + c->generic.y;
	c->generic.botRight[0] = c->generic.topLeft[0] + fieldWidth;
	c->generic.botRight[1] = c->generic.topLeft[1] + fieldHeight;
	c->generic.dynamicWidth = 0;
	c->generic.dynamicHeight = 0;
	c->generic.isExtended = false;
	c->generic.valueChanged = false;

	c->scrollState.scrollType = SCROLL_Y; // always scrolls vertically
	c->scrollState.scrollEnabled = (c->numItems > c->items_y);
	c->scrollState.scrollMin = 0;
	c->scrollState.scrollMax = max((c->numItems - c->items_y), 0);
	c->scrollState.scrollNumVisible = c->items_y;
	c->scrollState.scrollTopLeft[0] = c->generic.topLeft[0] + fieldWidth - LIST_SCROLLBAR_SIZE;
	c->scrollState.scrollTopLeft[1] = c->generic.botRight[1] + c->border;
	c->scrollState.scrollBotRight[0] = c->generic.botRight[0];
	c->scrollState.scrollBotRight[1] = c->generic.botRight[1] + c->dropHeight;
	UI_MenuScrollBar_SetPos (&c->scrollState, c->items_y, c->numItems, c->curValue);

	c->generic.flags |= QMF_NOLOOP;
}

//=========================================================

char *UI_MenuListView_GetValue (menuListView_s *l)
{
	if (l->itemValues)
		return va("%s", l->itemValues[l->curValue]);
	else
		return va("%i", l->curValue);
}

void UI_MenuListView_SetValue (menuListView_s *l)
{
	int	scrollUnits;

	if (l->generic.cvar && strlen(l->generic.cvar) > 0)
	{
		if (l->itemValues)
			l->curValue = UI_GetIndexForStringValue(l->itemValues, Cvar_VariableString(l->generic.cvar));
		else {
			UI_ClampCvarForControl (&l->generic);
			l->curValue = Cvar_VariableInteger(l->generic.cvar);
		}

		if (l->scrollState.scrollType == SCROLL_X)
			scrollUnits = (l->numItems / l->items_y) + ((l->numItems % l->items_y > 0) ? 1 : 0);
		else // SCROLL_Y
			scrollUnits = (l->numItems / l->items_x) + ((l->numItems % l->items_x > 0) ? 1 : 0);

		if (l->scrollState.scrollType == SCROLL_X)
			UI_MenuScrollBar_SetPos (&l->scrollState, l->items_x, scrollUnits, l->curValue/l->items_y);
		else // SCROLL_Y
			UI_MenuScrollBar_SetPos (&l->scrollState, l->items_y, scrollUnits, l->curValue/l->items_x);
	}
}

void UI_MenuListView_SaveValue (menuListView_s *l)
{
	if (l->generic.cvar && strlen(l->generic.cvar) > 0)
	{
		if (l->itemValues) {
			// Don't save to cvar if this itemvalue is the wildcard
			if ( Q_stricmp(va("%s", l->itemValues[l->curValue]), UI_ITEMVALUE_WILDCARD) != 0 )
				Cvar_Set (l->generic.cvar, va("%s", l->itemValues[l->curValue]));
		}
		else
			Cvar_SetInteger (l->generic.cvar, l->curValue);
	}
	l->generic.valueChanged = false;
}

qboolean UI_MenuListView_ValueChanged (menuListView_s *l)
{
	if (!l->generic.cvar || !strlen(l->generic.cvar))	// must have a valid cvar
		return false;
	if (!l->generic.cvarNoSave)	// only for cvarNoSave items
		return false;

	if (l->itemValues)
		return ( l->curValue != UI_GetIndexForStringValue(l->itemValues, Cvar_VariableString(l->generic.cvar)) );
	else {
		UI_ClampCvarForControl (&l->generic);
		return ( l->curValue != Cvar_VariableInteger(l->generic.cvar) );
	}
}

char *UI_MenuListView_Click (menuListView_s *l, qboolean mouse2)
{
	int			i, j, oldCurValue, x = 0, y = 0, lineSize, itemWidth, itemHeight, realItemHeight;
	int			startItem, visibleItems[2], itemIncrement[2];
	qboolean	scrollX, foundSelected = false;
	char		*s = ui_menu_null_sound;

	// return if it's just a mouse2 click
	if (mouse2)
		return s;

	// clicked on scroll bar
	if ( UI_MenuScrollBar_Click(&l->scrollState, &l->generic) ) {
		s = ui_menu_move_sound;
	}
	else // clicked on one of the items
	{
		scrollX			= (l->scrollState.scrollType == SCROLL_X);
		lineSize		= l->itemTextSize + 2;
		itemWidth		= l->itemWidth;
		itemHeight		= l->itemHeight;
		realItemHeight	= itemHeight + ((l->listViewType == LISTVIEW_TEXTIMAGE) ? lineSize : 0);

		if (scrollX)
		{
			visibleItems[0] = l->items_x;
			visibleItems[1] = l->items_y;
			itemIncrement[0] = itemWidth + l->itemSpacing;
			itemIncrement[1] = realItemHeight + l->itemSpacing;
		}
		else // scrollY
		{
			visibleItems[0] = l->items_y;
			visibleItems[1] = l->items_x;
			itemIncrement[0] = realItemHeight + l->itemSpacing;
			itemIncrement[1] = itemWidth + l->itemSpacing;
		}

		startItem = l->scrollState.scrollPos*visibleItems[1];
		for (j=0; j<visibleItems[0]; j++)
		{
			if (scrollX)
				x = l->generic.topLeft[0] + j*itemIncrement[0];
			else
				y = l->generic.topLeft[1] + j*itemIncrement[0];
			for (i=startItem; i<l->numItems && i<(startItem+visibleItems[1]); i++)
			{
				if (scrollX)
					y = l->generic.topLeft[1] + (i-startItem)*itemIncrement[1];
				else
					x = l->generic.topLeft[0] + (i-startItem)*itemIncrement[1];

				if ( UI_MouseOverSubItem(x, y, itemWidth, realItemHeight, l->generic.scrAlign) )
				{
					oldCurValue = l->curValue;
					l->curValue = i;
					s = ui_menu_move_sound;

					if (!l->generic.cvarNoSave)
						UI_MenuListView_SaveValue (l);
					else
						l->generic.valueChanged = UI_MenuListView_ValueChanged (l);

					if (oldCurValue != l->curValue) {	// new subitem
						ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 1;	// was 0
						if (l->generic.callback)
							l->generic.callback (l);
					}

					if ( (ui_mousecursor.buttonclicks[MOUSEBUTTON1] == 2) && l->generic.dblClkCallback )
						l->generic.dblClkCallback (l);

					foundSelected = true;
					break;
				}
			}
			startItem += visibleItems[1];
			if (foundSelected)
				break;
		}
	}
	return s;
}

void UI_MenuListView_Draw (menuListView_s *l)
{
//	menuFramework_s	*menu = l->generic.parent;
	int				i, j, x = 0, y = 0, lineSize, itemWidth, itemHeight, realItemHeight, boxWidth, boxHeight;
	int				startItem, visibleItems[2], itemIncrement[2];
	int				hc[3];
	int				hoverAlpha, copyLen;
	char			buf[64];
	qboolean		img_background = (l->background && strlen(l->background) > 0);
	qboolean		scrollX, itemPulse;

	// The player setup mwnu can change the imageNames and itemNames lists,
	// so these need to be checked to make sure they're not null.
	if (!l->imageNames)	return;
	if ((l->listViewType == LISTVIEW_TEXTIMAGE) && !l->itemNames)	return;

	UI_TextColorHighlight (Cvar_VariableInteger("alt_text_color"), &hc[0], &hc[1], &hc[2]);
	scrollX			= (l->scrollState.scrollType == SCROLL_X);
	hoverAlpha		= UI_MouseOverAlpha(&l->generic);
	lineSize		= l->itemTextSize + 2;
	itemWidth		= l->itemWidth;
	itemHeight		= l->itemHeight;
	realItemHeight	= itemHeight + ((l->listViewType == LISTVIEW_TEXTIMAGE) ? lineSize : 0);
	boxWidth		= l->generic.botRight[0] - l->generic.topLeft[0];
	boxHeight		= l->generic.botRight[1] - l->generic.topLeft[1];

	// name and header
	UI_MenuCommon_DrawItemName (&l->generic, -(2*RCOLUMN_OFFSET+l->border), 0, 0, -(l->border + MENU_LINE_SIZE), hoverAlpha);

	// border and background
	if (l->border > 0)
	{
		if (l->backColor[3] == 255 || img_background) // just fill whole area for border if not trans
			UI_DrawFill (l->generic.topLeft[0]-l->border, l->generic.topLeft[1]-l->border, boxWidth+(l->border*2), boxHeight+(l->border*2),
							l->generic.scrAlign, true, l->borderColor[0],l->borderColor[1],l->borderColor[2],l->borderColor[3]);
		else // have to do each side
			UI_DrawBorder ((float)l->generic.topLeft[0], (float)l->generic.topLeft[1], (float)boxWidth, (float)boxHeight,
							(float)l->border, l->generic.scrAlign, true, l->borderColor[0],l->borderColor[1],l->borderColor[2],l->borderColor[3]);
	}
	if (img_background)
		UI_DrawTiledPic (l->generic.topLeft[0], l->generic.topLeft[1], boxWidth, boxHeight, l->generic.scrAlign, true,
						(char *)l->background, (float)(l->backColor[3]/255.0f));
	else
		UI_DrawFill (l->generic.topLeft[0], l->generic.topLeft[1],
						boxWidth, boxHeight, l->generic.scrAlign, true, l->backColor[0],l->backColor[1],l->backColor[2],l->backColor[3]);

	// scrollbar
	UI_MenuScrollBar_Draw (&l->generic, &l->scrollState, l->generic.topLeft[0], l->generic.topLeft[1], boxWidth, boxHeight); 

	// items
	if (scrollX)
	{
		visibleItems[0] = l->items_x;
		visibleItems[1] = l->items_y;
		itemIncrement[0] = itemWidth + l->itemSpacing;
		itemIncrement[1] = realItemHeight + l->itemSpacing;
	}
	else // scrollY
	{
		visibleItems[0] = l->items_y;
		visibleItems[1] = l->items_x;
		itemIncrement[0] = realItemHeight + l->itemSpacing;
		itemIncrement[1] = itemWidth + l->itemSpacing;
	}

	startItem = l->scrollState.scrollPos*visibleItems[1];
	for (j=0; j<visibleItems[0]; j++)
	{
		if (scrollX)
			x = l->generic.topLeft[0] + j*itemIncrement[0];
		else
			y = l->generic.topLeft[1] + j*itemIncrement[0];
		for (i=startItem; i<l->numItems && i<(startItem+visibleItems[1]); i++)
		{
			if (scrollX)
				y = l->generic.topLeft[1] + (i-startItem)*itemIncrement[1];
			else
				x = l->generic.topLeft[0] + (i-startItem)*itemIncrement[1];

			itemPulse = UI_MouseOverSubItem(x, y, itemWidth, realItemHeight, l->generic.scrAlign);

			if (i == l->curValue)
				UI_DrawFill (x, y, itemWidth, realItemHeight, l->generic.scrAlign, false, hc[0], hc[1], hc[2], hoverAlpha);
			
			if (l->underImgColor[3] > 0)
				UI_DrawFill (x+l->itemPadding, y+l->itemPadding, itemWidth-l->itemPadding*2, itemHeight-l->itemPadding*2,
							l->generic.scrAlign, false, l->underImgColor[0], l->underImgColor[1], l->underImgColor[2], l->underImgColor[3]);
			if ( (l->generic.flags & QMF_COLORIMAGE) && (l->imageColors != NULL) )
				UI_DrawColoredPic (x+l->itemPadding, y+l->itemPadding, itemWidth-l->itemPadding*2, itemHeight-l->itemPadding*2,
							l->generic.scrAlign, false, l->imageColors[i], (char *)l->imageNames[i]);
			else
				UI_DrawPic (x+l->itemPadding, y+l->itemPadding, itemWidth-l->itemPadding*2, itemHeight-l->itemPadding*2,
							l->generic.scrAlign, false, (char *)l->imageNames[i], 1.0f);	// itemPulse ? (float)hoverAlpha*DIV255 : 1.0f);
			if ( l->listViewType == LISTVIEW_TEXTIMAGE && l->itemNames && l->itemNames[i] && strlen(l->itemNames[i]) )
			{
				copyLen = min((itemWidth-l->itemPadding*2)/l->itemTextSize, sizeof(buf));
				strncpy(buf, l->itemNames[i], copyLen);
				buf[min(copyLen, sizeof(buf)-1)] = '\0';
				UI_DrawMenuString (x+l->itemPadding, y+realItemHeight-l->itemPadding-(lineSize-1),
									l->itemTextSize, l->generic.scrAlign, buf, itemPulse ? hoverAlpha : 255, false, false);
			}
		}
		startItem += visibleItems[1];
	}
}

void UI_MenuListView_Setup (menuListView_s *l)
{
	menuFramework_s	*menu = l->generic.parent;
	int				i, numImages=0, numColors=0, numNames=0, lineSize, realItemHeight, boxWidth, boxHeight, scrollUnits;

	// count # of items
	for (i=0; l->imageNames[i]; i++);
		numImages = i;
	if (l->imageColors != NULL) {
		l->generic.flags |= QMF_COLORIMAGE;
		for (i=0; l->imageColors[i][3] != 0; i++);
			numColors = i;
	}
	if (l->itemNames != NULL) {
		for (i=0; l->itemNames[i]; i++);
			numNames = i;
	}

	l->numItems = numImages;

	// clamp curValue
	l->curValue = min(max(l->curValue, 0), l->numItems-1);

	// check if not enough item names
	if (l->listViewType == LISTVIEW_TEXTIMAGE && numNames < l->numItems)
		l->listViewType = LISTVIEW_IMAGE;
	// check if not enough image colors
	if ( (l->generic.flags & QMF_COLORIMAGE) && (numColors < numImages) )
		l->generic.flags &= ~QMF_COLORIMAGE;

	l->scrollState.scrollType = (l->scrollDir == 0) ? SCROLL_X : SCROLL_Y;
	l->items_x		= max(l->items_x, 1);
	l->items_y		= max(l->items_y, 1);
	l->itemWidth	= min(max(l->itemWidth, 16), 128);	// must be at least 16 px wide
	l->itemHeight	= min(max(l->itemHeight, 16), 128);	// must be at least 16 px tall
	l->itemSpacing	= max(l->itemSpacing, 0);
	l->itemPadding	= min(max(l->itemPadding, 0), min(l->itemWidth,l->itemHeight)/3);
	l->itemTextSize	= max (l->itemTextSize, 4);
	l->border		= max(l->border, 0);

	lineSize = l->itemTextSize + 2;
	realItemHeight = l->itemHeight + ((l->listViewType == LISTVIEW_TEXTIMAGE) ? lineSize : 0);
	boxWidth	= l->items_x * (l->itemWidth + l->itemSpacing)
						+ l->itemSpacing + ((l->scrollState.scrollType == SCROLL_Y)?LIST_SCROLLBAR_SIZE:0);
	boxHeight	= l->items_y * (realItemHeight + l->itemSpacing)
						+ l->itemSpacing + ((l->scrollState.scrollType == SCROLL_X)?LIST_SCROLLBAR_SIZE:0);

	// set min and max coords
	l->generic.topLeft[0] = menu->x + l->generic.x + RCOLUMN_OFFSET + l->border;
	l->generic.topLeft[1] = menu->y + l->generic.y;
	l->generic.botRight[0] = l->generic.topLeft[0] + boxWidth;
	l->generic.botRight[1] = l->generic.topLeft[1] + boxHeight;
	l->generic.dynamicWidth = 0;
	l->generic.dynamicHeight = 0;
	l->generic.isExtended = false;
	l->generic.valueChanged = false;

	if (l->scrollState.scrollType == SCROLL_X)
		scrollUnits = (l->numItems / l->items_y) + ((l->numItems % l->items_y > 0) ? 1 : 0);
	else // SCROLL_Y
		scrollUnits = (l->numItems / l->items_x) + ((l->numItems % l->items_x > 0) ? 1 : 0);

	l->scrollState.scrollEnabled = (l->scrollState.scrollType == SCROLL_X) ? (scrollUnits > l->items_x) : (scrollUnits > l->items_y);
	l->scrollState.scrollMin = 0;
	l->scrollState.scrollMax = max( ((l->scrollState.scrollType == SCROLL_X) ? (scrollUnits - l->items_x) : (scrollUnits - l->items_y)), 0);
	l->scrollState.scrollNumVisible = (l->scrollState.scrollType == SCROLL_X) ? l->items_x : l->items_y;

	if (l->scrollState.scrollType == SCROLL_X)
		UI_MenuScrollBar_SetPos (&l->scrollState, l->items_x, scrollUnits, l->curValue/l->items_y);
	else // SCROLL_Y
		UI_MenuScrollBar_SetPos (&l->scrollState, l->items_y, scrollUnits, l->curValue/l->items_x);

	l->scrollState.scrollTopLeft[0] = l->generic.topLeft[0] + ((l->scrollState.scrollType == SCROLL_Y) ? (boxWidth - LIST_SCROLLBAR_SIZE) : 0);
	l->scrollState.scrollTopLeft[1] = l->generic.topLeft[1] + ((l->scrollState.scrollType == SCROLL_X) ? (boxHeight - LIST_SCROLLBAR_SIZE) : 0);
	l->scrollState.scrollBotRight[0] = l->generic.botRight[0];
	l->scrollState.scrollBotRight[1] = l->generic.botRight[1];

	l->generic.flags |= QMF_MOUSEONLY;
}

//=========================================================

void UI_MenuTextScroll_Draw (menuTextScroll_s *t)
{
//	menuFramework_s	*menu = t->generic.parent;
	float			y, alpha;
	int				i, x, len, stringoffset;
	qboolean		bold;

	if (!t->initialized)
		return;

	if ( ((float)t->height - ((float)(cls.realtime - t->start_time)*t->time_scale)
		+ (float)(t->start_line * t->lineSize)) < 0 )
	{
		t->start_line++;
		if (!t->scrollText[t->start_line])
		{
			t->start_line = 0;
			t->start_time = cls.realtime;
		}
	}

	for (i=t->start_line, y=(float)t->generic.botRight[1] - ((float)(cls.realtime - t->start_time)*t->time_scale) + t->start_line * t->lineSize;
		t->scrollText[i] && y < t->generic.botRight[1]; y += (float)t->lineSize, i++)
	{
		stringoffset = 0;
		bold = false;

		if (y <= t->generic.topLeft[1]-t->generic.textSize)
			continue;
		if (y > t->generic.botRight[1])
			continue;

		if (t->scrollText[i][0] == '+')
		{
			bold = true;
			stringoffset = 1;
		}
		else
		{
			bold = false;
			stringoffset = 0;
		}

		if (y > (float)t->height*(7.0f/8.0f))
		{
			float y_test, h_test;
			y_test = y - t->height*(7.0f/8.0f);
			h_test = t->height/8;

			alpha = 1 - (y_test/h_test);

			alpha = max(min(alpha, 1), 0);
		}
		else if (y < (float)t->height/8)
		{
			float y_test, h_test;
			y_test = y;
			h_test = t->height/8;

			alpha = y_test/h_test;

			alpha = max(min(alpha, 1), 0);
		}
		else
			alpha = 1;

		len = (int)strlen(t->scrollText[i]) - stringLengthExtra(t->scrollText[i]);

		x = t->generic.topLeft[0] + (t->width - (len * t->generic.textSize) - (stringoffset * t->generic.textSize)) / 2
			+ stringoffset * t->generic.textSize;
		UI_DrawMenuString (x, (int)floor(y), t->generic.textSize, t->generic.scrAlign, t->scrollText[i], alpha*255, false, false);
	}
}

void UI_MenuTextScroll_UpdateCoords (menuTextScroll_s *t)
{
	menuFramework_s	*menu = t->generic.parent;

	t->generic.topLeft[0] = menu->x + t->generic.x;
	t->generic.topLeft[1] = menu->y + t->generic.y;
	t->generic.botRight[0] = t->generic.topLeft[0] + t->width;
	t->generic.botRight[1] = t->generic.topLeft[1] + t->height;
}

void UI_MenuTextScroll_Setup (menuTextScroll_s *t)
{
//	menuFramework_s	*menu = t->generic.parent;
	int				n, count;
	static char		*lineIndex[256];
	char			*p;

	t->initialized = false;

	// free this if reinitializing
	if (t->fileBuffer) {
		FS_FreeFile (t->fileBuffer);
		t->fileBuffer = NULL;
		t->scrollText = NULL;
	}

	if ( t->fileName && (strlen(t->fileName) > 0)
		&& (count = FS_LoadFile (t->fileName, &t->fileBuffer)) != -1 )
	{
		p = t->fileBuffer;
		for (n = 0; n < 255; n++)
		{
			lineIndex[n] = p;
			while (*p != '\r' && *p != '\n')
			{
				p++;
				if (--count == 0)
					break;
			}
			if (*p == '\r')
			{
				*p++ = 0;
				if (--count == 0)
					break;
			}
			*p++ = 0;
			if (--count == 0)
				break;
		}
		lineIndex[++n] = 0;
		t->scrollText = lineIndex;
		t->initialized = true;
	}
	else if (t->scrollText != NULL)
	{
		t->initialized = true;
	}

	t->generic.textSize = min(max(t->generic.textSize, 4), 32);			// text size must be between 4 and 32 px
	t->lineSize = min(max(t->lineSize, t->generic.textSize+1), t->generic.textSize*2);	// line spacing must be at least 1 px
	t->width = min(max(t->width, t->generic.textSize*4), SCREEN_WIDTH);	// width must be at least 4 columns 
	t->height = min(max(t->height, t->lineSize*8), SCREEN_HEIGHT);		// height must be at least 8 lines
	t->time_scale = min(max(t->time_scale, 0.01f), 0.25f);				// scroll speed must be between 100 and 4 px/s
	t->start_time = cls.realtime;
	t->start_line = 0;

	// set min and max coords
	UI_MenuTextScroll_UpdateCoords (t);
	t->generic.dynamicWidth = 0;
	t->generic.dynamicHeight = 0;
	t->generic.isExtended = false;
	t->generic.valueChanged = false;

	t->generic.flags |= QMF_NOINTERACTION;
}

//=========================================================


void UI_MenuModelView_Reregister (menuModelView_s *m)
{
	int				i;
	char			scratch[MAX_QPATH];

	for (i=0; i<MODELVIEW_MAX_MODELS; i++)
	{
		if (!m->modelValid[i] || !m->model[i])
			continue;

		Com_sprintf( scratch, sizeof(scratch),  m->modelName[i]);
		m->model[i] = R_RegisterModel(scratch);
		if (m->skin[i]) {
			Com_sprintf( scratch, sizeof(scratch),  m->skinName[i]);
			m->skin[i] = R_RegisterSkin (scratch);
		}
	}
}


void UI_MenuModelView_Draw (menuModelView_s *m)
{
	menuFramework_s	*menu = m->generic.parent;
	int				i, j, entnum;
	refdef_t		refdef;
	float			rx, ry, rw, rh;
	entity_t		entity[MODELVIEW_MAX_MODELS], *ent;
	qboolean		reRegister;
	char			scratch[MAX_QPATH];

	if ( R_RegistrationIsActive() )	// don't draw during map load, as forced re-registration can clobber BSP loading
		return;

	memset(&refdef, 0, sizeof(refdef));

	rx = m->generic.topLeft[0];		ry = m->generic.topLeft[1];
	rw = m->width;					rh = m->height;
	SCR_ScaleCoords (&rx, &ry, &rw, &rh, m->generic.scrAlign);
	refdef.x = rx;			refdef.y = ry;
	refdef.width = rw;		refdef.height = rh;
	refdef.fov_x = m->fov;
	refdef.fov_y = CalcFov (refdef.fov_x, refdef.width, refdef.height);
	refdef.time = cls.realtime*0.001;
	refdef.areabits = 0;
	refdef.lightstyles = 0;
	refdef.rdflags = RDF_NOWORLDMODEL;
	refdef.num_entities = 0;
	refdef.entities = entity;
	entnum = 0;

	for (i=0; i<MODELVIEW_MAX_MODELS; i++)
	{
		if (!m->modelValid[i] || !m->model[i])
			continue;

		ent = &entity[entnum];
		memset (&entity[entnum], 0, sizeof(entity[entnum]));

		// model pointer may become invalid after a vid_restart
		reRegister = ( !R_ModelIsValid(m->model[i]) );
		if (reRegister) {
			Com_sprintf (scratch, sizeof(scratch),  m->modelName[i]);
			m->model[i] = R_RegisterModel(scratch);
		}
		ent->model = m->model[i];
		if (m->skin[i]) {
			if (reRegister) {
				Com_sprintf (scratch, sizeof(scratch),  m->skinName[i]);
				m->skin[i] = R_RegisterSkin (scratch);
			}
			ent->skin = m->skin[i];
		}

		ent->flags = m->entFlags[i];
		VectorCopy (m->modelOrigin[i], ent->origin);
		VectorCopy (ent->origin, ent->oldorigin);
		ent->frame = m->modelFrame[i] + ((int)(cl.time*0.01f) % (m->modelFrameNumbers[i]+1)); // m->modelFrameTime[i]
		ent->backlerp = 0.0f;
		for (j=0; j<3; j++)
			ent->angles[j] = m->modelBaseAngles[i][j] + cl.time*m->modelRotation[i][j];
		if (m->isMirrored) {
			ent->flags |= RF_MIRRORMODEL;
			ent->angles[1] = 360 - ent->angles[1];
		}

		refdef.num_entities++;
		entnum++;
	}

	if (refdef.num_entities > 0)
		R_RenderFrame (&refdef);
}

void UI_MenuModelView_UpdateCoords (menuModelView_s *m)
{
	menuFramework_s	*menu = m->generic.parent;

	m->generic.topLeft[0] = menu->x + m->generic.x;
	m->generic.topLeft[1] = menu->y + m->generic.y;
	m->generic.botRight[0] = m->generic.topLeft[0] + m->width;
	m->generic.botRight[1] = m->generic.topLeft[1] + m->height;
}

void UI_MenuModelView_Setup (menuModelView_s *m)
{
//	menuFramework_s	*menu = m->generic.parent;
	int				i, j;
	char			scratch[MAX_QPATH];

	m->num_entities = 0;
	for (i=0; i<MODELVIEW_MAX_MODELS; i++) {
		m->modelValid[i] = false;
		m->model[i] = NULL;	m->skin[i] = NULL;
	}
	m->width	= min(max(m->width, 16), SCREEN_WIDTH);		// width must be at least 16 px
	m->height	= min(max(m->height, 16), SCREEN_HEIGHT);	// height must be at least 16 px
	m->fov		= min(max(m->fov, 10), 110);				// fov must be reasonble
	for (i=0; i<MODELVIEW_MAX_MODELS; i++)
	{
		if (!m->modelName[i] || !strlen(m->modelName[i]))
			continue;

		Com_sprintf( scratch, sizeof(scratch),  m->modelName[i]);
		if ( !(m->model[i] = R_RegisterModel(scratch)) )
			continue;

		if (m->skinName[i] && strlen(m->skinName[i])) {
			Com_sprintf( scratch, sizeof(scratch),  m->skinName[i]);
			m->skin[i] = R_RegisterSkin (scratch);
		}
		for (j=0; j<3; j++) {
			m->modelOrigin[i][j]		= min(max(m->modelOrigin[i][j], -1024), 1024);
			m->modelBaseAngles[i][j]	= min(max(m->modelBaseAngles[i][j], 0), 360);
			m->modelRotation[i][j]		= min(max(m->modelRotation[i][j], -1.0f), 1.0f);
		}
		m->modelFrame[i]		= max(m->modelFrame[i], 0);			// catch negative frames
		m->modelFrameNumbers[i]	= max(m->modelFrameNumbers[i], 0);	// catch negative frames
	//	m->modelFrameTime[i]	= min(max(m->modelFrameTime[i], 0.005f), 0.04f);	// must be between 5 and 40 fps
		m->modelValid[i] = true;
		m->num_entities++;
	}

	// set min and max coords
	UI_MenuModelView_UpdateCoords (m);
	m->generic.dynamicWidth = 0;
	m->generic.dynamicHeight = 0;
	m->generic.isExtended = false;
	m->generic.valueChanged = false;

	m->generic.flags |= QMF_NOINTERACTION;
}

//=========================================================


/*
==========================
UI_ReregisterMenuItem
Just refreshes anything registered
==========================
*/
void UI_ReregisterMenuItem (void *item)
{
	switch ( ((menuCommon_s *)item)->type )
	{
	case MTYPE_MODELVIEW:
		UI_MenuModelView_Reregister ((menuModelView_s *)item);
		break;
	default:
		break;
	}
}


/*
==========================
UI_UpdateMenuItemCoords
Just updates coords for display items
==========================
*/
void UI_UpdateMenuItemCoords (void *item)
{
	switch ( ((menuCommon_s *)item)->type )
	{
	case MTYPE_IMAGE:
		UI_MenuImage_UpdateCoords ((menuImage_s *)item);
		break;
	case MTYPE_BUTTON:
		UI_MenuButton_UpdateCoords ((menuButton_s *)item);
		break;
	case MTYPE_RECTANGLE:
		UI_MenuRectangle_UpdateCoords ((menuRectangle_s *)item);
		break;
	case MTYPE_TEXTSCROLL:
		UI_MenuTextScroll_UpdateCoords ((menuTextScroll_s *)item);
		break;
	case MTYPE_MODELVIEW:
		UI_MenuModelView_UpdateCoords ((menuModelView_s *)item);
		break;
	default:
		break;
	}
}


/*
==========================
UI_ItemCanBeCursorItem
Checks if an item is of a
valid type to be a cursor item.
==========================
*/
qboolean UI_ItemCanBeCursorItem (void *item)
{
	if (!item)	return false;
	
	switch ( ((menuCommon_s *)item)->type )
	{
	case MTYPE_IMAGE:
	case MTYPE_BUTTON:
	case MTYPE_RECTANGLE:
	case MTYPE_TEXTSCROLL:
	case MTYPE_MODELVIEW:
		return true;
	default:
		return false;
	}
//	return false;
}


/*
==========================
UI_ItemIsValidCursorPosition
Checks if an item can be used
as a cursor position.
==========================
*/
qboolean UI_ItemIsValidCursorPosition (void *item)
{
	if (!item)	return false;
	
	if ( (((menuCommon_s *)item)->flags & QMF_NOINTERACTION) || (((menuCommon_s *)item)->flags & QMF_MOUSEONLY) )
		return false;

	if ( ((menuCommon_s *)item)->isHidden )
		return false;

	switch ( ((menuCommon_s *)item)->type )
	{
	case MTYPE_LABEL:
	case MTYPE_IMAGE:
	case MTYPE_RECTANGLE:
	case MTYPE_TEXTSCROLL:
	case MTYPE_MODELVIEW:
		return false;
	default:
		return true;
	}
//	return true;
}


/*
==========================
UI_ItemHasMouseBounds
Checks if an item is mouse-interactive.
==========================
*/
qboolean UI_ItemHasMouseBounds (void *item)
{
	if (!item)	return false;

	if (((menuCommon_s *)item)->flags & QMF_NOINTERACTION)
		return false;

	switch ( ((menuCommon_s *)item)->type )
	{
	case MTYPE_LABEL:
	case MTYPE_IMAGE:
	case MTYPE_RECTANGLE:
	case MTYPE_TEXTSCROLL:
	case MTYPE_MODELVIEW:
		return false;
	default:
		return true;
	}
//	return true;
}


/*
==========================
UI_GetMenuItemValue
Retruns string value for menu item from linked cvar.
==========================
*/
char *UI_GetMenuItemValue (void *item)
{
	if (!item)	return "";

	switch ( ((menuCommon_s *)item)->type )
	{
	case MTYPE_FIELD:
		return UI_MenuField_GetValue ((menuField_s *)item);
		break;
	case MTYPE_SLIDER:
		return UI_MenuSlider_GetValue ((menuSlider_s *)item);
		break;
	case MTYPE_PICKER:
		return UI_MenuPicker_GetValue ((menuPicker_s *)item);
		break;
	case MTYPE_CHECKBOX:
		return UI_MenuCheckBox_GetValue ((menuCheckBox_s *)item);
		break;
	case MTYPE_LISTBOX:
		return UI_MenuListBox_GetValue ((menuListBox_s *)item);
		break;
	case MTYPE_COMBOBOX:
		return UI_MenuComboBox_GetValue ((menuComboBox_s *)item);
		break;
	case MTYPE_LISTVIEW:
		return UI_MenuListView_GetValue ((menuListView_s *)item);
		break;
	default:
		return "";
	}
//	return "";
}


/*
==========================
UI_SetMenuItemValue
Loads value for menu item from linked cvar.
==========================
*/
void UI_SetMenuItemValue (void *item)
{
	if (!item)	return;

	switch ( ((menuCommon_s *)item)->type )
	{
	case MTYPE_FIELD:
		UI_MenuField_SetValue ((menuField_s *)item);
		break;
	case MTYPE_SLIDER:
		UI_MenuSlider_SetValue ((menuSlider_s *)item);
		break;
	case MTYPE_PICKER:
		UI_MenuPicker_SetValue ((menuPicker_s *)item);
		break;
	case MTYPE_CHECKBOX:
		UI_MenuCheckBox_SetValue ((menuCheckBox_s *)item);
		break;
	case MTYPE_LISTBOX:
		UI_MenuListBox_SetValue ((menuListBox_s *)item);
		break;
	case MTYPE_COMBOBOX:
		UI_MenuComboBox_SetValue ((menuComboBox_s *)item);
		break;
	case MTYPE_LISTVIEW:
		UI_MenuListView_SetValue ((menuListView_s *)item);
		break;
	default:
		break;
	}
}


/*
==========================
UI_SaveMenuItemValue
Loads value for menu item from linked cvar
==========================
*/
void UI_SaveMenuItemValue (void *item)
{
	if (!item)	return;

	switch ( ((menuCommon_s *)item)->type )
	{
	case MTYPE_FIELD:
		UI_MenuField_SaveValue ((menuField_s *)item);
		break;
	case MTYPE_SLIDER:
		UI_MenuSlider_SaveValue ((menuSlider_s *)item);
		break;
	case MTYPE_PICKER:
		UI_MenuPicker_SaveValue ((menuPicker_s *)item);
		break;
	case MTYPE_CHECKBOX:
		UI_MenuCheckBox_SaveValue ((menuCheckBox_s *)item);
		break;
	case MTYPE_LISTBOX:
		UI_MenuListBox_SaveValue ((menuListBox_s *)item);
		break;
	case MTYPE_COMBOBOX:
		UI_MenuComboBox_SaveValue ((menuComboBox_s *)item);
		break;
	case MTYPE_LISTVIEW:
		UI_MenuListView_SaveValue ((menuListView_s *)item);
		break;
	default:
		break;
	}
}


/*
==========================
UI_DrawMenuItem
Calls draw functions for each item type
==========================
*/
void UI_DrawMenuItem (void *item)
{
	if (!item)	return;

	// skip hidden items
	if ( ((menuCommon_s *)item)->isHidden )
		return;

	switch ( ((menuCommon_s *)item)->type )
	{
	case MTYPE_ACTION:
		UI_MenuAction_Draw ((menuAction_s *)item);
		break;
	case MTYPE_KEYBIND:
		UI_MenuKeyBind_Draw ((menuKeyBind_s *)item);
		break;
	case MTYPE_KEYBINDLIST:
		UI_MenuKeyBindList_Draw ((menuKeyBindList_s *)item);
		break;
	case MTYPE_FIELD:
		UI_MenuField_Draw ((menuField_s *)item);
		break;
	case MTYPE_SLIDER:
		UI_MenuSlider_Draw ((menuSlider_s *)item);
		break;
	case MTYPE_PICKER:
		UI_MenuPicker_Draw ((menuPicker_s *)item);
		break;
	case MTYPE_CHECKBOX:
		UI_MenuCheckBox_Draw ((menuCheckBox_s *)item);
		break;
	case MTYPE_LABEL:
		UI_MenuLabel_Draw ((menuLabel_s *)item);
		break;
	case MTYPE_IMAGE:
		UI_MenuImage_Draw ((menuImage_s *)item);
		break;
	case MTYPE_BUTTON:
		UI_MenuButton_Draw ((menuButton_s *)item);
		break;
	case MTYPE_RECTANGLE:
		UI_MenuRectangle_Draw ((menuRectangle_s *)item);
		break;
	case MTYPE_LISTBOX:
		UI_MenuListBox_Draw ((menuListBox_s *)item);
		break;
	case MTYPE_COMBOBOX:
		UI_MenuComboBox_Draw ((menuComboBox_s *)item);
		break;
	case MTYPE_LISTVIEW:
		UI_MenuListView_Draw ((menuListView_s *)item);
		break;
	case MTYPE_TEXTSCROLL:
		UI_MenuTextScroll_Draw ((menuTextScroll_s *)item);
		break;
	case MTYPE_MODELVIEW:
		UI_MenuModelView_Draw ((menuModelView_s *)item);
		break;
	default:
		break;
	}
}


/*
==========================
UI_DrawMenuItemExtension
Calls draw functions for each item type
==========================
*/
void UI_DrawMenuItemExtension (void *item)
{
	if (!item)	return;

	if ( !((menuCommon_s *)item)->isExtended )
		return;

	switch ( ((menuCommon_s *)item)->type )
	{
	case MTYPE_COMBOBOX:
		UI_MenuComboBox_DrawExtension ((menuComboBox_s *)item);
		break;
	default:
		break;
	}
}


/*
==========================
UI_SetMenuItemDynamicSize
Calls SetDynamicSize functions for each item type
==========================
*/
void UI_SetMenuItemDynamicSize (void *item)
{
	if (!item)	return;

	switch ( ((menuCommon_s *)item)->type )
	{
	case MTYPE_KEYBIND:
		UI_MenuKeyBind_SetDynamicSize ((menuKeyBind_s *)item);
		break;
	case MTYPE_PICKER:
		UI_MenuPicker_SetDynamicSize ((menuPicker_s *)item);
		break;
	case MTYPE_COMBOBOX:
		UI_MenuComboBox_SetDynamicSize ((menuComboBox_s *)item);
		break;
	default:
		break;
	}
}


/*
==========================
UI_GetItemMouseoverType
Returns mouseover type for a menu item
==========================
*/
int UI_GetItemMouseoverType (void *item)
{
	if (!item)
		return MENUITEM_NONE;

	switch ( ((menuCommon_s *)item)->type )
	{
		case MTYPE_ACTION:
		case MTYPE_KEYBIND:
			return MENUITEM_ACTION;
		case MTYPE_KEYBINDLIST:
			return MENUITEM_KEYBINDLIST;
		case MTYPE_SLIDER:
			return MENUITEM_SLIDER;
		case MTYPE_PICKER:
			return MENUITEM_PICKER;
		case MTYPE_CHECKBOX:
			return MENUITEM_CHECKBOX;
		case MTYPE_FIELD:
			return MENUITEM_TEXT;
		case MTYPE_BUTTON:
			return MENUITEM_BUTTON;
		case MTYPE_LISTBOX:
			return MENUITEM_LISTBOX;
		case MTYPE_COMBOBOX:
			return MENUITEM_COMBOBOX;
		case MTYPE_LISTVIEW:
			return MENUITEM_LISTVIEW;
		default:
			return MENUITEM_NONE;
	}
//	return MENUITEM_NONE;
}


/*
=================
UI_ClickMenuItem
=================
*/
char *UI_ClickMenuItem (menuCommon_s *item, qboolean mouse2)
{
	char	*s = NULL;

	if ( UI_MouseOverScrollKnob(item) && !mouse2 )
	{
	//	Com_Printf ("Dragging scroll bar\n");
		UI_ClickItemScrollBar (item);
		return ui_menu_drag_sound;
	}

	if (item)
	{
		switch (item->type)
		{
		case MTYPE_ACTION:
			s = UI_MenuAction_Click ( (menuAction_s *)item, mouse2 );
			break;
		case MTYPE_KEYBIND:
			s = UI_MenuKeyBind_Click ( (menuKeyBind_s *)item, mouse2 );
			break;
		case MTYPE_KEYBINDLIST:
			s = UI_MenuKeyBindList_Click ( (menuKeyBindList_s *)item, mouse2 );
			break;
		case MTYPE_SLIDER:
			s = UI_MenuSlider_Click ( (menuSlider_s *)item, mouse2 );
			break;
		case MTYPE_PICKER:
			s = UI_MenuPicker_Click ( (menuPicker_s *)item, mouse2 );
			break;
		case MTYPE_FIELD:
			s = UI_MenuField_Click ( (menuField_s *)item, mouse2 );
			break;
		case MTYPE_CHECKBOX:
			s = UI_MenuCheckBox_Click ( (menuCheckBox_s *)item, mouse2 );
			break;
		case MTYPE_BUTTON:
			s = UI_MenuButton_Click ( (menuButton_s *)item, mouse2 );
			break;
		case MTYPE_LISTBOX:
			s = UI_MenuListBox_Click ( (menuListBox_s *)item, mouse2 );
			break;
		case MTYPE_COMBOBOX:
			s = UI_MenuComboBox_Click ( (menuComboBox_s *)item, mouse2 );
			break;
		case MTYPE_LISTVIEW:
			s = UI_MenuListView_Click ( (menuListView_s *)item, mouse2 );
			break;
		default:
			s = ui_menu_null_sound;
			break;
		}
		if (!s)
			s = ui_menu_null_sound;
		return s;
	}
	return ui_menu_null_sound;
}


/*
=================
UI_SelectMenuItem
=================
*/
qboolean UI_SelectMenuItem (menuFramework_s *s)
{
	menuCommon_s *item=NULL;

	if (!s)	return false;

	item = (menuCommon_s *)UI_ItemAtMenuCursor(s);

	if (item)
	{
		switch (item->type)
		{
		case MTYPE_ACTION:
			UI_MenuAction_DoEnter ((menuAction_s *)item);
			return true;
		case MTYPE_KEYBIND:
			UI_MenuKeyBind_DoEnter ((menuKeyBind_s *)item);
			return true;
		case MTYPE_KEYBINDLIST:
			UI_MenuKeyBindList_DoEnter ( (menuKeyBindList_s *)item );
			return true;
		case MTYPE_FIELD:
			return UI_MenuField_DoEnter ((menuField_s *)item) ;
		case MTYPE_PICKER:
			UI_MenuPicker_DoEnter ((menuPicker_s *)item);
			return false;
		case MTYPE_BUTTON:
			UI_MenuButton_DoEnter ((menuButton_s *)item);
			return true;
		case MTYPE_CHECKBOX:
			UI_MenuCheckBox_DoEnter ( (menuCheckBox_s *)item );
			return false;
		case MTYPE_LISTBOX:
			UI_MenuListBox_DoEnter ( (menuListBox_s *)item );
			return true;
		}
	}
	return false;
}


/*
=================
UI_SlideMenuItem
=================
*/
char *UI_SlideMenuItem (menuFramework_s *s, int dir)
{
	menuCommon_s *item=NULL;

	if (!s)	return ui_menu_null_sound;

	item = (menuCommon_s *) UI_ItemAtMenuCursor(s);

	if (item)
	{
		switch (item->type)
		{
		case MTYPE_KEYBINDLIST:
			UI_MenuKeyBindList_DoSlide ((menuKeyBindList_s *)item, dir);
			return ui_menu_null_sound;
		case MTYPE_SLIDER:
			UI_MenuSlider_DoSlide ((menuSlider_s *) item, dir);
			return ui_menu_move_sound;
		case MTYPE_PICKER:
			UI_MenuPicker_DoSlide ((menuPicker_s *) item, dir);
			return ui_menu_move_sound;
		case MTYPE_CHECKBOX:
			UI_MenuCheckBox_DoSlide ((menuCheckBox_s *)item, dir);
			return ui_menu_move_sound;
		case MTYPE_LISTBOX:
			UI_MenuListBox_DoSlide ((menuListBox_s *)item, dir);
			return ui_menu_move_sound;
		case MTYPE_COMBOBOX:
			UI_MenuComboBox_DoSlide ((menuComboBox_s *)item, dir);
			return ui_menu_move_sound;
		}
	}
	return ui_menu_null_sound;
}


/*
=================
UI_ScrollMenuItem
=================
*/
qboolean UI_ScrollMenuItem (menuFramework_s *s, int dir)
{
	menuCommon_s *item;
	
	if (!ui_mousecursor.menuitem)
		return false;

	item = (menuCommon_s *)ui_mousecursor.menuitem;
	if (!UI_ItemHasScrollBar(item))
		return false;

	switch (item->type)
	{
	case MTYPE_KEYBINDLIST:
		return UI_MenuScrollBar_Increment (&((menuKeyBindList_s *)item)->scrollState, dir);
	case MTYPE_LISTBOX:
		return UI_MenuScrollBar_Increment (&((menuListBox_s *)item)->scrollState, dir);
	case MTYPE_COMBOBOX:
		return UI_MenuScrollBar_Increment (&((menuComboBox_s *)item)->scrollState, dir);
	case MTYPE_LISTVIEW:
		return UI_MenuScrollBar_Increment (&((menuListView_s *)item)->scrollState, dir);
	default:
		return false;
	}
}


/*
==========================
UI_InitMenuItem
Calls setup functions for menu item type
==========================
*/
void UI_InitMenuItem (void *item)
{
	// only some items can be a cursor item
	if ( !UI_ItemCanBeCursorItem(item) )
		((menuCommon_s *)item)->isCursorItem = false;

	// default alignment is center
	if ( ((menuCommon_s *)item)->scrAlign == ALIGN_UNSET )
		((menuCommon_s *)item)->scrAlign = ALIGN_CENTER;

	// clamp text size
	if (!((menuCommon_s *)item)->textSize)
		((menuCommon_s *)item)->textSize = MENU_FONT_SIZE;
	((menuCommon_s *)item)->textSize = min(max(((menuCommon_s *)item)->textSize, 4), 32);

	switch ( ((menuCommon_s *)item)->type )
	{
	case MTYPE_ACTION:
		UI_MenuAction_Setup ((menuAction_s *)item);
		break;
	case MTYPE_KEYBIND:
		UI_MenuKeyBind_Setup ((menuKeyBind_s *)item);
		break;
	case MTYPE_KEYBINDLIST:
		UI_MenuKeyBindList_Setup ((menuKeyBindList_s *)item);
		break;
	case MTYPE_SLIDER:
		UI_MenuSlider_Setup ((menuSlider_s *)item);
		break;
	case MTYPE_PICKER:
		UI_MenuPicker_Setup ((menuPicker_s *)item);
		break;
	case MTYPE_CHECKBOX:
		UI_MenuCheckBox_Setup ((menuCheckBox_s *)item);
		break;
	case MTYPE_LABEL:
		UI_MenuLabel_Setup ((menuLabel_s *)item);
		break;
	case MTYPE_FIELD:
		UI_MenuField_Setup ((menuField_s *)item);
		break;
	case MTYPE_IMAGE:
		UI_MenuImage_Setup ((menuImage_s *)item);
		break;
	case MTYPE_BUTTON:
		UI_MenuButton_Setup ((menuButton_s *)item);
		break;
	case MTYPE_RECTANGLE:
		UI_MenuRectangle_Setup ((menuRectangle_s *)item);
		break;
	case MTYPE_LISTBOX:
		UI_MenuListBox_Setup ((menuListBox_s *)item);
		break;
	case MTYPE_COMBOBOX:
		UI_MenuComboBox_Setup ((menuComboBox_s *)item);
		break;
	case MTYPE_LISTVIEW:
		UI_MenuListView_Setup ((menuListView_s *)item);
		break;
	case MTYPE_TEXTSCROLL:
		UI_MenuTextScroll_Setup ((menuTextScroll_s *)item);
		break;
	case MTYPE_MODELVIEW:
		UI_MenuModelView_Setup ((menuModelView_s *)item);
		break;
	default:
		break;
	}
}
