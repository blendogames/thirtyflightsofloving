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
#include <mntent.h>

#include <dlfcn.h>

#include "../qcommon/qcommon.h"

#include "../linux/rw_linux.h"

cvar_t *nostdout;

unsigned	sys_frame_time;

uid_t saved_euid;
qboolean stdin_active = true;

// Knightmare- added exe / pref dirs
static char exe_dir[MAX_OSPATH];
static char pref_dir[MAX_OSPATH];
static char	download_dir[MAX_OSPATH];

// =======================================================================
// General routines
// =======================================================================

void Sys_ConsoleOutput (char *string)
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
	_exit(0);
}

void Sys_Init(void)
{
#if id386
//	Sys_SetFPCW();
#endif
}

void Sys_Error (char *error, ...)
{ 
	va_list     argptr;
	char        string[1024];

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
	va_list     argptr;
	char        string[1024];

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
    int     len;
	fd_set	fdset;
    struct timeval timeout;

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
	text[len-1] = 0;    // rip off the /n and terminate

	return text;
}

// Knightmare added
void Sys_ShowConsole (qboolean show)
{
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
		dlclose (game_library);
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

	// Knightmare- changed game library name for better cohabitation
#ifdef __i386__
	const char *gamename = "kmq2gamei386.so";
#elif defined __alpha__
	const char *gamename = "kmq2gameaxp.so";
#elif defined __x86_64__
	const char *gamename = "kmq2gamex86_64.so";
#elif defined __powerpc__
	const char *gamename = "kmq2gameppc.so";
#elif defined __sparc__
	const char *gamename = "kmq2gamesparc.so";
#else
#error Unknown arch
#endif

	setreuid(getuid(), getuid());
	setegid(getgid());

	if (game_library)
		Com_Error (ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadingGame");

	getcwd(curpath, sizeof(curpath));

	Com_Printf("------- Loading %s -------\n", gamename);

	// now run through the search paths
	path = NULL;
	while (1)
	{
		path = FS_NextPath (path);
		if (!path)
			return NULL;		// couldn't find one anywhere
		sprintf (name, "%s/%s/%s", curpath, path, gamename);
		game_library = dlopen (name, RTLD_LAZY );
		if (game_library)
		{
			Com_Printf ("LoadLibrary (%s)\n",name);
			break;
		}
	}

	GetGameAPI = (void *)dlsym (game_library, "GetGameAPI");
	if (!GetGameAPI)
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

void Sys_SendKeyEvents (void)
{
#ifndef DEDICATED_ONLY
	if (KBD_Update_fp)
		KBD_Update_fp();
#endif

	// grab frame time 
	sys_frame_time = Sys_Milliseconds();
}

/*****************************************************************************/

char *Sys_GetClipboardData(void)
{
	return NULL;
}

/*****************************************************************************/

// Knightmare- adapted from DK 1.3 Linux port
const char* Sys_ExeDir(void)
{
	return exe_dir;
}

const char* Sys_PrefDir(void)
{
	return pref_dir;
}

const char* Sys_DownloadDir(void)
{
	return download_dir;
}

static void Init_ExeDir (const char* argv0)
{
#if 1
	memset(exe_dir, 0, sizeof(exe_dir));
	Q_snprintfz(exe_dir, sizeof(exe_dir), ".");
#else
	char buf[MAX_OSPATH] = {0};
	const char	*lastSlash;
	static int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
	size_t buflen;
	size_t len = lastSlash ? (lastSlash - buf) : 0;

	memset(exe_dir, 0, sizeof(exe_dir));

#ifdef __linux__
	readlink("/proc/self/exe", buf, MAX_OSPATH-1);
#elif __FreeBSD__
	sysctl(mib, sizeof(mib)/sizeof(*mib), buf, &buflen, NULL, 0);
#endif

	 if (!*buf)
	 {
		 printf("WARNING: Couldn't get path to executable, reading from argv[0]!\n");

		 if (strlen(argv0) < sizeof(buf))
		 {
			 Q_strncpyz(buf, sizeof(buf), argv0);
		 }
		 else
		 {
			 buf[0] = '\0';
		 }
	 }

	// starting at the last slash the executable name begins - we only want the path up to there
	lastSlash = strrchr(buf, '/');
	len = lastSlash ? (lastSlash - buf) : 0;
	if (lastSlash == NULL || len >= sizeof(exe_dir) || len == 0)
	{
		printf("WARNING: Couldn't get path to executable! Defaulting to \".\"!\n");
		Q_snprintfz(exe_dir, sizeof(exe_dir), ".");
	}
	else
	{
		memcpy(exe_dir, buf, len);
	}
#endif
}

static void Sys_InitPrefDir (void)
{
	char *pp = getenv("XDG_DATA_HOME");

	memset(pref_dir, 0, sizeof(pref_dir));

	if (pp == NULL)
	{
		Q_snprintfz(pref_dir, sizeof(pref_dir), "%s/.local/share/Daikatana", getenv("HOME"));
		return;
	}

	if (strlen(pp) >= sizeof(pref_dir) - 1)
	{
		printf("WARNING: $XDG_DATA_HOME contains a too long path, defaulting to installation dir!\n");
		Q_strncpyz(pref_dir, sizeof(pref_dir), exe_dir);
		Q_strncpyz (download_dir, sizeof(download_dir), exe_dir);
		return;
	}

	Q_strncpyz (pref_dir, sizeof(pref_dir), pp);
	Q_strncpyz (download_dir, sizeof(download_dir), pp);
}
// end Knightmare

/*****************************************************************************/

int main (int argc, char **argv)
{
	int 	time, oldtime, newtime;

	// go back to real user for config loads
	saved_euid = geteuid();
	seteuid(getuid());

	// Knightmare- init exe/pref dirs
	Init_ExeDir(argv[0]);
	Sys_InitPrefDir();

	Qcommon_Init(argc, argv);

	fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);

	nostdout = Cvar_Get("nostdout", "0", 0);
	if (!nostdout->value) {
		fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
//		printf ("Linux Quake -- Version %0.3f\n", LINUX_VERSION);
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

void Sys_CopyProtect(void)
{
	FILE *mnt;
	struct mntent *ent;
	char path[MAX_OSPATH];
	struct stat st;
	qboolean found_cd = false;

	static qboolean checked = false;

	if (checked)
		return;

	if ((mnt = setmntent("/etc/mtab", "r")) == NULL)
		Com_Error(ERR_FATAL, "Can't read mount table to determine mounted cd location.");

	while ((ent = getmntent(mnt)) != NULL) {
		if (strcmp(ent->mnt_type, "iso9660") == 0) {
			// found a cd file system
			found_cd = true;
			sprintf(path, "%s/%s", ent->mnt_dir, "install/data/quake2.exe");
			if (stat(path, &st) == 0) {
				// found it
				checked = true;
				endmntent(mnt);
				return;
			}
			sprintf(path, "%s/%s", ent->mnt_dir, "Install/Data/quake2.exe");
			if (stat(path, &st) == 0) {
				// found it
				checked = true;
				endmntent(mnt);
				return;
			}
			sprintf(path, "%s/%s", ent->mnt_dir, "quake2.exe");
			if (stat(path, &st) == 0) {
				// found it
				checked = true;
				endmntent(mnt);
				return;
			}
		}
	}
	endmntent(mnt);

	if (found_cd)
		Com_Error (ERR_FATAL, "Could not find a Quake2 CD in your CD drive.");
	Com_Error (ERR_FATAL, "Unable to find a mounted iso9660 file system.\n"
		"You must mount the Quake2 CD in a cdrom drive in order to play.");
}

#if 0
/*
================
Sys_MakeCodeWriteable
================
*/
void Sys_MakeCodeWriteable (unsigned int startaddr, unsigned int length)
{

	int r;
	unsigned int addr;
	int psize = getpagesize();

	addr = (startaddr & ~(psize-1)) - psize;

//	fprintf(stderr, "writable code %lx(%lx)-%lx, length=%lx\n", startaddr,
//			addr, startaddr+length, length);

	r = mprotect((char*)addr, length + startaddr - addr + psize, 7);

	if (r < 0)
    		Sys_Error("Protection change failed\n");

}

#endif
