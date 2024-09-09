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

/*
================
UI_TextColor
This sets the text color for scrollbars,
defers to CL_ function for all others.
================
*/
void UI_TextColor (int colornum, qboolean scrollbar, int *red, int *green, int *blue)
{
	if (!red || !green || !blue) // paranoia
		return;

	if (!scrollbar) {
		CL_TextColor (colornum, red, green, blue);
		return;
	}

	switch (colornum)
	{
		case 1:		// red
			*red =	255;
			*green=	20;
			*blue =	20;
			break;
		case 2:		// green
			*red =	0;
			*green=	255;
			*blue =	90;
			break;
		case 3:		// yellow
			*red =	255;
			*green=	255;
			*blue =	0;
			break;
		case 4:		// blue
			*red =	0;
			*green=	60;
			*blue =	255;
			break;
		case 5:		// cyan
			*red =	0;
			*green=	255;
			*blue =	255;
			break;
		case 6:		// magenta
			*red =	255;
			*green=	0;
			*blue =	255;
			break;
		case 9:		// orange
			*red =	255;
			*green=	135;
			*blue =	0;
			break;
		case 10:	// violet
			*red =	160;
			*green=	20;
			*blue =	255;
			break;
		case 11:	// indigo
			*red =	80;
			*green=	0;
			*blue =	255;
			break;
		case 12:	// viridian
			*red =	80;
			*green=	255;
			*blue =	180;
			break;
		case 13:	// pink
			*red =	255;
			*green=	125;
			*blue =	175;
			break;
		case 0:		// gray
		case 7:		// white
		case 8:		// black
		case 14:	// silver
		default:	// white
			*red =	255;
			*green=	255;
			*blue =	255;
			break;
	}
}


/*
==========================
UI_TextColorHighlight
==========================
*/
void UI_TextColorHighlight (int colornum, int *red, int *green, int *blue)
{
	if (!red || !green || !blue) // paranoia
		return;

	UI_TextColor (colornum, false, red, green, blue);
	*red *= 0.7;
	*green *= 0.7;
	*blue *= 0.7;
}


/*
==========================
UI_IsValidImageFilename
==========================
*/
qboolean UI_IsValidImageFilename (char *name)
{
	size_t	len = strlen(name);

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

#ifndef NOTTHIRTYFLIGHTS
// Forward ported from old engine - Brad

/*
==========================
UI_ClampCvar
==========================
*/
void UI_ClampCvar (const char *varName, float cvarMin, float cvarMax)
{
	Cvar_SetValue ((char *)varName, ClampCvar( cvarMin, cvarMax, Cvar_VariableValue((char *)varName) ));
}
#endif

/*
==========================
UI_GetScreenAspect
==========================
*/
float UI_GetScreenAspect (void)
{
	float	screenAspect = (4.0f/3.0f);

	if ( (viddef.width > 0) && (viddef.height > 0) )
	{
		screenAspect = (float)viddef.width / (float)viddef.height;
		// properly handle surround modes
		if ( (Cvar_VariableInteger("scr_surroundlayout") != 0) && (screenAspect >= Cvar_VariableValue("scr_surroundthreshold")) ) {
			screenAspect *= (Cvar_VariableValue("scr_surroundright") - Cvar_VariableValue("scr_surroundleft"));
		}
		screenAspect = min(max(screenAspect, 1.2f), (32.0f/9.0f));	// clamp between 5:4 and 32:9
	}
	return screenAspect;
}


/*
==========================
UI_ClampCvarForControl
==========================
*/
void UI_ClampCvarForControl (menuCommon_s *item)
{
	float	cvarMin, cvarMax;

	if (!item->cvarClamp)
		return;
	cvarMin = item->cvarMin;
	cvarMax = item->cvarMax;
	if (cvarMin == 0 && cvarMax == 0)	 // unitialized
		return;
	Cvar_SetValue (item->cvar, ClampCvar( cvarMin, cvarMax, Cvar_VariableValue(item->cvar) ));
}


/*
==========================
UI_GetCurValueForControl
==========================
*/
int UI_GetCurValueForControl (menuCommon_s *item)
{
	if (!item)	return 0;

	switch (item->type)
	{
	case MTYPE_KEYBINDLIST:
		return ((menuKeyBindList_s *)item)->curValue;
	case MTYPE_SLIDER:
		return ((menuSlider_s *)item)->curPos;
	case MTYPE_PICKER:
		return ((menuPicker_s *)item)->curValue;
	case MTYPE_CHECKBOX:
		return ((menuCheckBox_s *)item)->curValue;
	case MTYPE_LISTBOX:
		return ((menuListBox_s *)item)->curValue;
	case MTYPE_COMBOBOX:
		return ((menuComboBox_s *)item)->curValue;
	case MTYPE_LISTVIEW:
		return ((menuListView_s *)item)->curValue;
	default:
		return 0;
	}
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
int UI_MouseOverAlpha (menuCommon_s *menu)
{
	if (ui_mousecursor.menuitem == menu)
	{
		int alpha;

#ifdef NOTTHIRTYFLIGHTS
		alpha = 195 + 60 * cos(anglemod(cl.time*0.005));

		if (alpha > 255) alpha = 255;
		if (alpha < 0) alpha = 0;
#else
        alpha = 255;
#endif

		return alpha;
	}
	else 
		return 255;
}


/*
==========================
UI_MouseOverSubItem
==========================
*/
qboolean UI_MouseOverSubItem (int x, int y, int w, int h, scralign_t align)
{
	float	x1, y1, w1, h1;

	x1 = x;	y1 = y;	w1 = w; h1 = h;
	SCR_ScaleCoords (&x1, &y1, &w1, &h1, align);

	if ( ui_mousecursor.x >= x1 && ui_mousecursor.x < (x1+w1) 
		&& ui_mousecursor.y >= y1 &&  ui_mousecursor.y < (y1+h1) )
		return true;

	return false;
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
void *UI_ItemAtMenuCursor (menuFramework_s *menu)
{
	if (!menu)	return NULL;

	if ( (menu->cursor < 0) || (menu->cursor >= menu->nitems) )
		return NULL;

	return menu->items[menu->cursor];
}


/*
=================
UI_SetMenuStatusBar
=================
*/
void UI_SetMenuStatusBar (menuFramework_s *menu, const char *string)
{
	if (!menu)	return;

	menu->statusbar = string;
}


/*
=================
UI_SetMenuCurrentItemStatusBar
=================
*/
void UI_SetMenuCurrentItemStatusBar (menuFramework_s *menu, const char *string)
{	
	menuCommon_s *curItem;

	if (!menu)	return;

	if ( (curItem = (menuCommon_s *)UI_ItemAtMenuCursor(menu)) != NULL )
		curItem->statusbar = string;
}


/*
=================
UI_TallyMenuSlots
=================
*/
int UI_TallyMenuSlots (menuFramework_s *menu)
{
	int		i, total = 0;

	if (!menu)	return 0;

	for (i = 0; i < menu->nitems; i++)
		total++;

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
void UI_StartSPGame (void)
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
===============
UI_LoadMod
===============
*/
void UI_LoadMod (char *modName)
{
	if ( Q_strcasecmp(Cvar_VariableString("game"), modName) != 0 ) {
		UI_ForceMenuOff ();
		Cbuf_AddText (va("changegame %s\n", modName) );
	}
}


/*
===============
UI_LoadModFromList
===============
*/
void UI_LoadModFromList (int index)
{
	// bounds check
	if ( (index < 0) || (index >= ui_num_mods) )
		return;

	if ( Q_strcasecmp(Cvar_VariableString("game"), ui_mod_info[index].gameDir) != 0 )
	{
		UI_ForceMenuOff ();
		Cvar_ForceSet ("basegame", ui_mod_info[index].baseGame1);
		Cvar_ForceSet ("basegame2", ui_mod_info[index].baseGame2);
		Cvar_ForceSet ("basegame3", ui_mod_info[index].baseGame3);
		Cvar_ForceSet ("quake_importpath_auto", va("%i", ui_mod_info[index].quakeImportPathAuto));
		Cvar_ForceSet ("quakerr_importpath_auto", va("%i", ui_mod_info[index].quakeRRImportPathAuto));
		Cvar_ForceSet ("quake_importpath", ui_mod_info[index].quakeImportPath);
		Cvar_ForceSet ("quake_maingame", ui_mod_info[index].quakeMainGame);
		Cvar_ForceSet ("quake_game1", ui_mod_info[index].quakeGame1);
		Cvar_ForceSet ("quake_game2", ui_mod_info[index].quakeGame2);
		Cvar_ForceSet ("quake_game3", ui_mod_info[index].quakeGame3);
		Cvar_ForceSet ("quake_game4", ui_mod_info[index].quakeGame4);
	
		Cvar_ForceSet ("quake2rr_importpath_auto", va("%i", ui_mod_info[index].quake2RRImportPathAuto));
		Cvar_ForceSet ("quake2rr_importpath", ui_mod_info[index].quake2RRImportPath);
		Cvar_ForceSet ("quake2rr_maingame", ui_mod_info[index].quake2RRMainGame);
		Cvar_ForceSet ("quake2rr_game1", ui_mod_info[index].quake2RRGame1);
		Cvar_ForceSet ("quake2rr_game2", ui_mod_info[index].quake2RRGame2);
		Cvar_ForceSet ("quake2rr_game3", ui_mod_info[index].quake2RRGame3);
		Cvar_ForceSet ("quake2rr_game4", ui_mod_info[index].quake2RRGame4);
	
		Cbuf_AddText ( va("changegame %s\n", ui_mod_info[index].gameDir) );
	}
}

/*
=======================================================================

	GENERIC ASSET LIST LOADING / MANAGEMENT

=======================================================================
*/

#define UI_USE_ZMALLOC

#ifdef UI_USE_ZMALLOC
#define UI_Malloc			Z_Malloc
#define UI_CopyString		CopyString
#define UI_Free				Z_Free
#else	// UI_USE_ZMALLOC
__inline void *UI_Malloc (size_t size)
{
	byte *ret;

	ret = malloc (size);
	memset (ret, 0, size);
	return (void *)ret;
}
#define UI_CopyString		strdup
#define UI_Free 			free
#endif	// UI_USE_ZMALLOC

/*
==========================
UI_InsertInAssetList
==========================
*/
void UI_InsertInAssetList (char **list, const char *insert, int len, int start, qboolean frontInsert)
{
	int i, j;

	if ( !list || !insert )	return;
	if ( (len < 1) || (start < 0) || (start > len) )	return;

	if (frontInsert)
	{
		for (i=start; i<len; i++)	// i=start so default stays first!
		{
			if (!list[i])
				break;

			if (strcmp( list[i], insert ))
			{
				for (j=len; j>i; j--)
					list[j] = list[j-1];

				list[i] = UI_CopyString((char *)insert);
				return;
			}
		}
	}
	else
	{
		for (i=start; i<len; i++)	// i=start so default stays first!
		{
			if (!list[i])
			{
				list[i] = UI_CopyString((char *)insert);
				return;
			}
		}
	}
	list[len] = UI_CopyString((char *)insert);
}


/*
=================
UI_ItemInAssetList
=================
*/
qboolean UI_ItemInAssetList (const char *check, int num, const char **list)
{
	int		i;

	if (!check || !list)
		return false;
	for (i=0; i<num; i++)
	{
		if (!list[i])
			continue;
		if ( !Q_strcasecmp((char *)check, (char *)list[i]) )
			return true;
	}
	return false;
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
	char	**list = 0, **itemFiles = 0;
	char	*path = NULL, *curItem = NULL, *p = NULL, *ext = NULL;
//	char	findName[1024];
	int		nItems = 0, nItemNames = 0, i, baseLen = 0, extLen = 0;

	// check pointers
	if (!dir || !nameMask || !firstItem || !returnCount || !checkName)
		return NULL;
	if (maxItems < 2) // must allow at least 2 items
		return NULL;

	list = UI_Malloc(sizeof(char *) * (maxItems+1));
//	memset (list, 0, sizeof(char *) * (maxItems+1));

	// set first item name
	list[0] = UI_CopyString(firstItem); 
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

		if ( !UI_ItemInAssetList(curItem, nItemNames, list) )
		{
			// frontInsert not needed due to sorting in FS_GetFileList()
			UI_InsertInAssetList (list, curItem, nItemNames, 1, false);	// start=1 so first item stays first!
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

			if ( !UI_ItemInAssetList(curItem, nItemNames, list) )
			{
				UI_InsertInAssetList (list, curItem, nItemNames, 1, frontInsert);	// start=1 so first item stays first!
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

			if ( !UI_ItemInAssetList(curItem, nItemNames, list) )
			{
				UI_InsertInAssetList (list, curItem, nItemNames, 1, frontInsert);	// start=1 so first item stays first!
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
=================
UI_FreeAssetList
=================
*/
void UI_FreeAssetList (char **list, int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		if (list && list[i])
		{
			UI_Free (list[i]);
			list[i] = 0;
		}
	}
	UI_Free (list);
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
	qboolean	surround = false;
	cvar_t		*surround_threshold = Cvar_Get ("scr_surroundthreshold", "3.6", 0);

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
	ui_resolution_names = UI_Malloc((numModes+2) * sizeof(char *));
	ui_video_modes = UI_Malloc((numModes+2) * sizeof(char *));
//	memset (ui_resolution_names, 0, (numModes+2) * sizeof(char *));
//	memset (ui_video_modes, 0, (numModes+2) * sizeof(char *));

	// add custom resolution item
	ui_resolution_names[0] = UI_CopyString ("custom      ???");
//	ui_resolution_names[0] = UI_CopyString ("[custom   ] [ ??? ]");
	ui_video_modes[0] = UI_CopyString ("-1");	

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
			if (aspect > surround_threshold->value) {	// 3.6f
				aspect /= 3.0f;
				surround = true;
			}
			if (aspect > 3.49f)
				tok = "32:9";
			else if (aspect > 2.39f)
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

			if (surround)
				Com_sprintf (aspectBuf, sizeof(aspectBuf), "3x%s", tok);
			else
				Com_sprintf (aspectBuf, sizeof(aspectBuf), "%s", tok);

			Com_sprintf (nameBuf, sizeof(nameBuf), "%-12s%s", resBuf, aspectBuf);
		//	Com_sprintf (nameBuf, sizeof(nameBuf), "[%-9s] [%-5s]", resBuf, aspectBuf);

			ui_resolution_names[j] = UI_CopyString (nameBuf);
			ui_video_modes[j] = UI_CopyString (va("%i", i));	
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
		UI_FreeAssetList (ui_resolution_names, ui_num_video_modes);
		UI_FreeAssetList (ui_video_modes, ui_num_video_modes);
	}
	ui_resolution_names = NULL;
	ui_video_modes = NULL;
	ui_num_video_modes = 0;
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
	ui_aniso_names = UI_Malloc((numValues+1) * sizeof(char *));
	ui_aniso_values = UI_Malloc((numValues+1) * sizeof(char *));
//	memset (ui_aniso_names, 0, (numValues+1) * sizeof(char *));
//	memset (ui_aniso_values, 0, (numValues+1) * sizeof(char *));

	// set names and values
	for (i=0; i<numValues; i++)
	{
		if (i == 0)
			ui_aniso_names[i] = (numValues == 1) ? UI_CopyString("not supported") : UI_CopyString("off");
		else
			ui_aniso_names[i] = UI_CopyString(va("%ix", 1<<i));
		ui_aniso_values[i] = UI_CopyString(va("%i", 1<<i));
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
		UI_FreeAssetList (ui_aniso_names, ui_num_aniso_values);
		UI_FreeAssetList (ui_aniso_values, ui_num_aniso_values);
	}
	ui_aniso_names = NULL;
	ui_aniso_values = NULL;
	ui_num_aniso_values = 0;
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
		UI_FreeAssetList (ui_font_names, ui_numfonts);
	}
	ui_font_names = NULL;
	ui_numfonts = 0;
}

/*
=======================================================================

	HUD NAME LOADING

=======================================================================
*/

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
	size_t		len = strlen(name);

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
	if (ui_numhuds > 0) {
		UI_FreeAssetList (ui_hud_names, ui_numhuds);
	}
	ui_hud_names = NULL;
	ui_numhuds = 0;
}

/*
=======================================================================

	CROSSHAIR LOADING

=======================================================================
*/

#define UI_MAX_CROSSHAIRS 101	// none + ch1-ch100
char **ui_crosshair_names = NULL;
char **ui_crosshair_display_names = NULL;
color_t *ui_crosshair_display_colors = NULL;
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
	char		crosshA[16], crosshB[16];
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
			if ( !list[j] || !list[j+1] ) continue;
			Q_strncpyz (crosshA, sizeof(crosshA), list[j]+2);
			Q_strncpyz (crosshB, sizeof(crosshB), list[j+1]+2);
			if ( atoi(crosshA) > atoi(crosshB) )
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
	char		crossh[16];
	size_t		len;

	if ( !UI_IsValidImageFilename(name) )
		return false;

	Q_strncpyz (crossh, sizeof(crossh), name);
	// filename must be chxxx
	if ( strncmp(crossh, "ch", 2) ) 
		return false;
	len = strlen(crossh);
	if ( (len < 7) || (len > 9) )
		return false;
	if ( !isNumeric(crossh[2]) )
		return false;
	if ( (len >= 8) && !isNumeric(crossh[3]) )
		return false;
	// ch0 is invalid
	if ( (len == 7) && (crossh[2] == '0') )
		return false;
	// ch100 is only valid 5-char filename
	if ( (len == 9) && ( (crossh[2] != '1') || (crossh[3] != '0') || (crossh[4] != '0') ) )
		return false;
	// ch100-ch128 are only valid 5-char filenames
/*	if ( (len == 9) &&
		( !isNumeric(crossh[4]) || (crossh[2] != '1')
		|| (( crossh[2] == '1') && (crossh[3] > '2') )
		|| ( (crossh[2] == '1') && (crossh[3] == '2') && (crossh[4] > '8') ) ) )
		return false; */

	return true;
}


/*
==========================
UI_UpdateCrosshairColors
==========================
*/
void UI_UpdateCrosshairColors (int inRed, int inGreen, int inBlue)
{
	int		i, red, green, blue;

	red = min(max(inRed, 0), 255);
	green = min(max(inGreen, 0), 255);
	blue = min(max(inBlue, 0), 255);

	for (i = 0; i < ui_numcrosshairs; i++) {
		ui_crosshair_display_colors[i][0] = red;
		ui_crosshair_display_colors[i][1] = green;
		ui_crosshair_display_colors[i][2] = blue;
	}
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

	ui_crosshair_display_names = UI_Malloc(sizeof(char *) * (UI_MAX_CROSSHAIRS+1));
	memcpy (ui_crosshair_display_names, ui_crosshair_names, sizeof(char *) * (UI_MAX_CROSSHAIRS+1));
	ui_crosshair_display_names[0] = UI_CopyString("chnone");

	ui_crosshair_display_colors = UI_Malloc(sizeof(color_t) * (UI_MAX_CROSSHAIRS+1));
	memset (ui_crosshair_display_colors, 255, sizeof(color_t) * (UI_MAX_CROSSHAIRS+1));

	ui_crosshair_values = UI_Malloc(sizeof(char *) * (UI_MAX_CROSSHAIRS+1));
//	memset (ui_crosshair_values, 0, sizeof(char *) * (UI_MAX_CROSSHAIRS+1));

	for (i=0; i<ui_numcrosshairs; i++)
		ui_crosshair_values[i] = (i == 0) ? UI_CopyString("0") : UI_CopyString(strtok(ui_crosshair_names[i], "ch")); 
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
		UI_FreeAssetList (ui_crosshair_names, ui_numcrosshairs);
		if (ui_crosshair_display_names)
		{
			if (ui_crosshair_display_names[0]) {
				UI_Free (ui_crosshair_display_names[0]);
			}
			UI_Free (ui_crosshair_display_names);
		}
		UI_Free (ui_crosshair_display_colors);
		UI_FreeAssetList (ui_crosshair_values, ui_numcrosshairs);
	}
	ui_crosshair_names = NULL;
	ui_crosshair_display_names = NULL;
	ui_crosshair_display_colors = NULL;
	ui_crosshair_values = NULL;
	ui_numcrosshairs = 0;
}

/*
=======================================================================

	KEY BIND LIST LOADING

=======================================================================
*/

keyBindListHandle_t ui_customKeyBindList;

/*
==========================
UI_CountKeyBinds
==========================
*/
void UI_CountKeyBinds (keyBindListHandle_t *handle, char **script, char *buffer, int bufSize)
{
	char		*p, *tok;
	qboolean	validBindItem;

	if ( !handle || !script || !buffer )
		return;

	p = *script;
	while (p < (buffer + bufSize))
	{
		tok = COM_ParseExt (&p, true);
		if (!tok[0]) {
			Com_Printf (S_COLOR_YELLOW"WARNING: UI_CountKeyBinds: no concluding '}' in 'keyBindList' in keybind list %s\n", handle->fileName);
			return;
		}
		if ( !Q_stricmp(tok, "}") )
			break;
		else if ( !Q_strcasecmp(tok, "keyBind") )
		{
			validBindItem = true;
			
			tok = COM_ParseExt (&p, true);
			if (!tok[0]) {
				Com_Printf (S_COLOR_YELLOW"WARNING: UI_CountKeyBinds: missing parameters for 'keyBind' in keybind list %s\n", handle->fileName);
				validBindItem = false;
			}

			if ( !Q_stricmp(tok, "{") )
			{
				while (p < (buffer + bufSize))
				{
					tok = COM_ParseExt (&p, true);
					if (!tok[0]) {
						Com_Printf (S_COLOR_YELLOW"WARNING: UI_CountKeyBinds: no concluding '}' in 'keyBind' in keybind list %s\n", handle->fileName);
						validBindItem = false;
					}
					if ( !Q_stricmp(tok, "}") )
						break;
				}
			}
			else {
				Com_Printf (S_COLOR_YELLOW"WARNING: UI_CountKeyBinds: expected '{', found '%s' instead in 'keyBind' in keybind list %s\n", tok, handle->fileName);
				validBindItem = false;
			}

			if (validBindItem) {
				handle->maxKeyBinds++;
			}
		}
		else {
			Com_Printf (S_COLOR_YELLOW"WARNING: UI_CountKeyBinds: unknown command '%s' after item %i while looking for 'keyBind' in keybind list %s\n", tok, handle->maxKeyBinds, handle->fileName);
			return;
		}
	}
//	Com_Printf ("UI_CountKeyBinds: counted %i 'keyBind' items in keybind list %s\n", handle->maxKeyBinds, handle->fileName);
}


/*
==========================
UI_ParseKeyBind
==========================
*/
qboolean UI_ParseKeyBind (keyBindListHandle_t *handle, char **script, char *buffer, int bufSize)
{
	char		command[MAX_OSPATH] = {0};
	char		display[MAX_OSPATH] = {0};
	char		*tok;
	qboolean	gotCommand = false, gotDisplay = false;

	if ( !handle || !script || !buffer )
		return false;

	if (handle->numKeyBinds == handle->maxKeyBinds) {
		Com_Printf (S_COLOR_YELLOW"WARNING: UI_ParseKeyBind: exceeded maxKeyBinds (%i > %i) in hud script %s\n", handle->numKeyBinds, handle->maxKeyBinds, handle->fileName);
		return false;
	}

	tok = COM_ParseExt (script, true);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: UI_ParseKeyBind: missing parameters for 'keyBind' item %i in keybind list %s\n", handle->numKeyBinds+1, handle->fileName);
		return false;
	}

	if ( !Q_stricmp(tok, "{") )
	{
		while ( (*script) < (buffer + bufSize) )
		{
			tok = COM_ParseExt (script, true);
			if (!tok[0]) {
				Com_Printf (S_COLOR_YELLOW"WARNING: UI_ParseKeyBind: no concluding '}' in 'keyBind' item %i in keybind list %s\n", handle->numKeyBinds+1, handle->fileName);
				return false;
			}
			if ( !Q_stricmp(tok, "}") )
				break;
			if ( !Q_strcasecmp(tok, "commandName") )
			{
				tok = COM_ParseExt (script, true);
				if (!tok[0]) {
					Com_Printf (S_COLOR_YELLOW"WARNING: UI_ParseKeyBind: missing parameter for 'commandName' in 'keyBind' item %i in keybind list %s\n", handle->numKeyBinds+1, handle->fileName);
					return false;
				}
				Q_strncpyz (command, sizeof(command), tok);
				gotCommand = true;
			}
			else if ( !Q_strcasecmp(tok, "displayName") )
			{
				tok = COM_ParseExt (script, true);
				if (!tok[0]) {
					Com_Printf (S_COLOR_YELLOW"WARNING: UI_ParseKeyBind: missing parameter for 'displayName' in 'keyBind' item %i in keybind list %s\n", handle->numKeyBinds+1, handle->fileName);
					return false;
				}
				Q_strncpyz (display, sizeof(display), tok);
				gotDisplay = true;
			}
			else {
				Com_Printf (S_COLOR_YELLOW"WARNING: UI_ParseKeyBind: unknown function '%s' for 'keyBind' item %i in keybind list %s\n", tok, handle->numKeyBinds+1, handle->fileName);
				return false;
			}
		}
	}
	else {
		Com_Printf (S_COLOR_YELLOW"WARNING: UI_ParseKeyBind: expected '{', found '%s' instead in 'keyBind' item %i in keybind list %s\n", tok, handle->numKeyBinds+1, handle->fileName);
		return false;
	}

	if ( gotCommand && gotDisplay ) {
		handle->bindList[handle->numKeyBinds].commandName = UI_CopyString(command);
		handle->bindList[handle->numKeyBinds].displayName = UI_CopyString(display);
		handle->numKeyBinds++;
	}
	else {
		Com_Printf (S_COLOR_YELLOW"WARNING: UI_ParseKeyBind: 'keyBind' item %i is missing 'commandName' or 'displayName' in keybind list %s\n", handle->numKeyBinds+1, handle->fileName);
		return false;
	}

	return true;
}


/*
==========================
UI_ParseKeyBindList
==========================
*/
qboolean UI_ParseKeyBindList (keyBindListHandle_t *handle, char *buffer, int bufSize)
{
	char		*p, *tok;
	qboolean	foundKeyBindList = false;

	if ( !handle || !buffer )
		return false;

	p = buffer;
	while (p < (buffer + bufSize) )
	{
		tok = COM_ParseExt (&p, true);
		if (!tok[0])
			break;

		if ( !Q_strcasecmp(tok, "keyBindList") )
		{
			// only one keyBindList per file!
			if (foundKeyBindList) {
				Com_Printf (S_COLOR_YELLOW"WARNING: UI_ParseKeyBindList: found extra 'keyBindList' in keybind list %s\n", handle->fileName);
				return false;
			}
			foundKeyBindList = true;

			tok = COM_ParseExt (&p, true);
			if ( Q_stricmp(tok, "{") ) {
				Com_Printf (S_COLOR_YELLOW"WARNING: UI_ParseKeyBindList: expected '{', found %s instead in keybind list %s\n", tok, handle->fileName);
				return false;
			}

			// count num of keybinds and alloc memory accordingly
			UI_CountKeyBinds (handle, &p, buffer, bufSize);
		//	Com_Printf ("UI_ParseKeyBindList: allocating memory for %i 'keyBind' items in keybind list %s\n", handle->maxKeyBinds, handle->fileName);
			handle->bindList = UI_Malloc(sizeof(keyBindSubitem_t) * (handle->maxKeyBinds+1));
		//	memset (handle->bindList, 0, sizeof(keyBindSubitem_t) * (handle->maxKeyBinds+1));

			while (p < (buffer + bufSize))
			{
				tok = COM_ParseExt (&p, true);
				if (!tok[0]) {
					Com_Printf (S_COLOR_YELLOW"WARNING: UI_ParseKeyBindList: no concluding '}' in 'keyBindList' in keybind list %s\n", handle->fileName);
					return false;
				}
				if ( !Q_stricmp(tok, "}") )
					break;
				else if ( !Q_strcasecmp(tok, "keyBind") ) {
					if ( !UI_ParseKeyBind(handle, &p, buffer, bufSize) )
						return false;
				}
				else {
					Com_Printf (S_COLOR_YELLOW"WARNING: UI_ParseKeyBindList: unknown command '%s' after item %i while looking for 'keyBind' in keybind list %s\n", tok, handle->numKeyBinds, handle->fileName);
					return false;
				}
			}
		}
		// ignore any crap after the keyBindList
		else if ( !foundKeyBindList ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: UI_ParseKeyBindList: unknown command '%s' while looking for 'keyBindList' in keybind list %s\n", tok, handle->fileName);
			return false;
		}
	}

	return true;
}


/*
==========================
UI_LoadKeyBindListFromFile
==========================
*/
void UI_LoadKeyBindListFromFile (keyBindListHandle_t *handle)
{
	int		i, bufSize;
	char	*buffer;

	if ( !handle || !handle->fileName || (strlen(handle->fileName) < 1) )
		return;

	bufSize = FS_LoadFile (handle->fileName, (void **)&buffer);
	if ( !buffer ) {
		// failed to load, keep bindlist if already loaded
		Com_Printf ("UI_LoadKeyBindListFromFile: couldn't load %s\n", handle->fileName);
		return;
	}
	if (bufSize < 1) {
		// 0 size file, keep bindlist if already loaded
		Com_Printf ("UI_LoadKeyBindListFromFile: %s has 0 size\n", handle->fileName);
		FS_FreeFile (buffer);
		return;
	}

	// Parse it
	if ( UI_ParseKeyBindList(handle, buffer, bufSize) ) {
		Com_Printf ("UI_LoadKeyBindListFromFile: loaded keyBindList %s with %i keyBinds\n", handle->fileName, handle->numKeyBinds);
	}
	else {	// handle parse failure
		if (handle->bindList)
		{
			for (i=0; i<handle->maxKeyBinds; i++) {
				if (handle->bindList[i].commandName)
					UI_Free ((char *)handle->bindList[i].commandName);
				if (handle->bindList[i].displayName)
					UI_Free ((char *)handle->bindList[i].displayName);
			}
			UI_Free (handle->bindList);
			handle->bindList = NULL;
			handle->maxKeyBinds = 0;
			handle->numKeyBinds = 0;
		}
	}

	FS_FreeFile (buffer);
}


/*
==========================
UI_LoadKeyBindList
==========================
*/
void UI_LoadKeyBindList (void)
{
	memset (&ui_customKeyBindList, 0, sizeof(ui_customKeyBindList));
	Com_sprintf (ui_customKeyBindList.fileName, sizeof(ui_customKeyBindList.fileName), UI_CUSTOM_KEYBIND_FILE);
	UI_LoadKeyBindListFromFile (&ui_customKeyBindList);
}


/*
==========================
UI_FreeKeyBindList
==========================
*/
void UI_FreeKeyBindList (void)
{
	int		i;

	if (ui_customKeyBindList.bindList)
	{
		for (i=0; i<ui_customKeyBindList.maxKeyBinds; i++) {
			if (ui_customKeyBindList.bindList[i].commandName)
				UI_Free ((char *)ui_customKeyBindList.bindList[i].commandName);
			if (ui_customKeyBindList.bindList[i].displayName)
				UI_Free ((char *)ui_customKeyBindList.bindList[i].displayName);
		}
		UI_Free (ui_customKeyBindList.bindList);
		ui_customKeyBindList.bindList = NULL;
		ui_customKeyBindList.maxKeyBinds = 0;
		ui_customKeyBindList.numKeyBinds = 0;
	}
}

/*
=======================================================================

	MOD LIST LOADING

=======================================================================
*/

#define UI_MAX_MODS 256
#define MAX_MODINFO_TEXT 8192

char		**ui_mod_names = NULL;
char		**ui_mod_values = NULL;
modInfo_t	*ui_mod_info = NULL;
int			ui_num_mods = 0;

/*
==========================
UI_ParseModInfoFromFile
==========================
*/
qboolean UI_ParseModInfoFromFile (const char *filename, char *gameDir, char *modTitle, char *baseGame1, char *baseGame2, char *baseGame3,
									char *quakeImportPathAuto, char *quakeRRImportPathAuto, char *quakeImportPath,
									char *quakeMainGame, char *quakeGame1, char *quakeGame2, char *quakeGame3, char *quakeGame4,
									char *quake2RRImportPathAuto, char *quake2RRImportPath, char *quake2RRMainGame,
									char *quake2RRGame1, char *quake2RRGame2, char *quake2RRGame3, char *quake2RRGame4,
									size_t bufSize)
{
	int				len;
	fileHandle_t	f;
	char			buf[MAX_MODINFO_TEXT];
	char			*s, *token, *dest;
	qboolean		foundModInfo = false;

	// sanity check pointers and bufSize
	if ( !filename || !gameDir || !modTitle || !baseGame1 || !baseGame2 || !baseGame3 ||
		!quakeImportPathAuto || !quakeRRImportPathAuto || !quakeImportPath ||
		!quakeMainGame || !quakeGame1 || !quakeGame2  || !quakeGame3  || !quakeGame4  || (bufSize < 1) )
		return false;

	// clear buffers
	gameDir[0] = modTitle[0] = 0;
	baseGame1[0] = baseGame2[0] = baseGame3[0] = 0;
	quakeImportPathAuto[0] = quakeRRImportPathAuto[0] = quakeImportPath[0] = 0;
	quakeMainGame[0] = quakeGame1[0] = quakeGame2[0] = quakeGame3[0] = quakeGame4[0] = 0;
	quake2RRImportPathAuto[0] = quake2RRImportPath[0] = 0;
	quake2RRMainGame[0] = quake2RRGame1[0] = quake2RRGame2[0] = quake2RRGame3[0] = quake2RRGame4[0] = 0;

	len = FS_FOpenFile (filename, &f, FS_READ);
	if (!f) {
	//	Com_Printf ("UI_ParseModInfoFromFile: file not found: %s\n", filename);
		return false;
	}
	if (len >= MAX_MODINFO_TEXT) {
		Com_Printf (S_COLOR_RED"UI_ParseModInfoFromFile: file too large: %s is %i, max allowed is %i", filename, len, MAX_MODINFO_TEXT);
		FS_FCloseFile (f);
		return false;
	}

	FS_Read (buf, len, f);
	buf[len] = 0;
	FS_FCloseFile (f);

//	Com_Printf ("UI_ParseModInfoFromFile: loaded file %s of size %i\n", filename, len);

	s = buf;
	while (s < (buf + len))
	{
		token = COM_ParseExt (&s, true);
		if (!token[0])
			break;

		if ( !Q_strcasecmp(token, "modInfo") )
		{
			// only one modInfo per file!
			if (foundModInfo) {
				Com_Printf ("UI_ParseModInfoFromFile: found extra 'modInfo' in file %s\n", filename);
				return false;
			}
			foundModInfo = true;

			token = COM_ParseExt (&s, true);
			if (token[0] != '{') {
				Com_Printf ("UI_ParseModInfoFromFile: found %s when expecting '{' in file %s\n", token, filename);
				return false;
			}

			// go through all the parms
			while (s < (buf + len))
			{
				dest = NULL;
				token = COM_ParseExt (&s, true);
				if ( !token[0] || !s ) {
					Com_Printf ("UI_ParseModInfoFromFile: EOF without closing brace in file %s\n", filename);
					break;
				}
				if (token[0] == '}')
					break;

				if ( !Q_strcasecmp(token, "gameDir") )
					dest = gameDir;
				else if ( !Q_strcasecmp(token, "title") )
					dest = modTitle;
				else if ( !Q_strcasecmp(token, "baseGame") )
					dest = baseGame1;
				else if ( !Q_strcasecmp(token, "baseGame2") )
					dest = baseGame2;
				else if ( !Q_strcasecmp(token, "baseGame3") )
					dest = baseGame3;
				else if ( !Q_strcasecmp(token, "quakeImportPathAuto") )
					dest = quakeImportPathAuto;
				else if ( !Q_strcasecmp(token, "quakeRRImportPathAuto") )
					dest = quakeRRImportPathAuto;
				else if ( !Q_strcasecmp(token, "quakeImportPath") )
					dest = quakeImportPath;
				else if ( !Q_strcasecmp(token, "quakeMainGame") )
					dest = quakeMainGame;
				else if ( !Q_strcasecmp(token, "quakeGame1") )
					dest = quakeGame1;
				else if ( !Q_strcasecmp(token, "quakeGame2") )
					dest = quakeGame2;
				else if ( !Q_strcasecmp(token, "quakeGame3") )
					dest = quakeGame3;
				else if ( !Q_strcasecmp(token, "quakeGame4") )
					dest = quakeGame4;
				else if ( !Q_strcasecmp(token, "quake2RRImportPathAuto") )
					dest = quake2RRImportPathAuto;
				else if ( !Q_strcasecmp(token, "quake2RRImportPath") )
					dest = quake2RRImportPath;
				else if ( !Q_strcasecmp(token, "quake2RRMainGame") )
					dest = quake2RRMainGame;
				else if ( !Q_strcasecmp(token, "quake2RRGame1") )
					dest = quake2RRGame1;
				else if ( !Q_strcasecmp(token, "quake2RRGame2") )
					dest = quake2RRGame2;
				else if ( !Q_strcasecmp(token, "quake2RRGame3") )
					dest = quake2RRGame3;
				else if ( !Q_strcasecmp(token, "quake2RRGame4") )
					dest = quake2RRGame4;
				else
					Com_Printf ("UI_ParseModInfoFromFile: unknown parameter '%s' in file %s\n", token, filename);

				if (dest)
				{
					token = COM_ParseExt (&s, true);
					if (!token[0]) {
						Com_Printf ("UI_ParseModInfoFromFile: unexpected EOF in file %s\n", filename);
						return false;
					}
					if (token[0] == '}') {
						Com_Printf ("UI_ParseModInfoFromFile: closing brace without data in file %s\n", filename);
						break;
					}
					if (!s) {
						Com_Printf ("UI_ParseModInfoFromFile: EOF without closing brace in file %s\n", filename);
						break;
					}
					Q_strncpyz (dest, bufSize, token);
				}
			}
		}
		// ignore any crap after the modInfo
		else if ( !foundModInfo ) {
			Com_Printf ("UI_ParseModInfoFromFile: unknown command '%s' while looking for 'modInfo' in file %s\n", token, filename);
			return false;
		}
	}
	if (strlen(gameDir) == 0) {
		Com_Printf (S_COLOR_ORANGE "UI_ParseModInfoFromFile: 'gameDir' not found in file %s\n", filename);
		return false;
	}
	if (strlen(modTitle) == 0) {
		Com_Printf (S_COLOR_ORANGE "UI_ParseModInfoFromFile: 'title' not found in file %s\n", filename);
		return false;
	}
	return true;
}


/*
==========================
UI_BuildModList
==========================
*/
void UI_BuildModList (void)
{
	char		findName[1024];
	char		infoFile[MAX_OSPATH];
	char		modDesc[MAX_TOKEN_CHARS];
	char		modFormatedName[MAX_TOKEN_CHARS];
	char		**dirnames;
	char		*modDir, *modName;
	char		gameDir[MAX_TOKEN_CHARS], baseGame1[MAX_TOKEN_CHARS], baseGame2[MAX_TOKEN_CHARS], baseGame3[MAX_TOKEN_CHARS];
	char		quakeImportPathAuto[MAX_TOKEN_CHARS], quakeRRImportPathAuto[MAX_TOKEN_CHARS];
	char		quakeImportPath[MAX_TOKEN_CHARS], quakeMainGame[MAX_TOKEN_CHARS];
	char		quakeGame1[MAX_TOKEN_CHARS], quakeGame2[MAX_TOKEN_CHARS], quakeGame3[MAX_TOKEN_CHARS], quakeGame4[MAX_TOKEN_CHARS];
	char		quake2RRImportPathAuto[MAX_TOKEN_CHARS];
	char		quake2RRImportPath[MAX_TOKEN_CHARS], quake2RRMainGame[MAX_TOKEN_CHARS];
	char		quake2RRGame1[MAX_TOKEN_CHARS], quake2RRGame2[MAX_TOKEN_CHARS], quake2RRGame3[MAX_TOKEN_CHARS], quake2RRGame4[MAX_TOKEN_CHARS];
	int			count = 0, ndirs = 0, nmods = 0;
	int			i;
	qboolean	unsupportedMod = false;

	ui_mod_names = UI_Malloc(sizeof(char *) * (UI_MAX_MODS+1));
	ui_mod_values = UI_Malloc(sizeof(char *) * (UI_MAX_MODS+1));
	ui_mod_info = UI_Malloc(sizeof(modInfo_t) * (UI_MAX_MODS+1));

	// add baseq2 first
	ui_mod_names[0] = UI_CopyString("Quake II (vanilla)"); 
	ui_mod_values[0] = UI_CopyString(BASEDIRNAME);
	ui_mod_info[0].gameDir = UI_CopyString(BASEDIRNAME);
	ui_mod_info[0].baseGame1 = UI_CopyString("");
	ui_mod_info[0].baseGame2 = UI_CopyString("");
	ui_mod_info[0].baseGame3 = UI_CopyString("");
	ui_mod_info[0].quakeImportPath = UI_CopyString("");
	ui_mod_info[0].quakeMainGame = UI_CopyString(Q1_MAINDIRNAME);
	ui_mod_info[0].quakeGame1 = UI_CopyString("");
	ui_mod_info[0].quakeGame2 = UI_CopyString("");
	ui_mod_info[0].quakeGame3 = UI_CopyString("");
	ui_mod_info[0].quakeGame4 = UI_CopyString("");
	ui_mod_info[0].quakeImportPathAuto = false;
	ui_mod_info[0].quakeRRImportPathAuto = false;
	ui_mod_info[0].quake2RRImportPath = UI_CopyString("");
	ui_mod_info[0].quake2RRMainGame = UI_CopyString(BASEDIRNAME);
	ui_mod_info[0].quake2RRGame1 = UI_CopyString("");
	ui_mod_info[0].quake2RRGame2 = UI_CopyString("");
	ui_mod_info[0].quake2RRGame3 = UI_CopyString("");
	ui_mod_info[0].quake2RRGame4 = UI_CopyString("");
	ui_mod_info[0].quake2RRImportPathAuto = false;
	ui_mod_info[0].isUnsupported = false;
	count++;

	// get a list of directories
	Com_sprintf (findName, sizeof(findName), "%s/*.*", FS_RootDataPath());
	dirnames = FS_ListFiles (findName, &ndirs, SFF_SUBDIR, 0);
	if (!dirnames) {
		ui_num_mods = count;
		return;
	}
	
	// go through the directories
	nmods = ndirs;
	if (nmods > UI_MAX_MODS)
		nmods = UI_MAX_MODS;
	if ( (count + nmods) > UI_MAX_MODS )
		nmods = UI_MAX_MODS - count;

	for (i = 0; i < nmods; i++)
	{
		if (dirnames[i] == 0)
			continue;
			
		modDir = COM_SkipPath(dirnames[i]);
		
		// Ignore baseq2
		if ( !Q_strcasecmp(modDir, BASEDIRNAME) )
			continue;
	
		// Must have a pak or pk3 file, a maps dir, or a modinfo.def file
		if ( !Sys_FindFirst( va("%s/*.pak", dirnames[i]), 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM) ) {
			Sys_FindClose ();
			if ( !Sys_FindFirst( va("%s/*.pk3", dirnames[i]), 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM) ) {
				Sys_FindClose ();
				if ( !Sys_FindFirst( va("%s/maps", dirnames[i]), SFF_SUBDIR, 0) ) {
					Sys_FindClose ();
					if ( !Sys_FindFirst( va("%s/modinfo.def", dirnames[i]), 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM) ) {
						Sys_FindClose ();
						continue;
					}
				}
			}
		}
		Sys_FindClose ();

		// check if this mod has a gamex86.dll/gamei386.so without an equivalent KMQ2 dll/so
		unsupportedMod = false;

	//	if ( Sys_FindFirst( va("%s/"STOCK_Q2_GAME_LIBRARY_NAME, dirnames[i]), 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM) )
		if ( FS_DirectFileExists( va("%s/"STOCK_Q2_GAME_LIBRARY_NAME, dirnames[i]) ) )
		{
		//	Sys_FindClose ();
		//	if ( !Sys_FindFirst( va("%s/"KMQ2_GAME_LIBRARY_NAME, dirnames[i]), 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM) ) {
			if ( !FS_DirectFileExists( va("%s/"KMQ2_GAME_LIBRARY_NAME, dirnames[i]) ) ) {
			//	Sys_FindClose ();
				unsupportedMod = true;
			//	Com_Printf ("UI_BuildModList: mod %s has an unsupported game library.\n", modDir);
			}
		}
	//	Sys_FindClose ();

		// clear optional fields
		baseGame1[0] = baseGame2[0] = baseGame3[0] = 0;
		quakeImportPathAuto[0] = quakeRRImportPathAuto[0] = 0;
		quakeImportPath[0] = quakeMainGame[0] = 0;
		quakeGame1[0] = quakeGame2[0] = quakeGame3[0] = quakeGame4[0] = 0;
		quake2RRImportPathAuto[0] = quake2RRImportPath[0] = quake2RRMainGame[0] = 0;
		quake2RRGame1[0] = quake2RRGame2[0] = quake2RRGame3[0] = quake2RRGame4[0] = 0;

		// try to load and parse modinfo.def
		Com_sprintf (infoFile, sizeof(infoFile), "../%s/modinfo.def", modDir);
		if ( UI_ParseModInfoFromFile(infoFile, gameDir, modDesc, baseGame1, baseGame2, baseGame3,
									quakeImportPathAuto, quakeRRImportPathAuto, quakeImportPath,
									quakeMainGame, quakeGame1, quakeGame2, quakeGame3, quakeGame4, 
									quake2RRImportPathAuto, quake2RRImportPath, quake2RRMainGame,
									quake2RRGame1, quake2RRGame2, quake2RRGame3, quake2RRGame4,
									MAX_TOKEN_CHARS) )
		{
		//	Com_Printf ("UI_BuildModList: loaded modinfo file %s\n  gameDir: %s modDesc: \"%s\"^r\n", infoFile, gameDir, modDesc);
		//	Com_Printf ("  baseGame1: %s baseGame2: %s baseGame3: %s \n", baseGame1, baseGame2, baseGame3);
		//	Com_Printf ("  quakeImportPathAuto: %s quakeRRImportPathAuto: %s quakeImportPath: %s\n", quakeImportPathAuto, quakeRRImportPathAuto, quakeImportPath);
		//	Com_Printf ("  quakeMainGame: %s quakeGame1: %s quakeGame2: %s quakeGame3: %s quakeGame4: %s\n", quakeMainGame, quakeGame1, quakeGame2, quakeGame3, quakeGame4);
		//	Com_Printf ("  quake2RRImportPathAuto: %s quake2RRImportPath: %s quake2RRMainGame: %s \n", quake2RRImportPathAuto, quake2RRImportPath, quake2RRMainGame);
		//	Com_Printf ("  quake2RRGame1: %s quake2RRGame2: %s quake2RRGame3: %s quake2RRGame4: %s\n", quake2RRGame1, quake2RRGame2, quake2RRGame3, quake2RRGame4);
			// make sure quakeMainGame is set
			if (quakeMainGame[0] == 0)
				Q_strncpyz (quakeMainGame, sizeof(quakeMainGame), Q1_MAINDIRNAME);
			// make sure quake2RRMainGame is set
			if (quake2RRMainGame[0] == 0)
				Q_strncpyz (quake2RRMainGame, sizeof(quake2RRMainGame), BASEDIRNAME);
			// catch gameDir in modinfo not matching game path
			if ( Q_strcasecmp(modDir, gameDir) != 0 ) {
				Com_Printf (S_COLOR_ORANGE"UI_BuildModList: modinfo file %s has conflicting 'gameDir' value: %s\n", infoFile, gameDir);
			}
			modName = modDesc;
		}
		else if ( !Q_strcasecmp(modDir, "ctf") )
			modName = "Quake II: Capture The Flag";
		else if ( !Q_strcasecmp(modDir, "xatrix") )
			modName = "Quake II: The Reckoning";
		else if ( !Q_strcasecmp(modDir, "rogue") )
			modName = "Quake II: Ground Zero";
		else if ( !Q_strcasecmp(modDir, "zaero") )
			modName = "Zaero Mission Pack";
		else if ( !Q_strcasecmp(modDir, "3zb2") )
			modName = "3rd-Zigock Bot II";
		else if ( !Q_strcasecmp(modDir, "awaken2") )
			modName = "Awakening II: The Resurrection";
		else if ( !Q_strcasecmp(modDir, "ra2") )
			modName = "Rocket Arena 2";
		else if ( !Q_strcasecmp(modDir, "bots") )
			modName = "Battle of the Sexes";
		else if ( !Q_strcasecmp(modDir, "lmctf") )
			modName = "Loki's Minions CTF";
		else if ( !Q_strcasecmp(modDir, "wf") )
			modName = "Weapons Factory";
		else if ( !Q_strcasecmp(modDir, "wod") )
			modName = "Weapons of Destruction";
		else if ( !Q_strcasecmp(modDir, "rts") )
			modName = "Rob the Strogg";
		else
			modName = modDir;

		if (unsupportedMod)
			Com_sprintf (modFormatedName, sizeof(modFormatedName), S_COLOR_ORANGE"%s\0", modName);
		else
			Q_strncpyz (modFormatedName, sizeof(modFormatedName), modName);

		if ( !UI_ItemInAssetList(modDir, count, ui_mod_values) )
		{
			UI_InsertInAssetList (ui_mod_names, modFormatedName, count, 1, false);	// start=1 so first item stays first!
			UI_InsertInAssetList (ui_mod_values, modDir, count, 1, false);	// start=1 so first item stays first!
			ui_mod_info[count].gameDir = UI_CopyString(modDir);
			ui_mod_info[count].baseGame1 = UI_CopyString(baseGame1);
			ui_mod_info[count].baseGame2 = UI_CopyString(baseGame2);
			ui_mod_info[count].baseGame3 = UI_CopyString(baseGame3);

			if ( ( (atoi(quakeImportPathAuto) > 0) || !strcmp(quakeImportPathAuto, "true") || !strcmp(quakeImportPathAuto, "yes") ) 
				&& ( (strcmp(quakeImportPathAuto, "false") != 0) && (strcmp(quakeImportPathAuto, "no") != 0) ) )
				ui_mod_info[count].quakeImportPathAuto = true;
			else
				ui_mod_info[count].quakeImportPathAuto = false;

			if ( ( (atoi(quakeRRImportPathAuto) > 0) || !strcmp(quakeRRImportPathAuto, "true") || !strcmp(quakeRRImportPathAuto, "yes") ) 
				&& ( (strcmp(quakeRRImportPathAuto, "false") != 0) && (strcmp(quakeRRImportPathAuto, "no") != 0) ) )
				ui_mod_info[count].quakeRRImportPathAuto = true;
			else
				ui_mod_info[count].quakeRRImportPathAuto = false;

			if ( ( (atoi(quake2RRImportPathAuto) > 0) || !strcmp(quake2RRImportPathAuto, "true") || !strcmp(quake2RRImportPathAuto, "yes") ) 
				&& ( (strcmp(quake2RRImportPathAuto, "false") != 0) && (strcmp(quake2RRImportPathAuto, "no") != 0) ) )
				ui_mod_info[count].quake2RRImportPathAuto = true;
			else
				ui_mod_info[count].quake2RRImportPathAuto = false;

			ui_mod_info[count].quakeImportPath = UI_CopyString(quakeImportPath);
			ui_mod_info[count].quakeMainGame = UI_CopyString(quakeMainGame);
			ui_mod_info[count].quakeGame1 = UI_CopyString(quakeGame1);
			ui_mod_info[count].quakeGame2 = UI_CopyString(quakeGame2);
			ui_mod_info[count].quakeGame3 = UI_CopyString(quakeGame3);
			ui_mod_info[count].quakeGame4 = UI_CopyString(quakeGame4);
			ui_mod_info[count].quake2RRImportPath = UI_CopyString(quake2RRImportPath);
			ui_mod_info[count].quake2RRMainGame = UI_CopyString(quake2RRMainGame);
			ui_mod_info[count].quake2RRGame1 = UI_CopyString(quake2RRGame1);
			ui_mod_info[count].quake2RRGame2 = UI_CopyString(quake2RRGame2);
			ui_mod_info[count].quake2RRGame3 = UI_CopyString(quake2RRGame3);
			ui_mod_info[count].quake2RRGame4 = UI_CopyString(quake2RRGame4);
			ui_mod_info[count].isUnsupported = unsupportedMod;
			count++;
		}
	}
	
	if (dirnames)
		FS_FreeFileList (dirnames, ndirs);

	ui_num_mods = count;
}


/*
==========================
UI_GetModList
==========================
*/
void UI_GetModList (void)
{
	UI_BuildModList ();
//	Com_Printf ("UI_GetModList: found %i mod dirs\n", ui_num_mods);
}


/*
==========================
UI_FreeModList
==========================
*/
void UI_FreeModList (void)
{
	int		i;

	if (ui_num_mods > 0)
	{
		UI_FreeAssetList (ui_mod_names, ui_num_mods);
		UI_FreeAssetList (ui_mod_values, ui_num_mods);
		// free modInfo table
		for (i = 0; i < ui_num_mods; i++)
		{
			UI_Free (ui_mod_info[i].gameDir);
			UI_Free (ui_mod_info[i].baseGame1);
			UI_Free (ui_mod_info[i].baseGame2);
			UI_Free (ui_mod_info[i].baseGame3);
			UI_Free (ui_mod_info[i].quakeImportPath);
			UI_Free (ui_mod_info[i].quakeMainGame);
			UI_Free (ui_mod_info[i].quakeGame1);
			UI_Free (ui_mod_info[i].quakeGame2);
			UI_Free (ui_mod_info[i].quakeGame3);
			UI_Free (ui_mod_info[i].quakeGame4);
			ui_mod_info[i].gameDir = NULL;
			ui_mod_info[i].baseGame1 = NULL;
			ui_mod_info[i].baseGame2 = NULL;
			ui_mod_info[i].baseGame3 = NULL;
			ui_mod_info[i].quakeImportPath = NULL;
			ui_mod_info[i].quakeMainGame = NULL;
			ui_mod_info[i].quakeGame1 = NULL;
			ui_mod_info[i].quakeGame2 = NULL;
			ui_mod_info[i].quakeGame3 = NULL;
			ui_mod_info[i].quakeGame4 = NULL;
		}
		UI_Free (ui_mod_info);
	}
	ui_mod_names = NULL;
	ui_mod_values = NULL;
	ui_mod_info = NULL;
	ui_num_mods = 0;
}

/*
=============================================================================

SAVEGAME / SAVESHOT HANDLING

=============================================================================
*/

char		*ui_savegame_names[UI_MAX_SAVEGAMES];
char		*ui_loadgame_names[UI_MAX_SAVEGAMES+1];
char		ui_savestrings[UI_MAX_SAVEGAMES][64]; // The default here is what TFOL uses - Brad
qboolean	ui_savevalid[UI_MAX_SAVEGAMES];
time_t		ui_savetimestamps[UI_MAX_SAVEGAMES];
qboolean	ui_savechanged[UI_MAX_SAVEGAMES];
qboolean	ui_saveshot_valid[UI_MAX_SAVEGAMES+1];
float		ui_saveshot_aspect[UI_MAX_SAVEGAMES+1];

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
	char	comment[32];
	char	mapname[MAX_TOKEN_CHARS];
	char	infoHeader[10];
	char	*ch;
	time_t	old_timestamp;
	struct	stat	st;

	for (i=0; i<UI_MAX_SAVEGAMES; i++)
	{
	//	Com_sprintf (name, sizeof(name), "%s/save/kmq2save%03i/server.ssv", FS_Savegamedir(), i);
#ifdef NOTTHIRTYFLIGHTS
		Com_sprintf (name, sizeof(name), "%s/%s/kmq2save%03i/server.ssv", FS_Savegamedir(), ARCH_SAVEDIR, i);	// was FS_Gamedir()
#else
		Com_sprintf (name, sizeof(name), "%s/%s/save%03i/server.ssv", FS_Savegamedir(), ARCH_SAVEDIR, i);	// was FS_Gamedir()
#endif

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
#ifdef NOTTHIRTYFLIGHTS
			Com_sprintf (name, sizeof(name), "%s/kmq2save%03i/server.ssv", ARCH_SAVEDIR, i);
#else
			Com_sprintf (name, sizeof(name), "%s/save%03i/server.ssv", ARCH_SAVEDIR, i);
#endif
			FS_FOpenFile (name, &f, FS_READ);
			if (!f) {
				Q_strncpyz (ui_savestrings[i], sizeof(ui_savestrings[i]), EMPTY_GAME_STRING);
				ui_savevalid[i] = false;
				ui_savetimestamps[i] = 0;
			}
			else
			{
			//	FS_Read (ui_savestrings[i], sizeof(ui_savestrings[i]), f);
				FS_Read (comment, sizeof(comment), f);
				Com_sprintf (ui_savestrings[i], sizeof(ui_savestrings[i]), "%s", comment);

				// grab mapname
				FS_Read (mapname, sizeof(mapname), f);
				if (i == 0)
				{
				//	FS_Read (mapname, sizeof(mapname), f);
					if (mapname[0] == '*') // skip * marker
						Com_sprintf (ui_mapname, sizeof(ui_mapname), mapname+1);
					else
						Com_sprintf (ui_mapname, sizeof(ui_mapname), mapname);
					if (ch = strchr (ui_mapname, '$'))
						*ch = 0; // terminate string at $ marker
				}

				// read extra save info if present
				FS_Read (infoHeader, sizeof(infoHeader), f);
				if ( !strncmp(infoHeader, "KMQ2SSV", 7) )
				{
					char	comment2[32], aspectstring[18], datestring[20];
					struct	tm		savedate;
					int		hour;
					float	aspect;

					FS_Read (&savedate, sizeof(struct tm), f);
					FS_Read (comment, sizeof(comment), f);
					FS_Read (comment2, sizeof(comment2), f);
					FS_Read (aspectstring, sizeof(aspectstring), f);

					hour = savedate.tm_hour;
					if (hour > 12) hour -= 12;
					if (hour == 0) hour += 12;
					Com_sprintf (datestring, sizeof(datestring), "%2i/%2i/%4i %2i:%i%i %s", savedate.tm_mon+1, savedate.tm_mday, savedate.tm_year+1900,
								hour, savedate.tm_min/10, savedate.tm_min%10, (savedate.tm_hour > 12) ? "PM" : "AM");

					if (strlen(comment2) > 0)
						Com_sprintf (ui_savestrings[i], sizeof(ui_savestrings[i]), "%s\n%s %s", comment, comment2, datestring);
					else
						Com_sprintf (ui_savestrings[i], sizeof(ui_savestrings[i]), "%s\n%s", comment, datestring);

					aspect = atof(aspectstring);
					if (aspect > 0.0f)
						ui_saveshot_aspect[i] = aspect;
					else
						ui_saveshot_aspect[i] = 0.0f;	// 0.0f defaults to current screen aspect
				}
				FS_FCloseFile(f);
				ui_savevalid[i] = true;
			}
		}
		ui_savechanged[i] = (ui_savetimestamps[i] != old_timestamp);
	}

	// create pointer arrays for load and save menus
	ui_loadgame_names[UI_MAX_SAVEGAMES] = NULL;
	for (i = 0; i < UI_MAX_SAVEGAMES; i++)
		ui_loadgame_names[i] = ui_savestrings[i];

	ui_savegame_names[UI_MAX_SAVEGAMES-1] = NULL;
	for (i = 0; i < UI_MAX_SAVEGAMES-1; i++)
		ui_savegame_names[i] = ui_savestrings[i+1];	// don't include the autosave slot
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
			if (i == 0)	// mapshot for autosave
				Com_sprintf(shotname, sizeof(shotname), "/levelshots/%s.pcx", ui_mapname);
			else
			{	// free previously loaded shots
			//	Com_sprintf(shotname, sizeof(shotname), "save/kmq2save%03i/shot.jpg", i);
#ifdef NOTTHIRTYFLIGHTS
				Com_sprintf(shotname, sizeof(shotname), "%s/kmq2save%03i/shot.jpg", ARCH_SAVEDIR, i);
#else
				Com_sprintf(shotname, sizeof(shotname), "%s/save%03i/shot.jpg", ARCH_SAVEDIR, i);
#endif
				R_FreePic (shotname);
			//	Com_sprintf(shotname, sizeof(shotname), "/save/kmq2save%03i/shot.jpg", i);
#ifdef NOTTHIRTYFLIGHTS
				Com_sprintf(shotname, sizeof(shotname), "/%s/kmq2save%03i/shot.jpg", ARCH_SAVEDIR, i);
#else
				Com_sprintf(shotname, sizeof(shotname), "/%s/save%03i/shot.jpg", ARCH_SAVEDIR, i);
#endif
			}
			if (R_DrawFindPic(shotname))
				ui_saveshot_valid[i] = true;
			else
				ui_saveshot_valid[i] = false;
		}
		else
			ui_saveshot_valid[i] = false;
	}
}


/*
==========================
UI_UpdateSaveshotAspect
==========================
*/
float UI_UpdateSaveshotAspect (int index)
{
	float	saveAspect, curAspect;

	// check index
	if (index < 0 || index >= UI_MAX_SAVEGAMES)
		return DEFAULT_SAVESHOT_ASPECT;

	if ( (index > 0) && UI_SaveshotIsValid(index) )
	{
		saveAspect = ui_saveshot_aspect[index];
		curAspect = UI_GetScreenAspect();
		if (saveAspect == 0.0f)		// 0.0f defaults to current screen aspect
			return curAspect;
		else
			return min(max(saveAspect, 1.2f), (curAspect * 1.2f));	// clamp between 5:4 and 1.2 * current screen aspect
	}
	else {
		return DEFAULT_SAVESHOT_ASPECT;	// levelshots and null saveshot are 4:3
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

	if ( ui_savevalid[index] && ui_saveshot_valid[index] ) {
		if ( index == 0 )
			Com_sprintf (ui_saveload_shotname, sizeof(ui_saveload_shotname), "/levelshots/%s.pcx", ui_mapname);
		else
		//	Com_sprintf (ui_saveload_shotname, sizeof(ui_saveload_shotname), "/save/kmq2save%03i/shot.jpg", index);
#ifdef NOTTHIRTYFLIGHTS
			Com_sprintf (ui_saveload_shotname, sizeof(ui_saveload_shotname), "/%s/kmq2save%03i/shot.jpg", ARCH_SAVEDIR, index);
#else
			Com_sprintf (ui_saveload_shotname, sizeof(ui_saveload_shotname), "/%s/save%03i/shot.jpg", ARCH_SAVEDIR, index);
#endif
	}
	else if ( ui_saveshot_valid[UI_MAX_SAVEGAMES] )
		Com_sprintf (ui_saveload_shotname, sizeof(ui_saveload_shotname), UI_NOSCREEN_NAME);
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
		ui_saveshot_aspect[i] = DEFAULT_SAVESHOT_ASPECT;
	}
	ui_saveshot_aspect[UI_MAX_SAVEGAMES] = DEFAULT_SAVESHOT_ASPECT;

	UI_Load_Savestrings (false);
	UI_ValidateSaveshots ();	// register saveshots

	// register null saveshot, this is only done once
	if ( R_DrawFindPic(UI_NOSCREEN_NAME) )
		ui_saveshot_valid[UI_MAX_SAVEGAMES] = true;
	else
		ui_saveshot_valid[UI_MAX_SAVEGAMES] = false;
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

	return ui_saveshot_valid[index];
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
char	*ui_serverlist_names[UI_MAX_LOCAL_SERVERS+1];

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
    int     iPing = 0;
    char    *pszProtocol = NULL;

	if (ui_num_servers == UI_MAX_LOCAL_SERVERS)
		return;

	while ( *info == ' ' )
		info++;

	// FIXME: How to check if address is a loopback?
//	if ( adr.type == NA_LOOPBACK )
//		return;

	// Ignore if duplicated
	for (i=0; i<ui_num_servers; i++)
        if ( strncmp(info, &ui_local_server_names[i][11], sizeof(ui_local_server_names[0])-10) == 0 )
			return;

    iPing = 0 ;
	for (i=0 ; i<UI_MAX_LOCAL_SERVERS ; i++)
    {
        if ( (memcmp(&adr.ip, &global_adr_server_netadr[i].ip, sizeof(adr.ip)) == 0)
             && (adr.port == global_adr_server_netadr[i].port) )
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
		Q_strncpyz (ui_local_server_names[i], sizeof(ui_local_server_names[i]), NO_SERVER_STRING);

	UI_DrawPopupMessage ("Searching for local servers.\nThis could take up to a minute,\nso please be patient.");

	// send out info packets
	CL_PingServers_f ();
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

	ui_serverlist_names[UI_MAX_LOCAL_SERVERS] = NULL;
	for ( i = 0; i < UI_MAX_LOCAL_SERVERS; i++ ) {
		Com_sprintf (ui_local_server_names[i], sizeof(ui_local_server_names[i]), NO_SERVER_STRING);
		ui_serverlist_names[i] = ui_local_server_names[i];
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

#define MAPLIST_FORMAT "%s\n%s"

maptype_t ui_svr_maptype;
char **ui_svr_mapnames = NULL;
static int	ui_svr_maplist_sizes[NUM_MAPTYPES] = {0, 0, 0, 0};
static char	**ui_svr_maplists[NUM_MAPTYPES] = {NULL, NULL, NULL, NULL};
int	ui_svr_listfile_nummaps = 0;
static char **ui_svr_listfile_mapnames = NULL;
static int	ui_svr_arena_nummaps[NUM_MAPTYPES] = {0, 0, 0, 0};
static char **ui_svr_arena_mapnames[NUM_MAPTYPES] = {NULL, NULL, NULL, NULL};
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
qboolean UI_ParseArenaFromFile (const char *filename, char *shortname, char *longname, char *gametypes, size_t bufSize)
{
	int				len;
	fileHandle_t	f;
	char			buf[MAX_ARENAS_TEXT];
	char			*s, *token, *dest;

	// sanity check pointers and bufSize
	if ( !filename || !shortname || !longname || !gametypes || (bufSize < 1) )
		return false;

	// zero buffers
	shortname[0] = longname[0] = gametypes[0] = 0;

	len = FS_FOpenFile (filename, &f, FS_READ);
	if (!f) {
		Com_Printf (S_COLOR_RED"UI_ParseArenaFromFile: file not found: %s\n", filename);
		return false;
	}
	if (len >= MAX_ARENAS_TEXT) {
		Com_Printf (S_COLOR_RED"UI_ParseArenaFromFile: file too large: %s is %i, max allowed is %i", filename, len, MAX_ARENAS_TEXT);
		FS_FCloseFile (f);
		return false;
	}

	FS_Read (buf, len, f);
	buf[len] = 0;
	FS_FCloseFile (f);

	s = buf;
	// get the opening curly brace
	token = COM_ParseExt (&s, true);
	if ( !token[0] || !s ) {
		Com_Printf ("UI_ParseArenaFromFile: unexpected EOF in file %s\n", filename);
		return false;
	}
	if (token[0] != '{') {
		Com_Printf ("UI_ParseArenaFromFile: found %s when expecting '{' in file %s\n", token, filename);
		return false;
	}

	// go through all the parms
	while (s < (buf + len))
	{
		dest = NULL;
		token = COM_ParseExt (&s, true);
		if ( !token[0] || !s ) {
			Com_Printf ("UI_ParseArenaFromFile: EOF without closing brace in file %s\n", filename);
			break;
		}
		if (token[0] == '}')
			break;

		if ( !Q_strcasecmp(token, "map") )
			dest = shortname;
		else if ( !Q_strcasecmp(token, "longname") )
			dest = longname;
		else if ( !Q_strcasecmp(token, "type") )
			dest = gametypes;
		else
			Com_Printf ("UI_ParseArenaFromFile: unknown parameter '%s' in file %s\n", token, filename);

		if (dest)
		{
			token = COM_ParseExt (&s, true);
			if (!token[0]) {
				Com_Printf ("UI_ParseArenaFromFile: unexpected EOF in file %s\n", filename);
				return false;
			}
			if (token[0] == '}') {
				Com_Printf ("UI_ParseArenaFromFile: closing brace without data in file %s\n", filename);
				break;
			}
			if (!s) {
				Com_Printf ("UI_ParseArenaFromFile: EOF without closing brace in file %s\n", filename);
				break;
			}
			Q_strncpyz (dest, bufSize, token);
		}
	}
	if (strlen(shortname) == 0) {
		Com_Printf (S_COLOR_RED "UI_ParseArenaFromFile: %s: 'map' parm not found\n", filename);
		return false;
	}
	if (strlen(longname) == 0) {
		Q_strncpyz (longname, bufSize, shortname);
	}
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
	char		*p, *s1, *s2, *tok2;	// *tok1
	char		**arenafiles = NULL;
	char		**tmplist = NULL;
	char		*path = NULL;
//	char		findName[1024];
	char		shortname[MAX_TOKEN_CHARS];
	char		longname[MAX_TOKEN_CHARS];
	char		gametypes[MAX_TOKEN_CHARS];
	char		scratch[200];
	char		tok1[MAX_TOKEN_CHARS];
	int			i, j, len, narenas = 0, narenanames = 0;
	qboolean	type_supported[NUM_MAPTYPES];

	//
	// free existing lists and allocate new ones
	//
	for (i=0; i<NUM_MAPTYPES; i++)
	{
		if (ui_svr_arena_mapnames[i])
			UI_FreeAssetList (ui_svr_arena_mapnames[i], ui_svr_arena_nummaps[i]);
		ui_svr_arena_nummaps[i] = 0;
		ui_svr_arena_mapnames[i] = UI_Malloc(sizeof(char *) * MAX_ARENAS);
	//	memset (ui_svr_arena_mapnames[i], 0, sizeof(char *) * MAX_ARENAS);
	}

	tmplist = UI_Malloc(sizeof(char *) * MAX_ARENAS);
//	memset (tmplist, 0, sizeof(char *) * MAX_ARENAS);

#if 1
	arenafiles = FS_GetFileList ("scripts", "arena", &narenas);
	for (i = 0; i < narenas && narenanames < MAX_ARENAS; i++)
	{
		if (!arenafiles || !arenafiles[i])
			continue;

		len = (int)strlen(arenafiles[i]);
		if ( strcmp(arenafiles[i] + max(len-6, 0), ".arena") )
			continue;

		p = arenafiles[i];

		if ( !UI_ItemInAssetList(p, narenanames, tmplist) ) // check if already in list
		{
			if (UI_ParseArenaFromFile (p, shortname, longname, gametypes, MAX_TOKEN_CHARS))
			{
				Com_sprintf(scratch, sizeof(scratch), MAPLIST_FORMAT, longname, shortname);
				
				for (j=0; j<NUM_MAPTYPES; j++)
					type_supported[j] = false;
				s1 = gametypes;
				Q_strncpyz (tok1, sizeof(tok1), COM_Parse(&s1));
				while (s1 != NULL)
				{
					for (j=0; j<NUM_MAPTYPES; j++)
					{
						s2 = gametype_names[j].tokens;
						tok2 = COM_Parse (&s2);
						while (s2 != NULL) {
							if ( !Q_strcasecmp(tok1, tok2) )
								type_supported[j] = true;
							tok2 = COM_Parse (&s2);
						}
					}
					Q_strncpyz (tok1, sizeof(tok1), COM_Parse(&s1));
				}

				for (j=0; j<NUM_MAPTYPES; j++)
					if (type_supported[j]) {
						ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]] = UI_CopyString(scratch);
						ui_svr_arena_nummaps[j]++;
					}

			//	Com_Printf ("UI_LoadArenas: successfully loaded arena file %s: mapname: %s levelname: %s gametypes: %s\n", p, shortname, longname, gametypes);
				narenanames++;
				UI_InsertInAssetList (tmplist, p, narenanames, 0, false); // add to list
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
			if ( strcmp(arenafiles[i] + max(len-6, 0), ".arena") )
				continue;

			p = arenafiles[i] + strlen(path) + 1;	// skip over path and next slash

			if ( !UI_ItemInAssetList(p, narenanames, tmplist) ) // check if already in list
			{
				if (UI_ParseArenaFromFile (p, shortname, longname, gametypes, MAX_TOKEN_CHARS))
				{
					Com_sprintf(scratch, sizeof(scratch), MAPLIST_FORMAT, longname, shortname);
					
					for (j=0; j<NUM_MAPTYPES; j++)
						type_supported[j] = false;
					s1 = gametypes;
					Q_strncpyz (tok1, sizeof(tok1), COM_Parse(&s1));
					while (s1 != NULL)
					{
						for (j=0; j<NUM_MAPTYPES; j++)
						{
							s2 = gametype_names[j].tokens;
							tok2 = COM_Parse (&s2);
							while (s2 != NULL) {
								if ( !Q_strcasecmp(tok1, tok2) )
									type_supported[j] = true;
								tok2 = COM_Parse (&s2);
							}
						}
						Q_strncpyz (tok1, sizeof(tok1), COM_Parse(&s1));
					}

					for (j=0; j<NUM_MAPTYPES; j++)
						if (type_supported[j]) {
							ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]] = UI_CopyString(scratch);
							ui_svr_arena_nummaps[j]++;
						}

				//	Com_Printf ("UI_LoadArenas: successfully loaded arena file %s: mapname: %s levelname: %s gametypes: %s\n", p, shortname, longname, gametypes);
					narenanames++;
					UI_InsertInAssetList (tmplist, p, narenanames, 0, false); // add to list
				}			
			}
		}
		if (narenas > 0)
			FS_FreeFileList (arenafiles, narenas);
		
		path = FS_NextPath (path);
	}

	//
	// check in paks for .arena files
	//
	if (arenafiles = FS_ListPak("scripts/", &narenas))
	{
		for (i = 0; i < narenas && narenanames < MAX_ARENAS; i++)
		{
			if (!arenafiles || !arenafiles[i])
				continue;

			len = (int)strlen(arenafiles[i]);
			if ( strcmp(arenafiles[i] + max(len-6, 0), ".arena") )
				continue;

			p = arenafiles[i];

			if ( !UI_ItemInAssetList(p, narenanames, tmplist) ) // check if already in list
			{
				if (UI_ParseArenaFromFile (p, shortname, longname, gametypes, MAX_TOKEN_CHARS))
				{
					Com_sprintf(scratch, sizeof(scratch), MAPLIST_FORMAT, longname, shortname);
					
					for (j=0; j<NUM_MAPTYPES; j++)
						type_supported[j] = false;
					s1 = gametypes;
					Q_strncpyz (tok1, sizeof(tok1), COM_Parse(&s1));
					while (s1 != NULL)
					{
						for (j=0; j<NUM_MAPTYPES; j++)
						{
							s2 = gametype_names[j].tokens;
							tok2 = COM_Parse (&s2);
							while (s2 != NULL) {
								if ( !Q_strcasecmp(tok1, tok2) )
									type_supported[j] = true;
								tok2 = COM_Parse (&s2);
							}
						}
						Q_strncpyz (tok1, sizeof(tok1), COM_Parse(&s1));
					}

					for (j=0; j<NUM_MAPTYPES; j++)
						if (type_supported[j]) {
							ui_svr_arena_mapnames[j][ui_svr_arena_nummaps[j]] = UI_CopyString(scratch);
							ui_svr_arena_nummaps[j]++;
						}

				//	Com_Printf ("UI_LoadArenas: successfully loaded arena file %s: mapname: %s levelname: %s gametypes: %s\n", p, shortname, longname, gametypes);
					narenanames++;
					UI_InsertInAssetList (tmplist, p, narenanames, 0, false); // add to list
				}
			}
		}
	}
#endif

	if (narenas > 0)
		FS_FreeFileList (arenafiles, narenas);

	if (narenanames > 0)
		UI_FreeAssetList (tmplist, narenanames);

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
	FILE	*fp;

	//
	// free existing list
	//
	if (ui_svr_listfile_mapnames) {
		UI_FreeAssetList (ui_svr_listfile_mapnames, ui_svr_listfile_nummaps);
	}
	ui_svr_listfile_mapnames = NULL;
	ui_svr_listfile_nummaps = 0;

	//
	// load the list of map names
	//
	Com_sprintf( mapsname, sizeof(mapsname), "%s/maps.lst", FS_Gamedir() );	// FIXME: should this be FS_Savegamedir()?
	if ( ( fp = fopen(mapsname, "rb") ) == 0 )
	{
		if ( ( length = FS_LoadFile("maps.lst", (void **)&buffer)) == -1 )
			Com_Error (ERR_DROP, "couldn't find maps.lst\n");
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
		buffer = malloc(length);
		fread (buffer, length, 1, fp);
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

	ui_svr_listfile_mapnames = UI_Malloc(sizeof(char *) * (ui_svr_listfile_nummaps + 1));
//	memset (ui_svr_listfile_mapnames, 0, sizeof(char *) * (ui_svr_listfile_nummaps + 1));

	s = buffer;

	for (i = 0; i < ui_svr_listfile_nummaps; i++)
	{
		char	shortname[MAX_TOKEN_CHARS];
		char	longname[MAX_TOKEN_CHARS];
		char	scratch[200];

	//	strncpy (shortname, COM_Parse(&s));
	//	strncpy (longname, COM_Parse(&s));
		Q_strncpyz (shortname, sizeof(shortname), COM_Parse(&s));
		Q_strncpyz (longname, sizeof(longname), COM_Parse(&s));
		Com_sprintf (scratch, sizeof(scratch), MAPLIST_FORMAT, longname, shortname);
		ui_svr_listfile_mapnames[i] = UI_CopyString(scratch);
	}
	ui_svr_listfile_mapnames[ui_svr_listfile_nummaps] = 0;

	if ( fp != 0 )
	{
		fp = 0;
		free (buffer);
	}
	else
		FS_FreeFile (buffer);

	UI_LoadArenas ();

	// build composite map lists
	for (i=0; i<NUM_MAPTYPES; i++)
	{
		if (ui_svr_maplists[i]) {
			UI_Free (ui_svr_maplists[i]);
		}
		ui_svr_maplists[i] = NULL;
		ui_svr_maplist_sizes[i] = ui_svr_listfile_nummaps + ui_svr_arena_nummaps[i];
		ui_svr_maplists[i] = UI_Malloc(sizeof(char *) * (ui_svr_maplist_sizes[i] + 1));
	//	memset (ui_svr_maplists[i], 0, sizeof(char *) * (ui_svr_maplist_sizes[i] + 1));

		for (j = 0; j < ui_svr_maplist_sizes[i]; j++)
		{
			if (j < ui_svr_listfile_nummaps)
				ui_svr_maplists[i][j] = ui_svr_listfile_mapnames[j];
			else
				ui_svr_maplists[i][j] = ui_svr_arena_mapnames[i][j-ui_svr_listfile_nummaps];
		}
	}

	ui_svr_maptype = MAP_DM; // initial maptype
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
			UI_Free (ui_svr_maplists[i]);
		}
		ui_svr_maplists[i] = NULL;
		ui_svr_maplist_sizes[i] = 0;
	}
	ui_svr_mapnames	= NULL;

	//
	// free list from file
	//
	if (ui_svr_listfile_mapnames) {
		UI_FreeAssetList (ui_svr_listfile_mapnames, ui_svr_listfile_nummaps);
	}
	ui_svr_listfile_mapnames = NULL;
	ui_svr_listfile_nummaps = 0;

	//
	// free arena lists
	//
	for (i=0; i<NUM_MAPTYPES; i++)
	{
		if (ui_svr_arena_mapnames[i]) {
			UI_FreeAssetList (ui_svr_arena_mapnames[i], ui_svr_arena_nummaps[i]);
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
		if (ui_svr_mapshotvalid[i]) {
			UI_Free (ui_svr_mapshotvalid[i]);
			ui_svr_mapshotvalid[i] = NULL;
		}

		// alloc and zero new list
		ui_svr_mapshotvalid[i] = UI_Malloc(sizeof(byte) * (ui_svr_maplist_sizes[i] + 1));
	//	memset (ui_svr_mapshotvalid[i], 0, sizeof(byte) * (ui_svr_maplist_sizes[i] + 1));

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
		if (ui_svr_mapshotvalid[i]) {
			UI_Free (ui_svr_mapshotvalid[i]);
			ui_svr_mapshotvalid[i] = NULL;
		}
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
int ui_numplayermodels = 0;

// save skins and models here so as to not have to re-register every frame
struct model_s *ui_playermodel;
struct model_s *ui_weaponmodel;
struct image_s *ui_playerskin;
char *ui_currentweaponmodel;

char	ui_playerconfig_playermodelname[MAX_QPATH];
char	ui_playerconfig_playerskinname[MAX_QPATH];
char	ui_playerconfig_weaponmodelname[MAX_QPATH];
/*
color_t ui_player_color_imageColors[] =
{
#include "ui_playercolors.h"
};
// last table entry is null value
#define UI_NUM_PLAYER_COLORS ((sizeof(ui_player_color_imageColors) / sizeof(ui_player_color_imageColors[0])) - 1)
char **ui_player_color_values = NULL;
char **ui_player_color_imageNames = NULL;
int ui_numplayercolors = 0;
*/

/*
==========================
UI_IsSkinIcon
==========================
*/
static qboolean UI_IsSkinIcon (char *name)
{
	int		len;
	char	*s, scratch[1024];

	Q_strncpyz (scratch, sizeof(scratch), name);
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
	char findName[1024];
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
			Com_sprintf (findName, sizeof(findName), "%s/players/*.*", path);

			if ( (dirnames = FS_ListFiles(findName, &ndirs, SFF_SUBDIR, 0)) != 0 )
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
		//	strncpy (scratch, dirnames[i]);
		//	strncat (scratch, "/tris.md2");
			Q_strncpyz (scratch, sizeof(scratch), dirnames[i]);
			Q_strncatz (scratch, sizeof(scratch), "/tris.md2");
			if ( !Sys_FindFirst(scratch, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM) )
			{
				free (dirnames[i]);
				dirnames[i] = 0;
				Sys_FindClose ();
				continue;
			}
			Sys_FindClose ();

			// verify the existence of at least one skin
			Q_strncpyz (scratch, sizeof(scratch), va("%s%s", dirnames[i], "/*.*")); // was "/*.pcx"
			imagenames = FS_ListFiles (scratch, &nimagefiles, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM);

			if (!imagenames)
			{
				free (dirnames[i]);
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

			Q_strncpyz (ui_pmi[ui_numplayermodels].displayname, sizeof(ui_pmi[ui_numplayermodels].displayname), c+1);
			Q_strncpyz (ui_pmi[ui_numplayermodels].directory, sizeof(ui_pmi[ui_numplayermodels].directory), c+1);

			skinnames = UI_Malloc(sizeof(char *) * (nskins+1));
			skiniconnames = UI_Malloc(sizeof(char *) * (nskins+1));
		//	memset (skinnames, 0, sizeof(char *) * (nskins+1));
		//	memset (skiniconnames, 0, sizeof(char *) * (nskins+1));

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
						Q_strncpyz (scratch, sizeof(scratch), c+1);

						if ( strrchr(scratch, '.') )
							*strrchr(scratch, '.') = 0;

						skinnames[s] = UI_CopyString(scratch);
						skiniconnames[s] = UI_CopyString(va("/players/%s/%s_i.pcx", ui_pmi[ui_numplayermodels].directory, scratch));
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

		//	Q_strncpyz (ui_pmi[ui_numplayermodels].displayname, sizeof(ui_pmi[ui_numplayermodels].displayname), c+1);
		//	Q_strncpyz (ui_pmi[ui_numplayermodels].directory, sizeof(ui_pmi[ui_numplayermodels].directory), c+1);

			FS_FreeFileList (imagenames, nimagefiles);

			ui_numplayermodels++;
		}
		
		if (dirnames)
			FS_FreeFileList (dirnames, ndirs);

	// if no valid player models found in path,
	// try next path, if there is one
	} while (path);	// (ui_numplayermodels == 0 && path);

	return true;	// DMP warning fix
}

#if 0
/*
==========================
UI_BuildPlayerColorList
==========================
*/
void UI_BuildPlayerColorList (void)
{
	int		i, numColors = 0;

	ui_player_color_values = UI_Malloc(sizeof(char *) * (UI_NUM_PLAYER_COLORS+1));
	ui_player_color_imageNames = UI_Malloc(sizeof(char *) * (UI_NUM_PLAYER_COLORS+1));
//	memset (ui_player_color_values, 0, sizeof(char *) * (UI_NUM_PLAYER_COLORS+1));
//	memset (ui_player_color_imageNames, 0, sizeof(char *) * (UI_NUM_PLAYER_COLORS+1));

	for (i = 0; i < UI_NUM_PLAYER_COLORS; i++)
	{	// last index is custom color
		if (i == UI_NUM_PLAYER_COLORS-1) {
			ui_player_color_values[i] = UI_CopyString(UI_ITEMVALUE_WILDCARD);
			ui_player_color_imageNames[i] = UI_CopyString(UI_CUSTOMCOLOR_PIC);
		}
		else {
			ui_player_color_values[i] = UI_CopyString(va("%02X%02X%02X", ui_player_color_imageColors[i][0], ui_player_color_imageColors[i][1], ui_player_color_imageColors[i][2]));
			ui_player_color_imageNames[i] = UI_CopyString(UI_SOLIDWHITE_PIC);
		}
		numColors++;
	}

	ui_numplayercolors = numColors;
}
#endif

/*
==========================
UI_LoadPlayerModels
==========================
*/
void UI_LoadPlayerModels (void)
{
	UI_PlayerConfig_ScanDirectories ();
//	UI_BuildPlayerColorList ();
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
		if (ui_pmi[i].nskins > 0) {
			UI_FreeAssetList (ui_pmi[i].skinDisplayNames, ui_pmi[i].nskins);
			UI_FreeAssetList (ui_pmi[i].skinIconNames, ui_pmi[i].nskins);
		}
		ui_pmi[i].skinDisplayNames = NULL;
		ui_pmi[i].skinIconNames = NULL;
		ui_pmi[i].nskins = 0;
	}
	ui_numplayermodels = 0;

/*	if (ui_numplayercolors > 0)
	{
		UI_FreeAssetList (ui_player_color_values, ui_numplayercolors);
		UI_FreeAssetList (ui_player_color_imageNames, ui_numplayercolors);
	}
	ui_player_color_values = NULL;
	ui_player_color_imageNames = NULL;
	ui_numplayercolors = 0; */
}


/*
==========================
UI_RefreshPlayerModels

Reloads player models if we recently downloaded a player model.
==========================
*/
void UI_RefreshPlayerModels (void)
{
	if (cls.refreshPlayerModels) {
		Com_DPrintf ("UI_RefreshPlayerModels: reloading player models due to recent download of a player model.\n");
		UI_FreePlayerModels ();
		UI_LoadPlayerModels ();
		cls.refreshPlayerModels = false;	// clear the flag
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
		
	Q_strncpyz (currentdirectory, sizeof(currentdirectory), Cvar_VariableString ("skin"));

	if ( strchr(currentdirectory, '/') )
	{
		Q_strncpyz (currentskin, sizeof(currentskin), strchr( currentdirectory, '/' ) + 1);
		*strchr(currentdirectory, '/') = 0;
	}
	else if ( strchr(currentdirectory, '\\') )
	{
		Q_strncpyz (currentskin, sizeof(currentskin), strchr( currentdirectory, '\\' ) + 1);
		*strchr(currentdirectory, '\\') = 0;
	}
	else
	{
		Q_strncpyz (currentdirectory, sizeof(currentdirectory), "male");
		Q_strncpyz (currentskin, sizeof(currentskin), "grunt");
	}

	qsort (ui_pmi, ui_numplayermodels, sizeof(ui_pmi[0]), UI_PlayerModelCmpFunc);

	memset (ui_pmnames, 0, sizeof(ui_pmnames));
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
	Q_strncpyz (ui_playerconfig_playermodelname, sizeof(ui_playerconfig_playermodelname), scratch);

	Com_sprintf (scratch, sizeof(scratch), "players/%s/%s.pcx", ui_pmi[mNum].directory, ui_pmi[mNum].skinDisplayNames[sNum]);
	ui_playerskin = R_RegisterSkin (scratch);
	Q_strncpyz (ui_playerconfig_playerskinname, sizeof(ui_playerconfig_playerskinname), scratch);

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
	Q_strncpyz (ui_playerconfig_weaponmodelname, sizeof(ui_playerconfig_weaponmodelname), scratch);
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
	Q_strncpyz (ui_playerconfig_playerskinname, sizeof(ui_playerconfig_playerskinname), scratch);
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
