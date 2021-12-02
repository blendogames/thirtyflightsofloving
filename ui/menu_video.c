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

// menu_video.c -- the video options menu

#include "../client/client.h"
#include "ui_local.h"

void Menu_Video_Init (void);

/*
=======================================================================

VIDEO MENU

=======================================================================
*/

static menuframework_s	s_video_menu;
static menulist_s		s_mode_list;
static menuseparator_s	s_customwidth_title;
static menuseparator_s	s_customheight_title;
static menufield_s		s_customwidth_field;
static menufield_s		s_customheight_field;
static menulist_s  		s_fs_box;
static menuslider_s		s_brightness_slider;
static menulist_s		s_texqual_box;
static menulist_s		s_texfilter_box;
static menulist_s		s_aniso_box;
static menulist_s		s_npot_mipmap_box;
static menulist_s		s_sgis_mipmap_box;
//static menulist_s  		s_texcompress_box;
static menulist_s  		s_vsync_box;
static menulist_s		s_refresh_box;	// Knightmare- refresh rate option
static menulist_s  		s_adjust_fov_box;
static menulist_s  		s_async_box;

static menuaction_s		s_advanced_action;
static menuaction_s		s_defaults_action;
static menuaction_s		s_apply_action;
static menuaction_s		s_backmain_action;

//=======================================================================

static void M_ShowCustomFields (void)
{
	qboolean	customHidden = (strcmp(ui_video_modes[s_mode_list.curValue], "-1") != 0);

	s_customwidth_title.generic.flags	= customHidden ? QMF_HIDDEN : 0;
	s_customwidth_field.generic.flags	= customHidden ? (QMF_NUMBERSONLY|QMF_HIDDEN) : QMF_NUMBERSONLY;
	s_customheight_title.generic.flags	= customHidden ? QMF_HIDDEN : 0;
	s_customheight_field.generic.flags	= customHidden ? (QMF_NUMBERSONLY|QMF_HIDDEN) : QMF_NUMBERSONLY;
}

static void VidModeCallback (void *unused)
{
	M_ShowCustomFields ();
}

static void BrightnessCallback (void *s)
{
	UI_MenuSlider_SaveValue (&s_brightness_slider, "vid_gamma");
}

static void AnisoCallback (void *s)
{
	UI_MenuSpinControl_SaveValue (&s_aniso_box, "r_anisotropic");
}

static void VsyncCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_vsync_box, "r_swapinterval");
}

static void AdjustFOVCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_adjust_fov_box, "cl_widescreen_fov");
}

static void AsyncCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_async_box, "cl_async");
}

static void M_AdvancedOptions (void *s)
{
	Menu_Video_Advanced_f ();
}

//=======================================================================

static void M_PrepareVideoRefresh (void)
{
	// set the right mode for refresh
	Cvar_Set( "vid_ref", "gl" );
	Cvar_Set( "gl_driver", "opengl32" );

	// tell them they're modified so they refresh
	Cvar_SetModified ("vid_ref", true);
}

static void M_ResetVideoDefaults (void *unused)
{
	Cvar_SetToDefault ("r_mode");
	Cvar_SetToDefault ("vid_fullscreen");
	Cvar_SetToDefault ("vid_gamma");
	Cvar_SetToDefault ("r_texturemode");
	Cvar_SetToDefault ("r_anisotropic");
	Cvar_SetToDefault ("r_picmip");
	Cvar_SetToDefault ("r_nonpoweroftwo_mipmaps");
	Cvar_SetToDefault ("r_sgis_generatemipmap");
	Cvar_SetToDefault ("r_ext_texture_compression");
	Cvar_SetToDefault ("r_swapinterval");
	Cvar_SetToDefault ("r_displayrefresh");
	Cvar_SetToDefault ("cl_widescreen_fov");
	Cvar_SetToDefault ("cl_async");

	Cvar_SetToDefault ("r_modulate");
	Cvar_SetToDefault ("r_intensity");
	Cvar_SetToDefault ("r_rgbscale");
	Cvar_SetToDefault ("r_trans_lighting");
	Cvar_SetToDefault ("r_warp_lighting");
	Cvar_SetToDefault ("r_lightcutoff");
	Cvar_SetToDefault ("r_glass_envmaps");
	Cvar_SetToDefault ("r_solidalpha");
	Cvar_SetToDefault ("r_pixel_shader_warp");
	Cvar_SetToDefault ("r_waterwave");
	Cvar_SetToDefault ("r_caustics");
	Cvar_SetToDefault ("r_particle_overdraw");
	Cvar_SetToDefault ("r_bloom");
	Cvar_SetToDefault ("r_model_shading");
	Cvar_SetToDefault ("r_shadows");
	Cvar_SetToDefault ("r_stencilTwoSide");
	Cvar_SetToDefault ("r_shelltype");
	Cvar_SetToDefault ("r_celshading");
	Cvar_SetToDefault ("r_celshading_width");
	Cvar_SetToDefault ("r_screenshot_format");
	Cvar_SetToDefault ("r_screenshot_jpeg_quality");
	Cvar_SetToDefault ("r_saveshotsize");
	Cvar_SetToDefault ("r_font_upscale");

	Menu_Video_Init();
}

static void M_ApplyVideoChanges (void *unused)
{
	int		customW, customH;
	char	*customStr;

	UI_MenuSpinControl_SaveValue (&s_mode_list, "r_mode");
	if (strcmp(ui_video_modes[s_mode_list.curValue], "-1") == 0)	// use custom mode fields
	{
		customW = atoi(s_customwidth_field.buffer);
		customH	= atoi(s_customheight_field.buffer);
		Cvar_SetInteger ("r_customwidth", ClampCvar( 640, 99999, customW ));
		Cvar_SetInteger ("r_customheight", ClampCvar( 480, 99999, customH ));

		// update fields in case values were clamped
		customStr = Cvar_VariableString("r_customwidth");
		Q_strncpyz (s_customwidth_field.buffer, sizeof(s_customwidth_field.buffer), customStr);
		s_customwidth_field.cursor = (int)strlen(customStr);
		
		customStr = Cvar_VariableString("r_customheight");
		Q_strncpyz (s_customheight_field.buffer, sizeof(s_customwidth_field.buffer), customStr);
		s_customheight_field.cursor = (int)strlen(customStr);
	}

	UI_MenuSpinControl_SaveValue (&s_fs_box, "vid_fullscreen");
	UI_MenuSlider_SaveValue (&s_brightness_slider, "vid_gamma");
	UI_MenuSpinControl_SaveValue (&s_texfilter_box, "r_texturemode");
	UI_MenuSpinControl_SaveValue (&s_aniso_box, "r_anisotropic");
	UI_MenuSpinControl_SaveValue (&s_texqual_box, "r_picmip");
	UI_MenuSpinControl_SaveValue (&s_npot_mipmap_box, "r_nonpoweroftwo_mipmaps");
	UI_MenuSpinControl_SaveValue (&s_sgis_mipmap_box, "r_sgis_generatemipmap");
//	UI_MenuSpinControl_SaveValue (&s_texcompress_box, "r_ext_texture_compression");
	UI_MenuSpinControl_SaveValue (&s_vsync_box, "r_swapinterval");
	UI_MenuSpinControl_SaveValue (&s_refresh_box, "r_displayrefresh");
	UI_MenuSpinControl_SaveValue (&s_adjust_fov_box, "cl_widescreen_fov");
	UI_MenuSpinControl_SaveValue (&s_async_box, "cl_async");

	M_PrepareVideoRefresh ();

//	UI_ForceMenuOff();
#ifndef NOTTHIRTYFLIGHTS
	if (!cls.consoleActive && Cvar_VariableValue ("maxclients") <= 1 && !Com_ServerState())
	{
		//if NOT in-game.
		UI_ForceMenuOff();

		//kick player out of limbo state.
		Con_ToggleConsole_f();
	}
#endif
}

//=======================================================================

void Menu_Video_Init (void)
{
	static const char *yesno_names[] =
	{
		"no",
		"yes",
		0
	};
	static const char *fullscreen_names[] =
	{
#ifdef NOTTHIRTYFLIGHTS
		"windowed",
		"fullscreen",
		"borderless",
#else
		"Windowed",
		"Fullscreen",
		"Borderless",
#endif
		0
	};
	static const char *refreshrate_names[] = 
	{
		"[default]",
		"[60Hz   ]",
		"[70Hz   ]",
		"[72Hz   ]",
		"[75Hz   ]",
		"[85Hz   ]",
		"[100Hz  ]",
		"[110Hz  ]",
		"[120Hz  ]",
		"[144Hz  ]",
		"[150Hz  ]",
		"[160Hz  ]",
		"[165Hz  ]",
		"[180Hz  ]",
		"[240Hz  ]",
		0
	};
	static const char *refreshrate_values[] = 
	{
		"0",
		"60",
		"70",
		"72",
		"75",
		"85",
		"100",
		"110",
		"120",
		"144",
		"150",
		"160",
		"165",
		"180",
		"240",
		0
	};
	static const char *texfilter_names[] =
	{
#ifdef NOTTHIRTYFLIGHTS
		"bilinear",
		"trilinear",
#else
		"Bilinear",
		"Trilinear",
#endif
		0
	};
	static const char *texfilter_values[] =
	{
		"GL_LINEAR_MIPMAP_NEAREST",
		"GL_LINEAR_MIPMAP_LINEAR",
		0
	};
	static const char *lmh_names[] =
	{
#ifdef NOTTHIRTYFLIGHTS
		"lowest",
		"low",
		"medium",
		"high",
		"highest",
#else
		"Lowest",
		"Low",
		"Medium",
		"High",
		"Highest",
#endif
		0
	};
	static const char *lmh_values[] =
	{
		"4",
		"3",
		"2",
		"1",
		"0",
		0
	};
	int			y = 0;
	char		*customStr;

	if ( !con_font_size )
		con_font_size = Cvar_Get ("con_font_size", "8", CVAR_ARCHIVE);

	s_video_menu.x = SCREEN_WIDTH*0.5;
//	s_video_menu.x = viddef.width * 0.50;
	s_video_menu.y = SCREEN_HEIGHT*0.5 - 80;
	s_video_menu.nitems = 0;

	s_mode_list.generic.type		= MTYPE_SPINCONTROL;
	s_mode_list.generic.textSize	= MENU_FONT_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_mode_list.generic.name		= "video mode";
#else
	s_mode_list.generic.name		= "Screen Resolution";
#endif
	s_mode_list.generic.x			= 0;
	s_mode_list.generic.y			= y;
	s_mode_list.itemNames			= ui_resolution_names;
	s_mode_list.itemValues			= ui_video_modes;
	s_mode_list.generic.callback	= VidModeCallback;
	s_mode_list.generic.statusbar	= "changes screen resolution";
	UI_MenuSpinControl_SetValue (&s_mode_list, "r_mode", 0, 0, false);

	s_customwidth_title.generic.type		= MTYPE_SEPARATOR;
	s_customwidth_title.generic.textSize	= MENU_FONT_SIZE;
	s_customwidth_title.generic.flags		= 0;
	s_customwidth_title.generic.name		= "custom width";
	s_customwidth_title.generic.x			= -2*MENU_FONT_SIZE;
	s_customwidth_title.generic.y			= y += 1.5*MENU_LINE_SIZE;

	s_customwidth_field.generic.type		= MTYPE_FIELD;
	s_customwidth_field.generic.textSize	= MENU_FONT_SIZE;
	s_customwidth_field.generic.flags		= QMF_NUMBERSONLY;
//	s_customwidth_field.generic.name		= "custom width";
	s_customwidth_field.generic.callback	= 0;
	s_customwidth_field.generic.x			= -14*MENU_FONT_SIZE;
	s_customwidth_field.generic.y			= y + 1.5*MENU_LINE_SIZE;
	s_customwidth_field.length				= 5;
	s_customwidth_field.visible_length		= 6;
	customStr = Cvar_VariableString("r_customwidth");
	Q_strncpyz (s_customwidth_field.buffer, sizeof(s_customwidth_field.buffer), customStr);
	s_customwidth_field.cursor				= (int)strlen( customStr );

	s_customheight_title.generic.type		= MTYPE_SEPARATOR;
	s_customheight_title.generic.textSize	= MENU_FONT_SIZE;
	s_customheight_title.generic.flags		= 0;
	s_customheight_title.generic.name		= "custom height";
	s_customheight_title.generic.x			= 14.5*MENU_FONT_SIZE;
	s_customheight_title.generic.y			= y;

	s_customheight_field.generic.type		= MTYPE_FIELD;
	s_customheight_field.generic.textSize	= MENU_FONT_SIZE;
	s_customheight_field.generic.flags		= QMF_NUMBERSONLY;
//	s_customheight_field.generic.name		= "custom height";
	s_customheight_field.generic.callback	= 0;
	s_customheight_field.generic.x			= 2*MENU_FONT_SIZE;
	s_customheight_field.generic.y			= y + 1.5*MENU_LINE_SIZE;
	s_customheight_field.length				= 5;
	s_customheight_field.visible_length		= 6;
	customStr = Cvar_VariableString("r_customheight");
	Q_strncpyz (s_customheight_field.buffer, sizeof(s_customheight_field.buffer), customStr);
	s_customheight_field.cursor				= (int)strlen( customStr );

	s_fs_box.generic.type			= MTYPE_SPINCONTROL;
	s_fs_box.generic.textSize		= MENU_FONT_SIZE;
	s_fs_box.generic.x				= 0;
	s_fs_box.generic.y				= y += 3.5*MENU_LINE_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_fs_box.generic.name			= "display type";	// "fullscreen"
#else
	s_fs_box.generic.name			= "Display Type";	// "fullscreen"
#endif
	s_fs_box.itemNames				= fullscreen_names;
//	s_fs_box.generic.statusbar		= "changes bettween fullscreen and windowed display";
	s_fs_box.generic.statusbar		= "changes bettween fullscreen, borderless window, and windowed display";
	UI_MenuSpinControl_SetValue (&s_fs_box, "vid_fullscreen", 0, 2, true);

	s_brightness_slider.generic.type		= MTYPE_SLIDER;
	s_brightness_slider.generic.textSize	= MENU_FONT_SIZE;
	s_brightness_slider.generic.x			= 0;
	s_brightness_slider.generic.y			= y += MENU_LINE_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_brightness_slider.generic.name		= "brightness";
#else
	s_brightness_slider.generic.name		= "Brightness";
#endif
	s_brightness_slider.generic.callback	= BrightnessCallback;
	s_brightness_slider.maxPos				= 20;
	s_brightness_slider.baseValue			= 1.3f;
	s_brightness_slider.increment			= -0.05f;
	s_brightness_slider.displayAsPercent	= false;
	s_brightness_slider.generic.statusbar	= "changes display brightness";
	UI_MenuSlider_SetValue (&s_brightness_slider, "vid_gamma", 0.3f, 1.3f, true);

	s_texfilter_box.generic.type		= MTYPE_SPINCONTROL;
	s_texfilter_box.generic.textSize	= MENU_FONT_SIZE;
	s_texfilter_box.generic.x			= 0;
	s_texfilter_box.generic.y			= y += 2*MENU_LINE_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_texfilter_box.generic.name		= "texture filter";
#else
	s_texfilter_box.generic.name		= "Texture Filter";
#endif
	s_texfilter_box.itemNames			= texfilter_names;
	s_texfilter_box.itemValues			= texfilter_values;
	s_texfilter_box.generic.statusbar	= "changes texture filtering mode";
	UI_MenuSpinControl_SetValue (&s_texfilter_box, "r_texturemode", 0, 0, false);

	s_aniso_box.generic.type		= MTYPE_SPINCONTROL;
	s_aniso_box.generic.textSize	= MENU_FONT_SIZE;
	s_aniso_box.generic.x			= 0;
	s_aniso_box.generic.y			= y += MENU_LINE_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_aniso_box.generic.name		= "anisotropic filter";
#else
	s_aniso_box.generic.name		= "Anisotropic Filter";
#endif
	s_aniso_box.itemNames			= ui_aniso_names;
	s_aniso_box.itemValues			= ui_aniso_values;
	s_aniso_box.generic.callback	= AnisoCallback;
	s_aniso_box.generic.statusbar	= "changes level of anisotropic mipmap filtering";
	UI_MenuSpinControl_SetValue (&s_aniso_box, "r_anisotropic", 0, 0, false);

	s_texqual_box.generic.type		= MTYPE_SPINCONTROL;
	s_texqual_box.generic.textSize	= MENU_FONT_SIZE;
	s_texqual_box.generic.x			= 0;
	s_texqual_box.generic.y			= y += MENU_LINE_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_texqual_box.generic.name		= "texture quality";
#else
	s_texqual_box.generic.name		= "Texture Quality";
#endif
	s_texqual_box.itemNames			= lmh_names;
	s_texqual_box.itemValues		= lmh_values;
	s_texqual_box.generic.statusbar	= "changes maximum texture size (highest = no limit)";
	UI_MenuSpinControl_SetValue (&s_texqual_box, "r_picmip", 0, 0, false);

#ifdef NOTTHIRTYFLIGHTS
	s_npot_mipmap_box.generic.type		= MTYPE_SPINCONTROL;
	s_npot_mipmap_box.generic.textSize	= MENU_FONT_SIZE;
	s_npot_mipmap_box.generic.x			= 0;
	s_npot_mipmap_box.generic.y			= y += MENU_LINE_SIZE;
	s_npot_mipmap_box.generic.name		= "non-power-of-2 mipmaps";
	s_npot_mipmap_box.itemNames			= yesno_names;
	s_npot_mipmap_box.generic.statusbar	= "enables non-power-of-2 mipmapped textures (requires driver support)";
	UI_MenuSpinControl_SetValue (&s_npot_mipmap_box, "r_nonpoweroftwo_mipmaps", 0, 1, true);

	s_sgis_mipmap_box.generic.type		= MTYPE_SPINCONTROL;
	s_sgis_mipmap_box.generic.textSize	= MENU_FONT_SIZE;
	s_sgis_mipmap_box.generic.x			= 0;
	s_sgis_mipmap_box.generic.y			= y += MENU_LINE_SIZE;
	s_sgis_mipmap_box.generic.name		= "SGIS mipmaps";
	s_sgis_mipmap_box.itemNames			= yesno_names;
	s_sgis_mipmap_box.generic.statusbar	= "enables driver-based mipmap generation";
	UI_MenuSpinControl_SetValue (&s_sgis_mipmap_box, "r_sgis_generatemipmap", 0, 1, true);
#endif
/*
	s_texcompress_box.generic.type		= MTYPE_SPINCONTROL;
	s_texcompress_box.generic.textSize	= MENU_FONT_SIZE;
	s_texcompress_box.generic.x			= 0;
	s_texcompress_box.generic.y			= y += MENU_LINE_SIZE;
	s_texcompress_box.generic.name		= "texture compression";
	s_texcompress_box.itemNames			= yesno_names;
	s_texcompress_box.generic.statusbar	= "reduces quality, increases performance (leave off unless needed)";
	UI_MenuSpinControl_SetValue (&s_texcompress_box, "r_ext_texture_compression", 0, 1, true);
*/
#ifdef NOTTHIRTYFLIGHTS
	s_vsync_box.generic.type			= MTYPE_SPINCONTROL;
#else
	s_vsync_box.generic.type			= MTYPE_CHECKBOX;
#endif
	s_vsync_box.generic.textSize		= MENU_FONT_SIZE;
	s_vsync_box.generic.x				= 0;
	s_vsync_box.generic.y				= y += 2*MENU_LINE_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_vsync_box.generic.name			= "video sync";
#else
	s_vsync_box.generic.name			= "Video Sync";
#endif
	s_vsync_box.generic.callback		= VsyncCallback;
	s_vsync_box.itemNames				= yesno_names;
	s_vsync_box.generic.statusbar		= "sync framerate with monitor refresh";
	UI_MenuSpinControl_SetValue (&s_vsync_box, "r_swapinterval", 0, 1, true);

	// Knightmare- refresh rate option
	s_refresh_box.generic.type			= MTYPE_SPINCONTROL;
	s_refresh_box.generic.textSize		= MENU_FONT_SIZE;
	s_refresh_box.generic.x				= 0;
	s_refresh_box.generic.y				= y += MENU_LINE_SIZE;
	s_refresh_box.generic.name			= "refresh rate";
	s_refresh_box.itemNames				= refreshrate_names;
	s_refresh_box.itemValues			= refreshrate_values;
	s_refresh_box.generic.statusbar		= "sets refresh rate for fullscreen modes";
	UI_MenuSpinControl_SetValue (&s_refresh_box, "r_displayrefresh", 0, 0, false);

	s_adjust_fov_box.generic.type		= MTYPE_SPINCONTROL;
	s_adjust_fov_box.generic.textSize	= MENU_FONT_SIZE;
	s_adjust_fov_box.generic.x			= 0;
	s_adjust_fov_box.generic.y			= y += MENU_LINE_SIZE;
	s_adjust_fov_box.generic.name		= "fov autoscaling";
	s_adjust_fov_box.generic.callback	= AdjustFOVCallback;
	s_adjust_fov_box.itemNames			= yesno_names;
	s_adjust_fov_box.generic.statusbar	= "automatic scaling of fov for widescreen modes";
	UI_MenuSpinControl_SetValue (&s_adjust_fov_box, "cl_widescreen_fov", 0, 1, true);

	s_async_box.generic.type			= MTYPE_SPINCONTROL;
	s_async_box.generic.textSize		= MENU_FONT_SIZE;
	s_async_box.generic.x				= 0;
	s_async_box.generic.y				= y += MENU_LINE_SIZE;
	s_async_box.generic.name			= "async refresh";
	s_async_box.generic.callback		= AsyncCallback;
	s_async_box.itemNames				= yesno_names;
	s_async_box.generic.statusbar		= "decouples network framerate from render framerate";
	UI_MenuSpinControl_SetValue (&s_async_box, "cl_async", 0, 1, true);

	s_advanced_action.generic.type		= MTYPE_ACTION;
	s_advanced_action.generic.textSize	= MENU_FONT_SIZE;
	s_advanced_action.generic.name		= "Advanced Options";
	s_advanced_action.generic.x			= 0;
	s_advanced_action.generic.y			= y += 3*MENU_LINE_SIZE;
	s_advanced_action.generic.callback	= M_AdvancedOptions;

	s_defaults_action.generic.type		= MTYPE_ACTION;
	s_defaults_action.generic.textSize	= MENU_FONT_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_defaults_action.generic.name		= "Reset to Defaults";
#else
	s_defaults_action.generic.name		= "Reset Defaults";
#endif
	s_defaults_action.generic.x			= 0;
	s_defaults_action.generic.y			= y += 3*MENU_LINE_SIZE;
	s_defaults_action.generic.callback	= M_ResetVideoDefaults;
	s_defaults_action.generic.statusbar	= "resets all video settings to internal defaults";

	// changed cancel to apply changes, thanx to MrG
	s_apply_action.generic.type			= MTYPE_ACTION;
	s_apply_action.generic.textSize		= MENU_FONT_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_apply_action.generic.name			= "Apply Changes";
#else
	s_apply_action.generic.name			= "APPLY CHANGES";
#endif
	s_apply_action.generic.x			= 0;
	s_apply_action.generic.y			= y += 2*MENU_LINE_SIZE;
	s_apply_action.generic.callback		= M_ApplyVideoChanges;

	s_backmain_action.generic.type		= MTYPE_ACTION;
	s_backmain_action.generic.textSize	= MENU_FONT_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_backmain_action.generic.name		= "Back to Main";
#else
	s_backmain_action.generic.name		= "Done";
#endif
	s_backmain_action.generic.x			= 0;
	s_backmain_action.generic.y			= y += 2*MENU_LINE_SIZE;
	s_backmain_action.generic.callback	= UI_BackMenu;

	UI_AddMenuItem (&s_video_menu, (void *) &s_mode_list);

	UI_AddMenuItem (&s_video_menu, (void *) &s_customwidth_title);
	UI_AddMenuItem (&s_video_menu, (void *) &s_customwidth_field);
	UI_AddMenuItem (&s_video_menu, (void *) &s_customheight_title);
	UI_AddMenuItem (&s_video_menu, (void *) &s_customheight_field);

	UI_AddMenuItem (&s_video_menu, (void *) &s_fs_box);
	UI_AddMenuItem (&s_video_menu, (void *) &s_brightness_slider);
	UI_AddMenuItem (&s_video_menu, (void *) &s_texfilter_box);
	UI_AddMenuItem (&s_video_menu, (void *) &s_aniso_box);
	UI_AddMenuItem (&s_video_menu, (void *) &s_texqual_box);
#ifdef NOTTHIRTYFLIGHTS
	UI_AddMenuItem (&s_video_menu, (void *) &s_npot_mipmap_box);
	UI_AddMenuItem (&s_video_menu, (void *) &s_sgis_mipmap_box);
#endif
//	UI_AddMenuItem (&s_video_menu, (void *) &s_texcompress_box);
	UI_AddMenuItem (&s_video_menu, (void *) &s_vsync_box);
#ifdef NOTTHIRTYFLIGHTS
	UI_AddMenuItem (&s_video_menu, (void *) &s_refresh_box);
	UI_AddMenuItem (&s_video_menu, (void *) &s_adjust_fov_box);
	UI_AddMenuItem (&s_video_menu, (void *) &s_async_box);
#endif

	UI_AddMenuItem (&s_video_menu, (void *) &s_advanced_action);
	UI_AddMenuItem (&s_video_menu, (void *) &s_defaults_action);
	UI_AddMenuItem (&s_video_menu, (void *) &s_apply_action);
	UI_AddMenuItem (&s_video_menu, (void *) &s_backmain_action);

//	UI_CenterMenu (&s_video_menu);
//	s_video_menu.x -= MENU_FONT_SIZE;
}

void Menu_Video_Draw (void)
{
	//int w, h;

	// draw the banner
	UI_DrawBanner ("m_banner_video");

	// move cursor to a reasonable starting position
	UI_AdjustMenuCursor (&s_video_menu, 1);

	// draw the menu
	UI_DrawMenu (&s_video_menu);
}

const char *Menu_Video_Key (int key)
{
	return UI_DefaultMenuKey (&s_video_menu, key);
}

void Menu_Video_f (void)
{
	Menu_Video_Init ();
	UI_PushMenu (Menu_Video_Draw, Menu_Video_Key);
	M_ShowCustomFields ();
}
