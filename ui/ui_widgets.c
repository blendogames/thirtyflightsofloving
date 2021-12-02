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


#include <string.h>
#include <ctype.h>

#include "../client/client.h"
#include "ui_local.h"

static	void	MenuAction_DoEnter (menuaction_s *a);
static	void	MenuAction_Draw (menuaction_s *a);
static	void	Menulist_DoEnter (menulist_s *l);
static	void	MenuList_Draw (menulist_s *l);
static	void	MenuSeparator_Draw (menuseparator_s *s);
static	void	MenuSlider_DoSlide (menuslider_s *s, int dir);
static	void	MenuSlider_Draw (menuslider_s *s);
static	void	MenuSpinControl_DoEnter (menulist_s *s);
static	void	MenuSpinControl_Draw (menulist_s *s);
static	void	MenuSpinControl_DoSlide (menulist_s *s, int dir);

#ifndef NOTTHIRTYFLIGHTS
static void	checkbox_DoEnter( menulist_s *s );
static void	checkbox_Draw( menulist_s *s );
static void	checkbox_DoSlide( menulist_s *s, int dir );
#endif

#define RCOLUMN_OFFSET  MENU_FONT_SIZE*2	// was 16
#define LCOLUMN_OFFSET -MENU_FONT_SIZE*2	// was -16

vec4_t		stCoord_arrow_left = {0.0, 0.0, 0.25, 0.25};
vec4_t		stCoord_arrow_right = {0.25, 0.0, 0.5, 0.25};
vec4_t		stCoord_arrow_up = {0.5, 0.0, 0.75, 0.25};
vec4_t		stCoord_arrow_down = {0.75, 0.0, 1, 0.25};
vec4_t		stCoord_scrollKnob_h = {0.0, 0.75, 0.25, 1.0};
vec4_t		stCoord_scrollKnob_v = {0.25, 0.75, 0.5, 1.0};
vec4_t		stCoord_field_left = {0.0, 0.0, 0.25, 1.0};
vec4_t		stCoord_field_center = {0.25, 0.0, 0.5, 1.0};
vec4_t		stCoord_field_right = {0.5, 0.0, 0.75, 1.0};
vec4_t		stCoord_slider_left = {0.0, 0.0, 0.125, 1.0};
vec4_t		stCoord_slider_center = {0.125, 0.0, 0.375, 1.0};
vec4_t		stCoord_slider_right = {0.375, 0.0, 0.5, 1.0};
vec4_t		stCoord_slider_knob = {0.5, 0.0, 0.625, 1.0};

extern viddef_t viddef;

#define VID_WIDTH viddef.width
#define VID_HEIGHT viddef.height

//======================================================

void MenuAction_DoEnter (menuaction_s *a)
{
	if (!a) return;

	if (a->generic.callback)
		a->generic.callback(a);
}

void MenuAction_Draw (menuaction_s *a)
{
	int		alpha;

	if (!a) return;

	alpha = UI_MouseOverAlpha(&a->generic);

	if (a->generic.flags & QMF_LEFT_JUSTIFY)
	{
		if (a->generic.flags & QMF_GRAYED)
			UI_DrawStringDark (a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET,
								a->generic.y + a->generic.parent->y, a->generic.textSize, ALIGN_CENTER, a->generic.name, FONT_UI, alpha);
		else
			UI_DrawString (a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET,
							a->generic.y + a->generic.parent->y, a->generic.textSize, ALIGN_CENTER, a->generic.name, FONT_UI, alpha);
	}
	else
	{
		if (a->generic.flags & QMF_GRAYED)
			UI_DrawStringR2LDark (a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET,
									a->generic.y + a->generic.parent->y, a->generic.textSize, ALIGN_CENTER, a->generic.name, FONT_UI, alpha);
		else
			UI_DrawStringR2L (a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET,
								a->generic.y + a->generic.parent->y, a->generic.textSize, ALIGN_CENTER, a->generic.name, FONT_UI, alpha);
	}
	if (a->generic.ownerdraw)
		a->generic.ownerdraw(a);
}

//=========================================================

qboolean MenuField_DoEnter (menufield_s *f)
{
	if (!f) return false;

	if (f->generic.callback)
	{
		f->generic.callback(f);
		return true;
	}
	return false;
}

void MenuField_Draw (menufield_s *f)
{
	int i, alpha, xtra;
	char tempbuffer[128]="";
	int offset;

	if (!f) return;

	alpha = UI_MouseOverAlpha(&f->generic);

	if (f->generic.name)
		UI_DrawStringR2LDark (f->generic.x + f->generic.parent->x + LCOLUMN_OFFSET,
								f->generic.y + f->generic.parent->y, f->generic.textSize, ALIGN_CENTER, f->generic.name, FONT_UI, 255);

	if (xtra = stringLengthExtra(f->buffer))
	{
		strncpy( tempbuffer, f->buffer + f->visible_offset, f->visible_length );
		offset = (int)strlen(tempbuffer) - xtra;

		if (offset > f->visible_length)
		{
			f->visible_offset = offset - f->visible_length;
			strncpy( tempbuffer, f->buffer + f->visible_offset - xtra, f->visible_length + xtra );
			offset = f->visible_offset;
		}
	}
	else
	{
		strncpy( tempbuffer, f->buffer + f->visible_offset, f->visible_length );
		offset = (int)strlen(tempbuffer);
	}

	if (ui_new_textfield->integer)
	{
		UI_DrawPicST (f->generic.x + f->generic.parent->x + RCOLUMN_OFFSET,
						f->generic.y + f->generic.parent->y - 4, f->generic.textSize, f->generic.textSize*2, stCoord_field_left,
						ALIGN_CENTER, true, color_identity, UI_FIELD_PIC);
		UI_DrawPicST (f->generic.x + f->generic.parent->x + (1+f->visible_length)*f->generic.textSize + RCOLUMN_OFFSET,
						f->generic.y + f->generic.parent->y - 4, f->generic.textSize, f->generic.textSize*2, stCoord_field_right,
						ALIGN_CENTER, true, color_identity, UI_FIELD_PIC);
	}
	else
	{
		UI_DrawChar (f->generic.x + f->generic.parent->x + RCOLUMN_OFFSET,
					f->generic.y + f->generic.parent->y - 4, f->generic.textSize, ALIGN_CENTER, 18, 255, 255, 255, 255, false, false);
		UI_DrawChar (f->generic.x + f->generic.parent->x + RCOLUMN_OFFSET,
					f->generic.y + f->generic.parent->y + 4, f->generic.textSize, ALIGN_CENTER, 24, 255, 255, 255, 255, false, false);
		UI_DrawChar (f->generic.x + f->generic.parent->x + (1+f->visible_length)*f->generic.textSize + RCOLUMN_OFFSET,
					f->generic.y + f->generic.parent->y - 4, f->generic.textSize, ALIGN_CENTER, 20, 255, 255, 255, 255, false, false);
		UI_DrawChar (f->generic.x + f->generic.parent->x + (1+f->visible_length)*f->generic.textSize + RCOLUMN_OFFSET,
					f->generic.y + f->generic.parent->y + 4, f->generic.textSize, ALIGN_CENTER, 26, 255, 255, 255, 255, false, false);
	}

	for (i = 0; i < f->visible_length; i++)
	{
		if (ui_new_textfield->integer) {
			UI_DrawPicST (f->generic.x + f->generic.parent->x + (1+i)*f->generic.textSize + RCOLUMN_OFFSET,
							f->generic.y + f->generic.parent->y - 4, f->generic.textSize, f->generic.textSize*2, stCoord_field_center,
							ALIGN_CENTER, true, color_identity, UI_FIELD_PIC);
		}
		else {
			UI_DrawChar (f->generic.x + f->generic.parent->x + (1+i)*f->generic.textSize + RCOLUMN_OFFSET,
						f->generic.y + f->generic.parent->y - 4, f->generic.textSize, ALIGN_CENTER, 19, 255, 255, 255, 255, false, false);
			UI_DrawChar (f->generic.x + f->generic.parent->x + (1+i)*f->generic.textSize + RCOLUMN_OFFSET,
						f->generic.y + f->generic.parent->y + 4, f->generic.textSize, ALIGN_CENTER, 25, 255, 255, 255, 255, false, (i==(f->visible_length-1)));
		}
	}

	// add cursor thingie
	if ( (UI_ItemAtMenuCursor(f->generic.parent) == f)  && ((int)(Sys_Milliseconds()/250))&1 )
		Com_sprintf (tempbuffer, sizeof(tempbuffer),	"%s%c", tempbuffer, 11);

	UI_DrawString (f->generic.x + f->generic.parent->x + f->generic.textSize*3,
					f->generic.y + f->generic.parent->y, f->generic.textSize, ALIGN_CENTER, tempbuffer, FONT_UI, alpha);
}

qboolean UI_MenuField_Key (menufield_s *f, int key)
{
	extern int keydown[];

	if (!f) return false;

	switch ( key )
	{
	case K_KP_SLASH:
		key = '/';
		break;
	case K_KP_MINUS:
		key = '-';
		break;
	case K_KP_PLUS:
		key = '+';
		break;
	case K_KP_HOME:
		key = '7';
		break;
	case K_KP_UPARROW:
		key = '8';
		break;
	case K_KP_PGUP:
		key = '9';
		break;
	case K_KP_LEFTARROW:
		key = '4';
		break;
	case K_KP_5:
		key = '5';
		break;
	case K_KP_RIGHTARROW:
		key = '6';
		break;
	case K_KP_END:
		key = '1';
		break;
	case K_KP_DOWNARROW:
		key = '2';
		break;
	case K_KP_PGDN:
		key = '3';
		break;
	case K_KP_INS:
		key = '0';
		break;
	case K_KP_DEL:
		key = '.';
		break;
	}

	// mxd- This blocked Shift-Ins combo in the next block.
	// Knightmare- allow only the INS key thru, otherwise mouse events end up as text input!
	if (key > 127)
	{
		switch (key)
		{
		case K_INS:
		case K_KP_INS:
			break;
		case K_DEL:
		default:
			return false;
		}
	}


	//
	// support pasting from the clipboard
	//
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
		return true;
	}

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
	}

	return true;
}

//=========================================================

void Menulist_DoEnter (menulist_s *l)
{
	int start;

	if (!l) return;

	start = l->generic.y / 10 + 1;

	l->curValue = l->generic.parent->cursor - start;

	if (l->generic.callback)
		l->generic.callback(l);
}

void MenuList_Draw (menulist_s *l)
{
	const char	**n;
	int			y = 0, alpha;

	if (!l) return;

	alpha = UI_MouseOverAlpha(&l->generic);

	UI_DrawStringR2LDark (l->generic.x + l->generic.parent->x + LCOLUMN_OFFSET,	// - 2*MENU_FONT_SIZE,
						l->generic.y + l->generic.parent->y, l->generic.textSize, ALIGN_CENTER, l->generic.name, FONT_UI, alpha);

	n = l->itemNames;

//	UI_DrawFill (l->generic.parent->x + l->generic.x - 112, l->generic.parent->y + l->generic.y + (l->curValue+1)*MENU_LINE_SIZE,
//				128, MENU_LINE_SIZE, ALIGN_CENTER, false, 16);
	UI_DrawFill (l->generic.parent->x + l->generic.x - 112, l->generic.parent->y + l->generic.y + (l->curValue+1)*MENU_LINE_SIZE,
				128, MENU_LINE_SIZE, ALIGN_CENTER, false, color8red(16), color8green(16), color8blue(16), 255);

	while (*n)
	{
		UI_DrawStringR2LDark (l->generic.x + l->generic.parent->x + LCOLUMN_OFFSET,
							l->generic.y + l->generic.parent->y + y + MENU_LINE_SIZE, l->generic.textSize, ALIGN_CENTER, *n, FONT_UI, alpha);
		n++;
		y += MENU_LINE_SIZE;
	}
}

//=========================================================

void MenuSeparator_Draw (menuseparator_s *s)
{
	int alpha;

	if (!s) return;

	alpha = UI_MouseOverAlpha(&s->generic);

	if (s->generic.name)
		UI_DrawStringR2LDark (s->generic.x + s->generic.parent->x,
								s->generic.y + s->generic.parent->y, s->generic.textSize, ALIGN_CENTER, s->generic.name, FONT_UI, alpha);
}

//=========================================================

void UI_MenuSlider_SetValue (menuslider_s *s, const char *varName, float cvarMin, float cvarMax, qboolean clamp)
{
	if (!s || !varName || varName[0] == '\0')
		return;
	if (!s->increment)
		s->increment = 1.0f;

	if (clamp) {
		UI_ClampCvar (varName, cvarMin, cvarMax);
	}
	s->curPos = (int)ceil((Cvar_VariableValue((char *)varName) - s->baseValue) / s->increment);
	s->curPos = min(max(s->curPos, 0), s->maxPos);
}

void UI_MenuSlider_SaveValue (menuslider_s *s, const char *varName)
{
	if (!s || !varName || varName[0] == '\0')
		return;

	Cvar_SetValue ((char *)varName, ((float)s->curPos * s->increment) + s->baseValue);
}

float UI_MenuSlider_GetValue (menuslider_s *s)
{
	if (!s) return 0.0f;

	if (!s->increment)
		s->increment = 1.0f;

	return ((float)s->curPos * s->increment) + s->baseValue;
}

void MenuSlider_DoSlide (menuslider_s *s, int dir)
{
	if (!s) return;

	s->curPos += dir;

	s->curPos = min(max(s->curPos, 0), s->maxPos);

	if (s->generic.callback)
		s->generic.callback(s);
}

#define SLIDER_RANGE 10

void MenuSlider_Draw (menuslider_s *s)
{
	int		i, x, y, alpha;
	float	tmpValue;
	char	valueText[8];

	if (!s) return;

	alpha = UI_MouseOverAlpha(&s->generic);

	UI_DrawStringR2LDark (s->generic.x + s->generic.parent->x + LCOLUMN_OFFSET,
							s->generic.y + s->generic.parent->y, s->generic.textSize, ALIGN_CENTER, s->generic.name, FONT_UI, alpha);

	if (!s->maxPos)
		s->maxPos = 1;
	if (!s->increment)
		s->increment = 1.0f;

	s->range = (float)s->curPos / (float)s->maxPos;

	if (s->range < 0)
		s->range = 0;
	if (s->range > 1)
		s->range = 1;

	x = s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET;
	y = s->generic.y + s->generic.parent->y;

	// draw left
#ifdef NOTTHIRTYFLIGHTS
	UI_DrawPicST (x, y, SLIDER_ENDCAP_WIDTH, SLIDER_HEIGHT,
						stCoord_slider_left, ALIGN_CENTER, true, color_identity, UI_SLIDER_PIC);
#else
	SCR_DrawChar (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET,
				s->generic.y + s->generic.parent->y, s->generic.textSize, ALIGN_CENTER, 128, FONT_UI, 255,255,255,255, false, false);
#endif

	// draw center
	x += SLIDER_ENDCAP_WIDTH;
	for (i = 0; i < SLIDER_RANGE; i++) {
#ifdef NOTTHIRTYFLIGHTS
		UI_DrawPicST (x + i*SLIDER_SECTION_WIDTH, y, SLIDER_SECTION_WIDTH, SLIDER_HEIGHT,
							stCoord_slider_center, ALIGN_CENTER, true, color_identity, UI_SLIDER_PIC);
#else
		SCR_DrawChar (s->generic.x + s->generic.parent->x + (i+1)*s->generic.textSize + RCOLUMN_OFFSET,
					s->generic.y + s->generic.parent->y, s->generic.textSize, ALIGN_CENTER, 129, FONT_UI, 255,255,255,255, false, false);
#endif
	}

	// draw right
#ifdef NOTTHIRTYFLIGHTS
	UI_DrawPicST (x + i*SLIDER_SECTION_WIDTH, y, SLIDER_ENDCAP_WIDTH, SLIDER_HEIGHT,
						stCoord_slider_right, ALIGN_CENTER, true, color_identity, UI_SLIDER_PIC);
#else
	SCR_DrawChar (s->generic.x + s->generic.parent->x + (i+1)*s->generic.textSize + RCOLUMN_OFFSET,
				s->generic.y + s->generic.parent->y, s->generic.textSize, ALIGN_CENTER, 130, FONT_UI, 255,255,255,255, false, false);
#endif

	// draw knob
#ifdef NOTTHIRTYFLIGHTS
	UI_DrawPicST (x + SLIDER_RANGE*SLIDER_SECTION_WIDTH*s->range - (SLIDER_KNOB_WIDTH/2), y, SLIDER_KNOB_WIDTH, SLIDER_HEIGHT,
						stCoord_slider_knob, ALIGN_CENTER, true, color_identity, UI_SLIDER_PIC);
#else
	SCR_DrawChar (s->generic.x + s->generic.parent->x + s->generic.textSize*((SLIDER_RANGE-1)*s->range+1) + RCOLUMN_OFFSET,
				s->generic.y + s->generic.parent->y, s->generic.textSize, ALIGN_CENTER, 131, FONT_UI, 255,255,255,255, false, true);
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
	UI_DrawString (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET + 2*SLIDER_ENDCAP_WIDTH + i*SLIDER_SECTION_WIDTH + MENU_FONT_SIZE/2,
					s->generic.y + s->generic.parent->y + 1, MENU_FONT_SIZE-2, ALIGN_CENTER, valueText, FONT_UI, alpha);
//	UI_DrawString (s->generic.x + s->generic.parent->x + s->generic.textSize*SLIDER_RANGE + RCOLUMN_OFFSET + 2.5*MENU_FONT_SIZE,
//					s->generic.y + s->generic.parent->y + 1, MENU_FONT_SIZE-2, ALIGN_CENTER, valueText, FONT_UI, alpha);
}

//=========================================================

void UI_MenuSpinControl_SetValue (menulist_s *s, const char *varName, float cvarMin, float cvarMax, qboolean clamp)
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

void UI_MenuSpinControl_SaveValue (menulist_s *s, const char *varName)
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

const char *UI_MenuSpinControl_GetValue (menulist_s *s)
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

void MenuSpinControl_DoEnter (menulist_s *s)
{
	if (!s || !s->itemNames || !s->numItems)
		return;

	s->curValue++;
	if (s->itemNames[s->curValue] == 0)
		s->curValue = 0;

	if (s->generic.callback)
		s->generic.callback(s);
}

void MenuSpinControl_DoSlide (menulist_s *s, int dir)
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
 
void MenuSpinControl_Draw (menulist_s *s)
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

void MenuKeyBind_DoEnter (menukeybind_s *k)
{
	menuframework_s	*menu = k->generic.parent;

	if (!menu)	return;

	UI_FindKeysForCommand (k->commandName, k->keys);

//	if (k->keys[1] != -1)
//		UI_UnbindCommand (k->commandName);

	UI_SetGrabBindItem (menu, (menucommon_s *)k);
		
	if (k->generic.callback)
		k->generic.callback (k);
}

void MenuKeyBind_Draw (menukeybind_s *k)
{
	menuframework_s	*menu = k->generic.parent;
	int				x, alpha = UI_MouseOverAlpha(&k->generic);
	const char		*keyName1, *keyName2;

	UI_DrawMenuString (menu->x + k->generic.x + LCOLUMN_OFFSET,
						menu->y + k->generic.y, MENU_FONT_SIZE, ALIGN_CENTER, k->generic.name, alpha,
						!(k->generic.flags & QMF_LEFT_JUSTIFY), (k->generic.flags & QMF_ALTCOLOR));

	if (k->commandName)
	{
		UI_FindKeysForCommand (k->commandName, k->keys);

		if (k->keys[0] == -1)
		{
			UI_DrawMenuString (menu->x + k->generic.x + RCOLUMN_OFFSET,
								menu->y + k->generic.y, MENU_FONT_SIZE, ALIGN_CENTER, "???", alpha, false, false);
		}
		else
		{
			keyName1 = Key_KeynumToString (k->keys[0]);
			UI_DrawMenuString (menu->x + k->generic.x + RCOLUMN_OFFSET,
								menu->y + k->generic.y, MENU_FONT_SIZE, ALIGN_CENTER, keyName1, alpha, false, false);
			if (k->keys[1] != -1)
			{
				x = (int)strlen(keyName1) * MENU_FONT_SIZE;
				keyName2 = Key_KeynumToString (k->keys[1]);
				UI_DrawMenuString (menu->x + k->generic.x + MENU_FONT_SIZE*3 + x,
									menu->y + k->generic.y, MENU_FONT_SIZE, ALIGN_CENTER, "or", alpha, false, false);
				UI_DrawMenuString (menu->x + k->generic.x + MENU_FONT_SIZE*6 + x,
									menu->y + k->generic.y, MENU_FONT_SIZE, ALIGN_CENTER, keyName2, alpha, false, false);
			}
		}
	}
	else
		Com_Printf ("UI_MenuKeyBind_Draw: keybind has no commandName!\n");

//	if (k->generic.ownerdraw)
//		k->generic.ownerdraw(k);
}

const char *UI_MenuKeyBind_Key (menukeybind_s *k, int key)
{
	menuframework_s	*menu = k->generic.parent;

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
		UI_PopMenu ();
		return ui_menu_out_sound;
	case K_ENTER:
	case K_KP_ENTER:
		MenuKeyBind_DoEnter (k);
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

void checkbox_DoEnter( menulist_s *s )
{
	s->curValue++;
	if ( s->itemNames[s->curValue] == 0 )
		s->curValue = 0;

	if ( s->generic.callback )
		s->generic.callback( s );
}

void checkbox_DoSlide( menulist_s *s, int dir )
{
	s->curValue += dir;

	/*if ( s->curValue < 0 )
		s->curValue = 0;
	else if ( s->itemnames[s->curValue] == 0 )
		s->curValue--;*/


	if ( s->curValue > 1 )
		s->curValue = 0;
	else if ( s->curValue < 0 )
		s->curValue = 1;


	if ( s->generic.callback )
		s->generic.callback( s );
}

void checkbox_Draw( menulist_s *s  )
{
	int alpha = UI_MouseOverAlpha(&s->generic);	
	char buffer[100];

	if ( s->generic.name )
	{

		if (s->generic.flags & QMF_LEFT_JUSTIFY)
		{
			UI_DrawString (s->generic.x + s->generic.parent->x + LCOLUMN_OFFSET + 13,
							s->generic.y + s->generic.parent->y,
							s->generic.textSize,
							ALIGN_CENTER,
							s->generic.name, FONT_UI, alpha);
		}
		else
		{
			UI_DrawStringR2LDark (s->generic.x + s->generic.parent->x + LCOLUMN_OFFSET,
				s->generic.y + s->generic.parent->y,
				s->generic.textSize,
				ALIGN_CENTER,
				s->generic.name,
				FONT_UI,
				alpha);
		}
	}

	if ( s->curValue > 0 )
	{
		strcpy(buffer, "m_checkbox_on");
	}
	else
		strcpy(buffer, "m_checkbox_off");

	if (buffer)
	{
		int yoffset=0;
		drawStruct_t ds;

		if (viddef.height==1024)
		{
			yoffset = MENU_FONT_SIZE*2;
		}

		memset(&ds, 0, sizeof(drawStruct_t));
		ds.pic = buffer;
		if (s->generic.flags & QMF_LEFT_JUSTIFY)
		{
			ds.x = (viddef.width / 2) + SCR_ScaledScreen(-38);
			/*R_DrawStretchPic (
				(viddef.width / 2) + SCR_ScaledScreen(-38),
				SCR_ScaledScreen(s->generic.y + s->generic.parent->y+yoffset),
				SCR_ScaledScreen(MENU_FONT_SIZE), SCR_ScaledScreen(MENU_FONT_SIZE), buffer,
				alpha / 255.0);*/
		}
		else
		{
			ds.x = (viddef.width / 2) + SCR_ScaledScreen(24);
			/*R_DrawStretchPic (
				(viddef.width / 2) + SCR_ScaledScreen(24),
				SCR_ScaledScreen(s->generic.y + s->generic.parent->y+yoffset),
				SCR_ScaledScreen(MENU_FONT_SIZE), SCR_ScaledScreen(MENU_FONT_SIZE), buffer,
				alpha / 255.0);*/
		}
		ds.y = SCR_ScaledScreen(s->generic.y + s->generic.parent->y+yoffset);
		ds.w = SCR_ScaledScreen(MENU_FONT_SIZE);
		ds.h = SCR_ScaledScreen(MENU_FONT_SIZE);
		Vector2Copy(vec2_origin, ds.offset);
		Vector4Copy(vec4_identity, ds.color);
		ds.color[3] = alpha / 255.0;
		R_DrawPic(ds);
	}


}

//=========================================================

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
	if ( ((menucommon_s *)item)->flags & QMF_HIDDEN )
		return;

	switch ( ((menucommon_s *)item)->type )
	{
	case MTYPE_FIELD:
		MenuField_Draw ((menufield_s *)item);
		break;
	case MTYPE_SLIDER:
		MenuSlider_Draw ((menuslider_s *)item);
		break;
	case MTYPE_LIST:
		MenuList_Draw ((menulist_s *)item);
		break;
	case MTYPE_SPINCONTROL:
		MenuSpinControl_Draw ((menulist_s *)item);
		break;
	case MTYPE_ACTION:
		MenuAction_Draw ((menuaction_s *)item);
		break;
	case MTYPE_SEPARATOR:
		MenuSeparator_Draw ((menuseparator_s *)item);
		break;
	case MTYPE_KEYBIND:
		MenuKeyBind_Draw ((menukeybind_s *)item);
		break;
	case MTYPE_CHECKBOX:
		checkbox_Draw((menulist_s *)item);
		break;
	default:
		break;
	}
}


/*
=================
UI_SelectMenuItem
=================
*/
qboolean UI_SelectMenuItem (menuframework_s *s)
{
	menucommon_s *item=NULL;

	if (!s)	return false;

	item = (menucommon_s *)UI_ItemAtMenuCursor(s);

	if (item)
	{
		switch (item->type)
		{
		case MTYPE_FIELD:
			return MenuField_DoEnter ( (menufield_s *)item ) ;
		case MTYPE_ACTION:
			MenuAction_DoEnter ( (menuaction_s *)item );
			return true;
		case MTYPE_LIST:
		//	Menulist_DoEnter ( (menulist_s *)item );
			return false;
		case MTYPE_SPINCONTROL:
		//	MenuSpinControl_DoEnter ( (menulist_s *)item );
			return false;
		case MTYPE_KEYBIND:
			MenuKeyBind_DoEnter ( (menukeybind_s *)item );
			return true;
		case MTYPE_CHECKBOX:
			return false;
		default:
			break;
		}
	}
	return false;
}


/*
=================
UI_MouseSelectItem
=================
*/
qboolean UI_MouseSelectItem (menucommon_s *item)
{
	if (!item)	return false;

	if (item)
	{
		switch (item->type)
		{
		case MTYPE_FIELD:
			return MenuField_DoEnter ( (menufield_s *)item ) ;
		case MTYPE_ACTION:
			MenuAction_DoEnter ( (menuaction_s *)item );
			return true;
		case MTYPE_KEYBIND:
			MenuKeyBind_DoEnter ( (menukeybind_s *)item );
			return true;
		case MTYPE_LIST:
		case MTYPE_SPINCONTROL:
			return false;
		default:
			break;
		}
	}
	return false;
}


/*
=================
UI_SlideMenuItem
=================
*/
void UI_SlideMenuItem (menuframework_s *s, int dir)
{
	menucommon_s *item=NULL;

	if (!s)	return;

	item = (menucommon_s *) UI_ItemAtMenuCursor(s);

	if (item)
	{
		switch (item->type)
		{
		case MTYPE_SLIDER:
			MenuSlider_DoSlide ((menuslider_s *) item, dir);
			break;
		case MTYPE_SPINCONTROL:
			MenuSpinControl_DoSlide ((menulist_s *) item, dir);
			break;
		case MTYPE_CHECKBOX:
			checkbox_DoSlide( ( menulist_s * ) item, dir );
			break;
		default:
			break;
		}
	}
}
