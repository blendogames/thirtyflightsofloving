/*
===========================================================================
Copyright (C) 2002-2004 Nicolas Flekenstein
Copyright (C) 2010-2022 Knightmare

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

// cl_hud_script.c -- parsing and drawing for for scripted HUDs

#include "client.h"
#include "../ui/ui_local.h"
#include "../qcommon/wildcard.h"

static hudDef_t			cl_hudDef;
static hudRenameSet_t	cl_hudRenames[HUD_MAX_RENAMESETS];
static unsigned int		cl_numHudRenames;
char					*cl_hudName = NULL;
char					*cl_hudVariant = NULL;

/*
===============================================================

SCRIPTED HUD PARSING

===============================================================
*/

/*
================
Hud_NameForDrawItem
================
*/
static char	hud_itemname_buf[MAX_QPATH+18];
char *Hud_NameForDrawItem (hudDrawItem_t *drawItem)
{
	char	*temp, *typeName;

	if (!drawItem) {
		typeName = "null drawItem";
		return typeName;
	}

	if	(drawItem->type == HUD_DRAWPIC)
		temp = "pic drawItem";
	else if (drawItem->type == HUD_DRAWSTRING)
		temp = "string drawItem";
	else if	(drawItem->type == HUD_DRAWNUMBER)
		temp = "number drawItem";
	else if	(drawItem->type == HUD_DRAWBAR)
		temp = "bar drawItem";
	else
		temp = "unknown drawItem";

	if ( strlen(drawItem->name) )
		Com_sprintf(hud_itemname_buf, sizeof(hud_itemname_buf), "%s '%s'", temp, drawItem->name);
	else
		Com_sprintf(hud_itemname_buf, sizeof(hud_itemname_buf), "%s", temp);

	typeName = hud_itemname_buf;
	return typeName;
}


/*
================
Hud_Parse_Variant
================
*/
qboolean Hud_Parse_Variant (hudVariant_t *variant, char **script, char *buffer, int bufSize)
{
	char		*tok;

	if ( !variant || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, false);
	if (!tok[0])
		return false;
	Q_strncpyz (variant->name, sizeof(variant->name), tok);

	return true;
}

/*
================
Hud_Parse_SkipIfStat
================
*/
qboolean Hud_Parse_SkipIfStat (hudSkipIfStat_t *skipIfStat, char **script, char *buffer, int bufSize)
{
	char		*tok;
	int			cmp = 0;
	qboolean	nextCmp;

	if ( !skipIfStat || !script || !buffer )
		return false;

	do
	{
		tok = COM_ParseExt (script, false);	// was true
		if (!tok[0]) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_Parse_SkipIfStat: missing parameters for 'skipIfStat' in hud script %s\n", cl_hudName);
			return false;
		}
		skipIfStat->stat[cmp] = min(max(atoi(tok), 0), MAX_STATS-1);

		tok = COM_ParseExt (script, false);	 // was true
		if (!tok[0]) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_Parse_SkipIfStat: missing parameters for 'skipIfStat' in hud script %s\n", cl_hudName);
			return false;
		}

		if ( !Q_stricmp(tok, ">") )
			skipIfStat->func[cmp] = HUD_GREATER;
		else if ( !Q_stricmp(tok, "<") )
			skipIfStat->func[cmp] = HUD_LESS;
		else if ( !Q_stricmp(tok, ">=") )
			skipIfStat->func[cmp] = HUD_GEQUAL;
		else if ( !Q_stricmp(tok, "<=") )
			skipIfStat->func[cmp] = HUD_LEQUAL;
		else if ( !Q_stricmp(tok, "==") )
			skipIfStat->func[cmp] = HUD_EQUAL;
		else if ( !Q_stricmp(tok, "!=") )
			skipIfStat->func[cmp] = HUD_NOTEQUAL;
		else if ( !Q_stricmp(tok, "&") )
			skipIfStat->func[cmp] = HUD_AND;
		else if ( !Q_stricmp(tok, "!&") || !Q_stricmp(tok, "&~"))
			skipIfStat->func[cmp] = HUD_NOTAND;
		else {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_Parse_SkipIfStat: unknown parameter '%s' for 'skipIfStat' in hud script %s\n", tok, cl_hudName);
			return false;
		}

		tok = COM_ParseExt (script, false);
		if (!tok[0]) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_Parse_SkipIfStat: missing parameters for 'skipIfStat' in hud script %s\n", cl_hudName);
			return false;
		}
		skipIfStat->value[cmp] = atoi(tok);

		cmp++;
		tok = COM_ParseExt (script, false);
		nextCmp = ( tok[0] && !Q_stricmp(tok, "&&") );

	} while (nextCmp && cmp < HUD_SKIPIFSTAT_MAX_CMPS);
	skipIfStat->numCmps = cmp;

//	if (skipIfStat->numCmps > 1)
//		Com_Printf ("Hud_Parse_SkipIfStat: parsed skipIfStat with %i cmps\n", skipIfStat->numCmps);

	return true;
}


/*
================
Hud_Parse_Int
================
*/
qboolean Hud_Parse_Int (int *value, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !value || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, false);
	if (!tok[0])
		return false;
	*value = atoi(tok);

	return true;
}


/*
================
Hud_Parse_Float
================
*/
qboolean Hud_Parse_Float (float *value, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !value || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, false);
	if (!tok[0])
		return false;
	*value = atof(tok);
	return true;
}


/*
================
Hud_Parse_ColorVec
================
*/
qboolean Hud_Parse_ColorVec (color_t *value, char **script, char *buffer, int bufSize)
{
	char	*tok;
	int		i;

	if ( !value || !script || !buffer )
		return false;

	for (i=0; i<4; i++)
	{
		tok = COM_ParseExt (script, false);
		if (!tok[0]) {
			return false;
		}
		(*value)[i] = 255 * min(max(atof(tok), 0.0f), 1.0f);
	}
	return true;
}


/*
================
Hud_ParseItem_Name
================
*/
qboolean Hud_ParseItem_Name (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, false);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Name: missing parameter for 'name' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	Q_strncpyz (drawItem->name, sizeof(drawItem->name), tok);
	return true;
}


/*
================
Hud_ParseItem_Type
================
*/
qboolean Hud_ParseItem_Type (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, false);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Type: missing parameter for 'type' in 'drawItem' in hud script %s\n", cl_hudName);
		return false;
	}
	if ( !Q_strcasecmp(tok, "pic") )
		drawItem->type = HUD_DRAWPIC;
	else if ( !Q_strcasecmp(tok, "string") )
		drawItem->type = HUD_DRAWSTRING;
	else if ( !Q_strcasecmp(tok, "number") )
		drawItem->type = HUD_DRAWNUMBER;
	else if ( !Q_strcasecmp(tok, "bar") )
		drawItem->type = HUD_DRAWBAR;
	else {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Type: unknown type '%s' for 'drawItem' in hud script %s\n", tok, cl_hudName);
		return false;
	}
	return true;
}


/*
================
Hud_ParseItem_Rect
================
*/
qboolean Hud_ParseItem_Rect (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	int		i;

	if ( !drawItem || !script || !buffer )
		return false;

	for (i=0; i<4; i++)
	{
		if ( !Hud_Parse_Int(&drawItem->rect[i], script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Rect: missing parameters for 'rect' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
	}
	return true;
}


/*
================
Hud_ParseItem_ScrAlign
================
*/
qboolean Hud_ParseItem_ScrAlign (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, false);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_ScrAlign: missing parameters for 'scralign' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	if ( !Q_strcasecmp(tok, "stretch") )
		drawItem->scrnAlignment = ALIGN_STRETCH;
	else if ( !Q_strcasecmp(tok, "center") )
		drawItem->scrnAlignment = ALIGN_CENTER;
	else if ( !Q_strcasecmp(tok, "top") )
		drawItem->scrnAlignment = ALIGN_TOP;
	else if ( !Q_strcasecmp(tok, "bottom") )
		drawItem->scrnAlignment = ALIGN_BOTTOM;
	else if ( !Q_strcasecmp(tok, "right") )
		drawItem->scrnAlignment = ALIGN_RIGHT;
	else if ( !Q_strcasecmp(tok, "left") )
		drawItem->scrnAlignment = ALIGN_LEFT;
	else if ( !Q_strcasecmp(tok, "topRight") )
		drawItem->scrnAlignment = ALIGN_TOPRIGHT;
	else if ( !Q_strcasecmp(tok, "topLeft") )
		drawItem->scrnAlignment = ALIGN_TOPLEFT;
	else if ( !Q_strcasecmp(tok, "bottomRight") )
		drawItem->scrnAlignment = ALIGN_BOTTOMRIGHT;
	else if ( !Q_strcasecmp(tok, "bottomLeft") )
		drawItem->scrnAlignment = ALIGN_BOTTOMLEFT;
	else if ( !Q_strcasecmp(tok, "topStretch") )
		drawItem->scrnAlignment = ALIGN_TOP_STRETCH;
	else if ( !Q_strcasecmp(tok, "bottomStretch") )
		drawItem->scrnAlignment = ALIGN_BOTTOM_STRETCH;
	else {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_ScrAlign: unknown parameter '%s' for 'scralign' in %s in hud script %s\n", tok, Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	return true;
}


/*
================
Hud_ParseItem_Size
================
*/
qboolean Hud_ParseItem_Size (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	int		i;

	if ( !drawItem || !script || !buffer )
		return false;

	for (i=0; i<2; i++)
	{
		if ( !Hud_Parse_Int(&drawItem->size[i], script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Size: missing parameters for 'size' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
	}
	return true;
}


/*
================
Hud_ParseItem_MinRange
================
*/
qboolean Hud_ParseItem_MinRange (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	int		i;
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, false);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_MinRange: missing parameters for 'minRange' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	if ( !Q_strcasecmp(tok, "fromStat") )
	{
		if ( !Hud_Parse_Int(&drawItem->minRangeStat, script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_MinRange: missing parameters for 'minRange' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
		drawItem->minRangeStat = min(max(drawItem->minRangeStat, 0), MAX_STATS-1);
		drawItem->fromStat |= HUD_STATMINRANGE;
		drawItem->minRangeStatMult = 1.0f;
		tok = COM_ParseExt (script, false);
		while (tok[0])
		{
			if ( !Q_strcasecmp(tok, "statClamp") )
			{
				for (i=0; i<2; i++) {
					if ( !Hud_Parse_Int(&drawItem->minRangeStatClamp[i], script, buffer, bufSize) ) {
						Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_MinRange: missing 'statClamp' parameters for 'minRange' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
						return false;
					}
				}
				drawItem->fromStat |= HUD_STATMINRANGECLAMP;
			}
			else if ( Q_strcasecmp(tok, "statMult") == 0 )
			{
				if ( Hud_Parse_Float(&drawItem->minRangeStatMult, script, buffer, bufSize) ) {
					// do nothing here
				}
				else {
					Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_MinRange: missing 'statMult' parameter for 'minRange' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
					return false;
				}
			}
			else {
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_MinRange: expected 'statMult' or 'statClamp', found '%s' for 'minRange' in %s in hud script %s\n", tok, Hud_NameForDrawItem(drawItem), cl_hudName);
				return false;
			}
			tok = COM_ParseExt (script, false);
		}
	}
	else
		drawItem->range[0] = atoi(tok);

	return true;
}


/*
================
Hud_ParseItem_MaxRange
================
*/
qboolean Hud_ParseItem_MaxRange (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	int		i;
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, false);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_MaxRange: missing parameters for 'maxRange' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	if ( !Q_strcasecmp(tok, "fromStat") )
	{
		if ( !Hud_Parse_Int(&drawItem->maxRangeStat, script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_MaxRange: missing parameters for 'maxRange' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
		drawItem->maxRangeStat = min(max(drawItem->maxRangeStat, 0), MAX_STATS-1);
		drawItem->fromStat |= HUD_STATMAXRANGE;
		drawItem->maxRangeStatMult = 1.0f;
		tok = COM_ParseExt (script, false);
		while (tok[0])
		{
			if ( !Q_strcasecmp(tok, "statClamp") )
			{
				for (i=0; i<2; i++) {
					if ( !Hud_Parse_Int(&drawItem->maxRangeStatClamp[i], script, buffer, bufSize) ) {
						Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_MaxRange: missing 'statClamp' parameters for 'maxRange' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
						return false;
					}
				}
				drawItem->fromStat |= HUD_STATMAXRANGECLAMP;
			}
			else if ( Q_strcasecmp(tok, "statMult") == 0 )
			{
				if ( Hud_Parse_Float(&drawItem->maxRangeStatMult, script, buffer, bufSize) ) {
					// do nothing here
				}
				else {
					Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_MaxRange: missing 'statMult' parameter for 'maxRange' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
					return false;
				}
			}
			else {
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_MaxRange: expected 'statMult' or 'statClamp', found '%s' for 'maxRange' in %s in hud script %s\n", tok, Hud_NameForDrawItem(drawItem), cl_hudName);
				return false;
			}
			tok = COM_ParseExt (script, false);
		}
	}
	else
		drawItem->range[1] = atoi(tok);

	return true;
}


/*
================
Hud_ParseItem_Range
================
*/
qboolean Hud_ParseItem_Range (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	int		i;

	if ( !drawItem || !script || !buffer )
		return false;

	for (i=0; i<2; i++)
	{
		if ( !Hud_Parse_Int(&drawItem->range[i], script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Range: missing parameters for 'range' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
	}
	return true;
}


/*
================
Hud_ParseItem_Offset
================
*/
qboolean Hud_ParseItem_Offset (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	int		i;

	if ( !drawItem || !script || !buffer )
		return false;

	for (i=0; i<2; i++)
	{
		if ( !Hud_Parse_Float(&drawItem->offset[i], script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Offset: missing parameters for 'offset' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
	}
	return true;
}


/*
================
Hud_ParseItem_Skew
================
*/
qboolean Hud_ParseItem_Skew (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	int		i;

	if ( !drawItem || !script || !buffer )
		return false;

	for (i=0; i<2; i++)
	{
		if ( !Hud_Parse_Float(&drawItem->skew[i], script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Skew: missing parameters for 'skew' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
	}
	return true;
}


/*
================
Hud_ParseItem_Scroll
================
*/
qboolean Hud_ParseItem_Scroll (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	int		i;

	if ( !drawItem || !script || !buffer )
		return false;

	for (i=0; i<2; i++)
	{
		if ( !Hud_Parse_Float(&drawItem->scroll[i], script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Scroll: missing parameters for 'scroll' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
	}
	return true;
}


/*
================
Hud_ParseItem_Padding
================
*/
qboolean Hud_ParseItem_Padding (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	if ( !drawItem || !script || !buffer )
		return false;

	if ( !Hud_Parse_Float(&drawItem->padding, script, buffer, bufSize) ) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Padding: missing parameter for 'padding' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	return true;
}


/*
================
Hud_ParseItem_FillDir
================
*/
qboolean Hud_ParseItem_FillDir (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, false);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_FillDir: missing parameters for 'fillDir' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	if ( !Q_strcasecmp(tok, "leftToRight") )
		drawItem->fillDir = HUD_LEFTTORIGHT;
	else if ( !Q_strcasecmp(tok, "rightToLeft") )
		drawItem->fillDir = HUD_RIGHTTOLEFT;
	else if ( !Q_strcasecmp(tok, "bottomToTop") )
		drawItem->fillDir = HUD_BOTTOMTOTOP;
	else if ( !Q_strcasecmp(tok, "topToBottom") )
		drawItem->fillDir = HUD_TOPTOBOTTOM;
	else {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_FillDir: unknown parameter '%s' for 'fillDir' in %s in hud script %s\n", tok, Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	return true;
}


/*
================
Hud_ParseItem_Color
================
*/
qboolean Hud_ParseItem_Color (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	if ( !Hud_Parse_ColorVec(&drawItem->color, script, buffer, bufSize) ) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Color: missing parameters for 'color' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	Vector4Copy (drawItem->color, drawItem->blinkColor);
	tok = COM_ParseExt (script, false);
	if (tok[0])
	{
		if ( Q_strcasecmp(tok, "blink") && Q_strcasecmp(tok, "flash") ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Color: expected 'blink', found '%s' instead in 'color' in %s in hud script %s\n", tok, Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
		if ( !Hud_Parse_ColorVec(&drawItem->blinkColor, script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Color: missing parameters for 'color' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
	}
	return true;
}


/*
================
Hud_ParseItem_ColorRange
================
*/
qboolean Hud_ParseItem_ColorRange (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize, int type)
{
	char	*tok;
	int		index;

	if ( !drawItem || !script || !buffer )
		return false;

	if (drawItem->numColorRanges == HUD_MAX_COLOR_RANGES) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_ColorRange: exceeded HUD_MAX_COLOR_RANGES for %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	if (drawItem->numColorRanges > 0 && type != drawItem->colorRangeType) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_ColorRange: both addColorRange and addColorRangeRel set for %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}

	index = drawItem->numColorRanges;
	drawItem->colorRangeType = type;
	if ( !Hud_Parse_Float(&drawItem->colorRanges[index].low, script, buffer, bufSize) ||
		 !Hud_Parse_Float(&drawItem->colorRanges[index].high, script, buffer, bufSize) ||
		 !Hud_Parse_ColorVec(&drawItem->colorRanges[index].color, script, buffer, bufSize) ) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_ColorRange: missing parameters for 'addColorRange' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	Vector4Copy (drawItem->colorRanges[index].color, drawItem->colorRanges[index].blinkColor);
	tok = COM_ParseExt (script, false);
	if (tok[0])
	{
		if ( Q_strcasecmp(tok, "blink") && Q_strcasecmp(tok, "flash") ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_ColorRange: expected 'blink', found '%s' instead in 'addColorRange' in %s in hud script %s\n", tok, Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
		if ( !Hud_Parse_ColorVec(&drawItem->colorRanges[index].blinkColor, script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_ColorRange: missing parameters for 'addColorRange' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
	}
	drawItem->numColorRanges++;
	return true;
}


/*
================
Hud_ParseItem_ColorRangeStat
================
*/
qboolean Hud_ParseItem_ColorRangeStat (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	if ( !Hud_Parse_Int(&drawItem->colorRangeStat, script, buffer, bufSize) ) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_ColorRangeStat: missing parameters for 'colorRangeStat' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	tok = COM_ParseExt (script, false);
	if (tok[0])
	{
		if ( Q_strcasecmp(tok, "maxStat") ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_ColorRangeStat: expected 'maxStat', found '%s' instead in 'colorRangeStat' in %s in hud script %s\n", tok, Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
		if ( !Hud_Parse_Int(&drawItem->colorRangeMaxStat, script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_ColorRangeStat: missing parameters for 'colorRangeStat' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
	}
	return true;
}


/*
================
Hud_ParseItem_StatFlash
================
*/
qboolean Hud_ParseItem_StatFlash (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	if ( !drawItem || !script || !buffer )
		return false;

	if ( !Hud_Parse_Int(&drawItem->statFlash.stat, script, buffer, bufSize) || 
		 !Hud_Parse_Int(&drawItem->statFlash.bit, script, buffer, bufSize) ||
		 !Hud_Parse_ColorVec(&drawItem->statFlash.color, script, buffer, bufSize) ) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_StatFlash: missing parameters for 'statFlash' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	return true;
}


/*
================
Hud_ParseItem_SkipIfStat
================
*/
qboolean Hud_ParseItem_SkipIfStat (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	hudSkipIfStat_t		*skipIfStat;

	if ( !drawItem || !script || !buffer )
		return false;

	if (drawItem->numSkipIfStats == HUD_ITEM_MAX_SKIPIFSTATS) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_SkipIfStat: exceeded HUD_ITEM_MAX_SKIPIFSTATS in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	skipIfStat = &drawItem->skipIfStats[drawItem->numSkipIfStats++];

	return Hud_Parse_SkipIfStat(skipIfStat, script, buffer, bufSize);
}


/*
================
Hud_ParseItem_Flags
================
*/
qboolean Hud_ParseItem_Flags (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	while ( (*script) < (buffer + bufSize) )
	{
		tok = COM_ParseExt (script, false);
		if (!tok[0])
			break;

		if ( !Q_strcasecmp(tok, "left") )
			drawItem->flags |= DSF_LEFT;
		else if ( !Q_strcasecmp(tok, "center") )
			drawItem->flags |= DSF_CENTER;
		else if ( !Q_strcasecmp(tok, "right") )
			drawItem->flags |= DSF_RIGHT;
		else if ( !Q_strcasecmp(tok, "lower") )
			drawItem->flags |= DSF_LOWERCASE;
		else if ( !Q_strcasecmp(tok, "upper") )
			drawItem->flags |= DSF_UPPERCASE;
		else if ( !Q_strcasecmp(tok, "shadow") )
			drawItem->flags |= DSF_DROPSHADOW;
		else if ( !Q_strcasecmp(tok, "flipx") )
			drawItem->flags |= DSF_FLIPX;
		else if ( !Q_strcasecmp(tok, "flipy") )
			drawItem->flags |= DSF_FLIPY;
		else if ( !Q_strcasecmp(tok, "overridepath") || !Q_strcasecmp(tok, "overridepic") )
			drawItem->flags |= DSF_OVERRIDEPATH;
		else if ( !Q_strcasecmp(tok, "masked") || !Q_strcasecmp(tok, "maskedpic") )
			drawItem->flags |= DSF_MASKSHADER;
		else if ( !Q_strcasecmp(tok, "additive") || !Q_strcasecmp(tok, "additiveBlend") )
			drawItem->flags |= DSF_ADDITIVESHADER;
		else {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Flags: unknown parameter '%s' for 'flags' in %s in hud script %s\n", tok, Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
	}
	return true;
}


/*
================
Hud_ParseItem_String
================
*/
qboolean Hud_ParseItem_String (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, false);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_String: missing parameters for 'string' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	if ( !Q_strcasecmp(tok, "fromStat") )
	{
		if ( !Hud_Parse_Int(&drawItem->stringStat, script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_String: missing parameters for 'string' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
		drawItem->stringStat = min(max(drawItem->stringStat, 0), MAX_STATS-1);
		drawItem->fromStat |= HUD_STATSTRING;
	}
	else if ( !Q_strcasecmp(tok, "fromPlayerName") ) {
		drawItem->fromStat |= HUD_PLAYERNAMESTRING;
	}
	else
		Q_strncpyz (drawItem->string, sizeof(drawItem->string), tok);
	return true;
}


/*
================
Hud_ParseItem_Number
================
*/
qboolean Hud_ParseItem_Number (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, false);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Number: missing parameters for 'number' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	if ( !Q_strcasecmp(tok, "fromStat") )
	{
		if ( !Hud_Parse_Int(&drawItem->numberStat, script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Number: missing parameters for 'number' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
		drawItem->numberStat = min(max(drawItem->numberStat, 0), MAX_STATS-1);
		drawItem->fromStat |= HUD_STATNUMBER;
	}
	else
		drawItem->number = atoi(tok);
	return true;
}


/*
================
Hud_ParseItem_Value
================
*/
qboolean Hud_ParseItem_Value (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, false);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Value: missing parameters for 'value' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	if ( !Q_strcasecmp(tok, "fromStat") )
	{
		if ( !Hud_Parse_Int(&drawItem->valueStat, script, buffer, bufSize) ) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Value: missing parameters for 'value' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
		drawItem->valueStat = min(max(drawItem->valueStat, 0), MAX_STATS-1);
		drawItem->fromStat |= HUD_STATVALUE;
	}
	else
		drawItem->value = atoi(tok);
	return true;
}


/*
================
Hud_ParseItem_Shader
================
*/
qboolean Hud_ParseItem_Shader (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	Q_strncpyz (drawItem->shaderRenameSet, sizeof(drawItem->shaderRenameSet), "\0");
	tok = COM_ParseExt (script, false);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Shader: missing parameters for 'shader' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	if ( !Q_strcasecmp(tok, "fromStat") )
	{
		tok = COM_ParseExt (script, false);
		if (!tok[0]) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Shader: missing parameters for 'shader' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
			return false;
		}
		drawItem->shaderStat = min(max(atoi(tok), 0), MAX_STATS-1);
		drawItem->fromStat |= HUD_STATSHADER;
	//	Com_Printf ("Hud_ParseItem_Shader: 'fromstat' parameter is %i for 'shader' in %s in hud script %s\n", drawItem->shaderStat, Hud_NameForDrawItem(drawItem), cl_hudName);
	}
	else if ( !Q_strcasecmp(tok, "fromWeaponModel") )
		drawItem->fromStat |= HUD_WEAPONMODELSHADER;
	else
		Q_strncpyz (drawItem->shader, sizeof(drawItem->shader), tok);

	if (drawItem->fromStat & (HUD_STATSHADER|HUD_WEAPONMODELSHADER))
	{
		tok = COM_ParseExt (script, false);
		if (tok[0])
		{
			if ( Q_strcasecmp(tok, "usingRenameSet") ) {
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Shader: expected 'usingRenameSet', found '%s' instead in 'shader' in %s in hud script %s\n", tok, Hud_NameForDrawItem(drawItem), cl_hudName);
				return false;
			}
			tok = COM_ParseExt (script, false);
			if (!tok[0]) {
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_Shader: missing parameters for 'usingRenameSet' in 'shader' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
				return false;
			}
			drawItem->flags |= DSF_RENAMESHADER;
			Q_strncpyz (drawItem->shaderRenameSet, sizeof(drawItem->shaderRenameSet), tok);
		//	Com_Printf ("Hud_ParseItem_Shader:  using renameSet %s for 'shader' in %s in hud script %s\n", drawItem->shaderRenameSet, Hud_NameForDrawItem(drawItem), cl_hudName);
		}
	}
	return true;
}


/*
================
Hud_ParseItem_ShaderMinus
================
*/
qboolean Hud_ParseItem_ShaderMinus (hudDrawItem_t *drawItem, char **script, char *buffer, int bufSize)
{
	char	*tok;

	if ( !drawItem || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, false);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItem_ShaderMinus: missing parameters for 'shaderMinus' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}
	Q_strncpyz (drawItem->shaderMinus, sizeof(drawItem->shaderMinus), tok);
	return true;
}


/*
================
Hud_ParseDrawItem
================
*/
qboolean Hud_ParseDrawItem (hudItemGroup_t *itemGroup, char **script, char *buffer, int bufSize)
{
	hudDrawItem_t	*drawItem;
	char			*tok;

	if ( !itemGroup || !script || !buffer )
		return false;

	if (itemGroup->numDrawItems == HUD_MAX_DRAWITEMS) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseDrawItem: exceeded HUD_MAX_DRAWITEMS in hud script %s\n", cl_hudName);
		return false;
	}

	drawItem = itemGroup->drawItems[itemGroup->numDrawItems++] = Z_Malloc(sizeof(hudDrawItem_t));
//	drawItem = &itemGroup->drawItems[itemGroup->numDrawItems++];

	tok = COM_ParseExt (script, false);
	if ( tok[0] )
		Q_strncpyz (drawItem->name, sizeof(drawItem->name), tok);

	// set some appropriate defaults
	drawItem->type = HUD_DRAWPIC;	// default type
	drawItem->scrnAlignment = ALIGN_STRETCH;
	drawItem->padding = 0;
	drawItem->color[0] = drawItem->color[1] = drawItem->color[2] = drawItem->color[3] = 255;
	Vector4Copy (drawItem->color, drawItem->blinkColor);
	drawItem->numColorRanges = 0;
	drawItem->numSkipIfStats = 0;

	tok = COM_ParseExt (script, true);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseDrawItem: missing parameters for %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}

	if ( !Q_stricmp(tok, "{") )
	{
		while ( (*script) < (buffer + bufSize) )
		{
			tok = COM_ParseExt (script, true);
			if (!tok[0]) {
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseDrawItem: no concluding '}' in %s in hud script %s\n", Hud_NameForDrawItem(drawItem), cl_hudName);
				return false;
			}
			if ( !Q_stricmp(tok, "}") )
				break;
			if ( !Q_strcasecmp(tok, "name") )
			{
				if ( !Hud_ParseItem_Name(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "type") )
			{
				if ( !Hud_ParseItem_Type(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "rect") )
			{
				if ( !Hud_ParseItem_Rect(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "scralign") || !Q_strcasecmp(tok, "align") )
			{
				if ( !Hud_ParseItem_ScrAlign(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "size") )
			{
				if ( !Hud_ParseItem_Size(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "minRange") )
			{
				if ( !Hud_ParseItem_MinRange(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "maxRange") )
			{
				if ( !Hud_ParseItem_MaxRange(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "range") )
			{
				if ( !Hud_ParseItem_Range(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "offset") )
			{
				if ( !Hud_ParseItem_Offset(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "skew") )
			{
				if ( !Hud_ParseItem_Skew(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "scroll") )
			{
				if ( !Hud_ParseItem_Scroll(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "padding") )
			{
				if ( !Hud_ParseItem_Padding(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "fillDir") )
			{
				if ( !Hud_ParseItem_FillDir(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "color") )
			{
				if ( !Hud_ParseItem_Color(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "addColorRange") )
			{
				if ( !Hud_ParseItem_ColorRange(drawItem, script, buffer, bufSize, HUD_RANGETYPE_ABSOLUTE) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "addColorRangeRel") )
			{
				if ( !Hud_ParseItem_ColorRange(drawItem, script, buffer, bufSize, HUD_RANGETYPE_RELATIVE) )
					return false;				
			}
			else if ( !Q_strcasecmp(tok, "colorRangeStat") )
			{
				if ( !Hud_ParseItem_ColorRangeStat(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "statFlash") )
			{
				if ( !Hud_ParseItem_StatFlash(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "skipIfStat") )
			{
				if ( !Hud_ParseItem_SkipIfStat(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "flags") )
			{
				if ( !Hud_ParseItem_Flags(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "string") )
			{
				if ( !Hud_ParseItem_String(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "number") )
			{
				if ( !Hud_ParseItem_Number(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "value") )
			{
				if ( !Hud_ParseItem_Value(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "shader") )
			{
				if ( !Hud_ParseItem_Shader(drawItem, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "shaderMinus") || !Q_strcasecmp(tok, "minus") )
			{
				if ( !Hud_ParseItem_ShaderMinus(drawItem, script, buffer, bufSize) )
					return false;
			}
			else {
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseDrawItem: unknown command '%s' in %s in hud script %s\n", tok, Hud_NameForDrawItem(drawItem), cl_hudName);
				return false;
			}
		}
	}
	else {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseDrawItem: expected '{', found '%s' instead in %s in hud script %s\n", tok, Hud_NameForDrawItem(drawItem), cl_hudName);
		return false;
	}

	return true;
}


/*
================
Hud_ParseItemGroup_SkipIfStat
================
*/
qboolean Hud_ParseItemGroup_Variant (hudItemGroup_t *itemGroup, char **script, char *buffer, int bufSize)
{
	hudVariant_t		*variant;

	if ( !itemGroup || !script || !buffer )
		return false;

	if (itemGroup->numVariants == HUD_GROUP_MAX_VARIANTS) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItemGroup_Variant: exceeded HUD_GROUP_MAX_VARIANTS in hud script %s\n", cl_hudName);
		return false;
	}
	variant = &itemGroup->variants[itemGroup->numVariants++];

	return Hud_Parse_Variant(variant, script, buffer, bufSize);
}


/*
================
Hud_ParseItemGroup_SkipIfStat
================
*/
qboolean Hud_ParseItemGroup_SkipIfStat (hudItemGroup_t *itemGroup, char **script, char *buffer, int bufSize)
{
	hudSkipIfStat_t		*skipIfStat;

	if ( !itemGroup || !script || !buffer )
		return false;

	if (itemGroup->numSkipIfStats == HUD_GROUP_MAX_SKIPIFSTATS) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItemGroup_SkipIfStat: exceeded HUD_GROUP_MAX_SKIPIFSTATS in hud script %s\n", cl_hudName);
		return false;
	}
	skipIfStat = &itemGroup->skipIfStats[itemGroup->numSkipIfStats++];

	return Hud_Parse_SkipIfStat(skipIfStat, script, buffer, bufSize);
}


/*
================
Hud_ParseItemGroup
================
*/
qboolean Hud_ParseItemGroup (hudDef_t *hudDef, char **script, char *buffer, int bufSize)
{
	hudItemGroup_t	*itemGroup;
	char			*tok;

	if ( !hudDef || !script || !buffer )
		return false;

	if (hudDef->numItemGroups == HUD_MAX_ITEMGROUPS) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItemGroup: exceeded HUD_MAX_ITEMGROUPS in hud script %s\n", cl_hudName);
		return false;
	}

	// Allocate itemGroup here
	itemGroup = hudDef->itemGroups[hudDef->numItemGroups++] = Z_Malloc(sizeof(hudItemGroup_t));
//	itemGroup = &hudDef->itemGroups[hudDef->numItemGroups++];

	tok = COM_ParseExt (script, true);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItemGroup: missing parameters for 'itemGroup' in hud script %s\n", cl_hudName);
		return false;
	}

	if ( !Q_stricmp(tok, "{") )
	{
		while ( (*script) < (buffer + bufSize) )
		{
			tok = COM_ParseExt (script, true);
			if (!tok[0]) {
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItemGroup: no concluding '}' in 'itemGroup' in hud script %s\n", cl_hudName);
				return false;
			}
			if ( !Q_stricmp(tok, "}") )
				break;
			if ( !Q_strcasecmp(tok, "name") ) {
				tok = COM_ParseExt (script, true);
				if (!tok[0]) {
					Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItemGroup: missing parameter for 'name' in 'itemGroup' in hud script %s\n", cl_hudName);
					return false;
				}
				Q_strncpyz(itemGroup->name, sizeof(itemGroup->name), tok);
			}
			else if ( !Q_strcasecmp(tok, "onlyMultiPlayer") ) {
				itemGroup->onlyMultiPlayer = true;
			}
			else if ( !Q_strcasecmp(tok, "variant") ) {
				if ( !Hud_ParseItemGroup_Variant(itemGroup, script, buffer, bufSize) )
					return false;				
			}
			else if ( !Q_strcasecmp(tok, "skipIfStat") ) {
				if ( !Hud_ParseItemGroup_SkipIfStat(itemGroup, script, buffer, bufSize) )
					return false;
			}
			else if ( !Q_strcasecmp(tok, "drawItem") || !Q_strcasecmp(tok, "itemDef") ) {
				if ( !Hud_ParseDrawItem(itemGroup, script, buffer, bufSize) )
					return false;
			}
			else {
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItemGroup: unknown function '%s' for 'itemGroup' in hud script %s\n", tok, cl_hudName);
				return false;
			}
		}
	}
	else {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseItemGroup: expected '{', found '%s' instead in 'itemGroup' in hud script %s\n", tok, cl_hudName);
		return false;
	}

	return true;
}


/*
================
Hud_CountRenameItemDef
================
*/
qboolean Hud_CountRenameItemDef (hudRenameSet_t *renameSet, char **script, char *buffer, int bufSize)
{
	char		*tok;

	if ( !renameSet || !script || !buffer )
		return false;

	tok = COM_ParseExt (script, true);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_CountRenameItemDef: missing parameters for 'renameItemDef' in hud script %s\n", cl_hudName);
		return false;
	}

	if ( !Q_stricmp(tok, "{") )
	{
		while ( (*script) < (buffer + bufSize) )
		{
			tok = COM_ParseExt (script, true);
			if (!tok[0]) {
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_CountRenameItemDef: no concluding '}' in 'renameItemDef' in hud script %s\n", cl_hudName);
				return false;
			}
			if ( !Q_stricmp(tok, "}") )
				break;
		}
	}
	else {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_CountRenameItemDef: expected '{', found '%s' instead in 'renameItemDef' in hud script %s\n", tok, cl_hudName);
		return false;
	}

	return true;
}


/*
================
Hud_CountRenameItems
================
*/
void Hud_CountRenameItems (hudRenameSet_t *renameSet, char **script, char *buffer, int bufSize)
{
	char	*p, *tok;

	if ( !renameSet || !script || !buffer )
		return;

	p = *script;
	while ( p < (buffer + bufSize) )
	{
		tok = COM_ParseExt (&p, true);
		if (!tok[0]) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_CountRenameItems: no concluding '}' in 'renameSetDef' in hud script %s\n", cl_hudName);
			return;
		}
		if ( !Q_stricmp(tok, "}") )
			break;
		if ( !Q_strcasecmp(tok, "name") ) {
			tok = COM_ParseExt (&p, false);
			if (!tok[0]) {
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_CountRenameItems: missing parameter for 'name' in 'renameSetDef' in hud script %s\n", cl_hudName);
				return;
			}
		}
		else if ( !Q_strcasecmp(tok, "renameItemDef") ) {
			if ( !Hud_CountRenameItemDef(renameSet, &p, buffer, bufSize) )
				return;
			renameSet->maxRenameItems++;
		}
		else {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_CountRenameItems: unknown command '%s' after item %i while looking for 'renameItemDef' in hud script %s\n", tok, cl_hudDef.numItemGroups, cl_hudName);
			return;
		}
	}
}


/*
================
Hud_ParseRenameItemDef
================
*/
qboolean Hud_ParseRenameItemDef (hudRenameSet_t *renameSet, char **script, char *buffer, int bufSize)
{
	hudRenameItem_t	*renameItem;
	char			*tok;

	if ( !renameSet || !script || !buffer )
		return false;

	if (renameSet->numRenameItems == renameSet->maxRenameItems) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameItemDef: exceeded maxRenameItems in renameSet '%s' in hud script %s\n", renameSet->name, cl_hudName);
		return false;
	}
	renameItem = &renameSet->renameItems[renameSet->numRenameItems++];

	tok = COM_ParseExt (script, true);
	if (!tok[0]) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameItemDef: missing parameters for 'renameItemDef' in hud script %s\n", cl_hudName);
		return false;
	}

	if ( !Q_stricmp(tok, "{") )
	{
		while ( (*script) < (buffer + bufSize) )
		{
			tok = COM_ParseExt (script, true);
			if (!tok[0]) {
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameItemDef: no concluding '}' in 'renameItemDef' in hud script %s\n", cl_hudName);
				return false;
			}
			if ( !Q_stricmp(tok, "}") )
				break;
			if ( !Q_strcasecmp(tok, "oldName") )
			{
				tok = COM_ParseExt (script, false);
				if (!tok[0]) {
					Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameItemDef:missing parameters for 'oldName' in 'renameItemDef' in hud script %s\n", cl_hudName);
					return false;
				}
				Q_strncpyz (renameItem->oldName, sizeof(renameItem->oldName), tok);
				renameItem->oldNameHash = Com_HashFileName(renameItem->oldName, 0, false);
			}
			else if ( !Q_strcasecmp(tok, "newName") )
			{
				tok = COM_ParseExt (script, false);
				if (!tok[0]) {
					Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameItemDef:missing parameters for 'newName' in 'renameItemDef' in hud script %s\n", cl_hudName);
					return false;
				}
				Q_strncpyz (renameItem->newName, sizeof(renameItem->newName), tok);
				renameItem->newNameHash = Com_HashFileName(renameItem->newName, 0, false);
			}
		}
		// If both oldName and newName contain a '*', then it's a wildcard rename
		if ( strstr(renameItem->oldName, "*") && strstr(renameItem->newName, "*") )
			renameItem->isWildcard = true;
	}
	else {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameItemDef: expected '{', found '%s' instead in 'renameItemDef' in hud script %s\n", tok, cl_hudName);
		return false;
	}

	if ( !strlen(renameItem->oldName) ) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameItemDef: required parameter 'oldName' not found in renameItemDef '%i' in renameSet %s in hud script %s\n", renameSet->numRenameItems, renameSet->name, cl_hudName);
		return false;
	}
	if ( !strlen(renameItem->newName) ) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameItemDef: required parameter 'newName' not found in renameItemDef '%i' in renameSet %s in hud script %s\n", renameSet->numRenameItems, renameSet->name, cl_hudName);
		return false;
	}

	return true;
}


/*
================
Hud_ParseRenameSet
================
*/
qboolean Hud_ParseRenameSet (char **script, char *buffer, int bufSize)
{
	hudRenameSet_t	*renameSet;
	char			*tok;

	if ( !script || !buffer )
		return false;

	if (cl_numHudRenames == HUD_MAX_RENAMESETS) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameSet: exceeded HUD_MAX_RENAMESETS in hud script %s\n", cl_hudName);
		return false;
	}
	renameSet = &cl_hudRenames[cl_numHudRenames++];

	tok = COM_ParseExt (script, true);
	if ( Q_stricmp(tok, "{") ) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameSet: expected '{', found %s instead in hud script %s\n", tok, cl_hudName);
		return false;
	}

	// count num of renames and alloc accordingly
	Hud_CountRenameItems (renameSet, script, buffer, bufSize);
//	renameSet->maxRenameItems++;
	renameSet->renameItems = Z_Malloc((renameSet->maxRenameItems+1) * sizeof(hudRenameItem_t));

	while ( (*script) < (buffer + bufSize) )
	{
		tok = COM_ParseExt (script, true);
		if (!tok[0]) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameSet: no concluding '}' in 'renameSetDef' in hud script %s\n", cl_hudName);
			return false;
		}
		if ( !Q_stricmp(tok, "}") )
			break;
		if ( !Q_strcasecmp(tok, "name") ) {
			tok = COM_ParseExt (script, false);
			if (!tok[0]) {
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameSet: missing parameter for 'name' in 'renameSetDef' in hud script %s\n", cl_hudName);
				return false;
			}
			Q_strncpyz (renameSet->name, sizeof(renameSet->name), tok);
		}
		else if ( !Q_strcasecmp(tok, "renameItemDef") )
		{
			if ( !Hud_ParseRenameItemDef(renameSet, script, buffer, bufSize) )
				return false;
		}
		else {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameSet: unknown command '%s' after item %i while looking for 'renameItemDef' in hud script %s\n", tok, renameSet->numRenameItems, cl_hudName);
			return false;
		}
	}
	if (!strlen(renameSet->name)) {
		Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseRenameSet: renameSet without a name in hud script %s\n", cl_hudName);
		return false;
	}
//	Com_Printf ("Hud_ParseRenameSet: successfully loaded renameSet %s with %i renameItems in hud script %s\n", renameSet->name, renameSet->numRenameItems, cl_hudName);
	return true;
}


/*
================
Hud_ParseHud
================
*/
qboolean Hud_ParseHud (char *buffer, int bufSize)
{
	char	*p, *tok;

	if ( !buffer )
		return false;

	p = buffer;
	while (p < (buffer + bufSize))
	{
		tok = COM_ParseExt (&p, true);
		if (!tok[0])
			break;

		if ( !Q_strcasecmp(tok, "hudDef") )
		{
			tok = COM_ParseExt (&p, true);
			if ( Q_stricmp(tok, "{") ) {
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseHud: expected '{', found %s instead in hud script %s\n", tok, cl_hudName);
				return false;
			}

			while (p < (buffer + bufSize))
			{
				tok = COM_ParseExt (&p, true);
				if (!tok[0]) {
					Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseHud: no concluding '}' in 'hudDef' in hud script %s\n", cl_hudName);
					return false;
				}
				if ( !Q_stricmp(tok, "}") )
					break;
				if ( !Q_strcasecmp(tok, "overridePath") ) {
					tok = COM_ParseExt (&p, false);
					if (!tok[0]) {
						Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseHud: missing parameter for 'overridePath' in 'hudDef' in hud script %s\n", cl_hudName);
						return false;
					}
					Q_strncpyz (cl_hudDef.overridePath, sizeof(cl_hudDef.overridePath), tok);
				}
				else if ( !Q_strcasecmp(tok, "itemGroup") ) {
					if ( !Hud_ParseItemGroup(&cl_hudDef, &p, buffer, bufSize) )
						return false;
				}
				else {
					Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseHud: unknown command '%s' after item %i while looking for 'itemGroup' in hud script %s\n", tok, cl_hudDef.numItemGroups, cl_hudName);
					return false;
				}
			}
		}
		else if ( !Q_strcasecmp(tok, "renameSetDef") )
		{
			if ( !Hud_ParseRenameSet(&p, buffer, bufSize) )
				return false;
		}
		// ignore any crap after the hudDef
		else if (cl_hudDef.numItemGroups == 0) {
			Com_Printf (S_COLOR_YELLOW"WARNING: Hud_ParseHud: unknown command '%s' while looking for 'hudDef' in hud script %s\n", tok, cl_hudName);
			return false;
		}
	}
	Com_sprintf (cl_hudDef.name, sizeof(cl_hudDef.name), cl_hudName);
	if ( !strlen(cl_hudDef.overridePath) )
		Q_strncpyz (cl_hudDef.overridePath, sizeof(cl_hudDef.overridePath), cl_hudDef.name);
	return true;
}


/*
================
Hud_ClearHudData
================
*/
void Hud_ClearHudData (void)
{
	int		i, j;

	for (i=0; i < HUD_MAX_RENAMESETS; i++)
	{
		if ( cl_hudRenames[i].numRenameItems > 0 && cl_hudRenames[i].renameItems )
			Z_Free (cl_hudRenames[i].renameItems);

		// Free rename wildcard cache
		if (cl_hudRenames[i].renameCacheHead != NULL)
		{
			hudRenameCacheItem_t	*cacheItem, *prevItem = NULL;
			for (cacheItem = cl_hudRenames[i].renameCacheHead; cacheItem; cacheItem = cacheItem->next) {
				if (prevItem) {
					Z_Free (prevItem);
				}
				prevItem = cacheItem;
			}
			if (prevItem) {
				Z_Free (prevItem);
			}
			cl_hudRenames[i].renameCacheHead = cl_hudRenames[i].renameCacheTail = NULL;
		}
	}
	memset (&cl_hudRenames, 0, sizeof(cl_hudRenames));
	cl_numHudRenames = 0;

	// Clear out allocated HUD items and groups
	for (i=0; i < cl_hudDef.numItemGroups; i++)
	{
		if (cl_hudDef.itemGroups[i] != NULL)
		{
			for (j=0; j < cl_hudDef.itemGroups[i]->numDrawItems; j++) {
				if (cl_hudDef.itemGroups[i]->drawItems[j] != NULL)
					Z_Free (cl_hudDef.itemGroups[i]->drawItems[j]);
			}
			Z_Free (cl_hudDef.itemGroups[i]);
		}
	}
	memset (&cl_hudDef, 0, sizeof(cl_hudDef));
}


/*
================
CL_LoadHud
================
*/
void CL_LoadHud (qboolean startup)
{
	char	fileName[MAX_QPATH];
	char	*hudName, *buffer;
	int		bufSize;

	if (startup)
		Hud_ClearHudData ();

	hudName = Cvar_VariableString("cl_hud");

	// default falls back on game-specified hud
	if ( !Q_strcasecmp(hudName, Cvar_DefaultString("cl_hud")) ) {
		Com_Printf ("CL_LoadHud: HUD set to default\n");
		return;
	}

	// catch 0-length hud name
	if ( !strlen(hudName) ) {
		Cvar_ForceSetToDefault ("cl_hud");
		return;
	}
	
	// catch same hud currently loaded
	if ( !Q_strcasecmp(hudName, cl_hudDef.name) ) {
		Com_Printf ("CL_LoadHud: HUD %s already loaded\n", cl_hudDef.name);
		return;
	}

	// Load the file
	Com_sprintf (fileName, sizeof(fileName), "scripts/huds/%s.hud", hudName);
	bufSize = FS_LoadFile (fileName, (void **)&buffer);
	if ( !buffer )
	{	// failed to load, keep scripted hud if already loaded
		Com_Printf ("CL_LoadHud: couldn't load %s\n", fileName);
	//	if (cl_hudDef.numItemGroups > 0)
	//		Cvar_Set("cl_hud", cl_hudDef.name);
	//	else
		Cvar_ForceSetToDefault ("cl_hud");
		return;
	}
	if (bufSize < 1) {
		// 0 size file, keep bindlist if already loaded
		Com_Printf ("CL_LoadHud: %s has 0 size\n", fileName);
		Cvar_ForceSetToDefault ("cl_hud");
		FS_FreeFile (buffer);
		return;
	}

	// Parse it
	cl_hudName = Cvar_VariableString("cl_hud");
	if (!startup) {
		Hud_ClearHudData ();
	}
	if ( Hud_ParseHud (buffer, bufSize) ) {
		Com_Printf ("CL_LoadHud: loaded hud %s with %i item groups\n", cl_hudDef.name, cl_hudDef.numItemGroups);
	}
	else {
		Hud_ClearHudData ();
		Cvar_ForceSetToDefault ("cl_hud");
	}

	// Make sure cl_hudVariant is set
	CL_SetHudVariant ();

	// free the file
	FS_FreeFile (buffer);
}


/*
================
CL_FreeHud
================
*/
void CL_FreeHud (void)
{
	Hud_ClearHudData ();
}


/*
================
CL_SetHud
================
*/
void CL_SetHud (const char *value)
{
	if ( !value || (strlen(value) == 0) )
		return;
	
	Cvar_ForceSet ("cl_hud", (char *)value);
	CL_LoadHud (false);
}


/*
================
CL_SetDefaultHud
================
*/
void CL_SetDefaultHud (void)
{	
	Cvar_ForceSetToDefault ("cl_hud");
	CL_LoadHud (false);
}


/*
================
CL_SetHudVariant
================
*/
void CL_SetHudVariant (void)
{
	int		cvar_len, cs_len;
	char	*cvarString;

	cvarString = Cvar_VariableString("cl_hud_variant");
	if (!cvarString)	// this should never happen
		return;

	// check cl_hud_variant cvar first, then fall back to CS_HUDVARIANT
	cvar_len = (int)strlen(cvarString);
	cs_len = (int)strlen(cl.configstrings[CS_HUDVARIANT]);
	if ( (cvar_len > 0) && (Q_stricmp(cvarString, "default") != 0) && (Q_stricmp(cvarString, "*") != 0) )
	{
		cl_hudVariant = cvarString;
		// skip leading *, allows forcing "default" if CS_HUDVARIANT contains a string other than "default"
		if ( (cvar_len > 1) && (cl_hudVariant[0] == '*') ) {
			cl_hudVariant++;
		}
	}
	else if (cs_len > 0) {
		cl_hudVariant = cl.configstrings[CS_HUDVARIANT];
	}
	else {
		cl_hudVariant = "default";
	}
}


/*
================
Hud_ScriptIsLoaded
================
*/
qboolean Hud_ScriptIsLoaded (void)
{
	return ( (Q_strcasecmp(Cvar_VariableString("cl_hud"), Cvar_DefaultString("cl_hud")) != 0)
		&& (cl_hudDef.numItemGroups > 0) );
}

/*
===============================================================

SCRIPTED HUD DRAWING

===============================================================
*/

/*
================
Hud_CheckVariant
================
*/
qboolean Hud_CheckVariant (hudVariant_t *variant)
{
	// check if variant is NULL, or cl_hudVariant is unset or 0 length
	if ( !variant || !cl_hudVariant || (cl_hudVariant[0] == '\0') )
		return false;

	// return false if variant name doesn't match
	if ( Q_strcasecmp(variant->name, cl_hudVariant) != 0 )
		return false;

	return true;
}


/*
================
Hud_CheckSkipIfStat
================
*/
qboolean Hud_CheckSkipIfStat (hudSkipIfStat_t *skipIfStat)
{
	int			i, increment;
	qboolean	skipped = false;

	increment = 0;
	for (i = 0; i < skipIfStat->numCmps; i++)
	{
		switch (skipIfStat->func[i])
		{
		case HUD_GREATER:
			skipped = (cl.frame.playerstate.stats[skipIfStat->stat[i]] > skipIfStat->value[i]);
			break;
		case HUD_LESS:
			skipped = (cl.frame.playerstate.stats[skipIfStat->stat[i]] < skipIfStat->value[i]);
			break;
		case HUD_GEQUAL:
			skipped = (cl.frame.playerstate.stats[skipIfStat->stat[i]] >= skipIfStat->value[i]);
			break;
		case HUD_LEQUAL:
			skipped = (cl.frame.playerstate.stats[skipIfStat->stat[i]] <= skipIfStat->value[i]);
			break;
		case HUD_EQUAL:
			skipped = (cl.frame.playerstate.stats[skipIfStat->stat[i]] == skipIfStat->value[i]);
			break;
		case HUD_NOTEQUAL:
			skipped = (cl.frame.playerstate.stats[skipIfStat->stat[i]] != skipIfStat->value[i]);
			break;
		case HUD_AND:
			skipped = (cl.frame.playerstate.stats[skipIfStat->stat[i]] & skipIfStat->value[i]);
			break;
		case HUD_NOTAND:
			skipped = !(cl.frame.playerstate.stats[skipIfStat->stat[i]] & skipIfStat->value[i]);
			break;
		}
		if (skipped)	// condition met
			increment++;
	}
	if (increment == skipIfStat->numCmps)	// all conditions met
		return true;
	else
		return false;
}


/*
================
Hud_GetRenamedPic
================
*/
char *Hud_GetRenamedPic (char *renameSetName, char *picName)
{
	int				i;
	unsigned int	picNameHash;
	static char		renameBuf[MAX_QPATH];
	char			oldPrefix[MAX_QPATH], newPrefix[MAX_QPATH];
	char			*oldName, *newName, *newPic = NULL;
	qboolean		wc_renamed;
	hudRenameSet_t	*foundSet = NULL;
	hudRenameCacheItem_t	*cacheCheck = NULL, *addCacheItem = NULL;

	if (!renameSetName || !picName)
		return picName;
	if ( !strlen(renameSetName) || !strlen(picName) )
		return picName;

	for (i=0; i < cl_numHudRenames; i++) {
		if ( !Q_strcasecmp(renameSetName, cl_hudRenames[i].name) ) {
			foundSet = &cl_hudRenames[i];
			break;
		}
	}
	if (foundSet == NULL) {
		Com_Printf ("Hud_GetRenamedPic: Couldn't find renameSet %s\n", renameSetName);
		return picName;
	}
	if (foundSet->numRenameItems == 0) {
		Com_Printf ("Hud_GetRenamedPic: renameSet %s has no items\n", renameSetName);
		return picName;
	}

	picNameHash = Com_HashFileName(picName, 0, false);

	// Check rename wildcard cache 
	for (cacheCheck = foundSet->renameCacheHead; cacheCheck; cacheCheck = cacheCheck->next)
	{
		oldName = cacheCheck->oldName;
		newName = cacheCheck->newName;
		if (picNameHash == cacheCheck->oldNameHash) {
			if ( !Q_strcasecmp(oldName, picName) ) {
				newPic = newName;
			//	Com_Printf ("Hud_GetRenamedPic: Renamed via wildcard cache %s -> %s\n", picName, newPic);
				break;
			}
		}
	}
	if (newPic)	return newPic;

	// Check non-wildcard renames
	for (i=0; i < foundSet->numRenameItems; i++)
	{
		if (foundSet->renameItems[i].isWildcard)
			continue;
		oldName = foundSet->renameItems[i].oldName;
		newName = foundSet->renameItems[i].newName;
		if (picNameHash == foundSet->renameItems[i].oldNameHash) {
			if ( !Q_strcasecmp(oldName, picName) ) {
				newPic = newName;
				break;
			}
		}
	}
	if (newPic)	return newPic;

	// Check wildcard renames
	for (i=0; i < foundSet->numRenameItems; i++)
	{
		if ( !foundSet->renameItems[i].isWildcard )
			continue;
		oldName = foundSet->renameItems[i].oldName;
		newName = foundSet->renameItems[i].newName;
		if ( wildcardfit(oldName, picName) )
		{
			char	*p;
			wc_renamed = false;
			Q_strncpyz (oldPrefix, sizeof(oldPrefix), oldName);
			Q_strncpyz (newPrefix, sizeof(newPrefix), newName);
			p = strstr(oldPrefix, "*");
			if (p)	*p = '\0';
			p = strstr(newPrefix, "*");
			if (p)	*p = '\0';
			if ( (strlen(oldPrefix) > 0) && (strlen(newPrefix) > 0) )
			{
			//	Com_Printf ("Hud_GetRenamedPic: Wildcard match: %s in %s to %s (%s -> %s)\n", oldName, picName, newName, oldPrefix, newPrefix);
				Com_sprintf(renameBuf, sizeof(renameBuf), "%s%s", newPrefix, picName + strlen(oldPrefix));
				newPic = renameBuf;
				wc_renamed = true;

				// Add to rename wildcard cache
				addCacheItem = Z_Malloc(sizeof(hudRenameCacheItem_t));
				Q_strncpyz (addCacheItem->oldName, sizeof(addCacheItem->oldName), picName);
				Q_strncpyz (addCacheItem->newName, sizeof(addCacheItem->newName), newPic);
				addCacheItem->oldNameHash = Com_HashFileName(addCacheItem->oldName, 0, false);
				addCacheItem->newNameHash = Com_HashFileName(addCacheItem->newName, 0, false);
				addCacheItem->next = foundSet->renameCacheHead;
				foundSet->renameCacheHead = addCacheItem;
				if (foundSet->renameCacheTail == NULL) {
					foundSet->renameCacheTail = addCacheItem;
				}
			//	Com_Printf ("Hud_GetRenamedPic: Saved to wildcard cache %s -> %s\n", picName, newPic);
			}
			if (wc_renamed)
				break;
		}
	}
	if (newPic)	return newPic;
	else		return picName;
}


#define FlipTexCoord(a)		(a = (a + 2*(0.5f-a)) )
/*
================
Hud_DrawItem
================
*/
void Hud_DrawItem (hudDrawItem_t *drawItem)
{
	hudSkipIfStat_t	*skipIfStat = NULL;
	byte			*color = NULL;
	char			*orgPic = NULL, *pic = NULL, *string = NULL;
	char			num[16], *ptr = NULL;
	char			shader[MAX_QPATH], shaderMinus[MAX_QPATH];
	int				index, max_configstrings, max_images, cs_images;
	int				i, number = 0, value = 0, width = 0, l = 0, frame = 0;
	int				minTemp = 0, maxTemp = 0;
	float			refVal = 0.0f;
	vec4_t			texCoord;
	vec2_t			offset;

	// Knightmare- hack for connected to server using old protocol
	// Changed config strings require different parsing
	if ( LegacyProtocol() ) {
		cs_images = OLD_CS_IMAGES;
		max_images = OLD_MAX_IMAGES;
		max_configstrings = OLD_MAX_CONFIGSTRINGS;
	}
	else {
		cs_images = CS_IMAGES;
		max_images = MAX_IMAGES;
		max_configstrings = MAX_CONFIGSTRINGS;
	}
	
	// check if item is blocked by a skipIfStat
	if (drawItem->numSkipIfStats > 0)
	{
		for (i = 0, skipIfStat = drawItem->skipIfStats; i < drawItem->numSkipIfStats; i++, skipIfStat++) {
			if ( Hud_CheckSkipIfStat(skipIfStat) )
				return;
		}
	}

	// check for skew, overrides offset
	if ((drawItem->skew[0] != 0) || (drawItem->skew[1] != 0))
	{
		// skew is relative to rect size
		for (i=0; i<2; i++)
			offset[i] = drawItem->skew[i] * (float)drawItem->rect[3-i];
	}
	else
		Vector2Copy (drawItem->offset, offset);
		
	if ((cl.frame.serverframe>>2) & 1)
		color = drawItem->blinkColor;
	else
		color = drawItem->color;

	// check for colorRanges
	if (drawItem->numColorRanges > 0)
	{
		refVal = cl.frame.playerstate.stats[drawItem->colorRangeStat];
		if (drawItem->colorRangeType == HUD_RANGETYPE_RELATIVE)
			refVal /= cl.frame.playerstate.stats[drawItem->colorRangeMaxStat];
		for (i=0; i<drawItem->numColorRanges; i++)
		{
			if (refVal >= drawItem->colorRanges[i].low && refVal <= drawItem->colorRanges[i].high)
			{
				if ((cl.frame.serverframe>>2) & 1)
					color = drawItem->colorRanges[i].blinkColor;
				else
					color = drawItem->colorRanges[i].color;
				break;
			}
		}
	}
	// check for stat flash color
	if (drawItem->statFlash.stat > 0 && drawItem->statFlash.bit > 0) {
		if (cl.frame.playerstate.stats[drawItem->statFlash.stat] & (drawItem->statFlash.bit))
			color = drawItem->statFlash.color;
	}

	if ( drawItem->fromStat & (HUD_STATSHADER|HUD_WEAPONMODELSHADER) )
	{
		char	picName[MAX_QPATH];

		if (drawItem->fromStat & HUD_STATSHADER)
		{
			index = cl.frame.playerstate.stats[drawItem->shaderStat];
			if (index < 0 || index >= max_images)
			{
				Com_Printf (S_COLOR_YELLOW"WARNING: Hud_DrawItem: bad pic index %i\n", index);
				index = max_images-1;
			}
			orgPic = cl.configstrings[cs_images+index];
		}
		else // HUD_WEAPONMODELSHADER
		{
			if (!ui_currentweaponmodel) // nothing to draw
				return;
			COM_StripExtension (ui_currentweaponmodel, picName, sizeof(picName));
			orgPic = picName;
		}

		if ( strlen(orgPic) == 0 )	return;

		if (drawItem->flags & DSF_RENAMESHADER)
			pic = Hud_GetRenamedPic (drawItem->shaderRenameSet, orgPic);
		else
			pic = orgPic;

		if (!pic)	return;

		// renamed pics implicitly use the override path
		if ( (drawItem->flags & DSF_OVERRIDEPATH) || (pic != orgPic) )
			Com_sprintf (shader, sizeof(shader), "/gfx/huds/%s/%s.pcx", cl_hudDef.overridePath, pic);
		else
			Com_sprintf (shader, sizeof(shader), "%s", pic);
	}
	else
		Com_sprintf (shader, sizeof(shader), "/%s.pcx", drawItem->shader);

	if (drawItem->type == HUD_DRAWPIC)
	{	// no modding of tex coords for existing HUD pics (.pcx is FUBARed)
		if ( (drawItem->fromStat & (HUD_STATSHADER|HUD_WEAPONMODELSHADER))
			&& !((drawItem->flags & DSF_OVERRIDEPATH) || (pic != orgPic)) )
		{
			SCR_DrawOffsetPic (drawItem->rect[0], drawItem->rect[1], drawItem->rect[2], drawItem->rect[3],
								offset, drawItem->scrnAlignment, false, color, shader);
		}
		else
		{
			float	padW, padH;
		//	float	sl, tl, sh, th;
			texCoord[0] = texCoord[1] = 0.0f;
			texCoord[2] = texCoord[3] = 1.0f;
		//	sl = tl = 0.0f;
		//	sh = th = 1.0f;

			if (fabs(drawItem->padding) > 0.0f)
			{
				padW = drawItem->padding / (float)drawItem->rect[2];
				padH = drawItem->padding / (float)drawItem->rect[3];
				texCoord[0] += padW;	// sl
				texCoord[2] -= padW;	// sh
				texCoord[1] += padH;	// tl
				texCoord[3] -= padH;	// th
			}
			if (drawItem->flags & DSF_FLIPX)
			{	FlipTexCoord (texCoord[0]);	FlipTexCoord (texCoord[2]);	}
			if (drawItem->flags & DSF_FLIPY)
			{	FlipTexCoord (texCoord[1]);	FlipTexCoord (texCoord[3]);	}

			if ( (drawItem->flags & DSF_MASKSHADER) && strlen(drawItem->shaderMinus) > 0 ) {
				Com_sprintf (shaderMinus, sizeof(shaderMinus), "/%s.pcx", drawItem->shaderMinus);
			//	SCR_DrawMaskedPic (drawItem->rect[0], drawItem->rect[1], drawItem->rect[2], drawItem->rect[3],
			//						offset, texCoord, drawItem->scroll, drawItem->scrnAlignment, false, color, shader, shaderMinus);
				SCR_DrawPicFull (drawItem->rect[0], drawItem->rect[1], drawItem->rect[2], drawItem->rect[3],
									offset, texCoord, drawItem->scroll, drawItem->scrnAlignment, false, color, (drawItem->flags & DSF_ADDITIVESHADER), shader, shaderMinus);
			}
			else
			//	SCR_DrawScrollPic (drawItem->rect[0], drawItem->rect[1], drawItem->rect[2], drawItem->rect[3],
			//						offset, texCoord, drawItem->scroll, drawItem->scrnAlignment, false, color, shader);
				SCR_DrawPicFull (drawItem->rect[0], drawItem->rect[1], drawItem->rect[2], drawItem->rect[3],
									offset, texCoord, drawItem->scroll, drawItem->scrnAlignment, false, color, (drawItem->flags & DSF_ADDITIVESHADER), shader, NULL);

		//	SCR_DrawOffsetPicST (drawItem->rect[0], drawItem->rect[1], drawItem->rect[2], drawItem->rect[3],
		//							offset, texCoord, drawItem->scrnAlignment, false, color, shader);
		}
	}
	else if (drawItem->type == HUD_DRAWSTRING)
	{
	//	float		x, y;

		if (drawItem->fromStat & HUD_STATSTRING)
		{
			index = cl.frame.playerstate.stats[drawItem->stringStat];
			if (index < 0 || index >= max_configstrings)
				Com_Error (ERR_DROP, "Hud_DrawItem: bad string index %i\n", index);
			string = cl.configstrings[index];
		}
		else if (drawItem->fromStat & HUD_PLAYERNAMESTRING)
		{	// grab from player name
			string = Cvar_VariableString("name");
		}
		else
			string = drawItem->string;

		Hud_DrawStringFromCharsPic (drawItem->rect[0], drawItem->rect[1], drawItem->size[0], drawItem->size[1], offset, 
									drawItem->rect[2], drawItem->scrnAlignment, string, color, shader, drawItem->flags); 
	/*
		x = drawItem->rect[0];	y = drawItem->rect[1];
		SCR_ScaleCoords (&x, &y, NULL, NULL, drawItem->scrnAlignment);
		CL_DrawStringGeneric (x, y, string, FONT_SCREEN, drawItem->color[4], drawItem->size[0], SCALETYPE_MENU, false);
	*/
	}
	else if (drawItem->type == HUD_DRAWNUMBER)
	{
		int		x, y;
		float	xOffset;

		if (drawItem->fromStat & HUD_STATNUMBER)
			number = cl.frame.playerstate.stats[drawItem->numberStat];
		else
			number = drawItem->number;

		if (drawItem->size[0])
			width = drawItem->rect[2] / drawItem->size[0];
		else
			width = 3;

		if (width < 1)
			return;
		width = min(width, 5);

		switch (width)
		{
		case 1: 
			number = min(max(number, 0), 9);
			break;
		case 2: 
			number = min(max(number, -9), 99);
			break;
		case 3: 
			number = min(max(number, -99), 999);
			break;
		case 4: 
			number = min(max(number, -999), 9999);
			break;
		case 5: 
			number = min(max(number, -9999), 99999);
			break;
		}

		Q_snprintfz (num, sizeof(num), "%i", number);
		l = (int)strlen(num);
		l = min(l, width);

		Com_sprintf (shaderMinus, sizeof(shaderMinus), "/%s.pcx", drawItem->shaderMinus);
		if (drawItem->flags & DSF_LEFT)
			x = drawItem->rect[0];
		else if  (drawItem->flags & DSF_CENTER)
			x = drawItem->rect[0] + ((width - l) * 0.5 * drawItem->size[0]);
		else // DSF_RIGHT
			x = drawItem->rect[0] + ((width - l) * drawItem->size[0]);
		y = drawItem->rect[1] + ((drawItem->rect[3] - drawItem->size[1]) / 2);

		ptr = num;
		while (*ptr && l)
		{
			if (*ptr == '-') {
			//	SCR_DrawOffsetPic (x, y, drawItem->size[0], drawItem->size[1], offset,
			//						drawItem->scrnAlignment, false, color, shaderMinus);
				SCR_DrawPicFull (x, y, drawItem->size[0], drawItem->size[1], offset,
									stCoord_identity, vec2_origin, drawItem->scrnAlignment, false, color, (drawItem->flags & DSF_ADDITIVESHADER), shaderMinus, NULL);
			}
			else {
				frame = *ptr - '0';
				xOffset = (float)frame * 0.1f;
				Vector4Set (texCoord, 0.0f+xOffset, 0.0f, 0.1f+xOffset, 1.0f);
			//	SCR_DrawOffsetPicST (x, y, drawItem->size[0], drawItem->size[1], offset,
			//						texCoord, drawItem->scrnAlignment, false, color, shader);
				SCR_DrawPicFull (x, y, drawItem->size[0], drawItem->size[1], offset,
									texCoord, vec2_origin, drawItem->scrnAlignment, false, color, (drawItem->flags & DSF_ADDITIVESHADER), shader, NULL);
			}
			x += drawItem->size[0];
			ptr++;
			l--;
		}
	}
	else if (drawItem->type == HUD_DRAWBAR)
	{
		float	x = 0, y = 0, w = 0, h = 0;	//, sl, tl, sh, th;
		float	totalSize = 0.0f, rangeSize = 0.0f, percent = 0.0f, adjustedPercent = 0.0f;
		int		minVal = 0, maxVal = 0;

		if (drawItem->fromStat & HUD_STATVALUE)
			value = cl.frame.playerstate.stats[drawItem->valueStat];
		else
			value = drawItem->value;

		if (drawItem->fromStat & HUD_STATMINRANGE)
		{
			if (drawItem->fromStat & HUD_STATMINRANGECLAMP)
				minTemp = min(max(cl.frame.playerstate.stats[drawItem->minRangeStat], drawItem->minRangeStatClamp[0]), drawItem->minRangeStatClamp[1]);
			else
				minTemp = cl.frame.playerstate.stats[drawItem->minRangeStat];
			minVal = (cl.frame.playerstate.stats[drawItem->minRangeStat]) ? (int)((float)minTemp * drawItem->minRangeStatMult) : drawItem->range[0];
		}
		else
			minVal = drawItem->range[0];

		if (drawItem->fromStat & HUD_STATMAXRANGE)
		{
			if (drawItem->fromStat & HUD_STATMAXRANGECLAMP)
				maxTemp = min(max(cl.frame.playerstate.stats[drawItem->maxRangeStat], drawItem->maxRangeStatClamp[0]), drawItem->maxRangeStatClamp[1]);
			else
				maxTemp = cl.frame.playerstate.stats[drawItem->maxRangeStat];
			maxVal = (cl.frame.playerstate.stats[drawItem->maxRangeStat]) ? (int)((float)maxTemp * drawItem->maxRangeStatMult) : drawItem->range[1];
		}
		else
			maxVal = drawItem->range[1];

		value = min(max(value, minVal), maxVal) - minVal;
		if ( (maxVal - minVal) > 0)
		{
			percent = (float)value / (float)(maxVal - minVal);
			if (percent <= 0.0f)
				return;

			percent = max(min(percent, 1.0f), 0.0f);

			switch (drawItem->fillDir)
			{
			case HUD_LEFTTORIGHT:
			case HUD_RIGHTTOLEFT:
				totalSize = drawItem->rect[2];
				break;
			case HUD_BOTTOMTOTOP:
			case HUD_TOPTOBOTTOM:
				totalSize = drawItem->rect[3];
				break;
			}
			rangeSize = totalSize - (drawItem->padding*2);
			adjustedPercent = (drawItem->padding + rangeSize*percent) / totalSize;
			adjustedPercent = max(min(adjustedPercent, 1.0f), 0.0f);
		//	if (drawItem->padding > 0)
		//		Com_Printf ("Hud_DrawItem: bar has padding of %6.3f, changed percent from %5.3f to %5.3f \n",
		//					drawItem->padding, percent, adjustedPercent);
		}
		else
			return;

		switch (drawItem->fillDir)
		{
		case HUD_LEFTTORIGHT:
			x = drawItem->rect[0];
			y = drawItem->rect[1];
			w = drawItem->rect[2] * adjustedPercent;
			h = drawItem->rect[3];
			texCoord[0] = 0.0f;
			texCoord[1] = 0.0f;
			texCoord[2] = adjustedPercent;
			texCoord[3] = 1.0f;
			break;
		case HUD_RIGHTTOLEFT:
			x = drawItem->rect[0] + (drawItem->rect[2] * (1.0f - adjustedPercent));
			y = drawItem->rect[1];
			w = drawItem->rect[2] * adjustedPercent;
			h = drawItem->rect[3];
			texCoord[0] = 1.0f - adjustedPercent;
			texCoord[1] = 0.0f;
			texCoord[2] = 1.0f;
			texCoord[3] = 1.0f;
			break;
		case HUD_BOTTOMTOTOP:
			x = drawItem->rect[0];
			y = drawItem->rect[1] + (drawItem->rect[3] * (1.0f - adjustedPercent));
			w = drawItem->rect[2];
			h = drawItem->rect[3] * adjustedPercent;
			texCoord[0] = 0.0f;
			texCoord[1] = 1.0f - adjustedPercent;
			texCoord[2] = 1.0f;
			texCoord[3] = 1.0f;
			break;
		case HUD_TOPTOBOTTOM:
			x = drawItem->rect[0];
			y = drawItem->rect[1];
			w = drawItem->rect[2];
			h = drawItem->rect[3] * adjustedPercent;
			texCoord[0] = 0.0f;
			texCoord[1] = 0.0f;
			texCoord[2] = 1.0f;
			texCoord[3] = adjustedPercent;
			break;
		}
		if (drawItem->flags & DSF_FLIPX)
		{	FlipTexCoord (texCoord[0]);	FlipTexCoord (texCoord[2]); }
		if (drawItem->flags & DSF_FLIPY)
		{	FlipTexCoord (texCoord[1]);	FlipTexCoord (texCoord[3]); }

	//	SCR_DrawScrollPic (x, y, w, h, offset, texCoord, drawItem->scroll, drawItem->scrnAlignment, false, color, shader);
		SCR_DrawPicFull (x, y, w, h, offset, texCoord, drawItem->scroll, drawItem->scrnAlignment, false, color, (drawItem->flags & DSF_ADDITIVESHADER), shader, NULL);
	//	SCR_DrawOffsetPicST (x, y, w, h, offset, texCoord, drawItem->scrnAlignment, false, color, shader);
	}
}


/*
================
Hud_DrawHud
================
*/
void Hud_DrawHud (void)
{
	hudItemGroup_t	*itemGroup;
	hudVariant_t	*variant;
	hudSkipIfStat_t	*skipIfStat;
	hudDrawItem_t	*drawItem;
	int				i, j;
	qboolean		variantMatch = false;

//	for (i = 0, itemGroup = cl_hudDef.itemGroups; i < cl_hudDef.numItemGroups; i++, itemGroup++)
	for (i = 0; i < cl_hudDef.numItemGroups; i++)
	{
		itemGroup = cl_hudDef.itemGroups[i];

		if (itemGroup->onlyMultiPlayer) {
			if ( !cl.configstrings[CS_MAXCLIENTS][0] || !strcmp(cl.configstrings[CS_MAXCLIENTS], "1") )
				continue;
		}

		// check if itemGroup is enabled by a variant
		if (itemGroup->numVariants > 0)
		{
			variantMatch = false;
			for (j = 0, variant = itemGroup->variants; j < itemGroup->numVariants; j++, variant++)
			{
				if ( Hud_CheckVariant(variant) )
					variantMatch = true;
			}
			if ( !variantMatch )
				continue;
		}

		// check if itemGroup is blocked by a skipIfStat
		if (itemGroup->numSkipIfStats > 0)
		{
			for (j = 0, skipIfStat = itemGroup->skipIfStats; j < itemGroup->numSkipIfStats; j++, skipIfStat++)
			{
				if ( Hud_CheckSkipIfStat(skipIfStat) )
					break;
			}
			if (j != itemGroup->numSkipIfStats)
				continue;
		}

	//	for (j = 0, drawItem = itemGroup->drawItems; j < itemGroup->numDrawItems; j++, drawItem++) {
		for (j = 0; j < itemGroup->numDrawItems; j++) {
			drawItem = itemGroup->drawItems[j];
			Hud_DrawItem (drawItem);
		}
	}
}
