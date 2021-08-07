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

//
// MD3 representation in memory
// Vic code with a few my changes
// -Harven
//

typedef struct maliascoord_s
{
	vec2_t			st;
} maliascoord_t;

typedef struct maliasvertex_s
{
	short			xyz[3];
	byte			normal[2];
	byte			lightnormalindex; // used for ye olde quantized normal shading
} maliasvertex_t;

typedef struct
{
    vec3_t			mins;
	vec3_t			maxs;
	vec3_t			scale;
    vec3_t			translate;
    float			radius;
} maliasframe_t;

typedef struct
{
	char			name[MD3_MAX_PATH];
	dorientation_t	orient;
} maliastag_t;

typedef enum {
	WAVEFORM_SIN,
	WAVEFORM_TRIANGLE,
	WAVEFORM_SQUARE,
	WAVEFORM_SAWTOOTH,
	WAVEFORM_INVERSESAWTOOTH,
	WAVEFORM_NOISE
} waveForm_t;

typedef struct
{
	waveForm_t		type;
	float			params[4];
} waveFunc_t;

typedef struct
{
	float			translate_x;
	float			translate_y;
	float			rotate;
	float			scale_x;
	float			scale_y;
	waveFunc_t		stretch;
	waveFunc_t		turb;
	float			scroll_x;
	float			scroll_y;
} tcmodParms_t;

typedef struct 
{
	qboolean		twosided;
	qboolean		alphatest;
	qboolean		fullbright;
	qboolean		nodraw;
	qboolean		noshadow;
	qboolean		nodiffuse;
	qboolean		blend;
	tcmodParms_t	tcmod;
	float			envmap;
	float			basealpha;
	GLenum			blendfunc_src;
	GLenum			blendfunc_dst;
	waveFunc_t		glow;
} renderparms_t;

typedef struct 
{
	char			name[MD3_MAX_PATH];
	char			glowname[MD3_MAX_PATH];
	image_t			*glowimage;
	renderparms_t	renderparms;
	//int				shader;
} maliasskin_t;

typedef struct
{
    int				num_verts;
	char			name[MD3_MAX_PATH];
	maliasvertex_t	*vertexes;
	maliascoord_t	*stcoords;

    int				num_tris;
    index_t			*indexes;
	int				*trneighbors;

    int				num_skins;
	maliasskin_t	*skins;
} maliasmesh_t;

typedef struct maliasmodel_s
{
    int				num_frames;
	maliasframe_t	*frames;

    int				num_tags;
	maliastag_t		*tags;

    int				num_meshes;
	maliasmesh_t	*meshes;
} maliasmodel_t;
