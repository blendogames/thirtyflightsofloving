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

// r_main.c

#include "r_local.h"
#include "vlights.h"

void R_Clear (void);

viddef_t	vid;

model_t		*r_worldmodel;

float		gldepthmin, gldepthmax;

glconfig_t	glConfig;
glstate_t	glState;
glmedia_t	glMedia;

entity_t	*currententity;
int			r_worldframe; // added for trans animations
model_t		*currentmodel;

cplane_t	frustum[4];

int			r_visframecount;	// bumped when going to a new PVS
int			r_framecount;		// used for dlight push checking

int			c_brush_calls, c_brush_surfs, c_brush_polys, c_alias_polys, c_part_polys;

float		v_blend[4];			// final blending color

int			maxsize;			// Nexus

void GL_Strings_f( void );

//
// view origin
//
vec3_t	vup;
vec3_t	vpn;
vec3_t	vright;
vec3_t	r_origin;

float	r_world_matrix[16];
float	r_base_world_matrix[16];
vec4_t	r_clearColor = {0, 0.5, 0.5, 0.5};				// for gl_clear

GLdouble	r_farz;	// Knightmare- variable sky range, made this a global var

//
// screen size info
//
refdef_t	r_newrefdef;

int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

cvar_t	*gl_allow_software;
cvar_t  *gl_driver;
cvar_t	*gl_clear;

cvar_t	*con_font; // Psychospaz's console font size option
cvar_t	*con_font_size;
cvar_t	*alt_text_color;
cvar_t	*scr_netgraph_pos;

cvar_t	*r_norefresh;
cvar_t	*r_drawentities;
cvar_t	*r_drawworld;
cvar_t	*r_speeds;
cvar_t	*r_fullbright;
cvar_t	*r_novis;
cvar_t	*r_nocull;
cvar_t	*r_lerpmodels;
cvar_t	*r_ignorehwgamma; // hardware gamma
cvar_t	*r_displayrefresh; // refresh rate control
cvar_t	*r_lefthand;
cvar_t	*r_waterwave;	// water waves
cvar_t  *r_caustics;	// Barnes water caustics
cvar_t  *r_glows;		// texture glows
cvar_t	*r_saveshotsize;//  save shot size option

cvar_t	*r_dlights_normal; // lerped dlights on models
cvar_t	*r_model_shading;
cvar_t	*r_model_dlights;
cvar_t	*r_model_minlight;
cvar_t	*r_entity_doublelight;

cvar_t	*r_lightlevel;	// FIXME: This is a HACK to get the client's light level

cvar_t	*r_rgbscale; // Vic's RGB brightening

//cvar_t	*r_nosubimage;	// unused
//cvar_t	*r_vertex_arrays;	// unused

//cvar_t	*r_ext_swapinterval;	// unused
cvar_t	*r_ext_multitexture;
cvar_t	*r_ext_draw_range_elements;
cvar_t	*r_ext_compiled_vertex_array;
cvar_t	*r_arb_texturenonpoweroftwo;	// Knightmare- non-power-of-two texture support
cvar_t	*r_nonpoweroftwo_mipmaps;		// Knightmare- non-power-of-two texture support
cvar_t	*r_sgis_generatemipmap;			// Knightmare- whether to use GL_SGIS_generate_mipmap
cvar_t	*r_newlightmapformat;			// Knightmare- whether to use new lightmap format
cvar_t	*r_ext_mtexcombine; // Vic's RGB brightening
cvar_t	*r_stencilTwoSide; // Echon's two-sided stenciling
cvar_t	*r_arb_fragment_program;
cvar_t	*r_arb_vertex_program;
cvar_t	*r_arb_vertex_buffer_object;
cvar_t	*r_pixel_shader_warp; // allow disabling the nVidia water warp
cvar_t	*r_trans_lighting; // disabling of lightmaps on trans surfaces
cvar_t	*r_warp_lighting; // allow disabling of lighting on warp surfaces
cvar_t	*r_warp_lighting_sample_offset; // allow adjustment of lighting sampling offset
cvar_t	*r_solidalpha;			// allow disabling of trans33+trans66 surface flag combining
cvar_t	*r_entity_fliproll;		// allow disabling of backwards alias model roll
cvar_t	*r_old_nullmodel;		// allow selection of nullmodel

cvar_t	*r_glass_envmaps; // Psychospaz's envmapping
//cvar_t	*r_trans_surf_sorting; // trans bmodel sorting
cvar_t	*r_shelltype; // entity shells: 0 = solid, 1 = warp, 2 = spheremap
cvar_t	*r_ext_texture_compression; // Heffo - ARB Texture Compression
cvar_t	*r_screenshot_format;		// determines screenshot format
//cvar_t	*r_screenshot_jpeg;			// Heffo - JPEG Screenshots
cvar_t	*r_screenshot_jpeg_quality;	// Heffo - JPEG Screenshots
cvar_t	*r_screenshot_gamma_correct;	// gamma correction for screenshots

//cvar_t	*r_motionblur;				// motionblur
cvar_t	*r_lightcutoff;	//** DMP - allow dynamic light cutoff to be user-settable

cvar_t	*r_log;
cvar_t	*r_bitdepth;
cvar_t	*r_drawbuffer;
cvar_t	*r_lightmap;
cvar_t	*r_shadows;
cvar_t	*r_shadowalpha;
cvar_t	*r_shadowrange;
cvar_t	*r_shadowvolumes;
cvar_t	*r_shadow_self;
cvar_t	*r_shadow_zfail;
cvar_t	*r_stencil;
cvar_t	*r_transrendersort; // correct trasparent sorting
cvar_t	*r_particle_lighting;	// particle lighting
cvar_t	*r_particle_min;
cvar_t	*r_particle_max;

cvar_t	*r_particledistance;
cvar_t	*r_particle_overdraw;

cvar_t	*r_mode;
cvar_t	*r_dynamic;
cvar_t  *r_monolightmap;

cvar_t	*r_modulate;
cvar_t	*r_nobind;
//cvar_t	*r_round_down;	// unused
cvar_t	*r_picmip;
cvar_t	*r_skymip;
//cvar_t	*r_playermip;	// unused
cvar_t	*r_showtris;
cvar_t	*r_showbbox;	// show model bounding box
cvar_t	*r_ztrick;
cvar_t	*r_finish;
cvar_t	*r_cull;
cvar_t	*r_polyblend;
cvar_t	*r_flashblend;
cvar_t  *r_saturatelighting;
cvar_t	*r_swapinterval;
cvar_t	*r_texturemode;
//cvar_t	*r_texturealphamode;
//cvar_t	*r_texturesolidmode;
cvar_t	*r_anisotropic;
cvar_t	*r_anisotropic_avail;
cvar_t	*r_font_upscale;
cvar_t	*r_scrap_upscale;
cvar_t	*r_nvfog_dist;
cvar_t	*r_nvfog_dist_mode;
cvar_t	*r_lockpvs;

cvar_t	*r_3dlabs_broken;

cvar_t	*vid_fullscreen;
cvar_t	*vid_gamma;
cvar_t	*vid_ref;

// Changable color for r_clearcolor (enabled by gl_clar)
cvar_t	*r_clearcolor_r;
cvar_t	*r_clearcolor_g;
cvar_t	*r_clearcolor_b;

cvar_t  *r_bloom;	// BLOOMS

// Discoloda's cel shading
cvar_t  *r_celshading;
cvar_t  *r_celshading_width;

cvar_t	*r_skydistance;		// variable sky range
cvar_t	*r_fog_skyratio;	// variable sky fog ratio
//cvar_t	*r_saturation;		//** DMP


/*
=================
R_CullBox

Returns true if the box is completely outside the frustom
=================
*/
qboolean R_CullBox (vec3_t mins, vec3_t maxs)
{
	int		i;

	if (r_nocull->integer)
		return false;

	for (i=0 ; i<4 ; i++)
		if ( BOX_ON_PLANE_SIDE(mins, maxs, &frustum[i]) == 2)
			return true;
	return false;
}


/*
============
R_PolyBlend
============
*/
void R_PolyBlend (void)
{
	if (!r_polyblend->integer)
		return;
	if (!v_blend[3])
		return;

	GL_Disable (GL_ALPHA_TEST);
	GL_Enable (GL_BLEND);
	GL_Disable (GL_DEPTH_TEST);
	GL_DisableTexture(0);

    qglLoadIdentity ();

	// FIXME: get rid of these
    qglRotatef (-90,  1, 0, 0);	    // put Z going up
    qglRotatef (90,  0, 0, 1);	    // put Z going up

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;
	VA_SetElem3(vertexArray[rb_vertex], 10, 100, 100);
	VA_SetElem4(colorArray[rb_vertex], v_blend[0], v_blend[1], v_blend[2], v_blend[3]);
	rb_vertex++;
	VA_SetElem3(vertexArray[rb_vertex], 10, -100, 100);
	VA_SetElem4(colorArray[rb_vertex], v_blend[0], v_blend[1], v_blend[2], v_blend[3]);
	rb_vertex++;
	VA_SetElem3(vertexArray[rb_vertex], 10, -100, -100);
	VA_SetElem4(colorArray[rb_vertex], v_blend[0], v_blend[1], v_blend[2], v_blend[3]);
	rb_vertex++;
	VA_SetElem3(vertexArray[rb_vertex], 10, 100, -100);
	VA_SetElem4(colorArray[rb_vertex], v_blend[0], v_blend[1], v_blend[2], v_blend[3]);
	rb_vertex++;
	RB_RenderMeshGeneric (false);

	GL_Disable (GL_BLEND);
	GL_EnableTexture(0);
	//GL_Enable (GL_ALPHA_TEST);

	qglColor4f (1,1,1,1);
}

//=======================================================================

int SignbitsForPlane (cplane_t *out)
{
	int	bits, j;

	// for fast box on planeside test

	bits = 0;
	for (j=0 ; j<3 ; j++)
	{
		if (out->normal[j] < 0)
			bits |= 1<<j;
	}
	return bits;
}


void R_SetFrustum (void)
{
	int		i;

#if 0
	//
	// this code is wrong, since it presume a 90 degree FOV both in the
	// horizontal and vertical plane
	//
	// front side is visible
	VectorAdd (vpn, vright, frustum[0].normal);
	VectorSubtract (vpn, vright, frustum[1].normal);
	VectorAdd (vpn, vup, frustum[2].normal);
	VectorSubtract (vpn, vup, frustum[3].normal);

	// we theoretically don't need to normalize these vectors, but I do it
	// anyway so that debugging is a little easier
	VectorNormalize( frustum[0].normal );
	VectorNormalize( frustum[1].normal );
	VectorNormalize( frustum[2].normal );
	VectorNormalize( frustum[3].normal );
#else
	// rotate VPN right by FOV_X/2 degrees
	RotatePointAroundVector( frustum[0].normal, vup, vpn, -(90-r_newrefdef.fov_x / 2 ) );
	// rotate VPN left by FOV_X/2 degrees
	RotatePointAroundVector( frustum[1].normal, vup, vpn, 90-r_newrefdef.fov_x / 2 );
	// rotate VPN up by FOV_X/2 degrees
	RotatePointAroundVector( frustum[2].normal, vright, vpn, 90-r_newrefdef.fov_y / 2 );
	// rotate VPN down by FOV_X/2 degrees
	RotatePointAroundVector( frustum[3].normal, vright, vpn, -( 90 - r_newrefdef.fov_y / 2 ) );
#endif

	for (i=0 ; i<4 ; i++)
	{
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct (r_origin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane (&frustum[i]);
	}
}

//=======================================================================

/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame (void)
{
	int i;
	mleaf_t	*leaf;

	r_framecount++;

// build the transformation matrix for the given view angles
	VectorCopy (r_newrefdef.vieworg, r_origin);

	AngleVectors (r_newrefdef.viewangles, vpn, vright, vup);

// current viewcluster
	if ( !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
	{
		r_oldviewcluster = r_viewcluster;
		r_oldviewcluster2 = r_viewcluster2;
		leaf = Mod_PointInLeaf (r_origin, r_worldmodel);
		r_viewcluster = r_viewcluster2 = leaf->cluster;

		// check above and below so crossing solid water doesn't draw wrong
		if (!leaf->contents)
		{	// look down a bit
			vec3_t	temp;

			VectorCopy (r_origin, temp);
			temp[2] -= 16;
			leaf = Mod_PointInLeaf (temp, r_worldmodel);
			if ( !(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2) )
				r_viewcluster2 = leaf->cluster;
		}
		else
		{	// look up a bit
			vec3_t	temp;

			VectorCopy (r_origin, temp);
			temp[2] += 16;
			leaf = Mod_PointInLeaf (temp, r_worldmodel);
			if ( !(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2) )
				r_viewcluster2 = leaf->cluster;
		}
	}

	for (i=0 ; i<4 ; i++)
		v_blend[i] = r_newrefdef.blend[i];

	c_brush_calls = 0;
	c_brush_surfs = 0;
	c_brush_polys = 0;
	c_alias_polys = 0;
	c_part_polys = 0;

	// clear out the portion of the screen that the NOWORLDMODEL defines
	/*if ( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
	{
		GL_Enable( GL_SCISSOR_TEST );
		qglClearColor( 0.3, 0.3, 0.3, 1 );
		qglScissor( r_newrefdef.x, vid.height - r_newrefdef.height - r_newrefdef.y, r_newrefdef.width, r_newrefdef.height );
		qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	//	qglClearColor( 1, 0, 0.5, 0.5 );
		qglClearColor (r_clearColor[0], r_clearColor[1], r_clearColor[2], r_clearColor[3]);
		GL_Disable( GL_SCISSOR_TEST );
	}*/
}


void MYgluPerspective( GLdouble fovy, GLdouble aspect,
		     GLdouble zNear, GLdouble zFar )
{
   GLdouble xmin, xmax, ymin, ymax;

   ymax = zNear * tan( fovy * M_PI / 360.0 );
   ymin = -ymax;

   xmin = ymin * aspect;
   xmax = ymax * aspect;

   xmin += -( 2 * glState.camera_separation ) / zNear;
   xmax += -( 2 * glState.camera_separation ) / zNear;

   qglFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}


/*
=============
R_SetupGL
=============
*/
void R_SetupGL (void)
{
	float	screenaspect;
//	float	yfov;
	int		x, x2, y2, y, w, h;
//	static	GLdouble farz;	// variable sky range
//	GLdouble boxsize;

	// Knightmare- update r_modulate in real time
    if (r_modulate->modified && (r_worldmodel)) //Don't do this if no map is loaded
	{
		msurface_t *surf; 
		int i;
		
        for (i=0,surf = r_worldmodel->surfaces; i<r_worldmodel->numsurfaces; i++,surf++)
            surf->cached_light[0]=0; 

        r_modulate->modified = 0; 
	} 

	//
	// set up viewport
	//
	x = floor(r_newrefdef.x * vid.width / vid.width);
	x2 = ceil((r_newrefdef.x + r_newrefdef.width) * vid.width / vid.width);
	y = floor(vid.height - r_newrefdef.y * vid.height / vid.height);
	y2 = ceil(vid.height - (r_newrefdef.y + r_newrefdef.height) * vid.height / vid.height);

	w = x2 - x;
	h = y - y2;

	qglViewport (x, y2, w, h);

	// Knightmare- variable sky range
	// calc farz falue from skybox size
	if (r_skydistance->modified)
	{
		GLdouble boxsize, farz;	// variable sky range

		r_skydistance->modified = false;
		boxsize = r_skydistance->value;
		boxsize -= 252 * ceil (boxsize / 2300);
		farz = 1.0;
		while (farz < boxsize) //make this a power of 2
		{
			farz *= 2.0;
			if (farz >= 65536) //don't make it larger than this
				break;
		}
		farz *= 2.0; //double since boxsize is distance from camera to edge of skybox
					//not total size of skybox
		VID_Printf(PRINT_DEVELOPER, "farz now set to %g\n", farz);
		r_farz = farz;	// save to global var
	}
	// end Knightmare

	//
	// set up projection matrix
	//
    screenaspect = (float)r_newrefdef.width/r_newrefdef.height;
//	yfov = 2*atan((float)r_newrefdef.height/r_newrefdef.width)*180/M_PI;
	qglMatrixMode(GL_PROJECTION);
    qglLoadIdentity ();

	//Knightmare- 12/26/2001- increase back clipping plane distance
    MYgluPerspective (r_newrefdef.fov_y,  screenaspect,  4, r_farz);	// was 4096
	//end Knightmare

	GL_CullFace(GL_FRONT);

	qglMatrixMode(GL_MODELVIEW);
    qglLoadIdentity ();

    qglRotatef (-90,  1, 0, 0);	    // put Z going up
    qglRotatef (90,  0, 0, 1);	    // put Z going up
    qglRotatef (-r_newrefdef.viewangles[2],  1, 0, 0);
    qglRotatef (-r_newrefdef.viewangles[0],  0, 1, 0);
    qglRotatef (-r_newrefdef.viewangles[1],  0, 0, 1);
    qglTranslatef (-r_newrefdef.vieworg[0],  -r_newrefdef.vieworg[1],  -r_newrefdef.vieworg[2]);

//	if ( glState.camera_separation != 0 && glState.stereo_enabled )
//		qglTranslatef ( glState.camera_separation, 0, 0 );

	qglGetFloatv (GL_MODELVIEW_MATRIX, r_world_matrix);

	//
	// set drawing parms
	//
	if (r_cull->integer)
		GL_Enable(GL_CULL_FACE);
	else
		GL_Disable(GL_CULL_FACE);

	GL_Disable(GL_BLEND);
	GL_Disable(GL_ALPHA_TEST);
	GL_Enable(GL_DEPTH_TEST);

	rb_vertex = rb_index = 0;
}


/*
=============
R_Clear
=============
*/
void R_Clear (void)
{
	GLbitfield	clearBits = 0;	// bitshifter's consolidation

	if (gl_clear->integer)
		clearBits |= GL_COLOR_BUFFER_BIT;

	if (r_ztrick->integer)
	{
		static int trickframe;

	//	if (gl_clear->integer)
	//		qglClear (GL_COLOR_BUFFER_BIT);

		trickframe++;
		if (trickframe & 1)
		{
			gldepthmin = 0;
			gldepthmax = 0.49999;
			GL_DepthFunc (GL_LEQUAL);
		}
		else
		{
			gldepthmin = 1;
			gldepthmax = 0.5;
			GL_DepthFunc (GL_GEQUAL);
		}
	}
	else
	{
	//	if (gl_clear->integer)
	//		qglClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//	else
	//		qglClear (GL_DEPTH_BUFFER_BIT);
		clearBits |= GL_DEPTH_BUFFER_BIT;

		gldepthmin = 0;
		gldepthmax = 1;
		GL_DepthFunc (GL_LEQUAL);
	}

	GL_DepthRange (gldepthmin, gldepthmax);

	// added stencil buffer
	if (glConfig.have_stencil)
	{
		if (r_shadows->integer == 3) // BeefQuake R6 shadows
			qglClearStencil(0);
		else
			qglClearStencil(1);
	//	qglClear(GL_STENCIL_BUFFER_BIT);
		clearBits |= GL_STENCIL_BUFFER_BIT;
	}
//	GL_DepthRange (gldepthmin, gldepthmax);

	if (clearBits)	// bitshifter's consolidation
		qglClear(clearBits);
}


/*
=============
R_Flash
=============
*/
void R_Flash (void)
{
	R_PolyBlend ();
}


/*
=============
R_DrawLastElements
=============
*/
void R_DrawLastElements (void)
{
	//if (parts_prerender)
	//	R_DrawParticles(parts_prerender);
	if (ents_trans)
		R_DrawEntitiesOnList(ents_trans);
}
//============================================


/*
================
R_RenderView

r_newrefdef must be set before the first call
================
*/
void R_RenderView (refdef_t *fd)
{
	if (r_norefresh->integer)
		return;

	r_newrefdef = *fd;

	if (!r_worldmodel && !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
		VID_Error (ERR_DROP, "R_RenderView: NULL worldmodel");

	if (r_speeds->integer)
	{
		c_brush_calls = 0;
		c_brush_surfs = 0;
		c_brush_polys = 0;
		c_alias_polys = 0;
		c_part_polys = 0;
	}

	R_PushDlights ();

	if (r_finish->integer)
		qglFinish ();

	R_SetupFrame ();

	R_SetFrustum ();

	R_SetupGL ();

	R_SetFog();

	R_MarkLeaves ();	// done here so we know if we're in water

	R_DrawWorld ();

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL) // options menu
	{
		R_SuspendFog ();

	//	R_DrawAllDecals ();
		R_DrawAllEntities( false);
		R_DrawAllParticles ();

		R_ResumeFog ();
	}
	else
	{
		GL_Disable (GL_ALPHA_TEST);

		R_RenderDlights();

		if (r_transrendersort->integer) {
		//	R_BuildParticleList ();
			R_SortParticlesOnList ();
			R_DrawAllDecals ();
		//	R_DrawAllEntityShadows ();
			R_DrawSolidEntities ();
			R_DrawEntitiesOnList (ents_trans);
		}
		else {
			R_DrawAllDecals ();
		//	R_DrawAllEntityShadows ();
			R_DrawAllEntities (true);
		}

		R_DrawAllParticles ();

		R_DrawEntitiesOnList(ents_viewweaps);

		R_ParticleStencil (1);
		R_DrawAlphaSurfaces ();
		R_ParticleStencil (2);

		R_ParticleStencil (3);
		if (r_particle_overdraw->integer) // redraw over alpha surfaces, those behind are occluded
			R_DrawAllParticles ();
		R_ParticleStencil (4);

		// always draw vwep last...
		R_DrawEntitiesOnList(ents_viewweaps_trans);

		R_BloomBlend (fd);	// BLOOMS

		R_Flash();
	}

	R_SetFog2D ();		// don't allow radial fogging of 2D elements
}


/*
================
R_SetGL2D
================
*/
void	Con_DrawString (int x, int y, char *string, int alpha);
float	SCR_ScaledScreen (float param);
#define	FONT_SIZE		SCR_ScaledScreen(con_font_size->value)

void R_SetGL2D (void)
{
	// set 2D virtual screen size
	qglViewport (0,0, vid.width, vid.height);
	qglMatrixMode(GL_PROJECTION);
    qglLoadIdentity ();
	qglOrtho  (0, vid.width, vid.height, 0, -99999, 99999);
	qglMatrixMode(GL_MODELVIEW);
    qglLoadIdentity ();
	GL_Disable (GL_DEPTH_TEST);
	GL_Disable (GL_CULL_FACE);
	GL_Disable (GL_BLEND);
	GL_Enable (GL_ALPHA_TEST);
	qglColor4f (1,1,1,1);

	// Knightmare- draw r_speeds (modified from Echon's tutorial)
	if (r_speeds->integer && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) // don't do this for options menu
	{
		char	S[128];
		int		lines, i, x, y, n = 0;

		lines = (glConfig.multitexture)?5:7;

		for (i=0; i<lines; i++)
		{
			switch (i) {
				case 0:	n = sprintf (S, S_COLOR_ALT S_COLOR_SHADOW"%5i wcall", c_brush_calls); break;
				case 1:	n = sprintf (S, S_COLOR_ALT S_COLOR_SHADOW"%5i wsurf", c_brush_surfs); break;
				case 2:	n = sprintf (S, S_COLOR_ALT S_COLOR_SHADOW"%5i wpoly", c_brush_polys); break;
				case 3: n = sprintf (S, S_COLOR_ALT S_COLOR_SHADOW"%5i epoly", c_alias_polys); break;
				case 4: n = sprintf (S, S_COLOR_ALT S_COLOR_SHADOW"%5i ppoly", c_part_polys); break;
				case 5: n = sprintf (S, S_COLOR_ALT S_COLOR_SHADOW"%5i tex  ", c_visible_textures); break;
				case 6: n = sprintf (S, S_COLOR_ALT S_COLOR_SHADOW"%5i lmaps", c_visible_lightmaps); break;
				default: break;
			}
			if (scr_netgraph_pos->integer)
				x = r_newrefdef.width - (n*FONT_SIZE + FONT_SIZE/2);
			else
				x = FONT_SIZE/2;
			y = r_newrefdef.height-(lines-i)*(FONT_SIZE+2);
			Con_DrawString (x, y, S, 255);
		}
	}
}


#if 0
static void GL_DrawColoredStereoLinePair (float r, float g, float b, float y)
{
	qglColor3f( r, g, b );
	qglVertex2f( 0, y );
	qglVertex2f( vid.width, y );
	qglColor3f( 0, 0, 0 );
	qglVertex2f( 0, y + 1 );
	qglVertex2f( vid.width, y + 1 );
}


static void GL_DrawStereoPattern (void)
{
	int i;

	if ( !glState.stereo_enabled )
		return;

	R_SetGL2D();

	qglDrawBuffer( GL_BACK_LEFT );

	for ( i = 0; i < 20; i++ )
	{
		qglBegin( GL_LINES );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 0 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 2 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 4 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 6 );
			GL_DrawColoredStereoLinePair( 0, 1, 0, 8 );
			GL_DrawColoredStereoLinePair( 1, 1, 0, 10);
			GL_DrawColoredStereoLinePair( 1, 1, 0, 12);
			GL_DrawColoredStereoLinePair( 0, 1, 0, 14);
		qglEnd();
		
		GLimp_EndFrame();
	}
}
#endif

/*
====================
R_SetLightLevel

====================
*/
void R_SetLightLevel (void)
{
	vec3_t		shadelight;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// save off light value for server to look at (BIG HACK!)

	R_LightPoint (r_newrefdef.vieworg, shadelight, false);//true);

	// pick the greatest component, which should be the same
	// as the mono value returned by software
	if (shadelight[0] > shadelight[1])
	{
		if (shadelight[0] > shadelight[2]) {
		//	r_lightlevel->value = 150*shadelight[0];
			Cvar_SetValue ("r_lightlevel", 150.0f*shadelight[0]);
		}
		else {
		//	r_lightlevel->value = 150*shadelight[2];
			Cvar_SetValue ("r_lightlevel", 150.0f*shadelight[2]);
		}
	}
	else
	{
		if (shadelight[1] > shadelight[2]) {
		//	r_lightlevel->value = 150*shadelight[1];
			Cvar_SetValue ("r_lightlevel", 150.0f*shadelight[1]);
		}
		else {
		//	r_lightlevel->value = 150*shadelight[2];
			Cvar_SetValue ("r_lightlevel", 150.0f*shadelight[2]);
		}
	}

}


/*
@@@@@@@@@@@@@@@@@@@@@
R_RenderFrame

@@@@@@@@@@@@@@@@@@@@@
*/
void R_RenderFrame (refdef_t *fd)
{
	R_RenderView( fd );
	R_SetLightLevel ();
	R_SetGL2D ();
}


void AssertCvarRange (cvar_t *var, float min, float max, qboolean isInteger)
{
	if (!var)
		return;

	if (isInteger && ((int)var->value != var->integer))
	{
		VID_Printf (PRINT_ALL, S_COLOR_YELLOW"Warning: cvar '%s' must be an integer (%f)\n", var->name, var->value);
		Cvar_Set (var->name, va("%d", var->integer));
	}

	if (var->value < min)
	{
		VID_Printf (PRINT_ALL, S_COLOR_YELLOW"Warning: cvar '%s' is out of range (%f < %f)\n", var->name, var->value, min);
		Cvar_Set (var->name, va("%f", min));
	}
	else if (var->value > max)
	{
		VID_Printf (PRINT_ALL, S_COLOR_YELLOW"Warning: cvar '%s' is out of range (%f > %f)\n", var->name, var->value, max);
		Cvar_Set (var->name, va("%f", max));
	}
}


void R_Register (void)
{
	// added Psychospaz's console font size option
	con_font = Cvar_Get ("con_font", "default", CVAR_ARCHIVE);
	Cvar_SetDescription ("con_font", "Sets name of font image for console/HUD/menu text.");
	con_font_size = Cvar_Get ("con_font_size", "8", CVAR_ARCHIVE);
//	Cvar_SetDescription ("con_font_size", "Sets size of console font.  Values > 8 are larger than default, < 8 are smaller.");
	alt_text_color = Cvar_Get ("alt_text_color", "2", CVAR_ARCHIVE);
//	Cvar_SetDescription ("alt_text_color", "Sets color of high-bit highlighted text.");

	scr_netgraph_pos = Cvar_Get ("netgraph_pos", "0", CVAR_ARCHIVE);

	gl_driver = Cvar_Get( "gl_driver", "opengl32", CVAR_ARCHIVE );
	Cvar_SetDescription ("gl_driver", "Sets driver for OpenGL.  This should stay as \"opengl32\".");
	gl_allow_software = Cvar_Get( "gl_allow_software", "0", 0 );
	Cvar_SetDescription ("gl_allow_software", "Whether to allow software implementations of OpenGL.");
	gl_clear = Cvar_Get ("gl_clear", "0", 0);
	Cvar_SetDescription ("gl_clear", "Enables clearing of the screen to prevent HOM effects.");

	r_lefthand = Cvar_Get( "hand", "0", CVAR_USERINFO | CVAR_ARCHIVE );
	r_norefresh = Cvar_Get ("r_norefresh", "0", CVAR_CHEAT);
	Cvar_SetDescription ("r_norefresh", "Disables rendering of viewport when set to 1.");
	r_fullbright = Cvar_Get ("r_fullbright", "0", CVAR_CHEAT);
	Cvar_SetDescription ("r_fullbright", "Enables fullbright rendering (no lighting).");
	r_drawentities = Cvar_Get ("r_drawentities", "1", 0);
	Cvar_SetDescription ("r_drawentities", "Enables drawing of entities.");
	r_drawworld = Cvar_Get ("r_drawworld", "1", CVAR_CHEAT);
	Cvar_SetDescription ("r_drawworld", "Enables drawing of the world.");
	r_novis = Cvar_Get ("r_novis", "0", CVAR_CHEAT);
	Cvar_SetDescription ("r_novis", "Disables use of VIS tables when set to 1.");
	r_nocull = Cvar_Get ("r_nocull", "0", CVAR_CHEAT);
	Cvar_SetDescription ("r_nocull", "Disables culling of nodes and bmodels when set to 1.");
	r_lerpmodels = Cvar_Get ("r_lerpmodels", "1", 0);
	Cvar_SetDescription ("r_lerpmodels", "Enables frame interpolation of models.");
	r_ignorehwgamma = Cvar_Get ("r_ignorehwgamma", "0", CVAR_ARCHIVE);	// hardware gamma
	Cvar_SetDescription ("r_ignorehwgamma", "Disables hardware gamma control when set to 1.");
	r_displayrefresh = Cvar_Get ("r_displayrefresh", "0", CVAR_ARCHIVE); // refresh rate control
	Cvar_SetDescription ("r_displayrefresh", "Sets display refresh rate.");
	AssertCvarRange (r_displayrefresh, 0, 240, true);
	r_speeds = Cvar_Get ("r_speeds", "0", 0);
	Cvar_SetDescription ("r_speeds", "Enables output of rendering surface/polygon total for surfaces, entities, and particles.");

	// lerped dlights on models
	r_dlights_normal = Cvar_Get("r_dlights_normal", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("r_dlights_normal", "Enables surface clipping of dynamic lights.");
	r_model_shading = Cvar_Get( "r_model_shading", "2", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_model_shading", "Enables shading on alias models.  Accepted values are 0, 1, 2, or 3.");
	r_model_dlights = Cvar_Get( "r_model_dlights", "8", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_model_dlights", "Sets maximum number of dynamic lights for a model.");
	r_model_minlight = Cvar_Get ("r_model_minlight", "0.02", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_model_minlight", "Sets minimum light level for alias model rendering.");
	r_entity_doublelight  = Cvar_Get ("r_entity_doublelight", "0", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_entity_doublelight", "Enables original glitchy double-lighting of models.  This is a sort-of-exploit, but is not considered a cheat.");

	r_lightlevel = Cvar_Get ("r_lightlevel", "0", 0);
	Cvar_SetDescription ("r_lightlevel", "Hack to send player's light level to server.  This is overwritten every render frame.");
	r_rgbscale = Cvar_Get ("r_rgbscale", "2", CVAR_ARCHIVE);	// Vic's RGB brightening
	Cvar_SetDescription ("r_rgbscale", "Sets RGB scaling factor.  Accepted values are 1,  2, or 4.");

	r_waterwave = Cvar_Get ("r_waterwave", "0", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_waterwave", "Sets amplitude of water waves.");
	r_caustics = Cvar_Get ("r_caustics", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_caustics", "Enables rendering of underwater caustic effect.  0 = disabled, 1 = normal, 2 = fragment warp.");
	r_glows = Cvar_Get ("r_glows", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_glows", "Enables rendering of texture glow layers.");
	r_saveshotsize = Cvar_Get ("r_saveshotsize", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_saveshotsize", "Enables saveshot resolutions above 256x256.");
//	r_nosubimage = Cvar_Get( "r_nosubimage", "0", 0 );	// unused

	// correct trasparent sorting
	r_transrendersort = Cvar_Get ("r_transrendersort", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_transrendersort", "Enables sorting of transparent entities and particles.");
	r_particle_lighting = Cvar_Get ("r_particle_lighting", "1.0", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_particle_lighting", "Lighting scale for particles.");
	r_particledistance = Cvar_Get ("r_particledistance", "0", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_particledistance", "Sets maximum distance for rendering particle effects. 0 = no limit.");
	r_particle_overdraw = Cvar_Get ("r_particle_overdraw", "0", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_particle_overdraw", "Enables redrawing of particles over translucent surfaces.");
	r_particle_min = Cvar_Get ("r_particle_min", "0", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_particle_min", "Sets minimum clipping distance for particle rendering.");
	r_particle_max = Cvar_Get ("r_particle_max", "0", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_particle_max", "Sets maximum clipping distance for particle rendering.");

	r_modulate = Cvar_Get ("r_modulate", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_modulate", "Sets brightness scale for lightmaps.");
	r_log = Cvar_Get( "r_log", "0", 0 );
	Cvar_SetDescription ("r_log", "Enables logging of OpenGL API calls.");
	r_bitdepth = Cvar_Get( "r_bitdepth", "0", 0 );
	Cvar_SetDescription ("r_bitdepth", "Sets color bit depth.  0 = desktop setting.");
	r_mode = Cvar_Get( "r_mode", "4", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_mode", "Sets enumerated video mode for renderer.  -1 = custom mode.");
	r_lightmap = Cvar_Get ("r_lightmap", "0", 0);
	Cvar_SetDescription ("r_lightmap", "Enables lightmap-only world rendering.");
	r_shadows = Cvar_Get ("r_shadows", "0", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_shadows", "Enables rendering of shadows.  0 = none, 1 = planar, 2 = dynamic planar, 3 = projection.");
	r_shadowalpha = Cvar_Get ("r_shadowalpha", "0.4", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_shadowalpha", "Sets opacity of shadows.");
	r_shadowrange  = Cvar_Get ("r_shadowrange", "768", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_shadowrange", "Sets maximum range for shadow rendering.");
	r_shadowvolumes = Cvar_Get ("r_shadowvolumes", "0", CVAR_CHEAT );
	Cvar_SetDescription ("r_shadowvolumes", "Enables rendering of shadow volumes for debugging.");
	r_shadow_self = Cvar_Get ("r_shadow_self", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_shadow_self", "Enables self-shadowing of models.");
	r_shadow_zfail = Cvar_Get ("r_shadow_zfail", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_shadow_zfail", "Enables z-fail shadow rendering method.  0 = z-pass shadows.");
	r_stencil = Cvar_Get ("r_stencil", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_stencil", "Enables use of stencil buffer.");

	r_dynamic = Cvar_Get ("r_dynamic", "1", 0);
	Cvar_SetDescription ("r_dynamic", "Enables dynamic lights.");
	r_nobind = Cvar_Get ("r_nobind", "0", CVAR_CHEAT);
	Cvar_SetDescription ("r_nobind", "Disables binding of textures for performance testing.");
//	r_round_down = Cvar_Get ("r_round_down", "1", 0);	// unused
	r_picmip = Cvar_Get ("r_picmip", "0", 0);
	Cvar_SetDescription ("r_picmip", "Sets maximum size for textures.  0 = no limit, 1 = 1024, 2 = 512, 3 = 256, 4 = 128.");
	r_skymip = Cvar_Get ("r_skymip", "0", 0);
	Cvar_SetDescription ("r_skymip", "Enables scaling down of skybox textures by a factor of 2.");
	r_showtris = Cvar_Get ("r_showtris", "0", CVAR_CHEAT);
	Cvar_SetDescription ("r_showtris", "Enables drawing of triangle outlines.  0 = disabled, 1 = no depth testing, 2 = depth testing enabled.");
	r_showbbox = Cvar_Get ("r_showbbox", "0", CVAR_CHEAT); // show model bounding box
	Cvar_SetDescription ("r_showbbox", "Enables drawing of entity culling bounding boxes.");
	r_ztrick = Cvar_Get ("r_ztrick", "0", 0);
	Cvar_SetDescription ("r_ztrick", "Enables the skipping of clearing the depth buffer before each render frame.");
	r_finish = Cvar_Get ("r_finish", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("r_finish", "Enables the delaying of view rendering until all buffered GL operations are complete.");
	r_cull = Cvar_Get ("r_cull", "1", 0);
	Cvar_SetDescription ("r_cull", "Enables backface culling.");
	r_polyblend = Cvar_Get ("r_polyblend", "1", 0);
	Cvar_SetDescription ("r_polyblend", "Enables view blending.");
	r_flashblend = Cvar_Get ("r_flashblend", "0", 0);
	Cvar_SetDescription ("r_flashblend", "Enables drawing of dynamic lights as glows instead of dynamicly lighting surfaces.");
//	r_playermip = Cvar_Get ("r_playermip", "0", 0);	// unused
	r_monolightmap = Cvar_Get( "r_monolightmap", "0", 0 );
	Cvar_SetDescription ("r_monolightmap", "Enables monochrome lightmaps.");

	r_anisotropic = Cvar_Get( "r_anisotropic", "0", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_anisotropic", "Sets level of anisotropic filtering.");
	r_anisotropic_avail = Cvar_Get( "r_anisotropic_avail", "0", 0 );
	Cvar_SetDescription ("r_anisotropic_avail", "The level of anisotropic filtering supported by the GL driver.");

	r_font_upscale = Cvar_Get ("r_font_upscale", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("r_font_upscale", "Enables upscaling of 128x128 fonts.");
	r_scrap_upscale = Cvar_Get ("r_scrap_upscale", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("r_scrap_upscale", "Enables upscaling of the atlassed scrap image.");

	r_nvfog_dist = Cvar_Get( "r_nvfog_dist", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_nvfog_dist", "Enables use of nVidia distance fog.");
	r_nvfog_dist_mode = Cvar_Get( "r_nvfog_dist_mode", "GL_EYE_RADIAL_NV", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_nvfog_dist_mode", "Sets nVidia distance fog mode.  Accepted values are GL_EYE_PLANE_ABSOLUTE_NV, GL_EYE_PLANE, and GL_EYE_RADIAL_NV.");

	// changed default texture mode to bilinear
	r_texturemode = Cvar_Get( "r_texturemode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_texturemode", "Sets texture filtering mode.  Accepted values are GL_LINEAR_MIPMAP_NEAREST and GL_LINEAR_MIPMAP_LINEAR.");
//	r_texturealphamode = Cvar_Get( "r_texturealphamode", "default", CVAR_ARCHIVE );
//	r_texturesolidmode = Cvar_Get( "r_texturesolidmode", "default", CVAR_ARCHIVE );
	r_lockpvs = Cvar_Get( "r_lockpvs", "0", 0 );
	Cvar_SetDescription ("r_lockpvs", "Enables locking of PVS to current viewpoint for testing map VIS.");

//	r_vertex_arrays = Cvar_Get( "r_vertex_arrays", "1", CVAR_ARCHIVE );	// unused

//	gl_ext_palettedtexture = Cvar_Get( "gl_ext_palettedtexture", "0", CVAR_ARCHIVE );
//	gl_ext_pointparameters = Cvar_Get( "gl_ext_pointparameters", "1", CVAR_ARCHIVE );
//	r_ext_swapinterval = Cvar_Get( "r_ext_swapinterval", "1", CVAR_ARCHIVE );	// unused
	r_ext_multitexture = Cvar_Get( "r_ext_multitexture", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_ext_multitexture", "Enables multitexture rendering of lightmapped surfaces.");
	r_ext_draw_range_elements = Cvar_Get("r_ext_draw_range_elements", "1", CVAR_ARCHIVE /*| CVAR_LATCH*/);
	Cvar_SetDescription ("r_ext_draw_range_elements", "Enables use of glDrawRangeElements().");
	r_ext_compiled_vertex_array = Cvar_Get( "r_ext_compiled_vertex_array", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_ext_compiled_vertex_array", "Enables use of compiled vertex arrays.");
	r_arb_texturenonpoweroftwo  = Cvar_Get("r_arb_texturenonpoweroftwo", "1", CVAR_ARCHIVE /*| CVAR_LATCH*/);
	Cvar_SetDescription ("r_arb_texturenonpoweroftwo", "Enables support for non-power-of-2 textures.");
	r_nonpoweroftwo_mipmaps = Cvar_Get("r_nonpoweroftwo_mipmaps", "1", CVAR_ARCHIVE /*| CVAR_LATCH*/);
	Cvar_SetDescription ("r_nonpoweroftwo_mipmaps", "Enables support for non-power-of-2 mipmaps.  This is not correctly supported by some drivers and may produce incorrect mipmaps.");
	r_sgis_generatemipmap = Cvar_Get("r_sgis_generatemipmap", "1", CVAR_ARCHIVE /*| CVAR_LATCH*/);
	Cvar_SetDescription ("r_sgis_generatemipmap", "Enables driver-based mipmap generation.  Set to 0 if you encounter corrupt or missing mipmaps.");

	r_newlightmapformat = Cvar_Get("r_newlightmapformat", "1", CVAR_ARCHIVE);	// whether to use new lightmap format
	Cvar_SetDescription ("r_newlightmapformat", "Enables modern, more efficient BGRA format for lightmaps.");

	// added Vic's RGB brightening
	r_ext_mtexcombine = Cvar_Get ("r_ext_mtexcombine", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("r_ext_mtexcombine", "Enables OpenGL register combiners.");

	// Echon's two-sided stenciling
	r_stencilTwoSide = Cvar_Get ("r_stencilTwoSide", "0", CVAR_ARCHIVE);
	Cvar_SetDescription ("r_stencilTwoSide", "Enables two-sided stenciling for projection shadows.");

	r_arb_fragment_program = Cvar_Get ("r_arb_fragment_program", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("r_arb_fragment_program", "Enables use of ARB fragment programs.");
	r_arb_vertex_program = Cvar_Get ("r_arb_vertex_program", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("r_arb_vertex_program", "Enables use of ARB vertex programs.");

	r_arb_vertex_buffer_object = Cvar_Get ("r_arb_vertex_buffer_object", "1", CVAR_ARCHIVE);
	Cvar_SetDescription ("r_arb_vertex_buffer_object", "Enables use of ARB vertex buffer objects.");

	// allow disabling the nVidia water warp
	r_pixel_shader_warp = Cvar_Get( "r_pixel_shader_warp", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_pixel_shader_warp", "Enables pixel shader water warp effect.");

	// allow disabling of lightmaps on trans surfaces
	r_trans_lighting = Cvar_Get( "r_trans_lighting", "2", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_trans_lighting", "Enables lighting on translucent surfaces. 0 = disabled, 1 = vertex lighting, 2 = lightmap.");

	// allow disabling of lighting on warp surfaces
	r_warp_lighting = Cvar_Get( "r_warp_lighting", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_warp_lighting", "Enables lighting on warp/water surfaces. 0 = disabled, 1 = vertex lighting.");

	// allow adjustment of lighting sampling offset
	r_warp_lighting_sample_offset = Cvar_Get( "r_warp_lighting_sample_offset", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_warp_lighting_sample_offset", "Sets offset sampling distance for vertex light sampling.");

	// allow disabling of trans33+trans66 surface flag combining
	r_solidalpha = Cvar_Get( "r_solidalpha", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_solidalpha", "Enables rendering of solid alphas (1.0f) when both trans33 and trans66 flags are set.");
	
	// allow disabling of backwards alias model roll
	r_entity_fliproll = Cvar_Get( "r_entity_fliproll", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_entity_fliproll", "Enables reversing of backwards entity roll.");

	// allow selection of nullmodel
	r_old_nullmodel = Cvar_Get( "r_old_nullmodel", "0", CVAR_ARCHIVE );	
	Cvar_SetDescription ("r_old_nullmodel", "Enables reversing of backwards entity roll.");

	// added Psychospaz's envmapping
	r_glass_envmaps = Cvar_Get( "r_glass_envmaps", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_glass_envmaps", "Enables environment maps on transparent surfaces.");
//	r_trans_surf_sorting = Cvar_Get( "r_trans_surf_sorting", "0", CVAR_ARCHIVE );	// unused

	r_shelltype = Cvar_Get( "r_shelltype", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_shelltype", "Sets render type for entity color shells.  0 = solid, 1 = flowing, 2 = envmap.");

	r_ext_texture_compression = Cvar_Get( "r_ext_texture_compression", "0", CVAR_ARCHIVE ); // Heffo - ARB Texture Compression
	Cvar_SetDescription ("r_ext_texture_compression", "Enables ARB texure compression.");

	r_screenshot_format = Cvar_Get( "r_screenshot_format", "jpg", CVAR_ARCHIVE );			// determines screenshot format
	Cvar_SetDescription ("r_screenshot_format", "Sets the image format for screenshots.  Accepted values are tga, jpg, and png.");
//	r_screenshot_jpeg = Cvar_Get( "r_screenshot_jpeg", "1", CVAR_ARCHIVE );					// Heffo - JPEG Screenshots
	r_screenshot_jpeg_quality = Cvar_Get( "r_screenshot_jpeg_quality", "85", CVAR_ARCHIVE );	// Heffo - JPEG Screenshots
	Cvar_SetDescription ("r_screenshot_jpeg_quality", "Sets the image quality for JPEG screenshots.  Accepted values are 1-100.");
	r_screenshot_gamma_correct = Cvar_Get( "r_screenshot_gamma_correct", "0", CVAR_ARCHIVE );	// gamma correction for screenshots
	Cvar_SetDescription ("r_screenshot_gamma_correct", "Enables gamma correction of screenshots.");

	//r_motionblur = Cvar_Get( "r_motionblur", "0", CVAR_ARCHIVE );	// motionblur

	r_drawbuffer = Cvar_Get( "r_drawbuffer", "GL_BACK", 0 );
	Cvar_SetDescription ("r_drawbuffer", "Sets draw buffer type.  Accepted values are GL_BACK and GL_FRONT.");
	r_swapinterval = Cvar_Get( "r_swapinterval", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_swapinterval", "Enables video sync.");

	r_saturatelighting = Cvar_Get( "r_saturatelighting", "0", 0 );
	Cvar_SetDescription ("r_saturatelighting", "Enables additive blend of lightmaps in single-texture rendering mode.");

	r_3dlabs_broken = Cvar_Get( "r_3dlabs_broken", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_3dlabs_broken", "Enables CDS hack for broken 3DLabs drivers.");

	vid_fullscreen = Cvar_Get( "vid_fullscreen", "1", CVAR_ARCHIVE );
	Cvar_SetDescription ("vid_fullscreen", "Enables fullscreen video mode.");
	vid_gamma = Cvar_Get( "vid_gamma", "0.8", CVAR_ARCHIVE ); // was 1.0
	Cvar_SetDescription ("vid_gamma", "Screen brightness value.  Uses inverse scale.");
	vid_ref = Cvar_Get( "vid_ref", "gl", CVAR_ARCHIVE );
	Cvar_SetDescription ("vid_ref", "Video renderer module in use.  This is always set to \"gl\" in KMQuake2.");

	// Changable color for r_clearcolor (enabled by gl_clar)
	r_clearcolor_r = Cvar_Get( "r_clearcolor_r", "0", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_clearcolor_r", "Sets red component (normalized) of background color used with gl_clear set to 1.  Accepted values are 0-1.");
	r_clearcolor_g = Cvar_Get( "r_clearcolor_g", "0.5", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_clearcolor_g", "Sets green component (normalized) of background color used with gl_clear set to 1.  Accepted values are 0-1.");
	r_clearcolor_b = Cvar_Get( "r_clearcolor_b", "0.5", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_clearcolor_b", "Sets blue component (normalized) of background color used with gl_clear set to 1.  Accepted values are 0-1.");

	r_bloom = Cvar_Get( "r_bloom", "0", CVAR_ARCHIVE );	// BLOOMS
	Cvar_SetDescription ("r_bloom", "Enables bloom postprocess effect.");

	r_celshading = Cvar_Get( "r_celshading", "0", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_celshading", "Enables cel shaded rendering of models.");
	r_celshading_width = Cvar_Get( "r_celshading_width", "4", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_celshading_width", "Sets width in pixels of cel shading outlines.");

	r_skydistance = Cvar_Get("r_skydistance", "24000", CVAR_ARCHIVE); // variable sky range
	Cvar_SetDescription ("r_skydistance", "Sets render distance of skybox.  Larger values mean a longer visible distance areas with the skybox visible.");
	r_fog_skyratio = Cvar_Get("r_fog_skyratio", "10", CVAR_ARCHIVE);	// variable sky fog ratio
	Cvar_SetDescription ("r_fog_skyratio", "Sets ratio of fog far distance for skyboxes versus standard world surfaces.");

//	r_saturation = Cvar_Get( "r_saturation", "1.0", CVAR_ARCHIVE );	//** DMP saturation setting (.89 good for nvidia)
	r_lightcutoff = Cvar_Get( "r_lightcutoff", "0", CVAR_ARCHIVE );	//** DMP dynamic light cutoffnow variable
	Cvar_SetDescription ("r_lightcutoff", "Sets cutoff distance for dynamic lights.  Lower = smoother.  Higher = faster.");

	Cmd_AddCommand ("imagelist", R_ImageList_f);
	Cmd_AddCommand ("screenshot", R_ScreenShot_f);
	Cmd_AddCommand ("screenshot_silent", R_ScreenShot_Silent_f);
	Cmd_AddCommand ("screenshot_tga", R_ScreenShot_TGA_f);
	Cmd_AddCommand ("screenshot_jpg", R_ScreenShot_JPG_f);
#ifdef PNG_SUPPORT
	Cmd_AddCommand ("screenshot_png", R_ScreenShot_PNG_f);
#endif	// PNG_SUPPORT
	Cmd_AddCommand ("modellist", Mod_Modellist_f);
	Cmd_AddCommand ("gl_strings", GL_Strings_f);
//	Cmd_AddCommand ("resetvertexlights", R_ResetVertextLights_f);
}


/*
==================
R_SetMode
==================
*/
qboolean R_SetMode (void)
{
	rserr_t err;
	qboolean fullscreen;

	if ( vid_fullscreen->modified && !glConfig.allowCDS )
	{
		VID_Printf (PRINT_ALL, "R_SetMode() - CDS not allowed with this driver\n" );
		Cvar_SetValue( "vid_fullscreen", !vid_fullscreen->integer );
		vid_fullscreen->modified = false;
	}

	fullscreen = vid_fullscreen->integer;
	r_skydistance->modified = true; // skybox size variable

	// don't allow modes 0, 1, or 2
	/*if ((r_mode->integer > -1) && (r_mode->integer < 3))
		Cvar_SetValue( "r_mode", 3);*/

	vid_fullscreen->modified = false;
	r_mode->modified = false;

	if ( ( err = GLimp_SetMode( &vid.width, &vid.height, r_mode->integer, fullscreen ) ) == rserr_ok )
	{
		glState.prev_mode = r_mode->integer;
	}
	else
	{
		if ( err == rserr_invalid_fullscreen )
		{
			Cvar_SetValue( "vid_fullscreen", 0);
			vid_fullscreen->modified = false;
			VID_Printf (PRINT_ALL, "ref_gl::R_SetMode() - fullscreen unavailable in this mode\n" );
			if ( ( err = GLimp_SetMode( &vid.width, &vid.height, r_mode->integer, false ) ) == rserr_ok )
				return true;
		}
		else if ( err == rserr_invalid_mode )
		{
			Cvar_SetValue( "r_mode", glState.prev_mode );
			r_mode->modified = false;
			VID_Printf (PRINT_ALL, "ref_gl::R_SetMode() - invalid mode\n" );
		}

		// try setting it back to something safe
		if ( ( err = GLimp_SetMode( &vid.width, &vid.height, glState.prev_mode, false ) ) != rserr_ok )
		{
			VID_Printf (PRINT_ALL, "ref_gl::R_SetMode() - could not revert to safe mode\n" );
			return false;
		}
	}
	return true;
}

#if 0	// replaced by Q_StrScanToken()
/*
===============
StringContainsToken

A non-ambiguous alternative to strstr.
Useful for parsing the GL extension string.
Similar to code in Fruitz of Dojo Quake2 MacOSX Port.
===============
*/
qboolean StringContainsToken (const char *string, const char *findToken)
{
	int			tokenLen;
	const char	*strPos;
	char		*tokPos, *terminatorPos;

	if ( !string || !findToken ) 
		return false;
	if ( (strchr(findToken, ' ') != NULL) || (findToken[0] == 0) )
		return false;

	strPos = string;
	tokenLen = (int)strlen(findToken);
	
	while (1)
	{
		tokPos = strstr (strPos, findToken);

		if ( !tokPos )
			break;

		terminatorPos = tokPos + tokenLen;

		if ( (tokPos == strPos || *(tokPos - 1) == ' ') && (*terminatorPos == ' ' || *terminatorPos == 0) )
			return true;

		strPos = terminatorPos;
	}

	return false;
}
#endif

/*
===============
R_CheckGLExtensions

Grabs GL extensions
===============
*/
qboolean R_CheckGLExtensions (char *reason)
{
	// OpenGL multitexture on GL 1.2.1 or later or GL_ARB_multitexture
	glConfig.multitexture = false;
	glConfig.max_texunits = 1;
	if ( (glConfig.version_major >= 2) || (glConfig.version_major == 1 && glConfig.version_minor > 2)
		|| (glConfig.version_major == 1 && glConfig.version_minor == 2 && glConfig.version_release >= 1) )
	{
		qglMultiTexCoord2fARB = (void *) qwglGetProcAddress( "glMultiTexCoord2f" );
		qglActiveTextureARB = (void *) qwglGetProcAddress( "glActiveTexture" );
		qglClientActiveTextureARB = (void *) qwglGetProcAddress( "glClientActiveTexture" );
		if (!qglMultiTexCoord2fARB || !qglActiveTextureARB || !qglClientActiveTextureARB)
		{
			VID_Printf (PRINT_ALL, "...OpenGL multitexture not found, checking for GL_ARB_multitexture\n" );
		}
		else {
			VID_Printf (PRINT_ALL, "...using OpenGL multitexture\n" );
			qglGetIntegerv(GL_MAX_TEXTURE_UNITS, &glConfig.max_texunits);
			VID_Printf (PRINT_ALL, "...GL_MAX_TEXTURE_UNITS: %i\n", glConfig.max_texunits);
			glConfig.multitexture = true;
		}
	}
	if ( (!glConfig.multitexture) && Q_StrScanToken( glConfig.extensions_string, "GL_ARB_multitexture", false ) )
	{
		if (r_ext_multitexture->integer)
		{
			qglMultiTexCoord2fARB = (void *) qwglGetProcAddress( "glMultiTexCoord2fARB" );
			qglActiveTextureARB = (void *) qwglGetProcAddress( "glActiveTextureARB" );
			qglClientActiveTextureARB = (void *) qwglGetProcAddress( "glClientActiveTextureARB" );
			if (!qglMultiTexCoord2fARB || !qglActiveTextureARB || !qglClientActiveTextureARB)
				VID_Printf (PRINT_ALL, "..." S_COLOR_RED "GL_ARB_multitexture not properly supported!\n"S_COLOR_YELLOW"WARNING: glow/caustic texture effects not enabled\n");
			else {
				VID_Printf (PRINT_ALL, "...using GL_ARB_multitexture\n" );
				qglGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &glConfig.max_texunits);
				VID_Printf (PRINT_ALL, "...GL_MAX_TEXTURE_UNITS_ARB: %i\n", glConfig.max_texunits);
				glConfig.multitexture = true;
			}
		}
		else
			VID_Printf (PRINT_ALL, "...ignoring GL_ARB_multitexture\n"S_COLOR_YELLOW"WARNING: glow/caustic texture effects not enabled\n" );
	}
	if (!glConfig.multitexture)
		VID_Printf (PRINT_ALL, "...GL_ARB_multitexture not found\n"S_COLOR_YELLOW"WARNING: glow/caustic texture effects not supported\n" );

	// GL_EXT_compiled_vertex_array
	// GL_SGI_compiled_vertex_array
	glConfig.extCompiledVertArray = false;
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_EXT_compiled_vertex_array", false ) || 
		 Q_StrScanToken( glConfig.extensions_string, "GL_SGI_compiled_vertex_array", false ) )
	{
		if (r_ext_compiled_vertex_array->integer)
		{
			qglLockArraysEXT = (void *) qwglGetProcAddress( "glLockArraysEXT" );
			qglUnlockArraysEXT = (void *) qwglGetProcAddress( "glUnlockArraysEXT" );
			if (!qglLockArraysEXT || !qglUnlockArraysEXT) {
				VID_Printf (PRINT_ALL, "..." S_COLOR_RED "GL_EXT/SGI_compiled_vertex_array not properly supported!\n");
				qglLockArraysEXT	= NULL;
				qglUnlockArraysEXT	= NULL;
			}
			else {
				VID_Printf (PRINT_ALL, "...enabling GL_EXT/SGI_compiled_vertex_array\n" );
				glConfig.extCompiledVertArray = true;
			}
		}
		else
			VID_Printf (PRINT_ALL, "...ignoring GL_EXT/SGI_compiled_vertex_array\n");
	}
	else
		VID_Printf (PRINT_ALL, "...GL_EXT/SGI_compiled_vertex_array not found\n" );

	// glDrawRangeElements on GL 1.2 or higher or GL_EXT_draw_range_elements
	glConfig.drawRangeElements = false;
	if ( (glConfig.version_major >= 2) || (glConfig.version_major == 1 && glConfig.version_minor >= 2) )
	{
		if (r_ext_draw_range_elements->integer)
		{
			qglDrawRangeElementsEXT = (void *) qwglGetProcAddress("glDrawRangeElements");
			if (!qglDrawRangeElementsEXT)
				qglDrawRangeElementsEXT = (void *) qwglGetProcAddress("glDrawRangeElementsEXT");
			if (!qglDrawRangeElementsEXT)
				VID_Printf (PRINT_ALL, "..." S_COLOR_RED "glDrawRangeElements not properly supported!\n");
			else {
				VID_Printf (PRINT_ALL, "...using glDrawRangeElements\n");
				glConfig.drawRangeElements = true;
			}
		}
		else
			VID_Printf (PRINT_ALL, "...ignoring glDrawRangeElements\n");
	}
	else if ( Q_StrScanToken( glConfig.extensions_string, "GL_EXT_draw_range_elements", false ) )
	{
		if (r_ext_draw_range_elements->integer)
		{
			qglDrawRangeElementsEXT = (void *) qwglGetProcAddress("glDrawRangeElementsEXT");
			if (!qglDrawRangeElementsEXT)
				qglDrawRangeElementsEXT = (void *) qwglGetProcAddress("glDrawRangeElements");
			if (!qglDrawRangeElementsEXT)
				VID_Printf (PRINT_ALL, "..." S_COLOR_RED "GL_EXT_draw_range_elements not properly supported!\n");
			else {
				VID_Printf (PRINT_ALL, "...enabling GL_EXT_draw_range_elements\n");
				glConfig.drawRangeElements = true;
			}
		}
		else
			VID_Printf (PRINT_ALL, "...ignoring GL_EXT_draw_range_elements\n");
	}
	else
		VID_Printf (PRINT_ALL, "...GL_EXT_draw_range_elements not found\n" );


	// GL_ARB_texture_non_power_of_two
	glConfig.arbTextureNonPowerOfTwo = false;
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_ARB_texture_non_power_of_two", false ) )
	{
		if (r_arb_texturenonpoweroftwo->integer) {
			VID_Printf (PRINT_ALL, "...using GL_ARB_texture_non_power_of_two\n");
			glConfig.arbTextureNonPowerOfTwo = true;
		}
		else {
			VID_Printf (PRINT_ALL, "...ignoring GL_ARB_texture_non_power_of_two\n");
		}

	}
	else
		VID_Printf (PRINT_ALL, "...GL_ARB_texture_non_power_of_two not found\n");

#ifdef _WIN32
	// WGL_EXT_swap_control
	if ( Q_StrScanToken( glConfig.extensions_string, "WGL_EXT_swap_control", false ) )
	{
		qwglSwapIntervalEXT = ( BOOL (WINAPI *)(int)) qwglGetProcAddress( "wglSwapIntervalEXT" );
		VID_Printf (PRINT_ALL, "...enabling WGL_EXT_swap_control\n" );
	}
	else
		VID_Printf (PRINT_ALL, "...WGL_EXT_swap_control not found\n" );
#endif


#if 0
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_EXT_point_parameters", false ) )
	{
		if ( gl_ext_pointparameters->integer )
		{
			qglPointParameterfEXT = ( void (APIENTRY *)( GLenum, GLfloat ) ) qwglGetProcAddress( "glPointParameterfEXT" );
			qglPointParameterfvEXT = ( void (APIENTRY *)( GLenum, const GLfloat * ) ) qwglGetProcAddress( "glPointParameterfvEXT" );
			VID_Printf (PRINT_ALL, "...using GL_EXT_point_parameters\n" );
		}
		else
			VID_Printf (PRINT_ALL, "...ignoring GL_EXT_point_parameters\n" );
	}
	else
		VID_Printf (PRINT_ALL, "...GL_EXT_point_parameters not found\n" );

	if ( !qglColorTableEXT &&
		Q_StrScanToken( glConfig.extensions_string, "GL_EXT_paletted_texture", false ) && 
		Q_StrScanToken( glConfig.extensions_string, "GL_EXT_shared_texture_palette", false ) )
	{
		if (gl_ext_palettedtexture->integer)
		{
			VID_Printf (PRINT_ALL, "...using GL_EXT_shared_texture_palette\n" );
			qglColorTableEXT = ( void ( APIENTRY * ) ( int, int, int, int, int, const void * ) ) qwglGetProcAddress( "glColorTableEXT" );
		}
		else
			VID_Printf (PRINT_ALL, "...ignoring GL_EXT_shared_texture_palette\n" );
	}
	else
		VID_Printf (PRINT_ALL, "...GL_EXT_shared_texture_palette not found\n" );
#endif

	// GL_ARB_vertex_buffer_object
	glConfig.vertexBufferObject = false;
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_ARB_vertex_buffer_object", false ) )
	{
		/*if (r_arb_vertex_buffer_object->integer)
		{
			VID_Printf(PRINT_ALL, "...using GL_ARB_vertex_buffer_object\n" );
			glConfig.vertexBufferObject = true;

			qglBindBufferARB = (void *) qwglGetProcAddress( "glBindBufferARB" );
			qglDeleteBuffersARB = (void *) qwglGetProcAddress( "glDeleteBuffersARB" );
			qglGenBuffersARB = (void *) qwglGetProcAddress( "glGenBuffersARB" );
			qglBufferDataARB = (void *) qwglGetProcAddress( "glBufferDataARB" );
			qglBufferSubDataARB = (void *) qwglGetProcAddress( "glBufferSubDataARB" );
			qglMapBufferARB = (void *) qwglGetProcAddress( "glMapBufferARB" );
			qglUnmapBufferARB = (void *) qwglGetProcAddress( "glUnmapBufferARB" );
		}
		else*/
			VID_Printf (PRINT_ALL, "...ignoring GL_ARB_vertex_buffer_object\n");
	}
	else
		VID_Printf (PRINT_ALL, "...GL_ARB_vertex_buffer_object not found\n" );

	// GL_ARB_texture_env_combine - Vic
	glConfig.mtexcombine = false;
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_ARB_texture_env_combine", false ) )
	{
		if (r_ext_mtexcombine->integer)
		{
			VID_Printf (PRINT_ALL, "...using GL_ARB_texture_env_combine\n");
			glConfig.mtexcombine = true;
		}
		else
			VID_Printf (PRINT_ALL, "..ignoring GL_ARB_texture_env_combine\n");
	}
	else
		VID_Printf (PRINT_ALL, "...GL_ARB_texture_env_combine not found\n");

#if 0
	// GL_EXT_texture_env_combine - Vic
	if (!glConfig.mtexcombine)
	{
		if ( Q_StrScanToken( glConfig.extensions_string, "GL_EXT_texture_env_combine", false ) )
		{
			if (r_ext_mtexcombine->integer)
			{
				VID_Printf (PRINT_ALL, "..using GL_EXT_texture_env_combine\n");
				glConfig.mtexcombine = true;
			}
			else
				VID_Printf (PRINT_ALL, "...ignoring GL_EXT_texture_env_combine\n");
		}
		else
			VID_Printf (PRINT_ALL, "...GL_EXT_texture_env_combine not found\n");
	}
#endif

	// GL_EXT_stencil_wrap
	glConfig.extStencilWrap = false;
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_EXT_stencil_wrap", false ) )
	{
		VID_Printf (PRINT_ALL, "...using GL_EXT_stencil_wrap\n");
		glConfig.extStencilWrap = true;
	}
	else
		VID_Printf (PRINT_ALL, "...GL_EXT_stencil_wrap not found\n");

	// GL_ATI_separate_stencil - Barnes
	glConfig.atiSeparateStencil = false;
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_ATI_separate_stencil", false ) )
	{
	//	if (r_stencilTwoSide->integer)
	//	{
			qglStencilOpSeparateATI = (void *) qwglGetProcAddress("glStencilOpSeparateATI");
			qglStencilFuncSeparateATI = (void *) qwglGetProcAddress("glStencilFuncSeparateATI");
			if (!qglStencilOpSeparateATI) {
				VID_Printf (PRINT_ALL, "..." S_COLOR_RED "GL_ATI_separate_stencil not properly supported!\n");
				qglStencilOpSeparateATI = NULL;
			}
			else {
				VID_Printf (PRINT_ALL, "...using GL_ATI_separate_stencil\n");
				glConfig.atiSeparateStencil = true;
			}
	//	}
	//	else
	//		VID_Printf (PRINT_ALL, "...ignoring GL_ATI_separate_stencil\n");
	}
	else
		VID_Printf (PRINT_ALL, "...GL_ATI_separate_stencil not found\n");

	// GL_EXT_stencil_two_side - Echon
	glConfig.extStencilTwoSide = false;
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_EXT_stencil_two_side", false ) )
	{
	//	if (r_stencilTwoSide->integer)
	//	{
			qglActiveStencilFaceEXT = (void *) qwglGetProcAddress( "glActiveStencilFaceEXT" );
			if (!qglActiveStencilFaceEXT) {
				VID_Printf (PRINT_ALL, "..." S_COLOR_RED "GL_EXT_stencil_two_side not properly supported!\n");
				qglActiveStencilFaceEXT = NULL;
			}
			else {
				VID_Printf (PRINT_ALL, "...using GL_EXT_stencil_two_side\n");
				glConfig.extStencilTwoSide = true;
			}
	//	}
	//	else
	//		VID_Printf (PRINT_ALL, "...ignoring GL_EXT_stencil_two_side\n");
	}
	else
		Com_Printf("...GL_EXT_stencil_two_side not found\n");

	// GL_ARB_fragment_program
	glConfig.arb_fragment_program = false;
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_ARB_fragment_program", false ) )
	{
		if (r_arb_fragment_program->integer)
		{
			qglProgramStringARB = (void *) qwglGetProcAddress( "glProgramStringARB" );
			qglBindProgramARB = (void *) qwglGetProcAddress( "glBindProgramARB" );
			qglDeleteProgramsARB = (void *) qwglGetProcAddress( "glDeleteProgramsARB" );
			qglGenProgramsARB = (void *) qwglGetProcAddress( "glGenProgramsARB" );
			qglProgramEnvParameter4dARB = (void *) qwglGetProcAddress( "glProgramEnvParameter4dARB" );
			qglProgramEnvParameter4dvARB = (void *) qwglGetProcAddress( "glProgramEnvParameter4dvARB" );
			qglProgramEnvParameter4fARB = (void *) qwglGetProcAddress( "glProgramEnvParameter4fARB" );
			qglProgramEnvParameter4fvARB = (void *) qwglGetProcAddress( "glProgramEnvParameter4fvARB" );
			qglProgramLocalParameter4dARB = (void *) qwglGetProcAddress( "glProgramLocalParameter4dARB" );
			qglProgramLocalParameter4dvARB = (void *) qwglGetProcAddress( "glProgramLocalParameter4dvARB" );
			qglProgramLocalParameter4fARB = (void *) qwglGetProcAddress( "glProgramLocalParameter4fARB" );
			qglProgramLocalParameter4fvARB = (void *) qwglGetProcAddress( "glProgramLocalParameter4fvARB" );
			qglGetProgramEnvParameterdvARB = (void *) qwglGetProcAddress( "glGetProgramEnvParameterdvARB" );
			qglGetProgramEnvParameterfvARB = (void *) qwglGetProcAddress( "glGetProgramEnvParameterfvARB" );
			qglGetProgramLocalParameterdvARB = (void *) qwglGetProcAddress( "glGetProgramLocalParameterdvARB" );
			qglGetProgramLocalParameterfvARB = (void *) qwglGetProcAddress( "glGetProgramLocalParameterfvARB" );
			qglGetProgramivARB = (void *) qwglGetProcAddress( "glGetProgramivARB" );
			qglGetProgramStringARB = (void *) qwglGetProcAddress( "glGetProgramStringARB" );
			qglIsProgramARB = (void *) qwglGetProcAddress( "glIsProgramARB" );
			if (!qglProgramStringARB || !qglBindProgramARB
				|| !qglDeleteProgramsARB || !qglGenProgramsARB
				|| !qglProgramEnvParameter4dARB || !qglProgramEnvParameter4dvARB
				|| !qglProgramEnvParameter4fARB || !qglProgramEnvParameter4fvARB
				|| !qglProgramLocalParameter4dARB || !qglProgramLocalParameter4dvARB
				|| !qglProgramLocalParameter4fARB || !qglProgramLocalParameter4fvARB
				|| !qglGetProgramEnvParameterdvARB || !qglGetProgramEnvParameterfvARB
				|| !qglGetProgramLocalParameterdvARB  || !qglGetProgramLocalParameterfvARB
				|| !qglGetProgramivARB || !qglGetProgramStringARB || !qglIsProgramARB)
				VID_Printf (PRINT_ALL, "..." S_COLOR_RED "GL_ARB_fragment_program not properly supported!\n");
			else {
				VID_Printf (PRINT_ALL, "...using GL_ARB_fragment_program\n");
				glConfig.arb_fragment_program = true;
			}
		}
		else
			VID_Printf (PRINT_ALL, "...ignoring GL_ARB_fragment_program\n");
	}
	else
		VID_Printf (PRINT_ALL, "...GL_ARB_fragment_program not found\n");

	// GL_ARB_vertex_program
	glConfig.arb_vertex_program = false;
	if (glConfig.arb_fragment_program)
	{
		if ( Q_StrScanToken( glConfig.extensions_string, "GL_ARB_vertex_program", false ) )
		{
			if (r_arb_vertex_program->integer)
			{
				qglGetVertexAttribdvARB = (void *) qwglGetProcAddress( "glGetVertexAttribdvARB" );
				qglGetVertexAttribfvARB	= (void *) qwglGetProcAddress( "glGetVertexAttribfvARB" );
				qglGetVertexAttribivARB	= (void *) qwglGetProcAddress( "glGetVertexAttribivARB" );
				qglGetVertexAttribPointervARB	= (void *) qwglGetProcAddress( "glGetVertexAttribPointervARB" );
				if (!qglGetVertexAttribdvARB || !qglGetVertexAttribfvARB
					|| !qglGetVertexAttribivARB || !qglGetVertexAttribPointervARB)
					VID_Printf (PRINT_ALL, "..." S_COLOR_RED "GL_ARB_vertex_program not properly supported!\n");
				else {
					VID_Printf (PRINT_ALL, "...using GL_ARB_vertex_program\n");
					glConfig.arb_vertex_program = true;
				}
			}
			else
				VID_Printf (PRINT_ALL, "...ignoring GL_ARB_vertex_program\n");
		}
		else
			VID_Printf (PRINT_ALL, "...GL_ARB_vertex_program not found\n");
	}

	R_Compile_ARB_Programs ();

	// GL_NV_texture_shader - MrG
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_NV_texture_shader", false ) )
	{
		VID_Printf (PRINT_ALL, "...using GL_NV_texture_shader\n" );
		glConfig.NV_texshaders = true;
	}
	else
	{
		VID_Printf (PRINT_ALL, "...GL_NV_texture_shader not found\n" );
		glConfig.NV_texshaders = false;
	}

	// GL_EXT_texture_filter_anisotropic - NeVo
	glConfig.anisotropic = false;
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_EXT_texture_filter_anisotropic", false ) )
	{
		VID_Printf (PRINT_ALL,"...using GL_EXT_texture_filter_anisotropic\n" );
		glConfig.anisotropic = true;
		qglGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &glConfig.max_anisotropy);
		Cvar_SetValue("r_anisotropic_avail", glConfig.max_anisotropy);
	}
	else
	{
		VID_Printf (PRINT_ALL,"..GL_EXT_texture_filter_anisotropic not found\n" );
		glConfig.anisotropic = false;
		glConfig.max_anisotropy = 0.0;
		Cvar_SetValue("r_anisotropic_avail", 0.0);
	} 

	// GL_NV_fog_distance
	glConfig.nvFogAvailable = false;
	glConfig.nvFogMode = 0;
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_NV_fog_distance", false ) )
	{
		if (r_nvfog_dist->integer)
		{
			glConfig.nvFogAvailable = true;
			VID_Printf (PRINT_ALL, "...using GL_NV_fog_distance\n");
		}
		else
		{
			VID_Printf (PRINT_ALL, "...ignoring GL_NV_fog_distance\n");
			Cvar_Set ("r_nvfog_dist", "0");
		}
	}
	else
	{
		VID_Printf (PRINT_ALL, "...GL_NV_fog_distance not found\n");
		Cvar_Set ("r_nvfog_dist", "0");
	}

	// GL_SGIS_generate_mipmap
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_SGIS_generate_mipmap", false ) )
	{
		if (r_sgis_generatemipmap->integer) {
			VID_Printf (PRINT_ALL, "...using GL_SGIS_generate_mipmap\n" );
			glState.sgis_mipmap = true;
		}
		else {
			VID_Printf (PRINT_ALL, "...ignoring GL_SGIS_generate_mipmap\n");
			glState.sgis_mipmap = false;
		}
	}
	else
	{
		VID_Printf (PRINT_ALL, "...GL_SGIS_generate_mipmap not found\n" );
		glState.sgis_mipmap = false;
	}

	// GL_ARB_texture_compression - Heffo
	if ( Q_StrScanToken( glConfig.extensions_string, "GL_ARB_texture_compression", false ) )
	{
		if(!r_ext_texture_compression->integer)
		{
			VID_Printf (PRINT_ALL, "...ignoring GL_ARB_texture_compression\n" );
			glState.texture_compression = false;
		}
		else
		{
			VID_Printf (PRINT_ALL, "...using GL_ARB_texture_compression\n" );
			glState.texture_compression = true;
		}
	}
	else
	{
		VID_Printf(PRINT_ALL, "...GL_ARB_texture_compression not found\n");
		glState.texture_compression = false;
		Cvar_Set("r_ext_texture_compression", "0");
	}

#ifdef _WIN32
	// WGL_3DFX_gamma_control
	if ( Q_StrScanToken(glConfig.extensions_string, "WGL_3DFX_gamma_control", false ) )
	{
		if (!r_ignorehwgamma->integer)
		{
			qwglGetDeviceGammaRamp3DFX	= ( BOOL (WINAPI *)(HDC, LPVOID)) qwglGetProcAddress( "wglGetDeviceGammaRamp3DFX" );
			qwglSetDeviceGammaRamp3DFX	= ( BOOL (WINAPI *)(HDC, LPVOID)) qwglGetProcAddress( "wglSetDeviceGammaRamp3DFX" );
			if (qwglGetDeviceGammaRamp3DFX && qwglSetDeviceGammaRamp3DFX)
				VID_Printf (PRINT_ALL, "...using WGL_3DFX_gamma_control\n" );
			else {
				qwglGetDeviceGammaRamp3DFX	= NULL;
				qwglSetDeviceGammaRamp3DFX	= NULL;
				VID_Printf (PRINT_ALL, "...failed to load WGL_3DFX_gamma_control\n" );
			}
		}
		else
			VID_Printf (PRINT_ALL, "...ignoring WGL_3DFX_gamma_control\n" );
	}
	else
		VID_Printf (PRINT_ALL, "...WGL_3DFX_gamma_control not found\n" );
#endif

	return true;
}


/*
===============
R_Init
===============
*/
qboolean R_Init ( void *hinstance, void *hWnd, char *reason )
{	
	char renderer_buffer[1000];
	char vendor_buffer[1000];
	int		err;
	int		j;
	extern float r_turbsin[256];

	for ( j = 0; j < 256; j++ )
	{
		r_turbsin[j] *= 0.5;
	}

	Draw_GetPalette ();
	R_Register();

	// place default error
	memcpy (reason, "Unknown failure on intialization!\0", 34);

#ifdef _WIN32
	// output system info
	VID_Printf (PRINT_ALL, "OS: %s\n", Cvar_VariableString("sys_osVersion"));
	VID_Printf (PRINT_ALL, "CPU: %s\n", Cvar_VariableString("sys_cpuString"));
	VID_Printf (PRINT_ALL, "RAM: %s MB\n", Cvar_VariableString("sys_ramMegs"));
#endif

	// initialize our QGL dynamic bindings
	if ( !QGL_Init( gl_driver->string ) )
	{
		QGL_Shutdown();
        VID_Printf (PRINT_ALL, "R_Init() - could not load \"%s\"\n", gl_driver->string );
		memcpy (reason, "Init of QGL dynamic bindings Failed!\0", 37);
		return false;
	}

	// initialize OS-specific parts of OpenGL
	if ( !GLimp_Init( hinstance, hWnd ) )
	{
		QGL_Shutdown();
		memcpy (reason, "Init of OS-specific parts of OpenGL Failed!\0", 44);
		return false;
	}

	// set our "safe" modes
	glState.prev_mode = 3;

	// create the window and set up the context
	if ( !R_SetMode () )
	{
		QGL_Shutdown();
        VID_Printf (PRINT_ALL, "R_Init() - could not R_SetMode()\n" );
		memcpy (reason, "Creation of the window/context set-up Failed!\0", 46);
		return false;
	}

	RB_InitBackend(); // init mini-backend


	//
	// get our various GL strings
	//
	glConfig.vendor_string = qglGetString (GL_VENDOR);
	VID_Printf (PRINT_ALL, "GL_VENDOR: %s\n", glConfig.vendor_string );
	glConfig.renderer_string = qglGetString (GL_RENDERER);
	VID_Printf (PRINT_ALL, "GL_RENDERER: %s\n", glConfig.renderer_string );
	glConfig.version_string = qglGetString (GL_VERSION);
	sscanf(glConfig.version_string, "%d.%d.%d", &glConfig.version_major, &glConfig.version_minor, &glConfig.version_release);
	VID_Printf (PRINT_ALL, "GL_VERSION: %s\n", glConfig.version_string );

	// Knighmare- added max texture size
	qglGetIntegerv(GL_MAX_TEXTURE_SIZE,&glConfig.max_texsize);
	VID_Printf (PRINT_ALL, "GL_MAX_TEXTURE_SIZE: %i\n", glConfig.max_texsize );

	glConfig.extensions_string = qglGetString (GL_EXTENSIONS);
//	VID_Printf (PRINT_DEVELOPER, "GL_EXTENSIONS: %s\n", glConfig.extensions_string );
	if (developer->integer > 0)	// print extensions 2 to a line
	{
		char		*extString, *extTok;
		unsigned	line = 0;
		VID_Printf (PRINT_DEVELOPER, "GL_EXTENSIONS: " );
		extString = (char *)glConfig.extensions_string;
		while (1)
		{
			extTok = COM_Parse(&extString);
			if (!extTok[0])
				break;
			line++;
			if ((line % 2) == 0)
				VID_Printf (PRINT_DEVELOPER, "%s\n", extTok );
			else
				VID_Printf (PRINT_DEVELOPER, "%s ", extTok );
		}
		if ((line % 2) != 0)
			VID_Printf (PRINT_DEVELOPER, "\n" );
	}

//	strncpy( renderer_buffer, glConfig.renderer_string );
	Q_strncpyz( renderer_buffer, glConfig.renderer_string, sizeof(renderer_buffer) );
	strlwr( renderer_buffer );

//	strncpy( vendor_buffer, glConfig.vendor_string );
	Q_strncpyz( vendor_buffer, glConfig.vendor_string, sizeof(vendor_buffer) );
	strlwr( vendor_buffer );

	// find out the renderer model
	if (strstr(vendor_buffer, "nvidia")) {
		glConfig.rendType = GLREND_NVIDIA;
		if (strstr(renderer_buffer, "geforce"))	glConfig.rendType |= GLREND_GEFORCE;
	}
	else if (strstr(vendor_buffer, "ati")) {
		glConfig.rendType = GLREND_ATI;
		if (strstr(vendor_buffer, "radeon"))		glConfig.rendType |= GLREND_RADEON;
	}
	else if (strstr(vendor_buffer, "matrox"))		glConfig.rendType = GLREND_MATROX;
	else if (strstr(vendor_buffer, "intel"))		glConfig.rendType = GLREND_INTEL;
	else if (strstr	(vendor_buffer, "sgi"))			glConfig.rendType = GLREND_SGI;
	else if (strstr	(renderer_buffer, "permedia"))	glConfig.rendType = GLREND_PERMEDIA2;
	else if (strstr	(renderer_buffer, "glint"))		glConfig.rendType = GLREND_GLINT_MX;
	else if (strstr	(renderer_buffer, "glzicd"))	glConfig.rendType = GLREND_REALIZM;
	else if (strstr	(renderer_buffer, "pcx1"))		glConfig.rendType = GLREND_PCX1;
	else if (strstr	(renderer_buffer, "pcx2"))		glConfig.rendType = GLREND_PCX2;
	else if (strstr	(renderer_buffer, "pmx"))		glConfig.rendType = GLREND_PMX;
	else if (strstr	(renderer_buffer, "verite"))	glConfig.rendType = GLREND_RENDITION;
	else if (strstr	(vendor_buffer, "sis"))			glConfig.rendType = GLREND_SIS;
	else if (strstr (renderer_buffer, "voodoo"))	glConfig.rendType = GLREND_VOODOO;
	else if (strstr	(renderer_buffer, "gdi generic")) glConfig.rendType = GLREND_MCD;
	else											glConfig.rendType = GLREND_DEFAULT;

	if ( toupper( r_monolightmap->string[1] ) != 'F' )
	{
		if (glConfig.rendType == GLREND_PERMEDIA2)
		{
			Cvar_Set( "r_monolightmap", "A" );
			VID_Printf (PRINT_ALL, "...using r_monolightmap 'a'\n" );
		}
		else
			Cvar_Set( "r_monolightmap", "0" );
	}

//	Cvar_Set( "scr_drawall", "0" );

#ifdef __linux__
	Cvar_SetValue( "r_finish", 1 );
#else
	Cvar_SetValue( "r_finish", 0 );
#endif
	r_swapinterval->modified = true;	// force swapinterval update

	// MCD has buffering issues
	if (glConfig.rendType == GLREND_MCD)
		Cvar_SetValue( "r_finish", 1 );

	if (glConfig.rendType & GLREND_3DLABS)
	{
		if ( r_3dlabs_broken->integer )
			glConfig.allowCDS = false;
		else
			glConfig.allowCDS = true;
	}
	else
		glConfig.allowCDS = true;

	if (glConfig.allowCDS)
		VID_Printf (PRINT_ALL, "...allowing CDS\n" );
	else
		VID_Printf (PRINT_ALL, "...disabling CDS\n" );

	// If using one of the mini-drivers, a Voodoo w/ WickedGL, or pre-1.2 driver,
	// use the texture formats determined by gl_texturesolidmode and gl_texturealphamode.
	if ( Q_stricmp(gl_driver->string, "opengl32") || glConfig.rendType == GLREND_VOODOO
		|| (glConfig.version_major < 2 && glConfig.version_minor < 2)
		|| (!r_newlightmapformat || !r_newlightmapformat->integer) )
	{
		VID_Printf (PRINT_ALL, "...using legacy lightmap format\n" );
		glConfig.newLMFormat = false;
	}
	else
	{
		VID_Printf (PRINT_ALL, "...using new lightmap format\n" );
		glConfig.newLMFormat = true;
	}

	//
	// grab extensions
	//
	if ( !R_CheckGLExtensions (reason) )
		return false;

/*
	Com_Printf( "Size of dlights: %i\n", sizeof (dlight_t)*MAX_DLIGHTS );
	Com_Printf( "Size of entities: %i\n", sizeof (entity_t)*MAX_ENTITIES );
	Com_Printf( "Size of particles: %i\n", sizeof (particle_t)*MAX_PARTICLES );
	Com_Printf( "Size of decals: %i\n", sizeof (particle_t)*MAX_DECAL_FRAGS );
*/

	// set r_clearColor
	r_clearColor[0] = min(max(r_clearcolor_r->value, 0.0f), 1.0f);
	r_clearColor[1] = min(max(r_clearcolor_g->value, 0.0f), 1.0f);
	r_clearColor[2] = min(max(r_clearcolor_b->value, 0.0f), 1.0f);

	GL_SetDefaultState();

	// draw our stereo patterns
#if 0 // commented out until H3D pays us the money they owe us
	GL_DrawStereoPattern();
#endif

	R_InitImages ();
	Mod_Init ();
	R_InitMedia ();
	R_DrawInitLocal ();

	R_InitDSTTex (); // init shader warp texture
	R_InitFogVars (); // reset fog variables
	VLight_Init (); // Vic's bmodel lights

	err = qglGetError();
	if ( err != GL_NO_ERROR )
		VID_Printf (PRINT_ALL, "glGetError() = 0x%x\n", err);

	return true;
}


/*
===============
R_ClearState
===============
*/
void R_ClearState (void)
{	
	R_SetFogVars (false, 0, 0, 0, 0, 0, 0, 0); // clear fog effets
	GL_EnableMultitexture (false);
	GL_SetDefaultState ();
}


/*
=================
GL_Strings_f
=================
*/
void GL_Strings_f (void)
{
	char		*extString, *extTok;
	unsigned	line = 0;

	VID_Printf (PRINT_ALL, "GL_VENDOR: %s\n", glConfig.vendor_string );
	VID_Printf (PRINT_ALL, "GL_RENDERER: %s\n", glConfig.renderer_string );
	VID_Printf (PRINT_ALL, "GL_VERSION: %s\n", glConfig.version_string );
	VID_Printf (PRINT_ALL, "GL_MAX_TEXTURE_SIZE: %i\n", glConfig.max_texsize );
//	VID_Printf (PRINT_ALL, "GL_EXTENSIONS: %s\n", glConfig.extensions_string );
	// print extensions 2 to a line
	VID_Printf (PRINT_ALL, "GL_EXTENSIONS: " );
	extString = (char *)glConfig.extensions_string;
	while (1)
	{
		extTok = COM_Parse(&extString);
		if (!extTok[0])
			break;
		line++;
		if ((line % 2) == 0)
			VID_Printf (PRINT_ALL, "%s\n", extTok );
		else
			VID_Printf (PRINT_ALL, "%s ", extTok );
	}
	if ((line % 2) != 0)
		VID_Printf (PRINT_ALL, "\n" );
}


/*
===============
R_Shutdown
===============
*/
void R_Shutdown (void)
{	
	Cmd_RemoveCommand ("modellist");
	Cmd_RemoveCommand ("screenshot");
	Cmd_RemoveCommand ("screenshot_silent");
	Cmd_RemoveCommand ("screenshot_tga");
	Cmd_RemoveCommand ("screenshot_jpg");
#ifdef PNG_SUPPORT
	Cmd_RemoveCommand ("screenshot_png");
#endif	// PNG_SUPPORT
	Cmd_RemoveCommand ("imagelist");
	Cmd_RemoveCommand ("gl_strings");
//	Cmd_RemoveCommand ("resetvertexlights");

	// Knightmare- Free saveshot buffer
	if (saveshotdata)
		free(saveshotdata);
	saveshotdata = NULL;	// make sure this is null after a vid restart!

	Mod_FreeAll ();

	R_ShutdownImages ();
//	R_ClearDisplayLists ();
	R_ShutdownMedia ();

	//
	// shut down OS specific OpenGL stuff like contexts, etc.
	//
	GLimp_Shutdown();

	//
	// shutdown our QGL subsystem
	//
	QGL_Shutdown();
}



/*
@@@@@@@@@@@@@@@@@@@@@
R_BeginFrame
@@@@@@@@@@@@@@@@@@@@@
*/
void UpdateGammaRamp (void); //Knightmare added
void RefreshFont (void);
void R_BeginFrame( float camera_separation )
{
	qboolean clearColor_modified = false;

	glState.camera_separation = camera_separation;

	// Knightmare- added Psychospaz's console font size option
	if (con_font->modified)
		RefreshFont ();

	if (con_font_size->modified)
	{
		if (con_font_size->integer < 4)
			Cvar_Set( "con_font_size", "4" );
		else if (con_font_size->integer > 24)
			Cvar_Set( "con_font_size", "24" );

		con_font_size->modified = false;
	}
	// end Knightmare

	//
	// change modes if necessary
	//
	if ( r_mode->modified || vid_fullscreen->modified )
	{	// FIXME: only restart if CDS is required
		cvar_t	*ref;

		ref = Cvar_Get ("vid_ref", "gl", 0);
		ref->modified = true;
	}

	if ( r_log->modified )
	{
		GLimp_EnableLogging( r_log->integer );
		r_log->modified = false;
	}

	if ( r_log->integer )
	{
		GLimp_LogNewFrame();
	}

	//
	// nVidia fog stuff
	//
	if (glConfig.nvFogAvailable && r_nvfog_dist_mode->modified)
	{
		r_nvfog_dist_mode->modified = false;
		if ( !Q_stricmp (r_nvfog_dist_mode->string, "GL_EYE_PLANE_ABSOLUTE_NV") ) {
			glConfig.nvFogMode = (int)GL_EYE_PLANE_ABSOLUTE_NV;
		}
		else if ( !Q_stricmp (r_nvfog_dist_mode->string, "GL_EYE_PLANE") ) {
			glConfig.nvFogMode = (int)GL_EYE_PLANE;
		}
		else if ( !Q_stricmp (r_nvfog_dist_mode->string, "GL_EYE_RADIAL_NV") ) {
			glConfig.nvFogMode = (int)GL_EYE_RADIAL_NV;
		}
		else {
			glConfig.nvFogMode = (int)GL_EYE_RADIAL_NV;
			Cvar_Set ("r_nvfog_dist_mode", "GL_EYE_RADIAL_NV");
		}
	}

	// change r_clearColor if necessary
	if (r_clearcolor_r->modified) {
		r_clearcolor_r->modified = false;
		clearColor_modified = true;
		r_clearColor[0] = min(max(r_clearcolor_r->value, 0.0f), 1.0f);
	}
	if (r_clearcolor_g->modified) {
		r_clearcolor_g->modified = false;
		clearColor_modified = true;
		r_clearColor[1] = min(max(r_clearcolor_g->value, 0.0f), 1.0f);
	}
	if (r_clearcolor_b->modified) {
		r_clearcolor_b->modified = false;
		clearColor_modified = true;
		r_clearColor[2] = min(max(r_clearcolor_b->value, 0.0f), 1.0f);
	}
	if (clearColor_modified) {
		qglClearColor (r_clearColor[0], r_clearColor[1], r_clearColor[2], r_clearColor[3]);
	}

	//
	// update 3Dfx gamma -- it is expected that a user will do a vid_restart
	// after tweaking this value
	//
	if ( vid_gamma->modified )
	{
		vid_gamma->modified = false;

		if ( glConfig.rendType == GLREND_VOODOO )
		{
			char envbuffer[1024];
			float g;

			g = 2.00 * ( 0.8 - ( vid_gamma->value - 0.5 ) ) + 1.0F;
			Com_sprintf( envbuffer, sizeof(envbuffer), "SSTV2_GAMMA=%f", g );
			putenv( envbuffer );
			Com_sprintf( envbuffer, sizeof(envbuffer), "SST_GAMMA=%f", g );
			putenv( envbuffer );
		}
		UpdateGammaRamp ();
	}

	GLimp_BeginFrame( camera_separation );

	//
	// go into 2D mode
	//
	qglViewport (0,0, vid.width, vid.height);
	qglMatrixMode(GL_PROJECTION);
    qglLoadIdentity ();
	qglOrtho  (0, vid.width, vid.height, 0, -99999, 99999);
	qglMatrixMode(GL_MODELVIEW);
    qglLoadIdentity ();

	GL_Disable (GL_DEPTH_TEST);
	GL_Disable (GL_CULL_FACE);
	GL_Disable (GL_BLEND);
	GL_Enable (GL_ALPHA_TEST);
	qglColor4f (1,1,1,1);

	//
	// draw buffer stuff
	//
	if ( r_drawbuffer->modified )
	{
		r_drawbuffer->modified = false;

		if ( glState.camera_separation == 0 || !glState.stereo_enabled )
		{
			if ( Q_stricmp( r_drawbuffer->string, "GL_FRONT" ) == 0 )
				qglDrawBuffer( GL_FRONT );
			else
				qglDrawBuffer( GL_BACK );
		}
	}

	//
	// texturemode stuff
	//
	if ( r_texturemode->modified )
	{
		GL_TextureMode( r_texturemode->string );
		r_texturemode->modified = false;
	}

	if (r_anisotropic->modified) // added anisotropic filter update
	{
		GL_UpdateAnisoMode ();
		r_anisotropic->modified = false;
	}
/*
	if ( r_texturealphamode->modified )
	{
		GL_TextureAlphaMode (r_texturealphamode->string);
		r_texturealphamode->modified = false;
	}

	if ( r_texturesolidmode->modified )
	{
		GL_TextureSolidMode (r_texturesolidmode->string);
		r_texturesolidmode->modified = false;
	}
*/
	//
	// swapinterval stuff
	//
	GL_UpdateSwapInterval();

	//
	// clear screen if desired
	//
	R_Clear ();
}

/*
=============
R_SetPalette
=============
*/
unsigned r_rawpalette[256];

void R_SetPalette ( const unsigned char *palette)
{
	int		i;

	byte *rp = ( byte * ) r_rawpalette;

	if ( palette )
	{
		for ( i = 0; i < 256; i++ )
		{
			rp[i*4+0] = palette[i*3+0];
			rp[i*4+1] = palette[i*3+1];
			rp[i*4+2] = palette[i*3+2];
			rp[i*4+3] = 0xff;
		}
	}
	else
	{
		for ( i = 0; i < 256; i++ )
		{
			rp[i*4+0] = d_8to24table[i] & 0xff;
			rp[i*4+1] = ( d_8to24table[i] >> 8 ) & 0xff;
			rp[i*4+2] = ( d_8to24table[i] >> 16 ) & 0xff;
			rp[i*4+3] = 0xff;
		}
	}
	//GL_SetTexturePalette( r_rawpalette );

	qglClearColor (0,0,0,0);
	qglClear (GL_COLOR_BUFFER_BIT);
//	qglClearColor (1,0, 0.5 , 0.5);
	qglClearColor (r_clearColor[0], r_clearColor[1], r_clearColor[2], r_clearColor[3]);
}
