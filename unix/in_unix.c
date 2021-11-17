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

// in_unix.c -- unix mouse and joystick code

#include "../client/client.h"
#include "../ui/ui_local.h"

cvar_t	*in_mouse;
cvar_t	*in_joystick;
extern int mx, my;
extern float controller_leftx, controller_lefty, controller_rightx, controller_righty;
static qboolean	mouse_avail;
static int old_mouse_x, old_mouse_y;

static qboolean	mlooking;
cvar_t	*m_filter;
cvar_t	*in_dgamouse;
cvar_t	*in_autosensitivity;
cvar_t	*in_menumouse; /// FIXME Menu Mouse on windowed mode 

extern cursor_t ui_mousecursor;

void UI_RefreshCursorMenu (void);
void UI_RefreshCursorLink (void);

void IN_MLookDown (void) { 
	mlooking = true; 
}

void IN_MLookUp (void) { 
	mlooking = false;
	if (!freelook->value && lookspring->value)
	IN_CenterView ();
}

void Force_CenterView_f (void)
{
	cl.viewangles[PITCH] = 0;
}

void IN_Init (void)
{
	in_mouse = Cvar_Get ("in_mouse", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("in_mouse", "Enables mouse input.");
	in_joystick = Cvar_Get ("in_joystick", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("in_joystick", "Enables joystick input.");
	m_filter = Cvar_Get ("m_filter", "0", 0);
	Cvar_SetDescription ("m_filter", "Enables mouse input filtering.");
	in_dgamouse = Cvar_Get ("in_dgamouse", "1", CVAR_ARCHIVE);
	in_menumouse = Cvar_Get ("in_menumouse", "0", CVAR_ARCHIVE);
	// Knightmare added
	in_autosensitivity = Cvar_Get ("in_autosensitivity", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("autosensitivity", "Enables scaling of mouse and joystick sensitivty when zoomed in.");

	Cmd_AddCommand ("+mlook", IN_MLookDown);
	Cmd_AddCommand ("-mlook", IN_MLookUp);
	Cmd_AddCommand ("force_centerview", Force_CenterView_f);

	mx = my = 0;
	controller_leftx = 0.0f;
	controller_lefty = 0.0f;
	controller_rightx = 0.0f;
	controller_righty = 0.0f;

	if (in_mouse->value)
		mouse_avail = true;
	else
		mouse_avail = false;

	// Knightmare- added Psychospaz's menu mouse support
	UI_RefreshCursorMenu();
	UI_RefreshCursorLink();
}

void IN_Shutdown (void)
{
	if (!mouse_avail)
		return;

	IN_Activate(false);

	mouse_avail = false;

	Cmd_RemoveCommand ("+mlook");
	Cmd_RemoveCommand ("-mlook");
	Cmd_RemoveCommand ("force_centerview");
}

void IN_Commands (void)
{
}

void IN_Move (usercmd_t *cmd)
{
	float speed, aspeed;

	if (!mouse_avail)
		return;

	if (!in_autosensitivity)
		in_autosensitivity = Cvar_Get ("in_autosensitivity", "1", CVAR_ARCHIVE);

	if (m_filter->value)
	{
		mx = (mx + old_mouse_x) * 0.5;
		my = (my + old_mouse_y) * 0.5;
	}

	old_mouse_x = mx;
	old_mouse_y = my;

	// now to set the menu cursor
	if (cls.key_dest == key_menu)
	{
		ui_mousecursor.oldx = ui_mousecursor.x;
		ui_mousecursor.oldy = ui_mousecursor.y;

		ui_mousecursor.x += mx *  ui_sensitivity->value;
		ui_mousecursor.y += my *  ui_sensitivity->value;

		if ( (ui_mousecursor.x != ui_mousecursor.oldx) || (ui_mousecursor.y != ui_mousecursor.oldy) )
			ui_mousecursor.mouseaction = true;

		if (ui_mousecursor.x < 0) ui_mousecursor.x = 0;
		if (ui_mousecursor.x > viddef.width) ui_mousecursor.x = viddef.width;
		if (ui_mousecursor.y < 0) ui_mousecursor.y = 0;
		if (ui_mousecursor.y > viddef.height) ui_mousecursor.y = viddef.height;

		if (!cls.consoleActive)
			UI_MouseCursor_Think ();
	}

	// psychospaz - zooming in preserves sensitivity
	if (in_autosensitivity->integer && cl.base_fov < 90)
	{
		mx *= sensitivity->value * (cl.base_fov/90.0);
		my *= sensitivity->value * (cl.base_fov/90.0);
	}
	else
	{
		mx *= sensitivity->value;
		my *= sensitivity->value;
	}

	// add mouse X/Y movement to cmd
	if ((in_strafe.state & 1) || (lookstrafe->value && mlooking))
		cmd->sidemove += m_side->value * mx;
	else
		cl.viewangles[YAW] -= m_yaw->value * mx;

	if ( (mlooking || freelook->value) && !(in_strafe.state & 1))
		cl.viewangles[PITCH] += m_pitch->value * my;
	else
		cmd->forwardmove -= m_forward->value * my;

	mx = my = 0;
}

void IN_Frame (void)
{
	if (!mouse_avail)
		return;

	if ( !cl.refresh_prepped || cls.key_dest == key_console || cls.key_dest == key_menu)
		IN_Activate(false);
	else
		IN_Activate(true);

}

