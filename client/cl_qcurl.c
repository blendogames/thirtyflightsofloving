/*
===========================================================================
Copyright (C) 2018 Yamagi
Copyright (C) 2022 Knightmare

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

// cl_qcurl.c -- implementation for qcurl dynamic bindings

#include "client.h"

#ifdef USE_CURL

#ifdef USE_CURL_DLL

// qcurl function pointers
CURL *(*qcurl_easy_init)(void);
void (*qcurl_easy_cleanup)(CURL *curl);
CURLcode (*qcurl_easy_setopt)(CURL *curl, CURLoption option, ...);
CURLcode (*qcurl_easy_getinfo)(CURL *curl, CURLINFO info, ...);
const char *(*qcurl_easy_strerror)(CURLcode);

CURLM *(*qcurl_multi_init)(void);
CURLMcode (*qcurl_multi_cleanup)(CURLM *multi_handle);
CURLMcode (*qcurl_multi_add_handle)(CURLM *multi_handle, CURL *curl_handle);
CURLMcode (*qcurl_multi_remove_handle)(CURLM *multi_handle, CURL *curl_handle);
CURLMsg *(*qcurl_multi_info_read)(CURLM *multi_handle, int *msgs_in_queue);
CURLMcode (*qcurl_multi_perform)(CURLM *multi_handle, int *running_handles);

CURLcode (*qcurl_global_init)(long flags);
void (*qcurl_global_cleanup)(void);

char *(*qcurl_version)(void);
// end qcurl function pointers

qboolean	qcurl_initialized = false; // whether qcurl bindings are intialized

cvar_t		*cl_libcurl_filename;	// user-settable libcurl filename

static void *CURL_handle = NULL;	// pointer to CURL DLL

//=======================================================

/*
===============
QCURL_Init
===============
*/
qboolean QCURL_Init (void)
{
	const char *libcurl_filenames[] =
	{
#ifdef __linux__
		"libcurl.so.3",
		"libcurl.so.4",
		"libcurl-gnutls.so.3",
		"libcurl-gnutls.so.4",
		"libcurl-nss.so.3",
		"libcurl-nss.so.4",
		"libcurl.so",
#else	// __linux__
		CURL_LIBNAME,
#endif	// __linux__
		NULL
	};
	int		i;
	qboolean	lib_fail = false, ptr_fail = false;

	if (qcurl_initialized)	// already intitialized
		return true;

	cl_libcurl_filename = Cvar_Get ("cl_libcurl_filename", (char *)libcurl_filenames[0], CVAR_ARCHIVE);

	Com_Printf ("Loading library: %s...", cl_libcurl_filename->string);
	Sys_LoadLibrary (cl_libcurl_filename->string, NULL, &CURL_handle);

	if (CURL_handle != NULL) {
		Com_Printf (" succeeded.\n");
	}
	else
	{
		Com_Printf (" failed!\n");

		for (i = 0; libcurl_filenames[i] != NULL; i++)
		{
			if ( !strcmp(libcurl_filenames[i], cl_libcurl_filename->string) )	// don't try the same filename twice
				continue;

			Com_Printf ("Loading library: %s...", libcurl_filenames[i]);
			Sys_LoadLibrary (libcurl_filenames[i], NULL, &CURL_handle);

			if (CURL_handle != NULL) {
				Com_Printf (" succeeded.\n");
				Cvar_Set ("cl_libcurl_filename", (char *)libcurl_filenames[i]);
				break;
			}
			else {
				Com_Printf (" failed!\n");
				continue;
			}
		}
	}

	if ( !CURL_handle ) {
		lib_fail = true;
	}

	qcurl_easy_init				= Sys_GetProcAddress (CURL_handle, "curl_easy_init");
	qcurl_easy_cleanup			= Sys_GetProcAddress (CURL_handle, "curl_easy_cleanup");
	qcurl_easy_setopt			= Sys_GetProcAddress (CURL_handle, "curl_easy_setopt");
	qcurl_easy_getinfo			= Sys_GetProcAddress (CURL_handle, "curl_easy_getinfo");
	qcurl_easy_strerror			= Sys_GetProcAddress (CURL_handle, "curl_easy_strerror");

	qcurl_multi_init			= Sys_GetProcAddress (CURL_handle, "curl_multi_init");
	qcurl_multi_cleanup			= Sys_GetProcAddress (CURL_handle, "curl_multi_cleanup");
	qcurl_multi_add_handle		= Sys_GetProcAddress (CURL_handle, "curl_multi_add_handle");
	qcurl_multi_remove_handle	= Sys_GetProcAddress (CURL_handle, "curl_multi_remove_handle");
	qcurl_multi_info_read		= Sys_GetProcAddress (CURL_handle, "curl_multi_info_read");
	qcurl_multi_perform			= Sys_GetProcAddress (CURL_handle, "curl_multi_perform");

	qcurl_global_init			= Sys_GetProcAddress (CURL_handle, "curl_global_init");
	qcurl_global_cleanup		= Sys_GetProcAddress (CURL_handle, "curl_global_cleanup");

	qcurl_version				= Sys_GetProcAddress (CURL_handle, "curl_version");

	if ( !qcurl_easy_init )				ptr_fail = true;
	if ( !qcurl_easy_cleanup )			ptr_fail = true;
	if ( !qcurl_easy_setopt )			ptr_fail = true;
	if ( !qcurl_easy_getinfo )			ptr_fail = true;
	if ( !qcurl_easy_strerror )			ptr_fail = true;

	if ( !qcurl_multi_init )			ptr_fail = true;
	if ( !qcurl_multi_cleanup )			ptr_fail = true;
	if ( !qcurl_multi_add_handle )		ptr_fail = true;
	if ( !qcurl_multi_remove_handle )	ptr_fail = true;
	if ( !qcurl_multi_info_read )		ptr_fail = true;
	if ( !qcurl_multi_perform )			ptr_fail = true;

	if ( !qcurl_global_init )			ptr_fail = true;
	if ( !qcurl_global_cleanup )		ptr_fail = true;

	if ( !qcurl_version )				ptr_fail = true;

	if ( !lib_fail && !ptr_fail ) {
	/*	Com_Printf ("Initializing curl state...");
		qcurl_global_init (CURL_GLOBAL_NOTHING);
		Com_Printf (" done.\n"); */
		Com_Printf ("Libcurl version: %s\n", qcurl_version());

		qcurl_initialized = true;
		return true;
	}

	// If we've reached this point, we've failed to intialize the qcurl bindings.

	if (ptr_fail)
		Com_Printf ("One or more qcurl function pointers failed to mount, qcurl not initialized.\n");

	QCURL_Shutdown ();

	return false;
}


/*
===============
QCURL_Shutdown
===============
*/
void QCURL_Shutdown (void)
{
/*	if (qcurl_initialized) {
		Com_Printf ("Cleaning up curl state...");
		qcurl_global_cleanup ();
		Com_Printf (" done.\n");
	} */

	Com_Printf ("Shutting down qcurl dynamic bindings...");

	qcurl_easy_init				= NULL;
	qcurl_easy_cleanup			= NULL;
	qcurl_easy_setopt			= NULL;
	qcurl_easy_getinfo			= NULL;
	qcurl_easy_strerror			= NULL;

	qcurl_multi_init			= NULL;
	qcurl_multi_cleanup			= NULL;
	qcurl_multi_add_handle		= NULL;
	qcurl_multi_remove_handle	= NULL;
	qcurl_multi_info_read		= NULL;
	qcurl_multi_perform			= NULL;

	qcurl_global_init			= NULL;
	qcurl_global_cleanup		= NULL;

	qcurl_version				= NULL;

	if (CURL_handle != NULL) {
		Sys_FreeLibrary (CURL_handle);
		CURL_handle = NULL;
	}

	Com_Printf (" done.\n");

	qcurl_initialized = false;
}

#else	// USE_CURL_DLL

qboolean	qcurl_initialized = true;	// this is always true for static linked builds

qboolean QCURL_Init (void)
{
//	qcurl_global_init (CURL_GLOBAL_NOTHING);

	return true;
}

void QCURL_Shutdown (void)
{
//	qcurl_global_cleanup ();
}

#endif	// USE_CURL_DLL

#endif	// USE_CURL
