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

//
#define MENU_SUBTEXT_FONT_SIZE	6
#define MENU_HEADER_FONT_SIZE	10
#define MENU_HEADER_LINE_SIZE	13
#define RCOLUMN_OFFSET  MENU_FONT_SIZE*2	// was 16
#define LCOLUMN_OFFSET -MENU_FONT_SIZE*2	// was -16

#define SLIDER_RANGE 10
//


typedef struct _tag_menuframework
{
	int x, y;
	int	cursor;

	int	nitems;
	int nslots;
	void *items[64];

	const char *statusbar;

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

//	float			minvalue;
//	float			maxvalue;
//	float			curvalue;

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

	int			curvalue;

	const char	**itemnames;
	int			numitemnames;
} menulist_s;

typedef struct
{
	menucommon_s generic;
} menuaction_s;

typedef struct
{
	menucommon_s generic;
} menuseparator_s;

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
	scrolltype_t	scrolltype;
	int			items_x;
	int			items_y;
	int			item_width;
	int			item_height;
	int			scrollpos;
	int			curvalue;

	const char	**itemnames;
	int			numitemnames;
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

cursor_t ui_mousecursor;

// ui_backend.c
qboolean MenuField_Key (menufield_s *field, int key);
void	MenuSlider_SetValue (menuslider_s *s, float value);
float	MenuSlider_GetValue (menuslider_s *s);

void	Menu_AddItem (menuframework_s *menu, void *item);
void	Menu_AdjustCursor (menuframework_s *menu, int dir);
void	Menu_Center (menuframework_s *menu);
void	Menu_Draw (menuframework_s *menu);
void	*Menu_ItemAtCursor (menuframework_s *m);
qboolean Menu_SelectItem (menuframework_s *s);
qboolean Menu_MouseSelectItem (menucommon_s *item);
void	Menu_SetStatusBar (menuframework_s *s, const char *string);
void	Menu_SlideItem (menuframework_s *s, int dir);
int		Menu_TallySlots (menuframework_s *menu);

void	Menu_DrawString (int x, int y, int size, const char *string, int alpha);
void	Menu_DrawStringDark (int x, int y, int size, const char *string, int alpha);
void	Menu_DrawStringR2L (int x, int y, int size, const char *string, int alpha);
void	Menu_DrawStringR2LDark (int x, int y, int size, const char *string, int alpha);

void	Menu_DrawTextBox (int x, int y, int width, int lines);
void	Menu_DrawBanner (char *name);

void	UI_Draw_Cursor (void);

//
// ui_utils.c
//
extern char **ui_font_names;
extern int	ui_numfonts;

//extern char **ui_hud_names;
//extern int	ui_numhuds;

extern char **ui_crosshair_names;
//extern char **ui_crosshair_display_names;
//extern char **ui_crosshair_values;
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

void UIStartSPGame (void);
void UI_StartServer (char *startmap, qboolean dedicated);

//char **UI_SetFontNames (void);
void UI_LoadFontNames (void);
void UI_FreeFontNames (void);

//void UI_LoadHudNames (void);
//void UI_FreeHudNames (void);

void UI_SortCrosshairs (char **list, int len);
//char **UI_SetCrosshairNames (void);
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
int UI_PlayerModelCmpFunc (const void *_a, const void *_b);
void UI_InitPlayerModelInfo (int *modelNum, int *skinNum);
void UI_UpdatePlayerModelInfo (int mNum, int sNum);
void UI_UpdatePlayerSkinInfo (int mNum, int sNum);
qboolean UI_HaveValidPlayerModels (void *unused);

// ui_subsystem.c
void UI_AddButton (buttonmenuobject_t *thisObj, int index, float x, float y, float w, float h);
void UI_AddMainButton (mainmenuobject_t *thisObj, int index, int x, int y, char *name);
void UI_RefreshCursorMenu (void);
void UI_RefreshCursorLink (void);
void UI_RefreshCursorButtons (void);
void UI_PushMenu ( void (*draw) (void), const char *(*key) (int k) );
void UI_ForceMenuOff (void);
void UI_PopMenu (void);
void UI_BackMenu (void *unused);
const char *Default_MenuKey( menuframework_s *m, int key );

void UI_Draw (void);
void UI_Keydown (int key);
void UI_Precache (void);
void UI_Init (void);
void UI_Shutdown (void);

// ui_main.c
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

extern	cvar_t	*ui_sensitivity;
extern	cvar_t	*ui_background_alpha;
extern	cvar_t	*ui_item_rotate;
extern	cvar_t	*ui_cursor_scale;

static char *menu_in_sound		= "misc/menu1.wav";
static char *menu_move_sound	= "misc/menu2.wav";
static char *menu_out_sound		= "misc/menu3.wav";

extern qboolean	ui_entersound;		// play after drawing a frame, so caching
									// won't disrupt the sound

int MainMenuMouseHover;

void M_Main_Draw (void);
void UI_CheckMainMenuMouse (void);

void M_Menu_Main_f (void);
	void M_Menu_Game_f (void);
		void M_Menu_LoadGame_f (void);
		void M_Menu_SaveGame_f (void);
		void M_Menu_PlayerConfig_f (void);
			void M_Menu_DownloadOptions_f (void);
		void M_Menu_Credits_f( void );
	void M_Menu_Multiplayer_f( void );
		void M_Menu_JoinServer_f (void);
			void M_Menu_AddressBook_f( void );
		void M_Menu_StartServer_f (void);
			void M_Menu_DMOptions_f (void);
		void M_Menu_PlayerConfig_f (void);
		void M_Menu_DownloadOptions_f (void);
	void M_Menu_Video_f (void);
		void M_Menu_Video_Advanced_f (void);
	void M_Menu_Options_f (void);
		void M_Menu_Options_Sound_f (void);
		void M_Menu_Options_Controls_f (void);
			void M_Menu_Keys_f (void);
		void M_Menu_Options_Screen_f (void);
		void M_Menu_Options_Effects_f (void);
		void M_Menu_Options_Interface_f (void);
	void M_Menu_Quit_f (void);
	void M_Menu_Credits( void );

static char *creditsBuffer;


// ui_credits.c
void M_Credits_MenuDraw (void);

// ui_options_screen.c
void Options_Screen_MenuDraw (void);
void MenuCrosshair_MouseClick (void);

// ui_mp_playersetup.c
void PlayerConfig_MenuDraw (void);
void PlayerConfig_MouseClick (void);
void M_PConfigSaveChanges (void);

#endif
