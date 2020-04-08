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

// r_image.c

#include "r_local.h"
//#include "r_cin.h"
#include "../include/jpeg/jpeglib.h"
#ifdef PNG_SUPPORT
#if defined (_MSC_VER) && (_MSC_VER <= 1200)	// use older version of libpng for MSVC6
#include "../include/zlibpng_vc6/png.h"
#else
#include "../include/zlibpng/png.h"
#endif
#endif	// PNG_SUPPORT

image_t		gltextures[MAX_GLTEXTURES];
int			numgltextures;
int			base_textureid;		// gltextures[i] = base_textureid+i

static byte			 intensitytable[256];
static unsigned char gammatable[256];

cvar_t		*r_intensity;

unsigned	d_8to24table[256];
float		d_8to24tablef[256][3]; //Knightmare- MrG's Vertex array stuff

qboolean GL_Upload8 (byte *data, int width, int height, imagetype_t type);
qboolean GL_Upload32 (unsigned *data, int width, int height, imagetype_t type);

int		gl_solid_format = 3;
int		gl_alpha_format = 4;
int		gl_tex_solid_format = 3;
int		gl_tex_alpha_format = 4;
int		gl_filter_min = GL_LINEAR_MIPMAP_NEAREST;
int		gl_filter_max = GL_LINEAR;

void GL_SetTexturePalette( unsigned palette[256] )
{
	int i;
	unsigned char temptable[768];

	if ( qglColorTableEXT )//&& gl_ext_palettedtexture->value )
	{
		for ( i = 0; i < 256; i++ )
		{
			temptable[i*3+0] = ( palette[i] >> 0 ) & 0xff;
			temptable[i*3+1] = ( palette[i] >> 8 ) & 0xff;
			temptable[i*3+2] = ( palette[i] >> 16 ) & 0xff;
		}

		qglColorTableEXT( GL_SHARED_TEXTURE_PALETTE_EXT,
						   GL_RGB,
						   256,
						   GL_RGB,
						   GL_UNSIGNED_BYTE,
						   temptable );
	}
}


typedef struct
{
	char *name;
	int	minimize, maximize;
} glmode_t;

glmode_t modes[] = {
	{"GL_NEAREST", GL_NEAREST, GL_NEAREST},
	{"GL_LINEAR", GL_LINEAR, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}
};

#define NUM_GL_MODES (sizeof(modes) / sizeof (glmode_t))

typedef struct
{
	char *name;
	int mode;
} gltmode_t;

gltmode_t gl_alpha_modes[] = {
	{"default", 4},
	{"GL_RGBA", GL_RGBA},
	{"GL_RGBA8", GL_RGBA8},
	{"GL_RGB5_A1", GL_RGB5_A1},
	{"GL_RGBA4", GL_RGBA4},
	{"GL_RGBA2", GL_RGBA2},
};

#define NUM_GL_ALPHA_MODES (sizeof(gl_alpha_modes) / sizeof (gltmode_t))

gltmode_t gl_solid_modes[] = {
	{"default", 3},
	{"GL_RGB", GL_RGB},
	{"GL_RGB8", GL_RGB8},
	{"GL_RGB5", GL_RGB5},
	{"GL_RGB4", GL_RGB4},
	{"GL_R3_G3_B2", GL_R3_G3_B2},
#ifdef GL_RGB2_EXT
	{"GL_RGB2", GL_RGB2_EXT},
#endif
};

#define NUM_GL_SOLID_MODES (sizeof(gl_solid_modes) / sizeof (gltmode_t))

/*
===============
GL_TextureMode
===============
*/
void GL_TextureMode( char *string )
{
	int		i;
	image_t	*glt;

	for (i=0 ; i< NUM_GL_MODES ; i++)
	{
		if ( !Q_stricmp( modes[i].name, string ) )
			break;
	}

	if (i == NUM_GL_MODES)
	{
		VID_Printf (PRINT_ALL, "bad filter name\n");
		return;
	}

	gl_filter_min = modes[i].minimize;
	gl_filter_max = modes[i].maximize;

	// clamp selected anisotropy
	if (glConfig.anisotropic)
	{
		if (r_anisotropic->value > glConfig.max_anisotropy)
			Cvar_SetValue("r_anisotropic", glConfig.max_anisotropy);
		else if (r_anisotropic->value < 1.0)
			Cvar_SetValue("r_anisotropic", 1.0);
	}

	// change all the existing mipmap texture objects
	for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
	{
		if (glt->type != it_pic && glt->type != it_sky)
		{
			GL_Bind (glt->texnum);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

			// Set anisotropic filter if supported and enabled
			if (glConfig.anisotropic && r_anisotropic->value)
			{
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, r_anisotropic->value);
				//GLfloat largest_supported_anisotropy;
				//qglGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
				//qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);
			}
		}
	}
}

/*
===============
GL_TextureAlphaMode
===============
*/
void GL_TextureAlphaMode( char *string )
{
	int		i;

	for (i=0 ; i< NUM_GL_ALPHA_MODES ; i++)
	{
		if ( !Q_stricmp( gl_alpha_modes[i].name, string ) )
			break;
	}

	if (i == NUM_GL_ALPHA_MODES)
	{
		VID_Printf (PRINT_ALL, "bad alpha texture mode name\n");
		return;
	}

	gl_tex_alpha_format = gl_alpha_modes[i].mode;
}

/*
===============
GL_TextureSolidMode
===============
*/
void GL_TextureSolidMode( char *string )
{
	int		i;

	for (i=0 ; i< NUM_GL_SOLID_MODES ; i++)
	{
		if ( !Q_stricmp( gl_solid_modes[i].name, string ) )
			break;
	}

	if (i == NUM_GL_SOLID_MODES)
	{
		VID_Printf (PRINT_ALL, "bad solid texture mode name\n");
		return;
	}

	gl_tex_solid_format = gl_solid_modes[i].mode;
}

/*
===============
R_ImageList_f
===============
*/
void R_ImageList_f (void)
{
	int		i;
	image_t	*image;
	int		texels;
	const char *palstrings[2] =
	{
		"RGB",
		"PAL"
	};

	VID_Printf (PRINT_ALL, "------------------\n");
	texels = 0;

	for (i=0, image=gltextures; i<numgltextures; i++, image++)
	{
		if (image->texnum <= 0)
			continue;
		texels += image->upload_width*image->upload_height;
		switch (image->type)
		{
		case it_skin:
			VID_Printf (PRINT_ALL, "M");
			break;
		case it_sprite:
			VID_Printf (PRINT_ALL, "S");
			break;
		case it_wall:
			VID_Printf (PRINT_ALL, "W");
			break;
		case it_pic:
			VID_Printf (PRINT_ALL, "P");
			break;
		case it_part:
			VID_Printf (PRINT_ALL, "P");
			break;
		default:
			VID_Printf (PRINT_ALL, " ");
			break;
		}

		VID_Printf (PRINT_ALL,  " %3i %3i %s: %s\n",
			image->upload_width, image->upload_height, palstrings[image->paletted], image->name);
	}
	VID_Printf (PRINT_ALL, "Total texel count (not counting mipmaps): %i\n", texels);
}


/*
=============================================================================

  scrap allocation

  Allocate all the little status bar obejcts into a single texture
  to crutch up inefficient hardware / drivers

=============================================================================
*/

#define	MAX_SCRAPS		1
#define	BLOCK_WIDTH		256
#define	BLOCK_HEIGHT	256

int			scrap_allocated[MAX_SCRAPS][BLOCK_WIDTH];
byte		scrap_texels[MAX_SCRAPS][BLOCK_WIDTH*BLOCK_HEIGHT];
qboolean	scrap_dirty;

// returns a texture number and the position inside it
int Scrap_AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;
	int		texnum;

	for (texnum=0 ; texnum<MAX_SCRAPS ; texnum++)
	{
		best = BLOCK_HEIGHT;

		for (i=0 ; i<BLOCK_WIDTH-w ; i++)
		{
			best2 = 0;

			for (j=0 ; j<w ; j++)
			{
				if (scrap_allocated[texnum][i+j] >= best)
					break;
				if (scrap_allocated[texnum][i+j] > best2)
					best2 = scrap_allocated[texnum][i+j];
			}
			if (j == w)
			{	// this is a valid spot
				*x = i;
				*y = best = best2;
			}
		}

		if (best + h > BLOCK_HEIGHT)
			continue;

		for (i=0 ; i<w ; i++)
			scrap_allocated[texnum][*x + i] = best + h;

		return texnum;
	}

	return -1;
//	Sys_Error ("Scrap_AllocBlock: full");
}

int	scrap_uploads;

void Scrap_Upload (void)
{
	scrap_uploads++;
	GL_Bind(TEXNUM_SCRAPS);
	GL_Upload8 (scrap_texels[0], BLOCK_WIDTH, BLOCK_HEIGHT, it_pic);
	scrap_dirty = false;
}

/*
=================================================================

PCX LOADING

=================================================================
*/


/*
==============
LoadPCX
==============
*/
void LoadPCX (char *filename, byte **pic, byte **palette, int *width, int *height)
{
	byte	*raw;
	pcx_t	*pcx;
	int		x, y;
	int		len;
	int		dataByte, runLength;
	byte	*out, *pix;

	*pic = NULL;
	*palette = NULL;

	//
	// load the file
	//
	len = FS_LoadFile (filename, (void **)&raw);
	if (!raw)
	{
		// Knightmare- skip this unless developer >= 2 because it spams the console
		if (developer->integer > 1)
			VID_Printf (PRINT_DEVELOPER, "Bad pcx file %s\n", filename);
		return;
	}

	//
	// parse the PCX file
	//
	pcx = (pcx_t *)raw;

    pcx->xmin = LittleShort(pcx->xmin);
    pcx->ymin = LittleShort(pcx->ymin);
    pcx->xmax = LittleShort(pcx->xmax);
    pcx->ymax = LittleShort(pcx->ymax);
    pcx->hres = LittleShort(pcx->hres);
    pcx->vres = LittleShort(pcx->vres);
    pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
    pcx->palette_type = LittleShort(pcx->palette_type);

	raw = &pcx->data;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640
		|| pcx->ymax >= 480)
	{
		VID_Printf (PRINT_ALL, "Bad pcx file %s\n", filename);
		return;
	}

	out = malloc ( (pcx->ymax+1) * (pcx->xmax+1) );

	*pic = out;

	pix = out;

	if (palette)
	{
		*palette = malloc(768);
		memcpy (*palette, (byte *)pcx + len - 768, 768);
	}

	if (width)
		*width = pcx->xmax+1;
	if (height)
		*height = pcx->ymax+1;

	for (y=0 ; y<=pcx->ymax ; y++, pix += pcx->xmax+1)
	{
		for (x=0 ; x<=pcx->xmax ; )
		{
			dataByte = *raw++;

			if((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}
			else
				runLength = 1;

			while(runLength-- > 0)
				pix[x++] = dataByte;
		}

	}

	if ( raw - (byte *)pcx > len)
	{
		VID_Printf (PRINT_DEVELOPER, "PCX file %s was malformed", filename);
		free (*pic);
		*pic = NULL;
	}

	FS_FreeFile (pcx);
}

/*
=========================================================

TARGA LOADING

=========================================================
*/

// Definitions for image types
#define TGA_Null      0   // no image data
#define TGA_Map         1   // Uncompressed, color-mapped images
#define TGA_RGB         2   // Uncompressed, RGB images
#define TGA_Mono      3   // Uncompressed, black and white images
#define TGA_RLEMap      9   // Runlength encoded color-mapped images
#define TGA_RLERGB      10   // Runlength encoded RGB images
#define TGA_RLEMono      11   // Compressed, black and white images
#define TGA_CompMap      32   // Compressed color-mapped data, using Huffman, Delta, and runlength encoding
#define TGA_CompMap4   33   // Compressed color-mapped data, using Huffman, Delta, and runlength encoding. 4-pass quadtree-type process
// Definitions for interleave flag
#define TGA_IL_None      0   // non-interleaved
#define TGA_IL_Two      1   // two-way (even/odd) interleaving
#define TGA_IL_Four      2   // four way interleaving
#define TGA_IL_Reserved   3   // reserved
// Definitions for origin flag
#define TGA_O_UPPER      0   // Origin in lower left-hand corner
#define TGA_O_LOWER      1   // Origin in upper left-hand corner
#define MAXCOLORS 16384

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;

#if 1

/*
=============
R_LoadTGA
NiceAss: LoadTGA() from Q2Ice, it supports more formats
=============
*/
void R_LoadTGA( char *filename, byte **pic, int *width, int *height )
{
   int         w, h, x, y, i, temp1, temp2;
   int         realrow, truerow, baserow, size, interleave, origin;
   int         pixel_size, map_idx, mapped, rlencoded, RLE_count, RLE_flag;
   TargaHeader   header;
   byte      tmp[2], r, g, b, a, j, k, l;
   byte      *dst, *ColorMap, *data, *pdata;

   // load file
   FS_LoadFile( filename, &data );

   if ( !data )
      return;

   pdata = data;

   header.id_length = *pdata++;
   header.colormap_type = *pdata++;
   header.image_type = *pdata++;
   
   tmp[0] = pdata[0];
   tmp[1] = pdata[1];
   header.colormap_index = LittleShort( *((short *)tmp) );
   pdata+=2;
   tmp[0] = pdata[0];
   tmp[1] = pdata[1];
   header.colormap_length = LittleShort( *((short *)tmp) );
   pdata+=2;
   header.colormap_size = *pdata++;
   header.x_origin = LittleShort( *((short *)pdata) );
   pdata+=2;
   header.y_origin = LittleShort( *((short *)pdata) );
   pdata+=2;
   header.width = LittleShort( *((short *)pdata) );
   pdata+=2;
   header.height = LittleShort( *((short *)pdata) );
   pdata+=2;
   header.pixel_size = *pdata++;
   header.attributes = *pdata++;

   if( header.id_length )
      pdata += header.id_length;

   // validate TGA type
   switch( header.image_type ) {
      case TGA_Map:
      case TGA_RGB:
      case TGA_Mono:
      case TGA_RLEMap:
      case TGA_RLERGB:
      case TGA_RLEMono:
         break;
      default:
         VID_Error ( ERR_DROP, "R_LoadTGA: Only type 1 (map), 2 (RGB), 3 (mono), 9 (RLEmap), 10 (RLERGB), 11 (RLEmono) TGA images supported\n" );
         return;
   }

   // validate color depth
   switch( header.pixel_size ) {
      case 8:
      case 15:
      case 16:
      case 24:
      case 32:
         break;
      default:
         VID_Error ( ERR_DROP, "R_LoadTGA: Only 8, 15, 16, 24 and 32 bit images (with colormaps) supported\n" );
         return;
   }

   r = g = b = a = l = 0;

   // if required, read the color map information
   ColorMap = NULL;
   mapped = ( header.image_type == TGA_Map || header.image_type == TGA_RLEMap || header.image_type == TGA_CompMap || header.image_type == TGA_CompMap4 ) && header.colormap_type == 1;
   if( mapped ) {
      // validate colormap size
      switch( header.colormap_size ) {
         case 8:
         case 16:
         case 32:
         case 24:
            break;
         default:
            VID_Error ( ERR_DROP, "R_LoadTGA: Only 8, 16, 24 and 32 bit colormaps supported\n" );
            return;
      }

      temp1 = header.colormap_index;
      temp2 = header.colormap_length;
      if( (temp1 + temp2 + 1) >= MAXCOLORS ) {
         FS_FreeFile( data );
         return;
      }
      ColorMap = (byte *)malloc( MAXCOLORS * 4 );
      map_idx = 0;
      for( i = temp1; i < temp1 + temp2; ++i, map_idx += 4 ) {
         // read appropriate number of bytes, break into rgb & put in map
         switch( header.colormap_size ) {
            case 8:
               r = g = b = *pdata++;
               a = 255;
               break;
            case 15:
               j = *pdata++;
               k = *pdata++;
               l = ((unsigned int) k << 8) + j;
               r = (byte) ( ((k & 0x7C) >> 2) << 3 );
               g = (byte) ( (((k & 0x03) << 3) + ((j & 0xE0) >> 5)) << 3 );
               b = (byte) ( (j & 0x1F) << 3 );
               a = 255;
               break;
            case 16:
               j = *pdata++;
               k = *pdata++;
               l = ((unsigned int) k << 8) + j;
               r = (byte) ( ((k & 0x7C) >> 2) << 3 );
               g = (byte) ( (((k & 0x03) << 3) + ((j & 0xE0) >> 5)) << 3 );
               b = (byte) ( (j & 0x1F) << 3 );
               a = (k & 0x80) ? 255 : 0;
               break;
            case 24:
               b = *pdata++;
               g = *pdata++;
               r = *pdata++;
               a = 255;
               l = 0;
               break;
            case 32:
               b = *pdata++;
               g = *pdata++;
               r = *pdata++;
               a = *pdata++;
               l = 0;
               break;
         }
         ColorMap[map_idx + 0] = r;
         ColorMap[map_idx + 1] = g;
         ColorMap[map_idx + 2] = b;
         ColorMap[map_idx + 3] = a;
      }
   }

   // check run-length encoding
   rlencoded = header.image_type == TGA_RLEMap || header.image_type == TGA_RLERGB || header.image_type == TGA_RLEMono;
   RLE_count = 0;
   RLE_flag = 0;

   w = header.width;
   h = header.height;

   if( width )
      *width = w;
   if( height )
      *height = h;

   size = w * h * 4;
   *pic = (byte *)malloc( size );

   memset( *pic, 0, size );

   // read the Targa file body and convert to portable format
   pixel_size = header.pixel_size;
   origin = (header.attributes & 0x20) >> 5;
   interleave = (header.attributes & 0xC0) >> 6;
   truerow = 0;
   baserow = 0;
   for( y = 0; y < h; y++ ) {
      realrow = truerow;
      if( origin == TGA_O_UPPER )
         realrow = h - realrow - 1;

      dst = *pic + realrow * w * 4;

      for( x = 0; x < w; x++ ) {
         // check if run length encoded
         if( rlencoded ) {
            if( !RLE_count ) {
               // have to restart run
               i = *pdata++;
               RLE_flag = (i & 0x80);
               if( !RLE_flag ) {
                  // stream of unencoded pixels
                  RLE_count = i + 1;
               } else {
                  // single pixel replicated
                  RLE_count = i - 127;
               }
               // decrement count & get pixel
               --RLE_count;
            } else {
               // have already read count & (at least) first pixel
               --RLE_count;
               if( RLE_flag )
                  // replicated pixels
                  goto PixEncode;
            }
         }

         // read appropriate number of bytes, break into RGB
         switch( pixel_size ) {
            case 8:
               r = g = b = l = *pdata++;
               a = 255;
               break;
            case 15:
               j = *pdata++;
               k = *pdata++;
               l = ((unsigned int) k << 8) + j;
               r = (byte) ( ((k & 0x7C) >> 2) << 3 );
               g = (byte) ( (((k & 0x03) << 3) + ((j & 0xE0) >> 5)) << 3 );
               b = (byte) ( (j & 0x1F) << 3 );
               a = 255;
               break;
            case 16:
               j = *pdata++;
               k = *pdata++;
               l = ((unsigned int) k << 8) + j;
               r = (byte) ( ((k & 0x7C) >> 2) << 3 );
               g = (byte) ( (((k & 0x03) << 3) + ((j & 0xE0) >> 5)) << 3 );
               b = (byte) ( (j & 0x1F) << 3 );
               a = 255;
               break;
            case 24:
               b = *pdata++;
               g = *pdata++;
               r = *pdata++;
               a = 255;
               l = 0;
               break;
            case 32:
               b = *pdata++;
               g = *pdata++;
               r = *pdata++;
               a = *pdata++;
               l = 0;
               break;
            default:
               VID_Error( ERR_DROP, "Illegal pixel_size '%d' in file '%s'\n", filename );
               return;
         }

PixEncode:
         if ( mapped )
         {
            map_idx = l * 4;
            *dst++ = ColorMap[map_idx + 0];
            *dst++ = ColorMap[map_idx + 1];
            *dst++ = ColorMap[map_idx + 2];
            *dst++ = ColorMap[map_idx + 3];
         }
         else
         {
            *dst++ = r;
            *dst++ = g;
            *dst++ = b;
            *dst++ = a;
         }
      }

      if (interleave == TGA_IL_Four)
         truerow += 4;
      else if (interleave == TGA_IL_Two)
         truerow += 2;
      else
         truerow++;

      if (truerow >= h)
         truerow = ++baserow;
   }

   if (mapped)
      free( ColorMap );
   
   FS_FreeFile( data );
}

#else

/*
=============
R_LoadTGA
=============
*/
void R_LoadTGA (char *name, byte **pic, int *width, int *height)
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	byte	*buffer;
	int		length;
	TargaHeader		targa_header;
	byte			*targa_rgba;
	byte tmp[2];

	*pic = NULL;

	//
	// load the file
	//
	length = FS_LoadFile (name, (void **)&buffer);
	if (!buffer)
	{	// Knightmare- removed this because it spams the console
		//VID_Printf (PRINT_DEVELOPER, "Bad tga file %s\n", name);
		return;
	}

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;
	
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_index = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_length = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.y_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.width = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.height = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	// Knightmare- check for bad data
	if (!targa_header.width || !targa_header.height) {
		VID_Printf (PRINT_ALL, "Bad tga file %s\n", name);
		FS_FreeFile (buffer);
		return;
	}

	if (targa_header.image_type != 2 
		&& targa_header.image_type != 10) {
	//	VID_Error (ERR_DROP, "R_LoadTGA: Only type 2 and 10 targa RGB images supported\n");
		VID_Printf (PRINT_ALL, "R_LoadTGA: %s has wrong image format; only type 2 and 10 targa RGB images supported.\n", name);
		FS_FreeFile (buffer);
		return;
	}

	if (targa_header.colormap_type !=0 
		|| (targa_header.pixel_size!=32 && targa_header.pixel_size!=24)) {
	//	VID_Error (ERR_DROP, "R_LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");
		VID_Printf (PRINT_ALL, "R_LoadTGA: %s has wrong image format; only 32 or 24 bit images supported (no colormaps).\n", name);
		FS_FreeFile (buffer);
		return;
	}

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

	targa_rgba = malloc (numPixels*4);
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment
	
	if (targa_header.image_type==2) {  // Uncompressed, RGB images
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; column++) {
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size) {
					case 24:
							
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
					case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) {   // Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; ) {
				packetHeader= *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					switch (targa_header.pixel_size) {
						case 24:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = 255;
								break;
						case 32:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
								break;
					}
	
					for(j=0;j<packetSize;j++) {
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;
						if (column==columns) { // run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}
					}
				}
				else {                            // non run-length packet
					for(j=0;j<packetSize;j++) {
						switch (targa_header.pixel_size) {
							case 24:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = 255;
									break;
							case 32:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									alphabyte = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = alphabyte;
									break;
						}
						column++;
						if (column==columns) { // pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}						
					}
				}
			}
			breakOut:;
		}
	}

	FS_FreeFile (buffer);
}

#endif

/*
=================================================================

PNG LOADING

From Quake2Max

=================================================================
*/
#ifdef PNG_SUPPORT

// Knightmare- abstracted function call for png_set_gray_1_2_4_to_8()
#if defined (PNG_LIBPNG_VER) && (PNG_LIBPNG_VER < 10209)
#define PNG_GRAYFUNC_1_2_4_to_8(a)	png_set_gray_1_2_4_to_8(a)
#else
#define PNG_GRAYFUNC_1_2_4_to_8(a)	png_set_expand_gray_1_2_4_to_8(a)
#endif

typedef struct png_handle_s 
{
	char	*tmpBuf;
	int		tmpi;
	long	fBgColor;		// DL Background color Added 30/05/2000
	int		fTransparent;	// DL Is this Image Transparent?   Added 30/05/2000
	long	fRowBytes;		// DL Added 30/05/2000
	double	fGamma;			// DL Added 07/06/2000
	double	fScreenGamma;	// DL Added 07/06/2000
	char	*fRowPtrs;		// DL Changed for consistancy 30/05/2000  
	char	*data;			// property data: pByte read fData;
	char	*title;
	char	*author;
	char	*description;
	int		bitDepth;
	int		bytesPerPixel;
	int		colorType;
	int		height;
	int		width;
	int		interlace;
	int		compression;
	int		filter;
	double	lastModified;
	int		transparent;
} png_handle_t;
png_handle_t	*r_png_handle = 0;

void R_InitializePNGData (void) 
{
	long* cvaluep; //ub
	long y;

	// Initialize Data and RowPtrs
	if (r_png_handle->data) 
	{
		free(r_png_handle->data);
		r_png_handle->data = 0;
	}
	if (r_png_handle->fRowPtrs) 
	{
		free(r_png_handle->fRowPtrs);
		r_png_handle->fRowPtrs = 0;
	}
	r_png_handle->data = malloc(r_png_handle->height * r_png_handle->fRowBytes ); // DL Added 30/5/2000
	r_png_handle->fRowPtrs = malloc(sizeof(void*) * r_png_handle->height);

	if ((r_png_handle->data)&&(r_png_handle->fRowPtrs)) 
	{
		cvaluep = (long*)r_png_handle->fRowPtrs;    
		for (y=0; y<r_png_handle->height; y++)
		{
			cvaluep[y] = (long)r_png_handle->data + ( y * (long)r_png_handle->fRowBytes ); //DL Added 08/07/2000      
		}
	}
}

void R_CreatePNG (void) 
{
	if (r_png_handle)
		return;
	r_png_handle = malloc(sizeof(png_handle_t));
	r_png_handle->data    = 0;
	r_png_handle->fRowPtrs = 0;
	r_png_handle->height  = 0;
	r_png_handle->width   = 0;
	r_png_handle->colorType = PNG_COLOR_TYPE_RGBA;	// was PNG_COLOR_TYPE_RGB
	r_png_handle->interlace = PNG_INTERLACE_NONE;
	r_png_handle->compression = PNG_COMPRESSION_TYPE_DEFAULT;
	r_png_handle->filter = PNG_FILTER_TYPE_DEFAULT;
}

void R_DestroyPNG (qboolean keepData) 
{  
	if (!r_png_handle) 
		return;
	if (r_png_handle->data && !keepData) 
		free(r_png_handle->data);
	if (r_png_handle->fRowPtrs) 
		free(r_png_handle->fRowPtrs);
	free(r_png_handle);
	r_png_handle = NULL;
}

void PNGAPI R_ReadPNGData (png_structp png, png_bytep data, png_size_t length) 
{	// called by pnglib
	int	i;

	for (i=0; i<length; i++) 
		data[i] = r_png_handle->tmpBuf[r_png_handle->tmpi++];    // give pnglib a some more bytes  
}


/*
==============
R_LoadPNG
==============
*/
void R_LoadPNG (char *filename, byte **pic, int *width, int *height) 
{
	png_structp	png;
	png_infop	pnginfo;
	byte		ioBuffer[8192];
	int			len;
	byte		*raw;

	*pic = NULL;

	len = FS_LoadFile (filename, (void **)&raw);

	if (!raw)
	{	// Knightmare- skip this unless developer >= 2 because it spams the console
		if (developer->integer > 1)
			VID_Printf (PRINT_DEVELOPER, "Bad png file %s\n", filename);
		return;
	}

	if ( png_sig_cmp(raw,0,4) ) 
		return;  

	png = png_create_read_struct (PNG_LIBPNG_VER_STRING,0,0,0);
	if (!png) 
		return;

	pnginfo = png_create_info_struct (png);

	if (!pnginfo) {
		png_destroy_read_struct (&png,&pnginfo,0);
		return;
	}

	png_set_sig_bytes (png, 0/*sizeof( sig )*/);

	R_CreatePNG (); // creates the r_png_handle struct

	r_png_handle->tmpBuf = raw; //buf = whole file content
	r_png_handle->tmpi=0; 
	png_set_read_fn (png, ioBuffer, R_ReadPNGData);
	png_read_info (png, pnginfo);

	png_get_IHDR (png, pnginfo, &r_png_handle->width, &r_png_handle->height, &r_png_handle->bitDepth,
				&r_png_handle->colorType, &r_png_handle->interlace, &r_png_handle->compression, &r_png_handle->filter);
	// ...removed bgColor code here...

	if (r_png_handle->colorType == PNG_COLOR_TYPE_PALETTE)  
		png_set_palette_to_rgb(png);
	if (r_png_handle->colorType == PNG_COLOR_TYPE_GRAY && r_png_handle->bitDepth < 8) 
	//	png_set_gray_1_2_4_to_8 (png);
		PNG_GRAYFUNC_1_2_4_to_8 (png);

	// Add alpha channel if present
	if ( png_get_valid(png, pnginfo, PNG_INFO_tRNS) )
		png_set_tRNS_to_alpha (png);

	// hax: expand 24bit to 32bit
//	if (r_png_handle->bitDepth == 8 && r_png_handle->colorType == PNG_COLOR_TYPE_RGB) 
//		png_set_filler (png,255, PNG_FILLER_AFTER);

	// Expand grayscale to RGB
	if ( (r_png_handle->colorType == PNG_COLOR_TYPE_GRAY) || (r_png_handle->colorType == PNG_COLOR_TYPE_GRAY_ALPHA) )
		png_set_gray_to_rgb (png);

	if (r_png_handle->bitDepth < 8)
		png_set_expand (png);

	// hax: expand 24bit to 32bit
	if (r_png_handle->bitDepth == 8 && r_png_handle->colorType == PNG_COLOR_TYPE_RGB) 
		png_set_filler (png,255, PNG_FILLER_AFTER);

	// update the info structure
	png_read_update_info( png, pnginfo );

	r_png_handle->fRowBytes = png_get_rowbytes (png, pnginfo);
	r_png_handle->bytesPerPixel = png_get_channels (png, pnginfo);  // DL Added 30/08/2000

	R_InitializePNGData ();
	if ((r_png_handle->data)&&(r_png_handle->fRowPtrs))
		png_read_image (png, (png_bytepp)r_png_handle->fRowPtrs);

	png_read_end (png, pnginfo); // read last information chunks

	png_destroy_read_struct (&png, &pnginfo, 0);

	// only load 32 bit by now...
	if (r_png_handle->bitDepth == 8)
	{
		*pic= r_png_handle->data;
		*width = r_png_handle->width;
		*height = r_png_handle->height;
	}
	else
	{
		VID_Printf (PRINT_DEVELOPER, "Bad png color depth: %s\n", filename);
		*pic = NULL;
		free (r_png_handle->data);
	}

	R_DestroyPNG (true);
	FS_FreeFile ((void *)raw);
}
#endif	// PNG_SUPPORT

/*
=================================================================

JPEG LOADING

By Robert 'Heffo' Heffernan

=================================================================
*/

void jpg_null(j_decompress_ptr cinfo)
{
}

unsigned char jpg_fill_input_buffer(j_decompress_ptr cinfo)
{
    VID_Printf(PRINT_ALL, "Premature end of JPEG data\n");
    return 1;
}

void jpg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
        
    cinfo->src->next_input_byte += (size_t) num_bytes;
    cinfo->src->bytes_in_buffer -= (size_t) num_bytes;

    if (cinfo->src->bytes_in_buffer < 0) 
		VID_Printf(PRINT_ALL, "Premature end of JPEG data\n");
}

//void jpeg_mem_src(j_decompress_ptr cinfo, byte *mem, int len)
void jpeg_mem_src(j_decompress_ptr cinfo, const byte *mem, unsigned long len)
{
    cinfo->src = (struct jpeg_source_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));
    cinfo->src->init_source = jpg_null;
    cinfo->src->fill_input_buffer = jpg_fill_input_buffer;
    cinfo->src->skip_input_data = jpg_skip_input_data;
    cinfo->src->resync_to_restart = jpeg_resync_to_restart;
    cinfo->src->term_source = jpg_null;
    cinfo->src->bytes_in_buffer = len;
    cinfo->src->next_input_byte = mem;
}

#define DSTATE_START	200	/* after create_decompress */
#define DSTATE_INHEADER	201	/* reading header markers, no SOS yet */

/*
==============
R_LoadJPG
==============
*/
void R_LoadJPG (char *filename, byte **pic, int *width, int *height)
{
	struct jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr			jerr;
	byte							*rawdata, *rgbadata, *scanline, *p, *q;
	int								rawsize, i;

	// Load JPEG file into memory
	rawsize = FS_LoadFile(filename, (void **)&rawdata);
	if (!rawdata)
	{
		// Knightmare- skip this unless developer >= 2 because it spams the console
		if (developer->integer > 1)
			VID_Printf (PRINT_DEVELOPER, "Bad jpg file %s\n", filename);
		return;	
	}

	// Knightmare- check for bad data
	if (	rawdata[6] != 'J'
		||	rawdata[7] != 'F'
		||	rawdata[8] != 'I'
		||	rawdata[9] != 'F') {
		VID_Printf (PRINT_ALL, "Bad jpg file %s\n", filename);
		FS_FreeFile(rawdata);
		return;
	}

	// Initialise libJpeg Object
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// Feed JPEG memory into the libJpeg Object
	jpeg_mem_src(&cinfo, rawdata, rawsize);

	// Process JPEG header
	jpeg_read_header(&cinfo, true); // bombs out here

	// Start Decompression
	jpeg_start_decompress(&cinfo);

	// Check Color Components
	if (cinfo.output_components != 3)
	{
		VID_Printf(PRINT_ALL, "Invalid JPEG color components in %s\n", filename);
		jpeg_destroy_decompress(&cinfo);
		FS_FreeFile(rawdata);
		return;
	}

	// Allocate Memory for decompressed image
	rgbadata = malloc(cinfo.output_width * cinfo.output_height * 4);
	if (!rgbadata)
	{
		VID_Printf(PRINT_ALL, "Insufficient RAM for JPEG buffer\n");
		jpeg_destroy_decompress(&cinfo);
		FS_FreeFile(rawdata);
		return;
	}

	// Pass sizes to output
	*width = cinfo.output_width; *height = cinfo.output_height;

	// Allocate Scanline buffer
	scanline = malloc(cinfo.output_width * 3);
	if (!scanline)
	{
		VID_Printf(PRINT_ALL, "Insufficient RAM for JPEG scanline buffer\n");
		free(rgbadata);
		jpeg_destroy_decompress(&cinfo);
		FS_FreeFile(rawdata);
		return;
	}

	// Read Scanlines, and expand from RGB to RGBA
	q = rgbadata;
	while (cinfo.output_scanline < cinfo.output_height)
	{
		p = scanline;
		jpeg_read_scanlines(&cinfo, &scanline, 1);

		for (i=0; i<cinfo.output_width; i++)
		{
			q[0] = p[0];
			q[1] = p[1];
			q[2] = p[2];
			q[3] = 255;

			p+=3; q+=4;
		}
	}

	// Free the scanline buffer
	free(scanline);

	// Finish Decompression
	jpeg_finish_decompress(&cinfo);

	// Destroy JPEG object
	jpeg_destroy_decompress(&cinfo);

	// Free raw data buffer
	FS_FreeFile(rawdata);

	// Return the 'rgbadata'
	*pic = rgbadata;
}


/*
====================================================================

IMAGE FLOOD FILLING

====================================================================
*/


/*
=================
Mod_FloodFillSkin

Fill background pixels so mipmapping doesn't have haloes
=================
*/

typedef struct
{
	short		x, y;
} floodfill_t;

// must be a power of 2
#define FLOODFILL_FIFO_SIZE 0x1000
#define FLOODFILL_FIFO_MASK (FLOODFILL_FIFO_SIZE - 1)

#define FLOODFILL_STEP( off, dx, dy ) \
{ \
	if (pos[off] == fillcolor) \
	{ \
		pos[off] = 255; \
		fifo[inpt].x = x + (dx), fifo[inpt].y = y + (dy); \
		inpt = (inpt + 1) & FLOODFILL_FIFO_MASK; \
	} \
	else if (pos[off] != 255) fdc = pos[off]; \
}

void R_FloodFillSkin( byte *skin, int skinwidth, int skinheight )
{
	byte				fillcolor = *skin; // assume this is the pixel to fill
	floodfill_t			fifo[FLOODFILL_FIFO_SIZE];
	int					inpt = 0, outpt = 0;
	int					filledcolor = -1;
	int					i;

	if (filledcolor == -1)
	{
		filledcolor = 0;
		// attempt to find opaque black
		for (i = 0; i < 256; ++i)
			if (d_8to24table[i] == (255 << 0)) // alpha 1.0
			{
				filledcolor = i;
				break;
			}
	}

	// can't fill to filled color or to transparent color (used as visited marker)
	if ((fillcolor == filledcolor) || (fillcolor == 255))
	{
		//printf( "not filling skin from %d to %d\n", fillcolor, filledcolor );
		return;
	}

	fifo[inpt].x = 0, fifo[inpt].y = 0;
	inpt = (inpt + 1) & FLOODFILL_FIFO_MASK;

	while (outpt != inpt)
	{
		int			x = fifo[outpt].x, y = fifo[outpt].y;
		int			fdc = filledcolor;
		byte		*pos = &skin[x + skinwidth * y];

		outpt = (outpt + 1) & FLOODFILL_FIFO_MASK;

		if (x > 0)				FLOODFILL_STEP( -1, -1, 0 );
		if (x < skinwidth - 1)	FLOODFILL_STEP( 1, 1, 0 );
		if (y > 0)				FLOODFILL_STEP( -skinwidth, 0, -1 );
		if (y < skinheight - 1)	FLOODFILL_STEP( skinwidth, 0, 1 );
		skin[x + skinwidth * y] = fdc;
	}
}

//=======================================================



/*
================
GL_ResampleTextureLerpLine
from DarkPlaces
================
*/

void GL_ResampleTextureLerpLine (byte *in, byte *out, int inwidth, int outwidth) 
{ 
	int j, xi, oldx = 0, f, fstep, l1, l2, endx;

	fstep = (int) (inwidth*65536.0f/outwidth); 
	endx = (inwidth-1); 
	for (j = 0,f = 0;j < outwidth;j++, f += fstep) 
	{ 
		xi = (int) f >> 16; 
		if (xi != oldx) 
		{ 
			in += (xi - oldx) * 4; 
			oldx = xi; 
		} 
		if (xi < endx) 
		{ 
			l2 = f & 0xFFFF; 
			l1 = 0x10000 - l2; 
			*out++ = (byte) ((in[0] * l1 + in[4] * l2) >> 16);
			*out++ = (byte) ((in[1] * l1 + in[5] * l2) >> 16); 
			*out++ = (byte) ((in[2] * l1 + in[6] * l2) >> 16); 
			*out++ = (byte) ((in[3] * l1 + in[7] * l2) >> 16); 
		} 
		else // last pixel of the line has no pixel to lerp to 
		{ 
			*out++ = in[0]; 
			*out++ = in[1]; 
			*out++ = in[2]; 
			*out++ = in[3]; 
		} 
	} 
}

/*
================
GL_ResampleTexture
================
*/
void GL_ResampleTexture (void *indata, int inwidth, int inheight, void *outdata, int outwidth, int outheight) 
{ 
	int i, j, yi, oldy, f, fstep, l1, l2, endy = (inheight-1);
	
	byte *inrow, *out, *row1, *row2; 
	out = outdata; 
	fstep = (int) (inheight*65536.0f/outheight); 

	row1 = malloc(outwidth*4); 
	row2 = malloc(outwidth*4); 
	inrow = indata; 
	oldy = 0; 
	GL_ResampleTextureLerpLine (inrow, row1, inwidth, outwidth); 
	GL_ResampleTextureLerpLine (inrow + inwidth*4, row2, inwidth, outwidth); 
	for (i = 0, f = 0;i < outheight;i++,f += fstep) 
	{ 
		yi = f >> 16; 
		if (yi != oldy) 
		{ 
			inrow = (byte *)indata + inwidth*4*yi; 
			if (yi == oldy+1) 
				memcpy(row1, row2, outwidth*4); 
			else 
				GL_ResampleTextureLerpLine (inrow, row1, inwidth, outwidth);

			if (yi < endy) 
				GL_ResampleTextureLerpLine (inrow + inwidth*4, row2, inwidth, outwidth); 
			else 
				memcpy(row2, row1, outwidth*4); 
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
				*out++ = (byte) ((*row1++ * l1 + *row2++ * l2) >> 16); 
			} 
			row1 -= outwidth*4; 
			row2 -= outwidth*4; 
		} 
		else // last line has no pixels to lerp to 
		{ 
			for (j = 0;j < outwidth;j++) 
			{ 
				*out++ = *row1++; 
				*out++ = *row1++; 
				*out++ = *row1++; 
				*out++ = *row1++; 
			} 
			row1 -= outwidth*4; 
		} 
	} 
	free(row1); 
	free(row2); 
} 
/*
void GL_ResampleTexture (unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight)
{
	int		i, j;
	unsigned	*inrow, *inrow2;
	unsigned	frac, fracstep;
	unsigned	p1[1024], p2[1024];
	byte		*pix1, *pix2, *pix3, *pix4;

	fracstep = inwidth*0x10000/outwidth;

	frac = fracstep>>2;
	for (i=0 ; i<outwidth ; i++)
	{
		p1[i] = 4*(frac>>16);
		frac += fracstep;
	}
	frac = 3*(fracstep>>2);
	for (i=0 ; i<outwidth ; i++)
	{
		p2[i] = 4*(frac>>16);
		frac += fracstep;
	}

	for (i=0 ; i<outheight ; i++, out += outwidth)
	{
		inrow = in + inwidth*(int)((i+0.25)*inheight/outheight);
		inrow2 = in + inwidth*(int)((i+0.75)*inheight/outheight);
		frac = fracstep >> 1;
		for (j=0 ; j<outwidth ; j++)
		{
			pix1 = (byte *)inrow + p1[j];
			pix2 = (byte *)inrow + p2[j];
			pix3 = (byte *)inrow2 + p1[j];
			pix4 = (byte *)inrow2 + p2[j];
			((byte *)(out+j))[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			((byte *)(out+j))[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			((byte *)(out+j))[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
			((byte *)(out+j))[3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3])>>2;
		}
	}
}*/


/*
================
GL_LightScaleTexture

Scale up the pixel values in a texture to increase the
lighting range
================
*/
void GL_LightScaleTexture (unsigned *in, int inwidth, int inheight, qboolean only_gamma )
{
	if ( only_gamma )
	{
		int		i, c;
		byte	*p;

		p = (byte *)in;

		c = inwidth*inheight;
		for (i=0 ; i<c ; i++, p+=4)
		{
			p[0] = gammatable[p[0]];
			p[1] = gammatable[p[1]];
			p[2] = gammatable[p[2]];
		}
	}
	else
	{
		int		i, c;
		byte	*p;

		p = (byte *)in;

		c = inwidth*inheight;
		for (i=0 ; i<c ; i++, p+=4)
		{
			p[0] = gammatable[intensitytable[p[0]]];
			p[1] = gammatable[intensitytable[p[1]]];
			p[2] = gammatable[intensitytable[p[2]]];
		}
	}
}

/*
================
GL_MipMap

Operates in place, quartering the size of the texture
================
*/
void GL_MipMap (byte *in, int width, int height)
{
	int		i, j;
	byte	*out;

	width <<=2;
	height >>= 1;
	out = in;
	for (i=0 ; i<height ; i++, in+=width)
	{
		for (j=0; j<width; j+=8, out+=4, in+=8)
		{
			out[0] = (in[0] + in[4] + in[width+0] + in[width+4])>>2;
			out[1] = (in[1] + in[5] + in[width+1] + in[width+5])>>2;
			out[2] = (in[2] + in[6] + in[width+2] + in[width+6])>>2;
			out[3] = (in[3] + in[7] + in[width+3] + in[width+7])>>2;
		}
	}
}


/*
================
GL_BuildPalettedTexture
================
*/
void GL_BuildPalettedTexture (unsigned char *paletted_texture, unsigned char *scaled, int scaled_width, int scaled_height)
{
	int i;

	for ( i = 0; i < scaled_width * scaled_height; i++ )
	{
		unsigned int r, g, b, c;

		r = ( scaled[0] >> 3 ) & 31;
		g = ( scaled[1] >> 2 ) & 63;
		b = ( scaled[2] >> 3 ) & 31;

		c = r | ( g << 5 ) | ( b << 11 );

		paletted_texture[i] = glState.d_16to8table[c];

		scaled += 4;
	}
}


/*
===============
here starts modified code
by Heffo/changes by Nexus
===============
*/

static	int		upload_width, upload_height; //** DMP made local to module
static	qboolean uploaded_paletted;			//** DMP ditto

/*
static qboolean IsPowerOf2 (int value)
{
	int i = 1;
	while (1) {
		if (value == i)
			return true;
		if (i > value)
			return false;
		i <<= 1;
	}
}
*/

int nearest_power_of_2 (int size)
{
	int i = 2;

	// NeVo - infinite loop bug-fix
//	if (size == 1)
	if (size < 2)	// FS: Better fix
		return size; 

	while (1) 
	{
		i <<= 1;
		if (size == i)
			return i;
		if (size > i && size < (i <<1)) 
		{
			if (size >= ((i+(i<<1))/2))
				return i<<1;
			else
				return i;
		}
	};
}

/*
===============
GL_Upload32

Returns has_alpha
===============
*/
//#define USE_GLMIPMAP
qboolean GL_Upload32 (unsigned *data, int width, int height, imagetype_t type)
{
	int			samples;
	unsigned 	*scaled;
	int			scaled_width, scaled_height;
	int			i, c, comp;
	qboolean	mipmap;
	byte		*scan;

	mipmap = ((type != it_pic) && (type != it_sky));
	uploaded_paletted = false;

	//
	// scan the texture for any non-255 alpha
	//
	c = width*height;
	scan = ((byte *)data) + 3;
	samples = gl_solid_format;
	for (i=0 ; i<c ; i++, scan += 4)
	{
		if ( *scan != 255 )
		{
			samples = gl_alpha_format;
			break;
		}
	}

	// Heffo - ARB Texture Compression
	qglHint(GL_TEXTURE_COMPRESSION_HINT_ARB, GL_NICEST);
	if (samples == gl_solid_format)
		comp = (glState.texture_compression && (type != it_pic)) ? GL_COMPRESSED_RGB_ARB : gl_tex_solid_format;
	else if (samples == gl_alpha_format)
		comp = (glState.texture_compression && (type != it_pic)) ? GL_COMPRESSED_RGBA_ARB : gl_tex_alpha_format;

	//
	// find sizes to scale to
	//
	if ( glConfig.arbTextureNonPowerOfTwo && (!mipmap || r_nonpoweroftwo_mipmaps->integer) ) {
		scaled_width = width;
		scaled_height = height;
	}
	else {
		scaled_width = nearest_power_of_2 (width);
		scaled_height = nearest_power_of_2 (height);
	}

	if (scaled_width > glConfig.max_texsize)
		scaled_width = glConfig.max_texsize;
	if (scaled_height > glConfig.max_texsize)
		scaled_height = glConfig.max_texsize;

	//
	// allow sampling down of the world textures for speed
	//
	if (mipmap && r_picmip->integer > 0)
	{
		int maxsize;

		if (r_picmip->integer == 1)		// clamp to 512x512
			maxsize = 512;
		else if (r_picmip->integer == 2) // clamp to 256x256
			maxsize = 256;
		else								// clamp to 128x128
			maxsize = 128;

		while (1) {
			if (scaled_width <= maxsize && scaled_height <= maxsize)
				break;
			scaled_width >>= 1;
			scaled_height >>= 1;
		}
	//	scaled_width >>= r_picmip->integer;
	//	scaled_height >>= r_picmip->integer;
	}

	//
	// resample texture if needed
	//
	if (scaled_width != width || scaled_height != height) 
	{
		scaled = malloc((scaled_width * scaled_height) * 4);
		GL_ResampleTexture (data, width, height, scaled, scaled_width, scaled_height);
	}
	else
	{
		scaled_width = width;
		scaled_height = height;
		scaled = data;
	}

	if (!glState.gammaRamp)
		GL_LightScaleTexture (scaled, scaled_width, scaled_height, !mipmap );

	//
	// generate mipmaps and upload
	//
#ifdef USE_GLMIPMAP
	qglTexImage2D (GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
	if (mipmap)
	{
		int		mip_width, mip_height, miplevel = 0;

		mip_width = scaled_width;	mip_height = scaled_height;
		while (mip_width > 1 || mip_height > 1)
		{
			GL_MipMap ((byte *)scaled, mip_width, mip_height);
			mip_width = max(mip_width>>1, 1);
			mip_height = max(mip_height>>1, 1);
			miplevel++;
			qglTexImage2D (GL_TEXTURE_2D, miplevel, comp, mip_width, mip_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
		}
	}
#else
	if (mipmap)
	{
		if (glState.sgis_mipmap) {
			qglTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, true);
			qglTexImage2D (GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
		} 
		else
			gluBuild2DMipmaps (GL_TEXTURE_2D, comp, scaled_width, scaled_height, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
	} 
	else
		qglTexImage2D (GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
#endif

	if (scaled_width != width || scaled_height != height)
		free(scaled);

	upload_width = scaled_width;	upload_height = scaled_height;

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (mipmap) ? gl_filter_min : gl_filter_max);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	// Set anisotropic filter if supported and enabled
	if (mipmap && glConfig.anisotropic && r_anisotropic->value)
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, r_anisotropic->value);

	return (samples == gl_alpha_format || samples == GL_COMPRESSED_RGBA_ARB);
}

/*
===============
GL_Upload8

Returns has_alpha
===============
*/
qboolean GL_Upload8 (byte *data, int width, int height, imagetype_t type)
{
//	unsigned	trans[512*256];
	unsigned	*trans = NULL;	// Knightmare- changed to dynamic allocation
	int			i, s;
	int			p;
	qboolean	has_alpha;	// Knightmare added

	s = width*height;

//	if (s > sizeof(trans)/4)
//		VID_Error (ERR_DROP, "GL_Upload8: too large");

	trans = (unsigned *)malloc(s*4);	// Knightmare- changed to dynamic allocation

	for (i=0 ; i<s ; i++)
	{
		p = data[i];
		trans[i] = d_8to24table[p];

		if (p == 255)
		{	// transparent, so scan around for another color
			// to avoid alpha fringes
			// FIXME: do a full flood fill so mips work...
			if (i > width && data[i-width] != 255)
				p = data[i-width];
			else if (i < s-width && data[i+width] != 255)
				p = data[i+width];
			else if (i > 0 && data[i-1] != 255)
				p = data[i-1];
			else if (i < s-1 && data[i+1] != 255)
				p = data[i+1];
			else
				p = 0;
			// copy rgb components
			((byte *)&trans[i])[0] = ((byte *)&d_8to24table[p])[0];
			((byte *)&trans[i])[1] = ((byte *)&d_8to24table[p])[1];
			((byte *)&trans[i])[2] = ((byte *)&d_8to24table[p])[2];
		}
	}

//	return GL_Upload32 (trans, width, height, type);

	// Knightmare- changed to dynamic allocation
	has_alpha = GL_Upload32 (trans, width, height, type);

	free (trans);
	trans = NULL;

	return has_alpha;
	// end Knightmare
}


/*
================
R_LoadPic

This is also used as an entry point for the generated notexture
Nexus  - changes for hires-textures
================
*/
image_t *R_LoadPic (char *name, byte *pic, int width, int height, imagetype_t type, int bits)
{
	image_t		*image;
	int			i;
	//Nexus'added vars
	int len; 
	char s[128]; 

	// find a free image_t
	for (i=0, image=gltextures ; i<numgltextures ; i++,image++)
	{
		if (!image->texnum)
			break;
	}
	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			VID_Error (ERR_DROP, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];

	if (strlen(name) >= sizeof(image->name))
		VID_Error (ERR_DROP, "Draw_LoadPic: \"%s\" is too long", name);
//	strncpy (image->name, name);
	Q_strncpyz (image->name, name, sizeof(image->name));
	image->hash = Com_HashFileName(name, 0, false);	// Knightmare added
	image->registration_sequence = registration_sequence;

	image->width = width;
	image->height = height;
	image->type = type;
	image->replace_scale_w = image->replace_scale_h = 1.0f; // Knightmare added

	if (type == it_skin && bits == 8)
		R_FloodFillSkin(pic, width, height);

// replacement scaling hack for TGA/JPEG HUD images and skins
// NOTE: replace this with shaders as soon as they are supported
	len = strlen(name); 
//	strncpy(s,name);
	Q_strncpyz(s, name, sizeof(s));
	// check if we have a tga/jpg pic
#ifdef PNG_SUPPORT
	if ((type == it_pic) && (!strcmp(s+len-4, ".tga") || !strcmp(s+len-4, ".png") || !strcmp(s+len-4, ".jpg")) )
#else	// PNG_SUPPORT
	if ((type == it_pic) && (!strcmp(s+len-4, ".tga") || !strcmp(s+len-4, ".jpg")) )
#endif	// PNG_SUPPORT
	{ 
		byte	*pic, *palette;
		int		pcxwidth, pcxheight;
		s[len-3]='p'; s[len-2]='c'; s[len-1]='x'; // replace extension 
		LoadPCX (s, &pic, &palette, &pcxwidth, &pcxheight); // load .pcx file 
		
		if (pic && (pcxwidth > 0) && (pcxheight > 0)) {
			image->replace_scale_w = (float)pcxwidth/image->width;
			image->replace_scale_h = (float)pcxheight/image->height;
		}
		if (pic) free(pic);
		if (palette) free(palette);
	}	

	// load little pics into the scrap
	if (image->type == it_pic && bits == 8
		&& image->width < 64 && image->height < 64)
	{
		int		x, y;
		int		i, j, k;
		int		texnum;

		texnum = Scrap_AllocBlock (image->width, image->height, &x, &y);
		if (texnum == -1)
			goto nonscrap;
		scrap_dirty = true;

		// copy the texels into the scrap block
		k = 0;
		for (i=0 ; i<image->height ; i++)
			for (j=0 ; j<image->width ; j++, k++)
				scrap_texels[texnum][(y+i)*BLOCK_WIDTH + x + j] = pic[k];
		image->texnum = TEXNUM_SCRAPS + texnum;
		image->scrap = true;
		image->has_alpha = true;
		image->sl = (x+0.01)/(float)BLOCK_WIDTH;
		image->sh = (x+image->width-0.01)/(float)BLOCK_WIDTH;
		image->tl = (y+0.01)/(float)BLOCK_WIDTH;
		image->th = (y+image->height-0.01)/(float)BLOCK_WIDTH;
	}
	else
	{
nonscrap:
		image->scrap = false;
		image->texnum = TEXNUM_IMAGES + (image - gltextures);
		GL_Bind(image->texnum);
		if (bits == 8)
		//	image->has_alpha = GL_Upload8 (pic, width, height, (image->type != it_pic && image->type != it_sky), image->type == it_sky );
			image->has_alpha = GL_Upload8 (pic, width, height, type);
		else
		//	image->has_alpha = GL_Upload32 ((unsigned *)pic, width, height, (image->type != it_pic && image->type != it_sky) );
			image->has_alpha = GL_Upload32 ((unsigned *)pic, width, height, type);
		image->upload_width = upload_width;		// after power of 2 and scales
		image->upload_height = upload_height;
		image->paletted = uploaded_paletted;
		image->sl = 0;
		image->sh = 1;
		image->tl = 0;
		image->th = 1;
	}

	return image;
}


// store the names of last images that failed to load
#define NUM_FAIL_IMAGES 1024
char lastFailedImage[NUM_FAIL_IMAGES][MAX_OSPATH];
long lastFailedImageHash[NUM_FAIL_IMAGES];
static unsigned failedImgListIndex;

/*
===============
R_InitFailedImgList
===============
*/
void R_InitFailedImgList (void)
{
	int		i;

	for (i=0; i<NUM_FAIL_IMAGES; i++) {
		Com_sprintf(lastFailedImage[i], sizeof(lastFailedImage[i]), "\0");
		lastFailedImageHash[i] = 0;
	}

	failedImgListIndex = 0;
}

/*
===============
R_CheckImgFailed
===============
*/
qboolean R_CheckImgFailed (char *name)
{
	int		i;
	long	hash;

	hash = Com_HashFileName(name, 0, false);
	for (i=0; i<NUM_FAIL_IMAGES; i++)
	{
		if (hash == lastFailedImageHash[i]) {	// compare hash first
			if (lastFailedImage[i] && strlen(lastFailedImage[i])
				&& !strcmp(name, lastFailedImage[i]))
			{	// we already tried to load this image, didn't find it
				//VID_Printf (PRINT_ALL, "R_CheckImgFailed: found %s on failed to load list\n", name);
				return true;
			}
		}
	}

	return false;
}

/*
===============
R_AddToFailedImgList
===============
*/
void R_AddToFailedImgList (char *name)
{
	if (!strncmp(name, "save/", 5)) // don't add saveshots
		return;

	//VID_Printf (PRINT_ALL, "R_AddToFailedImgList: adding %s to failed to load list\n", name);

	Com_sprintf(lastFailedImage[failedImgListIndex], sizeof(lastFailedImage[failedImgListIndex]), "%s", name);
	lastFailedImageHash[failedImgListIndex] = Com_HashFileName(name, 0, false);
	failedImgListIndex++;

	// wrap around to start of list
	if (failedImgListIndex >= NUM_FAIL_IMAGES)
		failedImgListIndex = 0;
}

/*
================
R_LoadWal
================
*/
image_t *R_LoadWal (char *name, imagetype_t type)
{
	miptex_t	*mt;
	int			width, height, ofs;
	image_t		*image;

	FS_LoadFile (name, (void **)&mt);
	if (!mt)
	{
		if (type == it_wall)
			VID_Printf (PRINT_ALL, "R_FindImage: can't load %s\n", name);
		//return glMedia.notexture;
		return NULL;
	}

	width = LittleLong (mt->width);
	height = LittleLong (mt->height);
	ofs = LittleLong (mt->offsets[0]);

	image = R_LoadPic (name, (byte *)mt + ofs, width, height, it_wall, 8);

	FS_FreeFile ((void *)mt);

	return image;
}


/*
===============
R_FindImage

Finds or loads the given image
===============
*/
image_t	*R_FindImage (char *name, imagetype_t type)
{
	image_t	*image;
	int		i, len;
	byte	*pic, *palette;
	int		width, height;
	char	s[128];
	char	*tmp;

	if (!name)
		return NULL;
	len = strlen(name);
	if (len<5)
		return NULL;

	// fix up bad image paths
    tmp = name;
    while ( *tmp != 0 )
    {
        if ( *tmp == '\\' )
            *tmp = '/';
        tmp++;
    }

	// look for it
	for (i=0, image=gltextures; i<numgltextures; i++,image++)
	{
		if (!strcmp(name, image->name))
		{
			image->registration_sequence = registration_sequence;
			return image;
		}
	}

	// don't try again to load an image that just failed
	if (R_CheckImgFailed (name))
	{
		if (!strcmp(name+len-4, ".tga"))
		{
#ifdef PNG_SUPPORT
			// fall back to png
			Q_strncpyz (s, name, sizeof(s));
			s[len-3]='p'; s[len-2]='n'; s[len-1]='g';
			return R_FindImage(s,type);	
#else	// PNG_SUPPORT
			// fall back to jpg
			Q_strncpyz(s,name, sizeof(s));
			s[len-3]='j'; s[len-2]='p'; s[len-1]='g';
			return R_FindImage(s,type);
#endif	// PNG_SUPPORT
		}
#ifdef PNG_SUPPORT
		else if (!strcmp(name+len-4, ".png"))
		{	// fall back to jpg
			Q_strncpyz (s, name, sizeof(s));
			s[len-3]='j'; s[len-2]='p'; s[len-1]='g';
			return R_FindImage(s,type);
		}
#endif	// PNG_SUPPORT
		else
			return NULL;
	}

	// MrG's automatic JPG & TGA loading
	// search for TGAs or JPGs to replace .pcx and .wal images
	if (!strcmp(name+len-4, ".pcx") || !strcmp(name+len-4, ".wal"))
	{
	//	strncpy(s, name);
		Q_strncpyz (s, name, sizeof(s));
		s[len-3]='t'; s[len-2]='g'; s[len-1]='a';
		image = R_FindImage(s,type);
		if (image)
			return image;
	}

	//
	// load the pic from disk
	//
	pic = NULL;
	palette = NULL;

	if (!strcmp(name+len-4, ".pcx"))
	{
		LoadPCX (name, &pic, &palette, &width, &height);
		if (pic)
			image = R_LoadPic (name, pic, width, height, type, 8);
		else
			image = NULL;
	}
	else if (!strcmp(name+len-4, ".wal"))
	{
		image = R_LoadWal (name, type);
	}
	else if (!strcmp(name+len-4, ".tga"))
	{
		R_LoadTGA (name, &pic, &width, &height);
		if (pic)
			image = R_LoadPic (name, pic, width, height, type, 32);
#ifdef PNG_SUPPORT
		else { // fall back to png
			R_AddToFailedImgList(name);
			Q_strncpyz (s, name, sizeof(s));
			s[len-3]='p'; s[len-2]='n'; s[len-1]='g';
			return R_FindImage(s,type);
		}
#else	// PNG_SUPPORT
		else { // fall back to jpg
			R_AddToFailedImgList(name);
			Q_strncpyz(s,name, sizeof(s));
			s[len-3]='j'; s[len-2]='p'; s[len-1]='g';
			return R_FindImage(s,type);
		}
#endif	// PNG_SUPPORT
	}
#ifdef PNG_SUPPORT
	else if (!strcmp(name+len-4, ".png"))
	{
		R_LoadPNG (name, &pic, &width, &height);
		if (pic)
			image = R_LoadPic(name, pic, width, height, type, 32);
		else { // fall back to jpg
			R_AddToFailedImgList(name);
			Q_strncpyz (s, name, sizeof(s));
			s[len-3]='j'; s[len-2]='p'; s[len-1]='g';
			return R_FindImage(s,type);
		}
	}
#endif	// PNG_SUPPORT
	else if (!strcmp(name+len-4, ".jpg")) // Heffo - JPEG support
	{
		R_LoadJPG(name, &pic, &width, &height);
		if (pic)
			image = R_LoadPic(name, pic, width, height, type, 32);
		else
			image = NULL;
	}
	/*else if (!strcmp(name+len-4, ".cin")) // Heffo
	{										// WHY .cin files? because we can!
		cinematics_t *newcin;

		newcin = CIN_OpenCin(name);
		if(!newcin)
			return NULL;

		pic = malloc(256*256*4);
		memset(pic, 192, (256*256*4));

		image = R_LoadPic (name, pic, 256, 256, type, 32);

		newcin->texnum = image->texnum;
		image->is_cin = true;
	}*/
	else
		image = NULL;

	if (!image)
		R_AddToFailedImgList(name);

	if (pic)
		free(pic);
	if (palette)
		free(palette);

	return image;
}



/*
===============
R_RegisterSkin
===============
*/
struct image_s *R_RegisterSkin (char *name)
{
	return R_FindImage (name, it_skin);
}


/*
================
R_FreeUnusedImages

Any image that was not touched on this registration sequence
will be freed.
================
*/
void R_FreeUnusedImages (void)
{
	int		i;
	image_t	*image;

	// never free notexture or particle textures
	glMedia.notexture->registration_sequence = registration_sequence;
	glMedia.whitetexture->registration_sequence = registration_sequence;
	glMedia.distTextureARB->registration_sequence = registration_sequence;
#ifdef ROQ_SUPPORT
	glMedia.rawtexture->registration_sequence = registration_sequence;
#endif // ROQ_SUPPORT
	glMedia.envmappic->registration_sequence = registration_sequence;
	glMedia.spheremappic->registration_sequence = registration_sequence;
	glMedia.shelltexture->registration_sequence = registration_sequence;
	glMedia.celshadetexture->registration_sequence = registration_sequence;
	glMedia.causticwaterpic->registration_sequence = registration_sequence;
	glMedia.causticslimepic->registration_sequence = registration_sequence;
	glMedia.causticlavapic->registration_sequence = registration_sequence;
	glMedia.particlebeam->registration_sequence = registration_sequence;

	for (i=0; i<PARTICLE_TYPES; i++)
		if (glMedia.particletextures[i]) // dont mess with null ones silly :p
			glMedia.particletextures[i]->registration_sequence = registration_sequence;

	for (i=0, image=gltextures; i<numgltextures; i++, image++)
	{
		if (image->registration_sequence == registration_sequence)
			continue;		// used this sequence
		if (!image->registration_sequence)
			continue;		// free image_t slot
		if (image->type == it_pic)
			continue;		// don't free pics

		//Heffo - Free Cinematic
		//if(image->is_cin)
		//	CIN_FreeCin(image->texnum);

		// free it
		qglDeleteTextures (1, &image->texnum);
		memset (image, 0, sizeof(*image));
	}
}


/*
===============
Draw_GetPalette
===============
*/
int Draw_GetPalette (void)
{
	int		i;
	int		r, g, b;
	unsigned	v;
	byte	*pic, *pal;
	int		width, height;
	//int		avg, dr, dg, db, d1, d2, d3;	//** DMP
	//float	sat;							//** DMP

	// get the palette

	LoadPCX ("pics/colormap.pcx", &pic, &pal, &width, &height);
	if (!pal)
		VID_Error (ERR_FATAL, "Couldn't load pics/colormap.pcx");

	for (i=0 ; i<256 ; i++)
	{
		r = pal[i*3+0];
		g = pal[i*3+1];
		b = pal[i*3+2];
		
		//** DMP adjust saturation
		/*avg = (r + g + b + 2) / 3;			// first calc grey value we'll desaturate to
		dr = avg - r;						// calc distance from grey value to each gun
		dg = avg - g;
		db = avg - b;
		d1 = abs(r - g);					// find greatest distance between all the guns
		d2 = abs(g - b);
		d3 = abs(b - r);
		if (d1 > d2)						// we will use this for our existing saturation
			if (d1 > d3)
				sat = d1;
			else
				if (d2 > d3)
					sat = d2;
				else
					sat = d3;
		else
			if (d2 > d3)
				sat = d2;
			else
				sat = d3;
		sat /= 255.0;						// convert existing saturationn to ratio
		sat = 1.0 - sat;					// invert so the most saturation causes the desaturation to lessen
		sat *= (1.0 - r_saturation->value); // scale the desaturation value so our desaturation is non-linear (keeps lava looking good)
		dr *= sat;							// scale the differences by the amount we want to desaturate
		dg *= sat;
		db *= sat;
		r += dr;							// now move the gun values towards total grey by the amount we desaturated
		g += dg;
		b += db;*/
		//** DMP end saturation mod

		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		d_8to24table[i] = LittleLong(v);
	}

	d_8to24table[255] &= LittleLong(0xffffff);	// 255 is transparent

	free (pic);
	free (pal);

	return 0;
}


/*
===============
R_InitImages
===============
*/
void R_InitImages (void)
{
	int		i, j;
	float	g = vid_gamma->value;

	// Knightmare- reinitialize these after a vid_restart
	// this is needed because the renderer is no longer a DLL
	gl_solid_format = 3;
	gl_alpha_format = 4;
	gl_tex_solid_format = 3;
	gl_tex_alpha_format = 4;
	gl_filter_min = GL_LINEAR_MIPMAP_NEAREST;
	gl_filter_max = GL_LINEAR;

	registration_sequence = 1;

	// init intensity conversions
	//r_intensity = Cvar_Get ("r_intensity", "2", CVAR_ARCHIVE);

	// Knightmare- added Vic's RGB brightening
	if (glConfig.mtexcombine)
		r_intensity = Cvar_Get ("r_intensity", "1", 0);
	else
		r_intensity = Cvar_Get ("r_intensity", "2", 0);
	// end Knightmare

	if ( r_intensity->value <= 1 )
		Cvar_Set( "r_intensity", "1" );

	glState.inverse_intensity = 1 / r_intensity->value;

	R_InitFailedImgList (); // Knightmare added

	Draw_GetPalette ();

	if (qglColorTableEXT)
	{
		FS_LoadFile( "pics/16to8.dat", &glState.d_16to8table );
		if ( !glState.d_16to8table )
			VID_Error( ERR_FATAL, "Couldn't load pics/16to8.pcx");
	}

	if (glConfig.rendType == GLREND_VOODOO)
	//if ( glConfig.renderer & ( GL_RENDERER_VOODOO | GL_RENDERER_VOODOO2 ) )
	{
		g = 1.0F;
	}

	for ( i = 0; i < 256; i++ )
	{
		if ( g == 1 )
		{
			gammatable[i] = i;
		}
		else
		{
			float inf;

			inf = 255 * pow ( (i+0.5)/255.5 , g ) + 0.5;
			if (inf < 0)
				inf = 0;
			if (inf > 255)
				inf = 255;
			gammatable[i] = inf;
		}
	}

	for (i=0 ; i<256 ; i++)
	{
		j = i*r_intensity->value;
		if (j > 255)
			j = 255;
		intensitytable[i] = j;
	}

	R_InitBloomTextures (); // BLOOMS
}

/*
===============
R_FreePic
by Knightmare
Frees a single pic
===============
*/
void R_FreePic (char *name)
{
	int		i;
	image_t	*image;

	for (i=0, image=gltextures; i<numgltextures; i++, image++)
	{
		if (!image->registration_sequence)
			continue;		// free image_t slot
		if (image->type != it_pic)
			continue;		// only free pics
		if (!strcmp(name, image->name))
		{
			//Heffo - Free Cinematic
			//if (image->is_cin)
			//	CIN_FreeCin(image->texnum);

			// free it
			qglDeleteTextures (1, &image->texnum);
			memset (image, 0, sizeof(*image));
			return; //we're done here
		}
	}
}

/*
===============
R_ShutdownImages
===============
*/
void R_ShutdownImages (void)
{
	int		i;
	image_t	*image;

	for (i=0, image=gltextures; i<numgltextures; i++, image++)
	{
		if (!image->registration_sequence)
			continue;		// free image_t slot

		//Heffo - Free Cinematic
		//if (image->is_cin)
		//	CIN_FreeCin(image->texnum);

		// free it
		qglDeleteTextures (1, &image->texnum);
		memset (image, 0, sizeof(*image));
	}
}

