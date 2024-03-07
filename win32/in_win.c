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

// in_win.c -- windows 95 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.

#include "../client/client.h"
#include "winquake.h"
#include "../ui/ui_local.h"

extern	unsigned	sys_msg_time;

// joystick defines and variables
// where should defines be moved?
#define JOY_ABSOLUTE_AXIS	0x00000000		// control like a joystick
#define JOY_RELATIVE_AXIS	0x00000010		// control like a mouse, spinner, trackball
#define	JOY_MAX_AXES		6				// X, Y, Z, R, U, V
#define JOY_AXIS_X			0
#define JOY_AXIS_Y			1
#define JOY_AXIS_Z			2
#define JOY_AXIS_R			3
#define JOY_AXIS_U			4
#define JOY_AXIS_V			5

enum _ControlList
{
	AxisNada = 0, AxisForward, AxisLook, AxisSide, AxisTurn, AxisUp
};

DWORD	dwAxisFlags[JOY_MAX_AXES] =
{
	JOY_RETURNX, JOY_RETURNY, JOY_RETURNZ, JOY_RETURNR, JOY_RETURNU, JOY_RETURNV
};

DWORD	dwAxisMap[JOY_MAX_AXES];
DWORD	dwControlMap[JOY_MAX_AXES];
PDWORD	pdwRawValue[JOY_MAX_AXES];

cvar_t	*m_noaccel; //sul
cvar_t	*in_mouse;
cvar_t	*in_joystick;

cvar_t	*in_autosensitivity;


// none of these cvars are saved over a session
// this means that advanced controller configuration needs to be executed
// each time.  this avoids any problems with getting back to a default usage
// or when changing from one controller to another.  this way at least something
// works.
cvar_t	*joy_name;
cvar_t	*joy_advanced;
cvar_t	*joy_advaxisx;
cvar_t	*joy_advaxisy;
cvar_t	*joy_advaxisz;
cvar_t	*joy_advaxisr;
cvar_t	*joy_advaxisu;
cvar_t	*joy_advaxisv;
cvar_t	*joy_forwardthreshold;
cvar_t	*joy_sidethreshold;
cvar_t	*joy_pitchthreshold;
cvar_t	*joy_yawthreshold;
cvar_t	*joy_forwardsensitivity;
cvar_t	*joy_sidesensitivity;
cvar_t	*joy_pitchsensitivity;
cvar_t	*joy_yawsensitivity;
cvar_t	*joy_upthreshold;
cvar_t	*joy_upsensitivity;

qboolean	joy_avail, joy_advancedinit, joy_haspov;
DWORD		joy_oldbuttonstate, joy_oldpovstate;

int			joy_id;
DWORD		joy_flags;
DWORD		joy_numbuttons;

static JOYINFOEX	ji;

extern cursor_t ui_mousecursorcursor;

qboolean	in_appactive;

// forward-referenced functions
void IN_StartupJoystick (void);
void Joy_AdvancedUpdate_f (void);
void IN_JoyMove (usercmd_t *cmd);

/*
============================================================

  MOUSE CONTROL

============================================================
*/

// mouse variables
cvar_t	*m_filter;

qboolean	mlooking;

void IN_MLookDown (void) { mlooking = true; }
void IN_MLookUp (void) {
	mlooking = false;
//	if (!freelook->value && lookspring->value)
	if (!freelook->integer && lookspring->integer)
		IN_CenterView ();
}

int			mouse_buttons;
int			mouse_oldbuttonstate;
POINT		current_pos;
int			mouse_x, mouse_y, old_mouse_x, old_mouse_y, mx_accum, my_accum;

int			old_x, old_y;

qboolean	mouseactive;	// false when not focus app

qboolean	restore_spi;
qboolean	mouseinitialized;
int		originalmouseparms[3], newmouseparms[3] = {0, 0, 1};
qboolean	mouseparmsvalid;

int			window_center_x, window_center_y;
RECT		window_rect;

qboolean	joystickVerticalMoved;

/*
===========
IN_ActivateMouse

Called when the window gains focus or changes in some way
===========
*/
void IN_ActivateMouse (void)
{
	int		width, height;

	if (!mouseinitialized)
		return;
//	if (!in_mouse->value)
	if (!in_mouse->integer)
	{
		mouseactive = false;
		return;
	}
	if (mouseactive)
		return;

	mouseactive = true;

//	if (m_noaccel->value) 
	if (m_noaccel->integer) 
		newmouseparms[2]=0; //sul XP fix?
	else 
		newmouseparms[2]=1;

	if (mouseparmsvalid)
		restore_spi = SystemParametersInfo (SPI_SETMOUSE, 0, newmouseparms, 0);

	width = GetSystemMetrics (SM_CXSCREEN);
	height = GetSystemMetrics (SM_CYSCREEN);

	GetWindowRect ( cl_hwnd, &window_rect);
	if (window_rect.left < 0)
		window_rect.left = 0;
	if (window_rect.top < 0)
		window_rect.top = 0;
	if (window_rect.right >= width)
		window_rect.right = width-1;
	if (window_rect.bottom >= height-1)
		window_rect.bottom = height-1;

	window_center_x = (window_rect.right + window_rect.left)/2;
	window_center_y = (window_rect.top + window_rect.bottom)/2;

	SetCursorPos (window_center_x, window_center_y);

	old_x = window_center_x;
	old_y = window_center_y;

	SetCapture ( cl_hwnd );
	ClipCursor (&window_rect);
	while (ShowCursor (FALSE) >= 0)
		;
}


/*
===========
IN_DeactivateMouse

Called when the window loses focus
===========
*/
void IN_DeactivateMouse (void)
{
	if (!mouseinitialized)
		return;
	if (!mouseactive)
		return;

	if (restore_spi)
		SystemParametersInfo (SPI_SETMOUSE, 0, originalmouseparms, 0);

	mouseactive = false;

	ClipCursor (NULL);
	ReleaseCapture ();
	while (ShowCursor (TRUE) < 0)
		;
}


/*
===========
IN_StartupMouse
===========
*/
void UI_RefreshCursorMenu (void);
void UI_RefreshCursorLink (void);
//void MW_Set_Hook (void);
//void MW_Shutdown (void);
void IN_StartupMouse (void)
{
	cvar_t		*cv;

	cv = Cvar_Get ("in_initmouse", "1", CVAR_NOSET);
	Cvar_SetDescription ("in_initmouse", "Enables the initialization of the mouse.");
//	if ( !cv->value ) 
	if ( !cv->integer ) 
		return; 

	// Knightmare- added Psychospaz's menu mouse support
	UI_RefreshCursorMenu();
	UI_RefreshCursorLink();

	ui_mousecursor.mouseaction = false;

	mouseinitialized = true;
	mouseparmsvalid = SystemParametersInfo (SPI_GETMOUSE, 0, originalmouseparms, 0);
	//MW_Set_Hook(); 	// Logitech mouse support
	mouse_buttons = 5; // was 3
}

/*
===========
IN_MouseEvent
===========
*/
void IN_MouseEvent (int mstate)
{
	int		i;

	if (!mouseinitialized)
		return;

// perform button actions
	for (i=0 ; i<mouse_buttons ; i++)
	{
		if ( (mstate & (1<<i)) &&
			!(mouse_oldbuttonstate & (1<<i)) )
		{
			Key_Event (K_MOUSE1 + i, true, sys_msg_time);
		}

		if ( !(mstate & (1<<i)) &&
			(mouse_oldbuttonstate & (1<<i)) )
		{
			Key_Event (K_MOUSE1 + i, false, sys_msg_time);
		}
	}	

	// set menu cursor buttons
	if (cls.key_dest == key_menu)
	{
		int multiclicktime = 750;
		int max = mouse_buttons;
		if (max > MENU_CURSOR_BUTTON_MAX) max = MENU_CURSOR_BUTTON_MAX;

		for (i=0 ; i<max ; i++)
		{
			if ( (mstate & (1<<i)) && !(mouse_oldbuttonstate & (1<<i)))
			{	// mouse press down
				if (sys_msg_time - ui_mousecursor.buttontime[i] < multiclicktime)
					ui_mousecursor.buttonclicks[i] += 1;
				else
					ui_mousecursor.buttonclicks[i] = 1;

				if (ui_mousecursor.buttonclicks[i]>max)
					ui_mousecursor.buttonclicks[i] = max;

				ui_mousecursor.buttontime[i] = sys_msg_time;

				ui_mousecursor.buttondown[i] = true;
				ui_mousecursor.buttonused[i] = false;
				ui_mousecursor.mouseaction = true;
			}
			else if ( !(mstate & (1<<i)) &&	(mouse_oldbuttonstate & (1<<i)) )
			{	// mouse let go
				ui_mousecursor.buttondown[i] = false;
				ui_mousecursor.buttonused[i] = false;
				ui_mousecursor.mouseaction = true;
			}
		}			
	}	

	mouse_oldbuttonstate = mstate;
}


/*
===========
IN_MouseMove
===========
*/
void IN_MouseMove (usercmd_t *cmd)
{
	int		mx, my;

	if (!in_autosensitivity)
		in_autosensitivity = Cvar_Get ("in_autosensitivity", "1", CVAR_ARCHIVE);

	if (!mouseactive)
		return;

	// find mouse movement
	if (!GetCursorPos (&current_pos))
		return;

	mx = current_pos.x - window_center_x;
	my = current_pos.y - window_center_y;

#if 0
	if (!mx && !my)
		return;
#endif

//	if (m_filter->value)
	if (m_filter->integer)
	{
		mouse_x = (mx + old_mouse_x) * 0.5;
		mouse_y = (my + old_mouse_y) * 0.5;
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}

	old_mouse_x = mx;
	old_mouse_y = my;

	//now to set the menu cursor
	if (cls.key_dest == key_menu)
	{
		ui_mousecursor.oldx = ui_mousecursor.x;
		ui_mousecursor.oldy = ui_mousecursor.y;

		ui_mousecursor.x += mouse_x * ui_sensitivity->value;
		ui_mousecursor.y += mouse_y * ui_sensitivity->value;

		if ( (ui_mousecursor.x != ui_mousecursor.oldx) || (ui_mousecursor.y != ui_mousecursor.oldy) )
			ui_mousecursor.mouseaction = true;

		if (ui_mousecursor.x < 0) ui_mousecursor.x = 0;
		if (ui_mousecursor.x > viddef.width) ui_mousecursor.x = viddef.width;
		if (ui_mousecursor.y < 0) ui_mousecursor.y = 0;
		if (ui_mousecursor.y > viddef.height) ui_mousecursor.y = viddef.height;
	}
	else
	{
		ui_mousecursor.oldx = 0;
		ui_mousecursor.oldy = 0;

		// psychospaz - zooming in preserves sensitivity
	//	if (in_autosensitivity->value && cl.base_fov < 90)
		if (in_autosensitivity->integer && cl.base_fov < 90)
		{
			mouse_x *= sensitivity->value * (cl.base_fov/90.0);
			mouse_y *= sensitivity->value * (cl.base_fov/90.0);
		}
		else
		{
			mouse_x *= sensitivity->value;
			mouse_y *= sensitivity->value;
		}

	// add mouse X/Y movement to cmd
	//	if ( (in_strafe.state & 1) || (lookstrafe->value && mlooking ))
		if ( (in_strafe.state & 1) || (lookstrafe->integer && mlooking ))
			cmd->sidemove += m_side->value * mouse_x;
		else
			cl.viewangles[YAW] -= m_yaw->value * mouse_x;

	//	if ( (mlooking || freelook->value) && !(in_strafe.state & 1))
		if ( (mlooking || freelook->integer) && !(in_strafe.state & 1))
		{
			cl.viewangles[PITCH] += m_pitch->value * mouse_y;
		}
		else
		{
			cmd->forwardmove -= m_forward->value * mouse_y;
		}
	}

	// force the mouse to the center, so there's room to move
	if (mx || my)
		SetCursorPos (window_center_x, window_center_y);
}

/*
=========================================================================

VIEW CENTERING

=========================================================================
*/

cvar_t	*v_centermove;
cvar_t	*v_centerspeed;


/*
===========
IN_Init
===========
*/
void IN_Init (void)
{
	// mouse variables
	in_autosensitivity		= Cvar_Get ("in_autosensitivity",			"1",		CVAR_ARCHIVE);
	Cvar_SetDescription ("in_autosensitivity", "Enables scaling of mouse and joystick sensitivty when zoomed in.");
	m_noaccel				= Cvar_Get ("m_noaccel",				"0",		CVAR_ARCHIVE); //sul  enables mouse acceleration XP fix?
	Cvar_SetDescription ("m_noaccel", "Disables mouse acceleration when set to 1.");
	m_filter				= Cvar_Get ("m_filter",					"0",		0);
	Cvar_SetDescription ("m_filter", "Enables mouse input filtering.");
    in_mouse				= Cvar_Get ("in_mouse",					"1",		CVAR_ARCHIVE);
	Cvar_SetDescription ("in_mouse", "Enables mouse input.");

	// joystick variables
	in_joystick				= Cvar_Get ("in_joystick",				"0",		CVAR_ARCHIVE);
	Cvar_SetDescription ("in_joystick", "Enables joystick input.");
	joy_name				= Cvar_Get ("joy_name",					"joystick",	0);
	Cvar_SetDescription ("joy_name", "Sets name of the joystick.");
	joy_advanced			= Cvar_Get ("joy_advanced",				"0",		0);
	Cvar_SetDescription ("joy_advanced", "Enables the advanced axis (\"joy_advaxis*\") cvars.  Use the \"joy_advancedupdate\" command after setting this and the \"joy_advaxis*\" cvars.");

	joy_advaxisx			= Cvar_Get ("joy_advaxisx",				"0",		0);
	Cvar_SetDescription ("joy_advaxisx", "Controls mapping of DirectInput axis X (typically joystick left and right).");
	joy_advaxisy			= Cvar_Get ("joy_advaxisy",				"0",		0);
	Cvar_SetDescription ("joy_advaxisy", "Controls mapping of DirectInput axis Y (typically joystick forward and backward).");
	joy_advaxisz			= Cvar_Get ("joy_advaxisz",				"0",		0);
	Cvar_SetDescription ("joy_advaxisz", "Controls mapping of DirectInput axis Z (typically joystick throttle).");
	joy_advaxisr			= Cvar_Get ("joy_advaxisr",				"0",		0);
	Cvar_SetDescription ("joy_advaxisr", "Controls mapping of DirectInput axis R (typically joystick rudder).");
	joy_advaxisu			= Cvar_Get ("joy_advaxisu",				"0",		0);
	Cvar_SetDescription ("joy_advaxisu", "Controls mapping of DirectInput axis U (custom axis - Assassin 3D trackball left and right and SpaceOrb roll).");
	joy_advaxisv			= Cvar_Get ("joy_advaxisv",				"0",		0);
	Cvar_SetDescription ("joy_advaxisv", "Controls mapping of DirectInput axis V (custom axis - Assassin 3D trackball forward and backward and SpaceOrb yaw).");

	joy_forwardthreshold	= Cvar_Get ("joy_forwardthreshold",		"0.15",		0);
	Cvar_SetDescription ("joy_forwardthreshold", "Controls the dead-zone for moving forward and backward.");
	joy_sidethreshold		= Cvar_Get ("joy_sidethreshold",		"0.15",		0);
	Cvar_SetDescription ("joy_sidethreshold", "Controls the dead-zone for moving side to side.");
	joy_upthreshold  		= Cvar_Get ("joy_upthreshold",			"0.15",		0);
	Cvar_SetDescription ("joy_upthreshold", "Controls the dead-zone for moving up and down.");
	joy_pitchthreshold		= Cvar_Get ("joy_pitchthreshold",		"0.15",		0);
	Cvar_SetDescription ("joy_pitchthreshold", "Controls the dead-zone for looking up and down.");
	joy_yawthreshold		= Cvar_Get ("joy_yawthreshold",			"0.15",		0);
	Cvar_SetDescription ("joy_yawthreshold", "Controls the dead-zone for looking left and right.");
	joy_forwardsensitivity	= Cvar_Get ("joy_forwardsensitivity",	"-1",		0);
	Cvar_SetDescription ("joy_forwardsensitivity", "Controls the ramp-up speed for moving forward and backward.");
	joy_sidesensitivity		= Cvar_Get ("joy_sidesensitivity",		"-1",		0);
	Cvar_SetDescription ("joy_sidesensitivity", "Controls the ramp-up speed for moving side to side.");
	joy_upsensitivity		= Cvar_Get ("joy_upsensitivity",		"-1",		0);
	Cvar_SetDescription ("joy_upsensitivity", "Controls the ramp-up speed for moving up and down.");
	joy_pitchsensitivity	= Cvar_Get ("joy_pitchsensitivity",		"1",		0);
	Cvar_SetDescription ("joy_pitchsensitivity", "Controls the speed that you look up and down.");
	joy_yawsensitivity		= Cvar_Get ("joy_yawsensitivity",		"-1",		0);
	Cvar_SetDescription ("joy_yawsensitivity", "Controls the speed that you look left to right.");

	// centering
	v_centermove			= Cvar_Get ("v_centermove",				"0.15",		0);
	v_centerspeed			= Cvar_Get ("v_centerspeed",			"500",		0);

	Cmd_AddCommand ("+mlook", IN_MLookDown);
	Cmd_AddCommand ("-mlook", IN_MLookUp);

	Cmd_AddCommand ("joy_advancedupdate", Joy_AdvancedUpdate_f);

	IN_StartupMouse ();
	IN_StartupJoystick ();

	joystickVerticalMoved = false;
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown (void)
{
	IN_DeactivateMouse ();
	//MW_Shutdown(); // Logitech mouse support
}


/*
===========
IN_Activate

Called when the main window gains or loses focus.
The window may have been destroyed and recreated
between a deactivate and an activate.
===========
*/
void IN_Activate (qboolean active)
{
	in_appactive = active;
	mouseactive = !active;		// force a new window check or turn off
}


/*
==================
IN_Frame

Called every frame, even if not generating commands
==================
*/
void IN_Frame (void)
{
	if (!mouseinitialized)
		return;

	if (!in_mouse || !in_appactive)
	{
		IN_DeactivateMouse ();
		return;
	}

//	if ( !cl.refresh_prepped
//		|| cls.key_dest == key_console
//		|| cls.key_dest == key_menu)
	//Knightmare- added Psychospaz's mouse menu support
	if ( (!cl.refresh_prepped && cls.key_dest != key_menu) || cls.consoleActive) //mouse used in menus...
	{
		// temporarily deactivate if in fullscreen
		if (Cvar_VariableValue ("vid_fullscreen") == 0)
		{
			IN_DeactivateMouse ();
			return;
		}
	}

	IN_ActivateMouse ();
}

/*
===========
IN_Move
===========
*/
void IN_Move (usercmd_t *cmd)
{
	if (!ActiveApp)
		return;

	IN_MouseMove (cmd);

	// Knightmare- added Psychospaz's mouse support
	if (cls.key_dest == key_menu && !cls.consoleActive) // Knightmare added
		UI_MouseCursor_Think ();

	//if (ActiveApp)
		IN_JoyMove (cmd);
}


/*
===================
IN_ClearStates
===================
*/
void IN_ClearStates (void)
{
	mx_accum = 0;
	my_accum = 0;
	mouse_oldbuttonstate = 0;
}


/*
=========================================================================

JOYSTICK

=========================================================================
*/

/* 
=============== 
IN_StartupJoystick 
=============== 
*/  
void IN_StartupJoystick (void) 
{ 
	int			numdevs;
	JOYCAPS		jc;
	MMRESULT	mmr;
	cvar_t		*cv;

 	// assume no joystick
	joy_avail = false; 

	// abort startup if user requests no joystick
	cv = Cvar_Get ("in_initjoy", "1", CVAR_NOSET);
	Cvar_SetDescription ("in_initjoy", "Enables the initialization of the joystick.");
//	if ( !cv->value ) 
	if ( !cv->integer ) 
		return; 
 
	// verify joystick driver is present
	if ((numdevs = joyGetNumDevs ()) == 0)
	{
//		Com_Printf ("\njoystick not found -- driver not present\n\n");
		return;
	}

	// cycle through the joystick ids for the first valid one
	for (joy_id=0 ; joy_id<numdevs ; joy_id++)
	{
		memset (&ji, 0, sizeof(ji));
		ji.dwSize = sizeof(ji);
		ji.dwFlags = JOY_RETURNCENTERED;

		if ((mmr = joyGetPosEx (joy_id, &ji)) == JOYERR_NOERROR)
			break;
	} 

	// abort startup if we didn't find a valid joystick
	if (mmr != JOYERR_NOERROR)
	{
		Com_Printf ("\njoystick not found -- no valid joysticks (%x)\n\n", mmr);
		return;
	}

	// get the capabilities of the selected joystick
	// abort startup if command fails
	memset (&jc, 0, sizeof(jc));
	if ((mmr = joyGetDevCaps (joy_id, &jc, sizeof(jc))) != JOYERR_NOERROR)
	{
		Com_Printf ("\njoystick not found -- invalid joystick capabilities (%x)\n\n", mmr); 
		return;
	}

	// save the joystick's number of buttons and POV status
	joy_numbuttons = jc.wNumButtons;
	joy_haspov = jc.wCaps & JOYCAPS_HASPOV;

	// old button and POV states default to no buttons pressed
	joy_oldbuttonstate = joy_oldpovstate = 0;

	// mark the joystick as available and advanced initialization not completed
	// this is needed as cvars are not available during initialization

	joy_avail = true; 
	joy_advancedinit = false;

	Com_Printf ("\njoystick detected\n\n"); 
}


/*
===========
RawValuePointer
===========
*/
PDWORD RawValuePointer (int axis)
{
	switch (axis)
	{
	case JOY_AXIS_X:
		return &ji.dwXpos;
	case JOY_AXIS_Y:
		return &ji.dwYpos;
	case JOY_AXIS_Z:
		return &ji.dwZpos;
	case JOY_AXIS_R:
		return &ji.dwRpos;
	case JOY_AXIS_U:
		return &ji.dwUpos;
	case JOY_AXIS_V:
		return &ji.dwVpos;
	}
	// We need a default return
	return &ji.dwXpos;
}


/*
===========
Joy_AdvancedUpdate_f
===========
*/
void Joy_AdvancedUpdate_f (void)
{

	// called once by IN_ReadJoystick and by user whenever an update is needed
	// cvars are now available
	int	i;
	DWORD dwTemp;

	// initialize all the maps
	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		dwAxisMap[i] = AxisNada;
		dwControlMap[i] = JOY_ABSOLUTE_AXIS;
		pdwRawValue[i] = RawValuePointer(i);
	}

	if( joy_advanced->value == 0.0)
	{
		// default joystick initialization
		// 2 axes only with joystick control
		dwAxisMap[JOY_AXIS_X] = AxisTurn;
		// dwControlMap[JOY_AXIS_X] = JOY_ABSOLUTE_AXIS;
		dwAxisMap[JOY_AXIS_Y] = AxisForward;
		// dwControlMap[JOY_AXIS_Y] = JOY_ABSOLUTE_AXIS;
	}
	else
	{
		if (strcmp (joy_name->string, "joystick") != 0)
		{
			// notify user of advanced controller
			Com_Printf ("\n%s configured\n\n", joy_name->string);
		}

		// advanced initialization here
		// data supplied by user via joy_axisn cvars
		dwTemp = (DWORD) joy_advaxisx->value;
		dwAxisMap[JOY_AXIS_X] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_X] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisy->value;
		dwAxisMap[JOY_AXIS_Y] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Y] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisz->value;
		dwAxisMap[JOY_AXIS_Z] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Z] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisr->value;
		dwAxisMap[JOY_AXIS_R] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_R] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisu->value;
		dwAxisMap[JOY_AXIS_U] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_U] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisv->value;
		dwAxisMap[JOY_AXIS_V] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_V] = dwTemp & JOY_RELATIVE_AXIS;
	}

	// compute the axes to collect from DirectInput
	joy_flags = JOY_RETURNCENTERED | JOY_RETURNBUTTONS | JOY_RETURNPOV;
	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		if (dwAxisMap[i] != AxisNada)
		{
			joy_flags |= dwAxisFlags[i];
		}
	}
}


/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
	int		i, key_index;
	DWORD	buttonstate, povstate;

	if (!joy_avail)
	{
		return;
	}

	
	// loop through the joystick buttons
	// key a joystick event or auxillary event for higher number buttons for each state change
	buttonstate = ji.dwButtons;
	for (i=0 ; i < joy_numbuttons ; i++)
	{
		if ( (buttonstate & (1<<i)) && !(joy_oldbuttonstate & (1<<i)) )
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, true, 0);
		}

		if ( !(buttonstate & (1<<i)) && (joy_oldbuttonstate & (1<<i)) )
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, false, 0);
		}
	}
	joy_oldbuttonstate = buttonstate;

	if (joy_haspov)
	{
		// convert POV information into 4 bits of state information
		// this avoids any potential problems related to moving from one
		// direction to another without going through the center position
		povstate = 0;
		if(ji.dwPOV != JOY_POVCENTERED)
		{
			if (ji.dwPOV == JOY_POVFORWARD)
				povstate |= 0x01;
			if (ji.dwPOV == JOY_POVRIGHT)
				povstate |= 0x02;
			if (ji.dwPOV == JOY_POVBACKWARD)
				povstate |= 0x04;
			if (ji.dwPOV == JOY_POVLEFT)
				povstate |= 0x08;
		}
		// determine which bits have changed and key an auxillary event for each change
		for (i=0 ; i < 4 ; i++)
		{
			if ( (povstate & (1<<i)) && !(joy_oldpovstate & (1<<i)) )
			{
				Key_Event (K_AUX29 + i, true, 0);
			}

			if ( !(povstate & (1<<i)) && (joy_oldpovstate & (1<<i)) )
			{
				Key_Event (K_AUX29 + i, false, 0);
			}
		}
		joy_oldpovstate = povstate;
	}
}


/* 
=============== 
IN_ReadJoystick
=============== 
*/  
qboolean IN_ReadJoystick (void)
{

	memset (&ji, 0, sizeof(ji));
	ji.dwSize = sizeof(ji);
	ji.dwFlags = joy_flags;

	if (joyGetPosEx (joy_id, &ji) == JOYERR_NOERROR)
	{
		return true;
	}
	else
	{
		// read error occurred
		// turning off the joystick seems too harsh for 1 read error,\
		// but what should be done?
		// Com_Printf ("IN_ReadJoystick: no response\n");
		// joy_avail = false;
		return false;
	}
}


/*
===========
IN_JoyMove
===========
*/
void IN_JoyMove (usercmd_t *cmd)
{
	float	speed, aspeed;
	float	fAxisValue;
	int		i;

	// complete initialization if first time in
	// this is needed as cvars are not available at initialization time
	if( joy_advancedinit != true )
	{
		Joy_AdvancedUpdate_f();
		joy_advancedinit = true;
	}

	// verify joystick is available and that the user wants to use it
//	if (!joy_avail || !in_joystick->value)
	if (!joy_avail || !in_joystick->integer)
	{
		return; 
	}
 
	// collect the joystick data, if possible
	if (IN_ReadJoystick () != true)
	{
		return;
	}

	if ( (in_speed.state & 1) ^ (int)cl_run->value)
		speed = 2;
	else
		speed = 1;
	aspeed = speed * cls.netFrameTime;

	// loop through the axes
	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		// get the floating point zero-centered, potentially-inverted data for the current axis
		fAxisValue = (float) *pdwRawValue[i];
		// move centerpoint to zero
		fAxisValue -= 32768.0;

		// convert range from -32768..32767 to -1..1 
		fAxisValue /= 32768.0;

		switch (dwAxisMap[i])
		{
		case AxisForward:
			if ((joy_advanced->value == 0.0) && mlooking)
			{
				// user wants forward control to become look control
				if (fabs(fAxisValue) > joy_pitchthreshold->value)
				{		
					// if mouse invert is on, invert the joystick pitch value
					// only absolute control support here (joy_advanced is false)
					if (m_pitch->value < 0.0)
					{
						if (in_autosensitivity->value && cl.base_fov < 90) // Knightmare added
							cl.viewangles[PITCH] -= (fAxisValue * joy_pitchsensitivity->value * (cl.base_fov/90.0)) * aspeed * cl_pitchspeed->value;
						else
							cl.viewangles[PITCH] -= (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
					}
					else
					{
					//	if (in_autosensitivity->value && cl.base_fov < 90) // Knightmare added
						if (in_autosensitivity->integer && cl.base_fov < 90) // Knightmare added
							cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity->value * (cl.base_fov/90.0)) * aspeed * cl_pitchspeed->value;
						else
							cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
					}
				}
			}
			else
			{
				// user wants forward control to be forward control
				if (fabs(fAxisValue) > joy_forwardthreshold->value && cl.frame.playerstate.stats[STAT_FREEZE] <= 0 /*BC 1/30/2024 don't allow joystick movement during stat_freeze*/ )
				{
					if (cl.frame.playerstate.stats[STAT_MOVESLOW]) //BC 1/30/2024 slow down movement during stat_moveslow
						speed *= .5f;

					cmd->forwardmove += (fAxisValue * joy_forwardsensitivity->value) * speed * cl_forwardspeed->value;
				}
			}


			//BC 1/30/2024 allow joystick menu selection.
			if (fabs(fAxisValue) > joy_forwardthreshold->value)
			{
				if (fAxisValue < 0)
				{
					//joystick up.
					//Com_Printf("joystick up\n");
					Key_Event(K_JOY_UP, true, sys_msg_time);
				}
				else
				{
					//joystick down.
					Key_Event(K_JOY_DOWN, true, sys_msg_time);
				}

				joystickVerticalMoved = true;
			}
			else if (joystickVerticalMoved == true)
			{
				joystickVerticalMoved = false;
				Key_Event(K_JOY_UP, false, 0);
				Key_Event(K_JOY_DOWN, false, 0);
			}

			break;

		case AxisSide:
			if (fabs(fAxisValue) > joy_sidethreshold->value)
			{
				cmd->sidemove += (fAxisValue * joy_sidesensitivity->value) * speed * cl_sidespeed->value;
			}
			break;

		case AxisUp:
			if (fabs(fAxisValue) > joy_upthreshold->value)
			{
				cmd->upmove += (fAxisValue * joy_upsensitivity->value) * speed * cl_upspeed->value;
			}
			break;

		case AxisTurn:
			if ((in_strafe.state & 1) || (lookstrafe->value && mlooking))
			{
				// user wants turn control to become side control
				if (fabs(fAxisValue) > joy_sidethreshold->value)
				{
					cmd->sidemove -= (fAxisValue * joy_sidesensitivity->value) * speed * cl_sidespeed->value;
				}
			}
			else
			{
				// user wants turn control to be turn control
				if (fabs(fAxisValue) > joy_yawthreshold->value)
				{
					if(dwControlMap[i] == JOY_ABSOLUTE_AXIS)
					{
					//	if (in_autosensitivity->value && cl.base_fov < 90) // Knightmare added
						if (in_autosensitivity->integer && cl.base_fov < 90) // Knightmare added
							cl.viewangles[YAW] += (fAxisValue * joy_yawsensitivity->value * (cl.base_fov/90.0)) * aspeed * cl_yawspeed->value;
						else
							cl.viewangles[YAW] += (fAxisValue * joy_yawsensitivity->value) * aspeed * cl_yawspeed->value;
					}
					else
					{
					//	if (in_autosensitivity->value && cl.base_fov < 90) // Knightmare added
						if (in_autosensitivity->integer && cl.base_fov < 90) // Knightmare added
							cl.viewangles[YAW] += (fAxisValue * joy_yawsensitivity->value * (cl.base_fov/90.0)) * speed * 180.0;
						else
							cl.viewangles[YAW] += (fAxisValue * joy_yawsensitivity->value) * speed * 180.0;
					}

				}
			}
			break;

		case AxisLook:
			if (mlooking)
			{
				if (fabs(fAxisValue) > joy_pitchthreshold->value)
				{
					// pitch movement detected and pitch movement desired by user
					if(dwControlMap[i] == JOY_ABSOLUTE_AXIS)
					{
					//	if (in_autosensitivity->value && cl.base_fov < 90) // Knightmare added
						if (in_autosensitivity->integer && cl.base_fov < 90) // Knightmare added
							cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity->value * (cl.base_fov/90.0)) * aspeed * cl_pitchspeed->value;
						else
							cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
					}
					else
					{
					//	if (in_autosensitivity->value && cl.base_fov < 90) // Knightmare added
						if (in_autosensitivity->integer && cl.base_fov < 90) // Knightmare added
							cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity->value * (cl.base_fov/90.0)) * speed * 180.0;
						else
							cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity->value) * speed * 180.0;
					}
				}
			}
			break;

		default:
			break;
		}
	}
}

