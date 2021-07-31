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

// r_warp.c -- sky and water polygons

#include "r_local.h"

extern	model_t	*loadmodel;
msurface_t	*warpface;

#define	SUBDIVIDE_SIZE	64

void BoundPoly (int numverts, float *verts, vec3_t mins, vec3_t maxs)
{
	int		i, j;
	float	*v;

	mins[0] = mins[1] = mins[2] = 999999;
	maxs[0] = maxs[1] = maxs[2] = -999999;
	v = verts;
	for (i=0; i<numverts; i++)
		for (j=0; j<3; j++, v++)
		{
			if (*v < mins[j])
				mins[j] = *v;
			if (*v > maxs[j])
				maxs[j] = *v;
		}
}

void SubdividePolygon (int numverts, float *verts)
{
	int		i, j, k;
	vec3_t	mins, maxs;
	float	m;
	float	*v;
	vec3_t	front[64], back[64];
	int		f, b;	// size
	float	dist[64];
	float	frac;
	glpoly_t	*poly;
	float	s, t;
	vec3_t	total;
	float	total_s, total_t;
	float	lightmap_total_s, lightmap_total_t;	// added for lightmaps

	if (numverts > 60)
		VID_Error (ERR_DROP, "numverts = %i", numverts);

	BoundPoly (numverts, verts, mins, maxs);

	for (i=0; i<3; i++)
	{
		m = (mins[i] + maxs[i]) * 0.5;
	//	m = SUBDIVIDE_SIZE * floor (m/SUBDIVIDE_SIZE + 0.5);
		m = r_subdivide_size->value * floor (m/r_subdivide_size->value + 0.5);
		if (maxs[i] - m < 8)
			continue;
		if (m - mins[i] < 8)
			continue;

		// cut it
		v = verts + i;
		for (j=0; j<numverts; j++, v+= 3)
			dist[j] = *v - m;

		// wrap cases
		dist[j] = dist[0];
		v-=i;
		VectorCopy (verts, v);

		f = b = 0;
		v = verts;
		for (j=0; j<numverts; j++, v+= 3)
		{
			if (dist[j] >= 0)
			{
				VectorCopy (v, front[f]);
				f++;
			}
			if (dist[j] <= 0)
			{
				VectorCopy (v, back[b]);
				b++;
			}
			if (dist[j] == 0 || dist[j+1] == 0)
				continue;
			if ( (dist[j] > 0) != (dist[j+1] > 0) )
			{
				// clip point
				frac = dist[j] / (dist[j] - dist[j+1]);
				for (k=0; k<3; k++)
					front[f][k] = back[b][k] = v[k] + frac*(v[3+k] - v[k]);
				f++;
				b++;
			}
		}

		SubdividePolygon (f, front[0]);
		SubdividePolygon (b, back[0]);
		return;
	}

	// add a point in the center to help keep warp valid
//	poly = Hunk_Alloc (sizeof(glpoly_t) + ((numverts-4)+2) * VERTEXSIZE*sizeof(float));
	poly = Hunk_Alloc (sizeof(glpoly_t) + ((numverts-4)+2) * sizeof(mpolyvertex_t));
	poly->next = warpface->polys;
	warpface->polys = poly;
	poly->numverts = numverts+2;

	// alloc vertex light fields
/*	size = poly->numverts*3*sizeof(byte);
	poly->vertexlight = Hunk_Alloc(size);
	poly->vertexlightbase = Hunk_Alloc(size);
	memset(poly->vertexlight, 0, size);
	memset(poly->vertexlightbase, 0, size); */
	poly->vertexlightset = false;
	
	VectorClear (total);
	total_s = total_t = 0;
	lightmap_total_s = lightmap_total_t = 0;	// added for lightmaps
	for (i=0; i<numverts; i++, verts+=3)
	{
		VectorCopy (verts, poly->verts[i+1].xyz);
		s = DotProduct (verts, warpface->texinfo->vecs[0]);
		t = DotProduct (verts, warpface->texinfo->vecs[1]);

		total_s += s;
		total_t += t;
		VectorAdd (total, verts, total);

		poly->verts[i+1].texture_st[0] = s;
		poly->verts[i+1].texture_st[1] = t;

		// added for lightmaps
		if (warpface->isLightmapped)
		{
			s = DotProduct (verts, warpface->texinfo->vecs[0]) + warpface->texinfo->vecs[0][3];
			s -= warpface->texturemins[0];
			s += warpface->light_s*16;
			s += 8;
			s /= LM_BLOCK_WIDTH*16;

			t = DotProduct (verts, warpface->texinfo->vecs[1]) + warpface->texinfo->vecs[1][3];
			t -= warpface->texturemins[1];
			t += warpface->light_t*16;
			t += 8;
			t /= LM_BLOCK_HEIGHT*16;

			poly->verts[i+1].lightmap_st[0] = s;
			poly->verts[i+1].lightmap_st[1] = t;

			lightmap_total_s += s;
			lightmap_total_t += t;
		}
	}

	VectorScale (total, (1.0/(float)numverts), poly->verts[0].xyz);
	VectorCopy(poly->verts[0].xyz, poly->center); // for vertex lighting
	poly->verts[0].texture_st[0] = total_s/numverts;
	poly->verts[0].texture_st[1] = total_t/numverts;
	// added for lightmaps
	if (warpface->isLightmapped)
	{
		poly->verts[0].lightmap_st[0] = lightmap_total_s/numverts;
		poly->verts[0].lightmap_st[1] = lightmap_total_t/numverts;
	}

	// copy first vertex to last
	memcpy (&poly->verts[i+1], &poly->verts[1], sizeof(mpolyvertex_t));
}

/*
================
R_SubdivideSurface

Breaks a polygon up along axial 64 unit
boundaries so that turbulent warps
can be done reasonably.
================
*/
void R_SubdivideSurface (msurface_t *surf)
{
	vec3_t		verts[64];
	int			numverts;
	int			i;
	int			lindex;
	float		*vec;

	// clamp r_subdivide_size to a reasonable range
	if (!r_subdivide_size)
		r_subdivide_size = Cvar_Get("r_subdivide_size", "64", 0);	// chop size for warp surfaces
	if ((r_subdivide_size->value < 64.0f) || (r_subdivide_size->value > 512.0f))
		Cvar_SetValue( "r_subdivide_size", 64);

	warpface = surf;

	//
	// convert edges back to a normal polygon
	//
	numverts = 0;
	for (i=0; i<surf->numedges; i++)
	{
		lindex = loadmodel->surfedges[surf->firstedge + i];

		if (lindex > 0)
			vec = loadmodel->vertexes[loadmodel->edges[lindex].v[0]].position;
		else
			vec = loadmodel->vertexes[loadmodel->edges[-lindex].v[1]].position;
		VectorCopy (vec, verts[numverts]);
		numverts++;
	}

	SubdividePolygon (numverts, verts[0]);
}

//=========================================================



// speed up sin calculations - Ed
float	r_turbsin[] =
{
	#include "warpsin.h"
};
#define TURBSCALE (256.0 / (2 * M_PI))



#if 0
// MrG - texture shader stuffs
#define DST_SIZE 16
unsigned int dst_texture_ARB;


/*
===============
CreateDSTTex_ARB

Create the texture which warps texture shaders
===============
*/
void CreateDSTTex_ARB (void)
{
	unsigned char	dist[DST_SIZE][DST_SIZE][4];
	int				x,y;

	srand(GetTickCount());
	for (x=0; x<DST_SIZE; x++)
		for (y=0; y<DST_SIZE; y++) {
			dist[x][y][0] = rand()%255;
			dist[x][y][1] = rand()%255;
			dist[x][y][2] = rand()%48;
			dist[x][y][3] = rand()%48;
		}

	qglGenTextures(1,&dst_texture_ARB);
	qglBindTexture(GL_TEXTURE_2D, dst_texture_ARB);
	qglTexImage2D (GL_TEXTURE_2D, 0, 4, DST_SIZE, DST_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, dist);

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);
	qglTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
}

/*
===============
R_InitDSTTex

Resets the texture which warps texture shaders.
Needed after a vid_restart.
===============
*/
void R_InitDSTTex (void)
{
	qglDeleteTextures(1, &dst_texture_ARB);
	dst_texture_ARB = 0;
	CreateDSTTex_ARB ();
}
//end MrG
#endif


image_t *R_TextureAnimation (msurface_t *surf);

/*
=============
RB_RenderWarpSurface

backend for R_DrawWarpSurface
=============
*/
void RB_RenderWarpSurface (msurface_t *surf)
{
	float		args[7] = {0,0.05,0,0,0.04,0,0};
	float		alpha = colorArray[0][3];
	image_t		*image = R_TextureAnimation (surf);
	qboolean	lightmapped = surf->isLightmapped && (r_worldmodel->bspFeatures & BSPF_WARPLIGHTMAPS);
	qboolean	vertexLight = r_warp_lighting->integer && !lightmapped && !(surf->texinfo->flags & SURF_NOLIGHTENV);
	qboolean	texShaderWarpARB = glConfig.arb_fragment_program && r_pixel_shader_warp->integer;
	qboolean	texShaderWarp = texShaderWarpARB;

	if (rb_vertex == 0 || rb_index == 0) // nothing to render
		return;

	c_brush_calls++;

	// Psychospaz's vertex lighting
	if (vertexLight) {
		GL_ShadeModel (GL_SMOOTH);
		if (!texShaderWarp)
			R_SetVertexRGBScale (true);
	}

	/*
	Texture Shader waterwarp
	Damn this looks fantastic
	WHY texture shaders? because I can!
	- MrG
	*/
	if (texShaderWarpARB)
	{
		GL_SelectTexture(0);
		GL_MBind(0, image->texnum);

		if (lightmapped)
		{
		//	GL_EnableTexture(1);
			if (r_fullbright->integer != 0)
				GL_MBind (1, glMedia.whitetexture->texnum);
			else
				GL_MBind (1, glState.lightmap_textures + surf->lightmaptexturenum);

			GL_EnableTexture(2);
		//	GL_MBind(2, dst_texture_ARB);
			GL_MBind(2, glMedia.distTextureARB->texnum);

			GL_Enable (GL_FRAGMENT_PROGRAM_ARB);
			qglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fragment_programs[F_PROG_WARP_LM]);
			qglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, r_rgbscale->value, r_rgbscale->value, r_rgbscale->value, 1.0);
		}
		else
		{
			GL_EnableTexture(1);
		//	GL_MBind(1, dst_texture_ARB);
			GL_MBind(1, glMedia.distTextureARB->texnum);

			GL_Enable (GL_FRAGMENT_PROGRAM_ARB);
			qglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fragment_programs[F_PROG_WARP]);
			qglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, r_rgbscale->value, r_rgbscale->value, r_rgbscale->value, 1.0);
		}
	}
	else
	{
		if (lightmapped)
		{
			GL_SelectTexture(0);
			GL_MBind(0, image->texnum);

			GL_EnableTexture(1);
			if (r_fullbright->integer != 0)
				GL_MBind (1, glMedia.whitetexture->texnum);
			else
				GL_MBind (1, glState.lightmap_textures + surf->lightmaptexturenum);
		}
		else
			GL_Bind(image->texnum);
	}

	RB_DrawArrays ();

	// MrG - texture shader waterwarp
	if (texShaderWarpARB)
	{
		if (lightmapped) {
			GL_SelectTexture(2);
			GL_Disable (GL_FRAGMENT_PROGRAM_ARB);
			GL_DisableTexture(2);
		}
		else {
			GL_Disable (GL_FRAGMENT_PROGRAM_ARB);
			GL_DisableTexture(1);
			GL_SelectTexture(0);
		}
	}

	// Psychospaz's vertex lighting
	if (vertexLight) {
		GL_ShadeModel (GL_FLAT);
		if (!texShaderWarp)
			R_SetVertexRGBScale (false); 
	}

	RB_DrawMeshTris ();
	rb_vertex = rb_index = 0;
}

/*
=============
R_DrawWarpSurface

Does a water warp on the pre-fragmented glpoly_t chain.
added Psychospaz's lightmaps on alpha surfaces
=============
*/
void R_DrawWarpSurface (msurface_t *surf, float alpha, qboolean render)
{
	glpoly_t	*p, *bp;
	float		s, t, scroll, dstscroll, rdt = r_newrefdef.time;	// *v
	float		dstscroll_x, dstscroll_y;
	mpolyvertex_t	*v;
	vec3_t		point;
	int			i, texWidth, texHeight;
	qboolean	lightmapped = surf->isLightmapped && (r_worldmodel->bspFeatures & BSPF_WARPLIGHTMAPS);
	qboolean	vertexLight = r_warp_lighting->integer && !lightmapped && !r_fullbright->integer && !(surf->texinfo->flags & SURF_NOLIGHTENV);

	c_brush_surfs++;

	texWidth = surf->texinfo->texWidth;
	texHeight = surf->texinfo->texHeight;

	dstscroll = -64 * ( (r_newrefdef.time*0.15) - (int)(r_newrefdef.time*0.15) );
	dstscroll_x = 64 * sin (r_newrefdef.time * 0.08 * M_PI) * 0.45;
	dstscroll_y = 64 * cos (r_newrefdef.time * 0.08 * M_PI) * 0.45;

	if (surf->texinfo->flags & SURF_FLOWING)
		scroll = -64 * ( (r_newrefdef.time*0.5) - (int)(r_newrefdef.time*0.5) );
	else
		scroll = 0.0f;

//	rb_vertex = rb_index = 0;
	for (bp = surf->polys; bp; bp = bp->next)
	{
		c_brush_polys += (bp->numverts-2);
		p = bp;
		if (RB_CheckArrayOverflow (p->numverts, (p->numverts-2)*3))
			RB_RenderWarpSurface (surf);
		for (i = 0; i < p->numverts-2; i++) {
			indexArray[rb_index++] = rb_vertex;
			indexArray[rb_index++] = rb_vertex+i+1;
			indexArray[rb_index++] = rb_vertex+i+2;
		}
		for (i=0, v=&p->verts[0]; i<p->numverts; i++, v++)
		{
		#if !id386
			s = v->texture_st[0] + r_turbsin[(int)((v->texture_st[1]*0.125+rdt) * TURBSCALE) & 255];
			t = v->texture_st[1] + r_turbsin[(int)((v->texture_st[0]*0.125+rdt) * TURBSCALE) & 255];
		#else
			s = v->texture_st[0] + r_turbsin[Q_ftol( ((v->texture_st[1]*0.125+rdt) * TURBSCALE) ) & 255];
			t = v->texture_st[1] + r_turbsin[Q_ftol( ((v->texture_st[0]*0.125+rdt) * TURBSCALE) ) & 255];
		#endif
			s += scroll;
			s /= (float)texWidth;
			t /= (float)texHeight;
//=============== Water waves ========================
			VectorCopy(v->xyz, point);
			if ( r_waterwave->value > 0 && !(surf->texinfo->flags & SURF_FLOWING)
				&& surf->plane->normal[2] > 0
				&& surf->plane->normal[2] > surf->plane->normal[0]
				&& surf->plane->normal[2] > surf->plane->normal[1] )
				point[2] = v->xyz[2] + r_waterwave->value * sin(v->xyz[0]*0.025+rdt) * sin(v->xyz[2]*0.05+rdt);
//=============== End water waves ====================
			// MrG - texture shader waterwarp
			if (lightmapped) {
				VA_SetElem2(texCoordArray[0][rb_vertex], s, t);
				VA_SetElem2(texCoordArray[1][rb_vertex], v->lightmap_st[0], v->lightmap_st[1]);
			//	VA_SetElem2(texCoordArray[2][rb_vertex], (v->texture_st[0]+dstscroll) / (float)texWidth, v->texture_st[1] / (float)texHeight);
				VA_SetElem2(texCoordArray[2][rb_vertex], (v->texture_st[0]+dstscroll_x) / (float)texWidth, (v->texture_st[1]+dstscroll_y) / (float)texHeight);
			}
			else { 
				VA_SetElem2(texCoordArray[0][rb_vertex], s, t);
				VA_SetElem2(texCoordArray[1][rb_vertex], (v->texture_st[0]+dstscroll) / (float)texWidth, v->texture_st[1] / (float)texHeight);
			}

			if (lightmapped)
				VA_SetElem4(colorArray[rb_vertex], 1, 1, 1, alpha);
			else if (vertexLight && p->vertexlightset)
				VA_SetElem4(colorArray[rb_vertex],
					(float)(v->lightcolor[0]*DIV255),
					(float)(v->lightcolor[1]*DIV255),
					(float)(v->lightcolor[2]*DIV255), alpha);
			else
				VA_SetElem4(colorArray[rb_vertex], glState.inverse_intensity, glState.inverse_intensity, glState.inverse_intensity, alpha);

			VA_SetElem3(vertexArray[rb_vertex], point[0], point[1], point[2]);

			rb_vertex++;
		}
	}

	if (render)
		RB_RenderWarpSurface (surf);
}
