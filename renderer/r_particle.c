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

// r_particle.c -- particle rendering
// moved from r_main.c

#include "r_local.h"

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

//#define BINARY_PART_SORT

#ifdef BINARY_PART_SORT
/*
===================================================
BINARY PARTICLE SORT
===================================================
*/
int partstosort;
sortedelement_t theparts[MAX_PARTICLES];
sortedelement_t *parts_prerender;
// Is this really needed?
//sortedelement_t *parts_last;

void RenderParticle (particle_t *p);
void RenderDecal (particle_t *p);

void resetPartSortList (void)
{
	partstosort = 0;
	parts_prerender = NULL;
	//parts_last = NULL;
}

qboolean particleClip( float len )
{
	if (r_particle_min->value > 0)
	{
		if (len < r_particle_min->value)
			return true;
	}
	if (r_particle_max->value > 0)
	{
		if (len > r_particle_max->value)
			return true;
	}

	return false;
}

sortedelement_t *NewSortPart ( particle_t *p)
{
	vec3_t distance;
	sortedelement_t *element;

	element = &theparts[partstosort];

	VectorSubtract(p->origin, r_origin, distance);
	VectorCopy(p->origin, element->org);

	element->data	= p;
	element->len	= VectorLength(distance);
	element->left	= NULL;
	element->right	= NULL;

	return element;
}

void AddPartTransTree( particle_t *p )
{
	sortedelement_t *thisPart;

	thisPart = NewSortPart(p);

	if (particleClip(thisPart->len))
		return;
	
	if (parts_prerender)
		ElementAddNode(parts_prerender, thisPart);	
	else
		parts_prerender = thisPart;
	
	//parts_last = thisPart;
	partstosort++;
}

void R_BuildParticleList (void)
{
	int		i;
	resetPartSortList();

	for ( i=0; i < r_newrefdef.num_particles; i++)
		AddPartTransTree(&r_newrefdef.particles[i]);
}

void RenderParticleTree (sortedelement_t *element)
{
	if (!element)
		return;

	RenderParticleTree(element->left);

	if (element->data)
		R_RenderParticle ((particle_t *)element->data);

	RenderParticleTree(element->right);
}
#endif // BINARY_PART_SORT
//===================================================


/*
===================================================

STANDARD PARTICLE SORT

===================================================
*/

typedef struct sortedpart_s
{
	particle_t *p;
	vec_t len;
} sortedpart_t;

sortedpart_t sorted_parts[MAX_PARTICLES];

int transCompare (const void *arg1, const void *arg2 ) 
{
	const sortedpart_t *a1, *a2;
	a1 = (sortedpart_t *) arg1;
	a2 = (sortedpart_t *) arg2;
	if ((r_transrendersort->integer == 1) && a1->p && a2->p)
	{
		int image1, image2;
		// FIXME: use hash of imagenum, blendfuncs, & critical flags
	//	image1 = (a1->p->flags & PART_SPARK) ? 0 : a1->p->image;
	//	image2 = (a2->p->flags & PART_SPARK) ? 0 : a2->p->image;
		image1 = a1->p->image;
		image2 = a2->p->image;
		return (image2 - image1)*10000 + (a2->len - a1->len);
	}
	else
		return (a2->len - a1->len);
}

sortedpart_t NewSortedPart (particle_t *p)
{
	vec3_t result;
	sortedpart_t s_part;

	s_part.p = p;
	VectorSubtract(p->origin, r_origin, result);
	s_part.len = (result[0] * result[0]) + (result[1] * result[1]) + (result[2] * result[2]);

	return s_part;
}


void R_SortParticlesOnList (void)
{
	int i, j=0, rangestart=0, rangecount=1;

	for (i=0; i<r_newrefdef.num_particles; i++)
		sorted_parts[i] = NewSortedPart(&r_newrefdef.particles[i]);

	qsort((void *)sorted_parts, r_newrefdef.num_particles, sizeof(sortedpart_t), transCompare);
}


/*
===================================================

PARTICLE RENDERING

===================================================
*/

#define DEPTHHACK_RANGE_SHORT	0.9985f
#define DEPTHHACK_RANGE_MID		0.975f
#define DEPTHHACK_RANGE_LONG	0.95f

#define DEPTHHACK_NONE	0
#define DEPTHHACK_SHORT	1
#define DEPTHHACK_MID	2
#define DEPTHHACK_LONG	3

void SetParticleOverbright (qboolean toggle);

qboolean	ParticleOverbright;
qboolean	fog_on;
vec3_t		particle_up;
vec3_t		particle_right;
vec3_t		particle_coord[4];
vec3_t		ParticleVec[4];
vec3_t		shadelight;

//===================================================

// Particle batching struct
// used for grouping particles by identical rendering state
// and rendering in vertex arrays
//typedef struct particleRenderState_s particleRenderState_t;
//struct particleRenderState_s
typedef struct
{
	GLenum		polymode;
//	qboolean	texture2D;
	qboolean	overbright;
	qboolean	polygon_offset_fill;
	qboolean	alphatest;

	GLenum		blendfunc_src;
	GLenum		blendfunc_dst;
	int			imagenum;
	int			depth_hack;
} particleRenderState_t;
particleRenderState_t	currentParticleState;

//int		particle_va, particle_index;

/*
===============
CompareParticleRenderState
Compares new particle state to current array
===============
*/
qboolean CompareParticleRenderState (particleRenderState_t *compare)
{
	if (!currentParticleState.polymode || !compare) // not initialized
		return false;
	
	if ( (currentParticleState.polymode != compare->polymode)
	//	|| (currentParticleState.texture2D != compare->texture2D)
		|| (currentParticleState.overbright != compare->overbright)
		|| (currentParticleState.polygon_offset_fill != compare->polygon_offset_fill)
		|| (currentParticleState.alphatest != compare->alphatest)
		|| (currentParticleState.blendfunc_src != compare->blendfunc_src)
		|| (currentParticleState.blendfunc_dst != compare->blendfunc_dst)
		|| (currentParticleState.imagenum != compare->imagenum)
		|| (currentParticleState.depth_hack != compare->depth_hack) )
		return false;

	return true;
	//return (memcmp(compare, &currentParticleState, sizeof(particleRenderState_t)) == 0);
}


/*
===============
R_DrawParticleArrays
===============
*/
void R_DrawParticleArrays (void)
{
	//VID_Printf(PRINT_DEVELOPER, "R_DrawParticleArrays: rendering %i indices with %i verts\n", rb_index, rb_vertex);
	c_part_polys += rb_index / 3;

	RB_RenderMeshGeneric (true);
}


/*
===============
R_CheckParticleRenderState
Draws array and sets new state if next particle is different
===============
*/
void R_CheckParticleRenderState (particleRenderState_t *check, int numVerts, int numIndex)
{
	if (!check)	return; // catch bad pointer

	// check for overflow
	if (RB_CheckArrayOverflow(numVerts, numIndex))
		R_DrawParticleArrays();

	if (!CompareParticleRenderState(check))
	{
		float depthmax;

		if (currentParticleState.polymode) // render particle array
			R_DrawParticleArrays();

		// copy state data
		memcpy(&currentParticleState, check, sizeof(particleRenderState_t));

		// set new render state
		GL_BlendFunc (currentParticleState.blendfunc_src, currentParticleState.blendfunc_dst);

		GL_Bind(currentParticleState.imagenum);
	/*	if (currentParticleState.texture2D) {
			GL_EnableTexture(0);
			GL_Bind(currentParticleState.imagenum);
		}
		else
			GL_DisableTexture(0);
	*/

		switch (currentParticleState.depth_hack)
		{
		case DEPTHHACK_LONG:
			depthmax = gldepthmin + DEPTHHACK_RANGE_LONG*(gldepthmax-gldepthmin);
			break;
		case DEPTHHACK_MID:
			depthmax = gldepthmin + DEPTHHACK_RANGE_MID*(gldepthmax-gldepthmin);
			break;
		case DEPTHHACK_SHORT:
			depthmax = gldepthmin + DEPTHHACK_RANGE_SHORT*(gldepthmax-gldepthmin);
			break;
		case DEPTHHACK_NONE:
		default:
			depthmax = gldepthmax;
			break;
		}
		GL_DepthRange (gldepthmin, depthmax);

		if (currentParticleState.overbright)
			SetParticleOverbright(true);
		else
			SetParticleOverbright(false);

		if (currentParticleState.polygon_offset_fill) {
			GL_Enable(GL_POLYGON_OFFSET_FILL);
			GL_PolygonOffset(-2, -1); 
		}
		else
			GL_Disable(GL_POLYGON_OFFSET_FILL);

		if (currentParticleState.alphatest)
			GL_Enable (GL_ALPHA_TEST);
		else
			GL_Disable (GL_ALPHA_TEST);
	}
	// else just continue adding to array
}


/*
===============
R_BeginParticles
Clears particle render state and sets OpenGL state
===============
*/
void R_BeginParticles (qboolean decals)
{
	if (!decals)
	{
		VectorSet (particle_up, vup[0]    * 0.75f, vup[1]    * 0.75f, vup[2]    * 0.75f);
		VectorSet (particle_right, vright[0] * 0.75f, vright[1] * 0.75f, vright[2] * 0.75f);

		VectorAdd      (particle_up, particle_right, particle_coord[0]);
		VectorSubtract (particle_right, particle_up, particle_coord[1]);
		VectorNegate   (particle_coord[0], particle_coord[2]);
		VectorNegate   (particle_coord[1], particle_coord[3]);

		// no fog on particles
		fog_on = false;
		if (qglIsEnabled(GL_FOG)) // check if fog is enabled
		{
			fog_on = true;
			qglDisable(GL_FOG); // if so, disable it
		}
	}
	// clear particle rendering state
	memset(&currentParticleState, 0, sizeof(particleRenderState_t));
	rb_vertex = rb_index = 0;
	ParticleOverbright = false;

	GL_TexEnv (GL_MODULATE);
	GL_DepthMask   (false);
	GL_Enable (GL_BLEND);
	GL_ShadeModel (GL_SMOOTH);
}


/*
===============
R_FinishParticles
Renders any particles left in the vertex array and resets OpenGL state
===============
*/
void R_FinishParticles (qboolean decals)
{
	if (currentParticleState.polymode && (rb_vertex > 0) && (rb_index > 0))
		R_DrawParticleArrays();

	GL_EnableTexture(0);
	SetParticleOverbright(false);
	GL_DepthRange (gldepthmin, gldepthmax);
	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_DepthMask (true);
	GL_Disable (GL_POLYGON_OFFSET_FILL);
	GL_Disable (GL_BLEND);
	qglColor4f (1,1,1,1);

	if (!decals)
	{	// re-enable fog if it was on
		if (fog_on)
			qglEnable(GL_FOG);
	}
}

//===================================================

/*
===============
TexParticle
===============
*/
int TexParticle (int type)
{	
	// check for out of bounds image num
	type =  max( type, 0 );
	type =  min( type, PARTICLE_TYPES-1 );
	// check for bad particle image num
	if (!glMedia.particletextures[type])
		glMedia.particletextures[type] = glMedia.notexture;
	return glMedia.particletextures[type]->texnum;
}


/*
===============
AngleFind
===============
*/
float AngleFind (float input)
{
	return 180.0/input;
}


/*
===============
SetBeamAngles
===============
*/
void SetBeamAngles (vec3_t start, vec3_t end, vec3_t up, vec3_t right)
{
	vec3_t move, delta;

	VectorSubtract(end, start, move);
	VectorNormalize(move);

	VectorCopy(move, up);
	VectorSubtract(r_newrefdef.vieworg, start, delta);
	CrossProduct(up, delta, right);
	//if(!VectorCompare(right, vec3_origin))
		VectorNormalize(right);
}


/*
===============
SetParticleOverbright
===============
*/
void SetParticleOverbright (qboolean toggle)
{
	if ( (toggle && !ParticleOverbright) || (!toggle && ParticleOverbright) )
	{
		R_SetVertexRGBScale (toggle);
		ParticleOverbright = toggle;
	}
}


/*
===============
GetParticleLight
===============
*/
void GetParticleLight (particle_t *p, vec3_t pos, float lighting, vec3_t shadelight)
{
	int j;
	float lightest = 0;

	if ( (r_fullbright->integer != 0) || !lighting )
	{
		VectorSet(shadelight, p->red, p->green, p->blue);
		return;
	}

	R_LightPoint (pos, shadelight, false);

	shadelight[0]= (lighting*shadelight[0]+(1-lighting)) * p->red;
	shadelight[1]= (lighting*shadelight[1]+(1-lighting)) * p->green;
	shadelight[2]= (lighting*shadelight[2]+(1-lighting)) * p->blue;

	//this cleans up the lighting
	for (j=0; j<3; j++)
		if (shadelight[j] > lightest)
			lightest= shadelight[j];
	if (lightest > 255)
		for (j=0; j<3; j++)
		{
			shadelight[j]*= 255/lightest;
			if (shadelight[j] > 255)
				shadelight[j] = 255;
		}
	for (j=0; j<3; j++)
	{
		if (shadelight[j] < 0)
			shadelight[j] = 0;
	}	
}


/*
===============
R_RenderParticle
===============
*/
void R_RenderParticle (particle_t *p)
{
	float		size, len, lighting = r_particle_lighting->value;
	int			oldrender=0, rendertype=0, numVerts, numIndex;
	
	vec3_t		shadelight, move;
	vec4_t		partColor;
	qboolean	shaded;
	vec2_t		tex_coord[4];
	vec3_t		coord[4];
	vec3_t		angl_coord[4];
	particleRenderState_t	thisPart;

	Vector2Set(tex_coord[0], 0, 1);
	Vector2Set(tex_coord[1], 0, 0);
	Vector2Set(tex_coord[2], 1, 0);
	Vector2Set(tex_coord[3], 1, 1);

	VectorCopy(particle_coord[0], coord[0]);
	VectorCopy(particle_coord[1], coord[1]);
	VectorCopy(particle_coord[2], coord[2]);
	VectorCopy(particle_coord[3], coord[3]);

	size = (p->size>0.1) ? p->size : 0.1;
	shaded = (p->flags & PART_SHADED);

	if (shaded) {
		GetParticleLight (p, p->origin, lighting, shadelight);
		Vector4Set(partColor, shadelight[0]*DIV255, shadelight[1]*DIV255, shadelight[2]*DIV255, p->alpha);
	}
	else
		Vector4Set(partColor, p->red*DIV255, p->green*DIV255, p->blue*DIV255, p->alpha);

/*	if (p->flags & PART_SPARK) {
		thisPart.texture2D = false;
		thisPart.imagenum = 0;
	}
	else {
		thisPart.texture2D = true;
		thisPart.imagenum = TexParticle(p->image);
	}*/

	thisPart.imagenum = TexParticle(p->image);
	thisPart.polymode = GL_TRIANGLES;
	thisPart.overbright = (p->flags & PART_OVERBRIGHT);
	thisPart.polygon_offset_fill = false;
	thisPart.alphatest = false;
	thisPart.blendfunc_src = p->blendfunc_src;
	thisPart.blendfunc_dst = p->blendfunc_dst;

	if (p->flags & PART_DEPTHHACK_SHORT) // nice little poly-peeking - psychospaz
		thisPart.depth_hack = DEPTHHACK_SHORT;
	else if (p->flags & PART_DEPTHHACK_MID)
		thisPart.depth_hack = DEPTHHACK_MID;
	else if (p->flags & PART_DEPTHHACK_LONG)
		thisPart.depth_hack = DEPTHHACK_LONG;
	else
		thisPart.depth_hack = DEPTHHACK_NONE;

	// estimate number of verts for this particle
	if (p->flags & PART_SPARK)
	{	numVerts = numIndex = 3;	}
	else if (p->flags & PART_LIGHTNING) {
		VectorSubtract(p->origin, p->angle, move);
		len = VectorNormalize(move);
		numVerts = numIndex = len / size + 1;
		numVerts *= 4;	numIndex *= 6;
	}
	else // PART_BEAM, PART_DIRECTION, PART_ANGLED, default
	{	numVerts = 4;	numIndex = 6;	}

	// check if render state changed from last particle
	R_CheckParticleRenderState (&thisPart, numVerts, numIndex);

	if (p->flags & PART_SPARK) // single triangles
	{
#if 1
		vec3_t	base, ang_up, ang_right;
		int		i;

		VectorMA (p->origin, -size, p->angle, base);
		SetBeamAngles (base, p->origin, ang_up, ang_right);

		VectorScale (ang_up, size*2.0f, ang_up);
		VectorScale (ang_right, size*0.25f, ang_right);

		VectorSubtract (ang_right, ang_up, angl_coord[0]);
		VectorAdd (ang_up, ang_right, angl_coord[1]);
		VectorNegate (angl_coord[1], angl_coord[1]);
		VectorAdd (p->origin, angl_coord[0], ParticleVec[0]);
		VectorAdd (p->origin, angl_coord[1], ParticleVec[1]);

		VA_SetElem3(vertexArray[rb_vertex], p->origin[0], p->origin[1], p->origin[2]);
		VA_SetElem4(colorArray[rb_vertex], partColor[0], partColor[1], partColor[2], partColor[3]);
		indexArray[rb_index++] = rb_vertex;
		rb_vertex++;

		for (i=0; i<2; i++) {
			VA_SetElem3(vertexArray[rb_vertex], ParticleVec[i][0], ParticleVec[i][1], ParticleVec[i][2]);
			VA_SetElem4(colorArray[rb_vertex], 0, 0, 0, 0);
			indexArray[rb_index++] = rb_vertex;
			rb_vertex++;
		}
#else // QMB style particles (8-sided cone)
		float	angle;
		vec3_t	v;
		int		i, j;

		for (i=1; i<9; i++) {
			j = (i<8) ? i+1 : 0;
			indexArray[rb_index++] = rb_vertex;
			indexArray[rb_index++] = rb_vertex+i;
			indexArray[rb_index++] = rb_vertex+j;
		}

		VA_SetElem3(vertexArray[rb_vertex], p->origin[0], p->origin[1], p->origin[2]);
		VA_SetElem4(colorArray[rb_vertex], partColor[0], partColor[1], partColor[2], partColor[3]);
		rb_vertex++;

		for (i=0; i<8; i++)
		{
			angle = i * 45; // was 22.5;
			v[0]=p->origin[0] - p->angle[0]*size + (particle_right[0]*cos(angle) + particle_up[0]*sin(angle));
			v[1]=p->origin[1] - p->angle[1]*size + (particle_right[1]*cos(angle) + particle_up[1]*sin(angle));
			v[2]=p->origin[2] - p->angle[2]*size + (particle_right[2]*cos(angle) + particle_up[2]*sin(angle));

			VA_SetElem3(vertexArray[rb_vertex], v[0], v[1], v[2]);
			VA_SetElem4(colorArray[rb_vertex], 0, 0, 0, 0);
			rb_vertex++;
		}
#endif
	}
	else if (p->flags & PART_BEAM) // given start and end positions, will have fun here :)
	{
		vec3_t	ang_up, ang_right;
		int		i;
		
		SetBeamAngles(p->angle, p->origin, ang_up, ang_right);
		VectorScale(ang_right, size, ang_right);
		
		VectorAdd(p->origin, ang_right, angl_coord[0]);
		VectorAdd(p->angle, ang_right, angl_coord[1]);
		VectorSubtract(p->angle, ang_right, angl_coord[2]);
		VectorSubtract(p->origin, ang_right, angl_coord[3]);

		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+1;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+3;
		for (i=0; i<4; i++)
		{
			VA_SetElem2(texCoordArray[0][rb_vertex], tex_coord[i][0], tex_coord[i][1]);
			VA_SetElem3(vertexArray[rb_vertex], angl_coord[i][0], angl_coord[i][1], angl_coord[i][2]);
			VA_SetElem4(colorArray[rb_vertex], partColor[0], partColor[1], partColor[2], partColor[3]);
			rb_vertex++;
		}
	}
	else if (p->flags & PART_LIGHTNING) // given start and end positions, will have fun here :)
	{
		float	tlen, halflen, dec = size, warpfactor, warpadd, oldwarpadd=0, warpsize, time, i=0, factor;
		vec3_t	lastvec, thisvec, tempvec;
		vec3_t	ang_up, ang_right, vdelta;
		int		j;
		
		warpsize = dec * 0.4; // Knightmare changed
		warpfactor = M_PI*1.0;
		time = -r_newrefdef.time*20.0;
				
		VectorCopy(move, ang_up);
		VectorSubtract(r_newrefdef.vieworg, p->angle, vdelta);
		CrossProduct(ang_up, vdelta, ang_right);
		if (!VectorCompare(ang_right, vec3_origin))
			VectorNormalize(ang_right);
		
		VectorScale (ang_right, dec, ang_right);
		VectorScale (ang_up, dec, ang_up);
		
		VectorScale(move, dec, move);
		VectorCopy(p->angle, lastvec);
		VectorAdd(lastvec, move, thisvec);
		VectorCopy(thisvec, tempvec);
		
		// lightning starts at point and then flares out
		#define LIGHTNINGWARPFUNCTION 0.25*sin(time+i+warpfactor)*(dec/len)
		
		warpadd = LIGHTNINGWARPFUNCTION;
		factor = 1;
		halflen = len/2.0;
		
		thisvec[0]= (thisvec[0]*2 + crandom()*5)/2;
		thisvec[1]= (thisvec[1]*2 + crandom()*5)/2;
		thisvec[2]= (thisvec[2]*2 + crandom()*5)/2;

		while (len > dec)
		{
			i += warpsize;
			VectorAdd(thisvec, ang_right, angl_coord[0]);
			VectorAdd(lastvec, ang_right, angl_coord[1]);
			VectorSubtract(lastvec, ang_right, angl_coord[2]);
			VectorSubtract(thisvec, ang_right, angl_coord[3]);

			Vector2Set(tex_coord[0], 0+warpadd, 1);
			Vector2Set(tex_coord[1], 0+oldwarpadd, 0);
			Vector2Set(tex_coord[2], 1+oldwarpadd, 0);
			Vector2Set(tex_coord[3], 1+warpadd, 1);

			indexArray[rb_index++] = rb_vertex+0;
			indexArray[rb_index++] = rb_vertex+1;
			indexArray[rb_index++] = rb_vertex+2;
			indexArray[rb_index++] = rb_vertex+0;
			indexArray[rb_index++] = rb_vertex+2;
			indexArray[rb_index++] = rb_vertex+3;
			for (j=0; j<4; j++)
			{
				VA_SetElem2(texCoordArray[0][rb_vertex], tex_coord[j][0], tex_coord[j][1]);
				VA_SetElem3(vertexArray[rb_vertex], angl_coord[j][0], angl_coord[j][1], angl_coord[j][2]);
				VA_SetElem4(colorArray[rb_vertex], partColor[0], partColor[1], partColor[2], partColor[3]);
				rb_vertex++;
			}

		//	tlen = (len<halflen) ? fabs(len-halflen) : fabs(len-halflen);
			tlen = fabs(len-halflen);
			factor = tlen / (size * size);
			
			if (factor > 4)
				factor = 4;
			else if (factor < 1)
				factor = 1;
			
			oldwarpadd = warpadd;
			warpadd = LIGHTNINGWARPFUNCTION; // was 0.30*sin(time+i+warpfactor)
			
			len-=dec;
			VectorCopy(thisvec, lastvec);
			VectorAdd(tempvec, move, tempvec);
			VectorAdd(thisvec, move, thisvec);
			thisvec[0]= ((thisvec[0] + crandom()*size) + tempvec[0]*factor)/(factor+1);
			thisvec[1]= ((thisvec[1] + crandom()*size) + tempvec[1]*factor)/(factor+1);
			thisvec[2]= ((thisvec[2] + crandom()*size) + tempvec[2]*factor)/(factor+1);
		}

		i+=warpsize;
		
		// one more time
		if (len>0)
		{
			VectorAdd(p->origin, ang_right, angl_coord[0]);
			VectorAdd(lastvec, ang_right, angl_coord[1]);
			VectorSubtract(lastvec, ang_right, angl_coord[2]);
			VectorSubtract(p->origin, ang_right, angl_coord[3]);

			Vector2Set(tex_coord[0], 0+warpadd, 1);
			Vector2Set(tex_coord[1], 0+oldwarpadd, 0);
			Vector2Set(tex_coord[2], 1+oldwarpadd, 0);
			Vector2Set(tex_coord[3], 1+warpadd, 1);
		
			indexArray[rb_index++] = rb_vertex+0;
			indexArray[rb_index++] = rb_vertex+1;
			indexArray[rb_index++] = rb_vertex+2;
			indexArray[rb_index++] = rb_vertex+0;
			indexArray[rb_index++] = rb_vertex+2;
			indexArray[rb_index++] = rb_vertex+3;
			for (j=0; j<4; j++)
			{
				VA_SetElem2(texCoordArray[0][rb_vertex], tex_coord[j][0], tex_coord[j][1]);
				VA_SetElem3(vertexArray[rb_vertex], angl_coord[j][0], angl_coord[j][1], angl_coord[j][2]);
				VA_SetElem4(colorArray[rb_vertex], partColor[0], partColor[1], partColor[2], partColor[3]);
				rb_vertex++;
			}
		}
	}
	else if (p->flags & PART_DIRECTION) // streched out in direction- beams etc...
	{
		vec3_t	ang_up, ang_right, vdelta;
		int		i;

		VectorAdd(p->angle, p->origin, vdelta); 
		SetBeamAngles(vdelta, p->origin, ang_up, ang_right);
				
		VectorScale (ang_right, 0.75f, ang_right);
		VectorScale (ang_up, 0.75f * VectorLength(p->angle), ang_up);
		
		VectorAdd      (ang_up, ang_right, angl_coord[0]);
		VectorSubtract (ang_right, ang_up, angl_coord[1]);
		VectorNegate   (angl_coord[0], angl_coord[2]);
		VectorNegate   (angl_coord[1], angl_coord[3]);
		
		VectorMA(p->origin, size, angl_coord[0], ParticleVec[0]);
		VectorMA(p->origin, size, angl_coord[1], ParticleVec[1]);
		VectorMA(p->origin, size, angl_coord[2], ParticleVec[2]);
		VectorMA(p->origin, size, angl_coord[3], ParticleVec[3]);

		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+1;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+3;
		for (i=0; i<4; i++)
		{
			VA_SetElem2(texCoordArray[0][rb_vertex], tex_coord[i][0], tex_coord[i][1]);
			VA_SetElem3(vertexArray[rb_vertex], ParticleVec[i][0], ParticleVec[i][1], ParticleVec[i][2]);
			VA_SetElem4(colorArray[rb_vertex], partColor[0], partColor[1], partColor[2], partColor[3]);
			rb_vertex++;
		}
	}
	else if (p->flags & PART_ANGLED) // facing direction...
	{
		vec3_t	ang_up, ang_right, ang_forward;
		int		j;

		AngleVectors(p->angle, ang_forward, ang_right, ang_up); 
		
		VectorScale (ang_right, 0.75f, ang_right);
		VectorScale (ang_up, 0.75f, ang_up);
		
		VectorAdd      (ang_up, ang_right, angl_coord[0]);
		VectorSubtract (ang_right, ang_up, angl_coord[1]);
		VectorNegate   (angl_coord[0], angl_coord[2]);
		VectorNegate   (angl_coord[1], angl_coord[3]);
		
		VectorMA(p->origin, size, angl_coord[0], ParticleVec[0]);
		VectorMA(p->origin, size, angl_coord[1], ParticleVec[1]);
		VectorMA(p->origin, size, angl_coord[2], ParticleVec[2]);
		VectorMA(p->origin, size, angl_coord[3], ParticleVec[3]);
		
		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+1;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+3;
		for (j=0; j<4; j++)
		{
			VA_SetElem2(texCoordArray[0][rb_vertex], tex_coord[j][0], tex_coord[j][1]);
			VA_SetElem3(vertexArray[rb_vertex], ParticleVec[j][0],ParticleVec[j][1], ParticleVec[j][2]);
			VA_SetElem4(colorArray[rb_vertex], partColor[0], partColor[1], partColor[2], partColor[3]);
			rb_vertex++;
		}
	}
	else // normal sprites
	{
		vec3_t	ang_up, ang_right, ang_forward;
		int		j;

		if (p->angle[2]) // if we have roll, we do calcs
		{
			VectorSubtract(p->origin, r_newrefdef.vieworg, angl_coord[0]);
			
			VecToAngleRolled(angl_coord[0], p->angle[2], angl_coord[1]);
			AngleVectors(angl_coord[1], ang_forward, ang_right, ang_up); 
			
			VectorScale (ang_forward, 0.75f, ang_forward);
			VectorScale (ang_right, 0.75f, ang_right);
			VectorScale (ang_up, 0.75f, ang_up);
			
			VectorAdd	   (ang_up, ang_right, angl_coord[0]);
			VectorSubtract (ang_right, ang_up, angl_coord[1]);
			VectorNegate   (angl_coord[0], angl_coord[2]);
			VectorNegate   (angl_coord[1], angl_coord[3]);
			
			VectorMA(p->origin, size, angl_coord[0], ParticleVec[0]);
			VectorMA(p->origin, size, angl_coord[1], ParticleVec[1]);
			VectorMA(p->origin, size, angl_coord[2], ParticleVec[2]);
			VectorMA(p->origin, size, angl_coord[3], ParticleVec[3]);
		}
		else
		{
			VectorMA(p->origin, size, coord[0], ParticleVec[0]);
			VectorMA(p->origin, size, coord[1], ParticleVec[1]);
			VectorMA(p->origin, size, coord[2], ParticleVec[2]);
			VectorMA(p->origin, size, coord[3], ParticleVec[3]);
		}

		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+1;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+3;
		for (j=0; j<4; j++)
		{
			VA_SetElem2(texCoordArray[0][rb_vertex], tex_coord[j][0], tex_coord[j][1]);
			VA_SetElem3(vertexArray[rb_vertex], ParticleVec[j][0],ParticleVec[j][1], ParticleVec[j][2]);
			VA_SetElem4(colorArray[rb_vertex], partColor[0], partColor[1], partColor[2], partColor[3]);
			rb_vertex++;
		}
	}
}


/*
===============
R_DrawAllParticles
===============
*/
void R_DrawAllParticles (void)
{
	int		i;
	particle_t	*p;

	R_BeginParticles (false);

	for ( i=0; i < r_newrefdef.num_particles; i++)
	{
		if (r_transrendersort->integer && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		{
			p = sorted_parts[i].p;
			if ( r_particledistance->value > 0 && sorted_parts[i].len > (100.0*r_particledistance->value))
				continue;
		}
		else
			p=&r_newrefdef.particles[i];
		R_RenderParticle(p);
	}

	R_FinishParticles (false);
}


#ifdef BINARY_PART_SORT
/*
===============
R_DrawParticles
===============
*/
void R_DrawParticles (sortedelement_t *list)
{
	R_BeginParticles (false);

	RenderParticleTree(list);

	R_FinishParticles (false);
}
#endif // BINARY_PART_SORT


/*
===================================================

DECAL RENDERING

===================================================
*/

/*
===============
R_RenderDecal
===============
*/
void R_RenderDecal (particle_t *p)
{
	float	size, alpha;
	vec2_t	tex_coord[4];
	vec3_t	angl_coord[4];
	vec3_t	ang_up, ang_right, ang_forward, color;
	vec4_t	partColor;
	int		i, j, numVerts, numIndex;
	int		mask, aggregatemask = ~0;			

	particleRenderState_t	thisPart;

	// frustum cull
	if (p->decal)
	{
		for (i=0; i<p->decal->numpolys; i++)
		{
			mask = 0;
			for (j=0; j<4; j++) {
				float dp = DotProduct(frustum[j].normal, p->decal->polys[i]);
				if ( ( dp - frustum[j].dist ) < 0 )
					mask |= (1<<j);
			}
			aggregatemask &= mask;
		}
		if (aggregatemask)
			return;
	}

	size = (p->size>0.1) ? p->size : 0.1;
	alpha = p->alpha;

	if (p->decal)
		thisPart.polygon_offset_fill = true;
	else
		thisPart.polygon_offset_fill = false;

	thisPart.polymode = GL_TRIANGLES;
//	thisPart.texture2D = true;
	thisPart.overbright = false;
	thisPart.alphatest = false;
	thisPart.blendfunc_src = p->blendfunc_src;
	thisPart.blendfunc_dst = p->blendfunc_dst;
	thisPart.imagenum = TexParticle(p->image);
	thisPart.depth_hack = DEPTHHACK_NONE;

	numVerts = (p->decal) ? p->decal->numpolys : 4;
	numIndex = (p->decal) ? (p->decal->numpolys-2)*3 : 4;

	// check if render state changed from last particle
	R_CheckParticleRenderState (&thisPart, numVerts, numIndex);

	if (p->flags & PART_SHADED) {
		GetParticleLight (p, p->origin, r_particle_lighting->value, shadelight);
		VectorSet(color, shadelight[0]*DIV255, shadelight[1]*DIV255, shadelight[2]*DIV255);
	}
	else {
		VectorSet(shadelight, p->red, p->green, p->blue);
		VectorSet(color, p->red*DIV255, p->green*DIV255, p->blue*DIV255);
	}

	if (p->flags & PART_ALPHACOLOR)
		Vector4Set(partColor, color[0]*alpha, color[1]*alpha, color[2]*alpha, alpha);
	else
		Vector4Set(partColor, color[0], color[1], color[2], alpha);

	if (p->decal)
	{	
		for (i = 0; i < p->decal->numpolys-2; i++) {
			indexArray[rb_index++] = rb_vertex;
			indexArray[rb_index++] = rb_vertex+i+1;
			indexArray[rb_index++] = rb_vertex+i+2;
		}
		for (i = 0; i < p->decal->numpolys; i++)
		{
			VA_SetElem2(texCoordArray[0][rb_vertex], p->decal->coords[i][0], p->decal->coords[i][1]);
			VA_SetElem3(vertexArray[rb_vertex], p->decal->polys[i][0], p->decal->polys[i][1], p->decal->polys[i][2]);
			VA_SetElem4(colorArray[rb_vertex], partColor[0], partColor[1], partColor[2], partColor[3]);
			rb_vertex++;
		}
	}
	else
	{
		AngleVectors(p->angle, ang_forward, ang_right, ang_up); 

		VectorScale (ang_right, 0.75f, ang_right);
		VectorScale (ang_up, 0.75f, ang_up);

		VectorAdd      (ang_up, ang_right, angl_coord[0]);
		VectorSubtract (ang_right, ang_up, angl_coord[1]);
		VectorNegate   (angl_coord[0], angl_coord[2]);
		VectorNegate   (angl_coord[1], angl_coord[3]);
		
		VectorMA(p->origin, size, angl_coord[0], ParticleVec[0]);
		VectorMA(p->origin, size, angl_coord[1], ParticleVec[1]);
		VectorMA(p->origin, size, angl_coord[2], ParticleVec[2]);
		VectorMA(p->origin, size, angl_coord[3], ParticleVec[3]);

		Vector2Set (tex_coord[0], 0, 1);
		Vector2Set (tex_coord[1], 0, 0);
		Vector2Set (tex_coord[2], 1, 0);
		Vector2Set (tex_coord[3], 1, 1);

		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+1;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+3;
		for (i=0; i<4; i++)
		{
			VA_SetElem2(texCoordArray[0][rb_vertex], tex_coord[i][0], tex_coord[i][1]);
			VA_SetElem3(vertexArray[rb_vertex], ParticleVec[i][0], ParticleVec[i][1], ParticleVec[i][2]);
			VA_SetElem4(colorArray[rb_vertex], partColor[0], partColor[1], partColor[2], partColor[3]);
			rb_vertex++;
		}
	}
}


/*
===============
R_DrawAllDecals
===============
*/
void R_DrawAllDecals (void)
{
	int		i;
	particle_t *d;

	R_BeginParticles (true);

	for ( i=0; i < r_newrefdef.num_decals; i++)
	{
		d = &r_newrefdef.decals[i];
		if (d->decal) // skip if not going to be visible
			if ((d->decal->node == NULL) || (d->decal->node->visframe != r_visframecount))
				continue;
		R_RenderDecal(d);
	}

	R_FinishParticles (true);
}
