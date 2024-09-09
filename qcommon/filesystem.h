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

#ifdef _WIN32
#include "../libraries/zlib/unzip.h"
#include "../libraries/zlib/zip.h"
#elif defined (__linux__)
#include "../unix/zip/unzip.h"
#include "../unix/zip/zip.h"
#else // _WIN32
//#include <minizip/unzip.h>
//#include <minizip/zip.h>
// KMQ2 MacOSX port builds with the above, but the below should work as well.
#include "../unix/zip/unzip.h"
#include "../unix/zip/zip.h"
#endif // _WIN32

#define BASEDIRNAME				"baseq2"

#define MAX_HANDLES				32
#define MAX_READ				0x10000
#define MAX_WRITE				0x10000
#define MAX_FIND_FILES			0x04000

#define	USE_SAVEGAMEDIR			// whether to use new fs_savegamedir/fs_downloaddir paths

#define USE_Q2RR_IMPORT_PATH	// whether to use Quake2 re-release import paths

//
// in memory
//

//
// Berserk's pk3 file support
//

typedef struct fsLink_s {
	char			*from;
	int				length;
	char			*to;
	struct fsLink_s	*next;
} fsLink_t;

typedef struct {
	char			name[MAX_QPATH];
	unsigned int	hash;				// To speed up searching
	unsigned int	size;
	unsigned int	offset;				// This is ignored in PK3 files
	qboolean		ignore;				// Whether this file should be ignored
	qboolean		isRemapped;			// Whether this file is renamed via an import list
} fsPackFile_t;

typedef struct {
	char			name[MAX_OSPATH];
	FILE			*pak;
	unzFile			*pk3;
	int				numFiles;
	fsPackFile_t	*files;
	unsigned int	contentFlags;
	qboolean		isProtectedPak;		// from Yamagi Q2
	qboolean		isQuakeImportPak;	// Whether this pak is a Quake import
	int				numRemappedFiles;	// Num of remapped files if pak is a Quake import
} fsPack_t;

typedef struct fsSearchPath_s {
	char			path[MAX_OSPATH];	// Only one of path or
	fsPack_t		*pack;				// pack will be used
	struct fsSearchPath_s	*next;
} fsSearchPath_t;

typedef struct {
	char			name[MAX_QPATH];
	fsMode_t		mode;
	FILE			*file;				// Only one of file or
	unzFile			*zip;				// zip will be used
	zipFile			*writeZip;			// Only used for writing to zip file
	fsPackFile_t	*pakFile;			// Only used for seek/tell in .pak files
} fsHandle_t;

typedef struct {
	char			orgName[MAX_QPATH];
	char			remapName[MAX_QPATH];
	unsigned int	timesUsed;
} fsPackItemRemap_t;

fsHandle_t		fs_handles[MAX_HANDLES];
fsLink_t		*fs_links;
fsSearchPath_t	*fs_searchPaths;
fsSearchPath_t	*fs_baseSearchPaths;
fsPackItemRemap_t *fs_pakItemRemaps;
extern int		fs_numPakItemRemaps;
extern char		fs_pakRemapScriptName[MAX_OSPATH];

char			fs_gamedir[MAX_OSPATH];
char			fs_savegamedir[MAX_OSPATH];
char			fs_downloaddir[MAX_OSPATH];
static char		fs_currentGame[MAX_QPATH];

static char				fs_fileInPath[MAX_OSPATH];
static qboolean			fs_fileInPack;

extern int		file_from_protected_pak;	// This is set by FS_FOpenFile, from Yamagi Q2
extern int		file_from_pak;				// This is set by FS_FOpenFile
extern int		file_from_pk3;				// This is set by FS_FOpenFile
extern char		last_pk3_name[MAX_QPATH];	// This is set by FS_FOpenFile

cvar_t	*fs_homepath;
cvar_t	*fs_basedir;
cvar_t	*fs_cddir;
cvar_t	*fs_basegamedir1;
cvar_t	*fs_basegamedir2;
cvar_t	*fs_basegamedir3;	// So we can mount Rogue, Xatrix, and Zaero assets at once
cvar_t	*fs_gamedirvar;
cvar_t	*fs_quakeimportpath_auto;		// Whether to auto-detect Quake1 Steam install path
cvar_t	*fs_quakerrimportpath_auto;		// Whether to auto-detect Quake1RR Steam install path
cvar_t	*fs_quakeimportpath;			// Install path of Quake1 for content mounting, id1 folder paks are automatically added
cvar_t	*fs_quakemaingame;				// Name override of Quake1 id1 folder, to allow mounting content for other Quake1 engine games such as Hexen2
cvar_t	*fs_quakegamedir1;				// Additional gamedirs for mounting the Quake mission packs and mods' .pak files
cvar_t	*fs_quakegamedir2;
cvar_t	*fs_quakegamedir3;
cvar_t	*fs_quakegamedir4;

#ifdef USE_Q2RR_IMPORT_PATH
cvar_t	*fs_quake2rrimportpath_auto;	// Whether to auto-detect Quake2RR Steam install path
cvar_t	*fs_quake2rrimportpath;			// Install path of Quake2RR for content mounting, baseq2 folder paks are automatically added
cvar_t	*fs_quake2rrmaingame;			// Name override of Quake2RR baseq2 folder, to allow mounting content for other Quake2 engine games
cvar_t	*fs_quake2rrgamedir1;			// Additional gamedirs for mounting Quake2RR mods' .pak files
cvar_t	*fs_quake2rrgamedir2;
cvar_t	*fs_quake2rrgamedir3;
cvar_t	*fs_quake2rrgamedir4;
#endif	// USE_Q2RR_IMPORT_PATH

cvar_t	*fs_debug;
cvar_t	*fs_xatrixgame;
cvar_t	*fs_roguegame;

#ifdef _WIN32
cvar_t	*win_use_profile_dir;	// whether to use user profile dir for savegames, configs, screenshots, etc
#endif

fsHandle_t	*FS_GetFileByHandle (fileHandle_t f);
