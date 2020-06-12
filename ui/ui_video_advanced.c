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

// ui_video_advanced.c -- the advanced video menu
 
#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

extern cvar_t *vid_ref;
// this cvar is needed for checking if it's been modified
cvar_t	*r_intensity;

/*
=======================================================================

ADVANCED VIDEO MENU

=======================================================================
*/
static menuframework_s	s_video_advanced_menu;
static menuseparator_s	s_options_advanced_header;	
static menuslider_s		s_lightmapscale_slider;
static menuslider_s		s_textureintensity_slider;
static menulist_s  		s_overbrights_box;
static menulist_s  		s_trans_lightmap_box;
static menulist_s  		s_texshader_warp_box;
static menulist_s  		s_solidalpha_box;
static menuslider_s  	s_waterwave_slider;
static menulist_s  		s_caustics_box;
static menuslider_s		s_particle_comp_slider;
static menulist_s		s_particle_overdraw_box;
static menulist_s		s_lightbloom_box;
static menuslider_s		s_decal_slider;
static menulist_s		s_modelshading_box;
static menulist_s		s_shadows_box;
static menulist_s		s_two_side_stencil_box;
static menulist_s  		s_ent_shell_box;
static menulist_s  		s_glass_envmap_box;
static menulist_s  		s_screenshotjpeg_box;
static menuslider_s  	s_screenshotjpegquality_slider;
static menulist_s  		s_saveshotsize_box;
static menuaction_s		s_advanced_apply_action;
static menuaction_s		s_back_action;


static void Video_Advanced_MenuSetValues ( void )
{
	Cvar_SetValue( "r_modulate", ClampCvar( 1, 2, Cvar_VariableValue("r_modulate") ) );
	s_lightmapscale_slider.curvalue = (Cvar_VariableValue("r_modulate") -1) * 10;

	Cvar_SetValue( "r_intensity", ClampCvar( 1, 2, Cvar_VariableValue("r_intensity") ) );
	s_textureintensity_slider.curvalue = (Cvar_VariableValue("r_intensity") -1) * 10;

	Cvar_SetValue( "r_overbrightbits", ClampCvar( 1, 2, Cvar_VariableValue("r_overbrightbits") ) );
	if (Cvar_VariableValue("r_overbrightbits") == 1)
		s_overbrights_box.curvalue = 0;
	else
		s_overbrights_box.curvalue = 1;

	Cvar_SetValue( "r_trans_lightmaps", ClampCvar( 0, 1, Cvar_VariableValue("r_trans_lightmaps") ) );
	s_trans_lightmap_box.curvalue	= Cvar_VariableValue("r_trans_lightmaps");

	Cvar_SetValue( "r_glass_envmaps", ClampCvar( 0, 1, Cvar_VariableValue("r_glass_envmaps") ) );
	s_glass_envmap_box.curvalue	= Cvar_VariableValue("r_glass_envmaps");

	Cvar_SetValue( "r_solidalpha", ClampCvar( 0, 1, Cvar_VariableValue("r_solidalpha") ) );
	s_solidalpha_box.curvalue	= Cvar_VariableValue("r_solidalpha");

	Cvar_SetValue( "r_pixel_shader_warp", ClampCvar( 0, 1, Cvar_VariableValue("r_pixel_shader_warp") ) );
	s_texshader_warp_box.curvalue	= Cvar_VariableValue("r_pixel_shader_warp");

	s_waterwave_slider.curvalue		= Cvar_VariableValue("r_waterwave");

	Cvar_SetValue( "r_caustics", ClampCvar( 0, 1, Cvar_VariableValue("r_caustics") ) );
	s_caustics_box.curvalue = Cvar_VariableValue("r_caustics");

	Cvar_SetValue( "cl_particle_scale", ClampCvar( 0, 5, Cvar_VariableValue("cl_particle_scale") ) );
	s_particle_comp_slider.curvalue	= (Cvar_VariableValue("cl_particle_scale") -3)*-1+3;

	Cvar_SetValue( "r_particle_overdraw", ClampCvar( 0, 1, Cvar_VariableValue("r_particle_overdraw") ) );
	s_particle_overdraw_box.curvalue	= Cvar_VariableValue("r_particle_overdraw");

	Cvar_SetValue( "r_bloom", ClampCvar( 0, 1, Cvar_VariableValue("r_bloom") ) );
	s_lightbloom_box.curvalue	= Cvar_VariableValue("r_bloom");

	Cvar_SetValue( "r_decals", ClampCvar (0, 1000, Cvar_VariableValue("r_decals")) );
	s_decal_slider.curvalue		= Cvar_VariableValue("r_decals") / 50;

	Cvar_SetValue( "r_model_shading", ClampCvar( 0, 3, Cvar_VariableValue("r_model_shading") ) );
	s_modelshading_box.curvalue	= Cvar_VariableValue("r_model_shading");

	Cvar_SetValue( "r_shadows", ClampCvar( 0, 3, Cvar_VariableValue("r_shadows") ) );
	s_shadows_box.curvalue	= Cvar_VariableValue("r_shadows");

	Cvar_SetValue( "r_stencilTwoSide", ClampCvar( 0, 1, Cvar_VariableValue("r_stencilTwoSide") ) );
	s_two_side_stencil_box.curvalue = Cvar_VariableValue("r_stencilTwoSide");

	Cvar_SetValue( "r_shelltype", ClampCvar( 0, 2, Cvar_VariableValue("r_shelltype") ) );
	s_ent_shell_box.curvalue	= Cvar_VariableValue("r_shelltype");

	Cvar_SetValue( "r_screenshot_jpeg", ClampCvar( 0, 1, Cvar_VariableValue("r_screenshot_jpeg") ) );
	s_screenshotjpeg_box.curvalue	= Cvar_VariableValue("r_screenshot_jpeg");

	Cvar_SetValue( "r_screenshot_jpeg_quality", ClampCvar( 50, 100, Cvar_VariableValue("r_screenshot_jpeg_quality") ) );
	s_screenshotjpegquality_slider.curvalue	= (Cvar_VariableValue("r_screenshot_jpeg_quality") -50) / 5;

	Cvar_SetValue( "r_saveshotsize", ClampCvar( 0, 1, Cvar_VariableValue("r_saveshotsize") ) );
	s_saveshotsize_box.curvalue	= Cvar_VariableValue("r_saveshotsize");
}

static void LightMapScaleCallback ( void *unused )
{
	Cvar_SetValue( "r_modulate", s_lightmapscale_slider.curvalue / 10 + 1);
}

static void TextureIntensCallback ( void *unused )
{
	Cvar_SetValue( "r_intensity", s_textureintensity_slider.curvalue / 10 + 1);
}

static void OverbrightsCallback ( void *unused )
{
	Cvar_SetValue( "r_overbrightbits", s_overbrights_box.curvalue + 1);
}

static void TransFullbrightCallback ( void *unused )
{
	Cvar_SetValue( "r_trans_lightmaps", s_trans_lightmap_box.curvalue);
}

static void GlassEnvmapCallback ( void *unused )
{
	Cvar_SetValue( "r_glass_envmaps", s_glass_envmap_box.curvalue);
}

static void SolidAlphaCallback ( void *unused )
{
	Cvar_SetValue( "r_solidalpha", s_solidalpha_box.curvalue);
}

static void TexShaderWarpCallback ( void *unused )
{
	Cvar_SetValue( "r_pixel_shader_warp", s_texshader_warp_box.curvalue);
}

static void WaterWaveCallback ( void *unused )
{
	Cvar_SetValue( "r_waterwave", s_waterwave_slider.curvalue);
}

static void CausticsCallback ( void *unused )
{
	Cvar_SetValue( "r_caustics", s_caustics_box.curvalue);
}

static void ParticleCompFunc( void *unused )
{
	Cvar_SetValue( "cl_particle_scale", (s_particle_comp_slider.curvalue-3)*-1+3);
}

static void ParticleOverdrawCallback( void *unused )
{
	Cvar_SetValue( "r_particle_overdraw", s_particle_overdraw_box.curvalue);
}

static void LightBloomCallback( void *unused )
{
	Cvar_SetValue( "r_bloom", s_lightbloom_box.curvalue);
}

// Knightmare- decal option
static void DecalFunc( void *unused )
{
	Cvar_SetValue( "r_decals", s_decal_slider.curvalue * 50);
}

static void ModelShadingCallback ( void *unused )
{
	Cvar_SetValue( "r_model_shading", s_modelshading_box.curvalue);
}

static void ShadowsCallback ( void *unused )
{
	Cvar_SetValue( "r_shadows", s_shadows_box.curvalue);
}

static void TwoSideStencilCallback ( void *unused )
{
	Cvar_SetValue( "r_stencilTwoSide", s_two_side_stencil_box.curvalue);
}

static void EntShellCallback ( void *unused )
{
	Cvar_SetValue( "r_shelltype", s_ent_shell_box.curvalue);
}

static void JPEGScreenshotCallback ( void *unused )
{
	Cvar_SetValue( "r_screenshot_jpeg", s_screenshotjpeg_box.curvalue);
}

static void JPEGScreenshotQualityCallback ( void *unused )
{
	Cvar_SetValue( "r_screenshot_jpeg_quality", (s_screenshotjpegquality_slider.curvalue * 5 + 50));
}

static void SaveshotSizeCallback ( void *unused )
{
	Cvar_SetValue( "r_saveshotsize", s_saveshotsize_box.curvalue);
}

static void AdvancedMenuApplyChanges ( void *unused )
{
	// update for modified r_intensity and r_stencilTwoSide
	if (r_intensity->modified)
		vid_ref->modified = true;
}

/*
================
Menu_Video_Advanced_Init
================
*/
void Menu_Video_Advanced_Init (void)
{
	static const char *yesno_names[] =
	{
		"no",
		"yes",
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
	static const char *shell_names[] =
	{
		"solid",
		"flowing",
		"envmap",
		0
	};
	int y = 0;

	r_intensity = Cvar_Get ("r_intensity", "1", 0);

	s_video_advanced_menu.x = SCREEN_WIDTH*0.5;
//	s_video_advanced_menu.x = viddef.width * 0.50;
	s_video_advanced_menu.y = SCREEN_HEIGHT*0.5 - 100;
	s_video_advanced_menu.nitems = 0;

	s_options_advanced_header.generic.type	= MTYPE_SEPARATOR;
	s_options_advanced_header.generic.name	= "ADVANCED OPTIONS";
	s_options_advanced_header.generic.x		= MENU_FONT_SIZE/2 * strlen(s_options_advanced_header.generic.name);
	s_options_advanced_header.generic.y		= y;

	s_lightmapscale_slider.generic.type		= MTYPE_SLIDER;
	s_lightmapscale_slider.generic.x		= 0;
	s_lightmapscale_slider.generic.y		= y += 2*MENU_LINE_SIZE;
	s_lightmapscale_slider.generic.name		= "lightmap scale";
	s_lightmapscale_slider.generic.callback = LightMapScaleCallback;
	s_lightmapscale_slider.minvalue			= 0;
	s_lightmapscale_slider.maxvalue			= 10;

	s_textureintensity_slider.generic.type		= MTYPE_SLIDER;
	s_textureintensity_slider.generic.x			= 0;
	s_textureintensity_slider.generic.y			= y += MENU_LINE_SIZE;
	s_textureintensity_slider.generic.name		= "texture intensity";
	s_textureintensity_slider.generic.callback	= TextureIntensCallback;
	s_textureintensity_slider.minvalue			= 0;
	s_textureintensity_slider.maxvalue			= 10;

	s_overbrights_box.generic.type		= MTYPE_CHECKBOX;
	s_overbrights_box.generic.x			= 0;
	s_overbrights_box.generic.y			= y += MENU_LINE_SIZE;
	s_overbrights_box.generic.name		= "overbrights";
	s_overbrights_box.generic.callback	= OverbrightsCallback;
	s_overbrights_box.itemnames			= yesno_names;

	s_trans_lightmap_box.generic.type		= MTYPE_CHECKBOX;
	s_trans_lightmap_box.generic.x			= 0;
	s_trans_lightmap_box.generic.y			= y += MENU_LINE_SIZE;
	s_trans_lightmap_box.generic.name		= "translucent lighting";
	s_trans_lightmap_box.generic.callback	= TransFullbrightCallback;
	s_trans_lightmap_box.itemnames			= yesno_names;
	s_trans_lightmap_box.generic.statusbar	= "vertex lighting on trans/warp surfaces, may crash on some maps";

	s_glass_envmap_box.generic.type		= MTYPE_CHECKBOX;
	s_glass_envmap_box.generic.x		= 0;
	s_glass_envmap_box.generic.y		= y += MENU_LINE_SIZE;
	s_glass_envmap_box.generic.name		= "glass envmaps";
	s_glass_envmap_box.generic.callback	= GlassEnvmapCallback;
	s_glass_envmap_box.itemnames		= yesno_names;
	s_glass_envmap_box.generic.statusbar	= "enable environment mapping on transparent surfaces";

	s_solidalpha_box.generic.type		= MTYPE_CHECKBOX;
	s_solidalpha_box.generic.x			= 0;
	s_solidalpha_box.generic.y			= y += MENU_LINE_SIZE;
	s_solidalpha_box.generic.name		= "solid alphas";
	s_solidalpha_box.generic.callback	= SolidAlphaCallback;
	s_solidalpha_box.itemnames			= yesno_names;
	s_solidalpha_box.generic.statusbar	= "enable solid drawing of trans33 + trans66 surfaces";

	s_texshader_warp_box.generic.type		= MTYPE_CHECKBOX;
	s_texshader_warp_box.generic.x			= 0;
	s_texshader_warp_box.generic.y			= y += MENU_LINE_SIZE;
	s_texshader_warp_box.generic.name		= "texture shader warp";
	s_texshader_warp_box.generic.callback	= TexShaderWarpCallback;
	s_texshader_warp_box.itemnames			= ifsupported_names;

	s_waterwave_slider.generic.type		= MTYPE_SLIDER;
	s_waterwave_slider.generic.x		= 0;
	s_waterwave_slider.generic.y		= y += MENU_LINE_SIZE;
	s_waterwave_slider.generic.name		= "water wave size";
	s_waterwave_slider.generic.callback	= WaterWaveCallback;
	s_waterwave_slider.minvalue			= 0;
	s_waterwave_slider.maxvalue			= 24;
	s_waterwave_slider.generic.statusbar = "size of waves on flat water surfaces";

	s_caustics_box.generic.type		= MTYPE_CHECKBOX;
	s_caustics_box.generic.x		= 0;
	s_caustics_box.generic.y		= y += MENU_LINE_SIZE;
	s_caustics_box.generic.name		= "underwater caustics";
	s_caustics_box.generic.callback	= CausticsCallback;
	s_caustics_box.itemnames		= yesno_names;
	s_caustics_box.generic.statusbar	= "caustic effect on underwater surfaces";

	s_particle_comp_slider.generic.type			= MTYPE_SLIDER;
	s_particle_comp_slider.generic.x			= 0;
	s_particle_comp_slider.generic.y			= y += 2*MENU_LINE_SIZE;
	s_particle_comp_slider.generic.name			= "particle effect complexity";
	s_particle_comp_slider.generic.callback		= ParticleCompFunc;
	s_particle_comp_slider.minvalue				= 1;
	s_particle_comp_slider.maxvalue				= 5;
	s_particle_comp_slider.generic.statusbar	= "lower = faster performance";

	s_particle_overdraw_box.generic.type		= MTYPE_CHECKBOX;
	s_particle_overdraw_box.generic.x			= 0;
	s_particle_overdraw_box.generic.y			= y += MENU_LINE_SIZE;
	s_particle_overdraw_box.generic.name		= "particle overdraw";
	s_particle_overdraw_box.generic.callback	= ParticleOverdrawCallback;
	s_particle_overdraw_box.itemnames			= yesno_names;
	s_particle_overdraw_box.generic.statusbar	= "redraw particles over trans surfaces";

	s_lightbloom_box.generic.type		= MTYPE_CHECKBOX;
	s_lightbloom_box.generic.x			= 0;
	s_lightbloom_box.generic.y			= y += MENU_LINE_SIZE;
	s_lightbloom_box.generic.name		= "light blooms";
	s_lightbloom_box.generic.callback	= LightBloomCallback;
	s_lightbloom_box.itemnames			= yesno_names;

	s_decal_slider.generic.type			= MTYPE_SLIDER;
	s_decal_slider.generic.x			= 0;
	s_decal_slider.generic.y			= y += MENU_LINE_SIZE;
	s_decal_slider.generic.name			= "decal quantity";
	s_decal_slider.generic.callback		= DecalFunc;
	s_decal_slider.minvalue				= 0;
	s_decal_slider.maxvalue				= 20;
	s_decal_slider.generic.statusbar	= "how many decals to display at once (max = 1000)";

	s_modelshading_box.generic.type		= MTYPE_SPINCONTROL;
	s_modelshading_box.generic.x		= 0;
	s_modelshading_box.generic.y		= y += MENU_LINE_SIZE;
	s_modelshading_box.generic.name		= "model shading";
	s_modelshading_box.generic.callback	= ModelShadingCallback;
	s_modelshading_box.itemnames		= shading_names;
	s_modelshading_box.generic.statusbar	= "level of shading to use on models";

	s_shadows_box.generic.type		= MTYPE_SPINCONTROL;
	s_shadows_box.generic.x			= 0;
	s_shadows_box.generic.y			= y += MENU_LINE_SIZE;
	s_shadows_box.generic.name		= "entity shadows";
	s_shadows_box.generic.callback	= ShadowsCallback;
	s_shadows_box.itemnames			= shadow_names;
	s_shadows_box.generic.statusbar	= "type of model shadows to draw";

	s_two_side_stencil_box.generic.type		= MTYPE_CHECKBOX;
	s_two_side_stencil_box.generic.x		= 0;
	s_two_side_stencil_box.generic.y		= y += MENU_LINE_SIZE;
	s_two_side_stencil_box.generic.name		= "two-sided stenciling";
	s_two_side_stencil_box.generic.callback	= TwoSideStencilCallback;
	s_two_side_stencil_box.itemnames		= ifsupported_names;
	s_two_side_stencil_box.generic.statusbar = "use single-pass shadow stenciling";

	s_ent_shell_box.generic.type		= MTYPE_SPINCONTROL;
	s_ent_shell_box.generic.x			= 0;
	s_ent_shell_box.generic.y			= y += MENU_LINE_SIZE;
	s_ent_shell_box.generic.name		= "entity shell type";
	s_ent_shell_box.generic.callback	= EntShellCallback;
	s_ent_shell_box.itemnames			= shell_names;
	s_ent_shell_box.generic.statusbar	= "envmap effect may cause instability on ATI cards";

	s_screenshotjpeg_box.generic.type		= MTYPE_CHECKBOX;
	s_screenshotjpeg_box.generic.x			= 0;
	s_screenshotjpeg_box.generic.y			= y += 2*MENU_LINE_SIZE;
	s_screenshotjpeg_box.generic.name		= "JPEG screenshots";
	s_screenshotjpeg_box.generic.callback	= JPEGScreenshotCallback;
	s_screenshotjpeg_box.itemnames			= yesno_names;
	s_screenshotjpeg_box.generic.statusbar	= "whether to take JPG screenshots instead of TGA";

	s_screenshotjpegquality_slider.generic.type	= MTYPE_SLIDER;
	s_screenshotjpegquality_slider.generic.x	= 0;
	s_screenshotjpegquality_slider.generic.y	= y += MENU_LINE_SIZE;
	s_screenshotjpegquality_slider.generic.name	= "JPEG screenshot quality";
	s_screenshotjpegquality_slider.generic.callback		= JPEGScreenshotQualityCallback;
	s_screenshotjpegquality_slider.minvalue		= 0;
	s_screenshotjpegquality_slider.maxvalue		= 10;
	s_screenshotjpegquality_slider.generic.statusbar = "quality of JPG screenshots, 50-100%";

	s_saveshotsize_box.generic.type			= MTYPE_CHECKBOX;
	s_saveshotsize_box.generic.x			= 0;
	s_saveshotsize_box.generic.y			= y += MENU_LINE_SIZE;
	s_saveshotsize_box.generic.name			= "hi-res saveshots";
	s_saveshotsize_box.generic.callback		= SaveshotSizeCallback;
	s_saveshotsize_box.itemnames			= yesno_names;
	s_saveshotsize_box.generic.statusbar	= "hi-res saveshots when running at 800x600 or higher";

	s_advanced_apply_action.generic.type		= MTYPE_ACTION;
	s_advanced_apply_action.generic.name		= "APPLY CHANGES";
	s_advanced_apply_action.generic.x			= 0;
	s_advanced_apply_action.generic.y			= y += 2*MENU_LINE_SIZE;
	s_advanced_apply_action.generic.callback	= AdvancedMenuApplyChanges;

	s_back_action.generic.type = MTYPE_ACTION;
	s_back_action.generic.name = "Done";
	s_back_action.generic.x    = 0;
	s_back_action.generic.y    = y += 2*MENU_LINE_SIZE;
	s_back_action.generic.callback = UI_BackMenu;

	Video_Advanced_MenuSetValues();

	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_options_advanced_header );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_lightmapscale_slider );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_textureintensity_slider );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_overbrights_box );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_trans_lightmap_box );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_glass_envmap_box );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_solidalpha_box );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_texshader_warp_box );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_waterwave_slider );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_caustics_box );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_particle_comp_slider );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_particle_overdraw_box );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_lightbloom_box );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_decal_slider );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_modelshading_box );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_shadows_box );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_two_side_stencil_box );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_ent_shell_box );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_screenshotjpeg_box );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_screenshotjpegquality_slider );
	Menu_AddItem( &s_video_advanced_menu, ( void * ) &	s_saveshotsize_box );

	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_advanced_apply_action );

	Menu_AddItem( &s_video_advanced_menu, ( void * ) &s_back_action );

//	Menu_Center( &s_video_advanced_menu );
//	s_video_advanced_menu.x -= MENU_FONT_SIZE;	
}

/*
================
Menu_Video_Advanced_Draw
================
*/
void Menu_Video_Advanced_Draw (void)
{
	//int w, h;

	// draw the banner
	Menu_Banner("m_banner_video");

	// move cursor to a reasonable starting position
	Menu_AdjustCursor( &s_video_advanced_menu, 1 );

	// draw the menu
	Menu_Draw( &s_video_advanced_menu );
}

/*
================
Video_Advanced_MenuKey
================
*/
const char *Video_Advanced_MenuKey( int key )
{
	return Default_MenuKey( &s_video_advanced_menu, key );

}

void M_Menu_Video_Advanced_f (void)
{
	Menu_Video_Advanced_Init();
	UI_PushMenu( Menu_Video_Advanced_Draw, Video_Advanced_MenuKey );
}
