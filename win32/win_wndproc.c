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

// win_windproc.c - WIN32 helper functions

#include <assert.h>
#include <float.h>

#include "..\client\client.h"
#include "winquake.h"
//#include "zmouse.h"

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  // message that will be supported by the OS 
#endif

// Backslash's Mouse buttons 4 & 5 support
/* These are #ifdefed out for non-Win2K in the February 2001 version of
   MS's platform SDK, but we need them for compilation. . . */
#ifndef WM_XBUTTONDOWN
   #define WM_XBUTTONDOWN      0x020B
   #define WM_XBUTTONUP      0x020C
#endif
#ifndef MK_XBUTTON1
   #define MK_XBUTTON1         0x0020
   #define MK_XBUTTON2         0x0040
#endif
// end Mouse buttons 4 & 5 support

// Logitech mouse support
//#define WM_MWHOOK (WM_USER + 1)
//int MW_Hook_Message (long buttons);
// end Logitech mouse support

HWND        cl_hwnd;            // Main window handle for life of program

LONG WINAPI MainWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

static UINT MSH_MOUSEWHEEL;
static qboolean s_alttab_disabled;
extern	unsigned	sys_msg_time;
extern	qboolean	s_win9X;
extern	qboolean	s_winNT;

//==========================================================================

// [Slipyx] mingw support
#ifdef __GNUC__
#define SPI_SCREENSAVERRUNNING SPI_SETSCREENSAVERRUNNING
#endif // __GNUC__

static void WIN_DisableAltTab (void)
{
	if ( s_alttab_disabled )
		return;

	if (s_winNT)
	{
		RegisterHotKey( 0, 0, MOD_ALT, VK_TAB );
		RegisterHotKey( 0, 1, MOD_ALT, VK_RETURN );
	}
	else
	{
		BOOL old;

		SystemParametersInfo( SPI_SCREENSAVERRUNNING, 1, &old, 0 );
	}
	s_alttab_disabled = true;
}

static void WIN_EnableAltTab (void)
{
	if ( s_alttab_disabled )
	{
		if (s_winNT)
		{
			UnregisterHotKey( 0, 0 );
			UnregisterHotKey( 0, 1 );
		}
		else
		{
			BOOL old;

			SystemParametersInfo( SPI_SCREENSAVERRUNNING, 0, &old, 0 );
		}
		s_alttab_disabled = false;
	}
}

void WIN_SetAltTab (void)
{
	if (!win_noalttab->modified)
		return;

	if (win_noalttab->integer)
		WIN_DisableAltTab ();
	else
		WIN_EnableAltTab ();
	win_noalttab->modified = false;
}

void WndActivate (BOOL fActive, BOOL minimize)
{
	Minimized = minimize;

	Key_ClearStates();

	// we don't want to act like we're active if we're minimized
	if (fActive && !Minimized)
		ActiveApp = true;
	else
		ActiveApp = false;

	// minimize/restore mouse-capture on demand
	if (!ActiveApp)
	{
		IN_Activate (false);
		CDAudio_Activate (false);
		S_Activate (false);

	//	if ( win_noalttab->value )
		if ( win_noalttab->integer )
		{
			WIN_EnableAltTab();
		}
	}
	else
	{
		IN_Activate (true);
		CDAudio_Activate (true);
		S_Activate (true);
	//	if ( win_noalttab->value )
		if ( win_noalttab->integer )
		{
			WIN_DisableAltTab();
		}
	}
}

/*
====================
MainWndProc

main window procedure
====================
*/
LONG WINAPI MainWndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
	LONG			lRet = 0;

	if ( uMsg == MSH_MOUSEWHEEL )
	{
		if ( ( ( int ) wParam ) > 0 )
		{
			Key_Event( K_MWHEELUP, true, sys_msg_time );
			Key_Event( K_MWHEELUP, false, sys_msg_time );
		}
		else
		{
			Key_Event( K_MWHEELDOWN, true, sys_msg_time );
			Key_Event( K_MWHEELDOWN, false, sys_msg_time );
		}
        return DefWindowProc (hWnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
	case WM_MOUSEWHEEL:
		//
		// this chunk of code theoretically only works under NT4 and Win98
		// since this message doesn't exist under Win95
		//
		if ( ( short ) HIWORD( wParam ) > 0 )
		{
			Key_Event( K_MWHEELUP, true, sys_msg_time );
			Key_Event( K_MWHEELUP, false, sys_msg_time );
		}
		else
		{
			Key_Event( K_MWHEELDOWN, true, sys_msg_time );
			Key_Event( K_MWHEELDOWN, false, sys_msg_time );
		}
		break;

	case WM_HOTKEY:
		return 0;

	case WM_CREATE:
		cl_hwnd = hWnd;

		MSH_MOUSEWHEEL = RegisterWindowMessage("MSWHEEL_ROLLMSG"); 
        return DefWindowProc (hWnd, uMsg, wParam, lParam);

	case WM_PAINT:
        return DefWindowProc (hWnd, uMsg, wParam, lParam);

	case WM_DESTROY:
		// let sound and input know about this?
		cl_hwnd = NULL;
        return DefWindowProc (hWnd, uMsg, wParam, lParam);

	case WM_ACTIVATE:
		{
			int	fActive, fMinimized;

			// KJB: Watch this for problems in fullscreen modes with Alt-tabbing.
			fActive = LOWORD(wParam);
			fMinimized = (BOOL) HIWORD(wParam);

			WndActivate( fActive != WA_INACTIVE, fMinimized);

			if ( kmgl_active )
				GLimp_AppActivate ( !( fActive == WA_INACTIVE ) );
		}
        return DefWindowProc (hWnd, uMsg, wParam, lParam);

	case WM_MOVE:
		{
			int		xPos, yPos;
			RECT r;
			int		style;

			if (!vid_fullscreen->integer)
			{
				xPos = (short) LOWORD(lParam);    // horizontal position 
				yPos = (short) HIWORD(lParam);    // vertical position 

				r.left   = 0;
				r.top    = 0;
				r.right  = 1;
				r.bottom = 1;

				style = GetWindowLongPtr( hWnd, GWL_STYLE );
				AdjustWindowRect( &r, style, FALSE );

				Cvar_SetValue( "vid_xpos", xPos + r.left);
				Cvar_SetValue( "vid_ypos", yPos + r.top);
				vid_xpos->modified = false;
				vid_ypos->modified = false;
				if (ActiveApp)
					IN_Activate (true);
			}
		}
        return DefWindowProc (hWnd, uMsg, wParam, lParam);

// this is complicated because Win32 seems to pack multiple mouse events into
// one update sometimes, so we always check all states and look for events
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_XBUTTONDOWN:// Backslash's imouse explorer buttons
	case WM_XBUTTONUP:	// Backslash's imouse explorer buttons 
	case WM_MOUSEMOVE:
	// Logitech mouse support
	//case WM_MWHOOK:
		{
			int	temp;

			temp = 0;

			if (wParam & MK_LBUTTON)
				temp |= 1;

			if (wParam & MK_RBUTTON)
				temp |= 2;

			if (wParam & MK_MBUTTON)
				temp |= 4;
			// Mouse buttons 4 & 5 support
			if (wParam & MK_XBUTTON1)
				temp |= 8;

			if (wParam & MK_XBUTTON2)
				temp |= 16;
			// end Mouse buttons 4 & 5 support

			// Logitech mouse support
			//if (uMsg == WM_MWHOOK)
			//	temp |= MW_Hook_Message (lParam);

			IN_MouseEvent (temp);
		}
		break;

	/*case WM_SYSCOMMAND:
		if ( wParam == SC_SCREENSAVE )
			return 0;
        return DefWindowProc (hWnd, uMsg, wParam, lParam);*/
	// Idle's fix
	case WM_SYSCOMMAND:
		switch (wParam & 0xfffffff0)	// bitshifter's fix for screensaver bug
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0;
		case SC_CLOSE:
			CL_Quit_f ();
		//case SC_MAXIMIZE:
		//	Cvar_SetValue ("vid_fullscreen", 1);
		//	return 0;
		}
		return DefWindowProc (hWnd, uMsg, wParam, lParam);

	case WM_SYSKEYDOWN:
		if ( wParam == 13 )
		{
			if ( vid_fullscreen )
			{
				Cvar_SetValue( "vid_fullscreen", !vid_fullscreen->integer );
			}
			return 0;
		}
		// fall through
	case WM_KEYDOWN:
		Key_Event (IN_MapKey( lParam ), true, sys_msg_time);
		break;

	case WM_SYSKEYUP:
	case WM_KEYUP:
		Key_Event (IN_MapKey( lParam ), false, sys_msg_time);
		break;

	case MM_MCINOTIFY:
		{
			LONG CDAudio_MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
			lRet = CDAudio_MessageHandler (hWnd, uMsg, wParam, lParam);
		}
		break;

	default:	// pass all unhandled messages to DefWindowProc
        return DefWindowProc (hWnd, uMsg, wParam, lParam);
    }

    /* return 0 if handled message, 1 if not */
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}
