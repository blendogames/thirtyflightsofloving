/*
===========================================================================
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

// qcurl.h -- header for qcurl dynamic bindings

#ifdef USE_CURL

#define USE_CURL_DLL


#ifndef USE_CURL_DLL

#ifdef _WIN32
#define CURL_STATICLIB
#define CURL_HIDDEN_SYMBOLS
#define CURL_EXTERN_SYMBOL
#define CURL_CALLING_CONVENTION __cdecl
#endif	// _WIN32

#define CURL_STATICLIB

#endif	// USE_CURL_DLL

#include "../include/curl/curl.h"

#define CURL_ERROR(x)	qcurl_easy_strerror(x)
//#endif

#ifdef _WIN32
#if defined (_M_X64) || defined (_M_AMD64) || defined (__x86_64__)
#ifdef _DEBUG
#define	CURL_LIBNAME	"shared_libs/libcurlx64d.dll"
#else
#define	CURL_LIBNAME	"shared_libs/libcurlx64.dll"
#endif	// _DEBUG
#else	// _M_X64
#ifdef _DEBUG
#define	CURL_LIBNAME	"shared_libs/libcurlx86d.dll"
#else
#define	CURL_LIBNAME	"shared_libs/libcurlx86.dll"
#endif	// _DEBUG
#endif	// _M_X64
#elif defined(__APPLE__) || (MACOSX)
#define	CURL_LIBNAME	"libcurl.dylib"
#else
#define	CURL_LIBNAME	"libcurl.so"
#endif

#ifdef USE_CURL_DLL

// qcurl function pointers
extern CURL *(*qcurl_easy_init)(void);
extern void (*qcurl_easy_cleanup)(CURL *curl);
extern CURLcode (*qcurl_easy_setopt)(CURL *curl, CURLoption option, ...);
extern CURLcode (*qcurl_easy_getinfo)(CURL *curl, CURLINFO info, ...);
extern const char *(*qcurl_easy_strerror)(CURLcode);

extern CURLM *(*qcurl_multi_init)(void);
extern CURLMcode (*qcurl_multi_cleanup)(CURLM *multi_handle);
extern CURLMcode (*qcurl_multi_add_handle)(CURLM *multi_handle, CURL *curl_handle);
extern CURLMcode (*qcurl_multi_remove_handle)(CURLM *multi_handle, CURL *curl_handle);
extern CURLMsg *(*qcurl_multi_info_read)(CURLM *multi_handle, int *msgs_in_queue);
extern CURLMcode (*qcurl_multi_perform)(CURLM *multi_handle, int *running_handles);

extern CURLcode (*qcurl_global_init)(long flags);
extern void (*qcurl_global_cleanup)(void);

extern char *(*qcurl_version)(void);

#else	// USE_CURL_DLL

// qcurl function macros
#define qcurl_easy_init				curl_easy_init
#define qcurl_easy_cleanup			curl_easy_cleanup
#define qcurl_easy_setopt			curl_easy_setopt
#define qcurl_easy_getinfo			curl_easy_getinfo
#define qcurl_easy_strerror			curl_easy_strerror

#define qcurl_multi_init			curl_multi_init
#define qcurl_multi_cleanup			curl_multi_cleanup	
#define qcurl_multi_add_handle		curl_multi_add_handle
#define qcurl_multi_remove_handle	curl_multi_remove_handle
#define qcurl_multi_info_read		curl_multi_info_read
#define qcurl_multi_perform			curl_multi_perform	

#define qcurl_global_init			curl_global_init
#define qcurl_global_cleanup		curl_global_cleanup

#define qcurl_version				curl_version

#endif	// USE_CURL_DLL

extern qboolean	qcurl_initialized; // whether qcurl bindings are intialized

qboolean QCURL_Init (void);
void QCURL_Shutdown (void);

#endif	// USE_CURL
