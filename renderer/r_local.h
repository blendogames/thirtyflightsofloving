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

#ifdef _WIN32
#  include <windows.h>
#endif

#include <stdio.h>

#if defined(__APPLE__) || (MACOSX)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif // __APPLE__ || MACOSX

#include "glext.h" // Knightmare- MrG's shader waterwarp support
#include <math.h>

#ifndef __linux__
#ifndef GL_COLOR_INDEX8_EXT
#define GL_COLOR_INDEX8_EXT GL_COLOR_INDEX
#endif
#endif

#include "../client/ref.h"

#include "qgl.h"

// define REF_INCLUDE so that the right print/utility function calls
// are used in shared code
//#define REF_INCLUDE

// up / down
#define	PITCH	0
// left / right
#define	YAW		1
// fall over
#define	ROLL	2

#define	WARP_LIGHTMAPS	// whether to support lightmaps on warp surfaces

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

#ifndef __VIDDEF_T
#define __VIDDEF_T
typedef struct
{
	unsigned		width, height;			// coordinates from main game
} viddef_t;
#endif

extern	viddef_t	vid;


/*
  skins will be outline flood filled and mip mapped
  pics and sprites with alpha will be outline flood filled
  pic won't be mip mapped

  model skin
  sprite frame
  wall texture
  pic
*/

typedef enum 
{
	it_skin,
	it_sprite,
	it_wall,
	it_pic,
	it_font,	// Knightmare added
	it_scrap,	// Knightmare added
	it_sky,
	it_part		// Knightmare added
} imagetype_t;

typedef struct image_s
{
	char		name[MAX_QPATH];			// game path, including extension
	unsigned int	hash;						// to speed up searching
	imagetype_t	type;
	int			width, height;				// source image
	int			upload_width, upload_height;	// after power of two and picmip
	int			registration_sequence;		// 0 = free
	struct		msurface_s	*texturechain;	// for sort-by-texture world drawing
	struct		msurface_s	*warp_texturechain;	// same as above, for warp surfaces
#ifdef WARP_LIGHTMAPS
	struct		msurface_s	*warp_lm_texturechain;	// same as above, for lightmapped warp surfaces
#endif
	int			texnum;						// gl texture binding
	float		sl, tl, sh, th;				// 0,0 - 1,1 unless part of the scrap
	qboolean	scrap;
	qboolean	has_alpha;

	qboolean	paletted;

	qboolean	is_cin;					// Heffo - To identify a cin texture's image_t
	float		replace_scale_w;			// Knightmare- for scaling hi-res replacement images
	float		replace_scale_h;			// Knightmare- for scaling hi-res replacement images
} image_t;

#define	MAX_LIGHTMAPS	256	// change by Brendon Chung, was 128
#define	MAX_SCRAPS		32	// was 1

#define	TEXNUM_LIGHTMAPS	1024
//#define	TEXNUM_SCRAPS		1152
//#define	TEXNUM_IMAGES		1153
#define	TEXNUM_SCRAPS		(TEXNUM_LIGHTMAPS + MAX_LIGHTMAPS)
//#define	TEXNUM_IMAGES		(TEXNUM_SCRAPS + 1)
#define	TEXNUM_IMAGES		(TEXNUM_SCRAPS + MAX_SCRAPS)

#define	MAX_GLTEXTURES	16384	// 4096 // Knightmare increased, was 1024

//Harven MD3 ++
#include "r_alias.h" // needs image_t struct
//Harven MD3 --

//===================================================================

typedef enum
{
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_borderless,	// borderless support
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

// borderless support
typedef enum
{
	dt_windowed = 0,
	dt_fullscreen = 1,
	dt_borderless = 2
} dispType_t;


#include "r_model.h"

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);

void GL_UpdateSwapInterval( void );

extern	float	gldepthmin, gldepthmax;

typedef struct
{
	float	x, y, z;
	float	s, t;
	float	r, g, b;
} glvert_t;


#define	MAX_LBM_HEIGHT		480

#define BACKFACE_EPSILON	0.01


//====================================================

extern	image_t		gltextures[MAX_GLTEXTURES];
extern	int			numgltextures;


#define PARTICLE_TYPES 256

typedef enum {
	DL_NULLMODEL1=0,
	DL_NULLMODEL2,
	NUM_DISPLAY_LISTS
} displaylist_t;

typedef struct glmedia_s {
	image_t		*noTexture;			// used for bad textures
	image_t		*whiteTexture;		// used for solid colors
	image_t		*distTextureARB;	// used for warp distortion
	image_t		*celShadeTexture;	// used for cel shading
	image_t		*rawTexture;		// used for cinematics
	image_t		*envMapTexture;
	image_t		*sphereMapTexture;
	image_t		*shellTexture;
	image_t		*flareTexture;		// used for Kex flares
	image_t		*causticWaterTexture;
	image_t		*causticSlimeTexture;
	image_t		*causticLavaTexture;
	image_t		*particleBeam;
	image_t		*particleTextures[PARTICLE_TYPES];
	unsigned	displayLists[NUM_DISPLAY_LISTS];
} glmedia_t;

extern	glmedia_t	glMedia;

extern	entity_t	*currententity;
extern	int			r_worldframe; // Knightmare added for trans animations
extern	model_t		*currentmodel;
extern	int			r_visframecount;
extern	int			r_framecount;
extern	cplane_t	frustum[4];
extern	int			c_brush_calls, c_brush_surfs, c_brush_polys, c_alias_polys, c_part_polys;

// Knightmare- saveshot buffer
typedef struct {
	int		width;
	int		height;
	byte	*buffer;
} saveShot_t;
extern	saveShot_t	r_saveShot;


extern	int			gl_filter_min, gl_filter_max;

//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

extern	GLdouble	r_farZ;	// Knightmare- variable sky range, made this a global var

//
// screen size info
//
extern	refdef_t	r_newrefdef;
extern	int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

extern  cvar_t  *gl_driver;
extern	cvar_t	*r_clear;

extern	cvar_t	*r_norefresh;
extern	cvar_t	*r_lefthand;
extern	cvar_t	*r_drawentities;
extern	cvar_t	*r_drawflares;
extern	cvar_t	*r_drawworld;
extern	cvar_t	*r_speeds;
extern	cvar_t	*r_fullbright;
extern	cvar_t	*r_novis;
extern	cvar_t	*r_nocull;
extern	cvar_t	*r_lerpmodels;
extern	cvar_t	*r_ignorehwgamma; // Knightmare- hardware gamma
extern	cvar_t	*r_displayrefresh; // Knightmare- refresh rate control

extern	cvar_t	*r_waterwave;	// Knightmare- water waves
extern	cvar_t  *r_caustics;	// Barnes water caustics
extern	cvar_t  *r_glows;		// texture glows

// Knightmare- lerped dlights on models
extern	cvar_t *r_dlights_normal;
extern	cvar_t	*r_model_shading;
extern	cvar_t	*r_model_dlights;
extern	cvar_t	*r_model_minlight;
extern	cvar_t	*r_entity_doublelight;

extern	cvar_t	*r_lightlevel;	// FIXME: This is a HACK to get the client's light level

// Knightmare- added Vic's RGB brightening
extern	cvar_t	*r_rgbscale;

// Knightmare- added Psychospaz's console font size option
extern	cvar_t	*con_font_size;
extern	cvar_t	*con_font;
extern	cvar_t	*scr_font;
extern	cvar_t	*ui_font;
extern	cvar_t	*alt_text_color;

//extern	cvar_t	*r_vertex_arrays;	// unused

//extern	cvar_t	*gl_ext_palettedtexture;
//extern	cvar_t	*gl_ext_pointparameters;
//extern	cvar_t	*r_ext_swapinterval;	// unused
extern	cvar_t	*r_ext_draw_range_elements;
extern	cvar_t	*r_ext_compiled_vertex_array;
extern	cvar_t	*r_arb_texturenonpoweroftwo;
extern	cvar_t	*r_nonpoweroftwo_mipmaps;
extern	cvar_t	*r_sgis_generatemipmap;		// Knightmare- whether to use GL_SGIS_generate_mipmap
extern	cvar_t	*r_newlightmapformat;		// Knightmare- whether to use new lightmap format
extern	cvar_t	*r_ext_mtexcombine; // Vic's RGB brightening
extern	cvar_t	*r_stencilTwoSide; // Echon's two-sided stenciling
extern	cvar_t	*r_arb_vertex_buffer_object;
extern	cvar_t	*r_pixel_shader_warp; // allow disabling the nVidia water warp
extern	cvar_t	*r_trans_lighting; // allow disabling of lighting on trans surfaces
extern	cvar_t	*r_warp_lighting; // allow disabling of lighting on warp surfaces
extern	cvar_t	*r_warp_lighting_sample_offset; // allow adjustment of lighting sampling offset
extern	cvar_t	*r_load_warp_lightmaps;		// allow loading of lightmaps on warp surfaces
extern	cvar_t	*r_solidalpha;			// allow disabling of trans33+trans66 surface flag combining
extern	cvar_t	*r_entity_fliproll;		// allow disabling of backwards alias model roll
extern	cvar_t	*r_old_nullmodel;		// allow selection of nullmodel
extern	cvar_t	*r_modelview_lightscale;	// lighting scale for menu modelviews
extern	cvar_t	*r_occlusion_test;			// allow disabling of OpenGL occlusion test for flares

extern	cvar_t	*r_glass_envmaps; // Psychospaz's envmapping
//extern	cvar_t	*r_trans_surf_sorting; // trans bmodel sorting
extern	cvar_t	*r_shelltype; // entity shells: 0 = solid, 1 = warp, 2 = spheremap
extern	cvar_t	*r_ext_texture_compression; // Heffo - ARB Texture Compression
extern	cvar_t	*r_lightcutoff;	//** DMP - allow dynamic light cutoff to be user-settable

extern	cvar_t	*r_debug_media;		// enables output of generated textures as .tga on startup
extern	cvar_t	*r_screenshot_format;		// determines screenshot format
//extern	cvar_t	*r_screenshot_jpeg;			// Heffo - JPEG Screenshots
extern	cvar_t	*r_screenshot_jpeg_quality;	// Heffo - JPEG Screenshots
extern	cvar_t	*r_screenshot_gamma_correct;	// gamma correction for screenshots
extern	cvar_t	*r_screenshot_use_mapname;	// screenshot filename contains mapname

//extern	cvar_t	*r_nosubimage;	// unused
extern	cvar_t	*r_bitdepth;
extern	cvar_t	*r_mode;
#ifdef _WIN32
extern	cvar_t	*r_mode_desktop;	// desktop-resolution display mode
#endif
extern	cvar_t	*r_log;
extern	cvar_t	*r_lightmap;
extern	cvar_t	*r_shadows;
extern	cvar_t	*r_shadowalpha;
extern	cvar_t	*r_shadowrange;
extern	cvar_t	*r_shadowvolumes;
extern	cvar_t	*r_shadow_self;
extern	cvar_t	*r_shadow_zfail;
extern	cvar_t	*r_stencil; //  stenciling for color shells
extern	cvar_t	*r_transrendersort; // correct trasparent sorting
extern	cvar_t	*r_particle_lighting; //  particle lighting
extern	cvar_t	*r_particle_min;
extern	cvar_t	*r_particle_max;
extern	cvar_t	*r_particledistance;

extern	cvar_t	*r_dynamic;
extern  cvar_t  *r_monolightmap;
extern	cvar_t	*r_nobind;
//extern	cvar_t	*r_round_down;	// unused
extern	cvar_t	*r_picmip;
extern	cvar_t	*r_skymip;
//extern	cvar_t	*r_playermip;	// unused
extern	cvar_t	*r_showtris;
extern	cvar_t	*r_showbbox;		// show model culling bounding box
extern	cvar_t	*r_showbbox_entity;	// show solid entity bounding box
extern	cvar_t	*r_finish;
extern	cvar_t	*r_ztrick;
extern	cvar_t	*r_cull;
extern	cvar_t	*r_polyblend;
extern	cvar_t	*r_flashblend;
extern	cvar_t	*r_lightmaptype;
extern	cvar_t	*r_modulate;
extern	cvar_t	*r_drawbuffer;
extern	cvar_t	*r_3dlabs_broken;
extern	cvar_t	*r_swapinterval;
extern	cvar_t	*r_anisotropic;
extern	cvar_t	*r_anisotropic_avail;
extern	cvar_t	*r_font_upscale;
extern	cvar_t	*r_scrap_upscale;
extern	cvar_t	*r_nvfog_dist;
extern	cvar_t	*r_nvfog_dist_mode;
extern	cvar_t	*r_texturemode;
//extern	cvar_t	*r_texturealphamode;
//extern	cvar_t	*r_texturesolidmode;
extern  cvar_t  *r_saturatelighting;
extern  cvar_t  *r_lockpvs;
extern	cvar_t	*r_intensity;

extern	cvar_t	*r_skydistance;		// variable sky range
extern	cvar_t	*r_fog_skyratio;	// variable sky fog ratio
extern	cvar_t	*r_subdivide_size;	// chop size for warp surfaces
//extern	cvar_t	*r_saturation;		//** DMP

// Changable color for r_clearcolor (enabled by gl_clar)
extern	cvar_t	*r_clearcolor_r;
extern	cvar_t	*r_clearcolor_g;
extern	cvar_t	*r_clearcolor_b;

extern  cvar_t  *r_bloom;

// Discoloda's cel shading
extern  cvar_t  *r_celshading;
extern  cvar_t  *r_celshading_width;

extern	cvar_t	*vid_fullscreen;
extern	cvar_t	*vid_gamma;


extern	int		gl_lightmap_format;
extern	int		gl_solid_format;
extern	int		gl_alpha_format;
extern	int		gl_tex_solid_format;
extern	int		gl_tex_alpha_format;

extern	int		c_visible_lightmaps;
extern	int		c_visible_textures;

extern	float	r_world_matrix[16];

extern	vec4_t	r_clearColor;			// for r_clear

// entity sorting struct
typedef struct sortedelement_s sortedelement_t;
struct sortedelement_s 
{
	void			*data;
	vec_t			len;
	vec3_t			org;
	sortedelement_t *left, *right;
};


//
// r_entity.c
//
extern	unsigned int	r_occlusionQueries[MAX_EDICTS];
extern	unsigned int	r_occlusionQuerySamples[MAX_EDICTS];
extern	byte			r_occlusionQueryPending[MAX_EDICTS];

extern	sortedelement_t *ents_trans;
extern	sortedelement_t *ents_flares;
extern	sortedelement_t *ents_viewweaps;
extern	sortedelement_t *ents_viewweaps_trans;

void R_ClearOcclusionQuerySampleList (void);
void R_ShutdownOcclusionQueries (void);
void R_RotateForEntity (entity_t *e, qboolean full);
int R_RollMult (void);
void ElementAddNode (sortedelement_t *base, sortedelement_t *thisElement);
void R_OccludeTestEntitiesOnList (sortedelement_t *list);
void R_DrawEntitiesOnList (sortedelement_t *list);
void R_DrawAllEntities (qboolean addViewWeaps);
void R_DrawAllEntityShadows (void);
void R_DrawSolidEntities ();


//
// r_particle.c
//
extern	sortedelement_t *parts_prerender;

int transCompare (const void *arg1, const void *arg2);
void R_BuildParticleList (void);
void R_SortParticlesOnList (void);
void R_DrawParticles (sortedelement_t *list);
void R_DrawAllParticles (void);
void R_DrawDecals (void);
void R_DrawAllDecals (void);


//
// r_light.c
//
#define DYNAMIC_LIGHT_WIDTH  128
#define DYNAMIC_LIGHT_HEIGHT 128

#define LIGHTMAP_BYTES 4

#define	LM_BLOCK_WIDTH	128
#define	LM_BLOCK_HEIGHT	128

//#define	MAX_LIGHTMAPS	128	// moved this up above to keeep TEXNUM_* values aligned

#define GL_LIGHTMAP_FORMAT	GL_BGRA // was GL_RGBA
#define GL_LIGHTMAP_TYPE	GL_UNSIGNED_INT_8_8_8_8_REV	// was GL_UNSIGNED_BYTE

#define BATCH_LM_UPDATES

typedef struct
{
	unsigned int	left;
	unsigned int	right;
	unsigned int	top;
	unsigned int	bottom;
} rect_t;

typedef struct
{
	int			internal_format;
	int			format;
	int			type;
	int			current_lightmap_texture;

//	msurface_t	*lightmap_surfaces[MAX_LIGHTMAPS];

	int			allocated[LM_BLOCK_WIDTH];

	// the lightmap texture data needs to be kept in
	// main memory so texsubimage can update properly
#ifndef BATCH_LM_UPDATES
	unsigned	lightmap_buffer[LM_BLOCK_WIDTH*LM_BLOCK_HEIGHT];
#else	// BATCH_LM_UPDATES
	unsigned	*lightmap_update[MAX_LIGHTMAPS];
	rect_t		lightrect[MAX_LIGHTMAPS];
	qboolean	modified[MAX_LIGHTMAPS];
#endif // BATCH_LM_UPDATES
} gllightmapstate_t;

extern gllightmapstate_t gl_lms;


void R_MaxColorVec (vec3_t color);
//void R_LightPoint (vec3_t p, vec3_t color);
void R_LightPoint (vec3_t p, vec3_t color, qboolean isEnt);
void R_LightPointDynamics (vec3_t p, vec3_t color, m_dlight_t *list, int *amount, int max);
void R_SurfLightPoint (msurface_t *surf, vec3_t p, vec3_t color, qboolean vertexLightBase);
void R_PushDlights (void);
void R_ShadowLight (vec3_t pos, vec3_t lightAdd);
void R_MarkLights (dlight_t *light, int bit, mnode_t *node);
void R_RenderDlights (void);
void R_AddDynamicLights (msurface_t *surf);
void R_SetCacheState (msurface_t *surf);
void R_BuildLightMap (msurface_t *surf, byte *dest, int stride);
void R_CreateSurfaceLightmap (msurface_t *surf);
void R_EndBuildingLightmaps (void);
void R_BeginBuildingLightmaps (model_t *m);

//====================================================================

extern	model_t	*r_worldmodel;

extern	unsigned	d_8to24table[256];

extern	int		registration_sequence;


//
// r_utils.c
//
float R_ClampValue (float in, float min, float max);
float R_SmoothStep (float in, float side0, float side1);
qboolean R_CullBox (vec3_t mins, vec3_t maxs);
char *R_CopyString (char *in);


//
// r_main.c
//
qboolean R_Init (void *hinstance, void *hWnd, char *reason);
void R_ClearState (void);
void R_Shutdown (void);
void R_RenderView (refdef_t *fd);
void R_BeginFrame (float camera_separation);
void R_SwapBuffers (int);
void R_SetPalette (const unsigned char *palette);
void R_SetFarZ (float skyDistance);


//
// r_misc.c
//
void R_CreateDisplayLists (void);
void R_ClearDisplayLists (void);
void R_InitMedia (void);
void R_ShutdownMedia (void);
void R_GrabScreen (void);					// screenshots for savegames
void R_ScaledScreenshot (const char *name);	// screenshots for savegames
void R_ScreenShot_f (void);
void R_ScreenShot_Silent_f (void);
void R_ScreenShot_TGA_f (void);
void R_ScreenShot_JPG_f (void);
void R_ScreenShot_PNG_f (void);


//
// r_alias_md2.c
//
void R_DrawAliasMD2Model (entity_t *e);
void R_DrawAliasMD2ModelShadow (entity_t *e);


//
// r_alias.c
//
void R_DrawAliasModel (entity_t *e);
void R_DrawAliasModelShadow (entity_t *e);


//
// r_alias_misc.c
//
#define NUMVERTEXNORMALS	162
extern	float	r_avertexnormals[NUMVERTEXNORMALS][3];

// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
extern	float	r_avertexnormal_dots[SHADEDOT_QUANT][256];

extern	float	*shadedots;

#define MAX_MODEL_DLIGHTS 32
extern	m_dlight_t	model_dlights[MAX_MODEL_DLIGHTS];
extern	int			model_dlights_num;

extern	vec3_t	shadelight;
extern	vec3_t	lightspot;
extern	float	shellFlowH, shellFlowV;

void	R_ShadowLight (vec3_t pos, vec3_t lightAdd);
void	R_SetShellBlend (qboolean toggle);
void	R_SetVertexRGBScale (qboolean toggle);
qboolean FlowingShell (void);
float	R_CalcEntAlpha (float alpha, vec3_t point);
float	R_CalcShadowAlpha (entity_t *e);
void	R_ShadowBlend (float alpha);
void	R_FlipModel (qboolean on, qboolean cullOnly);
void	R_SetBlendModeOn (image_t *skin);
void	R_SetBlendModeOff (void);
void	R_SetShadeLight (void);
void	R_DrawAliasModelBBox (vec3_t bbox[8], entity_t *e, float red, float green, float blue, float alpha);
void	R_DrawEntityBBox (entity_t *e, float red, float green, float blue, float alpha);


//
// r_sprite.c
//
void R_DrawSpriteModel (entity_t *e);
void R_OccludeTestFlare (entity_t *e);
void R_DrawFlare (entity_t *e);


//
// r_beam.c
//
void R_DrawBeam( entity_t *e );


//
// r_backend.c
//
// MrG's Vertex array stuff
#define MAX_TEXTURE_UNITS	4 // was 2
#define MAX_VERTICES	16384
#define MAX_INDICES		MAX_VERTICES * 4


#define VA_SetElem2(v,a,b)		((v)[0]=(a),(v)[1]=(b))
#define VA_SetElem3(v,a,b,c)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c))
#define VA_SetElem4(v,a,b,c,d)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c),(v)[3]=(d))

#define VA_SetElem2v(v,a)	((v)[0]=(a)[0],(v)[1]=(a)[1])
#define VA_SetElem3v(v,a)	((v)[0]=(a)[0],(v)[1]=(a)[1],(v)[2]=(a)[2])
#define VA_SetElem4v(v,a)	((v)[0]=(a)[0],(v)[1]=(a)[1],(v)[2]=(a)[2],(v)[3]=(a)[3])


extern unsigned	indexArray[MAX_INDICES];
extern float	texCoordArray[MAX_TEXTURE_UNITS][MAX_VERTICES][2];
extern float	vertexArray[MAX_VERTICES][3];
extern float	colorArray[MAX_VERTICES][4];
extern float	inTexCoordArray[MAX_VERTICES][2];
extern float	celTexCoordArray[MAX_VERTICES][2];	// for cel shading
extern unsigned rb_vertex, rb_index;
// end vertex array stuff

void RB_InitBackend (void);
float RB_CalcGlowColor (renderparms_t *parms);
void RB_ModifyTextureCoords (float *inArray, float *inVerts, int numVerts, tcmodParms_t *parms);
qboolean RB_CheckArrayOverflow (int numVerts, int numIndex);
void RB_RenderMeshGeneric (qboolean drawTris);
void RB_DrawArrays (void);
void RB_DrawRangeArrays (unsigned start, unsigned end);
void RB_DrawPrimitiveArrays (GLenum mode);
void RB_DrawMeshTris (void);


//
// r_arb_program.c
//
void R_Compile_ARB_Programs ();

typedef enum
{
	F_PROG_HEATHAZEMASK = 0,
	F_PROG_WARP,
	F_PROG_WARP_LM,
	F_PROG_WATER_DISTORT,
	NUM_FRAGMENT_PROGRAM
} fr_progs;

typedef enum
{
    V_PROG_DISTORT = 0,
	NUM_VERTEX_PROGRAM
} vrt_progs;


extern GLuint fragment_programs[NUM_FRAGMENT_PROGRAM];
extern GLuint vertex_programs[NUM_VERTEX_PROGRAM];


//
// r_warp.c
//
void R_SubdivideSurface (msurface_t *surf);
void R_DrawWarpSurface (msurface_t *surf, float alpha, qboolean render);


//
// r_sky.c
//
#define	OLD_Q2_SKYDIST		2300
#define DEFAULT_SKYDIST		18400
#define	MIN_SKYDIST			256
#define	SKY_GRID_NUMVERTS	24
#define	SKY_DOME_MULT		1.5f

typedef struct {
	int			ctr_i, ctr_j;	// center point
	int			numFlashes;
	float		startRad;
	float		curRad;
	float		startBright;
	float		curBright;
	float		decay;
	float		lastTime;		// last update timestamp
} skyLightning_t;

typedef struct {
	char		skyBoxName[MAX_QPATH];
	char		cloudName[MAX_QPATH];

	float		skyRotate;
	vec3_t		skyAxis;
	float		skyDistance;

	image_t		*sky_images[6];
	image_t		*cloud_image;

	float		skyMins[2][6];
	float		skyMaxs[2][6];
	float		sky_min, sky_max;

	float		lightningFreq;
	vec2_t		cloudDir;
	vec3_t		cloudTile;
	vec3_t		cloudSpeed;
	vec3_t		cloudAlpha;

	vec3_t		cloudVerts[SKY_GRID_NUMVERTS * SKY_GRID_NUMVERTS];
	vec2_t		cloudBaseTexCoords[SKY_GRID_NUMVERTS * SKY_GRID_NUMVERTS];
	vec2_t		cloudTexCoords[SKY_GRID_NUMVERTS * SKY_GRID_NUMVERTS];
	float		cloudTileSize;
	qboolean	cloudVertsInitialized;

	skyLightning_t	lightning;
} skyInfo_t;

extern skyInfo_t	r_skyInfo;

void R_AddSkySurface (msurface_t *surf);
float R_GetSkyDistance (void);
void R_InitSkyBoxInfo (void);
void R_ClearSkyBox (void);
void R_DrawSkyBox (void);
void R_SetSky (const char *skyName, const char *cloudName, float rotate, vec3_t axis, float distance,
			   float lightningFreq, vec2_t cloudDir, vec3_t cloudTile, vec3_t cloudSpeed, vec3_t cloudAlpha);


//
// r_surface.c
//
//extern	surfaceHandle_t	r_surfaceHandles[MAX_SURFACE_HANDLES];

image_t *R_TextureAnimation (msurface_t *surf);
qboolean R_WarpLightmaps_Enabled (void);
void R_DrawWorld (void);
void R_DrawAllAlphaSurfaces (void);
void R_DrawBrushModel (entity_t *e);
void R_MarkLeaves (void);
void R_BuildPolygonFromSurface (msurface_t *surf);
void R_ResetVertextLights_f (void);
void R_BuildVertexLight (msurface_t *surf);


// 
// r_bloom.c 
// 
void R_BloomBlend( refdef_t *fd ); 
void R_InitBloomTextures( void ); 


#if 0
short LittleShort (short l);
short BigShort (short l);
int	LittleLong (int l);
float LittleFloat (float f);

char	*va(char *format, ...);
// does a varargs printf into a temp buffer
#endif


//
// r_draw.c
//
void	R_RefreshFont (fontslot_t font);
void	R_RefreshAllFonts (void);
void	R_DrawInitLocal (void);
void	R_DrawGetPicSize (int *w, int *h, char *name);
float	R_CharMapScale (void);
void	R_InitChars (void);
void	R_FlushChars (fontslot_t font);
void	R_DrawChar (float x, float y, int num, fontslot_t font, float scale, 
			int red, int green, int blue, int alpha, qboolean italic, qboolean last);
void	R_DrawString (float x, float y, const char *string, fontslot_t font, float scale, 
				int red, int green, int blue, int alpha, qboolean italic, qboolean shadow);
image_t	*R_DrawFindPic (char *name);
void	R_DrawGetPicSize (int *w, int *h, char *pic);
void	R_DrawPic (drawStruct_t *ds);
void	R_DrawFill (int x, int y, int w, int h, int red, int green, int blue, int alpha);
void	R_DrawFadeScreen (void);
void	R_DrawCameraEffect (void);

#ifdef ROQ_SUPPORT
void	R_DrawStretchRaw (int x, int y, int w, int h, const byte *raw, int rawWidth, int rawHeight);
#else // old 8-bit, 256x256 version
void	r_DrawStretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data);
#endif // ROQ_SUPPORT


//
// r_image.c
//
void R_GammaCorrect (byte *buffer, int size);
int	Draw_GetPalette (void);
void GL_ResampleTexture (void *indata, int inwidth, int inheight, void *outdata,  int outwidth, int outheight);
struct image_s *R_RegisterSkin (char *name);

void R_LoadPCX (const char *filename, byte **pic, byte **palette, int *width, int *height);
void R_LoadTGA (const char *filename, byte **pic, int *width, int *height);
void R_WriteTGA (byte *rawImage, int width, int height, int nBytes, const char *filename, qboolean relativePath, qboolean checkIfExists);
void R_LoadPNG (const char *filename, byte **pic, int *width, int *height);
void R_WritePNG (byte *rawImage, int width, int height, int nBytes, const char *filename, qboolean relativePath, qboolean checkIfExists);
void R_LoadJPG (const char *filename, byte **pic, int *width, int *height);
void R_WriteJPG (byte *rawImage, int width, int height, int nBytes, const char *filename, int quality, qboolean relativePath, qboolean checkIfExists);

image_t *R_LoadPic (const char *name, byte *pic, int width, int height, imagetype_t type, int bits);
image_t *R_LoadQuakePic (const char *name, byte *pic, int width, int height, imagetype_t type, int bits);
image_t	*R_FindImage (const char *rawName, imagetype_t type);
void GL_UpdateAnisoMode (void);
void GL_TextureMode (char *string);
void R_ImageList_f (void);
void R_InitFailedImgList (void);
void R_InitImages (void);
void R_ShutdownImages (void);
void R_FreeUnusedImages (void);


//
// r_upscale.c
//
void R_Upscale2x_Render (unsigned int *output, const unsigned int *input, int width, int height);
void R_Upscale4x_Render (unsigned int *output, const unsigned int *input, int width, int height);
void R_Upscale_Init (void);

/*
** GL extension emulation functions
*/
void GL_DrawParticles( int num_particles );


//
// r_fog.c
//
void R_SetFog (void);
void R_SetFog2D (void);
void R_SetSkyFog (qboolean setSkyFog);
void R_SuspendFog (void);
void R_ResumeFog (void);
void R_InitFogVars (void);
void R_SetFogVars (qboolean enable, int model, int density,
				   int start, int end, int red, int green, int blue);


//
// r_fragment.c
//
int R_MarkFragments (const vec3_t origin, const vec3_t axis[3], float radius, int maxPoints, vec3_t *points, int maxFragments, markFragment_t *fragments);


/*
** GL config stuff
*/
enum {
	GLREND_DEFAULT		= 1 << 0,

	GLREND_MCD			= 1 << 1,
	GLREND_3DLABS		= 1 << 2,
	GLREND_GLINT_MX		= 1 << 3,
	GLREND_PCX1			= 1 << 4,
	GLREND_PCX2			= 1 << 5,
	GLREND_PERMEDIA2	= 1 << 6,
	GLREND_PMX			= 1 << 7,
	GLREND_POWERVR		= 1 << 8,
	GLREND_REALIZM		= 1 << 9,
	GLREND_RENDITION	= 1 << 10,
	GLREND_SGI			= 1 << 11,
	GLREND_SIS			= 1 << 12,
	GLREND_VOODOO		= 1 << 13,

	GLREND_NVIDIA		= 1 << 14,
	GLREND_GEFORCE		= 1 << 15,

	GLREND_ATI			= 1 << 16,
	GLREND_RADEON		= 1 << 17,

	GLREND_MATROX		= 1 << 18,
	GLREND_INTEL		= 1 << 19
};


typedef struct
{
	int         rendType;
	const char	*renderer_string;
	const char	*vendor_string;
	const char	*version_string;
	const char	*extensions_string;

	// for parsing newer OpenGL versions
	int			version_major;
	int			version_minor;
	int			version_release;

	qboolean	allowCDS;
	// max texture size
	int			max_texsize;
	int			max_texunits;

	// non-power of two texture support
	qboolean	arbTextureNonPowerOfTwo;

	qboolean	vertexBufferObject;
	qboolean	mtexcombine;	// added Vic's RGB brightening

	qboolean	have_stencil;
	qboolean	extStencilWrap;
	qboolean	atiSeparateStencil;
	qboolean	extStencilTwoSide;

//	qboolean	extCompiledVertArray;
	qboolean	drawRangeElements;

	// texture shader support
	qboolean	arb_fragment_program;
	qboolean	arb_vertex_program;
//	qboolean	NV_texshaders;

	// ARB occlusion query support
	qboolean	occlusionQuery;
	int			queryBitsSupported;

	// anisotropic filtering
	qboolean	anisotropic;
	float		max_anisotropy;

	// nVidia fog support
	qboolean	nvFogAvailable;
	int			nvFogMode;

	qboolean	newLMFormat;			// whether to use GL_BGRA lightmaps
} glconfig_t;


// Knightmare- OpenGL state manager
typedef struct
{
	float			inverse_intensity;
	qboolean		fullscreen;

	int				prev_mode;

//	unsigned char	*d_16to8table;

	int				lightmap_textures;

	int				currenttextures[MAX_TEXTURE_UNITS];
	unsigned int	currenttmu;
	qboolean		activetmu[MAX_TEXTURE_UNITS];

	float			camera_separation;
	qboolean		stereo_enabled;

	// advanced state manager - MrG
	qboolean		texgen;

	qboolean		regCombiners;
	qboolean		sgis_mipmap;
	unsigned int	dst_texture;

	qboolean		envAdd;
	qboolean		stencilEnabled;

	qboolean		gammaRamp;

	qboolean		cullFace;
	qboolean		polygonOffsetFill; // Knightmare added
//	qboolean		TexShaderNV;
	qboolean		vertexProgram;
	qboolean		fragmentProgram;
	qboolean		alphaTest;
	qboolean		blend;
	qboolean		stencilTest;
	qboolean		depthTest;
	qboolean		scissorTest;

	qboolean		texRectangle;
	qboolean		in2d;
//	qboolean		arraysLocked;
	// End - MrG

	GLenum			cullMode;
	GLenum			shadeModelMode;
	GLfloat			depthMin;
	GLfloat			depthMax;
	GLfloat			offsetFactor;
	GLfloat			offsetUnits;
	GLenum			alphaFunc;
	GLclampf		alphaRef;
	GLenum			blendSrc;
	GLenum			blendDst;
	GLenum			depthFunc;
	GLboolean		depthMask;

	qboolean		texture_compression; // Heffo - ARB Texture Compression
//	qboolean		resetVertexLights;

	unsigned char	originalRedGammaTable[256];
	unsigned char	originalGreenGammaTable[256];
	unsigned char	originalBlueGammaTable[256];
} glstate_t;

extern glconfig_t  glConfig;
extern glstate_t   glState;

//
// r_glstate.c
//
void	GL_Stencil (qboolean enable, qboolean shell);
void	R_ParticleStencil (int passnum);
qboolean GL_HasStencil (void);

void	GL_Enable (GLenum cap);
void	GL_Disable (GLenum cap);
void	GL_ShadeModel (GLenum mode);
void	GL_TexEnv (GLenum value);
void	GL_CullFace (GLenum mode);
void	GL_PolygonOffset (GLfloat factor, GLfloat units);
void	GL_AlphaFunc (GLenum func, GLclampf ref);
void	GL_BlendFunc (GLenum src, GLenum dst);
void	GL_DepthFunc (GLenum func);
void	GL_DepthMask (GLboolean mask);
void	GL_DepthRange (GLfloat rMin, GLfloat rMax);
//void	GL_LockArrays (int numVerts);
//void	GL_UnlockArrays (void);
void	GL_EnableTexture (unsigned tmu);
void	GL_DisableTexture (unsigned tmu);
void	GL_EnableMultitexture(qboolean enable);
void	GL_SelectTexture (unsigned tmu);
void	GL_Bind (int texnum);
void	GL_MBind (unsigned tmu, int texnum);
void	GL_SetDefaultState (void);
void	GL_PrintError (int errorCode, char *funcName);

/*
====================================================================

IMPORTED FUNCTIONS

====================================================================
*/

void	VID_Error (int err_level, char *str, ...);

void	Cmd_AddCommand (char *name, void(*cmd)(void));
void	Cmd_RemoveCommand (char *name);
int		Cmd_Argc (void);
char	*Cmd_Argv (int i);
void	Cbuf_ExecuteText (int exec_when, char *text);

void	VID_Printf (int print_level, char *str, ...);

// files will be memory mapped read only
// the returned buffer may be part of a larger pak file,
// or a discrete file from anywhere in the quake search path
// a -1 return means the file does not exist
// NULL can be passed for buf to just determine existance
char	**FS_GetFileList (const char *path, const char *extension, int *num);
int		FS_LoadFile (const char *name, void **buf);
void	FS_FreeFile (void *buf);

// gamedir will be the current directory that generated
// files should be stored to, ie: "f:\quake\id1"
char	*FS_Gamedir (void);
char	*FS_Savegamedir (void);

cvar_t	*Cvar_Get (char *name, char *value, int flags);
cvar_t	*Cvar_Set (char *name, char *value );
void	 Cvar_SetValue (char *name, float value);

qboolean	VID_GetModeInfo (int *width, int *height, int mode);
void		VID_NewWindow (int width, int height);

/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

void		GLimp_BeginFrame (float camera_separation);
void		GLimp_EndFrame (void);
int 		GLimp_Init (void *hinstance, void *hWnd);
void		GLimp_Shutdown (void);
//int     	GLimp_SetMode (int *pwidth, int *pheight, int mode, qboolean fullscreen);
int     	GLimp_SetMode (int *pwidth, int *pheight, int mode, dispType_t fullscreen);	// borderless support
void		GLimp_AppActivate (qboolean active);
void		GLimp_EnableLogging (qboolean enable);
void		GLimp_LogNewFrame (void);
