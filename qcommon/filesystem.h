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

#if defined (_MSC_VER) && (_MSC_VER <= 1200)	// use older version of zlib for MSVC6
#include "../include/zlibpng_vc6/unzip.h"
#include "../include/zlibpng_vc6/zip.h"
#else
#include "../include/zlibpng/unzip.h"
#include "../include/zlibpng/zip.h"
#endif

#define BASEDIRNAME				"baseq2"

#define MAX_HANDLES				32
#define MAX_READ				0x10000
#define MAX_WRITE				0x10000
#define MAX_FIND_FILES			0x04000

#define	USE_SAVEGAMEDIR			// whether to use new fs_savegamedir/fs_downloaddir paths

//
// in memory
//

//
// Berserk's pk3 file support
//

typedef struct {
	char			name[MAX_QPATH];
	unsigned int	hash;				// To speed up searching
	int				size;
	int				offset;				// This is ignored in PK3 files
	qboolean		ignore;				// Whether this file should be ignored
} fsPackFile_t;

typedef struct {
	char			name[MAX_QPATH];
	fsMode_t		mode;
	FILE			*file;				// Only one of file or
	unzFile			*zip;				// zip will be used
	zipFile			*writeZip;			// Only used for writing to zip file
	fsPackFile_t	*pakFile;			// Only used for seek/tell in .pak files
} fsHandle_t;

typedef struct fsLink_s {
	char			*from;
	int				length;
	char			*to;
	struct fsLink_s	*next;
} fsLink_t;


typedef struct {
	char			name[MAX_OSPATH];
	FILE			*pak;
	unzFile			*pk3;
	int				numFiles;
	fsPackFile_t	*files;
	unsigned int	contentFlags;
	qboolean		isProtectedPak;	// from Yamagi Q2
} fsPack_t;

typedef struct fsSearchPath_s {
	char			path[MAX_OSPATH];	// Only one of path or
	fsPack_t		*pack;				// pack will be used
	struct fsSearchPath_s	*next;
} fsSearchPath_t;

fsHandle_t		fs_handles[MAX_HANDLES];
fsLink_t		*fs_links;
fsSearchPath_t	*fs_searchPaths;
fsSearchPath_t	*fs_baseSearchPaths;

char			fs_gamedir[MAX_OSPATH];
char			fs_savegamedir[MAX_OSPATH];
char			fs_downloaddir[MAX_OSPATH];
static char		fs_currentGame[MAX_QPATH];

static char				fs_fileInPath[MAX_OSPATH];
static qboolean			fs_fileInPack;

int		file_from_protected_pak;	// This is set by FS_FOpenFile, from Yamagi Q2
int		file_from_pak = 0;		// This is set by FS_FOpenFile
int		file_from_pk3 = 0;		// This is set by FS_FOpenFile
char	last_pk3_name[MAX_QPATH];	// This is set by FS_FOpenFile

cvar_t	*fs_homepath;
cvar_t	*fs_basedir;
cvar_t	*fs_cddir;
cvar_t	*fs_basegamedir;
cvar_t	*fs_basegamedir2;
cvar_t	*fs_basegamedir3;	// So we can mount Rogue, Xatrix, and Zaero assets at once
cvar_t	*fs_gamedirvar;
cvar_t	*fs_debug;
cvar_t	*fs_roguegame;

#ifdef _WIN32
cvar_t	*win_use_profile_dir;	// whether to use user profile dir for savegames, configs, screenshots, etc
#endif

fsHandle_t	*FS_GetFileByHandle (fileHandle_t f);
