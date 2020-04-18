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
// screen.h

typedef struct
{
	float x;
	float y;
	float min;
} screenscale_t;

//screenscale_t screenScale;


typedef enum
{
	ALIGN_STRETCH,
	ALIGN_STRETCH_ALL,
	ALIGN_CENTER,
	ALIGN_LETTERBOX,
	ALIGN_TOP,
	ALIGN_BOTTOM,
	ALIGN_RIGHT,
	ALIGN_LEFT,
	ALIGN_TOPRIGHT,
	ALIGN_TOPLEFT,
	ALIGN_BOTTOMRIGHT,
	ALIGN_BOTTOMLEFT,
	ALIGN_TOP_STRETCH,
	ALIGN_BOTTOM_STRETCH
} scralign_t;

typedef enum
{
SCALETYPE_CONSOLE,
SCALETYPE_HUD,
SCALETYPE_MENU
} textscaletype_t;


void	SCR_Init (void);

void	SCR_UpdateScreen (void);

void	SCR_SizeUp (void);
void	SCR_SizeDown (void);
void	SCR_CenterPrint (char *str);
void	SCR_BeginLoadingPlaque (void);
void	SCR_EndLoadingPlaque (void);

void	SCR_DebugGraph (float value, int color);

void	SCR_TouchPics (void);

void	SCR_RunLetterbox (void);
void	SCR_RunConsole (void);

void	SCR_InitScreenScale (void);
void	SCR_AdjustFrom640 (float *x, float *y, float *w, float *h, scralign_t align);
float	SCR_ScaledScreen (float param);
float	SCR_GetScreenScale (void);

void	SCR_AdjustFrom640 (float *x, float *y, float *w, float *h, scralign_t align);
void	SCR_DrawFill (float x, float y, float width, float height, scralign_t align, int red, int green, int blue, int alpha);
void	SCR_DrawPic (float x, float y, float width, float height, scralign_t align, char *pic, float alpha);
void	SCR_DrawChar (float x, float y, int size, scralign_t align, int num, int red, int green, int blue, int alpha, qboolean italic, qboolean last);
void	SCR_DrawString (float x, float y, int size, scralign_t align, const char *string, int alpha);
void	SCR_DrawCrosshair (void);


extern	float		scr_con_current;
extern	float		scr_conlines;		// lines of console to display

extern	float		scr_letterbox_current;
extern	float		scr_letterbox_lines;		// lines of letterbox to display
extern	qboolean	scr_letterbox_active;
extern	qboolean	scr_hidehud;

extern	int			sb_lines;

extern	cvar_t		*scr_viewsize;
extern	cvar_t		*crosshair;
extern	cvar_t		*crosshair_scale;
extern	cvar_t		*crosshair_alpha;
extern	cvar_t		*crosshair_pulse;

extern	vrect_t		scr_vrect;		// position of render window

extern	char		crosshair_pic[MAX_QPATH];
extern	int			crosshair_width, crosshair_height;

// Psychospaz's scaled menu stuff
#define SCREEN_WIDTH	640.0f
#define SCREEN_HEIGHT	480.0f
#define STANDARD_ASPECT_RATIO ((float)SCREEN_WIDTH/(float)SCREEN_HEIGHT)

// rendered size of console font - everthing adjusts to this...
#define	FONT_SIZE		SCR_ScaledScreen(con_font_size->value)
#define CON_FONT_SCALE	SCR_ScaledScreen(con_font_size->value)/8.0f

#define MENU_FONT_SIZE	8
#define MENU_LINE_SIZE	10

#define HUD_FONT_SIZE	8.0


//
// scr_cin.c
//
void SCR_PlayCinematic (char *name);
qboolean SCR_DrawCinematic (void);
void SCR_RunCinematic (void);
void SCR_StopCinematic (void);
void SCR_FinishCinematic (void);

