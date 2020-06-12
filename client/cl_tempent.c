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
// cl_tent.c -- client side temporary entities

#include "client.h"
#include "particles.h"

//Knightmare- Psychospaz's enhanced particle code
cparticle_t *setupParticle (
			float angle0,		float angle1,		float angle2,
			float org0,			float org1,			float org2,
			float vel0,			float vel1,			float vel2,
			float accel0,		float accel1,		float accel2,
			float color0,		float color1,		float color2,
			float colorvel0,	float colorvel1,	float colorvel2,
			float alpha,		float alphavel,
			int	blendfunc_src,	int blendfunc_dst,
			float size,			float sizevel,			
			int	image,
			int flags,
			void (*think)(cparticle_t *p, vec3_t org, float *alpha, float *size, int *image), qboolean thinknext);

void CL_LightningBeam(vec3_t start, vec3_t end, int srcEnt, int dstEnt, float size);
//end Knightmare

typedef enum
{
	ex_free, ex_explosion, ex_misc, ex_flash, ex_mflash, ex_poly, ex_poly2
} exptype_t;

typedef struct
{
	exptype_t	type;
	entity_t	ent;

	int			frames;
	float		light;
	vec3_t		lightcolor;
	float		start;
	int			baseframe;
} explosion_t;



#define	MAX_EXPLOSIONS	512 //Knightmare- was 32
explosion_t	cl_explosions[MAX_EXPLOSIONS];


#define	MAX_BEAMS	512 //Knightmare- was 32
typedef struct
{
	int		entity;
	int		dest_entity;
	struct model_s	*model;
	int		endtime;
	vec3_t	offset;
	vec3_t	start, end;
} beam_t;
beam_t		cl_beams[MAX_BEAMS];
//PMM - added this for player-linked beams.  Currently only used by the plasma beam
beam_t		cl_playerbeams[MAX_BEAMS];


#define	MAX_LASERS	32
typedef struct
{
	entity_t	ent;
	int			endtime;
} laser_t;
laser_t		cl_lasers[MAX_LASERS];

//ROGUE
cl_sustain_t	cl_sustains[MAX_SUSTAINS];
//ROGUE

//PGM
extern void CL_TeleportParticles (vec3_t org);
//PGM

void vectoangles2 (vec3_t value1, vec3_t angles);

//Knightmare- Psychospaz's enhanced particle code
void CL_Explosion_Particle (vec3_t org, float scale, qboolean rocket);
void CL_Explosion_FlashParticle (vec3_t org, float size, qboolean large);
void CL_BloodHit (vec3_t org, vec3_t dir);
void CL_GreenBloodHit (vec3_t org, vec3_t dir);
void CL_ParticleEffectSparks (vec3_t org, vec3_t dir, vec3_t color, int count);
void CL_ParticleBulletDecal(vec3_t org, vec3_t dir, float size);
void CL_ParticlePlasmaBeamDecal(vec3_t org, vec3_t dir, float size);
void CL_ParticleBlasterDecal (vec3_t org, vec3_t dir, float size, int red, int green, int blue);
void CL_Explosion_Decal (vec3_t org, float size);
//end Knightmare

void CL_Explosion_Sparks (vec3_t org, int size);
void CL_BFGExplosionParticles (vec3_t org);

struct sfx_s	*cl_sfx_ric1;
struct sfx_s	*cl_sfx_ric2;
struct sfx_s	*cl_sfx_ric3;
struct sfx_s	*cl_sfx_lashit;
struct sfx_s	*cl_sfx_spark5;
struct sfx_s	*cl_sfx_spark6;
struct sfx_s	*cl_sfx_spark7;
struct sfx_s	*cl_sfx_railg;
struct sfx_s	*cl_sfx_rockexp;
struct sfx_s	*cl_sfx_grenexp;
struct sfx_s	*cl_sfx_watrexp;
// RAFAEL
//struct sfx_s	*cl_sfx_plasexp;
// Knightmare- shockwave impact
struct sfx_s	*cl_sfx_shockhit;


struct sfx_s	*cl_sfx_footsteps[3];
//Knightmare- Lazarus footstep sounds
struct sfx_s	*cl_sfx_metal_footsteps[4];
struct sfx_s	*cl_sfx_dirt_footsteps[4];
struct sfx_s	*cl_sfx_vent_footsteps[4];
struct sfx_s	*cl_sfx_grate_footsteps[4];
struct sfx_s	*cl_sfx_tile_footsteps[4];
struct sfx_s	*cl_sfx_grass_footsteps[4];
struct sfx_s	*cl_sfx_snow_footsteps[4];
struct sfx_s	*cl_sfx_carpet_footsteps[4];
struct sfx_s	*cl_sfx_force_footsteps[4];
struct sfx_s	*cl_sfx_gravel_footsteps[4];
struct sfx_s	*cl_sfx_ice_footsteps[4];
struct sfx_s	*cl_sfx_sand_footsteps[4];
struct sfx_s	*cl_sfx_wood_footsteps[4];

struct sfx_s	*cl_sfx_slosh[4];
struct sfx_s	*cl_sfx_wade[4];
struct sfx_s	*cl_sfx_mud_wade[2];
struct sfx_s	*cl_sfx_ladder[4];
//end Knightmare

struct model_s	*cl_mod_explode;
struct model_s	*cl_mod_smoke;
struct model_s	*cl_mod_flash;
struct model_s	*cl_mod_parasite_segment;
struct model_s	*cl_mod_grapple_cable;
struct model_s	*cl_mod_parasite_tip;
struct model_s	*cl_mod_explo4;
struct model_s	*cl_mod_bfg_explo;
struct model_s	*cl_mod_powerscreen;
// RAFAEL
struct model_s	*cl_mod_plasmaexplo;

//ROGUE
struct sfx_s	*cl_sfx_lightning;
struct sfx_s	*cl_sfx_disrexp;
struct model_s	*cl_mod_lightning;
struct model_s	*cl_mod_heatbeam;
struct model_s	*cl_mod_monster_heatbeam;
struct model_s	*cl_mod_explo4_big;
//ROGUE
// Knightmare- new effect models
struct model_s	*cl_mod_shocksplash;

void ReadTextureSurfaceAssignments();

/*
=================
CL_RegisterTEntSounds
=================
*/
void CL_RegisterTEntSounds (void)
{
	int		i;
	char	name[MAX_QPATH];

	// PMM - version stuff
//	Com_Printf ("%s\n", ROGUE_VERSION_STRING);
	// PMM
	cl_sfx_ric1 = S_RegisterSound ("world/ric1.wav");
	cl_sfx_ric2 = S_RegisterSound ("world/ric2.wav");
	cl_sfx_ric3 = S_RegisterSound ("world/ric3.wav");
	cl_sfx_lashit = S_RegisterSound("weapons/lashit.wav");
	cl_sfx_spark5 = S_RegisterSound ("world/spark5.wav");
	cl_sfx_spark6 = S_RegisterSound ("world/spark6.wav");
	cl_sfx_spark7 = S_RegisterSound ("world/spark7.wav");
	cl_sfx_railg = S_RegisterSound ("weapons/railgf1a.wav");
	cl_sfx_rockexp = S_RegisterSound ("weapons/rocklx1a.wav");
	cl_sfx_grenexp = S_RegisterSound ("weapons/grenlx1a.wav");
	cl_sfx_watrexp = S_RegisterSound ("weapons/xpld_wat.wav");
	// RAFAEL
	// cl_sfx_plasexp = S_RegisterSound ("weapons/plasexpl.wav");
	// Knightmare- shockwave impact
	cl_sfx_shockhit = S_RegisterSound ("weapons/shockhit.wav");


	S_RegisterSound ("player/land1.wav");
	S_RegisterSound ("player/fall2.wav");
	S_RegisterSound ("player/fall1.wav");

	
	for (i=0 ; i<3 ; i++)
	{
		Com_sprintf (name, sizeof(name), "player/step%i.wav", i+1);		
		cl_sfx_footsteps[i] = S_RegisterSound (name);
	}
	
	
	

	//Knightmare- Lazarus footstep sounds
	for (i=0 ; i<4 ; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_metal%i.wav", i+1);
		cl_sfx_metal_footsteps[i] = S_RegisterSound (name);
	}
	for (i=0 ; i<4 ; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_dirt%i.wav", i+1);
		cl_sfx_dirt_footsteps[i] = S_RegisterSound (name);
	}
	for (i=0 ; i<4 ; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_duct%i.wav", i+1);
		cl_sfx_vent_footsteps[i] = S_RegisterSound (name);
	}
	for (i=0 ; i<4 ; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_grate%i.wav", i+1);
		cl_sfx_grate_footsteps[i] = S_RegisterSound (name);
	}
	for (i=0 ; i<4 ; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_tile%i.wav", i+1);
		cl_sfx_tile_footsteps[i] = S_RegisterSound (name);
	}
	for (i=0 ; i<4 ; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_grass%i.wav", i+1);
		cl_sfx_grass_footsteps[i] = S_RegisterSound (name);
	}
	for (i=0 ; i<4 ; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_snow%i.wav", i+1);
		cl_sfx_snow_footsteps[i] = S_RegisterSound (name);
	}
	for (i=0 ; i<4 ; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_carpet%i.wav", i+1);
		cl_sfx_carpet_footsteps[i] = S_RegisterSound (name);
	}
	for (i=0 ; i<4 ; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_force%i.wav", i+1);
		cl_sfx_force_footsteps[i] = S_RegisterSound (name);
	}
	for (i=0 ; i<4 ; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_gravel%i.wav", i+1);
		cl_sfx_gravel_footsteps[i] = S_RegisterSound (name);
	}
	for (i=0; i<4; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_ice%i.wav", i+1);
		cl_sfx_ice_footsteps[i] = S_RegisterSound (name);
	}
	for (i=0; i<4; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_sand%i.wav", i+1);
		cl_sfx_sand_footsteps[i] = S_RegisterSound (name);
	}
	for (i=0; i<4; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_wood%i.wav", i+1);
		cl_sfx_wood_footsteps[i] = S_RegisterSound (name);
	}

	for (i=0; i<4; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_slosh%i.wav", i+1);
		cl_sfx_slosh[i] = S_RegisterSound (name);
	}
	for (i=0; i<4; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_wade%i.wav", i+1);
		cl_sfx_wade[i] = S_RegisterSound (name);
	}
	for (i=0; i<2; i++)
	{
		Com_sprintf (name, sizeof(name), "mud/wade_mud%i.wav", i+1);
		cl_sfx_mud_wade[i] = S_RegisterSound (name);
	}
	for (i=0; i<4; i++)
	{
		Com_sprintf (name, sizeof(name), "player/pl_ladder%i.wav", i+1);
		cl_sfx_ladder[i] = S_RegisterSound (name);
	}
	// Knightmare- read footstep defintion file
	if (cl_footstep_override->value)
		ReadTextureSurfaceAssignments();
	//end Knightmare

//PGM
	cl_sfx_lightning = S_RegisterSound ("weapons/tesla.wav");
	cl_sfx_disrexp = S_RegisterSound ("weapons/disrupthit.wav");
	// version stuff
//	sprintf (name, "weapons/sound%d.wav", ROGUE_VERSION_ID);
//	if (name[0] == 'w')
//		name[0] = 'W';
//PGM
}	

/*
=================
CL_RegisterTEntModels
=================
*/
void CL_RegisterTEntModels (void)
{
	cl_mod_explode = R_RegisterModel ("models/objects/fan/tris.md2");
	cl_mod_smoke = R_RegisterModel ("models/objects/smoke/tris.md2");
	cl_mod_flash = R_RegisterModel ("models/objects/flash/tris.md2");


	//cl_mod_parasite_segment = R_RegisterModel ("models/monsters/parasite/segment/tris.md2");
	cl_mod_parasite_segment = R_RegisterModel ("models/objects/chain/tris.md2");


	cl_mod_grapple_cable = R_RegisterModel ("models/ctf/segment/tris.md2");
	cl_mod_parasite_tip = R_RegisterModel ("models/monsters/parasite/tip/tris.md2");
	cl_mod_explo4 = R_RegisterModel ("models/objects/r_explode/tris.md2");
	cl_mod_bfg_explo = R_RegisterModel ("sprites/s_bfg2.sp2");
	cl_mod_powerscreen = R_RegisterModel ("models/items/armor/effect/tris.md2");

R_RegisterModel ("models/objects/laser/tris.md2");
R_RegisterModel ("models/objects/grenade2/tris.md2");
R_RegisterModel ("models/weapons/v_machn/tris.md2");
R_RegisterModel ("models/weapons/v_handgr/tris.md2");
R_RegisterModel ("models/weapons/v_shotg2/tris.md2");
R_RegisterModel ("models/objects/gibs/bone/tris.md2");
R_RegisterModel ("models/objects/gibs/sm_meat/tris.md2");
R_RegisterModel ("models/objects/gibs/bone2/tris.md2");
// RAFAEL
// R_RegisterModel ("models/objects/blaser/tris.md2");

R_DrawFindPic ("w_machinegun");
R_DrawFindPic ("a_bullets");
R_DrawFindPic ("i_health");
R_DrawFindPic ("a_grenades");

//ROGUE
	cl_mod_explo4_big = R_RegisterModel ("models/objects/r_explode2/tris.md2");
	cl_mod_lightning = R_RegisterModel ("models/proj/lightning/tris.md2");
	cl_mod_heatbeam = R_RegisterModel ("models/proj/beam/tris.md2");
	cl_mod_monster_heatbeam = R_RegisterModel ("models/proj/widowbeam/tris.md2");
//ROGUE

	// Knightmare- new effect models
	cl_mod_shocksplash = R_RegisterModel ("models/objects/shocksplash/tris.md2");
}	

/*
=================
CL_ClearTEnts
=================
*/
void CL_ClearTEnts (void)
{
	memset (cl_beams, 0, sizeof(cl_beams));
	memset (cl_explosions, 0, sizeof(cl_explosions));
	memset (cl_lasers, 0, sizeof(cl_lasers));

//ROGUE
	memset (cl_playerbeams, 0, sizeof(cl_playerbeams));
	memset (cl_sustains, 0, sizeof(cl_sustains));
//ROGUE
}

/*
=================
CL_AllocExplosion
=================
*/
explosion_t *CL_AllocExplosion (void)
{
	int		i;
	int		time;
	int		index;
	
	for (i=0; i<MAX_EXPLOSIONS; i++)
	{
		if (cl_explosions[i].type == ex_free)
		{
			memset (&cl_explosions[i], 0, sizeof (cl_explosions[i]));
			return &cl_explosions[i];
		}
	}
// find the oldest explosion
	time = cl.time;
	index = 0;

	for (i=0; i<MAX_EXPLOSIONS; i++)
		if (cl_explosions[i].start < time)
		{
			time = cl_explosions[i].start;
			index = i;
		}
	memset (&cl_explosions[index], 0, sizeof (cl_explosions[index]));
	return &cl_explosions[index];
}


void CL_Explosion_Flash (vec3_t origin, int dist, int size, qboolean plasma)
{
	int i,j,k;
	int limit = 2;
	vec3_t org;

	if (plasma)	limit = 1; // simpler explosion

	if (cl_particle_scale->value > 1 || plasma)
	{
		for (i=0; i<limit; i++)
			CL_Explosion_FlashParticle(origin, size+2*dist, true);
		return;
	}

	for (i=-1; i<2; i+=2)
		for (j=-1; j<2; j+=2)
			for (k=-1; k<2; k+=2)
			{
				org[0]=origin[0]+i*dist;
				org[1]=origin[1]+j*dist;
				org[2]=origin[2]+k*dist;
				CL_Explosion_FlashParticle(org, size, false);
			}
}

/*
=================
CL_SmokeAndFlash
=================
*/
void CL_SmokeAndFlash(vec3_t origin)
{
	explosion_t	*ex;

	ex = CL_AllocExplosion ();
	VectorCopy (origin, ex->ent.origin);
	ex->type = ex_flash;
	ex->ent.flags |= RF_FULLBRIGHT;
	ex->frames = 2;
	ex->start = cl.frame.servertime - 100;
	ex->ent.model = cl_mod_flash;
}

/*
=================
CL_ParseParticles
=================
*/
void CL_ParseParticles (void)
{
	int		color, count;
	vec3_t	pos, dir;

	MSG_ReadPos (&net_message, pos);
	MSG_ReadDir (&net_message, dir);

	color = MSG_ReadByte (&net_message);

	count = MSG_ReadByte (&net_message);

	CL_ParticleEffect (pos, dir, color, count);
}

/*
=================
CL_ParseBeam
=================
*/
int CL_ParseBeam (struct model_s *model)
{
	int		ent;
	vec3_t	start, end;
	beam_t	*b;
	int		i;
	
	ent = MSG_ReadShort (&net_message);
	
	MSG_ReadPos (&net_message, start);
	MSG_ReadPos (&net_message, end);

// override any beam with the same entity
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->entity == ent)
		{
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 200;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorClear (b->offset);
			return ent;
		}

// find a free beam
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
		{
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 200;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorClear (b->offset);
			return ent;
		}
	}
	Com_Printf ("beam list overflow!\n");	
	return ent;
}

/*
=================
CL_ParseBeam2
=================
*/
int CL_ParseBeam2 (struct model_s *model)
{
	int		ent;
	vec3_t	start, end, offset;
	beam_t	*b;
	int		i;
	
	ent = MSG_ReadShort (&net_message);
	
	MSG_ReadPos (&net_message, start);
	MSG_ReadPos (&net_message, end);
	MSG_ReadPos (&net_message, offset);

//	Com_Printf ("end- %f %f %f\n", end[0], end[1], end[2]);

// override any beam with the same entity

	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->entity == ent)
		{
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 200;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorCopy (offset, b->offset);
			return ent;
		}

// find a free beam
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
		{
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 200;	
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorCopy (offset, b->offset);
			return ent;
		}
	}
	Com_Printf ("beam list overflow!\n");	
	return ent;
}

// ROGUE
/*
=================
CL_ParsePlayerBeam
  - adds to the cl_playerbeam array instead of the cl_beams array
=================
*/
int CL_ParsePlayerBeam (struct model_s *model)
{
	int		ent;
	vec3_t	start, end, offset;
	beam_t	*b;
	int		i;
	
	ent = MSG_ReadShort (&net_message);
	
	MSG_ReadPos (&net_message, start);
	MSG_ReadPos (&net_message, end);
	// PMM - network optimization
	if (model == cl_mod_heatbeam)
		VectorSet(offset, 2, 7, -3);
	else if (model == cl_mod_monster_heatbeam)
	{
		model = cl_mod_heatbeam;
		VectorSet(offset, 0, 0, 0);
	}
	else
		MSG_ReadPos (&net_message, offset);

//	Com_Printf ("end- %f %f %f\n", end[0], end[1], end[2]);

// override any beam with the same entity
// PMM - For player beams, we only want one per player (entity) so..
	for (i=0, b=cl_playerbeams; i<MAX_BEAMS; i++, b++)
	{
		if (b->entity == ent)
		{
			b->entity = ent;
			b->model = model;

			b->endtime = cl.time + 200;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorCopy (offset, b->offset);
			return ent;
		}
	}

// find a free beam
	for (i=0, b=cl_playerbeams; i<MAX_BEAMS; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
		{
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 100;		// PMM - this needs to be 100 to prevent multiple heatbeams
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorCopy (offset, b->offset);
			return ent;
		}
	}
	Com_Printf ("beam list overflow!\n");	
	return ent;
}
//rogue

/*
=================
CL_ParseLightning
=================
*/
//Knightmare- Psychspaz's enhanced particles
int CL_ParseLightning ()
{

	vec3_t start,end; // ,move,vec;
	int		srcEnt, dstEnt; // , len, dec;
	
	srcEnt = MSG_ReadShort (&net_message);
	dstEnt = MSG_ReadShort (&net_message);

	MSG_ReadPos (&net_message, start);
	MSG_ReadPos (&net_message, end);

	CL_LightningBeam(start, end, srcEnt, dstEnt, 5);
		
	return srcEnt;
}


/*
=================
CL_ParseLaser
=================
*/

//Knightmare- Psychspaz's enhanced particles
void CL_ParseLaser (int colors)
{
	vec3_t start,end;
	vec3_t		move;
	vec3_t		vec;
	vec3_t		point;
	float		len;
	float		dec;

	MSG_ReadPos (&net_message, start);
	MSG_ReadPos (&net_message, end);

	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	VectorCopy (vec, point);
	dec = 20;
	VectorScale (vec, dec, vec);
	VectorCopy (start, move);

	while (len > 0)
	{
		len -= dec;

		setupParticle (
			point[0],	point[1],	point[2],
			move[0],	move[1],	move[2],
			0,	0,	0,
			0,		0,		0,
			50,	255,	50,
			0,	0,	0,
			1,		-2.5,
			GL_SRC_ALPHA, GL_ONE,
			dec*TWOTHIRDS,		0,			
			particle_beam,
			PART_DIRECTION,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}


//=============
//ROGUE
void CL_ParseSteam (void)
{
	vec3_t	pos, dir;
	int		id, i;
	int		r;
	int		cnt;
	int		color;
	int		magnitude;
	cl_sustain_t	*s, *free_sustain;

	id = MSG_ReadShort (&net_message);		// an id of -1 is an instant effect
	if (id != -1) // sustains
	{
		

		//Com_Printf ("Sustain effect id %d\n", id);
		free_sustain = NULL;
		for (i=0, s=cl_sustains; i<MAX_SUSTAINS; i++, s++)
		{
			if (s->id == 0)
			{
				free_sustain = s;
				break;
			}
		}
		if (free_sustain)
		{
			s->id = id;
			s->count = MSG_ReadByte (&net_message);
			MSG_ReadPos (&net_message, s->org);
			MSG_ReadDir (&net_message, s->dir);
			r = MSG_ReadByte (&net_message);
			s->color = r & 0xff;
			s->magnitude = MSG_ReadShort (&net_message);
			s->endtime = cl.time + MSG_ReadLong (&net_message);
			s->think = CL_ParticleSteamEffect2;
			s->thinkinterval = 100;
			s->nextthink = cl.time;
		}
		else
		{
			//Com_Printf ("No free sustains!\n");
			// FIXME - read the stuff anyway
			cnt = MSG_ReadByte (&net_message);
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			r = MSG_ReadByte (&net_message);
			magnitude = MSG_ReadShort (&net_message);
			magnitude = MSG_ReadLong (&net_message); // really interval
		}
	}
	else // instant
	{

		

		cnt = MSG_ReadByte (&net_message);
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		r = MSG_ReadByte (&net_message);
		magnitude = MSG_ReadShort (&net_message);
		color = r & 0xff;
		CL_ParticleSteamEffect (pos, dir,
			color8red(color), color8green(color), color8blue(color),
			-20, -20, -20, cnt, magnitude);


		
		//CL_ParticleSteamEffect (pos, dir, 240, 240, 240, -20, -20, -20, cnt, magnitude);
		//S_StartSound (pos,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
	}
}

void CL_ParseWidow (void)
{
	vec3_t	pos;
	int		id, i;
	cl_sustain_t	*s, *free_sustain;

	id = MSG_ReadShort (&net_message);

	free_sustain = NULL;
	for (i=0, s=cl_sustains; i<MAX_SUSTAINS; i++, s++)
	{
		if (s->id == 0)
		{
			free_sustain = s;
			break;
		}
	}
	if (free_sustain)
	{
		s->id = id;
		MSG_ReadPos (&net_message, s->org);
		s->endtime = cl.time + 2100;
		s->think = CL_Widowbeamout;
		s->thinkinterval = 1;
		s->nextthink = cl.time;
	}
	else // no free sustains
	{
		// FIXME - read the stuff anyway
		MSG_ReadPos (&net_message, pos);
	}
}

void CL_ParseNuke (void)
{
	vec3_t	pos;
	int		i;
	cl_sustain_t	*s, *free_sustain;

	free_sustain = NULL;
	for (i=0, s=cl_sustains; i<MAX_SUSTAINS; i++, s++)
	{
		if (s->id == 0)
		{
			free_sustain = s;
			break;
		}
	}
	if (free_sustain)
	{
		s->id = 21000;
		MSG_ReadPos (&net_message, s->org);
		s->endtime = cl.time + 1000;
		s->think = CL_Nukeblast;
		s->thinkinterval = 1;
		s->nextthink = cl.time;
	}
	else // no free sustains
	{
		// FIXME - read the stuff anyway
		MSG_ReadPos (&net_message, pos);
	}
}

//ROGUE
//=============

//Knightmare- Psychospaz's enhanced particle code
void CL_GunSmokeEffect (vec3_t org, vec3_t dir)
{
	vec3_t		velocity, origin;
	int			j;

	for (j=0 ; j<3 ; j++)
	{
		origin[j] = org[j] + dir[j]*10;
		velocity[j] = 10*dir[j];
	}
	velocity[2] = 10;

	CL_ParticleSmokeEffect (origin, velocity, 10);
}

/*
=================
CL_ParseTEnt
=================
*/
static byte splash_color[] = {0x00, 0xe0, 0xb0, 0x50, 0xd0, 0xe0, 0xe8};

void CL_ParseTEnt (void)
{
	int		type;
	vec3_t	pos, pos2, dir;
	explosion_t	*ex;
	int		cnt;
	int		color;
	int		r, i;
	int		ent;
	int		magnitude;

	type = MSG_ReadByte (&net_message);

	switch (type)
	{
	case TE_BLOOD:			// bullet hitting flesh
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
	//Knightmare- Psychospaz's enhanced particle code
		CL_BloodHit (pos, dir);
		break;

	case TE_GUNSHOT:			// bullet hitting wall
	case TE_SPARKS:
	case TE_BULLET_SPARKS:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
	//Knightmare- Psychospaz's enhanced particle code
		if (type == TE_BULLET_SPARKS)
			VectorScale(dir, 2, dir);
		{
			vec3_t color = { 255, 125, 10 };
			CL_ParticleEffectSparks (pos, dir, color, (type == TE_GUNSHOT)? 5 : 10);
		}

		if (type != TE_SPARKS)
		{
			CL_GunSmokeEffect (pos, dir);
		#ifdef DECALS
			if (type == TE_GUNSHOT || type == TE_BULLET_SPARKS)
				CL_ParticleBulletDecal(pos, dir, 2.5);
		#endif
			// impact sound
			cnt = rand()&15;
			if (cnt == 1)
				S_StartSound (pos, 0, 0, cl_sfx_ric1, 1, ATTN_NORM, 0);
			else if (cnt == 2)
				S_StartSound (pos, 0, 0, cl_sfx_ric2, 1, ATTN_NORM, 0);
			else if (cnt == 3)
				S_StartSound (pos, 0, 0, cl_sfx_ric3, 1, ATTN_NORM, 0);
		}
		break;
				
	case TE_SHOTGUN:			// bullet hitting wall
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
	//Knightmare- Psychospaz's enhanced particle code
		CL_GunSmokeEffect (pos, dir);
	#ifdef DECALS
		CL_ParticleBulletDecal(pos, dir, 2.8);
	#endif
		{
			vec3_t color = { 200, 100, 10 };
			CL_ParticleEffectSparks (pos, dir, color, 8);
		}
		break;

	case TE_SCREEN_SPARKS:
	case TE_SHIELD_SPARKS:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		if (type == TE_SCREEN_SPARKS)
			CL_ParticleEffect (pos, dir, 0xd0, 40);
		else
			CL_ParticleEffect (pos, dir, 0xb0, 40);
		//FIXME : replace or remove this sound
		S_StartSound (pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
		break;

	case TE_SPLASH:			// bullet hitting water
		cnt = MSG_ReadByte (&net_message);
		cnt = min(cnt, 40); // cap at 40
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		r = MSG_ReadByte (&net_message);
		if (r > 6)
			color = 0x00;
		else
			color = splash_color[r];
		// Knightmare- Psychospaz's enhanced particle code
		CL_ParticleEffectSplash (pos, dir, color, cnt);
		if (r == SPLASH_SPARKS)
		{
			r = rand() & 3;
			if (r == 0)
				S_StartSound (pos, 0, 0, cl_sfx_spark5, 1, ATTN_STATIC, 0);
			else if (r == 1)
				S_StartSound (pos, 0, 0, cl_sfx_spark6, 1, ATTN_STATIC, 0);
			else
				S_StartSound (pos, 0, 0, cl_sfx_spark7, 1, ATTN_STATIC, 0);
		}
		break;

	case TE_LASER_SPARKS:
		cnt = MSG_ReadByte (&net_message);
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		color = MSG_ReadByte (&net_message);
		CL_ParticleEffect2 (pos, dir, color, cnt);
		break;
	case TE_FREONHIT:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);


		CL_ParticleBlasterDecal(pos, dir, 7, 100,230,250);

		CL_FreonParticles (pos, dir, 1, 100,230,250, 0, -90, -30); // was 40

		

		


		break;
	case TE_BLASTER:			// blaster hitting wall
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
	//Knightmare- Psychospaz's enhanced particle code
	#if 1
		{
			int numparts = 40 / max(cl_particle_scale->value/2, 1);
			CL_BlasterParticles (pos, dir, numparts, 255, 150, 50, 0, -90, -30); // was 40
		#ifdef DECALS
			CL_ParticleBlasterDecal(pos, dir, 10, 255, 150, 50);
		#endif
		}
		for (r=0; r<3; r++)
		{
			dir[r]*=10;
			pos[r]+=dir[r];
		}
	#else
		CL_BlasterParticles (pos, dir);

		ex = CL_AllocExplosion ();
		VectorCopy (pos, ex->ent.origin);
		ex->ent.angles[0] = acos(dir[2])/M_PI*180;
	// PMM - fixed to correct for pitch of 0
		if (dir[0])
			ex->ent.angles[1] = atan2(dir[1], dir[0])/M_PI*180;
		else if (dir[1] > 0)
			ex->ent.angles[1] = 90;
		else if (dir[1] < 0)
			ex->ent.angles[1] = 270;
		else
			ex->ent.angles[1] = 0;

		ex->type = ex_misc;
		ex->ent.flags |= RF_FULLBRIGHT|RF_TRANSLUCENT|RF_NOSHADOW; // Knightmare- no shadow on these
		ex->start = cl.frame.servertime - 100;
		ex->light = 150;
		ex->lightcolor[0] = 1;
		ex->lightcolor[1] = 1;
		ex->ent.model = cl_mod_explode;
		ex->frames = 4;
	#endif
		//S_StartSound (pos,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
		break;
		
	case TE_RAILTRAIL:			// railgun effect
		MSG_ReadPos (&net_message, pos);
		MSG_ReadPos (&net_message, pos2);
		CL_RailTrail (pos, pos2, false);
		S_StartSound (pos2, 0, 0, cl_sfx_railg, 1, ATTN_NORM, 0);
		break;
	// Knightmare 12/23/2001 - red railgun trail
	case TE_RAILTRAIL2:			// red railgun effect
		MSG_ReadPos (&net_message, pos);
		MSG_ReadPos (&net_message, pos2);
		CL_RailTrail (pos, pos2, true);
		S_StartSound (pos2, 0, 0, cl_sfx_railg, 1, ATTN_NORM, 0);
		break;
	// end Knightmare

	case TE_SHRED:

		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);

		CL_shred (pos, dir, 32,
			255,255,255,
			0,0, 0); // was 40

		break;

	case TE_LOBBYGLASS:

		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);

		CL_lobbyglass (pos, dir, 48,
			255,255,255,
			0,0, 0); // was 40

		break;

	case TE_BURPGAS:
		//bc
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);

		CL_burpgas (pos, dir, 1,
			100,190,190,
			190,0, 0); // was 40





		break;
	case TE_CIGSMOKE:
		//bc
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);

		CL_cigsmoke (pos, dir, 1, 100,
			190,190, 190,
			0, 0,0); // was 40

		break;
	case TE_CIGBIGSMOKE:
		//bc
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);

		CL_cigbigsmoke (pos, dir, 1, 100,
			190,190, 190,
			0, 0,0); // was 40

		break;
	case TE_WINEGLASSBREAK:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);

		CL_wineglassbreak (pos, dir, 32, 255,
			255,255,
			0,
			0, 0,0); // was 40

		break;
	case TE_EXPLOSION2:
	case TE_GRENADE_EXPLOSION:
	case TE_GRENADE_EXPLOSION_WATER:
		MSG_ReadPos (&net_message, pos);
	//Knightmare- Psychospaz's enhanced particle code
		CL_Explosion_Particle (pos, 0, false);
		if (type!=TE_GRENADE_EXPLOSION_WATER)
			CL_Explosion_Flash (pos, 10, 50, false);
		CL_Explosion_Sparks (pos, 32);
	#ifdef DECALS
		if (type != TE_EXPLOSION2)
			CL_Explosion_Decal (pos, 50);
	#endif
		if (type == TE_GRENADE_EXPLOSION_WATER)
			S_StartSound (pos, 0, 0, cl_sfx_watrexp, 1, ATTN_NORM, 0);
		else
			S_StartSound (pos, 0, 0, cl_sfx_grenexp, 1, ATTN_NORM, 0);
		break;

	// RAFAEL
	case TE_PLASMA_EXPLOSION:
		MSG_ReadPos (&net_message, pos);
	//Knightmare- Psychospaz's enhanced particle code
		CL_Explosion_Particle (pos, 0, true);
		CL_Explosion_Sparks (pos, 32);
		CL_Explosion_Flash (pos, 10, 50, true);
	#ifdef DECALS
		CL_Explosion_Decal (pos, 50);
	#endif
		S_StartSound (pos, 0, 0, cl_sfx_rockexp, 1, ATTN_NORM, 0);
		break;
//Knightmare- Psychospaz's enhanced particle code
	case TE_EXPLOSION1_BIG:						// PMM
		MSG_ReadPos (&net_message, pos);
		
		CL_Explosion_Particle (pos, 150, true); //Knightmare- increased size
		CL_Explosion_Sparks (pos, 96);
		CL_Explosion_Flash (pos, 30, 150, false);

			S_StartSound (pos, 0, 0, cl_sfx_rockexp, 1, ATTN_NORM, 0);
		break;
	case TE_EXPLOSION1_NP:						// PMM

		


		MSG_ReadPos (&net_message, pos);
		//MSG_ReadDir (&net_message, dir);

		
		VectorSet(dir, 0, 0, 0);

 		CL_WaterfallParticles (pos, dir, 64, 190, 170, 255, 0, 0, 0);
		
		


		/*
		MSG_ReadPos (&net_message, pos);
		
		CL_Explosion_Particle (pos, 50, true);
		CL_Explosion_Sparks (pos, 21);
		CL_Explosion_Flash (pos, 7, 32, false);

			S_StartSound (pos, 0, 0, cl_sfx_grenexp, 1, ATTN_NORM, 0);

        */

		//bc waterfall fx.
		break;
	case TE_EXPLOSION1:
	case TE_PLAIN_EXPLOSION:
	case TE_ROCKET_EXPLOSION:
	case TE_ROCKET_EXPLOSION_WATER:
		MSG_ReadPos (&net_message, pos);
		CL_Explosion_Particle (pos, 0, true);
		CL_Explosion_Sparks (pos, 32);
		CL_Explosion_Flash (pos, 10, 50, false);
	#ifdef DECALS
		if (type == TE_ROCKET_EXPLOSION || type == TE_ROCKET_EXPLOSION_WATER)
			CL_Explosion_Decal (pos, 50);
	#endif
		if (type == TE_ROCKET_EXPLOSION_WATER)
			S_StartSound (pos, 0, 0, cl_sfx_watrexp, 1, ATTN_NORM, 0);
		else
			S_StartSound (pos, 0, 0, cl_sfx_rockexp, 1, ATTN_NORM, 0);
		break;

	case TE_BFG_EXPLOSION:
		MSG_ReadPos (&net_message, pos);
		ex = CL_AllocExplosion ();
		VectorCopy (pos, ex->ent.origin);
		ex->type = ex_poly;
		ex->ent.flags |= RF_FULLBRIGHT;
		ex->start = cl.frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[0] = 0.0;
		ex->lightcolor[1] = 1.0;
		ex->lightcolor[2] = 0.0;
		ex->ent.model = cl_mod_bfg_explo;
		ex->ent.flags |= RF_TRANSLUCENT | RF_TRANS_ADDITIVE;
		ex->ent.alpha = 0.30;
		ex->frames = 4;
		break;

	case TE_BFG_BIGEXPLOSION:
		MSG_ReadPos (&net_message, pos);
		CL_BFGExplosionParticles (pos);
		break;

	case TE_BFG_LASER:
		CL_ParseLaser (0xd0d1d2d3);
		break;

	case TE_BUBBLETRAIL:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadPos (&net_message, pos2);
		CL_BubbleTrail (pos, pos2);
		break;

	case TE_PARASITE_ATTACK:
	case TE_MEDIC_CABLE_ATTACK:
		ent = CL_ParseBeam (cl_mod_parasite_segment);
		break;

	case TE_BOSSTPORT:			// boss teleporting to station
		MSG_ReadPos (&net_message, pos);
		CL_BigTeleportParticles (pos);
		S_StartSound (pos, 0, 0, S_RegisterSound ("misc/bigtele.wav"), 1, ATTN_NONE, 0);
		break;

	case TE_GRAPPLE_CABLE:
		ent = CL_ParseBeam2 (cl_mod_grapple_cable);
		break;

	// RAFAEL
	case TE_WELDING_SPARKS:
		cnt = MSG_ReadByte (&net_message);
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		color = MSG_ReadByte (&net_message);
	//Knightmare- Psychospaz's enhanced particle code
		{
			vec3_t sparkcolor = {color8red(color), color8green(color), color8blue(color)};
			CL_ParticleEffectSparks (pos, dir, sparkcolor, 40);
		}
		ex = CL_AllocExplosion ();
		VectorCopy (pos, ex->ent.origin);
		ex->type = ex_flash;
		// note to self
		// we need a better no draw flag
		ex->ent.flags |= RF_BEAM;
		ex->start = cl.frame.servertime - 0.1;
		ex->light = 100 + (rand()%75);
		ex->lightcolor[0] = 1.0;
		ex->lightcolor[1] = 1.0;
		ex->lightcolor[2] = 0.3;
		ex->ent.model = cl_mod_flash;
		ex->frames = 2;
		break;

	case TE_GREENBLOOD:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
	//Knightmare- Psychospaz's enhanced particle code
		CL_GreenBloodHit (pos, dir);
		break;

	// RAFAEL
	case TE_TUNNEL_SPARKS:
		cnt = MSG_ReadByte (&net_message);
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		color = MSG_ReadByte (&net_message);
		CL_ParticleEffect3 (pos, dir, color, cnt);
		break;

//=============
//PGM
		// PMM -following code integrated for flechette (different color)
	case TE_BLASTER2:			// green blaster hitting wall
		


		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);

		if (pos)
		{
			vec3_t color = { 64, 64, 64 };
			CL_ParticleEffectSparks (pos, dir, color, 4);
		}

		break;

		
	case TE_BLUEHYPERBLASTER:	// blue blaster hitting wall
	case TE_REDBLASTER:			// red blaster hitting wall
	case TE_FLECHETTE:			// flechette
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		
	//Knightmare- Psychospaz's enhanced particle code
	#if 1
		// PMM
		{
			int red, green, blue;
			int numparts = 40 / max(cl_particle_scale->value/2, 1);
			if (type == TE_BLASTER2) {
				CL_BlasterParticles (pos, dir, numparts, 50, 235, 50, -10, 0, -10);
				red=50; green=235; blue=50; }
			else if (type == TE_BLUEHYPERBLASTER) {
				CL_BlasterParticles (pos, dir, numparts, 50, 50, 235, -10, 0, -10);
				red=50; green=50; blue=235; }
			else if (type == TE_REDBLASTER) {
				CL_BlasterParticles (pos, dir, numparts, 235, 50, 50, 0, -90, -30);
				red=235; green=50; blue=50; }
			else { // TE_FLECHETTE
 				CL_LavaParticles (pos, dir, numparts, 255, 50, 10, 0, 0, 0);
				red=100; green=100; blue=195; }

		#ifdef DECALS
			if (type != TE_FLECHETTE)
				CL_ParticleBlasterDecal(pos, dir, 10, red, green, blue);
		#endif
		}
	#else
		// PMM
		if (type == TE_BLASTER2)
			CL_BlasterParticles2 (pos, dir, 0xd0);
		else if (type == TE_REDBLASTER)
			CL_BlasterParticles (pos, dir, 0xd0);
		else
			CL_BlasterParticles2 (pos, dir, 0x6f); // 75

		ex = CL_AllocExplosion ();
		VectorCopy (pos, ex->ent.origin);
		ex->ent.angles[0] = acos(dir[2])/M_PI*180;
	// PMM - fixed to correct for pitch of 0
		if (dir[0])
			ex->ent.angles[1] = atan2(dir[1], dir[0])/M_PI*180;
		else if (dir[1] > 0)
			ex->ent.angles[1] = 90;
		else if (dir[1] < 0)
			ex->ent.angles[1] = 270;
		else
			ex->ent.angles[1] = 0;

		ex->type = ex_misc;
		ex->ent.flags |= RF_FULLBRIGHT|RF_TRANSLUCENT|RF_NOSHADOW; // Knightmare- no shadow on these

		// PMM
		if (type == TE_BLASTER2)
			ex->ent.skinnum = 1;
		else if (type == TE_REDBLASTER)
			ex->ent.skinnum = 3;
		else // flechette, bluehyperblaster
			ex->ent.skinnum = 2;

		ex->start = cl.frame.servertime - 100;
		ex->light = 150;
		// PMM
		if (type == TE_BLASTER2)
		{
			ex->lightcolor[0] = 0.15;
			ex->lightcolor[1] = 1;
			ex->lightcolor[2] = 0.15;
		}
		else if (type == TE_BLUEHYPERBLASTER)
		{
			ex->lightcolor[0] = 0.19;
			ex->lightcolor[1] = 0.41;
			ex->lightcolor[2] = 0.75;
		}
		else if (type == TE_REDBLASTER)
		{
			ex->lightcolor[0] = 0.75;
			ex->lightcolor[1] = 0.41;
			ex->lightcolor[2] = 0.19;
		}
		else // flechette
		{
			ex->lightcolor[0] = 0.39;
			ex->lightcolor[1] = 0.61;
			ex->lightcolor[2] = 0.75;
		}
		ex->ent.model = cl_mod_explode;
		ex->frames = 4;
	#endif
		S_StartSound (pos,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
		break;

	// Knightmare- shockwave impact effect
	case TE_SHOCKSPLASH:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		// Spawn 5 rings
		for (i = 0; i < 5; i++)
		{
			ex = CL_AllocExplosion ();
			VectorMA (pos, 16.0*(i+1), dir, ex->ent.origin);
			vectoangles2(dir, ex->ent.angles);
			//VectorCopy (dir, ex->ent.angles);
			ex->type = ex_poly2;
			ex->ent.flags |= RF_FULLBRIGHT|RF_TRANSLUCENT|RF_NOSHADOW;
			ex->start = cl.frame.servertime - 100;

			ex->light = 75;
			ex->lightcolor[0] = 0.39;
			ex->lightcolor[1] = 0.61;
			ex->lightcolor[2] = 0.75;

			ex->ent.model = cl_mod_shocksplash;
			ex->ent.alpha = 0.70;
			ex->baseframe = 4-i;
			ex->frames = 4+i;
		}
		S_StartSound (pos,  0, 0, cl_sfx_shockhit, 1, ATTN_NONE, 0);
		break;
	// end Knightmare

	case TE_LIGHTNING:
	//Knightmare- Psychospaz's enhanced particle code
		ent = CL_ParseLightning ();
		S_StartSound (NULL, ent, CHAN_WEAPON, cl_sfx_lightning, 1, ATTN_NORM, 0);
		break;

	case TE_DEBUGTRAIL:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadPos (&net_message, pos2);
		CL_DebugTrail (pos, pos2);
		break;

	/*case TE_PLAIN_EXPLOSION:
		MSG_ReadPos (&net_message, pos);

		ex = CL_AllocExplosion ();
		VectorCopy (pos, ex->ent.origin);
		ex->type = ex_poly;
		ex->ent.flags |= RF_FULLBRIGHT | RF_NOSHADOW; //Knightmare- noshadow flag
		ex->start = cl.frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[0] = 1.0;
		ex->lightcolor[1] = 0.5;
		ex->lightcolor[2] = 0.5;
		ex->ent.angles[1] = rand() % 360;
		ex->ent.model = cl_mod_explo4;
		if (frand() < 0.5)
			ex->baseframe = 15;
		ex->frames = 15;
		if (type == TE_ROCKET_EXPLOSION_WATER)
			S_StartSound (pos, 0, 0, cl_sfx_watrexp, 1, ATTN_NORM, 0);
		else
			S_StartSound (pos, 0, 0, cl_sfx_rockexp, 1, ATTN_NORM, 0);*/

	case TE_FLASHLIGHT:
		MSG_ReadPos(&net_message, pos);
		ent = MSG_ReadShort(&net_message);
		CL_Flashlight(ent, pos);
		break;

	case TE_FORCEWALL:
		MSG_ReadPos(&net_message, pos);
		MSG_ReadPos(&net_message, pos2);
		color = MSG_ReadByte (&net_message);
		CL_ForceWall(pos, pos2, color);
		break;

	case TE_HEATBEAM:
		//ent = CL_ParsePlayerBeam (cl_mod_heatbeam);

		//BC music notes.
		MSG_ReadPos (&net_message, pos);
		VectorSet(dir, 0, 0, 0);
 		CL_MusicParticles (pos, dir, 64, 255, 255, 255, 0, 0, 0);
		

		break;

	case TE_MONSTER_HEATBEAM:
		ent = CL_ParsePlayerBeam (cl_mod_monster_heatbeam);
		break;

	case TE_HEATBEAM_SPARKS:
		/*cnt = 50;
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		r = 8;
		magnitude = 60;
		CL_ParticleSteamEffect (pos, dir, 240, 240, 240, -20, -20, -20, cnt, magnitude);
		S_StartSound (pos,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);*/

		//bc hearts
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
 		CL_HeartParticles (pos, dir, 64, 255, 255, 255, 0, 0, 0);



		break;
	
	case TE_HEATBEAM_STEAM:
		cnt = 20;
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		magnitude = 60;
		CL_ParticleSteamEffect (pos, dir, 255, 150, 50, 0, -90, -30, cnt, magnitude);
	#ifdef DECALS
		CL_ParticlePlasmaBeamDecal (pos, dir, 10); // added burnmark
	#endif
		S_StartSound (pos,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
		break;

	case TE_STEAM:
		CL_ParseSteam();
		break;

	case TE_BUBBLETRAIL2:
		cnt = 3; // was 8
		MSG_ReadPos (&net_message, pos);
		MSG_ReadPos (&net_message, pos2);
		CL_BubbleTrail2 (pos, pos2, cnt);
		S_StartSound (pos,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
		break;

	case TE_MOREBLOOD:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
	//Knightmare- Psychospaz's enhanced particle code
		CL_BloodHit (pos, dir);
		CL_BloodHit (pos, dir);
		break;

	case TE_CHAINFIST_SMOKE:
		dir[0]=0; dir[1]=0; dir[2]=1;
		MSG_ReadPos(&net_message, pos);
	//Knightmare- Psychospaz's enhanced particle code
		CL_ParticleSmokeEffect (pos, dir, 8);
		break;

	case TE_ELECTRIC_SPARKS:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
	//Knightmare- new blue electric sparks
		CL_ElectricParticles (pos, dir, 70);
		//FIXME : replace or remove this sound
		S_StartSound (pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
		break;

	case TE_TRACKER_EXPLOSION:
		MSG_ReadPos (&net_message, pos);
		CL_ColorFlash (pos, 0, 150, -1, -1, -1);
	//	CL_ColorExplosionParticles (pos, 0, 1);
		CL_Tracker_Explode (pos);
		S_StartSound (pos, 0, 0, cl_sfx_disrexp, 1, ATTN_NORM, 0);
		break;

	case TE_TELEPORT_EFFECT:
	case TE_DBALL_GOAL:
		MSG_ReadPos (&net_message, pos);
		CL_TeleportParticles (pos);
		break;

	case TE_WIDOWBEAMOUT:
		CL_ParseWidow ();
		break;

	case TE_NUKEBLAST:
		CL_ParseNuke ();
		break;

	case TE_WIDOWSPLASH:
		MSG_ReadPos (&net_message, pos);
		CL_WidowSplash (pos);
		break;

	case TE_BLOODCOUGH:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);

		CL_bloodcough (pos, dir,  32, 255,
			255,255,
			0,
			0, 0, 0); // was 40

		break;
//PGM
//==============

	default:
		Com_Error (ERR_DROP, "CL_ParseTEnt: bad type");
	}
}

// Knightmare- backup of client angles
extern vec3_t old_viewangles;

/*
=================
CL_AddBeams
=================
*/
 
void CL_AddBeams (void)
{
	int			i,j;
	beam_t		*b;
	vec3_t		dist, org;
	float		d;
	entity_t	ent;
	float		yaw, pitch;
	float		forward;
	float		len, steps;
	float		model_length;
	// Knightmare added
	frame_t		*oldframe;
	player_state_t	*ps, *ops;
	qboolean	firstperson, chasecam;
	int			handmult;
	vec3_t		thirdp_grapple_offset;
	vec3_t		grapple_offset_dir;

	// Knightmare- chasecam grapple offset stuff
	if (hand)
	{
		if (hand->value == 2)
			handmult = 1;
		else if (hand->value == 1)
			handmult = -1;
		else
			handmult = 1;
	}
	else 
		handmult = 1;
	VectorSet(thirdp_grapple_offset, 6, 16, 16);
	// end Knightmare

// update beams
	for (i=0, b=cl_beams; i< MAX_BEAMS; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
			continue;

		firstperson = ((b->entity == cl.playernum+1) && !cl_3dcam->value);
		chasecam = ((b->entity == cl.playernum+1) && cl_3dcam->value);

		// if coming from the player, update the start position
		if (firstperson)	// entity 0 is the world
		{
			VectorCopy (cl.refdef.vieworg, b->start);
			b->start[2] -= 22;	// adjust for view height
		}
		else if (chasecam)
		{
			vec3_t		f,r,u;

			ps = &cl.frame.playerstate;
			j = (cl.frame.serverframe - 1) & UPDATE_MASK;
			oldframe = &cl.frames[j];
			if (oldframe->serverframe != cl.frame.serverframe-1 || !oldframe->valid)
				oldframe = &cl.frame;
			ops = &oldframe->playerstate;

			AngleVectors(old_viewangles, f, r, u);
			VectorClear (grapple_offset_dir);
			for (j=0; j<3; j++)
			{
				grapple_offset_dir[j] += f[j]*thirdp_grapple_offset[1];
				grapple_offset_dir[j] += r[j]*thirdp_grapple_offset[0]*handmult;
				grapple_offset_dir[j] += u[j]*(-thirdp_grapple_offset[2]);
			}

			for (j=0; j<3; j++)
				b->start[j] = cl.predicted_origin[j] + ops->viewoffset[j]
				+ cl.lerpfrac * (ps->viewoffset[j] - ops->viewoffset[j])
				+ grapple_offset_dir[j];
		//	Com_Printf("%f, %f, %f\n", grapple_offset_dir[0], grapple_offset_dir[1], grapple_offset_dir[2]);
		}
		if (chasecam)
			VectorCopy (b->start, org);
		else
			VectorAdd (b->start, b->offset, org);

	// calculate pitch and yaw
		VectorSubtract (b->end, org, dist);

		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
	// PMM - fixed to correct for pitch of 0
			if (dist[0])
				yaw = (atan2(dist[1], dist[0]) * 180 / M_PI);
			else if (dist[1] > 0)
				yaw = 90;
			else
				yaw = 270;
			if (yaw < 0)
				yaw += 360;
	
			forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = (atan2(dist[2], forward) * -180.0 / M_PI);
			if (pitch < 0)
				pitch += 360.0;
		}

	// add new entities for the beams
		d = VectorNormalize(dist);

		memset (&ent, 0, sizeof(ent));
		if (b->model == cl_mod_lightning)
		{
			model_length = 35.0;
			d-= 20.0;  // correction so it doesn't end in middle of tesla
		}
		else
		{
			model_length = 30.0;
		}
		steps = ceil(d/model_length);
		len = (d-model_length)/(steps-1);

		// PMM - special case for lightning model .. if the real length is shorter than the model,
		// flip it around & draw it from the end to the start.  This prevents the model from going
		// through the tesla mine (instead it goes through the target)
		if ((b->model == cl_mod_lightning) && (d <= model_length))
		{
//			Com_Printf ("special case\n");
			VectorCopy (b->end, ent.origin);
			// offset to push beam outside of tesla model (negative because dist is from end to start
			// for this beam)
//			for (j=0 ; j<3 ; j++)
//				ent.origin[j] -= dist[j]*10.0;
			ent.model = b->model;
			ent.flags |= RF_FULLBRIGHT;
			ent.angles[0] = pitch;
			ent.angles[1] = yaw;
			ent.angles[2] = rand()%360;
			//AnglesToAxis(ent.angles, ent.axis);
			V_AddEntity (&ent);			
			return;
		}
		while (d > 0)
		{
			VectorCopy (org, ent.origin);
			ent.model = b->model;
			if (b->model == cl_mod_lightning)
			{
				ent.flags |= RF_FULLBRIGHT;
				ent.angles[0] = -pitch;
				ent.angles[1] = yaw + 180.0;
				ent.angles[2] = rand()%360;
			}
			else
			{
				ent.angles[0] = pitch;
				ent.angles[1] = yaw;

				//bc
				//ent.angles[2] = rand()%360;

				ent.angles[2] = 24*sin(anglemod(cl.time*0.0015));
			}
			//AnglesToAxis(ent.angles, ent.axis);

			ent.flags |= RF_NOSHADOW; // Knightmare- beams don't cast shadows
//			Com_Printf("B: %d -> %d\n", b->entity, b->dest_entity);
			V_AddEntity (&ent);

			for (j=0 ; j<3 ; j++)
				org[j] += dist[j]*len;
			d -= model_length;
		}
	}
}


/*
//				Com_Printf ("Endpoint:  %f %f %f\n", b->end[0], b->end[1], b->end[2]);
//				Com_Printf ("Pred View Angles:  %f %f %f\n", cl.predicted_angles[0], cl.predicted_angles[1], cl.predicted_angles[2]);
//				Com_Printf ("Act View Angles: %f %f %f\n", cl.refdef.viewangles[0], cl.refdef.viewangles[1], cl.refdef.viewangles[2]);
//				VectorCopy (cl.predicted_origin, b->start);
//				b->start[2] += 22;	// adjust for view height
//				if (fabs(cl.refdef.vieworg[2] - b->start[2]) >= 10) {
//					b->start[2] = cl.refdef.vieworg[2];
//				}

//				Com_Printf ("Time:  %d %d %f\n", cl.time, cls.realtime, cls.frametime);
*/

/*
=================
ROGUE - draw player locked beams
CL_AddPlayerBeams
=================
*/
void CL_AddPlayerBeams (void)
{
	int			i,j;
	beam_t		*b;
	vec3_t		dist, org;
	float		d;
	entity_t	ent;
	float		yaw, pitch;
	float		forward;
	float		len, steps;
	int			framenum;
	float		model_length;
	float		hand_multiplier;
	frame_t		*oldframe;
	player_state_t	*ps, *ops;
	qboolean	firstperson, chasecam;
	int			newhandmult;
	vec3_t		thirdp_pbeam_offset;
	vec3_t		pbeam_offset_dir;

//PMM
	if (hand)
	{
		if (hand->value == 2)
			hand_multiplier = 0;
		else if (hand->value == 1)
			hand_multiplier = -1;
		else
			hand_multiplier = 1;
	}
	else 
	{
		hand_multiplier = 1;
	}
//PMM

	// Knightmare- chasecam beam offset stuff
	newhandmult = hand_multiplier;
	if (newhandmult == 0)
		newhandmult = 1;

	VectorSet(thirdp_pbeam_offset, 6.5, 0, 12);
	// end Knightmare

// update beams
	for (i=0, b=cl_playerbeams; i< MAX_BEAMS; i++, b++)
	{
		vec3_t		f,r,u;

		if (!b->model || b->endtime < cl.time)
			continue;

		firstperson = ((b->entity == cl.playernum+1) && !cl_3dcam->value);
		chasecam = ((b->entity == cl.playernum+1) && cl_3dcam->value);

		if(cl_mod_heatbeam && (b->model == cl_mod_heatbeam))
		{

			// if coming from the player, update the start position
			if (firstperson || chasecam)
			{	
				// set up gun position
				// code straight out of CL_AddViewWeapon
				ps = &cl.frame.playerstate;
				j = (cl.frame.serverframe - 1) & UPDATE_MASK;
				oldframe = &cl.frames[j];
				if (oldframe->serverframe != cl.frame.serverframe-1 || !oldframe->valid)
					oldframe = &cl.frame;		// previous frame was dropped or involid
				ops = &oldframe->playerstate;
				// Knightmare- lerp for chasecam mode
				if (chasecam)
				{	// Knightmare- use player's original viewangles
					AngleVectors(old_viewangles, f, r, u);
					VectorClear (pbeam_offset_dir);
					for (j=0; j<3; j++)
					{
						pbeam_offset_dir[j] += f[j]*thirdp_pbeam_offset[1];
						pbeam_offset_dir[j] += r[j]*thirdp_pbeam_offset[0]*newhandmult;
						pbeam_offset_dir[j] += u[j]*(-thirdp_pbeam_offset[2]);
					}
					for (j=0; j<3; j++)
						b->start[j] = cl.predicted_origin[j] + ops->viewoffset[j]
						+ cl.lerpfrac * (ps->viewoffset[j] - ops->viewoffset[j])
						+ pbeam_offset_dir[j];
				//	Com_Printf("%f, %f, %f\n", pbeam_offset_dir[0], pbeam_offset_dir[1], pbeam_offset_dir[2]);
					VectorMA (b->start, (newhandmult * b->offset[0]), r, org);
					VectorMA (     org, b->offset[1], f, org);
				 	VectorMA (     org, b->offset[2], u, org);
				}
				else // firstperson
				{
					for (j=0; j<3; j++)
					{
						b->start[j] = cl.refdef.vieworg[j] + ops->gunoffset[j]
							+ cl.lerpfrac * (ps->gunoffset[j] - ops->gunoffset[j]);
					}
					VectorMA (b->start, (hand_multiplier * b->offset[0]), cl.v_right, org);
					VectorMA (     org, b->offset[1], cl.v_forward, org);
					VectorMA (     org, b->offset[2], cl.v_up, org);
					if ((hand) && (hand->value == 2)) {
						VectorMA (org, -1, cl.v_up, org);
					}
					// FIXME - take these out when final
					VectorCopy (cl.v_right, r);
					VectorCopy (cl.v_forward, f);
					VectorCopy (cl.v_up, u);
				}
			}
			else // some other player or monster
				VectorCopy (b->start, org);
		}
		else // some other beam model
		{
			// if coming from the player, update the start position
			// Knightmare- skip for chasecam mode
			if (firstperson)
			{
				VectorCopy (cl.refdef.vieworg, b->start);
				b->start[2] -= 22;	// adjust for view height
			}
			VectorAdd (b->start, b->offset, org);
		}

	// calculate pitch and yaw
		VectorSubtract (b->end, org, dist);

//PMM
		if (cl_mod_heatbeam && (b->model == cl_mod_heatbeam) && (firstperson||chasecam))
		{
			vec_t len;

			len = VectorLength (dist);
			VectorScale (f, len, dist);
			if (chasecam)
				VectorMA (dist, (newhandmult * b->offset[0]), r, dist);
			else
				VectorMA (dist, (hand_multiplier * b->offset[0]), r, dist);
			VectorMA (dist, b->offset[1], f, dist);
			VectorMA (dist, b->offset[2], u, dist);
			if (chasecam) {
				VectorMA (dist, -(newhandmult * thirdp_pbeam_offset[0]), r, dist);
				VectorMA (dist, thirdp_pbeam_offset[1], f, dist);
				VectorMA (dist, thirdp_pbeam_offset[2], u, dist);
			}
			if ((hand) && (hand->value == 2) && !chasecam)
				VectorMA (org, -1, cl.v_up, org);
		}
//PMM

		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
	// PMM - fixed to correct for pitch of 0
			if (dist[0])
				yaw = (atan2(dist[1], dist[0]) * 180 / M_PI);
			else if (dist[1] > 0)
				yaw = 90;
			else
				yaw = 270;
			if (yaw < 0)
				yaw += 360;
	
			forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = (atan2(dist[2], forward) * -180.0 / M_PI);
			if (pitch < 0)
				pitch += 360.0; 
		}
		
		if (cl_mod_heatbeam && (b->model == cl_mod_heatbeam) )
		{
			if (!firstperson)
			{
				framenum = 2;
				ent.angles[0] = -pitch;
				ent.angles[1] = yaw + 180.0;
				ent.angles[2] = 0;
				// Knightmare- skip this for chasecam mode
				if (!chasecam)
				{
				//	Com_Printf ("%f %f - %f %f %f\n", -pitch, yaw+180.0, b->offset[0], b->offset[1], b->offset[2]);
					AngleVectors(ent.angles, f, r, u);

					// if it's a non-origin offset, it's a player, so use the hardcoded player offset
					if (!VectorCompare (b->offset, vec3_origin))
					{
						VectorMA (org, -(b->offset[0])+1, r, org);
						VectorMA (org, -(b->offset[1])+12, f, org); //was 0
						VectorMA (org, -(b->offset[2])-5, u, org); //was -10
					}
					else
					{
						// if it's a monster, do the particle effect
						CL_MonsterPlasma_Shell(b->start);
					}
				}
			}
			else
			{
				framenum = 1;
			}
		}

		// if it's the heatbeam, draw the particle effect
		// Knightmare- also do this in chasecam mode
		if ((cl_mod_heatbeam && (b->model == cl_mod_heatbeam) && (firstperson||chasecam)))
		{
			CL_HeatbeamParticles (org, dist);
		}

	// add new entities for the beams
		d = VectorNormalize(dist);

		memset (&ent, 0, sizeof(ent));
		if (b->model == cl_mod_heatbeam)
		{
			model_length = 32.0;
		}
		else if (b->model == cl_mod_lightning)
		{
			model_length = 35.0;
			d-= 20.0;  // correction so it doesn't end in middle of tesla
		}
		else
		{
			model_length = 30.0;
		}
		steps = ceil(d/model_length);
		len = (d-model_length)/(steps-1);

		// PMM - special case for lightning model .. if the real length is shorter than the model,
		// flip it around & draw it from the end to the start.  This prevents the model from going
		// through the tesla mine (instead it goes through the target)
		if ((b->model == cl_mod_lightning) && (d <= model_length))
		{
//			Com_Printf ("special case\n");
			VectorCopy (b->end, ent.origin);
			// offset to push beam outside of tesla model (negative because dist is from end to start
			// for this beam)
//			for (j=0 ; j<3 ; j++)
//				ent.origin[j] -= dist[j]*10.0;
			ent.model = b->model;
			ent.flags |= RF_FULLBRIGHT;
			ent.angles[0] = pitch;
			ent.angles[1] = yaw;
			ent.angles[2] = rand()%360;
			//AnglesToAxis(ent.angles, ent.axis);
			ent.flags |= RF_NOSHADOW; // Knightmare- beams don't cast shadows
			V_AddEntity (&ent);			
			return;
		}
		while (d > 0)
		{
			VectorCopy (org, ent.origin);
			ent.model = b->model;
			if (cl_mod_heatbeam && (b->model == cl_mod_heatbeam))
			{
			//	ent.flags |= RF_FULLBRIGHT|RF_TRANSLUCENT;
			//	ent.alpha = 0.6;
				ent.flags |= RF_FULLBRIGHT;
				ent.angles[0] = -pitch;
				ent.angles[1] = yaw + 180.0;
				ent.angles[2] = (cl.time) % 360;
			//	ent.angles[2] = rand()%360;
				ent.frame = framenum;
			}
			else if (b->model == cl_mod_lightning)
			{
				ent.flags |= RF_FULLBRIGHT;
				ent.angles[0] = -pitch;
				ent.angles[1] = yaw + 180.0;
				ent.angles[2] = rand()%360;
			}
			else
			{
				ent.angles[0] = pitch;
				ent.angles[1] = yaw;
				ent.angles[2] = rand()%360;
			}
			//AnglesToAxis(ent.angles, ent.axis);
			ent.flags |= RF_NOSHADOW; // Knightmare- beams don't cast shadows
		//	Com_Printf("B: %d -> %d\n", b->entity, b->dest_entity);
			V_AddEntity (&ent);

			for (j=0 ; j<3 ; j++)
				org[j] += dist[j]*len;
			d -= model_length;
		}
	}
}

/*
=================
CL_AddExplosions
=================
*/
void CL_AddExplosions (void)
{
	entity_t	*ent;
	int			i;
	explosion_t	*ex;
	float		frac;
	int			f;

	memset (&ent, 0, sizeof(ent));

	for (i=0, ex=cl_explosions ; i< MAX_EXPLOSIONS ; i++, ex++)
	{
		if (ex->type == ex_free)
			continue;
		frac = (cl.time - ex->start)/100.0;
		f = floor(frac);

		ent = &ex->ent;

		switch (ex->type)
		{
		case ex_mflash:
			if (f >= ex->frames-1)
				ex->type = ex_free;
			break;
		case ex_misc:
			if (f >= ex->frames-1)
			{
				ex->type = ex_free;
				break;
			}
			ent->alpha = 1.0 - frac/(ex->frames-1);
			break;
		case ex_flash:
			if (f >= 1)
			{
				ex->type = ex_free;
				break;
			}
			ent->alpha = 1.0;
			break;
		case ex_poly:
			if (f >= ex->frames-1)
			{
				ex->type = ex_free;
				break;
			}

			ent->alpha = (16.0 - (float)f)/16.0;

			if (f < 10)
			{
				ent->skinnum = (f>>1);
				if (ent->skinnum < 0)
					ent->skinnum = 0;
			}
			else
			{
				ent->flags |= RF_TRANSLUCENT;
				if (f < 13)
					ent->skinnum = 5;
				else
					ent->skinnum = 6;
			}
			break;
		case ex_poly2:
			if (f >= ex->frames-1)
			{
				ex->type = ex_free;
				break;
			}
			// Knightmare- since nothing else uses this,
			// I'd just thought I'd change it...
			ent->alpha = max((0.7 - f*0.10), 0.10);
			//ent->alpha = (5.0 - (float)f)/5.0;
			ent->skinnum = 0;
			ent->flags |= RF_TRANSLUCENT;
			break;
		}

		if (ex->type == ex_free)
			continue;
		if (ex->light)
		{
			V_AddLight (ent->origin, ex->light*ent->alpha,
				ex->lightcolor[0], ex->lightcolor[1], ex->lightcolor[2]);
		}

		VectorCopy (ent->origin, ent->oldorigin);

		if (f < 0)
			f = 0;
		ent->frame = ex->baseframe + f + 1;
		ent->oldframe = ex->baseframe + f;
		ent->backlerp = 1.0 - cl.lerpfrac;
		//AnglesToAxis(ent->angles, ent->axis);

		V_AddEntity (ent);
	}
}


/*
=================
CL_AddLasers
=================
*/
void CL_AddLasers (void)
{
	laser_t		*l;
	int			i;

	for (i=0, l=cl_lasers ; i< MAX_LASERS ; i++, l++)
	{
		if (l->endtime >= cl.time)
			V_AddEntity (&l->ent);
	}
}

/* PMM - CL_Sustains */
void CL_ProcessSustain ()
{
	cl_sustain_t	*s;
	int				i;

	for (i=0, s=cl_sustains; i< MAX_SUSTAINS; i++, s++)
	{
		if (s->id)
			if ((s->endtime >= cl.time) && (cl.time >= s->nextthink))
			{
//				Com_Printf ("think %d %d %d\n", cl.time, s->nextthink, s->thinkinterval);
				s->think (s);
			}
			else if (s->endtime < cl.time)
				s->id = 0;
	}
}

/*
=================
CL_AddTEnts
=================
*/
void CL_AddTEnts (void)
{
	CL_AddBeams ();
	// PMM - draw plasma beams
	CL_AddPlayerBeams ();
	CL_AddExplosions ();
	CL_AddLasers ();
	// PMM - set up sustain
	CL_ProcessSustain();
}
