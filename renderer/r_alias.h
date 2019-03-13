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
	qboolean		twosided;
	qboolean		alphatest;
	qboolean		fullbright;
	qboolean		nodraw;
	qboolean		noshadow;
	qboolean		nodiffuse;
	float			envmap;
	float			basealpha;
	float			translate_x;
	float			translate_y;
	float			rotate;
	float			scale_x;
	float			scale_y;
	waveFunc_t		stretch;
	waveFunc_t		turb;
	float			scroll_x;
	float			scroll_y;
	qboolean		blend;
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
