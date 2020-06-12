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

// r_light.c

#include "r_local.h"

int	r_dlightframecount;

void vectoangles (vec3_t value1, vec3_t angles);

//#define	DLIGHT_CUTOFF	64	//Knightmare- no longer hard-coded

/*
=============================================================================

DYNAMIC LIGHTS BLEND RENDERING

=============================================================================
*/

#define DLIGHTRADUIS 16.0 // was 32.0

/*
=============
R_AddDlight
=============
*/
void R_AddDlight (dlight_t *light)
{
	int		i, j, baseindex;
	float	a;
	vec3_t	v;
	float	rad;

	rad = light->intensity * 0.35;

	VectorSubtract (light->origin, r_origin, v);

	RB_CheckArrayOverflow (GL_TRIANGLES, DLIGHTRADUIS+1, DLIGHTRADUIS*3);
	baseindex = rb_vertex;

	for (i=0; i<3; i++)
		v[i] = light->origin[i] - vpn[i]*rad;

	VA_SetElem3(vertexArray[rb_vertex], v[0], v[1], v[2]);
	VA_SetElem4(colorArray[rb_vertex], light->color[0]*0.2, light->color[1]*0.2, light->color[2]*0.2, 1.0);
	rb_vertex++;

	for (i=DLIGHTRADUIS; i>0; i--)
	{
		a = i/DLIGHTRADUIS * M_PI*2;
		for (j=0; j<3; j++)
			v[j] = light->origin[j] + vright[j]*cos(a)*rad + vup[j]*sin(a)*rad;

		VA_SetElem3(vertexArray[rb_vertex], v[0], v[1], v[2]);
		VA_SetElem4(colorArray[rb_vertex], 0, 0, 0, 1.0);
		rb_vertex++;
	}

	for (i = 1; i <= DLIGHTRADUIS; i++) {
		indexArray[rb_index++] = baseindex;
		indexArray[rb_index++] = baseindex+i;
		indexArray[rb_index++] = baseindex+1+((i<DLIGHTRADUIS)?i:0);
	}
}

/*
=============
R_RenderDlights
=============
*/
void R_RenderDlights (void)
{
	int		i;
	dlight_t	*l;

	if (!r_flashblend->value)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
											// advanced yet for this frame
	GL_DepthMask (0);
	GL_DisableTexture (0);
	GL_ShadeModel (GL_SMOOTH);
	GL_Enable (GL_BLEND);
	GL_BlendFunc (GL_ONE, GL_ONE);

	rb_vertex = rb_index = 0;

	l = r_newrefdef.dlights;
	for (i=0; i<r_newrefdef.num_dlights; i++, l++)
		R_AddDlight (l);

	RB_DrawArrays (GL_TRIANGLES);
	RB_DrawMeshTris (GL_TRIANGLES, 1);

	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_Disable (GL_BLEND);
	GL_ShadeModel (GL_FLAT);
	GL_EnableTexture (0);
	GL_DepthMask (1);
}


/*
=============================================================================

DYNAMIC LIGHTS

=============================================================================
*/

/*
=============
R_MarkLights
=============
*/
extern cvar_t *r_dlights_normal;
void R_MarkLights (dlight_t *light, int bit, mnode_t *node)
{
	cplane_t	*splitplane;
	float		dist;
	msurface_t	*surf;
	int			i;
	int			sidebit; //Knightmare added

	if (node->contents != -1)
		return;

	splitplane = node->plane;
	dist = DotProduct (light->origin, splitplane->normal) - splitplane->dist;
	
	if (dist > light->intensity - r_lightcutoff->value)	//** DMP var dynalight cutoff
	{
		R_MarkLights (light, bit, node->children[0]);
		return;
	}
	if (dist < -light->intensity + r_lightcutoff->value)	//** DMP var dynalight cutoff
	{
		R_MarkLights (light, bit, node->children[1]);
		return;
	}
		
// mark the polygons
	surf = r_worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		//Knightmare- Discoloda's dynamic light clipping
		if (r_dlights_normal->value)
		{
			dist = DotProduct (light->origin, surf->plane->normal) - surf->plane->dist;
			if (dist >= 0)
				sidebit = 0;
			else
				sidebit = SURF_PLANEBACK;

			if ( (surf->flags & SURF_PLANEBACK) != sidebit)
				continue;
		}
		//end Knightmare

		if (surf->dlightframe != r_dlightframecount)
		{
			surf->dlightbits = bit; // was 0
			surf->dlightframe = r_dlightframecount;
		}
		else
			surf->dlightbits |= bit;
	}

	R_MarkLights (light, bit, node->children[0]);
	R_MarkLights (light, bit, node->children[1]);
}


/*
=============
R_PushDlights
=============
*/
void R_PushDlights (void)
{
	int		i;
	dlight_t	*l;

	if (r_flashblend->value)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
											//  advanced yet for this frame
	l = r_newrefdef.dlights;
	for (i=0 ; i<r_newrefdef.num_dlights ; i++, l++)
		R_MarkLights ( l, 1<<i, r_worldmodel->nodes );
}


/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

vec3_t			pointcolor;
cplane_t		*lightplane;		// used as shadow plane
vec3_t			lightspot;

int RecursiveLightPoint (mnode_t *node, vec3_t start, vec3_t end)
{
	float		front, back, frac;
	int			side;
	cplane_t	*plane;
	vec3_t		mid;
	msurface_t	*surf;
	int			s, t, ds, dt;
	int			i;
	mtexinfo_t	*tex;
	byte		*lightmap;
	int			maps;
	int			r;

	if (node->contents != -1)
		return -1;		// didn't hit anything
	
// calculate mid point

// FIXME: optimize for axial
	plane = node->plane;
	front = DotProduct (start, plane->normal) - plane->dist;
	back = DotProduct (end, plane->normal) - plane->dist;
	side = front < 0;
	
	if ( (back < 0) == side)
		return RecursiveLightPoint (node->children[side], start, end);
	
	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;
	
// go down front side	
	r = RecursiveLightPoint (node->children[side], start, mid);
	if (r >= 0)
		return r;		// hit something
		
	if ( (back < 0) == side )
		return -1;		// didn't hit anuthing
		
// check for impact on this node
	VectorCopy (mid, lightspot);
	lightplane = plane;

	surf = r_worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		if (surf->flags&(SURF_DRAWTURB|SURF_DRAWSKY)) 
			continue;	// no lightmaps

		tex = surf->texinfo;
		
		s = DotProduct (mid, tex->vecs[0]) + tex->vecs[0][3];
		t = DotProduct (mid, tex->vecs[1]) + tex->vecs[1][3];;

		if (s < surf->texturemins[0] ||
		t < surf->texturemins[1])
			continue;
		
		ds = s - surf->texturemins[0];
		dt = t - surf->texturemins[1];
		
		if ( ds > surf->extents[0] || dt > surf->extents[1] )
			continue;

		if (!surf->samples)
			return 0;

		ds >>= 4;
		dt >>= 4;

		lightmap = surf->samples;
		VectorCopy (vec3_origin, pointcolor);
		if (lightmap)
		{
			vec3_t scale;

			lightmap += 3*(dt * ((surf->extents[0]>>4)+1) + ds);

			for (maps=0; maps<MAXLIGHTMAPS && surf->styles[maps]!=255; maps++)
			{
				if (!r_newrefdef.lightstyles || !surf->styles)
					break;
				for (i=0; i<3; i++)
					scale[i] = r_modulate->value*r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];

				pointcolor[0] += lightmap[0] * scale[0] * (1.0/255);
				pointcolor[1] += lightmap[1] * scale[1] * (1.0/255);
				pointcolor[2] += lightmap[2] * scale[2] * (1.0/255);
				lightmap += 3*((surf->extents[0]>>4)+1) *
						((surf->extents[1]>>4)+1);
			}
		}
		
		return 1;
	}

// go down back side
	return RecursiveLightPoint (node->children[!side], mid, end);
}


/*
===============
R_LightPoint
===============
*/

// Knightmare added- Psychospaz's lightmaps on alpha surfaces
void R_MaxColorVec (vec3_t color)
{
	int j;
	float lightest;

	for (j=0;j<3;j++)
		if (color[j]>lightest)
			lightest= color[j];
	if (lightest>255)
		for (j=0;j<3;j++)
			color[j]*= 255/lightest;

	for (j=0;j<3;j++)
	{
		if (color[j]>1) color[j] = 1;
		if (color[j]<0) color[j] = 0;
	}
}


// Knightmare added- Psychospaz's lightmaps on alpha surfaces
void R_LightPoint (vec3_t p, vec3_t color, qboolean isEnt)
{
	vec3_t		end;
	float		r;
	int			lnum, i;
	dlight_t	*dl;
	float		light;
	vec3_t		dist;
	float		add;
	
	if (!r_worldmodel->lightdata)
	{
		color[0] = color[1] = color[2] = 1.0;
		return;
	}
	
	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 2048;
	
	r = RecursiveLightPoint (r_worldmodel->nodes, p, end);
	
	if (r == -1)
	{
		VectorCopy (vec3_origin, color);
	}
	else
	{
		VectorCopy (pointcolor, color);
	}

	//this catches too bright modulated color
	for (i=0;i<3;i++)
		if (color[i]>1) color[i] = 1;

	//
	// add dynamic lights
	//
	light = 0;
	dl = r_newrefdef.dlights;
	for (lnum=0; lnum < r_newrefdef.num_dlights; lnum++, dl++)
	{
		if (dl->spotlight) // spotlights
			continue;

		if (isEnt)
			VectorSubtract (currententity->origin, dl->origin, dist);
		else
			VectorSubtract (p, dl->origin, dist);

		add = dl->intensity - VectorLength(dist);
		add *= (1.0/256);
		if (add > 0)
			VectorMA (color, add, dl->color, color);
	}
	//VectorScale (color, r_modulate->value, color); // Knightmare- this makes ents too bright
	//VectorScale (color, r_modulate->value*1.5f, color); // Knightmare- this makes ents too bright
}


void R_LightPointDynamics (vec3_t p, vec3_t color, m_dlight_t *list, int *amount, int max)
{
	vec3_t		end;
	float		r;
	int			lnum, i, m_dl;
	dlight_t	*dl;
	vec3_t		dist, dlColor;
	float		add;
	
	if (!r_worldmodel->lightdata)
	{
		color[0] = color[1] = color[2] = 1.0;
		return;
	}
	
	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 2048;
	
	r = RecursiveLightPoint (r_worldmodel->nodes, p, end);
	
	if (r == -1)
	{
		VectorCopy (vec3_origin, color);
	}
	else
	{
		VectorCopy (pointcolor, color);
	}

	//this catches too bright modulated color
	for (i=0;i<3;i++)
		if (color[i]>1) color[i] = 1;

	//
	// add dynamic lights
	//
	m_dl = 0;
	dl = r_newrefdef.dlights;
	for (lnum=0 ; lnum<r_newrefdef.num_dlights ; lnum++, dl++)
	{
		if (dl->spotlight) // spotlights
			continue;

		VectorSubtract (dl->origin, p, dist);

		add = dl->intensity - VectorNormalize(dist);
		add *= (DIV256);
		if (add > 0)
		{
			float highest = -1;

			VectorScale(dl->color, add, dlColor);
			for (i=0;i<3;i++)
				if (highest<dlColor[i]) highest = dlColor[i];

			if (m_dl<max)
			{
				list[m_dl].strength = highest;
				VectorCopy(dist, list[m_dl].direction);
				VectorCopy(dlColor, list[m_dl].color);
				m_dl++;
			}
			else
			{
				float	least_val = 10;
				int		least_index = 0;

				for (i=0;i<m_dl;i++)
					if (list[i].strength<least_val)
					{


						least_val = list[i].strength;
						least_index = i;
					}

				VectorAdd (color, list[least_index].color, color);
				list[least_index].strength = highest;
				VectorCopy(dist, list[least_index].direction);
				VectorCopy(dlColor, list[least_index].color);
			}
		}
	}

	*amount = m_dl;
}


void R_SurfLightPoint (msurface_t *surf, vec3_t p, vec3_t color, qboolean baselight)
{
	vec3_t		end;
	float		r;
	int			lnum, i;
	dlight_t	*dl;
	float		light;
	vec3_t		dist;
	float		add;
	// Knightmare- fix for moving surfaces
	qboolean rotated = false;
	vec3_t	dlorigin, temp, forward, right, up;
	entity_t *hostent = NULL;

	if (!r_worldmodel->lightdata)
	{
		color[0] = color[1] = color[2] = 1.0;
		return;
	}

	// Knightmare- fix for moving surfaces
	if (surf)
		hostent = surf->entity;
	if (hostent && (hostent->angles[0] || hostent->angles[1] || hostent->angles[2]))
	{
		rotated = true;
		AngleVectors (hostent->angles, forward, right, up);
	}

	if (baselight)
	{
		end[0] = p[0];
		end[1] = p[1];
		end[2] = p[2] - 2048;
		
		r = RecursiveLightPoint (r_worldmodel->nodes, p, end);
		
		if (r == -1)
			VectorCopy (vec3_origin, color);
		else
			VectorCopy (pointcolor, color);

		//this catches too bright modulated color
		for (i=0;i<3;i++)
			if (color[i]>1) color[i] = 1;
	}
	else
	{
		VectorClear(color);
		//
		// add dynamic lights
		//
		light = 0;
		dl = r_newrefdef.dlights;
		for (lnum=0 ; lnum<r_newrefdef.num_dlights ; lnum++, dl++)
		{
			if (dl->spotlight) //spotlights
				continue;

			// Knightmare- fix for moving surfaces
			VectorCopy (dl->origin, dlorigin);
			if (hostent)
				VectorSubtract (dlorigin, hostent->origin, dlorigin);
			if (rotated)
			{
				VectorCopy (dlorigin, temp);
				dlorigin[0] = DotProduct (temp, forward);
				dlorigin[1] = -DotProduct (temp, right);
				dlorigin[2] = DotProduct (temp, up);
			}
			VectorSubtract (p, dlorigin, dist);
			// end Knightmare
			//VectorSubtract (p, dl->origin, dist);

			add = dl->intensity - VectorLength(dist);
			add *= (DIV256);
			if (add > 0)
			{
				VectorMA (color, add, dl->color, color);
			}
		}
	}
}

//===================================================================
// Knightmare- added Psychospaz's dynamic light-based shadows
/*
===============
R_ShadowLight
===============
*/

void R_ShadowLight (vec3_t pos, vec3_t lightAdd)
{
	int			lnum;
	dlight_t	*dl;
	vec3_t		dist, angle;
	float		add, shadowdist;

	if (!r_worldmodel)
		return;
	if (!r_worldmodel->lightdata) // keep old lame shadow
		return;
	
	VectorClear(lightAdd);

	//
	// add dynamic light shadow angles
	//
	if (r_shadows->value == 2)
	{
		dl = r_newrefdef.dlights;
		for (lnum=0; lnum<r_newrefdef.num_dlights; lnum++, dl++)
		{
			if (dl->spotlight) //spotlights
				continue;

			VectorSubtract (dl->origin, pos, dist);
			add = 0.20*sqrt(dl->intensity - VectorLength(dist));
			if (add > 0) {
				VectorNormalize(dist);
				VectorScale(dist, add, dist);
				VectorAdd (lightAdd, dist, lightAdd);
			}
		}
	}

#if 0
	shadowdist = VectorNormalize(lightAdd);
	if (shadowdist>2048) shadowdist=2048;
	if (shadowdist<=0) return;

	// now rotate according to model yaw
	vectoangles (lightAdd, angle);
	angle[YAW] = -(currententity->angles[YAW]-angle[YAW]);
	AngleVectors (angle, dist, NULL, NULL);
	VectorScale (dist, shadowdist, lightAdd); 
#else
	// Barnes improved code
	shadowdist = VectorNormalize(lightAdd);
	if (shadowdist > 4) shadowdist = 4;
	if (shadowdist < 1) // old style static shadow
	{
		//angle[PITCH] = currententity->angles[PITCH];
		//angle[YAW] =   -currententity->angles[YAW];
		//angle[ROLL] =   currententity->angles[ROLL];
		add = currententity->angles[1]/180*M_PI;
		dist[0] = cos(-add);
		dist[1] = sin(-add);
		dist[2] = 1;
		VectorNormalize (dist);
		shadowdist = 1;
	}
	else // shadow from dynamic lights
	{
		vectoangles (lightAdd, angle);
		angle[YAW] -= currententity->angles[YAW];
		AngleVectors (angle, dist, NULL, NULL);
	}
	VectorScale (dist, shadowdist, lightAdd); 
#endif
}

//===================================================================

static float s_blocklights[146*146*3]; //Knightmare-  was [34*34*3], supports max chop size of 1024?
/*
===============
R_AddDynamicLights
===============
*/

// fixes dynamic lighting for bmodels
void R_AddDynamicLights (msurface_t *surf)
{
	int			lnum;
	int			sd, td;
	float		fdist, frad, fminlight;
	vec3_t		impact, local, dlorigin, temp;
	int			s, t;
	int			i;
	int			smax, tmax;
	mtexinfo_t	*tex;
	dlight_t	*dl;
	float		*pfBL;
	float		fsacc, ftacc;
	qboolean rotated = false;
	vec3_t	forward, right, up;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	tex = surf->texinfo;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2])
	{
		rotated = true;
		AngleVectors (currententity->angles, forward, right, up);
	}

	for (lnum=0 ; lnum<r_newrefdef.num_dlights ; lnum++)
	{
		if ( !(surf->dlightbits & (1<<lnum) ) )
			continue;		// not lit by this light

		dl = &r_newrefdef.dlights[lnum];

		if (dl->spotlight) //spotlights
			continue;

		frad = dl->intensity;

		VectorCopy ( dl->origin, dlorigin );
		VectorSubtract (dlorigin, currententity->origin, dlorigin);

		if (rotated)
		{
			VectorCopy (dlorigin, temp);
			dlorigin[0] = DotProduct (temp, forward);
			dlorigin[1] = -DotProduct (temp, right);
			dlorigin[2] = DotProduct (temp, up);
		}

		fdist = DotProduct (dlorigin, surf->plane->normal) -
				surf->plane->dist;
		frad -= fabs(fdist);
		// rad is now the highest intensity on the plane

		fminlight = r_lightcutoff->value; 	//** DMP var dynalight cutoff
		if (frad < fminlight)
			continue;
		fminlight = frad - fminlight;

		for (i=0 ; i<3 ; i++)
		{
			impact[i] = dlorigin[i] -
					surf->plane->normal[i]*fdist;
		}

		local[0] = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3] - surf->texturemins[0];
		local[1] = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3] - surf->texturemins[1];

		pfBL = s_blocklights;
		for (t = 0, ftacc = 0 ; t<tmax ; t++, ftacc += 16)
		{
			td = local[1] - ftacc;
			if ( td < 0 )
				td = -td;

			for ( s=0, fsacc = 0 ; s<smax ; s++, fsacc += 16, pfBL += 3)
			{
				sd = Q_ftol( local[0] - fsacc );

				if ( sd < 0 )
					sd = -sd;

				if (sd > td)
					fdist = sd + (td>>1);
				else
					fdist = td + (sd>>1);

				if ( fdist < fminlight )
				{
					pfBL[0] += ( frad - fdist ) * dl->color[0];
					pfBL[1] += ( frad - fdist ) * dl->color[1];
					pfBL[2] += ( frad - fdist ) * dl->color[2];
				}
			}
		}
	}
}

/*void R_AddDynamicLights (msurface_t *surf)
{
	int			lnum;
	int			sd, td;
	float		fdist, frad, fminlight;
	vec3_t		impact, local, dlorigin, temp;
	int			s, t;
	int			i;
	int			smax, tmax;
	mtexinfo_t	*tex;
	dlight_t	*dl;
	float		*pfBL;
	float		fsacc, ftacc;
	qboolean rotated = false;
	vec3_t	forward, right, up;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	tex = surf->texinfo;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2])
	{
		rotated = true;
		AngleVectors (currententity->angles, forward, right, up);
	}

	for (lnum=0 ; lnum<r_newrefdef.num_dlights ; lnum++)
	{
		if ( !(surf->dlightbits & (1<<lnum) ) )
			continue;		// not lit by this light

		dl = &r_newrefdef.dlights[lnum];

		if (dl->spotlight) //spotlights
			continue;

		frad = dl->intensity;

		VectorCopy ( dl->origin, dlorigin );
		VectorSubtract (dlorigin, currententity->origin, dlorigin);

		if (rotated)
		{
			VectorCopy (dlorigin, temp);
			dlorigin[0] = DotProduct (temp, forward);
			dlorigin[1] = -DotProduct (temp, right);
			dlorigin[2] = DotProduct (temp, up);
		}

		fdist = DotProduct (dl->origin, surf->plane->normal) -
				surf->plane->dist;
		frad -= fabs(fdist);
		// rad is now the highest intensity on the plane

		fminlight = r_lightcutoff->value; 	//** DMP var dynalight cutoff
		if (frad < fminlight)
			continue;
		fminlight = frad - fminlight;

		for (i=0 ; i<3 ; i++)
		{
			impact[i] = dl->origin[i] -
					surf->plane->normal[i]*fdist;
		}

		local[0] = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3] - surf->texturemins[0];
		local[1] = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3] - surf->texturemins[1];

		pfBL = s_blocklights;
		for (t = 0, ftacc = 0 ; t<tmax ; t++, ftacc += 16)
		{
			td = local[1] - ftacc;
			if ( td < 0 )
				td = -td;

			for ( s=0, fsacc = 0 ; s<smax ; s++, fsacc += 16, pfBL += 3)
			{
				sd = Q_ftol( local[0] - fsacc );

				if ( sd < 0 )
					sd = -sd;

				if (sd > td)
					fdist = sd + (td>>1);
				else
					fdist = td + (sd>>1);

				if ( fdist < fminlight )
				{
					pfBL[0] += ( frad - fdist ) * dl->color[0];
					pfBL[1] += ( frad - fdist ) * dl->color[1];
					pfBL[2] += ( frad - fdist ) * dl->color[2];
					// Knightmare- fix dynamic lights
					//pfBL[0] += ( fminlight - fdist ) * dl->color[0];
					//pfBL[1] += ( fminlight - fdist ) * dl->color[1];
					//pfBL[2] += ( fminlight - fdist ) * dl->color[2];
				}
			}
		}
	}
}*/


/*
** R_SetCacheState
*/
void R_SetCacheState( msurface_t *surf )
{
	int maps;

	for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
		 maps++)
	{
		surf->cached_light[maps] = r_newrefdef.lightstyles[surf->styles[maps]].white;
	}
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

	if ( surf->texinfo->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP) )
	//if ( surf->texinfo->flags & (SURF_SKY|SURF_WARP) )
		VID_Error (ERR_DROP, "R_BuildLightMap called for non-lit surface");

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	size = smax*tmax;
	// FIXME- can this limit be directly increased?		Yep - Knightmare
	if (size > (sizeof(s_blocklights)>>4) )
		VID_Error (ERR_DROP, "Bad s_blocklights size");

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

				/*
				** determine the brightest of the three color components
				*/
				if (r > g)
					max = r;
				else
					max = g;
				if (b > max)
					max = b;

				/*
				** alpha is ONLY used for the mono lightmap case.  For this reason
				** we set it to the brightest of the color components so that 
				** things don't get too dim.
				*/
				a = max;

				/*
				** rescale all the color components if the intensity of the greatest
				** channel exceeds 1.0
				*/
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

				/*
				** determine the brightest of the three color components
				*/
				if (r > g)
					max = r;
				else
					max = g;
				if (b > max)
					max = b;

				/*
				** alpha is ONLY used for the mono lightmap case.  For this reason
				** we set it to the brightest of the color components so that 
				** things don't get too dim.
				*/
				a = max;

				/*
				** rescale all the color components if the intensity of the greatest
				** channel exceeds 1.0
				*/
				if (max > 255)
				{
					float t = 255.0F / max;

					r = r*t;
					g = g*t;
					b = b*t;
					a = a*t;
				}

				/*
				** So if we are doing alpha lightmaps we need to set the R, G, and B
				** components to 0 and we need to set alpha to 1-alpha.
				*/
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


//bleh, need this
void vectoangles (vec3_t value1, vec3_t angles)
{
	float	forward;
	float	yaw, pitch;
	
	if (value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
	// PMM - fixed to correct for pitch of 0
		if (value1[0])
			yaw = (atan2(value1[1], value1[0]) * 180 / M_PI);
		else if (value1[1] > 0)
			yaw = 90;
		else
			yaw = 270;

		if (yaw < 0)
			yaw += 360;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}

