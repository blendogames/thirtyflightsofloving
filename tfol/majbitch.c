/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2000-2002 Mr. Hyde and Mad Dog

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>

#ifndef NULL
  #define NULL ((void *)0)
#endif

typedef unsigned char byte;
typedef float vec_t;
typedef vec_t vec3_t[3];

#define LUMP_ENTDATA      0
#define LUMP_PLANES       1
#define LUMP_VERTEX       2
#define LUMP_VISLIST      3
#define LUMP_NODES        4
#define LUMP_TEXINFO      5
#define LUMP_FACES        6
#define LUMP_LIGHTDATA    7
#define LUMP_LEAFS        8
#define LUMP_LEAFFACES    9
#define LUMP_LEAFBRUSHES 10
#define LUMP_EDGES       11
#define LUMP_SURFEDGES   12
#define LUMP_MODELS      13
#define LUMP_BRUSHES     14
#define LUMP_BRUSHSIDES  15
#define LUMP_POP         16
#define LUMP_AREAS       17
#define LUMP_AREAPORTALS 18
#define LUMP_HEADER      19

#define MAX_MAP_AREAS          256
#define MAX_MAP_AREAPORTALS   1024
#define MAX_MAP_MODELS        1024
#define MAX_MAP_ENTITIES      2048
#define MAX_MAP_BRUSHES       8192
#define MAX_MAP_TEXINFO       8192
#define MAX_MAP_PLANES       65536
#define MAX_MAP_NODES        65536
#define MAX_MAP_BRUSHSIDES   65536
#define MAX_MAP_LEAFS        65536
#define MAX_MAP_VERTS        65536
#define MAX_MAP_FACES        65536
#define MAX_MAP_LEAFFACES    65536
#define MAX_MAP_LEAFBRUSHES  65536
#define MAX_MAP_PORTALS      65536
#define MAX_MAP_EDGES       128000
#define MAX_MAP_SURFEDGES   256000

#define MAX_MAP_ENTSTRING  0x40000
#define MAX_MAP_LIGHTING   0x200000
#define MAX_MAP_VISIBILITY 0x100000

// 0-2 are axial planes
#define PLANE_X  0
#define PLANE_Y  1
#define PLANE_Z  2

// 3-5 are non-axial planes
#define PLANE_ANYX  3
#define PLANE_ANYY  4
#define PLANE_ANYZ  5

#define  DVIS_PVS  0  // vis.bitofs[][0]
#define  DVIS_PHS  1  // vis.bitofs[][1]

typedef unsigned char byte;

//=============================
// Entire BSP Tree Structures
//=============================

typedef struct {
  float normal[3]; // normal to this plane
  float dist;      // used with normal to determine front/back of plane
  int   type;      // type of plane, (see: PLANE_X ... PLANE_ANYZ)
} plane_t;

typedef struct {
  float origin[3]; // location of this vertex
} vertex_t;

typedef struct {
  float blend[2][4]; // [s/t][4] - see q2 blend[] // Maj++ renamed
  int   flags;       // miptex flags (See SURF_* flag types)
  int   value;       // light emission
  char  texture[32]; // texture name (ie, "SKY")
  int   nexttexinfo; // next texinfo (-1 = end of chain)
} texinfo_t;

typedef struct {
  byte lightchar[1];
} lightdata_t;

typedef struct {
  short planenum;     // map->plane[]
  short side;         // 0 or 1 -1=none
  int   firstedge;    // map->edge[]
  short numedges;     // total edges associated with this face
  short texinfo;      // map->texinfo[]
  byte  lightstyle[4];// see Q2 lightstyles..
  int   lightofs;     // map->lightdata[]
} face_t;

typedef struct {
  int   planenum;  // map->plane[]
  int   right;     // child[0]: -1=leaf[-(right+1)],otherwise index to child node
  int   left;      // child[1]: -1=leaf[-(left+1)], otherwise index to child node
  short mins[3];   // minsize of this node (for tracing).
  short maxs[3];   // maxsize of this node (for tracing).
  short firstface; // map->face[]
  short numfaces;  // total faces associated with this node
} node_t;

typedef struct {
  int portalnum;   // number of this areaportal
  int nextarea;    // connected area[]
} areaportal_t;

typedef struct {
  int numareaportals;  // total areaportals associated with this area
  int firstareaportal; // map->areaportal[]
} area_t;

typedef struct {
  short dleaffaces[MAX_MAP_LEAFFACES]; // map->face[]
} leafface_t;

typedef struct {
  short planenum; // map->plane[]
  short texinfo;  // map->texinfo[]
} brushside_t;

typedef struct {
  int firstbrushside;// map->brushside[]
  int numbrushsides; // total brushsides associated with this surface
  int contents;      // pointcontents of this surface..
} brush_t;

typedef struct {
  short dleafbrushes[MAX_MAP_LEAFBRUSHES]; // map->brush[]
} leafbrush_t;

typedef struct {
  int numclusters; // ????
  int bitofs[8][2];// ????
} vis_t;

typedef struct {
  int    contents;       // pointcontents of this leaf
  short  cluster;        //
  short  area;           //
  short  mins[3];        // size of leaf
  short  maxs[3];        // size of leaf
  short  firstleafface;  // map->leafface[]
  short  numleaffaces;   // total leaffaces associated with this leaf
  short  firstleafbrush; // map->leafbrushes[]
  short  numleafbrushes; // total leafbrushes associated with this leaf
} leaf_t;

typedef struct {
  short v[2]; // vertex[xy] of this edge
} edge_t;

typedef struct {
  int dsurfedges[1];// map->edge[]
} surfedge_t;

typedef struct {
  byte dpop[256]; // ???????
} pop_t;

typedef struct {
  float mins[3];   // size of model
  float maxs[3];   // size of model
  float origin[3]; // location of model
  long  rootnode;  // start of BSP tree
  long  firstface; // map->face[]
  long  numfaces;  // total faces associated with this model
} model_t;

typedef struct {
  char entchar[MAX_MAP_ENTSTRING]; // map entities??? Maj++
} entdata_t;

//===============================================
typedef struct cplane_s {
  vec3_t normal;   // normal vector
  float  dist;     // distance
  byte   type;     // for fast side tests
  byte   signbits; // direction of normal
  byte   pad[2];   // unused
} cplane_t;

//=============================
//   BSP Header Structures
//=============================

typedef struct {
  int fileofs;
  int filelen;
} lump_t;

// LUMP_HEADER
typedef struct {
  char string[4];
  long version;
  lump_t lumps[LUMP_HEADER];
} header_t;

header_t header;

//===================================
// Map structure (w/o header)..
//===================================
typedef struct map_s {
  int          num_entdatas;// count of entdata_t's.
  entdata_t    *entdata;    //  0=LUMP INDEX
  int          num_planes;  // count of plane_t's
  cplane_t     *plane;      //  1=LUMP INDEX
  int          num_vertexs;
  vertex_t     *vertex;     //  2
  int          num_viss;
  vis_t        *vis;        //  3
  int          num_nodes;
  node_t       *node;       //  4
  int          num_texinfos;
  texinfo_t    *texinfo;    //  5
  int          num_faces;
  face_t       *face;       //  6
  long int     num_lightchars;
  lightdata_t  *lightdata;  //  7
  int          num_leafs;
  leaf_t       *leaf;       //  8
  int          num_leaffaces;
  leafface_t   *leafface;   //  9
  int          num_leafbrushes;
  leafbrush_t  *leafbrush;  // 10
  int          num_edges;
  edge_t       *edge;       // 11
  long int     num_surfedges;
  surfedge_t   *surfedge;   // 12
  int          num_models;
  model_t      *model;      // 13
  int          num_brushes;
  brush_t      *brush;      // 14
  int          num_brushsides;
  brushside_t  *brushside;  // 15
  int          num_popchars;
  pop_t        *pop;        // 16
  int          num_areas;
  area_t       *area;       // 17
  int          num_areaportals;
  areaportal_t *areaportal; // 18
} map_t;

map_t *map;

//==================================================
//========== MEMORY ALLOCATION ROUTINES ============
//==================================================

//==================================================
void *xmalloc(unsigned long size) {
void *mem;

  mem = malloc(size);
  if (!mem){
    fprintf(stderr, "xmalloc: %s\n", strerror(errno));
    exit(1); }
  return mem;
}
//==================================================
//============= DATA BUFFER READING ================
//==================================================

unsigned long  getp;     // Location of pointer in buffer
unsigned long  numbytes; // Size of buffer (in bytes)
unsigned char *buffer;   // Pointer to buffered data.

//==================================================
// Move num bytes from location into address
//==================================================
void getmem(void *addr, unsigned long bytes) {
  memmove(addr, buffer+getp, bytes);
  getp += bytes;
}

//=====================================================
//========= ROUTINES FOR READING BSP STRUCTS ==========
//=====================================================

//=====================================================
// LUMP 0 - Read all BSP entdata chars from buffer.
//=====================================================
entdata_t *read_entdata(void) {
entdata_t *entdata;
int i;

  // Set GET pointer to start of entdata_t data
  getp = header.lumps[LUMP_ENTDATA].fileofs;

  // How many entdata chars are there?
  map->num_entdatas = header.lumps[LUMP_ENTDATA].filelen;

  fprintf(stderr, "entdata count=%d\n",map->num_entdatas);

  if (map->num_entdatas <= 0) return NULL;

  // Allocate 1 entdata_t structure
  entdata = (entdata_t *)xmalloc(sizeof(entdata_t));

  // Sequentially read entchar[] chars from buffer
  for (i=0; i < map->num_entdatas; i++)
    getmem((void *)&entdata->entchar[i],sizeof(entdata->entchar[0]));

  return entdata;
}

//=====================================================
// LUMP 1 - Read all BSP plane structs from buffer.
// Modified to use cplane_t instead
//=====================================================
cplane_t *read_planes(void) {
cplane_t *plane;
int i;

  // Set GET pointer to start of plane_t data
  getp = header.lumps[LUMP_PLANES].fileofs;

  // How many plane_t records are there?
  map->num_planes = header.lumps[LUMP_PLANES].filelen/sizeof(plane_t);

  fprintf(stderr, "plane count=%d\n",map->num_planes);

  if (map->num_planes <= 0) return NULL;

  // Allocate all plane_t structures for num_planes
  plane = (cplane_t *)xmalloc(map->num_planes*sizeof(cplane_t));

  // Sequentially read plane_t structs from buffer
  for (i=0; i < map->num_planes; i++) {
    getmem((void *)&plane[i],sizeof(plane_t));

  plane[i].signbits=(int)(plane[i].normal[0] < 0.0F);

  if (plane[i].normal[1] < 0.0F)
    plane[i].signbits |= 2;

  if (plane[i].normal[2] < 0.0F)
    plane[i].signbits |= 4;

  plane[i].pad[0] = 0;
  plane[i].pad[1] = 0; }

  return plane;
}

//=====================================================
// LUMP 2 - Read all BSP vertex structs from buffer.
//=====================================================
vertex_t *read_vertexs(void) {
vertex_t *vertex;
int i;

  // Set GET pointer to start of vertex_t data
  getp = header.lumps[LUMP_VERTEX].fileofs;

  // How many vertex_t records are there?
  map->num_vertexs = header.lumps[LUMP_VERTEX].filelen/sizeof(vertex_t);

  fprintf(stderr, "vertex count=%d\n",map->num_vertexs);

  if (map->num_vertexs <= 0) return NULL;

  // Allocate all vertex_t structures for num_vertexs
  vertex = (vertex_t *)xmalloc(map->num_vertexs*sizeof(vertex_t));

  // Sequentially read vertex_t structs from buffer
  for (i=0; i < map->num_vertexs; i++)
    getmem((void *)&vertex[i],sizeof(vertex_t));

  return vertex;
}

//=====================================================
// LUMP 3 - Read all BSP vis structs from buffer.
//=====================================================
vis_t *read_vis(void) {
vis_t *viss;
int i;

  // Set GET pointer to start of vis_t data
  getp = header.lumps[LUMP_VISLIST].fileofs;

  // How many vis_t records are there?
  map->num_viss = header.lumps[LUMP_VISLIST].filelen/sizeof(vis_t);

  fprintf(stderr, "vis count=%d\n",map->num_viss);

  if (map->num_viss <= 0) return NULL;

  // Allocate all vis_t structures for num_viss
  viss = (vis_t *)xmalloc(map->num_viss*sizeof(vis_t));

  // Sequentially read vis_t structs from buffer
  for (i=0; i < map->num_viss; i++)
    getmem((void *)&viss[i],sizeof(vis_t));

  return viss;
}

//=====================================================
// LUMP 4 - Read all BSP node structs from buffer.
//=====================================================
node_t *read_nodes(void) {
node_t *node;
int i;

  // Set GET pointer to start of node_t data
  getp = header.lumps[LUMP_NODES].fileofs;

  // How many node_t records are there?
  map->num_nodes = header.lumps[LUMP_NODES].filelen/sizeof(node_t);

  fprintf(stderr, "node count=%d\n",map->num_nodes);

  if (map->num_nodes <= 0) return NULL;

  // Allocate all node_t structures for num_nodes
  node = (node_t *)xmalloc(map->num_nodes*sizeof(node_t));

  // Sequentially read node_t structs from buffer
  for (i=0; i < map->num_nodes; i++)
    getmem((void *)&node[i],sizeof(node_t));

  return node;
}

//=====================================================
// LUMP 5 - Read all BSP texinfo structs from buffer.
//=====================================================
texinfo_t *read_texinfo(void) {
texinfo_t *texinfo;
int i;

  // Set GET pointer to start of texinfo_t data
  getp = header.lumps[LUMP_TEXINFO].fileofs;

  // How many texinfo_t records are there?
  map->num_texinfos = header.lumps[LUMP_TEXINFO].filelen/sizeof(texinfo_t);

  fprintf(stderr, "texinfo count=%d\n",map->num_texinfos);

  if (map->num_texinfos <= 0) return NULL;

  // Allocate all texinfo_t structures for num_texinfos
  texinfo = (texinfo_t *)xmalloc(map->num_texinfos*sizeof(texinfo_t));

  // Sequentially read texinfo_t structs from buffer
  for (i=0; i < map->num_texinfos; i++)
    getmem((void *)&texinfo[i],sizeof(texinfo_t));

  return texinfo;
}

//=====================================================
// LUMP 6 - Read all BSP face structs from buffer.
//=====================================================
face_t *read_faces(void) {
face_t *face;
int i;

  // Set GET pointer to start of face_t data
  getp = header.lumps[LUMP_FACES].fileofs;

  // How many face_t records are there?
  map->num_faces = header.lumps[LUMP_FACES].filelen/sizeof(face_t);

  fprintf(stderr, "face count=%d\n",map->num_faces);

  if (map->num_faces <= 0) return NULL;

  // Allocate all face_t structures for num_faces
  face = (face_t *)xmalloc(map->num_faces*sizeof(face_t));

  // Sequentially read face_t structs from buffer
  for (i=0; i < map->num_faces; i++)
    getmem((void *)&face[i],sizeof(face_t));

  return face;
}

//=====================================================
// LUMP 7 - Read all BSP lightdata chars from buffer.
//=====================================================
lightdata_t *read_lightdata(void) {
lightdata_t *lightdata;
lightdata_t templight;
long int i;

  // Set GET pointer to start of lightdata_t data
  getp = header.lumps[LUMP_LIGHTDATA].fileofs;

  // How many lightdata chars are there?
  map->num_lightchars = header.lumps[LUMP_LIGHTDATA].filelen;

  fprintf(stderr, "lightdata count=%d\n",map->num_lightchars);

  if (map->num_lightchars <= 0) return NULL;

  // Allocate 1 lightdata_t structure
  lightdata = (lightdata_t *)xmalloc(map->num_lightchars*sizeof(templight.lightchar[0]));

  // Sequentially read all lightchar[] chars from buffer
  for (i=0; i < map->num_lightchars; i++)
    getmem((void *)&lightdata->lightchar[i],sizeof(lightdata->lightchar[0]));

  return lightdata;
}

//=====================================================
// LUMP 8 - Read all BSP leaf structs from buffer.
//=====================================================
leaf_t *read_leafs(void) {
leaf_t *leaf;
int i;

  // Set GET pointer to start of leaf_t data
  getp = header.lumps[LUMP_LEAFS].fileofs;

  // How many leaf_t records are there?
  map->num_leafs = header.lumps[LUMP_LEAFS].filelen/sizeof(leaf_t);

  fprintf(stderr, "leaf count=%d\n",map->num_leafs);

  if (map->num_leafs <= 0) return NULL;

  // Allocate all leaf_t structures for num_leafs
  leaf = (leaf_t *)xmalloc(map->num_leafs*sizeof(leaf_t));

  // Sequentially read leaf_t structs from buffer
  for (i=0; i < map->num_leafs; i++)
    getmem((void *)&leaf[i],sizeof(leaf_t));

  return leaf;
}

//=====================================================
// LUMP 9 - Read all BSP leafface structs from buffer.
//=====================================================
leafface_t *read_leaffaces(void) {
leafface_t *leafface;
leafface_t tempface;
int i;

  // Set GET pointer to start of leafface_t data
  getp = header.lumps[LUMP_LEAFFACES].fileofs;

  // How many leafface_t records are there?
  map->num_leaffaces = header.lumps[LUMP_LEAFFACES].filelen;

  fprintf(stderr, "leafface count=%d\n",map->num_leaffaces);

  if (map->num_leaffaces <= 0) return NULL;

  // Allocate 1 leafface_t structure
  leafface = (leafface_t *)xmalloc(map->num_leaffaces*sizeof(tempface.dleaffaces[0]));

  // Sequentially read all dleafface[] chars from buffer
  for (i=0; i < map->num_leaffaces; i++)
    getmem((void *)&leafface->dleaffaces[i],sizeof(leafface->dleaffaces[0]));

  return leafface;
}

//=====================================================
// LUMP 10 - Read all BSP leafbrush chars from buffer.
//=====================================================
leafbrush_t *read_leafbrushes(void) {
leafbrush_t *leafbrush;
leafbrush_t tempbrush;
int i;

  // Set GET pointer to start of leafbrush_t data
  getp = header.lumps[LUMP_LEAFBRUSHES].fileofs;

  // How many leafbrush_t records are there?
  map->num_leafbrushes = header.lumps[LUMP_LEAFBRUSHES].filelen;

  fprintf(stderr, "leafbrush count=%d\n",map->num_leafbrushes);

  if (map->num_leafbrushes <= 0) return NULL;

  // Allocate 1 leafbrush_t structure
  leafbrush = (leafbrush_t *)xmalloc(map->num_leafbrushes*sizeof(tempbrush.dleafbrushes[0]));

  // Sequentially read all dleafbrushes[] chars from buffer
  for (i=0; i < map->num_leafbrushes; i++)
    getmem((void *)&leafbrush->dleafbrushes[i],sizeof(leafbrush->dleafbrushes[0]));

  return leafbrush;
}

//=====================================================
// LUMP 11 - Read all BSP edge structs from buffer.
//=====================================================
edge_t *read_edges(void) {
edge_t *edge;
int i;

  // Set GET pointer to start of edge_t data
  getp = header.lumps[LUMP_EDGES].fileofs;

  // How many edge_t records are there?
  map->num_edges = header.lumps[LUMP_EDGES].filelen/sizeof(edge_t);

  fprintf(stderr, "edge count=%d\n",map->num_edges);

  if (map->num_edges <= 0) return NULL;

  // Allocate all edge_t structures for num_edges
  edge = (edge_t *)xmalloc(map->num_edges*sizeof(edge_t));

  // Sequentially read edge_t structs from buffer
  for (i=0; i < map->num_edges; i++)
    getmem((void *)&edge[i],sizeof(edge_t));

  return edge;
}

//=====================================================
// LUMP 12 - Read all BSP surfedge chars from buffer.
//=====================================================
surfedge_t *read_surfedges(void) {
surfedge_t *surfedge;
surfedge_t surftemp;
long int i;

  // Set GET pointer to start of surfedge_t data
  getp = header.lumps[LUMP_SURFEDGES].fileofs;

  // How many surfedge_t records are there?
  map->num_surfedges = header.lumps[LUMP_SURFEDGES].filelen;

  fprintf(stderr, "surfedge count=%d\n",map->num_surfedges);

  if (map->num_surfedges <= 0) return NULL;

  // Allocate 1 surfedge_t structure
  surfedge = (surfedge_t *) xmalloc(map->num_surfedges*sizeof(surftemp.dsurfedges[0]));

  // Sequentially read all dsurfedges[] chars from buffer
  for (i=0; i < map->num_surfedges; i++)
    getmem((void *)&surfedge->dsurfedges[i],sizeof(surfedge->dsurfedges[0]));

  return surfedge;
}

//=====================================================
// LUMP 13 - Read all BSP model structs from buffer.
//=====================================================
model_t *read_models(void) {
model_t *model;
int i;

  // Set GET pointer to start of model_t data
  getp = header.lumps[LUMP_MODELS].fileofs;

  // How many model_t records are there?
  map->num_models = header.lumps[LUMP_MODELS].filelen/sizeof(model_t);

  fprintf(stderr, "model count=%d\n",map->num_models);

  if (map->num_models <= 0) return NULL;

  // Allocate all model_t structures for num_models
  model = (model_t *)xmalloc(map->num_models*sizeof(model_t));

  // Sequentially read model_t structs from buffer
  for (i=0; i < map->num_models; i++)
    getmem((void *)&model[i],sizeof(model_t));

  return model;
}

//=====================================================
// LUMP 14 - Read all BSP brush structs from buffer.
//=====================================================
brush_t *read_brushes(void) {
brush_t *brush;
int i;

  // Set GET pointer to start of brush_t data
  getp = header.lumps[LUMP_BRUSHES].fileofs;

  // How many brush_t records are there?
  map->num_brushes = header.lumps[LUMP_BRUSHES].filelen/sizeof(brush_t);

  fprintf(stderr, "brush count=%d\n",map->num_brushes);

  if (map->num_brushes <= 0) return NULL;

  // Allocate all brush_t structures for num_brushes
  brush = (brush_t *)xmalloc(map->num_brushes*sizeof(brush_t));

  // Sequentially read brush_t structs from buffer
  for (i=0; i < map->num_brushes; i++)
    getmem((void *)&brush[i],sizeof(brush_t));

  return brush;
}

//=====================================================
// LUMP 15 - Read all BSP brushside structs from buffer.
//=====================================================
brushside_t *read_brushsides(void) {
brushside_t *brushside;
int i;

  // Set GET pointer to start of brushside_t data
  getp = header.lumps[LUMP_BRUSHSIDES].fileofs;

  // How many brushside_t records are there?
  map->num_brushsides = header.lumps[LUMP_BRUSHSIDES].filelen/sizeof(brushside_t);

  fprintf(stderr, "brushside count=%d\n",map->num_brushsides);

  if (map->num_brushsides <= 0) return NULL;

  // Allocate all brushside_t structures for num_brushsides
  brushside = (brushside_t *)xmalloc(map->num_brushsides*sizeof(brushside_t));

  // Sequentially read brushside_t structs from buffer
  for (i=0; i < map->num_brushsides; i++)
    getmem((void *)&brushside[i],sizeof(brushside_t));

  return brushside;
}

//=====================================================
// LUMP 16 - Read all BSP pop chars from buffer.
//=====================================================
pop_t *read_pop(void) {
pop_t *pop;
int i;

  // Set GET pointer to start of pop_t data
  getp = header.lumps[LUMP_POP].fileofs;

  // How many pop chars are there?
  map->num_popchars = header.lumps[LUMP_POP].filelen;

  fprintf(stderr, "pop count=%d\n",map->num_popchars);

  if (map->num_popchars <= 0) return NULL;

  // Allocate 1 pop_t structure
  pop = (pop_t *)xmalloc(sizeof(pop_t));

  // Sequentially read dpop[] bytes from buffer
  for (i=0; i < map->num_popchars; i++)
    getmem((void *)&pop->dpop[i],sizeof(pop->dpop[0]));

  return pop;
}

//=====================================================
// LUMP 17 - Read all BSP area structs from buffer.
//=====================================================
area_t *read_areas(void) {
area_t *area;
int i;

  // Set GET pointer to start of area_t data
  getp = header.lumps[LUMP_AREAS].fileofs;

  // How many area_t records are there?
  map->num_areas = header.lumps[LUMP_AREAS].filelen/sizeof(area_t);

  fprintf(stderr, "area count=%d\n",map->num_areas);

  if (map->num_areas <= 0) return NULL;

  // Allocate all area_t structures for num_areas
  area = (area_t *)xmalloc(map->num_areas*sizeof(area_t));

  // Sequentially read area_t structs from buffer
  for (i=0; i < map->num_areas; i++)
    getmem((void *)&area[i],sizeof(area_t));

  return area;
}

//=====================================================
// LUMP 18 - Read all BSP areaportal structs from buffer.
//=====================================================
areaportal_t *read_areaportals(void) {
areaportal_t *areaportal;
int i;

  // Set GET pointer to start of areaportal_t data
  getp = header.lumps[LUMP_AREAPORTALS].fileofs;

  // How many areaportal_t records are there?
  map->num_areaportals = header.lumps[LUMP_AREAPORTALS].filelen/sizeof(areaportal_t);

  fprintf(stderr, "areaportal count=%d\n",map->num_areaportals);

  if (map->num_areaportals <= 0) return NULL;

  // Allocate all areaportal_t structures for num_areaportals
  areaportal = (areaportal_t *)xmalloc(map->num_areaportals*sizeof(areaportal_t));

  // Sequentially read areaportal_t structs from buffer
  for (i=0; i < map->num_areaportals; i++)
    getmem((void *)&areaportal[i],sizeof(areaportal_t));

  return areaportal;
}

//================================================
// Reads entire BSP file into map_t struct.
//================================================
void load_bsp_map(char *filepath) {
FILE *f;
long size;

  f = fopen(filepath, "rb");
  if (!f) {
    fprintf(stderr, "fopen: %s\n", strerror(errno));
    fclose(f);
    return ; }

  errno=0;
  fseek(f, 0L, SEEK_END);
  if (errno) {
    fprintf(stderr, "fseek: %s\n", strerror(errno));
    return ; }

  errno=0;
  size = ftell(f);
  if (errno) {
    fprintf(stderr, "ftell: %s\n", strerror(errno));
    fclose(f);
    return ; }

  fseek(f, 0L, SEEK_SET);
  buffer=xmalloc(size);

  fread(buffer, 1, size, f);
  fclose(f);

  // Read BSP header
  getmem((void*)&header,sizeof(header_t));

  // Allocate entire map_t struct
  map = (map_t *)xmalloc(sizeof(map_t));

  map->entdata = read_entdata();
  map->plane = read_planes();
  map->vertex = read_vertexs();
  map->vis = read_vis();
  map->node = read_nodes();
  map->texinfo = read_texinfo();
  map->face = read_faces();
  map->lightdata = read_lightdata();
  map->leaf = read_leafs();
  map->leafface = read_leaffaces();
  map->leafbrush = read_leafbrushes();
  map->edge = read_edges();
  map->surfedge = read_surfedges();
  map->model = read_models();
  map->brush = read_brushes();
  map->brushside = read_brushsides();
  map->pop = read_pop();
  map->area = read_areas();
  map->areaportal = read_areaportals();

  free(buffer);
}

//=================================================
int main(int argc, char *argv[]) {
char t;

  fprintf(stderr, "BSP Tree contains: \n");
  fprintf(stderr, "----------------------\n");
  load_bsp_map("c:\\quake2\\baseq2\\maps\\q2dm2.bsp");
  fprintf(stderr, "----------------------\n\n\n\n");

  printf("waiting for your input  ");
  t=getchar();

  return 0;
}

 

