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
// GL_RSURF.C: surface-related refresh code
#include <assert.h>

#include "r_local.h"

static vec3_t	modelorg;		// relative to viewpoint

msurface_t	*r_alpha_surfaces;

#define DYNAMIC_LIGHT_WIDTH  128
#define DYNAMIC_LIGHT_HEIGHT 128

#define LIGHTMAP_BYTES 4

#define	BLOCK_WIDTH		128
#define	BLOCK_HEIGHT	128



int		c_visible_lightmaps;
int		c_visible_textures;

#define GL_LIGHTMAP_FORMAT GL_RGBA

typedef struct
{
	int internal_format;
	int	current_lightmap_texture;

	msurface_t	*lightmap_surfaces[MAX_LIGHTMAPS];

	int			allocated[BLOCK_WIDTH];

	// the lightmap texture data needs to be kept in
	// main memory so texsubimage can update properly
	byte		lightmap_buffer[4*BLOCK_WIDTH*BLOCK_HEIGHT];
} gllightmapstate_t;

static gllightmapstate_t gl_lms;


static void		LM_InitBlock( void );
static void		LM_UploadBlock( qboolean dynamic );
static qboolean	LM_AllocBlock (int w, int h, int *x, int *y);

extern void R_SetCacheState( msurface_t *surf );
extern void R_BuildLightMap (msurface_t *surf, byte *dest, int stride);

/*
=============================================================

	BRUSH MODELS

=============================================================
*/

/*
===============
R_TextureAnimation

Returns the proper texture for a given time and base texture
===============
*/
image_t *R_TextureAnimation (mtexinfo_t *tex)
{
	int		c;

	if (!tex->next)
		return tex->image;

	c = currententity->frame % tex->numframes;
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

Returns the proper glow texture for a given time and base texture
===============
*/
image_t *R_TextureAnimationGlow (mtexinfo_t *tex)
{
	int		c;

	if (!tex->next)
		return tex->glow;

	c = currententity->frame % tex->numframes;
	while (c)
	{
		tex = tex->next;
		c--;
	}

	return tex->glow;
}

/*
===============
R_TextureAnimationNew

Variant of the above for alpha surfaces
Uses msurface_t entity pointer, since currententity
is not valid for the alpha surface pass
===============
*/
image_t *R_TextureAnimationNew (msurface_t *surf)
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
R_SetLightingMode
===============
*/
void R_SetLightingMode (int renderflags)
{
	GL_SelectTexture (0);

	if (!gl_config.mtexcombine || (renderflags & RF_TRANSLUCENT)) 
	{
		GL_TexEnv (GL_REPLACE);
		GL_SelectTexture (1);

		if (r_lightmap->value)
			GL_TexEnv (GL_REPLACE);
		else 
			GL_TexEnv (GL_MODULATE);
	}
	else 
	{
		GL_TexEnv (GL_COMBINE_EXT);
		qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
		qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
		qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
		qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE);

		GL_SelectTexture (1);
		GL_TexEnv (GL_COMBINE_EXT);
		if (r_lightmap->value) 
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

		if (r_overbrightbits->value)
		{
			qglTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, r_overbrightbits->value);
		}
	}
}


/*
================
R_DrawGLPoly
modified to handle scrolling textures
================
*/
void R_DrawGLPoly (msurface_t *fa, float alpha, qboolean light, qboolean envmap)
{
	int			i;
	float		*v, scroll;
	glpoly_t	*p = fa->polys;
	image_t		*image = R_TextureAnimationNew (fa);

	GL_Bind (image->texnum);

	if (light) {
		R_SetVertexOverbrights (true);
		GL_ShadeModel (GL_SMOOTH);
	}

	if (fa->texinfo->flags & SURF_FLOWING) {
		scroll = -64 * ( (r_newrefdef.time / 40.0) - (int)(r_newrefdef.time / 40.0) );
		if (scroll == 0.0)	scroll = -64.0;
	}
	else
		scroll = 0.0;

	rb_vertex = rb_index = 0;
	v = p->verts[0];
	for (i=0; i<p->numverts; i++, v+= VERTEXSIZE)
	{
		if (light && p->vertexlight && p->vertexlightset)
			VA_SetElem4(colorArray[rb_vertex],
				(float)(p->vertexlight[i*3+0]*DIV255),
				(float)(p->vertexlight[i*3+1]*DIV255),
				(float)(p->vertexlight[i*3+2]*DIV255), alpha);
		else
			VA_SetElem4(colorArray[rb_vertex], gl_state.inverse_intensity, gl_state.inverse_intensity, gl_state.inverse_intensity, alpha);

		VA_SetElem2(texCoordArray[0][rb_vertex], v[3]+scroll, v[4]);
		VA_SetElem3(vertexArray[rb_vertex], v[0], v[1], v[2]);
		rb_vertex++;
	}

	for (i=0; i < p->numverts-2; i++) {
		indexArray[rb_index++] = 0;
		indexArray[rb_index++] = 0+i+1;
		indexArray[rb_index++] = 0+i+2;
	}
	RB_DrawArrays (GL_TRIANGLES);

	if (envmap)
	{
		float	envAlpha;
		// lightmapped trans surfaces have more solid envmapping
		if (r_trans_lightmaps->value && !(fa->texinfo->flags & SURF_NOLIGHTENV))
			envAlpha = 0.15;
		else envAlpha = 0.10;

		for (i=0; i<p->numverts; i++) 
			colorArray[i][3] = envAlpha;

		GL_Bind (r_envmappic->texnum);

		qglTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		qglTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

		qglEnable(GL_TEXTURE_GEN_S);
		qglEnable(GL_TEXTURE_GEN_T);

		RB_DrawArrays (GL_TRIANGLES);
		
		qglDisable(GL_TEXTURE_GEN_S);
		qglDisable(GL_TEXTURE_GEN_T);
	}

	if (light) {
		R_SetVertexOverbrights(false);
		GL_ShadeModel (GL_FLAT);
	}

	RB_DrawMeshTris (GL_TRIANGLES, 1);
}


/*
================
R_DrawTriangleOutlines
================
*/
void R_DrawTriangleOutlines (void)
{
	int			i, j;
	msurface_t	*surf;
	glpoly_t	*p;

	if (!r_showtris->value)
		return;

	if (r_showtris->value == 1)
		GL_Disable(GL_DEPTH_TEST);

	GL_DisableTexture (0);
	//qglDisable(GL_TEXTURE_2D);

	for (i = 0; i < MAX_LIGHTMAPS; i++)
	{
		for (surf = gl_lms.lightmap_surfaces[i]; surf != 0; surf = surf->lightmapchain)
		{
			for (p = surf->polys; p; p = p->chain)
			{
				for (j = 2; j < p->numverts; j++)
				{
                    qglBegin(GL_LINE_STRIP);
                        qglColor4f(1, 1, 1, 1);
                        qglVertex3fv(p->verts[0]);
                        qglVertex3fv(p->verts[j-1]);
                        qglVertex3fv(p->verts[j]);
                        qglVertex3fv(p->verts[0]);
                    qglEnd();
				}
			}
		}
	}

	GL_EnableTexture(0);
	//qglEnable(GL_TEXTURE_2D);

	if (r_showtris->value == 1)
		GL_Enable(GL_DEPTH_TEST);
}


/*
================
R_DrawGLPolyChain
================
*/
void R_DrawGLPolyChain (glpoly_t *p, float soffset, float toffset)
{
	if (soffset == 0 && toffset == 0)
	{
		for ( ; p != 0; p = p->chain)
		{
			float *v;
			int j;

			qglBegin (GL_POLYGON);
			v = p->verts[0];
			for (j=0 ; j<p->numverts ; j++, v+= VERTEXSIZE)
			{
				qglTexCoord2f (v[5], v[6] );
				qglVertex3fv (v);
			}
			qglEnd ();
		}
	}
	else
	{
		for ( ; p != 0; p = p->chain)
		{
			float *v;
			int j;

			qglBegin (GL_POLYGON);
			v = p->verts[0];
			for (j=0 ; j<p->numverts ; j++, v+= VERTEXSIZE)
			{
				qglTexCoord2f (v[5] - soffset, v[6] - toffset );
				qglVertex3fv (v);
			}
			qglEnd ();
		}
	}
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

	// don't bother if we're set to fullbright
	if (r_fullbright->value)
		return;
	if (!r_worldmodel->lightdata)
		return;

	// don't bother writing Z
	GL_DepthMask (false);

	// set the appropriate blending mode unless we're only looking at the
	// lightmaps.
	if (!r_lightmap->value)
	{
		GL_Enable (GL_BLEND);

		if (r_saturatelighting->value)
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
			GL_Bind( gl_state.lightmap_textures + i);

			for (surf = gl_lms.lightmap_surfaces[i]; surf != 0; surf = surf->lightmapchain)
			{
				if (surf->polys)
					R_DrawGLPolyChain(surf->polys, 0, 0);
			}
		}
	}

	// render dynamic lightmaps
	if (r_dynamic->value)
	{
		LM_InitBlock();

		GL_Bind(gl_state.lightmap_textures+0);

		if (currentmodel == r_worldmodel)
			c_visible_lightmaps++;

		newdrawsurf = gl_lms.lightmap_surfaces[0];

		for (surf = gl_lms.lightmap_surfaces[0]; surf != 0; surf = surf->lightmapchain)
		{
			int		smax, tmax;
			byte	*base;

			smax = (surf->extents[0]>>4)+1;
			tmax = (surf->extents[1]>>4)+1;

			if ( LM_AllocBlock( smax, tmax, &surf->dlight_s, &surf->dlight_t ) )
			{
				base = gl_lms.lightmap_buffer;
				base += (surf->dlight_t * BLOCK_WIDTH + surf->dlight_s) * LIGHTMAP_BYTES;

				R_BuildLightMap (surf, base, BLOCK_WIDTH*LIGHTMAP_BYTES);
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
				base += (surf->dlight_t * BLOCK_WIDTH + surf->dlight_s) * LIGHTMAP_BYTES;

				R_BuildLightMap (surf, base, BLOCK_WIDTH*LIGHTMAP_BYTES);
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
void R_BuildVertexLight (msurface_t *surf);
void R_RenderBrushPoly (msurface_t *fa)
{
	int			maps;
	//image_t		*image;
	qboolean	is_dynamic = false;
	qboolean	litPoly = r_trans_lightmaps->value && !(fa->texinfo->flags & SURF_NOLIGHTENV);

	c_brush_polys++;

	//image = R_TextureAnimation (fa->texinfo);

	if (fa->flags & SURF_DRAWTURB)
	{	
		R_BuildVertexLight (fa);

		// warp texture, no lightmaps
		GL_TexEnv(GL_MODULATE);

		R_EmitWaterPolys (fa, 1.0, litPoly);

		GL_TexEnv(GL_REPLACE);

		return;
	}

	//GL_Bind(image->texnum);
	//GL_TexEnv(GL_REPLACE);

	//GL_Bind(image->texnum);
	GL_TexEnv(GL_REPLACE);

	R_DrawGLPoly (fa, 1.0, litPoly, false);

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
		if (r_dynamic->value)
		{
			if ( !(fa->texinfo->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP)) )
			{
				is_dynamic = true;
			}
		}
	}

	if (is_dynamic)
	{
		if ( (fa->styles[maps] >= 32 || fa->styles[maps] == 0) && (fa->dlightframe != r_framecount) )
		{
			unsigned	temp[34*34];
			int			smax, tmax;

			smax = (fa->extents[0]>>4)+1;
			tmax = (fa->extents[1]>>4)+1;

			R_BuildLightMap(fa, (void *)temp, smax*4);
			R_SetCacheState(fa);

			GL_Bind(gl_state.lightmap_textures + fa->lightmaptexturenum);

			qglTexSubImage2D( GL_TEXTURE_2D, 0,
							  fa->light_s, fa->light_t, 
							  smax, tmax, 
							  GL_LIGHTMAP_FORMAT, 
							  GL_UNSIGNED_BYTE, temp);

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
SurfAlphaCalc
================
*/
float SurfAlphaCalc (int flags)
{
	if ((flags & SURF_TRANS33) && (flags & SURF_TRANS66) && r_solidalpha->value) // option to disable this
		return DIV254BY255;
	else if (flags & SURF_TRANS33)
		return 0.33333;
	else if (flags & SURF_TRANS66)
		return 0.66666;
	else
		return DIV254BY255;
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
	qboolean	transLit, solidAlpha, envMap;

	// the textures are prescaled up for a better lighting range,
	// so scale it back down

	for (s = r_alpha_surfaces; s; s = s->texturechain)
	{
		c_brush_polys++;

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

		transLit = r_trans_lightmaps->value && !(s->texinfo->flags & SURF_NOLIGHTENV);
		solidAlpha = ( (s->texinfo->flags & SURF_TRANS33) && (s->texinfo->flags & SURF_TRANS66) && (r_solidalpha->value > 0) );
		envMap = ( (s->flags & SURF_ENVMAP) && r_glass_envmaps->value && !solidAlpha);

		if (s->flags & SURF_DRAWTURB)		
			R_EmitWaterPolys (s, SurfAlphaCalc(s->texinfo->flags), transLit);
		else
			R_DrawGLPoly (s, SurfAlphaCalc(s->texinfo->flags), transLit, envMap);
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

/*
================
R_DrawTextureChains
================
*/
void R_DrawTextureChains (void)
{
	int		i;
	msurface_t	*s;
	image_t		*image;

	c_visible_textures = 0;

	//GL_TexEnv (GL_REPLACE);

	if (!gl_config.multitexture)
	{
		for (i = 0, image=gltextures; i<numgltextures; i++,image++)
		{
			if (!image->registration_sequence)
				continue;
			s = image->texturechain;
			if (!s)
				continue;
			c_visible_textures++;

			for ( ; s; s=s->texturechain)
				R_RenderBrushPoly (s);

			image->texturechain = NULL;
		}
	}
	else 
	{	// this is redundant in multitexture mode, right?
		for (i = 0, image=gltextures ; i<numgltextures ; i++,image++)
		{
			if (!image->registration_sequence)
				continue;
			if (!image->texturechain)
				continue;
			c_visible_textures++;

			for (s = image->texturechain; s; s=s->texturechain)
			{
				if (!(s->flags & SURF_DRAWTURB))
					R_RenderBrushPoly (s);
			}
		}

		GL_EnableMultitexture (false);
		for (i = 0, image=gltextures ; i<numgltextures ; i++,image++)
		{
			if (!image->registration_sequence)
				continue;
			s = image->texturechain;
			if (!s)
				continue;

			for ( ; s; s=s->texturechain)
			{
				if (s->flags & SURF_DRAWTURB)
					R_RenderBrushPoly (s);
			}

			image->texturechain = NULL;
		}
		//GL_EnableMultitexture (true);
	}

	GL_TexEnv (GL_REPLACE);
}


/*
===========================================
R_DrawCaustics
Underwater Caustic Effect by Kirk Barnes
===========================================
*/
void R_DrawCaustics (msurface_t *surf)
{
	glpoly_t	*p;
	float		*v;
	int			i, nv, vert=0;
	float		scrollh, scrollv, scaleh, scalev;
	
	nv = surf->polys->numverts;

	// adjustment for texture size and caustic image
	scaleh = surf->texinfo->texWidth / (r_causticpic->width*0.5);
	scalev = surf->texinfo->texHeight / (r_causticpic->height*0.5);

	// sin and cos circular drifting
	scrollh = sin(r_newrefdef.time * 0.08 * M_PI) * 0.45;
	scrollv = cos(r_newrefdef.time * 0.08 * M_PI) * 0.45;

	GL_DisableTexture (1);
	GL_MBind (0, r_causticpic->texnum);
	GL_BlendFunc (GL_DST_COLOR, GL_ONE);
	GL_Enable (GL_BLEND);
	
	// just reuse verts, color, and index from previous pass
	for (p = surf->polys; p; p = p->chain)
	{
		v = p->verts[0];
		for (i=0; i<nv; i++, v+= VERTEXSIZE) {
			VA_SetElem2(texCoordArray[0][vert], (v[3]*scaleh)+scrollh, (v[4]*scalev)+scrollv);
			vert++;
		}
	}
	RB_DrawArrays (GL_TRIANGLES);

	// restore state
	GL_Disable (GL_BLEND);
	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_EnableTexture (1);
}

/*
===========================================
R_DrawTexGlow
===========================================
*/
void R_DrawTexGlow (image_t *glowImage)
{
	GL_DisableTexture (1);
	GL_MBind (0, glowImage->texnum);
	GL_BlendFunc (GL_ONE, GL_ONE);
	GL_Enable (GL_BLEND);

	RB_DrawArrays (GL_TRIANGLES);

	// restore state
	GL_Disable (GL_BLEND);
	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_EnableTexture (1);
}

/*
================
R_RenderLightmappedSurface
================
*/
static void R_RenderLightmappedSurface (msurface_t *surf)
{
	int			nv = surf->polys->numverts;
	int			i, map, baseindex;
	float		*v, scroll;
	image_t		*image = R_TextureAnimationNew(surf);
	image_t		*glow = R_TextureAnimationGlow(surf->texinfo);
	qboolean	is_dynamic = false, glowPass, causticPass;
	unsigned	lmtex = surf->lightmaptexturenum;
	glpoly_t	*p;

	glowPass = ( r_glows->value && (glow != r_notexture) );
	causticPass = ( r_caustics->value
		&& !(surf->texinfo->flags & SURF_FLOWING)
		&& !(surf->texinfo->flags & SURF_ALPHATEST)
		&& (surf->flags & SURF_UNDERWATER) );

	 // Alpha test flag
	if (surf->texinfo->flags & SURF_ALPHATEST)
		GL_Enable (GL_ALPHA_TEST);

	for (map = 0; map < MAXLIGHTMAPS && surf->styles[map] != 255; map++)
	{
		if (r_newrefdef.lightstyles[surf->styles[map]].white != surf->cached_light[map])
			goto dynamic;
	}

	// dynamic this frame or dynamic previously
	if ((surf->dlightframe == r_framecount))
	{
dynamic:
		if (r_dynamic->value) {
			if ( !(surf->texinfo->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP)) )
				is_dynamic = true;
		}
	}

	if (is_dynamic)
	{
		unsigned	temp[128*128];
		int			smax, tmax;

		smax = (surf->extents[0]>>4)+1;
		tmax = (surf->extents[1]>>4)+1;

		R_BuildLightMap (surf, (void *)temp, smax*4);

		if ((surf->styles[map] >= 32 || surf->styles[map] == 0) && (surf->dlightframe != r_framecount))
		{
			R_SetCacheState (surf);
			GL_MBind (1, gl_state.lightmap_textures + surf->lightmaptexturenum);
			lmtex = surf->lightmaptexturenum;
		}
		else {
			GL_MBind (1, gl_state.lightmap_textures + 0);
			lmtex = 0;
		}

		qglTexSubImage2D (GL_TEXTURE_2D, 0,
						  surf->light_s, surf->light_t, 
						  smax, tmax, 
						  GL_LIGHTMAP_FORMAT, 
						  GL_UNSIGNED_BYTE, temp);
	}

	c_brush_polys++;

	GL_MBind (0, image->texnum);
	GL_MBind (1, gl_state.lightmap_textures + lmtex);
	
	if (surf->texinfo->flags & SURF_FLOWING) {
		scroll = -64 * ((r_newrefdef.time / 40.0) - (int)(r_newrefdef.time / 40.0));
		if (scroll == 0.0) scroll = -64.0;
	}
	else
		scroll = 0.0;

	rb_vertex = rb_index = 0;
	for (p = surf->polys; p; p = p->chain)
	{
		v = p->verts[0];

		RB_CheckArrayOverflow (GL_TRIANGLES, nv, (nv-2)*3);
		baseindex = rb_vertex;
		for (i=0; i<nv; i++, v+= VERTEXSIZE)
		{
			VA_SetElem2(texCoordArray[0][rb_vertex], (v[3]+scroll), v[4]);
			VA_SetElem2(texCoordArray[1][rb_vertex], v[5], v[6]);
			VA_SetElem3(vertexArray[rb_vertex], v[0], v[1], v[2]);
			VA_SetElem4(colorArray[rb_vertex], 1, 1, 1, 1);
			rb_vertex++;
		}
		for (i=0; i < nv-2; i++) {
			indexArray[rb_index++] = baseindex;
			indexArray[rb_index++] = baseindex+i+1;
			indexArray[rb_index++] = baseindex+i+2;
		}
	}
	RB_DrawArrays (GL_TRIANGLES);

	GL_Disable (GL_ALPHA_TEST); // Alpha test texture

	if (glowPass) // just redraw with existing arrays for glow
		R_DrawTexGlow (glow);

	if (causticPass) // Barnes caustics
		R_DrawCaustics (surf);

	RB_DrawMeshTris (GL_TRIANGLES, 2);
}


/*
=======================================================================
	Quake2Max vertex lighting code
=======================================================================
*/

/*
========================
R_CreateVertexLightmap
========================
*/
void R_CreateVertexLightmap (msurface_t *surf)
{
	glpoly_t *poly;
	int		size;
	
	//size = sizeof(byte)*3*(surf->polys->numverts);

	for (poly=surf->polys; poly; poly=poly->next)
	{	
		// alloc just enough for this poly
		size = sizeof(byte)*3*(poly->numverts);

		poly->vertexlight = Hunk_Alloc(size);
		poly->vertexlightbase = Hunk_Alloc(size);
		memset(poly->vertexlight, 0, size);
		memset(poly->vertexlightbase, 0, size);
		poly->vertexlightset = false;
	}
}


/*
========================
R_FindPolyCenters
========================
*/
void R_FindPolyCenters (msurface_t *surf)
{
	int i;
	float *v;
	vec3_t average;
	glpoly_t *poly;

	for (poly=surf->polys; poly; poly=poly->next)
	{
		VectorClear(average);

		for (i=0,v=poly->verts[0]; i<poly->numverts; i++, v+=VERTEXSIZE)
			VectorAdd(average, v, average);

		VectorScale(average, 1.0/(float)poly->numverts, poly->center);
	}
}


/*
=================
R_BuildVertexLightBase
=================
*/
void R_SurfLightPoint (msurface_t *surf, vec3_t p, vec3_t color, qboolean baselight);
void R_BuildVertexLightBase (msurface_t *surf, glpoly_t *poly)
{
	vec3_t color, point;
	int	i;
	float *v;

	for (i=0, v=poly->verts[0]; i<poly->numverts; i++, v+=VERTEXSIZE)
	{
		VectorCopy(v, point); // lerp outward away from plane to avoid dark spots?
		// lerp between each vertex and origin - use check for too dark?
		// this messes up curved glass surfaces
		//VectorSubtract (poly->center, v, point);
		//VectorMA(v, 0.01, point, point);

		R_SurfLightPoint (surf, point, color, true);
			
		R_MaxColorVec (color);

		poly->vertexlightbase[i*3+0] = (byte)(color[0]*255.0);
		poly->vertexlightbase[i*3+1] = (byte)(color[1]*255.0);
		poly->vertexlightbase[i*3+2] = (byte)(color[2]*255.0);
	}
}


/*
=================
R_ResetVertextLight
=================
*/
void R_ResetVertextLight (msurface_t *surf)
{
	glpoly_t *poly;

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
	vec3_t color, point;
	int	i;
	float *v;
	glpoly_t *poly;

	if (!r_trans_lightmaps->value)
		return;

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

	if (!r_trans_surf_sorting->value) // check if sorting disabled
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


/*
=================
R_DrawInlineBModel
=================
*/
void R_DrawInlineBModel (entity_t *e)
{
	int			i, k;
	cplane_t	*pplane;
	float		dot;
	msurface_t	*psurf;
	dlight_t	*lt;
	qboolean	modelalpha = false;
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
	if (!r_flashblend->value)
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
				R_MarkLights (lt, 1<<k, currentmodel->nodes + currentmodel->firstnode);
				VectorAdd (temp, currententity->origin, lt->origin);
			}
		} 
		else
		{
			for (k=0; k<r_newrefdef.num_dlights; k++, lt++)
			{
				VectorSubtract (lt->origin, currententity->origin, lt->origin);
				R_MarkLights (lt, 1<<k, currentmodel->nodes + currentmodel->firstnode);
				VectorAdd (lt->origin, currententity->origin, lt->origin);
			}
		}
	}

	psurf = &currentmodel->surfaces[currentmodel->firstmodelsurface];
	psurf->entity = NULL;

	if (currententity->flags & RF_TRANSLUCENT)
	{
		modelalpha = true;
		GL_Enable (GL_BLEND);
		GL_DepthMask (false);
		GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglColor4f (1, 1, 1, currententity->alpha); // Knightmare- draw correct alpha value
		GL_TexEnv (GL_MODULATE);
		qglDisableClientState (GL_COLOR_ARRAY); // use qglColor-set transparency
	}

	//
	// draw texture
	//
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
			{	// add to the translucent chain
				if (!modelalpha) // Don't allow mixing of alpha surfaces and model alpha (fixes hang)
				{
				#if 0
					msurface_t	*s, *last = NULL;
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
					psurf->texturechain = r_alpha_surfaces;
					r_alpha_surfaces = psurf;
					psurf->entity = e; // Knightmare added
				#endif
				}
			}
			else if (gl_config.multitexture && !(psurf->flags & SURF_DRAWTURB))
			{
				R_RenderLightmappedSurface(psurf);
			}
			else // warp surface
			{
				GL_EnableMultitexture (false);
				R_RenderBrushPoly(psurf);
				GL_EnableMultitexture (true);
			}
		}
	}

	if (currententity->flags & RF_TRANSLUCENT)
	{
		GL_Disable (GL_BLEND);
		GL_DepthMask (true);
		GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglColor4f (1,1,1,1);
		GL_TexEnv (GL_REPLACE);
		qglEnableClientState (GL_COLOR_ARRAY);
	}
	else
	{
		if (!gl_config.multitexture)
			R_BlendLightmaps ();
	}
}

/*
=================
R_DrawBrushModel
=================
*/
void R_DrawBrushModel (entity_t *e)
{
	vec3_t		mins, maxs;
	int			i;
	qboolean	rotated;

	if (currentmodel->nummodelsurfaces == 0)
		return;

	currententity = e;
	gl_state.currenttextures[0] = gl_state.currenttextures[1] = -1;

	if (e->angles[0] || e->angles[1] || e->angles[2])
	{
		rotated = true;
		for (i=0 ; i<3 ; i++)
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

    qglPushMatrix ();
	R_RotateForEntity (e, true);

	GL_EnableMultitexture (true);

// Vic - begin
	R_SetLightingMode(e->flags);
// Vic - end

	R_DrawInlineBModel (e);

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

		if (surf->texinfo->flags & SURF_SKY)
		{	// just adds to visible sky bounds
			R_AddSkySurface (surf);
		}
		else if (surf->texinfo->flags & (SURF_TRANS33|SURF_TRANS66))
		{	// add to the translucent chain
			surf->texturechain = r_alpha_surfaces;
			r_alpha_surfaces = surf;
			surf->entity = NULL; // Knightmare added
		}
		else if (gl_config.multitexture && !(surf->flags & SURF_DRAWTURB))
		{	
			R_RenderLightmappedSurface(surf);
		}
		else // warp surface
		{
			// the polygon is visible, so add it to the texture
			// sorted chain
			// FIXME: this is a hack for animation
			//image = R_TextureAnimation (surf->texinfo);
			image = R_TextureAnimationNew (surf);
			surf->texturechain = image->texturechain;
			image->texturechain = surf;
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

	if (!r_drawworld->value)
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

	gl_state.currenttextures[0] = gl_state.currenttextures[1] = -1;

	qglColor3f (1,1,1);
	memset (gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));
	R_ClearSkyBox ();

	if (gl_config.multitexture)
	{
		GL_EnableMultitexture (true);

	// Vic - begin
		R_SetLightingMode(0);
	// Vic - end

		R_RecursiveWorldNode (r_worldmodel->nodes);

		GL_EnableMultitexture (false);
	}
	else
	{
		R_RecursiveWorldNode (r_worldmodel->nodes);
	}

	// theoretically nothing should happen in the next two functions
	// if multitexture is enabled
	R_DrawTextureChains ();
	R_BlendLightmaps ();
	
	R_DrawSkyBox ();

	// Knightmare- GuyP's r_showtris fix
	if (!gl_config.multitexture)
		R_DrawTriangleOutlines ();
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

	if (r_oldviewcluster == r_viewcluster && r_oldviewcluster2 == r_viewcluster2 && !r_novis->value && r_viewcluster != -1)
		return;

	// development aid to let you run around and see exactly where
	// the pvs ends
	if (r_lockpvs->value)
		return;

	if (!r_worldmodel)    // 12-12-2011 KM potential crash fix
        return;

	r_visframecount++;
	r_oldviewcluster = r_viewcluster;
	r_oldviewcluster2 = r_viewcluster2;

	if (r_novis->value || r_viewcluster == -1 || !r_worldmodel->vis)
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
=============================================================================

  LIGHTMAP ALLOCATION

=============================================================================
*/

static void LM_InitBlock( void )
{
	memset( gl_lms.allocated, 0, sizeof( gl_lms.allocated ) );
}

static void LM_UploadBlock( qboolean dynamic )
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

	GL_Bind( gl_state.lightmap_textures + texture );
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if ( dynamic )
	{
		int i;

		for ( i = 0; i < BLOCK_WIDTH; i++ )
		{
			if ( gl_lms.allocated[i] > height )
				height = gl_lms.allocated[i];
		}

		qglTexSubImage2D( GL_TEXTURE_2D, 
						  0,
						  0, 0,
						  BLOCK_WIDTH, height,
						  GL_LIGHTMAP_FORMAT,
						  GL_UNSIGNED_BYTE,
						  gl_lms.lightmap_buffer );
	}
	else
	{
		qglTexImage2D( GL_TEXTURE_2D, 
					   0, 
					   gl_lms.internal_format,
					   BLOCK_WIDTH, BLOCK_HEIGHT, 
					   0, 
					   GL_LIGHTMAP_FORMAT, 
					   GL_UNSIGNED_BYTE, 
					   gl_lms.lightmap_buffer );
		if ( ++gl_lms.current_lightmap_texture == MAX_LIGHTMAPS )
			VID_Error( ERR_DROP, "LM_UploadBlock() - MAX_LIGHTMAPS exceeded\n" );
	}
}

// returns a texture number and the position inside it
static qboolean LM_AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;

	best = BLOCK_HEIGHT;

	for (i=0 ; i<BLOCK_WIDTH-w ; i++)
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

	if (best + h > BLOCK_HEIGHT)
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
void R_BuildPolygonFromSurface(msurface_t *fa)
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

	for (i=0 ; i<lnumverts ; i++)
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
		s /= BLOCK_WIDTH*16; //fa->texinfo->texture->width;

		t = DotProduct (vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t -= fa->texturemins[1];
		t += fa->light_t*16;
		t += 8;
		t /= BLOCK_HEIGHT*16; //fa->texinfo->texture->height;

		poly->verts[i][5] = s;
		poly->verts[i][6] = t;
	}

	poly->numverts = lnumverts;

}

/*
========================
R_CreateSurfaceLightmap
========================
*/
void R_CreateSurfaceLightmap (msurface_t *surf)
{
	int		smax, tmax;
	byte	*base;

	if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
		return;

	if (surf->texinfo->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP))
	//if (surf->texinfo->flags & (SURF_SKY|SURF_WARP))
		return;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;

	if ( !LM_AllocBlock( smax, tmax, &surf->light_s, &surf->light_t ) )
	{
		LM_UploadBlock( false );
		LM_InitBlock();
		if ( !LM_AllocBlock( smax, tmax, &surf->light_s, &surf->light_t ) )
		{
			VID_Error( ERR_FATAL, "Consecutive calls to LM_AllocBlock(%d,%d) failed\n", smax, tmax );
		}
	}

	surf->lightmaptexturenum = gl_lms.current_lightmap_texture;

	base = gl_lms.lightmap_buffer;
	base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * LIGHTMAP_BYTES;

	R_SetCacheState( surf );
	R_BuildLightMap (surf, base, BLOCK_WIDTH*LIGHTMAP_BYTES);
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
	unsigned		dummy[128*128];

	memset( gl_lms.allocated, 0, sizeof(gl_lms.allocated) );

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

	if (!gl_state.lightmap_textures)
	{
		gl_state.lightmap_textures	= TEXNUM_LIGHTMAPS;
	//	gl_state.lightmap_textures	= gl_state.texture_extension_number;
	//	gl_state.texture_extension_number = gl_state.lightmap_textures + MAX_LIGHTMAPS;
	}

	gl_lms.current_lightmap_texture = 1;

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

	// initialize the dynamic lightmap texture
	GL_Bind( gl_state.lightmap_textures + 0 );
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexImage2D( GL_TEXTURE_2D, 
				   0, 
				   gl_lms.internal_format,
				   BLOCK_WIDTH, BLOCK_HEIGHT, 
				   0, 
				   GL_LIGHTMAP_FORMAT, 
				   GL_UNSIGNED_BYTE, 
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

