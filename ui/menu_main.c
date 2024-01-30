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

// menu_main.c -- the main menu & support functions
 
#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

static int	m_main_cursor;
int			MainMenuMouseHover;

qboolean mainmouse;

// for checking if quad cursor model is available
#define QUAD_CURSOR_MODEL	"models/ui/quad_cursor.md2"
qboolean	quadModel_loaded;


/*
=======================================================================

MAIN MENU

=======================================================================
*/

#ifdef NOTTHIRTYFLIGHTS
#define	MAIN_ITEMS	5

char *main_names[] =
{
	"m_main_game",
	"m_main_multiplayer",
	"m_main_options",
	"m_main_video",
	"m_main_quit",
	0
};
#else
#define	MAIN_ITEMS	3

char *main_names[] =
{
	"m_main_game",
	"m_main_options",
	"m_main_quit",
	0
};
#endif

/*
=============
FindMenuCoords
=============
*/
void FindMenuCoords (int *xoffset, int *ystart, int *totalheight, int *widest)
{
	int w, h, i;

	*totalheight = 0;
	*widest = -1;

	for (i = 0; main_names[i] != 0; i++)
	{
		R_DrawGetPicSize (&w, &h, main_names[i]);
		if (w > *widest)
			*widest = w;
		*totalheight += (h + 12);
	}

#ifdef NOTTHIRTYFLIGHTS
	*xoffset = (SCREEN_WIDTH - *widest + 70) * 0.5;
	*ystart = SCREEN_HEIGHT*0.5 - 100;
#else
	*xoffset = SCREEN_WIDTH * 0.62;
	*ystart = SCREEN_HEIGHT * 0.6;
#endif
}


/*
=============
UI_DrawMainCursor

Draws an animating cursor with the point at
x,y.  The pic will extend to the left of x,
and both above and below y.
=============
*/
void UI_DrawMainCursor (int x, int y, int f)
{
	char	cursorname[80];
	static	qboolean cached;
	int		w,h;

	if (!cached)
	{
		int i;

		for (i = 0; i < NUM_MAINMENU_CURSOR_FRAMES; i++) {
			Com_sprintf (cursorname, sizeof(cursorname), "m_cursor%d", i);
			R_DrawFindPic (cursorname);
		}
		cached = true;
	}

	Com_sprintf (cursorname, sizeof(cursorname), "m_cursor%d", f);
	R_DrawGetPicSize (&w, &h, cursorname);
	UI_DrawPic (x, y, w, h, ALIGN_CENTER, false, cursorname, 1.0);
}


/*
=============
UI_DrawMainCursor3D

Draws a rotating quad damage model.
=============
*/
void UI_DrawMainCursor3D (int x, int y)
{
	refdef_t	refdef;
	entity_t	quadEnt, *ent;
	float		rx, ry, rw, rh;
	int			yaw;

	yaw = anglemod(cl.time/10);

	memset(&refdef, 0, sizeof(refdef));
	memset (&quadEnt, 0, sizeof(quadEnt));

	// size 24x34
	rx = x;				ry = y;
	rw = 24;			rh = 34;
	SCR_ScaleCoords (&rx, &ry, &rw, &rh, ALIGN_CENTER);
	refdef.x = rx;		refdef.y = ry;
	refdef.width = rw;	refdef.height = rh;
	refdef.fov_x = 40;
	refdef.fov_y = CalcFov (refdef.fov_x, refdef.width, refdef.height);
	refdef.time = cls.realtime*0.001;
	refdef.areabits = 0;
	refdef.lightstyles = 0;
	refdef.rdflags = RDF_NOWORLDMODEL;
	refdef.num_entities = 0;
	refdef.entities = &quadEnt;

	ent = &quadEnt;
	ent->model = R_RegisterModel (QUAD_CURSOR_MODEL);
	ent->flags = RF_FULLBRIGHT|RF_NOSHADOW|RF_DEPTHHACK;
	VectorSet (ent->origin, 40, 0, -18);
	VectorCopy( ent->origin, ent->oldorigin );
	ent->frame = 0;
	ent->oldframe = 0;
	ent->backlerp = 0.0;
	ent->angles[1] = yaw;
	refdef.num_entities++;

	R_RenderFrame( &refdef );
}


/*
=============
UI_CheckQuadModel

Checks for quad damage model.
=============
*/
void UI_CheckQuadModel (void)
{
	struct model_s *quadModel;

	quadModel = R_RegisterModel (QUAD_CURSOR_MODEL);
	
	quadModel_loaded = (quadModel != NULL);
}


/*
=============
Menu_Main_Draw
=============
*/
void Menu_Main_Draw (void)
{
	int i;
	int w, h, last_h;
	int ystart;
	int	xoffset;
	int widest = -1;
	int totalheight = 0;
	char litname[80];
#ifndef NOTTHIRTYFLIGHTS
	int selnum;
	drawStruct_t ds;
#endif

	FindMenuCoords (&xoffset, &ystart, &totalheight, &widest);

#ifndef NOTTHIRTYFLIGHTS
	if (Com_ServerState() != 2)//bc
	{
		int bgWidth = (viddef.height * 4) / 3.3f;
		int margin = (viddef.width - bgWidth) / 2;

		SCR_DrawFill (0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ALIGN_STRETCH, true, 52,47,0, 255);

		for (i = 0; i < SCREEN_HEIGHT; i += 48)
		{
			SCR_DrawFill (0, i, SCREEN_WIDTH, 32, ALIGN_STRETCH, true, 0,0,0, 64);
		}

		//BC MAIN MENU FULLSCREEN

		memset(&ds, 0, sizeof(drawStruct_t));
		ds.pic = "m_main_bg";
		ds.x = margin;
		ds.y = 0;
		ds.w = bgWidth;
		ds.h = viddef.height;
		Vector2Copy(vec2_origin, ds.offset);
		Vector4Copy(vec4_identity, ds.color);
		R_DrawPic(ds);
		/*R_DrawStretchPic (
			margin,
			0,
			bgWidth,
			viddef.height,
			"m_main_bg", 1.0);*/

		SCR_DrawPic (0, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT,
			ALIGN_STRETCH, false,
			"/pics/vignette.tga", 1.0);
	}
	else
	{
		int posy = viddef.height * 0.54;
		int adjustedWidth = (viddef.height * 4) / 3.3f;		

		int boxsize = viddef.height * 0.35;

		memset(&ds, 0, sizeof(drawStruct_t));
		ds.pic = "m_main_minibox";
		ds.x = (viddef.width /2) + (adjustedWidth * 0.1);
		ds.y = posy;
		ds.w = boxsize;
		ds.h = boxsize;
		Vector2Copy(vec2_origin, ds.offset);
		Vector4Copy(vec4_identity, ds.color);
		R_DrawPic(ds);
		/*R_DrawStretchPic (
			//adjustedWidth * 0.65,
			(viddef.width /2) + (adjustedWidth * 0.1),
			posy,
			boxsize,
			boxsize,
			"m_main_minibox", 1.0);*/
	}
#endif

	for (i = 0; main_names[i] != 0; i++)
		if (i != m_main_cursor) {
			R_DrawGetPicSize (&w, &h, main_names[i]);
			UI_DrawPic (xoffset, (ystart + i*40+3),
#ifdef NOTTHIRTYFLIGHTS
			            w, h,
#else
			            w*0.5, h*0.5,
#endif
			            ALIGN_CENTER, false, main_names[i],
#ifdef NOTTHIRTYFLIGHTS
			            1.0);
#else
			            0.5);
#endif
		}
#ifndef NOTTHIRTYFLIGHTS
		else selnum = i;
#endif

//	strncpy (litname, main_names[m_main_cursor]);
//	strncat (litname, "_sel");
	Q_strncpyz (litname, sizeof(litname), main_names[m_main_cursor]);
#ifdef NOTTHIRTYFLIGHTS
	Q_strncatz (litname, sizeof(litname), "_sel");
#endif
	R_DrawGetPicSize (&w, &h, litname);
#ifdef NOTTHIRTYFLIGHTS
	UI_DrawPic (xoffset-1, (ystart + m_main_cursor*40+2), w+2, h+2, ALIGN_CENTER, false, litname, 1.0);

	// Draw our nifty quad damage model as a cursor if it's loaded.
	if (quadModel_loaded)
		UI_DrawMainCursor3D (xoffset-27, ystart+(m_main_cursor*40+1));
	else
		UI_DrawMainCursor (xoffset-25, ystart+(m_main_cursor*40+1), (int)(cls.realtime/100)%NUM_MAINMENU_CURSOR_FRAMES);

	R_DrawGetPicSize (&w, &h, "m_main_plaque");
	UI_DrawPic (xoffset-(w/2+50), ystart, w, h, ALIGN_CENTER, false, "m_main_plaque", 1.0);
	last_h = h;

	R_DrawGetPicSize (&w, &h, "m_main_logo");
	UI_DrawPic (xoffset-(w/2+50), ystart+last_h+20, w, h, ALIGN_CENTER, false, "m_main_logo", 1.0);
#else
	UI_DrawPic (xoffset+5*sin(anglemod(cl.time*0.005)) ,
		(ystart + m_main_cursor*40+3),
		w*0.5, h*0.5,
		ALIGN_CENTER, false, litname, 1.0);
#endif
}


/*
=============
OpenMenuFromMain
=============
*/
void OpenMenuFromMain (void)
{
	switch (m_main_cursor)
	{
#ifdef NOTTHIRTYFLIGHTS
		case 0:
			Menu_Game_f ();
			break;

		case 1:
			Menu_Multiplayer_f();
			break;

		case 2:
			Menu_Options_f ();
			break;

		case 3:
			Menu_Video_f ();
			break;

		case 4:
			Menu_Quit_f ();
			break;
#else
		case 0:
			Menu_Game_f ();
			break;

		case 1:
			Menu_Options_f ();
			break;

		case 2:
			Menu_Quit_f ();
			break;
#endif
	}
}


/*
=============
UI_CheckMainMenuMouse
=============
*/
void UI_CheckMainMenuMouse (void)
{
	int ystart;
	int	xoffset;
	int widest;
	int totalheight;
	int i, oldhover;
	char *sound = NULL;
	mainmenuobject_t buttons[MAIN_ITEMS];

	oldhover = MainMenuMouseHover;
	MainMenuMouseHover = 0;

	FindMenuCoords(&xoffset, &ystart, &totalheight, &widest);
	for (i = 0; main_names[i] != 0; i++)
		UI_AddMainButton (&buttons[i], i, xoffset, ystart+(i*40+3), main_names[i]);

	// Exit with double click 2nd mouse button
	if (!ui_mousecursor.buttonused[MOUSEBUTTON2] && ui_mousecursor.buttonclicks[MOUSEBUTTON2]==2)
	{
		UI_PopMenu();
		sound = ui_menu_out_sound;
		ui_mousecursor.buttonused[MOUSEBUTTON2] = true;
		ui_mousecursor.buttonclicks[MOUSEBUTTON2] = 0;
	}

	for (i=MAIN_ITEMS-1; i>=0; i--)
	{
		if ( (ui_mousecursor.x >= buttons[i].min[0]) && (ui_mousecursor.x <= buttons[i].max[0]) &&
			(ui_mousecursor.y >= buttons[i].min[1]) && (ui_mousecursor.y <= buttons[i].max[1]) )
		{
#ifndef NOTTHIRTYFLIGHTS
			if (!mainmouse)
			{
				sound = ui_menu_move_sound;
				mainmouse=true;
			}
#endif

			if (ui_mousecursor.mouseaction)
				m_main_cursor = i;

			MainMenuMouseHover = 1 + i;

			if (oldhover == MainMenuMouseHover && MainMenuMouseHover-1 == m_main_cursor &&
				!ui_mousecursor.buttonused[MOUSEBUTTON1] && ui_mousecursor.buttonclicks[MOUSEBUTTON1]==1)
			{
				OpenMenuFromMain();
				sound = ui_menu_move_sound;
				ui_mousecursor.buttonused[MOUSEBUTTON1] = true;
				ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 0;
			}
			break;
		}
	}

	if (!MainMenuMouseHover)
	{
		ui_mousecursor.buttonused[MOUSEBUTTON1] = false;
		ui_mousecursor.buttonclicks[MOUSEBUTTON1] = 0;
		ui_mousecursor.buttontime[MOUSEBUTTON1] = 0;
#ifndef NOTTHIRTYFLIGHTS
		if (mainmouse)
			mainmouse=false;
#endif
	}

	ui_mousecursor.mouseaction = false;

	if (sound)
		S_StartLocalSound(sound);
}


/*
=============
Menu_Main_Key
=============
*/
const char *Menu_Main_Key (int key)
{
	const char *sound = ui_menu_move_sound;

	switch (key)
	{
	case K_JOY2: //BC 1/30/2024 gamepad B button
	case K_ESCAPE:
#ifdef ERASER_COMPAT_BUILD // special hack for Eraser build
		if (cls.state == ca_disconnected)
			Menu_Quit_f ();
		else
			UI_PopMenu ();
#else	// can't exit menu if disconnected,
#ifdef NOTTHIRTYFLIGHTS
		// so restart demo loop
		if (cls.state == ca_disconnected)
			Cbuf_AddText ("d1\n");
#else
		if (Com_ServerState() == 2)//bc if we're ingame.
#endif
		UI_PopMenu ();
#endif
		break;

	case K_AUX31: //BC 1/30/2024 dpad down.
	case K_KP_DOWNARROW:
	case K_DOWNARROW:
		if (++m_main_cursor >= MAIN_ITEMS)
			m_main_cursor = 0;
		return sound;

	case K_AUX29: //BC 1/30/2024 dpad up.
	case K_KP_UPARROW:
	case K_UPARROW:
		if (--m_main_cursor < 0)
			m_main_cursor = MAIN_ITEMS - 1;
		return sound;

	case K_JOY1: //BC 1/30/2024 gamepad A button
	case K_KP_ENTER:
	case K_ENTER:
		ui_entersound = true;

		switch (m_main_cursor)
		{
#ifdef NOTTHIRTYFLIGHTS
		case 0:
			Menu_Game_f ();
			break;

		case 1:
			Menu_Multiplayer_f();
			break;

		case 2:
			Menu_Options_f ();
			break;

		case 3:
			Menu_Video_f ();
			break;

		case 4:
			Menu_Quit_f ();
			break;
#else
		case 0:
			Menu_Game_f ();
			break;

		case 1:
			Menu_Options_f ();
			break;

		case 2:
			Menu_Quit_f ();
			break;
#endif
		}
	}
	return NULL;
}


/*
=============
Menu_Main_f
=============
*/
void Menu_Main_f (void)
{
#ifndef NOTTHIRTYFLIGHTS
	char fn[8];
#endif

	UI_CheckQuadModel ();
	UI_PushMenu (Menu_Main_Draw, Menu_Main_Key);

#ifndef NOTTHIRTYFLIGHTS
	if (cls.state == ca_disconnected)
	{
		Com_sprintf(cl.configstrings[CS_CDTRACK], sizeof(cl.configstrings[CS_CDTRACK]), "barsong");
		CL_PlayBackgroundTrack ();
	}
#endif
}
