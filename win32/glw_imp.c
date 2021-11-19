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

/*
** GLW_IMP.C
**
** This file contains ALL Win32 specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
**
*/
#include <assert.h>
#include <windows.h>
#include "../renderer/r_local.h"
#include "resource.h"
#include "glw_win.h"
#include "winquake.h"

static qboolean GLimp_SwitchFullscreen( int width, int height );
qboolean GLimp_InitGL (void);

glwstate_t glw_state;

extern cvar_t *vid_fullscreen;
extern cvar_t *vid_ref;

// Knightmare- added Vic's hardware gammaramp
WORD original_ramp[3][256];
WORD gamma_ramp[3][256];

void InitGammaRamp (void)
{
	if (!r_ignorehwgamma->integer)
	{
		if (qwglGetDeviceGammaRamp3DFX)
	/*	{
			WORD newramp[3*256];
			int j;

			glState.gammaRamp = qwglGetDeviceGammaRamp3DFX ( glw_state.hDC, newramp );

			for (j = 0; j < 256; j++)
			{
				original_ramp[0][j] = newramp[j+0];
				original_ramp[1][j] = newramp[j+256];
				original_ramp[2][j] = newramp[j+512];
			}
		}*/
			glState.gammaRamp = qwglGetDeviceGammaRamp3DFX ( glw_state.hDC, original_ramp );
		else
			glState.gammaRamp = GetDeviceGammaRamp ( glw_state.hDC, original_ramp );
	}
	else
		glState.gammaRamp = false;

	if (glState.gammaRamp)
		vid_gamma->modified = true;
}

void ShutdownGammaRamp (void)
{
//	if (r_ignorehwgamma->integer)
	if (!glState.gammaRamp)
		return;

	if (qwglSetDeviceGammaRamp3DFX)
/*	{
		WORD newramp[3*256];
		int j;

		for (j = 0; j < 256; j++)
		{
			newramp[j+0] = original_ramp[0][j];
			newramp[j+256] = original_ramp[1][j];
			newramp[j+512] = original_ramp[2][j];
		}
		qwglSetDeviceGammaRamp3DFX ( glw_state.hDC, newramp );
	}*/
		qwglSetDeviceGammaRamp3DFX ( glw_state.hDC, original_ramp );
	else
		SetDeviceGammaRamp (glw_state.hDC, original_ramp);
}

void UpdateGammaRamp (void)
{
	int	i, o;

	if (!glState.gammaRamp)
		return;
	
	memcpy (gamma_ramp, original_ramp, sizeof(original_ramp));

	for (o = 0; o < 3; o++) 
	{
		for (i = 0; i < 256; i++) 
		{
			signed int v;

			v = 255 * pow ((i+0.5)/255.5, vid_gamma->value ) + 0.5;
			if (v > 255) v = 255;
			if (v < 0) v = 0;
			gamma_ramp[o][i] = ((WORD)v) << 8;
		}
	}
	if (qwglSetDeviceGammaRamp3DFX)
/*	{
		WORD newramp[3*256];
		int		j;

		for (j = 0; j < 256; j++) {
			newramp[j+0] = gamma_ramp[0][j];
			newramp[j+256] = gamma_ramp[1][j];
			newramp[j+512] = gamma_ramp[2][j];
		}
		qwglSetDeviceGammaRamp3DFX (glw_state.hDC, newramp);
	}*/
		qwglSetDeviceGammaRamp3DFX (glw_state.hDC, gamma_ramp);
	else
		SetDeviceGammaRamp (glw_state.hDC, gamma_ramp);
}

void ToggleGammaRamp (qboolean enable)
{
//	WORD	newramp[3*256];
//	int		j;

	if (!glState.gammaRamp)
		return;

	if (enable)
	{
		if (qwglSetDeviceGammaRamp3DFX)
	/*	{
			for (j = 0; j < 256; j++) {
				newramp[j+0] = gamma_ramp[0][j];
				newramp[j+256] = gamma_ramp[1][j];
				newramp[j+512] = gamma_ramp[2][j];
			}
			qwglSetDeviceGammaRamp3DFX (glw_state.hDC, newramp);
		}*/
			qwglSetDeviceGammaRamp3DFX (glw_state.hDC, gamma_ramp);
		else
			SetDeviceGammaRamp (glw_state.hDC, gamma_ramp);
	}
	else
	{
		if (qwglSetDeviceGammaRamp3DFX)
	/*	{
			for (j = 0; j < 256; j++)
			{
				newramp[j+0] = original_ramp[0][j];
				newramp[j+256] = original_ramp[1][j];
				newramp[j+512] = original_ramp[2][j];
			}
			qwglSetDeviceGammaRamp3DFX ( glw_state.hDC, newramp );
		}*/
			qwglSetDeviceGammaRamp3DFX ( glw_state.hDC, original_ramp );
		else
			SetDeviceGammaRamp (glw_state.hDC, original_ramp);
	}
}
// end Vic's hardware gammaramp

static qboolean VerifyDriver( void )
{
	char buffer[1024];

//	strncpy (buffer, qglGetString(GL_RENDERER));
	Q_strncpyz (buffer, sizeof(buffer), qglGetString(GL_RENDERER));
	strlwr( buffer );
	if ( strcmp( buffer, "gdi generic" ) == 0 )
		if ( !glw_state.mcd_accelerated )
			return false;
	return true;
}


/*
** VID_CreateWindow
*/
#define	WINDOW_CLASS_NAME	"KMQuake2" // changed
#define	WINDOW_CLASS_NAME2	"KMQuake2 - The Reckoning" // changed
#define	WINDOW_CLASS_NAME3	"KMQuake2 - Ground Zero" // changed

qboolean VID_CreateWindow (int width, int height, dispType_t fullscreen)
{
	WNDCLASS		wc;
	RECT			r;
	cvar_t			*vid_xpos, *vid_ypos;
	int				stylebits;
	int				x, y, w, h;
	int				exstyle;

	/* Register the frame class */
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)glw_state.wndproc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = glw_state.hInstance;

	if (FS_ModType("xatrix")) { // The Reckoning
		wc.hIcon         = LoadIcon(glw_state.hInstance, MAKEINTRESOURCE(IDI_ICON2));
	//	wc.lpszClassName = WINDOW_CLASS_NAME2;
	}
	else if (FS_ModType("rogue"))  { // Ground Zero
		wc.hIcon         = LoadIcon(glw_state.hInstance, MAKEINTRESOURCE(IDI_ICON3));
	//	wc.lpszClassName = WINDOW_CLASS_NAME3;
	}
	else {
		wc.hIcon         = LoadIcon(glw_state.hInstance, MAKEINTRESOURCE(IDI_ICON1));
	}

    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = (void *)COLOR_GRAYTEXT;
    wc.lpszMenuName  = 0;
	wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClass (&wc) )
		VID_Error (ERR_FATAL, "Couldn't register window class");

	if ( fullscreen == dt_fullscreen )	// borderless support
	{
		exstyle = WS_EX_TOPMOST;
	//	stylebits = WS_POPUP|WS_VISIBLE;
		stylebits = WS_POPUP|WS_SYSMENU|WS_VISIBLE;
	}
	else if ( fullscreen == dt_borderless )	// borderless support
	{
		exstyle = 0;
		stylebits = WS_POPUP|WS_VISIBLE;
	}
	else // if ( fullscreen == dt_windowed )
	{
		exstyle = 0;
	//	stylebits = WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_VISIBLE;
		stylebits = WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_SYSMENU|WS_VISIBLE;
	}

	r.left = 0;
	r.top = 0;
	r.right  = width;
	r.bottom = height;

	AdjustWindowRect (&r, stylebits, FALSE);

	w = r.right - r.left;
	h = r.bottom - r.top;

	if ( fullscreen == dt_fullscreen )	// borderless support
	{
		x = 0;
		y = 0;
	}
	else if ( fullscreen == dt_borderless )	// borderless support
	{
		HDC		hDC;
		int		hRes, vRes;

		hDC = GetDC (NULL);
		hRes = GetDeviceCaps (hDC, HORZRES);
		vRes = GetDeviceCaps (hDC, VERTRES);
		ReleaseDC (0, hDC);

		if (hRes <= vid.width)
			x = 0;
		else
			x = (hRes - vid.width) / 2;

		if (vRes <= vid.height)
			y = 0;
		else
			y = (vRes - vid.height) / 2;
	}
	else // if ( fullscreen == dt_windowed )
	{
		vid_xpos = Cvar_Get ("vid_xpos", "0", 0);
		vid_ypos = Cvar_Get ("vid_ypos", "0", 0);
		x = vid_xpos->integer;
		y = vid_ypos->integer;
	}

	glw_state.hWnd = CreateWindowEx (
		 exstyle, 
		 WINDOW_CLASS_NAME,
		 "KMQuake2",		//Knightmare changed
		 stylebits,
		 x, y, w, h,
		 NULL,
		 NULL,
		 glw_state.hInstance,
		 NULL);

	if (!glw_state.hWnd)
		VID_Error (ERR_FATAL, "Couldn't create window");
	
	ShowWindow( glw_state.hWnd, SW_SHOW );
	UpdateWindow( glw_state.hWnd );

	// init all the gl stuff for the window
	if (!GLimp_InitGL ())
	{
		VID_Printf( PRINT_ALL, "VID_CreateWindow() - GLimp_InitGL failed\n");
		return false;
	}

	SetForegroundWindow( glw_state.hWnd );
	SetFocus( glw_state.hWnd );

	// let the sound and input subsystems know about the new window
	VID_NewWindow (width, height);

	return true;
}


/*
** GLimp_SetMode
*/
rserr_t GLimp_SetMode (int *pwidth, int *pheight, int mode, dispType_t fullscreen)
{
	int width, height;
	const char	*win_fs[] = { "W", "FS", "BL" };	// borderless support

	VID_Printf( PRINT_ALL, "Initializing OpenGL display\n");

	VID_Printf (PRINT_ALL, "...setting mode %d:", mode );

	if ( !VID_GetModeInfo( &width, &height, mode ) )
	{
		VID_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}

	VID_Printf( PRINT_ALL, " %dx%d %s\n", width, height, win_fs[min(max(fullscreen, 0), 2)] );	// clamp index

	// destroy the existing window
	if (glw_state.hWnd)
	{
		GLimp_Shutdown ();
	}

	// do a CDS if needed
	if ( fullscreen == dt_fullscreen )	// borderless support
	{
		DEVMODE fullscreenMode;

		VID_Printf( PRINT_ALL, "...attempting fullscreen\n" );

		memset( &fullscreenMode, 0, sizeof( fullscreenMode ) );

		fullscreenMode.dmSize = sizeof( fullscreenMode );

		fullscreenMode.dmPelsWidth  = width;
		fullscreenMode.dmPelsHeight = height;
		fullscreenMode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;

		// Added refresh rate control
		if ( r_displayrefresh->integer != 0 )
		{
			fullscreenMode.dmDisplayFrequency = r_displayrefresh->integer;
			fullscreenMode.dmFields |= DM_DISPLAYFREQUENCY;
			VID_Printf( PRINT_ALL, "...using r_displayrefresh of %d\n", r_displayrefresh->integer );
		}

		if ( r_bitdepth->integer != 0 )
		{
			fullscreenMode.dmBitsPerPel = r_bitdepth->integer;
			fullscreenMode.dmFields |= DM_BITSPERPEL;
			VID_Printf( PRINT_ALL, "...using r_bitdepth of %d\n", r_bitdepth->integer );
		}
		else
		{
			HDC hdc = GetDC( NULL );
			int bitspixel = GetDeviceCaps( hdc, BITSPIXEL );

			VID_Printf( PRINT_ALL, "...using desktop display depth of %d\n", bitspixel );

			ReleaseDC( 0, hdc );
		}

		VID_Printf( PRINT_ALL, "...calling CDS: " );
		if ( ChangeDisplaySettings( &fullscreenMode, CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL )
		{
			*pwidth = width;
			*pheight = height;

			glState.fullscreen = true;

			VID_Printf( PRINT_ALL, "ok\n" );

			if ( !VID_CreateWindow (width, height, true) )
				return rserr_invalid_mode;

			return rserr_ok;
		}
		else
		{
			*pwidth = width;
			*pheight = height;

			VID_Printf( PRINT_ALL, "failed\n" );

			VID_Printf( PRINT_ALL, "...calling CDS assuming dual monitors:" );

			fullscreenMode.dmPelsWidth = width * 2;
			fullscreenMode.dmPelsHeight = height;
			fullscreenMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

			if ( r_bitdepth->integer != 0 )
			{
				fullscreenMode.dmBitsPerPel = r_bitdepth->integer;
				fullscreenMode.dmFields |= DM_BITSPERPEL;
			}

			/*
			** our first CDS failed, so maybe we're running on some weird dual monitor
			** system 
			*/
			if ( ChangeDisplaySettings( &fullscreenMode, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
			{
				VID_Printf( PRINT_ALL, " failed\n" );

				VID_Printf( PRINT_ALL, "...setting windowed mode\n" );

				ChangeDisplaySettings( 0, 0 );

				*pwidth = width;
				*pheight = height;
				glState.fullscreen = false;
				if ( !VID_CreateWindow (width, height, dt_windowed) )
					return rserr_invalid_mode;
				return rserr_invalid_fullscreen;
			}
			else
			{
				VID_Printf( PRINT_ALL, " ok\n" );
				if ( !VID_CreateWindow (width, height, dt_fullscreen) )
					return rserr_invalid_mode;

				glState.fullscreen = true;
				return rserr_ok;
			}
		}
	}
	else if ( fullscreen == dt_borderless )	// borderless support
	{
		VID_Printf( PRINT_ALL, "...setting borderless window mode\n" );

		ChangeDisplaySettings( 0, 0 );

		*pwidth = width;
		*pheight = height;
		glState.fullscreen = false;
		if ( !VID_CreateWindow (width, height, dt_borderless) )
			return rserr_invalid_borderless;
	}
	else // if ( fullscreen == dt_windowed )
	{
		VID_Printf( PRINT_ALL, "...setting windowed mode\n" );

		ChangeDisplaySettings( 0, 0 );

		*pwidth = width;
		*pheight = height;
		glState.fullscreen = false;
		if ( !VID_CreateWindow (width, height, dt_windowed) )
			return rserr_invalid_mode;
	}

	return rserr_ok;
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/
void GLimp_Shutdown (void)
{
	// Knightmare- added Vic's hardware gamma ramp
	ShutdownGammaRamp ();

	if ( qwglMakeCurrent && !qwglMakeCurrent( NULL, NULL ) )
		VID_Printf( PRINT_ALL, "ref_gl::R_Shutdown() - wglMakeCurrent failed\n");
	if ( glw_state.hGLRC )
	{
		if (  qwglDeleteContext && !qwglDeleteContext( glw_state.hGLRC ) )
			VID_Printf( PRINT_ALL, "ref_gl::R_Shutdown() - wglDeleteContext failed\n");
		glw_state.hGLRC = NULL;
	}
	if (glw_state.hDC)
	{
		if ( !ReleaseDC( glw_state.hWnd, glw_state.hDC ) )
			VID_Printf( PRINT_ALL, "ref_gl::R_Shutdown() - ReleaseDC failed\n" );
		glw_state.hDC   = NULL;
	}
	if (glw_state.hWnd)
	{	//Knightmare- remove leftover button on taskbar
		ShowWindow (glw_state.hWnd, SW_HIDE);
		//end Knightmare
		DestroyWindow (	glw_state.hWnd );
		glw_state.hWnd = NULL;
	}

	if ( glw_state.log_fp )
	{
		fclose( glw_state.log_fp );
		glw_state.log_fp = 0;
	}

	UnregisterClass (WINDOW_CLASS_NAME, glw_state.hInstance);

	if ( glState.fullscreen )
	{
		ChangeDisplaySettings( 0, 0 );
		glState.fullscreen = false;
	}
}


/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.  Under Win32 this means dealing with the pixelformats and
** doing the wgl interface stuff.
*/
qboolean GLimp_Init (void *hinstance, void *wndproc)
{
#define OSR2_BUILD_NUMBER 1111

	OSVERSIONINFO	vinfo;

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	glw_state.allowdisplaydepthchange = false;

	if ( GetVersionEx( &vinfo) )
	{
		if ( vinfo.dwMajorVersion > 4 )
		{
			glw_state.allowdisplaydepthchange = true;
		}
		else if ( vinfo.dwMajorVersion == 4 )
		{
			if ( vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
			{
				glw_state.allowdisplaydepthchange = true;
			}
			else if ( vinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
			{
				if ( LOWORD( vinfo.dwBuildNumber ) >= OSR2_BUILD_NUMBER )
				{
					glw_state.allowdisplaydepthchange = true;
				}
			}
		}
	}
	else
	{
		VID_Printf( PRINT_ALL, "GLimp_Init() - GetVersionEx failed\n" );
		return false;
	}

	glw_state.hInstance = ( HINSTANCE ) hinstance;
	glw_state.wndproc = wndproc;

	return true;
}

qboolean GLimp_InitGL (void)
{
    PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,								// version number
		PFD_DRAW_TO_WINDOW |			// support window
		PFD_SUPPORT_OPENGL |			// support OpenGL
		PFD_DOUBLEBUFFER,				// double buffered
		PFD_TYPE_RGBA,					// RGBA type
		32,								// 32-bit color depth // was 24
		0, 0, 0, 0, 0, 0,				// color bits ignored
		0,								// no alpha buffer
		0,								// shift bit ignored
		0,								// no accumulation buffer
		0, 0, 0, 0, 					// accum bits ignored
		//Knightmare 12/24/2001- stencil buffer
		24,								// 24-bit z-buffer, was 32	
		8,								// 8-bit stencil buffer
		//end Knightmare
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main layer
		0,								// reserved
		0, 0, 0							// layer masks ignored
    };
    int  pixelformat;
	cvar_t *stereo;
	
	stereo = Cvar_Get( "cl_stereo", "0", 0 );

	/*
	** set PFD_STEREO if necessary
	*/
	if ( stereo->integer != 0 )
	{
		VID_Printf( PRINT_ALL, "...attempting to use stereo\n" );
		pfd.dwFlags |= PFD_STEREO;
		glState.stereo_enabled = true;
	}
	else
	{
		glState.stereo_enabled = false;
	}

	/*
	** figure out if we're running on a minidriver or not
	*/
	if ( strstr( gl_driver->string, "opengl32" ) != 0 )
		glw_state.minidriver = false;
	else
		glw_state.minidriver = true;

	/*
	** Get a DC for the specified window
	*/
	if ( glw_state.hDC != NULL )
		VID_Printf( PRINT_ALL, "GLimp_Init() - non-NULL DC exists\n" );

    if ( ( glw_state.hDC = GetDC( glw_state.hWnd ) ) == NULL )
	{
		VID_Printf( PRINT_ALL, "GLimp_Init() - GetDC failed\n" );
		return false;
	}

	if ( glw_state.minidriver )
	{
		if ( (pixelformat = qwglChoosePixelFormat( glw_state.hDC, &pfd)) == 0 )
		{
			VID_Printf (PRINT_ALL, "GLimp_Init() - qwglChoosePixelFormat failed\n");
			return false;
		}
		if ( qwglSetPixelFormat( glw_state.hDC, pixelformat, &pfd) == FALSE )
		{
			VID_Printf (PRINT_ALL, "GLimp_Init() - qwglSetPixelFormat failed\n");
			return false;
		}
		qwglDescribePixelFormat( glw_state.hDC, pixelformat, sizeof( pfd ), &pfd );
	}
	else
	{
		if ( ( pixelformat = ChoosePixelFormat( glw_state.hDC, &pfd)) == 0 )
		{
			VID_Printf (PRINT_ALL, "GLimp_Init() - ChoosePixelFormat failed\n");
			return false;
		}
		if ( SetPixelFormat( glw_state.hDC, pixelformat, &pfd) == FALSE )
		{
			VID_Printf (PRINT_ALL, "GLimp_Init() - SetPixelFormat failed\n");
			return false;
		}
		DescribePixelFormat( glw_state.hDC, pixelformat, sizeof( pfd ), &pfd );

		if ( !( pfd.dwFlags & PFD_GENERIC_ACCELERATED ) )
		{
			extern cvar_t *gl_allow_software;

			if ( gl_allow_software->integer )
				glw_state.mcd_accelerated = true;
			else
				glw_state.mcd_accelerated = false;
		}
		else
		{
			glw_state.mcd_accelerated = true;
		}
	}

	/*
	** report if stereo is desired but unavailable
	*/
	if ( !( pfd.dwFlags & PFD_STEREO ) && ( stereo->integer != 0 ) ) 
	{
		VID_Printf( PRINT_ALL, "...failed to select stereo pixel format\n" );
		Cvar_SetValue( "cl_stereo", 0 );
		glState.stereo_enabled = false;
	}

	/*
	** startup the OpenGL subsystem by creating a context and making
	** it current
	*/
	if ( ( glw_state.hGLRC = qwglCreateContext( glw_state.hDC ) ) == 0 )
	{
		VID_Printf (PRINT_ALL, "GLimp_Init() - qwglCreateContext failed\n");

		goto fail;
	}

    if ( !qwglMakeCurrent( glw_state.hDC, glw_state.hGLRC ) )
	{
		VID_Printf (PRINT_ALL, "GLimp_Init() - qwglMakeCurrent failed\n");

		goto fail;
	}

	if ( !VerifyDriver() )
	{
		VID_Printf( PRINT_ALL, "GLimp_Init() - no hardware acceleration detected.\nPlease install drivers provided by your video card/GPU vendor.\n" );
		goto fail;
	}

	/*
	** print out PFD specifics 
	*/
	VID_Printf( PRINT_ALL, "PIXELFORMAT: color(%d-bits) Z(%d-bit)\n", ( int ) pfd.cColorBits, ( int ) pfd.cDepthBits );

	// Knightmare- Vic's hardware gamma stuff
	InitGammaRamp ();

	// moved these to GL_SetDefaultState
	//glState.blend = false;
	//glState.alphaTest = false;
	//end Knightmare

	//Knightmare- 12/24/2001- stecil buffer
	{
		char buffer[1024];

	//	strncpy (buffer, qglGetString(GL_RENDERER));
		Q_strncpyz (buffer, sizeof(buffer), qglGetString(GL_RENDERER));
		strlwr( buffer );
		if (strstr(buffer, "Voodoo3")) {
			VID_Printf( PRINT_ALL, "... Voodoo3 has no stencil buffer\n" );
			glConfig.have_stencil = false;
		} else {
			if (pfd.cStencilBits) {
				VID_Printf( PRINT_ALL, "... Using stencil buffer\n" );
				glConfig.have_stencil = true; // Stencil shadows - MrG
			}
		}
	}
	//if (pfd.cStencilBits)
	//	glConfig.have_stencil = true;
	//end Knightmare

/*	Moved to GL_SetDefaultState in r_glstate.c
	// Vertex arrays
	qglEnableClientState (GL_TEXTURE_COORD_ARRAY);
	qglEnableClientState (GL_VERTEX_ARRAY);
	qglEnableClientState (GL_COLOR_ARRAY);

	qglTexCoordPointer (2, GL_FLOAT, sizeof(texCoordArray[0][0]), texCoordArray[0][0]);
	qglVertexPointer (3, GL_FLOAT, sizeof(vertexArray[0]), vertexArray[0]);
	qglColorPointer (4, GL_FLOAT, sizeof(colorArray[0]), colorArray[0]);
	//glState.activetmu[0] = true;
	// end vertex arrays
*/

	return true;

fail:
	if ( glw_state.hGLRC )
	{
		qwglDeleteContext( glw_state.hGLRC );
		glw_state.hGLRC = NULL;
	}

	if ( glw_state.hDC )
	{
		ReleaseDC( glw_state.hWnd, glw_state.hDC );
		glw_state.hDC = NULL;
	}
	return false;
}


/*
** GLimp_BeginFrame
*/
void GLimp_BeginFrame (float camera_separation)
{
	if ( r_bitdepth->modified )
	{
		if ( r_bitdepth->integer != 0 && !glw_state.allowdisplaydepthchange )
		{
			Cvar_SetValue( "r_bitdepth", 0 );
			VID_Printf( PRINT_ALL, "r_bitdepth requires Win95 OSR2.x or WinNT 4.x\n" );
		}
		r_bitdepth->modified = false;
	}

	if ( camera_separation < 0 && glState.stereo_enabled )
	{
		qglDrawBuffer( GL_BACK_LEFT );
	}
	else if ( camera_separation > 0 && glState.stereo_enabled )
	{
		qglDrawBuffer( GL_BACK_RIGHT );
	}
	else
	{
		qglDrawBuffer( GL_BACK );
	}
}

/*
** GLimp_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void GLimp_EndFrame (void)
{
	int		err;

	err = qglGetError();
	if (err != GL_NO_ERROR)	// Output error code instead
	//	VID_Printf (PRINT_DEVELOPER, "OpenGL Error %i\n", err);
		GL_PrintError (err, "GLimp_EndFrame");

	if ( stricmp( r_drawbuffer->string, "GL_BACK" ) == 0 )
	{
		if ( !qwglSwapBuffers( glw_state.hDC ) )
			VID_Error (ERR_FATAL, "GLimp_EndFrame() - SwapBuffers() failed!\n");
	}
}

/*
** GLimp_AppActivate
*/
void GLimp_AppActivate (qboolean active)
{
	static qboolean	desktop_restored;
	cvar_t			*restore_desktop = Cvar_Get( "win_alttab_restore_desktop", "1", CVAR_ARCHIVE );

	if ( active )
	{
		ToggleGammaRamp (true);
		SetForegroundWindow( glw_state.hWnd );
		ShowWindow( glw_state.hWnd, SW_RESTORE );

		// Knightmare- restore desktop settings on alt-tabbing from fullscreen
	//	if ( vid_fullscreen->integer && desktop_restored && glw_state.hGLRC != NULL )
		if ( (vid_fullscreen->integer == 1) && desktop_restored && glw_state.hGLRC != NULL )	// borderless support
		{
			int		width, height;
			DEVMODE	fullscreenMode;

			if ( !VID_GetModeInfo (&width, &height, r_mode->integer) )
			{
				VID_Printf( PRINT_ALL, "invalid mode\n" );
				return;
			}
			
			memset( &fullscreenMode, 0, sizeof( fullscreenMode ) );
			fullscreenMode.dmSize = sizeof( fullscreenMode );
			fullscreenMode.dmPelsWidth  = width;
			fullscreenMode.dmPelsHeight = height;
			fullscreenMode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;

			if ( r_displayrefresh->integer != 0 )
			{
				fullscreenMode.dmDisplayFrequency = r_displayrefresh->integer;
				fullscreenMode.dmFields |= DM_DISPLAYFREQUENCY;
			//	VID_Printf( PRINT_ALL, "...using r_displayrefresh of %d\n", (int)r_displayrefresh->integer );
			}

			if ( r_bitdepth->integer != 0 )
			{
				fullscreenMode.dmBitsPerPel = r_bitdepth->integer;
				fullscreenMode.dmFields |= DM_BITSPERPEL;
			//	VID_Printf( PRINT_ALL, "...using r_bitdepth of %d\n", (int)r_bitdepth->integer );
			}
			else
			{
				HDC hdc = GetDC( NULL );
				int bitspixel = GetDeviceCaps( hdc, BITSPIXEL );
			//	VID_Printf( PRINT_ALL, "...using desktop display depth of %d\n", bitspixel );
				ReleaseDC( 0, hdc );
			}

			VID_Printf( PRINT_ALL, "...calling CDS: " );
			if ( ChangeDisplaySettings( &fullscreenMode, CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL )
			{
				VID_Printf( PRINT_ALL, "ok\n" );
			}
			else
			{	// our first CDS failed, so maybe we're running on some weird dual monitor system 
				VID_Printf( PRINT_ALL, "failed\n" );
				VID_Printf( PRINT_ALL, "...calling CDS assuming dual monitors: " );

				fullscreenMode.dmPelsWidth = width * 2;
				fullscreenMode.dmPelsHeight = height;
				fullscreenMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

				if ( r_bitdepth->integer != 0 )
				{
					fullscreenMode.dmBitsPerPel = r_bitdepth->integer;
					fullscreenMode.dmFields |= DM_BITSPERPEL;
				}

				if ( ChangeDisplaySettings( &fullscreenMode, CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL )
				{
					VID_Printf( PRINT_ALL, "ok\n" );
				}
			}
			desktop_restored = false;
		}
	}
	else
	{
		ToggleGammaRamp (false);

		if ( vid_fullscreen->integer )
		{
			ShowWindow( glw_state.hWnd, SW_MINIMIZE );

			// Knightmare- restore desktop settings on alt-tabbing from fullscreen
			if (restore_desktop->integer) {
				ChangeDisplaySettings (0, 0);
				desktop_restored = true;
			}
			else
				desktop_restored = false;
		}
	}
}
