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
// Main windowed and fullscreen graphics interface module. This module
// is used for both the software and OpenGL rendering versions of the
// Quake refresh engine.
#include <assert.h>
#include <float.h>

#include "..\client\client.h"
#include "winquake.h"
//#include "zmouse.h"


#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  // message that will be supported by the OS 
#endif

static UINT MSH_MOUSEWHEEL;

//PGM
int	vidref_val;
//PGM

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

// Console variables that we need to access from this module
cvar_t		*scanforcd; // Knightmare- just here to enable command line option without error
cvar_t		*win_noalttab;
cvar_t		*win_alttab_restore_desktop;	// Knightmare- whether to restore desktop resolution on alt-tab
cvar_t		*vid_gamma;
cvar_t		*vid_ref;			// Name of Refresh DLL loaded
cvar_t		*vid_xpos;			// X coordinate of window position
cvar_t		*vid_ypos;			// Y coordinate of window position
cvar_t		*vid_fullscreen;
cvar_t		*r_customwidth;
cvar_t		*r_customheight;

// Global variables used internally by this module
viddef_t	viddef;				// global video state; used by other modules
qboolean	kmgl_active = 0;

HWND        cl_hwnd;            // Main window handle for life of program

#define VID_NUM_MODES ( sizeof( vid_modes ) / sizeof( vid_modes[0] ) )

LONG WINAPI MainWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

static qboolean s_alttab_disabled;
extern	unsigned	sys_msg_time;

/*
** WIN32 helper functions
*/
extern qboolean s_win95;

static void WIN_DisableAltTab( void )
{
	if ( s_alttab_disabled )
		return;

	if ( s_win95 )
	{
		BOOL old;

		SystemParametersInfo( SPI_SCREENSAVERRUNNING, 1, &old, 0 );
	}
	else
	{
		RegisterHotKey( 0, 0, MOD_ALT, VK_TAB );
		RegisterHotKey( 0, 1, MOD_ALT, VK_RETURN );
	}
	s_alttab_disabled = true;
}

static void WIN_EnableAltTab( void )
{
	if ( s_alttab_disabled )
	{
		if ( s_win95 )
		{
			BOOL old;

			SystemParametersInfo( SPI_SCREENSAVERRUNNING, 0, &old, 0 );
		}
		else
		{
			UnregisterHotKey( 0, 0 );
			UnregisterHotKey( 0, 1 );
		}

		s_alttab_disabled = false;
	}
}

/*
==========================================================================

DLL GLUE

==========================================================================
*/

#define	MAXPRINTMSG	8192 // was 4096
void VID_Printf (int print_level, char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	static qboolean	inupdate;
	
	va_start (argptr, fmt);
//	vsprintf (msg, fmt, argptr);
	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	if (print_level == PRINT_ALL)
	{
		Com_Printf ("%s", msg);
	}
	else if ( print_level == PRINT_DEVELOPER )
	{
		Com_DPrintf ("%s", msg);
	}
	else if ( print_level == PRINT_ALERT )
	{
		MessageBox( 0, msg, "PRINT_ALERT", MB_ICONWARNING );
		OutputDebugString( msg );
	}
}

void VID_Error (int err_level, char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	static qboolean	inupdate;
	
	va_start (argptr, fmt);
//	vsprintf (msg, fmt,argptr);
	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	Com_Error (err_level,"%s", msg);
}

//==========================================================================

byte        scantokey[128] = 
					{ 
//  0           1       2			3			4       5				6			7 
//  8           9       A			B			C       D				E			F 
	0,			27,     '1',		'2',		'3',    '4',			'5',		'6', 
	'7',		'8',    '9',		'0',		'-',    '=',			K_BACKSPACE,9,		    // 0 
	'q',		'w',    'e',		'r',		't',    'y',			'u',		'i', 
	'o',		'p',    '[',		']',		13 ,    K_CTRL,			'a',		's',		// 1 
	'd',		'f',    'g',		'h',		'j',    'k',			'l',		';', 
	'\'',		'`',    K_SHIFT,	'\\',		'z',    'x',			'c',		'v',		// 2 
	'b',		'n',    'm',		',',		'.',    '/',			K_SHIFT,	K_KP_MULT,	// KP_MULT was '*'
	K_ALT,		' ',	K_CAPSLOCK,	K_F1,		K_F2,	K_F3,			K_F4,		K_F5,	    // 3 
	K_F6,		K_F7,	K_F8,		K_F9,		K_F10,  K_PAUSE,		K_SCROLLOCK,K_HOME, 
	K_UPARROW,	K_PGUP,	K_KP_MINUS,	K_LEFTARROW,K_KP_5,	K_RIGHTARROW,	K_KP_PLUS,	K_END,      // 4 
	K_DOWNARROW,K_PGDN,	K_INS,		K_DEL,		0,		0,				0,			K_F11, 
	K_F12,		0,		0,			0,			0,		0,				0,			0,			// 5
	0,			0,		0,			0,			0,		0,				0,			0, 
	0,			0,		0,			0,			0,		0,				0,			0,			// 6 
	0,			0,		0,			0,			0,		0,				0,			0, 
	0,			0,		0,			0,			0,		0,				0,			0			// 7 
}; 

/*
=======
MapKey

Map from windows to quake keynums
=======
*/
int MapKey (int key)
{
	int result;
	int modified = ( key >> 16 ) & 255;
	qboolean is_extended = false;

	if ( modified > 127)
		return 0;

	if ( key & ( 1 << 24 ) )
		is_extended = true;

	result = scantokey[modified];

	if ( !is_extended )
	{
		switch ( result )
		{
		case K_HOME:
			return K_KP_HOME;
		case K_UPARROW:
			return K_KP_UPARROW;
		case K_PGUP:
			return K_KP_PGUP;
		case K_LEFTARROW:
			return K_KP_LEFTARROW;
		case K_RIGHTARROW:
			return K_KP_RIGHTARROW;
		case K_END:
			return K_KP_END;
		case K_DOWNARROW:
			return K_KP_DOWNARROW;
		case K_PGDN:
			return K_KP_PGDN;
		case K_INS:
			return K_KP_INS;
		case K_DEL:
			return K_KP_DEL;
		default:
			return result;
		}
	}
	else
	{
		switch ( result )
		{
		case 0x0D:
			return K_KP_ENTER;
		case 0x2F:
			return K_KP_SLASH;
		case 0xAF:
			return K_KP_PLUS;
		case K_PAUSE:
			return K_NUMLOCK;
		}
		return result;
	}
}

void AppActivate(BOOL fActive, BOOL minimize)
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

		if ( win_noalttab->value )
		{
			WIN_EnableAltTab();
		}
	}
	else
	{
		IN_Activate (true);
		CDAudio_Activate (true);
		S_Activate (true);
		if ( win_noalttab->value )
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

			AppActivate( fActive != WA_INACTIVE, fMinimized);

			if ( kmgl_active )
				GLimp_AppActivate ( !( fActive == WA_INACTIVE ) );
		}
        return DefWindowProc (hWnd, uMsg, wParam, lParam);

	case WM_MOVE:
		{
			int		xPos, yPos;
			RECT r;
			int		style;

			if (!vid_fullscreen->value)
			{
				xPos = (short) LOWORD(lParam);    // horizontal position 
				yPos = (short) HIWORD(lParam);    // vertical position 

				r.left   = 0;
				r.top    = 0;
				r.right  = 1;
				r.bottom = 1;

			//	style = GetWindowLong( hWnd, GWL_STYLE );
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
				Cvar_SetValue( "vid_fullscreen", !vid_fullscreen->value );
			}
			return 0;
		}
		// fall through
	case WM_KEYDOWN:
		Key_Event( MapKey( lParam ), true, sys_msg_time);
		break;

	case WM_SYSKEYUP:
	case WM_KEYUP:
		Key_Event( MapKey( lParam ), false, sys_msg_time);
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

/*
============
VID_Restart_f

Console command to re-start the video mode and refresh DLL. We do this
simply by setting the modified flag for the vid_ref variable, which will
cause the entire video mode and refresh DLL to be reset on the next frame.
============
*/
void VID_Restart_f (void)
{
	vid_ref->modified = true;
}

void VID_Front_f( void )
{
//	SetWindowLong( cl_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST );
	SetWindowLongPtr( cl_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST );
	SetForegroundWindow( cl_hwnd );
}

/*
** VID_GetModeInfo
*/
typedef struct vidmode_s
{
	const char *description;
	int         width, height;
	int         mode;
} vidmode_t;

// Knightmare- added 1280x1024, 1400x1050, 856x480, 1024x480 modes
vidmode_t vid_modes[] =
{
#include "../qcommon/vid_modes.h"
};

qboolean VID_GetModeInfo (int *width, int *height, int mode)
{
	if (mode == -1) // custom mode
	{
		*width  = r_customwidth->value;
		*height = r_customheight->value;
		return true;
	}

	if ( mode < 0 || mode >= VID_NUM_MODES )
		return false;


	*width  = vid_modes[mode].width;
	*height = vid_modes[mode].height;

	return true;
}

/*
==============
VID_UpdateWindowPosAndSize
==============
*/
void VID_UpdateWindowPosAndSize ( int x, int y )
{
	RECT r;
	int		style;
	int		w, h;

	r.left   = 0;
	r.top    = 0;
	r.right  = viddef.width;
	r.bottom = viddef.height;

//	style = GetWindowLong( cl_hwnd, GWL_STYLE );
	style = GetWindowLongPtr( cl_hwnd, GWL_STYLE );
	AdjustWindowRect( &r, style, FALSE );

	w = r.right - r.left;
	h = r.bottom - r.top;

	MoveWindow( cl_hwnd, vid_xpos->value, vid_ypos->value, w, h, TRUE );
}

/*
==============
VID_NewWindow
==============
*/
void VID_NewWindow ( int width, int height)
{
	viddef.width  = width;
	viddef.height = height;

	cl.force_refdef = true;		// can't use a paused refdef
}

void VID_FreeReflib (void)
{
	kmgl_active  = false;
}


extern	decalpolys_t	*active_decals;
static qboolean reclip_decals = false;
qboolean	vid_reloading; // Knightmare- flag to not unnecessarily drop console

/*
==============
UpdateVideoRef
==============
*/
void UpdateVideoRef (void)
{
	char	reason[128];

	if ( vid_ref->modified )
	{
		cl.force_refdef = true;		// can't use a paused refdef
		S_StopAllSounds();

		// unclip decals
		if (active_decals) {
			CL_UnclipDecals();
			reclip_decals = true;
		}
	}

	vid_reloading = false;

	while (vid_ref->modified)
	{	// refresh has changed
		vid_ref->modified = false;
		vid_fullscreen->modified = true;
		cl.refresh_prepped = false;
		if (cl.cinematictime > 0) // Knightmare added
			cls.disable_screen = false;
		else
			cls.disable_screen = true;
		vid_reloading = true;
		// end Knightmare

		//==========================
		// compacted code from VID_LoadRefresh
		//==========================
		if ( kmgl_active )
		{
			R_Shutdown();
			VID_FreeReflib ();
		}

		Com_Printf( "\n------ Renderer Initialization ------\n");

		if ( !R_Init( global_hInstance, MainWndProc, reason ) )
		{
			R_Shutdown();
			VID_FreeReflib ();
			Com_Error (ERR_FATAL, "Couldn't initialize OpenGL renderer!\n%s", reason);
		}

		Com_Printf( "------------------------------------\n");

		kmgl_active = true;
		//==========================
	}

	// added to close loading screen
	if (cl.refresh_prepped && vid_reloading)
		cls.disable_screen = false;

	// re-clip decals
	if (cl.refresh_prepped && reclip_decals) {
		CL_ReclipDecals();
		reclip_decals = false;
	}

 	vid_reloading = false;
}


/*
============
VID_CheckChanges

This function gets called once just before drawing each frame, and it's sole purpose in life
is to check to see if any of the video mode parameters have changed, and if they have to 
update the rendering DLL and/or video mode to match.
============
*/
void VID_CheckChanges (void)
{
	if ( win_noalttab->modified )
	{
		if ( win_noalttab->value )
			WIN_DisableAltTab();
		else
			WIN_EnableAltTab();
		win_noalttab->modified = false;
	}

	//update changed vid_ref
	UpdateVideoRef ();

	// update our window position
	if ( vid_xpos->modified || vid_ypos->modified )
	{
		if (!vid_fullscreen->value)
			VID_UpdateWindowPosAndSize( vid_xpos->value, vid_ypos->value );

		vid_xpos->modified = false;
		vid_ypos->modified = false;
	}
}

/*
============
VID_Init
============
*/
void VID_Init (void)
{
	/* Create the video variables so we know how to start the graphics drivers */
	vid_ref = Cvar_Get ("vid_ref", "gl", CVAR_ARCHIVE);
	vid_xpos = Cvar_Get ("vid_xpos", "3", CVAR_ARCHIVE);
	vid_ypos = Cvar_Get ("vid_ypos", "22", CVAR_ARCHIVE);
	vid_fullscreen = Cvar_Get ("vid_fullscreen", "1", CVAR_ARCHIVE);
	vid_gamma = Cvar_Get( "vid_gamma", "0.8", CVAR_ARCHIVE ); // was 1.0
	win_noalttab = Cvar_Get( "win_noalttab", "0", CVAR_ARCHIVE );
	win_alttab_restore_desktop = Cvar_Get( "win_alttab_restore_desktop", "1", CVAR_ARCHIVE );	// Knightmare- whether to restore desktop resolution on alt-tab
	r_customwidth = Cvar_Get( "r_customwidth", "1600", CVAR_ARCHIVE );
	r_customheight = Cvar_Get( "r_customheight", "1024", CVAR_ARCHIVE );
	// Knightmare- just here to enable command line option without error
	scanforcd = Cvar_Get( "scanforcd", "0", 0 );

	// force vid_ref to gl
	// older versions of Lazarus code check only vid_ref=gl for fadein effects
	Cvar_Set( "vid_ref", "gl" );
	vidref_val = VIDREF_GL;	// this is always in GL mode

	/* Add some console commands that we want to handle */
	Cmd_AddCommand ("vid_restart", VID_Restart_f);
	Cmd_AddCommand ("vid_front", VID_Front_f);

	/* Disable the 3Dfx splash screen */
	//putenv("FX_GLIDE_NO_SPLASH=0");

	/* Start the graphics mode and load refresh DLL */
	VID_CheckChanges();
}

/*
============
VID_Shutdown
============
*/
void VID_Shutdown (void)
{
	if ( kmgl_active )
	{
		R_Shutdown ();
		VID_FreeReflib ();
	}
}


