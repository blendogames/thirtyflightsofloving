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

// r_draw.c - 2D image drawing

#include "r_local.h"

image_t		*draw_chars;

extern	qboolean	scrap_dirty;
void Scrap_Upload (void);

#define DEFAULT_FONT_SIZE 8.0f

void RefreshFont (void)
{
	con_font->modified = false;

	draw_chars = R_FindImage (va("fonts/%s.pcx", con_font->string), it_pic);
	if (!draw_chars) // fall back on default font
		draw_chars = R_FindImage ("fonts/default.pcx", it_pic);
	if (!draw_chars) // fall back on old Q2 conchars
		draw_chars = R_FindImage ("pics/conchars.pcx", it_pic);
	if (!draw_chars) // prevent crash caused by missing font
		VID_Error (ERR_FATAL, "RefreshFont: couldn't load pics/conchars");

	GL_Bind( draw_chars->texnum );
}


/*
===============
R_DrawInitLocal
===============
*/
void R_DrawInitLocal (void)
{
	image_t	*R_DrawFindPic (char *name);

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// load console characters (don't bilerp characters)
	RefreshFont();

	R_InitChars (); // init char indexes
}


/*
================
R_CharMapScale
================
*/
float R_CharMapScale (void)
{
	return (draw_chars->width/128.0); //current width / original width
}


unsigned	char_count;
/*
================
R_InitChars
================
*/
void R_InitChars (void)
{
	char_count = 0;
}

/*
================
R_FlushChars
================
*/
void R_FlushChars (void)
{
	if (rb_vertex == 0 || rb_index == 0) // nothing to flush
		return;

	GL_Disable (GL_ALPHA_TEST);
	GL_TexEnv (GL_MODULATE);
	GL_Enable (GL_BLEND);
	GL_DepthMask (false);
	GL_Bind(draw_chars->texnum);

	RB_RenderMeshGeneric (false);
	char_count = 0;

	GL_DepthMask (true);
	GL_Disable (GL_BLEND);
	GL_TexEnv (GL_REPLACE);
	GL_Enable (GL_ALPHA_TEST);
}

/*
================
R_DrawChar
Draws one variable sized graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void R_DrawChar (float x, float y, int num, float scale, 
	 int red, int green, int blue, int alpha, qboolean italic, qboolean last)
{
	int			row, col, i;
	float		frow, fcol, size, cscale, italicAdd;
	vec2_t		texCoord[4], verts[4];
	qboolean	addChar = true;

	num &= 255;

	if (alpha > 255)
		alpha = 255;
	else if (alpha < 1)
		alpha = 1;

	if ((num & 127) == 32)	// space
		addChar = false;
	if (y <= -(scale * DEFAULT_FONT_SIZE))	// totally off screen
		addChar = false;

	row = num >> 4;
	col = num&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;
	cscale = scale * DEFAULT_FONT_SIZE;

	italicAdd = (italic) ? (cscale*0.25) : 0;

	if (addChar)
	{
		Vector2Set(texCoord[0], fcol, frow);
		Vector2Set(texCoord[1], fcol + size, frow);
		Vector2Set(texCoord[2], fcol + size, frow + size);
		Vector2Set(texCoord[3], fcol, frow + size);

		Vector2Set(verts[0], x+italicAdd, y);
		Vector2Set(verts[1], x+cscale+italicAdd, y);
		Vector2Set(verts[2], x+cscale-italicAdd, y+cscale);
		Vector2Set(verts[3], x-italicAdd, y+cscale);

		if (char_count == 0)
			rb_vertex = rb_index = 0;
		if (rb_vertex + 4 >= MAX_VERTICES || rb_index + 6 >= MAX_INDICES)
			R_FlushChars ();
		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+1;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+0;
		indexArray[rb_index++] = rb_vertex+2;
		indexArray[rb_index++] = rb_vertex+3;
		for (i=0; i<4; i++) {
			VA_SetElem2(texCoordArray[0][rb_vertex], texCoord[i][0], texCoord[i][1]);
			VA_SetElem3(vertexArray[rb_vertex], verts[i][0], verts[i][1], 0);
			VA_SetElem4(colorArray[rb_vertex], red*DIV255, green*DIV255, blue*DIV255, alpha*DIV255);
			rb_vertex++;
		}
		char_count++;
	}
	if (last)
		R_FlushChars ();
}


/*
=============
R_DrawFindPic
=============
*/
image_t	*R_DrawFindPic (char *name)
{
	image_t *gl;
	char	fullname[MAX_QPATH];

	if (name[0] != '/' && name[0] != '\\')
	{
		Com_sprintf (fullname, sizeof(fullname), "pics/%s.pcx", name);
		gl = R_FindImage (fullname, it_pic);
	}
	else
		gl = R_FindImage (name+1, it_pic);
	/* jitfix
	if (!gl) // jit -- remove "can't find pic" spam
		return glMedia.notexture;
	else 
	end jitfix */
	return gl;
}

/*
=============
R_DrawGetPicSize
=============
*/
void R_DrawGetPicSize (int *w, int *h, char *pic)
{
	image_t *gl;

	gl = R_DrawFindPic (pic);
	if (!gl)
	{
		*w = *h = -1;
		return;
	}
	// Factor in replace scale, so tga/jpg replacements are scaled down...
	*w = (int)((float)gl->width * gl->replace_scale_w);
	*h = (int)((float)gl->height * gl->replace_scale_w);
}


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


/*
======================
R_DrawFill

Fills a box of pixels with a
24-bit color w/ alpha
===========================
*/
void R_DrawFill (int x, int y, int w, int h, int red, int green, int blue, int alpha)
{
	int		i;
	vec2_t	verts[4];

	red = min(red, 255);
	green = min(green, 255);
	blue = min(blue, 255);
	alpha = max(min(alpha, 255), 1);

//	GL_DisableTexture (0);
	GL_Disable (GL_ALPHA_TEST);
	GL_TexEnv (GL_MODULATE);
	GL_Enable (GL_BLEND);
	GL_DepthMask   (false);

	GL_Bind (glMedia.whitetexture->texnum);

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
		VA_SetElem3(vertexArray[rb_vertex], verts[i][0], verts[i][1], 0);
		VA_SetElem4(colorArray[rb_vertex], red*DIV255, green*DIV255, blue*DIV255, alpha*DIV255);
		rb_vertex++;
	}
	RB_RenderMeshGeneric (false);

	GL_DepthMask (true);
	GL_Disable (GL_BLEND);
	GL_TexEnv (GL_REPLACE);
	GL_Enable (GL_ALPHA_TEST);
//	GL_EnableTexture (0);
}

//=============================================================================

/*
=============
R_DrawCameraEffect

Video camera effect
=============
*/
void Mod_SetRenderParmsDefaults (renderparms_t *parms);

void R_DrawCameraEffect (void)
{
	image_t			*image[2];
	int				x, y, w, h, i, j;
	float			texparms[2][4];
	vec2_t			texCoord[4];
	vec3_t			verts[4];
	renderparms_t	cameraParms;

	image[0] = R_DrawFindPic ("/gfx/2d/screenstatic.tga");
	image[1] = R_DrawFindPic ("/gfx/2d/scanlines.tga");

	if (!image[0] || !image[1])
		return;

	x = y = 0; w = vid.width; h = vid.height;
	GL_Disable (GL_ALPHA_TEST);
	GL_TexEnv (GL_MODULATE);
	GL_Enable (GL_BLEND);
	GL_BlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
	GL_DepthMask   (false);

	VectorSet(verts[0], x, y, 0);
	VectorSet(verts[1], x+w, y, 0);
	VectorSet(verts[2], x+w, y+h, 0);
	VectorSet(verts[3], x, y+h, 0);

	Vector4Set(texparms[0], 2, 2, -30, 10);
	Vector4Set(texparms[1], 1, 10, 0, 0);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;
	rb_vertex = 4;

	for (i=0; i<2; i++)
	{
		GL_Bind (image[i]->texnum);
		Vector2Set(texCoord[0], x/image[i]->width, y/image[i]->height);
		Vector2Set(texCoord[1], (x+w)/image[i]->width, y/image[i]->height);
		Vector2Set(texCoord[2], (x+w)/image[i]->width, (y+h)/image[i]->height);
		Vector2Set(texCoord[3], x/image[i]->width, (y+h)/image[i]->height);
		Mod_SetRenderParmsDefaults (&cameraParms);
		cameraParms.scale_x = texparms[i][0];
		cameraParms.scale_y = texparms[i][1];
		cameraParms.scroll_x = texparms[i][2];
		cameraParms.scroll_y = texparms[i][3];
		RB_ModifyTextureCoords (&texCoord[0][0], &verts[0][0], 4, cameraParms);
		for (j=0; j<4; j++) {
			VA_SetElem2(texCoordArray[0][j], texCoord[j][0], texCoord[j][1]);
			VA_SetElem3(vertexArray[j], verts[j][0], verts[j][1], verts[j][2]);
			VA_SetElem4(colorArray[j], 1, 1, 1, 1);
		}
		RB_DrawArrays ();
	}
	rb_vertex = rb_index = 0;

	GL_DepthMask (true);
	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_Disable (GL_BLEND);
	GL_TexEnv (GL_REPLACE);
	GL_Enable (GL_ALPHA_TEST);
}


//=============================================================================

/*
=============
R_DrawStretchRaw

Cinematic streaming
=============
*/
#ifdef ROQ_SUPPORT

void R_DrawStretchRaw (int x, int y, int w, int h, const byte *raw, int rawWidth, int rawHeight) //qboolean noDraw)
{
	int		i, width = 1, height = 1;
	vec2_t	texCoord[4], verts[4];

	// Make sure everything is flushed if needed
	//if (!noDraw)
	//	RB_RenderMesh();

	// Check the dimensions
	if (!glConfig.arbTextureNonPowerOfTwo) // skip if nonstandard textures sizes are supported
	{
		while (width < rawWidth)
			width <<= 1;
		while (height < rawHeight)
			height <<= 1;

		if (rawWidth != width || rawHeight != height)
			VID_Error(ERR_DROP, "R_DrawStretchRaw: size is not a power of two (%i x %i)", rawWidth, rawHeight);

		if (rawWidth > glConfig.max_texsize || rawHeight > glConfig.max_texsize)
			VID_Error(ERR_DROP, "R_DrawStretchRaw: size exceeds hardware limits (%i > %i or %i > %i)", rawWidth, glConfig.max_texsize, rawHeight, glConfig.max_texsize);
	}

	// Update the texture as appropriate
	GL_Bind(glMedia.rawtexture->texnum);
	
	if (rawWidth == glMedia.rawtexture->upload_width && rawHeight == glMedia.rawtexture->upload_height)
		qglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rawWidth, rawHeight, GL_RGBA, GL_UNSIGNED_BYTE, raw);
	else {
		glMedia.rawtexture->upload_width = rawWidth;
		glMedia.rawtexture->upload_height = rawHeight;
		qglTexImage2D(GL_TEXTURE_2D, 0, gl_tex_solid_format, rawWidth, rawHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw);
	}

	//if (noDraw)
	//	return;

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Draw it
	Vector2Set(texCoord[0], 0, 0);
	Vector2Set(texCoord[1], 1, 0);
	Vector2Set(texCoord[2], 1, 1);
	Vector2Set(texCoord[3], 0, 1);

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
		VA_SetElem4(colorArray[rb_vertex], 1, 1, 1, 1);
		rb_vertex++;
	}
	RB_RenderMeshGeneric (false);
}

#else // old 8-bit, 256x256 version

extern unsigned	r_rawpalette[256];

void R_DrawStretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data)
{
	unsigned		image32[256*256];
	unsigned char	image8[256*256];
	int				i, j, trows;
	int				frac, fracstep, row;
	float			hscale, t;
	vec2_t			texCoord[4], verts[4];
	byte			*source;

	// Nicolas' fix for stray pixels at bottom and top
	memset(image32, 0, sizeof(image32));

	GL_Bind (0);

	if (rows<=256)
	{
		hscale = 1;
		trows = rows;
	}
	else
	{
		hscale = rows/256.0;
		trows = 256;
	}
	t = rows*hscale / 256;

	if ( !qglColorTableEXT )
	{
		unsigned *dest;

		for (i=0 ; i<trows ; i++)
		{
			row = (int)(i*hscale);
			if (row > rows)
				break;
			source = data + cols*row;
			dest = &image32[i*256];
			fracstep = cols*0x10000/256;
			frac = fracstep >> 1;
			for (j=0 ; j<256 ; j++)
			{
				dest[j] = r_rawpalette[source[frac>>16]];
				frac += fracstep;
			}
		}

		qglTexImage2D (GL_TEXTURE_2D, 0, gl_tex_solid_format, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, image32);
	}
	else
	{
		unsigned char *dest;

		for (i=0 ; i<trows ; i++)
		{
			row = (int)(i*hscale);
			if (row > rows)
				break;
			source = data + cols*row;
			dest = &image8[i*256];
			fracstep = cols*0x10000/256;
			frac = fracstep >> 1;
			for (j=0 ; j<256 ; j++)
			{
				dest[j] = source[frac>>16];
				frac += fracstep;
			}
		}

		qglTexImage2D( GL_TEXTURE_2D, 
			           0, 
					   GL_COLOR_INDEX8_EXT, 
					   256, 256, 
					   0, 
					   GL_COLOR_INDEX, 
					   GL_UNSIGNED_BYTE, 
					   image8 );
	}
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if ( (glConfig.renderer == GL_RENDERER_MCD) || (glConfig.renderer & GL_RENDERER_RENDITION) ) 
		GL_Disable (GL_ALPHA_TEST);

	// Draw it
	Vector2Set(texCoord[0], 0, 0);
	Vector2Set(texCoord[1], 1, 0);
	Vector2Set(texCoord[2], 1, t);
	Vector2Set(texCoord[3], 0, t);

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
		VA_SetElem4(colorArray[rb_vertex], 1, 1, 1, 1);
		rb_vertex++;
	}
	RB_RenderMeshGeneric (false);

	if ( (glConfig.renderer == GL_RENDERER_MCD) || (glConfig.renderer & GL_RENDERER_RENDITION) ) 
		GL_Enable (GL_ALPHA_TEST);
}
#endif // ROQ_SUPPORT
