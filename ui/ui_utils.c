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

// ui_utils.c -- misc support functions for the menus

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"
#include <sys/types.h>
#include <sys/stat.h>

/*
=======================================================================

	MISC UTILITY FUNCTIONS

=======================================================================
*/

#if 0
/*
=================
FreeFileList
=================
*/
void FreeFileList (char **list, int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		if (list && list[i])
		{
			free(list[i]);
			list[i] = 0;
		}
	}
	free(list);
}

/*
=================
ItemInList
=================
*/
qboolean ItemInList (char *check, int num, char **list)
{
	int i;
	for (i=0;i<num;i++)
		if (!Q_strcasecmp(check, list[i]))
			return true;
	return false;
}

/*
=================
InsertInList
=================
*/
void InsertInList (char **list, char *insert, int len, int start)
{
	int i;
	if (!list) return;

	for (i=start; i<len; i++)
	{
		if (!list[i])
		{
			list[i] = strdup(insert);
			return;
		}
	}
	list[len] = strdup(insert);
}
#endif


/*
==========================
UI_IsValidImageFilename
==========================
*/
qboolean UI_IsValidImageFilename (char *name)
{
	int		len = (int)strlen(name);

	if (	!strcmp(name+max(len-4,0), ".pcx")
		||	!strcmp(name+max(len-4,0), ".tga")
#ifdef PNG_SUPPORT
		||	!strcmp(name+max(len-4,0), ".png")
#endif	// PNG_SUPPORT
		||  !strcmp(name+max(len-4,0), ".jpg")
		 )
		return true;

	return false;
}


/*
==========================
UI_ClampCvar
==========================
*/
void UI_ClampCvar (const char *varName, float cvarMin, float cvarMax)
{
	Cvar_SetValue ((char *)varName, ClampCvar( cvarMin, cvarMax, Cvar_VariableValue((char *)varName) ));
}


/*
==========================
UI_GetIndexForStringValue
==========================
*/
int	UI_GetIndexForStringValue (const char **item_values, char *value)
{
	int			i, index = 0, widlcardIndex = -1;
	qboolean	found = false;

	// catch null array
	if (!item_values) {
		Com_Printf ("UI_GetIndexForStringValue: null itemValues!\n");
		return 0;
	}

	for (i=0; item_values[i]; i++)
	{	// Store index of wildcard entry
		if ( !Q_stricmp(va("%s", item_values[i]), UI_ITEMVALUE_WILDCARD) )
		{	widlcardIndex = i;	}
		if ( !Q_strcasecmp(va("%s", item_values[i]), value) ) {
			index = i;
			found = true;
			break;
		}
	}
	// Assign index of wildcard entry if not found
	if ( !found && (widlcardIndex >= 0) ) {
		index = widlcardIndex;
	}
	return index;
}


/*
==========================
UI_MouseOverAlpha
==========================
*/
int UI_MouseOverAlpha (menucommon_s *m)
{
	if (ui_mousecursor.menuitem == m)
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


/*
==========================
UI_UnbindCommand
==========================
*/
void UI_UnbindCommand (char *command)
{
	int		j, len, len2;
	char	*b;

	len = (int)strlen(command);

	for (j=0; j<256; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		len2 = (int)strlen(b);
		// compare with longer length to prevent +attack2 being confused with +attack
		if ( !strncmp(b, command, max(len, len2)) )
			Key_SetBinding (j, "");
	}
}


/*
==========================
UI_FindKeysForCommand
==========================
*/
void UI_FindKeysForCommand (char *command, int *twokeys)
{
	int		count, j, len, len2;
	char	*b;

	twokeys[0] = twokeys[1] = -1;
	len = (int)strlen(command);
	count = 0;

	for (j=0; j<256; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		len2 = (int)strlen(b);
		// compare with longer length to prevent +attack2 being confused with +attack
		if ( !strncmp(b, command, max(len, len2)) )
		{
			twokeys[count] = j;
			count++;
			if (count == 2)
				break;
		}
	}
}


/*
=================
UI_ItemAtMenuCursor
=================
*/
void *UI_ItemAtMenuCursor (menuframework_s *m)
{
	if (m->cursor < 0 || m->cursor >= m->nitems)
		return 0;

	return m->items[m->cursor];
}


/*
=================
UI_SetMenuStatusBar
=================
*/
void UI_SetMenuStatusBar (menuframework_s *m, const char *string)
{
	if (!m)	return;

	m->statusbar = string;
}


/*
=================
UI_TallyMenuSlots
=================
*/
int UI_TallyMenuSlots (menuframework_s *menu)
{
	int i;
	int total = 0;

	for (i = 0; i < menu->nitems; i++)
	{
		if ( ((menucommon_s *)menu->items[i])->type == MTYPE_LIST )
		{
			int nitems = 0;
			const char **n = ((menulist_s *)menu->items[i])->itemNames;

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

	START GAME FUNCS

=======================================================================
*/

/*
===============
UI_StartSPGame
===============
*/
void UIStartSPGame (void)
{
	// disable updates and start the cinematic going
	cl.servercount = -1;
	UI_ForceMenuOff ();
	Cvar_SetValue( "deathmatch", 0 );
	Cvar_SetValue( "coop", 0 );
	Cvar_SetValue( "gamerules", 0 );		//PGM

	if (cls.state != ca_disconnected) // don't force loadscreen if disconnected
		Cbuf_AddText ("loading ; killserver ; wait\n");
	Cbuf_AddText ("newgame\n");
	cls.key_dest = key_game;
}


/*
===============
UI_StartServer
===============
*/
void UI_StartServer (char *startmap, qboolean dedicated)
{
	char	*spot = NULL;

	if ( UI_Coop_MenuMode() )
	{
 		if (Q_stricmp(startmap, "bunk1") == 0)
  			spot = "start";
 		else if (Q_stricmp(startmap, "mintro") == 0)
  			spot = "start";
 		else if (Q_stricmp(startmap, "fact1") == 0)
  			spot = "start";
 		else if (Q_stricmp(startmap, "power1") == 0)
  			spot = "pstart";
 		else if (Q_stricmp(startmap, "biggun") == 0)
  			spot = "bstart";
 		else if (Q_stricmp(startmap, "hangar1") == 0)
  			spot = "unitstart";
 		else if (Q_stricmp(startmap, "city1") == 0)
  			spot = "unitstart";
 		else if (Q_stricmp(startmap, "boss1") == 0)
			spot = "bosstart";
	}
	UI_ForceMenuOff ();

	if (dedicated)
		Cvar_ForceSet ("dedicated", "1");

	if (spot) {
		if (Com_ServerState())
			Cbuf_AddText ("disconnect\n");
		Cbuf_AddText (va("gamemap \"*%s$%s\"\n", startmap, spot));
	}
	else {
		Cbuf_AddText (va("map %s\n", startmap));
	}
//	UI_ForceMenuOff ();
}

/*
=======================================================================

	VIDEO INFO LOADING

=======================================================================
*/

#define UI_MAX_VIDMODES 128

char	**ui_resolution_names = NULL;
char	**ui_video_modes = NULL;
int		ui_num_video_modes = 0;

/*
==========================
UI_GetVideoModes
==========================
*/
void UI_GetVideoModes (void)
{
	int			i, j=0, w=0, h=0, firstMode=0, numModes=0;
	float		aspect;
	char		*tok, resBuf[12], aspectBuf[8], nameBuf[20];
//	qboolean	surround = false;
//	cvar_t		*surround_threshold = Cvar_Get ("scr_surroundthreshold", "3.6", 0);

	// count video modes >= 640x480
	for (i=0; i<UI_MAX_VIDMODES; i++)
	{
		if ( !VID_GetModeInfo(&w, &h, i) )
			break;

		if (w >= 640 && h >= 480) {
			numModes++;
			if (numModes == 1)
				firstMode = i;
		}
	}
	// allocate lists
	ui_resolution_names = malloc ((numModes+2) * sizeof(char *));
	memset (ui_resolution_names, 0, (numModes+2) * sizeof(char *));
	ui_video_modes = malloc ((numModes+2) * sizeof(char *));
	memset (ui_video_modes, 0, (numModes+2) * sizeof(char *));

	// add custom resolution item
//	ui_resolution_names[0] = strdup ("custom      ???");
	ui_resolution_names[0] = strdup ("[custom   ] [ ??? ]");
	ui_video_modes[0] = strdup ("-1");	

	// add to lists
	for (i=firstMode, j=1; i<(firstMode+numModes); i++, j++)
	{
		if ( !VID_GetModeInfo(&w, &h, i) )
			break;

		if (w >= 640 && h >= 480)
		{
			aspect = (float)w / (float)h;
			memset (resBuf, 0, sizeof(resBuf));
			memset (aspectBuf, 0, sizeof(aspectBuf));
			memset (nameBuf, 0, sizeof(nameBuf));
			Com_sprintf (resBuf, sizeof(resBuf), "%dx%d", w, h);

			// catch surround modes
		/*	if (aspect > surround_threshold->value) {	// 3.6f
				aspect /= 3.0f;
				surround = true;
			} */
			if (aspect > 2.39f)
				tok = "24:10";
			else if (aspect > 2.3f)
				tok = "21:9";
			else if (aspect > 1.9f)
				tok = "16:8";
			else if (aspect > 1.85f)
				tok = "17:10";
			else if (aspect > 1.65f)
				tok = "16:9";
			else if (aspect > 1.6f)
				tok = "15:9";
			else if (aspect > 1.55f)
				tok = "16:10";
			else if (aspect > 1.3f)
				tok = "4:3";
			else if (aspect > 1.2f)
				tok = "5:4";
			else
				tok = va("%3.1f:1", aspect);

		/*	if (surround)
				Com_sprintf (aspectBuf, sizeof(aspectBuf), "3x%s", tok);
			else */
				Com_sprintf (aspectBuf, sizeof(aspectBuf), "%s", tok);

		//	Com_sprintf (nameBuf, sizeof(nameBuf), "%-12s%s", resBuf, aspectBuf);
			Com_sprintf (nameBuf, sizeof(nameBuf), "[%-9s] [%-5s]", resBuf, aspectBuf);

			ui_resolution_names[j] = strdup (nameBuf);
			ui_video_modes[j] = strdup (va("%i", i));	
		}
	}

	ui_num_video_modes = numModes;
}


/*
==========================
UI_FreeVideoModes
==========================
*/
void UI_FreeVideoModes (void)
{
	if (ui_num_video_modes > 0) {
		FS_FreeFileList (ui_resolution_names, ui_num_video_modes);
		FS_FreeFileList (ui_video_modes, ui_num_video_modes);
	}
	ui_resolution_names = NULL;
	ui_video_modes = NULL;
}


char	**ui_aniso_names = NULL;
char	**ui_aniso_values = NULL;
int		ui_num_aniso_values = 0;

/*
==========================
UI_GetAnisoValues
==========================
*/
void UI_GetAnisoValues (void)
{
	int		i, numValues;
	float	aniso_avail = Cvar_VariableValue("r_anisotropic_avail");

	if (aniso_avail < 2.0)
		numValues = 1;
	else if (aniso_avail < 4.0)
		numValues = 2;
	else if (aniso_avail < 8.0)
		numValues = 3;
	else if (aniso_avail < 16.0)
		numValues = 4;
	else // >= 16.0
		numValues = 5;

	// allocate lists
	ui_aniso_names = malloc ((numValues+1) * sizeof(char *));
	memset (ui_aniso_names, 0, (numValues+1) * sizeof(char *));
	ui_aniso_values = malloc ((numValues+1) * sizeof(char *));
	memset (ui_aniso_values, 0, (numValues+1) * sizeof(char *));

	// set names and values
	for (i=0; i<numValues; i++)
	{
		if (i == 0)
			ui_aniso_names[i] = (numValues == 1) ? strdup("not supported") : strdup("off");
		else
			ui_aniso_names[i] = strdup(va("%ix", 1<<i));
		ui_aniso_values[i] = strdup(va("%i", 1<<i));
	}

	ui_num_aniso_values = numValues;
}


/*
==========================
UI_FreeAnisoValues
==========================
*/
void UI_FreeAnisoValues (void)
{
	if (ui_num_aniso_values > 0) {
		FS_FreeFileList (ui_aniso_names, ui_num_aniso_values);
		FS_FreeFileList (ui_aniso_values, ui_num_aniso_values);
	}
	ui_aniso_names = NULL;
	ui_aniso_values = NULL;
}

/*
==========================
UI_GetVideoInfo
==========================
*/
void UI_GetVideoInfo (void)
{
	UI_GetVideoModes ();
	UI_GetAnisoValues ();
}

/*
==========================
UI_FreeVideoInfo
==========================
*/
void UI_FreeVideoInfo (void)
{
	UI_FreeVideoModes ();
	UI_FreeAnisoValues ();
}

/*
=======================================================================

	GENERIC ASSET LIST LOADING

=======================================================================
*/

/*
==========================
UI_InsertInAssetList
==========================
*/
void UI_InsertInAssetList (char **list, char *insert, int len, int start)
{
	int i, j;

	if (!list || !insert) return;
	if (start < 0) return;
//	if (start >= len) return;
	if (start > len) return;

	// i=start so default stays first!
	for (i=start; i<len; i++)
	{
		if (!list[i])
			break;

		if (strcmp( list[i], insert ))
		{
			for (j=len; j>i; j--)
				list[j] = list[j-1];

			list[i] = strdup(insert);
			return;
		}
	}
	list[len] = strdup(insert);
}


/*
==========================
UI_LoadAssetList

Generic file list loader
Used for fonts, huds, and crosshairs
==========================
*/
char **UI_LoadAssetList (char *dir, char *nameMask, char *firstItem, int *returnCount, int maxItems, qboolean stripExtension, qboolean frontInsert, qboolean (*checkName)(char *p))
{
	char	**list = 0, **itemFiles;
	char	*path = NULL, *curItem, *p, *ext;
//	char	findName[1024];
	int		nItems = 0, nItemNames, i, baseLen, extLen;

	// check pointers
	if (!dir || !nameMask || !firstItem || !returnCount || !checkName)
		return NULL;
	if (maxItems < 2) // must allow at least 2 items
		return NULL;

	list = malloc(sizeof(char *) * (maxItems+1));
	memset(list, 0, sizeof(char *) * (maxItems+1));

	// set first item name
	list[0] = strdup(firstItem); 
	nItemNames = 1;

#if 1
	itemFiles = FS_GetFileList(va("%s/%s", dir, nameMask), NULL, &nItems);
	for (i=0; i<nItems && nItemNames < maxItems; i++)
	{
		if (!itemFiles || !itemFiles[i])
			continue;

		p = strrchr(itemFiles[i], '/'); p++;

		if ( !checkName(p) )
			continue;

		if (stripExtension && (ext = strrchr(p, '.')) ) {
			extLen = (int)strlen(ext);
			baseLen = (int)strlen(p) - extLen;
			p[baseLen] = 0;	// NULL
		}
		curItem = p;

		if (!FS_ItemInList(curItem, nItemNames, list))
		{
			// UI_InsertInAssetList (frontInsert) not needed due to sorting in FS_GetFileList()
			FS_InsertInList (list, curItem, nItemNames, 1);	// start=1 so first item stays first!
			nItemNames++;
		}
		
		// restore extension so whole string gets deleted
		if (stripExtension && ext)
			p[baseLen] = '.';
	}
#else
	path = FS_NextPath (path);
	while (path) 
	{
		Com_sprintf (findName, sizeof(findName), "%s/%s/%s", path, dir, nameMask);
		itemFiles = FS_ListFiles(findName, &nItems, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM);

		for (i=0; i < nItems && nItemNames < maxItems; i++)
		{
			if (!itemFiles || !itemFiles[i])
				continue;

			p = strrchr(itemFiles[i], '/'); p++;

			if ( !checkName(p) )
				continue;

			if (stripExtension && (ext = strrchr(p, '.')) ) {
				extLen = (int)strlen(ext);
				baseLen = (int)strlen(p) - extLen;
				p[baseLen] = 0;	// NULL
			}
			curItem = p;

			if (!FS_ItemInList(curItem, nItemNames, list))
			{
				if (frontInsert)
					UI_InsertInAssetList (list, curItem, nItemNames, 1);	// start=1 so first item stays first!
				else
					FS_InsertInList (list, curItem, nItemNames, 1);	// start=1 so first item stays first!
				nItemNames++;
			}
			
			// restore extension so whole string gets deleted
			if (stripExtension && ext)
				p[baseLen] = '.';
		}
		if (nItems)
			FS_FreeFileList (itemFiles, nItems);
		
		path = FS_NextPath (path);
	}

	// check pak after
	if (itemFiles = FS_ListPak(va("%s/", dir), &nItems))
	{
		for (i=0; i<nItems && nItemNames < maxItems; i++)
		{
			if (!itemFiles || !itemFiles[i])
				continue;

			p = strrchr(itemFiles[i], '/'); p++;

			if ( !checkName(p) )
				continue;

			if (stripExtension && (ext = strrchr(p, '.')) ) {
				extLen = (int)strlen(ext);
				baseLen = (int)strlen(p) - extLen;
				p[baseLen] = 0;	// NULL
			}
			curItem = p;

			if (!FS_ItemInList(curItem, nItemNames, list))
			{
				if (frontInsert)
					UI_InsertInAssetList (list, curItem, nItemNames, 1);	// start=1 so first item stays first!
				else
					FS_InsertInList (list, curItem, nItemNames, 1);	// start=1 so first item stays first!
				nItemNames++;
			}
			
			// restore extension so whole string gets deleted
			if (stripExtension && ext)
				p[baseLen] = '.';
		}
	}
#endif

	if (nItems)
		FS_FreeFileList (itemFiles, nItems);

	// re-count list, nItemNames is somehow counted with 1 extra
	for (i=0; list[i]; i++);
		nItemNames = i;

	if ( returnCount ) 
		*returnCount = nItemNames;

	return list;		
}

/*
=======================================================================

	FONT LOADING

=======================================================================
*/

#define UI_MAX_FONTS 32
char **ui_font_names = NULL;
int	ui_numfonts = 0;

/*
==========================
UI_IsValidFontName
==========================
*/
qboolean UI_IsValidFontName (char *name)
{
	return UI_IsValidImageFilename (name);
}


/*
==========================
UI_LoadFontNames
==========================
*/
void UI_LoadFontNames (void)
{
	ui_font_names = UI_LoadAssetList ("fonts", "*.*", "default", &ui_numfonts, UI_MAX_FONTS, true, true, UI_IsValidFontName);
}


/*
==========================
UI_FreeFontNames
==========================
*/
void UI_FreeFontNames (void)
{
	if (ui_numfonts > 0) {
		FS_FreeFileList (ui_font_names, ui_numfonts);
	}
	ui_font_names = NULL;
	ui_numfonts = 0;
}

/*
=======================================================================

	HUD NAME LOADING

=======================================================================
*/

// TODO: Enable this when HUD loading is working
#if 0
#define UI_MAX_HUDS 128
char **ui_hud_names = NULL;
int	ui_numhuds = 0;

/*
==========================
UI_IsValidHudName
==========================
*/
qboolean UI_IsValidHudName (char *name)
{
	int		len = (int)strlen(name);

	if ( !strcmp(name+max(len-4,0), ".hud") )
		return true;

	return false;
}


/*
==========================
UI_LoadHudNames
==========================
*/
void UI_LoadHudNames (void)
{
	ui_hud_names = UI_LoadAssetList ("scripts/huds", "*.hud", "default", &ui_numhuds, UI_MAX_HUDS, true, true, UI_IsValidHudName);
}


/*
==========================
UI_FreeHudNames
==========================
*/
void UI_FreeHudNames (void)
{
	if (ui_numhuds > 0){
		FS_FreeFileList (ui_hud_names, ui_numhuds);
	}
	ui_hud_names = NULL;
	ui_numhuds = 0;
}
#endif

/*
=======================================================================

	CROSSHAIR LOADING

=======================================================================
*/

#define UI_MAX_CROSSHAIRS 101	// none + ch1-ch100
char **ui_crosshair_names = NULL;
char **ui_crosshair_display_names = NULL;
char **ui_crosshair_values = NULL;
int	ui_numcrosshairs = 0;

/*
==========================
UI_SortCrosshairs
==========================
*/
void UI_SortCrosshairs (char **list, int len)
{
	int			i, j;
	char		*temp;
	qboolean	moved;

	if (!list || len < 2)
		return;

	for (i=(len-1); i>0; i--)
	{
		moved = false;
		for (j=0; j<i; j++)
		{
		//	if (!list[j]) break;
			if (!list[j] || !list[j+1]) continue;
			if ( atoi(strdup(list[j]+2)) > atoi(strdup(list[j+1]+2)) )
			{
				temp = list[j];
				list[j] = list[j+1];
				list[j+1] = temp;
				moved = true;
			}
		}
		if (!moved) break; // done sorting
	}
}


/*
==========================
UI_IsValidCrosshairName
==========================
*/
qboolean UI_IsValidCrosshairName (char *name)
{
	int		namelen;

	if ( !UI_IsValidImageFilename(name) )
		return false;

	// filename must be chxxx
	if ( strncmp(name, "ch", 2) ) 
		return false;
	namelen = (int)strlen(strdup(name));
	if (namelen < 7 || namelen > 9)
		return false;
	if ( !isNumeric(name[2]) )
		return false;
	if ( namelen >= 8 && !isNumeric(name[3]) )
		return false;
	// ch0 is invalid
	if ( namelen == 7 && name[2] == '0' )
		return false;
	// ch100 is only valid 5-char name
	if ( namelen == 9 && (name[2] != '1' || name[3] != '0' || name[4] != '0') )
		return false;
	// ch100-ch128 are only valid 5-char names
/*	if ( namelen == 9 &&
		( !isNumeric(name[4]) || name[2] != '1'
		|| (name[2] == '1' && name[3] > '2')
		|| (name[2] == '1' && name[3] == '2' && name[4] > '8') ) )
		return false;*/

	return true;
}


/*
==========================
UI_LoadCrosshairs
==========================
*/
void UI_LoadCrosshairs (void)
{
	int		i;

	ui_crosshair_names = UI_LoadAssetList ("pics", "ch*.*", "none", &ui_numcrosshairs, UI_MAX_CROSSHAIRS, true, false, UI_IsValidCrosshairName);
	UI_SortCrosshairs (ui_crosshair_names, ui_numcrosshairs);

	ui_crosshair_display_names = malloc( sizeof(char *) * (UI_MAX_CROSSHAIRS+1) );
	memcpy(ui_crosshair_display_names, ui_crosshair_names, sizeof(char *) * (UI_MAX_CROSSHAIRS+1));
	ui_crosshair_display_names[0] = strdup("chnone");

	ui_crosshair_values  = malloc( sizeof(char *) * (UI_MAX_CROSSHAIRS+1) );
	memset(ui_crosshair_values, 0, sizeof(char *) * (UI_MAX_CROSSHAIRS+1) );

	for (i=0; i<ui_numcrosshairs; i++)
		ui_crosshair_values[i] = (i == 0) ? strdup("0") : strdup(strtok(ui_crosshair_names[i], "ch")); 
}


/*
==========================
UI_FreeCrosshairs
==========================
*/
void UI_FreeCrosshairs (void)
{
	if (ui_numcrosshairs > 0)
	{
		FS_FreeFileList (ui_crosshair_names, ui_numcrosshairs);
		if (ui_crosshair_display_names)
		{
			if (ui_crosshair_display_names[0]) {
				free (ui_crosshair_display_names[0]);
			}
			free (ui_crosshair_display_names);
		}
		FS_FreeFileList (ui_crosshair_values, ui_numcrosshairs);
	}
	ui_crosshair_names = NULL;
	ui_crosshair_display_names = NULL;
	ui_crosshair_values = NULL;
	ui_numcrosshairs = 0;
}

/*
=============================================================================

SAVEGAME / SAVESHOT HANDLING

=============================================================================
*/

char		ui_savestrings[UI_MAX_SAVEGAMES][32];
qboolean	ui_savevalid[UI_MAX_SAVEGAMES+1];
time_t		ui_savetimestamps[UI_MAX_SAVEGAMES];
qboolean	ui_savechanged[UI_MAX_SAVEGAMES];
qboolean	ui_saveshotvalid[UI_MAX_SAVEGAMES+1];

char		ui_mapname[MAX_QPATH];
char		ui_saveload_shotname[MAX_QPATH];

/*
==========================
UI_Load_Savestrings
==========================
*/
void UI_Load_Savestrings (qboolean update)
{
	int		i;
	FILE	*fp;
	fileHandle_t	f;
	char	name[MAX_OSPATH];
	char	mapname[MAX_TOKEN_CHARS];
	char	*ch;
	time_t	old_timestamp;
	struct	stat	st;

	for (i=0; i<UI_MAX_SAVEGAMES; i++)
	{
	//	Com_sprintf (name, sizeof(name), "%s/save/kmq2save%03i/server.ssv", FS_Savegamedir(), i);	// was FS_Gamedir()
		Com_sprintf (name, sizeof(name), "%s/"SAVEDIRNAME"/kmq2save%03i/server.ssv", FS_Savegamedir(), i);	// was FS_Gamedir()

		old_timestamp = ui_savetimestamps[i];
		stat(name, &st);
		ui_savetimestamps[i] = st.st_mtime;

		// doesn't need to be refreshed
		if ( update && ui_savetimestamps[i] == old_timestamp ) {
			ui_savechanged[i] = false;
			continue;
		}

		fp = fopen (name, "rb");
		if (!fp) {
		//	Com_Printf("Save file %s not found.\n", name);
			Q_strncpyz (ui_savestrings[i], sizeof(ui_savestrings[i]), EMPTY_GAME_STRING);
			ui_savevalid[i] = false;
			ui_savetimestamps[i] = 0;
		}
		else
		{
			fclose (fp);
		//	Com_sprintf (name, sizeof(name), "save/kmq2save%03i/server.ssv", i);
			Com_sprintf (name, sizeof(name), SAVEDIRNAME"/kmq2save%03i/server.ssv", i);
			FS_FOpenFile (name, &f, FS_READ);
			if (!f)
			{
				//Com_Printf("Save file %s not found.\n", name);
				Q_strncpyz (ui_savestrings[i], sizeof(ui_savestrings[i]), EMPTY_GAME_STRING);
				ui_savevalid[i] = false;
				ui_savetimestamps[i] = 0;
			}
			else
			{
				FS_Read (ui_savestrings[i], sizeof(ui_savestrings[i]), f);

				if (i==0) { // grab mapname
					FS_Read (mapname, sizeof(mapname), f);
					if (mapname[0] == '*') // skip * marker
						Com_sprintf (ui_mapname, sizeof(ui_mapname), mapname+1);
					else
						Com_sprintf (ui_mapname, sizeof(ui_mapname), mapname);
					if (ch = strchr (ui_mapname, '$'))
						*ch = 0; // terminate string at $ marker
				}
				FS_FCloseFile(f);
				ui_savevalid[i] = true;
			}
		}
		ui_savechanged[i] = (ui_savetimestamps[i] != old_timestamp);
	}
}


/*
==========================
UI_ValidateSaveshots
==========================
*/
void UI_ValidateSaveshots (void)
{
	int i;
	char shotname[MAX_QPATH];

	for ( i = 0; i < UI_MAX_SAVEGAMES; i++ )
	{
		if ( !ui_savechanged[i] )	// doeesn't need to be reloaded
			continue;
		if ( ui_savevalid[i] )
		{
			if (i == 0)
				Com_sprintf(shotname, sizeof(shotname), "/levelshots/%s.pcx", ui_mapname);
			else
			{	// free previously loaded shots
			//	Com_sprintf(shotname, sizeof(shotname), "save/kmq2save%03i/shot.jpg", i);
				Com_sprintf(shotname, sizeof(shotname), SAVEDIRNAME"/kmq2save%03i/shot.jpg", i);
				R_FreePic (shotname);
			//	Com_sprintf(shotname, sizeof(shotname), "/save/kmq2save%03i/shot.jpg", i);
				Com_sprintf(shotname, sizeof(shotname), "/"SAVEDIRNAME"/kmq2save%03i/shot.jpg", i);
			}
			if (R_DrawFindPic(shotname))
				ui_saveshotvalid[i] = true;
			else
				ui_saveshotvalid[i] = false;
		}
		else
			ui_saveshotvalid[i] = false;
	}
}


/*
==========================
UI_UpdateSaveshot
==========================
*/
char *UI_UpdateSaveshot (int index)
{
	// check index
	if (index < 0 || index >= UI_MAX_SAVEGAMES)
		return NULL;

	if ( ui_savevalid[index] && ui_saveshotvalid[index] ) {
		if ( index == 0 )
			Com_sprintf(ui_saveload_shotname, sizeof(ui_saveload_shotname), "/levelshots/%s.pcx", ui_mapname);
		else
		//	Com_sprintf(ui_saveload_shotname, sizeof(ui_saveload_shotname), "/save/kmq2save%03i/shot.jpg", index);
			Com_sprintf(ui_saveload_shotname, sizeof(ui_saveload_shotname), "/"SAVEDIRNAME"/kmq2save%03i/shot.jpg", index);
	}
	else if ( ui_saveshotvalid[UI_MAX_SAVEGAMES] )
		Com_sprintf(ui_saveload_shotname, sizeof(ui_saveload_shotname), UI_NOSCREEN_NAME);
	else	// no saveshot or nullshot
		return NULL;

	return ui_saveload_shotname;
}


/*
==========================
UI_UpdateSavegameData
==========================
*/
void UI_UpdateSavegameData (void)
{
	UI_Load_Savestrings (true);
	UI_ValidateSaveshots ();	// register saveshots
}

/*
==========================
UI_InitSavegameData
==========================
*/
void UI_InitSavegameData (void)
{
	int		i;

	for (i=0; i<UI_MAX_SAVEGAMES; i++) {
		ui_savetimestamps[i] = 0;
		ui_savechanged[i] = true;
	}

	UI_Load_Savestrings (false);
	UI_ValidateSaveshots ();	// register saveshots

	// register null saveshot, this is only done once
	if (R_DrawFindPic("/gfx/ui/noscreen.pcx"))
		ui_saveshotvalid[UI_MAX_SAVEGAMES] = true;
	else
		ui_saveshotvalid[UI_MAX_SAVEGAMES] = false;

	ui_savevalid[UI_MAX_SAVEGAMES] = false;	// this element is always false to handle the back action
}


/*
==========================
UI_SaveIsValid
==========================
*/
qboolean UI_SaveIsValid (int index)
{
	// check index
	if (index < 0 || index >= UI_MAX_SAVEGAMES)
		return false;

	return ui_savevalid[index];
}


/*
==========================
UI_SaveshotIsValid
==========================
*/
qboolean UI_SaveshotIsValid (int index)
{
	// check index
	if (index < 0 || index >= UI_MAX_SAVEGAMES)
		return false;

	return ui_saveshotvalid[index];
}


/*
===============
UI_CanOpenSaveMenu
===============
*/
qboolean UI_CanOpenSaveMenu (void *unused)
{
	return (Com_ServerState() > 0);
}

/*
=======================================================================

	SERVER LISTING
	Copyright (C) 2001-2003 pat@aftermoon.net for modif flanked by <serverping>

=======================================================================
*/

int		ui_num_servers;

// user readable information
char ui_local_server_names[UI_MAX_LOCAL_SERVERS][UI_LOCAL_SERVER_NAMELEN];
//char	*ui_serverlist_names[UI_MAX_LOCAL_SERVERS+1];

// network address
netadr_t ui_local_server_netadr[UI_MAX_LOCAL_SERVERS];

#if 1
//<serverping> Added code for compute ping time of server broadcasted
// The server is displayed like : 
// "protocol ping hostname mapname nb players/max players"
// "udp 100ms Pat  q2dm1 2/8"

int      global_udp_server_time;
int      global_ipx_server_time;
int      global_adr_server_time[16];
netadr_t global_adr_server_netadr[16];

/*
==========================
UI_AddToServerList
==========================
*/
void UI_AddToServerList (netadr_t adr, char *info)
{
	int		i;
    int     iPing;
    char    *pszProtocol;

	if (ui_num_servers == UI_MAX_LOCAL_SERVERS)
		return;

	while ( *info == ' ' )
		info++;

	// Ignore if duplicated
	for (i=0; i<ui_num_servers; i++)
        if ( strncmp(info, &ui_local_server_names[i][11], sizeof(ui_local_server_names[0])-10)==0 )	// crashes here
			return;

    iPing = 0 ;
	for (i=0 ; i<UI_MAX_LOCAL_SERVERS ; i++)
    {
        if ( memcmp(&adr.ip, &global_adr_server_netadr[i].ip, sizeof(adr.ip))==0 
             && adr.port == global_adr_server_netadr[i].port )
        {
            // bookmark server
            iPing = Sys_Milliseconds() - global_adr_server_time[i] ;
            pszProtocol = "bkm" ;
            break;
        }
    }
    if ( i == UI_MAX_LOCAL_SERVERS )
    {
        if ( adr.ip[0] > 0 )    // udp server
        {
            iPing = Sys_Milliseconds() - global_udp_server_time ;
            pszProtocol = "UDP" ;
        }
        else                    // ipx server
        {
            iPing = Sys_Milliseconds() - global_ipx_server_time ;
            pszProtocol = "IPX" ;
        }
    }

    Com_sprintf( ui_local_server_names[ui_num_servers], 
                 sizeof(ui_local_server_names[0]), 
                 "%s %4dms %s", pszProtocol, iPing, info ) ;

    ui_local_server_netadr[ui_num_servers] = adr;

    ui_num_servers++;
}
// </serverping>
#else
void UI_AddToServerList (netadr_t adr, char *info)
{
	int		i;

	if (ui_num_servers == UI_MAX_LOCAL_SERVERS)
		return;
	while ( *info == ' ' )
		info++;

	// ignore if duplicated
	for (i=0 ; i<ui_num_servers ; i++)
		if (!strcmp(info, ui_local_server_names[i]))
			return;

	ui_local_server_netadr[ui_num_servers] = adr;
	strncpy (ui_local_server_names[ui_num_servers], info, sizeof(ui_local_server_names[0])-1);
	ui_num_servers++;
}
#endif


/*
==========================
UI_SearchLocalGames
==========================
*/
void UI_SearchLocalGames (void)
{
	int		i;

	ui_num_servers = 0;
	for (i=0 ; i<UI_MAX_LOCAL_SERVERS ; i++)
	//	strncpy (ui_local_server_names[i], NO_SERVER_STRING);
		Q_strncpyz (ui_local_server_names[i], sizeof(ui_local_server_names[i]), NO_SERVER_STRING);

	UI_DrawTextBox (168, 192, 36, 3);
	UI_DrawString (188, 192+MENU_FONT_SIZE, MENU_FONT_SIZE, S_COLOR_ALT"Searching for local servers, this", 255);
	UI_DrawString (188, 192+MENU_FONT_SIZE*2, MENU_FONT_SIZE, S_COLOR_ALT"could take up to a minute, so", 255);
	UI_DrawString (188, 192+MENU_FONT_SIZE*3, MENU_FONT_SIZE, S_COLOR_ALT"please be patient.", 255);

	// the text box won't show up unless we do a buffer swap
	GLimp_EndFrame();

	// send out info packets
	CL_PingServers_f();
}


/*
==========================
UI_JoinServer
==========================
*/
void UI_JoinServer (int index)
{
	char	buffer[128];

	// check bounds
	if (index < 0 || index >= UI_MAX_LOCAL_SERVERS || index >= ui_num_servers)
		return;

	if ( Q_stricmp( ui_local_server_names[index], NO_SERVER_STRING ) == 0 )
		return;

	Com_sprintf (buffer, sizeof(buffer), "connect %s\n", NET_AdrToString (ui_local_server_netadr[index]));
	Cbuf_AddText (buffer);
	UI_ForceMenuOff ();
	cls.disable_screen = 1; // Knightmare- show loading screen
}


/*
==========================
UI_InitServerList
==========================
*/
void UI_InitServerList (void)
{
	int		i;

//	ui_serverlist_names[UI_MAX_LOCAL_SERVERS] = NULL;
	for ( i = 0; i < UI_MAX_LOCAL_SERVERS; i++ ) {
		Com_sprintf (ui_local_server_names[i], sizeof(ui_local_server_names[i]), NO_SERVER_STRING);
	//	ui_serverlist_names[i] = ui_local_server_names[i];
	}
}

/*
=======================================================================

	START SERVER MAP LIST

=======================================================================
*/

gametype_names_t gametype_names[] = 
{
	{MAP_DM, "dm ffa team teamdm"},
	{MAP_COOP, "coop"},
	{MAP_CTF, "ctf"},
	{MAP_3TCTF, "3tctf"},
};

maptype_t ui_svr_maptype;
static int	ui_svr_nummaps;
char **ui_svr_mapnames;
static int	ui_svr_maplist_sizes[NUM_MAPTYPES] = {0, 0, 0, 0};
static char	**ui_svr_maplists[NUM_MAPTYPES] = {NULL, NULL, NULL, NULL};
int	ui_svr_listfile_nummaps;
static char **ui_svr_listfile_mapnames;
static int	ui_svr_arena_nummaps[NUM_MAPTYPES];
static char **ui_svr_arena_mapnames[NUM_MAPTYPES];
//static	byte *ui_svr_mapshotvalid; // levelshot truth table
static	byte *ui_svr_mapshotvalid[NUM_MAPTYPES] = {NULL, NULL, NULL, NULL};	// levelshot truth tables
char ui_startserver_shotname [MAX_QPATH];
qboolean	ui_svr_coop = false;
qboolean	ui_svr_ctf = false;

void UI_BuildStartSeverLevelshotTables (void);
void UI_FreeStartSeverLevelshotTables (void);


/*
===============
UI_ParseArenaFromFile
Partially from Q3 source
===============
*/
qboolean UI_ParseArenaFromFile (char *filename, char *shortname, char *longname, char *gametypes, size_t bufSize)
{
	int				len;
	fileHandle_t	f;
	char			buf[MAX_ARENAS_TEXT];
	char			*s, *token, *dest;

	len = FS_FOpenFile (filename, &f, FS_READ);
	if (!f) {
		Com_Printf (S_COLOR_RED "UI_ParseArenaFromFile: file not found: %s\n", filename);
		return false;
	}
	if (len >= MAX_ARENAS_TEXT) {
		Com_Printf (S_COLOR_RED "UI_ParseArenaFromFile: file too large: %s is %i, max allowed is %i", filename, len, MAX_ARENAS_TEXT);
		FS_FCloseFile (f);
		return false;
	}

	FS_Read (buf, len, f);
	buf[len] = 0;
	FS_FCloseFile (f);

	s = buf;
	// get the opening curly brace
	token = COM_Parse (&s);
	if (!token) {
		Com_Printf ("UI_ParseArenaFromFile: unexpected EOF\n");
		return false;
	}
	if (token[0] != '{') {
		Com_Printf ("UI_ParseArenaFromFile: found %s when expecting {\n", token);
		return false;
	}

	// go through all the parms
	while (s < (buf + len))
	{
		dest = NULL;
		token = COM_Parse (&s);
		if (token && (token[0] == '}')) break;
		if (!token || !s) {
			Com_Printf ("UI_ParseArenaFromFile: EOF without closing brace\n");
			break;
		}

		if (!Q_strcasecmp(token, "map"))
			dest = shortname;
		else if (!Q_strcasecmp(token, "longname"))
			dest = longname;
		else if (!Q_strcasecmp(token, "type"))
			dest = gametypes;
		if (dest)
		{
			token = COM_Parse (&s);
			if (!token) {
				Com_Printf ("UI_ParseArenaFromFile: unexpected EOF\n");
				return false;
			}
			if (token[0] == '}') {
				Com_Printf ("UI_ParseArenaFromFile: closing brace without data\n");
				break;
			}
			if (!s) {
				Com_Printf ("UI_ParseArenaFromFile: EOF without closing brace\n");
				break;
			}
		//	strncpy(dest, token);
			Q_strncpyz (dest, bufSize, token);
		}
	}
	if (!shortname || !strlen(shortname)) {
		Com_Printf (S_COLOR_RED "UI_ParseArenaFromFile: %s: map field not found\n", filename);
		return false;
	}
	if (!strlen(longname))
		longname = shortname;
	return true;
}


/*
===============
UI_SortArenas
===============
*/
void UI_SortArenas (char **list, int len)
{
	int			i, j;
	char		*temp, *s1, *s2;
	qboolean	moved;

	if (!list || len < 2)
		return;

	for (i=(len-1); i>0; i--)
	{
		moved = false;
		for (j=0; j<i; j++)
		{
			if (!list[j]) break;
			s1 = strchr(list[j], '\n')+1;
			s2 = strchr(list[j+1], '\n')+1;
			if (Q_stricmp(s1, s2) > 0)
			{
				temp = list[j];
				list[j] = list[j+1];
				list[j+1] = temp;
				moved = true;
			}
		}
		if (!moved) break; // done sorting
	}
}


/*
===============
UI_LoadArenas
===============
*/
void UI_LoadArenas (void)
{
	char		*p, *s, *s2, *tok, *tok2;
	char		**arenafiles = NULL;
	char		**tmplist = NULL;
	char		*path = NULL;
//	char		findName[1024];
	char		shortname[MAX_TOKEN_CHARS];
	char		longname[MAX_TOKEN_CHARS];
	char		gametypes[MAX_TOKEN_CHARS];
	char		scratch[200];
	int			i, j, len, narenas = 0, narenanames = 0;
	size_t		nameSize;
	qboolean	type_supported[NUM_MAPTYPES];

	//
	// free existing lists and malloc new ones
	//
	for (i=0; i<NUM_MAPTYPES; i++)
	{
		if (ui_svr_arena_mapnames[i])
			FS_FreeFileList (ui_svr_arena_mapnames[i], ui_svr_arena_nummaps[i]);
		ui_svr_arena_nummaps[i] = 0;
		ui_svr_arena_mapnames[i] = malloc( sizeof( char * ) * MAX_ARENAS );
		memset( ui_svr_arena_mapnames[i], 0, sizeof( char * ) * MAX_ARENAS );
	}

	tmplist = malloc( sizeof( char * ) * MAX_ARENAS );
	memset( tmplist, 0, sizeof( char * ) * MAX_ARENAS );

#if 1
	arenafiles = FS_GetFileList ("scripts", "arena", &narenas);
	for (i = 0; i < narenas && narenanames < MAX_ARENAS; i++)
	{
		if (!arenafiles || !arenafiles[i])
			continue;

		len = (int)strlen(arenafiles[i]);
		if ( strcmp(arenafiles[i]+max(len-6,0), ".arena") )
			continue;

		p = arenafiles[i];

		if (!FS_ItemInList(p, narenanames, tmplist)) // check if already in list
		{
			if (UI_ParseArenaFromFile (p, shortname, longname, gametypes, MAX_TOKEN_CHARS))
			{
			//	Com_sprintf(scratch, sizeof(scratch), MAPLIST_FORMAT, longname, shortname);
				Com_sprintf(scratch, sizeof(scratch), "%s\n%s", longname, shortname);
				
				for (j=0; j<NUM_MAPTYPES; j++)
					type_supported[j] = false;
				s = gametypes;
				tok = strdup(COM_Parse (&s));
				while (s != NULL)
				{
					for (j=0; j<NUM_MAPTYPES; j++)
					{
						s2 = gametype_names[j].tokens;
						tok2 = COM_Parse (&s2);
						while (s2 != NULL) {
							if ( !Q_strcasecmp(tok, tok2) )
								type_supported[j] = true;
							tok2 = COM_Parse (&s2);
						}
					}
					if (tok)	free (tok);
					tok = strdup(COM_Parse(&s));
				}
				if (tok)	free (tok);

				for (j=0; j<NUM_MAPTYPES; j++)
					if (type_supported[j]) {
						nameSize = strlen(scratch) + 1;
						ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]] = malloc(nameSize);
					//	strncpy(ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]], scratch);
						Q_strncpyz (ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]], nameSize, scratch);
						ui_svr_arena_nummaps[j]++;
					}

			//	Com_Printf ("UI_LoadArenas: successfully loaded arena file %s: mapname: %s levelname: %s gametypes: %s\n", p, shortname, longname, gametypes);
				narenanames++;
				FS_InsertInList(tmplist, p, narenanames, 0); // add to list
			}
		}
	}
#else
	//
	// search in searchpaths for .arena files
	//
	path = FS_NextPath (path);
	while (path) 
	{
		Com_sprintf (findName, sizeof(findName), "%s/scripts/*.arena", path);
		arenafiles = FS_ListFiles(findName, &narenas, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM);

		for (i=0; i < narenas && narenanames < MAX_ARENAS; i++)
		{
			if (!arenafiles || !arenafiles[i])
				continue;

			len = (int)strlen(arenafiles[i]);
			if ( strcmp(arenafiles[i]+max(len-6,0), ".arena") )
				continue;

			p = arenafiles[i] + strlen(path) + 1;	// skip over path and next slash

			if (!FS_ItemInList(p, narenanames, tmplist)) // check if already in list
			{
				if (UI_ParseArenaFromFile (p, shortname, longname, gametypes, MAX_TOKEN_CHARS))
				{
					Com_sprintf(scratch, sizeof(scratch), "%s\n%s", longname, shortname);
					
					for (j=0; j<NUM_MAPTYPES; j++)
						type_supported[j] = false;
					s = gametypes;
					tok = strdup(COM_Parse (&s));
					while (s != NULL)
					{
						for (j=0; j<NUM_MAPTYPES; j++)
						{
							s2 = gametype_names[j].tokens;
							tok2 = COM_Parse (&s2);
							while (s2 != NULL) {
								if ( !Q_strcasecmp(tok, tok2) )
									type_supported[j] = true;
								tok2 = COM_Parse (&s2);
							}
						}
						if (tok)	free (tok);
						tok = strdup(COM_Parse(&s));
					}
					if (tok)	free (tok);

					for (j=0; j<NUM_MAPTYPES; j++)
						if (type_supported[j]) {
							nameSize = strlen(scratch) + 1;
							ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]] = malloc(nameSize);
						//	strncpy(ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]], scratch);
							Q_strncpyz(ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]], nameSize, scratch);
							ui_svr_arena_nummaps[j]++;
						}

				//	Com_Printf ("UI_LoadArenas: successfully loaded arena file %s: mapname: %s levelname: %s gametypes: %s\n", p, shortname, longname, gametypes);
					narenanames++;
					FS_InsertInList(tmplist, p, narenanames, 0); // add to list
				}			
			}
		}
		if (narenas)
			FS_FreeFileList (arenafiles, narenas);
		
		path = FS_NextPath (path);
	}

	//
	// check in paks for .arena files
	//
	if (arenafiles = FS_ListPak("scripts/", &narenas))
	{
		for (i=0; i<narenas && narenanames<MAX_ARENAS; i++)
		{
			if (!arenafiles || !arenafiles[i])
				continue;

			len = (int)strlen(arenafiles[i]);
			if ( strcmp(arenafiles[i]+max(len-6,0), ".arena") )
				continue;

			p = arenafiles[i];

			if (!FS_ItemInList(p, narenanames, tmplist)) // check if already in list
			{
				if (UI_ParseArenaFromFile (p, shortname, longname, gametypes, MAX_TOKEN_CHARS))
				{
					Com_sprintf(scratch, sizeof(scratch), "%s\n%s", longname, shortname);
					
					for (j=0; j<NUM_MAPTYPES; j++)
						type_supported[j] = false;
					s = gametypes;
					tok = strdup(COM_Parse (&s));
					while (s != NULL)
					{
						for (j=0; j<NUM_MAPTYPES; j++)
						{
							s2 = gametype_names[j].tokens;
							tok2 = COM_Parse (&s2);
							while (s2 != NULL) {
								if ( !Q_strcasecmp(tok, tok2) )
									type_supported[j] = true;
								tok2 = COM_Parse (&s2);
							}
						}
						if (tok)	free (tok);
						tok = strdup(COM_Parse(&s));
					}
					if (tok)	free (tok);

					for (j=0; j<NUM_MAPTYPES; j++)
						if (type_supported[j]) {
							nameSize = strlen(scratch) + 1;
							ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]] = malloc(nameSize);
						//	strncpy(ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]], scratch);
							Q_strncpyz(ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]], nameSize, scratch);
							ui_svr_arena_nummaps[j]++;
						}

					//Com_Printf ("UI_LoadArenas: successfully loaded arena file %s: mapname: %s levelname: %s gametypes: %s\n", p, shortname, longname, gametypes);
					narenanames++;
					FS_InsertInList(tmplist, strdup(p), narenanames, 0); // add to list
				}
			}
		}
	}
#endif

	if (narenas)
		FS_FreeFileList (arenafiles, narenas);

	if (narenanames)
		FS_FreeFileList (tmplist, narenanames);

	for (i=0; i<NUM_MAPTYPES; i++)
		UI_SortArenas (ui_svr_arena_mapnames[i], ui_svr_arena_nummaps[i]);

//	Com_Printf ("UI_LoadArenas: loaded %i arena file(s)\n", narenanames);
//	for (i=0; i<NUM_MAPTYPES; i++)
//		Com_Printf ("%s: %i arena file(s)\n", gametype_names[i].tokens, ui_svr_arena_nummaps[i]);
}


/*
===============
UI_LoadMapList
===============
*/
void UI_LoadMapList (void)
{
	char	*buffer, *s;
	char	mapsname[1024];
	int		i, j, length;
	size_t	nameSize;
	FILE	*fp;

	//
	// free existing list
	//
	if (ui_svr_listfile_mapnames)
		FS_FreeFileList (ui_svr_listfile_mapnames, ui_svr_listfile_nummaps);
	ui_svr_listfile_nummaps = 0;

	//
	// load the list of map names
	//
	Com_sprintf( mapsname, sizeof( mapsname ), "%s/maps.lst", FS_Gamedir() );	// FIXME: should this be FS_Savegamedir()?
	if ( ( fp = fopen( mapsname, "rb" ) ) == 0 )
	{
		if ( ( length = FS_LoadFile( "maps.lst", ( void ** ) &buffer ) ) == -1 )
			Com_Error( ERR_DROP, "couldn't find maps.lst\n" );
	}
	else
	{
#ifdef _WIN32
		length = filelength( fileno( fp  ) );
#else
		fseek(fp, 0, SEEK_END);
		length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
#endif
		buffer = malloc( length );
		fread( buffer, length, 1, fp );
	}

	s = buffer;

	i = 0;
	while (i < length)
	{
		if (s[i] == '\r')
			ui_svr_listfile_nummaps++;
		i++;
	}

	if (ui_svr_listfile_nummaps == 0)
	{	// hack in a default map list
		ui_svr_listfile_nummaps = 1;
		buffer = "base1 \"Outer Base\"\n";
	}

	ui_svr_listfile_mapnames = malloc( sizeof( char * ) * ( ui_svr_listfile_nummaps + 1 ) );
	memset( ui_svr_listfile_mapnames, 0, sizeof( char * ) * ( ui_svr_listfile_nummaps + 1 ) );

	s = buffer;

	for (i = 0; i < ui_svr_listfile_nummaps; i++)
	{
		char	shortname[MAX_TOKEN_CHARS];
		char	longname[MAX_TOKEN_CHARS];
		char	scratch[200];

	//	strncpy( shortname, COM_Parse( &s ) );
	//	strncpy( longname, COM_Parse( &s ) );
		Q_strncpyz (shortname, sizeof(shortname), COM_Parse(&s));
		Q_strncpyz (longname, sizeof(longname), COM_Parse(&s));
		Com_sprintf (scratch, sizeof( scratch ), "%s\n%s", longname, shortname);
		nameSize = strlen(scratch) + 1;
		ui_svr_listfile_mapnames[i] = malloc( nameSize );
	//	strncpyz( ui_svr_listfile_mapnames[i], scratch );
		Q_strncpyz (ui_svr_listfile_mapnames[i], nameSize, scratch);
	}
	ui_svr_listfile_mapnames[ui_svr_listfile_nummaps] = 0;

	if ( fp != 0 )
	{
		fp = 0;
		free( buffer );
	}
	else
		FS_FreeFile( buffer );

	UI_LoadArenas ();

	// build composite map lists
	for (i=0; i<NUM_MAPTYPES; i++)
	{
		if (ui_svr_maplists[i]) {
			free (ui_svr_maplists[i]);
		}
		ui_svr_maplists[i] = NULL;
		ui_svr_maplist_sizes[i] = ui_svr_listfile_nummaps + ui_svr_arena_nummaps[i];
		ui_svr_maplists[i] = malloc( sizeof( char * ) * (ui_svr_maplist_sizes[i] + 1) );
		memset( ui_svr_maplists[i], 0, sizeof( char * ) * (ui_svr_maplist_sizes[i] + 1) );

		for (j = 0; j < ui_svr_maplist_sizes[i]; j++)
		{
			if (j < ui_svr_listfile_nummaps)
				ui_svr_maplists[i][j] = ui_svr_listfile_mapnames[j];
			else
				ui_svr_maplists[i][j] = ui_svr_arena_mapnames[i][j-ui_svr_listfile_nummaps];
		}
	}

	ui_svr_maptype = MAP_DM; // init maptype
	ui_svr_mapnames = ui_svr_maplists[ui_svr_maptype];

	UI_BuildStartSeverLevelshotTables ();
}


/*
===============
UI_FreeMapList
===============
*/
void UI_FreeMapList (void)
{
	int		i;

	//
	// free composite lists
	//
	for (i=0; i<NUM_MAPTYPES; i++)
	{
		if (ui_svr_maplists[i]) {
			free (ui_svr_maplists[i]);
		}
		ui_svr_maplists[i] = NULL;
		ui_svr_maplist_sizes[i] = 0;
	}
	ui_svr_mapnames	= NULL;

	//
	// free list from file
	//
	if (ui_svr_listfile_mapnames) {
		FS_FreeFileList (ui_svr_listfile_mapnames, ui_svr_listfile_nummaps);
	}
	ui_svr_listfile_mapnames = NULL;
	ui_svr_listfile_nummaps = 0;

	//
	// free arena lists
	//
	for (i=0; i<NUM_MAPTYPES; i++)
	{
		if (ui_svr_arena_mapnames[i]) {
			FS_FreeFileList (ui_svr_arena_mapnames[i], ui_svr_arena_nummaps[i]);
		}
		ui_svr_arena_mapnames[i] = NULL;
		ui_svr_arena_nummaps[i] = 0;
	}

	UI_FreeStartSeverLevelshotTables ();
}


/*
===============
UI_UpdateMapList
===============
*/
void UI_UpdateMapList (maptype_t maptype)
{
	ui_svr_maptype = maptype;
	ui_svr_mapnames = ui_svr_maplists[ui_svr_maptype];
}


/*
===============
UI_BuildStartSeverLevelshotTables
===============
*/
void UI_BuildStartSeverLevelshotTables (void)
{
	int		i;

	for (i=0; i<NUM_MAPTYPES; i++)
	{	// free existing list	
		if (ui_svr_mapshotvalid[i])
			free(ui_svr_mapshotvalid[i]);

		// alloc and zero new list
		ui_svr_mapshotvalid[i] = malloc( sizeof( byte ) * ( ui_svr_maplist_sizes[i] + 1 ) );
		memset( ui_svr_mapshotvalid[i], 0, sizeof( byte ) * ( ui_svr_maplist_sizes[i] + 1 ) );

		// register null levelshot
		if (ui_svr_mapshotvalid[i][ui_svr_maplist_sizes[i]] == M_UNSET) {	
			if (R_DrawFindPic(UI_NOSCREEN_NAME))
				ui_svr_mapshotvalid[i][ui_svr_maplist_sizes[i]] = M_FOUND;
			else
				ui_svr_mapshotvalid[i][ui_svr_maplist_sizes[i]] = M_MISSING;
		}
	}
}


/*
===============
UI_FreeStartSeverLevelshotTables
===============
*/
void UI_FreeStartSeverLevelshotTables (void)
{
	int		i;

	for (i=0; i<NUM_MAPTYPES; i++)
	{
		if (ui_svr_mapshotvalid[i])
			free(ui_svr_mapshotvalid[i]);
	}
}


/*
===============
UI_UpdateStartSeverLevelshot
===============
*/
char *UI_UpdateStartSeverLevelshot (int index)
{
	char	startmap[MAX_QPATH];
	char	mapshotname [MAX_QPATH];
	
	// check index
	if (index < 0 || index >= ui_svr_maplist_sizes[ui_svr_maptype])
		return NULL;

	Q_strncpyz (startmap, sizeof(startmap), strchr( ui_svr_maplists[ui_svr_maptype][index], '\n' ) + 1);

	if (ui_svr_mapshotvalid[ui_svr_maptype][index] == M_UNSET) { // init levelshot
		Com_sprintf(mapshotname, sizeof(mapshotname), "/levelshots/%s.pcx", startmap);
		if (R_DrawFindPic(mapshotname))
			ui_svr_mapshotvalid[ui_svr_maptype][index] = M_FOUND;
		else
			ui_svr_mapshotvalid[ui_svr_maptype][index] = M_MISSING;
	}

	if (ui_svr_mapshotvalid[ui_svr_maptype][index] == M_FOUND)
		Com_sprintf(ui_startserver_shotname, sizeof(ui_startserver_shotname), "/levelshots/%s.pcx", startmap);
	else if (ui_svr_mapshotvalid[ui_svr_maptype][ui_svr_maplist_sizes[ui_svr_maptype]] == M_FOUND)
		Com_sprintf(ui_startserver_shotname, sizeof(ui_startserver_shotname), UI_NOSCREEN_NAME);
	else
		return NULL;

	return ui_startserver_shotname;
}


/*
===============
UI_SetCoopMenuMode
===============
*/
void UI_SetCoopMenuMode (qboolean value)
{
	ui_svr_coop = value;
}


/*
===============
UI_Coop_MenuMode
===============
*/
qboolean UI_Coop_MenuMode (void)
{
	return ui_svr_coop;
}


/*
===============
UI_CanOpenDMFlagsMenu
===============
*/
qboolean UI_CanOpenDMFlagsMenu (void *unused)
{
	return !ui_svr_coop;
}

/*
===============
UI_SetCTFMenuMode
===============
*/
void UI_SetCTFMenuMode (qboolean value)
{
	ui_svr_ctf = value;
}

/*
===============
UI_CTF_MenuMode
===============
*/
qboolean UI_CTF_MenuMode (void)
{
	return ui_svr_ctf;
}

/*
=======================================================================

	PLAYER MODEL LOADING

=======================================================================
*/

playermodelinfo_s ui_pmi[MAX_PLAYERMODELS];
char *ui_pmnames[MAX_PLAYERMODELS];
int ui_numplayermodels;

// save skins and models here so as to not have to re-register every frame
struct model_s *ui_playermodel;
struct model_s *ui_weaponmodel;
struct image_s *ui_playerskin;
char *ui_currentweaponmodel;

//char	ui_playerconfig_playermodelname[MAX_QPATH];
//char	ui_playerconfig_playerskinname[MAX_QPATH];
//char	ui_playerconfig_weaponmodelname[MAX_QPATH];

/*
==========================
UI_IsSkinIcon
==========================
*/
static qboolean UI_IsSkinIcon (char *name)
{
	int		len;
	char	*s, scratch[1024];

	Q_strncpyz(scratch, sizeof(scratch), name);
	*strrchr(scratch, '.') = 0;
	s = scratch;
	len = (int)strlen(s);
	return	(!strcmp(s+max(len-2,0), "_i"));
}


/*
==========================
UI_IconOfSkinExists
==========================
*/
static qboolean UI_IconOfSkinExists (char *skin, char **files, int nfiles, char *suffix)
{
	int		i;
	char	scratch[1024];

	Q_strncpyz (scratch, sizeof(scratch), skin);
	*strrchr(scratch, '.') = 0;
	Q_strncatz (scratch, sizeof(scratch), suffix);
//	strncat(scratch, "_i.pcx");

	for (i = 0; i < nfiles; i++)
	{
		if ( strcmp(files[i], scratch) == 0 )
			return true;
	}

	return false;
}


/*
==========================
UI_IsValidSkin

Adds menu support for TGA and JPG skins
==========================
*/
static qboolean UI_IsValidSkin (char **filelist, int numFiles, int index)
{
	if ( UI_IsValidImageFilename(filelist[index]) && !UI_IsSkinIcon(filelist[index]) )
	{
		if (	UI_IconOfSkinExists (filelist[index], filelist, numFiles-1 , "_i.pcx")
			||	UI_IconOfSkinExists (filelist[index], filelist, numFiles-1 , "_i.tga")
#ifdef PNG_SUPPORT
			||	UI_IconOfSkinExists (filelist[index], filelist, numFiles-1 , "_i.png")
#endif // PNG_SUPPORT
			||	UI_IconOfSkinExists (filelist[index], filelist, numFiles-1 , "_i.jpg"))
			return true;
	}
	return false;
}


/*
==========================
UI_PlayerConfig_ScanDirectories
==========================
*/
static qboolean UI_PlayerConfig_ScanDirectories (void)
{
	char findname[1024];
	char scratch[1024];
	int ndirs = 0, npms = 0;
	char **dirnames;
	char *path = NULL;
	int i;

	ui_numplayermodels = 0;

	// loop back to here if there were no valid player models found in the selected path
	do
	{
		//
		// get a list of directories
		//
		do 
		{
			path = FS_NextPath(path);
			Com_sprintf( findname, sizeof(findname), "%s/players/*.*", path );

			if ( (dirnames = FS_ListFiles(findname, &ndirs, SFF_SUBDIR, 0)) != 0 )
				break;
		} while (path);

		if (!dirnames)
			return false;

		//
		// go through the subdirectories
		//
		npms = ndirs;
		if (npms > MAX_PLAYERMODELS)
			npms = MAX_PLAYERMODELS;
		if ( (ui_numplayermodels + npms) > MAX_PLAYERMODELS )
			npms = MAX_PLAYERMODELS - ui_numplayermodels;

		for (i = 0; i < npms; i++)
		{
			int			k, s;
			char		*a, *b, *c;
			char		**skinnames;
			char		**skiniconnames;
			char		**imagenames;
			int			nimagefiles;
			int			nskins = 0;
			qboolean	already_added = false;	

			if (dirnames[i] == 0)
				continue;

			// check if dirnames[i] is already added to the ui_pmi[i].directory list
			a = strrchr(dirnames[i], '/');
			b = strrchr(dirnames[i], '\\');
			c = (a > b) ? a : b;
			for (k=0; k < ui_numplayermodels; k++)
				if (!strcmp(ui_pmi[k].directory, c+1))
				{	already_added = true;	break;	}
			if (already_added)
			{	// todo: add any skins for this model not already listed to skinDisplayNames
				continue;
			}

			// verify the existence of tris.md2
		//	strncpy(scratch, dirnames[i]);
		//	strncat(scratch, "/tris.md2");
			Q_strncpyz(scratch, sizeof(scratch), dirnames[i]);
			Q_strncatz(scratch, sizeof(scratch), "/tris.md2");
			if ( !Sys_FindFirst(scratch, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM) )
			{
				free(dirnames[i]);
				dirnames[i] = 0;
				Sys_FindClose();
				continue;
			}
			Sys_FindClose();

			// verify the existence of at least one skin
		//	strncpy(scratch, va("%s%s", dirnames[i], "/*.*")); // was "/*.pcx"
			Q_strncpyz(scratch, sizeof(scratch), va("%s%s", dirnames[i], "/*.*")); // was "/*.pcx"
			imagenames = FS_ListFiles (scratch, &nimagefiles, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM);

			if (!imagenames)
			{
				free(dirnames[i]);
				dirnames[i] = 0;
				continue;
			}

			// count valid skins, which consist of a skin with a matching "_i" icon
			for (k = 0; k < nimagefiles-1; k++)
				if ( UI_IsValidSkin(imagenames, nimagefiles, k) )
					nskins++;

			if (!nskins)
				continue;

			// make short name for the model
			a = strrchr(dirnames[i], '/');
			b = strrchr(dirnames[i], '\\');
			c = (a > b) ? a : b;

		//	strncpy(ui_pmi[ui_numplayermodels].displayname, c+1, MAX_DISPLAYNAME-1);
		//	strncpy(ui_pmi[ui_numplayermodels].directory, c+1);
			Q_strncpyz(ui_pmi[ui_numplayermodels].displayname, sizeof(ui_pmi[ui_numplayermodels].displayname), c+1);
			Q_strncpyz(ui_pmi[ui_numplayermodels].directory, sizeof(ui_pmi[ui_numplayermodels].directory), c+1);

			skinnames = malloc(sizeof(char *) * (nskins+1));
			memset(skinnames, 0, sizeof(char *) * (nskins+1));
			skiniconnames = malloc(sizeof(char *) * (nskins+1));
			memset(skiniconnames, 0, sizeof(char *) * (nskins+1));

			// copy the valid skins
			if (nimagefiles)
				for (s = 0, k = 0; k < nimagefiles-1; k++)
				{
					char *a, *b, *c;
					if ( UI_IsValidSkin(imagenames, nimagefiles, k) )
					{
						a = strrchr(imagenames[k], '/');
						b = strrchr(imagenames[k], '\\');

						c = (a > b) ? a : b;

					//	strncpy(scratch, c+1);
						Q_strncpyz(scratch, sizeof(scratch), c+1);

						if ( strrchr(scratch, '.') )
							*strrchr(scratch, '.') = 0;

						skinnames[s] = strdup(scratch);
						skiniconnames[s] = strdup(va("/players/%s/%s_i.pcx", ui_pmi[ui_numplayermodels].directory, scratch));
						s++;
					}
				}

			// at this point we have a valid player model
			ui_pmi[ui_numplayermodels].nskins = nskins;
			ui_pmi[ui_numplayermodels].skinDisplayNames = skinnames;
			ui_pmi[ui_numplayermodels].skinIconNames = skiniconnames;

			// make short name for the model
		//	a = strrchr(dirnames[i], '/');
		//	b = strrchr(dirnames[i], '\\');
		//	c = (a > b) ? a : b;

		//	Q_strncpyz(ui_pmi[ui_numplayermodels].displayname, sizeof(ui_pmi[ui_numplayermodels].displayname), c+1);
		//	Q_strncpyz(ui_pmi[ui_numplayermodels].directory, sizeof(ui_pmi[ui_numplayermodels].directory), c+1);

			FS_FreeFileList (imagenames, nimagefiles);

			ui_numplayermodels++;
		}
		
		if (dirnames)
			FS_FreeFileList (dirnames, ndirs);

	// if no valid player models found in path,
	// try next path, if there is one
	} while (path);	// (s_numplayermodels == 0 && path);

	return true;	//** DMP warning fix
}


/*
==========================
UI_LoadPlayerModels
==========================
*/
void UI_LoadPlayerModels (void)
{
	UI_PlayerConfig_ScanDirectories ();
}


/*
==========================
UI_FreePlayerModels
==========================
*/
void UI_FreePlayerModels (void)
{
	int		i;

	for (i = 0; i < ui_numplayermodels; i++)
	{
		int j;

		for (j = 0; j < ui_pmi[i].nskins; j++)
		{
			if (ui_pmi[i].skinDisplayNames[j])
				free(ui_pmi[i].skinDisplayNames[j]);
			ui_pmi[i].skinDisplayNames[j] = NULL;
		}
		free(ui_pmi[i].skinDisplayNames);
		ui_pmi[i].skinDisplayNames = NULL;
		ui_pmi[i].nskins = 0;
	}
}


/*
==========================
UI_PlayerModelCmpFunc
==========================
*/
int UI_PlayerModelCmpFunc (const void *_a, const void *_b)
{
	const playermodelinfo_s *a = (const playermodelinfo_s *) _a;
	const playermodelinfo_s *b = (const playermodelinfo_s *) _b;

	//
	// sort by male, female, then alphabetical
	//
	if ( strcmp(a->directory, "male") == 0 )
		return -1;
	else if (strcmp( b->directory, "male") == 0 )
		return 1;

	if ( strcmp(a->directory, "female") == 0 )
		return -1;
	else if (strcmp( b->directory, "female") == 0 )
		return 1;

	return strcmp(a->directory, b->directory);
}


/*
==========================
UI_InitPlayerModelInfo
==========================
*/
void UI_InitPlayerModelInfo (int *modelNum, int *skinNum)
{
	int		i;
	int		currentdirectoryindex = 0;
	int		currentskinindex = 0;
	char	currentdirectory[1024];
	char	currentskin[1024];

	if (ui_numplayermodels <= 0) {
		modelNum = skinNum = 0;
		return;
	}
		
	Q_strncpyz(currentdirectory, sizeof(currentdirectory), Cvar_VariableString ("skin"));

	if ( strchr( currentdirectory, '/' ) )
	{
		Q_strncpyz(currentskin, sizeof(currentskin), strchr( currentdirectory, '/' ) + 1);
		*strchr( currentdirectory, '/' ) = 0;
	}
	else if ( strchr( currentdirectory, '\\' ) )
	{
		Q_strncpyz(currentskin, sizeof(currentskin), strchr( currentdirectory, '\\' ) + 1);
		*strchr( currentdirectory, '\\' ) = 0;
	}
	else
	{
		Q_strncpyz(currentdirectory, sizeof(currentdirectory), "male");
		Q_strncpyz(currentskin, sizeof(currentskin), "grunt");
	}

	qsort( ui_pmi, ui_numplayermodels, sizeof( ui_pmi[0] ), UI_PlayerModelCmpFunc );

	memset( ui_pmnames, 0, sizeof( ui_pmnames ) );
	for (i = 0; i < ui_numplayermodels; i++)
	{
		ui_pmnames[i] = ui_pmi[i].displayname;
		if (Q_stricmp( ui_pmi[i].directory, currentdirectory ) == 0)
		{
			int j;

			currentdirectoryindex = i;

			for (j = 0; j < ui_pmi[i].nskins; j++)
			{
				if (Q_stricmp( ui_pmi[i].skinDisplayNames[j], currentskin ) == 0)
				{
					currentskinindex = j;
					break;
				}
			}
		}
	}

	// precache this player model and skin
	UI_UpdatePlayerModelInfo (currentdirectoryindex, currentskinindex);

	if (modelNum)
		*modelNum = currentdirectoryindex;
	if (skinNum)
		*skinNum = currentskinindex;
}


/*
==========================
UI_UpdatePlayerModelInfo
==========================
*/
void UI_UpdatePlayerModelInfo (int mNum, int sNum)
{
	char	scratch[MAX_QPATH];

	Com_sprintf (scratch, sizeof(scratch), "players/%s/tris.md2", ui_pmi[mNum].directory);
	ui_playermodel = R_RegisterModel (scratch);
//	Q_strncpyz (ui_playerconfig_playermodelname, sizeof(ui_playerconfig_playermodelname), scratch);

	Com_sprintf (scratch, sizeof(scratch), "players/%s/%s.pcx", ui_pmi[mNum].directory, ui_pmi[mNum].skinDisplayNames[sNum]);
	ui_playerskin = R_RegisterSkin (scratch);
//	Q_strncpyz (ui_playerconfig_playerskinname, sizeof(ui_playerconfig_playerskinname), scratch);

	// show current weapon model (if any)
	if (ui_currentweaponmodel && strlen(ui_currentweaponmodel))
	{
		Com_sprintf (scratch, sizeof(scratch), "players/%s/%s", ui_pmi[mNum].directory, ui_currentweaponmodel);
		ui_weaponmodel = R_RegisterModel(scratch);
		if (!ui_weaponmodel) {
			Com_sprintf (scratch, sizeof(scratch), "players/%s/weapon.md2", ui_pmi[mNum].directory);
			ui_weaponmodel = R_RegisterModel (scratch);
		}
	}
	else {
		Com_sprintf (scratch, sizeof(scratch), "players/%s/weapon.md2", ui_pmi[mNum].directory);
		ui_weaponmodel = R_RegisterModel (scratch);
	}
//	Q_strncpyz (ui_playerconfig_weaponmodelname, sizeof(ui_playerconfig_weaponmodelname), scratch);
}


/*
==========================
UI_UpdatePlayerSkinInfo
==========================
*/
void UI_UpdatePlayerSkinInfo (int mNum, int sNum)
{
	char	scratch[MAX_QPATH];

	Com_sprintf(scratch, sizeof(scratch), "players/%s/%s.pcx", ui_pmi[mNum].directory, ui_pmi[mNum].skinDisplayNames[sNum]);
	ui_playerskin = R_RegisterSkin(scratch);
//	Q_strncpyz (ui_playerconfig_playerskinname, sizeof(ui_playerconfig_playerskinname), scratch);
}


/*
==========================
UI_HaveValidPlayerModels
==========================
*/
qboolean UI_HaveValidPlayerModels (void *unused)
{
	return (ui_numplayermodels > 0);
}
