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

// cl_hud.c -- parsing for HUD and client inventory screen

#include "client.h"
#include "../ui/ui_local.h"

#define	ICON_WIDTH	24
#define	ICON_HEIGHT	24
#define	ICON_SPACE	8

#define STAT_MINUS		10	// num frame for '-' stats digit
char		*sb_nums[2][11] = 
{
	{"num_0", "num_1", "num_2", "num_3", "num_4", "num_5",
	"num_6", "num_7", "num_8", "num_9", "num_minus"},
	{"anum_0", "anum_1", "anum_2", "anum_3", "anum_4", "anum_5",
	"anum_6", "anum_7", "anum_8", "anum_9", "anum_minus"}
};

//===============================================================================

/*
================
Hud_DrawString
================
*/
void Hud_DrawString (int x, int y, const char *string, int alpha, qboolean isStatusBar)
{
	CL_DrawStringGeneric (x, y, string, FONT_SCREEN, alpha, HUD_FONT_SIZE, (isStatusBar) ? SCALETYPE_HUD : SCALETYPE_MENU, false);
}


/*
================
Hud_DrawStringAlt
================
*/
void Hud_DrawStringAlt (int x, int y, const char *string, int alpha, qboolean isStatusBar)
{
	int		i, len;
	char	highString[1024];

//	Com_sprintf (highString, sizeof(highString), S_COLOR_ALT"%s", string);
	Com_sprintf (highString, sizeof(highString), "%s", string);
	len = (int)strlen(highString);
	for (i=0; i<len; i++) {
		highString[i] ^= 128;
	}
	CL_DrawStringGeneric (x, y, highString, FONT_SCREEN, alpha, HUD_FONT_SIZE, (isStatusBar) ? SCALETYPE_HUD : SCALETYPE_MENU, false);
//	CL_DrawStringGeneric (x, y, string, alpha, HUD_FONT_SIZE, (isStatusBar) ? SCALETYPE_HUD : SCALETYPE_MENU, true);
}


/*
================
Hud_DrawStringFromCharsPic
================
*/
void Hud_DrawStringFromCharsPic (float x, float y, float w, float h, vec2_t offset, float width, scralign_t align, char *string, color_t color, char *pic, int flags)
{
	vec2_t	scaledOffset;

	Vector2Copy (offset, scaledOffset);
	SCR_ScaleCoords (&x, &y, &w, &h, align);
//	SCR_ScaleCoords (NULL, NULL, &offsetX, NULL, align);
//	SCR_ScaleCoords (NULL, NULL, NULL, &offsetY, align);
	SCR_ScaleCoords (NULL, NULL, &scaledOffset[0], &scaledOffset[1], align);
	SCR_ScaleCoords (NULL, NULL, &width, NULL, align);

	CL_DrawStringFromCharsPic (x, y, w, h, scaledOffset, width, string, color, pic, flags);
}

/*
===============================================================

LAYOUT HUD CODE

===============================================================
*/

/*
================
CL_SizeLayoutString

Allow embedded \n in the string
================
*/
void CL_SizeLayoutString (char *string, int *w, int *h, qboolean isStatusBar)
{
	int		lines, width, current;
	float	(*scaleForScreen)(float in);

	// Get our scaling function
	if (isStatusBar)
		scaleForScreen = SCR_ScaledHud;
	else
		scaleForScreen = SCR_ScaledScreen;

	lines = 1;
	width = 0;

	current = 0;
	while (*string)
	{
		if (*string == '\n')
		{
			lines++;
			current = 0;
		}
		else
		{
			current++;
			if (current > width)
				width = current;
		}
		string++;
	}

//	*w = width * scaledHud(8);
//	*h = lines * scaledHud(8);
	*w = width * scaleForScreen(8);
	*h = lines * scaleForScreen(8);
}

/*
================
CL_DrawLayoutString
================
*/
void CL_DrawLayoutString (char *string, int x, int y, int centerwidth, int xor, qboolean isStatusBar)
{
	int		margin;
	int		width, visibleWidth;
	int		i, len;
	char	line[1024];
	float	(*scaleForScreen)(float in);

	// Get our scaling function
	if (isStatusBar)
		scaleForScreen = SCR_ScaledHud;
	else
		scaleForScreen = SCR_ScaledScreen;

	margin = x;

//	len = strlen(string);

	while (*string)
	{
		// scan out one line of text from the string
		width = 0;
		while (*string && *string != '\n')
			line[width++] = *string++;

		line[width] = 0;
		visibleWidth = stringLen(line);

		if (centerwidth)
		//	x = margin + (centerwidth - width*scaledHud(8))/2;
		//	x = margin + (centerwidth - width*scaleForScreen(8))/2;
			x = margin + (centerwidth - visibleWidth*scaleForScreen(8))/2;
		else
			x = margin;


		if (xor)
		{	// Knightmare- text color hack
		//	Com_sprintf (line, sizeof(line), S_COLOR_ALT"%s", line);
			len = (int)strlen(line);
			for (i=0; i<len; i++) {
				line[i] ^= xor;
			}
		}
		Hud_DrawString (x, y, line, 255, isStatusBar);

		if (*string)
		{
			string++;	// skip the \n
			x = margin;
		//	y += scaledHud(8);
			y += scaleForScreen(8);
		}
	}
}


/*
==============
CL_DrawLayoutField

Draws HUD number displays
==============
*/
void CL_DrawLayoutField (int x, int y, int color, int width, int value, qboolean flash, qboolean isStatusBar)
{
	char			num[16], *ptr;
	int				l, frame;
	float			digitWidth, digitOffset, fieldScale;
	float			flash_x, flashWidth;
	vec4_t			picColor;
	drawStruct_t	ds;
	float			(*scaleForScreen)(float in);
	float			(*getScreenScale)(void);

	if (width < 1)
		return;

	// Get our scaling functions
	if (isStatusBar) {
		scaleForScreen = SCR_ScaledHud;
		getScreenScale = SCR_GetHudScale;
	}
	else {
		scaleForScreen = SCR_ScaledScreen;
		getScreenScale = SCR_GetScreenScale;
	}

	// draw number string
	fieldScale = getScreenScale();
	if (width > 5)
		width = 5;

	Com_sprintf (num, sizeof(num), "%i", value);
	l = (int)strlen(num);
	if (l > width)
	{
	//	if (scr_hudsqueezedigits->value) {
		if (scr_hudsqueezedigits->integer) {
			l = min(l, width+2);
			fieldScale =  (1.0 - ((1.0 - (float)width/(float)l) * 0.5)) * getScreenScale();
		}
		else
			l = width;
	}
	digitWidth = fieldScale*(float)CHAR_WIDTH;
	digitOffset = width*scaleForScreen(CHAR_WIDTH) - l*digitWidth;
//	x += 2 + scaleForScreen(CHAR_WIDTH)*(width - l);
	x += 2 + digitOffset;
	flashWidth = l * digitWidth;
	flash_x = x;
	Vector4Set (picColor, 1.0f, 1.0f, 1.0f, scr_hudalpha->value);

	memset (&ds, 0, sizeof(drawStruct_t));
	ds.y = y;	ds.h = scaleForScreen(ICON_HEIGHT);
	Vector2Copy (vec2_origin, ds.offset);
	Vector4Copy (picColor, ds.color);

	if (flash){
		ds.pic = "field_3";
		ds.x = flash_x;	ds.w = flashWidth;	
		R_DrawPic (ds);
	//	R_DrawStretchPic (flash_x, y, flashWidth, scaleForScreen(ICON_HEIGHT), "field_3", scr_hudalpha->value);
	}

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		ds.pic = sb_nums[color][frame];
		ds.x = x;	ds.w = digitWidth;	
		R_DrawPic (ds);
	//	R_DrawStretchPic (x, y, digitWidth, scaleForScreen(ICON_HEIGHT), sb_nums[color][frame], scr_hudalpha->value);
		x += digitWidth;
		ptr++;
		l--;
	}
}


/*
================
CL_ExecuteLayoutString 
================
*/
void CL_ExecuteLayoutString (char *s, qboolean isStatusBar)
{
	int		x, y;
	int		value, stat, width, index, cs_images, max_images;
	char	*token;
	clientinfo_t	*ci;
	float			(*scaleForScreen)(float in);
	float			(*getScreenScale)(void);
	float			scrLeft, scrWidth;

	if (cls.state != ca_active || !cl.refresh_prepped)
		return;

	if (!s[0])
		return;

	// Knightmare- hack for connected to server using old protocol
	// Changed config strings require different parsing
	if ( LegacyProtocol() ) {
		cs_images = OLD_CS_IMAGES;
		max_images = OLD_MAX_IMAGES;
	}
	else {
		cs_images = CS_IMAGES;
		max_images = MAX_IMAGES;
	}

	// Get our scaling functions
	if (isStatusBar) {
		scaleForScreen = SCR_ScaledHud;
		getScreenScale = SCR_GetHudScale;
	}
	else {
		scaleForScreen = SCR_ScaledScreen;
		getScreenScale = SCR_GetScreenScale;
	}

	SCR_InitHudScale ();
	x = 0;
	y = 0;
	width = 3;

	scrLeft = 0;
	scrWidth = SCREEN_WIDTH;
	SCR_ScaleCoords (&scrLeft, NULL, &scrWidth, NULL, ALIGN_STRETCH);

	while (s)
	{
		token = COM_Parse (&s);
		if (!strcmp(token, "xl"))
		{
			token = COM_Parse (&s);
		//	x = scaleForScreen(atoi(token));
			x = (int)scrLeft + scaleForScreen(atoi(token));
			continue;
		}
		if (!strcmp(token, "xr"))
		{
			token = COM_Parse (&s);
		//	x = viddef.width + scaleForScreen(atoi(token));
			x = (int)(scrLeft + scrWidth) + scaleForScreen(atoi(token));
			continue;
		}
		if (!strcmp(token, "xv"))
		{
			token = COM_Parse (&s);
			x = viddef.width/2 - scaleForScreen(160) + scaleForScreen(atoi(token));
			continue;
		}
		if (!strcmp(token, "yt"))
		{
			token = COM_Parse (&s);
			y = scaleForScreen(atoi(token));
			continue;
		}
		if (!strcmp(token, "yb"))
		{
			token = COM_Parse (&s);
			y = viddef.height + scaleForScreen(atoi(token));
			continue;
		}
		if (!strcmp(token, "yv"))
		{
			token = COM_Parse (&s);
			y = viddef.height/2 - scaleForScreen(120) + scaleForScreen(atoi(token));
			continue;
		}

		if (!strcmp(token, "pic"))
		{	// draw a pic from a stat number
			token = COM_Parse (&s);
		//	value = cl.frame.playerstate.stats[atoi(token)];
			stat = atoi(token);
			if (stat < 0 || stat >= MAX_STATS)	// check bounds on stat
				Com_Error (ERR_DROP, "Bad pic stat index");
			value = cl.frame.playerstate.stats[stat];
			if (value >= max_images) // Knightmare- don't bomb out
			//	Com_Error (ERR_DROP, "Pic >= MAX_IMAGES");
			{
				Com_Printf (S_COLOR_YELLOW"WARNING: Pic >= MAX_IMAGES\n");
				value = max_images-1;
			}
			if (cl.configstrings[cs_images+value])
			{
			//	R_DrawScaledPic (x, y, getScreenScale(), scr_hudalpha->value, cl.configstrings[cs_images+value]);
				SCR_DrawLegacyPic (x, y, getScreenScale(), cl.configstrings[cs_images+value], scr_hudalpha->value);
			}
			continue;
		}

		if (!strcmp(token, "client"))
		{	// draw a deathmatch client block
			int		score, ping, time;

			token = COM_Parse (&s);
			x = viddef.width/2 - scaleForScreen(160) + scaleForScreen(atoi(token));
			token = COM_Parse (&s);
			y = viddef.height/2 - scaleForScreen(120) + scaleForScreen(atoi(token));

			token = COM_Parse (&s);
			value = atoi(token);
			if (value >= MAX_CLIENTS)
				Com_Error (ERR_DROP, "client (%d) >= MAX_CLIENTS", value);
			else if (value < 0)
				Com_Error (ERR_DROP, "client (%d) < 0", value);
			ci = &cl.clientinfo[value];

			token = COM_Parse (&s);
			score = atoi(token);

			token = COM_Parse (&s);
			ping = atoi(token);

			token = COM_Parse (&s);
			time = atoi(token);

			Hud_DrawStringAlt (x+scaleForScreen(32), y, va(S_COLOR_ALT"%s", ci->name), 255, isStatusBar);
			Hud_DrawString (x+scaleForScreen(32), y+scaleForScreen(8),  "Score: ", 255, isStatusBar);
			Hud_DrawStringAlt (x+scaleForScreen(32+7*8), y+scaleForScreen(8),  va(S_COLOR_ALT"%i", score), 255, isStatusBar);
			Hud_DrawString (x+scaleForScreen(32), y+scaleForScreen(16), va("Ping:  %i", ping), 255, isStatusBar);
			Hud_DrawString (x+scaleForScreen(32), y+scaleForScreen(24), va("Time:  %i", time), 255, isStatusBar);

			if (!ci->icon)
				ci = &cl.baseclientinfo;
		//	R_DrawScaledPic (x, y, getScreenScale(), scr_hudalpha->value,  ci->iconname);
			SCR_DrawLegacyPic (x, y, getScreenScale(), ci->iconname, scr_hudalpha->value);
			continue;
		}

		if (!strcmp(token, "ctf"))
		{	// draw a ctf client block
			int		score, ping;
			char	block[80];

			token = COM_Parse (&s);
			x = viddef.width/2 - scaleForScreen(160) + scaleForScreen(atoi(token));
			token = COM_Parse (&s);
			y = viddef.height/2 - scaleForScreen(120) + scaleForScreen(atoi(token));

			token = COM_Parse (&s);
			value = atoi(token);
			if (value >= MAX_CLIENTS)
				Com_Error (ERR_DROP, "client (%d) >= MAX_CLIENTS", value);
			else if (value < 0)
				Com_Error (ERR_DROP, "client (%d) < 0", value);
			ci = &cl.clientinfo[value];

			token = COM_Parse (&s);
			score = atoi(token);

			token = COM_Parse (&s);
			ping = atoi(token);
			if (ping > 999)
				ping = 999;

			sprintf(block, "%3d %3d %-12.12s", score, ping, ci->name);

			if (value == cl.playernum)
				Hud_DrawStringAlt (x, y, block, 255, isStatusBar);
			else
				Hud_DrawString (x, y, block, 255, isStatusBar);
			continue;
		}
		
		if (!strcmp(token, "3tctf")) // Knightmare- 3Team CTF block
		{	// draw a 3Team CTF client block
			int		score, ping;
			char	block[80];

			token = COM_Parse (&s);
			x = viddef.width/2 - scaleForScreen(160) + scaleForScreen(atoi(token));
			token = COM_Parse (&s);
			y = viddef.height/2 - scaleForScreen(120) + scaleForScreen(atoi(token));

			token = COM_Parse (&s);
			value = atoi(token);
			if (value >= MAX_CLIENTS)
				Com_Error (ERR_DROP, "client (%d) >= MAX_CLIENTS", value);
			else if (value < 0)
				Com_Error (ERR_DROP, "client (%d) < 0", value);
			ci = &cl.clientinfo[value];

			token = COM_Parse (&s);
			score = atoi(token);

			token = COM_Parse (&s);
			ping = atoi(token);
			if (ping > 999)
				ping = 999;
			// double spaced before player name for 2 flag icons
			sprintf(block, "%3d %3d  %-12.12s", score, ping, ci->name);

			if (value == cl.playernum)
				Hud_DrawStringAlt (x, y, block, 255, isStatusBar);
			else
				Hud_DrawString (x, y, block, 255, isStatusBar);
			continue;
		}

		if (!strcmp(token, "picn"))
		{	// draw a pic from a name
			token = COM_Parse (&s);
		//	R_DrawScaledPic (x, y, getScreenScale(), scr_hudalpha->value, token);
			SCR_DrawLegacyPic (x, y, getScreenScale(), token, scr_hudalpha->value);
			continue;
		}

		if (!strcmp(token, "num"))
		{	// draw a number
			token = COM_Parse (&s);
			width = atoi(token);
			token = COM_Parse (&s);
		//	value = cl.frame.playerstate.stats[atoi(token)];
			stat = atoi(token);
			if (stat < 0 || stat >= MAX_STATS)	// check bounds on stat
				Com_Error (ERR_DROP, "Bad num stat index");
			value = cl.frame.playerstate.stats[stat];
			CL_DrawLayoutField (x, y, 0, width, value, false, isStatusBar);
			continue;
		}

		if (!strcmp(token, "hnum"))
		{	// health number
			int		color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_HEALTH];
			if (value > 25)
				color = 0;	// green
			else if (value > 0)
				color = (cl.frame.serverframe>>2) & 1;		// flash
			else
				color = 1;

		//	if (cl.frame.playerstate.stats[STAT_FLASHES] & 1)
		//		R_DrawScaledPic (x, y, getScreenScale(), scr_hudalpha->value, "field_3");

			CL_DrawLayoutField (x, y, color, width, value, (cl.frame.playerstate.stats[STAT_FLASHES] & 1), isStatusBar);
			continue;
		}

		if (!strcmp(token, "anum"))
		{	// ammo number
			int		color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_AMMO];
			if (value > 5)
				color = 0;	// green
			else if (value >= 0)
				color = (cl.frame.serverframe>>2) & 1;		// flash
			else
				continue;	// negative number = don't show

		//	if (cl.frame.playerstate.stats[STAT_FLASHES] & 4)
		//		R_DrawScaledPic (x, y, getScreenScale(), scr_hudalpha->value, "field_3");

			CL_DrawLayoutField (x, y, color, width, value, (cl.frame.playerstate.stats[STAT_FLASHES] & 4), isStatusBar);
			continue;
		}

		if (!strcmp(token, "rnum"))
		{	// armor number
			int		color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_ARMOR];
			if (value < 1)
				continue;

			color = 0;	// green

		//	if (cl.frame.playerstate.stats[STAT_FLASHES] & 2)
		//		R_DrawScaledPic (x, y, getScreenScale(), scr_hudalpha->value, "field_3");

			CL_DrawLayoutField (x, y, color, width, value, (cl.frame.playerstate.stats[STAT_FLASHES] & 2), isStatusBar);
			continue;
		}


		if (!strcmp(token, "stat_string"))
		{
			token = COM_Parse (&s);
			stat = atoi(token);
			if (stat < 0 || stat >= MAX_STATS)	// MAX_CONFIGSTRINGS
				Com_Error (ERR_DROP, "Bad stat_string stat index");
			index = cl.frame.playerstate.stats[stat];
			if (index < 0 || index >= MAX_CONFIGSTRINGS)
				Com_Error (ERR_DROP, "Bad stat_string configstring index");
			Hud_DrawString (x, y, cl.configstrings[index], 255, isStatusBar);
			continue;
		}

		if (!strcmp(token, "cstring"))
		{
			token = COM_Parse (&s);
			CL_DrawLayoutString (token, x, y, scaleForScreen(320), 0, isStatusBar);
			continue;
		}

		if (!strcmp(token, "string"))
		{
			token = COM_Parse (&s);
			Hud_DrawString (x, y, token, 255, isStatusBar);
			continue;
		}

		if (!strcmp(token, "cstring2"))
		{
			token = COM_Parse (&s);
			CL_DrawLayoutString (token, x, y, scaleForScreen(320), 0x80, isStatusBar);
			continue;
		}

		if (!strcmp(token, "string2"))
		{
			token = COM_Parse (&s);
			Hud_DrawStringAlt (x, y, token, 255, isStatusBar);
			continue;
		}

		if (!strcmp(token, "if"))
		{	// draw a number
			token = COM_Parse (&s);
		//	value = cl.frame.playerstate.stats[atoi(token)];
			stat = atoi(token);
			if (stat < 0 || stat >= MAX_STATS)	// check bounds on stat
				Com_Error (ERR_DROP, "Bad if stat index");
			value = cl.frame.playerstate.stats[stat];
			if (!value)
			{	// skip to endif
				while (s && strcmp(token, "endif") )
				{
					token = COM_Parse (&s);
				}
			}

			continue;
		}

		// ifeq <stat> <value>
		// ifbit <stat> <value>

		// Knightmare- added more commands
		if (!strcmp(token, "ifeq"))
		{
			token = COM_Parse (&s);
			stat = atoi(token);
			if (stat < 0 || stat >= MAX_STATS)
				Com_Error (ERR_DROP, "Bad ifeq stat index");
			token = COM_Parse (&s);
			value = atoi(token);
			if (cl.frame.playerstate.stats[stat] != value)
			{	// skip to endif
				while (s && strcmp(token, "endif") )
				{
					token = COM_Parse (&s);
				}			
			}
		}

		if (!strcmp(token, "ifneq"))
		{
			token = COM_Parse (&s);
			stat = atoi(token);
			if (stat < 0 || stat >= MAX_STATS)
				Com_Error (ERR_DROP, "Bad ifneq stat index");
			token = COM_Parse (&s);
			value = atoi(token);
			if (cl.frame.playerstate.stats[stat] == value)
			{	// skip to endif
				while (s && strcmp(token, "endif") )
				{
					token = COM_Parse (&s);
				}			
			}
		}

		if (!strcmp(token, "ifgt"))
		{
			token = COM_Parse (&s);
			stat = atoi(token);
			if (stat < 0 || stat >= MAX_STATS)
				Com_Error (ERR_DROP, "Bad ifgt stat index");
			token = COM_Parse (&s);
			value = atoi(token);
			if (cl.frame.playerstate.stats[stat] <= value)
			{	// skip to endif
				while (s && strcmp(token, "endif") )
				{
					token = COM_Parse (&s);
				}			
			}
		}

		if (!strcmp(token, "ifge"))
		{
			token = COM_Parse (&s);
			stat = atoi(token);
			if (stat < 0 || stat >= MAX_STATS)
				Com_Error (ERR_DROP, "Bad ifge stat index");
			token = COM_Parse (&s);
			value = atoi(token);
			if (cl.frame.playerstate.stats[stat] < value)
			{	// skip to endif
				while (s && strcmp(token, "endif") )
				{
					token = COM_Parse (&s);
				}			
			}
		}

		if (!strcmp(token, "iflt"))
		{
			token = COM_Parse (&s);
			stat = atoi(token);
			if (stat < 0 || stat >= MAX_STATS)
				Com_Error (ERR_DROP, "Bad iflt stat index");
			token = COM_Parse (&s);
			value = atoi(token);
			if (cl.frame.playerstate.stats[stat] >= value)
			{	// skip to endif
				while (s && strcmp(token, "endif") )
				{
					token = COM_Parse (&s);
				}			
			}
		}

		if (!strcmp(token, "ifle"))
		{
			token = COM_Parse (&s);
			stat = atoi(token);
			if (stat < 0 || stat >= MAX_STATS)
				Com_Error (ERR_DROP, "Bad ifle stat index");
			token = COM_Parse (&s);
			value = atoi(token);
			if (cl.frame.playerstate.stats[stat] > value)
			{	// skip to endif
				while (s && strcmp(token, "endif") )
				{
					token = COM_Parse (&s);
				}			
			}
		}

		if (!strcmp(token, "ifbit"))
		{
			token = COM_Parse (&s);
			stat = atoi(token);
			if (stat < 0 || stat >= MAX_STATS)
				Com_Error (ERR_DROP, "Bad ifbit stat index");
			token = COM_Parse (&s);
			value = atoi(token);
			if (value < 1 || value > 16)
				Com_Error (ERR_DROP, "Bad ifbit bit value");
			if ( !(cl.frame.playerstate.stats[stat] & (1<<(value-1))) )
			{	// skip to endif
				while (s && strcmp(token, "endif") )
				{
					token = COM_Parse (&s);
				}			
			}
		}

		if (!strcmp(token, "ifnbit"))
		{
			token = COM_Parse (&s);
			stat = atoi(token);
			if (stat < 0 || stat >= MAX_STATS)
				Com_Error (ERR_DROP, "Bad ifnbit stat index");
			token = COM_Parse (&s);
			value = atoi(token);
			if (value < 1 || value > 16)
				Com_Error (ERR_DROP, "Bad ifnbit bit value");
			if ( cl.frame.playerstate.stats[stat] & (1<<(value-1)) )
			{	// skip to endif
				while (s && strcmp(token, "endif") )
				{
					token = COM_Parse (&s);
				}			
			}
		}
		// end Knightmare
	}
}


/*
================
CL_DrawStatus

The status bar is a small layout program that
is based on the stats array
================
*/
void CL_DrawStatus (void)
{
	CL_ExecuteLayoutString (cl.configstrings[CS_STATUSBAR], true);
}


/*
================
CL_DrawLayout

================
*/
#define	STAT_LAYOUTS		13

void CL_DrawLayout (void)
{
	qboolean isStatusBar = false;

	if (!cl.frame.playerstate.stats[STAT_LAYOUTS])
		return;

	// Special hack for visor HUD addition in Zaero
	if ( strncmp(cl.layout, "xv 26 yb -75 string \"Tracking ", 30) == 0 )
		isStatusBar = true;

	CL_ExecuteLayoutString (cl.layout, isStatusBar);
}

/*
===============================================================

INVENTORY CODE

===============================================================
*/

/*
================
CL_ParseInventory
================
*/
void CL_ParseInventory (void)
{
	int		i, max_items;

	// Knightmare- hack for connected to server using old protocol
	// Changed config strings require different parsing
	if ( LegacyProtocol() )
		max_items = OLD_MAX_ITEMS;
	else
		max_items = MAX_ITEMS;

	for (i = 0; i < max_items; i++)
		cl.inventory[i] = MSG_ReadShort (&net_message);
}


/*
================
CL_DrawInventory
================
*/
#define	DISPLAY_ITEMS	17

void CL_DrawInventory (void)
{
	int		i, j;
	int		num, selected_num, item;
	int		index[MAX_ITEMS];
	char	string[1024];
	int		x, y;
	char	binding[1024];
	char	*bind;
	int		selected;
	int		top;

	selected = cl.frame.playerstate.stats[STAT_SELECTED_ITEM];

	num = 0;
	selected_num = 0;
	for (i=0; i<MAX_ITEMS; i++)
	{
		if (i==selected)
			selected_num = num;
		if (cl.inventory[i])
		{
			index[num] = i;
			num++;
		}
	}

	// determine scroll point
	top = selected_num - DISPLAY_ITEMS/2;
	if (num - top < DISPLAY_ITEMS)
		top = num - DISPLAY_ITEMS;
	if (top < 0)
		top = 0;

//	x = viddef.width/2 - SCR_ScaledHud(128);
//	y = viddef.height/2 - SCR_ScaledHud(120);
	x = SCREEN_WIDTH/2 - 128;
	y = SCREEN_HEIGHT/2 - 116;

//	R_DrawScaledPic (x, y+SCR_ScaledHud(8), SCR_GetHudScale(), scr_hudalpha->value, "inventory");
//	y += SCR_ScaledHud(24);
//	x += SCR_ScaledHud(24);
//	Hud_DrawString (x, y, S_COLOR_BOLD"hotkey ### item");
//	Hud_DrawString (x, y+SCR_ScaledHud(8), S_COLOR_BOLD"------ --- ----");
//	y += SCR_ScaledHud(16);
	SCR_DrawPic (x, y, 256, 192, ALIGN_CENTER, false, "inventory", scr_hudalpha->value);
	x += 24;
	y += 20;
	SCR_DrawString (x, y, 8, ALIGN_CENTER, S_COLOR_WHITE"hotkey ### item", FONT_SCREEN, 255);
	y += 8;
	SCR_DrawString (x, y, 8, ALIGN_CENTER, S_COLOR_WHITE"------ --- ----", FONT_SCREEN, 255);
	x += 16;
	y += 8;

	for (i=top; i<num && i < top+DISPLAY_ITEMS; i++)
	{
		item = index[i];
		// search for a binding

		// Knightmare- BIG UGLY HACK for connected to server using old protocol
		// Changed config strings require different parsing
		if ( LegacyProtocol() )
			Com_sprintf (binding, sizeof(binding), "use %s", cl.configstrings[OLD_CS_ITEMS+item]);
		else
			Com_sprintf (binding, sizeof(binding), "use %s", cl.configstrings[CS_ITEMS+item]);

		bind = "";
		for (j=0; j<256; j++)
			if (keybindings[j] && !Q_stricmp (keybindings[j], binding))
			{
				bind = Key_KeynumToString(j);
				break;
			}

		// Knightmare- BIG UGLY HACK for connected to server using old protocol
		// Changed config strings require different parsing
		if ( LegacyProtocol() )
		{
			if (item != selected)
			{
				Com_sprintf (string, sizeof(string), " "S_COLOR_ALT"%3s %3i %7s", bind, cl.inventory[item],
					cl.configstrings[OLD_CS_ITEMS+item] );
			}
			else	// draw a blinky cursor by the selected item
			{
				Com_sprintf (string, sizeof(string), S_COLOR_WHITE">"S_COLOR_ITALIC"%3s %3i %7s", bind, cl.inventory[item],
					cl.configstrings[OLD_CS_ITEMS+item] );
			}
		}
		else
		{
			if (item != selected)
			{
				Com_sprintf (string, sizeof(string), " "S_COLOR_ALT"%3s %3i %7s", bind, cl.inventory[item],
					cl.configstrings[CS_ITEMS+item] );
			}
			else	// draw a blinky cursor by the selected item
			{
				Com_sprintf (string, sizeof(string), S_COLOR_WHITE">"S_COLOR_ITALIC"%3s %3i %7s", bind, cl.inventory[item],
					cl.configstrings[CS_ITEMS+item] );
			}
		}

	//	Hud_DrawString (x, y, string);
	//	y += SCR_ScaledHud(8);
		SCR_DrawString (x, y, 8, ALIGN_CENTER, string, FONT_SCREEN, 255);
		y += 8;
	}
}
