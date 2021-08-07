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

// r_draw.c - 2D image drawing

#include "r_local.h"

image_t		*r_con_draw_chars;
image_t		*r_scr_draw_chars;
image_t		*r_ui_draw_chars;

extern	qboolean	scrap_dirty;
void Scrap_Upload (void);

#define DEFAULT_FONT_SIZE 8.0f

void R_RefreshFont (fontslot_t font)
{
	cvar_t	*fontvar;
	image_t	*fontimage;

	switch (font)
	{
	case FONT_SCREEN:
		fontvar = scr_font;
		break;
	case FONT_UI:
		fontvar = ui_font;
		break;
	case FONT_CONSOLE:
	default:
		fontvar = con_font;
		break;
	}
	fontvar->modified = false;

	fontimage = R_FindImage (va("fonts/%s.pcx", fontvar->string), it_font);	// was it_pic
	if (!fontimage) // fall back on default font
		fontimage = R_FindImage ("fonts/default.pcx", it_font);	// was it_pic
	if (!fontimage) // fall back on old Q2 conchars
		fontimage = R_FindImage ("pics/conchars.pcx", it_font);	// was it_pic
	if (!fontimage) // prevent crash caused by missing font
		VID_Error (ERR_FATAL, "RefreshFont: couldn't load pics/conchars");

	GL_Bind (fontimage->texnum);

	switch (font)
	{
	case FONT_SCREEN:
		r_scr_draw_chars = fontimage;
		break;
	case FONT_UI:
		r_ui_draw_chars = fontimage;
		break;
	case FONT_CONSOLE:
	default:
		r_con_draw_chars = fontimage;
		break;
	}
}


/*
===============
R_RefreshAllFonts
===============
*/
void R_RefreshAllFonts (void)
{
	R_RefreshFont (FONT_CONSOLE);
	R_RefreshFont (FONT_SCREEN);
	R_RefreshFont (FONT_UI);
/*
	con_font->modified = false;

	r_con_draw_chars = R_FindImage (va("fonts/%s.pcx", con_font->string), it_pic);
	if (!r_con_draw_chars) // fall back on default font
		r_con_draw_chars = R_FindImage ("fonts/default.pcx", it_pic);
	if (!r_con_draw_chars) // fall back on old Q2 conchars
		r_con_draw_chars = R_FindImage ("pics/conchars.pcx", it_pic);
	if (!r_con_draw_chars) // prevent crash caused by missing font
		VID_Error (ERR_FATAL, "R_RefreshFont: couldn't load pics/conchars");

	GL_Bind (r_con_draw_chars->texnum );
*/
}


/*
===============
R_ImageForFont
===============
*/
image_t *R_ImageForFont (fontslot_t font)
{
	switch (font)
	{
	case FONT_SCREEN:
		return r_scr_draw_chars;
	case FONT_UI:
		return r_ui_draw_chars;
	case FONT_CONSOLE:
	default:
		return r_con_draw_chars;
	}
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
	R_RefreshAllFonts ();

	R_InitChars (); // init char indexes
}


/*
================
R_CharMapScale
================
*/
float R_CharMapScale (void)
{
	return (r_con_draw_chars->width/128.0); //current width / original width
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
void R_FlushChars (fontslot_t font)
{
	if (rb_vertex == 0 || rb_index == 0) // nothing to flush
		return;

	GL_Disable (GL_ALPHA_TEST);
	GL_TexEnv (GL_MODULATE);
	GL_Enable (GL_BLEND);
	GL_DepthMask (false);
	GL_Bind((R_ImageForFont(font))->texnum);

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
void R_DrawChar (float x, float y, int num, fontslot_t font, float scale, 
	 int red, int green, int blue, int alpha, qboolean italic, qboolean last)
{
	int			row, col, i;
	float		frow, fcol, size, cscale, italicAdd;
	vec2_t		texCoord[4];	// verts[4]
	int			verts[4][2];	// forcing this to int fixes wrap artifacts at odd resolutions
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
			R_FlushChars (font);
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
		R_FlushChars (font);
}


/*
================
R_DrawString
Draws a string of variable sized graphics characters.
================
*/
void R_DrawString (float x, float y, const char *string, fontslot_t font, float scale, 
				int red, int green, int blue, int alpha, qboolean italic, qboolean shadow)

{
	int		i, len;
	float	size = scale * DEFAULT_FONT_SIZE;

	len = (int)strlen(string);
	if (len < 1)	// nothing to draw
		return;

	if (shadow) {
		for (i=0; i<len; i++)
			R_DrawChar ( (x + i*size+size*0.25), y+(size*0.125), 
				string[i], font, scale, 0, 0, 0, alpha, italic, false );
	}
	for (i=0; i<len; i++)
		R_DrawChar ( (x + i*size), y, 
			string[i], font, scale, red, green, blue, alpha, italic, (i==(len-1)) );
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
	*h = (int)((float)gl->height * gl->replace_scale_h);
}


/*
=============
R_DrawPic_Standard
Actual pic drawing code.
Not to be called from other sections.
=============
*/
void R_DrawPic_Standard (int x, int y, int w, int h, vec2_t offset, vec4_t stCoords, vec4_t color, int texnum, renderparms_t *parms, qboolean texClamp)
{
	int			i;
	vec2_t		texCoord[4], verts[4];

	if (!parms)	return;

	if (scrap_dirty)
		Scrap_Upload ();

	if (parms->blend)
	{
		GL_Disable (GL_ALPHA_TEST);
		GL_TexEnv (GL_MODULATE);
		GL_Enable (GL_BLEND);
		GL_DepthMask (false);
	}

	GL_Bind (texnum);

	if (texClamp) {
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}

	Vector2Set(texCoord[0], stCoords[0], stCoords[1]);
	Vector2Set(texCoord[1], stCoords[2], stCoords[1]);
	Vector2Set(texCoord[2], stCoords[2], stCoords[3]);
	Vector2Set(texCoord[3], stCoords[0], stCoords[3]);

	Vector2Set(verts[0], x+offset[0], y+offset[1]);
	Vector2Set(verts[1], x+w+offset[0], y-offset[1]);
	Vector2Set(verts[2], x+w-offset[0], y+h-offset[1]);
	Vector2Set(verts[3], x-offset[0], y+h+offset[1]);

	RB_ModifyTextureCoords (&texCoord[0][0], &verts[0][0], 4, &parms->tcmod);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;
	for (i=0; i<4; i++) {
		VA_SetElem2v(texCoordArray[0][rb_vertex], texCoord[i]);
		VA_SetElem3(vertexArray[rb_vertex], verts[i][0], verts[i][1], 0);
		VA_SetElem4v(colorArray[rb_vertex], color);
		rb_vertex++;
	}
	RB_RenderMeshGeneric (false);

	if (texClamp) {
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	if (parms->blend)
	{
		GL_DepthMask (true);
		GL_TexEnv (GL_REPLACE);
		GL_Disable (GL_BLEND);
		GL_Enable (GL_ALPHA_TEST);
	}
}


/*
=============
R_DrawPic_Masked
=============
*/
void R_DrawPic_Masked (int x, int y, int w, int h, vec2_t offset, vec4_t stCoords, vec4_t color, int texnum, int maskTexnum, renderparms_t *parms, qboolean texClamp)
{
	int			i;
	vec2_t		texCoord[4], scrollTexCoord[4], verts[4];

	if (!parms)	return;

	if (scrap_dirty)
		Scrap_Upload ();

	if (parms->blend)
	{
		GL_Disable (GL_ALPHA_TEST);
		GL_Enable (GL_BLEND);
	//	GL_BlendFunc (GL_SRC_ALPHA, additive ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);	// GL_DST_ALPHA, GL_ONE
		GL_BlendFunc (parms->blendfunc_src, parms->blendfunc_dst);
		GL_DepthMask (false);
	}

	GL_SelectTexture (0);
	GL_Bind (maskTexnum);
	GL_TexEnv (GL_COMBINE_ARB);

	// Do nothing with this stage, it's just for alpha
	qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE);
	qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
	qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
	qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_INTERPOLATE);
	qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
	qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_TEXTURE);

	GL_EnableTexture (1);
	GL_Bind (texnum);
	GL_TexEnv (GL_COMBINE_ARB);

	if (texClamp) {
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}

	// This stage uses the previous one's alpha value
	if (parms->blend) {
		qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
	}
	else {
		qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
		qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
	}
	qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
	qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE0);	// GL_PREVIOUS_ARB

	Vector2Set(texCoord[0], stCoords[0], stCoords[1]);
	Vector2Set(texCoord[1], stCoords[2], stCoords[1]);
	Vector2Set(texCoord[2], stCoords[2], stCoords[3]);
	Vector2Set(texCoord[3], stCoords[0], stCoords[3]);

	for (i=0; i<4; i++)
		Vector2Copy(texCoord[i], scrollTexCoord[i]);

	Vector2Set(verts[0], x+offset[0], y+offset[1]);
	Vector2Set(verts[1], x+w+offset[0], y-offset[1]);
	Vector2Set(verts[2], x+w-offset[0], y+h-offset[1]);
	Vector2Set(verts[3], x-offset[0], y+h+offset[1]);

	RB_ModifyTextureCoords (&scrollTexCoord[0][0], &verts[0][0], 4, &parms->tcmod);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;
	for (i=0; i<4; i++) {
		VA_SetElem2v(texCoordArray[0][rb_vertex], texCoord[i]);
		VA_SetElem2v(texCoordArray[1][rb_vertex], scrollTexCoord[i]);
		VA_SetElem3(vertexArray[rb_vertex], verts[i][0], verts[i][1], 0);
		VA_SetElem4v(colorArray[rb_vertex], color);
		rb_vertex++;
	}
	RB_RenderMeshGeneric (false);

	if (texClamp) {
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	// Reset parms
	GL_DisableTexture (1);
	GL_SelectTexture (0);
	qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
	qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
	qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
	qglTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
	qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
	qglTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_PREVIOUS_ARB);
	GL_TexEnv (GL_REPLACE);

	if (parms->blend)
	{
		GL_DepthMask (true);
		GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GL_Disable (GL_BLEND);
		GL_Enable (GL_ALPHA_TEST);
	}
}


/*
=============
R_DrawPic
=============
*/
void R_DrawPic (drawStruct_t ds)	
{
	int				w, h;
	float			scale_x, scale_y;
	vec4_t			texCoords;
	image_t			*image;
	renderparms_t	drawParms;

	image = R_DrawFindPic (ds.pic);
	if (!image) {
		VID_Printf (PRINT_ALL, "Can't find pic: %s\n", ds.pic);
		return;
	}

	w = ds.w;	h = ds.h;
	if (ds.flags & DSFLAG_USESTCOORDS)	// use passed coords
		Vector4Copy (ds.stCoords, texCoords);
	else if (ds.flags & DSFLAG_TILED)	// use tiled coords
		Vector4Set (texCoords, (float)ds.x/(float)image->width, (float)ds.y/(float)image->height,
					(float)(ds.x+ds.w)/(float)image->width, (float)(ds.y+ds.h)/(float)image->height);
	else if (ds.flags & DSFLAG_SCALED)	// use scaled size
	{
		Vector4Set (texCoords, image->sl, image->tl, image->sh, image->th);

		scale_x = ds.scale[0];
		scale_y = ds.scale[1];
		scale_x *= image->replace_scale_w;	// scale down if replacing a pcx image
		scale_y *= image->replace_scale_h;	// scale down if replacing a pcx image
		w = image->width*scale_x;
		h = image->height*scale_y;
	}
	else	// use internal coords
		Vector4Set (texCoords, image->sl, image->tl, image->sh, image->th);

	Mod_SetRenderParmsDefaults (&drawParms);
	drawParms.tcmod.scroll_x = ds.scroll[0];
	drawParms.tcmod.scroll_y = ds.scroll[1];

	// masked image option
	if ( (ds.flags & DSFLAG_MASKED) && (ds.maskPic != NULL) && glConfig.mtexcombine )
	{
		image_t		*maskImage = R_DrawFindPic (ds.maskPic);

		if (maskImage != NULL)
		{
		//	VID_Printf (PRINT_ALL, "Drawing pic with mask: %s\n", pic);
			drawParms.blend = true;
			drawParms.blendfunc_src = GL_SRC_ALPHA;
			drawParms.blendfunc_dst = GL_ONE;
			R_DrawPic_Masked (ds.x, ds.y, w, h, ds.offset, ds.stCoords, ds.color, image->texnum, maskImage->texnum, &drawParms, (ds.flags & DSFLAG_CLAMP));
			return;
		}
	}

	drawParms.blend = (image->has_alpha || ds.color[3] < 1.0f);
	R_DrawPic_Standard (ds.x, ds.y, w, h, ds.offset, texCoords, ds.color, image->texnum, &drawParms, (ds.flags & DSFLAG_CLAMP));
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
void R_DrawCameraEffect (void)
{
	image_t			*image[2];
	int				x, y, w, h, i, j;
	float			texparms[2][4];
	vec2_t			texCoord[4];
	vec3_t			verts[4];
	tcmodParms_t	cameraParms;

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
		Mod_SetTCModParmsDefaults (&cameraParms);
		cameraParms.scale_x = texparms[i][0];
		cameraParms.scale_y = texparms[i][1];
		cameraParms.scroll_x = texparms[i][2];
		cameraParms.scroll_y = texparms[i][3];
		RB_ModifyTextureCoords (&texCoord[0][0], &verts[0][0], 4, &cameraParms);
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
	//	qglTexImage2D(GL_TEXTURE_2D, 0, gl_tex_solid_format, rawWidth, rawHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw);
		qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rawWidth, rawHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw);
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
