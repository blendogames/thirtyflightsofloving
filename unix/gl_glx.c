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
/*
** GLW_IMP.C
**
** This file contains ALL Linux specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
**
*/

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#ifdef __linux__
#include <sys/vt.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <dlfcn.h>

#include "../renderer/r_local.h"

#include "../client/keys.h"

#include "glw_unix.h"

#include <GL/glx.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include <X11/extensions/xf86dga.h>
#include <X11/extensions/xf86vmode.h>

glwstate_t glw_state;

static Display *dpy = NULL;
static int scrnum;
static Window win;
static GLXContext ctx = NULL;
static Atom wmDeleteWindow;

#define KEY_MASK (KeyPressMask | KeyReleaseMask)
#define MOUSE_MASK (ButtonPressMask | ButtonReleaseMask | \
		    PointerMotionMask | ButtonMotionMask )
#define X_MASK (KEY_MASK | MOUSE_MASK | VisibilityChangeMask | StructureNotifyMask )

/*****************************************************************************/
/* MOUSE                                                                     */
/*****************************************************************************/

// this is inside the renderer shared lib, so these are called from vid_so

int   mx, my;
static int	old_mouse_x, old_mouse_y;
int p_mouse_x, p_mouse_y;

static int win_x, win_y;

//static cvar_t	*m_filter;
//static cvar_t	*in_mouse;
extern cvar_t	*in_dgamouse;
extern cvar_t	*in_menumouse; /// FIXME Menu Mouse on windowed mode

static cvar_t	*r_fakeFullscreen;

static XF86VidModeModeInfo **vidmodes;
static int num_vidmodes;
static qboolean vidmode_active = false;
static XF86VidModeGamma oldgamma;

qboolean mouse_active = false;
qboolean dgamouse = false;
qboolean vidmode_ext = false;

static Time myxtime;

extern cursor_t cursor;

qboolean have_stencil = false; //Knightmare 12/24/2001- stencil shadows

static Cursor CreateNullCursor(Display *display, Window root)
{
    Pixmap cursormask; 
    XGCValues xgc;
    GC gc;
    XColor dummycolour;
    Cursor cursor;

    cursormask = XCreatePixmap(display, root, 1, 1, 1/*depth*/);
    xgc.function = GXclear;
    gc =  XCreateGC(display, cursormask, GCFunction, &xgc);
    XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
    dummycolour.pixel = 0;
    dummycolour.red = 0;
    dummycolour.flags = 04;
    cursor = XCreatePixmapCursor(display, cursormask, cursormask,
          &dummycolour,&dummycolour, 0,0);
    XFreePixmap(display,cursormask);
    XFreeGC(display,gc);
    return cursor;
}

void install_grabs(void)
{

// inviso cursor
	XDefineCursor(dpy, win, CreateNullCursor(dpy, win));

	XGrabPointer(dpy, win,
				 True,
				 0,
				 GrabModeAsync, GrabModeAsync,
				 win,
				 None,
				 CurrentTime);

	if (in_dgamouse->value) {
		int MajorVersion, MinorVersion;

		if (!XF86DGAQueryVersion(dpy, &MajorVersion, &MinorVersion)) { 
			dgamouse = false;
			// unable to query, probalby not supported
			Com_Printf( "Failed to detect XF86DGA Mouse\n" );
			Cvar_Set( "in_dgamouse", "0" );
		} else {
			dgamouse = true;
			XF86DGADirectVideo(dpy, DefaultScreen(dpy), XF86DGADirectMouse);
			XWarpPointer(dpy, None, win, 0, 0, 0, 0, 0, 0);
		}
	} else {
		p_mouse_x = vid.width / 2;
		p_mouse_y = vid.height / 2;
		XWarpPointer (dpy, None, win, 0, 0, 0, 0, p_mouse_x, p_mouse_y);
	}

	XGrabKeyboard(dpy, win,
				  False,
				  GrabModeAsync, GrabModeAsync,
				  CurrentTime);

	mx = 0;
	my = 0;
	old_mouse_x = 0;
	old_mouse_y = 0;
	
	mouse_active = true;
}

void uninstall_grabs(void)
{
	if (!dpy || !win)
		return;

	if (dgamouse) {
		dgamouse = false;
		XF86DGADirectVideo(dpy, DefaultScreen(dpy), 0);
	}

	XUngrabPointer(dpy, CurrentTime);
	XUngrabKeyboard(dpy, CurrentTime);

// inviso cursor
	XUndefineCursor(dpy, win);

	mouse_active = false;
}

void IN_DeactivateMouse( void ) 
{
	if (!dpy || !win)
		return;

	if (mouse_active) {
		uninstall_grabs();
		mouse_active = false;
	}
}

static void IN_ActivateMouse( void ) 
{
	if (!dpy || !win)
		return;

	if (!mouse_active) {
		mx = my = 0; // don't spazz
		install_grabs();
		mouse_active = true;
	}
}

void IN_Activate (qboolean active)
{

	if (active || vidmode_active)
		IN_ActivateMouse ();
	else if (!in_menumouse->value)
		IN_DeactivateMouse ();
}

/*****************************************************************************/
/* KEYBOARD                                                                  */
/*****************************************************************************/

static int XLateKey(XKeyEvent *ev)
{

	int key;
	char buf[64];
	KeySym keysym;

	key = 0;

	XLookupString(ev, buf, sizeof buf, &keysym, 0);

	switch(keysym)
	{
		case XK_KP_Page_Up:	 key = K_KP_PGUP; break;
		case XK_Page_Up:	 key = K_PGUP; break;

		case XK_KP_Page_Down: key = K_KP_PGDN; break;
		case XK_Page_Down:	 key = K_PGDN; break;

		case XK_KP_Home: key = K_KP_HOME; break;
		case XK_Home:	 key = K_HOME; break;

		case XK_KP_End:  key = K_KP_END; break;
		case XK_End:	 key = K_END; break;

		case XK_KP_Left: key = K_KP_LEFTARROW; break;
		case XK_Left:	 key = K_LEFTARROW; break;

		case XK_KP_Right: key = K_KP_RIGHTARROW; break;
		case XK_Right:	key = K_RIGHTARROW;		break;

		case XK_KP_Down: key = K_KP_DOWNARROW; break;
		case XK_Down:	 key = K_DOWNARROW; break;

		case XK_KP_Up:   key = K_KP_UPARROW; break;
		case XK_Up:		 key = K_UPARROW;	 break;

		case XK_Escape: key = K_ESCAPE;		break;

		case XK_KP_Enter: key = K_KP_ENTER;	break;
		case XK_Return: key = K_ENTER;		 break;

		case XK_Tab:		key = K_TAB;			 break;

		case XK_F1:		 key = K_F1;				break;

		case XK_F2:		 key = K_F2;				break;

		case XK_F3:		 key = K_F3;				break;

		case XK_F4:		 key = K_F4;				break;

		case XK_F5:		 key = K_F5;				break;

		case XK_F6:		 key = K_F6;				break;

		case XK_F7:		 key = K_F7;				break;

		case XK_F8:		 key = K_F8;				break;

		case XK_F9:		 key = K_F9;				break;

		case XK_F10:		key = K_F10;			 break;

		case XK_F11:		key = K_F11;			 break;

		case XK_F12:		key = K_F12;			 break;

		case XK_BackSpace: key = K_BACKSPACE; break;

		case XK_KP_Delete: key = K_KP_DEL; break;
		case XK_Delete: key = K_DEL; break;

		case XK_Pause:	key = K_PAUSE;		 break;

		case XK_Shift_L:
		case XK_Shift_R:	key = K_SHIFT;		break;

		case XK_Execute: 
		case XK_Control_L: 
		case XK_Control_R:	key = K_CTRL;		 break;

		case XK_Alt_L:	
		case XK_Meta_L: 
		case XK_Alt_R:	
		case XK_Meta_R: key = K_ALT;			break;

		case XK_KP_Begin: key = K_KP_5;	break;

		case XK_Insert:key = K_INS; break;
		case XK_KP_Insert: key = K_KP_INS; break;

		case XK_KP_Multiply: key = '*'; break;
		case XK_KP_Add:  key = K_KP_PLUS; break;
		case XK_KP_Subtract: key = K_KP_MINUS; break;
		case XK_KP_Divide: key = K_KP_SLASH; break;

		default:
			key = *(unsigned char*)buf;
			if (key >= 'A' && key <= 'Z')
				key = key - 'A' + 'a';
			if (key >= 1 && key <= 26) /* ctrl+alpha */
				key = key + 'a' - 1;
			break;
	} 

	return key;
}

void HandleEvents(void)
{
	XEvent event;
	qboolean dowarp = false;
	int mwx = vid.width / 2;
	int mwy = vid.height / 2;
	int multiclicktime = 750;
	int mouse_button;

	if (!dpy) 
		return;

	while (XPending(dpy))
	{
		XNextEvent (dpy, &event);
		
		if(event.xbutton.button == 2)
			mouse_button = 2;
		else if(event.xbutton.button == 3)
			mouse_button = 1;
		else
			mouse_button = event.xbutton.button - 1;

		switch (event.type) 
		{
			case KeyPress:
				myxtime = event.xkey.time;
			case KeyRelease:
				Key_Event (XLateKey(&event.xkey), event.type == KeyPress, Sys_Milliseconds());
				break;

			case MotionNotify:
				if (mouse_active)
				{
					if (dgamouse)
					{
						mx += event.xmotion.x_root * 2;
						my += event.xmotion.y_root * 2;
					}
					else
					{
						if( !event.xmotion.send_event ) {
    						    mx += event.xmotion.x - p_mouse_x;
						    my += event.xmotion.y - p_mouse_y;

						    if( abs( mwx - event.xmotion.x ) > mwx / 2 || abs( mwy - event.xmotion.y ) > mwy / 2 )
							dowarp = true;
						}
						p_mouse_x = event.xmotion.x;
						p_mouse_y = event.xmotion.y;
					}
				}
				break;

			case ButtonPress:
				myxtime = event.xbutton.time;	
				if (event.xbutton.button) {
					if (Sys_Milliseconds()-cursor.buttontime[mouse_button] < multiclicktime)
						cursor.buttonclicks[mouse_button] += 1;
					else
						cursor.buttonclicks[mouse_button] = 1;

				if (cursor.buttonclicks[mouse_button]>3)
					cursor.buttonclicks[mouse_button] = 3;

				cursor.buttontime[mouse_button] = Sys_Milliseconds();

				cursor.buttondown[mouse_button] = true;
				cursor.buttonused[mouse_button] = false;
				cursor.mouseaction = true;
				}
				if (event.xbutton.button == 1) Key_Event(K_MOUSE1, 1, Sys_Milliseconds());
				else if (event.xbutton.button == 2) Key_Event(K_MOUSE3, 1, Sys_Milliseconds());
				else if (event.xbutton.button == 3) Key_Event(K_MOUSE2, 1, Sys_Milliseconds());
				else if (event.xbutton.button == 4) Key_Event(K_MWHEELUP, 1, Sys_Milliseconds());
				else if (event.xbutton.button == 5) Key_Event(K_MWHEELDOWN, 1, Sys_Milliseconds());
				else if (event.xbutton.button >= 6 && event.xbutton.button <= 10) Key_Event(K_MOUSE4+event.xbutton.button-6, 1, Sys_Milliseconds());
				break;

			case ButtonRelease:
				if (event.xbutton.button) {
					cursor.buttondown[mouse_button] = false;
					cursor.buttonused[mouse_button] = false;
					cursor.mouseaction = true;
				}
				if (event.xbutton.button == 1) Key_Event(K_MOUSE1, 0, Sys_Milliseconds());
				else if (event.xbutton.button == 2) Key_Event(K_MOUSE3, 0, Sys_Milliseconds());
				else if (event.xbutton.button == 3) Key_Event(K_MOUSE2, 0, Sys_Milliseconds());
				else if (event.xbutton.button == 4) Key_Event(K_MWHEELUP, 0, Sys_Milliseconds());
				else if (event.xbutton.button == 5) Key_Event(K_MWHEELDOWN, 0, Sys_Milliseconds());
				else if (event.xbutton.button >= 6 && event.xbutton.button <= 10) Key_Event(K_MOUSE4+event.xbutton.button-6, 0, Sys_Milliseconds());
				break;

			case CreateNotify :
				win_x = event.xcreatewindow.x;
				win_y = event.xcreatewindow.y;
				break;

			case ConfigureNotify :
				win_x = event.xconfigure.x;
				win_y = event.xconfigure.y;
				break;

			case ClientMessage:
				if (event.xclient.data.l[0] == wmDeleteWindow) 
					Cbuf_ExecuteText(EXEC_NOW, "quit");
				break;
		}
	}

	if (dowarp) 
	{
		/* move the mouse to the window center again */
		p_mouse_x = mwx;
		p_mouse_y = mwy;
		XWarpPointer (dpy, None, win, 0, 0, 0, 0, mwx, mwy);
	}
}

/*****************************************************************************/

qboolean GLimp_InitGL (void);

static void signal_handler(int sig)
{
	printf("Received signal %d, exiting...\n", sig);
	GLimp_Shutdown();
	_exit(0);
}

static void InitSig(void)
{
	signal(SIGHUP, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGTRAP, signal_handler);
	signal(SIGIOT, signal_handler);
	signal(SIGBUS, signal_handler);
	signal(SIGFPE, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGTERM, signal_handler);
}

/*
** GLimp_SetMode
*/
int GLimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen )
{
	int width, height;
	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 1,
		GLX_STENCIL_SIZE, 1,
		None
	};
	Window root;
	XVisualInfo *visinfo;
	XSetWindowAttributes attr;
	unsigned long mask;
	int MajorVersion, MinorVersion;
	int actualWidth, actualHeight;
	XSizeHints *sizehints;
	XWMHints *wmhints;
	int i;

	r_fakeFullscreen = Cvar_Get( "r_fakeFullscreen", "0", CVAR_ARCHIVE);

	Com_Printf( "Initializing OpenGL display\n");

	if (fullscreen)
		Com_Printf ( "...setting fullscreen mode %d:", mode );
	else
		Com_Printf ( "...setting mode %d:", mode );

	if ( !VID_GetModeInfo( &width, &height, mode ) )
	{
		Com_Printf( " invalid mode\n" );
		return rserr_invalid_mode;
	}

	Com_Printf( " %dx%d\n", width, height );

	// destroy the existing window
	GLimp_Shutdown ();

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "Error couldn't open the X display\n");
		return rserr_invalid_mode;
	}

	scrnum = DefaultScreen(dpy);
	root = RootWindow(dpy, scrnum);

	// Get video mode list
	MajorVersion = MinorVersion = 0;
	if (!XF86VidModeQueryVersion(dpy, &MajorVersion, &MinorVersion)) { 
		vidmode_ext = false;
	} else {
		Com_Printf( "Using XFree86-VidModeExtension Version %d.%d\n",
			MajorVersion, MinorVersion);
		vidmode_ext = true;
	}

	visinfo = qglXChooseVisual(dpy, scrnum, attrib);
	if (!visinfo) {
		fprintf(stderr, "Error couldn't get an RGB, Double-buffered, Depth visual\n");
		return rserr_invalid_mode;
	}
	
	gl_state.gammaramp = false;

	if (vidmode_ext) {
		int best_fit, best_dist, dist, x, y;
		
		XF86VidModeGetAllModeLines(dpy, scrnum, &num_vidmodes, &vidmodes);

		// Are we going fullscreen?  If so, let's change video mode
		if (fullscreen && !r_fakeFullscreen->value) {
			best_dist = 9999999;
			best_fit = -1;

			for (i = 0; i < num_vidmodes; i++) {
				if (width > vidmodes[i]->hdisplay ||
					height > vidmodes[i]->vdisplay)
					continue;

				x = width - vidmodes[i]->hdisplay;
				y = height - vidmodes[i]->vdisplay;
				dist = (x * x) + (y * y);
				if (dist < best_dist) {
					best_dist = dist;
					best_fit = i;
				}
			}

			if (best_fit != -1) {
				actualWidth = vidmodes[best_fit]->hdisplay;
				actualHeight = vidmodes[best_fit]->vdisplay;

				// change to the mode
				XF86VidModeSwitchToMode(dpy, scrnum, vidmodes[best_fit]);
				vidmode_active = true;
				
				if (XF86VidModeGetGamma(dpy, scrnum, &oldgamma)) {
					gl_state.gammaramp = true;
					/* We can no reliably detect hardware gamma
					changes across software gamma calls, which
					can reset the flag, so change it anyway */
					vid_gamma->modified = true;
					Com_Printf( "Using hardware gamma\n");
				}

				// Move the viewport to top left
				XF86VidModeSetViewPort(dpy, scrnum, 0, 0);
			} else
				fullscreen = 0;
		}
	}

	/* window attributes */
	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(dpy, root, visinfo->visual, AllocNone);
	attr.event_mask = X_MASK;
	if (vidmode_active) {
		mask = CWBackPixel | CWColormap | CWSaveUnder | CWBackingStore | 
			CWEventMask | CWOverrideRedirect;
		attr.override_redirect = True;
		attr.backing_store = NotUseful;
		attr.save_under = False;
	} else
		mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	win = XCreateWindow(dpy, root, 0, 0, width, height,
						0, visinfo->depth, InputOutput,
						visinfo->visual, mask, &attr);
		
	XStoreName(dpy, win, "KMQuake2 0.19");
	
	sizehints = XAllocSizeHints();
	if (sizehints) {
		sizehints->min_width = width;
		sizehints->min_height = height;
		sizehints->max_width = width;
		sizehints->max_height = height;
		sizehints->base_width = width;
		sizehints->base_height = vid.height;
		
		sizehints->flags = PMinSize | PMaxSize | PBaseSize;
	}
	
	wmhints = XAllocWMHints();
	if (wmhints) {
		#include "q2icon.xbm"

		Pixmap icon_pixmap, icon_mask;
		unsigned long fg, bg;
		int i;
		
		fg = BlackPixel(dpy, visinfo->screen);
		bg = WhitePixel(dpy, visinfo->screen);
		icon_pixmap = XCreatePixmapFromBitmapData(dpy, win, (char *)q2icon_bits, q2icon_width, q2icon_height, fg, bg, visinfo->depth);
		for (i = 0; i < sizeof(q2icon_bits); i++)
			q2icon_bits[i] = ~q2icon_bits[i];
		icon_mask = XCreatePixmapFromBitmapData(dpy, win, (char *)q2icon_bits, q2icon_width, q2icon_height, bg, fg, visinfo->depth); 
	
		wmhints->flags = IconPixmapHint|IconMaskHint;
		wmhints->icon_pixmap = icon_pixmap;
		wmhints->icon_mask = icon_mask;
	}

	XSetWMProperties(dpy, win, NULL, NULL, NULL, 0,
			sizehints, wmhints, None);
	if (sizehints)
		XFree(sizehints);
	if (wmhints)
		XFree(wmhints);
	
	wmDeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &wmDeleteWindow, 1);
	XMapWindow(dpy, win);

	if (vidmode_active) {
		XMoveWindow(dpy, win, 0, 0);
		XRaiseWindow(dpy, win);
		XWarpPointer(dpy, None, win, 0, 0, 0, 0, 0, 0);
		XFlush(dpy);
		// Move the viewport to top left
		XF86VidModeSetViewPort(dpy, scrnum, 0, 0);
	}

	XFlush(dpy);

	ctx = qglXCreateContext(dpy, visinfo, NULL, True);

	qglXMakeCurrent(dpy, win, ctx);

	*pwidth = width;
	*pheight = height;

	// let the sound and input subsystems know about the new window
	VID_NewWindow (width, height);

	qglXMakeCurrent(dpy, win, ctx);

/*	Moved to GL_SetDefaultState in r_glstate.c
   // Vertex arrays
   qglEnableClientState (GL_VERTEX_ARRAY);
   qglEnableClientState (GL_TEXTURE_COORD_ARRAY);

   qglTexCoordPointer (2, GL_FLOAT, sizeof(tex_array[0]), tex_array[0]);
   qglVertexPointer (3, GL_FLOAT, sizeof(vert_array[0]), vert_array[0]);
   qglColorPointer (4, GL_FLOAT, sizeof(col_array[0]), col_array[0]);
*/

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
void GLimp_Shutdown( void )
{
	uninstall_grabs();
	mouse_active = false;
	dgamouse = false;

	if (dpy) {
		if (ctx)
			qglXDestroyContext(dpy, ctx);
		if (win)
			XDestroyWindow(dpy, win);
		if (gl_state.gammaramp) {
			XF86VidModeSetGamma(dpy, scrnum, &oldgamma);
			/* The gamma has changed, but SetMode will change it
			anyway, so why bother?
			vid_gamma->modified = true; */
		}
		if (vidmode_active)
			XF86VidModeSwitchToMode(dpy, scrnum, vidmodes[0]);
		XCloseDisplay(dpy);
	}
	ctx = NULL;
	dpy = NULL;
	win = 0;
	ctx = NULL;
}

/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.  
*/
int GLimp_Init( void *hinstance, void *wndproc )
{
	InitSig();
	
	Com_Printf( "... Using stencil buffer\n" );	
	have_stencil = true; // Stencil shadows - MrG

	return true;
}

/*
** GLimp_BeginFrame
*/
void CIN_ProcessCins(void);
void GLimp_BeginFrame( float camera_seperation )
{
	//Heffo - CIN Texture Update
	CIN_ProcessCins();
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
	qglFlush();
	qglXSwapBuffers(dpy, win);
}

/*
** GLimp_AppActivate
*/
void GLimp_AppActivate( qboolean active )
{
}

void Fake_glColorTableEXT( GLenum target, GLenum internalformat,
                             GLsizei width, GLenum format, GLenum type,
                             const GLvoid *table )
{
	byte temptable[256][4];
	byte *intbl;
	int i;

	for (intbl = (byte *)table, i = 0; i < 256; i++) {
		temptable[i][2] = *intbl++;
		temptable[i][1] = *intbl++;
		temptable[i][0] = *intbl++;
		temptable[i][3] = 255;
	}
	qgl3DfxSetPaletteEXT((GLuint *)temptable);
}

/*
** UpdateHardwareGamma
**
** We are using gamma relative to the desktop, so that we can share it
** with software renderer and don't require to change desktop gamma
** to match hardware gamma image brightness. It seems that Quake 3 is
** using the opposite approach, but it has no software renderer after
** all.
*/
void UpdateGammaRamp (void)
{
	XF86VidModeGamma gamma;
	float g;

	g = (1.3 - vid_gamma->value + 1);
	g = (g>1 ? g : 1);
	gamma.red = oldgamma.red * g;
	gamma.green = oldgamma.green * g;
	gamma.blue = oldgamma.blue * g;
	XF86VidModeSetGamma(dpy, scrnum, &gamma);
}

char *Sys_GetClipboardData()
{
	Window sowner;
	Atom type, property;
	unsigned long len, bytes_left, tmp;
	unsigned char *data;
	int format, result;
	char *ret = NULL;
			
	sowner = XGetSelectionOwner(dpy, XA_PRIMARY);
			
	if (sowner != None) {
		property = XInternAtom(dpy,
				       "GETCLIPBOARDDATA_PROP",
				       False);
				
		XConvertSelection(dpy,
				  XA_PRIMARY, XA_STRING,
				  property, win, myxtime); /* myxtime == time of last X event */
		XFlush(dpy);

		XGetWindowProperty(dpy,
				   win, property,
				   0, 0, False, AnyPropertyType,
				   &type, &format, &len,
				   &bytes_left, &data);
		if (bytes_left > 0) {
			result =
			XGetWindowProperty(dpy,
					   win, property,
					   0, bytes_left, True, AnyPropertyType,
					   &type, &format, &len,
					   &tmp, &data);
			if (result == Success) {
				ret = strdup((char *)data);
			}
			XFree(data);
		}
	}
	return ret;
}

/*------------------------------------------------*/
/* X11 Input Stuff */
/*------------------------------------------------*/

