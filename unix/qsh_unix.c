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

/* For mremap() - must be before sys/mman.h include! */
#if defined(__linux__) && !defined(_GNU_SOURCE)
 #define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <ctype.h>

#include "glob.h"

#include "../qcommon/qcommon.h"

#if defined(__FreeBSD__)
#include <machine/param.h>
#endif

//===============================================================================

// Hunk implementation taken from yquake2

byte *membase;
size_t maxhunksize;
size_t curhunksize;

void *Hunk_Begin (size_t maxsize)
{
	/* reserve a huge chunk of memory, but don't commit any yet */
	/* plus 32 bytes for cacheline */
	maxhunksize = maxsize + sizeof(size_t) + 32;
	curhunksize = 0;
	int flags = MAP_PRIVATE | MAP_ANONYMOUS;
	int prot = PROT_READ | PROT_WRITE;

#if defined(MAP_ALIGNED_SUPER)
	const size_t hgpagesize = 1UL<<21;
	size_t page_size = sysconf(_SC_PAGESIZE);

	/* Archs supported has 2MB for super pages size */
	if (maxhunksize >= hgpagesize)
	{
		maxhunksize = (maxhunksize & ~(page_size-1)) + page_size;
		flags |= MAP_ALIGNED_SUPER;
	}
#endif

#if defined(PROT_MAX)
	/* For now it is FreeBSD exclusif but could possibly be extended
	   to other like DFBSD for example */
	prot |= PROT_MAX(prot);
#endif

	membase = mmap(0, maxhunksize, prot,
			flags, -1, 0);

	if ((membase == NULL) || (membase == (byte *)-1))
	{
		Sys_Error("unable to virtual allocate %d bytes", maxsize);
	}

	*((size_t *)membase) = curhunksize;

	return membase + sizeof(size_t);
}

void *Hunk_Alloc (size_t size)
{
	byte *buf;

	/* round to cacheline */
	size = (size + 31) & ~31;

	if (curhunksize + size > maxhunksize)
	{
		Sys_Error("Hunk_Alloc overflow");
	}

	buf = membase + sizeof(size_t) + curhunksize;
	curhunksize += size;
	return buf;
}

size_t Hunk_End (void)
{
	byte *n = NULL;

#if defined(__linux__)
	n = (byte *)mremap(membase, maxhunksize, curhunksize + sizeof(size_t), 0);
#elif defined(__NetBSD__)
	n = (byte *)mremap(membase, maxhunksize, NULL, curhunksize + sizeof(size_t), 0);
#else
 #ifndef round_page
 size_t page_size = sysconf(_SC_PAGESIZE);
 #define round_page(x) ((((size_t)(x)) + page_size-1) & ~(page_size-1))
 #endif

	size_t old_size = round_page(maxhunksize);
	size_t new_size = round_page(curhunksize + sizeof(size_t));

	if (new_size > old_size)
	{
		/* Can never happen. If it happens something's very wrong. */
		n = 0;
	}
	else if (new_size < old_size)
	{
		/* Hunk is to big, we need to shrink it. */
		n = munmap(membase + new_size, old_size - new_size) + membase;
	}
	else
	{
		/* No change necessary. */
		n = membase;
	}
#endif

	if (n != membase)
	{
		Sys_Error("Hunk_End: Could not remap virtual block (%d)", errno);
	}

	*((size_t *)membase) = curhunksize + sizeof(size_t);

	return curhunksize;
}

void Hunk_Free (void *base)
{
	if (base)
	{
		byte *m;

		m = ((byte *)base) - sizeof(size_t);

		if (munmap(m, *((size_t *)m)))
		{
			Sys_Error("Hunk_Free: munmap failed (%d)", errno);
		}
	}
}

//===============================================================================


/*
================
Sys_Milliseconds
================
*/
int curtime;
int Sys_Milliseconds (void)
{
	struct timeval tp;
	struct timezone tzp;
	static int		secbase;

	gettimeofday(&tp, &tzp);
	
	if (!secbase)
	{
		secbase = tp.tv_sec;
		return tp.tv_usec/1000;
	}

	curtime = (tp.tv_sec - secbase)*1000 + tp.tv_usec/1000;
	
	return curtime;
}

void Sys_Mkdir (const char *path)
{
    mkdir (path, 0777);
}

//
// added from Q2E
//
void Sys_Rmdir (const char *path)
{
    rmdir (path);
}

/*
=================
Sys_GetCurrentDirectory
=================
*/
char *Sys_GetCurrentDirectory (void)
{
	static char	dir[MAX_OSPATH];

	if (!getcwd(dir, sizeof(dir)))
		Sys_Error("Couldn't get current working directory");

	return dir;
}

char *strlwr (char *s)
{
	char *p = s;
	while (*s) {
		*s = tolower(*s);
		s++;
	}
	return p;
}

//============================================

static	char	findbase[MAX_OSPATH];
static	char	findpath[MAX_OSPATH];
static	char	findpattern[MAX_OSPATH];
static	DIR		*fdir;

static qboolean CompareAttributes(char *path, char *name,
	unsigned musthave, unsigned canthave )
{
	struct stat st;
	char fn[MAX_OSPATH];

// . and .. never match
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		return false;

	return true;

	if (stat(fn, &st) == -1)
		return false; // shouldn't happen

	if ( ( st.st_mode & S_IFDIR ) && ( canthave & SFF_SUBDIR ) )
		return false;

	if ( ( musthave & SFF_SUBDIR ) && !( st.st_mode & S_IFDIR ) )
		return false;

	return true;
}

char *Sys_FindFirst (char *path, unsigned musthave, unsigned canhave)
{
	struct dirent *d;
	char *p;

	if (fdir)
		Sys_Error ("Sys_BeginFind without close");

//	COM_FilePath (path, findbase, sizeof(filebase));
//	strncpy(findbase, path);
	Q_strncpyz(findbase, sizeof(findbase), path);

	if ((p = strrchr(findbase, '/')) != NULL) {
		*p = 0;
	//	strncpy(findpattern, p + 1);
		Q_strncpyz(findpattern, sizeof(findpattern), p + 1);
	} else
	//	strncpy(findpattern, "*");
		Q_strncpyz(findpattern, sizeof(findpattern), "*");

	if (strcmp(findpattern, "*.*") == 0)
	//	strncpy(findpattern, "*");
		Q_strncpyz(findpattern, sizeof(findpattern), "*");
	
	if ((fdir = opendir(findbase)) == NULL)
		return NULL;
	while ((d = readdir(fdir)) != NULL) {
		if (!*findpattern || glob_match(findpattern, d->d_name)) {
//			if (*findpattern)
//				printf("%s matched %s\n", findpattern, d->d_name);
			if (CompareAttributes(findbase, d->d_name, musthave, canhave)) {
				Com_sprintf (findpath, sizeof(findpath), "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}
	return NULL;
}

char *Sys_FindNext (unsigned musthave, unsigned canhave)
{
	struct dirent *d;

	if (fdir == NULL)
		return NULL;
	while ((d = readdir(fdir)) != NULL) {
		if (!*findpattern || glob_match(findpattern, d->d_name)) {
//			if (*findpattern)
//				printf("%s matched %s\n", findpattern, d->d_name);
			if (CompareAttributes(findbase, d->d_name, musthave, canhave)) {
				Com_sprintf (findpath, sizeof(findpath), "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}
	return NULL;
}

void Sys_FindClose (void)
{
	if (fdir != NULL)
		closedir(fdir);
	fdir = NULL;
}


//============================================

