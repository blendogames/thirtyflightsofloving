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

// menu_backend.c -- supporting code for menu widgets


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



static void  checkbox_DoEnter( menulist_s *s );
static void  checkbox_Draw( menulist_s *s );
static void	 checkbox_DoSlide( menulist_s *s, int dir );



#define RCOLUMN_OFFSET  MENU_FONT_SIZE*2	// was 16
#define LCOLUMN_OFFSET -MENU_FONT_SIZE*2	// was -16

extern viddef_t viddef;




#define VID_WIDTH viddef.width
#define VID_HEIGHT viddef.height

// added Psychospaz's menu mouse support
//======================================================
int stringLen (const char *string);
int stringLengthExtra (const char *string)
{
	unsigned i, j;
	char modifier;
	float len = strlen( string );

	for ( i = 0, j = 0; i < len; i++ )
	{
		modifier = string[i];
		if (modifier>128)
			modifier-=128;

		if ((string[i] == '^') || (i>0 && string[i-1] == '^'))
			j++;
	}

	return j;
}

char *unformattedString (const char *string)
{
	unsigned i;
	int len;
	char character;
	char *newstring = "";

	len = strlen (string);

	for ( i = 0; i < len; i++ )
	{
		character = string[i];

		if (character&128) character &= ~128;
		if (character == '^' && i < len)
		{
			i++;
			continue;
		}
		character = string[i];

		va("%s%c", newstring, character);
	}

	return newstring;
}

int mouseOverAlpha (menucommon_s *m)
{


	if (cursor.menuitem == m)
	{
		int alpha;

		//alpha = 170 + 85 * cos(anglemod(cl.time*0.01));
		alpha=255;

		//if (alpha>255) alpha = 255;
		//if (alpha<0) alpha = 0;

		return alpha;
	}
	else 
		return 160;

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
								a->generic.y + a->generic.parent->y, a->generic.name, alpha);
			/*Menu_DrawStringDark (
				SCR_ScaledVideo(a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET),
				SCR_ScaledVideo(a->generic.y + a->generic.parent->y),
				a->generic.name, alpha );*/
		else
			Menu_DrawString (a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET,
							a->generic.y + a->generic.parent->y, a->generic.name, alpha);
			/*Menu_DrawString (
				SCR_ScaledVideo(a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET),
				SCR_ScaledVideo(a->generic.y + a->generic.parent->y),
				a->generic.name, alpha );*/
	}
	else
	{
		if (a->generic.flags & QMF_GRAYED)
			Menu_DrawStringR2LDark (a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET,
									a->generic.y + a->generic.parent->y, a->generic.name, alpha);
			/*Menu_DrawStringR2LDark (
				SCR_ScaledVideo(a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET),
				SCR_ScaledVideo(a->generic.y + a->generic.parent->y),
				a->generic.name, alpha );*/
		else
			Menu_DrawStringR2L (a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET,
								a->generic.y + a->generic.parent->y, a->generic.name, alpha);
			/*Menu_DrawStringR2L (
				SCR_ScaledVideo(a->generic.x + a->generic.parent->x + LCOLUMN_OFFSET),
				SCR_ScaledVideo(a->generic.y + a->generic.parent->y),
				a->generic.name, alpha );*/
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
								f->generic.y + f->generic.parent->y, f->generic.name, 255);
		/*Menu_DrawStringR2LDark (
						SCR_ScaledVideo(f->generic.x + f->generic.parent->x + LCOLUMN_OFFSET),
						SCR_ScaledVideo(f->generic.y + f->generic.parent->y),
						f->generic.name, 255 );*/

	if (xtra = stringLengthExtra(f->buffer))
	{
		strncpy( tempbuffer, f->buffer + f->visible_offset, f->visible_length );
		offset = strlen(tempbuffer) - xtra;

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
		offset = strlen(tempbuffer);
	}

	SCR_DrawChar (f->generic.x + f->generic.parent->x + MENU_FONT_SIZE*2,
				f->generic.y + f->generic.parent->y - 4, ALIGN_CENTER, 18, 255,255,255,255, false, false);
	SCR_DrawChar (f->generic.x + f->generic.parent->x + MENU_FONT_SIZE*2,
				f->generic.y + f->generic.parent->y + 4, ALIGN_CENTER, 24, 255,255,255,255, false, false);
	SCR_DrawChar (f->generic.x + f->generic.parent->x + (3+f->visible_length)*MENU_FONT_SIZE,
				f->generic.y + f->generic.parent->y - 4, ALIGN_CENTER, 20,255,255,255,255, false, false);
	SCR_DrawChar (f->generic.x + f->generic.parent->x + (3+f->visible_length)*MENU_FONT_SIZE,
				f->generic.y + f->generic.parent->y + 4, ALIGN_CENTER, 26, 255,255,255,255, false, false);

	for (i = 0; i < f->visible_length; i++)
	{
		SCR_DrawChar (f->generic.x + f->generic.parent->x + (3+i)*MENU_FONT_SIZE,
					f->generic.y + f->generic.parent->y - 4, ALIGN_CENTER, 19, 255,255,255,255, false, false);
		SCR_DrawChar (f->generic.x + f->generic.parent->x + (3+i)*MENU_FONT_SIZE,
					f->generic.y + f->generic.parent->y + 4, ALIGN_CENTER, 25, 255,255,255,255, false, (i==(f->visible_length-1)));
	}

	// add cursor thingie
	if ( Menu_ItemAtCursor(f->generic.parent)==f  && ((int)(Sys_Milliseconds()/250))&1 )
		Com_sprintf(tempbuffer, sizeof(tempbuffer),	"%s%c", tempbuffer, 11);

	Menu_DrawString (f->generic.x + f->generic.parent->x + MENU_FONT_SIZE*3,
					f->generic.y + f->generic.parent->y, tempbuffer, alpha);
	/*Menu_DrawString (
				SCR_ScaledVideo(f->generic.x + MENU_FONT_SIZE*3 + f->generic.parent->x),
				SCR_ScaledVideo(f->generic.y-1 + f->generic.parent->y),
				tempbuffer, alpha );*/

	/*if ( Menu_ItemAtCursor( f->generic.parent ) == f )
	{
		int offset;

		if ( f->visible_offset )
			offset = f->visible_length;
		else
			offset = f->cursor;

		if ( ( (int)( Sys_Milliseconds() / 250 ) ) & 1 )
			R_DrawChar (SCR_ScaledVideo(f->generic.x + f->generic.parent->x + (offset + 3) * MENU_FONT_SIZE),
						SCR_ScaledVideo(f->generic.y + f->generic.parent->y),
						11, SCR_VideoScale(), 255,255,255,255, false);
		else
			R_DrawChar (SCR_ScaledVideo(f->generic.x + f->generic.parent->x + (offset + 3) * MENU_FONT_SIZE),
						SCR_ScaledVideo(f->generic.y + f->generic.parent->y),
						' ', SCR_VideoScale(), 255,255,255,255, false);
	}*/
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

	if (key > 127)
	{
		switch (key)
		{
		case K_DEL:
		default:
			return false;
		}
	}

	//
	// support pasting from the clipboard
	//
	if ( ( toupper( key ) == 'V' && keydown[K_CTRL] ) ||
		 ( ( ( key == K_INS ) || ( key == K_KP_INS ) ) && keydown[K_SHIFT] ) )
	{
		char *cbd;
		
		if ( ( cbd = Sys_GetClipboardData() ) != 0 )
		{
			strtok( cbd, "\n\r\b" );

			strncpy( f->buffer, cbd, f->length - 1 );
			f->cursor = strlen( f->buffer );
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

			if ( f->visible_offset )
			{
				f->visible_offset--;
			}
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
		if ( !isdigit( key ) && ( f->generic.flags & QMF_NUMBERSONLY ) )
			return false;

		if ( f->cursor < f->length )
		{
			f->buffer[f->cursor++] = key;
			f->buffer[f->cursor] = 0;

			if ( f->cursor > f->visible_length )
			{
				f->visible_offset++;
			}
		}
	}

	return true;
}

void Menulist_DoEnter (menulist_s *l)
{
	int start;

	start = l->generic.y / 10 + 1;

	l->curvalue = l->generic.parent->cursor - start;

	if ( l->generic.callback )
		l->generic.callback( l );
}

void MenuList_Draw (menulist_s *l)
{
	const char **n;
	int y = 0, alpha = mouseOverAlpha(&l->generic);

	Menu_DrawStringR2LDark (l->generic.x + l->generic.parent->x - 2*MENU_FONT_SIZE,
						l->generic.y + l->generic.parent->y, l->generic.name, alpha);
	/*Menu_DrawStringR2LDark(
		SCR_ScaledVideo(l->generic.x + l->generic.parent->x + -2*MENU_FONT_SIZE),
		SCR_ScaledVideo(l->generic.y + l->generic.parent->y), l->generic.name, alpha );*/

	n = l->itemnames;

	SCR_DrawFill (l->generic.parent->x + l->generic.x - 112, l->generic.parent->y + l->generic.y + (l->curvalue+1)*MENU_LINE_SIZE,
				128, MENU_LINE_SIZE, ALIGN_CENTER, 16);

	while (*n)
	{
		Menu_DrawStringR2LDark (l->generic.x + l->generic.parent->x + LCOLUMN_OFFSET,
							l->generic.y + l->generic.parent->y + y + MENU_LINE_SIZE, *n, alpha);
		/*Menu_DrawStringR2LDark(
			SCR_ScaledVideo(l->generic.x + l->generic.parent->x + LCOLUMN_OFFSET),
			SCR_ScaledVideo(l->generic.y + l->generic.parent->y + y + MENU_LINE_SIZE), *n, alpha );*/
		n++;
		y += MENU_LINE_SIZE;
	}
}

void Separator_Draw (menuseparator_s *s)
{
	int alpha = mouseOverAlpha(&s->generic);

	if (s->generic.name)
		Menu_DrawStringR2LDark (s->generic.x + s->generic.parent->x,
							s->generic.y + s->generic.parent->y, s->generic.name, alpha);
		/*Menu_DrawStringR2LDark(
			SCR_ScaledVideo(s->generic.x + s->generic.parent->x),
			SCR_ScaledVideo(s->generic.y + s->generic.parent->y),
			s->generic.name, alpha );*/
}

void Slider_DoSlide (menuslider_s *s, int dir)
{
	s->curvalue += dir;

	if ( s->curvalue > s->maxvalue )
		s->curvalue = s->maxvalue;
	else if ( s->curvalue < s->minvalue )
		s->curvalue = s->minvalue;

	if ( s->generic.callback )
		s->generic.callback( s );
}

#define SLIDER_RANGE 10

void Slider_Draw (menuslider_s *s)
{
	int	i, alpha = mouseOverAlpha(&s->generic);

	char val[8];

	Menu_DrawStringR2LDark (s->generic.x + s->generic.parent->x + LCOLUMN_OFFSET,
							s->generic.y + s->generic.parent->y, s->generic.name, alpha);
	/*Menu_DrawStringR2LDark( SCR_ScaledVideo(s->generic.x + s->generic.parent->x + LCOLUMN_OFFSET),
							SCR_ScaledVideo(s->generic.y + s->generic.parent->y), 
							s->generic.name, alpha );*/

	s->range = ( s->curvalue - s->minvalue ) / ( float ) ( s->maxvalue - s->minvalue );

	if ( s->range < 0)
		s->range = 0;
	if ( s->range > 1)
		s->range = 1;



	SCR_DrawChar (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET,
				s->generic.y + s->generic.parent->y,
				ALIGN_CENTER,
				128,
				255,255,255,alpha, false, false);

	for (i = 0; i < SLIDER_RANGE; i++)
		SCR_DrawChar (s->generic.x + s->generic.parent->x + (i+1)*MENU_FONT_SIZE + RCOLUMN_OFFSET,
				s->generic.y + s->generic.parent->y, ALIGN_CENTER, 129,
				255,255,255,alpha, false, false);
	
	SCR_DrawChar (s->generic.x + s->generic.parent->x + (i+1)*MENU_FONT_SIZE + RCOLUMN_OFFSET,
				s->generic.y + s->generic.parent->y, ALIGN_CENTER, 130,
				255,255,255,alpha, false, false);

	//the slider bobbin.
	SCR_DrawChar (s->generic.x + s->generic.parent->x + MENU_FONT_SIZE*((SLIDER_RANGE-1)*s->range+1) + RCOLUMN_OFFSET,
				s->generic.y + s->generic.parent->y, ALIGN_CENTER, 131,
				255,255,255,alpha, false, true);


	
	Com_sprintf(val, sizeof(val),	"%.0f%%", (s->curvalue*100.0)/s->maxvalue);

	if (val)
	{
		Menu_DrawString (
					s->generic.x + s->generic.parent->x + (i+1.5)*MENU_FONT_SIZE + RCOLUMN_OFFSET,
					s->generic.y + s->generic.parent->y,
					val,
					alpha);
	}

}

void SpinControl_DoEnter (menulist_s *s)
{
	if (!s->itemnames || !s->numitemnames)
		return;

	s->curvalue++;
	if ( s->itemnames[s->curvalue] == 0 )
		s->curvalue = 0;

	if ( s->generic.callback )
		s->generic.callback( s );
}

void SpinControl_DoSlide (menulist_s *s, int dir)
{
	if (!s->itemnames || !s->numitemnames)
		return;

	s->curvalue += dir;

	if (s->generic.flags & QMF_SKINLIST) // don't allow looping around for skin lists
	{
		if ( s->curvalue < 0 )
			s->curvalue = 0;
		else if ( s->itemnames[s->curvalue] == 0 )
			s->curvalue--;
	}
	else {
		if ( s->curvalue < 0 )
			s->curvalue = s->numitemnames-1; // was 0
		else if ( s->itemnames[s->curvalue] == 0 )
			s->curvalue = 0; // was --
	}

	if ( s->generic.callback )
		s->generic.callback( s );
}
 
void SpinControl_Draw (menulist_s *s)
{
	int alpha = mouseOverAlpha (&s->generic);
	char buffer[100];

	if (s->generic.name)
	{
		Menu_DrawStringR2LDark (s->generic.x + s->generic.parent->x + LCOLUMN_OFFSET,
								s->generic.y + s->generic.parent->y, s->generic.name, alpha);

	}
	if (!strchr(s->itemnames[s->curvalue], '\n'))
	{
		Menu_DrawString (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET + (6),
						s->generic.y + s->generic.parent->y, s->itemnames[s->curvalue], alpha);

	}
	else
	{
		strcpy(buffer, s->itemnames[s->curvalue]);
		*strchr(buffer, '\n') = 0;
		Menu_DrawString (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET,
						s->generic.y + s->generic.parent->y, buffer, alpha);

		strcpy(buffer, strchr( s->itemnames[s->curvalue], '\n' ) + 1 );
		Menu_DrawString (s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET,
						s->generic.y + s->generic.parent->y + MENU_LINE_SIZE, buffer, alpha);

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
			if (citem->type != MTYPE_SEPARATOR)
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
			citem = Menu_ItemAtCursor(m);
			if ( citem )
				if ( citem->type != MTYPE_SEPARATOR )
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
			citem = Menu_ItemAtCursor(m);
			if (citem)
				if (citem->type != MTYPE_SEPARATOR)
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
		case MTYPE_CHECKBOX:
			checkbox_Draw( ( menulist_s * ) menu->items[i] );
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

			x1 = menu->x + item->x + RCOLUMN_OFFSET; //+ 2 chars for space + cursor
			y1 = menu->y + item->y;
			w1 = 0;			h1 = MENU_FONT_SIZE;
			SCR_AdjustFrom640 (&x1, &y1, &w1, &h1, ALIGN_CENTER);
			min[0] = x1;	max[0] = x1+w1;
			min[1] = y1;	max[1] = y1+h1;
		//	max[0] = min[0] = SCR_ScaledVideo(menu->x + item->x + RCOLUMN_OFFSET); //+ 2 chars for space + cursor
		//	max[1] = min[1] = SCR_ScaledVideo(menu->y + item->y);
		//	max[1] += SCR_ScaledVideo(MENU_FONT_SIZE);

			switch (item->type)
			{
				case MTYPE_ACTION:
					{
						len = strlen(item->name);
						
						if (item->flags & QMF_LEFT_JUSTIFY)
						{
							min[0] += SCR_ScaledVideo(LCOLUMN_OFFSET*2);
							max[0] = min[0] + SCR_ScaledVideo(len*MENU_FONT_SIZE);
						}
						else
							min[0] -= SCR_ScaledVideo(len*MENU_FONT_SIZE + MENU_FONT_SIZE*3);

						type = MENUITEM_ACTION;
					}
					break;
				case MTYPE_SLIDER:
					{
						min[0] -= SCR_ScaledVideo(16);
						max[0] += SCR_ScaledVideo((SLIDER_RANGE + 4)*MENU_FONT_SIZE);
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
							len = strlen(item->name);
							min[0] -= SCR_ScaledVideo(len*MENU_FONT_SIZE - LCOLUMN_OFFSET*2);
						}

						//len = strlen(spin->itemnames[spin->curvalue]);

						if (spin->curvalue >= 0) //BC 6-15-2020 crash fix if curvalue is < 0
							len = strlen(spin->itemnames[spin->curvalue]);
						else
							len = 3;

						max[0] += SCR_ScaledVideo(len*MENU_FONT_SIZE);

						type = MENUITEM_ROTATE;
					}
					break;
				case MTYPE_CHECKBOX:
					{
						int len;
						menulist_s *spin = menu->items[i];


						//mouseover detection.
						if (item->flags & QMF_LEFT_JUSTIFY)
						{
							min[0] += SCR_ScaledVideo(LCOLUMN_OFFSET*2);

							if (item->name)
							{
								len = strlen(item->name);
								max[0] += SCR_ScaledVideo(len*MENU_FONT_SIZE);
							}
						}
						else
						{
							if (item->name)
							{
								len = strlen(item->name);
								min[0] -= SCR_ScaledVideo(len*MENU_FONT_SIZE - LCOLUMN_OFFSET*2);
							}

							len = strlen(spin->itemnames[spin->curvalue]);
							max[0] += SCR_ScaledVideo(len*MENU_FONT_SIZE);
						}

						type = MENUITEM_ROTATE;
					}
					break;

				case MTYPE_FIELD:
					{
						menufield_s *text = menu->items[i];

						len = text->visible_length + 2;

						max[0] += SCR_ScaledVideo(len*MENU_FONT_SIZE);
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

					for (j=0;j<MENU_CURSOR_BUTTON_MAX;j++)
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
			/*
			SCR_DrawChar (menu->x+item->x+item->cursor_offset-24, menu->y+item->y,
						ALIGN_CENTER, 12+((int)(Sys_Milliseconds()/250)&1),
						255,255,255,255, false, true);
						*/

			SCR_DrawChar (menu->x+item->x+item->cursor_offset-28 + (3*sin(anglemod(cl.time*0.01))),
						menu->y+item->y,
						ALIGN_CENTER,
						13,
						255,255,255,
						255,
						false, true);

				
		}
		else
		{
			/*SCR_DrawChar (menu->x+item->cursor_offset, menu->y+item->y,
						ALIGN_CENTER, 12+((int)(Sys_Milliseconds()/250)&1),
						255,255,255,255, false, true);*/


			SCR_DrawChar (menu->x+item->cursor_offset + (5*sin(anglemod(cl.time*0.01))),
				menu->y+item->y,
						ALIGN_CENTER,
						13,
						255,255,255,255, false, true);
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
		int l = strlen( string );
		//int maxrow = VID_HEIGHT / MENU_FONT_SIZE;
		//int maxcol = VID_WIDTH / MENU_FONT_SIZE;
		//int col = maxcol / 2 - SCR_ScaledVideo(l / 2);

		SCR_DrawFill2( 0, SCREEN_HEIGHT-(MENU_FONT_SIZE+2), SCREEN_WIDTH, MENU_FONT_SIZE+2, ALIGN_BOTTOM_STRETCH, 60,60,60,255 );
		SCR_DrawFill2( 0, SCREEN_HEIGHT-(MENU_FONT_SIZE+3), SCREEN_WIDTH, 1, ALIGN_BOTTOM_STRETCH, 0,0,0,255 );
		SCR_DrawString( SCREEN_WIDTH/2-(l/2)*MENU_FONT_SIZE, SCREEN_HEIGHT-(MENU_FONT_SIZE+1), ALIGN_BOTTOM, string, 255 );
	}
	else
		SCR_DrawFill2( 0, SCREEN_HEIGHT-(MENU_FONT_SIZE+3), SCREEN_WIDTH, MENU_FONT_SIZE+3, ALIGN_BOTTOM_STRETCH, 0,0,0,255 );
}

void Menu_DrawString (int x, int y, const char *string, int alpha)
{
	SCR_DrawString (x, y, ALIGN_CENTER, string, alpha);
}

void Menu_DrawStringLeft (int x, int y, const char *string, int alpha)
{
	SCR_DrawString (x, y, ALIGN_LEFT, string, alpha);
}


void Menu_DrawStringDark (int x, int y, const char *string, int alpha)
{
	char	newstring[1024];

	Com_sprintf (newstring, sizeof(newstring), S_COLOR_ALT"%s", string);
	SCR_DrawString (x, y, ALIGN_CENTER, newstring, alpha);
}

void Menu_DrawStringR2L (int x, int y, const char *string, int alpha)
{
	x -= stringLen(string)*MENU_FONT_SIZE;
	SCR_DrawString (x, y, ALIGN_CENTER, string, alpha);
}

void Menu_DrawStringR2LDark (int x, int y, const char *string, int alpha)
{
	char	newstring[1024];

	Com_sprintf (newstring, sizeof(newstring), S_COLOR_ALT"%s", string);
	x -= stringLen(string)*MENU_FONT_SIZE;
	SCR_DrawString (x, y, ALIGN_CENTER, newstring, alpha);
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
		case MTYPE_CHECKBOX:
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
		case MTYPE_CHECKBOX:
			checkbox_DoSlide( ( menulist_s * ) item, dir );
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


#if 0
/*
================
Menu_DrawCharacter

Draws one solid graphics character.
cx and cy are in 320*240 coordinates, and will be centered on
higher res screens.
================
*/
void Menu_DrawCharacter (int cx, int cy, int num,
		int red, int green, int blue, int alpha, qboolean italic)
{
return;

	if (SCR_VideoScale() > 1)
		R_DrawChar (SCR_ScaledVideo( cx + ((SCREEN_WIDTH-320)/2) ),
					SCR_ScaledVideo( cy + ((SCREEN_HEIGHT-240)/2) ),
					num, SCR_VideoScale(), red, green, blue, alpha, italic, true);
	else
		R_DrawChar (cx + ((viddef.width - 320)>>1), cy + ((viddef.height - 240)>>1 ),
					num, SCR_VideoScale(), red, green, blue, alpha, italic, true);
}

/*
================
Menu_Print
================
*/
void Menu_Print (int cx, int cy, char *str)
{
	int red=0, green=255, blue=0;
	TextColor ((int)alt_text_color->value, &red, &green, &blue);
	while (*str)
	{
		Menu_DrawCharacter (cx, cy, (*str)+128, red,green,blue,255, false);
		str++;
		cx += MENU_FONT_SIZE;
	}
}

/*
================
Menu_PrintWhite
================
*/
void Menu_PrintWhite (int cx, int cy, char *str)
{
	while (*str)
	{
		Menu_DrawCharacter (cx, cy, *str, 255,255,255,255, false);
		str++;
		cx += MENU_FONT_SIZE;
	}
}

/*
================
Menu_DrawPic
================
*/
void Menu_DrawPic (int x, int y, char *pic)
{
	R_DrawPic (x + ((viddef.width - 320)>>1), y + ((viddef.height - 240)>>1), pic);
}
#endif


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
	SCR_DrawChar (cx, cy, ALIGN_CENTER, 1, 255,255,255,255, false, false);
	for (n = 0; n < lines; n++) {
		cy += MENU_FONT_SIZE;
		SCR_DrawChar (cx, cy, ALIGN_CENTER, 4, 255,255,255,255, false, false);
	}
	SCR_DrawChar (cx, cy+MENU_FONT_SIZE, ALIGN_CENTER, 7, 255,255,255,255, false, false);

	// draw middle
	cx += MENU_FONT_SIZE;
	while (width > 0)
	{
		cy = y;
		SCR_DrawChar (cx, cy, ALIGN_CENTER, 2, 255,255,255,255, false, false);
		for (n = 0; n < lines; n++) {
			cy += MENU_FONT_SIZE;
			SCR_DrawChar (cx, cy, ALIGN_CENTER, 5, 255,255,255,255, false, false);
		}
		SCR_DrawChar (cx, cy+MENU_FONT_SIZE, ALIGN_CENTER, 8, 255,255,255,255, false, false);
		width -= 1;
		cx += MENU_FONT_SIZE;
	}

	// draw right side
	cy = y;
	SCR_DrawChar (cx, cy, ALIGN_CENTER, 3, 255,255,255,255, false, false);
	for (n = 0; n < lines; n++) {
		cy += MENU_FONT_SIZE;
		SCR_DrawChar (cx, cy, ALIGN_CENTER, 6, 255,255,255,255, false, false);
	}
	SCR_DrawChar (cx, cy+MENU_FONT_SIZE, ALIGN_CENTER, 9, 255,255,255,255, false, true);
}


/*
=================
Menu_Banner
=================
*/
void Menu_Banner (char *name)
{
	

	int w, h;

	//return;

	R_DrawGetPicSize (&w, &h, name );

	w *= 0.6;
	h *= 0.6;

	SCR_DrawPic (SCREEN_WIDTH/2-w/2, SCREEN_HEIGHT/2-150, w, h, ALIGN_CENTER, name, 1.0);
}





void checkbox_DoEnter( menulist_s *s )
{
	s->curvalue++;
	if ( s->itemnames[s->curvalue] == 0 )
		s->curvalue = 0;

	if ( s->generic.callback )
		s->generic.callback( s );
}

void checkbox_DoSlide( menulist_s *s, int dir )
{
	s->curvalue += dir;

	/*if ( s->curvalue < 0 )
		s->curvalue = 0;
	else if ( s->itemnames[s->curvalue] == 0 )
		s->curvalue--;*/


	if ( s->curvalue > 1 )
		s->curvalue = 0;
	else if ( s->curvalue < 0 )
		s->curvalue = 1;


	if ( s->generic.callback )
		s->generic.callback( s );
}

void checkbox_Draw( menulist_s *s  )
{
	int alpha = mouseOverAlpha(&s->generic);	
	char buffer[100];

	if ( s->generic.name )
	{

		if (s->generic.flags & QMF_LEFT_JUSTIFY)
		{
			Menu_DrawString (s->generic.x + s->generic.parent->x + LCOLUMN_OFFSET + 13,
							s->generic.y + s->generic.parent->y, s->generic.name, alpha);
		}
		else
		{
			Menu_DrawStringR2LDark (s->generic.x + s->generic.parent->x + LCOLUMN_OFFSET,
				s->generic.y + s->generic.parent->y,
				s->generic.name,
				alpha);
		}
	}

	if ( s->curvalue > 0 )
	{
		strcpy(buffer, "m_checkbox_on");
	}
	else
		strcpy(buffer, "m_checkbox_off");

	if (buffer)
	{
		int yoffset=0;

		if (viddef.height==1024)
		{
			yoffset = MENU_FONT_SIZE*2;
		}

		if (s->generic.flags & QMF_LEFT_JUSTIFY)
		{
			R_DrawStretchPic (
				(viddef.width / 2) + SCR_ScaledVideo(-38),
				SCR_ScaledVideo(s->generic.y + s->generic.parent->y+yoffset),
				SCR_ScaledVideo(MENU_FONT_SIZE), SCR_ScaledVideo(MENU_FONT_SIZE), buffer,
				alpha / 255.0);
		}
		else
		{
			R_DrawStretchPic (
				(viddef.width / 2) + SCR_ScaledVideo(24),
				SCR_ScaledVideo(s->generic.y + s->generic.parent->y+yoffset),
				SCR_ScaledVideo(MENU_FONT_SIZE), SCR_ScaledVideo(MENU_FONT_SIZE), buffer,
				alpha / 255.0);
		}
	}


}
