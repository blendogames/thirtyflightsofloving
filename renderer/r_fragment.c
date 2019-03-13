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
// r_fragment.c -- fragment clipping

#include "r_local.h"

/*
=======================================================================

FRAGMENT CLIPPING

=======================================================================
*/

#define	SIDE_FRONT				0
#define	SIDE_BACK				1
#define	SIDE_ON					2

#define ON_EPSILON				0.1

#define MAX_FRAGMENT_POINTS		128
#define MAX_FRAGMENT_PLANES		6

static int				cm_numMarkPoints;
static int				cm_maxMarkPoints;
static vec3_t			*cm_markPoints;

static int				cm_numMarkFragments;
static int				cm_maxMarkFragments;
static markFragment_t	*cm_markFragments;

static cplane_t			cm_markPlanes[MAX_FRAGMENT_PLANES];

static int				cm_markCheckCount;


int PlaneTypeForNormal (const vec3_t normal)
{ 
	if (normal[0] == 1.0)  
		return 0;//PLANE_X;  
	if (normal[1] == 1.0)  
		return 1;//PLANE_Y;  
	if (normal[2] == 1.0)  
		return 2;//PLANE_Z; 
 	return 3;//PLANE_NON_AXIAL; 
} 


float *worldVert (int i, msurface_t *surf)
{
	int e = r_worldmodel->surfedges[surf->firstedge + i];
	if (e >= 0)
		return &r_worldmodel->vertexes[r_worldmodel->edges[e].v[0]].position[0];
	else
		return &r_worldmodel->vertexes[r_worldmodel->edges[-e].v[1]].position[0];

}


/*
=================
R_ClipFragment
=================
*/
static void R_ClipFragment (int numPoints, vec3_t points, int stage, markFragment_t *mf)
{
	int			i;
	float		*p;
	qboolean	frontSide;
	vec3_t		front[MAX_FRAGMENT_POINTS];
	int			f;
	float		dist;
	float		dists[MAX_FRAGMENT_POINTS];
	int			sides[MAX_FRAGMENT_POINTS];
	cplane_t	*plane;

	if (numPoints > MAX_FRAGMENT_POINTS-2)
		VID_Error(ERR_DROP, "Mod_ClipFragment: MAX_FRAGMENT_POINTS hit");

	if (stage == MAX_FRAGMENT_PLANES)
	{
		// Fully clipped
		if (numPoints > 2)
		{
			mf->numPoints = numPoints;
			mf->firstPoint = cm_numMarkPoints;
			
			if (cm_numMarkPoints + numPoints > cm_maxMarkPoints)
				numPoints = cm_maxMarkPoints - cm_numMarkPoints;

			for (i = 0, p = points; i < numPoints; i++, p += 3)
				VectorCopy(p, cm_markPoints[cm_numMarkPoints+i]);

			cm_numMarkPoints += numPoints;
		}

		return;
	}

	frontSide = false;

	plane = &cm_markPlanes[stage];
	for (i = 0, p = points; i < numPoints; i++, p += 3)
	{
		if (plane->type < 3)
			dists[i] = dist = p[plane->type] - plane->dist;
		else
			dists[i] = dist = DotProduct(p, plane->normal) - plane->dist;

		if (dist > ON_EPSILON){
			frontSide = true;
			sides[i] = SIDE_FRONT;
		}
		else if (dist < -ON_EPSILON)
			sides[i] = SIDE_BACK;
		else
			sides[i] = SIDE_ON;
	}

	if (!frontSide)
		return;		// Not clipped

	// Clip it
	dists[i] = dists[0];
	sides[i] = sides[0];
	VectorCopy(points, (points + (i*3)));

	f = 0;

	for (i = 0, p = points; i < numPoints; i++, p += 3){
		switch (sides[i]){
		case SIDE_FRONT:
			VectorCopy(p, front[f]);
			f++;
			break;
		case SIDE_BACK:
			break;
		case SIDE_ON:
			VectorCopy(p, front[f]);
			f++;
			break;
		}

		if (sides[i] == SIDE_ON || sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;

		dist = dists[i] / (dists[i] - dists[i+1]);

		front[f][0] = p[0] + (p[3] - p[0]) * dist;
		front[f][1] = p[1] + (p[4] - p[1]) * dist;
		front[f][2] = p[2] + (p[5] - p[2]) * dist;

		f++;
	}

	// Continue
	R_ClipFragment(f, front[0], stage+1, mf);
}


/*
=================
R_ClipFragmentToSurface
=================
*/
static void R_ClipFragmentToSurface (msurface_t *surf, const vec3_t normal, mnode_t *node)
{
	qboolean planeback = surf->flags & SURF_PLANEBACK;
	int				i;
	float			d;
	vec3_t			points[MAX_FRAGMENT_POINTS];
	markFragment_t	*mf;
	//glpoly_t *bp, *p;

	if (cm_numMarkPoints >= cm_maxMarkPoints || cm_numMarkFragments >= cm_maxMarkFragments)
		return;		// Already reached the limit somewhere else

	d = DotProduct(normal, surf->plane->normal);
	if ((planeback && d > -0.75) || (!planeback && d < 0.75))
		return;		// Greater than X degrees

	if (surf->texinfo->flags & SURF_ALPHATEST) // Alpha test surface- no decals
		return;

	for (i = 2; i < surf->numedges; i++)
	{
		mf = &cm_markFragments[cm_numMarkFragments];
		mf->firstPoint = mf->numPoints = 0;
		mf->node = node; // vis node
		
		VectorCopy(worldVert(0, surf), points[0]);
		VectorCopy(worldVert(i-1, surf), points[1]);
		VectorCopy(worldVert(i, surf), points[2]);

		R_ClipFragment(3, points[0], 0, mf);

		if (mf->numPoints)
		{
			cm_numMarkFragments++;

			if (cm_numMarkPoints >= cm_maxMarkPoints || cm_numMarkFragments >= cm_maxMarkFragments)
				return;
		}
	}
}


/*
=================
R_RecursiveMarkFragments
=================
*/
static void R_RecursiveMarkFragments (const vec3_t origin, const vec3_t normal, float radius, mnode_t *node)
{

	int			i;
	float		dist;
	cplane_t	*plane;
	msurface_t	*surf;

	if (cm_numMarkPoints >= cm_maxMarkPoints || cm_numMarkFragments >= cm_maxMarkFragments)
		return;		// Already reached the limit somewhere else

	if (node->contents != -1)
		return;

	// Find which side of the node we are on
	plane = node->plane;
	if (plane->type < 3)
		dist = origin[plane->type] - plane->dist;
	else
		dist = DotProduct(origin, plane->normal) - plane->dist;
	
	// Go down the appropriate sides
	if (dist > radius)
	{
		R_RecursiveMarkFragments(origin, normal, radius, node->children[0]);
		return;
	}
	if (dist < -radius)
	{
		R_RecursiveMarkFragments(origin, normal, radius, node->children[1]);
		return;
	}
	// Clip against each surface


	surf = r_worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		if (surf->checkCount == cm_markCheckCount)
			continue;	// Already checked this surface in another node

		if (surf->texinfo->flags & (SURF_SKY|SURF_WARP))
			continue;

		surf->checkCount = cm_markCheckCount;

		R_ClipFragmentToSurface(surf, normal, node);
	}
	// Recurse down the children
	R_RecursiveMarkFragments(origin, normal, radius, node->children[0]);
	R_RecursiveMarkFragments(origin, normal, radius, node->children[1]);
}


/*
=================
R_MarkFragments
=================
*/
int R_MarkFragments (const vec3_t origin, const vec3_t axis[3], float radius, int maxPoints, vec3_t *points, int maxFragments, markFragment_t *fragments)
{

	int		i;
	float	dot;

	if (!r_worldmodel || !r_worldmodel->nodes)
		return 0;			// Map not loaded

	cm_markCheckCount++;	// For multi-check avoidance

	// Initialize fragments
	cm_numMarkPoints = 0;
	cm_maxMarkPoints = maxPoints;
	cm_markPoints = points;

	cm_numMarkFragments = 0;
	cm_maxMarkFragments = maxFragments;
	cm_markFragments = fragments;

	// Calculate clipping planes
	for (i = 0; i < 3; i++)
	{
		dot = DotProduct(origin, axis[i]);

		VectorCopy(axis[i], cm_markPlanes[i*2+0].normal);
		cm_markPlanes[i*2+0].dist = dot - radius;
		cm_markPlanes[i*2+0].type = PlaneTypeForNormal(cm_markPlanes[i*2+0].normal);

		VectorNegate(axis[i], cm_markPlanes[i*2+1].normal);
		cm_markPlanes[i*2+1].dist = -dot - radius;
		cm_markPlanes[i*2+1].type = PlaneTypeForNormal(cm_markPlanes[i*2+1].normal);
	}

	// Clip against world geometry
	R_RecursiveMarkFragments(origin, axis[0], radius, r_worldmodel->nodes);

	return cm_numMarkFragments;
}
