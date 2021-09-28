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

#define MAXMENUITEMS	64

#define MTYPE_SLIDER		0
#define MTYPE_LIST			1
#define MTYPE_ACTION		2
#define MTYPE_SPINCONTROL	3
#define MTYPE_SEPARATOR  	4
#define MTYPE_FIELD			5
#define MTYPE_KEYBIND		6

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

#define QMF_LEFT_JUSTIFY	0x00000001
#define QMF_GRAYED			0x00000002
#define QMF_NUMBERSONLY		0x00000004
#define QMF_SKINLIST		0x00000008
#define QMF_HIDDEN			0x00000010
#define QMF_ALTCOLOR		0x00000020

//
#define MENU_SUBTEXT_FONT_SIZE	6
#define MENU_HEADER_FONT_SIZE	10
#define MENU_HEADER_LINE_SIZE	13
#define RCOLUMN_OFFSET  MENU_FONT_SIZE*2	// was 16
#define LCOLUMN_OFFSET -MENU_FONT_SIZE*2	// was -16

#define SLIDER_RANGE 10
#define SLIDER_HEIGHT 8
#define SLIDER_ENDCAP_WIDTH 5 // was 8
#define SLIDER_SECTION_WIDTH 10 // was 8
#define SLIDER_KNOB_WIDTH 5 // was 8
#define SLIDER_V_OFFSET (SLIDER_HEIGHT-MENU_FONT_SIZE)/2
//


typedef struct _tag_menuframework
{
	int x, y;
	int	cursor;

	int	nitems;
	int nslots;
	void *items[64];

	const char	*statusbar;
	int			grabBindCursor;

	void (*cursordraw)( struct _tag_menuframework *m );
	
} menuframework_s;

typedef struct
{
	int type;
	const char *name;
	int x, y;
	menuframework_s *parent;
	int cursor_offset;
	int	localdata[4];
	int textSize;
	unsigned flags;

	const char *statusbar;

	void (*callback)( void *self );
	void (*statusbarfunc)( void *self );
	void (*ownerdraw)( void *self );
	void (*cursordraw)( void *self );
} menucommon_s;

typedef struct
{
	menucommon_s generic;

	char		buffer[80];
	int			cursor;
	int			length;
	int			visible_length;
	int			visible_offset;
} menufield_s;

typedef struct 
{
	menucommon_s	generic;

	// Knightmare added
	unsigned int	maxPos;
	int				curPos;
	float			baseValue;
	float			increment;
	// end Knightmare

	float			range;
	qboolean		displayAsPercent;
} menuslider_s;

typedef struct
{
	menucommon_s	generic;

	qboolean	invertValue;	// Knightmare added
	int			curValue;

	const char	**itemNames;
	const char	**itemValues;	// Knightmare added
	int			numItems;
} menulist_s;

typedef struct
{
	menucommon_s generic;
} menuaction_s;

typedef struct
{
	menucommon_s generic;
} menuseparator_s;

typedef struct
{
	menucommon_s	generic;

	char			*commandName;
	const char		*enter_statusbar;
	int				keys[2];
	qboolean		grabBind;
} menukeybind_s;

typedef enum {
	LISTBOX_TEXT,
	LISTBOX_IMAGE,
	LISTBOX_TEXTIMAGE
} listboxtype_t;

typedef enum {
	SCROLL_X,
	SCROLL_Y
} scrolltype_t;

typedef struct
{
	menucommon_s	generic;

	listboxtype_t	type;
	scrolltype_t	scrollType;
	int			items_x;
	int			items_y;
	int			item_width;
	int			item_height;
	int			scrollPos;
	int			curValue;

	const char	**itemNames;
	int			numItems;
} menulistbox_s;

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

//=======================================================

// Knightmare- added Psychospaz's menu cursor
//cursor - psychospaz
#define MENU_CURSOR_BUTTON_MAX 2

#define MENUITEM_ACTION		1
#define MENUITEM_ROTATE		2
#define MENUITEM_SLIDER		3
#define MENUITEM_TEXT		4
#define MENUITEM_KEYBIND	5

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

//
// ui_utils.c
//
extern char	**ui_resolution_names;
extern char	**ui_video_modes;

extern char	**ui_aniso_names;
extern char	**ui_aniso_values;

//extern char	**ui_mod_names;
//extern char	**ui_mod_values;

extern char **ui_font_names;
extern int	ui_numfonts;

//extern char **ui_hud_names;
//extern int	ui_numhuds;

extern char **ui_crosshair_names;
extern char **ui_crosshair_display_names;
extern char **ui_crosshair_values;
extern int	ui_numcrosshairs;

//=======================================================

#define	UI_MAX_SAVEGAMES	25 // was 15, 21
#define	EMPTY_GAME_STRING	"<EMPTY>"

extern char		ui_savestrings[UI_MAX_SAVEGAMES][32];
extern qboolean	ui_savevalid[UI_MAX_SAVEGAMES+1];
extern qboolean	ui_saveshotvalid[UI_MAX_SAVEGAMES+1];

//=======================================================

#define UI_MAX_LOCAL_SERVERS 12
#define UI_LOCAL_SERVER_NAMELEN 80
#define	NO_SERVER_STRING	"<no server>"

extern int		ui_num_servers;

// user readable information
extern char ui_local_server_names[UI_MAX_LOCAL_SERVERS][UI_LOCAL_SERVER_NAMELEN];
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

extern struct model_s *ui_playermodel;
extern struct model_s *ui_weaponmodel;
extern struct image_s *ui_playerskin;

//extern char	ui_playerconfig_playermodelname[MAX_QPATH];
//extern char	ui_playerconfig_playerskinname[MAX_QPATH];
//extern char	ui_playerconfig_weaponmodelname[MAX_QPATH];

//=======================================================

qboolean UI_IsValidImageFilename (char *name);
void UI_ClampCvar (const char *varName, float cvarMin, float cvarMax);
int	UI_GetIndexForStringValue (const char **item_values, char *value);
int UI_MouseOverAlpha (menucommon_s *m);
void UI_UnbindCommand (char *command);
void UI_FindKeysForCommand (char *command, int *twokeys);
void *UI_ItemAtMenuCursor (menuframework_s *m);
void UI_SetMenuStatusBar (menuframework_s *s, const char *string);
int	 UI_TallyMenuSlots (menuframework_s *menu);

void UIStartSPGame (void);
void UI_StartServer (char *startmap, qboolean dedicated);
void UI_LoadMod (char *modName);

void UI_GetVideoInfo (void);
void UI_FreeVideoInfo (void);

//void UI_GetModList (void);
//void UI_FreeModList (void);

void UI_LoadFontNames (void);
void UI_FreeFontNames (void);

//void UI_LoadHudNames (void);
//void UI_FreeHudNames (void);

void UI_SortCrosshairs (char **list, int len);
void UI_LoadCrosshairs (void);
void UI_FreeCrosshairs (void);

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
extern	vec4_t stCoord_arrow_left;
extern	vec4_t stCoord_arrow_right;
extern	vec4_t stCoord_arrow_up;
extern	vec4_t stCoord_arrow_down;

qboolean UI_MenuField_Key (menufield_s *field, int key);
const char *UI_MenuKeyBind_Key (menukeybind_s *k, int key);

void	UI_MenuSlider_SetValue (menuslider_s *s, const char *varName, float cvarMin, float cvarMax, qboolean clamp);
void	UI_MenuSlider_SaveValue (menuslider_s *s, const char *varName);
float	UI_MenuSlider_GetValue (menuslider_s *s);
void	UI_MenuSpinControl_SetValue (menulist_s *s, const char *varName, float cvarMin, float cvarMax, qboolean clamp);
void	UI_MenuSpinControl_SaveValue (menulist_s *s, const char *varName);
const char *UI_MenuSpinControl_GetValue (menulist_s *s);

void	UI_DrawMenuItem (void *item);
qboolean UI_SelectMenuItem (menuframework_s *s);
qboolean UI_MouseSelectItem (menucommon_s *item);
void	UI_SlideMenuItem (menuframework_s *s, int dir);

//
// ui_mouse.c
//
void UI_RefreshCursorButtons (void);
void UI_RefreshCursorMenu (void);
void UI_RefreshCursorLink (void);
void UI_Mouseover_Check (menuframework_s *menu);
void UI_MouseCursor_Think (void);

//
// ui_menu.c
//
#define	MAX_MENU_DEPTH	8

typedef struct
{
	void	(*draw) (void);
	const char *(*key) (int k);
} menulayer_t;

extern menulayer_t	ui_layers[MAX_MENU_DEPTH];
extern int			ui_menudepth;
extern void	(*m_drawfunc) (void);
extern const char *(*m_keyfunc) (int key);

void UI_AddButton (buttonmenuobject_t *thisObj, int index, float x, float y, float w, float h);
void UI_AddMainButton (mainmenuobject_t *thisObj, int index, int x, int y, char *name);
void UI_PushMenu ( void (*draw) (void), const char *(*key) (int k) );
void UI_ForceMenuOff (void);
void UI_PopMenu (void);
void UI_BackMenu (void *unused);
void UI_AddMenuItem (menuframework_s *menu, void *item);
void UI_SetGrabBindItem (menuframework_s *m, menucommon_s *c);
void UI_ClearGrabBindItem (menuframework_s *m);
qboolean UI_HasValidGrabBindItem (menuframework_s *m);
void UI_AdjustMenuCursor (menuframework_s *menu, int dir);
void UI_CenterMenu (menuframework_s *menu);
void UI_DrawMenu (menuframework_s *menu);

const char *UI_DefaultMenuKey (menuframework_s *m, int key);

//
// ui_main.c
//
void UI_Draw (void);
void UI_Keydown (int key);
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
#define UI_DrawPicST(x, y, w, h, st, a, r, c, p)		SCR_DrawOffsetPicST(x, y, w, h, vec2_origin, st, a, r, c, p)
#define UI_DrawOffsetPicST								SCR_DrawOffsetPicST
#define UI_DrawTiledPic									SCR_DrawTiledPic
#define UI_DrawChar(x, y, s, t, n, r, g, b, a, i, l)	SCR_DrawChar(x, y, s, t, n, FONT_UI, r, g, b, a, i, l)

void UI_DrawMenuString (int x, int y, int size, const char *string, int alpha, qboolean R2L, qboolean altColor);
void UI_DrawString (int x, int y, int size, const char *string, int alpha);
void UI_DrawStringDark (int x, int y, int size, const char *string, int alpha);
void UI_DrawStringR2L (int x, int y, int size, const char *string, int alpha);
void UI_DrawStringR2LDark (int x, int y, int size, const char *string, int alpha);
void UI_DrawMenuStatusBar (const char *string);
void UI_DrawTextBox (int x, int y, int width, int lines);
void UI_DrawBanner (char *name);
void UI_Draw_Cursor (void);

//
// ui_main.c
//
#define NUM_MAINMENU_CURSOR_FRAMES 15

#define MOUSEBUTTON1 0
#define MOUSEBUTTON2 1

#define LOADSCREEN_NAME		"/gfx/ui/unknownmap.pcx"
#define UI_BACKGROUND_NAME	"/gfx/ui/menu_background.pcx"
#define UI_NOSCREEN_NAME	"/gfx/ui/noscreen.pcx"

#define UI_MOUSECURSOR_MAIN_PIC		"/gfx/ui/cursors/m_cur_main.pcx"
#define UI_MOUSECURSOR_HOVER_PIC	"/gfx/ui/cursors/m_cur_hover.pcx"
#define UI_MOUSECURSOR_CLICK_PIC	"/gfx/ui/cursors/m_cur_click.pcx"
#define UI_MOUSECURSOR_OVER_PIC		"/gfx/ui/cursors/m_cur_over.pcx"
#define UI_MOUSECURSOR_TEXT_PIC		"/gfx/ui/cursors/m_cur_text.pcx"

#define UI_MOUSECURSOR_PIC			"/gfx/ui/cursors/m_mouse_cursor.pcx"
#define UI_ITEMCURSOR_DEFAULT_PIC	"/gfx/ui/cursors/cursor_menuitem_default.pcx"
#define UI_ITEMCURSOR_KEYBIND_PIC	"/gfx/ui/cursors/cursor_menuitem_keybind.pcx"
#define UI_ITEMCURSOR_BLINK_PIC		"/gfx/ui/cursors/cursor_menuitem_blink.pcx"

#define UI_CHECKBOX_ON_PIC			"/gfx/ui/widgets/checkbox_on.pcx"
#define UI_CHECKBOX_OFF_PIC			"/gfx/ui/widgets/checkbox_off.pcx"
#define UI_FIELD_PIC				"/gfx/ui/widgets/field.pcx"
#define UI_TEXTBOX_PIC				"/gfx/ui/widgets/textbox.pcx"
#define UI_SLIDER_PIC				"/gfx/ui/widgets/slider.pcx"
#define UI_ARROWS_PIC				"/gfx/ui/widgets/arrows.pcx"

#define UI_ITEMVALUE_WILDCARD		"???"

extern	cvar_t	*ui_sensitivity;
extern	cvar_t	*ui_background_alpha;
extern	cvar_t	*ui_item_rotate;
extern	cvar_t	*ui_cursor_scale;
extern	cvar_t	*ui_new_textbox;
extern	cvar_t	*ui_new_textfield;

// moved these declarations to ui_subsystem.c to avoid redundancy
extern	char *ui_menu_null_sound;
extern	char *ui_menu_in_sound;
extern	char *ui_menu_move_sound;
extern	char *ui_menu_out_sound;
extern	char *ui_menu_drag_sound;

extern qboolean	ui_entersound;		// play after drawing a frame, so caching
									// won't disrupt the sound

int MainMenuMouseHover;

void Menu_Main_Draw (void);
void UI_CheckMainMenuMouse (void);

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
	void Menu_Options_f (void);
		void Menu_Options_Sound_f (void);
		void Menu_Options_Controls_f (void);
			void Menu_Keys_f (void);
		void Menu_Options_Screen_f (void);
		void Menu_Options_Effects_f (void);
		void Menu_Options_Interface_f (void);
	void Menu_Quit_f (void);
	void Menu_Credits( void );

static char *creditsBuffer;

//
// ui_credits.c
//
void Menu_Credits_Draw (void);

//
// ui_options_screen.c
//
void Menu_Options_Screen_Draw (void);
void Menu_Options_Screen_Crosshair_MouseClick (void);

//
// ui_mp_playersetup.c
//
void Menu_PlayerConfig_Draw (void);
void Menu_PlayerConfig_MouseClick (void);
void Menu_PConfigSaveChanges (void);

#endif
