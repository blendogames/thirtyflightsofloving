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

// menu_video_advanced.c -- the advanced video menu
 
#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

/*
=======================================================================

ADVANCED VIDEO MENU

=======================================================================
*/
static menuframework_s	s_video_advanced_menu;
static menuseparator_s	s_options_advanced_header;	
static menuslider_s		s_lightmapscale_slider;
static menuslider_s		s_textureintensity_slider;
static menulist_s  		s_rgbscale_box;
static menulist_s  		s_trans_lighting_box;
static menulist_s  		s_warp_lighting_box;
static menuslider_s		s_lightcutoff_slider;
static menulist_s  		s_solidalpha_box;
static menulist_s  		s_texshader_warp_box;
static menuslider_s  	s_waterwave_slider;
static menulist_s  		s_caustics_box;
static menulist_s		s_particle_overdraw_box;
static menulist_s		s_lightbloom_box;
static menulist_s		s_modelshading_box;
static menulist_s		s_shadows_box;
static menulist_s		s_two_side_stencil_box;
static menulist_s  		s_ent_shell_box;
static menulist_s  		s_celshading_box;
static menuslider_s  	s_celshading_width_slider;
static menulist_s  		s_glass_envmap_box;
static menulist_s  		s_screenshotformat_box;
static menuslider_s  	s_screenshotjpegquality_slider;
static menulist_s  		s_saveshotsize_box;
static menulist_s		s_upscale_font_box;

static menuaction_s		s_advanced_apply_action;
static menuaction_s		s_back_action;

//=======================================================================


static void LightMapScaleCallback (void *unused)
{
	UI_MenuSlider_SaveValue (&s_lightmapscale_slider, "r_modulate");
}

static void TextureIntensCallback (void *unused)
{
	UI_MenuSlider_SaveValue (&s_textureintensity_slider, "r_intensity");
}

static void RGBSCaleCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_rgbscale_box, "r_rgbscale");
}

static void TransLightingCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_trans_lighting_box, "r_trans_lighting");
}

static void WarpLightingCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_warp_lighting_box, "r_warp_lighting");
}

static void LightCutoffCallback(void *unused)
{
	UI_MenuSlider_SaveValue (&s_lightcutoff_slider, "r_lightcutoff");
}

static void GlassEnvmapCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_glass_envmap_box, "r_glass_envmaps");
}

static void SolidAlphaCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_solidalpha_box, "r_solidalpha");
}

static void TexShaderWarpCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_texshader_warp_box, "r_pixel_shader_warp");
}

static void WaterWaveCallback (void *unused)
{
	UI_MenuSlider_SaveValue (&s_waterwave_slider, "r_waterwave");
}

static void CausticsCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_caustics_box, "r_caustics");
}

static void ParticleOverdrawCallback(void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_particle_overdraw_box, "r_particle_overdraw");
}

static void LightBloomCallback(void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_lightbloom_box, "r_bloom");
}

static void ModelShadingCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_modelshading_box, "r_model_shading");
}

static void ShadowsCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_shadows_box, "r_shadows");
}

static void TwoSideStencilCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_two_side_stencil_box, "r_stencilTwoSide");
}

static void EntShellCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_ent_shell_box, "r_shelltype");
}

static void CelShadingCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_celshading_box, "r_celshading");
}

static void CelShadingWidthCallback (void *unused)
{
	UI_MenuSlider_SaveValue (&s_celshading_width_slider, "r_celshading_width");
}

static void ScreenshotFormatCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_screenshotformat_box, "r_screenshot_format");
}

static void JPEGScreenshotQualityCallback (void *unused)
{
	UI_MenuSlider_SaveValue (&s_screenshotjpegquality_slider, "r_screenshot_jpeg_quality");
}

static void SaveshotSizeCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_saveshotsize_box, "r_saveshotsize");
}

static void UpscaleFontCallback (void *unused)
{
	UI_MenuSpinControl_SaveValue (&s_upscale_font_box, "r_font_upscale");
}

//=======================================================================

static void M_AdvancedVideo_MenuSetValues (void)
{
	UI_MenuSlider_SetValue (&s_lightmapscale_slider, "r_modulate", 1, 2, true);
	UI_MenuSlider_SetValue (&s_textureintensity_slider, "r_intensity", 1, 2, true);
	UI_MenuSpinControl_SetValue (&s_rgbscale_box, "r_rgbscale", 1, 4, true);
	UI_MenuSpinControl_SetValue (&s_trans_lighting_box, "r_trans_lighting", 0, 2, true);
	UI_MenuSpinControl_SetValue (&s_warp_lighting_box, "r_warp_lighting", 0, 1, true);
	UI_MenuSlider_SetValue (&s_lightcutoff_slider, "r_lightcutoff", 0, 64, true);
	UI_MenuSpinControl_SetValue (&s_glass_envmap_box, "r_glass_envmaps", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_solidalpha_box, "r_solidalpha", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_texshader_warp_box, "r_pixel_shader_warp", 0, 1, true);
	UI_MenuSlider_SetValue (&s_waterwave_slider, "r_waterwave", 0, 24, true);
	UI_MenuSpinControl_SetValue (&s_caustics_box, "r_caustics", 0, 2, true);

	UI_MenuSpinControl_SetValue (&s_particle_overdraw_box, "r_particle_overdraw", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_lightbloom_box, "r_bloom", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_modelshading_box, "r_model_shading", 0, 3, true);
	UI_MenuSpinControl_SetValue (&s_shadows_box, "r_shadows", 0, 3, true);
	UI_MenuSpinControl_SetValue (&s_two_side_stencil_box, "r_stencilTwoSide", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_ent_shell_box, "r_shelltype", 0, 2, true);
	UI_MenuSpinControl_SetValue (&s_celshading_box, "r_celshading", 0, 1, true);
	UI_MenuSlider_SetValue (&s_celshading_width_slider, "r_celshading_width", 1, 12, true);

	UI_MenuSpinControl_SetValue (&s_screenshotformat_box, "r_screenshot_format", 0, 2, false);
	UI_MenuSlider_SetValue (&s_screenshotjpegquality_slider, "r_screenshot_jpeg_quality", 50, 100, true);
	UI_MenuSpinControl_SetValue (&s_saveshotsize_box, "r_saveshotsize", 0, 1, true);
	UI_MenuSpinControl_SetValue (&s_upscale_font_box, "r_font_upscale", 0, 2, true);
}

static void M_AdvancedMenuApplyChanges (void *unused)
{
	// update for modified r_intensity and r_stencilTwoSide
	if ( Cvar_IsModified("r_intensity") || Cvar_IsModified("r_font_upscale") )
		Cvar_SetModified ("vid_ref", true);
}

//=======================================================================

void Menu_Video_Advanced_Init (void)
{
	static const char *yesno_names[] =
	{
		"no",
		"yes",
		0
	};
	static const char *rgbscale_names[] =
	{
		"1x",
		"2x",
		"4x",
		0
	};
	static const char *rgbscale_values[] =
	{
		"1",
		"2",
		"4",
		0
	};
	static const char *lighting_names[] =
	{
		"no",
		"vertex",
		"lightmap (if available)",
		0
	};
	static const char *shading_names[] =
	{
		"off",
		"low",
		"medium",
		"high",
		0
	};
	static const char *shadow_names[] =
	{
		"no",
		"static planar",
		"dynamic planar",
		"projection",
		0
	};
	static const char *ifsupported_names[] =
	{
		"no",
		"if supported",
		0
	};
	static const char *caustics_names[] =
	{
		"no",
		"standard",
		"hardware warp (if supported)",
		0
	};
	static const char *shell_names[] =
	{
		"solid",
		"flowing",
		"envmap",
		0
	};
	static const char *screenshotformat_names[] =
	{
		"JPEG",
		"PNG",
		"TGA",
		0
	};
	static const char *screenshotformat_values[] =
	{
		"jpg",
		"png",
		"tga",
		0
	};
	static const char *font_upscale_names[] =
	{
		"no",
		"pixel copy",
		"blended",
		0
	};
	int y = 0;

	s_video_advanced_menu.x = SCREEN_WIDTH*0.5;
	s_video_advanced_menu.y = SCREEN_HEIGHT*0.5 - 100;
	s_video_advanced_menu.nitems = 0;

	s_options_advanced_header.generic.type		= MTYPE_SEPARATOR;
	s_options_advanced_header.generic.textSize	= MENU_HEADER_FONT_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_options_advanced_header.generic.name		= "Advanced Options";
#else
	s_options_advanced_header.generic.name		= "ADVANCED OPTIONS";
#endif
	s_options_advanced_header.generic.x			= MENU_HEADER_FONT_SIZE/2 * (int)strlen(s_options_advanced_header.generic.name);
	s_options_advanced_header.generic.y			= y;

	s_lightmapscale_slider.generic.type			= MTYPE_SLIDER;
	s_lightmapscale_slider.generic.textSize		= MENU_FONT_SIZE;
	s_lightmapscale_slider.generic.x			= 0;
	s_lightmapscale_slider.generic.y			= y += 2*MENU_LINE_SIZE;
	s_lightmapscale_slider.generic.name			= "lightmap scale";
	s_lightmapscale_slider.generic.callback		= LightMapScaleCallback;
	s_lightmapscale_slider.maxPos				= 10;
	s_lightmapscale_slider.baseValue			= 1.0f;
	s_lightmapscale_slider.increment			= 0.1f;
	s_lightmapscale_slider.displayAsPercent		= false;
	s_lightmapscale_slider.generic.statusbar	= "leave at minimum, washes out textures";

	s_textureintensity_slider.generic.type		= MTYPE_SLIDER;
	s_textureintensity_slider.generic.textSize	= MENU_FONT_SIZE;
	s_textureintensity_slider.generic.x			= 0;
	s_textureintensity_slider.generic.y			= y += MENU_LINE_SIZE;
	s_textureintensity_slider.generic.name		= "texture intensity";
	s_textureintensity_slider.generic.callback	= TextureIntensCallback;
	s_textureintensity_slider.maxPos			= 10;
	s_textureintensity_slider.baseValue			= 1.0f;
	s_textureintensity_slider.increment			= 0.1f;
	s_textureintensity_slider.displayAsPercent	= false;
	s_textureintensity_slider.generic.statusbar	= "leave at minimum, washes out textures";

	s_rgbscale_box.generic.type				= MTYPE_SPINCONTROL;
	s_rgbscale_box.generic.textSize			= MENU_FONT_SIZE;
	s_rgbscale_box.generic.x				= 0;
	s_rgbscale_box.generic.y				= y += MENU_LINE_SIZE;
	s_rgbscale_box.generic.name				= "RGB enhance factor";
	s_rgbscale_box.generic.callback			= RGBSCaleCallback;
	s_rgbscale_box.itemNames				= rgbscale_names;
	s_rgbscale_box.itemValues				= rgbscale_values;
	s_rgbscale_box.generic.statusbar		= "brightens textures without washing them out";

	s_trans_lighting_box.generic.type		= MTYPE_SPINCONTROL;
	s_trans_lighting_box.generic.textSize	= MENU_FONT_SIZE;
	s_trans_lighting_box.generic.x			= 0;
	s_trans_lighting_box.generic.y			= y += MENU_LINE_SIZE;
	s_trans_lighting_box.generic.name		= "translucent lighting";
	s_trans_lighting_box.generic.callback	= TransLightingCallback;
	s_trans_lighting_box.itemNames			= lighting_names;
	s_trans_lighting_box.generic.statusbar	= "lighting on translucent surfaces";

	s_warp_lighting_box.generic.type		= MTYPE_SPINCONTROL;
	s_warp_lighting_box.generic.textSize	= MENU_FONT_SIZE;
	s_warp_lighting_box.generic.x			= 0;
	s_warp_lighting_box.generic.y			= y += MENU_LINE_SIZE;
	s_warp_lighting_box.generic.name		= "warp surface lighting";
	s_warp_lighting_box.generic.callback	= WarpLightingCallback;
	s_warp_lighting_box.itemNames			= yesno_names;
	s_warp_lighting_box.generic.statusbar	= "vertex lighting on water and other warping surfaces";

	s_lightcutoff_slider.generic.type		= MTYPE_SLIDER;
	s_lightcutoff_slider.generic.textSize	= MENU_FONT_SIZE;
	s_lightcutoff_slider.generic.x			= 0;
	s_lightcutoff_slider.generic.y			= y += MENU_LINE_SIZE;
	s_lightcutoff_slider.generic.name		= "dynamic light cutoff";
	s_lightcutoff_slider.generic.callback	= LightCutoffCallback;
	s_lightcutoff_slider.maxPos				= 8;
	s_lightcutoff_slider.baseValue			= 0.0f;
	s_lightcutoff_slider.increment			= 8.0f;
	s_lightcutoff_slider.displayAsPercent	= false;
	s_lightcutoff_slider.generic.statusbar	= "lower = smoother blend, higher = faster";

	s_glass_envmap_box.generic.type			= MTYPE_SPINCONTROL;
	s_glass_envmap_box.generic.textSize		= MENU_FONT_SIZE;
	s_glass_envmap_box.generic.x			= 0;
	s_glass_envmap_box.generic.y			= y += MENU_LINE_SIZE;
	s_glass_envmap_box.generic.name			= "glass envmaps";
	s_glass_envmap_box.generic.callback		= GlassEnvmapCallback;
	s_glass_envmap_box.itemNames			= yesno_names;
	s_glass_envmap_box.generic.statusbar	= "enable environment mapping on transparent surfaces";

	s_solidalpha_box.generic.type			= MTYPE_SPINCONTROL;
	s_solidalpha_box.generic.textSize		= MENU_FONT_SIZE;
	s_solidalpha_box.generic.x				= 0;
	s_solidalpha_box.generic.y				= y += MENU_LINE_SIZE;
	s_solidalpha_box.generic.name			= "solid alphas";
	s_solidalpha_box.generic.callback		= SolidAlphaCallback;
	s_solidalpha_box.itemNames				= yesno_names;
	s_solidalpha_box.generic.statusbar		= "enable solid drawing of trans33 + trans66 surfaces";

	s_texshader_warp_box.generic.type		= MTYPE_SPINCONTROL;
	s_texshader_warp_box.generic.textSize	= MENU_FONT_SIZE;
	s_texshader_warp_box.generic.x			= 0;
	s_texshader_warp_box.generic.y			= y += MENU_LINE_SIZE;
	s_texshader_warp_box.generic.name		= "texture shader warp";
	s_texshader_warp_box.generic.callback	= TexShaderWarpCallback;
	s_texshader_warp_box.itemNames			= ifsupported_names;
	s_texshader_warp_box.generic.statusbar	= "enables hardware water warping effect";

	s_waterwave_slider.generic.type			= MTYPE_SLIDER;
	s_waterwave_slider.generic.textSize		= MENU_FONT_SIZE;
	s_waterwave_slider.generic.x			= 0;
	s_waterwave_slider.generic.y			= y += MENU_LINE_SIZE;
	s_waterwave_slider.generic.name			= "water wave size";
	s_waterwave_slider.generic.callback		= WaterWaveCallback;
	s_waterwave_slider.maxPos				= 24;
	s_waterwave_slider.baseValue			= 0.0f;
	s_waterwave_slider.increment			= 1.0f;
	s_waterwave_slider.displayAsPercent		= false;
	s_waterwave_slider.generic.statusbar	= "size of waves on flat water surfaces";

	s_caustics_box.generic.type				= MTYPE_SPINCONTROL;
	s_caustics_box.generic.textSize			= MENU_FONT_SIZE;
	s_caustics_box.generic.x				= 0;
	s_caustics_box.generic.y				= y += MENU_LINE_SIZE;
	s_caustics_box.generic.name				= "underwater caustics";
	s_caustics_box.generic.callback			= CausticsCallback;
	s_caustics_box.itemNames				= caustics_names;
	s_caustics_box.generic.statusbar		= "caustic effect on underwater surfaces";

	s_particle_overdraw_box.generic.type		= MTYPE_SPINCONTROL;
	s_particle_overdraw_box.generic.textSize	= MENU_FONT_SIZE;
	s_particle_overdraw_box.generic.x			= 0;
	s_particle_overdraw_box.generic.y			= y += 2*MENU_LINE_SIZE;
	s_particle_overdraw_box.generic.name		= "particle overdraw";
	s_particle_overdraw_box.generic.callback	= ParticleOverdrawCallback;
	s_particle_overdraw_box.itemNames			= yesno_names;
	s_particle_overdraw_box.generic.statusbar	= "redraw particles over trans surfaces";

	s_lightbloom_box.generic.type			= MTYPE_SPINCONTROL;
	s_lightbloom_box.generic.textSize		= MENU_FONT_SIZE;
	s_lightbloom_box.generic.x				= 0;
	s_lightbloom_box.generic.y				= y += MENU_LINE_SIZE;
	s_lightbloom_box.generic.name			= "light blooms";
	s_lightbloom_box.generic.callback		= LightBloomCallback;
	s_lightbloom_box.itemNames				= yesno_names;
	s_lightbloom_box.generic.statusbar		= "enables blooming of bright lights";

	s_modelshading_box.generic.type			= MTYPE_SPINCONTROL;
	s_modelshading_box.generic.textSize		= MENU_FONT_SIZE;
	s_modelshading_box.generic.x			= 0;
	s_modelshading_box.generic.y			= y += MENU_LINE_SIZE;
	s_modelshading_box.generic.name			= "model shading";
	s_modelshading_box.generic.callback		= ModelShadingCallback;
	s_modelshading_box.itemNames			= shading_names;
	s_modelshading_box.generic.statusbar	= "level of shading to use on models";

	s_shadows_box.generic.type				= MTYPE_SPINCONTROL;
	s_shadows_box.generic.textSize			= MENU_FONT_SIZE;
	s_shadows_box.generic.x					= 0;
	s_shadows_box.generic.y					= y += MENU_LINE_SIZE;
	s_shadows_box.generic.name				= "entity shadows";
	s_shadows_box.generic.callback			= ShadowsCallback;
	s_shadows_box.itemNames					= shadow_names;
	s_shadows_box.generic.statusbar			= "type of model shadows to draw";

	s_two_side_stencil_box.generic.type			= MTYPE_SPINCONTROL;
	s_two_side_stencil_box.generic.textSize		= MENU_FONT_SIZE;
	s_two_side_stencil_box.generic.x			= 0;
	s_two_side_stencil_box.generic.y			= y += MENU_LINE_SIZE;
	s_two_side_stencil_box.generic.name			= "two-sided stenciling";
	s_two_side_stencil_box.generic.callback		= TwoSideStencilCallback;
	s_two_side_stencil_box.itemNames			= ifsupported_names;
	s_two_side_stencil_box.generic.statusbar	= "use single-pass shadow stenciling";

	s_ent_shell_box.generic.type				= MTYPE_SPINCONTROL;
	s_ent_shell_box.generic.textSize			= MENU_FONT_SIZE;
	s_ent_shell_box.generic.x					= 0;
	s_ent_shell_box.generic.y					= y += MENU_LINE_SIZE;
	s_ent_shell_box.generic.name				= "entity shell type";
	s_ent_shell_box.generic.callback			= EntShellCallback;
	s_ent_shell_box.itemNames					= shell_names;
	s_ent_shell_box.generic.statusbar			= "envmap effect may cause instability on ATI cards";

	s_celshading_box.generic.type				= MTYPE_SPINCONTROL;
	s_celshading_box.generic.textSize			= MENU_FONT_SIZE;
	s_celshading_box.generic.x					= 0;
	s_celshading_box.generic.y					= y += MENU_LINE_SIZE;
	s_celshading_box.generic.name				= "cel shading";
	s_celshading_box.generic.callback			= CelShadingCallback;
	s_celshading_box.itemNames					= yesno_names;
	s_celshading_box.generic.statusbar			= "cartoon-style rendering of models";

	s_celshading_width_slider.generic.type		= MTYPE_SLIDER;
	s_celshading_width_slider.generic.textSize	= MENU_FONT_SIZE;
	s_celshading_width_slider.generic.x			= 0;
	s_celshading_width_slider.generic.y			= y += MENU_LINE_SIZE;
	s_celshading_width_slider.generic.name		= "cel shading width";
	s_celshading_width_slider.generic.callback	= CelShadingWidthCallback;
	s_celshading_width_slider.maxPos			= 11;
	s_celshading_width_slider.baseValue			= 1.0f;
	s_celshading_width_slider.increment			= 1.0f;
	s_celshading_width_slider.displayAsPercent	= false;
	s_celshading_width_slider.generic.statusbar	= "width of cel shading outlines";

	s_screenshotformat_box.generic.type			= MTYPE_SPINCONTROL;
	s_screenshotformat_box.generic.textSize		= MENU_FONT_SIZE;
	s_screenshotformat_box.generic.x			= 0;
	s_screenshotformat_box.generic.y			= y += 2*MENU_LINE_SIZE;
	s_screenshotformat_box.generic.name			= "screenshot format";
	s_screenshotformat_box.generic.callback		= ScreenshotFormatCallback;
	s_screenshotformat_box.itemNames			= screenshotformat_names;
	s_screenshotformat_box.itemValues			= screenshotformat_values;
	s_screenshotformat_box.generic.statusbar	= "image format for screenshots";

	s_screenshotjpegquality_slider.generic.type			= MTYPE_SLIDER;
	s_screenshotjpegquality_slider.generic.textSize		= MENU_FONT_SIZE;
	s_screenshotjpegquality_slider.generic.x			= 0;
	s_screenshotjpegquality_slider.generic.y			= y += MENU_LINE_SIZE;
	s_screenshotjpegquality_slider.generic.name			= "JPEG screenshot quality";
	s_screenshotjpegquality_slider.generic.callback		= JPEGScreenshotQualityCallback;
	s_screenshotjpegquality_slider.maxPos				= 10;
	s_screenshotjpegquality_slider.baseValue			= 50.0f;
	s_screenshotjpegquality_slider.increment			= 5.0f;
	s_screenshotjpegquality_slider.displayAsPercent		= false;
	s_screenshotjpegquality_slider.generic.statusbar	= "quality of JPG screenshots, 50-100%";

	s_saveshotsize_box.generic.type				= MTYPE_SPINCONTROL;
	s_saveshotsize_box.generic.textSize			= MENU_FONT_SIZE;
	s_saveshotsize_box.generic.x				= 0;
	s_saveshotsize_box.generic.y				= y += MENU_LINE_SIZE;
	s_saveshotsize_box.generic.name				= "hi-res saveshots";
	s_saveshotsize_box.generic.callback			= SaveshotSizeCallback;
	s_saveshotsize_box.itemNames				= yesno_names;
	s_saveshotsize_box.generic.statusbar		= "hi-res saveshots when running at 800x600 or higher";

	s_upscale_font_box.generic.type				= MTYPE_SPINCONTROL;
	s_upscale_font_box.generic.textSize			= MENU_FONT_SIZE;
	s_upscale_font_box.generic.x				= 0;
	s_upscale_font_box.generic.y				= y += 2*MENU_LINE_SIZE;
	s_upscale_font_box.generic.name				= "upscale old fonts";
	s_upscale_font_box.generic.callback			= UpscaleFontCallback;
	s_upscale_font_box.itemNames				= font_upscale_names;
	s_upscale_font_box.generic.statusbar		= "upscales 128x128 fonts to higher res based on screen resolution";

	s_advanced_apply_action.generic.type		= MTYPE_ACTION;
	s_advanced_apply_action.generic.textSize	= MENU_FONT_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_advanced_apply_action.generic.name		= "Apply Changes";
#else
	s_advanced_apply_action.generic.name		= "APPLY CHANGES";
#endif
	s_advanced_apply_action.generic.x			= 0;
	s_advanced_apply_action.generic.y			= y += 2*MENU_LINE_SIZE;
	s_advanced_apply_action.generic.callback	= M_AdvancedMenuApplyChanges;

	s_back_action.generic.type					= MTYPE_ACTION;
	s_back_action.generic.textSize				= MENU_FONT_SIZE;
#ifdef NOTTHIRTYFLIGHTS
	s_back_action.generic.name					= "Back";
#else
	s_back_action.generic.name					= "Done";
#endif
	s_back_action.generic.x						= 0;
	s_back_action.generic.y						= y += 2*MENU_LINE_SIZE;
	s_back_action.generic.callback				= UI_BackMenu;

	M_AdvancedVideo_MenuSetValues ();

	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_options_advanced_header);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_lightmapscale_slider);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_textureintensity_slider);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_rgbscale_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_trans_lighting_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_warp_lighting_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_lightcutoff_slider);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_glass_envmap_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_solidalpha_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_texshader_warp_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_waterwave_slider);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_caustics_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_particle_overdraw_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_lightbloom_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_modelshading_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_shadows_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_two_side_stencil_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_ent_shell_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_celshading_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_celshading_width_slider);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_screenshotformat_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_screenshotjpegquality_slider);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_saveshotsize_box);
	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_upscale_font_box);

	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_advanced_apply_action );

	UI_AddMenuItem (&s_video_advanced_menu, (void *) &s_back_action );

//	UI_CenterMenu (&s_video_advanced_menu);
//	s_video_advanced_menu.x -= MENU_FONT_SIZE;	
}

void Menu_Video_Advanced_Draw (void)
{
//	int w, h;

	// draw the banner
	UI_DrawBanner ("m_banner_video");

	// move cursor to a reasonable starting position
	UI_AdjustMenuCursor (&s_video_advanced_menu, 1);

	// draw the menu
	UI_DrawMenu (&s_video_advanced_menu);
}

const char *Menu_Video_Advanced_Key (int key)
{
	return UI_DefaultMenuKey (&s_video_advanced_menu, key);

}

void Menu_Video_Advanced_f (void)
{
	Menu_Video_Advanced_Init ();
	UI_PushMenu (Menu_Video_Advanced_Draw, Menu_Video_Advanced_Key);
}
