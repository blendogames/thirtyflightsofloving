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

// r_sprite.c -- sprite rendering
// moved from r_main.c

#include "r_local.h"

/*
=============================================================

	SPRITE MODELS

=============================================================
*/

/*
=================
R_DrawSpriteModel
=================
*/
void R_DrawSpriteModel (entity_t *e)
{
	float			alpha = 1.0f;
	vec2_t			texCoord[4];
	vec3_t			point[4];
	dsprite_t		*psprite;
	dsprframe_t		*frame;
	float			*up, *right;
	int				i, baseindex;

	// don't even bother culling, because it's just a single
	// polygon without a surface cache

	psprite = (dsprite_t *)currentmodel->extradata;

	e->frame %= psprite->numframes;

	frame = &psprite->frames[e->frame];

	if (!frame) return;

	// normal sprite
	up = vup;
	right = vright;

	if (e->flags & RF_TRANSLUCENT)
		alpha = e->alpha;

	R_SetVertexOverbrights(true);

	// Psychospaz's additive transparency
	if ((currententity->flags & RF_TRANS_ADDITIVE) && (alpha != 1.0f))
	{ 
		GL_Enable (GL_BLEND);
		GL_TexEnv (GL_MODULATE);
		GL_Disable (GL_ALPHA_TEST);
		GL_DepthMask (false);
		GL_BlendFunc (GL_SRC_ALPHA, GL_ONE);
	}
	else
	{
		GL_TexEnv (GL_MODULATE);
		if (alpha == 1.0f) {
			GL_Enable (GL_ALPHA_TEST);
			GL_DepthMask (true);
		}
		else {
			GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			GL_DepthMask (false);
			GL_Enable (GL_BLEND);
			GL_Disable (GL_ALPHA_TEST);
		}
	}
	GL_Bind(currentmodel->skins[0][e->frame]->texnum);

	Vector2Set(texCoord[0], 0, 1);
	Vector2Set(texCoord[1], 0, 0);
	Vector2Set(texCoord[2], 1, 0);
	Vector2Set(texCoord[3], 1, 1);

	VectorMA (e->origin, -frame->origin_y, up, point[0]);
	VectorMA (point[0], -frame->origin_x, right, point[0]);

	VectorMA (e->origin, frame->height - frame->origin_y, up, point[1]);
	VectorMA (point[1], -frame->origin_x, right, point[1]);

	VectorMA (e->origin, frame->height - frame->origin_y, up, point[2]);
	VectorMA (point[2], frame->width - frame->origin_x, right, point[2]);

	VectorMA (e->origin, -frame->origin_y, up, point[3]);
	VectorMA (point[3], frame->width - frame->origin_x, right, point[3]);

	rb_vertex = rb_index = baseindex = 0;
	for (i=0; i<4; i++) {
		VA_SetElem2(texCoordArray[0][rb_vertex], texCoord[i][0], texCoord[i][1]);
		VA_SetElem3(vertexArray[rb_vertex], point[i][0], point[i][1], point[i][2]);
		VA_SetElem4(colorArray[rb_vertex], 1.0f, 1.0f, 1.0f, alpha);
		rb_vertex++;
	}
	indexArray[rb_index++] = baseindex+0;
	indexArray[rb_index++] = baseindex+1;
	indexArray[rb_index++] = baseindex+2;
	indexArray[rb_index++] = baseindex+0;
	indexArray[rb_index++] = baseindex+2;
	indexArray[rb_index++] = baseindex+3;
	RB_DrawArrays (GL_TRIANGLES);

	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_TexEnv (GL_REPLACE);
	GL_DepthMask (true);
	GL_Disable (GL_ALPHA_TEST);
	GL_Disable (GL_BLEND);

	R_SetVertexOverbrights (false);

	RB_DrawMeshTris (GL_TRIANGLES, 1);
}
