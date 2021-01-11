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

// cl_string.c
// string drawing and formatting functions

#include "client.h"


/*
================
CL_StringSetParams
================
*/
qboolean CL_StringSetParams (char modifier, int *red, int *green, int *blue, int *bold, int *shadow, int *italic, int *reset)
{
	// sanity check
	if (!red || !green || !blue || !bold || !shadow || !italic || !reset)
		return false;

	if (!alt_text_color)
		alt_text_color = Cvar_Get ("alt_text_color", "2", CVAR_ARCHIVE);

	switch (modifier)
	{
		case 'R':
		case 'r':
			*reset = true;
			return true;
		case 'B':
		case 'b':
			if (*bold) 
				*bold = false;
			else 
				*bold = true;
			return true;
		case 'S':
		case 's':
			if (*shadow) 
				*shadow = false; 
			else 
				*shadow = true;
			return true;
		case 'I':
		case 'i':
			if (*italic) 
				*italic = false; 
			else 
				*italic = true;
			return true;
		case COLOR_RED:
		case COLOR_GREEN:
		case COLOR_YELLOW:
		case COLOR_BLUE:
		case COLOR_CYAN:
		case COLOR_MAGENTA:
		case COLOR_WHITE:
		case COLOR_BLACK:
		case COLOR_ORANGE:
		case COLOR_GRAY:
			CL_TextColor (atoi(&modifier), red, green, blue);
			return true;
		case 'A':	// alt text color
		case 'a':
		//	CL_TextColor ((int)alt_text_color->value, red, green, blue);
			CL_TextColor (alt_text_color->integer, red, green, blue);
			return true;
	}
	
	return false;
}


/*
================
CL_DrawStringGeneric
================
*/
void CL_DrawStringGeneric (int x, int y, const char *string, fontslot_t font, int alpha, int fontSize, textscaletype_t scaleType, qboolean altBit)
{
	unsigned	i, j;
	int			len, red, green, blue, italic, shadow, bold, reset;
	float		textSize, textScale;
	byte		modifier, character;
	qboolean	modified=false, flushChar;

	// defaults
	red = 255;
	green = 255;
	blue = 255;
	italic = false;
	shadow = false;
	bold = false;

	len = (int)strlen( string );
	for ( i = 0, j = 0; i < len; i++ )
	{
		modifier = (byte)string[i];
		if (modifier & 128) modifier &= ~128;

		if (modifier == '^' && i < len)
		{
			i++;

			reset = false;
			modifier = (byte)string[i];
			if (modifier & 128) modifier &= ~128;

			if (modifier != '^')
			{
				modified = CL_StringSetParams (modifier, &red, &green, &blue, &bold, &shadow, &italic, &reset);

				if ( (modifier != 'r') && (modifier != 'R') )	// fix false reset flag
					reset = false;

				if (reset)
				{
					red = 255;
					green = 255;
					blue = 255;
					italic = false;
					shadow = false;
					bold = false;
				}
				if (modified)
					continue;
				else
					i--;
			}
		}
		j++;

		character = (byte)string[i];
	//	if (bold && character < 128)
	//		character += 128;
	//	else if (bold && character > 128)
	//		character -= 128;
		if ( (bold) || (!modified && altBit) ) {
			character ^= 128;
		}

		// hack for alternate text color
		if (!modified)
		{
			if (character & 128) {
			//	CL_TextColor ((int)alt_text_color->value, &red, &green, &blue);
				CL_TextColor (alt_text_color->integer, &red, &green, &blue);
				if ( (red != 255) || (green != 255) || (blue != 255) )
					character &= ~128;
			}
			else
				red = green = blue = 255;
		}

		switch (scaleType)
		{
		case SCALETYPE_MENU:
			textSize = SCR_ScaledScreen(fontSize);	// MENU_FONT_SIZE
			textScale = SCR_GetScreenScale()*((float)fontSize/(float)MENU_FONT_SIZE);
			break;
		case SCALETYPE_HUD:
			textSize = SCR_ScaledHud(fontSize);		// HUD_FONT_SIZE
			textScale = SCR_GetHudScale()*((float)fontSize/(float)HUD_FONT_SIZE);
			break;
		case SCALETYPE_CONSOLE:
		default:
			textSize = fontSize;	// FONT_SIZE;
			textScale = (float)fontSize/8.0f;	// FONT_SIZE/8.0f
			break;
		}

		flushChar = (i==(len-1));

		if (shadow)
			R_DrawChar( ( x + (j-1)*textSize+textSize/4 ), y+(textSize/8), 
				character, font, textScale, 0, 0, 0, alpha, italic, false );
		R_DrawChar( ( x + (j-1)*textSize ), y,
			character, font, textScale, red, green, blue, alpha, italic, flushChar );
	}
}
