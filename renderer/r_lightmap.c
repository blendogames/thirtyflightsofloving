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

// r_lightmap.c: lightmap loading and handling functions

#include "r_local.h"


#define DYNAMIC_LIGHT_WIDTH  128
#define DYNAMIC_LIGHT_HEIGHT 128

#define LIGHTMAP_BYTES 4

#define	BLOCK_WIDTH		128
#define	BLOCK_HEIGHT	128

#define	MAX_LIGHTMAPS	128

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
		int size = lnumverts * 3*sizeof(byte);
		poly->vertexlight = Hunk_Alloc(size);
		poly->vertexlightbase = Hunk_Alloc(size);
		memset(poly->vertexlight, 0, size);
		memset(poly->vertexlightbase, 0, size);
		poly->vertexlightset = false;
	}

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
		s /= BLOCK_WIDTH*16; //fa->texinfo->texture->width;

		t = DotProduct (vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t -= fa->texturemins[1];
		t += fa->light_t*16;
		t += 8;
		t /= BLOCK_HEIGHT*16; //fa->texinfo->texture->height;

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
	int		smax, tmax;
	byte	*base;

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

	base = gl_lms.lightmap_buffer;
	base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * LIGHTMAP_BYTES;

	R_SetCacheState (surf);
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

	if (!glState.lightmap_textures)
	{
		glState.lightmap_textures	= TEXNUM_LIGHTMAPS;
	//	glState.lightmap_textures	= glState.texture_extension_number;
	//	glState.texture_extension_number = glState.lightmap_textures + MAX_LIGHTMAPS;
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
	GL_Bind( glState.lightmap_textures + 0 );
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


/*
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the floating format in blocklights
===============
*/
void R_BuildLightMap (msurface_t *surf, byte *dest, int stride)
{
	int			smax, tmax;
	int			r, g, b, a, max;
	int			i, j, size;
	byte		*lightmap;
	float		scale[4];
	int			nummaps;
	float		*bl;
	lightstyle_t	*style;
	int			monolightmap;

	// if ( surf->texinfo->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP) )
	if ( surf->texinfo->flags & (SURF_SKY|SURF_WARP) )
		VID_Error (ERR_DROP, "R_BuildLightMap called for non-lit surface");

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	size = smax*tmax;
	// FIXME- can this limit be directly increased?		Yep - Knightmare
	if (size > (sizeof(s_blocklights)>>4) )
		VID_Error (ERR_DROP, "Bad s_blocklights size: %d", size);

	// set to full bright if no light data
	if (!surf->samples)
	{
		int maps;

		for (i=0 ; i<size*3 ; i++)
			s_blocklights[i] = 255;
		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
			 maps++)
		{
			style = &r_newrefdef.lightstyles[surf->styles[maps]];
		}
		goto store;
	}

	// count the # of maps
	for ( nummaps = 0 ; nummaps < MAXLIGHTMAPS && surf->styles[nummaps] != 255 ;
		 nummaps++)
		;

	lightmap = surf->samples;

	// add all the lightmaps
	if ( nummaps == 1 )
	{
		int maps;

		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
			 maps++)
		{
			bl = s_blocklights;

			for (i=0 ; i<3 ; i++)
				scale[i] = r_modulate->value*r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];

			if ( scale[0] == 1.0F &&
				 scale[1] == 1.0F &&
				 scale[2] == 1.0F )
			{
				for (i=0 ; i<size ; i++, bl+=3)
				{
					bl[0] = lightmap[i*3+0];
					bl[1] = lightmap[i*3+1];
					bl[2] = lightmap[i*3+2];
				}
			}
			else
			{
				for (i=0 ; i<size ; i++, bl+=3)
				{
					bl[0] = lightmap[i*3+0] * scale[0];
					bl[1] = lightmap[i*3+1] * scale[1];
					bl[2] = lightmap[i*3+2] * scale[2];
				}
			}
			lightmap += size*3;		// skip to next lightmap
		}
	}
	else
	{
		int maps;

		memset( s_blocklights, 0, sizeof( s_blocklights[0] ) * size * 3 );

		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
			 maps++)
		{
			bl = s_blocklights;

			for (i=0 ; i<3 ; i++)
				scale[i] = r_modulate->value*r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];

			if ( scale[0] == 1.0F &&
				 scale[1] == 1.0F &&
				 scale[2] == 1.0F )
			{
				for (i=0 ; i<size ; i++, bl+=3 )
				{
					bl[0] += lightmap[i*3+0];
					bl[1] += lightmap[i*3+1];
					bl[2] += lightmap[i*3+2];
				}
			}
			else
			{
				for (i=0 ; i<size ; i++, bl+=3)
				{
					bl[0] += lightmap[i*3+0] * scale[0];
					bl[1] += lightmap[i*3+1] * scale[1];
					bl[2] += lightmap[i*3+2] * scale[2];
				}
			}
			lightmap += size*3;		// skip to next lightmap
		}
	}

	// add all the dynamic lights
	if (surf->dlightframe == r_framecount)
		R_AddDynamicLights (surf);

	// put into texture format
store:
	stride -= (smax<<2);
	bl = s_blocklights;

	monolightmap = r_monolightmap->string[0];

	if ( monolightmap == '0' )
	{
		for (i=0 ; i<tmax ; i++, dest += stride)
		{
			for (j=0 ; j<smax ; j++)
			{
				
				r = Q_ftol( bl[0] );
				g = Q_ftol( bl[1] );
				b = Q_ftol( bl[2] );

				// catch negative lights
				if (r < 0)
					r = 0;
				if (g < 0)
					g = 0;
				if (b < 0)
					b = 0;

				//
				// determine the brightest of the three color components
				//
				if (r > g)
					max = r;
				else
					max = g;
				if (b > max)
					max = b;

				//
				// alpha is ONLY used for the mono lightmap case.  For this reason
				// we set it to the brightest of the color components so that 
				// things don't get too dim.
				//
				a = max;

				//
				// rescale all the color components if the intensity of the greatest
				// channel exceeds 1.0
				//
				if (max > 255)
				{
					float t = 255.0F / max;

					r = r*t;
					g = g*t;
					b = b*t;
					a = a*t;
				}

				dest[0] = r;
				dest[1] = g;
				dest[2] = b;
				dest[3] = a;

				bl += 3;
				dest += 4;
			}
		}
	}
	else
	{
		for (i=0 ; i<tmax ; i++, dest += stride)
		{
			for (j=0 ; j<smax ; j++)
			{
				
				r = Q_ftol( bl[0] );
				g = Q_ftol( bl[1] );
				b = Q_ftol( bl[2] );

				// catch negative lights
				if (r < 0)
					r = 0;
				if (g < 0)
					g = 0;
				if (b < 0)
					b = 0;

				//
				// determine the brightest of the three color components
				//
				if (r > g)
					max = r;
				else
					max = g;
				if (b > max)
					max = b;

				//
				// alpha is ONLY used for the mono lightmap case.  For this reason
				// we set it to the brightest of the color components so that 
				// things don't get too dim.
				//
				a = max;

				//
				// rescale all the color components if the intensity of the greatest
				// channel exceeds 1.0
				//
				if (max > 255)
				{
					float t = 255.0F / max;

					r = r*t;
					g = g*t;
					b = b*t;
					a = a*t;
				}

				//
				// So if we are doing alpha lightmaps we need to set the R, G, and B
				// components to 0 and we need to set alpha to 1-alpha.
				//
				switch ( monolightmap )
				{
				case 'L':
				case 'I':
					r = a;
					g = b = 0;
					break;
				case 'C':
					// try faking colored lighting
					a = 255 - ((r+g+b)/3); //Knightmare changed
					r *= a*0.003921568627450980392156862745098; // /255.0;
					g *= a*0.003921568627450980392156862745098; // /255.0;
					b *= a*0.003921568627450980392156862745098; // /255.0;
					break;
				case 'A':
				default:
					r = g = b = 0;
					a = 255 - a;
					break;
				}

				dest[0] = r;
				dest[1] = g;
				dest[2] = b;
				dest[3] = a;

				bl += 3;
				dest += 4;
			}
		}
	}
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

	if (!r_trans_lighting->value)
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