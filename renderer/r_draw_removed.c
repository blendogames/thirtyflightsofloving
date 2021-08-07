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

// r_draw_removed.c - various removed pic drawing functions

#include "r_local.h"

#if 0

/*
=============
R_DrawStretchPic
=============
*/
void R_DrawStretchPic (int x, int y, int w, int h, char *pic, float alpha)
{
	image_t		*gl;
	int			i;
	vec2_t		texCoord[4], verts[4];

	gl = R_DrawFindPic (pic);
	if (!gl)
	{
		VID_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	if (scrap_dirty)
		Scrap_Upload ();

	// Psychospaz's transparent console support
	if (gl->has_alpha || alpha < 1.0)
	{
		GL_Disable (GL_ALPHA_TEST);
		GL_TexEnv (GL_MODULATE);
		GL_Enable (GL_BLEND);
		GL_DepthMask (false);
	}

	GL_Bind (gl->texnum);

	Vector2Set(texCoord[0], gl->sl, gl->tl);
	Vector2Set(texCoord[1], gl->sh, gl->tl);
	Vector2Set(texCoord[2], gl->sh, gl->th);
	Vector2Set(texCoord[3], gl->sl, gl->th);

	Vector2Set(verts[0], x, y);
	Vector2Set(verts[1], x+w, y);
	Vector2Set(verts[2], x+w, y+h);
	Vector2Set(verts[3], x, y+h);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;
	for (i=0; i<4; i++) {
		VA_SetElem2(texCoordArray[0][rb_vertex], texCoord[i][0], texCoord[i][1]);
		VA_SetElem3(vertexArray[rb_vertex], verts[i][0], verts[i][1], 0);
		VA_SetElem4(colorArray[rb_vertex], 1.0, 1.0, 1.0, alpha);
		rb_vertex++;
	}
	RB_RenderMeshGeneric (false);

	// Psychospaz's transparent console support
	if (gl->has_alpha || alpha < 1.0)
	{
		GL_DepthMask (true);
		GL_TexEnv (GL_REPLACE);
		GL_Disable (GL_BLEND);
		GL_Enable (GL_ALPHA_TEST);
	}
}


/*
=============
R_DrawScaledPic
Psychospaz's code for drawing stretched crosshairs
=============
*/
void R_DrawScaledPic (int x, int y, float scale, float alpha, char *pic)
{
	float	xoff, yoff;
	float	scale_x, scale_y;
	image_t *gl;
	int		i;
	vec2_t	texCoord[4], verts[4];

	gl = R_DrawFindPic (pic);

	if (!gl) {
		VID_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	if (scrap_dirty)
		Scrap_Upload ();

	// add alpha support
	if (gl->has_alpha || alpha < 1.0)
	{
		GL_Disable (GL_ALPHA_TEST);
		GL_TexEnv (GL_MODULATE);
		GL_Enable (GL_BLEND);
		GL_DepthMask (false);
	}

	GL_Bind (gl->texnum);

	scale_x = scale_y = scale;
	scale_x *= gl->replace_scale_w; // scale down if replacing a pcx image
	scale_y *= gl->replace_scale_h; // scale down if replacing a pcx image

	Vector2Set(texCoord[0], gl->sl, gl->tl);
	Vector2Set(texCoord[1], gl->sh, gl->tl);
	Vector2Set(texCoord[2], gl->sh, gl->th);
	Vector2Set(texCoord[3], gl->sl, gl->th);

	xoff = gl->width*scale_x-gl->width;
	yoff = gl->height*scale_y-gl->height;

	Vector2Set(verts[0], x, y);
	Vector2Set(verts[1], x+gl->width+xoff, y);
	Vector2Set(verts[2], x+gl->width+xoff, y+gl->height+yoff);
	Vector2Set(verts[3], x, y+gl->height+yoff);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;
	for (i=0; i<4; i++) {
		VA_SetElem2(texCoordArray[0][rb_vertex], texCoord[i][0], texCoord[i][1]);
		VA_SetElem3(vertexArray[rb_vertex], verts[i][0], verts[i][1], 0);
		VA_SetElem4(colorArray[rb_vertex], 1.0, 1.0, 1.0, alpha);
		rb_vertex++;
	}
	RB_RenderMeshGeneric (false);

	if (gl->has_alpha || alpha < 1.0)
	{
		GL_DepthMask (true);
		GL_TexEnv (GL_REPLACE);
		GL_Disable (GL_BLEND);
		GL_Enable (GL_ALPHA_TEST); // add alpha support
	}
}


/*
=============
R_DrawPic
=============
*/
void R_DrawPic (int x, int y, char *pic)
{
	image_t	*gl;
	int		i;
	vec2_t	texCoord[4], verts[4];

	gl = R_DrawFindPic (pic);

	if (!gl) {
		VID_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	if (scrap_dirty)
		Scrap_Upload ();

	GL_Bind (gl->texnum);

	Vector2Set(texCoord[0], gl->sl, gl->tl);
	Vector2Set(texCoord[1], gl->sh, gl->tl);
	Vector2Set(texCoord[2], gl->sh, gl->th);
	Vector2Set(texCoord[3], gl->sl, gl->th);

	Vector2Set(verts[0], x, y);
	Vector2Set(verts[1], x+gl->width, y);
	Vector2Set(verts[2], x+gl->width, y+gl->height);
	Vector2Set(verts[3], x, y+gl->height);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;
	for (i=0; i<4; i++) {
		VA_SetElem2(texCoordArray[0][rb_vertex], texCoord[i][0], texCoord[i][1]);
		VA_SetElem3(vertexArray[rb_vertex], verts[i][0], verts[i][1], 0);
		VA_SetElem4(colorArray[rb_vertex], 1.0, 1.0, 1.0, 1.0);
		rb_vertex++;
	}
	RB_RenderMeshGeneric (false);
}

/*
=============
R_DrawTileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void R_DrawTileClear (int x, int y, int w, int h, char *pic)
{
	image_t	*image;
	int		i;
	vec2_t	texCoord[4], verts[4];

	image = R_DrawFindPic (pic);

	if (!image) {
		VID_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	GL_Bind (image->texnum);
/*
	Vector2Set(texCoord[0], x/64.0, y/64.0);
	Vector2Set(texCoord[1], (x+w)/64.0, y/64.0);
	Vector2Set(texCoord[2], (x+w)/64.0, (y+h)/64.0);
	Vector2Set(texCoord[3], x/64.0, (y+h)/64.0);
*/
	Vector2Set(texCoord[0], (float)x/(float)image->width, (float)y/(float)image->height);
	Vector2Set(texCoord[1], (float)(x+w)/(float)image->width, (float)y/(float)image->height);
	Vector2Set(texCoord[2], (float)(x+w)/(float)image->width, (float)(y+h)/(float)image->height);
	Vector2Set(texCoord[3], (float)x/(float)image->width, (float)(y+h)/(float)image->height);

	Vector2Set(verts[0], x, y);
	Vector2Set(verts[1], x+w, y);
	Vector2Set(verts[2], x+w, y+h);
	Vector2Set(verts[3], x, y+h);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;
	for (i=0; i<4; i++) {
		VA_SetElem2(texCoordArray[0][rb_vertex], texCoord[i][0], texCoord[i][1]);
		VA_SetElem3(vertexArray[rb_vertex], verts[i][0], verts[i][1], 0);
		VA_SetElem4(colorArray[rb_vertex], 1.0, 1.0, 1.0, 1.0);
		rb_vertex++;
	}
	RB_RenderMeshGeneric (false);
}

#endif
