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

// ui_backend.c -- supporting code for menu widgets


#include <string.h>
#include <ctype.h>

#include "../client/client.h"
#include "ui_local.h"

static	void	Action_DoEnter( menuaction_s *a );
static	void	Action_Draw( menuaction_s *a );
static	void	Menu_DrawStatusBar( const char *string );
static	void	Menulist_DoEnter( menulist_s *l );
static	void	MenuList_Draw( menulist_s *l );
static	void	Separator_Draw( menuseparator_s *s );
static	void	Slider_DoSlide( menuslider_s *s, int dir );
static	void	Slider_Draw( menuslider_s *s );
static	void	SpinControl_DoEnter( menulist_s *s );
static	void	SpinControl_Draw( menulist_s *s );
static	void	SpinControl_DoSlide( menulist_s *s, int dir );

#define RCOLUMN_OFFSET  MENU_FONT_SIZE*2	// was 16
#define LCOLUMN_OFFSET -MENU_FONT_SIZE*2	// was -16

extern viddef_t viddef;

#define VID_WIDTH viddef.width
#define VID_HEIGHT viddef.height

// added Psychospaz's menu mouse support
//======================================================

int mouseOverAlpha (menucommon_s *m)
{
	if (cursor.menuitem == m)
	{
		int alpha;

		alpha = 125 + 25 * cos(anglemod(cl.time*0.005));

		if (alpha>255) alpha = 255;
		if (alpha<0) alpha = 0;

		return alpha;
	}
	else 
		return 255;
}
//======================================================


void Action_DoEnter (menuaction_s *a)
{
	if (a->generic.callback)
		a->generic.callback(a);
}

void Action_Draw (menuaction_s *a)
{
	int alpha = mouseOverAlpha(&a->generic);

	if (a->generic.flags & QMF_LEFT_JUSTIFY)
	{
		if (a->generic.flags & QMF_GRAYED)
			Menu_DrawStringDark (a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET,
								a->generic.y + a->generic.parent->y, a->generic.textSize, a->generic.name, alpha);
		else
			Menu_DrawString (a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET,
							a->generic.y + a->generic.parent->y, a->generic.textSize, a->generic.name, alpha);
	}
	else
	{
		if (a->generic.flags & QMF_GRAYED)
			Menu_DrawStringR2LDark (a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET,
									a->generic.y + a->generic.parent->y, a->generic.textSize, a->generic.name, alpha);
		else
			Menu_DrawStringR2L (a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET,
								a->generic.y + a->generic.parent->y, a->generic.textSize, a->generic.name, alpha);
	}
	if (a->generic.ownerdraw)
		a->generic.ownerdraw(a);
}

qboolean Field_DoEnter (menufield_s *f)
{
	if (f->generic.callback)
	{
		f->generic.callback(f);
		return true;
	}
	return false;
}

void Field_Draw (menufield_s *f)
{
	int i, alpha = mouseOverAlpha(&f->generic), xtra;
	char tempbuffer[128]="";
	int offset;

	if (f->generic.name)
		Menu_DrawStringR2LDark (f->generic.x + f->generic.parent->x + LCOLUMN_OFFSET,
								f->generic.y + f->generic.parent->y, f->generic.textSize, f->generic.name, 255);

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

	SCR_DrawChar (f->generic.x + f->generic.parent->x + RCOLUMN_OFFSET,
				f->generic.y + f->generic.parent->y - 4, f->generic.textSize, ALIGN_CENTER, 18, FONT_UI, 255,255,255,255, false, false);
	SCR_DrawChar (f->generic.x + f->generic.parent->x + RCOLUMN_OFFSET,
				f->generic.y + f->generic.parent->y + 4, f->generic.textSize, ALIGN_CENTER, 24, FONT_UI, 255,255,255,255, false, false);
	SCR_DrawChar (f->generic.x + f->generic.parent->x + (1+f->visible_length)*f->generic.textSize + RCOLUMN_OFFSET,
				f->generic.y + f->generic.parent->y - 4, f->generic.textSize, ALIGN_CENTER, 20, FONT_UI, 255,255,255,255, false, false);
	SCR_DrawChar (f->generic.x + f->generic.parent->x + (1+f->visible_length)*f->generic.textSize + RCOLUMN_OFFSET,
				f->generic.y + f->generic.parent->y + 4, f->generic.textSize, ALIGN_CENTER, 26, FONT_UI, 255,255,255,255, false, false);

	for (i = 0; i < f->visible_length; i++)
	{
		SCR_DrawChar (f->generic.x + f->generic.parent->x + (1+i)*f->generic.textSize + RCOLUMN_OFFSET,
					f->generic.y + f->generic.parent->y - 4, f->generic.textSize, ALIGN_CENTER, 19, FONT_UI, 255,255,255,255, false, false);
		SCR_DrawChar (f->generic.x + f->generic.parent->x + (1+i)*f->generic.textSize + RCOLUMN_OFFSET,
					f->generic.y + f->generic.parent->y + 4, f->generic.textSize, ALIGN_CENTER, 25, FONT_UI, 255,255,255,255, false, (i==(f->visible_length-1)));
	}

	// add cursor thingie
	if ( (Menu_ItemAtCursor(f->generic.parent) == f)  && ((int)(Sys_Milliseconds()/250))&1 )
		Com_sprintf(tempbuffer, sizeof(tempbuffer),	"%s%c", tempbuffer, 11);

	Menu_DrawString (f->generic.x + f->generic.parent->x + f->generic.textSize*3,
					f->generic.y + f->generic.parent->y, f->generic.textSize, tempbuffer, alpha);
}

qboolean Field_Key (menufield_s *f, int key)
{
	extern int keydown[];

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

void Menulist_DoEnter (menulist_s *l)
{
	int start;

	start = l->generic.y / 10 + 1;

	l->curvalue = l->generic.parent->cursor - start;

	if (l->generic.callback)
		l->generic.callback(l);
}

void MenuList_Draw (menulist_s *l)
{
	const char **n;
	int y = 0, alpha = mouseOverAlpha(&l->generic);
	
	Menu_DrawStringR2LDark (l->generic.x + l->generic.parent->x + LCOLUMN_OFFSET,	// - 2*MENU_FONT_SIZE,
						l->generic.y + l->generic.parent->y, l->generic.textSize, l->generic.name, alpha);

	n = l->itemnames;

//	SCR_DrawFill (l->generic.parent->x + l->generic.x - 112, l->generic.parent->y + l->generic.y + (l->curvalue+1)*MENU_LINE_SIZE,
//				128, MENU_LINE_SIZE, ALIGN_CENTER, 16);
	SCR_DrawFill (l->generic.parent->x + l->generic.x - 112, l->generic.parent->y + l->generic.y + (l->curvalue+1)*MENU_LINE_SIZE,
				128, MENU_LINE_SIZE, ALIGN_CENTER, color8red(16), color8green(16), color8blue(16), 255);

	while (*n)
	{
		Menu_DrawStringR2LDark (l->generic.x + l->generic.parent->x + LCOLUMN_OFFSET,
							l->generic.y + l->generic.parent->y + y + MENU_LINE_SIZE, l->generic.textSize, *n, alpha);
		n++;
		y += MENU_LINE_SIZE;
	}
}

void Separator_Draw (menuseparator_s *s)
{
	int alpha = mouseOverAlpha(&s->generic);

	if (s->generic.name)
		Menu_DrawStringR2LDark (s->generic.x + s->generic.parent->x,
								s->generic.y + s->generic.parent->y, s->generic.textSize, s->generic.name, alpha);
}

void Slider_DoSlide (menuslider_s *s, int dir)
{
	s->curvalue += dir;

	if (s->curvalue > s->maxvalue)
		s->curvalue = s->maxvalue;
	else if (s->curvalue < s->minvalue)
		s->curvalue = s->minvalue;

	if (s->generic.callback)
		s->generic.callback(s);
}

#define SLIDER_RANGE 10

void Slider_Draw (menuslider_s *s)
{
	int	i, alpha = mouseOverAlpha(&s->generic);

	Menu_DrawStringR2LDark (s->generic.x + s->generic.parent->x + LCOLUMN_OFFSET,
							s->generic.y + s->generic.parent->y, s->generic.textSize, s->generic.name, alpha);

	s->range = (s->curvalue - s->minvalue) / (float)(s->maxvalue - s->minvalue);

	if (s->range < 0)
		s->range = 0;
	if (s->range > 1)
		s->range = 1;

	SCR_DrawChar (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET,
				s->generic.y + s->generic.parent->y, s->generic.textSize, ALIGN_CENTER, 128, FONT_UI, 255,255,255,255, false, false);

	for (i = 0; i < SLIDER_RANGE; i++)
		SCR_DrawChar (s->generic.x + s->generic.parent->x + (i+1)*s->generic.textSize + RCOLUMN_OFFSET,
					s->generic.y + s->generic.parent->y, s->generic.textSize, ALIGN_CENTER, 129, FONT_UI, 255,255,255,255, false, false);

	SCR_DrawChar (s->generic.x + s->generic.parent->x + (i+1)*s->generic.textSize + RCOLUMN_OFFSET,
				s->generic.y + s->generic.parent->y, s->generic.textSize, ALIGN_CENTER, 130, FONT_UI, 255,255,255,255, false, false);

	SCR_DrawChar (s->generic.x + s->generic.parent->x + s->generic.textSize*((SLIDER_RANGE-1)*s->range+1) + RCOLUMN_OFFSET,
				s->generic.y + s->generic.parent->y, s->generic.textSize, ALIGN_CENTER, 131, FONT_UI, 255,255,255,255, false, true);
}

void SpinControl_DoEnter (menulist_s *s)
{
	if (!s->itemnames || !s->numitemnames)
		return;

	s->curvalue++;
	if (s->itemnames[s->curvalue] == 0)
		s->curvalue = 0;

	if (s->generic.callback)
		s->generic.callback(s);
}

void SpinControl_DoSlide (menulist_s *s, int dir)
{
	if (!s->itemnames || !s->numitemnames)
		return;

	s->curvalue += dir;

	if (s->generic.flags & QMF_SKINLIST) // don't allow looping around for skin lists
	{
		if (s->curvalue < 0)
			s->curvalue = 0;
		else if (s->itemnames[s->curvalue] == 0)
			s->curvalue--;
	}
	else {
		if (s->curvalue < 0)
			s->curvalue = s->numitemnames-1; // was 0
		else if (s->itemnames[s->curvalue] == 0)
			s->curvalue = 0; // was --
	}

	if (s->generic.callback)
		s->generic.callback(s);
}
 
void SpinControl_Draw (menulist_s *s)
{
	int alpha = mouseOverAlpha (&s->generic);
	char buffer[100];

	if (s->generic.name)
	{
		Menu_DrawStringR2LDark (s->generic.x + s->generic.parent->x + LCOLUMN_OFFSET,
								s->generic.y + s->generic.parent->y, s->generic.textSize, s->generic.name, alpha);
	}
	if (!strchr(s->itemnames[s->curvalue], '\n'))
	{
		Menu_DrawString (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET,
						s->generic.y + s->generic.parent->y, s->generic.textSize, s->itemnames[s->curvalue], alpha);
	}
	else
	{
	//	strncpy(buffer, s->itemnames[s->curvalue]);
		Q_strncpyz (buffer, sizeof(buffer), s->itemnames[s->curvalue]);
		*strchr(buffer, '\n') = 0;
		Menu_DrawString (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET,
						s->generic.y + s->generic.parent->y, s->generic.textSize, buffer, alpha);
	//	strncpy(buffer, strchr( s->itemnames[s->curvalue], '\n' ) + 1 );
		Q_strncpyz (buffer, sizeof(buffer), strchr( s->itemnames[s->curvalue], '\n' ) + 1);
		Menu_DrawString (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET,
						s->generic.y + s->generic.parent->y + MENU_LINE_SIZE, s->generic.textSize, buffer, alpha);
	}
}


/*
==========================
Menu_AddItem
==========================
*/
void Menu_AddItem (menuframework_s *menu, void *item)
{
	int			i;
	menulist_s	*list;
	menucommon_s *baseItem;

	if (menu->nitems == 0)
		menu->nslots = 0;

	if (menu->nitems < MAXMENUITEMS)
	{
		menu->items[menu->nitems] = item;
		( (menucommon_s *)menu->items[menu->nitems] )->parent = menu;
		menu->nitems++;
	}

	menu->nslots = Menu_TallySlots(menu);

	list = (menulist_s *)item;

	switch (list->generic.type) {
	case MTYPE_SPINCONTROL:
		for (i=0; list->itemnames[i]; i++);
		list->numitemnames = i;
		break;
	}

	// Knightmare- init text size
	baseItem = (menucommon_s *)item;
	if (!baseItem->textSize)
		baseItem->textSize = MENU_FONT_SIZE;
	baseItem->textSize = min(max(baseItem->textSize, 4), 32);
	// end Knightmare
}


/*
==========================
Menu_ItemIsValidCursorPosition
Checks if an item can be used
as a cursor position.
==========================
*/
qboolean Menu_ItemIsValidCursorPosition (void *item)
{
	if (!item)	return false;
	
//	if ( (((menuCommon_s *)item)->flags & QMF_NOINTERACTION) || (((menuCommon_s *)item)->flags & QMF_MOUSEONLY) )
//		return false;

	// hidden items are invalid
	if ( ((menucommon_s *)item)->flags & QMF_HIDDEN )
		return false;

	switch ( ((menucommon_s *)item)->type )
	{
	case MTYPE_SEPARATOR:
		return false;
	default:
		return true;
	}
	return true;
}


/*
==========================
Menu_AdjustCursor

This function takes the given menu, the direction, and attempts
to adjust the menu's cursor so that it's at the next available
slot.
==========================
*/
void Menu_AdjustCursor (menuframework_s *m, int dir)
{
	menucommon_s *citem;

	//
	// see if it's in a valid spot
	//
	if (m->cursor >= 0 && m->cursor < m->nitems)
	{
		if ( (citem = Menu_ItemAtCursor(m)) != 0 )
		{
		//	if (citem->type != MTYPE_SEPARATOR)
			if ( Menu_ItemIsValidCursorPosition(citem) )
				return;
		}
	}

	//
	// it's not in a valid spot, so crawl in the direction indicated until we
	// find a valid spot
	//
	if (dir == 1)
	{
		while (1)
		{
			if ( (citem = Menu_ItemAtCursor(m)) != 0 )
			//	if ( citem->type != MTYPE_SEPARATOR )
				if ( Menu_ItemIsValidCursorPosition(citem) )
					break;
			m->cursor += dir;
			if ( m->cursor >= m->nitems )
				m->cursor = 0;
		}
	}
	else
	{
		while (1)
		{
			if ( (citem = Menu_ItemAtCursor(m)) != 0 )
			//	if (citem->type != MTYPE_SEPARATOR)
				if ( Menu_ItemIsValidCursorPosition(citem) )
					break;
			m->cursor += dir;
			if (m->cursor < 0)
				m->cursor = m->nitems - 1;
		}
	}
}


/*
==========================
Menu_Center
==========================
*/
void Menu_Center (menuframework_s *menu)
{
	int height = ((menucommon_s *) menu->items[menu->nitems-1])->y + 10;
	menu->y = (SCREEN_HEIGHT - height)*0.5;
}


/*
==========================
Menu_Draw
==========================
*/
void Menu_Draw (menuframework_s *menu)
{
	int i;
	menucommon_s *item;

	//
	// draw contents
	//
	for (i = 0; i < menu->nitems; i++)
	{
		// skip hidden items
		if ( ((menucommon_s *)menu->items[i])->flags & QMF_HIDDEN )
			continue;

		switch ( ((menucommon_s *)menu->items[i])->type )
		{
		case MTYPE_FIELD:
			Field_Draw( ( menufield_s * )menu->items[i] );
			break;
		case MTYPE_SLIDER:
			Slider_Draw( (menuslider_s *)menu->items[i] );
			break;
		case MTYPE_LIST:
			MenuList_Draw( (menulist_s *)menu->items[i] );
			break;
		case MTYPE_SPINCONTROL:
			SpinControl_Draw( (menulist_s *)menu->items[i] );
			break;
		case MTYPE_ACTION:
			Action_Draw( (menuaction_s *)menu->items[i] );
			break;
		case MTYPE_SEPARATOR:
			Separator_Draw( (menuseparator_s *)menu->items[i] );
			break;
		}
	}

	// added Psychspaz's mouse support
	//
	// now check mouseovers - psychospaz
	//
	cursor.menu = menu;

	if (cursor.mouseaction)
	{
		menucommon_s *lastitem = cursor.menuitem;
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
			SCR_AdjustFrom640 (&x1, &y1, &w1, &h1, ALIGN_CENTER);
			min[0] = x1;	max[0] = x1+w1;
			min[1] = y1;	max[1] = y1+h1;
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

						len = (int)strlen(spin->itemnames[spin->curvalue]);
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

			if (cursor.x>=min[0] && 
				cursor.x<=max[0] &&
				cursor.y>=min[1] && 
				cursor.y<=max[1])
			{
				// new item
				if (lastitem!=item)
				{
					int j;

					for (j=0; j<MENU_CURSOR_BUTTON_MAX; j++)
					{
						cursor.buttonclicks[j] = 0;
						cursor.buttontime[j] = 0;
					}
				}

				cursor.menuitem = item;
				cursor.menuitemtype = type;
				
				menu->cursor = i;

				break;
			}
		}
	}

	cursor.mouseaction = false;
	// end mouseover code

	item = Menu_ItemAtCursor(menu);

	if (item && item->cursordraw)
	{
		item->cursordraw(item);
	}
	else if (menu->cursordraw)
	{
		menu->cursordraw(menu);
	}
	else if (item && item->type != MTYPE_FIELD)
	{
		if (item->flags & QMF_LEFT_JUSTIFY)
		{
			SCR_DrawChar (menu->x+item->x+item->cursor_offset-24, menu->y+item->y,
			//			MENU_FONT_SIZE, ALIGN_CENTER, 12+((int)(Sys_Milliseconds()/250)&1),
						item->textSize, ALIGN_CENTER, 12+((int)(Sys_Milliseconds()/250)&1),
						FONT_UI, 255,255,255,255, false, true);
		}
		else
		{
			SCR_DrawChar (menu->x+item->cursor_offset, menu->y+item->y,
			//			MENU_FONT_SIZE, ALIGN_CENTER, 12+((int)(Sys_Milliseconds()/250)&1),
						item->textSize, ALIGN_CENTER, 12+((int)(Sys_Milliseconds()/250)&1),
						FONT_UI, 255,255,255,255, false, true);
		}
	}

	if (item)
	{
		if (item->statusbarfunc)
			item->statusbarfunc ( (void *)item );
		else if (item->statusbar)
			Menu_DrawStatusBar (item->statusbar);
		else
			Menu_DrawStatusBar (menu->statusbar);
	}
	else
		Menu_DrawStatusBar( menu->statusbar );
}

void Menu_DrawStatusBar (const char *string)
{
	if (string)
	{
		int l = (int)strlen( string );

		SCR_DrawFill (0, SCREEN_HEIGHT-(MENU_FONT_SIZE+3), SCREEN_WIDTH, MENU_FONT_SIZE+4, ALIGN_BOTTOM_STRETCH, 60,60,60,255 );	// go 1 pixel past screen bottom to prevent gap from scaling
		SCR_DrawFill (0, SCREEN_HEIGHT-(MENU_FONT_SIZE+3), SCREEN_WIDTH, 1, ALIGN_BOTTOM_STRETCH, 0,0,0,255 );
		SCR_DrawString (SCREEN_WIDTH/2-(l/2)*MENU_FONT_SIZE, SCREEN_HEIGHT-(MENU_FONT_SIZE+1), MENU_FONT_SIZE, ALIGN_BOTTOM, string, FONT_UI, 255 );
	}
	else
		SCR_DrawFill (0, SCREEN_HEIGHT-(MENU_FONT_SIZE+3), SCREEN_WIDTH, MENU_FONT_SIZE+4, ALIGN_BOTTOM_STRETCH, 0,0,0,255 );	// go 1 pixel past screen bottom to prevent gap from scaling
}

void Menu_DrawString (int x, int y, int size, const char *string, int alpha)
{
	SCR_DrawString (x, y, size, ALIGN_CENTER, string, FONT_UI, alpha);
}

void Menu_DrawStringDark (int x, int y, int size, const char *string, int alpha)
{
	char	newstring[1024];

	Com_sprintf (newstring, sizeof(newstring), S_COLOR_ALT"%s", string);
	SCR_DrawString (x, y, size, ALIGN_CENTER, newstring, FONT_UI, alpha);
}

void Menu_DrawStringR2L (int x, int y, int size, const char *string, int alpha)
{
	x -= stringLen(string)*size;	// MENU_FONT_SIZE
	SCR_DrawString (x, y, size, ALIGN_CENTER, string, FONT_UI, alpha);
}

void Menu_DrawStringR2LDark (int x, int y, int size, const char *string, int alpha)
{
	char	newstring[1024];

	Com_sprintf (newstring, sizeof(newstring), S_COLOR_ALT"%s", string);
	x -= stringLen(string)*size;	// MENU_FONT_SIZE
	SCR_DrawString (x, y, size, ALIGN_CENTER, newstring, FONT_UI, alpha);
}


/*
=============
Menu_DrawTextBox
=============
*/
void Menu_DrawTextBox (int x, int y, int width, int lines)
{
	int		cx, cy;
	int		n;

	// draw left side
	cx = x;
	cy = y;
	SCR_DrawChar (cx, cy, MENU_FONT_SIZE, ALIGN_CENTER, 1, FONT_UI, 255,255,255,255, false, false);
	for (n = 0; n < lines; n++) {
		cy += MENU_FONT_SIZE;
		SCR_DrawChar (cx, cy, MENU_FONT_SIZE, ALIGN_CENTER, 4, FONT_UI, 255,255,255,255, false, false);
	}
	SCR_DrawChar (cx, cy+MENU_FONT_SIZE, MENU_FONT_SIZE, ALIGN_CENTER, 7, FONT_UI, 255,255,255,255, false, false);

	// draw middle
	cx += MENU_FONT_SIZE;
	while (width > 0)
	{
		cy = y;
		SCR_DrawChar (cx, cy, MENU_FONT_SIZE, ALIGN_CENTER, 2, FONT_UI, 255,255,255,255, false, false);
		for (n = 0; n < lines; n++) {
			cy += MENU_FONT_SIZE;
			SCR_DrawChar (cx, cy, MENU_FONT_SIZE, ALIGN_CENTER, 5, FONT_UI, 255,255,255,255, false, false);
		}
		SCR_DrawChar (cx, cy+MENU_FONT_SIZE, MENU_FONT_SIZE, ALIGN_CENTER, 8, FONT_UI, 255,255,255,255, false, false);
		width -= 1;
		cx += MENU_FONT_SIZE;
	}

	// draw right side
	cy = y;
	SCR_DrawChar (cx, cy, MENU_FONT_SIZE, ALIGN_CENTER, 3, FONT_UI, 255,255,255,255, false, false);
	for (n = 0; n < lines; n++) {
		cy += MENU_FONT_SIZE;
		SCR_DrawChar (cx, cy, MENU_FONT_SIZE, ALIGN_CENTER, 6, FONT_UI, 255,255,255,255, false, false);
	}
	SCR_DrawChar (cx, cy+MENU_FONT_SIZE, MENU_FONT_SIZE, ALIGN_CENTER, 9, FONT_UI, 255,255,255,255, false, true);
}


/*
=================
Menu_DrawBanner
=================
*/
void Menu_DrawBanner (char *name)
{
	int w, h;

	R_DrawGetPicSize (&w, &h, name );
	SCR_DrawPic (SCREEN_WIDTH/2-w/2, SCREEN_HEIGHT/2-150, w, h, ALIGN_CENTER, name, 1.0);
}


void *Menu_ItemAtCursor (menuframework_s *m)
{
	if (m->cursor < 0 || m->cursor >= m->nitems)
		return 0;

	return m->items[m->cursor];
}

qboolean Menu_SelectItem (menuframework_s *s)
{
	menucommon_s *item = (menucommon_s *)Menu_ItemAtCursor(s);

	if (item)
	{
		switch (item->type)
		{
		case MTYPE_FIELD:
			return Field_DoEnter ( (menufield_s *)item ) ;
		case MTYPE_ACTION:
			Action_DoEnter ( (menuaction_s *)item );
			return true;
		case MTYPE_LIST:
		//	Menulist_DoEnter ( (menulist_s *)item );
			return false;
		case MTYPE_SPINCONTROL:
		//	SpinControl_DoEnter ( (menulist_s *)item );
			return false;
		}
	}
	return false;
}

qboolean Menu_MouseSelectItem (menucommon_s *item)
{
	if (item)
	{
		switch (item->type)
		{
		case MTYPE_FIELD:
			return Field_DoEnter ( (menufield_s *)item ) ;
		case MTYPE_ACTION:
			Action_DoEnter ( (menuaction_s *)item );
			return true;
		case MTYPE_LIST:
		case MTYPE_SPINCONTROL:
			return false;
		}
	}
	return false;
}

void Menu_SetStatusBar (menuframework_s *m, const char *string)
{
	m->statusbar = string;
}

void Menu_SlideItem (menuframework_s *s, int dir)
{
	menucommon_s *item = (menucommon_s *) Menu_ItemAtCursor(s);

	if (item)
	{
		switch (item->type)
		{
		case MTYPE_SLIDER:
			Slider_DoSlide ((menuslider_s *) item, dir);
			break;
		case MTYPE_SPINCONTROL:
			SpinControl_DoSlide ((menulist_s *) item, dir);
			break;
		}
	}
}

int Menu_TallySlots (menuframework_s *menu)
{
	int i;
	int total = 0;

	for (i = 0; i < menu->nitems; i++)
	{
		if ( ((menucommon_s *)menu->items[i])->type == MTYPE_LIST )
		{
			int nitems = 0;
			const char **n = ((menulist_s *)menu->items[i])->itemnames;

			while (*n)
				nitems++, n++;

			total += nitems;
		}
		else
			total++;
	}

	return total;
}


/*
=======================================================================

Menu Mouse Cursor - psychospaz

=======================================================================
*/
extern	void	(*m_drawfunc) (void);
extern	const char *(*m_keyfunc) (int key);

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

#if 0
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

	return (int)(SCR_ScaledScreen(MENU_FONT_SIZE) + RCOLUMN_OFFSET + (SLIDER_RANGE)*SCR_ScaledScreen(MENU_FONT_SIZE) * range);
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
//	pos = x - SCR_ScaledScreen(s->generic.x + s->generic.parent->x + MENU_FONT_SIZE + RCOLUMN_OFFSET);

	newValue = ((float)pos)/((SLIDER_RANGE-1)*SCR_ScaledScreen(MENU_FONT_SIZE));
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
		s->generic.callback (s);
}

/*
=================
Menu_DragSlideItem
=================
*/
void Menu_DragSlideItem (menuframework_s *menu, void *menuitem)
{
//	menucommon_s *item = (menucommon_s *) menuitem;
	menuslider_s *slider = (menuslider_s *) menuitem;

	slider->curvalue = NewSliderValueForX(cursor.x, slider);
	Slider_CheckSlide( slider );
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

	if (cursor.x < min)
		Menu_SlideItem( menu, -1 );
	if (cursor.x > max)
		Menu_SlideItem( menu, 1 );
}
#endif

/*
=================
UI_Think_MouseCursor
=================
*/
void UI_Think_MouseCursor (void)
{
	char * sound = NULL;
	menuframework_s *m = (menuframework_s *)cursor.menu;

	if (m_drawfunc == M_Main_Draw) // have to hack for main menu :p
	{
		UI_CheckMainMenuMouse();
		return;
	}
	if (m_drawfunc == M_Credits_MenuDraw) // have to hack for credits :p
	{
		if (cursor.buttonclicks[MOUSEBUTTON2])
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

/*	// clicking on the player model menu...
	if (m_drawfunc == PlayerConfig_MenuDraw)
		PlayerConfig_MouseClick();
	// clicking on the screen menu
	if (m_drawfunc == Options_Screen_MenuDraw)
		MenuCrosshair_MouseClick();*/

	if (!m)
		return;

	// Exit with double click 2nd mouse button

	if (cursor.menuitem)
	{
		// MOUSE1
		if (cursor.buttondown[MOUSEBUTTON1])
		{
			if (cursor.menuitemtype == MENUITEM_SLIDER && !cursor.buttonused[MOUSEBUTTON1])
			{
			//	Menu_DragSlideItem(m, cursor.menuitem);
				Menu_SlideItem(m, 1);
				sound = menu_move_sound;
				cursor.buttonused[MOUSEBUTTON1] = true;
			}
			else if (!cursor.buttonused[MOUSEBUTTON1] && cursor.buttonclicks[MOUSEBUTTON1])
			{
				if (cursor.menuitemtype == MENUITEM_ROTATE)
				{
				//	if (ui_item_rotate->value)					
					if (ui_item_rotate->integer)					
						Menu_SlideItem( m, -1 );
					else			
						Menu_SlideItem( m, 1 );

					sound = menu_move_sound;
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
		// MOUSE2
		if (cursor.buttondown[MOUSEBUTTON2] && cursor.buttonclicks[MOUSEBUTTON2])
		{
			if (cursor.menuitemtype == MENUITEM_SLIDER && !cursor.buttonused[MOUSEBUTTON2])
			{
			//	Menu_ClickSlideItem(m, cursor.menuitem);
				Menu_SlideItem(m, -1);
				sound = menu_move_sound;
				cursor.buttonused[MOUSEBUTTON2] = true;
			}
			else if (!cursor.buttonused[MOUSEBUTTON2])
			{
				if (cursor.menuitemtype == MENUITEM_ROTATE)
				{
				//	if (ui_item_rotate->value)					
					if (ui_item_rotate->integer)					
						Menu_SlideItem( m, 1 );
					else			
						Menu_SlideItem( m, -1 );

					sound = menu_move_sound;
					cursor.buttonused[MOUSEBUTTON2] = true;
				}
			}
		}
	}
	else if (!cursor.buttonused[MOUSEBUTTON2] && (cursor.buttonclicks[MOUSEBUTTON2] == 2)
		&& cursor.buttondown[MOUSEBUTTON2])
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

	// clicking on the player model menu...
	if (m_drawfunc == PlayerConfig_MenuDraw)
		PlayerConfig_MouseClick();
	// clicking on the screen menu
	if (m_drawfunc == Options_Screen_MenuDraw)
		MenuCrosshair_MouseClick();

	if ( sound )
		S_StartLocalSound ( sound );
}


/*
=================
UI_Draw_Cursor
=================
*/
#if 1
void UI_Draw_Cursor (void)
{
	int		w, h;
	float	ofs_x, ofs_y;
	float	scale = SCR_ScaledScreen(ui_cursor_scale->value); // 0.4
	char	*cur_img = UI_MOUSECURSOR_PIC;

	//get sizing vars
	R_DrawGetPicSize( &w, &h, UI_MOUSECURSOR_PIC );
	ofs_x = SCR_ScaledScreen(w) * ui_cursor_scale->value * 0.5;
	ofs_y = SCR_ScaledScreen(h) * ui_cursor_scale->value * 0.5;
	
	R_DrawScaledPic (cursor.x - ofs_x, cursor.y - ofs_y, scale, 1.0f, cur_img);
}
#else
void UI_Draw_Cursor (void)
{
	float	alpha = 1, scale = SCR_ScaledScreen(0.66);
	int		w, h;
	char	*overlay = NULL;
	char	*cur_img = NULL;

	if (m_drawfunc == M_Main_Draw)
	{
		if (MainMenuMouseHover)
		{
			if ((cursor.buttonused[0] && cursor.buttonclicks[0])
				|| (cursor.buttonused[1] && cursor.buttonclicks[1]))
			{
				cur_img = "/gfx/ui/cursors/m_cur_click.pcx";
				alpha = 0.85 + 0.15*sin(anglemod(cl.time*0.005));
			}
			else
			{
				cur_img = "/gfx/ui/cursors/m_cur_hover.pcx";
				alpha = 0.85 + 0.15*sin(anglemod(cl.time*0.005));
			}
		}
		else
			cur_img = "/gfx/ui/cursors/m_cur_main.pcx";
		overlay = "/gfx/ui/cursors/m_cur_over.pcx";
	}
	else
	{
		if (cursor.menuitem)
		{
			if (cursor.menuitemtype == MENUITEM_TEXT)
			{
				cur_img = "/gfx/ui/cursors/m_cur_text.pcx";
			}
			else
			{
				if ((cursor.buttonused[0] && cursor.buttonclicks[0])
					|| (cursor.buttonused[1] && cursor.buttonclicks[1]))
				{
					cur_img = "/gfx/ui/cursors/m_cur_click.pcx";
					alpha = 0.85 + 0.15*sin(anglemod(cl.time*0.005));
				}
				else
				{
					cur_img = "/gfx/ui/cursors/m_cur_hover.pcx";
					alpha = 0.85 + 0.15*sin(anglemod(cl.time*0.005));
				}
				overlay = "/gfx/ui/cursors/m_cur_over.pcx";
			}
		}
		else
		{
			cur_img = "/gfx/ui/cursors/m_cur_main.pcx";
			overlay = "/gfx/ui/cursors/m_cur_over.pcx";
		}
	}
	
	if (cur_img)
	{
		R_DrawGetPicSize( &w, &h, cur_img );
		R_DrawScaledPic( cursor.x - scale*w/2, cursor.y - scale*h/2, scale, alpha, cur_img);

		if (overlay) {
			R_DrawGetPicSize( &w, &h, overlay );
			R_DrawScaledPic( cursor.x - scale*w/2, cursor.y - scale*h/2, scale, 1, overlay);
		}
	}
}
#endif

/*void UI_Draw_Cursor (void)
{
	int w,h;

	//get sizing vars
	R_DrawGetPicSize( &w, &h, "m_mouse_cursor" );
	w = SCR_ScaledScreen(w)*0.5;
	h = SCR_ScaledScreen(h)*0.5;
	R_DrawStretchPic (cursor.x-w/2, cursor.y-h/2, w, h, "m_mouse_cursor", 1.0);
}*/
