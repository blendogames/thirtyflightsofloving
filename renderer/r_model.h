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

/*
d*_t structures are on-disk representations
m*_t structures are in-memory
*/

/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


//
// in memory representation
//
// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	vec3_t		position;
} mvertex_t;

typedef struct
{
	vec3_t		mins, maxs;
	vec3_t		origin;		// for sounds or lights
	float		radius;
	int			headnode;
	int			visleafs;		// not including the solid leaf 0
	int			firstface, numfaces;
} mmodel_t;


#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2


#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWTURB		0x10
#define SURF_DRAWBACKGROUND	0x40
#define SURF_LIGHTMAPPED	0x80
#define SURF_UNDERWATER		0x100
#define SURF_UNDERSLIME		0x200
#define SURF_UNDERLAVA		0x400
#define SURF_MASK_CAUSTIC	(SURF_UNDERWATER|SURF_UNDERSLIME|SURF_UNDERLAVA)
// Psychospaz's envmapping
#define SURF_ENVMAP			0x800

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	unsigned short	v[2];
	unsigned int	cachededgeoffset;
} medge_t;

typedef struct mtexinfo_s
{
	float		vecs[2][4];
	int			texWidth;	// added Q2E hack
	int			texHeight;	// added Q2E hack
	int			flags;
	int			numframes;
	struct mtexinfo_s	*next;		// animation chain
	image_t		*image;
	image_t		*glow;		// glow overlay
} mtexinfo_t;

typedef struct
{
	vec3_t		xyz;
	vec2_t		texture_st;
	vec2_t		lightmap_st;
	byte		basecolor[3];
	byte		lightcolor[3];
} mpolyvertex_t;

//#define	VERTEXSIZE	7

typedef struct glpoly_s
{
	struct	glpoly_s	*next;
	struct	glpoly_s	*chain;
	int		numverts;

	qboolean	vertexlightset;
//	byte		*vertexlightbase;
//	byte		*vertexlight;
	vec3_t		center;

	int			flags;			// for SURF_UNDERWATER (not needed anymore?)
//	float		verts[4][VERTEXSIZE];	// variable sized (xyz s1t1 s2t2)
	mpolyvertex_t	verts[4];			// variable sized (xyz s1t1 s2t2 rgb rgb)
} glpoly_t;

typedef struct msurface_s
{
	int			visframe;		// should be drawn when node is crossed

	cplane_t	*plane;
	int			flags;

	int			firstedge;	// look up in model->surfedges[], negative numbers
	int			numedges;	// are backwards edges
	
	short		texturemins[2];
	short		extents[2];

	int			light_s, light_t;	// gl lightmap coordinates
	int			light_smax, light_tmax;
	int			dlight_s, dlight_t; // gl lightmap coordinates for dynamic lightmaps

	glpoly_t	*polys;				// multiple if warped
	struct	msurface_s	*texturechain;
	struct  msurface_s	*lightmapchain;

	mtexinfo_t	*texinfo;
	
// lighting info
	int			dlightframe;
	int			dlightbits[(MAX_DLIGHTS+31)>>5];	// derived from MAX_DLIGHTS
	qboolean	cached_dlight;
	qboolean	isLightmapped;	// Knightmare- for whether a warp surface is lightmapped

	int			lightmaptexturenum;
	byte		styles[MAXLIGHTMAPS];
	float		cached_light[MAXLIGHTMAPS];	// values currently used in lightmap
	byte		*samples;		// [numstyles*surfsize]
//	byte		*stains;		// added stainmaps

	void		*chain_part;
	void		*chain_ent;

	int			checkCount;
	entity_t	*entity;		// entity pointer
} msurface_t;


typedef struct surfaceHandle_s
{
	msurface_t				*surf;
	entity_t				*entity;		// entity pointer
	int						flags;			// entity-specific surface flags
	struct	surfaceHandle_s	*surfacechain;	// next surface
} surfaceHandle_t;


typedef struct mnode_s
{
// common with leaf
	int			contents;		// -1, to differentiate from leafs
	int			visframe;		// node needs to be traversed if current
	
	float		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// node specific
	cplane_t	*plane;
	struct mnode_s	*children[2];	

	unsigned short		firstsurface;
	unsigned short		numsurfaces;
} mnode_t;



typedef struct mleaf_s
{
// common with node
	int			contents;		// wil be a negative contents number
	int			visframe;		// node needs to be traversed if current

	float		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// leaf specific
	int			cluster;
	int			area;

	msurface_t	**firstmarksurface;
	int			nummarksurfaces;
} mleaf_t;


//===================================================================

//
// Whole model
//

//Harven++ MD3 added mod_md3
typedef enum {mod_bad, mod_brush, mod_sprite, mod_alias, mod_md2 } modtype_t;
//Harven-- MD3

typedef struct model_s
{
	char		name[MAX_QPATH];

	int			registration_sequence;

	modtype_t	type;
	int			numframes;
	
	int			flags;

//
// volume occupied by the model graphics
//		
	vec3_t		mins, maxs;
	float		radius;

//
// solid volume for clipping 
//
	qboolean	clipbox;
	vec3_t		clipmins, clipmaxs;

//
// brush model
//
	int			firstmodelsurface, nummodelsurfaces;
	int			lightmap;		// only for submodels

	int			numsubmodels;
	mmodel_t	*submodels;

	int			numplanes;
	cplane_t	*planes;

	int			numleafs;		// number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int			numvertexes;
	mvertex_t	*vertexes;

	int			numedges;
	medge_t		*edges;

	int			numnodes;
	int			firstnode;
	mnode_t		*nodes;

	int			numtexinfo;
	mtexinfo_t	*texinfo;

	int			numsurfaces;
	msurface_t	*surfaces;

	int			numsurfedges;
	int			*surfedges;

	int			nummarksurfaces;
	msurface_t	**marksurfaces;

	dvis_t		*vis;

	byte		*lightdata;

	// for alias models and skins
	// Echon's per-mesh skin support
	image_t		*skins[MD3_MAX_MESHES][MD2_MAX_SKINS];
//	image_t		*skins[MD2_MAX_SKINS];

	size_t		extradatasize;
	void		*extradata;

	qboolean	hasAlpha;		// if model has scripted transparency
	int			bspVersion;		// For checking BSP version for compatibility
	int			bspFeatures;	// flags for BSP features

#ifdef PROJECTION_SHADOWS // projection shadows from BeefQuake R6
	//signed int	edge_tri[MD2_MAX_TRIANGLES][3]; // make this dynamically allocated?
	int			*edge_tri;
#endif // end projection shadows from BeefQuake R6
} model_t;

// Knightmare- BSP features flags
#define	BSPF_WARPLIGHTMAPS		0x00000001

//============================================================================

void	Mod_Init (void);
void	Mod_ClearAll (void);
model_t *Mod_ForName (char *name, qboolean crash);
mleaf_t *Mod_PointInLeaf (float *p, model_t *model);
byte	*Mod_ClusterPVS (int cluster, model_t *model);

void	Mod_Modellist_f (void);

void	*Hunk_Begin (size_t maxsize);
void	*Hunk_Alloc (size_t size);
size_t	Hunk_End (void);
void	Hunk_Free (void *base);

void	Mod_FreeAll (void);
void	Mod_Free (model_t *mod);

extern qboolean	registration_active;	// map registration flag
