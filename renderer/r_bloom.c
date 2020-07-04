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

// r_bloom.c: 2D lighting post process effect

#include "r_local.h"


/* 
============================================================================== 
 
						LIGHT BLOOMS
 
============================================================================== 
*/ 

static float Diamond8x[8][8] = { 
		0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 0.2f, 0.3f, 0.3f, 0.2f, 0.0f, 0.0f, 
		0.0f, 0.2f, 0.4f, 0.6f, 0.6f, 0.4f, 0.2f, 0.0f, 
		0.1f, 0.3f, 0.6f, 0.9f, 0.9f, 0.6f, 0.3f, 0.1f, 
		0.1f, 0.3f, 0.6f, 0.9f, 0.9f, 0.6f, 0.3f, 0.1f, 
		0.0f, 0.2f, 0.4f, 0.6f, 0.6f, 0.4f, 0.2f, 0.0f, 
		0.0f, 0.0f, 0.2f, 0.3f, 0.3f, 0.2f, 0.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f };

static float Diamond6x[6][6] = { 
		0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 
		0.0f, 0.3f, 0.5f, 0.5f, 0.3f, 0.0f,  
		0.1f, 0.5f, 0.9f, 0.9f, 0.5f, 0.1f, 
		0.1f, 0.5f, 0.9f, 0.9f, 0.5f, 0.1f, 
		0.0f, 0.3f, 0.5f, 0.5f, 0.3f, 0.0f, 
		0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f };

static float Diamond4x[4][4] = {  
		0.3f, 0.4f, 0.4f, 0.3f,  
		0.4f, 0.9f, 0.9f, 0.4f, 
		0.4f, 0.9f, 0.9f, 0.4f, 
		0.3f, 0.4f, 0.4f, 0.3f };


static int		BLOOM_SIZE;

cvar_t		*r_bloom_alpha;
cvar_t		*r_bloom_diamond_size;
cvar_t		*r_bloom_intensity;
cvar_t		*r_bloom_threshold;
cvar_t		*r_bloom_darken;
cvar_t		*r_bloom_sample_size;
cvar_t		*r_bloom_fast_sample;

image_t		*r_bloomscreentexture;
image_t		*r_bloomeffecttexture;
image_t		*r_bloombackuptexture;
image_t		*r_bloomdownsamplingtexture;

static int		r_screendownsamplingtexture_size;
static int		screen_texture_width, screen_texture_height;
//static int		r_screenbackuptexture_size;
static int      r_screenbackuptexture_width, r_screenbackuptexture_height;

// current refdef size:
static int	curView_x;
static int	curView_y;
static int	curView_width;
static int	curView_height;

// texture coordinates of screen data inside screentexture
static float screenText_tcw;
static float screenText_tch;

static int	sample_width;
static int	sample_height;

// texture coordinates of adjusted textures
static float sampleText_tcw;
static float sampleText_tch;

// this macro is in sample size workspace coordinates
#define R_Bloom_DrawStart			\
	rb_vertex = rb_index = 0;

#define R_Bloom_DrawFinish			\
	RB_DrawArrays ();				\
	rb_vertex = rb_index = 0;

#define R_Bloom_SamplePass( xpos, ypos, r, g, b, a )								\
	indexArray[rb_index++] = rb_vertex+0;											\
	indexArray[rb_index++] = rb_vertex+1;											\
	indexArray[rb_index++] = rb_vertex+2;											\
	indexArray[rb_index++] = rb_vertex+0;											\
	indexArray[rb_index++] = rb_vertex+2;											\
	indexArray[rb_index++] = rb_vertex+3;											\
	VA_SetElem2(texCoordArray[0][rb_vertex], 0, sampleText_tch);					\
	VA_SetElem3(vertexArray[rb_vertex], xpos, ypos, 0);								\
	VA_SetElem4(colorArray[rb_vertex], r, g, b, a);									\
	rb_vertex++;																	\
	VA_SetElem2(texCoordArray[0][rb_vertex], 0, 0);									\
	VA_SetElem3(vertexArray[rb_vertex], xpos, ypos+sample_height, 0);				\
	VA_SetElem4(colorArray[rb_vertex], r, g, b, a);									\
	rb_vertex++;																	\
	VA_SetElem2(texCoordArray[0][rb_vertex], sampleText_tcw, 0);					\
	VA_SetElem3(vertexArray[rb_vertex], xpos+sample_width, ypos+sample_height, 0);	\
	VA_SetElem4(colorArray[rb_vertex], r, g, b, a);									\
	rb_vertex++;																	\
	VA_SetElem2(texCoordArray[0][rb_vertex], sampleText_tcw, sampleText_tch);		\
	VA_SetElem3(vertexArray[rb_vertex], xpos+sample_width, ypos, 0);				\
	VA_SetElem4(colorArray[rb_vertex], r, g, b, a);									\
	rb_vertex++;
	
#define R_Bloom_Quad( x, y, width, height, textwidth, textheight, r, g, b, a )	\
	rb_vertex = rb_index = 0;													\
	indexArray[rb_index++] = rb_vertex+0;										\
	indexArray[rb_index++] = rb_vertex+1;										\
	indexArray[rb_index++] = rb_vertex+2;										\
	indexArray[rb_index++] = rb_vertex+0;										\
	indexArray[rb_index++] = rb_vertex+2;										\
	indexArray[rb_index++] = rb_vertex+3;										\
	VA_SetElem2(texCoordArray[0][rb_vertex], 0, textheight);					\
	VA_SetElem3(vertexArray[rb_vertex],	x, y, 0);								\
	VA_SetElem4(colorArray[rb_vertex], r, g, b, a);								\
	rb_vertex++;																\
	VA_SetElem2(texCoordArray[0][rb_vertex], 0,	0);								\
	VA_SetElem3(vertexArray[rb_vertex], x, y+height, 0);						\
	VA_SetElem4(colorArray[rb_vertex], r, g, b, a);								\
	rb_vertex++;																\
	VA_SetElem2(texCoordArray[0][rb_vertex], textwidth,	0);						\
	VA_SetElem3(vertexArray[rb_vertex], x+width, y+height, 0);					\
	VA_SetElem4(colorArray[rb_vertex], r, g, b, a);								\
	rb_vertex++;																\
	VA_SetElem2(texCoordArray[0][rb_vertex], textwidth,	textheight);			\
	VA_SetElem3(vertexArray[rb_vertex], x+width, y, 0);							\
	VA_SetElem4(colorArray[rb_vertex], r, g, b, a);								\
	rb_vertex++;																\
	RB_DrawArrays ();															\
	rb_vertex = rb_index = 0;


/*
=================
R_Bloom_InitBackUpTexture
=================
*/
void R_Bloom_InitBackUpTexture (int width, int height)
{
	byte	*data;
	
	data = malloc( width * height * 4 );
	memset( data, 0, width * height * 4 );

//	r_screenbackuptexture_size = width;
	r_screenbackuptexture_width = width;
	r_screenbackuptexture_height = height; 

	r_bloombackuptexture = R_LoadPic ("***r_bloombackuptexture***", (byte *)data, width, height, it_pic, 3);
	
	free ( data );
}

/*
=================
R_Bloom_InitEffectTexture
=================
*/
void R_Bloom_InitEffectTexture (void)
{
	byte	*data;
	float	bloomsizecheck;
	
	if ( r_bloom_sample_size->integer < 32 )
		Cvar_SetValue ("r_bloom_sample_size", 32);

	// make sure bloom size is a power of 2
	BLOOM_SIZE = r_bloom_sample_size->integer;
	bloomsizecheck = (float)BLOOM_SIZE;
	while(bloomsizecheck > 1.0f) bloomsizecheck /= 2.0f;
	if ( bloomsizecheck != 1.0f )
	{
		BLOOM_SIZE = 32;
		while( BLOOM_SIZE < r_bloom_sample_size->integer )
			BLOOM_SIZE *= 2;
	}

	// make sure bloom size doesn't have stupid values
	if ( BLOOM_SIZE > screen_texture_width ||
		BLOOM_SIZE > screen_texture_height )
		BLOOM_SIZE = min( screen_texture_width, screen_texture_height );

	if ( BLOOM_SIZE != r_bloom_sample_size->integer )
		Cvar_SetValue ("r_bloom_sample_size", BLOOM_SIZE);

	// Knightmare- catch too large bloom size
	while ( ((BLOOM_SIZE * 2) > vid.height) || ((BLOOM_SIZE * 2) > vid.width) )
		BLOOM_SIZE /= 2;

	data = malloc( BLOOM_SIZE * BLOOM_SIZE * 4 );
	memset( data, 0, BLOOM_SIZE * BLOOM_SIZE * 4 );

	r_bloomeffecttexture = R_LoadPic ("***r_bloomeffecttexture***", (byte *)data, BLOOM_SIZE, BLOOM_SIZE, it_pic, 3);
	
	free ( data );
}

/*
=================
R_Bloom_InitTextures
=================
*/
void R_Bloom_InitTextures (void)
{
	byte	*data;
	int		size;

	// find closer power of 2 to screen size 
	for (screen_texture_width = 1; screen_texture_width < vid.width; screen_texture_width *= 2);
	for (screen_texture_height = 1; screen_texture_height < vid.height; screen_texture_height *= 2);

	// disable blooms if we can't handle a texture of that size
/*	if ( screen_texture_width > glConfig.maxTextureSize ||
		screen_texture_height > glConfig.maxTextureSize ) {
		screen_texture_width = screen_texture_height = 0;
		Cvar_SetValue ("r_bloom", 0);
		Com_Printf( "WARNING: 'R_InitBloomScreenTexture' too high resolution for Light Bloom. Effect disabled\n" );
		return;
	}*/

	//init the screen texture
	size = screen_texture_width * screen_texture_height * 4;
	data = malloc( size );
	memset( data, 255, size );
	r_bloomscreentexture = R_LoadPic ("***r_bloomscreentexture***", (byte *)data, screen_texture_width, screen_texture_height, it_pic, 3);
	free ( data );

	// validate bloom size and init the bloom effect texture
	R_Bloom_InitEffectTexture ();

	// if screensize is more than 2x the bloom effect texture, set up for stepped downsampling
	r_bloomdownsamplingtexture = NULL;
	r_screendownsamplingtexture_size = 0;
	if ( vid.width > (BLOOM_SIZE * 2) && !r_bloom_fast_sample->integer )
	{
		r_screendownsamplingtexture_size = (int)(BLOOM_SIZE * 2);
		data = malloc( r_screendownsamplingtexture_size * r_screendownsamplingtexture_size * 4 );
		memset( data, 0, r_screendownsamplingtexture_size * r_screendownsamplingtexture_size * 4 );
		r_bloomdownsamplingtexture = R_LoadPic ("***r_bloomdownsamplingtexture***", (byte *)data, r_screendownsamplingtexture_size, r_screendownsamplingtexture_size, it_pic, 3);
		free ( data );
	}

	// Init the screen backup texture
	if ( r_screendownsamplingtexture_size )
		R_Bloom_InitBackUpTexture( r_screendownsamplingtexture_size, r_screendownsamplingtexture_size );
	else
		R_Bloom_InitBackUpTexture( BLOOM_SIZE, BLOOM_SIZE );
	
}

/*
=================
R_InitBloomTextures
=================
*/
void R_InitBloomTextures (void)
{

//	r_bloom = Cvar_Get( "r_bloom", "0", CVAR_ARCHIVE );
	r_bloom_alpha = Cvar_Get( "r_bloom_alpha", "0.25", CVAR_ARCHIVE );			// was 0.33
	Cvar_SetDescription ("r_bloom_alpha", "Sets opacity of bloom blends.");
	r_bloom_diamond_size = Cvar_Get( "r_bloom_diamond_size", "8", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_bloom_diamond_size", "Sets size of bloom diamonds.  Accepted values are 4, 6, or 8.");
	r_bloom_intensity = Cvar_Get( "r_bloom_intensity", "2.5", CVAR_ARCHIVE );	// was 0.6
	Cvar_SetDescription ("r_bloom_intensity", "Sets intensity of bloom effect.");
	r_bloom_threshold = Cvar_Get( "r_bloom_threshold", "0.68", CVAR_ARCHIVE );	// was 0.08
	Cvar_SetDescription ("r_bloom_threshold", "Sets brightness threshold for bloom effect.");
	r_bloom_darken = Cvar_Get( "r_bloom_darken", "5", CVAR_ARCHIVE );			// was 4
	Cvar_SetDescription ("r_bloom_darken", "Sets number of darkening pass for bloom effect.");
	r_bloom_sample_size = Cvar_Get( "r_bloom_sample_size", "256", CVAR_ARCHIVE );	// was 128
	Cvar_SetDescription ("r_bloom_sample_size", "Sets sample size in pixels for bloom effect.");
	r_bloom_fast_sample = Cvar_Get( "r_bloom_fast_sample", "0", CVAR_ARCHIVE );
	Cvar_SetDescription ("r_bloom_fast_sample", "Disables downsampling of screen buffer for bloom sampling when set to 1.");

	BLOOM_SIZE = 0;
	if (!r_bloom->integer)
		return;

	R_Bloom_InitTextures ();
}

 
/*
=================
R_Bloom_DrawEffect
=================
*/
void R_Bloom_DrawEffect (void)
{
	GL_Bind(r_bloomeffecttexture->texnum);  
	GL_Enable(GL_BLEND);
	GL_BlendFunc(GL_ONE, GL_ONE);
	GL_TexEnv(GL_MODULATE);

	rb_vertex = rb_index = 0;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+1;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+0;
	indexArray[rb_index++] = rb_vertex+2;
	indexArray[rb_index++] = rb_vertex+3;
	VA_SetElem2(texCoordArray[0][rb_vertex], 0, sampleText_tch);
	VA_SetElem3(vertexArray[rb_vertex],	curView_x, curView_y, 0);
	VA_SetElem4(colorArray[rb_vertex], r_bloom_alpha->value, r_bloom_alpha->value, r_bloom_alpha->value, 1.0f);
	rb_vertex++;
	VA_SetElem2(texCoordArray[0][rb_vertex], 0,	0);
	VA_SetElem3(vertexArray[rb_vertex], curView_x, curView_y + curView_height, 0);
	VA_SetElem4(colorArray[rb_vertex], r_bloom_alpha->value, r_bloom_alpha->value, r_bloom_alpha->value, 1.0f);
	rb_vertex++;
	VA_SetElem2(texCoordArray[0][rb_vertex], sampleText_tcw, 0);
	VA_SetElem3(vertexArray[rb_vertex], curView_x + curView_width, curView_y + curView_height, 0);
	VA_SetElem4(colorArray[rb_vertex], r_bloom_alpha->value, r_bloom_alpha->value, r_bloom_alpha->value, 1.0f);
	rb_vertex++;
	VA_SetElem2(texCoordArray[0][rb_vertex], sampleText_tcw, sampleText_tch);
	VA_SetElem3(vertexArray[rb_vertex], curView_x + curView_width, curView_y, 0);
	VA_SetElem4(colorArray[rb_vertex], r_bloom_alpha->value, r_bloom_alpha->value, r_bloom_alpha->value, 1.0f);
	rb_vertex++;
	RB_DrawArrays ();
	rb_vertex = rb_index = 0;

	GL_Disable (GL_BLEND);
}


#if 0
/*
=================
R_Bloom_GeneratexCross - alternative bluring method
=================
*/
void R_Bloom_GeneratexCross (void)
{
	int				i;
	static int		BLOOM_BLUR_RADIUS = 8;
//	static float	BLOOM_BLUR_INTENSITY = 2.5f;
	float			BLOOM_BLUR_INTENSITY;
	static float	intensity;
	static float	range;

	// set up sample size workspace
	qglViewport( 0, 0, sample_width, sample_height );
	qglMatrixMode( GL_PROJECTION );
    qglLoadIdentity ();
	qglOrtho(0, sample_width, sample_height, 0, -10, 100);
	qglMatrixMode( GL_MODELVIEW );
    qglLoadIdentity ();

	// copy small scene into r_bloomeffecttexture
	GL_Bind(0, r_bloomeffecttexture);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);

	// start modifying the small scene corner
	GL_Enable(GL_BLEND);

	// darkening passes
	if (r_bloom_darken->integer)
	{
		GL_BlendFunc(GL_DST_COLOR, GL_ZERO);
		GL_TexEnv(GL_MODULATE);

		R_Bloom_DrawStart
		for (i=0; i<r_bloom_darken->integer; i++) {
			R_Bloom_SamplePass( 0, 0, 1.0f, 1.0f, 1.0f, 1.0f );
		}
		R_Bloom_DrawFinish
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);
	}

	// bluring passes
	if ( BLOOM_BLUR_RADIUS )
	{
		
		GL_BlendFunc(GL_ONE, GL_ONE);

		range = (float)BLOOM_BLUR_RADIUS;

		BLOOM_BLUR_INTENSITY = r_bloom_intensity->value;
		// diagonal-cross draw 4 passes to add initial smooth
		R_Bloom_DrawStart
		R_Bloom_SamplePass( 1, 1, 0.5f, 0.5f, 0.5f, 1.0 );
		R_Bloom_SamplePass( -1, 1, 0.5f, 0.5f, 0.5f, 1.0 );
		R_Bloom_SamplePass( -1, -1, 0.5f, 0.5f, 0.5f, 1.0 );
		R_Bloom_SamplePass( 1, -1, 0.5f, 0.5f, 0.5f, 1.0 );
		R_Bloom_DrawFinish
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);

		R_Bloom_DrawStart
		for (i=-(BLOOM_BLUR_RADIUS+1);i<BLOOM_BLUR_RADIUS;i++) {
			intensity = BLOOM_BLUR_INTENSITY/(range*2+1)*(1 - fabs(i*i)/(float)(range*range));
			if ( intensity < 0.05f ) continue;
			R_Bloom_SamplePass( i, 0, intensity, intensity, intensity, 1.0f );
		//	R_Bloom_SamplePass( -i, 0 );
		}
		R_Bloom_DrawFinish

		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);

		R_Bloom_DrawStart
	//	for (i=0;i<BLOOM_BLUR_RADIUS;i++) {
		for (i=-(BLOOM_BLUR_RADIUS+1);i<BLOOM_BLUR_RADIUS;i++) {
			intensity = BLOOM_BLUR_INTENSITY/(range*2+1)*(1 - fabs(i*i)/(float)(range*range));
			if ( intensity < 0.05f ) continue;
			R_Bloom_SamplePass( 0, i, intensity, intensity, intensity, 1.0f );
		//	R_Bloom_SamplePass( 0, -i );
		}
		R_Bloom_DrawFinish

		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);
	}
	
	// restore full screen workspace
	qglViewport( 0, 0, glState.width, glState.height );
	qglMatrixMode( GL_PROJECTION );
    qglLoadIdentity ();
	qglOrtho(0, glState.width, glState.height, 0, -10, 100);
	qglMatrixMode( GL_MODELVIEW );
    qglLoadIdentity ();
}
#endif


/*
=================
R_Bloom_GeneratexDiamonds
=================
*/
void R_Bloom_GeneratexDiamonds (void)
{
	int				i, j;
	static float	intensity;

	// set up sample size workspace
	qglViewport( 0, 0, sample_width, sample_height );
	qglMatrixMode( GL_PROJECTION );
    qglLoadIdentity ();
	qglOrtho(0, sample_width, sample_height, 0, -10, 100);
	qglMatrixMode( GL_MODELVIEW );
    qglLoadIdentity ();

	// copy small scene into r_bloomeffecttexture
	GL_Bind(r_bloomeffecttexture->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);

	// start modifying the small scene corner
	GL_Enable(GL_BLEND);

	// darkening passes
	if ( r_bloom_darken->integer )
	{
		GL_BlendFunc(GL_DST_COLOR, GL_ZERO);
		GL_TexEnv(GL_MODULATE);
		
		R_Bloom_DrawStart
		for (i=0; i<r_bloom_darken->integer; i++) {
			R_Bloom_SamplePass( 0, 0, 1.0f, 1.0f, 1.0f, 1.0f );
		}
		R_Bloom_DrawFinish
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);
	}

	// bluring passes
	// GL_BlendFunc(GL_ONE, GL_ONE);
	GL_BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	
	if ( r_bloom_diamond_size->integer > 7 || r_bloom_diamond_size->integer <= 3)
	{
		if ( r_bloom_diamond_size->integer != 8 ) Cvar_SetValue( "r_bloom_diamond_size", 8 );

		R_Bloom_DrawStart
		for (i=0; i<r_bloom_diamond_size->integer; i++) {
			for (j=0; j<r_bloom_diamond_size->integer; j++) {
				intensity = r_bloom_intensity->value * 0.3 * Diamond8x[i][j];
				if ( intensity < r_bloom_threshold->value ) continue;
				R_Bloom_SamplePass( i-4, j-4, intensity, intensity, intensity, 1.0 );
			}
		}
		R_Bloom_DrawFinish			
	}
	else if ( r_bloom_diamond_size->integer > 5 )
	{
		if ( r_bloom_diamond_size->integer != 6 ) Cvar_SetValue( "r_bloom_diamond_size", 6 );

		R_Bloom_DrawStart
		for (i=0; i<r_bloom_diamond_size->integer; i++) {
			for (j=0; j<r_bloom_diamond_size->integer; j++) {
				intensity = r_bloom_intensity->value * 0.5 * Diamond6x[i][j];
				if ( intensity < r_bloom_threshold->value ) continue;
				R_Bloom_SamplePass( i-3, j-3, intensity, intensity, intensity, 1.0 );
			}
		}
		R_Bloom_DrawFinish			
	}
	else if ( r_bloom_diamond_size->integer > 3 )
	{
		if ( r_bloom_diamond_size->integer != 4 ) Cvar_SetValue( "r_bloom_diamond_size", 4 );

		R_Bloom_DrawStart
		for (i=0; i<r_bloom_diamond_size->integer; i++) {
			for (j=0; j<r_bloom_diamond_size->integer; j++) {
				intensity = r_bloom_intensity->value * 0.8f * Diamond4x[i][j];
				if ( intensity < r_bloom_threshold->value ) continue;
				R_Bloom_SamplePass( i-2, j-2, intensity, intensity, intensity, 1.0 );
			}
		}
		R_Bloom_DrawFinish			
	}
	
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, sample_width, sample_height);

	// restore full screen workspace
	qglViewport( 0, 0, vid.width, vid.height );
	qglMatrixMode( GL_PROJECTION );
    qglLoadIdentity ();
	qglOrtho(0, vid.width, vid.height, 0, -10, 100);
	qglMatrixMode( GL_MODELVIEW );
    qglLoadIdentity ();
}											

/*
=================
R_Bloom_DownsampleView
=================
*/
void R_Bloom_DownsampleView (void)
{
	GL_Disable( GL_BLEND );

	// stepped downsample
	if ( r_screendownsamplingtexture_size )
	{
		int		midsample_width = r_screendownsamplingtexture_size * sampleText_tcw;
		int		midsample_height = r_screendownsamplingtexture_size * sampleText_tch;
		
		// copy the screen and draw resized
		GL_Bind(r_bloomscreentexture->texnum);
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, curView_x, vid.height - (curView_y + curView_height), curView_width, curView_height);
		R_Bloom_Quad( 0,  vid.height-midsample_height, midsample_width, midsample_height, screenText_tcw, screenText_tch,  1.0f, 1.0f, 1.0f, 1.0f );
		
		// now copy into Downsampling (mid-sized) texture
		GL_Bind(r_bloomdownsamplingtexture->texnum);
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, midsample_width, midsample_height);

		// now draw again in bloom size
		R_Bloom_Quad( 0,  vid.height-sample_height, sample_width, sample_height, sampleText_tcw, sampleText_tch, 0.5f, 0.5f, 0.5f, 1.0f );
		
		// now blend the big screen texture into the bloom generation space (hoping it adds some blur)
		GL_Enable( GL_BLEND );
		GL_BlendFunc(GL_ONE, GL_ONE);
		GL_Bind(r_bloomscreentexture->texnum);
		R_Bloom_Quad( 0,  vid.height-sample_height, sample_width, sample_height, screenText_tcw, screenText_tch, 0.5f, 0.5f, 0.5f, 1.0f );
		GL_Disable( GL_BLEND );

	}
	else // downsample simple
	{	
		GL_Bind(r_bloomscreentexture->texnum);
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, curView_x, vid.height - (curView_y + curView_height), curView_width, curView_height);
		R_Bloom_Quad( 0, vid.height-sample_height, sample_width, sample_height, screenText_tcw, screenText_tch, 1.0f, 1.0f, 1.0f, 1.0f );
	}
}

/*
=================
R_BloomBlend
=================
*/
void R_SetupGL (void);
void R_BloomBlend ( refdef_t *fd )
{
	if ( (fd->rdflags & RDF_NOWORLDMODEL) || !r_bloom->integer || r_showtris->integer )
		return;

	if ( !BLOOM_SIZE )
		R_Bloom_InitTextures();

	if ( screen_texture_width < BLOOM_SIZE ||
		screen_texture_height < BLOOM_SIZE )
		return;
	
	// set up full screen workspace
	qglViewport ( 0, 0, vid.width, vid.height );
	GL_TexEnv (GL_REPLACE);	// Knightmare added
	GL_Disable (GL_DEPTH_TEST);
	qglMatrixMode (GL_PROJECTION);
    qglLoadIdentity ();
	qglOrtho(0, vid.width, vid.height, 0, -10, 100);
	qglMatrixMode (GL_MODELVIEW);
    qglLoadIdentity ();
	GL_Disable (GL_CULL_FACE);

	GL_Disable (GL_BLEND);
	GL_EnableTexture (0);

	// set up current sizes
	curView_x = fd->x;
	curView_y = fd->y;
	curView_width = fd->width;
	curView_height = fd->height;
	screenText_tcw = ((float)fd->width / (float)screen_texture_width);
	screenText_tch = ((float)fd->height / (float)screen_texture_height);
	if ( fd->height > fd->width ) {
		sampleText_tcw = ((float)fd->width / (float)fd->height);
		sampleText_tch = 1.0f;
	} else {
		sampleText_tcw = 1.0f;
		sampleText_tch = ((float)fd->height / (float)fd->width);
	}
	sample_width = BLOOM_SIZE * sampleText_tcw;
	sample_height = BLOOM_SIZE * sampleText_tch;
	
	// copy the screen space we'll use to work into the backup texture
	GL_Bind(r_bloombackuptexture->texnum);
//	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, r_screenbackuptexture_size * sampleText_tcw, r_screenbackuptexture_size * sampleText_tch);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, r_screenbackuptexture_width, r_screenbackuptexture_height);

	// create the bloom image
	R_Bloom_DownsampleView();
	R_Bloom_GeneratexDiamonds();
//	R_Bloom_GeneratexCross();

	// restore the screen-backup to the screen
	GL_Disable(GL_BLEND);
	GL_Bind(r_bloombackuptexture->texnum);
/*	R_Bloom_Quad( 0, 
		vid.height - (r_screenbackuptexture_size * sampleText_tch),
		r_screenbackuptexture_size * sampleText_tcw,
		r_screenbackuptexture_size * sampleText_tch,
		sampleText_tcw,
		sampleText_tch, 1, 1, 1, 1 ); */
	R_Bloom_Quad( 0, vid.height - r_screenbackuptexture_height,
		r_screenbackuptexture_width,
		r_screenbackuptexture_height, 1.0, 1.0, 1, 1, 1, 1 );

	R_Bloom_DrawEffect();

	// Knightmare added
	R_SetupGL ();
	GL_BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_EnableTexture (0);
//	qglColor4f(1,1,1,1);
}
