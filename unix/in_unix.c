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

cvar_t	*in_mouse;
cvar_t	*in_joystick;
extern int mx, my;
static qboolean	mouse_avail;
static int old_mouse_x, old_mouse_y;

static qboolean	mlooking;
cvar_t	*m_filter;
cvar_t	*in_dgamouse;
cvar_t	*autosensitivity;
cvar_t	*in_menumouse; /// FIXME Menu Mouse on windowed mode 

extern cursor_t cursor;

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
    in_joystick = Cvar_Get ("in_joystick", "0", CVAR_ARCHIVE);
    m_filter = Cvar_Get ("m_filter", "0", 0);
    in_dgamouse = Cvar_Get ("in_dgamouse", "1", CVAR_ARCHIVE);
    in_menumouse = Cvar_Get ("in_menumouse", "0", CVAR_ARCHIVE);
    
    Cmd_AddCommand ("+mlook", IN_MLookDown);
    Cmd_AddCommand ("-mlook", IN_MLookUp);
    Cmd_AddCommand ("force_centerview", Force_CenterView_f);

    mx = my = 0.0;  
    
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

void M_Think_MouseCursor (void);
void IN_Move (usercmd_t *cmd)
{
	if (!mouse_avail)
		return;

	if (!autosensitivity)
		autosensitivity = Cvar_Get ("autosensitivity", "1", CVAR_ARCHIVE);

	if (m_filter->value)
	{
		mx = (mx + old_mouse_x) * 0.5;
		my = (my + old_mouse_y) * 0.5;
	}

	old_mouse_x = mx;
	old_mouse_y = my;

	//now to set the menu cursor
	if (cls.key_dest == key_menu)
	{
		cursor.oldx = cursor.x;
		cursor.oldy = cursor.y;

		cursor.x += mx *  menu_sensitivity->value;
		cursor.y += my *  menu_sensitivity->value;

		if (cursor.x!=cursor.oldx || cursor.y!=cursor.oldy)
			cursor.mouseaction = true;

		if (cursor.x < 0) cursor.x = 0;
		if (cursor.x > viddef.width) cursor.x = viddef.width;
		if (cursor.y < 0) cursor.y = 0;
		if (cursor.y > viddef.height) cursor.y = viddef.height;
		M_Think_MouseCursor();
	}
	
	mx *= sensitivity->value;
	my *= sensitivity->value;

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

