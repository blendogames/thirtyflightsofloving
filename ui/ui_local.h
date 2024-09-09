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

#ifndef __UI_LOCAL_H__
#define __UI_LOCAL_H__

#define MAX_MENUITEMS	64

enum {
	MTYPE_ACTION=0,
	MTYPE_KEYBIND,
	MTYPE_KEYBINDLIST,
	MTYPE_SLIDER,
	MTYPE_PICKER,
#ifndef NOTTHIRTYFLIGHTS
    //This probably doesn't work at all due to being removed and not being replaced upstream - Brad
    MTYPE_ROTATE,
    MTYPE_SEPARATOR,
    MTYPE_SPINCONTROL,
#endif
	MTYPE_CHECKBOX,
	MTYPE_LABEL,
	MTYPE_FIELD,
	MTYPE_IMAGE,
	MTYPE_BUTTON,
	MTYPE_RECTANGLE,
	MTYPE_LISTBOX,
	MTYPE_COMBOBOX,
	MTYPE_LISTVIEW,
	MTYPE_TEXTSCROLL,
	MTYPE_MODELVIEW
};

#define	K_TAB			9
#define	K_ENTER			13
#define	K_ESCAPE		27
#define	K_SPACE			32

// normal keys should be passed as lowercased ascii

#define	K_BACKSPACE		127
#define	K_UPARROW		128
#define	K_DOWNARROW		129
#define	K_LEFTARROW		130
#define	K_RIGHTARROW	131

enum {
	QMF_LEFT_JUSTIFY	= 1 << 0,
	QMF_ALTCOLOR		= 1 << 1,
	QMF_NUMBERSONLY		= 1 << 2,
	QMF_NOLOOP			= 1 << 3,
	QMF_MOUSEONLY		= 1 << 4,
	QMF_COLORIMAGE		= 1 << 5,
	QMF_DISABLED		= 1 << 6,
	QMF_NOINTERACTION	= 1 << 7,
#ifndef NOTTHIRTYFLIGHTS
    QMF_GRAYED          = 1 << 8,
    QMF_HIDDEN          = 1 << 9,
    QMF_SKINLIST        = 1 << 10,
#endif
};

//
#define MENU_SUBTEXT_FONT_SIZE	6
#define MENU_HEADER_FONT_SIZE	10
#define MENU_HEADER_LINE_SIZE	13
#define RCOLUMN_OFFSET  MENU_FONT_SIZE*2	// 16
#define LCOLUMN_OFFSET -MENU_FONT_SIZE*2	// -16
#define FIELD_VOFFSET MENU_FONT_SIZE/2

#define SLIDER_RANGE 10
#define SLIDER_HEIGHT 8	// 10
#define SLIDER_ENDCAP_WIDTH 5 // was 8
#define SLIDER_SECTION_WIDTH 10 // was 8
#define SLIDER_KNOB_WIDTH 5 // was 8
#define SLIDER_V_OFFSET 1	// (SLIDER_HEIGHT-MENU_FONT_SIZE)/2

#define CHECKBOX_WIDTH 10
#define CHECKBOX_HEIGHT 10
#define CHECKBOX_V_OFFSET (CHECKBOX_HEIGHT-MENU_FONT_SIZE)/2
//


typedef struct _tag_menuFramework
{
	int			x, y;

	int			cursor;
	int			nitems;
	int			nslots;
	void		*items[MAX_MENUITEMS]; // 64
	void		*cursorItem;
	float		cursorOscillate_amplitude;	// oscillate dist x
	float		cursorOscillate_timeScale;	// oscillate speed x
	const char	*background;				// background image

	const char	*statusbar;
#ifndef NOTTHIRTYFLIGHTS
    const char  *iconname;
#endif
	const char	*cantOpenMessage;
	const char	*defaultsMessage;
	const char	*applyChangesMessage[3];
	qboolean	hide_statusbar;
	qboolean	isPopup;
	int			grabBindCursor;
	int			bitFlags;			// for bit toggles
	char		*flagCvar;			// for dmflags var
	char		bitFlags_statusbar[128];

	void		(*cursordraw)	(struct _tag_menuFramework *m);
	void		(*drawFunc)		(struct _tag_menuFramework *m);
	const char	*(*keyFunc)		(struct _tag_menuFramework *m, int key);
	qboolean	(*canOpenFunc)	(void *self);
	void		(*onOpenFunc)	(void *unused);
	void		(*onExitFunc)	(void *self);
	void		(*defaultsFunc)	(void);
	void		(*applyChangesFunc) (void);	
} menuFramework_s;

typedef struct
{
	int						type;
	const char				*name;
	const char				*header;
	int						x;
	int						y;
	scralign_t				scrAlign;

	menuFramework_s			*parent;
	int						cursor_offset;
	int						localdata[4];
	int						textSize;
	unsigned				flags;

	float					topLeft[2];		// for mouse pointer detection
	float					botRight[2];	// for mouse pointer detection
	float					dynamicWidth;	// for size of changing list items
	float					dynamicHeight;	// for size of changing list items
	qboolean				isHidden;		// for hiding items
	qboolean				isDisabled;
	qboolean				isExtended;		// for items that have pop-out parts
	qboolean				isCursorItem;	// for cursor items
	int						cursorItemOffset[2];
	const char				*statusbar;
#ifndef NOTTHIRTYFLIGHTS
    const char  *iconname;
#endif

	char					*cvar;			// for option items
	qboolean				cvarNoSave;		// whether cvar is saved manually (apply changes func)
	qboolean				valueChanged;	// whether value has changed from cvar (unchanged)
	qboolean				cvarClamp;		// whether option cvar is clamped
	float					cvarMin;		// cvar clamp range
	float					cvarMax;

	void (*callback)		(void *self);
	void (*mouse2Callback)	(void *self);
	void (*dblClkCallback)	(void *self);
	void (*statusbarfunc)	(void *self);
	void (*ownerdraw)		(void *self);
	void (*cursordraw)		(void *self);
} menuCommon_s;

typedef struct
{
	menuCommon_s generic;

	char		buffer[80];
	int			cursor;
	int			length;
	int			visible_length;
	int			visible_offset;
} menuField_s;

#ifndef NOTTHIRTYFLIGHTS
// I have no clue if these will work but they compile; they're just copies of menuField_s - Brad
typedef struct
{
    menuCommon_s generic;

    char		buffer[80];
    int			cursor;
    int			length;
    int			visible_length;
    int			visible_offset;
} menuSeparator_s;

typedef struct
{
    menuCommon_s generic;

    char		buffer[80];
    int			cursor;
    int			length;
    int			visible_length;
    int			visible_offset;
} menuSpinControl_s;
#endif

typedef struct 
{
	menuCommon_s	generic;

	unsigned int	maxPos;
	int				curPos;
	float			baseValue;
	float			increment;
	float			range;
	float			barTopLeft[2];		// for checking if cursor is directly on slider
//	float			barBotRight[2];
	qboolean		displayAsPercent;
} menuSlider_s;

typedef struct
{
	menuCommon_s	generic;

	qboolean		invertValue;
	int				curValue;
	int				bitFlag;		// for dmflags bit toggle
	const int		*bitFlags;

	const char		**itemNames;
	const char		**itemValues;
	int				numItems;
} menuPicker_s;

typedef struct
{
	menuCommon_s	generic;

	qboolean		invertValue;
	float			baseValue;
	float			increment;
	int				curValue;
	int				bitFlag;		// for dmflags bit toggle
} menuCheckBox_s;

typedef struct
{
	menuCommon_s	generic;
#ifndef NOTTHIRTYFLIGHTS
    const char  *iconname;
#endif

	qboolean		usesMouse2;
} menuAction_s;

typedef struct
{
	menuCommon_s	generic;

	char			*commandName;
	const char		*enter_statusbar;
	int				keys[2];
	qboolean		grabBind;
} menuKeyBind_s;

typedef struct
{
	menuCommon_s	generic;
} menuLabel_s;

typedef struct
{
	menuCommon_s	generic;

	int				width;
	int				drawWidth;
	int				height;
	float			aspectRatio;
	char			*imageName;
//	qboolean		imageValid;

	qboolean		isAnimated;
	char			*animTemplate;
	int				numAnimFrames;
	float			animTimeScale;
	int				start_time;

	float			oscillate_amplitude[2];		// oscillate dist x/y
	float			oscillate_timeScale[2];		// oscillate speed x/y

	byte			alpha;
	int				border;
	byte			borderColor[4];
	byte			imageColor[4];
	qboolean		overrideColor;
	qboolean		hCentered;
	qboolean		vCentered;
	qboolean		useAspectRatio;
} menuImage_s;

typedef struct
{
	menuCommon_s	generic;

	int				width;
	int				drawWidth;
	int				height;
	float			aspectRatio;
	char			*imageName;
	char			*hoverImageName;

	float			oscillate_amplitude[2];			// oscillate dist x/y
	float			oscillate_timeScale[2];			// oscillate speed x/y
	float			hoverOscillate_amplitude[2];	// hover oscillate dist x/y
	float			hoverOscillate_timeScale[2];	// hover oscillate speed x/y

	byte			alpha;
	int				border;
	byte			borderColor[4];
	byte			imageColor[4];
	qboolean		overrideColor;
	qboolean		hCentered;
	qboolean		vCentered;
	qboolean		useAspectRatio;
	qboolean		usesMouse2;
} menuButton_s;

typedef struct
{
	menuCommon_s	generic;

	int				width;
	int				height;
	byte			color[4];
	int				border;
	byte			borderColor[4];
	qboolean		hCentered;
	qboolean		vCentered;
} menuRectangle_s;

#define LIST_SCROLLBAR_SIZE			12
#define LIST_SCROLLBAR_CONTROL_SIZE	12

typedef enum {
	SCROLL_X,
	SCROLL_Y
} scrollType_t;

typedef struct
{
	scrollType_t	scrollType;			// horizontal or vertical
	qboolean		scrollEnabled;
	int				scrollMin;
	int				scrollMax;
	int				scrollNumVisible;
	int				scrollPos;
	float			scrollTopLeft[2];	// for mouse pointer detection
	float			scrollBotRight[2];	// for mouse pointer detection
} widgetScroll_s;

#define	LISTBOX_ITEM_PADDING			((MENU_LINE_SIZE-MENU_FONT_SIZE)/2)

typedef enum {
	LIST_TEXT,
	LIST_IMAGE,
	LIST_TEXTIMAGE
} listType_t;

typedef enum {
	TEXTIMAGE_RIGHT,
	TEXTIMAGE_LEFT,
	TEXTIMAGE_TOP,
	TEXTIMAGE_BOTTOM
} textImageType_t;

typedef struct
{
	int				curValue;			// current selected item
	listType_t		listType;			// text, image, or both
	textImageType_t textImageType;		// alignment of text and image
	int				items_x;			// num of cells across
	int				items_y;			// num of cells tall
	int				numItems;			// # of items
	int				scrollDir;			// X or Y
	int				itemWidth;			// image size, textWidth is added to this
	int				itemHeight;			// image size, textLines is added to this
	int				textWidth;			// in characters
	int				textLines;			// # of lines
	int				itemSpacing;		// spacing between items
	int				itemPadding;		// inside image offset
	int				itemTextSize;		// text size for items
	int				border;				// border width
	byte			borderColor[4];		// color in rgba format
	byte			backColor[4];		// background color in rgba format
	byte			altBackColor[4];	// color for alternating text lines
	byte			underImgColor[4];	// for backing alpha images (crosshair)
	widgetScroll_s	scrollState;

	const char		*background;		// background image
	const char		**itemNames;		// hidden when type is image only
	const char		**itemValues;		// cvar values for each item
	const char		**imageNames;		// image (if any) for each item
	color_t			*imageColors;		// tint for color selection
} scrollList_s;

typedef struct
{
	menuCommon_s	generic;

	int				curValue;
	int				items_y;			// num of items high
	int				itemWidth;			// in characters
	int				itemHeight;			// in lines
	int				itemSpacing;		// spacing between lines
	int				itemTextSize;		// text size for items
	int				border;
	byte			borderColor[4];
	byte			backColor[4];
	byte			altBackColor[4];
	widgetScroll_s	scrollState;

	const char		*background;
	const char		**itemNames;
	const char		**itemValues;
	int				numItems;
} menuListBox_s;

typedef struct keyBindSubitem_s
{
	const char	*commandName;	// actual command text of key bind item
	const char	*displayName;	// title of key bind item
	int			keys[2];
} keyBindSubitem_t;

typedef struct
{
	menuCommon_s		generic;

	int					curValue;
	int					items_y;			// num of items high
	int					lineWidth;			// in characters
	int					itemNameWidth;		// in characters
	int					itemHeight;			// in lines, not used
	int					itemSpacing;		// spacing between lines
	int					itemTextSize;		// text size for items
	int					border;
	byte				borderColor[4];
	byte				backColor[4];
	byte				altBackColor[4];
	widgetScroll_s		scrollState;

	keyBindSubitem_t	*bindList;
	int					numItems;

	qboolean			useCustomBindList;	// uses custom list in ui_customKeyBindList
	qboolean			bindListIsFromFile;
	// in case scripted bind list fails to load, only for hard-coded keyBindList
	keyBindSubitem_t	*bindListBackup;

	const char			*enter_statusbar;
	qboolean			grabBind;
} menuKeyBindList_s;

typedef struct
{
	menuCommon_s	generic;

	int				dropHeight;
	const int		*bitFlags;
	int				curValue;
	int				items_y;		// num of items when extended
	int				itemWidth;		// in characters
	int				itemSpacing;	// spacing between lines
	int				itemTextSize;	// text size for items
	int				border;
	byte			borderColor[4];
	byte			backColor[4];
	widgetScroll_s	scrollState;

	const char		**itemNames;
	const char		**itemValues;
	int				numItems;
} menuComboBox_s;

typedef enum {
	LISTVIEW_IMAGE,
	LISTVIEW_TEXTIMAGE
} listViewType_t;

typedef struct
{
	menuCommon_s	generic; 

	int				curValue;
	int				items_x;		// num of cells across
	int				items_y;		// num of cells tall
	int				scrollDir;
	int				itemWidth;
	int				itemHeight;		// visible text is added to this
	int				itemSpacing;	// spacing between items
	int				itemPadding;	// inside image offset
	int				itemTextSize;	// text size for items
	int				border;
	byte			borderColor[4];
	byte			backColor[4];
	byte			underImgColor[4];	// for backing alpha images
	listViewType_t	listViewType;	// text, image, or both
	widgetScroll_s	scrollState;

	const char		*background;
	const char		**itemNames;	// hidden when type is image only
	const char		**itemValues;
	const char		**imageNames;
	color_t			*imageColors;	// tint for color selection
	int				numItems;
} menuListView_s;

typedef struct
{
	menuCommon_s	generic;

	int				width;
	int				height;
	int				lineSize;
	float			time_scale;
	int				start_time;
	unsigned int	start_line;
	qboolean		initialized;

	char			*fileName;
	char			*fileBuffer;
	const char		**scrollText;
} menuTextScroll_s;

#define MODELVIEW_MAX_MODELS 2
typedef struct
{
	menuCommon_s	generic;

	int				width;
	int				height;
	int				fov;
	qboolean		isMirrored;

	char			*modelName[MODELVIEW_MAX_MODELS];
	char			*skinName[MODELVIEW_MAX_MODELS];
	vec3_t			modelOrigin[MODELVIEW_MAX_MODELS];
	vec3_t			modelBaseAngles[MODELVIEW_MAX_MODELS];
	vec3_t			modelRotation[MODELVIEW_MAX_MODELS];
	int				modelFrame[MODELVIEW_MAX_MODELS];
	int				modelFrameNumbers[MODELVIEW_MAX_MODELS];
//	float			modelFrameTime[MODELVIEW_MAX_MODELS];
	int				entFlags[MODELVIEW_MAX_MODELS];

	int				num_entities;
	qboolean		modelValid[MODELVIEW_MAX_MODELS];
	struct model_s	*model[MODELVIEW_MAX_MODELS];
	struct image_s	*skin[MODELVIEW_MAX_MODELS];
} menuModelView_s;

//=======================================================

// Knightmare- added Psychospaz's menu cursor
#define MENU_CURSOR_BUTTON_MAX 2

#define MENUITEM_NONE			0
#define MENUITEM_ACTION			1
#define MENUITEM_SLIDER			2
#define MENUITEM_PICKER			3
#define MENUITEM_TEXT			4
#define MENUITEM_BUTTON			5
#define MENUITEM_CHECKBOX		6
#define MENUITEM_LISTBOX		7
#define MENUITEM_COMBOBOX		8
#define MENUITEM_LISTVIEW		9
#define MENUITEM_KEYBINDLIST	10
#ifndef NOTTHIRTYFLIGHTS
#define MENUITEM_ROTATE         11
#endif

typedef struct
{
	// only 2 buttons for menus
	float		buttontime[MENU_CURSOR_BUTTON_MAX];
	int			buttonclicks[MENU_CURSOR_BUTTON_MAX];
	int			buttonused[MENU_CURSOR_BUTTON_MAX];
	qboolean	buttondown[MENU_CURSOR_BUTTON_MAX];

	qboolean	mouseaction;

	// this is the active item that cursor is on.
	int			menuitemtype;
	void		*menuitem;
	void		*menu;

	// coords
	int		x;
	int		y;

	int		oldx;
	int		oldy;
} cursor_t;

extern cursor_t ui_mousecursor;

#ifndef NOTTHIRTYFLIGHTS
// Ported from old engine
typedef struct
{
	float	min[2];
	float max[2];
	int index;
} buttonmenuobject_t;

typedef struct
{
	int	min[2];
	int max[2];
	void (*OpenMenu)(void);
} mainmenuobject_t;
#endif

//
// ui_utils.c
//
extern char	**ui_resolution_names;
extern char	**ui_video_modes;

extern char	**ui_aniso_names;
extern char	**ui_aniso_values;

extern char **ui_font_names;
extern int	ui_numfonts;

extern char **ui_hud_names;
extern int	ui_numhuds;

extern char **ui_crosshair_names;
extern char **ui_crosshair_display_names;
extern color_t *ui_crosshair_display_colors;
extern char **ui_crosshair_values;
extern int	ui_numcrosshairs;

//=======================================================

#define UI_CUSTOM_KEYBIND_FILE	"scripts/ui/keybinds.def"

typedef struct {
	char				fileName[MAX_QPATH];
	int					maxKeyBinds;
	int					numKeyBinds;
	keyBindSubitem_t	*bindList;
} keyBindListHandle_t;

extern keyBindListHandle_t ui_customKeyBindList;

//=======================================================

typedef struct {
	char		*gameDir;
	char		*baseGame1;
	char		*baseGame2;
	char		*baseGame3;
	qboolean	quakeImportPathAuto;
	qboolean	quakeRRImportPathAuto;
	char		*quakeImportPath;
	char		*quakeMainGame;
	char		*quakeGame1;
	char		*quakeGame2;
	char		*quakeGame3;
	char		*quakeGame4;

	qboolean	quake2RRImportPathAuto;
	char		*quake2RRImportPath;
	char		*quake2RRMainGame;
	char		*quake2RRGame1;
	char		*quake2RRGame2;
	char		*quake2RRGame3;
	char		*quake2RRGame4;

	qboolean	isUnsupported;
} modInfo_t;

extern modInfo_t *ui_mod_info;
extern char	**ui_mod_names;
extern char	**ui_mod_values;
extern int ui_num_mods;

//=======================================================

// O.o Way too many save games available here - Brad
#ifdef NOTTHIRTYFLIGHTS
#define	UI_MAX_SAVEGAMES	257 // increased from 15, 21, 129
#define	EMPTY_GAME_STRING	"<EMPTY>"
#else
#define	UI_MAX_SAVEGAMES	25
#define	EMPTY_GAME_STRING	"- EMPTY -"
#endif
#define DEFAULT_SAVESHOT_ASPECT	(4.0f/3.0f)

extern char *ui_savegame_names[UI_MAX_SAVEGAMES];
extern char	*ui_loadgame_names[UI_MAX_SAVEGAMES+1];
extern char	ui_savestrings[UI_MAX_SAVEGAMES][64];

//=======================================================

#define UI_MAX_LOCAL_SERVERS 12
#ifdef NOTTHIRTYFLIGHTS
#define UI_LOCAL_SERVER_NAMELEN 64
#else
#define UI_LOCAL_SERVER_NAMELEN 80
#endif
#define	NO_SERVER_STRING	"<no server>"

extern int		ui_num_servers;

// user readable information
//extern char ui_local_server_names[UI_MAX_LOCAL_SERVERS][UI_LOCAL_SERVER_NAMELEN];
//extern char	*ui_serverlist_names[UI_MAX_LOCAL_SERVERS+1];

// network address
extern netadr_t ui_local_server_netadr[UI_MAX_LOCAL_SERVERS];

//=======================================================

// server map list stuff goes here
#define MAX_ARENAS 4096
#define MAX_ARENAS_TEXT 8192

#define M_UNSET 0
#define	M_MISSING 1
#define	M_FOUND 2

typedef enum {
	MAP_DM,
	MAP_COOP,
	MAP_CTF,
	MAP_3TCTF,
	NUM_MAPTYPES
} maptype_t;

typedef struct {
	int		index;
	char	*tokens;
} gametype_names_t;

extern maptype_t ui_svr_maptype;
extern char **ui_svr_mapnames;
extern int	ui_svr_listfile_nummaps;

//=======================================================

#define MAX_DISPLAYNAME 16
#define MAX_PLAYERMODELS 1024

typedef struct
{
	int		nskins;
	char	**skinDisplayNames;
	char	**skinIconNames;
	char	displayname[MAX_DISPLAYNAME];
	char	directory[MAX_QPATH];
} playermodelinfo_s;

extern playermodelinfo_s ui_pmi[MAX_PLAYERMODELS];
extern char *ui_pmnames[MAX_PLAYERMODELS];
extern int ui_numplayermodels;

extern char	ui_playerconfig_playermodelname[MAX_QPATH];
extern char	ui_playerconfig_playerskinname[MAX_QPATH];
extern char	ui_playerconfig_weaponmodelname[MAX_QPATH];
/*
extern color_t ui_player_color_imageColors[];
extern char **ui_player_color_values;
extern char **ui_player_color_imageNames;
extern int ui_numplayercolors;
*/
//=======================================================

void UI_TextColor (int colornum, qboolean scrollbar, int *red, int *green, int *blue);
void UI_TextColorHighlight (int colornum, int *red, int *green, int *blue);
qboolean UI_IsValidImageFilename (char *name);

// Forward ported from old engine - Brad
#ifndef NOTTHIRTYFLIGHTS
void UI_ClampCvar (const char *varName, float cvarMin, float cvarMax);
#endif

float UI_GetScreenAspect (void);
void UI_ClampCvarForControl (menuCommon_s *item);
int UI_GetCurValueForControl (menuCommon_s *item);
int	UI_GetIndexForStringValue (const char **item_values, char *value);
int UI_MouseOverAlpha (menuCommon_s *m);
qboolean UI_MouseOverSubItem (int x, int y, int w, int h, scralign_t align);
void UI_UnbindCommand (char *command);
void UI_FindKeysForCommand (char *command, int *twokeys);
void *UI_ItemAtMenuCursor (menuFramework_s *menu);
void UI_SetMenuStatusBar (menuFramework_s *menu, const char *string);
void UI_SetMenuCurrentItemStatusBar (menuFramework_s *menu, const char *string);
int	 UI_TallyMenuSlots (menuFramework_s *menu);

void UI_StartSPGame (void);
void UI_StartServer (char *startmap, qboolean dedicated);
void UI_LoadMod (char *modName);
void UI_LoadModFromList (int index);

void UI_GetVideoInfo (void);
void UI_FreeVideoInfo (void);

void UI_LoadFontNames (void);
void UI_FreeFontNames (void);

void UI_LoadHudNames (void);
void UI_FreeHudNames (void);

void UI_SortCrosshairs (char **list, int len);
void UI_UpdateCrosshairColors (int inRed, int inGreen, int inBlue);
void UI_LoadCrosshairs (void);
void UI_FreeCrosshairs (void);

void UI_LoadKeyBindList (void);
void UI_FreeKeyBindList (void);

//void UI_GetModList (void);
//void UI_FreeModList (void);

float UI_UpdateSaveshotAspect (int index);
char *UI_UpdateSaveshot (int index);
void UI_UpdateSavegameData (void);
void UI_InitSavegameData (void);
qboolean UI_SaveIsValid (int index);
qboolean UI_SaveshotIsValid (int index);
qboolean UI_CanOpenSaveMenu (void *unused);

void UI_AddToServerList (netadr_t adr, char *info);
void UI_SearchLocalGames (void);
void UI_JoinServer (int index);
void UI_InitServerList (void);

void UI_LoadArenas (void);
void UI_LoadMapList (void);
void UI_FreeMapList (void);
void UI_UpdateMapList (maptype_t maptype);
char *UI_UpdateStartSeverLevelshot (int index);
void UI_SetCoopMenuMode (qboolean value);
qboolean UI_Coop_MenuMode (void);
qboolean UI_CanOpenDMFlagsMenu (void *unused);
void UI_SetCTFMenuMode (qboolean value);
qboolean UI_CTF_MenuMode (void);

void UI_LoadPlayerModels (void);
void UI_FreePlayerModels (void);
void UI_RefreshPlayerModels (void);
int UI_PlayerModelCmpFunc (const void *_a, const void *_b);
void UI_InitPlayerModelInfo (int *modelNum, int *skinNum);
void UI_UpdatePlayerModelInfo (int mNum, int sNum);
void UI_UpdatePlayerSkinInfo (int mNum, int sNum);
qboolean UI_HaveValidPlayerModels (void *unused);

//
// ui_widgets.c
//
qboolean UI_MenuField_Key (menuField_s *field, int key);
const char *UI_MenuKeyBind_Key (menuKeyBind_s *k, int key);
const char *UI_MenuKeyBindList_Key (menuKeyBindList_s *k, int key);

// Forward ported from old engine - Brad
#ifndef NOTTHIRTYFLIGHTS
void	UI_MenuSpinControl_SetValue (menuPicker_s *s, const char *varName, float cvarMin, float cvarMax, qboolean clamp);
void	UI_MenuSpinControl_SaveValue (menuPicker_s *s, const char *varName);
const char *UI_MenuSpinControl_GetValue (menuPicker_s *s);
#endif

void UI_ReregisterMenuItem (void *item);
void UI_UpdateMenuItemCoords (void *item);
qboolean UI_ItemCanBeCursorItem (void *item);
qboolean UI_ItemIsValidCursorPosition (void *item);
qboolean UI_ItemHasMouseBounds (void *item);
char *UI_GetMenuItemValue (void *item);
void UI_SetMenuItemValue (void *item);
void UI_SaveMenuItemValue (void *item);
void UI_DrawMenuItem (void *item);
void UI_DrawMenuItemExtension (void *item);
void UI_SetMenuItemDynamicSize (void *item);
int	UI_GetItemMouseoverType (void *item);
char *UI_ClickMenuItem (menuCommon_s *item, qboolean mouse2);
void UI_InitMenuItem (void *item);
qboolean UI_SelectMenuItem (menuFramework_s *s);

#ifndef NOTTHIRTYFLIGHTS
// Forward ported from old engine - Brad
qboolean UI_MouseSelectItem (menuCommon_s *item);
#endif

char *UI_SlideMenuItem (menuFramework_s *s, int dir);
qboolean UI_ScrollMenuItem (menuFramework_s *s, int dir);

//
// ui_mouse.c
//
void UI_RefreshCursorButtons (void);
void UI_RefreshCursorMenu (void);
void UI_RefreshCursorLink (void);
void UI_Mouseover_Check (menuFramework_s *menu);
void UI_MouseCursor_Think (void);

//
// ui_menu.c
//
#define	MAX_MENU_DEPTH	8

typedef struct
{
	menuFramework_s *menu;
} menulayer_t;

extern menulayer_t	ui_layers[MAX_MENU_DEPTH];
extern int			ui_menudepth;
extern menulayer_t	ui_menuState;

#ifndef NOTTHIRTYFLIGHTS
void UI_AddButton (buttonmenuobject_t *button, int index, float x, float y, float w, float h);
void UI_AddMainButton (mainmenuobject_t *button, int index, float x, float y, char *name);
#endif
void UI_PushMenu (menuFramework_s *menu);
void UI_ForceMenuOff (void);
void UI_PopMenu (void);
void UI_BackMenu (void *item);
void UI_CheckAndPopMenu (menuFramework_s *menu);
void UI_LoadMenuBitFlags (menuFramework_s *menu);
void UI_SetMenuBitFlags (menuFramework_s *menu, int bit, qboolean set);
void UI_AddMenuItem (menuFramework_s *menu, void *item);
void UI_SetGrabBindItem (menuFramework_s *menu, menuCommon_s *c);
void UI_ClearGrabBindItem (menuFramework_s *menu);
qboolean UI_HasValidGrabBindItem (menuFramework_s *menu);
void UI_RefreshMenuItems (void);
void UI_SetMenuItemValues (menuFramework_s	*menu);
void UI_SetMenuDefaults (void);
const char *UI_GetDefaultsMessage (void);
void UI_Defaults_Popup (void *unused);
void UI_ApplyMenuChanges (void);
const char *UI_GetApplyChangesMessage (int line);
void UI_ApplyChanges_Popup (void *unused);
void UI_DrawMenu (menuFramework_s *menu);
void UI_AdjustMenuCursor (menuFramework_s *menu, int dir);
void UI_DefaultMenuDraw (menuFramework_s *menu);
const char *UI_DefaultMenuKey (menuFramework_s *menu, int key);
const char *UI_QuitMenuKey (menuFramework_s *menu, int key);

//
// ui_main.c
//
void UI_Draw (void);
void UI_Keydown (int key);
qboolean UI_MenuHasGrabBind (void);
void UI_RootMenu (void);
void UI_Precache (void);
void UI_Init (void);
void UI_Shutdown (void);
void UI_RefreshData (void);

//
// ui_draw.c
//
#define UI_DrawFill										SCR_DrawFill
#define UI_DrawBorder									SCR_DrawBorder
#define UI_DrawPic										SCR_DrawPic
#define UI_DrawColoredPic								SCR_DrawColoredPic
#define UI_DrawOffsetPic								SCR_DrawOffsetPic
#define UI_DrawPicST(x, y, w, h, st, a, r, c,  p)		SCR_DrawOffsetPicST(x, y, w, h, vec2_origin, st, a, r, c, p)
#define UI_DrawOffsetPicST								SCR_DrawOffsetPicST
#define UI_DrawTiledPic									SCR_DrawTiledPic
#define UI_DrawChar(x, y, s, t, n, r, g, b, a, i, l)	SCR_DrawSizedChar(x, y, s, t, n, FONT_UI, r, g, b, a, i, l)
#define UI_DrawString									SCR_DrawString

void UI_DrawMenuString (int x, int y, int size, scralign_t align, const char *string, int alpha, qboolean R2L, qboolean altColor);
void UI_DrawMenuStatusBar (const char *string);
void UI_DrawMenuNullCursor (void *self);
void UI_DrawMenuTextBox (int x, int y, int width, int lines);
void UI_DrawPopupMessage (char *message);
void UI_Draw_Cursor (void);

//
// menu_main.c
//
#define NUM_MAINMENU_CURSOR_FRAMES 15

//
// Global
//
#define MOUSEBUTTON1 0
#define MOUSEBUTTON2 1

#define LOADSCREEN_NAME		"/gfx/ui/unknownmap.pcx"
#define UI_BACKGROUND_NAME	"/gfx/ui/menu_background.pcx"
#define UI_NOSCREEN_NAME	"/gfx/ui/noscreen.pcx"

// old mouse cursors, no longer used
#define UI_MOUSECURSOR_MAIN_PIC		"/gfx/ui/cursors/m_cur_main.pcx"
#define UI_MOUSECURSOR_HOVER_PIC	"/gfx/ui/cursors/m_cur_hover.pcx"
#define UI_MOUSECURSOR_CLICK_PIC	"/gfx/ui/cursors/m_cur_click.pcx"
#define UI_MOUSECURSOR_OVER_PIC		"/gfx/ui/cursors/m_cur_over.pcx"
#define UI_MOUSECURSOR_TEXT_PIC		"/gfx/ui/cursors/m_cur_text.pcx"

#define UI_MOUSECURSOR_PIC			"/gfx/ui/cursors/m_mouse_cursor.pcx"
#define UI_ITEMCURSOR_DEFAULT_PIC	"/gfx/ui/cursors/cursor_menuitem_default.pcx"
#define UI_ITEMCURSOR_KEYBIND_PIC	"/gfx/ui/cursors/cursor_menuitem_keybind.pcx"
#define UI_ITEMCURSOR_BLINK_PIC		"/gfx/ui/cursors/cursor_menuitem_blink.pcx"

#define UI_CHECKBOX_PIC				"/gfx/ui/widgets/checkbox.pcx"
#define UI_FIELD_PIC				"/gfx/ui/widgets/field.pcx"
#define UI_TEXTBOX_PIC				"/gfx/ui/widgets/textbox.pcx"
#define UI_SLIDER_PIC				"/gfx/ui/widgets/slider.pcx"
#define UI_ARROWS_PIC				"/gfx/ui/widgets/arrows.pcx"
#define UI_SCROLLKNOB_PIC			"/gfx/ui/widgets/scroll_knobs.pcx"

#define UI_DEFAULTS_MESSAGE			"Reset all menu settings to defaults?"
#define UI_APPLYCHANGES_MESSAGE		"This will apply changes for this menu."
#define UI_APPLYCHANGES_MESSAGE2	"Continue?"

#define UI_ITEMVALUE_WILDCARD		"???"
#define	UI_CUSTOMCOLOR_PIC			"/gfx/ui/misc/custom_color.pcx"
#define	UI_SOLIDWHITE_PIC			"/gfx/ui/misc/solidwhite.pcx"
#define	UI_RAILCORE_PIC				"/gfx/ui/misc/rail_core.pcx"
#define	UI_RAILSPIRAL_PIC			"/gfx/ui/misc/rail_spiral.pcx"

extern	cvar_t	*ui_sensitivity;
extern	cvar_t	*ui_background_alpha;
extern	cvar_t	*ui_debug_itembounds;
extern	cvar_t	*ui_item_rotate;
extern	cvar_t	*ui_cursor_scale;
extern	cvar_t	*ui_new_textbox;
extern	cvar_t	*ui_new_textfield;
// Temp cvars for setting color1 in player config menu
extern	cvar_t	*ui_player_railred;
extern	cvar_t	*ui_player_railgreen;
extern	cvar_t	*ui_player_railblue;

// moved these declarations to ui_main.c to avoid redundancy
extern	char *ui_menu_null_sound;
extern	char *ui_menu_in_sound;
extern	char *ui_menu_move_sound;
extern	char *ui_menu_out_sound;
extern	char *ui_menu_drag_sound;

extern qboolean	ui_entersound;		// play after drawing a frame, so caching
									// won't disrupt the sound
#ifndef NOTTHIRTYFLIGHTS
extern int MainMenuMouseHover;
#endif

void Menu_Main_f (void);
	void Menu_Game_f (void);
		void Menu_LoadGame_f (void);
		void Menu_SaveGame_f (void);
		void Menu_PlayerConfig_f (void);
			void Menu_DownloadOptions_f (void);
		void Menu_Credits_f( void );
	void Menu_Multiplayer_f( void );
		void Menu_JoinServer_f (void);
			void Menu_AddressBook_f( void );
		void Menu_StartServer_f (void);
			void Menu_DMOptions_f (void);
		void Menu_PlayerConfig_f (void);
		void Menu_DownloadOptions_f (void);
	void Menu_Video_f (void);
		void Menu_Video_Advanced_f (void);
	void Menu_Mods_f (void);
	void Menu_Options_f (void);
		void Menu_Options_Sound_f (void);
		void Menu_Options_Controls_f (void);
			void Menu_Keys_f (void);
		void Menu_Options_Screen_f (void);
		void Menu_Options_Effects_f (void);
		void Menu_Options_Interface_f (void);
	void Menu_Quit_f (void);
	void Menu_DefaultsConfirm_f (void);
	void Menu_ApplyChanges_f (void);

#endif	// __UI_LOCAL_H__
