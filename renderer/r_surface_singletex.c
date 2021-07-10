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

// r_surf_singletex.c: old single-texture 2-pass world rendering (no longer used)

#include "r_local.h"

#if USE_SINGLETEXTURE_MODE

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
R_DrawAlphaSurfaces

Draw trans water surfaces and windows.
The BSP tree is waled front to back, so unwinding the chain
of alpha_surfaces will draw back to front, giving proper ordering.
================
*/
void R_DrawAlphaSurfaces (void)
{
	msurface_t	*s;
	qboolean	light;	//solidAlpha, envMap;

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
			R_DrawWarpSurface (s, R_SurfAlphaCalc(s->texinfo->flags), !R_SurfsAreBatchable (s, s->texturechain));
		else if ( (r_trans_lighting->integer == 2) && glConfig.multitexture && light && s->lightmaptexturenum )
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
R_DrawSingleTextureChains

Draws all solid textures in 2-pass mode
================
*/
void R_DrawSingleTextureChains (void)
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
=================
R_DrawInlineBModel_Old
=================
*/
void R_DrawInlineBModel_Old (entity_t *e, int causticflag)
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
			if ( glConfig.multitexture && !(psurf->texinfo->flags & (SURF_SKY|SURF_WARP)) )
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
				if ( glConfig.multitexture &/ !(psurf->flags & SURF_DRAWTURB) )
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
			#endif	// MULTITEXTURE_CHAINS */
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
================
R_RecursiveWorldNode_Old
================
*/
void R_RecursiveWorldNode_Old (mnode_t *node)
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
	R_RecursiveWorldNode_Old (node->children[side]);

	// draw stuff
	for ( c = node->numsurfaces, surf = r_worldmodel->surfaces + node->firstsurface; c ; c--, surf++)
	{
		if (surf->visframe != r_framecount)
			continue;

		if ((surf->flags & SURF_PLANEBACK) != sidebit)
			continue;		// wrong side

		surf->entity = NULL;

#ifdef BATCH_LM_UPDATES
		if ( glConfig.multitexture && !(surf->texinfo->flags & (SURF_SKY|SURF_DRAWTURB)) )
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
	R_RecursiveWorldNode_Old (node->children[!side]);
}


/*
=============
R_DrawWorld_Old
=============
*/
void R_DrawWorld_Old (void)
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

		R_RecursiveWorldNode_Old (r_worldmodel->nodes);

#ifndef MULTITEXTURE_CHAINS
		GL_EnableMultitexture (false);
#endif // MULTITEXTURE_CHAINS 

		R_DrawMultiTextureChains ();	// draw solid warp surfaces
	}
	else
	{	// add surfaces to texture chains for 2-pass rendering
		R_RecursiveWorldNode_Old (r_worldmodel->nodes);

		R_DrawSingleTextureChains ();

		R_BlendLightmaps ();

		R_DrawTriangleOutlines ();
	}

	R_DrawSkyBox ();
}

#endif	// USE_SINGLETEXTURE_MODE
