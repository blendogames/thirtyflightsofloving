/*
===========================================================================
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2000-2002 Knightmare

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

// r_backend.c: array and texture coord handling for protoshaders

#include "r_local.h"


#define TABLE_SIZE		1024
#define TABLE_MASK		1023

static float	rb_sinTable[TABLE_SIZE];
static float	rb_triangleTable[TABLE_SIZE];
static float	rb_squareTable[TABLE_SIZE];
static float	rb_sawtoothTable[TABLE_SIZE];
static float	rb_inverseSawtoothTable[TABLE_SIZE];
static float	rb_noiseTable[TABLE_SIZE];

// vertex arrays
unsigned	indexArray[MAX_INDICES];
float		texCoordArray[MAX_TEXTURE_UNITS][MAX_VERTICES][2];
float		vertexArray[MAX_VERTICES][3];
float		colorArray[MAX_VERTICES][4];
float		inTexCoordArray[MAX_VERTICES][2];
float		celTexCoordArray[MAX_VERTICES][2];	// for cel shading

unsigned	rb_vertex, rb_index;

/*
=================
RB_BuildTables
=================
*/
static void RB_BuildTables (void)
{
	int		i;
	float	f;

	for (i = 0; i < TABLE_SIZE; i++)
	{
		f = (float)i / (float)TABLE_SIZE;

		rb_sinTable[i] = sin(f * M_PI2);

		if (f < 0.25)
			rb_triangleTable[i] = 4.0 * f;
		else if (f < 0.75)
			rb_triangleTable[i] = 2.0 - 4.0 * f;
		else
			rb_triangleTable[i] = (f - 0.75) * 4.0 - 1.0;

		if (f < 0.5)
			rb_squareTable[i] = 1.0;
		else
			rb_squareTable[i] = -1.0;

		rb_sawtoothTable[i] = f;

		rb_inverseSawtoothTable[i] = 1.0 - f;

		rb_noiseTable[i] = crand();
	}
}

/*
=================
RB_TableForFunc
=================
*/
static float *RB_TableForFunc (const waveFunc_t *func)
{
	switch (func->type)
	{
	case WAVEFORM_SIN:
		return rb_sinTable;
	case WAVEFORM_TRIANGLE:
		return rb_triangleTable;
	case WAVEFORM_SQUARE:
		return rb_squareTable;
	case WAVEFORM_SAWTOOTH:
		return rb_sawtoothTable;
	case WAVEFORM_INVERSESAWTOOTH:
		return rb_inverseSawtoothTable;
	case WAVEFORM_NOISE:
		return rb_noiseTable;
	}
	VID_Error (ERR_DROP, "RB_TableForFunc: unknown waveform type %i", func->type);
	return rb_sinTable;
}

/*
=================
RB_InitBackend
=================
*/
void RB_InitBackend (void)
{
	// Build waveform tables
	RB_BuildTables();
}

/*
=================
RB_CalcGlowColor
=================
*/
float RB_CalcGlowColor (renderparms_t *parms)
{
	float	*table, rad, out=1.0f;

	if (!parms)	return 0.0f;

	if (parms->glow.type > -1)
	{
		table = RB_TableForFunc(&parms->glow);
		rad = parms->glow.params[2] + parms->glow.params[3] * r_newrefdef.time;
		out = table[((int)(rad * TABLE_SIZE)) & TABLE_MASK] * parms->glow.params[1] + parms->glow.params[0];
		out = max(min(out, 1.0f), 0.0f); // clamp
	}
	return out;
}

/*
=================
RB_ModifyTextureCoords
borrowed from EGL & Q2E
=================
*/
void RB_ModifyTextureCoords (float *inArray, float *inVerts, int numVerts, tcmodParms_t *parms)
{
	int		i;
	float	t1, t2, sint, cost, rad;
	float	*tcArray, *vertArray, *table;

	if (!inArray || !inVerts || !parms)
		return;

	if (parms->translate_x != 0.0f)
		for (tcArray=inArray, i=0; i<numVerts; i++, tcArray+=2)
			tcArray[0] += parms->translate_x;
	if (parms->translate_y != 0.0f)
		for (tcArray=inArray, i=0; i<numVerts; i++, tcArray+=2)
			tcArray[1] += parms->translate_y;

	if (parms->rotate != 0.0f)
	{
		rad = -DEG2RAD(parms->rotate * r_newrefdef.time);
		sint = sin(rad);
		cost = cos(rad);

		for (tcArray=inArray, i=0; i<numVerts; i++, tcArray+=2) {
			t1 = tcArray[0];
			t2 = tcArray[1];
			tcArray[0] = cost * (t1 - 0.5) - sint * (t2 - 0.5) + 0.5;
			tcArray[1] = cost * (t2 - 0.5) + sint * (t1 - 0.5) + 0.5;
		}
	}

	if (parms->stretch.type > -1)
	{
		table = RB_TableForFunc(&parms->stretch);
		rad = parms->stretch.params[2] + parms->stretch.params[3] * r_newrefdef.time;
		t1 = table[((int)(rad * TABLE_SIZE)) & TABLE_MASK] * parms->stretch.params[1] + parms->stretch.params[0];
		
		t1 = (t1) ? 1.0 / t1 : 1.0;
		t2 = 0.5 - 0.5 * t1;

		for (tcArray=inArray, i=0; i<numVerts; i++, tcArray+=2) {
			tcArray[0] = tcArray[0] * t1 + t2;
			tcArray[1] = tcArray[1] * t1 + t2;
		}
	}

	if (parms->scale_x != 1.0f)
		for (tcArray=inArray, i=0; i<numVerts; i++, tcArray+=2)
			tcArray[0] = tcArray[0] / parms->scale_x;
	if (parms->scale_y != 1.0f)
		for (tcArray=inArray, i=0; i<numVerts; i++, tcArray+=2)
			tcArray[1] = tcArray[1] / parms->scale_y;

	if (parms->turb.type > -1)
	{
		table = RB_TableForFunc(&parms->turb);
		t1 = parms->turb.params[2] + parms->turb.params[3] * r_newrefdef.time;

		for (tcArray=inArray, vertArray=inVerts, i=0; i<numVerts; i++, tcArray+=2, vertArray+=3) {
			tcArray[0] += (table[((int)(((vertArray[0] + vertArray[2]) * 1.0/128 * 0.125 + t1) * TABLE_SIZE)) & TABLE_MASK] * parms->turb.params[1] + parms->turb.params[0]);
			tcArray[1] += (table[((int)(((vertArray[1]) * 1.0/128 * 0.125 + t1) * TABLE_SIZE)) & TABLE_MASK] * parms->turb.params[1] + parms->turb.params[0]);
		}
	}

	if (parms->scroll_x != 0.0f)
		for (tcArray=inArray, i=0; i<numVerts; i++, tcArray+=2)
			tcArray[0] += r_newrefdef.time * parms->scroll_x;
	if (parms->scroll_y != 0.0f)
		for (tcArray=inArray, i=0; i<numVerts; i++, tcArray+=2)
			tcArray[1] += r_newrefdef.time * parms->scroll_y;
}

/*
=================
RB_CheckArrayOverflow
=================
*/
qboolean RB_CheckArrayOverflow (int numVerts, int numIndex)
{
	if (rb_vertex == 0 || rb_index == 0)	return false;  // nothing to purge

	if (numVerts > MAX_VERTICES || numIndex > MAX_INDICES)
		Com_Error(ERR_DROP, "RB_CheckArrayOverflow: %i > MAX_VERTICES or %i > MAX_INDICES", numVerts, numIndex);

	if (rb_vertex + numVerts <= MAX_VERTICES && rb_index + numIndex <= MAX_INDICES)
		return false;

//	VID_Printf(PRINT_DEVELOPER, "RB_CheckArrayOverflow: purging %i verts and %i indices to prevent overflow from %i, %i more\n", rb_vertex, rb_index, numVerts, numIndex);

	return true;
}

/*
=================
RB_DrawArrays
=================
*/
void RB_DrawArrays (void)
{
	if (rb_vertex == 0 || rb_index == 0) // nothing to render
		return;

	GL_LockArrays (rb_vertex);
	if (glConfig.drawRangeElements)
		qglDrawRangeElementsEXT(GL_TRIANGLES, 0, rb_vertex, rb_index, GL_UNSIGNED_INT, indexArray);
	else
		qglDrawElements(GL_TRIANGLES, rb_index, GL_UNSIGNED_INT, indexArray);
	GL_UnlockArrays ();
}

/*
=============
RB_DrawMeshTris
Re-draws a mesh in outline mode
=============
*/
void RB_DrawMeshTris (void)
{
	int i, numTMUs = 0;

	if (!r_showtris->integer)
		return;

	if (r_showtris->integer == 1)
		GL_Disable(GL_DEPTH_TEST);

	qglPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	for (i=0; i<glConfig.max_texunits; i++)
		if (glState.activetmu[i])
		{	numTMUs++;		GL_DisableTexture (i);	}
	qglDisableClientState (GL_COLOR_ARRAY);
	qglColor4f(1.0, 1.0, 1.0, 1.0);

	RB_DrawArrays ();

	qglEnableClientState (GL_COLOR_ARRAY);
	for (i=0; i<numTMUs; i++)
		GL_EnableTexture(i);
	qglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

	if (r_showtris->integer == 1)
		GL_Enable(GL_DEPTH_TEST);
}

/*
=================
RB_RenderMeshGeneric
=================
*/
void RB_RenderMeshGeneric (qboolean drawTris)
{
	RB_DrawArrays ();
	if (drawTris)
		RB_DrawMeshTris ();
	rb_vertex = rb_index = 0;
}
