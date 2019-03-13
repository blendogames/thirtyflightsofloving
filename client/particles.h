
// psychospaz's particle system

typedef enum
{
	particle_solid = 1,
	particle_generic,
	particle_inferno,
	particle_smoke,
	particle_blood,
	particle_blooddrop,
	particle_blooddrip,
	particle_redblood,
	particle_bubble,
	particle_blaster,
	particle_blasterblob,
	particle_beam,
	particle_beam2,
	particle_lightning,
//	particle_lensflare,
//	particle_lightflare,
//	particle_shield,
	particle_rflash,
	particle_rexplosion1,
	particle_rexplosion2,
	particle_rexplosion3,
	particle_rexplosion4,
	particle_rexplosion5,
	particle_rexplosion6,
	particle_rexplosion7,
//	particle_dexplosion1,
//	particle_dexplosion2,
//	particle_dexplosion3,
	particle_bfgmark,
	particle_burnmark,
	particle_blooddecal1,
	particle_blooddecal2,
	particle_blooddecal3,
	particle_blooddecal4,
	particle_blooddecal5,
	particle_shadow,
	particle_bulletmark,
	particle_trackermark,
//	particle_footprint,
} particle_type;

#define  GL_ZERO						0x0
#define  GL_ONE							0x1
#define  GL_SRC_COLOR					0x0300
#define  GL_ONE_MINUS_SRC_COLOR			0x0301
#define  GL_SRC_ALPHA					0x0302
#define  GL_ONE_MINUS_SRC_ALPHA			0x0303
#define  GL_DST_ALPHA					0x0304
#define  GL_ONE_MINUS_DST_ALPHA			0x0305
#define  GL_DST_COLOR					0x0306
#define  GL_ONE_MINUS_DST_COLOR			0x0307
#define  GL_SRC_ALPHA_SATURATE			0x0308
#define  GL_CONSTANT_COLOR				0x8001
#define  GL_ONE_MINUS_CONSTANT_COLOR	0x8002
#define  GL_CONSTANT_ALPHA				0x8003
#define  GL_ONE_MINUS_CONSTANT_ALPHA	0x8004
