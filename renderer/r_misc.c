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

// r_misc.c - particle image loading, and screenshots

#include "r_local.h"
#include "include/jpeglib.h" // Heffo - JPEG Screenshots


/*
==================
R_CreateNullTexture
==================
*/
#define NULLTEX_SIZE 16
void R_CreateNullTexture (void)
{
	byte	nulltex[NULLTEX_SIZE][NULLTEX_SIZE][4];
	int		x;

	memset (nulltex, 32, sizeof(nulltex));
	for (x=0; x<NULLTEX_SIZE; x++)
	{
		nulltex[0][x][0]=
		nulltex[0][x][1]=
		nulltex[0][x][2]=
		nulltex[0][x][3]= 255;

		nulltex[x][0][0]=
		nulltex[x][0][1]=
		nulltex[x][0][2]=
		nulltex[x][0][3]= 255;

		nulltex[NULLTEX_SIZE-1][x][0]=
		nulltex[NULLTEX_SIZE-1][x][1]=
		nulltex[NULLTEX_SIZE-1][x][2]=
		nulltex[NULLTEX_SIZE-1][x][3]= 255;

		nulltex[x][NULLTEX_SIZE-1][0]=
		nulltex[x][NULLTEX_SIZE-1][1]=
		nulltex[x][NULLTEX_SIZE-1][2]=
		nulltex[x][NULLTEX_SIZE-1][3]= 255;
	}
	r_notexture = R_LoadPic ("***r_notexture***", (byte *)nulltex, NULLTEX_SIZE, NULLTEX_SIZE, it_wall, 32);
}


/*
==================
LoadPartImg
==================
*/
image_t *LoadPartImg (char *name, imagetype_t type)
{
	image_t *image = R_FindImage(name, type);
	if (!image) image = r_notexture;
	return image;
}


/*
==================
R_SetParticlePicture
==================
*/
void R_SetParticlePicture (int num, char *name)
{
	r_particletextures[num] = LoadPartImg (name, it_part);
}


/*
==================
R_InitParticleTextures
==================
*/
void R_InitParticleTextures (void)
{
	int		x;
#ifdef ROQ_SUPPORT
	byte	data2D[256*256*4]; // Raw texture
#endif // ROQ_SUPPORT

	R_CreateNullTexture(); // Generate null texture

#ifdef ROQ_SUPPORT
	memset(data2D, 255, 256*256*4);
	r_rawtexture = R_LoadPic ("***r_rawtexture***", data2D, 256, 256, it_pic, 32);
#endif // ROQ_SUPPORT
	
	r_envmappic = LoadPartImg ("gfx/envmap.tga", it_wall);
	r_spheremappic = LoadPartImg ("gfx/spheremap.tga", it_skin);
	r_causticpic = LoadPartImg ("gfx/water/caustic.tga", it_wall);
	r_shelltexture = LoadPartImg ("gfx/shell_generic.tga", it_skin);
	r_particlebeam = LoadPartImg ("gfx/particles/beam.tga", it_part);

	//Knightmare- Psychospaz's enhanced particles
	for (x=0 ; x<PARTICLE_TYPES ; x++)
		r_particletextures[x] = NULL;

	SetParticleImages();
}


/* 
============================================================================== 
 
						SCREEN SHOTS 
 
============================================================================== 
*/ 

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


/*
================
R_ResampleShotLerpLine
from DarkPlaces
================
*/
void R_ResampleShotLerpLine (byte *in, byte *out, int inwidth, int outwidth) 
{ 
	int j, xi, oldx = 0, f, fstep, l1, l2, endx;

	fstep = (int) (inwidth*65536.0f/outwidth); 
	endx = (inwidth-1); 
	for (j = 0,f = 0; j < outwidth; j++, f += fstep) 
	{ 
		xi = (int) f >> 16; 
		if (xi != oldx) 
		{ 
			in += (xi - oldx) * 3; 
			oldx = xi; 
		} 
		if (xi < endx) 
		{ 
			l2 = f & 0xFFFF; 
			l1 = 0x10000 - l2; 
			*out++ = (byte) ((in[0] * l1 + in[3] * l2) >> 16);
			*out++ = (byte) ((in[1] * l1 + in[4] * l2) >> 16); 
			*out++ = (byte) ((in[2] * l1 + in[5] * l2) >> 16); 
		} 
		else // last pixel of the line has no pixel to lerp to 
		{ 
			*out++ = in[0]; 
			*out++ = in[1]; 
			*out++ = in[2]; 
		} 
	} 
}


/*
================
R_ResampleShot
================
*/
void R_ResampleShot (void *indata, int inwidth, int inheight, void *outdata, int outwidth, int outheight) 
{ 
	int i, j, yi, oldy, f, fstep, l1, l2, endy = (inheight-1);
	
	byte *inrow, *out, *row1, *row2; 
	out = outdata; 
	fstep = (int) (inheight*65536.0f/outheight); 

	row1 = malloc(outwidth*3); 
	row2 = malloc(outwidth*3); 
	inrow = indata; 
	oldy = 0; 
	R_ResampleShotLerpLine (inrow, row1, inwidth, outwidth); 
	R_ResampleShotLerpLine (inrow + inwidth*3, row2, inwidth, outwidth); 
	for (i = 0, f = 0; i < outheight; i++,f += fstep) 
	{ 
		yi = f >> 16; 
		if (yi != oldy) 
		{ 
			inrow = (byte *)indata + inwidth*3*yi; 
			if (yi == oldy+1) 
				memcpy(row1, row2, outwidth*3); 
			else 
				R_ResampleShotLerpLine (inrow, row1, inwidth, outwidth);

			if (yi < endy) 
				R_ResampleShotLerpLine (inrow + inwidth*3, row2, inwidth, outwidth); 
			else 
				memcpy(row2, row1, outwidth*3); 
			oldy = yi; 
		} 
		if (yi < endy) 
		{ 
			l2 = f & 0xFFFF; 
			l1 = 0x10000 - l2; 
			for (j = 0;j < outwidth;j++) 
			{ 
				*out++ = (byte) ((*row1++ * l1 + *row2++ * l2) >> 16); 
				*out++ = (byte) ((*row1++ * l1 + *row2++ * l2) >> 16); 
				*out++ = (byte) ((*row1++ * l1 + *row2++ * l2) >> 16); 
			} 
			row1 -= outwidth*3; 
			row2 -= outwidth*3; 
		} 
		else // last line has no pixels to lerp to 
		{ 
			for (j = 0;j < outwidth;j++) 
			{ 
				*out++ = *row1++; 
				*out++ = *row1++; 
				*out++ = *row1++; 
			} 
			row1 -= outwidth*3; 
		} 
	} 
	free(row1); 
	free(row2); 
} 


/* 
================== 
R_ScaledScreenshot
by Knightmare
================== 
*/
byte	*saveshotdata;
int		saveshotsize = 256;
void R_ScaledScreenshot (char *name)
{
	struct jpeg_compress_struct		cinfo;
	struct jpeg_error_mgr			jerr;
	JSAMPROW						s[1];
	FILE							*file;
	char							shotname[MAX_OSPATH];
	int								offset;
	
	if (!saveshotdata) return;

	Com_sprintf (shotname, sizeof(shotname), "%s", name);

	// Open the file for Binary Output
	file = fopen(shotname, "wb");
	if (!file)
	{
		VID_Printf (PRINT_ALL, "Menu_ScreenShot: Couldn't create %s\n", name); 
		return;
 	}

	// Initialise the JPEG compression object
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, file);

	// Setup JPEG Parameters
	cinfo.image_width = saveshotsize; //256;
	cinfo.image_height = saveshotsize; //256;
	cinfo.in_color_space = JCS_RGB;
	cinfo.input_components = 3;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 100, TRUE); // was 100

	// Start Compression
	jpeg_start_compress(&cinfo, true);

	// Feed Scanline data
	offset = (cinfo.image_width * cinfo.image_height * 3) - (cinfo.image_width * 3);
	while(cinfo.next_scanline < cinfo.image_height)
	{
		s[0] = &saveshotdata[offset - (cinfo.next_scanline * (cinfo.image_width * 3))];
		jpeg_write_scanlines(&cinfo, s, 1);
	}

	// Finish Compression
	jpeg_finish_compress(&cinfo);

	// Destroy JPEG object
	jpeg_destroy_compress(&cinfo);

	// Close File
	fclose(file);
}


/* 
================== 
R_GrabScreen
by Knightmare
================== 
*/
void R_GrabScreen (void)
{
	byte	*rgbdata;
	
	// Free saveshot buffer first
	if (saveshotdata)
		free(saveshotdata);

	// Optional hi-res saveshots
	if (r_saveshotsize->value && (vid.width >= 1024) && (vid.height >= 1024))
		saveshotsize = 1024;
	else if (r_saveshotsize->value && (vid.width >= 512) && (vid.height >= 512))
		saveshotsize = 512;
	else
		saveshotsize = 256;

	// Allocate room for a copy of the framebuffer
	rgbdata = malloc(vid.width * vid.height * 3);
	if (!rgbdata)
		return;
	// Allocate room for reduced screenshot
	saveshotdata = malloc(saveshotsize * saveshotsize * 3);
	if (!saveshotdata)
	{
		free(rgbdata);
		return;
	}

	// Read the framebuffer into our storage
	qglReadPixels(0, 0, vid.width, vid.height, GL_RGB, GL_UNSIGNED_BYTE, rgbdata);
	// Resize to 256x256
	R_ResampleShot(rgbdata, vid.width, vid.height, saveshotdata, saveshotsize, saveshotsize);

	// Free Temp Framebuffer
	free(rgbdata);
}


/* 
================== 
R_ScreenShot_JPG
By Robert 'Heffo' Heffernan
================== 
*/
void R_ScreenShot_JPG (void)
{
	struct jpeg_compress_struct		cinfo;
	struct jpeg_error_mgr			jerr;
	byte							*rgbdata;
	JSAMPROW						s[1];
	FILE							*file;
	char							picname[80], checkname[MAX_OSPATH];
	int								i, offset;

	// Create the scrnshots directory if it doesn't exist
	Com_sprintf (checkname, sizeof(checkname), "%s/scrnshot", FS_Gamedir());
	Sys_Mkdir (checkname);

	// Knightmare- changed screenshot filenames, up to 100 screenies
	// Find a file name to save it to 
	//strcpy(picname,"quake00.jpg");

	for (i=0; i<=999; i++) 
	{ 
		//picname[5] = i/10 + '0'; 
		//picname[6] = i%10 + '0'; 
		int one, ten, hundred;

		hundred = i*0.01;
		ten = (i - hundred*100)*0.1;
		one = i - hundred*100 - ten*10;

		Com_sprintf (picname, sizeof(picname), "kmquake2_%i%i%i.jpg", hundred, ten, one);
		Com_sprintf (checkname, sizeof(checkname), "%s/scrnshot/%s", FS_Gamedir(), picname);
		file = fopen (checkname, "rb");
		if (!file)
			break;	// file doesn't exist
		fclose (file);
	} 
	if (i==1000) 
	{
		VID_Printf (PRINT_ALL, "SCR_JPGScreenShot_f: Couldn't create a file\n"); 
		return;
 	}

	// Open the file for Binary Output
	file = fopen(checkname, "wb");
	if(!file)
	{
		VID_Printf (PRINT_ALL, "SCR_JPGScreenShot_f: Couldn't create a file\n"); 
		return;
 	}

	// Allocate room for a copy of the framebuffer
	rgbdata = malloc(vid.width * vid.height * 3);
	if(!rgbdata)
	{
		fclose(file);
		return;
	}

	// Read the framebuffer into our storage
	qglReadPixels(0, 0, vid.width, vid.height, GL_RGB, GL_UNSIGNED_BYTE, rgbdata);

	// Initialise the JPEG compression object
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, file);

	// Setup JPEG Parameters
	cinfo.image_width = vid.width;
	cinfo.image_height = vid.height;
	cinfo.in_color_space = JCS_RGB;
	cinfo.input_components = 3;
	jpeg_set_defaults(&cinfo);
	if ((r_screenshot_jpeg_quality->value >= 101) || (r_screenshot_jpeg_quality->value <= 0))
		Cvar_Set("r_screenshot_jpeg_quality", "100");
	jpeg_set_quality(&cinfo, r_screenshot_jpeg_quality->value, TRUE);

	// Start Compression
	jpeg_start_compress(&cinfo, true);

	// Feed Scanline data
	offset = (cinfo.image_width * cinfo.image_height * 3) - (cinfo.image_width * 3);
	while(cinfo.next_scanline < cinfo.image_height)
	{
		s[0] = &rgbdata[offset - (cinfo.next_scanline * (cinfo.image_width * 3))];
		jpeg_write_scanlines(&cinfo, s, 1);
	}

	// Finish Compression
	jpeg_finish_compress(&cinfo);

	// Destroy JPEG object
	jpeg_destroy_compress(&cinfo);

	// Close File
	fclose(file);

	// Free Temp Framebuffer
	free(rgbdata);

	// Done!
	VID_Printf (PRINT_ALL, "Wrote %s\n", picname);
}


/* 
================== 
R_ScreenShot_f
================== 
*/  
void R_ScreenShot_f (void) 
{
	byte		*buffer;
	char		picname[80]; 
	char		checkname[MAX_OSPATH];
	int			i, c, temp;
	FILE		*f;

	// Heffo - JPEG Screenshots
	if(r_screenshot_jpeg->value)
	{
		R_ScreenShot_JPG();
		return;
	}

	// create the scrnshots directory if it doesn't exist
	Com_sprintf (checkname, sizeof(checkname), "%s/scrnshot", FS_Gamedir());
	Sys_Mkdir (checkname);

// 
// find a file name to save it to 
// 

	// Knightmare- changed screenshot filenames, up to 100 screenies
	//strcpy(picname,"quake00.tga");

	for (i=0; i<=999; i++) 
	{ 
		//picname[5] = i/10 + '0'; 
		//picname[6] = i%10 + '0'; 
		int one, ten, hundred;

		hundred = i*0.01;
		ten = (i - hundred*100)*0.1;
		one = i - hundred*100 - ten*10;

		Com_sprintf (picname, sizeof(picname), "kmquake2_%i%i%i.tga", hundred, ten, one);
		Com_sprintf (checkname, sizeof(checkname), "%s/scrnshot/%s", FS_Gamedir(), picname);
		f = fopen (checkname, "rb");
		if (!f)
			break;	// file doesn't exist
		fclose (f);
	} 
	if (i==1000) 
	{
		VID_Printf (PRINT_ALL, "SCR_ScreenShot_f: Couldn't create a file\n"); 
		return;
 	}


	buffer = malloc(vid.width*vid.height*3 + 18);
	memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = vid.width&255;
	buffer[13] = vid.width>>8;
	buffer[14] = vid.height&255;
	buffer[15] = vid.height>>8;
	buffer[16] = 24;	// pixel size

	qglReadPixels (0, 0, vid.width, vid.height, GL_RGB, GL_UNSIGNED_BYTE, buffer+18 ); 

	// swap rgb to bgr
	c = 18+vid.width*vid.height*3;
	for (i=18; i<c; i+=3)
	{
		temp = buffer[i];
		buffer[i] = buffer[i+2];
		buffer[i+2] = temp;
	}

	f = fopen (checkname, "wb");
	fwrite (buffer, 1, c, f);
	fclose (f);

	free (buffer);
	VID_Printf (PRINT_ALL, "Wrote %s\n", picname);
} 


//============================================================================== 


/*
=================
GL_UpdateSwapInterval
=================
*/
void GL_UpdateSwapInterval (void)
{
	if ( r_swapinterval->modified )
	{
		r_swapinterval->modified = false;

		if ( !gl_state.stereo_enabled ) 
		{
#ifdef _WIN32
			if ( qwglSwapIntervalEXT )
				qwglSwapIntervalEXT( r_swapinterval->value );
#endif
		}
	}
}
