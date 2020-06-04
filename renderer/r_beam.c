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

// r_beam.c -- beam rendering
// moved from r_main.c

#include "r_local.h"


/*
=======================
R_RenderBeam
=======================
*/
void R_RenderBeam (vec3_t start, vec3_t end, float size, float red, float green, float blue, float alpha)
{
	float		len;
	vec3_t		up		= {vup[0]    * 0.75f, vup[1]    * 0.75f, vup[2]    * 0.75f};
	vec3_t		right	= {vright[0] * 0.75f, vright[1] * 0.75f, vright[2] * 0.75f};
	vec3_t		vert[4], ang_up, ang_right, vdelta;
	vec2_t		texCoord[4];
	vec4_t		beamColor;
	int			oldrender=0, rendertype=0, i;

	c_alias_polys += 2;

	GL_TexEnv (GL_MODULATE);
	GL_DepthMask (false);
	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE); // this fixes the black background
	GL_Enable (GL_BLEND);
	GL_ShadeModel (GL_SMOOTH);
	GL_Bind(glMedia.particlebeam->texnum);
	Vector4Set(beamColor, red/255, green/255, blue/255, alpha/255);

	VectorSubtract(start, end, ang_up);
	len = VectorNormalize(ang_up);

	VectorSubtract(r_newrefdef.vieworg, start, vdelta);
	CrossProduct(ang_up, vdelta, ang_right);
	if (!VectorCompare(ang_right, vec3_origin))
		VectorNormalize(ang_right);

	VectorScale (ang_right, size*2, ang_right); // Knightmare- a little narrower, please

	VectorAdd(start, ang_right, vert[0]);
	VectorAdd(end, ang_right, vert[1]);
	VectorSubtract(end, ang_right, vert[2]);
	VectorSubtract(start, ang_right, vert[3]);

	Vector2Set(texCoord[0], 0, 1);
	Vector2Set(texCoord[1], 0, 0);
	Vector2Set(texCoord[2], 1, 0);
	Vector2Set(texCoord[3], 1, 1);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;
	for (i=0; i<4; i++) {
		VA_SetElem2(texCoordArray[0][rb_vertex], texCoord[i][0], texCoord[i][1]);
		VA_SetElem3(vertexArray[rb_vertex], vert[i][0], vert[i][1], vert[i][2]);
		VA_SetElem4(colorArray[rb_vertex], beamColor[0], beamColor[1], beamColor[2], beamColor[3]);
		rb_vertex++;
	}
	RB_DrawArrays ();

	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_TexEnv (GL_MODULATE);
	GL_DepthMask (true);
	GL_Disable (GL_BLEND);

	RB_DrawMeshTris ();
	rb_vertex = rb_index = 0;
}


/*
=======================
R_DrawBeam
=======================
*/
void R_DrawBeam( entity_t *e )
{
	qboolean fog_on = false;

	// Knightmare- no fog on lasers
	if (qglIsEnabled(GL_FOG)) // check if fog is enabled
	{
		fog_on = true;
		qglDisable(GL_FOG); // if so, disable it
	}

	R_RenderBeam( e->origin, e->oldorigin, e->frame, 
		( d_8to24table[e->skinnum & 0xFF] ) & 0xFF,
		( d_8to24table[e->skinnum & 0xFF] >> 8 ) & 0xFF,
		( d_8to24table[e->skinnum & 0xFF] >> 16 ) & 0xFF,
		e->alpha*255 );

	// re-enable fog if it was on
	if (fog_on)
		qglEnable(GL_FOG);
}
