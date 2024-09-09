/*
===========================================================================
Copyright (C) 2024 Knightmare

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

// json_parse.c

#include "qcommon.h"
#include "../libraries/jsmn/jsmn.h"

/*
============================================================================

GENERAL JSON PARSING

============================================================================
*/

static void jsonprintf (qboolean verbose, char *format, ...)
{
	va_list		argptr;
	char		buf[1024];

	if ( !verbose )
		return;

	va_start (argptr, format);
	Q_vsnprintf (buf, sizeof(buf), format, argptr);
	va_end (argptr);

	Com_DPrintf ("%s", buf);
}


static int jsoneq (const char *json, jsmntok_t *tok, const char *s)
{
	if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
		strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
			return 0;
	}
	return -1;
}

/*
============================================================================

WAL_JSON LOADING

============================================================================
*/

typedef struct bitflag_s
{
	const char		*name;
	unsigned int	bit;
} bitflag_t;

bitflag_t Q2_surf_flags[] =
{
	{ "LIGHT",		0x1			},
	{ "SLICK",		0x2			},
	{ "SKY",		0x4			},
	{ "WARP",		0x8			},
	{ "TRANS33",	0x10		},
	{ "TRANS66",	0x20		},
	{ "FLOWING",	0x40		},
	{ "NODRAW",		0x80		},
	{ "HINT",		0x100		},
	{ "SKIP",		0x200		},
	{ "METAL",		0x400		},
	{ "DIRT",		0x800		},
	{ "VENT",		0x1000		},
	{ "GRATE",		0x2000		},
	{ "TILE",		0x4000		},
	{ "GRASS",		0x8000		},
	{ "SNOW",		0x10000		},
	{ "CARPET",		0x20000		},
	{ "FORCE",		0x40000		},
	{ "GRAVEL",		0x80000		},
	{ "ICE",		0x100000	},
	{ "SAND",		0x200000	},
	{ "WOOD",		0x400000	},
	{ "STANDARD",	0x800000	},
	{ "FULLBRIGHT",	0x1000000	},
	{ "ALPHATEST",	0x2000000	},
	{ "FOGPLANE",	0x4000000	}
};

#define NUM_Q2_SURF_FLAGS (sizeof(Q2_surf_flags) / sizeof(Q2_surf_flags[0]))

bitflag_t Q2_content_flags[] =
{
	{ "SOLID",			0x1			},
	{ "WINDOW",			0x2			},
	{ "AUX",			0x4			},
	{ "LAVA",			0x8			},
	{ "SLIME",			0x10		},
	{ "WATER",			0x20		},
	{ "MIST",			0x40		},
	{ "CLEAR",			0x80		},
	{ "NOTSOLID",		0x100		},
	{ "NOSHOOT",		0x200		},
	{ "FOG",			0x400		},
	{ "AREAPORTAL",		0x8000		},
	{ "PLAYERCLIP",		0x10000		},
	{ "MONSTERCLIP",	0x20000		},
	{ "CORRENT_0",		0x40000		},
	{ "CORRENT_90",		0x80000		},
	{ "CORRENT_180",	0x100000	},
	{ "CORRENT_270",	0x200000	},
	{ "CORRENT_UP",		0x400000	},
	{ "CORRENT_DOWN",	0x800000	},
	{ "ORIGIN",			0x1000000	},
	{ "MONSTER",		0x2000000	},
	{ "DEADMONSTER",	0x4000000	},
	{ "DETAIL",			0x8000000	},
	{ "TRANSLUCENT",	0x10000000	},
	{ "LADDER",			0x20000000	}
};

#define NUM_Q2_CONTENT_FLAGS (sizeof(Q2_content_flags) / sizeof(Q2_content_flags[0]))

#if 0
static unsigned int surfflags_from_string (const char *flagName)
{
	int				i;
	unsigned int	retVal = 0;

	for (i = 0; i < NUM_Q2_SURF_FLAGS; i++) {
		if ( !Q_strcasecmp((char *)Q2_surf_flags[i].name, (char *)flagName) ) {
			retVal = Q2_surf_flags[i].bit;
		}
	}
	
	return retVal;
}

static unsigned int contentflags_from_string (const char *flagName)
{
	int				i;
	unsigned int	retVal = 0;

	for (i = 0; i < NUM_Q2_CONTENT_FLAGS; i++) {
		if ( !Q_strcasecmp((char *)Q2_content_flags[i].name, (char *)flagName) ) {
			retVal = Q2_content_flags[i].bit;
		}
	}
	
	return retVal;
}
#endif

qboolean Com_ParseWalJSON (const char *fileName, const char *jsonStr, size_t jsonStrLen, miptex_t *mt, color_t *color, qboolean verbose)
{
	jsmn_parser		p;
	jsmntok_t		tok[1024];	//, *tok2;
	int				i, /*j,*/ nElements;
//	unsigned int	bit;
	char			keyStr[1024];
	char			valStr[1024];

	if ( !jsonStr || !mt || !color ) {
		jsonprintf (verbose, "ParseWalJSON (%s): called with NULL pointer(s)\n", fileName);
		return false;
	}

	jsonprintf (verbose, "Parsing %s, size = %i\n", fileName, jsonStrLen);

	jsmn_init (&p);
	nElements = jsmn_parse (&p, jsonStr, jsonStrLen, tok, sizeof(tok) / sizeof(tok[0]));

	// we had a parse error
	if (nElements < 0) {
		jsonprintf (verbose, "ParseWalJSON (%s): jsmn parse error %i\n", fileName, nElements);
		return false;
	}

	// must have at least 1 element
	if (nElements < 1) {
		jsonprintf (verbose, "ParseWalJSON (%s): too few elements (%i)\n", fileName, nElements);
		return false;
	}

	// assume top-level element is an object
	if (tok[0].type != JSMN_OBJECT) {
		jsonprintf (verbose, "ParseWalJSON (%s): first element is not an object\n", fileName);
		return false;
	}

	// We're only grabbing the width and height here, so zero everything else
	mt->value = mt->flags = mt->contents = 0;
	mt->animname[0] = 0;
	(*color)[0] = (*color)[1] = (*color)[2] = 0;

	// Parse all keys of JSON root object
	for (i = 1; i < nElements; i++)
	{
		if (jsoneq(jsonStr, &tok[i], "width") == 0)
		{
			if ( (tok[i+1].type == JSMN_STRING) || (tok[i+1].type == JSMN_PRIMITIVE) ) {
				memset (valStr, 0, sizeof(valStr));
				strncpy (valStr, jsonStr + tok[i+1].start, min(tok[i+1].end - tok[i+1].start, sizeof(valStr)-1));
				mt->width = atoi(valStr);
				jsonprintf (verbose, "width: %i\n", mt->width);
				i++;
			}
			else {
				jsonprintf (verbose, "%s has wrong type for 'width'\n", fileName);
			}
		}
		else if (jsoneq(jsonStr, &tok[i], "height") == 0)
		{
			if ( (tok[i+1].type == JSMN_STRING) || (tok[i+1].type == JSMN_PRIMITIVE) ) {
				memset (valStr, 0, sizeof(valStr));
				strncpy (valStr, jsonStr + tok[i+1].start, min(tok[i+1].end - tok[i+1].start, sizeof(valStr)-1));
				mt->height = atoi(valStr);
				jsonprintf (verbose, "height: %i\n", mt->height);
				i++;
			}
			else {
				jsonprintf (verbose, "%s has wrong type for 'height'\n", fileName);
			}
		}
		else if (jsoneq(jsonStr, &tok[i], "value") == 0)
		{
			if ( (tok[i+1].type == JSMN_STRING) || (tok[i+1].type == JSMN_PRIMITIVE) ) {
			/*	memset (valStr, 0, sizeof(valStr));
				strncpy (valStr, jsonStr + tok[i+1].start, min(tok[i+1].end - tok[i+1].start, sizeof(valStr)-1));
				mt->value = atoi(valStr);
				jsonprintf (verbose, "value: %i\n", mt->value); */
				i++;
			}
			else {
				jsonprintf (verbose, "%s has wrong type for 'value'\n", fileName);
			}
		}
		else if (jsoneq(jsonStr, &tok[i], "flags") == 0)
		{
			if (tok[i+1].type == JSMN_STRING) {
			/*	memset (valStr, 0, sizeof(valStr));
				strncpy (valStr, jsonStr + tok[i+1].start, min(tok[i+1].end - tok[i+1].start, sizeof(valStr)-1));
				// TODO: add support for parsing flags and contents from strings, e.g. LIGHT, SLICK, WATER, LADDER, etc.
				mt->flags = surfflags_from_string(valStr);
				jsonprintf (verbose, "flags: %s (%i)\n", valStr, mt->flags); */
				i++;
			}
			else if (tok[i+1].type == JSMN_PRIMITIVE) {
			/*	memset (valStr, 0, sizeof(valStr));
				strncpy (valStr, jsonStr + tok[i+1].start, min(tok[i+1].end - tok[i+1].start, sizeof(valStr)-1));
				mt->flags = atoi(valStr);
				jsonprintf (verbose, "flags: %s\n", valStr); */
				i++;
			}
			else if (tok[i+1].type == JSMN_ARRAY)
			{
			/*	mt->flags = 0;
				jsonprintf (verbose, "flags:");
				for (j = 0; j < tok[i+1].size; j++)
				{
					tok2 = &tok[i + j + 2];
					memset (valStr, 0, sizeof(valStr));
					strncpy (valStr, jsonStr + tok2->start, min(tok2->end - tok2->start, sizeof(valStr)-1));
					if (tok2->type == JSMN_STRING) {
						// TODO: add support for parsing flags and contents from strings, e.g. LIGHT, SLICK, WATER, LADDER, etc.
						bit = surfflags_from_string(valStr);
						mt->flags |= bit;
					}
					else if (tok2->type == JSMN_PRIMITIVE) {
						bit = atoi(valStr);
						mt->flags |= bit;
					}
					jsonprintf (verbose, " %s (%i)", valStr, bit);
				}
				jsonprintf (verbose, " = (%i)\n", mt->flags); */
				i += tok[i+1].size + 1;
			}
			else {
				jsonprintf (verbose, "%s has wrong type for 'flags'\n", fileName);
			}
		}
		else if (jsoneq(jsonStr, &tok[i], "contents") == 0)
		{
			if (tok[i+1].type == JSMN_STRING) {
			/*	memset (valStr, 0, sizeof(valStr));
				strncpy (valStr, jsonStr + tok[i+1].start, min(tok[i+1].end - tok[i+1].start, sizeof(valStr)-1));
				// TODO: add support for parsing flags and contents from strings, e.g. LIGHT, SLICK, WATER, LADDER, etc.
				mt->contents = contentflags_from_string(valStr);
				jsonprintf (verbose, "contents: %s (%i)\n", valStr, mt->contents); */
				i++;
			}
			else if (tok[i+1].type == JSMN_PRIMITIVE) {
			/*	memset (valStr, 0, sizeof(valStr));
				strncpy (valStr, jsonStr + tok[i+1].start, min(tok[i+1].end - tok[i+1].start, sizeof(valStr)-1));
				mt->contents = atoi(valStr);
				jsonprintf (verbose, "contents: %s\n", valStr); */
				i++;
			}
			else if (tok[i+1].type == JSMN_ARRAY)
			{
			/*	mt->contents = 0;
				jsonprintf (verbose, "contents:");
				for (j = 0; j < tok[i+1].size; j++)
				{
					tok2 = &tok[i + j + 2];
					memset (valStr, 0, sizeof(valStr));
					strncpy (valStr, jsonStr + tok2->start, min(tok2->end - tok2->start, sizeof(valStr)-1));
					if (tok2->type == JSMN_STRING) {
						// TODO: add support for parsing flags and contents from strings, e.g. LIGHT, SLICK, WATER, LADDER, etc.
						bit = contentflags_from_string(valStr);
						mt->contents |= bit;
					}
					else if (tok2->type == JSMN_PRIMITIVE) {
						bit = atoi(valStr);
						mt->contents |= bit;
					}
					jsonprintf (verbose, " %s (%i)", valStr, bit);
				}
				jsonprintf (verbose, " = (%i)\n", mt->contents); */
				i += tok[i+1].size + 1;
			}
			else {
				jsonprintf (verbose, "%s has wrong type for 'contents'\n", fileName);
			}
		}
		else if (jsoneq(jsonStr, &tok[i], "animation") == 0)
		{
			if (tok[i+1].type == JSMN_STRING)  {
			/*	memset (valStr, 0, sizeof(valStr));
				strncpy (valStr, jsonStr + tok[i+1].start, min(tok[i+1].end - tok[i+1].start, sizeof(valStr)-1));
				Q_strncpyz (mt->animname, sizeof(mt->animname), valStr);
				jsonprintf (verbose, "animname: %s\n", valStr); */
				i++;
			}
			else {
				jsonprintf (verbose, "%s has wrong type for 'animation'\n", fileName);
			}
		}
		else if (jsoneq(jsonStr, &tok[i], "color") == 0)
		{
			if (tok[i+1].type == JSMN_STRING) 
			{
			/*	memset (valStr, 0, sizeof(valStr));
				strncpy (valStr, jsonStr + tok[i+1].start, min(tok[i+1].end - tok[i+1].start, sizeof(valStr)-1));
				if (sscanf (valStr, "%i %i %i", &(*color)[0], &(*color)[1], &(*color)[2]) == EOF) {
					jsonprintf (verbose, "%s has invalid string for color: %s\n", fileName, valStr);
				}
				else {
					(*color)[3] = 255;
					jsonprintf (verbose, "color: %i %i %i\n", (*color)[0], (*color)[1], (*color)[2]);
				} */
				i++;
			}
			else if (tok[i+1].type == JSMN_ARRAY)
			{
			/*	if ( (tok[i+1].size == 3) || (tok[i+1].size == 4) )
				{
					(*color)[3] = 255;
					for (j = 0; j < 3; j++) {
						tok2 = &tok[i + j + 2];
						memset (valStr, 0, sizeof(valStr));
						strncpy (valStr, jsonStr + tok2->start, min(tok2->end - tok2->start, sizeof(valStr)-1));
						(*color)[j] = atoi(valStr);
					}
					jsonprintf (verbose, "color: %i %i %i\n", (*color)[0], (*color)[1], (*color)[2]);
				}
				else {
					jsonprintf (verbose, "%s has wrong array size for 'color'\n", fileName);
				} */
				i += tok[i+1].size + 1;
			}
			else {
				jsonprintf (verbose, "%s has wrong type for 'color'\n", fileName);
			}
		}
		else {
			memset (keyStr, 0, sizeof(keyStr));
			strncpy (keyStr, jsonStr + tok[i].start, min(tok[i].end - tok[i].start, sizeof(keyStr)-1));
			jsonprintf (verbose, "%s has unknown field: %s\n", fileName, keyStr);
		}
	}

	return true;
}

/*
============================================================================

OGG_JSON LOADING

============================================================================
*/

qboolean Com_ParseOggJSON (const char *fileName, const char *jsonStr, size_t jsonStrLen, oggImport_t *iData, qboolean verbose)
{
	jsmn_parser		p;
	jsmntok_t		tok[1024];	//, *tok2;
	int				i, j, nElements;
	char			keyStr[1024];
	char			valStr[1024];
	size_t			keyLen;
	char			numCh;
	char			numBuf[4] = {0};

	if ( !jsonStr || !iData ) {
		jsonprintf (verbose, "ParseOggJSON (%s): called with NULL pointer(s)\n", fileName);
		return false;
	}

	jsonprintf (verbose, "Parsing %s, size = %i\n", fileName, jsonStrLen);

	jsmn_init (&p);
	nElements = jsmn_parse (&p, jsonStr, jsonStrLen, tok, sizeof(tok) / sizeof(tok[0]));

	// we had a parse error
	if (nElements < 0) {
		jsonprintf (verbose, "ParseOggJSON (%s): jsmn parse error %i\n", fileName, nElements);
		return false;
	}

	// must have at least 1 element
	if (nElements < 1) {
		jsonprintf (verbose, "ParseOggJSON (%s): too few elements (%i)\n", fileName, nElements);
		return false;
	}

	// assume top-level element is an object
	if (tok[0].type != JSMN_OBJECT) {
		jsonprintf (verbose, "ParseOggJSON (%s): first element is not an object\n", fileName);
		return false;
	}

	// Zero all fields
	iData->virtualName[0] = 0;
	iData->importGame[0] = 0;
	for (i = 0; i < MAX_OGG_IMPORT_PATHS; i++) {
		iData->importPath[i][0] = 0;
	}

	// Parse all keys of JSON root object
	for (i = 1; i < nElements; i++)
	{
		if (jsoneq(jsonStr, &tok[i], "virtualName") == 0)
		{
			if (tok[i+1].type == JSMN_STRING)  {
				memset (valStr, 0, sizeof(valStr));
				strncpy (valStr, jsonStr + tok[i+1].start, min(tok[i+1].end - tok[i+1].start, sizeof(valStr)-1));
				Q_strncpyz (iData->virtualName, sizeof(iData->virtualName), valStr);
				jsonprintf (verbose, "virtualName: %s\n", valStr);
				i++;
			}
			else {
				jsonprintf (verbose, "%s has wrong type for 'virtualName'\n", fileName);
			}
		}
		else if (jsoneq(jsonStr, &tok[i], "importGame") == 0)
		{
			if (tok[i+1].type == JSMN_STRING)  {
				memset (valStr, 0, sizeof(valStr));
				strncpy (valStr, jsonStr + tok[i+1].start, min(tok[i+1].end - tok[i+1].start, sizeof(valStr)-1));
				Q_strncpyz (iData->importGame, sizeof(iData->importGame), valStr);
				jsonprintf (verbose, "importGame: %s\n", valStr);
				i++;
			}
			else {
				jsonprintf (verbose, "%s has wrong type for 'importGame'\n", fileName);
			}
		}
		else if ( (jsoneq(jsonStr, &tok[i], "importPath0") == 0) || (jsoneq(jsonStr, &tok[i], "importPath1") == 0) ||
				(jsoneq(jsonStr, &tok[i], "importPath2") == 0) || (jsoneq(jsonStr, &tok[i], "importPath3") == 0) ||
				(jsoneq(jsonStr, &tok[i], "importPath4") == 0) || (jsoneq(jsonStr, &tok[i], "importPath5") == 0) ||
				(jsoneq(jsonStr, &tok[i], "importPath6") == 0) || (jsoneq(jsonStr, &tok[i], "importPath7") == 0) )
		{
			memset (keyStr, 0, sizeof(keyStr));
			strncpy (keyStr, jsonStr + tok[i].start, min(tok[i].end - tok[i].start, sizeof(keyStr)-1));
			keyLen = strlen(keyStr);
			numCh = keyStr[keyLen - 1];
			numBuf[0] = numCh;
			j = atoi(numBuf);
		//	jsonprintf (verbose, "parsing %s (%c, %i)\n", keyStr, numCh, j);
			j = min(max(j, 0), MAX_OGG_IMPORT_PATHS - 1);
			if (tok[i+1].type == JSMN_STRING) {
				memset (valStr, 0, sizeof(valStr));
				strncpy (valStr, jsonStr + tok[i+1].start, min(tok[i+1].end - tok[i+1].start, sizeof(valStr)-1));
				Q_strncpyz (iData->importPath[j], sizeof(iData->importPath[j]), valStr);
				jsonprintf (verbose, "importPath%i: %s\n", j, valStr);
				i++;
			}
			else {
				jsonprintf (verbose, "%s has wrong type for 'importPath%i'\n", fileName, j);
			}
		}
		else {
			memset (keyStr, 0, sizeof(keyStr));
			strncpy (keyStr, jsonStr + tok[i].start, min(tok[i].end - tok[i].start, sizeof(keyStr)-1));
			jsonprintf (verbose, "%s has unknown field: %s\n", fileName, keyStr);
		}
	}

	return true;
}
