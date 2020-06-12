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

// ui_video.c -- the video options menu

#include "../client/client.h"
#include "ui_local.h"

extern cvar_t *vid_ref;

void Menu_Video_Init (void);

/*
=======================================================================

VIDEO MENU

=======================================================================
*/

static menuframework_s	s_video_menu;
static menulist_s		s_mode_list;
static menulist_s  		s_fs_box;
static menuslider_s		s_brightness_slider;
static menulist_s		s_texqual_box;
static menulist_s		s_texfilter_box;
static menulist_s		s_aniso_box;
static menulist_s  		s_texcompress_box;
static menulist_s  		s_vsync_box;
static menuaction_s		s_advanced_action;
static menuaction_s		s_defaults_action;
static menuaction_s		s_apply_action;
static menuaction_s		s_backmain_action;

static void BrightnessCallback( void *s )
{
	// invert sense so greater = brighter, and scale to a range of 0.3 to 1.3
	Cvar_SetValue( "vid_gamma", (1.3 - (s_brightness_slider.curvalue/20.0)) );
}

static void VsyncCallback ( void *unused )
{
	Cvar_SetValue( "r_swapinterval", s_vsync_box.curvalue);
}

static void AdvancedOptions( void *s )
{
	M_Menu_Video_Advanced_f ();
}

static void ResetVideoDefaults ( void *unused )
{
	Cvar_SetToDefault  ("vid_fullscreen");
	Cvar_SetToDefault  ("vid_gamma");
	Cvar_SetToDefault  ("r_mode");
	Cvar_SetToDefault  ("r_texturemode");
	Cvar_SetToDefault  ("r_anisotropic");
	Cvar_SetToDefault  ("r_picmip");
	Cvar_SetToDefault  ("r_ext_texture_compression");
	Cvar_SetToDefault  ("r_swapinterval");

	Cvar_SetToDefault  ("r_modulate");
	Cvar_SetToDefault  ("r_intensity");
	Cvar_SetToDefault  ("r_overbrightbits");
	Cvar_SetToDefault  ("r_trans_lightmaps");
	Cvar_SetToDefault  ("r_solidalpha");
	Cvar_SetToDefault  ("r_pixel_shader_warp");
	Cvar_SetToDefault  ("r_waterwave");
	Cvar_SetToDefault  ("r_caustics");
	Cvar_SetToDefault  ("cl_particle_scale");
	Cvar_SetToDefault  ("r_particle_overdraw");
	Cvar_SetToDefault  ("r_bloom");
	Cvar_SetToDefault  ("r_decals");
	Cvar_SetToDefault  ("r_model_shading");
	Cvar_SetToDefault  ("r_shadows");
	Cvar_SetToDefault  ("r_stencilTwoSide");
	Cvar_SetToDefault  ("r_shelltype");
	Cvar_SetToDefault  ("r_glass_envmaps");
	Cvar_SetToDefault  ("r_screenshot_jpeg");
	Cvar_SetToDefault  ("r_screenshot_jpeg_quality");
	Cvar_SetToDefault  ("r_saveshotsize");

	Menu_Video_Init();
}

static void prepareVideoRefresh( void )
{
	//set the right mode for refresh
	Cvar_Set( "vid_ref", "gl" );
	Cvar_Set( "gl_driver", "opengl32" );

	//tell them they're modified so they refresh
	vid_ref->modified = true;
}


static void ApplyChanges( void *unused )
{
	if (s_texfilter_box.curvalue == 0)
		Cvar_Set("r_texturemode", "GL_LINEAR_MIPMAP_NEAREST");
	else if (s_texfilter_box.curvalue == 1)
		Cvar_Set("r_texturemode", "GL_LINEAR_MIPMAP_LINEAR");

	Cvar_SetValue( "r_mode", s_mode_list.curvalue  ); // offset for eliminating < 640x480 modes
	Cvar_SetValue( "vid_fullscreen", s_fs_box.curvalue );
	// invert sense so greater = brighter, and scale to a range of 0.3 to 1.3
	Cvar_SetValue( "vid_gamma", (1.3 - (s_brightness_slider.curvalue/20.0)) );
	Cvar_SetValue( "r_picmip", 3-s_texqual_box.curvalue );

	switch ((int)s_aniso_box.curvalue)
	{
		case 1: Cvar_SetValue( "r_anisotropic", 2.0 ); break;
		case 2: Cvar_SetValue( "r_anisotropic", 4.0 ); break;
		case 3: Cvar_SetValue( "r_anisotropic", 8.0 ); break;
		case 4: Cvar_SetValue( "r_anisotropic", 16.0 ); break;
		default:
		case 0: Cvar_SetValue( "r_anisotropic", 0.0 ); break;
	}

	Cvar_SetValue( "r_ext_texture_compression", s_texcompress_box.curvalue );
	Cvar_SetValue( "r_swapinterval", s_vsync_box.curvalue );

	prepareVideoRefresh ();

	if (!cls.consoleActive && Cvar_VariableValue ("maxclients") <= 1 && !Com_ServerState())
	{
		//if NOT in-game.
		UI_ForceMenuOff();

		//kick player out of limbo state.
		Con_ToggleConsole_f();
	}
}


// Knightmare added
int texfilter_box_setval (void)
{
	char *texmode = Cvar_VariableString("r_texturemode");
	if (!Q_strcasecmp(texmode, "GL_LINEAR_MIPMAP_NEAREST"))
		return 0;
	else
		return 1;
}


static const char *aniso0_names[] =
{
	"not supported",
	0
};

static const char *aniso2_names[] =
{
	"off",
	"2x",
	0
};

static const char *aniso4_names[] =
{
	"off",
	"2x",
	"4x",
	0
};

static const char *aniso8_names[] =
{
	"off",
	"2x",
	"4x",
	"8x",
	0
};

static const char *aniso16_names[] =
{
	"off",
	"2x",
	"4x",
	"8x",
	"16x",
	0
};

static const char **GetAnisoNames ()
{
	float aniso_avail = Cvar_VariableValue("r_anisotropic_avail");
	if (aniso_avail < 2.0)
		return aniso0_names;
	else if (aniso_avail < 4.0)
		return aniso2_names;
	else if (aniso_avail < 8.0)
		return aniso4_names;
	else if (aniso_avail < 16.0)
		return aniso8_names;
	else // >= 16.0
		return aniso16_names;
}


float GetAnisoCurValue ()
{
	float aniso_avail = Cvar_VariableValue("r_anisotropic_avail");
	float anisoValue = ClampCvar (0, aniso_avail, Cvar_VariableValue("r_anisotropic"));
	if (aniso_avail == 0) // not available
		return 0;
	if (anisoValue < 2.0)
		return 0;
	else if (anisoValue < 4.0)
		return 1;
	else if (anisoValue < 8.0)
		return 2;
	else if (anisoValue < 16.0)
		return 3;
	else // >= 16.0
		return 4;
}


/*
================
Menu_Video_Init
================
*/
void Menu_Video_Init (void)
{
	// Knightmare- added 1280x1024, 1400x1050, 856x480, 1024x480 modes, removed 320x240, 400x300, 512x384 modes
	static const char *resolutions[] = 
	{
		"640x480",	//0
		"800x600",  //1
		"856x480", //2
		"960x720",  //3
		"1024x480", //4
		"1024x768",  //5
		"1152x864",  //6
		"1280x720", //7
		"1280x768", //8
		"1280x800", //9
		"1280x960",  //10
		"1280x1024", //11
		"1360x768", //12
		"1366x768", //13		
		"1400x1050", //14
		"1440x900", //15
		"1600x900", //16
		"1600x1200", //17
		"1680x1050", //18
		"1920x1080", //19
		"1920x1200", //20
		"1920x1440", //21		
		"2048x1536", //22
		"2560x1440", //23
		"3200x1080", //24
		"3840x1080", //25		 
		
		0
	};
	static const char *yesno_names[] =
	{
		"no",
		"yes",
		0
	};
	static const char *mip_names[] =
	{
		"Bilinear",
		"Trilinear",
		0
	};
	static const char *lmh_names[] =
	{
		"Low",
		"Medium",
		"High",
		"Highest",
		0
	};

	int  y = 0;

	if ( !con_font_size )
		con_font_size = Cvar_Get ("con_font_size", "8", CVAR_ARCHIVE);

	s_video_menu.x = SCREEN_WIDTH*0.5;
//	s_video_menu.x = viddef.width * 0.50;
	s_video_menu.y = SCREEN_HEIGHT*0.5 - 80;
	s_video_menu.nitems = 0;

	s_mode_list.generic.type = MTYPE_SPINCONTROL;
	s_mode_list.generic.name = "Screen Resolution";
	s_mode_list.generic.x = 0;
	s_mode_list.generic.y = y;
	s_mode_list.itemnames = resolutions;
	s_mode_list.curvalue = max((Cvar_VariableValue("r_mode") ), 0); // offset for getting rid of < 640x480 resolutions

	s_fs_box.generic.type = MTYPE_CHECKBOX;
	s_fs_box.generic.x	= 0;
	s_fs_box.generic.y	= y += MENU_LINE_SIZE;
	s_fs_box.generic.name	= "Fullscreen";
	s_fs_box.itemnames = yesno_names;
	s_fs_box.curvalue = Cvar_VariableValue("vid_fullscreen");

	s_brightness_slider.generic.type	= MTYPE_SLIDER;
	s_brightness_slider.generic.x	= 0;
	s_brightness_slider.generic.y	= y += MENU_LINE_SIZE;
	s_brightness_slider.generic.name	= "Brightness";
	s_brightness_slider.generic.callback = BrightnessCallback;
	s_brightness_slider.minvalue = 0;
	s_brightness_slider.maxvalue = 20;
	s_brightness_slider.curvalue = (1.3 - Cvar_VariableValue("vid_gamma")) * 20;

	s_texfilter_box.generic.type	= MTYPE_SPINCONTROL;
	s_texfilter_box.generic.x		= 0;
	s_texfilter_box.generic.y		= y += 2*MENU_LINE_SIZE;
	s_texfilter_box.generic.name	= "Texture Filter";
	s_texfilter_box.curvalue		= texfilter_box_setval();
	s_texfilter_box.itemnames		= mip_names;

	s_aniso_box.generic.type	= MTYPE_SPINCONTROL;
	s_aniso_box.generic.x		= 0;
	s_aniso_box.generic.y		= y += MENU_LINE_SIZE;
	s_aniso_box.generic.name	= "Anisotropic Filter";
	s_aniso_box.curvalue		= GetAnisoCurValue();
	s_aniso_box.itemnames		= GetAnisoNames();

	s_texqual_box.generic.type	= MTYPE_SPINCONTROL;
	s_texqual_box.generic.x		= 0;
	s_texqual_box.generic.y		= y += MENU_LINE_SIZE;
	s_texqual_box.generic.name	= "Texture Quality";
	s_texqual_box.curvalue		= ClampCvar (0, 3, 3-Cvar_VariableValue("r_picmip"));
	s_texqual_box.itemnames		= lmh_names;

	s_texcompress_box.generic.type		= MTYPE_CHECKBOX;
	s_texcompress_box.generic.x			= 0;
	s_texcompress_box.generic.y			= y += MENU_LINE_SIZE;
	s_texcompress_box.generic.name		= "Texture Compression";
	s_texcompress_box.itemnames			= yesno_names;
	s_texcompress_box.curvalue			= Cvar_VariableValue("r_ext_texture_compression");

	s_vsync_box.generic.type	= MTYPE_CHECKBOX;
	s_vsync_box.generic.x		= 0;
	s_vsync_box.generic.y		= y += 2*MENU_LINE_SIZE;
	s_vsync_box.generic.name	= "Video Sync";
	s_vsync_box.generic.callback = VsyncCallback;
	s_vsync_box.curvalue		= Cvar_VariableValue("r_swapinterval");
	s_vsync_box.itemnames		= yesno_names;
	s_vsync_box.generic.statusbar	= "Sync framerate with monitor refresh rate.";

	// Knightmare added
	s_advanced_action.generic.type = MTYPE_ACTION;
	s_advanced_action.generic.name = "Advanced Options";
	s_advanced_action.generic.x    = 0;
	s_advanced_action.generic.y    = y += 3*MENU_LINE_SIZE;
	s_advanced_action.generic.callback = AdvancedOptions;

	s_defaults_action.generic.type = MTYPE_ACTION;
	s_defaults_action.generic.name = "Reset Defaults";
	s_defaults_action.generic.x    = 0;
	s_defaults_action.generic.y    = y += 3*MENU_LINE_SIZE;
	s_defaults_action.generic.callback = ResetVideoDefaults;
	s_defaults_action.generic.statusbar	= "Reset all video settings to default values.";

	//Knightmare- changed cancel to apply changes, thanx to MrG
	s_apply_action.generic.type = MTYPE_ACTION;
	s_apply_action.generic.name = "APPLY CHANGES";
	s_apply_action.generic.x    = 0;
	s_apply_action.generic.y    = y += 2*MENU_LINE_SIZE;
	s_apply_action.generic.callback = ApplyChanges;
	//end Knightmare

	s_backmain_action.generic.type = MTYPE_ACTION;
	s_backmain_action.generic.name = "Done";
	s_backmain_action.generic.x    = 0;
	s_backmain_action.generic.y    = y += 2*MENU_LINE_SIZE;
	s_backmain_action.generic.callback = UI_BackMenu;

	Menu_AddItem( &s_video_menu, ( void * ) &s_mode_list );
	Menu_AddItem( &s_video_menu, ( void * ) &s_fs_box );
	Menu_AddItem( &s_video_menu, ( void * ) &s_brightness_slider );
	Menu_AddItem( &s_video_menu, ( void * ) &s_texfilter_box );
	Menu_AddItem( &s_video_menu, ( void * ) &s_aniso_box );
	Menu_AddItem( &s_video_menu, ( void * ) &s_texqual_box );
	Menu_AddItem( &s_video_menu, ( void * ) &s_texcompress_box );
	Menu_AddItem( &s_video_menu, ( void * ) &s_vsync_box );
	Menu_AddItem( &s_video_menu, ( void * ) &s_advanced_action );

	Menu_AddItem( &s_video_menu, ( void * ) &s_defaults_action );
	Menu_AddItem( &s_video_menu, ( void * ) &s_apply_action );
	Menu_AddItem( &s_video_menu, ( void * ) &s_backmain_action );

//	Menu_Center( &s_video_menu );
//	s_video_menu.x -= MENU_FONT_SIZE;
}

/*
================
Menu_Video_Draw
================
*/
void Menu_Video_Draw (void)
{
	//int w, h;

	// draw the banner
	Menu_Banner("m_banner_video");

	// move cursor to a reasonable starting position
	Menu_AdjustCursor( &s_video_menu, 1 );

	// draw the menu
	Menu_Draw( &s_video_menu );
}

/*
================
Video_MenuKey
================
*/
const char *Video_MenuKey( int key )
{
	return Default_MenuKey( &s_video_menu, key );
}

void M_Menu_Video_f (void)
{
	Menu_Video_Init();
	UI_PushMenu( Menu_Video_Draw, Video_MenuKey );
}
