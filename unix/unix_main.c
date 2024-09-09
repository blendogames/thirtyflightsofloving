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

// unix_main.c

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#include <dlfcn.h>		// needed for GCC to avoid dlopen errors
#include <sys/resource.h>

#include <SDL.h>

#include "../qcommon/qcommon.h"
#include "unixquake.h"

cvar_t *nostdout;

unsigned	sys_frame_time;

uid_t saved_euid;
qboolean stdin_active = true;
int			ActiveApp;

// Knightmare- added exe / pref dirs
static char exe_dir[MAX_OSPATH];
static char pref_dir[MAX_OSPATH];
static char	download_dir[MAX_OSPATH];

// Knightmare- system info cvars
cvar_t *sys_osVersion;
cvar_t *sys_cpuString;
cvar_t *sys_ramMegs;
cvar_t *sys_ramMegs_perApp;

// =======================================================================
// General routines
// =======================================================================

void Sys_ConsoleOutput (const char *string)
{
	if (nostdout && nostdout->value)
		return;

	fputs(string, stdout);
}

void Sys_Sleep (int msec)
{
	if (msec < 0)	return;
	usleep ((unsigned int)msec * 1000);
}

unsigned Sys_TickCount (void)
{
	return clock();
}

void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];
	unsigned char		*p;

	va_start (argptr, fmt);
//	vsprintf (text, fmt, argptr);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	if (strlen(text) > sizeof(text))
		Sys_Error("memory overwrite in Sys_Printf");

	if (nostdout && nostdout->value)
		return;

	for (p = (unsigned char *)text; *p; p++) {
		*p &= 0x7f;
		if ((*p > 128 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
			printf("[%02x]", *p);
		else
			putc(*p, stdout);
	}
}

void Sys_Quit (void)
{
	CL_Shutdown ();
	Qcommon_Shutdown ();
	fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
	_exit (0);
}

// Knightmare added
#if defined (__linux__)
/*
=================
Sys_DetectOS
parses /etc/os-release
=================
*/
static qboolean Sys_DetectOS (char *osString, int osStringSize)
{
	char		*line = NULL, *equalsTok = NULL;
	char		osNameBuf[128] = {0}, osVersionBuf[128] = {0};
	char		osName[64] = {0}, osVersion[64] = {0};
	FILE		*osFile = NULL;
	size_t		lineSize = 128, bufLen, stringLen;

	if ( !osString || (osStringSize <= 0) )
		return false;

	osFile = fopen("/etc/os-release", "r");
	if ( !osFile ) {
		Com_Printf ("couldn't open \"/etc/os-release\"\n");
		return false;
	}

	line = malloc (sizeof(char) * lineSize);
	while ( getdelim(&line, &lineSize, '\n', osFile) != -1 )
	{
		if ( !Q_strncasecmp(line, "NAME=", 5) ) {
			Q_strncpyz (osNameBuf, sizeof(osNameBuf), line);
			bufLen = strlen(osNameBuf);
			if (osNameBuf[bufLen-1] == '\n')
				osNameBuf[bufLen-1] = 0;
		}
		if ( !Q_strncasecmp(line, "VERSION=", 8) ) {
			Q_strncpyz (osVersionBuf, sizeof(osVersionBuf), line);
			bufLen = strlen(osVersionBuf);
			if (osVersionBuf[bufLen-1] == '\n')
				osVersionBuf[bufLen-1] = 0;
		}
	}
	free (line);
	fclose (osFile);

	// parse OS name
	equalsTok = strstr(osNameBuf, "=");
	if (equalsTok != NULL) {
		Q_strncpyz (osString, osStringSize, equalsTok + 2);
		stringLen = strlen(osString);
		if (osString[stringLen-1] == '\"')
			osString[stringLen-1] = 0;
	}
	else {	// no OS name
	//	Com_Printf ("no OS name found\n");
		return false;
	}

	// parse OS version
	equalsTok = strstr(osVersionBuf, "=");
	if (equalsTok != NULL) {
		Q_strncatz (osString, osStringSize, va(" %s", equalsTok + 2));
		stringLen = strlen(osString);
		if (osString[stringLen-1] == '\"')
			osString[stringLen-1] = 0;
	}

	return true;
}

/*
=================
Sys_DetectCPU
parses /proc/cpuinfo
=================
*/
static qboolean Sys_DetectCPU (char *cpuString, int cpuStringSize)
{
	char		*line = NULL, *colonTok = NULL;
	char		mdlNameBuf[128] = {0}, mhzBuf[128] = {0}, flagsBuf[2048] = {0};
	FILE		*cpuFile = NULL;
	size_t		lineSize = 1024, bufLen;
	int			numLogicalProcessors =  0;
	float		speed = 0.0f, speedTmp;
	qboolean	hasMMX, hasMMXExt, has3DNow, has3DNowExt, hasSSE, hasSSE2, hasSSE3, hasSSE41, hasSSE42, hasSSE4a, hasAVX;

	if ( !cpuString || (cpuStringSize <= 0) )
		return false;

	cpuFile = fopen("/proc/cpuinfo", "r");
	if ( !cpuFile ) {
		Com_Printf ("couldn't open \"/proc/cpuinfo\"\n");
		return false;
	}

	line = malloc (sizeof(char) * lineSize);
	while ( getdelim(&line, &lineSize, '\n', cpuFile) != -1 )
	{
		if ( !Q_strncasecmp(line, "model name", 10) ) {
			Q_strncpyz (mdlNameBuf, sizeof(mdlNameBuf), line);
			bufLen = strlen(mdlNameBuf);
			if (mdlNameBuf[bufLen-1] == '\n')
				mdlNameBuf[bufLen-1] = 0;
		}
		if ( !Q_strncasecmp(line, "cpu MHz", 7) ) {
			Q_strncpyz (mhzBuf, sizeof(mhzBuf), line);
			bufLen = strlen(mhzBuf);
			if (mhzBuf[bufLen-1] == '\n')
				mhzBuf[bufLen-1] = 0;
		}
		if ( !Q_strncasecmp(line, "flags", 5) ) {
			Q_strncpyz (flagsBuf, sizeof(flagsBuf), line);
			bufLen = strlen(flagsBuf);
			if (flagsBuf[bufLen-1] == '\n')
				flagsBuf[bufLen-1] = 0;
		}
		if ( !Q_strncasecmp(line, "processor", 9) )
			numLogicalProcessors++;

		// parse CPU speed for each logical processor
		// this prevents reporting the speed of an idle core
		colonTok = strstr(mhzBuf, ": ");
		if (colonTok != NULL) {
			speedTmp = atof(colonTok + 2);
			if (speedTmp > speed)
				speed = speedTmp;
		}
	}
	free (line);
	fclose (cpuFile);

	// parse CPU name
	colonTok = strstr(mdlNameBuf, ": ");
	if (colonTok != NULL) {
		Q_strncpyz (cpuString, cpuStringSize, colonTok + 2);
	}
	else {	// no model name
	//	Com_Printf ("no CPU model name found\n");
		return false;
	}

	// show CPU speed
	if (speed > 0.0f) {
		if (speed > 1000)
			Q_strncatz (cpuString, cpuStringSize, va(" %4.2f GHz", ((float)speed/1000.0f)));
		else
			Q_strncatz (cpuString, cpuStringSize, va(" %u MHz", speed));
	}

	// show num of logical processors
	if (numLogicalProcessors > 1) {
		Q_strncatz (cpuString, cpuStringSize, va(" (%u logical CPUs)", numLogicalProcessors));
	}

	// Get extended instruction sets supported
	colonTok = strstr(flagsBuf, ": ");
	if (colonTok != NULL)
	{
		hasMMX = Q_StrScanToken(colonTok, "mmx", false);
		hasMMXExt = Q_StrScanToken(colonTok, "mmxext", false);
		has3DNow = Q_StrScanToken(colonTok, "3dnow", false);
		has3DNowExt = Q_StrScanToken(colonTok, "3dnowext", false);
		hasSSE = Q_StrScanToken(colonTok, "sse", false);
		hasSSE2 = Q_StrScanToken(colonTok, "sse2", false);
		hasSSE3 = Q_StrScanToken(colonTok, "sse3", false);
		hasSSE41 = Q_StrScanToken(colonTok, "sse4_1", false);
		hasSSE42 = Q_StrScanToken(colonTok, "sse4_2", false);
		hasSSE4a = Q_StrScanToken(colonTok, "sse4a", false);
		hasAVX = Q_StrScanToken(colonTok, "avx", false);

		Q_strncatz (cpuString, cpuStringSize, " w/");

		if (hasMMX) {
			Q_strncatz (cpuString, cpuStringSize, " MMX");
			if (hasMMXExt)
				Q_strncatz (cpuString, cpuStringSize, "+");
		}
		if (has3DNow) {
			Q_strncatz (cpuString, cpuStringSize, " 3DNow!");
			if (has3DNowExt)
				Q_strncatz (cpuString, cpuStringSize, "+");
		}
		if (hasSSE) {
			Q_strncatz (cpuString, cpuStringSize, " SSE");
			if (hasSSE42)
				Q_strncatz (cpuString, cpuStringSize, "4.2");
			else if (hasSSE41)
				Q_strncatz (cpuString, cpuStringSize, "4.1");
			else if (hasSSE3)
				Q_strncatz (cpuString, cpuStringSize, "3");
			else if (hasSSE2)
				Q_strncatz (cpuString, cpuStringSize, "2");
		}
		if (hasSSE4a) {
			Q_strncatz (cpuString, cpuStringSize, " SSE4a");
		}
		if (hasAVX) {
			Q_strncatz (cpuString, cpuStringSize, " AVX");
		}
	}

	return true;
}


/*
=================
Sys_DetectRAM
parses /proc/meminfo
=================
*/
static qboolean Sys_DetectRAM (char *memString, int memStringSize, char *memStringAcc, int memStringAccSize)
{
	char			*line = NULL;
	char			memTotalBuf[64] = {0};
	FILE			*memFile = NULL;
	size_t			lineSize = 128, bufLen;
	unsigned int	memTotal_kb, memTotal_mb, memTotalAcc_mb;

	if ( !memString || (memStringSize <= 0) || !memStringAcc  || (memStringAccSize <= 0) )
		return false;

	memFile = fopen("/proc/meminfo", "r");
	if ( !memFile ) {
		Com_Printf ("couldn't open \"/proc/meminfo\"\n");
		return false;
	}

	line = malloc (sizeof(char) * lineSize);
	while ( getdelim(&line, &lineSize, '\n', memFile) != -1 )
	{
		if ( !Q_strncasecmp(line, "MemTotal:", 9) ) {
			Q_strncpyz (memTotalBuf, sizeof(memTotalBuf), line);
			bufLen = strlen(memTotalBuf);
			if (memTotalBuf[bufLen-1] == '\n')
				memTotalBuf[bufLen-1] = 0;
		}
	}
	free (line);
	fclose (memFile);

	// parse memTotal
	if (sscanf(memTotalBuf, "MemTotal: %u kB", &memTotal_kb) == 1) {
		memTotal_mb = (memTotal_kb >> 10);
#if defined (_M_X64) || defined (_M_AMD64) || defined (__x86_64__) || defined (__ia64__) || defined (__aarch64__)
		memTotalAcc_mb = memTotal_mb;
#else
		memTotalAcc_mb = min(memTotal_mb, 4095);	// cap at 4095 for 32-bit builds
#endif
		Q_strncpyz (memString, memStringSize, va("%u", memTotal_mb));
		Q_strncpyz (memStringAcc, memStringAccSize, va("%u", memTotalAcc_mb));
	}
	else {	// no model name
	//	Com_Printf ("no memTotal value found\n");
		return false;
	}

	return true;
}
#endif	// defined (__linux__)
// end Knightmare

void Sys_Init (void)
{
// Knightmare- added system info detection
#if defined (__linux__)
	char		osString[128], cpuString[128];
	char		memString[128], memStringAcc[128];

	// Detect OS version
	if ( Sys_DetectOS (osString, sizeof(osString)) ) {
		Com_Printf ("OS: %s\n", osString);
		sys_osVersion = Cvar_Get ("sys_osVersion", osString, CVAR_NOSET|CVAR_LATCH|CVAR_SAVE_IGNORE);
	}
	else {
		Com_Printf ("Couldn't detect OS info\n");
		sys_osVersion = Cvar_Get ("sys_osVersion", "Unknown", CVAR_NOSET|CVAR_LATCH|CVAR_SAVE_IGNORE);
	}

	// Detect CPU
	if ( Sys_DetectCPU (cpuString, sizeof(cpuString)) ) {
		Com_Printf ("CPU: %s\n", cpuString);
		sys_cpuString = Cvar_Get ("sys_cpuString", cpuString, CVAR_NOSET|CVAR_LATCH|CVAR_SAVE_IGNORE);
	}
	else {
		Com_Printf ("Unknown CPU found\n");
		sys_cpuString = Cvar_Get ("sys_cpuString", "Unknown", CVAR_NOSET|CVAR_LATCH|CVAR_SAVE_IGNORE);
	}

	// Detect physical memory
	if ( Sys_DetectRAM (memString, sizeof(memString), memStringAcc, sizeof(memStringAcc)) ) {
		Com_Printf ("Memory: %s MB (%s MB accessible)\n", memString, memStringAcc);
		sys_ramMegs = Cvar_Get ("sys_ramMegs", memString, CVAR_NOSET|CVAR_LATCH|CVAR_SAVE_IGNORE);
		sys_ramMegs_perApp = Cvar_Get ("sys_ramMegs_perApp", memStringAcc, CVAR_NOSET|CVAR_LATCH|CVAR_SAVE_IGNORE);
	}
	else {
		Com_Printf ("Unknown amount of memory\n");
		sys_ramMegs = Cvar_Get ("sys_ramMegs", "Unknown", CVAR_NOSET|CVAR_LATCH|CVAR_SAVE_IGNORE);
		sys_ramMegs_perApp = Cvar_Get ("sys_ramMegs_perApp", "Unknown", CVAR_NOSET|CVAR_LATCH|CVAR_SAVE_IGNORE);
	}
#endif	// defined (__linux__)
// end system info detection

#if id386
//	Sys_SetFPCW();
#endif
}

void Sys_Error (const char *error, ...)
{ 
	va_list	 	argptr;
	char		string[1024];

// change stdin to non blocking
	fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);

	CL_Shutdown ();
	Qcommon_Shutdown ();

	va_start (argptr, error);
//	vsprintf (string, error, argptr);
	Q_vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);
	fprintf(stderr, "Error: %s\n", string);

	_exit (1);
} 

void Sys_Warn (char *warning, ...)
{ 
	va_list	 	argptr;
	char		string[1024];

	va_start (argptr, warning);
//	vsprintf (string, warning, argptr);
	Q_vsnprintf (string, sizeof(string), warning, argptr);
	va_end (argptr);
	fprintf(stderr, "Warning: %s", string);
} 

/*
============
Sys_FileTime

returns -1 if not present
============
*/
int	Sys_FileTime (char *path)
{
	struct	stat	buf;
	
	if (stat (path,&buf) == -1)
		return -1;
	
	return buf.st_mtime;
}

void floating_point_exception_handler(int whatever)
{
//	Sys_Warn("floating point exception\n");
	signal(SIGFPE, floating_point_exception_handler);
}

char *Sys_ConsoleInput(void)
{
	static char text[256];
	int	 	len;
	fd_set	fdset;
	struct	timeval timeout;

	if (!dedicated || !dedicated->value)
		return NULL;

	if (!stdin_active)
		return NULL;

	FD_ZERO(&fdset);
	FD_SET(0, &fdset); // stdin
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	if (select (1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(0, &fdset))
		return NULL;

	len = read (0, text, sizeof(text));
	if (len == 0) { // eof!
		stdin_active = false;
		return NULL;
	}

	if (len < 1)
		return NULL;
	text[len-1] = 0;	// rip off the /n and terminate

	return text;
}

// Knightmare added
void Sys_ShowConsole (qboolean show)
{
}

/*****************************************************************************/

/*
=================
Sys_LoadLibrary

From Yamagi Q2
=================
*/
void *Sys_LoadLibrary (const char *libPath, const char *initFuncName, void **libHandle)
{
	void	*hLibrary, *funcPtr;

	*libHandle = NULL;

#ifdef USE_SANITIZER
	hLibrary = dlopen (libPath, RTLD_LAZY | RTLD_NODELETE);
#else
	hLibrary = dlopen (libPath, RTLD_LAZY);
#endif

	if ( !hLibrary ) {
		Com_DPrintf("Sys_LoadLibrary: failure on %s, dlopen returned %s\n", libPath, dlerror());
		return NULL;
	}

	if (initFuncName != NULL)
	{
		funcPtr = dlsym (hLibrary, initFuncName);

		if ( !funcPtr ) {
			Com_DPrintf("Sys_LoadLibrary: failure in %s on %s, dlclose returned %s\n", libPath, initFuncName, dlerror());
			dlclose (hLibrary);
			return NULL;
		}
	}
	else {
		funcPtr = NULL;
	}

	*libHandle = hLibrary;

	Com_DPrintf("Sys_LoadLibrary: sucessfully loaded %s\n", libPath);

	return funcPtr;
}


/*
=================
Sys_FreeLibrary

From Yamagi Q2
=================
*/
void Sys_FreeLibrary (void *libHandle)
{
	if ( libHandle && dlclose(libHandle) )
	{
		Com_Error(ERR_FATAL, "dlclose failed on %p: %s", libHandle, dlerror());
	}
}


/*
=================
Sys_GetProcAddress

From Yamagi Q2
=================
*/
void *Sys_GetProcAddress (void *libHandle, const char *initFuncName)
{
	if (libHandle == NULL)
	{
#ifdef RTLD_DEFAULT
		return dlsym(RTLD_DEFAULT, initFuncName);
#else
		/* POSIX suggests that this is a portable equivalent */
		static void *global_namespace = NULL;

		if (global_namespace == NULL)
			global_namespace = dlopen(NULL, RTLD_GLOBAL|RTLD_LAZY);

		return dlsym (global_namespace, initFuncName);
#endif
	}
	return dlsym (libHandle, initFuncName);
}

/*****************************************************************************/

static void *game_library;

/*
=================
Sys_UnloadGame
=================
*/
void Sys_UnloadGame (void)
{
	if (game_library) 
		SDL_UnloadObject (game_library);
	game_library = NULL;
}

/*
=================
Sys_GetGameAPI

Loads the game dll
=================
*/
void *Sys_GetGameAPI (void *parms)
{
	void	*(*GetGameAPI) (void *);
	char	name[MAX_OSPATH];
	char	curpath[MAX_OSPATH];
	char	*path;

#ifdef __APPLE__
	// KMQ2 MacOSX port uses the Fruitz of Dojo plug system.  So this will go unused.
	#define LIB_SUFFIX "dylib"
#else
	#define LIB_SUFFIX "so"
#endif

	// Knightmare- changed game library name for better cohabitation
#if defined (_M_X64) || defined (_M_AMD64) || defined (__x86_64__)
	const char *gamename = "kmq2gamex64." LIB_SUFFIX;
#elif defined (__i386__)
	const char *gamename = "kmq2gamei386." LIB_SUFFIX;
#elif defined (__ia64__)
	const char *gamename = "kmq2gameia64." LIB_SUFFIX;
#elif defined (__alpha__)
	const char *gamename = "kmq2gameaxp." LIB_SUFFIX;
#elif defined (__arm__)
	const char *gamename = "kmq2gamearm32." LIB_SUFFIX;
#elif defined (__aarch64__)
	const char *gamename = "kmq2gamearm64." LIB_SUFFIX;
#elif defined (__powerpc__)
	const char *gamename = "kmq2gameppc." LIB_SUFFIX;
#elif defined (__sparc__)
	const char *gamename = "kmq2gamesparc." LIB_SUFFIX;
#else
#error Unknown arch
#endif

	setreuid (getuid(), getuid());
	setegid (getgid());

	if (game_library)
		Com_Error (ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadingGame");

	getcwd (curpath, sizeof(curpath));

	Com_Printf ("------- Loading %s -------\n", gamename);

	// now run through the search paths
	path = NULL;
	while (1)
	{
	//	path = FS_NextPath (path);
		path = FS_NextGamePath (path);
		if ( !path )
			return NULL;		// couldn't find one anywhere
	//	Com_sprintf (name, sizeof(name), "%s/%s/%s", curpath, path, gamename);
		if (path[0] == '/') {
			// Path is rooted, override curpath
			Com_sprintf (name, sizeof(name), "%s/%s", path, gamename);
		}
		else {
			Com_sprintf (name, sizeof(name), "%s/%s/%s", curpath, path, gamename);
		}
		game_library = SDL_LoadObject (name);
		if (game_library)
		{
			Com_Printf ("LoadLibrary (%s)\n", name);
			break;
		}
		else {
			Com_Printf ("%s\n", SDL_GetError());
		}
	}

	GetGameAPI = (void *)SDL_LoadFunction (game_library, "GetGameAPI");
	if ( !GetGameAPI )
	{
		Sys_UnloadGame ();		
		return NULL;
	}

	return GetGameAPI (parms);
}

/*****************************************************************************/

void Sys_AppActivate (void)
{
}

void HandleEvents(void);
void Sys_SendKeyEvents (void)
{
#ifndef DEDICATED_ONLY
	HandleEvents();
#endif

	// grab frame time 
	sys_frame_time = Sys_Milliseconds();
}

/*****************************************************************************/

#if defined (__linux__)
static char Q1_install_path[MAX_OSPATH];
static char Q1RR_install_path[MAX_OSPATH];
static char Q2RR_install_path[MAX_OSPATH];

#define QUAKE1_STEAM_APP_ID "2310"
#define QUAKE1_STEAM_LIBRARY_PATH "/steamapps/common/Quake"
#define QUAKE1RR_STEAM_LIBRARY_PATH "/steamapps/common/Quake/rerelease"
#define QUAKE2_STEAM_APP_ID "2320"
#define QUAKE2RR_STEAM_LIBRARY_PATH "/steamapps/common/Quake 2/rerelease"

/*
==================
Sys_GetSteamInstallPath
==================
*/
void Sys_GetSteamInstallPath (char *path, size_t pathSize, const char *steamLibraryPath, const char *steamAppID)
{
	char		folderPath[MAX_OSPATH];
	size_t		readLen;
	size_t		fileSize;
	char		*fileContents = NULL;
	char		*gameInstallPath = NULL;
	FILE		*libraryFoldersFile = NULL;

	if ( !path || (pathSize < 1) || !steamLibraryPath || (steamLibraryPath[0] == 0) || !steamAppID || (steamAppID[0] == 0) )
		return;

	path[0] = 0;

	Q_strncpyz (folderPath, sizeof(folderPath), va("%s/.steam/steam/steamapps/libraryfolders.vdf", getenv("HOME")));
	libraryFoldersFile = fopen(folderPath, "rb");

	if ( !libraryFoldersFile ) {
		Com_Printf ("Couldn't find Steam library folders path %s, trying next path...\n", folderPath);
		Q_strncpyz (folderPath, sizeof(folderPath), va("%s/.local/share/Steam/SteamApps/libraryfolders.vdf", getenv("HOME")));
		libraryFoldersFile = fopen(folderPath, "rb");
	}

	if ( !libraryFoldersFile ) {
		Com_Printf ("Couldn't find Steam library folders path %s\n", folderPath);
		return;
	}

	Com_Printf ("Found Steam library folders path of %s\n", folderPath);

	fseek (libraryFoldersFile, 0L, SEEK_END);
	fileSize = ftell(libraryFoldersFile);
	fseek (libraryFoldersFile, 0L, SEEK_SET);

	fileContents = Z_Malloc(fileSize + 1);

	readLen = fread (fileContents, 1, fileSize, libraryFoldersFile);
	fclose (libraryFoldersFile);
	libraryFoldersFile = NULL;

	if (readLen != fileSize) {
		Com_Printf ("Error %lu reading libraryfolders.vdf\n", errno);
		Z_Free (fileContents);
		return;
	}

//	Com_Printf ("Sys_GetSteamInstallPath: Parsing %s (size %i)...\n", folderPath, fileSize);
	gameInstallPath = Com_ParseSteamLibraryFolders (fileContents, readLen, steamLibraryPath, steamAppID);

	if ( gameInstallPath && (gameInstallPath[0] != 0) ) {	// copy off install path
		Q_strncpyz (path, pathSize, gameInstallPath);
	//	Com_Printf ("Sys_GetSteamInstallPath: Found Steam install path of %s\n", path);
	}

	Z_Free (fileContents);
}


/*
==================
Sys_InitQ1SteamInstallDir
==================
*/
void Sys_InitQ1SteamInstallDir (void)
{
	Sys_GetSteamInstallPath (Q1_install_path, sizeof(Q1_install_path), QUAKE1_STEAM_LIBRARY_PATH, QUAKE1_STEAM_APP_ID);

	if (Q1_install_path[0] != 0)
		Com_Printf ("Found Q1 Steam install path of %s\n", Q1_install_path);
	else
		Com_Printf ("Couldn't find Q1 Steam install path of %s\n", Q1_install_path);

	// TODO: if above fails, check for GOG Q1 intstall path (if available)
}


/*
==================
Sys_InitQ1RRSteamInstallDir
==================
*/
void Sys_InitQ1RRSteamInstallDir (void)
{
	Sys_GetSteamInstallPath (Q1RR_install_path, sizeof(Q1RR_install_path), QUAKE1RR_STEAM_LIBRARY_PATH, QUAKE1_STEAM_APP_ID);

	if (Q1RR_install_path[0] != 0)
		Com_Printf ("Found Q1RR Steam install path of %s\n", Q1RR_install_path);
	else
		Com_Printf ("Couldn't find Q1RR Steam install path of %s\n", Q1RR_install_path);

	// TODO: if above fails, check for GOG Q1RR intstall path (if available)
}


/*
==================
Sys_InitQ2RRSteamInstallDir
==================
*/
void Sys_InitQ2RRSteamInstallDir (void)
{
	Sys_GetSteamInstallPath (Q2RR_install_path, sizeof(Q2RR_install_path), QUAKE2RR_STEAM_LIBRARY_PATH, QUAKE2_STEAM_APP_ID);

	if (Q2RR_install_path[0] != 0)
		Com_Printf ("Found Q2RR Steam install path of %s\n", Q2RR_install_path);
	else
		Com_Printf ("Couldn't find Q2RR Steam install path of %s\n", Q2RR_install_path);

	// TODO: if above fails, check for GOG Q2RR intstall path (if available)
}


/*
==================
Sys_Q1SteamInstallDir
==================
*/
const char *Sys_Q1SteamInstallDir (void)
{
	return Q1_install_path;
}


/*
==================
Sys_Q1RRSteamInstallDir
==================
*/
const char *Sys_Q1RRSteamInstallDir (void)
{
	return Q1RR_install_path;
}


/*
==================
Sys_Q2RRSteamInstallDir
==================
*/
const char *Sys_Q2RRSteamInstallDir (void)
{
	return Q2RR_install_path;
}
#endif	// defined (__linux__)

/*****************************************************************************/

// Knightmare- adapted from DK 1.3 Linux port
const char* Sys_ExeDir (void)
{
	return exe_dir;
}

const char* Sys_PrefDir (void)
{
	return pref_dir;
}

const char* Sys_DownloadDir (void)
{
	return download_dir;
}

void Init_ExeDir (const char* argv0)
{
	char *basedir = SDL_GetBasePath();
	if (basedir == NULL)
	{
		/* Oh well. */
		memset(exe_dir, 0, sizeof(exe_dir));
		Q_snprintfz(exe_dir, sizeof(exe_dir), ".");
		return;
	}
	Q_strncpyz(exe_dir, sizeof(exe_dir), basedir);
	SDL_free(basedir);

	/* Intentionally cut off the final separator */
	exe_dir[strlen(exe_dir) - 1] = '\0';
}

void Sys_InitPrefDir (void)
{
	char *pp = SDL_GetPrefPath(NULL, SAVENAME);

	if (pp != NULL)
	{
		Q_strncpyz(pref_dir, sizeof(pref_dir), pp);
		Q_strncpyz(download_dir, sizeof(download_dir), pp);
		SDL_free(pp);

		/* Intentionally cut off the final separator */
		pref_dir[strlen(pref_dir) - 1] = '\0';
		download_dir[strlen(download_dir) - 1] = '\0';
		return;
	}

	printf("WARNING: SDL_GetPrefPath failed, defaulting to fs_basedir!\n");
	Q_strncpyz (pref_dir, sizeof(pref_dir), fs_basedir->string);
	Q_strncpyz (download_dir, sizeof(download_dir), fs_basedir->string);
}
// end Knightmare

/*****************************************************************************/

int main (int argc, char **argv)
{
	int 	time, oldtime, newtime, result;
	// Knightmare added
//	const rlim_t	minStackSize = 16L * 1024L * 1024L;	// min stack size of 16MB
//	struct rlimit	rl;

	// go back to real user for config loads
	saved_euid = geteuid();
	seteuid (getuid());

	printf ("\n");	
	printf ("========= Initialization =================\n");
	printf ("KMQuake2 -- Version %4.2fu%d %s %s\n", VERSION, VERSION_UPDATE, CPUSTRING, COMPILETYPE_STRING);
	printf ("Linux Port by QuDos\n");
	printf ("SDL2 Port by flibitijibibo\n");
	printf ("http://qudos.quakedev.com/\n");
	printf ("Compiled: "__DATE__" -- "__TIME__"\n");
	printf ("==========================================\n\n");

	// Knightmare- set minimum stack size of 16MB
/*	printf ("Checking stack size...");
	result = getrlimit(RLIMIT_STACK, &rl);
	if (result == 0)
	{
		if (rl.rlim_cur < minStackSize)
		{
			printf (" succeeded.\nIncreasing stack size to %i...", minStackSize);
			rl.rlim_cur = minStackSize;
			result = setrlimit(RLIMIT_STACK, &rl);
			if (result != 0) {
				printf (" failed!\nsetrlimit() returned result of %i.\n", result);
			}
			else {
				printf (" succeeded.\n");
			}
		}
		else {
			printf (" succeeded.  Stack size is sufficient.\n");
		}
	}
	else {
		printf (" failed!\ngetrlimit() returned result of %i.\n", result);
	} */

	// Knightmare- init exe dir
	Init_ExeDir (argv[0]);

	Qcommon_Init (argc, argv);

	fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);

	nostdout = Cvar_Get("nostdout", "0", 0);
	if (!nostdout->value) {
		fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
	//	printf ("Linux Quake II -- Version %0.3f\n", KMQUAKE2_VERSION);
	}

	oldtime = Sys_Milliseconds ();
	while (1)
	{
		// find time spent rendering last frame
		do {
			newtime = Sys_Milliseconds ();
			time = newtime - oldtime;
		} while (time < 1);
		Qcommon_Frame (time);
		oldtime = newtime;
	}

}
