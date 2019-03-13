
#define MAX_SCALE_SIZE 256

typedef struct
{
	int		s_rate;
	int		s_width;
	int		s_channels;

	int		width;
	int		height;
	int		p2_width;
	int		p2_height;
	byte	*pic;
	byte	*pic_pending;

	// order 1 huffman stuff
	int		*hnodes1;	// [256][256][2];
	int		numhnodes1[256];

	int		h_used[512];
	int		h_count[512];

	byte	*cinematic_file;
	byte	*offset;
	byte	*framestart;

	int		time;
	int		lastupload;
	int		frame;
	byte	rawpalette[768];
	unsigned palette[256];

	int		texnum;

} cinematics_t;

cinematics_t *CIN_OpenCin (char *name);
void CIN_ProcessCins(void);
void CIN_FreeCin (int texnum);

extern cinematics_t	*cin;