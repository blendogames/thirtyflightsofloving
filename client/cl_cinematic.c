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


#include "client.h"

#define RoQ_INFO				0x1001
#define RoQ_QUAD_CODEBOOK		0x1002
#define RoQ_QUAD_VQ				0x1011
#define RoQ_SOUND_MONO			0x1020
#define RoQ_SOUND_STEREO		0x1021

#define RoQ_ID_MOT				0x0000
#define RoQ_ID_FCC				0x0001
#define RoQ_ID_SLD				0x0002
#define RoQ_ID_CCC				0x0003

#ifdef	ROQ_SUPPORT

typedef struct {
	unsigned short	id;
	unsigned int	size;
	unsigned short	argument;
} roqChunk_t;

typedef struct {
	byte			y[4];
	byte			u;
	byte			v;
} roqCell_t;

typedef struct {
	byte			idx[4];
} roqQCell_t;

typedef struct {
	char			name[MAX_QPATH];
	qboolean		playing;
	fileHandle_t	file;
	int				size;
	int				start;
	int				remaining;
	qboolean		isRoQ;
	int				rate;
	
	int				x;
	int				y;
	int				w;
	int				h;
	int				flags;

	int				sndRate;
	int				sndWidth;
	int				sndChannels;

	int				vidWidth;
	int				vidHeight;
	byte			*vidBuffer;

	int				rawWidth;
	int				rawHeight;
	byte			*rawBuffer;

	int				frameCount;
	int				frameTime;

	unsigned		palette[256];

	// PCX stuff
	byte			*pcxBuffer;

	// Order 1 Huffman stuff
	int				*hNodes1;
	int				hNumNodes1[256];

	int				hUsed[512];
	int				hCount[512];

	byte			*hBuffer;

	// RoQ stuff
	roqChunk_t		roqChunk;
	roqCell_t		roqCells[256];
	roqQCell_t		roqQCells[256];

	short			roqSndSqrTable[256];

	byte			*roqBuffer;
	byte			*roqBufferPtr[2];
} cinematic_t;

static cinematic_t	cinematics[MAX_CINEMATICS];


typedef struct
{
	int         rendType;
	const char	*renderer_string;
	const char	*vendor_string;
	const char	*version_string;
	const char	*extensions_string;

	// for parsing newer OpenGL versions
	int			version_major;
	int			version_minor;
	int			version_release;

	qboolean	allowCDS;
	// max texture size
	int			max_texsize;
	int			max_texunits;

	// non-power of two texture support
	qboolean	arbTextureNonPowerOfTwo;

	qboolean	vertexBufferObject;
	qboolean	multitexture;
	qboolean	mtexcombine;	// added Vic's RGB brightening

	qboolean	have_stencil;
	qboolean	extStencilWrap;
	qboolean	atiSeparateStencil;
	qboolean	extStencilTwoSide;

	qboolean	extCompiledVertArray;
	qboolean	drawRangeElements;

	// texture shader support
	qboolean	arb_fragment_program;
	qboolean	arb_vertex_program;
	qboolean	NV_texshaders;

	// anisotropic filtering
	qboolean	anisotropic;
	float		max_anisotropy;

	qboolean	newTexFormat;			// whether to use GL_RGBA textures / GL_BGRA lightmaps
} glconfig_t;
extern	glconfig_t glConfig;

#if 0
/*
=================
Q_strncat

Never goes past bounds or leaves without a terminating 0
=================
*/
void Q_strncat (char *dst, size_t dstSize, const char *src)
{
	int	len;

	len = (int)strlen(dst);
	if (len >= dstSize)
		Com_Error(ERR_FATAL, "Q_strncat: already overflowed");

	Q_strncpyz(dst + len, dstSize - len, src);
}
#endif

/*
=================
Com_FileExtension

Returns the extension, if any (does not include the .)
=================
*/
void Com_FileExtension (const char *path, char *dst, int dstSize)
{
	const char	*s, *last;

	s = last = path + strlen(path);
	while (*s != '/' && *s != '\\' && s != path){
		if (*s == '.'){
			last = s+1;
			break;
		}

		s--;
	}

	Q_strncpyz (dst, dstSize, last);
}

/*
=================
Com_DefaultExtension

If path doesn't have a .EXT, append newExtension (newExtension should
include the .)
=================
*/
void Com_DefaultExtension (char *path, size_t maxSize, const char *newExtension)
{
	char	*s;

	s = path + strlen(path);
	while (*s != '/' && *s != '\\' && s != path){
		if (*s == '.')
			return;			// It has an extension

		s--;
	}

	Q_strncatz (path, maxSize, newExtension);
}

/*
=================
Com_DefaultPath

If path doesn't have a / or \\, append newPath (newPath should not
include the /)
=================
*/
void Com_DefaultPath (char *path, int maxSize, const char *newPath)
{
	char    oldPath[MAX_OSPATH], *s;
		
	s = path;
	while (*s){
		if (*s == '/' || *s == '\\')
			return;		// It has a path
		
		s++;
	}

	Q_strncpyz (oldPath, sizeof(oldPath), path);
	Com_sprintf (path, maxSize, "%s/%s", newPath, oldPath);
}

//=============================================================

/*
=================
CIN_Skip
=================
*/
static void CIN_Skip (cinematic_t *cin, int count)
{
	FS_Seek(cin->file, count, FS_SEEK_CUR);
	cin->remaining -= count;
}


/*
=================
CIN_SoundSqrTableInit
=================
*/
static void CIN_SoundSqrTableInit (cinematic_t *cin)
{
	int		i;

	for (i = 0; i < 128; i++){
		cin->roqSndSqrTable[i] = i * i;
		cin->roqSndSqrTable[i+128] = -(i * i);
	}
}

/*
=================
CIN_ReadChunk
=================
*/
static void CIN_ReadChunk (cinematic_t *cin)
{
	roqChunk_t	*chunk = &cin->roqChunk;

	FS_Read(&chunk->id, sizeof(chunk->id), cin->file);
	FS_Read(&chunk->size, sizeof(chunk->size), cin->file);
	FS_Read(&chunk->argument, sizeof(chunk->argument), cin->file);

	chunk->id = LittleShort(chunk->id);
	chunk->size = LittleLong(chunk->size);
	chunk->argument = LittleShort(chunk->argument);

	cin->remaining -= sizeof(roqChunk_t);

	//if (cin->frameCount < 20)
	//	Com_Printf("read ROQ chunk of size %u, id %u, argument %u, %u remaining\n", chunk->size, chunk->id, chunk->argument, cin->remaining);
}

#define Clamp(a,b,c)		(((a) < (b)) ? (b) : ((a) > (c)) ? (c) : (a))
/*
=================
CIN_ReadInfo
=================
*/
static void CIN_ReadInfo (cinematic_t *cin)
{
	roqChunk_t	*chunk = &cin->roqChunk;
	short		data[4];

	FS_Read(data, sizeof(data), cin->file);
	cin->remaining -= sizeof(data);

	cin->vidWidth = LittleShort(data[0]);
	cin->vidHeight = LittleShort(data[1]);

	if (cin->roqBuffer)
		Z_Free(cin->roqBuffer);

	cin->roqBuffer = Z_Malloc(cin->vidWidth * cin->vidHeight * 4 * 2);

	cin->roqBufferPtr[0] = cin->roqBuffer;
	cin->roqBufferPtr[1] = cin->roqBuffer + cin->vidWidth * cin->vidHeight * 4;

	cin->rawWidth = Clamp(cin->vidWidth, 1, glConfig.max_texsize); // was 512
	cin->rawHeight = Clamp(cin->vidHeight, 1, glConfig.max_texsize); // was 512

	if (cin->rawWidth != cin->vidWidth || cin->rawHeight != cin->vidHeight)
	{
		if (cin->rawBuffer)
			Z_Free(cin->rawBuffer);

		cin->rawBuffer = Z_Malloc(cin->rawWidth * cin->rawHeight * 4);
	}
}

/*
=================
CIN_ReadCodebook
=================
*/
static void CIN_ReadCodebook (cinematic_t *cin)
{
	roqChunk_t	*chunk = &cin->roqChunk;
	int			nv1, nv2;

	nv1 = (chunk->argument >> 8) & 0xff;
	if (!nv1)
		nv1 = 256;

	nv2 = chunk->argument & 0xff;
	if (!nv2 && (nv1 * 6 < chunk->size))
		nv2 = 256;

	FS_Read(cin->roqCells, sizeof(roqCell_t) * nv1, cin->file);
	FS_Read(cin->roqQCells, sizeof(roqQCell_t) * nv2, cin->file);
	cin->remaining -= chunk->size;
}

/*
=================
CIN_DecodeBlock
=================
*/
static void CIN_DecodeBlock (byte *dst0, byte *dst1, const byte *src0, const byte *src1, float u, float v)
{
	int		rgb[3];

	// Convert YCbCr to RGB
	rgb[0] = 1.402 * v;
	rgb[1] = -0.34414 * u - 0.71414 * v;
	rgb[2] = 1.772 * u;

	// 1st pixel
	dst0[0] = Clamp(rgb[0] + src0[0], 0, 255);
	dst0[1] = Clamp(rgb[1] + src0[0], 0, 255);
	dst0[2] = Clamp(rgb[2] + src0[0], 0, 255);
	dst0[3] = 255;

	// 2nd pixel
	dst0[4] = Clamp(rgb[0] + src0[1], 0, 255);
	dst0[5] = Clamp(rgb[1] + src0[1], 0, 255);
	dst0[6] = Clamp(rgb[2] + src0[1], 0, 255);
	dst0[7] = 255;

	// 3rd pixel
	dst1[0] = Clamp(rgb[0] + src1[0], 0, 255);
	dst1[1] = Clamp(rgb[1] + src1[0], 0, 255);
	dst1[2] = Clamp(rgb[2] + src1[0], 0, 255);
	dst1[3] = 255;

	// 4th pixel
	dst1[4] = Clamp(rgb[0] + src1[1], 0, 255);
	dst1[5] = Clamp(rgb[1] + src1[1], 0, 255);
	dst1[6] = Clamp(rgb[2] + src1[1], 0, 255);
	dst1[7] = 255;
}

/*
=================
CIN_ApplyVector2x2
=================
*/
static void CIN_ApplyVector2x2 (cinematic_t *cin, int x, int y, const roqCell_t *cell)
{
	byte	*dst0, *dst1;

	dst0 = cin->roqBufferPtr[0] + (y * cin->vidWidth + x) * 4;
	dst1 = dst0 + cin->vidWidth * 4;

	CIN_DecodeBlock(dst0, dst1, cell->y, cell->y+2, (float)((int)cell->u - 128), (float)((int)cell->v - 128));
}

/*
=================
CIN_ApplyVector4x4
=================
*/
static void CIN_ApplyVector4x4 (cinematic_t *cin, int x, int y, const roqCell_t *cell)
{
	byte	*dst0, *dst1;
	byte	yp[4];
	float	u, v;

	u = (float)((int)cell->u - 128);
	v = (float)((int)cell->v - 128);

	yp[0] = yp[1] = cell->y[0];
	yp[2] = yp[3] = cell->y[1];

	dst0 = cin->roqBufferPtr[0] + (y * cin->vidWidth + x) * 4;
	dst1 = dst0 + cin->vidWidth * 4;

	CIN_DecodeBlock(dst0, dst0+8, yp, yp+2, u, v);
	CIN_DecodeBlock(dst1, dst1+8, yp, yp+2, u, v);

	yp[0] = yp[1] = cell->y[2];
	yp[2] = yp[3] = cell->y[3];

	dst0 += cin->vidWidth * 4 * 2;
	dst1 += cin->vidWidth * 4 * 2;

	CIN_DecodeBlock(dst0, dst0+8, yp, yp+2, u, v);
	CIN_DecodeBlock(dst1, dst1+8, yp, yp+2, u, v);
}

/*
=================
CIN_ApplyMotion4x4
=================
*/
static void CIN_ApplyMotion4x4 (cinematic_t *cin, int x, int y, byte mv, char meanX, char meanY)
{
	byte	*src, *dst;
	int		x1, y1;
	int		i;

	x1 = x + 8 - (mv >> 4) - meanX;
	y1 = y + 8 - (mv & 15) - meanY;

	src = cin->roqBufferPtr[1] + (y1 * cin->vidWidth + x1) * 4;
	dst = cin->roqBufferPtr[0] + (y * cin->vidWidth + x) * 4;

	for (i = 0; i < 4; i++)
	{
		memcpy(dst, src, 4 * 4);

		src += cin->vidWidth * 4;
		dst += cin->vidWidth * 4;
	}
}

/*
=================
CIN_ApplyMotion8x8
=================
*/
static void CIN_ApplyMotion8x8 (cinematic_t *cin, int x, int y, byte mv, char meanX, char meanY)
{
	byte	*src, *dst;
	int		x1, y1;
	int		i;

	x1 = x + 8 - (mv >> 4) - meanX;
	y1 = y + 8 - (mv & 15) - meanY;

	src = cin->roqBufferPtr[1] + (y1 * cin->vidWidth + x1) * 4;
	dst = cin->roqBufferPtr[0] + (y * cin->vidWidth + x) * 4;

	for (i = 0; i < 8; i++)
	{
		memcpy(dst, src, 8 * 4);

		src += cin->vidWidth * 4;
		dst += cin->vidWidth * 4;
	}
}

/*
=================
CIN_SmallestNode1
=================
*/
static int CIN_SmallestNode1 (cinematic_t *cin, int numNodes)
{
	int		i;
	int		best, bestNode;

	best = 99999999;
	bestNode = -1;

	for (i = 0; i < numNodes; i++){
		if (cin->hUsed[i])
			continue;
		if (!cin->hCount[i])
			continue;
		if (cin->hCount[i] < best){
			best = cin->hCount[i];
			bestNode = i;
		}
	}

	if (bestNode == -1)
		return -1;

	cin->hUsed[bestNode] = true;
	return bestNode;
}

/*
=================
CIN_Huff1TableInit

Reads the 64k counts table and initializes the node trees
=================
*/
static void CIN_Huff1TableInit (cinematic_t *cin)
{
	int		prev;
	int		j;
	int		*node, *nodeBase;
	byte	counts[256];
	int		numNodes;

	if (!cin->hNodes1)
		cin->hNodes1 = Z_Malloc(256 * 256 * 4 * 2);

	for (prev = 0; prev < 256; prev++)
	{
		memset(cin->hCount, 0, sizeof(cin->hCount));
		memset(cin->hUsed, 0, sizeof(cin->hUsed));

		// Read a row of counts
		FS_Read(counts, sizeof(counts), cin->file);
		cin->remaining -= sizeof(counts);

		for (j = 0; j < 256; j++)
			cin->hCount[j] = counts[j];

		// Build the nodes
		numNodes = 256;
		nodeBase = cin->hNodes1 + prev*256*2;

		while (numNodes != 511)
		{
			node = nodeBase + (numNodes-256)*2;

			// Pick two lowest counts
			node[0] = CIN_SmallestNode1(cin, numNodes);
			if (node[0] == -1)
				break;

			node[1] = CIN_SmallestNode1(cin, numNodes);
			if (node[1] == -1)
				break;

			cin->hCount[numNodes] = cin->hCount[node[0]] + cin->hCount[node[1]];
			numNodes++;
		}

		cin->hNumNodes1[prev] = numNodes-1;
	}
}

/*
=================
CIN_Huff1Decompress
=================
*/
static void CIN_Huff1Decompress (cinematic_t *cin, const byte *data, int size)
{
	const byte	*input;
	unsigned	*out;
	int			count;
	int			in;
	int			nodeNum;
	int			*nodes, *nodesBase;

	if (!cin->hBuffer)
		cin->hBuffer = Z_Malloc(cin->vidWidth * cin->vidHeight * 4);

	// Get decompressed count
	count = data[0] + (data[1]<<8) + (data[2]<<16) + (data[3]<<24);
	input = data + 4;
	out = (unsigned *)cin->hBuffer;

	// Read bits
	nodesBase = cin->hNodes1 - 256*2;	// Nodes 0-255 aren't stored

	nodes = nodesBase;
	nodeNum = cin->hNumNodes1[0];
	while (count)
	{
		in = *input++;
		
		if (nodeNum < 256)
		{
			nodes = nodesBase + (nodeNum<<9);
			*out++ = cin->palette[nodeNum];
			if (!--count)
				break;
			nodeNum = cin->hNumNodes1[nodeNum];
		}
		nodeNum = nodes[nodeNum*2 + (in&1)];
		in >>= 1;
		
		if (nodeNum < 256)
		{
			nodes = nodesBase + (nodeNum<<9);
			*out++ = cin->palette[nodeNum];
			if (!--count)
				break;
			nodeNum = cin->hNumNodes1[nodeNum];
		}
		nodeNum = nodes[nodeNum*2 + (in&1)];
		in >>= 1;
		
		if (nodeNum < 256)
		{
			nodes = nodesBase + (nodeNum<<9);
			*out++ = cin->palette[nodeNum];
			if (!--count)
				break;
			nodeNum = cin->hNumNodes1[nodeNum];
		}
		nodeNum = nodes[nodeNum*2 + (in&1)];
		in >>= 1;
		
		if (nodeNum < 256)
		{
			nodes = nodesBase + (nodeNum<<9);
			*out++ = cin->palette[nodeNum];
			if (!--count)
				break;
			nodeNum = cin->hNumNodes1[nodeNum];
		}
		nodeNum = nodes[nodeNum*2 + (in&1)];
		in >>= 1;
		
		if (nodeNum < 256)
		{
			nodes = nodesBase + (nodeNum<<9);
			*out++ = cin->palette[nodeNum];
			if (!--count)
				break;
			nodeNum = cin->hNumNodes1[nodeNum];
		}
		nodeNum = nodes[nodeNum*2 + (in&1)];
		in >>= 1;
		
		if (nodeNum < 256)
		{
			nodes = nodesBase + (nodeNum<<9);
			*out++ = cin->palette[nodeNum];
			if (!--count)
				break;
			nodeNum = cin->hNumNodes1[nodeNum];
		}
		nodeNum = nodes[nodeNum*2 + (in&1)];
		in >>= 1;
		
		if (nodeNum < 256)
		{
			nodes = nodesBase + (nodeNum<<9);
			*out++ = cin->palette[nodeNum];
			if (!--count)
				break;
			nodeNum = cin->hNumNodes1[nodeNum];
		}
		nodeNum = nodes[nodeNum*2 + (in&1)];
		in >>= 1;
		
		if (nodeNum < 256)
		{
			nodes = nodesBase + (nodeNum<<9);
			*out++ = cin->palette[nodeNum];
			if (!--count)
				break;
			nodeNum = cin->hNumNodes1[nodeNum];
		}
		nodeNum = nodes[nodeNum*2 + (in&1)];
		in >>= 1;
	}

	if (input - data != size && input - data != size+1)
		Com_Error(ERR_DROP, "CIN_Huff1Decompress: decompression overread by %i", (input - data) - size);
}

/*
=================
CIN_ReadPalette
=================
*/
static void CIN_ReadPalette (cinematic_t *cin)
{
	int		i;
	byte	palette[768], *pal;

	FS_Read(palette, sizeof(palette), cin->file);
	cin->remaining -= sizeof(palette);

	pal = (byte *)cin->palette;
	for (i = 0; i < 256; i++)
	{
		pal[i*4+0] = palette[i*3+0];
		pal[i*4+1] = palette[i*3+1];
		pal[i*4+2] = palette[i*3+2];
		pal[i*4+3] = 255;
	}
}

/*
=================
CIN_ResampleFrame
=================
*/
static void CIN_ResampleFrame (cinematic_t *cin)
{
	int			i, j;
	unsigned	*src, *dst;
	int			frac, fracStep;

	if (cin->rawWidth == cin->vidWidth && cin->rawHeight == cin->vidHeight)
		return;

	dst = (unsigned *)cin->rawBuffer;
	fracStep = cin->vidWidth * 0x10000 / cin->rawWidth;

	for (i = 0; i < cin->rawHeight; i++, dst += cin->rawWidth)
	{
		src = (unsigned *)cin->vidBuffer + cin->vidWidth * (i * cin->vidHeight / cin->rawHeight);
		frac = fracStep >> 1;

		for (j = 0; j < cin->rawWidth; j++)
		{
			dst[j] = src[frac>>16];
			frac += fracStep;
		}
	}
}

/*
=================
CIN_ReadVideoFrame
=================
*/
static void CIN_ReadVideoFrame (cinematic_t *cin)
{
	if (!cin->isRoQ)
	{
		byte		compressed[0x20000];
		int			size;

		FS_Read(&size, sizeof(size), cin->file);
		cin->remaining -= sizeof(size);

		size = LittleLong(size);
		if (size < 1 || size > sizeof(compressed))
			Com_Error(ERR_DROP, "CIN_ReadVideoFrame: bad compressed frame size (%i)", size);

		FS_Read(compressed, size, cin->file);
		cin->remaining -= size;

		CIN_Huff1Decompress(cin, compressed, size);

		cin->vidBuffer = cin->hBuffer;
	}
	else
	{
		roqChunk_t	*chunk = &cin->roqChunk;
		roqQCell_t	*qcell;
		int			i, vqFlgPos, vqId, pos, xPos, yPos, x, y, xp, yp;
		short		vqFlg;
		byte		c[4], *tmp;

		vqFlg = 0;
		vqFlgPos = -1;

		xPos = yPos = 0;
		pos = chunk->size;

		while (pos > 0)
		{
			for (yp = yPos; yp < yPos + 16; yp += 8)
			{
				for (xp = xPos; xp < xPos + 16; xp += 8)
				{
					if (vqFlgPos < 0)
					{
						FS_Read(&vqFlg, sizeof(vqFlg), cin->file);
						pos -= sizeof(vqFlg);

						vqFlg = LittleShort(vqFlg);
						vqFlgPos = 7;
					}

					vqId = (vqFlg >> (vqFlgPos * 2)) & 0x3;
					vqFlgPos--;
				
					switch (vqId)
					{
					case RoQ_ID_MOT:

						break;
					case RoQ_ID_FCC:
						FS_Read(c, 1, cin->file);
						pos--;

						CIN_ApplyMotion8x8(cin, xp, yp, c[0], (char)((chunk->argument >> 8) & 0xff), (char)(chunk->argument & 0xff));

						break;
					case RoQ_ID_SLD:
						FS_Read(c, 1, cin->file);
						pos--;

						qcell = cin->roqQCells + c[0];
						CIN_ApplyVector4x4(cin, xp, yp, cin->roqCells + qcell->idx[0]);
						CIN_ApplyVector4x4(cin, xp+4, yp, cin->roqCells + qcell->idx[1]);
						CIN_ApplyVector4x4(cin, xp, yp+4, cin->roqCells + qcell->idx[2]);
						CIN_ApplyVector4x4(cin, xp+4, yp+4, cin->roqCells + qcell->idx[3]);

						break;
					case RoQ_ID_CCC:
						for (i = 0; i < 4; i++)
						{
							x = xp; 
							y = yp;

							if (i & 0x01) 
								x += 4;
							if (i & 0x02) 
								y += 4;
						
							if (vqFlgPos < 0)
							{
								FS_Read(&vqFlg, sizeof(vqFlg), cin->file);
								pos -= sizeof(vqFlg);

								vqFlg = LittleShort(vqFlg);
								vqFlgPos = 7;
							}

							vqId = (vqFlg >> (vqFlgPos * 2)) & 0x3;
							vqFlgPos--;

							switch (vqId)
							{
							case RoQ_ID_MOT:

								break;
							case RoQ_ID_FCC:
								FS_Read(c, 1, cin->file);
								pos--;

								CIN_ApplyMotion4x4(cin, x, y, c[0], (char)((chunk->argument >> 8) & 0xff), (char)(chunk->argument & 0xff));

								break;
							case RoQ_ID_SLD:
								FS_Read(c, 1, cin->file);
								pos--;

								qcell = cin->roqQCells + c[0];
								CIN_ApplyVector2x2(cin, x, y, cin->roqCells + qcell->idx[0]);
								CIN_ApplyVector2x2(cin, x+2, y, cin->roqCells + qcell->idx[1]);
								CIN_ApplyVector2x2(cin, x, y+2, cin->roqCells + qcell->idx[2]);
								CIN_ApplyVector2x2(cin, x+2, y+2, cin->roqCells + qcell->idx[3]);

								break;
							case RoQ_ID_CCC:
								FS_Read(&c, 4, cin->file);
								pos -= 4;

								CIN_ApplyVector2x2(cin, x, y, cin->roqCells + c[0]);
								CIN_ApplyVector2x2(cin, x+2, y, cin->roqCells + c[1]);
								CIN_ApplyVector2x2(cin, x, y+2, cin->roqCells + c[2]);
								CIN_ApplyVector2x2(cin, x+2, y+2, cin->roqCells + c[3]);

								break;
							}
						}

						break;
					default:
						Com_Error(ERR_DROP, "CIN_ReadVideoFrame: unknown VQ code (%i)",  vqId);
					}
				}
			}
			
			xPos += 16;
			if (xPos >= cin->vidWidth)
			{
				xPos -= cin->vidWidth;
				yPos += 16;
			}
			if (yPos >= cin->vidHeight && pos)
			{
				CIN_Skip(cin, pos);
				break;
			}
		}

		cin->remaining -= (chunk->size - pos);
	
		if (cin->frameCount == 0)
			memcpy(cin->roqBufferPtr[1], cin->roqBufferPtr[0], cin->vidWidth * cin->vidHeight * 4);
		else
		{
			tmp = cin->roqBufferPtr[0];
			cin->roqBufferPtr[0] = cin->roqBufferPtr[1];
			cin->roqBufferPtr[1] = tmp;
		}

		cin->vidBuffer = cin->roqBufferPtr[1];
	}

	// Resample
	CIN_ResampleFrame(cin);
}


/*
=================
CIN_ReadAudioFrame
=================
*/
static void CIN_ReadAudioFrame (cinematic_t *cin)
{
	byte	data[0x40000];
	int		samples;

	if (!cin->isRoQ)
	{
		byte		*p;
		int			start, end, len;

		start = cin->frameCount*cin->sndRate/14;
		end = (cin->frameCount+1)*cin->sndRate/14;
		samples = end - start;
		len = samples * cin->sndWidth * cin->sndChannels;

		if (cin->flags & CIN_SILENT)
		{
			CIN_Skip(cin, len);
			return;
		}

		// HACK: gross hack to keep cinematic audio sync'ed using OpenAL
		if (cin->frameCount == 0)
		{
			samples += 4096;

			if (cin->sndWidth == 2)
				memset(data, 0x00, 4096 * cin->sndWidth * cin->sndChannels);
			else
				memset(data, 0x80, 4096 * cin->sndWidth * cin->sndChannels);

			p = data + (4096 * cin->sndWidth * cin->sndChannels);
		}
		else
			p = data;

		FS_Read(p, len, cin->file);
		cin->remaining -= len;
	}
	else
	{
		roqChunk_t	*chunk = &cin->roqChunk;
		byte		compressed[0x20000];
		short		l, r;
		int			i;

		if (cin->flags & CIN_SILENT)
		{
			CIN_Skip(cin, cin->roqChunk.size);
			return;
		}

		FS_Read(compressed, chunk->size, cin->file);
		cin->remaining -= chunk->size;

		if (chunk->id == RoQ_SOUND_MONO)
		{
			cin->sndChannels = 1;

			l = chunk->argument;

			for (i = 0; i < chunk->size; i++)
			{
				l += cin->roqSndSqrTable[compressed[i]];

				((short *)&data)[i] = l;
			}

			samples = chunk->size;
		}
		else if (chunk->id == RoQ_SOUND_STEREO)
		{
			cin->sndChannels = 2;

			l = chunk->argument & 0xff00;
			r = (chunk->argument & 0xff) << 8;

			for (i = 0; i < chunk->size; i += 2)
			{
				l += cin->roqSndSqrTable[compressed[i+0]];
				r += cin->roqSndSqrTable[compressed[i+1]];

				((short *)&data)[i+0] = l;
				((short *)&data)[i+1] = r;
			}

			samples = chunk->size / 2;
		}
	}

	// Send sound to mixer
	//S_StreamRawSamples(data, samples, cin->sndRate, cin->sndWidth, cin->sndChannels);
	S_RawSamples(samples, cin->sndRate, cin->sndWidth, cin->sndChannels, data, false);
}

/*
=================
CIN_ReadNextFrame
=================
*/
static qboolean CIN_ReadNextFrame (cinematic_t *cin)
{
	if (!cin->isRoQ)
	{
		int			command;
		
		while (cin->remaining > 0)
		{
			FS_Read(&command, sizeof(command), cin->file);
			cin->remaining -= sizeof(command);

			command = LittleLong(command);

			if (cin->remaining <= 0 || command == 2)
				return false;	// Done

			if (command == 1)
				CIN_ReadPalette(cin);

			CIN_ReadVideoFrame(cin);
			CIN_ReadAudioFrame(cin);

			cin->frameCount++;
			cl.cinematicframe = cin->frameCount;
			return true;
		}

		return false;
	}
	else
	{
		roqChunk_t	*chunk = &cin->roqChunk;

		while (cin->remaining > 0)
		{
			CIN_ReadChunk(cin);

			if (cin->remaining <= 0 || chunk->size > cin->remaining)
				return false;	// Done

			if (chunk->size <= 0)
				continue;

			if (chunk->id == RoQ_INFO)
				CIN_ReadInfo(cin);
			else if (chunk->id == RoQ_QUAD_CODEBOOK)
				CIN_ReadCodebook(cin);
			else if (chunk->id == RoQ_QUAD_VQ)
			{
				CIN_ReadVideoFrame(cin);

				cin->frameCount++;
				cl.cinematicframe = cin->frameCount;
				return true;
			}
			else if (chunk->id == RoQ_SOUND_MONO || chunk->id == RoQ_SOUND_STEREO)
				CIN_ReadAudioFrame(cin);
			else
				CIN_Skip(cin, cin->roqChunk.size);
		}

		return false;
	}
}

/*
=================
CIN_StaticCinematic
=================
*/
static qboolean CIN_StaticCinematic (cinematic_t *cin, const char *name)
{
	byte		*buffer;
	byte		*in, *out;
	pcx_t		*pcx;
	int			x, y, len;
	int			dataByte, runLength;
	byte		palette[768], *pal;
	int			i;

	// Load the file
	len = FS_LoadFile((char *)name, (void **)&buffer);
	if (!buffer)
		return false;

	// Parse the PCX file
	pcx = (pcx_t *)buffer;

    pcx->xmin = LittleShort(pcx->xmin);
    pcx->ymin = LittleShort(pcx->ymin);
    pcx->xmax = LittleShort(pcx->xmax);
    pcx->ymax = LittleShort(pcx->ymax);
    pcx->hres = LittleShort(pcx->hres);
    pcx->vres = LittleShort(pcx->vres);
    pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
    pcx->palette_type = LittleShort(pcx->palette_type);

	in = &pcx->data;

	if (pcx->manufacturer != 0x0A || pcx->version != 5 || pcx->encoding != 1)
	{
		FS_FreeFile(buffer);
		Com_Error(ERR_DROP, "CIN_StaticCinematic: invalid PCX header");
	}

	if (pcx->bits_per_pixel != 8 || pcx->color_planes != 1){
		FS_FreeFile(buffer);
		Com_Error(ERR_DROP, "CIN_StaticCinematic: only 8 bit PCX images supported");
	}
		
	if (pcx->xmax >= 640 || pcx->ymax >= 480 || pcx->xmax <= 0 || pcx->ymax <= 0)
	{
		FS_FreeFile(buffer);
		Com_Error(ERR_DROP, "CIN_StaticCinematic: bad PCX file (%i x %i)", pcx->xmax, pcx->ymax);
	}

	memcpy(palette, (byte *)buffer + len - 768, 768);

	pal = (byte *)cin->palette;
	for (i = 0; i < 256; i++){
		pal[i*4+0] = palette[i*3+0];
		pal[i*4+1] = palette[i*3+1];
		pal[i*4+2] = palette[i*3+2];
		pal[i*4+3] = 255;
	}

	cin->vidWidth = pcx->xmax+1;
	cin->vidHeight = pcx->ymax+1;

	cin->pcxBuffer = out = Z_Malloc(cin->vidWidth * cin->vidHeight * 4);

	for (y = 0; y <= pcx->ymax; y++)
	{
		for (x = 0; x <= pcx->xmax; )
		{
			dataByte = *in++;

			if((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = *in++;
			}
			else
				runLength = 1;

			while (runLength-- > 0)
			{
				*(unsigned *)out = cin->palette[dataByte];

				out += 4;
				x++;
			}
		}
	}

	if (in - buffer > len)
	{
		FS_FreeFile(buffer);
		Z_Free(cin->pcxBuffer);
		cin->pcxBuffer = NULL;
		Com_Error(ERR_DROP, "CIN_StaticCinematic: PCX file was malformed");
	}

	FS_FreeFile(buffer);

	cin->vidBuffer = cin->pcxBuffer;

	if (glConfig.arbTextureNonPowerOfTwo) {
		cin->rawWidth = cin->vidWidth;
		cin->rawHeight = cin->vidHeight;
	}
	else {
		cin->rawWidth = 256;
		cin->rawHeight = 256;
	}

	if (cin->rawWidth != cin->vidWidth || cin->rawHeight != cin->vidHeight)
		cin->rawBuffer = Z_Malloc(cin->rawWidth * cin->rawHeight * 4);

	// Resample
	CIN_ResampleFrame(cin);

	cin->frameCount = -1;
	cin->frameTime = cls.realtime;
	cl.cinematicframe = cin->frameCount;
	cl.cinematictime = cin->frameTime;

	cin->playing = true;

	return true;
}

/*
=================
CIN_HandleForVideo
=================
*/
static cinematic_t *CIN_HandleForVideo (cinHandle_t *handle)
{
	cinematic_t	*cin;
	int			i;

	for (i = 0, cin = cinematics; i < MAX_CINEMATICS; i++, cin++)
	{
		if (cin->playing)
			continue;

		*handle = i+1;
		return cin;
	}

	Com_Error(ERR_DROP, "CIN_HandleForVideo: none free\n");
	return NULL;
}

/*
=================
CIN_GetVideoByHandle
=================
*/
static cinematic_t *CIN_GetVideoByHandle (cinHandle_t handle)
{
	if (handle <= 0 || handle > MAX_CINEMATICS)
		Com_Error(ERR_DROP, "CIN_GetVideoByHandle: out of range");

	return &cinematics[handle-1];
}

/*
==================
SCR_RunCinematic
==================
*/
void SCR_RunCinematic (void)
{
	// Do nothing
	//CIN_RunCinematic(cls.cinematicHandle);
}

/*
=================
CIN_RunCinematic
=================
*/
qboolean CIN_RunCinematic (cinHandle_t handle)
{
	cinematic_t	*cin;
	int			frame;

	cin = CIN_GetVideoByHandle(handle);

	if (!cin->playing)
		return false;		// Not running
	
	if (cin->frameCount == -1)
		return true;		// Static image

	/*if (cls.key_dest != key_game)
	{	// pause if menu or console is up
		//cl.cinematictime = cls.realtime - cl.cinematicframe*1000/14;
		cin->frameTime = cls.realtime - cin->frameTime*1000/14;
		cl.cinematictime = cin->frameTime;
		return true;
	}*/

	frame = (cls.realtime - cin->frameTime) * cin->rate/1000;
	if (frame <= cin->frameCount)
		return true;

	if (frame > cin->frameCount+1)
		cin->frameTime = cls.realtime - cin->frameCount * 1000/cin->rate;

	if (!CIN_ReadNextFrame(cin))
	{
		if (cin->flags & CIN_LOOPED)
		{
			// Restart the cinematic
			FS_Seek(cin->file, 0, FS_SEEK_SET);
			cin->remaining = cin->size;

			// Skip over the header
			CIN_Skip(cin, cin->start);

			cin->frameCount = 0;
			cin->frameTime = cls.realtime;
			cl.cinematicframe = cin->frameCount;
			cl.cinematictime = cin->frameTime;

			return true;
		}

		return false;	// Finished
	}

	return true;
}

/*
=================
CIN_SetExtents
=================
*/
void CIN_SetExtents (cinHandle_t handle, int x, int y, int w, int h)
{
	cinematic_t	*cin;
	float		realx, realy, realw, realh;

	cin = CIN_GetVideoByHandle(handle);

	if (!cin->playing)
		return;			// Not running

	realx = x;	realy = y;	realw = w;	realh = h; 
	SCR_AdjustFrom640 (&realx, &realy, &realw, &realh, ALIGN_CENTER);
	cin->x = realx;
	cin->y = realy;
	cin->w = realw;
	cin->h = realh;
}

/*
=================
SCR_PlayCinematic
=================
*/
void SCR_PlayCinematic (char *name)
{
	char	filename[MAX_QPATH];

	// If currently playing another, stop it
	SCR_StopCinematic();

	Com_DPrintf("SCR_PlayCinematic( %s )\n", name);

	cl.cinematicframe = 0;
	if (!Q_stricmp(name + strlen(name)-4, ".pcx"))
	{
		Q_strncpyz (filename, sizeof(filename), name);
		Com_DefaultPath (filename, sizeof(filename), "pics");
		cl.cinematicframe = -1;
		cl.cinematictime = 1;
		SCR_EndLoadingPlaque ();
		cls.state = ca_active;
	}
	else
	{
		Q_strncpyz (filename, sizeof(filename), name);
		Com_DefaultPath (filename, sizeof(filename), "video");
		Com_DefaultExtension (filename, sizeof(filename), ".cin");
	}

//	cls.cinematicHandle = CIN_PlayCinematic(filename, 0, 0, viddef.width, viddef.height, CIN_SYSTEM);
	cls.cinematicHandle = CIN_PlayCinematic(filename, 0, 0, 640, 480, CIN_SYSTEM);
	if (!cls.cinematicHandle)
	{
		Com_Printf("Cinematic %s not found\n", filename);
		cl.cinematictime = 0;	// done
		SCR_FinishCinematic();
	}
	else
	{
		SCR_EndLoadingPlaque ();
		cls.state = ca_active;
		cl.cinematicframe = 0;
		cl.cinematictime = Sys_Milliseconds ();
	}
}

/*
=================
SCR_StopCinematic
=================
*/
void SCR_StopCinematic (void)
{
	if (!cls.cinematicHandle)
		return;

	Com_DPrintf("SCR_StopCinematic()\n");

	CIN_StopCinematic(cls.cinematicHandle);
	cls.cinematicHandle = 0;
}

/*
====================
SCR_FinishCinematic

Called when either the cinematic completes, or it is aborted
====================
*/
void SCR_FinishCinematic (void)
{
	// tell the server to advance to the next map / cinematic
	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	SZ_Print (&cls.netchan.message, va("nextserver %i\n", cl.servercount));
}

/*
=================
SCR_DrawCinematic
=================
*/
qboolean SCR_DrawCinematic (void)
{

	if (cl.cinematictime <= 0)
		return false;

	if (!cls.cinematicHandle)
		return false;

	if (!CIN_RunCinematic(cls.cinematicHandle))
	{
		SCR_StopCinematic();
		SCR_FinishCinematic();
		return false;
	}

//	CIN_SetExtents(cls.cinematicHandle, 0, 0, viddef.width, viddef.height);
	CIN_SetExtents(cls.cinematicHandle, 0, 0, 640, 480);
	CIN_DrawCinematic(cls.cinematicHandle);
	return true;
}

/*
=================
CIN_DrawCinematic
=================
*/
void CIN_DrawCinematic (cinHandle_t handle){

	cinematic_t	*cin;

	cin = CIN_GetVideoByHandle(handle);

	if (!cin->playing)
		return;			// Not running

	if (cin->frameCount == -1) // Knightmare- HACK to show JPG endscreens
	{
		char	picname[MAX_QPATH] = "/";
		float	x=0, y=0, w=640, h=480;
	//	strncat(picname, cin->name);
		Q_strncatz (picname, sizeof(picname), cin->name);
		SCR_AdjustFrom640 (&x, &y, &w, &h, ALIGN_CENTER);
		if (w < viddef.width || h < viddef.height)
			R_DrawFill (0, 0, viddef.width, viddef.height, 0, 0, 0, 255);
	//	R_DrawStretchPic (x, y, viddef.width, viddef.height, picname, 1.0);
		R_DrawStretchPic (x, y, w, h, picname, 1.0);
		return;
	} // end JPG hack

	if (cin->w < viddef.width || cin->h < viddef.height)
		R_DrawFill (0, 0, viddef.width, viddef.height, 0, 0, 0, 255);
	if (cin->rawWidth == cin->vidWidth && cin->rawHeight == cin->vidHeight)
		R_DrawStretchRaw(cin->x, cin->y, cin->w, cin->h, cin->vidBuffer, cin->vidWidth, cin->vidHeight); //(cin->flags & CIN_SHADER));
	else
		R_DrawStretchRaw(cin->x, cin->y, cin->w, cin->h, cin->rawBuffer, cin->rawWidth, cin->rawHeight); //(cin->flags & CIN_SHADER));
}

/*
=================
CIN_PlayCinematic
=================
*/
cinHandle_t CIN_PlayCinematic (const char *name, int x, int y, int w, int h, int flags)
{
	cinematic_t	*cin;
	cinHandle_t	handle;
	int			i;
	char		extension[8];
	float		realx, realy, realw, realh;

	// See if already playing this cinematic
	for (i = 0, cin = cinematics; i < MAX_CINEMATICS; i++, cin++)
	{
		if (!cin->playing)
			continue;

		if (!Q_stricmp(cin->name, (char *)name))
			return i+1;
	}

	Com_FileExtension(name, extension, sizeof(extension));

	if (!Q_stricmp(extension, "cin")) // RoQ autoreplace hack
	{
		char s[MAX_QPATH];
		int len;
		len = (int)strlen(name);
	//	strncpy (s, name);
		Q_strncpyz (s, sizeof(s), name);
		s[len-3]='r'; s[len-2]='o'; s[len-1]='q';
		handle  = CIN_PlayCinematic (s, x, y ,w, h, flags);
		if (handle)
			return handle;
	}

	// Find a free handle
	cin = CIN_HandleForVideo(&handle);

	// Fill it in
	Q_strncpyz (cin->name, sizeof(cin->name), name);
	realx = x;	realy = y;	realw = w;	realh = h; 
	SCR_AdjustFrom640 (&realx, &realy, &realw, &realh, ALIGN_CENTER);
	cin->x = realx;
	cin->y = realy;
	cin->w = realw;
	cin->h = realh;
	cin->flags = flags;

	if (cin->flags & CIN_SYSTEM)
	{
		CDAudio_Stop();					// Make sure CD audio isn't playing
		S_StopAllSounds();				// Make sure sound isn't playing
		//UI_SetActiveMenu(UI_CLOSEMENU);	
		UI_ForceMenuOff();				// Close the menu
	}

	//Com_FileExtension(name, extension, sizeof(extension));

	if (!Q_stricmp(extension, "pcx"))
	{
		// Static PCX image
		if (!CIN_StaticCinematic(cin, name))
			return 0;

		return handle;
	}

	if (!Q_strcasecmp(extension, "roq"))
	{
		cin->isRoQ = true;
		cin->rate = 30;
	}
	else if (!Q_stricmp(extension, "cin"))
	{
		cin->isRoQ = false;
		cin->rate = 14;
	}
	else
		return 0;

	// Open the cinematic file
	cin->size = FS_FOpenFile(name, &cin->file, FS_READ);
	if (!cin->file)
		return 0;

	cin->remaining = cin->size;
	
	// Read the header
	if (!cin->isRoQ)
	{
		FS_Read(&cin->vidWidth, sizeof(cin->vidWidth), cin->file);
		FS_Read(&cin->vidHeight, sizeof(cin->vidHeight), cin->file);
		cin->vidWidth = LittleLong(cin->vidWidth);
		cin->vidHeight = LittleLong(cin->vidHeight);

		FS_Read(&cin->sndRate, sizeof(cin->sndRate), cin->file);
		FS_Read(&cin->sndWidth, sizeof(cin->sndWidth), cin->file);
		FS_Read(&cin->sndChannels, sizeof(cin->sndChannels), cin->file);
		cin->sndRate = LittleLong(cin->sndRate);
		cin->sndWidth = LittleLong(cin->sndWidth);
		cin->sndChannels = LittleLong(cin->sndChannels);

		cin->remaining -= 20;

		if (glConfig.arbTextureNonPowerOfTwo) {
			cin->rawWidth = cin->vidWidth;
			cin->rawHeight = cin->vidHeight;
		}
		else {
			cin->rawWidth = 256;
			cin->rawHeight = 256;
		}

		if (cin->rawWidth != cin->vidWidth || cin->rawHeight != cin->vidHeight)
			cin->rawBuffer = Z_Malloc(cin->rawWidth * cin->rawHeight * 4);

		CIN_Huff1TableInit(cin);

		cin->start = FS_FTell(cin->file);
	}
	else
	{
		cin->sndRate = 22050;
		cin->sndWidth = 2;

		CIN_ReadChunk(cin);
		
		CIN_SoundSqrTableInit(cin);

		cin->start = FS_FTell(cin->file);
	}

	//if (!(cin->flags & CIN_SILENT))
	//	S_StartStreaming();

	cin->frameCount = 0;
	cin->frameTime = cls.realtime;
	cl.cinematicframe = cin->frameCount;
	cl.cinematictime = cin->frameTime;

	cin->playing = true;

	// Read the first frame
	CIN_ReadNextFrame(cin);

	return handle;
}

/*
=================
CIN_StopCinematic
=================
*/
void CIN_StopCinematic (cinHandle_t handle)
{
	cinematic_t	*cin;

	if (!handle)
		return;

	cin = CIN_GetVideoByHandle(handle);

	if (!cin->playing)
		return;			// Not running

	//if (!(cin->flags & CIN_SILENT))
	//	S_StopStreaming();

	if (cin->rawBuffer)
		Z_Free(cin->rawBuffer);

	if (cin->pcxBuffer)
		Z_Free(cin->pcxBuffer);

	if (cin->hNodes1)
		Z_Free(cin->hNodes1);
	if (cin->hBuffer)
		Z_Free(cin->hBuffer);

	if (cin->roqBuffer)
		Z_Free(cin->roqBuffer);

	if (cin->file)
		FS_FCloseFile(cin->file);

	memset(cin, 0, sizeof(*cin));
}
#endif // ROQ_SUPPORT
