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

// r_surface.c: surface-related rendering code

#include <assert.h>

#include "r_local.h"

static vec3_t	modelorg;		// relative to viewpoint

msurface_t	*r_alpha_surfaces;

int		c_visible_lightmaps;
int		c_visible_textures;


gllightmapstate_t gl_lms;

static void RB_DrawEnvMap (void);
static void RB_DrawTexGlow (image_t *glowImage);
static void RB_DrawCaustics (msurface_t *surf);
static void R_DrawLightmappedSurface (msurface_t *surf, qboolean render);

static void		LM_InitBlock (void);
static void		LM_UploadBlock (qboolean dynamic);
static qboolean	LM_AllocBlock (int w, int h, int *x, int *y);

extern void R_SetCacheState( msurface_t *surf );
extern void R_BuildLightMap (msurface_t *surf, byte *dest, int stride);

void R_BuildVertexLight (msurface_t *surf);

#ifdef BATCH_LM_UPDATES
void R_UpdateSurfaceLightmap (msurface_t *surf);
void R_RebuildLightmaps (void);
#endif

// render lightmapped surfaces from texture chains
#define MULTITEXTURE_CHAINS

/*
=============================================================

	BRUSH MODELS

=============================================================
*/

/*
===============
R_TextureAnimation

Returns the proper texture for a given time.
Uses msurface_t entity pointer, since currententity
is not valid for the alpha surface pass.
===============
*/
image_t *R_TextureAnimation (msurface_t *surf)
{
	int			c, frame;
	mtexinfo_t	*tex = surf->texinfo;

	if (!tex->next)
		return tex->image;

	if (tex->flags & (SURF_TRANS33|SURF_TRANS66)) {
		if (!surf->entity)
			frame = r_worldframe; 	// use worldspawn frame
		else
			frame = surf->entity->frame;
	}
	else
		frame = currententity->frame;

	c = frame % tex->numframes;
	while (c)
	{
		tex = tex->next;
		c--;
	}

	return tex->image;
}


/*
===============
R_TextureAnimationGlow

Returns the proper glow texture for a given time
===============
*/
image_t *R_TextureAnimationGlow (msurface_t *surf)
{
	int			c, frame;
	mtexinfo_t	*tex = surf->texinfo;

	if (!tex->next)
		return tex->glow;

	if (tex->flags & (SURF_TRANS33|SURF_TRANS66)) {
		if (!surf->entity)
			frame = r_worldframe; 	// use worldspawn frame
		else
			frame = surf->entity->frame;
	}
	else
		frame = currententity->frame;

	c = frame % tex->numframes;
	while (c)
	{
		tex = tex->next;
		c--;
	}

	return tex->glow;
}


/*
===============
R_SetLightingMode
===============
*/
void R_SetLightingMode (int renderflags)
{
	GL_SelectTexture (0);

	if (!glConfig.mtexcombine)// || (renderflags & RF_TRANSLUCENT)) 
	{
		GL_SelectTexture (0);
		GL_TexEnv (GL_REPLACE);
		GL_SelectTexture (1);

		if (r_lightmap->integer)
			GL_TexEnv (GL_REPLACE);
		else 
			GL_TexEnv (GL_MODULATE);
	}
#if 1
	else 
	{
		GL_SelectTexture (0);
		GL_TexEnv (GL_COMBINE_ARB);

		if (renderflags & RF_TRANSLUCENT) {
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
		}
		else {
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
		}

		GL_SelectTexture (1);
		GL_TexEnv (GL_COMBINE_ARB);
		if (r_lightmap->integer != 0) 
		{
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
		} 
		else 
		{
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);

			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_PREVIOUS_ARB);
		}

		if (r_rgbscale->integer)
		{
			qglTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, r_rgbscale->integer);
		}
	}
#else
	else 
	{
		GL_SelectTexture (0);
		GL_TexEnv (GL_COMBINE_EXT);

		if (renderflags & RF_TRANSLUCENT) {
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE);
		}
		else {
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE);
		}

		GL_SelectTexture (1);
		GL_TexEnv (GL_COMBINE_EXT);
		if (r_lightmap->integer) 
		{
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE);
		} 
		else 
		{
			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PREVIOUS_EXT);

			qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE);
			qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_PREVIOUS_EXT);
		}

		if (r_rgbscale->integer)
		{
			qglTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, r_rgbscale->integer);
		}
	}
#endif
}


/*
================
SurfAlphaCalc
================
*/
float SurfAlphaCalc (int flags)
{
	if ((flags & SURF_TRANS33) && (flags & SURF_TRANS66) && r_solidalpha->integer)
//		return DIV254BY255;
		return 1.0;
	else if (flags & SURF_TRANS33)
		return 0.33333;
	else if (flags & SURF_TRANS66)
		return 0.66666;
	else
		return 1.0;
}

/*
================
R_SurfIsDynamic
================
*/
qboolean R_SurfIsDynamic (msurface_t *surf, int *mapNum)
{
	int			map;
	qboolean	is_dynamic = false;

	if (!surf)	return false;
	if (r_fullbright->integer != 0)
		return false;

	for (map = 0; map < MAXLIGHTMAPS && surf->styles[map] != 255; map++) {
		if (r_newrefdef.lightstyles[surf->styles[map]].white != surf->cached_light[map])
			goto dynamic;
	}

	// dynamic this frame or dynamic previously
#ifdef BATCH_LM_UPDATES
	if ( (surf->dlightframe == r_framecount) || surf->cached_dlight )
#else
	if ( (surf->dlightframe == r_framecount) )
#endif	// BATCH_LM_UPDATES
	{
dynamic:
#ifdef BATCH_LM_UPDATES
		if (r_dynamic->integer || surf->cached_dlight) {
#else
		if (r_dynamic->integer) {
#endif	// BATCH_LM_UPDATES
			if ( !(surf->texinfo->flags & (SURF_SKY|SURF_WARP|SURF_NOLIGHTENV)) )
				is_dynamic = true;
		}
	}

	if (mapNum)	*mapNum = map;
	return is_dynamic;
}


/*
================
R_SurfIsLit
================
*/
qboolean R_SurfIsLit (msurface_t *s)
{
	if (!s || !s->texinfo)
		return false;

	if (r_fullbright->integer != 0)
		return false;

	if (s->flags & SURF_DRAWTURB)		
		return (s->texinfo->flags & (SURF_TRANS33|SURF_TRANS66))
				&& !(s->texinfo->flags & SURF_NOLIGHTENV) && r_warp_lighting->integer;
	else
		return (s->texinfo->flags & (SURF_TRANS33|SURF_TRANS66))
				&& !(s->texinfo->flags & SURF_NOLIGHTENV) && r_trans_lighting->integer;
}

/*
================
R_SurfHasEnvMap
================
*/
qboolean R_SurfHasEnvMap (msurface_t *s)
{
	qboolean	solidAlpha;
	if (!s || !s->texinfo)
		return false;

	solidAlpha = ( (s->texinfo->flags & SURF_TRANS33) && (s->texinfo->flags & SURF_TRANS66) && r_solidalpha->integer );
	return ( (s->flags & SURF_ENVMAP) && r_glass_envmaps->integer && !solidAlpha);
}


/*
================
RB_RenderGLPoly

backend for R_DrawGLPoly
================
*/
void RB_RenderGLPoly (msurface_t *surf, qboolean light)
{
	image_t		*image = R_TextureAnimation (surf);
	image_t		*glow = R_TextureAnimationGlow(surf);
	int			i;
	float		alpha = colorArray[0][3];
	qboolean	glowPass, envMap, causticPass;

	if (rb_vertex == 0 || rb_index == 0) // nothing to render
		return;

	glowPass = ( r_glows->integer && (glow != glMedia.notexture) && glConfig.multitexture && light );
	envMap = R_SurfHasEnvMap (surf);
	causticPass = ( r_caustics->integer && (surf->flags & SURF_MASK_CAUSTIC) && glConfig.multitexture && light );

	c_brush_calls++;

	GL_Bind (image->texnum);

	if (light) {
		R_SetVertexRGBScale (true);
		GL_ShadeModel (GL_SMOOTH);
	}

	RB_DrawArrays ();

	if (glowPass) {	// just redraw with existing arrays for glow
		qglDisableClientState (GL_COLOR_ARRAY);
		qglColor4f(1.0, 1.0, 1.0, alpha);
		RB_DrawTexGlow (glow);
		qglColor4f(1.0, 1.0, 1.0, 1.0);
		qglEnableClientState (GL_COLOR_ARRAY);
	}

	if (envMap && !causticPass)
	{	// vertex-lit trans surfaces have more solid envmapping
		float	envAlpha = (r_trans_lighting->integer && !(surf->texinfo->flags & SURF_NOLIGHTENV)) ? 0.15 : 0.10;
		for (i=0; i<rb_vertex; i++) 
			colorArray[i][3] = envAlpha;
		RB_DrawEnvMap ();
		for (i=0; i<rb_vertex; i++) 
			colorArray[i][3] = alpha;
	}

	if (causticPass) // Barnes caustics
		RB_DrawCaustics (surf);

	if (light) {
		R_SetVertexRGBScale (false);
		GL_ShadeModel (GL_FLAT);
	}

	RB_DrawMeshTris ();
	rb_vertex = rb_index = 0;
}


/*
================
R_DrawGLPoly
modified to handle scrolling textures
================
*/
void R_DrawGLPoly (msurface_t *surf, qboolean render)
{
	glpoly_t	*p;
	int			nv, i;
	float		*v, scroll, alpha;
	qboolean	light;

	alpha = SurfAlphaCalc (surf->texinfo->flags);
	light = R_SurfIsLit (surf);

	c_brush_surfs++;

	if (surf->texinfo->flags & SURF_FLOWING) {
		scroll = -64 * ( (r_newrefdef.time / 40.0) - (int)(r_newrefdef.time / 40.0) );
		if (scroll == 0.0)	scroll = -64.0;
	}
	else
		scroll = 0.0;

//	rb_vertex = rb_index = 0;
	for (p = surf->polys; p; p = p->chain)
	{
		nv = p->numverts;
		c_brush_polys += (nv-2);
		v = p->verts[0];
		if (RB_CheckArrayOverflow (nv, (nv-2)*3))
			RB_RenderGLPoly (surf, light);
		for (i=0; i < nv-2; i++) {
			indexArray[rb_index++] = rb_vertex;
			indexArray[rb_index++] = rb_vertex+i+1;
			indexArray[rb_index++] = rb_vertex+i+2;
		}
		for (i=0; i < nv; i++, v+= VERTEXSIZE)
		{
			if (light && p->vertexlight && p->vertexlightset)
				VA_SetElem4(colorArray[rb_vertex],
					(float)(p->vertexlight[i*3+0]*DIV255),
					(float)(p->vertexlight[i*3+1]*DIV255),
					(float)(p->vertexlight[i*3+2]*DIV255), alpha);
			else
				VA_SetElem4(colorArray[rb_vertex], glState.inverse_intensity, glState.inverse_intensity, glState.inverse_intensity, alpha);

			VA_SetElem2(texCoordArray[0][rb_vertex], v[3]+scroll, v[4]);
			VA_SetElem3(vertexArray[rb_vertex], v[0], v[1], v[2]);
			rb_vertex++;
		}
	}

	if (render)
		RB_RenderGLPoly (surf, light);
}

/*
================
R_DrawWarpPoly
================
*/
void R_DrawWarpPoly (msurface_t *surf)
{
	if (!(surf->flags & SURF_DRAWTURB))
		return;

	R_BuildVertexLight (surf);

	// warp texture, no lightmaps
	GL_EnableMultitexture (false);
//	GL_TexEnv(GL_MODULATE);

	R_DrawWarpSurface (surf, 1.0, true);

//	GL_TexEnv(GL_REPLACE);
	GL_EnableMultitexture (true);
}

/*
================
R_DrawTriangleOutlines
================
*/
void R_DrawTriangleOutlines (void)
{
	int			i, j, nv;
	float		*v;
	msurface_t	*surf;
	glpoly_t	*p;

	// not used in multitexture mode
	if (glConfig.multitexture)
		return;
	if (!r_showtris->integer)
		return;

	if (r_showtris->integer == 1)
		GL_Disable(GL_DEPTH_TEST);

	GL_DisableTexture (0);
	qglPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

	rb_vertex = rb_index = 0;
	for (i = 0; i < MAX_LIGHTMAPS; i++)
	{
		for (surf = gl_lms.lightmap_surfaces[i]; surf != 0; surf = surf->lightmapchain)
		{
			for (p = surf->polys; p; p = p->chain)
			{
				v = p->verts[0];
				nv = p->numverts;
				if (RB_CheckArrayOverflow (nv, (nv-2)*3))
					RB_RenderMeshGeneric (false);
				for (j=0; j < nv-2; j++) {
					indexArray[rb_index++] = rb_vertex;
					indexArray[rb_index++] = rb_vertex+j+1;
					indexArray[rb_index++] = rb_vertex+j+2;
				}
				for (j=0; j < nv; j++, v+= VERTEXSIZE)
				{
					VA_SetElem3(vertexArray[rb_vertex], v[0], v[1], v[2]);
					VA_SetElem4(colorArray[rb_vertex], 1, 1, 1, 1);
					rb_vertex++;
				}
			}
		}
	}
//	RB_DrawArrays ();
	RB_RenderMeshGeneric (false);

	qglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	GL_EnableTexture(0);

	if (r_showtris->integer == 1)
		GL_Enable(GL_DEPTH_TEST);
}


/*
================
R_DrawGLPolyChain
================
*/
void R_DrawGLPolyChain (glpoly_t *p, float soffset, float toffset)
{
	float	*v;
	int		j, nv;

	rb_vertex = rb_index = 0;
	for ( ; p != 0; p = p->chain)
	{
		v = p->verts[0];
		nv = p->numverts;
		if (RB_CheckArrayOverflow (nv, (nv-2)*3))
			RB_RenderMeshGeneric (false);
		for (j=0; j < nv-2; j++) {
			indexArray[rb_index++] = rb_vertex;
			indexArray[rb_index++] = rb_vertex+j+1;
			indexArray[rb_index++] = rb_vertex+j+2;
		}
		for (j=0; j < nv; j++, v+= VERTEXSIZE)
		{
			VA_SetElem2(texCoordArray[0][rb_vertex], v[5] - soffset, v[6] - toffset);
			VA_SetElem3(vertexArray[rb_vertex], v[0], v[1], v[2]);
			VA_SetElem4(colorArray[rb_vertex], 1, 1, 1, 1);
			rb_vertex++;
		}
	}
//	RB_DrawArrays ();
	RB_RenderMeshGeneric (false);
}


/*
================
R_BlendLightMaps

This routine takes all the given light mapped surfaces in the world and
blends them into the framebuffer.
================
*/
void R_BlendLightmaps (void)
{
	int			i;
	msurface_t	*surf, *newdrawsurf = 0;

	// not used in multitexture mode
	if (glConfig.multitexture)
		return;
	// don't bother if we're set to fullbright
	if (r_fullbright->integer)
		return;
	if (!r_worldmodel->lightdata)
		return;

	// don't bother writing Z
	GL_DepthMask (false);

	// set the appropriate blending mode unless we're only looking at the
	// lightmaps.
	if (!r_lightmap->integer)
	{
		GL_Enable (GL_BLEND);

		if (r_saturatelighting->integer)
		{
			GL_BlendFunc( GL_ONE, GL_ONE );
		}
		else
		{
			if (r_monolightmap->string[0] != '0')
			{
				switch (toupper(r_monolightmap->string[0]))
				{
				case 'I':
					GL_BlendFunc (GL_ZERO, GL_SRC_COLOR);
					break;
				case 'L':
					GL_BlendFunc (GL_ZERO, GL_SRC_COLOR);
					break;
				case 'A':
				default:
					GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					break;
				}
			}
			else
			{
				GL_BlendFunc (GL_ZERO, GL_SRC_COLOR);
			}
		}
	}

	if (currentmodel == r_worldmodel)
		c_visible_lightmaps = 0;

	// render static lightmaps first
	for (i = 1; i < MAX_LIGHTMAPS; i++)
	{
		if (gl_lms.lightmap_surfaces[i])
		{
			if (currentmodel == r_worldmodel)
				c_visible_lightmaps++;
			GL_Bind( glState.lightmap_textures + i);

			for (surf = gl_lms.lightmap_surfaces[i]; surf != 0; surf = surf->lightmapchain)
			{
				if (surf->polys)
					R_DrawGLPolyChain(surf->polys, 0, 0);
			}
		}
	}

	// render dynamic lightmaps
	if (r_dynamic->integer)
	{
		LM_InitBlock();

		GL_Bind(glState.lightmap_textures+0);

		if (currentmodel == r_worldmodel)
			c_visible_lightmaps++;

		newdrawsurf = gl_lms.lightmap_surfaces[0];

		for (surf = gl_lms.lightmap_surfaces[0]; surf != 0; surf = surf->lightmapchain)
		{
			int			smax, tmax;
			unsigned	*base;

			smax = (surf->extents[0]>>4)+1;
			tmax = (surf->extents[1]>>4)+1;

			if ( LM_AllocBlock( smax, tmax, &surf->dlight_s, &surf->dlight_t ) )
			{
				base = gl_lms.lightmap_buffer;
				base += (surf->dlight_t * LM_BLOCK_WIDTH + surf->dlight_s);		// * LIGHTMAP_BYTES

				R_BuildLightMap (surf, (void *)base, LM_BLOCK_WIDTH*LIGHTMAP_BYTES);
			}
			else
			{
				msurface_t *drawsurf;

				// upload what we have so far
				LM_UploadBlock(true);

				// draw all surfaces that use this lightmap
				for ( drawsurf = newdrawsurf; drawsurf != surf; drawsurf = drawsurf->lightmapchain )
				{
					if (drawsurf->polys)
						R_DrawGLPolyChain( drawsurf->polys, 
							              (drawsurf->light_s - drawsurf->dlight_s) * (1.0 / 128.0), 
										(drawsurf->light_t - drawsurf->dlight_t) * (1.0 / 128.0) );
				}

				newdrawsurf = drawsurf;

				// clear the block
				LM_InitBlock();

				// try uploading the block now
				if (!LM_AllocBlock(smax, tmax, &surf->dlight_s, &surf->dlight_t))
				{
					VID_Error( ERR_FATAL, "Consecutive calls to LM_AllocBlock(%d,%d) failed (dynamic)\n", smax, tmax );
				}

				base = gl_lms.lightmap_buffer;
				base += (surf->dlight_t * LM_BLOCK_WIDTH + surf->dlight_s);		// * LIGHTMAP_BYTES

				R_BuildLightMap (surf, (void *)base, LM_BLOCK_WIDTH*LIGHTMAP_BYTES);
			}
		}

		// draw remainder of dynamic lightmaps that haven't been uploaded yet
		if (newdrawsurf)
			LM_UploadBlock( true );

		for (surf = newdrawsurf; surf != 0; surf = surf->lightmapchain)
		{
			if (surf->polys)
				R_DrawGLPolyChain( surf->polys, (surf->light_s - surf->dlight_s) * (1.0 / 128.0), (surf->light_t - surf->dlight_t) * (1.0 / 128.0) );
		}
	}

	// restore state
	GL_Disable (GL_BLEND);
	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_DepthMask (true);
}


/*
================
R_RenderBrushPoly
================
*/
void R_RenderBrushPoly (msurface_t *fa)
{
	int			maps;
//	qboolean	is_dynamic = false;

	if (fa->flags & SURF_DRAWTURB)
	{	
		R_BuildVertexLight (fa);

		// warp texture, no lightmaps
		GL_TexEnv(GL_MODULATE);

		R_DrawWarpSurface (fa, 1.0, true);

		GL_TexEnv(GL_REPLACE);

		return;
	}

	GL_TexEnv(GL_REPLACE);

	R_DrawGLPoly (fa, true);

#if 0
	//
	// check for lightmap modification
	//
	for (maps = 0; maps < MAXLIGHTMAPS && fa->styles[maps] != 255; maps++)
	{
		if (r_newrefdef.lightstyles[fa->styles[maps]].white != fa->cached_light[maps])
			goto dynamic;
	}

	// dynamic this frame or dynamic previously
	if ((fa->dlightframe == r_framecount))
	{
dynamic:
		if (r_dynamic->integer)
		{
			if ( !(fa->texinfo->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP)) )
			{
				is_dynamic = true;
			}
		}
	}
#endif

	if (R_SurfIsDynamic(fa, &maps))
	{
		if ( (fa->styles[maps] >= 32 || fa->styles[maps] == 0) && (fa->dlightframe != r_framecount) )
		{
			unsigned	temp[34*34];
			int			smax, tmax;

			smax = (fa->extents[0]>>4)+1;
			tmax = (fa->extents[1]>>4)+1;

			R_BuildLightMap(fa, (void *)temp, smax*4);
			R_SetCacheState(fa);

			GL_Bind(glState.lightmap_textures + fa->lightmaptexturenum);

			qglTexSubImage2D( GL_TEXTURE_2D, 0,
							  fa->light_s, fa->light_t, 
							  smax, tmax, 
			//				  GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE,
							  gl_lms.format, gl_lms.type,
							  temp);

			fa->lightmapchain = gl_lms.lightmap_surfaces[fa->lightmaptexturenum];
			gl_lms.lightmap_surfaces[fa->lightmaptexturenum] = fa;
		}
		else
		{
			fa->lightmapchain = gl_lms.lightmap_surfaces[0];
			gl_lms.lightmap_surfaces[0] = fa;
		}
	}
	else
	{
		fa->lightmapchain = gl_lms.lightmap_surfaces[fa->lightmaptexturenum];
		gl_lms.lightmap_surfaces[fa->lightmaptexturenum] = fa;
	}
}


/*
================
R_SurfsAreBatchable
================
*/
qboolean R_SurfsAreBatchable (msurface_t *s1, msurface_t *s2)
{
	if (!s1 || !s2)
		return false;
	if (s1->entity != s2->entity)
		return false;
	if ((s1->flags & SURF_DRAWTURB) != (s2->flags & SURF_DRAWTURB))
		return false;
	if ( ((s1->texinfo->flags & (SURF_TRANS33|SURF_TRANS66)) != 0) != ((s2->texinfo->flags & (SURF_TRANS33|SURF_TRANS66)) != 0) )
		return false;
	if (R_TextureAnimation(s1) != R_TextureAnimation(s2))
		return false;

	if ( (s1->flags & SURF_DRAWTURB) && (s2->flags & SURF_DRAWTURB) )
	{
		if (R_SurfIsLit(s1) != R_SurfIsLit(s2))
			return false;
		return true;
	}
	else if ( (s1->texinfo->flags & (SURF_TRANS33|SURF_TRANS66))
		&& (s2->texinfo->flags & (SURF_TRANS33|SURF_TRANS66)) )
	{
		if (r_trans_lighting->integer == 2
			&& ((R_SurfIsLit(s1) && s1->lightmaptexturenum) || (R_SurfIsLit(s2) && s2->lightmaptexturenum)))
			return false;
		if (R_SurfIsLit(s1) != R_SurfIsLit(s2))
			return false;
		// must be single pass to be batchable
		if ( r_glows->integer
			&& ((R_TextureAnimationGlow(s1) != glMedia.notexture)
				|| (R_TextureAnimationGlow(s2) != glMedia.notexture)) )
			return false;
		if (R_SurfHasEnvMap(s1) || R_SurfHasEnvMap(s2))
			return false;
		if ( r_caustics->integer
			&& ((s1->flags & SURF_MASK_CAUSTIC) || (s2->flags & SURF_MASK_CAUSTIC)) )
			return false;
		return true;
	}
	else if ( !(s1->texinfo->flags & (SURF_DRAWTURB|SURF_TRANS33|SURF_TRANS66))
		&& !(s2->texinfo->flags & (SURF_DRAWTURB|SURF_TRANS33|SURF_TRANS66)) )	// lightmapped surfaces
	{
		if (s1->lightmaptexturenum != s2->lightmaptexturenum)	// lightmap image must be same
			return false;
#ifndef BATCH_LM_UPDATES
		if (R_SurfIsDynamic(s1, NULL) || R_SurfIsDynamic(s2, NULL)) // can't be dynamically list
			return false;
#endif	// BATCH_LM_UPDATES
		if ((s1->texinfo->flags & SURF_ALPHATEST) != (s2->texinfo->flags & SURF_ALPHATEST))
			return false;
		if (R_TextureAnimationGlow(s1) != R_TextureAnimationGlow(s2))
			return false;
		if (R_SurfHasEnvMap(s1) != R_SurfHasEnvMap(s2))
			return false;
		if ((s1->flags & SURF_MASK_CAUSTIC) != (s2->flags & SURF_MASK_CAUSTIC))
			return false;
		return true;
	}
	return false;
}


/*
================
R_DrawAlphaSurfaces

Draw trans water surfaces and windows.
The BSP tree is waled front to back, so unwinding the chain
of alpha_surfaces will draw back to front, giving proper ordering.
================
*/
void R_DrawAlphaSurfaces (void)
{
	msurface_t	*s;
	qboolean	light;//solidAlpha, envMap;

	// the textures are prescaled up for a better lighting range,
	// so scale it back down

	rb_vertex = rb_index = 0;
	for (s = r_alpha_surfaces; s; s = s->texturechain)
	{
		// go back to the world matrix
		qglLoadMatrixf (r_world_matrix);

		R_BuildVertexLight (s);
		GL_Enable (GL_BLEND);
		GL_TexEnv (GL_MODULATE);
		GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// disable depth testing for all bmodel surfs except solid alphas
		if ( s->entity && !((s->flags & SURF_TRANS33) && (s->flags & SURF_TRANS66)) )
			GL_DepthMask (false);
		else
			GL_DepthMask (true);

		// moving trans brushes - spaz
		if (s->entity)
			R_RotateForEntity (s->entity, true);

		light = R_SurfIsLit(s);
	//	solidAlpha = ( (s->texinfo->flags & SURF_TRANS33|SURF_TRANS66) == SURF_TRANS33|SURF_TRANS66 );
	//	envMap = ( (s->flags & SURF_ENVMAP) && r_glass_envmaps->integer && !solidAlpha);

		if (s->flags & SURF_DRAWTURB)		
			R_DrawWarpSurface (s, SurfAlphaCalc(s->texinfo->flags), !R_SurfsAreBatchable (s, s->texturechain));
		else if (r_trans_lighting->integer == 2 && glConfig.multitexture && light && s->lightmaptexturenum)
		{
			GL_EnableMultitexture (true);
			R_SetLightingMode (RF_TRANSLUCENT);
			R_DrawLightmappedSurface (s, true);
			GL_EnableMultitexture (false);
		}
		else
			R_DrawGLPoly (s, !R_SurfsAreBatchable (s, s->texturechain));// true);
	}

	// go back to the world matrix after shifting trans faces
	qglLoadMatrixf (r_world_matrix);

	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_TexEnv (GL_REPLACE);
	qglColor4f (1,1,1,1);
	GL_Disable (GL_BLEND);
	GL_DepthMask (true);

	r_alpha_surfaces = NULL;
}


#ifdef BATCH_LM_UPDATES
/*
=============
R_UpdateSurfaceLightmap

Based on code from MH's experimental Q2 engine
=============
*/
void R_UpdateSurfaceLightmap (msurface_t *surf)
{
	int			map;

	if (R_SurfIsDynamic (surf, &map))
	{
		unsigned	*base = gl_lms.lightmap_update[surf->lightmaptexturenum];
		rect_t		*rect = &gl_lms.lightrect[surf->lightmaptexturenum];

		base += (surf->light_t * LM_BLOCK_WIDTH) + surf->light_s;
		R_BuildLightMap (surf, (void *)base, LM_BLOCK_WIDTH*LIGHTMAP_BYTES);
		R_SetCacheState (surf);
		gl_lms.modified[surf->lightmaptexturenum] = true;

		if (surf->light_s < rect->left)
			rect->left = surf->light_s;
		if ((surf->light_s + surf->light_smax) > rect->right)
			rect->right = surf->light_s + surf->light_smax;
		if (surf->light_t < rect->top)
			rect->top = surf->light_t;
		if ((surf->light_t + surf->light_tmax) > rect->bottom)
			rect->bottom = surf->light_t + surf->light_tmax;
	}
}


/*
=============
R_RebuildLightmaps

Based on code from MH's experimental Q2 engine
=============
*/
void R_RebuildLightmaps (void)
{
	int			i;
	qboolean	storeSet = false;

	for (i=1; i<gl_lms.current_lightmap_texture; i++)
	{
		if (!gl_lms.modified[i])
			continue;

		if ( !glConfig.newLMFormat )
		{
			GL_MBind (1, glState.lightmap_textures + i);
			qglTexSubImage2D (GL_TEXTURE_2D, 0,
					0, gl_lms.lightrect[i].top, 
					LM_BLOCK_WIDTH, (gl_lms.lightrect[i].bottom - gl_lms.lightrect[i].top), 
			//		GL_LIGHTMAP_FORMAT, GL_LIGHTMAP_TYPE,
					gl_lms.format, gl_lms.type,
					gl_lms.lightmap_update[i] + (gl_lms.lightrect[i].top * LM_BLOCK_WIDTH));
		}
		else
		{
			if (!storeSet) {
				qglPixelStorei(GL_UNPACK_ROW_LENGTH, LM_BLOCK_WIDTH);
				storeSet = true;
			}
			GL_MBind (1, glState.lightmap_textures + i);
			qglTexSubImage2D (GL_TEXTURE_2D, 0,
					gl_lms.lightrect[i].left, gl_lms.lightrect[i].top, 
					(gl_lms.lightrect[i].right - gl_lms.lightrect[i].left), (gl_lms.lightrect[i].bottom - gl_lms.lightrect[i].top), 
			//		GL_LIGHTMAP_FORMAT, GL_LIGHTMAP_TYPE,
					gl_lms.format, gl_lms.type,
					gl_lms.lightmap_update[i] + (gl_lms.lightrect[i].top * LM_BLOCK_WIDTH) + gl_lms.lightrect[i].left);
			
		}
			
		gl_lms.modified[i] = false;
		gl_lms.lightrect[i].left = LM_BLOCK_WIDTH;
		gl_lms.lightrect[i].right = 0;
		gl_lms.lightrect[i].top = LM_BLOCK_HEIGHT;
		gl_lms.lightrect[i].bottom = 0;
	}
	if (storeSet)
		qglPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}
#endif // BATCH_LM_UPDATES


/*
================
R_DrawMultiTextureChains

Draws solid warp surfaces im multitexture mode
================
*/
void R_DrawMultiTextureChains (void)
{
	int			i;
	msurface_t	*s;
	image_t		*image;

	c_visible_textures = 0;

#ifdef MULTITEXTURE_CHAINS
	GL_EnableMultitexture (true);
	R_SetLightingMode (0);

#ifdef BATCH_LM_UPDATES
	R_RebuildLightmaps ();
#endif

	for (i=0, image=gltextures; i<numgltextures; i++, image++)
	{
		if (!image->registration_sequence)
			continue;
		if (!image->texturechain)
			continue;

		rb_vertex = rb_index = 0;
		for (s = image->texturechain; s; s=s->texturechain) {
			R_DrawLightmappedSurface (s, !R_SurfsAreBatchable(s, s->texturechain));
		}
		image->texturechain = NULL;
	}
	GL_EnableMultitexture (false);
#endif // MULTITEXTURE_CHAINS

	GL_TexEnv(GL_MODULATE); // warp textures, no lightmaps
	for (i=0, image=gltextures; i<numgltextures; i++, image++)
	{
		if (!image->registration_sequence)
			continue;
		if (!image->warp_texturechain)
			continue;

		rb_vertex = rb_index = 0;
		for (s = image->warp_texturechain; s; s=s->texturechain) {
			R_BuildVertexLight (s);
			R_DrawWarpSurface (s, 1.0, !R_SurfsAreBatchable(s, s->texturechain)); 
		}
		image->warp_texturechain = NULL;
	}
	GL_TexEnv (GL_REPLACE);
}


/*
================
R_DrawTextureChains

Draws all solid textures in 2-pass mode
================
*/
void R_DrawTextureChains (void)
{
	int			i;
	msurface_t	*s;
	image_t		*image;

	c_visible_textures = 0;

	for (i=0, image=gltextures; i<numgltextures; i++, image++)
	{
		if (!image->registration_sequence)
			continue;
		if (!image->texturechain)
			continue;

		c_visible_textures++;
		rb_vertex = rb_index = 0;
		for (s = image->texturechain; s; s=s->texturechain)
			R_RenderBrushPoly (s);

		image->texturechain = NULL;
	}

	for (i=0, image=gltextures; i<numgltextures; i++, image++)
	{
		if (!image->registration_sequence)
			continue;
		if (!image->warp_texturechain)
			continue;

	//	c_visible_textures++;
		rb_vertex = rb_index = 0;
		for (s = image->warp_texturechain; s; s=s->texturechain)
			R_RenderBrushPoly (s);

		image->warp_texturechain = NULL;
	}

	GL_TexEnv (GL_REPLACE);
}


/*
===========================================
RB_DrawEnvMap
===========================================
*/
static void RB_DrawEnvMap (void)
{
	qboolean	previousBlend = false;

	GL_MBind (0, glMedia.envmappic->texnum);
	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (!glState.blend)	GL_Enable (GL_BLEND);
	else				previousBlend = true;

	qglTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	qglTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	qglEnable(GL_TEXTURE_GEN_S);
	qglEnable(GL_TEXTURE_GEN_T);

	RB_DrawArrays ();
	
	qglDisable(GL_TEXTURE_GEN_S);
	qglDisable(GL_TEXTURE_GEN_T);

	if (!previousBlend) // restore state
		GL_Disable (GL_BLEND);
}

/*
===========================================
RB_DrawTexGlow
===========================================
*/
static void RB_DrawTexGlow (image_t *glowImage)
{
	qboolean	previousBlend = false;

	GL_MBind (0, glowImage->texnum);
	GL_BlendFunc (GL_ONE, GL_ONE);
	if (!glState.blend)	GL_Enable (GL_BLEND);
	else				previousBlend = true;

	RB_DrawArrays ();

	if (!previousBlend) // restore state
		GL_Disable (GL_BLEND);
	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


/*
===========================================
RB_CausticForSurface
===========================================
*/
image_t *RB_CausticForSurface (msurface_t *surf)
{
	if (surf->flags & SURF_UNDERLAVA)
		return glMedia.causticlavapic;
	else if (surf->flags & SURF_UNDERSLIME)
		return glMedia.causticslimepic;
	else
		return glMedia.causticwaterpic;
}


/*
===========================================
RB_DrawCaustics
Underwater caustic effect based on code by Kirk Barnes
===========================================
*/
extern unsigned int dst_texture_ARB;
static void RB_DrawCaustics (msurface_t *surf)
{
	int			i, vert=0;	// nv
	float		scrollh, scrollv, scaleh, scalev, dstscroll;	// *v,
	image_t		*causticpic = RB_CausticForSurface (surf);
	qboolean	previousBlend = false;
	qboolean	fragmentWarp = glConfig.multitexture && glConfig.arb_fragment_program && (r_caustics->integer > 1.0);
//	glpoly_t	*p;
	
	// adjustment for texture size and caustic image
	scaleh = surf->texinfo->texWidth / (causticpic->width*0.5);
	scalev = surf->texinfo->texHeight / (causticpic->height*0.5);

	// sin and cos circular drifting
	scrollh = sin(r_newrefdef.time * 0.08 * M_PI) * 0.45;
	scrollv = cos(r_newrefdef.time * 0.08 * M_PI) * 0.45;
	dstscroll = -1.0 * ( (r_newrefdef.time*0.15) - (int)(r_newrefdef.time*0.15) );

	GL_MBind (0, causticpic->texnum);
	if (fragmentWarp)
	{
		GL_EnableTexture(1);
		GL_MBind (1, dst_texture_ARB);
		GL_Enable (GL_FRAGMENT_PROGRAM_ARB);
		qglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fragment_programs[F_PROG_WARP]);
		qglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, 1.0, 1.0, 1.0, 1.0);
	}

	GL_BlendFunc (GL_DST_COLOR, GL_ONE);
	if (!glState.blend)	GL_Enable (GL_BLEND);
	else				previousBlend = true;

	// just reuse verts, color, and index from previous pass
	for (i=0; i<rb_vertex; i++) {
		VA_SetElem2(texCoordArray[0][i], (inTexCoordArray[i][0]*scaleh)+scrollh, (inTexCoordArray[i][1]*scalev)+scrollv);
		VA_SetElem2(texCoordArray[1][i], (inTexCoordArray[i][0]*scaleh)+dstscroll, (inTexCoordArray[i][1]*scalev));
	}
/*	for (p = surf->polys; p; p = p->chain)
	{
		v = p->verts[0];
		nv = p->numverts;
		for (i=0; i<nv; i++, v+= VERTEXSIZE) {
			VA_SetElem2(texCoordArray[0][vert], (v[3]*scaleh)+scrollh, (v[4]*scalev)+scrollv);
			VA_SetElem2(texCoordArray[1][vert], (v[3]*scaleh)+dstscroll, (v[4]*scalev));
			vert++;
		}
	}*/
	RB_DrawArrays ();

	if (fragmentWarp) {
		GL_Disable (GL_FRAGMENT_PROGRAM_ARB);
		GL_DisableTexture(1);
		GL_SelectTexture(0);
	}
	if (!previousBlend) // restore state
		GL_Disable (GL_BLEND);
	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


/*
===========================================
RB_RenderLightmappedSurface

backend for R_DrawLightmappedSurface
===========================================
*/
static void RB_RenderLightmappedSurface (msurface_t *surf)
{
	image_t		*image = R_TextureAnimation (surf);
	image_t		*glow = R_TextureAnimationGlow (surf);
	int			i;
	float		alpha = colorArray[0][3];
	unsigned	lmtex = surf->lightmaptexturenum;
	qboolean	glowLayer, glowPass, envMap, causticPass;
#ifndef BATCH_LM_UPDATES
	int			map;
#endif

	if (rb_vertex == 0 || rb_index == 0) // nothing to render
		return;

	glowLayer = ( r_glows->integer && (glow != glMedia.notexture) && (glConfig.max_texunits > 2) );
	glowPass = ( r_glows->integer && (glow != glMedia.notexture) && !glowLayer );
	envMap = R_SurfHasEnvMap (surf);
	causticPass = ( r_caustics->integer && !(surf->texinfo->flags & SURF_ALPHATEST)
		&& (surf->flags & SURF_MASK_CAUSTIC) );

	c_brush_calls++;

#ifndef BATCH_LM_UPDATES
	if (R_SurfIsDynamic (surf, &map))
	{
		unsigned	temp[LM_BLOCK_WIDTH*LM_BLOCK_HEIGHT];
		int			smax, tmax;

		smax = (surf->extents[0]>>4)+1;
		tmax = (surf->extents[1]>>4)+1;

		R_BuildLightMap (surf, (void *)temp, smax*4);

		if ((surf->styles[map] >= 32 || surf->styles[map] == 0) && (surf->dlightframe != r_framecount))
		{
			R_SetCacheState (surf);
			GL_MBind (1, glState.lightmap_textures + surf->lightmaptexturenum);
			lmtex = surf->lightmaptexturenum;
		}
		else {
			GL_MBind (1, glState.lightmap_textures + 0);
			lmtex = 0;
		}

		qglTexSubImage2D (GL_TEXTURE_2D, 0,
						  surf->light_s, surf->light_t, 
						  smax, tmax, 
		//				  GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE,
						  gl_lms.format, gl_lms.type,
						  temp);
	}
#endif // BATCH_LM_UPDATES

	// Alpha test flag
	if (surf->texinfo->flags & SURF_ALPHATEST)
		GL_Enable (GL_ALPHA_TEST);

	GL_MBind (0, image->texnum);
	if ( (r_fullbright->integer != 0) || (surf->texinfo->flags & SURF_NOLIGHTENV) )
		GL_MBind (1, glMedia.whitetexture->texnum);
	else
		GL_MBind (1, glState.lightmap_textures + lmtex);
	
	if (glowLayer) 
	{
		for (i=0; i<rb_vertex; i++) // copy texture coords
			VA_SetElem2(texCoordArray[2][i], texCoordArray[0][i][0], texCoordArray[0][i][1]);
		GL_EnableTexture (2);
		GL_MBind (2, glow->texnum);
		if (!glConfig.mtexcombine) // if we've got > 2 TMUs, this can't be the case, right?
			GL_TexEnv (GL_ADD);
		else
		{
			qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			qglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_ADD);
			qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
			qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
		//	qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_CONSTANT_ARB);
			qglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
			qglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
		//	qglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_SRC_ALPHA);
			if (alpha < 1.0f)
				qglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
			else
				qglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_ADD);
			qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
			qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_PREVIOUS_ARB);
		//	qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB, GL_CONSTANT_ARB);
			qglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
			qglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA);
		//	qglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, GL_SRC_ALPHA);
		}
	}

	RB_DrawArrays ();

	GL_Disable (GL_ALPHA_TEST); // Alpha test flag

	if (glowLayer) 
		GL_DisableTexture (2);

	if (glowPass || envMap || causticPass)
		GL_DisableTexture (1);

	if (glowPass) // just redraw with existing arrays for glow
		RB_DrawTexGlow (glow);

	if (envMap && !causticPass)
	{
		for (i=0; i<rb_vertex; i++) 
			colorArray[i][3] = alpha*0.20;
		RB_DrawEnvMap ();
		for (i=0; i<rb_vertex; i++) 
			colorArray[i][3] = alpha;
	}

	if (causticPass) // Barnes caustics
		RB_DrawCaustics (surf);

	if (envMap || glowPass || causticPass)
		GL_EnableTexture (1);

	RB_DrawMeshTris ();
	rb_vertex = rb_index = 0;
}


/*
===========================================
R_DrawLightmappedSurface
===========================================
*/
void R_DrawLightmappedSurface (msurface_t *surf, qboolean render)
{
	glpoly_t	*p;
	int			nv, i;
	float		*v, scroll, alpha;

	c_brush_surfs++;

	if (surf->texinfo->flags & (SURF_TRANS33|SURF_TRANS66))
		alpha = (surf->entity && (surf->entity->flags & RF_TRANSLUCENT)) ? surf->entity->alpha : 1.0;
	else
		alpha = (currententity && (currententity->flags & RF_TRANSLUCENT)) ? currententity->alpha : 1.0;
	alpha *= SurfAlphaCalc (surf->texinfo->flags);

	if (surf->texinfo->flags & SURF_FLOWING) {
		scroll = -64 * ((r_newrefdef.time / 40.0) - (int)(r_newrefdef.time / 40.0));
		if (scroll == 0.0) scroll = -64.0;
	}
	else
		scroll = 0.0;

//	rb_vertex = rb_index = 0;
	for (p = surf->polys; p; p = p->chain)
	{
		nv = p->numverts;
		c_brush_polys += (nv-2);
		v = p->verts[0];
		if (RB_CheckArrayOverflow (nv, (nv-2)*3))
			RB_RenderLightmappedSurface (surf);
		for (i=0; i < nv-2; i++) {
			indexArray[rb_index++] = rb_vertex;
			indexArray[rb_index++] = rb_vertex+i+1;
			indexArray[rb_index++] = rb_vertex+i+2;
		}
		for (i=0; i < nv; i++, v+= VERTEXSIZE)
		{
			VA_SetElem2(inTexCoordArray[rb_vertex], v[3], v[4]);
			VA_SetElem2(texCoordArray[0][rb_vertex], (v[3]+scroll), v[4]);
			VA_SetElem2(texCoordArray[1][rb_vertex], v[5], v[6]);
			VA_SetElem3(vertexArray[rb_vertex], v[0], v[1], v[2]);
			VA_SetElem4(colorArray[rb_vertex], 1, 1, 1, alpha);
			rb_vertex++;
		}
	}

	if (render)
		RB_RenderLightmappedSurface (surf);
}


#if 0
/*
=================
SurfInFront
Returns true if surf1 is in front of surf2

FIXME- need to find a better way to sort trans surfaces
like an algorithm that uses psurf->extents and psurf->plane->normal
relative to vieworigin and takes into account e's offset and angles
=================
*/
qboolean SurfInFront (msurface_t *surf1, msurface_t *surf2)
{
	float dist1, dist2;
	vec3_t org1, org2;

	if (!r_trans_surf_sorting->integer) // check if sorting disabled
		return true;

	if (!surf1->plane || !surf2->plane)
		return false;

	if (surf1->entity)
		VectorSubtract(r_newrefdef.vieworg, surf1->entity->origin, org1);
	else
		VectorCopy (r_newrefdef.vieworg, org1);

	if (surf2->entity)
		VectorSubtract(r_newrefdef.vieworg, surf2->entity->origin, org2);
	else
		VectorCopy (r_newrefdef.vieworg, org2);

	dist1 = DotProduct(org1, surf1->plane->normal) - surf1->plane->dist;
	dist2 = DotProduct(org2, surf2->plane->normal) - surf2->plane->dist;
		
	if (dist1 < dist2)
		return true;
	else
		return false;
	//return (surf2->plane->dist > surf1->plane->dist);
}
#endif

/*
=================
R_DrawInlineBModel
=================
*/
void R_DrawInlineBModel (entity_t *e, int causticflag)
{
	int			i, k;
	cplane_t	*pplane;
	float		dot;
	msurface_t	*psurf, *s;
	dlight_t	*lt;
	qboolean	duplicate;
	image_t		*image;

	psurf = &currentmodel->surfaces[currentmodel->firstmodelsurface];

	for (i=0; i<currentmodel->nummodelsurfaces; i++, psurf++)
	{
		// find which side of the face we are on
		pplane = psurf->plane;
		if ( pplane->type < 3 )
			dot = modelorg[pplane->type] - pplane->dist;
		else
			dot = DotProduct (modelorg, pplane->normal) - pplane->dist;
		// cull the polygon
		if (dot > BACKFACE_EPSILON)
			psurf->visframe = r_framecount;
	}

	// calculate dynamic lighting for bmodel
	if (!r_flashblend->integer)
	{
		lt = r_newrefdef.dlights;
		if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2])
		{
			vec3_t temp;
			vec3_t forward, right, up;
			AngleVectors (currententity->angles, forward, right, up);
			for (k=0; k<r_newrefdef.num_dlights; k++, lt++)
			{
				VectorSubtract (lt->origin, currententity->origin, temp);
				lt->origin[0] = DotProduct (temp, forward);
				lt->origin[1] = -DotProduct (temp, right);
				lt->origin[2] = DotProduct (temp, up);
				R_MarkLights (lt, k, currentmodel->nodes + currentmodel->firstnode);
				VectorAdd (temp, currententity->origin, lt->origin);
			}
		} 
		else
		{
			for (k=0; k<r_newrefdef.num_dlights; k++, lt++)
			{
				VectorSubtract (lt->origin, currententity->origin, lt->origin);
				R_MarkLights (lt, k, currentmodel->nodes + currentmodel->firstnode);
				VectorAdd (lt->origin, currententity->origin, lt->origin);
			}
		}
	}

#ifdef MULTITEXTURE_CHAINS
	if (!glConfig.multitexture)
#endif	// MULTITEXTURE_CHAINS
	{
		if (currententity->flags & RF_TRANSLUCENT)
		{
			GL_DepthMask (false);
			GL_TexEnv (GL_MODULATE);
			GL_Enable (GL_BLEND);
			GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	}	

	//
	// draw standard surfaces
	//
	R_SetLightingMode (e->flags); // set up texture combiners

	psurf = &currentmodel->surfaces[currentmodel->firstmodelsurface];

	for (i = 0; i < currentmodel->nummodelsurfaces; i++, psurf++)
	{
		// find which side of the node we are on
		pplane = psurf->plane;
		dot = DotProduct (modelorg, pplane->normal) - pplane->dist;

		// draw the polygon
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
			(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
		{
#ifdef BATCH_LM_UPDATES
			if ( glConfig.multitexture &&
				!(psurf->texinfo->flags & (SURF_SKY|SURF_DRAWTURB)) )
				R_UpdateSurfaceLightmap (psurf);
#endif
			psurf->entity = NULL;
			psurf->flags &= ~SURF_MASK_CAUSTIC; // clear old caustics
			if ( psurf->texinfo->flags & (SURF_TRANS33|SURF_TRANS66) )
			{	// add to the translucent chain
				// if bmodel is used by multiple entities, adding surface
				// to linked list more than once would result in an infinite loop
				duplicate = false;
				for (s = r_alpha_surfaces; s; s = s->texturechain)
					if (s == psurf)
					{	duplicate = true;	break;	}
				if (!duplicate) // Don't allow surface to be added twice (fixes hang)
				{
				#if 0
					msurface_t	*last = NULL;
					psurf->entity = e; // entity pointer to support movement
					for (s = r_alpha_surfaces; s; last = s, s = s->texturechain)
					{
						if (SurfInFront (s, psurf)) // s is in front of psurf
							break; // we know to insert here
					}
					if (last) { // if in front of at least one surface
						psurf->texturechain = s;
						last->texturechain = psurf;
					}
					else { // stuff in beginning of chain
						psurf->texturechain = r_alpha_surfaces;
						r_alpha_surfaces = psurf;
					}
				#else
					psurf->flags |= causticflag; // set caustics
					psurf->texturechain = r_alpha_surfaces;
					r_alpha_surfaces = psurf;
					psurf->entity = e; // entity pointer to support movement
				#endif
				}
			}
			else
			{
				image = R_TextureAnimation (psurf);
				if ( glConfig.multitexture && !(psurf->flags & SURF_DRAWTURB) )
				{
					psurf->flags |= causticflag; // set caustics
			#ifdef MULTITEXTURE_CHAINS
					psurf->texturechain = image->texturechain;
					image->texturechain = psurf;
			#else
					R_DrawLightmappedSurface (psurf, true);
			#endif	// MULTITEXTURE_CHAINS
				}
				else if ( glConfig.multitexture && (psurf->flags & SURF_DRAWTURB) )	// warp surface
				{ 
			#ifdef MULTITEXTURE_CHAINS
					psurf->texturechain = image->warp_texturechain;
					image->warp_texturechain = psurf;
			#else
					continue;
			#endif	// MULTITEXTURE_CHAINS
				}
				else // 2-pass mode
				{
					GL_EnableMultitexture (false);
					R_RenderBrushPoly (psurf);
					GL_EnableMultitexture (true);

					// 2-pass mode-specific stuff
					R_BlendLightmaps ();
					R_DrawTriangleOutlines ();
				}
			}

		}
	}

#ifndef MULTITEXTURE_CHAINS
	//
	// draw warp surfaces
	//
	psurf = &currentmodel->surfaces[currentmodel->firstmodelsurface];

	for (i = 0; i < currentmodel->nummodelsurfaces; i++, psurf++)
	{
		// find which side of the node we are on
		pplane = psurf->plane;
		dot = DotProduct (modelorg, pplane->normal) - pplane->dist;

		// draw the polygon
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
			(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
		{
			if ( psurf->texinfo->flags & (SURF_TRANS33|SURF_TRANS66) )
				continue;
			else if (!(psurf->flags & SURF_DRAWTURB)) // non-warp surface
				continue;
			else // warp surface
				R_DrawWarpPoly (psurf);			
		}
	}
#else	// MULTITEXTURE_CHAINS
	if (glConfig.multitexture)
	{
		if (currententity->flags & RF_TRANSLUCENT)
		{
			GL_DepthMask (false);
			GL_TexEnv (GL_MODULATE);
			GL_Enable (GL_BLEND);
			GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		R_DrawMultiTextureChains ();
	}
#endif	// MULTITEXTURE_CHAINS

	if (currententity->flags & RF_TRANSLUCENT)
	{
		GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GL_Disable (GL_BLEND);
		GL_TexEnv (GL_REPLACE);
		GL_DepthMask (true);
	}
}

/*
=================
R_DrawBrushModel
=================
*/
int CL_PMpointcontents (vec3_t point);
int CL_PMpointcontents2 (vec3_t point, model_t *ignore);
void R_DrawBrushModel (entity_t *e)
{
	vec3_t		mins, maxs, org;
	int			i, contents[9], contentsAND, contentsOR, causticflag = 0;
	qboolean	rotated, viewInWater;

	if (currentmodel->nummodelsurfaces == 0)
		return;

	currententity = e;
	glState.currenttextures[0] = glState.currenttextures[1] = -1;

	if (e->angles[0] || e->angles[1] || e->angles[2])
	{
		rotated = true;
		for (i=0; i<3; i++)
		{
			mins[i] = e->origin[i] - currentmodel->radius;
			maxs[i] = e->origin[i] + currentmodel->radius;
		}
	}
	else
	{
		rotated = false;
		VectorAdd (e->origin, currentmodel->mins, mins);
		VectorAdd (e->origin, currentmodel->maxs, maxs);
	}

	if (R_CullBox (mins, maxs))
		return;

	qglColor3f (1,1,1);
	memset (gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));

	VectorSubtract (r_newrefdef.vieworg, e->origin, modelorg);
	if (rotated)
	{
		vec3_t	temp;
		vec3_t	forward, right, up;

		VectorCopy (modelorg, temp);
		AngleVectors (e->angles, forward, right, up);
		modelorg[0] = DotProduct (temp, forward);
		modelorg[1] = -DotProduct (temp, right);
		modelorg[2] = DotProduct (temp, up);
	}

	// check for caustics, based on code by Berserker
	if (r_caustics->integer)
	{
		VectorSet(org, mins[0], mins[1], mins[2]);
	//	contents[0] = Mod_PointInLeaf(org, r_worldmodel)->contents;
		contents[0] = CL_PMpointcontents2 (org, currentmodel);
		VectorSet(org, maxs[0], mins[1], mins[2]);
		contents[1] = CL_PMpointcontents2 (org, currentmodel);
		VectorSet(org, mins[0], maxs[1], mins[2]);
		contents[2] = CL_PMpointcontents2 (org, currentmodel);
		VectorSet(org, maxs[0], maxs[1], mins[2]);
		contents[3] = CL_PMpointcontents2 (org, currentmodel);
		VectorSet(org, mins[0], mins[1], maxs[2]);
		contents[4] = CL_PMpointcontents2 (org, currentmodel);
		VectorSet(org, maxs[0], mins[1], maxs[2]);
		contents[5] = CL_PMpointcontents2 (org, currentmodel);
		VectorSet(org, mins[0], maxs[1], maxs[2]);
		contents[6] = CL_PMpointcontents2 (org, currentmodel);
		VectorSet(org, maxs[0], maxs[1], maxs[2]);
		contents[7] = CL_PMpointcontents2 (org, currentmodel);
		org[0] = (mins[0] + maxs[0]) * 0.5;
		org[1] = (mins[1] + maxs[1]) * 0.5;
		org[2] = (mins[2] + maxs[2]) * 0.5;
		contents[8] = CL_PMpointcontents2 (org, currentmodel);
		contentsAND = (contents[0]&contents[1]&contents[2]&contents[3]&contents[4]&contents[5]&contents[6]&contents[7]&contents[8]);
		contentsOR = (contents[0]|contents[1]|contents[2]|contents[3]|contents[4]|contents[5]|contents[6]|contents[7]|contents[8]);
	//	viewInWater = (Mod_PointInLeaf(r_newrefdef.vieworg, r_worldmodel)->contents & MASK_WATER);
		viewInWater = (CL_PMpointcontents(r_newrefdef.vieworg) & MASK_WATER);
		if ( (contentsAND & MASK_WATER) || ((contentsOR & MASK_WATER) && viewInWater) )
		{
			if (contentsOR & CONTENTS_LAVA)
				causticflag = SURF_UNDERLAVA;
			else if (contentsOR & CONTENTS_SLIME)
				causticflag = SURF_UNDERSLIME;
			else
				causticflag = SURF_UNDERWATER;
		}
	}

    qglPushMatrix ();
	R_RotateForEntity (e, true);

	GL_EnableMultitexture (true);
//	R_SetLightingMode (e->flags);

	R_DrawInlineBModel (e, causticflag);

	GL_EnableMultitexture (false);

	qglPopMatrix ();
}

/*
=============================================================

	WORLD MODEL

=============================================================
*/

/*
================
R_RecursiveWorldNode
================
*/
void R_RecursiveWorldNode (mnode_t *node)
{
	int			c, side, sidebit;
	cplane_t	*plane;
	msurface_t	*surf, **mark;
	mleaf_t		*pleaf;
	float		dot;
	image_t		*image;

	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	if (node->visframe != r_visframecount)
		return;
	if (R_CullBox (node->minmaxs, node->minmaxs+3))
		return;
	
	// if a leaf node, draw stuff
	if (node->contents != -1)
	{
		pleaf = (mleaf_t *)node;

		// check for door connected areas
		if (r_newrefdef.areabits)
		{
			if (! (r_newrefdef.areabits[pleaf->area>>3] & (1<<(pleaf->area&7)) ) )
				return;		// not visible
		}

		mark = pleaf->firstmarksurface;
		c = pleaf->nummarksurfaces;

		if (c)
		{
			do
			{
				(*mark)->visframe = r_framecount;
				mark++;
			} while (--c);
		}

		return;
	}

	// node is just a decision point, so go down the apropriate sides

	// find which side of the node we are on
	plane = node->plane;

	switch (plane->type)
	{
	case PLANE_X:
		dot = modelorg[0] - plane->dist;
		break;
	case PLANE_Y:
		dot = modelorg[1] - plane->dist;
		break;
	case PLANE_Z:
		dot = modelorg[2] - plane->dist;
		break;
	default:
		dot = DotProduct (modelorg, plane->normal) - plane->dist;
		break;
	}

	if (dot >= 0)
	{
		side = 0;
		sidebit = 0;
	}
	else
	{
		side = 1;
		sidebit = SURF_PLANEBACK;
	}

	// recurse down the children, front side first
	R_RecursiveWorldNode (node->children[side]);

	// draw stuff
	for ( c = node->numsurfaces, surf = r_worldmodel->surfaces + node->firstsurface; c ; c--, surf++)
	{
		if (surf->visframe != r_framecount)
			continue;

		if ((surf->flags & SURF_PLANEBACK) != sidebit)
			continue;		// wrong side

		surf->entity = NULL;

#ifdef BATCH_LM_UPDATES
		if ( glConfig.multitexture && 
			!(surf->texinfo->flags & (SURF_SKY|SURF_DRAWTURB)) )
			R_UpdateSurfaceLightmap (surf);
#endif

		if (surf->texinfo->flags & SURF_SKY)
		{	// just adds to visible sky bounds
			R_AddSkySurface (surf);
		}
		else if (surf->texinfo->flags & (SURF_TRANS33|SURF_TRANS66))
		{	// add to the translucent chain
			surf->texturechain = r_alpha_surfaces;
			r_alpha_surfaces = surf;
		}
#ifndef MULTITEXTURE_CHAINS
		else if (glConfig.multitexture && !(surf->flags & SURF_DRAWTURB))
		{	
			R_DrawLightmappedSurface (surf, true);
		}
#endif // MULTITEXTURE_CHAINS
		else
		{
			// the polygon is visible, so add it to the texture chain
			image = R_TextureAnimation (surf);
			if ( !(surf->flags & SURF_DRAWTURB) ) {
				surf->texturechain = image->texturechain;
				image->texturechain = surf;
			}
			else {
				surf->texturechain = image->warp_texturechain;
				image->warp_texturechain = surf;
			}
		}
	}

	// recurse down the back side
	R_RecursiveWorldNode (node->children[!side]);
}


/*
=============
R_DrawWorld
=============
*/
void R_DrawWorld (void)
{
	entity_t	ent;

	if (!r_drawworld->integer)
		return;

	if ( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
		return;

	currentmodel = r_worldmodel;

	VectorCopy (r_newrefdef.vieworg, modelorg);

	// auto cycle the world frame for texture animation
	memset (&ent, 0, sizeof(ent));
	// Knightmare added r_worldframe for trans animations
	ent.frame = r_worldframe = (int)(r_newrefdef.time*2); 
	currententity = &ent;

	glState.currenttextures[0] = glState.currenttextures[1] = -1;

	qglColor3f (1,1,1);
	memset (gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));
	R_ClearSkyBox ();

	if (glConfig.multitexture)
	{
#ifndef MULTITEXTURE_CHAINS
		GL_EnableMultitexture (true);
		R_SetLightingMode (0);
#endif // MULTITEXTURE_CHAINS

		R_RecursiveWorldNode (r_worldmodel->nodes);

#ifndef MULTITEXTURE_CHAINS
		GL_EnableMultitexture (false);
#endif // MULTITEXTURE_CHAINS

		R_DrawMultiTextureChains ();	// draw solid warp surfaces
	}
	else
	{	// add surfaces to texture chains for 2-pass rendering
		R_RecursiveWorldNode (r_worldmodel->nodes);

		R_DrawTextureChains ();

		R_BlendLightmaps ();

		R_DrawTriangleOutlines ();
	}

	R_DrawSkyBox ();
}


/*
===============
R_MarkLeaves

Mark the leaves and nodes that are in the PVS for the current
cluster
===============
*/
void R_MarkLeaves (void)
{
	byte	*vis;
	byte	fatvis[MAX_MAP_LEAFS/8];
	mnode_t	*node;
	int		i, c;
	mleaf_t	*leaf;
	int		cluster;

	if (r_oldviewcluster == r_viewcluster && r_oldviewcluster2 == r_viewcluster2
		&& !r_novis->integer && r_viewcluster != -1)
		return;

	// development aid to let you run around and see exactly where
	// the pvs ends
	if (r_lockpvs->integer)
		return;

	if (!r_worldmodel)	// Knightmare- potential crash fix
		return;

	r_visframecount++;
	r_oldviewcluster = r_viewcluster;
	r_oldviewcluster2 = r_viewcluster2;

	if (r_novis->integer || r_viewcluster == -1 || !r_worldmodel->vis)
	{
		// mark everything
		for (i=0 ; i<r_worldmodel->numleafs ; i++)
			r_worldmodel->leafs[i].visframe = r_visframecount;
		for (i=0 ; i<r_worldmodel->numnodes ; i++)
			r_worldmodel->nodes[i].visframe = r_visframecount;
		return;
	}

	vis = Mod_ClusterPVS (r_viewcluster, r_worldmodel);
	// may have to combine two clusters because of solid water boundaries
	if (r_viewcluster2 != r_viewcluster)
	{
		memcpy (fatvis, vis, (r_worldmodel->numleafs+7)/8);
		vis = Mod_ClusterPVS (r_viewcluster2, r_worldmodel);
		c = (r_worldmodel->numleafs+31)/32;
		for (i=0 ; i<c ; i++)
			((int *)fatvis)[i] |= ((int *)vis)[i];
		vis = fatvis;
	}
	
	for (i=0,leaf=r_worldmodel->leafs ; i<r_worldmodel->numleafs ; i++, leaf++)
	{
		cluster = leaf->cluster;
		if (cluster == -1)
			continue;
		if (vis[cluster>>3] & (1<<(cluster&7)))
		{
			node = (mnode_t *)leaf;
			do
			{
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}

#if 0
	for (i=0 ; i<r_worldmodel->vis->numclusters ; i++)
	{
		if (vis[i>>3] & (1<<(i&7)))
		{
			node = (mnode_t *)&r_worldmodel->leafs[i];	// FIXME: cluster
			do
			{
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}
#endif
}


/*
=======================================================================

	Quake2Max vertex lighting code

=======================================================================
*/

/*
=================
R_BuildVertexLightBase
=================
*/
void R_SurfLightPoint (msurface_t *surf, vec3_t p, vec3_t color, qboolean baselight);
qboolean R_BuildVertexLightBase (msurface_t *surf, glpoly_t *poly)
{
	vec3_t		color, point;
	int			i, j;
	float		*v;
	qboolean	lit = false;

	for (i=0, v=poly->verts[0]; i<poly->numverts; i++, v+=VERTEXSIZE)
	{
		VectorCopy(v, point); // lerp outward away from plane to avoid dark spots?
		// lerp between each vertex and origin - use check for too dark?
		// this messes up curved glass surfaces
		//VectorSubtract (poly->center, v, point);
		//VectorMA(v, 0.01, point, point);

		R_SurfLightPoint (surf, point, color, true);
			
		R_MaxColorVec (color);
		for (j=0; j<3; j++)
			if (color[j] > 0.0f)
				lit = true;

		poly->vertexlightbase[i*3+0] = (byte)(color[0]*255.0);
		poly->vertexlightbase[i*3+1] = (byte)(color[1]*255.0);
		poly->vertexlightbase[i*3+2] = (byte)(color[2]*255.0);
	}
	return lit;
}


/*
=================
R_ResetVertextLight
=================
*/
void R_ResetVertextLight (msurface_t *surf)
{
	glpoly_t	*poly;

	if (!surf->polys)
		return;

	for (poly=surf->polys; poly; poly=poly->next)
		poly->vertexlightset = false;
}


/*
=================
R_BuildVertexLight
=================
*/
void R_BuildVertexLight (msurface_t *surf)
{
	vec3_t		color, point;
	int			i;
	float		*v;
	glpoly_t	*poly;

	if (surf->flags & SURF_DRAWTURB)
	{	if (!r_warp_lighting->integer)	return;	}
	else
	{	if (!r_trans_lighting->integer)	return;	}

	if (!surf->polys)
		return;

	for (poly=surf->polys; poly; poly=poly->next)
	{
		if (!poly->vertexlight || !poly->vertexlightbase)
			continue;

		if (!poly->vertexlightset)
		{	
			R_BuildVertexLightBase(surf, poly);
			poly->vertexlightset = true;
		//	if (R_BuildVertexLightBase(surf, poly))
		//		poly->vertexlightset = true;
		//	else
		//		return; // don't bother if lightbase is all black
		}

		for (i=0, v=poly->verts[0]; i<poly->numverts; i++, v+=VERTEXSIZE)
		{
			VectorCopy(v, point); // lerp outward away from plane to avoid dark spots?
			// lerp between each vertex and origin - use check for too dark?
			// this messes up curved glass surfaces
			//VectorSubtract (poly->center, v, point);
			//VectorMA(v, 0.01, point, point);

			R_SurfLightPoint (surf, point, color, false);

			VectorSet(color,
				(float)poly->vertexlightbase[i*3+0]/255.0 + color[0],
				(float)poly->vertexlightbase[i*3+1]/255.0 + color[1],
				(float)poly->vertexlightbase[i*3+2]/255.0 + color[2]);
				
			R_MaxColorVec (color);

			poly->vertexlight[i*3+0] = (byte)(color[0]*255.0);
			poly->vertexlight[i*3+1] = (byte)(color[1]*255.0);
			poly->vertexlight[i*3+2] = (byte)(color[2]*255.0);
		}
	}
}

/*
=======================================================================
	end Quake2Max vertex lighting code
=======================================================================
*/


/*
=============================================================================

  LIGHTMAP ALLOCATION

=============================================================================
*/

/*
================
LM_InitBlock
================
*/
static void LM_InitBlock (void)
{
	memset( gl_lms.allocated, 0, sizeof( gl_lms.allocated ) );

#ifdef BATCH_LM_UPDATES
	// alloc lightmap update buffer if needed
	if (!gl_lms.lightmap_update[gl_lms.current_lightmap_texture]) {
		gl_lms.lightmap_update[gl_lms.current_lightmap_texture] = Z_Malloc (LM_BLOCK_WIDTH*LM_BLOCK_HEIGHT*LIGHTMAP_BYTES);
	}
#endif	// BATCH_LM_UPDATES
}


/*
================
LM_UploadBlock
================
*/
static void LM_UploadBlock (qboolean dynamic)
{
	int texture;
	int height = 0;

	if ( dynamic )
	{
		texture = 0;
	}
	else
	{
		texture = gl_lms.current_lightmap_texture;
	}

	GL_Bind( glState.lightmap_textures + texture );
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if ( dynamic )
	{
		int i;

		for ( i = 0; i < LM_BLOCK_WIDTH; i++ )
		{
			if ( gl_lms.allocated[i] > height )
				height = gl_lms.allocated[i];
		}

		qglTexSubImage2D( GL_TEXTURE_2D, 
						  0,
						  0, 0,
						  LM_BLOCK_WIDTH, height,
						  gl_lms.format,
						  gl_lms.type,
	//					  GL_LIGHTMAP_FORMAT,
	//					  GL_UNSIGNED_BYTE,
						  gl_lms.lightmap_buffer );
	}
	else
	{
		qglTexImage2D( GL_TEXTURE_2D, 
					   0, 
					   gl_lms.internal_format,
					   LM_BLOCK_WIDTH, LM_BLOCK_HEIGHT, 
					   0, 
					   gl_lms.format,
					   gl_lms.type,
	//				   GL_LIGHTMAP_FORMAT, 
	//				   GL_UNSIGNED_BYTE, 
#ifdef BATCH_LM_UPDATES
					   gl_lms.lightmap_update[gl_lms.current_lightmap_texture] );
#else
					   gl_lms.lightmap_buffer );
#endif	// BATCH_LM_UPDATES
		if ( ++gl_lms.current_lightmap_texture == MAX_LIGHTMAPS )
			VID_Error( ERR_DROP, "LM_UploadBlock() - MAX_LIGHTMAPS exceeded\n" );
	}
}


/*
================
LM_AllocBlock
returns a texture number and the position inside it
================
*/
static qboolean LM_AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;

	best = LM_BLOCK_HEIGHT;

	for (i=0 ; i<LM_BLOCK_WIDTH-w ; i++)
	{
		best2 = 0;

		for (j=0 ; j<w ; j++)
		{
			if (gl_lms.allocated[i+j] >= best)
				break;
			if (gl_lms.allocated[i+j] > best2)
				best2 = gl_lms.allocated[i+j];
		}
		if (j == w)
		{	// this is a valid spot
			*x = i;
			*y = best = best2;
		}
	}

	if (best + h > LM_BLOCK_HEIGHT)
		return false;

	for (i=0 ; i<w ; i++)
		gl_lms.allocated[*x + i] = best + h;

	return true;
}


/*
================
R_BuildPolygonFromSurface
================
*/
void R_BuildPolygonFromSurface (msurface_t *fa)
{
	int			i, lindex, lnumverts;
	medge_t		*pedges, *r_pedge;
	int			vertpage;
	float		*vec;
	float		s, t;
	glpoly_t	*poly;
	vec3_t		total;

// reconstruct the polygon
	pedges = currentmodel->edges;
	lnumverts = fa->numedges;
	vertpage = 0;

	VectorClear (total);

	//
	// draw texture
	//
	poly = Hunk_Alloc (sizeof(glpoly_t) + (lnumverts-4) * VERTEXSIZE*sizeof(float));
	poly->next = fa->polys;
	poly->flags = fa->flags;
	fa->polys = poly;
	poly->numverts = lnumverts;

	// alloc vertex light fields
	if (fa->texinfo->flags & (SURF_TRANS33|SURF_TRANS66)) {
		int size = lnumverts*3*sizeof(byte);
		poly->vertexlight = Hunk_Alloc(size);
		poly->vertexlightbase = Hunk_Alloc(size);
		memset(poly->vertexlight, 0, size);
		memset(poly->vertexlightbase, 0, size);
		poly->vertexlightset = false;
	}

	for (i=0; i<lnumverts; i++)
	{
		lindex = currentmodel->surfedges[fa->firstedge + i];

		if (lindex > 0)
		{
			r_pedge = &pedges[lindex];
			vec = currentmodel->vertexes[r_pedge->v[0]].position;
		}
		else
		{
			r_pedge = &pedges[-lindex];
			vec = currentmodel->vertexes[r_pedge->v[1]].position;
		}
		//
		// texture coordinates
		//
		s = DotProduct (vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s /= fa->texinfo->texWidth; //fa->texinfo->image->width; changed to Q2E hack

		t = DotProduct (vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t /= fa->texinfo->texHeight; //fa->texinfo->image->height; changed to Q2E hack
		
		VectorAdd (total, vec, total);
		VectorCopy (vec, poly->verts[i]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;

		//
		// lightmap texture coordinates
		//
		s = DotProduct (vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s -= fa->texturemins[0];
		s += fa->light_s*16;
		s += 8;
		s /= LM_BLOCK_WIDTH*16; //fa->texinfo->texture->width;

		t = DotProduct (vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t -= fa->texturemins[1];
		t += fa->light_t*16;
		t += 8;
		t /= LM_BLOCK_HEIGHT*16; //fa->texinfo->texture->height;

		poly->verts[i][5] = s;
		poly->verts[i][6] = t;
	}
	VectorScale(total, 1.0/(float)lnumverts, poly->center); // for vertex lighting

	poly->numverts = lnumverts;
}


/*
========================
R_CreateSurfaceLightmap
========================
*/
void R_CreateSurfaceLightmap (msurface_t *surf)
{
	int			smax, tmax;
	unsigned	*base;

	if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
		return;

	//if (surf->texinfo->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP))
	if (surf->texinfo->flags & (SURF_SKY|SURF_WARP))
		return;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;

	if ( !LM_AllocBlock (smax, tmax, &surf->light_s, &surf->light_t) )
	{
		LM_UploadBlock (false);
		LM_InitBlock();
		if ( !LM_AllocBlock (smax, tmax, &surf->light_s, &surf->light_t) )
		{
			VID_Error (ERR_FATAL, "Consecutive calls to LM_AllocBlock(%d,%d) failed\n", smax, tmax);
		}
	}

	surf->lightmaptexturenum = gl_lms.current_lightmap_texture;

	// copy extents
	surf->light_smax = smax;
	surf->light_tmax = tmax;

#ifdef BATCH_LM_UPDATES
	base = gl_lms.lightmap_update[surf->lightmaptexturenum];
#else
	base = gl_lms.lightmap_buffer;
#endif	// BATCH_LM_UPDATES

	base += (surf->light_t * LM_BLOCK_WIDTH + surf->light_s);	// * LIGHTMAP_BYTES

	R_SetCacheState (surf);
	R_BuildLightMap (surf, (void *)base, LM_BLOCK_WIDTH*LIGHTMAP_BYTES);
}


/*
==================
R_BeginBuildingLightmaps
==================
*/
void R_BeginBuildingLightmaps (model_t *m)
{
	static lightstyle_t	lightstyles[MAX_LIGHTSTYLES];
	int				i;
	unsigned		dummy[LM_BLOCK_WIDTH*LM_BLOCK_HEIGHT];	// 128*128

	memset( gl_lms.allocated, 0, sizeof(gl_lms.allocated) );

#ifdef BATCH_LM_UPDATES
	// free lightmap update buffers
	for (i=0; i<MAX_LIGHTMAPS; i++)
	{
		if (gl_lms.lightmap_update[i])
			Z_Free(gl_lms.lightmap_update[i]);
		gl_lms.lightmap_update[i] = NULL;
		gl_lms.modified[i] = false;
		gl_lms.lightrect[i].left = LM_BLOCK_WIDTH;
		gl_lms.lightrect[i].right = 0;
		gl_lms.lightrect[i].top = LM_BLOCK_HEIGHT;
		gl_lms.lightrect[i].bottom = 0;
	}
#endif	// BATCH_LM_UPDATES

	r_framecount = 1;		// no dlightcache

	GL_EnableMultitexture (true);
	GL_SelectTexture(1);

	// setup the base lightstyles so the lightmaps won't have to be regenerated
	// the first time they're seen
	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		lightstyles[i].rgb[0] = 1;
		lightstyles[i].rgb[1] = 1;
		lightstyles[i].rgb[2] = 1;
		lightstyles[i].white = 3;
	}
	r_newrefdef.lightstyles = lightstyles;

	if (!glState.lightmap_textures)
	{
		glState.lightmap_textures	= TEXNUM_LIGHTMAPS;
	//	glState.lightmap_textures	= glState.texture_extension_number;
	//	glState.texture_extension_number = glState.lightmap_textures + MAX_LIGHTMAPS;
	}

	gl_lms.current_lightmap_texture = 1;

#ifdef BATCH_LM_UPDATES
	// alloc lightmap update buffer if needed
	if (!gl_lms.lightmap_update[gl_lms.current_lightmap_texture]) {
		gl_lms.lightmap_update[gl_lms.current_lightmap_texture] = Z_Malloc (LM_BLOCK_WIDTH*LM_BLOCK_HEIGHT*LIGHTMAP_BYTES);
	}
#endif	// BATCH_LM_UPDATES


	/*
	** if mono lightmaps are enabled and we want to use alpha
	** blending (a,1-a) then we're likely running on a 3DLabs
	** Permedia2.  In a perfect world we'd use a GL_ALPHA lightmap
	** in order to conserve space and maximize bandwidth, however 
	** this isn't a perfect world.
	**
	** So we have to use alpha lightmaps, but stored in GL_RGBA format,
	** which means we only get 1/16th the color resolution we should when
	** using alpha lightmaps.  If we find another board that supports
	** only alpha lightmaps but that can at least support the GL_ALPHA
	** format then we should change this code to use real alpha maps.
	*/

	// Knightmare- old internal formats for compatibility with older GPUs/drivers
	if ( !glConfig.newLMFormat )
	{
		if ( toupper( r_monolightmap->string[0] ) == 'A' )
		{
			gl_lms.internal_format = gl_tex_alpha_format;
		}
		// try to do hacked colored lighting with a blended texture
		else if ( toupper( r_monolightmap->string[0] ) == 'C' )
		{
			gl_lms.internal_format = gl_tex_alpha_format;
		}
		else if ( toupper( r_monolightmap->string[0] ) == 'I' )
		{
			gl_lms.internal_format = GL_INTENSITY8;
		}
		else if ( toupper( r_monolightmap->string[0] ) == 'L' ) 
		{
			gl_lms.internal_format = GL_LUMINANCE8;
		}
		else
		{
			gl_lms.internal_format = gl_tex_solid_format;
		}

		gl_lms.format = GL_RGBA;
		gl_lms.type = GL_UNSIGNED_BYTE;
	}
	else
	{
		if ( toupper( r_monolightmap->string[0] ) == 'I' )
			gl_lms.internal_format = GL_INTENSITY8;
		else if ( toupper( r_monolightmap->string[0] ) == 'L' ) 
			gl_lms.internal_format = GL_LUMINANCE8;
		else
			gl_lms.internal_format = GL_RGBA8;

		gl_lms.format = GL_BGRA;
		gl_lms.type = GL_UNSIGNED_INT_8_8_8_8_REV;
	}

	// initialize the dynamic lightmap texture
	GL_Bind( glState.lightmap_textures + 0 );
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexImage2D( GL_TEXTURE_2D, 
				   0, 
				   gl_lms.internal_format,
				   LM_BLOCK_WIDTH, LM_BLOCK_HEIGHT, 
				   0, 
				   gl_lms.format,
				   gl_lms.type,
	//			   GL_LIGHTMAP_FORMAT, 
	//			   GL_UNSIGNED_BYTE, 
				   dummy );
}


/*
=======================
R_EndBuildingLightmaps
=======================
*/
void R_EndBuildingLightmaps (void)
{
	LM_UploadBlock (false);
	GL_EnableMultitexture (false);
}

