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
		for (j=0 ; j<3 ; j++, v++)
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
	int		f, b;
	float	dist[64];
	float	frac;
	glpoly_t	*poly;
	float	s, t;
	vec3_t	total;
	float	total_s, total_t;

	if (numverts > 60)
		VID_Error (ERR_DROP, "numverts = %i", numverts);

	BoundPoly (numverts, verts, mins, maxs);

	for (i=0; i<3; i++)
	{
		m = (mins[i] + maxs[i]) * 0.5;
		m = SUBDIVIDE_SIZE * floor (m/SUBDIVIDE_SIZE + 0.5);
		if (maxs[i] - m < 8)
			continue;
		if (m - mins[i] < 8)
			continue;

		// cut it
		v = verts + i;
		for (j=0 ; j<numverts ; j++, v+= 3)
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
	poly = Hunk_Alloc (sizeof(glpoly_t) + ((numverts-4)+2) * VERTEXSIZE*sizeof(float));
	poly->next = warpface->polys;
	warpface->polys = poly;
	poly->numverts = numverts+2;
	VectorClear (total);
	total_s = 0;
	total_t = 0;
	for (i=0; i<numverts; i++, verts+= 3)
	{
		VectorCopy (verts, poly->verts[i+1]);
		s = DotProduct (verts, warpface->texinfo->vecs[0]);
		t = DotProduct (verts, warpface->texinfo->vecs[1]);

		total_s += s;
		total_t += t;
		VectorAdd (total, verts, total);

		poly->verts[i+1][3] = s;
		poly->verts[i+1][4] = t;
	}

	VectorScale (total, (1.0/numverts), poly->verts[0]);
	poly->verts[0][3] = total_s/numverts;
	poly->verts[0][4] = total_t/numverts;

	// copy first vertex to last
	memcpy (poly->verts[i+1], poly->verts[1], sizeof(poly->verts[0]));
}

/*
================
R_SubdivideSurface

Breaks a polygon up along axial 64 unit
boundaries so that turbulent warps
can be done reasonably.
================
*/
void R_SubdivideSurface (msurface_t *fa)
{
	vec3_t		verts[64];
	int			numverts;
	int			i;
	int			lindex;
	float		*vec;

	warpface = fa;

	//
	// convert edges back to a normal polygon
	//
	numverts = 0;
	for (i=0; i<fa->numedges; i++)
	{
		lindex = loadmodel->surfedges[fa->firstedge + i];

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

// MrG - texture shader stuffs
#define DST_SIZE 16
unsigned int dst_texture = 0;



/*
===============
CreateDSTTex

Create the texture which warps texture shaders
===============
*/
void CreateDSTTex (void)
{
	char data[DST_SIZE][DST_SIZE][2];
	int x,y;

	for (x=0; x<DST_SIZE; x++)
		for (y=0; y<DST_SIZE; y++)
		{
			data[x][y][0]=rand()%255-128;
			data[x][y][1]=rand()%255-128;
		}

	qglGenTextures(1,&dst_texture);
	qglBindTexture(GL_TEXTURE_2D, dst_texture);
	qglTexImage2D(GL_TEXTURE_2D, 0, GL_DSDT8_NV, DST_SIZE, DST_SIZE, 0, GL_DSDT_NV,
				GL_BYTE, data);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
//end MrG


/*
===============
R_InitDSTTex

Resets the texture which warps texture shaders.
Needed after a vid_restart.
===============
*/
void R_InitDSTTex (void)
{
	dst_texture = 0;
	CreateDSTTex();
}

image_t *R_TextureAnimationNew (msurface_t *surf);

/*
=============
R_EmitWaterPolys

Does a water warp on the pre-fragmented glpoly_t chain.
added Psychospaz's lightmaps on alpha surfaces
=============
*/
void R_EmitWaterPolys (msurface_t *fa, float alpha, qboolean light)
{
	glpoly_t	*p, *bp;
	float		*v, s, t, os, ot, scroll;
	int			i, baseindex;
	float		rdt = r_newrefdef.time;
	vec3_t		point;//, color;
	image_t		*image;
	//MrG
	float		dstscroll;
	float		args[7] = {0,0.05,0,0,0.04,0,0};
	qboolean	texShaderWarp = (gl_config.NV_texshaders && gl_config.multitexture && r_pixel_shader_warp->value);

	image = R_TextureAnimationNew (fa); // fa->texinfo->image

	// Psychospaz's lightmaps on alpha surfaces
	if (light) {
		GL_ShadeModel (GL_SMOOTH);
		if (!texShaderWarp)
			R_SetVertexOverbrights(true);
	}

	/*
	Texture Shader waterwarp
	Damn this looks fantastic
	WHY texture shaders? because I can!
	- MrG
	*/
	if (texShaderWarp)
	{
		GL_SelectTexture(0);
		GL_MBind(0, dst_texture);
		qglTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_2D);

		GL_EnableTexture(1);
		GL_MBind(1, image->texnum);

		qglTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_2D);
		qglTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_OFFSET_TEXTURE_2D_NV);
		qglTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB);
		qglTexEnvfv(GL_TEXTURE_SHADER_NV, GL_OFFSET_TEXTURE_MATRIX_NV, &args[1]);

		// Psychospaz's lighting
		// use this so that the new water isnt so bright anymore
		// We wont bother check for the extensions availabiliy, as the hardware required
		// to make it this far definately supports this as well
		qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);

		qglEnable(GL_TEXTURE_SHADER_NV);
		dstscroll = -64 * ( (r_newrefdef.time*0.15) - (int)(r_newrefdef.time*0.15) );
	}
	else
		GL_Bind(image->texnum);

	// end MrG's changes

	if (fa->texinfo->flags & SURF_FLOWING)
		scroll = -64 * ( (r_newrefdef.time*0.5) - (int)(r_newrefdef.time*0.5) );
	else
		scroll = 0.0f;

	rb_vertex = rb_index = 0;
	for (bp=fa->polys; bp; bp=bp->next)
	{
		p = bp;

		RB_CheckArrayOverflow (GL_TRIANGLES, p->numverts, (p->numverts-2)*3);
		baseindex = rb_vertex;
		for (i=0, v=p->verts[0]; i<p->numverts; i++, v+=VERTEXSIZE)
		{
			os = v[3];
			ot = v[4];

			VectorCopy(v, point);

			/*if (texShaderWarp) {
				s = (os + scroll) / 64;
				t = ot / 64;
			}
			else {*/
			#if !id386
				s = os + r_turbsin[(int)((ot*0.125+rdt) * TURBSCALE) & 255];
			#else
				s = os + r_turbsin[Q_ftol( ((ot*0.125+rdt) * TURBSCALE) ) & 255];
			#endif
				s += scroll;
				s *= (1.0/64);

			#if !id386
				t = ot + r_turbsin[(int)((os*0.125+rdt) * TURBSCALE) & 255];
			#else
				t = ot + r_turbsin[Q_ftol( ((os*0.125+rdt) * TURBSCALE) ) & 255];
			#endif
				t *= (1.0/64);
			//}
//=============== Water waves ========================
			if (r_waterwave->value > 0 && !(fa->texinfo->flags & SURF_FLOWING)
				&& fa->plane->normal[2] > 0
				&& fa->plane->normal[2] > fa->plane->normal[0]
				&& fa->plane->normal[2] > fa->plane->normal[1])
			{
				point[2] = v[2] + r_waterwave->value * sin(v[0]*0.025+rdt) * sin(v[2]*0.05+rdt);
			}
//=============== End water waves ====================

			if (light && p->vertexlight && p->vertexlightset)
				VA_SetElem4(colorArray[rb_vertex],
					(float)(p->vertexlight[i*3+0]*DIV255),
					(float)(p->vertexlight[i*3+1]*DIV255),
					(float)(p->vertexlight[i*3+2]*DIV255), alpha);
			else
				VA_SetElem4(colorArray[rb_vertex], gl_state.inverse_intensity, gl_state.inverse_intensity, gl_state.inverse_intensity, alpha);

			// MrG - texture shader waterwarp
			if (texShaderWarp) {
				VA_SetElem2(texCoordArray[0][rb_vertex], (v[3]+dstscroll)*0.015625, v[4]*0.015625);
				VA_SetElem2(texCoordArray[1][rb_vertex], s, t);
			}
			else
				VA_SetElem2(texCoordArray[0][rb_vertex], s, t);
			// end MrG

			VA_SetElem3(vertexArray[rb_vertex], point[0], point[1], point[2]);
			rb_vertex++;
		}

		for (i = 0; i < p->numverts-2; i++) {
			indexArray[rb_index++] = baseindex;
			indexArray[rb_index++] = baseindex+i+1;
			indexArray[rb_index++] = baseindex+i+2;
		}
	}
	RB_DrawArrays (GL_TRIANGLES);

	// MrG - texture shader waterwarp
	if (texShaderWarp)
	{ 
		GL_DisableTexture(1);
		qglDisable(GL_TEXTURE_SHADER_NV);
		qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // Psychospaz's lighting

		GL_SelectTexture(0);
		qglDisable(GL_TEXTURE_SHADER_NV);
	}
	// end MrG

	// Psychospaz's lightmaps on alpha surfaces
	if (light) {
		GL_ShadeModel (GL_FLAT);
		if (!texShaderWarp)
			R_SetVertexOverbrights(false); 
	}

	RB_DrawMeshTris (GL_TRIANGLES, 1);
}
