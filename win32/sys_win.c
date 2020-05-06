/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// sys_win.h

#include "../qcommon/qcommon.h"
#include "winquake.h"
#include "resource.h"
#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <shlobj.h>
#include "../win32/conproc.h"

// [Slipyx] mingw support for _controlfp. from float.h. these would be defined
// if __STRICT_ANSI__ was undefined before including float.h, but i just copied
// out these specific definitions instead since they're the only ones used.
#if defined(__MINGW32__)
#define _MCW_PC 0x00030000
#define _PC_24 0x00020000
_CRTIMP unsigned int __cdecl __MINGW_NOTHROW _controlfp (unsigned int unNew, unsigned int unMask);
#endif // __MINGW32__

#define MINIMUM_WIN_MEMORY	0x0a00000
#define MAXIMUM_WIN_MEMORY	0x1000000

qboolean s_win95;

qboolean	s_win9X;
qboolean	s_winNT;
qboolean	s_winNT6;

int			starttime;
int			ActiveApp;
qboolean	Minimized;

static HANDLE		hinput, houtput;

unsigned	sys_msg_time;
unsigned	sys_frame_time;


static HANDLE		qwclsemaphore;

#define	MAX_NUM_ARGVS	128
int			argc;
char		*argv[MAX_NUM_ARGVS];

#define NT5_SAVEDIR "My Games/KMQuake2"
#define NT6_SAVEDIR "KMQuake2"
#define NT5_DLDIR "My Downloads/KMQuake2"
#define NT6_DLDIR "KMQuake2"
static char exe_dir[MAX_OSPATH];
static char	pref_dir[MAX_OSPATH];
static char	download_dir[MAX_OSPATH];

qboolean Detect_WinNT5orLater (void);
qboolean Detect_WinNT6orLater (void);

typedef HRESULT (WINAPI *SHGETFOLDERPATHW) (HWND hwnd, int CSIDL, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);
SHGETFOLDERPATHW fnSHGetFolderPathW = NULL;

// from DarkPlaces
const GUID qFOLDERID_SavedGames = {0x4C5C32FF, 0xBB9D, 0x43b0, {0xB5, 0xB4, 0x2D, 0x72, 0xE5, 0x4E, 0xAA, 0xA4}}; 
const GUID qFOLDERID_Downloads = {0x374DE290, 0x123F, 0x4565, {0x91, 0x64, 0x39, 0xC4, 0x92, 0x5E, 0x46, 0x7B}}; 
#define qREFKNOWNFOLDERID const GUID *
#define qKF_FLAG_CREATE 0x8000
#define qKF_FLAG_NO_ALIAS 0x1000
static HRESULT (WINAPI *fnSHGetKnownFolderPath) (qREFKNOWNFOLDERID rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);
static HRESULT (WINAPI *fnCoInitializeEx)(LPVOID pvReserved, DWORD dwCoInit);
static void (WINAPI *fnCoUninitialize)(void);
static void (WINAPI *fnCoTaskMemFree)(LPVOID pv);
// end DarkPlaces code


#ifndef NEW_DED_CONSOLE
/*
===============================================================================

DEDICATED CONSOLE

===============================================================================
*/
static char	console_text[256];
static int	console_textlen;

/*
================
Sys_InitConsole
================
*/
void Sys_InitConsole (void)
{
	if (!dedicated->value)
		return;

	if (!AllocConsole ())
		Sys_Error ("Couldn't create dedicated server console");
	hinput = GetStdHandle (STD_INPUT_HANDLE);
	houtput = GetStdHandle (STD_OUTPUT_HANDLE);
	
	// let QHOST hook in
	InitConProc (argc, argv);
}


/*
================
Sys_ConsoleInput
================
*/
char *Sys_ConsoleInput (void)
{
	INPUT_RECORD	recs[1024];
	int		dummy;
	int		ch, numread, numevents;

	if (!dedicated || !dedicated->value)
		return NULL;

	for ( ;; )
	{
		if (!GetNumberOfConsoleInputEvents (hinput, &numevents))
			Sys_Error ("Error getting # of console events");

		if (numevents <= 0)
			break;

		if (!ReadConsoleInput(hinput, recs, 1, &numread))
			Sys_Error ("Error reading console input");

		if (numread != 1)
			Sys_Error ("Couldn't read console input");

		if (recs[0].EventType == KEY_EVENT)
		{
			if (!recs[0].Event.KeyEvent.bKeyDown)
			{
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;

				switch (ch)
				{
					case '\r':
						WriteFile(houtput, "\r\n", 2, &dummy, NULL);	

						if (console_textlen)
						{
							console_text[console_textlen] = 0;
							console_textlen = 0;
							return console_text;
						}
						break;

					case '\b':
						if (console_textlen)
						{
							console_textlen--;
							WriteFile(houtput, "\b \b", 3, &dummy, NULL);	
						}
						break;

					default:
						if (ch >= ' ')
						{
							if (console_textlen < sizeof(console_text)-2)
							{
								WriteFile(houtput, &ch, 1, &dummy, NULL);	
								console_text[console_textlen] = ch;
								console_textlen++;
							}
						}
						break;
				}
			}
		}
	}
	return NULL;
}


/*
================
Sys_ConsoleOutput

Print text to the dedicated console
================
*/
void Sys_ConsoleOutput (char *string)
{
	int		dummy;
	char	text[256];

	if (!dedicated || !dedicated->value)
		return;

	if (console_textlen)
	{
		text[0] = '\r';
		memset(&text[1], ' ', console_textlen);
		text[console_textlen+1] = '\r';
		text[console_textlen+2] = 0;
		WriteFile(houtput, text, console_textlen+2, &dummy, NULL);
	}

	WriteFile(houtput, string, strlen(string), &dummy, NULL);

	if (console_textlen)
		WriteFile(houtput, console_text, console_textlen, &dummy, NULL);
}

//================================================================
#endif // NEW_DED_CONSOLE

/*
================
Sys_Sleep
================
*/
void Sys_Sleep (int msec)
{
	Sleep (msec);
}

/*
================
Sys_TickCount
================
*/
unsigned Sys_TickCount (void)
{
	return GetTickCount();
}

/*
================
Sys_SendKeyEvents

Send Key_Event calls
================
*/
void Sys_SendKeyEvents (void)
{
    MSG        msg;

	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage (&msg, NULL, 0, 0))
			Sys_Quit ();
		sys_msg_time = msg.time;
      	TranslateMessage (&msg);
      	DispatchMessage (&msg);
	}

	// grab frame time 
	sys_frame_time = timeGetTime();	// FIXME: should this be at start?
}


/*
================
Sys_GetClipboardData

================
*/
char *Sys_GetClipboardData( void )
{
	char *data = NULL;
	char *cliptext;

	if ( OpenClipboard( NULL ) != 0 )
	{
		HANDLE hClipboardData;

		if ( ( hClipboardData = GetClipboardData( CF_TEXT ) ) != 0 )
		{
			if ( ( cliptext = GlobalLock( hClipboardData ) ) != 0 ) 
			{
				data = malloc( GlobalSize( hClipboardData ) + 1 );
			//	strncpy( data, cliptext );
				Q_strncpyz( data, cliptext, GlobalSize( hClipboardData ) + 1 );
				GlobalUnlock( hClipboardData );
			}
		}
		CloseClipboard();
	}
	return data;
}


/*
===============================================================================

SYSTEM IO

===============================================================================
*/

#ifndef NEW_DED_CONSOLE
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	CL_Shutdown ();
	Qcommon_Shutdown ();

	va_start (argptr, error);
//	vsprintf (text, error, argptr);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	MessageBox(NULL, text, "Error", 0 /* MB_OK */ );

	if (qwclsemaphore)
		CloseHandle (qwclsemaphore);

// shut down QHOST hooks if necessary
	DeinitConProc ();

	exit (1);
}
#endif // NEW_DED_CONSOLE


void Sys_Quit (void)
{
	timeEndPeriod( 1 );

	CL_Shutdown();
	Qcommon_Shutdown ();
	CloseHandle (qwclsemaphore);
	if (dedicated && dedicated->value)
		FreeConsole ();

#ifdef NEW_DED_CONSOLE
	Sys_ShutdownConsole();
#else
// shut down QHOST hooks if necessary
	DeinitConProc ();
#endif

	exit (0);
}


void WinError (void)
{
	LPVOID lpMsgBuf;

	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);

	// Display the string.
	MessageBox( NULL, lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );

	// Free the buffer.
	LocalFree( lpMsgBuf );
}

//================================================================

/*
================
Sys_ScanForCD

================
*/
char *Sys_ScanForCD (void)
{
	static char	cddir[MAX_OSPATH];
	static qboolean	done;
	char		drive[4];
	FILE		*f;
	char		test[MAX_QPATH];
	qboolean	missionpack = false; // Knightmare added
	int			i; // Knightmare added

	if (done)		// don't re-check
		return cddir;

	// no abort/retry/fail errors
	SetErrorMode (SEM_FAILCRITICALERRORS);

	drive[0] = 'c';
	drive[1] = ':';
	drive[2] = '\\';
	drive[3] = 0;

	Com_Printf("\nScanning for game CD data path...");

	done = true;

	// Knightmare- check if mission pack gamedir is set
	for (i=0; i<argc; i++)
		if (!strcmp(argv[i], "game") && (i+1<argc))
		{
			if (!strcmp(argv[i+1], "rogue") || !strcmp(argv[i+1], "xatrix"))
				missionpack = true;
			break; // game parameter only appears once in command line
		}

	// scan the drives
	for (drive[0] = 'c' ; drive[0] <= 'z' ; drive[0]++)
	{
		// where activision put the stuff...
		if (missionpack) // Knightmare- mission packs have cinematics in different path
		{
			sprintf (cddir, "%sdata\\max", drive);
			sprintf (test, "%sdata\\patch\\quake2.exe", drive);
		}
		else
		{
			sprintf (cddir, "%sinstall\\data", drive);
			sprintf (test, "%sinstall\\data\\quake2.exe", drive);
		}
		f = fopen(test, "r");
		if (f)
		{
			fclose (f);
			if (GetDriveType (drive) == DRIVE_CDROM) {
				Com_Printf(" found %s\n", cddir);
				return cddir;
			}
		}
	}

	Com_Printf(" could not find %s on any CDROM drive!\n", test);

	cddir[0] = 0;
	
	return NULL;
}

//================================================================

/*
=================
Sys_DetectCPU
l33t CPU detection
Borrowed from Q2E
=================
*/
static qboolean Sys_DetectCPU (char *cpuString, int maxSize)
{
#if defined (_M_IX86) && !defined(__GNUC__)	// [Slipyx] mingw support

	char				vendor[16];
	int					stdBits, features, moreFeatures, extFeatures;
	int					family, extFamily, model, extModel;
	unsigned __int64	start, end, counter, stop, frequency;
	unsigned			speed;
	qboolean			hasMMX, hasMMXExt, has3DNow, has3DNowExt, hasSSE, hasSSE2, hasSSE3, hasSSE41, hasSSE42, hasSSE4a, hasAVX;
	SYSTEM_INFO			sysInfo;

	// Check if CPUID instruction is supported
	__try {
		__asm {
			mov eax, 0
			cpuid
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}

	// Get CPU info
	__asm {
		; // Get vendor identifier
		mov eax, 0
		cpuid
		mov dword ptr[vendor+0], ebx
		mov dword ptr[vendor+4], edx
		mov dword ptr[vendor+8], ecx
		mov dword ptr[vendor+12], 0

		; // Get standard bits and features
		mov eax, 1
		cpuid
		mov stdBits, eax
		mov moreFeatures, ecx ; // Knightmare added
		mov features, edx

		; // Check if extended functions are present
		mov extFeatures, 0
		mov eax, 80000000h
		cpuid
		cmp eax, 80000000h
		jbe NoExtFunction

		; // Get extended features
		mov eax, 80000001h
		cpuid
		mov extFeatures, edx

NoExtFunction:
	}

	// Get CPU name
	family = (stdBits >> 8) & 15;
	model = (stdBits >> 4) & 15;
	if ( (family == 15) || (family == 6) ) {
	//	extFamily = ( ((stdBits >> 20) & 15) << 4 ) + family;
		extFamily = ((stdBits >> 20) & 255) + family;
		extModel = ( ((stdBits >> 16) & 15) << 4 ) + model;
	}
	else {
		extFamily = (stdBits >> 20) & 255;
		extModel = (stdBits >> 16) & 15;
	}

	if (!Q_stricmp(vendor, "AuthenticAMD"))
	{
		strncpy(cpuString, "AMD", maxSize);
	//	Com_sprintf(cpuString, maxSize, "AMD Family %i ExtFamily %i Model %i ExtModel %i", family, extFamily, model, extModel);

		switch (family)
		{
		case 4:
			Q_strncatz(cpuString, " 5x86", maxSize);
			break;
		case 5:
			switch (model)
			{
			case 0:
			case 1:
			case 2:
			case 3:
				Q_strncatz(cpuString, " K5", maxSize);
				break;
			case 6:
			case 7:
				Q_strncatz(cpuString, " K6", maxSize);
				break;
			case 8:
				Q_strncatz(cpuString, " K6-2", maxSize);
				break;
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				Q_strncatz(cpuString, " K6-III", maxSize);
				break;
			}
			break;
		case 6: // K7 family
			switch (model)
			{
			case 1:		// 250nm core
			case 2:		// 180nm core
			case 4:		// Thunderbird core
			case 6:		// Palomino core
			case 8:		// Thoroughbred core
			case 10:	// Barton core
				Q_strncatz(cpuString, " Athlon", maxSize);
				break;
			case 3:		// Spitfire core
			case 7:		// Morgan core
				Q_strncatz(cpuString, " Duron", maxSize);
				break;
			default:
				Q_strncatz(cpuString, " K7", maxSize);
				break;
			}
			break;
		case 15: // refer to extended family
			if (extFamily == 0x0F) // K8 family
			{
				switch (model)
				{
			//	case 0:
			//	case 2:
			//	case 6:
				case 4:		// Clawhammer/Newark
				case 7:		// San Diego/Newcastle
				case 12:	// Newcastle/Albany
					Q_strncatz(cpuString, " Athlon 64", maxSize);
					break;
				case 3:		// Toledo
				case 11:	// Manchester/Brisbane
				case 15:	// Winchester/Venice
					Q_strncatz(cpuString, " Athlon 64 X2", maxSize);
					break;
				case 1:
				case 5:
					Q_strncatz(cpuString, " Athlon 64 FX / Opteron", maxSize);
					break;
				default:
					Q_strncatz(cpuString, " K8", maxSize);
					break;
				}
			}
			else if (extFamily == 0x10) // K10 family
			{
				switch (model)
				{
				case 0: // Barcelona A0-A2
				case 2: // Barcelona B0-B3
					Q_strncatz(cpuString, " Phenom / Opteron", maxSize);
					break;
				case 4: // Deneb / Shanghai
				case 10: // Thuban
					Q_strncatz(cpuString, " Phenom II / Opteron", maxSize);
					break;
				case 5: // Propus
				case 6: // Regor
					Q_strncatz(cpuString, " Athlon II", maxSize);
					break;
				default:
					Q_strncatz(cpuString, " K10", maxSize);
					break;
				}
			}
			else if (extFamily == 0x12) // Stars Fusion family
			{
				switch (model)
				{
				case 1:	// Llano
					Q_strncatz(cpuString, " A8 APU", maxSize);
					break;
				default:
					Q_strncatz(cpuString, " A series APU", maxSize);
					break;
				}
			}
			else if (extFamily == 0x14) // Bobcat family
			{
				switch (model)
				{
				case 1:	// Zacate
					Q_strncatz(cpuString, " E-350 APU", maxSize);
					break;
				default:
					Q_strncatz(cpuString, " E series APU", maxSize);
					break;
				}
			}
			else if (extFamily == 0x15) // Bulldozer family
			{
				switch (model)
				{
				case 0:
					if (extModel == 0x10)	// Trinity
						Q_strncatz(cpuString, " A series APU", maxSize);
					else if (extModel == 0x30) // Kaveri
						Q_strncatz(cpuString, " A series APU", maxSize);
					else // Zambezi
						Q_strncatz(cpuString, " FX series", maxSize);
					break;
				case 1:
					if (extModel == 1)	// Zambezi
						Q_strncatz(cpuString, " FX series", maxSize);
					else if (extModel == 0x60)	// Carrizo
						Q_strncatz(cpuString, " A series APU", maxSize);
					break;
				case 2:
					if (extModel == 2) // Zambezi / Vishera
						Q_strncatz(cpuString, " FX series", maxSize);
					break;
				case 3:
					if (extModel == 0x13)	// Richland
						Q_strncatz(cpuString, " A series APU", maxSize);
					break;
				case 5:
					if (extModel == 0x65)	// Bristol Ridge
						Q_strncatz(cpuString, " A series APU", maxSize);
					break;
				default:
					Q_strncatz(cpuString, " FX series", maxSize);
					break;
				}
			}
			else if (extFamily == 0x17)	// Ryzen Family
			{
				switch (model)
				{
				case 1:
					if (extModel == 1)	// Summit Ridge
						Q_strncatz(cpuString, " Ryzen 7/5/3 1x00", maxSize);
					else if (extModel == 0x11)	// Raven Ridge
						Q_strncatz(cpuString, " Ryzen 5/3 2x00G", maxSize);
					else if (extModel == 0x71)	// Matisse (Zen 2)
						Q_strncatz(cpuString, " Ryzen 9/7/5/3 3x00", maxSize);
					break;
				case 8:
					if (extModel == 8)	// Pinnacle Ridge
						Q_strncatz(cpuString, " Ryzen 7/5/3 2x00", maxSize);
					else if (extModel == 0x18)	// Picasso
						Q_strncatz(cpuString, " Ryzen 5/3 3x00G", maxSize);
					break;
				}
			}
			break;
		default: // unknown family
			break;
		}
	}
	else if (!Q_stricmp(vendor, "CyrixInstead"))
	{
		strncpy(cpuString, "Cyrix", maxSize);

		switch (family)
		{
		case 4:
			Q_strncatz(cpuString, " 5x86", maxSize);
			break;
		case 5:
			switch (model)
			{
			case 2:
				Q_strncatz(cpuString, " 6x86", maxSize);
				break;
			case 4:
				Q_strncatz(cpuString, " MediaGX", maxSize);
				break;
			default:
				Q_strncatz(cpuString, " 6x86 / MediaGX", maxSize);
				break;
			}
			break;
		case 6:
			Q_strncatz(cpuString, " 6x86MX", maxSize);
			break;
		default: // unknown family
			break;
		}
	}
	else if (!Q_stricmp(vendor, "CentaurHauls"))
	{
		strncpy(cpuString, "Centaur", maxSize);

		switch (family)
		{
		case 5:
			switch (model)
			{
			case 4:
				Q_strncatz(cpuString, " C6", maxSize);
				break;
			case 8:
				Q_strncatz(cpuString, " C2", maxSize);
				break;
			case 9:
				Q_strncatz(cpuString, " C3", maxSize);
				break;
			default: // unknown model
				break;
			}
			break;
		default: // unknown family
			break;
		}
	}
	else if (!Q_stricmp(vendor, "NexGenDriven"))
	{
		strncpy(cpuString, "NexGen", maxSize);

		switch (family)
		{
		case 5:
			switch (model)
			{
			case 0:
				Q_strncatz(cpuString, " Nx586 / Nx586FPU", maxSize);
				break;
			default: // unknown model
				break;
			}
			break;
		default: // unknown family
			break;
		}
	}
	else if (!Q_stricmp(vendor, "GenuineIntel"))
	{
		strncpy(cpuString, "Intel", maxSize);
	//	Com_sprintf(cpuString, maxSize, "Intel Family %i ExtFamily %i Model %i ExtModel %i", family, extFamily, model, extModel);

		switch (family)
		{
		case 5: // Pentium family
			switch (model)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 7:
			case 8:
			default:
				Q_strncatz(cpuString, " Pentium", maxSize);
				break;
			}
			break;
		case 6:
			if (model == extModel)	// P6 family, before extModel was used
			{
				switch (model)
				{
				case 0:
				case 1:
					Q_strncatz(cpuString, " Pentium Pro", maxSize);
					break;
				// Actual differentiation depends on cache settings
				case 3:		// Klamath
				case 5:		// Deschutes
					Q_strncatz(cpuString, " Pentium II", maxSize);
					break;
				case 6:
					Q_strncatz(cpuString, " Celeron", maxSize);
					break;
				// Actual differentiation depends on cache settings
				case 7:		// Katmai
				case 8:		// Coppermine
				case 10:	// Coppermine
				case 11:	// Tualatin
					Q_strncatz(cpuString, " Pentium III", maxSize);
					break;
				case 12:	// Silverthorne
					Q_strncatz(cpuString, " Atom", maxSize);
					break;
				case 9:		// Banias
				case 13:	// Dothan
					Q_strncatz(cpuString, " Pentium M", maxSize);
					break;
				case 14:	// Yonah
					Q_strncatz(cpuString, " Core", maxSize);
					break;
				case 15:	// Conroe / Kentsfield
					Q_strncatz(cpuString, " Core 2", maxSize);
					break;
				default:
					Q_strncatz(cpuString, " P6", maxSize);
					break;
				}
			}
			else	// Newer CPUs
			{
				switch (model)
				{
				case 5:
					if (extModel == 0x25)		// Clarkdale / Arrandale
						Q_strncatz(cpuString, " Core i5/i3 6xx / Core i3 5xx", maxSize);
					else if (extModel == 0x55)	// Skylake-X, Cascade Lake-X
						Q_strncatz(cpuString, " Core i9/i7 79xx / 78xx", maxSize);
					break;
				case 7:		
					if (extModel == 0x17)	// Wolfdale / Yorkfield (Penryn)
						Q_strncatz(cpuString, " Core 2", maxSize);
					else if (extModel == 0x47)	// Broadwell
						Q_strncatz(cpuString, " Core i7/i5 5xxx", maxSize);
					break;
				case 10:	
					if (extModel == 0x2A)	// Sandy Bridge
						Q_strncatz(cpuString, "  Core i7/i5/i3 2xxx", maxSize);
					else if (extModel == 0x3A)	// Ivy Bridge
						Q_strncatz(cpuString, "  Core i7/i5/i3 3xxx", maxSize);
					else if (extModel == 0x1A)	// Bloomfield
						Q_strncatz(cpuString, " Core i7 9xx", maxSize);
					break;
				case 12:
					if (extModel == 0x2C)	// Gulftown
						Q_strncatz(cpuString, " Core i7 9xx", maxSize);
					else if (extModel == 0x3C)	// Haswell
						Q_strncatz(cpuString, "  Core i7/i5/i3 4xxx", maxSize);
					else	// Silverthorne
						Q_strncatz(cpuString, " Atom", maxSize);
					break;
				case 13:	
					if (extModel == 0x2D)	// Sandy Bridge-E
						Q_strncatz(cpuString, " Core i7 39xx / 38xx", maxSize);
					break;
				case 14:
					if (extModel == 0x1E)	// Lynnfield
						Q_strncatz(cpuString, " Core i7 8xx / Core i5 7xx", maxSize);
					else if (extModel == 0x3E)	// Ivy Bridge-E
						Q_strncatz(cpuString, " Core i7 49xx / 48xx", maxSize);
					else if (extModel == 0x5E)	// Skylake
						Q_strncatz(cpuString, " Core i7/i5/i3 6xxx", maxSize);
					else if (extModel == 0x7E)	// Ice lake
						Q_strncatz(cpuString, " Core i9/i7/i5/i3 10xxx", maxSize);
					else if (extModel == 0x9E)	// Coffee Lake
						Q_strncatz(cpuString, " Core i7/i5/i3 8xxx", maxSize);
					break;
				case 15:	
					if (extModel == 0x0F)	// Conroe / Kentsfield (Merom)
						Q_strncatz(cpuString, " Core 2", maxSize);
					else if (extModel == 0x3F)	// Haswell-E
						Q_strncatz(cpuString, " Core i7 59xx / 58xx", maxSize);
					else if (extModel == 0x4F)	// Broadwell-E
						Q_strncatz(cpuString, " Core i7 69xx / 68xx", maxSize);
					else if (extModel == 0x5E)	// Kaby Lake
						Q_strncatz(cpuString, " Core i7/i5/i3 7xxx", maxSize);
					else if (extModel == 0x9E)	// Kaby Lake-X
						Q_strncatz(cpuString, " Core i7/i5 7xxx", maxSize);
					break;
				default:
					break;
				}
			}
			break;
		case 7: // Itanium family
			switch (model)
			{
			default:
				Q_strncatz(cpuString, " Itanium", maxSize);
				break;
			}
			break;
		case 15: // refer to extended family
			if (extFamily == 0x0F) // NetBurst family
			{
				switch (model)
				{
				case 0:		// Williamette
				case 1:		// Williamette
				case 2:		// Northwood
				case 3:		// Prescott
				case 4:		// Smithfield
				case 6:		// Cedar Mill / Presler
					Q_strncatz(cpuString, " Pentium 4", maxSize);
					break;
			//	case 4:		// Smithfield
			//	case 6:		// Cedar Mill / Presler
			//		Q_strncatz(cpuString, " Pentium D", maxSize);
			//		break;
				default:
					Q_strncatz(cpuString, " NetBurst", maxSize);
					break;
				}
			}
			else if (extFamily == 0x1F || extFamily == 0x2F) // Itanium 2 family
			{
				switch (model)
				{
				default:
					Q_strncatz(cpuString, " Itanium 2", maxSize);
					break;
				}
			}
			break;
		default: // unknown family
			break;
		}
	}
	else
		return false;

	// Check if RDTSC instruction is supported
	if ((features >> 4) & 1)
	{
		// Measure CPU speed
		QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);

		__asm {
			rdtsc
			mov dword ptr[start+0], eax
			mov dword ptr[start+4], edx
		}

		QueryPerformanceCounter((LARGE_INTEGER *)&stop);
		stop += frequency;

		do {
			QueryPerformanceCounter((LARGE_INTEGER *)&counter);
		} while (counter < stop);

		__asm {
			rdtsc
			mov dword ptr[end+0], eax
			mov dword ptr[end+4], edx
		}

		speed = (unsigned)((end - start) / 1000000);

		if (speed > 1000)
			Q_strncatz(cpuString, va(" %4.2f GHz", ((float)speed/1000.0f)), maxSize);
		else
			Q_strncatz(cpuString, va(" %u MHz", speed), maxSize);
	}

	// get number of logical processors
	GetSystemInfo(&sysInfo);
	if (sysInfo.dwNumberOfProcessors > 1)
		Q_strncatz(cpuString, va(" (%u logical CPUs)", sysInfo.dwNumberOfProcessors), maxSize);

	// Get extended instruction sets supported
	hasMMX = (features >> 23) & 1;
	hasMMXExt = (extFeatures >> 22) & 1;
	has3DNow = (extFeatures >> 31) & 1;
	has3DNowExt = (extFeatures >> 30) & 1;
	hasSSE = (features >> 25) & 1;
	hasSSE2 = (features >> 26) & 1;
	hasSSE3 = (moreFeatures >> 0) & 1;
	hasSSE41 = (moreFeatures >> 19) & 1;
	hasSSE42 = (moreFeatures >> 20) & 1;
	hasSSE4a = (moreFeatures >> 6) & 1;
	hasAVX = (moreFeatures >> 28) & 1;

	if (hasMMX || has3DNow || hasSSE)
	{
		Q_strncatz(cpuString, " w/", maxSize);

		if (hasMMX){
			Q_strncatz(cpuString, " MMX", maxSize);
			if (hasMMXExt)
				Q_strncatz(cpuString, "+", maxSize);
		}
		if (has3DNow){
			Q_strncatz(cpuString, " 3DNow!", maxSize);
			if (has3DNowExt)
				Q_strncatz(cpuString, "+", maxSize);
		}
		if (hasSSE){
			Q_strncatz(cpuString, " SSE", maxSize);
			if (hasSSE42)
				Q_strncatz(cpuString, "4.2", maxSize);
			else if (hasSSE41)
				Q_strncatz(cpuString, "4.1", maxSize);
			else if (hasSSE3)
				Q_strncatz(cpuString, "3", maxSize);
			else if (hasSSE2)
				Q_strncatz(cpuString, "2", maxSize);
		}
		if (hasSSE4a){
			Q_strncatz(cpuString, " SSE4a", maxSize);
		}
		if (hasAVX){
			Q_strncatz(cpuString, " AVX", maxSize);
		}
	}

	return true;

#else

//	Q_strncpyz(cpuString, "Alpha AXP", maxSize);
	Q_strncpyz(cpuString, CPUSTRING, maxSize); // [Slipyx] mingw support
	return true;
#endif
}


/*
================
Sys_Init
================
*/
void Sys_Init (void)
{
	OSVERSIONINFOEX	osInfo;
	SYSTEM_INFO		sysInfo;
	MEMORYSTATUS	memStatus;	// Knightmare added
	char			string[128];	// Knightmare added

#if 0
	// allocate a named semaphore on the client so the
	// front end can tell if it is alive

	// mutex will fail if semephore already exists
    qwclsemaphore = CreateMutex(
        NULL,         /* Security attributes */
        0,            /* owner       */
        "qwcl"); /* Semaphore name      */
	if (!qwclsemaphore)
		Sys_Error ("QWCL is already running on this system");
	CloseHandle (qwclsemaphore);

    qwclsemaphore = CreateSemaphore(
        NULL,         /* Security attributes */
        0,            /* Initial count       */
        1,            /* Maximum count       */
        "qwcl"); /* Semaphore name      */
#endif

	timeBeginPeriod( 1 );

	osInfo.dwOSVersionInfoSize = sizeof(osInfo);

	if (!GetVersionEx ((OSVERSIONINFO*) &osInfo))
		Sys_Error ("Couldn't get OS info");

	GetSystemInfo(&sysInfo);

	if (osInfo.dwMajorVersion < 4)
		Sys_Error ("KMQuake2 requires windows version 4 or greater");
	if (osInfo.dwPlatformId == VER_PLATFORM_WIN32s)
		Sys_Error ("KMQuake2 doesn't run on Win32s");
	else if ( osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
		s_win95 = true;

	// from Q2E - OS & CPU detection
	if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		
// wProductType field not supported in MSVC6
#if (_MSC_VER < 1300)
		if (osInfo.dwMajorVersion == 4) {
			Q_strncpyz (string, "Windows NT", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 5 && osInfo.dwMinorVersion == 0) {
			Q_strncpyz (string, "Windows 2000", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 5 && osInfo.dwMinorVersion == 1) {
			Q_strncpyz (string, "Windows XP", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 5 && osInfo.dwMinorVersion == 2) {
			Q_strncpyz (string, "Windows XP", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 0) {
			Q_strncpyz (string, "Windows Vista", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 1) {
			Q_strncpyz (string, "Windows 7", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 2) {
			Q_strncpyz (string, "Windows 8", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 3) {
			Q_strncpyz (string, "Windows 8.1", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 10 && osInfo.dwMinorVersion == 0) {
			Q_strncpyz (string, "Windows 10", sizeof(string));
		}
#else	// (_MSC_VER < 1300)
		if (osInfo.dwMajorVersion == 4) {
			Q_strncpyz (string, "Windows NT", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 5 && osInfo.dwMinorVersion == 0) {
			if (osInfo.wProductType == VER_NT_WORKSTATION)
				Q_strncpyz (string, "Windows 2000", sizeof(string));
			else
				Q_strncpyz (string, "Windows 2000 Server", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 5 && osInfo.dwMinorVersion == 1) {
			Q_strncpyz (string, "Windows XP", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 5 && osInfo.dwMinorVersion == 2) {
		//	if ( (osInfo.wProductType == VER_NT_WORKSTATION) && (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) )
		//		Q_strncpyz (string, "Windows XP x64 Edition", sizeof(string));
			if (osInfo.wProductType == VER_NT_WORKSTATION)
				Q_strncpyz (string, "Windows XP", sizeof(string));
			else
				Q_strncpyz (string, "Windows Server 2003", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 0) {
			if (osInfo.wProductType == VER_NT_WORKSTATION)
				Q_strncpyz (string, "Windows Vista", sizeof(string));
			else
				Q_strncpyz (string, "Windows Server 2008", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 1) {
			if (osInfo.wProductType == VER_NT_WORKSTATION)
				Q_strncpyz (string, "Windows 7", sizeof(string));
			else
				Q_strncpyz (string, "Windows Server 2008 R2", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 2) {
			if (osInfo.wProductType == VER_NT_WORKSTATION)
				Q_strncpyz (string, "Windows 8", sizeof(string));
			else
				Q_strncpyz (string, "Windows Server 2012", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 3) {
			if (osInfo.wProductType == VER_NT_WORKSTATION)
				Q_strncpyz (string, "Windows 8.1", sizeof(string));
			else
				Q_strncpyz (string, "Windows Server 2012 R2", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 10 && osInfo.dwMinorVersion == 0) {
			if (osInfo.wProductType == VER_NT_WORKSTATION)
				Q_strncpyz (string, "Windows 10", sizeof(string));
			else
				Q_strncpyz (string, "Windows Server 2016", sizeof(string));
		}
#endif	// (_MSC_VER < 1300)
		else {
			Q_strncpyz (string, va("Windows %i.%i", osInfo.dwMajorVersion, osInfo.dwMinorVersion), sizeof(string));
		}

		if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			Q_strncatz (string, " x64", sizeof(string));
		if (strlen(osInfo.szCSDVersion) > 0)
			Q_strncatz (string, va(" %s", osInfo.szCSDVersion), sizeof(string));
	}
	else if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		if (osInfo.dwMajorVersion == 4 && osInfo.dwMinorVersion == 0) {
			if (osInfo.szCSDVersion[1] == 'C' || osInfo.szCSDVersion[1] == 'B')
				Q_strncpyz (string, "Windows 95 OSR2", sizeof(string));
			else
				Q_strncpyz (string, "Windows 95", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 4 && osInfo.dwMinorVersion == 10) {
			if (osInfo.szCSDVersion[1] == 'A')
				Q_strncpyz (string, "Windows 98 SE", sizeof(string));
			else
				Q_strncpyz (string, "Windows 98", sizeof(string));
		}
		else if (osInfo.dwMajorVersion == 4 && osInfo.dwMinorVersion == 90)
			Q_strncpyz (string, "Windows ME", sizeof(string));
		else
			Q_strncpyz (string, va("Windows %i.%i", osInfo.dwMajorVersion, osInfo.dwMinorVersion), sizeof(string));
	}
	else
		Q_strncpyz (string, va("Windows %i.%i", osInfo.dwMajorVersion, osInfo.dwMinorVersion), sizeof(string));

	if (osInfo.dwBuildNumber > 0)
		Q_strncatz (string, va(", build %i", osInfo.dwBuildNumber), sizeof(string));
	Com_Printf ("OS: %s\n", string);
	Cvar_Get ("sys_osVersion", string, CVAR_NOSET|CVAR_LATCH|CVAR_SAVE_IGNORE);

	// Detect CPU
	Com_Printf("Detecting CPU... ");
	if (Sys_DetectCPU(string, sizeof(string))) {
		Com_Printf("Found %s\n", string);
		Cvar_Get("sys_cpuString", string, CVAR_NOSET|CVAR_LATCH|CVAR_SAVE_IGNORE);
	}
	else {
		Com_Printf("Unknown CPU found\n");
		Cvar_Get("sys_cpuString", "Unknown", CVAR_NOSET|CVAR_LATCH|CVAR_SAVE_IGNORE);
	}

	// Get physical memory
	GlobalMemoryStatus(&memStatus);
	strncpy(string, va("%u",memStatus.dwTotalPhys >> 20), sizeof(string));
	Com_Printf("Memory: %s MB\n", string);
	Cvar_Get("sys_ramMegs", string, CVAR_NOSET|CVAR_LATCH|CVAR_SAVE_IGNORE);
// end Q2E detection

#ifndef NEW_DED_CONSOLE
	Sys_InitConsole (); // show dedicated console, moved to function
#endif
}


/*
==============================================================================

 WINDOWS CRAP

==============================================================================
*/

/*
=================
Sys_AppActivate
=================
*/
void Sys_AppActivate (void)
{
	ShowWindow ( cl_hwnd, SW_RESTORE);
	SetForegroundWindow ( cl_hwnd );
}

/*
========================================================================

GAME DLL

========================================================================
*/

static HINSTANCE	game_library;

/*
=================
Sys_UnloadGame
=================
*/
void Sys_UnloadGame (void)
{
	if (!FreeLibrary (game_library))
		Com_Error (ERR_FATAL, "FreeLibrary failed for game library");
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
	char	*path;
	char	cwd[MAX_OSPATH];
#if defined _M_IX86
	//Knightmare- changed DLL name for better cohabitation
	const char *gamename = "kmq2gamex86.dll"; 

#ifdef NDEBUG
	const char *debugdir = "release";
#else
	const char *debugdir = "debug";
#endif

#elif defined _M_ALPHA
	const char *gamename = "kmq2gameaxp.dll";

#ifdef NDEBUG
	const char *debugdir = "releaseaxp";
#else
	const char *debugdir = "debugaxp";
#endif

#endif

	if (game_library)
		Com_Error (ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadingGame");

	// check the current debug directory first for development purposes
	_getcwd (cwd, sizeof(cwd));
	Com_sprintf (name, sizeof(name), "%s/%s/%s", cwd, debugdir, gamename);
	game_library = LoadLibrary ( name );
	if (game_library)
	{
		Com_DPrintf ("LoadLibrary (%s)\n", name);
	}
	else
	{
#ifdef DEBUG
		// check the current directory for other development purposes
		Com_sprintf (name, sizeof(name), "%s/%s", cwd, gamename);
		game_library = LoadLibrary ( name );
		if (game_library)
		{
			Com_DPrintf ("LoadLibrary (%s)\n", name);
		}
		else
#endif
		{
			// now run through the search paths
			path = NULL;
			while (1)
			{
			//	path = FS_NextPath (path);
				path = FS_NextGamePath (path);
				if (!path)
					return NULL;		// couldn't find one anywhere
				Com_sprintf (name, sizeof(name), "%s/%s", path, gamename);
				game_library = LoadLibrary (name);
				if (game_library)
				{
					Com_DPrintf ("LoadLibrary (%s)\n",name);
					break;
				}
			}
		}
	}

	GetGameAPI = (void *)GetProcAddress (game_library, "GetGameAPI");
	if (!GetGameAPI)
	{
		Sys_UnloadGame ();		
		return NULL;
	}

	return GetGameAPI (parms);
}

//=======================================================================


/*
==================
ParseCommandLine

==================
*/
void ParseCommandLine (LPSTR lpCmdLine)
{
	argc = 1;
	argv[0] = "exe";

	while (*lpCmdLine && (argc < MAX_NUM_ARGVS))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[argc] = lpCmdLine;
			argc++;

			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}
		}
	}
}


/*
==================
ReplaceBackSlashes

Replaces backslashes in a path with slashes.
==================
*/
static void ReplaceBackSlashes (char *path)
{
	char		*cur, *old;

	cur = old = path;
	if (strstr(cur, "\\") != NULL)
	{
		while (cur != NULL)
		{
			if ((cur - old) > 1) {
				*cur = '/';
			}
			old = cur;
			cur = strchr(old + 1, '\\');
		}
	}
}


/*
==================
Sys_ExeDir
==================
*/
const char *Sys_ExeDir (void)
{
	return exe_dir;
}


/*
==================
Sys_PrefDir
==================
*/
const char *Sys_PrefDir (void)
{
	return pref_dir;
}


/*
==================
Sys_DownloadDir
==================
*/
const char *Sys_DownloadDir (void)
{
	return download_dir;
}


/*
==================
Sys_InitPrefDir

Adapted from DK 1.3 source
==================
*/
void Sys_InitPrefDir (void)
{
	if ( win_use_profile_dir && win_use_profile_dir->integer )
	{
		char		profile[MAX_PATH], dlPath[MAX_PATH];
		const char	*reason = "No error!";
		const char	*reason_dl = "No error!";
		int			len, len_dl;
		qboolean	bGotNT6SavedGames=false, bGotNT6Downloads=false;

		// Use Saved Games/KMQuake2 on Win Vista and later, unless "mygames" parameter is set
		if ( Detect_WinNT6orLater() && !COM_CheckParm ("-mygames") && !COM_CheckParm ("+mygames")  )
		{
			WCHAR		*wprofile, *wDLPath;
			HMODULE		hShell32 = LoadLibrary("shell32");
			HMODULE		hOle32 = LoadLibrary("ole32");

			if ( !hShell32 || !hOle32 ) {
				reason = reason_dl = "shell32.dll or ole32.dll couldn't be loaded";
			}
			else
			{
				fnSHGetKnownFolderPath = (void *)GetProcAddress (hShell32, "SHGetKnownFolderPath");
				fnCoInitializeEx = (void *)GetProcAddress (hOle32, "CoInitializeEx");
				fnCoUninitialize = (void *)GetProcAddress (hOle32, "CoUninitialize");
				fnCoTaskMemFree =  (void *)GetProcAddress (hOle32, "CoTaskMemFree");
				if ( !fnSHGetKnownFolderPath || !fnCoInitializeEx || !fnCoUninitialize || !fnCoTaskMemFree ) {
					reason = reason_dl = "functions SHGetKnownFolderPath / CoInitializeEx / CoUninitialize / CoTaskMemFree couldn't be mapped";
				}
				else
				{
					// from DarkPlaces
					memset (profile, 0, sizeof(profile));
					fnCoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
					if (fnSHGetKnownFolderPath(&qFOLDERID_SavedGames, qKF_FLAG_CREATE | qKF_FLAG_NO_ALIAS, NULL, &wprofile) == S_OK)
					{
						memset(profile, 0, sizeof(profile));
				#if _MSC_VER >= 1400
						wcstombs_s(NULL, profile, sizeof(profile), wprofile, sizeof(profile)-1);
				#else
						wcstombs(profile, wprofile, sizeof(profile)-1);
				#endif
						fnCoTaskMemFree(wprofile);
					}
					// Also get Downloads Folder from qFOLDERID_Downloads
					if (fnSHGetKnownFolderPath(&qFOLDERID_Downloads, qKF_FLAG_CREATE | qKF_FLAG_NO_ALIAS, NULL, &wDLPath) == S_OK)
					{
						memset(dlPath, 0, sizeof(dlPath));
				#if _MSC_VER >= 1400
						wcstombs_s(NULL, dlPath, sizeof(dlPath), wDLPath, sizeof(dlPath)-1);
				#else
						wcstombs(dlPath, wDLPath, sizeof(dlPath)-1);
				#endif
						fnCoTaskMemFree(wDLPath);
					}
					fnCoUninitialize();
					// end DarkPlaces code

					len = (int)strlen(profile);
					if (len == 0) {
						reason = "SHGetKnownFolderPath()/wcstombs() returned 0 length string";
					}
					else
					{
						// Check if path is too long
						if ( (len + strlen(NT6_SAVEDIR) + 3) >= 256 ) {
							reason = "the resulting path would be too long (>= 256 chars)";
						}
						else
						{
							// Replace backslashes with slashes
							ReplaceBackSlashes (profile);
							Com_sprintf (pref_dir, sizeof(pref_dir), "%s/%s", profile, NT6_SAVEDIR);
							bGotNT6SavedGames = true;
						//	return;
						}
					}

					len_dl = (int)strlen(dlPath);
					if (len_dl == 0) {
						reason_dl = "SHGetKnownFolderPath()/wcstombs() returned 0 length string";
					}
					else
					{
						// Check if path is too long
						if ( (len_dl + strlen(NT6_DLDIR) + 3) >= 256 ) {
							reason_dl = "the resulting path would be too long (>= 256 chars)";
						}
						else
						{
							// Replace backslashes with slashes
							ReplaceBackSlashes (dlPath);
							Com_sprintf (download_dir, sizeof(download_dir), "%s/%s", dlPath, NT6_DLDIR);
							bGotNT6Downloads = true;
						//	return;
						}
					}
				}
			}
			if (bGotNT6SavedGames && bGotNT6Downloads)	// successfully got both dirs, return now
				return;

			// Output reason for why one or both dirs couldn't be found, then fall back on NT5 path
			if (!bGotNT6SavedGames)
				Com_Printf("Couldn't get PrefDir (Saved Games/KMQuake2), because %s.\n", reason);
			if (!bGotNT6Downloads)
				Com_Printf("Couldn't get DownloadDir (Downloads/KMQuake2), because %s.\n", reason_dl);
		}

		// Use My Documents/My Games/KMQuake2 on Win2K / XP
		if ( Detect_WinNT5orLater() )
		{
			WCHAR		sprofile[MAX_PATH];
			WCHAR		uprofile[MAX_PATH];
			HMODULE		hShell32 = LoadLibrary("shell32");

			if (!hShell32) {
				reason = "shell32.dll couldn't be loaded";
			}
			else
			{
				fnSHGetFolderPathW = (SHGETFOLDERPATHW)GetProcAddress (hShell32, "SHGetFolderPathW");
				if (!fnSHGetFolderPathW) {
					reason = "function SHGetFolderPathW couldn't be mapped";
				}
				else
				{
					memset (pref_dir, 0, sizeof(pref_dir));

					/* The following lines implement a horrible
					   hack to connect the UTF-16 WinAPI to the
					   ASCII Quake II. While this should work in
					   most cases, it'll fail if the "Windows to
					   DOS filename translation" is switched off.
					   In that case the function will return NULL
					   and no homedir is used. */

					// Get path to "My Documents" folder
					fnSHGetFolderPathW (NULL, CSIDL_PERSONAL, NULL, 8, uprofile);

					// Create a UTF-16 DOS path
					len = GetShortPathNameW (uprofile, sprofile, sizeof(sprofile));

					if (len == 0) {
						reason = "GetShortPathNameW() returned 0";
					}
					else
					{
						// Since the DOS path contains no UTF-16 characters, just convert it to ASCII
						len = WideCharToMultiByte (CP_ACP, 0, sprofile, -1, profile, sizeof(profile), NULL, NULL);

						if (len == 0) {
							reason = "WideCharToMultiByte() returned 0";
						}
						else
						{
							// Check if path is too long
							if ( ((len + strlen(NT5_SAVEDIR) + 3) >= 256) || ((len + strlen(NT5_DLDIR) + 3) >= 256) ) {
								reason = "The resulting path would be too long (>= 256 chars)";
							}
							else
							{
								// Replace backslashes with slashes
								ReplaceBackSlashes (profile);
								// Allow splitting of dirs from above NT6 section if only one failed
								if (!bGotNT6SavedGames)
									Com_sprintf (pref_dir, sizeof(pref_dir), "%s/%s", profile, NT5_SAVEDIR);
								if (!bGotNT6Downloads)
									Com_sprintf (download_dir, sizeof(download_dir), "%s/%s", profile, NT5_DLDIR);
								return;
							}
						}
					}
				}
			}
			Com_Printf("Couldn't get PrefDir (My Documents/My Games/KMQuake2), because %s.\n", reason);
		}
	}

	Com_sprintf (pref_dir, sizeof(pref_dir), exe_dir);
	Com_sprintf (download_dir, sizeof(download_dir), exe_dir);
}


/*
==================
Init_ExeDir
==================
*/
static void Init_ExeDir (void)
{
#if 1
	memset(exe_dir, 0, sizeof(exe_dir));
	Q_snprintfz (exe_dir, sizeof(exe_dir), ".");
#else
	char		buf[MAX_PATH];
	const char	*lastSlash;
	int			dirLen;

	memset(buf, 0, sizeof(buf));
	memset(exe_dir, 0, sizeof(exe_dir));

	GetModuleFileName (NULL, buf, sizeof(buf)-1);

	// get path up to last backslash
	lastSlash = strrchr(buf, '\\');
	if (lastSlash == NULL)
		lastSlash = strrchr(buf, '/');

	dirLen = lastSlash ? (lastSlash - buf) : 0;
	if ( lastSlash == NULL || dirLen == 0 || dirLen >= sizeof(exe_dir) ) {
		Q_snprintfz (exe_dir, sizeof(exe_dir), ".");
	}
	else {
		memcpy(exe_dir, buf, dirLen);
	}
#endif
}


/*
==================
FixWorkingDirectory
==================
*/
void FixWorkingDirectory (void)
{
	int		i;
	char	*p;
	char	curDir[MAX_PATH];

	GetModuleFileName (NULL, curDir, sizeof(curDir)-1);

	p = strrchr (curDir, '\\');
	p[0] = 0;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp (argv[i], "-nopathcheck"))
			goto skipPathCheck;

		if (!strcmp (argv[i], "-nocwdcheck"))
			return;
	}

	if (strlen(curDir) > (MAX_OSPATH - MAX_QPATH))
		Sys_Error ("Current path is too long. Please move your Quake II installation to a shorter path.");

skipPathCheck:

	SetCurrentDirectory (curDir);
}


/*
==================
Detect_WinNT5orLater
==================
*/
qboolean Detect_WinNT5orLater (void)
{
	DWORD	WinVersion;
	DWORD	WinLowByte, WinHighByte;

	WinVersion = GetVersion();
	WinLowByte = (DWORD)(LOBYTE(LOWORD(WinVersion)));
	WinHighByte = (DWORD)(HIBYTE(HIWORD(WinVersion)));

	if (WinLowByte <= 4) {
		Com_DPrintf("Windows 9x or NT 4 detected.\n");
		return false;
	}

	if (WinLowByte >= 5) {
		Com_DPrintf("Windows 5.x or later detected.\n");
		return true;
	}

	return false;
}


/*
==================
Detect_WinNT6orLater
==================
*/
qboolean Detect_WinNT6orLater (void)
{
	DWORD	WinVersion;
	DWORD	WinLowByte, WinHighByte;

	WinVersion = GetVersion();
	WinLowByte = (DWORD)(LOBYTE(LOWORD(WinVersion)));
	WinHighByte = (DWORD)(HIBYTE(HIWORD(WinVersion)));

	if (WinLowByte <= 4) {
		Com_DPrintf("Windows 9x or NT 4 detected.\n");
		return false;
	}

	if (WinLowByte == 5) {
		Com_DPrintf("Windows 5.x (Win2K / XP / Server 2003) detected.\n");
		return false;
	}

	if (WinLowByte >= 6) {
		Com_DPrintf("Windows 6.x (WinVista / 7 / 8) detected.\n");
		return true;
	}

	return false;
}


/*
==================
Sys_SetHighDPIMode

From Yamagi Quake2
==================
*/
typedef enum KMQ2_PROCESS_DPI_AWARENESS {
	KMQ2_PROCESS_DPI_UNAWARE = 0,
	KMQ2_PROCESS_SYSTEM_DPI_AWARE = 1,
	KMQ2_PROCESS_PER_MONITOR_DPI_AWARE = 2
} KMQ2_PROCESS_DPI_AWARENESS;

void Sys_SetHighDPIMode (void)
{
	HINSTANCE userDLL, shcoreDLL;

	/* For Vista, Win7 and Win8 */
	BOOL(WINAPI *SetProcessDPIAware)(void) = NULL;

	/* Win8.1 and later */
	HRESULT(WINAPI *SetProcessDpiAwareness)(KMQ2_PROCESS_DPI_AWARENESS dpiAwareness) = NULL;

	userDLL = LoadLibrary("USER32.DLL");
	if (userDLL)
	{
		SetProcessDPIAware = (BOOL(WINAPI *)(void)) GetProcAddress(userDLL,
				"SetProcessDPIAware");
	}

	shcoreDLL = LoadLibrary("SHCORE.DLL");
	if (shcoreDLL)
	{
		SetProcessDpiAwareness = (HRESULT(WINAPI *)(KMQ2_PROCESS_DPI_AWARENESS))
			GetProcAddress(shcoreDLL, "SetProcessDpiAwareness");
	}

	if (SetProcessDpiAwareness) {
		SetProcessDpiAwareness(KMQ2_PROCESS_PER_MONITOR_DPI_AWARE);
	}
	else if (SetProcessDPIAware) {
		SetProcessDPIAware();
	}
}


/*
==================
WinMain
==================
*/
HINSTANCE	global_hInstance;
HWND		hwnd_dialog; // Knightmare added

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG				msg;
	int				time, oldtime, newtime;
	char			*cddir;
	int				i; // Knightmare added
	qboolean		cdscan = false; // Knightmare added

    /* previous instances do not exist in Win32 */
    if (hPrevInstance)
        return 0;

	global_hInstance = hInstance;

	ParseCommandLine (lpCmdLine);

	Sys_SetHighDPIMode ();	// setup DPI awareness

	// r1ch: always change to our directory (ugh)
	FixWorkingDirectory ();

	Init_ExeDir ();	// Knightmare added

#ifndef NEW_DED_CONSOLE
	// Knightmare- startup logo, code from TomazQuake
	//if (!(dedicated && dedicated->value))
	{
		hwnd_dialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, NULL);
		RECT			rect; // Knightmare added

		if (hwnd_dialog)
		{
			if (GetWindowRect (hwnd_dialog, &rect))
			{
				if (rect.left > (rect.top * 2))
				{
					SetWindowPos (hwnd_dialog, 0, (rect.left/2) - ((rect.right - rect.left)/2), rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				}
			}

			ShowWindow (hwnd_dialog, SW_SHOWDEFAULT);
			UpdateWindow (hwnd_dialog);
			SetForegroundWindow (hwnd_dialog);
		}
	}
	// end Knightmare
#endif

#ifdef NEW_DED_CONSOLE // init debug console
	Sys_InitDedConsole ();
	Com_Printf("KMQ2 %4.2f %s %s %s\n", VERSION, CPUSTRING, BUILDSTRING, __DATE__);
#endif

	// Knightmare- scan for cd command line option
	for (i=0; i<argc; i++)
		if (!strcmp(argv[i], "scanforcd")) {
			cdscan = true;
			break;
		}

	// if we find the CD, add a +set cddir xxx command line
	if (cdscan)
	{
		cddir = Sys_ScanForCD ();
		if (cddir && argc < MAX_NUM_ARGVS - 3)
		{
			int		i;

			// don't override a cddir on the command line
			for (i=0 ; i<argc ; i++)
				if (!strcmp(argv[i], "cddir"))
					break;
			if (i == argc)
			{
				argv[argc++] = "+set";
				argv[argc++] = "cddir";
				argv[argc++] = cddir;
			}
		}
	}

	Qcommon_Init (argc, argv);
	oldtime = Sys_Milliseconds ();

    /* main window message loop */
	while (1)
	{
		// if at a full screen console, don't update unless needed
		if (Minimized || (dedicated && dedicated->value) )
		{
			Sleep (1);
		}

		while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage (&msg, NULL, 0, 0))
				Com_Quit ();
			sys_msg_time = msg.time;
			TranslateMessage (&msg);
   			DispatchMessage (&msg);
		}

		// DarkOne's CPU usage fix
		while (1)
		{
			newtime = Sys_Milliseconds();
			time = newtime - oldtime;
			if (time > 0) break;
			Sleep(0); // may also use Sleep(1); to free more CPU, but it can lower your fps
		}
		/*do
		{
			newtime = Sys_Milliseconds ();
			time = newtime - oldtime;
		} while (time < 1);*/
		//	Con_Printf ("time:%5.2f - %5.2f = %5.2f\n", newtime, oldtime, time);

		//	_controlfp( ~( _EM_ZERODIVIDE /*| _EM_INVALID*/ ), _MCW_EM );
		_controlfp( _PC_24, _MCW_PC );
		Qcommon_Frame (time);

		oldtime = newtime;
	}

	// never gets here
    return TRUE;
}
