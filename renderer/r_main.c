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

// r_main.c

#include "r_local.h"
#include "vlights.h"

void R_Clear (void);

viddef_t	vid;

model_t		*r_worldmodel;

float		gldepthmin, gldepthmax;

glconfig_t	gl_config;
glstate_t	gl_state;

image_t		*r_notexture;		// use for bad textures
#ifdef	ROQ_SUPPORT
image_t		*r_rawtexture;		// used for cinematics
#endif // ROQ_SUPPORT
image_t		*r_envmappic;
image_t		*r_spheremappic;
image_t		*r_causticpic;
image_t		*r_shelltexture;
image_t		*r_particlebeam;

//Knightmare- Psychospaz's enhanced particles
image_t		*r_particletextures[PARTICLE_TYPES]; //list for particles

entity_t	*currententity;
int			r_worldframe; // Knightmare added for trans animations
model_t		*currentmodel;

cplane_t	frustum[4];

int			r_visframecount;	// bumped when going to a new PVS
int			r_framecount;		// used for dlight push checking

int			c_brush_polys, c_alias_polys;

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


//
// screen size info
//
refdef_t	r_newrefdef;

int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

// Psychospaz's chasecam
cvar_t	*cl_3dcam;
cvar_t	*cl_3dcam_angle;
cvar_t	*cl_3dcam_dist;
cvar_t	*cl_3dcam_alpha;
cvar_t	*cl_3dcam_adjust;
cvar_t	*cl_3dcam_yaw;
// end chasecam

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
cvar_t	*r_lefthand;
cvar_t	*r_waterwave;	// water waves
cvar_t  *r_caustics;	// Barnes water caustics
cvar_t  *r_glows;		// texture glows
cvar_t	*r_saveshotsize;//  save shot size option

cvar_t	*r_dlights_normal; // lerped dlights on models
cvar_t	*r_model_shading;
cvar_t	*r_model_dlights;

cvar_t	*r_lightlevel;	// FIXME: This is a HACK to get the client's light level

cvar_t	*r_overbrightbits; // Vic's overbright rendering

cvar_t	*r_nosubimage;
cvar_t	*r_vertex_arrays;

cvar_t	*r_ext_swapinterval;
cvar_t	*r_ext_multitexture;
cvar_t	*r_ext_draw_range_elements;
cvar_t	*r_ext_compiled_vertex_array;
cvar_t	*r_ext_mtexcombine; // Vic's overbright rendering
cvar_t	*r_stencilTwoSide; // Echon's two-sided stenciling
cvar_t	*r_arb_vertex_buffer_object;
cvar_t	*r_trans_lightmaps; // disabling of lightmaps on trans and warp surfaces
cvar_t	*r_solidalpha;			// allow disabling of trans33+trans66 surface flag combining
cvar_t	*r_pixel_shader_warp; // allow disabling the nVidia water warp

cvar_t	*r_glass_envmaps; // Psychospaz's envmapping
cvar_t	*r_trans_surf_sorting; // trans bmodel sorting
cvar_t	*r_shelltype; // entity shells: 0 = solid, 1 = warp, 2 = spheremap
cvar_t	*r_ext_texture_compression; // Heffo - ARB Texture Compression
cvar_t	*r_screenshot_jpeg;			// Heffo - JPEG Screenshots
cvar_t	*r_screenshot_jpeg_quality;	// Heffo - JPEG Screenshots

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
cvar_t	*r_round_down;
cvar_t	*r_picmip;
cvar_t	*r_skymip;
cvar_t	*r_playermip;
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
cvar_t	*r_texturealphamode;
cvar_t	*r_texturesolidmode;
cvar_t	*r_anisotropic;
cvar_t	*r_anisotropic_avail;
cvar_t	*r_lockpvs;

cvar_t	*r_3dlabs_broken;

cvar_t	*vid_fullscreen;
cvar_t	*vid_gamma;
cvar_t	*vid_ref;

#ifdef LIGHT_BLOOMS
cvar_t  *r_bloom;
#endif

cvar_t	*r_skydistance; //Knightmare- variable sky range
cvar_t	*r_saturation;	//** DMP


/*
=================
R_CullBox

Returns true if the box is completely outside the frustom
=================
*/
qboolean R_CullBox (vec3_t mins, vec3_t maxs)
{
	int		i;

	if (r_nocull->value)
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
	if (!r_polyblend->value)
		return;
	if (!v_blend[3])
		return;

	GL_Disable (GL_ALPHA_TEST);
	GL_Enable (GL_BLEND);
	GL_Disable (GL_DEPTH_TEST);
	//qglDisable (GL_TEXTURE_2D);
	GL_DisableTexture(0);

    qglLoadIdentity ();

	// FIXME: get rid of these
    qglRotatef (-90,  1, 0, 0);	    // put Z going up
    qglRotatef (90,  0, 0, 1);	    // put Z going up

	qglColor4fv (v_blend);

	qglBegin (GL_QUADS);
	qglVertex3f (10, 100, 100);
	qglVertex3f (10, -100, 100);
	qglVertex3f (10, -100, -100);
	qglVertex3f (10, 100, -100);
	qglEnd ();

	GL_Disable (GL_BLEND);
	//qglEnable (GL_TEXTURE_2D);
	GL_EnableTexture(0);
	//GL_Enable (GL_ALPHA_TEST);

	qglColor4f(1,1,1,1);
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
	/*
	** this code is wrong, since it presume a 90 degree FOV both in the
	** horizontal and vertical plane
	*/
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

	c_brush_polys = 0;
	c_alias_polys = 0;

	// clear out the portion of the screen that the NOWORLDMODEL defines
	/*if ( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
	{
		GL_Enable( GL_SCISSOR_TEST );
		qglClearColor( 0.3, 0.3, 0.3, 1 );
		qglScissor( r_newrefdef.x, vid.height - r_newrefdef.height - r_newrefdef.y, r_newrefdef.width, r_newrefdef.height );
		qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		qglClearColor( 1, 0, 0.5, 0.5 );
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

   xmin += -( 2 * gl_state.camera_separation ) / zNear;
   xmax += -( 2 * gl_state.camera_separation ) / zNear;

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

	//Knightmare- variable sky range
	static GLdouble farz; 
	GLdouble boxsize;
	//end Knightmare

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
    MYgluPerspective (r_newrefdef.fov_y,  screenaspect,  4, farz); //was 4096
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

//	if ( gl_state.camera_separation != 0 && gl_state.stereo_enabled )
//		qglTranslatef ( gl_state.camera_separation, 0, 0 );

	qglGetFloatv (GL_MODELVIEW_MATRIX, r_world_matrix);

	//
	// set drawing parms
	//
	if (r_cull->value)
		GL_Enable(GL_CULL_FACE);
	else
		GL_Disable(GL_CULL_FACE);

	GL_Disable(GL_BLEND);
	GL_Disable(GL_ALPHA_TEST);
	GL_Enable(GL_DEPTH_TEST);
}


/*
=============
R_Clear
=============
*/
void R_Clear (void)
{
	if (r_ztrick->value)
	{
		static int trickframe;

		if (gl_clear->value)
			qglClear (GL_COLOR_BUFFER_BIT);

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
		if (gl_clear->value)
			qglClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		else
			qglClear (GL_DEPTH_BUFFER_BIT);
		gldepthmin = 0;
		gldepthmax = 1;
		GL_DepthFunc (GL_LEQUAL);
	}

	GL_DepthRange (gldepthmin, gldepthmax);

	//Knightmare- 12/24/2001- stencil buffer
	if (gl_config.have_stencil)
	{
		if (r_shadows->value == 3) // BeefQuake R6 shadows
			qglClearStencil(0);
		else
			qglClearStencil(1);
		qglClear(GL_STENCIL_BUFFER_BIT);
	}

	GL_DepthRange (gldepthmin, gldepthmax);
	//end Knightmare
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
	if (r_norefresh->value)
		return;

	r_newrefdef = *fd;

	if (!r_worldmodel && !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
		VID_Error (ERR_DROP, "R_RenderView: NULL worldmodel");

	if (r_speeds->value)
	{
		c_brush_polys = 0;
		c_alias_polys = 0;
	}

	R_PushDlights ();

	if (r_finish->value)
		qglFinish ();

	R_SetupFrame ();

	R_SetFrustum ();

	R_SetupGL ();

	R_MarkLeaves ();	// done here so we know if we're in water

	R_DrawWorld ();

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL) // options menu
	{
		qboolean fog_on = false;
		//Knightmare- no fogging on menu/hud models
		if (qglIsEnabled(GL_FOG)) //check if fog is enabled
		{
			fog_on = true;
			qglDisable(GL_FOG); //if so, disable it
		}

		//R_DrawAllDecals();
		R_DrawAllEntities(false);
		R_DrawAllParticles();

		//re-enable fog if it was on
		if (fog_on)
			qglEnable(GL_FOG);
	}
	else
	{
		GL_Disable (GL_ALPHA_TEST);

		R_RenderDlights();

		if (r_transrendersort->value) {
			//R_BuildParticleList();
			R_SortParticlesOnList();
		#ifdef DECALS
			R_DrawAllDecals();
		#endif
			//R_DrawAllEntityShadows();
			R_DrawSolidEntities();
			R_DrawEntitiesOnList(ents_trans);
		}
		else {
		#ifdef DECALS
			R_DrawAllDecals();
		#endif
			//R_DrawAllEntityShadows();
			R_DrawAllEntities(true);
		}

		R_DrawAllParticles ();

		R_DrawEntitiesOnList(ents_viewweaps);

		R_ParticleStencil (1);
		R_DrawAlphaSurfaces ();
		R_ParticleStencil (2);

		R_ParticleStencil (3);
		if (r_particle_overdraw->value) // redraw over alpha surfaces, those behind are occluded
			R_DrawAllParticles ();
		R_ParticleStencil (4);

		//always draw vwep last...
		R_DrawEntitiesOnList(ents_viewweaps_trans);

	#ifdef LIGHT_BLOOMS
		R_BloomBlend (fd);//BLOOMS
	#endif

		R_Flash();
	}
	R_SetFog();
}


/*
================
R_SetGL2D
================
*/
void Con_DrawString (int x, int y, char *string, int alpha);
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
	if (r_speeds->value && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) // don't do this for options menu
	{
		char	S[128];
		int		i, x, y, n = 0;

		for (i = 0; i < 4; i++)
		{
			switch (i) {
				case 0: n = sprintf (S, S_COLOR_ALT S_COLOR_SHADOW"%5i wpoly", c_brush_polys); break;
				case 1: n = sprintf (S, S_COLOR_ALT S_COLOR_SHADOW"%5i epoly", c_alias_polys); break;
				case 2: n = sprintf (S, S_COLOR_ALT S_COLOR_SHADOW"%5i tex  ", c_visible_textures); break;
				case 3: n = sprintf (S, S_COLOR_ALT S_COLOR_SHADOW"%5i lmaps", c_visible_lightmaps); break;
				default: break;
			}
			if (scr_netgraph_pos->value)
				x = r_newrefdef.width - (n*FONT_SIZE + FONT_SIZE/2);
			else
				x = FONT_SIZE/2;
			y = r_newrefdef.height-(4-i)*(FONT_SIZE+2);
			Con_DrawString (x, y, S, 255);
		}
	}
}


static void GL_DrawColoredStereoLinePair( float r, float g, float b, float y )
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

	//if ( !( gl_config.renderer & GL_RENDERER_INTERGRAPH ) )
	//	return;

	if ( !gl_state.stereo_enabled )
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
		if (shadelight[0] > shadelight[2])
			r_lightlevel->value = 150*shadelight[0];
		else
			r_lightlevel->value = 150*shadelight[2];
	}
	else
	{
		if (shadelight[1] > shadelight[2])
			r_lightlevel->value = 150*shadelight[1];
		else
			r_lightlevel->value = 150*shadelight[2];
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


void R_Register( void )
{
	// added Psychospaz's console font size option
	con_font = Cvar_Get ("con_font", "default", CVAR_ARCHIVE);
	con_font_size = Cvar_Get ("con_font_size", "16", CVAR_ARCHIVE);
	alt_text_color = Cvar_Get ("alt_text_color", "9", CVAR_ARCHIVE);
	scr_netgraph_pos = Cvar_Get ("netgraph_pos", "0", CVAR_ARCHIVE);
	// Psychospaz's chasecam
	cl_3dcam = Cvar_Get ("cl_3dcam", "0", CVAR_ARCHIVE);
	cl_3dcam_angle = Cvar_Get ("cl_3dcam_angle", "0", CVAR_ARCHIVE);
	cl_3dcam_dist = Cvar_Get ("cl_3dcam_dist", "50", CVAR_ARCHIVE);
	cl_3dcam_alpha = Cvar_Get ("cl_3dcam_alpha", "0", CVAR_ARCHIVE);
	cl_3dcam_adjust = Cvar_Get ("cl_3dcam_adjust", "1", CVAR_ARCHIVE);
	cl_3dcam_yaw = Cvar_Get ("cl_3dcam_yaw", "0", CVAR_ARCHIVE);

	gl_driver = Cvar_Get( "gl_driver", "opengl32", CVAR_ARCHIVE );
	gl_allow_software = Cvar_Get( "gl_allow_software", "0", 0 );
	gl_clear = Cvar_Get ("gl_clear", "0", 0);

	r_lefthand = Cvar_Get( "hand", "0", CVAR_USERINFO | CVAR_ARCHIVE );
	r_norefresh = Cvar_Get ("r_norefresh", "0", CVAR_CHEAT);
	r_fullbright = Cvar_Get ("r_fullbright", "0", CVAR_CHEAT);
	r_drawentities = Cvar_Get ("r_drawentities", "1", 0);
	r_drawworld = Cvar_Get ("r_drawworld", "1", CVAR_CHEAT);
	r_novis = Cvar_Get ("r_novis", "0", CVAR_CHEAT);
	r_nocull = Cvar_Get ("r_nocull", "0", CVAR_CHEAT);
	r_lerpmodels = Cvar_Get ("r_lerpmodels", "1", 0);
	r_speeds = Cvar_Get ("r_speeds", "0", 0);
	// hardware gamma
	r_ignorehwgamma = Cvar_Get ("r_ignorehwgamma", "0", CVAR_ARCHIVE);

	// lerped dlights on models
	r_dlights_normal = Cvar_Get("r_dlights_normal", "1", CVAR_ARCHIVE);
	r_model_shading = Cvar_Get( "r_model_shading", "2", CVAR_ARCHIVE );
	r_model_dlights = Cvar_Get( "r_model_dlights", "8", CVAR_ARCHIVE );

	r_lightlevel = Cvar_Get ("r_lightlevel", "0", 0);
	// added Vic's overbright rendering
	r_overbrightbits = Cvar_Get ("r_overbrightbits", "2", CVAR_ARCHIVE);

	r_waterwave = Cvar_Get ("r_waterwave", "5", CVAR_ARCHIVE );
	r_caustics = Cvar_Get ("r_caustics", "1", CVAR_ARCHIVE );
	r_glows = Cvar_Get ("r_glows", "1", CVAR_ARCHIVE );
	r_saveshotsize = Cvar_Get ("r_saveshotsize", "0", CVAR_ARCHIVE );
	r_nosubimage = Cvar_Get( "r_nosubimage", "0", 0 );

	// correct trasparent sorting
	r_transrendersort = Cvar_Get ("r_transrendersort", "1", CVAR_ARCHIVE );
	r_particle_lighting = Cvar_Get ("r_particle_lighting", "1.0", CVAR_ARCHIVE );
	r_particledistance = Cvar_Get ("r_particledistance", "0", CVAR_ARCHIVE );
	r_particle_overdraw = Cvar_Get ("r_particle_overdraw", "0", CVAR_ARCHIVE );
	r_particle_min = Cvar_Get ("r_particle_min", "0", CVAR_ARCHIVE );
	r_particle_max = Cvar_Get ("r_particle_max", "0", CVAR_ARCHIVE );

	r_modulate = Cvar_Get ("r_modulate", "1", CVAR_ARCHIVE );
	r_log = Cvar_Get( "r_log", "0", 0 );
	r_bitdepth = Cvar_Get( "r_bitdepth", "0", 0 );
	r_mode = Cvar_Get( "r_mode", "9", CVAR_ARCHIVE );
	r_lightmap = Cvar_Get ("r_lightmap", "0", 0);
	r_shadows = Cvar_Get ("r_shadows", "1", CVAR_ARCHIVE );
	r_shadowalpha = Cvar_Get ("r_shadowalpha", "0.4", CVAR_ARCHIVE );
	r_shadowrange  = Cvar_Get ("r_shadowrange", "768", CVAR_ARCHIVE );
	r_shadowvolumes = Cvar_Get ("r_shadowvolumes", "0", CVAR_CHEAT );
	r_stencil = Cvar_Get ("r_stencil", "1", CVAR_ARCHIVE );

	r_dynamic = Cvar_Get ("r_dynamic", "1", 0);
	r_nobind = Cvar_Get ("r_nobind", "0", CVAR_CHEAT);
	r_round_down = Cvar_Get ("r_round_down", "1", 0);
	r_picmip = Cvar_Get ("r_picmip", "0", 0);
	r_skymip = Cvar_Get ("r_skymip", "0", 0);
	r_showtris = Cvar_Get ("r_showtris", "0", CVAR_CHEAT);
	r_showbbox = Cvar_Get ("r_showbbox", "0", CVAR_CHEAT); // show model bounding box
	r_ztrick = Cvar_Get ("r_ztrick", "0", 0);
	r_finish = Cvar_Get ("r_finish", "0", CVAR_ARCHIVE);
	r_cull = Cvar_Get ("r_cull", "1", 0);
	r_polyblend = Cvar_Get ("r_polyblend", "1", 0);
	r_flashblend = Cvar_Get ("r_flashblend", "0", 0);
	r_playermip = Cvar_Get ("r_playermip", "0", 0);
	r_monolightmap = Cvar_Get( "r_monolightmap", "0", 0 );
	// changed default texture mode to bilinear

	//GL_LINEAR_MIPMAP_NEAREST = BILINEAR
	//GL_LINEAR_MIPMAP_LINEAR = TRILINEAR

	r_texturemode = Cvar_Get( "r_texturemode", "GL_LINEAR_MIPMAP_LINEAR", CVAR_ARCHIVE );
	r_texturealphamode = Cvar_Get( "r_texturealphamode", "default", CVAR_ARCHIVE );
	r_texturesolidmode = Cvar_Get( "r_texturesolidmode", "default", CVAR_ARCHIVE );
	r_anisotropic = Cvar_Get( "r_anisotropic", "0", CVAR_ARCHIVE );
	r_anisotropic_avail = Cvar_Get( "r_anisotropic_avail", "0", 0 );
	r_lockpvs = Cvar_Get( "r_lockpvs", "0", 0 );

	r_vertex_arrays = Cvar_Get( "r_vertex_arrays", "1", CVAR_ARCHIVE );

	//gl_ext_palettedtexture = Cvar_Get( "gl_ext_palettedtexture", "0", CVAR_ARCHIVE );
	//gl_ext_pointparameters = Cvar_Get( "gl_ext_pointparameters", "1", CVAR_ARCHIVE );
	r_ext_swapinterval = Cvar_Get( "r_ext_swapinterval", "1", CVAR_ARCHIVE );
	r_ext_multitexture = Cvar_Get( "r_ext_multitexture", "1", CVAR_ARCHIVE );
	r_ext_draw_range_elements = Cvar_Get("r_ext_draw_range_elements", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_ext_compiled_vertex_array = Cvar_Get( "r_ext_compiled_vertex_array", "1", CVAR_ARCHIVE );

	// added Vic's overbright rendering
	r_ext_mtexcombine = Cvar_Get ("r_ext_mtexcombine", "1", CVAR_ARCHIVE);

	// Echon's two-sided stenciling
	r_stencilTwoSide = Cvar_Get ("r_stencilTwoSide", "0", CVAR_ARCHIVE);

	r_arb_vertex_buffer_object = Cvar_Get ("r_arb_vertex_buffer_object", "1", CVAR_ARCHIVE);

	// allow disabling the nVidia water warp
	r_pixel_shader_warp = Cvar_Get( "r_pixel_shader_warp", "1", CVAR_ARCHIVE );

	// allow disabling of lightmaps on trans and warp surfaces
	r_trans_lightmaps = Cvar_Get( "r_trans_lightmaps", "0", CVAR_ARCHIVE );

	// allow disabling of trans33+trans66 surface flag combining
	r_solidalpha = Cvar_Get( "r_solidalpha", "1", CVAR_ARCHIVE );	

	// added Psychospaz's envmapping
	r_glass_envmaps = Cvar_Get( "r_glass_envmaps", "1", CVAR_ARCHIVE );
	r_trans_surf_sorting = Cvar_Get( "r_trans_surf_sorting", "0", CVAR_ARCHIVE );
	r_shelltype = Cvar_Get( "r_shelltype", "1", CVAR_ARCHIVE );

	r_ext_texture_compression = Cvar_Get( "r_ext_texture_compression", "0", CVAR_ARCHIVE ); // Heffo - ARB Texture Compression

	r_screenshot_jpeg = Cvar_Get( "r_screenshot_jpeg", "1", CVAR_ARCHIVE );					// Heffo - JPEG Screenshots
	r_screenshot_jpeg_quality = Cvar_Get( "r_screenshot_jpeg_quality", "100", CVAR_ARCHIVE );	// Heffo - JPEG Screenshots

	//r_motionblur = Cvar_Get( "r_motionblur", "0", CVAR_ARCHIVE );	// motionblur

	r_drawbuffer = Cvar_Get( "r_drawbuffer", "GL_BACK", 0 );
	r_swapinterval = Cvar_Get( "r_swapinterval", "1", CVAR_ARCHIVE );

	r_saturatelighting = Cvar_Get( "r_saturatelighting", "0", 0 );

	r_3dlabs_broken = Cvar_Get( "r_3dlabs_broken", "1", CVAR_ARCHIVE );

	vid_fullscreen = Cvar_Get( "vid_fullscreen", "1", CVAR_ARCHIVE );
	vid_gamma = Cvar_Get( "vid_gamma", "0.8", CVAR_ARCHIVE ); // was 1.0
	vid_ref = Cvar_Get( "vid_ref", "gl", CVAR_ARCHIVE );

#ifdef LIGHT_BLOOMS
	r_bloom = Cvar_Get( "r_bloom", "1", CVAR_ARCHIVE );
#endif

	r_skydistance = Cvar_Get("r_skydistance", "10000", CVAR_ARCHIVE); // variable sky range
	r_saturation = Cvar_Get( "r_saturation", "1.0", CVAR_ARCHIVE );	//** DMP saturation setting (.89 good for nvidia)
	r_lightcutoff = Cvar_Get( "r_lightcutoff", "0", CVAR_ARCHIVE );	//** DMP dynamic light cutoffnow variable

	Cmd_AddCommand( "imagelist", R_ImageList_f );
	Cmd_AddCommand( "screenshot", R_ScreenShot_f );
	Cmd_AddCommand( "modellist", Mod_Modellist_f );
	Cmd_AddCommand( "gl_strings", GL_Strings_f );
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

	if ( vid_fullscreen->modified && !gl_config.allowCDS )
	{
		VID_Printf( PRINT_ALL, "R_SetMode() - CDS not allowed with this driver\n" );
		Cvar_SetValue( "vid_fullscreen", !vid_fullscreen->value );
		vid_fullscreen->modified = false;
	}

	fullscreen = vid_fullscreen->value;
	r_skydistance->modified = true; // skybox size variable

	// don't allow modes 0, 1, or 2
	/*if ((r_mode->value > -1) && (r_mode->value < 3))
		Cvar_SetValue( "r_mode", 3);*/

	vid_fullscreen->modified = false;
	r_mode->modified = false;

	if ( ( err = GLimp_SetMode( &vid.width, &vid.height, r_mode->value, fullscreen ) ) == rserr_ok )
	{
		gl_state.prev_mode = r_mode->value;
	}
	else
	{
		if ( err == rserr_invalid_fullscreen )
		{
			Cvar_SetValue( "vid_fullscreen", 0);
			vid_fullscreen->modified = false;
			VID_Printf( PRINT_ALL, "ref_gl::R_SetMode() - fullscreen unavailable in this mode\n" );
			if ( ( err = GLimp_SetMode( &vid.width, &vid.height, r_mode->value, false ) ) == rserr_ok )
				return true;
		}
		else if ( err == rserr_invalid_mode )
		{
			Cvar_SetValue( "r_mode", gl_state.prev_mode );
			r_mode->modified = false;
			VID_Printf( PRINT_ALL, "ref_gl::R_SetMode() - invalid mode\n" );
		}

		// try setting it back to something safe
		if ( ( err = GLimp_SetMode( &vid.width, &vid.height, gl_state.prev_mode, false ) ) != rserr_ok )
		{
			VID_Printf( PRINT_ALL, "ref_gl::R_SetMode() - could not revert to safe mode\n" );
			return false;
		}
	}
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

	// output system info
	VID_Printf (PRINT_ALL, "OS: %s\n", Cvar_VariableString("sys_osVersion"));
	VID_Printf (PRINT_ALL, "CPU: %s\n", Cvar_VariableString("sys_cpuString"));
	VID_Printf (PRINT_ALL, "RAM: %s MB\n", Cvar_VariableString("sys_ramMegs"));

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
	gl_state.prev_mode = 3;

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
	gl_config.vendor_string = qglGetString (GL_VENDOR);
	VID_Printf (PRINT_ALL, "GL_VENDOR: %s\n", gl_config.vendor_string );
	gl_config.renderer_string = qglGetString (GL_RENDERER);
	VID_Printf (PRINT_ALL, "GL_RENDERER: %s\n", gl_config.renderer_string );
	gl_config.version_string = qglGetString (GL_VERSION);
	VID_Printf (PRINT_ALL, "GL_VERSION: %s\n", gl_config.version_string );

	// Knighmare- added max texture size
	qglGetIntegerv(GL_MAX_TEXTURE_SIZE,&gl_config.max_texsize);
	VID_Printf (PRINT_ALL, "GL_MAX_TEXTURE_SIZE: %i\n", gl_config.max_texsize );

	gl_config.extensions_string = qglGetString (GL_EXTENSIONS);
	VID_Printf (PRINT_ALL, "GL_EXTENSIONS: %s\n", gl_config.extensions_string );

	strcpy( renderer_buffer, gl_config.renderer_string );
	strlwr( renderer_buffer );

	strcpy( vendor_buffer, gl_config.vendor_string );
	strlwr( vendor_buffer );

	// find out the renderer model
	if (strstr(vendor_buffer, "nvidia")) {
		gl_config.rendType = GLREND_NVIDIA;
		if (strstr(renderer_buffer, "geforce"))	gl_config.rendType |= GLREND_GEFORCE;
	}
	else if (strstr(vendor_buffer, "ati")) {
		gl_config.rendType = GLREND_ATI;
		if (strstr(vendor_buffer, "radeon"))		gl_config.rendType |= GLREND_RADEON;
	}
	else if (strstr	(vendor_buffer, "sgi"))			gl_config.rendType = GLREND_SGI;
	else if (strstr	(renderer_buffer, "permedia"))	gl_config.rendType = GLREND_PERMEDIA2;
	else if (strstr	(renderer_buffer, "glint"))		gl_config.rendType = GLREND_GLINT_MX;
	else if (strstr	(renderer_buffer, "glzicd"))	gl_config.rendType = GLREND_REALIZM;
	else if (strstr	(renderer_buffer, "pcx1"))		gl_config.rendType = GLREND_PCX1;
	else if (strstr	(renderer_buffer, "pcx2"))		gl_config.rendType = GLREND_PCX2;
	else if (strstr	(renderer_buffer, "pmx"))		gl_config.rendType = GLREND_PMX;
	else if (strstr	(renderer_buffer, "verite"))	gl_config.rendType = GLREND_RENDITION;
	else if (strstr	(vendor_buffer, "sis"))			gl_config.rendType = GLREND_SIS;
	else if (strstr (renderer_buffer, "voodoo"))	gl_config.rendType = GLREND_VOODOO;
	else if (strstr	(renderer_buffer, "gdi generic")) gl_config.rendType = GLREND_MCD;
	else											gl_config.rendType = GLREND_DEFAULT;

	if ( toupper( r_monolightmap->string[1] ) != 'F' )
	{
		if (gl_config.rendType == GLREND_PERMEDIA2)
		{
			Cvar_Set( "r_monolightmap", "A" );
			VID_Printf( PRINT_ALL, "...using r_monolightmap 'a'\n" );
		}
		else
			Cvar_Set( "r_monolightmap", "0" );
	}

	Cvar_Set( "scr_drawall", "0" );

#ifdef __linux__
	Cvar_SetValue( "r_finish", 1 );
#else
	Cvar_SetValue( "r_finish", 0 );
#endif

	// MCD has buffering issues
	if (gl_config.rendType == GLREND_MCD)
		Cvar_SetValue( "r_finish", 1 );

	if (gl_config.rendType & GLREND_3DLABS)
	{
		if ( r_3dlabs_broken->value )
			gl_config.allowCDS = false;
		else
			gl_config.allowCDS = true;
	}
	else
		gl_config.allowCDS = true;

	if ( gl_config.allowCDS )
		VID_Printf( PRINT_ALL, "...allowing CDS\n" );
	else
		VID_Printf( PRINT_ALL, "...disabling CDS\n" );

	//
	// grab extensions
	//

	// GL_EXT_compiled_vertex_array
	// GL_SGI_compiled_vertex_array
	gl_config.extCompiledVertArray = false;
	if ( strstr( gl_config.extensions_string, "GL_EXT_compiled_vertex_array" ) || 
		 strstr( gl_config.extensions_string, "GL_SGI_compiled_vertex_array" ) )
	{
		if (r_ext_compiled_vertex_array->value) {
			qglLockArraysEXT = (void *) qwglGetProcAddress( "glLockArraysEXT" );
			if (qglLockArraysEXT)
				qglUnlockArraysEXT = (void *) qwglGetProcAddress( "glUnlockArraysEXT" );
			if (!qglUnlockArraysEXT) {
				VID_Printf( PRINT_ALL, "..." S_COLOR_RED "GL_EXT/SGI_compiled_vertex_array not properly supported!\n");
				qglLockArraysEXT	= NULL;
				qglUnlockArraysEXT	= NULL;
			}
			else {
				VID_Printf( PRINT_ALL, "...enabling GL_EXT/SGI_compiled_vertex_array\n" );
				gl_config.extCompiledVertArray = true;
			}
		}
		else
			Com_Printf("...ignoring GL_EXT/SGI_compiled_vertex_array\n");
	}
	else
		VID_Printf( PRINT_ALL, "...GL_EXT/SGI_compiled_vertex_array not found\n" );

	// GL_EXT_draw_range_elements
	gl_config.drawRangeElements = false;
	if ( strstr( gl_config.extensions_string, "GL_EXT_draw_range_elements" ) )
	{
		if (r_ext_draw_range_elements->value)
		{
			gl_config.drawRangeElements = true;
			qglDrawRangeElementsEXT = (void *) qwglGetProcAddress("glDrawRangeElementsEXT");
			Com_Printf("...enabling GL_EXT_draw_range_elements\n");
		}
		else
			Com_Printf("...ignoring GL_EXT_draw_range_elements\n");
	}
	else
		VID_Printf( PRINT_ALL, "...GL_EXT_draw_range_elements not found\n" );

#ifdef _WIN32
	// WGL_EXT_swap_control
	if ( strstr( gl_config.extensions_string, "WGL_EXT_swap_control" ) )
	{
		qwglSwapIntervalEXT = ( BOOL (WINAPI *)(int)) qwglGetProcAddress( "wglSwapIntervalEXT" );
		VID_Printf( PRINT_ALL, "...enabling WGL_EXT_swap_control\n" );
	}
	else
		VID_Printf( PRINT_ALL, "...WGL_EXT_swap_control not found\n" );
#endif


#if 0
	if ( strstr( gl_config.extensions_string, "GL_EXT_point_parameters" ) )
	{
		if ( gl_ext_pointparameters->value )
		{
			qglPointParameterfEXT = ( void (APIENTRY *)( GLenum, GLfloat ) ) qwglGetProcAddress( "glPointParameterfEXT" );
			qglPointParameterfvEXT = ( void (APIENTRY *)( GLenum, const GLfloat * ) ) qwglGetProcAddress( "glPointParameterfvEXT" );
			VID_Printf( PRINT_ALL, "...using GL_EXT_point_parameters\n" );
		}
		else
			VID_Printf( PRINT_ALL, "...ignoring GL_EXT_point_parameters\n" );
	}
	else
		VID_Printf( PRINT_ALL, "...GL_EXT_point_parameters not found\n" );

	if ( !qglColorTableEXT &&
		strstr( gl_config.extensions_string, "GL_EXT_paletted_texture" ) && 
		strstr( gl_config.extensions_string, "GL_EXT_shared_texture_palette" ) )
	{
		if (gl_ext_palettedtexture->value)
		{
			VID_Printf( PRINT_ALL, "...using GL_EXT_shared_texture_palette\n" );
			qglColorTableEXT = ( void ( APIENTRY * ) ( int, int, int, int, int, const void * ) ) qwglGetProcAddress( "glColorTableEXT" );
		}
		else
			VID_Printf( PRINT_ALL, "...ignoring GL_EXT_shared_texture_palette\n" );
	}
	else
		VID_Printf( PRINT_ALL, "...GL_EXT_shared_texture_palette not found\n" );
#endif

	// GL_ARB_vertex_buffer_object
	gl_config.vertexBufferObject = false;
	if ( strstr( gl_config.extensions_string, "GL_ARB_vertex_buffer_object" ) )
	{
		if (r_arb_vertex_buffer_object->value)
		{
			VID_Printf( PRINT_ALL, "...using GL_ARB_vertex_buffer_object\n" );
			gl_config.vertexBufferObject = true;

			qglBindBufferARB = (void *) qwglGetProcAddress( "glBindBufferARB" );
			qglDeleteBuffersARB = (void *) qwglGetProcAddress( "glDeleteBuffersARB" );
			qglGenBuffersARB = (void *) qwglGetProcAddress( "glGenBuffersARB" );
			qglBufferDataARB = (void *) qwglGetProcAddress( "glBufferDataARB" );
			qglBufferSubDataARB = (void *) qwglGetProcAddress( "glBufferSubDataARB" );
			qglMapBufferARB = (void *) qwglGetProcAddress( "glMapBufferARB" );
			qglUnmapBufferARB = (void *) qwglGetProcAddress( "glUnmapBufferARB" );
		}
		else
			VID_Printf( PRINT_ALL, "...ignoring GL_ARB_vertex_buffer_object\n");
	}
	else
		VID_Printf( PRINT_ALL, "...GL_ARB_vertex_buffer_object not found\n" );

	// GL_ARB_multitexture
	gl_config.multitexture = false;
	gl_config.max_texunits = 1;
	if ( strstr( gl_config.extensions_string, "GL_ARB_multitexture" ) )
	{
		if (r_ext_multitexture->value)
		{
			VID_Printf( PRINT_ALL, "...using GL_ARB_multitexture\n" );
			gl_config.multitexture = true;
			qglGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &gl_config.max_texunits);
			VID_Printf( PRINT_ALL, "...GL_MAX_TEXTURE_UNITS_ARB: %i\n", gl_config.max_texunits);
			qglMultiTexCoord2fARB = (void *) qwglGetProcAddress( "glMultiTexCoord2fARB" );
			qglActiveTextureARB = (void *) qwglGetProcAddress( "glActiveTextureARB" );
			qglClientActiveTextureARB = (void *) qwglGetProcAddress( "glClientActiveTextureARB" );
		}
		else
			VID_Printf( PRINT_ALL, "...ignoring GL_ARB_multitexture\n" );
	}
	else
		VID_Printf( PRINT_ALL, "...GL_ARB_multitexture not found\n" );

	// GL_ARB_texture_env_combine - Vic
	gl_config.mtexcombine = false;
	if (strstr(gl_config.extensions_string, "GL_ARB_texture_env_combine"))
	{
		if (r_ext_mtexcombine->value)
		{
			Com_Printf("...using GL_ARB_texture_env_combine\n");
			gl_config.mtexcombine = true;
		}
		else
			Com_Printf("..ignoring GL_ARB_texture_env_combine\n");
	}
	else
		Com_Printf("...GL_ARB_texture_env_combine not found\n");

	// GL_EXT_texture_env_combine - Vic
	if (!gl_config.mtexcombine)
	{
		if (strstr(gl_config.extensions_string, "GL_EXT_texture_env_combine"))
		{
			if (r_ext_mtexcombine->value)
			{
				Com_Printf("..using GL_EXT_texture_env_combine\n");
				gl_config.mtexcombine = true;
			}
			else
				Com_Printf("...ignoring GL_EXT_texture_env_combine\n");
		}
		else
			Com_Printf("...GL_EXT_texture_env_combine not found\n");
	}

	// GL_EXT_stencil_wrap
	gl_config.extStencilWrap = false;
	if (strstr(gl_config.extensions_string, "GL_EXT_stencil_wrap"))
	{
		Com_Printf ("...using GL_EXT_stencil_wrap\n");
		gl_config.extStencilWrap = true;
	}
	else
		Com_Printf("...GL_EXT_stencil_wrap not found\n");

	// GL_ATI_separate_stencil - Barnes
	gl_config.atiSeparateStencil = false;
	if (strstr(gl_config.extensions_string, "GL_ATI_separate_stencil"))
	{
		if (r_stencilTwoSide->value)
		{
			qglStencilOpSeparateATI = (void *) qwglGetProcAddress("glStencilOpSeparateATI");
			qglStencilFuncSeparateATI = (void *) qwglGetProcAddress("glStencilFuncSeparateATI");
			if (!qglStencilOpSeparateATI) {
				Com_Printf ("...GL_ATI_separate_stencil not properly supported!\n");
				qglStencilOpSeparateATI = NULL;
			}
			else {
				Com_Printf ("...using GL_ATI_separate_stencil\n");
				gl_config.atiSeparateStencil = true;
			}
		}
		else
			Com_Printf("...ignoring GL_ATI_separate_stencil\n");
	}
	else
		Com_Printf("...GL_ATI_separate_stencil not found\n");

	// GL_EXT_stencil_two_side - Echon
	gl_config.extStencilTwoSide = false;
	if (strstr(gl_config.extensions_string, "GL_EXT_stencil_two_side"))
	{
		if (r_stencilTwoSide->value)
		{
			qglActiveStencilFaceEXT = (void *) qwglGetProcAddress( "glActiveStencilFaceEXT" );
			if (!qglActiveStencilFaceEXT) {
				Com_Printf ("...GL_EXT_stencil_two_side not properly supported!\n");
				qglActiveStencilFaceEXT = NULL;
			}
			else {
				Com_Printf("...using GL_EXT_stencil_two_side\n");
				gl_config.extStencilTwoSide = true;
			}
		}
		else
			Com_Printf("...ignoring GL_EXT_stencil_two_side\n");
	}
	else
		Com_Printf("...GL_EXT_stencil_two_side not found\n");

	// GL_NV_texture_shader - MrG
	if ( strstr( gl_config.extensions_string, "GL_NV_texture_shader" ) )
	{
		VID_Printf(PRINT_ALL, "...using GL_NV_texture_shader\n");
		gl_config.NV_texshaders = true;
	}
	else
	{
		VID_Printf(PRINT_ALL, "...GL_NV_texture_shader not found\n");
		gl_config.NV_texshaders = false;
	}

	// GL_EXT_texture_filter_anisotropic - NeVo
	gl_config.anisotropic = false;
	if ( strstr(gl_config.extensions_string,"GL_EXT_texture_filter_anisotropic") )
	{
		VID_Printf(PRINT_ALL,"...using GL_EXT_texture_filter_anisotropic\n");
		gl_config.anisotropic = true;
		qglGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_config.max_anisotropy);
		Cvar_SetValue("r_anisotropic_avail", gl_config.max_anisotropy);
	}
	else
	{
		VID_Printf(PRINT_ALL,"..GL_EXT_texture_filter_anisotropic not found\n");
		gl_config.anisotropic = false;
		gl_config.max_anisotropy = 0.0;
		Cvar_SetValue("r_anisotropic_avail", 0.0);
	} 

	// GL_SGIS_generate_mipmap
	if ( strstr( gl_config.extensions_string, "GL_SGIS_generate_mipmap") )
	{
		VID_Printf(PRINT_ALL, "...using GL_SGIS_generate_mipmap\n");
		gl_state.sgis_mipmap = true;
	}
	else
	{
		VID_Printf(PRINT_ALL, "...GL_SGIS_generate_mipmap not found\n");
		gl_state.sgis_mipmap = false;
	}

	// GL_ARB_texture_compression - Heffo
	if ( strstr( gl_config.extensions_string, "GL_ARB_texture_compression" ) )
	{
		if(!r_ext_texture_compression->value)
		{
			VID_Printf(PRINT_ALL, "...ignoring GL_ARB_texture_compression\n");
			gl_state.texture_compression = false;
		}
		else
		{
			VID_Printf(PRINT_ALL, "...using GL_ARB_texture_compression\n");
			gl_state.texture_compression = true;
		}
	}
	else
	{
		VID_Printf(PRINT_ALL, "...GL_ARB_texture_compression not found\n");
		gl_state.texture_compression = false;
		Cvar_Set("r_ext_texture_compression", "0");
	}

	// WGL_3DFX_gamma_control
	if ( strstr(gl_config.extensions_string, "WGL_3DFX_gamma_control") )
	{
		if (!r_ignorehwgamma->value)
		{
			qwglGetDeviceGammaRamp3DFX	= ( BOOL (WINAPI *)(HDC, WORD *)) qwglGetProcAddress( "wglGetDeviceGammaRamp3DFX" );
			qwglSetDeviceGammaRamp3DFX	= ( BOOL (WINAPI *)(HDC, WORD *)) qwglGetProcAddress( "wglSetDeviceGammaRamp3DFX" );
			Com_Printf( "...using WGL_3DFX_gamma_control\n" );
		}
		else
			Com_Printf( "...ignoring WGL_3DFX_gamma_control\n" );
	}
	else
		Com_Printf( "...WGL_3DFX_gamma_control not found\n" );


	GL_SetDefaultState();

	// draw our stereo patterns
#if 0 // commented out until H3D pays us the money they owe us
	GL_DrawStereoPattern();
#endif

	R_InitImages ();
	Mod_Init ();
	R_InitParticleTextures ();
	R_DrawInitLocal ();

	R_InitDSTTex (); // init shader warp texture
	R_InitFogVars(); // reset fog variables
	VLight_Init(); // Vic's bmodel lights

	err = qglGetError();
	if ( err != GL_NO_ERROR )
		VID_Printf (PRINT_ALL, "glGetError() = 0x%x\n", err);

	return true;
}


/*
=================
GL_Strings_f
=================
*/
void GL_Strings_f (void)
{
	VID_Printf (PRINT_ALL, "GL_VENDOR: %s\n", gl_config.vendor_string );
	VID_Printf (PRINT_ALL, "GL_RENDERER: %s\n", gl_config.renderer_string );
	VID_Printf (PRINT_ALL, "GL_VERSION: %s\n", gl_config.version_string );
	VID_Printf (PRINT_ALL, "GL_MAX_TEXTURE_SIZE: %i\n", gl_config.max_texsize );
	VID_Printf (PRINT_ALL, "GL_EXTENSIONS: %s\n", gl_config.extensions_string );
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
	Cmd_RemoveCommand ("imagelist");
	Cmd_RemoveCommand ("gl_strings");

	// Knightmare- Free saveshot buffer
	if (saveshotdata)
		free(saveshotdata);

	Mod_FreeAll ();

	R_ShutdownImages ();

	/*
	** shut down OS specific OpenGL stuff like contexts, etc.
	*/
	GLimp_Shutdown();

	/*
	** shutdown our QGL subsystem
	*/
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

	gl_state.camera_separation = camera_separation;

	// Knightmare- added Psychospaz's console font size option
	if (con_font->modified)
		RefreshFont ();

	if (con_font_size->modified)
	{
		if (con_font_size->value<8)
			Cvar_Set( "con_font_size", "8" );
		else if (con_font_size->value>32)
			Cvar_Set( "con_font_size", "32" );

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
		GLimp_EnableLogging( r_log->value );
		r_log->modified = false;
	}

	if ( r_log->value )
	{
		GLimp_LogNewFrame();
	}

	/*
	** update 3Dfx gamma -- it is expected that a user will do a vid_restart
	** after tweaking this value
	*/
	if ( vid_gamma->modified )
	{
		vid_gamma->modified = false;

		if ( gl_config.rendType == GLREND_VOODOO )
		//if ( gl_config.renderer & ( GL_RENDERER_VOODOO ) )
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

	/*
	** go into 2D mode
	*/
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

	/*
	** draw buffer stuff
	*/
	if ( r_drawbuffer->modified )
	{
		r_drawbuffer->modified = false;

		if ( gl_state.camera_separation == 0 || !gl_state.stereo_enabled )
		{
			if ( Q_stricmp( r_drawbuffer->string, "GL_FRONT" ) == 0 )
				qglDrawBuffer( GL_FRONT );
			else
				qglDrawBuffer( GL_BACK );
		}
	}

	/*
	** texturemode stuff
	*/
	if ( r_texturemode->modified )
	{
		GL_TextureMode( r_texturemode->string );
		r_texturemode->modified = false;
	}

	if ( r_texturealphamode->modified )
	{
		GL_TextureAlphaMode( r_texturealphamode->string );
		r_texturealphamode->modified = false;
	}

	if ( r_texturesolidmode->modified )
	{
		GL_TextureSolidMode( r_texturesolidmode->string );
		r_texturesolidmode->modified = false;
	}

	/*
	** swapinterval stuff
	*/
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
	qglClearColor (1,0, 0.5 , 0.5);
}
