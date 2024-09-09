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

// screen.h

typedef enum
{
	ALIGN_UNSET = 0,
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

typedef struct
{
	float x;
	float y;
	float min;
} screenscale_t;

//
// scripted status bar stuff, from Quake2Evolved
//
#define	STAT_MINUS		10	// num frame for '-' stats digit
#define CHAR_WIDTH		16

#define HUD_MAX_RENAMESETS			8
#define HUD_MAX_ITEMGROUPS			128
#define	HUD_GROUP_MAX_VARIANTS		16
#define	HUD_GROUP_MAX_SKIPIFSTATS	8
#define HUD_SKIPIFSTAT_MAX_CMPS		2
#define	HUD_MAX_DRAWITEMS			32
#define	HUD_ITEM_MAX_SKIPIFSTATS	2
#define HUD_MAX_COLOR_RANGES		8

#define	HUD_STATSTRING			1
#define	HUD_STATNUMBER			2
#define	HUD_STATVALUE			4
#define	HUD_STATMINRANGE		8
#define	HUD_STATMAXRANGE		16
#define	HUD_STATMINRANGECLAMP	32
#define	HUD_STATMAXRANGECLAMP	64
#define	HUD_STATSHADER			128
#define	HUD_WEAPONMODELSHADER	256
#define	HUD_PLAYERNAMESTRING	512

#define HUD_RANGETYPE_ABSOLUTE  0
#define HUD_RANGETYPE_RELATIVE  1

// flags for hud drawing items
#define DSF_FORCECOLOR			1
#define DSF_DROPSHADOW			2
#define DSF_LEFT				4
#define DSF_CENTER				8
#define DSF_RIGHT				16
#define DSF_LOWERCASE			32
#define DSF_UPPERCASE			64
#define	DSF_FLIPX				128
#define	DSF_FLIPY				256
#define DSF_OVERRIDEPATH		512
#define	DSF_RENAMESHADER		1024
#define	DSF_MASKSHADER			2048	// uses shaderMinus for mask image
#define	DSF_ADDITIVESHADER		4096	// specifies additive blending

typedef enum 
{
	HUD_GREATER,
	HUD_LESS,
	HUD_GEQUAL,
	HUD_LEQUAL,
	HUD_EQUAL,
	HUD_NOTEQUAL,
	HUD_AND,
	HUD_NOTAND
} hudCmpFunc_t;

typedef enum
{
	HUD_DRAWPIC,
	HUD_DRAWSTRING,
	HUD_DRAWNUMBER,
	HUD_DRAWBAR
} hudDrawType_t;

typedef enum
{
	HUD_LEFTTORIGHT,
	HUD_RIGHTTOLEFT,
	HUD_BOTTOMTOTOP,
	HUD_TOPTOBOTTOM
} hudFillDir_t;

typedef struct {
	color_t		color;
	color_t		blinkColor;
	float		low;
	float		high;
} colorRangeDef_t;

typedef struct {
	color_t		color;
	int			stat;
	int			bit;
} statFlash_t;

typedef struct {
	char		oldName[MAX_QPATH];
	char		newName[MAX_QPATH];
	unsigned int	oldNameHash;
	unsigned int	newNameHash;
	qboolean	isWildcard;
} hudRenameItem_t;

typedef struct hudRenameCacheItem_s {
	char		oldName[MAX_QPATH];
	char		newName[MAX_QPATH];
	unsigned int	oldNameHash;
	unsigned int	newNameHash;
	struct hudRenameCacheItem_s	*next;
} hudRenameCacheItem_t;

typedef struct {
	char			name[MAX_QPATH];
	int				maxRenameItems;
	int				numRenameItems;
	hudRenameItem_t	*renameItems;
	hudRenameCacheItem_t	*renameCacheHead;
	hudRenameCacheItem_t	*renameCacheTail;
} hudRenameSet_t;

typedef struct {
	char			name[MAX_QPATH];
} hudVariant_t;

typedef struct {
	int				numCmps;
	int				stat[HUD_SKIPIFSTAT_MAX_CMPS];
	hudCmpFunc_t	func[HUD_SKIPIFSTAT_MAX_CMPS];
	int				value[HUD_SKIPIFSTAT_MAX_CMPS];
} hudSkipIfStat_t;

typedef struct {
	char			name[MAX_QPATH];
	hudDrawType_t	type;
	int				rect[4];
	scralign_t		scrnAlignment;
	int				size[2];
	int				range[2];
	float			offset[2];
	float			skew[2];
	float			scroll[2];
	float			padding;
	hudFillDir_t	fillDir;
	color_t			color;
	color_t			blinkColor;
	statFlash_t		statFlash;
	hudSkipIfStat_t	skipIfStats[HUD_ITEM_MAX_SKIPIFSTATS];
	int				numSkipIfStats;
	int				flags;
	int				fromStat;
	int				numColorRanges;
	int				colorRangeType;
	int				colorRangeStat;
	int				colorRangeMaxStat;
	colorRangeDef_t	colorRanges[HUD_MAX_COLOR_RANGES];
	char			string[64];
	int				number;
	int				value;
	char			shader[MAX_QPATH];
	char			shaderMinus[MAX_QPATH];
	int				stringStat;
	int				numberStat;
	int				valueStat;
	int				minRangeStat;
	int				maxRangeStat;
	int				minRangeStatClamp[2];
	int				maxRangeStatClamp[2];
	float			minRangeStatMult;
	float			maxRangeStatMult;
	int				shaderStat;
	char			shaderRenameSet[MAX_QPATH];
} hudDrawItem_t;

typedef struct {
	char			name[MAX_QPATH];
	qboolean		onlyMultiPlayer;
	hudVariant_t	variants[HUD_GROUP_MAX_VARIANTS];
	int				numVariants;
	hudSkipIfStat_t	skipIfStats[HUD_GROUP_MAX_SKIPIFSTATS];
	int				numSkipIfStats;
	hudDrawItem_t	*drawItems[HUD_MAX_DRAWITEMS];
	int				numDrawItems;
} hudItemGroup_t;

typedef struct {
	char			name[MAX_QPATH];
	char			overridePath[MAX_QPATH];
	hudItemGroup_t	*itemGroups[HUD_MAX_ITEMGROUPS];
	int				numItemGroups;
} hudDef_t;
//	end scripted status bar stuff


// Psychospaz's scaled menu stuff
#define SCREEN_WIDTH	640.0f
#define SCREEN_HEIGHT	480.0f
#define STANDARD_ASPECT_RATIO ((float)SCREEN_WIDTH/(float)SCREEN_HEIGHT)

// rendered size of console font - everything adjusts to this...
#define	FONT_SIZE		SCR_ScaledScreen(con_font_size->value)
#define CON_FONT_SCALE	SCR_ScaledScreen(con_font_size->value)/8

#define MENU_FONT_SIZE	8
#define MENU_LINE_SIZE	10
#define HUD_FONT_SIZE	8.0
#define	HUD_CHAR_WIDTH	16

extern	char		*sb_nums[2][11];

extern	float		scr_con_current;
extern	float		scr_conlines;		// lines of console to display

extern	float		scr_letterbox_current;
extern	float		scr_letterbox_lines;		// lines of letterbox to display
extern	qboolean	scr_letterbox_active;
extern	qboolean	scr_hidehud;

extern	qboolean	scr_initialized;	// ready to draw

extern	int			sb_lines;

extern	cvar_t		*scr_viewsize;
extern	cvar_t		*crosshair;
extern	cvar_t		*crosshair_scale;
extern	cvar_t		*crosshair_alpha;
extern	cvar_t		*crosshair_pulse;

extern	vrect_t		scr_vrect;		// position of render window

extern color_t		color_identity;
extern vec4_t		vec4_identity;
extern vec4_t		stCoord_identity;
extern vec4_t		stCoord_default;
extern vec4_t		stCoord_tile;

extern	char		crosshair_pic[MAX_QPATH];
extern	int			crosshair_width, crosshair_height;

//
// cl_screen.c
//
void	SCR_Init (void);
void	SCR_Shutdown (void);
void	SCR_UpdateScreen (void);

void	SCR_SizeUp (void);
void	SCR_SizeDown (void);
void	SCR_CenterPrint (char *str);

qboolean SCR_NeedLoadingPlaque (void);
void	SCR_BeginLoadingPlaque (void);
void	SCR_EndLoadingPlaque (void);

void	SCR_DebugGraph (float value, int color);

void	SCR_TouchPics (void);

void	SCR_RunLetterbox (void);
void	SCR_RunConsole (void);

void	SCR_InitHudScale (void);
float	SCR_ScaledHud (float param);
float	SCR_GetHudScale (void);

void	SCR_InitScreenScale (void);
float	SCR_ScaledScreen (float param);
float	SCR_GetScreenScale (void);
void	SCR_ScaleCoords (float *x, float *y, float *w, float *h, scralign_t align);
void	SCR_GetPicPosWidth (char *pic, int *x, int *w);

void	SCR_DrawFill (float x, float y, float width, float height, scralign_t align, qboolean roundOut, int red, int green, int blue, int alpha);
void	SCR_DrawBorder (float x, float y, float width, float height, float borderSize, scralign_t align, qboolean roundOut, int red, int green, int blue, int alpha);
void	SCR_DrawPic (float x, float y, float width, float height, scralign_t align, qboolean roundOut, char *pic, float alpha);
void	SCR_DrawScaledPic (float x, float y, float scale, qboolean centerCoords, qboolean roundOut, char *pic, float alpha);
void	SCR_DrawLegacyPic (float x, float y, float scale, char *pic, float alpha);
void	SCR_DrawColoredPic (float x, float y, float width, float height, scralign_t align, qboolean roundOut, color_t color, char *pic);
void	SCR_DrawOffsetPic (float x, float y, float width, float height, vec2_t offset, scralign_t align, qboolean roundOut, color_t color, char *pic);
void	SCR_DrawOffsetPicST (float x, float y, float width, float height, vec2_t offset, vec4_t texCorners, scralign_t align, qboolean roundOut, color_t color, char *pic);
void	SCR_DrawScrollPic (float x, float y, float width, float height, vec2_t offset, vec4_t texCorners, vec2_t scroll, scralign_t align, qboolean roundOut, color_t color, char *pic);
void	SCR_DrawMaskedPic (float x, float y, float width, float height, vec2_t offset, vec4_t texCorners, vec2_t scroll, scralign_t align, qboolean roundOut, color_t color, char *pic, char *maskPic);
void	SCR_DrawPicFull (float x, float y, float width, float height, vec2_t offset, vec4_t texCorners, vec2_t scroll, scralign_t align, qboolean roundOut, color_t color, qboolean additive, char *pic, char *maskPic);
void	SCR_DrawTiledPic (float x, float y, float width, float height, scralign_t align, qboolean roundOut, char *pic, float alpha);
void	SCR_DrawChar (float x, float y, scralign_t align, int num, fontslot_t font, int red, int green, int blue, int alpha, qboolean italic, qboolean last);
void	SCR_DrawSizedChar (float x, float y, int size, scralign_t align, int num, fontslot_t font, int red, int green, int blue, int alpha, qboolean italic, qboolean last);
void	SCR_DrawString (float x, float y, int size, scralign_t align, const char *string, fontslot_t font, int alpha);
void	SCR_DrawCrosshair (void);

//
// cl_hud.c
//
void	Hud_DrawString (int x, int y, const char *string, int alpha, qboolean isStatusBar);
void	Hud_DrawStringAlt (int x, int y, const char *string, int alpha, qboolean isStatusBar);
void	Hud_DrawStringFromCharsPic (float x, float y, float w, float h, vec2_t offset, float width, scralign_t align, char *string, color_t color, char *pic, int flags);
void	CL_DrawStatus (void);
void	CL_DrawLayout (void);
void	CL_DrawInventory (void);

//
// cl_hud_script.c
//
void	CL_LoadHud (qboolean startup);
void	CL_FreeHud (void);
void	CL_SetHud (const char *value);
void	CL_SetDefaultHud (void);
void	CL_SetHudVariant (void);
qboolean Hud_ScriptIsLoaded (void);
void	Hud_DrawHud (void);

//
// cl_cin.c
//
void	SCR_PlayCinematic (char *name);
qboolean SCR_DrawCinematic (void);
void	SCR_RunCinematic (void);
void	SCR_StopCinematic (void);
void	SCR_FinishCinematic (void);
