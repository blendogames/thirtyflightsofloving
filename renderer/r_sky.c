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

// r_sky.c -- sky rendering
// Moved from r_warp.c

#include "r_local.h"

vec3_t	skyclip[6] = {
	{1, 1, 0},
	{1, -1, 0},
	{0, -1, 1},
	{0, 1, 1},
	{1, 0, 1},
	{-1, 0, 1} 
};
int	c_sky;

// 1 = s, 2 = t, 3 = 2048
int	st_to_vec[6][3] =
{
	{3,-1,2},
	{-3,1,2},

	{1,3,2},
	{-1,-3,2},

	{-2,-1,3},		// 0 degrees yaw, look straight up
	{2,-1,-3}		// look straight down

//	{-1,2,3},
//	{1,2,-3}
};

// s = [0]/[2], t = [1]/[2]
int	vec_to_st[6][3] =
{
	{-2,3,1},
	{2,3,-1},

	{1,3,2},
	{-1,3,-2},

	{-2,-1,3},
	{-2,1,-3}

//	{-1,2,3},
//	{1,2,-3}
};

skyInfo_t	r_skyInfo;

//===========================================================================

/*
=================
R_DrawSkyPolygon
=================
*/
void R_DrawSkyPolygon (int nump, vec3_t vecs)
{
	int		i,j;
	vec3_t	v, av;
	float	s, t, dv;
	int		axis;
	float	*vp;

	c_sky++;

	// decide which face it maps to
	VectorCopy (vec3_origin, v);
	for (i = 0, vp = vecs; i < nump; i++, vp += 3)
	{
		VectorAdd (vp, v, v);
	}
	av[0] = fabs(v[0]);
	av[1] = fabs(v[1]);
	av[2] = fabs(v[2]);
	if (av[0] > av[1] && av[0] > av[2])
	{
		if (v[0] < 0)
			axis = 1;
		else
			axis = 0;
	}
	else if (av[1] > av[2] && av[1] > av[0])
	{
		if (v[1] < 0)
			axis = 3;
		else
			axis = 2;
	}
	else
	{
		if (v[2] < 0)
			axis = 5;
		else
			axis = 4;
	}

	// project new texture coords
	for (i = 0; i < nump; i++, vecs += 3)
	{
		j = vec_to_st[axis][2];
		if (j > 0)
			dv = vecs[j - 1];
		else
			dv = -vecs[-j - 1];
		if (dv < 0.001)
			continue;	// don't divide by zero
		j = vec_to_st[axis][0];
		if (j < 0)
			s = -vecs[-j -1] / dv;
		else
			s = vecs[j-1] / dv;
		j = vec_to_st[axis][1];
		if (j < 0)
			t = -vecs[-j -1] / dv;
		else
			t = vecs[j-1] / dv;

		if (s < r_skyInfo.skyMins[0][axis])
			r_skyInfo.skyMins[0][axis] = s;
		if (t < r_skyInfo.skyMins[1][axis])
			r_skyInfo.skyMins[1][axis] = t;
		if (s > r_skyInfo.skyMaxs[0][axis])
			r_skyInfo.skyMaxs[0][axis] = s;
		if (t > r_skyInfo.skyMaxs[1][axis])
			r_skyInfo.skyMaxs[1][axis] = t;
	}
}


/*
=================
R_ClipSkyPolygon
=================
*/
#define	ON_EPSILON		0.1			// point on plane side epsilon
#define	MAX_CLIP_VERTS	64
void R_ClipSkyPolygon (int nump, vec3_t vecs, int stage)
{
	float	*norm;
	float	*v;
	qboolean	front, back;
	float	d, e;
	float	dists[MAX_CLIP_VERTS];
	int		sides[MAX_CLIP_VERTS];
	vec3_t	newv[2][MAX_CLIP_VERTS];
	int		newc[2];
	int		i, j;

	if (nump > MAX_CLIP_VERTS-2)
		VID_Error (ERR_DROP, "R_ClipSkyPolygon: MAX_CLIP_VERTS");
	if (stage == 6)
	{	// fully clipped, so draw it
		R_DrawSkyPolygon (nump, vecs);
		return;
	}

	front = back = false;
	norm = skyclip[stage];
	for (i = 0, v = vecs; i < nump; i++, v += 3)
	{
		d = DotProduct (v, norm);
		if (d > ON_EPSILON)
		{
			front = true;
			sides[i] = SIDE_FRONT;
		}
		else if (d < -ON_EPSILON)
		{
			back = true;
			sides[i] = SIDE_BACK;
		}
		else
			sides[i] = SIDE_ON;
		dists[i] = d;
	}

	if (!front || !back)
	{	// not clipped
		R_ClipSkyPolygon (nump, vecs, stage+1);
		return;
	}

	// clip it
	sides[i] = sides[0];
	dists[i] = dists[0];
	VectorCopy (vecs, (vecs+(i*3)) );
	newc[0] = newc[1] = 0;

	for (i = 0, v = vecs; i < nump; i++, v += 3)
	{
		switch (sides[i])
		{
		case SIDE_FRONT:
			VectorCopy (v, newv[0][newc[0]]);
			newc[0]++;
			break;
		case SIDE_BACK:
			VectorCopy (v, newv[1][newc[1]]);
			newc[1]++;
			break;
		case SIDE_ON:
			VectorCopy (v, newv[0][newc[0]]);
			newc[0]++;
			VectorCopy (v, newv[1][newc[1]]);
			newc[1]++;
			break;
		}

		if (sides[i] == SIDE_ON || sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;

		d = dists[i] / (dists[i] - dists[i+1]);
		for (j = 0; j < 3; j++)
		{
			e = v[j] + d*(v[j+3] - v[j]);
			newv[0][newc[0]][j] = e;
			newv[1][newc[1]][j] = e;
		}
		newc[0]++;
		newc[1]++;
	}

	// continue
	R_ClipSkyPolygon (newc[0], newv[0][0], stage+1);
	R_ClipSkyPolygon (newc[1], newv[1][0], stage+1);
}


/*
=================
R_AddSkySurface
=================
*/
void R_AddSkySurface (msurface_t *surf)
{
	int			i;
	vec3_t		verts[MAX_CLIP_VERTS];
	glpoly_t	*p;

	// calculate vertex values for sky box
	for (p=surf->polys; p; p=p->next)
	{
		for (i=0; i<p->numverts; i++)
		{
			VectorSubtract (p->verts[i].xyz, r_origin, verts[i]);
		}
		R_ClipSkyPolygon (p->numverts, verts[0], 0);
	}
}

//===========================================================================

/*
==============
R_GetSkyDistance
==============
*/
float R_GetSkyDistance (void)
{
	if (r_skyInfo.skyDistance > 0.0f)
		return max(r_skyInfo.skyDistance, MIN_SKYDIST);

	if (r_skydistance != NULL)
		return max(r_skydistance->value, MIN_SKYDIST);

	return DEFAULT_SKYDIST;
}


/*
==============
R_CalcSkyCloudVerts
==============
*/
void R_CalcSkyCloudVerts (void)
{
	int		i, j, curIdx;
	float	skyDomeSize, gridIncrement, radStride;
	float	xScalar, yScalar;
	vec3_t	minVert, vertOfs;

//	skyDomeSize = (r_skydistance != NULL) ? r_skydistance->value : DEFAULT_SKYDIST;
	skyDomeSize = R_GetSkyDistance();
	gridIncrement = 2 * skyDomeSize / (SKY_GRID_NUMVERTS - 1);
	radStride = ((float)M_PI / (float)(SKY_GRID_NUMVERTS - 1));
	VectorSet (minVert, -skyDomeSize, -skyDomeSize, 0.0f);

	for (i = 0; i < SKY_GRID_NUMVERTS; i++)
	{
		xScalar = sin((float)i * radStride);
		for (j = 0; j < SKY_GRID_NUMVERTS; j++) {
			yScalar = sin((float)j * radStride);
			curIdx = i * SKY_GRID_NUMVERTS + j;
			VectorSet (vertOfs, gridIncrement * i, gridIncrement * j, xScalar * yScalar * skyDomeSize * 0.25f);
			VectorAdd (minVert, vertOfs, r_skyInfo.cloudVerts[curIdx]);
		}
	}
	r_skyInfo.cloudVertsInitialized = true;
}


/*
==============
R_CalcSkyCloudTexCoords
==============
*/
void R_CalcSkyCloudTexCoords (float cloudTileSize)
{
	int		i, j, curIdx;
	float	texIncrement;

	r_skyInfo.cloudTileSize = cloudTileSize;
	texIncrement = r_skyInfo.cloudTileSize / (SKY_GRID_NUMVERTS - 1);

	for (i = 0; i < SKY_GRID_NUMVERTS; i++) {
		for (j = 0; j < SKY_GRID_NUMVERTS; j++) {
			curIdx = i * SKY_GRID_NUMVERTS + j;
			r_skyInfo.cloudBaseTexCoords[curIdx][0] = texIncrement * i;
			r_skyInfo.cloudBaseTexCoords[curIdx][1] = texIncrement * j;
		}
	}
}


/*
==============
R_DrawSkyClouds
==============
*/
void R_DrawSkyClouds (float texScale, float scroll, vec4_t color, qboolean last)
{
	int		i, j, curIdx, ti, tj, nv;
	float	du, dv;

	if ( !r_skyInfo.cloudVertsInitialized )
		R_CalcSkyCloudVerts ();

	if (r_skyInfo.cloudTileSize != r_skyInfo.cloudTile[0])
		R_CalcSkyCloudTexCoords (r_skyInfo.cloudTile[0]);

	// calc scroll increments
	du = r_skyInfo.cloudDir[0] * r_skyInfo.cloudTileSize * scroll;
	dv = r_skyInfo.cloudDir[1] * r_skyInfo.cloudTileSize * scroll;

	// calc scrolled tex coords
	for (i = 0; i < SKY_GRID_NUMVERTS; i++)
	{
		for (j = 0; j < SKY_GRID_NUMVERTS; j++) {
			curIdx = i * SKY_GRID_NUMVERTS + j;
			r_skyInfo.cloudTexCoords[curIdx][0] = texScale * r_skyInfo.cloudBaseTexCoords[curIdx][0] + du;
			r_skyInfo.cloudTexCoords[curIdx][1] = texScale * r_skyInfo.cloudBaseTexCoords[curIdx][1] + dv;
		}
	}

	GL_Bind (r_skyInfo.cloud_image->texnum);

	for (i = 0; i < SKY_GRID_NUMVERTS - 1; i++)
	{
		nv = SKY_GRID_NUMVERTS * 2;
		if ( RB_CheckArrayOverflow(nv, (nv / 2) * 3) )
			RB_RenderMeshGeneric (last);
		for (j = 0; j < SKY_GRID_NUMVERTS - 1; j++)
		{
			indexArray[rb_index++] = rb_vertex + j * 2 + 0;
			indexArray[rb_index++] = rb_vertex + j * 2 + 1;
			indexArray[rb_index++] = rb_vertex + j * 2 + 2;
			indexArray[rb_index++] = rb_vertex + j * 2 + 1;
			indexArray[rb_index++] = rb_vertex + j * 2 + 3;
			indexArray[rb_index++] = rb_vertex + j * 2 + 2;
		}
		for (j = 0; j < SKY_GRID_NUMVERTS; j++)
		{
			ti = i;	tj = j;
			curIdx = ti * SKY_GRID_NUMVERTS + tj;
			VA_SetElem2v(texCoordArray[0][rb_vertex], r_skyInfo.cloudTexCoords[curIdx]);
			VA_SetElem3v(vertexArray[rb_vertex], r_skyInfo.cloudVerts[curIdx]);
			VA_SetElem4v(colorArray[rb_vertex], color);
			rb_vertex++;

			ti = i + 1;	tj = j;
			curIdx = ti * SKY_GRID_NUMVERTS + tj;
			VA_SetElem2v(texCoordArray[0][rb_vertex], r_skyInfo.cloudTexCoords[curIdx]);
			VA_SetElem3v(vertexArray[rb_vertex], r_skyInfo.cloudVerts[curIdx]);
			VA_SetElem4v(colorArray[rb_vertex], color);
			rb_vertex++;
		}
	}
	RB_RenderMeshGeneric (last);
}


/*
==============
R_UpdateSkyLightning
==============
*/
void R_UpdateSkyLightning (void)
{
	float			deltaT, decay, r1, r2;
	skyLightning_t	*l;

	l = &r_skyInfo.lightning;
	if ( !l )
		return;

	deltaT = r_newrefdef.time - r_skyInfo.lightning.lastTime;
	if (deltaT < 0.0f) {
		l->lastTime = r_newrefdef.time;
		return;
	}

	if ( (l->curBright > 0.0f) || (l->curRad > 0.0f) )
	{
		if (l->numFlashes == 1)
			decay = l->decay;
		else
			decay = 20 * l->decay;

		if (l->curBright > 0.0f) {
			l->curBright -= deltaT * decay;
			l->curBright = max(l->curBright, 0.0f);
		}
		if (l->curRad > 0.0f) {
			l->curRad -= deltaT * SKY_GRID_NUMVERTS * decay;
			l->curRad = max(l->curRad, 0.0f);
		}
		l->lastTime = r_newrefdef.time;
	}
	else if (l->numFlashes > 1) {
		l->numFlashes--;
		l->curRad = l->startRad;
		l->curBright = l->startBright;
		l->lastTime = r_newrefdef.time;
	}
	else if (deltaT > r_skyInfo.lightningFreq)	// generate new flash
	{
		if ( (rand() & 3) == 0 )
		{
			// get new values
			r1 = random();	r2 = random();
			if (r1 < 0.25f)
				l->decay = 9.0f;
			else if (r2 < 0.5f)
				l->decay = 4.0f;
			else
				l->decay = 1.0f;

			l->numFlashes = (rand() % 3) + 1;

			r1 = random();	r2 = random();
			if (r1 < 0.5f)
			{
				l->ctr_i = rand() % SKY_GRID_NUMVERTS;	// range 0 -> SKY_GRID_NUMVERTS-1

				if (r2 < 0.5f)
					l->ctr_j = rand() % (SKY_GRID_NUMVERTS / 3);
				else
					l->ctr_j = SKY_GRID_NUMVERTS - (rand() % (SKY_GRID_NUMVERTS / 3)) - 1;
			}
			else
			{
				l->ctr_j = rand() % SKY_GRID_NUMVERTS;	// range 0 -> SKY_GRID_NUMVERTS-1

				if (r2 < 0.5f)
					l->ctr_i = rand() % (SKY_GRID_NUMVERTS / 3);
				else
					l->ctr_i = SKY_GRID_NUMVERTS - (rand() % (SKY_GRID_NUMVERTS / 3)) - 1;
			}

			l->curRad = l->startRad = (float)(SKY_GRID_NUMVERTS - 1) * ((float)rand() / (float)RAND_MAX);
			l->curBright = l->startBright = 2.0f;
		/*	VID_Printf (PRINT_DEVELOPER, "LFlash: startRad %.2f startBright %.2f decay %.2f numFlashes: %i ctr_i %i ctr_j %i\n",
						l->startRad, l->startBright, l->decay, l->numFlashes, l->ctr_i, l->ctr_j); */
		}
		l->lastTime = r_newrefdef.time;
	}
}


/*
==============
R_DrawSkyLightning
==============
*/
void R_DrawSkyLightning (void)
{
	int				i, j, curIdx, ti, tj, nv, radCeil;
	int				iMin, jMin, iMax, jMax;
	float			colorScale;
	vec4_t			color;
	skyLightning_t	*l;

	l = &r_skyInfo.lightning;
	if ( !l )
		return;

	radCeil = ceil(l->curRad);
	colorScale = min(l->curBright, 1) / (radCeil * radCeil);

	iMin = max(l->ctr_i - radCeil, 0);
	jMin = max(l->ctr_j - radCeil, 0);
	iMax = min(l->ctr_i + radCeil, SKY_GRID_NUMVERTS - 1);
	jMax = min(l->ctr_j + radCeil, SKY_GRID_NUMVERTS - 1);

	GL_Bind (glMedia.whiteTexture->texnum);

	for (i = iMin; i < iMax; i++)
	{
		nv = (jMax - jMin + 1) * 2;
		if ( RB_CheckArrayOverflow(nv, (nv / 2) * 3) )
			RB_RenderMeshGeneric (false);
		for (j = jMin; j < jMax; j++)
		{
			indexArray[rb_index++] = rb_vertex + (j - jMin) * 2 + 0;
			indexArray[rb_index++] = rb_vertex + (j - jMin) * 2 + 1;
			indexArray[rb_index++] = rb_vertex + (j - jMin) * 2 + 2;
			indexArray[rb_index++] = rb_vertex + (j - jMin) * 2 + 1;
			indexArray[rb_index++] = rb_vertex + (j - jMin) * 2 + 3;
			indexArray[rb_index++] = rb_vertex + (j - jMin) * 2 + 2;
		}
		for (j = jMin; j <= jMax; j++)
		{
			ti = i;	tj = j;
			curIdx = ti * SKY_GRID_NUMVERTS + tj;
			color[0] = color[1] = color[2] = color[3] = colorScale * max(l->curRad - abs(l->ctr_i - ti), 0) * max(l->curRad - abs(l->ctr_j - tj), 0);
			VA_SetElem2v(texCoordArray[0][rb_vertex], r_skyInfo.cloudTexCoords[curIdx]);
			VA_SetElem3v(vertexArray[rb_vertex], r_skyInfo.cloudVerts[curIdx]);
			VA_SetElem4v(colorArray[rb_vertex], color);
			rb_vertex++;

			ti = i + 1;	tj = j;
			curIdx = ti * SKY_GRID_NUMVERTS + tj;
			color[0] = color[1] = color[2] = color[3] = colorScale * max(l->curRad - abs(l->ctr_i - ti), 0) * max(l->curRad - abs(l->ctr_j - tj), 0);
			VA_SetElem2v(texCoordArray[0][rb_vertex], r_skyInfo.cloudTexCoords[curIdx]);
			VA_SetElem3v(vertexArray[rb_vertex], r_skyInfo.cloudVerts[curIdx]);
			VA_SetElem4v(colorArray[rb_vertex], color);
			rb_vertex++;
		}
	}
	RB_RenderMeshGeneric (false);
}


/*
==============
R_InitSkyBoxInfo
==============
*/
void R_InitSkyBoxInfo (void)
{
	r_skyInfo.cloudTileSize = 0.0f;
	r_skyInfo.cloudVertsInitialized = false;
}

/*
==============
R_ClearSkyBox
==============
*/
void R_ClearSkyBox (void)
{
	int		i;

	for (i = 0; i < 6; i++)
	{
		r_skyInfo.skyMins[0][i] = r_skyInfo.skyMins[1][i] = 99999;
		r_skyInfo.skyMaxs[0][i] = r_skyInfo.skyMaxs[1][i] = -99999;
	}
}


/*
==============
R_MakeSkyVec
==============
*/
void R_MakeSkyVec (float s, float t, int axis, float distScale)
{
	float		skyBoxSize;
	vec3_t		v, b;
	int			j, k;

	// Knightmare- 12/26/2001- variable back clipping plane distance
//	b[0] = s * r_skydistance->value * distScale;
//	b[1] = t * r_skydistance->value * distScale;
//	b[2] = r_skydistance->value * distScale;
	skyBoxSize = R_GetSkyDistance();
	b[0] = s * skyBoxSize * distScale;
	b[1] = t * skyBoxSize * distScale;
	b[2] = skyBoxSize * distScale;

	for (j = 0; j < 3; j++)
	{
		k = st_to_vec[axis][j];
		if (k < 0)
			v[j] = -b[-k - 1];
		else
			v[j] = b[k - 1];
	}

	// avoid bilerp seam
	s = (s + 1) * 0.5;
	t = (t + 1) * 0.5;

	if (s < r_skyInfo.sky_min)
		s = r_skyInfo.sky_min;
	else if (s > r_skyInfo.sky_max)
		s = r_skyInfo.sky_max;
	if (t < r_skyInfo.sky_min)
		t = r_skyInfo.sky_min;
	else if (t > r_skyInfo.sky_max)
		t = r_skyInfo.sky_max;

	t = 1.0 - t;

	VA_SetElem2(texCoordArray[0][rb_vertex], s, t);
	VA_SetElem3v(vertexArray[rb_vertex], v);
	VA_SetElem4(colorArray[rb_vertex], 1.0f, 1.0f, 1.0f, 1.0f);
	rb_vertex++;
}


/*
==============
R_DrawSkyBox
==============
*/
int	skytexorder[6] = {0,2,1,3,4,5};
void R_DrawSkyBox (void)
{
	int			i;
	float		outerDistScale = SKY_DOME_MULT;
	float		timer, cloudTexScale;
	double		unusedVal;
	vec4_t		cloudColor;
	qboolean	drawClouds;

	if (r_skyInfo.skyRotate)
	{	// check for no sky at all
		for (i = 0; i < 6; i++)
		{
			if ( (r_skyInfo.skyMins[0][i] < r_skyInfo.skyMaxs[0][i]) &&
				(r_skyInfo.skyMins[1][i] < r_skyInfo.skyMaxs[1][i]) )
				break;
		}
		if (i == 6)
			return;		// nothing visible

		// hack, forces full sky to draw when rotating
		for (i=0; i<6; i++) {
			r_skyInfo.skyMins[0][i] = -1;
			r_skyInfo.skyMins[1][i] = -1;
			r_skyInfo.skyMaxs[0][i] = 1;
			r_skyInfo.skyMaxs[1][i] = 1;
		}
	}

	qglPushMatrix ();
	qglTranslatef (r_origin[0], r_origin[1], r_origin[2]);
	qglRotatef (r_newrefdef.time * r_skyInfo.skyRotate, r_skyInfo.skyAxis[0], r_skyInfo.skyAxis[1], r_skyInfo.skyAxis[2]);

	drawClouds = (r_skyInfo.cloud_image != glMedia.noTexture);
	if (drawClouds) 
	{
		R_SuspendFog ();

		// draw outer skybox
		for (i=0; i<6; i++)
		{
			if (r_skyInfo.skyMins[0][i] >= r_skyInfo.skyMaxs[0][i]
			|| r_skyInfo.skyMins[1][i] >= r_skyInfo.skyMaxs[1][i])
				continue;

			GL_Bind (r_skyInfo.sky_images[skytexorder[i]]->texnum);

			rb_vertex = rb_index = 0;
			indexArray[rb_index++] = rb_vertex+0;
			indexArray[rb_index++] = rb_vertex+1;
			indexArray[rb_index++] = rb_vertex+2;
			indexArray[rb_index++] = rb_vertex+0;
			indexArray[rb_index++] = rb_vertex+2;
			indexArray[rb_index++] = rb_vertex+3;
			R_MakeSkyVec (r_skyInfo.skyMins[0][i], r_skyInfo.skyMins[1][i], i, outerDistScale);
			R_MakeSkyVec (r_skyInfo.skyMins[0][i], r_skyInfo.skyMaxs[1][i], i, outerDistScale);
			R_MakeSkyVec (r_skyInfo.skyMaxs[0][i], r_skyInfo.skyMaxs[1][i], i, outerDistScale);
			R_MakeSkyVec (r_skyInfo.skyMaxs[0][i], r_skyInfo.skyMins[1][i], i, outerDistScale);
			RB_RenderMeshGeneric (false);
		}

		GL_Enable (GL_BLEND);
		GL_TexEnv (GL_MODULATE);
		GL_ShadeModel (GL_SMOOTH);
		GL_DepthMask (false);

		// draw first cloud layer
		timer = r_newrefdef.time * 0.002f;
		timer = modf(timer, &unusedVal);
		if (r_skyInfo.cloudAlpha[0] > 0.0f) {
			cloudTexScale = 1.0f;
			Vector4Set (cloudColor, 1.0f, 1.0f, 1.0f, r_skyInfo.cloudAlpha[0]);
			R_DrawSkyClouds (cloudTexScale, r_skyInfo.cloudSpeed[0] * timer, cloudColor, (r_skyInfo.cloudAlpha[1] <= 0.0f));
		}

		// draw lightning
		if (r_skyInfo.lightningFreq > 0.0f)
		{
			R_UpdateSkyLightning ();
			if ( (r_skyInfo.lightning.curBright > 0.0f) && (r_skyInfo.lightning.curRad > 0.0f) ) {
				R_DrawSkyLightning ();
			}
		}

		// draw second cloud layer
		if (r_skyInfo.cloudAlpha[1] > 0.0f)
		{
			if ( (r_skyInfo.cloudTile[0] > 0) && (r_skyInfo.cloudTile[1] > 0) )
				cloudTexScale = r_skyInfo.cloudTile[1] / r_skyInfo.cloudTile[0];
			else
				cloudTexScale = 1.0f;
			Vector4Set (cloudColor, 1.0f, 1.0f, 1.0f, r_skyInfo.cloudAlpha[1]);
			R_DrawSkyClouds (cloudTexScale, r_skyInfo.cloudSpeed[1] * timer, cloudColor, (r_skyInfo.cloudAlpha[2] <= 0.0f));
		}

		// draw third cloud layer
		if (r_skyInfo.cloudAlpha[2] > 0.0f)
		{
			if ( (r_skyInfo.cloudTile[0] > 0) && (r_skyInfo.cloudTile[2] > 0) )
				cloudTexScale = r_skyInfo.cloudTile[2] / r_skyInfo.cloudTile[0];
			else
				cloudTexScale = 1.0f;
			Vector4Set (cloudColor, 1.0f, 1.0f, 1.0f, r_skyInfo.cloudAlpha[2]);
			R_DrawSkyClouds (cloudTexScale, r_skyInfo.cloudSpeed[2] * timer, cloudColor, true);
		}

		GL_ShadeModel (GL_FLAT);
		GL_DepthMask (true);
		R_ResumeFog ();
	}

	R_SetSkyFog (true);	// set sky distance fog

	for (i=0; i<6; i++)
	{
		if (r_skyInfo.skyMins[0][i] >= r_skyInfo.skyMaxs[0][i]
			|| r_skyInfo.skyMins[1][i] >= r_skyInfo.skyMaxs[1][i])
			continue;

		GL_Bind (r_skyInfo.sky_images[skytexorder[i]]->texnum);

		rb_vertex = rb_index = 0;
		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+1;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+3;
		R_MakeSkyVec (r_skyInfo.skyMins[0][i], r_skyInfo.skyMins[1][i], i, 1.0f);
		R_MakeSkyVec (r_skyInfo.skyMins[0][i], r_skyInfo.skyMaxs[1][i], i, 1.0f);
		R_MakeSkyVec (r_skyInfo.skyMaxs[0][i], r_skyInfo.skyMaxs[1][i], i, 1.0f);
		R_MakeSkyVec (r_skyInfo.skyMaxs[0][i], r_skyInfo.skyMins[1][i], i, 1.0f);
		RB_RenderMeshGeneric (true);
	}

	R_SetSkyFog (false);	// restore normal distance fog

	if (drawClouds) {
		GL_Disable (GL_BLEND);
		GL_TexEnv (GL_REPLACE);
	}

	qglPopMatrix ();
}


/*
============
R_SetSky
============
*/
// 3dstudio environment map names
char	*suf[6] = {"rt", "bk", "lf", "ft", "up", "dn"};
void R_SetSky (const char *skyName, const char *cloudName, float rotate, vec3_t axis, float distance,
			   float lightningFreq, vec2_t cloudDir, vec3_t cloudTile, vec3_t cloudSpeed, vec3_t cloudAlpha)
{
	int		i;
	char	pathname[MAX_QPATH];
	float	imagesize;

	Q_strncpyz (r_skyInfo.skyBoxName, sizeof(r_skyInfo.skyBoxName), skyName);
	r_skyInfo.skyRotate = rotate;
	VectorCopy (axis, r_skyInfo.skyAxis);
	r_skyInfo.skyDistance = distance;

	for (i = 0; i < 6; i++)
	{
		if (r_skymip->integer) {	// take less memory
		//	r_picmip->integer++;	// directly setting cvar values is bad bad bad!!!
			Cvar_SetInteger("r_picmip", r_picmip->integer + r_skymip->integer);
		}

		Com_sprintf (pathname, sizeof(pathname), "env/%s%s.tga", r_skyInfo.skyBoxName, suf[i]);
		r_skyInfo.sky_images[i] = R_FindImage (pathname, it_sky);
		if ( !r_skyInfo.sky_images[i] )
			r_skyInfo.sky_images[i] = glMedia.noTexture;

		if ( (r_skyInfo.sky_images[i]->height == r_skyInfo.sky_images[i]->width) && (r_skyInfo.sky_images[i]->width >= 256) )
			imagesize = r_skyInfo.sky_images[i]->width;
		else
			imagesize = 256.0f;

		if (r_skymip->integer) {	// take less memory
		//	r_picmip->integer--;	// directly setting cvar values is bad bad bad!!!
			Cvar_SetInteger ("r_picmip", r_picmip->integer - r_skymip->integer);
			imagesize = min(512.0f, imagesize); // cap at 512
		//	imagesize = min(1024.0f, imagesize); // cap at 1024
		}
		else
			imagesize = min(1024.0f, imagesize); // cap at 1024
		//	imagesize = min(2048.0f, imagesize); // cap at 2048

		r_skyInfo.sky_min = 1.0f / imagesize; // was 256
		r_skyInfo.sky_max = (imagesize - 1.0f) / imagesize;
	}

	// set cloud texture
	if ( cloudName && (cloudName[0] != 0) ) {
		Q_strncpyz (r_skyInfo.cloudName, sizeof(r_skyInfo.cloudName), cloudName);
		Com_sprintf (pathname, sizeof(pathname), "env/%s.tga", r_skyInfo.cloudName, suf[i]);
		r_skyInfo.cloud_image = R_FindImage (pathname, it_sky);
		if ( !r_skyInfo.cloud_image )
			r_skyInfo.cloud_image = glMedia.noTexture;
	}
	else
		r_skyInfo.cloud_image = glMedia.noTexture;

	// copy info
	r_skyInfo.lightningFreq = lightningFreq;
	Vector2Copy (cloudDir, r_skyInfo.cloudDir);
	VectorCopy (cloudTile, r_skyInfo.cloudTile);
	VectorCopy (cloudSpeed, r_skyInfo.cloudSpeed);
	VectorCopy (cloudAlpha, r_skyInfo.cloudAlpha);

	// set r_farZ
	R_SetFarZ ( R_GetSkyDistance() );

	VID_Printf (PRINT_DEVELOPER, "R_SetSky: skyName '%s' cloudName '%s' rotate %.2f axis (%.2f %.2f %.2f) distance %.0f\n", 
				skyName, cloudName, rotate, axis[0], axis[1], axis[2], distance);
	VID_Printf (PRINT_DEVELOPER, "  lightningFreq %.2f cloudDir (%.2f %.2f) cloudTile (%.2f %.2f %.2f)\n",
				lightningFreq, cloudDir[0], cloudDir[1], cloudTile[0], cloudTile[1], cloudTile[2]);
	VID_Printf (PRINT_DEVELOPER, "  cloudSpeed (%.2f %.2f %.2f) cloudAlpha (%.2f %.2f %.2f)\n",
				cloudSpeed[0], cloudSpeed[1], cloudSpeed[2], cloudAlpha[0], cloudAlpha[1], cloudAlpha[2]);
}
