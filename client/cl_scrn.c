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

// cl_scrn.c -- master for refresh, status bar, console, chat, notify, etc

/*
  full screen console
  put up loading plaque
  blanked background with loading plaque
  blanked background with menu
  cinematics
  full screen image for quit and victory

  end of unit intermissions
  */

#include "client.h"
#include "../ui/ui_local.h"

float		scr_con_current;	// aproaches scr_conlines at scr_conspeed
float		scr_conlines;		// 0.0 to 1.0 lines of console to display

qboolean	scr_initialized;		// ready to draw

int			scr_draw_loading;

vrect_t		scr_vrect;		// position of render window on screen


cvar_t		*scr_viewsize;
cvar_t		*scr_conspeed;
cvar_t		*scr_centertime;
cvar_t		*scr_showturtle;
cvar_t		*scr_showpause;
cvar_t		*scr_printspeed;

cvar_t		*scr_netgraph;
cvar_t		*scr_netgraph_pos;
cvar_t		*scr_timegraph;
cvar_t		*scr_debuggraph;
cvar_t		*scr_graphheight;
cvar_t		*scr_graphscale;
cvar_t		*scr_graphshift;
cvar_t		*scr_drawall;

cvar_t		*hud_scale;
cvar_t		*hud_width;
cvar_t		*hud_height;
cvar_t		*hud_alpha;


//Knightmare 12/28/2001- BramBo's FPS counter
cvar_t		*cl_drawfps;
//end Knightmare
cvar_t		*cl_demomessage;
cvar_t		*cl_loadpercent;

typedef struct
{
	int		x1, y1, x2, y2;
} dirty_t;

dirty_t		scr_dirty, scr_old_dirty[2];

char		crosshair_pic[MAX_QPATH];
int			crosshair_width, crosshair_height;

void SCR_TimeRefresh_f (void);
void SCR_Loading_f (void);

#define	ICON_WIDTH	24
#define	ICON_HEIGHT	24
#define	CHAR_WIDTH	16
#define	ICON_SPACE	8

/*
===============================================================================

SCREEN SCALING

===============================================================================
*/

/*
================
SCR_InitScreenScale
================
*/
void SCR_InitScreenScale (void)
{
	// also, don't scale if < 640x480, then it would be smaller
	//if (viddef.width > SCREEN_WIDTH || viddef.height > SCREEN_HEIGHT)
	//{
		screenScale.x = viddef.width/SCREEN_WIDTH;
		screenScale.y = viddef.height/SCREEN_HEIGHT;
		//screenScale.avg = (screenScale.x + screenScale.y)*0.5f;
		screenScale.avg = min(screenScale.x, screenScale.y); // use smaller value instead of average
	/*}
	else
	{
		screenScale.x = 1;
		screenScale.y = 1;
		screenScale.avg = 1;
	}*/
}


float SCR_ScaledVideo (float param)
{
	return param*screenScale.avg;
}


float SCR_VideoScale (void)
{
	return screenScale.avg;
}


/*
================
SCR_AdjustFrom640
Adjusted for resolution and screen aspect ratio
================
*/
void SCR_AdjustFrom640 (float *x, float *y, float *w, float *h, scralign_t align)
{
	float	tmp_x, tmp_y, xscale, yscale;

	SCR_InitScreenScale ();

	xscale = viddef.width / SCREEN_WIDTH;
	yscale = viddef.height / SCREEN_HEIGHT;

	// aspect-ratio independent scaling
	switch (align)
	{
	case ALIGN_CENTER:
		if (w) 
			*w *= screenScale.avg;
		if (h)
			*h *= screenScale.avg;
		if (x) {
			tmp_x = *x;
			*x = (tmp_x - (0.5 * SCREEN_WIDTH)) * screenScale.avg + (0.5 * viddef.width);
		}
		if (y) {
			tmp_y = *y;
			*y = (tmp_y - (0.5 * SCREEN_HEIGHT)) * screenScale.avg + (0.5 * viddef.height);
		}
		break;
	case ALIGN_TOP:
		if (w) 
			*w *= screenScale.avg;
		if (h)
			*h *= screenScale.avg;
		if (x) {
			tmp_x = *x;
			*x = (tmp_x - (0.5 * SCREEN_WIDTH)) * screenScale.avg + (0.5 * viddef.width);
		}
		if (y)
			*y *= screenScale.avg;
		break;
	case ALIGN_BOTTOM:
		if (w) 
			*w *= screenScale.avg;
		if (h)
			*h *= screenScale.avg;
		if (x) {
			tmp_x = *x;
			*x = (tmp_x - (0.5 * SCREEN_WIDTH)) * screenScale.avg + (0.5 * viddef.width);
		}
		if (y) {
			tmp_y = *y;
			*y = (tmp_y - SCREEN_HEIGHT) * screenScale.avg + viddef.height;
		}
		break;
	case ALIGN_RIGHT:
		if (w) 
			*w *= screenScale.avg;
		if (h)
			*h *= screenScale.avg;
		if (x) {
			tmp_x = *x;
			*x = (tmp_x - SCREEN_WIDTH) * screenScale.avg + viddef.width;
		}
		if (y) {
			tmp_y = *y;
			*y = (tmp_y - (0.5 * SCREEN_HEIGHT)) * screenScale.avg + (0.5 * viddef.height);
		}
		break;
	case ALIGN_LEFT:
		if (w) 
			*w *= screenScale.avg;
		if (h)
			*h *= screenScale.avg;
		if (x)
			*x *= screenScale.avg;
		if (y) {
			tmp_y = *y;
			*y = (tmp_y - (0.5 * SCREEN_HEIGHT)) * screenScale.avg + (0.5 * viddef.height);
		}
		break;
	case ALIGN_TOPRIGHT:
		if (w) 
			*w *= screenScale.avg;
		if (h)
			*h *= screenScale.avg;
		if (x) {
			tmp_x = *x;
			*x = (tmp_x - SCREEN_WIDTH) * screenScale.avg + viddef.width;
		}
		if (y)
			*y *= screenScale.avg;
		break;
	case ALIGN_TOPLEFT:
		if (w) 
			*w *= screenScale.avg;
		if (h)
			*h *= screenScale.avg;
		if (x)
			*x *= screenScale.avg;
		if (y)
			*y *= screenScale.avg;
		break;
	case ALIGN_BOTTOMRIGHT:
		if (w) 
			*w *= screenScale.avg;
		if (h)
			*h *= screenScale.avg;
		if (x) {
			tmp_x = *x;
			*x = (tmp_x - SCREEN_WIDTH) * screenScale.avg + viddef.width;
		}
		if (y) {
			tmp_y = *y;
			*y = (tmp_y - SCREEN_HEIGHT) * screenScale.avg + viddef.height;
		}
		break;
	case ALIGN_BOTTOMLEFT:
		if (w) 
			*w *= screenScale.avg;
		if (h)
			*h *= screenScale.avg;
		if (x)
			*x *= screenScale.avg;
		if (y) {
			tmp_y = *y;
			*y = (tmp_y - SCREEN_HEIGHT) * screenScale.avg + viddef.height;
		}
		break;
	case ALIGN_BOTTOM_STRETCH:
		if (w) 
			*w *= xscale;
		if (h)
			*h *= screenScale.avg;
		if (x)
			*x *= xscale;
		if (y) {
			tmp_y = *y;
			*y = (tmp_y - SCREEN_HEIGHT) * screenScale.avg + viddef.height;
		}
		break;
	case ALIGN_STRETCH:
	default:
		if (x)
			*x *= xscale;
		if (y) 
			*y *= yscale;
		if (w) 
			*w *= xscale;
		if (h)
			*h *= yscale;
		break;
	}
}

/*
================
SCR_DrawFill
Coordinates are 640*480 virtual values
=================
*/
void SCR_DrawFill (float x, float y, float width, float height, scralign_t align, int color)
{
	SCR_AdjustFrom640 (&x, &y, &width, &height, align);
	R_DrawFill (x, y, width, height, color);
}

/*
================
SCR_DrawFill2
Coordinates are 640*480 virtual values
=================
*/
void SCR_DrawFill2 (float x, float y, float width, float height, scralign_t align, int red, int green, int blue, int alpha)
{
	SCR_AdjustFrom640 (&x, &y, &width, &height, align);
	R_DrawFill2 (x, y, width, height, red, green, blue, alpha);
}

/*
================
SCR_DrawPic
Coordinates are 640*480 virtual values
=================
*/
void SCR_DrawPic (float x, float y, float width, float height, scralign_t align, char *pic, float alpha)
{
	SCR_AdjustFrom640 (&x, &y, &width, &height, align);
	R_DrawStretchPic (x, y, width, height, pic, alpha);
}

/*
================
SCR_DrawChar
Coordinates are 640*480 virtual values
=================
*/
void SCR_DrawChar (float x, float y, scralign_t align, int num, int red, int green, int blue, int alpha, qboolean italic, qboolean last)
{
	SCR_AdjustFrom640 (&x, &y, NULL, NULL, align);
	R_DrawChar(x, y, num, screenScale.avg, red, green, blue, alpha, italic, last);
}

/*
================
SCR_DrawString
Coordinates are 640*480 virtual values
=================
*/
void SCR_DrawString (float x, float y, scralign_t align, const char *string, int alpha)
{
	SCR_AdjustFrom640 (&x, &y, NULL, NULL, align);
	DrawStringGeneric (x, y, string, alpha, SCALETYPE_MENU, false);
}

//===============================================================================


/*
================
Hud_DrawString
================
*/
void Hud_DrawString (int x, int y, const char *string, int alpha)
{
	DrawStringGeneric (x, y, string, alpha, SCALETYPE_HUD, false);
}


/*
================
Hud_DrawStringAlt
================
*/
void Hud_DrawStringAlt (int x, int y, const char *string, int alpha)
{
	DrawStringGeneric (x, y, string, alpha, SCALETYPE_HUD, true);
}


//Knightmare 12/28/2001- BramBo's FPS counter
/*
================
SCR_ShowFPS
================
*/
int		fpscounter;
char	fpsText[32];

void SCR_ShowFPS (void)
{
	int	x, y;

	InitHudScale ();
	if ((cl.time + 1000) < fpscounter)
		fpscounter = cl.time + 100;

	if (cl.time > fpscounter)
	{
		Com_sprintf(fpsText, sizeof(fpsText), S_COLOR_BOLD S_COLOR_SHADOW"%3.0ffps", 1/cls.frametime);
		fpscounter = cl.time + 100;
	}
	// leave space for 3-digit frag counter
	//x = (viddef.width - strlen(fpsText)*FONT_SIZE - 3*HudScale()*(CHAR_WIDTH+2));
	x = (viddef.width - strlen(fpsText)*HUD_FONT_SIZE*HudScale() - 3*HudScale()*(CHAR_WIDTH+2));
	y = 0;
	DrawStringGeneric (x, y, fpsText, 255, SCALETYPE_HUD, false); // SCALETYPE_CONSOLE
	SCR_AddDirtyPoint (x, y);
	SCR_AddDirtyPoint (x+FONT_SIZE*strlen(fpsText), y+FONT_SIZE);
}
//end Knightmare

/*
===============================================================================

BAR GRAPHS

===============================================================================
*/

/*
==============
CL_AddNetgraph

A new packet was just parsed
==============
*/

int currentping;
void CL_AddNetgraph (void)
{
	int		i;
	int		in;
	int		ping;

	in = cls.netchan.incoming_acknowledged & (CMD_BACKUP-1);
	currentping = cls.realtime - cl.cmd_time[in];

	// if using the debuggraph for something else, don't
	// add the net lines
	if (scr_debuggraph->value || scr_timegraph->value)
		return;

	for (i=0 ; i<cls.netchan.dropped ; i++)
		SCR_DebugGraph (30, 0x40);

	for (i=0 ; i<cl.surpressCount ; i++)
		SCR_DebugGraph (30, 0xdf);

	// see what the latency was on this packet
	ping = currentping/30;
	//ping = cls.realtime - cl.cmd_time[in];
	//ping /= 30;
	if (ping > 30)
		ping = 30;
	SCR_DebugGraph (ping, 0xd0);
}


typedef struct
{
	float	value;
	int		color;
} graphsamp_t;

static	int			current;
static	graphsamp_t	values[1024];

/*
==============
SCR_DebugGraph
==============
*/
void SCR_DebugGraph (float value, int color)
{
	values[current&1023].value = value;
	values[current&1023].color = color;
	current++;
}

/*
==============
SCR_DrawDebugGraph
==============
*/
void SCR_DrawDebugGraph (void)
{
	int		a, x, y, w, i, h, min, max;
	float	v;
	int		color;
	static	float lasttime = 0;
	static	int fps, ping;

	h = (2*FONT_SIZE > 40)?60+2*FONT_SIZE:100;
	w = (9*FONT_SIZE>100)?9*FONT_SIZE:100;

	if (scr_netgraph_pos->value == 0) // bottom right
	{
		x = scr_vrect.width - (w+2) - 1;
		y = scr_vrect.height - (h+2) - 1;
	}	
	else // bottom left
	{
		x = 0;
		y = scr_vrect.height - (h+2) - 1;
	}	

	// trans background
	R_DrawFill2 (x, y, w+2, h+2, 255, 255, 255, 90);

	for (a=0 ; a<w ; a++)
	{
		i = (current-1-a+1024) & 1023;
		v = values[i].value;
		color = values[i].color;
		v = v*scr_graphscale->value;
		
		if (v < 1)
			v += h * (1+(int)(-v/h));

		max = (int)v % h + 1;
		min = y + h - max - scr_graphshift->value;

		//bind to box!
		if (min<y+1) min = y+1;
		if (min>y+h) min = y+h;
		if (min+max > y+h) max = y+h-max;

		R_DrawFill (x+w-a, min, 1, max, color);
	}

	if (cls.realtime - lasttime > 50)
	{
		lasttime = cls.realtime;
		fps = (cls.frametime)? 1/cls.frametime: 0;
		ping = currentping;
	}

	DrawStringGeneric (x, y + 5, va(S_COLOR_SHADOW"fps: %3i", fps), 255, SCALETYPE_CONSOLE, false);
	DrawStringGeneric (x, y + 5 + FONT_SIZE , va(S_COLOR_SHADOW"ping:%3i", ping), 255, SCALETYPE_CONSOLE, false);

	// draw border
	R_DrawFill2 (x,			y,			(w+2),	1,		0, 0, 0, 255);
	R_DrawFill2 (x,			y+(h+2),	(w+2),	1,		0, 0, 0, 255);
	R_DrawFill2 (x,			y,			1,		(h+2),	0, 0, 0, 255);
	R_DrawFill2 (x+(w+2),	y,			1,		(h+2),	0, 0, 0, 255);
}

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

char		scr_centerstring[1024];
float		scr_centertime_start;	// for slow victory printing
float		scr_centertime_off;
float		scr_centertime_end;
int			scr_center_lines;
int			scr_erase_center;

/*
==============
SCR_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void SCR_CenterPrint (char *str)
{
	char	*s;
	char	line[64];
	int		i, j, l;

	strncpy (scr_centerstring, str, sizeof(scr_centerstring)-1);
	scr_centertime_off = scr_centertime->value;
	scr_centertime_end = scr_centertime_off;
	scr_centertime_start = cl.time;

	// count the number of lines for centering
	scr_center_lines = 1;
	s = str;
	while (*s)
	{
		if (*s == '\n')
			scr_center_lines++;
		s++;
	}

	// echo it to the console
	Com_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");

	s = str;
	do	
	{
	// scan the width of the line
		for (l=0 ; l<40 ; l++)
			if (s[l] == '\n' || !s[l])
				break;
		for (i=0 ; i<(40-l)/2 ; i++)
			line[i] = ' ';

		for (j=0 ; j<l ; j++)
		{
			line[i++] = s[j];
		}

		line[i] = '\n';
		line[i+1] = 0;

		Com_Printf ("%s", line);

		while (*s && *s != '\n')
			s++;

		if (!*s)
			break;
		s++;		// skip the \n
	} while (1);
	Com_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	Con_ClearNotify ();
}


void SCR_DrawCenterString (void)
{
	char	*start, line[512];
	int		l;
	int		j;
	int		y; //, x;
	int		remaining;
	// added Psychospaz's fading centerstrings
	int		alpha = 255 * ( 1 - (((cl.time + (scr_centertime->value-1) - scr_centertime_start) / 1000.0) / (scr_centertime_end)));		

	// the finale prints the characters one at a time
	remaining = 9999;

	scr_erase_center = 0;
	start = scr_centerstring;

	if (scr_center_lines <= 4)
		y = viddef.height*0.35;
	else
		y = 48;

	do
	{	// scan the width of the line
		for (l=0 ; l<40 ; l++)
			if (start[l] == '\n' || !start[l])
				break;

		Com_sprintf (line, sizeof(line), "");
		for (j=0 ; j<l ; j++)
		{
			
			Com_sprintf (line, sizeof(line), "%s%c", line, start[j]);
			
			if (!remaining--)
				return;
		}
		DrawStringGeneric ( (int)((viddef.width-strlen(line)*FONT_SIZE)*0.5), y, line, alpha, SCALETYPE_CONSOLE, false);
		y += FONT_SIZE;

		while (*start && *start != '\n')
			start++;

		if (!*start)
			break;
		start++;		// skip the \n
	} while (1);
}

void SCR_CheckDrawCenterString (void)
{
	scr_centertime_off -= cls.frametime;
	
	if (scr_centertime_off <= 0)
		return;

	SCR_DrawCenterString ();
}

//=============================================================================

/*
=================
SCR_CalcVrect

Sets scr_vrect, the coordinates of the rendered window
=================
*/
static void SCR_CalcVrect (void)
{
	int		size;

	// bound viewsize
	if (scr_viewsize->value < 40)
		Cvar_Set ("viewsize","40");
	if (scr_viewsize->value > 100)
		Cvar_Set ("viewsize","100");

	size = scr_viewsize->value;

	scr_vrect.width = viddef.width*size/100;
	scr_vrect.width &= ~7;

	scr_vrect.height = viddef.height*size/100;
	scr_vrect.height &= ~1;

	scr_vrect.x = (viddef.width - scr_vrect.width)/2;
	scr_vrect.y = (viddef.height - scr_vrect.height)/2;
}


/*
=================
SCR_SizeUp_f

Keybinding command
=================
*/
void SCR_SizeUp_f (void)
{	
	//Cvar_SetValue ("viewsize",scr_viewsize->value+10);

	// Knightmare- now handle HUD alpha
	float hudalpha = Cvar_VariableValue("hud_alpha")+0.1;
	if (hudalpha > 1) hudalpha = 0.1;
	Cvar_SetValue ("hud_alpha", hudalpha);
}


/*
=================
SCR_SizeDown_f

Keybinding command
=================
*/
void SCR_SizeDown_f (void)
{
	//Cvar_SetValue ("viewsize",scr_viewsize->value-10);

	// Knightmare- now handle HUD scale
	int hudscale = Cvar_VariableValue("hud_scale")+1;
	if (hudscale > 6) hudscale = 1;
	Cvar_SetValue ("hud_scale", hudscale);
}

/*
=================
SCR_Sky_f

Set a specific sky and rotation speed
=================
*/
void SCR_Sky_f (void)
{
	float	rotate;
	vec3_t	axis;

	if (Cmd_Argc() < 2)
	{
		Com_Printf ("Usage: sky <basename> <rotate> <axis x y z>\n");
		return;
	}
	if (Cmd_Argc() > 2)
		rotate = atof(Cmd_Argv(2));
	else
		rotate = 0;
	if (Cmd_Argc() == 6)
	{
		axis[0] = atof(Cmd_Argv(3));
		axis[1] = atof(Cmd_Argv(4));
		axis[2] = atof(Cmd_Argv(5));
	}
	else
	{
		axis[0] = 0;
		axis[1] = 0;
		axis[2] = 1;
	}

	R_SetSky (Cmd_Argv(1), rotate, axis);
}

//============================================================================

/*
==================
SCR_Init
==================
*/
void SCR_Init (void)
{
	//Knightmare 12/28/2001- FPS counter
	cl_drawfps = Cvar_Get ("cl_drawfps", "0", CVAR_ARCHIVE);
	cl_demomessage = Cvar_Get ("cl_demomessage", "1", CVAR_ARCHIVE);
	cl_loadpercent = Cvar_Get ("cl_loadpercent", "0", CVAR_ARCHIVE);

	scr_viewsize = Cvar_Get ("viewsize", "100", CVAR_ARCHIVE);
	scr_conspeed = Cvar_Get ("scr_conspeed", "3", 0);
	scr_showturtle = Cvar_Get ("scr_showturtle", "0", 0);
	scr_showpause = Cvar_Get ("scr_showpause", "1", 0);
	// Knightmare- increased for fade
	scr_centertime = Cvar_Get ("scr_centertime", "3.5", 0);
	scr_printspeed = Cvar_Get ("scr_printspeed", "8", 0);
	scr_netgraph = Cvar_Get ("netgraph", "0", 0);
	scr_netgraph_pos = Cvar_Get ("netgraph_pos", "0", CVAR_ARCHIVE);
	scr_timegraph = Cvar_Get ("timegraph", "0", 0);
	scr_debuggraph = Cvar_Get ("debuggraph", "0", 0);
	scr_graphheight = Cvar_Get ("graphheight", "32", 0);
	scr_graphscale = Cvar_Get ("graphscale", "1", 0);
	scr_graphshift = Cvar_Get ("graphshift", "0", 0);
	scr_drawall = Cvar_Get ("scr_drawall", "0", 0);

	hud_scale = Cvar_Get ("hud_scale", "3", CVAR_ARCHIVE);
	hud_width = Cvar_Get ("hud_width", "640", CVAR_ARCHIVE);
	hud_height = Cvar_Get ("hud_height", "480", CVAR_ARCHIVE);
	hud_alpha = Cvar_Get ("hud_alpha", "1", CVAR_ARCHIVE);

//
// register our commands
//
	Cmd_AddCommand ("timerefresh",SCR_TimeRefresh_f);
	Cmd_AddCommand ("loading",SCR_Loading_f);
	Cmd_AddCommand ("sizeup",SCR_SizeUp_f);
	Cmd_AddCommand ("sizedown",SCR_SizeDown_f);
	Cmd_AddCommand ("sky",SCR_Sky_f);

	SCR_InitScreenScale ();
	InitHudScale ();

	scr_initialized = true;
}


/*
==============
SCR_DrawNet
==============
*/
void SCR_DrawNet (void)
{
	if (cls.netchan.outgoing_sequence - cls.netchan.incoming_acknowledged 
		< CMD_BACKUP-1)
		return;

	R_DrawPic (scr_vrect.x+scaledHud(64), scr_vrect.y, "net");
}

/*
==============
SCR_DrawAlertMessagePicture
==============
*/
void SCR_DrawAlertMessagePicture (char *name, qboolean center, int yOffset)
{
	float ratio;//, scale;
	int w, h;

	//scale = SCR_VideoScale();

	R_DrawGetPicSize (&w, &h, name);
	if (w)
	{
		ratio = 35.0/(float)h;
		h = 35;
		w *= ratio;
	}
	else
		return;

	if (center)
		SCR_DrawPic((SCREEN_WIDTH - w)*0.5, (SCREEN_HEIGHT - h)*0.5,
				w, h, ALIGN_CENTER, name, 1.0);
	else
		SCR_DrawPic((SCREEN_WIDTH - w)*0.5, SCREEN_HEIGHT*0.5 + yOffset,
				w, h, ALIGN_CENTER, name, 1.0);
}

/*
==============
SCR_DrawPause
==============
*/
void SCR_DrawPause (void)
{
	int		w, h;

	if (!scr_showpause->value)		// turn off for screenshots
		return;

	if (!cl_paused->value)
		return;

	// Knightmare- no need to draw when in menu
	if (cls.key_dest == key_menu)
		return;

	R_DrawGetPicSize (&w, &h, "pause");

	w*= 0.5;
	h*= 0.5;

	SCR_DrawPic ((SCREEN_WIDTH-w)*0.5, (SCREEN_HEIGHT-h) * 0.8, w, h, ALIGN_CENTER, "pause", 1.0);
}


/*
==============
SCR_DrawLoadingBar
==============
*/
void SCR_DrawLoadingBar (float x, float y, float w, float h, int percent, float sizeRatio)
{	
	int		red, green, blue;

	float	iRatio, hiRatio;

	//hardcode loadbar color
	red = 224;
	green = 0;
	blue = 0;


	// changeable download/map load bar color
	//TextColor((int)alt_text_color->value, &red, &green, &blue);
	iRatio = 1 - fabs(sizeRatio);
	hiRatio = iRatio * 0.5;

	SCR_DrawFill2 (x, y, w, h, ALIGN_STRETCH, 255,255,255, 48);

	if (percent != 0)
		SCR_DrawFill2 (x+(h*hiRatio), y+(h*hiRatio), (w-(h*iRatio))*percent*0.01, h*sizeRatio,
						ALIGN_STRETCH, red, green, blue, 255);
}


char *load_saveshot;
//void Menu_DrawString( int x, int y, const char *string, int alpha );
int stringLen (char *string);

/*
==============
SCR_DrawLoading
==============
*/
void SCR_DrawLoading (void)
{
	//int		w, h;
	char	mapfile[32];
	qboolean isMap = false;

	if (!scr_draw_loading)
		return;

	scr_draw_loading = 0;

	//	scale = 1.5;

	//loading a map...
	if (loadingMessage && cl.configstrings[CS_MODELS+1][0])
	{
		strcpy (mapfile, cl.configstrings[CS_MODELS+1] + 5);	// skip "maps/"
		mapfile[strlen(mapfile)-4] = 0;		// cut off ".bsp"

		// show saveshot here
		if (load_saveshot && strlen(load_saveshot) && R_DrawFindPic(load_saveshot))
			SCR_DrawPic (0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ALIGN_STRETCH, load_saveshot, 1.0);
		// else try levelshot
		else if (R_DrawFindPic(va("/levelshots/%s.pcx", mapfile)))
			SCR_DrawPic (0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ALIGN_STRETCH, va("/levelshots/%s.pcx", mapfile), 1.0);
		// else fall back on loadscreen
		else if (R_DrawFindPic("/gfx/loadscreen.pcx"))
		{
			SCR_DrawFill2 (0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ALIGN_STRETCH, 255, 255, 255, 255);

			//BC 45PRM LP LOADING SCREEN
			
			/*SCR_DrawPic (140, 20,
				440, 440,
				ALIGN_STRETCH,
				"/gfx/loadscreen.pcx", 1.0);			*/

			R_DrawStretchPic (
				(viddef.width / 2) - (viddef.height/2),
				0,
				viddef.height,
				viddef.height,
				"/gfx/loadscreen.pcx",
				1.0);


			SCR_DrawPic (0, 0,
				SCREEN_WIDTH, SCREEN_HEIGHT,
				ALIGN_STRETCH,
				"/pics/vignette.tga", 1.0);
		}
		else
			SCR_DrawFill2 (0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ALIGN_STRETCH, 0, 0, 0, 255);

		isMap = true;
	}
	//else if (R_DrawFindPic("/gfx/loadscreen.pcx"))
	//	SCR_DrawPic (0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ALIGN_STRETCH, "/gfx/loadscreen.pcx", 1.0);
	else
		SCR_DrawFill2 (0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ALIGN_STRETCH, 0, 0, 0, 255);

	// Add Download info stuff...
	if (cls.download) //download bar...
	{
		char *download = va("Downloading ["S_COLOR_ALT"%s"S_COLOR_WHITE"]", cls.downloadname);

		/*Menu_DrawString(
			(int)(viddef.width*0.5 - SCR_VideoScale()*MENU_FONT_SIZE*stringLen(download)*0.5), 
			(int)(viddef.height*0.5) + scale*MENU_FONT_SIZE*3, 
			download, 255);

		Menu_DrawString(
			(int)(viddef.width*0.5) - (int)(scale*MENU_FONT_SIZE*1.5), 
			(int)(viddef.height*0.5 + scale*MENU_FONT_SIZE*4), // was 5.7
			va("%3d%%", (int)cls.downloadpercent), 255);
		
		SCR_DrawLoadingBar ((int)(viddef.width*0.5) - scale*MENU_FONT_SIZE*15,
							(int)(viddef.height*0.5) + scale*MENU_FONT_SIZE*5, 
							scale*MENU_FONT_SIZE*30, scale*MENU_FONT_SIZE*2, cls.downloadpercent, true);
		*/
		SCR_DrawString (SCREEN_WIDTH*0.5 - MENU_FONT_SIZE*stringLen(download)*0.5,
						SCREEN_HEIGHT*0.5 + MENU_FONT_SIZE*4.5, ALIGN_CENTER, download, 255);
		SCR_DrawString (SCREEN_WIDTH*0.5 - MENU_FONT_SIZE*2,
						SCREEN_HEIGHT*0.5 + MENU_FONT_SIZE*6, ALIGN_CENTER, va("%3d%%", cls.downloadpercent), 255);
		// MENU_FONT_SIZE*22.5, MENU_FONT_SIZE*7.5, MENU_FONT_SIZE*45, MENU_FONT_SIZE*3
		SCR_DrawLoadingBar (SCREEN_WIDTH*0.5 - 180, SCREEN_HEIGHT*0.5 + 60, 360, 24, cls.downloadpercent, 0.75);
		//SCR_DrawAlertMessagePicture("loading", false, -130);
	}
	// Loading message stuff && loading bar...
	else if (isMap)
	{
		char *mapmsg;

		/*mapmsg = va(S_COLOR_SHADOW S_COLOR_WHITE"Loading Map ["S_COLOR_ALT"%s"S_COLOR_WHITE"]", cl.configstrings[CS_NAME]);
		Menu_DrawString(
			(int)(viddef.width*0.5 - SCR_VideoScale()*MENU_FONT_SIZE*stringLen(mapmsg)*0.5), 
			(int)(viddef.height*0.5) - scale*MENU_FONT_SIZE*5,
			mapmsg, 255);

		mapmsg = va(S_COLOR_SHADOW"%s", loadingMessages);
		Menu_DrawString(
			(int)(viddef.width*0.5 - SCR_VideoScale()*MENU_FONT_SIZE*stringLen(mapmsg)*0.5), 
			(int)(viddef.height*0.5 - scale*MENU_FONT_SIZE*3), 
			mapmsg, 255);

		SCR_DrawLoadingBar ((int)(viddef.width*0.5 - scale*MENU_FONT_SIZE*15),
							(int)(viddef.height - scale*MENU_FONT_SIZE*1.5), 
							scale*MENU_FONT_SIZE*30, (int)(scale*MENU_FONT_SIZE*1.2), (int)loadingPercent, false);
		*/




		/*
		//bc domment out
		mapmsg = va(S_COLOR_SHADOW S_COLOR_WHITE"Loading Map ["S_COLOR_ALT"%s"S_COLOR_WHITE"]", cl.configstrings[CS_NAME]);


		SCR_DrawString (SCREEN_WIDTH*0.5 - MENU_FONT_SIZE*stringLen(mapmsg)*0.5,
						SCREEN_HEIGHT*0.5 - MENU_FONT_SIZE*7.5, ALIGN_CENTER, mapmsg, 255);


		mapmsg = va(S_COLOR_SHADOW"%s", loadingMessages);

		SCR_DrawString (SCREEN_WIDTH*0.5 - MENU_FONT_SIZE*stringLen(mapmsg)*0.5,
						SCREEN_HEIGHT*0.5 - MENU_FONT_SIZE*4.5, ALIGN_CENTER, mapmsg, 255);

		*/


		// MENU_FONT_SIZE*22.5, MENU_FONT_SIZE*2.25, MENU_FONT_SIZE*45, MENU_FONT_SIZE*1.8
		//SCR_DrawLoadingBar (SCREEN_WIDTH*0.5 - 180, SCREEN_HEIGHT - 20, 360, 15, (int)loadingPercent, 0.6);
		
		
		//SCR_DrawLoadingBar (SCREEN_WIDTH*0.5 - 180, SCREEN_HEIGHT *0.9, 360, 15, (int)loadingPercent, 0.6);

		SCR_DrawLoadingBar (SCREEN_WIDTH /2 - (170/2), SCREEN_HEIGHT - 95,
			170, 10,
			(int)loadingPercent, 0.6);


		//SCR_DrawAlertMessagePicture("loading", false, -130);
	}
	//else // just a plain old loading plaque
	//	SCR_DrawAlertMessagePicture("loading", true, 0);
}

//=============================================================================

/*
==================
SCR_RunConsole
Scroll it up or down
==================
*/
void SCR_RunConsole (void)
{
	// Knightmare- clamp console height
	//if (con_height->value < 0.1f || con_height->value > 1.0f)
	//	Cvar_SetValue( "con_height", ClampCvar( 0.1, 1, con_height->value ) );

	// decide on the height of the console
	if (cls.consoleActive) // (cls.key_dest == key_console)
		//scr_conlines = halfconback?0.5:con_height->value; // variable height console
		scr_conlines = 0.5;
	else
		scr_conlines = 0;				// none visible
	
	if (scr_conlines < scr_con_current)
	{
		scr_con_current -= scr_conspeed->value*cls.frametime;
		if (scr_conlines > scr_con_current)
			scr_con_current = scr_conlines;

	}
	else if (scr_conlines > scr_con_current)
	{
		scr_con_current += scr_conspeed->value*cls.frametime;
		if (scr_conlines < scr_con_current)
			scr_con_current = scr_conlines;
	}

}

/*
==================
SCR_DrawConsole
==================
*/
void SCR_DrawConsole (void)
{
	Con_CheckResize ();
	
	// clamp console height
	//if (con_height->value < 0.1f || con_height->value > 1.0f)
	//	Cvar_SetValue ( "con_height", ClampCvar(0.1, 1, con_height->value) );

	/*if ( (cls.key_dest != key_menu)
		&& (cls.state == ca_disconnected || cls.state == ca_connecting) )
	{
		R_DrawFill (0, 0, viddef.width, viddef.height, 0);
		Con_DrawConsole (con_height->value, false);
		return;
	}

	// connected, but can't render
	if ( (cls.key_dest != key_menu) && (cl.cinematicframe <= 0)
		&& (cls.state != ca_active || !cl.refresh_prepped) 
		&& (Com_ServerState() != 5) ) // fix stuck console over menu		 
	{
		if ((scr_draw_loading))
			Con_DrawConsole (con_height->value, true);
		else
		{
			R_DrawFill (0, 0, viddef.width, viddef.height, 0);
			Con_DrawConsole (con_height->value, false);
		}
		return;
	}*/

	// can't render menu or game
	if ( (cls.key_dest != key_menu) &&
		( (cls.state == ca_disconnected || cls.state == ca_connecting) ||
			( (cls.state != ca_active || !cl.refresh_prepped) &&
				(cl.cinematicframe <= 0) &&
				(Com_ServerState() < 3) // was != 5
			)
		) )
	{
		if (!scr_draw_loading) // no background
			R_DrawFill2 (0, 0, viddef.width, viddef.height, 0, 0, 0, 255);
		//Con_DrawConsole (halfconback?0.5:con_height->value, false);
		Con_DrawConsole (0.5, false);
		return;
	}

	if (scr_con_current)
		Con_DrawConsole (scr_con_current, true);
	else if (!cls.consoleActive && !cl.cinematictime
		&& (cls.key_dest == key_game || cls.key_dest == key_message))
		Con_DrawNotify (); // only draw notify in game
}

//=============================================================================

qboolean needLoadingPlaque (void)
{
	if (!cls.disable_screen || !scr_draw_loading)
		return true;
	return false;
}

/*
================
SCR_BeginLoadingPlaque
================
*/
void SCR_BeginLoadingPlaque (void)
{
	S_StopAllSounds ();
	cl.sound_prepped = false;		// don't play ambients
	CDAudio_Stop ();
	//if (cls.disable_screen)
	//	return;
	if (developer->value)
		return;

	cls.consoleActive = false; // Knightmare added

	/*if (cls.state == ca_disconnected)
		return;	// if at console, don't bring up the plaque
	if (cls.key_dest == key_console)
		return;*/
	if (cl.cinematictime > 0)
		scr_draw_loading = 2;	// clear to black first
	else
		scr_draw_loading = 1;

	SCR_UpdateScreen ();
	cls.disable_screen = Sys_Milliseconds ();
	cls.disable_servercount = cl.servercount;
}

/*
================
SCR_EndLoadingPlaque
================
*/
void SCR_EndLoadingPlaque (void)
{
	// make loading saveshot null here
	load_saveshot = NULL;
	cls.disable_screen = 0;
	scr_draw_loading = 0; // Knightmare added
	Con_ClearNotify ();
}

/*
================
SCR_Loading_f
================
*/
void SCR_Loading_f (void)
{
	SCR_BeginLoadingPlaque ();
}

/*
================
SCR_TimeRefresh_f
================
*/
int entitycmpfnc( const entity_t *a, const entity_t *b )
{
	/*
	** all other models are sorted by model then skin
	*/
	if ( a->model == b->model )
	{
		return ( ( int ) a->skin - ( int ) b->skin );
	}
	else
	{
		return ( ( int ) a->model - ( int ) b->model );
	}
}

void SCR_TimeRefresh_f (void)
{
	int		i;
	int		start, stop;
	float	time;

	if ( cls.state != ca_active )
		return;

	start = Sys_Milliseconds ();

	if (Cmd_Argc() == 2)
	{	// run without page flipping
		R_BeginFrame( 0 );
		for (i=0 ; i<128 ; i++)
		{
			cl.refdef.viewangles[1] = i/128.0*360.0;
			R_RenderFrame (&cl.refdef);
		}
		GLimp_EndFrame();
	}
	else
	{
		for (i=0 ; i<128 ; i++)
		{
			cl.refdef.viewangles[1] = i/128.0*360.0;

			R_BeginFrame( 0 );
			R_RenderFrame (&cl.refdef);
			GLimp_EndFrame();
		}
	}

	stop = Sys_Milliseconds ();
	time = (stop-start)/1000.0;
	Com_Printf ("%f seconds (%f fps)\n", time, 128/time);
}

/*
=================
SCR_AddDirtyPoint
=================
*/
void SCR_AddDirtyPoint (int x, int y)
{
	if (x < scr_dirty.x1)
		scr_dirty.x1 = x;
	if (x > scr_dirty.x2)
		scr_dirty.x2 = x;
	if (y < scr_dirty.y1)
		scr_dirty.y1 = y;
	if (y > scr_dirty.y2)
		scr_dirty.y2 = y;
}

void SCR_DirtyScreen (void)
{
	SCR_AddDirtyPoint (0, 0);
	SCR_AddDirtyPoint (viddef.width-1, viddef.height-1);
}

/*
==============
SCR_TileClear

Clear any parts of the tiled background that were drawn on last frame
==============
*/
void SCR_TileClear (void)
{
	int		i;
	int		top, bottom, left, right;
	dirty_t	clear;

	if (scr_drawall->value)
		SCR_DirtyScreen ();	// for power vr or broken page flippers...

	if (scr_con_current == 1.0)
		return;		// full screen console
	if (scr_viewsize->value == 100)
		return;		// full screen rendering
	if (cl.cinematictime > 0)
		return;		// full screen cinematic

	// erase rect will be the union of the past three frames
	// so tripple buffering works properly
	clear = scr_dirty;
	for (i=0 ; i<2 ; i++)
	{
		if (scr_old_dirty[i].x1 < clear.x1)
			clear.x1 = scr_old_dirty[i].x1;
		if (scr_old_dirty[i].x2 > clear.x2)
			clear.x2 = scr_old_dirty[i].x2;
		if (scr_old_dirty[i].y1 < clear.y1)
			clear.y1 = scr_old_dirty[i].y1;
		if (scr_old_dirty[i].y2 > clear.y2)
			clear.y2 = scr_old_dirty[i].y2;
	}

	scr_old_dirty[1] = scr_old_dirty[0];
	scr_old_dirty[0] = scr_dirty;

	scr_dirty.x1 = 9999;
	scr_dirty.x2 = -9999;
	scr_dirty.y1 = 9999;
	scr_dirty.y2 = -9999;

	// don't bother with anything convered by the console)
	top = scr_con_current*viddef.height;
	if (top >= clear.y1)
		clear.y1 = top;

	if (clear.y2 <= clear.y1)
		return;		// nothing disturbed

	top = scr_vrect.y;
	bottom = top + scr_vrect.height-1;
	left = scr_vrect.x;
	right = left + scr_vrect.width-1;

	if (clear.y1 < top)
	{	// clear above view screen
		i = clear.y2 < top-1 ? clear.y2 : top-1;
		R_DrawTileClear (clear.x1 , clear.y1,
			clear.x2 - clear.x1 + 1, i - clear.y1+1, "backtile");
		clear.y1 = top;
	}
	if (clear.y2 > bottom)
	{	// clear below view screen
		i = clear.y1 > bottom+1 ? clear.y1 : bottom+1;
		R_DrawTileClear (clear.x1, i,
			clear.x2-clear.x1+1, clear.y2-i+1, "backtile");
		clear.y2 = bottom;
	}
	if (clear.x1 < left)
	{	// clear left of view screen
		i = clear.x2 < left-1 ? clear.x2 : left-1;
		R_DrawTileClear (clear.x1, clear.y1,
			i-clear.x1+1, clear.y2 - clear.y1 + 1, "backtile");
		clear.x1 = left;
	}
	if (clear.x2 > right)
	{	// clear left of view screen
		i = clear.x1 > right+1 ? clear.x1 : right+1;
		R_DrawTileClear (i, clear.y1,
			clear.x2-i+1, clear.y2 - clear.y1 + 1, "backtile");
		clear.x2 = right;
	}
}

/*
===============================================================

HUD CODE

===============================================================
*/

#define STAT_MINUS		10	// num frame for '-' stats digit
char		*sb_nums[2][11] = 
{
	{"num_0", "num_1", "num_2", "num_3", "num_4", "num_5",
	"num_6", "num_7", "num_8", "num_9", "num_minus"},
	{"anum_0", "anum_1", "anum_2", "anum_3", "anum_4", "anum_5",
	"anum_6", "anum_7", "anum_8", "anum_9", "anum_minus"}
};

// Knghtmare- scaled HUD support functions
float scaledHud (float param)
{
	return param*hudScale.avg;
}

float HudScale (void)
{
	return hudScale.avg;
}

void InitHudScale (void)
{
	switch ((int)hud_scale->value)
	{
	case 0:
		Cvar_SetValue( "hud_width", 0);
		Cvar_SetValue( "hud_height", 0);
		break;
	case 1:
		Cvar_SetValue( "hud_width", 1024);
		Cvar_SetValue( "hud_height", 768);
		break;
	case 2:
		Cvar_SetValue( "hud_width", 800);
		Cvar_SetValue( "hud_height", 600);
		break;
	case 3:
		Cvar_SetValue( "hud_width", 640);
		Cvar_SetValue( "hud_height", 480);
		break;
	case 4:
		Cvar_SetValue( "hud_width", 512);
		Cvar_SetValue( "hud_height", 384);
		break;
	case 5:
		Cvar_SetValue( "hud_width", 400);
		Cvar_SetValue( "hud_height", 300);
		break;
	case 6:
		Cvar_SetValue( "hud_width", 320);
		Cvar_SetValue( "hud_height", 240);
		break;
	default:
		Cvar_SetValue( "hud_width", 640);
		Cvar_SetValue( "hud_height", 480);
		break;
	}
	// allow disabling of hud scaling
	// also, don't scale if < hud_width, then it would be smaller
	if (hud_scale->value && viddef.width > hud_width->value )
	{
		hudScale.x = viddef.width/hud_width->value;
		hudScale.y = viddef.height/hud_height->value;
		//hudScale.avg = (hudScale.x + hudScale.y)*0.5f;
		hudScale.avg = min(hudScale.x, hudScale.y); // use smaller value instead of average
	}
	else
	{
		hudScale.x = 1;
		hudScale.y = 1;
		hudScale.avg = 1;
	}
}
// end Knightmare

/*
================
SizeHUDString

Allow embedded \n in the string
================
*/
void SizeHUDString (char *string, int *w, int *h)
{
	int		lines, width, current;

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

	*w = width * scaledHud(8);
	*h = lines * scaledHud(8);
}

void _DrawHUDString (char *string, int x, int y, int centerwidth, int xor)
{
	int		margin;
	char	line[1024];
	int		width;
	//int		i, len;

	margin = x;

	//len = strlen(string);

	while (*string)
	{
		// scan out one line of text from the string
		width = 0;
		while (*string && *string != '\n')
			line[width++] = *string++;

		line[width] = 0;

		if (centerwidth)
			x = margin + (centerwidth - width*scaledHud(8))/2;
		else
			x = margin;


		if (xor)
		{	// Knightmare- text color hack
			Com_sprintf (line, sizeof(line), S_COLOR_ALT"%s", line);
			Hud_DrawStringAlt(x, y, line, 255);
		}
		else
			Hud_DrawString(x, y, line, 255);

		if (*string)
		{
			string++;	// skip the \n
			x = margin;
			y += scaledHud(8);
		}
	}
}


/*
==============
SCR_DrawField
==============
*/
void SCR_DrawField (int x, int y, int color, int width, int value)
{
	char	num[16], *ptr;
	int		l;
	int		frame;

	if (width < 1)
		return;

	// draw number string
	if (width > 5)
		width = 5;

	SCR_AddDirtyPoint (x, y);
	SCR_AddDirtyPoint (x+width*scaledHud(CHAR_WIDTH+2), y+scaledHud(24)-1);

	Com_sprintf (num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;
	x += 2 + scaledHud(CHAR_WIDTH)*(width - l);

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		R_DrawScaledPic (x, y, HudScale(), hud_alpha->value,sb_nums[color][frame]);
		x += scaledHud(CHAR_WIDTH);
		ptr++;
		l--;
	}
}


/*
===============
SCR_TouchPics

Allows rendering code to cache all needed sbar graphics
===============
*/
void SCR_TouchPics (void)
{
	int		i, j;

	for (i=0 ; i<2 ; i++)
		for (j=0 ; j<11 ; j++)
			R_DrawFindPic (sb_nums[i][j]);

	if (crosshair->value)
	{
		if (crosshair->value > 100 || crosshair->value < 0) //Knightmare increased
			crosshair->value = 1;

		Com_sprintf (crosshair_pic, sizeof(crosshair_pic), "ch%i", (int)(crosshair->value));
		R_DrawGetPicSize (&crosshair_width, &crosshair_height, crosshair_pic);
		if (!crosshair_width)
			crosshair_pic[0] = 0;
	}

}

/*
================
SCR_ExecuteLayoutString 

================
*/
void SCR_ExecuteLayoutString (char *s)
{
	int		x, y;
	int		value;
	char	*token;
	char	string[1024];
	int		width;
	int		index;
	clientinfo_t	*ci;

	int		selected;
	int		num,selected_num,i;

	int w, h;	

	if (cls.state != ca_active || !cl.refresh_prepped)
	{
		return;
	}

	if (!s[0])
	{
		return;
	}


	//FULLSCREEN VIGNETTE EFFECT
	R_DrawStretchPic (
		0,
		0,
		viddef.width,
		viddef.height,
		"/pics/vignette.pcx",
		0.8);


	value = cl.frame.playerstate.stats[STAT_HINTTITLE];

    if (value > 0)
	{
		//R_DrawScaledPic (viddef.width/2 - scaledHud(180), viddef.height/2 - scaledHud(45), HudScale() * 0.7, value * 0.01, "hint_title");
		//int title;

		//GravityBone hack. Make the GravityBone title appear if we're in a GB map.
		char	mapfile[32];

		if (cl.configstrings[CS_MODELS+1][0])
		{
			strcpy (mapfile, cl.configstrings[CS_MODELS+1] + 5);	// skip "maps/"
			mapfile[strlen(mapfile)-4] = 0;		// cut off ".bsp"
		}

		if (Q_stricmp( mapfile, "parlo1") == 0) //name of GB map.
		{
			R_DrawScaledPic (viddef.width/2 - scaledHud(180), viddef.height/2 - scaledHud(45), HudScale() * 0.7, value * 0.01, "hint_title");	//GB image
		}
		else
		{
			R_DrawScaledPic (
				viddef.width - 512 - 64,
				viddef.height/2 - 256,
				1,
				value * 0.01,
				"title");

			R_DrawScaledPic (
				viddef.width/2 - 256,
				viddef.height - 148,
				1,
				value * 0.01,
				"copyright");
		}
	}


	//terrible hack. the last drawn thing gets corrupted for some reason. so this is a little dummy picture.
		R_DrawStretchPic (
		0,
		0,
		1,
		1,
		"/pics/ch1.pcx",
		0);

	

	


	value = cl.frame.playerstate.stats[STAT_HUDMSG];

    if (value == 1)
	{
		/*
		R_DrawStretchPic (
		0,
		0,
		viddef.width,
		viddef.height,
		"/pics/drunk.png",
		0.6);
*/

		


			R_DrawScaledPic (
				viddef.width/2 - scaledHud(240),
				viddef.height/2 - scaledHud(16),
				HudScale() * 0.5,
				1.0,
				"contractcomplete");

	}
	else if (value == 2)
	{
		R_DrawScaledPic (
			scaledHud(1),
			viddef.height - scaledHud(64),
			HudScale() * 0.35,
			1.0,
			"find_exit");
	}
	else if (value == 3)
	{
		//TFOL "the end"
		R_DrawScaledPic (
			viddef.width - scaledHud(128),
			viddef.height - scaledHud(128),
			HudScale() * 0.8,
			1.0,
			"drunk");
	}

	

	//selected = cl.frame.playerstate.stats[STAT_SELECTED_ITEM];
	//Com_Printf ("%i\n",cl.frame.playerstate.stats[STAT_SELECTED_ITEM]);


	//===== draw camera viewfinder.
	if (cl.frame.playerstate.stats[STAT_SELECTED_ITEM])
	{
		if (cl.frame.playerstate.stats[STAT_SELECTED_ITEM] == 12)
		{
			R_DrawStretchPic (
				0,
				0,
				viddef.width,
				viddef.height,
				"/pics/viewfinder.tga",
				1.0);


			if (cl.frame.playerstate.stats[STAT_PHOTOCOUNT])
			{
				char pcount[32];
				sprintf(pcount, "%d of %d Photos", cl.frame.playerstate.stats[STAT_PHOTOCOUNT], cl.frame.playerstate.stats[STAT_MAXPHOTOS]);

				//cl.frame.playerstate.stats[STAT_PHOTOCOUNT]
				Hud_DrawString (scaledHud(270), scaledHud(465), pcount, 255);
			}
		}
	}


	//Com_Printf ("%i\n",cl.frame.playerstate.stats[STAT_SELECTED_ITEM]);

	if (cl.frame.playerstate.stats[STAT_WEAPBOX] > 0)
	{
		float adj=1.0;
	

		selected = cl.frame.playerstate.stats[STAT_SELECTED_ITEM];
		num = 0;
		selected_num = 0;
		


		if (cl.frame.playerstate.stats[STAT_WEAPBOX] < 20)
		{
			adj = 0.05 * cl.frame.playerstate.stats[STAT_WEAPBOX];
			
		}

		for (i=0; i<MAX_ITEMS; i++)
		{
			float alpha;
			float boxalpha;
			char block[32];
			char num[2];
			char picname[16];
			float alph;



			if (cl.inventory[i])
			{
				if (i==selected)
				{
					selected_num = num;
					alpha=1.0;
					boxalpha=1.0;
					alph=255;
					
				}
				else
				{
					alpha=0.55;
					boxalpha=0.4;
					alph = 150;
				}
				//Com_Printf ("%s\n",cl.configstrings[CS_ITEMS+i]);





				if (i==9)
					sprintf(picname, "%s", "w_freon");
				else if (i==10)
					sprintf(picname, "%s", "w_hammer");
				else if (i==11)
					sprintf(picname, "%s", "w_drill");
				else
					sprintf(picname, "%s", "w_camera");

			}

			if (cl.frame.playerstate.stats[STAT_WEAPBOX] < 20)
			{
				alpha *= adj;
				boxalpha *= adj;
				alph *= adj;
			}

			sprintf(num, "%i", i-8);
			sprintf(block, "%s", cl.configstrings[CS_ITEMS+i]);
			



			
			if (cl.inventory[i])
			{
				if (i!=selected)
				{
					R_DrawScaledPic (
						scaledHud(560),
						scaledHud(-560+(64*i)),
						HudScale() * 0.45,
						boxalpha, "w_box");
				}
				else
				{
					R_DrawScaledPic (
						scaledHud(560),
						scaledHud(-560+(64*i)),
						HudScale() * 0.45,
						boxalpha, "w_box_sel");
				}

				R_DrawScaledPic (
					scaledHud(560),
					scaledHud(-560+(64*i)),
					HudScale() * 0.45,
					alpha, picname);

				//draw number.
				Hud_DrawString (
					scaledHud(607),
					scaledHud(-535+(64*i)),
					num,
					alph);
			}

			if (i==selected)
			{
				sprintf(picname, "%s%s", picname,"_name");

				R_DrawScaledPic (
					scaledHud(500),
					scaledHud(-560+(64*i)),
					HudScale() * 0.45,
					alpha, picname);
			}
		}
	}


	InitHudScale ();
	x = 0;
	y = 0;
	width = 3;

	while (s)
	{
		token = COM_Parse (&s);
		if (!strcmp(token, "xl"))
		{
			token = COM_Parse (&s);
			x = scaledHud(atoi(token));
			continue;
		}
		if (!strcmp(token, "xr"))
		{
			token = COM_Parse (&s);
			x = viddef.width + scaledHud(atoi(token));
			continue;
		}
		if (!strcmp(token, "xv"))
		{
			token = COM_Parse (&s);
			x = viddef.width/2 - scaledHud(160) + scaledHud(atoi(token));
			continue;
		}
		if (!strcmp(token, "yt"))
		{
			token = COM_Parse (&s);
			y = scaledHud(atoi(token));
			continue;
		}
		if (!strcmp(token, "yb"))
		{
			token = COM_Parse (&s);
			y = viddef.height + scaledHud(atoi(token));
			continue;
		}
		if (!strcmp(token, "yv"))
		{
			token = COM_Parse (&s);
			y = viddef.height/2 - scaledHud(120) + scaledHud(atoi(token));
			continue;
		}

		if (!strcmp(token, "pic"))
		{	// draw a pic from a stat number
			token = COM_Parse (&s);
			value = cl.frame.playerstate.stats[atoi(token)];
			// Knightmare- 1/2/2002- BIG UGLY HACK for old demos or
			// connected to server using old protocol;
			// Changed config strings require different offsets
			if ( LegacyProtocol() )
			{
				if (value >= OLD_MAX_IMAGES) // Knightmare- don't bomb out
				//	Com_Error (ERR_DROP, "Pic >= MAX_IMAGES");
				{
					Com_Printf (S_COLOR_YELLOW"Warning: Pic >= MAX_IMAGES\n");
					value = OLD_MAX_IMAGES-1;
				}
				if (cl.configstrings[OLD_CS_IMAGES+value])
				{
					SCR_AddDirtyPoint (x, y);
					SCR_AddDirtyPoint (x+scaledHud(24)-1, y+scaledHud(24)-1);
					R_DrawScaledPic (x, y, HudScale(), hud_alpha->value, cl.configstrings[OLD_CS_IMAGES+value]);
				}
			}
			else
			{
				if (value >= MAX_IMAGES) // Knightmare- don't bomb out
				//	Com_Error (ERR_DROP, "Pic >= MAX_IMAGES");
				{
					Com_Printf (S_COLOR_YELLOW"Warning: Pic >= MAX_IMAGES\n");
					value = MAX_IMAGES-1;
				}
				if (cl.configstrings[CS_IMAGES+value])
				{
					SCR_AddDirtyPoint (x, y);
					SCR_AddDirtyPoint (x+scaledHud(24)-1, y+scaledHud(24)-1);
					R_DrawScaledPic (x, y, HudScale(), hud_alpha->value, cl.configstrings[CS_IMAGES+value]);
				}
			}
			//end Knightmare
			continue;
		}

		if (!strcmp(token, "client"))
		{	// draw a deathmatch client block
			int		score, ping, time;

			token = COM_Parse (&s);
			x = viddef.width/2 - scaledHud(160) + scaledHud(atoi(token));
			token = COM_Parse (&s);
			y = viddef.height/2 - scaledHud(120) + scaledHud(atoi(token));
			SCR_AddDirtyPoint (x, y);
			SCR_AddDirtyPoint (x+scaledHud(160)-1, y+scaledHud(32)-1);

			token = COM_Parse (&s);
			value = atoi(token);
			if (value >= MAX_CLIENTS || value < 0)
				Com_Error (ERR_DROP, "client >= MAX_CLIENTS");
			ci = &cl.clientinfo[value];

			token = COM_Parse (&s);
			score = atoi(token);

			token = COM_Parse (&s);
			ping = atoi(token);

			token = COM_Parse (&s);
			time = atoi(token);

			Hud_DrawStringAlt (x+scaledHud(32), y, va(S_COLOR_ALT"%s", ci->name), 255);
			Hud_DrawString (x+scaledHud(32), y+scaledHud(8),  "Score: ", 255);
			Hud_DrawStringAlt (x+scaledHud(32+7*8), y+scaledHud(8),  va(S_COLOR_ALT"%i", score), 255);
			Hud_DrawString (x+scaledHud(32), y+scaledHud(16), va("Ping:  %i", ping), 255);
			Hud_DrawString (x+scaledHud(32), y+scaledHud(24), va("Time:  %i", time), 255);

			if (!ci->icon)
				ci = &cl.baseclientinfo;
			R_DrawScaledPic(x, y, HudScale(), hud_alpha->value,  ci->iconname);
			continue;
		}

		if (!strcmp(token, "ctf"))
		{	// draw a ctf client block
			int		score, ping;
			char	block[80];

			token = COM_Parse (&s);
			x = viddef.width/2 - scaledHud(160) + scaledHud(atoi(token));
			token = COM_Parse (&s);
			y = viddef.height/2 - scaledHud(120) + scaledHud(atoi(token));
			SCR_AddDirtyPoint (x, y);
			SCR_AddDirtyPoint (x+scaledHud(160)-1, y+scaledHud(32)-1);

			token = COM_Parse (&s);
			value = atoi(token);
			if (value >= MAX_CLIENTS || value < 0)
				Com_Error (ERR_DROP, "client >= MAX_CLIENTS");
			ci = &cl.clientinfo[value];

			token = COM_Parse (&s);
			score = atoi(token);

			token = COM_Parse (&s);
			ping = atoi(token);
			if (ping > 999)
				ping = 999;

			sprintf(block, "%3d %3d %-12.12s", score, ping, ci->name);

			if (value == cl.playernum)
				Hud_DrawStringAlt (x, y, block, 255);
			else
				Hud_DrawString (x, y, block, 255);
			continue;
		}
		
		if (!strcmp(token, "3tctf")) // Knightmare- 3Team CTF block
		{	// draw a 3Team CTF client block
			int		score, ping;
			char	block[80];

			token = COM_Parse (&s);
			x = viddef.width/2 - scaledHud(160) + scaledHud(atoi(token));
			token = COM_Parse (&s);
			y = viddef.height/2 - scaledHud(120) + scaledHud(atoi(token));
			SCR_AddDirtyPoint (x, y);
			SCR_AddDirtyPoint (x+scaledHud(160)-1, y+scaledHud(32)-1);

			token = COM_Parse (&s);
			value = atoi(token);
			if (value >= MAX_CLIENTS || value < 0)
				Com_Error (ERR_DROP, "client >= MAX_CLIENTS");
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
				Hud_DrawStringAlt (x, y, block, 255);
			else
				Hud_DrawString (x, y, block, 255);
			continue;
		}

		if (!strcmp(token, "picn"))
		{	// draw a pic from a name
			token = COM_Parse (&s);
			SCR_AddDirtyPoint (x, y);
			SCR_AddDirtyPoint (x+scaledHud(24)-1, y+scaledHud(24)-1);
			R_DrawScaledPic (x, y, HudScale(), hud_alpha->value, token);
			continue;
		}
		
		if (!strcmp(token, "picn2"))
		{	// draw a pic from a name
			token = COM_Parse (&s);

			


			SCR_AddDirtyPoint (x, y);

			R_DrawFill2 (0, 0, viddef.width, viddef.height, 0, 0, 0, 180);

			//zzzz
			//SCR_AddDirtyPoint (x+scaledHud(24)-1, y+scaledHud(24)-1);


			//R_DrawScaledPic (scaledHud(640-512), 0, HudScale()*0.5, 1.0, token);
			R_DrawStretchPic (scaledHud(128), 0, scaledHud(512), viddef.height, token, 1.0);
			continue;
		}		

		if (!strcmp(token, "num"))
		{	// draw a number
			token = COM_Parse (&s);
			width = atoi(token);
			token = COM_Parse (&s);
			value = cl.frame.playerstate.stats[atoi(token)];
			SCR_DrawField (x, y, 0, width, value);
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

			if (cl.frame.playerstate.stats[STAT_FLASHES] & 1)
				R_DrawScaledPic (x, y, HudScale(), hud_alpha->value, "field_3");

			SCR_DrawField (x, y, color, width, value);
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

			if (cl.frame.playerstate.stats[STAT_FLASHES] & 4)
				R_DrawScaledPic (x, y, HudScale(), hud_alpha->value, "field_3");

			SCR_DrawField (x, y, color, width, value);
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

			if (cl.frame.playerstate.stats[STAT_FLASHES] & 2)
				R_DrawScaledPic (x, y, HudScale(), hud_alpha->value, "field_3");

			SCR_DrawField (x, y, color, width, value);
			continue;
		}


		if (!strcmp(token, "stat_string"))
		{
			token = COM_Parse (&s);
			index = atoi(token);
			if (index < 0 || index >= MAX_CONFIGSTRINGS)
				Com_Error (ERR_DROP, "Bad stat_string index");
			index = cl.frame.playerstate.stats[index];
			if (index < 0 || index >= MAX_CONFIGSTRINGS)
				Com_Error (ERR_DROP, "Bad stat_string index");
			Hud_DrawString (x, y, cl.configstrings[index], 255);
			continue;
		}

		if (!strcmp(token, "cstring"))
		{
			token = COM_Parse (&s);
			_DrawHUDString (token, x, y, scaledHud(320), 0);
			continue;
		}

		if (!strcmp(token, "string"))
		{
			token = COM_Parse (&s);
			Hud_DrawString (x, y, token, 255);
			continue;
		}

		if (!strcmp(token, "cstring2"))
		{
			token = COM_Parse (&s);
			_DrawHUDString (token, x, y, scaledHud(320),0x80);
			continue;
		}

		if (!strcmp(token, "string2"))
		{
			token = COM_Parse (&s);
			Com_sprintf (string, sizeof(string), S_COLOR_ALT"%s", token);
			Hud_DrawStringAlt (x, y, string, 255);
			continue;
		}

		if (!strcmp(token, "if"))
		{	// draw a number
			token = COM_Parse (&s);
			value = cl.frame.playerstate.stats[atoi(token)];
			if (!value)
			{	// skip to endif
				while (s && strcmp(token, "endif") )
				{
					token = COM_Parse (&s);
				}
			}

			continue;
		}


	}
}


/*
================
SCR_DrawStats

The status bar is a small layout program that
is based on the stats array
================
*/
void SCR_DrawStats (void)
{
	SCR_ExecuteLayoutString (cl.configstrings[CS_STATUSBAR]);
}


/*
================
SCR_DrawLayout

================
*/
#define	STAT_LAYOUTS		13

void SCR_DrawLayout (void)
{
	if (!cl.frame.playerstate.stats[STAT_LAYOUTS])
		return;
	SCR_ExecuteLayoutString (cl.layout);
}

//=======================================================

void DrawDemoMessage (void)
{
	//running demo message
	if ( cl.attractloop && !(cl.cinematictime > 0 && cls.realtime - cl.cinematictime > 1000))
	{
		int len;
		char *message = "Running Demo";
		len = strlen(message);

		SCR_DrawFill2 (0, SCREEN_HEIGHT-(MENU_FONT_SIZE+2), SCREEN_WIDTH, MENU_FONT_SIZE+2, ALIGN_BOTTOM_STRETCH, 60,60,60,255);
		SCR_DrawFill2 (0, SCREEN_HEIGHT-(MENU_FONT_SIZE+3), SCREEN_WIDTH, 1, ALIGN_BOTTOM_STRETCH, 0,0,0,255);
		SCR_DrawString (SCREEN_WIDTH/2-(len/2)*MENU_FONT_SIZE, SCREEN_HEIGHT-(MENU_FONT_SIZE+1), ALIGN_BOTTOM, message, 255);
	}
}

/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.
==================
*/
void SCR_UpdateScreen (void)
{
	int numframes;
	int i;
	float separation[2] = { 0, 0 };

	// if the screen is disabled (loading plaque is up, or vid mode changing)
	// do nothing at all
	if (cls.disable_screen)
	{
		if (cls.download) // Knightmare- don't time out on downloads
			cls.disable_screen = Sys_Milliseconds ();
		if (Sys_Milliseconds() - cls.disable_screen > 120000
			&& cl.refresh_prepped && !(cl.cinematictime > 0)) // Knightmare- dont time out on vid restart
		{
			cls.disable_screen = 0;
			Com_Printf ("Loading plaque timed out.\n");
			return; // Knightmare- moved here for loading screen
		}
		scr_draw_loading = 2; // Knightmare- added for loading screen
	}

	if (!scr_initialized || !con.initialized)
		return;				// not initialized yet

	/*
	** range check cl_camera_separation so we don't inadvertently fry someone's
	** brain
	*/
	if ( cl_stereo_separation->value > 1.0 )
		Cvar_SetValue( "cl_stereo_separation", 1.0 );
	else if ( cl_stereo_separation->value < 0 )
		Cvar_SetValue( "cl_stereo_separation", 0.0 );

	if ( cl_stereo->value )
	{
		numframes = 2;
		separation[0] = -cl_stereo_separation->value / 2;
		separation[1] =  cl_stereo_separation->value / 2;
	}		
	else
	{
		separation[0] = 0;
		separation[1] = 0;
		numframes = 1;
	}

	for ( i = 0; i < numframes; i++ )
	{
		R_BeginFrame( separation[i] );

		if (scr_draw_loading == 2)
		{	//  loading plaque over black screen
			//R_SetPalette(NULL);
			// Knightmare- refresh loading screen
			SCR_DrawLoading ();

			// Knightmare- set back for loading screen
			if (cls.disable_screen)
				scr_draw_loading = 2;

			if (cls.consoleActive)
				Con_DrawConsole (0.5, false);

			//NO FULLSCREEN CONSOLE!!!
			continue;
		} 
		// if a cinematic is supposed to be running, handle menus
		// and console specially
		else if (cl.cinematictime > 0)
		{
			if (cls.key_dest == key_menu)
			{
				/*if (cl.cinematicpalette_active)
				{
					R_SetPalette(NULL);
					cl.cinematicpalette_active = false;
				}*/
				UI_Draw ();
			}
			else
				SCR_DrawCinematic();
		}
		else 
		{
			// Knightmare added- disconnected menu
			if (cls.state == ca_disconnected && cls.key_dest != key_menu && !scr_draw_loading) 
			{
				SCR_EndLoadingPlaque ();	// get rid of loading plaque
				cls.consoleActive = true; // show error in console
				M_Menu_Main_f();
			}

			// make sure the game palette is active
			/*if (cl.cinematicpalette_active)
			{
				R_SetPalette(NULL);
				cl.cinematicpalette_active = false;
			}*/

			// do 3D refresh drawing, and then update the screen
			SCR_CalcVrect ();

			// clear any dirty part of the background
			SCR_TileClear ();

			V_RenderView ( separation[i] );

			SCR_DrawStats ();
			if (cl.frame.playerstate.stats[STAT_LAYOUTS] & 1)
				SCR_DrawLayout ();
			if (cl.frame.playerstate.stats[STAT_LAYOUTS] & 2)
				CL_DrawInventory ();

			SCR_DrawNet ();
			SCR_CheckDrawCenterString ();

			if (scr_timegraph->value)
				SCR_DebugGraph (cls.frametime*300, 0);

			if (scr_debuggraph->value || scr_timegraph->value || scr_netgraph->value)
				SCR_DrawDebugGraph ();

			SCR_DrawPause ();

			if (cl_demomessage->value)
				DrawDemoMessage();

			//Knightmare 12/28/2001- BramBo's FPS counter
			if ((cl_drawfps->value) && (cls.state == ca_active))
				SCR_ShowFPS();

			UI_Draw ();

			SCR_DrawLoading ();
		}
		SCR_DrawConsole ();
	}
	GLimp_EndFrame();
}
